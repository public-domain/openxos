//------------------------------------------------------------------------------
//
//  STRING.H - ANSI C header file for string functions.
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

#ifndef _STRING_H_
#define _STRING_H_

# include <_prolog.h>

#ifndef _SIZE_T
#define _SIZE_T

typedef _size_t size_t;

#endif

//
// Prototypes for ANSI functions
//

extern void *memchr(const void *s, int c, size_t n);

extern void *memscan(const void *s, int c, size_t n);

extern int memcmp(const void *s1, const void *s2, size_t n);

//@@@ 00Aug01 CEY
int memicmp(const void *s1, const void *s2, size_t n);

extern void *memcpy(void *s1, const void *s2, size_t n);

extern void *memmove(void *s1, const void *s2, size_t n);

extern void *memset(void *s, int c, size_t n);

extern char *strcat(char *s1, const char *s2);

extern char *strchr(const char *s, int c);

extern int strcmp(const char *s1, const char *s2);
extern int stricmp(const char *s1, const char *s2);

extern int strcoll(const char *s1, const char *s2);

extern char *strcpy(char *s1, const char *s2);

extern size_t strcspn(const char *s1, const char *s2);

extern char *strerror(int errnum);

extern size_t strlen(const char *s);
extern size_t strnlen(const char *s, int len);

extern char *strncat(char *s1, const char *s2, size_t n);

extern int strncmp(const char *s1, const char *s2, size_t n);

extern char *strncpy(char *s1, const char *s2, size_t n);

extern char *strpbrk(const char *s1, const char *s2);

extern char *strrchr(const char *s, int c);

extern size_t strspn(const char *s1, const char *s2);

extern char *strstr(const char *s1, const char *s2);

extern char *strtok(char *s1, const char *s2);

extern char *strupr(char *s);

extern size_t strxfrm(char *s1, const char *s2, size_t n);

# include <_epilog.h>

#endif // _STRING_H_
