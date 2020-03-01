/*******************************/
/*                             */
/* Program to produce formated */
/*  dump of XOS RUN files      */
/*                             */
/*******************************/

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <ERRNO.H>
#include <XOS.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>

#define VERSION 2
#define EDIT    1

FILE    *fp;
char     filespec[100];
char     symname[40];
char     prgname[] = "RUNDMP";

int   hdrcnt;
int   segcnt;
int   segnum;
int   msectcnt;
int   msectnum;
int   filever;
int   minsize;
long  importoffset;
long  importnum;
long  exportoffset;
long  exportnum;
long  fileid1;
long  fileid2;
long  fileid3;

ulong addr;
char *tname[] =
{   "not specified",
    "Data",
    "Code",
    "Stack",
    "Combined"
};

struct ml
{   ulong ml_offset;
    ulong ml_size;
    ulong ml_relocos;
    ulong ml_relocsz;
};

struct ml *msectpnt;
struct ml  msectlist[256];

char *typename[] =
{   "Selector",
    "16-bit abs",
    "32-bit abs",
    "16-bit rel",
    "32-bit rel",
    "??? (5)",
    "??? (6)",
    "??? (7)"
};

char *typetbl[] =
{   "Selector",
    "??? (1)",
    "??? (2)",
    "??? (3)",
    " 8-bit abs",
    " 8-bit rel",
    "16-bit abs",
    "16-bit rel",
    "32-bit abs",
    "32-bit rel",
    "16-bit addr",
    "32-bit addr",
    "16-bit addr",
    "??? (D)"
    "32-bit addr",
    "??? (F)"
};
char *kindtbl[] =
{   "Segment",
    "Msect",
    "Symbol",
    "None"
};

void  dumpmsect(ulong position, ulong size);
void  dumpreloc(ulong position, long  size);
int   getbyte(void);
long  getitem(int byte);
ulong getlong(void);
void  getname(void);
long  getvarval(void);
int   getword(void);
void  notused1(char *leader);
void  notused2(char *leader);
void  notused3(char *leader);
void  notused4(char *leader);

main(
    int   argc,
    char *argv[])

