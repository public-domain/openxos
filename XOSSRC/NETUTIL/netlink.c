
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

#include <PROCARG.H>
#include <XOSERRMSG.H>
#include <XOS.H>
#include <XOSTRM.H>
#include <XOSSIG.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSTIME.H>
#include <XOSSTR.H>

#define VERSION 1
#define EDITNO  1

#define IOPAR_NET_CTLMODE 0x8001

#define CLASS_NET  0x54454EL
#define CLASS_SNAP 0x50414E53L
#define CLASS_IPS  0x535049L

long  device;
long  ipsdev;
long  icmpdev;
long  snapdev;
long  netdev;
long  rtn;
long  timeout = 60*XT_SECOND;
char  givenname[16];
char  devname[16] = "_";
char  ipsname[16] = "_";
char  icmpname[16] = "_ICMP";
char  snapname[16] = "_";
char  netname[16] = "_";
char  dialcmd[40] = "ATDT";
char  prgname[] = "NETLINK";
char  respbufr[60];
char  quiet;
char  tone = TRUE;

struct
{   byte4_parm  class;
    byte4_parm  filoptn;
    lngstr_parm filspec;
    char        end;
} devparms =
{   {PAR_GET|REP_TEXT, 4, IOPAR_CLASS},
    {PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, FO_NOPREFIX|FO_XOSNAME},
    {PAR_GET|REP_STR , 0, IOPAR_FILSPEC, &devname[1], 0, 15, 15}
};

struct
{   text4_parm  class;
    byte1_parm  value;
    char        end;
} lcmonparms =
{   {PAR_SET|REP_TEXT, 4, IOPAR_CLASS  , "NET"},
    {PAR_SET|REP_HEXV, 1, IOPAR_NET_CTLMODE, 1}
};

struct
{   text4_parm  class;
    byte1_parm  value;
    char        end;
} lcmoffparms =
{   {PAR_SET|REP_TEXT, 4, IOPAR_CLASS  , "NET"},
    {PAR_SET|REP_HEXV, 1, IOPAR_NET_CTLMODE, 0}
};

struct
{   byte2_parm  bits;
    char        end;
} init1parms =
{   {PAR_SET|REP_HEXV, 2, IOPAR_TRMSPMODEM, SPMC_RTS}
};

struct
{   byte2_parm  bits;
    char        end;
} init2parms =
{   {PAR_SET|REP_HEXV, 2, IOPAR_TRMSPMODEM, SPMC_RTS|SPMC_DTR}
};

struct
{   byte4_parm  time;
    char        end;
} nowaitparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_TIMEOUT, 0}
};

struct
{   byte4_parm  time;
    char        end;
} waitparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_TIMEOUT, 0}
};

struct
{   byte4_parm  time;
    char        end;
} owaitparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_TIMEOUT, 2*XT_SECOND}
};

struct snapdevchar
{   text8_char snapdev;
    char       end;
} snapdevchar =
{   {PAR_GET|REP_TEXT, 8, "SNAPDEV"}
};

type_qab snapdevqab =
{   QFNC_WAIT|QFNC_DEVCHAR,	// qab_func
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    0,				// qab_handle
    0,				// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    O_NOMOUNT|O_PHYS|CF_VALUES,	// qab_option
    0,				// qab_count
    0, 0,			// qab_buffer1
    (char *)&snapdevchar, 0,	// qab_buffer2
    NULL, 0			// qab_parm
};

struct netdevchar
{   text8_char netdev;
    char       end;
} netdevchar =
{   {PAR_GET|REP_TEXT, 8, "NETDEV"}
};

type_qab netdevqab =
{   QFNC_WAIT|QFNC_DEVCHAR,	// qab_func
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    0,				// qab_handle
    0,				// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    O_NOMOUNT|O_PHYS|CF_VALUES,	// qab_option
    0,				// qab_count
    0, 0,			// qab_buffer1
    (char *)&netdevchar, 0,	// qab_buffer2
    NULL, 0			// qab_parm
};

struct
{   ulong ipaddr;
    long  xxx1;
    char  retryt;
    char  retryn;
    short id;
    long  length;
    char  data[32];
} message =
{   0, 0, 3, 5, 0x5522, 24, "PASSWRD:"};

