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
#include <XCSTRING.H>
#include <PROCARG.H>
#include <MALLOC.H>
#include <XCMALLOC.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSRUN.H>
#include <XOSERR.H>
#include <XOSRTN.H>
#include "MKBOOT.H"

// The BOOT.RUN file is a stand-alone .RUN file with a somewhat unusual
//   structure as follows:
//	Segment:  boot_s
//	  Msect:  level3_m , Offset=0x8200
//	  Msect:  fat16_m  , Offset=0x7C00
//	  Msect:  fat32_m  , Offset=0x7C00
//	  Msect:  xfs1_m   , Offset=0x7C00
//	  Msect:  xfs2_m   , Offset=0x6000
//   There may be additional segments after this which will be effectively
//     conbined with the boot_s segment.  These will normally contain a
//     debugger.
//   Since .RUN files do not contain names for segments or msects, we use the
//   order of the segments and msects to identify them!

// The existance of multiple msects with the same offset in a segment means
//   that this file is not loadable in the normal way.  Instead, this program
//   selects the desired msects for the type of file system and uses them to
//   construct the XOS.BSF file, which is mostly a simple memory image file.

// The bootstrap is implemented in 2 or 3 levels depending on the type of
//   file system.  The level 3 bootstrap is the same of all file systems.  It
//   contains code to support all file systems.  This means that you can load
//   from any supported file system, independent of which file system the
//   bootstrap came from.  For all file systems, the level 1 bootstrap is
//   contained in the boot block. The level 3 bootstrap is contained in the
//   level3_m msect.

// For the FAT12/FAT16 file system, the level 1 bootstrap loads a level 2
//   bootstrap which is stored in the first block of the XOS.BSF file.  The
//   level 2 bootstrap then loads the remainder of the XOS.BSF file which is
//   the level 3 bootstrap.  Both level 1 and level 2 bootstraps are contained
//   in the fat16_m msect.  The level 1 bootstrap is at offset 7C00h (the
//   beginning of the msect) and the level bootstrap is at offset 7E00h
//   (immediately following the level 1 bootstrap).

// For the FAT32 file system, the level 1 bootstrap loads the level 3 bootstrap
//   directly.  There is no level 2 bootstrap.  The level 1 bootstrap is
//   contained in the fat32_m msect at offset 7C00h.

// For the XOS file system, the level 1 bootstrap loads the level 2 bootstrap
//   for the reserved bootstrap area of the file system.  The level 2 bootstrap
//   then loads the level 3 bootstrap from the XOS.BSF file.  The level 1
//   bootstrap is contained in the xos1_m msect at offset 7C00h.  The level 2
//   bootstrap is contained in the xos2_m msect at offset 6000h.

// This program copies the selected level 1 bootstrap to the boot block and
//   constructs the XOS.BSF file.

// For the FAT file systems the XOS.BSF file contains the level 2 bootstrap
//   (one block), followed by the saved boot block (one block), followed by
//   the configuration block (one block) followed by the level_m msect which
//   contains the level 3 bootstrap code, followed by any additional segments.

// For the XOS file systems the XOS.BSF file contains the saved boot block
//   (one block), followed by the configuration block (one block) followed by
//   the level_m msect which contains the level 3 bootstrap code, followed by
//   any additional segments. The level 2 bootstrap is stored in a seperate
//   area of the file system.


typedef struct header
{   char   id1[8];				// First ID value ('XOS*BOOT')
    long   id2;					// Second ID value (0FFFFFFFFh)
    ushort version;				// Version number
    ushort edit;				// Edit number
    long   ofsboot;				// Offset of level 3 bootstrap code
    long   lenboot;				// Length of level 3 bootstrap code
    long   ofsfat16;			// Offset of level 1 FAT12/FAT16 bootstrap code
    long   lenfat16;			// Length of level 1 FAT12/FAT16 bootstrap code
    long   ofsfat32;			// Offset of level 1 FAT32 bootstrap code
    long   lenfat32;			// Length of level 1 FAT32 bootstrap code
    long   ofsxfs;				// Offset of level 1 XFS bootstrap code
    long   lenxfs;				// Length of level 1 XFS bootstrap code
} HEADER;

