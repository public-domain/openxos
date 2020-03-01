//*****************************************
// Functions to output binary data for XMAC
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
#include <XOS.H>
#include <XOSERRMSG.H>
#include "XMAC.H"
#include "XMACEXT.H"

//*************************************************
// Function: bininit - Initialize for binary output
// Returned: Nothing
//*************************************************

void bininit(void)

{
    int    cnt;
    union
    {   struct sd *s;
        struct md *m;
        struct pd *p;
        char  *c;
    } pnt;

    if (!test)				// Want binary output?
    {
	outfsp = defspec(outfsp, (char *)NULL, "OBJ");
					// Yes - finish setting up default
	if ((outfp=fopen(outfsp, "wb")) == NULL) // Open the output file
	    femsg(prgname, -errno, outfsp); // If error
	binbyte('\204');		// Store file format byte
	binbyte('\0');			// Indicate program module
	binbyte('\201');		// Store module format byte
        if (ptype == 0)			// Was a processor specified?
            ptype = P_8086;		// No - assume 8086
	binbyte(ptype);			// Store processor type
	strsect(SC_START);		// Start the start of module
					//  section
	cnt = SYMMAXSZ;			// Put null at end of module
					//  name
	pnt.c = hdrbfr;			//  (this will destroy the
	do				//   header buffer, but this
        {   if (*pnt.c++ == ' ')	//   is OK since have already
	    {				//   closed listing output)
		*(--pnt.c) = '\0';
		break;
	    }
	} while (--cnt > 0);
	binxsm(hdrbfr);			// Output module name
	endsect();			// End the section

        if ((pnt.s = sghead) != NULL)	// Point to first segment data block
        {
            strsect(SC_SEG);		// Start the segment definition
					//  section
            do
            {   binval((long)(pnt.s->sd_select), pnt.s->sd_atrb);
					// Output attribute byte and segment
					//   selector value
                binval(pnt.s->sd_addr, pnt.s->sd_flag);
					// Output flag byte and address or
					//   modulus value
                if (pnt.s->sd_flag & SF_LINKED) // Have a linked segment?
                    binbyte(pnt.s->sd_linked->sd_sgn); // Yes
                binbyte(pnt.s->sd_type); // Output segment type
                binbyte(pnt.s->sd_priv); // Output priviledge level
                binxsm(pnt.s->sd_nba->sn_name); // Output segment name
            } while ((pnt.s = pnt.s->sd_next) != NULL); // Loop for all
							//   segments
            endsect();			// End this section
        }

	if ((pnt.m = mshead) != NULL)	// Point to first msect data block
        {
            strsect(SC_MSECT);		// Start the msect definition
					//  section
            do
            {   binval(pnt.m->md_max, pnt.m->md_atrb);
					// Output attribute byte and maximum
					//   space needed
                binval(pnt.m->md_addr, pnt.m->md_flag);
					// Output flag byte and address or
					//   modulus value
                binbyte(0);		// Output msect type (not used)
                binbyte(0);		// Output priviledge level (not used)
                binbyte(pnt.m->md_sgn);	// Output segment number
                binxsm(pnt.m->md_nba->mn_name); // Output msect name
            } while ((pnt.m = pnt.m->md_next) != NULL); // Loop for all msects
            endsect();			// End this section
        }

        if ((pnt.p = pshead) != NULL)	// Point to first psect data block
        {
            strsect(SC_PSECT);		// Start the psect definition
					//  section
            do
            {   binval(pnt.p->pd_tsize, pnt.p->pd_atrb);
					// Output attribute byte and total
					//   size value
                binval(pnt.p->pd_lsize, 0); // Output loaded size
                binval(pnt.p->pd_addr, pnt.p->pd_flag);
					// Output flag byte and address or
					//   modulus value
                binbyte(pnt.p->pd_msn);	// Output msect number
                binxsm(pnt.p->pd_nba->pn_name); // Output psect name
            } while ((pnt.p = pnt.p->pd_next) != NULL); // Loop for all psects
            endsect();			// End this section
        }

        if (symnum != 0)		// Do we have any symbols?
	{				// Yes
            symnum = 1;
            extimpsect(SC_EXTERN, SYF_EXTERN);
            extimpsect(SC_IMPORT, SYF_IMPORT);
        }
        dbkpnt = dbkbuf;		// Initialize data block pointer
        dbkcnt = 0;			// And count
        binpsn = 0xFF;			// Force initial addr. output
        strsect(SC_DATA);		// Start the data section
    }
}

