//**********************************************************
// Functions to process macro and repeat pseudo-ops for XMAC
//**********************************************************
// Written by John Goltz
//**********************************************************

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

struct ab *getargs();

// Allocate local data

struct ab   *abpnt;		// Pointer to argument block
struct body *bodybgn;		// Pointer to beginning of first body block
union				// Pointer to body block
{   struct body *bp;
    struct body **bpp;
    char  *cp;
} bodypnt;
int    bodycnt;			// Number of bytes left in body block

//********************************
// Function: invmac - Invoke macro
// Returned: Nothing
//********************************

void invmac(
    struct sy *sym)

{
    struct ab *args;

    adrflg = TRUE;			// List address on this line
    if (!prmflg)
        sym->sy_flag |= SYF_USED;
    refsym(sym, TRUE);			// Mark symbol as referenced
    args = getargs();			// Process argument list
    if (chkend(stopper))
    {
        insbody (ICT_MAC, sym->sy_val.body, args);
        inspnt->ic_gac = 0;		// Reset generated argument count
    }
}

//*******************************************
// Function: pmacro - Handle .MACRO pseudo-op
// Returned: Nothing
//*******************************************

void pmacro(void)

{
    struct sy *macsym;	 	// Pointer to symbol table entry for macro
    char  *cpnt;
    int    cnt;
    char   chr;			// Temp character
    char   genprm;		// TRUE for generated parameter

    getconcat();			// Get concatination character
    getsym(nxtnbc());			// Collect name of macro
    while (stopper == '.')
        persym();
    cpnt = symbuf.nsym;			// Convert macro name to upper case
    cnt = symsize;
    do
    { *cpnt = toupper(*cpnt);
      ++cpnt;
    } while (--cnt > 0);
    symbuf.nsym[0] |= 0x80;		// Indicate macro name
    macsym = findsym(TRUE);		// Search symbol table
    macsym->sy_flag = 0;
    if (!prmflg)			// If not in parameter file
        macsym->sy_flag |= SYF_USED;	// Indicate used
    if (macsym->sy_val.ab)		// If have body list
        givlist(macsym->sy_val.ab);	// Give it up
    abpnt = (struct ab *)(&alspnt);
    stopper = nxtnb0(stopper);
    if (!eolflg && stopper != ';')	// Do we have any arguments?
    {					// Yes
        hldchar = stopper;
        do
        {   if ((chr=nxtnbc()) == '=')	// Generated parameter?
            {
                genprm = TRUE;		// Yes
                chr = nxtchar();
            }
            else
                genprm = FALSE;		// No
            getdmy(chr, genprm);	// Collect dummy parameter
        } while (stopper == ',');
    }
    chkend(stopper);			// Should be at end of line here
    abpnt->ab_next = 0;			// Clear final argument link
    macbody(CC_IMA, CC_EMB, macnest, 'M');
    macsym->sy_val.body = bodybgn;
}

//***************************************
// Function: pirp - Handle .IRP pseudo-op
// Returned: Nothing
//***************************************

void pirp(void)

{
    struct ab *args;

    irpdmy();				// Get dummy parameter
    args = getargs();			// Process argument list
    chkend(stopper);
    macbody(CC_IIA, CC_EIB, reptnest, 'R'); // Store the body
    if (errflag == 0)
    {
        if (args)			// Have any arguments?
            insbody(ICT_IRP, bodybgn, args); // Yes - insert body now
        else
            givlist((struct ab *)bodybgn); // No - just throw it away!
    }
}

//*****************************************
// Function: pirpc - Handle .IRPC pseudo-op
// Returned: Nothing
//*****************************************

void pirpc(void)

{
    struct ab *arglst;
    char   chr;

    irpdmy();				// Get dummy parameter
    if ((chr=nxtnbc()) != '{')
    {
        queerr();
        return;
    }
    setbody();				// Setup list for arguments
    while (!eolflg && (chr=nxtchar()) != '}')
    {
        if (chr == '\\')
            chr = nxtchar();
        putbody(chr);			// Store character in list
    }
    if (eolflg)				// Error if stopped by end of line
    {
        givlist((struct ab *)bodybgn);
        queerr();
        return;
    }
    chkendx();				// Should be at end of line here
    stopper = nxtnbc();
    putbody(CC_EAG);			// Indicate end of argument list
    arglst = (struct ab *)bodybgn;	// Remember argument list
    macbody(CC_ICA, CC_ECB, reptnest, 'R'); // Store the body
    if (errflag == 0)
    {
        insbody(ICT_IRPC, bodybgn, arglst); // Setup to insert the body
        inspnt->ic_sbc = 0;		// Initialize argument list offset
    }
}

