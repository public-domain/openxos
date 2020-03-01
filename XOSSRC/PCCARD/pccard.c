//===========================================
// PCCARD - PC-card management server for XOS
// Written by John Goltz
//===========================================

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

#include <STDIO.H>
#include <STDDEF.H>
#include <STDLIB.H>
#include <STRING.H>
#include <SIGNAL.H>
#include <CTYPE.H>
#include <XOS.H>
#include <XOSSTR.H>
#include <XOSTIME.H>
#include <ERRMSG.H>
#include <XOSSVC.H>
#include <XOSTRM.H>
#include <XOSACCT.H>
#include <XOSERR.H>
#include <XOSPROCARG.H>
#include <XOSTHREADS.H>
#include <XOSSERVERT.H>
#include "PCCARD.H"

// A seperate server instance is created for each PC-card controller device
//   which is handled.  Each instance is implemented as a single thread
//   which handles event signals from the PC-card controller, obtains the
//   status and configuration of the currently inserted PC-card, initializes
//   the PC-card as needed, ands loads and initializes the necessary device
//   drivers for the PC-card.

// Each instance is allocated an IDB (Instance Data Block) which contains all
//   data associated with the instance, including the THDDATA structure for
//   the instance's thread. The THDDATA structure is the first item in the
//   IDB so its address is the same as the IDB's address.
// Note that the 0x800 bytes below each TDB contains the stack for the thread.

// The address for each IDB is calculated by adding 0x408800 to the instance
//   number times 0x4000.  The base thread data area (TDB and stack) is at
//   0x400000 and is 0x4000 (16KB) bytes long.

// Instance numbers start at 0 and are allocated automatically by the server.
//   Instance numbers are not visible to the user. instances are identified
//   by the name of the registered PC-card device.

// An instance is created with a REGISTER server command which has the
//   following format:
//	SERVER PCCARD REGISTER PCCARD=pccdev
// Once an instance has been created, device configurations are specified
//   with the DEVICE command as follows:
//	SERVER PCCARD DEVICE PCCARD=pccdev:socket CLASS=class UNIT=unit
//		TYPE=type IOREG=ioreg INT=int

// Currently, the only class supported is DISK and the only TYPE supported
//   is HDKA.

// Each possible socket on the PC-card device is described by a SOCBLK
//   structure.  Each possible type of device which can be inserted into a
//   socket is described by a TYPEBLK structure.  There is a linked list
//   of TYPEBLK structures for each socket.

#define MAJVER  0
#define MINVER  1
#define EDITNUM 0


// Data used when decodeing CIS data

CFG     cfg;			// Place to store configuration data
CFGENT  dftcfg;			// Default configuration entry values
CFGENT *curcfg;			// Current configuration entry values (points
				//   to entry in cfg.cfgtbl)

struct cmddata
{   THDDATA cmddata;
    long    xxx;
} cmddata;

struct
{   text8_parm  class;
    char        end;
} pccoparms =
{   {PAR_GET|REP_TEXT, 8, IOPAR_CLASS}
};

struct
{   byte4_char  numsoc;
    char        end;
} pccochars =
{   {PAR_GET|REP_DECV, 4, "NUMSOC"}
};

char  bufr[320];
char  prgname[] = "PCCARD";
char  srvname[] = "PCCard";

char  class[16];
char  type[8];
char  dosnames[12];
int   index;
int   unit;
int   ioreg;
int   intreq;
int   pccardsoc;
char  pccardname[32];


static int   pcchndl;



static int   instance;
static ulong instx;
static int   numins;

static char *cmdpntr[2];

static IDB  *idbhead;


static void banner(void);
static int  begincmd(char *cmd, arg_spec *keyword, int require);
static void cmddevice(char *cmd);
static void cmdregister(char *cmd);
static void cmdremove(char *cmd);
static void cmdstatus(char *cmd);
static void devicestatus(IDB *idb, int num);
static IDB *findpccard(int report);
static int  fncpccard(arg_data *arg);
static int  fncdevclass(arg_data *arg);
static int  fncdevdosnames(arg_data *arg);
static int  fncdevindex(arg_data *arg);
static int  fncdevunit(arg_data *arg);
static int  fncdevtype(arg_data *arg);
static int  fncdevioreg(arg_data *arg);
static int  fncdevint(arg_data *arg);
static void pccsignal(INTDATA intdata);
static void pccthread(void);
static int  storedev(char *bufr, char *name, int length);


