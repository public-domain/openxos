//=============================
// XZIP.C
// Program to create a ZIP-file
//
// Written by: John Goltz
//
//=============================

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
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <xoserr.h>
#include <xostrm.h>
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
#include <dirscan.h>
#include <zlib.h>
#include "xzip.h"

#define VERSION 0
#define EDITNO  1

char  dirspec[600];
int   dirspeclen;
char *zipspec;
PS   *firstpath = NULL;
PS   *pathpnt;
PS  **prevpspnt = &firstpath;
FS   *firstexclude;
FS  **prevexclude = &firstexclude;
FI   *firstfi;
FI   *fipnt;
FI  **prevfipnt = &firstfi;

char *filebufr;
char *outbufr;
long  zfhndl;
long  ifhndl;
long  level = 8;

// Function prototypes

static int  askuser(void);
static int  comp(FI *one, FI *two);
static void errormsg(const char *arg, long  code);
static void getfiles(char *pnt, FS **prevfilepnt, FS **prevexcludepnt);
static long makedir(char *spec, long level);
static int  nonopt(char *arg);
static int  optdir(arg_data *);
static int  opterror(arg_data *arg);
static int  optexclude(arg_data *);
static int  optlevel(arg_data *);
static int  procfile(DIRSCANDATA *dsd);
static PS  *storefileset(char *arg);

// Stuff needed by dirscan

struct
{   byte4_parm  filoptn;
    lngstr_parm filspec;
    byte2_parm  srcattr;
    byte2_parm  filattr;
    byte4_parm  dirhndl;
    byte4_parm  length;
    byte8_parm  cdate;
    char        end;
} fileparm =
{   {PAR_SET|REP_HEXV, 4 , IOPAR_FILOPTN , FO_VOLNAME|FO_NODENUM|FO_NODENAME|
            FO_RVOLNAME|FO_PATHNAME|FO_FILENAME|FO_ATTR},
    {PAR_GET|REP_STR , 0 , IOPAR_FILSPEC , NULL, 0, 0, 0},
    {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR , A_NORMAL},
    {PAR_GET|REP_HEXV, 2 , IOPAR_FILATTR , 0},
    {PAR_SET|REP_HEXV, 4 , IOPAR_DIRHNDL , 0},
    {PAR_GET|REP_DECV, 4 , IOPAR_LENGTH  , 0},
    {PAR_GET|REP_HEXV, 8 , IOPAR_CDATE   , 0},
};

DIRSCANDATA dsd =
{   (DIRSCANPL *)&fileparm,
						// parmlist    - Address of parameter list
    FALSE,				// repeat      - TRUE if should do repeated search
    TRUE,				// dfltwildext - TRUE if want wildcard as default
    QFNC_DEVPARM,		// function    - Value for low byte of QAB_FUNC
    DSSORT_NONE,		// sort        - Directory sort order
    NULL,               // hvellip     - Function called when ellipsis specified
    procfile,			// func        - Function called for each file matched
    errormsg			// error       - Function called on error
};

FS allfiles = {NULL, TRUE, "*.*"};
PS nullpath = {NULL, &allfiles, NULL, FALSE, ""};

int    erraction = -1;
long   totalsize = 0;
long   totalcomsize = 0;
long   numfiles = 0;
long   maxsize;

struct
{	CDH  h;
	char name[600];
} cdh = {ZIPCDHID, 20, 20};

struct
{	ECD  h;
	char comment[200];
} ecd = {ZIPECDID};

struct
{   byte4_parm  oldtim;
    byte4_parm  clrtim;
    byte4_parm  settim;
    uchar       end;
} trmparm1 =
{   {PAR_GET|REP_HEXV, 4, IOPAR_TRMSINPMODE, 0},
    {PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0x7FFFFFFF},
    {PAR_SET|REP_DECV, 4, IOPAR_TRMSINPMODE, TIM_CHAR}
};