//*******************************************************
// Function: extimpsect - Output EXTERN or IMPORT section
// Returned: Nothing
//*******************************************************

void extimpsect(
    int type,
    int flags)

{
    int    cnt;
    int    scnt;
    struct sy **hp;
    struct sy  *sp;

    hp = hashtbl;
    cnt = HASHSIZE;
    scnt = 0;
    do
    {
        if ((sp = *hp++) != NULL)	// Get start of hash list
        {
            do
            {
                if ((sp->sy_type == SYT_SYM) && (sp->sy_flag & flags))
                {			// Do we want this one?
                    if (scnt == 0)	// Yes
                        strsect(type);
                    scnt++;
                    sp->sy_symnum = symnum++; // Assign symbol number
                    if (flags & SYF_IMPORT)
                        binbyte((sp->sy_flag&SYF_ENTRY)? 0x40: 0);
                    binxsm(sp->sy_name);// Output symbol name
                    if (scnt >= 200)	// Only allow 200 symbols in each
                    {			//   section
                        endsect();
                        scnt = 0;
                    }
                }
            } 
            while ((sp = sp->sy_hash) != NULL);
        }
    } 
    while (--cnt > 0);
    if (scnt != 0)
        endsect();
}

//*****************************************
// Function: absbyte - Output absolute byte
// Returned: Nothing
//*****************************************

void absbyte(
    long value)

{
    if (blbpnt.c < (blbbuf+BLBSIZE-6))	// Room in bin list buffer?
    {
	++blbcnt;			// Yes
        *blbpnt.c++ = LT_BYTE;
	*blbpnt.c++ = 0;		// Store psect number
	*blbpnt.s++ = value;		// Store value for listing
    }
    if (passtwo && !test)		// Need output to OBJ file?
    {
	chkadr();
	bindata(value);			// Yes
    }
    bumpofs(1);				// Bump offset
    bnoflg = TRUE;			// Indicate binary output done
    adrflg = TRUE;			// List address on this line
}

//*******************************
// Function: putbyte: Output byte
// Returned: Nothing
//*******************************

void putbyte(
    int  type,
    struct vl *value)

{
    int kind;

    chkrel(value);			// Resolve relative value if can
    if (blbpnt.c < (blbbuf+BLBSIZE-6))	// Room in binary listing buffer?
    {
	++blbcnt;			// Yes
        *blbpnt.c++ = LT_BYTE;
        *blbpnt.c++ = ((kind = value->vl_kind) == VK_EXT || kind == VK_SLX)?
                0xFF: (kind == VK_SEL)? 0xFE:
                (kind == VK_ABS || kind == VK_SEG)? 0: value->vl_psect;
	*blbpnt.s++ = value->vl_val.vl_vl; // Store value for listing
    }
    if (passtwo && !test)
    {
	if (value->vl_kind == VK_ABS)	// Should we check range here?
        {				// Yes
	    if ((value->vl_val.vl_vl & 0xFFFFFF00L) == 0)
            {				// Are high 24 bits all 0?
		if (type != U_VALUE && (value->vl_val.vl_vc.lslc & 0x80))
		    errsub(errtmsg, ERR_T); // Yes - error if signed and
					    //   bit 7 is set
	    }
	    else
	    {
		if ((value->vl_val.vl_vl & 0xFFFFFF80L) != 0xFFFFFF80L)
					// Are high 25 bits all 1?
		    errsub(errtmsg, ERR_T); // No - error
	    }
	}
	if (!test)			// Need output to OBJ file?
	{
	    chkadr();			// Output address if need to
            if (value->vl_kind == VK_ABS || value->vl_kind == VK_ABA)
            {
                if (value->vl_type >= VL_BREL)
                {
                    if (!value->vl_undef)
                        adrerr();
                    value->vl_val.vl_vl = 0;
                }
                bindata(value->vl_val.vl_vc.lslc);
            }
            else if (value->vl_kind == VK_SEG)
                bindata(0);
	    else
		general(value, (type==U_VALUE)? PO_STRUB: PO_STRSB); // No
	}
    }
    bumpofs(1);				// Bump offset
    bnoflg = TRUE;			// Indicate binary output done
    adrflg = TRUE;			// List address on this line
}

