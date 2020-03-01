;Define macro to generate the common initial part of the protected mode INT
;  handlers.  These handlers must start with a label of the from pintvn, where
;  "n" is the INT number.  This routine is called with a CALL with a stack
;  frame set up as indicated below.  All user registers have been saved in
;  the stack frame.  Registers are set up as follows:
;	c{EAX} = Undefined
;	c{EBX} = Undefined
;	c{ECX} = Unchanged from user mode
;	c{EDX} = Unchanged from user mode
;	c{EDI} = Unchanged from user mode
;	c{ESI} = Unchanged from user mode
;	c{EBP} = Current frame pointer
;	c{ESP} = Current stack pointer
;	c[DS]  = GS_REAL32|03h
;	c[ES]  = Unchanged from user mode
;	c[FS]  = Unchanged from user mode
;	c[GS]  = Undefined
;	c[SS]  = Current stack selector
;	c[CS]  = Current code selector

	.MACRO	PROTINT v

;Default protected mode INT xx handler for 16-bit clients - Get here when
;  INT xx is executed in 16-bit protected user mode.  When get here, the
;  stack is set up as follows:
;	c[{ESP}+6] = First pushed argument
;	c[{ESP}+4] = FR
;	c[{ESP}+2] = CS
;	c[{ESP}+0] = IP

;Note that the code below uses 16-bit stack operations even though they are
;  slower (this code is in a 32-bit segment) to save space on the user stack.
;  This code uses a maximum of 16 bytes of user stack space (including the
;  6 bytes used by the INT to save the caller's address and FR value.

pint16v'v::
	MOVZWL	ESP, SP			;Make sure no junk in high half of ESP
	PUSHW	DS
	PUSHW	#GS_REAL32|03h.B
	POPW	DS
	CMPB	dda_dpmilckpsflg, #0	;Using locked stack now?
	JNE	4$			;Yes
	PUSHL	EAX			;No
	PUSHL	#08h.B			;Disable all signals here
	CALLF	svcSchSetLevel#
	PUSHW	GS
	PUSHW	SS
	POPW	GS
	PUSHW	BX
	MOVW	BX, SP
	ADDW	BX, #16t.B
	MOVB	dda_dpmilckpsflg, #1	;No - indicate using locked stack now
	LSSL	ESP, dda_dpmilckpESP
	PUSHL	GS			;First save his stack pointer on our
	PUSHW	#0.B			;  stack
	PUSHW	BX
	PUSHL	#0.B			;Save the vector number/item count
	PUSHW	#0A00h
	PUSHW	GS:-2t.B[BX]		;Save FR
	PUSHL	GS:-4t.B[BX]		;Save CS
	PUSHW	#0.B
	PUSHW	GS:-6t.B[BX]		;Save IP
	PUSHL	GS:-8t.B[BX]		;Save DS
	PUSHL	ES			;Save ES
	PUSHL	FS			;Save FS
	PUSHL	GS:-14t.B[BX]		;Save GS
	PUSHL	EAX			;Allow signals now
	CALLF	svcSchSetLevel#
	MOVL	EAX, GS:-12t.B[BX]	;Restore his EAX value
	PUSHAL				;Save general registers
	MOVL	EBP, ESP		;Initialize the frame pointer
	MOVW	BX, GS:-16t.B[BX]	;Save his real BX value
	MOVW	dff_EBX.B[EBP], BX
	JMP	pintv'v

;Here to continue using the same stack - in this case we convert the 16-bit
;  stack frame to a standard 32-bit stack frame

4$:	POPW	DS
	SUBL	ESP, #4t.B
	PUSHL	#0000200h
	PUSHW	12t.B[ESP]		;Copy the FR value
	PUSHW	#0.B
	PUSHW	14t.B[ESP]		;Copy the CS value
	PUSHW	#0.B
	PUSHW	16t.B[ESP]		;Copy the IP value
;;;;;	JMP	8$.S
	JMP	8$

;Default protected mode INT xx handler for 32-bit clients - Get here when
;  INT xx is executed in 32-bit protected user mode.  When get here, the
;  stack is set up as follows:
;	c{{EBP}+12} = First pushed argument
;	c{{ESP}+8}  = EFR
;	c{{ESP}+4}  = CS
;	c{{ESP}+0}  = EIP

;  This code uses a maximum of 28 bytes of user stack space (including the
;  12 bytes used by the INT to save the caller's address and EFR value.

pint32v'v::
	PUSHL	DS
	PUSHL	#GS_REAL32|03h.B
	POPL	DS
	CMPB	dda_dpmilckpsflg, #0	;Using locked stack?
	JNE	6$.S			;Yes
	PUSHL	EAX			;No
	PUSHL	#08h.B			;Disable all signals here
	CALLF	svcSchSetLevel#
	PUSHL	GS
	PUSHL	SS
	POPL	GS
	PUSHL	EBX
	MOVL	EBX, ESP
	ADDL	EBX, #28t.B
	MOVB	dda_dpmilckpsflg, #1	;Indicate using locked stack now
	LSSL	ESP, dda_dpmilckpESP
	PUSHL	GS			;First save his stack pointer on our
	PUSHL	EBX			;  stack
	PUSHL	#0.B
	PUSHL	GS:-4t.B[EBX]		;Save EFR
	PUSHL	GS:-8t.B[EBX]		;Save CS
	PUSHL	GS:-12t.B[EBX]		;Save EIP
	PUSHL	GS:-16t.B[EBX]		;Save DS
	PUSHL	ES			;Save ES
	PUSHL	FS			;Save FS
	PUSHL	GS:-24t.B[EBX]		;Save GS
	PUSHL	EAX			;Allow signals now
	CALLF	svcSchSetLevel#
	MOVL	EAX, GS:-20t.B[EBX]	;Restore his EAX value
	PUSHAL				;Save general registers
	MOVL	EBP, ESP		;Initialize the frame pointer
	MOVW	dff_EFR+2t.B[EBP], #0A00h
	MOVL	EBX, GS:-28t.B[EBX]	;Save his correct EBX value
	MOVL	dff_EBX.B[EBP], EBX
	JMP	pintv'v.S

;Here to continue using the same stack

6$:	POPL	DS
	PUSHL	8t.B[ESP]		;Move the EFR value
	PUSHL	8t.B[ESP]		;Move the CS value
	PUSHL	8t.B[ESP]		;Move the ESP value
8$:	MOVL	20t.B[ESP], SS		;Store his stack pointer
	MOVL	16t.B[ESP], ESP
	ADDL	16t.B[ESP], #24t.B
	MOVL	12t.B[ESP], #0
	PUSHL	DS			;Save DS
	PUSHL	ES			;Save ES
	PUSHL	FS			;Save FS
	PUSHL	GS			;Save GS
	PUSHAL				;Save general registers
	MOVL	EBP, ESP		;Initialize the frame pointer
	PUSHL	#GS_REAL32|03h.B
	POPL	DS
	JMP	pintv'v.S

	.ENDM
