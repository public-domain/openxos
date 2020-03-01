//========================================================
// XUNZIP.C
// Program to extract files from, list, or test a ZIP-file
//
// Written by: John Goltz
//
//========================================================

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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <xoserr.h>
#include <xostrm.h>
#include <xosrtn.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <xoserrmsg.h>
#include <progarg.h>
#include <proghelp.h>
#include <global.h>
#include <xosstr.h>
#include <xoszipfile.h>
#include <xoswildcard.h>
#include <zlib.h>
#include "xunzip.h"

#define VERSION 0
#define EDITNO  1

FS   dfltspec = {NULL, -1, "*.*"};
FS  *dirspec;
FS  *zipspec;
FS  *firstspec;
FS  *thisspec;
FS  *firstxcld;

CDH  cdheader;
LFH  lfheader;

long cdhpos;

long crcvalue;

char *methodtbl[] =
{   "Str",					// METHOD_STORE    = 0  - No compression
    "Snk",					// METHOD_SHRUNK   = 1  - Shrunk
    "Rd1",					// METHOD_REDUCE1  = 2  - Reduced (factor 1)
    "Rd2",					// METHOD_REDUCE2  = 3  - Reduced (factor 2)
    "Rd3",					// METHOD_REDUCE3  = 4  - Reduced (factor 3)
    "Rd4",					// METHOD_REDUCE4  = 5  - Reduced (factor 4)
    "Imp",					// METHOD_IMPLODE  = 6  - Impleded
    "Tok",					// METHOD_TOKEN    = 7  - Reserved for tokenizing
							//   compression algorithm
    "Dfl",					// METHOD_DEFLATE  = 8  - Deflated
    "Edf",					// METHOD_EDEFLATE = 9  - Reserved for enhanced
							//   deflating
    "Pkd",					// METHOD_PKDATE   = 10	- PKWARE date
							//   compression library imploding
    "???"					// Unknown method
};

char monthtbl[] = "???\0Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0"
	"Oct\0Nov\0Dec\0???\0???\0???";

#define MAXMETHOD (sizeof(methodtbl)/sizeof(char *) - 1)

// Function prototypes

int   askuser(char *msg, int quit);
void  badformat(char *msg);
void  fileerror(char *msg, long code, char *name, int del);
void  finish(void);
long  makedir(char *spec, long level);
int   nonopt(char *arg);
int   optdir(arg_data *);
int   opterror(arg_data *);
void  opthelp(void);
int   optpause(arg_data *);
int   optx(arg_data *);
void  pagecheck(void);
long  ratio(long num1, long num2);
int   speccmp(FS *spec);
FS   *storespec(char *arg);

extern uint _malloc_amount;

long   maxmem;

long   zipdev;
long   filedev = -1;
long   dstbits;
long   curline;
char  *inbufr;
char  *outbufr;
long   bufrsize = 32 * 1024;

char  *namepnt;
long   pathlen;
long   pathlevel;
long   dircnt;
long   filecnt;

int    erraction = -1;

char   filename[512];
char   dstname[512];

long   totalsize;
long   totalcomsize;
long   numfiles;

char  *errtext;
char  *errname;

struct
{   byte4_parm  oldtim;
    byte4_parm  clrtim;
    byte4_parm  settim;
    char        end;
} trmparm1 =
{   {PAR_GET|REP_HEXV, 4, IOPAR_TRMSINPMODE, 0},
    {PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0x7FFFFFFF},
    {PAR_SET|REP_DECV, 4, IOPAR_TRMSINPMODE, TIM_CHAR}
};

struct
{   byte4_parm  clrtim;
    byte4_parm  settim;
    char        end;
} trmparm2 =
{   {PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0x7FFFFFFF},
    {PAR_SET|REP_DECV, 4, IOPAR_TRMSINPMODE, 0}
};

struct
{   byte4_parm  filoptn;
    lngstr_parm filspec;
    byte4_parm  length;
    char        end;
} openparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, FO_NOPREFIX|FO_DOSNAME|FO_NODENUM|
	    FO_NODENAME|FO_RVOLNAME|FO_PATHNAME|FO_FILENAME},
    {PAR_GET|REP_STR , 0, IOPAR_FILSPEC, NULL, 0, 512, 512},
    {PAR_GET|REP_DECV, 4, IOPAR_LENGTH , 0}
};

