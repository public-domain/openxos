//******************************************
// Function to assemble single line for XMAC
//******************************************
// Written by John Goltz
//******************************************

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
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <XOS.H>
#include "XMAC.H"
#include "XMACEXT.H"

// Link to private external data

extern struct pstbl psutbl[];	// Pseudo-op table
extern int    psusize;		// Number of entries in pseudo-op table
extern int    psuents;		// Size of one entry in pseudo-op table
extern struct optbl *opcpnt[];	// Opcode pointer table
extern int    opcsize[];	// Opcode size table

//*****************************************
// Function: asmline - Assemble single line
// Returned: Nothing
//*****************************************

void asmline(void)

{
    struct sy *sym;
    union
    {   struct optbl *o;
        struct pstbl *p;
    }      tbl;
    char  *cpnt;
    int    cnt;
    char   chr;
    char   spastop;

    for (;;)
    {
        if (ffflg)			// Do we have a form-feed?
	{
            ffflg = FALSE;		// Yes
	    nextpage();			// Start new page
	}
	if (getsyq() == 0)		// Collect a symbol
	{
            if (ffflg)			// No symbol - form-feed?
	    {
                eolflg = FALSE;		// Yes - ignore the funny "line"
		continue;
	    }
	    else
	    {
                chkend(stopper); 	// Must be blank line
		return;
	    }
	}				// If get here, have symbol on line
	if (isspace(stopper))		// Stopped by whitespace?
	{
            stopper = nxtnbc();		// Yes - get next non-blank character
	    spastop = TRUE;		// Remember stopped by space
	}
	else
	    spastop = FALSE;
	if (stopper == ':')		// Is this a label?
	{
            chkpsect();			// Yes - must have psect defined
            adrflg = TRUE;		// List address on line
	    sym = findsym(TRUE);	// Look for symbol in symbol table
	    if (sym->sy_flag & SYF_UNDEF) // Is it undefined now?
	    {
                sym->sy_flag = SYF_LABEL|SYF_ADDR; // Yes - indicate this is
						   //   a label and address
		sym->sy_val.val = curpsd->pd_ofs; // Value is current address
		sym->sy_psect = curpsd->pd_psn;
	    }
	    else
	    {
                if (!(sym->sy_flag & SYF_LABEL) // Not defined as a label?
		    || (sym->sy_flag & SYF_MULT)) // Or mult defined?
			errsub(errmmsg, ERR_M); // Yes - error
		else
		{
                    if (sym->sy_val.val != curpsd->pd_ofs // Same value
			|| sym->sy_psect != curpsd->pd_psn)
		    {
                        if (!(sym->sy_flag & SYF_MULT) && passtwo)
					// Phase error?
			    errsub(errpmsg, ERR_P); // Yes
			else
			{
			    errsub(errmmsg, ERR_M); // No
			    sym->sy_flag |= SYF_MULT;
			}
		    }
		}
	    }   
	    if (!prmflg)		// Is this in a parameter file?
		sym->sy_flag |= SYF_USED; // No - indicate symbol is used
	    if ((chr=nxtnbc()) == ':')	// Internal symbol?
	    {				// Yes
                if (!(sym->sy_flag & SYF_EXPORT)) // Also exported?
                    sym->sy_flag |= SYF_INTERN;// No - make it internal
		chr = nxtnbc ();
	    }
	    if (chr == '!')		// Suppressed symbol?
		sym->sy_flag |= SYF_SUP; // Yes
	    else
		hldchar = chr;		// No - put the character back
		continue;		// Process rest of line
	}

	// Here if symbol not followed by ":"

	if (stopper == '=')		// Is this an assignment?
	{				// Yes
	    if (symsize==1 && symbuf.nsym[0]=='$')
	    {				// Setting location counter?
	        exprsn();		// Yes - get value
		if (!chkend(stopper))	// Must have end next
		    return;
		lstadr = curpsd->pd_ofs = val.vl_val.vl_vl;
					// Store new location counter
		if (curpsd->pd_tsize < curpsd->pd_ofs)
		    curpsd->pd_tsize = curpsd->pd_ofs;
		adrflg = TRUE;		// List address
		return;
	    }
	    sym = findsym(TRUE);	// Look for symbol in symbol table
	    if (!prmflg)		// Yes - is this a parameter file?
		sym->sy_flag |= SYF_USED; // No - indicate symbol used
	    if (!symbuf.lsym.ls_flag)	// Local symbol?
	    {
                queerr();		// Yes - Q error
		return;
	    }
	    if ((sym->sy_flag & SYF_LABEL) || (sym->sy_type != SYT_SYM))
					// Already defined as label or as
            {				//   segment, msect or psect?
                errsub(errmmsg, ERR_M);	// Yes - multiply defined
		skiprest();
		return;
	    }
	    chr = nxtnbc();		// Get next character
	    if (chr == '=')		// Internal symbol?
	    {				// Yes
                if (!(sym->sy_flag & SYF_EXPORT)) // Also exported?
		    sym->sy_flag |= SYF_INTERN; // No - make it internal
		chr = nxtnbc();
	    }
	    if (chr == '!')		// Suppressed symbol?
		sym->sy_flag |= SYF_SUP;// Yes
	    else
		hldchar = chr;		// No
	    exprsn();			// Get value of expression
	    if (!chkend(stopper))	// Must have end next
		return;
	    if (val.vl_kind == VK_EXT || val.vl_kind == VK_SLX) // External?
	    {
                errorx();		// Yes - error
		return;
	    }
            *blbpnt.c++ = LT_LONG;	// Put value in listing
            *blbpnt.c++ = (val.vl_kind == VK_OFS)? 1: 0;
            *blbpnt.l++ = val.vl_val.vl_vl;
            ++blbcnt;
            if (val.vl_kind == VK_OFS)	// If address, put selector in
            {				//   listing too
                *blbpnt.c++ = LT_WORD;
                *blbpnt.c++ = 1;
                *blbpnt.s++ = 0;
                ++blbcnt;
            }
            else if (val.vl_kind == VK_ABA)
            {
                *blbpnt.c++ = LT_WORD;
                *blbpnt.c++ = 0;
                *blbpnt.s++ = val.vl_psect;
                ++blbcnt;
            }
	    sym->sy_flag &= ~(SYF_UNDEF|SYF_REG|SYF_ABS|SYF_ADDR);
	    if (errflag & ERR_U)	// Was anything undefined?
		sym->sy_flag |= SYF_UNDEF; // Yes - indicate this symbol is
					   //   also undefined
	    sym->sy_val.val = val.vl_val.vl_vl; // Store value
	    sym->sy_psect = val.vl_psect;
            if (val.vl_kind == VK_ABS)	// Absolute value?
                sym->sy_flag |= SYF_ABS; // Yes
            else if (val.vl_kind == VK_ABA) // Absolute address?
                sym->sy_flag |= SYF_ABS|SYF_ADDR; // Yes
            else if (val.vl_kind == VK_OFS) // Address?
                sym->sy_flag |= SYF_ADDR; // Yes
	    else if (val.vl_kind == VK_REG) // Register value?
	    {
                if (sym->sy_flag & SYF_INTERN) // Yes - cannot be internal
		    errorx();
		else
		    sym->sy_flag |= SYF_REG;
	    }
            else
		if (val.vl_type != VL_VAL)// Normal value?
		    errorx();		// No - error
	    return;
	}

	// Here if symbol not followed by "="

	if (!spastop)			// Stopped by space?
	    persym();			// No - collect rest of symbol if it
				 	//   contained a period(s)
	hldchar = stopper;		// Put back the stopper
        cpnt = symbuf.nsym;		// Convert to all upper case
        cnt = symsize;
        do
        {   *cpnt = toupper(*cpnt);
            ++cpnt;
        } while (--cnt > 0);

        if (symbuf.nsym[symsize-1] == '%') // Want auto-size?
            symbuf.nsym[symsize-1] = (cursgd && !(segatrb & SA_32BIT))?
                    'W': 'L';
	symbuf.nsym[0] |= 0x80;		// See if this is a macro name
	if ((sym = looksym()) != NULL)
	{
            invmac(sym);		// Yes - invoke the macro
	    return;
	}
	symbuf.nsym[0] &= ~0x80;	// Not macro
	if (symbuf.nsym[0] == '.')	// Might this be a pseudo-op?
	{   if ((symsize <= 7) && (tbl.p=(struct pstbl *)srchtbl(symbuf.nsym+1,
                         psutbl, psusize, 6, psuents)) != NULL)
		(*tbl.p->pt_dsp)();	// Dispatch to pseudo-op routine
	    else
	    {
                badopcode();
		return;
	    }
	}
	else				// See if this is an opcode
	{
            if ((symsize<=7) && symbuf.nsym[0] >= 'A' && symbuf.nsym[0] <= 'Z'
                   && (tbl.o=(struct optbl *)srchtbl(&(symbuf.nsym[1]),
                   (opcpnt-'A')[symbuf.nsym[0]], (opcsize-'A')[symbuf.nsym[0]],
                   6, sizeof(struct optbl))) != NULL && (pbits&tbl.o->ot_proc))
            {
                chkpsect();		// Make sure have psect
		(*tbl.o->ot_dsp)(tbl.o); // Dispatch to opcode routine
            }
	    else
	    {
                badopcode();
		return;			// All finished with this line
	    }
	}
	chkend(stopper);		// Must be at end of line now
	return;
    }
}

