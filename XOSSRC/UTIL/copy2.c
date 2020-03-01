//--------------------------------------------------------------------------*
// COPY2.C
// File copy utility for XOS
//
// Edit History:
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

#include <CTYPE.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <TIME.H>
#include <XOSERR.H>
#include <XOSTRM.H>
#include <XOSRTN.H>
#include <XOS.H>
#include <XOSERMSG.H>
#include <XOSSVC.H>
#include <XOSTIME.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <DIRSCAN.H>
#include <GLOBAL.H>
#include <XOSSTR.H>


#define MAXSWITNAME 8		/* Maximum option name size */
#define ENVSWT      20		/* Maximum number of environment options */
#define TIMBFR      40		/* Size of the time string buffer */
#define HELP_COL    39		/* Width of the help column */

#define WILDCARD "*"
#define COLWIDTH 16

#define VERSION 3
#define EDITNO  6

// Structure for file specification blocks

typedef struct filespec FILESPEC;
struct  filespec
{   FILESPEC *next;
    char      name[1];
};

FILESPEC *srcspec;
FILESPEC *thisspec;
FILESPEC *dstspec;

// File description data structure

typedef struct filedescp FILEDESCP;
struct  filedescp
{   FILEDESCP *next;		// Address of next file block
    FILEDESCP *sort;		// Used by heapsort
    char       name[1];		// File name
};

FILEDESCP *firstfile;
FILEDESCP *lastfile;
FILEDESCP *thisfile;

uchar doconcat;			// TRUE if doing concatanation
uchar concatnxt;		// TRUE if saw + last in command line
uchar srcwild;			// TRUE if have source wild-card character

char *srcellipbgn;
char *dstellipbgn;


char  owner[36];

char *ellipstr;

struct
{   byte4_parm  filoptn;
    lngstr_parm filspec;
    byte2_parm  srcattr;
    char        end;
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
    char        end;
} fileparm =
{   {PAR_SET|REP_HEXV, 4 , IOPAR_FILOPTN, FO_VOLNAME|FO_NODENUM|FO_NODENAME|
            FO_RVOLNAME|FO_PATHNAME|FO_FILENAME|FO_ATTR},
    {PAR_GET|REP_STR , 0 , IOPAR_FILSPEC, NULL, 0, 0, 0},
    {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR, A_NORMAL},
    {PAR_GET|REP_HEXV, 2 , IOPAR_FILATTR, 0},
    {PAR_SET|REP_HEXV, 4 , IOPAR_DIRHNDL, 0}
};


int deloutfile = FALSE;
int image = FALSE;
int confirm = FALSE;

int   copysize = 16*1024;
uchar nosort = FALSE;
uchar quiet = FALSE;
uchar verbose = FALSE;
uchar verify = FALSE;
uchar totalbytes = FALSE;
uchar totalfiles = FALSE;
uchar rate = FALSE;
char  startfrom[300];	// ??? LENGTH OF THIS ???

char   errdone;			// TRUE if have output an error message
int     numfiles;

extern int _malloc_amount;
int     maxmem;

Prog_Info pib;
char    copymsg[] = "";
char    prgname[] = "COPY2";	// Our programe name
char    envname[] = "^XOS^COPY2^OPT"; // The environment option name
char    example[] = "{{/}{-}option{=sub}} {...\\}source1{+sourcen} "
	"{...\\}destination";
char    description[] = "This program will copy one or more files from most "
	"standard devices.";

struct
{   byte4_parm modev;
    byte4_parm modec;
    byte4_parm modes;
    char       end;
} snglparm =
{   {PAR_GET|REP_HEXV, 4, IOPAR_TRMSINPMODE, 0},
    {PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, TIM_ECHO},
    {PAR_SET|REP_HEXV, 4, IOPAR_TRMSINPMODE, TIM_IMAGE}
};

struct
{   byte4_parm modec;
    byte4_parm modes;
    char       end;
} normparm =
{   {PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0xFFFFFFFF},
    {PAR_SET|REP_HEXV, 4, IOPAR_TRMSINPMODE, 0}
};

// Function prototypes

int   comp(FILEDESCP *one, FILEDESCP *two);
void  copyallfiles(void);
void  copyfile(FILESPEC *src, char *dstname);
void *getmem(size_t size);
void  helpprint(char *help_string, int state, int newline);
int   nonopt(char *arg);
int   procfile(void);
void  errormsg(const char *arg, long  code);
int  optafter(arg_data *);
int  optbinary(arg_data *);
int  optbefore(arg_data *);
int  optbuffer(arg_data *);
void opthelp(void);
int  optnodir(void);
int  optquiet(arg_data *);
int  optsort(arg_data *);
int  optstart(arg_data *);
int  opttotals(arg_data *);
int  optverbose(arg_data *);
int  optverify(arg_data *);

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

