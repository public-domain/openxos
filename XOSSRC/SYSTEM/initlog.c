/******************************************/
/*                                        */
/* INITLOG.C - Log file routines for INIT */
/*                                        */
/******************************************/
/*                                        */
/* Written by John Goltz                  */
/*                                        */
/******************************************/

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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <utility.h>
#include <xossvc.h>
#include <xos.h>
#include <xostrm.h>
#include <xosmsg.h>
#include <xostime.h>
#include <xoserr.h>
#include <xosusr.h>
#include "init.h"

static int msgcnt;

static struct
{	time8_parm dt;
	uchar      end;
} loparms =
{	{PAR_GET|REP_DT, 8, IOPAR_CDATE}
};


//********************************************
// Function: initlog - Initialize the log file
// Returned: Nothing
//********************************************

void initlog(void)

{
	if ((logdev = svcIoOpen(O_IN|O_OUT|O_XWRITE|O_APPEND, "XOSACT:XOS.LOG",
			&loparms)) < 0)
	{
		if (logdev == ER_FILNF)
		{
			if ((logdev = svcIoOpen(O_IN|O_OUT|O_CREATE|O_XWRITE,
					"XOSACT:XOS.LOG", NULL)) > 0)
			{
				logdate = bgndt.date;
				putinlog(initpid, prgname, "Log file created: No log file "
						"when system started");
			}

		}
		return;
	}
	if (loparms.dt.value.date < bgndt.date) // Was the log file created today?
		rolllog(loparms.dt.value.date, "System started on different date");
	else
		logdate = loparms.dt.value.date;
}


void rolllog(
	long  date,
	char *text)


{
	time_s  dt;
	char   *pnt;
	char    bufr[128];

	dt.date = date;
	logdate = 0x7FFFFFFF;				// Needed to keep putinlog from going
										//   endlessly recurive!
	sprintf(bufr, "Log file terminated: %s", text);
	putinlog(initpid, prgname, bufr);
	pnt = bufr + sdt2str(bufr, "XOSACT:XOSLOG.%L-%r-%d", (time_sz *)&dt);
	svcIoClose(logdev, 0);
	while ((logdev = svcIoRename(0, "XOSACT:XOS.LOG", bufr, NULL)) < 0)
	{
		if (pnt[0] == 0)
		{
			pnt[0] = 'a';
			pnt[1] = 0;
		}
		else if (pnt[0] == 'z')
			return;
		else
			(pnt[0])++;
	}
	if ((logdev = svcIoOpen(O_IN|O_OUT|O_CREATE|O_XWRITE, "XOSACT:XOS.LOG",
			&loparms)) > 0)
	{
		sprintf(bufr, "Log file created: %s", text);
		putinlog(initpid, prgname, bufr);
	}
	logdate = loparms.dt.value.date;
}


//***************************************************
// Function: putinlog - Write data prefix to log file
// Returned: TRUE if OK, FALSE if error
//***************************************************

void putinlog(
    ulong procid,
    char *label,
    char *text)

{
	char   *dpnt;
	char   *spnt;
	time_sz dt;
	long    rtn;
	char    bufr[400];
	char    chr;

	if (logdev < 0)
		return;
	svcSysDateTime(T_GTXDTTM, &dt);		// Get current date and time
	if (logdate < dt.date)
		rolllog(logdate, "Date changed");
    ++msgcnt;
	bufr[0] = '$';
	dpnt = bufr + 1 + sdt2str(bufr + 1, "%D-%3n-%y %H:%m:%s.%f", &dt);
	dpnt += sprintf(dpnt, " %5d.%-3d %-8.8s ", procid >> 16, procid&0xFFFF,
			label);
	spnt = text;
	while ((chr = *spnt++) != 0 && dpnt < (bufr + 396))
	{
		if (chr == '\r')
		{
			*dpnt++ = '\r';
			*dpnt++ = '\n';
			if (*spnt == '\n')
				spnt++;
		}
		else if (chr == '\n')
		{
			*dpnt++ = '\r';
			*dpnt++ = '\n';
			if (*spnt == '\r')
				spnt++;
		}
		else
			*dpnt++ = chr;
	}
	*dpnt++ = '\r';
	*dpnt++ = '\n';
	if ((rtn = svcIoOutBlock(logdev, bufr, dpnt - bufr)) < 0)
		logdev = rtn;
	else
		logcommit = TRUE;
}


//*********************************************************
// Function: syslogdata -  Process data for system log file
// Returned: Nothing
//*********************************************************

void syslogdata(void)
{
    char buffer[200];

    msgbfr[(int)(msginpqab.qab_amount)] = '\0';
    switch (msgbfr[1])
    {
     case 1:							// Event at address
        sprintf(buffer, "%4.4s event, Address = %04.4X:%08.8lX, Data = %08.8lX",
                &(msgbfr[12]), *((short *)&srcbfr[3]), *((short *)&srcbfr[1]),
                *((short *)&msgbfr[20]), *((long *)&msgbfr[16]),
                *((long *)&msgbfr[22]));
        goto putin;

     case 2:							// Message from process
        sprintf(buffer, "Message from process \"%.160s\"", &msgbfr[12]);
        goto putin;

     case 3:							// Error from process
        putinlog(*((long *)&srcbfr[1]), &msgbfr[4], &msgbfr[12]);
        break;

	 case 0xFF:							// Roll log file
		if (msginpqab.qab_amount >= 12)
	        putinlog(*((long *)&srcbfr[1]), &msgbfr[4], &msgbfr[12]);
		rolllog(logdate, "Requested");
		break;

     default:
        sprintf(buffer, "Illegal log message #%d\r\n", &msgbfr[1]);
     putin:
        putinlog(*((long *)&srcbfr[1]), &msgbfr[4], buffer);
        break;
    }
}
