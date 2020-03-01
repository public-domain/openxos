//******************************************************
// Routines to read single file for first pass for XLINK
//******************************************************
// Written by John Goltz
//******************************************************

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
#include <XCMALLOC.H>
#include <XOSSTR.H>
#include "XLINK.H"
#include "XLINKEXT.H"

// Allocate local data

struct modblk *modlst;	// Pointer to current module list entry
long   svdblk;			// Place to save objblk while reading library entry
						//   section
int    libcnt;			// Place to save objcnt while loading library
uchar *libpnt;			// Place to save objpnt while loading library
int    libsec;			// Place to save seccnt while loading library
long   libblk;			// Place to save objblk while loading library
long   libamnt;			// Place to save objamnt while loading library
char   liblow;			// Place to save lowfirst while loading library
char   libdone;			// Library input done flag

// Section type dispatch table

void (*dspone[])(void) =
{   oneendmod,			// SC_EOF     = 0  End of module
    badsec,				// SC_INDEX   = 1  Library index section
    onestart,			// SC_START   = 2  Start of module section
    oneseg,				// SC_SEG     = 3  Segment definition section
    onemsect,			// SC_MSECT   = 4  Msect definition section
    onepsect,			// SC_PSECT   = 5  Psect definition section
    onedata,			// SC_DATA    = 6  Data section
    oneintern,			// SC_INTERN  = 7  Internal symbol definition section
    onelocal,			// SC_LOCAL   = 8  Local symbol definition section
    oneexport,			// SC_EXPORT  = 9  Exported symbol definition section
    onestradr,			// SC_STRADR  = 10 Starting address section
    onedebug,			// SC_DEBUG   = 11 Debugger address section
    onestack,			// SC_STACK   = 12 Initial stack address section
    onereq,				// SC_FILEREQ = 13 File request section
    oneextern,			// SC_EXTERN  = 14 Extern symbol definition section
    badsec,				// SC_SHARED  = 15 Shared library definition section
    oneimport,			// SC_IMPORT  = 16 Imported symbol definition section
    badsec,				//            = 17 Illegal
    badsec,				//            = 18 Illegal
    badsec,				//            = 19 Illegal
    oneendmod,			// SC_ENTRY   = 20 Entry list section
    badsec,				// SC_BAD     = 21 Illegal MS section type
    ms1mtheadr,			// SC_MTHEADR = 22 (0x80) Module header
    ms1mcoment,			// SC_MCOMENT = 23 (0x88) Comment
    ms1mmodend,			// SC_MMODEND = 24 (0x8A, 0x8B) Module end
    ms1mextdef,			// SC_MEXTDEF = 25 (0x8C) External definition
    ms1mtypdef,			// SC_MTYPDEF = 26 (0x8E) Type definition
    ms1mpubdef,			// SC_MPUBDEF = 27 (0x90, 0x91) Public definition
    ms1mlocsym,			// SC_MLOCSYM = 28 (0x92, 0x93) Local symbols
    ms1mlinnum,			// SC_MLINNUM = 29 (0x94, 0x95) Source line number
    ms1mlnames,			// SC_MLNAMES = 30 (0x96) Name list
    ms1msegdef,			// SC_MSEGDEF = 31 (0x98, 0x99) Segment definition
    ms1mgrpdef,			// SC_MGRPDEF = 32 (0x9A) Group definition
    (void (*)(void))nullfunc,
						// SC_MFIXUPP = 33 (0x9C, 0x9D) Fix up previous data
						//		     image
    ms1mledata,			// SC_MLEDATA = 34 (0xA0, 0xA1) Logical data image
    ms1mlidata,			// SC_MLIDATA = 35 (0xA2, 0xA3) Logical repeated
						//		     (iterated) data image
    ms1mcomdef,			// SC_MCOMDEF = 36 (0xB0) Communal names definition
    ms1mextdef,			// SC_MLOCEXD = 37 (0xB4) External definition visible
						//		     within module only
    ms1mflcexd,			// SC_MFLCEXD = 38 (0xB5) Func ext definition visible
						//		     within module only
    ms1mpubdef,			// SC_MLOCPUB = 39 (0xB6, 0xB7) Public definition
						//		     visible within module only
    ms1mcomdef,			// SC_MLOCCOM = 40 (0xB8) Communal name visible
						//	    	     within moduld only
    ms1mcomdat			// SC_MCOMDAT = 41 (0xC2, 0xC3) Initialized communal
};						//		     data

// Polish item type dispatch table

void (*oneddsp[])(int) =
{   (void (*)(int))nullfunc,	// 10000xxx - Arithmetic operators
    (void (*)(int))nullfunc,	// 10001xxx - Arithmetic operators
    onepush,					// 10010xxx - Push operators
    (void (*)(int))nullfunc,	// 10011xxx - Store operators
    onedsvl,					// 10100xxx - Push absolute value
    onedsvl,					// 10101xxx - Push relative absolute value
    onedsvl,					// 10110xxx - Push relocated value for
								//		current psect
    onedsvl,					// 10111xxx - Push relocated relative value
								//		for current psect
    onedsvn,					// 11000xxx - Push relocated value for any
								//		psect
    onedsvn,					// 11001xxx - Push relocated relative value
								//		for any psect
    onedsvs,					// 11010xxx - Push external value
    onedsvs,					// 11011xxx - Push external relative value,
								//		symbol name follows
    onedsvl,					// 11100xxx - Push external value, previous
								//		symbol
    onedsvl,					// 11101xxx - Push external relative value,
								//		previous symbol
    onedsvn,					// 11110xxx - Push external value, symbol
								//		number follows
    onedsvn						// 11111xxx - Push external relative value,
};								//		symbol number follows

// Push operators dispatch table

void (*onepdsp[])(void) =
{   (void (*)(void))onesym,
						// 10010000 - Push segment selector for external
						//		symbol, symbol name follows
    (void (*)(void))nullfunc,
						// 10010001 - Push segment selector for external
						//		symbol, previous symbol
    (void (*)(void))nullfunc,
						// 10010010 - Push segment selector for currect psect
    (void (*)(void))getnumber,
						// 10010011 - Push segment selector for any psect
    (void (*)(void))getnumber,
						// 10010100 - Push segment selector for any msect
    (void (*)(void))getnumber,
						// 10010101 - Push segment selector for any segment
    (void (*)(void))getnumber,
						// 10010110 - Push offset for any msect
    (void (*)(void))getnumber
						// 10010111 - Push segment selector for external
};						//		symbol, symbol number follows


//*******************************************************
// Function: fileone - Process single file for first pass
// Returned: Nothing
//*******************************************************

#if CFG_XLINK_MULTIPASS
 extern int	g_nPass1Lev;
#endif

void fileone( struct objblk *obj )
{
	int			tcnt;
	uchar		* tp1;
	uchar		* tp2;

#if CFG_XLINK_MULTIPASS
	if( g_nPass1Lev > 0 &&
			strnicmp( obj->obj_fsp, "XOSLIB:\\XOS\\LIB", 15 ) != 0 )
		return;
	fprintf(stdout, "->fileone(%3d) obj_mdl=%08X obj_fsp=[%.40s]\n",
			g_nPass1Lev, (int)obj->obj_mdl, obj->obj_fsp);

#endif

	opnobj (obj);						// Open the OBJ file
	if (!libflag)						// Library file?
	{			 						// No
		curmod = obj->obj_mdl =			// Just one module
					(struct modblk *)getspace(sizeof(struct modblk));
		moduleone();
	}
	else
	{
		// Processing a library file

		setobj();						// Yes - setup to read the index
		libdone = FALSE;				//   module
		svdblk = objblk;

		// Initialize module list pointer

		modlst = (struct modblk *)&obj->obj_mdl;

#if CFG_XLINK_MULTIPASS
		while (modlst->mb_next != 0 && modlst->mb_next != (struct modblk *)(-1))
		{
			fprintf(stdout, " ... traverse modlst %08X <- %08X\n",
					(int)modlst, (int)modlst->mb_next);
			modlst = modlst->mb_next;
		}
#endif

		if (auxloc == 0)				// Do we have a library index buffer?
			auxloc = (uchar *)getspace((long)512); // No - get one now

		tcnt = objcnt;					// Copy remaining part of buffer
		tp1 = objpnt;
		objpnt = tp2 = auxloc + (int)(objpnt - objbuf);

		while (--tcnt >= 0)
			* tp2++ = * tp1++;

		objbuf = auxloc;				// Use library index buffer now
		do
		{
			switch (getsec())			// Get next section     
		    {
			 case SC_EOM:
				libdone = TRUE;
				break;

			 case SC_INDEX:
				onelibent();			// Process library entry section
				break;

			 default:
				badsec();
				break;
			}
		} while (!libdone);

		if (curobj->obj_mdl == 0)		// Did we find anything to load?
		    curobj->obj_mdl = (struct modblk *)-1; // No - remember this!

	}
	closeobj();							// Close the input file
}


