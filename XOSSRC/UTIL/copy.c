//--------------------------------------------------------------------------*
// COPY.C
// File copy utility for XOS
//
// Written by John Goltz
//
//--------------------------------------------------------------------------*

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

// This program is a complete rewrite of the XOS copy utility.  It supports
//   most of the features from the previous (3.6) and adds several new
//   features and major bug fixes:
//	Ellipsis copies work correctly when copying directories with no files
//	/ERROR option added to control error handling on wild-card copies
//	/Y option added which is the same as the DOS COPY /Y option
//	/EMPTY option added to control creating empty directories when
//	   doing ellipsis copy
//	/NOTOTALS option added to easily suppress totals
//	/QUIET and /VERBOSE work better

// 4.0 11-Mar-99 First version after rewrite

#include <CTYPE.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <TIME.H>
#include <XOSERR.H>
#include <XOSTRM.H>
#include <XOSRTN.H>
#include <XOS.H>
#include <XOSERRMSG.H>
#include <XOSSVC.H>
#include <XOSTIME.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <DIRSCAN.H>
#include <GLOBAL.H>
#include <XOSSTR.H>

typedef struct fs FS;
struct  fs
{   FS *next;
    s8  name[4];
};

typedef struct fd FD;
struct  fd
{   FD *nxtfile;
    FD *nxtdir;
    s8 *dstspec;
    s32 level;
    s8  dironly;
    s8  name[1];
};

int version = 4;
int editno  = 1;

long  srcdev = -1;		// Source device handle
long  dstdev = -1;			// Destination device handle

char *spintbl[] = {"\b/", "\b-", "\b\\", "\b|"};
long  spin;

long   bytecnt;
long   result[2];
time_s starttime;
time_s stoptime;
time_s xfertime;

FS    dfltspec = {NULL, "*.*"};
FS   *srcspec;
FS   *thisspec;
FS   *dstspec;

FD   *firstdir;
FD   *lastdir;
FD   *firstfile;
FD   *lastfile;
FD   *thisfile;

u8    domove;			// TRUE if doing MOVE command
u8    srcwild;			// TRUE if have source wild-card character
u8    dstnamewild;		// TRUE if wild-card in destination name
u8    dstextwild;		// TRUE if wild-card in destination extension
u8    isterm;			// TRUE if stdout is controlling terminal
u8    doconcat;			// TRUE if doing concatanation
u8    concatnxt;		// TRUE if saw + last in command line
u8    difdev;			// TRUE if MOVEing to different device

s32   filecnt;
s32   dircnt;

char *srcellipbgn;
char *dstellipbgn;


char  owner[36];

char *ellipstr;

struct
{   byte4_parm  filoptn;
    lngstr_parm filspec;
    byte4_parm  length;
    byte8_parm  cdate;
    byte8_parm  mdate;
    char        end;
} srcparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN},
    {PAR_GET|REP_STR , 0, IOPAR_FILSPEC, NULL, 0, 512, 512},
    {PAR_GET|REP_DECV, 4, IOPAR_LENGTH},
    {PAR_GET|REP_HEXV, 8, IOPAR_CDATE},
    {PAR_GET|REP_HEXV, 8, IOPAR_MDATE}
};

struct
{   byte4_parm  filoptn;
    lngstr_parm filspec;
    byte8_parm  cdate;
    byte8_parm  mdate;
    char        end;
} dstparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN},
    {PAR_GET|REP_STR , 0, IOPAR_FILSPEC, NULL, 0, 512, 512},
    {PAR_SET|REP_HEXV, 8, IOPAR_CDATE},
    {PAR_SET|REP_HEXV, 8, IOPAR_MDATE}
};

struct
{   byte4_parm  filoptn;
    lngstr_parm filspec;
    char        end;
} renparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, 0},
    {PAR_GET|REP_STR , 0, IOPAR_FILSPEC, NULL, 0, 512, 512},
};

int    deloutfile = FALSE;
int    image = FALSE;
int    copysize = 16*1024;
uchar *copybufr;
uchar  totalbytes = FALSE;
uchar  rate = FALSE;

char   *prgname = "COPY";	// Our programe name
char   *envname = "^XOS^COPY^OPT"; // The environment option name
char   *example = "{{/}{-}option{=sub}} {...\\}source1{+sourcen} "
	"{...\\}destination";
char   *description = "This program will copy one or more files from most "
	"standard devices.";

struct 
{   byte4_parm  filoptn;
    lngstr_parm filspec;
    byte2_parm  srcattr;
    u8          end;
} ellipparm =
{   {PAR_SET|REP_HEXV, 4 , IOPAR_FILOPTN, FO_NOPREFIX|FO_VOLNAME|FO_NODENUM|
	    FO_NODENAME|FO_RVOLNAME|FO_PATHNAME|FO_FILENAME},
    {PAR_GET|REP_STR , 0 , IOPAR_FILSPEC, NULL, 0, 512, 512},
    {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR, A_DIRECT}
};

struct
{   byte4_parm  filoptn;
    lngstr_parm filspec;
    byte2_parm  srcattr;
    byte2_parm  filattr;
    byte4_parm  dirhndl;
    u8          end;
} fileparm =
{   {PAR_SET|REP_HEXV, 4 , IOPAR_FILOPTN, FO_VOLNAME|FO_NODENUM|FO_NODENAME|
            FO_RVOLNAME|FO_PATHNAME|FO_FILENAME|FO_ATTR},
    {PAR_GET|REP_STR , 0 , IOPAR_FILSPEC, NULL, 0, 0, 0},
    {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR, A_NORMAL},
    {PAR_GET|REP_HEXV, 2 , IOPAR_FILATTR, 0},
    {PAR_SET|REP_HEXV, 4 , IOPAR_DIRHNDL, 0}
};

int    select = FALSE;
uchar  dosort = TRUE;
int    quiet = FALSE;
int    verbose = FALSE;
int    overwrite = FALSE;
int    erraction = -1;
int    crempty = TRUE;

