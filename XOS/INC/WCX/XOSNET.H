//------------------------------------------------------------------------------
//
//  XOSNET.H - Network definitions for XOS network support routines
//
//  Edit History:
//  -------------
//  09/07/92(brn) - Add comment Header
//
//------------------------------------------------------------------------------

// ++++
// This software is in the public domain.  It may be freely copied and used
// for whatever purpose you see fit, including commerical uses.  Anyone
// modifying this software may claim ownership of the modifications, but not
// the complete derived code.  It would be appreciated if the authors were
// told what this software is being used for, but this is not a requirement.

//   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

#ifndef _XOSNET_H_
#define _XOSNET_H_

# include <_prolog.h>

// Define bits for the value of the IOPAR_NETCMODE and IOPAR_NETSMODE parameters

#define NMODE_IDLEMARK	0x00004000	// Send mark when idle 
#define NMODE_SENDSYNC	0x00002000	// Send sync pattern before data packets
#define NMODE_BRDCST  	0x00000800	// Broadcast next packet
#define NMTCP_PUSH    	0x00000020	// Do push after TCP output
#define NMTCP_CONP    	0x00000010	// Push is conditional (Nagel algorithm)

// Define device class specific IO parameters for IPS devices

#define IOPAR_IPS_RTUSE  0x8000+1	// Number of routing table entries
#define IOPAR_IPS_RTDATA 0x8000+2	// Routing table data

// Define special device function for IPS devices

#define IPSSF_FINDIPA   1		// Map domain name to IP addresses
#define IPSSF_FINDCNAME 2		// Map domain name to canonical name
#define IPSSF_FINDMAIL  3		// Map domain name to mail names
#define IPSSF_FINDPTR   4		// Map domain name to pointer
#define IPSSF_DUMP      7		// Dump DNS cache contents
#define IPSSF_INITCACHE 8		// Initialize DNS cache
#define IPSSF_CLRCACHE  9		// Clear DNS cache
#define IPSSF_ADDIPA    10		// Add domain name entry to DNS cache
#define IPSSF_ADDCNAME  11		// Add canonical name entry to DNS cache
#define IPSSF_ADDMAIL   12		// Add mail name entry to DNS cache
#define IPSSF_ADDPTR    13		// Add pointer entry to DNS cache
#define IPSSF_WAKEIPA   14		// Wake up waiters for IPA addresses
#define IPSSF_ERROR     15		// Set error for domain name
#define IPSSF_REMOVE    16		// Remove IP address entry from DNS
					//   cache
// Define 802.2 SAP values

#define SAP_NULL   	0x000		// Null
#define SAP_ILLCMAN	0x002		// LLC sublayer management (individual)
#define SAP_GLLCMAN	0x003		// LLC sublayer management (group)
#define SAP_ISNAPC 	0x004		// SNA path control (individual)
#define SAP_GSNAPC 	0x005		// SNA path control (group)
#define SAP_IP     	0x006		// Internet Protocol (obsolete)
#define SAP_3CXNS  	0x080		// 3Com XNS
#define SAP_SNAP   	0x0AA		// SNAP
#define SAP_NETWARE	0x0E0		// Novell NetWare
#define SAP_NETBIOS	0x0F0		// IBM NetBIOS
#define SAP_ILANMAN	0x0F4		// LAN management (individual)
#define SAP_GLANMAN	0x0F5		// LAN management (group)
#define SAP_RPL    	0x0F8		// IBM Remote Program Load
#define SAP_ISONET 	0x0FE		// ISO network layer
#define SAP_GLOBAL 	0x0FF		// Global

// Define internal protocol IDs - these values are used to request standard
//   protocols and are mapped to the corresponding physical protocol IDs at
//   the interface driver level