struct
{   byte4_parm  clrtim;
    byte4_parm  settim;
    uchar       end;
} trmparm2 =
{   {PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0x7FFFFFFF},
    {PAR_SET|REP_DECV, 4, IOPAR_TRMSINPMODE, 0}
};

struct
{   byte4_parm  datetime;
    byte4_parm  length;
	byte16_parm glbid;
    uchar       end;
} ifopnparms =
{   {PAR_GET|REP_HEXV, 4, IOPAR_CDATE},
    {PAR_GET|REP_DECV, 4, IOPAR_LENGTH},
	{PAR_GET|REP_HEXV,16 , IOPAR_GLBID, 0}	
};

ZFOUTPARMS zfoutparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_ABSPOS}
};

struct
{	byte4_parm len;
	uchar      end;
} zftrunparms =
{	{PAR_SET|REP_HEXV, 4, IOPAR_LENGTH}
};

struct
{	byte4_parm  opts;
	lngstr_parm spec;
	uchar       end;
} dpparms =
{	{PAR_SET|REP_HEXV, 4 , IOPAR_FILOPTN , FO_NOPREFIX|FO_VOLNAME|FO_NODENUM|
			FO_NODENAME|FO_RVOLNAME|FO_PATHNAME|FO_FILENAME|FO_ATTR},
	{PAR_GET|REP_STR , 0 , IOPAR_FILSPEC, dirspec, NULL, sizeof(dirspec),
			sizeof(dirspec)}
};

struct
{	byte16_parm gblid;
	uchar       end;
} zfopnparms =
{	{PAR_GET|REP_HEXV, 16, IOPAR_GLBID}
};

// Variables addressed directly in option table

long   dopath = 2;				// 0 = FLAT, 1 = RELATIVE, 2 = ABSOLUTE
long   quiet = FALSE;			// TRUE if should not list names


SubOpts errorcmd[] =
{   {"S*TOP"    , "Stop inserting after error"},
    {"A*SK"     , "Ask user for instructions after error"},
    {"C*ONTINUE", "Continue inserting after error"},
    {NULL}
};

arg_spec options[] =
{	{"ABS*OLUTE" , ASF_BOOL|ASF_STORE  , NULL    , &dopath    , 2,
			"Include the absolute path for each file in the ZIPfile"},

	{"REL*ATIVE" , ASF_BOOL|ASF_STORE  , NULL    , &dopath    , 1,
			"Include the relative path for each file in the ZIPfile"},

	{"FLA*T"     , ASF_BOOL|ASF_STORE  , NULL    , &dopath    , 0,
			"Do not Include path information in the ZIPfile"},

	{"D*IRECTORY", ASF_VALREQ|ASF_LSVAL, NULL    ,  optdir    , 0,
			"Specify base directory to take files from"},

	{"EXC*LUDE"  , ASF_VALREQ|ASF_LSVAL, NULL    ,  optexclude, TRUE,
			"Specify files to exclude"},

	{"X"         , ASF_VALREQ|ASF_LSVAL, NULL    ,  optexclude, TRUE,
			"Same as EXCLUDE"},

	{"LEV*EL"    , ASF_VALREQ|ASF_NVAL , NULL    ,  optlevel  , 0,
			"Compression level (0 = no compression, 1 = least compression, "
			"fastest; 9 = most compression, slowest)"},
	{"ERR*OR"    , ASF_XSVAL           , errorcmd,  opterror  , 0,
			"Specify action to take on error"},

	{"Q*UIET"    , ASF_BOOL|ASF_STORE  , NULL    , &quiet     , TRUE,
			"Don't display names as files are inserted"},

	{"H*ELP"     , 0                   , NULL    ,  opthelp   , 0,
			"Display this message" },

	{"?"         , 0                   , NULL    ,  opthelp   , 0,
			"Display this message"},
	{NULL}
};

