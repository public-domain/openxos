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
#include <CTYPE.H>
#include <ERRNO.H>
#include <STRING.H>
#include <TIME.H>
#include <XOS.H>
#include <XOSERRMSG.H>
#include <XCSTRING.H>
#include <XOSTIME.H>
#include <XOSERR.H>
#include <XOSSVC.H>

#define VERSNO 3
#define EDITNO 0

// Define XOS OBJ file section types

#define SC_EOM     0		// End of module
#define SC_INDEX   1		// Library index section
#define SC_START   2		// Start of module section
#define SC_SEG     3		// Segment definition section
#define SC_MSECT   4		// Msect definition section
#define SC_PSECT   5		// Psect definition section
#define SC_DATA    6		// Data section
#define SC_INTERN  7		// Internal symbol definition section
#define SC_LOCAL   8		// Local symbol definition section
#define SC_EXPORT  9		// Exported symbol definition section
#define SC_STRADR  10		// Starting address section
#define SC_DEBUG   11		// Debugger address section
#define SC_STACK   12		// Initial stack address section
#define SC_FILEREQ 13		// File request section
#define SC_EXTERN  14		// External symbol definition section
#define SC_SHARED  15		// Shared library definition section
#define SC_IMPORT  16		// Imported symbol definition section
#define SC_ENTRY   20		// Entry list section (obsolete)

// Define MS format OBJ file section types

#define MSSC_MIN    0x80
#define MSSC_THEADR 0x80	// Module header
#define MSSC_COMENT 0x88	// Comment
#define MSSC_MODEND 0x8A	// 16-bit module end
#define MSSC_386END 0x8B	// 32-bit module end
#define MSSC_EXTDEF 0x8C	// External definition
#define MSSC_TYPDEF 0x8E	// Type definition
#define MSSC_PUBDEF 0x90	// 16-bit public definition
#define MSSC_PUB386 0x91	// 32-bit public definition
#define MSSC_LOCSYM 0x92	// 16-bit local symbols
#define MSSC_LOC386 0x93	// 32-bit local symbols
#define MSSC_LINNUM 0x94	// 16-bit source line number
#define MSSC_LIN386 0x95	// 32-bit source line number
#define MSSC_LNAMES 0x96	// Name list
#define MSSC_SEGDEF 0x98	// 16-bit segment definition
#define MSSC_SEG386 0x99	// 32-bit segment definition
#define MSSC_GRPDEF 0x9A	// Group definition
#define MSSC_FIXUPP 0x9C	// Fix up previous 16-bit data image
#define MSSC_FIX386 0x9D	// Fix up previous 32-bit data image
#define MSSC_LEDATA 0xA0	// 16-bit logical data image
#define MSSC_LED386 0xA1	// 32-bit logical data image
#define MSSC_LIDATA 0xA2	// 16-bit logical repeated (iterated) data
				//   image
#define MSSC_LID386 0xA3	// 32-bit logical repeated (iterated) data
				//   image
#define MSSC_COMDEF 0xB0	// Communal names definition
#define MSSC_LOCEXD 0xB4	// External definition visible within module
				//   only
#define MSSC_FLCEXD 0xB5	// Func ext definition vis within module only
#define MSSC_LOCPUB 0xB6	// 16-bit public definition visible within
				//   module only
#define MSSC_LPB386 0xB7	// 32-bit public definition visible within
				//   module only
#define MSSC_LOCCOM 0xB8	// Communal name visible within module only
#define MSSC_MAX    0xB8

#define MSSC_COMDAT 0xC2	// 16-bit initialized communal data
#define MSSC_CMD386 0xC3	// 32-bit initialized communal data


#define MT_XOS 1
#define MT_MS  2

long  seccnt;
ulong offset;
long  symnum;
long  namenum;
long  segnum;
FILE *fp;
char *tname[] =
{   "not specified",
    "Data",
    "Code",
    "Stack",
    "Combined"
};
int   byte1;
int   byte2;
int   filetype;
int   modtype;
char  filespec[100];
char  name[258];
char  lowfirst;
char  prgname[] = "OBJDMP";
char  eofok;

void addrsect(char *name);
void colmsname(int cnt);
void colname(void);
void datsect(void);
void displaydata(int count);
void entsect(void);
void externsect(void);
void fail(void);
void filreqsect(void);
int  getbyte(void);
int  getitem(int flg);
int  getlong(void);
int  getmsnumber(void);
int  getnumber(void);
int  getword(void);
void importsect(void);
void libsect(void);
void mscoment(void);
void msextdef(void);
void msfixupp(int size);
void msledata(int size);
void mslnames(void);
void mssegdef(int size);
void mstheadr(void);
void mstsect(void);
void pstsect(void);
void secthdr(char *str);
void segsect(void);
void skipmsrest(void);
void skiprest(void);
void strsect(void);
void symsect(void);

//**********************************
// Function: main - Main program
// Returned: 0 if normal, 1 if error
//**********************************

main(
    int   argc,
    char *argv[])

