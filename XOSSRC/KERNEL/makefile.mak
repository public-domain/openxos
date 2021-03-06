.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	WCC386 /d1 /nm=$(@DPN) $(@DPN)

XOSX.RUN:	DATA.OBJ ERROR.OBJ CLOCK.OBJ CMOS.OBJ SCHED1.OBJ \
		SCHED2.OBJ SPAWN.OBJ SIGNAL.OBJ ALARM.OBJ EVENT.OBJ \
		CRITICAL.OBJ MEMORY1.OBJ MEMORY2.OBJ MEMORY3.OBJ \
		MEMORY4.OBJ SVC.OBJ UTILITY.OBJ LOGNAME.OBJ ENVIRON.OBJ \
		SYSTEM.OBJ LKE.OBJ ERRMSG.OBJ WILDCARD.OBJ IOCS1.OBJ \
		IOCS2.OBJ IOCS3.OBJ IOCS4.OBJ RANDOM.OBJ DMA.OBJ REAL.OBJ \
		REALBIOS.OBJ REALDOS.OBJ DPMI.OBJ TRMCLS1.OBJ TRMCLS2.OBJ \
		TRMFNC.OBJ TRMGRAPH.OBJ TRMTBL.OBJ IPMCLS.OBJ MOUSE.OBJ \
		PATH.OBJ PIPECLS.OBJ NULLCLS.OBJ PROT.OBJ PROTDOS.OBJ USR.OBJ \
		USRIO.OBJ USRREAL.OBJ USRRN.OBJ USRRNRUN.OBJ USRRNDOS.OBJ \
		USRRNDEF.OBJ USRDOS1.OBJ USRDOS2.OBJ USRDOS3.OBJ USRDOS4.OBJ \
		USRBIOS.OBJ USRDPMI.OBJ COUNTRY.OBJ BUGLOG.OBJ PDADEF.OBJ \
		ONCEUSER.OBJ \
		ONCEPROT.OBJ ONCEDISK.OBJ ONCECON.OBJ EXPORT.OBJ ONCEREAL.OBJ
	XLINK DATA ERROR CLOCK CMOS SCHED1 SCHED2 SIGNAL SPAWN \
		ALARM EVENT CRITICAL MEMORY1 MEMORY2 MEMORY3 MEMORY4 SVC \
		UTILITY LOGNAME ENVIRON SYSTEM LKE ERRMSG WILDCARD \
		IOCS1 IOCS2 IOCS3 IOCS4 RANDOM DMA REAL REALBIOS REALDOS DPMI \
		TRMCLS1 TRMCLS2 TRMFNC TRMGRAPH TRMTBL IPMCLS MOUSE PATH \
		PIPECLS NULLCLS PROT PROTDOS USR USRIO USRREAL USRRN USRRNRUN \
		USRRNDOS USRRNDEF USRDOS1 USRDOS2 USRDOS3 USRDOS4 USRBIOS \
		USRDPMI COUNTRY BUGLOG PDADEF EXPORT \
		XOSLIB:\XOS\GECKOX ONCEUSER ONCEPROT ONCEDISK ONCECON ONCEREAL \
		/OUTPUT=XOSX /MAP=XOSX /SYM386 /ALONE
	COPY /OVER XOSX.RUN NEWSYS:XOSX.RUN

ALL:	XOS.RUN XOSX.RUN
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

