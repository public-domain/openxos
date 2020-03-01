//*******************************
// Input/output routines for XMAC
//*******************************
// Written by John Goltz
//*******************************

//++++
// This software is in the public domain.  It may be freely copied and used
// for whatever purpose you see fit, including commerical uses.  Anyone
// modifying this software may claim ownership of the modifications, but not
// the complete derived code.  It would be appreciated if the authors were
// told what this software is being used for, but this is not a requirement.

//   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

#include <STDIO.H>
#include <CTYPE.H>
#include <ERRNO.H>
#include <XOS.H>
#include <XOSERRMSG.H>
#include "XMAC.H"
#include "XMACEXT.H"

// Allocate local data

int argnum;			// Macro argument number being inserted
int gentemp;			// Current generated argument number

//**************************************************
// Function: nxtnbc - Get non-blank source character
// Returned: Character
//**************************************************

char nxtnbc(void)

{
    char chr;

    do
    {   chr = nxtchar();		// Get next input character
    } while (chr != '\n' && isspace(chr));
    return (chr);
}

//**********************************************************
// Function: nxtnb0 - Ensure have non-blank source character
// Returned: Character
//**********************************************************

char nxtnb0(
    char chr)

{
    while (chr != '\n' && isspace(chr))
	chr = nxtchar();
    return (chr);
}

//**********************************************
// Function: nxtchar - Get next source character
// Returned: Character
//**********************************************

char nxtchar(void)

