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
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSTIME.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>
#include <XOSNET.H>

#define VERSION 3
#define EDITNO  0

//long  dev;
long  ipsdev;
long  rtn;
long  sdfarg[1] = {0xFFFFFFFFL};
long  timeintv;
char  netname[16] = "ICMP0:";
char  prgname[] = "PING";

struct
{   ulong ipaddr;
    long  xxx1;
    char  retryt;
    char  retryn;
    short id;
    long  length;
    char  data[20];
} message =
{   0, 0, 2, 3, 0xAA55, 16, "ping..PING..ping"};

char  phyname[20];
char  ipsname[20] = "_IPS";
uchar addrbufr[260];

struct
{   byte4_parm  filoptn;
    lngstr_parm filspec;
    char        end;
} opnparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, FO_NOPREFIX|FO_XOSNAME},
    {PAR_GET|REP_STR , 0, IOPAR_FILSPEC, phyname, 0, sizeof(phyname), sizeof(phyname)}
};

struct
{   text4_parm  class;
    char        end;
} spcparms =
{   {PAR_SET|REP_TEXT, 4, IOPAR_CLASS  , "ICMP"}
};

type_qab spcqab =
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
    &spcparms, 0		// qab_parm
};

struct
{   text4_parm class;
    char       end;
} ipsparms =
{   {PAR_SET|REP_TEXT, 4, IOPAR_CLASS, "IPS"}
};

void badipa(void);
int  haveipaddr(char *);
int  havetime(arg_data *);
void help(void);

#define AF(func) (int (*)(arg_data *))func

arg_spec options[] =
{   {"?"   , 0       , NULL, AF(help)   , 0},
    {"HELP", 0       , NULL, AF(help)   , 0},
    {"H"   , 0       , NULL, AF(help)   , 0},
    {"T"   , ASF_NVAL, NULL,    havetime, 0},
    {"TIM" , ASF_NVAL, NULL,    havetime, 0},
    {"TIME", ASF_NVAL, NULL,    havetime, 0},
    {NULL  , 0       , NULL, AF(NULL)   , 0}
};


main(argc, argv)
int   argc;
char *argv[];

{
    char  *pnt;
    time_s time1;
    time_s time2;
    time_s diff;
    int    cnt;
    ulong  value;
    char   chr;

    if (--argc > 0)
    {
        argv++;
        procarg(argv, 0, options, NULL, haveipaddr,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    if ((spcqab.qab_handle = svcIoOpen(0, netname, &opnparms)) < 0)
        femsg2(prgname, "Error opening network device", spcqab.qab_handle,
                netname);

    // First, see if we have an IP address or a domain name (an IP address
    //   can contain only digits and periods.

    pnt = addrbufr + 1;
    while ((chr = *pnt++) != 0)
    {
	if (chr != '.' && !isdigit(chr))
	    break;
    }

    if (chr == 0)
    {
	message.ipaddr = 0;
	pnt = addrbufr + 1;
	cnt = 4;
	do
	{
	    value = 0;
	    while ((isdigit(chr=*pnt++)))
	    {
		value *= 10;
		value += (chr & 0xF);
	    }
	    if (value > 255)
		badipa();
	    message.ipaddr >>= 8;
	    message.ipaddr += (value << 24);
	} while(--cnt > 0 && chr == '.');
	if (chr != '\0' && (chr != ':' || pnt[0] != ':' || pnt[1] != '\0'))
	    badipa();
    }
    else
    {
	strcpy(ipsname + 4, phyname + 4);
	if ((ipsdev = svcIoOpen(0, ipsname, NULL)) < 0)
	    femsg2(prgname, "Error opening IPS device", ipsdev, ipsname);
	addrbufr[0] = 0;		// Indicate using period format
	if ((rtn = svcIoSpecial(ipsdev, IPSSF_FINDIPA, addrbufr,
		sizeof(addrbufr), &ipsparms)) < 0)
	    femsg2(prgname, "Error obtaining IP address", rtn, NULL);
	message.ipaddr = *((ulong *)(addrbufr + 1));
    }
    if (message.ipaddr == 0)
    {
        fputs("? PING: No IP address specified\n", stderr);
        exit(1);
    }
    for (;;)
    {
        svcSysDateTime(11, &time1);
        if ((rtn = svcIoQueue(&spcqab)) < 0 || (rtn = spcqab.qab_error) < 0)
	{
	    sprintf(addrbufr, "Error getting echo response from "
		    "_%s%u.%u.%u.%u::", phyname, ((uchar *)&message.ipaddr)[0],
		    ((uchar *)&message.ipaddr)[1],
		    ((uchar *)&message.ipaddr)[2],
		    ((uchar *)&message.ipaddr)[3]);
            femsg2(prgname, addrbufr, rtn, NULL);
	}
        svcSysDateTime(11, &time2);
        sdtsub(&diff, &time2, &time1);
        printf("Response received from _%s%u.%u.%u.%u:: in %ld.%02ld ms\n",
                phyname, ((uchar *)&message.ipaddr)[0],
                ((uchar *)&message.ipaddr)[1], ((uchar *)&message.ipaddr)[2],
                ((uchar *)&message.ipaddr)[3],
                (diff.time + XT_MILLISEC/2)/XT_MILLISEC,
                (diff.time % XT_MILLISEC)*2);
        if (timeintv != 0)
            svcSchSuspend(NULL, timeintv);
        else
            return (0);
    }
}

int haveipaddr(
    char *arg)

{
    int   cnt;
    char *pnt;
    char  chr;

    pnt = arg;
    while ((chr = *pnt++) != '\0')	//First see if have device name
    {
        if (chr == ':')
        {
            if (*pnt == ':')
                break;
            pnt = netname;
            cnt = 14;
            do
            {   if (--cnt <= 0)
                {
                    fputs("? PING: Illegal device name specified\n", stderr);
                    return (FALSE);
                }
                chr = *arg++;
                *pnt++ = chr;
            } while (chr != ':');
            *pnt = '\0';
        }
    }
    pnt = strnmov(addrbufr + 1, arg, 256);
    if ((pnt - addrbufr) > 256)
	badipa();
    return (TRUE);
}

int  havetime(arg_data *arg)

{
    timeintv = arg->val.n * XT_SECOND;
    return (TRUE);
}

void help(void)

{
    fprintf(stderr, "PING - version %d.%d\n\n", VERSION, EDITNO);
    fputs("A single argument of the form n.n.n.n which specifies the IP "
          "address of the\n  node to ping must be given\n\n"
          "The following options may also be specified:\n"
	  "  /HELP or /?  Display this help message\n"
          "  /TIME=time   Specify repeat rate in seconds\n", stderr);
    exit(1);
}

void badipa(void)

{
    fputs("? PING: Illegal IP address\n", stderr);
    exit(1);
}
