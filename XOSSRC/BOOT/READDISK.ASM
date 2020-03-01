	PAGE	52, 132
	TITLE	readdisk - C callable function to read absolute disk block
	SUBTTL	copyright (c) 1989 Saguaro Software, Ltd/JRG

	INCLUDE	TCSEGS.PAR
	INCLUDE	MSDOS.PAR

	EXTRN	$errrtn:NEAR
	CODESEG	LIB

;Calling sequence:
;	int   disk;		/* Number for disk */
;	int   sector;		/* Starting sector number */
;	char *buffer;		/* Buffer */
;	int   blocks;		/* Number of blocks to read */
;	readdisk(disk, sector, buffer, blocks);
;  Value returned is 0 if normal or iNT 25h error code if error

	PROCBGN	readdisk
	PUSH	BP
	MOV	BP, SP
	PUSH	DS
	PUSH	SI
	PUSH	DI

;	MOV	AL, [BP+_AOS]		;Get disk number
;	MOV	DX, [BP+_AOS+2]
;	GETADS	BX, [BP+_AOS+4]		;Get buffer address
;	MOV	CX, [BP+_AOS+8]		;Get block count
;	INT	25h
;	POP	BX			;Remove junk from the stack

	MOV	DL, [BP+_AOS]
	OR	DL, 80h
	GETAES	BX, [BP+_AOS+4]
	MOV	AL, [BP+_AOS+8]
	MOV	AH, 2
	MOV	DH, 0
	MOV	CL, 1
	MOV	CH, 0
	NOP
	NOP
	INT	13h

	POP	DI
	POP	SI
	POP	DS
	POP	BP
	JC	SHORT rd2		;If error
	XOR	AX, AX			;OK - return 0
	RET

;Here if error indicated

rd2:	CBW				;Expand error code to 16 bits
	INC	AX			;Add 1
	RET

	PROCEND	readdisk
	CODEEND	LIB

	END
