// This program automatically installs the console device.  Ths initial
//   version simply installs the standard VGA and keyboard drivers.  A
//   future version will probably support other console devices and will
//   also load the graphics extension driver that matchs the display.

// This program is only intented to be run at startup time.


#include <CTYPE.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <TIME.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <XOSERR.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSTIME.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>
#include <XOSSINFO.H>
#include "LKELOAD.H"


#define VERSION 1
#define EDITNO  0

extern char    lkename[];
extern LKEARGS lkeargs;

struct
{   lngstr_char result;
    uchar       end;
} lkechars =
{   {PAR_GET|REP_STR, 0, "RESULT", 0, 0, 1024, 1024}
};

long data[2];

struct chkchars
{   lngstr_char data;
    char        end;
} chkchars =
{   {PAR_GET|REP_DATAS, 0, "VALUE", data, 0, sizeof(data), sizeof(data)}
};

struct 
{   byte1_char unit;
    text4_char type;
    text8_char adapter;
    byte1_char screen;
    byte2_char ioreg;
    byte2_char kbioreg;
    byte1_char kbint;
    char       end;
} vgachars =
{   {PAR_SET|REP_HEXV, 1, "UNIT"   , 0},
    {PAR_SET|REP_TEXT, 4, "TYPE"   , "VGAA"},
    {PAR_SET|REP_TEXT, 8, "ADAPTER", "COLOR"},
    {PAR_SET|REP_HEXV, 1, "SCREEN" , 1},
    {PAR_SET|REP_HEXV, 2, "IOREG"  , 0x3C0},
    {PAR_SET|REP_HEXV, 2, "KBIOREG", 0x60},
    {PAR_SET|REP_HEXV, 1, "KBINT"  , 1}
};

char trmclass[] = "TRM:";

QAB addqab =
{   QFNC_WAIT|QFNC_CLASSFUNC,	// qab_func     - Function
    0,							// qab_status   - Returned status
    0,							// qab_error    - Error code
    0,							// qab_amount   - Amount
    0,							// qab_handle   - Device handle
    0,							// qab_vector   - Vector for interrupt
    0,	        				// qab_level    - Signal level for direct I/O
    0,  						// qab_prvlevel - Previous signal level (int.)
    CF_ADDUNIT,					// qab_option   - Options or command
    0,							// qab_count    - Count
    trmclass, 0,				// qab_buffer1  - Pointer to file spec
    &vgachars, 0,				// qab_buffer2  - Pointer to characteristics
    NULL, 0						// qab_parm     - Pointer to parameters
};

char  prgname[] = "CONINST";


int lkeload(int quiet, char *name, struct chkchars *chars);


int main(void)

{
    int rtn;

    if (lkeload(0x01, "VGACHK.LKE", &chkchars) != 0)
		return (1);
    if (data[0] == 0)
    {
		fputs("? CONINST: No console adapter found\n", stdout);
		return (1);
    }
    if (lkeload(0, "CONDRV.LKE", NULL) != 0)
		return (1);
    if (lkeload(0, "KBDADRV.LKE", NULL) != 0)
		return (1);
    if (lkeload(0, "VGAADRV.LKE", NULL) != 0)
		return (1);
    if (data[0] != 2)
		strcpy(vgachars.adapter.value, "MONO");
    if ((rtn = svcIoQueue(&addqab)) < 0 || (rtn = addqab.qab_error) < 0)
		femsg2(prgname, "Error adding console device", rtn, NULL);
    return (0);
}

//********************************
// Function: lkeload - Load an LKE
// Returned: 0 if OK, 1 if error
//********************************

int lkeload(
    int    quiet,
    char  *name,
    struct chkchars *chars)

{
    strcpy(lkename+7, name);
    return (lkeloadf(quiet, (struct lkechar *)chars));
}

//*************************************************
// Function: message - Display message for lkeloadf
// Returned: Nothing
//*************************************************

void  message(
    int level,
    char *text)

{
    level = level;

    fputs(text, stdout);
}