XOSLCDX.RUN:	DATA.OBJ ERROR.OBJ CLOCK.OBJ CMOS.OBJ SCHED1.OBJ \
		SCHED2.OBJ SPAWN.OBJ SIGNAL.OBJ ALARM.OBJ EVENT.OBJ \
		CRITICAL.OBJ MEMORY1.OBJ MEMORY2.OBJ MEMORY3.OBJ \
		MEMORY4.OBJ SVC.OBJ UTILITY.OBJ LOGNAME.OBJ ENVIRON.OBJ \
		SYSTEM.OBJ LKE.OBJ ERRMSG.OBJ WILDCARD.OBJ IOCS1.OBJ \
		IOCS2.OBJ IOCS3.OBJ IOCS4.OBJ DMA.OBJ REAL.OBJ REALBIOS.OBJ \
		REALDOS.OBJ DPMI.OBJ TRMCLS1.OBJ TRMCLS2.OBJ TRMFNC.OBJ \
		TRMGRAPH.OBJ TRMTBL.OBJ IPMCLS.OBJ MOUSE.OBJ PATH.OBJ \
		PIPECLS.OBJ NULLCLS.OBJ PROT.OBJ PROTDOS.OBJ USR.OBJ \
		USRIO.OBJ USRREAL.OBJ USRRN.OBJ USRRNRUN.OBJ USRRNDOS.OBJ \
		USRRNDEF.OBJ USRDOS1.OBJ USRDOS2.OBJ USRDOS3.OBJ USRDOS4.OBJ \
		USRBIOS.OBJ USRDPMI.OBJ COUNTRY.OBJ BUGLOG.OBJ PDADEF.OBJ \
		ONCEUSER.OBJ \
		ONCEPROT.OBJ ONCEDISK.OBJ ONCELCD.OBJ EXPORT.OBJ ONCEREAL.OBJ
	XLINK DATA ERROR CLOCK CMOS SCHED1 SCHED2 SIGNAL SPAWN ALARM \
		EVENT CRITICAL MEMORY1 MEMORY2 MEMORY3 MEMORY4 SVC \
		UTILITY LOGNAME ENVIRON SYSTEM LKE ERRMSG WILDCARD \
		IOCS1 IOCS2 IOCS3 IOCS4 DMA REAL REALBIOS REALDOS DPMI \
		TRMCLS1 TRMCLS2 TRMFNC TRMGRAPH TRMTBL IPMCLS MOUSE PATH \
		PIPECLS NULLCLS PROT PROTDOS USR USRIO USRREAL USRRN USRRNRUN \
		USRRNDOS USRRNDEF USRDOS1 USRDOS2 USRDOS3 USRDOS4 USRBIOS \
		USRDPMI COUNTRY BUGLOG PDADEF EXPORT \
		XOSLIB:\XOS\GECKOX ONCEUSER ONCEPROT ONCEDISK ONCELCD ONCEREAL \
		/OUTPUT=XOSLCDX /MAP=XOSLCDX /SYM386 /ALONE
	COPY /OVER XOSLCDX.RUN NEWSYS:XOSLCDX.RUN

XOSB.RUN:	DATAB.OBJ ERROR.OBJ CLOCK.OBJ CMOS.OBJ SCHED1.OBJ \
		SCHED2.OBJ SPAWN.OBJ SIGNAL.OBJ ALARM.OBJ EVENT.OBJ \
		CRITICAL.OBJ MEMORY1.OBJ MEMORY2.OBJ MEMORY3.OBJ \
		MEMORY4.OBJ SVC.OBJ UTILITY.OBJ LOGNAME.OBJ ENVIRON.OBJ \
		SYSTEM.OBJ LKE.OBJ ERRMSG.OBJ WILDCARD.OBJ IOCS1.OBJ \
		IOCS2.OBJ IOCS3.OBJ IOCS4.OBJ DMA.OBJ REAL.OBJ REALBIOS.OBJ \
		REALDOS.OBJ DPMI.OBJ TRMCLS1.OBJ TRMCLS2.OBJ TRMFNC.OBJ \
		TRMGRAPH.OBJ TRMTBL.OBJ IPMCLS.OBJ MOUSE.OBJ PATH.OBJ \
		PIPECLS.OBJ NULLCLS.OBJ PROT.OBJ PROTDOS.OBJ USR.OBJ \
		USRIO.OBJ USRREAL.OBJ USRRN.OBJ USRRNRUN.OBJ USRRNDOS.OBJ \
		USRRNDEF.OBJ USRDOS1.OBJ USRDOS2.OBJ USRDOS3.OBJ USRDOS4.OBJ \
		USRBIOS.OBJ USRDPMI.OBJ COUNTRY.OBJ BUGLOG.OBJ PDADEF.OBJ \
		ONCEUSER.OBJ \
		ONCEPROT.OBJ ONCEDISK.OBJ ONCECON.OBJ EXPORT.OBJ ONCEREALB.OBJ
	XLINK DATAB ERROR CLOCK CMOS SCHED1 SCHED2 SIGNAL SPAWN ALARM \
		EVENT CRITICAL MEMORY1 MEMORY2 MEMORY3 MEMORY4 SVC \
		UTILITY LOGNAME ENVIRON SYSTEM LKE ERRMSG WILDCARD \
		IOCS1 IOCS2 IOCS3 IOCS4 DMA REAL REALBIOS REALDOS DPMI \
		TRMCLS1 TRMCLS2 TRMFNC TRMGRAPH TRMTBL IPMCLS MOUSE PATH \
		PIPECLS NULLCLS PROT PROTDOS USR USRIO USRREAL USRRN USRRNRUN \
		USRRNDOS USRRNDEF USRDOS1 USRDOS2 USRDOS3 USRDOS4 USRBIOS \
		USRDPMI COUNTRY BUGLOG PDADEF EXPORT \
		ONCEUSER ONCEPROT ONCEDISK ONCECON ONCEREALB \
		XOSLIB:\XOS\GECKOB \
		/OUTPUT=XOSB /MAP=XOSB /SYM386 /ALONE
	COPY /OVER XOSB.RUN NEWSYS:XOSB.RUN

