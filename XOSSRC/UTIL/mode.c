//--------------------------------------------------------------------------*
// Mode.c
//
// Written by: Tom Goltz
//
// Edit History:
// 04/05/91(tmg) - created first version
// 05/12/94(brn) - Fixed command abbreviations and version number for 32 bit
// 04/04/95(sao) - Added progasst package
// 05/16/95(sao) - Chnged exit codes to reflect XOSRTN
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
#include <CTYPE.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSTRM.H>
#include <XOSRTN.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <XOSERMSG.H>


///////////////////////////////////////////////////
////                                           ////
////  THIS PROGRAM IS NO LONGER SUPPORTED !!!  ////
////                                           ////
///////////////////////////////////////////////////


#define VERSION 3
#define EDITNO  2

/*
 * Define status values for command-line parameters
 */

#define NEXT_device     0
#define NEXT_baud       1
#define NEXT_parity     2
#define NEXT_word       3
#define NEXT_stop       4
#define NEXT_timeout    5
#define NEXT_prnwidth   6
#define NEXT_prnheight  7
#define NEXT_invalid    8

char  copymsg[] = "";
char  prgname[] = "MODE";
char  envname[] = "^XOS^MODE";
Prog_Info pib;
char  example[] = "parameters";
char  description[] = "The MODE command is provided for DOS compatibility." \
    "  It does not conform to the XOS standard command syntax.  It " \
    "is recommended that this command be used as little as possible due " \
    "to the limited nature of the options it provides.";


void opt_help(void);
int  key_lpt1(arg_data *arg);
int  key_lpt2(arg_data *arg);
int  key_lpt3(arg_data *arg);
void key_lpt(char unitnum, arg_data *arg);
int  key_com1(arg_data *arg);
int  key_com2(arg_data *arg);
int  key_width(arg_data *arg);
int  key_bw(arg_data *arg);
int  key_co(arg_data *arg);
int  key_mono(arg_data *arg);
int  key_wild(arg_data *arg);
void procarg_error(void);

#define AF(func) (int (*)(arg_data *))func

arg_spec options[] =
{
    {"?"   , 0, NULL, AF(opthelp), 0, "This message."},
    {"H*ELP", 0, NULL, AF(opthelp), 0, "This message."},
    {NULL  , 0, NULL, AF(NULL)    , 0, NULL}
};

SubOpts lptlist[] =
{
    { "COM1|:", "Comm Port 1" },
    { "COM2|:", "Comm Port 2" },
    { NULL, NULL }
};

