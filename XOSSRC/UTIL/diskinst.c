// This program automatically installs all standard disks.  The following
//   disks are installed:
//	Floppy:	Standard floppies A and B as F0: and F1:.  Disks types are
//		obtained from the CMOS data
//	IDE:	Primary master disk as D0:, primary slave disk as D1:,
//		secondary master disk as D2:, secondary slave disk as D3:.
//		Disk information is obtained from the disks if possible or
//		from the CMOS data for the primary disks if not available
//		from the disk.  Disks on the scondary controller must
//		provide information.  Disks may be fixed, removable, or
//		CDROM.
//	SCSI:	Single SCSI controller in standard boot configuration.  Up
//		to 7 disks (S0: throught S6:) are supported.  Unit number is
//		the SCSI target number.  Disks may be fixed, removable, or
//		CDROM.

// All necessary LKEs are loaded if they are not already loaded. DOS drive
//   letters are not defined.  The FIXDRIVE and RMVDRIVE programs should be
//   run after this program completes to define DOS drive letters.

// This program can be executed from the once-only RAM-disk.  It is normally
//   executed from the once-only RAM-disk to install the boot devices.  After
//   the system is running, it is normally executed again to install all other
//   disk devices.

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
#define EDITNO  1


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
#define SCSIA0 0x0010
#define FLPY0  0x4000
#define FLPY1  0x8000

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
{   {PAR_SET|REP_HEXV, 4, "MASK"  , FLPY0|FLPY1|IDE0|IDE1|IDE2|IDE3|SCSIA0},
    {PAR_GET|REP_STR , 0, "RESULT", 0, 0, 1024, 1024}
};

struct
{   lngstr_char result;
    uchar       end;
} lkechars =
{   {PAR_GET|REP_STR, 0, "RESULT", 0, 0, 1024, 1024}
};


struct
{   byte4_char target[7];
    uchar      end;
} targetchars =
{   {	{PAR_GET|REP_HEXV, 4, "TARGET0"},
	{PAR_GET|REP_HEXV, 4, "TARGET1"},
	{PAR_GET|REP_HEXV, 4, "TARGET2"},
	{PAR_GET|REP_HEXV, 4, "TARGET3"},
	{PAR_GET|REP_HEXV, 4, "TARGET4"},
	{PAR_GET|REP_HEXV, 4, "TARGET5"},
	{PAR_GET|REP_HEXV, 4, "TARGET6"}
    }
};

struct sdfp
{   text8_parm class;
    char       end;
} sdfp =
{   {PAR_SET|REP_TEXT, 8, IOPAR_CLASS, "SCSI"}
};

char diskclass[] = "DISK:";
char scsiclass[] = "SCSI:";

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

char  prgname[] = "DISKINST";

char  flpylke;
char  idelke;
char  scsiclslke;
char  scsiadtalke;
char  scsidsklke;
char  cdromlke;
char  dfslke;

char  flpyunit[2];
char  ideunit[4];
char  scsiadev;
char  scsiaunit[7];

uchar reqscsi = FALSE;
uchar reqcdrom = FALSE;

char *flpytbl[] = {"DD5", "HD5", "DD3", "HD3"};

char lkeerrmsg[] = "Cannot obtain list of installed LKEs";

arg_spec options[] =
{
    {"Q*UIET" ,ASF_BOOL|ASF_STORE, NULL,   &quiet   , 0x01,
	    "Only output error messages." },
    {"?"      , 0,                 NULL, AF(opthelp), 0,
	    "Display this message." },
    {"H*ELP"  , 0,                 NULL, AF(opthelp), 0,
	    "Display this message." },
    {NULL}
};

char  example[] = "{/option}";
char  description[] = "This command automatically loads installs all standard "
    "floppy disk, IDE disk, and SCSI disk drives. All necessary LKEs are "
    "loaded if they are not already loaded. Since this command probes looking "
    "for installed devices, it MUST NOT be executed when the system is active. "
    "Normally, it should only be executed as part of the system startup "
    "sequence. Its use at any other time may result in serious problems, "
    "including system crashes and data loss! This command does not assign "
    "DOS drive letters. The DOSDRIVE command should be executed after this "
    "command to assign DOS drive letters for fixed disks. DOS drive letters "
    "for removable disks must be assigned manually using the DEVCHAR command.";

