//------------------------------------------------------------------------------
// GETCMD.C - Low level functions to process commands
//
// Written by John R. Goltz and Bruce R. Nevins
//
// Edit History:
// -------------
// 08/23/89(brn) - Add redirect error on append >>&
// 11/02/89(brn) - Fix path to return drive as well like (Ugh) dos
// 12/05/89(brn) - Fix changed TRUNC to TRUNCA
// 03/02/90(brn) - Take out special XOS disk name code and use ENV variable
// 02/28/91(brn) - Make default prompt work like dos
// 06/06/91(brn) - fix printdosdrive and printdospath to not use the
//                 firstarg pointer.
// 06/16/91(brn) - Support keypad up and down arrow keys
// 10/09/91(jrg) - Change to support Z: instead of DSK: for DOS compatibility
// 03/22/92(brn) - Add use of global.c module like all the utils
// 03/24/92(brn) - Allow white space between redir operator and filename
//               - Fix getline to handle function keys after input
// 08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
// 03/31/94(brn) - Chnage getsingle to getch
// 23May95 (fpj) - Changed code to output to cmderr instead of cmdout.
// 11/02/95(brn) - Fixed getcmd to only store % when part of a environment
//			string lookup.  Continuation line "%"s are not saved.
// 08/21/96(brn) - Fixed getcmd to store % at end of line when its part of
//			an environment string
// 08/27/96(brn) - Add fflush if detached process
//------------------------------------------------------------------------------*

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

#include <CTYPE.H>
#include <ERRNO.H>
#include <SETJMP.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <TIME.H>
#include <XOSERR.H>
#include <XOSSVC.H>
#include <XOS.H>
#include <XOSTIME.H>

#include <GLOBAL.H>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"
#include "KEYS.H"

int   fgetstr(char *str, int size, FILE *stream);

static FILE *doredir(FILE **c_fd, long *place, char *mode);
static void printpath(void);
static void printdosdrive(void);

static char default_prompt[] = "$n$g";
/*
 * Function to input command line(s)
 */

