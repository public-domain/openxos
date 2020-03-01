//------------------------------------------------------------------------------
//
//  XOSSVC.H - Definitions for XOS system service calls
//
//  Edit History:
//  -------------
//  09/07/92(brn) - Add comment Header
//  13Apr95 (fpj) - Delete svc_xxx() prototypes.
//  20Jun95 (fpj) - Added prototypes for 16-bit code.
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

#ifndef _XOSSVC_H_
#define _XOSSVC_H_

//------------------------------------------------------------------------------
//
//  Notes on the layout of this file:
//
//  1. We presently recognize two C compilers: BORLAND and WATCOM. To add
//     support for additional compilers, modify the #if statement below so
//     that "XOSSVC" is defined in such a way that the compiler will know
//     to generate a "far" call, using the PASCAL calling convention, to
//     to a function that will return a "long".
//
//  2. There are prototypes for two different environments: 16-bit and 32-bit.
//     The reasons for this are that there are some svcs that exist or make
//     sense in only one environment, although most svcs are common to both.
//     Also, any svcs that use pointers must have separate prototypes, since
//     "far" pointers require 32 bits of stack space in the 16-bit environment,
//     and 64 bits of stack space in the 32-bit environment. Consequently, to
//     keep things simple within the kernel, we simply add some dummy arguments
//     to the stack in the 16-bit environment, so that the offsets of the argu-
//     ments will be in the same locations.
//
//     Note that the addition of such dummy arguments is not necessary for
//     the "pseudo-I/O" svcs, such as svcIoInSingle, since those are really
//     calls to library routines that will perform the necessary fixup.
//
//     Also note that if we do have to add dummy arguments, we do so for each
//     svc with a macro that has the same name as the function. This is allowed
//     in ANSI C, since expansion of a macro name is prohibited within the
//     macro itself.
//
//  3. The choice of which set of prototypes to use is determined by the
//     __16BIT__ macro. If it is defined, we use the 16-bit prototypes, else
//     we use the 32-bit prototypes.
//
//  4. When adding prototypes, keep in mind that the "int" keyword should be
//     avoided, since "ints" are different sizes in the 16- and 32-bit environ-
//     ments. Use "shorts" and "longs" instead.
//
//     Also, all pointers must be "far" pointers.
//
//------------------------------------------------------------------------------

# include <_prolog.h>

#if     defined(__WATCOMC__)

// Special stuff to make WATCOM C do the right things

#pragma aux __syscall "*" parm [] reverse routine modify [EAX] value [EAX];
#define XOSSVC long far __syscall

#elif   defined(__BORLANDC__) || defined(__TURBOC__)

#define XOSSVC long far pascal
#define XOSSVC long far pascal

#else

#error  ;Unknown or unsupported compiler!

#endif

#ifdef  __16BIT__

//------------------------------------------------------------------------------
//
//  Prototypes for 16-bit code
//
//------------------------------------------------------------------------------

// DPMI svcs

XOSSVC svcDpmiSetup(long flag, long dummy2, void far *regs);

#define svcDpmiSetup(flag, regs) \
        svcDpmiSetup(flag, 0, regs)

// IO svcs
                
XOSSVC svcIoCancel(long dummy1, void far *qab, long bits);

#define svcIoCancel(qab, bits) \
        svcIoCancel(0, qab, bits)

XOSSVC svcIoClear(long dev, long bits);

XOSSVC svcIoClose(long dev, long bits);

XOSSVC svcIoClsChar(char far *name, void far *chrlst);

XOSSVC svcIoCommit(long dev);

XOSSVC svcIoControl(long dummy1, void far *qab, long func, long data);

#define svcIoControl(qab, func, data) \
        svcIoControl(0, qab, func, data)

XOSSVC svcIoDefLog(long level, long type, long dummy3, char far *name,
        long dummy5, char far *defin);

#define svcIoDefLog(level, type, name, defin) \
        svcIoDefLog(level, type, 0, name, 0, defin)

XOSSVC svcIoDelete(long cmd, char far *name, void far *parm);

XOSSVC svcIoDevChar(long handle, void far *chrlst);

XOSSVC svcIoDevParm(long bits, char far *name, void far *parm);

XOSSVC svcIoDstName(long dummy1, char far *rs, long dummy3, char far *ms,
        long dummy5, char far *ds, long dl);

#define svcIoDstName(rs, ms, ds, dl) \
        svcIoDstName(0, rs, 0, ms, 0, ds, dl)

