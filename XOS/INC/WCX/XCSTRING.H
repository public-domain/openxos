//------------------------------------------------------------------------
// XCSTR.H
// Standard XOS string functions header
//
// Written by: John R. Goltz
//
// Edit History:
// 16-Oct-92(JRG) - First version
//
//------------------------------------------------------------------------

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

#ifndef _ALGSTR_H
#define _ALGSTR_H

char  *strnchr(char *str, char chr, int len);
char  *strnmov(char *dest, const char *src, size_t maxlen);
char  *strmov(char *dest, const char *src);
char  *strupper(char *s);
char  *strlower(char *s);
void  *heapsort(void *root, int (*comp)(void *a, void *b, void *data),
    void *data);
int    stricmp(const char *s1, const char *s2);
int    strnicmp(const char *s1, const char *s2, size_t maxlen);
void	 movedata(unsigned srcseg, unsigned srcoff, unsigned dstseg,
		  unsigned dstoff, unsigned n);
int	 memicmp(void *s1, void *s2, unsigned n);
char  *strlwr(char *s);
char  *strupr(char *s);
char  *strdup(char *s);
char  *strset(char *s, char ch);
char  *strnset(char *s, char ch, size_t n);
char  *strrev(char *s);
char  *stristr(char *s1, char *s2);

#endif // _ALGSTR_H
