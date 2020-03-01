////////////////////////////////////////////////////////////////////////////////
//
//  LOG.C - Routines to manipulate log files (for symbionts).
//
//  Edit history:
//
//   Date    Who  Description
//  -------------------------
//  19Oct94  FPJ  Original creation.
//
////////////////////////////////////////////////////////////////////////////////

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

#include <ERRNO.H>
#include <LIMITS.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <XOSERR.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSTIME.H>

#include <LOG.H>


#define HEADER_DUMMY    (RECORD_SIZE - (sizeof(SERVER_ID) - 1) - \
                        sizeof(unsigned short) - SERVER_SIZE - \
                        sizeof(unsigned short) - sizeof(unsigned short) - \
                        sizeof(unsigned short))

#define FIRST_DATA      (RECORD_SIZE - sizeof(unsigned short) - \
                        sizeof(unsigned short) - sizeof(unsigned short) - \
                        sizeof(time_s))

#define EXTRA_DATA      (RECORD_SIZE - sizeof(unsigned short))

#define DATA_OFFSET     (sizeof(struct hblock))
#define FIRST_BLOCK     (DATA_OFFSET / RECORD_SIZE)

#define FILE_CREATE     0
#define FILE_OPEN       1


// Structure of blockette for log file header

struct hblock
{
    char id[sizeof(SERVER_ID) - 1]; // Log file ID ("SLOG")
    unsigned short version;         // Version number
    char server[SERVER_SIZE];       // Server name
    unsigned short size;            // Max. no. of 64-byte blocks (exc. header)
    unsigned short lastread;        // Last record read
    unsigned short lostcount;       // No. of records not read
    char dummy[HEADER_DUMMY];       // Dummy bytes
};

// Structure of blockette for log file message

union dblock
{
    struct
    {
        unsigned short sequence;    // Sequence number
        unsigned short count;       // No. of data bytes
        unsigned short type;        // Record type
        time_s datetime;            // Date and time
        char data[FIRST_DATA];      // Data bytes
    } first;

    struct
    {
        unsigned short sequence;    // Sequence number
        char data[EXTRA_DATA];      // Data bytes
    } extra;
};

// Local data

static long log_handle = 0;         // File handle, positive if file open
static int next_block;              // Next blockette number to use (1+)
static int next_sequence;           // Next sequence number (1+)
static int max_blocks;              // Max. no. of data blockettes
static int file_size;               // File size in bytes (at time of open)
static int high_sequence;           // Highest sequence number seen

// Function prototypes

static int file_open(const char *file, int mode);
static int find_last(void);
static int get_block(void *block);
static int get_message(int *start_block, int *end_block, int *sequence,
    int *type, time_s *datetime, char *pstring, int nbytes);
static int set_pos(long offset);


//
//  file_open() - Internal function to open or create file
//
////////////////////////////////////////////////////////////////////////////////

static int file_open(
    const char *filename,               // Name of file to open or create
    int mode)                           // Mode to open file in
{
    char *fullname;                     // Full file name (including ext.)
    int len;                            // Length of file name plus ext.

    if (log_handle > 0)                 // Is file already open?
        return EXIT_FAILURE;

    if (strchr(filename, '.') != NULL)  // File extension specified?
        fullname = (char *)filename;
    else                                // No extension, so add one
    {
        len = strlen(filename) + sizeof(LOG_EXTENSION) - 1;

        if ((fullname = malloc(len + 1)) == NULL)
            return EXIT_FAILURE;
        else
        {
            strcpy(fullname, filename);
            strcat(fullname, LOG_EXTENSION);
        }
    }

    if (mode == FILE_CREATE)
        log_handle = svcIoOpen(O_OUT | O_IN | O_CREATE | O_TRUNCW,
                fullname, NULL);
    else
        log_handle = svcIoOpen(O_OUT | O_IN, fullname, NULL);

    if (fullname != filename)           // De-allocate memory if necessary
        free(fullname);

    if (log_handle <= 0)                // Did open fail?
    {
        log_handle = 0;

        return EXIT_FAILURE;
    }
    else
        return EXIT_SUCCESS;
}


