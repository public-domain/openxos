//------------------------------------------------------------------------------
//
//  XOSXAFS.H - Parameters for XOS File System (XFS)
//
//  Edit History:
//  -------------
//  09/05/88(brn) - Created first version
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

#ifndef _XOSXAFS_H_
#define _XOSXAFS_H_

#ifndef _BASELINE_H_

#include <baseline.h>

#endif // _BASELINE_H_


#ifndef _TIME_S
#define _TIME_S

typedef struct sysdttm
{
    unsigned long time;
    long     date;
} time_s;

#endif // _TIME_S

// Define structure XOS File System disk boot block

typedef struct bootblk
{   uchar  xb_jump[3];		// Jump instruction to start of bootstrap code
    uchar  xb_xxx[5];		// Not used, must be 0
    char   xb_xfssig[8];	// XOS file system signature = 'XOS**XFS'
    long   xb_version;		// File system versino (currently = 0x10000)
    ushort xb_sects;		// Number of sectors/track
    ushort xb_heads;		// Number of heads
    long   xb_clyns;		// Number of cylinders
    ushort xb_sectsz;		// Size of a sector (must be 512)
    ushort xb_homblk;		// Block number of home block (usually 16)
    ushort xb_btxblk;		// Block number of start of extended boot area
    ushort xb_btxsize;		// Size (in blocks) of extended boot area
    uchar  xb_btcode[474];	// Bootstrap code
    ushort xb_btsig;		// Boot block signature bytes (always = 0AA55)
} BOOTBLK;

// Define structure for the XOS File System disk home block

typedef struct homblk
{   char   hom_label[4];	// Block name = 'HOM*'
    ulong  hom_selfl;		// Block number of this block (low 32 bits)
    ulong  hom_selfh;		// Block number of this block (high 32 bits)
    ulong  hom_firstclus;	// Block number of first cluster
    ulong  hom_numclus;		// Number of clusters on disk
    ushort hom_clussize;	// Blocks per cluster
    ushort hom_grpsize;		// Default group size for disk
    ushort hom_numsats;		// Number of SATs
    uchar  hom_flag;		// Format flag bits
    uchar  hom_pntsize;		// Size (in bytes) of the cluster pointers
    ulong  hom_size;		// Total number of clusters on disk
    ulong  hom_root;		// Cluster number for FIB for root directory
    char   hom_vollabel[32];	// Volume label
    char   hom_vodcode[16];	// Volume code
    ushort hom_wrpcc;		// Write pre-comp cylinder number
    ushort hom_wrcur;		// Reduce write current cylinder number
    char   hom_xxx[12];		// Reserved, should be 0
    ulong  hom_sattbl[388/4];	// SAT block pointer table
} HOMBLK;

typedef struct fibpnt2
{   uchar  cnt;
    ushort pnt;
} FIBPNT2;

typedef struct fibpnt3
{   uchar  cnt;
    ushort pntl;
    uchar  pnth;
} FIBPNT3;

typedef struct fibpnt4
{   uchar cnt;
    ulong pnt;
} FIBPNT4;

// Define structure for the XOS File System disk SAT block

typedef struct satblk
{   char  sat_label[4];		// Block name = 'SAT*'
    ulong sat_selfl;		// Block number of this block (low 32 bits)
    ulong sat_selfh;		// Block number of this block (high 32 bits)
    uchar sat_data[500];	// Start of data area
} SATBLK;

// Define structure for the file index block (FIB)