struct inparms inparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_ABSPOS, 0}
};

struct
{	time8_parm fmdt;
	time8_parm fcdt;
	uchar      end;
} dstparms =
{	{PAR_SET|REP_DT, 8, IOPAR_MDATE},
	{PAR_SET|REP_DT, 8, IOPAR_CDATE}
};

// Variables addressed directly in option table

long   doflat = FALSE;			// TRUE if restoring all files to one directory
long   doview = FALSE;			// TRUE if doing view
long   dotest = FALSE;			// TRUE if doing test
long   doover = FALSE;			// TRUE if should overwrite existing files
								//   without confirmation
long   dopause = TRUE;			// TRUE if should pause at end of page
long   quiet = FALSE;			// TRUE if should not list names


SubOpts errorcmd[] =
{   {"S*TOP"    , "Stop restoring after error"},
    {"A*SK"     , "Ask user for instructions after error"},
    {"C*ONTINUE", "Continue restoring after error"},
    {NULL}
};

arg_spec options[] =
{
    {"F*LAT"     , ASF_BOOL|ASF_STORE  , NULL    , &doflat   , TRUE,
	    "Restore all files to the specified directory"},
    {"D*IRECTORY", ASF_VALREQ|ASF_LSVAL, NULL    ,  optdir   , 0   ,
	    "Specify directory to restore to"},
    {"O*VER"     , ASF_BOOL|ASF_STORE  , NULL    , &doover   , TRUE,
	    "Overwrite existing files without confirmation"},
    {"EX*CLUDE"  , ASF_VALREQ|ASF_LSVAL, NULL    ,  optx     , TRUE,
	    "Specify files to exclude"},
    {"X"         , ASF_VALREQ|ASF_LSVAL, NULL    ,  optx     , TRUE,
	    "Same as EXCLUDE"},
    {"V*IEW"     , ASF_BOOL|ASF_STORE  , NULL    , &doview   , TRUE,
	    "Display ZIP file contents"},
    {"T*EST"     , ASF_BOOL|ASF_STORE  , NULL    , &dotest   , TRUE,
	    "Test ZIP file validity"},
    {"ERR*OR"    , ASF_XSVAL           , errorcmd,  opterror , 0   ,
	    "Specify action to take on error"},
    {"Q*UIET"    , ASF_BOOL|ASF_STORE  , NULL    , &quiet    , TRUE,
	    "Don't display names as files are extracted"},
    {"P*AUSE"    , ASF_BOOL|ASF_STORE  , NULL    , &dopause  , TRUE,
	    "Pause at end of page for /TEST and /VIEW" },
    {"H*ELP"     , 0                   , NULL    ,  opthelp  , 0   ,
	    "Display this message" },
    {"?"         , 0                   , NULL    ,  opthelp  , 0   ,
	    "Display this message"},
    {NULL}
};

Prog_Info pib;
char    prgname[] = "XUNZIP";	// Our programe name
char    envname[] = "^XOS^XUNZIP^OPT"; // The environment option name
char    example[] = "{/options} zipfile {file1 {file2 {...}}}";
char    description[] = "\nThis command extracts files from a ZIPfile or "
    "displays and/or tests the contents of a ZIPfile. Files are normally "
    "restored to the current directory. This can be overridden with the "
    "/DIRECTORY opton. Note that the default is to preserve the directory "
    "tree specified in the ZIPfile (most implementations of UNZIP default "
    "to ignoring directories specified in the ZIPfile)."
    "When specifying files to extract or exclude, if no directory is specified "
    "only the name and extension are compared. If a directory is specified "
    "the entire specification is compared. To specify files in the base "
    "directory only, use .\\filename.ext.";



void main(
    int   argc,
    char *argv[])

