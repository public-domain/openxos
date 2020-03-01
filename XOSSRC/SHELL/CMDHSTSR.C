//------------------------------------------------------------------------------
// CMDHSTSR.C - Command History Search
//
// Written by Bruce R. Nevins
// 
// Edit History:
// -------------
// 02/18/90(brn) - Changed blkstrncmp to blkstrnicmp so history will ignore
// 		case on recall
// 05/22/91(brn) - Fix bug where !command with no history kills shell
// 02/17/92(brn) - Fix bug in command recall, use new strinb function
// 03/23/92(brn) - Did away with rmcmd function
// 08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
// 02/02/93(brn) - Fix Program name not diplayed when search fails
// 07/06/95(brn) - Fix history placement error if search failed
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
#include <XOSSVC.H>
#include <XOS.H>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

int cmdhstsrch(void)
{
    char *pnt;                  /* Pointer to string */
    short foundnum;             /* Flag, true if a number */
    int num;                    /* Number to search for */
    int j;

    foundnum = FALSE;                   /* Assume did not find a number */
    if (firstarg == NULL)
    {
        pnt = keyword;
        if (strlen(pnt) == 1)
            strcat(pnt, "1");           /* If Just ! default to 1 */

        foundnum = TRUE;                /* Assume we found a number */

	pnt++;
        if (!isdigit(*pnt))
            foundnum = FALSE;		/* Did not find a number */
    }

    if (foundnum)
    {
        num = atoi(pnt);
        if ((num <= (histcnt + histlow)) && (num > histlow))
        {
	    hsttmp = hstfrst;
            if (hsttmp == NULL)
            {
                hsttmp = hstlast;
                fprintf(cmderr, "? %: Ran out of list before end\n", prgname);
		if (detached)
		    fflush(cmderr);
                return (TRUE);
            }

            for(j = 1; num > j + histlow; j++)
            {
                if ((hsttmp = hsttmp->a.nextlst) == NULL)
                {
                    hsttmp = hstlast;
                    fprintf(cmderr, "? %s: Ran out of list before end\n", prgname);
		    if (detached)
			fflush(cmderr);
                    return (TRUE);
                }
            }
            if (!strinb(hsttmp))
                return (FALSE);
            return (TRUE);
        }
        else
        {
            hsttmp = hstlast;
            fprintf(cmderr, "? %s: No such command in History\n", prgname);
	    if (detached)
		fflush(cmderr);
            return (TRUE);
        }

    }

    if (hsttmp == NULL)
    {
        hsttmp = hstlast;
        fprintf(cmderr, "? %s: No such command in History\n", prgname);
	if (detached)
	    fflush(cmderr);
        return (TRUE);
    }

    for(;;)
    {
        blkinic(argbase);               /* Start search string from beginning */
        blkgetc(argbase);               /* Skip past the ! */
        blkinic(hsttmp);                /* Start history string from beginning */
        if (blkstrnicmp(argbase, hsttmp, blklen(argbase)) == 0)  /* search for match */
            break;
        if ((hsttmp = hsttmp->a.lastlst) == NULL)
        {
            hsttmp = hstlast;
            fprintf(cmderr, "? %s: No such command in History\n", prgname);
	    if (detached)
		fflush(cmderr);
            return (TRUE);
        }

    }

    if (!strinb(hsttmp))
	return (FALSE);
    return (TRUE);
}
