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
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <TIME.H>

#include <PROCARG.H>
#include <XOSERRMSG.H>
#include <XOS.H>
#include <XOSSIG.H>
#include <XOSTRM.H>
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
#define VECT_TRM     24
#define VECT_MODEM   25

long  device;
long  dev;
long  netdev;
long  snapdev;
long  ipsdev;
long  rtn;
char  givenname[16];
char  devname[16] = "_";
char  netname[16] = "_";
char  snapname[16] = "_";
char  ipsname[16] = "_";
char  respbufr[60];
char  prgname[] = "NETMODEM";
char  trmbufr[60];
char  modembufr[60];
char  quiet;

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
{   text4_parm class;
    byte1_parm value;
    char       end;
} lcmparms =
{   {PAR_SET|REP_TEXT, 4, IOPAR_CLASS  , "NET"},
    {PAR_SET|REP_HEXV, 1, IOPAR_NET_CTLMODE, 1}
};

struct
{   byte2_parm bits;
    char       end;
} init1parms =
{   {PAR_SET|REP_HEXV, 2, IOPAR_TRMSPMODEM, SPMC_RTS}
};

struct
{   byte2_parm bits;
    char       end;
} init2parms =
{   {PAR_SET|REP_HEXV, 2, IOPAR_TRMSPMODEM, SPMC_RTS|SPMC_DTR}
};

struct
{   byte4_parm sinm;
    byte4_parm cinm;
    char        end;
} trmparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0xFFFFFFFEL},
    {PAR_SET|REP_HEXV, 4, IOPAR_TRMSINPMODE, TIM_IMAGE}
};

type_qab trmqab =
{   QFNC_INBLOCK,		// qab_func
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    DH_STDTRM,			// qab_handle
    VECT_TRM,			// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    0,				// qab_option
    sizeof(trmbufr),		// qab_count
    trmbufr, 0,			// qab_buffer1
    NULL, 0,			// qab_buffer2
    NULL, 0			// qab_parm
};

type_qab modemqab =
{   QFNC_INBLOCK,		// qab_func
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    0,				// qab_handle
    VECT_MODEM,			// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    0,				// qab_option
    sizeof(modembufr),		// qab_count
    modembufr, 0,		// qab_buffer1
    NULL, 0,			// qab_buffer2
    NULL, 0			// qab_parm
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

void badresp(void);
int  flashdtr(int msg);
void getnetdev(void);
void getresp(long time);
int  havearg(char *arg);
void help(void);
void modemint(void);
int  optquiet(arg_data *arg);
void sendcmd(char *cmd, long time);
void trmint(void);

#define AF(func) (int (*)(arg_data *))func

arg_spec options[] =
{   {"?"      , 0, NULL, AF(help)   , 0},
    {"HELP"   , 0, NULL, AF(help)   , 0},
    {"H"      , 0, NULL, AF(help)   , 0},
    {"Q"      , 0, NULL,    optquiet, TRUE},
    {"QUI"    , 0, NULL,    optquiet, TRUE},
    {"QUIET"  , 0, NULL,    optquiet, TRUE},
    {"NOQ"    , 0, NULL,    optquiet, FALSE},
    {"NOQUIET", 0, NULL,    optquiet, FALSE},
    {NULL     , 0, NULL, AF(NULL)   , 0}
};

/****************************************/
/* Function: main - Main program	*/
/* Returned: 0 if normal, 1 if error	*/
/****************************************/

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
        fputs("? NETMODEM: No device specified\n", stderr);
        exit(1);
    }

    /* Make sure have colon after the device name */

    strupr(givenname);
    pnt = givenname;
    while ((chr = *pnt++) != 0)
    {
        if (chr == ':')
            break;
    }
    if (chr == 0)
        pnt[-1] = ':';

    /* Now get the physical name for the device and get its device class */

    if ((device = svcIoOpen(O_IN|O_OUT, givenname, &devparms)) < 0)
        errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, device,
                "Error opening device", givenname);
    if (devparms.class.value == CLASS_NET)
    {
        strmov(netname, devname);
        netdev = device;
    }
    else if (devparms.class.value == CLASS_SNAP)
    {
        strmov(snapname, devname);
        snapdev = device;
        getnetdev();			/* Find the NET device */
    }
    else if (devparms.class.value == CLASS_IPS)
    {
        /* Here when have an IPS device */

        strmov(ipsname, devname);
        ipsdev = device;
        pnt = ipsname + 1;		/* If have generic name (IPSn:), */
        while ((chr = *pnt++) != 0)	/*   change it to IPSnA: */
            ;
        if (isdigit(pnt[-3]))
        {
            pnt[-2] = 'A';
            pnt[-1] = ':';
        }

        /* First find the underlying SNAP device */

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
        getnetdev();			/* Find the NET device */
    }
    else
    {
        fputs("? NETMODEM: Device is not a NET, SNAP, or IPS device\n", stderr);
        exit(1);
    }

    /* When get here, we have all three devices (NET, SNAP, and IPS open) */

    if (snapdev)			/* Close the SNAP device since we */
        svcIoClose(snapdev, 0);	/*  don't need it any more */
    if (ipsdev)				/* Close the IPS device since we */
        svcIoClose(ipsdev, 0);		/*  don't need it any more */

    if (!quiet)
        printf("%% NETMODEM: Using network interface %s\n", netname);

    if ((rtn = svcIoOutBlockP(netdev, NULL, 0, &lcmparms)) < 0)
        errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                "Error enabling link control mode", NULL);
    if ((rtn = svcIoInBlockP(DH_STDTRM, NULL, 0, &trmparms)) < 0)
        errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                "Error setting terminal parameters", NULL);
    if (!quiet)
        fputs("% NETMODEM: Initializing modem\n", stdout);

    if (!flashdtr(TRUE))
        exit(1);

    svcSchSuspend(NULL, 500*XT_MILLISEC);
    setvector(VECT_TRM, 0x84, trmint);
    setvector(VECT_MODEM, 0x84, modemint);
    modemqab.qab_handle = netdev;
    sendcmd("ATE1V1Q", 2*XT_SECOND);	/* Enable echo, response codes, use */
    if (strcmp(respbufr, "OK") != 0)	/*   text mode response codes */
    {
        fprintf(stderr, "? NETLINK: Unexpected response from modem: %s\n",
                respbufr);
        exit(1);
    }
    if ((rtn = svcIoQueue(&trmqab)) < 0)
        errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                "Error starting terminal input", NULL);
    if ((rtn = svcIoQueue(&modemqab)) < 0)
        errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                "Error starting modem input", NULL);
    if (!quiet)
        fputs("% NETMODEM: Ready (type ^C to exit)\n\n", stdout);

    svcSchSetLevel(0);
    for (;;)
        svcSchSuspend(NULL, -1);
}

