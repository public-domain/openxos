rem This file contains startup commands which are unique to this system

rem Following line defines the command search path

LOGICAL/Q/SYS CMD:="Z:,XOSPRG:,XOSCMD:"

rem Following lines define environment strings that are used by some common
rem DOS programs.

SETENV/Q/SYS BPATH=;F:\B31\MACROS
SETENV/Q/SYS BHELP=F:\B31\HELP
SETENV/Q/SYS BFILE=.\BRIEF.RST
SETENV/Q/SYS BBACKUP=.
SETENV/Q/SYS BFLAGS = "-zi180l255Mpu100 -mBRN -mrestore -Dega -D101key -DF:\B31\MACROS\EGA.DRV"
SETENV/Q/SYS BCC="""!cl -c %%s.c"""
SETENV/Q/SYS CBPATH=E:\CODEBLDR
SETENV/Q/SYS WCC386="/zq /3s /s /j /w3 /we /fp3 /i=P:\WCX\"
rem SETENV/Q/SYS WCGMEMORY="?"

rem This line defines the command shell prompt

SETENV/Q/SYS PROMPT = $e[3;0v$_$e[2K$P}

rem These environment strings specify defaults for various XOS commands

SETENV/Q/SYS ^XOS^DIR^OPT = "/SORT=DIR /PAUSE /WIDE"
SETENV/Q/SYS ^XOS^COPY^OPT = "/TOTALS=RATE /TOTALS=FILES /TOTALS=BYTES /VERBOSE"
SETENV/Q/SYS ^XOS^RENAME^OPT = "/VERBOSE"
SETENV/Q/SYS ^XOS^TYPE^OPT = "/NAMES /PAUSE"
rem SETENV/Q/SYS ^XOS^VID^OPT = "/LIN=60/TXTC=BWH/TXTB=CYA"
SETENV/Q/SYS ^XOS^VID^OPT = "/LIN=60"
SETENV/Q/SYS ^XOS^XMAC^OPT = "/COMPACT"
SETENV/Q/SYS ^XOS^GCP=DOSQUIRK=FALSE

rem This line disables the per-session memory limit.

CLSCHAR SESSION PMLIMIT=0

rem This line loads network support - Edit it to specify the correct node
rem name and node number, also edit STARTNET.BAT as indicated in the comments
rem in that file

rem CALL STARTNET nodename 99

rem This line sets the text color to green

COLOR FGC=GREEN
