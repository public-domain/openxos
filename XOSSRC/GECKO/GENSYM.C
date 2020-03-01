/******************************************/
/*                                        */
/*          ****  GENSYM  ****            */
/*                                        */
/* Program to generate GECKO symbol table */
/* file from LINK86 map file              */
/******************************************/

#include <CTYPE.H>
#include <ERRNO.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>

#include <XOSSVC.H>
#include <XOSERR.H>

void   femsg(char *name, long code, char *file);

// Format of the symbol table file:
// File header (12 bytes):
//      ID       (4 bytes)  = 030222D4h
//	Length   (4 bytes)  = Number of bytes in file
//	Count    (4 bytes)  = Number of table entries in file
// Symbol definition:
//	Flags    (1 bytes)  = Symbol flags:
//				80h - Symbol is a segment selector value
//				40h - Symbol is multiply defined
//				20h - Symbol is an address
//				10h - Symbol is suppressed
//				08h - Symbol is global
//				04h - Symbol is a msect selector/offset value
//				02h - Symbol is a module name
//	Offset   (2 bytes)  = Offset part of value
//	Selector (2 bytes)  = Selector part of value
//	Name     (variable) = Symbol name (high bit set on last byte)

FILE *ifile;
FILE *ofile;
long  value;			// Value of hex field
short segment;			// Segment part of value
int   symsize;			// Length of symbol
long  symcnt;			// Number of entries in SYM file
long  tblsize;
long  temp;
int   line;			// Line number in map (for error reporting)
char  lastchr;			// Last character input
char  flags;			// Symbol entry flag bits
char  symbol[34];		// Value of alpha field
char  prgname[] = "GENSYM";
char  symhead[] = {0xD4, 0x22, 0x02, 0x03, 0, 0, 0, 0, 0, 0, 0, 0};
char  iname[100];		// Buffer for input file name
char  oname[100];		// Buffer for output file name

int  advance(char *str);
void badmap(void);
void fatal(char *name);
int  gethex(void);
void getin(void);
void getlabel(void);
void getname(void);
void nextline(void);
void putec(char byte);
void putes(char *str, int cnt);
void skpchrs(int cnt);
void symentry(char flagbyte);

main(argc, argv)
int   argc;
char *argv[];

{
    char *pnt;
    char  chr;

    fputs("GENSYM - version 1.1\n", stdout);
    if (argc != 2)
    {
        puts("? GENSYM: Illegal command usage, correct usage is:\n\
          GENSYM name");
        exit(1);
    }
    strcpy(iname, argv[1]);		// Copy file name
    pnt = iname;			// Make sure all upper case
    while ((chr = *pnt) != '\0')
        *pnt++ = toupper(chr);
    strcpy(oname, iname);		// Copy again for output name
    strcat(iname, ".MAP");		// Make up complete input name
    strcat(oname, ".SYM");		// And complete output name
    if ((ifile=fopen(iname, "r")) == NULL)
        femsg(prgname, -errno, iname);
    if ((ofile=fopen(oname, "wb")) == NULL)
        femsg(prgname, -errno, oname);
    putes(symhead, 12);			// Write out symbol file header
    tblsize = 4;
    symcnt = 0;

    // Output segment names as symbols

    nextline();
    nextline();
    nextline();
    segment = 0;
    while (gethex())			// Get value
    {
        value /= 0x10;			// Change to selector value
        skpchrs(15);			// Skip 15 characters
        getname();			// Get symbol name
        symentry(0x98);			// Generate symbol table entry for
    }					//   segment name

    // Output group names as symbols

    getin();
    getin();
    if (lastchr == 'O')			// Do we have group stuff?
    {
        nextline();
        while (gethex())		// Get value of group name
        {
           skpchrs(4);			// Skip to name
           getname();			// Get symbol name
           symentry(0x98);		// Generate symbol table entry
        }
        getin();
    }
    if (lastchr != ' ')
        badmap();

    // Output normal global symbols

    nextline();				// Advance to first data line
    nextline();
    while (gethex())
    {
        segment = (short)value;		// Store segment value
        gethex();			// Get offset value
        getin();
        getin();
        if (lastchr == 'A')		// Absolute value?
            flags = 0x08;		// Yes
        else
            flags = 0x28;		// No - its an address
        skpchrs(4);
        getname();
        symentry(flags);
    }

    // Output line numbers as local symbols

    if (advance("Line"))
    {
        do
        {
            do				// Scan to left paren
            {
                if (lastchr == '\0' || lastchr == '\n')
                    badmap();
                getin();
            } while (lastchr != '(');
            getname();			// Get name of module
            segment = 0;
            value = 0;
            symentry(0x02);		// Put module entry in file
            nextline();			// Skip following blank line
            while (getin(), lastchr == ' ')
            {
                getlabel();
                gethex();		// Get segment value
                segment = (short)value;	// Store segment value
                gethex();		// Get offset value
                symentry(0x20);		// Generate symbol table entry
            }
            getin();
            if (lastchr == 'P')
            {
                nextline();
                nextline();
                nextline();
                getin();
            }
        } while (lastchr == 'L');
    }
    if (fseek(ofile, 4L, 0) != 0)
        fatal(oname);
    temp = tblsize;
    putec(temp);
    putec(temp >> 8);
    putec(temp >> 16);
    putec(temp >> 24);
    putec(symcnt);
    putec(symcnt >> 8);
    putec(symcnt >> 16);
    putec(symcnt >> 24);
    fprintf(stderr, "Symbol table has %ld entries using %ld bytes\n", symcnt,
        temp);
    if (fclose(ofile) == -1)
        fatal(oname);
    if (fclose(ifile) == -1)
        fatal(iname);
    return (0);
}