#define PROT_SSL1 	0x80000001	// SSL 1st Ethernet protocol 81-BA
#define PROT_SSL2 	0x80000002	// SSL 2nd Ethernet protocol 81-BB
#define PROT_SSL3 	0x80000003	// SSL 3rd Ethernet protocol 81-BC
#define PROT_SSL4 	0x80000004	// SSL 4th Ethernet protocol 81-BD
#define PROT_SSL5 	0x80000005	// SSL 5th Ethernet protocol 81-BE
#define PROT_SSL6 	0x80000006	// SSL 6th Ethernet protocol 81-BF
#define PROT_SSL7 	0x80000007	// SSL 7th Ethernet protocol 81-C0
#define PROT_SSL8 	0x80000008	// SSL 8th Ethernet protocol 81-C1
#define PROT_IP   	0x80000010	// Internet IP protocol
#define PROT_ARP  	0x80000011	// Internet ARP protocol
#define PROT_RARP 	0x80000012	// Internet RARP protocol

// Define special IP addresses

#define IPA_DOMAIN	0x01000000	// 0.0.0.1   - Domain name server
#define IPA_LOCAL 	0x0000007F	// 127.0.0.0 - Local address

// Define values for TCP public ports

#define TCPP_ECHO   	7	// Echo server (test)
#define TCPP_DISCARD	9	// Data sink (test)
#define TCPP_CHARGEN	19	// Character generator server (test)
#define TCPP_FTPDATA	20	// FTP data connection
#define TCPP_FTPCMD 	21	// FTP control connection
#define TCPP_TELNET 	23	// Telnet
#define TCPP_SMTP   	25	// Simple Mail Transport Protocol server
#define TCPP_TIME   	37	// Time server
#define TCPP_RLP    	39	// Resource Location Protocol server
#define TCPP_DOMAIN 	53	// Domain Name server
#define TCPP_TNP    	59	// TAMNet Protocol server
#define TCPP_XFP    	60	// XOS File Protocol server
#define TCPP_FINGER 	79	// Finger Protocol server
#define TCPP_NTP    	123	// Network Time Protocol server

// Define values for UDP public ports

#define UDPP_ECHO   	7	// Echo server (test)
#define UDPP_DISCARD	9	// Data sink (test)
#define UDPP_CHARGEN	19	// Character generater server (test)
#define UDPP_TIME   	37	// Time server
#define UDPP_RLP    	39	// Resource Location Protocol server
#define UDPP_NICNAME	43	// Who Is server
#define UDPP_DOMAIN 	53	// Domain Name server
#define UDPP_AFP    	60	// XOS File Protocol server
#define UDPP_BOOTPS 	67	// Bootstrap Protocol server
#define UDPP_BOOTPC 	68	// Bootstrap Protocol client
#define UDPP_TFTP   	69	// Trivial File Transfer Protocol server
#define UDPP_FINGER 	79	// Finger Protocol server
#define UDPP_NTP    	125	// Network Time Protocol server
#define UDPP_XECHO  	4376 // Extended echo protocol

// Define offsets in the ARP packets

struct arp_pkt
{
    ushort arp_hwtype;		    // Hardware type
    ushort arp_prot;		    // Target protocol
    uchar arp_hlen;		        // Hardware address length
    uchar arp_plen;	    	    // Protocol address length
    ushort arp_oper;		    // Operation
    uchar arp_senderha[6];	    // Sender hardware address
    uchar arp_ensenderpa[4];	// Sender protocol address (Ethernet only)
    uchar arp_entargetha[6];	// Target hardware address (Ethernet only)
    uchar arp_entargetpa[4];	// Target protocol address (Ethernet only)
};

#define arp_SIZE      (sizeof(struct arp_pkt))

// Define offsets in the IP datagarm packets

struct ip_pkt
{
    uchar ip_vershlen;	    // Version (always 4) and header length
    uchar ip_service;	    // Service level
    ushort ip_tlength;  	// Total length
    ushort ip_ident;    	// Packet identification
    ushort ip_fragment; 	// Fragmentation data
    uchar ip_time;          // Time to live
    uchar ip_ipprot;    	// IP protocol
    ushort ip_hdrchksum;   	// Header checksum
    uchar ip_srcipaddr[4];  // IP source address
    uchar ip_dstipaddr[4];  // IP destination address
    uchar ip_options;   	// (start of)Options
};

