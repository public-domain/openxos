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
#include <SIGNAL.H>
#include <XOSSTR.H>
#include <XOS.H>
#include <XOSERMSG.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSTIME.H>
#include <XOSTRM.H>

uchar bufr1[64];
uchar bufr2[64];

type_qab qab1 =
{   QFNC_INBLOCK,		// qab_func
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    0,				// qab_handle
    60,				// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    0,				// qab_option
    sizeof(bufr1),		// qab_count
    bufr1, 0,			// qab_buffer1
    NULL, 0,			// qab_buffer2
    NULL, 0			// qab_parm
};

type_qab qab2 =
{   QFNC_INBLOCK,		// qab_func
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    0,				// qab_handle
    61,				// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    0,				// qab_option
    sizeof(bufr2),		// qab_count
    bufr2, 0,			// qab_buffer1
    NULL, 0,			// qab_buffer2
    NULL, 0			// qab_parm
};

struct
{   byte4_char rate;
    byte1_char dbits;
    text8_char rarity;
    char       end;
} trmchars =
{   {PAR_SET|REP_HEXV, 4, "RATE"  , 9600},
    {PAR_SET|REP_HEXV, 1, "DBITS" , 8},
    {PAR_SET|REP_TEXT, 8, "PARITY", "M"}
};

struct
{   byte4_parm cval;
    byte4_parm sval;
    char       end;
} trmparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0x7FFFFFFF},
    {PAR_SET|REP_HEXV, 4, IOPAR_TRMSINPMODE, TIM_IMAGE}
};

char prgname[] = "MDBMON";


void havechar1(void);
void havechar2(void);


void main(void)

{
    long rtn;

    setvector(60, 0x04, havechar1);
    setvector(61, 0x04, havechar2);
    if ((qab1.qab_handle = svcIoOpen(O_IN, "TRM1:", NULL)) < 0)
	femsg2(prgname, "Error opening TRM1", qab1.qab_handle, NULL);
    if ((qab2.qab_handle = svcIoOpen(O_IN, "TRM2:", NULL)) < 0)
	femsg2(prgname, "Error opening TRM2", qab2.qab_handle, NULL);
    if ((rtn = svcIoDevChar(qab1.qab_handle, &trmchars)) < 0)
	femsg2(prgname, "Error setting characteristics for TRM1",
		qab1.qab_handle, NULL);
    if ((rtn = svcIoDevChar(qab2.qab_handle, &trmchars)) < 0)
	femsg2(prgname, "Error setting characteristics for TRM2",
		qab2.qab_handle, NULL);
    if ((rtn = svcIoInBlockP(qab1.qab_handle, NULL, 0, &trmparms)) < 0)
	femsg2(prgname, "Error setting parameters for TRM1", qab1.qab_handle,
		NULL);
    if ((rtn = svcIoInBlockP(qab2.qab_handle, NULL, 0, &trmparms)) < 0)
	femsg2(prgname, "Error setting parameters for TRM2", qab2.qab_handle,
		NULL);
    if ((rtn = svcIoQueue(&qab1)) < 0)
	femsg2(prgname, "Error starting input on TRM1", rtn, NULL);
    if ((rtn = svcIoQueue(&qab2)) < 0)
	femsg2(prgname, "Error starting input on TRM2", rtn, NULL);
    svcSchSetLevel(0);
    for(;;)
    {
	svcSchSuspend(NULL, -1);
    }
}

void havechar1(void)

{
    char   *pnt;
    time_sz dt;
    long    rtn;
    char    time[32];

    if ((qab1.qab_status & QSTS_DONE) == 0)
	return;
    if (qab1.qab_error < 0)
	femsg2(prgname, "Error on input from TRM1", qab1.qab_error, NULL);
    svcSysDateTime(11, &dt);
    sdt2str(time, "%m:%s.%f", &dt);
    pnt = bufr1;
    while (--qab1.qab_amount >= 0)
	printf("      %d %02.2X  %s\n", 0, *pnt++, time);
    if ((rtn = svcIoQueue(&qab1)) < 0)
	femsg2(prgname, "Error starting input on TRM1", rtn, NULL);
}

void havechar2(void)

{
    char   *pnt;
    time_sz dt;
    long    rtn;
    char    time[32];

    if ((qab2.qab_status & QSTS_DONE) == 0)
	return;
    if (qab2.qab_error < 0)
	femsg2(prgname, "Error on input from TRM2", qab2.qab_error, NULL);
    svcSysDateTime(11, &dt);
    sdt2str(time, "%m:%s.%f", &dt);
    pnt = bufr2;
    while (--qab2.qab_amount >= 0)
	printf("      %d %02.2X  %s\n", 0, *pnt++, time);
    if ((rtn = svcIoQueue(&qab2)) < 0)
	femsg2(prgname, "Error starting input on TRM2", rtn, NULL);
}
