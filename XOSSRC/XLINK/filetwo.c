//*******************************************************
// Routines to read single file for second pass for XLINK                                     */
//*******************************************************
// Written by John Goltz
//*******************************************************

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
#include <XOSSTR.H>
#include "XLINK.H"
#include "XLINKEXT.H"

long   symval;
struct md *symmsd;
struct pd *sympsd;
struct sy *sympnt;
char   symabs;

//****************************
// Section type dispatch table
//****************************

void (*dsptwo[])(void) =
{
    twoeom,				// SC_EOM     = 0  End of module
    (void (*)(void))nullfunc,
						// SC_INDEX   = 1  Library index section
    twostart,			// SC_START   = 2  Start of module section
    (void (*)(void))nullfunc,
						// SC_SEG     = 3  Segment definition section
    (void (*)(void))nullfunc,
						// SC_MSECT   = 4  Msect definition section
    twopsect,			// SC_PSECT   = 5  Psect definition section
    twodata,			// SC_DATA    = 6  Data section
    twointern,			// SC_INTERN  = 7  Internal symbol definition section
    twolocal,			// SC_LOCAL   = 8  Local symbol definition section
    twointern,			// SC_EXPORT  = 9  Exported symbol definition section
    twostradr,			// SC_STRADR  = 10 Starting address section
    twodebug,			// SC_DEBUG   = 11 Debugger address section
    twostack,			// SC_STACK   = 12 Initial stack address section
    (void (*)(void))nullfunc,
						// SC_FILEREQ = 13 File request section
    (void (*)(void))nullfunc,
						// SC_EXTERN  = 14 External symbol definition section
    badsec,				// SC_SHARED  = 15 Shared library definition section
    twoimport,			// SC_IMPORT  = 16 Imported symbol definition section
    badsec,				//            = 17 Illegal
    badsec,				//            = 17 Illegal
    badsec,				//            = 19 Illegal
    twoeom,				// SC_ENTRY   = 20 Entry list section
    badsec,				// SC_BAD     = 21 Illegal MS section type
    ms2mtheadr,			// SC_MTHEADR = 22 (0x80) Module header
    ms2mcoment,			// SC_MCOMENT = 23 (0x88) Comment
    ms2mmodend,			// SC_MMODEND = 24 (0x8A, 0x8B) Module end
    (void (*)(void))nullfunc,
						// SC_MEXTDEF = 25 (0x8C) External definition
    ms2mtypdef,			// SC_MTYPDEF = 26 (0x8E) Type definition
    ms2mpubdef,			// SC_MPUBDEF = 27 (0x90, 0x91) Public definition
    ms2mlocsym,			// SC_MLOCSYM = 28 (0x92, 0x93) Local symbols
    ms2mlinnum,			// SC_MLINNUM = 29 (0x94, 0x95) Source line number
    (void (*)(void))nullfunc,
						// SC_MLNAMES = 30 (0x96) Name list
    ms2msegdef,			// SC_MSEGDEF = 31 (0x98, 0x99) Segment definition
    (void (*)(void))nullfunc,
						// SC_MGRPDEF = 32 (0x9A) Group definition
    ms2mfixupp,			// SC_MFIXUPP = 33 (0x9C, 0x9D) Fix up previous data
						//		     image
    ms2mlxdata,			// SC_MLEDATA = 34 (0xA0, 0xA1) Logical data image
    ms2mlxdata,			// SC_MLIDATA = 35 (0xA2, 0xA3) Logical repeated
						//		     (iterated) data image
    ms2mcomdef,			// SC_MCOMDEF = 36 (0xB0) Communal names definition
    (void (*)(void))nullfunc,
						// SC_MLOCEXD = 37 (0xB4) External definition visible
						//		     within module only
    ms2mflcexd,			// SC_MFLCEXD = 38 (0xB5) Func ext definition visible
						//		     within module only
    ms2mpubdef,			// SC_MLOCPUB = 39 (0xB6, 0xB7) Public definition
						//		     visible within module only
    ms2mcomdef,			// SC_MLOCCOM = 40 (0xB8) Communal name visible
						//	    	     within moduld only
    ms2mcomdat			// SC_MCOMDAT = 41 (0xC2, 0xC3) Initialized communal
};						//		     data


//********************************
// Polish item type dispatch table
//********************************

void (*twoddsp[])(int) =
{
    polopr,			// 10000xxx - Arithmetic operators
    polopr,			// 10001xxx - Arithmetic operators
    polopr,			// 10010xxx - Store operators
    polopr,			// 10011xxx - Store operators
    polpav,			// 10100xxx - Push absolute value
    polpar,			// 10101xxx - Push relative absolute value
    polrac,			// 10110xxx - Push offset value for current
				//		psect
    (void (*)(int))badfmt,	// 10111xxx - Illegal
    polraa,			// 11000xxx - Push offset value for any psect
    polrra,			// 11001xxx - Push offset value, relative for
				//		any psect
    (void (*)(int))badfmt,	// 11010xxx - Illegal
    (void (*)(int))badfmt,	// 11011xxx - Illegal
    (void (*)(int))badfmt,	// 11100xxx - Illegal
    (void (*)(int))badfmt,	// 11101xxx - Illegal
    polexan,			// 11110xxx - Push symbol offset value
    polexrn			// 11111xxx - Push symbol offset value, relative
};

//****************************************************
// Polish arithmetic and store operator dispatch table
//****************************************************

void (*oprdsp[]) (void) =
{
    athadd,		// 10000000 - Add
    athsub,		// 10000001 - Subtract
    athmul,		// 10000010 - Multiply
    athdiv,		// 10000011 - Divide
    athsft,		// 10000100 - Shift
    athand,		// 10000101 - And
    athior,		// 10000110 - Inclusive or
    athxor,		// 10000111 - Exclusive or
    athcom,		// 10001000 - Complement
    athneg,		// 10001001 - Negate
    badfmt,		// 10001010 - Illegal
    badfmt,		// 10001011 - Illegal
    badfmt,		// 10001100 - Illegal
    badfmt,		// 10001101 - Illegal
    badfmt,		// 10001110 - Illegal
    badfmt,		// 10001111 - Illegal
    badfmt,		// 10010000 - Illegal
    badfmt,		// 10010001 - Illegal
    polsscp,		// 10010010 - Push segment selector for current psect
    polssap,		// 10010011 - Push segment selector for any psect,
			//		psect number follows
    polssam,		// 10010100 - Push segment selector for any msect,
			//		msect number follows
    polssas,		// 10010101 - Push segment selector for any sesgment,
			//		segment number follows
    polosam,		// 10010110 - Push offset for any msect, msect number
			//		follows
    polssexn,		// 10010111 - Push selector value for symbol
    polstraddrw,	// 10011000 - Store word address
    polstraddrl,	// 10011001 - Store long address
    polstrub,		// 10011010 - Store unsigned byte
    polstruw,		// 10011011 - Store unsigned word
    polstrl,		// 10011100 - Store unsigned long
    polstrsb,		// 10011101 - Store signed byte
    polstrsw,		// 10011110 - Store signed word
    polstrl		// 10011111 - Store signed long
};

//********************************************************
// Function: filetwo - Process single file for second pass
// Returned: Nothing
//********************************************************

void filetwo(
    struct objblk *obj)		// Pointer to OBJ block

{
#if CFG_XLINK_DEBUG
	fprintf( stdout, "->filetwo(obj) obj_mdl=%08X obj_fsp=[%.40s]\n"
				, (int)obj->obj_mdl, obj->obj_fsp );
#endif

	if( obj->obj_mdl == (struct modblk *)(-1) ) // Nothing to do if this is a
		return;				      				//   library file we don't need

	opnobj( obj );								// Open the OBJ file
	if( !libflag )								// Library file?
	{											// No - just one module
		curmod = obj->obj_mdl;
		moduletwo();
	}
    else
    {
		if( (curmod = obj->obj_mdl) != NULL )	// Yes - scan module list
		{
			do
			{
				setblk(curmod->mb_mod);			// Setup to read module
				moduletwo();					// Load the module
			} while( (curmod = curmod->mb_next) != NULL );
		}
	}
	closeobj();				// Close the input file
}

//********************************************************************
// Function: moduletwo - Process single program module for second pass
// Returned: Nothing
//********************************************************************

void moduletwo(void)

{
    setobj();
    psnumpnt = curmod->mb_psectnumtbl;
    psnumcnt = 0;
    msnumpnt = curmod->mb_msectnumtbl;
    msnumcnt = 0;
    segnumpnt = curmod->mb_segnumtbl;
    segnumcnt = 0;
    done = FALSE;
    do
    {
        getsec();
        if (debugflag)
            printf("p2: mod=0x%02.2X, size=%d\n", sectype, seccnt);
        (*dsptwo[sectype])();		// Dispatch on section type
    } while (!done);
}

//*************************************************
// Function: twoeom - Process end of module section
// Returned: Nothing
//*************************************************

void twoeom(void)