{
    printf("OBJDMP - version %d.%d\n", VERSNO, EDITNO);
    if (argc != 2)
    {
        fputs("? OBJDMP: Command error, correct usage is:\n\
              RELDMP filename\n", stderr);
        exit(1);
    }
    strupper(argv[1]);
    strmov(strmov(filespec, argv[1]), ".OBJ");
    fp = fopen(filespec, "rb");
    if (fp == NULL)
        femsg(prgname, -errno, filespec);
    seccnt = 100;
    byte1 = getbyte();			// Read file header
    if (byte1 == 0x80)			// MS format OBJ file?
    {
        ungetc(byte1, fp);
        fputs("\nMS format OBJ file\n", stdout);
        filetype = MT_MS;
        lowfirst = TRUE;
    }
    else
    {
        byte2 = getbyte();
        printf("\nXOS format OBJ file\nFile header: 0x%2.2X 0x%2.2X\n",
                byte1, byte2);
        filetype = MT_XOS;
    }
    for (;;)
    {
        if (filetype == MT_XOS)
        {
            seccnt = 100;
            eofok = TRUE;
            byte1 = getbyte();		// Read module header
            eofok = FALSE;
            if (byte1 == 0)
            {
                puts("\nEnd of file mark\n");
                fclose(fp);
                exit(0);
            }
            byte2 = getbyte();
            if (byte1 == 0xC1)
            {
                lowfirst = TRUE;
                modtype = MT_MS;
            }
            else if (byte1 == 0x80)
            {
                lowfirst = FALSE;
                modtype = MT_XOS;
            }
            else if (byte1 == 0x81)
            {
                lowfirst = TRUE;
                modtype = MT_XOS;
            }
            else
            {
                printf("\nIllegal module header: 0x%02.2X 0x%02.2X\n",
                        byte1, byte2);
                fail();
            }
            printf("\n%s format module\nModule header: 0x%2.2X 0x%2.2X "
                     "(%s order byte first), offset = 0x%lX\n",
                     (modtype == MT_XOS)? "XOS": "MS", byte1, byte2,
                     (lowfirst)? "low": "high", ftell(fp)-2);
		}
        for (;;)
        {
            seccnt = 100;
            byte1 = getbyte();		// Get section type
			eofok = FALSE;
            if (byte1 == 0)
            {
                puts("\n  Section type = 0x00, SC_EOM (End of module)\n");
                break;
            }
            seccnt = (ulong)getword();	// Get byte count for section
            switch (byte1)
            {
             case SC_INDEX:	// Library index section
                secthdr("SC_INDEX (Library index)");
                libsect();
                break;

             case SC_START:	// Start of module section
                secthdr("SC_START (Module start)");
                strsect();
                break;

             case SC_SEG:	// Segment definition section
                secthdr("SC_SEG (Segment definition)");
                segsect();
                break;

             case SC_MSECT:	// Msect definition section
                secthdr("SC_MSECT (Msect definition)");
                mstsect();
                break;

             case SC_PSECT:	// Psect definition section
                secthdr("SC_PSECT (Psect definition)");
                pstsect();
                break;

             case SC_DATA:	// Data section
                secthdr("SC_DATA (Data)");
                datsect();
                break;

             case SC_INTERN:	// Intern symbol definition section
                secthdr("SC_INTERN (Internal symbols)");
                symsect();
                break;

             case SC_LOCAL:	// Local symbol definition section
                secthdr("SC_LOCAL (Local symbols)");
                symsect();
                break;

             case SC_EXPORT:			// Exported symbol definition section
                secthdr("SC_EXPORT (Exported symbols)");
                symsect();
                break;

             case SC_STRADR:			// Start address section
                secthdr("SC_STRADR (Starting address)");
                addrsect("Starting");
                break;

             case SC_DEBUG:				// Debugger address section
                secthdr("SC_DEBUG (Debugger address)");
                addrsect("Debugger");
                break;

             case SC_STACK:				// Initial stack pointer section
                secthdr("SC_STACK (Initial stack pointer)");
                addrsect("Stack");
                break;

             case SC_FILEREQ:			// File request section
                secthdr("SC_FILEREQ (File request)");
                filreqsect();
                break;

             case SC_EXTERN:			// External symbol table section
                secthdr("SC_EXTERN (External symbols)");
                externsect();
                break;

             case SC_IMPORT:			// Imported symbol table section
                secthdr("SC_IMPORT (Imported symbols)");
                importsect();
                break;

             case SC_ENTRY:				// Entry list section
                secthdr("SC_ENTRY (Entry list)");
                entsect();
                break;

             case MSSC_THEADR:			// MS module header
                secthdr("MSSC_THEADR (Module header)");
                mstheadr();
                break;

             case MSSC_COMENT:			// MS comment
                secthdr("MSSC_COMENT (Comment)");
                mscoment();
                break;

             case MSSC_MODEND:			// MS 16-bit module end
                secthdr("MSSC_MODEND (16-bit module end)");
		        displaydata(seccnt);
				eofok = TRUE;
                break;

             case MSSC_386END:			// MS 32-bit module end
                secthdr("MSSC_386END (32-bit module end)");
		        displaydata(seccnt);
				eofok = TRUE;
                break;

             case MSSC_EXTDEF:			// MS external definition
                secthdr("MSSC_EXTDEF (External definition)");
                msextdef();
                break;

             case MSSC_TYPDEF:			// MS type definition
                secthdr("MSSC_TYPDEF (Type defintion)");
		        displaydata(seccnt);
                break;

             case MSSC_PUBDEF:			// MS 16-bit public definition
                secthdr("MSSC_PUBDEF (16-bit public defintion)");
		        displaydata(seccnt);
                break;

             case MSSC_PUB386:			// MS 32-bit public definition
                secthdr("MSSC_PUB386 (32-bit public defintion)");
		        displaydata(seccnt);
                break;

             case MSSC_LOCSYM:			// MS 16-bit local symbols
                secthdr("MSSC_LOCSYM (16-bit local symbols)");
		        displaydata(seccnt);
                break;

             case MSSC_LOC386:			// MS 32-bit local symbols
                secthdr("MSSC_LOC386 (32-bit local symbols)");
		        displaydata(seccnt);
                break;

             case MSSC_LINNUM:			// MS 16-bit source line number
                secthdr("MSSC_LINNUM (16-bit line numbers)");
		        displaydata(seccnt);
                break;

             case MSSC_LIN386:			// MS 32-bit source line number
                secthdr("MSSC_LIN386 (32-bit line numbers)");
		        displaydata(seccnt);
                break;

             case MSSC_LNAMES:			// MS name list
                secthdr("MSSC_LNAMES (Name list)");
                mslnames();
                break;

             case MSSC_SEGDEF:			// MS 16-bit segment definition
                secthdr("MSSC_SEGDEF (16-bit segment definition)");
                mssegdef(16);
                break;

             case MSSC_SEG386:			// MS 32-bit segment definition
                secthdr("MSSC_SEG386 (32-bit segment definition)");
                mssegdef(32);
                break;

             case MSSC_GRPDEF:			// MS group definition
                secthdr("MSSC_GRPDEF (Group definition)");
                msgrpdef();
                break;

             case MSSC_FIXUPP:			// MS fix up previous 16-bit data image
                secthdr("MSSC_FIXUPP (16-bit fix-up");
                msfixupp(16);
                break;

             case MSSC_FIX386:			// MS fix up previous 32-bit data image
                secthdr("MSSC_FIX386 (32-bit fix-up)");
                msfixupp(32);
                break;

             case MSSC_LEDATA:			// MS 16-bit logical data image
                secthdr("MSSC_LEDATA (16-bit data)");
                msledata(16);
                break;

             case MSSC_LED386:			// MS 32-bit logical data image
                secthdr("MSSC_LEDATA (32-bit data)");
                msledata(32);
                break;

             case MSSC_LIDATA:			// MS 16-bit repeated data
                secthdr("MSSC_LIDATA (16-bit repeated data)");
		        displaydata(seccnt);
                break;

             case MSSC_LID386:			// MS 32-bit repeated data
                secthdr("MSSC_LID386 (32-bit repeated data)");
		        displaydata(seccnt);
                break;

             case MSSC_COMDEF:			// Communal names definition
                secthdr("MSSC_COMDEF (Communal names definiton)");
		        displaydata(seccnt);
                break;

             case MSSC_LOCEXD:			// MS local external definition
                secthdr("MSSC_LOCEXD (Local external definition)");
		        displaydata(seccnt);
                break;

             case MSSC_FLCEXD:			// MS Func ext definition vis within module
                secthdr("MSSC_FLCEXD (????)");
		        displaydata(seccnt);
                break;

             case MSSC_LOCPUB:			// MS 16-bit local public definition
                secthdr("MSSC_LOCPUB (16-bit local public definition)");
		        displaydata(seccnt);
                break;

             case MSSC_LPB386:			// MS 32-bit local public definition
                secthdr("MSSC_LPB386 (32-bit local public definition)");
		        displaydata(seccnt);
                break;

             case MSSC_LOCCOM:			// MS local communal name definition
                secthdr("MSSC_LOCCOM (Local communal name definition)");
		        displaydata(seccnt);
                break;

             case MSSC_COMDAT:			// 16-bit initialized communal data
                secthdr("MSSC_COMDAT (16-bit initialized communal data)");
		        displaydata(seccnt);
                break;

             case MSSC_CMD386:			// 32-bit initialized communal data
                secthdr("MSSC_CMD386 (32-bit initialized communal data)");
		        displaydata(seccnt);
                break;

             default:					// Illegal section type
                secthdr("ILLEGAL");
                fail();
            }
        }
    }
}


