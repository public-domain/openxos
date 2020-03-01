//------------------------------------------------------------------------------
//
//  XOS.H - Standard XOS I/O header
//
//  Edit History:
//  -------------
//  09/05/92(brn) - Added header to include file
//   6Jun95 (fpj) - Resynchronized with XOS.PAR.
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

#ifndef _XOS_H_
#define _XOS_H_

# include <_prolog.h>

// Define general limit values

#define FILESPCSIZE 256

// Define some ASCII characters

#define EOT	0x04		// End of text
#define BEL	0x07		// Bell
#define BSP	0x08		// Backspace
#define HT 	0x09		// Horizontal tab
#define LF 	0x0A		// Line feed
#define FF 	0x0C		// Form feed
#define CR 	0x0D		// Carriage return
#define SFO	0x0E		// Shift out
#define SFI	0x0F		// Shift in
#define XON	0x11		// XON (^Q)
#define XOF	0x13		// XOF (^S)
#define SYN	0x16		// Sync
#define SUB	0x1A		// SUB (used as EOF flag in files)
#define ESC	0x1B		// ESC
#define SPA	0x20		// Space
#define LA 	0x3C		// Left angle
#define RA 	0x3E		// Right angle
#define LB 	0x7B		// Left brace
#define RB 	0x7D		// Right brace
#define DEL	0x7F		// Delete (Rub-out)

// Define global selectors which user programs can use

#define GS_REAL16  	0x53	// Real mode "segment" as a 16 bit data segment
#define GS_REAL32  	0x5B	// Real mode "segment" as a 32 bit data segment
#define GS_USERCODE	0x00A0	// Global user code segment
#define GS_VDOSDATA	0x00B8	// Segment which maps the DOS data area

// Define offset in the real mode data area

#define rmda_base 	0x0EC000
#define rmda_stack	0x0ECFE0  // Real mode stack
#define rmda_vmESP	0x0ECFF0  // Current protected mode stack pointer
#define rmda_vmSS 	0x0ECFF4
#define rmda_pmESP	0x0ECFF8  // Current real mode stack pointer
#define rmda_pmSS 	0x0ECFFC

// Define segment types and associated bits

#define ST_TOP      0x80000000  // Search backwards to allocate segment

#define ST_32RODATA 	1	// 32 bit read only data segment
#define ST_32RWDATA 	2	// 32 bit read/write data segment
#define ST_32STACK  	3	// 32 bit stack segment
#define ST_32NXOCODE	4	// 32 bit execute only normal code segment
#define ST_32NXRCODE	5	// 32 bit execute/read normal code segment
#define ST_32CXOCODE	6	// 32 bit execute only conformable code segment
#define ST_32CXRCODE	7	// 32 bit execute/read conformable code segment
#define ST_16RODATA 	9	// 16 bit read only data segment
#define ST_16RWDATA 	10	// 16 bit read/write data segment
#define ST_16STACK  	11	// 16 bit stack segment
#define ST_16NXOCODE	12	// 16 bit execute only normal code segment
#define ST_16NXRCODE	13	// 16 bit execute/read normal code segment
#define ST_16CXOCODE	14	// 16 bit execute only conformable code segment
#define ST_16CXRCODE	15	// 16 bit execute/read conformable code segment
#define ST_MAX      	15

// Define page type bits

#define PG_VIRTUAL	0x08	// Virtual page
#define PG_EXECUTE	0x04	// Executable page (not supported)
#define PG_WRITE  	0x02	// Writable page
#define PG_READ   	0x01	// Readable (always set)

// Define termination codes (positive values indicate process has been stopped,
//   negative values indicate process has been killed)

#define TC_EXIT   	1	// Normal exit
#define TC_KILL   	2	// Killed
#define TC_PIPE   	3	// No input available from pipe
#define TC_NOVECT 	4	// No user vector setup for condition
#define TC_BDUSTK 	5	// Bad user stack address
#define TC_NOMEM  	6	// No memory available
#define TC_UNIMOP 	7	// Unimplemented operation
#define TC_RUNFAIL	8	// RUN failure
#define TC_SUBTSK 	11	// Sub-task error
#define TC_MEMCRPD	12	// DOS memory allocation data corrupted
#define TC_DPMI   	13	// Fatal DPMI error
#define TC_NODOS  	14	// DOS environment not set up
#define TC_DOSBADE	15	// Invalid error code returned for DOS function

// Define software interrupt numbers

// Interrupts 000h through 0FFh are protected mode INTs

// Interrupts 100h through 11Fh are protected mode processor traps

#define VECT_PDIVERR 	0x100	// Divide error
#define VECT_PDEBUG  	0x101	// Debug trap
#define VECT_PNMI    	0x102	// Non-maskable interrupt
#define VECT_PBRKPNT 	0x103	// Breakpoint
#define VECT_PINTO   	0x104	// INTO instruction trap
#define VECT_PBOUND  	0x105	// BOUND instruction trap
#define VECT_PILLINS 	0x106	// Illegal instruction trap
#define VECT_PFPPNAVL	0x107	// Floating pointer processor not available trap
#define VECT_PDBLEXP 	0x108	// Double exception
#define VECT_PFPPSOVR	0x109	// Floating point processor segment overrun
#define VECT_PITSS   	0x10A	// Invalid task state segment
#define VECT_PSEGNP  	0x10B	// Segment not present
#define VECT_PSTKERR 	0x10C	// Stack error
#define VECT_PPROT   	0x10D	// Memory protection error
#define VECT_PPAGEFLT	0x10E	// Page fault

#define VECT_PFPUERR 	0x110	// Floating point processor error
#define VECT_PALNCHK 	0x111	// Alignment check

// Interrupts 120h through 13Fh are real mode processor traps

#define VECT_RDIVERR 	0x120	// Divide error
#define VECT_RDEBUG  	0x121	// Debug trap
#define VECT_RNMI    	0x122	// Non-maskable interrupt
#define VECT_RBRKPNT 	0x123	// Breakpoint
#define VECT_RINTO   	0x124	// INTO instruction trap
#define VECT_RBOUND  	0x125	// BOUND instruction trap
#define VECT_RILLINS 	0x126	// Illegal instruction trap
#define VECT_RFPPNAVL	0x127	// Processor extension not available trap
#define VECT_RDBLEXP 	0x128	// Double exception
#define VECT_RFPPSOVR	0x129	// Processor extension segment overrun
#define VECT_RITSS   	0x12A	// Invalid task state segment
#define VECT_RSEGNP  	0x12B	// Segment not present
#define VECT_RSTKERR 	0x12C	// Stack error
#define VECT_RPROT   	0x12D	// Memory protection error
#define VECT_RPAGEFLT	0x12E	// Page fault

