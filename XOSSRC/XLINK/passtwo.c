//*************************************
// Routines to do second pass for XLINK
//*************************************
// Written by John Goltz
//*************************************

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
#include <PROCARG.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERRMSG.H>
#include "XLINK.H"
#include "XLINKEXT.H"

// External routines

extern char *strchr();

// Link to private external data

extern struct outbhd outhead[];
extern char   bf0sta[];
extern char   bf1sta[];
extern char   bf2sta[];
extern char   bf3sta[];


//***********************************
// Function: passtwo - Do second pass
// Returned: Nothing
//***********************************

void passtwo(void)

{
    struct objblk *obj;			// Pointer to OBJ block
    struct sd     *sgd;
    struct md     *msd;
    struct sy     *sym;
    long   rtn;

    listgbl(undefcnt, "un", SYF_UNDEF);	// List undefined globals
    listgbl(mltcnt, "multiply ", SYF_MULT); // List multiply defined globals
    if (opsymsz)						// Want a debugger symbol table?
		stboffset = stbpsd->pd_offset + 8; // Yes - point to start

    if (needrun)						// Do we need output?
    {									// Yes - allocate output buffers
		outhead[0].ob_buf = getspace((long)1024);
		outhead[0].ob_sta = bf0sta;
		outhead[1].ob_buf = getspace((long)1024);
		outhead[1].ob_sta = bf1sta;
		outhead[2].ob_buf = getspace((long)1024);
		outhead[2].ob_sta = bf2sta;
		outhead[3].ob_buf = getspace((long)1024);
		outhead[3].ob_sta = bf3sta;
        if (outfsp == NULL)				// Was a name given?
            outfsp = usefif(outext);	// No - use name of first input file
		if ((outdd=svcIoOpen(O_CREATE|O_TRUNCA|O_IN|O_OUT, outfsp, NULL)) < 0)
										// Open the output file
			femsg(prgname, outdd, outfsp); // If error
		if (auxloc == NULL)				// Make sure auxillary buffer
			auxloc = (uchar *)getspace(512L); //   is alocated
		clrbuf((long *)auxloc, 512);	// Clear it
		offset = 0;						// Output header at beginning of file
		lowfirst = (ptype < P_HIGH);	// Header is low byte first for Intel
										//   processors or high byte first
										//   for Motorolla processors
        putbyte(0xD7);					// ID bytes
        putbyte(0x22);
		putbyte(2);						// File version number
        putbyte(ptype);					// Processor type
        putbyte(itype);					// Image type
		putbyte(0);						// Not used
		putbyte(FHDRSIZE);				// Length of file header
        putbyte(seggbn);				// Number of segments
		putlong(stradr);				// Start address
        putbyte(strpsd? (strpsd->pd_msd->md_num): 0);
        putbyte(0);
        putword(0);
        putlong(dbgadr);				// Debugger address
        putbyte(dbgpsd? (dbgpsd->pd_msd->md_num): 0);
        putbyte(0);
        putword(0);
        putlong(stkadr);				// Initial stack pointer
        putbyte(stkpsd? (stkpsd->pd_msd->md_num): 0);
        putbyte(0);
        putword(0);
        putlong((exportnum != 0)? FHDRSIZE + 6 + seggbn*SHDRSIZE +
                mscgbn*MHDRSIZE: 0);
        putlong(exportnum);
        importnumoffset = offset;		// Remember where import data goes
        offset = FHDRSIZE+6;
        if ((sgd=sgdhead) != NULL)
        {
            do
            {
                putbyte(SHDRSIZE);		// Length of segment header
                putbyte(sgd->sd_msn);	// Number of msects in segment
                putbyte(sgd->sd_flag);	// Segment status byte
                putbyte((sgd->sd_linked == NULL)? // Number of linked segment
                        0: sgd->sd_linked->sd_num);
                putbyte(sgd->sd_type);	// Segment type
                putbyte(sgd->sd_priv);	// Privilege level
                putword(sgd->sd_select); // Segment selector
                putlong((sgd->sd_flag & SA_ADDR)? sgd->sd_addr: sgd->sd_mod);
										// Address or modulus
                if ((msd=sgd->sd_head) != NULL)
                {						// Scan all msects for segment
                    do
                    {
                        putbyte(MHDRSIZE); // Length of msect header
                        putbyte(0);		// Not used
                        putbyte(msd->md_flag); // Msect status byte
                        putbyte(0);		// Not used
                        putbyte(msd->md_type); // Msect type
                        putbyte(msd->md_priv); // Privledge level
                        putword(0);
                        putlong((msd->md_flag & MA_ADDR)?   // Address or
                                msd->md_addr: msd->md_mod); //   modulus?
                        putlong(msd->md_max); // Maximum segment needed
                        putlong(msd->md_tsize); // Total size of msect
                        putlong((msd->md_lsize)? msd->md_offset: 0);
										// Offset from start of file to data
                        putlong(msd->md_lsize); // Loaded size of msect
                        msd->md_rdhdros = offset;
										// Remember where to put data
										//   about the relocation data
                        putlong(0);		// Space for offset to relocation
										//   data
                        putlong(0);		// Space for size of relocation data
                    } while ((msd=msd->md_next) != NULL);
                }						// Advance to next msect in address
										//   space
            } while ((sgd=sgd->sd_next) != NULL); // Advance to next address
		}										  //   space
		if (exportnum != 0)
		{
			sym = exporthead;
			do
			{
				putbyte((sym->sy_flag & (SYF_ABS|SYF_ADDR|SYF_SUP)) | 0x03);
				putlong(sym->sy_val.v);
				if (sym->sy_flag & SYF_ADDR)
				{
					if (sym->sy_flag & SYF_ABS)
						putword(sym->sy_sel);
					else
						putbyte(sym->sy_psd->pd_msd->md_num);
				}
				outname(sym->sy_name+1);
			} while ((sym = sym->sy_export) != NULL);
		}
	}
	if (needsym)						// Want symbol table file output?
	{
		if (symfsp == NULL)
			symfsp = usefif(symext);
		if ((symdd=svcIoOpen(O_CREATE|O_TRUNCA|O_IN|O_OUT, symfsp, 0)) < 0)
										// Open the symbol table file
			femsg(prgname, symdd, symfsp); // If error
		symbufr = getspace(1024L);		// Allocate buffer
		((long *)symbufr)[0] = 0x040222D4L;
		symbpnt = symbufr + 12;
		symbcnt = 1024 - 12;
	}
	obj = (struct objblk *)(&firstobj);	// Start with first OBJ file
	while ((obj=obj->obj_next) != NULL)	// Process each OBJ file
		filetwo(obj);
	if (opsymsz)						// Storing debugger symbols?
	{
		offset = stboffset;				// Yes - end it with a dummy module
		putbyte(0);						//   entry
		putbyte(DBF_MOD);
		offset = stbpsd->pd_offset;		// Put header data in table
		lowfirst = opsymord;
		putlong(stbnum);
		putlong(stbxsp);
	}
    if (outdd)							// If doing output, scan all msects
    {									//   and output all relocation data
        offset = rdoffset;

        if (importnum != 0)				// Have any imported symbols?
        {								// Yes - output the imported symbol
            importnum = 0;				//   table
            importos = offset;
            sym = importhead;
            do
            {
                if (sym->sy_flag & SYF_USED) // Need this one?
                {						// Yes
                    putbyte(0);			// Output header byte
                    outname(sym->sy_name+1); // Output the symbol name
                    sym->sy_val.v = ++importnum;
                }
            } while ((sym = sym->sy_next) != NULL);
        }

        if ((sgd=sgdhead) != NULL)
        {
            do
            {
                if ((msd=sgd->sd_head) != NULL)
                {						// Scan all msects for address
                    do					//   space
                    {
                        relocout(msd);	// Output relocation data for msect
                    } while ((msd=msd->md_next)!= NULL);
                }						// Advance to next msect in address
										//   space
            } while ((sgd=sgd->sd_next) != NULL); // Advance to next address
        }					  			//   space

        if (importnum)
        {
            offset = importnumoffset;
            putlong(importos);
            putlong(importnum);
        }

        if ((sgd=sgdhead) != NULL)		// Scan all msects again and fix up
        {								//   header data if necessary (we do
            do							//   this in another pass to minimize
            {							//   paging in the output file)
                if ((msd=sgd->sd_head) != NULL)
                {						// Scan all msects for segment
                    do
                    {
                        if (msd->md_rdoffset)
                        {
                            offset = msd->md_rdhdros;
                            putlong((msd->md_rdcnt)? msd->md_rdoffset: 0l);
                            putlong(msd->md_rdcnt);
                        }
                    } while ((msd=msd->md_next) != NULL);
                }						// Advance to next msect in address
										//   space
            } while ((sgd=sgd->sd_next) != NULL); // Advance to next address
        }										  //   space
        outclose();						// Close the output file
    }
    if (symdd)							// Outputting symbol file?
    {
        symbcnt = 1024 - symbcnt;		// Yes - get amount in buffer now
        if (symblk != 0)				// More than one buffer full?
        {								// Yes - output last buffer full
            if (((rtn=svcIoOutBlock(symdd, symbufr, symbcnt)) < 0) ||
					((rtn=svcIoSetPos(symdd, 0L, 0)) < 0) ||
					((rtn=svcIoInBlock(symdd, symbufr, 512)) < 0))
										// Get first block again
                femsg(prgname, rtn, symfsp); // If error
            symbcnt = 512;
        }
        ((long *)symbufr)[1] = symlength; // Store length of symbol table
        ((long *)symbufr)[2] = symtotal; // Store total number of symbols
        if (((rtn=svcIoSetPos(symdd, 0L, 0)) < 0) ||
				((rtn=svcIoOutBlock(symdd, symbufr, symbcnt)) < 0))
										// Output updated first block
            femsg(prgname, rtn, symfsp); // If error
        if ((rtn=svcIoClose(symdd, 0)) < 0) // Close the symbol table file
            femsg(prgname, rtn, symfsp);
    }
}


