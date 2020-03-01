//------------------------------------------------------------------------------
//
//  STDARG.H - ANSI C header file for variable numbers of arguments
//
//  Edit History:
//  -------------
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

#ifndef _STDARG_H_
#define _STDARG_H_

# include <_prolog.h>

#ifndef _VA_LIST
#define _VA_LIST

typedef void *va_list;

#endif  // _VA_LIST

#define __size(x) ((sizeof(x)+sizeof(int)-1) & ~(sizeof(int)-1))

#define va_start(ap, parmN) ((void)((ap) = (va_list)((char *)(&parmN)\
  +__size(parmN))))

#define va_arg(ap, type) (*(type *)(((*(char * *)&(ap))\
  +=__size(type))-(__size(type))))

#define va_end(ap) ((void)0)

# include <_epilog.h>

#endif  // _STDARG_H_
