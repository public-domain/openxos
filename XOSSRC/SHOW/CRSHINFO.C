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
#include <TIME.H>
#include <XOS.H>
#include <XOSSINFO.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSTIME.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>
#include "SHOW.H"

extern char prgname[];

typedef struct
{   ushort size;
    short  xxx;
    long   address;
} gxrv;

typedef struct
{   ushort v;
    short  x;
} reg16;

struct crshdata
{   long   crshMAGIC[3];
    time_s crshTIME;
    char   crshCODE[4];
    long   crshEAX;
    long   crshEBX;
    long   crshECX;
    long   crshEDX;
    long   crshESP;
    long   crshEBP;
    long   crshEDI;
    long   crshESI;
    reg16  crshCS;
    reg16  crshSS;
    reg16  crshDS;
    reg16  crshES;
    reg16  crshFS;
    reg16  crshGS;
    long   crshEPC;
    long   crshEFR;
    reg16  crshTR;
    reg16  crshLDT;
    gxrv   crshIDT;
    gxrv   crshGDT;
    long   crshERRC;
    long   crshCR0;
    long   crshCR2;
    long   crshCR3;
    long   crshDATA[4];
    long   crshSTK[32];
} cd;

struct crshchar
{   lngstr_char crshdata;
    char        end;
} crshchar =
{   {PAR_GET|REP_DATAS, 0, "CRSHDATA", (char *)&cd, 0, sizeof(cd), sizeof(cd)}
};

type_qab crshqab =
{   QFNC_WAIT|QFNC_CLASSFUNC,	// qab_func
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    0,				// qab_handle
    0,				// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    CF_VALUES,			// qab_option
    0,				// qab_count
    "SYSTEM:", 0,		// qab_buffer1
    (char *)&crshchar, 0,	// qab_buffer2
    NULL, 0			// qab_parm
};

char  buffer[80] = " Stk: ";

int crshinfo(void)

{
    long *dp;
    char *pnt;
    long  offset;
    int   cnt1;
    int   cnt2;
    char  oneout;

    if ((offset=svcIoQueue(&crshqab)) < 0 || (offset=crshqab.qab_error) < 0)
        femsg2(prgname, "Error getting crash data", offset, NULL);
    ++validcount;
    if (!((cd.crshMAGIC[0] == 0xF0F0A5A5L || cd.crshMAGIC[0] == 0xF0A5A5L)
            && cd.crshMAGIC[1] == 0x48535243L && cd.crshMAGIC[2] == 0x41544144L)
            || cd.crshCODE == 0)
    {
        puts("No crash data saved");
        return (TRUE);
    }
    sdt2str(buffer+10, "%z%h:%m:%s on %D-%3n-%y", (time_sz *)&cd.crshTIME);
    printf("\nCrash data has %sbeen logged\nCrash occurred at %s, Code ="
            " %-4.4s\n", (cd.crshMAGIC[0] == 0xF0A5A5L)? "": "not ", buffer+10,
            cd.crshCODE);
    printf(" EAX: %08.8lX  EBX: %08.8lX  ECX: %08.8lX  EDX: %08.8lX\n",
            cd.crshEAX, cd.crshEBX, cd.crshECX, cd.crshEDX);
    printf(" ESP: %08.8X  EBP: %08.8X  EDI: %08.8X  ESI: %08.8X\n",
            cd.crshEBP, cd.crshEBP, cd.crshEDI, cd.crshESI);
    printf(" CS: %04.4X  SS: %04.4X  DS: %04.4X  ES: %04.4X  FS: %04.4X "
            " GS: %04.4X\n", cd.crshCS.v, cd.crshSS.v, cd.crshDS.v,
            cd.crshES.v, cd.crshFS.v, cd.crshGS.v);
    printf(" EPC: %08.8lX  EFR: %08.8lX  TR: %04.4X  LDT: %04.4X\n",
            cd.crshEPC, cd.crshEFR, cd.crshTR.v, cd.crshLDT.v);
    printf(" IDT: %04.4X-%08.8lX  GDT: %04.4X-%08.8lX\n", cd.crshIDT.size,
            cd.crshIDT.address, cd.crshGDT.size, cd.crshGDT.address);
    printf(" ERR: %08.8lX  CR0: %08.8lX  CR2: %08.8lX  CR3: %08.8lX\n",
            cd.crshERRC, cd.crshCR0, cd.crshCR2, cd.crshCR3);
    printf(" Data:           %08.8lX  %08.8lX  %08.8lX  %08.8lX\n",
            cd.crshDATA[0], cd.crshDATA[1], cd.crshDATA[2], cd.crshDATA[3]);
    dp = cd.crshSTK;
    cnt1 = 8;
    offset = cd.crshESP;
    oneout = FALSE;
    do
    {   pnt = buffer + 15;
        sprintf(buffer+6, "%08.8lX>", offset);
        cnt2 = 4;
        do
        {   if (offset >= 0x1000 && oneout)
                break;
            offset += 4;
            pnt += sprintf(pnt, "  %08.8lX", *dp++);
            oneout = TRUE;
        } while (--cnt2 > 0);
        if (pnt > (buffer+15))
            puts(buffer);
        strnmov(buffer+1, "    ", 4);
    } while (--cnt1 > 0 && offset < 0x1000);
    return (TRUE);
}
