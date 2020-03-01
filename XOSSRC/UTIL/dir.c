//--------------------------------------------------------------------------*
// DIR.C
// Directory display utility for XOS
//
// Written by: Bruce R. Nevins
//
// Edit History:
// 08/11/88(brn) - Created first version
// 04/13/89(brn) - Fix to display version numbers if used
// 04/20/89(brn) - Add support for long file names
// 05/02/89(brn) - Add ... construct for recursive directories
// 05/25/89(brn) - Add code to get size of screen we are on
// 09/08/89(brn) - Change options and environment string name
// 10/05/89(brn) - Make file names with no extension or "." default to ".*"
//               - Fix case sensitivity in option names and values
//               - Fix options to complain when they have a bad value
// 11/12/89(brn) - Remove support for / in path names
// 	 - Add support for dos device names
// 11/16/89(brn) - Make / or - be the option characters
// 12/10/89(brn) - Fix bug in getlabel returning a . in the name
// 12/30/89(brn) - Add support for procarg
// 01/27/90(brn) - Fix minor bugs and add WIDE switch for DOS compatibility
// 02/09/90(brn) - Make page default to false and listing default to wide
// 04/19/90(brn) - V1.11 Fixed long filename wrapping for VMS files
// 12/13/90(jrg) - Converted to run under XOS v1.6
// 02/27/91(brn) - Fix spelling of ASCENDING
// 03/28/91(brn) - Add support for dynamic configuration of row and column
// 		size.
// 03/28/91(brn) - Add support for global_parameter checks
// 03/29/91(brn) - Add dosquirk support with dos compatible output
// 04/02/91(brn) - Fix bug in doing multiple files and directorys from
// 		command line.
// 05/18/91(brn) - Add code to handle new ioinsinglep call
// 06/17/91(brn) - Fix header print even if no files we found.  Fix bug with
// 		two file not found messages.
// 10/06/91(brn) - Fix print_header to search for the device name from
// 		the right.
// 	   Fix getlabel to not print any errors and to save and
// 		restore fileparm.options.value
// 	   Fix file not found handling
// 12/23/91(brn) - Add before and after switch to handle including and
// 		excluding of dates to be displayed
// 01/07/92(brn) - Fix to get dates if sorting by date with filename only
// 		display
// 1.21 24-Feb-92 (JRG) Major changes to clean up recursive and multiple
// 		directory listings
// 1.22 10-Mar-92 (JRG) Added ER_BUSY to non-fatal errors
// 1.23 23-Apr-92 (JRG) Major changes to use dirscan library routine
// 08/20/92(brn) - Change reference to global.h from local to library
// 05/12/94(brn) - Change version number to reflect 32 bit version
// 03/15/95(sao) - Add progasst package
// 04/14/94(sao) - Changed input type on ALL option (fixes help display)
// 05/13/95(sao) - Changed 'optional' indicator from [] to {}
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
//  6Jul95 (fpj) - Changed time_t to time_s, to correct bug in handling of
//                 volume ID for disks.
// 3.6  ????????? (JRG) Added several options to support short/long filenames
// 3.7  10-Mar-99 (JRG) Changed to merge multiple file list in same directory,
//                added /X option
// 3.8  26-Mar-99 (JRG) Changed date display to 4 digits
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
#include <XOSSVC.H>
#include <XOSTIME.H>
#include <XOSWILDCARD.H>
#include <XOSERRMSG.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <DIRSCAN.H>
#include <GLOBAL.H>
#include <XOSSTR.H>

#define WILDCARD "*"
#define COLWIDTH 16

#define VERSION 3
#define EDITNO  8

typedef struct filedescp FILEDESCP;

// Structure for file specification blocks

typedef struct fs FS;
struct  fs
{   FS   *next;					// Pointer to next independent file spec
    FS   *link;					// Pointer to next file spec for same
								//   directory
    long  glbid[4];				// Global device ID for path
    long  devsts;				// Device status bits
    char *ellip;				// Offset of start of ellipsis
    char *name;					// Offset of start of name part
    char  spec[4];				// File specification
};

FS *firstspec;
FS *thisspec;

typedef struct xname XNAME;
struct  xname
{   XNAME *next;
    char   name[4];
};

XNAME *firstname;

// File description data structure

struct filedescp
{   FILEDESCP *next;			// Address of next file block
    FILEDESCP *sort;			// Used by heapsort
    uchar      filattr;			// File attributes
    uchar      xxx;				// Reserved
    long       error;			// Error code
    char       data[2];			// Start of data area
};

// Items are stored in the data area as needed in the following order:
//	amount allocated		4 bytes
//	protection				4 bytes
//	owner name				36 bytes
//	access date and time	8 bytes
//	modify date and time	8 bytes
//	creation date and time	8 bytes
//	file length				4 bytes
//	DOS file name			16 bytes
//	long file name			variable

#define ALLOC(pnt) ((long *)(((FILEDESCP *)pnt)->data+needalloc))
#define PROT(pnt) ((long *)(((FILEDESCP *)pnt)->data+needprot))
#define OWNER(pnt) (((FILEDESCP *)pnt)->data+needowner)
#define ADATE(pnt) ((time_s *)(((FILEDESCP *)pnt)->data+needadate))
#define MDATE(pnt) ((time_s *)(((FILEDESCP *)pnt)->data+needmdate))
#define CDATE(pnt) ((time_s *)(((FILEDESCP *)pnt)->data+needcdate))
#define LENGTH(pnt) ((long *)(((FILEDESCP *)pnt)->data+needlength))
#define NAMEDOS(pnt) ((char *)(((FILEDESCP *)pnt)->data+neednamedos))
#define NAME(pnt) (((FILEDESCP *)pnt)->data+needname)

FILEDESCP *firstfile;
FILEDESCP *lastfile;
FILEDESCP *thisfile;

// Function prototypes

int   comp(FILEDESCP *one, FILEDESCP *two);
void *getmem(size_t size);
int   nonopt(char *arg);
void  pagecheck(void);
void  printfiles(void);
void  printheading(void);
int   procfile(void);
char *wideentry(FILEDESCP *file, char *linebufr, char *linepnt);
void  errormsg(const char *arg, long  code);

// Switch settings functions

int  optafter(arg_data *);
int  optall(void);
int  optbefore(arg_data *);
int  optdebug(arg_data *);
int  optdir(void);
int  optdisplay(arg_data *);
int  optfull(void);
void opthelp(void);
int  optlong(void);
int  optnames(void);
int  optnodir(void);
int  optnonames(void);
int  optone(void);
int  optpause(arg_data *);
int  optshort(void);
int  optsort(arg_data *);
int  optx(arg_data *);
int  allhave( arg_data *);

