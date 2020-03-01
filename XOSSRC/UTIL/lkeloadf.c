//-----------------------------------------------------------------------
// LKELOADF.C
// Subroutine to load LKE
// 
// Written by: John R. Goltz
// 
// This function is intended to be called by LKELOAD and INSTALL
//-----------------------------------------------------------------------

// ++++
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

#include <CTYPE.H>
#include <ERRNO.H>
#include <SETJMP.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSRUN.H>
#include <XOSSTR.H>
#include <XOSERR.H>
#include <XCMALLOC.H>

#include "LKELOAD.H"

// Usage notes:

//   This function references the following function which the caller must
//   provide:
//	void message(int level, char *text);

//   This function is called whenever LKELOADF has information to display to
//   the user.  The level argument specifies the kind of information as
//   follows:
//	LKEML_INFO     = 1 - Routine information
//	LKEML_ERROR    = 2 - Error information
//	LKEML_FININFO  = 3 - Final routine information
//	LKEML_FINERROR = 4 - Final error information

//   The calling of the this function is controled by the value of the "quiet" 
//   argument as follows:
//	Bit 0 - Set if function is not to be called for routine information
//	Bit 1 - Set if function is not be be called for ER_FILNF errors

//   This function defines an internal char array named lkename which is 64
//   bytes long.  The caller must store the full pathname for the LKE to load
//   into this array before calling lkeloadf.  This array is initialized to
//   "XOSSYS:" so the caller can just store the filename starting at byte 7
//   if desired.

//   The addbfr argument must point to a device characteristics list.  It may
//   be NULL if there are no characteristics.

//   This function does NO output to the controlling terminal.

//   This file also declares the following internal items:
//	outpnt, outbufr,  and errjmp
//   Programs linked with this file must not use these names.

extern char prgname[];

char  *outpnt;
char   outbufr[1400];

static char loaddone;		// TRUE if LKE has been loaded

static struct inpparms
{   byte4_parm pos;
    char       end;
} inpparms =
{  {PAR_SET|REP_HEXV, 4, IOPAR_ABSPOS, -1}
};

static struct inpparms rdparms =
{  {PAR_SET|REP_HEXV, 4, IOPAR_ABSPOS, 0}
};

static char erriomsg[] = "Error reading LKE file";

static struct importdata
{   long  offset;
    short selector;
};
static struct importdata *importtbl;
static struct importdata *importpnt;

static long   importcnt;
static long   exportcnt;
static long   symcnt;
static long   undefcnt;

static long   dskcnt;
static long   dsksize;
static uchar *dskpnt;
static uchar *dskbufr;

static long   lkefile;
static struct runhead  header;

static struct runseg   dataseg;
static struct runmsect datamsect;
static struct runmsect symbolmsect;
static struct runseg   codeseg;
static struct runmsect codemsect;

static union			// Unions used to access the kernels exported
{   char far  *f;		//   symbol which requires a far address
    char near *n;
    long x[2];
}  exporttbl;
static union
{   char  far  *f;
    char  near *c;
    short near *s;
    long  near *l;
}  exportpnt;

static union
{   char c[8];
    long l[2];
}      prefix;
int    pfsize;

static char *lkecode;
static char *lkedata;
static char *lkesymbol;

#define defbuffer ((char *)0x300000)

static char far *pdefbuffer;

static struct _mab defmem =
{   defbuffer, defbuffer, defbuffer};

static long   symsize;
static int    quietval;
static char   symname[52];

static struct cfgchar
{
    byte4_char debug;
    char       end;
} cfgchar =
{
    {(PAR_GET|REP_TEXT), 4, "DEBUG", 0},
    0
};

