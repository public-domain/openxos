//*****************************************
// Functions to process pseudo-ops for XMAC
//*****************************************
// Written by John Goltz
//*****************************************

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
#include <ERRNO.H>
#include <MALLOC.H>
#include <XOS.H>
#include <XOSERRMSG.H>
#include <XCMALLOC.H>
#include "XMAC.H"
#include "XMACEXT.H"

#define vv(f) (void (*)(void *))f

//********************************************
// Function: ptitle - Process .TITLE pseudo-op
// Returned: Nothing
//********************************************

void ptitle(void)

{
    char  *pnt1;
    char  *pnt2;
    int    cnt;

    if (ttlflg)				// Have we seen a title this pass?
    {
	errsub(errmmsg, ERR_M);		// Yes - error
	skiprest();
	return;
    }
    ttlflg = TRUE;
    if (!getsym(nxtnbc()))		// Collect program name
    {
	queerr();
	return;
    }
    for (cnt = SYMMAXSZ, pnt1 = symbuf.nsym, pnt2 = hdrbfr; --cnt >= 0;)
	*pnt2++ = *pnt1++;		// Copy program name
    copytext(pnt2, TITLESZ-SYMMAXSZ);
}

//********************************************
// Function: psbttl - Process .SBTTL pseudo-op
// Returned: Nothing
//********************************************

void psbttl(void)

{
    copytext(hdrstb, SBTTLSZ);		// Copy text to sub-title buffer
    if (!passtwo)			// If first pass,
	listtoc();			// Output table of contents if wanted
}

//************************************************
// Function: copytexT - copy text to header buffer
// Returned: Nothing
//************************************************

void copytext(
    char *buffer,
    int   size)

{
    char chr;

    while (!eolflg)			// Copy text part to header buffer
    {
	chr = nxtchar();
	if (chr == ';')
	{
	    skiprest();
	    break;
	}
	if (--size >= 0)
	    *buffer++ = (chr == '\t')? ' ': chr;
    }
    *buffer = '\0';
    stopper = '\0';
}

//********************************************
// Function: pprint - Process .PRINT pseudo-op
// Returned: Nothing
//********************************************

void pprint(void)

{
    errflag |= ERR_XX;			// Set non-error error bit!
    skiprest();				// Skip rest of the line
}

//********************************************
// Function: perror - Process .ERROR pseudo-op
// Returned: Nothing
//********************************************

void peerror(void)

{
    errsub(erremsg, ERR_E);		// Indicate E error
    skiprest();				// Skip rest of the line
}

//**********************************************
// Function: pinclud -  Handle .INCLUD pseudo-op
// Returned: Nothing
//**********************************************

void pinclud(void)

{
    char *pnt;
    int   cnt;
    char  chr;

    if (srclvl >= INCLMAX)		// Level too deep?
    {
	fprintf(stderr, "? XMAC: Include level too deep from file %s\n",
                srcfsp[INCLMAX]);
	exit (1);
    }
    pnt = srcfsp[srclvl+1];
    if (pnt == NULL)
    {
	pnt = getspace ((long)(INCLFSSZ+1));
        srcfsp[srclvl+1] = pnt;
    }
    cnt = INCLFSSZ;
    chr = nxtnbc ();
    while (!eolflg && chr != ';' && !isspace(chr))
    {
	if (--cnt < 0)
	{
	    fprintf(stderr,
                    "? XMAC: Include file specification \"%s\" too long\n",
	            srcfsp[srclvl+1]);
	    exit (1);
	}
	*pnt++ = chr;
	chr = nxtchar();
    }
    *pnt = '\0';
    stopper = chr;
    if (!ifend(chr))
    {
	fprintf(stderr,
                "? XMAC: Syntax error in include statement from file %s\n",
	        srcfsp[srclvl]);
	exit (1);
    }
    listline();				// List this line
    ++srclvl;
    srcfp[srclvl] = fopen(srcfsp[srclvl], "r"); // Open the file
    if (srcfp[srclvl] == NULL)
	femsg("XMAC(.INCLUD)", -errno, srcfsp[srclvl]); // If error
    savlineno[srclvl] = lineno;		// Save current input line number
    savlinesz[srclvl] = linesz;
    lineno = 1;				// Initialize line number for
}					//   this file

//**********************************************
// Function: prequir -  Handle .REQUIR pseudo-op
// Returned: Nothing
//**********************************************

void prequir(void)

{
    struct reqblk *reqpnt;
    char  *pnt;
    int    cnt;
    char   chr;
    char   buffer[204];

    pnt = buffer;
    cnt = 200;
    chr = nxtnbc();
    while (!eolflg && chr != ';' && !isspace(chr))
    {
	if (--cnt < 0)
	{
            errsub(errinmsg, ERR_IN);
            return;
	}
	*pnt++ = chr;
	chr = nxtchar();
    }
    *pnt = '\0';
    cnt = 200 - cnt;
    stopper = chr;
    if (!ifend(chr))
    {
        errsub(errinmsg, ERR_IN);
        return;
    }
    listline();				// List this line
    reqpnt = reqhead;			// See if we already have this one
    while (reqpnt != NULL)
    {
        if (cnt == reqpnt->size && strcmp(buffer, reqpnt->name) == 0)
            return;			// Have it - ignore this request
        reqpnt = reqpnt->next;
    }
    reqpnt = (struct reqblk *)getspace(cnt + offsetof(reqblk, name) + 2);
    strcpy(reqpnt->name, buffer);	// Don't have it - allocate a block
    reqpnt->size = cnt;			//   and store the request
    reqpnt->next = NULL;
    if (reqtail == NULL)
        reqhead = reqpnt;
    else
        reqtail->next = reqpnt;
    reqtail = reqpnt;
}

//********************************************
// Function: pradix - Process .RADIX pseudo-op
// Returned: Nothing
//********************************************

void pradix(void)

{
    iradix = 10;			// Always evalulate the expresssion
    exprsn();				//   using radix 10
    if (val.vl_kind != VK_ABS)
    {
        errorx();
	skiprest();
	return;
    }
    if (!chkend(stopper))		// Must have end next
	return;
    val.vl_val.vl_vl &= 0xF;
    if (val.vl_val.vl_vl == 0)		// If value is 0
	val.vl_val.vl_vl = 16;		// Use 16
    iradix = val.vl_val.vl_vl;		// Store as new default radix
    *blbpnt.c++ = LT_BYTE;		// List value as a byte
    *blbpnt.c++ = 0;
    *blbpnt.s++ = iradix;
    ++blbcnt;
}

//******************************************
// Function: ppage - Process .PAGE pseudo-op
// Returned: Nothing
//******************************************

void ppage(void)

{
    if (chkendx() && nlscnt >= 0)	// Must have end of line next
    {					//   and be listing now
        albcnt = 0;			// Don't list this line
	nextpage();			// Start new page
    }
}

//****************************************
// Function: plsb - Process .LSB pseudo-op
// Returned: Nothing
//****************************************

void plsb(void)

{
    if (chkendx())			// Must have end of line next
	++lsbnum;
}

