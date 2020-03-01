//----------------------------
// RENAME.C
// File rename utility for XOS
//
// Written by John Goltz
//
//----------------------------

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

// This program is a complete rewrite of the XOS rename utility.

// 4.0 24-Mar-99 First version after rewrite (derived from new COPY.C)

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
    s32 level;
    s8  name[1];
};

int version = 4;
int editno  = 0;

FS    dfltspec = {NULL, "*.*"};
FS   *srcspec;
FS   *thisspec;
FS   *dstspec;

FD   *firstdir;
FD   *lastdir;
FD   *firstfile;
FD   *lastfile;
FD   *thisfile;

u8    srcwild;			// TRUE if have source wild-card character
u8    dstnamewild;		// TRUE if wild-card in destination name
u8    dstextwild;		// TRUE if wild-card in destination extension

s32   filecnt;

char *srcellipbgn;
char *dstellipbgn;

char *ellipstr;

char *srcbufr;

struct
{   byte4_parm  filoptn;
    lngstr_parm filspec;
    byte1_parm  srcattr;
    char        end;
} renparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, 0},
    {PAR_GET|REP_STR , 0, IOPAR_FILSPEC, NULL, 0, 512, 512},
    {PAR_SET|REP_HEXV, 1, IOPAR_SRCATTR , A_DIRECT|A_NORMAL}
};

char   *prgname = "RENAME";	// Our programe name
char   *envname = "^XOS^RENAME^OPT"; // The environment option name
char   *example = "{{/}{-}option{=sub}} {...\\}source destination";
char   *description = "This program will rename one or more files.";

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

uchar  totalfiles = FALSE;

extern int _malloc_amount;
int    maxmem;

Prog_Info pib;

// Function prototypes

int   askuser(char *msg, int quit);
int   comp(FD *one, FD *two);
int   dorename(char *src, char *dst);
void  errormsg(const char *arg, long  code);
void  fileerror(int nl, long errcode, char *text, char *name, int last);
char *findname(char *bgn);
void  finish(void);
void *getmem(size_t size);
int   nonopt(char *arg);
int   opterror(arg_data *);
void  opthelp(void);
int   optnodir(void);
int   optnototals(arg_data *);
int   opttotals(arg_data *);
void  procdir(void);
int   procfile(void);
void  renamefile(char *srcstr, char *dststr, int last);
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
{   {"F*ILES"  , "Display total number of files renamed"},
    {"NOF*ILES", "Don't display total number of files renamed"},
    {NULL}
};

SubOpts errorcmd[] =
{   {"Q*UIT"    , "Quit renaming after error"},
    {"A*SK"     , "Ask user for instructions after error"},
    {"C*ONTINUE", "Continue renaming after error"},
    {NULL}
};