//*******************************************************************
// Function: onelibent - Process library entry section for first pass
// Returned: Nothing
//*******************************************************************

void onelibent(void)

{
	struct sy	*sym;
	long		modpnt;

	modpnt = getlong();					// Get pointer to module in file
	while (seccnt != 0)
    {
		getsym();						// Get symbol
		symbuf[0] = SYM_SYM;

		if ((sym=looksym()) != NULL && (sym->sy_flag & SYF_UNDEF))
		{
			modlst->mb_next = curmod =	// Add to used module list
				(struct modblk *)getspace(sizeof(struct modblk));

			modlst = modlst->mb_next;
			modlst->mb_next = NULL;
			modlst->mb_mod = modpnt;

		    libpnt = objpnt;			// Save our state
		    libcnt = objcnt;
		    libblk = objblk;
			libamnt = objamnt;

		    libsec = seccnt;
		    liblow = lowfirst;
		    objblk = svdblk;
		    objbuf = objloc;

			setblk(modpnt);				// Setup to read module

		    moduleone();				// Load the module
		    objpnt = libpnt;			// Restore input state
		    objcnt = libcnt;
		    objblk = libblk;
			objamnt = libamnt;
		    objbuf = auxloc;
			modtype = OT_XOS;
			seccnt = libsec;
			lowfirst = liblow;
		    break;
		}
	}
}


//*******************************************************************
// Function: moduleone - Process single program module for first pass
// Returned: Nothing
//*******************************************************************

void moduleone(void)

{
    setobj();

    curmod->mb_symnumtbl = symnumbase = (struct sy **)symnummem.mab_mempnt;
    curmod->mb_symnummax = 0;
    curmod->mb_lnamestbl = (char **)lnamesmem.mab_mempnt;
    curmod->mb_lnamesmax = 0;
    curmod->mb_segnumtbl = (struct sd **)segnummem.mab_mempnt;
    curmod->mb_segnummax = 0;
    curmod->mb_msectnumtbl = (struct md **)msectnummem.mab_mempnt;
    curmod->mb_msectnummax = 0;
    curmod->mb_psectnumtbl = (struct pd **)psectnummem.mab_mempnt;
    curmod->mb_psectnummax = 0;

    done = FALSE;
    do
    {
        getsec();
        if( debugflag )
            printf("p1: sec=0x%02.2X, size=%d\n", sectypx, seccnt);
        (*dspone[sectype])();			// Dispatch on section type
    } while (!done);
}


//**********************************************************
// Function: oneendmod - Update offset for all psects at end
//	of OBJ module during pass one
// Returned: Nothing
//**********************************************************

void oneendmod(void)
{
    struct sd *sgd;
    struct sd *lnksgd;
    struct md *msd;
    struct pd *psd;

	if( (sgd = sgdhead) != NULL )		// Scan all segments (if any)
	{
        do
        {
            if (sgd->sd_lnknum != 0)
            {
                if (sgd->sd_lnknum > curmod->mb_segnummax)
                {
                    char buf[80];

                    sprintf(buf, "Illegal segment number %d for linked segment",
                            sgd->sd_lnknum);
                    fail(buf);
                }
                lnksgd = curmod->mb_segnumtbl[(int)sgd->sd_lnknum-1];
                if (sgd->sd_linked != NULL &&
                        sgd->sd_linked != lnksgd)
                    confllink(sgd->sd_sym);
                else
                    sgd->sd_linked = lnksgd;
                sgd->sd_lnknum = 0;
            }
            if ((msd=sgd->sd_head) != NULL) // Scan all msects (if any)
            {
                do
                {   if ((psd=msd->md_head) != NULL)
                    {			// Scan all psects (if any)
                        do
                        {   if (psd->pd_flag & PA_OVER) // Overlayed?
                            {
                                if (psd->pd_lsize < psd->pd_qsize) // Yes
                                    psd->pd_lsize = psd->pd_qsize;
                            }
                            else		// If not overlayed
                            {
                                if (psd->pd_qsize != 0)
                                    psd->pd_lsize = psd->pd_ofs + psd->pd_qsize;
                                psd->pd_ofs = psd->pd_nxo; // Update offset
                            }
                            psd->pd_qsize = 0;
                        } while ((psd=psd->pd_next) != NULL);
                    }
                } while ((msd=msd->md_next) != NULL);
            }
        } while ((sgd=sgd->sd_next) != NULL);
    }
    modname[0] = 0;
    done = TRUE;						// Indicate finished with file
}


//**************************************************
// Function: onestart - Process module start section
// Returned: Nothing
//**************************************************

void onestart(void)

{
    getsym();							// Get module name
    strmov(modname, symbuf+1);			// Save module name
    if (debugflag)
        printf("    mod=%s\n", modname);
    if (opsymsz)						// Loading debugger symbols?
        resvsym();						// Reserve symbol table space
}


//******************************************************
// Function: oneseg - Process segment definition section
// Returned: Nothing
//******************************************************

void oneseg(void)

{
    long   addr;
    ulong  linked;
    int    select;
    uchar  flag1;
    uchar  flag2;
    uchar  type;
    uchar  priv;
    struct sy  *sym;
    struct sd  *sgd;
    struct sd **pnt;

    while (seccnt != 0)
    {
	flag1 = getbyte();					// Get first flag byte
        select = getitem(flag1);		// Get segment selector
        flag2 = getbyte();				// Get second flag byte
        addr = getitem(flag2);			// Get address or modulus
        linked = (flag2 & SF2_LINKED)? getnumber(): 0;
					// Get number of linked segment
        type = getbyte();				// Get segment type
        if (type > 4)					// Check for valid type
        {
            char buf[50];

            sprintf(buf, "Illegal segment type %d", type);
            fail(buf);
        }
        priv = getbyte();				// Get privilege level
        getsym();						// Get name of segment
        symbuf[0] = SYM_SEG;			// Indicate segment name
        if ((sym=looksym()) == NULL)	// Find segment symbol table entry
        {
            sym = entersym();			// Not there - create one now
            sym->sy_psd = NULL;
            sym->sy_flag = 0;
            sgd = (struct sd *)getspace(sizeof(struct sd));
            sym->sy_val.s = sgd;		// Make segment data block
            sgd->sd_sym = sym;
            sgd->sd_flag = flag1 & (SA_LARGE|SA_32BIT|SA_CONF|SA_READ|SA_WRITE);
            if (flag2 & SF2_EXTEND)
                sgd->sd_flag |= SA_EXTEND;
            if (flag2 & SF2_ADDR)
                sgd->sd_flag |= SA_ADDR;
            sgd->sd_type = type;
            sgd->sd_priv = priv;
            sgd->sd_lnknum = linked;
            sgd->sd_linked = NULL;
            if (sgd->sd_flag & SA_ADDR)	// Was address specified?
            {
                sgd->sd_addr = addr;	// Yes
                sgd->sd_mod = 0xFFFFFFFFL;
            }
            else
            {
                sgd->sd_addr = 0xFFFFFFFFL; // No - value is modulus
                sgd->sd_mod = addr;
            }
            sgd->sd_select = select;
            if (sgdtail)		// Link into segment list
                sgdtail->sd_next = sgd;
            else
                sgdhead = sgd;
            sgdtail = sgd;
            sgd->sd_next = NULL;
            sgd->sd_head = NULL;		// Initialize msect list for
            sgd->sd_tail = NULL;		//   segment
            sgd->sd_msn = 0;
            sgd->sd_num = ++seggbn;
        }
        else							// If this segment is already defined
        {								// Check for conflicts and complain
            sgd = sym->sy_val.s;		//   if find any
            if ((sgd->sd_flag ^ flag1) & SA_LARGE)
                conflatr("large/small", "segment", sgd->sd_sym);
            if ((sgd->sd_flag ^ flag1) & SA_32BIT)
                conflatr("32-bit/16-bit", "segment", sgd->sd_sym);
            if ((sgd->sd_flag ^ flag1) & SA_READ)
                conflatr("readable", "segment", sgd->sd_sym);
            if ((sgd->sd_flag ^ flag1) & SA_WRITE)
                conflatr("writable", "segment", sgd->sd_sym);
            flag1 = 0;
            if (flag2 & SF2_EXTEND)
                flag1 |= SA_EXTEND;
            if (flag2 & SF2_ADDR)
                flag1 |= SA_ADDR;
            if ((sgd->sd_flag ^ flag1) & SA_EXTEND)
                conflatr("extendable", "segment", sgd->sd_sym);
            if (sgd->sd_type != type)
                conflitm("segment", "type", sgd->sd_sym);
            if (sgd->sd_priv != priv)
                conflitm("segment", "privilege level", sgd->sd_sym);
            if (select != 0xFFFFFFFFL && sgd->sd_select != select)
                conflsel(sgd->sd_sym);
            if (addr != 0xFFFFFFFFL && (((sgd->sd_flag ^ flag1) & SA_ADDR) ||
                    ((flag1 & SA_ADDR)? sgd->sd_addr: sgd->sd_mod) != addr))
                confladdr("segment", sgd->sd_sym);
            sgd->sd_lnknum = linked;
        }
        if ((pnt = (struct sd **)sbrkx(sizeof(struct sd *), &segnummem))
                == NULL)
            fail("Cannot allocate memory for segment number table");
        *pnt = sgd;			// Store pointer in local table
        (curmod->mb_segnummax)++;
    }
}

