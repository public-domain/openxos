/*--------------------------------------------------------------------------*
 * XCMALLOC.H
 * XC specific memory management functions
 *
 * Written by: Bruce R. Nevins
 *
 * Edit History:
 * 17-Oct-92(brn) - Created first version
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

#ifndef _XCMALLOC_H
#define _XCMALLOC_H

typedef struct _mab
{   char *mab_membase;
    char *mab_mempnt;
    char *mab_memtop;
} MAB;

void *sbrkx(long incr, MAB *mb);
void *sbrkxx(long incr, MAB *mb);
void *brkx(long base, MAB *mb);
char *getspace(long size);
void  junkx(MAB *mb);
long  makex(MAB *mb, long offset);

#endif	// _XCMALLOC_H
