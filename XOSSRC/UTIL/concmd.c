//---------------------------------------------------------
// concmd.c
// Program to conditionally execute a command with time-out
//
// Written by: John R. Goltz
//
// Edit History:
// 28-Oct-02 (JRG) - 1.0 - First version
//---------------------------------------------------------

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
//   CONCMD timeout defaultcmd {arg1 {arg2 {...}}}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <xos.h>
#include <xostime.h>
#include <xosvect.h>
#include <xoserr.h>
#include <xossvc.h>
#include <xosrtn.h>
#include <xoserrmsg.h>
#include <xosstr.h>

#define VERSION 1
#define EDITNO  0

char *argpnt;
int   prglen;

char  cmdbufr[200];

struct
{   long src;
    long dst;
    long cmd;
    long xxx;
} devlist[] =
{	{DH_STDIN , DH_STDIN},
	{DH_STDOUT, DH_STDOUT},
	{DH_STDERR, DH_STDERR},
	{DH_STDTRM, DH_STDTRM}
};

struct
{   lngstr_parm arglist;
    lngstr_parm devlist;
    char        end;
} runparms =
{   {PAR_SET|REP_STR, 0, IOPAR_RUNCMDTAIL},
    {PAR_SET|REP_STR, 0, IOPAR_RUNDEVLIST, (char *)devlist, 0,
            sizeof(devlist), 0}
};

type_qab runqab =
{
    RFNC_WAIT|RFNC_RUN,			// qab_func     - Function
    0,							// qab_status   - Returned status
    0,							// qab_error    - Error code
    0,							// qab_amount   - Amount
    0,							// qab_handle   - Device handle
    0,							// qab_vector   - Vector for interrupt
    0,	        				// qab_level    - Signal level for direct I/O
    0,  						// qab_prvlevel - Previous signal level (int.)
    R_SAMEPROC|R_CPYENV|R_ALLDEV|R_CPYPTH,
								// qab_option   - Options or command
    0,							// qab_count    - Count
    NULL, 0,					// qab_buffer1  - Pointer to file spec
    NULL, 0,					// qab_buffer2  - Unused
    &runparms, 0				// qab_parm     - Pointer to parameter area
};


struct
{	byte4_parm time;
	uchar      end;
} conparms =
{	{PAR_SET|REP_DECV, 4, IOPAR_TIMEOUT}
};

char  prgname[] = "CONCMD";
uchar haveinput = FALSE;;

void getcommand(void);
void runprogram(char *argpnt, int prglen);
void timeout(void);


mainalt(
	img_data *argp)

{
	char *cpnt;
	int   time;
	char  chr;

	if (strtok(argp->cmdtail, " \t") == NULL ||
			(argpnt = strtok(NULL, " \t")) == NULL)
		badcmd();
	time = 0;
	while ((chr = *argpnt++) != 0 && isdigit(chr))
		time = time * 10 + (chr & 0x0F);
	if (chr != 0)
	{
		fputs("? CONCMD: Illegal time-out value\n", stderr);
		exit(1);
	}
	if ((argpnt = strtok(NULL, "")) == NULL)
		badcmd();
	cpnt = argpnt;
	while ((chr = *cpnt) != 0 && !isspace(chr) && chr != '/')
		cpnt++;
	if ((prglen = cpnt - argpnt) > 170)
	{
		fputs("? CONCMD: Program name is too long\n", stderr);
		exit(1);
	}
	fprintf(stdtrm, "\nDefault command \"%s\" will be executed in %d second%s"
			"\n", argpnt, time, (time == 1) ? "" : "s");
	setvector(50, 0x04, timeout);
	svcSchAlarm(3, 0, 50, 0, 0, time * XT_SECOND);
	svcSchSetLevel(0);
	getcommand();
}


void getcommand(void)

{
	char *arg;
	char *cpnt;
	int   len;
	char  chr;

	while (TRUE)
	{
		fputs("\nCommand: ", stdtrm);
		fgets(cmdbufr, sizeof(cmdbufr), stdtrm);
		haveinput = TRUE;
		cpnt = cmdbufr;						// Remove possible final newline
		arg = NULL;
		while ((chr = *cpnt) != 0 && chr != '\n')
		{
			if (arg == NULL && !isspace(chr))
				arg = cpnt;
			cpnt++;
		}
		*cpnt = 0;
		if (arg != NULL)
		{
			cpnt = arg;
			while ((chr = *cpnt) != 0 && !isspace(chr) && chr != '/')
				cpnt++;

			if ((len = cpnt - arg) > 170)
			{
				fputs("? CONCMD: Program name is too long\n", stderr);
				continue;
			}
			runprogram(arg, len);
		}
		else
			runprogram(argpnt, prglen);
	}
}


void timeout(void)

{
	static struct
	{	byte4_parm sts;
		uchar      end;
	} stsparms =
	{	{PAR_GET|REP_HEXV, 4, IOPAR_INPSTS}
	};

	long rtn;

	if ((rtn = svcIoInBlockP(DH_STDIN, NULL, 0, &stsparms)) < 0)
		femsg2(prgname, "Error getting input status", rtn, NULL);
	if (!haveinput && (stsparms.sts.value & 0x01) == 0)
	{
		printf("%s\n", argpnt);
		runprogram(argpnt, prglen);
	}
}


void runprogram(
	char *arg,
	int   len)

{
	static char *exttbl[] = {"RUN", "BAT", "EXE", "COM", NULL};

	char **extpnt;
	char  *endpnt;
	long   rtn;
	char   progspec[200];

	endpnt = progspec + sprintf(progspec, "CMD:%.*s.", len, arg);
	runqab.qab_buffer1 = progspec;
	runparms.arglist.buffer = arg;
	runparms.arglist.strlen = runparms.arglist.bfrlen = strlen(arg);
	extpnt = exttbl;
	do
	{
		strcpy(endpnt, *extpnt++);
		if ((rtn = svcIoRun(&runqab)) < 0)
		{
			if (rtn == ER_FILNF)
				continue;
			femsg2(prgname, "Error running program", rtn, NULL);
		}
		fputs("? CONCMD: Run SVC returned\n", stderr);
		exit(1);
	} while (*extpnt != NULL);
	fputs("\n? CONCMD: Illegal command\n", stdtrm);
	getcommand();
	exit(0);
}


void badcmd(void)

{
	fputs("\n? CONCMD: Incorrect syntax, correct usage is\n"
			"            CONCMD timeout command {args ...}\n"
			"          \"timeout\" value is in seconds.\n", stderr);
	exit(1);
}
