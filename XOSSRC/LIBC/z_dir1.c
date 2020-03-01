#include <CTYPE.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <TIME.H>
#include <XOSERR.H>
#include <XOSTRM.H>
#include <XOSRTN.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSTIME.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <DIRSCAN.H>
#include <GLOBAL.H>
#include <XOSSTR.H>

#include "z_dir1.h"

//
// Forward declarations
//
int z_DirScan( DIRSCANDATA * pDir );

void z_errormsg( const char *arg, long  code);
long	g_dwFileLen;

typedef struct filedescp FILEDESCP;

//
// File description data structure
//
struct filedescp
{
	FILEDESCP *next;		// Address of next file block
    FILEDESCP *sort;		// Used by heapsort
    uchar      filattr;		// File attributes
    uchar      xxx;			// Reserved
    long       error;		// Error code
    char       data[2];		// Start of data area
};

char		g_szLastPath[256];

//
// Stuff needed by dirscan
//
char owner[36];

//
// Define file attribute bits
//
#if 0
# define A_NORMAL	0x80		// Normal file
# define A_ARCH  	0x20		// Archive bit (set if file has been modified)
# define A_DIRECT	0x10		// Directory
# define A_LABEL 	0x08		// Volume label
# define A_SYSTEM	0x04		// System file
# define A_HIDDEN	0x02		// Hidden file
# define A_RDONLY	0x01		// Read only file
#endif

struct fileparm g_fileparm =
{
	{PAR_SET|REP_HEXV, 4 , IOPAR_FILOPTN
		, FO_VOLNAME
		| FO_NODENUM
		| FO_NODENAME
		| FO_RVOLNAME
		| FO_PATHNAME
		| FO_ATTR
		//| FO_NOPREFIX
		}
	, {PAR_GET|REP_STR , 0 , IOPAR_FILSPEC , NULL, 0, 0, 0}
	, {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR
		, A_NORMAL | A_DIRECT | A_RDONLY }

	, {PAR_GET|REP_HEXV, 2 , IOPAR_FILATTR , 0}
	, {PAR_SET|REP_HEXV, 4 , IOPAR_DIRHNDL , 0}

	, {PAR_GET|REP_HEXV, 4 , IOPAR_DEVSTS  , 0}
	, {PAR_GET|REP_DECV, 4 , IOPAR_LENGTH  , 0}
	, {PAR_GET|REP_DECV, 4 , IOPAR_REQALLOC, 0}
	, {PAR_GET|REP_HEXV, 8 , IOPAR_CDATE   , 0}
	, {PAR_GET|REP_HEXV, 8 , IOPAR_MDATE   , 0}
	, {PAR_GET|REP_HEXV, 8 , IOPAR_ADATE   , 0}
	, {PAR_GET|REP_HEXV, 4 , IOPAR_PROT    , 0}
	, {PAR_GET|REP_STR , 0 , IOPAR_OWNER   , owner, 0, 36, 36}
};

//#typedef struct fileparm		FILEPARM;

//
//
//
DIRSCANDATA g_dsd =
{
	(DIRSCANPL *)&g_fileparm,
							// parmlist    - Address of parameter list
    FALSE,					// repeat      - TRUE if should do repeated search
//    TRUE,					// repeat      - TRUE if should do repeated search
    TRUE,					// dfltwildext - TRUE if want wildcard as default
    QFNC_DEVPARM,			// function    - Value for low byte of QAB_FUNC
    DSSORT_NONE,			// sort        - Directory sort order
    NULL,               	// hvellip     - Function called when ellipsis specified
    (int (*)(DIRSCANDATA *))z_DirScan,
							// func        - Function called for each file
							//				 matched
    z_errormsg				// error       - Function called on error
};



//***********************************************
// Function: z_errormsg - Display XOS error message
// Returned: Nothing
//***********************************************

void z_errormsg(
    const char *arg,
    long  code
	)
{
    char buffer[80];            // Buffer to receive error message

    if (code != 0)
    {
        svcSysErrMsg(code, 3, buffer);	// Get error message
        fprintf( stderr, "\n? DIR: %s%s%s\n"
			, buffer
			, (arg[0] != '\0') ? "; " : ""
			, arg
			);			// Output error message
    }
    else
        fprintf( stderr, "\n? z_dir1: %s\n", arg );

//    errdone = TRUE;
}

//**************************************************************
// Function: procfile - Function called by dirscan for each file
// Returned: TRUE if should continue, FALSE if should terminate
//**************************************************************

// This function is called by dirscan for each file found.

