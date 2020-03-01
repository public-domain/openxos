//------------------------------------------------------------------------------
// batecho.c
//
// Written by Bruce R. Nevins
//
// Edit History:
// -------------
// 01/01/87(brn) - Created first version
// 05/13/91(brn) - Change to return TRUE if command accepted
// 08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
// 03/31/94(brn) - Change the handling of echoed strings
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
#include <STRING.H>
#include <XOS.H>
#include <XOSSTR.H>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

/*
 * Function to process ECHO command
 */

int batecho(void)
{
    char tempbuf[LINESIZE+2];
    char chr;

    blkinic(expndarg);                  /* Start from beginning of string */
    batnkeyw(tempbuf, expndarg);        /* Skip past the command */

    if (!batnkeyw(tempbuf, expndarg))
    {
        if (echo == TRUE)
              fprintf(cmdout, "ECHO is on\n");
        else
              fprintf(cmdout, "ECHO is off\n");
	if (detached)
	    fflush(cmdout);
        return (TRUE);
    }

    if (!batnkeyw(tempbuf, expndarg))   /* Must not be anything else on the line */
    {
        if (stricmp(tempbuf, "OFF") == 0)
        {
            echo = FALSE;
            return (TRUE);
        }
        else if (stricmp(tempbuf, "ON") == 0)
        {
            echo = TRUE;
            return (TRUE);
        }
    }

/*
 * Print the echo string
 */
    blkinic(expndarg);                  /* Start from beginning of string */
    batnkeyw(tempbuf, expndarg);        /* Skip past the command */
    while ((chr = blkgetc(expndarg)) != '\0')
	if (!isspace(chr))
	    break;
    fputc(chr, cmdout);
    blkputs(expndarg, cmdout);          // Print the rest of the line
    fputc('\n', cmdout);                /* End the line */
    if (detached)
	fflush(cmdout);
    return (TRUE);
}

