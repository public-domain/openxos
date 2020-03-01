//--------------------------------------------------------------------------*
// MKDIR.C
//
// Written by: John R. Goltz
//
// Edit History:
// 01/07/90(brn) - Fixed old use of /
// 08/20/92(brn) - Change reference to global.h from local to library
// 04/25/94(brn) - Make success return 0
// 05/12/94(brn) - Fix command abbreviations
// 04/03/95(sao) - Added progasst package
// 05/14/95(sao) - Changed example,  added mute option
// 05/16/95(sao) - Changed exit codes to reflect XOSRTN
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
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

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSRTN.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <GLOBAL.H>

#define VERSION 3
#define EDITNO  4

Prog_Info pib;
char  copymsg[] = "";
char  example[] = "{/option} dir-name";
char  description[] = "This command creates the specified directory.  " \
    "The directory to be created must not already exist, but any parent " \
    "directories or device names specified in the path must exist.";
char  prgname[] = "MKDIR";
char  envname[] = "^XOS^MKDIR^OPT";

int  namecount = 0;

int  ignore_str(void);
int  nonopt(char *);
long quiet=FALSE;
long mute=FALSE;

#define AF(func) (int (*)(arg_data *))func

arg_spec options[] =
{
    {"Q*UIET" , ASF_BOOL|ASF_STORE, NULL, &quiet  , 0, "Suppress all but error messages."},
    {"M*UTE"  , ASF_BOOL|ASF_STORE, NULL, &mute   , 0, "Suppress all messages."},
    {"?"      , 0, NULL, AF(opthelp)   , 0, "This message."},
    {"H*ELP"  , 0, NULL, AF(opthelp)   , 0, "This message."},
    {NULL     , 0, NULL, AF(NULL)       , 0, NULL}
};

struct				/* Paramters for device information	*/
{
    byte4_parm  options;
    lngstr_parm spec;
    char        end;
} parm_open =
{
    {(PAR_SET|REP_HEXV), 4, IOPAR_FILOPTN, FO_NOPREFIX|FO_VOLNAME|FO_PATHNAME|
            FO_FILENAME},
    {(PAR_GET|REP_STR),  0, IOPAR_FILSPEC, NULL, 0, FILESPCSIZE, 0},
    0
};

char rtnname[FILESPCSIZE+4] = "";

void main(argc, argv)
int   argc;
char *argv[];

{
    char   strbuf[512];
    char  *foo[2];

	reg_pib(&pib);

	init_Vars();

    global_parameter(TRUE);
    if (gcp_dosquirk)
	quiet = TRUE;

    if(svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
    {
        foo[0] = strbuf;
        foo[1] = '\0';
        progarg(foo, 0, options, NULL, (int (*)(char *))NULL,
                    (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

/*
 * Process command-line parameters
 */

    if (argc > 1)
    {
        ++argv;
        progarg(argv, 0, options, NULL, (int (*)(char *))ignore_str,
                    (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
        progarg(argv, 0, NULL, NULL, nonopt, (void (*)(char *, char *))NULL,
                    (int (*)(void))NULL, NULL);
    }
    if (namecount == 0)
    {
		if ( !mute )
        fprintf(stderr, "? %s: No directory name specified\n", prgname);
        exit(EXIT_INVSWT);
    }
    exit(EXIT_NORM);			// No errors
}

int ignore_str(void)

{
    return (TRUE);
}

int nonopt(
    char *arg)

{
    char *pnt1;
    long  rtn;
    char  namebfr[512];
    char  chr;

    ++namecount;		/* Increase number of names found */

    pnt1 = namebfr;		/* Copy name over and append trailing slash */
    do				/*   if needed */
    {
	chr = *pnt1++ = *arg++;
    } while (*arg != '\0');

    if (chr != '\\')		/* Trailing slash found?	*/
	*pnt1++ = '\\';		/* No - add one!		*/
    *pnt1 = '\0';		/* Terminate string properly	*/

    parm_open.spec.buffer = (char *)rtnname;
    if (gcp_dosdrive)
        parm_open.options.value = FO_NOPREFIX|FO_DOSNAME|FO_PATHNAME|
		FO_FILENAME;
    if ((rtn = svcIoOpen(O_CREATE|O_ODF|O_FAILEX, namebfr, &parm_open)) < 0)
    {
        svcSysErrMsg(rtn, 3, rtnname); /* Get error message string */
		if ( !mute )
        fprintf(stderr, "? %s: Error creating directory %s\n"
                        "         %s\n", prgname, namebfr, rtnname);
    }
    else if (!quiet && !mute)
        printf("%s: Directory %s created\n", prgname, rtnname);
    return (TRUE);
}

//***************************************************
// Function: init_Vars - Set up the program information block
// Returned: none
//***************************************************
void init_Vars(void)
{
	// set Program Information Block variables
	pib.opttbl=options; 		// Load the option table
    pib.kwdtbl=NULL;
	pib.build=__DATE__;
	pib.majedt = VERSION; 			// major edit number
	pib.minedt = EDITNO; 			// minor edit number
	pib.copymsg=copymsg;
	pib.prgname=prgname;
	pib.desc=description;
	pib.example=example;
	pib.errno=0;
	getTrmParms();
	getHelpClr();
};


