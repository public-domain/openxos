//**************************************
// Function to do assembly pass for XMAC
//**************************************
// Written by John Goltz
//**************************************

//++++
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
#include <CTYPE.H>
#include <XOS.H>
#include "XMAC.H"
#include "XMACEXT.H"

//*************************************
// Function: assemble: Do assembly pass
// Returned: Nothing
//*************************************

void assemble(void)

{
    struct pd *pdpnt;

    eofflg = FALSE;			// Reset EOF indicator
    ttlflg = FALSE;			// And .TITLE seen flag
    tocflg = TRUE;			// Default is list table of contents
    lstseq = 1;				// Reset listing sequence number
    pagnum = 1;				// Reset page number
    lsbnum = 1;				// Reset local symbol block number
    gnacnt = 0;				// Reset generated argument number
    iradix = 16;			// Reset input radix
    curpsd = NULL;			// No psect to begin with
    condlvl = 0;			// Not in conditional
    hdrstb[0] = '\0';			// Indicate no subtitle text
    thissrc = (struct sb *)(&firstsrc);
    while (getsrc())			// Setup for input
    {   do
	{   listline();			// List previous line
	    asmline();			// Process single input line
	} while (!eofflg);		// Continue until run out of input
    }
    while (inspnt)			// Clean up any hanging insersion!
	endins();
    if (condlvl)			// Have open conditional?
        errsub(errcomsg, ERR_CO);	// Yes - complain
    if (errflag != 0 && albcnt == 0)	// Make sure list final error(s)
    {   albcnt = 1;
	albbuf[0] = '\0';
    }
    listline();				// List last line
    pdpnt = pshead;			// Point to first psect
					//   data block
    while (pdpnt != NULL)		// Scan all psect data blocks
    {
        pdpnt->pd_ofs = 0;		// Reset offset
        pdpnt = pdpnt->pd_next;		// Advance to next block
    }
}