int main(
    int   argc,
    char *argv[])

{

    lkeinfo_data *lkepnt;
    int           rtn;
    int           target;

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
	if (strcmp(lkepnt->name, "FDKADRV") == 0)
	    flpylke = HAVE;
	else if (strcmp(lkepnt->name, "HDKADRV") == 0)
	    idelke = HAVE;
	else if (strcmp(lkepnt->name, "SCSICLS") == 0)
	    scsiclslke = HAVE;
	else if (strcmp(lkepnt->name, "SCSIADTA") == 0)
	    scsiadtalke = HAVE;
	else if (strcmp(lkepnt->name, "SDSKDRV") == 0)
	    scsidsklke = HAVE;
	else if (strcmp(lkepnt->name, "CDRACLS") == 0)
	    cdromlke = HAVE;
	else if (strcmp(lkepnt->name, "DFSCLS") == 0)
	    dfslke = HAVE;
	lkepnt++;
    }
    free(lkelist);

    // Load the DFS (DOS file system) driver if we need to and can

    if (dfslke != HAVE)
    {
	if (lkeload(quiet | 0x02, "DFSCLS.LKE", (struct lkechar *)&lkechars)
		== 0)
	    dfslke = HAVE;
    }

    // Determine which floppy disk units are already installed

    if (flpylke == HAVE)		// Is the floppy disk driver loaded?
    {
	flpyunit[0] = check("_F0:", ~FLPY0); // Yes - check for units
	flpyunit[1] = check("_F1:", ~FLPY1);
    }

    // Determine which IDE disk units are already installed

    if (idelke == HAVE)			// Is the floppy disk driver loaded?
    {
	ideunit[0] = check("_D0:", ~IDE0); // Yes - check for units
	ideunit[1] = check("_D1:", ~IDE1);
	ideunit[2] = check("_D2:", ~IDE2);
	ideunit[3] = check("_D3:", ~IDE3);
    }

    // Determine if the SCSI controller device is already installed

    if (scsiclslke == HAVE && scsiadtalke == HAVE)
	scsiadev = check("_SCSI0:", ~SCSIA0);

    // Now determine which devices we have have by loading DISKCHK.LKE. This
    //   is an init-only LKE which probes for the standard disk devices are
    //   returns the results.  Note that we do not probe for devices which we
    //   have determined are already installed.

    if (lkeload(0x01, "DISKCHK.LKE", (struct lkechar *)&dcchars) == 0)
	dcchars.mask.value = lkeargs.lla_value;

    // Install floppy disk units if necessary

    flpyunit[0] = needflpy(flpyunit[0], 4, FLPY0);
    flpyunit[1] = needflpy(flpyunit[1], 0, FLPY1);

    if ((flpyunit[0] >= NEED || flpyunit[1] >= NEED) && flpylke != HAVE)
    {
	if (lkeload(quiet | 0x02, "FDKADRV.LKE", (struct lkechar *)&lkechars)
		== 0)
	    flpylke = HAVE;
    }
    if (flpylke == HAVE)
    {
	installflpy(flpyunit[0], 0, "FDKA", 0x3F0, 1);
	installflpy(flpyunit[1], 1, "FDKA", 0x3F0, 2);
    }

    // Install IDE disk units if necessary

    ideunit[0] = needdev(ideunit[0], IDE0);
    ideunit[1] = needdev(ideunit[1], IDE1);
    ideunit[2] = needdev(ideunit[2], IDE2);
    ideunit[3] = needdev(ideunit[3], IDE3);

    if ((ideunit[0] >= NEED || ideunit[1] >= NEED || ideunit[2] >= NEED ||
	    ideunit[3] >= NEED) && idelke != HAVE)
    {
	if (lkeload(quiet | 0x02, "HDKADRV.LKE", (struct lkechar *)&lkechars)
		== 0)
	    idelke = HAVE;
    }
    if (idelke == HAVE)
    {
	installide(ideunit[0], 0, "HDKA", 0x1F0, 14, 1);
	installide(ideunit[1], 1, "HDKA", 0x1F0, 14, 2);
	installide(ideunit[2], 2, "HDKA", 0x170, 15, 1);
	installide(ideunit[3], 3, "HDKA", 0x170, 15, 2);
    }

    // Install the SCSI controller device if necessary

    scsiadev = needdev(scsiadev, SCSIA0);

    if (scsiadev == NEED)
    {
	if (scsiclslke != HAVE)
	{
	    if (lkeload(quiet | 0x02, "SCSICLS.LKE",
		    (struct lkechar *)&lkechars) == 0)
		scsiclslke = HAVE;
	}
	if (scsiadtalke != HAVE)
	{
	    if (lkeload(quiet | 0x02, "SCSIADTA.LKE",
		    (struct lkechar *)&lkechars) == 0)
		scsiadtalke = HAVE;
	}
    }
    if (scsiclslke == HAVE && scsiadtalke == HAVE)
	scsiadev = installscsia(scsiadev);

    if (scsiadev == HAVE)
    {
	if ((scsihndl = svcIoOpen(O_PHYS, "_SCSI0:", 0)) < 0)
	    warning(scsihndl, "Error opening SCSI controller device SCSI0:");
	else
	{
	    if ((rtn = svcIoDevChar(scsihndl, &targetchars)) < 0)
		warning(rtn, "Error obtaining SCSI target information from "
			"SCSI0:");
	    else
	    {
		target = 0;
		do
		{
		    if (targetchars.target[target].value & 0x01)
		    {
			if ((rtn = svcIoSpecial(scsihndl, 2 + (target << 8),
				buffer, 255, &sdfp)) < 0)
			{
			    sprintf(buffer, "Error obtaining SCSI device "
				    "information for SCSI0: target %d", target);
			    warning(rtn, buffer);
			    continue;
			}
			rtn = buffer[0] & 0x1F;
			if (rtn == 0 || rtn == 5)
			{
			    scsiaunit[target] = rtn + 1;
			    reqscsi = TRUE;
			}
		    }
		} while (++target <= 6);
		svcIoClose(scsihndl, 0);
		if (reqscsi)
		{
		    if (scsidsklke != HAVE)
		    {
			if (lkeload(quiet | 0x02, "SDSKDRV.LKE",
				(struct lkechar *)&lkechars) == 0)
			    scsidsklke = HAVE;
		    }
		    target = 0;
		    do
		    {
			if (scsiaunit[target] != 0)
			{
			    if (scsiaunit[target] == 6)
				reqcdrom = TRUE;
			    sprintf(buffer, "_S%d:", target);
			    scsiaunit[target] = (check(buffer, -1) == NONE) ?
				    NEED : HAVE;
			    scsiaunit[target] = installscsidsk(
				    scsiaunit[target], target);
			}
		    } while (++target <= 6);
		}
	    }
	}
    }
    if (reqcdrom && cdromlke != HAVE)
	lkeload(quiet | 0x02, "CDRACLS.LKE", (struct lkechar *)&lkechars);
    return (0);
}