// Stuff needed by dirscan

char owner[36];

struct
{   byte4_parm  filoptn;
    lngstr_parm filspec;
    byte2_parm  srcattr;
    byte2_parm  filattr;
    byte4_parm  dirhndl;
    byte4_parm  devsts;
    byte4_parm  length;
    byte4_parm  alloc;
    byte8_parm  cdate;
    byte8_parm  mdate;
    byte8_parm  adate;
    byte4_parm  prot;
    lngstr_parm owner;
    char        end;
} fileparm =
{   {PAR_SET|REP_HEXV, 4 , IOPAR_FILOPTN , FO_VOLNAME|FO_NODENUM|FO_NODENAME|
            FO_RVOLNAME|FO_PATHNAME|FO_ATTR},
    {PAR_GET|REP_STR , 0 , IOPAR_FILSPEC , NULL, 0, 0, 0},
    {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR , A_NORMAL|A_DIRECT},
    {PAR_GET|REP_HEXV, 2 , IOPAR_FILATTR , 0},
    {PAR_SET|REP_HEXV, 4 , IOPAR_DIRHNDL , 0},
    {PAR_GET|REP_HEXV, 4 , IOPAR_DEVSTS  , 0},
    {PAR_GET|REP_DECV, 4 , IOPAR_LENGTH  , 0},
    {PAR_GET|REP_DECV, 4 , IOPAR_REQALLOC, 0},
    {PAR_GET|REP_HEXV, 8 , IOPAR_CDATE   , 0},
    {PAR_GET|REP_HEXV, 8 , IOPAR_MDATE   , 0},
    {PAR_GET|REP_HEXV, 8 , IOPAR_ADATE   , 0},
    {PAR_GET|REP_HEXV, 4 , IOPAR_PROT    , 0},
    {PAR_GET|REP_STR , 0 , IOPAR_OWNER   , owner, 0, 36, 36}
};

DIRSCANDATA dsd =
{   (DIRSCANPL *)&fileparm,
						// parmlist    - Address of parameter list
    FALSE,				// repeat      - TRUE if should do repeated search
    TRUE,				// dfltwildext - TRUE if want wildcard as default
    QFNC_DEVPARM,		// function    - Value for low byte of QAB_FUNC
    DSSORT_ASCEN,		// sort        - Directory sort order
    NULL,               // hvellip     - Function called when ellipsis specified
    (int (*)(DIRSCANDATA *))procfile,
						// func        - Function called for each file matched
    errormsg			// error       - Function called on error
};

#define OPT(name) ((int (*)(arg_data *))name)

SubOpts discmd[] =
{   {"T*OTALS"      , "Show statistical totals"},
    {"NOT*OTALS"    , "Do not show totals"},
    {"P*ATH"        , "Show path" },
    {"NOP*ATH"      , "Do not show path"},
    {"V*OLUME"      , "Show device volume"},
    {"NOV*OLUME"    , "Do not show device volume"},
    {"AT*TRIBUTES"  , "Show file attributes"},
    {"NOAT*TRIBUTES", "Do not show file attributes"},
    {"DOS*NAME"     , "Show DOS (8x3) names"},
    {"NODOS*NAME"   , "Do not show DOS (8x3) names"},
    {"L*ONGNAME"    , "Show long names"},
    {"NOL*ONGNAME"  , "Do not show long names"},
    {"D*OWN"        , "Sort in columns"},
    {"A*CROSS"      , "Sort in rows"},
    {"E*XPAND"      , "Expand column widths for longest name"},
    {"NOE*XPAND"    , "Do not expand column widths"},
    { NULL          , NULL}
};

SubOpts sortcmd[] =
{   {"NO*NE"      , "Do not sort"},
    {"A*SCENDING" , "Sort in ascending order"},
    {"R*EVERSE"   , "Sort in reverse order"},
    {"N*AME"      , "Sort by name"},
    {"E*XTENSION" , "Sort by extension"},
    {"D*ATE"      , "Sort by date"},
    {"SI*ZE"      , "Sort by size"},
    {"DI*RFIRST"  , "Sort directories first"},
    {"NODI*RFIRST", "Do not sort directories first"},
    { NULL        , NULL}
};

// Variables addressed directly in option table

long   shownames = TRUE;	// TRUE if file names should be shown
long   debug = FALSE;		// TRUE if want debug output
long   page = FALSE;		// Page flag
long   lsum = FALSE;		// TRUE if should list summary at end
long   onlytotals = FALSE;	// TRUE if should list totals only

arg_spec options[] =
{

/// {"AF*TER"  , ASF_VALREQ|ASF_LSVAL, NULL   ,     optafter  ,
///	    0, "Show files after"},

    {"AL*L"    , 0                   , NULL   , OPT(allhave)  ,
	    A_NORMAL|A_RDONLY|A_HIDDEN|A_SYSTEM|A_DIRECT, "Show all files"},

/// {"B*EFORE" , ASF_VALREQ|ASF_LSVAL, NULL   ,     optbefore ,
///	    0, "Show files before"},

    {"DE*BUG"  , ASF_BOOL|ASF_STORE  , NULL   ,    &debug     ,
	    1, NULL},
    {"DIR"     , 0                   , NULL   , OPT(optdir)   ,
	    0, "Show directories only"},
    {"NODIR"   , 0                   , NULL   , OPT(optnodir)   ,
	    0, "Do not show directories"},
    {"DI*SPLAY", ASF_XSVAL           , &discmd,     optdisplay,
	    0, "Specify display options"},
    {"F*ULL"   , 0                   , NULL   , OPT(optfull)  ,
	    0, "Display full listing"},
    {"H*ELP"   , 0                   , NULL   ,     opthelp   ,
	    0, "Display this message" },
    {"L*ONG"   , 0                   , NULL   , OPT(optlong)  ,
	    0, "Show file details"},
    {"N*AMES"  , ASF_BOOL|ASF_STORE  , NULL   ,    &shownames ,
	    1, "Show file names"},
    {"O*NE"    , 0                   , NULL   , OPT(optone)   ,
	    0, "Display single column, short listing"},
    {"P*AUSE"  , ASF_BOOL|ASF_STORE  , NULL   ,    &page      ,
	    1, "Pause at end of page" },
    {"SH*ORT"  , 0                   , NULL   , OPT(optshort) ,
	    0, "Display short listing" },
    {"SO*RT"   , ASF_XSVAL           , sortcmd,     optsort   ,
	    0, "Specify output order" },
    {"T*OTALS" , ASF_BOOL|ASF_STORE  , NULL   ,    &onlytotals,
	    1, "Show totals only (no names)" },
    {"W*IDE"   , 0                   , NULL   , OPT(optshort) ,
	    0, "Display short listing"},
    {"X"       , ASF_VALREQ|ASF_LSVAL, NULL   ,     optx      ,
	    0, "Specify files to exclude"},
    {"?"       , 0                   , NULL   ,     opthelp   ,
	    0, "Display this message"},
    {NULL      , 0                   , NULL   ,     NULL      ,
	    0, NULL}
};

