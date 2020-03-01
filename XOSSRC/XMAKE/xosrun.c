//********************************************************************
// XOSRUN.C - C function to run a program as a child process under XOS
//********************************************************************

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
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSERRMSG.H>
#include "XMAKE.H"

int fexist(const char *s);

static char *target_name = NULL;	/* Name of target currently being built */

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
} devlist =		/* Device list for run */
{   {STDIN , STDIN , -1},
    {STDOUT, STDOUT, -1},
    {STDERR, STDERR, -1},
    {STDTRM, STDTRM, -1}
};

struct runparm 
{
    lngstr_parm arglist;
    lngstr_parm devlist;
    char        end;
} runparm =
{
    {(PAR_SET|REP_STR), 0, IOPAR_RUNCMDTAIL, NULL, 0, 1, 0},
    {(PAR_SET|REP_STR), 0, IOPAR_RUNDEVLIST, (char *)&devlist, 0,
            sizeof(devlist), 0},
    0
};

type_qab runqab =
{   RFNC_WAIT|RFNC_RUN,		/* qab_func    - Function */
    0,				/* qab_status  - Returned status */
    0,				/* qab_error   - Error code */
    0,				/* qab_amount  - Process ID */
    0,				/* qab_handle  - Device handle */
    0,				/* qab_vector  - Vector for interrupt */
    0, 0,			/*             - Reserved */
    R_CHILDTERM|R_ACSENV|R_CHGENV,
				/* qab_option  - Options or command */
    0,				/* qab_count   - Count */
    NULL, 0,			/* qab_buffer1 - Pointer to file spec */
    NULL, 0,			/* qab_buffer2 - Unused */
    &runparm, 0			/* qab_parm    - Pointer to parameter area */
};

void cntlcsrv(void)
{
    svcSchCtlCDone();
    if (target_name != NULL && OPT_precious == FALSE &&
	    svcIoDelete(0, target_name, NULL) >= 0)
	fprintf(stderr, "\n? XMAKE: Removing %s on Ctrl-C abort\n",
		target_name);
    else
	fputs("\n? XMAKE: Ctrl-C abort\n", stderr);
    exit(1);
}

void runprog(filespec, command, tree)
char      *filespec;
char      *command;
TREE_NODE *tree;

