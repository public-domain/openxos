/*--------------------------------------------------------------------------*
 * MALLOC.H
 * Memory management functions
 *
 * Written by: Bruce R. Nevins
 *
 * Edit History:
 * 09/10/92(brn) - Created first version
 *
 *-------------------------------------------------------------------------*/

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

#ifndef _MALLOC_H
#define _MALLOC_H

//@@@ CEY-00Jul04

#ifdef __cplusplus

extern "C" {

#endif

//---

void *brk(void *new);
void *calloc(unsigned nitems, unsigned size);
void  free(void *block);
void *malloc(unsigned size);
void *realloc(void *buffer, size_t size);
void *sbrk(long incr);
void *sbrkp(long incr);

//@@@ CEY-00Jul04

#ifdef __cplusplus

}

#endif

//---

#endif	/* _MALLOC_H */