{
    struct sd *sgd;
    struct md *msd;
    struct pd *psd;

    if ((sgd=sgdhead) != NULL)		// Scan all segments (if any)
    {
        do
        {   if ((msd=sgd->sd_head) != NULL) // Scan all msects (if any)
            {
                do
                {   if ((psd=msd->md_head) != NULL)
                    {			// Scan all psects (if any)
                        do
                        {   if (!(psd->pd_flag & PA_OVER)) // Overlayed?
                                psd->pd_ofs = psd->pd_nxo;    // No - update
                        } while ((psd=psd->pd_next) != NULL); //   offset
                    }
                } while ((msd=msd->md_next) != NULL);
            }
        } while ((sgd=sgd->sd_next) != NULL);
    }
    done = TRUE;			// Indicate finished with file
    modname[0] = 0;			// No current module now
}

//*****************************************************
// Function: twostart - Process start of module section
// Returned: Nothing
//*****************************************************

void twostart(void)

{
    getsym();				// Get module name
    if (opsymsz || symdd)		// Loading debugger symbol table?
	putinsym(NULL, NULL, 0, 0, DBF_MOD); // Yes - put in symbol table
    strmov(modname, symbuf+1);		// Save module name
}

//******************************************************
// Function: twopsect - Process psect definition section
// Returned: Nothing
//******************************************************

void twopsect(void)

{
    struct pd *psd;
    long   size;

    while (seccnt > 0)
    {
        if (++psnumcnt > curmod->mb_psectnummax)
            fail("More psects encountered in pass 2 than in pass 1");
        psd = *psnumpnt++;
        size = getitem(getbyte());
        if (!(psd->pd_flag & PA_OVER))
            psd->pd_nxo = psd->pd_ofs + size;
					// Calculate offset for next file
        getitem(getbyte());		// Yes - discard loaded size of psect
        getitem(getbyte());		// Discard address or modulus
        getbyte();			// Discard msect number
        getsym();			// Get name of psect
    }
}

//*****************************************
// Function: twodata - Process data section
// Returned: Nothing
//*****************************************

void twodata(void)

{
    int header;

    polspnt = polstk + POLSSIZE;	// Reset polish stack pointer
    while (seccnt != 0)
    {
	if ((header=getbyte()) & 0x80)	// Get item header byte
	    (*twoddsp[(header>>3)&0x0F])(header);// If polish item
	else
	{
	    if (header >= 0x78)		// Is this an address?
	    {
		offset = getitem(header); // Yes - get address
		curpsd = getpsd(getbyte());
                curmsd = curpsd->pd_msd;
                cursgd = curmsd->md_sgd;
		offset += curpsd->pd_offset + curpsd->pd_ofs;
            }				// Relocate address
	    else
		while (--header >= 0)	// Load absolute data
		    putbyte(getbyte());
	}
    }
    if (polspnt != polstk + POLSSIZE)
        fail("Invalid expression");
}

//*****************************************************************
// Function: polopr - Process polish arithmetic and store operators
// Returned: Nothing
//*****************************************************************

void polopr(
    int header)

{
    (*oprdsp[header&0x1F])();		// Dispatch to arithmetic routine
}

//***********************************************
// Function: athadd - Process add polish operator
// Returned: Nothing
//***********************************************

void athadd(void)

{
    int    type1;
    long   value1;
    struct md *pnt1;

    if (polspnt >= (polstk + (POLSSIZE-1))) // Check for stack underflow
        underflow();
    else
    {
        type1 = polspnt->ps_type;	// Get first item off of the stack
        value1 = polspnt->ps_value;
        pnt1 = polspnt->ps_pnt.m;
        ++polspnt;			// Pop the stack pointer
        if (type1 != PT_ABSOLUTE && polspnt->ps_type != PT_ABSOLUTE)
            badrexp();			// At least one must be absolute
        if (type1 != PT_ABSOLUTE)	// Use right type
        {
            polspnt->ps_type = type1;
            polspnt->ps_pnt.m = pnt1;
        }
        polspnt->ps_value += value1;	// Do the add
    }
}

//****************************************************
// Function: athsub - Process subtract polish operator
// Returned: Nothing
//****************************************************

void athsub(void)

{
    int    type1;
    long   value1;
    struct md *pnt1;

    if (polspnt >= (polstk + (POLSSIZE-1))) // Check for stack underflow
        underflow();
    else
    {
        type1 = polspnt->ps_type;	// Get first item off of the stack
        value1 = polspnt->ps_value;
        pnt1 = polspnt->ps_pnt.m;
        ++polspnt;			// Pop the stack pointer
        if (polspnt->ps_type != PT_ABSOLUTE && (polspnt->ps_type != type1 ||
                polspnt->ps_pnt.m != pnt1))
            badrexp();
        if (polspnt->ps_type == PT_ABSOLUTE)
        {
            polspnt->ps_type = type1;
            polspnt->ps_pnt.m = pnt1;
        }
        else
            polspnt->ps_type = PT_ABSOLUTE;
        polspnt->ps_value = value1 - polspnt->ps_value;
    }
}

//****************************************************
// Function: athmul - Process multiply polish operator
// Returned: Nothing
//****************************************************

void athmul(void)

{
    int  type1;
    long value1;

    if (polspnt >= (polstk + (POLSSIZE-1))) // Check for stack underflow
        underflow();
    else
    {
        type1 = polspnt->ps_type;	// Get first item off of the stack
        value1 = polspnt->ps_value;
        ++polspnt;			// Pop the stack pointer
        if (type1 != PT_ABSOLUTE || polspnt->ps_type != PT_ABSOLUTE)
            badrexp();			// Both must be absolute
        polspnt->ps_value *= value1;	// Do the multiply
    }
}

//**************************************************
// Function: athdiv - Process divide polish operator
// Returned: Nothing
//**************************************************

void athdiv(void)

{
    int  type1;
    long value1;

    if (polspnt >= (polstk + (POLSSIZE-1))) // Check for stack underflow
        underflow();
    else
    {
        type1 = polspnt->ps_type;	// Get first item off of the stack
        value1 = polspnt->ps_value;
        ++polspnt;			// Pop the stack pointer
        if (type1 != PT_ABSOLUTE || polspnt->ps_type != PT_ABSOLUTE)
            badrexp();			// Both must be absolute
        polspnt->ps_value = value1 / polspnt->ps_value;
    }
}

//*************************************************
// Function: athsft - Process shift polish operator
// Returned: Nothing
//*************************************************

void athsft(void)

{
    int  type1;
    long value1;

    if (polspnt >= (polstk + (POLSSIZE-1))) // Check for stack underflow
        underflow();
    else
    {
        type1 = polspnt->ps_type;	// Get first item off of the stack
        value1 = polspnt->ps_value;
        ++polspnt;			// Pop the stack pointer
        if (type1 != PT_ABSOLUTE || polspnt->ps_type != PT_ABSOLUTE)
            badrexp();			// Both must be absolute
        if (value1 >= 0)
            polspnt->ps_value <<= value1; // Shift left
        else
            polspnt->ps_value >>= (-value1); // Shift right
    }
}

//***********************************************
// Function: athand - Process and polish operator
// Returned: Nothing
//***********************************************

void athand(void)

{
    int  type1;
    long value1;

    if (polspnt >= (polstk + (POLSSIZE-1))) // Check for stack underflow
        underflow();
    else
    {
        type1 = polspnt->ps_type;	// Get first item off of the stack
        value1 = polspnt->ps_value;
        ++polspnt;			// Pop the stack pointer
        if (type1 != PT_ABSOLUTE || polspnt->ps_type != PT_ABSOLUTE)
            badrexp();			// Both must be absolute
        polspnt->ps_value &= value1;	// Do the and
    }
}

//********************************************************
// Function: athior - Process inclusive or polish operator
// Returned: Nothing
//********************************************************

void athior(void)

{
    int  type1;
    long value1;

    if (polspnt >= (polstk + (POLSSIZE-1))) // Check for stack underflow
        underflow();
    else
    {
        type1 = polspnt->ps_type;	// Get first item off of the stack
        value1 = polspnt->ps_value;
        ++polspnt;			// Pop the stack pointer
        if (type1 != PT_ABSOLUTE || polspnt->ps_type != PT_ABSOLUTE)
            badrexp();			// Both must be absolute
        polspnt->ps_value |= value1;	// Do the inclusive or
    }
}

//********************************************************
// Function: athxor - Process exclusive or polish operator
// Returned: Nothing
//********************************************************

void athxor(void)

{
    int  type1;
    long value1;

    if (polspnt >= (polstk + (POLSSIZE-1))) // Check for stack underflow
        underflow();
    else
    {
        type1 = polspnt->ps_type;	// Get first item off of the stack
        value1 = polspnt->ps_value;
        ++polspnt;			// Pop the stack pointer
        if (type1 != PT_ABSOLUTE || polspnt->ps_type != PT_ABSOLUTE)
            badrexp();			// Both must be absolute
        polspnt->ps_value ^= value1;	// Do the exclusive or
    }
}

