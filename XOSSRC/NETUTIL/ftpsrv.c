//--------------------------------------------------------------------------*
// FTPSRV.C
// FTP server for XOS
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

#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <xoserr.h>
#include <xostrm.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <xoserrmsg.h>
#include <xosnet.h>
#include <procarg.h>
#include <server.h>
#include <xosstr.h>

#define MAJVER  1
#define MINVER  0
#define EDITNUM 0

#define ver ver2(MAJVER,MINVER,EDITNUM)
#define ver2(a,b,c) ver3(a,b,c)
#define ver3(maj,min,edit) #maj "." #min "." #edit

#define INSTMAX  31				// Maximum instance number
#define CDBBASE  0x30000		// Base offset for first CDB
#define MAXOPENS 128
#define DFTOPENS 3
#define MAXCONS  4088
#define DFTCONS  20

// This program is the FTP file server.  It is initialized as a symbiont with
//   the following command:

//	SYMBIONT FTPSRV UNIT=n LOGLEVEL=l LOGFILE=log
//	  Where:
//		UNIT     = Server unit number (default = 1)
//		LOGLEVEL = Specifies the logging level, default is 3 if LOGFILE
//			   is specified, 0 otherwise
//				0 = No logging
//				1 = Log major events
//				2 = Log all network messages
//		LOGFILE  = Specifies file for log output, if not specified no
//			   log output is generated

// Each copy of XFPSRV running on a system must have a unique unit number.  The
//   actual connections between network RCP devices and the server are made
//   after the server is loaded using the SERVER utility.

// Description of data structures used:

//   The server can support up to 31 server instances.  Each instance is
//   associated with an RCP unit/port number pair and can support up to 4088
//   open files (in theory!).  For each instance an IDB is allocated (at the
//   base of a 16MB address space which is reserved for the IDB and the
//   associated CDBs) and contains all data needed to operate the instance.
//   The IDB includes a table of pointers to each possible CDB.  The size of
//   this table is equal to the maximum number of open files allowed.  CDBs
//   (each of which is 4KB) are only allocated when a file is opened.  Each
//   one is allocated as a separate 1 page msect.

// Memory layout for a single instance (starts on a 16MB boundry, base for
//   unit 1 starts at 16MB):
//    base + 00000000:        IDB
//    base + idb_cdbtbl:      CDB table (first entry is for CDB number 8)
//    base + 00008000:        First CDB
//    base + 00009000:        Second CDB
//    ...

//   Free CDBs are managed using a free list linked through the CDB table.
//   each free list entry contains the NUMBER of the next free CDB.  The number
//   is the difference between the CDBs offset and the base of the IDB divided
//   by 1000h.  The first CDB thus has a number of 8.

#define IDBBASE 0x01000000

#define VECT_CMDCONNECT 70		// TCP connection done signal
#define VECT_OPEN       71		// TCP open done signal
#define VECT_CMDINPUT   72		// TCP command input available signal
#define VECT_CMDOUTPUT  73		// TCP command output complete signal
#define VECT_CMDCLOSE   74		// TCP command close done signal
#define VECT_DISK       75		// Disk IO done signal

typedef struct _cdb CDB;
typedef struct _idb IDB;

// Define the CDB (Connection Data Block)

struct _cdb
{	QAB   cmdinqab;
	QAB   cmdoutqab;
	QAB   dataqab;
	QAB   diskqab;
	struct
	{	byte4_parm timeout;
		uchar      end;
		uchar      xxx[3];
	}     cmdinparms;
	struct
	{	byte4_parm timeout;
		uchar      end;
		uchar      xxx[3];
	}     cmdoutparms;
	IDB  *idb;
	int   slot;
	long  rmtaddr;
	long  rmtcmdport;
	long  dataaddr;
	long  dataport;
	char *cmdlinepnt;
	int   cmdlinecnt;
	char  cmdinbufr[200];
	char  cmdlinebufr[512];
};

// Define the IDB (Instance Data Block)

struct _idb
{	QAB   cmdconqab;			// Command connection QAB
	struct
	{	byte4_parm hndl;
		byte4_parm timeout;
		byte4_parm addr;
		byte4_parm port;
		uchar      end;
		uchar      xxx[3];
	}     cmdconparms;			// Command connection IO parameters
	char  devname[32];
	long  cmdport;
	long  dataport;
	int   instance;				// Instance number
	int   numclients;			// Current number of client connections
	int   maxclients;
	long  cdbfree;				// Index for first free CDB
	CDB  *cdbtbl[1];			// CDB table
};