{
    char *pnt;
    char *prefix;
    int   byte1;
    int   byte2;
    int   status;
    int   type;
    char  buf[80];
    char  chr;

    fprintf(stderr, "RUNDMP - version %d.%d\n", VERSION, EDIT);
    if (argc != 2)
    {
        fputs("? RUNDMP: Command error, correct usage is:\n"
              "              RUNDMP filename\n", stderr);
        exit(1);
    }
    pnt = argv[1];
    if (strlen(pnt) > (sizeof(filespec) - 6))
    {
        fputs("? RUNDMP: File specification is too long\n", stderr);
        exit(1);
    }
    prefix = filespec;
    type = FALSE;
    while ((chr = *pnt++) != 0)
    {
        if (chr == '.')
            type = TRUE;
        *prefix++ = chr;
    }
    if (!type)
        strmov(prefix, ".RUN");
    fp = fopen(filespec, "rb");
    if (fp == NULL)
        femsg(prgname, -errno, filespec);
    byte1 = getbyte();			/* Read file header */
    byte2 = getbyte();
    printf("\n**** File header\n  ID bytes: 0x%2.2X 0x%2.2X\n", byte1, byte2);
    filever = getbyte();
    printf("  Version number: %d\n", filever);
    printf("  Processor type: 0x%2.2X\n", getbyte());
    printf("  Image type: %d\n", getbyte());
    notused1("  ");
    hdrcnt = getbyte();			/* Get count for header */
    printf("  Byte count: %d\n", hdrcnt);
    if (filever != 1 && filever != 2)
    {
        puts("? RUNDMP: File version not 1 or 2, cannot continue");
        exit(1);
    }
    minsize = (filever == 1)? 20: 58;
    if (hdrcnt < minsize)
    {
        fprintf(stderr, "? RUNDMP: Header byte count (%d) less than %d, cannot "
                "continue\n", hdrcnt, minsize);
        exit(1);
    }
    segcnt = getbyte();				/* Get number of segments */
    printf("  Number of segments: %d\n", segcnt);
    addr = getlong();
    printf("  Start address: 0x%8.8lX 0x%2.2X\n", addr, getbyte());
    if (filever == 1)
        notused1("  ");
    else
        notused3("  ");
    addr = getlong();
    printf("  Debugger address: 0x%8.8lX 0x%2.2X\n", addr, getbyte());
    if (filever == 1)
        notused1("  ");
    else
        notused3("  ");
    addr = getlong();
    printf("  Stack address: 0x%8.8lX 0x%2.2X\n", addr, getbyte());
    if (filever == 1)
        notused1("  ");
    else
        notused3("  ");
    if (filever != 1)
    {
        exportoffset = getlong();
        exportnum = getlong();
        importoffset = getlong();
        importnum = getlong();
        printf(  "  Offset of exported symbol table: 0x%08.8X\n"
                 "  Size of exported symbol table: %d\n"
                 "  Offset of imported symbol table: 0x%08.8X\n"
                 "  Size of imported symbol table: %d\n", exportoffset,
                 exportnum, importoffset, importnum);
        fileid1 = getlong();
        fileid2 = getlong();
        fileid3 = getlong();
        printf("  Image file ID: 0x%08.8X 0x%08.8X 0x%08.8X\n", fileid1,
                fileid2, fileid3);
        notused4("  ");
    }
    segnum = 0;
    msectnum = 0;
    msectpnt = msectlist;
    while (--segcnt >= 0)
    {
        printf("\n**** Segment header (segment #%d)\n", ++segnum);
        hdrcnt = getbyte();			/* Get count for header */
        printf("  Byte count: %d\n", hdrcnt);
        if (hdrcnt < 12)
        {
            fprintf(stderr, "? RUNDMP: Segment header byte count (%d) less "
                    "than 12, cannot continue", hdrcnt);
            exit(1);
        }
        msectcnt = getbyte();
        printf("  Number of msects: %d\n", msectcnt);
        status = getbyte();
        prefix = "(";
        pnt = buf;
        if (status & 0x80)
        {
            pnt = strmov(strmov(pnt, prefix), "large");
            prefix = ", ";
        }
        pnt = strmov(strmov(pnt, prefix), (status & 0x40)? "32-bit": "16-bit");
        if (status & 0x20)
            pnt = strmov(pnt, ", conformable");
        if (status & 0x10)
            pnt = strmov(pnt, ", readable");
        if (status & 0x08)
            pnt = strmov(pnt, ", writable");
        printf("  Segment status: 0x%02.2X %s)\n", status, buf);
        printf("  Linked segment: %d\n", getbyte());
        type = getbyte();
        printf("  Segment type: %d (%s)\n", type,
                (type <= 4)? tname[type]: "???");
        printf("  Privilege level: %d\n", getbyte());
        printf("  Segment selector: 0x%4.4X\n", getword());
        printf("  Segment address: 0x%8.8lX\n", getlong());
        if ((hdrcnt -= 12) > 0)
        {
            printf("  %d extra header byte%s skipped\n", hdrcnt,
                    (hdrcnt == 1)? "": "s");
            do
            {
                getbyte();
            } while (--hdrcnt > 0);
        }
        while (--msectcnt >= 0)
        {
            printf("\n  **** Msect header (msect #%d)\n", ++msectnum);
            hdrcnt = getbyte();			/* Get count for header */
            printf("    Byte count: %d\n", hdrcnt);
            minsize = (filever == 1)? 34: 36;
            if (hdrcnt != minsize)
            {
                fprintf(stderr, "? RUNDMP: Msect header count not equal to %d,"
                    " cannot continue\n", minsize);
                exit(1);
            }
            notused1("    ");
            status = getbyte();
            pnt = buf;
            prefix = " (";
            if (status & 0x20)
            {
            pnt = strmov(strmov(pnt, prefix), "conformable");
            prefix = ", ";
            }
            if (status & 0x10)
            {
                pnt = strmov(strmov(pnt, prefix), "readable");
                prefix = ", ";
            }
            if (status & 0x08)
            {
                pnt = strmov(strmov(pnt, prefix), "writable");
                prefix = ", ";
            }
            if (pnt != buf)
                *pnt++ = ')';
            *pnt = '\0';
            printf("    Msect status: 0x%02.2X%s\n", status, buf);
            notused1("    ");
            type = getbyte();
            printf("    Msect type: %d (%s)\n", type,
                    (type <= 4) ? tname[type] : "???" );
            printf("    Privilege level: %d\n", getbyte());
            if (filever != 1)
                notused2("    ");
            printf("    Msect offset: 0x%8.8lX\n", getlong());
            printf("    Reserved address space: 0x%8.8lX\n", getlong());
            printf("    Number of bytes to allocate: %ld\n", getlong());
            msectpnt->ml_offset = getlong();
            printf("    Offset in file for data to load: 0x%8.8lX\n",
                    msectpnt->ml_offset);
            msectpnt->ml_size = getlong();
            printf("    Number of bytes to load: %ld\n",
                    msectpnt->ml_size);
            msectpnt->ml_relocos = getlong();
            printf("    Offset in file for relocation records: 0x%8.8lX\n",
                    msectpnt->ml_relocos);
            msectpnt->ml_relocsz = getlong();
            printf("    Number of relocation records: %ld\n",
                    msectpnt->ml_relocsz);
            ++msectpnt;
        }
    }

    if (exportoffset != 0)
    {
        printf("\n**** Exported symbol table (%d symbol%s)\n"
                "  Attributes    Offset    Ms/Sel  Name\n", exportnum,
                (exportnum == 1)? "": "s");
        if (fseek(fp, exportoffset, 0) < 0)
            femsg(prgname, -errno, filespec);
        do
        {
            byte1 = getbyte();
            switch (byte1 & 0x03)
            {
             case 0:
                type = 0;
                break;

             case 1:
                type = getbyte();
                if (byte1 & 0x04)
                    type |= 0xFFFFFF00;
                break;

             case 2:
                type = getword();
                if (byte1 & 0x04)
                    type |= 0xFFFF0000;
                break;

             case 3:
                type = getlong();
                break;
            }
            if (byte1 & 0x20)
            {
                if (byte1 & 0x80)
                    byte2 = getword();
                else
                    byte2 = getbyte();
            }
            getname();
            pnt = buf;
            pnt = strmov(pnt, (byte1 & 0x80)? "Abs": "...");
            pnt = strmov(pnt, (byte1 & 0x20)? "Addr": "....");
            strmov(pnt, (byte1 & 0x10)? "Sup": "...");

            if (byte1 & 0x20)
            {
                if (byte1 & 0x80)
                    printf("  %s  0x%08.8X  0x%04.4X  %s\n", buf, type, byte2,
                            symname);
                else
                    printf("  %s  0x%08.8X    %-4d  %s\n", buf, type, byte2,
                            symname);
            }
            else
                printf("  %s  0x%08.8X          %s\n", buf, type, symname);
        } while (--exportnum > 0);
    }
    if (importoffset != 0)
    {
        printf("\n**** Imported symbol table (%d symbol%s)\n"
                "  Number Flag Name\n", importnum,
                (importnum == 1)? "": "s");
        if (fseek(fp, importoffset, 0) < 0)
            femsg(prgname, -errno, filespec);
        byte1 = 0;
        do
        {
            byte2 = getbyte();		// Get the flag byte
            getname();			// Get the symbol name
            printf("%8d  %02.2X  %s\n", ++byte1, byte2, symname);
        } while (--importnum > 0);
    }

    msectcnt = msectnum;
    msectnum = 0;
    msectpnt = msectlist;
    while (--msectcnt >= 0)
    {
        printf("\n**** Data for msect #%d\n", ++msectnum);
        dumpmsect(msectpnt->ml_offset, msectpnt->ml_size);
        ++msectpnt;
    }
    msectcnt = msectnum;
    msectnum = 1;
    msectpnt = msectlist;
    while (--msectcnt >= 0)
    {
        if (msectpnt->ml_relocsz)
        {
            printf("\n**** Relocation records for msect #%d (%ld record%s)\n",
                    msectnum, msectpnt->ml_relocsz,
                    (msectpnt->ml_relocsz == 1)? "":"s");
            dumpreloc(msectpnt->ml_relocos, msectpnt->ml_relocsz);
        }
        ++msectpnt;
        ++msectnum;
    }
    return(0);
}