typedef struct cfgblk
{   long  id;					// ID value (0FFFFFFFEh)
    short timeout;				// Time-out value - number of clock ticks (18.2
								//   per second)
    short numline;				// Number of menu lines
    char  dftname[44];			// Default file name
    char  f1name[44];			// Name for F1
    char  f2name[44];			// Name for F2
    char  f3name[44];			// Name for F3
    char  f4name[44];			// Name for F4
    char  f5name[44];			// Name for F5
    char  f6name[44];			// Name for F6
    char  f7name[44];			// Name for F7
    char  f8name[44];			// Name for F8
    char  f9name[44];			// Name for F9
    char  f10name[44];			// Name for F10
} CFGBLK;

typedef struct btblk
{   char   jump[3];				// Jump instruction to start of bootstrap code
    char   oemname[8];			// OEM name and version
    ushort secsize;				// Sector size in bytes
    uchar  secpcls;				// Sectors per cluster
    ushort ressec;				// Total number of reserved sectors
    uchar  numfats;				// Number of FATs
    ushort rdirent;				// Number of root directory entries
    ushort sectors;				// Total number of sectors on disk (16 bit
								//   value)
    uchar  media;				// Media description byte
    ushort secpfat;				// Number of sectors per FAT table
    ushort secptrk;				// Number of sectors per track
    ushort heads;				// Number of heads
    long   hidsec;				// Number of hidden sectors
    long   ttlsec;				// Total number of sectors on disk (32 bit
								//   value)
    long   ttlsecpfat;			// Sectors per FAT (32 bit value)
    ushort extflags;			// Flags
    ushort fsversion;			// File system version
    long   rootcls;				// Number of first cluster in root directory
    ushort fsinfo;				// Sector number of the file system intormation
								//   sector
    ushort backboot;			// Sector number of the backup boot sector
								//   (0FFFFh if no backup boot sector)
    uchar  devunit;				// Device unit (0 = floppy, 80 = hard disk)
								//   (FAT12/FAT16)
    uchar  fill[3];				// Not used (FAT12/FAT16)
    long   partofs;				// Partition offset (FAT12/FAT16)
    uchar  junk[36];
    long   fatbegin;			// Block number on disk of first FAT (FAT32)
    long   databegin;			// Block number on disk of first allocated
								//   block (FAT32)
	long   level2blk;
    uchar  code[369];			// Level 1 bootstrap code
	uchar  version;				// Bootstrap version number
	uchar  editnum;				// Bootstrap edit number
    char   errmsg[31];			// Error message text
    ushort id;					// ID value (0xAA55)
} BTBLK;

static long  rundev;			// Device handle for input (RUN) file
static long  memdev;			// Device handle for output (MEM) file
static long  diskdev;			// Device handle for physical disk device

static BTBLK *oldbootblk;

static CFGBLK *cfgpnt;

static long  datasize = 1024;

static short bootvernum;		// Bootstrap version number
static short bootedtnum;		// Bootstrap edit number

static char  runname[] = "XOSSYS:BOOT.RUN";
static char  memname[32];		// File specification for output file
static char  notvalid[] =
		"XOSSYS:BOOT.RUN is not a valid stand-alone RUN file";

static char *databfr;			// Data buffer
static char *datapnt;
static char *databgn;


typedef struct msd
{   char *buffer;
    long  dataos;
    long  datasz;
    long  alloc;
    long  relocos;
    long  relocsz;
    long  select;
    long  offset;
} MSD;

static MSD *level1data;
static MSD  fat16data;
static MSD  fat32data;
static MSD  xfs1data;
static MSD  xfs2data;
static MSD  level3data[6];
static MSD *mspnt;