// Option related variables

char   longfmt = FALSE;		// TRUE if long listing
char   fullfmt = FALSE;		// TRUE if full detailed listing
char   prtvolid = FALSE;	// TRUE if should print the volume ID
char   prtpath = FALSE;		// TRUE if should print the path name
char   showattr = TRUE;		// TRUE if should show file attributes
char   onecol = FALSE;		// TRUE if one column simple display
char   datesort = FALSE;	// TRUE if should sort by date
char   extsort = FALSE;		// TRUE if should sort by file extension
char   revsort = FALSE;		// TRUE for reverse sort listing
char   dirsort = FALSE;		// TRUE if should sort subdirectories first
char   sizesort = FALSE;	// TRUE if should sort on file size
char   nosort = FALSE;		// TRUE if should not sort
char   prtacross = TRUE;	// TRUE if should print across
char   prtexpand = FALSE;	// TRUE if should expand columns
char   prtnamedos = FALSE;
char   prtnamelong = TRUE;
char   changed;
int    needed;			// Spaces needed for longest name
int    namesize;		// Length of last file name seen
int    needlength = -1;		// Offset of file length in filedescp
int    needalloc = -1;		// Offset of file allocation in filedescp
int    needcdate = -1;		// Offset of creation date/time in filedecscp
int    needmdate = -1;		// Offset of modify date/time in filedescp
int    needadate = -1;		// Offset of access date/time in filedescp
int    needowner = -1;		// Offset of file owner names in filedescp
int    needprot = -1;		// Offset of file protection in filedescp
int    neednamedos = -1;	// Offset of DOS file name in filedescp
int    needname = 0;		// Offset of file name in filedescp
int    curline = -1;		// Current line on the screen
time_s after_time;		// Time after to display filenames
time_s before_time;		// Time before to display filenames
char   errdone;			// TRUE if have output an error message
char   hvdefault;
int    numlisted = 0;		// Number of directories listed
long   numfiles;
long   filecnt;
long   dircnt;
LLONG  totallen;
LLONG  totalalloc;
long   gfilecnt;
long   gdircnt;
LLONG  gtotallen;
LLONG  gtotalalloc;
char  *fspnt;

// Misc. variables

Prog_Info pib;
char    copymsg[] = "";
char    prgname[] = "DIR";	// Our programe name
char    envname[] = "^XOS^DIR^OPT"; // The environment option name
char    example[] = "{/options} filespec";
char    description[] = "This command produces a directory listing of the " \
    "files on a specified disk drive.  The file specification given " \
    "determines which files are included in the listing.  Wildcard and " \
    "elipsis notation are allowed in the file specification.  If no file " \
    "specification is given, *.* is assumed.  More than one file and/or " \
    "wildcard may be specified on the command line.  Many options are " \
    "possible to the directory listing.  It is recommended that the " \
    "user select a preferred directory format using command line options " \
    "and enter it as the default in the USTARTUP.BAT file using the " \
    "DEFAULT command.";

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

struct diparm
{   byte4_parm cblksz;
    byte4_parm clssz;
    byte4_parm avail;
    char      end;
} diparm =
{   {PAR_GET|REP_DECV, 4, IOPAR_DSKSECTSIZE},
    {PAR_GET|REP_DECV, 4, IOPAR_DSKCLSSIZE},
    {PAR_GET|REP_DECV, 4, IOPAR_DSKAVLSPACE}
};

struct
{   char   type[8];
    time_s date;
    char   xxx[8];
    char   name[36];
} vollabel;

type_qab lblqab =
{   QFNC_WAIT|QFNC_LABEL,	// qab_open
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    0,				// qab_handle
    0,				// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    1,				// qab_option
    sizeof(vollabel),		// qab_count
    (char *)&vollabel, 0,	// qab_buffer1
    NULL, 0,			// qab_buffer2
    NULL, 0			// qab_parm
};

extern uint _malloc_amount;
uint   maxmem;

void main(
    int   argc,
    char *argv[])