int getcmd(void)
{
    register char  chr;
    int gotfirst;               // Flag if we got first string
    int envseen = FALSE;	// Handle % for environment string

    gotfirst = FALSE;
    if (argbase != NULL)                /* Make sure there is a block to free */
        argbase = givechain(argbase);   /* Give up our old chain of memory */
                                        /*  blocks */
    if (getline() == 0)                 /* Read command line */
        return (FALSE);


/*
 * Main Command loop
 */
                                        /* Read in a command */
    while (linecnt != 0)
    {
		if ((chr = getcmdc()) == '\0' && (linecnt != 0))
			if (!do_function_key()) 	/* If null Must be a function key */
				return (FALSE);

        switch (chr)
        {
         case '<':						// Input redirection?
			putcmdc(chr);				// Save each character
            if (!doredir(&cmdin, &devlist.dlstdin.src, "r"))
                return (FALSE);
            devlist.dlstdin.cmd = O_IN;
            break;

         case '>':						// Output redirection?
			putcmdc(chr);				// Save each character
            if ((chr = chkcmdc()) == '>') /* Append output? */
            {
                putcmdc(chr);
                getcmdc();              // Get rid of matching character
                if ((chr = chkcmdc()) == '&') // Standard error redirection?
                {
                    putcmdc(chr);
                    getcmdc();          /* Get rid of matching character */
                    if (!doredir(&cmderr, &devlist.dlstderr.src, "a+"))
                        return (FALSE);
                    devlist.dlstdout.src = devlist.dlstderr.src & 0x7FFFFFFFL;
                    devlist.dlstdout.cmd = O_CREATE|O_IN|O_OUT|O_APPEND;
                    devlist.dlstderr.cmd = O_CREATE|O_IN|O_OUT|O_APPEND;
					cmdout = cmderr;
                }
                else
                {
                    if (!doredir(&cmdout, &devlist.dlstdout.src, "a+"))
                        return (FALSE);
                    devlist.dlstdout.cmd = O_CREATE|O_IN|O_OUT|O_APPEND;
                }
                break;
            }
            else if (chr == '&')        /* Standard error redirection? */
            {
                putcmdc(chr);
                getcmdc();              /* Get rid of matching character */
                if (!doredir(&cmderr, &devlist.dlstderr.src, "w"))
                    return (FALSE);

				// Redirect standard out too
				// Must have high bit clear so we don't delete it out
				//   from under RUN.
 
                devlist.dlstdout.src = devlist.dlstderr.src & 0x7FFFFFFFL;
                devlist.dlstdout.cmd = O_CREATE|O_IN|O_OUT|O_APPEND;
                devlist.dlstderr.cmd = O_CREATE|O_IN|O_OUT|O_APPEND;
				cmdout = cmderr;
                break;

            }
            else if (!doredir(&cmdout, &devlist.dlstdout.src, "w"))
                return (FALSE);
            devlist.dlstdout.cmd = O_CREATE|O_IN|O_OUT|O_APPEND;
            break;

#if     0               // FIXME when pipes work! - FPJ, 23May95

         case '|':						// Pipe ?
			putcmdc(chr);				// Save each character
            if ((chr = chkcmdc()) == '&') // Standard error redirection?
            {
                putcmdc(chr);
                getcmdc();          /* Get rid of matching character */
                if (!gotfirst)
                {
                    fprintf(cmderr, "? %s: Need a program to pipe from\n",
                            prgname);
					if (detached)
						fflush(cmderr);
                    linecnt = 0;
                    return (FALSE);         
                }

                if (chkcmdc() == '\0')
                {
                    fprintf(cmderr, "? %s: Need a program to pipe to\n",
                            prgname);
					if (detached)
						fflush(cmderr);
                    linecnt = 0;
                    return (FALSE);         
                }
                pipeseen = TRUE;
                putcmdc('\0');              /* Make sure have null at end */
                return (TRUE);              /* All done */
            }
            else
            {
                if (!gotfirst)
                {
                    fprintf(cmderr, "? %s: Need a program to pipe from\n",
                            prgname);
					if (detached)
						fflush(cmderr);
                    linecnt = 0;
                    return (FALSE);         
                }

                if (chkcmdc() == '\0')
                {
                    fprintf(cmderr, "? %s: Need a program to pipe to\n",
                            prgname);
					if (detached)
						fflush(cmderr);
                    linecnt = 0;
                    return (FALSE);         
                }
                pipeseen = TRUE;
                putcmdc('\0');			// Make sure have null at end
                return (TRUE);			// All done
            }
#endif

         case '"':						// Quoted argument?
			putcmdc(chr);				// Save each character
			if (chkcmdc() == '"')
			{
				chr = getcmdc();
				putcmdc(chr);
				break;
			}
            while ((chr = getcmdc()) != '\0')
            {
				if (chr == '"')
				{
					if (chkcmdc() != '"')
						break;
					putcmdc(chr);
					chr = getcmdc();
				}
                putcmdc(chr);
                if (chr == '%')
                {
                    chr = getcmdc();
                    if (chr == '\0')
					{
						if (envseen)
						{
							envseen = FALSE;
							putcmdc(chr); // Save the % for the
						}				  //  environment string
                        break;
					}
					if (envseen)
						envseen = FALSE;
					else
						envseen = TRUE;

                    putcmdc(chr);
                }
            }
			putcmdc(chr);
            if (chr != '"')
            {
                fprintf(cmderr, "? %s: Unterminated quote, adding...\n",
                         prgname);
				if (detached)
					fflush(cmderr);
                putcmdc('"');			// Add missing quote
                putcmdc('\0');
                return (TRUE);			// Send it on anyway
            }
            break;

         case '%':						// Continue the line?
            if ((chr = getcmdc()) == '\0' && envseen == FALSE)
            {
                doprompt(CONT);
                getline();
            }
            else
			{
				if (envseen)
					envseen = FALSE;
				else
					envseen = TRUE;

				putcmdc('%');			// Put a % in the line for the
										//   environment string
                putcmdc(chr);           // Store character no matter what
			}

            break;

         default:
			putcmdc(chr);				// Save each character
            gotfirst = TRUE;
        }
    }
    putcmdc('\0');			/* Make sure have null at end */
    return (TRUE);			/* All done */
}

/*
 * Function to store recalled command in terminal input buffer
 */

int strinb(
    struct firstblk *bpnt)

{
    long rtn;

