#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Debug level
//	0	None
//	1	Log fatal errors/potential problems...
//	2	Log warnings
//	...
//	7	Log all - Verbose
//
#define CFG_DEBUG		1


#if (CFG_DEBUG > 0)

# pragma off (unreferenced);

extern "C" FILE		* g_fileLog;

#endif


#define MAX_NEW		1000

int		g_fNewInit = 0;

void *	g_anew[MAX_NEW];

//
//
//
void ** _findNew( void * pVoid )
{
	int		n;
	for( n = 0; n < MAX_NEW; n++ )
	{
		if( pVoid == g_anew[n] )
			return( &g_anew[n] );
	}
	return( 0 );
}

//
//
//
void _addNew( void * pVoid )
{
	if( !g_fNewInit )
	{
		memset( g_anew, 0x00, sizeof(g_anew) );
		g_fNewInit = 1;
	}

	//
	// Check if already exists
	//
	void	** pp = _findNew( pVoid );

	if( pp )
	{
#if (CFG_DEBUG >= 2)
		fprintf( g_fileLog, "? _addNew(%08X) exists!\n", (int)pVoid );
#endif
		return;
	}

	//
	// Find a free slot
	//
	pp = _findNew( 0 );
	if( !pp )
	{
#if (CFG_DEBUG >= 2)
		fprintf( g_fileLog, "? _addNew(%08X) table full\n", (int)pVoid );
#endif
		return;
	}

	*pp = pVoid;
}

//
//
//
int _delNew( void * pVoid )
{
	if( !pVoid )
	{
		//
		// Null pointer passed
		//
#if (CFG_DEBUG >= 2)
		fprintf( g_fileLog, "?? _delNew(%08X) NULL PTR! @@@@@@\n", pVoid );
		fflush( g_fileLog );
#endif
		return -1;
	}

	void	** pp = _findNew( pVoid );
	if( !pp )
	{
#if (CFG_DEBUG >= 2)
		fprintf( g_fileLog, "?? _delNew(%08X) not found @@@@@@\n", pVoid );
		fflush( g_fileLog );
#endif
		return -2;
	}

	*pp = 0;
	return 0;
}

//
//
//
extern "C" void **	_malloc_mhead;
extern "C" void **	_malloc_mtail;
extern "C" long		_malloc_amount;

extern "C" void _dumpHeap( );	//@@@
extern "C" void _dumpMem ( void * pMem, long nMax );

void _dumpMem( void * pMem, long nMax )
{
	uchar	* p		= (uchar *)pMem;
	int		ixc;

	if( nMax < 1 )		nMax = 1;
	if( nMax > 1024 )	nMax = 1024;

	for( int ix = 0; ix < nMax; ix += 16 )
	{
		int		nSize = nMax - ix;

		if( nSize > 16 )	nSize = 16;

		for( ixc = 0; ixc < nSize; ixc++ )
		{
			fprintf( g_fileLog, " %02X", p[ix+ixc] );
		}
		while( ixc < 16 )
		{
			fprintf( g_fileLog, "   " );
			ixc++;
		}

		fprintf( g_fileLog, "[ " );
		for( ixc = 0; ixc < nSize; ixc++ )
		{
			if( p[ix+ixc] >= 0x20 && p[ix+ixc] < 0x7F )
				fprintf( g_fileLog, "%c", p[ix+ixc] );
			else
				fprintf( g_fileLog, "." );
		}

		fprintf( g_fileLog, " ]\n" );
	}
	fprintf( g_fileLog, "\n" );
}

void _dumpHeap( )
{
#if 0
	int		n;
	int		nMax;
	long	dwCnt	= 0;
	long	dwSize	= 0;
#endif

#if (CFG_DEBUG >= 1)
	fprintf( g_fileLog, "****** _malloc_mhead = %08X _mtail = %08X ******\n"
		, (int)(_malloc_mhead)
		, (int)(_malloc_mtail)
		);

	void	** pp = _malloc_mhead;
	void	** p_mnext;
	void	** p_mprev;
	void	** p_fnext;
	void	** p_fprev;
	long	dwSize;
	long	dwData;

	while( pp )
	{
		dwData  = (long   )(pp[ 0]);		// 1st 4 bytes of user data
		dwSize	= (long   )(pp[-1]);		// Size, # bytes allocated
		p_fnext	= (void **)(pp[-2]);		// Get mb_fnext (Free)
		p_fprev	= (void **)(pp[-3]);		// Get mb_fprev
		p_mnext	= (void **)(pp[-4]);		// Get mb_mnext
		p_mprev	= (void **)(pp[-5]);		// Get mb_mprev

		fprintf( g_fileLog, " [%08X]: size=%8d mnext=%08X mprev=%08X fnext=%08X fprev=%08X data=%08X\n"
			, (int)pp
			, dwSize
			, (int)(p_mnext)
			, (int)(p_mprev)
			, (int)(p_fnext)
			, (int)(p_fprev)
			, dwData
			);

		_dumpMem( pp, (dwSize > 64) ? 64 : dwSize );
		fflush( g_fileLog );

		if( (int)p_mnext == -1 )
			break;		//???

		pp = p_mnext;
	}

	if( _malloc_mtail )
		fprintf( g_fileLog, "\n  *mtail = %08X ???\n", (int)(*_malloc_mtail) );

	fprintf( g_fileLog, "  amount = %8u\n", _malloc_amount );

#endif
}

