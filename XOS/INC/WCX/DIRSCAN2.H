//-------------------------------------------------------------------------
// DIRSCAN2.H
// XOS - Prototypes and structures for the dirscan subroutine
//
// Written by: John R. Goltz
//
// Edit History:
// 09/07/92(brn) - Add comment Header
//
//-------------------------------------------------------------------------

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

#ifndef _DIRSCAN_H
#define _DIRSCAN_H

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
} dirscan_pl;

// Structure used internally by dirscan to store directory names during
//   ellipsis search

struct dirname
{   struct dirname *next;
    struct dirname *sortpnt;
    char   name[2];
};

// Following is the main dirscan data structure - items are flagged as follows:
 *	U - Must be initialized by user before calling dirscan
 *	D - Value is set by dirscan before calling user's function
 *	I - Used internally by dirscan


typedef struct dirscandata
{   dirscan_pl *parmlist;	// U - Address of parameter list
    char        repeat;		// U - TRUE if should do repeated search
    char        dfltwild;	// U - TRUE if want wildcard as default
    char        function;	// U - Value for low byte of QAB_FUNC for
				//	 searches  - should be QFNC_DEVPARM
				//	 or QFNC_DELETE
    char        sort;		// U - Directory sort order
    int       (*func)(struct dirscandata *dsd);
				// U - Function to call for each file matched
    void      (*errormsg)(const char *name, long code);
				// U - Function called on fatal error
    int         pathmasklen;	// U - Length of pathmask string
    char       *pathmask;	// U - Address of pathmask string buffer
    char        changed;	// D - Changed flags
    char        xxx;
    short       attr;		// D - Attribute value

    char       *srcstr		// U - Address of source string
    char       *dststr		// U - Address of destination string (may be
				//       NULL
    char       *srcspec;	// U - Address of buffer to receive source spec 
    long        srcspeclen;	// D - Length of source spec string
    char       *dstspec;	// U - Address of buffer to receive destination
				//       spec (may be NULL)
    long        dstspeclen;	// D - Length of destination spec string
    long        error;		// D - Error code
				// Everything after this is private to
				//   dirscan
    char       *namepnt;	// I - Pointer to file name string
    char       *elippos;	// I - Position of ellipsis in name
    struct dirname *firstdir;
    struct dirname *thisdir;
    struct
    {   byte4_parm  filoptn;
        lngstr_parm filspec;
        byte2_parm srcattr;
        char       end;
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
    type_qab dirqab;
    type_qab fileqab;
} dirscan_data;

// Define values for directory sort order

#define DSSORT_NONE  0		// No sort
#define DSSORT_ASCEN 1		// Ascending sort
#define DSSORT_DECEN 2		// Decending sort

// Define values for changed

#define DSCHNG_DEVNAME  0x80
#define DSCHNG_NODENAME 0x40
#define DSCHNG_RDEVNAME 0x20
#define DSCHNG_PATHNAME 0x10

int dirscan(char *filename, dirscan_data *dsd);

#endif	// _DIRSCAN_H_
