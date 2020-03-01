// ***************************************************
// *                                                 *
// * Program to convert XOS RUN file to DOS EXE file *
// *                                                 *
// ***************************************************

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
#include <XOSERRMSG.H>
#include <XOSSTR.H>

#define VERSION 1
#define EDIT    2

// RUN files to be converted must consist of one or more segments, each of
//   which contains one msect which has a starting offset of 0.  The EXE
//   image is constructed by concatenating all segments in the order they
//   are defined in the RUN file.  The MOD attribute of each segment is
//   used to determine allignment.  Segment addresses or selectors may not
//   be specified.

#define BFRSIZE  2048

struct exehead
{   ushort magic;		// Magic number (0x5A4D)
    ushort lastpage;		// Length of file mod 512
    ushort length;		// Length of file in 512 byte pages
    ushort relocnum;		// Number of relocation items
    ushort headsize;		// Size of header (in paragraphs)
    ushort minmem;		// Minimum memory needed (paragraphs)
    ushort maxmem;		// Maximum memory wanted (paragraphs)
    ushort spseg;		// Value for SS
    ushort spofs;		// Value for SP
    ushort checksum;		// Word checksum (not used)
    ushort pcofs;		// Value for PC
    ushort pcseg;		// Value for CS
    ushort relocofs;		// Offset in file of relocation data
    ushort overlay;		// Overlay number (must be 0)
} exehead;

struct seghead
{   struct runseg sh;
    ulong  needed;		// Space needed for segment
    ulong  offset;		// Memory offset
    struct runmsect mshead;	// Msect header
};

struct runhead runhead;

struct symhead
{   ulong magic;		// Magic number 0x030222D4 or 0x040222D4
    long  length;		// Length of file
    long  count;		// Number of symbols
} symhead;

struct sym16
{   uchar  flag;
    ushort offset;
    ushort selector;
    char   name[34];
} sym16;

struct sym32
{   uchar  flag;
    ulong  offset;
    ushort selector;
    char   name[34];
} sym32;

int   dsz;
long  delta;
long  apply;
long  xapply;
long  frpos = 0x1E;
long  fdpos;
long  sympos;
long  symcnt;
long  foffset;
long  moffset;
long  exesize;
long  exelblk;
long  rfoffset;
ulong runpos = 0xFFFF000L;
long  runcnt;
int   headsize;
long  maxsize;
long  totalsize;
long  maxreloc;
long  totalreloc;
long  needed;
long  hdrsize;
uchar segnum;

long  exedev;			// Device handle for output (EXE) file
long  rundev;			// Device handle for input (RUN) file
char *spnt;
FILE *symfile;
char *exespec;			// File specification for output (EXE) file
char *exeext;
char *runspec;			// File specification for input (RUN) file
char *runext;
long  exepos;
char *runbfr;			// RUN file buffer
struct seghead *segtbl;
struct seghead *segpnt;
char  *databfr;
short *relpnt;
short *relbfr;

char  prgname[] = "RUN2EXE";	// Name of this program
char  data;
char  type;

void  badsym(char *msg);
void  runget(char *bfr, ulong pos, int size);
long  varget(void);
void  warning(void);

main(argc, argv)
int   argc;
char *argv[];

