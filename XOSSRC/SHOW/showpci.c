//--------------------------------------------------------------------------*
// SHOWPCI.C
// Command to display PCI device configuration
//
// Written by: John R. Goltz
//
//--------------------------------------------------------------------------*

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

// Command format:
//   SHOWPCI

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <XOSTIME.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSRTN.H>
#include <XOSERR.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>

#define VERSION 1
#define EDITNO  0


typedef struct
{	uchar type;
	uchar length;
	char  name[8];
	union
	{	struct
		{	long lownum;
			long hinum;
		};
		struct
		{	char  far *pnt;
			short xxx;
			short size;
			short amount;
		};
        time_s dtv;
        char   ss[8];
    };
} DCHAR;


typedef union
{	char  *c;
	DCHAR *d;
} DPNT;

type_qab charqab =
{
    QFNC_WAIT|QFNC_DEVCHAR,
    0,							// qab_status   - Returned status
    0,							// qab_error    - Error code
    0,							// qab_amount   - Amount
    0,							// qab_handle   - Device handle
    0,							// qab_vector   - Vector for interrupt
    0,	        				// qab_level    - Signal level for direct I/O
    0,  						// qab_prvlevel - Previous signal level (int.)
    CF_SIZE,					// qab_option   - Options or command
    0,							// qab_count    - Count
    NULL, 0,					// qab_buffer1  - Pointer to file spec
    NULL, 0,					// qab_buffer2  - Unused
    NULL, 0						// qab_parm     - Pointer to parameter area
};

char *cctbl[] =
{	"Unspec",		//  0 - Unspecified
	"Storage",		//  1 - Mass storage controller
	"Network",		//  2 - Network controller
	"Display",		//  3 - Display controller
	"Multimedia",	//  4 - Multimedia device
	"Memory",		//  5 - Memory controller
	"Bridge",		//  6 - Bridge device
	"Comm",			//  7 - Simple sommunications controller
	"System",		//  8 - Base system peripherals
	"Input",		//  9 - Input device
	"Docking",		// 10 - Docking station
	"Processor",	// 11 - Processor
	"SerialBus",	// 12 - Serial bus controller
	"Wireless",		// 13 - Wireless controller
	"Intelligent",	// 14 - Intelligent IO controller
	"Satellte",		// 15 - Satellite communications controller
	"Encryption",	// 16 - Encryption/decryption controller
	"DataAcq"		// 17 - Data Acquisition and singal processing controller
};

extern int (*_sprintfpnt)(int, char **);

ulong  parmblk[32];
ulong *ppnt = parmblk+2;
char  *xpfpnt;
char  *buffer;
int    bufmax;
int    outsize;
int    charcnt;
int    chartotal;
int    labelsize;
char  *charpnt;
char  *parmlist;
struct parm    *parmpnt;
struct parmbfr *parmbfr;
struct parmbfr *firstparm;
struct parmbfr *lastparm = (struct parmbfr *)(&firstparm);
long   brief=FALSE;
long   quiet=FALSE;           // TRUE if no output wanted
long   mute=FALSE;           // TRUE if brief output wanted
char   errbfr[PROGASSTEBSZ]; // procarg error buffer


Prog_Info pib;
char  copymsg[] = "";

char prgname[] = "SHOWPCI";
char envname[] = "^XOS^SHOWPCI^OPT";

char  example[] = "{/option}";
char description[] = "This command is used to display the PCI " \
    "device configuration.";


#define AF(func) (int (*)(arg_data *))func

/*
arg_spec keywords[] =
{   {"*" , ASF_LSVAL, NULL, charhave, 0},
    {NULL, 0        , NULL, NULL    , 0}
};
*/

int havebrief( arg_data *arg);

