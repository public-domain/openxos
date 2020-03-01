//--------------------------------------------------------------------------*
// CONFIG - program to modify ^XOS^GCP options
//
//  Command format:
// 	CONFIG
// 		Display current settings
//    or
// 	CONFIG DOSQUIRK=ON|OFF DOSNAME=ON|OFF
//
// Written by: Tom Goltz
//
// Edit History:
// 10/14/91(tmg) - Created initial version
// 08/20/92(brn) - Change reference to global.h from local to library
// 05/09/94(brn) - Use C++ comments and change to 32 bit C
// 02/28/95(sao) - Added progarg pkg, added mute option
// 05/13/95(sao) - Changed 'option' indicator
// 05/15/95(sao) - Changed exit codes to reflect XOSRTN
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
#include <XOSRTN.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSRTN.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <GLOBAL.H>
#include <XOSERRMSG.H>

#define VERSION 3
#define EDITNO  3

char  	prgname[] = "CONFIG";
char  	envname[] = "^XOS^CONFIG^OPT";
char	example[] = "{/option} {DOSQUIRK=ON|OFF} {DOSNAME=ON|OFF}";
char    copymsg[] = "";
char    description[] = "CONFIG allows the global configuration variables DOSQUIRK and DOSDRIVE to be changed.";

long	quiet     = FALSE;
long	mute	  = FALSE;

void   init_Vars(void);

SubOpts onoff[3] =
{
	{ "OFF", "  "},
    { "ON","  "},
    { NULL, NULL }
};

arg_spec options[] =
{
    {"H*ELP"    , 0, NULL, AF(opthelp), 0, "This message."},
    {"?"        , 0, NULL, AF(opthelp), 0, "This message."},
    {"Q*UIET"   , ASF_BOOL|ASF_STORE, NULL, &quiet, TRUE, "Only error messages displayed."},
	{"M*UTE"    , ASF_BOOL|ASF_STORE, NULL, &mute , TRUE, "No messages displayed."},
    {NULL     , 0, NULL, AF(NULL)    , 0, NULL}
};

arg_spec keywords[] =
{
    {"DOSQ*UIRK", ASF_XSVAL|ASF_STORE, &onoff, &gcp_dosquirk , 0, "Changes the global DOS quirk setting."},
    {"DOSN*AME" , ASF_XSVAL|ASF_STORE, &onoff, &gcp_dosdrive , 0, "Changes the global DOS drive name setting."},
    {NULL     , 0, NULL, AF(NULL)    , 0, NULL}
};

Prog_Info pib;

int main(
    int argc,
    char *argv[])

{
    long   rtn;
    char   buffer[512];
    char  *foo[2];

	long o_dq;
	long o_dn;

	reg_pib(&pib);

	init_Vars();

// Process global configuration parameters
    global_parameter(TRUE);
	o_dq=gcp_dosquirk;
	o_dn=gcp_dosdrive;

// Process default options for this program

    if(svcSysFindEnv(0, envname, NULL, buffer, sizeof(buffer), NULL) > 0)
    {
	foo[0] = buffer;
	foo[1] = '\0';
	progarg(foo, 0, options, NULL, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

// Process command-line options

    if (argc > 1)
    {
		++argv;
        progarg(argv, 0, options, keywords, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
		if ( mute )
			quiet=TRUE;
		if (gcp_dosquirk!=o_dq || gcp_dosdrive!=o_dn )
		{
	    	sprintf(buffer, "DOSQUIRK=%s DOSDRIVE=%s", gcp_dosquirk ? "ON" : "OFF",
						       gcp_dosdrive ? "ON" : "OFF");
            if ((rtn = svcSysDefEnv(FES_SESSION, "^XOS^GCP", buffer)) < 0)
			{
				femsg(prgname, rtn, "^XOS^GCP");
				return (EXIT_SVCERR);
			};
		};
    }
    if ( !quiet && !mute)
	{
	    printf("DOSQUIRK (DOS-compatible utilities)  are %s\n", gcp_dosquirk ? "ON" : "OFF");
    	printf("DOSNAME  (DOS-compatible disk names) are %s\n", gcp_dosdrive ? "ON" : "OFF");
	};
    return (EXIT_NORM);
}

//***************************************************
// Function: init_Vars - Set up the program information block
// Returned: none
//***************************************************
void init_Vars(void){

	// Set defaults of control variables
	quiet=FALSE;
	mute=FALSE;

	// set Program Information Block variables
	pib.opttbl=options; 		// Load the option table
	pib.kwdtbl=keywords;
	pib.majedt = VERSION; 			// major edit number
	pib.minedt = EDITNO; 			// minor edit number
	pib.copymsg=copymsg;
	pib.prgname=prgname;
	pib.desc=description;
	pib.example=example;
	pib.errno=0;
	pib.build=__DATE__;
	getTrmParms();
	getHelpClr();
};

