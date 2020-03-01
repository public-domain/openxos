//------------------------------------------------------------------------------
//
//  SIGNAL.H - ANSI C header file for signal-handling functions
//
//  Edit History:
//  -------------
//  17Aug94 (fpj) - Original creation.
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

#ifndef _SIGNAL_H_
#define _SIGNAL_H_

# include <_prolog.h>

typedef int sig_atomic_t;

typedef void _sigfun(int);

#define SIG_DFL         (_sigfun *)0    // Default handling
#define SIG_ERR         (_sigfun *)1    // Error handling
#define SIG_IGN         (_sigfun *)2    // Ignore signal

#define SIGABRT         1               // Program abort
#define SIGFPE          2               // Arithmetic error
#define SIGILL          3               // Illegal instruction
#define SIGINT          4               // Interactive attention
#define SIGSEGV         5               // Invalid memory access
#define SIGTERM         6               // Program termination

//
// Function prototypes
//

extern int raise(int sig);

extern _sigfun *signal(int sig, _sigfun *function);

int setvector(short vec, short level, void (*func)());

# include <_epilog.h>

#endif	// _SIGNAL_H_
