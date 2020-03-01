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


struct sdfparms
{   text4_parm class;
    char       end;
} sdfparms =
{   {PAR_SET|REP_TEXT, 4, IOPAR_CLASS, "IPS"},
};

uchar *pnt;

void nsdns(void)

{
    long dev;
    long rtn;
    int  cnt;
    int  cnnum;
    int  anum;
    int  mxnum;
    int  ptrnum;

    if (argument[0] != 0)
    {
        if (strchr(argument, ':') == NULL)
            strcat(argument, ":");
    }
    else
        strcpy(argument, "IPS0:");
    if ((dev = svcIoOpen(O_PHYS, argument, NULL)) < 0)
        femsg2(prgname, "Error opening network device", dev, argument);
    if ((cnt = svcIoSpecial(dev, IPSSF_DUMP, NULL, 0, &sdfparms)) < 0)
	femsg2(prgname, "Error getting size of DNS cache data", cnt, argument);
    cnt += 8000;
    pnt = getspace(cnt);
    if ((cnt = svcIoSpecial(dev, IPSSF_DUMP, pnt, cnt, &sdfparms)) < 0)
	femsg2(prgname, "Error getting DNS cache data", cnt, argument);

    for (;;)
    {
	rtn = showdname(pnt + 9);
	if (pnt[8] && 0xC0)
	{
	    printf("    Not %s (%d)\n", (pnt[8] & 0x80)? "defined": "found",
		    *((ulong *)(pnt + 4)));
	}
	cnnum = pnt[0];
	anum = pnt[1];
	mxnum = pnt[2];
	ptrnum = pnt[3];
	pnt += (rtn + 10);
	while (--cnnum >= 0)
	{
	    printf("    CN: (%d) ", *((ulong *)pnt));
	    pnt += (showdname(pnt + 6) + 7);
	}
	while (--anum >= 0)
	{
	    printf("    A: (%d) %d.%d.%d.%d\n", *((ulong *)pnt), pnt[7], pnt[8],
		    pnt[9], pnt[10]);
	    pnt += (pnt[6] + 7);
	}
	while (--mxnum >= 0)
	{
	    printf("    MX: (%d) %d ", *((ulong *)pnt), *((ushort *)(pnt + 4)));
	    pnt += (showdname(pnt + 6) + 7);
	}
	while (--ptrnum >= 0)
	{
	    printf("    PTR: (%d) ", *((ulong *)pnt));
	    pnt += (showdname(pnt + 6) + 7);
	}
	if (--cnt < 0)
	    break;
	fputc('\n', stdout);
    }
}

int showdname(
    char *np)

{
    char *bp;
    int   rtn;
    int   cnt;
    char  bufr[260];

    if ((rtn = *np++) != 0)
    {
	bp = bufr;
	while ((cnt = *np++) != 0)
	{
	    while (--cnt >= 0)
		*bp++ = *np++;
	    *bp++ = '.';
	}
	bp[-1] = '\n';
	bp[0] = 0;
	fputs(bufr, stdout);
    }
    else
        fputc('\n', stdout);
    return (rtn);
}