#define VECT_RFPUERR 	0x130	// Floating point error
#define VECT_RALNCHK 	0x131	// Alignment check

// Interrupts 140h through 15Fh are XOS traps

#define VECT_EXIT    	0x140	// Process termination trap
#define VECT_CHILD   	0x141	// Child died
#define VECT_CNTC    	0x142	// Control C
#define VECT_CNTP    	0x143	// Control P
#define VECT_HNGUP   	0x144	// Controlling terminal hung up

// Interrupts 160h through 1FFh are reserved

// Interrupts 200h through 2FFh are real mode INTs

// Define vector type values

#define VT_NONE   	0	// None
#define VT_XOSS   	1	// XOS signal vector
#define VT_XOST   	2	// XOS trap vector
#define VT_HWS16  	3	// 16-bit hardware signal vector
#define VT_HWS32  	4	// 32-bit hardware signal vector
#define VT_HWT16  	5	// 16-bit hardware trap vector
#define VT_HWT32  	6	// 32-bit hardware trap vector
#define VT_DPMI16O	7	// DPMI v0.9 16-bit CPU exception vector
#define VT_DPMI32O	8	// DPMI v0.9 32-bit CPU exception vector
#define VT_DPMI16N	9	// DPMI v1.0 16-bit CPU exception vector
#define VT_DPMI32N	10	// DPMI v1.0 32-bit CPU exception vector

// Define offsets in the buffer filled in by the UF_GTCFG svcUtility function

#define cf_totmem	0	// (4) Total memory in system (pages)
#define cf_avlmem	4	// (4) Available memory in system (pages)
#define cf_hrddsk	8	// (2) Number of hard disk units
#define cf_flpdsk	10	// (2) Number of floppy disk units
#define cf_serprt	12	// (2) Number of serial ports
#define cf_parprt	14	// (2) Number of parallel ports
#define cf_SIZE  	16

// Define standard device handles

#define DH_STDIN 	1	// Standard input device
#define DH_STDOUT	2	// Standard output device
#define DH_STDERR	3	// Standard error output device
#define DH_STDPRN	4	// Standard listing output device
#define DH_STDTRM	5	// Controlling terminal for session (cannot be
						//  redirected)
#define DH_FV    	6	// First variable device handle

// Define offsets in the block which specifies a device characteristics value

#define dcv_type   	0	// (1) Description byte
#define dcv_vallen 	1	// (1) Length of value field
#define dcv_name   	2	// (8) Name
#define dcv_infopnt	10	// (8) Address of information buffer
#define dcv_ibfrlen	18	// (2) Size of the information buffer
#define dcv_istrlen	20	// (2) Length of the information string
						// Note:  Following offsets are defined assuming
						//	that no information pointer is present -
						//	if there is an information pointer, add
						//	12t to each offset
#define dcv_value  	10	// (8) Numeric value or address of string value
						//  buffer
#define dcv_vbfrlen	18	// (2) Size of the string value buffer
#define dcv_vstrlen	20	// (2) Length of the returned string value
#define dcv_SIZE   	22	// Total size of a long string value entry

// Define offsets in the block which specifies an add unit characteristics value

#define aucv_type  	0	// (1) Description byte
#define aucv_name  	1	// (7) Name
#define aucv_value 	8	// (8) Numeric value or address of string value
						//	 buffer
#define aucv_valsz 	16	// (2) Size of the string value buffer
#define aucv_vallen	18	// (2) Length of the returned string value

// Define offsets in the queued IO functions argument block

// Define queued IO argument block structure (QAB)

typedef struct qab
{
    ushort    qab_func;		// Function
    ushort    qab_status;	// Returned status
    long      qab_error;	// Error code (returned by all functions)
    long      qab_amount;	// Amount transfered or items processed
    long      qab_handle;	// Device handle (returned by open, used by all
							//  others)
    ushort    qab_vector;	// Vector for interrupt
    uchar     qab_level;	// Signal level for direct I/O wait (if
							//  QFNC_LEVEL set)
    uchar     qab_prevlvl;	// Previous signal level (internal XOS use only)
    ulong     qab_option;	// Options or command
    long      qab_count;	// Count for transfer
    char far *qab_buffer1;	// Address of first data buffer
    short     qab_buffer1x;
    char far *qab_buffer2;	// Address of second data buffer
    short     qab_buffer2x;
    void far *qab_parm;		// Address of parameter area
    short     qab_parmx;
} QAB;

#define type_qab QAB		// For compatability with old name

// Define values for qab_func for the svcIoQueue system service

#define QFNC_WAIT     	0x8000	// Wait until function is complete
#define QFNC_DIO      	0x4000	// Direct IO (not queued)
#define QFNC_LEVEL    	0x2000	// Set signal level from qab_level when waiting
								//   for direct
#define QFNC_CONT     	0x1000	// Continued operation (internal use only,
								//   should never
#define QFNC_OOBD     	0x0400	// Out-of-band data (output functions only)
#define QFNC_POLL     	0x0200	// Poll (output functions for half-duplex
								//   devices only)
#define QFNC_OPEN     	1		// Open device/file
#define	QFNC_DEVPARM  	2		// Device parameter function

#define QFNC_DEVCHAR  	4		// Device characteristics
#define QFNC_DELETE   	5		// Delete file
#define QFNC_RENAME   	6		// Rename file

#define QFNC_PATH     	8		// Path functions
#define QFNC_CLASSFUNC	9		// Class functions

#define QFNC_INBLOCK  	12		// Input block

#define QFNC_OUTBLOCK 	14		// Output block
#define QFNC_OUTSTRING	15		// Output string

#define QFNC_TRMFUNC  	17		// Terminal functions

#define QFNC_SPECIAL  	19		// Special device functions
#define QFNC_LABEL    	20		// Get device label
#define QFNC_COMMIT   	21		// Commit data
#define QFNC_CLOSE    	22		// Close file

// Define values for qab_func for the svcIoRun system service

#define RFNC_WAIT     	0x8000	// Wait until function is complete
#define RFNC_RUN      	1		// Run program
#define RFNC_LOAD     	2		// Load overlay

// Define bits used in qab_status

