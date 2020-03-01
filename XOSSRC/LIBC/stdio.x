////////////////////////////////////////////////////////////////////////////////
//
//  stdio.h - ANSI C header file for standard I/O
//
//  Copyright (c) 1988 Saguaro Software, Ltd. All rights reserved.
//  Copyright (c) 1994 Allegro Systems, Ltd. All rights reserved.
//
//  Edit history:
//
//   Date    Who  Description
//  -------------------------
//  17Aug94  FPJ  Modified for new CLIBX.
//   7May87  JRG  Original creation.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _STDIO_H
#define _STDIO_H

#ifndef _HOME_H

#include <home.h>

#endif // _HOME_H

#ifdef  __WATCOMC__

// Special stuff to make WATCOM C do the right things

#pragma pack (0);
#pragma off (check_stack);
#pragma on (unreferenced);
#pragma aux default "*" modify [EAX EBX ECX EDX GS];
#pragma aux ftesvc "*" parm reverse routine modify [EAX];

#endif  // __WATCOMC__

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
// Function prototypes
//

extern void clearerr(FILE *stream);

extern int fclose(FILE *stream);

extern int feof(FILE *stream);

extern int ferror(FILE *stream);

extern int fflush(FILE *stream);

extern int fgetc(FILE *stream);

extern int fgetpos(FILE *stream, fpos_t *pos);

extern char *fgets(char *s, int n, FILE *stream);

extern FILE *fopen(const char *filename, const char *mode);

extern int fprintf(FILE *stream, const char *format, ...);

extern int fputc(int c, FILE *stream);

extern int fputs(const char *s, FILE *stream);

extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

extern FILE *freopen(const char *filename, const char *mode, FILE *stream);

extern int fscanf(FILE *stream, const char *format, ...);

extern int fseek(FILE *stream, long int offset, int whence);

extern int fsetpos(FILE *stream, const fpos_t *pos);

extern long int ftell(FILE *stream);

extern size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

#define getc(stream)    fgetc(stream)

extern int getchar(void);

extern char *gets(char *s);

extern void perror(const char *s);

extern int printf(const char *format, ...);

#define putc(c, stream) fputc(c, stream)

extern int putchar(int c);

extern int puts(const char *s);

extern int remove(const char *filename);

extern int rename(const char *old, const char *new);

extern void rewind(FILE *stream);

extern int scanf(const char *format, ...);

extern void setbuf(FILE *stream, char *buf);

extern int setvbuf(FILE *stream, char *buf, int mode, size_t size);

extern int sprintf(char *s, const char *format, ...);

extern int sscanf(const char *s, const char *format, ...);

extern FILE *tmpfile(void);

extern char *tmpnam(char *s);

extern int ungetc(int c, FILE *stream);

extern int vfprintf(FILE *stream, const char *format, va_list arg);

extern int vprintf(const char *format, va_list arg);

extern int vsprintf(char *s, const char *format, va_list arg);

#endif // _STDIO_H