int z_DirEllipsis( DIRSCANDATA * pDir )
{
	char		szAttr[10];

	printf( "\n********** ... ****************\n" );
	printf( " Path  =%.40s\n", pDir->pathname	);

	printf( " Size   =%6d Attr=%02X File=%.40s plen=%d\n"
			, g_fileparm.length.value
			, g_fileparm.filattr.value
			, pDir->filename
			, pDir->pathnamelen
			);

	printf( " PBase  =%.40s\n", pDir->pathnamebase );

	sprintf( szAttr, "%c%c%c%c%c%c%c\n"
		, (pDir->attr & A_LABEL ) ? 'V' : '-'
		, (pDir->attr & A_NORMAL) ? 'N' : '-'
		, (pDir->attr & A_DIRECT) ? 'D' : '-'
		, (pDir->attr & A_SYSTEM) ? 'S' : '-'
		, (pDir->attr & A_HIDDEN) ? 'H' : '-'
		, (pDir->attr & A_RDONLY) ? 'R' : '-'
		, (pDir->attr & A_ARCH  ) ? 'M' : '-'
		);

	printf( " Attr  =%s Owner=%-5.5s File=%.40s\n"
		, szAttr
		, owner
		, pDir->filename
		);


    return (TRUE);
}

//
// Desc:
//  Callback
//
int z_DirScan( DIRSCANDATA * pDir )
{
	char		szAttr[10];

	if( strnicmp( pDir->pathname, g_szLastPath, pDir->pathnamelen ) != 0 )
	{
		printf( "Path=%.40s\n"
			, pDir->pathname
			);

		strncpy( g_szLastPath, pDir->pathname, pDir->pathnamelen );
		g_szLastPath[ sizeof(g_szLastPath) - 1 ] = 0;
	}

	sprintf( szAttr, "%c%c%c%c%c%c%c"
		, (pDir->attr & A_LABEL ) ? 'V' : '-'
		, (pDir->attr & A_NORMAL) ? 'N' : '-'
		, (pDir->attr & A_DIRECT) ? 'D' : '-'
		, (pDir->attr & A_SYSTEM) ? 'S' : '-'
		, (pDir->attr & A_HIDDEN) ? 'H' : '-'
		, (pDir->attr & A_RDONLY) ? 'R' : '-'
		, (pDir->attr & A_ARCH  ) ? 'M' : '-'
		);

	printf( "%s %-5.5s %.40s\n"
		, szAttr
		, owner
		, pDir->filename
		);

    return (TRUE);
}

//
// Desc:
//
void z_Dir( char * pszMask, Z_DIRFN pfn )
{

	g_fileparm.filoptn.value
		= FO_VOLNAME
		| FO_NODENUM
		| FO_NODENAME
		| FO_RVOLNAME
		| FO_PATHNAME
		| FO_ATTR
		| FO_FILENAME
		| FO_VERSION
		//| FO_FILEDOS

		//| FO_NOPREFIX
		;
	g_fileparm.length.value = 0;


    // Call DIRSCAN to get the names

	//g_dsd.parmlist = (DIRSCANPL *)&g_fileparm;

	if( pfn )
		g_dsd.func	= pfn;
	else
		g_dsd.func	= z_DirScan;

	//# g_dsd.hvellip	= z_DirEllipsis;

	//@ owner[0] = 0;

	strcpy( g_szLastPath, "X&1@$$$Unlikely" );

	dirscan( pszMask, &g_dsd );	// Scan the directory
}

//////////////////////////////////////////////////////////////////////
// z_mkdir
//////////////////////////////////////////////////////////////////////

struct				// Paramters for device information
{
    byte4_parm  options;
    lngstr_parm spec;
    char        end;
} parm_mkdir =
{
    {(PAR_SET|REP_HEXV), 4, IOPAR_FILOPTN, FO_NOPREFIX|FO_VOLNAME|FO_PATHNAME|
            FO_FILENAME},
    {(PAR_GET|REP_STR),  0, IOPAR_FILSPEC, NULL, 0, FILESPCSIZE, 0},
    0
};

char mkdir_rtnname[FILESPCSIZE+4] = "";

int z_mkdir( char * pszName )
{
	int		nStat = 0;
	int		nLen  = strlen( pszName );
    char	namebfr[512];

	if( nLen <= 0 )
		return( -1 );

	strcpy( namebfr, pszName );
	if( namebfr[nLen-1] != '\\' )
	{
		namebfr[nLen++] = '\\';
		namebfr[nLen  ] = '\0';
	}


    parm_mkdir.spec.buffer = (char *)mkdir_rtnname;

#if 0	//@@@
    if (gcp_dosdrive)
        parm_open.options.value = FO_NOPREFIX|FO_DOSNAME|FO_PATHNAME|
		FO_FILENAME;
#endif

    if( (nStat = svcIoOpen(O_CREATE|O_ODF|O_FAILEX, namebfr, &parm_mkdir)) < 0)
    {
		printf( "? z_mkdir error=%d\n", nStat );
    }
	return( nStat );
}
