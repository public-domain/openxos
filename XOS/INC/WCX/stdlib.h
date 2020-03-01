//------------------------------------------------------------------------------
//
//  STDLIB.H - ANSI C header file for general utility functions
//
//  Edit History:
//  -------------
//  17Aug94 (fpj) - Modified for new CLIBX.
//  26Aug88 (brn) - Original creation.
//	01Aug00 (cey) - Move source to \XOSSRC\INC.
//					o) Create \XOSSRC\INC\MAKEFILE.MAK for installation.
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

#ifndef _STDLIB_H_
#define _STDLIB_H_

# include <_prolog.h>

#ifndef _SIZE_T
#define _SIZE_T

typedef _size_t size_t;

#endif

#ifndef _WCHAR_T
#define _WCHAR_T

typedef _wchar_t wchar_t;

#endif  // _WCHAR_T

typedef struct
{   int quot;					// Quotient
    int rem;					// Remainder
} div_t;

typedef struct
{   long quot;					// Quotient
    long rem;					// Remainder
} ldiv_t;

typedef struct
{	long low;
	long high;
} LLONG;

#define NULL    _NULL

#define EXIT_FAILURE 1			// Program terminated w/ failure
#define EXIT_SUCCESS 0			// Program terminated w/ success

#define RAND_MAX     32767		// Largest size of random no.

#define MB_CUR_MAX   1			// Largest size of multibyte chr.
								//  in current locale
void BREAK(void);
#pragma aux BREAK = "INT 3"

// Prototypes for ANSI functions

void abort(void);

int abs(int j);

//@@@

typedef void (*FNEXIT)() ;

//@ extern int atexit(void (*func)(void));

int    atexit( FNEXIT );
double atof(const char *nptr);
int    atoi(const char *nptr);
long   int atol(const char *nptr);
void  *bsearch(const void *key, const void *base, size_t nmemb,
        size_t size, int (*compar)(const void *, const void *));
void  *calloc(size_t nmemb, size_t size);

//// div_t div(int numer, int denom);

void   exit(int status);
void   free(void *ptr);
char  *getenv(const char *name);
long   labs(long int j);

//// ldiv_t ldiv(long int numer, long int denom);

void  *malloc(size_t size);
int    mblen(const char *s, size_t n);
size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n);
int    mbtowc(wchar_t *pwc, const char *s, size_t n);
void   qsort(void *base, size_t nmemb, size_t size,
        int (*compar)(const void *, const void *));
int    rand(void);
void  *realloc(void *ptr, size_t size);
void   srand(unsigned int seed);
double strtod(const char *nptr, char **endptr);
long   strtol(const char *nptr, char **endptr, int base);
unsigned long int strtoul(const char *nptr, char **endptr, int base);
int    system(const char *string);
size_t wcstombs(char *s, const wchar_t *pwcs, size_t n);
int    wctomb(char *s, wchar_t wchar);

// Prototypes for non-ANSI functions

void  *brk( void * pNew);
void  *getspace(long size);
void  *heapsort(void *root, int (*comp)(void *a, void *b, void *data),
	    void *data);
void   longadd(LLONG *result, long value);
void   longlongadd(LLONG *result, LLONG *value);
void   longadddiv(long *result, long num1, long num2, long num3, long dem);
void   longdiv(long *result, long num1, long num2, long dem);
void   nullfunc(void);
unsigned char  rotate8(unsigned char value, int amnt);
unsigned short rotate16(unsigned short value, int amnt);
unsigned long  rotate32(unsigned long value, int amnt);
void  *sbrk(long incr);

# include <_epilog.h>

#endif // _STDLIB_H_