/*
 * Function to dump data for msect
 */

void dumpmsect(
    ulong position,
    ulong size)

{
    ulong  curloc;
    int    cnt1;
    int    cnt2;
    int    amnt;
    uchar *pnt;
    uchar  chr;
    uchar  data[16];

    if (fseek(fp, position, 0) < 0)
        femsg(prgname, -errno, filespec);
    curloc = 0;
    while (size)
    {
        amnt = (size > 16)? 16: size;
        size -= amnt;
        pnt = data;
        cnt1 = amnt;
        while (--cnt1 >= 0)
            *pnt++ = getbyte();
        printf("  %6.6lX> ", curloc);
        curloc += 16;
        cnt1 = 16;
        cnt2 = amnt;
        pnt = data;
        while (--cnt1 >= 0)
        {
            if (--cnt2 >= 0)
                printf(" %2.2X", *pnt++);
            else
                fputs("   ", stdout);
        }
        fputs("  ", stdout);
        cnt2 = amnt;
        pnt = data;
        while (--cnt2 >= 0)
        {
            if ((chr = (*pnt++ & 0x7F)) >= ' ')
                putchar(chr);
            else
                putchar('.');
        }
        putchar('\n');
    }
}

/*
 * Function to dump relocation records for msect
 */

void dumpreloc(
    ulong position,
    long  size)