struct
{   text4_parm  class;
    char        end;
} echoparms =
{   {PAR_SET|REP_TEXT, 4, IOPAR_CLASS  , "ICMP"}
};

type_qab echoqab =
{   QFNC_WAIT|QFNC_SPECIAL,	// qab_open
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    0,				// qab_handle
    0,				// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    1,				// qab_option
    sizeof(message),		// qab_count
    (char *)&message, 0,	// qab_buffer1
    NULL, 0,			// qab_buffer2
    &echoparms, 0		// qab_parm
};

struct statechar
{   text8_char state;
    char       end;
} statechar =
{   {PAR_SET|REP_TEXT, 8, "STATE", "ESTAB"}
};

type_qab stateqab =
{   QFNC_WAIT|QFNC_DEVCHAR,	// qab_func
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    0,				// qab_handle
    0,				// qab_vector
    0, 0,
    O_NOMOUNT|O_PHYS|CF_VALUES,	// qab_option
    0,				// qab_count
    0, 0,			// qab_buffer1
    (char *)&statechar, 0,	// qab_buffer2
    NULL, 0			// qab_parm
};

struct adjchar
{   byte4_char adjaddr;
    byte4_char rmtaddr;
    char       end;
} adjchar =
{   {PAR_SET|REP_HEXV, 4, "ADJADDR"},
    {PAR_SET|REP_HEXV, 4, "RMTADDR"}
};

type_qab adjqab =
{   QFNC_WAIT|QFNC_DEVCHAR,	// qab_func
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    0,				// qab_handle
    0,				// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    O_NOMOUNT|O_PHYS|CF_VALUES,	// qab_option
    0,				// qab_count
    0, 0,			// qab_buffer1
    (char *)&adjchar, 0,	// qab_buffer2
    NULL, 0			// qab_parm
};

struct getsnapdevchar
{   byte4_char  sapths;
    text16_char sapdev;
    char       end;
} getsnapdevchar =
{   {PAR_SET|REP_DATAB,  4, "SAPTHIS", 0xFFFF},
    {PAR_GET|REP_DATAB, 16, "SAPDEV"}
};

type_qab getsnapdevqab =
{   QFNC_WAIT|QFNC_DEVCHAR,	// qab_func
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    0,				// qab_handle
    0,				// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    O_NOMOUNT|O_PHYS|CF_VALUES,	// qab_option
    0,				// qab_count
    0, 0,			// qab_buffer1
    (char *)&getsnapdevchar, 0,	// qab_buffer2
    NULL, 0			// qab_parm
};

struct getipsdevchar
{   byte4_char  etypeths;
    text16_char etypedev;
    char       end;
} getipsdevchar =
{   {PAR_SET|REP_DATAB,  4, "ETYPETHS", 0xFFFF},
    {PAR_GET|REP_DATAB, 16, "ETYPEDEV"}
};

type_qab getipsdevqab =
{   QFNC_WAIT|QFNC_DEVCHAR,	// qab_func
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    0,				// qab_handle
    0,				// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    O_NOMOUNT|O_PHYS|CF_VALUES,	// qab_option
    0,				// qab_count
    0, 0,			// qab_buffer1
    (char *)&getipsdevchar, 0,	// qab_buffer2
    NULL, 0			// qab_parm
};

int  cmdmode(int msg);
void controlc(void);
int  flashdtr(int msg);
void getipsdev(void);
void getnetdev(void);
void getresp(long time);
int  havearg(char *arg);
void killlink(int msg);
void opthelp(void);
int  optquiet(arg_data *arg);
int  opttimeout(arg_data *arg);
int  opttone(arg_data *arg);
void sendcmd(char *cmd, long time);

#define AF(func) (int (*)(arg_data *))func

