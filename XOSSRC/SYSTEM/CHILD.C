/********************************************************/
/* CHILD.C                                              */
/* Functions dealing with child processes               */
/********************************************************/
/* Written by John Goltz                                */
/********************************************************/

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
#include <TIME.H>
#include <EXIT.H>
#include <XOSSVC.H>
#include <XOS.H>
#include <XOSVECT.H>
#include <XOSTRM.H>
#include <XOSMSG.H>
#include <XOSTIME.H>
#include <XOSUSR.H>
#include "INIT.H"
#include "INITEXT.H"

// Function to service child died interrupt

void childgone(
    struct intdata intdata)

{
    char *str;
    int   code;
    long  status;

    code = intdata.term >> 24;
    status = intdata.term & 0x00FFFFFFL;

    switch (code)
    {
     case TC_EXIT:		// Process terminated with exit
        sprintf(textbfr+16, "terminated; status = %06.6X", status);
        break;

     case TC_KILL:		// Process killed
        sprintf(textbfr+16, "killed; status = %06.6X", status);
        break;

     case TC_PIPE:		// Process terminated because pipe
				//   had no more input
        sprintf(textbfr+16, "terminated, Pipe out of input data: Status = "
                "%06.6X", status);
        break;

     case TC_BDUSTK:		// Bad user stack address
        sprintf(textbfr+16, "terminated, Illegal user stack pointer;"
                " CS:EIP = %04.4X:%08.8X, EFR = %08.8X"
                " SS:ESP = %04.4X:%08.8X", intdata.pCS&0xFFFF, intdata.pEIP,
                intdata.pEFR, intdata.pseg&0xFFFF, intdata.pos);
        break;

     case TC_NOMEM:			// No memory available
        sprintf(textbfr+16, "terminated, No memory available;"
                " CS:EIP = %04.4X:%08.8X, EFR = %08.8X",
                intdata.pCS&0xFFFF, intdata.pEIP, intdata.pEFR);
        break;

     case TC_UNIMOP:		// Unimplemented operation
        sprintf(textbfr+16, "terminated, Unimplemented operation;"
                " CS:EIP = %04.4X:%08.8X, EFR = %08.8X"
                " Data = %02.2X, %02.2X, %02.2X, %02.2X",
                intdata.pCS&0xFFFF, intdata.pEIP, intdata.pEFR,
                intdata.data&0xFF, (intdata.data>>8)&0xFF,
                (intdata.data>>16)&0xFF, (intdata.data>>24)&0xFF);
        break;

     case TC_NOVECT:		// Uninitialized user vector
        switch ((int)status)
        {
         case VECT_PILLINS:	// Illegal instruction
         case VECT_RILLINS:
            str = "Illegal instruction";
            goto novect;

         case VECT_PDIVERR:	// Divide error
         case VECT_RDIVERR:
            str = "Divide error";
            goto novect;

         case VECT_PTRACE:	// Trace trap
         case VECT_RTRACE:
            str = "Trace trap";
            goto novect;

         case VECT_PBRKPNT:	// Breakpoint interrupt
         case VECT_RBRKPNT:
            str = "Breakpoint interrupt";
            goto novect;

         case VECT_PFPPNAVL:	// Coprocessor not available
         case VECT_RFPPNAVL:
            str = "Coprocessor not available";
            goto novect;

         case VECT_PFPPERR:	// Coprocessor error
         case VECT_RFPPERR:
            str = "Coprocessor error";
            goto novect;

         case VECT_PBOUND:	// BOUND instruction
         case VECT_RBOUND:
            str = "BOUND instruction interrupt";
            goto novect;

         case VECT_PINTO:	// INTO instruction
         case VECT_RINTO:
            str = "INTO instruction interrupt";
            goto novect;

         case VECT_CHILD:	// Child died
            str = "Child process terminated interrupt";
            goto novect;

         case VECT_CNTC:		// Control-C
            str = "Control-C interrupt";
         novect:
            sprintf(textbfr+16, "terminated, %s; CS:EIP = %04.4X:%08.8X,"
                    " EFR = %08.8X", str, intdata.pCS&0xFFFF,
                    intdata.pEIP, intdata.pEFR);
            break;

         case VECT_PPROT:	// Protection fault
         case VECT_RPROT:
            str = "Protection fault";
            goto addrerr;

         case VECT_PPAGEFLT:	// Page fault
         case VECT_RPAGEFLT:
            str = "Page fault";
            goto addrerr;

         case VECT_PSEGNP:	// Segment not present
            str = "Segment not present";
            addrerr:
            sprintf(textbfr+16, "terminated, %s; CS:EIP = %04.4X:%08.8X,"
                    " EFR = %08.8X, Memory = %04.4X:%08.8X", str,
                    intdata.pCS&0xFFFF, intdata.pEIP, intdata.pEFR,
                    intdata.pseg, intdata.pos);
            break;

         default:
            sprintf(textbfr+16, "terminated, Interrupt for uninitialized "
                    "vector %X; CS:EIP = %04.4X:%08.8X, EFR = %08.8X",
                    status, intdata.pCS&0xFFFF, intdata.pEIP, intdata.pEFR);
            break;
        }
        break;

     default:
        sprintf(textbfr+16, "terminated, Unknown termination type = %02.2X,"
                    " status = %06.6X", code, status);
        break;
    }
    if (startupdone)
    {
        putinlog(intdata.pid, "PROCTERM", textbfr);
        closelog();
    }
}
