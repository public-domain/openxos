//***********************
// Main program for XLINK
//***********************
// Written by John Goltz
//***********************

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
#include <CTYPE.H>
#include <ERRNO.H>
#include <STRING.H>
#include <XOS.H>
#include <PROCARG.H>
#include <XCMALLOC.H>
#include <XOSSVC.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>
#include "XLINK.H"

extern void nullfunc(void);

// v 1.15 - 2-May-92
//	Changed command processor to accept any characters in file names
// v 1.16 - 8-May-92
//	Fixed some bugs in library handling, converted to use procarg
// v 2.0 - 13-May-92
//	Changed to use CLIBTV
// v 2.1 - 19-Jul-92
//	Fixed bug in filetwo which caused failure if unneeded library was
//	specified
// v 2.2 - 17-Oct-92
//	Added help text
// v 2.3 - 30-Aug-93
//	Fixed bug in handling absolute addresses in pass 2
// v 2.4 - 5-Dec-93
//	Added support for symbol numbers; changed default extension to .OBJ.
// v 2.5 - 7-Dec-93
//	Several bug fixes for symbol number support.
// v 2.6 - 8-Dec-93
//	Changed to handle symbol definitions without extra byte for absolute
//	non-address symbols
// v 2.7 - 23-Dec-93
//	Added support for MS OBJ files; major clean-up and fixes otherwise.
// v 3.0 - 27-Dec-93
//	Converted to 32-bit.
// v 3.1 - 29-Dec-93
//	Added support for line number symbols for MS OBJ files.
// v 3.2 - 11-Jan-94
//	Changed to use heapsort for sorting symbols in the map; changed to
//	start each module's part of each psect according to the psect's modulus.
// v 3.3 - 9-May-94
//	Added support for version 2 RUN files, added support for imported
//	and exported symbols.
// v 3.4 - ????
// v 3.5 - 8-Jul-94
//	Added support for MS communal items; fixed several bugs relating to
//	relocation.
// v 3.6 - 12-Jul-94
//	Fixed problem with generating relocation fixup stuff introduced in 3.5;
//	fixed problem with fixup handling for MS COMDEF records.
// v 3.7 - 16-Jul-94
//	Added reasonable error message for unsupported relative reference to
//	absolute symbol (did cause page fault).
// v 3.8 - 4-Oct-94
//	Fixed problem with processing libraries in pass one, was not saving
//	value of objamnt.
// v 3.9 - 30-Nov-94 (FPJ)
//	Added prototype for nullfunc().
// v 3.10 - 28-Sep-95 (JRG)
//	Merged dirvergent versions, no know changes!
// v 3.11 - 22-Dec-95 (JRG)
//	Fixed problem with not initializing the msbuffer pointer if loading an
//	MS module from an XOS library when no other MS modules had been
//	loaded.
// v 3.12 - 29-Jun-98 (JRG)
//	Fixed problem with calculating size of exported symbol table
//	incorrectly.  (Changed to always output 4 byte values of exported
//	symbols since need length of value before value is relocated!)
// v 3.13 - 26-Sep-98 (JRG)
//	Fixed minor problem with displaying selector values in the map.

// Allocate global data

int    version = CFG_XLINK_VER_MAJ;	// LINK version number
int    editnum = CFG_XLINK_VER_MIN;	// LINK edit number

long   offset;					// Current offset in RUN file
long   rdoffset;				// Offset in RUN file for start of relocation
								//   data
long   symtblsize;				// Number of symbol table entries
long   stbtotal;				// Number of in-memory debugger symbols
int    seggbn;					// Current global segment number
int    mscgbn;					// Number of msects
int    pscgbn;					// Current global psect number
int    symsize;					// Length of symbol in symbuf
int    mltcnt;					// Number of multiply defined symbols
int    undefcnt;				// Number of undefined symbols seen
long   outdd;					// Device descriptor for output file
long   symdd;					// Device descriptor for symbol table file
long   stradr;					// Starting address for program
struct pd *strpsd;				// Address of psect data block for starting
								//   address
long   stkadr;					// Initial stack address
struct pd *stkpsd;				// Address of psect data block for initial
								//   stack address
long   dbgadr;					// Debugger address for program
struct pd *dbgpsd;				// Address of psect data block for debugger
								//   address