XOS.RUN:	DATA.OBJ ERROR.OBJ CLOCK.OBJ CMOS.OBJ SCHED1.OBJ \
		SCHED2.OBJ SPAWN.OBJ SIGNAL.OBJ ALARM.OBJ EVENT.OBJ \
		CRITICAL.OBJ MEMORY1.OBJ MEMORY2.OBJ MEMORY3.OBJ \
		MEMORY4.OBJ SVC.OBJ UTILITY.OBJ LOGNAME.OBJ ENVIRON.OBJ \
		SYSTEM.OBJ LKE.OBJ ERRMSG.OBJ WILDCARD.OBJ IOCS1.OBJ \
		IOCS2.OBJ IOCS3.OBJ IOCS4.OBJ RANDOM.OBJ DMA.OBJ REAL.OBJ \
		REALBIOS.OBJ REALDOS.OBJ DPMI.OBJ TRMCLS1.OBJ TRMCLS2.OBJ \
		TRMFNC.OBJ TRMGRAPH.OBJ TRMTBL.OBJ IPMCLS.OBJ MOUSE.OBJ \
		PATH.OBJ PIPECLS.OBJ NULLCLS.OBJ PROT.OBJ PROTDOS.OBJ USR.OBJ \
		USRIO.OBJ USRREAL.OBJ USRRN.OBJ USRRNRUN.OBJ USRRNDOS.OBJ \
		USRRNDEF.OBJ USRDOS1.OBJ USRDOS2.OBJ USRDOS3.OBJ USRDOS4.OBJ \
		USRBIOS.OBJ USRDPMI.OBJ COUNTRY.OBJ BUGLOG.OBJ PDADEF.OBJ \
		ONCEUSER.OBJ \
		ONCEPROT.OBJ ONCEDISK.OBJ ONCECON.OBJ EXPORT.OBJ ONCEREAL.OBJ
	XLINK DATA ERROR CLOCK CMOS SCHED1 SCHED2 SIGNAL SPAWN ALARM \
		EVENT CRITICAL MEMORY1 MEMORY2 MEMORY3 MEMORY4 SVC \
		UTILITY LOGNAME ENVIRON SYSTEM LKE ERRMSG WILDCARD \
		IOCS1 IOCS2 IOCS3 IOCS4 RANDOM DMA REAL REALBIOS REALDOS DPMI \
		TRMCLS1 TRMCLS2 TRMFNC TRMGRAPH TRMTBL IPMCLS MOUSE PATH \
		PIPECLS NULLCLS PROT PROTDOS USR USRIO USRREAL USRRN USRRNRUN \
		USRRNDOS USRRNDEF USRDOS1 USRDOS2 USRDOS3 USRDOS4 USRBIOS \
		USRDPMI COUNTRY BUGLOG PDADEF EXPORT \
		ONCEUSER ONCEPROT ONCEDISK ONCECON ONCEREAL \
		/OUTPUT=XOS /MAP=XOS /ALONE
	COPY /OVER XOS.RUN NEWSYS:XOS.RUN