//******************************************************
// Function: athcom - Process complement polish operator
// Returned: Nothing
//******************************************************

void athcom(void)

{
    if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
        underflow();
    else
    {
        if (polspnt->ps_type != PT_ABSOLUTE) // Must be absolute
            badrexp();
        polspnt->ps_value = ~polspnt->ps_value; // Do the complement
    }
}

//**************************************************
// Function: athneg - Process negate polish operator
// Returned: Nothing
//**************************************************

void athneg(void)

{
    if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
        underflow();
    else
    {
        if (polspnt->ps_type != PT_ABSOLUTE) // Must be absolute
            badrexp();
        polspnt->ps_value = -polspnt->ps_value; // Do the negate
    }
}

//***************************************************************
// Function: posssexn - Process push selector for external symbol
// Returned: Nothing
//***************************************************************

void polssexn(void)

{
    symnum();				// Get symbol number
    if (symmsd == NULL)
    {
        if (sympnt->sy_flag & SYF_IMPORT)
            polpush(0, 0, PT_SELSYM, sympnt);
        else
        {
            if (sympnt->sy_sel == 0 && !(sympnt->sy_flag & SYF_UNDEF))
                nosymseg();		// Complain if no segment defined
            polpush((ulong)(sympnt->sy_sel), 0, PT_ABSOLUTE, NULL);
        }
    }
    else
        segpush(symmsd->md_sgd);
}

//********************************************************************
// Function: polsscp - Process push segment selector for current psect
// Returned: Nothing
//********************************************************************

void polsscp(void)

{
    segpush(cursgd);
}

//****************************************************************
// Function: polssap - Process push segment selector for any psect
// Returned: Nothing
//****************************************************************

void polssap(void)

{
    segpush(getpsd(getbyte())->pd_msd->md_sgd);
}

//*****************************************************************
// Function: polsssam - Process push segment selector for any msect
// Returned: Nothing
//*****************************************************************

void polssam(void)

{
    segpush(getmsd(getbyte())->md_sgd);
}

//******************************************************************
// Function: polssas - Process push segment selector for any segment
// Returned: Nothing
//******************************************************************

void polssas(void)

{
    segpush(getsgd(getbyte()));
}

//******************************************************
// Function: polosam - Process push offset for any msect
// Returned: Nothing
//******************************************************

void polosam(void)

{
    struct md *msd;

    msd = getmsd(getbyte());
    if (msd->md_flag & MA_ADDR)		// Address specified for this msect?
        polpush(msd->md_addr, msd->md_sgd->sd_select, PT_ABSOLUTE, NULL); // Yes
    else
        polpush(0, 0, PT_ABSOFSMS, msd); // No
}

//************************************************************
// Function: polstraddrw - Process store word address operator
// Returned: Nothing
//************************************************************

void polstraddrw(void)

{
    if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
        underflow();
    else
    {
        if (polspnt->ps_type == PT_ABSOLUTE) // Check for truncation if should
        {
            if (polspnt->ps_value & 0xFFFF0000L)
                truncate(16);
            putword(polspnt->ps_value);
            putword(polspnt->ps_select);
        }
        else
        {
            if (polspnt->ps_type == PT_ABSOFSMS)
            {
                if (polspnt->ps_pnt.m->md_flag & MA_ADDR)
                {
                    polspnt->ps_type = PT_SELMS;
                    polspnt->ps_pnt.s = polspnt->ps_pnt.m->md_sgd;
                    offset += 2;
                    putreld(polspnt, 16);
                    offset -= 2;
                }
                else
                {
                    polspnt->ps_type = PT_ADDRMS;
                    putreld(polspnt, 16);
                }
            }
            else if (polspnt->ps_type == PT_ABSOFSSYM)
            {
                polspnt->ps_type = PT_ADDRSYM;
                putreld(polspnt, 16);
            }
            else
                badfmt();
            putword(polspnt->ps_value);
            putword(0);
        }
        ++polspnt;
    }
}

//************************************************************
// Function: polstraddrl - Process store long address operator
// Returned: Nothing
//************************************************************

void polstraddrl(void)

{
    if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
        underflow();
    else
    {
        if (polspnt->ps_type == PT_ABSOLUTE)
        {
            putlong(polspnt->ps_value);
            putword(polspnt->ps_select);
        }
        else
        {
            if (polspnt->ps_type == PT_ABSOFSMS)
            {
                if (polspnt->ps_pnt.m->md_flag & MA_ADDR)
                {
                    polspnt->ps_type = PT_SELMS;
                    polspnt->ps_pnt.s = polspnt->ps_pnt.m->md_sgd;
                    offset += 4;
                    putreld(polspnt, 16);
                    offset -= 4;
                }
                else
                {
                    polspnt->ps_type = PT_ADDRMS;
                    putreld(polspnt, 32);
                }
            }
            else if (polspnt->ps_type == PT_ABSOFSSYM)
            {
                polspnt->ps_type = PT_ADDRSYM;
                putreld(polspnt, 32);
            }
            else
                badfmt();
            putlong(polspnt->ps_value);
            putword(0);
        }
        ++polspnt;
    }
}

//*********************************************************
// Function: polstub - Process store unsigned byte operator
// Returned: Nothing
//*********************************************************

void polstrub(void)

{
    if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
        underflow();
    else
    {
        if (polspnt->ps_type == PT_ABSOLUTE || // Check for truncation if should
                (polspnt->ps_type == PT_ABSOFSMS &&
                (polspnt->ps_pnt.m->md_flag & MA_ADDR)))
        {
            if ((polspnt->ps_value & 0xFFFFFF00L) &&
                    (polspnt->ps_value & 0xFFFFFF80L) != 0xFFFFFF80L)
                truncate(8);
        }
        else
            putreld(polspnt, 8);
        putbyte(polspnt->ps_value);	// Store byte
        ++polspnt;
    }
}

//*********************************************************
// Function: polstuw - Process store unsigned word operator
// Returned: Nothing
//*********************************************************

void polstruw(void)

{
    if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
        underflow();
    else
    {
        if (polspnt->ps_type == PT_ABSOLUTE || // Check for truncation if should
                (polspnt->ps_type == PT_ABSOFSMS &&
                (polspnt->ps_pnt.m->md_flag & MA_ADDR)))
        {
            if ((polspnt->ps_value & 0xFFFF0000L) &&
                    (polspnt->ps_value & 0xFFFF8000L) != 0xFFFF8000L)
                truncate(16);
        }
        else
            putreld(polspnt, 16);
        putword(polspnt->ps_value);	// Store word
        ++polspnt;
    }
}

//*******************************************************
// Function: polstsb - Process store signed byte operator
// Returned: Nothing
//*******************************************************

void polstrsb(void)

{
    if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
        underflow();
    else
    {
        if (polspnt->ps_type == PT_ABSOLUTE ||
                (polspnt->ps_type == PT_ABSOFSMS &&
                (polspnt->ps_pnt.m->md_flag & MA_ADDR)))
        {
            if (polspnt->ps_value & 0x80) // Check for truncation
            {
                if ((polspnt->ps_value & 0xFFFFFF00L) != 0xFFFFFF00L)
                    truncate(8);
            }
            else
            {
                if (polspnt->ps_value & 0xFFFFFF00L)
                    truncate(8);
            }
        }
        else
            putreld(polspnt, 8);
        putbyte(polspnt->ps_value);	// Store byte
        ++polspnt;
    }
}

//*******************************************************
// Function: polstsw - Process store signed word operator
// Returned: Nothing
//*******************************************************

void polstrsw(void)

{
    if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
        underflow();
    else
    {
        if (polspnt->ps_type == PT_ABSOLUTE ||
                (polspnt->ps_type == PT_ABSOFSMS &&
                (polspnt->ps_pnt.m->md_flag & MA_ADDR)))
        {
            if (polspnt->ps_value & 0x8000) // Check for truncation
            {
                if ((polspnt->ps_value & 0xFFFF0000L) != 0xFFFF0000L)
                    truncate(16);
            }
            else
            {
                if (polspnt->ps_value & 0xFFFF0000L)
                    truncate(16);
            }
        }
        else
            putreld(polspnt, 16);
        putword(polspnt->ps_value);	// Store word
        ++polspnt;
    }
}

//***********************************************
// Function: polstl - Process store long operator
// Returned: Nothing
//***********************************************

void polstrl(void)

{
    if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
        underflow();
    else
    {
        if (polspnt->ps_type != PT_ABSOLUTE &&
                !(polspnt->ps_type == PT_ABSOFSMS &&
                (polspnt->ps_pnt.m->md_flag & MA_ADDR)))
            putreld(polspnt, 32);
        putlong(polspnt->ps_value);
        ++polspnt;
    }
}

//********************************************************
// Function: polpav - Process push absolute value operator
// Returned: Nothing
//********************************************************

void polpav(
    int header)

{
    polpush(getitem(header), 0, PT_ABSOLUTE, NULL); // Push value onto the stack
}

