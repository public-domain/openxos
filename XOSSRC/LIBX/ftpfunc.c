//--------------------------------------------------------------------------*
// FTPFUNC.C
// FTP client fuctions for XOS
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

// This module contains functions which allow a program in internally
//   implement the major FTP client functions. This includes connecting
//   to a remote system and reading and writing files. All externally
//   callable functions in this module begin with the 3 letters "ftp".
//   All other functions are local to this module.


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
#include <XOSFTPFUNC.H>

#define VERSION 1
#define EDITNO  0

#define NAMEMAX  1024

typedef struct filedescp FILEDESCP;

typedef struct cmdtbl CMDTBL;

static long  openflag;

static int   rspnum;
static long  cmdhndl = -1;
static long  datahndl;

static long *rspvalpnt;
static char *rspstrpnt;
static int   rspstrlen;

static char *typestr = "A N";

static long  tcpavl;
static char *tcppnt;
static char  tcpbufr[256];

static char  datadev[32];

static uchar debug = 0;

uchar ftp_XosSrvr;

static struct
{	byte4_parm  rmtaddr;
	byte4_parm  rmtport;
	text4_parm  lclneta;
	byte4_parm  lclport;
	char        end;
} cmdopnparms =
{	{PAR_GET|REP_HEXV, 4, IOPAR_NETRMTNETAR},
	{PAR_SET|REP_HEXV, 4, IOPAR_NETRMTPORTS, TCPP_FTPCMD},
	{PAR_GET|REP_HEXV, 4, IOPAR_NETLCLNETA},
	{PAR_GET|REP_HEXV, 4, IOPAR_NETLCLPORT}
};

static struct
{	byte4_parm  limit;
	byte4_parm  lclport;
    char        end;
} actopn1parms =
{	{PAR_SET|REP_HEXV, 4, IOPAR_NETCONLIMIT, 1},
    {PAR_GET|REP_HEXV, 4, IOPAR_NETLCLPORT}
};

static struct
{	byte4_parm  hndl;
	byte4_parm  rmtaddr;
	byte4_parm  rmtport;
    char        end;
} actopn2parms =
{	{PAR_SET|REP_HEXV, 4, IOPAR_NETCONHNDL},
	{PAR_GET|REP_HEXV, 4, IOPAR_NETRMTNETAR},
	{PAR_GET|REP_HEXV, 4, IOPAR_NETRMTPORTR}
};

static struct
{	byte4_parm  rmtaddr;
	byte4_parm  rmtport;
	byte4_parm  lclport;
	uchar       end;
} pasopnparms =
{	{PAR_SET|REP_HEXV, 4, IOPAR_NETRMTNETAS},
	{PAR_SET|REP_HEXV, 4, IOPAR_NETRMTPORTS},
    {PAR_GET|REP_HEXV, 4, IOPAR_NETLCLPORT}
};

static int  datatofile(char *buffer, int size);
static int  datatouser(char *buffer, int size);
static int  get2dig(char **ppnt, ushort *val);
static int  getbytevalue(char **ppnt, uchar *vpnt);
static long getloop(long (*func)(char *bufr, int size), char *bufr, int size);
static long putloop(long (*func)(char *bufr, int size), char *bufr, int size);
static int  response(void);
static long sendcmd(char *cmd, char *arg);
static long sendxfer(char *cmd, char *arg, void (*notify)(char *str),
		time_sz *filedt);
static long xferdata(char *cmd, char *arg, void (*notify)(char *str),
		time_sz *fdt, long (*func)(char *bufr, int size), char *bufr,
		int size, int partial, long (*loop)(long (*func)(char *bufr,
		int size), char *bufr, int size));


//=========================================================
// Following functions are used to manage the FTP functions
//   and to connect to and disconnect from the remote host
//=========================================================


//********************************************************
// Function: ftpInit - Initialize the FTP functions
// Returned: 0 if normal, negative XOS error code if error
//********************************************************

long ftpInit(
    long *rspval,
	char *rspstr,
	int   rsplen,
    int   debugarg)

{
	rspvalpnt = rspval;
	rspstrpnt = rspstr;
	rspstrlen = rsplen;

	debug = debugarg;
	svcSchSetLevel(0);			// Enable signals
	return (0);
}

