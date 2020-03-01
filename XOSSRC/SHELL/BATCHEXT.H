//------------------------------------------------------------------------------
//
//  batchext.h - External variables for the Batch processor
//
//  Copyright (c) 1987-1994 Saguaro Software, Ltd.  All rights reserved.
//  Copyright (c) 1994-1995 Allegro Systems, Ltd.  All rights reserved.
//
//
//  Written by Bruce R. Nevins
//
//  Edit History:
//  -------------
//  05/13/91(brn) - Add definition for extended batch
//  06/14/92(brn) - Add definition for local batch
//  05/03/94(brn) - Add definition for Pause batch on error
//
//------------------------------------------------------------------------------

#ifndef __BATCHEXT_H__

#define LABELSIZE 8			// Number of bytes in a label

extern int	detached;		// TRUE if running under the batch server
extern int     argcounter;		// Batch arg counter
extern char   **argpointer;		// Batch arg pointers
extern struct firstblk *expndarg;	// Expanded input line
extern struct devlist devlist;		// Device list for run
extern struct devlist devlist_det;	// Device list for run when detached
extern long    errrtn;			// Error return from child
extern char    tempnoecho;		// TRUE if only this command
					//  is not to be echoed
extern char    tempnohist;		// TRUE if this command is not to
					//  go in history
extern int     extend_batch;		// Default is no extended batch
extern int     ctrlc_prompt;		// Display prompt in batch when Control C
extern int     local_batch;		// Don't exit shell at end of batch file
extern int     bat_return;		// Return from batch file
extern int     error_pause;		// Pause batch on error return not 0

//
// Declare function prototypes
//
int  batargs(char *args, char **argv[]);
int  batbat(char *batfile);
int  batcall(void);
void batcmd(void);
int  batecho(void);
int  batfor(void);
int  batgoto(void);
int  batif(void);
int  batjump(void);
int  batlabel(void);
int  batnkeyw(register char *outstr, register struct firstblk *inblk);
int  batpause(void);
int  batrem(void);
int  batreturn(void);
int  batshift(void);
int  batopt(void);
struct firstblk *batxlate(struct firstblk *dststr, struct firstblk *srcstr);
void synerr(char *errstr, char *secondstr);
void dointcmd(struct firstblk *cmdstr);

#endif