    blkinic(bpnt);
    linecnt = blklen(bpnt);		/* Get size of internal command */
    if (linecnt > (LINESIZE-2))
    {
        linecnt = 0;
        fprintf(cmderr, "? %s: Internal command line to large\n", prgname);
		if (detached)
			fflush(cmderr);
        return (FALSE);
    }
    linecnt = 0;
    linepnt = cline;
    while ((*linepnt++ = blkgetc(bpnt)) != '\0')
        linecnt++;						// Copy internal command to standard
										//   line buffer
    if ((rtn = svcTrmWrtInB(STDTRM, cline, linecnt)) < 0)
    {
        svcSysErrMsg(rtn, 3, cline);	// Get error message
        fprintf(cmderr, "? %s: Error writing to terminal buffer:\n"
                "         %s\n", prgname, cline);
		if (detached)
			fflush(cmderr);
        return (FALSE);
    }
    return (TRUE);
}

/*
 * Function to read in single command line
 */

long getline(void)
{
    linepnt = cline;
    if (linecnt != 0)
    {
        if (pipeseen)
        {
            pipeseen = FALSE;
            return (linecnt);
        }
        else
            linecnt = 0;
    }

    if (inlinecmd)
    {
        inlinecmd = FALSE;
        blkinic(interncmd);
        linecnt = blklen(interncmd);    /* Get size of internal command */
        if (linecnt > LINESIZE)
        {
            linecnt = 0;
            fprintf(cmderr, "? %s: Internal command line too large\n",
                    prgname);
			if (detached)
				fflush(cmderr);
            return (linecnt);
        }
        while ((*linepnt++ = blkgetc(interncmd)) != '\0')
            ;                           /* Copy internal command to standard */
                                        /*  line buffer */
        *--linepnt = '\n';              /* Make new line be at end */
        *++linepnt = '\0';              /* Just in case mark end with null */
        linecnt++;                      /* Count the new character */
        linepnt = cline;                /* Point to beginning */
    }
    else
    {
		if (cmdin == stdin)
		{
            if ((linecnt=svcIoInBlock(STDIN, cline, LINESIZE)) < 0)
            {
                errno = -(int)linecnt;
                linecnt = 0;
                cline[0] = '\0';
            }
            else if (linecnt >= 2)
            {
                if ((cline[(int)linecnt - 2] & 0x7F) == '\r')
                {
                    cline[(int)linecnt - 2] = '\n';
                    --linecnt;
                }
            }
            if (cline[(int)linecnt - 4] == '\0')
                cline[(int)linecnt++] = '\n';        /* This is to fool getcmdc */

            cline[(int)linecnt] = '\0';
        }
		else
		{
            linecnt = fgetstr(cline, LINESIZE, cmdin);
		}
    }
    if ((linecnt < 0) && (errno != -ER_EOF))
    {
        linecnt = 0;
        fileerr(filname, -errno);
    }
    while (isspace(*linepnt))
    {
		linepnt++;
		linecnt--;
    }
    return (linecnt);
}

/*
 * Function to get next command character
 */

char getcmdc(void)

{
    char chr;

    if (linecnt == 0)
        return ('\0');

    linecnt--;
    chr = *linepnt++ & 0x7F;

    if ((linecnt == 0) && (chr != '\n'))
    {
		if (cmdin == stdin)
		{
            if ((linecnt=svcIoInBlock(STDIN, cline, LINESIZE)) < 0)
            {
                errno = -(int)linecnt;
                linecnt = 0;
                cline[0] = '\0';
            }
            else if (linecnt >= 2)
            {
                if ((cline[(int)linecnt - 2] & 0x7F) == '\r')
                {
                    cline[(int)linecnt - 2] = '\n';
                    --linecnt;
                }
            }
            if ((cline[(int)linecnt - 4] == '\0') && linecnt == 4)
                cline[(int)linecnt++] = '\n';        /* This is to fool getcmdc */

            cline[(int)linecnt] = '\0';
        }
		else
		{
            linecnt = fgetstr(cline, LINESIZE, cmdin);
		}
        linepnt = cline;                /* Point to beginning */

		if ((linecnt < 0) && (errno != -ER_EOF))
		{
            linecnt = 0;
            fileerr(filname, -errno);
		}
    }
    if (chr == '\n')
    {
        chr = '\0';
        linecnt = 0;
    }
    return (chr & 0x7F);
}

/*
 * Function to check next command character
 */

char chkcmdc(void)

{
    char chr;

    if (linecnt == 0)
        return ('\0');
    chr = *linepnt;
    if ((chr & 0x7F) == '\n')
    {
        chr = '\0';
        linecnt = 0;
    }
    return (chr & 0x7F);
}

/*
 * Function to do file redirection
 */

static FILE *doredir(
    FILE  **c_fd,		/* C library file descriptor */
    long  *xos_fd,		/* Location for XOS file descriptor */
    char  *mode)		/* Pointer to fopen mode text */

