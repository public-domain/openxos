//************************************
// Routines to do first pass for XLINK
//************************************
// Written by John Goltz
//************************************

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
#include <PROCARG.H>
#include "XLINK.H"
#include "XLINKEXT.H"

// Allocate local data

long curphs;					// Used to hold phase address while setting
								//   up psects

//**********************************
// Function: passone - Do first pass
// Returned: Nothing
//**********************************

#if CFG_XLINK_MULTIPASS
 extern int		g_cUndefPrev;
 extern int		g_nPass1Lev;
#endif

void passone(void)
{
    struct objblk *obj;
    struct sy     *sym;
    struct sy     *wsym;
    struct sd     *sgd;
    struct md     *msd;
    struct pd     *psd;
    struct sy    **tblpnt;
    int    tblcnt;
    int    mscnum;

	stbsize = 10;

	if ((obj = firstobj) != NULL)		// Start with first OBJ file
	{
		do								// Process each OBJ file
		{
			fileone( obj );
		} while( (obj = obj->obj_next) != NULL );
	}

	// Note that while we do keep track of weak and lazy symbols seperately
	//   we treat them the same here!

	if (haveweak || havelazy)
	{
	    tblcnt = HASHSIZE;
	    tblpnt = hashtbl;
	    while (--tblcnt >= 0)				// Scan the hash table
	    {
			if ((sym=*tblpnt++) != NULL)
			{
				do
				{
					if ((sym->sy_flag & SYF_UNDEF) &&
							(wsym = sym->sy_weak) != NULL)
					{
						printf("### weak sym: %s --> %s\n", sym->sy_name + 1,
								wsym->sy_name + 1);

						sym->sy_flag = wsym->sy_flag;
						sym->sy_val.v = wsym->sy_val.v;
						sym->sy_sel = wsym->sy_sel;
						psd = wsym->sy_psd;
						sym->sy_psd = psd;
						if (psd->pd_slt == NULL) // Link into psect symbol list
							psd->pd_slh = sym;
						else
							psd->pd_slt->sy_next = sym;
						psd->pd_slt = sym;
						sym->sy_next = NULL;

						if ((sym->sy_flag & SYF_UNDEF) == 0) // Still undefined?
						{
				            --undefcnt;	// No - reduce count
							if (opsymsz) // Loading debugger symbols?
								resvsym();
						}
					}
				} while ((sym=sym->sy_hash) != NULL);
			}
		}
	}
	if (opsymsz)						// Want a symbol table in memory?
    {
        strcpy(&symbuf[1], "_symbols_p"); // Yes - find the psect for it
        symsize = 11;
        symbuf[0] = SYM_PSCT;			// Indicate psect name
        if ((sym=looksym()) == NULL)	// Find psect symbol table entry
            nosyms("present");			// If not there
        else
        {
            stbpsd = sym->sy_val.p;
            if (!(stbpsd->pd_flag & PA_OVER)) // It must be overlayed
                nosyms("overlayed");
            else
            {
                stbmsd = stbpsd->pd_msd;
                if (stbpsd->pd_lsize < stbsize)
                    stbpsd->pd_lsize = stbsize;
                if (stbpsd->pd_tsize < stbsize)
                    stbpsd->pd_tsize = stbsize;
            }
        }
    }

    if (exportnum)						// Have any exported symbols?
	{									// Yes - scan the list to see how much
		sym = exporthead;				//   space the exported symbol section
		do								//   will occupy
		{
			if (!(sym->sy_flag & SYF_ADDR))
				exporttotal += ((sym->sy_flag & SYF_ABS)? 2 : 1);

			exporttotal += ( 5 + strlen(sym->sy_name) );
		} while ((sym = sym->sy_export) != NULL);
    }

    offset = FHDRSIZE + 6 + seggbn*SHDRSIZE + mscgbn*MHDRSIZE + exporttotal;
    mscnum = 0;

    if ((sgd=sgdhead) != NULL)			// Scan all segments
    {
        do
        {
            sgd->sd_offset = offset;
            if ((msd=sgd->sd_head) != NULL)
            {
                do						// Scan all msects
                {
                    msd->md_num = ++mscnum;
                    msd->md_offset = offset;
                    if (ptype >= P_Z80 && ptype <= P_80286)
                    {
                        if (msd->md_next)
                        {
                            fprintf(stderr, "? XLINK: More than 1 msect in"
                                    " segment %s for processor\n"
                                    "         which does not allow this"
                                    " in file %s\n", sgd->sd_sym->sy_name+1,
                                    curobj->obj_fsp);
                            exit(1);
                        }
                        if ((msd->md_flag & MA_ADDR) && msd->md_addr != 0)
                            warnaddr(msd);
                        msd->md_flag |= MA_ADDR;
                        msd->md_addr = 0;
                    }
                    if ((psd=msd->md_head) != NULL)
                    {
                        do				// Scan all psect data blocks
                        {
                            if (psd->pd_mod != 0xFFFFFFFFL)
										// Was a modulus given?
                                msd->md_tsize = // Yes - use it
                                        ((msd->md_tsize + psd->pd_mod - 1)/
                                        psd->pd_mod) * psd->pd_mod;
                            if (psd->pd_addr != 0xFFFFFFFFL &&
                                    psd->pd_addr > msd->md_tsize)
										// Valid address given for psect?
                                msd->md_tsize = psd->pd_addr; // Yes
                            if (psd->pd_lsize) // Anything loaded in psect?
                                msd->md_lsize = msd->md_tsize; // Yes
                            psd->pd_offset = offset + msd->md_lsize;
                            psd->pd_reloc = msd->md_tsize;
                            if (msd->md_addr != 0xFFFFFFFFL)
                                psd->pd_reloc += msd->md_addr;
                            psd->pd_ofs = 0; // Clear offsets
                            psd->pd_nxo = 0;
                            msd->md_lsize += psd->pd_lsize;
                            msd->md_tsize += psd->pd_tsize;
                        } while ((psd=psd->pd_next) != NULL);
                        offset += msd->md_lsize;
                    }
                } while ((msd=msd->md_next) != NULL);
            }
        } while ((sgd=sgd->sd_next) != NULL);
    }
    rdoffset = offset;					// Remember where to put relocation
										//   data
    if ((sgd=sgdhead) != NULL)			// Scan everything again to relocate
    {									//   all symbols
        do
        {
            if ((msd=sgd->sd_head) != NULL)
            {
                do
                {
                    if ((psd=msd->md_head) != NULL)
                    {
                        do				// Scan all psect data blocks
                        {
                            if ((sym=psd->pd_slh) != NULL)
                            {			// Get first symbol for psect
                                do
                                {
                                    sym->sy_val.v += psd->pd_reloc;
										// Relocate symbol
                                } while ((sym=sym->sy_next) != NULL);
                            }
                        } while ((psd=psd->pd_next) != NULL);
                    }
                } while ((msd=msd->md_next) != NULL);
            }
        } while ((sgd=sgd->sd_next) != NULL);
    }
    if (strpsd)							// Relocate starting address if have
    {									//   one
	stradr += strpsd->pd_reloc;
        if (strpsd->pd_msd->md_addr != 0xFFFFFFFFL)
            stradr -= strpsd->pd_msd->md_addr;
    }
    if (dbgpsd)							// Relocate debugger address if have
    {									//   one
        dbgadr += dbgpsd->pd_reloc;
        if (dbgpsd->pd_msd->md_addr != 0xFFFFFFFFL)
            dbgadr -= dbgpsd->pd_msd->md_addr;
    }
    if (stkpsd)							// Relocate stack address if have one
    {
        stkadr += stkpsd->pd_reloc;
        if (stkpsd->pd_msd->md_addr != 0xFFFFFFFFL)
            stkadr -= stkpsd->pd_msd->md_addr;
    }
}
