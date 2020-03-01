//------------------------------------------------------------------------------
//
//  LOG.H - Header file for XOS log functions
//
// Format of header
//
//  Byte    Description
//
//   0-3    "SLOG" in ASCII (for server log)
//   4-35   Name of server in ASCII (padded with NUL bytes)
//  36-37   No. of 64-byte blocks to allow for file (0 means infinite)
//  38-39   Sequence no. of record last read (unused in this version)
//  40-41   No. of records lost (unused in this version)
//  42-63   Unused (should be zero)
//
// Format of records
//
//  Byte    Description
//
//   0-1    Sequence number (1-65535)
//   2-3    No. of bytes in message
//   4-5    Record type (currently undefined)
//   6-13   Date/time (XOS format)
//  14-63   Data
//
//  -OR-
//
//  Byte    Description
//
//   0-1    Sequence number (0 means continuation)
//   2-63   Data
//
// Multi-byte values are stored in little-endian format.
//
//  Edit History:
//  -------------
//  18May95 (fpj) - Added comment block.
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

#ifndef _LOG_H_
#define _LOG_H_

#ifndef _BASELINE_H_

#include <baseline.h>

#endif // _BASELINE_H_


#define LOG_EXTENSION   ".LOG"      // Default file extension
#define LOG_VERSION     0x0101
#define RECORD_SIZE     64
#define SERVER_ID       "SLOG"
#define SERVER_SIZE     32

// Structure of log header

struct log_header
{
    char id[sizeof(SERVER_ID) - 1]; // Log file ID ("SLOG")
    int version;                    // Version number
    char server[SERVER_SIZE];       // Server name
    int size;                       // Max. no. of 64-byte blocks (exc. header)
};

// Structure of event record

struct log_data
{
    int sequence;                   // Sequence number
    int type;                       // Record type
    time_s datetime;                // Date and time
};


int log_close(void);
//
// return is EXIT_FAILURE or EXIT_SUCCESS

int log_open_read(const char *file);
//
// return is EXIT_FAILURE or EXIT_SUCCESS

int log_open_write(const char *file, int max_size, const char *server);
//
// return is EXIT_FAILURE or EXIT_SUCCESS

int log_read_data(char *data, int nbytes, struct log_data *info);
//
// return is no. of bytes in next record, or EOF if no more records

int log_read_header(struct log_header *info);
//
// return is EXIT_SUCCESS

int log_rewind(void);
//
// return is EXIT_FAILURE or EXIT_SUCCESS

int log_write(int type, const char *buffer);
//
// return is no. of bytes unwritten, 0 means none, -1 means EOF

#endif  // _LOG_H_
