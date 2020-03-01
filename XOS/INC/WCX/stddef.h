//------------------------------------------------------------------------------
//
//  STDDEF.H - ANSI C header file for standard definitions
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

#ifndef _STDDEF_H_
#define _STDDEF_H_

#ifndef _BASELINE_H_

#include <baseline.h>

#endif // _BASELINE_H_


#define NULL    _NULL

#ifndef _SIZE_T
#define _SIZE_T

typedef _size_t size_t;

#endif

typedef _ptrdiff_t ptrdiff_t;

#ifndef _WCHAR_T
#define _WCHAR_T

typedef _wchar_t wchar_t;

#endif  // _WCHAR_T

#define offsetof(stype, field) \
    ((size_t)( (char *) &(((stype *)0)->field) - (char *)0))

#endif  // _STDDEF_H_