#define QSTS_DONE  	0x8000		// Operation is complete
#define QSTS_ACTIVE	0x4000		// Operation is active
#define QSTS_WAIT  	0x2000		// Should wait for device (XOS internal use)
#define QSTS_REDO  	0x1000		// Operation should be re-done (XOS internal
								//   use)
#define QSTS_FINAL 	0x0020		// Final input
#define QSTS_EXCPTN	0x0010		// Exception event has occured
#define QSTS_OOBD  	0x0008		// Out-of-band input data
#define QSTS_CANCEL	0x0004		// Operation canceled before started
#define QSTS_ABORT 	0x0002		// Operation aborted
#define QSTS_TRUNC 	0x0001		// Data truncated

// Each parameter consists of a two byte header followed by a 0 to 32 byte value

// Define values for the first parameter header byte - this byte specifies the
//   format of the parameter value and also contains the system's error
//   indication - these values also apply for the first device characteristics
//   header byte

#define PAR_SET    	0x80	// Set (use) value of this parameter
#define PAR_GET    	0x40	// Get (return) value of this parameter
							//   Both PAR_SET and PAR_GET clear indicates
							//   the end of the parameter list
#define PAR_ERROR  	0x20	// Error in this parameter
#define PAR_INFO   	0x10	// Return parameter information
#define PAR_REP    	0x0F	// Value representation
#define   REP_DECV   	1	//   Decimal value
#define   REP_HEXV   	2	//   Hex value
#define   REP_OCTV   	3	//   Octal value
#define   REP_BINV   	4	//   Binary value
#define   REP_DECB   	5	//   Decimal bytes
#define   REP_HEXB   	6	//   Hex bytes
#define   REP_OCTB   	7	//   Octal bytes
#define   REP_VERN   	8	//   Version number (8 bits . 8 bits . 16 bits)
#define   REP_TIME   	9	//   Time value
#define   REP_DATE   	10	//   Date value
#define   REP_DT     	11	//   Date/time value
#define   REP_DATAB  	12	//   Data bytes
#define   REP_DATAS  	13	//   Data string
#define   REP_TEXT   	14	//   Text bytes
#define   REP_STR    	15	//   String value

// The second header byte specifies the length of the value in the parameter list
//   in bytes - a value of 0FFh initicates a long string value (8 byte pointer
//   followed by 2 byte buffer length and 2 byte string length - buffer length is
//   used by system, string length field set by system if IOPAR_GET specified

#define SIZE_LNGSTR	0x0FF	// Long string value

// Define values for the 3rd and 4th parameter header bytes - these byte specify
//   the parameter being set or obtained - values below 8000h are common to all
//   devices, values above 128t are device dependent and may only be used
//   following a IOPAR_CLASS parameter with PAR_SET set which specifies the
//   actual device class (this is intended to make sure that the intended
//   function is really done!)

#define IOPAR_GENBASE     0x0000
#define IOPAR_FILOPTN     0x0001 //  s4  File option bits
#define IOPAR_FILSPEC     0x0002 // g S  File specification string
#define IOPAR_DEVSTS      0x0003 // g 4  Device status
#define IOPAR_UNITNUM     0x0004 // g 4  Unit number
#define IOPAR_GLBID       0x0005 // g 16 Global device ID
#define IOPAR_DELAY       0x0006 //  s4  IO delay value (fractional days)
#define IOPAR_TIMEOUT     0x0007 //  s4  IO time-out value (fractional days)
#define IOPAR_INPSTS      0x0008 // g 1  Device input ready status
#define IOPAR_OUTSTS      0x0009 // g 1  Device output ready status
#define IOPAR_INPQLMT     0x000A // gs1  Input queue limit
#define IOPAR_OUTQLMT     0x000B // gs1  Output queue limit
#define IOPAR_SIGVECT1    0x000C // gs1  First signal vector
#define IOPAR_SIGVECT2    0x000D // gs1  Second signal vector
#define IOPAR_SIGDATA     0x000E // gs4  Signal data
#define IOPAR_NUMOPEN     0x000F // g 2  Number of times device is open
#define IOPAR_BUFRLMT     0x0010 // gs2  Internal buffering limit

// Define file system IO parameters

#define IOPAR_FILBASE     0x0100
#define IOPAR_DIRHNDL     0x0101 //  s2  Directory handle for search
#define IOPAR_SRCATTR     0x0102 //  s2  File attributes for search
#define IOPAR_FILATTR     0x0103 // gs2  File attributes
#define IOPAR_DIROFS      0x0104 // gs4  Directory offset for search
#define IOPAR_ABSPOS      0x0105 // gs4  Absolute pos. in file
#define IOPAR_RELPOS      0x0106 // gs4  Relative pos. in file (returns abs.)
#define IOPAR_EOFPOS      0x0107 // gs4  Pos. in file rel. to EOF (returns abs.)
#define IOPAR_VBOF        0x0108 // gs4  Virtual beginning of file position
#define IOPAR_LENGTH      0x0109 // gs4  Written length of file (bytes)
#define IOPAR_REQALLOC    0x010A // gs4  Request file space allocation (bytes)
#define IOPAR_RQRALLOC    0x010B // gs4  Require file space allocation (bytes)
#define IOPAR_GRPSIZE     0x010C // gs4  Allocation group size (bytes)
#define IOPAR_ADATE       0x010D // gs8  Last access date/time
#define IOPAR_CDATE       0x010E // gs8  Creation date/time
#define IOPAR_MDATE       0x010F // gs8  Modify date/time
#define IOPAR_PROT        0x0110 // gs4  File protection
#define IOPAR_OWNER       0x0111 // gsS  Owner name
#define IOPAR_USER        0x0112 // gsS  User name for access
#define IOPAR_SETLOCK     0x0113 // gs8  Set file lock
#define IOPAR_CLRLOCK     0x0114 // gs8  Clear file lock
#define IOPAR_CLSTIME     0x0115 // gs4  Close time value
#define IOPAR_CLSNAME     0x0116 //  sS  Close name
#define IOPAR_CLSMSG      0x0117 //  s16 Close message destination
#define IOPAR_SHRPARMS    0x0118 // gs4  DOS file sharing parameter values
#define IOPAR_ACSNETWK    0x0119 //  s0  Use network access field

// Define terminal IO parameters