XOSSVC svcIoDupHandle(long oldhndl, long newhndl, long cmdbits);

XOSSVC svcIoFindLog(long level, long dummy2, char far *str,
        long dummy4, char far *name, long dummy6, char far *defin,
        long length, long dummy9, long far *skip);

#define svcIoFindLog(level, str, name, defin, length, skip) \
        svcIoFindLog(level, 0, str, 0, name, 0, defin, length, 0, skip)

XOSSVC svcIoGetHndlMap(long dummy1, unsigned char far *map, long size);

#define svcIoGetHndlMap(map, size) \
        svcIoGetHndlMap(0, map, size)

XOSSVC svcIoInBlock(long dev, char far *data, long size);

XOSSVC svcIoInBlockP(long dev, char far *data, long size,
        void far *parm);

XOSSVC svcIoInSingle(long dev);

XOSSVC svcIoInSingleP(long dev, void far *parm);

XOSSVC svcIoMakePipe(void);

XOSSVC svcIoOpen(long bits, char far *name, void far *parm);

XOSSVC svcIoOutBlock(long dev, char far *data, long size);

XOSSVC svcIoOutBlockP(long dev, char far *data, long size,
        void far *parm);

XOSSVC svcIoOutSingle(long dev, long byte);

XOSSVC svcIoOutSingleP(long dev, long byte, void far *parm);

XOSSVC svcIoOutString(long dev, char far *data, long size);

XOSSVC svcIoOutStringP(long dev, char far *data, long size,
        void far *parm);

XOSSVC svcIoPath(char far *name, long opt, char far *bfr, long size);

XOSSVC svcIoPorts(long func, long base, long num);

XOSSVC svcIoQueue(long dummy1, void far *qab);

#define svcIoQueue(qab) \
        svcIoQueue(0, qab)

XOSSVC svcIoRename(long cmd, char far *oname, char far *nname,
        void far *parm);

XOSSVC svcIoRun(long dummy1, void far *qab);

#define svcIoRun(qab) \
        svcIoRun(0, qab)

XOSSVC svcIoSetPos(long dev, long pos, long mode);

XOSSVC svcIoSpecial(long dev, long func, void far *addr, long count,
        void far *parm);

XOSSVC svcIoTransName(long func, long dummy2, char far *name,
        long dummy4, char far *buffer, long size);

#define svcIoTransName(func, name, buffer, size) \
        svcIoTransName(func, 0, name, 0, buffer, size)

XOSSVC svcIoWait(long dummy1, void far *pnt);

#define svcIoWait(pnt) \
        svcIoWait(0, pnt)

// Memory management svcs

XOSSVC svcMemChange(long dummy1, void far *base, long bits, long size);

#define svcMemChange(base, bits, size) \
        svcMemChange(0, base, bits, size)

XOSSVC svcMemConvShr(long, void far *addr, long, char far *name,
        long type, long level, long prot, long, void far *acl);

#define svcMemConvShr(addr, name, type, level, prot, acl) \
        svcMemConvShr(0, addr, 0, name, type, level, prot, 0, acl)

XOSSVC svcMemCopy2PM(long dummy1, void far *dest, long dummy3, void far *src,
        long count);

#define svcMemCopy2PM(dest, src, count) \
        svcMemCopy2PM(0, dest, 0, src, count);

XOSSVC svcMemCreate(long select, long type);

XOSSVC svcMemDebug(long func, long dummy3, void far *addr,
        long dummy5, long far *value);

#define svcMemDebug(func, addr, value) \
        svcMemDebug(func, 0, addr, 0, value)

XOSSVC svcMemLink(long newselect, long oldselect, long type);

XOSSVC svcMemLinkShr(long dummy1, void far *addr, long dummy3, char far *name,
        long level);

#define svcMemLinkShr(addr, name, level) \
        svcMemLinkShr(0, addr, 0, name, level)

XOSSVC svcMemMap(long dummy1, void far *base, long phys, long bits,
        long size);

#define svcMemMap(base, phys, bits, size) \
        svcMemMap(0, base, phys, bits, size)

XOSSVC svcMemNull(char far *base, long offset, long pagebits, long size);

XOSSVC svcMemPageType(long dummy1, void far *base, long bottom,
        long top, long bits);

#define svcMemPageType(base, bottom, top, bits) \
        svcMemPageType(0, base, bottom, top, bits)

XOSSVC svcMemSegType(long select, long type);