{
    ulong curloc;
    int   byte;
    int   blk;
    int   type;
    int   kind;

    if (fseek(fp, position, 0) < 0)
        femsg(prgname, -errno, filespec);
    curloc = 0;
    puts((filever == 1)?
            "    Type        Position    Item number":
            "    Type         Kind     Position    Item number");
    while (--size >= 0)
    {
        byte = getbyte();		/* Get header byte */
        if (filever == 1)
        {
            blk = (byte >> 2) & 0x07;	/* Get msect or segment number */
            if (blk == 0x07)
                blk = getbyte();
            curloc += getitem(byte);
            type = byte >> 5;
            printf("    %-12s0x%8.8lX  %d\n", typename[type], curloc, blk);
        }
        else
        {
            kind = (byte>>2) & 0x03;
            if (kind != 3)
                blk = getvarval();
            curloc += getitem(byte & 3);
            if (kind != 3)
                printf("    %-13s%-9s0x%8.8X  %d\n",
                        typetbl[byte>>4], kindtbl[kind], curloc, blk);
            else
                printf("    %-13s%-9s0x%8.8X\n",
                        typetbl[byte>>4], kindtbl[kind], curloc);
        }
    }
}

long getitem(
    int byte)

{
    ulong loc;

    switch (byte & 0x03)
    {
     case 0:
        loc = getbyte();
        break;
     case 1:
        loc = getword();
        break;
     case 2:
        loc = getbyte();
        loc += (getword() << 8);
        break;
     default:
        loc = getlong();
        break;
    }
    return (loc);
}

long getvarval(void)

{
    long value;

    value = getbyte();
    if ((value & 0x80) == 0)
        return (value);
    value = (value << 8) + getbyte();
    if ((value & 0x4000) == 0)
        return (value & 0x3FFF);
    value = (value << 8) + getbyte();
    if ((value & 0x200000) == 0)
        return (value & 0x1FFFFF);
    return (((value << 8) + getbyte()) & 0x1FFFFFFF);
}

/*
 * Function to display not used byte
 */

void notused1(
    char *leader)

{
    printf("%sNot used: 0x%02.2X\n", leader, getbyte());
}

void notused2(
    char *leader)

{
    long data1;
    long data2;

    data1 = getbyte();
    data2 = getbyte();
    printf("%sNot used: 0x%02.2X 0x%02.2X\n", leader, data1, data2);
}

void notused3(
    char *leader)

{
    long data1;
    long data2;
    long data3;

    data1 = getbyte();
    data2 = getbyte();
    data3 = getbyte();
    printf("%sNot used: 0x%02.2X 0x%02.2X 0x%2.2X\n", leader, data1, data2,
            data3);
}

void notused4(
    char *leader)

{
    long data1;
    long data2;
    long data3;
    long data4;

    data1 = getbyte();
    data2 = getbyte();
    data3 = getbyte();
    data4 = getbyte();
    printf("%sNot used: 0x%02.2X 0x%02.2X 0x%2.2X 0x%2.2X\n", leader, data1,
            data2, data3, data4);
}

/*
 * Function to get long from rel file
 */

ulong getlong(void)

{
    ulong word1;
    ulong word2;

    word1 = getword();
    word2 = getword();
    return ((word2<<16) + word1);
}

/*
 * Function to get word for rel file
 */

int getword(void)

{
    int byte1;
    int byte2;

    byte1 = getbyte();
    byte2 = getbyte();
    return ((byte2<<8) + byte1);
}

/*
 * Function to get byte from rel file
 */

int getbyte(void)

{
    int value;

    value = getc(fp);
    if (value < 0)
    {
        if (errno == 0)
        {
            printf("\n? RUNDMP: Unexpected end of file\n");
            exit(1);
        }
        else
            femsg(prgname, -errno, filespec);
    }
    return (value);
}

//************************************
// Function: getname - Get symbol name
// Returned: Nothing
//************************************

void getname(void)

{
    char *pnt;
    int   cnt;
    char  chr;

    pnt = symname;
    cnt = sizeof(symname-4);
    while (((chr = getbyte()) & 0x80) == 0)
    {
        if (--cnt >= 0)
            *pnt++ = chr;
    }
    if (--cnt >= 0)
        *pnt++ = chr & 0x7F;
    else
    {
        *pnt++ = '*';
        *pnt++ = '*';
    }
    *pnt = 0;
}
