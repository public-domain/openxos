//--------------------------------------------------------------------------*
// FIXDRIVE.C - XOS utility for setting up DOS-equivilent disk names
//		for fixed disks
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
#include <STRING.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <XOS.H>
#include "FIXDRIVE.H"

#define VERSION 3
#define EDITNO  5

// 3.2 - 6-Dec-94
//	Divided into DOSDRIVE and DOSDRVF so the dosdrivef function (from
//	DOSDRVF) could be included in other programs (mainly INSTALL).
// 3.3 - 26-Mar-99
//	Changed name of DOSDRVF to DOSDRIVEF and moved it to LIBX (was in
//	LIBC)
// 3.4 - 18-Oct-99
//	Changed names of DOSDRIVE and DOSDRIVEF to FIXDRIVE and FIXDRIVEF

void error(long, char *, char *);
int  optxcld(arg_data *arg);

long      quiet = FALSE;
long      mute = FALSE;
XCLDLIST *xlist;

arg_spec options[] =
{
    {"Q*UIET", ASF_BOOL|ASF_STORE  , NULL, &quiet     , TRUE,
	    "Limit output to error messages." },
    {"M*UTE" , ASF_BOOL|ASF_STORE  , NULL, &mute      , TRUE,
	    "No output messages." },
	{"X*CLD" , ASF_VALREQ|ASF_LSVAL, NULL,    optxcld , 0,
		"Exclude drive from primary partition scan"},
    {"?"     , 0                   , NULL, AF(opthelp), 0,
	    "This message" },
    {"H*ELP" , 0                   , NULL, AF(opthelp), 0,
	    "This message" },
    {NULL    , 0                   , NULL, AF(NULL)   , 0,
	    NULL }
};

Prog_Info pib;
char  prgname[] = "FIXDRIVE";
char  copymsg[] = "";
char  example[] = "{/option}";
char  description[] = "This command automatically assigns DOS style drive "
	"letters to the standard system disks.  This includes the first two "
	"floppy drives, all fixed IDE disk drives and all fixed SCSI disk "
	"drives.  This command mostly duplicates the scheme used by DOS to "
	"assign drive letters, but there may be differences with some "
	"unusual configurations.  This command ignores any currently "
	"assigned drive letters, so it should be executed before any "
	"drive letters are assigned manually.  It is normally executed from "
	"the STARTUP.BAT file immediately following the DISKINST command, "
	"but it can be executed at any time.";


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
		progarg(argv, 0, options, NULL, NULL, (void (*)(char *, char*))NULL,
				(int (*)(void))NULL, NULL);
    }
    return (fixdrivef(quiet, xlist));
}

void fixdrivemessage(
    int   type,
    char *text)

{
    type = type;

    if ( !mute )
		fprintf(stderr, "%sFIXDRIVE: %s", (type >= FDML_ERROR)? "? ": "", text);
}



int  optxcld(
	arg_data *arg)

{
	XCLDLIST *xpnt;

	xpnt = getspace(sizeof(XCLDLIST) + 1 + arg->length);
	strcpy(xpnt->drive, arg->val.s);
	if (xpnt->drive[arg->length - 1] != ':')
	{
		xpnt->drive[arg->length] = ':';
		xpnt->drive[arg->length + 1] = 0;
	}
	xpnt->next = xlist;
	xlist = xpnt;
	return (TRUE);
}