//
//  find_last() - Scan log file
//
////////////////////////////////////////////////////////////////////////////////

static int find_last(void)
{
    int exit_code;
    int current_end = 1;
    int current_sequence = 0;
    int last_end = 1;
    int last_sequence = 0;
    int pos;

    next_block = 1;

    // If linear file, or we have not reached our maximum size,
    // just position to end of file.

    do
    {
        if (last_sequence < current_sequence)
        {
            last_sequence = current_sequence;
            last_end = current_end;
        }
        exit_code = get_message(NULL, &current_end, &current_sequence,
                    NULL, NULL, NULL, 0);

        if (exit_code == EXIT_FAILURE)
            if (!max_blocks || (max_blocks + 1) * RECORD_SIZE > file_size)
            {
                pos = file_size;
                break;
            }
            else
                return EXIT_FAILURE;

    } while (last_sequence < current_sequence);

    if (!max_blocks || (max_blocks + 1) * RECORD_SIZE > file_size)
        pos = file_size;
    else
        pos = last_end * RECORD_SIZE;

    next_sequence = last_sequence + 1;

    return set_pos(pos);
}


//
//  get_block() - Internal function to get next block of log file.
//
////////////////////////////////////////////////////////////////////////////////

static int get_block(
    void *block)
{
    long rcode;
    int i;

    for (i = 0; i < max_blocks; ++i)
    {
        rcode = svcIoInBlock(log_handle, (void far *)block, RECORD_SIZE);

        if (rcode == RECORD_SIZE)
        {
            if (next_block)
                ++next_block;

            return 0;
        }
        else if (rcode != ER_EOF)
            return EXIT_FAILURE;
        else if (set_pos(DATA_OFFSET) == EXIT_FAILURE)
            return EXIT_FAILURE;
    }
    return EXIT_FAILURE;
}


//
//  get_message() - Internal function to get next message of log file.
//
////////////////////////////////////////////////////////////////////////////////

static int get_message(
    int *start_block,                   // Starting blockette no. for message
    int *end_block,                     // Ending blockette no. for message
    int *sequence,                      // Sequence number for message
    int *type,                          // Message type
    time_s *datetime,                   // Date and time of message
    char *pstring,                      // Place to store message data (or NULL)
    int nbytes)                         // No. of bytes for message
{
    union dblock dblock;
    int max_bytes = FIRST_DATA;
    char *current_data = &dblock.first.data;
    int nextras;                        // No. of extra blockettes to read

    do
    {
        if (get_block(&dblock) == EXIT_FAILURE)
            return EXIT_FAILURE;
    } while (!dblock.first.sequence);

    if (start_block != NULL)
        if ((*start_block = next_block - 1) == 0)
            *start_block = max_blocks;

    if (nbytes <= 0)
        pstring = NULL;                 // Don't output if no room
    else
        --nbytes;                       // Subtract one for the trailing NUL

    if ((nextras = dblock.first.count - FIRST_DATA) > 0)
        nextras = (nextras + EXTRA_DATA - 1) / EXTRA_DATA;
    else
        nextras = 0;

    if (sequence != NULL)
        *sequence = dblock.first.sequence;

    if (type != NULL)
        *type = dblock.first.type;

    if (datetime != NULL)
        *datetime = dblock.first.datetime;

    for (;;)
    {
        if (pstring != NULL)
        {
            if (nbytes >= max_bytes)
            {
                memcpy(pstring, current_data, max_bytes);
                pstring += max_bytes;
                nbytes -= max_bytes;
            }
            else
            {
                memcpy(pstring, current_data, nbytes);
                pstring += nbytes;      // Skip past what we just output
                memset(pstring, 0, max_bytes - nbytes);
                nbytes = 0;
            }

            if (max_bytes != EXTRA_DATA) // If first record, change currency
            {
                max_bytes = EXTRA_DATA;
                current_data = dblock.extra.data;
            }
        }

        if (nextras-- <= 0)
            break;

        if (get_block(&dblock) < 0)
            return EXIT_FAILURE;
    }

    if (pstring != NULL)
        *pstring++ = '\0';

    if (end_block != NULL)
    {
        if (next_block > max_blocks)
            *end_block = 1;
        else
            *end_block = next_block;
    }

    return EXIT_SUCCESS;
}


