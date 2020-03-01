//------------------------------------------------------------------------------
//
//  DIRSCAN.H - Prototypes and structures for the dirscan subroutine
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

#ifndef _DIRSCAN_H_
#define _DIRSCAN_H_

#ifndef _BASELINE_H_

#include <baseline.h>

#endif // _BASELINE_H_


// Following is the minimum parameter list which must be specified by the
//   caller.  The first 5 items in the list MUST match this structure but any
//   additional items needed may be added after filattr.  The filspec buffer
//   must be allocated by the user and the address and length stored in the
//   filspec parameter.  The filoptn value must be filled in to specify the
//   items desired by the user.  The srcattr value must be filled to specify
//   the attribute mask used when searching for files.  The filattr and dirhndl
//   parameters are used by dirscan and the user does not need to specify
//   initial values

typedef struct dirscanpl
{   byte4_parm  filoptn;	// PAR_SET|REP_HEXV
    lngstr_parm filspec;	// PAR_GET|REP_STR
    byte2_parm  srcattr;	// PAR_SET|REP_HEXV
    byte2_parm  filattr;	// PAR_GET|REP_HEXV
    byte4_parm  dirhndl;	// PAR_SET|REP_DECV
    char        end;
} DIRSCANPL;

#define FILSPECSIZE  4096

#define NODENAMESIZE  128	// Size of base node name buffer
#define PATHNAMESIZE  512	// Size of base path name buffer
#define PATHDOSSIZE   128	// Size of base path DOS buffer
#define FILENAMESIZE  256	// Size of base file name buffer
#define FILEDOSSIZE    32	// Size of base file DOS buffer

// Structure used internally by dirscan to store directory names during
//   ellipsis search

typedef struct dirname DIRNAME;
struct  dirname
{   DIRNAME *next;
    DIRNAME *sortpnt;
    char     name[2];
};

// The netbufr structure contains pointers, buffer length, and returned
//   string length values for the rdevname and nodename buffers.  One copy of
//   this structure is statically allocated within the dirscandata structure.
//   Additional copies are allocated seperate from the dirscandata structure
//   and linked into a list beginning with the statically allocated structure.
//   These additional structure are allocated by dirscan as needed.  The caller
//   must not directly modify any values in this structure.  The *pnt and *len
//   entries are to be used by the caller to obtain the strings returned by
//   dirscan for each match.  The *size values are for internal use by dirscan
//   only.

// The buffers referenced by the netbufr structure are allocated by dirsacn
//   as required.

typedef struct rmtdata RMTDATA;
struct  rmtdata
{   RMTDATA *next;
    char    *nodename;
    int      nodenamelen;
    char    *nodenamebase;
    int      nodenamesize;
    char     rmtdevname[32];
    int      rmtdevnamelen;
};

// When displaying the file specification returned by dirscan, the items should
//   be displayed in the following order:
//	Device name
//	{   Node name
//	    Remote device name
//	}			// For each NETBUFR structure present
//	Path name (or path DOS)
//	File name (or file DOS)
//	File version number

// Any necessary prefix or postfix delimiter characters are added by dirscan
//   and all XOS prefix codes are removed by dirscan.

// Following is the main dirscan data structure - Items are flagged as follows:
//	U - Must be initialized by user before calling dirscan
//	D - Value is set by dirscan before calling user's function
//	I - Used internally by dirscan

typedef struct dirscandata
{   DIRSCANPL *parmlist;		// U - Parameter list
    char       repeat;			// U - TRUE if should do repeated search
    char       dfltwildext;		// U - TRUE if want default wildcard extension
    char       function;		// U - Value for low byte of QAB_FUNC for
								//	 searches  - should be QFNC_DEVPARM
								//	 or QFNC_DELETE
    char       sort;			// U - Directory sort order
    long     (*hvellip)(struct dirscandata *dsd);
								// U - Function to call before doing ellipsis
    long     (*func)(struct dirscandata *dsd);
								// U - Function to call for each file matched
    void     (*errormsg)(const char *name, long code);
								// U - Function called on fatal error
    char       changed;			// D - Changed flags
    char       xxx;
    short      attr;			// D - Attribute value

    long       level;			// D - Directory level
    char       devname[32];		// D - Device name string
    long       devnamelen;		// D - Length of device name string

    RMTDATA    rmtdata;			// D - Network information data

    char      *pathname;		// D - Path name string (long names)
    long       pathnamelen;		// D - Length of path name string
    char      *pathnamebase;	// I - Address of base path name buffer
    long       pathnamesize;	// I - Size of path name buffer

    char      *pathdos;			// D - Path DOS string (DOS 8x3 names)
    long       pathdoslen;		// D - Length of path DOS string
    char      *pathdosbase;		// I - Address of base path DOS buffer
    long       pathdossize;		// I - Size of path DOS buffer

    char      *filename;		// D - Filename string
    long       filenamelen;		// D - Length of filename string
    char      *filenamebase;	// I - Address of base file name buffer
    long       filenamesize;	// I - Size of filename buffer

    char      *filedos;			// D - File DOS string (DOS 8x3 name)
    long       filedoslen;		// D - Length of file DOS string
    char      *filedosbase;		// I - Address of base file DOS buffer
    long       filedossize;		// I - Size of file DOS buffer

    char       version[32];		// D - Version number
    long       versionlen;		// D - Length of version number string

    long       error;			// D - Error code
								// Everything after this is private to
								//   dirscan
    char      *newfilespec;
    char      *namepnt;			// I - Pointer to file name string
    char      *elippos;			// I - Position of ellipsis in name

    DIRNAME   *firstdir;
    DIRNAME   *thisdir;
    struct
    {   byte4_parm  filoptn;
        lngstr_parm filspec;
        byte2_parm  srcattr;
	byte4_parm  devsts;
        char        end;
    } diropnparm; 
    struct
    {   byte4_parm iopos;
        byte4_parm devsts;
        char       end;
    } dirposparm;
    struct
    {   byte4_parm  filoptn;
        lngstr_parm filspec;
        byte2_parm  srcattr;
        byte4_parm  dirhndl;
        char        end;
    } dirsrchparm;
    QAB dirqab;
    QAB fileqab;
} DIRSCANDATA;

// Define values for directory sort order

#define DSSORT_NONE  0		// No sort
#define DSSORT_ASCEN 1		// Ascending sort
#define DSSORT_DECEN 2		// Decending sort

// Define values for changed

#define DSCHNG_DEVNAME  0x80
#define DSCHNG_NODENAME 0x40
#define DSCHNG_RDEVNAME 0x20
#define DSCHNG_PATHNAME 0x10

int dirscan(char *filespec, DIRSCANDATA *dsd);

#endif	// _DIRSCAN_H_