static long   numsegs;
static long   reloccnt;

static union
{   char  *c;
    short *s;
}  position;
static long delta;
static long segment;
static long header;

static struct runhead runhead;
static struct runseg seghead;
static struct runmsect mschead;

static struct
{   byte1_parm attrib;
    char       end;
} memparm1 =
{   {(PAR_SET|REP_HEXV), 1, IOPAR_FILATTR, A_NORMAL}
};

static struct
{   byte1_parm attrib;
    char       end;
} memparm2 =
{   {(PAR_SET|REP_HEXV), 1, IOPAR_FILATTR, A_RDONLY|A_HIDDEN|A_SYSTEM}
};

static struct
{	byte4_parm blk;
	uchar      end;
} blkparms =
{	{(PAR_GET|REP_DECV), 4, IOPAR_DSKBLOCK}
};

static struct
{   text8_char  class;
    text8_char  fstype;
    byte4_char  partoff;
    char        end;
} diskchar1 =
{   {(PAR_GET|REP_TEXT), 8, "CLASS"},
    {(PAR_GET|REP_TEXT), 8, "FSTYPE"},
    {(PAR_GET|REP_DECV), 4, "PARTOFF"}
};

static struct
{   text8_char  unittype;
    char        end;
} diskchar2 =
{   {(PAR_GET|REP_TEXT), 8, "UNITTYPE"}
};

static type_qab diskqab =
{
    QFNC_WAIT|QFNC_DEVCHAR,		// qab_open
    0,							// qab_status
    0,							// qab_error
    0,							// qab_amount
    0,							// qab_handle
    0,							// qab_vector
    0, 0,
    DCF_VALUES,					// qab_option
    0,							// qab_count
    NULL, 0,					// qab_buffer1
    (char *)&diskchar1, 0,		// qab_buffer2
    NULL, 0						// qab_parm
};

static void   cantsave(void);
static void   errclose(void);
static void (*fail)(char *str1, long code, char *str2);
static int    getmsectheader(MSD *data);
static int    getsegheader(void);
static void (*notice)(char *str);
static int    runread(char *buffer, long size,
    void fail(char *str1, long code, char *str2));
static int    runskip(long size, void fail(char *str1, long code, char *str2));


//**************************************************
// Function: mkbootf - Write XOS bootstrap to a disk
// Returned: 0 if normal, 1 if error
//**************************************************

int mkbootf(
    char      diskname[12],		// Name of disk
    char     *disktype,			// Disk type: 0x80 = hard disk, 0 = floppy
	char     *fstype,			// File system type
    char      fname[10][50],
    char      defprog[42],
    char      autoboot,
    int       timeout,
    char      quiet,
    mkbfail   failarg,			// void fail(char *str1, long code, char *str2)
    mkbnotice noticearg)		// void notice(char *str)