//****************************************************
// Function: ftpDebug - Set or clear DEBUG mode
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpDebug (
    int arg)

{
    debug = arg;
    return (0);
}

//****************************************************
// Function: ftpOpen - Establish an FTP connection
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

// The host specification is of the form:
//		{dev:}ipaddress{::}
//   If dev is not present, TCP0: is assumed

long ftpOpen(
    char *host)

{
    char *pnt;
	int   rtn;
	uchar havedev;
	uchar haveaddr;
	char  chr;
	char  bufr[64];

    pnt = host;
    havedev = FALSE;
    haveaddr = FALSE;

	// First scan the host specification and determine what is present

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

	// Now construct a complete specification in our buffer, adding the
	//   parts that are missing

    if (chr != 0 || (pnt - host) > 248)
		return (ER_BDSPC);
    pnt = bufr;
    if (!havedev)
		pnt = strmov(pnt, "TCP0:");
    pnt = strmov(pnt, host);
    if (!haveaddr)
		strmov(pnt, "::");
    pnt = bufr;
    while (pnt[0] != ':' && pnt[1] != ':')
		pnt++;

	// Store the device name we need for data connections

	if ((rtn = pnt - bufr) > 28)
		return (ER_BDSPC);
	strnmov(datadev, bufr, rtn + 2);
	datadev[rtn + 2] = 0;

	// Establish the command connection to the server

	if ((cmdhndl = svcIoOpen(O_IN|O_OUT|O_PARTIAL, bufr, &cmdopnparms)) < 0)
		return (cmdhndl);
	if (debug)
		printf("ftp: Using %s (local port %d) for command connection\n",
				host, cmdopnparms.lclport.value);
	if ((rtn = response()) < 0)
		return (rtn);
    return (0);
}

//****************************************************
// Function: ftpUser - Process the USER FTP command
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpUser(
    char *user)

{
    long rtn;

    if (cmdhndl < 0)
		return (ER_NNCON);
    if ((rtn = sendcmd("USER ", user)) != 0)
		return (rtn);
    return (response());
}

//****************************************************
// Function: ftpPassword - Process the PWD FTP command
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpPassword(
    char *password)

{
    long rtn;

    if (cmdhndl < 0)
		return (ER_NNCON);
    if ((rtn = sendcmd("PASS ", password)) != 0)
		return (rtn);
    return (response());
}

long ftpSystemType(void)

{
	long rtn;
	
	if ((rtn = sendcmd("SYST ", "")) < 0 || (rtn = response()) >= 0)
		ftp_XosSrvr = (strncmp(rspstrpnt, "215 XOS ", 7) == 0);
	return (rtn);
}

//***********************************************************
// Function: ftpClose - Terminate an FTP connection to a host
// Returned: 0 if OK, negative XOS error code if error
//***********************************************************

long ftpClose (void)

{
    return (ER_NIYT);
}


//=======================================================
// The following functions set parameters which determine
//   how files on the remote host are accessed
//=======================================================


//****************************************************
// Function: ftpAscii - Set transfer mode to ASCII
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpAscii(void)

{
    typestr = "A N";					// Remember the type
    return (sendtype());				// Send type now if connected
}

//****************************************************
// Function: ftpBinary - Set transfer mode to BINARY
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpBinary(void)

{
    typestr = "I";						// Remember the type
    return (sendtype());				// Send type now if connected
}

//****************************************************
// Function: ftpType - Process the TYPE FTP command
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpType(
    char *arg)

{
    long rtn;

    if (cmdhndl < 0)
		return (ER_NNCON);
    if ((rtn = sendcmd("TYPE ", arg)) != 0)
		return (rtn);
    return (response());
}

//****************************************************
// Function: ftpCd - Process the CD FTP command
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpCd(
    char *newdir)

{
    long rtn;

    if (cmdhndl < 0)
		return (ER_NNCON);
    if ((rtn = sendcmd("CWD ", newdir)) != 0)
		return (rtn);
    return (response());
}

//****************************************************
// Function: ftpPWD - Process the PWD FTP command
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpPWD(void)

