//------------------------------------------------------------------------
// XOSDCSTS.H
//
// Written by: John R. Goltz
//
// Edit History:
// 09/07/92(brn) - Add comment header
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

#ifndef _XOSDCSTS_H
#define _XOSDCSTS_H

typedef struct dcstsdata
{   long dcs_dcbt;	// Total number of disk DCBs
    long dcs_dcbf;	// Number of free disk DCBs
    long dcs_dcbu;	// Number of in-use disk DCBs
    long dcs_ofibs;	// Number of open FIBs
    long dcs_qelt;	// Total number of QELs 
    long dcs_qelf;	// Number of free QELs
    long dcs_sbuft;	// Total number of system cache buffers
    long dcs_sbufa;	// Number of available system cache buffers
    long dcs_sbuff;	// Number of free system cache buffers
    long dcs_sbuf0;	// Number of system cache buffers with 0 users
    long dcs_sbuf1;	// Number of system cache buffers with 1 user
    long dcs_sbuf2;	// Number of system cache buffers with 2 users
    long dcs_sbuf3;	// Number of system cache buffers with 3 users
    long dcs_sbuf4;	// Number of system cache buffers with 4 users
    long dcs_sbuf5;	// Number of system cache buffers with 5 or more
			//   users
    long dcs_dbuft;	// Total number of data cache buffers
    long dcs_dbufa;	// Number of available data cache buffers
    long dcs_dbuff;	// Number of free data cache buffers
    long dcs_dbuf0;	// Number of data cache buffers with 0 users
    long dcs_dbuf1;	// Number of data cache buffers with 1 user
    long dcs_dbuf2;	// Number of data cache buffers with 2 users
    long dcs_dbuf3;	// Number of data cache buffers with 3 users
    long dcs_dbuf4;	// Number of data cache buffers with 4 users
    long dcs_dbuf5;	// Number of data cache buffers with 5 or more users
} dcsts_data;

#endif	// _XOSDCSTS_H