typedef struct
{   long  svdEDI;
    long  svdESI;
    long  svdEBP;
    long  svdESP;
    long  svdEBX;
    long  svdEDX;
    long  svdECX;
    long  svdEAX;
    long  intEIP;
    long  intCS;
    long  intEFR;
    long  intGS;
    long  intFS;
    long  intES;
    long  intDS;
    short intcnt;
    short intnum;
    long  qab;
} INTDATA;

long instance;
long instx;

char devname[32];
char phyname[32];
long cmdport;
long dataport;
long conlimit;
long maxclients;

long version = MAJVER;
long editnum = EDITNUM;



void ftpcmduser(CDB *cdb, char *pnt);
void ftpcmdpass(CDB *cdb, char *pnt);
void ftpcmdacct(CDB *cdb, char *pnt);
void ftpcmdquit(CDB *cdb, char *pnt);
void ftpcmdhelp(CDB *cdb, char *pnt);
void ftpcmdnoop(CDB *cdb, char *pnt);
void ftpcmdcwd(CDB *cdb, char *pnt);
void ftpcmdcdup(CDB *cdb, char *pnt);
void ftpcmdsmnt(CDB *cdb, char *pnt);
void ftpcmdrein(CDB *cdb, char *pnt);
void ftpcmdport(CDB *cdb, char *pnt);
void ftpcmdpasv(CDB *cdb, char *pnt);
void ftpcmdtype(CDB *cdb, char *pnt);
void ftpcmdstru(CDB *cdb, char *pnt);
void ftpcmdmode(CDB *cdb, char *pnt);
void ftpcmdretr(CDB *cdb, char *pnt);
void ftpcmdstor(CDB *cdb, char *pnt);
void ftpcmdstou(CDB *cdb, char *pnt);
void ftpcmdappe(CDB *cdb, char *pnt);
void ftpcmdallo(CDB *cdb, char *pnt);
void ftpcmdrest(CDB *cdb, char *pnt);
void ftpcmdrnfr(CDB *cdb, char *pnt);
void ftpcmdrnto(CDB *cdb, char *pnt);
void ftpcmdabor(CDB *cdb, char *pnt);
void ftpcmddele(CDB *cdb, char *pnt);
void ftpcmdrmd(CDB *cdb, char *pnt);
void ftpcmdmkd(CDB *cdb, char *pnt);
void ftpcmdpwd(CDB *cdb, char *pnt);
void ftpcmdlist(CDB *cdb, char *pnt);
void ftpcmdnlst(CDB *cdb, char *pnt);
void ftpcmdsite(CDB *cdb, char *pnt);
void ftpcmdsyst(CDB *cdb, char *pnt);
void ftpcmdstat(CDB *cdb, char *pnt);
void ftpcmdxafl(CDB *cdb, char *pnt);


void cmdoutput(CDB *cdb, char *msg, int len);
int getbyteval(char **ppnt, uchar *val, int stopper);


static void banner(void);

static void cmdadd(char *cmd);
static void cmdstatus(char *cmd);

static void cmdconnectdone(INTDATA idata);
static void opendone();
static void cmdinputdone(INTDATA idata);
static void cmdoutputdone(INTDATA idata);
static void cmdclosedone(INTDATA idata);
static void diskdone(INTDATA idata);


static int  fncaddcmdport(arg_data *arg);
static int  fncadddataport(arg_data *arg);
static int  fncadddevice(arg_data *arg);
static int  fncaddlimit(arg_data *arg);
static int  fncaddmaximum(arg_data *arg);
static int  fncinstance(arg_data *arg);

static IDB *requireinst(char *cmd, arg_spec *keyword);


///static void timerthread(void);

arg_spec cmdword[] =
{   {"ADD"   , 0, NULL, srvFncCommand, (long)cmdadd},
    {"STATUS", 0, NULL, srvFncCommand, (long)cmdstatus},
    {"STA"   , 0, NULL, srvFncCommand, (long)cmdstatus},
    {0}
};