// Define values for the ip_ipprot byte

#define IPP_ICMP	1		// Internet Control Message Protocol
#define IPP_TCP 	6		// Transmission Control Protocol
#define IPP_UDP 	17		// User Datagram Protocol
#define IPP_XDP 	18		// eXtended Datagram Protocol
#define IPP_IARP	99		// Imbedded Address Resolution Protocol

// Define offsets in the ICMP packets - offsets are relative to the beginning
//   of the ICMP part of the packet

struct icmp_pkt
{
    uchar icmp_type;        // Type
    uchar icmp_code;        // Code
    ushort icmp_checksum;   // Checksum
    ushort icmp_id;         // Identifier value
    ushort icmp_seqnum;     // Sequence number
    uchar icmp_data;        // First data byte
};

#define icmp_pointer    icmp_id	// Pointer to bad byte (parameter problem message)
#define icmp_gwipaddr   icmp_id // Gateway IP address (redirect message)

// Define values for icmp_type

#define ICMPT_ECHOREPLY	0	// Echo reply
#define ICMPT_DSTUNR   	3	// Destination unreachable
#define ICMPT_SRCQUENCH	4	// Source quench
#define ICMPT_REDIRECT 	5	// Redirect
#define ICMPT_ECHOREQ  	8	// Echo request
#define ICMPT_TIMEEXCD 	11	// Time exceeded for a datagram
#define ICMPT_PARMPROB 	12	// Parameter problem with a datagram
#define ICMPT_TSREQ    	13	// Timestamp request
#define ICMPT_TSREPLY  	14	// Timestamp reply
#define ICMPT_INFOREQ  	15	// Information request (obsolete)
#define ICMPT_INFOREPLY	16	// Information reply (obsolete)
#define ICMPT_AMREQ    	17	// Address mask request
#define ICMPT_AMREPLY  	18	// Address mask reply
#define ICMPT_MAX      	18

// Define values for icmp_code

#define ICMPC_NETUNR   	0	// Network unreachable
#define ICMPC_HOSTUNR  	1	// Host unreachable
#define ICMPC_PROTUNR  	2	// Protocol unreachable
#define ICMPC_PORTUNR  	3	// Port unreachable
#define ICMPC_CANTFRAG 	4	// Fragmentation needed and DF set
#define ICMPC_SRFAIL   	5	// Source routine failed
#define ICMPC_DSTNUNKN 	6	// Destination network unknown
#define ICMPC_DSTHUNKN 	7	// Destination host unknown
#define ICMPC_SRCHISOL 	8	// Source host isolated
#define ICMPC_NETNALW  	9	// Communication with destination network administratively
							//   prohibited
#define ICMPC_HOSTNALW 	10	// Communication with destination host administratively
							//   prohibited
#define ICMPC_NETUNRTS 	11	// Network unreachable for type of service
#define ICMPC_HOSTUNRTS	12	// Host unreachable for type of service

// Define offsets in UDP packets - offsets are relative to the beginning of the
//   UDP part of the packet (necessary since the IP header is variable length)

struct udp_pkt
{
    ushort udp_srcport; 	// Source port number
    ushort udp_dstport; 	// Destination port number
    ushort udp_length;  	// Length
    ushort udp_checksum; 	// Checksum
    uchar udp_data;     	// Start of UPD data
};

// Define offsets in TCP packets - offsets are relative to the beginning of the
//   TCP part of the packet (necessary since the IP header is variable length)

struct tcp_pkt
{
    ushort tcp_srcport;     // Source port number
    ushort tcp_dstport; 	// Destination port number
    ulong tcp_seqnum;   	// Sequence number
    ulong tcp_acknum;   	// Acknowledgement number
    uchar tcp_offset;   	// Offset of start of data (high 4 bits, low 4 unused)
    uchar tcp_code;     	// Code bits
    ushort tcp_window;  	// Window amount
    ushort tcp_checksum;    // Checksum
    ushort tcp_urgent;  	// Urgent data pointer
    uchar tcp_options;  	// (start of) Options
};

