//------------------------------------------------------------------------------
// batbat.c - BATCH file processor
// 
// Written by Bruce R. Nevins
//
// Edit History:
// -------------
// 07/23/92(brn) - Created first version
// 08/05/92(brn) - Change quit to bat_ctlc
// 08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
// 03/01/93(brn) - Remove close of cmdin let batcmd handle it.
// 07/19/96(brn) - Changed to save and restore argpointer and argcounter
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
#include <STRING.H>
#include <XOS.H>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

int batbat(char *batfile)
{
    FILE    *temp_file;			/* FD for Batch input */
    char    *tempstr;
    register char  chr;
    char     temp_command[LINESIZE+2];	/* Temp string of command list */
    int      temp_argc;			/* Argument count */
    char    **temp_argv;		/* Pointer to the array of pointers */
    int      save_argc;			/* Argument count */
    char    **save_argv;		/* Pointer to the array of pointers */

    if ((temp_file = fopen(batfile, "r")) != NULL)
    {
        cmdin = temp_file;

/*
 * Setup the argument list
 */
	tempstr = temp_command;
	blkinic(expndarg);	/* Point to beginning of string */

	while ((chr = blkgetc(expndarg)) != '\0' &&
	        chr != '>' && chr != '<' && chr != '|')
	{
	    *tempstr++ = chr;
	}
	*tempstr = '\0';            /* end string */
	temp_argc = batargs(temp_command, &temp_argv);

	local_batch = TRUE;	/* So we don't exit at batch end */
	save_argc = argcounter;	// Save previous batch file settings
	save_argv = argpointer;
	argcounter = temp_argc;	// Setup new batch parameters
	argpointer = temp_argv;
	while (TRUE)
	{
	    if (bat_ctlc || feof(cmdin) || bat_return)
	    {
	        if (local_batch)
	        {
		    local_batch = FALSE;
		}
		if (bat_return)
		    bat_return = FALSE;

		break;
	    }
	    batcmd();
	}
	argcounter = save_argc;	// Restore old batch paramters
	argpointer = save_argv;
	return (TRUE);
    }
    else
    {
	return (FALSE);
    }
}

