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
#include <XOS.H>
#include <XOSSINFO.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSERRMSG.H>
#include "SHOW.H"

typedef struct
{	char   name[16];
	uchar  number;
	uchar  bits;
	ushort xxx;
} INTDATA;

extern char    prgname[];
static char    sysclass[] = "SYSTEM:";
static struct
{   lngstr_char ints;
    char        end;
} syslist =
{{  PAR_GET|REP_DATAS, 0, "IRQTABLE", NULL, sizeof(INTDATA) * 40,
		sizeof(INTDATA) * 40}
};


int intinfo(void)

{
    INTDATA *ipnt;
    long     rtn;
	int      num;
    int      cnt;
	int      offset;
	INTDATA  intdata[40];
    
    ++validcount;
	syslist.ints.buffer = &intdata;
    if ((rtn=svcIoClsChar(sysclass, (char *)&syslist)) < 0)
        femsg(prgname, rtn, NULL);
    printf("IRQ S Usage             IRQ S Usage\n");
    ipnt = &intdata;
	num = syslist.ints.strlen/sizeof(INTDATA);
	cnt = num/2;
	offset = (num + 1)/2;
    do
    {
		printf("%3d %s %-16.16s  %3d %s %-.16s\n", ipnt->number,
				(ipnt->bits & 0x80) ? "S" : " ", ipnt->name,
				(ipnt+offset)->number, ((ipnt+offset)->bits & 0x80) ?
				"S" : " ", (ipnt+offset)->name);
        ipnt++;
    } while (--cnt > 0);
	if (num & 0x01)
		printf("%3d %s %-.16s\n", ipnt->number, (ipnt->bits & 0x80) ?
				"S" : " ", ipnt->name);
    return (TRUE);
}
