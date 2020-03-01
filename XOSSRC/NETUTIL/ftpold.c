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
uchar debug;
uchar prompt = 0x01;

time_s starttime;
time_s stoptime;
int    totalbytes;

int   rspnum;
long  cmdhndl = -1;
long  datahndl = -1;
long  filehndl = -1;

long  have150;

int   tcpavl;

long  openflag;

char *typestr = "A N";

char *tcppnt;
char  tcpbufr[256];
char  rspbufr[256];

char  trmbufr[256];
char  cmdbufr[256];

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
{   QFNC_OPEN,			// qab_open
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    0,				// qab_handle
    VECT_OPEN,			// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    0,				// qab_option
    0,				// qab_count
    datadev, 0,			// qab_buffer1
    NULL, 0,			// qab_buffer2
    &dataopnparms, 0		// qab_parm
};

void    cmdallfls(void);
void    cmdappend(void);
void    cmdascii(void);
void    cmdbinary(void);
void    cmdcd(void);
void    cmdclose (void);
void    cmddel(void);
void    cmddebug (void);
void    cmddir(void);
void    cmdget(void);
void    cmdhelp(void);
void    cmdlcd(void);
void    cmdlit(void);
void    cmdls(void);
void    cmdmget(void);
void    cmdmkdir(void);
void    cmdmput(void);
void    cmdopen(void);
void    cmdprompt(void);
void    cmdput(void);
void    cmdpwd(void);
void    cmdquit(void);
void    cmdquote(void);
void    cmdrhelp(void);
void    cmdren(void);
void    cmdrmdir(void);
void    cmdsts(void);
void    cmdtype(void);
void    cmduser(void);
int     datatofile(char *buffer, int size);
int     datatouser(char *buffer, int size);
void    errormsg(char *msg, int code);
CMDTBL *getcmd(void);
char   *getname(void);
int     nonopt(char *arg);
void    opendone(void);
void    opthelp(void);
int     setuplocal(char *name);


#define OPT(name) ((int (*)(arg_data *))name)

arg_spec options[] =
{   {"H*ELP"   , 0                   , NULL   ,     opthelp   ,
	    0, "Display this message" },
    {"?"       , 0                   , NULL   ,     opthelp   ,
	    0, "Display this message"},
    {NULL      , 0                   , NULL   ,     NULL      ,
	    0, NULL}
};

// FTP command table

