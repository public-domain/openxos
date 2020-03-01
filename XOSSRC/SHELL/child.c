//------------------------------------------------------------------------------
// CHILD.C - Functions dealing with child processes
//
// Written by John R. Goltz
//
// Edit History:
// -------------
// 02/14/90(brn) - Added standard header
// 02/24/90(brn) - Remove cntlccnt and cntlcflg
// 03/02/90(jrg) - Add support for new ^C handling
// 08/03/92(brn) - Fix bug in child gone that set childcnt negative
// 08/05/92(brn) - Change quit to bat_ctlc
// 08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
// 12/29/92(brn) - Fix handling of input when asking for batch file
//                 termination
// 02/21/94(brn) - Change prompt for Control C in batch to be under keyword
//                 control.
// 04/01/94(brn) - Change cntlcsrv to do longjmp not return when in batch
// 05/23/95(fpj) - Added code to reset terminal after CTRL/C - this fixed bug
//                 with F9 and pause commands, among others.
// 12/22/95(brn) - Fix ^C handling to not prompt for abort input if standard
// 	    		in is not the console
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
#include <SETJMP.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <XOSSVC.H>
#include <XOS.H>
#include <XOSTRM.H>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "SHELLJMP.H"
#include "BATCHEXT.H"

// Function to service cntl-C interrupt

void cntlcsrv(void)
{
    register struct cprocs *pnt;
    register short  cnt;
    char reply;
    int show_cc;

    show_cc = FALSE;

    if ((batch || local_batch) && stdin->iob_handle == STDIN)
    {
        if (childcnt)					// Are we running a program(s) now?
        {
            pnt = cproc;				// Yes - try to halt them all
            cnt = MAXCHILD;
            do
            {   if (pnt->c_state > 0)	// Is this process running?
                    svcSchIntrProc(pnt->c_pid, 3, 0); // Yes - halt it
                pnt++;
            } while (-- cnt > 0);
        }
        svcSchCtlCDone();
		while (ctrlc_prompt)
		{
			fputs("^C\nTerminate batch job (Y/N)? ", cmderr);
			reply = (char)getche();	// Get the reply
			if ((toupper(reply) == 'Y') || (toupper(reply) == 'N'))
				break;
			if (reply == '\0')
			{
				getch();				// Clear the rest of the
				getch();				// function key data
				getch();
			}
		}
        fputc('\n', cmderr);
        if (toupper(reply) == 'N' && ctrlc_prompt == TRUE)
        {
            if (childcnt)				// Are we running a program(s) now?
            {
                pnt = cproc;			// Yes - try to resume them all
                cnt = MAXCHILD;
                do
                {   if (pnt->c_state > 0) // Is this process running?
                        svcSchIntrProc(pnt->c_pid, 4, 0); // Yes Resume it
                    pnt++;
                } while (-- cnt > 0);
            }
            return;						// Continue the Batch Job
        }
        else
        {
            if (childcnt)				// Are we running a program(s) now?
            {
                pnt = cproc;			// Yes - try to stop them all
                cnt = MAXCHILD;
                do
                {
                  if (pnt->c_state > 0) // Is this process running?
                        svcSchIntrProc(pnt->c_pid, 1, 0); // Yes - kill it
                    pnt++;
                } while (-- cnt > 0);
				bat_ctlc = TRUE;
				local_batch = FALSE;
				return;
            }
			else
			{
				bat_ctlc = TRUE;
				local_batch = FALSE;

				svcTrmFunction(STDTRM, TF_RESET); // Reset terminal to default
				longjmp(batrstbuf, 1);			  //   state
			}
        }
    }
    else
    {
        if (childcnt)			// Are we running a program(s) now?
        {
            pnt = cproc;		// Yes - try to stop them all
            cnt = MAXCHILD;
            do
            {
				if (pnt->c_state > 0)	// Is this process running?
                    if(!svcSchIntrProc(pnt->c_pid, 1, 0)) // Yes - kill it
						show_cc = TRUE;

                pnt++;
            } while (-- cnt > 0);
            svcSchCtlCDone();
			if (show_cc)
				fputs("^C\n", cmderr);
            return;
        }
        else				// If not running a program
        {
            svcSchCtlCDone();
            fputs("^C\n", cmderr);
            svcTrmFunction(STDTRM, TF_RESET);
					// Reset terminal to default state
            longjmp(rstbuf, 1);
        }
    }
}

// Function to service child died interrupt

void childgone(
    struct intdata intdata)

{
    struct cprocs *pnt;
    short  cnt;