arg_spec options[] =
{
    {"H*ELP"    , 0                 , NULL, AF(opthelp) , 0   ,
            "This message."},
    {"?"        , 0                 , NULL, AF(opthelp) , 0   ,
            "This message."},
    {"V*ERBOSE" , ASF_NEGATE        , NULL,    havebrief, TRUE,
            "Detailed output."},
    {"Q*UIET"   , ASF_BOOL|ASF_STORE, NULL,   &quiet    , TRUE,
            "No output, except error messages."},
    {"M*UTE"    , ASF_BOOL|ASF_STORE, NULL,   &mute     , TRUE,
            "No output, even error messages."},
    {NULL}
};

char *chars;

main(
	int   argc,
	char *argv[])

{
	long   rtn;
	DPNT   cpnts;
	DPNT   cpntd;
    char  *args[2];
	ulong lowval;
	ulong hival;
	int   venid;
	int   devid;
	int   ccval;
	int   scval;
	int   pival;
	int   rev;
	int    length;
	int    type;
	int    bus;
	int    dev;
	int    fnc;
    char   strbuf[512];
	char   ccstr[16];
	char   scstr[16];
	char   pistr[16];

    reg_pib(&pib);

    // Set Program Information Block variables

    pib.opttbl=options; 		// Load the option table
    pib.kwdtbl=NULL;
    pib.build=__DATE__;
    pib.majedt = VERSION; 		// Major edit number
    pib.minedt = EDITNO; 		// Minor edit number
    pib.copymsg=copymsg;
    pib.prgname=prgname;
    pib.desc=description;
    pib.example=example;
    pib.errno=0;
    getTrmParms();
    getHelpClr();

    if(svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
    {
		args[0] = strbuf;
		args[1] = '\0';
		progarg(args, 0, options, NULL, (int (*)(char *))NULL,
				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    if (argc !=0)
		++argv;
	progarg(argv, PAF_EATQUOTE, options, /* keywords */ NULL, (int (*)(char *))NULL,
			(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

    // Here with all arguments processed - now do what we need to do

	if ((charqab.qab_handle = svcIoOpen(0, "_PCI0:", NULL)) < 0)
	{
		if (charqab.qab_handle == ER_NSDEV)
		{
			fputs("? SHOWPCI: PCI support is not installed\n", stderr);
			exit (1);
		}
		else
			femsg2(prgname, "Error opening PCI0: device", charqab.qab_handle,
					NULL);
	}
	if ((rtn = svcIoQueue(&charqab)) < 0 || (rtn = charqab.qab_error) < 0)
		femsg2(prgname, "Error getting PCI0: device characteristics", rtn,
				NULL);
	charqab.qab_buffer2 = (char far *)getspace(charqab.qab_amount + 16);

	charqab.qab_option = CF_ALL;

	if ((rtn = svcIoQueue(&charqab)) < 0 || (rtn = charqab.qab_error) < 0)
		femsg2(prgname, "Error getting PCI0: device characteristics", rtn,
				NULL);

	// Now we have a complete list of the device characteristics for the
	//   PCI0 device.  We now remove everything except for the DEVbbddn
	//   characteristics

	cpnts.c = cpntd.c = (char near *)charqab.qab_buffer2;
	while (--charqab.qab_amount >= 0)
	{
		type = cpnts.d->type & 0x0F;
		length = (type == 13 || type == 15) ? sizeof(DCHAR) :
				cpnts.d->length + 10;

		if (strncmp(cpnts.d->name, "DEV", 3) == 0)
		{
			memcpy(cpntd.c, cpnts.c, length);
			cpntd.d->type |= PAR_GET;
			cpntd.c += length;
		}
		cpnts.c += length;
	}
	*cpntd.c = 0;
	charqab.qab_option = CF_VALUES;
	if ((rtn = svcIoQueue(&charqab)) < 0 || (rtn = charqab.qab_error) < 0)
		femsg2(prgname, "Error getting PCI0: device characteristics", rtn,
				NULL);
	cpnts.c = (char near *)charqab.qab_buffer2;
	fputs("\n  Bus Dev Fnc Vendor Device Class      Sub-class   Interface  "
			"Revision\n", stdout);
	while (cpnts.d->type != 0)
	{
		bus = gethexval(cpnts.d->name + 3, 2);
		dev = gethexval(cpnts.d->name + 5, 2);
		fnc = gethexval(cpnts.d->name + 7, 1);
		lowval = (cpnts.d->length >= 4) ? cpnts.d->lownum : 0;
		hival = (cpnts.d->length >= 8) ? cpnts.d->hinum : 0;
		venid = (ushort)lowval;
		devid = lowval >> 16;
		ccval = hival >> 24;
		scval = (uchar)(hival >> 16);
		pival = (uchar)(hival >> 8);
		rev = (uchar)hival;

		switch (ccval)
		{
		 case 0:						// Unspecified
			ccstr[0] = 0;
			switch (scval)
			{
			 case 0:
				scstr[0] = 0;
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 case 1:
				strcpy(scstr, "VGA");
				if (pival == 1)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "0x%02.2X", pival);
			}
			break;

		 case 1:						// Mass storage controller
			strcpy(ccstr, "Storage");
			switch (scval)
			{
			 case 0:					// SCSI
				strcpy(scstr, "SCSI");
				goto picom1;

			 case 1:					// IDE
				strcpy(scstr, "IDE");
				sprintf(pistr, "0x%02.2X", pival);
				break;

			 case 2:					// Floppy disk
				strcpy(scstr, "Floppy");
				goto picom1;

			 case 3:					// IPI
				strcpy(scstr, "IPI");
				goto picom1;

			 case 4:					// RAID
				strcpy(scstr, "RAID");
			 picom1:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "0x%02.2X", pival);
				break;
			}
			break;

		 case 2:						// Network controller
			strcpy(ccstr, "Network");
			switch (scval)
			{
			 case 0:					// Ethernet
				strcpy(scstr, "Ethernet");
				goto picom2;

			 case 1:					// Token ring
				strcpy(scstr, "TokenRing");
				goto picom2;

			 case 2:					// FDDI
				strcpy(scstr, "FDDI");
				goto picom2;

			 case 3:					// ATM
				strcpy(scstr, "ATM");
				goto picom2;

			 case 4:					// ISDN
				strcpy(scstr, "ISDN");
			 picom2:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "0x%02.2X", pival);
				break;
			}
			break;

		 case 3:						// Display controller
			strcpy(ccstr, "Display");
			switch (scval)
			{
			 case 0:					// PC
				strcpy(scstr, "PC");

				if (pival == 0)
					strcpy(pistr, "VGA");
				else if (pival == 1)
					strcpy(pistr, "8514");
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 case 1:					// XGA
				strcpy(scstr, "XGA");
				goto picom3;

			 case 2:					// 3D
				strcpy(scstr, "3D");
			 picom3:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "0x%02.2X", pival);
				break;
			}
			break;

		 case 4:						// Multimedia device
			strcpy(ccstr, "Multimedia");
			switch (scval)
			{
			 case 0:					// Video
				strcpy(scstr, "Video");
				goto picom4;

			 case 1:					// Audio
				strcpy(scstr, "Audio");
				goto picom4;

			 case 2:					// Telephony
				strcpy(scstr, "Telephony");
			 picom4:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "0x%02.2X", pival);
				break;
			}
			break;

		 case 5:						// Memory controller
			strcpy(ccstr, "Memory");
			switch (scval)
			{
			 case 0:					// RAM
				strcpy(scstr, "RAM");
				goto picom5;

			 case 1:					// Flash
				strcpy(scstr, "Flash");
			 picom5:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "0x%02.2X", pival);
				break;
			}
			break;

		 case 6:						// Bridge device
			strcpy(ccstr, "Bridge");
			switch (scval)
			{
			 case 0:					// Host/PCI
				strcpy(scstr, "Host/PCI");
				goto picom6;

			 case 1:					// PCI/ISA
				strcpy(scstr, "PCI/ISA");
				goto picom6;

			 case 2:					// PCI/EISA
				strcpy(scstr, "PCI/EISA");
				goto picom6;

			 case 3:					// PCI/MicroChannel
				strcpy(scstr, "PCI/MicroCh");
				goto picom6;

			 case 4:					// PCI/PCI
				strcpy(scstr, "PCI/PCI");
				if (pival == 0)
					pistr[0] = 0;
				else if (pival == 1)
					strcpy(pistr, "SubDecode");
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 case 5:					// PCI/PCMCIA
				strcpy(scstr, "PCI/PCMCIA");
				goto picom6;

			 case 6:					// PCI/NuBus
				strcpy(scstr, "PCI/NuBus");
				goto picom6;

			 case 7:					// PCI/CardBus
				strcpy(scstr, "PCI/CardBus");
			 picom6:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 case 8:					// RACEway
				strcpy(scstr, "RACEway");
				sprintf(pistr, "0x%02.2X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "0x%02.2X", pival);
				break;
			}
			break;

		 case 7:						// Simple connumications controller
			strcpy(ccstr, "Comm");
			switch (scval)
			{
			 case 0:					// PC com ports
				strcpy(scstr, "PCcom");
				switch (pival)
				{
				 case 0:				// Generic XT compatible
					strcpy(pistr, "Generic");
					break;

				 case 1:				// 16450
					strcpy(pistr, "16450");
					break;

				 case 2:				// 16550
					strcpy(pistr, "16550");
					break;

				 case 3:				// 16650
					strcpy(pistr, "16650");
					break;

				 case 4:				// 16750
					strcpy(pistr, "16750");
					break;

				 case 5:				// 16850
					strcpy(pistr, "16850");
					break;

				 case 6:				// 16950
					strcpy(pistr, "16950");
					break;

				 default:
					sprintf(pistr, "0x%02.2X", pival);
					break;
				}
				break;

			 case 1:					// Parallel port
				strcpy(scstr, "Parallel");
				switch (pival)
				{
				 case 0:				// Generic PC parallel port
					strcpy(pistr, "Generic");
					break;

				 case 1:				// Bi-directional
					strcpy(pistr, "Bi-dir");
					break;

				 case 2:				// ECP 1.X
					strcpy(pistr, "ECP 1.X");
					break;

				 case 3:				// IEEE 1284 controller
					strcpy(pistr, "1284 cont");
					break;

				 case 0xFE:				// IEEE 1284 target
					strcpy(pistr, "1284 tar");
					break;

				 default:
					sprintf(pistr, "0x%02.2X", pival);
					break;
				}
				break;

			 case 2:					// Multiport serial controller
				strcpy(scstr, "Multiport");
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 case 3:					// Modem
				strcpy(scstr, "Modem");
				switch (pival)
				{
				 case 0:				// Generic modem
					strcpy(pistr, "Generic");
					break;

				 case 1:				// Hayes/16450
					strcpy(pistr, "Hayes16450");
					break;

				 case 2:				// Hayes/16550
					strcpy(pistr, "Hayes16550");
					break;

				 case 3:				// Hayes/16650
					strcpy(pistr, "Hayes16650");
					break;

				 case 0xFE:				// Hayes/16750
					strcpy(pistr, "Hayes16750");
					break;

				 default:
					sprintf(pistr, "0x%02.2X", pival);
					break;
				}
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "0x%02.2X", pival);
				break;
			}
			break;

		 case 8:						// Base system peripherals
			strcpy(ccstr, "System");
			switch (scval)
			{
			 case 0:					// Interrupt controller
				strcpy(scstr, "PIC");
				switch (pival)
				{
				 case 0:				// Generic
					strcpy(pistr, "Generic");
					break;

				 case 1:				// ISA
					strcpy(pistr, "ISA");
					break;

				 case 2:				// EISA
					strcpy(pistr, "EISA");
					break;

				 case 10:				// IO APIC
					strcpy(pistr, "IO APIC");
					break;

				 case 20:				// IO(x) APIC
					strcpy(pistr, "IO(x) APIC");
					break;

				 default:
					sprintf(pistr, "0x%02.2X", pival);
					break;
				}
				break;

			 case 1:					// DMA controller
				strcpy(scstr, "DMA");
				switch (pival)
				{
				 case 0:				// Generic DMA controller
					strcpy(pistr, "Generic");
					break;

				 case 1:				// ISA
					strcpy(pistr, "ISA");
					break;

				 case 2:				// EISA
					strcpy(pistr, "EISA");
					break;

				 default:
					sprintf(pistr, "0x%02.2X", pival);
					break;
				}
				break;

			 case 2:					// 8237 timer
				strcpy(scstr, "Timer");
				switch (pival)
				{
				 case 0:				// Generic timer
					strcpy(pistr, "Generic");
					break;

				 case 1:				// ISA
					strcpy(pistr, "ISA");
					break;

				 case 2:				// EISA
					strcpy(pistr, "EISA");
					break;

				 default:
					sprintf(pistr, "0x%02.2X", pival);
					break;
				}
				break;

			 case 3:					// RTC
				strcpy(scstr, "RTC");
				switch (pival)
				{
				 case 0:				// Generic RTC
					strcpy(pistr, "Generic");
					break;

				 case 1:				// ISA
					strcpy(pistr, "ISA");
					break;

				 default:
					sprintf(pistr, "0x%02.2X", pival);
					break;
				}
				break;

			 case 4:					// PCI Hot-Plug controller
				strcpy(scstr, "HotPlug");
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "0x%02.2X", pival);
				break;
			}
			break;

		 case 9:						// Input device
			strcpy(ccstr, "Input");
			switch (scval)
			{
			 case 0:					// Keyboard
				strcpy(scstr, "Keyboard");
				goto picom9;

			 case 1:					// Digitizer
				strcpy(scstr, "Digitizer");
				goto picom9;

			 case 2:					// Mouse
				strcpy(scstr, "Mouse");
				goto picom9;

			 case 3:					// Scanner
				strcpy(scstr, "Scanner");
			 picom9:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 case 4:					// Gameport
				strcpy(scstr, "GamePort");
				if (pival == 0)
					pistr[0] = 0;
				else if (pival == 16)
					strcpy(pistr, "Legacy");
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "0x%02.2X", pival);
				break;
			}
			break;

		 case 10:						// Docking station
			strcpy(ccstr, "Docking");
			switch (scval)
			{
			 case 0:					// Generic
				strcpy(scstr, "Generic");
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 case 4:					// Gameport
				strcpy(scstr, "GamePort");
				if (pival == 0)
					pistr[0] = 0;
				else if (pival == 16)
					strcpy(pistr, "Legacy");
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "0x%02.2X", pival);
				break;
			}
			break;

		 case 11:						// Processor
			strcpy(ccstr, "Processor");
			switch (scval)
			{
			 case 0:					// 80386
				strcpy(scstr, "80386");
				goto picom11;

			 case 1:					// 80486
				strcpy(scstr, "80486");
				goto picom11;

			 case 2:					// Pentinum
				strcpy(scstr, "Pentium");
				goto picom11;

			 case 16:					// Alpha
				strcpy(scstr, "Alpha");
				goto picom11;

			 case 32:					// Power PC
				strcpy(scstr, "Power PC");
				goto picom11;

			 case 48:					// MIPS
				strcpy(scstr, "MIPS");
			 picom11:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "0x%02.2X", pival);
				break;
			}
			break;

		 case 12:						// Serial bus controller
			strcpy(ccstr, "SerialBus");

			switch (scval)
			{
			 case 0:					// FireWire (IEEE 1394)
				strcpy(scstr, "FireWire");
				switch (pival)
				{
				 case 0:				// Not specified
					pistr[0] = 0;
					break;

				 case 32:				// 1394 OpenHCI spec
					strcpy(pistr, "OpenHCI");
					break;

				 default:
					sprintf(pistr, "0x%02.2X", pival);
					break;
				}
				break;

			 case 1:					// ACCESS bus
				strcpy(scstr, "ACCESS");
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 case 2:					// SSA (Serial Storage Architecture)
				strcpy(scstr, "SSA");
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 case 3:					// USB
				strcpy(scstr, "USB");
				switch (pival)
				{
				 case 0x00:				// UHC spec
					strcpy(pistr, "UHC");
					break;

				 case 0x10:				// OHC spec
					strcpy(pistr, "OHC");
					break;

				 case 0x20:				// EHC spec
					strcpy(pistr, "EHC");
					break;

				 case 128:
					pistr[0] = 0;
					break;

				 default:
					sprintf(pistr, "0x%02.2X", pival);
					break;
				}
				break;

			 case 4:					// Fibre channel
				strcpy(scstr, "FIBRE");
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 case 5:					// SMBus (System Management Bus)
				strcpy(scstr, "SMBus");
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;
			}
			break;

		 case 13:						// Wireless controller
			strcpy(ccstr, "Wireless");
			switch (scval)
			{
			 case 0:					// iRDA compatible
				strcpy(scstr, "iRDA");
				goto picom13;

			 case 1:					// Consumer IR controller
				strcpy(scstr, "ConsumerIR");
				goto picom13;

			 case 16:					// RF controller
				strcpy(scstr, "RF");
			 picom13:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "0x%02.2X", pival);
				break;
			}
			break;

		 case 14:						// Intelligent IO controller
			strcpy(ccstr, "Intelligent");
			switch (scval)
			{
			 case 0:					// I2O
				strcpy(scstr, "I2O");

			 default:
				sprintf(scstr, "%d", scval);
				break;
			}
			sprintf(pistr, "0x%02.2X", pival);
			break;

		 case 15:						// Satellite communications controller
			strcpy(ccstr, "Satellte");
			switch (scval)
			{
			 case 1:					// TV
				strcpy(scstr, "TV");
				goto picom15;

			 case 2:					// Audio
				strcpy(scstr, "Audio");
				goto picom15;

			 case 3:					// Voice
				strcpy(scstr, "Voice");
				goto picom15;

			 case 4:					// Data
				strcpy(scstr, "Data");
				goto picom15;
			 picom15:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "0x%02.2X", pival);
				break;
			}
			break;

		 case 16:						// Encryption/decryption controller
			strcpy(ccstr, "Encryption");
			switch (scval)
			{
			 case 0:					// Data
				strcpy(scstr, "Data");
				goto picom16;

			 case 1:					// Entertainment
				strcpy(scstr, "Entrtnmnt");
			 picom16:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "0x%02.2X", pival);
				break;
			}
			break;

		 case 17:						// Data Acquisition and singal
										//   processing controller
			strcpy(ccstr, "DataAcq");
			switch (scval)
			{
			 case 0:					// DPIO
				strcpy(scstr, "DPIO");
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "0x%02.2X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "0x%02.2X", pival);
				break;
			}
			break;

		 default:
			sprintf(ccstr, "%d", ccval);
			sprintf(scstr, "%d", scval);
			sprintf(pistr, "0x%02.2X", pival);
			break;
		}

		printf("  %-3d %-3d %-3d %04.4X   %04.4X   %-10s %-11s %-10s %d\n",
				bus, dev, fnc, venid, devid, ccstr, scstr, pistr, rev);

		cpnts.c += (cpnts.d->length + 10);
	}
}

int havebrief(
    arg_data *arg)

{
    brief = ((arg->flags & ASF_NEGATE) != 0);
    return (TRUE);
}


int gethexval(
	char *str,
	int   len)

{
	int  value;
	char chr;

	value = 0;
	while (--len >= 0)
	{
		if ((chr = *str++) >= 'A')
			chr += 9;
		value = value * 16 + (chr & 0x0F);
	}
	return (value);

}