// Define bits in the tcp_code byte

#define TCPC_URG	0x20	// Urgent pointer field is significant
#define TCPC_ACK	0x10	// Acknowledgement number field is significant
#define TCPC_PSH	0x08	// Push function
#define TCPC_RST	0x04	// Reset connection
#define TCPC_SYN	0x02	// Synchronize sequence number (establish connection)
#define TCPC_FIN	0x01	// Terminate connection

// Define offsets in RCP packets - offsets are relative to the beginning of the
//   RCP part of the UDP packet

struct rcp_pkt
{
    ushort rcp_srcport;     // Source port number (UDP header)
    ushort rcp_dstport;     // Destination port number (UDP header)
    ushort rcp_length;      // Length (UDP header)
    ushort rcp_checksum;    // Checksum (UDP header)
    uchar rcp_type;         // Packet type
    uchar rcp_rsrvd1;       // Reserved
    ushort rcp_window;      // Flow control window value
    ulong rcp_acknum;       // Acknowledgment number
    ulong rcp_seqnum;       // Send sequence number
    uchar rcp_data;         // Start of data area
};

// Define RCP packet type values

#define RCPP_CONREQ	1		// Connection request
#define RCPP_CONACK	2		// COnnection acknowledgment
#define RCPP_CONREJ	3		// Connection reject
#define RCPP_CONBSY	4		// Connection already exists
#define RCPP_INT   	8		// Interrupt data
#define RCPP_DATA  	9		// User data
#define RCPP_SUPER 	10		// Supervisory data
#define RCPP_WINPRB	11		// Window probe
#define RCPP_WINACK	12		// Window probe acknowledgment
#define RCPP_KILREQ	16		// KIll request
#define RCPP_KILACK	17		// Kill acknowlegment
#define RCPP_RSTREQ	18		// Reset connection request
#define RCPP_RSTACK	19		// Reset connection acknowledgment
#define RCPP_RSTREJ	20		// Reset connection reject

// Define RCP kill reason values

#define RCPKR_REQ	1		// Requested

// Define RCP supervisory data packet function values

#define RCPSF_KILREQ	1	// Kill request
#define RCPSF_KILRSP	2	// Kill response
#define RCPSF_MK1REQ	3	// Type 1 marker request
#define RCPSF_MR1RSP	4	// Type 1 marker response
#define RCPSF_MK2REQ	5	// Type 2 marker request
#define RCPSF_MR2RSP	6	// Type 2 marker response

// Define offsets in the Domain Name Server message header (relative to start of
//   the UDP packet data area)

struct dname_hdr
{
    ushort dns_idval;       // ID value
    ushort dns_parm;        // Parameter value
    ushort dns_numques;     // Number of questions
    ushort dns_numans;      // Number of answers
    ushort dns_numauth;     // Number of authority
    ushort dns_numadd;      // Number of additional
    uchar dns_ques;         // Start of question section
};

// Define offsets in BOOTP packets (relative to start of the UDP packet data area)

struct bootp_pkt
{
    uchar bootp_op;             // Operation
    uchar bootp_htype;          // Hardware type (same as for ARP and RARP)
    uchar bootp_hlen;           // Hardware address length
    uchar bootp_hops;           // Number of hops to final server
    ulong bootp_id;             // Transaction ID
    ushort bootp_seconds;       // Seconds since boot started
    ushort dummy1;
    ulong bootp_cipaddr;        // Client IP address
    ulong bootp_yipaddr;        // Your IP address
    ulong bootp_sipaddr;        // Server IP address
    ulong bootp_gipaddr;        // Gateway IP address
    uchar bootp_chwaddr[16]; 	// Client hardware address
    uchar bootp_hostname[64];	// Server host name
    uchar bootp_filename[128];	// Boot file name
    uchar bootp_vendor[64];		// Ventor specific area
};