//********************************************
// Function: pnlist - Process .NLIST pseudo-op
// Returned: Nothing
//********************************************

void pnlist(void)

{
    if (chkendx())
	--nlscnt;
}

//******************************************
// Function: plist - Process .LIST pseudo-op
// Returned: Nothing
//******************************************

void plist(void)

{
    if (chkendx() && nlscnt++ < 0)	// Are we listing now?
	albcnt = 0;			// No - don't list this line
}

//******************************************
// Function: pcref - Process .CREF pseudo-op
// Returned: Nothing
//******************************************

void pcref(void)

{
    if (chkendx() && listtype == LT_CREF)
	++crfcnt;
}

//********************************************
// Function: pncref - Process .NCREF pseudo-op
// Returned: Nothing
//********************************************

void pncref(void)

{
    if (chkendx() && listtype == LT_CREF)
	--crfcnt;
}

//********************************************
// Function: palmex - Process .ALMEX pseudo-op
// Returned: Nothing
//********************************************

void palmex(void)

{
    if (chkendx())
	lstmode = 2;
}

//********************************************
// Function: pslmex - Process .SLMEX pseudo-op
// Returned: Nothing
//********************************************

void pslmex(void)

{
    if (chkendx())
	lstmode = 0;
}

//********************************************
// Function: pblmex - Process .BLMEX pseudo-op
// Returned: Nothing
//********************************************

void pblmex(void)

{
    if (chkendx())
	lstmode = 1;
}

//********************************************
// Function: pnlmex - Process .NLMEX pseudo-op
// Returned: Nothing
//********************************************

void pnlmex(void)

{
    if (chkendx())
	lstmode = -1;
}

//******************************************
// Function: plbex - Process .LBEX pseudo-op
// Returned: Nothing
//******************************************

void plbex(void)

{
    if (chkendx())
	bexflg = FALSE;
}

//********************************************
// Function: pnlbex - Process .NLBEX pseudo-op
// Returned: Nothing
//********************************************

void pnlbex(void)

{
    if (chkendx())
	bexflg = TRUE;
}

//******************************************
// Function: plsym - Process .LSYM pseudo-op
// Returned: Nothing
//******************************************

void plsym(void)

{
    if (chkendx())
        lsymflg = TRUE;
}

//********************************************
// Function: pnlsym - Process .NLSYM pseudo-op
// Returned: Nothing
//********************************************

void pnlsym(void)

{
    if (chkendx())
        lsymflg = FALSE;
}

//********************************************
// Function: pngsym - Process .NGSYM pseudo-op
// Returned: Nothing
//********************************************

void pngsym(void)

{
    if (chkendx())
        gsymflg = FALSE;
}

//****************************************
// Function: pcot - Process .COT pseudo-op
// Returned: Nothing
//****************************************

void pcot(void)

{
    if (chkendx())
	tocflg = TRUE;
}

//******************************************
// Function: pncot - Process .NCOT pseudo-op
// Returned: Nothing
//******************************************

void pncot(void)

{
    if (chkendx())
	tocflg = FALSE;
}

//******************************************
// Function: pparm - Process .PARM pseudo-op
// Returned: Nothing
//******************************************

void pparm(void)

{
    struct sy **hshpnt;
    struct sy *sympnt;
    struct sy *predpnt;
    int    hshcnt;

    if (!chkendx())
	return;
    if (inspnt)				// Are we inserting?
    {
	badopcode();			// Yes - error
	return;
    }
    if (nopar)
	return;
    if (!passtwo)			// First pass?
    {
	prmflg = TRUE;			// Yes
	return;
    }
    hshpnt = hashtbl;			// Set to scan hash table
    hshcnt = 128;
    while (--hshcnt >= 0)
    {
	predpnt = (struct sy *)hshpnt;
	sympnt = *hshpnt++;
	while (sympnt)
	{
	    if (sympnt->sy_type == SYT_SYM &&
		    !(sympnt->sy_flag & (SYF_USED|SYF_UNDEF)))
	    {
		if (sympnt->sy_name[0] < 0) // Is this a macro name?
		    givlist(sympnt->sy_val.ab);
					// Yes - give up body list

		// CODE HERE TO GIVE UP REFERENCE LIST FOR SYMBOL!!

		predpnt->sy_hash = sympnt->sy_hash;
					// Remove from its hash list
		givblock((struct fb *)sympnt); // Give up symbol block
	    }
	    else
		predpnt = sympnt;
	    sympnt = predpnt->sy_hash;	// Advance pointer
	}
    }
    eofreq = TRUE;			// Force end of file
}

//*****************************************
// Attributes table for the .PROC pseudo-op
//*****************************************

struct atributes proctbl[] =
{   {{'8','0','1','8','6',' '}, vv(proc80186)},
    {{'8','0','2','8','6',' '}, vv(proc80286)},
    {{'8','0','3','8','6',' '}, vv(proc80386)},
    {{'8','0','4','8','6',' '}, vv(proc80486)},
    {{'8','0','8','6',' ',' '}, vv(proc8086)}
};
#define PRSIZE (sizeof(proctbl)/sizeof(struct atributes))

//******************************************
// Function: pproc - Process .PROC pseudo-op
// Returned: Nothing
//******************************************

void pproc(void)

{
    hndlatr(proctbl, PRSIZE, NULL);
    chkend(stopper);
}

//****************************************************************
// Function: proc8086 - Process 8086 attribute for .PROC pseudo-op
// Returned: Nothing
//****************************************************************

void proc8086(void)

{
    if (ptype && ptype != P_8086)
        errorc();
    else
    {
        ptype = P_8086;
        pbits = B_8086;
        stack32 = FALSE;
    }
}

//******************************************************************
// Function: proc80186 - Process 80186 attribute for .PROC pseudo-op
// Returned: Nothing
//******************************************************************

void proc80186(void)

{
    if (ptype && ptype != P_80186)
        errorc();
    else
    {
        ptype = P_80186;
        pbits = B_80186;
        stack32 = FALSE;
    }
}

//******************************************************************
// Function: proc80286 - Process 80286 attribute for .PROC pseudo-op
// Returned: Nothing
//******************************************************************

void proc80286(void)

{
    if (ptype && ptype != P_80286)
        errorc();
    else
    {
        ptype = P_80286;
        pbits = B_80286;
        stack32 = FALSE;
    }
}

//******************************************************************
// Function: proc80386 - Process 80386 attribute for .PROC pseudo-op
// Returned: Nothing
//******************************************************************

void proc80386(void)

{
    if (ptype && ptype != P_80386)
        errorc();
    else
    {
        ptype = P_80386;
        pbits = B_80386;
        stack32 = TRUE;
    }
}

//******************************************************************
// Function: proc80486 - Process 80486 attribute for .PROC pseudo-op
// Returned: Nothing
//******************************************************************

void proc80486(void)

{
    if (ptype && ptype != P_80486)
        errorc();
    else
    {
        ptype = P_80486;
        pbits = B_80486;
        stack32 = TRUE;
    }
}

