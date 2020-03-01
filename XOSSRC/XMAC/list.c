//**************************
// Listing routines for XMAC
//**************************
// Written by John Goltz
//**************************

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
#include <TIME.H>
#include <CTYPE.H>
#include <ERRNO.H>
#include <XOS.H>
#include <XOSTIME.H>
#include <XOSSVC.H>
#include <XOSERRMSG.H>
#include "XMAC.H"
#include "XMACEXT.H"

//***************************************************
// Function: listinit - Initialize for listing output
// Returned: Nothing
//***************************************************

extern int fisctrm(FILE *);

void listinit(void)

{
    time_d timeloc;

    if (lstfsp)				// Do we want listing output?
    {
        svcSysDateTime(2, &timeloc);	// Get current data and time
        ddt2str(timebfr, "%H:%m %W %D-%3n-%y", (time_dz *)&timeloc);
					// Change to string
        if (*lstfsp == '\0')
            lstfp = stdout;
        else
        {
	    if ((lstfp = fopen(lstfsp, "w")) == NULL)
		femsg(prgname, -errno, lstfsp);
        }
        lststd = fisctrm(lstfp);	// Remember if listing is to console
    }
    albcnt = 0;				// Nothing to list now
    blbcnt = 0;
    lincnt = 0;
}

//************************************
// Function: nextpage - Start new page
// Returned: Nothing
//************************************

void nextpage(void)

{
    if (passtwo || listtype == LT_DEBUG) // If second pass
    {
	lincnt = 0;			// Force new page
	pagcnm = 0;			// Reset continuation number
    }
    lstseq = 1;				// Reset sequence number
    ++pagnum;				// Bump page number
    ++lsbnum;				// Bump local symbol block number
}

//***********************************
// Function: listline - List one line
// Returned: Nothing
//***********************************

void listline(void)