//*****************************************************************
// Function: polpar - Process push absolute relative value operator
// Returned: Nothing
//*****************************************************************

void polpar(
    int header)

{
    polpush(getitem(header) - (offset - curpsd->pd_offset + curpsd->pd_reloc),
            0, ((curmsd->md_flag & MA_ADDR)? PT_RELATIVE: PT_RELOFSMS),
            (struct md *)&abspsd);
}

//***************************************************************************
// Function: polrac - Process push relocated value for current psect operator
// Returned: Nothing
//***************************************************************************

void polrac(
    int header)

{
    checkcur();
    polpush(getitem(header) + curpsd->pd_reloc + curpsd->pd_ofs, 0,
            (curmsd->md_flag & MA_ADDR)? PT_ABSOLUTE: PT_ABSOFSMS, curmsd);
}

//***********************************************************************
// Function: polrra - Process push relocated value for any psect operator
// Returned: Nothing
//***********************************************************************

void polraa(
    int header)

{
    long   value;
    struct pd *psd;

    value = getitem(header);
    psd = getpsd(getbyte());

    polpush(value + psd->pd_reloc + psd->pd_ofs, psd->pd_msd->md_sgd->sd_select,
            (psd->pd_msd->md_flag & MA_ADDR)? PT_ABSOLUTE: PT_ABSOFSMS,
            psd->pd_msd);
}

//*****************************************************************
// Function: polrra - process push relocated relative value for any
//		psect operator
// Returned: Nothing
//*****************************************************************

void polrra(
    int header)

{
    long   value;
    struct pd *psd;
    struct sd *sgd;

    value = getitem(header);
    psd = getpsd(getbyte());
    sgd = psd->pd_msd->md_sgd;
    if (sgd != cursgd && (sgd->sd_select != cursgd->sd_select ||
            sgd->sd_select == 0))	// In same segment?
    {
        diffspace(NULL);		      // No - complain (value pushed is
	polpush(value, 0, PT_RELATIVE, NULL); //   nonsense in this case!)
    }
    else
        polpush(value + psd->pd_reloc + psd->pd_ofs
                - (offset - curpsd->pd_offset + curpsd->pd_reloc),
                psd->pd_msd->md_sgd->sd_select,
                (((psd->pd_msd == curmsd) || ((psd->pd_msd->md_flag&MA_ADDR) &&
                (curmsd->md_flag & MA_ADDR)))? PT_ABSOLUTE: PT_RELOFSMS),
                psd->pd_msd);
}

//********************************************************
// Function: polexan - Process push symbol offset operator
// Returned: Nothing
//********************************************************

void polexan(
    int header)

{
    long value;

    value = getitem(header);
    symnum();
    symval += value;
    if (sympnt->sy_flag & SYF_UNDEF)
        polpush(0, 0, PT_ABSOLUTE, NULL);
    else
        polpush(symval,
                (sympnt->sy_flag & SYF_IMPORT)? 0:
                    ((sympnt->sy_flag & SYF_ABS)? sympnt->sy_sel:
                    symmsd->md_sgd->sd_select),
                (sympnt->sy_flag & SYF_IMPORT)? PT_ABSOFSSYM:
                    (((symmsd == NULL) || ((symmsd->md_flag & MA_ADDR) &&
                    (symmsd->md_sgd->sd_select != 0)))? PT_ABSOLUTE:
                    PT_ABSOFSMS),
                (sympnt->sy_flag & SYF_IMPORT)? (void *)sympnt: (void *)symmsd);
}

//*******************************************************************
// Function: polexrn - Process push symbol offset (relative) operator
// Returned: Nothing
//*******************************************************************

void polexrn(
    int header)

{
    long   value;
    struct sd *sgd;

    value = getitem(header);
    symnum();
    symval += value;
    if (sympnt->sy_flag & SYF_UNDEF)
        polpush(0, 0, PT_RELATIVE, NULL);
    else if (sympnt->sy_flag & SYF_IMPORT) // Imported symbol?
        polpush(symval, 0, PT_RELOFSSYM, sympnt); // Yes
    else				// No
    {
        if (symmsd != NULL)
        {
            sgd = symmsd->md_sgd;
            if (sgd != cursgd && (sgd->sd_select != cursgd->sd_select ||
                    sgd->sd_select == 0)) // In same segment?
            {
                if (!(sympnt->sy_flag & SYF_UNDEF)) // No
                    diffspace(sympnt);	// Complain unless undefined
                polpush(symval, 0, PT_RELATIVE, NULL); // Push a value we can
            }					       //   handle!
            else
                polpush(symval - (offset - curpsd->pd_offset +
                        curpsd->pd_reloc), 0,
                        (symmsd == curmsd || (symmsd->md_addr != 0 &&
                        curmsd->md_addr != 0)) ?
                        PT_ABSOLUTE: PT_RELOFSMS, symmsd);
        }
        else
        {
            char bufr[100];

            sprintf(bufr, "Unsupported relative reference to absolute "
                    "symbol\n         %s", sympnt->sy_name+1);
            fail(bufr);
        }
    }
}

//*****************************************************************
// Function: twointern - Process internal symbol definition section
// Returned: Nothing
//*****************************************************************

void twointern(void)

{
    struct sy *sym;
    struct md *msd;
    long   newvalue;
    int    newpsn;
    char   flag;

    if (!mltcnt && !opsymsz && !symdd)	// Do we need to do this?
	return;				// No
    while (seccnt)
    {
	flag = getbyte();		// Get flag byte
	newvalue = getitem(flag);	// Get the value
        if (flag & SYF_ADDR)		// Address?
            newpsn = (flag & SYF_ABS)?	// Yes - get psect number or selector
                    (uint)getword(): (uint)getnumber();
        else				// Not address - if not indicated as
        {				//   ABS eat the extra byte which old
            if (!(flag & SYF_ABS))	//   versions of XMAC inserted!
                getbyte();
            newpsn = 0;
        }
	getsym();			// Get symbol
	symbuf[0] = SYM_SYM;
	if ((sym=looksym()) != NULL)	// Find it in the symbol table
	{
	    if (sym->sy_flag & SYF_MULT) // Multiply defined?
		multdef(sym, newvalue, newpsn); // Yes - complain
	    if ((opsymsz || symdd) && !(sym->sy_flag&SYF_INSYMT))
            {				// Loading symbol table?
		sym->sy_flag |= SYF_INSYMT; // Yes - load this symbol if its
					    //   not already loaded
                msd = sym->sy_psd->pd_msd;
		putinsym(msd, (msd)? msd->md_sgd: NULL, sym->sy_val.v,
                        sym->sy_sel,
			(sym->sy_flag & (DBF_ADR|DBF_SUP))|DBF_INT);
	    }
	}
        else
        {
            char buf[80];

            sprintf(buf, "Internal error: symbol \"%s\" not found during "
                    "pass 2\n", symbuf+1);
            fail(buf);
        }
    }
}

//*************************************************************
// Function: twolocal - Process local symbol definition section
// Returned: Nothing
//*************************************************************

void twolocal(void)

{
    long   value;
    int    flag;
    int    psn;
    struct pd *psd;
    struct md *msd;

    if (!(opsymsz || symdd))		// Do we need to do this?
        return;				// No
    while (seccnt)
    {
	flag = getbyte();		// Get flag byte
	value = getitem(flag);		// Get the value
        if (flag & SYF_ADDR)		// Address?
            psn = (flag & SYF_ABS)?	// Yes - get psect number or selector
                    (uint)getword(): (uint)getnumber();
        else				// Not address - if not indicated as
        {				//   ABS eat the extra byte which old
            if (!(flag & SYF_ABS))	//   versions of XMAC inserted!
                getbyte();
            psn = 0;
        }
        getsym();			// Get symbol
        psd = (flag & SYF_ABS)? &abspsd: getpsd(psn); // Get psect data block
        if (psn)			// Relocatable symbol?
            value += psd->pd_reloc + psd->pd_ofs; // Yes - relocate value
        msd = psd->pd_msd;
        putinsym(msd, (msd)? msd->md_sgd: 0, // Put in symbol table
                value, (flag & SYF_ABS)? psn: msd->md_sgd->sd_select,
                flag & (DBF_ADR|DBF_SUP));
    }
}

//*************************************************************
// Function: twoimport - Process local imported symbols section
// Returned: Nothing
//*************************************************************

// The only reason we need to process imported symbols during pass 2 is to
//   store the symbols in the debugger symbol table

void twoimport(void)

{
    if ((itype != I_XOSLKE) && (opsymsz || symdd)) // Do we need to do this?
    {					// Yes
        while (seccnt)
        {
	    getbyte();			// Get flag byte
            getsym();			// Get symbol
            putinsym(NULL, NULL, 0, 0, DBF_IMP); // Put in symbol table
        }
    }
}

//*******************************************************
// Function: twostradr - Process starting address section
// Returned: Nothing
//*******************************************************

void twostradr(void)