{
    MSD   *msectpnt;
    long   rtn;
    int    num;
    int    width;
    int    numpline;
    int    numlines;
    int    numitems;
    int    begin;
    int    line;
    int    pos;
    int    len;
    int    cnt;
    int    ffat;
    int    selector;
    BTBLK  newbootblk;
	char   bufr[128];

    fail = failarg;
    notice = noticearg;
    rundev = 0;
    memdev = 0;
    diskdev = 0;

    strmov(strmov(memname, diskname), "\\XOS.BSF");
    if ((rtn = svcIoDevParm(0, memname, &memparm1)) < 0) // Clear attributes
    {
        if (rtn != ER_FILNF)
        {
            errclose();
            fail("Error clearing attributes for bootstrap file", rtn, memname);
            return (1);
        }
    }
    if ((rundev = svcIoOpen(O_IN, runname, NULL)) < 0)
    {
        errclose();
        fail("Error opening bootstrap file", rundev, runname);
        return (1);
    }
    if ((diskdev = svcIoOpen(O_IN|O_OUT|O_PHYS, diskname, NULL)) < 0)
    {
        errclose();
        fail("Error opening physical disk", diskdev, diskname);
        return (1);
    }
    diskqab.qab_handle = diskdev;
    if ((rtn = svcIoQueue(&diskqab)) < 0 || (rtn = diskqab.qab_error) < 0)
    {
        errclose();
        fail("Error obtaining disk class name", rtn, diskname);
        return (1);
    }
    if (strcmp(diskchar1.class.value, "DISK") != 0)
    {
        sprintf(bufr, "Device %s is not a disk", diskname);
        errclose();
        fail(bufr, 0, NULL);
        return (1);
    }
    diskqab.qab_buffer2 = (char *)&diskchar2;
    if ((rtn = svcIoQueue(&diskqab)) < 0 || (rtn = diskqab.qab_error) < 0)
    {
        errclose();
        fail("Error obtaining disk unit type", rtn, diskname);
        return (1);
    }
    *disktype = (stricmp(diskchar2.unittype.value, "HARD") == 0)? 0x80: 0;

    if (runread((char *)(&runhead), 64, fail) != 0) // Read the RUN file header
        return (1);
    if (runhead.magic != 0x22D7 || runhead.hdrsize < 58 ||
            runhead.version != 2 || runhead.imagetype != 1 ||
            runhead.numsegs == 0)
    {
        errclose();
        fail(notvalid, 0, NULL);		// Make sure valid RUN file
        return (1);
    }
    if (runhead.numsegs > 8)
    {
        errclose();
        fail("Input file specifies more than 8 segments", 0, NULL);
        return (1);
    }
    if (runhead.hdrsize > 58)			// Skip excess file header data
    {
        if (runskip(runhead.hdrsize - 58, fail) != 0)
            return (1);
    }
    if ((numsegs = runhead.numsegs) < 1)
    {
		errclose();
		fail("File does not contain at least one segment", 0, NULL);
		return (1);
    }
    if (!getsegheader())				// Read the first segment header
		return (1);

    if ((cnt = seghead.nummsc - 2) < 0)	// This segment must contain at least
    {									//   2 msects
		errclose();
		fail("First segment does not contains at least two msects", 0, NULL);
		return (1);
    }
    if (!getmsectheader(&level3data[0]))
		return (1);
    level3data[0].select = 0;
    selector = (level3data[0].alloc >> 4) + 0x840;
    datasize += level3data[0].alloc;
    if (!getmsectheader(&fat16data))
		return (1);
    if (--cnt >= 0)
    {
		if (!getmsectheader(&fat32data))
			return (1);
		if ((cnt -= 2) >= 0)
		{
			if (!getmsectheader(&xfs1data))
				return (1);
			if (!getmsectheader(&xfs2data))
				return (1);
		}
    }
    while (--cnt >= 0)					// Discard any msects we don't expect
    {
		if (!getmsectheader(&level3data[1]))
			return (1);
    }
    msectpnt = level3data + 1;			// Read any additional segment headers
    cnt = numsegs;
    while (--cnt > 0)
    {
		if (!getsegheader())
			return (1);
		if (seghead.nummsc != 1)		// This segment must contain 1 msect
		{
			errclose();
			fail("Segment does not contains one msect", 0, NULL);
			return (1);
		}
		if (!getmsectheader(msectpnt))
			return (1);
		msectpnt->select = selector;
		selector += (msectpnt->alloc >> 4);
		datasize += msectpnt->alloc;
		msectpnt++;
    }

    // Here with all headers read, now determine if we have the boot code for
    //   the type of file system that we have.  This version of MKBOOT can
    //   handle FAT12/FAT16 and FAT32 only.  In all cases the first 512 bytes
    //   of the level 1 code contains the boot block image.  The next 512
	//   bytes contain the level 2 code. This code is copied to the .BSF file
	//   at offset 0x400.  The level 3 code is copied to the .BSF file
    //   immediately following this.

	if (strcmp(diskchar1.fstype.value, "DOS12") == 0)
	{
		strcpy(fstype, "FAT12");
		level1data = &fat16data;
		datasize += 512;
	}
	else if (strcmp(diskchar1.fstype.value, "DOS16") == 0)
    {
		strcpy(fstype, "FAT16");
		level1data = &fat16data;
		datasize += 512;
    }
    else if (strcmp(diskchar1.fstype.value, "DOS32") == 0)
	{
		strcpy(fstype, "FAT32");
		level1data = &fat32data;
		datasize += 512;
	}
    else
    {
		printf(bufr, "Booting from the %.8%s file system is not supported",
				diskchar1.fstype.value);
		fail(bufr, 0, NULL);
		return (1);
    }
    if (level1data->datasz == 0)
    {
		printf(bufr, "No bootstrap code is available for the %.8%s file system",
		diskchar1.fstype.value);
		fail(bufr, 0, NULL);
		return (1);
	}

    // Now read the level 1 code for the boot block

    if ((rtn = svcIoSetPos(rundev, level1data->dataos, 0)) < 0)
    {									// Position to read data
		errclose();
		fail("Error setting position for reading data", rtn, runname);
		return (1);
    }
    if (runread((char *)&newbootblk, 512, fail) != 0)
		return (1);

	datapnt = databfr = getspace(datasize); // Allocate buffer    

    if (level1data == &fat16data ||		// If a FAT file system, read the
			level1data == &fat32data)	//   level 2 code
	{
		if (runread(datapnt, 512, fail) != 0) // Read the RUN file data
			return (1);
		datapnt += 512;
    }
    cfgpnt = (CFGBLK *)datapnt;
    datapnt += 512;
    oldbootblk = (BTBLK *)datapnt;
    datapnt += 512;
    databgn = datapnt;
    mspnt = level3data;
    cnt = numsegs;
    do
    {
		if ((rtn = svcIoSetPos(rundev, mspnt->dataos, 0)) < 0)
		{								// Position to read data
			errclose();
			fail("Error setting position for reading data", rtn, runname);
			return (1);
		}
		if (runread(datapnt, mspnt->datasz, fail) != 0)
			return (1);					// Read the RUN file data

        if (mspnt->relocsz)
        {
            position.c = datapnt;
            if ((rtn = svcIoSetPos(rundev, mspnt->relocos, 0)) < 0)
            {							// Position to read data
                errclose();
                fail("Error setting position for reading relocation "
                        "information", rtn, runname);
                return (1);
            }
            num = mspnt->relocsz;
            do
            {
                if ((header = svcIoInSingle(rundev)) < 0)
                {
                    errclose();
                    fail("Error reading relocation information", rtn, runname);
                    return (1);
                }
                if (header & 0xFC)
                {
                    errclose();
                    fail("Illegal relocation type", 0, NULL);
                    return (1);
                }
                if ((segment = svcIoInSingle(rundev)) < 0)
                {
                    errclose();
                    fail("Error reading relocation information", rtn, runname);
                    return (1);
                }
                if (segment == 0 || segment > runhead.numsegs)
                {
                    errclose();
                    fail("Illegal segment number for relocation", 0, NULL);
                    return (1);
                }
                delta = 0;
                if ((rtn = svcIoInBlock(rundev, (char *)&delta, header+1)) < 0)
                {
                    errclose();
                    fail("Error reading relocation information", rtn, runname);
                    return (1);
                }
                position.c += delta;
                if ((position.c < datapnt) ||
                        (position.c > (datapnt + mspnt->alloc)))
                {
                    errclose();
                    fail("Relocation out of range", 0, NULL);
                    return (1);
                }
                *position.s = level3data[segment-1].select;
                ++reloccnt;
            } while (--num > 0);
        }
        datapnt += mspnt->alloc;
        ++mspnt;
    } while (--cnt > 0);
    svcIoClose(rundev, 0);				// Close the input file

    bootvernum = *((short *)(databgn + 0)); // Get bootstrap's version
    bootedtnum = *((short *)(databgn + 2)); //   number

	if (bootvernum < 7)
	{
		sprintf(bufr, "Cannot install bootstrap version %d.%d (must be 7.0 or "
				"greater)", bootvernum, bootedtnum);
		fail(bufr, 0, NULL);
		return (1);
	}

    // Read the current boot block

    if ((rtn = svcIoSetPos(diskdev, 0, 0)) < 0)
    {									// Position to read boot block
        errclose();
        fail("Error setting position for reading", rtn, diskname);
        return (1);
    }
    if ((rtn = svcIoInBlock(diskdev, (char *)oldbootblk, 512)) < 0) // Read it
    {
        errclose();
        fail("Error reading boot block", rtn, diskname);
        return (1);
    }

    if (strncmp(newbootblk.errmsg, oldbootblk->errmsg,
			sizeof(newbootblk.errmsg) + 2) == 0)
	{									// Has the boot block been modified?
										// Yes
		if (level1data == &fat32data && oldbootblk->version < 7)
		{
			fail("Cannot update FAT32 bootstrap previous to version 7.0\n"
					"          Remove old bootstrap first", 0, NULL);
			return (1);
		}
        if (!quiet)
        {
            sprintf(bufr, "%% MKBOOT: Updating XOS bootstrap version %d.%d to "
                    "version %d.%d", oldbootblk->version, oldbootblk->editnum,
					bootvernum, bootedtnum);
            notice(bufr);
        }

		// Here if we have modified the boot block. - We must copy the
		//   original saved boot block from the current XOS.BSF file.

		// The format of the XOS.BSF file on FAT file system is as follows:
		//	Offset	Description
		//	000h	FAT level 2 code
		//	200h	Configuration block
		//	400h	Saved boot block
		//	600h	Start of level 3 code

		// The format of the XOS.BSF file on an XOS file system is as follows:
		//	Offset	Description
		//	000h	Configuration block
		//	200h	Saved boot block
		//	400h	Start of level 3 code

        if ((memdev = svcIoOpen(O_IN, memname, NULL)) < 0)
		{
            cantsave();
			return (1);
		}
        else							// Here with XOS.BSF open
        {
			if ((rtn = svcIoSetPos(memdev, (level1data == &fat16data ||
					level1data == &fat32data) ? 1024 : 512, 0)) < 0 ||
					(rtn = svcIoInBlock(memdev, (char *)oldbootblk, 512)) < 0)
			{							// Read the block
				errclose();
				fail("Error reading saved boot block", rtn, memname);
				return (1);				// If error
			}
            if (oldbootblk->id != 0xAA55)
			{
                cantsave();
				return (1);
			}
            svcIoClose(memdev, 0);
        }
    }
	else								// Here if disk does not now have
	{									//   our bootstrap
        if (!quiet)
        {
            char bufr[80];

            sprintf(bufr, "%% MKBOOT: Installing XOS bootstrap version %d.%d",
					bootvernum, bootedtnum);
            notice(bufr);
        }
    }
    if (reloccnt != 0)					// Tell him if we did any relocation
    {
        sprintf(bufr, "%% MKBOOT: %d relocation item%s processed",
                reloccnt, (reloccnt == 1)? "": "s");
        notice(bufr);
    }

    // Now finish setting up the new boot block.

    if (level1data == &fat16data)
    {
		memcpy(((char *)&newbootblk) + 3, ((char *)oldbootblk) + 3, 52 - 3);
										// Copy header part of boot block to
										//   the new boot block
		newbootblk.devunit = *disktype;
		if (bootvernum > 6 || (bootvernum == 6 && bootedtnum >= 2))
			newbootblk.partofs = diskchar1.partoff.value;
    }
    else if (level1data == &fat32data)
    {
		memcpy(((char *)&newbootblk) + 3, ((char *)oldbootblk) + 3, 96 - 3);
										// Copy header part of boot block to
										//   the new boot block

		// Calculate the number of the first FAT block and of the first data
		//   block.

		newbootblk.fatbegin = ffat = diskchar1.partoff.value +
				newbootblk.ressec;
		newbootblk.databegin = ffat + newbootblk.ttlsecpfat *
				newbootblk.numfats - 2 * newbootblk.secpcls;
    }
    if (newbootblk.sectors != 0)
        newbootblk.ttlsec = newbootblk.sectors;

    // Now set up the configuration block

    cfgpnt->id = 0xFFFFFFFE;
    cfgpnt->timeout = autoboot ? 0xFFFF : timeout * 18;
										// Store timeout value
    strmov(cfgpnt->dftname, defprog);	// Store default program name
    if (oldbootblk->id != 0xAA55)		// Was there a previous valid boot 
										//   block?
		fname[9][2] = 0;				// No - don't show the F10=DOS item
    numitems = 0;
    width = 8;
    numpline = 5;
    num = 0;							// See how much space we need for
    do									//   each menu item
    {
        if (fname[num][2])
        {
            ++numitems;
            len = strlen(fname[num]+2);
            if (len > width)
            {
                if (len <= 12)
                {
                    width = 12;
                    numpline = 4;
                }
                else if (len <= 18)
                {
                    width = 18;
                    numpline = 3;
                }
                else if (len <= 30)
                {
                    width = 30;
                    numpline = 2;
                }
                else
                {
                    width = 48;
                    numpline = 1;
                }
            }
        }
    } while (++num < 10);
    numlines = (numitems+numpline-1)/numpline;
    begin = (numpline == 5)? 9: 8;
    line = 7;
    pos = begin;
    num = 0;							// Calculate where the menu items
    do									//   should be displayed
    {
        if (fname[num][2])
        {
            fname[num][0] = pos;
            fname[num][1] = line;
            if ((pos += (width+6)) > 65)
            {
                ++line;
                pos = begin;
            }
        }
    } while (++num < 10);
    num = 0;							// Copy the menu items to the
    do									//   bootstrap
    {
        if (fname[num][2])
            strcpy(cfgpnt->f1name + num * 44, (fname[num]));
    } while (++num < 10);
    if (numlines == 0)
        ++numlines;
    cfgpnt->numline = numlines + 2;

    if ((memdev = svcIoOpen(O_CREATE|O_TRUNCW|O_OUT|O_IN, memname, NULL)) < 0)
    {									// Create new XOS.BSF
        errclose();
        fail("Error opening bootstrap file", memdev, memname);
        return (1);
    }
    if ((rtn = svcIoOutBlock(memdev, databfr, datasize)) != datasize)
    {									// Write XOS.BSF
        errclose();
        if (rtn < 0)					// Error reported?
            fail("Error writing to bootstrap file", rtn, memname);
        else							// No - report as incomplete output
            fail("Incomplete output to bootstrap file", 0, memname);
        return (1);
    }
    if (level1data == &fat32data)
	{
		if ((rtn = svcIoSetPos(memdev, 0, 0)) < 0 ||
		    	(rtn = svcIoInBlock(memdev, databfr, 512)) < 0 ||
				(rtn = svcIoInBlockP(memdev, NULL, 0, &blkparms)) < 0)
		{
       	    fail("Error obtaining block number for level 2 code", rtn, memname);
			return (1);
		}

		printf("### level 2 code at %d\n", blkparms.blk.value);

		newbootblk.level2blk = diskchar1.partoff.value + blkparms.blk.value;
	}
    if ((rtn = svcIoInBlockP(memdev, NULL, 0, &memparm2)) < 0)
    {
        errclose();
        fail("Error setting attributes for bootstrap file", rtn, memname);
        return (1);
    }
    if ((rtn = svcIoClose(memdev, 0)) < 0) // Close XOS.BSF
    {
        errclose();
        fail("Error closing bootstrap file", rtn, memname);
        return (1);
    }
    if ((rtn = svcIoSetPos(diskdev, 0, 0)) < 0)
    {									// Position to write the boot block
        errclose();
        fail("Error setting position for writing boot block", rtn, diskname);
        return (1);
    }
    if ((rtn = svcIoOutBlock(diskdev, (char *)&newbootblk, 512)) < 0)
    {									// Write the boot block
        errclose();
        fail("Error writing boot block", rtn, diskname);
        return (1);
    }

// NEED TO CLOSE diskdev HERE!!!

    return (0);							// All done!
}