static type_qab cfgqab =
{   QFNC_WAIT|QFNC_CLASSFUNC,	// qab_func    - Function
    0,				// qab_status  - Returned status
    0,				// qab_error   - Error code
    0,				// qab_amount  - Process ID
    0,				// qab_handle  - Device handle
    0,				// qab_vector  - Vector for interrupt
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    CF_VALUES,			// qab_option  - Options or command
    0,				// qab_count   - Count
    (char near *)"SYSTEM:", 0,	// qab_buffer1 - Pointer to file spec
    (char *)&cfgchar, 0,	// qab_buffer2 - Unused
    NULL, 0			// qab_parm    - Pointer to parameter area
};

LKEARGS lkeargs;

jmp_buf errjmp;

char   lkename[64] = "XOSSYS:";	// Buffer for LKE name

static ulong getbyte(void);
static char *getdefmem(long size);
static ulong getlong(void);
static void  getsym(void);
static long  getvar(int size);
static ulong getword(void);
static void  hexdigit(ulong value);
static int   lkefail(char *msg, long code);
static void  relocate(char *location, int count, long base);

void  decput(ulong value);
char *getmem(long size);
void  hex4put(ulong value);
void  message(int level, char *text);
void  spacesput(int cnt);
void  strput(char *text);
int   xstrncmp(char far *str1, char *str2, int size);


int lkeloadf(
    int    quiet,
    struct lkechar *addbfr)

{
    long   rtn;
    long   msnum;
    ulong  header1;
    ulong  header2;
    ulong  offset;
    int    ofssize;
    ulong  select;
    int    selsize;
    int    cnt;

    quietval = quiet;
    if (setjmp(&errjmp))
    {
		if (lkefile > 0)
			svcIoClose(lkefile, 0);
        return(1);
    }
    *((long *)&pdefbuffer) = (long)defbuffer;
    dskbufr = getmem(1024);

    //=====================================================================
    // First we open the LKE file and read the headers
    //=====================================================================

    loaddone = FALSE;
    if ((lkefile = svcIoOpen(O_IN, lkename, NULL)) <= 0) // Open the LKE file
        lkefail("Error opening LKE image file", lkefile);

    if ((rtn = svcIoInBlock(lkefile, (char *)&header, sizeof(struct runhead)))
            < 1)
        lkefail("Error reading LKE image file header", rtn);

    if (header.magic != 0x22D7)		// Check magic number
    {
        char errbufr[80];

        outpnt = errbufr;
		strput("Invalid signature word ");
        hex4put(header.magic);
        strput(" in image header");
        lkefail(errbufr, 0);
    }
    if (header.version != 2)		// Check RUN version number
		lkefail("LKE is not a version 2 image file", 0);
    if (header.proctype != RUNP_80386 && header.proctype != RUNP_80486)
		lkefail("LKE is not an 80386 image file", 0);
    if (header.imagetype != RUNT_XOSLKE)
		lkefail("LKE is not an LKE image", 0);

    if ((cnt = header.hdrsize - 58) < 0)
        lkefail("LKE image header is too small", 0);
    if (cnt != 0)
    {
        if ((rtn = svcIoSetPos(lkefile, cnt, 0)) < 0)
            lkefail("Error positioning for IO in LKE image file", rtn);
    }
    if (header.numsegs != 2)
		lkefail("LKE image does not contain exactly two segments", 0);

    //=====================================================================
    // Read the data segment header
    //=====================================================================

    if ((rtn = svcIoInBlock(lkefile, (uchar *)&dataseg, sh_SIZE)) < 1)
        lkefail("Error reading data segment header in LKE", rtn);

    if (dataseg.hdrsize != 12)
		lkefail("Data segment header in LKE is not 12 bytes long", 0);

    if (dataseg.type != 1)
		lkefail("First segment in LKE is not a data segment", 0);

    if ((dataseg.status & 1) == 1)
		lkefail("Data segment in LKE specifies load address", 0);

    if (dataseg.nummsc != 2)
        lkefail("Data segment does not contain exactly two msects", 0);

    //=====================================================================
    // Read the data msect header
    //=====================================================================