XOSLCD.RUN:	DATA.OBJ ERROR.OBJ CLOCK.OBJ CMOS.OBJ SCHED1.OBJ \
		SCHED2.OBJ SPAWN.OBJ SIGNAL.OBJ ALARM.OBJ EVENT.OBJ \
		CRITICAL.OBJ MEMORY1.OBJ MEMORY2.OBJ MEMORY3.OBJ \
		MEMORY4.OBJ SVC.OBJ UTILITY.OBJ LOGNAME.OBJ ENVIRON.OBJ \
		SYSTEM.OBJ LKE.OBJ ERRMSG.OBJ WILDCARD.OBJ IOCS1.OBJ \
		IOCS2.OBJ IOCS3.OBJ IOCS4.OBJ DMA.OBJ REAL.OBJ REALBIOS.OBJ \
		REALDOS.OBJ DPMI.OBJ TRMCLS1.OBJ TRMCLS2.OBJ TRMFNC.OBJ \
		TRMGRAPH.OBJ TRMTBL.OBJ IPMCLS.OBJ MOUSE.OBJ PATH.OBJ \
		PIPECLS.OBJ NULLCLS.OBJ PROT.OBJ PROTDOS.OBJ USR.OBJ \
		USRIO.OBJ USRREAL.OBJ USRRN.OBJ USRRNRUN.OBJ USRRNDOS.OBJ \
		USRRNDEF.OBJ USRDOS1.OBJ USRDOS2.OBJ USRDOS3.OBJ USRDOS4.OBJ \
		USRBIOS.OBJ USRDPMI.OBJ COUNTRY.OBJ BUGLOG PDADEF.OBJ \
		ONCEUSER.OBJ \
		ONCEPROT.OBJ ONCEDISK.OBJ ONCELCD.OBJ EXPORT.OBJ ONCEREAL.OBJ
	XLINK DATA ERROR CLOCK CMOS SCHED1 SCHED2 SIGNAL SPAWN ALARM \
		EVENT CRITICAL MEMORY1 MEMORY2 MEMORY3 MEMORY4 SVC \
		UTILITY LOGNAME ENVIRON SYSTEM LKE ERRMSG WILDCARD \
		IOCS1 IOCS2 IOCS3 IOCS4 DMA REAL REALBIOS REALDOS DPMI \
		TRMCLS1 TRMCLS2 TRMFNC TRMGRAPH TRMTBL IPMCLS MOUSE PATH \
		PIPECLS NULLCLS PROT PROTDOS USR USRIO USRREAL USRRN USRRNRUN \
		USRRNDOS USRRNDEF USRDOS1 USRDOS2 USRDOS3 USRDOS4 USRBIOS \
		USRDPMI COUNTRY BUGLOG PDADEF EXPORT \
		ONCEUSER ONCEPROT ONCEDISK ONCELCD ONCEREAL \
		/OUTPUT=XOSLCD /MAP=XOSLCD /ALONE
	COPY /OVER XOSLCD.RUN NEWSYS:XOSLCD.RUN

DATA.OBJ:	DATA.M86 SVCDEF.INC
DATAB.OBJ:	DATA.M86 XOSGECKO.M86 SVCDEF.INC
	XMAC	XOSGECKO DATA /O=DATAB.OBJ

