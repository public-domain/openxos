//------------------------------------------------------------------------------
//
//  CTYPE.H - ANSI C header file for character-handling functions
//
//  Edit History:
//  -------------
//  17Aug94 (fpj) - Modified for new CLIBX.
//	01Aug00 (cey) - Create XOSSRC\INC as official home/source dir.
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

#ifndef _CTYPE_H_
#define _CTYPE_H_

# include <_prolog.h>

//
// Function prototypes
//

extern int isalnum(int c);

extern int isalpha(int c);

extern int isascii(int c);

extern int iscntrl(int c);

extern int isdigit(int c);

extern int isgraph(int c);

extern int islower(int c);

extern int isprint(int c);

extern int ispunct(int c);

extern int isspace(int c);

extern int isupper(int c);

extern int isxdigit(int c);

extern int toascii(int c);

extern int tolower(int c);

extern int toupper(int c);

#define _UP     0x1                     // Upper-case
#define _LO     0x2                     // Lower-case
#define _DG     0x4                     // Numeric (decimal digit)
#define _WS     0x8                     // Whitespace
#define _PN     0x10                    // Punctuation
#define _CN     0x20                    // Control
#define _BL     0x40                    // Blank (ASCII 0x20)
#define _HX     0x80                    // Hexidecimal digit
#define _XS     0x100                   // Extra space
#define _XA     0x200                   // Extra alphabetic

extern const char _chrtbl[];

#define isalnum(c)      ((_chrtbl)[c] & (_UP | _LO | _DG))
#define isalpha(c)      ((_chrtbl)[c] & (_UP | _LO))
#define isascii(c)      ((unsigned)(c) <= 0x7F)
#define iscntrl(c)      ((_chrtbl)[c] & _CN)
#define isdigit(c)      ((_chrtbl)[c] & _DG)
#define isgraph(c)      ((_chrtbl)[c] & (_PN | _UP | _LO | _DG))
#define islower(c)      ((_chrtbl)[c] & _LO)
#define isprint(c)      ((_chrtbl)[c] & (_PN | _UP | _LO | _DG | _BL))
#define ispunct(c)      ((_chrtbl)[c] & _PN)
#define isspace(c)      ((_chrtbl)[c] & _WS)
#define isupper(c)      ((_chrtbl)[c] & _UP)
#define isxdigit(c)     ((_chrtbl)[c] & _HX)
#define toascii(c)      ((c) & 0x7F)
#define _tolower(c)     (isupper(c) ? (c) - 'A' + 'a' : (c))
#define _toupper(c)     (islower(c) ? (c) - 'a' + 'A' : (c))

# include <_epilog.h>

#endif    // _CTYPE_H_