arg_spec keywordadd[] =
{   {"INSTANCE", ASF_VALREQ|ASF_NVAL , NULL, fncinstance   , 0},
    {"INS"     , ASF_VALREQ|ASF_NVAL , NULL, fncinstance   , 0},
	{"DEVICE"  , ASF_VALREQ|ASF_LSVAL, NULL, fncadddevice  , 0},
	{"DEV"     , ASF_VALREQ|ASF_LSVAL, NULL, fncadddevice  , 0},
	{"CMDPORT" , ASF_VALREQ|ASF_NVAL , NULL, fncaddcmdport , 0},
	{"CMD"     , ASF_VALREQ|ASF_NVAL , NULL, fncaddcmdport , 0},
	{"DATAPORT", ASF_VALREQ|ASF_NVAL , NULL, fncadddataport, 0},
	{"DAT"     , ASF_VALREQ|ASF_NVAL , NULL, fncadddataport, 0},
	{"LIMIT"   , ASF_VALREQ|ASF_NVAL , NULL, fncaddlimit   , 0},
	{"LIM"     , ASF_VALREQ|ASF_NVAL , NULL, fncaddlimit   , 0},
	{"MAXIMUM" , ASF_VALREQ|ASF_NVAL , NULL, fncaddmaximum , 0},
	{"MAX"     , ASF_VALREQ|ASF_NVAL , NULL, fncaddmaximum , 0},
	{0}
};

arg_spec keywordsts[] =
{   {"INSTANCE", ASF_VALREQ|ASF_NVAL, NULL, fncinstance, 0},
    {"INS"     , ASF_VALREQ|ASF_NVAL, NULL, fncinstance, 0},
    {0}
};


struct
{   text8_parm  class;
    byte4_parm  filoptn;
    lngstr_parm filspec;
    byte4_parm  tcpport;
    byte4_parm  conlimit;
    char        end;
} tcpoparms =
{   {PAR_GET|REP_TEXT, 8, IOPAR_CLASS},
    {PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, FO_XOSNAME},
    {PAR_GET|REP_STR , 0, IOPAR_FILSPEC, phyname, 0, sizeof(phyname),
        sizeof(phyname)},
    {PAR_SET|REP_HEXV, 4, IOPAR_NETLCLPORT, 0},
    {PAR_SET|REP_HEXV, 4, IOPAR_NETCONLIMIT, 0}
};

char prgname[] = "FTPSRV";
char srvname[] = "FTPSRV";


char msg200[] = "200 Command OK\r\n";
char msg220[] = "220 FTP (XOS) Login required\r\n";
char msg230[] = "230 User logged in, proceed\r\n";
char msg331[] = "331 User name OK, need password\r\n";
char msg500a[] = "500 Command unreconnized\r\n";
char msg500b[] = "500 Syntax error in command\r\n";
char msg502[] = "502 Command not implemented\r\n";


void setup1(void)

{
	if (setvector(VECT_CMDCONNECT, 0x84, cmdconnectdone) < 0 ||
										// Set the connection done vector
			setvector(VECT_OPEN, 0x84, opendone) < 0 ||
										// Set the open done vector
			setvector(VECT_CMDINPUT, 0x84, cmdinputdone) < 0 ||
										// Set the input ready vector
			setvector(VECT_CMDOUTPUT, 0x84, cmdoutputdone) < 0 ||
										// Set the output complete vector
			setvector(VECT_CMDCLOSE, 0x84, cmdclosedone) < 0 ||
										// Set the close done vector
			setvector(VECT_DISK, 0x84, diskdone) < 0)
										// Set the disk IO done vector
	{

	}
}


void setup2(void)

{

}


void setupcmd(void)

{
	instance = 0;
}


void finishcmd(void)

{

}


int message(void)

{
	return (TRUE);
}


//*******************************************
// Function: cmdadd - Process the ADD command
// Returned: Nothing
//*******************************************

static void cmdadd(
    char *cmd)

