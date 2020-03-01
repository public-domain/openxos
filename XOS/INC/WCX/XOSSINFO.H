//------------------------------------------------------------------------------
//
//  XOSSINFO.H - Definitions for the svcSysGetInfo system call for XOS
//
//  Edit History:
//  -------------
//  09/07/92(brn) - Add comment Header
//
//------------------------------------------------------------------------------

// ++++
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

#ifndef _XOSSINFO_H_
#define _XOSSINFO_H_

#ifndef _BASELINE_H_

#include <baseline.h>

#endif // _BASELINE_H_


// Define function values for the svcSysGetInfo system call

#define GSI_LKE  1	// Get LKE information
#define GSI_LAS  2	// Get linear address space information
#define GSI_DEV  3	// Get device information
#define GSI_XFF  4	// Get XFF information
#define GSI_DCS  5	// Get disk cache status information
#define GSI_TERM 6	// Get terminal information

// Define offsets in the returned data for the GSI_LKE function for the
// svcSysGetInfo system call

typedef struct lkeinfodata
{   char     name[16];		// LKE name
    unsigned long type;		// LKE type
    struct
    {   unsigned short edit;
        unsigned char  minor;
        unsigned char  major;
    }        version;		// LKE version
    struct
    {   unsigned short minor;
        unsigned short major;
    }        level; 		// LKE compatibility level
    unsigned long codeo;	// Code offset
    unsigned long codes;	// Code size
    unsigned long datao;	// Data offset
    unsigned long datas;	// Data size
    unsigned long symo;		// Symbol table offset
    unsigned long syms;		// Symbol table size
} lkeinfo_data;

#define	lkei_SIZE	(sizeof(lkeinfo_data))

// Define offsets in the returned data for the GSI_LAS function for the
// svcSysGetInfo system call

typedef struct lasinfodata
{
    pid_data pid;	// PID for process or segment which owns linear
			//  address space block (high bit is set if segment)
    long     la;	// Linear address
    short    sel;	// Selector
    short    dummy1;	// (not used)
    long     size;	// Size of linear address space block
} lasinfo_data;

#define	lasi_SIZE	(sizeof(lasinfo_data))

// Define offsets in the returned data for the GSI_DEV function for the
// svcSysGetInfo system call

typedef struct devinfodata
{
    char   cname[8];	// Class name
    char   dname[8];	// Device name
    char   tname[4];	// Type name
    ulong  gpid;	// Group PID
    ulong  ocount;	// Open count
    ulong  dcbid;	// DCB ID
    ushort outcnt;	// Output count
    ushort incnt;	// Input count
    long   dummy1[6];	// (reserved)
} devinfo_data;

#define	devi_SIZE	(sizeof(devinfo_data))

// Define offsets in the returned data for the GSI_DISK function for the
// svcSysGetInfo system call

typedef struct diskinfodata
{
    char  name[8];	// Disk name
    char  disktype;	// Disk type
    unsigned char filetype; // File system type
    short dummy1;	// (reserved)
    long  diskmisc;	// Miscellaneous disk data
    long  blksize;	// Size of a disk block (bytes)
    long  clssize;	// Size of a disk cluster (blocks)
    long  blocks;	// Total blocks on disk
    long  avail;	// Available blocks
    char  heads;	// Number of heads
    char  sects;	// Number of sectors
    short tracks;	// Number of tracks
} diskinfo_data;

#define	dski_SIZE	(sizeof(diskinfo_data))

// Define bits for diskmisc

#define DSKIM_MNTIP  0x20000000	// Mount is in progress
#define DSKIM_TKDEN  0x00C00000	// Track density
#define   TD_48TPI 0		//   48 tpi (also all hard disks)
#define   TD_96TPI 1		//   96 tpi
#define DSKIM_DBLS   0x00200000	// Disk is double sided (floppy)
#define DSKIM_M8H    0x00200000	// Disk has more than 8 heads (hard disk)
#define DSKIM_DEN    0x00180000	// Data density
#define   DN_SD    0		//   Single density (also all hard disks)
#define   DN_DD    1		//   Double density
#define   DN_HD    2		//   High density
#define DSKIM_RSIZE  0x00070000	// Record size
#define   RS_UNKWN 0		//   Unknown
#define   RS_128   1		//   128 byte records
#define   RS_256   2		//   256 byte records
#define   RS_512   3		//   512 byte records
#define   RS_1024  4		//   1024 byte records
#define DSKIM_WPROT  0x00008000	// Disk is write protected
#define DSKIM_MOUNT  0x00004000	// Disk is mounted
#define DSKIM_VALID  0x00002000	// Disk contains valid data
#define DSKIM_NOTF   0x00001000	// Disk is not file structured
#define DSKIM_MEDIA  0x00000800	// Media type is specified
#define DSKIM_RECAL  0x00000400	// Need to recalibrate
#define DSKIM_HFTRK  0x00000200	// Have 48 tpi disk in 96 tpi drive
#define DSKIM_MOTON  0x00000100	// Motor is on (floppy only)
#define DSKIM_REMOVE 0x00000080	// Disk is removeable
#define DSKIM_MEDIAS 0x00000040	// Unit has media change sensor

#endif	// _XOSSINFO_H_