// Define Telnet command values

#define TNC_IAC 	0x0FF		// Command prefix value

#define TNC_SE  	0x0F0		// End of subnegotiation
#define TNC_NOP 	0x0F1		// No operation
#define TNC_DM  	0x0F2		// Data mark
#define TNC_BRK 	0x0F3		// Out-of-band break character
#define TNC_IP  	0x0F4		// Interrupt process function
#define TNC_AO  	0x0F5		// Abort output function
#define TNC_AYT 	0x0F6		// Are you there function
#define TNC_EC  	0x0F7		// Erase character function
#define TNC_EL  	0x0F8		// Erase line function
#define TNC_GA  	0x0F9		// Go ahead signal
#define TNC_SB  	0x0FA		// Start of subnegotiation
#define TNC_WILL	0x0FB		// Start of option negotiation
#define TNC_WONT	0x0FC		// Start of option negotiation
#define TNC_DO  	0x0FD		// Start of option negotiation
#define TNC_DONT	0x0FE		// Start of option negotiation
#define TNC_FF  	0x0FF		// Literal 0FFh value

// Define Telnet option values

#define TNO_BINARY 	0x00	// Binary transmission
#define TNO_ECHO   	0x01	// Echo
#define TNO_RCP    	0x02	// Reconnection
#define TNO_SUPGA  	0x03	// Suppress go ahead
#define TNO_NAMS   	0x04	// Approx message size negotiation
#define TNO_STATUS 	0x05	// Status
#define   STS_IS     	0x00	//   Status is ...
#define   STS_SEND   	0x01	//   Request to send status
#define TNO_MARK   	0x06	// Timing mark
#define TNO_RCTE   	0x07	// Remote controlled trans and echo
#define TNO_OLW    	0x08	// Output line width
#define TNO_OPS    	0x09	// Output page size
#define TNO_OCRD   	0x0A	// Output carriage-return disposition
#define TNO_OHTS   	0x0B	// Output horizontal tab stops
#define TNO_OHTD   	0x0C	// Output horizontal tab disposition
#define TNO_OFFD   	0x0D	// Output formfeed disposition
#define TNO_OVTS   	0x0E	// Output vertical tab stops
#define TNO_OVTD   	0x0F	// Output vertical tab disposition
#define TNO_OLFD   	0x10	// Output linefeed disposition
#define TNO_XASCII 	0x11	// Extended ASCI
#define TNO_LOGOUT 	0x12	// Logout
#define TNO_BM     	0x13	// Byte macro
#define TNO_DET    	0x14	// Data entry terminal
#define TNO_SUPDUP 	0x16	// SUPDUP
#define TNO_SDOUT  	0x17	// SUPDUP output
#define TNO_TERM   	0x18	// Terminal type
#define   TERM_IS    	0x00	//   Terminal type is ...
#define   TERM_SEND  	0x01	//   Request for terminal type
#define TNO_EOR    	0x19	// End of record
#define TNO_TACACS 	0x1A	// TACACS user identification
#define TNO_OM     	0x1B	// Output marking
#define TNO_TLN    	0x1C	// Terminal location number
#define TNO_3270   	0x1D	// 3270 terminal mode
#define TNO_X3PAD  	0x1E	// X.3-Pad mode
#define TNO_NAWS   	0x1F	// Negotiate about window size
#define TNO_SPEED  	0x20	// Terminal speed option
#define   SPEED_IS   	0x00	//   Terminal speed is ...
#define   SPEED_SEND 	0x01	//   Request for terminal speed
#define TNO_TFC    	0x21	// Toggle flow control
#define   TFC_OFF    	0x00	//   Turn flow control off
#define   TFC_ON     	0x01	//   Turn flow control on
#define TNO_LM     	0x22	// Line mode
#define TNO_XLOC   	0x23	// X-display location
#define TNO_XOSMODE	0x30	// XOS mode
#define   XOS_BGNKB  	1	//   Begin using XOS mode keyboard encoding
#define   XOS_ENDKB  	2	//   End using XOS mode keyboard encoding
#define   XOS_BGNDSP 	3	//   Begin using XOS mode display encoding
#define   XOS_ENDDSP 	4	//   End using XOS mode display encoding
#define   XOS_VIDMODE	5	//   Display mode set response
#define   XOS_VIDFONT	6	//   Display font set response
#define TNOMAX     	0x30h