//*****************************************
// Function: absword - Output absolute word
// Returned: Nothing
//*****************************************

void absword(
    long value)

{
    char byte;

    if (blbpnt.c < (blbbuf+BLBSIZE-6))	// Room in binary listing
					//  buffer?
    {
	++blbcnt;			// Yes
	*blbpnt.c++ = LT_WORD;		// Store value for listing
	*blbpnt.c++ = 0;		// Store psect number
	*blbpnt.s++ = value;
    }
    if (passtwo && !test)		// Need output to OBJ file?
    {
	chkadr();			// Output address if need to
	byte = value;
	bindata(byte);
        byte = value >> 8;
	bindata(byte);
    }
    bumpofs(2);				// Bump offset
    bnoflg = TRUE;			// Indicate binary output done
    adrflg = TRUE;			// List address on this line
}

//********************************
// Function: putword - Output word
// Returned: Nothing
//********************************

void putword(
    int type,
    struct vl *value)

{
    chkrel(value);			// Resolve relative value if can
    listword(value);
    if (passtwo && !test)
    {
        if (value->vl_kind == VK_ABS)	// Should we check range here?
	{				// Yes
	    if (value->vl_val.vl_vs.hs == 0) // Are high 16 bits all 0
	    {
		if (type != U_VALUE && (value->vl_val.vl_vs.ls & 0x8000L))
		    errsub(errtmsg, ERR_T); // Error if signed and bit 15 set
	    }
	    else
	    {
		if ((value->vl_val.vl_vl & 0xFFFF8000L) != 0xFFFF8000L)
					// Are high 17 bits all 1?
		    errsub(errtmsg, ERR_T); // No - error
	    }
	}
	if (!test)			// Need output to OBJ file?
	{
	    chkadr();			// Output address if need to
            if (value->vl_kind == VK_ABS || value->vl_kind == VK_ABA)
	    {
                if (value->vl_type >= VL_BREL)
                {
                    if (!value->vl_undef)
                        adrerr();
                    value->vl_val.vl_vl = 0;
                }
                bindata(value->vl_val.vl_vc.lslc); // Yes
                bindata(value->vl_val.vl_vc.lshc);
	    }
            else if (value->vl_kind == VK_SEG)
            {
                bindata(0);
                bindata(0);
            }
            else
		general(value, (type==U_VALUE)?PO_STRUW:PO_STRSW); // No
	}
    }
    bumpofs(2);				// Bump offset
    bnoflg = TRUE;			// Indicate binary output done
    adrflg = TRUE;			// List address on this line
}

void listword(
    struct vl *value)

{
    int kind;

    if (blbpnt.c < (blbbuf+BLBSIZE-6))	// Room in binary listing buffer?
    {
	++blbcnt;			// Yes
        *blbpnt.c++ = LT_WORD;
        *blbpnt.c++ = ((kind = value->vl_kind) == VK_EXT || kind == VK_SLX)?
                0xFF: (kind == VK_SEL)? 0xFE:
                (kind == VK_ABS || kind == VK_SEG)? 0: value->vl_psect;
	*blbpnt.s++ = value->vl_val.vl_vl; // Store value for listing
    }
}

//********************************
// Function: putlong - Output long
// Returned: Nothing
//********************************

void putlong(
    int    type,
    struct vl *value)

