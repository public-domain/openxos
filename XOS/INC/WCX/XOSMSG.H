//------------------------------------------------------------------------
// XOSMSG.H
// Interprocess message types for XOS
//
// Written by: John R. Goltz
//
// Edit History:
// 09/07/92(brn) - Add comment Header
//
//------------------------------------------------------------------------

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

// Define standard message types (this value is stored in the first byte
//   of a standard system message)
//
// Values 1 - 15 are reserved for general response messages

#ifndef _XOSMSG_H
#define _XOSMSG_H

#define MT_INTRMDMSG 1		// Intermediate normal response
#define MT_INTRMDWRN 2		// Intermediate warning response
#define MT_INTRMDERR 3		// Intermediate error response
#define MT_FINALMSG  5		// Final normal response
#define MT_FINALWRN  6		// Final warning response
#define MT_FINALERR  7		// Final error response

// Values 16 - 31 are reserved for requests to INIT

#define MT_SYMBREQ   16		// Request to run symbiont
#define MT_TERMREQ   17		// Request to change terminal state
#define MT_SYSLOG    18		// Data for system log
#define MT_USERREQ   19		// Userfile request
#define MT_TERMDATA  20		// Data from idle terminal

// Values 32 - 47 are reserved for requests to general servers

#define MT_SRVCMD    32		// Server command

// Values 48 - 63 are reserved for requests to the screen symbiont

#define MT_EGASWITCH 48		// EGA screen switch
#define MT_EGACLEAR  49		// EGA screen clear
#define MT_EGADUMP   50		// EGA screen dump to printer

// Values 64 - 79 are reserved for requests to the error logging symbiont

// Values 80 - 95 are reserved for requests to unspool symbionts

#define MT_UNSPLREADY 80	// File is ready to unspool
#define MT_UNSPLCMD   81	// Command

// Values 96 - 99 are reserved for accounting functions

#define MT_ACT        96	// System accounting message

// Values 100 - 127 are reserved for additional system functions

// Values 128 - 255 are available for user defined uses

#endif	// _XOSMSG_H_