{
    long rtn;

    if (cmdhndl < 0)
		return (ER_NNCON);
    if ((rtn = sendcmd("PWD ", "")) != 0)
		return (rtn);
    return (response());
}

//****************************************************
// Function: ftpStatus - Process the STAT FTP command
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpStatus(void)

{
    long rtn;

    if (cmdhndl < 0)
		return (ER_NNCON);
    if ((rtn = sendcmd("STAT", "")) != 0)
		return (rtn);
    return (response());
}

//*********************************************************
// Function: ftpAllFiles - Process the ALLFILES FTP command
// Returned: 0 if OK, negative XOS error code if error
//*********************************************************

// This command is XOS specific. It determines if a priviledged
//   user can access file above his base directory or specify
//   a device.

long ftpAllFiles(
    char *arg)

{
    long rtn;

    if (cmdhndl < 0)
		return (ER_NNCON);
    if ((rtn = sendcmd("XAFL ", arg)) != 0)
		return (rtn);
    return (response());
}

//****************************************************
// Function: ftpLiteral - Send any FTP command
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpLiteral(
    char *text)

{
    long rtn;

    if (cmdhndl < 0)
		return (ER_NNCON);
    if ((rtn = sendcmd("", text)) != 0)
		return (rtn);
    return (response());
}


//===========================================================
// The following functions create, delete, or renamd files or
//   directories on the on the host but do not transfer data.
//===========================================================


//*****************************************************
// Function: ftpDelete - Process the DELETE FTP command
// Returned: 0 if OK, negative XOS error code if error
//*****************************************************

long ftpDelete(
    char *file)

{
    long rtn;

    if (cmdhndl < 0)
		return (ER_NNCON);
    if ((rtn = sendcmd("DELE ", file)) != 0)
		return (rtn);
    return (response());
}

//*****************************************************
// Function: ftpRename - Process the RENAME FTP command
// Returned: 0 if OK, negative XOS error code if error
//*****************************************************

long ftpRename(
    char *oldname,
    char *newname)

{
    long  rtn;

    if (cmdhndl < 0)
		return (ER_NNCON);
    if ((rtn = sendcmd("RNFR ", oldname)) != 0)
		return (rtn);
    if ((rtn = response()) != 0)
		return (rtn);
    if ((rtn = sendcmd("RNTO ", newname)) != 0)
		return (rtn);
    return (response());
}

//****************************************************
// Function: ftpRmDir - Process the RMDIR FTP command
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpRmDir(
    char *name)

{
    long rtn;

    if (cmdhndl < 0)
		return (ER_NNCON);
    if ((rtn = sendcmd("RMD ", name)) != 0)
		return (rtn);
    return (response());
}

//****************************************************
// Function: ftpMkDir - Process the MKDIR FTP command
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpMkDir(
    char *name)

{
    long rtn;

    if (cmdhndl < 0)
		return (ER_NNCON);
    if ((rtn = sendcmd("MKD ", name)) != 0)
		return (rtn);
    return (response());
}


//=================================================================
// The following functions perform a data transfer to or from the
//   remote host.
//=================================================================


//****************************************************
// Function: ftpLs - Process the LS FTP command
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpLs(
	char  *name,
	void (*notify)(char *str),
    long (*func)(char *bufr, int size),
    char  *bufr,
    int    size,
    int    partial)

{
    if (cmdhndl < 0)
		return (ER_NNCON);
    return (xferdata("NLST ", name, notify, NULL, func, bufr, size, partial,
			getloop));
}

//****************************************************
// Function: ftpDir - Process the DIR FTP command
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpDir(
	char  *name,
	void (*notify)(char *str),
    long (*func)(char *bufr, int size),
    char  *bufr,
    int    size,
    int    partial)

{
    if (cmdhndl < 0)
		return (ER_NNCON);
    return (xferdata("LIST ", name, notify, NULL, func, bufr, size, partial,
			getloop));
}

//****************************************************
// Function: ftpGet - Process the GET FTP command
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpGet(
    char    *name,
	void   (*notify)(char *str),
    long   (*func)(char *bufr, int size),
    char    *bufr,
    int      size,
	time_sz *filedt,
    int      partial)