{
	IDB  *idb;
	long *pnt;
	long  rtn;
	long  tcphndl;
	int   cnt;
	char  bufr[128];

	devname[0] = 0;
	cmdport = dataport = 0;
	conlimit = 20;
	maxclients = 100;
    if ((idb = requireinst(cmd, keywordadd)) == NULL)
		return;
    if (instx & (1 << instance))
    {
		srvCmdErrorResp(0, "Instance already exists");
		return;
    }
    if (devname[0] == 0)
    {
		srvCmdErrorResp(0, "DEVICE value not specified");
		return;
    }
    if (cmdport == 0)
    {
		srvCmdErrorResp(0, "CMDPORT value not specified");
		return;
    }
    if (dataport == 0)
    {
		srvCmdErrorResp(0, "DATAPORT value not specified");
		return;
    }
    strupr(devname);
    tcpoparms.tcpport.value = cmdport;
    tcpoparms.conlimit.value = conlimit;
    if ((tcphndl = svcIoOpen(O_IN|O_OUT, devname, &tcpoparms)) < 0)
    {
		sprintf(bufr, "Cannot open TCP device %s", devname);
		srvCmdErrorResp(tcphndl, bufr);
		return;
    }
    if (strcmp(tcpoparms.class.value, "TCP") != 0)
    {
		sprintf(bufr, "Device %s is not a TCP device", devname);
		srvCmdErrorResp(0, bufr);
		svcIoClose(tcphndl, 0);
		return;
    }
    idb = (IDB *)(((instance - 1) << 20) + IDBBASE);
	if ((rtn = svcMemChange(idb, PG_READ|PG_WRITE, sizeof(IDB))) < 0)
	{
		srvCmdErrorResp(0, "Cannot allocate memory for IDB");
		svcIoClose(tcphndl, 0);
		return;
    }
	instx |= (1 << instance);
    strcpy(idb->devname, devname);
	idb->instance = instance;
    idb->cmdport = cmdport;
	idb->dataport = dataport;
    idb->instance = instance;
	idb->maxclients = maxclients;
	pnt = (long *)(idb->cdbtbl);
	cnt = 2;
	do
	{
		*pnt++ = cnt++;
	} while (cnt <= maxclients);
	idb->cdbfree = 1;

	// Set up the command connection QAB

	idb->cmdconqab.qab_func = QFNC_OPEN;
	idb->cmdconqab.qab_vector = VECT_CMDCONNECT;
	idb->cmdconqab.qab_buffer1 = idb->devname;
	idb->cmdconqab.qab_parm = (uchar *)&(idb->cmdconparms);
	idb->cmdconqab.qab_option = O_IN|O_OUT|O_PARTIAL;
	idb->cmdconparms.hndl.desp = (uchar)(PAR_SET|REP_HEXV);
	idb->cmdconparms.hndl.size = 4;
	idb->cmdconparms.hndl.index = IOPAR_NETCONHNDL;
	idb->cmdconparms.hndl.value = tcphndl;
	idb->cmdconparms.timeout.desp = PAR_SET|REP_HEXV;
	idb->cmdconparms.timeout.size = 4;
	idb->cmdconparms.timeout.index = IOPAR_TIMEOUT;
	idb->cmdconparms.timeout.value = 0xFFFFFFFF;
	idb->cmdconparms.addr.desp = PAR_GET|REP_HEXV;
	idb->cmdconparms.addr.size = 4;
	idb->cmdconparms.addr.index = IOPAR_NETRMTNETAR;
	idb->cmdconparms.port.desp = PAR_GET|REP_HEXV;
	idb->cmdconparms.port.size = 4;
	idb->cmdconparms.port.index = IOPAR_NETRMTPORTR;
//	idb->cmdconparms.end = 0;			// This is already 0!
	if ((rtn = svcIoQueue(&(idb->cmdconqab))) < 0)
	{
		srvCmdErrorResp(0, "Error queuing initial passive open");
		giveinstance(idb);
		return;
	}
	sprintf(bufr, STR_MT_FINALMSG"FTPSRV: "ver" - Instance %d created\n"
			"        TCP device: %s, Cmd port: %d, Data port: %d", instance,
			devname, cmdport, dataport);
	srvLogSysLog(0, bufr + 11);
	srvCmdResponse(bufr);
}


static int  fncadddevice(
	arg_data *arg)

{
	char *pnt;

	if (arg->length > 28)
	{
		srvCmdErrorResp(0, "DEVICE name is too long");
		return (FALSE);
	}
	strcpy(devname, arg->val.s);

	pnt = devname + arg->length - 1;
	if (*pnt++ != ':')
		*pnt++ = ':';
	*pnt = 0;
	return (TRUE);
}


static int  fncaddcmdport(
	arg_data *arg)

{
	cmdport = arg->val.n;
	return (TRUE);
}


static int  fncadddataport(
	arg_data *arg)

{
	dataport = arg->val.n;
	return (TRUE);
}