arg_spec keywords[] =
{
    {"LPT1|:"   , ASF_XSVAL, &lptlist, key_lpt1 , 0,"Re-direct LPTx [x = 1|2|3] through the selected communications port.  Line lengths of 80 or 132 may be specified; e.g. LPT2:132 = COM2:" },
    {"LPT2|:"   , ASF_XSVAL, &lptlist, key_lpt2 , 0,NULL},
    {"LPT3|:"   , ASF_XSVAL, &lptlist, key_lpt3 , 0,NULL},
    {"LPT1:80"  , ASF_XSVAL, &lptlist, key_lpt1 , 80,NULL},
    {"LPT2:80"  , ASF_XSVAL, &lptlist, key_lpt2 , 80,NULL},
    {"LPT3:80"  , ASF_XSVAL, &lptlist, key_lpt3 , 80,NULL},
    {"LPT1:132" , ASF_XSVAL, &lptlist, key_lpt1 , 132,NULL},
    {"LPT2:132" , ASF_XSVAL, &lptlist, key_lpt2 , 132,NULL},
    {"LPT3:132" , ASF_XSVAL, &lptlist, key_lpt3 , 132,NULL},
    {"COM1|:"   , 0        , NULL   , key_com1 , 0, "COMx:Baud, Parity, Size, Stop, Timeout Flag [x=1|2]"},
    {"COM2|:"   , 0        , NULL   , key_com2 , 0,NULL},
    {"COM1:110" , 0        , NULL   , key_com1 , 110,NULL},
    {"COM1:150" , 0        , NULL   , key_com1 , 150,NULL},
    {"COM1:300" , 0        , NULL   , key_com1 , 300,NULL},
    {"COM1:600" , 0        , NULL   , key_com1 , 600,NULL},
    {"COM1:1200", 0        , NULL   , key_com1 , 1200,NULL},
    {"COM1:2400", 0        , NULL   , key_com1 , 2400,NULL},
    {"COM1:4800", 0        , NULL   , key_com1 , 4800,NULL},
    {"COM1:9600", 0        , NULL   , key_com1 , 9600,NULL},
    {"COM2:110" , 0        , NULL   , key_com2 , 110,NULL},
    {"COM2:150" , 0        , NULL   , key_com2 , 150,NULL},
    {"COM2:300" , 0        , NULL   , key_com2 , 300,NULL},
    {"COM2:600" , 0        , NULL   , key_com2 , 600,NULL},
    {"COM2:1200", 0        , NULL   , key_com2 , 1200,NULL},
    {"COM2:2400", 0        , NULL   , key_com2 , 2400,NULL},
    {"COM2:4800", 0        , NULL   , key_com2 , 4800,NULL},
    {"COM2:9600", 0        , NULL   , key_com2 , 9600,NULL},
    {"40"       , 0        , NULL   , key_width, 40, "Set screen width to 40 characters."},
    {"80"       , 0        , NULL   , key_width, 80, "Set screen width to 80 characters."},
    {"BW40"     , 0        , NULL   , key_bw   , 40, "Set colors to black & white and screen width to 40 characters."},
    {"BW80"     , 0        , NULL   , key_bw   , 80, "Set colors to black & white and screen width to 80 characters."},
    {"CO40"     , 0        , NULL   , key_co   , 40, "Set screen width to 40 characters with color."},
    {"CO80"     , 0        , NULL   , key_co   , 80, "Set screen width to 80 characters with color."},
    {"MONO"     , 0        , NULL   , key_mono , 0, "Set screen to monochrome."},
    {"*"        , 0        , NULL   , key_wild , 0, NULL},
    {NULL       , 0        , NULL   , NULL     , 0, NULL}
};

struct				/* Paramters for device information	*/
{
    byte4_char  irate;
    byte4_char  rate;
    byte4_char  iparity;
    byte4_char  parity;
    byte1_char  idbits;
    byte1_char  dbits;
    byte1_char  isbits;
    byte1_char  sbits;
    char        end;
} comchar =
{
    {(PAR_SET|REP_DECV), 4, "IRATE"},
    {(PAR_SET|REP_DECV), 4, "RATE"},
    {(PAR_SET|REP_TEXT), 4, "IPARITY"},
    {(PAR_SET|REP_TEXT), 4, "PARITY"},
    {(PAR_SET|REP_DECV), 1, "IDBITS"},
    {(PAR_SET|REP_DECV), 1, "DBITS"},
    {(PAR_SET|REP_DECV), 1, "ISBITS"},
    {(PAR_SET|REP_DECV), 1, "SBITS"},
    0
};

type_qab comqab =
{
    QFNC_WAIT|QFNC_OPEN,
    0,				// qab_status   - Returned status
    0,				// qab_error    - Error code
    0,				// qab_amount   - Amount
    0,				// qab_handle   - Device handle
    0,				// qab_vector   - Vector for interrupt
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    O_PHYS|O_NOMOUNT,		// qab_option   - Options or command
    0,				// qab_count    - Count
    NULL, 0,			// qab_buffer1  - Pointer to file spec
    0, 0,			// qab_buffer2  - Unused
    &comchar, 0			// qab_parm     - Pointer to parameter area
};

TRMMODES data;         		// Structure used by svcTrmDspMode