{
    char *dpnt;
    long  rtn;
    int   cnt;
    int   cnt2;
    int   zero;
    char  chr;

    if (argc != 2 && argc != 3)
    {
        fputs("? RUN2EXE: Command error, correct usage is:\n"
              "             RUN2EXE filename\n"
              "           where \"filename\".RUN is the input file and "
              "\"filename\".EXE is\n"
              "           the output file or:\n"
              "             RUN2EXE runname exename\n"
              "           where \"exename\".RUN is the input file and "
              "\"runname\".EXE is\n"
              "           the output file\n", stderr);
        exit(1);
    }
    fprintf(stderr, "RUN2EXE - version %d.%d\n", VERSION, EDIT);
    strupper(argv[1]);
    if (argc == 3)
        strupper(argv[2]);
    else
        argv[2] = argv[1];
    runspec = getspace(strlen(argv[1]) + 6);
    strmov((runext=strmov(runspec, argv[1])), ".RUN");
    exespec = getspace(strlen(argv[2]) + 6);
    strmov((exeext=strmov(exespec, argv[2])), ".EXE");
    runbfr = getspace(BFRSIZE);		// Allocate RUN file buffer
    if ((rundev = svcIoOpen(O_IN, runspec, NULL)) < 0)
        femsg2(prgname, "Error opening RUN file", rundev, runspec);
    if ((exedev = svcIoOpen(O_IN|O_OUT|O_CREATE|O_TRUNCA, exespec, 0)) < 0)
        femsg2(prgname, "Error creating EXE file", exedev, exespec);

    runget((char *)(&runhead), 0, sizeof(runhead)); // Read the RUN file header
    if (runhead.magic != 0x22D7 || runhead.version != 2 ||
            runhead.hdrsize < (sizeof(runhead)-6)) // Make sure valid RUN file
    {
        fputs("? RUN2EXE: Input file is not a valid RUN file\n", stderr);
        exit(1);
    }					// Allocate space for relocation data
    if (runhead.numsegs == 0)
    {
        fputs("? RUN2EXE: Input file does not contain any segments\n",
                stderr);
        exit(1);
    }
    if (runhead.startmsect == 0)
    {
        fputs("? RUN2EXE: No PC value given in RUN file\n", stderr);
        exit(1);
    }
    if (runhead.startmsect > runhead.numsegs)
    {
        fputs("? RUN2EXE: Illegal msect number for PC value in RUN file\n",
                stderr);
        exit(1);
    }
    if (runhead.stkmsect == 0)
    {
        fputs("? RUN2EXE: No SP value given in RUN file\n", stderr);
        exit(1);
    }
    if (runhead.stkmsect > runhead.numsegs)
    {
        fputs("? RUN2EXE: Illegal msect number for SP value in RUN file\n",
                stderr);
        exit(1);
    }
    cnt = runhead.numsegs;
    segpnt = segtbl = (struct seghead *)getspace(runhead.numsegs *
            sizeof(struct seghead));
    foffset = runhead.hdrsize + 6;
    do
    {   runget((char *)(segpnt), foffset, 12); // Read a segment header
        if (segpnt->sh.hdrsize < 12)	// Make sure its big enough
        {
            fputs("? RUN2EXE: Input file segment header length too short\n",
                    stderr);
            exit(1);
        }
        if (segpnt->sh.nummsc != 1)
        {
            fputs("? RUN2EXE: Segment does not contain single msect\n",
                    stderr);
            exit(1);
        }
        if (segpnt->sh.select != 0 || ((segpnt->sh.status & 0x01) &&
                segpnt->sh.addr != 0xFFFFFFFFL))
        {
            fputs("? RUN2EXE: Segment selector or address specified\n",
                    stderr);
            exit(1);
        }
        foffset += segpnt->sh.hdrsize;
        segpnt->offset = moffset;
        runget((char *)(&segpnt->mshead), foffset, 34);
					// Read the msect header
        if (segpnt->mshead.hdrsize < 34) // Make sure its big enough
        {
            fputs("? RUN2EXE: Msect header length too small\n", stderr);
            exit(1);
        }
        if (!(segpnt->mshead.status & 0x01) || segpnt->mshead.addr != 0)
        {
            fputs("? RUN2EXE: Msect not absolute or offset not zero\n", stderr);
            exit(1);
        }
        segpnt->needed = needed = (((cnt == 1)? segpnt->mshead.datasz:
                segpnt->mshead.alloc) + 0xF) & ~0xFL;
        if (maxsize < needed)
            maxsize = needed;
        totalsize += needed;
        if (maxreloc < segpnt->mshead.relsz)
            maxreloc = segpnt->mshead.relsz;
        totalreloc += segpnt->mshead.relsz;
        foffset += segpnt->mshead.hdrsize;
        moffset += needed;
        segpnt++;
    } while (--cnt > 0);
    segpnt--;				// Point to last segment header
    databfr = getspace(maxsize);	// Allocate segment data buffer
    if (maxreloc != 0)			// Allocate relocation buffer
        relbfr = (short *)getspace(maxreloc * 4);
    hdrsize = ((0x1E + totalreloc * 4 + 511) & ~0x1FFL);
    exesize = totalsize + hdrsize;	// Set up the EXE file headaer
    exehead.magic = 0x5A4D;
    exehead.lastpage = exesize % 512;
    exehead.length = (exesize + 511)/512;
    exehead.relocnum = totalreloc;
    exehead.headsize = hdrsize >> 4;
    exehead.minmem = (segpnt->mshead.alloc - segpnt->mshead.datasz + 0xFL) >> 4;
    exehead.maxmem = 0xFFFF;
    exehead.spofs = runhead.stack;
    exehead.spseg = (segtbl[runhead.stkmsect-1].offset) >> 4;
    exehead.checksum = 0;
    exehead.pcofs = runhead.start;
    exehead.pcseg = (segtbl[runhead.startmsect-1].offset) >> 4;
    exehead.relocofs = (totalreloc != 0)? 0x1E: 0;

    if ((rtn = svcIoOutBlock(exedev, (char *)&exehead, 0x1E)) < 0)
        femsg2(prgname, "Error writing to EXE file", rtn, exespec);

    // Now we loop for each segment in the RUN file - we read each entire
    //   segment into our segment buffer and its corresponding relocation
    //   data into our relocation buffer - we then apply relocation to the
    //   segment data and then write out both the segment data and the
    //   relocation data (after converting it to EXE format)

    segpnt = segtbl;
    fdpos = hdrsize;
    cnt = runhead.numsegs;
    do
    {   if (segpnt->mshead.datasz != 0)
        {
            if ((rtn = svcIoSetPos(rundev, segpnt->mshead.dataos, 0)) < 0 ||
                    (rtn = svcIoInBlock(rundev, databfr,
                    segpnt->mshead.datasz)) != segpnt->mshead.datasz)
            {				// Read one segment
                if (rtn >= 0)
                    rtn = EOF;
                femsg2(prgname, "Error reading from RUN file", rtn, runspec);
            }
        }
        if ((zero = (int)(segpnt->needed - segpnt->mshead.datasz)) > 0)
            memset(databfr+(int)(segpnt->mshead.datasz), 0, zero);
        relpnt = relbfr;
        if ((cnt2 = (int)(segpnt->mshead.relsz)) != 0)
        {
            rfoffset = segpnt->mshead.relos;
            apply = 0;
            do				// Read the relocation data
            {
                runget(&data, rfoffset++, 1); // Get relocation byte
                if ((data & 0xFC) != 0)	// Must be segment relocation
                {
                    fputs("? RUN2EXE: Illegal relocation type in RUN file\n",
                            stderr);
                    exit(1);
                }
                segnum = varget();	// Get segment number
                if (segnum == 0 || segnum > runhead.numsegs)
                {
                    fprintf(stderr, "? RUN2EXE: Illegal segment number %d in "
                            "RUN file\n", segnum);
                    exit(1);
                }
                delta = 0;
                dsz = (data & 0x03) + 1;
                runget((char *)(&delta), rfoffset, dsz);
                rfoffset += dsz;
                apply += delta;		// Apply relocation to the segment
                *((short *)(&databfr[(int)apply])) +=
                        (int)(segtbl[segnum-1].offset >> 4);
                xapply = apply + segpnt->offset;
                *relpnt++ = xapply & 0x0F; // Store EXE relocation item
                *relpnt++ = xapply >> 4;
            } while (--cnt2 > 0);
        }

        // We have now done relocation on the segment - now we write out the
        //   relocation table and then the segment text

        if ((cnt2 = 2 * (int)(relpnt - relbfr)) != 0)
        {				// Write out the relocation table
            if ((rtn = svcIoSetPos(exedev, frpos, 0)) < 0 ||
                    (rtn = svcIoOutBlock(exedev, (char *)relbfr, cnt2)) < 0)
                femsg2(prgname, "Error writing relocation data to EXE file",
                        rtn, exespec);
            frpos += cnt2;
        }
        if ((rtn = svcIoSetPos(exedev, fdpos, 0)) < 0 ||
                (rtn = svcIoOutBlock(exedev, databfr, segpnt->needed)) < 0)
					// Write out the segment data
            femsg2(prgname, "Error writing segment data to EXE file", rtn,
                    exespec);
        fdpos += segpnt->needed;
        segpnt++;
    } while (--cnt > 0);

    svcIoClose(rundev, 0);		// Close input file
    if ((rtn = svcIoClose(exedev, 0)) < 0) // Close output file
        femsg2(prgname, "Error closing EXE file", rtn, exespec);
    printf("RUN file converted; %d segment%s (%ld byte%s),"
            " %ld relocation item%s\n",
            runhead.numsegs, (runhead.numsegs == 1)? "":"s",
            totalsize, (totalsize == 1)? "":"s",
            totalreloc, (totalreloc == 1)? "":"s");

    // Now we must convert the symbol table file if we have one

    strmov(runext, ".SYM");
    if ((rundev = svcIoOpen(O_IN, runspec, NULL)) < 0)
    {
        if (rundev != ER_FILNF)
            femsg2(prgname, "Error opening input SYM file", rundev, runspec);
        fputs("No SYM file found to convert\n", stdout);
        return (0);
    }
    strmov(exeext, ".SYM");

    if ((symfile = fopen(exespec, "wb")) == NULL)
        femsg2(prgname, "Error creating output SYM file", -errno, exespec);
    runpos = 0xFFFF0000L;
    runget((char *)(&symhead), 0, 12); // Read the SYM file header
    if (symhead.magic != 0x040222D4L)
        badsym("Magic number is incorrect");
    if ((symhead.length -= 2*symhead.count) <= 0)
        badsym("Length value in header is invalid");
    symhead.magic = 0x030222D4L;
    fwrite((char *)(&symhead), 1, 12, symfile); // Output SYM file header

    sympos = 12;
    symcnt = symhead.count;
    while (--symcnt >= 0)
    {
        runget((char *)(&sym32), sympos, 8);
        sympos += 8;
        spnt = sym32.name;
        cnt = 32;
        while (*spnt++ >= 0)
        {
            if (--cnt < 0)
                badsym("Symbol name too long");
            runget(spnt, sympos++, 1);
        }
        sym16.flag = sym32.flag & 0x3E;
        sym16.offset = (ushort)(sym32.offset);
        if (sym32.flag & 0x01)
        {
            if (sym32.selector > runhead.numsegs)
            {
                badsym("Illegal msect number for symbol");
                sym16.selector = 0;
            }
            else
                sym16.selector = (segtbl[sym32.selector-1].offset) >> 4;
        }
        else
            sym16.selector = sym32.selector;
        spnt = sym32.name;
        dpnt = sym16.name;
        do
        {
            *dpnt++ = (chr = *spnt++);
        } while (chr >= 0);

        if (sym32.flag & 0x20)
        {
            if (sym32.offset > 0xFFFFL)
                warning();
        }
        else
        {
            if (sym32.offset != 0x8000L)
            {
                rtn = sym32.offset & 0xFFFF8000L;
                if (rtn != 0 && rtn != 0xFFFF8000L)
                    warning();
            }
        }
        fwrite(&sym16, 1, (int)(dpnt - sym16.name) + 5, symfile);
    }
    svcIoClose(rundev, 0);		// Close input file
    if (fclose(symfile) < 0)		// Close output file
        femsg2(prgname, "Error closing output SYM file", -errno, exespec);
    printf("SYM file converted; %ld symbol%s using %ld bytes\n",
            symhead.count, (symhead.count == 1)? "":"s", symhead.length);
    return (0);
}