static int  fncaddlimit(
	arg_data *arg)

{
	conlimit = arg->val.n;
	if (conlimit < 5)
		conlimit = 5;
	else if (conlimit > 50)
		conlimit = 50;
	return (TRUE);
}


static int  fncaddmaximum(
	arg_data *arg)

{
	maxclients = arg->val.n;

	if (maxclients < 10)
		maxclients = 10;
	else if (maxclients > 1200)
		maxclients = 1200;
	return (TRUE);
}


static int  fncinstance(
	arg_data *arg)

{
    instance = arg->val.n;
	return (TRUE);
}


//*************************************************
// Function: cmdstatus - Process the STATUS command
// Returned: Nothing
//*************************************************

static void cmdstatus(
    char *cmd)

{
    cmd = cmd;

    srvCmdResponse(STR_MT_FINALERR"FTPSRV "ver" - The STATUS command is not "
			"available yet");

}


//***********************************************************
// Function: requireinst - Do initial command processing when
//		an instance must be specified
// Returned: Address of IDB if OK, NULL if error
//***********************************************************

static IDB *requireinst(
    char     *cmd,
    arg_spec *keyword)

{
	char *cmdpntr[2];

    cmdpntr[0] = cmd;
	cmdpntr[1] = NULL;
    if (!procarg(cmdpntr, PAF_INDIRECT|PAF_EATQUOTE, NULL, keyword, NULL,
		    srvCmdError, NULL, NULL))
		return (NULL);
    banner();
    if (instance == 0)
    {
		srvCmdErrorResp(0, "No instance specified");
		return (NULL);
    }
    if (instance > INSTMAX)		// Valid instance?
    {
		srvCmdErrorResp(0, "Invalid instance number");
		return (NULL);
    }
    return ((IDB *)((instance << 16) + IDBBASE));
}



//**********************************************
// Function: banner - Display the initial banner
// Returned: Nothing
//**********************************************

static void banner(void)

{
    char bufr[80];

    if (instance != 0)
		sprintf(bufr, STR_MT_INTRMDMSG"FTPSRV: "ver" - Unit %d, Instance %d",
				srvUnitNum, instance);
    else
		sprintf(bufr, STR_MT_INTRMDMSG"FTPSRV: "ver" - Unit %d", srvUnitNum);
    srvCmdResponse(bufr);
}


//*********************************************
// Function: giveinstance - Give up an instance
// Returned: Nothing
//*********************************************

void giveinstance(
	IDB *idb)

{
	idb = idb;

}


//*******************************************************
// Function: cmdconnectdone - Signal function called when
//				have an incoming command port connection
// Returned: Nothing
//*******************************************************

static void cmdconnectdone(
	INTDATA idata)