{
    char *pnt;
    char *envpnt[2];
    FS   *temp;
    long  rtn;
    char  strbuf[256];			// String buffer

    // Set defaults

	reg_pib(&pib);

	init_Vars();

    // Check Global Configuration Parameters

    global_parameter(TRUE);

    prtvolid = TRUE;					// Print the volume ID message
    prtpath = TRUE;						// Print the path name
    nosort  = FALSE;					// Don't sort by default
    lsum = TRUE;						// Listing summary

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

    // Set up the conditions

    if (onlytotals)
    {
	lsum = TRUE;
	shownames = FALSE;
    }
    if (gcp_dosdrive)
        fileparm.filoptn.value = FO_DOSNAME|FO_NODENUM|FO_NODENAME|FO_RDOSNAME|
                FO_PATHNAME|FO_ATTR;
    if (prtnamedos)
		fileparm.filoptn.value |= FO_FILEDOS;
    if (!prtnamedos || prtnamelong)
		fileparm.filoptn.value |= (FO_FILENAME|FO_VERSION);
    if (firstspec == NULL)
		nonopt(WILDCARD);
    if (!pib.console)
        page = FALSE;
    if (datesort)						// Want date sort?
    {									// Yes
        if (needcdate < 0)				// Already getting the date?
        {								// No - fix it up so we will get it
			needcdate = ((needalloc >= 0)? 4: 0) + ((needprot >= 0)? 4: 0) +
					((needowner >= 0)? 36: 0) + ((needadate >= 0)? 8: 0) +
					((needmdate >= 0)? 8: 0);
            if (needlength >= 0)
                needlength += 8;
			if (neednamedos >= 0)
				neednamedos += 8;
			needname += 8;
        }
    }
    else if (sizesort)					// Want size sort?
    {									// Yes
        if (needlength < 0)				// Already getting the size?
        {								// No - fix it up so we will get it
            needlength = ((needalloc >= 0)? 4: 0) + ((needprot >= 0)? 4: 0) +
					((needowner >= 0)? 36: 0) + ((needadate >= 0)? 8: 0) +
					((needmdate >= 0)? 8: 0) + ((needcdate >= 0)? 8: 0);
			if (neednamedos >= 0)
				neednamedos += 8;
			needname += 8;
        }
    }
    if (prtnamedos && prtnamelong)		// Want to display both DOS name and
    {									//   the long name?
		neednamedos = needname;			// Yes
		needname += 16;
    }
    if (needname == 0 || neednamedos == 0) // Need to do long search?
    {
        dsd.repeat = TRUE;				// No - do it the quick way
        fileparm.devsts.desp = 0;
    }
    if (!longfmt && !fullfmt && prtnamedos) // If wide format and short names?
		prtnamelong = FALSE;			// Yes - don't get long names

    // Call DIRSCAN to get the names

    thisspec = firstspec;
    do									// Scan the directory
    {
		if (thisspec->link != NULL)
		{
			rtn = thisspec->name - thisspec->spec;
			pnt = getmem(strlen(thisspec->name) + rtn + 4);
			strmov(strnmov(pnt, thisspec->spec, rtn), "*.*");
			dirscan(pnt, &dsd);
			free(pnt);
		}
		else
			dirscan(thisspec->spec, &dsd);
		if (numfiles != 0)				// Anything left to print?
			printfiles();				// Yes - print it
		temp = thisspec->next;
		free(thisspec);					// Give up storage for file spec
	} while ((thisspec = temp) != NULL); // Continue if more to do

	// Display final summary if need to

	if (lsum && (numlisted > 1))
	{
		pagecheck();
		putchar('\n');
		pagecheck();
		printf("[%,d director%s listed containing a grand total of\n", numlisted,
				(numlisted == 1) ? "y": "ies");
		pnt = strbuf;
		if (gdircnt != 0)
			pnt += sprintf(pnt, " %,ld director%s,", gdircnt, (gdircnt == 1) ?
					"y": "ies");
		pnt += sprintf(pnt, " %,ld file%s", gfilecnt, (gfilecnt == 1) ? "" : "s");
		if (longfmt)
	    	pnt += sprintf(pnt, ", %,lld byte%s written, %,lld byte%s allocated",
		    		gtotallen, (gtotallen.low == 1 && gtotallen.high == 0) ?
					"" : "s", gtotalalloc, (gtotalalloc.low == 1 &&
					gtotalalloc.high == 0) ? "" : "s");
		pagecheck();
		strmov(pnt, "]\n");
		fputs(strbuf, stdout);
	}
    if (!errdone && (gdircnt + gfilecnt) == 0)
		fputs("? DIR: File not found\n", stderr);
    if (debug)
    {
		pagecheck();
		putchar('\n');
		printf("Maximum memory: %,ld, current memory: %,ld\n", maxmem,
				_malloc_amount);
	}
	exit(EXIT_NORM);					// Return with no error
}

//***********************************************************
// Function: init_Vars - Set up the program information block
// Returned: Nothing
//***********************************************************

void init_Vars(void)

{
    // Set Program Information Block variables

    pib.opttbl=options; 				// Load the option table
    pib.kwdtbl=NULL;
    pib.build=__DATE__;
    pib.majedt = VERSION; 				// Major edit number
    pib.minedt = EDITNO; 				// Minor edit number
    pib.copymsg=copymsg;
    pib.prgname=prgname;
    pib.desc=description;
    pib.example=example;
    pib.errno=0;
    getTrmParms();
    getHelpClr();
}

//*****************************************************************
// Function: allhave - Function called progarg when ALL is selected
// Returned: TRUE if should continue, FALSE if should terminate
//*****************************************************************

int allhave( arg_data *arg )

{
    fileparm.srcattr.value=arg->data;
    return TRUE;
}


//**************************************************************
// Function: procfile - Function called by dirscan for each file
// Returned: TRUE if should continue, FALSE if should terminate
//**************************************************************

// This function is called by dirscan for each file found.

int procfile(void)

{
    char  *pnt;
    FS    *spec;
    XNAME *name;
    int    fill;

    changed |= dsd.changed;
    if (thisspec->link != NULL)			// Is this a complex search?
    {
		spec = thisspec;				// Yes - scan through the list of names
		do								//   and check each one
		{
			if (wildcmp(spec->name, dsd.filename,
					(spec->devsts & DS_CASESEN) ? 1 : 0) == 0)
				break;
		} while ((spec = spec->link) != NULL);
		if (spec == NULL)				// Did we find a match?
			return (TRUE);				// No - skip this one
	}
    if (firstname != NULL)				// Have any excluded names?
    {
		name = firstname;				// Yes - check for match
		do
		{
			if (wildcmp(name->name, dsd.filename,
					(spec->devsts & DS_CASESEN) ? 1 : 0) == 0)
				return (TRUE);			// Match - skip this one
		} while ((name = name->next) != NULL);
    }

    // Here if want to process this file - it is not excluded and we are doing
    //   a simple search or it matches one of our names for a complex search

    if (changed && (numfiles != 0))
	printfiles();
    if (dsd.error < 0 && ((!fullfmt && !longfmt) ||
			(dsd.error != ER_FILAD && dsd.error != ER_FBFER &&
			dsd.error != ER_FBFER && dsd.error != ER_FBWER &&
			dsd.error != ER_BUSY)))
    {
		char buffer[256];

		pnt = strmov(buffer, dsd.devname);
		fill = 254-dsd.devnamelen;
		pnt = strnmov(pnt, dsd.rmtdata.nodename, fill);
		if ((fill -= dsd.rmtdata.nodenamelen) > 0)
		{
			pnt = strnmov(pnt, dsd.rmtdata.rmtdevname, fill);
			if ((fill -= dsd.rmtdata.rmtdevnamelen) > 0)
			{
				pnt = strnmov(pnt, dsd.pathname, fill);
				if ((fill -= dsd.pathnamelen) > 0)
					pnt = strnmov(pnt, dsd.filename, fill);
			}
		}
		buffer[255] = '\0';				// Make sure have null at end
		errormsg(buffer, dsd.error);
		return (FALSE);
    }
	if ((dsd.filenamelen + dsd.filedoslen) == 0)
		return (TRUE);					// Finished here if no name
    if (changed)
        printheading();					// First get length of file spec
    changed = FALSE;
    if (!(fileparm.srcattr.value & A_HIDDEN) && dsd.filenamelen > 0 &&
			dsd.filenamelen <= 2 && dsd.filename[0] == '.' &&
			(dsd.filenamelen == 1 || dsd.filename[1] == '.'))
										// . or .. entry and not listing
										//    hidden files?
	return (TRUE);						// Yes - discard it

    if (prtnamelong)					// Displaying long name?
    {									// Yes
		thisfile = getmem(sizeof(FILEDESCP) - 1 + dsd.filenamelen +
				needname);				// Allocate memory for entry
		namesize = dsd.filenamelen;
		if (dsd.attr & A_DIRECT)
			namesize++;
		if (needed < namesize)			// Is this a new maximum size?
			needed = namesize;			// Yes - remember it

		strmov(NAME(thisfile), dsd.filename); // Copy the file name

		if (neednamedos >= 0)			// Also need DOS name?
			strmov(NAMEDOS(thisfile), dsd.filedos);
    }
    else
    {
		thisfile = getmem(sizeof(FILEDESCP) - 1 + dsd.filedoslen +
				needname);				// Allocate memory for entry
		strmov(NAME(thisfile), dsd.filedos);
    }
    numfiles++;
    thisfile->error = dsd.error;

    if (firstfile == NULL)
        firstfile = thisfile;
    else
        lastfile->next = thisfile;
    lastfile = thisfile;
    thisfile->next = NULL;
    if (needlength >= 0)
		*LENGTH(thisfile) = fileparm.length.value;
    if (needalloc >= 0)
		*ALLOC(thisfile) = fileparm.alloc.value;
    if (needcdate >= 0)
		*CDATE(thisfile) = *(time_s *)(&fileparm.cdate.value);
    if (needmdate >= 0)
		*MDATE(thisfile) = *(time_s *)(&fileparm.mdate.value);
    if (needadate >= 0)
		*ADATE(thisfile) = *(time_s *)(&fileparm.adate.value);
    if (needprot >= 0)
		*PROT(thisfile) = fileparm.prot.value;
    if (needowner >= 0)
		strcpy(OWNER(thisfile), owner);
    if ((thisfile->filattr = dsd.attr) & A_DIRECT)
		dircnt++;
	else
		filecnt++;
	if (dsd.error >= 0)
	{
		longadd(&totallen, fileparm.length.value);
		longadd(&totalalloc, fileparm.alloc.value);
	}
    return (TRUE);
}

