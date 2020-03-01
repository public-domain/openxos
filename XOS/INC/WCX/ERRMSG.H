//------------------------------------------------------------------------------
//
//  ERRMSG.H - Definitions for error message function
//
//  Edit History:
//  -------------
//  18May95 (fpj) - Added comment block.
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

#ifndef _ERRMSG_H_
#define _ERRMSG_H_

#ifndef _BASELINE_H_

#include <baseline.h>

#endif // _BASELINE_H_


/* Definitions for ERRMSG.C */

#define EM_INL  0x0001	/* Include initial NL */
#define EM_IQM  0x0002	/* Include initial ? */
#define EM_IPC  0x0004	/* Include initial % */
#define EM_NAME 0x0008	/* Include program name */
#define EM_CODE 0x0010	/* Include error code */
#define EM_TEXT 0x0020	/* Include error message text */
#define EM_FNL  0x0040	/* Include final NL */
#define EM_EXIT 0x8000	/* Terminate after displaying message */

extern void errmsg(int bits, char *buffer, long code, char *msg1, char *msg2);

extern void femsg(char *name, long code, char *file);

extern void femsg2(char *name, char *msg, long code, char *file);

#endif  // _ERRMSG_H_