{
	time_x datetime;
    char  *envpnt[2];
    union
    {	char *c;
		ECD  *ecd;
    }      pnt;
    char  *name;
    long   rtn;
    long   amount;
    long   value;
	int    needperiod;
    char   chr;
    char   strbuf[256];			// String buffer
    char   ziprtnd[512];

    // Set defaults

    reg_pib(&pib);
    pib.opttbl = options; 				// Load the option table
    pib.kwdtbl = NULL;
    pib.build = __DATE__;
    pib.majedt = VERSION; 				// Major edit number
    pib.minedt = EDITNO; 				// Minor edit number
    pib.copymsg = "";
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
    if (firstspec == NULL)
		firstspec = &dfltspec;

    inbufr = getmem(bufrsize);
    dstbits = (!doover) ? (O_OUT|O_CREATE | O_TRUNCA | O_FAILEX) :
			(O_OUT | O_CREATE | O_TRUNCA);

    // Open the ZIP-file

    openparms.filspec.buffer = ziprtnd;
    if ((zipdev = svcIoOpen(O_IN|O_XWRITE, zipspec->spec, &openparms)) < 0)
		femsg2(prgname, "Error opening ZIP file", zipdev, zipspec->spec);

    // Find the central directory (which is at the end of the ZIP-file)

    if (openparms.length.value <= 1024)
    {
		inparms.pos.value = 0;
		pnt.ecd = (ECD *)(inbufr + openparms.length.value - sizeof(ECD));
		amount = openparms.length.value;
    }
    else
    {
		inparms.pos.value = openparms.length.value - 1024;
		pnt.ecd = (ECD *)(inbufr + 1024 - sizeof(ECD));
		amount = 1024;
    }
    if ((rtn = svcIoInBlockP(zipdev, inbufr, amount, &inparms)) != amount)
    {
		if (rtn >= 0)
			rtn = ER_EOF;
		femsg2(prgname, "Error reading central directory in ZIP-file", rtn,
		ziprtnd);
    }
    value = 0;
    for (;;)
    {
		if (pnt.c < inbufr)
		{
			fprintf(stderr, "? XUNZIP: Could not locate central directory in "
					"ZIPfile\n          %s\n", ziprtnd);
			exit(1);
		}
		if (pnt.ecd->commentlen == value && pnt.ecd->id == ZIPECDID)
			break;
		pnt.c--;
		value++;
    }
    printf("\nZIPfile: %s\n", ziprtnd);
    if (value != 0)
		printf("Comment: %*.*s\n", value, value, pnt.c + sizeof(ECD));
	fputs("\n", stdout);
	pagecheck();
	if (doview)
	{
		fputs("    Length   Meth      Size    %    Time      Date       Name\n",
				stdout);
		pagecheck();
    }
    cdhpos = openparms.length.value - pnt.ecd->size - sizeof(ECD);
    do
    {
		// Read the fixed part of the next central directory header record

		inparms.pos.value = cdhpos;
		if ((rtn = svcIoInBlockP(zipdev, (char *)&cdheader, sizeof(CDH),
				&inparms)) != sizeof(CDH))
		{
			if (rtn == 0 || rtn == ER_EOF || (rtn == sizeof(ECD) &&
					cdheader.id == ZIPECDID))
				finish();
			if (rtn > 0)
				rtn = ER_EOF;
			femsg2(prgname, "Error reading central directory header record",
					rtn, zipspec->spec);
		}
		if (cdheader.id != ZIPCDHID)
			badformat("Incorrect central directory header record signature "
					"value");

		// Read the variable part of the central directory header record

		amount = cdheader.namelen + cdheader.extralen + cdheader.commentlen;
		if ((rtn = svcIoInBlock(zipdev, inbufr, amount)) != amount)
		{
			if (rtn >= 0)
				rtn = ER_EOF;
			femsg2(prgname, "Error reading central directory header record",
					rtn, zipspec->spec);
		}
		cdhpos += (sizeof(CDH) + amount);
		memcpy(filename, inbufr, cdheader.namelen);
		filename[cdheader.namelen] = 0;

		// See if we want to process this file

		pnt.c = filename;				// Find start of the name part

		if (pnt.c[0] == '/' || pnt.c[0] == '\\')
			pnt.c++;
		namepnt = pnt.c;
		pathlevel = 0;

		needperiod = 1;
		while ((chr = *pnt.c++) != 0)
		{
			if (chr == '/' || chr == '\\')
			{
				pnt.c[-1] = '\\';
				namepnt = pnt.c;
				pathlevel++;
			}
			else if (chr == '.')
				needperiod = 0;
		}
		if (needperiod)
		{
			pnt.c[-1] = '.';
			pnt.c[0] = 0;
			needperiod = pnt.c - filename - 1;
		}

		pathlen = namepnt - filename;
		if (speccmp(firstspec) != 0 ||
				(firstxcld != NULL && speccmp(firstxcld) == 0))
			continue;

		// Here with a file to process

		if (needperiod)
			filename[needperiod] = 0;
		if (!(quiet | doview | dotest))
			printf("%s: %s\n", (cdheader.method == METHOD_STORE) ?
					"Unstoring" :  "Inflating", filename);
		inparms.pos.value = cdheader.lfh;
		if ((rtn = svcIoInBlockP(zipdev, (char *)&lfheader, sizeof(LFH),
				&inparms)) != sizeof(LFH))
		{
			if (rtn > 0)
				rtn = ER_EOF;
			fileerror("Error reading local file header record for", rtn,
					filename, FALSE);
			continue;
		}
		if (lfheader.id != ZIPLFHID)
			badformat("Incorrect signature in local file header");
		inparms.pos.value += (sizeof(LFH) + lfheader.namelen +
				lfheader.extralen);
		if (cdheader.method > MAXMETHOD)
			cdheader.method = MAXMETHOD;

		// Get file date and time in XOS format

		memset(&datetime, 0, sizeof(datetime));
		datetime.dos.tmx_hour = cdheader.time >> 11;
		datetime.dos.tmx_min = (cdheader.time >> 5) & 0x3F;
		datetime.dos.tmx_sec = (cdheader.time & 0x1F) * 2;
		datetime.dos.tmx_mday = cdheader.date & 0x1F;
		datetime.dos.tmx_mon = (cdheader.date >> 5) & 0x0F;
		datetime.dos.tmx_year = (cdheader.date >> 9) + 1980;

///		printf("%2d.%02d.%02d  %2d-%02d-%04d\n", datetime.dos.tmx_hour,
///				datetime.dos.tmx_min, datetime.dos.tmx_sec,
///				datetime.dos.tmx_mday, datetime.dos.tmx_mon,
///				datetime.dos.tmx_year);

		datetime.sys.date = 0;
		svcSysDateTime(T_CVD2X, &datetime);

///		printf("date: %d, time: %08.8X\n", datetime.sys.date, datetime.sys.time);


		if (doview | dotest)			// /TEST and/or /VIEW specified?
		{
			totalsize += cdheader.size;
			totalcomsize += cdheader.comsize;
			numfiles++;
			if (doview)
			{
				if (datetime.sys.date != 0)
					sdt2str(strbuf, "%H:%m:%s %D-%3n-%L",
							(time_sz *)&(datetime.sys));
				else
					strmov(strbuf, "??:??:?? ??""-??""?-??""??");
				printf("%,12u %3s%,13u %2d%% %s %s\n",
						cdheader.size, methodtbl[cdheader.method],
						cdheader.comsize, ratio(cdheader.comsize,
						cdheader.size), strbuf, filename);
				pagecheck();
			}
			if (dotest)
			{
				if (!doview)
					printf("Testing: %s ... ", filename);
				if (cdheader.method == METHOD_STORE) // Is it compressed?
					rtn = unstorefile(FALSE);
				else
					rtn = inflatefile(FALSE);
				if (rtn == 0)
				{
					if (!doview)
					{
						fputs("OK\n", stdout);
						pagecheck();
					}
				}
				else
				{
					if (rtn < 0)
					{
						char errbufr[100];

						svcSysErrMsg(rtn, 0x03, errbufr);
						if (!doview)
							printf("%s file\n    %s\n", errtext, errbufr);
						else
							printf("? %s file %s\n  %s\n", errtext, errname,
									errbufr);
					}
					else
					{
						if (!doview)
							fprintf(stdout, "Incorrect CRC value for file "
									"%08.8X %08.8X\n", cdheader.crc32,
									crcvalue);
						else
							printf("? Incorrect CRC value for file %s %08.8X "
									"%08.8X\n", errname, cdheader.crc32,
									crcvalue);
					}
					pagecheck();
				}
			}
		}
		else							// If extracting files
		{
			if (cdheader.method != METHOD_STORE &&
					cdheader.method != METHOD_DEFLATE)
			{
				printf(inbufr, "? Unsupported compression method (%s) for file "
						"%s\n  Skipping file\n", methodtbl[cdheader.method],
						filename);
				continue;
			}
			dstparms.fcdt.value = dstparms.fmdt.value = datetime.sys;
			pnt.c = (dirspec == NULL) ? dstname : strmov(dstname,
					dirspec->spec);
			if (doflat)
			{
				name = namepnt;
				rtn = cdheader.namelen - pathlen;
			}
			else
			{
				name = filename;
				rtn = cdheader.namelen;
			}
			if (((pnt.c - dstname) + rtn) >= 512)
			{
				fileerror("Expanded file specification is too long", 0, name,
						FALSE);
				continue;
			}
			strmov(pnt.c, name);
			if ((filedev = svcIoOpen(dstbits, dstname, &dstparms)) < 0)
			{
				if (!doflat && filedev == ER_DIRNF)
				{
					if ((rtn = makedir(dstname, pathlevel)) < 0)
					{
						fileerror("Error creating directory for", rtn, dstname,
								FALSE);
						continue;
					}
				}
				filedev = svcIoOpen(dstbits, dstname, &dstparms);
		    }
		    if (!doover && filedev == ER_FILEX)
		    {
				sprintf(inbufr, "Overwrite %s", dstname);
				switch (askuser(inbufr, TRUE))
				{
				 case 0:				// No
					continue;
	
				 case 2:				// All
					doover = TRUE;
					dstbits &= ~O_FAILEX;
				 case 1:				// Yes
					filedev = svcIoOpen(dstbits & ~O_FAILEX, dstname, 
							&dstparms);
					break;
				}
			}
			if (filedev < 0)
			{
				fileerror("Error creating", filedev, dstname, FALSE);
				continue;
			}
			if (cdheader.method == METHOD_STORE) // Is it compressed?
				rtn = unstorefile(TRUE);
			else
				rtn = inflatefile(TRUE);

/*			if (rtn > 0)
			{
				fileerror("Incorrect CRC value for", 0, filename, TRUE);
				continue;
			}
			else */ if (rtn < 0)
			{
				fileerror(errtext, rtn, errname, TRUE);
				continue;
			}
			if ((rtn = svcIoClose(filedev, 0)) < 0) // Almost done, close the
			{										//   file
				fileerror("Error writing", rtn, filename, TRUE);
				continue;
			}
			filedev = -1;
			filecnt++;					// All is OK, count the file
		}
    } while (openparms.length.value > 0);

    exit(0);							// Return with no error
}

