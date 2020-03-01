/*
 * SHOW - command to display information
 *           about the system
 *
 * Modified 1/18/91 to use procarg / tmg
 */

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

// Command format:
//	show name

#include <STDIO.H>
#include <STDLIB.H>
#include <CTYPE.H>
#include <STRING.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <PROCARG.H>
#include <XOSERRMSG.H>
#include "SHOW.H"

#define VERSION 1
#define EDITNO  1

#define AF(func) (int (*)(arg_data *))func

arg_spec keywords[] =
{
    {"DCS"   , 0, NULL, AF(dcsinfo) , 0},
    {"DEV"   , 0, NULL, AF(devinfo) , 0},
    {"DEVICE", 0, NULL, AF(devinfo) , 0},
    {"LKE"   , 0, NULL, AF(lkeinfo) , 0},
    {"DISK"  , 0, NULL, AF(diskinfo), 0},
    {"INT"   , 0, NULL, AF(intinfo) , 0},
    {"IO"    , 0, NULL, AF(ioinfo)  , 0},
    {"CRASH" , 0, NULL, AF(crshinfo), 0},
    {NULL    , 0, NULL, AF(NULL)    , 0}
};

void help(void);

arg_spec options[] =
{   {"?",     0, NULL, AF(help)     , 0},
    {"HELP",  0, NULL, AF(help)     , 0},
    {"H",     0, NULL, AF(help)     , 0},
    {NULL,    0, NULL, AF(NULL)     , 0}
};

int   infocnt;
char  prgname[] = "SHOW";
int   validcount;

main(argc, argv)
int   argc;
char *argv[];
{
    validcount = 0;		/* Nothing valid specified	*/
    argc--;
    argv++;
    procarg(argv, 0, options, keywords, (int (*)(char *))NULL,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

    if (validcount == 0)
    {
	fputs("? SHOW: Command error, syntax is\n  SHOW function\n", stderr);
	help();
    }
    return (0);
}

/*
 * Functions to process command-line switches
 */

void help(void)
{
    fprintf(stderr, "SHOW - version %d.%d\n", VERSION, EDITNO);
    fputs("\nThe following options may be specified:\n\n"
	  "  /?      Display this help message\n"
	  "The following functions may be specified:\n\n"
	  "  DCS      Show disk cache status\n"
	  "  DEVICE   Show device information\n"
	  "  DISK     Show disk information\n"
          "  INT      Show interrupt usage\n"
	  "  LKE      Show Loadable Kernal Extensions\n", stderr);
    exit(1);
}

/***************************************************************************
 * Function: getinfo - Get system information
 * Returned: Pointer to information block
 ***************************************************************************/

void *getinfo(
    int func,		/* svcSysGetInfo function */
    int size)		/* Size of one returned information item */

{
    void *blk;
    long  rtn;

    for (;;)
    {
        if ((rtn=svcSysGetInfo(func, 0, NULL, 0)) < 0)
            femsg(prgname, rtn, NULL);	/* See how much room we need */
        rtn = (rtn + 10) * size;
        if ((blk = malloc((int)rtn)) == NULL)
        {				/* Get space for the data */
            fputs("? SHOW: Not enough memory available\n", stderr);
            exit(1);
        }
        if ((rtn=svcSysGetInfo(func, 0, blk, rtn)) < 0) /* Get the data */
            femsg(prgname, rtn, NULL);
        if ((rtn & 0x40000000L) == 0)	/* Was it truncated? */
        {
            infocnt = (int)rtn;		/* No - finished */
            return (blk);
        }
        free(blk);			/* Yes - give up the block and */
    }					/*   try again */
}