//**************************************************
// Function: libsect - Display library index section
// Returned: Nothing
//**************************************************

void libsect(void)

{
    long offset;

    offset = getlong();
    printf("    Module offset = 0x%lX\n", offset);
    entsect();

}

//********************************************************
// Function: strsect - Display XOS start of module section
// Returned: Nothing
//********************************************************

void strsect(void)

{
    colname();				// Display program name
    printf("    Program name = %s\n", name);
    if (seccnt > 0)
    {
        time_s create;
        char   buffer[60];

        create.time = getlong();
        create.date = getlong();
        sdt2str(buffer, "    Created at %H:%m:%s %D-%3n-%2y\n",
		(time_sz *)&create);
        fputs(buffer, stdout);
    }
    skiprest();				// Skip anything left over
}

//*******************************************************
// Function: segsect - Display segment definition section
// Returned: Nothing
//*******************************************************

void segsect(void)

{
    char *pnt;
    char *tpnt;
    int   num;
    int   flag;
    int   atrb;
    int   linked;
    int   type;
    int   priv;
    int   select;
    ulong addr;
    char *prefix;
    char  buf[80];

    num = 1;
    while (seccnt)
    {
        atrb = getbyte();		// Get attributes
        select = getitem(atrb);		// Get segment selector value
        flag = getbyte();		// Get flag byte
        addr = getitem(flag);		// Get address value
        if (flag & 0x10)
            linked = (int)getnumber();
        type = getbyte();		// Get segment type
        priv = getbyte();		// Get segment priviledge level
        colname();			// Get segment name
        pnt = buf;
        prefix = "(";
        if (atrb & 0x80)
        {
            pnt = strmov(strmov(pnt, prefix), "large");
            prefix = ", ";
        }
        pnt = strmov(strmov(pnt, prefix), (atrb & 0x40)? "32-bit": "16-bit");
        if (atrb & 0x20)
            pnt = strmov(pnt, ", conformable");
        if (atrb & 0x10)
            pnt = strmov(pnt, ", readable");
        if (atrb & 0x08)
            pnt = strmov(pnt, ", writable");
        tpnt = (type <= 4)? tname[type]: "???";
        printf("    Segment %d\n"
               "      Attributes = 0x%02.2X %s)\n"
               "      Selector = 0x%04.4X\n"
               "      Flags = 0x%02.2X%s\n"
               "      Address = %08.8lX\n", num, atrb, buf, select, flag,
               (flag & 0x08)? " (address specified)": "", addr);
        if (flag & 0x10)
            printf("      Linked segment = %d\n", linked);
        printf("      Type = %d (%s)\n"
               "      Priviledge level = %d\n"
               "      Name = %s\n",
                type, tpnt, priv, name);
        ++num;
    }
}

//*****************************************************
// Function: mstsect - Display msect definition section
// Returned: Nothing
//*****************************************************

void mstsect(void)

{
    char *tpnt;
    char *pnt;
    char *prefix;
    int   num;
    int   atrb;
    int   flag;
    uint  type;
    int   priv;
    int   seg;
    ulong max;
    ulong addr;
    char  buf[80];

    num = 1;
    while (seccnt)
    {
        atrb = getbyte();		// Get attribute byte
        max = getitem(atrb);		// Get max size value
        flag = getbyte();		// Get flag byte
        addr = getitem(flag);		// Get address value
        type = getbyte();		// Get msect type
        priv = getbyte();		// Get msect priviledge level
        seg = getbyte();		// Get segment number
        colname();			// Get psect name

        pnt = buf;
        prefix = " (";
        if (atrb & 0x20)
        {
            pnt = strmov(strmov(pnt, prefix), "conformable");
            prefix = ", ";
        }
        if (atrb & 0x10)
        {
            pnt = strmov(strmov(pnt, prefix), "readable");
            prefix = ", ";
        }
        if (atrb & 0x08)
        {
            pnt = strmov(strmov(pnt, prefix), "writable");
            prefix = ", ";
        }
        if (pnt != buf)
            *pnt++ = ')';
        *pnt = '\0';
        tpnt = (type <= 4)? tname[type]: "???";
        printf("    Msect %d\n"
               "      Attributes = 0x%02.2X%s\n"
               "      Max = 0x%08.8lX\n"
               "      Flags = 0x%02.2X%s\n"
               "      Address = 0x%08.8lX\n"
               "      Type  = %d (%s)\n"
               "      Priviledge = %d, Segment = %d\n"
               "      Name  = %s\n", num, atrb, buf, max, flag,
               (flag & 0x08)? " (address specified)": "", addr, type, tpnt,
               priv, seg, name);
        ++num;
    }
}