    if ((rtn = svcIoInBlock(lkefile, (uchar *)&datamsect,
            sizeof(struct runmsect))) < 1)
        lkefail("Error reading data msect header in LKE", rtn);
    if (datamsect.hdrsize != 36)
        lkefail("Data msect header size incorrect in LKE", 0);

    //=====================================================================
    // Read the symbol table msect header
    //=====================================================================

    if ((rtn = svcIoInBlock(lkefile, (uchar *)&symbolmsect,
            sizeof(struct runmsect))) < 1)
        lkefail("Error reading symbol table msect header in LKE", rtn);
    if (datamsect.hdrsize != 36)
        lkefail("Symbol table msect header size incorrect in LKE", 0);

    //=====================================================================
    // There must be a code segment, which must contain only a single msect
    //=====================================================================

    //=====================================================================
    // Read the code segment header
    //=====================================================================

    if ((rtn = svcIoInBlock(lkefile, (uchar *)&codeseg,
            sizeof(struct runseg))) < 1)
        lkefail("Error reading code segment header in LKE", rtn);

    if (codeseg.hdrsize != 12)
	lkefail("Code segment header in LKE is not 12 bytes long", 0);

    if (codeseg.type != 2)
	lkefail("Second segment in LKE is not a code segment", 0);

    if ((codeseg.status & 1) == 1)
	lkefail("Code segment in LKE specifies load address", 0);

    if (codeseg.nummsc != 1)
        lkefail("Code segment does not contain exactly one msect", 0);

    if ((rtn = svcIoInBlock(lkefile, (uchar *)&codemsect,
            sizeof(struct runmsect))) < 1)
        lkefail("Error reading code msect header in LKE", rtn);

    if (codemsect.hdrsize != 36)
        lkefail("Code msect header size incorrect in LKE", 0);

    //=====================================================================
    // Now read the imported symbol table from the LKE we are loading.
    //   We search for each symbol as we read the symbol from the LKE and
    //   store its value in our in-memory imported symbol table.  We do not
    //   save the names, since we don't care about them once we have the
    //   associated values.
    //=====================================================================