{
    if (cmdhndl < 0)
		return (ER_NNCON);
    return (xferdata("RETR ", name, notify, filedt, func, bufr, size, partial,
			getloop));
}

//****************************************************
// Function: ftpPut - Process the PUT FTP command
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpPut(
    char    *name,
	time_sz *filedt,
	void   (*notify)(char *str),
    long   (*func)(char *bufr, int size),
    char    *bufr,
    int      size,
    int      partial)

{
	char *pnt;
	char  nbfr[600];

    if (cmdhndl < 0)
		return (ER_NNCON);
	if (ftp_XosSrvr)
	{
		sdt2str(strnmov(nbfr, name, 570), " (%l%r%d%h%m%s%O)", filedt);
		pnt = nbfr;
	}
	else
		pnt = name;
    return (xferdata("STOR ", pnt, notify, NULL, func, bufr, size, partial,
			putloop));
}

//*****************************************************
// Function: ftpAppend - Process the APPEND FTP command
// Returned: 0 if OK, negative XOS error code if error
//*****************************************************

long ftpAppend(
    char  *name,
    long (*func)(char *bufr, int size),
    char  *bufr,
    int    size,
    int    partial)

{
    name = name;
    func = func;
    bufr = bufr;
    size = size;
    partial = partial;

    return (ER_NIYT);
}

//****************************************************
// Function: ftpHelp - Process the HELP FTP command
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpHelp(
    char *name)

{
	name = name;

    if (cmdhndl < 0)
		return (ER_NNCON);
    return (ER_NIYT);
}

//****************************************************
// Function: ftpQuit - Process the QUIT FTP command
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

long ftpQuit(void)

{
	if (datahndl > 0)
	{
		svcIoClose(datahndl, 0);
		datahndl = 0;
	}
    if (cmdhndl > 0)
	{
		svcIoClose(cmdhndl, 0);
		cmdhndl = 0;
	}
	return (0);
}

int sendtype(void)

{
    sendcmd("TYPE ", typestr);
    return (response());
}


//**************************************************
// Function: response - Get response from FTP server
// Returned: TRUE if OK, FALSE if error
//**************************************************

static int response(void)

{
    char *pnt;
    long  temp;
    long  rtn;
    char  chr;

	*rspvalpnt = 0;
	rspstrpnt[0] = 0;
    if ((rtn = getrsp(rspstrpnt)) < 0)
		return (rtn);
    rspnum = 0;
    pnt = rspstrpnt;
    while ((chr = *pnt++) != 0 && isdigit(chr))
		rspnum = rspnum * 10 + (chr & 0x0F);
    if (chr == '-')			// Multi-line reply?
    {
		do
		{
			if ((rtn = getrsp(rspstrpnt)) < 0)
				return (rtn);
			temp = 0;
			pnt = rspstrpnt;
			while ((chr = *pnt++) != 0 && isdigit(chr))
				temp = temp * 10 + (chr & 0x0F);
		} while (temp != rspnum);
    }
    *rspvalpnt = rspnum;
	return (0);
}


int getrsp(
    char *bufr)

{
    char *pnt;
    char  chr;

    pnt = bufr;
	while (TRUE)
    {
		if (--tcpavl < 0)
		{
			if ((tcpavl = svcIoInBlock(cmdhndl, tcpbufr, sizeof(tcpbufr))) < 0)
				return (tcpavl);
			tcppnt = tcpbufr;
			continue;
		}
		if ((chr = *tcppnt++) == '\n')
		{
			*pnt = 0;
			if (debug)
				printf("ftp: rsp: \"%s<CR><LF>\"\n", bufr);
			return (pnt - bufr);
		}
		if ((pnt - bufr) > (rspstrlen - 2))
		    return (ER_NAPER);
		if (chr != '\r')
	    	*pnt++ = chr;
    }
}

//****************************************************
// Function: sendcmd - send command to remote system
// Returned: 0 if OK, negative XOS error code if error
//****************************************************

static long sendcmd(
    char *cmd,
    char *arg)