struct pstk *polspnt;			// Polish expression stack pointer
struct pstk polstk[POLSSIZE];	// The polish expression stack
struct pd  abspsd;				// Psect data block used to link absolute
								//   symbols
struct pd *curpsd;				// Pointer to psect data block for psect
								//   being loaded
struct md *curmsd;
struct sd *cursgd;
struct pd *stbpsd;				// Pointer to psect data block for psect
								//   containing debugger symbol table
struct md *stbmsd;				// Pointer to msect data block for msect
								//   containing debugger symbol table
struct msgb *msgbhead;			// First MS group MSGB
struct msgb *msgbtail;			// Last MS group MSGB
struct pd **psnumpnt;
int    psnumcnt;
struct md **msnumpnt;
int    msnumcnt;
struct sd **segnumpnt;
int    segnumcnt;

uchar *msbuffer;
int    mscount;
ulong  msoffset;
void (*msdispatch)(void) = (void (*)(void))nullfunc;

ulong  stbsize;					// Space needed for debugger symbol names
ulong  stboffset;				// Offset in RUN file for next symbol entry
ulong  stbnum;					// Number of entries in the debugger symbol
								//   table
ulong  stbxsp;					// Number of extra entries in debugger symbol
								//   table
uint   seccnt;					// Byte count for section
uint   sectype;					// Section type
uint   sectypx;
long   objdd;					// Device descriptor for OBJ file
int    objcnt;					// Bytes left in current OBJ file block
uchar *objpnt;					// Pointer to OBJ file buffer
int    objamnt;

long   objblk;					// Current block number for OBJ file

struct objblk *firstobj;		// Pointer to first OBJ data block
struct objblk *curobj;			// Address of OBJ file block for current file

struct objblk *prevobj = (struct objblk *)&firstobj;
								// Address of previous OBJ file block
struct modblk *curmod;
char  *firstname;				// Pointer to copy of name of first OBJ file
char  *firstext;
char  *symbpnt;					// Symbol table file output pointer
int    symbcnt;					// Symbol table file output count
long   symlength;				// Symbol table file length
long   symtotal;				// Symbol table file number of entries
struct sd  *sgdhead;			// Segment data block list head pointer
struct sd  *sgdtail;			// Segment data block list tail pointer
struct sy **symnumbase;			// Current symbol number table base
struct sy  *hashtbl[HASHSIZE];	// The hash table

struct sy *importhead;
struct sy *importtail = (struct sy *)(&importhead);
long   importnum;				// Number of imported symbols
long   importnumoffset;			// Offset in file to store header pointers
long   importos;				// Offset in file to store imported symbol data
struct sy *exporthead;
struct sy *exporttail;
long   exportnum;
long   exporttotal;

struct _mab symnummem =
{   SYMNUMBASE, SYMNUMBASE, SYMNUMBASE};
struct _mab segnummem =
{   SEGNUMBASE, SEGNUMBASE, SEGNUMBASE};
struct _mab msectnummem =
{   MSECTNUMBASE, MSECTNUMBASE, MSECTNUMBASE};
struct _mab psectnummem =
{   PSECTNUMBASE, PSECTNUMBASE, PSECTNUMBASE};
struct _mab lnamesmem =
{   LNAMESBASE, LNAMESBASE, LNAMESBASE};

uchar *auxloc;					// Address of auxillary buffer
uchar *objloc;					// Address of OBJ file buffer
uchar *objbuf;					// Address of current input buffer
char  *symbufr;					// Address of symbol table file output buffer
char  *outfsp;					// Pointer to output file specification
char  *mapfsp;					// Pointer to map file specification
char  *symfsp;					// Pointer to symbol file specification
FILE  *mapfp;					// File pointer for map file
long   debgaddr;				// Address where debugger is loaded
int    opsymsz;					// Size of symbol table entry excluding name
								//   field, 0 if not loading symbol table
uchar  ptype;					// Processor type
char   itype = I_XOSUSER;		// Image type
char   opsymwrd;				// TRUE if symbol table entries must be word
								//   aligned
char   opsymord;				// TRUE if symbol table stored low order byte
								//   first
