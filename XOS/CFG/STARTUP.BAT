@ECHO OFF
BATOPT ERRP=ON

rem This file is executed first after XOS is loaded.

rem Note that this file will initialize a usuable system as supplied but
rem not all features will be enabled.  To enable additional features, edit
rem this file as indicated below.

rem Following line ensures that an initial session will be created on the
rem console.

SYSCHAR/Q INITIAL=YES

rem Following line sets the initial size of the disk cache.

CLSCHAR/Q DISK: NUMDBUF=2000 NUMSBUF=500

rem Following lines load drivers and add units for all standard disks, fixed
rem and removable.

DISKINST
FIXDRIVE/Q
RMVDRIVE/Q SYQLTR=K ZIPLTR=M LSLTR=P OTHLTR=T CDLTR=X

rem Following lines load the VGA graphics extensions and the mouse driver
rem only one of S3ADRV or ATIBDRV should be loaded.

rem LKELOAD S3ADRV
rem LKELOAD ATIBDRV
rem LKELOAD MOUSEACLS
rem ADDUNIT MOUSEA: UNIT=0 IOREG=0x2F8 INT=3
rem DEVCHAR/Q MOUSEA0: CONDEV=0

rem Following lines assign some logical names that are handy for some
rem programs.

LOGICAL/Q/SYS AUX:=_TRM1:
LOGICAL/Q/SYS COM1:=_TRM1:
LOGICAL/Q/SYS COM2:=_TRM2:
LOGICAL/Q/SYS PRN:=_SPL1:
LOGICAL/Q/SYS NET:=XFP0:
LOGICAL/Q/SYS NDG:=UDP0:

rem Following lines save a previous system crash (if any) to the system
rem log file.

CRSHSAVE

rem Following lines load the parallel port printer driver and adds units
rem for the standard ports that are present.

rem LKELOAD PPRCLS
rem DOSLPT

rem Following lines load the serial port driver and adds units (the ADDUNIT
rem lines should be uncommented as desired. If the mouse driver was loaded
rem there cannot be a terminal unit for the same port).

LKELOAD SERADRV

rem ADDUNIT TRM: UNIT=1 TYPE=SERA IOREG=0x3F8 INT=4
rem DEVCHAR/Q TRM1: IRATE=9600 IINFLOW=DTR IOUTFLOW=DSR SESSION=YES

rem ADDUNIT TRM: UNIT=2 TYPE=SERA IOREG=0x2F8 INT=3
rem DEVCHAR/Q TRM2: IRATE=60000 IINFLOW=DTR IOUTFLOW=DSR SESSION=NO

rem ADDUNIT TRM: UNIT=4 TYPE=SERA IOREG=0x2E8 INT=2
rem DEVCHAR/Q TRM4: IRATE=60000 IINFLOW=DTR IOUTFLOW=DSR SESSION=NO

rem Following lines load the screen symbiont and sets parameters for each
rem virtual screen.

SYMBIONT SCREEN 0:16

DEVCHAR/Q TRM0S1: IOUTFLOW=XON OUTFLOW=XON VERTFREQ=80 SESSION=YES
DEVCHAR/Q TRM0S2: IOUTFLOW=XON VERTFREQ=80 SESSION=YES
DEVCHAR/Q TRM0S3: IOUTFLOW=XON VERTFREQ=80 SESSION=YES
DEVCHAR/Q TRM0S4: IOUTFLOW=XON VERTFREQ=80 SESSION=YES
DEVCHAR/Q TRM0S5: IOUTFLOW=XON VERTFREQ=80 SESSION=YES
DEVCHAR/Q TRM0S6: IOUTFLOW=XON VERTFREQ=80 SESSION=YES
DEVCHAR/Q TRM0S7: IOUTFLOW=XON VERTFREQ=80 SESSION=YES
DEVCHAR/Q TRM0S8: IOUTFLOW=XON VERTFREQ=80 SESSION=YES
DEVCHAR/Q TRM0S9: IOUTFLOW=XON VERTFREQ=80 SESSION=YES
DEVCHAR/Q TRM0S10: IOUTFLOW=XON VERTFREQ=80 SESSION=YES
DEVCHAR/Q TRM0S11: IOUTFLOW=XON VERTFREQ=80 SESSION=YES
DEVCHAR/Q TRM0S12: IOUTFLOW=XON VERTFREQ=80 SESSION=YES
DEVCHAR/Q TRM0S13: IOUTFLOW=XON VERTFREQ=80 SESSION=YES
DEVCHAR/Q TRM0S14: IOUTFLOW=XON VERTFREQ=80 SESSION=YES
DEVCHAR/Q TRM0S15: IOUTFLOW=XON VERTFREQ=80 SESSION=YES
DEVCHAR/Q TRM0S16: IOUTFLOW=XON VERTFREQ=80 SESSION=YES

rem Following lines load and initialize the printer spooler.

rem LKELOAD SPLCLS
rem ADDUNIT SPL: UNIT=1
rem DEVCHAR/Q SPL1: SPLSPEC=E:\SPOOL\S01_#4.TMP CLSNAME=S01_#4.SPL CLSTIME=5
rem SYMBIONT UNSPOOL UNIT=1 OUTPUT=_TRM1: DIRECT=C:\SPOOL\

rem Following line defines the environment variable COMSPEC to point to the
rem dummy COMMAND.COM.  Some DOS programs need this.

SETENV/Q/SYS COMSPEC = C:\XOS\CMD\COMMAND.COM

rem Following lines define some additional logical names, which are mostly
rem used when building executables from the sources.

LOGICAL/Q/SYS XOSPRG:=XOSDIR:\PRG\
LOGICAL/Q/SYS XOSIMG:=XOSDIR:\IMG\
LOGICAL/Q/SYS XOSFNT:=XOSDIR:\FNT\
LOGICAL/Q/SYS XOSINC:=XOSDIR:\INC\
LOGICAL/Q/SYS XOSLIB:=XOSDIR:\LIB\
LOGICAL/Q/SYS NEWSYS:=XOSDIR:\SYS\
LOGICAL/Q/SYS NEWCMD:=XOSDIR:\CMD\

rem Follwoing line executes the user startup file USTARTUP.BAT.

CALL XOSCFG:\USTARTUP.BAT
