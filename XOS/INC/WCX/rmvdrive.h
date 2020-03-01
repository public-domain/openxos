//------------------------------------------------------------------------------
//
//  RMVDRIVE.H - Definitions for RMVDRIVEF
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

#ifndef _RMVDRIVE_H_
#define _RMVDRIVE_H_

#ifndef _BASELINE_H_
#include <baseline.h>
#endif

#define RDML_INFO     1
#define RDML_ERROR    2
#define RDML_FININFO  3
#define RDML_FINERROR 4

typedef struct _typspec
{   int ltr;
    int max;
} TYPSPEC;


typedef struct _rmvspec
{   TYPSPEC cd;
    TYPSPEC syq;
    TYPSPEC zip;
    TYPSPEC ls;
    TYPSPEC oth;
} RMVSPEC;

int  rmvdrivef(int quiet, RMVSPEC *spec);
void rmvdrivemessage(int type, char *text);

#endif  // _RMVDRIVE_H_