SubOpts totalscmd[] =
{
    {"B*YTES"  , "Total number of bytes copied"},
    {"F*ILES"  , "Total number of files copied"},
    {"R*ATE"   , "Total transfer rate for copy"},
    {"NOB*YTES", "Don't show total number of bytes copied"},
    {"NOF*ILES", "Don't show total number of files copied"},
    {"NOR*ATE" , "Don't show total transfer rate for copy"},
    {NULL      , NULL}
};

arg_spec options[] =
{   {"?"        , 0                   , NULL     , AF(opthelp)    , 0    ,
	    "This message"},
    {"A*SCII"   , 0                   , NULL     ,    optbinary   , FALSE,
	    "Text copy mode"},
//  {"BE*FORE"  , ASF_VALREQ|ASF_LSVAL, NULL     ,    optbefore   , 0    ,
//	    "Copy all matching files before this Date/Time"},
    {"B*INARY"  , 0                   , NULL     ,    optbinary   , TRUE ,
	"Binary copy mode"},
    {"BU*FFER"  , ASF_VALREQ|ASF_NVAL , NULL     ,    optbuffer   , 0    ,
	    "Change input buffer size"},
    {"C*ONFIRM" , ASF_BOOL|ASF_STORE  , NULL     ,   &confirm     , TRUE ,
	    "Choose which files to copy"},
    {"D*ELETE"  , ASF_BOOL|ASF_STORE  , NULL     ,   &deloutfile  , TRUE ,
	    "Delete incomplete output files"},
    {"DOSD*RIVE", ASF_BOOL|ASF_STORE  , NULL     ,   &gcp_dosdrive, TRUE ,
	    "Overrides the global DOSDRIVE setting"},
    {"H*ELP"    , 0                   , NULL     , AF(opthelp)    , 0    ,
	    "This message"},
    {"I*MAGE"   , ASF_BOOL|ASF_STORE  , NULL     ,   &image       , TRUE ,
	    "Disk image I/O handling enabled"},
    {"Q*UIET"   , ASF_BOOL            , NULL     ,    optquiet    , TRUE ,
	    "Don't display any copy status"},
//  {"SI*NCE"   , ASF_VALREQ|ASF_LSVAL, NULL     ,    optsince    , 0    ,
//	    "Copy all matching files since this Date/Time"},
    {"S*ORT"    , ASF_BOOL            , NULL     ,    optsort     , TRUE ,
	    "Sort files before copying"},
    {"ST*ART"   , ASF_VALREQ|ASF_LSVAL, NULL     ,    optstart    , 0    ,
	    "Start copy with the specified file"},
    {"T*OTALS"  , ASF_XSVAL           , totalscmd,    opttotals   , 0    ,
	    "Display summary of copy"},
    {"VERB*OSE" , ASF_BOOL            , NULL     ,    optverbose  , TRUE ,
	    "Display full file names and status"},
    {"V*ERIFY"  , ASF_BOOL            , NULL     ,    optverify   , TRUE ,
	    "Verify error free transfer"},
    {NULL       , 0                   , NULL     , AF(NULL)       , 0    ,
	    NULL}
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
    int   rtn;
    char  chr;
    char  strbuf[512];		// String buffer

    // Set defaults

    reg_pib(&pib);

    // Set Program Information Block variables

    pib.opttbl = options; 		// Load the option table
    pib.kwdtbl = NULL;
    pib.build = __DATE__;
    pib.majedt = VERSION; 		// Major edit number
    pib.minedt = EDITNO; 		// Minor edit number
    pib.copymsg = copymsg;
    pib.prgname = prgname;
    pib.desc = description;
    pib.example = example;
    pib.errno = 0;
    getTrmParms();
    getHelpClr();

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

    // Process the command line

    if (argc >= 2)
    {
	++argv;
        progarg(argv, 0, options, NULL, nonopt,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

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
	exit(0);
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

    if (srcwild || srcellipbgn != NULL)	// Did we find a wild-card?
    {					// Yes
	if (doconcat)			// Doing concatanation?
	{
	    fputs("? COPY2: Cannot use wild-cards with concatanation\n",
		    stderr);
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
				fputs("? COPY2: Invalid ellipsis in "
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

	    printf("### ELIP: %s\n", ellipstr);

	    *srcellipbgn = chr;
	}
	dirscan(srcspec->name, &dsd);	// Scan the directory
	if (numfiles != 0)		// Have any files in the last directory?
	    copyallfiles();		// Yes - copy them
    }
    else
	copyfile(srcspec, dstspec->name); // No wild cards or ellipsis - just
					  //   copy the file
    // Display final summary if need to




    exit(EXIT_NORM);			// Return with no error
}

//**************************************************************
// Function: procfile - Function called by dirscan for each file
// Returned: TRUE if should continue, FALSE if should terminate
//**************************************************************

int procfile(void)

{
    char    *pnt;
    RMTDATA *rmt;
    int      len;

    // We build a list of files to copy in each directory.  When we get a
    //   different directry, we sort (if necessary) the list and copy the
    //   files.  Since we have to open each file to copy it, all we need
    //   in our list here is the file specification.

    if (dsd.changed && (numfiles != 0))
	copyallfiles();

    if (dsd.error < 0 && ((dsd.error != ER_FILAD && dsd.error != ER_FBFER &&
	    dsd.error != ER_FBFER && dsd.error != ER_FBWER &&
	    dsd.error != ER_BUSY)))
    {
	// Error - display error message!!!

	return (FALSE);
    }
    if ((dsd.filenamelen) == 0)
        return (TRUE);			// Forget it if no name

    // Get total length of the file specification

    len = dsd.devnamelen + dsd.pathnamelen + dsd.filenamelen;
    rmt = &dsd.rmtdata;
    while (rmt != NULL)
    {
	len += (rmt->rmtdevnamelen + rmt->nodenamelen);
	rmt = rmt->next;
    }

    thisfile = getmem(sizeof(FILEDESCP) + len);

    pnt = strmov(thisfile->name, dsd.devname);
    rmt = &dsd.rmtdata;
    while (rmt != NULL)
    {
	pnt = strmov(strmov(pnt, rmt->rmtdevname), rmt->nodename);
	rmt = rmt->next;
    }
    strmov(strmov(pnt, dsd.pathname), dsd.filename);

    printf("### SPEC: %s\n", thisfile->name);

    numfiles++;
    if (firstfile == NULL)
        firstfile = thisfile;
    else
        lastfile->next = thisfile;
    lastfile = thisfile;
    thisfile->next = NULL;
    return (TRUE);
}

//************************************************
// Function: comp - Compare two filenames for sort
// Returned: Negative if a < b
//	     Zero if a == b
//           Positive if a > b
//************************************************

int comp(
    FILEDESCP *one,
    FILEDESCP *two)

{
    return (stricmp(one->name, two->name)); // Always sort on name
}

//*******************************************************
// Function: copyallfiles - Copy all files in a directory
// Returned: Nothing
//*******************************************************

void copyallfiles(void)

{
    char *pnt;
    char  dst[512];

    printf("### copyallfiles\n");

    if (!nosort)			// Should we sort the list?
	firstfile = heapsort(firstfile,	// Yes
		(int (*)(void *a, void *b, void *d))comp, NULL);

    while (firstfile != NULL)
    {
	thisfile = firstfile;
	thisspec = (FILESPEC *)&(thisfile->sort);
	firstfile = firstfile->next;
	thisspec->next = NULL;


	if (strncmp(thisspec->name, ellipstr, ellipparm.filspec.strlen) != 0)
	{
	    printf("??? Ellipsis string does not match!!!\n");
	    exit(1);
	}

	pnt = strnmov(dst, dstspec->name, dstellipbgn - dstspec->name);
	pnt = strmov(pnt, thisspec->name + ellipparm.filspec.strlen);
	pnt = strmov(pnt, dstellipbgn + 4);

	copyfile(thisspec, dst);
	free(thisfile);
    }
}

//****************************************
// Function: copyfile - Copy a single file
// Returned: Nothing
//****************************************

void copyfile(
    FILESPEC *spec,
    char     *dstname)


{
    printf("##### %s --> %s\n", spec->name, dstname);
}

//******************************************
// Function: optafter - Process AFTER option
// Returned: TRUE
//******************************************

int optafter(
    arg_data *arg)

{
    char *time_ptr;

    time_ptr = arg->val.s;
    printf("after time = %s\n", time_ptr);
    return (TRUE);
}

//********************************************
// Function: optbefore - Process BEFORE option
// Returned: TRUE
//********************************************

int optbefore(
    arg_data *arg)

{
    char *time_ptr;

    time_ptr = arg->val.s;
    printf("before time = %s\n", time_ptr);
    return (TRUE);
}

//****************************************
// Function: optsort - Process SORT option
// Returned: TRUE
//****************************************

int optsort(
    arg_data *arg)

{
    nosort = arg->data;
    return (TRUE);
}

//*****************************************
// Function: optquiet - Proces QUIET option
// Returned: TRUE
//*****************************************

int optquiet(
    arg_data *arg)

{
    quiet = arg->data;
    verbose = !quiet;
    return (TRUE);
}

//*********************************************
// Function: optverbose - Proces VERBOSE option
// Returned: TRUE
//*********************************************

int optverbose(
    arg_data *subopt)

{
    verbose = subopt->data;
    quiet = !verbose;
    return (TRUE);
}

//*******************************************
// Function: optverify - Proces VERIFY option
// Returned: TRUE
//*******************************************

int optverify(
    arg_data *subopt)

{
    verify = subopt->data;
    return (TRUE);
}

//********************************************
// Function: optbefore - Process BEFORE option
// Returned: TRUE
//********************************************

#if     0                               // FIXME!

int optbefore(
    arg_data *arg)

{
    if (dt_parse(arg->val.s, &before_time))
    	printf("Couldn't parse >%s<\n", arg->val.s);
    return (TRUE);
}

#endif

//*******************************************
// Function: optbinary - Proces BINARY option
// Returned: TRUE
//*******************************************

int optbinary(
    arg_data *arg)

{
    arg = arg;

/*
    if (!destination_seen)    
    {
	ascii_write = ascii_read = !(subopt->data);	//  Switch before source file
	binary_write = binary_read = (int)subopt->data;
    }
    else
    {
	ascii_write = !(subopt->data);	//  Switch after source file
	binary_write = (int)subopt->data;
    }
*/

    return (TRUE);
}

//*************************************************
// Function: optbuffer - Process BUFFER size option
// Returned: TRUE
//*************************************************

int optbuffer(
    arg_data *subopt)

{
    copysize = ((subopt->val.n + 511) / 512) * 512;
    if (copysize < 512)
    {
        fprintf(stderr, "\n? %s: Buffer size (%d) is too small\n",
		pib.prgname, copysize);
        exit(EXIT_MALLOC);
    }
    return (TRUE);
}

//*****************************************
// Function: optstart - Proces START option
// Returned: TRUE
//*****************************************

int optstart(
    arg_data *subopt)

{
    strcpy(startfrom, subopt->val.s);
    return (TRUE);
}

//*******************************************
// Function: opttotals - Proces TOTALS option
// Returned: TRUE
//*******************************************

int opttotals(
     arg_data *subopt)

{
    if ((subopt->flags & ADF_XSVAL) == 0)
	return (TRUE);

    switch ((int)subopt->val.n)
    {
     case 0:				//  BYTES
	totalbytes = TRUE;
	break;

     case 1:				//  FILES
	totalfiles = TRUE;
	break;

     case 2:				//  RATE
	rate = TRUE;
	break;

     case 3:				//  NOBYTES
	totalbytes = FALSE;
	break;

     case 4:				// NOFILES
	totalfiles = FALSE;
	break;

     case 5:				// NORATE
	rate = FALSE;
	break;

     default:
	fprintf(stderr, "? %s: Invalid TOTALS option value, %d\n",
		pib.prgname, subopt->val.n);
	exit(EXIT_INVSWT);
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
    FILESPEC *spec;
    int       size;

    size = strlen(arg);			// Get length of argument
    if (size == 1 && arg[0] == '+')	// Is it +
    {					// Yes - this means concatanation
	if (concatnxt || srcspec == NULL || dstspec != NULL)
	{
	    fputs("? COPY2: Illegal syntax for concatanation\n", stderr);
	    exit(EXIT_INVSWT);
	}
	doconcat = TRUE;
	concatnxt = TRUE;
	return (TRUE);
    }
    spec = getmem(size + sizeof(FILESPEC));
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
		fputs("? COPY2: Too many files specified\n", stderr);
		exit(EXIT_INVSWT);
	    }
	    dstspec = spec;
	}
    }
    spec->next = NULL;
    thisspec = spec;
    return (TRUE);
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
        fputs("? COPY2: Not enough memory available\n", stderr);
        exit(EXIT_MALLOC);
    }
    if (maxmem < _malloc_amount)
        maxmem = _malloc_amount;
    return (ptr);
}

//***********************************************
// Function: errormsg - Display XOS error message
// Returned: Nothing
//***********************************************

void errormsg(
    const char *arg,
    long  code)

{
    char buffer[80];            // Buffer to receive error message

    if (numfiles != 0)
        copyallfiles();
    if (code != 0)
    {
        svcSysErrMsg(code, 3, buffer);	// Get error message
        fprintf(stderr, "\n? COPY2: %s%s%s\n", buffer, (arg[0] != '\0')? "; ": "",
                arg);			// Output error message
    }
    else
        fprintf(stderr, "\n? COPY2: %s\n", arg);
    errdone = TRUE;
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