//****************************************
// Attributes table for the .SEG pseudo-op
//****************************************

struct atributes satrtbl[] =
{   {{'1','6','B','I','T',' '}, vv(satr16b)},
    {{'3','2','B','I','T',' '}, vv(satr32b)},
    {{'A','D','D','R',' ',' '}, vv(satraddr)},
    {{'C','O','D','E',' ',' '}, vv(satrcode)},
    {{'C','O','M','B',' ',' '}, vv(satrcomb)},
    {{'C','O','N','F',' ',' '}, vv(satrconf)},
    {{'D','A','T','A',' ',' '}, vv(satrdata)},
    {{'E','X','T','E','N','D'}, vv(satrext)},
    {{'L','A','R','G','E',' '}, vv(satrlarge)},
    {{'M','O','D',' ',' ',' '}, vv(satrmod)},
    {{'P','R','I','V',' ',' '}, vv(satrpriv)},
    {{'R','E','A','D',' ',' '}, vv(satrread)},
    {{'S','E','L','E','C','T'}, vv(satrselect)},
    {{'S','T','A','C','K',' '}, vv(satrstack)},
    {{'W','R','I','T','E',' '}, vv(satrwrite)}
};
#define SASIZE (sizeof(satrtbl)/sizeof(struct atributes))

//****************************************
// Function: pseg - Process .SEG pseudo-op
// Returned: Nothing
//****************************************

void pseg(void)

{
    struct sn  *sname;
    struct sd  *sgd;
    long   addr;
    int    select;
    char   atrb;
    char   flag;

    if (ptype == 0)
        ptype = P_8086;
    if (!getsym(nxtnbc()))		// Collect segment name
    {
        errorq();
        return;
    }
    else				// collect rest of symbol if stopped
	persym();			//   here by a period
    if (isspace(stopper))
        stopper = nxtnbc();
    sname = (struct sn *)looksym();	// Look for it in the symbol table
    if (sname == NULL || (sname->sn_flag&SYF_UNDEF)) // If not defined
    {
        if (sgtail && sgtail->sd_sgn == 254)
        {				// Not there - room for another?
            fputs("? XMAC: Too many segments\n", stderr);
            exit(1);
        }
        if (sname == NULL)
            sname = (struct sn *)entersym(); // OK - put name block in symbol
	else				     //   table
            sname->sn_flag &= ~SYF_UNDEF;
        sname->sn_type = SYT_SEG;
        sgd = (struct sd *)getblock(); // Get block for segment data block
        sgd->sd_nba = sname;		// Initialize it
        sgd->sd_sgn = (sgtail)? sgtail->sd_sgn + 1: 0x201;
        sgd->sd_atrb = (ptype >= P_80386)? SA_32BIT: 0;
        sgd->sd_flag = 0;
        sgd->sd_priv = 0;
        sgd->sd_type = 0;
        sgd->sd_select = 0;
        sgd->sd_addr = -1;
        if (sgtail)
            sgtail->sd_next = sgd;
        else
            sghead = sgd;
        sgtail = sgd;
        sgd->sd_next = NULL;
        sname->sn_sgn = sgd->sd_sgn;
        sname->sn_flag = 0;
        sname->sn_dba = sgd;
        while (stopper == ',')
            hndlatr(satrtbl, SASIZE, (struct pd *)sgd);
        if (sgd->sd_type == 0)
            sgd->sd_type = ST_COMB;
    }
    else
    {
        sgd = sname->sn_dba;
        atrb = sgd->sd_atrb;
        flag = sgd->sd_flag;
        addr = sgd->sd_addr;
        select = sgd->sd_select;
        while (stopper == ',')
            hndlatr(satrtbl, SASIZE, (struct pd *)sgd);
        if (sgd->sd_atrb != atrb || sgd->sd_flag != flag ||
                sgd->sd_addr != addr || sgd->sd_select != select)
        {
            errorc();
            sgd->sd_atrb = atrb;
            sgd->sd_flag = flag;
            sgd->sd_addr = addr;
            sgd->sd_select = select;
        }
    }
    chkend(stopper);
}

//*****************************************************************
// Function: satrlarge - Process LARGE attribute for .SEG pseudo-op
// Returned: Nothing
//*****************************************************************

void satrlarge(
    struct sd *sgd)

{
    sgd->sd_atrb |= SA_LARGE;
}

//****************************************************************
// Function: satrext - Process EXTEND attribute for .SEG pseudo-op
// Returned: Nothing
//****************************************************************

void satrext(
    struct sd *sgd)

{
    sgd->sd_atrb |= SF_EXTEND;
}

//***************************************************************
// Function: satr16b - Process 16BIT attribute for .SEG pseudo-op
// Returned: Nothing
//***************************************************************

void satr16b(
    struct sd *sgd)

{
    sgd->sd_atrb &= ~SA_32BIT;
}

//*****************************************************************
// Function: satr32bit - Process 32bit attribute for .SEG pseudo-op
// Returned: Nothing
//*****************************************************************

void satr32b(
    struct sd *sgd)

{
    if (ptype >= P_80386)
        sgd->sd_atrb |= SA_32BIT;
    else
        queerr();
}

//***************************************************************
// Function: satrconf - Process CONF attribute for .SEG pseudo-op
// Returned: Nothing
//***************************************************************

void satrconf(
    struct sd *sgd)

{
    sgd->sd_atrb |= SA_CONF;
}

//*****************************************************************
// Function: satrwrite - Process WRITE attribute for .SEG pseudo-op
// Returned: Nothing
//*****************************************************************

void satrwrite(
    struct sd *sgd)

{
    sgd->sd_atrb |= SA_WRITE;
}

//***************************************************************
// Function: satrread - Process READ attribute for .SEG pseudo-op
// Returned: Nothing
//***************************************************************

void satrread(
    struct sd *sgd)

{
    sgd->sd_atrb |= SA_READ;
}

//***************************************************************
// Function: satrdata - Process DATA attribute for .SEG pseudo-op
// Returned: Nothing
//***************************************************************

void satrdata(
    struct sd *sgd)

{
    sgd->sd_type = ST_DATA;
}

//***************************************************************
// Function: satrcode - Process CODE attribute for .SEG pseudo-op
// Returned: Nothing
//***************************************************************

void satrcode(
    struct sd *sgd)

{
    sgd->sd_type = ST_CODE;
}

//*****************************************************************
// Function: satrstack - Process STACK attribute for .SEG pseudo-op
// Returned: Nothing
//*****************************************************************

void satrstack(
    struct sd *sgd)

{
    sgd->sd_type = ST_STACK;
}

//***************************************************************
// Function: satrcomb - Process COMB attribute for .SEG pseudo-op
// Returned: Nothing
//***************************************************************

void satrcomb(
    struct sd *sgd)

{
    sgd->sd_type = ST_COMB;
}

//***************************************************************
// Function: satrpriv - Process PRIV attribute for .SEG pseudo-op
// Returned: Nothing
//***************************************************************

void satrpriv(
    struct sd *sgd)

{
    ulong value;

