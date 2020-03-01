	TITLE	RUN

	PAGE	52, 132
	INCLUDE	MSDOS.PAR

stack	SEGMENT	STACK
	DW	256 DUP (?)
stktop	LABEL	BYTE
stack	ENDS

code	SEGMENT	PUBLIC
code	ENDS

data	SEGMENT	PUBLIC

	PUBLIC	pspseg, execblk, xname
pspseg	DW	0
execblk	DW	0
	DW	80h, 0
	DW	5Ch, 0
	DW	6ch, 0
	DW	0, 0, 0, 0

xname	DB	"LOAD.EXE", 0

data	ENDS

	SUBTTL	Start of RUN

	ASSUME	CS:code, DS:data, SS:stack

code	SEGMENT

	PUBLIC	start
start:	MOV	AX, stack
	MOV	SS, AX
	MOV	SP, OFFSET stktop
	MOV	AX, data		;Initialize DS
	MOV	DS, AX
	MOV	pspseg, ES

	MOV	BX, 1000h
	MOV	AX, 4A00h
	INT	21h

	MOV	ES, pspseg
	MOV	AX, ES:WORD PTR 2Ch
	MOV	execblk+0, AX
	MOV	execblk+4, ES
	MOV	execblk+8, ES
	MOV	execblk+12, ES

	MOV	BX, OFFSET execblk
	MOV	AX, SEG execblk
	MOV	ES, AX
	MOV	DX, OFFSET xname
	MOV	AX, 4B01h
	INT	21h
	NOP

	MOV	AH, 51h
	INT	21h
	NOP

	PUBLIC	done
done:	JMP	done

code	ENDS

	END	start