{
    chkrel(value);			// Resolve relative value if can
    listlong(value);
    if (passtwo && !test)		// Need output to OBJ file?
    {
	chkadr();			// Output address if need to
        if (value->vl_kind == VK_ABS || value->vl_kind == VK_ABA)
	{
            if (value->vl_type >= VL_BREL)
            {
                if (!value->vl_undef)
                    adrerr();
                value->vl_val.vl_vl = 0;
            }
            bindata(value->vl_val.vl_vc.lslc);
            bindata(value->vl_val.vl_vc.lshc);
            bindata(value->vl_val.vl_vc.hslc);
            bindata(value->vl_val.vl_vc.hshc);
        }
        else if (value->vl_kind == VK_SEG)
        {
            bindata(0);
            bindata(0);
            bindata(0);
            bindata(0);
        }
        else
	    general(value, (type==U_VALUE)? PO_STRUL: PO_STRSL); // No
    }
    bumpofs(4);				// Bump offset
    bnoflg = TRUE;			// Indicate binary output done
    adrflg = TRUE;			// List address on this line
}

void listlong(
    struct vl *value)

{
    int kind;

    if (blbpnt.c < (blbbuf+BLBSIZE-8))	// Room in binary listing buffer?
    {
	++blbcnt;			// Yes
        *blbpnt.c++ = LT_LONG;
        *blbpnt.c++ = ((kind = value->vl_kind) == VK_EXT || kind == VK_SLX)?
                0xFF: (kind == VK_SEL)? 0xFE:
                (kind == VK_ABS || kind == VK_SEG)? 0: value->vl_psect;
	*blbpnt.l++ = value->vl_val.vl_vl; // Store value for listing
    }
}

//*****************************************************
// Function: putsel - Output segment selector for value
// Returned: Nothing
//*****************************************************

void putsel(
    struct vl *value)

{
    listsel(value);
    if (passtwo && !test)
    {
        if (value->vl_kind == VK_ABA)	// Want selector of absolute address?
        {
	    bindata(value->vl_psect);	// Yes
	    bindata(value->vl_psect>>8);
        }
        else if (value->vl_kind >= VK_SEL || value->vl_kind == VK_ABS)
        {				// Want selector of selector(!) or
					//   of absolute value?
            errorna();			// Yes - complain
            bindata(0);			// And store 0
            bindata(0);
        }
        else
        {
            wrtblk();
            if (value->vl_kind == VK_EXT) // External value?
            {
                binbyte(PO_PSELSYM);	// Yes
                binnumber(value->vl_pnt->sy_symnum);
            }
            else			// Not external
            {          
                if (value->vl_psect == curpsd->pd_psn) // Current psect?
                    binbyte(PO_PSELCP);	// Yes
                else
                {
                    binbyte(PO_PSELAP);	// No
                    binbyte(value->vl_psect);
                }
            }
            binbyte(PO_STRUW);		// Output store operator
        }
    }
    bumpofs(2);				// Bump offset
    bnoflg = TRUE;			// Indicate binary output done
    adrflg = TRUE;			// List address on this line
}

void listsel(
    struct vl *value)

{
    if (blbpnt.c < (blbbuf+BLBSIZE-8))	// Room in binary listing buffer?
    {
	++blbcnt;			// Yes
        *blbpnt.c++ = LT_WORD;
        if (value->vl_kind == VK_ABA)
        {
            *blbpnt.c++ = 0;
            *blbpnt.s++ = value->vl_psect;
        }
        else if (value->vl_kind >= VK_SEL || value->vl_kind == VK_ABS)
        {
            *blbpnt.c++ = 0;
            *blbpnt.s++ = 0;
        }
        else
        {
            *blbpnt.c++ = (value->vl_kind == VK_EXT)? 0xFF:
                    (value->vl_kind >= VK_SEL)? 0: value->vl_psect;
            *blbpnt.s++ = 0;		// Always list 0 as "value" for
        }				//   segment selector!
    }
}

//*****************************************
// Function: putaddrw - Output word address
// Returned: Nothing
//*****************************************

