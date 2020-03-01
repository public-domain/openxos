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
#include <XOS.H>
#include <XOSDISK.H>
#include <XOSSVC.H>
#include <XOSERRMSG.H>

#include "SHOW.H"

extern char prgname[];

dcsinfo_data dcsinfodata;

struct
{   lngstr_char  dcsinfo;
    char         end;
} dcsinfochar =
{   {PAR_GET|REP_DATAS, 0, "DCSINFO", &dcsinfodata, sizeof(dcsinfo_data),
        sizeof(dcsinfo_data)}
};

char fmt1[]    = " %5ld %5ld %5ld %5ld %5ld %5ld %5ld %5ld %5ld\n";

int dcsinfo(void)
{
    long         rtn;

    if ((rtn=svcIoClsChar("DISK:", &dcsinfochar)) < 0)
        femsg2(prgname, "Error getting disk cache buffer data", rtn, NULL);
    ++validcount;
    fputs( "                Total Avail  Free     0     1     2     3     4"
           "    >4\nSystem buffers:", stdout);
    printf(fmt1, dcsinfodata.sbuft, dcsinfodata.sbufa, dcsinfodata.sbuff,
            dcsinfodata.sbuf0, dcsinfodata.sbuf1, dcsinfodata.sbuf2,
            dcsinfodata.sbuf3, dcsinfodata.sbuf4, dcsinfodata.sbuf5);
    fputs( "Data buffers:  ", stdout);
    printf(fmt1, dcsinfodata.dbuft, dcsinfodata.dbufa, dcsinfodata.dbuff,
            dcsinfodata.dbuf0, dcsinfodata.dbuf1, dcsinfodata.dbuf2,
            dcsinfodata.dbuf3, dcsinfodata.dbuf4, dcsinfodata.dbuf5);
    return (TRUE);
}