    if (getaval(&value))
    {
        if (value > 3)
            errorx();
        else
            sgd->sd_priv = value;
    }
}

//*******************************************************************
// Function: satrselect - Process SELECT attribute for .SEG pseudo-op
// Returned: Nothing
//*******************************************************************

void satrselect(
    struct sd *sgd)

{
    ulong value;

    if (getaval(&value))
    {
        if (value == 0)
            errorx();
        else
            sgd->sd_select = value;
    }
}

//***************************************************************
// Function: satraddr - Process ADDR attribute for .SEG pseudo-op
// Returned: Nothing
//***************************************************************

void satraddr(
    struct sd *sgd)

{
    ulong value;

    if (getaval(&value))
    {
        if (value == 0xFFFFFFL)
            errorx();
        else
        {
            sgd->sd_addr = value;
            sgd->sd_flag |= SF_ADDR;
        }
    }
}

//*************************************************************
// Function: satrmod - Process MOD attribute for .SEG pseudo-op
// Returned: Nothing
//*************************************************************

void satrmod(
    struct sd *sgd)

{
    ulong value;

    if (getaval(&value))
    {
        if (value == 0xFFFFFFFFL)
            errorx();
        else
        {
            sgd->sd_addr = value;
            sgd->sd_flag &= ~SF_ADDR;
        }
    }
}

//********************************************
// Function: pseg16 - Process .SEG16 pseudo-op
// Returned: Nothing
//********************************************

void pseg16(void)

{
    if (chkendx())
        segatrb &= ~SA_32BIT;
}

//********************************************
// Function: pseg32 - Process .SEG32 pseudo-op
// Returned: Nothing
//********************************************

void pseg32(void)

{
    if (chkendx())
        segatrb |= SA_32BIT;
}

//**********************************************
// Function: plnkseg - Process .LNKSEG pseudo-op
// Returned: Nothing
//**********************************************

void plnkseg(void)

{
    struct sn *s1name;
    struct sn *s2name;
    struct sd *s1dba;
    struct sd *s2dba;

    if (!getsym(nxtnbc()))		// Collect first segment name
    {
        errorq();
        return;
    }
    else				// Collect rest of symbol if stopped
	persym();			//   here by a period
    if (isspace(stopper))
        stopper = nxtnbc();
    if ((s1name = (struct sn *)looksym()) == NULL)
    {					// Look for it in the symbol table
        errsub(errimsg, ERR_I);		// Error if not defined
        return;
    }
    if (stopper != ',' || !getsym(nxtnbc())) // Collect second segment name
    {
        errorq();
        return;
    }
    else				// Collect rest of symbol if stopped
	persym();			//   here by a period
    if (isspace(stopper))
        stopper = nxtnbc();
    s1dba = s1name->sn_dba;
    if ((s2name = (struct sn *)looksym()) == NULL || s1name == s2name ||
            ( (((s2dba = s2name->sn_dba)->sd_flag & SF_LINKED) ||
            (s1dba->sd_flag & SF_LINKED)) &&
            (s2dba->sd_linked != s1dba || s1dba->sd_linked != s2dba)))
    {					// Look for it in the symbol table
        errsub(errimsg, ERR_I);		// Error if not defined or if same
        return;				//   or if either segment already
    }					//   linked
    if (!chkend(stopper))		// Must have end of line here
        return;
    s1dba->sd_linked = s2dba;		// Link the two segments
    s1dba->sd_flag |= SF_LINKED;
    s2dba->sd_linked = s1dba;
    s2dba->sd_flag |= SF_LINKED;
}

//******************************************
// Attributes table for the .MSECT pseudo-op
//******************************************

struct atributes matrtbl[] =
{   {{'A','D','D','R',' ',' '}, vv(matraddr)},
    {{'M','A','X',' ',' ',' '}, vv(matrmax)},
    {{'M','O','D',' ',' ',' '}, vv(matrmod)},
    {{'R','E','A','D',' ',' '}, vv(matrread)},
    {{'W','R','I','T','E',' '}, vv(matrwrite)}
};
#define MASIZE (sizeof(matrtbl)/sizeof(struct atributes))

//********************************************
// Function: pmsect - Process .MSECT pseudo-op
// Returned: Nothing
//********************************************

void pmsect(void)

{
    struct mn *mname;
    struct sn *sname;
    struct md *msd;
    long   addr;
    long   max;
    char   atrb;
    char   flag;

    if (ptype == 0)
        ptype = P_8086;
    if (!getsym(nxtnbc()))		// Collect msect name
    {
        errorq();
        return;
    }
    else				// Collect rest of symbol if stopped
	persym();			//   here by a period
    if (isspace(stopper))
        stopper = nxtnbc();
    mname = (struct mn *)looksym();	// Look for it in the symbol table
    if (mname == NULL || (mname->mn_flag&SYF_UNDEF))
    {
        if (mstail && mstail->md_msn == 254)
        {				// Not there - room for another?
            fputs("? XMAC: Too many msects\n", stderr);
            exit(1);
        }
        if (mname == NULL)
            mname = (struct mn *)entersym(); // OK - put name block in symbol
	else				     //   table
            mname->mn_flag &= ~SYF_UNDEF;
        mname->mn_type = SYT_MSC;
        msd = (struct md *)getblock();	// Block for msect data block
        msd->md_nba = mname;		// Initialize it
        msd->md_addr = -1;
        msd->md_max = -1;
        msd->md_msn = (mstail)? mstail->md_msn + 1: 0x101;
        msd->md_atrb = 0;
        msd->md_flag = 0;
        msd->md_sgn = 0;
        msd->md_sdb = NULL;
        msd->md_next = msd;
        mname->mn_msn = msd->md_msn;
        mname->mn_flag = 0;
        mname->mn_dba = msd;
    }
    else
        msd = mname->mn_dba;
    if (stopper == ',')
    {
        if (!getsym(nxtnbc()))		// Collect segment name
        {
            errsub(errimsg, ERR_I);
            return;
        }
        else				// Collect rest of symbol if stopped
            persym();			//   here by a period
        if (isspace(stopper))
            stopper = nxtnbc();
        sname = (struct sn *)looksym(); // Look for it in the symbol table
        if (sname == NULL || sname->sn_type != SYT_SEG)
        {
            errsub(errimsg, ERR_I);
            return;
        }
        else
        {
            if (msd->md_sdb)
            {
                if (msd->md_sdb->sd_nba != sname)
                {
                    errsub(errdnmsg, ERR_DN);
                    return;
                }
            }
            else
            {
                msd->md_sgn = sname->sn_sgn;
                msd->md_sdb = sname->sn_dba;
            }
        }
    }
    else				// No segment specified        
    {
        if (msd->md_sdb == NULL)	// Already have a segment?
        {
            errsub(errimsg, ERR_I);	// No - error
            return;
        }
    }
    while (stopper == ',')
        hndlatr(matrtbl, MASIZE, (struct pd *)msd);
    if (msd->md_next == msd)		// Is this block linked yet?
    {
        if (mstail)			// No - link it now
            mstail->md_next = msd;
        else
            mshead = msd;
        mstail = msd;
        msd->md_next = NULL;
        while (stopper == ',')
            hndlatr(matrtbl, MASIZE, (struct pd *)msd);
    }
    else
    {
        atrb = msd->md_atrb;
        flag = msd->md_flag;
        addr = msd->md_addr;
        max = msd->md_max;
        while (stopper == ',')
            hndlatr(matrtbl, MASIZE, (struct pd *)msd);
        if (msd->md_atrb != atrb || msd->md_flag != flag ||
                msd->md_addr != addr || msd->md_max != max)
        {
            errorc();
            msd->md_atrb = atrb;
            msd->md_flag = flag;
            msd->md_addr = addr;
            msd->md_max = max;
        }
    }
    chkend(stopper);
}