{
    char         *extpnt;
    char          execname[512]; // Space to keep actual name used in exec
    unsigned long errcode;
             long rtn;		// Error code returned by program

    *execname = 0;
    if (!instr(filespec, ":") && !instr(filespec, "\\") && !instr(filespec, "/"))
	strcpy(execname, "CMD:");
    if (strlen(filespec) + 5 < 512)
	strcat(execname, filespec); /* Append filename */
    else
    {
	fprintf(stderr, "? XMAKE: Program pathname \"%s\" is too long\n",
		filespec);
	exit(1);
    }
    if (!instr(filespec, "."))
    {
	if ((rtn = strlen(execname)) + 6 < 512)
	    extpnt = strcpy(execname + rtn, ".RUN");
	else
	{
	    fprintf(stderr, "? XMAKE: Program pathname \"%s\" is too long\n", filespec);
	    exit(1);
	}
    }
    else
	extpnt = NULL;
    if (!fexist(execname))		/* If not found */
    {
	fprintf(stderr, "? XMAKE: No such program, %s\n", filespec);
	exit(1);
    }
    else
    {
	if ((OPT_noexec && !OPT_question) || !(OPT_silent || CMD_silent))
	{
	    putchar('\t');
	    puts(command);
	}

	if (!OPT_noexec)			/* We may not want to exec */
	{
	    target_name = tree->file->fname;
	    devlist.dlstdin.dst  = STDIN;
	    devlist.dlstdout.dst = STDOUT;
	    devlist.dlstderr.dst = STDERR;
	    devlist.dlstdtrm.dst = STDTRM;
	    runparm.arglist.buffer = command;
	    runparm.arglist.bfrlen = runparm.arglist.strlen = strlen(command);
	    runqab.qab_buffer1     = execname;

	    if ((rtn = svcIoRun(&runqab)) < 0 || (rtn = runqab.qab_error) < 0)
	    {
		if (extpnt != NULL && rtn == ER_FILNF)
		{
		    strcpy(extpnt, ".EXE");
		    if ((rtn = svcIoRun(&runqab)) < 0 ||
			    (rtn = runqab.qab_error) < 0)
		    {
			if (rtn == ER_FILNF)
			{
			    strcpy(extpnt, ".COM");
			    if ((rtn = svcIoRun(&runqab)) < 0 ||
				(rtn = runqab.qab_error) < 0)
			    {
				if (rtn == ER_FILNF)
				{
				    strcpy(extpnt, ".BAT");
				    if ((rtn = svcIoRun(&runqab)) >= 0)
					    rtn = runqab.qab_error;
				}
			    }
			}
		    }
		}
		if (rtn < 0)
		{
		    if (!OPT_precious)		/* If we don't want to keep this */
		    {
			if (svcIoDelete(0, tree->file->fname, NULL) >= 0)
			    fprintf(stderr, "XMAKE: Removing %s",
				    tree->file->fname);
		    }
		    femsg("XMAKE", rtn, execname);
		}
	    }
	    errcode = runqab.qab_amount & 0xFFFFFFL;
	    if (runqab.qab_amount & 0x800000L)
		errcode |= 0xFF000000L;
	    if (errcode != 0)
	    {
		if (!OPT_ignore && !CMD_ignore)
		{
		    fprintf(stderr, "? XMAKE: %s terminated with error status"
                            " %ld\n", command, errcode);
		    if (!OPT_precious)	/* If we don't want to keep this */
		    {
			if (svcIoDelete(0, tree->file->fname, NULL) >= 0)
			    fprintf(stderr, "XMAKE: Removing %s\n",
                                    tree->file->fname);
		    }
		    exit(1);
		}
	    }
	    target_name = NULL;		/* Finished processing this target */

	}
    }
}

