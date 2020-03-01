//------------------------------------------------------------------------------
// batif.c
//
// Written by Bruce R. Nevins
//
// Edit History:
// -------------
// 01/01/87(brn) - Created first version
// 05/13/91(brn) - Change to return TRUE if command accepted
// 07/22/92(brn) - Add parameters for call to batcmd
// 08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
// 03/21/94(brn) - add givechain(interncmd) to dointcmd in case old internal
// 			command was around.
// 04/21/94(brn) - Make internal command not go into the command history list
// 04/25/94(brn) - fix bug where we did not giveup command copy
// 07/19/96(brn) - Remove parameters from batcmd
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <xos.h>
#include <xossvc.h>
#include <xoserr.h>
#include <xoserrmsg.h>
#include <xosstr.h>
#include "shell.h"
#include "shellext.h"
#include "batchext.h"

/*
 * Function to process IF command
 */

int batif(void)
{
    long rtn;
    char firststr[LINESIZE+2];
    char secondstr[LINESIZE+2];
    int  notseen;				// if a NOT was seen in the string
    int  count;					// I/O count
    struct firstblk *expndlcl;	// Local copy of expanded line

    notseen = FALSE;

    expndlcl = NULL;					// Let copy allocate the block
    blkinic(expndarg);          		// Start from beginning of string
    expndlcl = blkcpy(expndlcl, expndarg); // Make a copy in case we recurse
    blkinic(expndlcl);          		// Start from beginning of string

    if (!batnkeyw(keyword, expndlcl))   // Get past the IF
    {
        synerr("IF: Internal error, Could not find IF", NULL);
	givechain(expndlcl);				// Give back our copy of the command
        return (TRUE);
    }

	//Check for NOT keyword

    if (!batnkeyw(keyword, expndlcl))	// Get keyword
    {
        synerr("IF: Missing keyword", NULL); // Syntax error msg
		givechain(expndlcl);			// Give back our copy of the command
        return (TRUE);
    }

    if (stricmp(keyword, "NOT") == 0)	// Was it NOT ?
    {
        notseen = TRUE;
        if (!batnkeyw(keyword, expndlcl)) // Get next keyword
        {
            synerr("IF: Missing keyword", NULL); // Syntax error msg
			givechain(expndlcl);		// Give back our copy of the command
            return (TRUE);
        }

    }

	// Check for ERRORLEVEL keyword

    if (stricmp(keyword, "ERRORLEVEL") == 0)
    {
        if (!batnkeyw(keyword, expndlcl)) // Get next keyword
        {
            synerr("IF: Missing Errorlevel number to check", NULL);
			givechain(expndlcl);		// Give back our copy of the command
            return (TRUE);
        }

        if (!isdigit(*keyword))
		{
            if (strcmp(keyword, "==") == 0)
            {
                if (!batnkeyw(keyword, expndlcl)) // Get next keyword
                {
                    synerr("IF: Missing Errorlevel number to check", NULL);
					givechain(expndlcl); // Give back our copy of the command
                    return (TRUE);
                }
                if (!isdigit(*keyword))
                {
                    synerr("IF: Invalid Errorlevel number to check", keyword);
					givechain(expndlcl); // Give back our copy of the command
                    return (TRUE);
                }
			}
            else
            {
                synerr("IF: Invalid Errorlevel number to check", keyword);
				givechain(expndlcl);	// Give back our copy of the command
                return (TRUE);
            }
		}
        if (errrtn < atoi(keyword))
        {
            if (notseen)
            {
                dointcmd(expndlcl);
				givechain(expndlcl);	// Give back our copy of the command
                return (TRUE);
            }
            else
			{
				givechain(expndlcl);	// Give back our copy of the command
                return (TRUE);
			}
        }
        else
        {
            if (notseen)
			{
				givechain(expndlcl);	// Give back our copy of the command
                return (TRUE);
			}
            else
            {
                dointcmd(expndlcl);
				givechain(expndlcl);	// Give back our copy of the command
                return (TRUE);
            }
        }
    }

	// Check for EXIST keyword

    if (stricmp(keyword, "EXIST") == 0)
    {
        if (!batnkeyw(keyword, expndlcl)) // Get next keyword
        {
            synerr("IF: Missing filename to check", NULL);
			givechain(expndlcl);		// Give back our copy of the command
            return (TRUE);
        }

        if ((rtn=svcIoDevParm(0, keyword, NULL)) < 0)
            if (rtn == ER_FILNF)
                count = 0;
            else
            {
                count = 0;
                femsg(prgname, rtn, keyword);
            }
        else
            count = 1;

        if (count == 0)
		{
            if (notseen)
            {
                dointcmd(expndlcl);
				givechain(expndlcl);	// Give back our copy of the command
                return (TRUE);
            }
            else
			{
				givechain(expndlcl);	// Give back our copy of the command
				return (TRUE);
			}
		}
		else
		{
			if (notseen)
			{
				givechain(expndlcl);	// Give back our copy of the command
                return (TRUE);
			}
            else
            {
                dointcmd(expndlcl);
				givechain(expndlcl);	// Give back our copy of the command
                return (TRUE);
            }
		}
    }

    strcpy(firststr, keyword);			// Save first string
    if (!batnkeyw(keyword, expndlcl))	// Get next keyword
    {
        synerr("IF: Incomplete", NULL);	// Syntax error msg
		givechain(expndlcl);			// Give back our copy of the command
        return (TRUE);
    }

	// Check for "==" string compare

    if (strcmp(keyword, "=") == 0)		// Is this a compare?
    {
		if (!batnkeyw(keyword, expndlcl)) // Get next keyword
		{
            synerr("IF: Incomplete", NULL); // Syntax error msg
			givechain(expndlcl);		// Give back our copy of the command
            return (TRUE);
		}
		else
		{
			if (strcmp(keyword, "=") != 0) // Is this a compare?
			{
				synerr("IF: Missing second =", keyword); // Syntax error msg
				givechain(expndlcl);	// Give back our copy of the command
				return (TRUE);
			}
		}
        if (!batnkeyw(secondstr, expndlcl)) // Get next keyword
        {
            synerr("IF: Missing second argument to ==", NULL);
			givechain(expndlcl);		// Give back our copy of the command
            return (TRUE);
        }
        if (strcmp(firststr, secondstr) == 0)
        {
            if (notseen)
			{
				givechain(expndlcl);	// Give back our copy of the command
                return (TRUE);
			}
            else
            {
                dointcmd(expndlcl);
				givechain(expndlcl);	// Give back our copy of the command
                return (TRUE);
            }
        }
        else if (notseen)
		{
			dointcmd(expndlcl);
			givechain(expndlcl);		// Give back our copy of the command
			return (TRUE);
		}
		else
		{
			givechain(expndlcl);		// Give back our copy of the command
			return (TRUE);
		}
    }
    else
        synerr("IF: Invalid keyword", keyword);

    givechain(expndlcl);				// Give back our copy of the command
    return (TRUE);
}

//
// Do internal command
//

void dointcmd(
	struct firstblk *inblk)

{
    interncmd = givechain(interncmd);   // Give up our old chain of memory
    interncmd = blkputc(interncmd, '@');// So the command will not echo
    blkcat(interncmd, inblk);
    inlinecmd = TRUE;					// Mark as an inline command
    tempnohist = TRUE;					// Don't place command in history
    blkinic(interncmd);
    batcmd();
    interncmd = givechain(interncmd);   // Give up our old chain of memory
}
