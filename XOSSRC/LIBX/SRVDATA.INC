	.MACRO	SRVDATA
	SRV	srvCmdWord  ,   4, cmdword#
	SRV	srvSetup1   ,   4, setup1#
	SRV	srvSetup2   ,   4, setup2#
	SRV	srvSetupCmd ,   4, setupcmd#
	SRV	srvFinishCmd,   4, finishcmd#
	SRV	srvMessage  ,   4, message#

	SRV	srvCmdPnt   ,   4, 0
	SRV	srvLogLevel ,   4, 0
	SRV	srvLogHandle,   4, 0
	SRV	srvLogName,     4, 0
	SRV	srvDebugLevel , 4, 0
	SRV	srvDebugStream, 4, 0

	SRV	srvVersion  ,   4, version#
	SRV	srvEditNum  ,   4, editnum#
	SRV	srvUnitNum  ,   4, 1
	SRV	srvSrvName  ,   4, srvname#
	SRV	srvPrgName  ,   4, prgname#
	SRV	srvReqName  ,   64t
	.ENDM