// Function to advance to line beginning with a specified string

int advance(str)
char *str;

{
    register char *pnt;
    register char  chr;

    for (;;)
    {
        while (lastchr != '\n')
        {
            if(lastchr == '\0')
                return(FALSE);
            getin();
        }
        pnt = str;
        while ((chr = *pnt++) != '\0')
        {
            getin();
            if (lastchr == '\0')
                return(FALSE);
            if (lastchr == '\n' || chr != lastchr)
                break;
        }
        if (chr == '\0')
            return (TRUE);
    }
}

// Function to output bytes to MAP file

void putes(str, cnt)
char *str;
int   cnt;

{
    while (--cnt >= 0)
        putec(*str++);
}

// Function to output symbol table entry to MAP file

void symentry(flagbyte)
char flagbyte;

{
    putec(flagbyte);
    putec(value);
    putec((int)(value) >> 8);
    putec(segment);
    putec(segment >> 8);
    putes(symbol, symsize);
    ++symcnt;
}

// Function to output byte to SYM file

void putec(byte)
char byte;

{
    if (putc(byte, ofile) < 0)
        femsg(prgname, -errno, oname);
    tblsize++;
}

// Function to get symbol field

void getname(void)

{
    register char *pnt;

    symsize = 0;
    pnt = symbol;
    while (getin(), lastchr != ' ' && lastchr != ')' && lastchr != '\n')
    {
        if (symsize >= 32)
            fprintf(stderr, "% GENSYM: Symbol too long, truncated to %s\n",
                    symbol);
        else
        {
            if (lastchr != '\\')
            {
                *pnt++ = lastchr;
                symsize++;
            }
        }
    }
    pnt[-1] |= 0x80;			// Flag last byte as end
    while (lastchr != '\n')
        getin();
}

// Function to get name of line label

void getlabel(void)

{
    register char *pnt;

    symsize = 1;
    symbol[0] = '$';
    pnt = &symbol[1];
    while (getin(), lastchr == ' ')
        ;
    while (lastchr != ' ' && lastchr != '\n')
    {
        if (symsize >= 32)
            fprintf(stderr, "% GENSYM: Line label too long, truncated to %s\n",
                    symbol);
        else
        {
            *pnt++ = lastchr;
            symsize++;
        }
        getin();
    }
    pnt[-1] |= 0x80;			// Flag last byte as end
}

// Function to get value of hex field - value is stored in the global
//   long variable "value"

int gethex(void)

{
    int rtn;

    value = 0;
    rtn = 0;
    do					// Skip leading spaces
    {   getin();
    } while (lastchr == ' ');
    while (isxdigit(lastchr))
    {
        if (lastchr > '9')
            lastchr += 9;
        value *= 16;
        value += (long)(lastchr & 0x0F);
        rtn = 1;
        getin();
    }
    return (rtn);
}

// Function to skip and discard input characters

void skpchrs(cnt)
int cnt;

{
    while (--cnt >= 0)
        getin();
}

// Function to input character

void getin(void)

{
    if ((lastchr=fgetc(ifile)) == -1)
    {
        if (errno == 0 || errno == -ER_EOF) // At EOF?
        {
            lastchr = '\0';		// Yes - return null
            return;
        }
        else
            femsg(prgname, -errno, iname);
    }
    if (lastchr == '\n')
        ++line;
}

// Function to advance to start of next non-blank line

void nextline(void)

{
    do
    {   getin();
    } while (lastchr && lastchr != '\n');
}

// Function to indicate failure while reading MAP file

void badmap(void)

{
    char str[4];

    if (lastchr == '\n')
    {
        str[0] = '\\';
        str[1] = 'n';
        str[2] = '\0';
    }
    else
    {
        str[0] = lastchr;
        str[1] = '\0';
    }
    fprintf(stderr, "? GENSYM: Illegal MAP file format, line %d, char = %s\n",
        line, str);
    exit(1);
}

// Function to report fatal IO error after output file created

void fatal(name)
char *name;

{
    fclose(ofile);			// Make sure output file is closed
    svcIoDelete(0, oname, NULL);	// Try to delete output file
    femsg(prgname, -errno, name);	// Complain and die
}