{
    long   addr;
    struct pd *psd;

    addr = getlong();
    psd = getpsd(getnumber());
    addr += psd->pd_ofs + psd->pd_reloc;
    if (psd->pd_msd->md_addr != 0xFFFFFFFFL)
        addr -= psd->pd_msd->md_addr;
    if (stradr != addr || strpsd != psd)
        multaddr("starting");
}

//******************************************************
// Function: twodebug - Process debugger address section
// Returned: Nothing
//******************************************************

void twodebug(void)

{
    long   addr;
    struct pd *psd;

    addr = getlong();
    psd = getpsd(getbyte());
    addr += psd->pd_ofs + psd->pd_reloc;
    if (psd->pd_msd->md_addr != 0xFFFFFFFFL)
        addr -= psd->pd_msd->md_addr;
    if (dbgadr != addr || dbgpsd != psd)
        multaddr("debugger");
}

//***********************************************************
// Function: twostack - Process initial stack address section
// Returned: Nothing
//***********************************************************

void twostack(void)

{
    long   addr;
    struct pd *psd;

    addr = getlong();
    psd = getpsd(getbyte());
    addr += psd->pd_ofs + psd->pd_reloc;
    if (psd->pd_msd->md_addr != 0xFFFFFFFFL)
        addr -= psd->pd_msd->md_addr;
    if (stkadr != addr || stkpsd != psd)
        multaddr("stack");
}

//*****************************************************
// Function: putinsym - put entry into debugger symbol
//		table(s) - this routine handles both
//		in-memory tables and symbol table files
// Returned: Nothing
//*****************************************************

void putinsym(
    struct md *msd,	// Pointer to msect data block for symbol
    struct sd *sgd,	// Pointer to segment data block for symbol
    long   value,	// Value of symbol
    int    select,	// Selector
    int    flags)	// Flag bits for symbol

{
    struct rd *rld;
    char   savfirst;
    char  *cpnt;
    int    ccnt;
    int    symnum;

    if (opsymsz)			// Building symbol table in memory?
    {					// Yes
        savfirst = lowfirst;
        lowfirst = opsymord;		// Set byte order for symbol table
        offset = stboffset;		//   entry
        putbyte(opsymwrd? (symsize & ~1): (symsize - 1));
					// Output length of symbol
        putbyte(flags);			// Output symbol flags
        if (msd && !(msd->md_flag & MA_ADDR)) // Is it relocatable?
        {
            rld = (struct rd *)getspace(sizeof(struct rd)); // Yes
            stbmsd->md_rdtail->rd_next = rld; // Allocate relocation data
            stbmsd->md_rdtail = rld;	      //   block
            rld->rd_next = NULL;
            rld->rd_loc = offset - stbmsd->md_offset;
            rld->rd_type = (opsymsz == 6)? (RT_16ABSOFS|RT_MSECT):
                    (RT_32ABSOFS|RT_MSECT);
            rld->rd_data.n = msd->md_num;
        }
        if (opsymsz == 6)
            putword(value);
        else
            putlong(value);
        if (sgd)			// Is this symbol associated with
        {				//   a segment?
            if (sgd->sd_select == 0)	// Yes - Do we have a selector
            {				//   specified?
                rld = (struct rd *) getspace(sizeof(struct rd)); // No
                stbmsd->md_rdtail->rd_next = rld; // Allocate relocation
                stbmsd->md_rdtail = rld;	  //   data block
                rld->rd_next = NULL;
                rld->rd_loc = offset - stbmsd->md_offset;
                rld->rd_type = RT_SELECTOR|RT_SEGMENT;
                rld->rd_data.n = sgd->sd_num;
            }
            putword(sgd->sd_select);	// Output the selector
        }
        else				// No segment for symbol - use
            putword(select);		//   possible absolute selector value
        cpnt = &symbuf[1];		// Output name of symbol
        ccnt = symsize - 1;
        while (--ccnt >= 0)
            putbyte(*cpnt++);
        if (opsymwrd && !(symsize & 1))
            putbyte(0);
        ++stbtotal;
        stboffset = offset;
        lowfirst = savfirst;
    }
    if (symdd)				// Generating symbol table file?
    {
        if ((msd && !(msd->md_flag & MA_ADDR)) || (sgd && sgd->sd_select == 0))
        {				// Is it relocatable?
            flags |= DBF_REL;		// Yes
            if (msd && (msd->md_addr != 0xFFFFFFFFL))
                value -= msd->md_addr;
        }
        outsymbyte(flags);		// Output symbol flags
        outsymbyte(value);		// Output value of symbol
        outsymbyte(value >> 8);
        outsymbyte(value >> 16);
        outsymbyte(value >> 24);
        if (flags & DBF_REL)
        {				// Is it relocatable?
            symnum = (flags & DBF_SEG)? sgd->sd_num: msd->md_num;
            outsymbyte(symnum);
            outsymbyte(symnum >> 8);
        }
        else				// Not relocatable
        {
            if (sgd)			// Get selector if have segment
                select = sgd->sd_select;
            outsymbyte(select);		// Output absolute selector value
            outsymbyte(select >> 8);
        }
        cpnt = &symbuf[1];		// Output name of symbol
        ccnt = symsize - 2;
        while (--ccnt >= 0)
            outsymbyte(*cpnt++);
        outsymbyte(*cpnt | 0x80);
        ++symtotal;			// Count the symbol
    }
}

//********************************************************
// Function: outsymbyte - Output byte to symbol table file
// Returned: Nothing
//********************************************************

void outsymbyte(chr)
uint chr;

{
    long rtn;

    if (--symbcnt < 0)			// Have room in current buffer?
    {
        if ((rtn=svcIoOutBlock(symdd, symbufr, 1024)) < 0)
					// No - output current buffer
            femsg(prgname, rtn, symfsp); // If error
        symblk = TRUE;
        symbpnt = symbufr;
        symbcnt = 1023;
    }
    *symbpnt++ = chr;
    ++symlength;
}

//**************************************************************
// Function: segpush - Push segment selector on the polish stack
// Returned: Nothing
//**************************************************************

void segpush(sgd)
struct sd *sgd;

{
    if (sgd->sd_select)
        polpush((long)(sgd->sd_select), 0, PT_ABSOLUTE, NULL);
    else
        polpush(0, 0, PT_SELMS, (struct md *)sgd);
}

//***************************************************
// Function: polpush - Push value on the polish stack
// Returned: Nothing
//***************************************************

void polpush(
    long  value,
    long  select,
    int   type,
    void *pnt)

{
    if (polspnt <= polstk)		// Check for stack overflow
	overflow();
    else
    {
        --polspnt;
        polspnt->ps_value = value;
        polspnt->ps_select = select;
        polspnt->ps_type = type;
        polspnt->ps_pnt.m = pnt;
    }
}

//************************************************
// Function: symname - Get name of external symbol
// Returned: Nothing
//************************************************

void symname(void)

{
    getsym();				// Get symbol
    symbuf[0] = SYM_SYM;
    if ((sympnt=looksym()) == NULL)
        fail("Cannot find data for symbol during pass 2");
    if (!(sympnt->sy_flag & SYF_UNDEF))
    {					// Look it up in the symbol table
	symval += sympnt->sy_val.v;	// Use value if defined
        symmsd = sympnt->sy_psd->pd_msd;
    }
    else
    {
	varwmsg(sympnt, "Undefined", "referenced"); // Complain if undefined
        symmsd = NULL;
    }
    return;
}

//*************************************************
// Function: symnum - Get number of external symbol
// Returned: Nothing
//*************************************************

void symnum(void)

{
    int symn;

    symn = (sectypx < MSSC_MIN)? getnumber(): getmsnum();
    if (symn > curmod->mb_symnummax)
    {
        char buf[60];

        sprintf(buf, "Illegal symbol number %d", symn);
        fail(buf);
    }
    sympnt = curmod->mb_symnumtbl[symn-1];
    if (!(sympnt->sy_flag & SYF_UNDEF))
    {
        if (sympnt->sy_flag & SYF_IMPORT)
        {
            symval = 0;
            sympsd = NULL;
            symmsd = NULL;
        }
        else
        {
            symval = sympnt->sy_val.v;
            sympsd = sympnt->sy_psd;
            symmsd = sympsd->pd_msd;
        }
    }
    else
    {
		varwmsg(sympnt, "Undefined", "referenced"); // Complain if undefined
        symval = 0;
        sympsd = NULL;
        symmsd = NULL;
    }
}


//***********************************************
// Function: checkcur - Ensure have current psect
// Returned: Nothing
//***********************************************

void checkcur(void)

{
    if (curpsd == NULL)
        fail("Indeterminate psect reference");
}


//******************************************
// Function: putreld - Store relocation item
// Returned: Nothing
//******************************************

void putreld(
    struct pstk *item,
    int    size)