//*************************************************************
// Function: printheading - Print heading for directory listing
// Returned: Nothing
//*************************************************************

void printheading(void)

{
    char *linepnt;
    char *pnt;
    char  linebufr[200];

    if ((dsd.changed & (DSCHNG_DEVNAME|DSCHNG_NODENAME|DSCHNG_RDEVNAME)) &&
            prtvolid)
    {
        lblqab.qab_handle = fileparm.dirhndl.value;
        if (svcIoQueue(&lblqab) < 0 || lblqab.qab_error < 0 ||
                lblqab.qab_amount < 25)
            vollabel.name[0] = '\0';
    }
    linepnt = linebufr;
    pagecheck();
    putchar('\n');
    if (prtvolid)
    {
	linepnt += sprintf(linepnt ,"Volume: %s%s%s", dsd.devname,
		dsd.rmtdata.nodename, dsd.rmtdata.rmtdevname);
	if (vollabel.name[0] != '\0')
	    linepnt += sprintf(linepnt, " (%.36s)", vollabel.name);
	linepnt = strmov(linepnt, (prtpath)? "  ": "\n");
    }
    pnt = linepnt;
    if (prtpath)
	linepnt += sprintf(linepnt, "Path: %s", dsd.pathname);
    if (prtvolid || prtpath)
    {
	pagecheck();
	strmov(linepnt, "\n");
	if (prtvolid && prtpath &&
		((int)(linepnt - linebufr) > pib.screen_width))
	{
	    strmov(pnt-2, "\n");
	    fputs(linebufr, stdout);
	        pagecheck();
	        fputs(pnt, stdout);
	}
	else
	    fputs(linebufr, stdout);
    }
}

//**************************************************************
// Function: printfiles - Print file names for directory listing
// Returned: Nothing
//**************************************************************

int        colwidth;
int        collength;
int        fill;
int        left;
int        numcolumns;
FILEDESCP *colarray[5];

void printfiles(void)

