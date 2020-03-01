//--------------------------------------------------------------------------*
// KILLPROC.C
// Program to kill another process
//
// Written by: John R. Goltz
//
// Edit History:
// 08/20/92(brn) - Add comment header at beginning of file
// 05/12/94(brn) - Fix command abbreviations and version number for 32 bit
// 04/01/94(sao) - Added progasst package
// 05/14/95(sao) - Changed description,  added MUTE option
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
#include <CTYPE.H>
#include <XOSSVC.H>
#include <XOSRTN.H>
#include <PROGARG.H>
#include <PROGHELP.H>

#define VERSION 3
#define EDITNO  3


int  get_procid(char *string);
long decarg(char **str);
void badpid(void);

#define AF(func) (int (*)(arg_data *))func
long quiet=FALSE;
long mute =FALSE;

arg_spec options[] =
{
    {"Q*UIET" , ASF_BOOL|ASF_STORE, NULL, &quiet      , TRUE, "Only error messages are displayed."},
    {"M*UTE"  , ASF_BOOL|ASF_STORE, NULL, &mute       , TRUE, "No messages are displayed."},
    {"?"      , 0,                  NULL, AF(opthelp), 0, "This message."},
    {"H*ELP"  , 0,                  NULL, AF(opthelp), 0, "This message."},
    {NULL     , 0,                  NULL, AF(NULL)    , 0, NULL}
};

char  copymsg[] = "";
char  prgname[] = "KILLPROC";
char  envname[] = "^XOS^KILLPROC^OPT";
char  example[] = "{/option} process-ID";
char  description[] = "This command removes a process from the system.  " \
    "The full process ID (PID) must be specified as SEQ.NUM where SEQ is " \
    "the process sequence number and ID is the process number.  Process " \
    "IDs can be obtained from the SYSDIS function.";

long  procid = 0xFFFFFFFFL;
char *pidarg;
Prog_Info pib;

main(argc, argv)
int   argc;
char *argv[];

{
    long  rtn;
    char  buffer[100];
    char   strbuf[512];
    char  *foo[2];

	reg_pib(&pib);

	init_Vars();
    if(svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
    {
	foo[0] = strbuf;
	foo[1] = '\0';
    progarg(foo, 0, options, NULL, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    argc--;
    argv++;
    progarg(argv, 0, options, (arg_spec *)NULL, get_procid,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    if (procid == 0xFFFFFFFFL)		/* Make sure have process ID */
    {
		if ( !mute )
        fputs("? KILLPROC: No process specified\n"
	      "            type KILLPROC /? for usage\n", stderr);
        exit(EXIT_INVSWT);
    }

    rtn = svcSysGetPid();      /* Get our process ID */
    if (rtn == procid)			/* Make sure not this process! */
        badpid();
    if ((rtn = svcSchKill(-1, procid)) < 0) /* OK - terminate the process */
    {					/* If error */
        svcSysErrMsg(rtn, 3, buffer); /* Get error message string */
		if ( !mute )
        fprintf(stderr, "? KILLPROC: Error terminating process %ld.%ld\n"
                        "            %s\n", procid>>16, procid&0xFFFF, buffer);
        exit(EXIT_SVCERR);
    }					/* It worked */
    if (!quiet && !mute)				/* Tell him about it if we should */
        printf("KILLPROC: Process %ld.%ld terminated\n", procid>>16,
                procid&0xFFFF);
    return (EXIT_NORM);
}

//***************************************************
// Function: init_Vars - Set up the program information block
// Returned: none
//***************************************************
void init_Vars(void){

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

int get_procid(char *string)
{
    unsigned long rtn;

    if (procid != 0xFFFFFFFFL)		/* Already have a process ID?	*/
	{
		if (!mute) fprintf(stderr,"? ERROR %s:  Process ID already defined.\n",prgname);
		exit(EXIT_INVSWT);
	}
    pidarg = string;			/* Save the string for later	*/
    rtn = decarg(&string);	/* No - get first part of process ID	*/
    if (rtn > 0xFFFF || *string != '.')	/* Valid value?			*/
	badpid();			/* No				*/
    ++string;				/* Yes - get second part	*/
    procid = decarg(&string);
    if (procid < 2)
    {
		if ( !mute )
		fprintf(stderr, "? KILLPROC: Illegal process ID \"%s\" specified;\n"
			"            system processes may not be killed\n", pidarg);
		exit(EXIT_INVSWT);
    }
    if (procid >0xFFF || *string != '\0')	/* Valid value?		*/
	badpid();				/* No			*/
    procid |= rtn << 16;		/* Yes - get complete process ID */
    return (TRUE);
}

/*
 * Function to get value of decimal argument
 */

long decarg(str)
register char **str;

{
    long  value;
    char  chr;

    value = 0;
    while ((chr = **str) != '\0' && isdigit(chr))
    {
        (*str)++;
        chr &= 0xF;
        value = value * 10 + chr;
    }
    return (value);
}

/*
 * Function to report bad process ID value
 */

void badpid()

{
	if ( !mute )
    fprintf(stderr, "? KILLPROC: Illegal process ID \"%s\" specified; process"
            " ID must\n            be two decimal values separated by a"
            " period\n", pidarg);
    exit(EXIT_INVSWT);
}