uchar  totalfiles = FALSE;

extern int _malloc_amount;
int    maxmem;

Prog_Info pib;

// Function prototypes

int   askuser(char *msg, int quit);
void  closeoutfile(int last);
int   comp(FD *one, FD *two);
void  copyfile(int dironly, char *srcstr, char *dststr, int last);
int   dorename(char *src, char *dst);
void  errordel(char *delspec, int last);
void  errormsg(const char *arg, long  code);
void  fileerror(int nl, long errcode, char *text, char *name, char *delspec,
    int last);
char *findname(char *bgn);
void  finish(void);
void *getmem(size_t size);
long  makedir(char *spec, long level);
int   nonopt(char *arg);
int   optbuffer(arg_data *);
int   opterror(arg_data *);
void  opthelp(void);
int   optnodir(void);
int   optnototals(arg_data *);
int   opttotals(arg_data *);
void  procdir(void);
int   procfile(void);
void  showstatus(void);

// dirscan data

DIRSCANDATA dsd =
{   (DIRSCANPL *)&fileparm,
			// parmlist    - Address of parameter list
    TRUE,		// repeat      - TRUE if should do repeated search
    FALSE,		// dfltwildext - TRUE if want wildcard as default
    QFNC_DEVPARM,	// function    - Value for low byte of QAB_FUNC
    DSSORT_ASCEN,	// sort        - Directory sort order
    NULL,               // hvellip     - Function called when ellipsis specified
    (int (*)(DIRSCANDATA *))procfile,
			// func        - Function called for each file matched
    errormsg		// error       - Function called on error
};

// progarg data

SubOpts totalscmd[] =
{   {"F*ILES"  , "Display total number of files copied"},
    {"NOF*ILES", "Don't display total number of files copied"},
    {"B*YTES"  , "Display total number of bytes copied"},
    {"NOB*YTES", "Don't display total number of bytes copied"},
    {"R*ATE"   , "Display total transfer rate for copy"},
    {"NOR*ATE" , "Don't display total transfer rate for copy"},
    {NULL}
};

SubOpts errorcmd[] =
{   {"Q*UIT"    , "Quit copying after error"},
    {"A*SK"     , "Ask user for instructions after error"},
    {"C*ONTINUE", "Continue copying after error"},
    {NULL}
};

arg_spec options[] =
{
//  {"BE*FORE"  , ASF_VALREQ|ASF_LSVAL, NULL     ,    optbefore   , 0    ,
//	    "Transfer all matching files before this Date/Time"},
//  {"SI*NCE"   , ASF_VALREQ|ASF_LSVAL, NULL     ,    optsince    , 0    ,
//	    "Transfer all matching files since this Date/Time"},
    {"BU*FFER"  , ASF_VALREQ|ASF_NVAL , NULL     ,    optbuffer   , 0    ,
	    "Specify data buffer size"},
    {"S*ELECT"  , ASF_BOOL|ASF_STORE  , NULL     ,   &select      , TRUE ,
	    "Select which files to transfer"},
    {"O*VER"    , ASF_BOOL|ASF_STORE  , NULL     ,   &overwrite   , TRUE ,
	    "Overwrite overwritting existing files without confirmation"},
    {"EMP*TY"   , ASF_BOOL|ASF_STORE  , NULL     ,   &crempty     , TRUE ,
	    "Create empty directries during ellipsis transfer"},
    {"D*ELETE"  , ASF_BOOL|ASF_STORE  , NULL     ,   &deloutfile  , TRUE ,
	    "Delete incomplete output files"},
    {"ERR*OR"   , ASF_XSVAL           , errorcmd ,   opterror     , 0    ,
	    "Specify action to take on error"},
    {"DOS*DRIVE", ASF_BOOL|ASF_STORE  , NULL     ,   &gcp_dosdrive, TRUE ,
	    "Override the global DOSDRIVE setting"},
    {"I*MAGE"   , ASF_BOOL|ASF_STORE  , NULL     ,   &image       , TRUE ,
	    "Enable image mode (non-file) I/O"},
    {"Q*UIET"   , ASF_BOOL|ASF_STORE  , NULL     ,   &quiet       , TRUE ,
	    "Don't display any status"},
    {"V*ERBOSE" , ASF_BOOL|ASF_STORE  , NULL     ,   &verbose     , TRUE ,
	    "Display full status"},
    {"SO*RT"    , ASF_BOOL|ASF_STORE  , NULL     ,   &dosort      , TRUE ,
	    "Sort file list before transferring"},
    {"T*OTALS"  , ASF_XSVAL           , totalscmd,    opttotals   , 0    ,
	    "Display summary when complete"},
    {"NOT*OTALS", 0                   , NULL     ,    optnototals , 0    ,
	    "Don't display summary"},
    {"H*ELP"    , 0                   , NULL     , AF(opthelp)    , 0    ,
	    "Display this message"},
    {"?"        , 0                   , NULL     , AF(opthelp)    , 0    ,
	    "Display this message"},
    {NULL}
};

//***************************************
// Function: main - Main program function
// Returned: Never returns
//***************************************

void main(
    int   argc,
    char *argv[])

