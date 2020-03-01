//*************************************************************
// SWITCH.C - Routines to read and handle command-line switches
//*************************************************************

//++++
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

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include "XMAKE.H"

#define CMDBUFR 200		/* Size of command buffer */

char **pntpnt;
int    pntcnt;
char  *cmdpnt;			/* Command buffer pointer */
char   cmdbfr[CMDBUFR+2];	/* Command buffer */

/*
 * Option table
 */

void o_help(void),    o_error(void),  o_precious(void), o_debug(void),
     o_default(void), o_fname(void),  o_print(void),    o_ignore(void),
     o_cont(void),    o_silent(void), o_nop(void),      o_env(void),
     o_noexec(void),  o_question(void);

struct opttbl opttable[] =
{   {{"?         "}, o_help},		// Display XMAKE's help screen
    {{"HELP      "}, o_help},
    {{"H         "}, o_help},
    {{"T         "}, o_error},
    {{"F         "}, o_fname},
    {{"FILE      "}, o_fname},
    {{"P         "}, o_print},
    {{"PRINT     "}, o_print},
    {{"PRECIOUS  "}, o_precious},
    {{"I         "}, o_ignore},
    {{"IGNORE    "}, o_ignore},
/*    {{"K         "}, o_cont},	*/	/* Continue, Unix style */
/*    {{"C         "}, o_cont}, */
/*    {{"CONTINUE  "}, o_cont}, */
    {{"S         "}, o_silent},
    {{"SILENT    "}, o_silent},
    {{"R         "}, o_nop},		/* No internal rules */
    {{"N         "}, o_noexec},
    {{"NOEXEC    "}, o_noexec},
    {{"B         "}, o_nop},		/* Old makefile Compatability mode */
    {{"E         "}, o_env},
    {{"ENVIROMENT"}, o_env},
    {{"M         "}, o_nop},
    {{"D         "}, o_debug},
    {{"DEBUG     "}, o_debug},
    {{"DEFAULT   "}, o_default},
    {{"Q         "}, o_question},
    {{"QUESTION  "}, o_question}
};
int optsize = sizeof(opttable)/sizeof(struct opttbl);

/*
 * Text for help options
 */

/*
 * Function to process XMAKE command lines
 */

char *switchparse(argc, argv)
int    argc;			/* Argument count */
char **argv;			/* Argument array */
{
    register char *target;

    if (argc >= 2)			/* Do we have a command line now? */
    {
        cmdpnt = argv[1];
        pntpnt = &argv[2];
        pntcnt = argc - 2;
    }
    else
	return (0);			/* No - do default make */
    while (peekchar())
    {
	chkoptn();			/* Check for option before file name */
	target = getfsp(NULL);		/* Get file specification */
	chkoptn();			/* Check for option after file name */
    }
    return (target);			/* Return the target name */
}

/*
 * Function to check for option
 */

void chkoptn(void)
{
    register char   chr, *pnt;
    register short  cnt;
    register struct opttbl *optpnt;
             char   symbuf[256];

    for (;;)
    {
	skipsp();			/* Skip leading whitespace */
	if (peekchar() != '-')		/* Start of option? */
	    return;			/* No - finished here */
	++cmdpnt;			/* Yes */
        pnt = symbuf;			/* Get option name in symbuf */
	strcpy(symbuf, "          "),
        cnt = 10;
	while ((chr=peekchar()) != 0 && (isalpha(chr) || chr == '?'))
	{
            ++cmdpnt;
            if (--cnt >= 0)
                *pnt++ = toupper(chr);
        }
        if (cnt >= 0 && (optpnt=(struct opttbl *)srchtbl(symbuf, opttable, optsize)) != 0)
	    (*optpnt->op_rout)();
        if (cnt < 0 || !optpnt)
	{
	    fprintf(stderr, "? XMAKE: Illegal option -%s\n", symbuf);
	    exit(1);			// Error - fail
	}
    }
}

/*
 * Function to search for an option
 */

struct opttbl *srchtbl(name, opttable, optsize)
char          *name;		/* Option to search for          */
struct opttbl *opttable;	/* Pointer to table of options   */
int            optsize;		/* Number of entries in opttable */
{
    while (--optsize >= 0)
    {
	if (strcmp(name, opttable->op_name) == 0)
	    return (opttable);	/* If match found */
	opttable += 1;
    }
    return (0);
}

/*
 * Function to get file specification for option
 */

char *optfile(dftext)
char *dftext;			/* Default extension */
{
    if (peekchar() == ':')		/* Do we have a file spec next? */
    {					/* Yes */
        ++cmdpnt;
	return (getfsp(dftext));
    }
    else
	return (0);			/* No */
}


/*
 * Function to get file specification
 */