{
	IDB *idb;
	CDB *cdb;
	long rtn;
	long addr;
	long port;
	int  slot;

	idb = (IDB *)(idata.qab - offsetof(IDB, cmdconqab));

	if (idb->cmdconqab.qab_error < 0)	// Error?
	{
		femsg2(prgname, "### error on passive open", idb->cmdconqab.qab_error,
				NULL);

	}

	addr = idb->cmdconparms.addr.value;
	port = idb->cmdconparms.port.value;

	if ((rtn = svcIoQueue(&(idb->cmdconqab))) < 0)
	{
		femsg2(prgname, "### error queuing next passive open", rtn, NULL);

	}

	if ((slot = (idb->cdbfree - 1)) < 0)
	{
		printf("### no more clients!\n");
		exit(1);
	}

	if (srvDebugLevel > 0)
		fprintf(srvDebugStream, "CON: %d %d.%d.%d.%d %d\n", slot + 1,
				((uchar *)&addr)[0], ((uchar *)&addr)[1], ((uchar *)&addr)[2],
				((uchar *)&addr)[3], port);

	cdb = (CDB *)(((long)idb) + CDBBASE + (slot << 14));
	idb->cdbfree = (long)(idb->cdbtbl[slot]);
	idb->cdbtbl[slot] = cdb;

	if ((rtn = svcMemChange(cdb, PG_READ|PG_WRITE, sizeof(CDB))) < 0)
	{
		femsg2(prgname, "### Cannot allocate memory for CDB", rtn, NULL);

	}
	cdb->idb = idb;
	cdb->slot = slot + 1;
	cdb->rmtaddr = addr;
	cdb->rmtcmdport = port;

	cdb->cmdinqab.qab_func = QFNC_INBLOCK;
	cdb->cmdinqab.qab_handle = idb->cmdconqab.qab_handle;
	cdb->cmdinqab.qab_vector = VECT_CMDINPUT;
	cdb->cmdinqab.qab_buffer1 = cdb->cmdinbufr;
	cdb->cmdinqab.qab_count = sizeof(cdb->cmdinbufr);
	cdb->cmdinqab.qab_parm = (uchar *)&(cdb->cmdinparms);
	cdb->cmdinparms.timeout.desp = PAR_SET|REP_HEXV;
	cdb->cmdinparms.timeout.size = 4;
	cdb->cmdinparms.timeout.index = IOPAR_TIMEOUT;
	cdb->cmdinparms.timeout.value = XT_SECOND * 300;

	cdb->cmdlinepnt = cdb->cmdlinebufr;
	cdb->cmdlinecnt = sizeof(cdb->cmdlinebufr);

	cdb->cmdoutqab.qab_func = QFNC_OUTBLOCK;
	cdb->cmdoutqab.qab_handle = idb->cmdconqab.qab_handle;
	cdb->cmdoutqab.qab_vector = VECT_CMDOUTPUT;
	cdb->cmdoutqab.qab_parm = &(cdb->cmdoutparms);
	cdb->cmdoutparms.timeout.desp = PAR_SET|REP_HEXV;
	cdb->cmdoutparms.timeout.size = 4;
	cdb->cmdoutparms.timeout.index = IOPAR_TIMEOUT;
	cdb->cmdoutparms.timeout.value = XT_SECOND * 60;
	cmdoutput(cdb, msg220, sizeof(msg220) -1);
}


static void opendone()

{

}


static void cmdinputdone(
	INTDATA idata)

{
	CDB  *cdb;
	char *pnt;
	char  chr;

	cdb = (CDB *)(idata.qab - offsetof(CDB, cmdinqab));


	if (cdb->cmdinqab.qab_error < 0)
	{


		femsg2(prgname, "Error reading command input", cdb->cmdinqab.qab_error,
				NULL);
	}

	pnt = cdb->cmdinbufr;
	while (cdb->cmdinqab.qab_amount > 0)
	{
		cdb->cmdinqab.qab_amount--;
		chr = *pnt++;
		if (chr == '\r' || chr == 0)
			continue;
		if (chr == '\n')
		{
			*cdb->cmdlinepnt = 0;
			docommand(cdb);
			return;
		}
		else if (cdb->cmdlinecnt > 0)
		{
			cdb->cmdlinecnt--;
			*cdb->cmdlinepnt++ = chr;
		}
	}
}


static void cmdoutputdone(
	INTDATA idata)

{
	CDB *cdb;
	long rtn;

	cdb = (CDB *)(idata.qab - offsetof(CDB, cmdoutqab));

	if ((rtn = svcIoQueue(&(cdb->cmdinqab))) < 0)
	{
		femsg2(prgname, "### error queuing command input", rtn, NULL);
	}
}


static void cmdclosedone(
	INTDATA idata)

{
	idata.qab = idata.qab;

}


static void diskdone(
	INTDATA idata)

{
	idata.qab = idata.qab;

}


void docommand(
	CDB *cdb)

