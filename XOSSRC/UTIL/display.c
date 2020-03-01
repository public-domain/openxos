//--------------------------------------------------------------------------*
// DISPLAY.C
// Utiliy to change the display characteristics
//
// Written by: John R. Goltz
//
// Edit History:
// 08/20/92(brn) - Add comment header
// 05/12/94(brn) - Fix abreviations in options
// 03/16/95(sao) - Added the progasst package
// 05/13/95(sao) - Changed 'optional' indicator
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
#include <XOSTRM.H>
#include <XOSSVC.H>
#include <PROGARG.H>
#include <PROGHELP.H>

#define VERSION 3
#define EDITNO  3
Prog_Info pib;

void help_print(char *help_string, int state, int newline);
void fatal(long, char *);
int  havebios(arg_data *);
//// int  haveblink(arg_data *);
int  havecols(arg_data *);
int  havehoriz(arg_data *);
int  havemode(arg_data *);
int  haveclear(arg_data *arg);
int  haveload(arg_data *arg);
int  haverows(arg_data *);
//// int  haveunln(arg_data *);
int  havevert(arg_data *);
void help(void);
int  quiet(arg_data *);
int  noquiet(arg_data *);
int  verbose(arg_data *);

SubOpts colmon[] =
{
    { "C*OLOR", "Set to color" },
    { "M*ONOCHROME", "Set to mono" },
    { NULL, NULL }
};

SubOpts onoff[] =
{   { "OFF"," " },
    { "ON"," " },
    { NULL, NULL }
};

#define AF(func) (int (*)(arg_data *))func
long quietflg = FALSE;      // TRUE if /QUIET seen
long verboseflg = FALSE;    // TRUE if /VERBOSE seen
long mute = FALSE;

arg_spec keywords[] =
{   {"T*EXT"     , 0                   , NULL  ,    havemode  , DM_TEXT, "Set text mode"},
    {"BI*OS"     , ASF_VALREQ|ASF_NVAL , NULL  ,    havebios  , 0, "Set bios mode"},
////    {"BL*INK"    , ASF_VALREQ|ASF_XSVAL, onoff ,    haveblink , 0, "Turn blink on or off"},
////    {"U*NDERLINE", ASF_VALREQ|ASF_XSVAL, onoff ,    haveunln  , 0, "Turn underline on or off" },
    {"L*OAD"     , ASF_BOOL            , NULL  ,    haveload  , TRUE, "Load"},
    {"CL*EAR"    , ASF_BOOL            , NULL  ,    haveclear , TRUE, "Clear display on change"},
    {"R*OWS"     , ASF_VALREQ|ASF_NVAL , NULL  ,    haverows  , 0, "Set number of text rows on display"},
    {"C*OLUMNS"  , ASF_VALREQ|ASF_NVAL , NULL  ,    havecols  , 0, "Set number of text colomns" },
    {"V*ERT"     , ASF_VALREQ|ASF_NVAL , NULL  ,    havevert  , 0, "Set vertical resolution value"},
    {"H*ORIZ"    , ASF_VALREQ|ASF_NVAL , NULL  ,    havehoriz , 0, "Set horizontal resolution value"},
    {NULL        , 0                   , NULL  , AF(NULL)     , 0, NULL}
};

arg_spec options[] =
{
    {"M*UTE"    , 0, NULL, &mute       , TRUE, "No output.  Error level returned."},
    {"Q*UIET"   , 0, NULL, &quietflg   , TRUE, "Only error messages displayed."},
    {"V*ERBOSE" , 0, NULL, &verboseflg , TRUE, "Detailed messages output."},
    {"H*ELP"    , 0, NULL, AF(opthelp),    0, "This message."},
    {"?"        , 0, NULL, AF(opthelp),    0, "This message."},
    {NULL       , 0, NULL, AF(NULL)    ,    0, NULL }
};

char *modetbl[] =
{   "????",
    "TEXT",	// DM_TEXT    = 1
    "CGA2",	// DM_CGA2C   = 2
    "CGA4",	// DM_CGA4C   = 3
    "EGA2",	// DM_EGA2C   = 4
    "EGA4",	// DM_EGA4C   = 5
    "EGA16",	// DM_EGA16C  = 6
    "VGA4",	// DM_VGA4C   = 7
    "VGA16",	// DM_VGA16C  = 8
    "VGA256C"	// DM_VGA256C = 9
};

unsigned long bits;
unsigned long biosvalue;
TRMMODES data;

char prgname[] = "DISPLAY";
Prog_Info pib;
char  copymsg[] = "";
char  example[] = "{/option} {attribute{=value}} ...";
char  description[] = "This program is used to display/modify the characteristics of the display.";


//
// Function: main - Main program
// Returned: Never returns
//