arg_spec srvCmdWord[] =
{   {"REGISTER", 0, NULL, srvFncCommand, (long)cmdregister},
    {"REG"     , 0, NULL, srvFncCommand, (long)cmdregister},
    {"REMOVE"  , 0, NULL, srvFncCommand, (long)cmdremove},
    {"REM"     , 0, NULL, srvFncCommand, (long)cmdremove},
    {"DEVICE"  , 0, NULL, srvFncCommand, (long)cmddevice},
    {"DEV"     , 0, NULL, srvFncCommand, (long)cmddevice},
    {"STATUS"  , 0, NULL, srvFncCommand, (long)cmdstatus},
    {"STA"     , 0, NULL, srvFncCommand, (long)cmdstatus},
    {0}
};

arg_spec keywordregister[] =
{   {"PCCARD", ASF_VALREQ|ASF_LSVAL, NULL, fncpccard, 0},
    {"PCC"   , ASF_VALREQ|ASF_LSVAL, NULL, fncpccard, 0},
    {0}
};

arg_spec keyworddevice[] =
{   {"PCCARD"  , ASF_VALREQ|ASF_LSVAL, NULL, fncpccard     , 0},
    {"PCC"     , ASF_VALREQ|ASF_LSVAL, NULL, fncpccard     , 0},
    {"CLASS"   , ASF_VALREQ|ASF_LSVAL, NULL, fncdevclass   , 0},
    {"INDEX"   , ASF_VALREQ|ASF_NVAL , NULL, fncdevindex   , 0},
    {"UNIT"    , ASF_VALREQ|ASF_NVAL , NULL, fncdevunit    , 0},
    {"TYPE"    , ASF_VALREQ|ASF_LSVAL, NULL, fncdevtype    , 0},
    {"DOSNAMES", ASF_VALREQ|ASF_LSVAL, NULL, fncdevdosnames, 0},
    {"IOREG"   , ASF_VALREQ|ASF_NVAL , NULL, fncdevioreg   , 0},
    {"INT"     , ASF_VALREQ|ASF_NVAL , NULL, fncdevint     , 0},
    {0}
};


//*********************************************************************
// Function: mainalt - Main program (alternate command line processing)
// Returned: Never returns
//*********************************************************************

void mainalt(
    char *args)

{
    setvector(60, 0x01, pccsignal);	// Set status change vector (level 1
					//   so it cannot interrupt a thread)
    srvInitialize(args, (void *)0x400000, 0x4000, 70, 30, MAJVER, MINVER,
	    EDITNUM);			// Initialize the server routines
}					//   (Use a 16KB control stack to
					//   allow enough space to execute
					//   our signal routines)

//*****************************************************************
// Function: srvSetup1 - Called after server set up is complete but
//		 before final response message is sent to INIT
//*****************************************************************

void srvSetup1(void)

{

}

//**************************************************
// Function: srvSetup1 - Called after final response
//		message is sent to INIT
//**************************************************

void srvSetup2(void)

{

}

//*************************************************************
// Function: srvSetupCmd - Called before a command is processed
// Returned: Nothing
//*************************************************************

void srvSetupCmd(void)

{
    pccardname[0] = class[0] = type[0] = dosnames[0] = 0;
    index = unit = ioreg = intreq = -1;
}

//*************************************************************
// Function: srvFinishCmd - Called after a command is processed
// Returned: Nothing
//*************************************************************

void srvFinishCmd(void)

{

}

//*********************************************************
// Function: srvMessage - Called for all IPM messages which
//		are not server command (MT_SRVCMD) messages
// Returned: Nothing
//*********************************************************

void srvMessage(
    char *msg,
    int   size)

{
    msg = msg;
    size = size;
}

//*****************************************************
// Function: cmdregister - Process the REGISTER command
// Returned: Nothing
//*****************************************************

static void cmdregister(
    char *cmd)

