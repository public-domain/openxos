//
// Name:
//	z_dir.c
//
// Desc:
//	Linux/POSIX directory stream support.
//	1) opendir, readdir, closedir
//	2) rewinddir
//	3) telldir, seekdir, ... Not implemented
//
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

#define FILSPCSIZE	256			// Buffer size for readdir( ) filspec's


char	g_fname[FILSPCSIZE+4] = "";

//
//
//
typedef struct opendir_s
{
	byte4_parm  filoptn;	// PAR_SET|REP_HEXV
    lngstr_parm filspec;	// PAR_GET|REP_STR
	char		end;
} OPENDIR_S;

OPENDIR_S g_opendir =
{
	{PAR_SET|REP_HEXV, 4 , IOPAR_FILOPTN
		, FO_VOLNAME
//		| FO_NODENUM
//		| FO_NODENAME
//		| FO_RVOLNAME
		| FO_PATHNAME
//		| FO_ATTR
		| FO_NOPREFIX
		}
	, {PAR_GET|REP_STR , 0 , IOPAR_FILSPEC , &g_fname, 0, FILSPCSIZE, 0}
	, { 0 }
};


//
// Desc:
//
// Notes:
//

DIR	g_dirs[10];


typedef struct dirscanpl
{
	byte4_parm  filoptn;	// PAR_SET|REP_HEXV
    lngstr_parm filspec;	// PAR_GET|REP_STR
    byte2_parm  srcattr;	// PAR_SET|REP_HEXV
    byte2_parm  filattr;	// PAR_GET|REP_HEXV
    byte4_parm  dirhndl;	// PAR_SET|REP_DECV
    byte4_parm  dirofs;		// PAR_SET|REP_DECV
    char        end;
} DIRSCANPL1;



//
//
//
char g_filespec[FILSPCSIZE+4];

DIRSCANPL1 g_fileparm =
{
	//--- filoptn
	//
	{PAR_SET|REP_HEXV, 4 , IOPAR_FILOPTN
		, FO_VOLNAME
		| FO_NODENUM
		| FO_NODENAME
//		| FO_RVOLNAME
		| FO_PATHNAME
		| FO_FILENAME
		| FO_ATTR
//		| FO_NOPREFIX
		}
	//--- filspec
	//
	, {PAR_GET|REP_STR , 0 , IOPAR_FILSPEC , &g_filespec, 0, FILSPCSIZE, 0}

	, {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR
		, 0
		| A_NORMAL
		| A_RDONLY
		| A_HIDDEN | A_SYSTEM
		| A_DIRECT | A_LABEL | A_ARCH
		}

	//--- filattr (MS-DOS encoding - see A_XXX in <xos.h> )
	//
	, {PAR_GET|REP_HEXV, 2 , IOPAR_FILATTR , 0}

	//--- dirhndl - handle from prior svcIoOpen( O_ODF,...)
	//
	, {PAR_SET|REP_HEXV, 4 , IOPAR_DIRHNDL , 0}

	//--- dirofs - directory entry index (offset = val * 32 for FAT-XX)
	//
	, {PAR_GET|REP_HEXV, 4 , IOPAR_DIROFS	, 0}

//	, {PAR_GET|REP_HEXV, 4 , IOPAR_DEVSTS  , 0}
//	, {PAR_GET|REP_DECV, 4 , IOPAR_LENGTH  , 0}
//	, {PAR_GET|REP_DECV, 4 , IOPAR_REQALLOC, 0}
//	, {PAR_GET|REP_HEXV, 8 , IOPAR_CDATE   , 0}
//	, {PAR_GET|REP_HEXV, 8 , IOPAR_MDATE   , 0}
//	, {PAR_GET|REP_HEXV, 8 , IOPAR_ADATE   , 0}
//	, {PAR_GET|REP_HEXV, 4 , IOPAR_PROT    , 0}
//	, {PAR_GET|REP_STR , 0 , IOPAR_OWNER   , owner, 0, 36, 36}

	, { 0 }
};


