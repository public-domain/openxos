/*
 * INIT - log file routines
 *
 * Written by John Goltz
 */

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
#include <TIME.H>

#include <UTILITY.H>
#include <XOSSVC.H>
#include <XOS.H>
#include <XOSTRM.H>
#include <XOSMSG.H>
#include <XOSTIME.H>
#include <XOSERR.H>
#include <XOSUSR.H>
#include "INIT.H"
#include "INITEXT.H"

int msgcnt;
int lasterr;

/********************************************************/
/* Function: putinlog - Write data prefix to log file	*/
/* Returned: TRUE if OK, FALSE if error			*/
/********************************************************/

int putinlog(
    ulong procid,
    char *label,
    char *text)

{
    char bufr[30];

    if (!syslogopen)
    {
        if ((logdev = fopen("XOSACT:XOS.LOG", "a+")) == NULL)
					// Open log file
        {				// If error
            if (errno == -ER_FILNF)	// Was the error file not found?
            {				// Yes - try to create the log file
                if ((logdev = fopen("XOSACT:XOS.LOG", "w")) == NULL)
                    return (FALSE);	// If can't create it
            }
            else			// If error other than file not found
                return (FALSE);
        }
        syslogopen = TRUE;
    }

    ++msgcnt;

    if (!startupdone)
    {
        char strtbfr[140];

        startupdone = TRUE;
        sprintf(strtbfr, "%s startup (RAM = %dKb)",
                (char near *)(cfgchar.sysname.buffer), cfgchar.memtotal.value);
        putinlog(initpid, prgname, strtbfr); // Put startup message in the log
    }
    svcSysDateTime(2, &dttm);		// Get current date and time
    ddt2str(bufr, "%D-%3n-%y %H:%m:%s.%f", &dttm);
    if ((lasterr=fprintf(logdev, "$ %s %5ld.%-3ld %-8.8s %s\n", bufr,
            procid>>16, procid&0xFFFF, label, text)) < 0)
        return (FALSE);
    return (TRUE);
}

/****************************************/
/* Function: closelog - Close log file	*/
/* Returned: Nothing			*/
/****************************************/

void closelog(void)
{
    if (syslogopen)
    {
        fclose(logdev);
        syslogopen = FALSE;
    }
}

/****************************************************************/
/* Function: syslogdata -  Process data for system log file	*/
/* Returned: Nothing						*/
/****************************************************************/

void syslogdata(void)
{
    char buffer[200];

    switch (msgbfr[1])
    {
     case 1:			// Event at address
        sprintf(buffer, "%4.4s event, Address = %04.4X:%08.8lX, Data = %08.8lX",
                &(msgbfr[12]), *((short *)&srcbfr[3]), *((short *)&srcbfr[1]),
                *((short *)&msgbfr[20]), *((long *)&msgbfr[16]),
                *((long *)&msgbfr[22]));
        goto putin;

     case 2:			// Message from process
        msgbfr[(int)(msginpqab.qab_amount)] = '\0';
        sprintf(buffer, "Message from process \"%.160s\"", &msgbfr[12]);
        goto putin;

     case 3:			// Error from process
        msgbfr[(int)(msginpqab.qab_amount)] = '\0';
        putinlog(*((long *)&srcbfr[1]), &msgbfr[4], &msgbfr[12]);
        break;

     default:
        sprintf(buffer, "Illegal log message #%d\r\n", &msgbfr[1]);
     putin:
        putinlog(*((long *)&srcbfr[1]), &msgbfr[4], buffer);
        break;
    }
    closelog();
}
