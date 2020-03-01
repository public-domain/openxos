//------------------------------------------------------------------------------
//
//  ASSERT.H - ANSI C header file for assertions
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

#undef assert

#ifndef _BASELINE_H_

#include <baseline.h>

#endif // _BASELINE_H_


#ifdef  NDEBUG                          // If debugging features suppressed

#define assert(test)    ((void)0)

#else                                   // If debugging features enabled

#define _str(x) _val(x)
#define _val(x) #x

#define assert(test) ((test) ? (void)0 : \
    _assert("? " __FILE__ " (line " _str(__LINE__) "): assertion \"" #test \
    "\" failed\n"))

//
// Function prototypes
//

extern void _assert(const char *text);

#endif  // NDEBUG