//******************************************************
// Function: onemsect - Process msect definition section
// Returned: Nothing
//******************************************************

void onemsect(void)

{
    ulong  max;
    ulong  addr;
    uint   sgn;
    uchar  flag1;
    uchar  flag2;
    uchar  type;
    uchar  priv;
    struct md  *msd;
    struct sd  *sgd;
    struct sy  *sym;
    struct md **pnt;

    while (seccnt != 0)
    {
		flag1 = getbyte();				// Get first flag byte
        max = getitem(flag1);			// Get maximum space value
        flag2 = getbyte();				// Get second flag byte
        addr = getitem(flag2);			// Get address or modulus
        type = getbyte();				// Get msect type
        if (type > 4)					// Check for valid type
            fail("Illegal msect type");
        priv = getbyte();				// Get privledge level
        sgn = getbyte();				// Get segment number
        if (sgn == 0 || sgn > curmod->mb_segnummax) // Is it valid?
            fail("Illegal segment number"); // Yes - fail
        sgd = curmod->mb_segnumtbl[sgn-1]; // OK - get address of data block
										//   for the segment
        getsym();						// Get name of msect
        symbuf[0] = SYM_MSCT;			// Indicate msect name
        if ((sym=looksym()) == NULL)	// Find msect symbol table entry
        {
            sym = entersym();			// Create one now
            sym->sy_psd = NULL;
            sym->sy_flag = 0;
            msd = (struct md *)getspace(sizeof(struct md));
            sym->sy_val.m = msd;		// Make msect data block
            msd->md_sym = sym;
            msd->md_sgd = sgd;
            msd->md_flag = flag1 & (MA_CONF|MA_READ|MA_WRITE);
            if (flag2 & MF2_ADDR)
                msd->md_flag |= MA_ADDR;
            msd->md_type = type;
            msd->md_priv = priv;
            msd->md_max = max;
            if (msd->md_flag & MA_ADDR)	// Was address specified?
            {
                msd->md_addr = addr;	// Yes
                msd->md_mod = 0xFFFFFFFFL;
            }
            else
            {
                msd->md_addr = 0xFFFFFFFFL; // No - value is modulus
                msd->md_mod = addr;
            }
            msd->md_tsize = 0;
            msd->md_lsize = 0;
            msd->md_rdhead = NULL;
            msd->md_rdtail = (struct rd *)(&(msd->md_rdhead));
            msd->md_rdoffset = 0;
            msd->md_rdcnt = 0;
            if (sgd->sd_tail)			// Link into msect list
                sgd->sd_tail->md_next = msd;
            else
                sgd->sd_head = msd;
            sgd->sd_tail = msd;
            sgd->sd_msn++;
            msd->md_next = NULL;
            msd->md_head = NULL;		// Initialize psect list for msect
            msd->md_tail = NULL;
            ++mscgbn;					// Count the msect
        }
        else							// If this msect is already defined
        {
            msd = sym->sy_val.m;
            if ((msd->md_flag ^ flag1) & MA_READ)
                conflatr("readable", "msect", msd->md_sym);
            if ((msd->md_flag ^ flag1) & MA_WRITE)
                conflatr("writable", "msect", msd->md_sym);
            if ((msd->md_flag ^ flag1) & MA_CONF)
                conflatr("conformable", "msect", msd->md_sym);
            if (msd->md_type != type)
                conflitm("msect", "type", msd->md_sym);
            if (msd->md_priv != priv)
                conflitm("msect", "privilege level", msd->md_sym);
            if (flag2 & MF2_ADDR)
            {
                if ((msd->md_flag & MA_ADDR) && msd->md_addr != addr)
                    confladdr("msect", msd->md_sym);
                else
                {
                    msd->md_flag |= MA_ADDR;
                    msd->md_addr = addr;
                }
            }
            if (msd->md_sgd != sgd)
                conflblk("Msect", msd->md_sym, "segment", sgd->sd_sym);
        }
        if ((pnt = (struct md **)sbrkx(sizeof(struct md *), &msectnummem))
                == NULL)
            fail("Cannot allocate memory for msect number table");
        *pnt = msd;						// Store pointer in local table
        (curmod->mb_msectnummax)++;
    }
}

//******************************************************
// Function: onepsect - Process psect definition section
// Returned: Nothing
//******************************************************

void onepsect(void)

{
    long   tsize;
    long   lsize;
    int    msn;
    char   flag1;
    char   flag3;
    long   addr;
    struct pd  *psd;
    struct md  *msd;
    struct sy  *sym;
    struct pd **pnt;

    while (seccnt != 0)
    {
	flag1 = getbyte();					// Get flag byte
        tsize = getitem(flag1);			// Get total size of psect
        lsize = getitem(getbyte());		// Get loaded size of psect
        flag3 = getbyte();				// Get byte
        addr = getitem(flag3);			// Get address or modulus
        msn = getbyte();				// Get msect number
        if (msn == 0 || msn > curmod->mb_msectnummax) // Is it valid?
            fail("Illegal msect number"); // Yes - fail
        msd = curmod->mb_msectnumtbl[msn-1]; // OK - get address of block
											 //   for the msect
        getsym();						// Get name of psect
        symbuf[0] = SYM_PSCT;			// Indicate psect name
        if ((sym=looksym()) == NULL)	// Find psect symbol table entry
        {
            sym = entersym();			// Not there - create one now
            sym->sy_psd = NULL;
            psd = (struct pd *)getspace(sizeof(struct pd));
            sym->sy_val.p = psd;		// Create psect data block
            psd->pd_sym = sym;
            psd->pd_msd = msd;
            psd->pd_slh = NULL;
            psd->pd_slt = NULL;
            psd->pd_flag = flag1 & PA_OVER;
            if (flag3 & PF3_ADDR)
                psd->pd_flag |= PA_ADDR;
            if (psd->pd_flag & PA_ADDR)	// Was address specified?
            {
                psd->pd_addr = addr;	// Yes
                psd->pd_mod = 0xFFFFFFFFL;
            }
            else
            {
                psd->pd_addr = 0xFFFFFFFFL; // No - value is modulus
                psd->pd_mod = addr;
            }
            psd->pd_ssmax = 0;
            if (msd->md_tail)			// Link into psect list
                msd->md_tail->pd_next = psd;
            else
                msd->md_head = psd;
            msd->md_tail = psd;
            psd->pd_next = NULL;
            psd->pd_ofs = 0;
            psd->pd_nxo = tsize;		// Store size for advancing offset
            psd->pd_tsize = tsize;		// And as size
            psd->pd_qsize = lsize;
            psd->pd_lsize = 0;
            psd->pd_num = ++pscgbn;
        }
        else							// If this psect is already defined
        {
            psd = sym->sy_val.p;
            if ((psd->pd_flag ^ flag1) & PA_OVER)
                conflatr("concatenated/overlayed", "psect", psd->pd_sym);
            if (psd->pd_msd != msd)
                conflblk("Psect", psd->pd_sym, "msect", msd->md_sym);
            flag1 = (flag3 & PF3_ADDR)? PA_ADDR: 0;


            if (flag3 & PF3_ADDR)
            {
                if ((psd->pd_flag & PA_ADDR) && psd->pd_addr != addr)
                    confladdr("psect", psd->pd_sym);
                else
                {
                    psd->pd_flag |= PA_ADDR;
                    psd->pd_addr = addr;
                }
            }
            if (psd->pd_flag & PA_OVER)	// Overlayed psect?
            {
                if (tsize > psd->pd_tsize) // Yes - is this piece bigger?
                    psd->pd_tsize = tsize; // Yes - make this the current
					   //   size
            }
            else
            {
                psd->pd_nxo = psd->pd_ofs + tsize; // Not overlayed
                psd->pd_tsize += tsize;
            }
            psd->pd_qsize = lsize;
        }
        if ((pnt = (struct pd **)sbrkx(sizeof(struct pd *), &psectnummem))
                == NULL)
            fail("Cannot allocate memory for psect number table");
        *pnt = psd;						// Store pointer in local table
        (curmod->mb_psectnummax)++;
    }
}