//*****************************************************
// Function: pstsect - Display psect definition section
// Returned: Nothing
//*****************************************************

void pstsect(void)

{
    int  num;
    int  atrb1;
    int  atrb2;
    int  flag;
    int  msect;
    long tsize;
    long lsize;
    long addr;

    num = 1;
    while (seccnt)
    {
        atrb1 = getbyte();		// Get first attribute byte
        tsize = getitem(atrb1);		// Get total size
        atrb2 = getbyte();		// Get second attribute byte
        lsize = getitem(atrb2);		// Get loaded size
        flag = getbyte();		// Get flag byte
        addr = getitem(flag);		// Get address value
        msect = getbyte();		// Get msect number
        colname();			// Get psect name
        printf("    Psect %d\n"
               "      Attributes1 = 0x%02.2X (%s)\n"
               "      Total size  = %ld\n"
               "      Attributes2 = 0x%02.2X\n"
               "      Loaded size = %ld\n"
               "      Flags = 0x%02.2X%s\n"
               "      Address = 0x%08.8lX\n"
               "      Msect = %d\n"
               "      Name  = %s\n", num, atrb1,
               (atrb1 & 0x40)? "overlayed": "concatenated", tsize, atrb2,
               lsize, flag, (flag & 0x08)? " (address specified)": "", addr,
               msect, name);
        ++num;
    }
}

// Polish operator table

char badopr[] = "PO_???? (Illegal)";
char *polopr[] =
{   "PO_ADD (Add)",					// 0x80
    "PO_SUB (Subtract)",				// 0x81
    "PO_MUL (Multiply)",				// 0x82
    "PO_DIV (Divide)",					// 0x83
    "PO_SHF (Shift)",					// 0x84
    "PO_AND (And)",					// 0x85
    "PO_IOR (Inclusive or)",				// 0x86
    "PO_XOR (Exclusive or)",				// 0x87
    "PO_COM (1's complement",				// 0x88
    "PO_NEG (Negate)",					// 0x89
    badopr,						// 0x8A
    badopr,						// 0x8B
    badopr,						// 0x8C
    badopr,						// 0x8D
    badopr,						// 0x8E
    badopr,						// 0x8F
    badopr,						// 0x90
    badopr,						// 0x91
    "PO_PSELCP (Push selector for current psect)",	// 0x92
    "PO_PSELAP (Push selector for any psect)",		// 0x93
    "PO_PSELAM (Push selector for any msect)",		// 0x94
    "PO_PSELAS (Push selector for any segment)",	// 0x95
    "PO_POFSAM (Push offset for any msect)",		// 0x96
    "PO_PSELSYM (Push symbol selector value)",		// 0x97
    "PO_STRAW (Store word address)",			// 0x98
    "PO_STRAL (Store long address)",			// 0x99
    "PO_STRUB (Store unsigned byte)",			// 0x9A
    "PO_STRUW (Store unsigned word)",			// 0x9B
    "PO_STRUL (Store unsigned long)",			// 0x9C
    "PO_STRSB (Store signed byte)",			// 0x9D
    "PO_STRSW (Store signed word)",			// 0x9E
    "PO_STRSL (Store signed long)"			// 0x9F
};

//******************************************
// Function: datasect - Display data section
// Returned: Nothing
//******************************************

void datsect(void)

