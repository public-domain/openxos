//********************************************
// Functions for conditional assembly for XMAC
//********************************************
// Written by John Goltz
//********************************************

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

extern struct iftbl ifctbl[];	// Condition table
extern int    ifsize;		// Number of entries in condition table
extern int    ifents;
extern struct pstbl cptbl[];	// False pseudo-op table
extern int    cpsize;
extern int    cpents;

// Declare local data

char  ifsbfr[IFSTRSZ+2];	// String buffer

//***************************************
// Function: pift - Handle .IFT pseudo-op
// Returned: Nothing
//***************************************

void pift(void)

{
    if (incond())
    {
        if (!condtbl[condlvl])
            iffalse();
    }
}

//***************************************
// Function: piff - Handle .IFF pseudo-op
// Returned: Nothing
//***************************************

void piff(void)

{
    if (incond())
    {
        if (condtbl[condlvl])
            skipcond();			// Switch to false condition
    }
}

//*****************************************
// Function: pendc - Handle .ENDC pseudo-op
// Returned: Nothing
//*****************************************

void pendc(void)

{
    if (incond())
        --condlvl;
}

//*********************************************************
// Function: incond - Check that are processing conditional
// Returned: Nothing
//*********************************************************

int incond(void)

{
    if (condlvl == 0)
    {
        errsub(errcemsg, ERR_CE);
        skiprest();
        return (FALSE);
    }
    else
    {
        chkend(stopper);
        return (TRUE);
    }
}

//*************************************
// Function: pif - Handle .IF pseudo-op
// Returned: Nothing
//*************************************

void pif(void)

{
    struct iftbl *pnt;

    getsym(nxtnbc());			// Collect condition
    if (symsize <= 3)
        pnt = (struct iftbl *)srchtbl(symbuf.nsym, ifctbl, ifsize, 3, ifents);
    else
        pnt = NULL;
    if (pnt)
        (*pnt->if_dsp)();
    else
	queerr();
}

//*************************************
// Function: ifeq - Handle ".IF EQ exp"
// Returned: Nothing
//*************************************

void ifeq(void)

{
    if (getifx() == 0)
	iftrue();
    else
	iffalse();
}

//*************************************
// Function: ifne - Handle ".IF NE exp"
// Returned: Nothing
//*************************************

void ifne(void)

{
    if (getifx() != 0)
	iftrue();
    else
	iffalse();
}

//*************************************
// Function: iflt - Handle ".IF LT exp"
// Returned: Nothing
//*************************************

void iflt(void)

{
    if (getifx() < 0)
	iftrue();
    else
	iffalse();
}

//*************************************
// Function: ifle - Handle ".IF LE exp"
// Returned: Nothing
//*************************************

void ifle(void)

{
    if (getifx() <= 0)
	iftrue();
    else
	iffalse();
}

//*************************************
// Function: ifge - Handle ".IF GE exp"
// Returned: Nothing
//*************************************

void ifge(void)

{
    if (getifx() >= 0)
	iftrue();
    else
	iffalse();
}

//*************************************
// Function: ifgt - Handle ".IF GT exp"
// Returned: Nothing
//*************************************

void ifgt(void)

{
    if (getifx() > 0)
	iftrue();
    else
	iffalse();
}

//************************************************
// Function: ifidn - Handle ".IF IDN <str1><str2>"
// Returned: Nothing
//************************************************

void ifidn(void)

{
    if (cmpifs())
	iftrue();
    else
	iffalse();
}

//***********************************************
// Function: ifdif - Handle ".IF DIF <str1><str2>
// Returned: Nothing
//***********************************************

void ifdif(void)

{
    if (!cmpifs())
	iftrue();
    else
	iffalse();
}

//******************************************
// Function: ifdef - Handle ".IF DEF symbol"
// Returned: Nothing
//******************************************

void ifdef(void)

{
    struct sy *sym;

    getsym(stopper);			// Collect symbol
    if ((sym=looksym()) != NULL)
        sym->sy_flag |= SYF_USED;	// Indicate symbol has been used
    if (sym != NULL && (!(sym->sy_flag&SYF_UNDEF)))
	iftrue();
    else
	iffalse();
}

//******************************************
// Function: ifndf - Handle ".IF NDF symbol"
// Returned: Nothing
//******************************************

void ifndf(void)

{
    struct sy *sym;

    getsym(stopper);			// Collect symbol
    if ((sym=looksym()) != NULL)
        sym->sy_flag |= SYF_USED;	// Indicate symbol has been used
    if (sym == NULL || (sym->sy_flag&SYF_UNDEF))
	iftrue();
    else
	iffalse();
}

//*************************************
// Function: ifnb - Handle ".IF NB str"
// Returned: Nothing
//*************************************

void ifnb(void)

{
    if (!blkifs())
	iftrue();
    else
	iffalse();
}

//***********************************
// Function: ifb - Handle ".IF B str"
// Returned: Nothing
//***********************************

void ifb(void)

{
    if (blkifs())
	iftrue();
    else
	iffalse();
}

//*********************************
// Function: ifp1 - Handle ".IF P1"
// Returned: Nothing
//*********************************

void ifp1(void)

{
    if (!passtwo)
	iftrue();
    else
	iffalse();
}

//*********************************
// Function: ifp1 - Handle ".IF P2"
// Returned: Nothing
//*********************************

void ifp2(void)

{
    if (passtwo)
	iftrue();
    else
	iffalse();
}

//*******************************************************
// Function: getifx - Get value of conditional expression
// Returned: Nothing
//*******************************************************

int getifx(void)

