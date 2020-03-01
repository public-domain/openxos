rem This file specifies setup for an Internet connection

rem This file is intended to be edited for each machine

BATOPT ERRP=ON

LKELOAD NETCLS
LKELOAD SNAPCLS

rem Load one of the following 3 low level network drivers

rem Driver for most WD and SMS ISA network cards
rem LKELOAD ENWDADRV

rem Driver for NE1000 and NE2000 and compatable ISA and PCI cards
rem LKELOAD ENNEADRV

rem Driver for RealTek 8139 based PCI cards
LKELOAD ENRTADRV

LKELOAD ARPCLS
LKELOAD IPSCLS
LKELOAD TCPCLS
LKELOAD UDPCLS
LKELOAD RCPCLS
LKELOAD XFPCLS

rem Following are sample lines for various types of network cards (For PCI
rem   cards use the SHOWPCI command to determine the bus and slot numbers)
rem ADDUNIT NET: UNIT=0 TYPE=EWDA IOREG=0x300 INT=15 MEM=0x0D0000
rem ADDUNIT NET: UNIT=0 TYPE=ENEA IOREG=0x300 BUS=0 INT=9
rem ADDUNIT NET: UNIT=0 TYPE=ENEA BUS= 0 SLOT=19
ADDUNIT NET: UNIT=0 TYPE=ERTA BUS=0 SLOT=9

rem Following sets up the IP protocol stack

ADDUNIT SNAP: UNIT=0 NETDEV=NET0

rem   Include one of the following to specify Bluebook (SAP=0FF-FF) or 
rem     802.3 (SAP=0AA-AA) Ethernet support

DEVCHAR SNAP0: SAP=0FF-FF
rem DEVCHAR/Q SNAP0: SAP=0AA-AA

ADDUNIT ARP: UNIT=0 SNAPDEV=SNAP0
DEVCHAR/Q ARP0: ETYPE=8-6

ADDUNIT IPS: UNIT=0 SNAPDEV=SNAP0
DEVCHAR/Q IPS0: CHKSUMH=FULL

DEVCHAR/Q IPS0: ETYPE=8-0 ARPDEV=ARP0
DEVCHAR/Q IPS0: IPADDR=10.3.0.1 SUBMASK=0FF-0-0-0
DEVCHAR/Q IPS0: DOMAIN=JRG2.ALLEGROSYS.COM 
DEVCHAR/Q IPS0: DRT1ADDR=10.0.0.1

ADDUNIT TCP: UNIT=0 IPSDEV=IPS0
DEVCHAR/q TCP0 IPPROT=6
ADDUNIT UDP UNIT=0 IPSDEV=IPS0
DEVCHAR/q UDP0 IPPROT=17
ADDUNIT RCP UNIT=0 UDPDEV=UDP0
DEVCHAR/Q RCP0 ACCESS=LANA

rem Following installs XOS remote file system support

ADDUNIT XFP UNIT=0 RCPDEV=RCP0
DEVCHAR/Q RCP0 CON60=XFPSRV^1^1
SYMBIONT XFPSRV
SERVER XFPSRV ADD INS=1 NUMCON=1000

rem Following installs the Telnet server - PASSWORD specifies a "system"
rem   password and/or requests user password verification - a trailing
rem   # means to verify the user password after verifying the system
rem   password (if one is specified)

LKELOAD PCNCLS
ADDUNIT PCN UNIT=1
DEVCHAR PCN1 SESSION=YES PASSWORD=abcd#
SYMBIONT TLNSRV TLNDEV=TCP0

rem Following installs the FTP server

SYMBIONT FTPSRV
SERVER FTPSRV ADD INS=1 DEV=TCP0

rem Following installs the XOS password/user information server

SYMBIONT UDFSRV
SERVER /SYSLOG /LOG=abc.log UDFSRV ADD INS=1 DEV=UDP0: PORT=3002 GROUP=USER
SERVER /SYSLOG /LOG=abc.log UDFSRV MODIFY INS=1 MASK=255.255.255.255 ADD=10.3.0.1
SERVER /SYSLOG /LOG=abc.log UDFSRV MODIFY INS=1 MASK=255.255.255.255 ADD=10.2.0.1
