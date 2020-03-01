	.MACRO	SRVDATA
	SRV	_srvCmdWord   , 4, srvCmdWord#
	SRV	_srvSetup1    , 4, srvSetup1#
	SRV	_srvSetup2    , 4, srvSetup2#
	SRV	_srvSetupCmd  , 4, srvSetupCmd#
	SRV	_srvFinishCmd , 4, srvFinishCmd#
	SRV	_srvMessage   , 4, srvMessage#

	SRV	srvCmdPnt     , 4, 0
	SRV	srvLogLevel   , 4, 0
	SRV	srvLogHandle  , 4, 0
	SRV	srvLogName    , 4, 0
	SRV	srvDebugLevel , 4, 0
	SRV	srvDebugStream, 4, 0
	SRV	srvUnitNum    , 4, 1
	SRV	srvMsgDst     , 4, 0
	SRV	srvSrvName    , 4, srvname#
	SRV	srvPrgName    , 4, prgname#
	SRV	srvReqName    , 64t
	SRV	srvIpmName    , 32t
	.ENDM