{
    struct rd *rld;

    rld = (struct rd *)getspace(sizeof(struct rd)); // Allocate relocation
    curmsd->md_rdtail->rd_next = rld;		    //   data block
    curmsd->md_rdtail = rld;		// Link to end of relocation list
    rld->rd_next = NULL;
    rld->rd_loc = offset - curmsd->md_offset;
    switch (item->ps_type)
    {
     case PT_ABSOLUTE:
        rld->rd_type = (size == 32)? (RT_32ABSOFS|RT_NONE):
                ((size == 16)? (RT_16ABSOFS|RT_NONE): (RT_8ABSOFS|RT_NONE));
        rld->rd_data.n = 0;
        break;

     case PT_RELATIVE:
        rld->rd_type = (size == 32)? (RT_32RELOFS|RT_NONE):
                ((size == 16)? (RT_16RELOFS|RT_NONE): (RT_8RELOFS|RT_NONE));
        rld->rd_data.n = 0;
        break;

     case PT_SELMS:
        rld->rd_type = RT_SELECTOR|RT_SEGMENT;
        rld->rd_data.n = item->ps_pnt.s->sd_num;
        break;

     case PT_ABSOFSMS:
        rld->rd_type = (size == 32)? (RT_32ABSOFS|RT_MSECT):
                ((size == 16)? (RT_16ABSOFS|RT_MSECT): (RT_8ABSOFS|RT_MSECT));
        goto notsym;

     case PT_RELOFSMS:
        rld->rd_type = (size == 32)? (RT_32RELOFS|RT_MSECT):
                ((size == 16)? (RT_16RELOFS|RT_MSECT): (RT_8RELOFS|RT_MSECT));
        goto notsym;

     case PT_ADDRMS:
        rld->rd_type = (size == 32)? (RT_32ADDR|RT_MSECT):
                (RT_16ADDR|RT_MSECT);
     notsym:
        rld->rd_data.n = item->ps_pnt.m->md_num;        
        break;

     case PT_SELSYM:
        rld->rd_type = RT_SELECTOR|RT_SYMBOL;
        goto havesym;

     case PT_ABSOFSSYM:
        rld->rd_type = (size == 32)? (RT_32ABSOFS|RT_SYMBOL):
                ((size == 16)? (RT_16ABSOFS|RT_SYMBOL): (RT_8ABSOFS|RT_SYMBOL));
        goto havesym;

     case PT_RELOFSSYM:
        rld->rd_type = (size == 32)? (RT_32RELOFS|RT_SYMBOL):
                ((size == 16)? (RT_16RELOFS|RT_SYMBOL): (RT_8RELOFS|RT_SYMBOL));
        goto havesym;


     case PT_ADDRSYM:
        rld->rd_type = (size == 32)? (RT_32ADDR|RT_SYMBOL):
                (RT_16ADDR|RT_SYMBOL);
     havesym:
        rld->rd_data.s = item->ps_pnt.i;
        item->ps_pnt.i->sy_flag |= SYF_USED;
        ++importnum;
        break;

     default:
        {
            char buf[60];

            sprintf(buf, "Illegal relocatable item type (%d) (internal error)",
                    item->ps_type);
            fail(buf);
        }
    }
}

//**********************************************************
// Function: ms2mtheadr - Process MS module header (MTHEADR)
// Returned: Nothing
//**********************************************************

void ms2mtheadr(void)

{
    getmssym();				// Get module name
    if (opsymsz || symdd)		// Loading debugger symbol table?
	putinsym(NULL, NULL, 0, 0, DBF_MOD); // Yes - put in symbol table
    strmov(modname, symbuf+1);		// Save module name
}

//****************************************************
// Function: ms2mcoment - Process MS comment (MCOMENT)
// Returned: Nothing
//****************************************************

void ms2mcoment(void)

{
}

//**************************************************************
// Function: ms2mmodend - Process MS 16-bit module end (MMODEND)
// Returned: Nothing
//**************************************************************

void ms2mmodend(void)

{
    uchar mtype;

    mtype = getbyte();			// Get module type
    if (mtype & 0x80)
        fail("Start address not valid in MS format OBJ file");
    twoeom();
}

//************************************************************
// Function: ms2mtypdef - Process MS type definition (MTYPDEF)
// Returned: Nothing
//************************************************************

void ms2mtypdef(void)

{
}

//**************************************************************
// Function: ms2mpubdef - Process MS public definition (MPUBDEF)
// Returned: Nothing
//**************************************************************

void ms2mpubdef(void)

{
    struct sy *sym;
    struct md *msd;
    long   newoffset;
    int    newseg;
    int    newgrp;

    if (!mltcnt && !opsymsz && !symdd)	// Do we need to do this?
	return;				// No
    newgrp = getmsnum();		// Yes - get group number
    newseg = getmsnum();		// Get "segment" number
    if (newseg == 0 && newgrp == 0)
    {
        if(getword() != 0)
            fail("MS public symbol value is absolute address (unsupported)");
    }
    while (seccnt > 1)
    {
        getmssym();			// Get symbol name
	symbuf[0] = SYM_SYM;
        newoffset = (sectypx == MSSC_PUBDEF)? getword(): getlong();
        getbyte();			// Discard the type index
	if ((sym=(sectypx < MSSC_LOCPUB)? looksym(): looklocsym()) != NULL)
        {				// Find it in the symbol table

	    if (sym->sy_flag & SYF_MULT) // Multiply defined?
		multdef(sym, newoffset, newseg); // Yes - complain
	    if ((opsymsz || symdd) && !(sym->sy_flag&SYF_INSYMT))
            {				// Loading symbol table?
		sym->sy_flag |= SYF_INSYMT; // Yes - load this symbol if its
					    //   not already loaded
                msd = sym->sy_psd->pd_msd;
		putinsym(msd, (msd)? msd->md_sgd: NULL, sym->sy_val.v,
                        sym->sy_sel, (sym->sy_flag & (DBF_ADR|DBF_SUP))|
                        ((sectypx < MSSC_LOCPUB)? DBF_INT: 0));
	    }
	}
        else
        {
            char buf[80];

            sprintf(buf, "Internal error: MS symbol \"%s\" not found during "
                    "pass 2\n", symbuf+1);
            fail(buf);
        }
    }
}

//**********************************************************
// Function: ms2mlocsym - Process MS local symbols (MLOCSYM)
// Returned: Nothing
//**********************************************************

void ms2mlocsym(void)

{
}

//***************************************************************
// Function: ms2mlinnum - Process MS source line number (MLINNUM)
// Returned: Nothing
//***************************************************************

void ms2mlinnum(void)

{
    struct pd *psd;
    struct md *msd;
    struct sd *sgd;
    char  *pnt1;
    char  *pnt2;
    int    linenum;
    ulong  offset;
    ulong  base;
    int    select;
    int    flag;
    char   chr;

	//@@@ CEY
	if( sectypx == MSSC_LINSYM16 || sectypx == MSSC_LINSYM32 )
	{
		while( seccnt > 1 )
		{
			getbyte( );
		}
		return;
	}

    if (opsymsz == 0 && symdd == 0)
        return;
    getmsnum();				// Discard the group number
    psd = getmspsd();			// Get MS "segment" number
    msd = psd->pd_msd;
    base = psd->pd_ofs;

    if (msd != NULL)
    {
        sgd = msd->md_sgd;
        select = sgd->sd_select;
       

        if (msd->md_flag & MA_ADDR)
        {
            flag = DBF_ADR;
            base += msd->md_addr;
        }
        else
            flag = DBF_ADR|DBF_REL;
    }
    else
    {
        sgd = NULL;
        select = 0;
        flag = 0;
    }
    while (seccnt > 1)
    {
        linenum = getword();		// Get line number
        offset = ((sectypx == MSSC_LINNUM)? getword(): getlong()) + base;
					// Get offset
        symbuf[1] = '$';
        pnt1 = pnt2 = symbuf+2;
        do				// Generate digits in reverse order
        {   *pnt1++ = linenum % 10 + '0';
        } while ((linenum /= 10) > 0);
        *pnt1 = '\0';			// Terminate the string

        symsize = pnt1 - symbuf;

        while (pnt2 < --pnt1)		// Reverse the digits
        {
            chr = *pnt2;
            *pnt2++ = *pnt1;
            *pnt1 = chr;
        }
        putinsym(msd, sgd, offset, select, flag);
    }
}

//****************************************************************
// Function: ms2msegdef - Process MS segment definitions (MSEGDEF)
// Returned: Nothing
//****************************************************************

void ms2msegdef(void)

{
    struct pd *psd;
    long size;

    if (++psnumcnt > curmod->mb_psectnummax)
        fail("More MS segments encountered in pass 2 than in pass 1");
    psd = *psnumpnt++;

    if (!(psd->pd_flag & PA_OVER))
    {
        if ((getbyte() & 0xE0) == 0)	// Absolute segment?
        {
            getword();			// Yes - discard selector
            getbyte();			// Skip unused byte
        }

        size = (sectypx == MSSC_SEGDEF)? getword(): getlong();

        if (psd->pd_mod > 1)
            size = ((size + psd->pd_mod - 1)/psd->pd_mod) * psd->pd_mod;
        psd->pd_nxo = psd->pd_ofs + size;
    }
}