    if ((header.importos + header.exportos) != 0)
    {
        importtbl = importpnt = (struct importdata *)getmem(
                sizeof(struct importdata) * header.importsz);
        setpos(header.importos);
        importcnt = header.importsz;
        exporttbl.x[0] = 0;
        exporttbl.x[1] = 0xC0;
        exporttbl.n = *(char near * far *)exporttbl.f;
        while (--importcnt >= 0)
        {
            getbyte();			// Discard the header byte
            getsym();			// Get symbol name
            symcnt = -1;
            pfsize = (islower(symname[3]))? 6: 3;
            prefix.l[0] = prefix.l[1] = 0;
            strnmov(prefix.c, symname, pfsize);
            exportpnt.f = exporttbl.f;
            do
            {
                if (*((long far *)(exportpnt.f+4)) == prefix.l[0] &&
                        *((long far *)(exportpnt.f+8)) == prefix.l[1])
                    break;
                exportpnt.c = *((char near * far *)exportpnt.f);
            } while (exportpnt.c != NULL);
            if (exportpnt.c != NULL)
            {
                symcnt = *((long far *)(exportpnt.f+12));
                exportpnt.c += 16;
                symsize -= (pfsize + 1); // Adjust length of name
                do
                {
                    if (symsize == (*((uchar far *)(exportpnt.f+1)) & 0x1F) &&
                            (xstrncmp(exportpnt.f+2, symname+pfsize, symsize+1)
                            == 0))
                        break;
                    exportpnt.c += *((uchar far *)exportpnt.f);
                }
                while (--symcnt > 0);
            }
            if (symcnt > 0)
            {
                header1 = *exportpnt.f;
                header2 = *(exportpnt.f+1);
                exportpnt.c += (header2 & 0x1F) + 3;
                selsize = (!(header2 & 0x80))? 0: (header2 & 0x40)? 2: 1;
                ofssize = header1 - (header2 & 0x1F) - selsize - 3;

                switch (ofssize)
                {
                 case 0:
                    offset = (header2 & 0x20)? -1: 0;
                    break;

                 case 1:
                    offset = *((uchar far *)exportpnt.f);
                    if (header2 & 0x20)
                        offset |= 0xFFFFFF00;
                    break;

                 case 2:
                    offset = *((ushort far *)exportpnt.f);
                    if (header2 & 0x20)
                        offset |= 0xFFFF0000;
                    break;

                 case 3:
                    offset = *((uchar far *)exportpnt.f) +
                            (*((ushort far *)(exportpnt.f+1)) << 8);
                    if (header2 & 0x20)
                        offset |= 0xFF000000;
                    break;

                 case 4:
                    offset = *((long far *)exportpnt.f);
                    break;

                 default:
                    {
                        char errbufr[80];

                        outpnt = errbufr;
                        strput("Illegal offset size for exported symbol ");
                        strput(symname);
                        lkefail(errbufr, 0);
                    }
                }
                importpnt->offset = offset;
                exportpnt.c += ofssize;
                importpnt->selector = (selsize == 0)? 0: ((selsize == 1)?
                        (long)(*((uchar far *)exportpnt.f)):
                        (long)(*((short far *)exportpnt.f)));
            }
            else
            {
                outpnt = outbufr;
                if (undefcnt == 0)
                {
                    strput("? LKELOAD: Unable to load LKE ");
                    strput(lkename);
                    strput("\n");
                }
                ++undefcnt;
                strput("           Imported symbol ");
                strput(symname);
                strput(" is undefined\n");
                message(LKEML_ERROR, outbufr);
            }
            ++importpnt;
        }
        if (undefcnt != 0)				// Did we have any undefined symbols?
        {								// Yes - stop now!
            outpnt = outbufr;
            strput("           ");
            decput(undefcnt);
            strput(" undefined imported symbol");
            if (undefcnt != 1)
                strput("s");
            strput(" in ");
            strput(lkename);
            strput("\n");
            message(LKEML_FINERROR, outbufr);
			svcIoClose(lkefile, 0);
            return (1);
        }
    }

    //=====================================================================
    // Allocate memory to hold the code, data, and symbol table  msects
    //=====================================================================

    lkecode = getmem(codemsect.alloc);
    if (datamsect.alloc != 0)
        lkedata = getmem(datamsect.alloc);
    if (symbolmsect.alloc >= 8)
    {
        svcIoQueue(&cfgqab);			// See if XDT is loaded
        if ((char)cfgchar.debug.value == 'Y')
            lkesymbol = getmem(symbolmsect.alloc);
        else
        {
            symbolmsect.alloc = 0;
            symbolmsect.datasz = 0;
        }
    }
    else
    {
        symbolmsect.alloc = 0;
        symbolmsect.datasz = 0;
    }

    //=====================================================================
    // Read the code, data, and symbol table msects
    //=====================================================================

    if (datamsect.datasz != 0)
    {
        rdparms.pos.value = datamsect.dataos;
        if ((rtn = svcIoInBlockP(lkefile, lkedata, datamsect.datasz,
                &rdparms)) < 1)
	    lkefail("Error reading data msect contents", rtn);
    }
    if (symbolmsect.datasz != 0)
    {
        rdparms.pos.value = symbolmsect.dataos;
        if ((rtn = svcIoInBlockP(lkefile, lkesymbol, symbolmsect.datasz,
                &rdparms)) < 1)
	    lkefail("Error reading symbol table msect contents", rtn);
    }
    rdparms.pos.value = codemsect.dataos;
    if ((rtn = svcIoInBlockP(lkefile, lkecode, codemsect.datasz, &rdparms)) < 1)
		lkefail("Error reading code msect contents", rtn);