{
    IDB     *idb;
    SOCBLK **sbpnt;
    SOCBLK  *socblk;
    long     rtn;
    long     mask;
    int      cnt;
    int      num;

    static struct
    {	byte4_parm data;
	byte4_parm vector;
	char       end;
    } sigparms =
    {	{PAR_SET|REP_HEXV, 4, IOPAR_SIGDATA , 0},
	{PAR_SET|REP_HEXV, 4, IOPAR_SIGVECT1, 60}
    };


    if (!begincmd(cmd, keywordregister, TRUE))
	return;
    if (findpccard(FALSE) != NULL)
    {
	srvCmdErrorResp(0, "PC-card already registered", NULL, srvMsgDst);
	return;
    }
    if (pccardsoc >= 0)
    {
	srvCmdErrorResp(0, "PCCARD value cannot include a socket number", NULL,
		srvMsgDst);
	return;
    }
    if ((pcchndl = svcIoOpen(O_IN|O_OUT, pccardname, &pccoparms)) < 0)
    {
	sprintf(bufr, "Cannot open PC-card device %s", pccardname);
	srvCmdErrorResp(pcchndl, bufr, NULL, srvMsgDst);
	return;
    }
    if (strncmp(pccoparms.class.value, "PCC", 3) != 0)
    {
	sprintf(bufr, "Device %s is not a PC-card device", pccardname);
	srvCmdErrorResp(0, bufr, NULL, srvMsgDst);
	svcIoClose(pcchndl, 0);
	return;
    }
    if ((rtn = svcIoDevChar(pcchndl, &pccochars)) < 0)
    {
	sprintf(bufr, "Error getting initial %s device characteristics",
		pccardname);
	srvCmdErrorResp(rtn, bufr, NULL, srvMsgDst);
	return;
    }
    if (pccochars.numsoc.value < 1 || pccochars.numsoc.value > 8)
    {
	sprintf(bufr, "Invalid number of socket reported by %s device",
		pccardname);
	srvCmdErrorResp(pcchndl, bufr, NULL, srvMsgDst);
	return;
    }
    instance = 0;
    mask = 0x01;
    while ((instx & mask) != 0)
    {
	if (instance >= 15)
	{
	    srvCmdErrorResp(rtn, "Too many PC-card devices registered", NULL,
		    srvMsgDst);
	    svcIoClose(pcchndl, 0);
	    return;
	}
	instance++;
	mask <<= 1;
    }
    idb = (IDB *)((char *)(instance * 0x4000 + 0x408800));

    if ((rtn = thdCtlCreate((long)&(idb->tdb), 0x800, sizeof(IDB) +
	    (pccochars.numsoc.value - 1) * sizeof(SOCBLK *), pccthread,
	    NULL, 0)) < 0)
    {
	srvCmdErrorResp(rtn, "Error creating thread for instance", NULL,
		srvMsgDst);
	svcIoClose(pcchndl, 0);
	return;
    }
    sigparms.data.value = (long)idb;
    if ((rtn = svcIoInBlockP(pcchndl, NULL, 0, &sigparms)) < 0)
    {
	sprintf(bufr, "Error enabling status change signal for %s", pccardname);
	srvCmdErrorResp(rtn, bufr, NULL, srvMsgDst);
	return;
    }
    instx |= mask;
    numins++;
    strcpy(idb->pccname, pccardname);
    idb->pcchndl = pcchndl;

    // Allocate and set up the socket data blocks for the PC-card device

    cnt = idb->numsoc = pccochars.numsoc.value;
    num = 0;
    sbpnt = &(idb->soctbl);
    do
    {
	if ((socblk = (SOCBLK *)getmem(sizeof(SOCBLK))) == NULL)
	    return;
	socblk->num = num++;
	socblk->curtype = 0;
	socblk->firsttype = NULL;
	*sbpnt++ = socblk;
    } while (--cnt > 0);
    idb->next = idbhead;
    idbhead = idb;
    sprintf(bufr, "xPCCARD: PC-card %s registered (%d socket%s supported)",
	    pccardname, pccochars.numsoc.value, (pccochars.numsoc.value != 1) ?
	    "s" : "");
    bufr[0] = MT_FINALMSG;
    srvCmdResponse(bufr, srvMsgDst);
}

//*************************************************
// Function: cmddevice - Process the DEVICE command
// Returned: Nothing
//*************************************************

// This command specifies a device which is to be set up when a matching
//   PC-card is detected.  The command parameters required depend on the
//   device class and type as follows:
//     Class  Type   Required command parameters
//     DISK   HDKA   UNIT, IOREG, INT