#define IOPAR_TRMBASE     0x0200
#define IOPAR_TRMSINPMODE 0x0201 // gs4  Set input modes
#define IOPAR_TRMCINPMODE 0x0202 // gs4  Clear input modes
#define IOPAR_TRMSOUTMODE 0x0203 // gs4  Set output modes
#define IOPAR_TRMCOUTMODE 0x0204 // gs4  Clear output modes
#define IOPAR_TRMBFRLIMIT 0x0205 //  s4  Input buffer limit value
#define IOPAR_TRMCLRBUFR  0x0206 //  s1  Clear buffer(s)
#define IOPAR_TRMCURTYPE  0x0207 // gs2  Cursor type
#define IOPAR_TRMCURPOS   0x0208 // gs4  Cursor position
#define IOPAR_TRMDISPAGE  0x0209 // gs1  Display page
#define IOPAR_TRMSPSTATUS 0x020A // g 2  Serial port status
#define IOPAR_TRMSPBREAK  0x020B // gs1  Break control
#define IOPAR_TRMSPMODEM  0x020C //  s1  Modem control
#define IOPAR_TRMSETDFC   0x020D
#define IOPAR_TRMCLRDFC   0x020E
#define IOPAR_TRMLSTDFC   0x020F
#define IOPAR_TRMALLDFC   0x0210
#define IOPAR_TRMCCVECT   0x0211 // gs2  Control-C signal vector
#define IOPAR_TRMCCDATA   0x0212 // gs2  Control-C signal data
#define IOPAR_TRMCPVECT   0x0213 // gs2  Control-P signal vector
#define IOPAR_TRMCPDATA   0x0214 // gs2  Control-P signal data
#define IOPAR_TRMHUVECT   0x0215 // gs2  Hang-up signal vector
#define IOPAR_TRMHUDATA   0x0216 // gs2  Hang-up signal data

// Define disk IO parameters

#define IOPAR_DSKBASE     0x0300
#define IOPAR_DSKFSTYPE   0x0301 // g 1  File system type
#define IOPAR_DSKSECTSIZE 0x0302 // g 2  Sector size
#define IOPAR_DSKCLSSIZE  0x0303 // g 1  Cluster size
#define IOPAR_DSKTTLSPACE 0x0304 // g 4  Total space (in clusters)
#define IOPAR_DSKAVLSPACE 0x0305 // g 4  Available space (in clusters)
#define IOPAR_DSKNUMHEAD  0x0306 // g 1  Number of heads
#define IOPAR_DSKNUMSECT  0x0307 // g 1  Number of sectors
#define IOPAR_DSKNUMCYLN  0x0308 // g 1  Number of cylinders
#define IOPAR_DSKBLOCK    0x0309 // g 4  Get last disk block accessed

// Define tape IO parameters

#define IOPAR_TAPBASE     0x0400
#define IOPAR_TAPRECMIN   0x0401 // g 4  Minimum record length
#define IOPAR_TAPRECMAX   0x0402 // g 4  Maximum record length
#define IOPAR_TAPRECLEN   0x0403 // gs4  Current fixed record length
#define IOPAR_TAPDENSITY  0x0404 // gs4  Tape density
#define IOPAR_TAPGAPLEN   0x0405 // gs1  Gap length
#define IOPAR_TAPBFRMODE  0x0406 // gs1  Buffering mode

// Define network IO parameters

#define IOPAR_NETBASE     0x0500
#define IOPAR_NETSUBUMASK 0x0501 // g 4  Sub-unit bit mask
#define IOPAR_NETPROTOCOL 0x0502 // gs2  Protocol
#define IOPAR_NETLCLPORT  0x0503 // gs2  Local port number
#define IOPAR_NETRMTHWAS  0x0504 // gs8  Remote hardware address (send)
#define IOPAR_NETRMTHWAR  0x0505 // g 8  Remote hardware address (receive)
#define IOPAR_NETRMTNETAS 0x0506 // gs4  Remote network address (send)
#define IOPAR_NETRMTNETAR 0x0507 // g 4  Remote network address (receive)
#define IOPAR_NETRMTPORTS 0x0508 // gs2  Remote port number (send)
#define IOPAR_NETRMTPORTR 0x0509 // g 2  Remote port number (receive)
#define IOPAR_NETDSTNAME  0x050A // gsS  Destination name
#define IOPAR_NETSMODE    0x050B // gs4  Set network mode bits
#define IOPAR_NETCMODE    0x050C // gs4  Clear network mode bits
#define IOPAR_NETRCVWIN   0x050D // gs4  Receive window size
#define IOPAR_NETLCLNETA  0x050E // g 4  Local network address
#define IOPAR_NETKATIME   0x050F // gs1  Keep-alive time interval (seconds)
#define IOPAR_NETCONLIMIT 0x0510 // gs4  Incoming connection queue limit
#define IOPAR_NETCONHNDL  0x0511 // gs4  Incoming connection handle

// Define IPM IO parameters

#define IOPAR_IPMBASE     0x0600
#define IOPAR_IPMRMTPID   0x0601 // g 4  IPM remote PID

// Define message IO parameters

#define IOPAR_MSGBASE     0x0700
#define IOPAR_MSGLCLADDR  0x0701 // g S  Message local address
#define IOPAR_MSGRMTADDRS 0x0702 // gsS  Message remote address (send)
#define IOPAR_MSGRMTADDRR 0x0703 // g S  Message remote address (receive)

// Define RUN function IO parameters

#define IOPAR_RUNBASE     0x1000
#define IOPAR_RUNCMDTAIL  0x1001 //  sS  Command tail (argument list)
#define IOPAR_RUNDEVLIST  0x1002 //  sS  Device list
#define IOPAR_RUNENVLIST  0x1003 //  sS  Additional environment data
#define IOPAR_RUNDEBUGBFR 0x1004 // g S  Buffer for debug data
#define IOPAR_RUNADDRESS  0x1005 // gs8  Load address
#define IOPAR_RUNRELOCVAL 0x1006 // gs4  Relocation value
#define IOPAR_RUNFCB1     0x1007 //  sS  First DOS FCB
#define IOPAR_RUNFCB2     0x1008 //  sS  Second DOS FCB
#define IOPAR_RUNACTPRIV  0x1009 //  sS  Active privileges for child
#define IOPAR_RUNAVLPRIV  0x100A //  sS  Available privileges for child

#define IOPAR_RUNWSLIMIT  0x100C //  s4  Working set size limit for child
#define IOPAR_RUNWSALLOW  0x100D //  s4  Working set size allowed for child
#define IOPAR_RUNTMLIMIT  0x100E //  s4  Total user memory limit for child
#define IOPAR_RUNTMALLOW  0x100F //  s4  Total user memory allowed for child
#define IOPAR_RUNPMLIMIT  0x1010 //  s4  Protected mode memory limit for child
#define IOPAR_RUNPMALLOW  0x1011 //  s4  Protected mode memory allowed for child
#define IOPAR_RUNRMLIMIT  0x1012 //  s4  Real mode memory limit for child
#define IOPAR_RUNRMALLOW  0x1013 //  s4  real mode memory allowed for child
#define IOPAR_RUNOMLIMIT  0x1014 //  s4  Overhead memory limit for child
#define IOPAR_RUNOMALLOW  0x1015 //  s4  Overhead memory allowed for child