//********************************
// Function: lkeload - Load an LKE
// Returned: 0 if OK, 1 if error
//********************************

int lkeload(
    int    quiet,
    char  *name,
    struct lkechar *chars)

{
    dcchars.result.buffer = buffer;
    buffer[0] = 0;
    strcpy(lkename+7, name);
    return (lkeloadf(quiet, chars));
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

//*****************************************************************
// Function: needflpy - Determine if need to add a floppy disk unit
// Returned: NONE (0), HAVE (-1), or NEED (positive drive type)
//*****************************************************************

int needflpy(
    int state,			// Current state of floppy unit
    int shift,			// Amount to shift CMOS data bits
    int bit)			// Hardware present bit

{
    int type;

    if (state == HAVE)
	return (HAVE);

    if ((type = svcSysCmos(0x10, -1)) >= 0)
    {
	type >>= shift;
	type &= 0x0F;
	if (type > 4 || (dcchars.mask.value & bit) == 0)
	    type = 0;
    }
    else
	type = 0;
    return (type);
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

//*****************************************************
// Function: installflpy - Install a floppy disk device
// Returned: Nothing
//*****************************************************

void installflpy(
    int   state,		// Unit state
    int   unitnum,		// Unit number
    char *type,			// Controller type
    int   ioreg,		// IO register value
    int   index)		// Index value

{
    static struct
    {	byte4_char  unit;
	text4_char  type;
	text4_char  unittype;
	byte4_char  ioreg;
	byte4_char  index;
	uchar       end;
    } flpyaddchars =
    {	{PAR_SET|REP_HEXV, 4, "UNIT"},
	{PAR_SET|REP_TEXT, 4, "TYPE"},
	{PAR_SET|REP_TEXT, 4, "UNITTYPE"},
	{PAR_SET|REP_HEXV, 4, "IOREG"},
	{PAR_SET|REP_HEXV, 4, "INDEX"}
    };

    int rtn;

    if (state < NEED)
    {
	if (state == HAVE && !quiet)
	    printf("DISKINST: Disk F%d: is already installed\n", unitnum);
	return;
    }
    flpyaddchars.unit.value = unitnum;
    *(long *)(flpyaddchars.type.value) = *(long *)type;
    *(long *)(flpyaddchars.unittype.value) = *(long *)(flpytbl[state - 1]);
    flpyaddchars.ioreg.value = ioreg;
    flpyaddchars.index.value = index;
    addqab.qab_buffer1 = diskclass;
    addqab.qab_buffer2 = (char *)&flpyaddchars;
    if ((rtn = svcIoQueue(&addqab)) < 0 || (rtn = addqab.qab_error) < 0)
    {
	sprintf(buffer, "Error adding device F%d:", unitnum);
	warning(rtn, buffer);
    }
    else
    {
	if (!quiet)
	    printf("DISKINST: Disk F%d: added\n", unitnum);
    }
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

//********************************************************
// Function: installscsia - Install SCSI controller driver
// Returend: New state
//********************************************************

int installscsia(
    int state)

{
    static struct
    {	byte4_char  unit;
	text4_char  type;
	byte4_char  ioreg;
	uchar       end;
    } scsiaaddchars =
    {	{PAR_SET|REP_HEXV, 4, "UNIT" , 0},
	{PAR_SET|REP_TEXT, 4, "TYPE" , "ADTA"},
	{PAR_SET|REP_HEXV, 4, "IOREG", 0x330}
    };

    int rtn;

    if (state == NONE)
	return (NONE);
    if (state == HAVE)
    {
	if (!quiet)
	    fputs("DISKINST: SCSI controller SCSI0: is already installed\n",
		    stdout);
	return (HAVE);
    }
    addqab.qab_buffer1 = scsiclass;
    addqab.qab_buffer2 = (char *)&scsiaaddchars;
    if ((rtn = svcIoQueue(&addqab)) < 0 || (rtn = addqab.qab_error) < 0)
    {
	warning(rtn, "Error adding device SCSI0:");
	return (NONE);
    }
    if (!quiet)
	fputs("DISKINST: SCSI controller SCSI0: added\n", stdout);
    return (HAVE);
}

//******************************************************
// Function: installscsidsk - Install a SCSI disk device
// Returned: None
//******************************************************

void installscsidsk(
    int state,
    int target)

{
    static struct
    {	byte4_char  unit;
	text4_char  type;
	text8_char  scsidev;
	byte4_char  scsitar;
	uchar       end;
    } scsidskaddchars =
    {	{PAR_SET|REP_HEXV, 4, "UNIT"   , 0},
	{PAR_SET|REP_TEXT, 4, "TYPE"   , "SDSK"},
	{PAR_SET|REP_TEXT, 8, "SCSIDEV", "SCSI0"},
	{PAR_SET|REP_HEXV, 4, "SCSITAR"}
    };

    int rtn;

    if (state == NONE)
	return;
    if (state == HAVE)
    {
	if (!quiet)
	    printf("DISKINST: Disk S%d: is already installed\n", target);
	return;
    }
    scsidskaddchars.unit.value = scsidskaddchars.scsitar.value = target;
    addqab.qab_buffer1 = diskclass;
    addqab.qab_buffer2 = (char *)&scsidskaddchars;
    if ((rtn = svcIoQueue(&addqab)) < 0 || (rtn = addqab.qab_error) < 0)
    {
	sprintf(buffer, "Error adding device S%d:", target);
	warning(rtn, buffer);
    }
    else
    {
	if (!quiet)
	    printf("DISKINST: Disk S%d: added\n", target);
    }
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
