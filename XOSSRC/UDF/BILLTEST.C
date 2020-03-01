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
#include <XOS.H>
#include <XOSSTR.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <ERRMSG.H>
#include "XOSINC:\PAR\UDF.H"
#include "XOSINC:\PAR\UDFSRV.H"
#include "XOSINC:\PAR\UDFUTIL.H"

#define VERSION 2
#define EDITNUM 0

extern ulong swapword (ulong value);
#pragma aux swapword =	\
   "xchg al, ah"	\
    parm [EAX] value [EAX];

extern ulong swaplong (ulong value);
#pragma aux swaplong =	\
   "xchg al, ah"	\
   "ror  eax, 16"	\
   "xchg al, ah"	\
    parm [EAX] value [EAX];


long   udp;
char   prgname[] = "BILLTEST";

extern long errno;

struct req
{   char  type;
    char  subtype;
    short seq;
    ulong procid;
    ulong handle;
    uchar data[100];
};

struct resp
{   char  type;
    char  subtype;
    short seq;
    ulong procid;
    ulong handle;
    uchar data[400];
};

struct
{   byte4_parm node;
    byte4_parm port;
    char       end;
} outparms =
{  {PAR_SET|REP_HEXV, 4, IOPAR_NETRMTNETAS, 0x64000000L},
   {PAR_SET|REP_HEXV, 4, IOPAR_NETRMTPORTS, 3002}
};

struct
{   byte4_parm node;
    byte4_parm port;
    char       end;
} inparms =
{  {PAR_GET|REP_HEXV, 4, IOPAR_NETRMTNETAR, 0},
   {PAR_GET|REP_HEXV, 4, IOPAR_NETRMTPORTR, 0}
};

ulong  userlen;
ulong  lastday;
ulong  lastmon;
ulong  cardtype;
ulong  cardmon;
ulong  cardyear;
ulong  cardziplen;
uchar *cardzip;
ulong  cardlimit;

char   havebillusr;


void main(
   int   argc,
   char *argv[])

