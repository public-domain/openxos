//===============================================================
// Program: UDFPWC.C - Converts from clear to encrypted passwords
//===============================================================

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
#include <XOSENCPSWD.H>
#include <XOSUDF.H>
#include <XOSCRYPT.H>

long   newudf;
long   oldudf;

long   offset;

uchar  blkbufr[1024];
uchar  inbufr[2048];
uchar  outbufr[2048];
uchar *blkpnt;
uchar *inpnt;
uchar *outpnt;
uchar *userpnt;

int    blkcnt;
int    incnt;
int    size;
int    itemsz;
int    copynum;
int    delnum;
int    pswdnum;

uchar  header[2] = {0x81, 0xC6};
char   prgname[] = "UDFPWC";
uchar  pswdbufr[42] = {UDF_PASSWORD, 40};

extern long errno;


void main(
   int   argc,
   char *argv[])

{
    long rtn;
    int  data;
    char save;

    if (argc != 3)
    {
       fputs("? UDFPWC: Command error, usage is:\n"
             "            UDFPWC oldfile newfile\n", stderr);
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
        fprintf(stderr, "? UDFPWC: Input file header is incorrect\n");
        exit(1);
    }
    if ((rtn = svcIoOutBlock(newudf, header, 2)) < 0)
	femsg2(prgname, "Error writing output file", rtn, NULL);

    offset = 2;

    if ((data = getbyte()) < 0)
    {
	fputs("? UDFPWC: No data in input file\n", stderr);
	exit(1);
    }
    if (data != 0xAA)
    {
	fputs("? UDFPWC: Initial byte is not beginning of record\n", stderr);
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
		    data = 0xBB;
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
	userpnt = "\x00\x04????";
	do
	{
	    if (inpnt[0] == 0)
		break;
	    itemsz = inpnt[1] + 2;
	    if (inpnt[0] != UDF_PASSWORD) // Is this a password item?
	    {
		if (inpnt[0] == UDF_USERNAME)
		    userpnt = inpnt;
		putitem(inpnt, itemsz);	// No - just copy it
	    }
	    else
	    {
		++pswdnum;
		save = inpnt[itemsz];
		inpnt[itemsz] = 0;

		printf("Password: %s\n", inpnt + 2);

		if (!encodepassword(inpnt + 2, pswdbufr + 2))
		{
		    printf("%% Weak key generated for user %.*s\n", userpnt[1],
			    userpnt + 2);
		}

		printf("Salt: %02.2X %02.2X %02.2X %02.2X %02.2X %02.2X "
			"%02.2X %02.2X\n", pswdbufr[2], pswdbufr[3],
			pswdbufr[4], pswdbufr[5], pswdbufr[6], pswdbufr[7],
			pswdbufr[8], pswdbufr[9]);
		printf("Code: %02.2X %02.2X %02.2X %02.2X %02.2X %02.2X "
			"%02.2X %02.2X\n", pswdbufr[10], pswdbufr[11],
			pswdbufr[12], pswdbufr[13], pswdbufr[14], pswdbufr[15],
			pswdbufr[16], pswdbufr[17]);
		printf("      %02.2X %02.2X %02.2X %02.2X %02.2X %02.2X "
			"%02.2X %02.2X\n", pswdbufr[18], pswdbufr[19],
			pswdbufr[20], pswdbufr[21], pswdbufr[22], pswdbufr[23],
			pswdbufr[24], pswdbufr[25]);
		printf("      %02.2X %02.2X %02.2X %02.2X %02.2X %02.2X "
			"%02.2X %02.2X\n", pswdbufr[26], pswdbufr[27],
			pswdbufr[28], pswdbufr[29], pswdbufr[30], pswdbufr[31],
			pswdbufr[32], pswdbufr[33]);
		printf("      %02.2X %02.2X %02.2X %02.2X %02.2X %02.2X "
			"%02.2X %02.2X\n", pswdbufr[34], pswdbufr[35],
			pswdbufr[36], pswdbufr[37], pswdbufr[38], pswdbufr[39],
			pswdbufr[40], pswdbufr[41]);

		inpnt[itemsz] = save;
		putitem(pswdbufr, 42);
	    }
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

void finish(void)

{
    long rtn;

    if ((rtn = svcIoClose(newudf, 0)) < 0)
	femsg2(prgname, "Error closing output file", rtn, NULL);
    printf("%% UDFPWC: %d record%s copied, %d deleted record%s discarded,\n"
	    "          %d password%s converted\n", copynum,
	    (copynum == 1)? "": "s", delnum, (delnum == 1)? "": "s",
	    pswdnum, (pswdnum == 1)? "": "s");
    exit(0);
}
