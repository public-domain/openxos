//
// Name:
//	share.h
//
#if 0	//@@@@
extern "C" int chsize( int fd, long lSize );
extern "C" long filelength( int fd );


//
// POSIX Error codes...??? @@@
//
#define EINVAL		1
#define EACCES		2

#define LK_NBLCK	0x0001
#define LK_UNLCK	0x0002

extern "C" int locking( int fd, int /* LK_NBLCK */, int num_bytes ) ;
#endif


#ifndef _SHARE_H
# define _SHARE_H

# include <_prolog.h>

int chsize( int fd, long lSize );
long filelength( int fd );


//
// POSIX Error codes...??? @@@
//
#define EINVAL		1
#define EACCES		2

#define LK_NBLCK	0x0001
#define LK_UNLCK	0x0002

int locking( int fd, int /* LK_NBLCK */, int num_bytes ) ;

# include <_epilog.h>
#endif
