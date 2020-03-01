//--------------------------------------------------------------------------*
// LOGGER.C
// System logging server for XOS
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
#include <errno.h>
#include <signal.h>
#include <xoserr.h>
#include <xostrm.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <xoserrmsg.h>
#include <xosnet.h>
#include <procarg.h>
#include <xosthreads.h>
#include <xosvfyuser.h>
#include <xosservert.h>
#include <xosstr.h>

// This program is the system logging server.  It is initialized as a symbiont
//   with the following command:

//	SYMBIONT LOGGER UNIT=n LOGLEVEL=l LOGFILE=log
//	  Where:
//		UNIT     = Server unit number (default = 1)
//		LOGLEVEL = Specifies the debug logging level, default is 3 if LOGFILE
//			   is specified, 0 otherwise
//				0 = No debug logging
//				1 = Log major events
//				2 = Log all network messages
//		LOGFILE  = Specifies file for log output, if not specified no
//			   log output is generated

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


#define MAJVER  1
#define MINVER  0
#define EDITNUM 0

typedef union
{	long v;
	struct
	{	ushort editnum;
		uchar minor;
		uchar major;
	};
} VERSION;


#define INSTMAX  31
#define CMDTDB   0x00800000
#define TMRTDB   0x00820000

#define ver ver2(MAJVER,MINVER,EDITNUM)
#define ver2(a,b,c) ver3(a,b,c)
#define ver3(maj,min,edit) #maj "." #min "." #edit

// Define the IDB (Instance Data Block)

typedef struct _idb IDB;
struct _idb
{	THDDATA tdb;
	IDB  *next;
	char  logname[32];
	int   instance;				// Instance number
	long  logdate;				// Creation date/time of long file
	long  filehndl;
	long  msghndl;
	uchar commit;
	uchar xxx[3];
};


VERSION xosver;

int   slot;
int   instance;
long  instx;
long  ready;
long  ourpid;
char  logname[32];
long  cmdtdb;
long  version = MAJVER;
long  editnum = EDITNUM;
char *errormsg;
IDB  *firstidb;

static void  banner(void);
static void  cmdadd(char *cmd);
static void  cmdremove(char *cmd);
static void  cmdstatus(char *cmd);
static int   fncaddname(arg_data *arg);
static int   fncinstance(arg_data *arg);
static void  msgthread(void);
static void  putinlog(IDB *idb, ulong procid, char *label, char *text);
static IDB  *requireinst(char *cmd, arg_spec *keyword);
static void  rolllog(IDB *idb, long date, char *text);
static void  showinst(IDB *idb);
static void  tmrthread(void);

arg_spec srvCmdWord[] =
{   {"ADD"   , 0, NULL, srvFncCommand, (long)cmdadd},
    {"REMOVE", 0, NULL, srvFncCommand, (long)cmdremove},
    {"REM"   , 0, NULL, srvFncCommand, (long)cmdremove},
    {"STATUS", 0, NULL, srvFncCommand, (long)cmdstatus},
    {"STA"   , 0, NULL, srvFncCommand, (long)cmdstatus},
    {0}
};

arg_spec keywordadd[] =
{   {"INSTANCE", ASF_VALREQ|ASF_NVAL , NULL, fncinstance, 0},
    {"INS"     , ASF_VALREQ|ASF_NVAL , NULL, fncinstance, 0},
	{"NAME"    , ASF_VALREQ|ASF_LSVAL, NULL, fncaddname , 0},
	{"NAM"     , ASF_VALREQ|ASF_LSVAL, NULL, fncaddname , 0},
	{0}
};

arg_spec keywordins[] =
{   {"INSTANCE", ASF_VALREQ|ASF_NVAL, NULL, fncinstance, 0},
    {"INS"     , ASF_VALREQ|ASF_NVAL, NULL, fncinstance, 0},
    {0}
};

static struct
{	time8_parm dt;
	uchar      end;
} loparms =
{	{PAR_GET|REP_DT, 8, IOPAR_CDATE}
};

char prgname[] = "LOGGER";
char srvname[] = "LOGGER";

uchar debugflg;
uchar havetmr = FALSE;

void mainalt(
    char *args)

{
	srvInitialize(args, (void *)CMDTDB, 0x2000, 70, 50, MAJVER, MINVER,
			EDITNUM);
}


//**********************************************************
// Function: srvSetup2 - Called by server routine at startup
//				before response is sent
// Returned: Nothing
//**********************************************************

void srvSetup1(void)

{
	debugflg = (srvReqName[0] == 0);
    ourpid = svcSysGetPid();			// Get our process ID
}


//**********************************************************
// Function: srvSetup2 - Called by server routine at startup
//				after response is sent
// Returned: Nothing
//**********************************************************

void srvSetup2(void)

{

}