{
    register  char  chr;
    register  char *pnt;
    char      errstr[90];
    char      fspbuf[FILESPCSIZE];
    long      cnt;
    FILE     *rtn_file;		/* Pointer to new open file */

    cnt = FILESPCSIZE;
    pnt = fspbuf;

/*
 * Get the file name to open
 */
    while (((chr = getcmdc()) != 0) && isspace(chr) && --cnt >= 0)
    {
        putcmdc(chr);
    }
    putcmdc(chr);
    *pnt++ = chr;

    while (((chr = getcmdc()) != 0) && !isspace(chr) && --cnt >= 0)
    {
        putcmdc(chr);
        *pnt++ = chr;
    }
    if (cnt < 0)
    {
        fprintf(cmderr, "? %s: File specification for redirection is too long\n",
                prgname);
	if (detached)
	    fflush(cmderr);
        return (NULL);
    }
    *pnt = '\0';

/*
 * Open the redirection file
 */
    if ((rtn_file = fopen(fspbuf, mode)) == NULL)
    {
        svcSysErrMsg(-errno, 3, errstr); /* Error - get message */
        fprintf(cmderr, "? %s: Cannot open file (%s) for redirection\n           %s\n",
                prgname, fspbuf, errstr);
	if (detached)
	    fflush(cmderr);
    }
    else
    {
	*xos_fd = rtn_file->iob_handle;	/* Return XOS file descriptor */
	*c_fd = rtn_file;		/* Return C file descriptor */
    }

    return (rtn_file);			/* Return open file descriptor */
}

/*
 * Function to display IO error message - Always return FALSE
 */

int fileerr(fsp, code)
char *fsp;              /* Pointer to file name */
long  code;             /* Error code */

{
    long err_len;
    char errbuf[100];

    err_len = svcSysErrMsg(code, 3, errbuf);     /* Get error message */
    if (err_len < 0)
    {
	fprintf(cmderr, "\n? %s: fileerr, Could not get error message\n",
		prgname);
    }
    fprintf(cmderr, "\n? %s: %s, file %s\n", prgname, errbuf, fsp); /* Output it */
    if (detached)
	fflush(cmderr);
    return (FALSE);
}

/*
 * Function to store character into command string
 */

void putcmdc(
    char chr)

{
    argbase = blkputc(argbase, chr);
}

/*
 * prompt - Function to output prompt
 */

void prompt(
    int prmtype)

{
    char    *prmstring;			/* Pointer to prompt string */
    union
    {
        struct xos_ver
        {
            unsigned edit : 16;
            unsigned minor : 8;
            unsigned major : 8;
        } x_version;
        unsigned long x_long;
    } a;
    char     buffer[FILESPCSIZE];	/* Buffer for current path string */
    char     prmstr[FILESPCSIZE];	/* Pointer to prompt string */
    time_d   systime;			/* System time value */

    if (svcSysFindEnv(0, "PROMPT", NULL, prmstr, 256, NULL) < 0)
    {
        prmstring = default_prompt;
    }
    else
    {
        prmstring = prmstr;
    }

    while (*prmstring != '\0')
    {
        if (*prmstring != '$')
            fputc(*(prmstring++), cmderr);
        else
        {
            switch (tolower(*(++prmstring)))
            {
            case '$':		/* Print a $ */
                fputc('$', cmderr);
                break;

            case 't':		/* Print the time */
                if (svcSysDateTime(T_GTDDTTM, &systime) < 0)
                    fprintf(cmderr, "? %s: Could not get system time\n",
                            prgname);
                ddt2str(buffer, "%H:%m:%s", (time_dz *)&systime);
                fprintf(cmderr, "%s", buffer);
		if (detached)
		    fflush(cmderr);
                break;

            case 'd':		/* Print the date */
                if (svcSysDateTime(T_GTDDTTM, (time_dz *)&systime) < 0)
                {
                    fprintf(cmderr, "? %s: Could not get system time\n",
                        prgname);
		    if (detached)
			fflush(cmderr);
                }
		else
		{
                    ddt2str(buffer, "%3w %D-%3m-%y", (time_dz *)&systime);
                    fprintf(cmderr, "%s", buffer);
		}
		if (detached)
		    fflush(cmderr);
                break;

            case 'p':		/* Print the current drive and path */
                printpath();
                break;

            case 'v':		/* Print the DOS version number */
                a.x_long = getsyschar("DOSVER");
    				/* Get DOS emulator version */
                fprintf(cmderr, "MS-DOS %d.%d emulation", a.x_version.major,
                        a.x_version.minor);
		if (detached)
		    fflush(cmderr);
                break;

            case 'y':		/* Print the XOS version number */
                a.x_long = getsyschar("XOSVER");
                fprintf(cmderr, "XOS version %d.%d.%d", a.x_version.major,
                        a.x_version.minor, a.x_version.edit);
		if (detached)
		    fflush(cmderr);
                break;

            case 'n':		/* Print current drive */
                printdosdrive();
                break;

            case 'g':		/* Print a > */
                fputc('>', cmderr);
                break;

            case 'l':		/* Print a < */
                fputc('<', cmderr);
                break;

            case 'b':		/* Print a | */
                fputc('|', cmderr);
                break;

            case 'q':		/* Print a = */
                fputc('=', cmderr);
                break;

            case 'h':		/* Erase previous character */
                fputs("\b \b", cmderr);
                break;

            case 'e':		/* Print an escape */
                fputc('\x1b', cmderr);
                break;

            case '_':		/* Print a CR LF */
                fputs("\r\n", cmderr);
                break;
            }
            prmstring++;		/* Skip this character */
        }
    }
    if (prmtype == CONT)
        fputc('-', cmderr);		/* Print continued prompt */
}