{
	typedef struct
	{	char   name[6];
		void (*func)(CDB *cdb, char *pnt);
	} CMDTBL;

	static CMDTBL cmdtbl[] =
	{	{"USER", ftpcmduser},
		{"PASS", ftpcmdpass},
		{"ACCT", ftpcmdacct},
		{"QUIT", ftpcmdquit},
		{"HELP", ftpcmdhelp},
		{"NOOP", ftpcmdnoop},
		{"CWD" , ftpcmdcwd},
		{"CDUP", ftpcmdcdup},
		{"SMNT", ftpcmdsmnt},
		{"REIN", ftpcmdrein},
		{"PORT", ftpcmdport},
		{"PASV", ftpcmdpasv},
		{"TYPE", ftpcmdtype},
		{"STRU", ftpcmdstru},
		{"MODE", ftpcmdmode},
		{"RETR", ftpcmdretr},
		{"STOR", ftpcmdstor},
		{"STOU", ftpcmdstou},
		{"APPE", ftpcmdappe},
		{"ALLO", ftpcmdallo},
		{"REST", ftpcmdrest},
		{"RNFR", ftpcmdrnfr},
		{"RNTO", ftpcmdrnto},
		{"ABOR", ftpcmdabor},
		{"DELE", ftpcmddele},
		{"RMD" , ftpcmdrmd},
		{"MKD" , ftpcmdmkd},
		{"PWD" , ftpcmdpwd},
		{"LIST", ftpcmdlist},
		{"NLST", ftpcmdnlst},
		{"SITE", ftpcmdsite},
		{"SYST", ftpcmdsyst},
		{"STAT", ftpcmdstat},
		{"XAFL", ftpcmdxafl}
	};

	CMDTBL *tpnt;
	char   *pnt;
	char   *apnt;
	int     cnt;
	char    atom[16];
	char    chr;

	if (srvDebugLevel > 0)
		fprintf(srvDebugStream, "CMD: %d %s\n", cdb->slot, cdb->cmdlinebufr);

	pnt = cdb->cmdlinebufr;
	apnt = atom;
	cnt = 14;
	while ((chr = *pnt) != 0 && isspace(chr))
		pnt++;
	while ((chr = *pnt) != 0 && !isspace(chr))
	{
		pnt++;
		if (--cnt > 0)
			*apnt++ = toupper(chr);
	}
	*apnt = 0;
	while ((chr = *pnt) != 0 && isspace(chr))
		pnt++;
	tpnt = cmdtbl;
	cnt = sizeof(cmdtbl)/sizeof(CMDTBL);
	do
	{
		if (strcmp(tpnt->name, atom) == 0)
		{
			(tpnt->func)(cdb, pnt);
			return;
		}
		tpnt++;
	} while (--cnt > 0);
	cmdoutput(cdb, msg500a, sizeof(msg500a) - 1);
}


//****************************************************
// Function: ftpcmduser - Process the USER FTP command
// Returned: Nothing
//****************************************************

void ftpcmduser(
	CDB  *cdb,
	char *pnt)

{
	pnt = pnt;

	printf("### have USER command\n");

	cmdoutput(cdb, msg331, sizeof(msg331) - 1);
}


//****************************************************
// Function: ftpcmdpass - Process the PASS FTP command
// Returned: Nothing
//****************************************************

void ftpcmdpass(
	CDB  *cdb,
	char *pnt)

{
	pnt = pnt;

	printf("### have PASS command\n");

	cmdoutput(cdb, msg230, sizeof(msg230) - 1);
}


//****************************************************
// Function: ftpcmdacct - Process the ACCT FTP command
// Returned: Nothing
//****************************************************

void ftpcmdacct(
	CDB  *cdb,
	char *pnt)