{
    time_s  cdt;
    struct  req  req;
    struct  resp resp;
    GRPDATA grpdata;
    uchar  *pnt;
    long    rtn;
    char    group[20];

    if (argc != 2)
    {
       fputs("? UDFMERGE: Command error, usage is:\n"
             "            BILLTEST group\n", stderr);
       exit(1);
    }

    if ((strlen(argv[1]) > 16) ||
            (strmov(strmov(group,argv[1]),".X"), getgroup(group, &grpdata) < 0))
    {
        printf("? USEREDIT: Group %s is not defined\n", strupper(argv[1]));
        exit(1);
    }

    if ((udp = svcIoOpen(O_IN|O_OUT, "UDP0:", NULL)) < 0)
        femsg2(prgname, "Error opening UDP device", udp, NULL);

    svcSysDateTime(1, &cdt);

    req.type = MT_UDF;
    req.subtype = UDFM_BILLREQ;
    req.seq = 1;
    req.procid = svcSysGetPid();
    req.handle = 0;

    // First, send the UDFM_BILLREQ message and get the UDFM_BILLACP response

    req.data[0] = 7;			// BillDay
    req.data[1] = 10;			// BillMonth
    req.data[2] = 5;			// Window
    req.data[3] = 0;
    *(ulong *)(req.data + 4) = 0;
    *(ulong *)(req.data + 8) = 0;
    req.data[12] = grpdata.grplen;
    pnt = strnmov(req.data + 13, grpdata.grpname, grpdata.grplen);

    req.seq++;
    if ((rtn = reqresp(udp, grpdata.addr, grpdata.port, &req,
	    (pnt - (char *)&req), &resp, sizeof(resp))) < 0)
					// Send request and get response
	femsg2(prgname, "Error sending update request to UDF "
                            "server", rtn, NULL);
    if (rtn < 24)
    {
	fprintf(stderr, "? BILLTEST: UDFM_BILLACP response from UDF server is "
		"too short (%d bytes)\n", rtn);
	exit(1);
    }
    if (resp.subtype != UDFM_BILLACP)
    {
	printf("? BILLTEST: Expected UDFM_BILLACP; UDF server response "
		"was %d\n", resp.subtype);
	exit(1);
    }
    printf("Have UDFM_BILLACP response: handle = %X, window size = %d\n",
	    *(ulong *)(resp.data + 8), resp.data[12]);
    havebillusr = FALSE;
    for (;;)
    {
	// Read a UDFM_BILLUSR packet from the UDF server

	if (!havebillusr)
	{
	    if ((svcIoInBlockP(udp, (char *)&resp, sizeof(resp), &inparms)) < 0)
		femsg2(prgname, "Error inputting UDFM_BILLUSR packet", rtn, NULL);
	}
	if (rtn < 24)
	{
	    fprintf(stderr, "? BILLTEST: UDFM_BILLUSR packet from UDF server "
		"is too short (%d bytes)\n", rtn);
	    exit(1);
	}
	if (resp.subtype != UDFM_BILLUSR)
	{
	    printf("? BILLTEST: Expected UDFM_BILLUSR; UDF server packet was "
		"%d\n", resp.subtype);
	    exit(1);
	}
	userlen = resp.data[8];
	pnt = resp.data + 9 + userlen;
	lastday = pnt[0];
	lastmon = pnt[1];
	cardtype = pnt[2];
	cardmon = pnt[3];
	cardyear = pnt[4];
	cardziplen = pnt[5];
	cardzip = pnt + 6;
	pnt += (cardziplen + 6);
	cardlimit = *(ulong *)pnt;
	printf("Have UDFM_BILLUSR packet: handle = %08.8X, user = %.*s\n"
		"    Last day = %d, Last month = %d, Card type = %d\n"
		"    Card month = %d, Card year = %d\n, Card Zip = %.*s\n"
		"    Card limit = %d, Card num = %*.s\n",  resp.handle,

		userlen, *(ulong *)(resp.data + 21), lastday, lastmon, cardtype,
		cardmon, cardyear, cardziplen, cardzip, cardlimit, pnt[4],
		pnt + 5);

	// Output the UDFM_BILLACK packet

	req.subtype = UDFM_BILLACK;
	if ((svcIoOutBlockP(udp, (char *)&req, 12, &outparms)) < 0)
	    femsg2(prgname, "Error outputting UDFM_BILLACK packet", rtn, NULL);

	// Construct and output the UDFM_BILLUDN packet for this user

	req.subtype = UDFM_BILLACK;
	req.data[0] = 1;		// Billing status = normal
	req.data[1] = 0;		// Not used
	req.data[2] = 0;
	req.data[3] = 0;
	*(ulong *)(req.data + 4) = swaplong(222); // Total free connect time
	*(ulong *)(req.data + 8) = swaplong(111); // Total billed connect time
	*(ulong *)(req.data + 12) = swaplong(40321); // Total connect charges
	*(ulong *)(req.data + 16) = swaplong(1033); // Total other charges
	*(ulong *)(req.data + 20) = swaplong(222); // Amount of last payment
	*(ulong *)(req.data + 24) = swaplong(cdt.date); // Date of last payment
	if ((rtn = reqresp(udp, grpdata.addr, grpdata.port, &req, 40, &resp,
		sizeof(resp))) < 0)
	if (rtn < 12)
	{
	    fprintf(stderr, "? BILLTEST: UDFM_BILLUDA or UDFMBILLUSR packet "
		    "from UDF server is too short (%d bytes)\n", rtn);
	    exit(1);
	}
	if (resp.subtype != UDFM_BILLUDA && resp.subtype != UDFM_BILLUSR)
	{
	    printf("? BILLTEST: Expected UDMF_BILLUDA or UDFM_BILLUSR; UDF server packet was "
		"%d\n", resp.subtype);
	    exit(1);
	}
	havebillusr = (resp.subtype != UDFM_BILLUDA);
    }
}


void fail(void)

{
    femsg2(prgname, "Error reading input file", errno, NULL);
}
