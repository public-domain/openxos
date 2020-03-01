#include <ERRNO.H>
#include <LIMITS.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <XOSERR.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSTIME.H>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>

//@@@ CEY - Added 00Aug25
char * strdup( char * s )
{
	int		nLen = strlen( s );
	char	* t;

	t = malloc( nLen + 1 );
	strcpy( t, s );

	return t;
}

//
// Difference in seconds between local and UCT/GMT for
// Unix times (time_t)
//
// Notes:
//	1) FAT32 directory entry times are saved as Local Times.
//	   we need to artificially adjust to maintain compatibility
//	   with unix/linux...
//
//long	g_dwTimeZoneOffset	= -5 * 3600L;	// -0500 from GMT
long	g_dwTimeZoneOffset	= -4 * 3600L;	// -0500 from GMT

//
// Desc:
//	Convert/return XOS system time format as unix-time.
//
// Inputs:
//	t		time_t (unixtime) value to convert
//	ts		pointer to time_s buffer to receive converted time.
//
// Returns:
//	0		Success
//	~0		XOS Error code
//
// Notes:
//  UnixTime is defined as count of seconds since Jan 0, 1970 GMT.
//
long time_unix_to_s( time_t t, time_s * ts, int adjust_flag )
{
	long		days;
	long		secs;
	long		utime = (long)t;

	if( utime < 0 )
	{
		utime = 0;
	}

	days	= utime / 86400L;
	secs	= utime % 86400L;

	days	+= 135140L;

	if( adjust_flag )
	{
		//
		// Adjust for Local vs UCT time
		//
		secs	+= g_dwTimeZoneOffset;	// TZ offset @@@

		if( secs < 0L )
		{
			secs += 86400L;
			days--;
		}
		else if( secs >= 86400L )
		{
			secs -= 86400L;
			days++;
		}
	}

	//
	// 2^32 / 86400 = 49710.26963
	//
	// Conversion factor to convert seconds to fractional days...
	//
	ts->date = days;
	ts->time = secs * 49710L + 40000L;	// Convert to fractional days


	return( 0 );
}

//
// Desc:
//	Convert/return XOS system time format as unix-time.
//
// Notes:
//  UnixTime is defined as count of seconds since Jan 0, 1970 GMT.
//
long time_s_to_unix( time_s * ts, int adjust_flag )
{
	long		days;
	long		utime;
	time_x		tmx;
	time_d		* pt = &tmx.dos;

	//
	// Initialize 'time_x' struct for conversion.
	//
	tmx.sys = *ts;

    svcSysDateTime(T_CVX2D, &tmx );

	days = ts->date;

	days -= 135140L;		// Difference between XOS/Unix dates (days)

	utime = days * 86400L;
	utime += pt->tmx_hour * 3600L;
	utime += pt->tmx_min * 60L;
	utime += pt->tmx_sec;

	if( adjust_flag )
	{
		utime -= g_dwTimeZoneOffset;			// Adjust for TZ...
	}

	return( utime );
}

