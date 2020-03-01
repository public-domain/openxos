/***********************************/
/*                                 */
/* Program to convert DOS EXE file */
/*   to XOS type 3 RUN file        */
/*                                 */
/***********************************/

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>

#define VERSION 2
#define EDIT    1

struct exehead
{   ushort magic;	/* Magic number (0x5A4D) */
    ushort lastpage;	/* Length of file mod 512 */
    ushort length;	/* Length of file in 512 byte pages */
    ushort relocnum;	/* Number of relocation items */
    ushort headsize;	/* Size of header (in paragraphs) */
    ushort minmem;	/* Minimum memory needed (paragraphs) */
    ushort maxmem;	/* Maximum memory wanted (paragraphs) */
    ushort spseg;	/* Value for SS */
    ushort spofs;	/* Value for SP */
    ushort checksum;	/* Word checksum (not used) */
    ushort pcofs;	/* Value for PC */
    ushort pcseg;	/* Value for CS */
    ushort relocofs;	/* Offset in file of relocation data */
    ushort overlay;	/* Overlay number (must be 0) */
} exehead;

struct runhead
{   ushort magic;	/* Magic number (0x22D7) */
    uchar  version;	/* Version number (1) */
    uchar  proctype;	/* Processor type (2) */
    uchar  imagetype;	/* Image type (3) */
    uchar  notused0;	/* Not used */
    uchar  headsize;	/* Header size (20) */
    uchar  numadsp;	/* Number of address spaces (1) */
    ushort pcofs;	/* Value for PC */
    ushort pcseg;	/* Value for CS */
    uchar  pcmsect;	/* Msect for PC value (1) */
    uchar  notused1;	/* Not used */
    ushort debugofs;	/* Debugger address offset (0) */
    ushort debugseg;	/* Debugger address segment (0) */
    uchar  debugmsect;	/* Debugger address msect (0) */
    uchar  notused2;	/* Not used */
    ushort spofs;	/* Value for SP */
    ushort spseg;	/* Value for SS */
    uchar  spmsect;	/* Msect for SP value (1) */
    uchar  notused3;	/* Not used */
    uchar  asheadsize;	/* Size of address space header (12) */
    uchar  asnummsect;	/* Number of msects (1) */
    uchar  asstatus;	/* Address space status byte (0x19) */
    uchar  aslinked;	/* Number of linked address space (0) */
    uchar  astype;	/* Address space type (5) */
    uchar  aspriv;	/* Privledge level (0) */
    ushort asselect;	/* Selector value (0xFFFF) */
    ulong  asaddress;	/* Address (0xFFFFFFFF) */
    uchar  msheadsize;	/* Size of msect header (34) */
    uchar  notused4;	/* Not used */
    uchar  msstatus;	/* Msect status byte (0x19) */
    uchar  notused5;	/* Not used */
    uchar  mstype;	/* Msect type (5) */
    uchar  mspriv;	/* Privledge level (0) */
    ulong  msoffset;	/* Offset (0x8000) */
    ulong  msavail;	/* Space to leave available (0xA00000)*/
    ulong  msalloc;	/* Space to allocate (bytes) */
    ulong  msdataofs;	/* Offset in file of data to load */
    ulong  msload;	/* Bytes to load */
    ulong  msrelocofs;	/* Offset in file of relocation info */
    ulong  msrelocnum;	/* Number of relocation items */
} runhead =
{   0x22D7, 1, 2, 3, 0, 20, 1,	/* File header data */
    0, 0, 1, 0,			/* Start address */
    0, 0, 0, 0,			/* Debug address */
    0, 0, 1, 0,			/* Stack address */
    12, 1, 0x19, 0, 5, 0,	/* Address space header data */
    0, 0xFFFFFFFFL,
    34, 0, 0x19, 0, 5, 0,	/* Msect header data */
    0x8000, 0xA00000L, 0, 72, 0, 0, 0
};

