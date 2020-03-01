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

#include <XOSSTR.H>
#include <XOS.H>
#include <XOSERMSG.H>
#include <XOSSVC.H>


long   rtn;
long   pcc;
uchar *pnt;
uchar *data;
char  *socstr;
int    socket;
int    cnt;
int    type;
int    size;
int    length;
uchar  buffer[255];
char   prgname[] = "CISDUMP";

uchar  cisdata[1024];

struct
{   byte1_char  socket;
    lngstr_char cisdata;
    char        end;
} chrlist =
{   {PAR_SET|REP_DECV , 1, "SOCKET" , 0},
    {PAR_GET|REP_DATAS, 0, "CISDATA", cisdata, 1024, 1024}
};

char undefined[] = "???";

char *tuplename[] =
{   "NULL",		// 00 - Null tuple
    "DEVICE",		// 01 - Device information tuple for common memory
    undefined,		// 02 - Undefined tuple
    undefined,		// 03 - Undefined tuple
    undefined,		// 04 - Undefined tuple
    undefined,		// 05 - Undefined tuple
    undefined,		// 06 - Undefined tuple
    undefined,		// 07 - Undefined tuple
    undefined,		// 08 - Undefined tuple
    undefined,		// 09 - Undefined tuple
    undefined,		// 0A - Undefined tuple
    undefined,		// 0B - Undefined tuple
    undefined,		// 0C - Undefined tuple
    undefined,		// 0D - Undefined tuple
    undefined,		// 0E - Undefined tuple
    undefined,		// 0F - Undefined tuple
    "CHECKSUM",		// 10 - Checksum control tuple - contains checksum
			//	  for specified area of CIS
    "LONGLINK_A",	// 11 - Long link control tuple for attribute memory
    "LONGLINK_C",	// 12 - Long link control tuple for common memory
    "LINKTARGET",	// 13 - Long link target control tuple
    "NO_LINK",		// 14 - No link control tuple
    "VERS_1",		// 15 - Level 1 version/product information tuple
    "ALTSTR",		// 16 - Alternate language string tuple
    "DEVICE_A",		// 17 - Device information tuple for attribute memory
    "JEDEC_C",		// 18 - JEDEC identifier for common memory
    "JEDEC_A",		// 19 - JEDEC identifier for attribute memory
    "CONFIG",		// 1A - Configuration tuple
    "CFTABLE_ENTRY",	// 1B - Configuration tuple entry
    "DEVICE_OC",	// 1C - Operating conditions for common memory
    "DEVICE_AC",	// 1D - Operating conditions for attribute memory
    "DEVICE_GEO_C",	// 1E - Device geometry info tuple for common memory
    "DEVICE_GEO_A",	// 1F - Device geometry info tuple for attribute memory
    "MANFID",		// 20 - Manufacturer identification
    "FUNCID",		// 21 - Function identificatin tuple
    "FUNCE",		// 22 - Function extension tuple
    "SWIL",		// 23 - Software interleave tuple
    undefined,		// 24 - Undefined tuple
    undefined,		// 25 - Undefined tuple
    undefined,		// 26 - Undefined tuple
    undefined,		// 27 - Undefined tuple
    undefined,		// 28 - Undefined tuple
    undefined,		// 29 - Undefined tuple
    undefined,		// 2A - Undefined tuple
    undefined,		// 2B - Undefined tuple
    undefined,		// 2C - Undefined tuple
    undefined,		// 2D - Undefined tuple
    undefined,		// 2E - Undefined tuple
    undefined,		// 2F - Undefined tuple
    undefined,		// 30 - Undefined tuple
    undefined,		// 31 - Undefined tuple
    undefined,		// 32 - Undefined tuple
    undefined,		// 33 - Undefined tuple
    undefined,		// 34 - Undefined tuple
    undefined,		// 35 - Undefined tuple
    undefined,		// 36 - Undefined tuple
    undefined,		// 37 - Undefined tuple
    undefined,		// 38 - Undefined tuple
    undefined,		// 39 - Undefined tuple
    undefined,		// 3A - Undefined tuple
    undefined,		// 3B - Undefined tuple
    undefined,		// 3C - Undefined tuple
    undefined,		// 3D - Undefined tuple
    undefined,		// 3E - Undefined tuple
    undefined,		// 3F - Undefined tuple

    "VERS_2",		// 40 - Level 2 version tuple
    "FORMAT",		// 41 - Format tuple
    "GEOMETRY",		// 42 - Geometry tuple
    "BYTEORDER",	// 43 - Byte order tuple
    "DATE",		// 44 - Card initialization date tuple
    "BATTERY",		// 45 - Battery replacement date tuple
    "ORG",		// 46 - Organization tuple
};