XOSSVC svcMemWPCntl(unsigned long mask);

XOSSVC svcMemWPSet(long number, long type, long size, long dummy4,
        void far *address);

#define svcMemWPSet(number, type, size, address) \
        svcMemWPSet(number, type, size, 0, address)

// Scheduler svcs

XOSSVC svcSchAlarm(long func, long handle, long vect, long data,
        long date, long time);

XOSSVC svcSchClrEvent(long dummy1, char far *name, long mask);

#define svcSchClrEvent(name, mask) \
        svcSchClrEvent(0, name, mask)

XOSSVC svcSchClrSignal(long vector);

XOSSVC svcSchCtlCDone(void);

void svcSchDismiss(void);

XOSSVC svcSchExit(long status);

XOSSVC svcSchGetVector(long vector, long, void far **addr);

#define svcSchGetVector(vector, addr) \
        svcSchGetVector(vector, 0, addr)

XOSSVC svcSchInitVector(long type);

XOSSVC svcSchIntrProc(long pid, long func, long status);

void svcSchIRet(void);

XOSSVC svcSchKill(long status, long pid);

XOSSVC svcSchKillX(unsigned long status, unsigned long process, long dummy3,
        void far *memaddr, long dummy5, void far *cseip, unsigned long efr,
        unsigned long data);

#define svcSchKillX(status, process, memaddr, cseip, efr, data) \
        svcSchKillX(status, process, 0, memaddr, 0, cseip, efr, data)

XOSSVC svcSchMakEvent(long dummy1, char far *name, long size, long intr);

#define svcSchMakEvent(name, size, intr) \
        svcSchMakEvent(0, name, size, intr)

XOSSVC svcSchRelEvent(long dummy1, char far *name, long mask);

#define svcSchRelEvent(name, mask) \
        svcSchRelEvent(0, name, mask)

XOSSVC svcSchResEvent(long dummy1, char far *name, long event);

#define svcSchResEvent(name, event) \
        svcSchResEvent(0, name, event)

XOSSVC svcSchSetEvent(long dummy1, char far *name, long event, long val,
    long pid);

#define svcSchSetEvent(name, event, val, pid) \
        svcSchSetEvent(0, name, event, val, pid)

XOSSVC svcSchSetLevel(long level);

XOSSVC svcSchSetVector(long vector, long lvl, long dummy3, void far *addr);

#define svcSchSetVector(vector, lvl, addr) \
        svcSchSetVector(vector, lvl, 0, addr)

XOSSVC svcSchSleep(void);

XOSSVC svcSchSuspend(long dummy1, void far *flag, long time);

#define svcSchSuspend(flag, time) \
        svcSchSuspend(0, flag, time)

XOSSVC svcSchWaitProc(long pid, long time);

XOSSVC svcSchWaitMEvent(long dummy1, char far *name, long mask, long timeout);

#define svcSchWaitMEvent(name, mask, timeout) \
        svcSchWaitMEvent(0, name, mask, timeout)

XOSSVC svcSchWaitSEvent(long dummy1, char far *name, long event, long timeout);

#define svcSchWaitSEvent(name, event, timeout) \
        svcSchWaitSEvent(0, name, event, timeout)

XOSSVC svcSchWaitSignal(long dummy1, void far *flag, long time);

#define svcSchWaitSignal(flag, time) \
        svcSchWaitSignal(0, flag, time)

// General system svcs

XOSSVC svcSysCmos(long addr, long data);

XOSSVC svcSysDateTime(long func, long dummy2, void far *data);

#define svcSysDateTime(func, data) \
        svcSysDateTime(func, 0, data)

XOSSVC svcSysDefEnv(long level, long dummy2, void far *str,
        long dummy4, void far *def);

#define svcSysDefEnv(level, str, def) \
        svcSysDefEnv(level, 0, str, 0, def)

XOSSVC svcSysErrMsg(long code, long format, long dummy3, void far *str);

#define svcSysErrMsg(code, format, str) \
        svcSysErrMsg(code, format, 0, str)

XOSSVC svcSysFindEnv(long level,  long dummy2, char far *str,
        long dummy4, char far *fnd, long dummy6, char far *defin,
        long length, long dummy9, long far *skip);

#define svcSysFindEnv(level, str, fnd, defin, length, skip) \
        svcSysFindEnv(level, 0, str, 0, fnd, 0, defin, length, 0, skip)

