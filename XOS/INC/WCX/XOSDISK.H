//------------------------------------------------------------------------------
//
//  XOSDISK.H - User-mode disk definitions.
//
//  Edit History:
//  -------------
//  09/07/92(brn) - Add comment header
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

// Define functions for the svcIoSpecFunc SVC for disks

#ifndef _XOSDISK_H_
#define _XOSDISK_H_

#ifndef _BASELINE_H_

#include <baseline.h>

#endif // _BASELINE_H_


// Define file system types - this is the same value used in the "system
//   indication" byte of a hard disk boot block - it is stored in ucb_fstype
//   for each disk unit - it is also returned as the value of the IOPAR_FSTYPE
//   I/O parameter

#define FS_DOS12  	0x01	// DOS with 12 bit FAT entries
#define FS_DOS16  	0x04	// DOS with 16 bit FAT entries
#define FS_DOSEXT 	0x05	// DOS extended partition
#define FS_DOSHP  	0x06	// DOS huge partition
#define FS_XOS		0x23	// XOS native file system
#define FS_DSS12  	0x0E1	// DOS SpeedStor compatible with 12 bit FAT entries
#define FS_DSS12L 	0x0F1	// DOS speedStor extended with 12 bit FAT entries
#define FS_DSS16  	0x0E4	// DOS SpeedStor compatible with 16 bit FAT entries
#define FS_DSS16L 	0x0F4	// DOS SpeedStor extended with 16 bit FAT entires

// Define functions for the svcIoSpecFunc SVC for disks

#define DSF_SETDS   	1	// Set disk state
#define DSF_DISMOUNT	2	// Dismount disk
#define DSF_MOUNT   	3	// Mount disk
#define DSF_FORMAT  	4	// Format disk

// Define offsets for the disk/cache status data block (returned as the
//  value of the DCSINFO DISK class characteristic)

typedef struct dcsinfodata
{
    long sbuft;		// Total number of system cache buffers
    long sbufa;		// Number of available system cache buffers
    long sbuff;		// Number of free system cache buffers
    long sbuf0;		// Number of system cache buffers with 0 users
    long sbuf1;		// Number of system cache buffers with 1 user
    long sbuf2;		// Number of system cache buffers with 2 users
    long sbuf3;		// Number of system cache buffers with 3 users
    long sbuf4;		// Number of system cache buffers with 4 users
    long sbuf5;		// Number of system cache buffers with 5 or more
			//   users
    long dbuft;		// Total number of data cache buffers
    long dbufa;		// Number of available data cache buffers
    long dbuff;		// Number of free data cache buffers
    long dbuf0;		// Number of data cache buffers with 0 users
    long dbuf1;		// Number of data cache buffers with 1 user
    long dbuf2;		// Number of data cache buffers with 2 users
    long dbuf3;		// Number of data cache buffers with 3 users
    long dbuf4;		// Number of data cache buffers with 4 users
    long dbuf5;		// Number of data cache buffers with 5 or more users
} dcsinfo_data;

#define	dcsi_SIZE	(sizeof(dcsinfo_data))

#endif	// _XOSDISK_H_
