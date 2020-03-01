	TITLE	COMMAND

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

;This program does nothing but run SHELL.EXE in the same process - it is
;  intended to allow DOS programs to exec COMMAND.COM as they expect to!

;	PAGE	52, 132
;	INCLUDE	XOSMASM.PAR
;	INCLUDE	XOSERR.PAR
	.286C

;;;;;;;;;;
;;;;;;;;;;  The following is temporary...
;;;;;;;;;;

LF  =0Ah		;Line feed
CR  =0Dh		;Carriage return
RFNC$WAIT     =8000h	;Wait until function is complete
RFNC_RUN      =1	;Run program
PAR$SET    =80h		;Set (use) value of this parameter
  REP_STR    =15	;  String value
IOPAR_RUNCMDTAIL =1001h	; sS  Command tail (argument list)
IOPAR_RUNDEVLIST =1002h	; sS  Device list
RH$CHILDTERM=40000000h	;Function is not complete until child process terminates
ER_FILNF=-38		;File not found
ER_DIRNF=-42		;Directory not found
DH_STDIN =1		;Standard input device
DH_STDOUT=2		;Standard output device
DH_STDERR=3		;Standard error output device
DH_STDTRM=5		;Controlling terminal for session (cannot be redirected)
O$OUT     =00000002h	;Output is allowed
O$IN      =00000001h	;Input is allowed
qab_error  =4		;(4) Error code (returned by all functions)


seg_svc	SEGMENT AT 0EC00h

	PUBLIC	svcIoRun, svcSysErrMsg, svcIoOutString

	ORG	11Ch
svcSysErrMsg	LABEL FAR

	ORG	1D0h
svcIoRun	LABEL FAR

	ORG	24Ch
svcIoOutString	LABEL FAR

seg_svc	ENDS


;;;;;;;;;;
;;;;;;;;;; ...to here
;;;;;;;;;;


program	SEGMENT	PUBLIC

	ASSUME	CS:program, DS:program, SS:program

	ORG	100h

	PUBLIC	start

start:	MOV	argpnt+2, DS		;Store selector for argument data
	MOV	devpnt+2, DS		;And for device list
	MOV	shladr+2, DS
	MOV	prmadr+2, DS
	XOR	BX, BX
	MOV	BL, [BX+80h]		;Get length of argument data
	CMP	BL, 0			;Have any at all?
	JE	nocmd			;No
	CMP	BYTE PTR [BX+80h], CR	;Yes - does it end with CR?
	JNE	$002			;No
	DEC	BX			;Yes reduce by one character
	JE	nocmd
$002:	MOV	BYTE PTR [BX+81h], 0	;Make sure have null at end
	MOV	argsize, BX		;Store argument length
	MOV	CX, BX
	MOV	SI, 81h			;Point to start of command tail
	CLD
$004:	LODSB				;Find first non-blank character
	CMP	AL, ' '
	JE	$006
	CMP	AL, 9
	JNE	$008
$006:	LOOP	$004
	MOV	argsize, 0		;All blank!
	JMP	SHORT nocmd

;Here with first non-blank character in command tail

$008:	DEC	CX			;Have 2 or more characters left?
	JE	nocmd			;No
	CMP	AL, '/'			;Have /C next?
	JNE	nocmd			;No
	LODSB				;Maybe
	CMP	AL, 'c'
	JE	havecmd
	CMP	AL, 'C'
	JE	havecmd			;Yes

;Here if command tail does not begin with /C - just run the shell

	PUBLIC	nocmd

nocmd:	MOV	BX, 7Bh			;Store "SHELL" before the command tail
	MOV	argpnt+0, BX		;  since XOS expects the 0th argument
	MOV	[BX], 'HS'
	MOV	[BX+2], 'LE'
	MOV	[BX+4], ' L'
	ADD	argsize, 6
	CALL	dorun2			;Run the shell
	JMP	$028

;Here if command tail begins with /C

	PUBLIC	havecmd

havecmd:LODSB				;Scan to first non-blank character
	CMP	AL, ' '
	JE	havecmd
	CMP	AL, 9
	JE	havecmd
	DEC	SI
	MOV	argpnt+0, SI
	LEA	AX, [SI-81h]
	SUB	argsize, AX
	INC	prgmflg			;Indicate running program
	XOR	DX, DX			;Copy program name to our program
	MOV	DI, OFFSET progname	;  name buffer
	MOV	CX, 200
$012:	LODSB
	CMP	AL, 0
	JE	nameend
	CMP	AL, ' '
	JE	nameend
	CMP	AL, 9
	JE	nameend
	CMP	AL, ':'			;Have a device name or path?
	JE	$014
	CMP	AL, '\'
	JE	$014
	CMP	AL, '/'
	JNE	$016
$014:	MOV	DH, 1			;Yes
$016:	CMP	AL, '.'			;have an extension?
	JNE	$018
	MOV	DL, 1			;Yes
$018:	STOSB
	LOOP	$012
	JMP	SHORT nocmd		;Too long for our name buffer - let the
					;  shell handle it!
;Here at end of program name

	PUBLIC	nameend

nameend:MOV	endpnt, DI		;Remember where to put the extension
	MOV	AX, OFFSET progname	;Get offset of name buffer
	CMP	DH, 0			;Did he specify a device name?
	JNE	$022			;Yes
	SUB	AX, 4			;No - use CMD:
$022:	MOV	shladr+0, AX
	CMP	DL, 0			;Did he specify an extension?
	JE	$024			;No
	MOV	SI, OFFSET nullext	;Yes - use it
	JMP	SHORT $026

;Here if no extension given

$024:	MOV	SI, OFFSET runext
	CALL	dorun
	JNC	done
	CMP	AX, ER_FILNF
	JE	SHORT $024a
	CMP	AX, ER_DIRNF
	JNE	$030
$024a:	MOV	SI, OFFSET batext
	CALL	dorun
	JNC	done
	CMP	AX, ER_FILNF
	JE	SHORT $024b
	CMP	AX, ER_DIRNF
	JNE	$030
$024b:	MOV	SI, OFFSET exeext
	CALL	dorun
	JNC	done
	MOV	SI, OFFSET comext
$026:	CALL	dorun
$028:	JNC	done			;OK
$030:	PUSH	DX			;Error
	PUSH	AX
	MOV	AX, OFFSET errmsg1	;Display first part of message
	CMP	prgmflg, 0
	JE	$032
	MOV	AX, OFFSET errmsg2
$032:	CALL	wrstr
	PUSH	0			;Get message for error (error code
	PUSH	3			;  already on the stack)
	PUSH	0
	PUSH	0
	PUSH	DS
	PUSH	80h
	CALL	svcSysErrMsg
	MOV	AX, 80h			;Display message
	CALL	wrstr
	MOV	AX, OFFSET errmsg3	;Display final part of message
	CALL	wrstr

	PUBLIC	done

done:	MOV	AX, 4C00h
	INT	21h

;Subroutine to run a program
;	c[SI] = Offset of extension to add
;	CALL	dorun
;	C:set = Error
;	  c{DX,AX} = Error code
;	C:clr = Normal

	PUBLIC	dorun

dorun:	MOV	DI, endpnt		;Add the extension
	CLD
$040:	LODSB
	STOSB
	CMP	AL, 0
	JNE	$040
dorun2:	PUSH	0			;Address of run parameter block
	PUSH	0
	PUSH	DS
	PUSH	OFFSET runqab
	CALL	svcIoRun
	OR	DX, DX			;Error return?
	JS	$042			;Yes
	MOV	AX, runqab+qab_error+0	;Maybe
	MOV	DX, runqab+qab_error+2
	OR	DX, DX
	JNS	$044			;Not error
$042:	STC				;Error - set C
$044:	RET

;Subroutine to output string to terminal
;	c[AX] = Offset of string
;	CALL	wrstr

	PUBLIC	wrstr

wrstr:	PUSH	0			;Display error message
	PUSH	DH_STDTRM
	PUSH	DS
	PUSH	AX
	PUSH	0
	PUSH	0
	CALL	svcIoOutString
	RET

;Data

	PUBLIC	runqab

runqab	LABEL	WORD			;Run SVC parameter block

	DW	RFNC$WAIT OR RFNC_RUN	;qab_func
	DW	0			;qab_status
	DD	0			;qab_error
	DD	0			;qab_amount
	DD	0			;qab_handle
	DB	0, 0, 0, 0		;qab_vector
	DD	RH$CHILDTERM		;qab_option
	DD	0			;qab_count
shladr	DW	OFFSET shell, 0, 0, 0	;qab_buffer1
	DD	0, 0			;qab_buffer2
prmadr	DW	OFFSET runparms, 0, 0, 0 ;qab_parm

shell	DB	'XOSCMD:SHELL.RUN', 0	;Name of program to run
errmsg1	DB	CR, LF, '? COMMAND: Cannot run command shell', CR, LF
	DB	'           ', 0
errmsg2	DB	CR, LF, '? COMMAND: Cannot run program', CR, LF
	DB	'           ', 0
errmsg3	DB	CR, LF, 0

	PUBLIC	runparms

runparms LABEL	BYTE

	DB	PAR$SET OR REP_STR
	DB	0
	DW	IOPAR_RUNCMDTAIL
argpnt	DW	81h, 0, 0, 0
argsize	DW	0, 0

	DB	PAR$SET OR REP_STR
	DB	0
	DW	IOPAR_RUNDEVLIST
devpnt	DW	OFFSET devlist, 0, 0, 0
	DW	DEVSIZE, 0

	DB	0

	PUBLIC	devlist

devlist	LABEL	WORD			;Device list

	DD	DH_STDIN , DH_STDIN , O$IN, 0 ;Standard input device
	DD	DH_STDOUT, DH_STDOUT, O$IN OR O$OUT, 0;Standard output device
	DD	DH_STDERR, DH_STDERR, O$IN OR O$OUT, 0;Standard error device
	DD	DH_STDTRM, DH_STDTRM, O$IN OR O$OUT, 0;Controlling terminal

DEVSIZE = $ - devlist

	PUBLIC	endpnt

endpnt	DW	0

runext	DB	".RUN"
nullext	DB	0
batext	DB	".BAT", 0
exeext	DB	".EXE", 0
comext	DB	".COM", 0
prgmflg DB	0		;Set to 1 if running program instead of shell

	PUBLIC	progname

	 DB	"CMD:"		;Attach search list at front of name
progname DB	220 DUP (?)

program	ENDS

	END	start