//*****************************************
// Function: prept - Handle .REPT pseudo-op
// Returned: Nothing
//*****************************************

void prept(void)

{
    getifx();				// Get repeat count
    chkend(stopper);			// Should have end of line next
    alspnt = NULL;			// .REPT never has any arguments
    catchr = 0;				// No concatination character
    macbody(0, CC_ERB, reptnest, 'R');	// Store repeat body
    if (errflag == 0)
        insbody(ICT_REPT, bodybgn, (struct ab *)(val.vl_val.vl_vl));
}					// Insert the body

//**************************************************************
// Function: irpdmy - Collect dummy parameter for .IRP and .IRPC
// Returned: Nothing
//**************************************************************

void irpdmy(void)

{
    char chr;

    getconcat();			// Get concatination character
    adrflg = TRUE;			// List address on this line
    abpnt = (struct ab *)(&alspnt);
    chr = nxtnbc();
    getdmy(chr, FALSE);			// Collect single dummy parameter
    abpnt->ab_next = 0;			// Clear argument link
    if (stopper != ',')			// Make sure stopped by comma
        queerr();
}

//******************************************************************
// Function: getconcat - Get concatination character if one is given
// Returned: Nothing
//******************************************************************

void getconcat(void)

{
    if (!chksys(stopper))		// Have concatination character?
    {
        hldchar = 0;			// Yes
        catchr = stopper;		// Use it
    }
    else
        catchr = '\'';			// No - use single quote
}

//**************************************************
// Function: getdmy - Collect single dummy parameter
// Returned: Nothing
//**************************************************

void getdmy(
    char chr,
    int  genflg)

{
    char  *pnt1;
    char  *pnt2;
    struct ab *bpnt;
    int    cnt;

    if (!getsym(chr))			// Collect parameter
    {
        if (abpnt != alspnt)		// Not there - any parameters at all?
            queerr();			// Yes - error
        chkend(stopper);
        return;
    }
    bpnt = (struct ab *)getblock();	// Get memory block
    abpnt->ab_next = bpnt;		// Link to previous block
    abpnt = bpnt;
    cnt = SYMMAXSZ;			// Copy dummy parameter to block
    pnt1 = bpnt->ab_data;
    pnt2 = symbuf.nsym;
    while (--cnt >= 0)
        *pnt1++ = *pnt2++;
    *pnt1 = genflg;
    stopper = nxtnb0(stopper);
}

//**********************************************************************
// Function: getargs - Process argument list for invoking macro and .IRP
// Returned: Nothing
//**********************************************************************

struct ab *getargs(void)

{
    struct ab *args;		// Pointer to first argument
    struct ab *curarg;		// Pointer to current argument
    char  *pnt1;
    int    cnt;
    char   chr;

    args = 0;
    curarg = (struct ab *)(&args);
    do
    {   if ((chr=nxtnbc()) == ';' || eolflg)// Another argument?
            break;			// No
        setbody();			// Yes - setup body list for arg
        curarg->ab_next = (struct ab *)bodybgn;// Link to previous list
        curarg = (struct ab *)bodybgn;
        bodypnt.cp += 4;		// Allow space for pointer
        bodycnt -= 4;
        curarg->ab_next = 0;		// Clear pointer
        if (chr == '{')			// Delimited argument?
        {
            cnt = 0;
            for (;;)
            {   if ((chr = nxtchar()) == '\\')
                    chr = nxtchar();
                else if (chr == '{')
                    ++cnt;
                else if (chr == '}' && --cnt < 0)
                    break;
                else if (eolflg)	// If stopped by end of line
                {
                    queerr();
                    break;
                }
                putbody(chr);		// Store character in list
            }
            chr = nxtnbc();		// Get stopper
        }
        else if (chr == '>')		// Value argument?
        {
            exprsn();			// Yes - evaluate expression
            if ((iradix == 10) && (val.vl_val.vl_vl < 0))
            {
                val.vl_val.vl_vl = -val.vl_val.vl_vl;
                putbody('-');
            }
            pnt1 = digbuf;
            cnt = 0;
            do
            {   *pnt1++ = ((ulong)(val.vl_val.vl_vl)) % iradix;
                ++cnt;			// Store digit value
                val.vl_val.vl_vl = ((ulong)(val.vl_val.vl_vl)) / iradix;
            } while (val.vl_val.vl_vl != 0);
            while (--cnt >= 0)
            {
                chr = *(--pnt1);
                if (chr >= 10)
                    chr += 'A' - '0' - 10;
                chr += '0';		// Change to ASCII
                putbody(chr);		// Store in list
            }
            chr = nxtnb0(stopper);
        }
        else				// Otherwise normal argument
        {
            while (!eolflg && chr != ',' && chr != ';')
            {
                putbody(chr);		// Store character in list
                chr = nxtnbc();
            }
        }
        putbody(CC_EAG);		// Indicate end of argument
    } while (chr == ',');
    stopper = nxtnb0(chr);
    return (args);
}