#define IOPAR_CLASS       0x8000 // gs4  Device class

// Define bits in the IOPAR_CLRBUFR value

#define CB_OUTPUT 	0x40	// Clear output buffer
#define CB_INPUT  	0x20	// Clear current input buffer
#define CB_AHEAD  	0x10	// Clear type-ahead buffer

// Define bits in the IOPAR_FILOPTN value

#define FO_NOPREFIX 0x80000000	// Do not return prefix codes
#define FO_PHYNAME  0x00800000	// Return physical device name even if rooted
				//   logical
#define FO_DOSNAME  0x00400000	// Return DOS disk name
#define FO_VOLNAME  0x00200000	// Return disk volume name 
#define FO_XOSNAME  0x00100000	// Return XOS device name
#define FO_NODENUM  0x00080000	// Return node number
#define FO_NODENAME 0x00040000	// Return network node name
#define FO_NODEPORT 0x00020000	// Return node port number
#define FO_RPHYNAME 0x00008000	// Return remote physical device name even if
				//   rooted
#define FO_RDOSNAME 0x00004000	// Return remote DOS disk name
#define FO_RVOLNAME 0x00002000	// Return remote disk volume name
#define FO_RXOSNAME 0x00001000	// Return remote XOS device name
#define FO_PATHNAME 0x00000800	// Return file path
#define FO_PATHDOS  0x00000400	// Return file path using short names (DOS 8x3)
#define FO_FILENAME 0x00000200	// Return file name (includes extension)
#define FO_FILEDOS  0x00000100	// Return file short name (DOS 8x3)
#define FO_VERSION  0x00000080	// Return file version number
#define FO_ATTR     0x00000040	// Return file attribute byte

// Define values for the prefix bytes in the IOPAR_FILSPEC string

#define	FS_MIN      	0xE0	// Minimum special value
#define FS_DOSNAME  	0xE1	// DOS disk name is next
#define FS_VOLNAME  	0xE2	// Disk volume name is next
#define FS_XOSNAME  	0xE3	// XOS device name is next
#define FS_NODENUM  	0xE4	// Network remote node number is next
#define FS_NODENAME 	0xE5	// Network remote node name is next
#define FS_NODEPORT 	0xE6	// Network remote port number is next
#define FS_RDOSNAME 	0xE9	// Remote DOS disk name is next
#define FS_RVOLNAME 	0xEA	// Remote disk volume name is next
#define FS_RXOSNAME 	0xEB	// Remote XOS disk name is next
#define FS_PATHNAME    	0xF0	// Path is next
#define FS_PATHDOS      0xF1
#define FS_FILENAME 	0xF2	// File name is next (includes extension)
#define FS_FILEDOS      0xF3
#define FS_VERSION  	0xF4	// File version number is next
#define FS_ATTR     	0xF5	// File attribute byte is next
#define FS_NPATHNAME   	0xF8	// Path for new specification is next (rename)
#define FS_NPATHDOS     0xF9
#define FS_NFILENAME	0xFA	// New file name is next (includes extension)
				//   (rename)
#define FS_NFILEDOS     0xFB
#define FS_NVERSION 	0xFC	// New file version number is next (rename)
#define FS_TRUNC    	0xFE	// Entry has been truncated
#define FS_ESC      	0xFF	// Escape

// Define bits for the IOPAR_DEVSTS device parameter value (these bits are stored
//   in dcb_dsp) 

#define DS_CASESEN 0x00080000	// File names are case sensitive
#define DS_ALIAS   0x00040000	// File structure supports DOS 8x3 file name
								//   aliases
#define DS_NAMEEXT 0x00020000	// File structured device uses name.ext format
#define DS_LCLDISK 0x00010000	// Device is a local disk
#define DS_SPOOL   0x00008000	// Device is spooled
#define DS_CONTROL 0x00004000	// Device is controlling terminal for session
#define DS_NOABORT 0x00002000	// Device cannot be aborted
#define DS_UNBFRD  0x00001000	// Device should be unbuffered
#define DS_DUPLEX  0x00000800	// Device is full duplex (simultanious input
								//   and output)
#define DS_MAPPED  0x00000400	// Memory mapped device
#define DS_MLTUSER 0x00000200	// Multi-user device (any process can open
								//   device, even if device is in use)
#define DS_REMOTE  0x00000100	// Device is remote
#define DS_FILE    0x00000080	// Device is file structured
#define DS_SODIR   0x00000040	// Device supports search open directory
								//   operation
#define DS_PHYS    0x00000020	// Physical device
#define DS_REMOVE  0x00000010	// Removeable media device
#define DS_DOUT    0x00000008	// Device can do direct output
#define DS_DIN     0x00000004	// Device can do direct input
#define DS_QOUT    0x00000002	// Device can do queued output
#define DS_QIN     0x00000001	// Device can do queued input

// Define bits for the IOPAR_USTS device parameter value

#define US_CHNGD   0x8000	// Disk media has been changed
#define US_MOUNT   0x4000	// Disk is mounted
#define US_VALID   0x2000	// Disk contains valid data
#define US_NOTF    0x1000	// Disk is not file structured
#define US_MEDIA   0x0800	// Media type is specified
#define US_RECAL   0x0400	// Need to recalabrate
#define US_HFTRK   0x0200	// Have 48 tpi disk in 96 tpi drive
#define US_MOTON   0x0100	// Motor is on (floppy disk only)
#define US_TKDEN   0x00C0	// Track density
#define US_DBLS    0x0020	// Disk is double sided (floppy only)
#define US_M8H     0x0020	// Disk has more than 8 heads (hard disk only)
#define US_DEN     0x0018	// Data density
#define   DN_SD      0		//   Single density (also all hard
							//     disks)
#define	  DN_DD      1		//   Double density
#define	  DN_HD      2		//   High density
#define US_RSIZE   0x0007	// Record size
#define	  RS_UNKWN   0		//   Unknown
#define   RS_128     1		//   128 byte records
#define   RS_256     2		//   256 byte records
#define   RS_512     3		//   512 byte records
#define   RS_1024    4		//   1024 byte records