//************************************
// Function: queerr - Indicate Q error
// Returned: FALSE
//************************************

int queerr(void)

{
    skiprest();
    errorq();
    return (FALSE);
}

//****************************************
// Function: badopcode - Report bad opcode
// Returned: Nothing
//****************************************

void badopcode(void)

{
    skiprest();				// Skip rest of line
    errsub(erromsg, ERR_O);		// Indicate bad opcode
}

//******************************************************************
// Function: chkend - Check for end of line, indicate Q error if not
// Returned: TRUE if end is next, FALSE if not end
//******************************************************************

int chkendx(void)

{
    return (chkend(nxtnbc()));
}

//******************************************************************
// Function: chkend - Check for end of line, indicate Q error if not
// Returned: TRUE if end is next, FALSE if not end
//******************************************************************

int chkend(
    char chr)

{
    if (ifend(chr))
	return (TRUE);
    return (queerr());			// Not end - indicate Q error
}

//************************************************
// Function: ifend - See if end of line is next
// Returned: TRUE if end is next, FALSE if not end
//************************************************

int ifend(
    char chr)

{
    chr = nxtnb0(chr);			// Make sure have non-blank character
    if (chr == '\0')			// If real end of line
	return (TRUE);			// Finished
    while (chr == '}')			// Skip any right braces
	chr = nxtnbc();
    if (chr == ';')			// Comment next?
    {
        skiprest();			// Yes - eat the rest of the line
	return (TRUE);
    }
    else
        return (FALSE);			// Not end - return FALSE
}

//*******************************************
// Function: skiprest - Skip rest of the line
// Returned: Nothing
//*******************************************

void skiprest(void)

{
    while (!eolflg)			// Eat the rest of the line
	nxtnbc();
    stopper = '\0';
}

//****************************************************************
// Function: findsym - Search symbol table for reference to symbol
// Returned: Address of symbol table entry
//****************************************************************

struct sy *findsym(
    int def)				// TRUE if defining reference

{
    register struct sy *sym;

    if ((sym=looksym()) == 0)		// Search symbol table
    {   sym = entersym();		// Not there - put it in the table
	sym->sy_flag = SYF_UNDEF;	// Indicate undefined
	sym->sy_val.val = 0;		// Initialize value
	sym->sy_psect = 0;
	sym->sy_ref = 0;		// No reference list now
        sym->sy_type = SYT_SYM;
    }
    if (!prmflg)			// Is this a parameter file?
    {
	sym->sy_flag |= SYF_USED;	// No - indicate symbol has been used
	refsym(sym,def);		// Make entry in reference list if
					//   necessary
    }
    return (sym);
}