{
    uchar byteh;
    long  val;

    while (seccnt)
    {
        byteh = getbyte();
        if (byteh == 0)
        {
            fputs("    Illegal text header byte = 0", stdout);
            fail();
        }
        else if (!(byteh & 0x80))
        {
            if (byteh >= 120)		// Address?
            {
                val = getitem(byteh);
                printf("    Offset = (%d) 0x%8.8lX\n", getnumber(), val);
            }
            else
            {
                printf("    Absolute data, count = %d\n", byteh);
                displaydata(byteh);
            }
        }
        else				// If operator
        {
            if (byteh < 0xA0)		// Stack operator?
            {
                printf("    Operator = %s = 0x%2.2X", polopr[byteh-0x80],
                        byteh);
                switch (byteh)
                {
                 case 0x93:	// Push selector for any psect
                    printf("\n      Psect = %d", getnumber());
                    break;

                 case 0x94:	// Push selector for any msect
                 case 0x96:	// Push offset for any msect
                    printf("\n      Msect = %d", getnumber());
                    break;

                 case 0x95:	// Push selector for any segment
                    printf("\n      Segment = %d", getnumber());
                    break;

                 case 0x97:	// Push selector for symbol selector value
                    printf("\n      Symbol = %d", getnumber());
                }
                putchar('\n');
            }
            else
            {
                switch (byteh & 0xF8)
                {
                 case 0xA0:	// Push absolute value
                    val = getitem(byteh);
                    printf("    Operator = PO_PVAL (Push absolute value)"
                           " = 0x%2.2X\n      Value = 0x%8.8lX\n", byteh, val);
                    break;

                 case 0xA8:	// Push absolute value, relative
                    val = getitem(byteh);
                    printf("    Operator = PO_PVALREL (Push absolute value,"
                           " relative) = 0x%2.2X\n      Value = 0x%8.8lX\n",
                           byteh, val);
                    break;

                 case 0xB0:	 // Push offset, current psect
                    val = getitem(byteh);
                    printf("    Operator = PO_POFSCP (Push offset,"
                           " current psect) = 0x%2.2X\n      Value ="
                           " 0x%8.8lX\n", byteh, val);
                    break;

                 case 0xC0:	// Push offset, any psect
                    val = getitem(byteh);
                    printf("    Operator = PO_POFSAP (Push offset, any psect)"
                           " = 0x%2.2X\n      Value = 0x%8.8lX, psect"
                           " = 0x%2.2X\n", byteh, val, getnumber());
                    break;

                 case 0xC8:	 // Push offset, relative, any psect
                    val = getitem(byteh);
                    printf("    Operator = PO_POFSRELAP (Push offset, relative,"
                           " any psect) = 0x%2.2X\n      Value ="
                           " 0x%8.8lX, psect = 0x%2.2X\n", byteh, val,
                           getnumber());
                    break;

                 case 0xD8:	// Push address, current psect
                    printf("    Operator = PO_PADRCP (Push address, current"
                           " psect) = 0x%2.2X\n      Offset = 0x%8.8lX\n",
                           byteh, getitem(byteh));
                    break;

                 case 0xE0:	// Push address, any psect*/
                    val = getitem(byteh);
                    printf("    Operator = PO_PADRAP (Push address, any psect)"
                           " = 0x%2.2X\n      Offset = 0x%8.8lX, psect = %d'n",
                           byteh, val, getnumber());
                    break;

                 case 0xE8:	// Push symbol address value
                    val = getitem(byteh);
                    printf("    Operator = PO_PADRSYM (Push symbol address "
                           "value) = 0x%2.2X\n      Offset = 0x%8.8lX, "
                           "symbol = %d\n", byteh, val, getnumber());
                    break;

                 case 0xF0:	// Push symbol offset value 
                    val = getitem(byteh);
                    printf("    Operator = PO_POFSSYM (Push symbol offset "
                           "value) = 0x%2.2X\n      Offset = 0x%8.8lX, symbol "
                           "= %d\n", byteh, val, getnumber());
                    break;

                 case 0xF8:	// Push symbol offset value, relative
                    val = getitem(byteh);
                    printf("    Operator = PO_POFSRELSYM (Push symbol offset "
                           "value, relative) = 0x%2.2X\n      Offset = "
                           "0x%8.8lX, symbol = %d\n", byteh, val, getnumber());
                    break;

                 default:		// Illegal
                    printf("    Operator = PO_???? (Illegal) = 0x%2.2X"
                           "\n*** Unknown parameter size ***", byteh);
                    fail();
                }
            }
        }
    }
}

//**********************************************************************
// Function: symsect - Display global or local symbol definition section
// Returned: Nothing
//**********************************************************************

void symsect(void)

{
    int   flag;
    int   psn;
    int   val;
    char *pnt;
    char  attrib[20];

    fputs("     Attributes          Value         Name\n", stdout);
    while (seccnt)
    {
        flag = getbyte();		// Get flags
        val = getitem(flag);		// Get value
        if (flag & 0x20)
            psn = (flag & 0x80)? getword(): getnumber();
        else
        {
            if (!(flag & 0x80))		// If ABS bit not set, eat the extra
                getbyte();		//   byte that old versions of XMAC
        }				//   put in!
        colname();			// Get symbol name

        pnt = strmov(attrib, (flag&0x80)? "Abs": "...");
        pnt = strmov(pnt, (flag&0x40)? "Ent": "...");
        pnt = strmov(pnt, (flag&0x20)? "Adr": "...");
        pnt = strmov(pnt, (flag&0x10)? "Sup": "...");

        if (flag & 0x20)
        {
            if (flag & 0x80)
                printf("    %s  0x%04.4X:0x%08.8X  %s\n", attrib, psn, val,
                        name);
            else
                printf("    %s   (%3d) 0x%8.8X  %s\n", attrib, psn, val, name);
        }
        else
            printf("    %s         0x%08.8X  %s\n", attrib, val, name);
    }
}

//****************************************************
// Function: firreqsect - Display file request section
// Returned: Nothing
//****************************************************

void filreqsect(void)

{
    char *pnt;
    int   cnt;
    char  buffer[514];

    pnt = buffer;
    cnt = 512;
    while (seccnt)
    {
        if (--cnt >= 0)
            *pnt++ = getbyte();
    }
    if (cnt < 0)
        *pnt++ = '*';
    *pnt = '\0';
    printf("    File = %s\n", buffer); // Display it
}

//**************************************************
// Function: addrsect - Display address from section
// Returned: Nothing
//**************************************************

void addrsect(
    char *name)

{
    long temp;

    temp = getlong();			// Display starting address
    printf("    %s address = 0x%8.8lX 0x%2.2X\n", name, temp, getbyte());
    skiprest();				// Skip anything left over
}

//***********************************************
// Function: entsect - Display entry list section
// Returned: Nothing
//***********************************************

void entsect(void)

{
    while (seccnt)
    {
        colname();			// Get symbol name
        printf("    Symbol = %s\n", name); // Display it
    }
}

//*************************************************************
// Function: externsect - Display external symbol names section
// Returned: Nothing
//*************************************************************

void externsect(void)

{
    fputs("    Number   Name\n", stdout);
    while (seccnt)
    {
        colname();			// Get symbol name
        printf("    %5d  %s\n", ++symnum, name);
    }
}

//*************************************************************
// Function: importsect - Display imported symbol names section
// Returned: Nothing
//*************************************************************

void importsect(void)

{
    int flag;

    fputs("    Number Attrbts  Name\n", stdout);
    while (seccnt)
    {
        flag = getbyte();		// Get flags
        colname();			// Get symbol name
        printf("    %5d   %s   %s\n", ++symnum, (flag&0x40)? "Ent": "...",
                name);
    }
}

//***********************************************
// Function: mstheadr - Display MS THEADR section
// Returned: Nothing
//***********************************************

void mstheadr(void)

{
    colmsname(getbyte());		// Display program name
    printf("    Program name = %s\n", name);
    if (seccnt >= 9)
    {
        time_s create;
        char   buffer[60];

        create.time = getlong();
        create.date = getlong();
        sdt2str(buffer, "    Created at %H:%m:%s %D-%3n-%2y\n",
		(time_sz *)&create);
        fputs(buffer, stdout);
    }
    skipmsrest();			// Skip anything left over
}

