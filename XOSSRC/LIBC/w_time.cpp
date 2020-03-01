#include <ERRNO.H>
#include <LIMITS.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <XOSERR.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSTIME.H>
//#include <fcntl.h>
#include <time.h>
#include <ctype.h>

#include <TimeCvt.h>

extern "C" long time_unix_to_s( time_t t, time_s * ts, int adjust_flag );

//
// Desc:
//
time_t time( time_t * t )
{
//@@@	time_d		tmd;
	time_t		uTime;		// Unix time

	//@@@ dosdttm( &tmd );



#if 0	//@@@@ REMOVE
	svcSysDateTime( T_GTDDTTM, &tmd );

	uTime = (time_t)tmd.tmx_sec + (time_t)(tmd.tmx_min * 60L)
			+ (time_t)(tmd.tmx_hour * 3600L);


//@@@	uTime += (time_t)tmd.tmx_yday * 86400L;

	uTime += (time_t)(  (time_t)(tmd.tmx_year - 1970L) * (864L * 36525L));
	uTime += (time_t)(  (time_t)(tmd.tmx_mon) * (864L * 3050) );
	uTime += (time_t)(  (time_t)(tmd.tmx_mday) * 86400L );

	uTime += 86400 * 100L;


#endif


	TimeCvt		tc;

	tc.Now( );
	uTime = tc.GetUnix( );

	if( t )
		*t = uTime;

	return( uTime );
}


//
// Desc:
//
struct tm * localtime( const time_t * timer )
{
	static struct tm	tmval;
	time_t				t;

	if( !timer )
	{
		t = time( NULL );
	}
	else
	{
		t = *timer;
	}


	time_x			tmx;
	time_d			* pt = &tmx.dos;

	time_unix_to_s( t, &tmx.sys, 1 );

    svcSysDateTime(T_CVX2D, &tmx );

	tmval.tm_sec	= pt->tmx_sec;
	tmval.tm_min	= pt->tmx_min;
	tmval.tm_hour	= pt->tmx_hour;
	tmval.tm_year   = pt->tmx_year	- 1900;
	tmval.tm_mon    = pt->tmx_mon	- 1;
	tmval.tm_mday   = pt->tmx_mday;
	tmval.tm_yday   = 303;


	return( &tmval );
}

