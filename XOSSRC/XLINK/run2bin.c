// ***********************************************************
// *                                                         *
// * Program to convert XOS RUN file to a simple binary file *
// *                                                         *
// ***********************************************************

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
#include <ERRNO.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSRUN.H>
#include <XOSERMSG.H>
#include <XOSSTR.H>

#define VERSION 1
#define EDIT    0

// RUN files to be converted must consist of exactly one segment which must
//   have a specified selector value.  This segment must contain exactly one
//   msect which must have a specified starting offset.  A starting address
//   and/or a stack address does not need to be specified.  If it is specified,
//   it is ignored.  References to imported symbols are not allowed.  This
//   implies that there can be no fixup items in the RUN file.  Any symbol
//   definitions in the RUN file are ignored.

// The BIN file generated starts at the beginning of the msect and includes
//   all bytes in the msect.  The length of the BIN file is always equal to
//   the length of the msect.

#define BFRSIZE 2048

struct runhead  runhead;
struct runseg   runseg;
struct runmsect runmsect;

long  needed;
long  left;
long  amount;
long  bindev;			// Device handle for output (BIN) file
long  rundev;			// Device handle for input (RUN) file
char *binspec;			// File specification for output (BIN) file
char *binext;
char *runspec;			// File specification for input (RUN) file
char *runext;
char *databfr;
char  prgname[] = "RUN2BIN";	// Name of this program


void runget(char *buffer, ulong pos, int count);


main(
    int   argc,
    char *argv[])

{
    long  rtn;

    if (argc != 2 && argc != 3)
    {
        fputs("? RUN2BIN: Command error, correct usage is:\n"
              "             RUN2BIN filename\n"
              "           where \"filename\".RUN is the input file and "
              "\"filename\".BIN is\n"
              "           the output file or:\n"
              "             RUN2BIN runname binname\n"
              "           where \"runname\".RUN is the input file and "
              "\"binname\".BIN is\n"
              "           the output file\n", stderr);
        exit(1);
    }
    fprintf(stderr, "RUN2BIN - version %d.%d\n", VERSION, EDIT);
    strupper(argv[1]);
    if (argc == 3)
        strupper(argv[2]);
    else
        argv[2] = argv[1];
    runspec = getspace(strlen(argv[1]) + 6);
    strmov((runext = strmov(runspec, argv[1])), ".RUN");
    binspec = getspace(strlen(argv[2]) + 6);
    strmov((binext = strmov(binspec, argv[2])), ".BIN");
    databfr = getspace(BFRSIZE);	// Allocate the data buffer
    if ((rundev = svcIoOpen(O_IN, runspec, NULL)) < 0)
        femsg2(prgname, "Error opening RUN file", rundev, runspec);
    if ((bindev = svcIoOpen(O_IN|O_OUT|O_CREATE|O_TRUNCA, binspec, 0)) < 0)
        femsg2(prgname, "Error creating BIN file", bindev, binspec);
    runget((char *)(&runhead), 0, sizeof(runhead)); // Read the RUN file header
    if (runhead.magic != 0x22D7 || runhead.version != 2 ||
            runhead.hdrsize < (sizeof(runhead)-6)) // Make sure valid RUN file
    {
        fputs("? RUN2BIN: Input file is not a valid RUN file\n", stderr);
        exit(1);
    }
    if (runhead.numsegs != 1)
    {
        fputs("? RUN2BIN: Input file does not contain a single segment\n",
                stderr);
        exit(1);
    }
    runget((char *)(&runseg), runhead.hdrsize + 6, sizeof(runseg));
					// Read the segment header
    if (runseg.hdrsize < 12)	// Make sure its big enough
    {
	fputs("? RUN2BIN: Input file segment header length too short\n",
	    stderr);
	exit(1);
    }
    if (runseg.nummsc != 1)
    {
	fputs("? RUN2BIN: Segment does not contain a single msect\n",
		stderr);
	exit(1);
    }
    if (runseg.select == 0)
    {
	fputs("? RUN2BIN: Segment selector not specified\n",
		stderr);
	exit(1);
    }
    runget((char *)(&runmsect), runhead.hdrsize + runseg.hdrsize + 6,
	    sizeof(runmsect));					// Read the msect header
    if (runmsect.hdrsize < 34)		// Make sure its big enough
    {
	fputs("? RUN2BIN: Msect header length too small\n", stderr);
	exit(1);
    }
    if (!(runmsect.status & MS_ABS))
    {
	fputs("? RUN2BIN: Msect not absolute\n", stderr);
	exit(1);
    }
    if (runmsect.relsz != 0)
    {
	fputs("? RUN2BIN: Msect requires fixup\n", stderr);
	exit(1);
    }
    if (runmsect.alloc == 0)
    {
	fputs("? RUN2BIN: Msect contains no data\n", stderr);
	exit(1);
    }

    // Now we simply copy the msect contents to the output file

    if (runmsect.datasz != 0)
    {
	if ((rtn = svcIoSetPos(rundev, runmsect.dataos, 0)) < 0)
	    femsg2(prgname, "Error reading from RUN file", rtn, runspec);
	left = runmsect.datasz;
	do
	{
	    if ((amount = left) > BFRSIZE)
		amount = BFRSIZE;
	    if ((rtn = svcIoInBlock(rundev, databfr, amount)) != amount)
	    {				// Read some data
		if (rtn >= 0)
		    rtn = ER_EOF;
		femsg2(prgname, "Error reading from RUN file", rtn, runspec);
	    }
	    if ((rtn = svcIoOutBlock(bindev, databfr, amount)) != amount)
	    {
		if (rtn >= 0)
		    rtn = ER_INCMO;
		femsg2(prgname, "Error writing to BIN file", rtn, binspec);
	    }
	} while ((left -= amount) > 0);
    }
    svcIoClose(rundev, 0);		// Close the input file

    if (runmsect.alloc > runmsect.datasz)
    {
	amount = runmsect.alloc - runmsect.addr;
	if ((rtn = svcIoSetPos(bindev, amount - 1, 0)) < 0 ||
		(rtn = svcIoOutBlock(bindev, "", 1)) != 1)
	{
	    if (rtn >= 0)
		rtn = ER_INCMO;
	    femsg2(prgname, "Error writing to BIN file", rtn, binspec);
	}
    }

    if ((rtn = svcIoClose(bindev, 0)) < 0) // Close output file
        femsg2(prgname, "Error closing BIN file", rtn, binspec);

    printf("RUN file converted:\n    Offset = 0x%X\n    Size   = 0x%X (%d)\n",
	    runmsect.addr, runmsect.alloc, runmsect.alloc);
    return (0);
}

//******************************************
// Function: runget - Read from the RUN file
// Returned: Nothing
//******************************************

void runget(
    char *buffer,
    ulong pos,
    int   count)

{
    long rtn;

    if ((rtn = svcIoSetPos(rundev, pos, 0)) < 0 ||
	    (rtn = svcIoInBlock(rundev, buffer, count)) != count)
    {
	if (rtn >= 0)
	    rtn = ER_EOF;
	femsg2(prgname, "Error reading from RUN file", rtn, runspec);
    }
}
