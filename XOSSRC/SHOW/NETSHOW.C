/*
 * NEWSHOW - command to display information
 *           about the network
 *
 * Created 6-Aug-92 from SHOW
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

/*
 * Command format:
 *	NETSHOW NAME {ARGUMENT}
 */

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <PROCARG.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSSTR.H>
#include "NETSHOW.H"

#define VERSION 1
#define EDITNO  1

char argument[104];
void (*func)(void);

#define AF(func) (int (*)(arg_data *))func

arg_spec keywords[] =
{
    {"ROU"   , 0        , NULL, AF(hvroute) , 0},
    {"ROUTE" , 0        , NULL, AF(hvroute) , 0},
    {"DNS"   , 0        , NULL, AF(hvdns)   , 0},
    {"*"     , ASF_LSVAL, NULL,    otherarg , 0},
    {NULL    , 0        , NULL, AF(NULL)    , 0}
};

arg_spec options[] =
{   {"?",     0, NULL, AF(help)     , 0},
    {"HELP",  0, NULL, AF(help)     , 0},
    {"H",     0, NULL, AF(help)     , 0},
    {NULL,    0, NULL, AF(NULL)     , 0}
};

int   infocnt;
char  prgname[] = "NETSHOW";
int   validcount;

main(argc, argv)
int   argc;
char *argv[];
{
    if (--argc > 0)
    {
        argv++;
        procarg(argv, 0, options, keywords, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    if (func == NULL)
    {
	fputs("? NETSHOW: Command error, syntax is\n  NETSHOW function\n",
                stderr);
	help();
    }
    else
        func();
    return (0);
}

int otherarg(arg_data *arg)

{
    if (argument[0] != 0)
    {
        fputs("NETSHOW: More than one argument given\n", stderr);
        exit(1);
    }
    if (strlen(arg->name) > 100)
    {
        fputs("NETSHOW: Argument is too long\n", stderr);
        exit(1);
    }
    strmov(argument, arg->name);
    return (TRUE);
}

int hvroute(void)

{
    func = nsroute;
    return (TRUE);
}

int hvdns(void)

{
    func = nsdns;
    return (TRUE);
}

/*
 * Functions to process command-line switches
 */

void help(void)
{
    fprintf(stderr, "NETSHOW - version %d.%d\n", VERSION, EDITNO);
    fputs("\nThe following options may be specified:\n"
	  "  /?      Display this help message\n"
	  "\nThe following functions may be specified:\n\n"
	  "  ROUTE    Show routing table\n", stderr);
    exit(1);
}