//*****************************************
// Function: onedata - Process data section
// Returned: Nothing
//*****************************************

void onedata(void)

{
    int header;

    while (seccnt != 0)
    {
		if ((header=getbyte()) & 0x80)	// Get item header byte
			(*oneddsp[(header>>3)&0x0F])(header); // If polish item
		else
		{
			if (header >= 0x78)			// Is this an address?
                onedsvn(header);		// Yes - skip the address value
            else
                skpbyte(header);		// No - skip the data
		}
    }
}


//*******************************************
// Function: onepush - Process push operators
// Returned: Nothing
//*******************************************

void onepush(
    int header)

{
    (*onepdsp[header&7])();
}


//*****************************************
// Function: onedsvl - Skip following value
// Returned: Nothing
//*****************************************

void onedsvl(
    int header)

{
    if ((header &= 3) != 0)
    {
        if (header == 3)
            ++header;
        skpbyte(header);
    }
}


//*********************************************************
// Function: onedsvs - Skip following value and process the
//	external symbol reference
// Returned: NOthing
//*********************************************************

void onedsvs(
    int header)

{
    onedsvl(header);					// Skip value
    onesym();							// Process the symbol
}


//***********************************************************
// Function: onedsvn - Skip following value and the following
//	symbol or psect number
// Returned: Nothing
//***********************************************************

void onedsvn(
    int header)

{
    onedsvl(header);				// Skip value
    getnumber();					// Skip symbol number
}


//*****************************************************
// Function: onesym - Process external symbol reference
// Returned: Pointer to symbol table entry
//*****************************************************

struct sy *onesym(void)

{
	struct sy *sym;

	getsym();						// Get symbol

	symbuf[0] = SYM_SYM;
	if ((sym=looksym()) == NULL)	// Is it in the symbol table now?
	{
		sym = entersym();			// No - put it in now
		++symtblsize;
		sym->sy_flag = SYF_UNDEF;	// Indicate undefined
		++undefcnt;					// Count it
	}
	return (sym);
}


//*********************************************************************
// Function: oneintern - Process internal symbol definition (SC_INTERN)
// Returned: Nothing
//*********************************************************************

void oneintern(void)

{
    long   value;
    int    psnum;
    struct pd *psd;
    int    header;

    while (seccnt != 0)
    {
		header = getbyte();				// Get header byte
		value = getitem(header);		// Get value of symbol
        if (header & SYF_ADDR)			// Address?
            psnum = (header & SYF_ABS)?	// Yes - get psect value or number
                    (unsigned)getword(): (unsigned)getnumber();
        else							// Not address - if not indicated as
        {								//   ABS eat the extra byte which old
            if (!(header & SYF_ABS))	//   versions of XMAC inserted!
                getbyte();
            psnum = 0;
        }
        if (header & SYF_ABS)		
            psd = &abspsd;
        else
        {
            psd = getpsd(psnum);		// Get pointer to psect data
            if (psnum)					// Relocate if relative
                value += psd->pd_ofs;
            else
                header |= SYF_ABS;
            psnum = 0;
        }
		getsym();						// Get symbol name
        definternal(TRUE, header, psd, value, psnum);
    }									// Define the internal symbol
}


//*************************************************************
// Function: onelocal - Process local symbol definition section
// Returned: Nothing
//*************************************************************

void onelocal(void)

{
    int header;

    if (opsymsz || needsym)				// Loading debugger symbols?
    {
		while (seccnt != 0)				// Yes - must see how much space
		{								//   is needed
            header = getbyte();
            getitem(header);
            if (header & SYF_ADDR)		// Address?
            {
                if (header & SYF_ABS)	// Yes - get psect value or number
                    getword();
                else
                    getnumber();
            }
            else						 // Not address - if not indicated as
            {							 //   ABS eat the extra byte which
                if (!(header & SYF_ABS)) //   old versions of XMAC inserted!
                    getbyte();

            }
		    getsym();					// Get symbol name
            if (opsymsz)				// Loading debugger symbols?
                resvsym();				// Reserve debugger symbol space
		}
    }
}


//*****************************************************************
// Function: oneexport - Process exported symbol definition section
// Returned: Nothing
//*****************************************************************

void oneexport(void)

{
    long   value;
    int    psnum;
    struct pd *psd;
    struct sy *sym;
    int    header;

    while (seccnt != 0)
    {
		header = getbyte();				// Get header byte
		value = getitem(header);		// Get value of symbol
        if (header & SYF_ADDR)			// Address?
            psnum = (header & SYF_ABS) ? // Yes - get psect value or number
                    (unsigned)getword() : (unsigned)getnumber();
        else							// Not address - if not indicated as
        {								//   ABS eat the extra byte which old
            if (!(header & SYF_ABS))	//   versions of XMAC inserted!
                getbyte();
            psnum = 0;
        }
        if (header & SYF_ABS)		
            psd = &abspsd;
        else
        {
            psd = getpsd(psnum);		// Get pointer to psect data
            if (psnum)					// Relocate if relative
                value += psd->pd_ofs;
            else
                header |= SYF_ABS;
            psnum = 0;
        }
		getsym();						// Get symbol name
        sym = definternal(TRUE, header, psd, value, psnum); // Define the symbol
        sym->sy_flag |= SYF_EXPORT;
        if (exporttail == NULL)
            exporthead = sym;
        else
            exporttail->sy_export = sym;
        exporttail = sym;
        ++exportnum;
    }
}


//****************************************************
// Function: onestradr - Process start address section
// Returned: Nothing
//****************************************************

void onestradr(void)

{
    if (strpsd == NULL)
    {
        stradr = getlong();				// Get starting address
        strpsd = getpsd(getnumber());	// Get its psect
        if (strpsd == &abspsd)			// Make sure not absolute
        {
            absaddr("starting");
            strpsd = NULL;
        }
        else
            stradr += strpsd->pd_ofs;
    }
}


//******************************************************
// Function: onedebug - Process debugger address section
// Returned: Nothing
//******************************************************

void onedebug(void)

{
    static char msg[] = "debugger";

    if (dbgpsd == NULL)
    {
        dbgadr = getlong();				// Get debugger address
        dbgpsd = getpsd(getbyte());		// Get its psect
        if (dbgpsd == &abspsd)			// Make sure not absolute
        {
            absaddr(msg);
            dbgpsd = NULL;
        }
        else
            dbgadr += dbgpsd->pd_ofs;
    }
}


//***********************************************************
// Function: onestack - Process initial stack address section
// Returned: Nothing
//***********************************************************

void onestack(void)

{
    static char msg[] = "stack";

    if (stkpsd == NULL)
    {
        stkadr = getlong();				// Get stack pointer
        stkpsd = getpsd(getbyte());		// Get its psect
        if (stkpsd == &abspsd)			// Make sure not absolute
        {
            absaddr(msg);
            stkpsd = NULL;
        }
        else
            stkadr += stkpsd->pd_ofs;
    }
}


//************************************************
// Function: onereq - Process file request section
// Returned: Nothing
//************************************************

void onereq(void)

{
    struct objblk *file;
    char  *pnt;
    char   chr;
    char   haveext;

    file = (struct objblk *)getspace(offsetof(objblk, obj_fsp) + seccnt + 6);
    pnt = file->obj_fsp;				// Get space for the name
    haveext = FALSE;
    while (seccnt > 0)					// Read the name
    {
         chr = getbyte();
         if (chr == '.')
             haveext = TRUE;
        *pnt++ = chr;
    }
    if (!haveext)						// Add default extension if none
        strcpy(pnt, inpext);			//   given
    else
        *pnt = '\0';
    prevobj->obj_next = file;			// Link in the block
    file->obj_next = NULL;
    prevobj = file;
}


//****************************************************************
// Function: onextern - Process external symbol definition section
// Returned: Nothing
//****************************************************************

void oneextern(void)

{
    struct sy  *sym;
    struct sy **pnt;

    while (seccnt > 0)
    {
        sym = onesym();
        if ((pnt = (struct sy **)sbrkx(sizeof(struct sy *), &symnummem))
                == NULL)
            fail("Cannot allocate memory for symbol number table");
        *pnt = sym;
        (curmod->mb_symnummax)++;
    }
}


//****************************************************************
// Function: onimport - Process imported symbol definition section
// Returned: Nothing
//****************************************************************

void oneimport(void)