char *device;			/* Current device name			*/
int   device_baud    = 2400;	/* Baud rate for current serial device	*/
char  device_parity  = 'E';	/* Parity for current serial device	*/
char  device_stop    = 1;	/* Stop bits for current serial device	*/
char  device_word    = 7;	/* Word size for current serial device	*/
char  device_timeout = TRUE;	/* Timeout retry for both serial and	*/
				/* parellel devices			*/
int   printer_width  = 0;	/* New width for printer		*/
int   printer_height = 0;	/* New lines per inch for printer	*/

int   procarg_state = NEXT_device;	/* Next kind of option expected	*/

void main(argc, argv)
int   argc;
char *argv[];
{
    FILE *fp;
    char  *ptr;

	reg_pib(&pib);

	init_Vars();

    if (argc > 1)
    {
	++argv;
    progarg(argv, 0, options, keywords, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    else
    {
	fputs("\nInvalid number of parameters\007\n", stderr);
	exit(EXIT_INVSWT);
    }

    if ( *device== 'X' )
    {
        exit(EXIT_NORM);
    }
    if (*device == 'L')		/* Printer device specified?	*/
    {
        printf("%s not rerouted\n", device);
        if ((fp = fopen(device, "wb")) == NULL)
        {
            fputs("\nInvalid parameters\007\n", stderr);
            exit(EXIT_INVSWT);
        }
        else
        {
            switch (printer_width)
            {
            case 132:
                printf("\n%s set for 132\n", device);
                fputc(15, fp);
                break;
            case 80:
                printf("\n%s set for 80\n", device);
            default:
                fputc(18, fp);
            }
            switch (printer_height)
            {
            case 6:
                fputs("\nPrinter lines per inch set\n", stdout);
                fputs("\033\040", fp);
                break;
            case 8:
                fputs("\nPrinter lines per inch set\n", stdout);
                fputs("\033\036", fp);
            }
            fclose(fp);
        }
        if (device_timeout == TRUE)
            fputs("\nInfinite retry on parallel printer time-out\n", stdout);
        else
            fputs("\nNo retry on parallel printer time-out\n", stdout);
    }
    else			/* Serial device specified	*/
    {
        if (device_baud == 0)   /* Make sure all values are correct */
            device_baud = 2400;
        if (device_stop == 0)
            device_stop = 1;
        if (device_word == 0)
            device_word = 7;
    /*
     * Now configure the terminal device
     */
        comqab.qab_buffer1   = device;  /* Point to device name */
        comchar.irate.value  = comchar.rate.value  = device_baud;
        comchar.idbits.value = comchar.dbits.value = device_word;
        comchar.isbits.value = comchar.sbits.value = device_stop;
        switch (device_parity)
        {
            case 'N':
            ptr = "NONE";
            break;
            case 'M':
            ptr = "MARK";
            break;
            case 'S':
            ptr = "SPAC";
            break;
            case 'O':
            ptr = "ODD";
            break;
            case 'E':
            ptr = "EVEN";
            break;
            default:
            fputs("\nInvalid parameters\007\n", stderr);
            exit(EXIT_INVSWT);
        }
        comchar.iparity.value = comchar.parity.value = *((long *)ptr);
        if (svcIoQueue(&comqab) < 0)
        {
            fputs("\nInvalid parameters\007\n", stderr);
            exit(EXIT_INVSWT);
        }
        printf("\n%s %d,%c,%d,%d,%c\n", device, device_baud, device_parity,
            device_word, device_stop, device_timeout == TRUE ? 'P' : '-');
    }
}

//***************************************************
// Function: init_Vars - Set up the program information block
// Returned: none
//***************************************************
void init_Vars(void)
{
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


/*
 * Function to handle printer re-direction
 */

int key_lpt1(
    arg_data *arg)

{
    key_lpt('1', arg);
    return (TRUE);
}

int key_lpt2(
    arg_data *arg)

{
    key_lpt('2', arg);
    return (TRUE);
}

int key_lpt3(
    arg_data *arg)

{
    key_lpt('3', arg);
    return (TRUE);
}

void key_lpt(char unitnum, arg_data *arg)

{
    long  rtn;
    char *destination;

    device = "LPT :";
    *(device + 3) = unitnum;

    if ((arg->flags & ADF_XSVAL) == 0)	/* Undo printer re-direction */
    {
	printer_width = (int)arg->data;
	if (printer_width != 0)
	    procarg_state = NEXT_prnheight;
	else
	    procarg_state = NEXT_prnwidth;
    svcIoDefLog(0, 0, device, NULL);
	return;
    }
/*
 * Note: When the printer driver knows about time-out, this needs to be able
 *       to interrogate the driver and display the time-out setting correctly
 */

    if (arg->data != 0)
    {
	fputs("\nInvalid parameters\007\n", stderr);
	exit(EXIT_INVSWT);
    }

    switch((int)arg->val.n)
    {
	case 0:			/* Com1	*/
	case 2:
	    destination = "COM1:";
	    break;
	case 1:			/* Com2	*/
	case 3:
	    destination = "COM2:";
	    break;
    }
    if ((rtn = svcIoDefLog(0, 0, device, destination)) < 0)
	femsg(prgname, rtn, device);
    printf("%s rerouted to %s\n\nNo retry on parallel printer time-out\n", device, destination);
    exit(EXIT_NORM);
}

/*
 * Functions to begin processing of a serial port setup
 */

int key_com1(
    arg_data *arg)

{
   device      = "COM1:";
   device_baud = (int)arg->data;

   if (device_baud)
	procarg_state = NEXT_parity;
   else
	procarg_state = NEXT_baud;
   return (TRUE);
}

int key_com2(
    arg_data *arg)

{
    device      = "COM2:";
    device_baud = (int)arg->data;

    if (device_baud)
	procarg_state = NEXT_parity;
    else
	procarg_state = NEXT_baud;
    return (TRUE);
}

/*
 * Function to set display width
 */

int key_width(
    arg_data *arg)

{
    long rtn;
    if ((rtn = svcTrmDspMode(STDTRM, DM_RTNDATA, &data)) < 0)
        femsg(prgname, rtn, "console");
    data.dm_columns = arg->data;
    if (svcTrmDspMode(STDTRM, rtn|DM_USEDATA, &data) < 0 ||
        svcTrmDspMode(STDTRM, DM_SETBASE, &data) < 0)
    {
	fputs("Invalid parameters\007\n", stderr);
        exit(EXIT_NORM);
    }
    device="X";
    return (TRUE);
}

/*
 * Function to set black/white modes
 */

int key_bw(
    arg_data *arg)

{
    long rtn, bits;

    if ((rtn = svcTrmDspMode(STDTRM, DM_RTNDATA, &data)) < 0)
	femsg(prgname, rtn, "console");
    bits = (rtn & ~(DM_MONOADPT|DM_COLORPALT|DM_NOCLEAR)) |
            (DM_USEDATA|DM_COLORADPT|DM_MONOPALT);
    data.dm_columns = arg->data;
    if (svcTrmDspMode(STDTRM, bits, &data) < 0 ||
        svcTrmDspMode(STDTRM, DM_SETBASE, &data) < 0)
    {
	fputs("Invalid parameters\007\n", stderr);
        exit(EXIT_NORM);
    }
    device="X";
    return (TRUE);
}

/*
 * Function to set color modes
 */

int key_co(
    arg_data *arg)

{
    long rtn, bits;

    if ((rtn = svcTrmDspMode(STDTRM, DM_RTNDATA, &data)) < 0)
	femsg(prgname, rtn, "console");
    bits = (rtn & ~(DM_MONOADPT|DM_MONOPALT|DM_NOCLEAR)) |
            (DM_USEDATA|DM_COLORADPT|DM_COLORPALT);
    data.dm_columns = arg->data;
    if (svcTrmDspMode(STDTRM, bits, &data) < 0 ||
        svcTrmDspMode(STDTRM, DM_SETBASE, &data) < 0)
    {
	fputs("Invalid parameters\007\n", stderr);
        exit(EXIT_NORM);
    }
    device="X";
    return (TRUE);
}

/*
 * Function to set monochrome mode
 */

int key_mono(
    arg_data *arg)

{
    long rtn, bits;

    if ((rtn = svcTrmDspMode(STDTRM, DM_RTNDATA, &data)) < 0)
	femsg(prgname, rtn, "console");
    bits = (rtn & ~(DM_COLORADPT|DM_COLORPALT|DM_NOCLEAR)) |
            (DM_USEDATA|DM_MONOADPT|DM_MONOPALT);
    data.dm_columns = arg->data;
    if (svcTrmDspMode(STDTRM, bits, &data) < 0 ||
        svcTrmDspMode(STDTRM, DM_SETBASE, &data) < 0)
    {
	fputs("Invalid parameters\007\n", stderr);
        exit(EXIT_NORM);
    }
    device="X";
    return (TRUE);
}

int key_wild(
    arg_data *arg)

{
    switch (procarg_state)
    {
	case NEXT_invalid:	/* No further input expected	 */
	    fputs("\nInvalid number of parameters\007\n", stderr);
	    exit(EXIT_INVSWT);
	case NEXT_device:	/* No device name has been found */
        invalid:
	    fprintf(stderr, "\nInvalid parameter '%s'\007\n", arg->name);
	    exit(EXIT_INVSWT);
	case NEXT_baud:		/* COM port baud rate expected next */
	    device_baud   = atoi(arg->name);
	    procarg_state = NEXT_parity;
	    break;
	case NEXT_parity:	/* COM port parity expected next */
	    device_parity = toupper(*arg->name);
	    procarg_state = NEXT_word;
	    break;
	case NEXT_word:		/* COM port word size expected next */
	    device_word   = *arg->name - '0';
	    procarg_state = NEXT_stop;
	    break;
	case NEXT_stop:		/* COM port stop bits expected next */
	    device_stop   = *arg->name - '0';
	    procarg_state = NEXT_timeout;
	    break;
	case NEXT_timeout:	/* COM/LPT port timeout expected next	*/
	    device_timeout = (toupper(*arg->name) == 'P');
	    procarg_state  = NEXT_invalid;
	    break;
	case NEXT_prnwidth:	/* LPT page width expected next	*/
	    printer_width  = atoi(arg->name);
	    if (printer_width != 132 && printer_width != 80)
		goto invalid;
	    procarg_state  = NEXT_prnheight;
	    break;
	case NEXT_prnheight:	/* LPT lines per inch expected next */
	    printer_height = atoi(arg->name);
	    if (printer_height != 8 && printer_height != 6)
		goto invalid;
	    procarg_state  = NEXT_timeout;
    }
    return (TRUE);
}

void opt_help(void)
{
    fprintf(stderr, "\n%s %d.%d (%s) %s\n\n", prgname,
			VERSION, EDITNO, __DATE__, copymsg);
    fputs("The following are supported:\n\n"
          "  MODE 40\n  MODE 80\n  MODE BW40\n  MODE BW80\n  MODE CO40\n"
	  "  MODE CO80\n  MODE LPT#=COMn\n"
	  "  MODE LPT#,w,n,p (w = width, n = cpi, p = no timeout)\n"
          "  MODE COMn:b,p,s,timeout (b = baud, p = parity, s = stop bits, p = no timeout)\n", stderr);
    exit(EXIT_INVSWT);
}

void procarg_error(void)
{
    fputs("Invalid parameters\007\n", stderr);
    exit(EXIT_INVSWT);
}