{
    FILEDESCP  *file;
    FILEDESCP  *next;
    FILEDESCP **pnt;
    char       *linepnt;
    char       *cpnt;
    int         cnt;
    int         size;
    int         temp;
    char        chr;
    char        linebufr[200];

    if (shownames)
    {
        if (longfmt)
            prtacross = TRUE;
        if (!prtacross)
            prtexpand = TRUE;
        if (!nosort)
            firstfile = heapsort(firstfile,
                    (int (*)(void *a, void *b, void *d))comp, NULL);

        if (!longfmt)					// Want short (wide) listing?
        {								// Yes
			if ((numcolumns = (onecol)? 1: (pib.screen_width /
					((prtexpand) ? (needed + 1) : COLWIDTH))) > 5)
				numcolumns = 5;
			colwidth = pib.screen_width/numcolumns;
			linepnt = linebufr;
            left = numcolumns;
            if (prtacross)
            {
                do						// Loop for each file
                {
                    linepnt = wideentry(firstfile, linebufr, linepnt);
                    next = firstfile->next;
                    free(firstfile);	// Give up description block
                } while ((firstfile = next) != NULL);
                pagecheck();
                strmov(linepnt, "\n");
                fputs(linebufr, stdout); // Output the final line
            }
            else
            {
                do
                {   collength = (int)((numfiles+numcolumns-1)/numcolumns);
                    if (page)
                    {
                        if ((temp = pib.screen_height - curline - 3) <= 0)
                            temp = pib.screen_height - 3;
                        if (collength > temp)
                            collength = temp;
                    }
                    numfiles -= collength * numcolumns;
                    cnt = numcolumns;
                    pnt = colarray;
                    do
                    {   *pnt++ = firstfile;
                        temp = collength;
                        while (firstfile != NULL && --temp > 0)
                            firstfile = firstfile->next;
                        if (firstfile != NULL)
                        {
                            next = firstfile->next;
                            firstfile->next = NULL;
                            firstfile = next;
                        }
                    } while (--cnt > 0);
                    do
                    {   cnt = numcolumns;
                        pnt = colarray;
                        while ((file = *pnt) != NULL && --cnt >= 0)
                        {
                            linepnt = wideentry(file, linebufr, linepnt);
                            *pnt = file->next;
                            free(file);
                            pnt++;
                        }
                        pagecheck();
                        strmov(linepnt, "\n");
                        fputs(linebufr, stdout);
                        linepnt = linebufr;
                        left = numcolumns;
                    } while (colarray[0] != NULL);
                    if (page && firstfile != NULL)
                    {
                        pagecheck();
                        putchar('\n');
					}
                } while (firstfile != NULL);
            }
        }
        else if (!fullfmt)				// Want long listing?
        {								// Yes
            do							// Loop for each file
            {
				linepnt = linebufr;
				if (prtnamedos || !prtnamelong) // Want the DOS name
				{								//   displayed?
				    cnt = 14;			// Yes
				    cpnt = (prtnamelong) ? NAMEDOS(firstfile) : NAME(firstfile);

					while ((chr = *cpnt++) != 0)
					{
						*linepnt++ = chr;
						--cnt;
					}
					if (firstfile->filattr & A_DIRECT) // Is it a directory?
					{
						*linepnt++ = '\\'; // Yes - put \ after name
						--cnt;
					}
					while (--cnt > 0)
						*linepnt++ = ' ';
				}
				if (firstfile->error >= 0)
				{
					if (showattr)		// Want to display attributes?
					{					// Yes
						*linepnt++ = ((firstfile->filattr & A_RDONLY) != 0) ?
								'R' : '-';
						*linepnt++ = ((firstfile->filattr & A_HIDDEN) != 0) ?
								'H' : '-';
						*linepnt++ = ((firstfile->filattr & A_SYSTEM) != 0) ?
								'S' : '-';
						*linepnt++ = ((firstfile->filattr & A_ARCH) != 0) ?
								'M' : '-';
					}
					linepnt += sdt2str(linepnt, "%Z %h:%m:%s %D-%3n-%l",
							(time_sz *)(CDATE(firstfile)));
					linepnt += sprintf(linepnt, "%,14ld",
							*LENGTH(firstfile));
				}
				else
				{
					linepnt += (int)svcSysErrMsg(firstfile->error, 3, linepnt);
					size = ((showattr) ? 38 : 33) - (int)(linepnt - linebufr);
					while (--size > 0)
						*linepnt++ = ' ';
				}
				if (prtnamelong)
					linepnt += sprintf(linepnt, " %s%s", NAME(firstfile),
							(firstfile->filattr & A_DIRECT)? "\\": "");
                pagecheck();
                strmov(linepnt, "\n");
                fputs(linebufr, stdout); // Output the line
                next = firstfile->next;
                free(firstfile);		// Give up description block
            } while ((firstfile = next) != NULL);
        }
        else							// Must want full listing
        {
			do							// Loop for each file
			{
				pagecheck();
				printf("File: %s%s\n", NAME(firstfile),
						(firstfile->filattr & A_DIRECT)? "\\": "");
				pagecheck();
				if (firstfile->error >= 0)
				{
					linepnt = strmov(linebufr, "  File attributes: ");
					linepnt = strmov(linepnt, ((firstfile->filattr & A_RDONLY)
							!= 0)? "Read only, ": "Read/write, ");
					linepnt = strmov(linepnt, ((firstfile->filattr & A_HIDDEN)
							!= 0)? "Hidden, ": "Visible, ");
					linepnt = strmov(linepnt, ((firstfile->filattr & A_SYSTEM)
							!= 0)? "System, ": "User, ");
					linepnt = strmov(linepnt, ((firstfile->filattr & A_ARCH)
							!= 0)? "Modified\n": "Not modified\n");
					fputs(linebufr, stdout);
					sdt2str(linebufr, "%Z  Created:  %h:%m:%s %D-%3n-%l\n",
							(time_sz *)(CDATE(firstfile)));
					pagecheck();
					fputs(linebufr, stdout);
					sdt2str(linebufr, "%Z  Modified: %h:%m:%s %D-%3n-%l\n",
							(time_sz *)(MDATE(firstfile)));
					pagecheck();
					fputs(linebufr, stdout);
					sdt2str(linebufr, "%Z  Accessed: %h:%m:%s %D-%3n-%l\n",
							(time_sz *)(ADATE(firstfile)));
					pagecheck();
					fputs(linebufr, stdout);
					pagecheck();
					printf("  Written length:   %,ld\n", *LENGTH(firstfile));
					pagecheck();
					printf("  Allocated length: %,ld\n", *ALLOC(firstfile));
				}
				else					// If have error
				{
                    linepnt = strmov(linebufr,
                            "  Can not obtain file information: ");
                    svcSysErrMsg(firstfile->error, 3, linepnt);
                    strmov(linepnt, "\n");
                    fputs(linebufr, stdout);
                }
                next = firstfile->next;
                free(firstfile);		// Give up description block
            } while ((firstfile = next) != NULL);
        }
    }
    if (lsum)
    {
        pagecheck();
		linepnt = strmov(linebufr, "[");
		if (dircnt != 0)
			linepnt += sprintf(linepnt, "%,d director%s, ", dircnt,
					(dircnt==1)? "y": "ies");
		linepnt += sprintf(linepnt, "%,d file%s", filecnt,
				(filecnt==1) ? "" : "s");
		if (longfmt)
			linepnt += sprintf(linepnt, ",%s %,lld bytes written, %,lld bytes"
					" allocated", (longfmt && dircnt != 0) ? "\n" : "",
					totallen, totalalloc);
		strmov(linepnt, "]\n");
		fputs(linebufr, stdout);
    }
    firstfile = NULL;
    gfilecnt += filecnt;
    filecnt = 0;						// Add in to the grand totals and
    gdircnt += dircnt;					//   reset some counts
    dircnt = 0;
    longlongadd(&gtotallen, &totallen);
    totallen.low = totallen.high = 0;
    longlongadd(&gtotalalloc, &totalalloc);
    totalalloc.low = totalalloc.high = 0;
    numfiles = 0;
    numlisted++;						// Count the directory we have listed
    return;
}

//************************************************
// Function: wideentry - Put file name into buffer
//		for wide format listing
// Returned: Updated buffer pointer
//************************************************

char *wideentry(
    FILEDESCP *file,
    char      *linebufr,
    char      *linepnt)