arg_spec options[] =
{   {"?"      , 0                  , NULL, AF(opthelp)  , 0},
    {"H"      , 0                  , NULL, AF(opthelp)  , 0},
    {"HELP"   , 0                  , NULL, AF(opthelp)  , 0},
    {"TON"    , 0                  , NULL,    opttone   , TRUE},
    {"TONE"   , 0                  , NULL,    opttone   , TRUE},
    {"PUL"    , 0                  , NULL,    opttone   , FALSE},
    {"PULSE"  , 0                  , NULL,    opttone   , FALSE},
    {"TIM"    , ASF_NVAL|ASF_VALREQ, NULL,    opttimeout, 0},
    {"TIMEOUT", ASF_NVAL|ASF_VALREQ, NULL,    opttimeout, 0},
    {"Q"      , 0                  , NULL,    optquiet  , TRUE},
    {"QUI"    , 0                  , NULL,    optquiet  , TRUE},
    {"QUIET"  , 0                  , NULL,    optquiet  , TRUE},
    {"NOQ"    , 0                  , NULL,    optquiet  , FALSE},
    {"NOQUIET", 0                  , NULL,    optquiet  , FALSE},
    {NULL     , 0                  , NULL, AF(NULL)     , 0}
};

//**********************************
// Function: main - Main program
// Returned: 0 if normal, 1 if error
//**********************************

main(argc, argv)
int   argc;
char *argv[];