    memset(&lkeargs, 0, sizeof(lkeargs));
    lkeargs.lla_char = (char *)addbfr;	// Store address of characteristics list
    lkeargs.lla_ddata = lkedata;
    lkeargs.lla_dcount = datamsect.alloc;
    lkeargs.lla_sdata = lkesymbol;
    lkeargs.lla_scount = symbolmsect.alloc;
    lkeargs.lla_cdata = lkecode;
    lkeargs.lla_ccount = codemsect.alloc;

    //=====================================================================
    // Issue begin_load SVC
    //=====================================================================

    if ((rtn = svcSysLoadLke(&lkeargs)) < 0) // Error in begin_load
		lkefail("Error when initializing LKE load operation", rtn);
    if (header.start != 0)
        lkeargs.lla_init = header.start + lkeargs.lla_coffset;
										// Relocate starting offset
    setpos(datamsect.relos);
    relocate(lkedata, (int)datamsect.relsz, lkeargs.lla_doffset);

    if (symbolmsect.datasz != 0)
    {
        setpos(symbolmsect.relos);
        relocate(lkesymbol, (int)symbolmsect.relsz, lkeargs.lla_soffset);
    }
    setpos(codemsect.relos);
    relocate(lkecode, (int)codemsect.relsz, lkeargs.lla_coffset);

    //=====================================================================
    // Read the exported symbol defintions from the LKE image file and
    //   construct the new kernel exported symbol table which we will pass
    //   with the svcSysLoadLke SVC.  It is built in exactly the same format
    //   as it appears in the kernel's exported symbol table
    //=====================================================================

    if ((exportcnt = header.exportsz) != 0)
    {
        setpos(header.exportos);
        prefix.l[0] = prefix.l[1] = 0;
        do
        {
            header1 = getbyte();		// Get the header byte

            switch (header1 & 0x03)		// Get the offset value
            {
             case 0:
                offset = 0;
                break;

             case 1:
                offset = getbyte();
                if (header1 & 0x04)
                    offset |= 0xFFFFFF00;
                break;

             case 2:
                offset = getword();
                if (header1 & 0x04)
                    offset |= 0xFFFF0000;
                break;

             case 3:
                offset = getlong();
                break;
            }

            if (header1 & 0x20)
            {
                if (header1 & 0x80)
                    lkefail("Absolute selector specified for exported"
                            " symbol", 0);
                else
                    msnum = getbyte();
            }
            else
                msnum = 0;
            getsym();					// Get symbol name
            if (symsize < 7)
            {
                char errbufr[80];

                outpnt = errbufr;
                strput("Exported symbol name ");
                strput(symname);
                strput(" is too short");
                lkefail(errbufr, 0);
            }
            if (msnum == 1)
            {
                offset += lkeargs.lla_doffset;
                select = 0x18;
            }
            else if(msnum == 2)
            {
                offset += lkeargs.lla_soffset;
                select = 0x18;
            }
            else if(msnum == 3)
            {
                offset += lkeargs.lla_coffset;
                select = 0x20;
            }
            else if (msnum != 0)
            {
                char errbufr[80];

                outpnt = errbufr;
                strput("Msect number for exported symbol ");
                strput(symname);
                strput(" is out of range");
                lkefail(errbufr, 0);
            }
            if (prefix.c[0] != 0)
            {
                if (strncmp(symname, prefix.c, 6) != 0)
                    lkefail("More than one symbol prefix value found", 0);
            }
            else
            {
                lkeargs.lla_xdata = exportpnt.c = getdefmem(16);
                exportpnt.l[0] = 0;
                exportpnt.l[2] = 0;
                strnmov(exportpnt.c+4, symname, 6);
                strnmov(prefix.c, symname, 6);
                exportpnt.l[3] = header.exportsz;
            }
            if (offset == 0 || offset == -1)
                ofssize = 0;
            else if ((offset & 0xFFFFFF00) == 0 ||
                    (offset & 0xFFFFFF00) == 0xFFFFFF00)
                ofssize = 1;
            else if ((offset & 0xFFFF0000) == 0 ||
                    (offset & 0xFFFF0000) == 0xFFFF0000)
                ofssize = 2;
            else if ((offset & 0xFF000000) == 0 ||
                    (offset & 0xFF000000) == 0xFF000000)
                ofssize = 3;
            else
                ofssize = 4;
            header2 = symsize - 7;
            if (ofssize != 4 && (offset & 0x80000000))
                header2 |= 0x20;

            if (msnum == 0)
                selsize = 0;
            else
            if (select & 0xFF00)
            {
                selsize = 2;
                header2 |= 0xC0;
            }
            else
            {
                selsize = 1;
                header2 |= 0x80;
            }
            header1 = symsize + ofssize + selsize - 4;
            exportpnt.c = getdefmem(header1);
            *exportpnt.c++ = header1;
            *exportpnt.c++ = header2;
            exportpnt.c = strnmov(exportpnt.c, symname+6, symsize-6);
            switch (ofssize)
            {
             case 1:
                *exportpnt.c++ = (char)offset;
                break;

             case 2:
                *exportpnt.s++ = (short)offset;
                break;

             case 3:
                *exportpnt.c++ = (char)offset;
                *exportpnt.s++ = (short)(offset >> 8);
                break;

             case 4:
                *exportpnt.l++ = offset;
                break;
            }
            switch (selsize)
            {
             case 1:
                *exportpnt.c++ = (char)select;
                break;

             case 2:
                *exportpnt.s++ = (short)select;
                break;
            }

        } while (--exportcnt > 0);
        lkeargs.lla_xcount = ((exportpnt.c - defmem.mab_membase) + 3) & (~3);
    }

