//------------------------------------------------------------------------------
//
//  XOSSTR.H - XOS header file for string functions.
//
//  Edit History:
//  -------------
//   7Sep92 (brn) - Add comment header.
//  17Aug94 (fpj) - Modified for new CLIBX.
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

#ifndef _XOSSTR_H_
#define _XOSSTR_H_

# include <_prolog.h>

extern char *strdup(char *s);

extern int stricmp(const char *s1, const char *s2);

extern char *stristr(char *s1, char *s2);

extern char *strlower(char *s);

extern char *strlwr(char *s);

extern char *strmov(char *dest, const char *src);

extern char *strnchr(char *str, char chr, int len);

extern int strnicmp(const char *s1, const char *s2, size_t maxlen);

extern char *strnmov(char *dest, const char *src, size_t maxlen);

extern char *strnupper(char *s, int len);

extern char *strnupr(char *s, int len);

extern char *strupper(char *s);

extern char *strupr(char *s);

extern char *strncpyn(char *dst, char *src, int length);

# include <_epilog.h>

#endif // _XOSSTR_H_