//***************************************************************
// Function: ms2mfixupp - Process MS fix-up (MFIXUPP or (MFIX386)
// Returned: Nothing
//***************************************************************

// Note that this version ignores the frame specification and does all
//   fixups assuming that the frame is the current psect

void ms2mfixupp(void)

{
    struct sy *tarsym;
    struct sd *tarsgd;
    struct md *tarmsd;
    struct pd *tarpsd;
    int    locat;
    uchar  fixdat;
    uchar  header;
    int    type;
    int    fmethod;
    int    ftarget;
    int    size;
    int    bytes;
    ulong  selofs;
    ulong  tarofs;
    ulong  tarsel;
    int    boffset;
    struct pstk item;
    char   tarabssel;
    char   tarabsofs;

    static char badoffset[] = "MS fix-up offset is too large";

    while (seccnt > 1)
    {
        if ((header = getbyte()) & 0x80) // Get header byte
        {								// If fix-up item
            if (msdispatch != msdoledata && msdispatch != msdolidata)
                fail("MS MFIXUPP record not preceeded by MLEDATA or MLIDATA");
            locat = getbyte() | (header << 8); // Get the locat value
            boffset = locat & 0x3FF;	// Get offset in data buffer
            fixdat = getbyte();			// Get the fixdat byte
            if (fixdat & 0x80)			// Is frame is specified by a thread?
                fail("MS thread fix-up not implemented yet");
            else
            {							// Frame is specified explicitly
                type = (locat >> 10) & 0x0F;
                fmethod = (fixdat >> 4) & 0x7;
                switch (fmethod)
                {
                 case 0:				// Frame is specified as segment number
                    getmsnum();
                    break;

                 case 1:				// Frame is specified as group number
                    getmsnum();
                    break;

                 case 2:				// Frame is specified as symbol number
                    symnum();
                    break;

                 case 4:				// Frame is current psect
                    break;

                 case 5:				// Frame is psecified by the target
                    break;

                 default:
                    {
                        char buf[80];

                        sprintf(buf, "Illegal MS fixup frame method %d",
                                fmethod);
                        fail(buf);
                    }
                }
                ftarget = fixdat & 0x3;
                switch (ftarget)
                {
                 case 0:				// Target is specified as MS segment
										//   number
                    tarpsd = getmspsd();
                    tarmsd = tarpsd->pd_msd;
                    tarsgd = tarmsd->md_sgd;
                    tarsym = NULL;
                    if (tarsgd->sd_select != 0)
                    {
                        tarabssel = TRUE;
                        tarsel = tarsgd->sd_select;
                    }
                    else
                    {
                        tarabssel = FALSE;
                        tarsel = 0;
                    }
                    tarabsofs = (tarpsd->pd_msd->md_flag & MA_ADDR);
                    tarofs = tarpsd->pd_reloc + tarpsd->pd_ofs;
                    break;

                 case 1:				// Target is specified as group number
                    tarmsd = getmsmsd();
                    tarsgd = (symmsd != NULL)? symmsd->md_sgd: NULL;
                    tarsym = NULL;
                    if (tarsgd->sd_select != 0)
                    {
                        tarabssel = TRUE;
                        tarsel = tarsgd->sd_select;
                    }
                    else
                    {
                        tarabssel = FALSE;
                        tarsel = 0;
                    }
                    if (tarmsd->md_flag & MA_ADDR)
                    {
                        tarabsofs = TRUE;
                        tarofs = 0;
                    }
                    else
                    {
                        tarabsofs = FALSE;
                        tarofs = 0;
                    }
                    break;

                 case 2:				// Target is specified as symbol number
                    symnum();
                    tarsgd = (symmsd != NULL)? symmsd->md_sgd: NULL;
                    tarmsd = symmsd;
                    tarsym = sympnt;
                    if (symmsd == NULL)
                    {
                        tarabssel = TRUE;
                        tarsel = sympnt->sy_sel;
                        tarabsofs = TRUE;
                        tarofs = sympnt->sy_val.v;
                    }
                    else
                    {
                        if (tarsgd->sd_select != 0)
                        {
                            tarabssel = TRUE;
                            tarsel = tarsgd->sd_select;
                        }
                        else
                        {
                            tarabssel = FALSE;
                            tarsel = 0;
                        }
                        tarabsofs = (symmsd->md_flag & MA_ADDR);
                        tarofs = sympnt->sy_val.v;
                    }
                    break;

                 default:
                    fail("Illegal MS fixup target method 3");
                }
                if (!(fixdat & 0x04))
                {
                    long temp;

                    temp = ((sectypx == MSSC_FIXUPP)? getword(): getlong());
                    tarofs += temp;
                }
            }
            switch (type)
            {
             case 0:					// 8-bit value
                if (boffset >= mscount)
                    fail(badoffset);
                if ((tarabsofs) && (tarsym) && (tarsym->sy_flag & SYF_IMPORT))
                {
                    offset = msoffset + boffset;
                    item.ps_type = PT_ABSOFSSYM;
                    item.ps_pnt.i = tarsym;
                    putreld(&item, 8);
                }
                else
                    *(uchar *)((msbuffer+boffset)) += (uchar)tarofs;
                break;

             case 2:					// Selector
                if (boffset > (mscount-2))
                    fail(badoffset);
                if (tarabssel)
                {
                    if ((tarsym) && (tarsym->sy_flag & SYF_IMPORT))
                    {
                        offset = msoffset + boffset;
                        item.ps_type = PT_SELSYM;
                        item.ps_pnt.i = tarsym;
                        putreld(&item, 0);
                    }
                    else
                        *(ushort *)((msbuffer+boffset)) += tarsel;
                }
                else
                {
                    offset = msoffset + boffset;
                    item.ps_type = PT_SELMS;
                    item.ps_pnt.s = tarsgd;
                    putreld(&item, 0);
                }
                break;

             case 3:					// 16-bit address
                if (boffset > (mscount-4))
                    fail(badoffset);
                size = 16;
                bytes = 2;
                selofs = boffset + 2;
                goto addrcom;

             case 11:					// 32-bit address
                if (boffset > (mscount-6))
                    fail(badoffset);
                size = 32;
                bytes = 4;
                selofs = boffset + 4;
             addrcom:
                if ((tarabssel) && (tarabsofs) &&
                        (tarsym->sy_flag & SYF_IMPORT) && (locat & 0x4000))
                {
                    offset = msoffset + boffset;
                    item.ps_type = PT_ADDRSYM;
                    item.ps_pnt.i = tarsym;
                    putreld(&item, size);
                    break;
                }
                if (tarabssel)
                {
                    if ((tarsym) && (tarsym->sy_flag & SYF_IMPORT))
                    {
                        offset = msoffset + selofs;
                        item.ps_type = PT_SELSYM;
                        item.ps_pnt.i = tarsym;
                        putreld(&item, 0);
                    }
                    else
                        *(ushort *)((msbuffer+selofs)) += tarsel;
                }
                else
                {
                    offset = msoffset + selofs;
                    item.ps_type = PT_SELMS;
                    item.ps_pnt.s = tarsgd;
                    putreld(&item, 0);
                }
                goto offsetcom;

             case 1:					// 16-bit offset
             case 5:
                if (boffset > (mscount-2))
                    fail(badoffset);
                size = 16;
                bytes = 2;
                goto offsetcom;   

             case 9:					// 32-bit offset
             case 13:
                if (boffset > (mscount-4))
                    fail(badoffset);
                size = 32;
                bytes = 4;
             offsetcom:
                if (tarabsofs)			// Absolute value?
                {
                    if ((tarsym) && (tarsym->sy_flag & SYF_IMPORT))
                    {
                        offset = msoffset + boffset;
                        item.ps_type = (locat & 0x4000)?
                                PT_ABSOFSSYM: PT_RELOFSSYM;
                        item.ps_pnt.i = tarsym;
                        putreld(&item, size);
                    }
                    else
                    {
                        if (!(locat & 0x4000)) // Yes - relative?
                        {				// Yes
                            tarofs -= ((msoffset - curmsd->md_offset) +
                                    boffset + bytes);
                            if (curmsd->md_flag & MA_ADDR) // Msect offset
                                tarofs -= curmsd->md_addr; //   known?
                            else
                            {
                                offset = msoffset + boffset;
                                item.ps_type = PT_RELATIVE;
                                putreld(&item, size);
                            }
                        }
                    }
                }
                else					// If not absolute
                {
                    if (!(locat & 0x4000)) // Relative?
                    {					// Yes
                        tarofs -= (msoffset + boffset);
                        if (curmsd != tarmsd)
                        {
                            offset = msoffset + boffset;
                            item.ps_type = PT_RELOFSMS;
                            item.ps_pnt.m = tarmsd;
                            putreld(&item, size);
                        }
                    }
                    else				// If not relative
                    {
                        item.ps_type = PT_ABSOFSMS;
                        offset = msoffset + boffset;
                        item.ps_pnt.m = tarmsd;
                        putreld(&item, size);
                    }
                }
                if (size == 16)
                    *(ushort *)((msbuffer+boffset)) += (ushort)tarofs;
                else
                    *(ulong *)((msbuffer+boffset)) += (ulong)tarofs;
                break;

             default:
                {
                    char buf[60];

                    sprintf(buf, "Illegal MS fix-up type %d", type);
                    fail(buf);
                }
            }
        }
        else							// If thread definition
            fail("MS thread definition not implemented yet");

    }
}

