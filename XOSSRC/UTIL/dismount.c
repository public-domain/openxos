//-----------------------------------------------------------------------
//  DISMOUNT.C - XOS utility to dismount a disk drive
//
//  Written by: John R. Goltz
//
//  Edit history:
//  ------------
//  08/20/92(brn) - Add comment header
//  03/16/95(sao) - Added progasst package
//  05/13/95(sao) - Changed 'optional' indicators
//  05/16/95(sao) - Changed exit codes to reflect XOSRTN
//  18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                  optusage() to opthelp().
//  16Jun95 (fpj) - Cleaned up code; got rid up femsg() calls.
//-----------------------------------------------------------------------

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
#include <XOSDISK.H>
#include <XOSERR.H>
#include <XOSRTN.H>
#include <XOSERRMSG.H>
#include <XOSSVC.H>
#include <PROGARG.H>
#include <PROGHELP.H>

#define PROGRAM     DISMOUNT	// Program name
#define VERSION     3		// Program version number
#define EDIT        4		// Program edit number

#define string(x)   stringx(x)
#define stringx(x)  #x

#define ERRBITS     (EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_EXIT)
				// Bits to use for errmsg() calls

char *diskname = NULL;		// Name of disk to dismount

long quiet = FALSE;		// TRUE if /QUIET specified

// Prototypes

static void diserr(int code);
static void getarg(char *argv[]);
static int  getkey(char *keyword);

static arg_spec options[] =
{
    { "Q*UIET", ASF_BOOL | ASF_STORE, NULL, &quiet,
            TRUE,   "No output except error messages" },
    { "H*ELP" , 0                   , NULL, opthelp,
            0,      "Output this message" },
    { "?"     , 0                   , NULL, opthelp,
            0,      "Output this message" },
    { NULL}
};

Prog_Info pib;

char  copymsg[] = "Copyright (c) 1992-1995 Allegro Systems, Ltd.";
char  prgname[] = string(PROGRAM);
char  envname[] = "^ALG^" string(PROGRAM) "^OPT";
char  example[] = "{/options} disk:";
char  description[] = "DISMOUNT removes a disk from the system.  All " \
    "information about the disk is discarded.  If there are any unwritten " \
    "blocks in the disk cache for the disk, they are discarded.  This " \
    "command is normally not needed.  It should never be needed for " \
    "fixed (non-removable) disks.  It can be used with removable disks " \
    "to insure that data does not remain in memory after a disk has been " \
    "changed.  The system normally does this automatically.  This command " \
    "should be used with care.  It should not be issued when any output " \
    "files are open on the disk unless data loss can be tolerated.";

struct diskparm
{   text8_parm  class;
    char        end;
};

static struct diskparm diskparm =
{   { (PAR_SET | REP_TEXT), 8, IOPAR_CLASS, "DISK" },
};

static type_qab diskqab =
{   QFNC_WAIT | QFNC_OPEN,      // qab_open
    0,                          // qab_status
    0,                          // qab_error
    0,                          // qab_amount
    0,                          // qab_handle
    0,                          // qab_vector
    0,                          // qab_level    - Signal level for direct I/O
    0,                          // qab_prvlevel - Previous signal level (int.)
    O_RAW | O_NOMOUNT,          // qab_option
    0,                          // qab_count
    NULL, 0,                    // qab_buffer1
    NULL, 0,                    // qab_buffer2
    &diskparm, 0                // qab_parm
};

//*********************************************************
// Function: main - Main program entry for DISMOUNT utility
// Returned: The usual EXIT_xxx codes
//*********************************************************

int main(
    int   argc,
    char *argv[])

{
    long rval;			// Returned SVC code
    char buffer[80];		// Buffer used for output strings

    (void)argc;

    getarg(++argv);			// Parse the arguments

    diskqab.qab_buffer1 = diskname;

    // First check to see if the disk is real

    if ((rval = svcIoQueue(&diskqab)) < 0 || (rval = diskqab.qab_error) < 0)
    {
	if (rval == ER_PARMV)
	{
	    sprintf(buffer, "Device %s is not a disk", diskname);
	    errmsg(ERRBITS, NULL, 0, buffer, NULL);
        }
	else
	    diserr(rval);
    }

    // Then try to dismount it

    diskqab.qab_func = QFNC_WAIT | QFNC_SPECIAL;
    diskqab.qab_option = DSF_DISMOUNT;

    if ((rval = svcIoQueue(&diskqab)) < 0 || (rval = diskqab.qab_error) < 0)
	diserr(rval);
    if (!quiet)
    {
        if (diskqab.qab_amount)
            printf("Disk %s dismounted\n", diskname);
        else
            printf("Disk %s was not mounted\n", diskname);
    }
    return (0);
}

//**********************************************
// Function: getarg - Get command-line arguments
// Returned: Nothing
//**********************************************

static void getarg(
    char *argv[])

{
    int  rval;
    char buffer[512];

    static char *array[] = { NULL, NULL };

    reg_pib(&pib);                      // FIXME: what does this do?

    // set Program Information Block variables

    pib.majedt = VERSION;               // Version number
    pib.minedt = EDIT;                  // Edit number
    pib.errno = 0;
    pib.copymsg = copymsg;
    pib.prgname = prgname;
    pib.build = __DATE__;
    pib.desc = description;
    pib.example = example;
    pib.opttbl = options;               // Load the option table
    pib.kwdtbl = NULL;

    getTrmParms();                      // FIXME: what does this do?
    getHelpClr();                       // FIXME: what does this do?

    // Check for environment variable first

    rval = svcSysFindEnv(0, envname, NULL, buffer, sizeof(buffer), NULL);

    if (rval > 0)
    {
        array[0] = buffer;
        progarg(array, PAF_EATQUOTE, options, NULL, getkey, NULL, NULL,
                NULL);
    }

    // Process the command-line options

    rval = progarg(argv, PAF_EATQUOTE, options, NULL, getkey, NULL, NULL,
                NULL);

    if (rval < 0)
        errmsg(ERRBITS, NULL, rval, "Option or keyword error", NULL);
    if (diskname == NULL)
        errmsg(ERRBITS, NULL, 0, "No disk specified", NULL);
}

//**************************************************
// Function: getkey - Get general keyword
// Returned: 0 if no error, else negative error code
//**************************************************

static int getkey(
    char *keyword)

{
    char *pnt;
    int   len;

    if (diskname != NULL)
        errmsg(ERRBITS, NULL, 0, "Disk device already specified", NULL);
    len = strlen(keyword);    
    if ((diskname = malloc(len + 2)) == NULL)
    {
	errmsg(ERRBITS, NULL, 0, "Can't get memory for disk name", NULL);
	exit(EXIT_INVSWT);
    }
    strcpy(diskname, keyword);
    if ((pnt = strchr(diskname, ':')) == NULL)
    {
	diskname[len] = ':';
	diskname[len+1] = 0;
    }
    else if (pnt[1] != 0)
    {
	fprintf(stderr, "? DISMOUNT: %s is not a valid disk name\n", keyword);
	exit(1);
    }
    strupr(diskname);
    return (0);
}

//**************************************************************
// Function: diserr - Report general error when dismounting disk
// Returned: Nothing
//**************************************************************

static void diserr(
    int code)

{
    char buffer[80];

    sprintf(buffer, "Error dismounting %s", diskname);
    errmsg(ERRBITS, NULL, code, buffer, NULL);
}