//*************************************************
// Function: ratio - Calculate ratio of two numbers
// Returned: The ratio (%, never more than 99)
//*************************************************

long ratio(
    long num1,
    long num2)

{
    long temp;
    long value[2];

    if ((temp = num2 - num1) <= 0)
		return (0);
    longadddiv(&value, temp, 100, num2/2, num2);
    if (value[0] > 99)
		value[0] = 99;
    return (value[0]);
}

//************************************************
// Function: specdmp - Compare file specifications
// Returned: 0 if match, non-zero if no match
//************************************************

int speccmp(
    FS *spec)

{
	do
	{
		if (spec->pathlen < 0)
		{
			if (wildcmp(spec->spec, namepnt, 0) == 0)
				return (0);
		}
		else
		{
			if (spec->pathlen != pathlen || (pathlen != 0 &&
					strnicmp(spec->spec, filename, pathlen) != 0))
				continue;
			if (wildcmp(spec->spec + pathlen, namepnt, 0) == 0)
				return (0);
		}
	} while ((spec = spec->next) != NULL);
	return (1);
}

//***************************************************
// Function: pagecheck - Check the page count and pause if necessary
// Returned: Nothing
//***************************************************

void pagecheck(void)

{
	int chr;
	int more;

    if (dopause && (++curline >= pib.screen_height - 2))
    {
		svcIoInBlockP(DH_STDIN, NULL, 0, &trmparm1);
		fputs("\33[7m-MORE- ^C, G, H, Q, <Enter> or <Space>\33[0m", stderr);
		svcTrmFunction(DH_STDIN, TF_CLRINP);
		do
		{
			chr = svcIoInSingle(DH_STDTRM);
			more = FALSE;		// Assume good input
			switch (toupper(chr & 0x7F))
			{
			 case 'Q':			// Quit!, don't print the rest
			 case 0x03:
				fputs("\r\33[K", stdout);
				exit(0);

			 case 'G':			// Don't ask for any more
				dopause = FALSE;
				break;

			 case '\r':
			 case ' ':			// Do another screen full
				curline = 0;
				break;

			 default:			// Tell him this is wrong
				fputs("\r\33[K\33[7m"
						" ^C      - Exit                    \n"
						"  G      - Go, don't ask for -MORE-\n"
						"  H or ? - Help, this message      \n"
						"  Q      - Quit program            \n"
						" <Enter> - Next screen             \n"
						" <Space> - Next screen             \n"
						"-MORE- ^C, G, H, Q, <Enter> or <Space>\33[0m",
						stderr);

			  case 0:
				more = TRUE;		// Loop back and check again
				break;
			}
		} while (more);
		trmparm2.settim.value = trmparm1.oldtim.value;
		svcIoInBlockP(DH_STDIN, NULL, 0, &trmparm2);
		svcTrmFunction(DH_STDIN, TF_CLRINP);
		fputs("\r\33[K", stderr);
	}
}