static void cmddevice(
    char *cmd)

{
    IDB    *idb;

    if (!begincmd(cmd, keyworddevice, TRUE))
	return;
    if ((idb = findpccard(TRUE)) == NULL)
	return;
    if (pccardsoc < 0)
    {
	srvCmdErrorResp(0, "No socket number specified in PCCARD value", NULL,
		srvMsgDst);
	return;
    }
    if (pccardsoc >= idb->numsoc)
    {
	srvCmdErrorResp(0, "Socket number is too large", NULL, srvMsgDst);
	return;
    }

    if (strcmp(class, "DISK") == 0)
	devicedisk(idb, idb->soctbl[pccardsoc]);
    else
    {
	sprintf(bufr, "Device class %s is not supported", class);
	srvCmdErrorResp(0, bufr, NULL, srvMsgDst);
    }
}

//*************************************************
// Function: cmdremove - Process the REMOVE command
// Returned: Nothing
//*************************************************

static void cmdremove(
    char *cmd)

{
    cmd = cmd;

    banner();
    srvCmdErrorResp(0, "Remove command is not available yet", NULL, srvMsgDst);
}

//*************************************************
// Function: cmdstatus - Process the STATUS command
// Returned: Nothing
//*************************************************

static void cmdstatus(
    char *cmd)

{
    int  num;
    IDB *idb;

    if (!begincmd(cmd, keywordregister, FALSE))
	return;
    if (pccardname[0] == 0)		// PCCARD specified?
    {					// No
	if (idbhead == NULL)
	    strcpy(bufr, "x        No PC-card devices are registered");
	else
	{
	    sprintf(bufr, STR_MT_INTRMDMSG "        %d PC-card device%s "
		    "registered\n        Device    Number of sockets", numins,
		    (numins != 1) ? "s are" : " is");
	    srvCmdResponse(bufr, srvMsgDst);
	    idb = idbhead;
	    for (;;)
	    {
		sprintf(bufr, STR_MT_INTRMDMSG "        %-12s %d", idb->pccname,
			idb->numsoc);
		if (idb->next == NULL)
		    break;
		srvCmdResponse(bufr, srvMsgDst);
		idb = idb->next;
	    }
	}
    }
    else				// If PCCARD specified
    {
	if ((idb = findpccard(TRUE)) == NULL)
	    return;
	if (pccardsoc >= 0)
	    devicestatus(idb, pccardsoc);
	else
	{
	    num = 0;
	    for (;;)
	    {
		devicestatus(idb, num);
		if (++num >= idb->numsoc)
		    break;
		bufr[0] = MT_INTRMDMSG;
		srvCmdResponse(bufr, srvMsgDst);
	    }
	}
    }
    bufr[0] = MT_FINALMSG;
    srvCmdResponse(bufr, srvMsgDst);
}

//******************************************************
// Function: devicestatus - Display status of one device
//		associated with a PC-card device socket
// Returned: Nothing
//******************************************************

static void devicestatus(
    IDB *idb,
    int  num)

{
    SOCBLK  *socblk;
    TYPEBLK *typeblk;

    static char *nametbl[] =
    {	"????????",
	"HDKADISK"
    };

    sprintf(bufr, STR_MT_INTRMDMSG "        Status for socket %s%d",
	    idb->pccname, num);
    srvCmdResponse(bufr, srvMsgDst);

    socblk = idb->soctbl[num];
    if (socblk->firsttype == NULL)
    {
	strcpy(bufr, STR_MT_INTRMDMSG "          No devices are associated "
		"with this socket");
	return;
    }
    srvCmdResponse(STR_MT_INTRMDMSG "          Devices associated with this "
	    "socket:", srvMsgDst);

    typeblk = socblk->firsttype;
    for (;;)
    {
	sprintf(bufr, STR_MT_INTRMDMSG "            Class=%s  Type=%4.4s  "
		"Unit=%d  Name=%s \n                IOreg=0x%04.4X  Int=%d",
		nametbl[typeblk->type] + 4, nametbl[typeblk->type],
		typeblk->unit, typeblk->name, typeblk->ioreg, typeblk->intreq);
	if (typeblk->next == NULL)
	    return;
	srvCmdResponse(bufr, srvMsgDst);
    }
}