XOSSVC svcSysGetEnv(long level, long dummy2, char far *buffer, long size);

#define svcSysGetEnv(level, buffer, size) \
        svcSysGetEnv(level, 0, buffer, size)

XOSSVC svcSysGetInfo(long func, long data, long dummy3, void far *bfr,
        long size);

#define svcSysGetInfo(func, data, bfr, size) \
        svcSysGetInfo(func, data, 0, bfr, size)

XOSSVC svcSysGetPData(long pid, long dummy2, void far *data);

#define svcSysGetPData(pid, data) \
        svcSysGetPData(pid, 0, data)

XOSSVC svcSysGetPid(void);

XOSSVC svcSysGetPLevel(long pid);

XOSSVC svcSysGetPName(long pid, long dummy2, void far *str);

#define svcSysGetPName(pid, str) \
        svcSysGetPName(pid, 0, str)

XOSSVC svcSysLog(long dummy1, char far *msg, long size);

#define svcSysLog(msg, size) \
        svcSysLog(0, msg, size)

XOSSVC svcSysSetPName(long dummy1, void far *name);

#define svcSysSetPName(name) \
        svcSysSetPName(0, name)

//  Terminal svcs

XOSSVC svcTrmAttrib(long dev, long func, long dummy3, void far *data);

#define svcTrmAttrib(dev, func, data) \
        svcTrmAttrib(dev, func, 0, data)

XOSSVC svcTrmCurPos(long dev, long page, long chr, long line);

XOSSVC svcTrmCurType(long dev, long curtype);

XOSSVC svcTrmDspMode(long dev, long func, long dummy3, void far *data);

#define svcTrmDspMode(dev, func, data) \
        svcTrmDspMode(dev, func, 0, data)

XOSSVC svcTrmDspPage(long dev, long page);

XOSSVC svcTrmEgaPalet(long dev, long dummy2, void far *list);

#define svcTrmEgaPalet(dev, list) \
        svcTrmEgaPalet(dev, 0, list)

XOSSVC svcTrmFunction(long dev, long func);

XOSSVC svcTrmGetAtChr(long dev, long page);

XOSSVC svcTrmGetPixel(long dev, long page, long xcoord, long ycoord);

XOSSVC svcTrmLdCusFont(long dev, long table,  long size, long dummy,
	void far *data, long begin, long count);

#define svcTrmLdCusFont(dev, table, size, dummy, data, begin, count) \
	svcTrmLdCusFont(dev, table, size, dummy, 0, data, begin, count)

XOSSVC svcTrmLdStdFont(long dev, long table, long font, long begin, long count);

XOSSVC svcTrmMapScrn(long dev, long dummy2, void far *buffer, long size,
        long offset);

#define svcTrmMapScrn(dev, buffer, size, offset) \
        svcTrmMapScrn(dev, 0, buffer, size, offset)

XOSSVC svcTrmScroll(long dev, long page, long xul, long yul,
        long xlr, long ylr, long cnt, long attrib);

XOSSVC svcTrmSelFont(long descriptor, long fonta, long fontb);

XOSSVC svcTrmSetAtChr(long dev, long page, long chr, long attrib,
        long count);

XOSSVC svcTrmSetChr(long dev, long page, long chr, long color,
        long count);

XOSSVC svcTrmSetPixel(long dev, long page, long xcoord, long ycoord,
        long value);

XOSSVC svcTrmWrtInB(long dev, long dummy2, void far *str, long cnt);

#define svcTrmWrtInB(dev, str, cnt) \
        svcTrmWrtInB(dev, 0, str, cnt)

#else

//------------------------------------------------------------------------------
//
//  Prototypes for 32-bit code
//
//------------------------------------------------------------------------------

//  I/O svcs

XOSSVC svcIoCancel(void far *qab, long bits);

XOSSVC svcIoClear(long dev, long bits);

XOSSVC svcIoClose(long dev, long bits);

XOSSVC svcIoClsChar(char far *name, void far *chrlst);

XOSSVC svcIoCommit(long dev);

XOSSVC svcIoControl(void far *qab, long func, long data);

XOSSVC svcIoDefLog(long level, long type, char far *name, char far *defin);

XOSSVC svcIoDelete(long cmd, char far *name, void far *parm);

XOSSVC svcIoDevChar(long handle, void far *chrlst);

XOSSVC svcIoDevParm(long bits, char far *name, void far *parm);

XOSSVC svcIoDstName(char far *rs, char far *ms, char far *ds, long dl);