// Define functions for QFNC_DEVCHAR (stored in qab_option)

#define DCF_SIZE    1		// Return size of complete characteristics array
							//   without info pointers
#define DCF_SIZEIP  2		// Return size of complete characteristics array
							//   with info pointers
#define DCF_ALL     3		// Return name and type of all characteristics
							//   without info pointers
#define DCF_ALLIP   4		// Return name and type of all characteristics
							//   with info pointers
#define DCF_TYPE    5		// Obtain type of single characteristic without
							//   info pointer
#define DCF_TYPEIP  6		// Obtain type of single characteristic with
							//   info pointer
#define DCF_VALUES  7		// Get or set characteristics values

// Define functions for QFNC_CLASSFUNC (stored in qab_option)

#define CF_SIZE   	1		// Return size of complete characteristics array
							//   without info pointers
#define CF_SIZEIP 	2		// Return size of complete characteristics array
							//   with info pointers
#define CF_ALL    	3		// Return name and type of all characteristics
							//   without info pointers
#define CF_ALLIP  	4		// Return name and type of all characteristics
							//   with info pointers
#define CF_TYPE   	5		// Obtain type of single characteristic without
							//   info pointers
#define CF_TYPEIP   6		// Obtain type of single characteristic with
							//   info pointers
#define CF_VALUES 	7		// Get or set characteristics values
#define CF_ADDUNIT	8		// Add unit
#define CF_PUNITS 	9		// Get names of physical units
#define CF_AUNITS 	10		// Get names of active units
#define CF_DEVSRCH	11		// Search for device given name

// Define command bits for qab_option for svcIoRun

#define R_SAMEPROC  0x80000000	// Use same process
#define R_CHILDTERM 0x40000000	// Function is not complete until child process
								//   terminates
#define R_SESSION   0x04000000	// Create new session for child process
#define R_DEBUG     0x02000000	// Do debug load of program into same process
#define R_CPYENV    0x00800000	// Copy current enviroment to new process
#define R_ACSENV    0x00400000	// Allow new process to access this process's
								//   enviroment
#define R_CHGENV    0x00200000	// Allow new process to change this process's
								//   enviroment
#define R_ALLDEV    0x00080000	// Pass all devices to new process (device list
								//   is not used)
#define R_CPYPTH    0x00040000	// Copy default paths to new process
#define R_CHGPTH    0x00020000	// Allow new process to change this process's
								//   default paths
#define R_DOSEXEC   0x00000001	// DOS EXEC function (XOS internal use only)

// Define offsets in the argument data msect

#define arg_filofs 	0	// File specification offset
#define arg_fillen 	4	// File specification length
#define arg_cmdofs 	8	// Command data offset
#define arg_cmdlen 	12	// Command data length
#define arg_filspec	16	// File specification

// Define offset in the spawn function argument block (SAB)

#define sab_func   	0 	// (2) Function
#define sab_status 	2	// (2) Returned status
#define sab_error  	4	// (4) Returned error code
#define sab_pid    	8	// (4) Returned PID
#define sab_type   	12	// (1) Returned process type
			  			// (3) Reserved, must be 0
#define sab_vector 	16	// (2) Vector number
#define sab_numseg 	18	// (1) Number of segments to give to new process
			  			// (1) Reserved, must be 0
#define sab_option 	20	// (4) Option bits
#define sab_name   	24	// (8) Address of name for new process
#define sab_EIP    	32	// (4) Initial EIP value for new process
#define sab_CS     	36	// (4) Initial CS value for new process
#define sab_EFR    	40	// (4) Initial EFR value for new process
#define sab_ESP    	44	// (4) Initial ESP value for new process
#define sab_SS     	48	// (4) Initial SS value for new process
#define sab_EAX    	52	// (4) Initial EAX value for new process
#define sab_EDI    	56	// (4) Initial EDI value for new process
#define sab_parm   	60	// (8) Address of parameter list
#define sab_srcsel 	68	// (4) Selector for first segment to give to new process
#define sab_dstsel 	72	// (4) Selector for first destination segment
#define sab_SIZE   	76

// Define values for sab_func

#define SFNC_WAIT  	0x8000	// Wait until function is complete
#define SFNC_CHILD 	1    	// Create child process

// Define values for sab_option

#define S_SESSION	0x04000000	// Create new session for child process
#define S_ALLDEV 	0x00080000	// Pass all devices to new process (device list is
#define S_SETUP  	0x00000200	// Place child process in set up mode
#define S_NOCDS  	0x00000100	// Child process should not produce child died signal
#define S_EVENT  	0x000000FF	// Event number for termination (used if either S_SETUP
								//   or S_NOCDI is set)

// Define argument bits for svcIoCancel

#define CAN_WAIT  	0x0100	// Wait until finished
#define CAN_ALLDEV	0x0080	// Cancel all requests for all devices
#define CAN_ALL   	0x0040	// Cancel all requests for handle
#define CAN_AFTER 	0x0020	// Cancel specified request and all following requests
#define CAN_NOINT 	0x0010	// Suppress IO done interrupts
#define CAN_OPEN  	0x0004	// Cancel open request
#define CAN_OUTPUT	0x0002	// Cancel output queue requests
#define CAN_INPUT 	0x0001	// Cancel input queue requests

// Define function bits for svcIoTransName

#define TNB_NOFINALBS	0x20	// Suppress final \ in directory names
#define TNB_SEMICOLON	0x10	// Use semi-colon instead of comma to seperate names
#define TNB_FLAG     	0x08	// Insert flag character before device names
#define TNB_VOLNAME  	0x04	// Return volume name if available
#define TNB_DOSNAME  	0x02	// Return DOS disk name if available
#define TNB_ALLDEF   	0x01	// Return all definitions

// Define protection set values

#define FP_NETWORK	0x01000000	// (N) Network users
#define FP_WORLD  	0x00010000	// (W) Local users not part of the owner's group (world)
#define FP_GROUP  	0x00000100	// (G) Members of the owner's group
#define FP_OWNER  	0x00000001	// (O) Owner of the file

// Define file protection bit values

#define	FP_EXEC  	0x10		// (X) File is executable
#define	FP_READ  	0x08		// (R) File is readable
#define	FP_EXTEND	0x04		// (E) File can be extended
#define	FP_WRITE 	0x02		// (W) File is writable
#define	FP_MODIFY	0x01		// (M) File attributes can be modified

// Define directory protection bit values

