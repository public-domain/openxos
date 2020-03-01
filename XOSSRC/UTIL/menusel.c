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
#include <procarg.h>

#define VERSION 1
#define EDITNO  0

char *argpnt;
int   prglen;
int   time;
int   dfltnum;
char *itemtbl[9];

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
{	lngstr_parm arglist;
	lngstr_parm devlist;
	char        end;
} runparms =
{	{PAR_SET|REP_STR, 0, IOPAR_RUNCMDTAIL},
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
    R_CHILDTERM|R_CPYENV|R_CPYPTH,
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
int  optdefault(arg_data *arg);
int  optitem(arg_data *arg);
int  optusage(void);
int  opttime(arg_data *arg);
void runprogram(char *str);
void timeout(void);

#define OPT(name) ((int (*)(arg_data *))name)

arg_spec options[] =
{   {"?"      , 0                   , NULL, OPT(optusage) , 0},
    {"HELP"   , 0                   , NULL, OPT(optusage) , 0},
    {"HEL"    , 0                   , NULL, OPT(optusage) , 0},
    {"H"      , 0                   , NULL, OPT(optusage) , 0},
    {"TIME"   , ASF_VALREQ|ASF_NVAL , NULL,     opttime   , 0},
    {"TIM"    , ASF_VALREQ|ASF_NVAL , NULL,     opttime   , 0},
    {"DEFAULT", ASF_VALREQ|ASF_NVAL , NULL,     optdefault, 0},
    {"DEF"    , ASF_VALREQ|ASF_NVAL , NULL,     optdefault, 0},
    {"1"      , ASF_VALREQ|ASF_LSVAL, NULL,     optitem   , 1},
    {"2"      , ASF_VALREQ|ASF_LSVAL, NULL,     optitem   , 2},
    {"3"      , ASF_VALREQ|ASF_LSVAL, NULL,     optitem   , 3},
    {"4"      , ASF_VALREQ|ASF_LSVAL, NULL,     optitem   , 4},
    {"5"      , ASF_VALREQ|ASF_LSVAL, NULL,     optitem   , 5},
    {"6"      , ASF_VALREQ|ASF_LSVAL, NULL,     optitem   , 6},
    {"7"      , ASF_VALREQ|ASF_LSVAL, NULL,     optitem   , 7},
    {"8"      , ASF_VALREQ|ASF_LSVAL, NULL,     optitem   , 8},
    {"9"      , ASF_VALREQ|ASF_LSVAL, NULL,     optitem   , 9},
    {NULL     , 0                   , NULL,     NULL      , 0}
};


main(
	int   argc,
	char *argv[])

{
	int num;

    if (argc >= 2)
    {
		++argv;
		procarg(argv, PAF_EATQUOTE, options, NULL, NULL,
				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
	fputs("\nSelect command:\n\n", stdtrm);
	num = 0;
	do
	{
		if (itemtbl[num] != NULL)
			fprintf(stdtrm, "%5d: %s\n", num + 1, itemtbl[num]);
	} while (++num < 9);

	fprintf(stdtrm, "\nDefault selection will be executed in %d second%s\n",
			time, (time == 1) ? "" : "s");

	setvector(50, 0x04, timeout);
	svcSchAlarm(3, 0, 50, 0, 0, time * XT_SECOND);
	svcSchSetLevel(0);
	getcommand();
}


int optdefault(
	arg_data *arg)

{
	dfltnum = arg->val.n;
	return (TRUE);
}


int optitem(
	arg_data *arg)

{
	char **ipnt;

	ipnt = itemtbl + arg->data - 1;

	if ((*ipnt = (char *)malloc(arg->length + 2)) == NULL)
	{
		fputs("? MENUSEL: Cannot allocate memory for item\n", stderr);
		exit(1);
	}
	strcpy(*ipnt, arg->val.s);
	return (TRUE);
}


int optusage(void)

{

	return (FALSE);
}


int  opttime(
	arg_data *arg)

{
	time = arg->val.n;
	return (TRUE);
}


void getcommand(void)

{
	char *arg;
	char *cpnt;
	int   number;
	char  chr;

	while (TRUE)
	{
		fprintf(stdtrm, "\nEnter number [%d]: ", dfltnum);
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
		if (cmdbufr[0] == 0)
			runprogram(itemtbl[dfltnum - 1]);
		else
		{
			cpnt = cmdbufr;
			number = 0;
			while ((chr = *cpnt++) != 0 && isdigit(chr))
				number = number * 10 + (chr & 0x0F);
			if (chr != 0)
			{
				fputs("? MENUSEL: Not a valid number\n", stdtrm);
				continue;
			}
			if (number < 1 || number > 9 || itemtbl[number - 1] == NULL)
			{
				fputs("? MENUSEL: Invalid selection\n", stdtrm);
				continue;
			}
			runprogram(itemtbl[number - 1]);
		}
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
		printf("%d\n", dfltnum);
		runprogram(itemtbl[dfltnum - 1]);
	}
}


void runprogram(
	char *str)

{
	static char *exttbl[] = {"RUN", "BAT", "EXE", "COM", NULL};

	char **extpnt;
	char  *pnt;
	int    len;
	long   rtn;
	uchar  havedev;
	char   chr;
	char   progspec[200];

	havedev = FALSE;
	pnt = str;
	while ((chr = *pnt) != 0 && !isspace(chr) && chr != '/')
	{
		pnt++;
		if (chr == ':' || chr == '\\')
			havedev = TRUE;
	}
	len = pnt - str;




///	endpnt = progspec + sprintf(progspec, "CMD:%.*s.", endpnt - str, str);




	runqab.qab_buffer1 = progspec;
	runparms.arglist.buffer = str;
	runparms.arglist.strlen = runparms.arglist.bfrlen = strlen(str);
	extpnt = exttbl;
	do
	{
		pnt = progspec;
		if (!havedev)
			pnt = strmov(pnt, "CMD:");
		pnt = strnmov(pnt, str, len);
		*pnt++ = '.';
		strmov(pnt, *extpnt++);

///		printf("### prog: %s\n", progspec);

		if ((rtn = svcIoRun(&runqab)) < 0)
		{
			if (rtn == ER_FILNF)
				continue;
			femsg2(prgname, "Error running program", rtn, NULL);
		}
		exit(0);
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
