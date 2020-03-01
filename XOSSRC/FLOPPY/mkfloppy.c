// Program to write a floppy disk image from a file to a floppy disk
//   under DOS/Windows.

// This program is written for Watcom C version 10.5

#include <STDIO.H>
#include <STDLIB.H>
#include <CTYPE.H>
#include <STRING.H>
#include <BIOS.H>
#include <ERRNO.H>
#include <IO.H>
#include <FCNTL.H>
#include <I86.H>

#define TRUE  1
#define FALSE 0

int   floppy;
int   file;
long  handle;
long  length;
long  flpsize;
int   trksize;
int   trksecs;
int   track;
long  rtn;

char *buffer;
char  cmdbufr[128];

char *msgtbl[] =
{   "No error indicated",		// 0x00
    "Invalid function",			// 0x01
    "Address mark not found",		// 0x02
    "Disk is write protected",		// 0x03
    "Sector not found",			// 0x04
    NULL,				// 0x05
    "Diskette change line active",	// 0x06
    NULL,				// 0x07
    "DMA overrun on operation",		// 0x08
    "Data boundary error",		// 0x09
    NULL,				// 0x0A
    NULL,				// 0x0B
    "Media type not found",		// 0x0C
    NULL,				// 0x0D
    NULL,				// 0x0E
    NULL,				// 0x0F
    "Uncorrectable data error"		// 0x10
};
char msg20[] = "General controller failure";
char msg40[] = "Seek operation failed";
char msg80[] = "Timeout";

char flpchars[] = {0x4E, 4, 'U','N','I','T','T','Y','P','E', 0, 0, 0, 0};

char xos;

char *getcmd(char *prompt);

union REGPACK regs;


long _pascal (*svcIoClose)(long hndl, long bits) =
	(long _pascal (*)())0xED00020C;
long _pascal (*svcIoDevChar)(long hdnl, void *parm) =
	(long _pascal (*)())0xED000220;
long _pascal (*svcIoOpen)(long bits, char *name, void *parm) =
	(long _pascal (*)())0xED000204;
long _pascal (*svcIoOutBlock)(long hndl, char *bufr, long cnt) =
	(long _pascal (*)())0xED000244;
long _pascal (*svcSysErrMsg)(long code, long fmt, long x1, char *bufr) =
	(long _pascal (*)())0xED00011C;
void xosfail(long code, char *msg);


void main(void)

