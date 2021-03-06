# Makefile for XOS SHELL/BATCH

.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	WCC386 /D1 /I=XOSINC:\WCX\ /ZQ /3S /J /W3 /WE /FP3 /S \
		/NM=$(@DPN) $(@DPN)

ALL:	SHELL.RUN
	@ECHO Finished.

CLEAN:
	-DELETE *.BAK
	-DELETE *.DMP
	-DELETE *.ERR
	-DELETE *.EXE
	-DELETE *.LKE
	-DELETE *.LST
	-DELETE *.MAP
	-DELETE *.OBJ
	-DELETE *.RST
	-DELETE *.RUN
	-DELETE *.SYM
	-DELETE *.TMP

SHELL.RUN:	SHELL.OBJ CMDDATA.OBJ GETCMD.OBJ CHILD.OBJ HUNGUP.OBJ \
		CMDRESET.OBJ CMDCD.OBJ CMDEXIT.OBJ CMDLGOUT.OBJ CMDHIST.OBJ \
		CMDHSTSR.OBJ CMDCLS.OBJ MEMBLK.OBJ INICMD.OBJ UTILITY.OBJ \
                CMDALIAS.OBJ CMDALSSR.OBJ CMDVER.OBJ BATARGS.OBJ \
		BATXLATE.OBJ BATBAT.OBJ BATCMD.OBJ  BATCALL.OBJ BATECHO.OBJ \
		BATFOR.OBJ BATGOTO.OBJ BATIF.OBJ BATLABEL.OBJ BATNKEYW.OBJ \
		BATPAUSE.OBJ BATREM.OBJ BATRETUR.OBJ BATSHIFT.OBJ \
		BATJUMP.OBJ BATOPT.OBJ
	XLINK @SHELL

BATARGS.OBJ:	BATARGS.C SHELL.H SHELLEXT.H BATCHEXT.H

BATBAT.OBJ:	BATBAT.C SHELL.H SHELLEXT.H BATCHEXT.H

BATCALL.OBJ:	BATCALL.C SHELL.H SHELLEXT.H BATCHEXT.H

BATCH.OBJ:	BATCH.C SHELL.H SHELLEXT.H BATCHEXT.H

BATCMD.OBJ:	BATCMD.C SHELL.H SHELLEXT.H BATCHEXT.H

BATECHO.OBJ:	BATECHO.C SHELL.H SHELLEXT.H BATCHEXT.H

BATFOR.OBJ:	BATFOR.C SHELL.H SHELLEXT.H BATCHEXT.H

BATGOTO.OBJ:	BATGOTO.C SHELL.H SHELLEXT.H BATCHEXT.H

BATIF.OBJ:	BATIF.C SHELL.H SHELLEXT.H BATCHEXT.H

BATJUMP.OBJ:	BATJUMP.C SHELL.H SHELLEXT.H BATCHEXT.H

BATLABEL.OBJ:	BATLABEL.C SHELL.H SHELLEXT.H BATCHEXT.H

BATNKEYW.OBJ:	BATNKEYW.C SHELL.H SHELLEXT.H BATCHEXT.H

BATOPT.OBJ:	BATOPT.C SHELL.H SHELLEXT.H BATCHEXT.H

BATPAUSE.OBJ:	BATPAUSE.C SHELL.H SHELLEXT.H BATCHEXT.H

BATREM.OBJ:	BATREM.C SHELL.H SHELLEXT.H BATCHEXT.H

BATRETUR.OBJ:	BATRETUR.C SHELL.H SHELLEXT.H BATCHEXT.H

BATSHIFT.OBJ:	BATSHIFT.C SHELL.H SHELLEXT.H BATCHEXT.H

BATXLATE.OBJ:	BATXLATE.C SHELL.H SHELLEXT.H BATCHEXT.H

CHILD.OBJ:	CHILD.C SHELL.H SHELLEXT.H SHELLJMP.H BATCHEXT.H

CMDALIAS.OBJ:   CMDALIAS.C SHELL.H SHELLEXT.H BATCHEXT.H

CMDALSSR.OBJ:	CMDALSSR.C SHELL.H SHELLEXT.H

CMDCD.OBJ:	CMDCD.C SHELL.H SHELLEXT.H

CMDCLS.OBJ:	CMDCLS.C SHELL.H SHELLEXT.H

CMDDATA.OBJ:	CMDDATA.C SHELL.H SHELLEXT.H

CMDEXIT.OBJ:	CMDEXIT.C SHELL.H SHELLEXT.H

CMDHIST.OBJ:	CMDHIST.C SHELL.H SHELLEXT.H

CMDHSTSR.OBJ:	CMDHSTSR.C SHELL.H SHELLEXT.H

CMDLGOUT.OBJ:	CMDLGOUT.C SHELL.H SHELLEXT.H

CMDRESET.OBJ:	CMDRESET.C SHELL.H SHELLEXT.H

CMDVER.OBJ:     CMDVER.C SHELL.H SHELLEXT.H

GETCMD.OBJ:	GETCMD.C SHELL.H SHELLEXT.H KEYS.H

HUNGUP.OBJ:	HUNGUP.C SHELL.H SHELLEXT.H

INICMD.OBJ:	INICMD.C SHELL.H SHELLEXT.H BATCHEXT.H

MEMBLK.OBJ:	MEMBLK.C SHELL.H SHELLEXT.H

SHELL.OBJ:	SHELL.C SHELL.H SHELLEXT.H SHELLJMP.H BATCHEXT.H

UTILITY.OBJ:	UTILITY.C SHELL.H SHELLEXT.H

