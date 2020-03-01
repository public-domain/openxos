//--------------------------------------------------------------------------*
// DOSCOM.C - Program to set up DOS standard serial ports
//
// Written by: John R. Goltz
//
// Edit History:
// 01/03/95(brn) - Cloned from doslpt
// 05/10/95(sao) - Added progasst package
// 05/13/95(sao) - Changed 'optional' indicator
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
#include <XOSERR.H>
#include <PROGARG.H>
#include <PROGHELP.H>

#define VERSION 3
#define EDITNO  3

char comname[] = "TRM:";	/* Class name */

struct addchar
{   byte4_char unit;
    text4_char type;
    byte4_char ioreg;
    byte4_char intlvl;
    char       end;
} addcom =
{   {PAR_SET|REP_DECV, 4, "UNIT" , 1},
    {PAR_SET|REP_TEXT, 4, "TYPE", "SERA"},
    {PAR_SET|REP_HEXV, 4, "IOREG", 0x3f8},
    {PAR_SET|REP_DECV, 4, "INT"  , 4}
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
    comname, 0,			// qab_buffer1  - Pointer to file spec
    (char *)&addcom, 0,		// qab_buffer2  - Pointer to char. list
    NULL, 0			// qab_parm     - Unused
};

long   quiet     = FALSE;    /* TRUE if only error messages wanted */
long   mute      = FALSE;    /* TRUE if no output wanted */
char  copymsg[] = "";
char  prgname[] = "DOSCOM";
char  envname[] = "^XOS^DOSCOM^OPT";
char  example[] = "{/options}";
char  description[] = "This command attempts to add terminal class units " \
    "1 and 2 (TRM1: and TRM2:) in order to use the standard hardware COM1 " \
    "and COM2 serial ports.  If this is successful, logical device COM1: " \
    "is defined as TRM1: and COM2: as TRM2:";

int  setcom(int, int);
Prog_Info pib;

#define AF(func) (int (*)(arg_data *))func

arg_spec options[] =
{
    {"Q*UIET"  , ASF_BOOL|ASF_STORE, NULL, &quiet , TRUE, "Suppress all messages, except errors." },
    {"M*UTE"   , ASF_BOOL|ASF_STORE, NULL, &mute , TRUE, "Suppress all messages, except errors." },
    {"H*ELP"   , 0, NULL, AF(opthelp), 0, "This message."},
    {"?"      , 0, NULL, AF(opthelp), 0, "This message."},
    {NULL     , 0, NULL, AF(NULL)    , 0, NULL}
};

main(argc, argv)
int   argc;
char *argv[];

{
    char  strbuf[32];
    char *foo[2];
    int comseen;

	reg_pib(&pib);

	init_Vars();

    comseen = FALSE;

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

    if (setcom(0x3f8, 4))
	comseen = TRUE;

    if (!setcom(0x2f8, 3) && !quiet && !comseen)
        fputs("? DOSCOM: No serial ports found\n", stdout);
    return (EXIT_NORM);
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

/********************************************************/
/* Function: setcom - Set up single COM serial port	*/
/* Returned: Nothing					*/
/********************************************************/

int setcom(int ioreg, int irq)

{
    long   rtn;

    addcom.ioreg.value = ioreg;
    addcom.intlvl.value = irq;
    if ((rtn = svcIoQueue(&addqab)) < 0 || (rtn = addqab.qab_error) < 0)
    {
        char errmsg[100];

        if (rtn == ER_PDNAV)		/* Physical device not available? */
            return (FALSE);			/* Yes - just return quietly */
        svcSysErrMsg(rtn, 3, errmsg);  /* No - get error message string */
	if (!mute)				/* Tell him about it if we should */
            fprintf(stderr, "? DOSCOM: Error adding unit %ld to class %s\n"
                        "          %s\n", addcom.unit.value, comname, errmsg);
        return (FALSE);
    }
    if (!quiet && !mute)				/* Tell him about it if we should */
        printf("DOSCOM: TRM%ld: set up - IO register = 0x%X, interrupt = %ld \n",
                addcom.unit.value, ioreg, addcom.intlvl.value);
    ++addcom.unit.value;
    return (TRUE);
}