{
    char *envpnt[2];
    char *pnt;
    char *dstname;
    char *dstext;
    int   rtn;
    char  chr;
    char  srcrtn[512];
    char  dstrtn[512];
    char  strbuf[512];		// String buffer

    if (strnicmp(argv[0], "MOVE", 4) == 0)
    {
	domove = TRUE;
	prgname = "MOVE";
	envname = "^XOS^MOVE^OPT";
	example = "{{/}{-}option{=sub}} {...\\}source {...\\}destination";
	description = "This program will rename one or more files within "
		"the same directory or between different directories or will "
		"move one or more files between different devices. In all "
		"cases, the source file(s) is deleted.";
    }

    // Set defaults

    reg_pib(&pib);

    // Set Program Information Block variables

    pib.opttbl = options; 		// Load the option table
    pib.kwdtbl = NULL;
    pib.build = __DATE__;
    pib.majedt = version; 		// Major edit number
    pib.minedt = editno; 		// Minor edit number
    pib.copymsg = "";
    pib.prgname = prgname;
    pib.desc = description;
    pib.example = example;
    pib.errno = 0;
    getTrmParms();
    getHelpClr();

    srcparms.filspec.buffer = srcrtn;
    dstparms.filspec.buffer = renparms.filspec.buffer = dstrtn;

    // Check Global Configuration Parameters

    global_parameter(TRUE);

    // Check Local Configuration Parameters

    if(svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
    {
	envpnt[0] = strbuf;
	envpnt[1] = NULL;
        progarg(envpnt, 0, options, NULL, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    isterm = fisctrm(stdout);

    // Process the command line

    if (argc >= 2)
    {
	++argv;
        progarg(argv, 0, options, NULL, nonopt,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    if (dstspec == NULL)		// Destination specified?
	dstspec = &dfltspec;		// No - use default

    // Determine if we have any wild-card characters in any source specs.
    //   (This is very simple check.  If there are wild-card characters in
    //   the device name or directory fields, we will see them here.  In
    //   this case, we just let dirscan complain about the illegal wild-card
    //   usage!)  Note that source wild-cards are NOT allowed if concatanation
    //   is specified.

    // Destination wild cards are allowed only to indicate complete fields.
    //   That is, they can specify the source path, name, or extension.  When
    //   a destination wild card is specified, no other character can appear
    //   in the name or extension field.  The * must be the last character
    //   in the path field.  Only one ... is allowed in the destination path
    //   and it must be at the end of the path specification.  It means that
    //   it will be replaced with whatever matched the source ...
    //   specification, starting with the first source ... .

    if ((thisspec = srcspec) == NULL)
    {
	fprintf(stderr, "? %s: No files specified, type %s /H for help\n",
		prgname, prgname);
	exit(0);
    }
    do
    {
	pnt = thisspec->name;
	while ((chr = *pnt++) != 0)
	{
	    if (chr == '*' || chr == '?')
		srcwild = TRUE;
	    if (chr == '.' && pnt[0] != 0 && pnt[0] == '.' && pnt[1] != 0 &&
		    pnt[1] == '.' && (pnt[2] == '\\' || pnt[2] == '/'))
	    {
		if (srcellipbgn == NULL)
		    srcellipbgn = pnt - 1;
	    }
	}
    } while ((thisspec = thisspec->next) != NULL);

    pnt = dstname = findname(dstspec->name); // Find start of destination name
    if (*pnt == 0)			// Null name?
    {
	strmov(pnt, "*.*");		// Yes
	dstnamewild = dstextwild = TRUE;
    }
    else
    {
	dstext = NULL;			// Find start of destination extension
	while ((chr = *pnt++) != 0)
	{
	    if (chr == '.')
		dstext = pnt;
	}
	if (dstext == NULL)
	    dstext = pnt;

	if (dstname[0] == '*')		// Wild card destination name?
	{
	    if ((dstext - dstname) != 2 && dstname[1] != 0)
	    {				// Yes - is it valid?
		fprintf(stderr, "? %s: Illegal wild-card usage in destination "
			"name\n", prgname);
		exit (1);
	    }
	    dstnamewild = TRUE;
	}
	else				// If destination name is not wild
	    dstnamewild = FALSE;
	if (dstext[0] == '*')		// Wild card destination extension?
	{
	    if (dstext[1] != 0)		// Yes - is it valid?
	    {				// No
		fprintf(stderr, "? %s: Illegal wild-card usage in destination "
			"extension\n", prgname);
		exit (1);
	    }
	    dstextwild = TRUE;
	}
	else
	    dstextwild = FALSE;
    }
    copybufr = getmem(copysize);	// Allocate our data buffer
    srcparms.filoptn.value = FO_NOPREFIX|FO_FILENAME;
    if (verbose)
	srcparms.filoptn.value |= ((gcp_dosdrive) ?
		FO_DOSNAME|FO_NODENUM|FO_NODENAME|FO_RVOLNAME|FO_PATHNAME :
		FO_XOSNAME|FO_NODENUM|FO_NODENAME|FO_RVOLNAME|FO_PATHNAME);
    dstparms.filoptn.value = (gcp_dosdrive) ?
	    FO_NOPREFIX|FO_DOSNAME|FO_NODENUM|FO_NODENAME|FO_RVOLNAME|
		    FO_PATHNAME|FO_FILENAME :
	    FO_NOPREFIX|FO_XOSNAME|FO_NODENUM|FO_NODENAME|FO_RVOLNAME|
		    FO_PATHNAME|FO_FILENAME;
    renparms.filoptn.value = (gcp_dosdrive) ?
	    FO_DOSNAME|FO_NODENUM|FO_NODENAME|FO_RVOLNAME|FO_PATHNAME|
		    FO_FILENAME :
	    FO_XOSNAME|FO_NODENUM|FO_NODENAME|FO_RVOLNAME|FO_PATHNAME|
		    FO_FILENAME;
    if (rate)
        svcSysDateTime(T_GTHRDTTM, &starttime);
    if (srcwild || srcellipbgn != NULL)	// Did we find a source wild-card?
    {					// Yes
	if (doconcat)			// Doing concatanation?
	{
	    fprintf(stderr, "? %s: Cannot use wild-cards with concatanation\n",
		    prgname);
	    exit(1);
	}
	if (srcellipbgn != NULL)	// Have source ellipsis?
	{
	    // Here if have source ellipsis - We must find if we have a
	    // destination ellipsis and if it is valid and where it begins.

	    if (dstspec != NULL)
	    {
		pnt = dstspec->name;
		while ((chr = *pnt++) != 0)
		{
		    if (chr == '.' && pnt[0] != 0 && pnt[0] == '.' &&
			    pnt[1] != 0 && pnt[1] == '.' &&
			    (pnt[2] == '\\' || pnt[2] == '/'))
		    {
			dstellipbgn = pnt - 1;
			pnt += 3;
			while ((chr = *pnt++) != 0)
			{
			    if (chr == '\\' || chr == '/' || chr == ':')
			    {
				fprintf(stderr, "? %s: Invalid ellipsis in "
					"destination specification", prgname);
				exit(1);
			    }
			}
		    }
		}
	    }

	    // Now we must determine the full device and path up to the start
	    //   of the source ellipsis.  We do this by opening this directory.

	    chr = *srcellipbgn;
	    *srcellipbgn = 0;
	    ellipparm.filspec.buffer = strbuf;
	    if ((rtn = svcIoDevParm(O_ODF, srcspec->name, &ellipparm)) < 0)
		femsg2(prgname, "Cannot open source directory for search",
			rtn, NULL);
	    ellipstr = getmem(ellipparm.filspec.strlen + 1);
	    strmov(ellipstr, strbuf);
	    *srcellipbgn = chr;
	}
	dirscan(srcspec->name, &dsd);	// Scan the directory
	if (firstfile != NULL)		// Have any files in the last directory?
	    procdir();			// Yes

	if (firstdir == NULL)
	{
	    erraction = 0;
	    fileerror(FALSE, ER_FILNF, "Error opening source file",
		    srcspec->name, NULL, TRUE);
	}
	else
	{
	    do
	    {
		thisfile = firstdir;
		while (thisfile != NULL)
		{
		    copyfile(thisfile->dironly, thisfile->name,
			    (thisfile->dstspec != NULL) ?
			    thisfile->dstspec : dstspec->name,
			    (thisfile->nxtfile == NULL &&
			    thisfile->nxtdir == NULL));
		    thisfile = thisfile->nxtfile;
		}
	    } while ((firstdir = firstdir->nxtdir) != NULL);
	}
    }
    else
    {
	if (doconcat)
	{
	    thisspec = srcspec;
	    while (thisspec != NULL)
	    {
		copyfile(FALSE, thisspec->name, dstspec->name, FALSE);
		thisspec = thisspec->next;
	    }
	}
	else				// No wild cards or ellipsis or
					//   concatination - just copy the
					//   single file
	    copyfile(FALSE, srcspec->name, dstspec->name, TRUE);
    }
    finish();				// Display final summary if need to
}

//**************************************************
// Function: finish - Display final summary and exit
// Returend: Never returns
//**************************************************

static void finish(void)

{
    if (dstdev > 0)			// Make sure the output file is closed
		closeoutfile(TRUE);
    if (domove && !difdev)
		totalbytes = rate = FALSE;
    if (totalbytes | totalfiles | rate)
    {
		fputs("\t", stdout);
		if (rate)
			svcSysDateTime(T_GTHRDTTM, &stoptime);
		if (totalfiles)
		{
			if (dircnt > 0)
				printf("%,d directr%s created\n\t", dircnt, (dircnt != 1) ?
						"ies" : "y");
			printf("%,d file%s %sed%s ", filecnt, (filecnt != 1) ? "s" : "",
					(domove) ? "mov" : "copi", (totalbytes) ? "," : "");
		}
		if (totalbytes)
			printf("%,d byte%s ", bytecnt, (bytecnt != 1) ? "s" : "");
		if (rate)
		{
			sdtsub(&xfertime, &stoptime, &starttime);
			if (bytecnt != 0 && xfertime.time != 0)
				longdiv(result, bytecnt, XT_SECOND, xfertime.time);
			else
				result[0] = 0;
			if (result[0] != 0)
				printf("(%,u byte%s/sec)", result[0], (result[0] != 1) ?
						"s" : "");
		}
		fputs("\n", stdout);
	}
	exit(0);				// Return with no error
}

//****************************************
// Function: copyfile - Copy a single file
// Returned: Nothing
//****************************************

// All source wild cards have been resolved before this function is called.
//   Destination wild cards are resolved here.

static void copyfile(
    int   dironly,		// TRUE if reports empty directory
    char *srcstr,		// Source string
    char *dststr,		// Destination string
    int   last)			// TRUE if this is the last file to copy

{
    char *srcname;
    char *srcext;
    char *dstname;
    char *dstext;
    char *pnt;
    long  rtn;
    long  bits;
    long  amount;
    char  chr;
    char  dst[512];

    if (dironly)			// Have empty directory?
    {
	if (crempty)			// Yes - should we create it?
	{
	    if ((rtn = makedir(dststr, thisfile->level)) < 0 &&
		    rtn != ER_FILEX)
		fileerror(FALSE, rtn, "Error creating directory", dststr,
			NULL, last);
	}
	return;
    }
    if (select && (srcwild || srcellipbgn != NULL))
    {
	sprintf(copybufr, (domove) ? "Move file %s" : "Copy file %s", srcstr);
	switch(askuser(copybufr, TRUE))
	{
	 case 0:			// No
	    svcIoClose(srcdev, 0);
	    srcdev = -1;
	    return;

	 case 2:			// All
	    select = FALSE;
	 case 1:			// Yes
	    break;
	}
    }
    if (domove || dstdev < 0)		// Need to construct destination
    {					//   specification?
	pnt = dstname = findname(dststr); // Yes - find start of destination
					  //   name
	if (dstnamewild | dstextwild)	// Have a destination wild card?
	{
	    srcname = pnt = findname(srcstr); // Yes - find start of source
	    srcext = NULL;		       //   name
	    while ((chr = *pnt) != 0)	// Find start of source extension
	    {
		pnt++;
		if (chr == '.')
		    srcext = pnt;
	    }
	    if (srcext == NULL)		// Null extension?
		srcext = pnt;		// Yes - point to end of string
	    pnt = dststr;		// Find start of destination extension
	    dstext = NULL;
	    while ((chr = *pnt) != 0)
	    {
		pnt++;
		if (chr == '.')
		    dstext = pnt;
	    }
	    if (dstext == NULL)		// Null extension?
		dstext = pnt;		// Yes - point to end of string
	    pnt = strnmov(dst, dststr, dstname - dststr);
					// Copy destination path
	    if (dstnamewild)		// Wild card destination name?
		pnt = strnmov(pnt, srcname, srcext - srcname);
	    else			// If destination name is not wild
		pnt = strnmov(pnt, dstname, dstext - dstname);
	    strmov(pnt, (dstextwild) ? srcext : dstext);
	    dststr = dst;
	}
    }
    if (domove && !difdev)
    {
	if ((rtn = dorename(srcstr, dststr)) >= 0)
	{
	    showstatus();
	    return;
	}
	if (rtn == ER_DFDEV)
	    difdev = TRUE;
	else
	{
	    if (dstellipbgn != NULL && rtn == ER_DIRNF && thisfile->level > 0)
	    {
		if ((rtn = makedir(dststr, thisfile->level)) < 0)
		{
		    fileerror(FALSE, rtn, "Error creating directory for",
			    dststr, NULL, last);
		    return;
		}
		rtn = dorename(srcstr, dststr);
	    }
	    if (rtn == ER_FILEX)
	    {
		if (!overwrite)
		{
		    sprintf((char *)(srcparms.filspec.buffer), "Replace %s",
			    (char *)(dstparms.filspec.buffer));
		    switch (askuser((char *)(srcparms.filspec.buffer), TRUE))
		    {
		     case 0:		// No
			return;

		     case 2:		// All
			overwrite = TRUE;
		     case 1:		// Yes
			break;
		    }
		    if (!difdev && (rtn = svcIoDelete(0, dststr, NULL)) < 0)
		    {
			fileerror(FALSE, rtn, "Error deleting existing "
				"destination file", dststr, NULL, last);
			return;
		    }
		    rtn = dorename(srcstr, dststr);
		}
	    }
	    if (rtn < 0)
	    {
		fileerror(FALSE, rtn, "Error renaming file", srcstr, NULL,
			last);
		return;
	    }
	}
    }

    // Here if doing COPY or MOVE to different device

    rtn = O_IN;				// Open the source file
    if (image)
    {
	amount = strlen(srcstr) - 1;
	if (amount < 0 || srcstr[amount] == '\\' || srcstr[amount] == '/' ||
		srcstr[amount] == ':')
	    rtn = O_IN|O_PHYS;
    }
    if ((srcdev = svcIoOpen(rtn, srcstr, &srcparms)) < 0)
    {
	fileerror(FALSE, srcdev, "Error opening source file", srcstr,
		(dstdev > 0) ? dststr : NULL, last);
	return;
    }
    if (dstdev < 0)			// Need to open the destination?
    {

	dstparms.cdate.value[0] = srcparms.cdate.value[0];
	dstparms.cdate.value[1] = srcparms.cdate.value[1];
	dstparms.mdate.value[0] = srcparms.mdate.value[0];
	dstparms.mdate.value[1] = srcparms.mdate.value[1];
	bits = O_OUT|O_CREATE|O_TRUNCW|O_XWRITE|O_XREAD;
	if (image)
	{
	    amount = strlen(dststr) - 1;
	    if (amount < 0 || dststr[amount] == '\\' ||
		    dststr[amount] == '/' || dststr[amount] == ':')
		bits = O_OUT|O_PHYS;
	}
	else if (!overwrite)
	    bits |= O_FAILEX;

	if ((dstdev = svcIoOpen(bits, dststr, &dstparms))
		< 0)
	{
	    if (dstellipbgn != NULL && dstdev == ER_DIRNF && thisfile->level
		    > 0)
	    {
		if ((rtn = makedir(dststr, thisfile->level)) < 0)
		{
		    fileerror(FALSE, rtn, "Error creating directory for file",
			    dststr, NULL, last);
		    return;
		}
		dstdev = svcIoOpen(bits, dststr, &dstparms);
	    }
	    if (dstdev == ER_FILEX)
	    {
		if (!overwrite)
		{
		    sprintf(copybufr, "Overwrite %s",
			    dststr);
		    switch (askuser(copybufr, TRUE))
		    {
		     case 0:		// No
			return;

		     case 2:		// All
			overwrite = TRUE;
			bits &= ~O_FAILEX;
		     case 1:		// Yes
			dstdev = svcIoOpen(bits & ~O_FAILEX, dststr, &dstparms);
			break;
		    }
		}
	    }
	    if (dstdev < 0)
	    {
		fileerror(FALSE, dstdev, "Error creating destination file",
			dststr, NULL, last);
		return;
	    }
	}
	if (isterm && isctrm(dstdev))
	    isterm = FALSE;
    }
    filecnt++;
    spin = 0;
    showstatus();
    while ((amount = svcIoInBlock(srcdev, copybufr, copysize)) > 0)
    {
		if ((rtn = svcIoOutBlock(dstdev, copybufr, amount)) != amount)
		{
			if (rtn >= 0)
				rtn = ER_ICMIO;
			fileerror(TRUE, rtn, "Error writing destination file",
					(char *)(dstparms.filspec.buffer),
					(char *)(dstparms.filspec.buffer), last);
			return;
		}
		bytecnt += amount;
		if (isterm && verbose && !quiet)
		{
			fputs(spintbl[spin], stdout);	
			spin = (spin + 1) & 0x03;
		}
	}
	if (amount != 0 && amount != ER_EOF)
    {
		fileerror(TRUE, amount, "Error reading source file",
				(char *)(srcparms.filspec.buffer),
				(char *)(dstparms.filspec.buffer), last);
		return;
	}
	if (!doconcat)
		closeoutfile(last);
    if (domove)
    {
		if ((rtn = svcIoClose(srcdev, C_DELETE)) < 0)
		{
			srcdev = -1;
			fileerror(FALSE, rtn, "Error deleting source file",
					(char *)(srcparms.filspec.buffer), NULL, last);
			return;
		}
    }
    else
		svcIoClose(srcdev, 0);
    srcdev = -1;
    if (isterm && verbose && !quiet)
		fputs("\b \b\b\n", stdout);
}

//****************************************************
// Function: dorename - Rename file for MOVE command
// Returned: 0 if normal, negative error code if error
//****************************************************

int dorename(
    char *src,
    char *dst)

{
    uchar *pnts;
    uchar *pntd;
    uchar *pntdev;
    long   rtn;
    uchar  uchr;

    if ((rtn = svcIoRename(0, src, dst, &renparms)) >= 0 && !quiet)
	{
		pntdev = NULL;
		pnts = (char * near)(srcparms.filspec.buffer);
		pntd = (char * near)(dstparms.filspec.buffer);
		while ((uchr = *pntd) != 0)
		{
	    	if (uchr < FS_MIN)
				*pnts++ = uchr;
		    else
		    {
				if (uchr <= FS_XOSNAME)
					pntdev = pntd;
				if (uchr == FS_NPATHNAME || uchr == FS_NFILENAME)
					break;
				else if (uchr == FS_ESC)
				{
					pntd++;
					if ((uchr = *pntd) != 0)
						*pnts++ = uchr;
				}
			}
			pntd++;
		}
		*pnts = 0;
		if (verbose)
		{
			pnts = (char * near)(dstparms.filspec.buffer);
			if (pntdev != NULL)
			{
				while ((uchr = *pntdev++) != 0)
				{
					if (uchr < FS_MIN)
						*pnts++ = uchr;
					else if (uchr == FS_ESC)
					{
						if ((uchr = *pntdev++) != 0)
							*pnts++ = uchr;
					}
					else if (uchr >= FS_PATHNAME)
						break;
				}
			}
			while ((uchr = *pntd++) != 0)
			{
				if (uchr < FS_MIN)
					*pnts++ = uchr;
				else if (uchr == FS_ESC)
				{
					if ((uchr = *pntd++) != 0)
						*pnts++ = uchr;
				}
			}
			*pnts = 0;
		}
	}
	return (rtn);
}

//*******************************************
// Function: showstatus - Display status line
// Returned: Nothing
//*******************************************

void showstatus(void)

{
    if (!quiet)
	{
		if (verbose)
			printf("%s %c> %s%s", (char * near)(srcparms.filspec.buffer),
					(domove) ? '=' : (doconcat) ? '+' : '-',
					(char * near)(dstparms.filspec.buffer),
					(isterm && (!domove || difdev)) ? " |" : "\n");
		else
			printf("%s\n", (char * near)(srcparms.filspec.buffer));
	}
}

//**************************************************************
// Function: procfile - Function called by dirscan for each file
// Returned: TRUE if should continue, FALSE if should terminate
//**************************************************************

static int procfile(void)

{
    char    *pnt;
    RMTDATA *rmt;
    int      len;

    // We build a list of files to copy in each directory.  When we get a
    //   different directry, we sort (if necessary) the list and copy the
    //   files.  Since we have to open each file to copy it, all we need
    //   in our list here is the file specification.

	if (dsd.changed && (firstfile != NULL))
		procdir();

    // Get total length of the file specification

    len = dsd.devnamelen + dsd.pathnamelen + dsd.filenamelen;
    rmt = &dsd.rmtdata;
    while (rmt != NULL)
    {
		len += (rmt->rmtdevnamelen + rmt->nodenamelen);
		rmt = rmt->next;
    }
    thisfile = getmem(sizeof(FD) + len);
    thisfile->dironly = (dsd.filenamelen == 0);
    pnt = strmov(thisfile->name, dsd.devname);
    rmt = &dsd.rmtdata;
    while (rmt != NULL)
    {
		pnt = strmov(strmov(pnt, rmt->nodename), rmt->rmtdevname);
		rmt = rmt->next;
    }
    strmov(strmov(pnt, dsd.pathname), dsd.filename);
    if (dsd.error < 0)			// Error reported by DIRSCAN?
		fileerror(FALSE, dsd.error, "Error searching directory for",
				thisfile->name, NULL, FALSE);
    if ((dsd.filenamelen) == 0 && dsd.level < 1)
    {
		free(thisfile);			// Forget it if no name at level 0
		return (TRUE);
	}
	thisfile->level = dsd.level;
	if (firstfile == NULL)
		firstfile = thisfile;
	else
		lastfile->nxtfile = thisfile;
	lastfile = thisfile;
	thisfile->nxtfile = NULL;
	return (TRUE);
}

//************************************************
// Function: comp - Compare two filenames for sort
// Returned: Negative if a < b
//	     Zero if a == b
//           Positive if a > b
//************************************************

static int comp(
    FD *one,
    FD *two)

{
	return (stricmp(one->name, two->name)); // Always sort on name
}

//*****************************************************
// Function: procdir - Process all files in a directory
// Returned: Nothing
//*****************************************************

void procdir(void)

{
    char *pnt;
    char *bgn;
    long  len;

    if (dosort && firstfile->nxtfile != NULL) // Should we sort the list?
		firstfile = heapsort(firstfile,	// Yes
				(int (*)(void *a, void *b, void *d))comp, NULL);
    thisfile = firstfile;
    while (thisfile != NULL)
    {
		thisfile->nxtdir = NULL;
		if (dstellipbgn != NULL)
		{
			if (strncmp(thisfile->name, ellipstr, ellipparm.filspec.strlen)
					!= 0)
			{
				fprintf(stderr, "? %s: Internal error: Ellipsis string does "
						"not match!\n", prgname);
				exit(1);
			}
			bgn = thisfile->name + ellipparm.filspec.strlen;
			len = findname(bgn) - bgn;
			pnt = getmem(dstellipbgn - dstspec->name + len +
					strlen(dstellipbgn + 4) + 1);
			thisfile->dstspec = pnt;
			pnt = strnmov(pnt, dstspec->name, dstellipbgn - dstspec->name);
			pnt = strnmov(pnt, bgn, len);
			strmov(pnt, dstellipbgn + 4);
		}
		else
			thisfile->dstspec = NULL;
		thisfile = thisfile->nxtfile;
    }
    if (firstdir == NULL)
		firstdir = firstfile;
    else
		lastdir->nxtdir = firstfile;
    lastdir = firstfile;
    firstfile = NULL;
}

//************************************************************
// Function: findname - Find name part of a file specification
// Returend: Pointer to first character of name
//************************************************************

char *findname(
    char *bgn)

{
    char *end;
    char  chr;

    end = bgn;
    while ((chr = *bgn++) != 0)
    {
		if (chr == ':' || chr == '\\' || chr == '/')
			end = bgn;
    }
    return (end);
}

//*******************************************
// Function: makedir - Create directory
// Returned: 0 if OK, negative error if error
//*******************************************

long makedir(
    char *spec,
    long  level)

{
    char *src;
    char *dst;
    char *end;
    long  rtn;
    char  chr;
    char  bufr[512];

	static struct
	{	byte4_parm  filoptn;
		lngstr_parm filspec;
		char        end;
    } dirparms =
	{	{PAR_SET|REP_HEXV, 4 ,IOPAR_FILOPTN ,0},
		{PAR_GET|REP_STR , 0 ,IOPAR_FILSPEC ,NULL, 0, 512, 512},
	};

    dirparms.filspec.buffer = bufr;
    dirparms.filoptn.value = (gcp_dosdrive) ?
		FO_NOPREFIX|FO_DOSNAME|FO_NODENUM|FO_NODENAME|FO_RVOLNAME|
				FO_PATHNAME|FO_FILENAME :
		FO_NOPREFIX|FO_XOSNAME|FO_NODENUM|FO_NODENAME|FO_RVOLNAME|
				FO_PATHNAME|FO_FILENAME;
    if ((rtn = svcIoDevParm(O_CREATE|O_ODF, spec, &dirparms)) < 0)
    {
		if (rtn == ER_DIRNF && --level > 0)
		{
			src = spec;
			dst = end = bufr;
			while ((chr = *src++) != 0)
			{
				*dst = chr;
				if (chr == '\\' || chr == '/')
					end = dst;
				dst++;
			}
			*end = 0;
			if ((rtn = makedir(bufr, level)) < 0)
				return (rtn);
			dirparms.filspec.buffer = bufr;
			dirparms.filoptn.value = (gcp_dosdrive) ?
				FO_NOPREFIX|FO_DOSNAME|FO_NODENUM|FO_NODENAME|FO_RVOLNAME|
						FO_PATHNAME|FO_FILENAME :
				FO_NOPREFIX|FO_XOSNAME|FO_NODENUM|FO_NODENAME|FO_RVOLNAME|
						FO_PATHNAME|FO_FILENAME;
			if ((rtn = svcIoDevParm(O_CREATE|O_ODF, spec, &dirparms)) < 0)
				return (rtn);
		}
		else
			return (rtn);
	}
	dircnt++;
	printf("Directory %s created\n", bufr);
	return (0);
}

//***************************************************
// Function: closeoutfile - Close the output file
// Returned: Nothing (does not return if final error)
//***************************************************

void closeoutfile(
    int last)			// TRUE if this is the last file to copy

{
    long rtn;

    if ((rtn = svcIoClose(dstdev, 0)) < 0)
    {
		dstdev = -1;
		fileerror(TRUE, rtn, "Error closing destination file", dstspec->name,
				dstspec->name, last);
		return;
    }
    dstdev = -1;
}

//****************************************
// Function: askuser - Ask user what to do
// Returned: 0 if No, 1 if Yes, 2 if All
//****************************************

static int askuser(
    char *msg,
    int   quit)

{
    long rtn;
    char bufr[] = {0, '\b', 0};

    static struct
    {	byte4_parm  oldtim;
		byte4_parm  clrtim;
		byte4_parm  settim;
	char        end;
    } trmparm1 =
    {	{PAR_GET|REP_HEXV, 4, IOPAR_TRMSINPMODE, 0},
		{PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0x7FFFFFFF},
		{PAR_SET|REP_DECV, 4, IOPAR_TRMSINPMODE, TIM_CHAR}
    };

    static struct
    {	byte4_parm  clrtim;
		byte4_parm  settim;
		char        end;
    } trmparm2 =
    {	{PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0x7FFFFFFF},
		{PAR_SET|REP_DECV, 4, IOPAR_TRMSINPMODE, 0}
    };

    time_s entertime;
    time_s leavetime;
    time_s difftime;

    svcSysDateTime(T_GTHRDTTM, &entertime);
    svcIoInBlockP(DH_STDIN, NULL, 0, &trmparm1);
    fprintf(stderr, "%s (Yes/No/All%s)? ", msg, (quit) ? "/Quit" : "");
    svcTrmFunction(DH_STDIN, TF_CLRINP);
    rtn = -1;
    for (;;)
    {
		svcIoInBlock(DH_STDIN, bufr, 1);
		bufr[0] = toupper(bufr[0]);
		if (bufr[0] == '\r' && rtn >= 0)
			break;
		else if (bufr[0] == 'N')
		{
			rtn = 0;
			goto echo;
		}
		else if (bufr[0] == 'Y')
		{
			rtn = 1;
			goto echo;
		}
		else if (bufr[0] == 'A')
		{
			rtn = 2;
			goto echo;
		}
		else if (!quit)
			continue;
		else if (bufr[0] == 'Q')
		{
			rtn = 3;
			goto echo;
		}
		continue;

		echo: fputs(bufr, stderr);
    }
    fputs("\n", stderr);
    trmparm2.settim.value = trmparm1.oldtim.value;
    svcIoInBlockP(DH_STDIN, NULL, 0, &trmparm2);
    svcTrmFunction(DH_STDIN, TF_CLRINP);
    svcSysDateTime(T_GTHRDTTM, &leavetime);
    sdtsub(&difftime, &leavetime, &entertime);
    sdtadd(&starttime, &starttime, &difftime);
    if (rtn == 3)
		finish();
    return (rtn);
}

//*************************************************
// Function: optbuffer - Process BUFFER size option
// Returned: TRUE
//*************************************************

static int optbuffer(
    arg_data *arg)

{
    copysize = ((arg->val.n + 511) / 512) * 512;
    if (copysize < 512)
    {
        fprintf(stderr, "\n? %s: Buffer size (%d) is too small\n",
		pib.prgname, copysize);
        exit(EXIT_MALLOC);
    }
    return (TRUE);
}

//*******************************************
// Function: opttotals - Proces TOTALS option
// Returned: TRUE
//*******************************************

static int opttotals(
     arg_data *arg)

{
    if ((arg->flags & ADF_XSVAL) == 0)
		return (TRUE);

    switch ((int)arg->val.n)
    {
     case 0:				// Files
		totalfiles = TRUE;
		break;

     case 1:				// Nofiles
		totalfiles = FALSE;
		break;

     case 2:				// Bytes
		totalbytes = TRUE;
		break;

     case 3:				// Nobytes
		totalbytes = FALSE;
		break;

     case 4:				// Rate
		rate = TRUE;
		break;

     case 5:				// Norate
		rate = FALSE;
		break;
    }
    return (TRUE);
}

//***********************************************
// Function: optnototals - Proces NOTOTALS option
// Returned: TRUE
//***********************************************

static int optnototals(
     arg_data *arg)

{
    arg = arg;

    totalbytes = totalfiles = rate = FALSE;
    return (TRUE);
}

//******************************************
// Function: opterror - Process ERROR option
// Returned: TRUE
//******************************************

int opterror(
    arg_data *arg)

{
    if ((arg->flags & ADF_XSVAL) == 0)
		return (TRUE);

    switch ((int)arg->val.n)
    {
     case 0:				// Stop
		erraction = 0;
		break;

     case 1:				// Ask
		erraction = -1;
		break;

     case 2:				// Continue
		erraction = 1;
		break;
    }
    return (TRUE);
}

//********************************************
// Function: nonopt - process non-option input
// Returned: Nothing
//********************************************

int nonopt(
    char *arg)

{
    FS *spec;
    int size;

    size = strlen(arg);			// Get length of argument
    if (size == 1 && arg[0] == '+')	// Is it +
    {					// Yes - this means concatanation
		if (concatnxt || srcspec == NULL || dstspec != NULL)
		{
			fprintf(stderr, "? %s: Illegal syntax for concatanation\n",
					prgname);
			exit(EXIT_INVSWT);
		}
		doconcat = TRUE;
		concatnxt = TRUE;
		return (TRUE);
    }
    spec = getmem(size + sizeof(FS));
    strmov(spec->name, arg);
    if (srcspec == NULL)
		srcspec = spec;
    else
    {
		if (concatnxt)
		{
			thisspec->next = spec;
			concatnxt = NULL;
		}
		else
		{
			if (dstspec != NULL)
			{
				fprintf(stderr, "? %s: Too many files specified\n", prgname);
				exit(EXIT_INVSWT);
			}
			dstspec = spec;
		}
    }
    spec->next = NULL;
    thisspec = spec;
    return (TRUE);
}

//**********************************************************
// Function: fileerror - Report errors associated with files
// Returned: Nothing, only returns if should continue
//**********************************************************

void fileerror(
    int   nl,			// TRUE if need initial NL
    long  errcode,		// XOS error code
    char *text,
    char *name,			// Name of file associated with error
    char *delspec,		// File spec for file to delete
    int   last)			// TRUE if this is the last file to copy

{
    int  action;
    char msg[100];

    // First display the error message

    svcSysErrMsg(errcode, 0x03, msg);
    fprintf(stderr, "%s? %s: %s %s\n        %s\n", (nl) ? "\n" : "", prgname,
			text, name, msg);

    // Close the source file if its open

    if (srcdev > 0)
    {
		svcIoClose(srcdev, 0);
		srcdev = -1;
    }

    // Then determine if we should see about continueing

    if (erraction != 0)
    {
		action = 0;
		if ((srcwild || srcellipbgn != NULL) && !last)
			action = (erraction < 0) ? askuser("% COPY: Continue copying",
					FALSE) : erraction;
		if (action == 2)
			erraction = 1;
		if (action != 0)
		{
			if (!doconcat)
				errordel(delspec, last);
			return;
		}
    }
    errordel(delspec, last);
    finish();
}

//*****************************************************************
// Function: errordel - Delete output file after error if necessary
// Returned: Nothing
//*****************************************************************

void errordel(
    char *delspec,
    int   last)

{
    long rtn;

    if (!deloutfile || delspec == NULL)	// Want to delete the output file?
		return;				// No
    if (dstdev > 0)			// Yes - make sure its closed
		closeoutfile(last);
    if ((rtn = svcIoDelete(0, delspec, NULL)) < 0) // Delete the file
    {					// If error
		char *msg[100];

		svcSysErrMsg(rtn, 0x03, msg);	// Tell him about it
		fprintf(stderr, "? %s: Error deleting partial output file\n"
				"        %s; %s\n", prgname, msg, delspec);
    }
}

//*****************************************************
// Function: getmem - Get memory with malloc, exit with
//		error if out of memory
// Returned: Pointer to memory obtained
//*****************************************************

void *getmem(
    size_t size)

{
    void *ptr;

    ptr = malloc(size);
    if (ptr == NULL)
    {
        fprintf(stderr, "? %s: Not enough memory available\n", prgname);
        exit(EXIT_MALLOC);
    }
    if (maxmem < _malloc_amount)
        maxmem = _malloc_amount;
    return (ptr);
}

//***********************************************************
// Function: errormsg - Display XOS error message for dirscan
// Returned: Never returns
//***********************************************************

static void errormsg(
    const char *arg,
    long  code)

{
    char buffer[100];		// Buffer to receive error message

    if (code != 0)
    {
		svcSysErrMsg(code, 3, buffer);	// Get error message
		fprintf(stderr, "\n? %s: %s%s%s\n", prgname, buffer, (arg[0] != '\0') ?
				"; ": "", arg);		// Output error message
    }
    else
		fprintf(stderr, "\n? %s: %s\n", prgname, arg);
    exit(1);
}
