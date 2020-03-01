//**************************************************
// Global configuration parameter processing routine
// Written by Tom Goltz
//**************************************************

// ++++
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
#include <CTYPE.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <PROGHELP.H>

#include <GLOBAL.H>

extern char prgname[];

/*
 * The following variables may be used by any program linking
 * with this module:
 */

int gcp_dosdrive = TRUE;	// Use DOS-type device names for output*/
int gcp_dosquirk = TRUE;	// Follow DOS quirks exactly

SubOpts list_tf[] =
{
	{"OFF",""}, {"ON",""},
	{"N*O",""} , {"Y*ES",""},
	{"F*ALSE",""}, {"T*RUE",""},
	{NULL,NULL }
};

int    do_drive(arg_data *);  // Routine to process keywords
int    do_quirk(arg_data *);  // Routine to process keywords
static void do_errors(char *, char *); // Routine to display errors

static arg_spec keywords[] =
{
	{"DOSDRIVE", ASF_XSVAL, list_tf, do_drive, 1, 0},
	{"DOSQUIRK", ASF_XSVAL, list_tf, do_quirk, 1, 0},
    {NULL,       0,         NULL,      NULL,       0}
};

static int report;
static int rtnval;

/****************************************************************/
/* Function: global_parameter - Process global parameters	*/
/* Returned: Number of errors					*/
/****************************************************************/

int global_parameter(
    int reportarg)		// 0 = Don't report errors
				// 1 = Report errors as warnings
				// 2 = Report errors as errors

{
    char *argv[2];
    char  buffer[512];

    report = reportarg;
    rtnval = 0;
    if(svcSysFindEnv(0, "^XOS^GCP", NULL, buffer, 512, NULL) > 0)
    {
	argv[0] = buffer;
	argv[1] = 0;
	progarg(argv, 0, NULL, keywords, (int (*)(char *))NULL,
		do_errors, (int (*)(void))NULL, NULL);
    }
    return (rtnval);
}

int do_drive(arg_data *arg)
{
	gcp_dosdrive = (arg->val.n) & 1; // Store TRUE if odd, FALSE if
    return (TRUE);			      //   even
}

int do_quirk(arg_data *arg)
{
	gcp_dosquirk = (arg->val.n) & 1; // Store TRUE if odd, FALSE if
    return (TRUE);			      //   even
}

static void do_errors(
    char *msg1,
    char *msg2)

{
    ++rtnval;
    if (report != 0)
    {
        fprintf(stderr, "%c %s: %s\n    %*swhile processing global "
                "configuration parameters\n", (report == 1)? '%': '?',
                prgname, msg1, strlen(prgname), "");
        if (msg2)
            fprintf(stderr, "    %.*s%s\n", strlen(prgname), "", msg2);
    }
}