main( int argc, char **argv)
{
    long bios;
    long rtn;
    int  mode;


	reg_pib(&pib);

	init_Vars();

    argc--;
    argv++;
    progarg(argv, 0, options, keywords, (int (*)(char *))NULL,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

    if (biosvalue != 0)
    {
        if (bits != 0)
        {
            if ( !mute )
                fputs("? DISPLAY: No other values can be specified with \"BIOS\"\n",
                   stderr);
            exit(EXIT_INVSWT);
        }
        bits = biosvalue | DM_BIOSMODE;
    }
    if (bits != 0 && (char)bits == 0)
    {
        if ((rtn = svcTrmDspMode(STDTRM, 0, NULL)) < 0)
            fatal(rtn, "Error getting current display mode");
        bits |= (rtn & 0xFF);
    }
    if ((rtn = svcTrmDspMode(STDTRM, bits | DM_RTNDATA, &data)) < 0)
        fatal(rtn, "Error setting display mode");
    if ((bios = svcTrmDspMode(STDTRM, DM_SETBASE, NULL)) < 0)
        fatal(bios, "Error making display mode permanent");
    if ((bits & 0xFF) == 0 || verboseflg)
    {
        if (bits & DM_BIOSMODE)		// Did we set a BIOS mode?
        {				// Yes - we need normal values
            if ((rtn = svcTrmDspMode(STDTRM, DM_RTNDATA, &data)) < 0)
                fatal(rtn, "Error getting new display mode");
        }
        if ((bios = svcTrmDspMode(STDTRM, DM_BIOSMODE|0xFF, NULL)) < 0)
            fatal(bios, "Error getting BIOS mode value");
        mode = (int)(rtn & 0xFF);
        if (mode > sizeof(modetbl)/sizeof(char *))
            mode = 0;
    if (!quietflg && !mute )
	{
            printf("Mode    = %-6s BIOS    = 0x%X\n", modetbl[mode],
                (char)bios);
            printf("Rows    = %l-5d  Columns = %l-5d  "
               "Vert  = %l-5d Horiz     = %ld\n",
                data.dm_rows, data.dm_columns, data.dm_vert, data.dm_horiz);
	}
    }
    return (EXIT_NORM);
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

//
// Function: havemode - Process mode keyword
//		(called by procarg)
// Returned: TRUE
//

int havemode(
    arg_data *arg)		// Address of argument data

{
    if ((char)bits != 0 && (char)bits != arg->data)
    {					// Already have a mode specified?
        fputs("? DISPLAY: Conflicting modes specified\n", stderr);
        exit(EXIT_INVSWT);
    }
    bits |= arg->data;
    return (TRUE);
}

//
// Function: havebios - Process BIOS keyword
//		(called by procarg)
// Returned: TRUE
//

int havebios(
    arg_data *arg)		// Address of argument data

{
    biosvalue = arg->val.n;
    if (biosvalue == 0 || biosvalue > 0x7E)
        biosvalue = 0x7E;
    return (TRUE);
}

//
// Function: haveblink - Process BLINK keyword
//		(called by procarg)
// Returned: TRUE
//

/*

int haveblink(
    arg_data *arg)		// Addres of argument data

{
    bits |= (arg->val.n != 0)? DM_BLINKON: DM_BLINKOFF;
    return (TRUE);
}

//
// Function: haveunln - Process UNDERLINE keyword
//		(called by procarg)
// Returned: TRUE
//

int haveunln(
    arg_data *arg)		// Address of argument data

{
    bits |= (arg->val.n != 0)? DM_UNDERON: DM_UNDEROFF;
    return (TRUE);
}

*/

//
// Function: haveload - Process {NO}LOAD keyword
//		(called by procarg)
// Returned: TRUE
//

int haveload(arg_data *arg)

{
    if (arg->data)
	bits &= ~DM_NOFONT;
    else
	bits |= DM_NOFONT;
    return (TRUE);
}

//
// Function: haveclear - Process {NO}CLEAR keyword
//		(called by procarg)
// Returned: TRUE
//

int haveclear(arg_data *arg)

{
    if (arg->data)
	bits &= ~DM_NOCLEAR;
    else
	bits |= DM_NOCLEAR;
    return (TRUE);
}

//
// Function: haverows - Process ROWS keyword
//		(called by procarg)
// Returned: TRUE
//

int haverows(
    arg_data *arg)		// Address of argument data

{
    data.dm_rows = arg->val.n;
    bits |= DM_USEDATA;
    return (TRUE);
}

//
// Function: havecols - Process COLUMNS keyword
//		(called by procarg)
// Returned: TRUE
//

int havecols(
    arg_data *arg)		// Address of argument data

{
    data.dm_columns = arg->val.n;
    bits |= DM_USEDATA;
    return (TRUE);
}

//
// Function: havevert - Process VERTICAL keyword
//		(called by procarg)
// Returned: TRUE
//

int havevert(
    arg_data *arg)		// Address of argument data

{
    data.dm_vert = arg->val.n;
    bits |= DM_USEDATA;
    return (TRUE);
}

//
// Function: havehoriz - Process HORIZONTAL keyword
//		(called by procarg)
// Returned: TRUE
//

int havehoriz(
    arg_data *arg)		// Address of argument data

{
    data.dm_horiz = arg->val.n;
    bits |= DM_USEDATA;
    return (TRUE);
}


//
// Function: fatal - Display error message and terminate
// Returned: Never returns
//

void fatal(code, msg)
long  code;			// System error code
char *msg;			// Address of message

{
    char bfr[100];

    svcSysErrMsg(code, 3, bfr);    // Get error message
    if ( !mute )
        fprintf(stderr, "? DISPLAY: %s\n           %s\n", msg, bfr);
    exit(EXIT_SVCERR);
}