//***************************************************************
// Function: msgrpdef - Display MS group definition (MSSC_GRPDEF)
// Returned: Nothing
//***************************************************************

void msgrpdef(void)

{
    printf("    Group   = %d\n    Members =", getmsnumber());
    while (seccnt >= 3)
    {
        getbyte();
        printf(" %d", getmsnumber());
    }
    fputs("\n", stdout);
    getbyte();				// Discard the checksum byte
}

//*****************************************************************
// Function: msfixupp - Display MS group definition (MSSC_FIXUPP or
//		MSSC_FIX386)
// Returned: Nothing
//*****************************************************************

void msfixupp(
    int size)

{
    static char *loctbl[] =
    {   "byte",
        "word",
        "selector",
        "word addr",
        "??? (4)",
        "word",
        "??? (6)",
        "??? (7)",
        "??? (8)",
        "long",
        "??? (10)",
        "long addr",
        "??? (11)",
        "??? (12)",
        "long",
        "??? (14)",
        "??? (15)"
    };

    static char *targettbl[] =
    {   "SEGDEF",
        "GRPDEF",
        "EXTDEF",
        "frame"
    };

    static char *frametbl[] =
    {   "SEGDEF",
        "GRPDEF",
        "EXTDEF",
        "?? (3)",
        "DATA",
        "TARGET",
        "?? (6)",
        "?? (7)"
    };

    int temp1;
    int frame;
    int target;
    int fdatum;
    int tdatum;
    int displ;

    while (seccnt >= 2)
    {
        temp1 = getbyte();
        if (temp1 & 0x80)		// Fixup subrecord?
        {				// Yes
            printf("    %s %-9.9s, Offset = %04.4X\n",
                    (temp1&0x40)? "Abs": "Rel", loctbl[(temp1&0x3C)>>2],
                    ((temp1<<8) + getbyte()) & 0x3FF);

            temp1 = getbyte();		// Get the fix data byte
            frame = (temp1>>4) & 0x07;
            target = temp1 & 0x07;
            if (((temp1 & 0x80) == 0) && (frame <= 2))
                fdatum = getmsnumber();
            if ((temp1 & 0x08) == 0)
                tdatum = getmsnumber();
            if ((temp1 & 0x04) == 0)
                displ = (size == 16)? getword(): getlong();
            else
                displ = 0;

            if (temp1 & 0x80)
                printf("      Frame:  thread #%d\n", fdatum);
            else if (frame <= 2)
                printf("      Frame:  method = %s, data = %d\n", frametbl[frame],
                        fdatum);
            else
                printf("      Frame:  method = %s\n", frametbl[frame]);

            if (temp1 & 0x08)
                printf("      Target: thread #d, disp = 0x%X\n", target, displ);
            else
                printf("      Target: method = %s, disp = 0x%X, data = %d\n",
                    targettbl[target&0x03], displ, tdatum);
        }
        else				// If thread subrecord
        {
            printf("? Have thread subrecord!\n");
            exit(1);

            fputs("\n", stdout);
        }
    }
    getbyte();				// Discard the checksum byte
}

//*********************************************************
// Function: msledata - Display MS data record (MSSC_LEDATA
//		or MSSC_LED386)
// Returned: Nothing
//*********************************************************

void msledata(
    int size)

{
    printf("    Segment = %d\n", getmsnumber());
    if (size == 16)
        printf("    Offset  = 0x%04.4X\n", getword());
    else
        printf("    Offset  = 0x%08.8X\n", getlong());
    if (seccnt > 1)
        displaydata(seccnt - 1);	// Display the data
    getbyte();				// Discard the checksum byte
}

//*************************************************************
// Function: mscoment - Display MS comment record (MSSC_COMENT)
// Returned: Nothing
//*************************************************************

void mscoment(void)