    //=====================================================================
    // Issue do_load SVC
    //=====================================================================

    if ((rtn = svcSysLoadLke(&lkeargs)) < 0) // Error in do_load
		lkefail("Error when doing LKE load operation", rtn);
    loaddone = TRUE;
    if ((quietval & 0x01) == 0)
    {
		outpnt = outbufr;
		strput(prgname);
		strput(": LKE ");

////	strput(msgbfr);
////	strmov(strmov(msgbfr, prgname), ": LKE ");
////	strput(msgbfr);


		strput(lkename);
		rtn = strlen(prgname) + 2;
		if (lkeargs.lla_csize == 0)
		{
			strput(" initialized but not loaded\n");
			spacesput(rtn);
			strput("Data size: ");
		}
		else
		{
			strput(" loaded\n");
			spacesput(rtn);
			strput("Code Size: ");
			decput(lkeargs.lla_csize);
			strput(", Data Size: ");
		}
		decput(lkeargs.lla_dsize);
		if (lkeargs.lla_scount != 0)
		{
			strput(", Symbol Size: ");
			decput(lkeargs.lla_ssize);
		}
		strput("\n");
		if (header.exportsz != 0)
		{
			spacesput(rtn);
			strput("Exported symbol table updated, ");
			decput(header.exportsz);
			strput(" symbol");
			if (header.exportsz != 1)
				strput("s");
			strput(" added, prefix is ");
			outpnt = strnmov(outpnt, symname, 6);
			strput("\n");
		}
		message(LKEML_FININFO, outbufr);
    }
    svcIoClose(lkefile, 0);
    return(0);
}


//***************************************************
// Function: lkefail - display message on fatal error
// Returned: Never returns
//***************************************************

static void lkefail(
    char *msg,
    long  code)

{
    if (code != ER_FILNF || (quietval & 0x02) == 0)
    {
		outpnt = outbufr;
		if (undefcnt == 0 || loaddone)
		{
			strput((loaddone) ?
					"? LKELOAD: Unable to update system export definitions "
					"after\n             loading LKE " :
					"? LKELOAD: Unable to load LKE ");
			strput(lkename);
			strput("\n");
		}
		strput("           ");
		strput(msg);
		strput("\n");
		if (code != 0)
		{
			strput("           ");
			outpnt += svcSysErrMsg(code, 0x03, outpnt);
			strput("\n");
		}
		message(LKEML_FINERROR, outbufr);
    }
    longjmp(&errjmp, 1);
}