{
    char *pnt;
    long  rtn;

    pnt = strmov(strmov(tcpbufr, cmd), arg);
    if (debug)
		printf("ftp: cmd: \"%s<CR><LF>\"\n", tcpbufr);
    rtn = strmov(pnt, "\r\n") - tcpbufr;
    if ((rtn = svcIoOutBlock(cmdhndl, tcpbufr, rtn)) < 0)
		return (rtn);
    return (0);
}

//**************************************************************
// Function: xferdata - Set up for function which transfers data
// Returned: 0 if OK, negative XOS error code if error
//**************************************************************

static long xferdata(
    char    *cmd,
    char    *arg,
	void   (*notify)(char *str),
	time_sz *filedt,
    long   (*func)(char *bufr, int size),
    char    *bufr,
    int      size,
    int      partial,
    long   (*loop)(long (*func)(char *bufr, int size), char *bufr, int size))

{
	char  *pnt;
	union
	{	long v;
		char c[4];
	}      addr;
	union
	{	long v;
		char c[2];
	}      port;
 	long   rtn;
	long   cls;
    char   portarg[32];
	char   chr;

    openflag = 1;
    if (debug)
		printf("ftp: Opening data connection using %s\n", datadev);

	// Send a PASV command

	if ((rtn = sendcmd("PASV", "")) < 0 || (rtn = response()) != 0)
		return (rtn);
	pnt = rspstrpnt;
	while ((chr = *pnt++) != 0 && chr != '(')
		;
	port.v = 0;
	if (rspnum == 227 && chr == '(' &&
			getbytevalue(&pnt, &(addr.c[0])) == ',' &&
			getbytevalue(&pnt, &(addr.c[1])) == ',' &&
			getbytevalue(&pnt, &(addr.c[2])) == ',' &&
			getbytevalue(&pnt, &(addr.c[3])) == ',' &&
			getbytevalue(&pnt, &(port.c[1])) == ',' &&
			getbytevalue(&pnt, &(port.c[0])) == ')')
	{
		// Here if the PASV command worked and the response format looks
		//   valid - use passive mode

		pasopnparms.rmtaddr.value = cmdopnparms.rmtaddr.value;
		pasopnparms.rmtport.value = port.v;

		if ((rtn = svcIoOpen((partial) ? (O_IN|O_OUT|O_PARTIAL) : (O_IN|O_OUT),
				datadev, &pasopnparms)) < 0)
			return (rtn);
		datahndl = rtn;
	    if (debug)
			printf("ftp: Using local port %d and remote passive port %d for "
					"data connection\n", pasopnparms.lclport.value, port.v);

		// Now send the transfer command

		if ((rtn = sendxfer(cmd,arg, notify, filedt)) < 0)
			return (rtn);
	}
	else
	{
		// Here if the PASV command did not work - Use normal (active) mode.
		//   Start by opening a TCP device and sending a PORT command

		if ((rtn = svcIoOpen(0, datadev, &actopn1parms)) < 0)
			return (rtn);
		datahndl = actopn2parms.hndl.value = rtn;
	    sprintf(portarg, "%d,%d,%d,%d,%d,%d",
				(uchar)(cmdopnparms.lclneta.value[0]),
				(uchar)(cmdopnparms.lclneta.value[1]),
				(uchar)(cmdopnparms.lclneta.value[2]),
				(uchar)(cmdopnparms.lclneta.value[3]),
				(uchar)(actopn1parms.lclport.value >> 8),
				(uchar)(actopn1parms.lclport.value));
	    if ((rtn = sendcmd("PORT ", portarg)) != 0 || (rtn = response()) != 0)
		{
			svcIoClose(datahndl, 0);
			datahndl = 0;
			return (rtn);
		}

		// Send the transfer command

		if ((rtn = sendxfer(cmd,arg, notify, filedt)) < 0)
			return (rtn);

		// Now wait until the server connects to us

		rtn = svcIoOpen((partial) ? (O_IN|O_OUT|O_PARTIAL) : (O_IN|O_OUT),
				datadev, &actopn2parms);
		svcIoClose(datahndl, 0);
		datahndl = 0;
		if (rtn <= 0)
			return (rtn);
		datahndl = rtn;
		if (debug)
			printf("ftp: Using local port %d and remote active port %d for "
					"data connection\n", actopn1parms.lclport.value,
					actopn2parms.rmtport.value);
	}
    rtn = loop(func, bufr, size);
	if ((cls = svcIoClose(datahndl, 0)) < 0 && rtn >= 0)
		rtn = cls;
	datahndl = 0;
	while (rspnum < 200)
		response();
	return (rtn);
}