//
// Table to map month values(1..12) to Unix/POSIX 3 char string.
// Used by: ctime, asctime,...
//
static char * g_aszMonths[12+1] =
{
	"?Mn",
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static char * g_aszDOW[7] =
{
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

//
// Desc:
//	Converts a unixtime(time_t) to an ASCIZ string. (Unix/POSIX)
//
// Notes:
//	1) Format is "ddd mmm DD HH:MM:SS YYYY\n"
//	2) Example:  "Wed Jun 30 21:49:%02d 1998\n"
//	3) returns 26 bytes (24 + NL + NUL)
//
char * ctime( time_t * timep )
{
	static char		szBuf[32];
	time_x			tmx;
	time_d			* pt = &tmx.dos;

	time_unix_to_s( *timep, &tmx.sys, 1 );

    svcSysDateTime(T_CVX2D, &tmx );

	//	sprintf( szBuf,
	//		, secs % 60
	//		);

	sprintf( szBuf, "%-3.3s %-3.3s %2d %02d:%02d:%02d %4d\n"
		, g_aszDOW[pt->tmx_wday % 7]
		, g_aszMonths[pt->tmx_mon]
		, pt->tmx_mday
		, pt->tmx_hour
		, pt->tmx_min
		, pt->tmx_sec
		, pt->tmx_year
		);
	return( szBuf );
}

#define CFG_FILATTR	1
//
// Desc:
//	Get file information (type, size, timestamps...)
//
// Notes:
//	1) Implements, where possible, Unix/Linux/POSIX functionality
//
int fstat( int fd, struct stat * p )
{
    static struct posparms
    {
        byte4_parm		pos;
#if CFG_FILATTR
		byte2_parm		fattr;
#endif
		byte4_parm		devsts;
        time8_parm		mtime;
        time8_parm		atime;
        time8_parm		ctime;
		byte16_parm		m_guid;
        char       		end;
	} info =
    {
        { PAR_GET | REP_HEXV, 4, IOPAR_LENGTH,	0 },
#if CFG_FILATTR
		{ PAR_GET | REP_HEXV, 2, IOPAR_FILATTR, 0 },
#endif
        { PAR_GET | REP_HEXV, 4, IOPAR_DEVSTS,	0 },
        { PAR_GET | REP_HEXV, 8, IOPAR_MDATE,	0 },
        { PAR_GET | REP_HEXV, 8, IOPAR_ADATE,	0 },
        { PAR_GET | REP_HEXV, 8, IOPAR_CDATE,	0 },
        { PAR_GET | REP_HEXV,16, IOPAR_GLBID,	0 },
        { 0 },
    };


	memset( p, 0x00, sizeof(struct stat) );

	memset( info.m_guid.value, 0x00, 16 );

	//
	// Get file creation, modification and access times
	//
    if( svcIoInBlockP( fd, NULL, 0, &info ) < 0 )
	{
		fprintf( stderr, "? fstat:get info failed.\n" );
        //return -1;            // Quit if can't set position
	}
	else
	{
		if( (info.fattr.value & A_DIRECT) != 0 )
		{
			//# fprintf( stdout, "%% fd=%d -> DIRECTORY\n", fd );

			p->st_mode	= __S_IFDIR;		// Directory
		}
		else
			p->st_mode	= __S_IFREG;		// Regular file

		p->st_rdev	= 0;				// @@@

		//
		// File timestamps (ctime=mtime=atime for XOS/FAT32...)
		//
		p->st_ctime = time_s_to_unix( &info.ctime.value, 1 );
		p->st_mtime = time_s_to_unix( &info.mtime.value, 1 );
		p->st_atime = time_s_to_unix( &info.atime.value, 1 );

		//
		// Written length of file
		//
		p->st_size	= info.pos.value;

#if 0
		fprintf( stdout, "%% fstat:ctime=%9d mtime=%9d devsts=%04X attr=%04X len=%d\n"
			, p->st_ctime, p->st_mtime
			, info.devsts.value
			, info.fattr.value
			, p->st_size );
		fprintf( stdout, " mtime=%s\n", ctime( &p->st_mtime ) );

		fprintf( stdout, " %%fstat: GUID= %08X %08X %08X %08X\n"
					, *(long *)(info.m_guid.value + 12)
					, *(long *)(info.m_guid.value +  8)
					, *(long *)(info.m_guid.value +  4)
					, *(long *)(info.m_guid.value +  0)
					);

#endif
	}

	return 0;
}

//
// Desc:
//
int stat( const char * name, struct stat * p )
{
	int		fd;
	int		nRet;

	//# fprintf( stdout, "%%  stat name=%s\n", name );

	fd = open( (char *)name, O_RDONLY, 0 );

	if( fd < 0 )
	{
		char	szDir[256];

		strcpy( szDir, name );
		strcat( szDir, "\\" );

	    fd = svcIoOpen( O_ODF, szDir, NULL);
		if( fd < 0 )
		{
			return( -1 );
		}
		//# fprintf( stdout, " %% stat name=%s O_ODF=%d\n", szDir, fd );
		nRet = fstat( fd, p );

		svcIoClose( fd, 0 );
		return( nRet );
	}

	nRet = fstat( fd, p );

	close( fd );

	return nRet;
}

//
// Desc:
//	Open a file or device for R/W access.
//
// Inputs:
//	name	File name to open/create
//	oflags	Open Flags (logical or of:)
//				O_CREAT
//				O_TRUNC
//				O_RDRW | O_RDONLY | O_WRONLY
//				O_APPENDX !!!
//
// Returns:
//	< 0			Error code
//  >= 0		File Handle
//
// Notes:
//	1) The unix/posix O_APPEND conflicted with the XOS definition.
//	   Since these were not numerically identical, O_APPENDX
//     was defined. This needs to be changed...
//	2) Error return codes are XOS specific.
//	   Need to map XOS->POSIX codes. This is a non-trival issue
//	   involving errno.h, error message to string, etc.
//	   Additionally, where does this end? Do we need to support
//	   POSIX error codes for Buffered-IO (fopen, fread...)?
//
//	   If the goal is 'simplifying' unix/linux ports, the current
//	   implementation is sufficient. Compiling un-modified linux code
//	   such as GCC/GPP will require much more work. We need to
//	   examine this closely regarding tradeoffs..
//
int  open( char * name, int oflags, int mode )
{
	long	bits = 0;
	long	handle;

	mode = mode;	// Keep compiler happy

	//
	// Map oflags to svcIoOpen 'bits'
	//
	if( (oflags & O_CREAT)  != 0 )		bits |= O_CREATE ;
	if( (oflags & O_TRUNC)  != 0 )		bits |= O_TRUNCW ;
	switch( oflags & 0x03 )
	{
	case O_RDWR:
		bits |= (O_IN|O_OUT) ;
		break;

	case O_WRONLY:
		bits |= (O_OUT) ;
		break;

	case O_RDONLY:
		bits |= (O_IN) ;
		break;

	default:
		fprintf( stderr, "? open(%.40s, %04X,...) invalid oflags\n"
					, name, oflags );
		break;

	}

	if( (oflags & O_APPENDX)!= 0 )		bits |= O_APPEND ;

	//
	//
	//
    handle = svcIoOpen( bits, name, NULL);

	if( handle < 0 )
		return( -1 );

	return (int)handle;
}

//@@@@
//int  creat(char *name, int mode);

//
// Desc:
//  Read 'count' bytes from an unbuffered file/device handle.
//
// Inputs:
//
// Returns:
//	<0		XOS Error
//	0		EOF
//	+n		Cound of bytes transfered (read)
//
int  read( int handle, char *buffer, int count )
{
	long		res;

	res = svcIoInBlock( handle, (char far *)buffer, count );

	if( res < 0 )
	{
		printf( "? read:%d\n", res );
		return( -1 );
	}

	return (int)res;
}


//
// Desc:
//  Write 'count' bytes to an unbuffered file/device handle.
//
int  write( int handle, char * buffer, int count )
{
	long		res;

	res = svcIoOutBlock( handle, (char far *)buffer, count );

	if( res < 0 )
	{
		return( -1 );
	}

	return (int)res;
}

//
// Desc:
//
long filelength( int handle )
{
    struct posparms
    {
        byte4_parm pos;
        char       end;
    } getabs =
    {
        { PAR_GET | REP_HEXV, 4, IOPAR_LENGTH, 0 },
        { 0 },
    };


    if( svcIoInBlockP( handle, NULL, 0, &getabs ) < 0 )
        return -1;            // Quit if can't set position


	return getabs.pos.value;
}

//
// Desc:
//
int chsize( int handle, long newlen )
{
    struct posparms
    {
        byte4_parm pos;
        char       end;
    } getabs =
    {
        { PAR_SET | REP_HEXV, 4, IOPAR_LENGTH, 0 },
        { 0 },
    };

	getabs.pos.value = newlen;

    if( svcIoInBlockP( handle, NULL, 0, &getabs ) < 0 )
        return -1;            // Quit if can't set position


	return 0;
}

//
// Desc:
//
long tell( int handle )
{
    struct posparms
    {
        byte4_parm pos;
        char       end;
    } getabs =
    {
        { PAR_GET | REP_HEXV, 4, IOPAR_ABSPOS, 0 },
        { 0 },
    };


    if (svcIoInBlockP( handle, NULL, 0, &getabs ) < 0 )
        return -1;            // Quit if can't set position


	return getabs.pos.value;
}

//
// Desc:
//
long lseek( int handle, long position, int mode )
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

	switch( mode )
	{
	case SEEK_SET:		// 0
    	setabs.pos.value = position;
		break;

	case SEEK_CUR:
		setabs.pos.value = position + tell( handle );
		break;

	case SEEK_END:
		setabs.pos.value = position + filelength( handle );
		break;

	default:
		//!!! errno
		return -1;
	}


    if (svcIoInBlockP( handle, NULL, 0, &setabs ) < 0 )
	{
		//!!! set_errno
		
        return -1;            // Quit if can't set position
	}


	return position;
}

//
// Desc:
//
int  close( int handle )
{
	long res;

	res = svcIoClose( handle, 0 );

	if( res < 0 )
		return -1;

	return( 0 );
}