void putaddrw(
    struct vl *value)

{
    chkrel(value);
    if ((value->vl_kind != VK_OFS && value->vl_kind != VK_EXT) ||
            value->vl_type >= VL_BREL)
    {
        putword(U_VALUE, value);
        putsel(value);
    }
    else				// Have non-relative address if get here
    {
        listsel(value);
        listword(value);
        if (passtwo && !test)		// Need output to OBJ file?
        {
            wrtblk();
            chkadr();			// Output address if need to
            if (value->vl_kind == VK_EXT) // External?
            {	    			  // Yes
                binval(value->vl_val.vl_vl, PO_POFSSYM);
                binnumber(value->vl_pnt->sy_symnum);
            }
            else
            {
                if (value->vl_psect == curpsd->pd_psn) // For current psect?
                    binval(value->vl_val.vl_vl,	PO_POFSCP); // Yes
                else			// No - some other psect
                {   
                    binval(value->vl_val.vl_vl, PO_POFSAP);
                    binbyte(value->vl_psect); // Output psect number
                }
            }
            binbyte(PO_STRAW);
        }
        bumpofs(4);			// Bump offset
        bnoflg = TRUE;			// Indicate binary output done
        adrflg = TRUE;			// List address on this line
    }
}

//*****************************************
// Function: putaddrl - Output long address
// Returned: Nothing
//*****************************************

void putaddrl(
    struct vl *value)

{
    chkrel(value);
    if ((value->vl_kind != VK_OFS && value->vl_kind != VK_EXT) ||
            value->vl_type >= VL_BREL)
    {
        putlong(U_VALUE, value);
        putsel(value);
    }
    else				// Have non-relative address if get here
    {
        listsel(value);
        listlong(value);
        if (passtwo && !test)		// Need output to OBJ file?
        {
            wrtblk();
            chkadr();			// Output address if need to
            if (value->vl_kind == VK_EXT) // External?
            {	    			  // Yes
                binval(value->vl_val.vl_vl, PO_POFSSYM);
                binnumber(value->vl_pnt->sy_symnum);
            }
            else
            {
                if (value->vl_psect == curpsd->pd_psn) // For current psect?
                    binval(value->vl_val.vl_vl,	PO_POFSCP); // Yes
                else			// No - some other psect
                {   
                    binval(value->vl_val.vl_vl, PO_POFSAP);
                    binbyte(value->vl_psect); // Output psect number
                }
            }
            binbyte(PO_STRAL);
        }
        bumpofs(6);			// Bump offset
        bnoflg = TRUE;			// Indicate binary output done
        adrflg = TRUE;			// List address on this line
    }
}

//*************************************************************************
// Function: binfin - Finish binary output and close the binary output file
// Returned: Nothing
//*************************************************************************

void binfin(void)

{
    char  *cp;
    char   chr;

    if (!test)				// Are we doing binary output
    {
        wrtblk();			// Write out last data block
        endsect();			// End the data section
        if (gsymflg)
        {
            binsyms(SC_INTERN, SYF_INTERN); // Output internal symbol defs
            binsyms(SC_EXPORT, SYF_EXPORT); // Output exported symbol defs
        }
        if (lsymflg)			// Want local symbol definitions?
            binsyms(SC_LOCAL, 0);	// Yes
        if (strpsn)			// Have starting address?
        {
            strsect(SC_STRADR);		// Yes - start the section
            binlong(stradr);		// Store starting address
            binbyte(strpsn);		// Followed by its psect number
            endsect();			// End the section
        }
        if (xdtpsn)			// Have debugger address?
        {
            strsect(SC_DEBUG);		// Yes - start the section
            binlong(xdtadr);		// Store debugger address
            binbyte(xdtpsn);		// Followed by its psect number
            endsect();			// End the section
        }
        if (stkpsn)			// Have stack address?
        {
            strsect(SC_STACK);		// Yes - start the section
            binlong(stkadr);		// Store stack address
            binbyte(stkpsn);		// Followed by its psect number
            endsect();			// End the section
        }
        while (reqhead)			// Have any .REQUIR names?
        {
            strsect(SC_FILREQ);		// Yes - start the section
            cp = reqhead->name;
            while ((chr = *cp++) != '\0')
                binbyte(chr);		// Output character
            endsect();
            reqhead = reqhead->next;
        }
        binbyte('\0');			// Ensure at least 1 zero byte at end
        if (fclose(outfp) < 0)		// Close the binary output file
            femsg(prgname, -errno, outfsp);
    }
}