//
//  log_close() - Close log file
//
////////////////////////////////////////////////////////////////////////////////

int log_close(void)
{
    if (!log_handle)
        return EXIT_FAILURE;

    svcIoClose(log_handle, 0);

    log_handle = 0;

    return EXIT_SUCCESS;
}


//
//  log_open_read() - Open log file for reading.
//
////////////////////////////////////////////////////////////////////////////////

int log_open_read(
    const char *filename)               // Name of file to open
{
    struct hblock hblock;

    if (file_open(filename, FILE_OPEN) == EXIT_FAILURE)
        return EXIT_FAILURE;

    // Read log file header and make sure it's okay

    if (svcIoInBlockP(log_handle, (char far *)&hblock, RECORD_SIZE, NULL)
            != RECORD_SIZE)
    {
        log_close();

        return EXIT_FAILURE;
    }

    max_blocks = hblock.size;

    if (find_last() == EXIT_FAILURE)
    {
        log_close();

        return EXIT_FAILURE;
    }
    else
        return EXIT_SUCCESS;
}


//
//  log_open_write() - Open log file for writing (and reading).
//
////////////////////////////////////////////////////////////////////////////////

int log_open_write(
    const char *filename,               // Name of file to create
    int max_bytes,                      // File size in bytes
    const char *servername)             // Name of symbiont
{
    struct lenparm
    {
        byte4_parm len;
        char       end;
    } getlen =
    {
        { PAR_GET | REP_HEXV, 4, IOPAR_LENGTH, 0 },
        { 0 },
    };

    int max_size;
    struct hblock hblock;

    if (max_bytes)
        max_size = max_bytes / RECORD_SIZE - 1;
    else
        max_size = 0;

    if (max_size && max_size <= RECORD_SIZE)
        return EXIT_FAILURE;            // File is not large enough!

    // First try to open an existing log file.

    if (file_open(filename, FILE_OPEN) == EXIT_SUCCESS)
    {
        // Read log file header and make sure it's okay

        if (svcIoInBlockP(log_handle, (char far *)&hblock, RECORD_SIZE,
                &getlen) != RECORD_SIZE)
        {
            log_close();

            return EXIT_FAILURE;
        }

        if (hblock.size != (unsigned short)max_size)
        {
            log_close();

            return EXIT_FAILURE;
        }

        if ((file_size = getlen.len.value) < RECORD_SIZE)
            return EXIT_FAILURE;

        max_blocks = max_size;

        if (find_last() == EXIT_FAILURE)
        {
            log_close();

            return EXIT_FAILURE;
        }
        else
            return EXIT_SUCCESS;
    }

    // If log file doesn't exist, try to create it.

    if (file_open(filename, FILE_CREATE) == EXIT_FAILURE)
        return EXIT_FAILURE;

    memset(&hblock, '\0', RECORD_SIZE);

    strncpy(hblock.id, SERVER_ID, sizeof(SERVER_ID) - 1);
    hblock.version = LOG_VERSION;
    strncpy(hblock.server, servername, SERVER_SIZE);
    hblock.size = (unsigned short)max_size;
    hblock.lastread = 0;
    hblock.lostcount = 0;

    if (svcIoOutBlock(log_handle, (char far *)&hblock, RECORD_SIZE) < 0)
    {
        log_close();

        return EXIT_FAILURE;
    }

    next_sequence = 1;
    next_block = FIRST_BLOCK;
    max_blocks = max_size;

    return EXIT_SUCCESS;
};


//
//  log_read_data() - Read data from log file
//
////////////////////////////////////////////////////////////////////////////////

int log_read_data(
    char *pstring,                      // Where to store data
    int nbytes,                         // No. of bytes to output
    struct log_data *info)              // Place to store extra info
{
    if (get_message(NULL, NULL, &info->sequence, &info->type, &info->datetime,
            pstring, nbytes) == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }
    
    if (high_sequence >= info->sequence)
        return EXIT_FAILURE;            // Quit if we've wrapped back to start
    else
    {
        high_sequence = info->sequence;

        return EXIT_SUCCESS;
    }
}