{
    int   size;

    size = strlen(NAME(file));			// Get length of file name
    if (file->filattr & A_DIRECT)		// Is it a directory?
        size++;							// Yes - Allow space for the
										//   trailling backslash
    needed = (size + colwidth)/colwidth; // Get number of columns
    if (needed > left)					// Do we have enough left?
    {									// No - start a new line
        pagecheck();
        strmov(linepnt, "\n");
        fputs(linebufr, stdout);
        linepnt = linebufr;
        left = numcolumns;
    }									// Have more space on line - get
    else if (left != numcolumns)		//   number of spaces to fill
    {									//   out previous field
        fill = colwidth - (int)(linepnt - linebufr) % colwidth;
        do
        {   *linepnt++ = ' ';			// Output that many spaces
        } while(--fill > 0);
    }
    linepnt = strmov(linepnt, NAME(file));
    left -= needed;			// Copy file name
    if (file->filattr & A_DIRECT)		// Is it a directory?
        *linepnt++ = '\\';				// Yes - put \ after name
    return (linepnt);
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
    char  *aext;
    char  *bext;
    int    retval;
    char   onetype;

    if (dirsort)			// Sorting directories first?
    {					// Yes - see if only one is directory
        onetype = one->filattr & A_DIRECT;
        if (onetype != (two->filattr & A_DIRECT))
            return ((onetype)? -1: 1);	// Yes - this overrides all else!
    }
    if (revsort)						// Want reverse order sort?
    {
        FILEDESCP *temp;

        temp = one;
        one = two;
        two = temp;
    }
    if (datesort)						// Date sort?
    {									// Yes - same date?
        if (CDATE(one)->date != CDATE(two)->date)
            return ((CDATE(one)->date < CDATE(two)->date)? -1: 1);
										// No - base order on date
        if (CDATE(one)->time != CDATE(two)->time)
            return ((CDATE(one)->time < CDATE(two)->time)? -1: 1);
										// Yes - base order on time
    }									// If same time, sort by name
    else if (extsort)					// Extension sort?
    {
        aext = strrchr(NAME(one), '.');	// Yes - find extensions
        bext = strrchr(NAME(two), '.');
        if (aext != NULL && bext != NULL) // Have both extensions?
        {								// Yes - base it on this if different
            if ((retval = strcmp(aext, bext)) != 0)
                return (retval);
        }								// If same, base it on name
        else if (aext != bext)			// One or no extensions
            return ((aext == NULL)? -1: 1); // One with no extension is first
    }									// No extensions - base it on name
    else if (sizesort)					// File length sort?
    {									// Yes
        if (*LENGTH(one) != *LENGTH(two)) // If different, use length
            return ((*LENGTH(one) < *LENGTH(two))? -1: 1);
    }									// If same length, use name
    return (stricmp(NAME(one), NAME(two))); // Sort on name
}

//******************************************************************
// Function: pagecheck - Check the page count and pause if necessary
// Returned: Nothing
//******************************************************************

void pagecheck(void)

{
    long temp;					// Temp int for keyboard status
    int  more;

    if (page && (++curline >= pib.screen_height - 2))
    {
        fputs("\33[7m-MORE- ^C, G, H, Q, <Enter> or <Space>\33[0m", stdout);
        more = TRUE;
        while (more == TRUE)
        {
            temp = svcIoInSingleP(DH_STDTRM, &snglparm);

            normparm.modes = snglparm.modev;
            svcIoInBlockP(DH_STDTRM, NULL, 0, &normparm);
            more = FALSE;				// Assume good input

            switch (toupper(((int)temp) & 0x7F))
            {
             case 'Q':					// Quit!, don't print the rest
             case 0x03:
                fputs("\r\33[K", stdout);
                exit(EXIT_NORM);

             case 'G':					// Don't ask for any more
                page = FALSE;
                break;

             case '\r':
             case ' ':					// Do another screen full
                curline = 0;
                break;

             case 'H':
             case '?':
             default:					// Tell him this is wrong
				fputs("\r\33[K\33[7m"
						" ^C      - Exit                    \n"
						"  G      - Go, don't ask for -MORE-\n"
						"  H or ? - Help, this message      \n"
						"  Q      - Quit program            \n"
						" <Enter> - Next screen             \n"
						" <Space> - Next screen             \n"
						"-MORE- ^C, G, H, Q, <Enter> or <Space>\33[0m",
						stdout);

              case 0:
                more = TRUE;			// Loop back and check again
                break;
            }
        }
        fputs("\r\33[K", stdout);
    }
}


//**********************************************
// Function: optdisplay - Process DISPLAY option
// Returned: Nothing
//**********************************************

int optdisplay(
    arg_data *arg)

{
    if ((arg->flags & ADF_XSVAL) == 0)
	return (TRUE);
    switch ((int)arg->val.n)
    {
     case 0:			// TOTALS
        lsum = TRUE;
        break;

     case 1:			// NOTOTALS
        lsum = FALSE;
        break;

     case 2:			// PATH
        prtpath = TRUE;
        break;

     case 3:			// NOPATH
        prtpath = FALSE;
        break;

     case 4:			// VOLUME
        prtvolid = TRUE;
        break;

     case 5:			// NOVOLUME
        prtvolid = FALSE;
        break;

     case 6:			// ATTRIBUTES - Show file attributes
		showattr = TRUE;
		break;

     case 7:			// NOATTRIBUTES - Do not show file attributes
		showattr = FALSE;
		break;

     case 8:			// DOSNAME
		prtnamedos = TRUE;
		break;

     case 9:			// NODOSNAME
		prtnamedos = FALSE;
		prtnamelong = TRUE;
		break;

     case 10:			// LONGNAME
		prtnamelong = TRUE;
		break;

     case 11:			// NOLONGNAME
		prtnamelong = FALSE;
		prtnamedos = TRUE;
		break;

     case 12:			// DOWN
        prtacross = FALSE;
        break;

     case 13:			// ACROSS
        prtacross = TRUE;
        break;

     case 14:			// EXPAND
        prtexpand = TRUE;
        break;

     case 15:			// NOEXPAND
        prtexpand = FALSE;
        break;

     default:
		fprintf(stderr, "? %s: Invalid DISPLAY option value, %d\n",
				prgname, arg->val.n);
		exit(EXIT_INVSWT);
    }
    return (TRUE);
}

//******************************************
// Function: optafter - Process AFTER option
// Returned: Nothing
//******************************************

int optafter(
    arg_data *arg)

{
    char *time_ptr;

    time_ptr = arg->val.s;
    printf("after time = %s\n", time_ptr);
    return (TRUE);
}

//**************************************
// Function: optall - Process ALL option
// Returned: Nothing
//**************************************

int optall(void)

{
    fileparm.srcattr.value = A_NORMAL|A_RDONLY|A_HIDDEN|A_SYSTEM|A_DIRECT;
    return (TRUE);
}

//********************************************
// Function: optbefore - Process BEFORE option
// Returned: Nothing
//********************************************

int optbefore(
    arg_data *arg)

{
    char *time_ptr;

    time_ptr = arg->val.s;
    printf("before time = %s\n", time_ptr);
    return (TRUE);
}

//**************************************
// Function: optdir - Process DIR option
// Returned: Nothing
//**************************************

int optdir(void)

{
    fileparm.srcattr.value = A_DIRECT;	// Look for directories only
    return (TRUE);
}

//******************************************
// Function: optnodir - Process NODIR option
// Returned: Nothing
//******************************************

int optnodir(void)

{
    fileparm.srcattr.value &= ~A_DIRECT; // Do not look for directories
    return (TRUE);
}

//**************************************
// Function: optone - Process ONE option
// Returned: Nothing
//**************************************

int optone(void)

{
    onecol = TRUE;
    optshort();
    return (TRUE);
}

//****************************************
// Function: optfull - Process FULL option
// Returned: Nothing
//****************************************

int optfull(void)

{
    fullfmt = TRUE;
    longfmt = TRUE;
    needalloc = 0;
    needowner = 4;
    needprot = 40;
    needadate = 44;
    needmdate = 52;
    needcdate = 60;
    needlength = 68;
    needname = 72;
    return (TRUE);
}

