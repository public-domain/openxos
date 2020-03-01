//-------------------------------------------------------------------------
// BINCOM.C
// Binary compare utility
//
// Written by: John R. Goltz
//
// Edit History:
// 08/20/92(brn) - Add comment header
// 02/24/95(sao) - Add progasst update, added mute&quiet options, changed
//					return value to be # of differences
// 05/13/95(sao) - Changed help syntax, allowed test to continue after
//					limit reached.
// 05/16/95(sao) - Changed return values to match XOSRTN,  BINCOM is an
//					inverse case EXIT_NORM is BAD!!!!
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
//-------------------------------------------------------------------------

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
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSRTN.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <XOSERRMSG.H>

#define VERSION 3
#define EDITNO 2

#define BUFSIZE (128*1024)

struct file
{   long   dev;
    char  *name;
    uchar *buffer;
    uchar *pnt;
    int    cnt;
};

#define AF(func) (int (*)(arg_data *))func


// The program information block
Prog_Info pib;

char   copymsg[] = "";
char   example[] = "{/options} file1 file2";
char   description[] = "BINCOM performs a binary comparison between " \
    "two files.  The program will display up to LIMIT differences." \
    "  The program displays and returns the total number of differences found.";
char   prgname[] = "BINCOM";
char   envname[] = "^XOS^BINCOM^OPT";


struct file file1;
struct file file2;
char *fnm1=NULL;
char *fnm2=NULL;
long   diffcnt;
long   diffmax;
long	quiet=FALSE;
long	mute=FALSE;
int    value1;
int    value2;
long   offset;

int  getvalue(struct file *);
void openfile(struct file *, char *);
int cmdLineFile(char *);

arg_spec options[] =
{
    {"H*ELP"  , 0, NULL, AF(opthelp), 0, "This message."},
    {"?"      , 0, NULL, AF(opthelp), 0, "This message."},
    {"Q*UIET" , ASF_BOOL|ASF_STORE, NULL, &quiet, TRUE, "No output, except error messages."},
    {"M*UTE"  , ASF_BOOL|ASF_STORE, NULL, &mute, TRUE, "No output, even error messages."},
	{"L*IMIT" , ASF_NVAL|ASF_STORE, NULL, &diffmax, 12, "Maximum number of differences to display."},
    {NULL     , 0, NULL, AF(NULL)    , 0}
};

void bail(long error){
	error=error;
	if ( fnm1 != NULL )
		free(fnm1);
	if ( fnm2 != NULL )
		free(fnm2);
	exit(EXIT_NORM);
}

main(
    int   argc,
    char *argv[])

{
    char  strbuf[512];
    char *foo[2];

    reg_pib(&pib);
    init_Vars();
    if(svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
    {
	foo[0] = strbuf;
	foo[1] = '\0';
	progarg(foo, 0, options, NULL, cmdLineFile,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    if (argc>1)
    	++argv;
    progarg(argv, 0, options, NULL, cmdLineFile,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    if (mute)
	quiet=TRUE;
    if (fnm1 == NULL || fnm2 == NULL)
    {
	if ( !mute )
	    fputs("? BINCOM: Two file names are required\n", stderr);
	bail(EXIT_INVSWT);
    }
    if (argc < 3 || argc > 6)
    {
	if (!mute)
	    fputs("? BINCOM: Invalid number of options provided\n", stderr);
        bail(EXIT_INVSWT);
    }
    openfile(&file1, fnm1);
    openfile(&file2, fnm2);
    for (;;)
    {
        value1 = getvalue(&file1);
        value2 = getvalue(&file2);
        if (value1 < 0 && value2 < 0)
            break;
        if (value1 != value2 && ++diffcnt<=diffmax )
        {
	    if (!quiet && !mute)
	    {
		printf(" %06.6lX: ", offset);
		if (value1 >= 0)
		    printf(" %02.2X ", value1);
		else
		    fputs("eof", stdout);
		if (value2 >= 0)
		    printf(" %02.2X", value2);
		else
		    fputs("eof", stdout);
		fputs("\n", stdout);
	    }
	}
	++offset;
    }
    svcIoClose(0, file1.dev);
    svcIoClose(0, file2.dev);
    if (!quiet && !mute)
    {
	if (diffcnt == 0)
	    fputs("No differences found\n", stdout);
	else
	    printf("%ld difference%s found\n", diffcnt, (diffcnt == 1) ?
		    "" : "s");
    }
    if (fnm1 != NULL)
	free(fnm1);
    if (fnm2 != NULL)
	free(fnm2);
    return (diffcnt);
}

//***************************************************
// Function: cmdLineFile - Open the requested file
// Returned: TRUE
//***************************************************

int cmdLineFile(
    char *f)

{
    if (fnm1 == NULL)
    {
	fnm1 = malloc(strlen(f) + 1);
	strcpy(fnm1, f);
    }
    else
    {
	if (fnm2 == NULL)
	{
	    fnm2=malloc(strlen(f) + 1);
	    strcpy(fnm2, f);
	}
	else
	{
	    if (!mute)
		fprintf(stderr,"? ERROR %s: TOO MANY FILE NAMES PROVIDED\n",
			prgname);
	    bail(EXIT_INVSWT);
	}
    }
    return (TRUE);
}

//***********************************************************
// Function: init_Vars - Set up the program information block
// Returned: Nothing
//***********************************************************

void init_Vars(void)

{
    // Set defaults of control variables

    quiet = FALSE;
    mute = FALSE;
    file1.name = NULL;
    file2.name = NULL;
    diffmax = 12;

    // Set Program Information Block variables

    pib.opttbl = options; 		// Load the option table
    pib.build = __DATE__;
    pib.majedt = VERSION;		// major edit number
    pib.minedt = EDITNO;		// minor edit number
    pib.copymsg = copymsg;
    pib.prgname = prgname;
    pib.desc = description;
    pib.example = example;
    pib.errno = 0;
    getTrmParms();
    getHelpClr();
}


void openfile(
    struct file *file,
    char  *name)

{
    file->buffer = (uchar *)getspace(BUFSIZE);
    if ((file->dev = svcIoOpen(O_IN, name, NULL)) < 0)
    {
	if (!mute)
	    fprintf(stderr,"? BINCOM: Unable to open file %s\n",
		    prgname,name);
	bail(EXIT_FNF);
    }
    file->name = name;
}

int getvalue(
    struct file *file)

{
    if (file->cnt == -1)
        return (-1);
    if (--(file->cnt) < 0)
    {
        if ((file->cnt = (int)svcIoInBlock(file->dev, (char *)(file->buffer),
                BUFSIZE)) < 0)
        {
            if (file->cnt != ER_EOF)
                femsg(prgname, file2.dev, file->name);
            else
            {
                file->cnt = -1;
                return (-1);
            }
        }
        file->pnt = file->buffer;
        --(file->cnt);
    }
    return ((int)*(file->pnt)++);
}
