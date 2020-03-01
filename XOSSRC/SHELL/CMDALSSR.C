//------------------------------------------------------------------------------
// CMDALSSR.C - Command Alias Search
//
// Written by Bruce R. Nevins
// 
// Edit History:
// -------------
// 05/23/91(brn) - Created first version
// 03/09/94(brn) - Finish code
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
#include <STDLIB.H>
#include <STRING.H>
#include <XOSSVC.H>
#include <XOS.H>
#include "SHELL.H"
#include "BATCHEXT.H"
#include "SHELLEXT.H"

int cmdalssrch(void)
{
    long   i;			// A scratch counter
    char   chr;
    int    argc;		// Argument count
    char **argv;		// Pointer to the array of pointers
    int      save_argc;		// Argument count
    char    **save_argv;	// Pointer to the array of pointers
    char   dummycmd[LINESIZE];	// Dummy command to build arguments with

    if ((aliastmp = aliasfrst) == NULL)
    {
        return (FALSE);
    }

    for(;;)
    {
        blkinic(aliastmp);		// Start string from beginning
        blkinic(argbase);		// Start string from beginning

        if (blkstrnicmp(argbase, aliastmp, strlen(keyword)) == 0)
	{
	    if (blkgetc(aliastmp) == '=') // Make sure its not a partial
        	break;			// Match found break out
	}

        if ((aliastmp = aliastmp->a.nextlst) == NULL)
            return (FALSE);		// No match found, return
    }

    blkinic(aliastmp);

// Execute command here
    while (((chr = blkgetc(aliastmp)) != '=') && (chr != '\0'))
	;				// Skip past the =
    interncmd = blkcpy(interncmd, aliastmp);	// Copy the aliased command
						//  to execute
// Make the dummy command
    i = 0;
    blkinic(argbase);			// Reset to beginning
    while ((dummycmd[i++] = blkgetc(argbase)) != '\0')
	;
    dummycmd[i] = '\0';			// End the string
    argc = batargs(dummycmd, &argv);	// Setup the argument list
    inlinecmd = TRUE;			// Show this is in line cmd
    skipprompt = TRUE;			// Skip the next prompt
    tempnohist = TRUE;			// disable history for this cmd
    save_argc = argcounter;		// Save previous batch file settings
    save_argv = argpointer;
    argcounter = argc;			// Setup new batch parameters
    argpointer = argv;
    batcmd();				// Process the command
    argcounter = save_argc;		// Restore old batch paramters
    argpointer = save_argv;

    return (TRUE);
}
