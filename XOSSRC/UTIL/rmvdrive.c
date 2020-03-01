//--------------------------------------------------------------------------*
// RMVDRIVE.C - XOS utility for setting up DOS-equivilent disk names for
//		removable disks
//
// Written by: Tom Goltz, John Goltz
//
// Edit History:
// 05/12/94(brn) - Fix command abbreviations and version number for 32 bit
// 03/17/95(sao) - Added progasst package
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
#include <PROGARG.H>
#include <PROGHELP.H>
#include <XOS.H>
#include "RMVDRIVE.H"

#define VERSION 1
#define EDITNO  0

RMVSPEC spec = {{0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}};

int  cdltr(arg_data *);
int  cdmax(arg_data *);
void error(long, char *, char *);
int  ltrcheck(int ltr, char *msg);
int  syqltr(arg_data *);
int  syqmax(arg_data *);
int  zipltr(arg_data *);
int  zipmax(arg_data *);
int  lsltr(arg_data *);
int  lsmax(arg_data *);
int  othltr(arg_data *);
int  othmax(arg_data *);

long quiet = FALSE;
long mute = FALSE;

arg_spec options[] =
{
    {"Q*UIET", ASF_BOOL|ASF_STORE, NULL, &quiet     ,TRUE,
	    "Limit output to error messages." },
    {"M*UTE" , ASF_BOOL|ASF_STORE, NULL, &mute      ,TRUE,
	    "No output messages." },
    {"?"      , 0,                 NULL, AF(opthelp),0   ,
	    "Display this message" },
    {"H*ELP" , 0,                 NULL, AF(opthelp),0   ,
	    "Display this message" },
    {NULL}
};

arg_spec keywords[] =
{
    {"CDLTR" , ASF_VALREQ|ASF_SSVAL, NULL, cdltr , 0,
	    "First letter to assign to CDROM drives"},
    {"CDMAX" , ASF_VALREQ|ASF_NVAL , NULL, cdmax , 0,
	    "Maximum number of CDROM drives"},
    {"SYQLTR", ASF_VALREQ|ASF_SSVAL, NULL, syqltr, 0,
	    "First letter to assign to Syquest drives"},
    {"SYQMAX", ASF_VALREQ|ASF_NVAL , NULL, syqmax, 0,
	    "Maximum number of Syquest drives" },
    {"ZIPLTR", ASF_VALREQ|ASF_SSVAL, NULL, zipltr, 0,
	    "First letter to assign to ZIP drives"},
    {"ZIPMAX", ASF_VALREQ|ASF_NVAL , NULL, zipmax, 0,
	    "Maximum number of ZIP drives"},
    {"LSLTR" , ASF_VALREQ|ASF_SSVAL, NULL, lsltr , 0,
	    "First letter to assign to LS-120 drives"},
    {"LSMAX" , ASF_VALREQ|ASF_NVAL , NULL, lsmax , 0,
	    "Maximum number of LS-120 drives"},
    {"OTHLTR", ASF_VALREQ|ASF_SSVAL, NULL, othltr, 0,
	    "First letter to assign to LS-120 drives"},
    {"OTHMAX", ASF_VALREQ|ASF_NVAL , NULL, othmax, 0,
	    "Maximum number of other drives"},
    {NULL}
};

Prog_Info pib;
char  prgname[] = "RMVDRIVE";
char  copymsg[] = "";
char  example[] = "{/option}";
char  description[] = "This command automatically assigns DOS style drive "
	"letters to removable disks.  It allows the specification of the "
	"first letter to assign for each of the following classes of disks: "
	"CDROMs, Syquest disks, ZIP disks, and LS-120 disks.  If a first "
	"letter is not specified for a type of disk, no letters are assigned "
	"for type type of disk.  If no maximum number of disks is specified, "
	"a value of 2 is assumed.  If there is more than one of a type of "
	"disk, letters are assigned in order of the unit numbers, with IDE "
	"disks coming before SCSI disks.  This command ignores any currently "
	"assigned drive letters, so it should be executed before any drive "
	"letters are assigned manually.  It is normally executed from the "
	"STARTUP.BAT file immediately following the FIXDRIVE command, but it "
	"can be executed at any time.";