#define MAXNAME (sizeof(tuplename)/sizeof(char *) - 1)


void dumpcfgtbl(void);
void dumpconfig(void);
void dumpdata(uchar *data, int length);
void dumpiospace(void);
void dumpirq(void);
void dumpmemory(void);
void dumpmisc(void);
void dumppower(char *label);
void dumppowval(char *label, int unit);
void dumptiming(void);
int  get2bytes(void);
int  getbyte(void);
int  getvalue(int len);
void truncated(void);



void main(
    int   argc,
    char *argv[])

{
    if (argc != 3)
    {
        fputs("? Command error, usage is:\n"
              "    CISDUMP pccdev: socket\n", stderr);
        exit(1);
    }

    socstr = argv[2];
    if (socstr[1] == 0 && isalpha(socstr[0])) // Is socket a single letter?
	chrlist.socket.value = toupper(socstr[0]) - 'A';
    else if (isdigit(socstr[0]))
	chrlist.socket.value = atoi(socstr);
    else
	fputs("? CISDUMP: Illegal socket specified\n", stderr);

    if ((pcc = svcIoOpen(0, argv[1], NULL)) < 0)
        femsg(prgname, pcc, "Error opening PCC device");

    if ((rtn = svcIoDevChar(pcc, &chrlist)) < 0)
        femsg(prgname, rtn, "Error getting device characteristics");


    pnt = cisdata;
    cnt = chrlist.cisdata.strlen;
    do
    {
	if (--cnt < 0)
	    truncated();
	if ((type = *pnt++) == 0xFF)
	{
	    fputs("\nType = 0xFF (END)\n", stdout);
	    if (cnt > 0)
		printf("Have %d byte%s after END tuple\n", cnt, (cnt > 1) ?
			"s" : "");
	    exit(0);
	}
	if (--cnt < 0)
	    truncated();
	if ((size = *pnt++) > 0)
	{
	    if ((cnt -= size) < 0)
		truncated();

	    printf("\nType = 0x%02.2X (%s), length = %d\n", type,
		    (type >= 0 && type <= MAXNAME) ? tuplename[type] :
		    undefined, size);

	    dumpdata(pnt, size);

	    data = pnt;
	    length = size;

	    switch (type)
	    {
	     case 0x15:			// VERS_1 tuple
		dumpvers1();
		break;

	     case 0x1A:			// CONFIG tuple
		dumpconfig();
		break;

	     case 0x1B:			// CFTABLE_ENTRY tuple
		dumpcfgtbl();
		break;

	     case 0x20:			// MANFID tuple
		dumpmanfid();
		break;
	     case 0x21:			// FUNCID tuple
		dumpfuncid();
		break;
	    }
	    pnt += size;
	}
    } while (cnt > 0);
    exit(0);
}

//**************************************************
// Function: dumpvers1- Dump the VERS_1 (0x15) tuple
// Returned: Nothing
//**************************************************

void dumpvers1()

{
    char *pnt;
    int   val;
    int   major;
    int   minor;
    char  buffer[132];

    if ((major = getbyte()) < 0)
	return;
    if ((minor = getbyte()) < 0)
	return;
    printf("  Version: %d.%d\n", major, minor);
    for (;;)
    {
	if ((val = getbyte()) < 0)
	    return;
	if (val == 0xFF)
	    break;
	pnt = buffer;
	while (val != 0)
	{
	    if (pnt < (buffer + 128))
		*pnt++ = val;
	    if ((val = getbyte()) < 0)
		return;
	}
	*pnt = 0;
	printf("    %s\n", buffer);
    }

}