// Define offsets in a TFTP packet

struct tftp_pkt
{
    ushort tftp_opcode;     	// Opcode (all messges)
    ushort tftp_block;      	// Block number (data and ack messages)
    uchar tftp_data;    		// Data (data message)
};

#define tftp_filename   tftp_block // File name (read and write request)
#define tftp_errcode    tftp_block // Error code (error message)
#define tftp_errmsg     tftp_data  // Error message (error message)

// Define TFTP message types/opcodes

#define TFTPO_RRQ  	1		// Read request
#define TFTPO_WRQ  	2		// Write request
#define TFTPO_DATA 	3		// Data
#define TFTPO_ACK  	4		// Acknowledgment
#define TFTPO_ERROR	5		// Error

// Define TFTP error codes

#define TFTPE_NOTDEF 	0	// Error not defined
#define TFTPE_NOTFND 	1	// File not found
#define TFTPE_ACSERR 	2	// Access violation
#define TFTPE_DSKFULL	3	// Disk full or allocation exceeded
#define TFTPE_ILLOP  	4	// Illegal TFTP operation
#define TFTPE_BADID  	5	// Unknown transfer ID
#define TFTPE_FILEX  	6	// File already exists
#define TFTPE_NOUSER 	7	// No such user

// Define XFP commands sent to the server

#define XFPF_DATA     	1	// Data message (response required)
#define XFPF_DATAPAR  	2	// Data message with parameters (response required)
#define XFPF_DATANR   	3	// Data message (no response)
#define XFPF_DATAPARNR	4	// Data message with parameters (no response)
#define XFPF_DATAREQ  	8	// Data request
#define XFPF_OPEN     	9	// Open file
#define XFPF_PARM     	10	// Parameter function
#define XFPF_CLOSE    	11	// Close file
#define XFPF_DELETE   	12	// Delete file
#define XFPF_RENAME   	13	// Rename file
#define XFPF_COMMIT   	14	// Commit data

// Define XFP commands sent by the serer

#define XFPF_DATA     	1	// Data message
#define XFPF_DRESP    	3	// Data response
#define XFPF_DRESPPAR 	4	// Data response with parameters
#define XFPF_DRESPF   	5	// Data response (final)
#define XFPF_DRESPFPAR	6	// Data response (final) with parameters
#define XFPF_OPNRESP  	7	// Open response
#define XFPF_RESPONSE 	8	// Normal response
#define XFPF_EOF      	9	// EOF response
#define XFPF_ERROR    	10	// Error response

// Define XFP parameter values

#define XFPAR_FILOPTN 	1	// File option bits
#define XFPAR_FILSPEC 	2	// File specification string
#define XFPAR_DEVSTS  	3	// Device status
#define XFPAR_UNITNUM 	4	// Unit number
#define XFPAR_GLBID   	5	// Global device ID
#define XFPAR_DIRHNDL 	12	// Directory handle for search
#define XFPAR_SRCATTR 	14	// File attributes for search
#define XFPAR_FILATTR 	15	// File attributes
#define XFPAR_DIROFS  	16	// Directory offset for search
#define XFPAR_ABSPOS  	17	// Absolute position in file
#define XFPAR_RELPOS  	18	// Relative position in file (returns absolute position)
#define XFPAR_EOFPOS  	19	// Position in file relative to EOF (returns absolute
			//   position)