{
    char *pnt;
    int   cnt1;
    int   cnt2;
    int   seqsize;

    eolflg = FALSE;			// Advance to next line
    hldchar = '\0';
    if (!blbcnt && !albcnt)		// Anything to list now?
    {
	resetlst();			// No
	return;
    }
    if ((errflag & (ERR_V|ERR_NP|ERR_XX))
            || (errflag && (passtwo || listtype == LT_DEBUG || prmflg))
	    || ((passtwo || listtype == LT_DEBUG) && lstfp && (nlscnt >= 0)
                && (!lstinp || !inspnt || lstmode == 2
                    || (lstmode != -1 && bnoflg))))
    {
        if (sleflg)
        {
            seqsize = (lstseq <= 99)? 2: (lstseq <= 999)? 3: 4;
            cnt2 = 6 - seqsize;
            pnt = errltrs;
            if (errcnt)
            {
                errpnt = errtbl;
                cnt1 = errcnt;
                if (cnt1 > cnt2)
                    cnt1 = cnt2;
                cnt2 -= cnt1;
                do
                {
                    *pnt++ = *(*errpnt++);
                } while (--cnt1 > 0);
            }
            while (--cnt2 >= 0)
                *pnt++ = ' ';
            *pnt = '\0';
        }
        if (lstfp)
        {
            chkhead();
            if (sleflg)
                fprintf(lstfp, "%s%*ld%c", errltrs, seqsize, lstseq,
                        srclvl? '+':' ');
            else
                fprintf(lstfp, "%5ld%c ", lstseq, (srclvl)? '+':' ');
        }
        if (errflag && !lststd)
        {
            if (sleflg)
                fprintf(stderr, "%s%*ld%c", errltrs, seqsize, lstseq,
                        srclvl? '+':' ');
            else
                fprintf(stderr, "%5ld%c ", lstseq, (srclvl)? '+':' ');
        }
	if (adrflg)			// Should we list the address?
	    listadr();			// Yes - output address
	else
	    liststr("          ");	// No
	blbpnt.c = blbbuf;		// Point to start of binary data
	if (blbcnt == 0)		// Any binary to list?
            lstpos = 17;
        else
            listbin();			// Yes - list as much as can here
	if ((albcnt > 1) && (errflag || lstmode != 1 || (!lstinp || !inspnt)))
	{				// List alpha part of line if need to
            lstpos &= ~0x7;
            while (lstpos < 48)
            {
                listchr('\t');
                lstpos += 8;
            }
	    liststr(albbuf);
	}
	listnxl();			// End the line
	if (!bexflg)			// Should we list extra binary data?
	{
	    while (blbcnt > 0)		// Yes
	    {
		liststr("       ");
		listadr();		// List address
                listbin();		// List as much binary data as will
					//   fit
                listnxl();		// End the line
	    }
	}
	if (errflag)			// If have some errors
	{
	    if (errflag & ~ERR_XX)	// Only if real error
            {
                if (!lststd)
                {
                    fprintf(stderr, "*****%c Error on page %d ",
                            (sleflg)? '*': ' ', pagnum);
                    if (lineno == -1)
                        fputs("(end of source)", stderr);

                    else
                    {
                        fprintf(stderr, "(line %ld in file %s)", lineno,
                                srclvl ? srcfsp[srclvl]: thissrc->sb_fsp);
                    }
                }
                if (lstfp)
                {
                    chkhead();
                    fprintf(lstfp, "*****%c Error on page %d ",
                            (sleflg)? '*': ' ',pagnum);
                    if (lineno == -1)
                        fputs("(end of source)", lstfp);
                    else
                    {
                        fprintf(lstfp, "(line %ld in file %s)", lineno,
                                srclvl ? srcfsp[srclvl]: thissrc->sb_fsp);
                    }
                    listnxl();
                }
                else
                    putchar('\n');
	    }
            totalerr += errcnt;

            if (!sleflg)
            {
                errpnt = errtbl;	// List all error messages for this
                while (--errcnt >= 0)	//   line
                {
                    liststr("*****  ");
                    liststr(*errpnt++ + 1);
                    listnxl();
                }
            }
	}
    }
    errflag = 0;			// Clear error bits
    if (!prmflg)			// From parameter file?
	++lstseq;			// No - bump listing sequence number
    resetlst();
}

//************************************************
// Function: listtoc - List table of contents line
// Returned: Nothing
//************************************************

void listtoc(void)

{
    if (lstfp && tocflg)		// Do we need output here?
    {
	if (!tchflg)			// Have we output the heading yet?
	{
	    tchflg = TRUE;		// No - do it now
            chkhead();
	    fputs("***  Table of contents  ***", lstfp);
	    listnxl();
	    listnxl();
	    fputs("  Page-Line     Sub-title", lstfp);
	    listnxl();
	}
        chkhead();
	fprintf(lstfp, "%6d-%-6ld  %s", pagnum, lstseq, hdrstb);
	listnxl();
    }
}

//************************************************
// Function: listbin - Binary part of listing line
// Returned: Nothing
//************************************************

void listbin(void)

{
    genloc value;
    long   reloc;

    lstpos = 17;
    do
    {
        switch (*blbpnt.c)
        {
        case LT_BYTE:
            if (lstpos > 43)
                return;
            ++blbpnt.c;
            lstpos += 3;
            reloc = *blbpnt.c++;	// Get relocation value
            lstadr += 1;		// Bump address for listing
            value.s = (unsigned char)*blbpnt.s++; // Get value
            if (errflag && !lststd)
                fprintf(stderr, "%2.2X", value.s);
            if (lstfp)
                fprintf(lstfp, "%2.2X", value.s);
            listrlc(reloc);		// Output relocation
            break;
        case LT_WORD:
            if (lstpos > 42)
                return;
            ++blbpnt.c;
            lstpos += 5;
            reloc = *blbpnt.c++;	// Get relocation value
            lstadr += 2;		// Bump address for listing
            value.s = *blbpnt.s++;	// Get value
            if (errflag && !lststd)
                fprintf(stderr, "%04.4X", (ushort)(value.s));
            if (lstfp)
                fprintf(lstfp, "%04.4X", (ushort)(value.s));
            listrlc(reloc);		// Output relocation
            break;
        case LT_LONG:
            if (lstpos > 38)
                return;
            ++blbpnt.c;
            lstpos += 9;
            reloc = *blbpnt.c++;	// Get relocation value
            lstadr += 4;		// Bump address for listing
            value.l = *blbpnt.l++;
            if (errflag && !lststd)
                fprintf(stderr, "%08.8X", value.l);
            if (lstfp)
                fprintf(lstfp, "%08.8X", value.l);
            listrlc(reloc);		// Output relocation
            break;
        }
    } while (--blbcnt > 0);
}