//***********************************************************
// Function: macbody - Store body for .MACRO, .IRP, and .IRPC
// Returned: Nothing
//***********************************************************

void macbody(
    char  argval,
    char  endval,
    int (*nestfnc)(void),
    char  endchr)

{
    struct ab   *apnt;
    struct body *savepnt;
    char  *pnt1;
    int    savecnt;
    int    nestcnt;
    int    cnt;
    char   chr;
    char   chknest;

    listline();				// List previous line
    setbody();				// Initialize body list
    chr = nxtchar();
    savecnt = bodycnt;
    savepnt = bodypnt.bp;
    chknest = TRUE;
    nestcnt = 0;
    while (!eofflg)
    {
        while (!eofflg && chr != '.' && !chksys(chr))
        {				// Can this start a symbol?
            if (eolflg)			// No - at end of line now?
            {
                listline();		// Yes - list this line and advance
                savecnt = bodycnt;	// Remember where we are in case line
                savepnt = bodypnt.bp;	//   ends the body
                chknest = TRUE;		// Indicate at beginning of line
            }
            if (!catchr || chr != catchr) // Concatination character?
            {
                putbody(chr);		// No - must store character in body
                chr = nxtchar();	// Get next character
            }
            else
            {
                while ((chr=nxtchar()) == catchr) // Yes - strip it off but
                    putbody(chr);		  //   store any following
            }					  //   concatination chrs
        }
        if (eofflg)			// Stopped by end of file?
        {
            errsub(errummsg, ERR_UM);	// Yes - report the error
            break;			// Terminate definition
        }
        symsize = 0;			// Clear symbol size
        for (pnt1 = symbuf.nsym, cnt = SYMMAXSZ; --cnt >= 0;)
            *pnt1++ = ' ';		// Clear symbuf
        pnt1 = symbuf.nsym;
        do				// Collect symbol
        {   ++symsize;
            *pnt1++ = chr;		// Store character in symbol
            chr = nxtchar();		// Get next character
        } while (symsize <= SYMMAXSZ && chksym(chr));
        if (!chkarg(argval, chr))	// See if symbol is an argument
        {
            if (chknest && chr != '\'' && chr != ':')
            {				// Should we check for nesting?
                if ((*nestfnc)())	// Yes - is this a nesting pseudo-op?
                    ++nestcnt;		// Yes - increase nesting level
                else if (symsize == 5 && symbuf.nsym[0] == '.' &&
                        (symbuf.nsym[1] == 'e' || symbuf.nsym[1] == 'E') &&
                        (symbuf.nsym[2] == 'n' || symbuf.nsym[2] == 'N') &&
                        (symbuf.nsym[3] == 'd' || symbuf.nsym[3] == 'D') &&
                        (symbuf.nsym[4] == endchr ||
                        symbuf.nsym[4] == endchr+0x20))
                {			// Is this the terminating pseudo-op?
                    if (nestcnt)	// Yes - anything nested?
                         --nestcnt;	// Yes - reduce nesting level
                    else
                        break;
                }
                chknest = FALSE;
            }
            pnt1 = symbuf.nsym;
            do
            {   putbody(*pnt1++);	// Copy symbol to body
            } while (--symsize > 0);
            while (chksym(chr))
            {				// If anything left over, copy it
                putbody(chr);		//   to the body too
                chr = nxtchar();
            }
        }
    }
    chkend(chr);			// No - end of line should be next
    bodypnt.bp = savepnt;		// Discard last line
    bodycnt = savecnt;
    savepnt = (struct body *)(bodypnt.cp - (BLKSIZE - 4 - savecnt));
    givlist((struct ab *)(savepnt->bb_link));
    savepnt->bb_link = NULL;
    while (alspnt)			// Give up the parameter blocks
    {
        apnt = alspnt->ab_next;
        givblock((struct fb *)alspnt);
        alspnt = apnt;
    }
    stopper = chr;
    putbody('\0');			// No - put end of line in body
    putbody(endval);			// Store final byte in body
}

//*********************************************************************
// Function: macnest - Check for nesting pseudo-op for .MACRO pseudo-op
// Returned: Nothing
//*********************************************************************

int macnest(void)