//***************************************************************
// Function: matrmax - Process MAX attribute for .MSECT pseudo-op
// Returned: Nothing
//***************************************************************

void matrmax(
    struct md *msd)

{
    ulong value;

    if (getaval(&value))
    {
        if (value == 0xFFFFFFFFL)
            errorx();
        else
            msd->md_max = value;
    }
}

//*******************************************************************
// Function: matrwrite - Process WRITE attribute for .MSECT pseudo-op
// Returned: Nothing
//*******************************************************************

void matrwrite(
    struct md *msd)

{
    msd->md_atrb |= MA_WRITE;
}

//*****************************************************************
// Function: matrread - Process READ attribute for .MSECT pseudo-op
// Returned: Nothing
//*****************************************************************

void matrread(
    struct md *msd)

{
    msd->md_atrb |= MA_READ;
}

//*****************************************************************
// Function: matraddr - Process ADDR attribute for .MSECT pseudo-op
// Returned: Nothing
//*****************************************************************

void matraddr(
    struct md *msd)

{
    ulong value;

    if (getaval(&value))
    {
        if (value == 0xFFFFFFFFL)
            errorx();
        else
        {
            msd->md_addr = value;
            msd->md_flag |= MF_ADDR;
        }
    }
}

//***************************************************************
// Function: matrmod - Process MOD attribute for .MSECT pseudo-op
// Returned: Nothing
//***************************************************************

void matrmod(
    struct md *msd)

{
    ulong value;

    if (getaval(&value))
    {
        if (value == 0xFFFFFFFFL)
            errorx();
        else
        {
            msd->md_addr = value;
            msd->md_flag &= ~MF_ADDR;
        }
    }
}

//******************************************
// Attributes table for the .PSECT pseudo-op
//******************************************

struct atributes patrtbl[] =
{   {{'A','D','D','R',' ',' '}, vv(patraddr)},
    {{'M','O','D',' ',' ',' '}, vv(patrmod)},
    {{'O','V','E','R',' ',' '}, vv(patrover)}
};
#define PASIZE (sizeof(patrtbl)/sizeof(struct atributes))

//********************************************
// Function: ppsect - Process .PSECT pseudo-op
// Returned: Nothing
//********************************************

void ppsect(void)

{
    struct pn *pname;
    struct mn *mname;
    struct pd *psd;
    long   addr;
    char   atrb;
    char   flag;

    if (ptype == 0)
        ptype = P_8086;
    if (!getsym(nxtnbc()))		// Collect psect name
    {
        errorq();
        return;
    }
    else				// Collect rest of symbol if stopped
	persym();			//   here by a period
    if (isspace(stopper))
        stopper = nxtnbc();
    pname = (struct pn *)looksym();	// Look for it in the symbol table
    if (pname == NULL || (pname->pn_flag&SYF_UNDEF))
    {
        if (pstail && pstail->pd_psn == 254)
        {				// Not there - room for another?
            fputs("? XMAC: Too many psects - cannot continue\n", stderr);
            exit(1);
        }
        if (pname == NULL)
            pname = (struct pn *)entersym(); // OK - put name block in symbol
        else				     //   table
            pname->pn_flag &= ~SYF_UNDEF;
        pname->pn_type = SYT_PSC;
        psd = (struct pd *)getblock();	// Get block for psect data block
        psd->pd_nba = pname;		// Initialize it
        psd->pd_ofs = 0;
        psd->pd_tsize = 0;
        psd->pd_lsize = 0;
        psd->pd_psn = (pstail)? pstail->pd_psn + 1: 1;
        psd->pd_atrb = 0;
        psd->pd_flag = 0;
        psd->pd_msn = 0;
        psd->pd_mdb = NULL;
        psd->pd_next = psd;
        psd->pd_addr = 0xFFFFFFFFL;
        pname->pn_psn = psd->pd_psn;
        pname->pn_flag = 0;
        pname->pn_dba = psd;
    }
    else
        psd = pname->pn_dba;
    if (stopper == ',')
    {
        if (!getsym(nxtnbc()))		// Collect msect name
        {
            errsub(errimsg, ERR_I);
            return;
        }
        else				// Collect rest of symbol if stopped
            persym();			//   here by a period
        if (isspace(stopper))
            stopper = nxtnbc();
        mname = (struct mn *)looksym(); // Look for it in the symbol table
        if (mname == NULL || mname->mn_type != SYT_MSC)
        {
            errsub(errimsg, ERR_I);
            return;
        }
        else
        {
            if (psd->pd_mdb != NULL)
            {
                if (psd->pd_mdb->md_nba != mname)
                {
                    errsub(errdnmsg, ERR_DN);
                    return;
                }
            }
            else
            {
                psd->pd_msn = mname->mn_msn;
                psd->pd_mdb = mname->mn_dba;
            }
        }
    }
    else				// No msect specified        
    {
        if (psd == NULL || psd->pd_mdb == NULL) // Already have an msect?
        {
            errsub(errimsg, ERR_I);	// No - error
            return;
        }
    }
    if (psd->pd_next == psd)		// Is this block linked yet?
    {
        if (pstail)			// No - link it now
            pstail->pd_next = psd;
        else
            pshead = psd;
        pstail = psd;
        psd->pd_next = NULL;
        while (stopper == ',')
            hndlatr(patrtbl, PASIZE, psd);
    }
    else
    {
        atrb = psd->pd_atrb;
        flag = psd->pd_flag;
        addr = psd->pd_addr;
        while (stopper == ',')
            hndlatr(patrtbl, PASIZE, psd);
        if (psd->pd_atrb != atrb || psd->pd_flag != flag ||
                psd->pd_addr != addr)
        {
            errorc();
            psd->pd_atrb = atrb;
            psd->pd_flag = flag;
            psd->pd_addr = addr;
        }
    }
    if (psd->pd_mdb->md_sdb == NULL)
        errsub(errimsg, ERR_I);		// Error if no msect for segment
    else
    {
        curpsd = psd;			// Store pointer to current psect
        curmsd = psd->pd_mdb;		// Store pointer to current msect
        cursgd = curmsd->md_sdb;	// Store pointer to current segment
        segatrb = cursgd->sd_atrb;	// Store current segment attributes
    }
    chkend(stopper);
}

//*****************************************************************
// Function: patrover - Process OVER attribute for .MSECT pseudo-op
// Returned: Nothing
//*****************************************************************

