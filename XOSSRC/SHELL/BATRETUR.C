//------------------------------------------------------------------------------
//
//  BATRETUR.C
//
//  Written by Bruce R. Nevins
//
//  Edit History:
//  -------------
//  01/01/87(brn) - Created first version
//  05/13/91(brn) - Change to return TRUE if command accepted
//  07/27/92(brn) - Set return flag
//  05/03/94(brn) - Add handling for Pause batch on error
//
//------------------------------------------------------------------------------

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
#include <XOS.H>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

//
// Function to process RETURN command
//

int batreturn(void)

{
    char tempstr[LINESIZE];     // temp to get just the return value

    if (!extend_batch)		// This only works in extended batch mode
	return (FALSE);

    if (firstarg == NULL)
        bat_return = TRUE;
    else
    {
        blkinic(expndarg);              // Start from beginning of string
        batnkeyw(tempstr, expndarg);    // Skip past the command

        batnkeyw(tempstr, expndarg);
        errrtn = atoi(tempstr);
        bat_return = TRUE;
	if ((batch || local_batch) && error_pause && (errrtn != 0))
	    errpause(); 		// If Pause batch on error
    }
    return (TRUE);
}