{
    struct sy  *sym;
    struct sy **pnt;

    while (seccnt > 0)
    {
        getbyte();						// Discard the flag byte
        sym = onesym();
        if (sym->sy_flag & SYF_UNDEF)	// Was it already in the table?
        {				// No
            sym->sy_flag |= SYF_IMPORT;
            sym->sy_flag &= ~SYF_UNDEF;
			--undefcnt;			// Count it
            importtail->sy_next = sym;
            sym->sy_next = NULL;
            importtail = sym;
            sym->sy_val.v = 0;
        }
        else
        {
            if (!(sym->sy_flag & SYF_IMPORT))
                printf("#### IMPORT CONFLICT %s\n", sym->sy_name);
        }
        if ((pnt = (struct sy **)sbrkx(sizeof(struct sy *), &symnummem))
                == NULL)
            fail("Cannot allocate memory for symbol number table");
        *pnt = sym;
        (curmod->mb_symnummax)++;
    }
}


//**********************************************************
// Function: ms1mtheadr - Process MS module header (MTHEADR)
// Returned: Nothing
//**********************************************************

void ms1mtheadr(void)

{
    getmssym();							// Get module name
    --symsize;
    if (opsymsz)						// Loading debugger symbols?
        resvsym();						// Reserve debugger symbol space
    strmov(modname, symbuf+1);			// Save module name
}


//****************************************************
// Function: ms1mcoment - Process MS comment (MCOMENT)
// Returned: Nothing
//****************************************************

void ms1mcoment(void)

{
	getbyte();							// Discard the comment type
	switch (getbyte())					// Dispatch on the comment class
	{
	 case 0xA0:							// OMF extensions
		break;

	 case 0xA8:							// Weak extern record
		weakextern(FALSE);
		haveweak = TRUE;
		break;

	 case 0xA9:							// Laxy Extern record
		weakextern(TRUE);
		havelazy = TRUE;
		break;
	}
}


struct sy *getsymbymsnum();

void weakextern(
	int lazy)

{
	struct sy *sym;

	sym = getsymbymsnum();

///	printf("### %s sym: %s\n", (lazy) ? "lazy" : "weak", sym->sy_name + 1);

	sym->sy_weak = getsymbymsnum();

///	printf("### %s ref: %s\n", (lazy) ? "lazy" : "weak",
///			sym->sy_weak->sy_name + 1);

	if (lazy)
		sym->sy_flag |= SYF_LAZY;
}


struct sy *getsymbymsnum()

{
	int symn;

    symn = getmsnum();
    if (symn > curmod->mb_symnummax)
    {
        char buf[60];

        sprintf(buf, "Illegal symbol number %d", symn);
        fail(buf);
    }
    return (curmod->mb_symnumtbl[symn-1]);
}


//******************************************************************
// Function: ms1mmodend - Process MS module end (MMODEND or M386END)
// Returned: Nothing
//******************************************************************

void ms1mmodend(void)

{
    struct pd  **msgpnt;
    struct md   *msd;
    struct pd   *psd;
    struct msgb *msgb;
    int    cnt;

    // If any MS groups were defined, we must change them into msects.
    //   Note that if there is an existing msect or segment with the
    //   right name, it will be used.

    if (msgbhead)
    {
        do
        {
            msgb = msgbhead;
            if ((cnt = msgb->msgb_size) != 0)
            {
                msgpnt = msgb->msgb_segs;
                psd = *msgpnt;
                if ((msd = psd->pd_msd) == NULL)
                    msd = assocmsseg(msgb->msgb_sym->sy_name+1, psd);
                while (--cnt >= 0)
                {
                    psd = *msgpnt++;
                    if (psd->pd_msd != 0)
                    {
                        if (psd->pd_msd != msd)
                            conflmsseg(psd->pd_sym);
                    }
                    else
                    {
                        // CHECK FOR ATTRIBUTE CONFLICTS HERE!

                        psd->pd_msd = msd;
                        if (msd->md_tail) // Link into psect list
                            msd->md_tail->pd_next = psd;
                        else
                            msd->md_head = psd;
                        msd->md_tail = psd;
                        psd->pd_next = NULL;
                        msd->md_psn++;
                    }
                }
            }
            else						// If have empty group, just create
            {							//   an msect for it
                char  *npnt;
                struct sy  *sym;
                struct sy  *ssym;
                struct sd  *sgd;
                struct md **pnt;

                npnt = strmov(symbuf+1, msgb->msgb_sym->sy_name+1);
                npnt[-1] = 'm';
                symbuf[0] = SYM_MSCT;
                symsize = (int)(npnt - symbuf);
                if ((sym=looksym()) == NULL) // Find msect symbol table entry
                {
                    sym = entersym();	// Create one now
                    sym->sy_psd = NULL;
                    msd = (struct md *)getspace(sizeof(struct md));
                    sym->sy_val.m = msd; // Make msect data block
                    msd->md_sym = sym;
                    msd->md_flag = 0;
                    msd->md_type = 0;
                    msd->md_priv = 0;
                    msd->md_max = 0;
                    msd->md_addr = 0xFFFFFFFFL;
                    msd->md_mod = 0xFFFFFFFFL;
                    msd->md_tsize = 0;
                    msd->md_lsize = 0;
                    msd->md_rdhead = NULL;
                    msd->md_rdtail = (struct rd *)(&(msd->md_rdhead));
                    msd->md_rdoffset = 0;
                    msd->md_rdcnt = 0;
                    npnt[-1] = 's';	// Look for the segment
                    symbuf[0] = SYM_SEG;
                    if ((ssym = looksym()) == NULL)
                    {
                        ssym = entersym(); // Not there - create it now
                        ssym->sy_psd = NULL;
                        sgd = (struct sd *)getspace(sizeof(struct sd));
                        ssym->sy_val.s = sgd; // Make segment data block
                        sgd->sd_sym = ssym;
                        sgd->sd_flag = 0;
                        sgd->sd_type = 0;
                        sgd->sd_priv = 0;
                        sgd->sd_lnknum = 0;
                        sgd->sd_linked = NULL;
                        sgd->sd_addr = 0xFFFFFFFFL;
                        sgd->sd_mod = 0xFFFFFFFFL;
                        if (sgdtail)	// Link into segment list
                            sgdtail->sd_next = sgd;
                        else
                            sgdhead = sgd;
                        sgdtail = sgd;
                        sgd->sd_next = NULL;
                        sgd->sd_head = NULL; // Initialize msect list for
                        sgd->sd_tail = NULL; //   segment
                        sgd->sd_msn = 0;
                        sgd->sd_num = ++seggbn;
                    }
                    msd->md_sgd = sgd;
                    if (sgd->sd_tail)	// Link into msect list
                        sgd->sd_tail->md_next = msd;
                    else
                        sgd->sd_head = msd;
                    sgd->sd_tail = msd;
                    sgd->sd_msn++;
                    msd->md_next = NULL;
                    msd->md_head = NULL; // Initialize psect list for msect
                    msd->md_tail = NULL;
                    ++mscgbn;			// Count the msect
                }
                else					// If this msect is already defined
                {
                    msd = sym->sy_val.m;
                }
                if ((pnt = (struct md **)sbrkx(sizeof(struct md *),
                        &msectnummem)) == NULL)
                    fail("Cannot allocate memory for msect number table");
                *pnt = msd;				// Store pointer in local table
                (curmod->mb_msectnummax)++;
            }
            msgbhead = msgb->msgb_next;
            msgb->msgb_next = (struct msgb *)(-1L);
        } while (msgbhead != NULL);
    }
    msgbhead = msgbtail = NULL;

    // If we have any MS segments (which are really psects!) which are
    //   not part of a group, we must create msects and real segments for
    //   them.  Note that if there is an existing msect or segment with
    //   the right name, it will be used.

    cnt = (int)(curmod->mb_psectnummax);
    msgpnt = curmod->mb_psectnumtbl;
    while (--cnt >= 0)
    {
        psd = *msgpnt++;
        if (psd->pd_msd == NULL)
        {
            msd = assocmsseg(psd->pd_sym->sy_name+1, psd);
            psd->pd_msd = msd;
            if (msd->md_tail) 			// Link into psect list
                msd->md_tail->pd_next = psd;
            else
                msd->md_head = psd;
            msd->md_tail = psd;
            psd->pd_next = NULL;
            msd->md_psn++;
        }
    }
    oneendmod();
}

//********************************************************
// Function: assocmsseg - Associate msect and real segment
//		with MS segment (which is really a psect!)
// Returned: Nothing
//********************************************************

// NOTE: This routine always forces the final letter of the msect name to be m.
//	 This is normally the right thing to do based on the conventions used
//	 by XLINK when generating msect names for MS defined items.  This may
//	 cause incorrect behavior if the psect corresponding to an MS segment
//	 is previously defined as belonging to an msect whose name does not end
//	 in m.  Normally this should not be done, but it is possible!

struct md *assocmsseg(
    char  *npnt,				// Pointer to base name
    struct pd *psd)

{
    struct sd  *sgd;
    struct md  *msd;
    struct sy  *msym;
    struct sy  *ssym;
    struct md **pnt;

