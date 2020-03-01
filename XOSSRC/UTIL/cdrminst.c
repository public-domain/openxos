// This program is a stripped down version of DISKINST intended to be run
//   runing initial startup to load the necessary drivers when booting from
//   a CDROM.  The following disks are installed:
//	IDE:	Primary master disk as D0:, primary slave disk as D1:,
//		secondary master disk as D2:, secondary slave disk as D3:.
//		Disk information is obtained from the disks if possible or
//		from the CMOS data for the primary disks if not available
//		from the disk.  Disks on the scondary controller must
//		provide information.  Disks may be fixed, removable, or
//		CDROM.
//   Not floppy disk or SCSI drivers or disk units are installed.
// All necessary LKEs are loaded if they are not already loaded.  DOS drive
//   letters are not defined.

// WARNING: Since this program probes looking for devices, it should only
//	    be run when the system is known to be idle otherwise.  Normally,
//	    it should only be run as part of the system startup sequence.
//	    If any of the devices accessed by this program are active when
//	    it is run, serious errors, including system crashes or data loss,
//	    may result!

#include <CTYPE.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <TIME.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <XOSERR.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSTIME.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>
#include <XOSSINFO.H>
#include "LKELOAD.H"


#define VERSION 1
#define EDITNO  0


#define  FAILED -2
#define  HAVE   -1
#define  NEED    1
#define  NONE    0

// Define bits for controller checks (these must match the bits defined in
//   the DISKCHK LKE) - The FLPY0 and FLPY1 bits are not used by the LKE,
//   they will never be cleared.

#define IDE0   0x0001
#define IDE1   0x0002
#define IDE2   0x0004
#define IDE3   0x0008

extern char    lkename[];
extern LKEARGS lkeargs;

lkeinfo_data *lkelist;

int  scsihndl;

char buffer[1024];

struct
{   byte4_char  mask;
    lngstr_char result;
    uchar       end;
} dcchars =
{   {PAR_SET|REP_HEXV, 4, "MASK"  , IDE0|IDE1|IDE2|IDE3},
    {PAR_GET|REP_STR , 0, "RESULT", 0, 0, 1024, 1024}
};

struct
{   lngstr_char result;
    uchar       end;
} lkechars =
{   {PAR_GET|REP_STR, 0, "RESULT", 0, 0, 1024, 1024}
};

char diskclass[] = "DISK:";

QAB addqab =
{   QFNC_WAIT|QFNC_CLASSFUNC,	// qab_func     - Function
    0,				// qab_status   - Returned status
    0,				// qab_error    - Error code
    0,				// qab_amount   - Amount
    0,				// qab_handle   - Device handle
    0,				// qab_vector   - Vector for interrupt
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    CF_ADDUNIT,			// qab_option   - Options or command
    0,				// qab_count    - Count
    diskclass, 0,		// qab_buffer1  - Pointer to file spec
    NULL, 0,			// qab_buffer2  - Pointer to characteristics
    NULL, 0			// qab_parm     - Pointer to parameters
};

Prog_Info pib;

int   quiet;

char  prgname[] = "CDRMINST";

char  idelke;
char  cdromlke;
char  ideunit[4];
uchar reqcdrom = FALSE;

char lkeerrmsg[] = "Cannot obtain list of installed LKEs";

arg_spec options[] =
{
    {"Q*UIET" ,ASF_BOOL|ASF_STORE, NULL,   &quiet   , TRUE,
	    "Only output error messages." },
    {"?"      , 0,                 NULL, AF(opthelp), 0,
	    "Display this message." },
    {"H*ELP"  , 0,                 NULL, AF(opthelp), 0,
	    "Display this message." },
    {NULL}
};

char  example[] = "{/option}";
char  description[] = "This command should only be issued during startup to "
    "load the necessary drivers when booting from a CDROM.";

int main(
    int   argc,
    char *argv[])