int cmdcopy(dest, source, depend, srcsize, destsize)
register char *dest;		// Pointer to destination for string
register unsigned char *source;	// Pointer to source for string
TREE_NODE *depend;		// Pointer to current dependency
int        srcsize;		// Pointer to number of chars in source
int        destsize;		// Pointer to max number of chars in dest
{
    char mfnc;			// Function for special macros
    char sep;			// Filename separator for special macros
    char c;			// Temporary character
    int  i;			// String counter
    int  dlx;			// Temporary place to keep destsize for loop
    int  dev, path, name, ext;	// Flags for various parts of filename
    int  once; 			// Flag for only first name of list of names
    int  all;			// Flag for all or only out of date filenames
    int  looped;		// Counter for single time around macro loop
    TREE_NODE *namelist;	// Temporary pointer into dependency list
    char *namepnt;		// Temporary pointer into filename
    char *x;			// Very Temporary pointer into filename

    while (*source && *source != CR)
    {
	if (srcsize == 0)
	    break;		/* If there's nothing left to copy */
	if (destsize == 0)
	    return (EOF);    	/* Return an overflow error */

	if (*source < 0x80)	/* If not the beginning of a macro */
	{
	    if (srcsize != -1)	/* Check if we're tracking this size */
		--srcsize;
	    *dest++ = *source++; /* Copy a character */
	    --destsize;		/* Reduce size of available destination */
	    continue;		/* Go around and process next character */
	}
/*
 * Here with special macro expansion
 */
	mfnc = *source++ & 0x7f; /* Get macro function */
	once = dev = path = name = ext = FALSE;
	while (*source < 0x80)
	{
	    if (srcsize != -1)
		--srcsize;
	    switch (toupper(*source++))
	    {
		case '1' : once = TRUE; break;
		case 'D' : dev  = TRUE; break;
		case 'P' : path = TRUE; break;
		case 'N' : name = TRUE; break;
		case 'E' : ext  = TRUE; break;
		default  :
		{
		    char z;
		    --source;
		    if (*source < ' ')    /* If non-printing character */
		    {
			z = '^';
			*source = *source + '@';
		    }
		    else
			z = ' ';
		    fprintf(stderr, "? XMAKE: Illegal modifier '%c%c' for "
			    "internal macro $%c\n", z, *source, mfnc);
		    exit(1);
		    break;
		}
	    }
	}
	sep = *source++ & 0x7f;	 /* Get filename separator char */
	if (srcsize != -1)
	    --srcsize;
	if (dev == FALSE && path == FALSE && name == FALSE && ext == FALSE)
	    dev = path = name = ext = TRUE; /* If none specified, copy all */
	switch (mfnc)		/* Selection actions for each macro */
	{
	    case '*' :
		namelist = depend->tree_list;
		all = TRUE;
		break;
	    case '@' :
		namelist = depend;
		all = TRUE;
		once = TRUE;
		break;
	    case '?' :
		namelist = depend->tree_list;
		all = FALSE;
		break;
	}
	looped = 0;		/* Counter to once around feature */
	while (namelist != (TREE_NODE *)NULL && (once == FALSE || looped == 0) && destsize > 0)
	{			/* Until out of names to process */
	    dlx = destsize;	/* Store this for later */
	    if (all == FALSE && depend->file->fdatetime > namelist->file->fdatetime)
	    {
		namelist = namelist->next;	/* Advance to next node */
		continue;			/* And try again */
	    }
	    namepnt = namelist->file->fname;	/* Get ptr to name */
	    if (inchr(namepnt, ':') && dev == TRUE) /* If device name */
	    {
		while ((c = *namepnt++) != ':' && destsize > 0 && c != 0)
		{
		    *dest++ = c;		/* Copy device name */
		    --destsize;
		}
		*dest++ = c;			/* Copy trailing colon */
		--destsize;
	    }
	    else if (dev != TRUE && inchr(namepnt, ':') && *namepnt != 0)
		while (*namepnt++ != ':')	/* Otherwise ignore */
		    ;
	    if ((inchr(namepnt, '/') || inchr(namepnt, '\\')) && *namepnt != 0 && path == TRUE)
	    {
		x = namepnt;
		i = 0;
		while (*x != 0)		/* Advance to end of string */
		{
		    ++x;
		    ++i;
		}
		while (i && *x != '\\' && *x != '/')
		{
		    --i;
		    --x;		/* Find first slash */
		}
		if (i && destsize >= i)
		{			/* Copy pathname to dest */
		    ++i;
		    strncpy(dest, namepnt, i);
		    *(dest + i) = 0; /* Must manually terminate */
		    dest += i;
		    destsize -= i;
		    namepnt = x + 1;
		}
		else if (x != namepnt)
		    return (EOF);	/* Indicate string overflow */
	    }
	    else if (path == FALSE && *namepnt != 0 && (inchr(namepnt, '/') || inchr(namepnt, '\\')))
	    {
		while (*namepnt++ != 0) /* Otherwise ignore pathname */
		    ;
		while (*namepnt != '\\' && *namepnt != '/')
		    --namepnt;
		++namepnt;
	    }
	    if (name == TRUE && *namepnt != 0) /* If filename body wanted */
	    {
		while ((c = *namepnt++) != '.' && c != 0 && destsize > 0)
		{
		    *dest++ = c;
		    --destsize;
		}
		--namepnt;			/* Leave it at '.' */
	    }
	    else if (name == FALSE && *namepnt != 0) /* Ignore filename body */
	    {
		while ((c = *namepnt++) != '.' && c != 0)
		    ;
		--namepnt;
	    }
	    if (ext == TRUE && *namepnt != 0) /* If filename ext wanted */
		while (*namepnt != 0 && destsize > 0)
		{
		    *dest++ = *namepnt++;
		    --destsize;
		}
	    if (destsize > 0 && destsize != dlx)
	    {		/* If not out of space and we generated something */
		*dest++ = sep;	/* Store the separator char at the end */
		--destsize;
	    }
	    namelist = namelist->next;	/* Advance to next name */
	    looped   = 1;		/* Indicate around loop */
	}
	if (destsize != dlx)	/* If we generated any filenames */
	{
	    --dest;			/* Ignore last separator */
	    ++destsize;
	}
    }
    *dest = 0;
    return (0);
}
