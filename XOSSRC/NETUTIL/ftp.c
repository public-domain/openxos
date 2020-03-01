//--------------------------------------------------------------------------*
// FTP.C
// FTP client for XOS
//
// Written by: John Goltz
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

#include <CTYPE.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <TIME.H>
#include <SIGNAL.H>
#include <XOSERR.H>
#include <XOSTRM.H>
#include <XOSRTN.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSTIME.H>
#include <XOSERRMSG.H>
#include <XOSNET.H>
#include <XOSFTPFUNC.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <GLOBAL.H>
#include <XOSSTR.H>

#define VERSION 1
#define EDITNO  0

#define VECT_OPEN 30


#define NAMEMAX  1024

typedef struct filedescp FILEDESCP;

typedef struct cmdtbl CMDTBL;

char *hostaddr;
char *cmdpnt;

int   trmcnt;
char *trmpnt;

char  trmbfr[128];
char  atom[64];
uchar prompt = 0x01;

time_s starttime;
time_s stoptime;
int    totalbytes;

long  cmdhndl = -1;
long  datahndl = -1;
long  filehndl = -1;
int   debug = FALSE;

long  rspnum;
char  rspstr[256];

long  have150;

int   tcpavl;

long  openflag;

char *typestr = "A N";

char *tcppnt;

char  cmdbufr[256];
char  xferbufr[1024];

char  lclpath[512] = "";

char  datadev[32];

char  lclname[NAMEMAX];

struct
{   byte4_parm  rmtport;
    byte4_parm  lclport;
    char        end;
} cmdopnparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_NETRMTPORTS, TCPP_FTPCMD},
    {PAR_GET|REP_HEXV, 4, IOPAR_NETLCLPORT}
};

struct
{   byte4_parm  rmtport;
    byte4_parm  lclport;
    char        end;
} dataopnparms =
{   {PAR_GET|REP_HEXV, 4, IOPAR_NETRMTPORTR},
    {PAR_SET|REP_HEXV, 4, IOPAR_NETLCLPORT}
};

QAB dataqab =
{   QFNC_OPEN,					// qab_open
    0,							// qab_status
    0,							// qab_error
    0,							// qab_amount
    0,							// qab_handle
    VECT_OPEN,					// qab_vector
    0,	        				// qab_level
    0,  						// qab_prvlevel
    0,							// qab_option
    0,							// qab_count
    datadev, 0,					// qab_buffer1
    NULL, 0,					// qab_buffer2
    &dataopnparms, 0			// qab_parm
};

long    cmdappend(char *arg);
long    cmddebug (char *arg);
long    cmddir(char *arg);
long    cmdget(char *arg);
long    cmdhelp(char *arg);
long    cmdlcd(char *arg);
long    cmdls(char *arg);
long    cmdmget(char *arg);
long    cmdmput(char *arg);
long    cmdopen(char *arg);
long    cmdprompt(char *arg);
long    cmdput(char *arg);
long    cmdquit(char *arg);
long    cmdquote(char *arg);
long    cmdrhelp(char *arg);
long    cmdren(char *arg);
long    cmduser(char *arg);
int     datatofile(char *buffer, int size);
int     datatouser(char *buffer, int size);
void    errormsg(char *msg, int code);
CMDTBL *getcmd(void);
char   *getname(char **str);
long    getxfer(char *bufr, int size);
int     nonopt(char *arg);
void    notify(char *str);
void    opendone(void);
void    opthelp(void);
long    putxfer(char *bufr, int size);
int     setuplocal(char *name);
long    trmxfer(char *bufr, int size);


#define OPT(name) ((int (*)(arg_data *))name)

arg_spec options[] =
{	{"DEB*UG", ASF_BOOL|ASF_STORE, NULL, &debug , 1, "Run in debug mode"},
	{"H*ELP" , 0                 , NULL, opthelp, 0, "Display this message"},
    {"?"     , 0                 , NULL, opthelp, 0, "Display this message"},

    {NULL}
};

// FTP command table

struct cmdtbl
{   char  *name;
    long (*func)(char *arg);
    char  *help;
};

char helptext[] =
		"Valid commands are:\n\n"
		"   ?      ALLFILES APPEND ASCII      BINARY BYE    CD   CLOSE\n"
		"   DELETE DEBUG    DIR    DISCONNECT GET    HELP   LCD  LITERAL\n"
		"   LS     MGET     MKDIR  MPUT       OPEN   PROMPT PUT  PWD\n"
		"   QUIT   USER     RECV   REMOTEHELP RENAME RMDIR  SEND STATUS\n"
		"   TYPE\n\n"
		"For a description of a command, type \"HELP cmdname\"\n\n";