#define FP_ACCESS	0x10		// (A) Files in directory can be accessed
#define FP_SEARCH	0x08		// (S) Directory can be searched
#define FP_CREATE	0x02		// (C) Files can be created in directory
#define FP_MODIFY	0x01		// (M) Directory attributes can be modified

// Define function values for the svcMemDescSet call

#define SDF_BASE  	1		// Set segment base linear address
#define SDF_LIMIT 	2		// Set segment limit
#define SDF_ACCESS	3		// Set access bits

// Define standard message types (this value is stored in the first byte of a
//   standard system message)

// Values 1 - 15 are reserved for general response messages

#define MT_INTRMDMSG     1	// Intermediate normal response
#define STR_MT_INTRMDMSG "\x01"
#define MT_INTRMDWRN     2	// Intermediate warning response
#define STR_MT_INTRMDWRN "\x02"
#define MT_INTRMDERR     3	// Intermediate error response
#define STR_MT_INTRMDERR "\x03"
#define MT_INTRMDSTS     4	// Intermediate status (temp) response
#define STR_MT_INTRMDSTS "\x04"
#define MT_FINALMSG      5	// Final normal response
#define STR_MT_FINALMSG  "\x05"
#define MT_FINALWRN      6	// Final warning response
#define STR_MT_FINALWRN  "\x06"
#define MT_FINALERR      7	// Final error response
#define STR_MT_FINALERR  "\x07"

// Values 16 - 31 are reserved for requests to INIT and LOGGER

#define MT_SYMBREQ  	16	// Request to run symbiont (INIT)
#define MT_TERMREQ  	17	// Request to change terminal state (INIT)
#define MT_SYSLOG   	18	// Data for system log (LOGGER)
#define STR_MT_SYSLOG "\x12"
#define MT_USERFILE 	19
#define MT_TERMDATA 	20	// Data from idle terminal (INIT)

// Values 32 - 47 are reserved for requests to general servers

#define MT_SRVCMD   	32
#define STR_MT_SRVCMD   "\x20"

// Values 48 - 63 are reserved for requests to the screen symbiont

#define MT_SSSTATUS 	48	// Screen status report
#define MT_EGACLEAR 	49	// EGA screen clear
#define MT_EGADUMP  	50	// EGA screen dump to printer
#define MT_SSREGPRG     51	// Register program to handle screen switching
#define MT_SSACTION     52	// Action request from program
#define MT_SSREPORT     53	// Report to program
#define MT_SSALMCHAR	63	// Alarm window character

// Values 64 - 79 are reserved for requests to the error logging symbiont

// Values 80 - 87 are reserved for requests to unspool symbionts

#define MT_UNSPLREADY	80	// File is ready to unspool
#define MT_UNSPLCMD  	81	// Command

// Values 88 - 95 are reserved for communications related requests

#define MT_NETLINK  88		// Network link has changed state

// Values 96 - 99 are reserved for accounting functions

#define MT_ACT		96		// System accounting message
#define MT_UDF		97      // UDF access requests (to or from USERSRV)
#define MT_RELOGREQ	98		// Relogin request
#define MT_CRDTCARD	99		// Credit card requests (to or from CRDTSRV)

// Values 100 - 127 are reserved for additional system functions

// Values 128-255 are available for user defined functions

// Define command bits for qab_command for open type functions (note: The high
//   order 13 bits describe open actions, they are not saved once the device is
//   open.  The low order 19 bits are saved and describe how the device is used
//   while open.)

// The following 13 bits are not stored in the handle table - they are only used
//   when the device is initially opened

#define O_REPEAT   0x80000000	// Repeated operation
#define O_REQFILE  0x40000000	// Require file structured device
#define O_NOMOUNT  0x20000000	// Do not mount device if not mounted (ignored
								//   if O_RAW or O$PHYS not also set)
#define O_ODF      0x10000000	// Open directory as file
#define O_FAILEX   0x08000000	// Fail if file exists (has no efect if file
								//   does not exist)
#define O_CREATE   0x04000000	// Create new file if file does not exist (has
								//   no effect if file exists)
#define O_TRUNCA   0x02000000	// Truncate existing file allocated length (has no effect
#define O_TRUNCW   0x01000000	// Truncate existing file written length (has
								//   no effect does not exist)
#define O_APPEND   0x00800000	// Append to file
#define O_FHANDLE  0x00400000	// Force handle (uses value of qab_handle to
								//   specify handle, fails if handle in use)
#define O_UNQNAME  0x00200000	// Create unique name for file
#define O_USEDOS   0x00100000	// Use DOS names when searching for directory
								//   or file
#define O_NOLONG   0x00080000	// Do not use long names when search for
								//   directory or file

// The following 2 bits are used internally by XOS and are stored in the
//   handle table

#define O_NOQUE    0x00040000	// IO queueing is disabled
#define O_OPNCLS   0x00020000	// Open or close is in progress (close if
								//   O_NOQUE is also set)

// The following bits are stored in the handle table and effect the operation
//   of a device for as long as it is open

#define O_NORDAH   0x00010000	// Do not read ahead (do single sector
								//   transfers)
#define O_NODFWR   0x00008000	// Do not defer writes
#define O_CONTIG   0x00004000	// File allocation must be contiguous
#define O_CRIT     0x00002000	// Do critical error processing
#define O_FAPPEND  0x00001000	// Force append
#define O_NOREOPEN 0x00000800	// Do not re-open spooled device after close
								//   time-out
#define O_PHYS     0x00000400	// Physical IO (exact meaning depends on device!)
#define O_RAW      0x00000200	// Raw IO (exact meaning depends on device!)
#define O_FNR      0x00000100	// Fail if device is not ready
#define O_PARTIAL  0x00000080	// Accept partial input
#define O_NOINH    0x00000040	// Device cannot be passed to child
#define O_XWRITE   0x00000020	// Exclusive write access
#define O_XREAD    0x00000010	// Exclusive read access
#define O_COMPAT   0x00000008	// DOS compatibility mode access

#define O_OUT      0x00000002	// Output is allowed
#define O_IN       0x00000001	// Input is allowed

// Define file attribute bits

#define A_NORMAL	0x80		// Normal file
#define A_ARCH  	0x20		// Archive bit (set if file has been modified)
#define A_DIRECT	0x10		// Directory
#define A_LABEL 	0x08		// Volume label
#define A_SYSTEM	0x04		// System file
#define A_HIDDEN	0x02		// Hidden file
#define A_RDONLY	0x01		// Read only file

// Define option bits for the close SVC

