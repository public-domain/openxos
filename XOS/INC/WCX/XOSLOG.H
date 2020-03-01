//-------------------------------------------------------------------------
// XOSLOG.H
// Parameters for LOGICAL name searches for XOS
//
// Written by: John R. Goltz
//
// Edit History:
// 09/07/92(brn) - Add comment Header
//
//-------------------------------------------------------------------------

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

// This file defines values associatd with the svcIoFindLog and svcIoDefLog
//   system calls

// Following bits control logical name searches (used in the level argument)

#ifndef _XOSLOG_H
#define _XOSLOG_H

#define FLN_SYSTEM  0xC000	// Search system level
#define FLN_SESSION 0x8000	// Specify search level relative to session
#define FLN_PROCESS 0x4000	// Specify search level relative to process

/* Following bits are used in the type argument and are returned in the */
/*    value of the svc_iodeflog system call */

#define TLN_SUBST   0x40000000L	// Substituted name
#define TLN_ROOTED  0x20000000L	// Rooted name

#endif	/* _XOSLOG_H_ */