struct cmdtbl
{   char *name;
    void (*func)(void);
    char *help;
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
{   {"?"          , cmdhelp  , msghelp},
    {"AL*LFILES"  , cmdallfls, msgallfls},
    {"AP*PEND"    , cmdappend, "Append to remote file"},
    {"AS*CII"     , cmdascii , "Set transfer type to ASCII non-print (AN)"},
    {"BI*NARY"    , cmdbinary, "Set transfer type to Image (I)"},
    {"BY*E"       , cmdquit  , msgquit},
    {"CD*"        , cmdcd    , "Change remote directory path"},
    {"CL*OSE"     , cmdclose , msgclose},
    {"DEL*ETE"    , cmddel   , "Delete remote file"},
    {"DEB*UG"     , cmddebug , "Toggle debug mode"},
    {"DIR*"       , cmddir   , msgdir},
    {"DIS*CONNECT", cmdclose , msgclose},
    {"G*ET"       , cmdget   , msgget},
    {"HE*LP"      , cmdhelp  , msghelp},
    {"LC*D"       , cmdlcd   , "Change local directory path"},
    {"LI*TERAL"   , cmdlit   , "????"},
    {"LS*"        , cmdls    , "List remote directory"},
    {"MG*ET"      , cmdmget  , "Transfer multiple files from the remote system"},
    {"MK*DIR"     , cmdmkdir , "Create directory on remote system"},
    {"MP*UT"      , cmdmput  , "Transfer multiple files to the remote system"},
    {"O*PEN"      , cmdopen  , "Establish network connection"},
    {"PR*OMPT"    , cmdprompt, "Toggle prompt mode"},
    {"PU*T"       , cmdput   , msgput},
    {"PW*D"       , cmdpwd   , "Display current remote directory path"},
    {"QUI*T"      , cmdquit  , msgquit},
    {"QUO*TE"     , cmdquote , "????"},
    {"REC*V"      , cmdget   , msgget},
    {"REM*OTEHELP", cmdrhelp , "Display help text from remote system"},
    {"REN*AME"    , cmdren   , "Rename file on remote system"},
    {"RM*DIR"     , cmdrmdir , "Remove directory on remote system"},
    {"SE*ND"      , cmdput   , msgput},
    {"ST*ATUS"    , cmdsts   , "Display current status"},
    {"TY*PE"      , cmdtype  , "Set transfer type (ASCII or BINARY)"},
    {"U*SER"      , cmduser  , "Specify new user name"},
    {NULL}
};

// Misc. variables

Prog_Info pib;
char    copymsg[] = "";
char    prgname[] = "FTP";	// Our programe name
char    envname[] = "^XOS^FTP^OPT"; // The environment option name
char    example[] = "{/options} {hostaddr}";
char    description[] = "This command implements an FTP client.  If a host "\
    "address is specified, a command connection is established to that host.";
char    synerr[] = "? Incorrect syntax in network address\n";


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
    setvector(VECT_OPEN, (VT_XOSS<8)+4, opendone);
    svcSchSetLevel(0);			// Enable signals
    if (hostaddr != NULL)
    {
	cmdpnt = hostaddr;
	if ((rtn = connect()) < 0)
	    exit(1);
    }
    trmcnt = 0;
    for (;;)
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
	    (tbl->func)();
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

//*****************************************************
// Function: cmdappend - Process the APPEND FTP command
// Returned: Nothing
//*****************************************************

void cmdappend(void)

{

}

//***************************************************
// Function: cmdascii - Process the ASCII FTP command
// Returned: Nothing
//***************************************************

void cmdascii(void)

{
    typestr = "A N";			// Remember the type
    if (sendtype())			// Send type now if connected
	fputs("% Transfer type set to ASCII (non-print)\n", stdout);
}

//*****************************************************
// Function: cmdbinary - Process the BINARY FTP command
// Returned: Nothing
//*****************************************************

void cmdbinary(void)

{
    typestr = "I";			// Remember the type
    if (sendtype())			// Send type now if connected
	fputs("% Transfer type set to Image\n", stdout);
}

//*********************************************
// Function: cmdcd - Process the CD FTP command
// Returned: Nothing
//*********************************************

void cmdcd(void)

{
    if (!checkcon())
	return;
    sendcmd("CWD ", cmdpnt);
    response(FALSE);
}

//***************************************************
// Function: cmdclose - Process the CLOSE FTP command
// Returned: Nothing
//***************************************************

void cmdclose (void)

{

}

//**************************************************
// Function: cmddel - Process the DELETE FTP command
// Returned: Nothing
//**************************************************

void cmddel(void)

{
    if (!checkcon())
	return;
    sendcmd("DELE ", cmdpnt);
    response(FALSE);
}

//***************************************************
// Function: cmddebug - Process the DEBUG FTP command
// Returned: Nothing
//***************************************************

void cmddebug (void)

{
    debug ^= 0x01;
    printf("%% Debug mode is %s\n", (debug) ? "enabled" : "disabled");
}

//***********************************************
// Function: cmddir - Process the DIR FTP command
// Returned: Nothing
//***********************************************

void cmddir(void)

{
    if (!checkcon())
	return;
    xferdata("LIST ", datatouser);
}

//***********************************************
// Function: cmdget - Process the GET FTP command
// Returned: Nothing
//***********************************************

void cmdget(void)

{
    char *lclpnt;
    char *rmtpnt;

    if (!checkcon())
	return;
    rmtpnt = getname();
    lclpnt = getname();
    if (lclpnt == NULL)
	lclpnt = rmtpnt;
    if (setuplocal(lclpnt))
    {
	cmdpnt = rmtpnt;
	xferdata("RETR ", datatofile);
    }
}

//*************************************************
// Function: cmdhelp - Process the HELP FTP command
// Returned: Nothing
//*************************************************

void cmdhelp(void)

{
    CMDTBL *tbl;
    char   *pnt1;
    char   *pnt2;
    char    chr;

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
}

//***********************************************
// Function: cmdlcd - Process the LCD FTP command
// Returned: Nothing
//***********************************************

void cmdlcd(void)

{

}

//***************************************************
// Function: cmdlit - Process the LITERAL FTP command
// Returned: Nothing
//***************************************************

void cmdlit(void)

{
    if (!checkcon())
	return;
    sendcmd("", cmdpnt);
    response(FALSE);
}

//*********************************************
// Function: cmdls - Process the LS FTP command
// Returned: Nothing
//*********************************************

void cmdls(void)

{
    if (!checkcon())
	return;
    xferdata("NLST ", datatouser);
}

//*************************************************
// Function: cmdmget - Process the MGET FTP command
// Returned: Nothing
//*************************************************

void cmdmget(void)

{
    fputs("? The MPUT command is not implemented yet\n", stdout);
}

//***************************************************
// Function: cmdmkdir - Process the MKDIR FTP command
// Returned: Nothing
//***************************************************

void cmdmkdir(void)

{
    if (!checkcon())
	return;
    sendcmd("MKD ", cmdpnt);
    response(FALSE);
}

//*************************************************
// Function: cmdmput - Process the MPUT FTP command
// Returned: Nothing
//*************************************************

void cmdmput(void)

{
    fputs("? The MPUT command is not implemented yet\n", stdout);
}

//*************************************************
// Function: cmdopen - Process the OPEN FTP command
// Returned: Nothing
//*************************************************

void cmdopen(void)

{
    connect();
}

//*******************************************************
// Function: cmdallfls - Process the ALLFILES FTP command
// Returned: Nothing
//*******************************************************

void cmdallfls(void)

{
    if (!checkcon())
	return;
    sendcmd("XAFL ", cmdpnt);
    response(FALSE);
}

//*****************************************************
// Function: cmdprompt - Process the PROMPT FTP command
// Returned: Nothing
//*****************************************************

void cmdprompt(void)

{
    prompt ^= 0x01;
    printf("%% Prompt mode is %s\n", (prompt) ? "enabled" : "disabled");
}

//***********************************************
// Function: cmdput - Process the PUT FTP command
// Returned: Nothing
//***********************************************

void cmdput(void)

{
    char *lclpnt;
    char *rmtpnt;

    if (!checkcon())
	return;
    lclpnt = getname();
    rmtpnt = getname();
    if (rmtpnt == NULL)
	rmtpnt = lclpnt;
    if (setuplocal(lclpnt))
    {
	if ((filehndl = svcIoOpen(O_IN, lclname, NULL)) < 0)
	{
	    errormsg("Error opening local file", filehndl);
	    return;
	}
	cmdpnt = rmtpnt;
	xferdata("STOR ", NULL);
    }
}

//***********************************************
// Function: cmdpwd - Process the PWD FTP command
// Returned: Nothing
//***********************************************

void cmdpwd(void)

{
    if (!checkcon())
	return;
    sendcmd("PWD ", cmdpnt);
    response(FALSE);
}

//*************************************************
// Function: cmdquit - Process the QUIT FTP command
// Returned: Nothing
//*************************************************

void cmdquit(void)

{
    exit(0);
}

//***************************************************
// Function: cmdquote - Process the QUOTE FTP command
// Returned: Nothing
//***************************************************

void cmdquote(void)

{
}

//*************************************************
// Function: cmdhelp - Process the HELP FTP command
// Returned: Nothing
//*************************************************

void cmdrhelp(void)

{
    if (!checkcon())
	return;
    sendcmd("HELP ", cmdpnt);
    response(FALSE);
}

//**************************************************
// Function: cmdren - Process the RENAME FTP command
// Returned: Nothing
//**************************************************

void cmdren(void)

{
    char *pnt;
    char  chr;

    if (!checkcon())
	return;
    pnt = cmdpnt;
    while ((chr = *pnt) != 0 && !isspace(chr))
	pnt++;
    if (chr == 0)
    {
	fputs("? No new name specified\n", stdout);
	return;
    }
    *pnt = 0;
    sendcmd("RNFR ", cmdpnt);
    response(TRUE);
    sendcmd("RNTO ", pnt + 1);
    response(FALSE);
}

//***************************************************
// Function: cmdrmdir - Process the RMDIR FTP command
// Returned: Nothing
//***************************************************

void cmdrmdir(void)

{
    if (!checkcon())
	return;
    sendcmd("RMD ", cmdpnt);
    response(FALSE);
}

//**************************************************
// Function: cmdsts - Process the STATUS FTP command
// Returned: Nothing
//**************************************************

void cmdsts(void)

{
    if (!checkcon())
	return;
    sendcmd("STAT ", cmdpnt);
    response(FALSE);
}

//*************************************************
// Function: cmdtype - Process the TYPE FTP command
// Returned: Nothing
//*************************************************

void cmdtype(void)

{
    if (!checkcon())
	return;
    sendcmd("TYPE ", cmdpnt);
    response(FALSE);
}

//*************************************************
// Function: cmduser - Process the USER FTP command
// Returned: Nothing
//*************************************************

void cmduser(void)

{
    if (!checkcon())
	return;
    sendcmd("USER ", cmdpnt);
    if (response(FALSE))
	finishlogin();
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

//****************************************************************
// Function: connect - Connect to the remote system
// Returend: TRUE if OK, FALSE if error, message already displayed
//****************************************************************

int connect(void)

{
    char *pnt;
    int   rtn;
    char  chr;
    char  havedev;
    char  haveaddr;
    char  bufr[256];

    skipws();
    pnt = cmdpnt;
    havedev = FALSE;
    haveaddr = FALSE;
    while ((chr = *pnt++) != 0 && !isspace(chr))
    {
	if (chr == ':')
	{
	    if (*pnt != ':')
		havedev = TRUE;
	    else
		haveaddr = TRUE;
	}
    }
    if (chr != 0 || (pnt - cmdpnt) > 248)
    {
	fputs(synerr, stderr);
	return (FALSE);
    }
    pnt = bufr;
    if (!havedev)
	pnt = strmov(pnt, "TCP0:");
    pnt = strmov(pnt, cmdpnt);
    if (!haveaddr)
	strmov(pnt, "::");
    pnt = bufr;
    while (pnt[0] != ':' && pnt[1] != ':')
	pnt++;

    if ((rtn = pnt - bufr) > 28)
    {
	fputs(synerr, stderr);
	return (FALSE);
    }
    strmov(strnmov(datadev, bufr, rtn + 2), "*::");
    if ((cmdhndl = svcIoOpen(O_IN|O_OUT|O_PARTIAL, bufr, &cmdopnparms)) < 0)
    {
	errormsg("Error connecting to remote system", cmdhndl);
	return (FALSE);
    }
    if (debug)
	printf("ftp: Using %s (local port %d) for command connection\n",
		bufr, cmdopnparms.lclport.value);
    if (!response(FALSE))
	return (FALSE);
    if (rspnum == 220)			// Need a user name?
    {					// Yes
	fputs("User name for remote system: ", stdout);
	gettrm();
	if (!sendcmd("USER ", cmdbufr))
	    return (FALSE);
	if (!response(FALSE))
	    return (FALSE);
    }
    if (!finishlogin())
	return (FALSE);
    sendtype();
    return (TRUE);
}

int finishlogin()

{
    if (rspnum == 331)			// Need a password?
    {					// Yes
	fputs("Password for remote system: ", stdout);
	svcTrmFunction(DH_STDIN, TF_DSECHO);
	gettrm();
	fputs("\n", stdout);
	svcTrmFunction(DH_STDIN, TF_ENECHO);
	if (!sendcmd("PASS ", cmdbufr))
	    return (FALSE);
	if (!response(FALSE))
	    return (FALSE);
    }
    if (rspnum == 332)			// Need a login account?
    {					// Yes
	fputs("Login account for remote system: ", stdout);
	gettrm();
	if (!sendcmd("ACCT ", cmdbufr))
	    return (FALSE);
	if (!response(FALSE))
	    return (FALSE);
    }
    if (rspnum != 230)			// Should be logged in by now
    {
	fputs("% Disconnecting from remote system\n", stdout);
	svcIoClose(cmdhndl, 0);
	cmdhndl = -1;
	return (FALSE);
    }
    return (TRUE);
}


int sendtype(void)

{
    sendcmd("TYPE ", typestr);
    return (response(TRUE));
}

//**************************************************
// Function: response - Get response from FTP server
// Returned: TRUE if OK, FALSE if error
//**************************************************

int response(
    int suppress)		// TRUE if should suppress display of normal
				//   responses
{
    char *pnt;
    char *flag;
    int   temp;
    char  chr;

    tcpavl = 0;
    if (!getrsp())
	return (FALSE);
    rspnum = 0;
    pnt = rspbufr;
    while ((chr = *pnt++) != 0 && isdigit(chr))
	rspnum = rspnum * 10 + (chr & 0x0F);

    flag = (rspnum < 400) ? "" : "? ";

    if (!suppress || rspnum != 200)
	printf("%s%s\n", flag, rspbufr);
    if (chr == '-')			// Multi-line reply?
    {
	do
	{
	    if (!getrsp())
		return (FALSE);
	    if (!suppress || rspnum != 200)
		printf("%s%s\n", flag, rspbufr);
	    temp = 0;
	    pnt = rspbufr;
	    while ((chr = *pnt++) != 0 && isdigit(chr))
		temp = temp * 10 + (chr & 0x0F);
	} while (temp != rspnum);
    }
    return (TRUE);
}


int getrsp(void)

{
    char *pnt;
    char  chr;

    pnt = rspbufr;
    for (;;)
    {

	if (--tcpavl < 0)
	{
	    if ((tcpavl = svcIoInBlock(cmdhndl, tcpbufr, sizeof(tcpbufr))) < 0)
	    {
		errormsg("Error reading response from remote system", tcpavl);
		return (FALSE);
	    }
	    tcppnt = tcpbufr;
	    continue;
	}
	if ((chr = *tcppnt++) == '\n')
	{
	    *pnt = 0;
	    return (TRUE);
	}

	if ((pnt - rspbufr) > (sizeof(rspbufr) - 1))
	{
	    fputs("? Response is too long\n\n", stdout);
	    return (FALSE);
	}
	if (chr != '\r')
	    *pnt++ = chr;
    }
}




int sendcmd(
    char *cmd,
    char *arg)

{
    int   rtn;

    cmd = strmov(strmov(tcpbufr, cmd), arg);
    while (cmd > tcpbufr && isspace(*(cmd-1)))
	--cmd;
    if (debug)
    {
	*cmd = 0;
	printf("ftp: cmd: \"%s<CR><LF>\"\n", tcpbufr);
    }
    rtn = strmov(cmd, "\r\n") - tcpbufr;
    if ((rtn = svcIoOutBlock(cmdhndl, tcpbufr, rtn)) < 0)
    {
	errormsg("Error sending command to remote system", rtn);
	return (FALSE);
    }
    return (TRUE);
}


//**************************************************************
// Function: checkcon - See if connected
// Returned: TRUE if connected, FALSE if not (message displayed)
//**************************************************************

int checkcon(void)

{
    if (cmdhndl < 0)
    {
	fputs("? Not connected\n\n", stdout);
	return (FALSE);
    }
    return (TRUE);
}

//*****************************************
// Function: xferdata - Transfer data 
// Returned: TRUE if normal, FALSE if error
//*****************************************

int xferdata(
    char *cmd,
    int (*func)(char *bufr, int size))

{

    int  rtn;
    int  amount;
    char buffer[2048];

    openflag = 1;
    if (debug)
	printf("ftp: Opening data connection using %s\n", datadev);
    dataopnparms.lclport.value = cmdopnparms.lclport.value | 0x40000000;
    dataqab.qab_option = O_IN|O_OUT|O_PARTIAL;
    if ((rtn = svcIoQueue(&dataqab)) < 0)
    {
	errormsg("Error opening data connection to remote system", rtn);
	return (FALSE);
    }
    if (!sendcmd(cmd, cmdpnt))
	return (FALSE);
    if (!response(FALSE) || rspnum >= 400)
	return (FALSE);
    if (rspnum != 125 && rspnum != 150)
    {
	fputs("? Incorrect response received from server\n", stdout);
	return (FALSE);
    }
    svcSchSuspend(&openflag, -1);
    if (dataqab.qab_error < 0)
    {
	errormsg("Error opening data connection to remote system", rtn);
	return (FALSE);
    }
    if (debug)
	printf("ftp: Using remote port %d for data connection\n",
		dataopnparms.rmtport.value);
    svcSysDateTime(T_GTHRDTTM, &starttime);
    totalbytes = 0;
    if (func != NULL)
    {
	for (;;)
	{
	    if ((rtn = svcIoInBlock(datahndl, buffer,
		    sizeof(buffer))) < 0)
	    {
		func(NULL, 0);
		svcIoClose(datahndl, 0);
		datahndl = -1;
		if (rtn != ER_NCCLR)
		{
		    errormsg("Error receiving data from remote system", rtn);
		    return (FALSE);
		}
		break;
	    }
	    if (!func(buffer, rtn))
	    {
		svcIoClose(datahndl, 0);
		datahndl = -1;
		return (FALSE);
	    }
	}
    }
    else
    {
	for (;;)
	{
	    if ((amount = svcIoInBlock(filehndl, buffer, 1024)) <= 0)
	    {
		if (amount != 0 && amount != ER_EOF)
		{
		    svcIoClose(filehndl, 0);
		    errormsg("Error reading local file", amount);
		    filehndl = -1;
		    svcIoClose(datahndl, 0);
		    datahndl = -1;
		    return (FALSE);
		}
		if ((rtn = svcIoClose(filehndl, 0)) < 0)
		{
		    errormsg("Error closing local file", rtn);
		    filehndl = -1;
		    svcIoClose(datahndl, 0);
		    datahndl = -1;
		    return (FALSE);
		}
		svcSysDateTime(T_GTHRDTTM, &stoptime);
		filehndl = -1;
		svcIoClose(datahndl, 0);
		datahndl = -1;
		break;
	    }
	    if ((rtn = svcIoOutBlock(datahndl, buffer, amount)) < 0)
	    {
		errormsg("Error sending data to remote system", rtn);
		svcIoClose(filehndl, 0);
		filehndl = -1;
		svcIoClose(datahndl, 0);
		datahndl = -1;
		return (FALSE);
	    }
	    totalbytes += amount;
	}
    }
    dispstat();
    response(FALSE);
    return (TRUE);
}

//******************************************************
// Function: opendone - Signal function called when data
//		connection open if complete
// Returned: Nothing
//******************************************************

void opendone(void)

{
    datahndl = dataqab.qab_handle;
    openflag = 0;
}

//*********************************************************
// Function: datatouser - Display received data to the user
// Returned: TRUE always
//*********************************************************

int datatouser(
    char *buffer,
    int   size)

{
    if (buffer != NULL)
    {
	fwrite(buffer, size, 1, stdout);
	totalbytes += size;
    }
    else
        svcSysDateTime(T_GTHRDTTM, &stoptime);
    return (TRUE);
}

//*****************************************************
// Function: datatofile - Store received data to a file
// Returned: TRUE if normal, FALSE if error
//*****************************************************

int datatofile(
    char *buffer,
    int   size)

{
    int rtn;

    if (filehndl < 0)
    {
	if ((filehndl = svcIoOpen(O_OUT|O_CREATE|O_TRUNCA, lclname, NULL)) < 0)
	{
	    errormsg("Error opening local file", filehndl);
	    return (FALSE);
	}
        svcSysDateTime(T_GTHRDTTM, &starttime);
	totalbytes = 0;
    }
    if (buffer == NULL)
    {
	if ((rtn = svcIoClose(filehndl, 0)) < 0)
	{
	    errormsg("Error closing local file", rtn);
	    filehndl = -1;
	    return (FALSE);
	}
	filehndl = -1;
	svcSysDateTime(T_GTHRDTTM, &stoptime);
    }
    else
    {
	if ((rtn = svcIoOutBlock(filehndl, buffer, size)) != size)
	{
	    if (rtn >= 0)
		rtn = ER_DATTR;
	    errormsg("Error writing to local file", rtn);
	    svcIoClose(filehndl, 0);
	    filehndl = -1;
	    return (FALSE);
	}
	totalbytes += rtn;
    }
    return (TRUE);
}

//**********************************************************
// Function: getname - Get a file name from the command line
// Returned: Pointer to first charater in name
//**********************************************************

char *getname(void)

{
    char *begin;
    char  chr;
    char  quote;

    quote = 0;
    skipws();
    begin = cmdpnt;
    while ((chr = *cmdpnt) != 0)
    {
	if (chr == '"')
	    quote ^= 0x01;
	else
	{
	    if (!quote && isspace(chr))
		break;
	}
	cmdpnt++;
    }
    if (begin == cmdpnt)
	return (NULL);
    *cmdpnt++ = 0;
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
