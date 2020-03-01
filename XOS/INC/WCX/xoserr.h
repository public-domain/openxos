//------------------------------------------------------------------------------
//
//  XOSERR.H - XOS error code definitions
//
//  Edit History:
//  -------------
//  09/07/92(brn) - Add comment header
//  09/08/92(brn) - Add comments for each error code
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

#ifndef _XOSERR_H_
#define _XOSERR_H_

#ifndef _BASELINE_H_

#include <baseline.h>

#endif // _BASELINE_H_


// Define SVC error codes

#define ER_NOERR   0	// Normal return
#define ER_EOF    -1	// End of file
#define ER_SVC    -2	// Illegal SVC function
#define ER_FUNC   -3	// Illegal function
#define ER_FUNCM  -4	// Illegal function for current mode
#define ER_VALUE  -5	// Illegal value
#define ER_PARMI  -6	// Illegal parameter index
#define ER_PARMV  -7	// Illegal parameter value
#define ER_PARMS  -8	// Illegal parameter value size
#define ER_PARMT  -9	// Illegal parameter type
#define ER_PARMF  -10	// Illegal parameter function
#define ER_PARMM  -11	// Required parameter missing
#define ER_CHARN  -12	// Illegal characteristic name
#define ER_CHARV  -13	// Illegal characteristic value
#define ER_CHARS  -14	// Illegal characteristic value size
#define ER_CHART  -15	// Illegal characteristic type
#define ER_CHARF  -16	// Illegal characteristic function
#define ER_CHARM  -17	// Required characteristic missing
#define ER_BDNAM  -18	// Bad process name
#define ER_BDPID  -19	// Bad process ID
#define ER_NSP    -20	// No such process
#define ER_PRIV   -21	// Not enough privilege
#define ER_NSEGA  -22	// No segment available
#define ER_NEMA   -23	// Not enough memory available
#define ER_MACFT  -24	// Memory allocation conflict
#define ER_MAERR  -25	// Memory allocation error
#define ER_NODCB  -26	// No disk cache buffer available
#define ER_NOBUF  -27	// No system buffer available
#define ER_ACT    -28	// Device is active
#define ER_BDSPC  -29	// Bad device or file specification
#define ER_NSDEV  -30	// No such device
#define ER_DEVIU  -31	// Device in use
#define ER_DEVIO  -32	// Device is open
#define ER_DEVNO  -33	// Device is not open
#define ER_LASNA  -34	// Linear address space not available
#define ER_DEVFL  -35	// Device is full
#define ER_TMDVP  -36	// Too many devices open for process
#define ER_DFDEV  -37	// Different device for rename
#define ER_FILNF  -38	// File not found
#define ER_FILEX  -39	// File exists
#define ER_BUSY   -40	// File or device is busy
#define ER_FILAD  -41	// File access denied
#define ER_DIRNF  -42	// Directory not found
#define ER_DIRFL  -43	// Directory full
#define ER_DIRNE  -44	// Directory not empty
#define ER_DIRTD  -45	// Directory level too deep
#define ER_DATER  -46	// Data error
#define ER_IDFER  -47	// ID field error
#define ER_SEKER  -48	// Seek error
#define ER_RNFER  -49	// Record not found error
#define ER_LSTER  -50	// Lost data error
#define ER_WRTER  -51	// Write fault error
#define ER_WPRER  -52	// Write protect error
#define ER_DEVER  -53	// Device error
#define ER_DATTR  -54	// Data truncated
#define ER_NORSP  -55	// Device did not respond
#define ER_BDDBK  -56	// Bad disk block number
#define ER_BDDVH  -57	// Bad device handle
#define ER_NOOUT  -58	// Output not allowed
#define ER_NOIN   -59	// Input not allowed
#define ER_ADRER  -60	// Address error
#define ER_IIFT   -61	// Illegal image file type
#define ER_IIFF   -62	// Illegal image file format
#define ER_IIFRD  -63	// Illegal relocation data in image file
#define ER_RELTR  -64	// Relocation truncation in image file
#define ER_NOSAD  -65	// No starting address specified in image file
#define ER_NOSTK  -66	// No stack specified in image file
#define ER_IFDEV  -67	// Illegal function for device
#define ER_ICDEV  -68	// Illegal count for device
#define ER_IADEV  -69	// Illegal buffer address for device
#define ER_MDCHG  -70	// Media changed
#define ER_RTOBG  -71	// Record too big
#define ER_NACT   -72	// Device not active
#define ER_FMTER  -73	// Format error
#define ER_NTRDY  -74	// Device not ready
#define ER_NTDIR  -75	// File is not a directory
#define ER_ISDIR  -76	// File is a directory
#define ER_NTTRM  -77	// Device is not a terminal
#define ER_ILSEK  -78	// Illegal seek function
#define ER_BPIPE  -79	// Pipe error
#define ER_DLOCK  -80	// Deadlock error
#define ER_FBFER  -81	// FIB format error
#define ER_FBPER  -82	// FIB pointer error
#define ER_FBRER  -83	// FIB read error
#define ER_FBWER  -84	// FIB write error
#define ER_HMFER  -85	// HOM format error
#define ER_HMRER  -86	// HOM read error
#define ER_SBFER  -87	// SAT format error
#define ER_SBRER  -88	// SAT read error
#define ER_SBWER  -89	// SAT write error
#define ER_DRFER  -90	// Directory format error
#define ER_DRRER  -91	// Directory read error
#define ER_DRWER  -92	// Directory write error
#define ER_NTFIL  -93	// Not a file structured device
#define ER_IATTR  -94	// Illegal file attribute change
#define ER_NTDSK  -95	// Device is not a disk
#define ER_DQUOT  -96	// Disk quota exceeded
#define ER_FSINC  -97	// File system is inconsistant
#define ER_NTDEF  -98	// Not defined
#define ER_BDLNM  -99	// Bad logical name
#define ER_WLDNA  -100	// Wild card name not allowed
#define ER_NTLNG  -101	// Name is too long
#define ER_TMUSR  -102	// Too many users
#define ER_TMPSS  -103	// Too many processes or shared segments in system
#define ER_PDTYP  -104	// Physical device type incorrect
#define ER_PDNAV  -105	// Physical device not available
#define ER_PDADF  -106	// Physical device already defined
#define ER_DUADF  -107	// Device unit already defined
#define ER_NSCLS  -108	// No such device class
#define ER_CLSAD  -109	// Device class already defined
#define ER_XFRBK  -110	// Data transfer blocked
#define ER_TMDVC  -111	// Too many devices open for device class
#define ER_NPERR  -112	// Network protocol error
#define ER_NPRNO  -113	// Network port not open
#define ER_NPRIU  -114	// Network port in use
#define ER_NILPR  -115	// Illegal port number
#define ER_NILAD  -116	// Illegal network address
#define ER_NILRF  -117	// Illegal request format
#define ER_NILPC  -118	// Illegal network protcol type
#define ER_NPCIU  -119	// Network protocol type in use
#define ER_NCONG  -120	// Network congestion
#define ER_NRTER  -121	// Network routing error
#define ER_NSNOD  -122	// No such network node
#define ER_NTTIM  -123	// Network time out
#define ER_NCLST  -124	// Network connection lost
#define ER_NHSNA  -125	// Network host not available
#define ER_NCCLR  -126	// Network connection cleared
#define ER_NCRFS  -127	// Network connection refused
#define ER_NNNDF  -128	// Network name is not defined
#define ER_NNSNC  -129	// Network name server not capable
#define ER_NNSRF  -130	// Network name server refused request
#define ER_NNSNA  -131	// Network name server not available
#define ER_NNSRQ  -132	// Network name server bad format request
#define ER_NNSRS  -133	// Network name server bad format response
#define ER_NNSER  -134	// Network name server error
#define ER_NNMTD  -135	// Network name mapping is too deep
#define ER_NRTNA  -136	// Network router not available
#define ER_NNCON  -137	// No connection established
#define ER_NDRTL  -138	// Network data rejected - too long
#define ER_NPSQE  -139	// Network protocol sequencing error
#define ER_NOMEM  -140	// No memory allocated
#define ER_ALDEF  -141	// Already allocated
#define ER_NCOMP  -142	// Not compatible
#define ER_NOPAP  -143	// Printer is out of paper
#define ER_IMEMA  -144	// Illegal memory address
#define ER_NSTYP  -145	// No such device type
#define ER_CHNNA  -146	// DMA channel not available
#define ER_BDLA   -147	// Bad linear address
#define ER_TMRNC  -148	// Too many requests for network connection
#define ER_DKRMV  -149	// Disk removed
#define ER_ABORT  -150	// IO operation aborted
#define ER_CANCL  -151	// IO operation canceled
#define ER_SELNA  -152	// Segment selector not allocated
#define ER_BDSEL  -153	// Bad segment selector
#define ER_DOSMC  -154	// DOS memory allocation data corrupted
#define ER_NDOSD  -155	// No DOS IO data block available
#define ER_IDEVC  -156	// Incorrect device class
#define ER_DTINT  -157	// Data transfer interrupted
#define ER_IOSAT  -158	// IO saturation
#define ER_IDREN  -159	// Invalid directory rename
#define ER_LKEAL  -160	// LKE already loaded
#define ER_CDAAD  -161	// LKE comman data area already defined
#define ER_CDAND  -162	// LKE common data area not defined
#define ER_ININU  -163	// Interrupt number in use
#define ER_DIFER  -164	// Device interface error
#define ER_DVDER  -165	// Device driver error
#define ER_FINTR  -166	// Function interrupted
#define ER_NTIMP  -167	// Not implemented
#define ER_ERROR  -168	// Unspecified general error
#define ER_IOINU  -169	// IO register block in use
#define ER_DOURN  -170	// Data overrun or underrun
#define ER_00171  -171
#define ER_00172  -172
#define ER_TMIOM  -173	// Too many IO requests for memory page
#define ER_TMIOP  -174	// Too many IO request pointers
#define ER_MPILK  -175	// Memory page is locked
#define ER_TMIOQ  -176	// Too many IO requests queued
#define ER_TMUDV  -177	// Too many users for device
#define ER_TMDDV  -178	// Too many device units for device
#define ER_NTLCL  -179	// Not local
#define ER_DOSPB  -180	// Permanent DOS process is busy
#define ER_ICMIO  -181	// Incomplete IO operation
#define ER_NSLP   -182	// Not a session level process
#define ER_LOCK   -183	// File record lock violation
#define ER_CAASP  -184	// Close action already specified
#define ER_CAERR  -185	// Close action error
#define ER_FTPER  -186	// FAT block pointer error
#define ER_FTRER  -187	// Error reading FAT block
#define ER_FTWER  -188	// Error writing FAT block
#define ER_TMRQB  -189	// Too many requests for buffer
#define ER_CCMSS  -190	// Cannot change memory section size
#define ER_NNOPC  -191	// No network protocol specified
#define ER_IPDIR  -192	// Illegal pointer in directory
#define ER_MSNPR  -193	// Msect is not private
#define ER_INVST  -194	// Invalid segment type
#define ER_NLKNA  -195	// Network link not available
#define ER_EVRES  -196	// Event is reserved
#define ER_EVNRS  -197	// Event is not reserved
#define ER_EVSET  -198	// Event is set
#define ER_CPDNR  -199	// Child process did not respond
#define ER_STKER  -200	// Stack error
#define ER_DIVER  -201	// Divide error
#define ER_ILLIN  -202	// Illegal instruction
#define ER_UNXSI  -203	// Unexpected signal
#define ER_NWPA   -204	// No watchpoint available
#define ER_BDALM  -205	// Bad alarm handle
#define ER_TMALM  -206	// Too many alarms for process
#define ER_DPMIC  -207	// DPMI environment corrupted
#define ER_MEMLX  -208	// Memory limit exceeded
#define ER_VECNS  -209	// Signal vector not set up
#define ER_TRMNA  -210	// Terminal is not attached
#define ER_STIIU  -211	// SCSI target ID is in use
#define ER_SLKCE  -212	// SCSI linked command error
#define ER_STDNR  -213	// SCSI target did not respond
#define ER_SDLNE  -214	// SCSI data length error
#define ER_SUXBF  -215	// SCSI unexpected bus free state
#define ER_STBPS  -216	// SCSI target bus phase sequence failure
#define ER_STARI  -217	// SCSI target number is illegal
#define ER_SLUNI  -218	// SCSI logical unit number is illegal
#define ER_SSUNI  -219	// SCSI sub-unit number is illegal
#define ER_SDVTI  -220	// SCSI device type is incompatible
#define ER_BLANK  -221	// Media is blank
#define ER_NBLNK  -222	// Media is not blank
#define ER_EOS    -223	// End of set
#define ER_EOM    -224	// End of media
#define ER_IIFSU  -225	// Illegal image file symbol - undefined
#define ER_IIFSL  -226	// Illegal image file symbol - name too long
#define ER_IFXST  -227	// Illegal format in exported symbol table
#define ER_OUTNE  -228	// Output is not enabled
#define ER_NOACK  -229	// Output was not acknowleged
#define ER_TMORQ  -230	// Too many output requests
#define ER_NMBTS  -231	// Name buffer is too small
#define ER_IINUM  -232	// Illegal interrupt number
#define ER_IDSPC  -233	// Illegal destination file specification
#define ER_TYPAD  -234	// Device type already defined
#define ER_NEDMA  -235	// Not enough device memory available
#define ER_PWUSR  -236	// Incorrect user name or password
#define ER_NNPA   -237	// No network port available}
#define ER_SCTNA  -238	// Section not available
#define ER_TMDVS  -239	// Too many devices open for system
#define ER_IONTC  -240	// IO operation not complete
#define ER_MDICN  -241	// Modem is connected
#define ER_MDNCN  -242	// Modem not connected
#define ER_NSSRV  -243	// No such server
#define ER_ISREQ  -244	// Illegal server request
#define ER_ISRSP  -245	// Illegal server response
#define ER_SIOER  -246	// Server IO error
#define ER_IDFMT  -247	// Illegal data format
#define ER_NAPER  -248	// Network application protocol error
#define ER_MSGNF  -249	// Message not found
#define ER_MSGFE  -250	// Message format error
#define ER_NPNIU  -251	// Network port not in use
#define ER_NNDFP  -252	// No destination for network protocol

#define ER_NIYT   -256	// Not implemented yet!
#define ER_MATH   -257	// Math function error
#define ER_RANGE  -258	// Math function argument out of range
#define ER_TMTHD  -259	// Too many threads
#define ER_THDNS  -260	// Thread is not suspended
#define ER_NODAT  -261	// No data in file
#define ER_BDFMT  -262	// Bad file format
#define ER_BDPFX  -263	// Bad prefixed value
#define ER_REC2L  -264	// Record is too long
#define ER_REC2S  -265	// Record is too short
#define ER_BLK2S  -266	// Block is too short
#define ER_BDFNT  -267	// Bad font file
#define ER_BDWNH  -268	// Bad window handle
#define ER_NRMTE  -269	// Error reported by remote system
#define ER_NOCON  -270	// No console device available
#define ER_TMDVA  -271	// Too many device addresses in use
#define ER_TMHUB  -272	// Too many USB hubs

#endif    // _XOSERR_H_