//*************************************************
// Function: listrlc - Output relocation indication
// Returned: Nothing
//*************************************************

void listrlc(reloc)
unsigned char reloc;

{
    listchr((reloc==0)? ' ' :(reloc==0xFF)? '*' :(reloc==0xFE)? '!' :'\'');
}

//*******************************************************
// Function: resetlst - Reset listing pointers and counts
// Returned: Nothing
//*******************************************************

void resetlst(void)

{
    albcnt = 0;
    blbcnt = 0;
    albpnt = albbuf;
    blbpnt.c = blbbuf;
    adrflg = FALSE;
    bnoflg = FALSE;
    if (curpsd)
    {
        lstadr = curpsd->pd_ofs;
        lstpsn = curpsd->pd_psn;
    }
    else
    {
        lstadr = 0;
        lstpsn = 0;
    }
    lstinp = inspnt;
    errflag = 0;
    errcnt = 0;
    errpnt = errtbl;
}

//*********************************
// Function: listadr - List address
// Returned: Nothing
//*********************************

void listadr(void)

{
    if (errflag && !lststd)
	fprintf(stderr, "%08.8X", lstadr);
    if (lstfp)
    {
	chkhead();
	fprintf(lstfp, "%08.8X", lstadr);
    }
    listrlc(lstpsn);
    listchr(' ');
}

//********************************
// Function: liststr - List string
// Returned: Nothing
//********************************

void liststr(str)
char *str;

{
    if (errflag && !lststd)
	fputs(str, stderr);
    if (lstfp)
    {
	chkhead();
	fputs(str, lstfp);
    }
}

//************************************
// Function: listchar - List character
// Returned: Nothing
//************************************

void listchr(chr)
char chr;

{
    if (errflag && !lststd)
	putc(chr,stderr);
    if (lstfp)
    {
	chkhead();				// See if need heading here
	putc(chr, lstfp);
    }
}

//***********************************************
// Function: listnxl - Start next line on listing
// Returned: Nothing
//***********************************************

void listnxl(void)

{
    if (errflag && !lststd)
        putc('\n', stderr);
    if (lstfp)
    {
        chkhead();				// See if need heading here
        putc('\n', lstfp);
        --lincnt;
    }
}


//*************************************************
// Function: chkhead - Output page header if needed
// Returned: Nothing
//*************************************************

void chkhead(void)

{
    if (lincnt <= 0)			// At top of page?
    {
	if (fhdflg)			// Is this the first header line?
	    putc('\f', lstfp);		// No - output form-feed first
	else
	    fhdflg = TRUE;		// Yes
	fprintf(lstfp, "\n\n\n\n\n%-65.65s %-24.24s   XMAC - version %d.%d   "
		"Page %d", hdrbfr, timebfr, version, editnum,
                (passtwo || listtype == LT_DEBUG)? pagnum: 0);
	if (pagcnm)
	    fprintf(lstfp, ".%d", pagcnm);
	++pagcnm;
	if (passtwo || listtype == LT_DEBUG)
	    fprintf(lstfp, "\n%s\n%s\n\n", thissrc?thissrc->sb_fsp:"", hdrstb);
	else
	    fputs("\n\n\n\n", lstfp);
	lincnt = LSTPGSZ;
    }
}