//********************************************************
// Function: srvSetupCmd - Called by server routine before
//				a command is processed
// Returned: Nothing
//********************************************************

void srvSetupCmd(void)

{
	cmdtdb = (long)thdData;
	instance = 0;
}


//********************************************************
// Function: srvFinishCmd - Called by server routine after
//				a command is processed
// Returned: Nothing
//********************************************************

void srvFinishCmd(void)

{

}


//*****************************************************
// Function: srvMessage - Called by server routine when
//				non-server message received
// Returned: Nothing
//*****************************************************

void srvMessage(
    char *msg,
    int   size)

{
	msg = msg;
	size = size;
}


//*******************************************
// Function: cmdadd - Process the ADD command
// Returned: Nothing
//*******************************************

static void cmdadd(
    char *cmd)

{
	IDB  *idb;
	long  rtn;
	int   inst;
	char  bufr[256];

	logname[0] = 0;
    if ((idb = requireinst(cmd, keywordadd)) == NULL)
		return;
    if (instx & (1 << instance))
    {
		srvCmdErrorResp(0, "Instance already exists", NULL, srvMsgDst);
		return;
    }
    if (logname[0] == 0)
    {
		srvCmdErrorResp(0, "NAME value not specified", NULL, srvMsgDst);
		return;
    }
    strupr(logname);

	// Create the timer thread if it does not already exist

	if (!havetmr)
	{
	    if ((rtn = thdCtlCreate(TMRTDB + 0x200, 0x3200, 0, tmrthread, NULL,
				0)) < 0)
    	{
			srvCmdErrorResp(rtn, "Error message connection thread - Cannot "
					"continue", NULL, srvMsgDst);
			return;
		}
		havetmr = TRUE;
	}

    // Create a message thread which reads IPM messages from clients.
	//   The data area for this thread contains the IDB.

    idb = (IDB *)((instance << 27) + 0x200);
    if ((rtn = thdCtlCreate((long)idb, 0x3200, sizeof(IDB), msgthread,
			NULL, 0)) < 0)
    {
		srvCmdErrorResp(rtn, "Error message connection thread - Cannot "
				"continue", NULL, srvMsgDst);
		return;
    }
	inst = instance;
	ready = 0;
	while (ready == 0)
		thdCtlSuspend(NULL, -1);
	if (ready < 0)
	{
		srvCmdErrorResp(rtn, errormsg, NULL, srvMsgDst);
		return;
    }
	instx |= (1 << inst);
	sprintf(bufr, STR_MT_FINALMSG"LOGGER: "ver" - Instance %d created\n"
			"        Log name: %s\n", inst, logname);
	srvCmdResponse(bufr, srvMsgDst);
}


//*************************************************
// Function: fncaddname - Process the NAME argument
//				for the ADD command
// Returned: TRUE if successful, FALSE if error
//*************************************************

static int  fncaddname(
	arg_data *arg)

{
	if (arg->length > 28)
	{
		srvCmdErrorResp(0, "Log NAME is too long", NULL, srvMsgDst);
		return (FALSE);
	}
	strcpy(logname, arg->val.s);
	return (TRUE);
}


//*************************************************
// Function: cmdremove - Process the REMOVE command
// Returned: Nothing
//*************************************************

static void  cmdremove(
	char *cmd)

{
	cmd = cmd;
}


//*************************************************
// Function: cmdstatus - Process the STATUS command
// Returned: Nothing
//*************************************************

static void cmdstatus(
    char *cmd)

{
	char *cmdpntr[2];
///	IDB  *idb;

	ulong bits;
	int   inst;
	int   cnt;
	char  bufr[256];

	static char insthead[] = STR_MT_INTRMDMSG"\n  Inst Tcp Device   IP "
			"Address   Cmd Port Data Port Login Clients";

    cmdpntr[0] = cmd;
	cmdpntr[1] = NULL;
    if (!procarg(cmdpntr, PAF_INDIRECT|PAF_EATQUOTE, NULL, keywordins, NULL,
		    srvCmdError, NULL, NULL))
		return;
    banner();
    if (instance == 0)
    {
		srvCmdResponse(insthead, srvMsgDst);
		bits = instx;
		inst = 1;
		cnt = 0;
		while ((bits >>= 1) != 0)
		{
			if (bits & 0x01)
			{
///				showinst((IDB *)((inst << 27) + 0x200));
				cnt++;
			}
			inst++;
		}
		sprintf(bufr, STR_MT_FINALMSG"   There %s %d instance%s", 
				(cnt == 1) ? "is" : "are", cnt, (cnt == 1) ?
				"" : "s");
	    srvCmdResponse(bufr, srvMsgDst);
	}
}

