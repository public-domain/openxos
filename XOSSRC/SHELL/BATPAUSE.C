//------------------------------------------------------------------------------
//
//  BATPAUSE.C
//
//  Written by Bruce R. Nevins
//
//  Edit History:
//  -------------
//  01/01/87(brn) - Created first version
//  05/13/91(brn) - Change to return TRUE if command accepted
//  08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
//  02/21/93(brn) - Fix to eat extra characters for function keys
//  03/31/94(brn) - Change to use getch instead of getsingle
//  12/22/95(brn) - Make PAUSE do nothing if there is no console
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

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <XOS.H>
#include <XOSSVC.H>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

// Function to process PAUSE command

int batpause(void)

{
    char chr;

    if (stdin->iob_handle != STDIN || detached)
					// If not running at a console
	return (TRUE);			//  PAUSE does nothing

    fputs("Strike a key when ready . . . \n", cmdout);
    svcSchSetLevel(0);			// Enable Software interrupts now
    chr = (char)getch();		// Get the character with no echo
    svcSchSetLevel(8);			// Disable Software interrupts now
    if (chr == '\0')			// If 0 the next 3 bytes are extended
    {					//  character status
	getch();			// Get the character with no echo
	getch();			// Get the character with no echo
	getch();			// Get the character with no echo
    }
    fputs("\n", cmdout);
    if (detached)
	fflush(cmdout);
    return (TRUE);
}