{
    stopper = nxtnbc();			// Get value of expression without
    level2(&val);			//   suffix
    if (errflag || val.vl_kind != VK_ABS) // Errors or not absolute?
    {
        errsub(errvmsg, ERR_V);		// Yes - also give V error
	val.vl_val.vl_vl = 0;		// And set value to 0
    }
    if (eolflg || stopper == ';')	// At end of line now?
    {
        *blbpnt.c++ = LT_LONG;		// Yes - list value of conditional
        *blbpnt.c++ = val.vl_psect;	//   expression
        *blbpnt.l++ = val.vl_val.vl_vl;
        ++blbcnt;
    }
    return ((int)(val.vl_val.vl_vl));	// Return the value
}

//***************************************
// Function: cmpifs - Compare two strings
// Returned: Nothing
//***************************************

int cmpifs(void)

{
    char *pnt;
    int   cnt;
    char  chr;

    stopper = nxtnb0(stopper);
    if (stopper != '{')			// First string must be next
    {
        queerr();
        return (FALSE);
    }
    pnt = ifsbfr;			// Copy first string to buffer
    cnt = IFSTRSZ;
    for (;;)
    {
        if (eolflg || --cnt < 0)
        {
            queerr();
            return (FALSE);
        }
        if ((chr=nxtchar()) == '}')
            break;
        *pnt++ = chr;
    }
    *pnt = '\0';
    if (nxtnbc() != '{')		// Get start of second string
    {
        queerr();
        return (FALSE);
    }
    pnt = ifsbfr;			// Compare strings
    for (;;)
    {
        if ((chr=nxtchar()) == '}')
            break;
        if (*pnt++ != chr)
        {
            eatifs();			// Different - eat rest of string
            return (FALSE);
        }
    }
    stopper = nxtnbc();			// Same - get stopper character
    if (*pnt == '\0')
        return (TRUE);
    else
        return (FALSE);
}

//*********************************************
// Function: blkifs - to check for blank string
// Returned: Nothing
//*********************************************

int blkifs(void)

{
    stopper = nxtnb0(stopper);
    if (stopper != '{')
    {
	queerr();
        stopper = '\0';
	return (FALSE);
    }
    if (nxtchar() == '}')		// Null string?
    {
        stopper = nxtnbc();
	return (TRUE);			// Yes
    }
    else
    {
	eatifs();			// No
	return (FALSE);
    }
}

//*************************************************
// Function: iftrue - Handle true condition for .IF
// Returned: Nothing
//*************************************************

void iftrue(void)

{
    if (!eolflg && stopper != ';')	// Single line conditional?
    {
        hldchar = stopper;
        asmline();			// Yes - assemble the line
    }
    else
    {
        if (condlvl >= CONDMAX)
            errsub(errcxmsg, ERR_CX);
        else
            condtbl[++condlvl] = TRUE;	// No - bump conditional level
    }
}

//***************************************************
// Function: iffalse - Handle false condition for .IF
// Returned: Nothing
//***************************************************

void iffalse(void)

{
    if (!eolflg && stopper != ';')	// Single line conditional?
        skiprest();			// Yes - skip the line
    else
    {
        if (condlvl >= CONDMAX)
            errsub(errcxmsg, ERR_CX);
        else
            condtbl[++condlvl] = FALSE;	// No - bump conditional level
        skipcond();
    }
}

//*******************************************
// Function: skipcond -Skip conditional stuff
// Returned: Nothing
//*******************************************

void skipcond(void)

{
    struct cptbl *tbl;

    do
    {
        listline();
        if (getsyq() && stopper != ':' && stopper != '='  &&
                symbuf.nsym[0] == '.' && symsize <= 5 &&
                (tbl=(struct cptbl *)srchtbl(symbuf.nsym+1, cptbl,
                cpsize, 6, cpents)) != NULL)
        {
            if ((*tbl->pt_dsp)())
                break;
        }
        else
            skiprest();
    } while (!eofflg);
}

//***********************************************************************
// Function: cpif - Handle .IF pseudo-op while processing false condition
// Returned: FALSE
//***********************************************************************

int cpif(void)

{
    skiprest();
    falselvl++;				// Indicate another level
    return (FALSE);
}

//***************************************************************************
// Function: cpiftf - Handle .IFTF pseudo-op while processing false condition
// Returned: TRUE if currently FALSE, FALSE if currently TRUE
//***************************************************************************

int cpiftf(void)

{
    chkend(stopper);
    return (falselvl == 0);
}

//*************************************************************************
// Function: cpiff - Handle .IFF pseudo-op while processing false condition
// Returned: ????
//*************************************************************************

int cpiff(void)

{
    chkend(stopper);
    return ((falselvl == 0)? !condtbl[condlvl]: FALSE);
}

//*************************************************************************
// Function: cpift - Handle .IFT pseudo-op while processing false condition
// Returned: ????
//*************************************************************************

int cpift(void)

{
    chkend(stopper);
    return ((falselvl == 0)? condtbl[condlvl]: FALSE);
}

//***************************************************************************
// Function: cpendc - Handle .ENDC pseudo-op while processing false condition
// Returned: ????
//***************************************************************************

int cpendc(void)

{
    chkend(stopper);
    if (falselvl == 0)
    {
        --condlvl;
        return (TRUE);			// This terminates the conditional
    }
    else
    {
        --falselvl;
        return (FALSE);
    }
}

//*******************************************
// Function: eatifs - Eat remainder of string
// Returned: Nothing
//*******************************************

void eatifs(void)

{
    while (!eolflg && nxtchar() != '}')
        ;
    stopper = nxtnbc();
}