XOSSVC svcIoDupHandle(long oldhndl, long newhndl, long cmdbits);

XOSSVC svcIoFindLog(long level, char far *str, char far *name, char far *defin,
        long length, long far *skip);

XOSSVC svcIoGetHndlMap(unsigned char far *map, long size);

XOSSVC svcIoInBlock(long dev, char far *data, long size);

XOSSVC svcIoInBlockP(long dev, char far *data, long size, void far *parm);

XOSSVC svcIoInSingle(long dev);

XOSSVC svcIoInSingleP(long dev, void far *parm);

XOSSVC svcIoMakePipe(void);

XOSSVC svcIoOpen(long bits, char far *name, void far *parm);

XOSSVC svcIoOutBlock(long dev, char far *data, long size);

XOSSVC svcIoOutBlockP(long dev, char far *data, long size, void far *parm);

XOSSVC svcIoOutSingle(long dev, long byte);

XOSSVC svcIoOutSingleP(long dev, long byte, void far *parm);

XOSSVC svcIoOutString(long dev, char far *data, long size);

XOSSVC svcIoOutStringP(long dev, char far *data, long size, void far *parm);

XOSSVC svcIoPath(char far *name, long opt, char far *bfr, long size);

XOSSVC svcIoPorts(long func, long base, long num);

XOSSVC svcIoQueue(void far *qab);

XOSSVC svcIoRename(long cmd, char far *oname, char far *nname, void far *parm);

XOSSVC svcIoRun(void far *qab);

XOSSVC svcIoSetPos(long dev, long pos, long mode);

XOSSVC svcIoSpecial(long dev, long func, void far *addr, long count,
        void far *parm);

XOSSVC svcIoTransName(long func, char far *name, char far *buffer, long size);

XOSSVC svcIoWait(void far *pnt);

//  Memory management svcs

XOSSVC svcMemBlkAlloc(long lapn, long size, long bits, long clientnum);

XOSSVC svcMemBlkChange(long lapn, long size, long bits, long dostask);

XOSSVC svcMemBlkFree(long dostask);

XOSSVC svcMemChange(void far *base, long bits, long size);

XOSSVC svcMemConvShr(void far *addr, char far *name, long type, long level,
        long prot, void far *acl);

XOSSVC svcMemCreate(long select, long type);

XOSSVC svcMemDebug(long func, void far *addr, long far *value);

XOSSVC svcMemDescAlloc(long selector, long num, long kind);

XOSSVC svcMemDescFind(long kind, long la);

XOSSVC svcMemDescFree(long selector);

XOSSVC svcMemDescRead(long selector, void far *data);

XOSSVC svcMemDescSet(long selector, long func, long data);

XOSSVC svcMemDescWrite(long selector, void far *data);

XOSSVC svcMemDosSetup(long amount, void far *data);

XOSSVC svcMemLink(long newselect, long oldselect, long type);

XOSSVC svcMemLinkShr(void far *addr, char far *name, long level);

XOSSVC svcMemMap(void far *base, long phys, long bits, long size);

XOSSVC svcMemMove(long nselect, long noffset, long oselect, long offset);

XOSSVC svcMemNull(char far *base, long offset, long pagebits, long size);

XOSSVC svcMemPageType(void far *base, long bottom, long top, long bits);

XOSSVC svcMemRemove(long select);

XOSSVC svcMemRmvMult(long data);

XOSSVC svcMemSegType(long select, long type);

XOSSVC svcMemWPCntl(unsigned long mask);

XOSSVC svcMemWPSet(long number, long type, long size, void far *address);

//  Scheduler svcs

XOSSVC svcSchAlarm(long func, long handle, long vect, long data, long date,
        long time);

XOSSVC svcSchClrEvent(char far *name, long mask);

XOSSVC svcSchClrSignal(long vector);

XOSSVC svcSchCritDone(long *keyitem);

XOSSVC svcSchCritWaitB(long *keyitem);

XOSSVC svcSchCritWaitR(long *keyitem);

XOSSVC svcSchCtlCDone(void);

void svcSchDismiss(void);

XOSSVC svcSchExit(long status);

XOSSVC svcSysGetEnv(long level, char far *buffer, long size);

XOSSVC svcSchGetVector(long vector, void far **addr);

XOSSVC svcSchInitVector(long type);

XOSSVC svcSchIntrProc(long pid, long func, long status);

void svcSchIRet(void);