    npnt = strmov(symbuf+1, npnt);
    npnt[-1] = 'm';
    symbuf[0] = SYM_MSCT;
    symsize = (int)(npnt - symbuf);
    if ((msym = looksym()) == NULL)		// Look for the msect
    {									// If not defined now
        msym = entersym();				// Put msect in the symbol table
        msym->sy_psd = NULL;
        msd = (struct md *)getspace(sizeof(struct md));
        msym->sy_val.m = msd;			// Make msect data block
        msd->md_sym = msym;
        msd->md_flag = MA_READ;
        if (psd->pd_flag & PA_MSWRITE)
            msd->md_flag |= MA_WRITE;
        msd->md_type = 0;
        msd->md_priv = 0;
        msd->md_max = 0;
        msd->md_addr = 0xFFFFFFFFL;
        msd->md_mod = 0xFFFFFFFFL;
        npnt[-1] = 's';					// Look for the segment
        symbuf[0] = SYM_SEG;
        if ((ssym = looksym()) == NULL)
        {
            ssym = entersym();			// Not there - create it now
            ssym->sy_psd = NULL;
            sgd = (struct sd *)getspace(sizeof(struct sd));
            ssym->sy_val.s = sgd;		// Make segment data block
            sgd->sd_sym = ssym;
            sgd->sd_flag = SA_READ;
            if (psd->pd_flag & PA_MSWRITE)
                sgd->sd_flag |= SA_WRITE;
            if (psd->pd_flag & PA_MS32BIT)
                sgd->sd_flag |= SA_32BIT;
            sgd->sd_type = (psd->pd_flag & PA_MSCODE)? 2: 1;
            sgd->sd_priv = 0;
            sgd->sd_lnknum = 0;
            sgd->sd_linked = NULL;
            sgd->sd_addr = 0xFFFFFFFFL;
            sgd->sd_mod = 0xFFFFFFFFL;
            if (sgdtail)				// Link into segment list
                sgdtail->sd_next = sgd;
            else
                sgdhead = sgd;
            sgdtail = sgd;
            sgd->sd_next = NULL;
            sgd->sd_head = NULL;		// Initialize msect list for segment
            sgd->sd_tail = NULL;
            sgd->sd_msn = 0;
            sgd->sd_num = ++seggbn;
        }
        msd->md_sgd = sgd;
        if (sgd->sd_tail)				// Link into msect list
            sgd->sd_tail->md_next = msd;
        else
            sgd->sd_head = msd;
        sgd->sd_tail = msd;
        sgd->sd_msn++;
        msd->md_next = NULL;
        msd->md_head = NULL;			// Initialize psect list for msect
        msd->md_tail = NULL;
        msd->md_tsize = 0;
        msd->md_lsize = 0;
        msd->md_rdhead = NULL;
        msd->md_rdtail = (struct rd *)(&(msd->md_rdhead));
        ++mscgbn;						// Count the msect
    }
    else								// If msect is already defined
        msd = msym->sy_val.m;
    if ((pnt = (struct md **)sbrkx(sizeof(struct md *), &msectnummem)) == NULL)
        fail("Cannot allocate memory for msect number table");
    *pnt = msd;							// Store pointer in local table
    (curmod->mb_msectnummax)++;
    return (msd);
}


//****************************************************************
// Function: ms1mextdef - Process MS external definition (MEXTDEF)
// Returned: Nothing
//****************************************************************

void ms1mextdef(void)

{
    struct sy  *sym;
    struct sy **pnt;

    while (seccnt > 1)
    {
        getmssym();						// Get symbol
        getmsnum();						// Get and discard type index
        symbuf[0] = SYM_SYM;
        if ((sym=(sectypx < MSSC_LOCEXD)? looksym(): looklocsym()) == NULL)
        {								// Is it in the symbol table now?
            sym = entersym();			// No - put it in now
            if (sectypx >= MSSC_LOCEXD)	// Local symbol?
                sym->sy_mod = curmod; 	// Yes - make it local
            ++symtblsize;
            sym->sy_flag = SYF_UNDEF;	 // Indicate undefined
            ++undefcnt;					// Count it
        }
        if ((pnt = (struct sy **)sbrkx(sizeof(struct sy *), &symnummem))
                == NULL)
            fail("Cannot allocate memory for symbol number table");
        *pnt = sym;
        (curmod->mb_symnummax)++;
    }
}


//************************************************************
// Function: ms1mtypdef - Process MS type definition (MTYPDEF)
// Returned: Nothing
//************************************************************

void ms1mtypdef(void)

{
}


//****************************************************
// Function: ms1mpubdef - Process MS public definition
//		 (MPUBDEF and MPUB386)
// Returned: Nothing
//****************************************************

void ms1mpubdef(void)

{
    struct pd *psd;
    ulong  value;
    uint   selector;
    uint   header;
    int    basenum;
    int    segnum;

    basenum = (int)getmsnum();		// Get MS base group number
    if ((segnum = (int)getmsnum()) != 0) // Get MS "segment" number
    {
        if (segnum > curmod->mb_psectnummax)
            fail("Illegal MS segment number");
        psd = curmod->mb_psectnumtbl[(int)(segnum-1)];
    }
    if (basenum == 0 && segnum == 0)
    {
        selector = getword();
        header = (selector == 0)? SYF_ABS: SYF_ABS|SYF_ADDR;
    }
    else
        header = SYF_ADDR;
    while (seccnt > 1)
    {
        getmssym();			// Get symbol name
        value = (sectypx == MSSC_PUBDEF)? getword(): getlong();
        if (segnum != 0)		// Relocate if relative
            value += psd->pd_ofs;
        getmsnum();			// Get and discard type index
        definternal((sectypx < MSSC_LOCPUB),header, psd, value, selector);
    }					// Define the symbol
}

//*****************************************************************
// Function: ms1mlocsym - Process MS 16-bit local symbols (MLOCSYM)
// Returned: Nothing
//*****************************************************************

void ms1mlocsym(void)

{
}

//*******************************************************************
// Function: ms1mlinnum - Process MS 16-bit src line number (MLINNUM)
// Returned: Nothing
//*******************************************************************

void ms1mlinnum(void)

{
    int linenum;

	//@@@ CEY
	if( sectypx == MSSC_LINSYM16 || sectypx == MSSC_LINSYM32 )
	{
		while( seccnt > 1 )
		{
			getbyte( );
		}
		return;
	}

    if (opsymsz)			// Want symbols in memory?
    {					// Yes - must see how much space we
					//   need for these symbols
        getmsnum();			// Discard group and MS "segment"
        getmsnum();			//   numbers
        while (seccnt > 1)
        {
            linenum = getword();	// Get line number
            if (sectypx == MSSC_LINNUM)
                getword();		// Discard offset value
            else
                getlong();
            if (linenum < 10)
                symsize = 3;
            else if (linenum < 100)
                symsize = 4;
            else if (linenum < 1000)
                symsize = 5;
            else if (linenum < 10000)
                symsize = 6;
            else
                symsize = 7;
            resvsym();
        }
    }
}

//******************************************************
// Function: ms1mlnames - Process MS name list (MLNAMES)
// Returned: Nothing
//******************************************************

void ms1mlnames(void)

{
    char **pnt;
    char  *name;

    while (seccnt > 1)
    {
        getmssym();			// Get name
        name = getspace(symsize+2);
        strmov(name+1, symbuf+1);
        name[0] = symsize - 1;
        if ((pnt = (char **)sbrkx(sizeof(struct sy *), &lnamesmem)) == NULL)
            fail("Cannot allocate memory for symbol number table");
        *pnt = name;
        curmod->mb_lnamesmax++;
    }
}

int modtbl[] =
{   0,			// 0 - Absolute segment
    1,			// 1 - Relocatable, byte aligned
    2,			// 2 - Relocatable, word aligned
    16,			// 3 - Relocatable, paragraph aligned
    256,		// 4 - Relocatable, "page" aligned
    4,			// 5 - Relocatable, long aligned
    4096		// 6 - Relocatable, page aligned
};

//**************************************************
// Function: ms1msegdef - Process segment definition
//		 (MSEGDEF and MSEG386
// Returned: Nothing
//**************************************************

void ms1msegdef(void)