//**************************************************************
// Function: fncpccard - Process PCCARD keyword for all commands
// Returned: TRUE if OK, FALSE if error
//**************************************************************

static int fncpccard(
    arg_data *arg)

{
    char *colon;
    char  chr;

    if (arg->length > 30)
    {
	srvCmdErrorResp(0, "PCCARD value is longer than 30 characters",
		NULL, srvMsgDst);
	return (FALSE);
    }
    strupper(arg->val.s);
    pccardsoc = -1;			// Assume no socket value given
    if ((colon = strchr(arg->val.s, ':')) == NULL) // Have a colon?
	strmov(strmov(pccardname, arg->val.s), ":"); // No
    else
    {
	strmov(strnmov(pccardname, arg->val.s, colon - arg->val.s), ":");
	if (*(++colon) != 0)
	{
	    pccardsoc = 0;
	    while ((chr = *colon++) != 0 && isdigit(chr))
		pccardsoc = pccardsoc * 10 + (chr & 0x0F);
	    if (chr != 0)
	    {
		srvCmdErrorResp(0, "Invalid socket number value specified",
			NULL, srvMsgDst);
		return (FALSE);
	    }
	}
    }
    return (TRUE);
}

//*********************************************************************
// Function: fncdevclass - Process CLASS keyword for the DEVICE command
// Returned: TRUE if OK, FALSE if error
//*********************************************************************

static int fncdevclass(
    arg_data *arg)

{
    if (arg->length > 15)
    {
	srvCmdErrorResp(0, "CLASS value is longer than 15 characters",
		NULL, srvMsgDst);
	return (FALSE);
    }
    strupper(arg->val.s);
    strcpy(class, arg->val.s);
    return (TRUE);
}

//***************************************************************************
// Function: fncdevdosnames - Process DOSNAMES keyword for the DEVICE command
// Returned: TRUE if OK, FALSE if error
//***************************************************************************

static int fncdevdosnames(
    arg_data *arg)

{
    if (arg->length > 8)
    {
	srvCmdErrorResp(0, "DOSNAMES value is longer than 8 characters",
		NULL, srvMsgDst);
	return (FALSE);
    }
    strupper(arg->val.s);
    strcpy(dosnames, arg->val.s);
    return (TRUE);
}

//*********************************************************************
// Function: fncdevindex - Process INDEX keyword for the DEVICE command
// Returned: TRUE if OK, FALSE if error
//*********************************************************************

static int fncdevindex(
    arg_data *arg)

{
    index = arg->val.n;
    return (TRUE);
}

//*******************************************************************
// Function: fncdevunit - Process UNIT keyword for the DEVICE command
// Returned: TRUE if OK, FALSE if error
//*******************************************************************

static int fncdevunit(
    arg_data *arg)

{
    unit = arg->val.n;
    return (TRUE);
}

//*******************************************************************
// Function: fncdevtype - Process TYPE keyword for the DEVICE command
// Returned: TRUE if OK, FALSE if error
//*******************************************************************

static int fncdevtype(
    arg_data *arg)

{
    if (arg->length != 4)
    {
	srvCmdErrorResp(0, "TYPE value is not 4 characters in length",
		NULL, srvMsgDst);
	return (FALSE);
    }
    strupper(arg->val.s);
    strcpy(type, arg->val.s);
    return (TRUE);
}

//*********************************************************************
// Function: fncdevioreg - Process IOREG keyword for the DEVICE command
// Returned: TRUE if OK, FALSE if error
//*********************************************************************

static int fncdevioreg(
    arg_data *arg)

{
    ioreg = arg->val.n;
    return (TRUE);
}

//*****************************************************************
// Function: fncdevint - Process INT keyword for the DEVICE command
// Returned: TRUE if OK, FALSE if error
//*****************************************************************

static int fncdevint(
    arg_data *arg)

{
    intreq = arg->val.n;
    return (TRUE);
}

//***************************************************
// Function: begincmd - Do initial command processing
// Returned: TRUE if normal, FALSE if error
//***************************************************

static int begincmd(
    char     *cmd,
    arg_spec *keyword,
    int       require)