//*******************************************
// Function: runread - Read from the RUN file
// Returned: 0 if OK, 1 if error
//*******************************************

static int runread(
    char *buffer,
    long  count,
    void fail(char *str1, long code, char *str2))

{
    long rtn;

    if ((rtn = svcIoInBlock(rundev, buffer, count)) != count)
    {
        errclose();
        if (rtn < 0)					// Error reported?
            fail("Error reading master bootstrap file", rtn, runname);
        else							// No - report as unexpected EOF
            fail("Unexpected end-of-file on bootstrap file", 0, runname);
        return (1);
    }
    return (0);
}

static int getsegheader(void)

{
    if (runread((char *)&seghead, 12, fail) != 0) // Read the segment header
		return (FALSE);

    if (seghead.hdrsize < 12 || seghead.nummsc == 0)
    {
		errclose();
		fail(notvalid, 0, NULL);
		return (FALSE);
    }
    if (seghead.hdrsize > 12)			// Skip excess file header data
    {
		if (runskip(seghead.hdrsize - 12, fail) != 0)
			return (FALSE);
    }
    return (TRUE);
}


static int getmsectheader(
    MSD *data)

{
    if (runread((char *)&mschead, 36, fail) != 0) // Read the msect header
		return (FALSE);
    if (mschead.hdrsize < 36)
    {
		errclose();
		fail(notvalid, 0, NULL);
		return (FALSE);
    }
    data->dataos = mschead.dataos;		// Store position and size of data to
    data->datasz = mschead.datasz;		//   load
    data->alloc = (mschead.alloc + 15) & ~0xF; // Store amount to allocate
    data->relocos = mschead.relos;		// Store relocation data position and
    data->relocsz = mschead.relsz;		//   size
    data->offset = mschead.addr;

    if (mschead.hdrsize > 36)			// Skip excess msect header data
    {
		if (runskip(mschead.hdrsize - 36, fail) != 0)
			return (FALSE);
    }
    return (TRUE);
}


//************************************************
// Function: runskip - Skip bytes in the .RUN file
// Returned: 0 if OK 1 if error
//************************************************

static int runskip(
    long count,
    void fail(char *str1, long code, char *str2))

{
    long rtn;

    if ((rtn = svcIoSetPos(rundev, count, 1)) < 0)
    {
        errclose();
        fail("Error setting position for reading", rtn, runname);
        return (1);
    }
    return (0);
}


static void cantsave(void)

{
    fail("Can not preserve the original bootstrap.  You must reinstall\n"
			"  the original bootstrap and then install the XOS bootstrap.\n",
			0, NULL);
}

//*********************************************************
// Function: errclose - Close all open files after an error
// Returned: Nothing
//*********************************************************

void errclose(void)

{
    if (rundev != 0)
        svcIoClose(rundev, 0);
    if (memdev != 0)
        svcIoClose(memdev, 0);
    if (diskdev != 0)
        svcIoClose(diskdev, 0);
}