{
    char *cmd;
    int   cnt;
    int   size;
    char  chr;

    puts("MKFLOPPY version 1.0");
    xos = (*((long *)0x00000500) == 0x2A534F44L);
    puts("This program copies a disk file which contains a floppy disk "
	    "image to a\nfloppy disk.  It is intended to be used to create a "
	    "bootable XOS system\ndisk.  WARNING: The current contents of the "
	    "floppy disk will be lost.");
    while (TRUE)
    {
	cmd = getcmd("File to read");
	if ((file = open(cmd, O_RDONLY|O_BINARY)) < 0)
	{
	    printf("? Cannot open file %s\n  %s\n", cmd, strerror(errno));
	    continue;
	}
	break;
    }
    while (TRUE)
    {
	cmd = getcmd((xos) ? "Floppy disk [A (F0:) or B (F1:)]" :
		"Floppy disk [A or B]");
	if (((chr = toupper(*cmd++)) != 'A' && chr != 'B') ||
		(cmd[0] != 0 && (cmd[0] != ':' || cmd[1] != 0)))
	{
	    puts("? Must specify A or B");
	    continue;
	}
	break;
    }
    floppy = chr - 'A';
    if ((length = filelength(file)) < 1)
    {
	printf("? Error getting length of input file\n  %s\n", strerror(errno));
	exit (1);
    }
    if (xos)
    {
	if ((handle = svcIoOpen(0x402L, (floppy) ? "_F1:" : "_F0:", 0L))
		< 0)			// O$IN|O$PHYS
	    xosfail(handle, "Can not open floppy disk");

	if ((rtn = svcIoDevChar(handle, flpchars)) < 0)
	    xosfail(rtn, "Error getting floppy characteristics");

	if (*((long *)(flpchars+10)) == 0x354448)
	{
	    flpsize = 1228800;
	    trksize = 7680;
	    cmd = "1.2MB";
	}
	else if (*((long *)(flpchars+10)) == 0x334448)
	{
	    flpsize = 1474560;
	    trksize = 9216;
	    cmd = "1.44MB";
	}
	else
	{
	    puts("? Unsupported floppy type, must be 1.2MB or 1.44MB");
	    exit(1);
	}
    }
    else				// If running under DOS
    {
	memset(&regs, 0, sizeof(union REGPACK));
	regs.w.ax = 0x0800;
	regs.w.dx = floppy;
	intr(0x13, &regs);
	regs.w.bx &= 0x07;
	if (regs.w.bx == 2)		// 1.2M floppy?
	{
	    flpsize = 1228800;
	    trksize = 7680;
	    trksecs = 0x0300 + 15;
	    cmd = "1.2MB";
	}
	else if (regs.w.bx == 4)	// 1.44M floppy?
	{
	    flpsize = 1474560;
	    trksize = 9216;
	    trksecs = 0x0300 + 18;
	    cmd = "1.44MB";
	}
	else
	{
	    puts("? Unsupported floppy type, must be 1.2MB or 1.44MB");
	    exit(1);
	}
    }
    printf("\nHave %s floppy disk\n", cmd);
    if (length != flpsize)
    {
	puts("\n? File size does not match floppy capacity");
	exit(1);
    }
    if ((buffer = malloc(trksize)) == NULL)
    {
	puts("\n? Cannot allocate memory for track buffer");
	exit (1);
    }
    track = 0x0001;
    cnt = 160;				// Number of tracks
    do
    {
	if ((size = read(file, buffer, trksize)) != trksize)
	{
	    if (trksize >= 0)
		puts("\n? Incomplete read from disk file");
	    else
		printf("\n? Error reading disk file\n  %s\n", strerror(errno));
	    exit(1);
	}
	if (xos)
	{
	    if ((rtn = svcIoOutBlock(handle, buffer, (long)trksize)) < 0)
		xosfail(rtn, "Error writing to floppy disk");
	}
	else
	{
	    regs.w.ax = trksecs;
	    regs.w.cx = track;
	    regs.w.dx = floppy;
	    regs.w.bx = (int)buffer;
	    regs.w.es = ((long)buffer) >> 16;
	    intr(0x13, &regs);
	    if (regs.w.flags & 0x01)
	    {
		cnt = regs.w.ax >> 8;
		if (cnt <= 0x10)
		    cmd = msgtbl[cnt];
		else if (cnt == 0x20)
		    cmd = msg20;
		else if (cnt == 0x40)
		    cmd = msg40;
		else if (cnt == 0x80)
		    cmd = msg80;
		else
		    cmd = NULL;
		fputs("? Error writing to floppy disk\n  ", stdout);
		if (cmd != NULL)
		    puts(cmd);
		else
		    printf("Error code = 0x%2.2X\n", cnt);
		exit (1);
	    }
	}
	floppy ^= 0x0100;
	if ((floppy & 0x0100) == 0)
	{
	    track += 0x0100;
	    if (track & 0x0100)
	    {
		putchar('.');
		fflush(stdout);
	    }
	}
    } while (--cnt > 0);
    if (xos && (rtn = svcIoClose(handle, 0)) < 0)
	xosfail(rtn, "Error closing floppy disk");
    puts("\nFloppy image write completed");
}

char *getcmd(
    char *prompt)

{
    char *pnt;
    int   cnt;
    char  chr;

    while (TRUE)
    {
	printf("\n%s: ", prompt);
	if (fgets(cmdbufr, sizeof(cmdbufr) - 1, stdin) == NULL)
	{
	    puts("? Error reading command input");
	    continue;
	}
	pnt = cmdbufr;
	cnt = sizeof(cmdbufr);
	while (--cnt > 0 && (chr = *pnt) != 0 && chr != '\n')
	    pnt++;
	*pnt = 0;
	pnt = cmdbufr;
	while ((chr = *pnt) != 0 && isspace(chr))
	    pnt++;
	if (*pnt != 0)
	    return (pnt);
    }
}

void xosfail(
    long  code,
    char *msg)

{
    char buffer[100];

    svcSysErrMsg(code, 0x03L, 0L, buffer);
    printf("? %s\n  %s\n", msg, buffer);
    exit(1);
}
