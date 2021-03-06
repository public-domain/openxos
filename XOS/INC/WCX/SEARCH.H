//------------------------------------------------------------------------
// SEARCH.H
// Header file for sorting and searching for XOS
//
// Written by: Bruce R. Nevins
//
// Edit History:
// 08/26/88(brn) - Created first version
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

#ifndef _SEARCH_H
#define _SEARCH_H

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned size_t;
#endif	// _SIZE_T

char *lsearch(char *, char *, unsigned int *, unsigned int, int (*)(void *,
    void *));
char *lfind(char *, char *, unsigned int *, unsigned int, int (*)(void *,
    void *));
void *bsearch(const void *, const void *, size_t, size_t,
    int (*)(const void *, const void *));
void  qsort(void *, size_t, size_t, int (*)(const void *, const void*));
void *heapsort(void *root, int (*comp)(void *a, void *b, void *data),
    void *data);

#endif	// _SEARCH_H