static long sendxfer(
	char    *cmd,
	char    *arg,
	void   (*notify)(char *str),
	time_sz *filedt)

{
	time_x dtx;
	char  *pnt;
	long   rtn;
	ushort year;
	ushort tzm;
	ushort tzh;
	char   chr;

    if ((rtn = sendcmd(cmd, arg)) != 0 || (rtn = response()) != 0)
	{
		svcIoClose(datahndl, 0);
		datahndl = 0;
		return (rtn);
	}
	if (notify != NULL)
		notify(rspstrpnt);
	if (rspnum != 125 && rspnum != 150)
	{
		rtn = ER_NRMTE;
		svcIoClose(datahndl, 0);
		datahndl = 0;
		return (rtn);
	}
	if (filedt != NULL)
	{
		filedt->date = 0;
		if (ftp_XosSrvr)
		{
			pnt = rspstrpnt;
			while ((chr = *pnt++) != 0)
			{
				if (chr == '(')
				{
					if (!get2dig(&pnt, &year) ||
							!get2dig(&pnt, &(dtx.dos.tmx_year)) ||
							!get2dig(&pnt, &(dtx.dos.tmx_mon)) ||
							!get2dig(&pnt, &(dtx.dos.tmx_mday)) ||
							!get2dig(&pnt, &(dtx.dos.tmx_hour)) ||
							!get2dig(&pnt, &(dtx.dos.tmx_min)) ||
							!get2dig(&pnt, &(dtx.dos.tmx_sec)))
						break;
					dtx.dos.tmx_year += (year * 100);
					if ((chr = *pnt) == '+' || chr == '-')
					{
						pnt++;
						if (!get2dig(&pnt, &tzh) || !get2dig(&pnt, &tzm))
							return (TRUE);
					}
					if (*pnt++ != ')')
						break;
					svcSysDateTime(T_CVD2X, &dtx);
					filedt->date = dtx.sys.date;
					filedt->time = dtx.sys.time;
					filedt->tzone = tzh * 60 + tzm;
					if (chr == '-')
						filedt->tzone = -filedt->tzone;
					filedt->dlst = 0;
					break;
				}
			}
		}
	}
	return (0);
}


static int get2dig(
	char  **ppnt,
	ushort *val)

{
	char chr1;
	char chr2;

	chr1 = (*ppnt)[0];
	chr2 = (*ppnt)[1];
	if (!isdigit(chr1) || !isdigit(chr2))
		return (FALSE);
	*val = (chr2 & 0x0F) + (chr1 & 0x0F) * 10;
	(*ppnt) += 2;
	return (TRUE);
}


static long getloop(
    long (*func)(char *bufr, int size),
    char  *bufr,
    int    size)

{
    long rtn;

	while (TRUE)
	{
		if ((rtn = svcIoInBlock(datahndl, bufr, size)) <= 0)
			return ((rtn == ER_NCCLR) ? 0 : rtn);
		if ((rtn = func(bufr, rtn)) < 0)
			return (rtn);
    }
}

static long putloop(
    long (*func)(char *bufr, int size),
    char  *bufr,
    int    size)

{
    long rtn;

    while (TRUE)
    {
		if ((rtn = func(bufr, size)) <= 0)
			return (rtn);
		if ((rtn = svcIoOutBlock(datahndl, bufr, rtn)) <= 0)
			return ((rtn == ER_NCCLR) ? 0 : rtn);
    }
}


static int getbytevalue(
	char **ppnt,
	uchar *vpnt)

{
	char *pnt;
	long  value;
	char  chr;

	pnt = *ppnt;
	value = 0;
	while ((chr = *pnt++) != 0 && isdigit(chr))
		value = value * 10 + (chr & 0x0F);
	*ppnt = pnt;
	*vpnt = (uchar)value;
	return (chr);
}