Prog_Info pib;
char prgname[] = "XZIP";				// Our programe name
char envname[] = "^XOS^XZIP^OPT";		// The environment option name
char example[] = "{/options} zipfile {fileset1 {fileset2 {...}}}";
char description[] = "\nThis command create a ZIPfile. Ellipsis may be "
		"used to indicate directory recusion. Multiple files may be "
		"specified in a directory by seperating them by commas without "
		"any spaces. Multiple directory specifications are seperated "
		"with spaces. Excluded files for a single directory specification "
		"are specified by preceeding the file name with a \"!\" character. "
		"A file name to be excluded from all directores specified is "
		"specified using the /EXCLUDE option";

void main(
    int   argc,
    char *argv[])

{
	char *fnp;
	char *pnt;
    char *envpnt[2];
	long  rtn;
	long  offset;
	long  savepos;
	int   dlen;
    char  strbuf[256];			// String buffer
	char  bufr[600];

	filebufr = getmem(FILEBUFR);
	outbufr = getmem(OUTBUFR);

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

	if (zipspec == NULL)
	{
		fputs("? XZIP: No zip file specified\n", stderr);
		exit(1);
	}
	if ((pathpnt = firstpath) == 0)
		firstpath = &nullpath;
	else if (dopath == 1)
	{
		do
		{
			if (pathpnt->notrel)
			{
				fputs("? XZIP: All paths must be relative when /RELATIVE "
						"specified\n", stderr);
				exit(1);
			}
		} while ((pathpnt = pathpnt->next) != NULL);
	}

	if (dirspec[0] != 0)
	{
		dpparms.spec.strlen = dirspeclen;
		if ((rtn = svcIoDevParm(O_ODF, dirspec, &dpparms)) < 0)
			femsg2(prgname, "Error opening base directory", rtn, NULL);
		dirspeclen = dpparms.spec.strlen;
	}
	else
	{
		if ((dirspeclen = svcIoPath("Z:", FO_NOPREFIX|FO_VOLNAME|FO_NODENUM|
				FO_NODENAME|FO_RVOLNAME|FO_PATHNAME|FO_FILENAME, dirspec,
				sizeof(dirspec) - 1)) < 0)
			femsg2(prgname, "Error obtaining default directory path",
					dirspeclen, NULL);
	}
	printf("Creating ZIPfile: %s\n", zipspec);
	pathpnt = firstpath;
	do
	{
		fnp = (pathpnt->file->next == NULL) ? pathpnt->file->name : "*.*";
		dlen = (pathpnt->notrel) ? 0 : dirspeclen;
		if ((dlen + strlen(pathpnt->path) + strlen(fnp)) > 1023)
		{
			fputs("? XZIP: Path is too long\n", stderr);
			exit(1);
		}
		if (dlen > 0)
			pnt = strmov(bufr, dirspec);
		else
			pnt = bufr;
		strmov(strmov(pnt, pathpnt->path), fnp);
		dirscan(bufr, &dsd);
	} while ((pathpnt = pathpnt->next) != NULL);

	if (firstfi == NULL)
	{
		fputs("? XZIP: No matching files were found\n", stderr);
		exit(1);
	}

	// Create the ZIPfile

	if ((rtn = svcIoOpen(O_OUT|O_CREATE|O_TRUNCA, zipspec, NULL)) < 0 ||
			(zfhndl = rtn, (rtn = svcIoOutBlock(zfhndl, "", 1)) < 0) ||
			(rtn = svcIoInBlockP(zfhndl, NULL, 0, &zfopnparms)) < 0)

		femsg2(prgname, "Error creating ZIPfile", rtn, NULL);

	// Sort the list of files

	firstfi = heapsort(firstfi, (int (*)(void *a, void *b, void *d))comp, NULL);

	// Add each file to the ZIPfile

	fipnt = firstfi;
	do
	{
		if ((ifhndl = svcIoOpen(O_IN, fipnt->spec, &ifopnparms)) < 0)
		{

			fileerror("opening", ifhndl);
			fipnt->spec[0] = 0;			// Indicate file not there
			continue;
		}
		if (memcmp(ifopnparms.glbid.value, zfopnparms.gblid.value, 16) == 0)
		{
			fipnt->spec[0] = 0;
			continue;					// If this is our ZIPfile!
		}
		fipnt->lfh.id = ZIPLFHID;
		fipnt->lfh.extract = 20;
		fipnt->offset = zfoutparms.pos.value;
		fipnt->lfh.namelen -= fipnt->skip;
		zfoutparms.pos.value += (sizeof(LFH) + fipnt->lfh.namelen);
		*(long *)&(fipnt->lfh.time) = ifopnparms.datetime.value;
		memcpy(fipnt->spec, fipnt->spec + fipnt->skip, fipnt->lfh.namelen);

		// File is open, deflate or store it

		offset = zfoutparms.pos.value;
		if (level == 0)
		{
			if (!storefile())
			{
				maxsize = zfoutparms.pos.value;
				zfoutparms.pos.value = offset;
				continue;
			}
		}
		else
		{
			if (!deflatefile())			// Deflate the file
			{
				maxsize = zfoutparms.pos.value;
				zfoutparms.pos.value = offset;
				continue;
			}
			if (fipnt->lfh.size <= fipnt->lfh.comsize)
			{							// Did it get bigger?
				fputc('\r', stdout);	// Yes - junk this and just store it
				maxsize = zfoutparms.pos.value;
				zfoutparms.pos.value = offset;
				if ((rtn = svcIoSetPos(ifhndl, 0, 0)) < 0)
				{
					fileerror("rewinding", rtn);
					continue;
				}
				if (!storefile())
				{
					maxsize = zfoutparms.pos.value;
					zfoutparms.pos.value = offset;
					continue;
				}
			}
		}
		printf("%d%%\n", ratio(fipnt->lfh.comsize, fipnt->lfh.size));
		numfiles++;
		totalsize += fipnt->lfh.size;
		totalcomsize += fipnt->lfh.comsize;
		svcIoClose(ifhndl, 0);
		savepos = zfoutparms.pos.value;
		zfoutparms.pos.value = fipnt->offset;
		if ((rtn = svcIoOutBlockP(zfhndl, (char *)&(fipnt->lfh),
				sizeof(LFH) + fipnt->lfh.namelen, &zfoutparms)) < 0)
										// Write out the file header
			femsg2(prgname, "Error writing ZIPfile file header", rtn, NULL);
		zfoutparms.pos.value = savepos;
	} while ((fipnt = fipnt->next) != NULL);

	// All files have been output, now create the central directory

	ecd.h.start = zfoutparms.pos.value;
	cdh.h.extralen = cdh.h.commentlen = 0;
	fipnt = firstfi;
	do
	{
		if (fipnt->spec[0] == 0)
			continue;
		cdh.h.flag = fipnt->lfh.flag;
		cdh.h.method = fipnt->lfh.method;
		cdh.h.time = fipnt->lfh.time;
		cdh.h.date = fipnt->lfh.date;
		cdh.h.crc32 = fipnt->lfh.crc32;
		cdh.h.comsize = fipnt->lfh.comsize;
		cdh.h.size = fipnt->lfh.size;
		cdh.h.namelen = fipnt->lfh.namelen;
		cdh.h.lfh = fipnt->offset;
		memcpy(cdh.name, fipnt->spec, fipnt->lfh.namelen);
		if ((rtn = svcIoOutBlockP(zfhndl, (char *)&cdh, sizeof(CDH) +
				fipnt->lfh.namelen, &zfoutparms)) < 0)
										// Write out the central directory
										//   record
			femsg2(prgname, "Error writing ZIPfile central directory record",
					rtn, NULL);
		zfoutparms.pos.value += (sizeof(CDH) + fipnt->lfh.namelen);
		ecd.h.enthere++;
	} while ((fipnt = fipnt->next) != NULL);

	// Create the "end of central directory" record

	ecd.h.enttotal = ecd.h.enthere;
	ecd.h.size = zfoutparms.pos.value - ecd.h.start;

	if ((rtn = svcIoOutBlockP(zfhndl, (char *)&ecd, sizeof(ECD) +
			ecd.h.commentlen, &zfoutparms)) < 0)
		femsg2(prgname, "Error writing end of central directory record", rtn,
				NULL);
	zfoutparms.pos.value += (sizeof(ECD) + ecd.h.commentlen);

	// Finished writing the ZIPfile - now truncate it if need to (This is
	//   unlikely but may be necessary if we stored a large file near the
	//   end of the ZIPfile after deflate failed to compress it or if we
	//   continued after an input error near the end of the ZIPfile.)

	if (maxsize > zfoutparms.pos.value)
	{
		zftrunparms.len.value = zfoutparms.pos.value;
		if ((rtn = svcIoInBlockP(zfhndl, NULL, 0, &zftrunparms)) < 0)
			femsg2(prgname, "Error while truncating ZIPfile", rtn, NULL);
	}

	// Finally, close the ZIPfile

	if ((rtn = svcIoClose(zfhndl, 0)) < 0)
		femsg2(prgname, "Error closing ZIPfile", rtn, NULL);

	// All finished, give him a short summary of what we did

	printf("%,d file%s (%,d byte%s) stored (%,d byte%s uncompressed - %d%%)\n",
			numfiles, (numfiles == 1) ? "" : "s", totalcomsize,
			(totalcomsize == 1) ? "" : "s", totalsize, (totalsize == 1) ?
			"" : "s", ratio(totalcomsize, totalsize));
	exit(0);
}

