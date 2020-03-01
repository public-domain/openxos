//============================================================
// Program: UDFC5XC6.C - Converts from the original UDF format
//		(C5) to the new prefixed format (C6)
//============================================================

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

long  newudf;
long  oldudf;

long  offset;
int   lvl1size;
int   lvl1type;
int   outsize;
int   lvl1num = 0;
int   prefixnum = 0;

uchar *lvl2pnt;
int    lvl2type;
int    lvl2size;
int    lvl2cnt;
int    lvl2left;

uchar *inbufr;
uchar *outbufr;
uchar *inpnt;
uchar *outpnt;

int    incnt;
int    outleft;

uchar  header[2] = {0x81, 0xC6};
char   prgname[] = "UDFC5XC6";

extern long errno;


void main(
   int   argc,
   char *argv[])

{
    long rtn;
    int  chr;
    char lvl2bufr[2000];

    if (argc != 3)
    {
       fputs("? UDFC5XC6: Command error, usage is:\n"
             "            UDFC5XC6 oldfile newfile\n", stderr);
       exit(1);
    }
    if ((oldudf = svcIoOpen(O_IN, argv[1], NULL)) < 0)
        femsg2(prgname, "Error opening input file", oldudf, NULL);
    if ((newudf = svcIoOpen(O_OUT|O_CREATE|O_TRUNCA, argv[2], NULL)) < 0)
        femsg2(prgname, "Error opening output file", newudf, NULL);
    inbufr = (char *)getspace(4000);
    outbufr = (char *)getspace(6000);
    if ((lvl1size = svcIoInBlock(oldudf, inbufr, 2)) < 0)
        femsg2(prgname, "Error reading input file", lvl1size, NULL);
    if (inbufr[0] != 0x81 || inbufr[1] != 0xC5)
    {
        fprintf(stderr, "? UDFC5XC6: Input file header is incorrect\n");
        exit(1);
    }
    if ((rtn = svcIoOutBlock(newudf, header, 2)) < 0)
	femsg2(prgname, "Error writing output file", rtn, NULL);

    offset = 2;
    for (;;)
    {
        if ((lvl1size = svcIoInBlock(oldudf, inbufr, 3)) < 0)
        {
            if (lvl1size == ER_EOF)
                finish();
            femsg2(prgname, "Error reading input file", lvl1size, NULL);
        }
        if (lvl1size != 3)
        {
            fprintf(stderr, "? UDFC5XC6: EOF while reading level one header at"
                    " %ld\n", offset);
            exit(1);

        }
        lvl1type = inbufr[0];
        lvl1size = inbufr[1];
        if (lvl1size & 0x80)
        {
            lvl1size = ((lvl1size << 8) + inbufr[2]) & 0x7FFF;
            inpnt = inbufr;
            rtn = lvl1size;
        }
        else
        {
            inpnt = inbufr + 1;
            inbufr[0] = inbufr[2];
            rtn = lvl1size - 1;
        }
        if ((rtn = svcIoInBlock(oldudf, inpnt, rtn)) < 0)
            femsg2(prgname, "Error reading input file", lvl1size, NULL);
	++lvl1num;
	outbufr[0] = 0xAA;
	outbufr[1] = lvl1type;
	outpnt = outbufr+2;
	outleft = 6000-2;
	inpnt = inbufr;
	incnt = lvl1size;
	while (--incnt >= 0)
	{
	    if ((lvl2type = *inpnt++) == 0) // Get level 2 type
		break;
	    lvl2size = getlvl2chr();	// Get first size byte
	    if (lvl2size & 0x80)
		lvl2size = ((lvl2size & 0x7FF) << 8) + getlvl2chr();
	    lvl2pnt = lvl2bufr;
	    lvl2left = sizeof(lvl2bufr);
	    lvl2cnt = lvl2size;
	    while (--lvl2cnt >= 0)
	    {
		if (--lvl2left < 0)
		    toobig();
		if ((chr = getlvl2chr()) == 0xAA || chr == 0xAB)
		{
		    ++prefixnum;
		    *lvl2pnt++ = 0xAB;
		    if (--lvl2left < 0)
			toobig();
		    *lvl2pnt++ = (chr == 0xAA)? 1: 2;
		}
		else
		    *lvl2pnt++ = chr;
	    }
	    lvl2cnt = lvl2size;
	    lvl2pnt = lvl2bufr;
	    if (--outleft < 0)
		toobig();
	    *outpnt++ = lvl2type;
	    if (lvl2size > 0x7F)
	    {
		if (--outleft < 0)
		    toobig();
		*outpnt++ = (lvl2size >> 8) | 0x80;
		lvl2size &= 0xFF;
	    }
	    if (--outleft < 0)
		toobig();
	    if (lvl2size == 0xAA || lvl2size == 0xAB)
	    {
		++prefixnum;
		*outpnt++ = 0xAB;
		if (--outleft < 0)
		    toobig();
		*outpnt++ = (chr == 0xAA)? 1: 2;
	    }
	    else
		*outpnt++ = lvl2size;
	    if ((outleft -= lvl2cnt) < 0)
		toobig();
	    lvl2pnt = lvl2bufr;
	    while (--lvl2cnt >= 0)
		*outpnt++ = *lvl2pnt++;
	}
	outsize = outpnt - outbufr;
        if ((rtn = svcIoOutBlock(newudf, outbufr, outsize)) < 0)
            femsg2(prgname, "Error writing output file", rtn, NULL);
	offset += outsize;
    }
}

int getlvl2chr(void)

{
    if (--incnt < 0)
    {
	fprintf(stdout, "? UDFC5XC6: Level 2 record is too long for "
		"containing level 1 record\n              at offset %d\n",
		offset);
	exit(1);
    }
    return (*inpnt++);
}

void toobig(void)

{
    fprintf(stdout, "? UDFC5XC6: Input record is too long for output buffer at "
	    "offset %d\n", offset);
    exit(1);
}

void finish(void)

{
    long rtn;

    if ((rtn = svcIoClose(newudf, 0)) < 0)
	femsg2(prgname, "Error closing output file", rtn, NULL);
    printf("%% UDFC5XC6: %d record%s converted, %d prefix byte%s added\n",
	    lvl1num, (lvl1num == 1)? "": "s",
	    prefixnum, (prefixnum == 1)? "": "s");
    exit(0);
}
