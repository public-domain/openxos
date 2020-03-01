//--------------------------------------------------------------------------*
// DOSQUIRK.C
// Program to condition DOSQUIRK option
//
// Written by: John R. Goltz
//
// Edit History:
// 08/20/92(brn) - Change reference to global.h from local to library
// 03/17/95(sao) - Added progasst package.
// 05/14/95(sao) - Changed 'optional' indicators
// 05/16/95(sao) - Changed exit values to reflect ALGRTN
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
//-------------------------------------------------------------------------*

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
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSRTN.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <GLOBAL.H>
#include <XOSERRMSG.H>

#define VERSION 3
#define EDITNO  3


int optON(arg_data *);
int optOFF(arg_data *);

arg_spec options[] =
{
    {"H*ELP"  , 0, NULL, AF(opthelp), 0, "This message."},
    {"?"      , 0, NULL, AF(opthelp), 0, "This message."},
    {NULL     , 0, NULL, AF(NULL)    , 0, NULL}
};

arg_spec keywords[] =
{
    {"ON"    , 0, NULL, optON, 0, "Set DOSQuirk ON. YES and TRUE have the same affect."},
    {"OFF"   , 0, NULL, optOFF,0, "Set DOSQuirk OFF. NO and FALSE have the same affect."},
    {"Y*ES"  , 0, NULL, optON, 0, NULL},
    {"N*O"   , 0, NULL, optOFF,0, NULL},
    {"T*RUE" , 0, NULL, optON, 0, NULL},
    {"F*ALSE", 0, NULL, optOFF,0, NULL},
    {NULL  , 0, NULL, AF(NULL),0, NULL}
};

Prog_Info pib;
char  prgname[] = "DOSQUIRK";
char  copymsg[] = "";
char  example[] = "{/option} {keyword}";
char  description[] = "This command is used to view/set the global " \
    "DOSQUIRK value.  The DOSQUIRK value is used by many XOS " \
    "utilities to determine the display/output formats.  If DOSQUIRK " \
    "is true (on), the output should closely mimic that of DOS.  " \
    "Otherwise, XOS native mode will be used (which is usually " \
    "a bit more descriptive.)";

int main(int argc, char **argv)
{

/*
 * Process global configuration parameters
 */

    int  rtn=0;
	long o_dq;
	long o_dn;
    char buffer[128];

	reg_pib(&pib);

	init_Vars();

    global_parameter(TRUE);
	o_dq=gcp_dosquirk;
	o_dn=gcp_dosdrive;

    if (argc > 1)
    {
        argv++;
        progarg(argv, 0, options, keywords, (int (*)(char *))NULL,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    };
//	 else {
//		fprintf(stderr,"? ERROR %s:  Insufficient number of parameters.\n",prgname);
//    }

    if (gcp_dosquirk!=o_dq )
    {
        sprintf(buffer, "DOSQUIRK=%s DOSDRIVE=%s", gcp_dosquirk ? "ON" : "OFF",
                            gcp_dosdrive ? "ON" : "OFF");
        if ((rtn = svcSysDefEnv(FES_SESSION, "^XOS^GCP", buffer)) < 0)
        {
            femsg(prgname, rtn, "^XOS^GCP");
            return (EXIT_SVCERR);
        };
    };
    printf("DOSQUIRK is %s\n",gcp_dosquirk ? "ON" : "OFF" );
    return (EXIT_NORM);
}

//*********************************************************************
// Function: optON
// Turn DOSQuirk ON
//*********************************************************************

int optON(arg_data *arg)
{
    arg->data=arg->data;
    gcp_dosquirk=TRUE;
    return 0;
}

//*********************************************************************
// Function: optOFF
// Turn DOSQuirk OFF
//*********************************************************************

int optOFF(arg_data *arg)
{
    arg->data=arg->data;
    gcp_dosquirk=FALSE;
    return 0;
}


//***************************************************
// Function: init_Vars - Set up the program information block
// Returned: none
//***************************************************
void init_Vars(void){

	// set Program Information Block variables
	pib.opttbl=options; 		// Load the option table
    pib.kwdtbl=keywords;
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
