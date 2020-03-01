//------------------------------------------------------------------------------
// CMDLGOUT.C
//
// Written by Bruce R. Nevins
//
// Edit History:
// -------------
// 05/13/91(brn) - Created first version
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

#include <STDIO.H>
#include <STDLIB.H>
#include <XOSSVC.H>
#include <XOS.H>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

char loname[] = "CMD:LOGOUT.{{RUN}{EXE}";

/*
 * Function to process LOGOUT and BYE commands
 */

void cmdlogout(void)
{
    killdet();                          /* Kill any detached children */
/*  runparm.rn_argpnt = NULL;
    runparm.rn_argcnt = 0; */
/*  if ((svcIoRun(R_CHILD|R_ACSENV|R_CHGENV, loname, &runparm)) < 0) */
    {
        loggedin = FALSE;               /* No longer logged in */
        fprintf(cmderr, "\n? %s: Accounting failure\n\n\377\377\377\377",
             prgname);
	if (detached)
	    fflush(cmderr);
        exit(3);
    }
/*  else
        exit(0); */
}
