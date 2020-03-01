//------------------------------------------------------------------------------
//
//  MKBOOT.H - Definitions for bootstrap functions
//
//  Edit History:
//  -------------
//  18May95 (fpj) - Added comment block.
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

#ifndef _MKBOOT_H_
#define _MKBOOT_H_

#ifndef _BASELINE_H_

#include <baseline.h>

#endif // _BASELINE_H_


typedef void (*mkbfail)(char *str1, long code, char *str2);
typedef void (*mkbnotice)(char *str);

int mkbootf(char diskname[], char *disktype, char fname[10][50],
    char defprog[], char autoboot, int timeout, char quiet,
    mkbfail fail, mkbnotice notice);

int rmbootf(char diskname[], char *disktype, mkbfail fail);

#endif  // _MKBOOT_H_
