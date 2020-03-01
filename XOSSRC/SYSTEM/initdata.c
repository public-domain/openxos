/******************************************/
/*                                        */
/* INITDATA.C - Data definitions for INIT */
/*                                        */
/******************************************/
/*                                        */
/* Written by John Goltz                  */
/*                                        */
/******************************************/

// ++++
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
#include <XOSVECT.H>
#include <XOSSVC.H>
#include <XOS.H>
#include <XOSTRM.H>
#include <XOSTIME.H>
#include <XOSSYSP.H>
#include <XOSUSR.H>
#include "INIT.H"

int     mincnt = 60;
ulong   initpid;				// Process ID for INIT
long    conhndl;
long    logdev;
long    logdate;
time_sz bgndt;
long    rtnval;
char    dttmbfr[40];			// Buffer for date/time string

char welcome[50];
char childtext[160] = "----PROCTERM----Level 1 process ";

struct ratedata ratedata1[] =	// Auto-baud data for type 1
{   {0xFF, 0x00,  110, 1200},	// 110 baud
    {0xFF, 0x00,  150, 1200},	// 150 baud
    {0xFF, 0x00,  300,  600},	// 300 baud
    {0xFF, 0x00, 1200,  200},	// 1200 baud
    {0xFF, 0x00, 2400,  100},	// 2400 baud
};
int numrates1 = (sizeof(ratedata1))/(sizeof(struct ratedata));

struct ratedata ratedata2[] =	// Auto-baud data for type 2
{   {0xFF, 0x00,  300, 600},	// 300 baud
    {0xFF, 0x80,  600, 300},	// 600 baud
    {0xFF, 0x78, 1200, 200},	// 1200 baud
    {0xED, 0x8C, 1800, 100},	// 1800 baud
    {0xFF, 0xE6, 2400, 100},	// 2400 baud
    {0xDC, 0x18, 3600, 100},	// 3600 baud
    {0xFF, 0x0D, 4800,   0},	// 4800 baud
    {0xF0, 0xF0, 9600,   0}		// 9600 baud
};
int numrates2 = (sizeof(ratedata2))/(sizeof(struct ratedata));

uchar logcommit = FALSE;

char prgname[]  = "INIT";

char sysname[34];
char startupdone = FALSE;		// TRUE if startup stuff complete

char ipmerrmsg[] = "? INIT: Cannot open message device IPM:SYS^INIT\r\n"
		"        %s\r\n";
///char ipmstarterr[] = "? INIT: Cannot start message input on IPM:SYS^INIT\r\n"
///		"        %s\r\n";
///char starterr[] = "? INIT: Cannot execute XOSCFG:\\STARTUP.BAT\r\n"
///		"        %s\r\n";

struct cfgchar cfgchar =
{   {(PAR_GET|REP_STR ), 0, "SYSNAME" , sysname, 0, 34, 0},
    {(PAR_GET|REP_DECV), 4, "TOTALMEM", 0}
};

struct statechar statechar =
{   {(PAR_SET|REP_HEXV), 4, "STATE", 1},
    {(PAR_GET|REP_TEXT), 4, "INITIAL", 0}
};

char systemspec[] = "SYSTEM:";

type_qab cfgqab =
{   QFNC_WAIT|QFNC_CLASSFUNC,	// qab_func    - Function
    0,							// qab_status  - Returned status
    0,							// qab_error   - Error code
    0,							// qab_amount  - Process ID
    0,							// qab_handle  - Device handle
    0,							// qab_vector  - Vector for interrupt
    0, 0,						//             - Reserved
    CF_VALUES,					// qab_option  - Options or command
    0,							// qab_count   - Count
    systemspec, 0, 				// qab_buffer1 - Pointer to file spec
    (char far *)&cfgchar, 0,	// qab_buffer2 - Unused
    NULL, 0						// qab_parm    - Pointer to parameter area
};

char msgbfr[2000];
char srcbfr[64];

struct msgparm msginpparm =
{   {(PAR_GET|REP_STR), 0, IOPAR_MSGRMTADDRR, srcbfr, 0, 64, 0}
};

struct msgparm msgoutparm =
{   {(PAR_SET|REP_STR), 0, IOPAR_MSGRMTADDRS, srcbfr, 0, 64, 0}
};

type_qab msginpqab =
{   QFNC_INBLOCK,				// qab_func    - Function
    0,							// qab_status  - Returned status
    0,							// qab_error   - Error code
    0,							// qab_amount  - Amount transfered
    0,							// qab_handle  - Device handle
    VECT_MESSAGE,				// qab_vector  - Vector for interrupt
    0, 0,						//             - Reserved
    0,							// qab_option  - Options or command
    2000,						// qab_count   - Amount to transfer
    msgbfr, 0,					// qab_buffer1 - Pointer to data buffer
    0, 0,						// qab_buffer2 - Pointer to source string
    &msginpparm, 0				// qab_parm    - Pointer to parameter area
};

struct trmclrparms trmclrparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0xFFFFFFFFL},
    {PAR_SET|REP_HEXV, 4, IOPAR_TRMSINPMODE, TIM_ECHO|TIM_ILFACR},
    {PAR_SET|REP_HEXV, 4, IOPAR_TRMCOUTMODE, 0xFFFFFFFFL}
};

char pwbufr[36];
char prgmbufr[36];

struct pwchar pwchar =
{   {(PAR_GET|REP_STR), 0, "PASSWORD", pwbufr  , 0, 32, 0},
    {(PAR_GET|REP_STR), 0, "PROGRAM" , prgmbufr, 0, 32, 0}
};

type_qab pwqab =
{   QFNC_WAIT|QFNC_DEVCHAR,		// qab_func    - Function
    0,							// qab_status  - Returned status
    0,							// qab_error   - Error code
    0,							// qab_amount  - Process ID
    0,							// qab_handle  - Device handle
    0,							// qab_vector  - Vector for interrupt
    0, 0,						//             - Reserved
    DCF_VALUES,					// qab_option  - Options or command
    0,							// qab_count   - Count
    0, 0,						// qab_buffer1 -
    (char far *)&pwchar, 0,		// qab_buffer2 - Pointer to char list
    0, 0						// qab_parm    -
};

struct devlist devlist =		// Device list for runprgm
{  {0, STDIN , O_IN      },		//   STDIN = 1
   {0, STDOUT, O_IN|O_OUT},		//   STDOUT = 2
   {0, STDERR, O_IN|O_OUT},		//   STDERR = 3
   {0, STDTRM, O_IN|O_OUT},		//   STDTRM = 5
};

struct runparm runparm =
{  {PAR_SET|REP_STR, 0, IOPAR_RUNCMDTAIL},
   {PAR_SET|REP_STR, 0, IOPAR_RUNDEVLIST, (char far *)&devlist, 0,
            sizeof(devlist), 0}
};

type_qab runqab =
{   RFNC_WAIT|RFNC_RUN,			// qab_func    - Function
    0,							// qab_status  - Returned status
    0,							// qab_error   - Error code
    0,							// qab_amount  - Process ID
    0,							// qab_handle  - Device handle
    0,							// qab_vector  - Vector for interrupt
    0, 0,						//             - Reserved
    R_CHILDTERM,				// qab_option  - Options or command
    0,							// qab_count   - Count
    NULL, 0,					// qab_buffer1 - Pointer to file spec
    NULL, 0,					// qab_buffer2 - Unused
    &runparm, 0					// qab_parm    - Pointer to parameter area
};