void patrover(
    struct pd *psd)

{
    psd->pd_atrb |= PA_OVER;
}

//*****************************************************************
// Function: patraddr - Process ADDR attribute for .MSECT pseudo-op
// Returned: Nothing
//*****************************************************************

void patraddr(
    struct pd *psd)

{
    ulong value;

    if (getaval(&value))
    {
        if (value == 0xFFFFFFFFL)
            errorx();
        else
        {
            psd->pd_addr = value;
            psd->pd_flag |= PF_ADDR;
        }
    }
}

//***************************************************************
// Function: patrmod - Process MOD attribute for .MSECT pseudo-op
// Returned: Nothing
//***************************************************************

void patrmod(
    struct pd *psd)

{
    ulong value;

    if (getaval(&value))
    {
        if (value == 0xFFFFFFFFL)
            errorx();
        else
        {
            psd->pd_addr = value;
            psd->pd_flag &= ~PF_ADDR;
        }
    }
}

//*************************************************
// Function: hndlatr - Process single attribute for
//	.SEG, .MSECT, or .PSECT
// Returned: Nothing
//*************************************************

void hndlatr(
    struct atributes *tbl,
    int    size,
    struct pd *block)

{
    char  *cpnt;
    struct atributes *bpnt;
    int    cnt;
    char   chr;

    chr = nxtnbc();
    cpnt = symbuf.nsym;
    cnt = SYMMAXSZ;
    while (--cnt >= 0)
	*cpnt++ = ' ';			// Clear symbuf
    symsize = 0;			// Clear symbol size
    cpnt = symbuf.nsym;
    do
    {   if (symsize < 6)		// Is symbol full now?
	{
	    ++symsize;			// No
	    *cpnt++ = toupper(chr);	// Store character in symbol
	}
        else
        {
            errorq();
            return;
        }
	chr = nxtnbc();			// Get next character
    } while (chr && (chr != ',') && (chr != '='));
    stopper = chr;			// Store stopper
    if ((bpnt = (struct atributes *)srchtbl(symbuf.nsym, tbl, size, 6,
            sizeof(struct atributes))) == NULL)
        errorq();
    else
    {
        (*bpnt->at_func)(block);
        if (isspace(stopper))
            stopper = nxtnbc();
    }
}

//********************************************
// Function: getaval - Get value for attribute
// Returned: TRUE if OK, FALSE if error
//********************************************

int getaval(
    ulong *value)

{
    if (stopper == '=')			// Does value follow?
    {
        getabs();			// Yes - get it
        if (errflag & ERR_X)		// Error?
            return (FALSE);		// Yes
        else
        {
            *value = val.vl_val.vl_vl;	// No
            return (TRUE);
        }
    }
    else
    {
        errorx();
        return (FALSE);
    }
}

//********************************************
// Function: pentry - Process .ENTRY pseudo-op
// Returned: Nothing
//********************************************

void pentry(void)

{
    struct sy *sym;

    do
    {   sym = getnxs();			// Collect symbol name
	if (sym->sy_flag & SYF_EXTERN)	// Is it external?
        {
	    adrerr();			// Yes - error
            continue;
        }
	else if (sym->sy_flag & SYF_UNDEF)// Undefined?
	    errsub(errumsg, ERR_U);	// Yes - error
        else if (sym->sy_flag & SYF_IMPORT) //Imported?
            sym->sy_flag |= SYF_ENTRY;
	else
	    sym->sy_flag |= (SYF_INTERN|SYF_ENTRY); // OK - set bits
    } while (stopper == ',');
    chkend(stopper);
}

//**********************************************
// Function: pintern - Process .INTERN pseudo-op
// Returned: Nothing
//**********************************************

void pintern(void)

{
    entint(SYF_INTERN, ~0);
}


//**********************************************
// Function: pexport - Process .EXPORT pseudo-op
// Returned: Nothing
//**********************************************

void pexport(void)

{
    entint(SYF_EXPORT, ~SYF_INTERN);
}

//*********************************************************************
// Function: entint - Common to .ENTRY, .INTERN, and .EXPORT pseudo-ops
// Returned: Nothing
//*********************************************************************

void entint(
    int flags, int clear)

{
    struct sy *sym;

    do
    {   sym = getnxs();			// Collect symbol name
	if (sym->sy_flag & (SYF_EXTERN|SYF_IMPORT)) // Is it external or
        {					    //   or imported?
	    adrerr();			// Yes - error
            continue;
        }
	else if (sym->sy_flag & SYF_UNDEF)// Undefined?
	    errsub(errumsg, ERR_U);	// Yes - error
	else
	    sym->sy_flag |= flags;	// OK - set bits
        sym->sy_flag &= clear;
    } while (stopper == ',');
    chkend(stopper);
}

//**********************************************
// Function: pimport - Process .IMPORT pseudo-op
// Returned: Nothing
//**********************************************

void pimport(void)

{
    struct sy *sym;

    do
    {   sym = getnxs();			// Collect symbol name
	if (sym->sy_flag & SYF_IMPORT)	// Is it imported?
	    continue;			// Yes - nothing needed here
	else if (!(sym->sy_flag & (SYF_UNDEF|SYF_EXTERN)))
					// Undefined or external?
	    adrerr();			// No - error
	else
	{
	    sym->sy_flag |= (SYF_IMPORT); // OK - indicate imported
	    sym->sy_flag &= ~(SYF_UNDEF|SYF_EXTERN); // Not undefined or
            sym->sy_val.val = 0;		     //   or external
            sym->sy_psect = 0;
	}
    } while (stopper == ',');
    chkend(stopper);
}

//**********************************************
// Function: pextern - Process .EXTERN pseudo-op
// Returned: Nothing
//**********************************************

void pextern(void)

{
    struct sy *sym;

    do
    {   sym = getnxs();			// Collect symbol name
	if (sym->sy_flag & (SYF_EXTERN|SYF_IMPORT)) // Is it an external or
						    //   imported?
	    continue;			// Yes - nothing needed here
	else if (!(sym->sy_flag & SYF_UNDEF))// Undefined?
	    adrerr();			// No - error
	else
	{
	    sym->sy_flag |= (SYF_EXTERN); // OK - indicate external
	    sym->sy_flag &= ~SYF_UNDEF;	// Not undefined
            sym->sy_val.val = 0;
            sym->sy_psect = 0;
	}
    } while (stopper == ',');
    chkend(stopper);
}

//***********************************************************
// Function: getnxs - Get symbol name for .ENTRY, .INTERN and
//	.EXTERN pseudo-ops
// Returned: Address of symbol table entry for name
//***********************************************************

struct sy *getnxs(void)

{
    getsym(nxtnbc());			// Collect symbol
    stopper = nxtnb0(stopper);		// Make sure non-blank stopper
    return (findsym(FALSE));
}

//****************************************
// Function: podd - Process .ODD pseudo-op
// Returned: Nothing
//****************************************

void podd(void)