{
    lkeinfo_data *lkepnt;
    int           rtn;

    // Set program information block variables

    reg_pib(&pib);
    pib.opttbl = options; 		// Load the option table
    pib.kwdtbl = NULL;
    pib.build = __DATE__;
    pib.majedt = VERSION; 		// Major edit number
    pib.minedt = EDITNO; 		// Minor edit number
    pib.copymsg = "";
    pib.prgname = prgname;
    pib.desc = description;
    pib.example = example;
    pib.errno = 0;
    getTrmParms();
    getHelpClr();
    if (argc > 1)			// Is there an arg to process?
    {
	argv++;
	progarg(argv, 0, options, NULL, NULL, (void (*)(char *, char*))NULL,
                (int (*)(void))NULL, NULL);
    }

    lkechars.result.buffer = buffer;

    // First determine which LKEs are already loaded

    for (;;)
    {
        if ((rtn = svcSysGetInfo(GSI_LKE, 0, NULL, 0)) < 0)
					// See how much room we need
            femsg2(prgname, lkeerrmsg, rtn, NULL);
        rtn = (rtn + 10) * sizeof(lkeinfo_data);
        if ((lkelist = malloc(rtn)) == NULL)
        {				// Get space for the data
            fputs("? DISKINST: Not enough memory available\n", stderr);
            exit(1);
        }
        if ((rtn=svcSysGetInfo(GSI_LKE, 0, lkelist, rtn)) < 0) // Get the data
            femsg2(prgname, lkeerrmsg, rtn, NULL);
        if ((rtn & 0x40000000L) == 0)	// Was it truncated?
	    break;			// No
        free(lkelist);			// Yes - give up the block and
    }					//   try again
    lkepnt = lkelist;
    while (--rtn >= 0)
    {
	if (strcmp(lkepnt->name, "HDKADRV") == 0)
	    idelke = HAVE;
	else if (strcmp(lkepnt->name, "CDRACLS") == 0)
	    cdromlke = HAVE;
	lkepnt++;
    }
    free(lkelist);

    // Determine which IDE disk units are already installed

    if (idelke == HAVE)			// Is the IDE disk driver loaded?
    {
	ideunit[0] = check("_D0:", ~IDE0); // Yes - check for units
	ideunit[1] = check("_D1:", ~IDE1);
	ideunit[2] = check("_D2:", ~IDE2);
	ideunit[3] = check("_D3:", ~IDE3);
    }

    // Now determine which devices we have have by loading DISKCHK.LKE. This
    //   is an init-only LKE which probes for the standard disk devices are
    //   returns the results.  Note that we do not probe for devices which we
    //   have determined are already installed.

    if (lkeload(TRUE, "DISKCHK.LKE", (struct lkechar *)&dcchars) >= 0)
	dcchars.mask.value = lkeargs.lla_value;

    // Install IDE disk units if necessary

    ideunit[0] = needdev(ideunit[0], IDE0);
    ideunit[1] = needdev(ideunit[1], IDE1);
    ideunit[2] = needdev(ideunit[2], IDE2);
    ideunit[3] = needdev(ideunit[3], IDE3);

    if ((ideunit[0] >= NEED || ideunit[1] >= NEED || ideunit[2] >= NEED ||
	    ideunit[3] >= NEED) && idelke != HAVE)
    {
	if (lkeload(quiet, "HDKADRV.LKE", (struct lkechar *)&lkechars) >= 0)
	    idelke = HAVE;
    }
    if (idelke == HAVE)
    {
	installide(ideunit[0], 0, "HDKA", 0x1F0, 14, 1);
	installide(ideunit[1], 1, "HDKA", 0x1F0, 14, 2);
	installide(ideunit[2], 2, "HDKA", 0x170, 15, 1);
	installide(ideunit[3], 3, "HDKA", 0x170, 15, 2);
    }
    if (reqcdrom && cdromlke != HAVE)
	lkeload(quiet, "CDRACLS.LKE", (struct lkechar *)&lkechars);
    return (0);
}

//********************************************
// Function: lkeload - Load an LKE
// Returned: Positive if OK, negative if error
//********************************************

int lkeload(
    int    quiet,
    char  *name,
    struct lkechar *chars)

{
    dcchars.result.buffer = buffer;
    buffer[0] = 0;
    strcpy(lkename+7, name);
    return (lkeloadf(quiet, chars, buffer));
}

//**************************************************
// Function: check - Check to see if a device exists
// Returned: NONE (0) or HAVE (-1)
//**************************************************