typedef struct fibblk
{
    // The following items are used in all FIB's

    char   fib_label[4];	// Block name (FIB*)
    ulong  fib_selfl;		// Block number of this block (low 32 bits)
    ulong  fib_selfh;		// Block number of this block (high 32 bits)
    ulong  fib_floor;		// Cluster number for first file pointer in
				//   this block
    ulong  fib_roof;		// Cluster number for first file pointer above
				//   this block
    ulong  fib_base;		// Base FIB cluster number
    ulong  fib_next;		// Next FIB cluster number
    ulong  fib_prev;		// Previous FIB cluster number
    ulong  fib_number;		// FIB number

    // The following offsets are only used in the base FIB

    ulong  fib_size;		// Number of bytes written
    ulong  fib_mxsz;		// Maximum number of bytes written
    ulong  fib_alloc;		// Number of clusters allocated
    ulong  fib_dirpos;		// Position of entry in directory
    ulong  fib_dirfib;		// Directory FIB cluster number
    uchar  fib_name[32];	// File name
    uchar  fib_attrib;		// File attribute bits
    uchar  fib_grpsize;		// Group size for allocation
    ushort fib_xxx2;
    ulong  fib_prot;		// Protection
    time_s fib_cdt;		// Creation date/time
    time_s fib_mdt;		// Modify date/time
    time_s fib_adt;		// Access date/time
    uchar  fib_owner[16];	// Owner name
    uchar  fib_group[16];	// Group name
    union
    {	FIBPNT2 p2;
	FIBPNT3 p3;
	FIBPNT4 p4;
    } fib_f1pnt[1];		// First file pointer in base FIB
} FIBBLK;

// Define structures for directory blocks

// Define structure for first blcck of a directory entry

typedef struct de1blk
{   uchar de1_type;		// Block type: 0 = not in use, 1 = first block
				//   of a directory entry
    uchar de1_status;		// Status bits
    long  de1_fibcls;		// Cluster number of file index block
    uchar de1_name[26];		// First part of name
} DE1BLK;

// Define structure for additional blocks of a directory entry

typedef struct denblk
{   uchar den_type;		// Block type: 2 or greater, indicates position
				//   of this block in the directory entry
    uchar den_xxx;		// Reserved, must be 0
    uchar den_name[30];		// File name characters
} DENBLK;

typedef struct dirblk
{   union
    {	DE1BLK ent1;
	DENBLK entn;
    };
} DIRBLK;

// Define bits for de1_status (except for DEB$UNICODE, these bits are the same
//   as the DOS file attribute bits)

#define DEB_FILE    0x80	// File is a file
#define DEB_DIR     0x10	// File is a directory
#define DEB_PERM    0x08	// File is permanet
#define DEB_SYSTEM  0x04	// File is a system file
#define DEB_HIDDEN  0x02	// File is hidden
#define DEB_UNICODE 0x01	// File name is Unicode

// Define offsets in the file access block (FAB)

struct fab_status
{
    uchar fab_status;		// Status bits
    char fab_index; 		// Size index for FAB block buffer
    ushort fab_number;		// Number of FIB entries
    ulong fab_lock;  		// Offset of lock table
    ulong fab_fibtbl;		// FIB table
};

// Define structure for the FIB table entries

typedef struct fibtblent
{   long fte_roof;		// First cluster in file not pointed to by this FIB
    long fte_cluster;		// Cluster were this FIB is located
} FIBTBLENT;

// Define structure for the FIB table

typedef struct fibtbl
{   long fibtbl_next;		// Offset of next FIB table block
    long fibtbl_data;		// First FIB table entry in this block
} FIBTBL;

// Define structure for the file access block (FAB) - These are allocated in
//   QELs (64 bytes) so the first block of the fibtbl contains up to 6 entries
//   and each additional block contains up to 7 entries.

typedef struct fabblk
{   uchar  fab_status;		// Status bits
    uchar  fab_xxx;		// Reserved, must be 0
    ushort fab_number;		// Number of FIB block in file
    long   fab_lock;		// Offset of lock table
    FIBTBL fab_fibtbl[6];	// Start of the FIB table
} FABBLK;

// Define bits for fab_status

#define FS_MODIFIED 0x40	// File has been modified
#define FS_ADTSET   0x20	// Access date/time value has been set
#define FS_CDTSET   0x10	// Creation date/time value has been set
#define FS_MDTSET   0x08	// Modify date/time value has been set

#endif  // _XOSXAFS_H_
