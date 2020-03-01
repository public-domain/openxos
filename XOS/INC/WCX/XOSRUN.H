//------------------------------------------------------------------------------
//
//  XOSRUN.H - Run file header format
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

// Define offsets in the RUN file header

#ifndef _XOSRUN_H_
#define _XOSRUN_H_

# include <_prolog.h>

// Define image types

#define IT_RUN386	1	// 80386 .RUN file
#define IT_RUN286	2	// 80286 .RUN file (??)
#define IT_RUN86 	3	// 8086 .RUN file (??)
#define IT_EXE   	8	// .EXE file
#define IT_COM   	9	// .COM file
#define IT_BATCH 	15	// .BAT file

// Define offsets in the relocation data buffer

#define rd_EAX    	0	// Initial value for EAX
#define rd_ECX    	4	// Initial value for ECX
#define rd_EDX    	8	// Initial value for EDX
#define rd_EBX    	12	// Initial value for EBX
#define rd_ESP    	16	// Initial value for ESP
#define rd_EBP    	20	// Initial value for EBP
#define rd_ESI    	24	// Initial value for ESI
#define rd_EDI    	28	// Initial value for EDI
#define rd_EIP    	32	// Initial value for EIP
#define rd_EFR    	36	// Initial value for EFR
#define rd_DS     	40	// Initial value for DS
#define rd_ES     	44	// Initial value for ES
#define rd_FS     	48	// Initial value for FS
#define rd_GS     	52	// Initial value for GS
#define rd_CS     	56	// Initial value for CS
#define rd_SS     	60	// Initial value for SS
#define rd_size   	64	// Total size of loaded program
#define rd_nseg   	68	// Number of segments
#define rd_nmsect 	70	// Number of msects
#define rd_data   	72	// Start of relocation data
#define rd_MINSIZE	80	// Minimum size for the relocation buffer

// Define offsets in the RUN file header

#define ih_id      	0	// (2) RUN file ID bytes
#define ih_ver     	2	// (1) RUN file version
#define ih_proc    	3	// (1) Processor type
#define ih_type    	4	// (1) RUN file type
				// (1) Not used
#define ih_hdrsize 	6	// (1) Header byte count
#define ih_numseg  	7	// (1) Number of segments in file
#define ih_start   	8	// (4) Start address offset
#define ih_startms 	12	// (1) Msect number for start address
				// (3) Not used
#define ih_debug   	16	// (4) Debugger address offset
#define ih_debugms 	20	// (1) Msect number for debugger address
				// (3) Not used
#define ih_stack   	24	// (4) Stack address offset
#define ih_stackms 	28	// (1) Msect number for stack address
				// (3) Not used
#define ih_exportos	32	// (4) Offset in file for exported symbol table
#define ih_exportno	36	// (4) Number of entries in exported symbol table
#define ih_importos	40	// (4) Offset in file for imported symbol table
#define ih_importno	44	// (4) Number of entries in imported symbol table
#define ih_fileid  	48	// (10)
#define ih_SIZE  	(64-6)	// Size of RUN file header (excluding first 6 bytes)

// Define offsets in each segment header

struct runseg
{
    uchar  hdrsize;		// Header byte count
    uchar  nummsc;		// Number of msects in segment
    uchar  status;		// Segment status byte
    uchar  linked;		// Linked segment
    uchar  type;		// Segment type
    uchar  priv;		// Privilege level
    ushort select;		// Selector for segment
    ulong  addr;		// Physical address or modulus
};

#define sh_SIZE (sizeof(struct runseg)) // Size of segment header

// Define offsets in each msect header

struct runmsect
{
    uchar hdrsize;		// Header byte count
    uchar dummy1;		// (not used)
    uchar status;		// Msect status byte
    uchar dummy2;		// (not used)
    uchar type;			// Msect type
    uchar privilege;  	        // Privilege level
    short dummy3;		// (not used)
    ulong addr; 		// Address or modulus in segment
    ulong avail;		// Bytes of address space to leave available for
                                //   msect
    ulong alloc;		// Bytes to allocate for msect
    ulong dataos;		// Offset from start of RUN file of data to load
    ulong datasz;		// Number of bytes to load
    ulong relos;		// Offset from start of RUN file for relocation
				        //   information
    ulong relsz;		// Number of relocation items
};

#define mh_SIZE (sizeof(struct runmsect)) // Size of msect header

// Define values for the #define ih_proc byte

#define RUNP_LOW   	00		// Unspecified low-order-byte first processor
#define RUNP_Z80   	01		// Z80
#define RUNP_8086  	02		// 8086
#define RUNP_80186 	03		// 80186
#define RUNP_80286 	04		// 80286
#define RUNP_80386 	05		// 80386
#define RUNP_80486      06		// 80486
#define RUNP_HIGH  	80		// Unspecified high-order-byte first processor
#define RUNP_68000 	81		// 68000
#define RUNP_68010 	82		// 68010
#define RUNP_68020 	83		// 68020
#define RUNP_68030 	84		// 68030

// Define values for the #define ih_type byte

#define RUNT_ALONE  	1		// Stand-alone image
#define RUNT_XOSUSER	2		// XOS user mode image
#define RUNT_MSDOS  	3		// MS-DOS image
#define RUNT_XOSLKE 	7		// XOS loadable kernel extension

// Define bits for the sh_sts byte

#define SS_LARGE  	0x80	// Large segment
#define SS_32BIT  	0x40	// 32-bit segment
#define SS_CONFORM	0x20	// Conformable segment
#define SS_READ   	0x10	// Readable segment
#define SS_WRITE  	0x08	// Writeable segment
#define SS_SHARE  	0x02	// Shareable or global segment
#define SS_ADDR   	0x01	// Address specified

// Define values for the sh_type byte

#define ST_DATA 	1	// Data segment
#define ST_CODE 	2	// Code segment
#define ST_STACK	3	// Stack segment
#define ST_REAL 	5	// Real mode segment

// Define bits for the mh_sts byte

#define MS_CONF   	0x20	// Conformable msect
#define MS_READ   	0x10	// Readable msect
#define MS_WRITE  	0x08	// Writable msect
#define MS_SHAR   	0x02	// Sharable msect
#define MS_ABS    	0x01	// Absolute msect


struct runhead
{
    ushort magic;		// Magic number (0x22D7)
    uchar  version;		// Version number
    uchar  proctype;		// Processor type
    uchar  imagetype;		// Image type
    uchar  notused0;		// Not used
    uchar  hdrsize;		// Header size
    uchar  numsegs;		// Number of segments
    ulong  start;		// Value for EIP
    uchar  startmsect;		// Msect number for EIP value
    char   notused1[3];		// Not used
    ulong  debug;		// Debugger address
    uchar  debugmsect;		// Debugger address msect number
    char   notused2[3];		// Not used
    ulong  stack;		// Value for stack pointer
    uchar  stkmsect;		// Msect number for stack pointer
    char   notused3[3];		// Not used
    ulong  exportos;		// Offset in file for exported symbol table
    ulong  exportsz;		// Size of exported symbol table
    ulong  importos;		// Offset in file for imported symbol table
    ulong  importsz;		// Size of imported symbol table
    ulong  fileid[3];
    ulong  notused4;
};

# include <_epilog.h>

#endif	// _XOSRUN_H_