int check(
    char *name,
    int   bit)

{
    int rtn;

    if ((rtn = svcIoDevParm(O_PHYS|O_NOMOUNT, name, NULL)) < 0)
    {
	if (rtn == ER_NSDEV)
	    return (NONE);
	sprintf(buffer, "Error accessing device %s, device not installed",
		name + 1);
	warning(rtn, buffer);
	return (FAILED);
    }
    dcchars.mask.value &= bit;
    return (HAVE);
}

//**************************************************************
// Function: needdev - Determine if need to add a general device
// Returned: NONE (0), HAVE (-1), or NEED (1)
//**************************************************************

int needdev(
    int state,			// Current state of floppy unit
    int bit)			// Hardware present bit

{
    if (state == HAVE)
	return (HAVE);
    return (((dcchars.mask.value & bit) == 0) ? NONE : NEED);
}

//**************************************************
// Function: installide - Install an IDE disk device
// Returned: Nothing
//**************************************************

void installide(
    int   state,		// Unit state
    int   unitnum,		// Unit number
    char *type,			// Controller type
    int   ioreg,		// IO register value
    int   intnum,		// Interrupt number
    int   index)		// Index value

{
    static struct
    {	byte4_char  unit;
	text4_char  type;
	byte4_char  ioreg;
	byte4_char  index;
	byte4_char  intnum;
	uchar       end;
    } ideaddchars =
    {	{PAR_SET|REP_HEXV, 4, "UNIT"},
	{PAR_SET|REP_TEXT, 4, "TYPE"},
	{PAR_SET|REP_HEXV, 4, "IOREG"},
	{PAR_SET|REP_HEXV, 4, "INDEX"},
	{PAR_SET|REP_HEXV, 4, "INT"},
    };

    int rtn;

    if (state == NONE)
	return;
    if (state == HAVE)
    {
	if (!quiet)
	    printf("DISKINST: Disk D%d: is already installed\n", unitnum);
    }
    else if (state >= NEED)
    {
	ideaddchars.unit.value = unitnum;
	*(long *)(ideaddchars.type.value) = *(long *)type;
	ideaddchars.ioreg.value = ioreg;
	ideaddchars.intnum.value = intnum;
	ideaddchars.index.value = index;
	addqab.qab_buffer1 = diskclass;
	addqab.qab_buffer2 = (char *)&ideaddchars;
	if ((rtn = svcIoQueue(&addqab)) < 0 || (rtn = addqab.qab_error) < 0)
	{
	    sprintf(buffer, "Error adding device D%d:", unitnum);
	    warning(rtn, buffer);
	    return;
	}
	if (!quiet)
	    printf("DISKINST: Disk D%d: added\n", unitnum);
    }
    checkcdrom('D', unitnum);
}

checkcdrom(
    int letter,
    int unitnum)

{
    static struct
    {	text8_char  unittype;
	uchar       end;
    } typechars =
    {	{PAR_GET|REP_TEXT, 8, "UNITTYPE"}
    };

    int handle;
    int rtn;

    sprintf(buffer, "_%c%d:", letter, unitnum);
    if ((rtn = svcIoOpen(O_PHYS|O_NOMOUNT, buffer, NULL)) > 0)
    {
	handle = rtn;
	rtn = svcIoDevChar(handle, &typechars);
	svcIoClose(handle, 0);
	if (rtn >= 0)
	{
	    if (stricmp(typechars.unittype.value, "CDRM") == 0)
		reqcdrom = TRUE;
	    return;
	}
    }
    sprintf(buffer, "Error obtaining disk type for disk D%d:", unitnum);
    warning(rtn, buffer);
}


//********************************************
// Function: warning - Display warning message
// Returned: Nothing
//********************************************

void warning(
    int   code,
    char *msg)

{
    printf("DISKINST: %s\n", msg);
    if (code != 0)
    {
	svcSysErrMsg(code, 0x03, buffer);
	printf("             %s\n", buffer);
    }
}

//*************************************************
// Function: message - Display message for lkeloadf
// Returned: Nothing
//*************************************************

void  message(
    int level,
    char *text)

{
    level = level;

    fputs(text, stdout);
}