//
// Desc:
//  Open a directory stream
//
// Inputs:
//	name		Path name for target directory (can have trailing '\')
//
DIR * opendir( const char * name )
{
	long	rtn;
	int		n;
	DIR		*pDir = NULL;
//#	char	*cp;
	char	szDir[256];

	//
	// Find free DIR instance
	//
	for( n = 0; n < 10; n++ )
	{
		pDir	= g_dirs + n;

		if( ! pDir->m_fInUse )
			break;
	}

	if( n >= 10 )
		return( (DIR *)0 );		// All busy...

	strcpy( szDir, name );
	n = strlen( szDir );
	if( n > 0 && szDir[n-1] != '\\' )
	{
		szDir[n++] = '\\';
		szDir[n]   = '\0';
	}

	if( (rtn = svcIoOpen( O_ODF, szDir, &g_opendir)) < 0 )
	{
		//@@@ printf( "? opendir:open(O_ODF) rtn=%d\n", rtn );
		return( (DIR *)0 );
	}

	//@@@ printf( "%% opendir: rtn=%d g_fname=%.40s\n", rtn, g_fname );


	pDir->m_fInUse	= 1;
	pDir->m_hDir	= rtn;
	pDir->m_errno	= 0;

	return( pDir );
}



DIRENT	g_de;

//
//
//
DIRENT * readdir( DIR * pDir )
{
	int		n;
	int		nLen;
	long	rtn;
	unsigned char	* p = (unsigned char *)g_filespec;
	char	*pszName	= g_de.d_name;

	if( !pDir )
		return 0;

	memset( &g_de, 0x00, sizeof(g_de) );

	g_fileparm.dirhndl.value = pDir->m_hDir;

//    rtn = svcIoDevParm( IOPAR_FILSPEC, "*.*", &g_fileparm );
	//
	//@@@@
	//
 
    rtn = svcIoDevParm( 0, "*.*", &g_fileparm );
	if( rtn < 0 )
	{	
		switch( rtn )
		{
		case ER_FILNF:		// File not found
			pDir->m_errno = rtn;
			return( 0 );

		case ER_FILAD:		// Access denied
			pDir->m_errno = rtn;
			fprintf( stderr, "? Access denied\n" );
			return( 0 );
			//break;

		default:
			pDir->m_errno = rtn;
			fprintf( stderr, "? readdir: IoDevParm rtn=%d\n", rtn );
			return( 0 );
		}

	}

	nLen = g_fileparm.filspec.strlen;

	for( ; *p; )
	{
		switch( *p )
		{
		case 0xF5:		//@@@ Attrib
//@@@			printf( " Attr: %04X\n", *(short *)(p+1) );
			p += 3;
			continue;

		case 0xF2:		//@@@ File name
			p++;
			nLen = 0;
			while( *p && *p < 0x80 )
			{
				*pszName++ = (char)(*p++);
				nLen++;
			}
			*pszName = 0;
			g_de.d_reclen = (unsigned short)nLen;
			continue;

		default:
			n = 0;
			do
			{
				if( *p >= 0x20 && *p < 0x7F )
					fprintf( stderr, "  %c", *p );
				else
					fprintf( stderr, " %02X", *p );

				if( (n & 0x0F) == 0x0F )
					fprintf( stderr, "\n" );
				p++;
				n++;
			} while( *p && *p < 0x80 );

			fprintf( stderr, "\n" );
			continue;
		}
	}

	return( &g_de );
}


//
// Desc:
//  Close directory stream
//
int closedir( DIR * pDir )
{
	long		rtn = 0;

	if( pDir )
	{
		//@@@ CLOSE DIR!!!

		if( pDir->m_hDir >= 0 )
		{
			rtn = svcIoClose( pDir->m_hDir, 0 );	//@@@ see svcIoClose flags
			if( rtn < 0 )
				fprintf( stderr, "? closedir: %d\n", rtn );
		}
		
		pDir->m_fInUse = 0;
	}

	return( (int)rtn );
}