char *getfsp(dftext)
char *dftext;			/* Default extension */
{
    register int    size;
    register short  cnt;
             short  haveext;
    register char   chr;
    register char  *pnt1;
    register char  *pnt2;
    register char  *blk;
             char   fspbuf[513];

    skipsp();				/* Skip leading whitespace */
    if ((chr = peekchar()) == 0 || chr == '-') /* If nothing there */
	return (0);			/* Return nothing */
    haveext = FALSE;
    cnt = 512;				/* Copy file specification */
    pnt1 = fspbuf;
    while ((chr=peekchar()) != 0 && chr != ',' && !isspace(chr))
    {
	++cmdpnt;
	if (chr == '.')
	    haveext = TRUE;
	--cnt;
	if (cnt == 0)
	    fatal("Target specification too long", fspbuf, 1);
	*pnt1++ = chr;
    }
    *pnt1 = '\0';
    size = 513 - cnt;
    if (dftext != 0 && !haveext)
	size += strlen(dftext);
    blk = getmemory((long)size);	/* Get memory for file name block */
    pnt1 = blk;				/* Point to place to put file spec */
    pnt2 = fspbuf;
    while (*pnt2)			/* Copy file spec */
	*pnt1++ = toupper(*pnt2++);
    if (dftext != 0 && !haveext)
    {
	pnt2 = dftext;
	while (*pnt2)
	    *pnt1++ = toupper(*pnt2++);
    }
    *pnt1 = 0;
    skipsp();
    return (blk);
}

/*
 * Function to peek at next command character
 */

char peekchar(void)
{
    if (!*cmdpnt && --pntcnt >= 0)	/* At end of this string? */
    {
	cmdpnt = *pntpnt++;		/* Setup to use next string */
	return (' ');			/* Return space this time */
    }
    if (*cmdpnt == '\\' && *(cmdpnt + 1) == 0) /* Need next line? */
    {
	getcx("*+");			/* Yes - get it */
	return (peekchar());
    }
    else
    {
	if (*cmdpnt == '\n')
	    *cmdpnt = '\0';
	return (*cmdpnt);
    }
}

/*
 * Function to input command line
 */

void getcx(prompt)
char *prompt;			/* Prompt string */
{
    fputs(prompt, stderr);		/* Output prompt */
    fgets(cmdbfr, CMDBUFR, stdin);	/* Get command input */
    cmdpnt = cmdbfr;			/* Reset pointer */
    pntcnt = 0;
}

/*
 * Function to skip whitespace in command line
 */

void skipsp(void)
{
    while (isspace(peekchar()))		/* Skip whitespace */
	++cmdpnt;
}

/*
 * Function to report error if bad command
 */

void badcmd(void)
{
    fatal("Command error", (char *)NULL, 1);
}

void o_cont(void)
{
    OPT_cont = TRUE;
}

void o_question(void)
{
    OPT_question = OPT_silent = OPT_noexec = TRUE;
}

void o_fname(void)
{
    char c;

    if (peekchar() == ':')
	++cmdpnt;
    if ((c = peekchar()) != 0 && !isspace(c) && c != '-')
        xmakex = getfsp(".MAK"); /* Get the filename */
    else
	makefile = stdin;
}

void o_help(void)
{
    fputs("The following command-line options are available:\n", stderr);
    fputs("  -?\t\tDisplay this help message.\n  -(H)ELP\tDisplay this help message\n", stderr);
    fputs("  -(F)ILE:makefile\tDefine description file name.  A null filename\
\t\t\t\tdenotes the standred input.\n", stderr);
    fputs("  -(P)RINT\tPrint out the Makefile after external macros have been expanded.\n", stderr);
    fputs("  -(I)GNORE\tIgnore error codes returned by invoked commands.\n", stderr);
/*    fputs("  -(C)ONTINUE\tAbandon work on the current entry on error, but \
continue on\n\t\t\other branches that do not depend on that entry.\n", stderr);
    fputs("  -K\t\tUnix form of -CONTINUE\n", stderr); */
    fputs("  -(S)ILENT\tSilent mode.  Do not print command lines before executing.\n", stderr);
    fputs("  -(N)OEXEC\tNo execute mode.  Print commands, but do not \
execute them.\n", stderr);
    fputs("  -(E)NVIROMENT\tDo not re-define enviroment variables from within the Makefile. (NIY)\n", stderr);
    fputs("  -(D)EBUG\tDebug mode.  Print out detailed information while processing.\n", stderr);
    fputs("  -(Q)UESTION\tQuestion.  Returns a zero or non-zero status code \
depending on\n\t\twhether the target file is or is not up-to-date.\n\n", stderr);
    exit(1);
}

void o_ignore(void)
{
    OPT_ignore = TRUE;
}

void o_silent(void)
{
    OPT_silent = TRUE;
}

void o_debug(void)
{
    OPT_debug = TRUE;
}

void o_env(void)
{
    OPT_envlock = TRUE;
}

void o_precious(void)
{
    OPT_precious = TRUE;
}

void o_nop(void)		/* This is a do-nothing */
{
}

void o_error(void)
{
    fprintf(stderr, "? XMAKE: Touch option not supported by this "
	    "implementation\n");
}

void o_default(void)
{
    /* WRITE THE REST OF THIS LATER */
}

void o_noexec(void)		/* No execute, just display commands */
{
    OPT_noexec = TRUE;
}

void o_print(void)	/* Print out all macro definitions and target specs */
{
    OPT_print = TRUE;
}
