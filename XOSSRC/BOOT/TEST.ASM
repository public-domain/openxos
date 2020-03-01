	TITLE	TEST

	PAGE	52, 132
	.286P
	INCLUDE	MSDOS.PAR

stack	SEGMENT	STACK
	DW	256 DUP (?)
stktop	LABEL	BYTE
stack	ENDS

code	SEGMENT	PUBLIC
code	ENDS

data	SEGMENT	PUBLIC

	PUBLIC	pspseg, drive, pdrive, buffer
pspseg   DW	0		;PSP segment
drive    DW	2
buffer   DB     512 DUP (?)	;Data buffer

data	ENDS

	SUBTTL	Start of TEST

	ASSUME	CS:code, DS:data, SS:stack

code	SEGMENT

	PUBLIC	pcyl, pdrive, vect13
pcyl     DW	0
pdrive   DB	0
vect13	 DD	0, 0

	PUBLIC	start
start:	MOV	AX, stack
	MOV	SS, AX
	MOV	SP, OFFSET stktop
	MOV	AX, data		;Initialize DS
	MOV	DS, AX
	MOV	pspseg, ES

	XOR	AX, AX
	MOV	ES, AX
	MOV	AX, ES:WORD PTR 13h*4
	MOV	CS:WORD PTR vect13, AX
	MOV	AX, ES:WORD PTR 13h*4+2
	MOV	CS:WORD PTR vect13+2, AX

	MOV	AX, OFFSET check
	MOV	ES:WORD PTR 13h*4, AX
	MOV	ES:WORD PTR 13h*4+2, CS

	MOV	BX, OFFSET buffer
	MOV	DL, 80h
	MOV	AX, drive
	MOV	DX, 0
	MOV	CX, 1
	INT	25h

	XOR	AX, AX
	MOV	ES, AX
	MOV	AX, CS:WORD PTR vect13+2
	MOV	ES:WORD PTR 13h*4+2, AX
	MOV	AX, CS:WORD PTR vect13
	MOV	ES:WORD PTR 13h*4, AX

	PUBLIC	done
done:	JMP	start

	PUBLIC	check
check:	MOV	CS:pdrive, DL
	MOV	CS:pcyl, CX
	JMP	CS:vect13

code	ENDS

	END	start