{
    cmdpntr[0] = cmd;
    if (!procarg(cmdpntr, PAF_INDIRECT|PAF_EATQUOTE, NULL, keyword, NULL,
	    srvCmdError, NULL, NULL))
	return (FALSE);
    banner();
    if (require && pccardname[0] == 0)
    {
	srvCmdErrorResp(0, "PCCARD value not specified", NULL, srvMsgDst);
	return (FALSE);
    }
    return (TRUE);
}

//********************************************************
// Function: findpccard - Find instance given PCCARD value
// Returned: Pointer to IDB for instance
//********************************************************

static IDB *findpccard(
    int report)

{
    IDB *idb;

    if (pccardname[0] == 0)
    {
	srvCmdErrorResp(0, "No PCCARD value specified", NULL, srvMsgDst);
	return (NULL);
    }
    idb = idbhead;
    while (idb != NULL)
    {
	if (strcmp(pccardname, idb->pccname) == 0)
	    return (idb);
	idb = idb->next;
    }
    if (report)
    {
	sprintf(bufr, "PC-card device %s is not registered", pccardname);
	srvCmdErrorResp(0, bufr, NULL, srvMsgDst);
    }
    return (NULL);
}

//********************************************************
// Function: findtype - Find TYPEBLK structure for given
//		device type and PC-card socket
// Returned: Address of TYPEBLK structure or NULL if error
//********************************************************

TYPEBLK *findtype(
    int     type,
    SOCBLK *socblk)

{
    TYPEBLK *typeblk;

    typeblk = socblk->firsttype;
    while (typeblk != NULL)
    {
	if (typeblk->type == type)
	    break;
	typeblk = typeblk->next;
    }
    return (typeblk);
}

//**********************************************
// Function: banner - Display the initial banner
// Returned: Nothing
//**********************************************

static void banner(void)

{
    if (instance != 0)
        sprintf(bufr, "xPCCARD: Version %d.%d.%d, Unit %d, Instance %d",
		MAJVER, MINVER, EDITNUM, srvUnitNum, instance);
    else
	sprintf(bufr, "xPCCARD: Version %d.%d.%d, Unit %d", MAJVER, MINVER,
		EDITNUM, srvUnitNum);
    bufr[0] = MT_INTRMDMSG;
    srvCmdResponse(bufr, srvMsgDst);
}

//******************************************************
// Function: getmem - Allocate memory for server command
// Returned: Address of memory obtained or NULL if error
//******************************************************

void *getmem(
    int size)

{
    void *mem;

    if ((mem = malloc(size)) == NULL)
	srvCmdErrorResp(0, "Cannot allocate memory", NULL, srvMsgDst);
    return (mem);
}

//*****************************************************
// Function: fail - Log message for general failure not
//		associated with a command
// Returned: Nothing
//*****************************************************

void fail(
    IDB  *idb,
    long  code,
    char *msg)

{
    char *pnt;
    char *logpnt;
    char  text[200];

    logpnt = text + sprintf(text, "Unit %d, pc-card %s, ", srvUnitNum,
	    idb->pccname);
    pnt = strmov(logpnt, msg);
    srvLogSysLog(code, text);
    if (srvDebugLevel != 0)
    {
	logpnt = text + sprintf(text, "Error: %s\n", msg);
	svcSysErrMsg(code, 0x03, logpnt);
	logstring(text);
    }
}

//*******************************************
// Function: pccthread - Main thread function
// Returned: Nothing
//*******************************************

static void pccthread(void)

{
    IDB *idb;
    long pcchndl;


    idb = (IDB *)thdData;
    pcchndl = idb->pcchndl;

    for (;;)
    {
	thdCtlSuspend(NULL, -1);
    }
}

//***************************************************************
// Function: pccsignal - Signal routine for PC-card status change
// Returned: Nothing
//***************************************************************

int xxxx;

static void pccsignal(
    INTDATA intdata)

{
    if (srvDebugLevel & DEBUG_DEVICE)
    {
	char text[120];

	sprintf(text, "Status change signal: Soc=%d, %s", intdata.socket & 0xFF,
		(intdata.socket & 0x80000000) ? "Inserted" : "Removed");
	logstring(text);
    }
    pccsetup(intdata.idb, intdata.idb->soctbl[intdata.socket & 0xFF]);
    if (srvDebugLevel & DEBUG_DEVICE)
	logstring("Signal processing complete");
}