//***************************************************************
// Function: procfile - Process matching file (called by dirscan)
// Returned: TRUE if should continue, FALSE if should stop
//***************************************************************

// This function is called by dirscan for each matching file.  If there is
//   only one file specified in a file-set, the search is for that file
//   specification only. If more than one file is specified, the search is
//   for all files and we must loop thougth all of the file specifications
//   looking for a match.

int procfile(
	DIRSCANDATA *dsdarg)

{
	char *pnt;
	FS   *fpnt;
	int   namelen;
	char  chr;

	dsdarg = dsdarg;

	if (dsd.filename[0] == 0)
		return (TRUE);
	fpnt = pathpnt->file;				// First see if this matches any
	if (fpnt->next != NULL)				//   desired files
	{
		do
		{
			if (wildcmp(fpnt->name, dsd.filename, 0) == 0)
				break;
		} while ((fpnt = fpnt->next) != NULL);
		if (fpnt == NULL)
			return (TRUE);
	}
	fpnt = firstexclude;				// Now see if this matches any
	while (fpnt != NULL)				//   globally excluded files
	{
		if (wildcmp(fpnt->name, dsd.filename, 0) == 0)
			return (TRUE);
		fpnt = fpnt->next;
	}
	fpnt = pathpnt->exclude;			// Now see if this matches any
	while (fpnt != NULL)				//   locally excluded files
	{
		if (wildcmp(fpnt->name, dsd.filename, 0) == 0)
			return (TRUE);
		fpnt = fpnt->next;
	}

	// Where with a file we want to include in the ZIPfile

	pnt = dsd.pathname;
	while ((chr = *pnt) != 0)
	{
		if (chr == '\\')
			*pnt = '/';
		pnt++;
	}
	namelen = dsd.devnamelen + dsd.pathnamelen + dsd.filenamelen;
	fipnt = getmem(namelen + 1 + offsetof(FI, spec));
	fipnt->lfh.size = fileparm.length.value;
	fipnt->lfh.namelen = namelen;
	pnt = strmov(fipnt->spec, dsd.devname);

	if (dopath == 1)					// Want relative path?
		fipnt->skip = dirspeclen;		// Yes
	else
	{
		fipnt->skip = pnt - fipnt->spec + 1;
		if (dopath == 0)				// Want to completely discard paths?
			fipnt->skip += (dsd.pathnamelen - 1); // Yes
	}
	strmov(strmov(pnt, dsd.pathname), dsd.filename);
	fipnt->next = NULL;
	*prevfipnt = fipnt;
	prevfipnt = &(fipnt->next);
	return (TRUE);
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


//******************************************
// Function: optlevel - Process LEVEL option
// Returned: Nothing
//******************************************

int optlevel(
    arg_data *arg)

{
	if ((level = arg->val.n) < 0 || level > 9)
	{
		fputs("? XZIP: Invalid compression level, must be between 1 and 9\n",
				stderr);
		exit(1);
	}
	return (TRUE);
}


//********************************************
// Function: optdir - Process DIRECTORY option
// Returned: Nothing
//********************************************

int optdir(
    arg_data *arg)

{
    char *pnt;
    uchar needfs;
    char  chr;

    if (dirspec[0] != 0)
    {
		fputs("? XZIP: More than one base directory specified\n", stderr);
		exit (1);
    }
    if ((dirspeclen = arg->length) > 590)
	{
		fputs("? XZIP: Base directory specification is too long\n", stderr);
		exit(1);
	}
    needfs = FALSE;
    if ((chr = arg->val.s[dirspeclen - 1]) != ':' || chr != '\\' && chr != '/')
    {
		needfs = TRUE;
		dirspeclen++;
    }

    pnt = strmov(dirspec, arg->val.s);
    if (needfs)
        strmov(pnt, "\\");
    return (TRUE);
}

//******************************************
// Function: opterror - Process ERROR option
// Returned: TRUE
//******************************************

static int opterror(
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

//****************************************************
// Function: optexclude - Process EXCLUDE or X options
// Returned: Nothing
//****************************************************

static int optexclude(
    arg_data *arg)

{
	getfiles(arg->val.s, prevexclude, NULL);
    return (TRUE);
}

//********************************************
// Function: nonopt - process non-option input
// Returned: Nothing
//********************************************

static int nonopt(
    char *arg)

{
    PS   *spec;
    char *pnt;
    int   size;
    char  chr;
    uchar needext;

    if (zipspec == NULL)
    {
		needext = TRUE;
		pnt = arg;
		while ((chr = *pnt++) != 0)
		{
			if (chr == ':' || chr == '\\' || chr == '/')
				needext = TRUE;
			else if (chr == '.')
				needext = FALSE;
		}
		size = pnt - arg;
		if (needext)
			size += 4;
		zipspec = getmem(size + 1);
		pnt = strmov(zipspec, arg);
		if (needext)
			strmov(pnt, ".zip");
	}
	else
	{
		spec = storefileset(arg);
		*prevpspnt = spec;
		prevpspnt = &(spec->next);
	}
    return (TRUE);
}

//*****************************************************
// Function: storefileset - Store fileset specification
// Returend: Pointer to FS structure created
//*****************************************************

static PS *storefileset(
    char *arg)

{
    PS   *path;
    char *pnt;
	char *pend;
    int   size;
    char  chr;
	uchar notrel;

    pnt = arg;

	// Find the end of the path

	pend = pnt;
	notrel = FALSE;
    while ((chr = *pnt++) != 0)
    {
		if (chr == ':' || chr == '\\')
		{
			if (chr == ':' || pnt == (arg + 1))
				notrel = TRUE;
			pend = pnt;
		}
	}	size = pend - arg;
	path = (PS *)getmem(size + 1 + offsetof(PS, path));
	path->next = NULL;
	path->notrel = notrel;
	memcpy(path->path, arg, size);
	path->path[size] = 0;
	path->file = path->exclude = NULL;
	pnt = pend;
	getfiles(pend, &(path->file), &(path->exclude));
										// Process the file names
	if (path->file == NULL)				// Did we have any files?
		path->file = &allfiles;			// No - use *.*
	return (path);
}


static void getfiles(
	char *pnt,
	FS  **prevfilepnt,
	FS  **prevexcludepnt)

{
	FS   *file;
	char *fpnt;
	int   size;
	char  chr;
	uchar needext;
	uchar exclude;
	uchar wild;

	while ((chr = *pnt) != 0)
	{
		if (chr == '!')
		{
			if (prevexcludepnt == NULL)
			{
				fputs("? XZIP: Illegal initial character (!) in file name\n",
						stderr);
				exit(1);
			}
			pnt++;
			exclude = TRUE;
		}
		else
			exclude = FALSE;
		fpnt = pnt;
		needext = TRUE;
		wild = FALSE;
		while ((chr = *pnt) != 0 && chr != ',')
		{
			pnt++;
			if (chr == '*' || chr == '?')
				wild = TRUE;
			if (chr == '.')
				needext = FALSE;
		}
		if ((size = pnt - fpnt) == 0)
		{
			fputs("? XZIP: File name cannot be null\n", stderr);
			exit(1);
		}
		file = (FS *)getmem(size + ((needext) ? 3 : 1) + offsetof(FS, name));
		file->next = NULL;
		file->wild = wild;
		memcpy(file->name, fpnt, size);
		if (needext)
		{
			file->name[size] = '.';
			file->name[size + 1] = '*';
			file->name[size + 2] = 0;
		}
		else
			file->name[size] = 0;

		if (exclude)
		{
			*prevexcludepnt = file;
			prevexcludepnt = &(file->next);
		}
		else
		{
			*prevfilepnt = file;
			prevfilepnt = &(file->next);
		}
		if (chr != 0)
			pnt++;
	}
}


//****************************************
// Function: askuser - Ask user what to do
// Returned: 0 if No, 1 if Yes, 2 if All
//****************************************

static int askuser(void)

{
    long rtn;
    char bufr[] = {0, '\b', 0};

    svcIoInBlockP(DH_STDIN, NULL, 0, &trmparm1);
    fputs("Continue creating ZIPfile (Yes/No/All) ? ", stderr);
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
		exit(0);
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
        fputs("? XZIP: Not enough memory available\n", stderr);
        exit(1);
    }
    return (ptr);
}


//**********************************************************
// Function: fileerror - Report error associated with a file
// Returned: Nothing (only returns if should continue
//**********************************************************

void fileerror(
    char *msg,
    long  code)

{
    int  action;

	fprintf(stderr, "? XZIP: Error %s file %s\n", msg, fipnt->spec);
	if (code != 0)
	{
		char bufr[100];

		svcSysErrMsg(code, 0x03, bufr);
		fprintf(stderr, "        %s\n", bufr);
	}

	// Determine if we should see about continueing

	action = 0;
	if (erraction != 0 && (action = (erraction < 0) ? askuser() :
			erraction) == 2)
		erraction = 1;
	if (action == 0)
		exit(0);
}

//***********************************************
// Function: errormsg - Display XOS error message
// Returned: Nothing
//***********************************************

static void errormsg(
    const char *arg,
    long  code)

{
    char buffer[80];            // Buffer to receive error message

	if (code != 0)
	{
		svcSysErrMsg(code, 3, buffer);	// Get error message
		fprintf(stderr, "\n? XZIP: %s%s%s\n", buffer, (arg[0] != '\0') ?
				"; ": "", arg);		// Output error message
	}
	else
		fprintf(stderr, "\n? XZIP: %s\n", arg);
}


static int comp(
    FI *one,
    FI *two)

{
	return (stricmp(one->spec + one->skip, two->spec + two->skip));
}