{
    if (!(curpsd->pd_ofs & 1))
    {
	++curpsd->pd_ofs;
	lstadr = curpsd->pd_ofs;
    }
    adrflg = TRUE;			// List address
    chkendx();
}

//******************************************
// Function: peven - Process .EVEN pseudo-op
// Returned: Nothing
//******************************************

void peven(void)

{
    if (curpsd->pd_ofs & 1)
    {
	++curpsd->pd_ofs;
	lstadr = curpsd->pd_ofs;
    }
    adrflg = TRUE;			// List address
    chkendx();
}

//****************************************
// Function: pmod - Process .MOD pseudo-op
// Returned: Nothing
//****************************************

void pmod(void)

{
    long amnt;

    getabs();				// Get absolute value
    if (val.vl_val.vl_vl == 0)		// Must not be 0
    {
        errorx();
	skiprest();
	return;
    }
    if (!chkend(stopper))		// Must have end next
	return;
    if ((curpsd != NULL) && (amnt = curpsd->pd_ofs % val.vl_val.vl_vl) != 0)
    {
	curpsd->pd_ofs += val.vl_val.vl_vl - amnt;
	lstadr = curpsd->pd_ofs;
        if (curpsd->pd_tsize < curpsd->pd_ofs) // Does this increase the
					       //   total size?
            curpsd->pd_tsize = curpsd->pd_ofs; // Yes (does not change loaded
    }					       //   size)
    adrflg = TRUE;			// List address
}

//******************************************
// Function: pbyte - Process .BYTE pseudo-op
// Returned: Nothing
//******************************************

void pexpb(void)

{
    chkpsect();				// Make sure have psect specified
    do
    {   exprsn();			// Get value
        putbyte(U_VALUE, &val);		// Output unsigned byte
    } while (stopper == ',');		// Continue if more
    chkend(stopper);
}

//***************************************************
// Function: pexpw - Process .EXPW or .WORD pseudo-op
// Returned: Nothing
//***************************************************

void pexpw(void)

{
    chkpsect();				// Make sure have psect specified
    do
    {   exprsn();			// Get value
        putword(U_VALUE, &val);		// Output unsigned word
    } while (stopper == ',');		// Continue if more
    chkend(stopper);
}

//***************************************************
// Function: pexpl - Process .EXPL or .LONG pseudo-op
// Returned: Nothing
//***************************************************

void pexpl(void)

{
    chkpsect();				// Make sure have psect specified
    do
    {   exprsn();			// Get value
        putlong(U_VALUE, &val);		// Output unsigned long
    } while (stopper == ',');		// Continue if more
    chkend(stopper);
}

//********************************************
// Function: paddrw - Process .ADDRW pseudo-op
// Returned: Nothing
//********************************************

void paddrw(void)

{
    chkpsect();				// Make sure have psect specified
    do
    {   exprsn();			// Get value
        putword(U_VALUE, &val);		// Output unsigned word
        putsel(&val);			// Output segment selector
    } while (stopper == ',');		// Continue if more
    chkend(stopper);
}

//********************************************
// Function: paddrl - Process .ADDRL pseudo-op
// Returned: Nothing
//********************************************

void paddrl(void)

{
    chkpsect();				// Make sure have psect specified
    do
    {   exprsn();			// Get value
        putlong(U_VALUE, &val);		// Output unsigned long
        putsel(&val);			// Output segment selector
    } while (stopper == ',');		// Continue if more
    chkend(stopper);
}

//******************************************
// Function: pblkb - Process .BLKB pseudo-op
// Returned: Nothing
//******************************************

void pblkb(void)

{
    doblk(1);
}

//******************************************
// Function: pblkw - Process .BLKW pseudo-op
// Returned: Nothing
//******************************************

void pblkw(void)

{
    doblk(2);
}

//******************************************
// Function: pblkl - Process .BLKL pseudo-op
// Returned: Nothing
//******************************************

void pblkl(void)

{
    doblk(4);
}

//**************************************************
// Function: doblk - Common to all .BLK.s pseudo-ops
// Returned: Nothing
//**************************************************

void doblk(
    int size)

{
    chkpsect();				// Make sure have psect specified
    exprsn();				// Get value of expression
    if (val.vl_kind != VK_ABS || (errflag&ERR_U) || val.vl_val.vl_vl < 0)
    {					// Value must be absolute, positive
	errsub(errvmsg, ERR_V);		//   and defined, else V error
        val.vl_kind = VK_ABS;
        val.vl_psect = 0;
        val.vl_val.vl_vl = 0;
    }
    curpsd->pd_ofs += val.vl_val.vl_vl * size; // Bump offset
    if (curpsd->pd_tsize < curpsd->pd_ofs) // Does this increase the total
					   //   size?
        curpsd->pd_tsize = curpsd->pd_ofs; // Yes (does not change loaded
					   //   size)
    adrflg = TRUE;			// List address
    chkend(stopper);
} 

//********************************************
// Function: pasciz - Process .ASCIZ pseudo-op
// Returned: Nothing
//********************************************

void pasciz(void)

{
    chrstring(absbyte);
    absbyte('\0');
}

//********************************************
// Function: pascii - Process .ASCII pseudo-op
// Returned: Nothing
//********************************************

void pascii(void)

{
    chrstring(absbyte);
}

//********************************************
// Function: pascii - Process .ASCIU pseudo-op
// Returned: Nothing
//********************************************

void pasciu(void)

{
    chrstring(upperchr);
}

//********************************************
// Function: pascii - Process .ASCIL pseudo-op
// Returned: Nothing
//********************************************

void pascil(void)

{
    chrstring(lowerchr);
}

//*************************************************************************
// Function: chrstring - Do the work for .ASCII, .ASCIZ, .ASCIU, and .ASCIL
// Returned: Nothing
//*************************************************************************

void chrstring(
    void (*func)(long c))

{
    char chr;
    char delim;

    chkpsect();				// Make sure have psect specified
    while ((chr=nxtnbc()) != ';' && !eolflg)
    {
	if (chr == '{')			// If value
	{
	    do
	    {   exprsn();
		putbyte(U_VALUE, &val);
	    } while (stopper == ',');
	    if (stopper != '}')
	    {
		queerr();
		return;
	    }
	}
	else				// If character string
	{
	    delim = chr;
	    while ((chr=nxtchar()) != delim)
	    {
		if(eolflg)
		{
		    queerr();
		    return;
		}
		func(chr);
	    }
	}
    }
    stopper = nxtnb0(chr);
}


//**********************************************************
// Function: upperchr - Store character forced to upper case
// Returned: Nothing
//**********************************************************

void upperchr(
    long chr)

{
    absbyte(toupper(chr));
}

//**********************************************************
// Function: upperchr - Store character forced to lower case
// Returned: Nothing
//**********************************************************

void lowerchr(
    long chr)

{
    absbyte(tolower(chr));
}

//********************************************
// Function: pstype - Process .STYPE pseudo-op
// Returned: Nothing
//********************************************