char    copymsg[] = "";
char   prgname[] = "XLINK";		// Our name
char   envname[] = "^XOS^XLINK^OPT"; // The environment option name
char   inpext[] = ".OBJ";		// Default extension for input files
char   outext[] = ".RUN";		// Default extension for output image file
char   symext[] = ".SYM";		// Default extension for symbol table file
char   mapext[] = ".MAP";		// Default extension for map file
char   cmdext[] = ".LNK";		// Default extension for command file
char   objtype;					// Type of current OBJ file
char   modtype;					// Type of current module
char   needrun = TRUE;			// TRUE if generating RUN file
char   needmap;					// TRUE if generating MAP file
char   needsym;					// TRUE if generating SYM file
char   lowfirst;				// TRUE if byte order is low order byte first
char   libflag;					// TRUE if loading from library file
char   mapflag;					// TRUE if map output is not to the console
char   sumflag;					// TRUE if want final summary displayed
char   debugflag;				// TRUE if want debug output to console
char   done;					// Done flag
char   errorflg;				// TRUE if have fatal error
char   symblk;					// True if more than 1 block in symbol table
								//   file
char   modname[SYMMAXSZ+3];		// Name of current module
char   symbuf[SYMMAXSZ+3];		// Symbol name buffer
char   haveweak = FALSE;		// TRUE if have weak symbols
char   havelazy = FALSE;		// TRUE if have lazy symbols

// Procarg option table

#define OPT(name) ((int (*)(arg_data *))name)

arg_spec options[] =
{   {"?"        , 0        , NULL, OPT(opthelp) , 0},
    {"ALO"      , 0        , NULL, OPT(optalone), 0},
    {"ALONE"    , 0        , NULL, OPT(optalone), 0},
    {"H"        , 0        , NULL, OPT(opthelp) , 0},
    {"HEL"      , 0        , NULL, OPT(opthelp) , 0},
    {"HELP"     , 0        , NULL, OPT(opthelp) , 0},
    {"LKE"      , 0        , NULL, OPT(optlke)  , 0},
    {"O"        , ASF_LSVAL, NULL,     optout   , TRUE},
    {"OUT"      , ASF_LSVAL, NULL,     optout   , TRUE},
    {"OUTPUT"   , ASF_LSVAL, NULL,     optout   , TRUE},
    {"M"        , ASF_LSVAL, NULL,     optmap   , 0},
    {"MAP"      , ASF_LSVAL, NULL,     optmap   , 0},
    {"NOO"      , 0        , NULL,     optout   , FALSE},
    {"NOOUT"    , 0        , NULL,     optout   , FALSE},
    {"NOOUTPUT" , 0        , NULL,     optout   , FALSE},
    {"SYM"      , ASF_LSVAL, NULL,     optsym   , 0},
    {"SYM86"    , ASF_NVAL , NULL,     opts86   , 0},
    {"SYM386"   , ASF_NVAL , NULL,     opts386  , 0},
    {"SYM68K"   , ASF_NVAL , NULL,     opts68k  , 0},
    {"SUM"      , 0        , NULL,     optsum   , TRUE},
    {"SUMMARY"  , 0        , NULL,     optsum   , TRUE},
    {"NOSUM"    , 0        , NULL,     optsum   , FALSE},
    {"NOSUMMARY", 0        , NULL,     optsum   , FALSE},
    {"DEBUG"    , 0        , NULL,     optdebug , TRUE},
    {"NODEBUG"  , 0        , NULL,     optdebug , FALSE},
    {NULL       , 0        , NULL,     NULL     , 0}
};

#if CFG_XLINK_MULTIPASS
int		g_cUndefPrev = 0;
int		g_nPass1Lev	 = 0;
#endif

//****************************************
// Function: main - Main program for XLINK
// Returned: 0 if normal, 1 if error
//****************************************

int main(
    int  argc,
    char *argv[])

