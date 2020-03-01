// ++++
// This software is in the public domain.  It may be freely copied and used
// for whatever purpose you see fit, including commerical uses.  Anyone
// modifying this software may claim ownership of the modifications, but not
// the complete derived code.  It would be appreciated if the authors were
// told what this software is being used for, but this is not a requirement.

//   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

//@@@??? No "#include <BASELINE.H>" nor _FCNTL_H definition!
//@@@    Added 00Aug01 CEY...

#ifndef _FCNTL_H
# define _FCNTL_H

# include <_prolog.h>

#define O_BINARY	0x0000		//@@@ Added 00Jul03/CEY for compatibility...
#define S_IREAD		0x0000		//@@@
#define S_IWRITE	0x0000

#define SH_DENYNO	0x0000
#define SH_DENYWR	0x0000
#define SH_DENYRD	0x0000
#define SH_DENYWT	0x0000

#define sopen(aName,aFlags,aShare,aPmode) open(aName,aFlags,aPmode)

#define O_EXCL   0x1000		/* If O_CREAT is set, fail if file exists */
#define O_TRUNC  0x0800		/* Truncate file to zero length if it exists */
#define O_CREAT  0x0400		/* Create new file if it does not exist */
#define O_APPENDX 0x0200		/* Append to end of file */
#define O_RDWR   0x0002		/* Open for reading and writing */
#define O_WRONLY 0x0001		/* Open for writing only */
#define O_RDONLY 0x0000		/* Open for reading only */

int  open(char *name, int oflags, int mode);
int  creat(char *name, int mode);
int  read(int handle, char *buffer, int count);
int  write(int handle, char *buffer, int count);
long lseek(int handle, long position, int mode);
long tell( int handle );
long filelength( int handle );
int chsize( int handle, long newlen );

int  close(int handle);

//@@@@@ needs to be defined

int	dup( int fd );


# include <_epilog.h>

#endif		// _FNCTL_H
