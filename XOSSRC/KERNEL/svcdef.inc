	.SBTTL	Parameter file SVCDEF.INC

;++++
; This software is in the public domain.  It may be freely copied and used
; for whatever purpose you see fit, including commerical uses.  Anyone
; modifying this software may claim ownership of the modifications, but not
; the complete derived code.  It would be appreciated if the authors were
; told what this software is being used for, but this is not a requirement.

;   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
;   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
;   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
;   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
;   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
;   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
;   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
;   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
;   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
;   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;----

	.NLBEX
	.PARM

	.MACRO	SVCDEFSYS
 SD 					;0100:0000  ED00:0000  (300)
					;  Not used
 SD  1, SchExit      ,  1, SchExit	;0108:0000  ED00:0004  (301)
					;  Terminate process
 SD  2, SchKill      ,  2, SchKill	;0110:0000  ED00:0008  (302)
	 				;  Terminate any process
 SD  8, SchKillX     ,  8, SchKillX	;0118:0000  ED00:000C  (303)
					;  Extended terminate process
 SD  2, SchSpawn			;0120:0000
					;  Create process
 SD					;0128:0000  ED00:0014  (305)
					;  Not used
 SD  3, SchSuspend   ,  3, SchSuspend	;0130:0000  ED00:0018  (306)
					;  Suspend process
 SD  0, SchSleep     ,  0, SchSleep	;0138:0000  ED00:001C  (307)
					;  Sleep
 SD  6, SchAlarm     ,  6, SchAlarm	;0140:0000  ED00:0020  (308)
					;  Request signal at specified time
 SD  0, SchRstUsrStk ,  1, SchRMRet	;0148:0000  ED00:0024  (309)
					;  Restore user stack
					;  Return from real mode
 SD  4, SchMakEvent  ,  4, SchMakEvent	;0150:0000  ED00:0028  (30A)
					;  Make event cluster
 SD  3, SchResEvent  ,  3, SchResEvent	;0158:0000  ED00:002C  (30B)
					;  Reserve event
 SD  3, SchRelEvent  ,  3, SchRelEvent	;0160:0000  ED00:0030  (30C)
					;  Release event
 SD  5, SchSetEvent  ,  5, SchSetEvent	;0168:0000  ED00:0034  (30D)
					;  Declare event
 SD  3, SchClrEvent  ,  3, SchClrEvent	;0170:0000  ED00:0038  (30E)
					;  Clear event(s)
 SD  4, SchWaitSEvent,  4, SchWaitSEvent;0178:0000  ED00:003C  (30F)
					;  Wait for single event
 SD  4, SchWaitMEvent,  4, SchWaitMEvent;0180:0000  ED00:0040  (310)
					;  Wait for multiple events
 SD  2, SchWaitProc  ,  2, SchWaitProc	;0188:0000  ED00:0044  (311)
					;  Wait for process to terminate
 SD  1, SchSetLevel  ,  1, SchSetLevel	;0190:0000  ED00:0048  (312)
					;  Set signal level
 SD  4, SchSetVector ,  4, SchSetVector	;0198:0000  ED00:004C  (313)
					;  Set signal vector
 SD  3, SchGetVector ,  3, SchGetVector	;01A0:0000  ED00:0050  (314)
					;  Get signal vector
 SD  1, SchDismiss   ,  1, SchDismiss	;01A8:0000  ED00:0054  (315)
					;  Dismiss signal
 SD  1, SchIRet      ,  1, SchIRet	;01B0:0000  ED00:0058  (316)
					;  Extended IRET
 SD  1, SchInitVector,  1, SchInitVector;01B8:0000  ED00:005C  (317)
					;  Not used
 SD  1, SchClrSignal ,  1, SchClrSignal	;01C0:0000  ED00:0060  (318)
					;  Clear signals for vector
 SD  5, SchReqSignal ,  5, SchReqSignal	;01C8:0000  ED00:0064  (319)
					;  Request signal
 SD  5, SchDisSignal ,  5, SchDisSignal	;01D0:0000  ED00:0060  (31A)
					;  Discard signals
 SD  3, SchIntrProc  ,  3, SchIntrProc	;01D8:0000  ED00:006C  (31B)
					;  Interrupt process
 SD  0, SchCtlCDone  ,  0, SchCtlCDone	;01E0:0000  ED00:0070  (31C)
					;  Report cntl-C processing done
 SD  2, SchCritWaitB ,  2, SchCritWaitB	;01E8:0000  ED00:0074  (31D)
					;  Wait for busy critical region
 SD  2, SchCritWaitR ,  2, SchCritWaitR	;01F0:0000  ED00:0078  (31E)
					;  Wait for requested critical region
 SD  2, SchCritDone  ,  2, SchCritDone	;01F8:0000  ED00:007C  (31F)
					;  Critical region done
 SD  1, MemRmvMult			;0200:0000
					;  Remove multiple segments
 SD  2, MemCreate    ,  2, MemCreate	;0208:0000  ED00:0084  (321)
					;  Create segment
 SD  1, MemRemove			;0210:0000
					;  Remove segment
 SD  4, MemChange    ,  4, MemChange	;0218:0000  ED00:008C  (323)
					;  Allocate or deallocate memory pages
 SD  4, MemMove				;0220:0000
					;  Move msect
 SD  3, MemLink      ,  3, MemLink	;0228:0000  ED00:0094  (325)
					;  Link segments
 SD  5, MemMap       ,  5, MemMap	;0230:0000  ED00:0098  (326)
					;  Map physical or system memory
 SD  4, MemNull      ,  4, MemNull	;0238:0000  ED00:009C  (327)
					;  Map null physical memory
 SD  3, MemDosSetup  ,  0, DpmiSetup	;0240:0000  ED00:00A0  (328)
					;  Set up V86 segment for DOS program
					;  Initial DPMI entry set up
 SD  9, MemConvShr   ,  9, MemConvShr	;0248:0000  ED00:00A4  (329)
					;  Convert msect to shared section
 SD  5, MemLinkShr   ,  5, MemLinkShr	;0250:0000  ED00:00A8  (32A)
					;  Link to shared section
 SD  2, MemSegType   ,  2, MemSegType	;0258:0000  ED00:00AC  (32B)
					;  Change segment type
 SD  5, MemPageType  ,  5, MemPageType	;0260:0000  ED00:00B0  (32C)
					;  Change page(s) type
 SD  5, MemDebug     ,  5, MemDebug	;0268:0000  ED00:00B4  (32D)
					;  Memory debug functions
 SD  5, MemWPSet     ,  5, MemWPSet	;0270:0000  ED00:00B8  (32E)
					;  Set or clear watchpoint
 SD  1, MemWPCntl    ,  1, MemWPCntl	;0278:0000  ED00:00BC  (32F)
					;  Get memory debug status bits (DR6)
 SD  4, MemBlkAlloc  ,  4, MemCopy2PM	;0280:0000  ED00;00C0  (330)
					;  Allocate linear address space block
					;  Copy to protected mode memory
 SD  4, MemBlkChange			;0288:0000
					;  Change size of linear address block
 SD  5, MemBlkType			;0290:0000
					;  Change type of linear address block
 SD  1, MemBlkFree                      ;0298:0000
					;  Free up all blocks for DOS task
 SD  3, MemDescAlloc			;02A0:0000
					;  Allocate local descriptors
 SD  1, MemDescFree			;02A8:0000
					;  Free local descriptors
 SD  3, MemDescWrite			;02B0:0000
					;  Write local descriptors
 SD  3, MemDescRead			;02B8:0000
					;  Read local descriptors
 SD  3, MemDescSet			;02C0:0000
					;  Set local descriptors
 SD  2, MemDescFind			;02C8:0000
					;  Find descriptor given linear address
					;    and kind
 SD  0, SysGetPid    ,  0, SysGetPid	;02D0:0000  ED00:00E8  (33A)
					;  Get process ID
 SD  3, SysGetPName  ,  3, SysGetPName	;02D8:0000  ED00:00EC  (33B)
					;  Get process name
 SD  2, SysSetPName  ,  2, SysSetPName	;02E0:0000  ED00:00F0  (33C)
					;  Set process name
 SD  1, SysGetPLevel ,  1, SysGetPLevel	;02E8:0000  ED00:00F4  (33D)
					;  Get process level
 SD  3, SysGetPData  ,  3, SysGetPData	;02F0:0000  ED00:00F8  (33E)
					;  Get process data
 SD		 			;02F8:0000  ED00:00FC  (33F)
					;  Not used
 SD  5, SysGetInfo   ,  5, SysGetInfo	;0300:0000  ED00:0100  (340)
					;  Get system information
 SD					;0308:0000  ED00:0104  (341)
					;  Not used
 SD  2, SysSetBillLvl,  2, SysSetBillLvl;0310:0000  ED00:0108  (342)
					;  Set billing level
 SD  3, SysDateTime  ,  3, SysDateTime	;0318:0000  ED00:010C  (343)
					;  Get or set date and time of day
 SD  3, SysLog       ,  3, SysLog	;0320:0000  ED00:0110  (344)
					;  Place message in system log
 SD  5, SysDefEnv    ,  5, SysDefEnv	;0328:0000  ED00:0114  (345)
					;  Define environment string
 SD 10, SysFindEnv   , 10, SysFindEnv	;0330:0000  ED00:0118  (346)
					;  Search for environment string
 SD  4, SysErrMsg    ,  4, SysErrMsg	;0338:0000  ED00:011C  (347)
					;  Generate error message
 SD  2, SysCmos      ,  2, SysCmos	;0340:0000  ED00:0120  (348)
					;  Read or write CMOS memory data
 SD  2, SysLoadLke   ,  0, SchExitRXM	;0348:0000  ED00:0124  (349)
					;  Load loadable kernel extension
					;    (protected mode only)
					;  Exit real exec mode (real mode only)
 SD  4, SysGetEnv    ,  4, SysGetEnv	;0350:0000  ED00:0128  (34A)
					;  Get all environment strings
 SD  3, SysGetRandom ,  3, SysGetRandom	;0358:0000  ED00:012C  (34B)
					;  Get random bytes
 SD					;0360:0000  ED00:0130  (34C)
					;  Not used
 SD					;0368:0000  ED00:0134  (34D)
					;  Not used
 SD					;0370:0000  ED00:0138  (34E)
					;  Not used
 SD					;0378:0000  ED00:013C  (34F)
					;  Not used
 SD  2, IoQueue      ,  2, IoQueue	;0380:0000  ED00:0140  (350)
					;  Queue IO function
 SD  3, IoCancel     ,  3, IoCancel	;0388:0000  ED00:0144  (351)
					;  Cancel IO request(s)
 SD  2, IoWait       ,  2, IoWait	;0390:0000  ED00:0148  (352)
					;  Wait for IO operation to finish
 SD  4, IoControl    ,  4, IoControl	;0398:0000  ED00:014C  (353)
					;  Queued IO control functions
 SD  0, IoMakePipe   ,  0, IoMakePipe	;03A0:0000  ED00:0150  (354)
					;  Create pipe
 SD  2, IoDupHandle  ,  2, IoDupHandle	;03A8:0000  ED00:0154  (355)
					;  Duplicate device handle
 SD  3, IoGetHndlMap ,  3, IoGetHndlMap	;03B0:0000  ED00:0158  (356)
					;  Get device handle map
 SD  1, IoClear      ,  1, IoClear	;03B8:0000  ED00:015C  (357)
					;  Clear IO device(s)
 SD  6, IoDefLog     ,  6, IoDefLog	;03C0:0000  ED00:0160  (358)
					;  Define logical name
 SD 10, IoFindLog    , 10, IoFindLog	;03C8:0000  ED00:0164  (359)
					;  Search for logical name
 SD  6, IoTransName  ,  6, IoTransName	;03D0:0000  ED00:0168  (35A)
					;  Translate logical name
 SD					;03D8:0000  ED00:016C  (35B)
					;  Not used
 SD  3, IoPorts      ,  3, IoPorts	;03E0:0000  ED00:0170  (35C)
					;  Allocate IO ports for direct access
 SD					;03E8:0000  ED00:0174  (35D)
					;  Not used
 SD					;03F0:0000  ED00:0178  (35E)
					;  Not used
 SD					;03F8:0000  ED00:017C  (35F)
					;  Not used
 SD		 			;0400:0000  ED00:0180  (360)
					;  Not used
 SD  2, TrmFunction  ,  2, TrmFunction	;0408:0000  ED00:0184  (361)
					;  General terminal functions
 SD  4, TrmDspMode   ,  4, TrmDspMode	;0410:0000  ED00:0188  (362)
					;  Get or set display mode
 SD  5, TrmMapScrn   ,  5, TrmMapScrn	;0418:0000  ED00:018C  (363)
					;  Map screen buffer
 SD  2, TrmCurType   ,  2, TrmCurType	;0420:0000  ED00:0190  (364)
					;  Get or set cursor type
					;  (INT 10, function 01)
 SD  4, TrmCurPos    ,  4, TrmCurPos	;0428:0000  ED00:0194  (365)
					;  Get or set cursor  position
					;  (INT 10, function 02)
 SD  2, TrmDspPage   ,  2, TrmDspPage	;0430:0000  ED00:0198  (366)
					;  Get or set display page
					;  (INT 10, function 05)
 SD 10, TrmScroll    , 10, TrmScroll	;0438:0000  ED00:019C  (367)
					;  Scroll wnidow
					;  (INT 10, function 06 and 07)
 SD  2, TrmGetAtChr  ,  2, TrmGetAtChr	;0440:0000  ED00:01A0  (368)
					;  Get attribute and character
					;  (INT 10, function 08)
 SD  5, TrmSetAtChr  ,  5, TrmSetAtChr	;0448:0000  ED00:01A4  (369)
					;  Set attribute and character
					;  (INT 10, function 09)
 SD  5, TrmSetChr    ,  5, TrmSetChr	;0450:0000  ED00:01A8  (36A)
					;  Set character only
					;  (INT 10, function 0A)
 SD  5, TrmLdStdFont ,  5, TrmLdStdFont	;0458:0000  ED00:01AC  (36B)
					;  Load standard display font
					;  (INT 10, function 11)
 SD  7, TrmLdCusFont ,  7, TrmLdCusFont	;0460:0000  ED00:01B0  (36C)
					;  Load custom display font
					;  (INT 10, function 11)
 SD  3, TrmSelFont   ,  3, TrmSelFont	;0468:0000  ED00:01B4  (36D)
					;  Select font toable and mode
					;  (INT 10, function 11)
 SD  4, TrmAttrib    ,  4, TrmAttrib	;0470:0000  ED00:01B8  (36E)
					;  Get or set screen attributes
 SD  4, TrmWrtInB    ,  4, TrmWrtInB	;0478:0000  ED00:01BC  (36F)
					;  Write to input buffer
 SD					;0480:0000  ED00:01C0  (370)
					;  Not used
 SD					;0488:0000  ED00:01C4  (371)
					;  Not used
 SD 10, TrmGSetCurPat,			;0490:0000  ED00:01C8
					;  Set graphic cursor pattern
 SD  4, TrmGSetCurCol,			;0498:0000  ED00:01CC
					;  Set graphic cursor colors
 SD  4, TrmGSetCurPos,			;04A0:0000  ED00:01D0
					;  Set graphic cursor position
 SD  3, TrmGGetCurPos,			;04A8:0000  ED00:01D4
					;  Get graphic cursor position
 SD					;04B0:0000  ED00:01D8  (372)
					;  Not used
 SD					;04B8:0000  ED00:01DC  (373)
					;  Not used
 SD					;04C0:0000  ED00:01E0
					;  Not used
 SD					;04C8:0000  ED00:01E4
					;  Not used
 SD					;04D0:0000  ED00:01E8
					;  Not used
 SD   ,		     ,  0, IoRun	;04D8:0000  ED00:01EC  (374)
					;  Run program (real mode only)
 SD  4, ScnMapBufr			;04E0:0000  ED00:01F0
					;  Map physical screen buffer
 SD  8, ScnTrans			;04E8:0000  ED00:01F4
					;  Transfer data for screen symbiont
 SD  8, ScnMaskWrt			;04F0:0000  ED00:01F8
					;  Masked write to display memory
 SD  2, ScnUtil				;04F8:0000  ED00:01FC
					;  Screen symbiont utility functions
 SD					;0500:0000
 SD					;0508:0000
 SD					;0510:0000
 SD					;0518:0000
 SD					;0520:0000
 SD					;0528:0000
 SD					;0530:0000
 SD					;0538:0000
 SD					;0540:0000
 SD					;0548:0000
 SD					;0550:0000
 SD					;0558:0000
 SD					;0560:0000
 SD					;0568:0000
 SD					;0570:0000
 SD					;0578:0000
 SD					;0580:0000
 SD					;0588:0000
 SD					;0590:0000
 SD					;0598:0000
 SD					;05A0:0000
 SD					;05A8:0000
 SD					;05B0:0000
 SD					;05B8:0000
 SD					;05C0:0000
 SD					;05C8:0000
 SD					;05D0:0000
 SD					;05D8:0000
 SD					;05E0:0000
 SD					;05E8:0000
 SD					;05F0:0000
 SD					;05F8:0000
	.ENDM

	.MACRO	SVCDEFUSER
 SD					;00A0:1000  ED00:0200
					;  Not used
 SD	IoOpen      , IoOpen		;00A0:1008  ED00:0204
					;  Open file/device
 SD					;00A0:1010  ED00:0208
					;  Create temporary file
 SD	IoClose     , IoClose		;00A0:1018  ED00:020C
					;  Close file/device
 SD	IoDevParm   , IoDevParm		;00A0:1020  ED00:0210
					;  Get IO parameter values
 SD	IoPath      , IoPath		;00A0:1028  ED00:0214
					;  Not used
 SD	IoSpecial   , IoSpecial		;00A0:1030  ED00:0218
					;  Special device functions
 SD	IoSetPos    , IoSetPos		;00A0:1038  ED00:021C
					;  Set position for IO
 SD	IoDevChar   , IoDevChar		;00A0:1040  ED00:0220
					;  Device characteristics functions
 SD	IoInSingle  , IoInSingle	;00A0:1048  ED00:0224
					;  Input single byte
 SD	IoInSingleP , IoInSingleP	;00A0:1050  ED00:0228
					;  Input single byte/parms
 SD	IoInBlock   , IoInBlock		;00A0:1058  ED00:022C
					;  Input block
 SD	IoInBlockP  , IoInBlockP	;00A0:1060  ED00:0230
					;  Input block/parms
 SD					;00A0:1068  ED00:0234
					;  Not used
 SD					;00A0:1070  ED00:0238
					;  Not used
 SD	IoOutSingle , IoOutSingle	;00A0:1078  ED00:023C
					;  Output single byte
 SD	IoOutSingleP, IoOutSingleP	;00A0:1080  ED00:0240
					;  Output single byte/parms
 SD	IoOutBlock  , IoOutBlock	;00A0:1088  ED00:0244
					;  Output block
 SD	IoOutBlockP , IoOutBlockP	;00A0:1090  ED00:0248
					;  Output block/parms
 SD	IoOutString , IoOutString	;00A0:1098  ED00:024C
					;  Output string
 SD	IoOutStringP, IoOutStringP	;00A0:10A0  ED00:0250
					;  Output string/parms
 SD					;00A0:10A8  ED00:0254
					;  Not used
 SD					;00A0:10B0  ED00:0258
					;  Not used
 SD	IoDelete    , IoDelete		;00A0:10B8  ED00:025C
					;  Delete file
 SD	IoRename    , IoRename		;00A0:10C0  ED00:0260
					;  Rename file
 SD	IoCommit    , IoCommit		;00A0:10C8  ED00:0264
					;  Commit output data
 SD	IoClsChar   , IoClsChar		;00A0:10D0  ED00:0268
					;  Class characteristics functions
 SD					;00A0:10D8  ED00:026C
					;  Not used
 SD					;00A0:10E0  ED00:0270
					;  Not used
 SD	IoRun       ,			;00A0:10E8  ED00:0274
					;  Run program
 SD					;00A0:10F0  ED00:0278
					;  Not used
 SD					;00A0:10F8  ED00:027C
					;  Not used
	.ENDM