{
    int type;

    type = getbyte();
    printf("    Type  = 0x%02.2X (%s%s%s)\n", type, (type & 0x80)? "no purge": "",
            ((type&0xC0) == 0xC0)? ", ": "", (type & 0x40)? "no list": "");
    type = getbyte();
    printf("    Class = 0x%02.2X (", type);
    switch (type)
    {
     case 0x00:
        fputs("translator)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0x01:
        fputs("copyright)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0x81:
        fputs("library specifier - obsolete)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0x9C:
        fputs("MS-DOS version - obsolete)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0x9D:
        fputs("memory model)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0x9E:
        fputs("DOSSEG)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0x9F:
        fputs("default library search name)\n", stdout);
        colmsname(seccnt - 1);
        printf("    Name  = %s\n", name);
        break;

     case 0xA0:
        fputs("OMF extensions)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0xA1:
        fputs("new OMF extensions)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0xA2:
        fputs("LINK pass)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0xA3:
        fputs("LIBMOD - library module comment)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0xA4:
        fputs("EXESTR - executable string)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0xA6:
        fputs("INCERR - incremental compilation error)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0xA7:
        fputs("NOPAD - no segment padding)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0xA8:
        fputs("WKEXT - weak extern record)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0xA9:
        fputs("LZEXT - lazy extern record)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0xAA:
        fputs("PharLap format)\n", stdout);
        colmsname(seccnt - 1);
        printf("    Name  = %s\n", name);
        break;

     case 0xB0:
        fputs("initial IBM OMF386 format - obsolete)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0xB1:
        fputs("record order - obsolete)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0xDA:
        fputs("comment)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0xDB:
        fputs("compiler)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0xDC:
        fputs("date)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0xDD:
        fputs("time)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0xDF:
        fputs("user)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0xE9:
        fputs("dependency file)\n", stdout);
        displaydata(seccnt - 1);
        break;

     case 0xFF:
        fputs("command line)\n", stdout);
        displaydata(seccnt - 1);
        break;

     default:
        fputs("unknown class)\n", stdout);
        displaydata(seccnt - 1);
        break;
    }
    getbyte();				// Discard the checksum byte
}

//***********************************************************************
// Function: msextdef - Display MS external name definition (MSSC_EXTDEF)
// Returned: Nothing
//***********************************************************************

void msextdef(void)

{
    fputs("    Number Type   Name\n", stdout);
    while (seccnt > 3)
    {
        colmsname(getbyte());		// Get symbol name
        printf("    %5d %4d  %s\n", ++symnum, getmsnumber(), name);
    }
    getbyte();				// Discard the checksum byte
}

//****************************************************************
// Function: mssegdef - Display MS segment definition (MSSC_SEGDEF
//		or MSSC_SEG386)
// Returned: Nothing
//****************************************************************

void mssegdef(
    int size)

{
    long attrib;
    long temp2;
    static char *aligntbl[] =
    {   "byte",
        "word",
        "paragraph",
        "256-byte",
        "long",
        "4K-byte",
        "?????"
    };
    static char *combtbl[] =
    {   "Private",
        "Reserved (1)",
        "Public",
        "Reserved (3)",
        "Public",
        "Stack",
        "Common",
        "Public"
    };

    printf("    Number  = %d\n", ++segnum);
    attrib = getbyte();
    temp2 = attrib >> 5;
    if (temp2 == 0)
    {
        printf("    Absolute, frame = %d\n", getword());
        getbyte();
    }
    else
        printf("    Relocatable, alignment = %s\n", aligntbl[temp2-1]);
    printf("    Combine = %s\n", combtbl[(attrib>>2)&0x07]);
    printf("    Width   = %d\n", (attrib & 0x01)? 32: 16);
    temp2 = (size == 16)? getword(): getlong();
    if (attrib & 0x02)
        printf("    Size    = %s\n", (size == 16)? "65536": "4294967296");
    else
        printf("    Size    = %u\n", temp2);
    printf("    Name    = %d\n", getmsnumber());
    printf("    Class   = %d\n", getmsnumber());
    printf("    Overlay = %d\n", getmsnumber());
    getbyte();
}

//*******************************************************************
// Function: mslnames - Display MS list of names record (MSSC_LNAMES)
// Returned: Nothing
//*******************************************************************

void mslnames(void)

{
    fputs("    Number   Name\n", stdout);
    while (seccnt > 1)
    {
        colmsname(getbyte());		// Get symbol name
        printf("    %5d  %s\n", ++namenum, name);
    }
    getbyte();				// Discard the checksum byte
}

//*****************************************************************
// Function: mscomdat - Display MS initialized communal data record
//		(MSSC_LNAMES)
// Returned: Nothing
//*****************************************************************

void mscomdat(
    int size)

{
    int temp1;
    int alloc;
    static char *selecttbl[] =
    {   "No match",
        "Pick any",
        "Same size",
        "Exact match",
        "Illegal (0x4)",
        "Illegal (0x5)",
        "Illegal (0x6)",
        "Illegal (0x7)",
        "Illegal (0x8)",
        "Illegal (0x9)",
        "Illegal (0xA)",
        "Illegal (0xB)",
        "Illegal (0xC)",
        "Illegal (0xD)",
        "Illegal (0xE)",
        "Illegal (0xF)"
    };
    static char *alloctbl[] =
    {   "Explicit",
        "Far code",
        "Far data",
        "Code32",
        "Data32",
        "Illegal (0x5)",
        "Illegal (0x6)",
        "Illegal (0x7)",
        "Illegal (0x8)",
        "Illegal (0x9)",
        "Illegal (0xA)",
        "Illegal (0xB)",
        "Illegal (0xC)",
        "Illegal (0xD)",
        "Illegal (0xE)",
        "Illegal (0xF)"
    };
    static char *aligntbl[] =
    {   "From SEGDEF",
        "Byte",
        "Word",
        "Paragraph",
        "256-byte",
        "long"
    };

    temp1 = getbyte();
    printf("    Flags  = 0x%02.2X%s%s%s%s%s%s%s%s%s\n", temp1,
            (temp1&0x0F)? " (":"", (temp1&0x08)? "codeseg":"",
            ((temp1&0x08)&&(temp1&0x07))? ", ":"", (temp1&0x04)? "local":"",
            ((temp1&0x04)&&(temp1&0x03))? ", ":"", (temp1&0x02)? "iterated":"",
            ((temp1&0x02)&&(temp1&0x01))? ", ":"",
            (temp1&0x01)? "continuation":"", (temp1&0x0F)? ")":"");
    temp1 = getbyte();
    alloc = temp1 & 0x0F;
    printf("    Select = %s\n    Alloc  = %s\n", selecttbl[temp1>>4],
            alloctbl[alloc]);
    temp1 = getbyte();
    if (temp1 <= 5)
        printf("    Align  = %s\n", aligntbl[temp1]);
    else
        printf("    Align  = Illegal (0x%X)\n", temp1);
    if (size == 16)
        printf("    Offset = 0x%04.4X\n", getword());
    else
        printf("    Offset = 0x%08.8X\n", getlong());
    printf("    Type   = %d\n", getmsnumber());
    if (alloc == 0)
    {
        int seg;
        int grp;

        seg = getbyte();
        grp = getbyte();
        if ((seg | grp) != 0)
            printf("    Group  = %d, Segment = %d\n", seg, grp);
        else
            printf("    Frame = 0x%04.4X\n", getword());
    }
    printf("    Name   = %d\n", getmsnumber());
    if (seccnt > 1)
        displaydata(seccnt - 1);	// Display the data
    getbyte();				// Discard the checksum byte
}

//************************************************
// Function: secthdr - Display section header line
// Returned: Nothing
//************************************************

void secthdr(
    char *str)

{
    printf("\n  Section type = 0x%02.2X, size = %ld, %s\n"
             "        Offset = 0x%lX\n", byte1,
            seccnt, str, ftell(fp)-3);
}

//***************************************************
// Function: skiprest - Skip remainder of XOS section
// Returned: Nothing
//***************************************************

void skiprest(void)

{
    if (seccnt)
    {
        printf("  Extra data in section, %d byte%s discarded\n", seccnt,
                (seccnt == 1)? "":"s");
        while (seccnt > 0)
            getbyte();
    }
}

//****************************************************
// Function: skipmsrest - Skip remainder of MS section
// Returned: Nothing
//****************************************************

void skipmsrest(void)

{
    getbyte();				// Eat the checksum byte
    skiprest();
}

//**********************************************************
// Function: colname - Collect XOS symbol name from OBJ file
// Returned: Nothing
//**********************************************************

// Symbol is stored without any header or count byte.  End of symbol is flagged
//   by the high order bit set in the last byte.

void colname(void)

{
    int   cnt;
    char *pnt;
    char  chr;

    cnt = 256;
    pnt = name;
    do
    {
        chr = getbyte();
        if (--cnt >= 0)
            *pnt++ = chr & 0x7F;
    } while (chr > 0);
    if (cnt < 0)
        *pnt++ = '*';
    *pnt = '\0';
}

//***********************************************************
// Function: colmsname - Collect MS symbol name from OBJ file
// Returned: Nothing
//***********************************************************

// Symbol is stored preceeded by a count byte which specifies the number of
//   bytes in the symbol.  Count does not include the count byte.

void colmsname(
    int   cnt)

{
    int   size;
    char *pnt;
    char  chr;

    size = 0;
    pnt = name;
    while (--cnt >= 0)
    {
        chr = getbyte();
        if (++size < 256)
            *pnt++ = chr;
    }
    if (size >= 256)
        *pnt++ = '*';
    *pnt = '\0';
}

//************************************************************
// Function: getitem - Get variable length value from OBJ file
// Returned: Nothing
//************************************************************

int getitem(
    int flg)

{
    ulong value;

    switch (flg & 7)
    {
    case 0:
        value = 0;
        break;
    case 1:
        value = getbyte();
        break;
    case 2:
        value = getword();
        break;
    case 3:
    case 7:
        value = getlong();
        break;
    case 4:
        value = 0xFFFFFFFFl;
        break;
    case 5:
        value = getbyte() | 0xFFFFFF00l;
        break;
    case 6:
        value = getword() | 0xFFFF0000l;
        break;
    }
    return (value);
}

//*******************************************
// Function: getlong - Get long from OBJ file
// Returned: Value of long
//*******************************************

int getlong(void)

{
    ulong word1;
    ulong word2;

    word1 = getword();
    word2 = getword();
    if (lowfirst)
        return ((word2<<16) + word1);
    else
        return ((word1<<16) + word2);
}

//*******************************************
// Function: getword - Get word from OBJ file
// Returned: Value of word
//*******************************************

int getword(void)

{
    int byte1;
    int byte2;

    byte1 = getbyte();
    byte2 = getbyte();
    if (lowfirst)
        return ((byte2<<8) + byte1);
    else
        return ((byte1<<8) + byte2);
}

//*******************************************
// Function: getbyte - Get byte from OBJ file
// Returned: Value of byte
//*******************************************

int getbyte(void)

{
    int value;

    if (--seccnt < 0)
    {
        fputs("Not enough data in section", stdout);
        fail();
    }
    value = getc(fp);
    if (value < 0)
    {
        if (errno == 0 || errno == -ER_EOF)
        {
            if (eofok)
            {
                printf("\nPhysical end of file\n");
                exit(0);
            }
            else
            {
                printf("\nUnexpected physical end of file\n");
                fail();
            }
        }
        else
            femsg(prgname, -errno, filespec);
    }
    ++offset;
    return (value);
}

//**************************************************************
// Function: getnumber - Get variable length value from OBJ file
// Returned: Value of number
//**************************************************************

int getnumber(void)

{
    ulong value;

    value = getbyte();			// Get first byte
    if (!(value & 0x80L))		// 8 bit value?
        return (value);			// Yes - return it
    value = (value << 8) + getbyte();	// No - get next 8 bits

    if (!(value & 0x4000L))		// 16 bit value?
        return (value & 0x7FFFL);	// Yes - return it

    value = (value << 8) + getbyte();	// Get next 8 bits

    if (!(value & 0x200000L))		// 24 bit value?
        return (value & 0x3FFFFFL);	// Yes - return it
    return (  ((value << 8) + getbyte()) & 0x1FFFFFFFL);
}					// Return 32 bit value

//*******************************************************************
// Function: getmsnumber - Get variable length MS value from OBJ file
// Returned: Value of number
//*******************************************************************

int getmsnumber(void)

{
    ulong value;

    value = getbyte();			// Get first byte
    if (!(value & 0x80L))		// 8 bit value?
        return (value);			// Yes - return it
    value = (value << 8) + getbyte();	// No - get next 8 bits
    return (value & 0x7FFFL);
}

//*********************************************
// Function: displaydata - Display data section
// Returned: Nothing
//*********************************************

void displaydata(
    int count)

{
    char *pnt1;
    int  *pnt2;
    int   cnt1;
    int   byte;
    int   offset;
    int   data[16];
    char  chr;
    char  buffer[100];

    if (count == 0)
        return;
    strmov(buffer, "      ");
    offset = 0;
    do
    {
        pnt1 = buffer + 6;
        pnt1 += sprintf(pnt1, "%04.4X> ", offset);
        offset += 16;
        cnt1 = 16;
        pnt2 = data;
        do
        {
            if (--count >= 0)
            {
                *pnt2++ = byte = getbyte(); // Get a byte
                pnt1 += sprintf(pnt1, "%02.2X ", byte);    
            }
            else
            {
                *pnt2++ = -1;
                pnt1 = strmov(pnt1, "   ");
            }
        } while (--cnt1 > 0);

        pnt1 = strmov(pnt1, " |");
        pnt2 = data;
        cnt1 = 16;
        do
        {
            *pnt1++ = (((chr = (*pnt2++)&0x7F) >= 0x20) && (chr < 0x7F))?
                    chr: '.';
        } while (--cnt1 > 0 && *pnt2 != -1);
        strmov(pnt1, "|\n");
        fputs(buffer, stdout);
    } while (count > 0);
}

//********************************
// Function: fail - Report failure
// Returned: Never returns
//********************************

void fail(void)

{
    printf("\n\n****************************\n"
               "* File offset = 0x%08.8lX *\n"
               "? Cannot continue!         *\n"
               "****************************\n", ftell(fp));
    exit(1);
}
