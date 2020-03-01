//
// Name:
//	timecvt.h
//
class TimeCvt
{
public:
	long		m_SysTime[2];			// XOS - Days, fractional days
	int			m_fDebug;				// Enable debug output

	//
	// Constructor: (Default)
	//
	TimeCvt( )
	{
		m_fDebug		= 0;
		m_SysTime[0]	= 0;
		m_SysTime[1]	= 0;
	}

	void Now( )
	{
		svcSysDateTime( T_GTXDTTM, m_SysTime );
	}

	//
	// Desc:
	//
	void Set( long * pSysTime )
	{
		m_SysTime[0] = pSysTime[0];
		m_SysTime[1] = pSysTime[1];
	}

	//
	// Desc:
	//  Get XOS "system" formatted date/time
	//
	void Get( long * pSysTime )
	{
		pSysTime[0] = m_SysTime[0];
		pSysTime[1] = m_SysTime[1];
	}

	//
	// Desc:
	//	Convert/return XOS system time format as unix-time.
	//
	// Notes:
	//  UnixTime is defined as count of seconds since Jan 0, 1970 GMT.
	//
	long GetUnix( );

	//
	// Desc:
	//  Convert/Set a unixtime value.
	//
	void SetUnix( long lTime );

};


