//------------------------------------------------------------------------------
// batcall.c - CALL routine for BATCH
// 
// Written by Bruce R. Nevins
//
// Edit History:
// -------------
// 04/20/90(brn) - Fixed bug not setting PID properly
// 05/13/91(brn) - Change to return TRUE if command accepted
// 08/05/92(brn) - Change quit to bat_ctlc
// 08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
// 07/19/96(brn) - Changed to save and restore argpointer and argcounter
// 08/27/96(brn) - Add fflush if we are detached
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
#include <STDIO.H>
#include <STRING.H>
#include <XOSERR.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSSTR.h>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

/*
 * Function to process CALL command
 */

static char    batext[] = ".BAT";	/* Default extension for BATCH */

int batcall()
{
    register char  chr;
    FILE    *old_cmdin;			/* FD for Batch Command input */
    long     old_stdin;			/* FD for Batch Standard input */
    FILE    *temp_file;                 /* FD for Batch input */
    char     temp_command[LINESIZE+2];	/* Temp string of command list */
    char     temp[LINESIZE];
    int      temp_argc;			/* Argument count */
    char    **temp_argv;		/* Pointer to the array of pointers */
    int      save_argc;			/* Argument count */
    char    **save_argv;		/* Pointer to the array of pointers */
    char    *tempstr;
    int      old_batch;			/* Old value of batch state */

    blkinic(expndarg);			/* Start from beginning of string */
    batnkeyw(temp, expndarg);		/* Skip past the command */
    batnkeyw(temp, expndarg);
    if (strchr(temp, '.') != NULL)
        strcpy(runname, temp);
    else
        strcpy(strmov(runname, temp), batext);

    if ((temp_file = fopen(runname, "r")) != NULL)
    {
	old_cmdin = cmdin;		/* Save old file handle for return */	    
	cmdin = temp_file;
	old_batch = batch;
	batch = TRUE;

/*
 * Setup the argument list
 */
	tempstr = temp_command;
	blkinic(expndarg);	/* Point to beginning of string */
	batnkeyw(temp, expndarg);		/* Skip past the command */
	while ((chr = blkgetc(expndarg)) != '\0' &&
	        chr != '>' && chr != '<' && chr != '|')
	{
	    *tempstr++ = chr;
	}
	*tempstr = '\0';            /* end string */
	temp_argc = batargs(temp_command, &temp_argv);

	old_stdin = devlist.dlstdin.src;
	save_argc = argcounter;	// Save previous batch file settings
	save_argv = argpointer;
	argcounter = temp_argc;	// Setup new batch parameters
	argpointer = temp_argv;
	while (TRUE)
	{
	    if (bat_ctlc || feof(cmdin) || bat_return)
	    {
		if (fclose(cmdin) != 0)
		{
		    fprintf(cmderr,
			    "? %s: Could not close current batch file\n",
		    	    prgname);
		    if (detached)
			fflush(cmderr);
		}
		cmdin = old_cmdin;
	    	devlist.dlstdin.src = old_stdin;
		batch = old_batch;
		if (bat_return)
		    bat_return = FALSE;

		break;
	    }
	    batcmd();
	}
	argcounter = save_argc;	// Restore old batch paramters
	argpointer = save_argv;
    }
    else
    {
        if (errno == -ER_FILNF || errno == -ER_DIRNF)
            fputs("Bad command or filename\n", cmderr);
        else
            fileerr(runname, -errno);
        errrtn = -errno;			/* save any return value from command */
    }
    return (TRUE);
}