{
    return (symsize == 6 && symbuf.nsym[0] == '.' &&
            (symbuf.nsym[1] == 'm' || symbuf.nsym[1] == 'M') &&
            (symbuf.nsym[2] == 'a' || symbuf.nsym[2] == 'A') &&
            (symbuf.nsym[3] == 'c' || symbuf.nsym[3] == 'C') &&
            (symbuf.nsym[4] == 'r' || symbuf.nsym[4] == 'R') &&
            (symbuf.nsym[5] == 'o' || symbuf.nsym[5] == 'O'));
}

//************************************************************
// Function: reptnest - Check for nesting pseudo-op for .REPT,
//	.IRP, and .IRPC pseudo-ops
// Returned: TRUE if nested, FALSE otherwise
//************************************************************

int reptnest(void)

{
    if (symbuf.nsym[0] != '.')
        return (FALSE);
    if (symsize == 4 &&
            (symbuf.nsym[1] == 'i' || symbuf.nsym[1] == 'I') &&
            (symbuf.nsym[2] == 'r' || symbuf.nsym[2] == 'R') &&
            (symbuf.nsym[3] == 'p' || symbuf.nsym[3] == 'P'))
        return (TRUE);
    if (symsize != 5)
        return (FALSE);
    if ((symbuf.nsym[1] == 'i' || symbuf.nsym[1] == 'I') &&
            (symbuf.nsym[2] == 'r' || symbuf.nsym[2] == 'R') &&
            (symbuf.nsym[3] == 'p' || symbuf.nsym[3] == 'P') &&
            (symbuf.nsym[4] == 'c' || symbuf.nsym[4] == 'C'))
        return (TRUE);
    if ((symbuf.nsym[1] == 'r' || symbuf.nsym[1] == 'R') &&
            (symbuf.nsym[2] == 'e' || symbuf.nsym[2] == 'E') &&
            (symbuf.nsym[3] == 'p' || symbuf.nsym[3] == 'P') &&
            (symbuf.nsym[4] == 't' || symbuf.nsym[4] == 'T'))
        return (TRUE);
    return (FALSE);
}

//***************************************************************************
// Function: chkarg - See if symbol is macro argument and process it if it is
// Returned: TRUE if macor argument, FALSE otherwise
//***************************************************************************

int chkarg(
    char argval,
    char chr)

{
    struct ab *apnt;
    char  *pnt1;
    char  *pnt2;
    int    cnt;
    char   argnum;

    apnt = alspnt;			// Got atom - point to first
    argnum = 1;				//   parameter
    while (apnt)
    {
        cnt = SYMMAXSZ;
        pnt1 = apnt->ab_data;
        pnt2 = symbuf.nsym;
        while (--cnt >= 0)
        {
            if (*pnt1++ != *pnt2++)
                break;
        }
        if (cnt < 0)			// Did we get a match?
            break;			// Yes
        else
        {
            apnt = apnt->ab_next;	// No - try next argument
            ++argnum;
        }
    }
    if (apnt)				// Is this an argument?
    {
        while (chksym(chr))
            chr = nxtchar();		// Yes - skip remainder of atom
        putbody(apnt->ab_data[SYMMAXSZ]?CC_IMX:argval);
        if ((unsigned char)argval == CC_IMA) // If storing macro body
            putbody(argnum);		// Store argument number in body
        return (TRUE);			// Indicate have processed argument
    }
    else
        return (FALSE);			// Indicate not argument
}

//**************************************
// Function: setbody - Setup insert body
// Returned: Nothing
//**************************************

void setbody(void)

{
    bodypnt.bp = bodybgn = (struct body *)getblock();
					// Get first memory block for body
    bodypnt.bp->bb_link = 0;		// Clear link in first block
    bodycnt = BLKSIZE - 4;		// Initialize count
}

//**********************************************
// Function: putbody - Store character into body
// Returned: Nothing
//**********************************************

void putbody(chr)
char chr;

{
    struct body *blk;

    if (--bodycnt < 0)			// Need another body block?
    {
        blk = (struct body *)getblock();// Yes - get another memory block
        *bodypnt.bpp = blk;		// Link it to the previous block
        bodypnt.bp = blk;
        blk->bb_link = 0;		// Clear link in new block
        bodycnt = BLKSIZE - 5;
    }
    *bodypnt.cp++ = chr;		// Store character in block
}

//***************************************
// Function: insbody - Insert insert body
// Returned: Nothing
//***************************************

void insbody(
    int    type,
    struct body *body,
    struct ab *argval)

{
    struct ic *icb;

    icb = (struct ic *)getblock();	// Get block for the ICB
    icb->ic_.arg = argval;
    icb->ic_cbp =
    icb->ic_body = (char *)body;
    icb->ic_cbc = BLKSIZE - 4;
    icb->ic_type = type;
    listline();				// List this line
    icb->ic_link = inspnt;		// Make this the current ICB
    inspnt = icb;
}
