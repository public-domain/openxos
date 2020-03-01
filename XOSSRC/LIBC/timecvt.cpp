#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xossvc.h>
#include <xostime.h>

#include "timecvt.h"


//
// Desc:
//	Convert/return XOS system time format as unix-time.
//
// Notes:
//  UnixTime is defined as count of seconds since Jan 0, 1970 GMT.
//
long TimeCvt::GetUnix( )
{
	long		days;
	long		utime;
	time_x		tmx;
	time_d		* pt = &tmx.dos;


	if( m_fDebug )
	{
		printf( "*** GetUnix sys=%10d:%08X (Enter)\n", m_SysTime[1], m_SysTime[0] );
	}

	tmx.sys = *(time_s *)m_SysTime;

    svcSysDateTime(T_CVX2D, &tmx );

	days = m_SysTime[1];

	if( m_fDebug )
	{
		printf( " #GetUnix  =%2d/%2d/%2d %02d:%02d:%02d (sys[1]=%10d)\n"
			, pt->tmx_mon
			, pt->tmx_mday
			, pt->tmx_year
			, pt->tmx_hour
			, pt->tmx_min
			, pt->tmx_sec
			, days
			);
	}

	days -= 135140L;

	utime = days * 86400L;
	utime += pt->tmx_hour * 3600L;
	utime += pt->tmx_min * 60L;
	utime += pt->tmx_sec;
	utime += 4 * 3600L;				// Adjust for TZ...

	return( utime );
}

void TimeCvt::SetUnix( long lTime )
{
	long		days;
	long		secs;
	long		utime = lTime;

	if( utime < 0 )
	{
		utime = 0;
	}

	days	= utime / 86400L;
	secs	= utime % 86400L;

	days	+= 135140L;
	secs	-= 4 * 3600L;

	if( secs < 0L )
	{
		secs += 86400L;
		days--;
	}


	//
	// 2^32 / 86400 = 49710.26963
	//
	if( m_fDebug )
	{
		printf( "->SetUnix(%10d)  = %d days, %d secs %08X\n"
			, utime
			, days, secs
			, secs * 49710L
			);
	}

	m_SysTime[1] = days;
	m_SysTime[0] = secs * 49710L + 40000L;

	if( m_fDebug )
		printf( "*** SetUnix sys=%10d:%08X\n", m_SysTime[1], m_SysTime[0] );
}