{
    long  rtn;
    char *pnt;
    char  chr;

    if (--argc > 0)
    {
        argv++;
        procarg(argv, 0, options, NULL, havearg,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    if (givenname[0] == 0)
    {
        fputs("? NETLINK: No device specified\n", stderr);
        exit(1);
    }
    if (!tone)
        dialcmd[3] = 'P';

    // Make sure have colon after the device name

    strupr(givenname);
    pnt = givenname;
    while ((chr = *pnt++) != 0)
    {
        if (chr == ':')
            break;
    }
    if (chr == 0)
        pnt[-1] = ':';

    // Now get the physical name for the device and get its device class

    if ((device = svcIoOpen(O_IN|O_OUT, givenname, &devparms)) < 0)
        errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, device,
                "Error opening device", givenname);
    if (devparms.class.value == CLASS_NET)
    {

        strmov(netname, devname);
        netdev = device;

	// Now find the SNAP device, given the NET device

        getsnapdevqab.qab_handle = netdev;
        if ((rtn = svcIoQueue(&getsnapdevqab)) < 0 ||
                (rtn = getsnapdevqab.qab_error) < 0)
            errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                    "Error obtaining name of SNAP device", NULL);
        if (getsnapdevchar.sapdev.value[0] == 0)
        {
            fputs("? NETLINK: No SNAP device corresponds to the NET device\n",
                    stderr);
            exit(1);
        }
        *strnmov(&snapname[1], getsnapdevchar.sapdev.value, 8) = ':';
        if ((snapdev = svcIoOpen(O_IN|O_OUT, snapname, NULL)) < 0)
            errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, snapdev,
                    "Error opening SNAP device", snapname);
        getipsdev();			// Find the IPS device

    }
    else if (devparms.class.value == CLASS_SNAP)
    {

        strmov(snapname, devname);
        snapdev = device;
        getipsdev();			// Find the IPS device
        getnetdev();			// Find the NET device
    }
    else if (devparms.class.value == CLASS_IPS)
    {
        // Here when have an IPS device

        strmov(ipsname, devname);
        ipsdev = device;
        pnt = ipsname + 1;		// If have generic name (IPSn:),
        while ((chr = *pnt++) != 0)	//   change it to IPSnA:
            ;
        if (isdigit(pnt[-3]))
        {
            pnt[-2] = 'A';
            pnt[-1] = ':';
        }

        // First find the underlying SNAP device

        snapdevqab.qab_handle = ipsdev;
        if ((rtn = svcIoQueue(&snapdevqab)) < 0 ||
                (rtn = snapdevqab.qab_error) < 0)
            errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                    "Error obtaining name of SNAP device", NULL);
        pnt = strnmov(&snapname[1], snapdevchar.snapdev.value, 8);
        *pnt = ':';
        if ((snapdev = svcIoOpen(O_IN|O_OUT, snapname, NULL)) < 0)
            errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, snapdev,
                    "Error opening SNAP device", netname);
        getnetdev();			// Find the NET device
    }
    else
    {
        fputs("? NETLINK: Device is not a NET, SNAP, or IPS device\n", stderr);
        exit(1);
    }

    // When get here, we have all three devices (NET, SNAP, and IPS open)

    svcIoClose(snapdev, 0);		// Close the SNAP device since we
					//   need it any more

    pnt = netname;
    while (*pnt != 0 && !isdigit(*pnt))
        pnt++;
    strmov(&icmpname[5], pnt);
    if ((icmpdev = svcIoOpen(O_IN|O_OUT, icmpname, NULL)) < 0)
        errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, icmpdev,
                "Error opening ICMP device", icmpname);

    if (!quiet)
        printf("%% NETLINK: Using network interface %s\n", netname);

    stateqab.qab_handle = netdev;
    if (dialcmd[4] == 0)		// Was a phone number given?
    {
        killlink(TRUE);			// No - just disconnect
        exit(0);
    }
    if (!quiet)
        fputs("% NETLINK: Initializing modem\n", stdout);
    if (!cmdmode(TRUE))
        exit(1);

    setvector(VECT_CNTC, 0x84, controlc);
    svcSchSetLevel(0);

    if (!quiet)
        printf("%% NETLINK: Dialing %s\n", &dialcmd[4]);
    sendcmd(dialcmd, timeout);
    while (strncmp(respbufr, "CONNECT", 7) != 0)
    {
        if (strncmp(respbufr, "RINGING", 7) == 0 ||
                strncmp(respbufr, "CARRIER", 7) == 0 ||
                strncmp(respbufr, "COMPRESSION", 11) == 0 ||
                strncmp(respbufr, "PROTOCOL", 8) == 0)
        {
            if (!quiet)
            {
                printf("%% NETLINK: %s\n", respbufr);
            }
            getresp(timeout);
            continue;
        }
        fprintf(stderr, "? NETLINK: Could not establish connection; %s\n",
                respbufr);
        exit(1);
    }

    if (!quiet)
        printf("%% NETLINK: Link connected: %s\n", respbufr);
    svcSchSuspend(NULL, 1000*XT_MILLISEC);

    if ((rtn = svcIoOutBlockP(netdev, NULL, 0, &lcmoffparms)) < 0)
        errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                "Error clearing link control mode", NULL);
    echoqab.qab_handle = icmpdev;
    if ((rtn = svcIoQueue(&echoqab)) < 0 || (rtn = echoqab.qab_error) < 0)
    {
        errmsg(EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                "Error during access verification", NULL);
        killlink(TRUE);
    }
    if (strncmp(message.data, "REJECTED", 8) == 0)
    {
        fputs("? NETLINK: Password incorrect, link not established\n", stderr);
        killlink(TRUE);
    }
    if (strncmp(message.data, "ESTBLSH:", 8) != 0)
    {
        fputs("? NETLINK: Access verification failed, link not established\n",
                stderr);
        killlink(TRUE);
    }
    if ((rtn = svcIoQueue(&stateqab)) < 0 || (rtn = stateqab.qab_error) < 0)
    {
        errmsg(EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                "Error setting link state", NULL);
        killlink(TRUE);
    }
    if (!quiet)
        printf("%% NETLINK: Link established, adjacent IP address = "
                "%d.%d.%d.%d\n", ((uchar *)(&message.ipaddr))[0],
                ((uchar *)(&message.ipaddr))[1],
                ((uchar *)(&message.ipaddr))[2],
                ((uchar *)(&message.ipaddr))[3]);
    adjqab.qab_handle = ipsdev;
    adjchar.adjaddr.value = message.ipaddr;
    adjchar.rmtaddr.value = message.ipaddr;
    if ((rtn = svcIoQueue(&adjqab)) < 0 || (rtn = adjqab.qab_error) < 0)
    {
        errmsg(EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                "Error storing adjacent IP address", NULL);
        killlink(TRUE);
    }
    return (0);
}

//*******************************************************
// Function: getnetdev - Get NET device given SNAP device
//*******************************************************

void getnetdev(void)

{
    netdevqab.qab_handle = snapdev;
    if ((rtn = svcIoQueue(&netdevqab)) < 0 ||
            (rtn = netdevqab.qab_error) < 0)
        errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                "Error obtaining name of network device", NULL);
    *strnmov(&netname[1], netdevchar.netdev.value, 8) = ':';
    if ((netdev = svcIoOpen(O_IN|O_OUT, netname, NULL)) < 0)
        errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, netdev,
                "Error opening network device", netname);
}

//*******************************************************
// Function: getipsdev - Get IPS device given SNAP device
//*******************************************************

