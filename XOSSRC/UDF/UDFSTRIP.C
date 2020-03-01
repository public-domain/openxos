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
int    memnum;

uchar  header[2] = {0x81, 0xC6};
char   prgname[] = "UDFAFX";
uchar  newbfr[132] = {UDF_HOMEDIR};

extern long errno;



void main(
   int   argc,
   char *argv[])

{
    int  rtn;
    int  data;
    char useit;

    if (argc != 3)
    {
       fputs("? UDFSTRIP: Command error, usage is:\n"
             "              UDFSTRIP oldfile newfile\n",
	     stderr);
       exit(1);
    }
    if ((oldudf = svcIoOpen(O_IN, argv[1], NULL)) < 0)
        femsg2(prgname, "Error opening input file", oldudf, NULL);
    if ((newudf = svcIoOpen(O_OUT|O_CREATE|O_TRUNCA, argv[2], NULL)) < 0)
        femsg2(prgname, "Error opening output file", newudf, NULL);
    if ((rtn = svcIoInBlock(oldudf, inbufr, 2)) < 0)
        femsg2(prgname, "Error reading input file", rtn, NULL);
    if (inbufr[0] != 0x81 || inbufr[1] != 0xC6)
    {
        fprintf(stderr, "? UDFSTRIP: Input file header is incorrect\n");
        exit(1);
    }
    if ((rtn = svcIoOutBlock(newudf, header, 2)) < 0)
	femsg2(prgname, "Error writing output file", rtn, NULL);

    offset = 2;

    if ((data = getbyte()) < 0)
    {
	fputs("? UDFSTRIP: No data in input file\n", stderr);
	exit(1);
    }
    if (data != 0xAA)
    {
	fputs("? UDFSTRIP: Initial byte is not beginning of record\n", stderr);
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
		    printf("%% UDFSTRIP: Unexpected prefixed value 0x%02.2X at "
			   "offset %d ignored\n", data, offset);
		    continue;
		}
	    }
	    if (--incnt > 0)
		*inpnt++ = data;
	    else if (incnt == 0)
		printf("%% UDFSTRIP: Record to long at offset %d\n", offset);
	}

	if ((incnt = inpnt - inbufr) < 2)
	{
	    if (data == -1)
		finish();
	    printf("%% UDFSTRIP: Short item discarded at offset %d\n", offset);

	    continue;
	}
	if (inbufr[0] == 0xFF)
	{
	    ++delnum;
	    continue;
	}
	outbufr[0] = 0xAA;
	outbufr[1] = inbufr[0];		// Copy the record type byte
	inpnt = inbufr + 1;
        outpnt = outbufr + 2;
	useit = TRUE;
	do
	{
	    if (inpnt[0] == 0)
		break;
	    itemsz = inpnt[1] + 2;
	    if (useit)
	    {
		if (inpnt[0] == UDF_USERID)
		{
		    useit = FALSE;
		    ++memnum;
		}
		else
		    putitem(inpnt, itemsz);
	    }
	    inpnt += itemsz;
	} while ((incnt -= itemsz) >= 2);
	if (useit)
	{
	    if ((rtn = svcIoOutBlock(newudf, outbufr, outpnt - outbufr)) < 0)
		femsg2(prgname, "Error writing output file", rtn, NULL);
	    ++copynum;
	}
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

void finish(void)

{
    long rtn;

    if ((rtn = svcIoClose(newudf, 0)) < 0)
	femsg2(prgname, "Error closing output file", rtn, NULL);
    printf("%% UDFSTRIP: %d record%s copied, %d deleted record%s discarded,\n"
	    "            %d member record%s discarded\n", copynum,
	    (copynum == 1)? "": "s", delnum, (delnum == 1)? "": "s",
	    memnum, (memnum == 1)? "": "s");
    exit(0);
}
