//------------------------------------------------------------------------------
//
//  BASELINE.H - Internal header file for "baseline" definitions
//
//  Edit History:
//  -------------
//   4Jun95 (fpj) - Original creation.
//	03Aug00 (cey) - Create/move source to XOSSRC/INC
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

#ifndef _BASELINE_H_
#define _BASELINE_H_

#if     defined(__WATCOMC__)

// Special stuff to make WATCOM C do the right things

#pragma pack (0);
#pragma off (check_stack);
#pragma on (unreferenced);
#pragma aux default "*" modify [EAX EBX ECX EDX GS];

#endif  // defined(__WATCOM__)

#define _NULL   (void *)0

typedef unsigned int _size_t;

typedef long int _time_t;

typedef long int _clock_t;

typedef unsigned int _ptrdiff_t;

typedef unsigned short _wchar_t;

#define _CLOCKS_PER_SEC 50	// 50 ticks per second


typedef unsigned char  uchar;
typedef signed   char  schar;

typedef unsigned char  u8;
typedef signed   char  s8;

typedef unsigned short ushort;
typedef signed   short sshort;

typedef unsigned short u16;
typedef signed   short s16;

typedef unsigned int   uint;
typedef signed   int   sint;

typedef unsigned int   u32;
typedef signed   int   s32;

typedef unsigned long  ulong;
typedef signed   long  slong;

#ifndef FALSE

#define FALSE 0

#endif

#ifndef TRUE

#define TRUE  1

#endif

#endif  // _BASELINE_H_