void warning(void)

{
    spnt[-1] &= 0x7F;
    spnt[0] = 0;
    printf("Warning: value of symbol %s (%08.8lX) truncated\n",
            sym32.name, sym32.offset);
}

//*************************************************
// Function: runget - Read from the RUN or SYM file
// Returned: Nothing
//*************************************************

void runget(
    char *buffer,
    ulong pos,
    int   count)

{
    char *pnt;
    long  rtn;

    if (pos < runpos || (pos + count) > (runpos + BFRSIZE))
    {
        if ((rtn = svcIoSetPos(rundev, pos, 0)) < 0 ||
                (rtn = svcIoInBlock(rundev, runbfr, BFRSIZE)) < 0)
            femsg2(prgname, "Error reading from RUN file", rtn, runspec);
        runpos = pos;
        runcnt = rtn;
    }
    pnt = runbfr + (int)(pos - runpos);
    do
    {   *buffer++ = *pnt++;
    } while (--count > 0);
}

//**************************************************************
// Function: varget - Get variable length item from the RUN file
// Returned: Value obtained
//**************************************************************

long varget(void)

{
    long value;

    value = 0;
    runget((char *)&value, rfoffset++, 1);
    if (value & 0x80)
    {
        value <<= 8;
        runget((char *)&value, rfoffset++, 1);
        value &= 0x7FFF;
        if (value & 0x4000)
        {
            value <<= 8;
            runget((char *)&value, rfoffset++, 1);
            value &= 0x3FFFFF;
            if (value & 0x200000)
            {
                value <<= 8;
                runget((char *)&value, rfoffset++, 1);
                value &= 0x1FFFFFFF;
            }
        }
    }
    return (value);
}


void badsym(
    char *msg)

{
    fprintf(stderr, "SYM file not processed because of illegal format:\n"
                    "    %s\n", msg);
    fclose(symfile);
    svcIoDelete(0, exespec, NULL);
    exit(0);
}