XOSSVC svcSchKill(long status, long pid);

XOSSVC svcSchKillX(unsigned long status, unsigned long process,
        void far *memaddr, void far *cseip, unsigned long efr,
        unsigned long data);

XOSSVC svcSchMakEvent(char far *name, long size, long intr);

XOSSVC svcSchRelEvent(char far *name, long mask);

XOSSVC svcSchResEvent(char far *name, long event);

XOSSVC svcSchSetEvent(char far *name, long event, long val, long pid);

XOSSVC svcSchSetLevel(long level);

XOSSVC svcSchSetVector(long vector, long lvl, void far *addr);

XOSSVC svcSchSleep(void);

XOSSVC svcSchSuspend(long far *flag, long time);

XOSSVC svcSchWaitProc(long pid, long time);

XOSSVC svcSchWaitMEvent(char far *name, long mask, long timeout);

XOSSVC svcSchWaitSEvent(char far *name, long event, long timeout);

XOSSVC svcSchWaitSignal(long far *flag, long time);

//  Screen symbiont svcs

XOSSVC svcSssClsAlm(long exectdb, long func, long size, long pos,
        char far *save);

XOSSVC svcSssDone(long exectdb, long clear[]);

XOSSVC svcSssGetMod(long exectdb, long far *value);

XOSSVC svcSssGetTdb(long descriptor);

XOSSVC svcSssOpnAlm(long exectdb, long size);

XOSSVC svcSssPutAlm(long exectdb, long size, long pos, char far *save,
        char far *data);

XOSSVC svcSssRead(long exectdb, char far *list[]);

XOSSVC svcSssSwitch(long oexectdb, long nexectdb, char far *list[]);

XOSSVC svcSssWrite(long exectdb, char far *list[]);

//  General system svcs

XOSSVC svcSysCmos(long addr, long data);

XOSSVC svcSysDateTime(long func, void far *data);

XOSSVC svcSysDefEnv(long level, void far *str, void far *def);

XOSSVC svcSysErrMsg(long code, long format, void far *str);

XOSSVC svcSysFindEnv(long level, char far *str, char far *fnd,
        char far *defin, long length, long far *skip);

XOSSVC svcSysGetEnv(long level, char far *buffer, long size);

XOSSVC svcSysGetInfo(long func, long data, void far *bfr, long size);

XOSSVC svcSysGetPData(long pid, void far *data);

XOSSVC svcSysGetPid(void);

XOSSVC svcSysGetPLevel(long pid);

XOSSVC svcSysGetPName(long pid, void far *str);

XOSSVC svcSysGetRandom(unsigned char far *data, long size);

XOSSVC svcSysLoadLke(void far *bfr);

XOSSVC svcSysLog(char far *msg, long size);

XOSSVC svcSysSetPName(void far *name);

//  Terminal svcs

XOSSVC svcTrmAttrib(long dev, long func, void far *data);

XOSSVC svcTrmCurPos(long dev, long page, long chr, long line);

XOSSVC svcTrmCurType(long dev, long curtype);

XOSSVC svcTrmDspMode(long dev, long func, void far *data);

XOSSVC svcTrmDspPage(long dev, long page);

XOSSVC svcTrmEgaPalet(long dev, void far *list);

XOSSVC svcTrmFunction(long dev, long func);

XOSSVC svcTrmGetAtChr(long dev, long page);

XOSSVC svcTrmGetPixel(long dev, long page, long xcoord, long ycoord);

XOSSVC svcTrmLdCusFont(long dev, long table,  long size, void far *data,
	long begin, long count);

XOSSVC svcTrmLdStdFont(long dev, long table, long font, long begin, long count);

XOSSVC svcTrmMapScrn(long dev, void far *buffer, long size, long offset);

XOSSVC svcTrmScroll(long dev, long page, long xul, long yul, long xlr,
        long ylr, long cnt, long attrib);

XOSSVC svcTrmSelFont(long descriptor, long fonta, long fontb);

XOSSVC svcTrmSetAtChr(long dev, long page, long chr, long attrib, long count);

XOSSVC svcTrmSetChr(long dev, long page, long chr, long color, long count);

XOSSVC svcTrmSetPixel(long dev, long page, long xcoord, long ycoord,
        long value);

XOSSVC svcTrmWrtInB(long dev, void far *str, long cnt);

#endif  // __16BIT__


# include <_epilog.h>

#endif  // _XOSSVC_H_