#define XFPAR_LENGTH  	21	// Written length of file (bytes)
#define XFPAR_REQALLOC	22	// Request file space allocation (bytes)
#define XFPAR_RQRALLOC	23	// Require file space allocation (bytes)
#define XFPAR_GRPSIZE 	24	// Allocation group size (bytes)
#define XFPAR_ADATE   	25	// Last access date/time
#define XFPAR_CDATE   	26	// Creation date/time
#define XFPAR_MDATE   	27	// Modify date/time
#define XFPAR_PROT    	28	// File protection
#define XFPAR_OWNER   	29	// Owner name

#define XFPAR_SETLOCK 	31	// Set file lock
#define XFPAR_CLRLOCK 	32	// Clear file lock
#define XFPAR_SHRPARMS	33	// File sharing parameters

#define XFPAR_FSTYPE  	96	// File system type
#define XFPAR_SECTSIZE	97	// Sector size (in bytes)
#define XFPAR_CLSSIZE 	98	// Cluster size (in sectors)
#define XFPAR_TTLSPACE	99	// Total space (in clusters)
#define XFPAR_AVLSPACE	100	// Available space (in clusters)
#define XFPAR_NUMHEAD 	101	// Number of heads
#define XFPAR_NUMSECT 	102	// Number of sectors
#define XFPAR_NUMCYLN 	103	// Number of cylinders

#define XFPAR_MODE    	110	// Transfer mode
#define XFPAR_USER    	111	// User name
#define XFPAR_PSWRD   	112	// Password

// Define AFP commands sent to the server

#define AFPF_OPEN     	1	// Open file
#define AFPF_PARM     	2	// Parameter function
#define AFPF_CLOSE    	3	// Close file
#define AFPF_COMMIT   	4	// Commit data
#define AFPF_DELETE   	5	// Delete file
#define AFPF_RENAME   	6	// Rename file
#define AFPF_DATA     	8	// Data
#define AFPF_DATAREQ  	9	// Data request

// Define AFP commands sent by the serer

#define AFPF_RESPONSE 	1	// Normal response
#define AFPF_DATA     	8	// Data
#define AFPF_DATARESP 	9	// Data response

// Define AFP parameter values

#define AFPAR_FILOPTN 	1	// File option bits
#define AFPAR_FILSPEC 	2	// File specification string
#define AFPAR_DEVSTS  	3	// Device status
#define AFPAR_UNITNUM 	4	// Unit number
#define AFPAR_GLBID   	5	// Global device ID
#define AFPAR_DIRHNDL 	12	// Directory handle for search
#define AFPAR_SRCATTR 	14	// File attributes for search
#define AFPAR_FILATTR 	15	// File attributes
#define AFPAR_DIROFS  	16	// Directory offset for search
#define AFPAR_ABSPOS  	17	// Absolute position in file
#define AFPAR_RELPOS  	18	// Relative position in file (returns absolute position)
#define AFPAR_EOFPOS  	19	// Position in file relative to EOF (returns absolute
			//   position)
#define AFPAR_LENGTH  	21	// Written length of file (bytes)
#define AFPAR_REQALLOC	22	// Request file space allocation (bytes)
#define AFPAR_RQRALLOC	23	// Require file space allocation (bytes)
#define AFPAR_GRPSIZE 	24	// Allocation group size (bytes)
#define AFPAR_ADATE   	25	// Last access date/time
#define AFPAR_CDATE   	26	// Creation date/time
#define AFPAR_MDATE   	27	// Modify date/time
#define AFPAR_PROT    	28	// File protection
#define AFPAR_OWNER   	29	// Owner name

#define AFPAR_SETLOCK 	31	// Set file lock
#define AFPAR_CLRLOCK 	32	// Clear file lock
#define AFPAR_SHRPARMS	33	// File sharing parameters

#define AFPAR_USERNAME	64	// User name
#define AFPAR_USERKEY 	65	// Security key

