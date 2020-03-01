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
#include <PROCARG.H>
#include <XOS.H>
#include <XOSSINFO.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSNET.H>
#include <XOSERRMSG.H>
#include "NETSHOW.H"

extern char prgname[];

#define RTT_MANUAL   2
#define RTT_LOCAL    3
#define RTT_ROUTER   4
#define RTT_FOREIGN  5
#define RTT_LPENDING 9
#define RTT_FPENDING 10
#define RTT_DPENDING 11

char *ttbl[] =
{   " 0",
    "T ",
    "M ",
    "L ",
    "R ",
    "F ",
    " 6",
    " 7",
    "P ",
    "LP",
    "FP",
    "DP"
};

struct rtparm1
{   text4_parm class;
    byte4_parm rtuse;
    char       end;
} rtparm1 =
{   {PAR_SET|REP_TEXT, 4, IOPAR_CLASS, "IPS"},
    {PAR_GET|REP_DECV, 4, IOPAR_IPS_RTUSE, 0}
};

struct rtparm2
{   text4_parm  class;
    lngstr_parm rtdata;
    char        end;
} rtparm2 =
{   {PAR_SET|REP_TEXT , 4 , IOPAR_CLASS, "IPS"},
    {PAR_GET|REP_DATAS, 12, IOPAR_IPS_RTDATA}
};

void nsroute(void)

{
    long dev;
    long rtn;
    int  cnt;
    int  type;
    struct ipsroute *rtp;
    char *tname;
    char buf[24];
    char tbuf[8];

    if (argument[0] != 0)
    {
        if (strchr(argument, ':') == NULL)
            strcat(argument, ":");
    }
    else
        strcpy(argument, "IPS0:");
    if ((dev = svcIoOpen(O_PHYS, argument, NULL)) < 0)
        femsg(prgname, dev, argument);
    if ((rtn = svcIoInBlockP(dev, NULL, 0, &rtparm1)) < 0)
        femsg(prgname, rtn, NULL);
    rtparm2.rtdata.bfrlen = (rtparm1.rtuse.value + 10) *
            sizeof(struct ipsroute);
    rtparm2.rtdata.buffer = getspace(rtparm2.rtdata.bfrlen);
    if ((rtn = svcIoInBlockP(dev, NULL, 0, &rtparm2)) < 0)
        femsg(prgname, rtn, NULL);
    cnt = rtparm2.rtdata.strlen/20;
    rtp = (struct ipsroute *)(rtparm2.rtdata.buffer);
    printf("Routing table for IPS network device %s\n", argument);
    fputs("Type IF   IP address     Time   HW/Router address\n", stdout);

    while (--cnt >= 0)
    {
        type = (int)(rtp->type);
        if (type <= 11)
            tname = ttbl[type];
        else
        {
            sprintf(tbuf, "%2d", type);
            tname = tbuf;
        }
        sprintf(buf, "%d.%d.%d.%d", rtp->ipa[0], rtp->ipa[1], rtp->ipa[2],
                rtp->ipa[3]);
        printf(" %s  %c  %-16s%5ld", tname, rtp->interface+'A', buf, rtp->t2l);

        if (type == RTT_LOCAL || type == RTT_ROUTER)
            printf("   %02.2X-%02.2X-%02.2X-%02.2X-%02.2X-%02.2X",
                    rtp->gwipa[0], rtp->gwipa[1], rtp->gwipa[2],
                    rtp->gwipa[3], rtp->gwipa[4], rtp->gwipa[5]);
        else if (type == RTT_FOREIGN || type == RTT_FPENDING ||
                type == RTT_DPENDING)
            printf("   %d.%d.%d.%d", rtp->gwipa[0], rtp->gwipa[1],
                    rtp->gwipa[2], rtp->gwipa[3]);
        fputs("\n", stdout);
        rtp++;
    }
}
