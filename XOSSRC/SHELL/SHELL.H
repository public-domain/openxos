//------------------------------------------------------------------------------
//
//  shell.h - Parameter file for SHELL
//
//  Copyright (c) 1984-1994 Saguaro Software, Ltd.  All rights reserved.
//  Copyright (c) 1994-1995 Allegro Systems, Ltd.  All rights reserved.
//
//
//  Written by John R. Goltz and Bruce R. Nevins
//
//  Edit History:
//  -------------
//  05/17/89(brn) - Add support for cmdver
//  12/05/89(brn) - remove reference to intscreen, remove errno
//  03/28/94(brn) - define blkungetc and add saved char to block header
//  04/01/94(brn) - Remove definition for getsingle
//
//------------------------------------------------------------------------------

#ifndef __SHELL_H__

#define TRUE  1
#define FALSE 0

#define FIRST 0			/* First prompt line */
#define CONT  1			/* Continued prompt line */

#define LINESIZE 512		/* Maximum length of single command line */
#define MAXARGS  100		/* Maximum number of command arguments */
#define MAXCHILD 4		/* Maximum number of children */
#define CMDLVL   3		/* Maximum command file levels */
#define HISTDEFAULT 20		/* Default size of history buffer */
#define HISTMAX 1024		/* Maximum size of the history buffer */
#define TRMCLASS 0x4D5254L

#define uchar  unsigned char
#define ushort unsigned short
#define uint   unsigned int
#define ulong  unsigned long

extern char *_clib_mempnt;	/* Memory allocation pointer */
extern char *_clib_memtop;	/* Top of memory pointer */

/*
 * Define command dispatch structure
 */
#define CD(cmd) (int (*)(void))cmd

struct command
{
    char *cmd;
    int (*fnc)(void);
};
/*
 * Define memory block structure
 */
#define BLOCKSIZE 64		/* Total size of memory block */

struct nextblk
{
    struct nextblk *next;
    char   text[BLOCKSIZE - sizeof(void *)];
				/* Array of data in next block */
};

struct firsthdr
{
    struct nextblk *next;	/* Pointer to next block, NULL if last */
    struct firstblk *nextlst;	/* Next list of blocks */
    struct firstblk *lastlst;	/* Last list of blocks */
    char  *txtptr;		/* Pointer to next char in text */
    long  txtcount;		/* Number of bytes left */
    long  totalcnt;		/* Number of bytes in this string */
    char  saved;		/* The saved character from blkungetc */
};

struct firstblk
{
    struct firsthdr a;	/* First block header */
    char   text[BLOCKSIZE - sizeof(struct firsthdr)];
				/* Array of data in first block */
};

struct intdata
{   long  svdEDI;
    long  svdESI;
    long  svdEBP;
    long  svdESP;
    long  svdEBX;
    long  svdEDX;
    long  svdECX;
    long  svdEAX;
    long  intEIP;
    long  intCS;
    long  intEFR;
    long  intGS;
    long  intFS;
    long  intES;
    long  intDS;
    short intcnt;
    short intnum;
    long  pid;
    long  term;
    long  pEIP;
    long  pCS;
    long  pEFR;
    long  pos;
    long  pseg;
    long  data;
};

/*
 * Define structure which contains child process data
 */

struct cprocs
{   long c_state;		/* Process state */
    long c_pid;			/* Process ID */
    long c_trm;			/* Termination type */
    long c_status;		/* Termination status */
    long c_pEFR;		/* Child's EFR */
    long c_pEIP;		/* Child's EIP */
    long c_pCS;			/* Child's CS */
    long c_data;		/* Data item */
    long c_offset;		/* Offset part of address */
    long c_segment;		/* Segment part of address */
};

/*
 * Define values for c_state
 */

#define C_DET    -1		/* Process is running detached */
#define C_NONE    0		/* Process does not exist */
#define C_RUN     1		/* Process is running */
#define C_KILLED  2		/* Process has just been killed */

struct devitem
{   long src;
    long dst;
    long cmd;
    long xxx;
};

struct devlist
{   struct devitem dlstdin;
    struct devitem dlstdout;
    struct devitem dlstderr;
    struct devitem dlstdtrm;
    struct devitem xxx;
};

/*
 * Define structures for QFNC_RUN parameters
 */

struct runparm
{   lngstr_parm arglist;
    lngstr_parm devlist;
    char        end;
};

struct pathparm
{   byte4_parm  filoptn;
    lngstr_parm filspec;
    char        end;
};

/*
 * Declare prototypes for shell
 */

struct firstblk *blkcpy(struct firstblk *outblk, struct firstblk *inblk);
struct firstblk *blkcat(struct firstblk *outblk, struct firstblk *inblk);
char   blkgetc(struct firstblk *blkptr);
int    blkungetc(register struct firstblk *blkptr, char chr);
void   blkinic(struct firstblk *blkptr);
long   blklen(struct firstblk *inblk);
struct firstblk *blkputc(struct firstblk *blkptr, char newchr);
long   blkputs(register struct firstblk *inblk, FILE *stream);
int    blkstrcmp(struct firstblk *blk1, struct firstblk *blk2);
int    blkstrncmp(struct firstblk *blk1, struct firstblk *blk2, long num);
int    blkstrnicmp(struct firstblk *blk1, struct firstblk *blk2, long num);
void   childgone(struct intdata intdata);
void   childreport(struct cprocs *pnt);
char   chkcmdc(void);
int    cmdalias(void);
int    cmdalssrch(void);
int    cmdcd(void);
int    cmdchngdrv(char *drive);
int    cmdcls(void);
void   cmdexit(void);
int    cmdhist(void);
int    cmdhstsrch(void);
void   cmdlogout(void);
int    cmdmode(void);
int    cmdreset(void);
char  *cmdspace(int size);
int    cmdver(void);
void   cmnderr(char *name, long code);
void   cntlcsrv(void);
int    doprompt(short lintyp);
int    fileerr(char *fsp, long code);
struct firstblk *getblock(void);
int    getcmd(void);
char   getcmdc(void);
long   gethexval(char *str);
int    getkeyword(struct firstblk *blkptr);
long   getline(void);
ulong  getsyschar(char *str);
struct firstblk *giveblock(struct firstblk *blkptr);
struct firstblk *givechain(struct firstblk *blkptr);
void   hungup(void);
void   inicmd(char *env);
void   killdet(void);
void   prompt(int prmtype);
void   putcmdc(char chr);
void   resetscrn(void);
int    srchatom(char *atom, struct command *table, short size);
int    strinb(struct firstblk *bpnt);
int    do_function_key(void);
int    errpause(void);

#endif