//****************************************************
// Function: general - Output general non-simple value
// Returned: Nothing
//****************************************************

void general(
    struct vl *value,
    char   store)		// Polish store operator to use

{
    wrtblk();				// Write out current data block
    if (value->vl_kind == VK_ABS && value->vl_type >= VL_BREL)
					// Absolute relative value?
	binval(value->vl_val.vl_vl, PO_PVALREL); // Yes
    else if (value->vl_kind == VK_EXT)	// No - external?
    {	    				// Yes
	binval(value->vl_val.vl_vl,
                (value->vl_type>=VL_BREL)? PO_POFSRELSYM: PO_POFSSYM);
        binnumber(value->vl_pnt->sy_symnum);
    }
    else if (value->vl_kind < VK_MSC)	// Real value?
    {
        if (value->vl_psect == curpsd->pd_psn) // Relocatable - for current
					       //   psect? (note that this
					       //   cannot be relative since
					       //   self-relative references
					       //   have already been resolved)
            binval(value->vl_val.vl_vl,	PO_POFSCP); // Yes
        else				// No - some other psect
        {   
            binval(value->vl_val.vl_vl,
                    (value->vl_type>=VL_BREL)?PO_POFSRELAP:PO_POFSAP);
            binbyte(value->vl_psect);		// Output psect number
        }
    }
    else if (value->vl_kind == VK_MSC)	// Msect offset?
    {
        binbyte(PO_POFSAM);		// Yes
        binbyte(value->vl_psect);
        if (value->vl_val.vl_vl)	// Any offset?
        {
            binval(value->vl_val.vl_vl, PO_PVAL); // Yes - push it
            binbyte(PO_ADD);		// Add offset
        }
    }
    else if (value->vl_kind == VK_SEL)	// Selector?
    {
        binbyte(PO_PSELAS);		// Yes
        binbyte(value->vl_psect);
    }
    else				// Must be external selector
    {
        binbyte(PO_PSELSYM);		// Yes
        binnumber(value->vl_pnt->sy_symnum);
    }
    binbyte(store);			// Output store operator
}

//*********************************************************
// Function: chkadr - Check if address is needed in the OBJ
//	file and output it if so 
// Returned: Nothing
//*********************************************************

void chkadr(void)

{
    if (curpsd->pd_psn != binpsn || curpsd->pd_ofs != binofs)
    {   
	wrtblk();			// Need address - output current
					//  data block
	binpsn = curpsd->pd_psn;
	binofs = curpsd->pd_ofs;
	binval(binofs, '\170');		// Output address
	binbyte(binpsn);		// Output psect number
    }
}

//******************************************************
// Function: chkrel - Resolve relative value if possible
// Returned: Nothing
//******************************************************

void chkrel(
    struct vl *value)

{
    if (value->vl_kind == VK_OFS && value->vl_type >= VL_BREL &&
            value->vl_psect == curpsd->pd_psn)
    {					// Reloc value for current psect?
	value->vl_val.vl_vl -= curpsd->pd_ofs; // Yes - relocate it
	value->vl_type -= VL_BREL - VL_BYTE; // Not relocatble now
        value->vl_kind = VK_ABS;
	value->vl_psect = 0;
    }
}

//********************************
// Function: bumpofs - Bump offset
// Returned: Nothing
//********************************

void bumpofs(
    int amnt)