int main(
    int   argc,
    char *argv[])

{
    reg_pib(&pib);
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
    if (argc > 1)			// Is there an arg to process?
    {
	argv++;
	progarg(argv, 0, options, keywords, NULL, (void (*)(char *, char*))NULL,
                (int (*)(void))NULL, NULL);
    }
    return (rmvdrivef(quiet, &spec));
}


//************************************************
// Function: cdltr - Process the CDLTR keyword
// Returned: TRUE always (does not return if error
//************************************************

int cdltr(
    arg_data *arg)

{
    spec.cd.ltr = ltrcheck(arg->val.n, "CD");
    return (TRUE);
}

//********************************************
// Function: cdmax - Process the CDMAX keyword
// Returned: TRUE always
//********************************************

int cdmax(
    arg_data *arg)

{
    spec.cd.max = arg->val.n;
    return (TRUE);
}

//************************************************
// Function: syqltr - Process the SYQLTR keyword
// Returned: TRUE always (does not return if error
//************************************************

int syqltr(
    arg_data *arg)

{
    spec.syq.ltr = ltrcheck(arg->val.n, "SYQ");
    return (TRUE);
}

//**********************************************
// Function: syqmax - Process the SYQMAX keyword
// Returned: TRUE always
//**********************************************

int syqmax(
    arg_data *arg)

{
    spec.syq.max = arg->val.n;
    return (TRUE);
}

//************************************************
// Function: zipltr - Process the ZIPLTR keyword
// Returned: TRUE always (does not return if error
//************************************************

int zipltr(
    arg_data *arg)

{
    spec.zip.ltr = ltrcheck(arg->val.n, "ZIP");
    return (TRUE);
}

//**********************************************
// Function: zipmax - Process the ZIPMAX keyword
// Returned: TRUE always
//**********************************************

int zipmax(
    arg_data *arg)

{
    spec.zip.max = arg->val.n;
    return (TRUE);
}

//************************************************
// Function: lsltr - Process the LSLTR keyword
// Returned: TRUE always (does not return if error
//************************************************

int lsltr(
    arg_data *arg)

{
    spec.ls.ltr = ltrcheck(arg->val.n, "LS");
    return (TRUE);
}

//********************************************
// Function: lsmax - Process the LSMAX keyword
// Returned: TRUE always
//********************************************

int lsmax(
    arg_data *arg)

{
    spec.ls.max = arg->val.n;
    return (TRUE);
}

//************************************************
// Function: othltr - Process the OTHLTR keyword
// Returned: TRUE always (does not return if error
//************************************************

int othltr(
    arg_data *arg)

{
    spec.oth.ltr = ltrcheck(arg->val.n, "OTH");
    return (TRUE);
}

//**********************************************
// Function: othmax - Process the OTHMAX keyword
// Returned: TRUE always
//**********************************************

int othmax(
    arg_data *arg)

{
    spec.oth.max = arg->val.n;
    return (TRUE);
}

//********************************************
// Function: ltrcheck - Check for valid letter
// Returned: The letter
//********************************************

int ltrcheck(
    int   ltr,
    char *msg)

{
    if (ltr >= 'a' && ltr <= 'z')
	ltr += ('A' - 'a');
    if (ltr < 'A' || ltr > 'Y')
    {
	printf("? RMVDRIVE: Value for the keyword %sLTR is not a letter "
		"between A and Y\n", msg);
	exit (1);
    }
    return (ltr);
}


void rmvdrivemessage(
    int   type,
    char *text)

{
    type = type;

    if ( !mute )
        fprintf(stderr, "%sRMVDRIVE: %s", (type >= RDML_ERROR)? "? ": "", text);
}