    pnt = cproc;
    cnt = MAXCHILD;

#ifdef DEBUG
    printf("\n### Child died interrupt:\n"
           "###   pid  = %8.8lX term = %8.8lX pEIP = %8.8lX pCS  = %8.8lX\n"
           "###   pEFR = %8.8lX pos  = %8.8lX pseg = %8.8lX data = %8.8lX\n",
           intdata.pid, intdata.term, intdata.pEIP, intdata.pCS, intdata.pEFR,
           intdata. pos, intdata.pseg, intdata.data);
    printf("\nchildcnt = %d\n", childcnt);
    if (detached)
        fflush(cmderr);
#endif

    do
    {   if (pnt->c_pid == intdata.pid)	// This process?
        {
            pnt->c_trm = intdata.term >> 24;
            pnt->c_status = (intdata.term & 0x00800000L)?
                    intdata.term | 0xFF000000L: intdata.term & 0x00FFFFFFL;
            pnt->c_pEIP = intdata.pEIP;
            pnt->c_pCS = intdata.pCS & 0xFFFFL;
            pnt->c_pEFR = intdata.pEFR;
            pnt->c_offset = intdata.pos;
            pnt->c_segment = intdata.pseg & 0xFFFFL;
            pnt->c_data = intdata.data;
            pnt->c_state = C_KILLED;
            break;
        }
        pnt++;
    } while (-- cnt > 0);
    if (cnt == 0)
    {
        resetscrn();
        fprintf(cmderr, "\nUnknown process terminated, PID = %lu.%lu,"
                " Reason = %ld, Status = %6.6lX\n", intdata.pid>>16,
                intdata.pid&0xFFFF, intdata.term>>24, intdata.term&0xFFFFFFL);
	if (detached)
	    fflush(cmderr);
        return;         				// Just continue
    }
    if (--childcnt == 0)
    {
        pnt = cproc;
        cnt = MAXCHILD;
        do
        {
            if (pnt->c_state == C_KILLED) // Was this process just killed?
            {
                pnt->c_state = C_NONE;  // Yes
                childreport(pnt);
            }
            ++pnt;
        } while (-- cnt > 0);
        return;         				// Just continue
    }
}

// Function to report reason child died