//******************************************
// Function: optshort - Process SHORT option
// Returned: Nothing
//******************************************

int optshort(void)

{
    longfmt = FALSE;
    fullfmt = FALSE;
    needalloc = -1;
    needowner = -1;
    needprot = -1;
    needadate = -1;
    needmdate = -1;
    needcdate = -1;
    needlength = -1;
    needname = 0;
    return (TRUE);
}

//****************************************
// Function: optlong - Process LONG option
// Returned: Nothing
//****************************************

int optlong(void)

{
    longfmt = TRUE;
    fullfmt = FALSE;
    needalloc = -1;
    needowner = 0;
    needprot = 36;
    needadate = -1;
    needmdate = -1;
    needcdate = 40;
    needlength = 48;
    needname = 52;
    return (TRUE);
}

//****************************************
// Function: optsort - Process SORT option
// Returned: Nothing
//****************************************

int optsort(
    arg_data *arg)

{
    if ((arg->flags & ADF_XSVAL) == 0)
    {
        datesort = TRUE;
        revsort = TRUE;
        extsort = FALSE;
        nosort = FALSE;
		return (TRUE);
    }
    switch ((int)arg->val.n)
    {
     case 0:				// NONE - Do not sort
        nosort = TRUE;
        dirsort = FALSE;
        revsort = FALSE;
        extsort = FALSE;
        datesort = FALSE;
        break;

     case 1:				// ASCENDING - Sort in ascending order
        revsort = FALSE;
        nosort = FALSE;
        break;

     case 2:				// REVERSE - Sort in reverse order
        revsort = TRUE;
        nosort = FALSE;
        break;

     case 3:				// NAME - Sort by name
        datesort = FALSE;
        extsort = FALSE;
        sizesort = FALSE;
        nosort = FALSE;
        break;

     case 4:				// EXTENSION - Sort by extension
        extsort = TRUE;
        datesort = FALSE;
        sizesort = FALSE;
        nosort = FALSE;
        break;

     case 5:				// DATE - Sort by date
        datesort = TRUE;
        extsort = FALSE;
        sizesort = FALSE;
        nosort = FALSE;
        break;

     case 6:				// SIZE - Sort by size
        sizesort = TRUE;
        extsort = FALSE;
        datesort = FALSE;
        nosort = FALSE;
        break;

     case 7:				// DIRFIRST - Sort directories first
        dirsort = TRUE;
        nosort = FALSE;
        break;

     case 8:				// NODIRFIRST - Do not sort directories
        dirsort = FALSE;		//   first
        nosort = FALSE;
        break;

     default:
        fprintf(stderr, "? %s: Invalid SORT option value, %d\n",
                prgname, arg->val.n);
        exit(EXIT_INVSWT);
    }
    return (TRUE);
}

//**********************************
// Function: optx - Process X option
// Returned: Nothing
//**********************************

int optx(
    arg_data *arg)

{
    XNAME *name;

    name = getmem(arg->length + sizeof(XNAME) + 2);
    strmov(name->name, arg->val.s);
    name->next = firstname;
    firstname = name;
    return (TRUE);
}

//********************************************
// Function: nonopt - process non-option input
// Returned: Nothing
//********************************************

int nonopt(
    char *arg)

{
    FS   *spec;
    FS   *spnt;
    char *pnt;
    long  rtn;
    int   size;
    char  chr;

    static struct
    {	byte16_parm  glbid;
		byte4_parm   devsts;
		char         end;
    } glbidparm =
    {	{PAR_GET|REP_HEXV, 16, IOPAR_GLBID , 0},
		{PAR_GET|REP_HEXV,  4, IOPAR_DEVSTS, 0}
    };

    hvdefault = FALSE;
    size = strlen(arg);
    if ((chr = arg[size-1]) == ':' || chr == '\\')
    {
        hvdefault = TRUE;
        size += sizeof(WILDCARD);
    }
    spec = getmem(size + sizeof(FS) + 2);
    spec->link = NULL;
    pnt = strmov(spec->spec, arg);
    if (hvdefault)
        strmov(pnt, WILDCARD);

    // Get the global device ID for the directory we are searching.  If
    //   have ellipsis, this is the directory just before the first ellipsis.
    //   This value is used to determine if directories are identical and
    //   can be merged for a single listing.

    if ((spec->ellip = strstr(spec->spec, "...")) != NULL)
    {
		spec->ellip[(spec->ellip == spec->spec) ? 1 : 0] = 0;
		rtn = svcIoDevParm(O_ODF, spec->spec, &glbidparm);
		spec->ellip[0] = spec->ellip[1] = '.';
    }
    else
	rtn = svcIoDevParm(O_ODF, spec->spec, &glbidparm);
    if (rtn < 0)			// Error getting global device ID?
    {
		spec->glbid[0] = 0xFFFFFFFF;	// Yes - make a fake unique ID
		spec->glbid[1] = (long)spec;
		spec->glbid[2] = spec->glbid[3] = 0;
    }
    else				// No - use the real global device ID
	memcpy(spec->glbid, glbidparm.glbid.value, 16);
    spec->devsts = glbidparm.devsts.value;
    spec->name = pnt = spec->spec;
    while ((chr = *pnt++) != 0)		// Find the start of the name part
    {
		if (chr == ':' || chr == '\\' || chr == '/')
		    spec->name = pnt;
    }
    if (firstspec == NULL)		// Is this the first file spec?
        firstspec = spec;		// Yes - put in on the list
    else				// No
    {
        spnt = firstspec;		// See if have a duplicate path
        rtn = spec->name - spec->ellip;
		do
		{
	    	if (memcmp(spec->glbid, spnt->glbid, 16) == 0)
			{
				if (spec->ellip != NULL)
				{
					if (rtn != (spnt->name - spnt->ellip) ||
						strnicmp(spec->ellip, spnt->ellip, rtn) != 0)
					continue;
				}
				break;
			}
		} while ((spnt = spnt->next) != NULL);
		if (spnt != NULL)		// Is it a duplicate?
			spnt->link = spec;		// Yes
		else
			thisspec->next = spec;
	}
	spec->next = spec->link = NULL;
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
        fputs("? DIR: Not enough memory available\n", stderr);
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
        printfiles();
    pagecheck();
    if (code != 0)
    {
        svcSysErrMsg(code, 3, buffer);	// Get error message
        fprintf(stderr, "\n? DIR: %s%s%s\n", buffer, (arg[0] != '\0')? "; ": "",
                arg);			// Output error message
    }
    else
        fprintf(stderr, "\n? DIR: %s\n", arg);
    errdone = TRUE;
}