//
//
//
void _dumpNew( )
{
#if (CFG_DEBUG > 0)
	int		n;
	int		nMax;
	long	dwCnt	= 0;
	long	dwSize	= 0;


	for( n = 0; n < MAX_NEW; n++ )
	{
		if( g_anew[n] )
		{
			unsigned char	* p = (unsigned char *)g_anew[n];
			long			* pdwLen = (long *)(p - 4);

			dwCnt++;
			dwSize += *pdwLen;

			fprintf( g_fileLog, "@@@ _dumpNew: mem[%3d]=%08X Len=%5d\n"
				, n, (int)(g_anew[n]), *pdwLen );

			nMax = *pdwLen;
#if 0
			if( nMax < 1 )		nMax = 1;
			if( nMax > 1024 )	nMax = 1024;

			for( int ix = 0; ix < nMax; ix++ )
			{
				fprintf( g_fileLog, " %02X", p[ix] );
				if( (ix & 0x0F) == 0x0F )
					fprintf( g_fileLog, "\n" );
			}
			fprintf( g_fileLog, "\n\n" );
#endif
			_dumpMem( p, nMax );
		}
	}

	fprintf( g_fileLog, "****** %u Bytes in %u blocks NOT deleted\n"
		, dwSize, dwCnt );

	_dumpHeap( );
#endif
}

//
// Desc:
//  This replaces the default 'global' operator new. This is required
//	for native XOS projects. This should eventually be moved to a system
//	library...
//
void * operator new( size_t nSize )
{
	if( nSize == 0 )
	{
		//
		// Allocating 0 bytes!!!
		//
		//	Allocate at least (nnn 1?)
		//
		nSize++;
	}

	void	* pVoid = (void *)malloc(nSize);

	//@@@ Check for allocation errors.
	//@@@ See new_handler( ) definition in C++ manual.

#if 1
	if( pVoid )
	{
//#		memset( pVoid, 0xEE, nSize );		//@@@ Debugging

		memset( pVoid, 0x00, nSize );		//@@@ Debugging
	}
#endif

#if (CFG_DEBUG > 1)
	fprintf( g_fileLog, "  *** new  (%5d) -> %08X ***\n", nSize, (int)pVoid );
#endif

	_addNew( pVoid );
	return pVoid;
}

//
//
//
void * operator new( size_t nSize, void * pVoid )
{

#if (CFG_DEBUG > 1)
	fprintf( g_fileLog, "  *** new@ (%5d) -> %08X ***\n", nSize, (int)pVoid );
#endif

	return( pVoid );
}

//
//
//
void * operator new []( int unsigned nSize )
{
#if (CFG_DEBUG > 1)
	fprintf( g_fileLog, "  *** new[](%d) ***\n", nSize );
#endif
	return operator new( nSize );
}

//
//
//
void operator delete( void * pVoid )
{
	long	dwSize = pVoid ? ((long *)pVoid)[-1] : -2;

#if (CFG_DEBUG > 1)
	fprintf( g_fileLog, "  *** delete  (%08X) size=%8d ***\n", (int)pVoid, dwSize );
#endif

	if( _delNew( pVoid ) == 0 )
		free( pVoid );
}

#if (CFG_DEBUG >= 7)
extern "C" void breakpnt( void );	//@@@ Create a debug module for LIBC...

void breakpnt( void )
{
}
#endif

void operator delete []( void * pVoid )
{
	fprintf( g_fileLog, "  *** delete[](%08X) ***\n", (int)pVoid );

	if( _delNew( pVoid ) == 0 )
		free( pVoid );

#if (CFG_DEBUG >= 7)
	breakpnt( );
#endif
}
