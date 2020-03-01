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
#include <dirent.h>

#define MAX_DIRENTS		1000

int main( )
{
	int		n;
	DIR		* pDir;
	DIRENT	* pde;

//	pDir = opendir( "c:\\temp\\" );
//	pDir = opendir( "c:\\windows\\" );
	pDir = opendir( "c:\\windows" );
	if( !pDir )
	{
		printf( "? opendir failed\n" );
		exit( 1 );
	}

	for( n = 0; n < MAX_DIRENTS; n++ )
	{
		pde = readdir( pDir );
		if( !pde )
		{
			printf( "%% No more files err:%d\n", pDir->m_errno );
			break;
		}

		printf( "ino=%08X [%.40s]\n", pde->d_ino, pde->d_name );
	}

	n = closedir( pDir );
	printf( "%% test: closedir -> %d\n", n );
	return( 0 );
}
