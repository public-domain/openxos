//------------------------------------------------------------------------------
//
//  TIME.H - ANSI C header file for time functions
//
//  Edit History:
//  -------------
//   7Jun87 (jrg) - Original creation.
//  26Aug88 (brn) - Added ANSI support functions.
//   8Sep92 (brn) - Move XOS specific stuff to XOSTIME.H.
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

#ifndef _TIME_H_
#define _TIME_H_

# include <_prolog.h>

#define NULL    _NULL

#ifndef _SIZE_T
#define _SIZE_T

typedef _size_t size_t;

#endif // _SIZE_T

#ifndef _TIME_S
#define _TIME_S

typedef struct sysdttm
{
    ulong time;
    long  date;
} time_s;

#endif // _TIME_S

typedef _time_t time_t;

typedef _clock_t clock_t;

#define CLOCKS_PER_SEC  _CLOCKS_PER_SEC

struct	tm
{
    int tm_sec;		// Seconds after the minute - [0 - 59]
    int tm_min;		// Minutes after the hour - [0- 59]
    int tm_hour;	// Hours since midnight - [0 - 23]
    int tm_mday;	// Day of the month - [1 -31]
    int tm_mon;		// Months since January - [0 - 11]
    int tm_year;	// Years since 1900
    int tm_wday;	// Days since Sunday - [0 - 6]
    int tm_yday;	// Days since January 1 - [0, 365]
    int tm_isdst;	// Daylight Saving time flag
};

//
// Function prototypes
//

//
// @@@ 
//
extern char *asctime(const struct tm *timeptr);

extern clock_t clock(void);		// return processor time used

//
//
//
extern char *ctime(const time_t *timer);

extern double difftime(time_t time1, time_t time0);

extern struct tm *gmtime(const time_t *timer);

extern struct tm *localtime(const time_t *timer);

extern time_t mktime(struct tm *timeptr);

extern size_t strftime(char *s, size_t maxsize, const char *format,
        const struct tm *timeptr);

time_t time(time_t *timer);

# include <_epilog.h>

#endif // _TIME_H_