static void printdosdrive(void)
{
    type_qab pathqab;
    char     buffer[FILESPCSIZE];	/* Buffer for current path string */
    char    *tmpptr;
    extern struct pathparm pathparm;

    tmpptr = "Z:";
    if (gcp_dosdrive)
		pathparm.filoptn.value = FO_NOPREFIX|FO_DOSNAME|FO_NODENUM|
				FO_NODENAME|FO_RDOSNAME;
    else
		pathparm.filoptn.value = FO_NOPREFIX|FO_VOLNAME|FO_NODENUM|
				FO_NODENAME|FO_RVOLNAME;

    pathqab.qab_func = QFNC_WAIT|QFNC_PATH;
    pathqab.qab_vector = 0;
    pathqab.qab_option = 0;
    pathqab.qab_buffer1 = tmpptr;
    pathqab.qab_buffer2 = NULL;
    pathqab.qab_parm = &pathparm;
    pathparm.filspec.buffer = buffer;
    if (svcIoQueue(&pathqab) < 0 || pathqab.qab_error < 0)
        fputs("?", cmderr);		/* Do path SVC	*/
    else
        fputs(buffer, cmderr);		/* Print the device */
}

static void printpath(void)
{
    type_qab pathqab;
    char     buffer[FILESPCSIZE];	/* Buffer for current path string */
    char    *tmpptr;
    extern struct pathparm pathparm;

    tmpptr = "Z:";
    pathparm.filoptn.value = (gcp_dosdrive)? FO_NOPREFIX|FO_DOSNAME|FO_PATHNAME:
            FO_NOPREFIX|FO_VOLNAME|FO_PATHNAME;
    pathqab.qab_func = QFNC_WAIT|QFNC_PATH;
    pathqab.qab_vector = 0;
    pathqab.qab_option = 0;
    pathqab.qab_buffer1 = tmpptr;
    pathqab.qab_buffer2 = NULL;
    pathqab.qab_parm = &pathparm;
    pathparm.filspec.buffer = buffer;

    if (svcIoQueue(&pathqab) < 0 || pathqab.qab_error < 0)
        fputs("?:", cmderr);		/* Do path SVC	*/
    else
        fputs(buffer, cmderr);		/* Print the device and path */
}

/*
 * doprompt - Local function to read in single command line
 */

int doprompt(short lintyp)
{
    if (lintyp == FIRST)
    {
        if (echo && !skipprompt)
            prompt(FIRST);
    }
    else if (lintyp == CONT)
    {
        if (echo && !skipprompt)
            prompt(CONT);
    }
    else
    {
        fprintf(cmderr, "? %s: Bad value in doprompt() for linetype = %n\n",
                prgname, lintyp);
		if (detached)
			fflush(cmderr);
        return (FALSE);
    }

    skipprompt = FALSE;         /* Turn off skip prompt */
    return (TRUE);
}

/*
 * do_function_key - Handle all function keys
 */