long  exedev;			/* Device handle for input (EXE) file */
long  rundev;			/* Device handle for output (RUN) file */
char *relocbfr;			/* Buffer for relocation data */
char *databfr;			/* Data buffer */
union
{   uchar  *c;
    ushort *s;
    ulong  *l;
} datapnt;
long  relcnt;			/* Byte count used when outputting */
				/*   relocation items */
char *relpnt;			/* Pointer used when outputting relocation */
				/*   items */
char *exespec;			/* File specification for input (EXE) file */
char *runspec;			/* File specification for output (RUN) file */
char  prgname[] = "EXE2RUN";	/* Name of this program */

void  exeread(char *buffer, long size);
void  runwrite(char *buffer, long size);
void  relwrite(int data);

main(argc, argv)
int   argc;
char *argv[];

{
    ulong  amount;
    long   count;
    long   cnt2;
    long   lrtn;
    ulong  lastoffset = 0;
    ulong  delta;
    ulong *sortpnt;

    if (argc != 2 && argc != 3)
    {
        fputs("? EXE2RUN: Command error, correct usage is:\n"
              "             EXE2RUN filename\n"
              "           where \"filename\".EXE is the input file and "
              "\"filename\".RUN is\n"
              "           the output file or:\n"
              "             EXE2RUN exename runname\n"
              "           where \"exename\".EXE is the input file and "
              "\"runname\".RUN is\n"
              "           the output file\n", stderr);
        exit(1);
    }
    fprintf(stderr, "EXE2RUN - version %d.%d\n", VERSION, EDIT);
    strupper(argv[1]);
    if (argc == 3)
        strupper(argv[2]);
    else
        argv[2] = argv[1];
    exespec = getspace(strlen(argv[1]) + 6);
    runspec = getspace(strlen(argv[2]) + 6);
    strmov(strmov(exespec, argv[1]), ".EXE");
    strmov(strmov(runspec, argv[2]), ".RUN");
    if ((exedev = svcIoOpen(O_IN, exespec, 0)) < 0)
        femsg(prgname, exedev, exespec);
    if ((rundev = svcIoOpen(O_CREATE|O_TRUNCA|O_OUT|O_IN, runspec, 0)) < 0)
        femsg(prgname, rundev, runspec);
    exeread((char *)(&exehead), 0x1B);	/* Read the EXE file header */
    if (exehead.magic != 0x5A4D || exehead.overlay != 0)
    {					/* Make sure valid EXE file */
        fputs("? EXE2RUN: Input file is not a valid EXE file\n", stderr);
        exit(1);
    }					/* Allocate space for relocation data */
    relocbfr = getspace(exehead.relocnum * 4);
    databfr = getspace(32 * 1024L);	/* And for a data buffer */
    if ((lrtn=svcIoSetPos(exedev, exehead.relocofs, 0)) < 0)
					/* Read relocation data */
        femsg(prgname, lrtn, exespec);	/* If error */
    exeread(relocbfr, exehead.relocnum * 4);
    runhead.pcofs = exehead.pcofs;	/* Setup the RUN file header */
    runhead.pcseg = exehead.pcseg;
    runhead.spofs = exehead.spofs;
    runhead.spseg = exehead.spseg;
    runhead.msload = (ulong)(exehead.length) * 512L
            - (ulong)(exehead.headsize) * 16L;
    if (exehead.lastpage != 0)
        runhead.msload += (ulong)(exehead.lastpage) - 512L;
    runhead.msalloc = (ulong)(runhead.msload) + (ulong)(exehead.minmem) * 16L;
    runhead.msrelocnum = exehead.relocnum;
    runhead.msrelocofs = (ulong)(runhead.msload) + 72L;
    runwrite((char *)(&runhead), 72);	/* Write out the RUN file header */
    if ((lrtn=svcIoSetPos(exedev, exehead.headsize*16, 0)) < 0)
					/* Position to read data */
        femsg(prgname, lrtn, exespec);	/* If error */
    while (runhead.msload != 0)		/* Copy data area to RUN file */
    {
        amount = (runhead.msload > 32*1024L)? 32*1024L: runhead.msload;
        runhead.msload -= amount;
        exeread(databfr, amount);	/* Read from EXE file */
        runwrite(databfr, amount);	/* Write to RUN file */
    }
    relpnt = databfr;			/* Initialize relocation buffer */
    relcnt = 32*1024L;
    if ((count = exehead.relocnum) != 0)
    {
        datapnt.c = (uchar *)relocbfr;	/* Convert relocation values to */
        while (--count >= 0)		/*   offsets */
            *datapnt.l++ = (ulong)(datapnt.s[0]) + (ulong)(datapnt.s[1]) * 16L;
        count = exehead.relocnum;	/* Sort the relocation offsets into */
        datapnt.c = (uchar *)relocbfr;	/*   assending order */
        while (--count >= 0)
        {
            cnt2 = count;
            sortpnt = ++(datapnt.l);
            while (--cnt2 >= 0)
            {
                if (*sortpnt < *datapnt.l)
                {
                    amount = *sortpnt;
                    *sortpnt = *datapnt.l;
                    *datapnt.l = amount;
                }
                ++sortpnt;
            }
        }
        count = exehead.relocnum;	/* Now output the relocation items */
        datapnt.c = (uchar *)relocbfr;	/*   for the RUN file */
        while (--count >= 0)
        {
            delta = *datapnt.l - lastoffset;
            lastoffset = *datapnt.l++;
            if (delta <= 0xFFL)
            {
                relwrite(0x04);
                relwrite((int)delta);
            }
            else if (delta <= 0xFFFFL)
            {
                relwrite(0x05);
                relwrite((int)delta);
                relwrite((int)(delta >> 8));
            }
            else if (delta <= 0xFFFFFFL)
            {
                relwrite(0x06);
                relwrite((int)delta);
                relwrite((int)(delta >> 8));
                relwrite((int)(delta >> 16));
            }
            else
            {
                relwrite(0x07);
                relwrite((int)delta);
                relwrite((int)(delta >> 8));
                relwrite((int)(delta >> 16));
                relwrite((int)(delta >> 24));
            }
        }
        runwrite(databfr, 32*1024L - relcnt); /* Write out final buffer */
    }
    svcIoClose(exedev, 0);		/* Close input file */
    if ((lrtn=svcIoClose(rundev, 0)) < 0) /* Close output file */
        femsg(prgname, lrtn, runspec);
    return (0);
}

/*
 * Function to read from the EXE file
 */

void exeread(
    char *buffer,
    long  count)

{
    long lrtn;

    if ((lrtn = svcIoInBlock(exedev, buffer, count)) < 0)
    {
        if (lrtn < 0)			/* Error reported? */
            femsg(prgname, lrtn, exespec);
        else				/* No - report as unexpected EOF */
        {
            fprintf(stderr, "? EXE2RUN: Unexpected end-of-file reading %s\n",
                    exespec);
            exit(1);
        }
    }
}

/*
 * Function to write to the RUN file
 */

void runwrite(
    char *buffer,
    long  count)

{
    long lrtn;

    if ((lrtn = svcIoOutBlock(rundev, buffer, count)) != count)
    {
        if (lrtn >= 0)
            lrtn = ER_ICMIO;
        femsg(prgname, lrtn, runspec);
    }
}

/*
 * Function to write byte of relocation data to the RUN file
 */

void relwrite(
    int data)

{
    if (--relcnt < 0)			/* Is relocation buffer full? */
    {
        runwrite(databfr, 32*1024L);	/* Yes - write it out */
        relpnt = databfr;
        relcnt = 32*1024L-1;
    }
    *relpnt++ = data;
}