//****************************************************
// Function: dumpconfig - Dump the CONFIG (0x1A) tuple
// Returned: Nothing
//****************************************************

void dumpconfig(void)

{
    int   val;
    int   value;
    int   cnt;
    int   radrbytes;
    int   rmskbytes;
    char *pnt;
    char  line[12];

    if ((val = getbyte()) < 0)
	return;
    radrbytes = val & 0x03;
    rmskbytes = (val >> 2) & 0x0F;

    if ((val = getbyte()) < 0)
	return;
    printf("  Last entry in CCT: 0x%02.2X\n", val & 0x3F);

    pnt = (char *)&value;
    value = 0;
    do
    {
	if ((val = getbyte()) < 0)
	    return;
	*pnt++ = val;
    } while (--radrbytes >= 0);
    printf("  CFG Reg: 0x%X\n  Register map:\n", value);
    do
    {
	if ((val = getbyte()) < 0)
	    return;

	cnt = 8;
	pnt = line;
	do
	{
	    *pnt++ = (val & 0x01) ? 'X' : '.';
	    val >>= 1;
	} while (--cnt > 0 && val != 0);
	*pnt = 0;
	printf("    %s\n", line);
    } while (--radrbytes >= 0);

}

//***********************************************************
// Function: dumpcfgtbl - Dump the CFTABLE_ENTRY (0x1B) tuple
// Returned: Nothing
//***********************************************************

void dumpcfgtbl(void)

{
    static char rsrvd[] = "Reserved";
    static char custom[] = "Custom";
    static char *iftbl[] =
    {	"Memory",		// 00
	"I/O",			// 01
	rsrvd,			// 02
	rsrvd,			// 03
	custom,			// 04
	custom,			// 05
	custom,			// 06
	custom,			// 07
	rsrvd,			// 08
	rsrvd,			// 09
	rsrvd,			// 0A
	rsrvd,			// 0B
	rsrvd,			// 0C
	rsrvd,			// 0D
	rsrvd,			// 0E
	rsrvd			// 0F
    };

    int val;
    int value;
    int options;

    // First byte is always there and specifies if the second byte is there

    if ((val = getbyte()) < 0)
	return;
    if (val & 0x40)
	fputs("  Default values\n", stdout);
    printf("  CCReg value: 0x%02.2X\n", val & 0x3F);
    if (val & 0x80)			// Is the second byte there?
    {
	if ((val = getbyte()) < 0)	// Yes - get it
	    return;
	printf("  Interface config: %s\n    BVD1 & BVD2 active: %s\n"
		"    Write protect active: %s\n    +RDY/-BSY active: %s\n"
		"    Wait sig supported for memory: %s\n", iftbl[val & 0xF],
		(val & 0x10) ? "Yes" : " No", (val & 0x20) ? "Yes" : " No",
		(val & 0x40) ? "Yes" : " No", (val & 0x80) ? "Yes" : " No");
    }

    // "Third" byte is always there and which of the following items are there

    if ((options = getbyte()) < 0)
	return;

    switch (options & 0x03)		// Dispatch on power options
    {
     case 0x01:				// Vcc description only
	dumppower("Vcc");
	break;

     case 0x02:				// Vcc and Vpp descriptions
	dumppower("Vcc");
	dumppower("Vpp");
	break;

     case 0x03:				// Vcc, Vpp1, and Vpp2 descriptions
	dumppower("Vcc");
	dumppower("Vpp1");
	dumppower("Vpp2");
	break;
    }

    if (options & 0x04)			// Have timing description?
	dumptiming();			// Yes
    if (options & 0x08)			// Have IO space description?
	dumpiospace();			// Yes
    if (options & 0x10)			// Have IRQ description?
	dumpirq();			// Yes
    switch (options & 0x60)		// Dispatch on memory options
    {
     case 0x20:				// Single 2 byte length specified

	if ((value = getbyte()) < 0)
	    return;
	if ((val = getbyte()) < 0)
	    return;
	printf("  Memory: Length: 0x%04.4X\n", value + (val << 8));
	break;

     case 0x40:				// 2 byte length followed by 2 byte
	if ((value = getbyte()) < 0)	//   card address specified
	    return;
	if ((val = getbyte()) < 0)
	    return;
	printf("    Memory: Length: 0x%04.4X\n", value + (val << 8));
	if ((value = getbyte()) < 0)
	    return;
	if ((val = getbyte()) < 0)
	    return;
	printf("         Address: 0x%04.4X\n", value + (val << 8));
	break;

     case 0x60:				// Have description structure
	dumpmemory();
	break;
    }
    if (options & 0x80)			// Have misc description?
	dumpmisc();			// Yes

}