/****************************************************************/
/* Function: getnetdev - Get NET device given SNAP device	*/
/* Returned: Nothing (netname and netdev are set)		*/
/****************************************************************/

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

/********************************************************/
/* Function: modemint - Software interrupt routine	*/
/*		 for modem input ready			*/
/* Returned: Nothing					*/
/********************************************************/

void modemint(void)

{
    long rtn;

    while (modemqab.qab_status & QSTS_DONE)
    {
        if (modemqab.qab_error < 0)
            errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL,
                    modemqab.qab_error, "Error doing input from modem", NULL);
        if ((rtn = svcIoOutBlock(DH_STDTRM, modembufr,
                modemqab.qab_amount)) < 0)
            errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                    "Error doing output to terminal", NULL);
        modemqab.qab_vector = 0;
        if ((rtn = svcIoQueue(&modemqab)) < 0)
            errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                    "Error starting modem input", NULL);
        modemqab.qab_vector = VECT_MODEM;
    }
}

/********************************************************/
/* Function: trmint - Software interrupt routine	*/
/*		 for terminal input ready		*/
/* Returned: Nothing					*/
/********************************************************/

void trmint(void)

{
    long rtn;

    while (trmqab.qab_status & QSTS_DONE)
    {
        if (trmqab.qab_error < 0)
            errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL,
                    trmqab.qab_error, "Error doing input from terminal", NULL);
        if (strnchr(trmbufr, 3, sizeof(trmbufr)))
        {
            if (!quiet)
                 fputs("\n% NETMODEM: Terminating\n", stderr);
            exit(0);
        }
        if ((rtn = svcIoOutBlock(netdev, trmbufr, trmqab.qab_amount)) < 0)
            errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                    "Error doing output to modem", NULL);
        trmqab.qab_vector = 0;
        if ((rtn = svcIoQueue(&trmqab)) < 0)
            errmsg(EM_EXIT|EM_IQM|EM_NAME|EM_CODE|EM_TEXT|EM_FNL, NULL, rtn,
                    "Error starting terminal input", NULL);
        trmqab.qab_vector = VECT_TRM;
    }
}

/****************************************************************/
/* Function: sendcmd - Send command to modem and get response	*/
/* Returned: Nothing (final response is in respbufr)		*/
/****************************************************************/

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
        if (strcmp(cmd, "OK") != 0)
            badresp();
    }
    else
        getresp(time);
}

/********************************************************/
/* Function: getresp - Get response line from modem	*/
/* Returned: Nothing (response is in respbufr)		*/
/********************************************************/

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
            chr = *pnt2++;
            if ((chr/* = *pnt2++*/) == '\n')
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
            *pnt1++ = chr;
        }
    }
    *pnt1 = 0;
}

/************************************************/
/* Function: flashdtr - flash the DTR line	*/
/* Returned: TRUE if OK, FALSE if error		*/
/************************************************/

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

/********************************************************/
/* Function: badresp - Report unexpected modem response	*/
/* Returned: Never returns				*/
/********************************************************/

void badresp(void)

{
    fprintf(stderr, "? NETLINK: Unexpected response from modem: %s\n",
            respbufr);
    exit(1);
}

/********************************************************/
/* Function: havearg - Process non-option argument	*/
/* Returned: TRUE (does not return if error)		*/
/********************************************************/

int havearg(
    char *arg)

{
    if (givenname[0] == 0)
    {
        if (strlen(arg) > 14)
        {
            fputs("? NETMODEM: Device name is too long\n", stderr);
            exit(1);
        }
        strmov(givenname, arg);
    }
    else
    {
        fputs("? NETMODEM: Too many arguments\n", stderr);
        exit(1);
    }
    return (TRUE);
}

/********************************************************/
/* Function: optquiet - Process {NO}QUIET option	*/
/* Returned: TRUE					*/
/********************************************************/

int optquiet(
    arg_data *arg)

{
    quiet = arg->data;
    return (TRUE);
}

/****************************************/
/* Function: help - Display help text	*/
/* Returned: Never returns		*/
/****************************************/

void help(void)

{
    fprintf(stderr, "NETMODEM - version %d.%d\n\n", VERSION, EDITNO);
    fputs("  Usage is:\n"
          "    NETMODEM {/option} netdev\n"
          "  Where:\n"
          "    netdev   = Name of link level device (required)\n"
          "  The following options may also be specified:\n"
	  "    /HELP or /? = Display this help message\n", stderr);
    exit(1);
}