{
	pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdquit - Process the QUIT FTP command
// Returned: Nothing
//****************************************************

void ftpcmdquit(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdhelp - Process the HELP FTP command
// Returned: Nothing
//****************************************************

void ftpcmdhelp(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdnoop - Process the NOOP FTP command
// Returned: Nothing
//****************************************************

void ftpcmdnoop(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//**************************************************
// Function: ftpcmdcwd - Process the CWD FTP command
// Returned: Nothing
//**************************************************

void ftpcmdcwd(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdcdup - Process the CDUP FTP command
// Returned: Nothing
//****************************************************

void ftpcmdcdup(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdsmnt - Process the SMNT FTP command
// Returned: Nothing
//****************************************************

void ftpcmdsmnt(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdrein - Process the REIN FTP command
// Returned: Nothing
//****************************************************

void ftpcmdrein(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdport - Process the PORT FTP command
// Returned: Nothing
//****************************************************

void ftpcmdport(
	CDB  *cdb,
	char *pnt)

{
	union
	{	long  v;
		uchar c[4];
	} addr;
	union
	{   ushort v;
		uchar  c[2];
	} port;

	if (!getbyteval(&pnt, &addr.c[0], ',') ||
			!getbyteval(&pnt, &addr.c[1], ',') ||
			!getbyteval(&pnt, &addr.c[2], ',') ||
			!getbyteval(&pnt, &addr.c[3], ',') ||
			!getbyteval(&pnt, &port.c[1], ',') ||
			!getbyteval(&pnt, &port.c[0], 0))
	{
		cmdoutput(cdb, msg500b, sizeof(msg500b) - 1);
		return;
	}

	cdb->dataaddr = addr.v;
	cdb->dataport = port.v;

	printf("### addr: %08.8X, port %d\n", addr.v, port.v);

	cmdoutput(cdb, msg200, sizeof(msg200) - 1);
}


//****************************************************
// Function: ftpcmdpasv - Process the PASV FTP command
// Returned: Nothing
//****************************************************

void ftpcmdpasv(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdtype - Process the TYPE FTP command
// Returned: Nothing
//****************************************************

void ftpcmdtype(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdstru - Process the STRU FTP command
// Returned: Nothing
//****************************************************

void ftpcmdstru(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdmode - Process the MODE FTP command
// Returned: Nothing
//****************************************************

void ftpcmdmode(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdretr - Process the RETR FTP command
// Returned: Nothing
//****************************************************

void ftpcmdretr(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdstor - Process the STOR FTP command
// Returned: Nothing
//****************************************************

void ftpcmdstor(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdstou - Process the STOU FTP command
// Returned: Nothing
//****************************************************

void ftpcmdstou(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdappe - Process the APPE FTP command
// Returned: Nothing
//****************************************************

void ftpcmdappe(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdallo - Process the ALLO FTP command
// Returned: Nothing
//****************************************************

void ftpcmdallo(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdrest - Process the REST FTP command
// Returned: Nothing
//****************************************************

void ftpcmdrest(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdrnfr - Process the RNFR FTP command
// Returned: Nothing
//****************************************************

void ftpcmdrnfr(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdrnto - Process the RNTO FTP command
// Returned: Nothing
//****************************************************

void ftpcmdrnto(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdabor - Process the ABOR FTP command
// Returned: Nothing
//****************************************************

void ftpcmdabor(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmddele - Process the DELE FTP command
// Returned: Nothing
//****************************************************

void ftpcmddele(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//**************************************************
// Function: ftpcmdrmd - Process the RMD FTP command
// Returned: Nothing
//**************************************************

void ftpcmdrmd(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//**************************************************
// Function: ftpcmdmkd - Process the MKD FTP command
// Returned: Nothing
//**************************************************

void ftpcmdmkd(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdpwd - Process the PWD FTP command
// Returned: Nothing
//****************************************************

void ftpcmdpwd(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdlist - Process the LIST FTP command
// Returned: Nothing
//****************************************************

void ftpcmdlist(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdnlst - Process the NLST FTP command
// Returned: Nothing
//****************************************************

void ftpcmdnlst(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdsite - Process the SITE FTP command
// Returned: Nothing
//****************************************************

void ftpcmdsite(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdsyst - Process the SYST FTP command
// Returned: Nothing
//****************************************************

void ftpcmdsyst(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdstat - Process the STAT FTP command
// Returned: Nothing
//****************************************************

void ftpcmdstat(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


//****************************************************
// Function: ftpcmdxafl - Process the XAFL FTP command
// Returned: Nothing
//****************************************************

void ftpcmdxafl(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdoutput(cdb, msg502, sizeof(msg502) - 1);
}


void cmdoutput(
	CDB  *cdb,
	char *msg,
	int   len)

{
	long rtn;

	cdb->cmdlinepnt = cdb->cmdlinebufr;
	cdb->cmdlinecnt = sizeof(cdb->cmdlinebufr) - 2;

	if (srvDebugLevel > 0)
		fprintf(srvDebugStream, "RSP: %d %.*s\n", cdb->slot, len - 2, msg);

	cdb->cmdoutqab.qab_count = len;
	cdb->cmdoutqab.qab_buffer1 = msg;
	if ((rtn = svcIoQueue(&(cdb->cmdoutqab))) < 0)
	{
		femsg2(prgname, "### error queuing command output", rtn, NULL);

	}
}


int getbyteval(
	char **ppnt,
	uchar *val,
	int    stopper)

{
	char *pnt;
	long  value;
	char  chr;

	pnt = *ppnt;
	while ((chr = *pnt) != 0 && isspace(chr))
		pnt++;
	value = 0;
	while ((chr = *pnt) != 0 && isdigit(chr))
	{
		pnt++;
		value = value * 10 + (chr & 0x0F);
	}
	pnt++;
	while (chr != 0 && isspace(chr))
	{
		pnt++;
		chr = *pnt;
	}
	*ppnt = pnt;

	printf("### value = %d, stopper = %d (%d)\n", value, chr, stopper);

	*val = (uchar)value;
	return (chr == stopper);
}