{
    struct ic *tpnt;
    int    rtn;
    char   temp;

    if (eolflg || eofflg)		// At end of line or end of file?
	return ('\0');			// Yes - just return NULL
    if (hldchar)			// Do we have a held character?
    {
	temp = hldchar;			// Yes
	hldchar = '\0';			// Clear it out
	return (temp);			// And return it
    }
    for (;;)
    {
	if (inspnt == NULL)		// Are we inserting now?
	{
	    if (!eofreq)		// Want to force EOF now?
	    {
		if ((rtn=getc(srcfp[srclvl])) >= 0) // No - get character
		{
                    temp = rtn;
		    if (temp == '\n')	// End of line?
                    {
			lineno += linesz; // Yes - bump line number
                        linesz = 1;
                    }
                    else if (temp < ' ' && temp != '\t')
			continue;
                    if (temp == '-')	// Might this be continuation?
                    {
                        if ((rtn=getc(srcfp[srclvl])) >= 0)
                        {
                            if (rtn == '\n') // End of line next?
                            {
                                ++linesz; // Yes - the line is continued
                                continue;
                            }
                            ungetc(rtn, srcfp[srclvl]); //No - put it back
                        }
                    }
		    return (nxtcrtn(temp)); // Return with character
		}
		if (!feof(srcfp[srclvl])) // Error - is it end of file?
		    femsg(prgname, -errno, srcfsp[srclvl]); // No - fail
	    }
	    fclose(srcfp[srclvl]);	// EOF - close current input file
	    prmflg = FALSE;
	    eofreq = FALSE;
	    if (srclvl > 0)		// Are we including?
            {
		lineno = savlineno[srclvl]; // Yes - restore line number
                linesz = savlinesz[srclvl];
		--srclvl;		// Reduce source level
                continue;
            }
	    else
	    {
		eofflg = TRUE;		// No more input - indicate end of
		eolflg = TRUE;		//   file
		lineno = -1;
		return ('\0');
	    }
	}
	if (!((rtn=getinx()) & 0x80))	// Get inserted character
	    return (nxtcrtn(rtn));	// Finished if normal character
	else
	{
	    switch (rtn & 0xFF)
	    {
	     case (CC_IMA & 0xFF):	// Insert macro argument
		macarg();		// Find argument
		break;

	     case (CC_IMX & 0xFF):	// Insert macro argument with
					//   generated symbol if null
		if (!macarg())		// Setup argument if have one
		{
		    if (inspnt->ic_gac < argnum) // None there
			inspnt->ic_gac = argnum;
		    gentemp = gnacnt + argnum - 1;
		    inspnt->ic_sbp = inspnt->ic_cbp; // Save current insert
		    inspnt->ic_sbc = inspnt->ic_cbc; //   state
		    inspnt->ic_cbp = "@@\202\203\204\205\214";
		    inspnt->ic_cbc = 8;	// Setup to insert argument
		}
		break;

	     case (CC_G1S & 0xFF):	// Insert first character of
					//   generated symbol
		return (nxthrtn(gentemp>>12));

	     case (CC_G2S & 0xFF):	// Insert second character of
					//   generated symbol
		return (nxthrtn(gentemp>>8));

	     case (CC_G3S & 0xFF):	// Insert third character of
					//   generated symbol
		return (nxthrtn(gentemp>>4));

	     case (CC_G4S & 0xFF):	// Insert fourth character of
					//   generated symbol
		return (nxthrtn(gentemp));
 
             case (CC_IIA & 0xFF):	// Insert IRP argument
		inspnt->ic_sbp = inspnt->ic_cbp;// Save current insert state
		inspnt->ic_sbc = inspnt->ic_cbc;
		inspnt->ic_cbp = inspnt->ic_.arg->ab_data;
		    				// Setup to insert argument
		inspnt->ic_cbc = BLKSIZE - 8;
		break;

	     case (CC_ICA & 0xFF):	// Insert IRPC argument
		return (nxtcrtn(((struct body *)(inspnt->ic_.arg))->bb_data
                            [inspnt->ic_sbc])); // Return argument character

	     case (CC_EMB & 0xFF):	// End of macro body
		endins();		// Terminate the macro
		break;

	     case (CC_ERB & 0xFF):	// End of repeat body
		if (--inspnt->ic_.cnt > 0) // Need to do more?
		{
		    inspnt->ic_cbp = inspnt->ic_body; // Yes - start over at
		    inspnt->ic_cbc = BLKSIZE - 4;     //   beginning
		}
		else
		    endins();		// No - stop this
		break;

	     case (CC_EIB & 0xFF):	// End of IRP body
		tpnt = (struct ic *)inspnt->ic_.arg; // Point to this argument
		inspnt->ic_.arg = ((struct ab *)tpnt)->ab_next;
					// Get next argument
		givlist((struct ab *)tpnt); // Give up this argument
		if (inspnt->ic_.arg)	// Do we have more to do?
		{
		    inspnt->ic_cbp = inspnt->ic_body; // Yes - start over at
		    inspnt->ic_cbc = BLKSIZE - 4;     //   beginning
		}
		else
		    endins();		// No - stop this
		break;

	     case (CC_ECB & 0xFF):	// End of IRPC body
		if (++inspnt->ic_sbc >= BLKSIZE-4) // Another character in
						   //   this argument block?
		{
		    tpnt = (struct ic *)
			    (((struct body *)(inspnt->ic_.arg))->bb_link);
					// No - advance to next block
		    givblock((struct fb *)(inspnt->ic_.arg));
					// Give up the previous block
		    inspnt->ic_.arg = (struct ab *)tpnt;
					// Reset argument pointer
		    inspnt->ic_sbc = 0;	// And offset in block
		}
		if ((unsigned char)(((struct body *)(inspnt->ic_.arg))->
                        bb_data[inspnt->ic_sbc]) != CC_EAG)
                {			// End of the IRPC?
		    inspnt->ic_cbp = inspnt->ic_body; // No - start over at
		    inspnt->ic_cbc = BLKSIZE - 4;     //   beginning
		}
		else
		    endins();		// Yes
		break;

	     case (CC_EAG & 0xFF):	// End of macro or IRP argument
		inspnt->ic_cbp = inspnt->ic_sbp; // Restore pointer to body
		inspnt->ic_cbc = inspnt->ic_sbc;
		break;
	    }
	}
    }
}

//********************************************************
// Function: nxtcrtn - Check character returned by nxtchar
// Returned: Character
//********************************************************

char nxtcrtn(chr)
char chr;

{
    if (chr == '\n')			// End of line?
	chr = '\0';			// Yes
    else if (chr == '\f')		// Form-feed?
    {
	ffflg = TRUE;			// Yes
	chr = '\0';
    }
    if (chr == '\0')
    {
	eolflg = TRUE;
        bolflg = TRUE;
    }
    if (albcnt < ALBSIZE)		// Room for more in listing buffer?
    {
	*albpnt++ = chr;		// Yes - store character in buffer
	++albcnt;
    }
    bolflg = FALSE;
    return (chr);
}