//********************************************************************
// Function: dumpiospace - Dump I/O space data for CFTABLE_ENTRY tuple
// Returned: Nothing
//********************************************************************

void dumpiospace(void)

{
    int val;
    int cnt;
    int adrsize;
    int lensize;
    int address;
    int length;

    if ((val = getbyte()) < 0)
	return;
    printf("  I/O space: lines decoded: %d\n", val & 0x1F);
    switch (val & 0x60)
    {
     case 0x20:
	fputs("    8-bit I/O only\n", stdout);
	break;

     case 0x40:
	fputs("    8/16-bit I/O, 8-bit access to 16-bit regs NOT supported\n",
		stdout);
	break;

     case 0x60:
	fputs("    8/16-bit I/O, 8-bit access to 16-bit regs supported\n",
		stdout);
	break;
    }
    if (val & 0x80)
    {
	if ((val = getbyte()) < 0)
	    return;
	cnt = val & 0x0F;
	adrsize = (val >> 4) & 0x03;
	lensize = val >> 6;
	do
	{
	    if ((address = getvalue(adrsize)) < 0)
		return;
	    if ((length = getvalue(lensize)) < 0)
		return;
	    printf("    I/O regs @ 0x%X (len = 0x%X)\n", address, length + 1);
	} while (--cnt >= 0);
    }
}

//**********************************************************
// Function: dumpirq - Dump IRQ data for CFTABLE_ENTRY tuple
// Returned: Nothing
//**********************************************************

void dumpirq(void)

{
    char *pnt;
    int   val;
    int   irq;
    int   tmp;
    char  buffer[20];
    char  num;

    if ((val = getbyte()) < 0)
	return;
    if (val & 0x10)			// Have mask bits?
    {
	printf("  IRQ: NMI:  %d\n       IOCK: %d\n       BERR: %d\n"
		"       VEND: %d\n", val & 0x01, (val >> 1) & 0x01,
		(val >> 2) & 0x01, (val >> 3) & 0x01);

	if ((irq = getbyte()) < 0)
	    return;
	if ((tmp = getbyte()) < 0)
	    return;
	irq += (tmp << 8);

	pnt = buffer;
	num = '0';
	do
	{
	    *pnt++ = (irq & 0x01) ? num : '.';
	    irq >>= 1;
	    if (num == '9')
		num = 'A';
	    else
		++num;
	} while (num != 'G');
	*pnt = 0;
	printf("    %s\n", buffer);
    }
    else
	printf("    IRQ: %X\n", val & 0x0F);
    printf("    Level mode: %s\n    Pulse mode: %s\n    Shared: %s\n",
	    (val & 0x20) ? "Yes" : "No", (val & 0x40) ? "Yes" : "No",
	    (val & 0x80) ? "Yes" : "No");
}

//****************************************************************
// Function: dumpmemory - Dump memory data for CFTABLE_ENTRY tuple
// Returned: Nothing
//****************************************************************

void dumpmemory(void)

{
    fputs("  MEMORY STUFF\n", stdout);
}

//************************************************************
// Function: dumpmisc - Dump misc data for CFTABLE_ENTRY tuple
// Returned: Nothing
//************************************************************

void dumpmisc(void)

