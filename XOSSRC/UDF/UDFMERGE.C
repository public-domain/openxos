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
#include <XOSERRMSG.H>
#include <XOSUDF.H>
#include <XOSUDFUTIL.H>

#define VERSION 2
#define EDITNUM 0

// This version of UDFMERGE reads a C6 format UDF.

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

#define REPORTINTERVAL 100
#define ENDOFREC       5000
#define TOOBIG         5001

FILE  *input;
long   udp;
int    lvl1type;
char   prgname[] = "UDFMERGE";

extern long errno;

struct req
{   char  type;
    char  subtype;
    short seq;
    ulong procid;
    ulong bits1;
    ulong bits2;
    char  data[1000];
};

struct resp
{   char  type;
    char  subtype;
    short seq;
    ulong procid;
    ulong bits1;
    ulong bits2;
    char  data[20];
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


void main(
   int   argc,
   char *argv[])

{
    struct  req  req;
    struct  resp resp;
    GRPDATA grpdata;
    char   *reqpnt;
    uchar  *pnt;
    uchar  *keypnt;
    int     keysize;
    int     temp;
    int     cnt;
    long    rtn;
    int     recnum;
    int     rectype;
    int     recsize;
    int     nextnum;
    char    valid;
    char    group[20];
    uchar   buffer[4000];

    if (argc != 3)
    {
       fputs("? UDFMERGE: Command error, usage is:\n"
             "            UDFMERGE group filespec\n", stderr);
       exit(1);
    }

    if ((strlen(argv[1]) > 16) ||
            (strmov(strmov(group,argv[1]),".X"), getgroup(group, &grpdata) < 0))
    {
        printf("? USEREDIT: Group %s is not defined\n", strupper(argv[1]));
        exit(1);
    }
    if ((input = fopen(argv[2], "rb")) == NULL)
        femsg2(prgname, "Error opening input file", errno, NULL);

    if (fgetc(input) != 0x81 || fgetc(input) != 0xC6)
    {
        fprintf(stderr, "? UDFMERGE: File header is incorrect\n");
        exit(1);
    }
    if ((udp = svcIoOpen(O_IN|O_OUT, "UDP0:", NULL)) < 0)
        femsg2(prgname, "Error opening UDP device", udp, NULL);

    req.type = MT_UDF;
    req.subtype = 1;
    req.seq = 1;
    req.procid = svcSysGetPid();
    req.data[0] = grpdata.grplen;
    reqpnt = strnmov(req.data+1, grpdata.grpname, grpdata.grplen);
    recnum = 0;
    nextnum = REPORTINTERVAL;
    for (;;)
    {
	if ((temp = fgetc(input)) < 0)
	{
	    if (errno == -ER_EOF)
		break;
	    else
		fail();
	}
	if (temp != 0xAA)
	{
	    fputs("% UDFMERGE: Did not find beginning of record, skipping to "
		    "next record", stderr);
	    while ((getl1byte()) >= 0)
		;
	    if (errno == -ER_EOF)
		break;
	    if (errno == ENDOFREC)
		continue;
	    fail();
	}
        if ((lvl1type = getl1byte()) < 0)
	{
	    if (errno == -ER_EOF)
		break;
	    if (errno == ENDOFREC)
		continue;
	    fail();
	}
        ++recnum;
        if (recnum >= nextnum)
        {
            printf("%% UDFMERGE: %6d records processed\r", recnum);
            nextnum += REPORTINTERVAL;
        }

        if (lvl1type == 1 || lvl1type == 2)
        {
	    keypnt = NULL;
	    valid = TRUE;
            pnt = buffer;
	    cnt = sizeof(buffer) - 4;
            do
            {
                if ((rectype = getl1byte()) < 0)
		{
		    if (errno != ENDOFREC && errno != -ER_EOF)
			valid = FALSE;
		    break;
		}
                if ((recsize = getl1byte()) < 0)
		{
		    valid = FALSE;
		    break;
		}
                cnt -= 2;
                if (recsize & 0x80)
                {
		    if ((temp = getl1byte()) < 0)
		    {
			valid = FALSE;
			break;
		    }
                    recsize = ((recsize & 0x7F) << 8) + temp;
                }
		*pnt++ = rectype;
		*pnt++ = recsize;
		cnt -= 2;
		if (recsize > 127)
		{
		    *pnt++ = (recsize >> 8) | 0x80;
		    --cnt;
		}
		if ((cnt -= recsize) < 0)
		{
		    errno = TOOBIG;
		    break;
		}
		if (keypnt == NULL && (rectype == 1 || rectype == 32))
		{
		    keypnt = pnt;
		    keysize = recsize;
		}
		while (--recsize >= 0)
		{
		    *pnt++ = getl1byte();
		}
            } while (cnt > 0);

	    if (!valid)
	    {
		if (errno == TOOBIG)
		{
		    fputs("? UDFMERGE: Input record is too long\n", stderr);
		    exit(1);
		}
		else if (errno == -ER_EOF)
		{
		    fputs("% UDFMERGE: Warning: Unexpected EOF\n", stderr);
		    break;
		}
		femsg(prgname, errno, NULL);
	    }

	    // Here with the record copied to our record buffer

            if (keypnt == NULL)
                printf("\n% UDFMERGE: Record %d at %d has no key, record "
			"ignored\n", recnum, ftell(input));
            else
            {
		recsize = pnt - buffer;
                req.bits1 = swaplong((rectype == 1)?
                        (URQ1_ACTIVE|URQ1_CREATE|URQ1_UPDATE):
                        (URQ1_ACTIVE|URQ1_CREATE|URQ1_UPDATE|URQ1_KEYUID));
                if (lvl1type == 2)
                    req.bits1 ^= (URQ1_ACTIVE|URQ1_INACTIVE);
                req.bits2 = 0;
                req.seq++;
                reqpnt[0] = keysize;
                memcpy(reqpnt + 1, keypnt, keysize); // Store the key
                memcpy(reqpnt + 1 + keysize, buffer, recsize);
					// Store the record
                if ((rtn = reqresp(udp, grpdata.addr, grpdata.port, &req,
                        (reqpnt - (char *)&req) + keysize + 1 + recsize, &resp,
			sizeof(resp))) < 0)
					// Send request and get response
                    femsg2(prgname, "Error sending update request to UDF "
                            "server", rtn, NULL);
                if (rtn < 16)
                {
                    fprintf(stderr, "? UDFMERGE: Response from UDF server is "
                            "too short (%d bytes)\n", rtn);
                    exit(1);
                }
                if (resp.subtype != 2)
                    printf("\nRec # %d at %d, UDF server response was %d\n",
                            recnum, ftell(input), resp.subtype);
            }
        }
	else				// If don't have a type 1 or 2 record
	{
	    fprintf(stderr, "? UDFMERGE: Illegal record type %d\n", rectype);
	    exit(1);
	}
    }
}

int getl1byte(void)

{
    int temp;

    if ((temp = fgetc(input)) < 0)
	return (-1);
    if (temp == 0xAA)
    {
        ungetc(0xAA, input);
	errno = ENDOFREC;
	return (-1);
    }
    return (temp);
}

void fail(void)

{
    femsg2(prgname, "Error reading input file", errno, NULL);
}