//**********************************************
// Function: pccsetup - Set up a PC-card for use
// Returned: Nothing
//**********************************************

void pccsetup(
    IDB    *idb,
    SOCBLK *soc)

{
    if (!getconfig(idb, soc->num))	// Get configuration for the card
	return;				// Forget it if error
    if (cfg.cfgoffset == 0)		// Null configuration (Either there
					//   is no card present or the card
					//   did not report valid CIS data.
					//   In either case, act like there is
    {					//   no card present.
	if (soc->curtype != NULL)	// Did we have a card before?
	{
	    (soc->curtype->remove)(idb, soc); // Yes - remove it
	    soc->curtype = NULL;	// Indicate no card

	}
	if (srvDebugLevel & DEBUG_DEVICE)
	{
	    sprintf(bufr, "No PC-card present for %s%d", idb->pccname,
		    soc->num);
	    logstring(bufr);
	}
	return;
    }

    // Here with configuration data for the card.  We need to see if there is
    //   a match with an associated device.

    switch (cfg.cardtype)		// Dispatch on the card type
    {
     case 4:				// Disk
	setupdisk(idb, soc);
	break;

     default:				// Unknown card type
	if (srvDebugLevel & DEBUG_DEVICE)
	{
	    sprintf(bufr, "Unsupported PC-card type %d for %s%d", cfg.cardtype,
		    idb->pccname, soc->num);
	    logstring(bufr);
	}
	soc->curtype = 0;		// Indicate no card
	break;
    }
}

//*******************************************
// Function: setupiocard - Set up the PC-card
//		controller for an IO card
// Returned: TRUE if normal, FALSE if error
//*******************************************

int setupiocard(
    IDB    *idb,
    int     soc,
    CFGENT *cfgent,
    int     ioreg1,
    int     iosize1,
    int     ioreg2,
    int     iosize2,
    int     intreq)

{
    static struct
    {	byte1_char socket;
	text4_char cardtype;
	byte1_char socint;
	byte4_char cfgaddr;
	byte4_char cfgvalue;
	byte4_char io0reg;
	byte4_char io0size;
	byte4_char io1reg;
	byte4_char io1size;
	text4_char enabled;
	char       end;
    } chrlist =
    {	{PAR_SET|REP_DECV, 1, "SOCKET"},
	{PAR_SET|REP_TEXT, 4, "CARDTYPE", "IO"},
	{PAR_SET|REP_DECV, 1, "SOCINT"},
	{PAR_SET|REP_DECV, 4, "CFGADDR"},
	{PAR_SET|REP_DECV, 4, "CFGVALUE"},
	{PAR_SET|REP_DECV, 4, "IO0REG"},
	{PAR_SET|REP_DECV, 4, "IO0SIZE"},
	{PAR_SET|REP_DECV, 4, "IO1REG"},
	{PAR_SET|REP_DECV, 4, "IO1SIZE"},
	{PAR_SET|REP_TEXT, 4, "ENABLE", "YES"}
    };

    int rtn;

    if (srvDebugLevel & DEBUG_DEVICE)
    {
	sprintf(bufr, "IO card set up: Soc=%d, CfgAdr=0x%04.4X, "
		"CfgVal=0x%02.2X, Int=%d\n    IO0reg=0x%04.4X, IO0size=%d, "
		"IO1reg=0x%04.4X, IO1size=%d", soc, cfg.cfgoffset,
		cfgent->cfgval, intreq, ioreg1, iosize1, ioreg2, iosize2);
	logstring(bufr);
    }
    chrlist.socket.value = soc;
    chrlist.socint.value = intreq;
    chrlist.cfgaddr.value = cfg.cfgoffset;
    chrlist.cfgvalue.value = cfgent->cfgval | 0x40;
    chrlist.io0reg.value = ioreg1;
    chrlist.io0size.value = iosize1;
    chrlist.io1reg.value = ioreg2;
    chrlist.io1size.value = iosize2;
    if ((rtn = svcIoDevChar(idb->pcchndl, &chrlist)) < 0)
    {
	sprintf("Error configuring PC-card controller %s", idb->pccname);
	fail(idb, rtn, bufr);
	return (FALSE);
    }
    return (TRUE);
}

void logstring(
    char *str)

{
    fprintf(srvDebugStream, "%s\n", str);
}