//************************************************************
// Function: macarg - Find requested macro argument string
// Returned: FALSE if argument is null, TRUE if it is not null
//************************************************************

int macarg(void)

{
    int    cnt;
    struct ab *pnt;

    argnum = cnt = getinx();		// Get argument number
    if ((pnt=inspnt->ic_.arg) == NULL)	// Point to start of argument list
	return (FALSE);
    while (--cnt > 0)
    {
	if ((pnt=pnt->ab_next) == NULL)	// More arguments?
	    return (FALSE);		// No
    }
    if (!pnt)
	return (FALSE);
    if ((unsigned char)(pnt->ab_data[0]) == CC_EAG) // Null argument?
	return (FALSE);			// Yes
    inspnt->ic_sbp = inspnt->ic_cbp;	// No - save current insert state
    inspnt->ic_sbc = inspnt->ic_cbc;
    inspnt->ic_cbp = pnt->ab_data;	// Setup to insert argument
    inspnt->ic_cbc = BLKSIZE - 8;
    return (TRUE);
}

//***************************************************************
// Function: nxthrtn - Return ASCII character for hex digit value
// Returned: Character
//***************************************************************

char nxthrtn(
    long value)

{
    char chr;

    value &= 0xF;			// Remove junk
    chr = (value < 10) ? value+'0' : value+('A'-10);
    return (nxtcrtn(chr));
}

//*******************************************************
// Function: endins - Terminate processing of insert body
// Returned: Nothing
//*******************************************************

void endins(void)

{
    struct ic *tpnt;

    switch (inspnt->ic_type)
    {
    case ICT_MAC:			// If macro body
	gnacnt += inspnt->ic_gac;	// Bump inserted symbol count
	givprm();
	goto endrep4;
    case ICT_IRPC:			// If IRPC body
	givlist(inspnt->ic_.arg);	// Give up the argument list
	goto endrept;			// Join common stuff
    case ICT_IRP:			// If IRP body
	givprm();			// Give up actual parameters
	goto endrept;			// Join common stuff
    case ICT_REPT:			// If repeat body
    endrept:
	givlist((struct ab *)(inspnt->ic_body)); // Give up the body
    endrep4:
	tpnt = inspnt;
	inspnt = inspnt->ic_link;	// Unlink this ICB
	givblock((struct fb *)tpnt);	// Give up the ICB
	return;
    }
}

//******************************************
// Function: getinx - Get inserted character
// Returned: Character
//******************************************


int getinx(void)

{
    if (inspnt->ic_cbc-- == 0)		// Anything left in current block?
    {					// No - advance pointer
	inspnt->ic_cbp = (char *)(*((struct ic **)(inspnt->ic_cbp)));
	inspnt->ic_cbc = BLKSIZE-5;	// Reset count
    }
    return (*inspnt->ic_cbp++);		// Return character
}

//*************************************************
// Function: givprm - Give up actual parameter list
// Returned: Nothing
//*************************************************

void givprm(void)

{
    struct ab *tpnt;
    struct ab *save;

    tpnt = inspnt->ic_.arg;		// Point to first argument
    inspnt->ic_.arg = 0;		// Clear pointer
    while (tpnt)
    {
	save = tpnt->ab_next;		// Get next argument
	givlist(tpnt);			// Give up this one
	tpnt = save;
    }
}

//*********************************************************
// Function: getsrc: Get source file ready for input
// Returned: TRUE if have another source file, FALSE if not
//*********************************************************

int getsrc(void)

{
    if ((thissrc = thissrc->sb_next) != NULL)
    {
	if ((srcfp[0]=fopen(thissrc->sb_fsp, "r")) == 0) // Open the file
	    femsg(prgname, -errno, thissrc->sb_fsp); // If error
	srcfsp[0] = thissrc->sb_fsp;	// Remember where the file spec is
        eofflg = FALSE;			// Not at EOF now
        lineno = 0;			// Reset line number
        linesz = 1;
	return (TRUE);
    }
    else
	return (FALSE);			// If not more input available
}