//
//  log_read_header() - Read header from log file
//
////////////////////////////////////////////////////////////////////////////////

int log_read_header(
    struct log_header *info)            // Place to store header info
{
    struct hblock hblock;

    if (set_pos(0) == EXIT_FAILURE)     // Position to start of log file
        return EXIT_FAILURE;

    if (svcIoInBlock(log_handle, (char far *)&hblock, RECORD_SIZE)
            != RECORD_SIZE)
    {
        return EXIT_FAILURE;            // Read the header block
    }

    strncpy(info->id, hblock.id, sizeof(SERVER_ID) - 1);
    info->version = (int)hblock.version;
    strncpy(info->server, hblock.server, SERVER_SIZE);
    info->size = (int)hblock.size;

    return EXIT_SUCCESS;
}


//
//  log_rewind() - Position to start of oldest event in log file.
//
////////////////////////////////////////////////////////////////////////////////

int log_rewind(void)
{
    int current_sequence = 1;
    int current_start = 1;
    int first_sequence = 0;
    int exit_code;

    if (set_pos(DATA_OFFSET) == EXIT_FAILURE)
        return EXIT_FAILURE;

    do
    {
        if (first_sequence < current_sequence)
            first_sequence = current_sequence;

        exit_code = get_message(&current_start, NULL, &current_sequence, NULL,
                NULL, NULL, 0);

        if (exit_code == EXIT_FAILURE)
            return EXIT_FAILURE;

    } while (current_sequence > first_sequence);

    high_sequence = 0;

    return set_pos(current_start * RECORD_SIZE);
}


//
//  log_write() - Write data to log file
//
////////////////////////////////////////////////////////////////////////////////

int log_write(
    int type,                           // Record type
    const char *pstring)                // Message string to output
{
    union dblock dblock;
    int max_bytes = FIRST_DATA;
    int nbytes = strlen(pstring);       // No. of bytes in message
    char *current_data = &dblock.first.data;

    if (!nbytes)                        // Must have something to output
        return EXIT_FAILURE;

    dblock.first.sequence = (unsigned short)next_sequence++;
    dblock.first.count = (unsigned short)nbytes;
    dblock.first.type = (unsigned short)type;

    if (svcSysDateTime(T_GTXDTTM, &dblock.first.datetime) < 0)
        return EXIT_FAILURE;
    
    do
    {
        //
        // If this is not a linear file (size != 0) and we
        // have wrapped around, then reset to start of file.
        //

        if (max_blocks && next_block++ > max_blocks)
        {
            if (set_pos(DATA_OFFSET) == EXIT_FAILURE)
                return EXIT_FAILURE;
        }

        if (nbytes >= max_bytes)
            memcpy(current_data, pstring, max_bytes);
        else
        {
            memcpy(current_data, pstring, nbytes);
            memset(current_data + nbytes, 0, max_bytes - nbytes);
        }

        if (svcIoOutBlock(log_handle, (char far *)&dblock, RECORD_SIZE) < 0)
            return EXIT_FAILURE;

        pstring += max_bytes;           // Skip past what we just output
        nbytes -= max_bytes;

        if (max_bytes != EXTRA_DATA)    // If first record, change currency
        {
            max_bytes = EXTRA_DATA;
            current_data = dblock.extra.data;
            dblock.extra.sequence = 0;
        }
    } while (nbytes > 0);

    return EXIT_SUCCESS;
}


//
//  set_pos() - Set position in log file.
//
////////////////////////////////////////////////////////////////////////////////

static int set_pos(
    long offset)
{
    struct posparms
    {
        byte4_parm pos;
        char       end;
    } setabs =
    {
        { PAR_SET | REP_HEXV, 4, IOPAR_ABSPOS, 0 },
        { 0 },
    };

    setabs.pos.value = offset;

    if (svcIoInBlockP(log_handle, NULL, 0, &setabs) < 0)
        return EXIT_FAILURE;            // Quit if can't set position

    next_block = offset / RECORD_SIZE;  // Calculate next block to use

    return EXIT_SUCCESS;
}

