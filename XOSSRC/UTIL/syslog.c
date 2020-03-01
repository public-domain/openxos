//--------------------------------------------------------------------------*
// SYSLOG.C
//
// Written by: John R. Goltz
//
// Edit History:
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xos.h>
#include <xossvc.h>
#include <xosrtn.h>
#include <progarg.h>
#include <proghelp.h>
#include <global.h>

#define VERSION 1
#define EDITNO  0

Prog_Info pib;

char  logstr[300] = "\0\0\0\0SYSLOG  ";
char *pntstr = logstr + 12;

long quiet=FALSE;
long mute=FALSE;

int  nonopt(char *);
int  optroll(arg_data *);

#define AF(func) (int (*)(arg_data *))func

arg_spec options[] =
{
    {"Q*UIET" , ASF_BOOL|ASF_STORE, NULL, &quiet     , 0,
			"Suppress all but error messages."},
    {"M*UTE"  , ASF_BOOL|ASF_STORE, NULL, &mute      , 0,
			"Suppress all messages."},
	{"R*OLL"  , 0                 , NULL, optroll    , 0,
			"Archive current system log file and start new log file."},
    {"?"      , 0                 , NULL, AF(opthelp), 0,
			"Display this message."},
    {"H*ELP"  , 0                 , NULL, AF(opthelp), 0,
			"Display this message."},
    {NULL}
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

char  copymsg[] = "";
char  example[] = "{/option} logfile text string";
char  description[] = "This command inserts the specified text into the "
		"system log file. Multiple spaces or tabs are converted to single "
		"spaces unless the entire text string is inclosed in double quotes. "
		"It also provides an option for \"rolling\" the system log file, "
		"that is, for archiving the current system log file and creating a "
		"new log file.";

char  prgname[] = "SYSLOG";
char  envname[] = "^XOS^SYSLOG^OPT";


void main(
	int   argc,
	char *argv[])

{
    char   strbuf[512];
    char  *foo[2];

	reg_pib(&pib);
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

	// Process command-line parameters

    if (argc > 1)
    {
        ++argv;
///        progarg(argv, 0, options, NULL, (int (*)(char *))NULL,
///                    (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);


        progarg(argv, 0, options, NULL, nonopt, (void (*)(char *, char *))NULL,
                    (int (*)(void))NULL, NULL);
    }
	if (logstr[0] == 0 && pntstr == (logstr + 12))
	{
		fputs("SYSLOG: Nothing to do\n", stderr);
		exit(1);
	}
	svcSysLog(logstr, pntstr - logstr);
	exit(0);
}


int optroll(
	arg_data *arg)

{
	arg = arg;
	memcpy(logstr, "roll", 4);
	return (TRUE);
}


int nonopt(
    char *arg)

{
	int avail;
	int len;

	avail = sizeof(logstr) - 14 - (pntstr - logstr);
	len = strlen(arg);
	if (avail > len)
		avail = len;
	if (pntstr != (logstr + 12))
		*pntstr++ = ' ';
	memcpy(pntstr, arg, avail);
	pntstr += avail;
    return (TRUE);
}
