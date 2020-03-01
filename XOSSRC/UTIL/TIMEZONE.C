//--------------------------------------------------------------------------*
// TIMEZONE.C - Program to set time zone and define daylight savings values
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

#include <STDIO.H>
#include <STDLIB.H>
#include <CTYPE.H>
#include <STRING.H>
#include <XOS.H>
#include <XOSTIME.H>
#include <XOSSVC.H>
#include <XOSRTN.H>
#include <XOSERR.H>
#include <PROGARG.H>
#include <PROGHELP.H>

#define VERSION 1
#define EDITNO  0

struct tzone
{   ulong timezone;
    ulong dlstbgntime;
    ulong dlstbgnday;
    ulong dlstendtime;
    ulong dlstendday;
    long  dlstoffset;
    ulong dlstactive;
} tzone;

time_s bgntime;
time_s endtime;

long  quiet     = FALSE;    /* TRUE if no output wanted */
long  mute      = FALSE;
char  haveoffset = FALSE;

char  prgname[] = "TIMEZONE";
char  envname[] = "^XOS^TIMEZONE^OPT";
Prog_Info pib;
char  copymsg[] = "";
char  example[] = "zone {/option}";
char  description[] = "This command specifies the system's time zone and the "
	"parameters for beginning and ending daylight savings time.  The "
	"\"zone\" value must be given and is the signed offset from GMT (UT) "
	"in minutes.  For example, MST would be specified as -420 (-7 hours).  "
	"The standard time zone names may also be used.  The beginning and "
	"ending values for daylight savings time are specified as "
	"\"hh:mm_dd-mon\" or as the string \"NONE\" if daylight savings "
	"time is not used.  For example, \"03:00_20-Oct\".  If beginning "
	"and ending values are not specified, the default is no daylight "
	"savings time.  TIMEZONE is normally executed in the STARTUP.BAT file "
	"when the system is initialized, although it can be executed at any "
	"time.";

#define AF(func) (int (*)(arg_data *))func

long argoffset(char *arg);
long optbegin(arg_data *argdata);
long optend(arg_data *argdata);
long optoffset(arg_data *argdata);

arg_spec options[] =
{
    {"Q*UIET" , ASF_BOOL|ASF_STORE  , NULL,   &quiet    , TRUE,
	    "Display error messages only."},
    {"M*UTE"  , ASF_BOOL|ASF_STORE  , NULL,   &mute     , TRUE,
	    "Do not display any messages."},
    {"B*EGIN" , ASF_VALREQ|ASF_LSVAL, NULL,    optbegin , 0   ,
	    "Specify the beginning day and time for daylight savings time."},
    {"E*ND"   , ASF_VALREQ|ASF_LSVAL, NULL,    optend   , 0   ,
	    "Specify the ending day and time for daylight savings time."},
    {"O*FFSET", ASF_VALREQ|ASF_NVAL , NULL,    optoffset, 0   ,
	    "Specify the daylight savings time offset in minutes.  Default "
	    "value is +60."},
    {"H*ELP"  , 0                   , NULL, AF(opthelp) , 0   ,
	    "Display this message." },
    {"?"      , 0                   , NULL, AF(opthelp) , 0   ,
	    "Display this message." },
    {NULL     , 0                   , NULL, AF(NULL)    , 0   , NULL }
};

main(argc, argv)
int   argc;
char *argv[];

{
    char *foo[2];
    char  strbuf[300];
    char  bgnstr[16];
    char  endstr[16];

    reg_pib(&pib);

    // Set Program Information Block variables

    pib.opttbl = options; 		// Load the option table
    pib.kwdtbl = NULL;
    pib.build = __DATE__;
    pib.majedt = VERSION; 		// Major edit number
    pib.minedt = EDITNO; 		// Minor edit number
    pib.copymsg = copymsg;
    pib.prgname = prgname;
    pib.desc = description;
    pib.example = example;
    pib.errno = 0;
    getTrmParms();
    getHelpClr();

    if(svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
    {
	foo[0] = strbuf;
	foo[1] = '\0';
	progarg(foo, 0, options, NULL, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    if (argc != 0)
        ++argv;
    progarg(argv, PAF_MNORMAL, options, NULL, (int (*)(char *))argoffset,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

    if (haveoffset)
    {
	svcSysDateTime(T_STTZINFO, &tzone);
    }
    else
    {
	svcSysDateTime(T_GTTZINFO, &tzone);
	if (tzone.dlstoffset == 0)
	    printf("Time zone information:\n    Time zone offset = %d "
		    "minutes\n    Daylight savings time is not being used\n",
		    tzone.timezone);
	else
	{
	    bgntime.time = tzone.dlstbgntime;
	    sdt2str(bgnstr, "%h:%m", (time_sz *)&bgntime);
	    endtime.time = tzone.dlstendtime;
	    sdt2str(endstr, "%h:%m", (time_sz *)&endtime);
	    printf("Time zone information:\n"
		    "                  Time zone offset = %d minutes\n"
		    "     Daylight savings begining day = %d\n"
		    "    Daylight savings begining time = %s\n"
		    "       Daylight savings ending day = %d\n"
		    "      Daylight savings ending time = %s\n"
		    "           Daylight savings offset = %d minutes\n"
		    "    Daylight savings time is %snow active\n",
		    tzone.timezone, tzone.dlstbgnday, bgnstr, tzone.dlstendday,
		    endstr, (tzone.dlstactive) ? "" : "not ");
	}
    }
    return (EXIT_NORM);
}

long argoffset(
    char *arg)

{
    char minus;
    char chr;

    if (haveoffset)
    {
	fputs("TIMEZONE: More than one time zone offset value specified\n",
		stderr);
	return (FALSE);
    }
    minus = FALSE;
    if ((chr = *arg) == '-' || chr == '+')
    {
	arg++;
	if (chr == '-')
	    minus = TRUE;
    }
    while ((chr = *arg++) != 0 && isdigit(chr))
	tzone.timezone = tzone.timezone * 10 + (chr & 0x0F);

    if (chr != 0 || tzone.timezone > 1440)
    {
	fputs("TIMEZONE: Illegal time zone offset value specified\n", stderr);
	return (FALSE);
    }
    if (minus)
	tzone.timezone = -tzone.timezone;
    haveoffset = TRUE;
    return (TRUE);
}

long optbegin(
    arg_data *argdata)

{
    argdata = argdata;

    fprintf(stderr, "? TIMEZONE: /BEGIN not implemented yet!\n");
    return (FALSE);
}

long optend(
    arg_data *argdata)

{
    argdata = argdata;

    fprintf(stderr, "? TIMEZONE: /END not implemented yet!\n");
    return (FALSE);
}

long optoffset(
    arg_data *argdata)

{
    argdata = argdata;

    fprintf(stderr, "? TIMEZONE: /OFFSET not implemented yet!\n");
    return (FALSE);
}
