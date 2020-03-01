//------------------------------------------------------------------------------
// UTILITY.C - Utility functions for SHELL
//
// Written by John R. Goltz and Bruce R. Nevins
//
// Edit History:
// -------------
// 05/25/89(brn) - Moved getkeyword and srchatom from shell and batcmd
// 11/16/89(brn) - Make / be a command delimiter
// 12/05/89(brn) - Remove old screenmode handling and add new state save
//                 and restore
// 05/24/91(brn) - Make getkeyword take arg pointer
// 06/06/91(brn) - Fix getkeyword to mark a keyword with a drive and no
//                 path not SIMPLE.
// 10/09/91(jrg) - Change to support Z: instead of DSK: for DOS compatibility
// 08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
// 02/23/94(brn) - Change srchatom to use the command structure
// 04/01/94(brn) - Remove getsingle call, use getch or getche
//  30Nov94(fpj) - Changed system[] to systemspec[] to avoid conflict with
//                 ANSI C function.
// 11/02/95(brn) - Fix getkeyword to check length of keyword and argument
// 		strings.  Return an error if too long
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
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSTRM.H>
#include <XOSstr.h>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

#define DOSCMDS (sizeof(doscmd) / sizeof (struct command))

struct command doscmd[] =
{
	{"BREAK",	CD(NULL)},
	{"CD",		CD(NULL)},
	{"CHDIR",	CD(NULL)},
	{"CLS",		CD(NULL)},
	{"COMMAND",	CD(NULL)},
	{"COPY",	CD(NULL)},
	{"CTTY",	CD(NULL)},
	{"DATE",	CD(NULL)},
	{"DEL",		CD(NULL)},
	{"DIR",		CD(NULL)},
	{"ECHO",	CD(NULL)},
	{"ERASE",	CD(NULL)},
	{"EXIT",	CD(NULL)},
	{"FOR",		CD(NULL)},
	{"GOTO",	CD(NULL)},
	{"IF",		CD(NULL)},
	{"MD",		CD(NULL)},
	{"MKDIR",	CD(NULL)},
	{"PATH",	CD(NULL)},
	{"PAUSE",	CD(NULL)},
	{"PROMPT",	CD(NULL)},
	{"RD",		CD(NULL)},
	{"RMDIR",	CD(NULL)},
	{"REM",		CD(NULL)},
	{"REN",		CD(NULL)},
	{"RENAME",	CD(NULL)},
	{"SET",		CD(NULL)},
	{"SHIFT",	CD(NULL)},
	{"TIME",	CD(NULL)},
	{"TYPE",	CD(NULL)},
	{"VER",		CD(NULL)},
	{"VERIFY",	CD(NULL)},
	{"VOL",		CD(NULL)}
};

/*
 * Function to reset screen mode - screen is cleared if changing modes
 */

void resetscrn(void)

{
    svcTrmDspMode(STDTRM, DM_USEBASE, NULL);
    svcTrmFunction(STDTRM, TF_RESET);	/* Reset terminal to default state */
}

/*
 * Function to convert string to hex value
 */

long gethexval(str)
register char *str;

{
    long value;
    char chr;

    value = 0;
    while (isxdigit(chr=*str++))
    {
        value *= 0x10;
        if (chr >= 'A')
            chr += 9;
        value += chr & 0x0F;
    }
    if (chr != '\0')                    /* Stopped by end of string? */
        value = 0xFFFFFFFFL;            /* No - return illegal value */
    return (value);
}

int cmdchngdrv(drive)
char *drive;
{
    long rtn;

    if ((rtn=svcIoDefLog(0, 0, "Z:", drive)) < 0)
        cmnderr(NULL, rtn);             /* If error */
    return (TRUE);
}

/*
 * Function to report general error as returned from XOS
 */

void cmnderr(name, code)
char *name;
long  code;

{
    char message[80];
    svcSysErrMsg(code, 3, message);    /* Get error message string */
    fprintf(cmderr, "? %s: %s", prgname, message);
    if (name)
        fprintf(cmderr, ", file %s", name);
    fputc('\n', cmderr);
    if (detached)
	fflush(cmderr);
}

/*
 * Function to get a single key
 */

/*
 * Function to search for atom in table
 */

int srchatom(char *atom, struct command *table, short size)
{
    register int item;
    register char *ap,*tp;
    register char ac,tc;

    item = 0;
    while (--size >= 0)
    {
        ap = atom;
        tp = table->cmd;
        table++;
        do
        {   tc  = *tp++;
            ac = *ap++;
        } while (tc && ac && tc == ac);
        if (tc == ac)
            break;
        ++item;
    }
    if (tc == ac)                       /* Did we find a match? */
        return (item);                  /* Yes - return index */
    else
        return (-1);                    /* No - return -1 */
}

/*
 * Function to get first command keyword
 */