{
    char  *name;
    struct sy  *sym;
    struct pd  *psd;
    struct pd **pnt;
    uchar  attr;
    uchar  combine;
    uchar  alignment;
    ulong  segsize;
    uint   segnamenum;
    uint   segclassnum;

    attr = getbyte();			// Get attribute byte
    if ((combine = (attr >> 2) & 0x07) > 7)
    {
        char buf[60];

        sprintf(buf, "Illegal MS segment combine type %d", combine);
        fail(buf);
    } 
    if ((alignment = attr >> 5) == 0)	// Absolute segment?
        fail("Absolute MS segment (unsupported) specified");
    else if (alignment > 6)
    {
        char buf[60];

        sprintf(buf, "Illegal MS segment alignment type %d", alignment);
        fail(buf);
    }
    segsize = (sectypx == MSSC_SEGDEF)? getword(): getlong();
    if ((segnamenum = (int)getnumber()) > curmod->mb_lnamesmax)
    {					// Get segment name number
        char buf[60];

        sprintf(buf, "Illegal MS segment name index %d", segnamenum);
        fail(buf);
    }
    if ((segclassnum = (int)getnumber()) > curmod->mb_lnamesmax)
    {					// Get segment class number
        char buf[60];

        sprintf(buf, "Illegal MS segment class index %d", segclassnum);
        fail(buf);
    }
    getmsnum();				// Skip overlay name number
    name = curmod->mb_lnamestbl[segnamenum-1];

    if (alignment == 0)
    {
        strmov(strnmov(symbuf+1, name+1, name[0]), "_s");
        symbuf[0] = SYM_SEG;		// Indicate segment name

        fail("Fixed MS segment not supported!");

    }
    else
    {
        strmov(strnmov(symbuf+1, name+1, name[0]), "_p");
        symsize = name[0] + 3;
        symbuf[0] = SYM_PSCT;		// Indicate psect name
        if ((sym=looksym()) == NULL)	// Find psect symbol table entry
        {
            sym = entersym();		// Not there - create one now
            sym->sy_psd = NULL;
            psd = (struct pd *)getspace(sizeof(struct pd));
            sym->sy_val.p = psd;	// Create psect data block
            psd->pd_sym = sym;
            psd->pd_msd = NULL;
            psd->pd_slh = NULL;
            psd->pd_slt = NULL;
            psd->pd_addr = 0xFFFFFFFFL;
            psd->pd_flag = (combine == 6)? PA_OVER: 0;
            if (attr & 0x01)		// 32-bit segment?
                psd->pd_flag |= PA_MS32BIT; // Yes
            if (attr & 0x02)
            {
                if (sectypx == MSSC_SEGDEF && segsize == 0)
                    segsize = 0x10000L;
            }
            psd->pd_flag |= (strcmp(curmod->mb_lnamestbl[segclassnum-1]+1,
                    "CODE") == 0)? PA_MSCODE: PA_MSWRITE;
            psd->pd_mod = (psd->pd_flag & PA_MSCODE)? -1: modtbl[alignment];
            psd->pd_ofs = 0;
            psd->pd_nxo = 0;
            psd->pd_ssmax = 0;
            psd->pd_next = NULL;
            psd->pd_nxo = 0;
            psd->pd_tsize = 0;
            psd->pd_lsize = 0;
            psd->pd_qsize = 0;
            psd->pd_num = ++pscgbn;
        }
        else				// If this psect is already defined
        {
            psd = sym->sy_val.p;
            if (((psd->pd_flag & PA_OVER) == PA_OVER) ^ (combine == 6))
                conflatr("concatenated/overlayed", "MS segment", psd->pd_sym);
        }
        if (psd->pd_mod > 1)
            segsize = ((segsize + psd->pd_mod - 1)/psd->pd_mod) * psd->pd_mod;
        if (psd->pd_flag & PA_OVER)	// Overlayed psect?
        {
            if (segsize > psd->pd_tsize) // Yes - is this piece bigger?
                psd->pd_tsize = segsize; // Yes - make this the current size
        }
        else
        {
            psd->pd_nxo = psd->pd_ofs + segsize; // Not overlayed
            psd->pd_tsize += segsize;
        }
        if ((pnt = (struct pd **)sbrkx(sizeof(struct pd *), &psectnummem))
                == NULL)
            fail("Cannot allocate memory for psect number table");
        *pnt = psd;			// Store pointer in local table
        (curmod->mb_psectnummax)++;
    }
}

//*************************************************************
// Function: ms1mgrpdef - Process MS group definition (MGRPDEF)
// Returned: Nothing
//*************************************************************

void ms1mgrpdef(void)

{
    struct pd  **pnt;
    struct sy   *sym;
    struct msgb *msgb;
    struct pd   *msseg;
    char  *name;
    uint   namenum;
    uint   deftype;
    uint   segnum;
    int    cnt;

    if ((namenum = getmsnum()) > curmod->mb_lnamesmax)
    {					// Get group name number
        char buf[60];

        sprintf(buf, "Illegal MS group name index %d", namenum);
        fail(buf);
    }
    name = curmod->mb_lnamestbl[namenum-1];
    strmov(strnmov(symbuf+1, name+1, name[0]), "_g");
    symsize = name[0] + 3;
    symbuf[0] = SYM_MSGRP;		// Indicate MS group name
    if ((sym=looksym()) == NULL)	// Find symbol table entry
    {
        sym = entersym();		// Not there - create one now
        msgb = (struct msgb *)getspace(sizeof(struct msgb));
        sym->sy_val.g = msgb;
        msgb->msgb_next = (struct msgb *)(-1L);
        msgb->msgb_sym = sym;
        msgb->msgb_size = 0;
    }
    else
        msgb = sym->sy_val.g;
    if (msgb->msgb_next == (struct msgb *)(-1L))
    {
        if (msgbtail)			// Link into MS group list
            msgbtail->msgb_next = msgb;
        else
            msgbhead = msgb;
        msgbtail = msgb;
        msgb->msgb_next = NULL;
    }

    while (seccnt > 2)
    {
        if ((deftype = getbyte()) != 0xFF)
        {
            char buf[60];

            sprintf(buf, "Illegal MS group definition type 0x%02.2X", deftype);
            fail(buf);
        }

        if ((segnum = getmsnum()) > curmod->mb_psectnummax)
        {
            char buf[60];

            sprintf(buf, "Illegal MS group segment number %d", segnum);
            fail(buf);
        }
        msseg = curmod->mb_psectnumtbl[segnum-1];
        pnt = msgb->msgb_segs;
        cnt = msgb->msgb_size;
        while (--cnt >= 0)
            if (*pnt++ == msseg)
                break;
        if (cnt < 0)
        {
            if (msgb->msgb_size >= MSGRPMAX) // Is the group too big?
            {
                char buf[80];

                sprintf(buf, "Too many MS segments for MS group %s",
                        symbuf+1);
                fail(buf);
            }
            msgb->msgb_segs[msgb->msgb_size] = msseg;
            msgb->msgb_size++;
        }
    }
}

//********************************************************
// Function: ms1mledata - Process MS 16-bit data (MLEDATA)
//Returned: Nothing
//********************************************************

void ms1mledata(void)

{
    int segnum;

    if ((segnum = getmsnum()) == 0 || segnum > curmod->mb_psectnummax)
        fail("Illegal MS segment number");
    curpsd = curmod->mb_psectnumtbl[(int)(segnum-1)];
    offset = (sectypx == MSSC_LEDATA)? getword(): getlong();
    curpsd->pd_qsize += (offset + seccnt - 1);
}

//*****************************************************************
// Function: ms1mlidata - Process MS 16-bit repeated data (MLIDATA)
// Returned: Nothing
//*****************************************************************

void ms1mlidata(void)

{
    int segnum;
    int temp;

    if ((segnum = getmsnum()) == 0 || segnum > curmod->mb_psectnummax)
        fail("Illegal MS segment number");
    curpsd = curmod->mb_psectnumtbl[(int)(segnum-1)];
    offset = (sectypx == MSSC_LEDATA)? getword(): getlong();
    curpsd->pd_qsize += (offset + (temp=lid1block()));
}

//************************************************************
// Function: lid1block - Get length of LIDATA data block field
// Returned: Length of data block field
//************************************************************

// This function consumes a data block and determines how much data it
//   generate when expanded

int lid1block(void)

{
    int reptcnt;
    int blkcnt;
    int bytecnt;
    int size;

    reptcnt = (sectypx == MSSC_LEDATA)? getword(): getlong();
    blkcnt = getword();
    if (blkcnt == 0)
    {
        size = bytecnt = getbyte();
        while (--bytecnt >= 0)
            getbyte();
    }
    else
    {
        size = 0; 
        while (--blkcnt >= 0)
            size += lid1block();
    }
    return (reptcnt * size);
}

//***********************************************************
// Function: ms1mcomdef - Process MS communal names (MCOMDEF)
// Returned: Nothing
//***********************************************************

void ms1mcomdef(void)

{
    int size;
    int type;
    int pflag;

    while (seccnt >= 5)
    {
        getmssym();			// Collect the symbol name
        getmsnum();			// Get and discard the type index
        type = getbyte();		// Get the data type
        if (type == 0x61)
        {
            size = getmsxnum() * getmsxnum();
            pflag = PA_OVER|PA_MS32BIT;
        }
        else if (type == 0x62)
        {
            size = getmsxnum();
            pflag = PA_OVER;
        }
        else
        {
            char buf[80];

            sprintf(buf, "Illegal type value (0x02.2X) in MS communal "
                    "definition");
            fail(buf);
        }
        defcommunal((sectypx == MSSC_COMDEF)? 0: 0x04, size, pflag, 4, symbuf+1,
                "DATA_m");
    }
}

