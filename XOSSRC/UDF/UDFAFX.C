//========================================================
// Program: UDFAFX.C - Changes user directory IP addresses
//========================================================

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
#include <ENCPSWD.H>
#include "XOSINC:\PAR\UDF.H"

long   newudf;
long   oldudf;

long   offset;

uchar  blkbufr[1024];
uchar  inbufr[2048];
uchar  outbufr[2048];
uchar *blkpnt;
uchar *inpnt;
uchar *outpnt;

int    blkcnt;
int    incnt;
int    size;
int    itemsz;
int    copynum;
int    delnum;
int    ipanum;

union
{   long  l;
    uchar c[4];
} oldipaddr;
union
{   long  l;
    uchar c[4];
} newipaddr;
union
{   long  l;
    uchar c[4];
} ipaddr;

char  *cpnt;

uchar  header[2] = {0x81, 0xC6};
char   prgname[] = "UDFAFX";
uchar  newbfr[132] = {UDF_HOMEDIR};

extern long errno;


ulong getipaddr(char *str);
int   getvalue(int stopper);


void main(
   int   argc,
   char *argv[])

{
    char *pnt;
    int   rtn;
    int   data;

    if (argc != 5)
    {
       fputs("? UDFAFX: Command error, usage is:\n"
             "            UDFAFX oldfile newfile oldipaddr newipaddr\n",
	     stderr);
       exit(1);
    }

    oldipaddr.l = getipaddr(argv[3]);
    newipaddr.l = getipaddr(argv[4]);

    if ((oldudf = svcIoOpen(O_IN, argv[1], NULL)) < 0)
        femsg2(prgname, "Error opening input file", oldudf, NULL);
    if ((newudf = svcIoOpen(O_OUT|O_CREATE|O_TRUNCA, argv[2], NULL)) < 0)
        femsg2(prgname, "Error opening output file", newudf, NULL);
    if ((rtn = svcIoInBlock(oldudf, inbufr, 2)) < 0)
        femsg2(prgname, "Error reading input file", rtn, NULL);
    if (inbufr[0] != 0x81 || inbufr[1] != 0xC6)
    {
        fprintf(stderr, "? UDFAFX: Input file header is incorrect\n");
        exit(1);
    }
    if ((rtn = svcIoOutBlock(newudf, header, 2)) < 0)
	femsg2(prgname, "Error writing output file", rtn, NULL);

    offset = 2;

    if ((data = getbyte()) < 0)
    {
	fputs("? UDFAFX: No data in input file\n", stderr);
	exit(1);
    }
    if (data != 0xAA)
    {
	fputs("? UDFAFX: Initial byte is not beginning of record\n", stderr);
	exit(1);
    }
    for (;;)
    {
	inpnt = inbufr;
	incnt = 2048;
	for (;;)
	{
	    if ((data = getbyte()) < 0 || data == 0xAA)
		break;
	    if (data == 0xAB)
	    {
		switch (data = getbyte())
		{
		 case 1:
		    data = 0xAA;
		    break;

		 case 2:
		    data = 0xAB;
		    break;

		 default:
		    printf("%% UDFPWC: Unexpected prefixed value 0x%02.2X at "
			   "offset %d ignored\n", data, offset);
		    continue;
		}
	    }
	    if (--incnt > 0)
		*inpnt++ = data;
	    else if (incnt == 0)
		printf("%% UDFPWC: Record to long at offset %d\n", offset);
	}

	if ((incnt = inpnt - inbufr) < 2)
	{
	    if (data == -1)
		finish();
	    printf("%% UDFPWC: Short item discarded at offset %d\n", offset);

	    continue;
	}
	if (inbufr[0] == 0xFF)
	{
	    ++delnum;
	    continue;
	}
	++copynum;
	outbufr[0] = 0xAA;
	outbufr[1] = inbufr[0];		// Copy the record type byte
	inpnt = inbufr + 1;
        outpnt = outbufr + 2;
	do
	{
	    if (inpnt[0] == 0)
		break;
	    itemsz = inpnt[1] + 2;
	    if (inpnt[0] == UDF_HOMEDIR &&
		    strnicmp("AFP0:", inpnt + 2, 5) == 0)
	    {
		cpnt = inpnt + 7;
		ipaddr.c[0] = getvalue('.');
		ipaddr.c[1] = getvalue('.');
		ipaddr.c[2] = getvalue('.');
		ipaddr.c[3] = getvalue(':');
		if (cpnt != NULL && ipaddr.l == oldipaddr.l)
		{
		    pnt = strnmov(newbfr + 2, inpnt + 2, 5);
		    pnt += sprintf(pnt, "%d.%d.%d.%d:", newipaddr.c[0],
			    newipaddr.c[1], newipaddr.c[2], newipaddr.c[3]);
		    rtn = itemsz - (cpnt - inpnt);
		    memcpy(pnt, cpnt, rtn);
		    rtn += (pnt - newbfr);
		    newbfr[1] = rtn - 2;
		    putitem(newbfr, rtn);
		    ipanum++;
		    inpnt += itemsz;
		    continue;
		}
	    }
	    putitem(inpnt, itemsz);
	    inpnt += itemsz;
	} while ((incnt -= itemsz) >= 2);
	if ((rtn = svcIoOutBlock(newudf, outbufr, outpnt - outbufr)) < 0)
	    femsg2(prgname, "Error writing output file", rtn, NULL);
    }
}

int getbyte(void)

{
    if (--blkcnt < 0)
    {
        if ((blkcnt = svcIoInBlock(oldudf, blkbufr, 1024)) < 0)
        {
            if (blkcnt == ER_EOF)
                return (-1);
            femsg2(prgname, "Error reading input file", blkcnt, NULL);
        }
	blkcnt--;
	blkpnt = blkbufr;
    }
    ++offset;
    return (*blkpnt++);
}

void putitem(
    uchar *bufr,
    int    size)

{
    uchar chr;

    do
    {
	if ((chr = *bufr++) == 0xAA || chr == 0xAB)
	{
	    *outpnt++ = 0xAB;
	    *outpnt++ = chr - 0xA9;
	}
	else
	    *outpnt++ = chr;
    } while (--size > 0);
}


ulong getipaddr(
    char *str)

{
    uchar *vp;
    ulong  value;

    cpnt = str;
    value = 0;
    vp = (uchar *)&value;
    *vp++ = getvalue('.');
    *vp++ = getvalue('.');
    *vp++ = getvalue('.');
    *vp = getvalue(0);
    if (cpnt == NULL)
	value = 0;
    return (value);
}

int getvalue(
    int stopper)

{
    int  value;
    char chr;

    if (cpnt != NULL)
    {
	value = 0;
	while ((chr = *cpnt++) != 0 && isdigit(chr))
	    value = value * 10 + (chr & 0x0F);
    }
    if (chr != stopper)
    {
	cpnt = NULL;
	value = 0;
    }
    return (value);
}


void finish(void)

{
    long rtn;

    if ((rtn = svcIoClose(newudf, 0)) < 0)
	femsg2(prgname, "Error closing output file", rtn, NULL);
    printf("%% UDFPWC: %d record%s copied, %d deleted record%s discarded,\n"
	    "          %d IP address%s converted\n", copynum,
	    (copynum == 1)? "": "s", delnum, (delnum == 1)? "": "s",
	    ipanum, (ipanum == 1)? "": "es");
    exit(0);
}
