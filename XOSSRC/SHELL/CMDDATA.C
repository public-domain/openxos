//------------------------------------------------------------------------------
//
//  CMDDATA.C - Data for SHELL/BATCH
//
//  Written by Bruce R. Nevins
//
//  Edit History:
//  -------------
//  08/18/92(brn) - Created first version
//  05/03/94(brn) - Add definition for Pause batch on error
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

#include <STDIO.H>
#include <SETJMP.H>
#include <XOS.H>
#include "SHELL.H"
#include "BATCHEXT.H"

//
// Allocate storage
//

char    prgname[] = "SHELL";	// Name our program
int     major_version = 3;
int     minor_version = 25;
char	*date_version = __DATE__;
struct firstblk *argbase = NULL;// Pointer to start of argument string
struct firstblk *hstfrst = NULL;// Pointer to command history list beginning
struct firstblk *hstlast = NULL;// Pointer to command history list end
struct firstblk *hsttmp = NULL; // Temp pointer into history list
struct firstblk *aliasfrst = NULL;
                                // Pointer to command alias list beginning
struct firstblk *aliaslast = NULL;
                                // Pointer to command alias list end
struct firstblk *aliastmp = NULL;
				// Temp pointer into alias list
int	detached = FALSE;	// TRUE if running under the batch server
int    aliascnt = 0;            // Number of alias' defined
unsigned long histlimit = HISTDEFAULT;// Default size of history list
int     histcnt = 0;
int	histlow = 0;
int     topseen = FALSE;        // Flag for top of command list
int     botseen = FALSE;        // Flag for bottom of command list
int     pipeseen = FALSE;       // Flag if doing a pipe
int     extend_batch = FALSE;	// Default is no extended batch
int	ctrlc_prompt = TRUE;	// Display prompt in batch when Control C
int	local_batch = FALSE;	// Don't exit shell at end of batch file
int     bat_return = FALSE;	// Return from batch file
int	error_pause = FALSE;	// Pause batch on error return not 0
char   *firstarg;               // Pointer to first argument
char    firstbuf[LINESIZE];	// Buffer for first arg
int     blkcnt;                 // Character count for putcmdc
char   *blkpnt;                 // Pointer for putcmdc
struct  cprocs cproc[MAXCHILD]; // Data about children
unsigned int childcnt;          // Child count

jmp_buf rstbuf;                 // Place to save state for restarts
jmp_buf batrstbuf;              // Place to save batch state for restarts
long    linecnt;                // Number of bytes in cline
long    errrtn;                 // Error return from child
char   *linepnt;                // Pointer to current byte in cline
char    dftdev[] = "CMD:";      // Default device for commands
char    inpredir[FILESPCSIZE];  // File specification for input redirection
char    outredir[FILESPCSIZE];  // File specification for output redirection
char    cline[LINESIZE+2];      // Command line buffer
struct firstblk *expndarg;      // Expanded input line
struct firstblk *interncmd = NULL; // Internal command buffer
char    simple;                 // TRUE if file specification is simple name
char    loggedin = TRUE;        // TRUE if logged in
char    echo = TRUE;            // TRUE if BATCH commands are to be echoed
char    keyword[LINESIZE];
char    runname[FILESPCSIZE];   // Name of program to run
char    filname[FILESPCSIZE] = "STDIN:";
                                // Name of file where input is to come from
char    bat_ctlc = FALSE;	// TRUE if stop batch since a ^C was seen
char    batch = FALSE;          // FALSE  if not running batch
char    skipprompt = FALSE;     // Skip printing the next prompt
char    inlinecmd  = FALSE;     // Process inline command next not file
char    singlearg;              // TRUE if one arg
FILE    *cmdin;			// FD for command input
FILE    *cmdout = stdout;	// FD for command output
FILE    *cmderr = stderr;	// FD for command error

//
// Batch defines
//
char    tempnoecho = FALSE;     // TRUE if only this command is not to be echoed
char	tempnohist = FALSE;	// TRUE if this command is not to go in history

struct devlist devlist =	// Device list for run
{   {STDIN , STDIN , O_IN},
    {STDOUT, STDOUT, O_OUT|O_IN},
    {STDERR, STDERR, O_OUT|O_IN},
    {STDTRM, STDTRM, O_OUT|O_IN},
    {0}
};

struct devlist devlist_det =	// Device list for run when detached
{   {STDOUT, STDOUT, O_OUT|O_IN},
    {STDERR, STDERR, O_OUT|O_IN},
    {STDTRM, STDTRM, O_OUT|O_IN},
    {0}
};

static char nulltail[] = " ";

struct runparm runparm =
{   {(PAR_SET|REP_STR), 0, IOPAR_RUNCMDTAIL, nulltail, 0, 1, 0},
    {(PAR_SET|REP_STR), 0, IOPAR_RUNDEVLIST, (char *)&devlist, 0,
            sizeof(devlist), 0}
};

type_qab runqab =
{   RFNC_WAIT|RFNC_RUN,		// qab_func     - Function
    0,				// qab_status   - Returned status
    0,				// qab_error    - Error code
    0,		                // qab_amount   - Process ID
    0,				// qab_handle   - Device handle
    0,				// qab_vector   - Vector for interrupt
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    R_ACSENV|R_CHGENV,		// qab_option   - Options or command
    0,				// qab_count    - Count
    runname, 0,			// qab_buffer1  - Pointer to file spec
    0, 0,			// qab_buffer2  - Unused
    &runparm, 0			// qab_parm     - Pointer to parameter area
};

//
// Default ALIAS commands
//
char *alias_defaults[] = 
{
    "ALIAS DEL=DELETE %*\0",
    "ALIAS ERASE=DELETE %*\0",
    "ALIAS MD=MKDIR %*\0",
    "ALIAS RD=RMDIR %*\0",
    "ALIAS REN=RENAME %*\0",
    NULL
};