/*
static void showinst(
	IDB *idb)

{
	char bufr[256];

	sprintf(bufr, STR_MT_INTRMDMSG"   %3d %-12s %-15s %5d %9d   %s %7d",
			idb->instance, idb->devname, fmtaddr(idb->ipaddr.c),
			idb->cmdport.v, idb->dataport.v, (idb->nologin) ? " No" : "Yes",
			idb->numclients);
    srvCmdResponse(bufr, srvMsgDst);
}
*/


//******************************************************
// Function: fncinstance - Process the INSTANCE argument
//				for all commands
// Returned: TRUE if successful, FALSE if error
//******************************************************

static int  fncinstance(
	arg_data *arg)

{
    instance = arg->val.n;
	return (TRUE);
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
		srvCmdErrorResp(0, "No instance specified", NULL, srvMsgDst);
		return (NULL);
    }
    if (instance > INSTMAX)		// Valid instance?
    {
		srvCmdErrorResp(0, "Invalid instance number", NULL, srvMsgDst);
		return (NULL);
    }
    return ((IDB *)((instance << 27) + 0x200));
}


//**********************************************
// Function: banner - Display the initial banner
// Returned: Nothing
//**********************************************

static void banner(void)

{
    char bufr[80];

    if (instance != 0)
		sprintf(bufr, STR_MT_INTRMDMSG"LOGGER: "ver" - Unit %d, Instance %d",
				srvUnitNum, instance);
    else
		sprintf(bufr, STR_MT_INTRMDMSG"LOGGER: "ver" - Unit %d", srvUnitNum);
    srvCmdResponse(bufr, srvMsgDst);
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


//*******************************************
// Function: tmrthread - Main thread function
//				for the timer thread
// Returned: Never returns
//*******************************************

static void tmrthread(void)

{
	IDB *idb;
	long rtn;

	while (TRUE)
	{
		idb = firstidb;
		while (idb != NULL)
		{
			if (idb->commit)
			{
				idb->commit = FALSE;
				if ((rtn = svcIoCommit(idb->filehndl)) < 0)
				{
					svcIoClose(0, idb->filehndl);
					idb->filehndl = rtn;
				}
			}
			idb = idb->next;
		}
		thdCtlSuspend(NULL, XT_SECOND);
	}
}


//**********************************************************
// Function: msgthread - Main thread function for the thread
//				that reads IPM messages
// Returned: Never returns
//**********************************************************

static void msgthread(void)

{
	time_s bgndt;
	IDB   *idb;
	char  *pnt;
	long   rtn;
	char   buffer[300];
	char   msgbfr[2000];
	char   srcbfr[64];
	char   filename[48];
	struct
	{	lngstr_parm src;
		uchar       end;
	}      msginpparms =
			{{(PAR_GET|REP_STR), 0, IOPAR_MSGRMTADDRR, NULL, 0, 64, 0}};

	msginpparms.src.buffer = srcbfr;
	idb = (IDB *)thdData;
    strcpy(idb->logname, logname);
	idb->instance = instance;

	if ((pnt = strchr(logname, ':')) != NULL)
		pnt++;
	else
		pnt = logname;
	sprintf(filename, "IPM:SYS^%sLOG", pnt);
	if ((idb->msghndl = svcIoOpen(O_IN|O_OUT, filename, NULL)) < 0)
	{
		errormsg = "Error opening IPM device - Cannot continue";
		ready = idb->msghndl;
		thdCtlWake(cmdtdb);
		thdCtlTerminate();
	}
    svcSysDateTime(T_GTXDTTM, &bgndt);	// Get current date and time

	// Open (or create) the log file

	sprintf(filename, "%s.LOG", logname);
	while ((idb->filehndl = svcIoOpen(O_IN|O_OUT|O_XWRITE|O_APPEND, filename,
			&loparms)) < 0)
	{
		if (idb->filehndl == ER_FILNF)
		{
			if ((idb->filehndl = svcIoOpen(O_IN|O_OUT|O_CREATE|O_XWRITE,
					filename, &loparms)) > 0)
			{
				idb->logdate = bgndt.date;
				putinlog(idb, ourpid, prgname, "Log file created: No log file "
						"when logging started");
				break;
			}
		}
		errormsg = "Error opening log file - Cannot continue";
		ready = idb->filehndl;
		thdCtlWake(cmdtdb);
		thdCtlTerminate();
	}
	idb->logdate = loparms.dt.value.date;
	putinlog(idb, ourpid, prgname, "Logging started");

	// Here with everything ready - link this IDB into the timer list and let
	//   the command thread continue

	idb->next = firstidb;
	firstidb = idb;
	ready = 1;
	thdCtlWake(cmdtdb);
	while (TRUE)
	{
		if ((rtn = thdIoInBlockP(idb->msghndl, msgbfr, sizeof(msgbfr),
				&msginpparms)) < 0)		// Get a message
		{

			continue;
		}
		if (msgbfr[0] != MT_SYSLOG)
			continue;
		msgbfr[rtn] = '\0';
		switch (msgbfr[1])
		{
		 case 1:							// Event at address
			sprintf(buffer, "%4.4s event, Address = %04.4X:%08.8lX, "
					"Data = %08.8lX", &(msgbfr[12]), *((short *)&srcbfr[3]),
					*((short *)&srcbfr[1]), *((short *)&msgbfr[20]),
					*((long *)&msgbfr[16]), *((long *)&msgbfr[22]));
	        goto putin;

		 case 2:							// Message from process
			sprintf(buffer, "Message from process \"%.160s\"", &msgbfr[12]);
			goto putin;

		 case 3:							// General message
			if (*(long *)&srcbfr[1] == 0x00020001)
				putinlog(idb, *((long *)&msgbfr[12]), &msgbfr[4], &msgbfr[16]);
			else
				putinlog(idb, *((long *)&srcbfr[1]), &msgbfr[4], &msgbfr[12]);
			break;

		 case 0xFF:							// Roll log file
			if (rtn > 12 && msgbfr[12] != 0)
				putinlog(idb, *((long *)&srcbfr[1]), &msgbfr[4], &msgbfr[12]);
			rolllog(idb, idb->logdate, "Requested");
			break;

		 default:
			sprintf(buffer, "Illegal log message #%d\r\n", &msgbfr[1]);
		 putin:
			putinlog(idb, *((long *)&srcbfr[1]), &msgbfr[4], buffer);
			break;
		}
	}
}


//*********************************************
// Function: rolllog - Renames current log file
//				and creates new log file
// Returned: Nothing
//*********************************************

void rolllog(
	IDB  *idb,
	long  date,
	char *text)

{
	time_s dt;
	char  *pnt;
	char   extension[16];
	char   logname[128];
	char   arcname[128];
	char   bufr[300];

	dt.date = date;
	idb->logdate = 0x7FFFFFFF;			// Needed to keep putinlog from going
										//   endlessly recurive!
	sprintf(bufr, "Log file terminated: %s", text);
	putinlog(idb, ourpid, prgname, bufr);

	sdt2str(extension, "%L-%r-%d", (time_sz *)&dt);

	sprintf(logname, "%s.LOG", idb->logname);
	pnt = arcname + sprintf(arcname, "%sLOG.%s", idb->logname,
			extension);
	svcIoClose(idb->filehndl, 0);
	while ((idb->filehndl = svcIoRename(0, logname, arcname, NULL)) < 0)
	{
		if (pnt[0] == 0)
		{
			pnt[0] = 'a';
			pnt[1] = 0;
		}
		else if (pnt[0] == 'z')
			return;
		else
			(pnt[0])++;
	}
	if ((idb->filehndl = svcIoOpen(O_IN|O_OUT|O_CREATE|O_XWRITE, logname,
			&loparms)) > 0)
	{
		sprintf(bufr, "Log file created: %s", text);
		putinlog(idb, ourpid, prgname, bufr);
	}
	idb->logdate = loparms.dt.value.date;
}


//***************************************************
// Function: putinlog - Write data prefix to log file
// Returned: TRUE if OK, FALSE if error
//***************************************************

void putinlog(
	IDB  *idb,
    ulong procid,
    char *label,
    char *text)

{
	char   *dpnt;
	char   *spnt;
	time_sz dt;
	long    rtn;
	char    bufr[400];
	char    chr;

	if (idb->filehndl < 0)
		return;
	svcSysDateTime(T_GTXDTTM, &dt);		// Get current date and time
	if (idb->logdate < dt.date)
		rolllog(idb, idb->logdate, "Date changed");
	bufr[0] = '$';
	dpnt = bufr + 1 + sdt2str(bufr + 1, "%D-%3n-%y %H:%m:%s.%f", &dt);
	dpnt += sprintf(dpnt, " %5d.%-3d %-8.8s ", procid >> 16, procid&0xFFFF,
			label);
	spnt = text;
	while ((chr = *spnt++) != 0 && dpnt < (bufr + 396))
	{
		if (chr == '\r')
		{
			*dpnt++ = '\r';
			*dpnt++ = '\n';
			if (*spnt == '\n')
				spnt++;
		}
		else if (chr == '\n')
		{
			*dpnt++ = '\r';
			*dpnt++ = '\n';
			if (*spnt == '\r')
				spnt++;
		}
		else
			*dpnt++ = chr;
	}
	*dpnt++ = '\r';
	*dpnt++ = '\n';
	if ((rtn = svcIoOutBlock(idb->filehndl, bufr, dpnt - bufr)) < 0)
		idb->filehndl = rtn;
	else
		idb->commit = TRUE;
}