ERROR.OBJ:	ERROR.M86
CLOCK.OBJ:	CLOCK.M86
CMOS.OBJ:	CMOS.M86
SCHED1.OBJ:	SCHED1.M86
SCHED2.OBJ:	SCHED2.M86
SPAWN.OBJ:	SPAWN.M86
SIGNAL.OBJ:	SIGNAL.M86
ALARM.OBJ:	ALARM.M86
EVENT.OBJ:	EVENT.M86
CRITICAL.OBJ:	CRITICAL.M86
MEMORY1.OBJ:	MEMORY1.M86
MEMORY2.OBJ:	MEMORY2.M86
MEMORY3.OBJ:	MEMORY3.M86
MEMORY4.OBJ:	MEMORY4.M86
SVC.OBJ:	SVC.M86 SVCDEF.INC
UTILITY.OBJ:	UTILITY.M86
LOGNAME.OBJ:	LOGNAME.M86
ENVIRON.OBJ:	ENVIRON.M86
SYSTEM.OBJ:	SYSTEM.M86
LKE.OBJ:	LKE.M86
ERRMSG.OBJ:	ERRMSG.M86 XOSINC:\XMAC\XOSERR.PAR
WILDCARD.OBJ:	WILDCARD.M86
IOCS1.OBJ:	IOCS1.M86
IOCS2.OBJ:	IOCS2.M86
IOCS3.OBJ:	IOCS3.M86
IOCS4.OBJ:	IOCS4.M86
RANDOM.OBJ:	RANDOM.M86
DMA.OBJ:	DMA.M86
USRRN.OBJ:	USRRN.M86
USRRNRUN.OBJ:	USRRNRUN.M86
USRRNDOS.OBJ:	USRRNDOS.M86
USRRNDEF.OBJ:	USRRNDEF.M86
REAL.OBJ:	REAL.M86
REALBIOS.OBJ:	REALBIOS.M86
REALDOS.OBJ:	REALDOS.M86
DPMI.OBJ:	DPMI.M86
COUNTRY.OBJ:	COUNTRY.M86
TRMCLS1.OBJ:	TRMCLS1.M86
TRMCLS2.OBJ:	TRMCLS2.M86
TRMFNC.OBJ:	TRMFNC.M86
TRMGRAPH.OBJ:	TRMGRAPH.M86
TRMTBL.OBJ:	TRMTBL.M86
IPMCLS.OBJ:	IPMCLS.M86
MOUSE.OBJ:	MOUSE.M86
PRNCLS.OBJ:	PRNCLS.M86
GPIBCLS.OBJ:	GPIBCLS.M86
PATH.OBJ:	PATH.M86
PIPECLS.OBJ:	PIPECLS.M86
NULLCLS.OBJ:	NULLCLS.M86
PROT.OBJ:	PROT.M86
PROTDOS.OBJ:	PROTDOS.M86 PROT.INC
USR.OBJ:	USR.M86 SVCDEF.INC
USRIO.OBJ:	USRIO.M86
USRREAL.OBJ:	USRREAL.M86 SVCDEF.INC
USRDOS1.OBJ:	USRDOS1.M86
USRDOS2.OBJ:	USRDOS2.M86
USRDOS3.OBJ:	USRDOS3.M86
USRDOS4.OBJ:	USRDOS4.M86
USRBIOS.OBJ:	USRBIOS.M86
USRDPMI.OBJ:	USRDPMI.M86 SVCDEF.INC PROT.INC
ONCEUSER.OBJ:	ONCEUSER.M86
ONCEPROT.OBJ:	ONCEPROT.M86
ONCEDISK.OBJ:	ONCEDISK.M86
ONCECON.OBJ:	ONCECON.M86
ONCELCD.OBJ:	ONCELCD.M86
EXPORT.OBJ:	EXPORT.M86 SVCDEF.INC
ONCEREAL.OBJ:	ONCEREAL.M86
ONCEREALB.OBJ:	ONCEREAL.M86 XOSGECKO.M86
	XMAC	XOSGECKO ONCEREAL /O=ONCEREALB.OBJ

BUGLOG.OBJ:	BUGLOG.M86
PDADEF.OBJ:	PDADEF.M86