//*************************************************************
// Function: relocate - Process relocation records for an msect
// Returned: Nothing
//*************************************************************

static void relocate(
    char *buffer,
    int   count,
    long  base)

{
    char *pnt;
    ulong item;
    ulong location;
    ulong relocoffset;
    ulong relocselector;
    uchar byte;
    uchar type;
    uchar kind;

    location = 0;
    while (--count >= 0)
    {
        byte = getbyte();
        type = byte >> 4;		// Get relocation type
        kind = (byte >> 2) & 0x03;	// Get relocation kind
        item = getbyte();		// Get item number
        if (item & 0x80)
        {
            item = (item & 0x7F) << 8;
            item |= getbyte();
            if (item & 0x4000)
            {
                item = (item & 0x3FFF) << 8;
                item |= getbyte();
                if (item & 0x200000)
                {
                    item = (item & 0x1FFFFF) << 8;
                    item |= getbyte(); 
                }
            }
        }
        if (kind == 1)
        {
            if (item == 1)
            {
                relocoffset = lkeargs.lla_doffset;
                relocselector = 0x18;
            }
            else if (item == 2)
            {
                relocoffset = lkeargs.lla_soffset;
                relocselector = 0x18;
            }
            else if (item == 3)
            {
                relocoffset = lkeargs.lla_coffset;
                relocselector = 0x20;
            }
            else
                lkefail("Bad relocation information in LKE: Msect number out"
                        " of range", 0);
        }
        else if (kind == 2)
        {
            if (item == 0 || item > header.importsz)
                lkefail("Bad relocation information in LKE: Symbol number out"
                        " of range", 0);
            relocoffset = importtbl[item-1].offset;
            relocselector = importtbl[item-1].selector;
        }
        else
        {
            lkefail("Bad relocation information in LKE: Illegal relocation"
                        " kind", 0);
        }

        location += getvar(byte);	// Get location for relocation

        pnt = buffer + location;

        switch (type)
        {
         case 0:		// Selector
            *((short *)pnt) = relocselector;
            break;

         case 5:		// 8-bit relative offset
            relocoffset -= (location + base);

         case 4:		// 8-bit absolute offset
			*((char *)pnt) += (char)relocoffset;
            break;

         case 7:		// 16-bit relative offset
            relocoffset -= (location + base);

         case 6:		// 16-bit absolute offset
			*((short *)pnt) += (short)relocoffset;
            break;

         case 9:		// 32-bit relative offset
            relocoffset -= (location + base);

         case 8:		// 32-bit absolute offset
			*((long *)pnt) += relocoffset;
            break;

         case 0xA:		// 16-bit address
			*((short *)pnt) += (short)relocoffset;
            *((short *)(pnt+2)) = relocselector;
            break;

         case 0xB:		// 32-bit address
			*((long *)pnt) += relocoffset;
            *((short *)(pnt+4)) = relocselector;
            break;

         default:
            lkefail("Bad relocation information in LKE: Illegal relocation"
                        " type", 0);
		}
    }
}


//**************************************************************
// Function: getsym - Collect symbol name from relocation record
// Returned: Nothing
//**************************************************************

static void getsym(void)

{
    int   cnt;
    char *pnt;
    char  chr;

    cnt = 48;
    pnt = symname;
    while (!((chr = getbyte()) & 0x80))
    {
        if (--cnt < 0)
            lkefail("Symbol name in LKE is too long", 0);
        *pnt++ = chr;
    }
    *pnt++ = chr & 0x7F;
    symsize = pnt - symname;
    *pnt = 0;
}


//************************************************************************
// Function: getvar - Collect variable length value from relocation record
// Returend: Value collected
//************************************************************************

