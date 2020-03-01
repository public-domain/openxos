//------------------------------------------------------------------------------
//
//  STDIO.H - ANSI C header file for standard I/O
//
//  Edit History:
//  -------------
//   7May87 (jrg) - Original creation.
//  17Aug94 (fpj) - Modified for new CLIBX.
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

#ifndef _STDIO_H_
#define _STDIO_H_

#include <_prolog.h>

#ifndef _SIZE_T
#define _SIZE_T

typedef _size_t size_t;

#endif

struct _iobuf
{
    long iob_handle;                    // Device handle
    long iob_flag;                      // Flag bits
    long iob_error;                     // Error code
    long iob_length;                    // Length of file (bytes)
    long iob_ungetc;                    // ungetc() character
    long iob_offset;                    // Offset in file for buffer (bytes)
    long iob_buffer;                    // Offset of buffer
    long iob_bsize;                     // Buffer size (bytes)
    long iob_pnt;                       // Offset of next byte in buffer
    long iob_count;                     // No. of bytes available in buffer
    long iob_end;                       // Value of iob_count for EOF
};

typedef struct _iobuf FILE;

typedef unsigned long fpos_t;

#define NULL    _NULL

// Define buffering types used by setvbuf

#define _IOFBF          0               // Fully buffered
#define _IOLBF          1               // Line buffered
#define _IONBF          2               // Not buffered

#define BUFSIZ          512             // Size of buffer in setvbuf()

#define EOF             (-1)            // End-of-file indicator

#define FOPEN_MAX       25              // Max. no. of open files

#define FILENAME_MAX    (8+1+3)         // Longest file name

#define L_tmpnam        (8+1+3)         // Longest temporary file name

// Define values for fseek()

#define SEEK_SET        0               // From beginning of file
#define SEEK_CUR        1               // From current position
#define SEEK_END        2               // From end of file

#define TMP_MAX         20              // Min. number of unique temp. files

extern FILE _stdin;
extern FILE _stdout;
extern FILE _stderr;
extern FILE _stdtrm;
extern FILE _iob[FOPEN_MAX];

#define stdin   (&_stdin)
#define stdout  (&_stdout)
#define stderr  (&_stderr)
#define stdtrm  (&_stdtrm)

#define STDIN           1               // Standard input
#define STDOUT          2               // Standard output
#define STDERR          3               // Standard error output
#define STDPRN          4               // Standard printer
#define STDTRM          5               // Standard terminal
#define STDFV           6               // First variable

#ifndef _TIME_S
#define _TIME_S

typedef struct sysdttm
{
    ulong time;
    long  date;
} time_s;

#endif // _TIME_S

// File Info structure

typedef struct fileinfo
{
    ulong  wsize;
    ulong  asize;
    time_s create;
    time_s modify;
    time_s access;
    ulong  protect;
    char   owner[16];
    char   group[16];
    ulong  grpsize;
} file_info;

// Define va_list for v[fs]printf() functions prototypes

#ifndef _VA_LIST
#define _VA_LIST

typedef void *va_list;

#endif  // _VA_LIST

// Misc. Defines that are used

#define TRUE            1
#define FALSE           0

#define _F_CHNG  0x80                   // Buffer has been changed
#define _F_DIR   0x40                   // Direct (non-buffered) file
#define _F_DISK  0x20                   // Disk file
#define _F_ASCII 0x10                   // ASCII file
#define _F_ALLOC 0x08                   // Buffer allocated with malloc
#define _F_WRITE 0x02                   // File is open for output
#define _F_READ  0x01                   // File is open for input

// Bits for the if_usts word

#define US_CHNGD  0x8000                // Disk has been changed
#define US_MOUNT  0x4000                // Disk is mounted
#define US_VALID  0x2000                // Disk contains valid data
#define US_NOTF   0x1000                // Disk is not file structured
#define US_MEDIA  0x0800                // Media type is specified
#define US_RECAL  0x0400                // Need to recalibrate
#define US_HFTRK  0x0200                // Have 48 tpi disk in 96 tpi drive
#define US_MOTON  0x0100                // Motor is on (floppy only)
#define US_TKDEN  0x00C0                // Track density
#define US_DBLS   0x0020                // Disk is double sided (floppy only)
#define US_M8H    0x0020                // Disk has more than 8 heads (hard disk
                                        //   only)
#define US_DEN    0x0018                // Data density
#define US_RSIZE  0x0007                // Record size

//
// Prototypes for ANSI functions
//

extern void  clearerr(FILE *stream);

extern int   fclose(FILE *stream);

extern FILE *fdopen(const long handle, const char *mode);

extern int   feof(FILE *stream);

extern int   ferror(FILE *stream);

extern int   fflush(FILE *stream);

extern int   fgetc(FILE *stream);

extern int   fgetpos(FILE *stream, fpos_t *pos);

extern char *fgets(char *s, int n, FILE *stream);
extern int   fgetsl(char *s, int n, FILE *stream);

extern FILE *fopen(const char *filename, const char *mode);

extern int   fprintf(FILE *stream, const char *format, ...);

extern int   fputc(int c, FILE *stream);

extern int   fputs(const char *s, FILE *stream);

extern int   fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

extern FILE *freopen(const char *filename, const char *mode, FILE *stream);

extern int   fscanf(FILE *stream, const char *format, ...);

extern int   fseek(FILE *stream, long int offset, int whence);

extern int   fsetpos(FILE *stream, const fpos_t *pos);

extern long  int ftell(FILE *stream);

extern int   fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

#define getc(stream) fgetc(stream)

extern int   getchar(void);

extern char *gets(char *s);

extern void  perror(const char *s);

extern int   printf(const char *format, ...);

#define putc(c, stream) fputc(c, stream)

extern int   putchar(int c);

extern int   puts(const char *s);

extern int   remove(const char *filename);

extern int   rename(const char *oldfile, const char *newfile);

extern void  rewind(FILE *stream);

extern int   scanf(const char *format, ...);

extern void  setbuf(FILE *stream, char *buf);

extern int   setvbuf(FILE *stream, char *buf, int mode, size_t size);

extern int   sprintf(char *s, const char *format, ...);

extern int   sscanf(const char *s, const char *format, ...);

extern FILE *tmpfile(void);

extern char *tmpnam(char *s);

extern int   ungetc(int c, FILE *stream);

extern int   vfprintf(FILE *stream, const char *format, va_list arg);

extern int   vprintf(const char *format, va_list arg);

extern int   vsprintf(char *s, const char *format, va_list arg);

// Prototypes for non-ANSI functions

extern void  _exit(int status);

extern FILE *_fsetup(FILE *stream);

extern int   fgetfinfo(FILE *stream, file_info *info);

extern int   fgetstr(char *str, int size, FILE *stream);

extern int   findfile(const char *filespec, int attributes,
    int (*function)(const char *filename));

extern int   fisctrm(FILE *stream);

extern int   getch(void);

extern int   getche(void);

extern int   getfinfo(long handle, file_info *info);

extern int   isctrm(long handle);

#include <_epilog.h>

#endif // _STDIO_H_