{
    int val;

    if ((val = getbyte()) < 0)
	return;

    printf("  Misc: Max twin cards: %d\n    Audio: %s\n    Read only: %s\n"
	    "    Power down: %s\n", val & 0x07, (val & 0x08) ? "Yes" : "No",
	    (val & 0x10) ? "Yes" : "No", (val & 0x20) ? "Yes" : "No");

    if (val & 0x80)
    {
	if ((val = getbyte()) < 0)
	    return;
	printf("    Extension byte: %02.2X\n", val);
    }
}

//**************************************************************
// Function: dumppower - Dump power data for CFTABLE_ENTRY tuple
// Returned: Nothing
//**************************************************************

void dumppower(
    char *label)

{
    int bits;

    if ((bits = getbyte()) < 0)
	return;
    printf("  %s description:\n", label);

    if (bits & 0x01)
	dumppowval("Nominal supply voltage", 'V');
    if (bits & 0x02)
	dumppowval("Minimum supply voltage", 'V');
    if (bits & 0x04)
	dumppowval("Maximum supply voltage", 'V');
    if (bits & 0x08)
	dumppowval("Continuous supply current", 'A');
    if (bits & 0x10)
	dumppowval("Maximum current (1 sec ave)", 'A');
    if (bits & 0x20)
	dumppowval("Maximum current (10 ms ave)", 'A');
    if (bits & 0x40)
	dumppowval("Power down supply current", 'A');
}

//*********************************************************************
// Function: dumppowval - Dump power data value for CFTABLE_ENTRY tuple
// Returned: Nothing
//*********************************************************************

void dumppowval(
    char *label,
    int   unit)

{
    static int mantissa[] =
    {	1000,			// 00
	1200,			// 01
	1300,			// 02
	1500,			// 03
	2000,			// 04
	2500,			// 05
	3000,			// 06
	3500,			// 07
	4000,			// 08
	4500,			// 09
	5000,			// 0A
	5500,			// 0B
	6000,			// 0C
	7000,			// 0D
	8000,			// 0E
	9000			// 0F
    };

    static int mult[] =		// V   A
    {	100,			//     00
	1  ,			//     01
	10 ,			// 00  02
	100,			// 01  03
	1  ,			// 02  04
	10 ,			// 03  05
	100,			// 04  06
	1  ,			// 05  07
	10 ,			// 06
	100			// 07
    };

    static char *letter[] =	// V   A
    {	"n",			//     00
	"u",			//     01
	"u",			// 00  02
	"u",			// 01  03
	"m",			// 02  04
	"m",			// 03  05
	"m",			// 04  06
	"" ,			// 05  07
	"" ,			// 06
	""			// 07
    };

    int val;
    int value;
    int manval;
    int expidx;

    uchar extval;
    uchar nopwrdn;
    uchar zeroreq;
    uchar noconn;

    if ((val = getbyte()) < 0)
	return;

    manval = mantissa[(val >> 3) & 0x0F];
    expidx = val & 0x07;
    if (unit == 'V')
	expidx += 2;

    extval = nopwrdn = zeroreq = noconn = FALSE;

    while (val & 0x80)			// Need another value byte?
    {
	if ((val = getbyte()) < 0)
	    return;
	value = val & 0x7F;
	if (value < 0x64 && !extval)
	{
	    extval = TRUE;
	    manval += value;
	}
	else if (value == 0x7D)
	    nopwrdn = TRUE;
	else if (value == 0x7E)
	    zeroreq = TRUE;
	else if (value == 0x7F)
	    noconn = TRUE;
    }

    printf("    %s\n", label);
    if (noconn)
	fputs("      No connection required\n", stdout);
    else if (zeroreq)
	fputs("      Zero value required\n", stdout);
    else
    {
	if (noconn)
	    fputs("      No connection required during power down\n", stdout);
	manval *= mult[expidx];
	printf("      Value: %d.%04.4d%s%c\n", manval/10000, manval % 10000,
		letter[expidx], unit);

    }
}

//****************************************************************
// Function: dumptiming - Dump timing data for CFTABLE_ENTRY tuple
// Returned: Nothing
//****************************************************************

