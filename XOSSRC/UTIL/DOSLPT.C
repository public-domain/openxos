//--------------------------------------------------------------------------*
// DOSLPT.C - Program to set up DOS standard parallel ports
//
// Written by: John R. Goltz
//
// Edit History:
// 10/16/91(tmg) - Misc. cleanup, added help screen
// 05/12/94(brn) - Changed version number to reflect 32 bit version
// 03/17/95(sao) - Added progasst package
// 05/13/95(sao) - Changed 'optional' indicator
// 05/16/95(sao) - Changed exit codes to reflect ALGRTN
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
#include <XOSERR.H>
#include <PROGARG.H>
#include <PROGHELP.H>

#define VERSION 3
#define EDITNO  3

char lptname[] = "PPR:";	// Class name

struct addchar
{   byte4_char unit;
    byte4_char ioreg;
    byte4_char intlvl;
    char       end;
} addchar =
{   {PAR_SET|REP_DECV, 4, "UNIT" , 1},
    {PAR_SET|REP_HEXV, 4, "IOREG", 0},
    {PAR_SET|REP_DECV, 4, "INT"  , 7}
};

type_qab addqab =
{
    QFNC_WAIT|QFNC_CLASSFUNC,	// qab_func     - Function
    0,				// qab_status   - Returned status
    0,				// qab_error    - Error code
    0,				// qab_amount   - Amount
    0,				// qab_handle   - Device handle
    0,				// qab_vector   - Vector for interrupt
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    CF_ADDUNIT,			// qab_option   - Options or command
    0,				// qab_count    - Count
    lptname, 0,			// qab_buffer1  - Pointer to file spec
    (char *)&addchar, 0,	// qab_buffer2  - Pointer to char. list
    NULL, 0			// qab_parm     - Unused
};

long  quiet     = FALSE;    /* TRUE if no output wanted */
long  mute      = FALSE;
char  prgname[] = "DOSLPT";
char  envname[] = "^XOS^DOSLPT^OPT";
Prog_Info pib;
char  copymsg[] = "";
char  example[] = "{/option}";
char  description[] = "This command sets up the first two standard parallel " \
    "printer devices.  Parallel printer setup is somewhat complex since " \
    "there exists some confusion about which I/O register values correspond " \
    "to which printer ports.  This command uses the same algorithm as DOS " \
    "to assign printer ports.  The XOS PPR class units 1 (PPR1:) and 2 " \
    "(PPR2:) are added if the corresponding physical interfaces exist in " \
    "the system and logical names LPT1 and LPT2 are defined as the " \
    "corresponding XOS names.  DOSLPT is normally executed in the " \
    "STARTUP.BAT file when the system is initialized, although it can " \
    "be executed at any time.  Note that alternately, the ADDUNIT and " \
    "LOGICAL commands can be used to explicitly set up the parallel " \
    "printer ports.";

void setlpt(int);

#define AF(func) (int (*)(arg_data *))func

arg_spec options[] =
{
    {"Q*UIET" , ASF_BOOL|ASF_STORE, NULL, &quiet,    TRUE, "Display error messages only."},
    {"M*UTE"  , ASF_BOOL|ASF_STORE, NULL, &mute,     TRUE, "Do not display any messages."},
    {"H*ELP"  , 0,                  NULL, AF(opthelp), 0, "This message." },
    {"?"      , 0,                  NULL, AF(opthelp), 0, "This message." },
    {NULL     , 0,                  NULL, AF(NULL),     0, NULL }
};

main(argc, argv)
int   argc;
char *argv[];

{
    char  strbuf[32];
    char *foo[2];

	reg_pib(&pib);

	init_Vars();

    if(svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
    {
	foo[0] = strbuf;
	foo[1] = '\0';
    progarg(foo, 0, options, NULL, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    if (argc != 0)
        ++argv;
    progarg(argv, 0, options, NULL, (int (*)(char *))NULL,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

    setlpt(0x3BC);
    setlpt(0x378);
    if (addchar.unit.value < 3)
        setlpt(0x278);
    if (!quiet && addchar.unit.value == 1)
        fputs("? DOSLPT: No parallel ports found\n", stdout);
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

/********************************************************/
/* Function: setlpt - Set up single LPT serial port	*/
/* Returned: Nothing					*/
/********************************************************/

void setlpt(
    int ioreg)

{
    long   rtn;

    addchar.ioreg.value = ioreg;
    if ((rtn = svcIoQueue(&addqab)) < 0 || (rtn = addqab.qab_error) < 0)
    {
        char errmsg[100];

        if (rtn == ER_PDNAV)		/* Physical device not available? */
            return;			/* Yes - just return quietly */
        svcSysErrMsg(rtn, 3, errmsg);  /* No - get error message string */
        if ( !mute )
            fprintf(stderr, "? DOSLPT: Error adding unit %ld to class PPR:\n"
                        "          %s\n", addchar.unit.value, errmsg);
        exit(EXIT_SVCERR);
    }
    if ( !quiet && !mute )             /* Tell him about it if we should */
        printf("DOSLPT: PPR%ld: set up - IO register = 0x%X, interrupt = %ld \n",
                addchar.unit.value, ioreg, addchar.intlvl.value);
    ++addchar.unit.value;
    addchar.intlvl.value = 5;
}