arg_spec options[] =
{
//  {"BE*FORE"  , ASF_VALREQ|ASF_LSVAL, NULL     ,    optbefore   , 0    ,
//	    "Transfer all matching files before this Date/Time"},
//  {"SI*NCE"   , ASF_VALREQ|ASF_LSVAL, NULL     ,    optsince    , 0    ,
//	    "Transfer all matching files since this Date/Time"},
    {"S*ELECT"  , ASF_BOOL|ASF_STORE  , NULL     ,   &select      , TRUE ,
	    "Select which files to rename"},
    {"O*VER"    , ASF_BOOL|ASF_STORE  , NULL     ,   &overwrite   , TRUE ,
	    "Replace existing files without confirmation"},
    {"ERR*OR"   , ASF_XSVAL           , errorcmd ,   opterror     , 0    ,
	    "Specify action to take on error"},
    {"DOS*DRIVE", ASF_BOOL|ASF_STORE  , NULL     ,   &gcp_dosdrive, TRUE ,
	    "Override the global DOSDRIVE setting"},
    {"Q*UIET"   , ASF_BOOL|ASF_STORE  , NULL     ,   &quiet       , TRUE ,
	    "Don't display any status"},
    {"V*ERBOSE" , ASF_BOOL|ASF_STORE  , NULL     ,   &verbose     , TRUE ,
	    "Display full status"},
    {"SO*RT"    , ASF_BOOL|ASF_STORE  , NULL     ,   &dosort      , TRUE ,
	    "Sort file list before renaming"},
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
    char *dstext;
    int   rtn;
    char  chr;
    char  srcrtn[512];
    char  dstrtn[512];
    char  strbuf[512];		// String buffer

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

    srcbufr = srcrtn;
    renparms.filspec.buffer = dstrtn;

    // Check Global Configuration Parameters

    global_parameter(TRUE);

    // Check Local Configuration Parameters

    if(svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
    {
	envpnt[0] = strbuf;
	envpnt[1] = NULL;
        progarg(envpnt, PAF_EATQUOTE, options, NULL, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
////    isterm = fisctrm(stdout);

    // Process the command line

    if (argc >= 2)
    {
	++argv;
        progarg(argv, PAF_EATQUOTE, options, NULL, nonopt,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    if (dstspec == NULL)		// Destination specified?
	dstspec = &dfltspec;		// No - use default

    // Determine if we have any wild-card characters in any source specs.
    //   (This is very simple check.  If there are wild-card characters in
    //   the device name or directory fields, we will see them here.  In
    //   this case, we just let dirscan complain about the illegal wild-card
    //   usage!)

    // Destination wild cards are allowed only to indicate complete fields.
    //   That is, they can specify the name or extension.  When a destination
    //   wild card is specified, no other character can appear in the name or
    //   extension field.

    if ((thisspec = srcspec) == NULL)
    {
	fputs("? RENAME: No files specified, type RENAME /H for help\n",
		stderr);
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

    pnt = dstspec->name;
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

	    if (chr == ':' || chr == '\\' || chr == '/')
	    {
		fputs ("? RENAME: Cannot specify device or path for "
			"destination.  Use\n          the MOVE command to "
			"change directories or devices.\n", stderr);
		exit(1);
	    }
	    if (chr == '.')
		dstext = pnt;
	}
	if (dstext == NULL)
	    dstext = pnt;

	if (dstspec->name[0] == '*')	// Wild card destination name?
	{
	    if ((dstext - dstspec->name) != 2 && dstspec->name[1] != 0)
	    {				// Yes - is it valid?
		fputs("? RENAME: Illegal wild-card usage in destination name\n",
			stderr);
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
		fputs("? RENAME: Illegal wild-card usage in destination "
			"extension\n", stderr);
		exit (1);
	    }
	    dstextwild = TRUE;
	}
	else
	    dstextwild = FALSE;
    }
    renparms.filoptn.value = (gcp_dosdrive) ?
	    FO_DOSNAME|FO_NODENUM|FO_NODENAME|FO_RVOLNAME|FO_PATHNAME|
		    FO_FILENAME :
	    FO_XOSNAME|FO_NODENUM|FO_NODENAME|FO_RVOLNAME|FO_PATHNAME|
		    FO_FILENAME;
    if (srcwild || srcellipbgn != NULL)	// Did we find a source wild-card?
    {					// Yes
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
				fputs("? RENAME: Invalid ellipsis in "
					"destination specification", stderr);
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
	    fileerror(FALSE, ER_FILNF, "Error renaming file", srcspec->name,
		    TRUE);
	}
	else
	{
	    do
	    {
		thisfile = firstdir;
		while (thisfile != NULL)
		{
		    renamefile(thisfile->name, dstspec->name,
			    (thisfile->nxtfile == NULL &&
			    thisfile->nxtdir == NULL));
		    thisfile = thisfile->nxtfile;
		}
	    } while ((firstdir = firstdir->nxtdir) != NULL);
	}
    }
    else				// No wild cards or ellipsis - just
					//   rename the single file
	renamefile(srcspec->name, dstspec->name, TRUE);

    // Display final summary if need to

    finish();
}

//**************************************************
// Function: finish - Display final summary and exit
// Returend: Never returns
//**************************************************

static void finish(void)

{
    if (totalfiles)
	printf("\t%d file%s renamed\n", filecnt, (filecnt != 1) ? "s" : "");
    exit(0);				// Return with no error
}

//********************************************
// Function: renamefile - Rename a single file
// Returned: Nothing
//********************************************

// All source wild cards have been resolved before this function is called.
//   Destination wild cards are resolved here.

static void renamefile(
    char *srcstr,		// Source string
    char *dststr,		// Destination string
    int   last)			// TRUE if this is the last file to rename

{
    char *srcname;
    char *srcext;
    char *dstname;
    char *dstext;
    char *pnt;
    long  rtn;
    char  chr;
    char  dst[512];

    if (select && (srcwild || srcellipbgn != NULL))
    {
	sprintf(dst, "Rename file %s", srcstr);
	switch(askuser(dst, TRUE))
	{
	 case 0:			// No
	    return;

	 case 2:			// All
	    select = FALSE;
	 case 1:			// Yes
	    break;
	}
    }
    pnt = dstname = findname(dststr);	// Find start of destination name
    srcname = findname(srcstr);		// Find start of source name
    dstname = strnmov(dst, srcstr, srcname - srcstr);
					// Copy source device and path
    if (dstnamewild | dstextwild)	// Have a destination wild card?
    {
	pnt = srcname;
	srcext = NULL;
	while ((chr = *pnt) != 0)	// Find start of source extension
	{
	    pnt++;
	    if (chr == '.')
		srcext = pnt;
	}
	if (srcext == NULL)		// Null extension?
	    srcext = pnt;		// Yes - point to end of string
	pnt = dststr;			// Find start of destination extension
	dstext = NULL;
	while ((chr = *pnt) != 0)
	{
	    pnt++;
	    if (chr == '.')
		dstext = pnt;
	}
	if (dstext == NULL)		// Null extension?
	    dstext = pnt;		// Yes - point to end of string
	pnt = (dstnamewild) ? strnmov(dstname, srcname, srcext - srcname) :
		strnmov(dstname, dststr, dstext - dststr);
	strmov(pnt, (dstextwild) ? srcext : dstext);
    }
    else
	strmov(dstname, dststr);
    if ((rtn = dorename(srcstr, dst)) < 0)
    {
	if (rtn == ER_FILEX)
	{
	    if (!overwrite)
	    {
		sprintf(srcbufr, "Replace %s",
			(char *)(renparms.filspec.buffer));
		switch (askuser(srcbufr, TRUE))
		{
		 case 0:		// No
		    return;

		 case 2:		// All
		    overwrite = TRUE;
		 case 1:		// Yes
		    break;
		}
		if ((rtn = svcIoDelete(0, dststr, NULL)) < 0)
		{
		    fileerror(FALSE, rtn, "Error deleting existing file",
			    dststr, last);
		    return;
		}
		rtn = dorename(srcstr, dst);
	    }
	}
	if (rtn < 0)
	{
	    fileerror(FALSE, rtn, "Error renaming file", srcstr, last);
	    return;
	}
    }
    showstatus();
    return;
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

    if ((rtn = svcIoRename(0, src, dst, &renparms)) >= 0)
    {
	filecnt++;
	if (!quiet)
	{
	    pntdev = NULL;
	    pnts = srcbufr;
	    pntd = (char * near)(renparms.filspec.buffer);
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
		pnts = (char * near)(renparms.filspec.buffer);
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
	    printf("%s => %s\n", srcbufr,
		    (char * near)(renparms.filspec.buffer));
	else
	    printf("%s\n", srcbufr);
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

    if (dsd.filenamelen == 0)		// Ignore empty directories
	return (TRUE);

    // We build a list of files to rename in each directory.

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
    pnt = strmov(thisfile->name, dsd.devname);
    rmt = &dsd.rmtdata;
    while (rmt != NULL)
    {
	pnt = strmov(strmov(pnt, rmt->rmtdevname), rmt->nodename);
	rmt = rmt->next;
    }
    strmov(strmov(pnt, dsd.pathname), dsd.filename);
    if (dsd.error < 0)			// Error reported by DIRSCAN?
	fileerror(FALSE, dsd.error, "Error searching directory for",
		thisfile->name, FALSE);
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
    thisfile->nxtfile = thisfile->nxtdir = NULL;
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
    if (dosort && firstfile->nxtfile != NULL) // Should we sort the list?
	firstfile = heapsort(firstfile,	// Yes
		(int (*)(void *a, void *b, void *d))comp, NULL);
    thisfile = firstfile;
    while (thisfile != NULL)
    {
	thisfile->nxtdir = NULL;
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
    if (rtn == 3)
	finish();
    return (rtn);
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

    totalfiles = FALSE;
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
    spec = getmem(size + sizeof(FS));
    strmov(spec->name, arg);
    if (srcspec == NULL)
	srcspec = spec;
    else if (dstspec == NULL)
	dstspec = spec;
    else
    {
	fputs("? RENAME: Too many files specified\n", stderr);
	exit(EXIT_INVSWT);
    }
    spec->next = NULL;
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
    int   last)			// TRUE if this is the last file to rename

{
    int  action;
    char msg[100];

    // First display the error message

    svcSysErrMsg(errcode, 0x03, msg);
    fprintf(stderr, "%s? RENAME: %s %s\n          %s\n", (nl) ? "\n" : "",
	    text, name, msg);

    // Determine if we should continue

    if (erraction != 0)
    {
	action = 0;
	if ((srcwild || srcellipbgn != NULL) && !last)
	    action = (erraction < 0) ? askuser("% RENAME: Continue renaming",
		    FALSE) : erraction;
	if (action == 2)
	    erraction = 1;
	if (action != 0)
	    return;
    }
    finish();
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
        fputs("? RENAME: Not enough memory available\n", stderr);
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
	fprintf(stderr, "\n? RENAME: %s%s%s\n", buffer, (arg[0] != '\0') ?
		"; ": "", arg);		// Output error message
    }
    else
	fprintf(stderr, "\n? RENAME: %s\n", arg);
    exit(1);
}