void childreport(register struct cprocs *pnt)
{
    long  status;
    long  intnum;
    char *str1;
    char *str2;

    status = pnt->c_status;
    switch ((short)pnt->c_trm)
    {
	 case TC_EXIT:						// Process terminated with exit
	 case TC_KILL:						// Process killed
		break;

	 case TC_PIPE:						// Process terminated because pipe
		resetscrn();					//   had no more input
		fprintf(cmderr, "\n\x1B[KProcess %lu.%lu out of input data: "
				"status = %6.6lX\n\X1B[K", pnt->c_pid>>16,
				pnt->c_pid&0xFFFFL, status);
		if (detached)
		fflush(cmderr);
		break;

	 case TC_BDUSTK:					// Bad user stack address
		resetscrn();
		fprintf(cmderr, "\n\x1B[K? %s: Illegal user stack pointer in "
				"process %lu.%lu\n\x1B[K         "
				"CS:EIP = %4.4lX:%8.8lX, EFR = %8.8lX, "
				"SS:ESP = %4.4lX:%8.8lX\n\x1B[K", prgname, pnt->c_pid >> 16,
				pnt->c_pid&0xFFFFL, pnt->c_pCS, pnt->c_pEIP, pnt->c_pEFR,
				pnt->c_segment, pnt->c_offset);
		if (detached)
			fflush(cmderr);
		break;

	 case TC_NOMEM:						// No memory available
		resetscrn();
		fprintf(cmderr, "\n\x1B[K? %s: No memory available for process "
				"%lu.%lu\n\x1B[K         CS:EIP = %4.4lX:%8.8lX, "
				"EFR = %8.8lX\n\x1B[K", prgname, pnt->c_pid>>16,
				pnt->c_pid&0xFFFFL, pnt->c_pCS, pnt->c_pEIP, pnt->c_pEFR);
		if (detached)
			fflush(cmderr);
		break;

	 case TC_UNIMOP:					// Unimplemented operation
		resetscrn();
		fprintf(cmderr, "\n\x1B[K? %s: Unimplemented operation in process "
				"%lu.%lu\n         CS:EIP = %4.4lX:%8.8lX, EFR = %8.8lX"
				"\n\x1B[K         Data = %2.2X, %2.2X, %2.2X, "
				"%2.2X\n\x1B[K", prgname, pnt->c_pid>>16,
				pnt->c_pid&0xFFFFL, pnt->c_pCS, pnt->c_pEIP, pnt->c_pEFR,
				(int)(pnt->c_data&0xFF), (int)((pnt->c_data/0x100)&0xFF),
				(int)((pnt->c_data/0x10000L)&0xFF),
				(int)((pnt->c_data/0x1000000L)&0xFF));
		if (detached)
			fflush(cmderr);
		break;

	 case TC_MEMCRPD:					// DOS memory data corrupted
		resetscrn();
		fprintf(cmderr, "\n\x1B[K? %s: DOS memory allocation data corrupted "
				"in process %lu.%lu\n\x1B[K", prgname, pnt->c_pid>>16,
				pnt->c_pid&0xFFFFL);
		if (detached)
			fflush(cmderr);
		break;

	 case TC_RUNFAIL:					// Run svc failed
		resetscrn();
		fprintf(cmderr, "\n\x1B[K? %s: Run SVC failed in process "
				"%lu.%lu\n\x1B[K         CS:EIP = %4.4lX:%8.8lX, "
				"EFR = %8.8lX\n\x1B[K", prgname, pnt->c_pid>>16,
				pnt->c_pid&0xFFFFL, pnt->c_pCS, pnt->c_pEIP, pnt->c_pEFR);
		if (detached)
			fflush(cmderr);
		break;

	 case TC_SUBTSK:					// Sub-task error
		resetscrn();
		fprintf(cmderr, "\n\x1B[K? %s: Sub-task error in process "
				"%lu.%lu\n\x1B[K         CS:EIP = %4.4lX:%8.8lX, "
				"EFR = %8.8lX\n\x1B[K", prgname, pnt->c_pid>>16,
				pnt->c_pid&0xFFFFL, pnt->c_pCS, pnt->c_pEIP, pnt->c_pEFR);
		if (detached)
			fflush(cmderr);
		break;

	 case TC_DPMI:						// DPMI error
		resetscrn();
		fprintf(cmderr, "\n\x1B[K? %s: DPMI error in process "
				"%lu.%lu\n\x1B[K         CS:EIP = %4.4lX:%8.8lX, "
				"EFR = %8.8lX\n\x1B[K", prgname, pnt->c_pid>>16,
				pnt->c_pid&0xFFFFL, pnt->c_pCS, pnt->c_pEIP, pnt->c_pEFR);
		if (detached)
			fflush(cmderr);
		break;

	 case TC_NODOS:						// No DOS environment set up
		resetscrn();
		fprintf(cmderr, "\n\x1B[K? %s: No DOS environment available in process "
				"%lu.%lu\n\x1B[K         CS:EIP = %4.4lX:%8.8lX, "
				"EFR = %8.8lX\n\x1B[K", prgname, pnt->c_pid>>16,
				pnt->c_pid&0xFFFFL, pnt->c_pCS, pnt->c_pEIP, pnt->c_pEFR);
		if (detached)
			fflush(cmderr);
		break;

	 case TC_DOSBADE:					// Invalid DOS environment
		resetscrn();
		fprintf(cmderr, "\n\x1B[K? %s: Invalid DOS environment in process "
				"%lu.%lu\n\x1B[K         CS:EIP = %4.4lX:%8.8lX, "
				"EFR = %8.8lX\n\x1B[K", prgname, pnt->c_pid>>16,
				pnt->c_pid&0xFFFFL, pnt->c_pCS, pnt->c_pEIP, pnt->c_pEFR);
		if (detached)
			fflush(cmderr);
		break;

	 case TC_NOVECT:					// Uninitialized user vector
		resetscrn();
		switch ((int)status)
		{
		 case VECT_PILLINS:				// Illegal instruction
		 case VECT_RILLINS:
			str1 = "Illegal instruction";
			goto novect;

		 case VECT_PDIVERR:				// Divide error
		 case VECT_RDIVERR:
			str1 = "Divide error";
			goto novect;

		 case VECT_PBRKPNT:				// Breakpoint interrupt
		 case VECT_RBRKPNT:
			str1 = "Breakpoint trap";
			goto novect;

		 case VECT_PFPPNAVL:			// Coprocessor not available
		 case VECT_RFPPNAVL:
			str1 = "Coprocessor not available";
			goto novect;

		 case VECT_PFPUERR:				// Coprocessor error
		 case VECT_RFPUERR:
			str1 = "Coprocessor error";
			goto novect;

		 case VECT_PBOUND:				// BOUND instruction
		 case VECT_RBOUND:
			str1 = "BOUND instruction trap";
			goto novect;

		 case VECT_PINTO:				// INTO instruction
		 case VECT_RINTO:
			str1 = "INTO instruction interrupt";
			goto novect;

		 case VECT_CHILD:				// Child died
			str1 = "Child process terminated interrupt";
			goto novect;

		 case VECT_CNTC:				// Control-C
			str1 = "Control-C interrupt";
		 novect:
			fprintf(cmderr, "\n\x1B[K? %s: %s in process %lu.%lu"
					"\n\x1B[K         CS:EIP = %4.4lX:%8.8lX, "
					"EFR = %8.8lX\n\x1B[K", prgname, str1,
					pnt->c_pid>>16, pnt->c_pid&0xFFFFL, pnt->c_pCS,
					pnt->c_pEIP, pnt->c_pEFR);
			if (detached)
				fflush(cmderr);
			break;

		 case VECT_PPROT:				// Protection fault
		 case VECT_RPROT:
			str1 = "Protection";
			goto segerr;

		 case VECT_PSEGNP:				// Segment not present
			str1 = "Segment not present";
		 segerr:
			fprintf(cmderr, "\n\x1B[K? %s: %s exception in process "
					"%lu.%lu\n\x1B[K         CS:EIP = %4.4lX:%8.8lX, "
					"EFR = %8.8lX, Selector = %4.4lX\n\x1B[K",
					prgname, str1, pnt->c_pid>>16, pnt->c_pid&0xFFFL,
					pnt->c_pCS, pnt->c_pEIP, pnt->c_pEFR,
					pnt->c_segment & 0xFFFC);
			if (detached)
				fflush(cmderr);
			break;

		 case VECT_PPAGEFLT:			// Page fault
		 case VECT_RPAGEFLT:
			str1 = "Page fault";
			goto addrerr;

		 case VECT_PALNCHK:				// Alignment error
		 case VECT_RALNCHK:
			str1 = "Alignment";
		 addrerr:
			fprintf(cmderr, "\n\x1B[K? %s: %s exception in process "
					"%lu.%lu\n\x1B[K         CS:EIP = %4.4lX:%8.8lX, "
					"EFR = %8.8lX, Memory = %4.4lX:%8.8lX\n\x1B[K",
					prgname, str1, pnt->c_pid>>16, pnt->c_pid&0xFFFL,
					pnt->c_pCS, pnt->c_pEIP, pnt->c_pEFR,
					pnt->c_segment, pnt->c_offset);
			if (detached)
				fflush(cmderr);
			break;

		 default:
			intnum = status;
			if (status < 0x100)
			{
				str1 = "protected mode ";
				str2 = "interrupt";
			}
			else if (status < 0x120)
			{
				str1 = "protected mode ";
				str2 = "processor exception";
				intnum -= 0x100;
			}
			else if (status < 0x140)
			{
				str1 = "V86 mode ";
				str2 = "processor exception";
				intnum -= 0x120;
			}
			else if (status < 0x160)
			{
				str1 = "";
				str2 = "XOS exception";
				intnum -= 0x140;
			}
			else if (status >= 0x200 && status < 0x300)
			{
				str1 = "V86 mode ";
				str2 = "processor exception";
				intnum -= 0x200;
			}
			else
			{
				fprintf(cmderr, "\n\x1B[K? %s: Uninitialized signal "
						"0x%lX\n\x1B{K         in process %lu.%lu, "
						"CS:EIP = %4.4lX:%8.8lX, EFR = %8.8lX\n",
						prgname, status, pnt->c_pid>>16,
						pnt->c_pid&0xFFFFL, pnt->c_pCS, pnt->c_pEIP,
						pnt->c_pEFR);
				if (detached)
					fflush(cmderr);
				break;
			}
			fprintf(cmderr, "\n\x1B[K? %s: Uninitialized %s%s 0x%lX "
					"(signal 0x%lX)\n\x1B[K         in process "
					"%lu.%lu, CS:EIP = %4.4lX:%8.8lX, EFR = %8.8lX\n",
					prgname, str1, str2, intnum, status, pnt->c_pid>>16,
					pnt->c_pid&0xFFFFL, pnt->c_pCS, pnt->c_pEIP,
					pnt->c_pEFR);
			if (detached)
				fflush(cmderr);
			break;
		}
		break;

	 default:
		resetscrn();
		fprintf(cmderr, "\n\x1B[K? %s: Process %lu.%lu terminated "
				"with unknown termination type = %2.2lX\n\x1B[K",
				prgname, pnt->c_pid>>16, pnt->c_pid&0xFFFFL, pnt->c_trm);
		if (detached)
			fflush(cmderr);
		break;
	}
}

// Function to kill all detached children

void killdet(void)

{
}