char msgallfls[] =
		"Toggle all-files mode (XOS specific command, user must have\r\n"
		"              the FTPALL privilege available)";

char msgdir[] = "Display directory listing";
char msgget[] = "Transfer file from the remote system";
char msgput[] = "Transfer file to the remote system";
char msghelp[] = "Display help text";
char msgclose[] = "Terminate the network connection";
char msgquit[] = "Quit FTP";


CMDTBL cmdtbl[] =
{   {"?"          , cmdhelp    , msghelp},
    {"AL*LFILES"  , ftpAllFiles, msgallfls},
    {"AP*PEND"    , cmdappend  , "Append to remote file"},
    {"AS*CII"     , ftpAscii   , "Set transfer type to ASCII non-print (AN)"},
    {"BI*NARY"    , ftpBinary  , "Set transfer type to Image (I)"},
    {"BY*E"       , cmdquit    , msgquit},
    {"CD*"        , ftpCd      , "Change remote directory path"},
    {"CL*OSE"     , ftpClose   , msgclose},
    {"DEL*ETE"    , ftpDelete  , "Delete remote file"},
    {"DEB*UG"     , cmddebug   , "Toggle debug mode"},
    {"DIR*"       , cmddir     , msgdir},
    {"DIS*CONNECT", ftpClose   , msgclose},
    {"G*ET"       , cmdget     , msgget},
    {"HE*LP"      , cmdhelp    , msghelp},
    {"LC*D"       , cmdlcd     , "Change local directory path"},
    {"LI*TERAL"   , ftpLiteral , "Send literal command"},
    {"LS*"        , cmdls      , "List remote directory"},
    {"MG*ET"      , cmdmget    , "Transfer multiple files from the remote system"},
    {"MK*DIR"     , ftpMkDir   , "Create directory on remote system"},
    {"MP*UT"      , cmdmput    , "Transfer multiple files to the remote system"},
    {"O*PEN"      , cmdopen    , "Establish network connection"},
    {"PR*OMPT"    , cmdprompt  , "Toggle prompt mode"},
    {"PU*T"       , cmdput     , msgput},
    {"PW*D"       , ftpPWD     , "Display current remote directory path"},
    {"QUI*T"      , cmdquit    , msgquit},
    {"QUO*TE"     , cmdquote   , "????"},
    {"REC*V"      , cmdget     , msgget},
    {"REM*OTEHELP", cmdrhelp   , "Display help text from remote system"},
    {"REN*AME"    , cmdren     , "Rename file on remote system"},
    {"RM*DIR"     , ftpRmDir   , "Remove directory on remote system"},
    {"SE*ND"      , cmdput     , msgput},
    {"ST*ATUS"    , ftpStatus  , "Display current status"},
    {"TY*PE"      , ftpType    , "Set transfer type (ASCII or BINARY)"},
    {"U*SER"      , ftpUser    , "Specify new user name"},
    {NULL}
};

// Misc. variables

Prog_Info pib;
char copymsg[] = "";
char prgname[] = "FTP";			// Our programe name
char envname[] = "^XOS^FTP^OPT"; // The environment option name
char example[] = "{/options} {hostaddr}";
char description[] = "This command implements an FTP client.  If a host "
		"address is specified, a command connection is established to that host.";
char synerr[] = "? Incorrect syntax in network address\n";


void main(
    int   argc,
    char *argv[])

