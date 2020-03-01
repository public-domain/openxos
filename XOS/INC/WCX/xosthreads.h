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

#ifndef XOSTHREADS_H
#define XOSTHREADS_H

#include <_prolog.h>

typedef struct thddata
{   QAB  qab;			// QAB for IO operations
    long number;		// Thread number
    char state;
    char wkseq;			// Wake sequence number
    char xxx[2];
    long alhndl;		// Alarm handle
    long stkalc;		// Base offset for allocated stack space
    long ESP;			// Saved ESP
} THDDATA;

extern THDDATA *thdData;

XOSSVC thdCtlAlarm(long thdbase, long func, long handle, time_s datetime);

XOSSVC thdCtlCreate(long thdbase, long stksize, long datasize,
    	void (* fnc)(), void *arglst, long argnum);

XOSSVC thdCtlInitialize(long maxnum, long vectbase, long membase, long stksize,
    	long datasize);

XOSSVC thdCtlKill(long thdbase);

XOSSVC thdCtlSuspend(long far *flags, ulong time);

XOSSVC thdCtlTerminate(void);

XOSSVC thdCtlWake(long thdbase);

XOSSVC thdIoClose(long handle, long cmdbits);

XOSSVC thdIoDelete(long cmdbits, char far *name, void far *parms);

XOSSVC thdIoDevParm(long cmdbits, char far *name, void far *parms);

XOSSVC thdIoFunc(QAB far *qab);

XOSSVC thdIoInBlock(long handle, char far *buffer, long count);

XOSSVC thdIoInBlockP(long handle, char far *buffer, long count,
    	void far *parms);

XOSSVC thdIoOpen(long cmdbits, char far *name, void far *parms);

XOSSVC thdIoOutBlock(long handle, char far *buffer, long count);

XOSSVC thdIoOutBlockP(long handle, char far *buffer, long count,
    	void far *parms);

XOSSVC thdIoOutString(long handle, char far *buffer, long count);

XOSSVC thdIoOutStringP(long handle, char far *buffer, long count,
    	void far *parms);

XOSSVC thdIoRename(long cmdbits, char far *newname, char far *oldname,
		void far *parms);

XOSSVC thdIoSpecial(long handle, long func, char far *buffer, long count,
    	void far *parms);

# include <_epilog.h>

#endif