{
    binofs += amnt;			// Bump offset in file
    curpsd->pd_ofs += amnt;		// And bump offset in psect
    if (curpsd->pd_lsize < curpsd->pd_ofs) // Does this increase the size?
    {
        curpsd->pd_lsize = curpsd->pd_ofs; // Yes - bump size
        if (curpsd->pd_tsize < curpsd->pd_ofs)
            curpsd->pd_tsize = curpsd->pd_ofs;
    }
}

//******************************************************
// Function: bindata - Store data byte into the OBJ file
// Returned: Nothing
//******************************************************

void bindata(value)
char value;

{
    if (dbkcnt >= 119)			// Is the current data block full?
	wrtblk();			// Yes - write it out
    ++dbkcnt;				// Bump count
    *dbkpnt++ = value;			// Store value
}

//*************************************************
// Function: wrtblk - Output the current data block
// Returned: Nothing
//*************************************************

void wrtblk(void)

{
    char *pnt;
    char  byte;

    if (dbkcnt)				// Anything in the data block?
    {   
	byte = dbkcnt;
	binbyte(byte);			// Output header byte
	pnt = dbkbuf;			// Point to buffer
	dbkpnt = pnt;			// Reset store pointer
	do
	{   binbyte(*pnt++);		// Output data byte
	} while (--dbkcnt > 0);
    }
}

//*******************************************************
// Function: binsyms - Output all defined global or local
//	symbols to the OBJ file
// Returned: Nothing
//*******************************************************

void binsyms(
    char type,			// Section type
    int  fbit)			// Value for SYF_INTERN bit

{
    struct sy *sym;
    int    bits;
    char   head;

    head = FALSE;
    if ((sym = symhead) != NULL)	// Point to start of symbol list
    {
        do
        {
             if (!(sym->sy_flag & SYF_UNDEF) && 
                    !(sym->sy_flag & (SYF_EXTERN|SYF_IMPORT)) &&
                    (sym->sy_flag & (SYF_INTERN|SYF_EXPORT)) == fbit)
             {				// Is this one we want?
                if (!head)		// Have we started the section?
                {
                    strsect(type);	// No - do so now
                    head = TRUE;
                }
                bits = sym->sy_flag & (SYF_ABS|SYF_ENTRY|SYF_ADDR|SYF_SUP);
                if (!(bits & SYF_ADDR))	// Have an address?
                    bits |= SYF_ABS;	// No - always absolute!
                binval(sym->sy_val.val, bits);
                if (bits & SYF_ABS)	// Absolute?
                {
                    if (bits & SYF_ADDR) // Yes - address?
                        binword(sym->sy_psect); // Yes - output selector
                }
                else
                    binbyte(sym->sy_psect); // No - output psect number
                binxsm(sym->sy_name);	// Output name
            }
        } while ((sym = sym->sy_hash) != NULL);	// Loop for all symbols
        if (head)			// Did we do any output?
            endsect();			// Yes - end the section
    }
}

//*************************************************************************
// Function: binval - Store general variable length value into the OBJ file
// Returned: Nothing
//*****************************************************************************

void binval(
    long value,			// Value to output
    char prefix)		// High order 5 bits for prefix byte

{
    int word;

    prefix &= 0xFC;			// Remove junk from low bits
    if ((value & 0xFFFF0000L) == 0)	// Is the high word of the value 0?
    {
	if ((value & 0xFF00L) == 0)	// Yes - is the high byte of the low
	{				//   word of the value 0?
	    if (value == 0)		// Yes - is the entire value 0?
		prefix |= PX_EX0|PX_0BV; // Yes - no value needed
	    else
		prefix |= PX_EX0|PX_1BV; // No - need single byte value
	}
	else
	    prefix |= PX_EX0|PX_2BV;	// Low word not zero - need 2 byte
					//   value
    }
    else
    {
	    if ((value & 0xFFFF0000L) == 0xFFFF0000L) // Is the high word
	    {					      //   0xFFFF?
		if ((value & 0xFF00L) == 0xFF00L) // Is the high byte of the
		{				  //   low word 0xFF?
		    if (value  == 0xFFFFFFFFL)	// Is the entire value
						//   0xFFFFFFFF?
			prefix |= PX_EX1|PX_0BV; // Yes - no value is needed
		    else
			prefix |= PX_EX1|PX_1BV; // No - need 1 byte value
		}
		else
		    prefix |= PX_EX1|PX_2BV; // High byte of low word not
					     //   0xFF need 2 byte value
	    }
	    else
		prefix |= PX_4BV;	// Need full 4 byte value
    }
    binbyte(prefix);			// Output prefix byte
    switch (prefix & 0x03)
    {
    case 0:				// No value is needed
	break;
    case 1:				// One byte value is needed
	prefix = value;
	binbyte(prefix);
	break;
    case 2:				// Two byte value is needed
	word = value;
	binword(word);
	break;
    case 3:
	binlong(value);			// Four byte value is needed
	break;
    }
}

