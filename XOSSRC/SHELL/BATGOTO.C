//------------------------------------------------------------------------------
// batgoto.c - GOTO routine for BATCH
//
// Written by Bruce R. Nevins
//
// Edit History:
// -------------
// 03/02/90(brn) - Added header to file
// 05/13/91(brn) - Change to return TRUE if command accepted
// 06/24/91(brn) - Fix bug where blank line terminated goto
// 08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
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
#include <STDIO.H>
#include <STRING.H>
#include <XOS.H>
#include <XOSSTR.H>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

//
// Function to process GOTO command
//

int batgoto(void)

{
    char label[LINESIZE+2];     // Place to store the string

    blkinic(expndarg);          	// Start from beginning of string
    batnkeyw(label, expndarg);  	// Skip past the GOTO command


    if (firstarg == NULL)
    {
        synerr("GOTO: No label specified", NULL);
        return (TRUE);
    }

    batnkeyw(label, expndarg);  	// Get processed keyword
    strupper(label);

    if (fseek(cmdin, 0L, SEEK_SET) !=0)
    {
        fileerr(filname, -errno);
        return (TRUE);
    }
    while (!feof(cmdin))
    {
        if (!getcmd())          	// Get command
        {
            linecnt = 0;                // If error, discard rest of line
	    continue;
        }
        skipprompt = TRUE;              // Skip the next prompt
    
        if (!getkeyword(argbase))	// Get first command atom
	    return (FALSE);

        strupper(keyword);              // Convert to upper case
        expndarg = batxlate(expndarg, argbase);
                                        // Expand the command line arguments

        if (*keyword == ':')            // If label
        {
	    batlabel();			// Record label that was seen
            if (strncmp(&label[0], &keyword[1], LABELSIZE) == 0)
            {
	        skipprompt = FALSE;
                return (TRUE);
            }
        }
    }
    fprintf(cmderr, "Label (%s) not found\n", label);
    if (detached)
        fflush(cmderr);
    return (TRUE);
}

