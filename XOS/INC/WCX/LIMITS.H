//------------------------------------------------------------------------------
//
//  LIMITS.H - ANSI C header file for environmental limits
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

#ifndef _LIMITS_H_
#define _LIMITS_H_

#ifndef _BASELINE_H_

#include <baseline.h>

#endif // _BASELINE_H_


#define CHAR_BIT        8               // No. of bits in a char

#define SCHAR_MIN       0x80
#define SCHAR_MAX       0x7F
#define UCHAR_MAX       0xFFu

#if     ('\x80' > 0)                    // If char is unsigned

#define CHAR_MIN        0
#define CHAR_MAX        UCHAR_MAX

#else                                   // If char is signed

#define CHAR_MIN        SCHAR_MIN
#define CHAR_MAX        SCHAR_MAX

#endif

#define SHRT_MIN        0x8000
#define SHRT_MAX        0x7FFF
#define USHRT_MAX       0xFFFFu

#ifdef  _2INT                           // FIXME: use proper symbol here

#define INT_MIN         0x8000
#define INT_MAX         0x7FFF
#define UINT_MAX        0xFFFFu

#else

#define INT_MIN         0x80000000
#define INT_MAX         0x7FFFFFFF
#define UINT_MAX        0xFFFFFFFFu

#endif

#define LONG_MIN        0x80000000L
#define LONG_MAX        0x7FFFFFFFL
#define ULONG_MAX       0xFFFFFFFFuL

#define MB_LEN_MAX      1               // Max. no. of bytes in multibyte char

#endif  // _LIMITS_H_

