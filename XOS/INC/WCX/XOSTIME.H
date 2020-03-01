//------------------------------------------------------------------------------
//
//  XOSTIME.H - Time parameters for XOS
//
//  Edit History:
//  -------------
//  09/05/88(brn) - Created first version
//   8Feb95 (sao) - Creation of dt_parse() routine
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

#ifndef _XOSTIME_H_
#define _XOSTIME_H_

# include <_prolog.h>

// XOS system time conversion factors

#define XT_MILLISEC     50        // Number of timer counts per millisec.
#define XT_SECOND       49710     // Number of timer counts per second
#define XT_MINUTE       2982616   // Number of timer counts per minute
#define XT_HOUR         178956971 // Number of timer counts per hour

// Define offsets in the time_d structure - this is used to hold DOS (or broken
//   down) time values

typedef struct tmx
{   unsigned short tmx_msec;    // Milliseconds (0 - 999)
    unsigned short tmx_sec;     // Second (0 -59)
    unsigned short tmx_min;     // Minute (0 - 59)
    unsigned short tmx_hour;    // Hour (0 - 23)
    unsigned short tmx_mday;    // Day of month (1 - 31)
    unsigned short tmx_mon;     // Month in year (1 - 12)
    unsigned short tmx_year;    // Year (1600 - )
    unsigned short tmx_wday;    // Day of week (0 - 6, 0 = Sunday)
    unsigned short tmx_yday;    // Day of year (0 - 365)
} time_d;

typedef struct tmxz
{   unsigned short tmx_msec;    // Milliseconds (0 - 999)
    unsigned short tmx_sec;     // Second (0 -59)
    unsigned short tmx_min;     // Minute (0 - 59)
    unsigned short tmx_hour;    // Hour (0 - 23)
    unsigned short tmx_mday;    // Day of month (1 - 31)
    unsigned short tmx_mon;     // Month in year (1 - 12)
    unsigned short tmx_year;    // Year (1600 - )
    unsigned short tmx_wday;    // Day of week (0 - 6, 0 = Sunday)
    unsigned short tmx_yday;    // Day of year (0 - 365)
             short tmx_tzone;	// Time zone (offset from GMT in minutes)
             short tmx_dlst;	// Daylight savings time offset (minutes)
} time_dz;

// Define function values for the svcSysDateTime system call

#define T_GTXDTTM      1		// Get current date/time in system format
#define T_GTDDTTM      2		// Get current date/time in DOS format
#define T_GTXDTTMTZ    3		// Get current date/time with time zone
								//   information in system format
#define T_GTDDTTMTZ    4		// Get current date/time with time zone
								//   information in DOS format
#define T_GTPETIM      6		// Get process elapsed time (system format)
#define T_GTSETIM      7		// Get session elapsed time (system format)
#define T_GTPCTIM      8		// Get process CPU time (system format)
#define T_CVD2X        9		// Convert DOS to system date/time
#define T_CVX2D       10		// Convert system to DOS date/time
#define T_GTHRDTTM    11		// Get current DHR date/time (system format)
#define T_GTHRPETIM   12		// Get high-res process time (system format)
#define T_GTHRSETIM   13		// Get high-res session time (system format)
#define T_GTHRDTTMTZ  14		// Get current DHR date/time with time zone
								//   information (system format)
#define T_GTRDTTM     15		// Get RTC date/time (admin priv)
#define T_STXDTTM     16		// Set system date/time (admin priv)
#define T_STRDTTM     17		// Set RTC date/time (admin priv)
#define T_GTTZINFO    18		// Get time zone information
#define T_STTZINFO    19		// Set time zone information

// Define time for XOS

#ifndef _TIME_S
#define _TIME_S

//
// Legacy/Standard "C"/XOS definition
//
typedef struct sysdttm
{	unsigned long time;
	long     date;
} time_s;


#endif // _TIME_S

typedef struct sysdttmtz
{
    unsigned long time;
    long     date;
    short    tzone;
    short    dlst;
} time_sz;

typedef struct
{
    time_s sys;
    time_d dos;
} time_x;

struct hrtime
{
    unsigned long second;
    unsigned long fraction;
};

//  The following #defines are used by dt_parse

// Acceptable date formats for dt_parse are:
//
//    US      mm/dd/[[yy]yy]
//    INTL    dd.mm.[[yy]yy]
//    TEXT    dd[-]mmm[-][[yy]yy]  or
//            mmm[-]dd[-][[yy]yy]
//    PACK    yymmdd (all six values must be present)
//    TIME    hh:mm[:ss][A|P]
//
//  Note:    If the year is not included, current year is used
//           If century is not included, current century is used

#define INVDTFORMAT     1
#define INVMONTH        2
#define INVDAY          3
#define INVDTRANGE      4
#define INVTMMIN        5
#define INVTMHOUR       6
#define INVTMSECOND     7
#define INVTMFORMAT     8

// XOS time functions

void add2sdt(time_s *dtv, long value);
int  dt_parse(char *inp_str, time_s *dt);
int  ddt2sdt(time_d *tm, time_s *dtv);
int  ddt2str(char *string, char *format, time_dz *tm);
void dosdttm(time_d *datetime);
long hourmin2sys(long time);
void sdt2ddt(time_s *dtv, time_d *tm);
int  sdt2str(char *string, char *format, time_sz *sdt);
void sdtadd(time_s *sum, time_s *a1, time_s *a2);
int  sdtcmp(time_s *a, time_s *b);
void sdtsub(time_s *dif, time_s *s1, time_s *s2);
void xos_sysdttm(time_s *datetime);

# include <_epilog.h>

#endif  // _XOSTIME_H_
