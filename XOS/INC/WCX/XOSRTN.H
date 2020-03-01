//------------------------------------------------------------------------------
//
//  XOSRTN.H - Return codes to Batch and Shell for XOS
//
//  Edit History:
//  -------------
//  10/28/89(brn) - Created first version
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

#ifndef _XOSRTN_H_
#define _XOSRTN_H_

#ifndef _BASELINE_H_

#include <baseline.h>

#endif // _BASELINE_H_


//
// Define standard exit codes for XOS utilities
//

#define EXIT_NORM       0       // No error
#define EXIT_FNF        1       // File Not Found
#define EXIT_MALLOC     2       // Malloc failed
#define EXIT_INVSWT     3       // Invalid option
#define EXIT_PATH       4       // Invalid Path
#define EXIT_UNK        5       // Unknown DOS error
#define EXIT_INVDRV     6       // Invalid Drive
#define EXIT_INTERR     7       // Internal error
#define EXIT_BADCOMMAND 8       // Error exit status
#define EXIT_IOERR      9       // XOS I/O error occured
#define EXIT_SVCERR	10 	// XOS svc call error

#endif	// _XOSRTN_H_
