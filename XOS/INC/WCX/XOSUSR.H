//------------------------------------------------------------------------------
//
//  XOSUSR.H - User-mode definitions for XOS
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

#ifndef _XOSUSR_H_
#define _XOSUSR_H_

# include <_prolog.h>

// Define time for XOS

#ifndef _TIME_S
#define _TIME_S

typedef struct sysdttm
{
    unsigned long time;
    long     date;
} time_s;

#endif // _TIME_S

//
// Userlist structure for login
//

#define USER_VERSION  1		    // Version number for user file

struct user_header		        // Size of struct must be 512 bytes
{
    unsigned short  version;	// Version number for this file
    unsigned short  password;	// Min. length for password
    unsigned short  maxfail;	// Login fail count before lockout
    unsigned long   timedelay;	// Lockout time
    unsigned long   failcount;	// System-wide login failure count
    unsigned long   logincount;	// System-wide login count

    char            spare[494];	// Unused bytes
};

struct user			// Size of struct must be 512 bytes
{
             char   username[16]; // Username
             char   groupname[16];// User's group name
    unsigned char   password[32]; // Encrypted user password
             char   homedir[64];  // Home directory for user
             char   shell[64];	  // Default shell for user
             short  login_failure;// # of failed logins
	     time_s lastfail;	  // Time of last failure
             short  total_login;  // Total # of logins
             time_s laston;	  // Date/Time of last login
    unsigned long   initpriv[2];  // Initial privlege bits
    unsigned long   allowpriv[2]; // Privleges that are allowed*/
             short  terminal;	  // Terminal type
    unsigned long   expire_date;  // Date this account expires
             short  bbs_level;	  // BBS user level
    unsigned long   bbs_bits;	  // BBS access privlege bits
             char   spare[272];	  // Unused bytes
};

struct ufr			                // Struct for userfile replies from init
{
    char replytype;		            // Reply type!
    unsigned long record;	        // Record number in userfile
    struct user userentry;	        // Complete user entry to return
};

# include <_epilog.h>

#endif	// _XOSUSR_H_
