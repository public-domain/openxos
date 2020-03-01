//--------------------------------------------------------------------------*
// CRSHSAVE.C
//
// Written by: John R. Goltz
//
// Edit History:
// 01/13/94(jrg) - Created first version
// 05/03/94(brn) - Changed to not return error status when run
// 05/12/94(brn) - Fixed command abbreviations
// 02/28/95(sao) - Added progarg package
// 05/16/95(sao) - Changed return codes to reflect XOSRTN
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
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

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSTIME.H>
#include <XOSRTN.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>

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
    long   crshEIP;
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
} crshgetchar =
{   {PAR_GET|REP_DATAS, 0, "CRSHDATA", (char *)&cd, 0, sizeof(cd), sizeof(cd)}
};

struct crshchar crshsetchar =
{   {PAR_SET|REP_DATAS, 0, "CRSHDATA", (char *)&cd, 0, sizeof(cd), sizeof(cd)}
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
    (char near *)"SYSTEM:", 0,	// qab_buffer1
    (char *)&crshgetchar, 0,	// qab_buffer2
    NULL, 0			// qab_parm
};

#define VERSION 3
#define EDITNO  2

Prog_Info pib;

char copymsg[] = "";
char prgname[] = "CRSHSAVE";
char description[] = "CRSHSAVE saves extended crash " \
    "data to the system log.  Normally this utility will be launched " \
    "during system startup (i.e. included in either STARTUP.BAT or " \
    "USTARTUP.BAT.";
char example[] = "{/option}";

long quiet = FALSE;
long mute = FALSE;

arg_spec options[] =
{
    {"Q*UIET" , ASF_BOOL|ASF_STORE, NULL, &quiet, TRUE, "Suppress all but error messages."},
    {"M*UTE"  , ASF_BOOL|ASF_STORE, NULL, &mute , TRUE, "Suppress all messages."},
    {"?"      , 0, NULL, AF(opthelp), 0, "This message." },
    {"H*ELP"  , 0, NULL, AF(opthelp), 0, "This message." },
    { NULL    , 0, NULL, AF(NULL)   , 0, NULL }
};

void   init_Vars(void);


int main(argc, argv)
int   argc;
char *argv[];
{
    long *dp;
    char *bfr;
    long  offset;
    int   cnt1;
    int   cnt2;
    char  oneout;
    char  timebfr[40];
    char  msgbfr[2000];


	reg_pib(&pib);

	init_Vars();

    if (argc > 1)		/* Is there an arg to process?	*/
    {
	argv++;
	progarg(argv, 0, options, NULL, (int(*)(char *))NULL,
                (void (*)(char *, char*))NULL, (int (*)(void))NULL, NULL);
    }

    if ((offset=svcIoQueue(&crshqab)) < 0 || (offset=crshqab.qab_error) < 0)
	{
        femsg2(prgname, "Error getting crash data", offset, NULL);
		exit(EXIT_SVCERR);
	}
    if (!(cd.crshMAGIC[0] == 0xF0F0A5A5L && cd.crshMAGIC[1] == 0x48535243L &&
            cd.crshMAGIC[2] == 0x41544144L) || cd.crshCODE == 0)
    {
        if (!quiet && !mute)
            puts("CRSHSAVE: No crash data to log");
        return (EXIT_NORM);
    }
    sdt2str(timebfr, "%D-%3n-%y %z%h:%m:%s.%f", (time_sz *)&cd.crshTIME);
    bfr = msgbfr;
    bfr += sprintf(bfr, "  + CRSHDATA%-4.4s at %s\n", cd.crshCODE, timebfr);
    bfr += sprintf(bfr, "EAX: %08.8lX  EBX: %08.8lX  ECX: %08.8lX"
            "  EDX: %08.8lX\n", cd.crshEAX, cd.crshEBX, cd.crshECX,
            cd.crshEDX);
    bfr += sprintf(bfr, "ESP: %08.8X  EBP: %08.8X  EDI: %08.8X"
            "  ESI: %08.8X\n", cd.crshEBP, cd.crshEBP, cd.crshEDI,
            cd.crshESI);
    bfr += sprintf(bfr, "CS: %04.4X  SS: %04.4X  DS: %04.4X  ES: %04.4X"
            "  FS: %04.4X  GS: %04.4X\n", cd.crshCS.v, cd.crshSS.v,
            cd.crshDS.v, cd.crshES.v, cd.crshFS.v, cd.crshGS.v);
    bfr += sprintf(bfr, "EIP: %08.8lX  EFR: %08.8lX  TR: %04.4X"
            "  LDT: %04.4X\n", cd.crshEIP, cd.crshEFR, cd.crshTR.v,
            cd.crshLDT.v);
    bfr += sprintf(bfr, "IDT: %04.4X-%08.8lX  GDT: %04.4X-%08.8lX\n",
            cd.crshIDT.size, cd.crshIDT.address, cd.crshGDT.size,
            cd.crshGDT.address);
    bfr += sprintf(bfr, "ERR: %08.8lX  CR0: %08.8lX  CR2: %08.8lX"
            "  CR3: %08.8lX\n", cd.crshERRC, cd.crshCR0, cd.crshCR2,
            cd.crshCR3);
    bfr += sprintf(bfr, "Data:%08.8lX  %08.8lX  %08.8lX"
            "  %08.8lX\n", cd.crshDATA[0], cd.crshDATA[1], cd.crshDATA[2],
            cd.crshDATA[3]);
    dp = cd.crshSTK;
    cnt1 = 8;
    offset = cd.crshESP;
    oneout = FALSE;
    bfr = strmov(bfr, "Stk: ");
    for (;;)
    {   bfr += sprintf(bfr, "%08.8lX>", offset);
        cnt2 = 4;
        do
        {   if (offset >= 0x1000 && oneout)
                break;
            offset += 4;
            if (cnt2 != 4)
                *bfr++ = ' ';
            bfr += sprintf(bfr, " %08.8lX", *dp++);
            oneout = TRUE;
        } while (--cnt2 > 0);
        if (--cnt1 <= 0 || offset >= 0x1000)
            break;
        bfr = strmov(bfr, "\n     ");
    }
    if ((offset = svcSysLog(msgbfr, bfr-msgbfr)) < 0)
	{
        femsg2(prgname, "Error logging crash data", offset, NULL);
		exit(EXIT_SVCERR);
	}
    else
    {
        crshqab.qab_buffer2 = (char *)&crshsetchar;
        crshsetchar.crshdata.buffer[0] = 0;
        svcIoQueue(&crshqab);
        if ( !quiet && !mute )
            puts("CRSHSAVE: Crash data logged");
    }
    return (EXIT_NORM);
}

//***************************************************
// Function: init_Vars - Set up the program information block
// Returned: none
//***************************************************
void init_Vars(void){

	// set Program Information Block variables
	pib.opttbl=options; 		// Load the option table
	pib.majedt = VERSION; 			// major edit number
	pib.minedt = EDITNO; 			// minor edit number
	pib.copymsg=copymsg;
	pib.prgname=prgname;
	pib.desc=description;
	pib.example=example;
	pib.build=__DATE__;
	pib.errno=0;
	getTrmParms();
	getHelpClr();
};