void getipsdev(void)

{
    getipsdevqab.qab_handle = snapdev;
    if ((rtn = svcIoQueue(&getipsdevqab)) < 0 ||
            (rtn = getipsdevqab.qab_error) < 0)
        errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                "Error obtaining name of IPS device", NULL);
    if (getipsdevchar.etypedev.value[0] == 0)
    {
        fputs("? NETLINK: No IPS device corresponds to the SNAP device\n",
                stderr);
        exit(1);
    }
    *strnmov(&ipsname[1], getipsdevchar.etypedev.value, 8) = ':';
    if ((ipsdev = svcIoOpen(O_IN|O_OUT, ipsname, NULL)) < 0)
        errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, ipsdev,
                "Error opening IPS device", ipsname);
}

//************************************************
// Function: cmdmode - put modem into command mode
// Returned: TRUE if normal, FALSE if error
//************************************************

int cmdmode(
    int msg)

{
    if ((rtn = svcIoOutBlockP(netdev, NULL, 0, &lcmonparms)) < 0)
    {
        if (msg)
            errmsg(EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                    "Error enabling link control mode", NULL);
        return (FALSE);
    }
    if (!flashdtr(msg))
        return (FALSE);
    svcSchSuspend(NULL, 500*XT_MILLISEC);
    while (svcIoInBlockP(netdev, respbufr, sizeof(respbufr), &nowaitparms) > 0)
        ;				// Clear any pending input
    sendcmd("ATE1V1Q", 2*XT_SECOND);	// Enable echo, response codes, use
					//   text mode response codes
    if (strcmp(respbufr, "OK") != 0 && strcmp(respbufr, "NO CARRIER") != 0)
    {
        if (msg)
            fprintf(stderr, "? NETLINK: Unexpected response from modem: %s\n",
                    respbufr);
        return (FALSE);
    }
    return (TRUE);
}

//****************************************
// Function: flashdtr - flash the DTR line
// Returned: TRUE if OK, FALSE if error
//****************************************

int flashdtr(
    int msg)

{
    if ((rtn = svcIoInBlockP(netdev, NULL, 0, &init1parms)) < 0)
    {
        if (msg)
            errmsg(EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                    "Error initializing modem", NULL);
        return (FALSE);
    }
    svcSchSuspend(NULL, 500*XT_MILLISEC);
    if ((rtn = svcIoInBlockP(netdev, NULL, 0, &init2parms)) < 0)
    {
        if (msg)
            errmsg(EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                    "Error initializing modem", NULL);
        return (FALSE);
    }
    return (TRUE);
}

//***********************************************************
// Function: sendcmd - Send command to modem and get response
// Returned: Nothing (final response is in respbufr)
//***********************************************************

void sendcmd(
    char *cmd,
    long  time)

{
    svcSchSuspend(NULL, 200*XT_MILLISEC);

    if ((rtn = svcIoOutBlockP(netdev, cmd, strlen(cmd), &owaitparms)) < 0 ||
            (rtn = svcIoOutSingle(netdev, '\r')) < 0)
    {
        errmsg(EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                "Error outputting command to modem", NULL);
        flashdtr(FALSE);
        exit(1);
    }
    getresp(2*XT_SECOND);
    if (strcmp(cmd, respbufr) != 0)
    {
        if (strcmp(respbufr, "OK") != 0 && strcmp(respbufr, "NO CARRIER") != 0)
        {
            fprintf(stderr, "? NETLINK: Unexpected response from modem: %s\n",
                    respbufr);
            exit(1);
        }
    }
    else
        getresp(time);
}

//*************************************************
// Function: getresp - Get response line from modem
// Returned: Nothing (response is in respbufr)
//*************************************************

void getresp(
    long time)

{
    char *pnt1;
    char *pnt2;
    long  rtn;
    int   cnt;
    char  textbufr[20];
    char  done;
    char  chr;

    waitparms.time.value = time;
    pnt1 = respbufr;
    cnt = sizeof(respbufr) - 4;
    done = FALSE;
    while (!done)
    {
        if ((rtn = svcIoInBlockP(netdev, textbufr, sizeof(textbufr),
                &waitparms)) < 0)
        {
            errmsg(EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                    "Error getting response from modem", NULL);
            flashdtr(FALSE);
            exit(1);
        }
        pnt2 = textbufr;
        while (--rtn >= 0)
        {
            if ((chr = *pnt2++) == '\n')
                continue;
            if (chr == '\r')
            {
                if (pnt1 == respbufr)
                    continue;
                done = TRUE;
                break;
            }
            if (--cnt < 0)
            {
                fputs("? NETLINK: Modem response text too long\n", stderr);
                exit(1);
            }
            *pnt1++ = toupper(chr);
        }
    }
    *pnt1 = 0;
}