void dumptiming(void)

{
    fputs("  TIMING STUFF\n", stdout);
}

//*********************************************
// Function: dumpmanfid - Dump the MANFID tuple
// Returned: Nothing
//*********************************************

void dumpmanfid(void)

{
    int val;

    if ((val = get2bytes()) < 0)
	return;
    printf("  PCMCIA ID: %04.4X\n", val);
    if ((val = get2bytes()) < 0)
	return;
    printf("  OEM info:  %04.4X\n", val);
}

//*********************************************
// Function: dumpfuncid - Dump the FUNCID tuple
// Returned: Nothing
//*********************************************

void dumpfuncid(void)

{
    int val;

    static char *names[] =
    {	"Multifunction card",
	"Memory card",
	"Serial port/modem",
	"Parallel port",
	"Fixed disk",
	"Video adapter",
	"Network/LAN adapter",
	"AIMS"
    };
    if ((val = getbyte()) < 0)
	return;
    if (val <= 7)
	printf("  Card type: %s\n", names[val]);
    else
	printf("  Card type: 0x%02.2X (unknown type)\n");
    if ((val = getbyte()) < 0)
	return;
    printf("  POST should initialize card: %s\n  Expansion ROM: %s\n",
	    (val & 0x01) ? "Yes" : "No", (val & 0x02) ? "Yes" : "No");
}

//************************************************
// Function: dumpdata - Dump data as hex and ascii
// Returned: Nothing
//************************************************

void dumpdata(
    uchar *data,
    int    length)

{
    int   ofs;
    int   cnt;
    char *pnt1;
    char *pnt2;
    char  line1[50];
    char  line2[30];
    uchar chr;

    ofs = 0;
    do
    {
	pnt1 = line1;
	pnt2 = line2;
	cnt = 16;
	do
	{
	    if (--length < 0)
		break;
	    chr = *data++;
	    sprintf(pnt1, " %02.2X", chr);
	    pnt1 += 3;
	    if (chr < ' ' || chr == 0xFF)
		chr = '.';
	    *pnt2++ = chr;
	} while (--cnt > 0);
	*pnt1 = 0;
	*pnt2 = 0;
	printf("    %02.2X:%-48s |%s|\n", ofs, line1, line2);
	ofs += 16;
    } while (length > 0);
}

int getvalue(
    int len)

{
    int rtn;
    int tmp;

    switch (len)
    {
     case 1:
	return (getbyte());

     case 2:
	if ((rtn = getbyte()) < 0)
	    return (-1);
	if ((tmp = getbyte()) < 0)
	    return (-1);
	return ((tmp << 8) + rtn);

     case 3:
	if ((rtn = getbyte()) < 0)
	    return (-1);
	if ((tmp = getbyte()) < 0)
	    return (-1);
	rtn += (tmp << 8);
	if ((tmp = getbyte()) < 0)
	    return (-1);
	rtn += (tmp << 16);
	if ((tmp = getbyte()) < 0)
	    return (-1);
	return ((tmp << 24) + rtn);
    }
    return (0);
}

//*******************************************************************
// Function: get2bytes - Get next 2 bytes (16-bit value) for tuple
// Returned: Value (0 extended) or -1 if at end of tuple
//*******************************************************************

int get2bytes(void)

{
    int val1;
    int val2;

    if ((val1 = getbyte()) < 0)
	return (-1);
    if ((val2 = getbyte()) < 0)
	return (-1);
    return ((val2 << 8) + val1);
}

//*******************************************************************
// Function: getbyte - Get next byte for tuple
// Returned: Value of next byte (0 extended) or -1 if at end of tuple
//*******************************************************************

int getbyte(void)

{
    if (--length >= 0)
	return (*data++);
    fputs("**** Tuple data truncated\n", stdout);
    return (-1);
}

//*******************************************************
// Function: truncated - Report error for truncated tuple
// Returned: Never returns
//*******************************************************

void truncated(void)

{
    fputs("? Data tuple truncated\n", stdout);
    exit(1);
}