int do_function_key(void)
{
    unsigned int keyboard_bits;
    int i;
    char numstr[6];
    char chr;

    {
        if (linecnt == 0)
            return (FALSE);		/* No - Just end of line */

        chr = getcmdc();                /* Get the function key */
        keyboard_bits = getcmdc();	/* Get Shift-Control-Alt bits */
        keyboard_bits += getcmdc() << 8;/* Get lock bits */
        switch (chr)
        {
/*
 * Copy previous command 1 char at a press
 */
        case F1:
            skipprompt = TRUE;
	    break;

/*
 * Copy previous command up to next typed char
 */
        case F2:
            skipprompt = TRUE;
	    break;

/*
 * Copy remainder of previous command
 */
        case F3:			/* DOS compatible command recall */

/*
 * Display previous command from history
 */
        case UPARROW:			/* Normal down arrow */
        case KEYPUPARROW:		/* Keypad down arrow */
            skipprompt = TRUE;
            if (hstfrst == NULL)
                return (FALSE);
            if (!botseen)
            {
                if ((hsttmp = hsttmp->a.lastlst) == NULL)
                {
                    hsttmp = hstfrst;
                    topseen = TRUE;
                }
                else
                    topseen = FALSE;
            }
            if (!strinb(hsttmp))
                return (FALSE);
            botseen = FALSE;
            break;

/*
 * Display next command from history
 */
        case DOWNARROW:			/* Normal down arrow */
        case KEYPDOWNARROW:		/* Keypad down arrow */
            skipprompt = TRUE;
            if (botseen || (hstfrst == NULL))
            return (FALSE);
            if ((hsttmp = hsttmp->a.nextlst) == NULL)
            {
                hsttmp = hstlast;
                botseen = TRUE;
                return (FALSE);
            }
            else
                botseen = FALSE;

            if (!strinb(hsttmp))
                return(FALSE);
            topseen = FALSE;
            break;

/*
 * delete template up to but not including next input character
 */
        case F4:
            skipprompt = TRUE;
	    break;

/*
 * Copy command to template but do not execute it
 */
        case F5:
            skipprompt = TRUE;
	    break;

/*
 * insert ^Z into input buffer
 */
        case F6:
            skipprompt = TRUE;
	    break;

/*
 * Show history list
 */
        case F7:
	    firstarg = NULL;
	    fputc('\n', cmderr);
	    cmdhist();
	    break;

/*
 * Search for previous input string, if none this time use last input
 *  and loop through all of history
 */
        case F8:
            skipprompt = TRUE;
	    break;

/*
 * Prompt for number of history command to fetch
 */
        case F9:
            skipprompt = TRUE;
	    fputs("Line number: ", cmderr);
	    numstr[0] = '\0';
	    i = 0;
	    while(TRUE)
	    {
		chr = (char)getch();	/* Get the character with no echo */
		if (isdigit(chr))
		{
		    if (i == (sizeof(numstr) - 1))
			continue;

		    numstr[i++] = chr;
		    numstr[i] = '\0';
		    fputc(chr, cmderr);
		}
		if (chr == '\r')
		    break;

		if (chr == '\b' && i > 0)
		{
		    numstr[--i] = '\0';
		    fputs("\b \b", cmderr);
		}
		if (chr == '\x1b')
		{
		    i = i + 13;
		    while (i != 0)
		    {
		        fputs("\b \b", cmderr);
			i--;
		    }
		}
	    }
	    if (i == 0)
		break;

	    firstarg = NULL;
	    strcpy(keyword, "!");
	    strcat(keyword, numstr);
	    cmdhstsrch();
	    break;

/*
 * Not used
 */
        case F10:
            skipprompt = TRUE;
	    break;

/*
 * Not used
 */
        case F11:
            skipprompt = TRUE;
	    break;

/*
 * Not used
 */
        case F12:
            skipprompt = TRUE;
	    break;

/*
 * Jump to oldest command in history
 */
		case PGUP:
		case KEYPPGUP:
            skipprompt = TRUE;

            if (hstfrst == NULL)
                break;

            hsttmp = hstfrst;
            if (!strinb(hsttmp))
                return(FALSE);
            topseen = TRUE;
	    botseen = FALSE;
	    break;

/*
 * Jump to newest command in history
 */
		case PGDN:
		case KEYPPGDN:
            skipprompt = TRUE;

            if (hstlast == NULL)
                break;

            hsttmp = hstlast;
            if (!strinb(hsttmp))
                return(FALSE);
            topseen = FALSE;
	    botseen = TRUE;
	    break;

        default:
            break;
        }
        return (FALSE);
    }
}
