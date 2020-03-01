//------------------------------------------------------------------------------
//
//  shellext.h - External variables for the shell processor
// 
//  Copyright (c) 1989-1994 Saguaro Software, Ltd.  All rights reserved.
//  Copyright (c) 1994-1995 Allegro Systems, Ltd.  All rights reserved.
//
// 
//  Written by John R. Goltz and Bruce R. Nevins
// 
//  Edit History:
//  -------------
//  04/19/89(brn) - Add alias command variables
//  12/05/89(brn) - Remove screenmode variable
// 
//------------------------------------------------------------------------------

#ifndef __SHELLEXT_H__

extern char    *xmempnt;
extern struct   firstblk
	       *argbase;
extern char    *firstarg;
extern int      blkcnt;
extern char    *blkpnt;
extern struct   cprocs cproc[];
extern long     childcnt;
extern struct   devlist devlist;
extern struct   runparm runparm;	/* Parameter block for RUN SVC */
extern long     linecnt;
extern char    *linepnt;
extern char     dftdev[];
extern char     inpredir[];
extern char     outredir[];
extern char     cline[];
extern char     simple;
extern char     loggedin;
extern char     keyword[];
extern char     prgname[];
extern char     runname[];
extern char     filname[];	        /* Name of file where input is to come from */
extern char     echo;		        /* TRUE if BATCH commands are to be echoed */
extern char     bat_ctlc;		/* TRUE if stop batch */
extern char     batch;		        /* FALSE  if not running batch */
extern char     skipprompt;	        /* Skip printing the next prompt */
extern struct   firstblk *interncmd;	/* Internal command buffer */
extern char     inlinecmd;	        /* Process inline command next not file */
extern char     singlearg;	        /* TRUE if one arg */
extern FILE    *cmdin;		        /* FD for command input */
extern FILE    *cmdout;			/* FD for command output */
extern FILE    *cmderr;			/* FD for command error */
extern struct   firstblk
               *hstfrst;                /* Pointer to command history list beginning */
extern struct   firstblk
               *hstlast;                /* Pointer to command history list end */
extern struct   firstblk
               *hsttmp;                 /* Temp pointer into history list */
extern unsigned long
                histlimit;		/* Default size of history list */
extern int	histcnt;
extern int	histlow;
extern int	topseen;	        /* Flag for top of command list */
extern int	botseen;	        /* Flag for bottom of command list */
extern struct   firstblk
               *aliasfrst;              /* Pointer to command alias list beginning */
extern struct   firstblk
               *aliaslast;              /* Pointer to command alias list end */
extern struct   firstblk
	       *aliastmp;		/* Temp pointer into alias list */
extern int	pipeseen;		/* Flag if doing a pipe */
extern int	doprompt(short lintyp);
extern int	cmdhstsrch(void);	/* Search command history */
extern int      aliascnt;               /* Number of alias' defined */
extern int      major_version;
extern int      minor_version;
extern char	*date_version;
extern char     firstbuf[];             /* Buffer for first arg */
extern char	*alias_defaults[];	/* Alias default table */

#endif