#define AFPAR_FSTYPE  	96	// File system type
#define AFPAR_SECTSIZE	97	// Sector size (in bytes)
#define AFPAR_CLSSIZE 	98	// Cluster size (in sectors)
#define AFPAR_TTLSPACE	99	// Total space (in clusters)
#define AFPAR_AVLSPACE	100	// Available space (in clusters)
#define AFPAR_NUMHEAD 	101	// Number of heads
#define AFPAR_NUMSECT 	102	// Number of sectors
#define AFPAR_NUMCYLN 	103	// Number of cylinders

// Define TNP function and response values

#define TNPF_OPEN    	1	// Open file
#define TNPF_CLOSE   	2	// Close file
#define TNPF_DELETE  	3	// Delete file
#define TNPF_RENAME  	4	// Rename file
#define TNPF_DREQ    	5	// Data request
#define TNPF_SETPOS  	6	// Set position for IO
#define TNPF_CKSUMREQ	7	// Checksum request
#define TNPF_RESPN   	129	// Normal response
#define TNPF_EOF     	130	// EOF response
#define TNPF_ERROR   	131	// Error response
#define TNPF_CKSUMRSP	132	// Checksum response

// Define bits for bytes 1 through 3 in the TNPF_OPEN command

// Values for byte 1

#define TNPO_RDVNM 	0x00800000	// Return device name
#define TNPO_RPATH 	0x00400000	// Return path name
#define TNPO_RFLNM 	0x00200000	// Return file name
#define TNPO_RATTR 	0x00100000	// Return file attribute byte
#define TNPO_RVRSN 	0x00080000	// Return file version number
#define TNPO_RSMASK	0x00040000	// Return search mask
#define TNPO_RFLEN 	0x00020000	// Return written length of file
#define TNPO_RDIRO 	0x00010000	// Return directory offset

// Values for byte 2

#define TNPO_RINFO 	0x00008000	// Return other information about file
#define TNPO_RCDATE	0x00004000	// Return creation date and time
#define TNPO_WDFLT 	0x00001000	// Wild extension should be default
#define TNPO_ODF   	0x00000800	// Open directory as file
#define TNPO_CLOSE 	0x00000400	// Close immediately
#define TNPO_REPEAT	0x00000200	// Repeated operation
#define TNPO_CONTIN	0x00000100	// Continuous mode

// Values for byte 3

#define TNPO_XREAD 	0x00000080	// Exclusive read mode
#define TNPO_XWRITE	0x00000040	// Exclusive write mode
#define TNPO_FAILEX	0x00000020	// Fail if file exists
#define TNPO_CREATE	0x00000010	// Create new file
#define TNPO_TRUNC 	0x00000008	// Truncate existing file
#define TNPO_APPEND	0x00000004	// Append
#define TNPO_IN    	0x00000002	// Allow input
#define TNPO_OUT   	0x00000001	// Allow output

// Define values for TNP open command options

#define TNPO_MODE   	1	// File mode
#define TNPO_CDATE  	2	// Creation date and time
#define TNPO_MDATE  	3	// Modify date and time
#define TNPO_ADATE  	4	// Access date and time
#define TNPO_PROT   	5	// File protection
#define TNPO_FATTR  	6	// Attribute bits for file
#define TNPO_SATTR  	7	// Attribute bits for directory search
#define TNPO_WRTN   	8	// Written length of file
#define TNPO_ALLOC  	9	// Allocated length of file
#define TNPO_USER   	10	// User name
#define TNPO_PSWRD  	11	// Password
#define TNPO_OWNER  	12	// Owner name
#define TNPO_GROUP  	13	// Group name
#define TNPO_DHANDLE	14	// Directory handle
#define TNPO_DOFFSET	15	// Directory offset

// Define structure for data returned by the IOPAR_IPS_RTDATA IO parameter

struct ipsroute
{
    char type;
    char interface;
    char xxx1;
    char xxx2;
    unsigned char ipa[4];
    long t2l;
    unsigned char gwipa[8];
};

# include <_epilog.h>

#endif	// _XOSNET_H_
