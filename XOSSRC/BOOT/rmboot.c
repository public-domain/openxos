//---------------------------------------------------------------------
// RMBOOT.C - Program to remove XOS bootstrap from a disk
//
// Written by: John R. Goltz
//
// Edit History:
// 08/01/91(brn) - Add quiet switch and help text
//---------------------------------------------------------------------

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
#include <XCSTRING.H>
#include <PROCARG.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSERRMSG.H>
#include <XOSRTN.H>
#include <MKBOOT.H>

#define VERSION 3
#define EDITNO  1

char    copymsg[] = "";
char  diskname[12];		// Name of disk
char  prgname[] = "RMBOOT";	// Name of this program
char  quiet = FALSE;		// TRUE if /QUIET specified
char  disktype;			// Disk type

void fail(char *str1, long code, char *str2);
void help_print(char *help_string, int state, int newline);
int  hvdisk(char *arg);
void notice(char *str);
void notvalid(void);
int  optquiet(arg_data *);
void optusage(void);

// Switch settings functions

#define OPT(name) ((int (*)(arg_data *))name)

arg_spec options[] =
{   {"?"        , 0        , NULL, OPT(optusage), 0},
    {"H"        , 0        , NULL, OPT(optusage), 0},
    {"HEL"      , 0        , NULL, OPT(optusage), 0},
    {"HELP"     , 0        , NULL, OPT(optusage), 0},
    {"Q"        , 0        , NULL,     optquiet , TRUE},
    {"QUI"      , 0        , NULL,     optquiet , TRUE},
    {"QUIET"    , 0        , NULL,     optquiet , TRUE},
    {"NOQ"      , 0        , NULL,     optquiet , FALSE},
    {"NOQUIET"  , 0        , NULL,     optquiet , FALSE},
    {NULL       , 0        , NULL, OPT(NULL)    , FALSE}
};

main(argc, argv)
int   argc;
char *argv[];

{
    if (argc >= 2)
    {
	++argv;
	procarg(argv, 0, options, NULL,	hvdisk,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
        if (!quiet)
            fprintf(stderr, "RMBOOT - version %d.%d\n", VERSION, EDITNO);

    }
    if (diskname[0] == '\0')
    {
        fputs("? RMBOOT: No disk specified\n", stderr);
        exit(1);
    }
    if (rmbootf(diskname, &disktype, fail) != 0)
        exit(1);
    if (!quiet)
	printf("%% RMBOOT: Orignal bootstrap restored to %s %s\n",
            disktype? "hard disk partition": "floppy disk", diskname);
    return (0);
}

int hvdisk(
    char *arg)

{
    char *pnt;

    if (diskname[0])
    {
        fputs("? RMBOOT: More than one disk specified\n", stderr);
        exit(1);
    }
    if (strlen(arg) > 10)
    {
        fputs("? RMBOOT: Illegal disk name\n", stderr);
        exit(1);
    }
    pnt = strmov(diskname, strupr(arg)); // Copy disk name
    if (pnt[-1] != ':')
        *pnt = ':';
    return (TRUE);
}

//*************************************************
// Function: help_print - Print help option entries
// Returned: Nothing
//*************************************************

void help_print(char *help_string, int state, int newline)
{
    char str_buf[132];

    strcpy(str_buf, help_string);
    if (state)
        strcat(str_buf, " *");

    if (newline)
    {
        fprintf(stderr, "%s\n", str_buf);
    }
    else
        fprintf(stderr, "%-38s", str_buf);

}

void optusage(void)

{
    fprintf(stderr, "\n%s version %d.%d (%s) %s\n\n", prgname,
			VERSION, EDITNO, __DATE__, copymsg);
    fprintf(stderr, "%s {/option} {...\}file_specs\n", prgname);
    fprintf(stderr, "\nOptions:\n");
    help_print(" HELP or ? - this message", FALSE, TRUE);
    help_print(" {NO}QUIET - Supress routine output messages", quiet, TRUE);
    fprintf(stderr, "\nA * shows this option is the current default.\n");
    fprintf(stderr, "All options and values may be abbreviated to 1 or 3 letters.\n");
    exit(EXIT_INVSWT);          // Return as if invalid option
}

int optquiet(
    arg_data *arg)

{
    quiet = arg->val.n;
    return (TRUE);
}

void notice(char *str)

{
    puts(str);
}

void fail(char *str1, long code, char *str2)

{
    femsg2(prgname, str1, code, str2);
}
