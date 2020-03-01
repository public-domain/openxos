/***************************************************/
/* Symbiont request handler for INIT               */
/***************************************************/
/* Written by John Goltz                           */
/***************************************************/

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
#include <TIME.H>
#include <XCSTRING.H>
#include <UTILITY.H>
#include <XOSSVC.H>
#include <XOS.H>
#include <XOSTRM.H>
#include <XOSMSG.H>
#include <XOSTIME.H>
#include <XOSUSR.H>
#include "INIT.H"
#include "INITEXT.H"

// Function to process symbiont request message

void symbiontreq()

{
    char *cpnt;
    char *apnt;
    char *tpnt;
    long  rtn;
    int   cnt;
    char  symname[200];		// Name of symbiont
    char  symargs[300];		// Buffer for argument list
    char  havedev;
    char  haveext;
    char  chr;

    apnt = msgbfr + 1;
    cnt = (int)(msginpqab.qab_amount - 1);
    while ((--cnt >= 0) && ((chr=*apnt++) != '\0') && !isspace(chr))
        ;
    if ((msginpqab.qab_amount - cnt) > 85)
    {
        response("\7? ", "Symbiont name too long");
        return;
    }
    if (cnt >= 0)
        --apnt;
    *apnt++ = '\0';

    // First setup the name of the symbiont for the run SVC

    tpnt = msgbfr + 1;
    havedev = FALSE;
    haveext = FALSE;
    while ((chr=*tpnt++) != '\0')
    {
        if (chr == ':')
            havedev = TRUE;
        else if (chr == '.')
            haveext = TRUE;
    }
    if (havedev)
        cpnt = symname;
    else
        cpnt = strmov(symname, "XOSSYS:");
    cpnt = strmov(cpnt, (char *)(msgbfr+1));
    if (!haveext)
        strmov(cpnt, ".RUN");

    // Now setup the command arguments

    cpnt = strmov(symargs, (char *)(msgbfr+1)); // Put name of program first
    if (cnt > 280 - (cpnt-symargs))
    {
        response("\7? ", "Symbiont arguments too long");
        return;
    }
    *cpnt++ = ' ';			// Put source address in as 1st
    cpnt = strmov(cpnt, srcbfr);	//   argument
    *cpnt++ = ' ';
    while ((--cnt >= 0) && (*cpnt++ = *apnt++) != '\0')
        ;				// Put in remaining data as arugments
    *cpnt = '\0';			// Make sure have null at end
    runqab.qab_buffer1 = symname;
    runqab.qab_option = R_SESSION;
    runparm.arglist.buffer = symargs;
    runparm.arglist.bfrlen = runparm.arglist.strlen = cpnt - symargs;
    runparm.devlist.desp = 0;

    if ((rtn=svcIoRun(&runqab)) < 0 || (rtn=runqab.qab_error) < 0)
    {
        svcSysErrMsg(rtn, 3, symargs); // Get error message string
        response("\7? ", symargs);	// And complain
    }
}
