//
// Name:
//	dirent.h
//
// History:
//--------------------------------------------------------------------
// 00Aug04 CEY	o) Creation (POSIX.1 definitions)
//

//
// Desc:
//	Definition of a single directory entry/item
//
struct dirent
{
	long			d_ino;
	long			d_off;

	unsigned short	d_reclen;		// ???
	unsigned char	d_type;			// ???
	char			d_fill1;		// Keep 4-byte alignment

	char			d_name[256];	// FQFN
};


struct _dirstream_s
{
	int		m_fInUse;		// In-use flag
	int		m_hDir;			// XOS I/O handle
	int		m_errno;		// Lase XOS Error code
};


typedef struct _dirstream_s		DIR;
typedef struct dirent			DIRENT;

//
// Desc:
//  Open a directory stream
//
// Inputs:
//	name	ASCIZ path for directory to scan
//
// Returns:
//	NULL	Failed. errno updated. (XOS Codes?)
//
DIR * opendir( const char * name );

//
// Desc:
//  Read next directory entry
//
// Inputs:
//	pDir	Pointer to user supplied DIR buf. (_dirent_s)
//
// Returns:
//	NULL		End of directory or invalid DIR/handle
//
DIRENT * readdir( DIR * pDir );

//
// Desc:
//	Close a directory stream
//
int closedir( DIR * pDir );