//*********************************************************************
// Function: ms1mflcexd - Process MS Loc func ext definitions (MFLCEXD)
// Returned: Nothing
//*********************************************************************

void ms1mflcexd(void)

{
    fail("Unsupported MS record type FLCEXD");
}

//**********************************************************************
// Function: ms1mcomdef - Process MS initialized communal data (MCOMDAT)
// Returned: Nothing
//**********************************************************************

// This MS record looks for an overlayed psect with a named formed by
//   appending _p to the symbol name in the _TEXT_m or DATA_m msects.
//   If it is not found, it is created.  The symbol is defined to be an
//   internal whose value is the address of the first byte in the psect.
//   The data is ignored during pass one.  It will be loaded during pass
//   two.

void ms1mcomdat(void)

{
    int    flags;
    int    select;
    int    alloc;
    int    align;
    int    group;
    int    temp;
    int    segnum;
    int    size;
    int    pflag;
    long   dataos;
    struct md *msd;
    struct pd *msseg;
    char  *name;
    char  *pnt;
    char   msname[130];

    static int cdmodtbl[] = {1, 2, 16, 256, 4};

    flags = getbyte();
    select = getbyte();
    alloc = select & 0x0F;
    select >>= 4;
    if ((align = getbyte()) > 5)
        align = 1;
    dataos = (sectypx == MSSC_COMDAT)? getword(): getlong();
    getmsnum();

    // We need to determine the name of the msect to which the psect for the
    //   communal item will belong.  If an explicit MS segment is specified
    //   and that MS segment now belongs to an msect (which means it is part
    //   of an MS group) we use the name of that msect.  If it does not belong
    //   to an msect, we construct the msect name from the name of the psect
    //   by replacing the final _p with _m.  If the allocation is specified as
    //   "far code" or "code32", we use the name _TEXT_m.  If it is specified
    //   as "far data" or "data32", we use the name DATA_m.  Note that we do
    //   not differeniate between near and far items!

    // WARNING: If the MS segment specified here is later added to an MS group,
    //		this may not produce the expected results!  Generally, this
    //		should not be a problem, since MS OBJ files seem to define
    //		groups early in the file.

    pflag = PA_OVER;
    msseg = NULL;
    switch (alloc)
    {
     case 0:
        group = getmsnum();
        segnum = getmsnum();
        if ((group | segnum) == 0)
            fail("No segment specified for MS COMDAT record");    
        if (segnum > curmod->mb_psectnummax)
        {
            char buf[60];

            sprintf(buf, "Illegal MS segment number %d for communal data",
                    segnum);
            fail(buf);
        }
        msseg = curmod->mb_psectnumtbl[segnum-1];
        if ((msd = msseg->pd_msd) != NULL)
            strmov(msname, msd->md_sym->sy_name+1);
        else
        {
            pnt = strmov(msname, msseg->pd_sym->sy_name+1);
            pnt[-1] = 'm';
        }
        break;

     case 3:
        pflag |= PA_MS32BIT;
     case 1:
        strcpy(msname, "_TEXT_m");
        break;

     case 4:
        pflag |= PA_MS32BIT;
     case 2:
        strcpy(msname, "DATA_m");
        break;

     default:
        fail("Illegal allocation type for MS communal data");
    }

    // Here with the name of the msect to use determined.  Now we must get the
    //   name of the communal item and create or find an existing psect for it.

    temp = getmsnum();			// Get the name of the communal item
    name = curmod->mb_lnamestbl[temp-1]+1;
    size = seccnt - 1 + dataos;
    defcommunal(flags, size, pflag, (int)((align == 0)? ((msseg == NULL)? 4:
            msseg->pd_mod): cdmodtbl[align-1]), name, msname);
}

//**********************************************
// Function: defcommunal - Define communal items
// Returned: Nothing
//**********************************************

void defcommunal(
    int   flags,
    int   size,
    int   pflag,
    int   modulus,
    char *symname,
    char *msname)

{
    struct sy *sym;
    struct pd *psd;
    struct md *msd;
    char  *pnt;

    pnt = strmov(strmov(symbuf+1, symname), "_p");
    symsize = pnt - symbuf;
    symbuf[0] = SYM_PSCT;		// Indicate psect name
    if ((sym=looksym()) == NULL)	// Find psect symbol table entry
    {
        sym = entersym();		// Not there - create one now
        sym->sy_psd = NULL;
        psd = (struct pd *)getspace(sizeof(struct pd));
        sym->sy_val.p = psd;		// Create psect data block
        psd->pd_sym = sym;
        psd->pd_msd = NULL;
        psd->pd_slh = NULL;
        psd->pd_slt = NULL;
        psd->pd_addr = 0xFFFFFFFFL;
        psd->pd_flag = pflag;
        psd->pd_mod = modulus;
        psd->pd_ofs = 0;
        psd->pd_nxo = 0;
        psd->pd_next = NULL;
        psd->pd_next = NULL;
        psd->pd_nxo = 0;
        psd->pd_tsize = size;
        psd->pd_lsize = size;
        psd->pd_qsize = size;
        psd->pd_offset = 0;
        psd->pd_reloc = 0;
        psd->pd_num = 0xFFFF;
        psd->pd_ssmax = 0;
        msd = assocmsseg(msname, psd);	// Set up the corresponding msect
        psd->pd_msd = msd;
        if (msd->md_tail)		// Link into psect list
            msd->md_tail->pd_next = psd;
        else
            msd->md_head = psd;
        msd->md_tail = psd;
        msd->md_psn++;
    }
    else				// If this psect is already defined
    {
        psd = sym->sy_val.p;
        if ((psd->pd_flag & PA_OVER) != PA_OVER)
            conflatr("concatenated/overlayed", "MS segment", psd->pd_sym);
        psd->pd_mod = modulus;
    }
    if (psd->pd_mod > 1)
        size = ((size + psd->pd_mod - 1)/psd->pd_mod) * psd->pd_mod;
    if (size > psd->pd_tsize) 		// Is this piece bigger?
        psd->pd_tsize = size;	 	// Yes - make this the current size
					//   segment
    pnt = strmov(symbuf+1, symname);	// Define the symbol as an internal
    symsize = pnt - symbuf;
    definternal(!(flags&0x04), SYF_ADDR, psd, 0, 0);
}

//***********************************************
// Function: definternal - Define internal symbol
// Returned: Pointer to symbol table entry
//***********************************************

struct sy *definternal(
    int    global,
    int    header,
    struct pd *psd,
    ulong  value,
    uint   selector)

{
    struct sy *sym;

    symbuf[0] = SYM_SYM;
    if ((sym = (global) ? looksym() : looklocsym()) == NULL)
    {									// Look for symbol in symbol table
        sym = entersym();				// Put it in if its not there now
        ++symtblsize;
        if (sectypx >= MSSC_LOCPUB)		// Should it be local?
            sym->sy_mod = curmod;		// Yes - make it local
        if (opsymsz)					// Loading debugger symbols?
            resvsym();
    }
    else
    {
        if (sym->sy_flag & SYF_UNDEF)	// Is it undefined now?
        {
            --undefcnt;					// Yes - reduce count
            if (opsymsz)				// Loading debugger symbols?
                resvsym();
        }
        else
        {
            if (value != sym->sy_val.v || psd != sym->sy_psd)
            {
                if (!(sym->sy_flag & SYF_MULT)) // If mult. defined
                    ++mltcnt;
                sym->sy_flag |= SYF_MULT;
            }
            return (sym);
        }
    }
    if (psd->pd_ssmax < symsize)		// Is this longest symbol name for
										//   this psect?
        psd->pd_ssmax = symsize;		// Yes
    sym->sy_flag = header & (SYF_ABS|SYF_ADDR|SYF_SUP); // Store flags
    sym->sy_val.v = value;				// Store value
    sym->sy_sel = selector;
    sym->sy_psd = psd;					// Store psect data block address
    if (psd->pd_slt == NULL)			// Link into psect symbol list
        psd->pd_slh = sym;
    else
        psd->pd_slt->sy_next = sym;
    psd->pd_slt = sym;
    sym->sy_next = NULL;
    return (sym);
}

//******************************************************************
// Function: resvsym - Reserve space for debugger symbol table entry
// Returned: Nothing
//******************************************************************

void resvsym(void)

{
    stbsize += (long)((opsymwrd? (symsize & ~1):(symsize - 1)) + opsymsz);
    ++stbnum;							// Increase size of symbol table
}