//*************************************************************
// Function: relocout - Output relocation data for single msect
// Returned: Nothing
//*************************************************************

void relocout(
    struct md *msd)

{
    struct rd *thisrld;
    struct rd *prevrld;
    struct rd *nextrld;
    ulong  prevloc;
    ulong  loc;
    int    type;
    char   done;

    prevloc = 0;
    msd->md_rdoffset = offset;
    if (msd->md_rdhead != NULL)
    {
        do				// First sort the relocation list
        {
            done = TRUE;
            prevrld = (struct rd *)(&(msd->md_rdhead));
            thisrld = msd->md_rdhead;
            nextrld = thisrld->rd_next;
            while (nextrld)
            {
                if (thisrld->rd_loc > nextrld->rd_loc)
                {
                    prevrld->rd_next = nextrld;
                    thisrld->rd_next = nextrld->rd_next;
                    nextrld->rd_next = thisrld;
                    nextrld = thisrld;
                    thisrld = prevrld->rd_next;
                    done = FALSE;
                }
                else
                {
                    prevrld = thisrld;
                    thisrld = nextrld;
                    nextrld = thisrld->rd_next;
                }
            }
        } while (!done);

        thisrld = msd->md_rdhead;
        prevloc = 0;
        do
        {
            loc = thisrld->rd_loc - prevloc;
            prevloc = thisrld->rd_loc;
            type = thisrld->rd_type;	// Get relocation type and kind
            if (loc > 0xFFFFFFL)	// Add in the length of the
                type |= 3;		//   delta offset field
            else if (loc > 0xFFFFL)
                type |= 2;
            else if (loc > 0xFFL)
                type |= 1;
            putbyte(type);
            if ((type&0x0C) == RT_SYMBOL)
                putvarval(thisrld->rd_data.s->sy_val.v);
            else if ((type&0x0C) != RT_NONE)
                putbyte(thisrld->rd_data.n);
            switch(type & 0x03)		//   number
            {				// Output delta offset field
             case 0:
                putbyte(loc);
                break;
             case 1:
                putword(loc);
                break;
             case 2:
                if (lowfirst)
                {
                    putbyte(loc);
                    putword(loc>>8);
                }
                else
                {
                    putword(loc>>8);
                    putbyte(loc);
                }
                break;
             default:
                putlong(loc);
                break;
            }
            ++(msd->md_rdcnt);
        } while ((thisrld=thisrld->rd_next) != NULL);
    }
}