int getkeyword(struct firstblk *blkptr)
{
    register char *pnt2;
    register char  chr;
    register int   bfrlen;		// Length of Keyword/Argument buffer

    singlearg = FALSE;                  // Assume not a single arg
    pnt2   = keyword;                   // Point to keyword array
    bfrlen = LINESIZE;			// Start with keyword buffer
    simple = TRUE;                      // Assume simple keyword
    *pnt2  = 0;                         // Assume null string

    blkinic(blkptr);                    // Start at beginning of block
    chr = blkgetc(blkptr);              // Get the first character

/*
 * Remove leading spaces
 */
    while ((chr != '\0') && isspace(chr))
        chr = blkgetc(blkptr);          // Get the first character

/*
 * Copy keyword
 */
    while ((chr != '\0') && !isspace(chr) && --bfrlen)
    {
	if (chr == '/')
	    break;

	if (chr == '\\' || chr == '.')
	{
	    *pnt2 = '\0';		// Include the NULL
            strupper(keyword);		// Convert to upper case
	    if (srchatom(keyword, doscmd, DOSCMDS) >= 0)
	    	break;			// Search the command table
	}

	if ((chr == ';') || (chr == ','))
        {
            simple = FALSE;
            break;
        }
        if (!isalnum(chr))
        {
            simple = FALSE;
        }
        *pnt2++ = chr;
        chr = blkgetc(blkptr);          /* Get the next character */
    }
    if (bfrlen == 0)
    {
	fprintf(cmderr, "? %s: Command keyword too large\n",
                prgname);
	if (detached)
	    fflush(cmderr);
	return (FALSE);
    }

    *pnt2 = '\0';                       /* Include the NULL */
    if (*(pnt2 - 1) == ':')
    {
        simple = TRUE;
    }

/*
 * Skip trailing spaces
 */
    while ((chr != '\0') && isspace(chr))
        chr = blkgetc(blkptr);          /* Check the character */

/*
 * Copy first argument (if any)
 */
    if (chr != '\0')
    {
        firstarg = firstbuf;            // Point to the buffer
	bfrlen = LINESIZE;		// Size of argument buffer

        while (chr != '\0' && !isspace(chr) && --bfrlen)
        {
	    if ((chr == ';') || (chr == ',') || (chr == '/'))
                break;

            *firstarg++ = chr;
            chr = blkgetc(blkptr);      /* Get the first character */
        }
	if (bfrlen == 0)
	{
	    fprintf(cmderr, "? %s: Command argument too large\n",
			prgname);
	    return (FALSE);
	}

        *firstarg = '\0';               /* End the string */
        firstarg = firstbuf;            /* Point to the buffer */
    }
    else
        firstarg = NULL;

    if (chr == '\0')
        singlearg = TRUE;               /* Just one argument */

#ifdef DEBUG
    printf("in getkeyword keyword = %s\n", keyword);
    if (firstarg != NULL)
	printf("in getkeyword firstarg = %s\n", firstarg);
#endif

    return (TRUE);
}

/*
 * Function to get value of system class characteristic
 */

struct
{   byte4_char syschar;
    char       end;
} sysparm =
{   {(PAR_GET|REP_HEXV), 4}
};

static char systemspec[] = "SYSTEM:";

type_qab charqab =
{   QFNC_WAIT|QFNC_CLASSFUNC,	/* qab_func     - Function */
    0,				/* qab_status   - Returned status */
    0,				/* qab_error    - Error code */
    0,				/* qab_amount   - Process ID */
    0,				/* qab_handle   - Device handle */
    0,				/* qab_vector   - Vector for interrupt */
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    CF_VALUES,			/* qab_option   - Options or command */
    0,				/* qab_count    - Count */
    systemspec, 0, 		/* qab_buffer1  - Pointer to file spec */
    (char *)&sysparm, 0,	/* qab_buffer2  - Unused */
    NULL, 0			/* qab_parm     - Pointer to parameter area */
};

ulong getsyschar(
    char *str)

{
    strncpy(sysparm.syschar.name, str, 8);
    svcIoQueue(&charqab);
    return ((ulong)(sysparm.syschar.value));
}

/*
 * Print syntax error message
 */
void synerr(char *errstr, char *secondstr)
{
    if (errstr == NULL)
        fprintf(cmderr, "Syntax error\n");
    if (secondstr == NULL)
        fprintf(cmderr, "Syntax error -- %s\n", errstr);
    else
        fprintf(cmderr, "Syntax error -- %s: %s\n", errstr, secondstr);
}

//
// errpause - Print error pause message ans wait for any key
//

int errpause(void)
{
    char chr;

    if (stdin->iob_handle != STDIN || detached)
					// If not running at a console
	return (TRUE);			//  PAUSE does nothing

    fputs("? BATCH: Error : Strike a key to continue . . . \n", cmdout);
    svcSchSetLevel(0);			// Enable Software interrupts now
    chr = (char)getch();		// Get the character with no echo
    svcSchSetLevel(8);			// Disable Software interrupts now
    if (chr == '\0')
    {
	getch();			// Get the character with no echo
	getch();			// Get the character with no echo
	getch();			// Get the character with no echo
    }
    fputs("\n", cmdout);
    if (detached)
	fflush(cmdout);
    return (TRUE);
}