// Format is:
//	.STYPE	sym1,sym2
//   The value of sym1 is set as follows
//   based on the attributes of sym2
//	2000 = Exported sumbol
//	1000 = Imported symbol
//	0800 = Macro name
//	0400 = Local symbol
//	0200 = Relocatable symbol
//	0100 = External symbol
//	0080 = Defined symbol
//	0040 = Multiply defined symbol
//	0020 = Label
//	0010 = Suppressed symbol
//	0008 = Internal symbol
//	0004 = Entry symbol
//	0002 = Used symbol
//	0001 = Byte symbol

void pstype(void)

{
    struct sy *sym1,*sym2;
    int    temp;

    if ((sym1=nchsty()) == NULL)	// Setup first symbol
	return;
    if (!getsym(nxtnbc()))		// Collect second symbol
    {
	queerr();
	return;
    }
    if (symbuf.nsym[0] == 0)		// Local symbol?
	sym1->sy_val.val = 0x0400;	// Yes
    else if (symbuf.nsym[0] |= 0x80, looksym())// Macro name?
	sym1->sy_val.val = 0x0800;	// Yes
    else
	sym1->sy_val.val = 0x0000;
    symbuf.nsym[0] &= ~0x80;
    if ((sym2 = looksym()) != NULL)	// Look for symbol in symbol table
    {
	if (sym2->sy_flag & SYF_IMPORT)	// Imported?
	    sym1->sy_val.val |= 0x1000;	// Yes
	else if (sym2->sy_flag & SYF_EXTERN) // External?
	    sym1->sy_val.val |= 0x0100;	// Yes
	else if (sym2->sy_flag & SYF_EXTERN) // Relocatable?
	    sym1->sy_val.val |= 0x0200;	// Yes
	temp = sym2->sy_flag ^ SYF_UNDEF; // Get flag bits with undefined bit
					//   inverted
	sym1->sy_val.val |= (long)(temp & 0xFF);
    }
    if (!prmflg)
	sym1->sy_flag |= SYF_USED;
    *blbpnt.c++ = LT_WORD;
    *blbpnt.c++ = 0;
    *blbpnt.s++ = sym1->sy_val.val;
    ++blbcnt;
    chkend(stopper);
}

//********************************************
// Function: pnchar - Process .NCHAR pseudo-op
// Returned: Nothing
//********************************************

// Format is:
//	.NCHAR	sym,<str>
//   The value of sym is set to the
//   number of characters in str

void pnchar(void)

{
    struct sy *sym;
    char   chr;

    if ((sym=nchsty()) == NULL)		// Setup first symbol
	return;
    chr = nxtnbc();
    if (chr != '{')
    {
	queerr();
	return;
    }
    sym->sy_val.val = -1;
    do					// Scan to matching right angle
    {   ++sym->sy_val.val;
        chr = nxtchar();
    } while (!eolflg && chr != '}');
    if (eolflg)
    {
	queerr ();
	return;
    }
    if (!prmflg)
	sym->sy_flag |= SYF_USED;
    *blbpnt.c++ = LT_WORD;
    *blbpnt.c++ = 0;
    *blbpnt.s++ = sym->sy_val.val;
    ++blbcnt;
    chkend(stopper);
}

//*********************************************************
// Function: nchsty - Do common setup for .STYPE and .NCHAR
// Returned: Address of symbol table entry
//*********************************************************

struct sy *nchsty(void)

{
    struct sy *sym;

    if (!getsym(nxtnbc()) || stopper != ',') // Collect symbol to be defined
    {
	queerr();
	return (NULL);
    }
    sym = findsym(TRUE);		// Find it in the symbol table
    if (sym->sy_flag & (SYF_LABEL|SYF_EXTERN|SYF_IMPORT))
    {					// Error if external or label
	errsub(errmmsg, ERR_M);
	skiprest();
	return (NULL);
    }
    sym->sy_flag &= ~SYF_UNDEF;		// Indicate defined and absolute
    sym->sy_flag |= SYF_ABS;
    sym->sy_psect = 0;
    return (sym);
}

//**********************************************
// Function: pnrname - Process .NRNAME pseudo-op
// Returned: Nothing
//**********************************************

void pnrname(void)

{
    if (chkendx ())
	prnflg = FALSE;
}

//**********************************************
// Function: pprname - Process .PRNAME pseudo-op
// Returned: Nothing
//**********************************************

void pprname(void)

{
    if (chkendx ())
	prnflg = TRUE;
}

//********************************************
// Function: pstart - Process .START pseudo-op
// Returned: Nothing
//********************************************

void pstart(void)

{
    chkpsect();				// Make sure have psect specified
    exprsn();				// Get starting address
    chkend(stopper);
    if (stradr || strpsn)		// Can only have one!
    {
        errsub(errdmsg, ERR_D);
        return;
    }
    if (val.vl_kind != VK_OFS)		// Must be address
    {
	adrerr();
        return;
    }
    stradr = val.vl_val.vl_vl;		// Store starting address
    strpsn = val.vl_psect;
    *blbpnt.c++ = LT_LONG;		// List it
    *blbpnt.c++ = val.vl_psect;
    *blbpnt.l++ = val.vl_val.vl_vl;
    ++blbcnt;
}

//********************************************
// Function: pstack - Process .STACK pseudo-op
// Returned: Nothing
//********************************************

void pstack(void)

{
    chkpsect();				// Make sure have psect specified
    exprsn();				// Get stack address
    chkend(stopper);
    if (stkadr || stkpsn)		// Can only have one!
    {
        errsub(errdmsg, ERR_D);
        return;
    }
    if (val.vl_kind != VK_OFS)		// Must be address
    {
	adrerr();
        return;
    }
    stkadr = val.vl_val.vl_vl;		// Store stack address
    stkpsn = val.vl_psect;
    *blbpnt.c++ = LT_LONG;		// List it
    *blbpnt.c++ = val.vl_psect;
    *blbpnt.l++ = val.vl_val.vl_vl;
    ++blbcnt;
}

//********************************************
// Function: pstk16 - Process .STK16 pseudo-op
// Returned: Nothing
//********************************************

void pstk16(void)

{
    if (chkendx())
        stack32 = FALSE;
}

//********************************************
// Function: pstk32 - Process .STK32 pseudo-op
// Returned: Nothing
//********************************************

void pstk32(void)

{
    if (chkendx())
        stack32 = TRUE;
}

//********************************************
// Function: pdebug - Process .DEBUG pseudo-op
// Returned: Nothing
//********************************************

void pdebug(void)

{
    chkpsect();				// Make sure have psect specified
    exprsn();				// Get debugger address
    chkend(stopper);
    if (xdtadr || xdtpsn)		// Can only have one!
    {
        errsub(errdmsg, ERR_D);
        return;
    }
    if (val.vl_kind != VK_OFS)		// Must be address
    {
	adrerr();
        return;
    }
    xdtadr = val.vl_val.vl_vl;		// Store stack address
    xdtpsn = val.vl_psect;
    *blbpnt.c++ = LT_LONG;		// List it
    *blbpnt.c++ = val.vl_psect;
    *blbpnt.l++ = val.vl_val.vl_vl;
    ++blbcnt;
}