{
    CMDTBL *tbl;
    char   *envpnt[2];
    long    rtn;
    char    strbuf[256];

    // Set defaults

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

    // Check Global Configuration Parameters

    global_parameter(TRUE);

    // Check Local Configuration Parameters

    if(svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
    {
		envpnt[0] = strbuf;
		envpnt[1] = NULL;
		progarg(envpnt, 0, options, NULL, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    // Process the command line

    if (argc >= 2)
    {
		++argv;
		progarg(argv, 0, options, NULL, nonopt,
				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

///	setvector(VECT_OPEN, (VT_XOSS<8)+4, opendone);
///	svcSchSetLevel(0);					// Enable signals

	ftpInit(&rspnum, rspstr, 2000, debug);

    if (hostaddr != NULL)
    {
		if ((rtn = cmdopen(hostaddr)) < 0)
			errormsg("Error connecting", rtn);
    }
    trmcnt = 0;
    while (TRUE)
    {
		fputs("FTP> ", stdout);
		if (!gettrm())
		{

		}
		if ((tbl = getcmd()) == NULL)
			fputs("? Invalid command\n", stdout);
		else
		{
			skipws();
			if ((rtn = (tbl->func)(cmdpnt)) < 0)
				errormsg("Error sending command", rtn);
			else if (rspstr[0] != 0)
			{
				printf("%s\n", rspstr);
				rspstr[0] = 0;
			}
		}
    }
}



int gettrm(void)

{
    char *pnt;
    char  chr;

    pnt = cmdbufr;
    for (;;)
    {
		if (--trmcnt < 0)
		{
			if ((trmcnt = svcIoInBlock(DH_STDIN, trmbfr, sizeof(trmbfr))) < 0)
				return (FALSE);
			trmpnt = trmbfr;
			while (--trmcnt >= 0)
			{
				if ((chr = *trmpnt++) == '\n')
				{
					*pnt = 0;
					cmdpnt = cmdbufr;
					return (TRUE);
				}
				if (chr != '\r')
				{
					if ((pnt - cmdbufr) > (sizeof(cmdbufr) - 2))
					{
						// COMPLAIN ABOUT LONG LINE HERE!!!!

						return (FALSE);
					}
					*pnt++ = chr;
				}
			}
		}
    }
}

//********************************************
// Function: nonopt - Process non-option input
// Returned: Nothing
//********************************************

int nonopt(
    char *arg)

{
    int size;

    if (hostaddr != NULL)
    {
		fputs(synerr, stdout);
		exit(1);
    }
    size = strlen(arg);
    hostaddr = getspace(size + 2);
    strcpy(hostaddr, arg);
    return (TRUE);
}

//****************************************************************
// Function: getcmd - Get command keyword and search command table
// Returned: Pointer to command table entry or NULL if bad command
//****************************************************************

CMDTBL *getcmd(void)

{
    char   *pnt1;
    char   *pnt2;
    CMDTBL *tbl;
    int     cnt;
    char    chr;
    char    tc;
    char    exact;

    skipws();
    pnt1 = atom;
    cnt = sizeof(atom) - 2;
    while ((chr = toupper(*cmdpnt)) != 0 && !isspace(chr))
    {
		cmdpnt++;
		if (--cnt <= 0)
			return (NULL);
		*pnt1++ = chr;
    }
    *pnt1 = 0;
    tbl = cmdtbl;
    do
    {
		pnt1 = atom;
		pnt2 = tbl->name;
		exact = TRUE;
		do
		{
			chr = *pnt1++;
			if ((tc = *pnt2++) == '*')
			{
				exact = FALSE;
				tc = *pnt2++;
			}
		} while (chr != 0 && chr == tc);
		if (chr == 0 && (tc == 0 || !exact))
			return (tbl);
	} while ((++tbl)->name != NULL);
	return (NULL);
}

void skipws(void)

{
    char chr;

    while ((chr = *cmdpnt) != 0 && isspace(chr))
		cmdpnt++;
}


//*************************************************
// Function: cmdopen - Process the OPEN FTP command
// Returned: Nothing
//*************************************************

long cmdopen(
	char *arg)

{
	long rtn;

	if ((rtn = ftpOpen(arg)) < 0)
		return (rtn);
	printf("%s\n", rspstr);

///	printf("### After OPEN, response = %d |%s|\n", rspnum, rspstr);

    if (rspnum == 220)					// Need a user name?
    {									// Yes
		fputs("User name for remote system: ", stdout);
		gettrm();
		if ((rtn = ftpUser(cmdpnt)) < 0)
		{
			errormsg("Error sending user name", rtn);
			return (0);
		}
		printf("%s\n", rspstr);
		if (rspnum == 331)				// Need a password?
		{								// Yes
			fputs("Password for remote system: ", stdout);
			svcTrmFunction(DH_STDIN, TF_DSECHO);
			gettrm();
			fputs("\n", stdout);
			svcTrmFunction(DH_STDIN, TF_ENECHO);
			if ((rtn = ftpPassword(cmdpnt)) < 0)
			{
				errormsg("Error sending password", rtn);
				return (0);
			}
			printf("%s\n", rspstr);

///			printf("### After PSWD, response = %d |%s|\n", rspnum, rspstr);
		}
	}
	return (0);
}

//*****************************************************
// Function: cmdappend - Process the APPEND FTP command
// Returned: Nothing
//*****************************************************

long cmdappend(
	char *arg)

{
	arg = arg;

	return (ER_NIYT);
}

//***************************************************
// Function: cmddebug - Process the DEBUG FTP command
// Returned: Nothing
//***************************************************

long cmddebug (
	char *arg)

{
	arg = arg;

    debug ^= 0x01;
	ftpDebug(debug);
    printf("%% Debug mode is %s\n", (debug) ? "enabled" : "disabled");
	return (0);
}

//***********************************************
// Function: cmdlcd - Process the LCD FTP command
// Returned: Nothing
//***********************************************

long cmdlcd(
	char *arg)

{
	arg = arg;

	return (ER_NIYT);
}

//***********************************************
// Function: cmddir - Process the DIR FTP command
// Returned: Nothing
//***********************************************

long cmddir(
	char *arg)

{
	return (ftpDir(arg, notify, trmxfer, xferbufr, sizeof(xferbufr), TRUE));
}

//*********************************************
// Function: cmdls - Process the LS FTP command
// Returned: Nothing
//*********************************************

long cmdls(
	char *arg)

{
	return (ftpLs(arg, notify, trmxfer, xferbufr, sizeof(xferbufr), TRUE));
}

//***********************************************
// Function: cmdget - Process the GET FTP command
// Returned: Nothing
//***********************************************

long cmdget(
	char *arg)

{
    char *lclpnt;
    char *rmtpnt;
	long  rtn;

	rmtpnt = getname(&arg);
	lclpnt = getname(&arg);
    if (lclpnt == NULL)
		lclpnt = rmtpnt;
    if (!setuplocal(lclpnt))
		return (0);
	if (debug)
		printf("ftp: Opening local file %s for writing\n", lclname);
	if ((filehndl = svcIoOpen(O_OUT|O_TRUNCA|O_CREATE, lclname, NULL)) < 0)
	{
		errormsg("Error opening local file", filehndl);
		return (0);
	}
	if ((rtn = ftpGet(rmtpnt, notify, getxfer, xferbufr, sizeof(xferbufr),
			TRUE)) < 0)
	{
		svcIoClose(filehndl, 0);
		return (rtn);
	}
	if ((rtn = svcIoClose(filehndl, 0)) < 0)
		errormsg("Error closing local file", rtn);
	return (0);
}

//*************************************************
// Function: cmdhelp - Process the HELP FTP command
// Returned: Nothing
//*************************************************

long cmdhelp(
	char *arg)

{
    CMDTBL *tbl;
    char   *pnt1;
    char   *pnt2;
    char    chr;

	arg = arg;

    if (atend())
		fputs(helptext, stdout);
    else
    {
		if ((tbl = getcmd()) == NULL)
			printf("? %s is not a valid command\n\n", atom);
		else
		{
			pnt1 = tbl->name;
			pnt2 = atom;
			do
			{
				if ((chr = *pnt1++) != '*')
					*pnt2++ = chr;
			} while (chr != 0);
			printf("   %s:  %s\n\n", atom, tbl->help);
		}
    }

	return (0);
}

//*************************************************
// Function: cmdmget - Process the MGET FTP command
// Returned: Nothing
//*************************************************

long cmdmget(
	char *arg)

{
	arg = arg;

	return (ER_NIYT);
}

//*************************************************
// Function: cmdmput - Process the MPUT FTP command
// Returned: Nothing
//*************************************************

long cmdmput(
	char *arg)

{
	arg = arg;

	return (ER_NIYT);
}

//*****************************************************
// Function: cmdprompt - Process the PROMPT FTP command
// Returned: Nothing
//*****************************************************

long cmdprompt(
	char *arg)

{
	arg = arg;

    prompt ^= 0x01;
    printf("%% Prompt mode is %s\n", (prompt) ? "enabled" : "disabled");
	return (0);
}

//***********************************************
// Function: cmdput - Process the PUT FTP command
// Returned: Nothing
//***********************************************

long cmdput(
	char *arg)

{
    char *lclpnt;
    char *rmtpnt;
	long  rtn;

	lclpnt = getname(&arg);
	rmtpnt = getname(&arg);
    if (rmtpnt == NULL)
		rmtpnt = lclpnt;
    if (!setuplocal(lclpnt))
		return (0);
	if (debug)
		printf("ftp: Opening local file %s for reading\n", lclname);
	if ((filehndl = svcIoOpen(O_IN, lclname, NULL)) < 0)
	{
		errormsg("Error opening local file", filehndl);
		return (0);
	}
	rtn = ftpPut(rmtpnt, notify, putxfer, xferbufr, sizeof(xferbufr), TRUE);
	svcIoClose(filehndl, 0);
	return (rtn);
}

//*************************************************
// Function: cmdquit - Process the QUIT FTP command
// Returned: Nothing
//*************************************************

long cmdquit(
	char *arg)

{
	arg = arg;

    exit(0);
	return (0);
}

//***************************************************
// Function: cmdquote - Process the QUOTE FTP command
// Returned: Nothing
//***************************************************

long cmdquote(
	char *arg)

{
	arg = arg;

	return (ER_NIYT);
}

//*************************************************
// Function: cmdhelp - Process the HELP FTP command
// Returned: Nothing
//*************************************************

long cmdrhelp(
	char *arg)

{
	arg = arg;

	return (ER_NIYT);
}

//**************************************************
// Function: cmdren - Process the RENAME FTP command
// Returned: Nothing
//**************************************************

long cmdren(
	char *arg)

{
    char *pnt;
    char  chr;

	arg = arg;

    pnt = cmdpnt;
    while ((chr = *pnt) != 0 && !isspace(chr))
		pnt++;
    if (chr == 0)
    {
		fputs("? No new name specified\n", stdout);
		return (0);
    }
    *pnt = 0;

	return (ER_NIYT);
}

//******************************************************
// Function: atend - Determine if at end of command line
// Returned: TRUE if at end, FALSE if not
//******************************************************

int atend(void)

{
    skipws();
    	return (*cmdpnt == 0);
}

//**********************************************************
// Function: getname - Get a file name from the command line
// Returned: Pointer to first charater in name
//**********************************************************

char *getname(
	char **str)

{
    char *begin;
	char *pnt;
    char  chr;
    char  quote;

    quote = 0;
	pnt = *str;

	while (isspace(*pnt))
		pnt++;
    begin = pnt;
    while ((chr = *pnt) != 0)
    {
		if (chr == '"')
			quote ^= 0x01;
		else
		{
			if (!quote && isspace(chr))
				break;
		}
		pnt++;
    }
    if (begin == pnt)
		begin = NULL;
    *pnt++ = 0;
	*str = pnt;
    return (begin);
}

//*******************************************************
// Function: setuplocal - Set up local file specification
// Returned: TRUE if OK, FALSE if error
//*******************************************************

int setuplocal(
    char *name)

{
    if (name == NULL)
    {
		fputs("? No file name specified\n", stdout);
		return (FALSE);
    }
    strmov(strmov(lclname, lclpath), name);
    return (TRUE);
}

//**********************************************
// Function: dispstat - Display transfer statics
// Returned: Nothing
//**********************************************

void dispstat(void)

{
    int    millisec;
    long   result[2];
    time_s xfertime;

    sdtsub(&xfertime, &stoptime, &starttime);
    millisec = xfertime.time/XT_MILLISEC;
    printf("%% %d byte%s transfered in %d.%03.3d second%s", totalbytes,
	    (totalbytes == 1) ? "" : "s", millisec/1000, millisec % 1000,
	    (millisec == 1000) ? "" : "s");
    if (totalbytes != 0 && xfertime.time != 0)
		longdiv(result, totalbytes, XT_SECOND, xfertime.time);
    else
		result[0] = 0;
    if (result[0] != 0)
		printf(" (%u byte%s/sec)", result[0], (result[0] != 1) ? "s" : "");
    fputs("\n", stdout);
}

//*****************************************************
// Function: trmxfer - Called when have input data that
//				is to be displayed on the terminal
// Returned: 0 if normal, negative error code if error
//*****************************************************

long trmxfer(
	char *bufr,
	int   size)

{
	fwrite(bufr, 1, size, stdout);
	return (0);
}

long getxfer(
	char *bufr,
	int   size)

{
	return (svcIoOutBlock(filehndl, bufr, size));
}

long putxfer(
	char *bufr,
	int   size)

{
	long rtn;

	if ((rtn = svcIoInBlock(filehndl, bufr, size)) == ER_EOF)
		rtn = 0;
	return (rtn);
}


void notify(
	char *str)

{
	printf("%s\n", str);
	str[0] = 0;
}

//*******************************************
// Function: errormsg - Display error message
// Returned: Nothing
//*******************************************

void errormsg(
    char *msg,
    int   code)

{
    char bufr[100];

    svcSysErrMsg(code, 0x03, bufr);
    printf("? %s\n  %s\n\n", msg, bufr);
}
