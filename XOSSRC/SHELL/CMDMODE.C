//------------------------------------------------------------------------------
// CMDMODE.C
//
// Written by Bruce R. Nevins
// 
// Edit History:
// -------------
// 05/13/91(brn) - Created first version
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

#include <STDIO.H>
#include <SETJMP.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSRUN.H>
#include "SHELL.H"
#include "SHELLEXT.H"

/*
 * Function to process MODE command
 */

int cmdmode(void)

{
    long newmode;

    if (firstarg != NULL)
    {
        if (!singlearg)                 /* Must have one argument */
        {
            fprintf(cmderr, "? %s: Too many arguments\n", prgname);
	    if (detached)
		fflush(cmderr);
            return (TRUE);
        }
        newmode = gethexval(firstarg);  /* Get new mode value */
        if (newmode > 0x7E ||
                (newmode = svcTrmDspMode(STDTRM, newmode|0x100, NULL)) < 0)
        {
            fprintf(cmderr, "? %s: Illegal mode value\n", prgname);
	    if (detached)
		fflush(cmderr);
            return (TRUE);
        }
        screenmode = newmode & 0xFF;	/* OK - change screen mode */
    }
    else
    {
        printf("Screen mode = 0x%X\n", screenmode);
	if (detached)
	    fflush(stdout);
    }
    return (TRUE);
}