{
    char *apnt[2];
    char  strbuf[256];

    fprintf(stderr, "XLINK - version %d.%d\n", version, editnum);
    if (argc >= 2)
    {
        if(svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
        {
            apnt[0] = strbuf;
            apnt[1] = NULL;
            procarg(apnt, PAF_INDIRECT, options, NULL, (int (*)(char *))NULL,
                    (void (*)(char *, char*))NULL, (int (*)(void))NULL, ".DFT");
        }
        ++argv;
        procarg(argv, PAF_INDIRECT|PAF_ECHOINDIR, options, NULL, havearg,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, cmdext);
    }
    if (firstobj == NULL)		// Did we get any files to load?
    {
        fputs("? XLINK: No input files specified\n", stderr);
        exit(1);
    }
    objloc = (uchar *)getspace(512);	// Allocate memory for input buffer

#if CFG_XLINK_MULTIPASS

	undefcnt	= 0;

	do
	{
		g_cUndefPrev	= undefcnt;		// Previous undefined count

//@@@	undefcnt		= 0;

    	passone();						// Do first pass

		fprintf( stderr, "%% XLINK:passone[%d] undefcnt=%d\n"
			, g_nPass1Lev, undefcnt );

		g_nPass1Lev++;

	} while( undefcnt && undefcnt != g_cUndefPrev && g_nPass1Lev < 7 );

#else
    passone();				// Do first pass
#endif

	if( needmap )			// If generating a map
		mapout();			// Generate the map

    passtwo();				// Do second pass

	if( sumflag )			// Want summary?
		dosummary();		// Yes

    if (mapfp)				// If generating a map
    {
        if (fclose(mapfp) < 0)		// Close map output
	    femsg(prgname, -errno, mapfsp);
    }
    return (((undefcnt+mltcnt)==0)? errorflg: 1);
}

//*************************************************
// Function: havearg - Process non-option arguments
// Returned: TRUE if normal, FALSE if error
//*************************************************

int havearg( char * arg )
{
	int		n = 1;

	if( strnicmp( arg, "XOSLIB:\\XOS\\LIBCB5", 18 ) == 0 )
		n = 5;

while( n-- > 0 )
{
    curobj = (struct objblk *)getfsp(offsetof(objblk, obj_fsp), arg, inpext);
					// Get file specification
    curobj->obj_next = 0;		// Initialize the OBJ block
    curobj->obj_mdl = 0;
    prevobj->obj_next = curobj;		// Link to previous OBJ block
    prevobj = curobj;
}

    return (TRUE);
}

//*******************************************
// Function: optout - Process /OUTPUT options
// Returned: TRUE if normal, FALSE if error
//*******************************************

int optout(
    arg_data *arg)

{
    needrun = (char)(arg->data);
    outfsp = optfile(arg, outext);
    return (TRUE);
}

//*****************************************
// Function: optmap - Process /MAP option
// Returned: TRUE if normal, FALSE if error
//*****************************************

int optmap(
    arg_data *arg)

{
    needmap = TRUE;
    mapfsp = (arg->flags & ADF_NULL)? "#": optfile(arg, mapext);
    return (TRUE);
}

//*****************************************
// Function: optsym - Process /SYM option
// Returned: TRUE if normal, FALSE if error
//*****************************************

int optsym(
    arg_data *arg)

{
    needsym = TRUE;
    symfsp = optfile(arg, symext);
    return (TRUE);
}

//*******************************************
// Function: optalone - Process /ALONE option
// Returned: TRUE if normal, FALSE if error
//*******************************************

int optalone(void)

{
    itype = I_ALONE;
    return (TRUE);
}

//*****************************************
// Function: optlke - Process /LKE option
// Returned: TRUE if normal, FALSE if error
//*****************************************

int optlke(void)

{
    itype = I_XOSLKE;
    return (TRUE);
}

//*****************************************
// Function: opts86 - Process /SYM86 option
// Returned: TRUE if normal, FALSE if error
//*****************************************

int opts86(
    arg_data *arg)

{
    stbxsp = arg->val.n;		// Get size of extra space for symbol
    opsymsz = 6;
    opsymord = TRUE;
    opsymwrd = FALSE;
    return (TRUE);
}

//*******************************************
// Function: opts386 - Process /SYM386 option
// Returned: TRUE if normal, FALSE if error
//*******************************************

int opts386(
    arg_data *arg)

{
    stbxsp = arg->val.n;		// Get size of extra space for symbol
    opsymsz = 8;
    opsymord = TRUE;
    opsymwrd = FALSE;
    return (TRUE);
}

//*******************************************
// Function: opts68k - Process /SYM68K option
// Returned: TRUE if normal, FALSE if error
//*******************************************

int opts68k(
    arg_data *arg)

{
    stbxsp = arg->val.n;		// Get size of extra space for symbol
    opsymsz = 8;
    opsymord = FALSE;
    opsymwrd = TRUE;
    return (TRUE);
}

//***********************************************************
// Function: optsum - Process /SUMMARY and /NOSUMMARY options
// Returned: TRUE if normal, FALSE if error
//***********************************************************

int optsum(
    arg_data *arg)

{
    sumflag = arg->data;
    return (TRUE);
}

//*********************************************************
// Function: optdebug - Process /DEBUG and /NODEBUG options
// Returned: TRUE if normal, FALSE if error
//*********************************************************

int optdebug(
    arg_data *arg)

{
    debugflag = arg->data;
    return (TRUE);
}

//*****************************************
// Function: opthelp - Process /HELP option
// Returned: TRUE if normal, FALSE if error
//*****************************************

void opthelp(void)

{
    fprintf(stderr, "\nXLINK version %d.%d (%s) %s\n\n", version, editnum,
            __DATE__, copymsg);
    fputs("XLINK {{-}{/}option{=val}} {file_specs} {{-}{/}option{=val} ...\n"
            " Options:\n", stderr);
    helpprint(" ?        - Displays this list", FALSE, TRUE);
    helpprint(" ALONE    - Specifies standalone image", (itype==I_ALONE), TRUE);
    helpprint(" HELP     - Displays this list", FALSE, TRUE);
    helpprint(" LKE      - Specifies LKE image", (itype==I_XOSLKE), TRUE);
    helpprint(" NOOUTPUT - Indicates no executable file output", FALSE, TRUE);
    helpprint(" OUTPUT   - Requests executable output file, specifies file",
            !needrun, TRUE);
    helpprint(" SYM      - Requests symbol table file, specifies file",
            (symfsp != NULL), TRUE);
    helpprint(" SYM86    - Requests in-memory 8086 format symbol table",
            (opsymsz == 6), TRUE);
    helpprint(" SYM386   - Requests in-memory 80386 format symbol table",
            (opsymsz == 8 && opsymord == TRUE), TRUE);
    helpprint(" SYM68K   - Requests in-memory 68000 format symbol table",
            (opsymsz == 8 && opsymord == FALSE), TRUE);
    fputs("All options except SYM86, SYM386, and SYM68K can be abbreviated to"
            " 3 letters.\n  HELP, MAP, and OUTPUT can be abbreviated to 1"
            " letter.\n", stderr);
    fputs("Use the DEFAULT command to change any default option. (* indicates"
            " current\n  default)\n", stderr);
    exit (0);
}

//************************************************
// Function: helpprint - Print help option entries
// Returned: Nothing
//************************************************

void helpprint(
    char *helpstring,
    int state,
    int newline)

{
    char strbuf[132];

    if (state)
    {
        strmov(strmov(strbuf, helpstring), " *");
        helpstring = strbuf;
    }
    fprintf(stderr, (newline)? "%s\n": "%-38s", helpstring);
}

//******************************************************
// Function: optfile - Get file specification for option
// Returned: Pointer to file spec or NULL if none
//******************************************************

char *optfile(
    arg_data *arg,
    char     *dftext)		// Default extension

{
    if (arg->flags & ADF_LSVAL)		// Was a file spec given?
	return (getfsp(0, arg->val.s, dftext)); // Yes - return it
    else
	return (NULL);			// No - return NULL
}

//******************************************
// Function: getfsp - Get file specification
// Returned: Pointer to OBJ block
//******************************************

char *getfsp(
    int   offset,		// Offset from start of block for name
    char *fsp,
    char *dftext)		// Default extension

{
    char *pnt1;
    char *pnt2;
    char *blk;
    long  size;
    char  haveext;

    haveext = (strchr(fsp, '.') != NULL);
    size = offset + strlen(fsp) + ((haveext)? 1: 5);
    blk = getspace(size);		// Get memory for OBJ block
    pnt1 = blk + offset;		// Point to place to put file spec
    pnt2 = strmov(pnt1, fsp);
    if (!haveext)
        strmov(pnt2, dftext);
    strupr(pnt1);
    return (blk);
}