//************************************************************
// Function: ms2mledata - Process MS data (MLEDATA or MLED386)
// Returned: Nothing
//************************************************************

void ms2mlxdata(void)

{
    uchar *pnt;

    curpsd = getmspsd();
    if (seccnt > 1025)
    {
        char buf[80];

        sprintf(buf, "MS data record larger than 1024 bytes (%d)", seccnt-1);
        fail(buf);
    }
    curmsd = curpsd->pd_msd;
    cursgd = curmsd->md_sgd;
    msoffset = ((sectypx == MSSC_LEDATA)? getword(): getlong()) +
            curpsd->pd_offset + curpsd->pd_ofs;
    mscount = seccnt - 1;
    pnt = msbuffer;
    while (seccnt > 1)
        *pnt++ = getbyte();
    msdispatch = (sectypx < MSSC_LIDATA)? msdoledata: msdolidata;
}

//****************************************************
// Function: msdoledata - Copy MS data to the RUN file
// Returned: Nothing
//****************************************************

void msdoledata(void)

{
    uchar *pnt;

    offset = msoffset;
    pnt = msbuffer;
    while (--mscount >= 0)
        putbyte(*pnt++);
    msdispatch = (void (*)(void))nullfunc;
}

//****************************************************
// Function: msdolidata - Copy MS data to the RUN file
// Returned: Nothing
//****************************************************

void msdolidata(void)

{
    offset = msoffset;					// Set offset for storing data
    lid2block(msbuffer);				// Process the outer data block
    msdispatch = (void (*)(void))nullfunc;
}

//******************************************************
// Function: lid2block - Process LIDATA data block field
// Returned: Pointer to next byte in record
//******************************************************

uchar *lid2block(
    uchar *pnt1)

{
    union
    {   uchar  *c;
        ushort *s;
        ulong  *l;
    } pnt2;
    int    reptcnt;
    int    blkcnt;
    int    bytecnt;

    pnt2.c = pnt1;
    reptcnt = (sectypx == MSSC_LEDATA)? *pnt2.s++: *pnt2.l++;
    blkcnt = *pnt2.s++;
    while (--reptcnt >= 0)
    {
        pnt1 = pnt2.c;
        if (blkcnt == 0)
        {
            bytecnt = *pnt1++;
            while (--bytecnt >= 0)
                putbyte(*pnt1++);
        }
        else
        {
            while (--blkcnt >= 0)
                pnt1 = lid2block(pnt1);
        }
    }
    return (pnt1);
}

//***********************************************************
// Function: ms2mcomdef - Process MS communal names (MCOMDEF)
// Returned: Nothing
//***********************************************************

void ms2mcomdef(void)

{
    int    type;
    struct sy *sym;
    struct md *msd;

    if (!mltcnt && !opsymsz && !symdd)	// Do we need to do this?
	return;				// No
    while (seccnt >= 5)			// Yes
    {
        getmssym();			// Collect the symbol name
        getmsnum();			// Get and discard the type index
        type = getbyte();		// Get the data type
        if (type == 0x61)
        {
            getmsxnum();
            getmsxnum();
        }
        else if (type == 0x62)
            getmsxnum();
        else
        {
            char buf[80];

            sprintf(buf, "Illegal type value (0x02.2X) in MS communal "
                    "definition", type);
            fail(buf);
        }
	symbuf[0] = SYM_SYM;
	if ((sym=(sectypx == MSSC_COMDEF)? looksym(): looklocsym()) != NULL)
        {				// Find it in the symbol table

	    if (sym->sy_flag & SYF_MULT) // Multiply defined?
		multdef(sym, 0, 0);	// Yes - complain
	    if ((opsymsz || symdd) && !(sym->sy_flag&SYF_INSYMT))
            {				// Loading symbol table?
		sym->sy_flag |= SYF_INSYMT; // Yes - load this symbol if its
					    //   not already loaded
                msd = sym->sy_psd->pd_msd;
		putinsym(msd, (msd)? msd->md_sgd: NULL, sym->sy_val.v,
                        sym->sy_sel, (sym->sy_flag & (DBF_ADR|DBF_SUP))|
                        ((sectypx < MSSC_LOCPUB)? DBF_INT: 0));
	    }
	}
        else
        {
            char buf[80];

            sprintf(buf, "Internal error: MS symbol \"%s\" not found during "
                    "pass 2\n", symbuf+1);
            fail(buf);
        }
    }
}

//*********************************************************************
// Function: ms2mlocexd - Process MS Local extern definitions (MLOCEXD)
// Returned: Nothing
//*********************************************************************

void ms2mlocexd(void)

{
    fail("Unsupported MS record type LOCEXD");
}

//*********************************************************************
// Function: ms2mmodend - Process MS Loc func ext definitions (MFLCEXD)
// Returned: Nothing
//*********************************************************************

void ms2mflcexd(void)

{
    fail("Unsupported MS record type FLCEXD");
}

//**********************************************************************
// Function: ms2mcomdat - Process MS initialized communal data (MCOMDAT)
// Returned: Nothing
//**********************************************************************

void ms2mcomdat(void)

{
    int    flags;
    int    select;
    int    namenum;
    uchar *pnt;
    char  *name;
    struct sy *sym;

    flags = getbyte();			// Get flag byte
    select = getbyte();			// Get attribute byte
    getbyte();				// Get align byte
    msoffset = (sectypx == MSSC_COMDAT)? getword(): getlong();
    getmsnum();				// Get and discard type index
    if ((select & 0x0F) == 0)		// If necessary, get and discard
    {					//   the group and segment indexes
        getbyte();
        getbyte();
    }
    if ((namenum = getmsnum()) > curmod->mb_lnamesmax)
    {					// Get public name index
        char buf[60];

        sprintf(buf, "Illegal MS public name index %d", namenum);
        fail(buf);
    }
    name = curmod->mb_lnamestbl[namenum-1];
    strmov(symbuf+1, name+1);
    symsize = name[0] + 1;
    symbuf[0] = SYM_SYM;		// Indicate MS group name

    if (seccnt > 1025)
    {
        char buf[80];

        sprintf(buf, "MS communal data record larger than 1024 bytes (%d)",
                seccnt-1);
        fail(buf);
    }
    if ((sym=(flags & 0x04)? looklocsym(): looksym()) == NULL)
    {					// Find symbol table entry
        char buf[100];

        sprintf(buf, "MS public name %0.30s\n         not found in symbol "
                "table during pass 2", name+1);
        fail(buf);
    }
    curpsd = sym->sy_psd;
    curmsd = curpsd->pd_msd;
    cursgd = curmsd->md_sgd;
    if ((opsymsz || symdd) && !(sym->sy_flag&SYF_INSYMT))
    {					// Loading symbol table?
        sym->sy_flag |= SYF_INSYMT;	// Yes - load this symbol if its
					//   not already loaded
        putinsym(curmsd, cursgd, sym->sy_val.v, sym->sy_sel,
                (sym->sy_flag&(DBF_ADR|DBF_SUP))|((flags & 0x04)? 0: DBF_INT));
    }
    msoffset += curpsd->pd_offset + curpsd->pd_ofs;
    mscount = seccnt - 1;
    pnt = msbuffer;
    while (seccnt > 1)
        *pnt++ = getbyte();
    msdispatch = (flags & 0x02)? msdolidata: msdoledata;
}

//*******************************************************
// Function: getmspsd - Get pointer to PSD for MS segment
// Returned: Pointer to PSD
//*******************************************************

struct pd *getmspsd()

{
    uint segnum;

    if ((segnum = (uint)getmsnum()) == 0 ||
            segnum > (uint)(curmod->mb_psectnummax))
    {
        char buf[80];

        sprintf(buf, "Illegal MS segment number %d", segnum);
        fail(buf);
    }
    return (curmod->mb_psectnumtbl[segnum-1]);
}

//*****************************************************
// Function: getmsmsd - Get pointer to MSD for MS group
// Returned: Pointer to PSD
//*****************************************************

struct md *getmsmsd()

{
    uint grpnum;

    if ((grpnum = (uint)getmsnum()) == 0 ||
            grpnum > (uint)(curmod->mb_msectnummax))
    {
        char buf[80];

        sprintf(buf, "Illegal MS group number %d", grpnum);
        fail(buf);
    }
    return (curmod->mb_msectnumtbl[grpnum-1]);
}
