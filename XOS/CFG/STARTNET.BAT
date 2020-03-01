@BATOPT ECHO=OFF EXTEND=ON

rem **********************************************
rem  Network startup file for this network
rem  The first argument is the node name
rem  The second argument is the node number
rem  Edit this file to change the network part of
rem  the IP address.
rem **********************************************

rem This file must be edited as indicate below before use.

IF "%1" == "" GOTO NEEDNAME
IF "%2" == "" GOTO NEEDNUM

rem The following lines load the network LKEs.

LKELOAD NETCLS
LKELOAD SNAPCLS

rem Uncomment one of the following two lines depending on the type of
rem network card in use.

rem LKELOAD ENWDADRV
rem LKELOAD ENNEADRV

LKELOAD ARPCLS
LKELOAD IPSCLS
LKELOAD TCPCLS
LKELOAD UDPCLS
LKELOAD RCPCLS
LKELOAD XFPCLS

rem The following line adds the NET device which is the hardware level
rem device at the bottom of the network protocol stack.

rem Uncomment one of the following two lines depending on the type of
rem network card in use and edit the IOREG, INT, and MEM parameters as
rem necessary.

rem ADDUNIT NET: UNIT=0 TYPE=EWDA IOREG=0x340 INT=5 MEM=0xE8000
rem ADDUNIT NET: UNIT=0 TYPE=ENEA IOREG=0x300 INT=15

rem The following line adds the SNAP level device.

ADDUNIT SNAP: UNIT=0 NETDEV=NET0

rem Use the send line below if using 802.3 Ethernet, leave this alone if
rem using Bluebook Ethernet (usual case).

DEVCHAR SNAP0: SAP=0FF-FF
rem DEVCHAR SNAP0: SAP=0AA-AA

rem The following lines add the IP and ARP level devices (These are at the
rem same level in the protocol stack.

ADDUNIT ARP: UNIT=0 SNAPDEV=SNAP0
DEVCHAR ARP0: ETYPE=8-6
ADDUNIT IPS: UNIT=0 SNAPDEV=SNAP0
DEVCHAR IPS0: CHKSUMH=FULL

rem Edit the following line to change the network part of the IP address.

DEVCHAR IPS0: ETYPE=8-0 ARPDEV=ARP0 IPADDR=200.1.1.%2

rem Edit the following line to change the domain name.

DEVCHAR IPS0: DOMAIN=%1.USER.COM

rem Edit the following line to change the sub-net mask (probably will not
rem need to change this).

DEVCHAR IPS0: SUBMASK=0FF-FF-FF-00

rem The following add the transport layer protocol devices.

ADDUNIT TCP UNIT=0 IPSDEV=IPS0
DEVCHAR TCP0 IPPROT=6
ADDUNIT UDP UNIT=0 IPSDEV=IPS0
DEVCHAR UDP0 IPPROT=17
ADDUNIT RCP UNIT=0 UDPDEV=UDP0

rem The following three lines load and start the basic Telnet server.
rem Uncomment these lines if using the basic Telnet server.

rem Uncomment the following three lines to load and start the basic
rem Telnet server.

rem LKELOAD TLNACLS
rem ADDUNIT TLN UNIT=0 TCPDEV=TCP0
rem DEVCHAR TLN0 SESSION=YES

rem The following four lines load and start the enhanced Telnet server.
rem Comment out these lines if using the basic Telnet server.

LKELOAD PCNCLS
ADDUNIT PCN UNIT=1
DEVCHAR PCN1 SESSION=YES
SYMBIONT TLNSRV TLNDEV=TCP0

rem The following lines start the XFP file server.

ADDUNIT XFP UNIT=0 RCPDEV=RCP0
DEVCHAR/Q RCP0 CON60=XFPSRV^1^1
SYMBIONT XFPSRV
SERVER XFPSRV ADD INS=1

rem One of following lines starts the FTP server.  If you want to enable
rem user name and password checking, use the line with the PASSWORD option.
rem In this case you must also start the UDF server (see below).

rem SYMBIONT FTPSRV FTPDEV=TCP0
rem SYMBIONT FTPSRV PASSWORD FTPDEV=TCP0

rem The following lines start the UDF (user description file) server.  If you
rem use this, change the ADD value in the second SERVER commmand to be your
rem IP address. Also, you should start the FTP server with the PASSWORD
rem option, to enable user name and password checking.

rem SYMBIONT UDFSRV
rem SERVER /SYSLOG UDFSRV ADD INS=1 DEV=UDP0: PORT=3002 GROUP=USER
rem SERVER /SYSLOG UDFSRV MODIFY INS=1 MASK=255.255.255.255 ADD=200.1.1.2

RETURN

:NEEDNAME
ECHO Please specify the node name
RETURN

:NEEDNUM
ECHO Please specify the node number
RETURN