#define C_RESET   	0x80000000	// Reset file/device/connection
#define C_KILL    	0x40000000	// Kill file/device/connection
#define C_KEEPHNDL	0x10000000	// Keep handle
#define C_ONCEONLY	0x00008000	// Final once-only close
#define C_DELETE  	0x00000008	// Delete file being closed
#define C_NODEAL  	0x00000001	// Do not deallocate extra space in file

// Define bits for the svcSysFindEnv and svcSysDefEnv system calls

#define FES_SYSTEM 	0x0C000		// Search system level
#define FES_SESSION	0x08000		// Specify search level relative to session
#define FES_PROCESS	0x04000		// Specify search level relative to process

// Define bits for the svcIoFndLog and svcIoDefLog system calls

#define FLN_SYSTEM 	0x0C000		// Search system level
#define FLN_SESSION	0x08000		// Specify search level relative to session
#define FLN_PROCESS	0x04000		// Specify search level relative to process

#define TLN_SUBST  	0x40000000	// Substituted name
#define TLN_ROOTED 	0x20000000	// Rooted name

// Define bits for lck_bits

#define LCK_BUSY 	0x01		// Critical region is busy

// Define structure for PID

typedef struct piddata
{
    short number;
    short code;
} pid_data;

// Define structure for file spec and argument data for XOS images

typedef struct imgdata
{
    char *imgspec;
    long  imgspecsz;
    char *cmdtail;
    long  cmdtailsz;
} img_data;

// Define structure for parameter list entry for 0 byte value

typedef struct byte0parm
{
    uchar  desp;
    uchar  size;
    ushort index;
} byte0_parm;

// Define structure for parameter list entry for 1 byte value

typedef struct byte1parm
{
    uchar  desp;
    uchar  size;
    ushort index;
    char   value;
} byte1_parm;

// Define structure for parameter list entry for 2 byte value

typedef struct byte2parm
{
    uchar  desp;
    uchar  size;
    ushort index;
    short  value;
} byte2_parm;

// Define structure for parameter list entry for 4 byte value

typedef struct byte4parm
{
    uchar  desp;
    uchar  size;
    ushort index;
    long   value;
} byte4_parm;

// Define structure for parameter list entry for 8 byte value

typedef struct byte8parm
{
    uchar  desp;
    uchar  size;
    ushort index;
    long   value[2];
} byte8_parm;

// Define structure for parameter list entry for 8 byte time value

typedef struct time8parm
{
    uchar  desp;
    uchar  size;
    ushort index;
    time_s value;
} time8_parm;

// Define structure for parameter list entry for 2 byte text value

typedef struct text2parm
{
    uchar  desp;
    uchar  size;
    ushort index;
    char   value[2];
} text2_parm;

// Define structure for parameter list entry for 4 byte text value

typedef struct text4parm
{
    uchar  desp;
    uchar  size;
    ushort index;
    char   value[4];
} text4_parm;

// Define structure for parameter list entry for 8 byte text value

typedef struct text8parm
{
    uchar  desp;
    uchar  size;
    ushort index;
    char   value[8];
} text8_parm;

// Define structure for parameter list entry for 16 byte value

typedef struct byte16parm
{
    uchar  desp;
    uchar  size;
    ushort index;
    char   value[16];
} byte16_parm;

// Define structure for parameter list entry for long string value

typedef struct lngstrparm
{
    uchar  desp;
    uchar  size;
    ushort index;
    char   far *buffer;
    short  xxx;
    ushort bfrlen;
    ushort strlen;
} lngstr_parm;

// Define structure for characteristics list entry for 1 byte value

typedef struct byte1char
{
    uchar desp;
    uchar size;
    char  name[8];
    char  value;
} byte1_char;

typedef struct byte1charip
{
    uchar  desp;
    uchar  size;
    char   name[8];
    char   far *infopnt;
    short  xxx;
    ushort infosz;
    ushort infolen;
    char   value;
} byte1_charip;

// Define structure for characteristics list entry for 2 byte value

typedef struct byte2char
{
    uchar desp;
    uchar size;
    char  name[8];
    short value;
} byte2_char;

typedef struct byte2charip
{
    uchar  desp;
    uchar  size;
    char   name[8];
    char   far *infopnt;
    short  xxx;
    ushort infosz;
    ushort infolen;
    short  value;
} byte2_charip;

// Define structure for characteristics list entry for 4 byte value

typedef struct byte4char
{
    uchar desp;
    uchar size;
    char  name[8];
    long  value;
} byte4_char;

typedef struct byte4charip
{
    uchar  desp;
    uchar  size;
    char   name[8];
    char   far *infopnt;
    short  xxx;
    ushort infosz;
    ushort infolen;
    long   value;
} byte4_charip;

typedef struct text2char
{
    uchar desp;
    uchar size;
    char  name[8];
    char  value[2];
} text2_char;

typedef struct text4char
{
    uchar desp;
    uchar size;
    char  name[8];
    char  value[4];
} text4_char;

typedef struct text4charip
{
    uchar  desp;
    uchar  size;
    char   name[8];
    char   far *infopnt;
    short  xxx;
    ushort infosz;
    ushort infolen;
    char   value[4];
} text4_charip;

typedef struct text8char
{
    uchar desp;
    uchar size;
    char  name[8];
    char  value[8];
} text8_char;

typedef struct text8charip
{
    uchar  desp;
    uchar  size;
    char   name[8];
    char   far *infopnt;
    short  xxx;
    ushort infosz;
    ushort infolen;
    char   value[8];
} text8_charip;

typedef struct text16char
{
    uchar desp;
    uchar size;
    char  name[8];
    char  value[16];
} text16_char;

typedef struct text16charip
{
    uchar  desp;
    uchar  size;
    char   name[8];
    char   far *infopnt;
    short  xxx;
    ushort infosz;
    ushort infolen;
    char   value[16];
} text16_charip;

// Define structure for characteristics list entry for long string value

typedef struct lngstrchar
{
    uchar  desp;
    uchar  size;
    char   name[8];
    char   far *buffer;
    short  xxx;
    ushort bfrlen;
    ushort strlen;
} lngstr_char;

typedef struct lngstrcharip
{
    uchar  desp;
    uchar  size;
    char   name[8];
    char   far *infopnt;
    short  xxx1;
    ushort infosz;
    ushort infolen;
    char   far *buffer;
    short  xxx2;
    ushort bfrlen;
    ushort strlen;
} lngstr_charip;

# include <_epilog.h>

#endif // _XOS_H_
