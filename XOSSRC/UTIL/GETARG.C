//------------------------------------------------------------------------------
//
//  GETARG.C - Process command-line options for DISMOUNT utility
//
//  Copyright (c) 1995 Allegro Systems, Ltd. All rights reserved.
//
//
//  Edit history:
//  -------------
//  15Jun95 (fpj) - Original creation.
//  16Jun95 (fpj) - Fixed bug in getkey().
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algerr.h>
#include <algrtn.h>
#include <algsvc.h>
#include <progarg.h>
#include <proghelp.h>
#include <algermsg.h>

#include "dismount.h"

// Here are our configuration options

char *disk_name = NULL;                 // Name of disk to dismount

long quiet = FALSE;                     // TRUE if /QUIET specified
long mute = FALSE;                      // TRUE if /MUTE specified

// Local prototypes

static int getkey(char *keyword);


static arg_spec options[] =
{
    { "Q*UIET",     ASF_BOOL | ASF_STORE,   NULL, &quiet,
            TRUE,   "No output except error messages" },
    { "M*UTE",      ASF_BOOL | ASF_STORE,   NULL, &mute,
            TRUE,   "No output at all (return status only)" },
    { "H*ELP",      0,                      NULL, opthelp,
            0,      "Output this message" },
    { "?",          0,                      NULL, opthelp,
            0,      "Output this message" },
    { NULL,         0,                      NULL, NULL,
            0,      NULL },
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


//
//  Function: getarg() - Get command-line arguments.
//
//  Return: (n/a)
//
//------------------------------------------------------------------------------

void getarg(char *argv[])
{
    int rval;
    char buffer[512];
    char *array[] = { NULL, NULL };

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
    {
        errmsg(ERRBITS, NULL, rval, "Option or keyword error", NULL);
    }

    if (disk_name == NULL)
    {
        errmsg(ERRBITS, NULL, 0, "No disk specified", NULL);
    }
}


//
//  Function: getkey() - Get general keyword.
//
//  Return: 0 if no error, else negative error code.
//
//------------------------------------------------------------------------------

static int getkey(char *keyword)
{
    if (disk_name != NULL)
        errmsg(ERRBITS, NULL, 0, "Disk device already specified", NULL);

    disk_name = malloc(strlen(keyword) + 1);

    if (disk_name == NULL)
    {
        if (!quiet)
            errmsg(ERRBITS, NULL, 0, "Can't get memory for disk name", NULL);

        exit(EXIT_INVSWT);
    }

    strcpy(disk_name, keyword);

    return 0;
}