//********************************************
// Function: optdir - Process DIRECTORY option
// Returned: Nothing
//********************************************

int optdir(
    arg_data *arg)

{
    char *pnt;
    int   size;
    uchar needfs;
    char  chr;

    if (dirspec != NULL)
    {
		fputs("? XUNZIP: More than one restore directory specified\n", stderr);
		exit (1);
    }
    size = arg->length;
    needfs = FALSE;
    if ((chr = arg->val.s[size]) != ':' || chr != '\\' && chr != '/')
    {
		needfs = TRUE;
		size++;
    }
    dirspec = getmem(size + sizeof(FS));
    pnt = strmov(dirspec->spec, arg->val.s);
    if (needfs)
        strmov(pnt, "\\");
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

//**********************************
// Function: optx - Process X option
// Returned: Nothing
//**********************************

int optx(
    arg_data *arg)

{
    FS *spec;

    spec = storespec(arg->val.s);
    spec->next = firstxcld;
    firstxcld = spec;
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
    char *pnt;
    int   size;
    char  chr;
    uchar hvext;

    if (zipspec == NULL)
    {
		hvext = FALSE;
		pnt = arg;
		while ((chr = *pnt++) != 0)
		{
			if (chr == ':' || chr == '\\' || chr == '/')
				hvext = FALSE;
			else if (chr == '.')
				hvext = TRUE;
		}
		size = pnt - arg;
		if (hvext)
			size += 4;
		zipspec = getmem(size + sizeof(FS));
		pnt = strmov(zipspec->spec, arg);
		if (!hvext)
			strmov(pnt, ".zip");
		zipspec->pathlen = 0;
		return (TRUE);
	}
	spec = storespec(arg);
	if (firstspec == NULL)				// Is this the first file spec?
		firstspec = spec;				// Yes - put in on the list
    else								// No
	thisspec->next = spec;
    spec->next = NULL;
    thisspec = spec;
    return (TRUE);
}

//***********************************************
// Function: storespec - Store file specification
// Returend: Pointer to FS structure created
//***********************************************

FS *storespec(
    char *arg)

{
    FS   *spec;
    char *pnt;
    int   size;
    int   pathlen;
    uchar hvext;
    char  chr;

    pathlen = 0;
    hvext = FALSE;
    pnt = arg;
    while ((chr = *pnt++) != 0)
    {
		if (chr == ':')
		{
			fputs("? XUNZIP: Cannot specify device with file to extract",
					stderr);
			exit(1);
		}
		if (chr == '\\' || chr == '/')
		{
	    	pnt[-1] = '\\';
	    	pathlen = pnt - arg;
			hvext = FALSE;
		}
		else if (chr == '.')
			hvext = TRUE;
	}
   	size = pnt - arg;
	if (!hvext)
		size += 2;
	if (pathlen == 0)
		pathlen = -1;
	else if (pathlen == 2 && arg[0] == '.')
	{
		pathlen = 0;
		arg += 2;
		size -= 2;
    }
    spec = getmem(size + sizeof(FS));
    pnt = strmov(spec->spec, arg);
    if (!hvext)
        strmov(pnt, ".*");
    spec->pathlen = pathlen;
    return (spec);
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

    if (--level < 0)
		return (ER_DIRNF);

    dirparms.filspec.buffer = bufr;
    dirparms.filoptn.value = (gcp_dosdrive) ?
			FO_NOPREFIX|FO_DOSNAME|FO_NODENUM|FO_NODENAME|FO_RVOLNAME|
				FO_PATHNAME|FO_FILENAME :
			FO_NOPREFIX|FO_XOSNAME|FO_NODENUM|FO_NODENAME|FO_RVOLNAME|
				FO_PATHNAME|FO_FILENAME;
	if ((rtn = svcIoDevParm(O_CREATE|O_ODF, spec, &dirparms)) < 0)
	{
		if (rtn == ER_DIRNF)
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

//****************************************
// Function: askuser - Ask user what to do
// Returned: 0 if No, 1 if Yes, 2 if All
//****************************************

int askuser(
    char *msg,			// Message to display
    int   quit)			// TRUE if should include "Quit" option

{
    long rtn;
    char bufr[] = {0, '\b', 0};

    svcIoInBlockP(DH_STDIN, NULL, 0, &trmparm1);
    fprintf(stderr, "%s (Yes/No/All%s) ? ", msg, (quit) ? "/Quit" : "");
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
        fputs("? XUNZIP: Not enough memory available\n", stderr);
        exit(EXIT_MALLOC);
    }
    if (maxmem < _malloc_amount)
        maxmem = _malloc_amount;
    return (ptr);
}

//*************************************************
// Function: finish - Finish up processing and exit
// Returned: Never returns
//*************************************************

void finish(void)

{
	if (doview)
		printf("------------     ------------ ---                      ------\n"
				"%,12u    %,13u %2d%%                    %,8d\n", totalsize,
				totalcomsize, ratio(totalcomsize, totalsize), numfiles);
	else if (!dotest)
	{
		if (filecnt == 0)
			fputs("\tNo files restored", stdout);
		else
			printf("\t%d file%s restored", filecnt, (filecnt == 1) ? "" : "s");
		if (dircnt == 0)
			fputs("\n", stdout);
		else
			printf(", %d director%s created\n", dircnt, (dircnt == 1) ? "y" :
					"ies");
	}
	exit (0);
}

//*******************************************************
// Function: badformat - Report bad format in the ZIPfile
// Returned: Never returns
//*******************************************************

void badformat(
    char *msg)

{
    msg = msg;

    exit (1);
}

//**********************************************************
// Function: fileerror - Report error associated with a file
// Returned: Nothing (only returns if should continue
//**********************************************************

void fileerror(
    char *msg,
    long  code,
    char *name,
    int   del)

{
    int  action;
    long rtn;

	if (filedev > 0)
	{
		svcIoClose(filedev, 0);
		filedev = -1;
	}
	fprintf(stderr, "? XUNZIP: %s file %s\n", msg, name);
	if (code != 0)
	{
		char bufr[100];

		svcSysErrMsg(code, 0x03, bufr);
		fprintf(stderr, "          %s\n", bufr);
	}

	// Determine if we should see about continueing

	action = 0;
	if (erraction != 0 && (action = (erraction < 0) ?
			askuser("Continue restoring", FALSE) : erraction) == 2)
	erraction = 1;
	if (del)
	{
		if ((rtn = svcIoDelete(0, dstname, NULL)) < 0) // Delete the file
		{								// If error
			char *msg[100];

			svcSysErrMsg(rtn, 0x03, msg); // Tell him about it
			fprintf(stderr, "? XUNZIP: Error deleting partial output file\n"
					"          %s; %s\n", msg, dstname);
		}
	}
	if (action == 0)
		finish();
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

	pagecheck();
	if (code != 0)
	{
		svcSysErrMsg(code, 3, buffer);	// Get error message
		fprintf(stderr, "\n? XUNZIP: %s%s%s\n", buffer, (arg[0] != '\0') ?
				"; ": "", arg);		// Output error message
	}
	else
		fprintf(stderr, "\n? XUNZIP: %s\n", arg);
}