static long getvar(
    int size)

{
    switch (size & 0x03)
    {
     case 0:
        return (getbyte());

     case 1:
        return (getword());
        break;

     case 2:
     {
        long temp;

        temp  = getbyte();
        return ((getword() << 8) | temp);
        break;
     }

     default:
        return (getlong());
    }    
}


//*********************************************************
// Function: setpos - Set IO position for next getbyte call
// Returned: Nothing
//*********************************************************

static void setpos(
    long pos)

{
    long block;
    long offset;
    long rtn;

    block = pos & ~0x3FF;
    offset = pos & 0x3FF;
    if (block != inpparms.pos.value)
    {
        inpparms.pos.value = block;
        if ((rtn = svcIoInBlockP(lkefile, dskbufr, 1024, &inpparms)) < 1)
            lkefail(erriomsg, rtn);
        dsksize = rtn;
    }
    dskcnt = dsksize - offset;
    dskpnt = dskbufr + offset;
}


//***************************************************
// Function: getbyte - Get byte value from image file
// Returned: Value
//***************************************************

static ulong getbyte(void)

{
    long  rtn;

    if (--dskcnt < 0)
    {
        inpparms.pos.value += 1024;
        if ((rtn = svcIoInBlockP(lkefile, dskbufr, 1024, &inpparms)) < 1)
            lkefail(erriomsg, rtn);
        dskcnt = rtn - 1;
        dsksize = rtn;
        dskpnt = dskbufr;
    }
    return (*dskpnt++);
}


//***************************************************
// Function: getword - Get word value from image file
// Returned: Value
//***************************************************

static ulong getword(void)

{
    ulong data;

    data = getbyte();
    data |= (getbyte() << 8);
    return (data);
}


//***************************************************
// Function: getlong - Get long value from image file
// Returned: Value
//***************************************************

static ulong getlong(void)

{
    ulong data;

    data = getword();
    data |= (getword() << 16);
    return (data);
}


//**************************************
// Function: Allocate memory
// Returned: Pointer to memory allocated
//**************************************

char *getmem(
    long size)

{
    char *rtn;

    if ((rtn = sbrk(size)) == (char *)(-1))
    {
        lkefail("Error allocating memory", -errno);
        return (NULL);
    }
    return (rtn);
}


static char *getdefmem(
    long size)

{    
    char *rtn;

    if ((rtn = sbrkxx(size, &defmem)) == (char *)(-1))
        lkefail("Error allocating memory for export definitions", -errno);
    return (rtn);
}


//*******************************************
// Function: strput - Store string for output
// Returned: Nothing
//*******************************************

void strput(
    char *str)

{
    char chr;

    while ((chr = *str++) != 0)
    {
        if (chr == '\n')
            *outpnt++ = '\r';
        *outpnt++ = chr;
    }
    *outpnt = 0;
}


//**********************************************
// Function: spacesput - Store spaces for output
// Returned: Nothing
//**********************************************

void spacesput(
    int cnt)

{
    while (--cnt >= 0)
	*outpnt++ = ' ';
}


//**************************************************
// Function: decput - Store decimal value for output
// Returned: Nothing
//**************************************************

void decput(
    ulong value)

{
    ulong remain;

    remain = value % 10;
    if ((value /= 10) != 0)
        decput(value);
    *outpnt++ = remain + '0';
}


//*******************************************************
// Function: hex4put - Store 4 digit hex value for output
// Returned: Nothing
//*******************************************************

static void hex4put(
    ulong value)

{
    hexdigit(value>>12);
    hexdigit(value>>8);
    hexdigit(value>>4);
    hexdigit(value);
}


//*******************************************************
// Function: hexdigit - Store single hex digit for output
// Returned: Nothing
//*******************************************************

static void hexdigit(
    ulong value)

{
    value &= 0xF;
    if (value > 9)
        value += 'A' - 10 - '0';
    value += '0';
    *outpnt++ = value;
}