void controlc(void)

{
    svcSchCtlCDone();
    fputs("% NETLINK: Aborted\n", stderr);
    svcIoCancel(&stateqab, CAN_ALL|CAN_INPUT|CAN_OUTPUT|CAN_WAIT);
    killlink(FALSE);
    exit(1);
}

//*************************************************
// Function: killlink - Kill connection after error
// Returned: Never returns
//*************************************************

void killlink(
    int msg)

{
    svcIoOutBlockP(netdev, NULL, 0, &lcmoffparms);
    if (cmdmode(FALSE))
    {
        sendcmd("ATH", 2*XT_SECOND);
        if (strcmp(respbufr, "OK") == 0 || strcmp(respbufr, "NO CARRIER") == 0)
        {
            if (!quiet && msg)
                fputs("% NETLINK: Link disconnected\n", stderr);
        }
        else
            fprintf(stderr, "? NETLINK: Unexpected response from modem: %s\n",
                    respbufr);
        exit(1);
    }
    fputs("? NETLINK: Error disconnecting link", stderr);
    exit(1);
}

//************************************************
// Function: havearg - Process non-option argument
// Returned: TRUE (does not return if error)
//************************************************

int havearg(
    char *arg)

{
    if (givenname[0] == 0)
    {
        if (strlen(arg) > 14)
        {
            fputs("? NETLINK: Device name is too long\n", stderr);
            exit(1);
        }
        strmov(givenname, arg);
    }
    else if (dialcmd[4] == 0)
    {
        if (strlen(arg) > 32)
        {
            fputs("? NETLINK: Phone number is too long\n", stderr);
            exit(1);
        }
        strmov(&dialcmd[4], arg);
    }
    else if (message.data[8] == 0)
    {
        if (strlen(arg) > 16)
        {
            fputs("? NETLINK: Password is too long\n", stderr);
            exit(1);
        }
        strupr(arg);
        strmov(&message.data[8], arg);
    }
    else
    {
        fputs("? NETLINK: Too many arguments\n", stderr);
        exit(1);
    }
    return (TRUE);
}

//*************************************************
// Function: opttone - Process TONE or PULSE option
// Returned: TRUE
//*************************************************

int opttone(
    arg_data *arg)

{
    tone = arg->data;
    return (TRUE);
}

//**********************************************
// Function: opttimeout - Process TIMEOUT option
// Returned: TRUE
//**********************************************

int  opttimeout(
    arg_data *arg)

{
    timeout = arg->val.n * XT_SECOND;
    return (TRUE);
}

//**********************************************
// Function: optquiet - Process {NO}QUIET option
// Returned: TRUE
//**********************************************

int optquiet(
    arg_data *arg)

{
    quiet = arg->data;
    return (TRUE);
}

//***********************************
// Function: help - Display help text
// Returned: Never returns
//***********************************

void opthelp(void)

{
    fprintf(stderr, "NETLINK - version %d.%d\n\n", VERSION, EDITNO);
    fputs("  This command establishes or hang up a dial-up network connection."
          "  Usage is:\n"
          "    NETLINK netdev number password\n"
          "  Where:\n"
          "    netdev   = Name of link level device (required)\n"
          "    number   = Phone number to dial (optional)\n"
          "    password = Link password (optional)\n"
          "  If \"number\" is not present, the link is hung up.\n"
          "  The following options may also be specified:\n"
	  "    /Help or /? = Display this help message\n"
          "    /TONe       = Use tone dailing (default)\n"
          "    /PULse      = Use pulse dailing\n"
          "    /TIMeout=n  = Set call connect time-out to n seconds (default"
          " = 40)\n"
          "    /{NO}Quiet  = Suppress all non-error messages\n", stderr);
    exit(1);
}
