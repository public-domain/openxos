//------------------------------------------------------------------------------
//
//  SETJMP.H - ANSI C header file for non-local jumps
//
//  Edit History:
//  -------------
//    7Sep92 (brn) - Add comment header
//   17Aug94 (fpj) - Modified for new CLIBX.
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

#ifndef _SETJMP_H_
#define _SETJMP_H_

# include <_prolog.h>

typedef struct
{
    unsigned long longjmp_ESP;          // Saved stack pointer
    unsigned long longjmp_EFLAG;        // Saved flags
    unsigned long longjmp_EIP;          // Saved instruction pointer
    unsigned long longjmp_EBP;          // Saved frame pointer
    unsigned long longjmp_EDI;          // Saved destination pointer
    unsigned long longjmp_ESI;          // Saved source pointer
} jmp_buf[1];

//
// Function prototypes
//

extern void longjmp(jmp_buf env, int val);

extern int setjmp(jmp_buf env);


# include <_epilog.h>

#endif	// _SETJMP_H_