//***************************************************************
// Function: binxsm -  Store symbol name string into the OBJ file
// Returned: Nothing
//***************************************************************

void binxsm(
    char *str)

{
    int  cnt;
    char prev;
    char this;

    cnt = SYMMAXSZ -1;
    prev = *str++;
    do
    {   if ((this = *str++) != '\0')	// Get next character
	{
	    binbyte(prev & 0x7F);	// Output previous character
	    prev = this;
	}
	else
	    break;			// If at end
    } while (--cnt > 0);
    binbyte(prev | 0x80);		// Output last character
}

//*******************************************
// Function: strsect - Start OBJ file section
// Returned: Nothing
//*******************************************

void strsect(
    char type)

{
    binbyte(type);			// Output section type
    sectsize = ftell(outfp);		// Save position for section size
    binbyte('\0');			// Reserve space for size (16 bits)
    binbyte('\0');
}

//*****************************************
// Function: endsect - End OBJ file section
// Returned: Nothing
//*****************************************

void endsect(void)

{
    long place;
    int  size;

    place = ftell(outfp);		// Save current position in file
    if (fseek(outfp, sectsize, 0) < 0)	// Set to place to put section size
	femsg(prgname, -errno, outfsp);
    size = place - sectsize - 2;	// Calculate size of section
    binword(size);			// Store size in section header
    if (fseek(outfp, place, 0) < 0)	// Restore position in file
	femsg(prgname, -errno, outfsp);
}

//********************************************************
// Function: binlong - Output 32 bit value to the OBJ file
// Returned: Nothing
//********************************************************

void binlong(value)
long value;

{
    int word;

    word = value;			// Get low order word
    binword(word);			// Output it
    word = value >> 16;			// Get high order word
    binword(word);			// Output it
}

//********************************************************
// Function: binword - Output 16 bit value to the OBJ file
// Returned: Nothing
//********************************************************

void binword(
    long value)

{
    char byte;

    byte = value;			// Get low order byte
    binbyte(byte);			// Output it
    byte = value >> 8;			// Get high order byte
    binbyte(byte);			// Output it
}

//*******************************************************
// Function: binbyte - Output 8 bit value to the OBJ file
// Returned: Nothing
//*******************************************************

void binbyte(
    char value)

{
    if (putc(value, outfp) < 0)		// Output the byte
	femsg(prgname, -errno, outfsp);	// If error on output
}

//****************************************************************************
// Function: binnumber - Output number (variable length) value to the OBJ file
// Returned: Nothing
//****************************************************************************

void binnumber(
    ulong value)

{
    if (value <= 0x7FL)
        binbyte((char)value);
    else if (value <= 0x3FFFL)
    {
        binbyte(((value>>8) & 0xFF) | 0x80);
        binbyte(value & 0xFF);
    }
    else if (value <= 0x1FFFFFL)
    {
        binbyte(((value>>16) & 0xFF) | 0xC0);
        binbyte((value>>8) & 0xFF);
        binbyte(value & 0xFF);
    }
    else
    {
        binbyte(((value>>24) & 0xFF) | 0xE0);
        binbyte((value>>16) & 0xFF);
        binbyte((value>>8) & 0xFF);
        binbyte(value & 0xFF);
    }
}
