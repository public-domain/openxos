/***************************************/
/* INIT	- Main program                 */
/* General process initializer for XOS */
/***************************************/
/* Written by John Goltz               */
/***************************************/

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
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <xcstring.h>
#include <utility.h>
#include <xossvc.h>
#include <xos.h>
#include <xostrm.h>
#include <xosvect.h>
#include <xosmsg.h>
#include <xoserr.h>
#include <xostime.h>
#include <xossysp.h>
#include <xosusr.h>
#include "init.h"

extern int errno;

#define TRMCLASS 0x4D5254L

// Main program

static char conname[] = "TRM0S1:";
static char msgname[] = "IPM:SYS^INIT";

#define VERSION 1
#define EDITNO  5

// 1.5  18-Apr-99
//	Changed names so all modules begin with INIT; removed some obsolete
//	stubs.


void mainalt(
    img_data *args)

{
	time_sz dt;
	long    rtn;
    char    buffer[160];

    svcMemChange(args, 0, 0);			// Give up the argument msect
    initpid = svcSysGetPid();			// Get the INIT process ID
    svcIoQueue(&cfgqab);
    svcSysDateTime(T_GTXDTTM, &bgndt);	// Get current date and time

    sprintf(welcome, "\r\n%s\r\n", sysname);
    if ((conhndl = svcIoOpen(O_IN|O_OUT|O_PHYS, conname, NULL)) < 0)
		exit(1);
    sdt2str(dttmbfr, "%z%T:%m%a on %w, %D-%3n-%l", &bgndt);
    sprintf(buffer, "INIT - version %d.%d\r\nSystem initialization started "
			"at %s\r\nTotal memory is %dKB\r\n", VERSION, EDITNO, dttmbfr,
			cfgchar.memtotal.value);
    svcIoOutString(conhndl, buffer, 0);
    setvector(VECT_MESSAGE, 0x04, message); // Set message received vector
    if ((msginpqab.qab_handle = svcIoOpen(O_IN|O_OUT, msgname, NULL)) < 0)
    {									// Cannot open IPM:SYS^INIT
        svcSysErrMsg(msginpqab.qab_handle, 3, msgbfr);
        sprintf(buffer, ipmerrmsg, msgbfr);
        svcIoOutString(conhndl, buffer, 0);
    }
    else
    {
        msginpparm.srcname.bfrlen = 64;
        if ((rtnval=svcIoQueue(&msginpqab)) < 0 ||
                (rtnval=msginpqab.qab_error) < 0)
        {								// Cannot start IPM: input
            svcSysErrMsg(rtnval, 3, msgbfr);
            sprintf(buffer, "? INIT: Cannot start message input on "
					"IPM:SYS^INIT\r\n        %s\r\n", msgbfr);
            svcIoOutString(conhndl, buffer, 0);
        }
    }
    setvector(VECT_CHILD, 0x04, childgone); // Set child died signal vector
	setvector(VECT_SECOND, 0x04, onceasecond); // Set the once-a-second vector
	svcSchAlarm(4, 0, VECT_SECOND, 0, 0, XT_SECOND); // Request once-a-second
													 //   signal
    svcSchSetLevel(0);					// Enable signals

	if (!dorun("XOSCMD:SYMBIONT.RUN", "XOSCMD:SYMBIONT LOGGER"))
	{
		sprintf(buffer, "? INIT: Error loading logging symbiont "
				"(XOSCMD:LOGGER.RUN)\r\n        %s\r\n\r\n", msgbfr);
		svcIoOutString(conhndl, buffer, 0);
	}
	else if (!dorun("XOSCMD:SERVER.RUN", "XOSCMD:SERVER LOGGER ADD INS=1 "
			"NAME=XOSACT:XOS"))
	{
		sprintf(buffer, "? INIT: Error starting logging symbiont "
				"(XOSCMD:LOGGER.RUN)\r\n        %s\r\n\r\n", msgbfr);
		svcIoOutString(conhndl, buffer, 0);
	}
	rtn = sprintf(buffer, "----INIT    ----%s (INIT v %d.%d) startup "
			"(RAM = %dKb)", sysname, VERSION, EDITNO, cfgchar.memtotal.value);
	*(long *)&buffer[12] = initpid;		// Put initial message in the log
	svcSysLog(buffer, rtn);
	if (!dorun("XOSCFG:STARTUP.BAT", "XOSCFG:STARTUP "))
	{
        sprintf(buffer, "? INIT: Cannot execute XOSCFG:\\STARTUP.BAT\r\n"
		"        %s\r\n", msgbfr);
        svcIoOutString(conhndl, buffer, 0);
    }

	// Make a final log entry

	rtn = strmov(buffer, "----INIT    ----System initialization complete") -
			buffer;
	*(long *)&buffer[12] = initpid;
	svcSysLog(buffer, rtn);

    startupdone = TRUE;					// Startup message has been output
    svcSysDateTime(T_GTXDTTM, &dt);		// Get current date and time
    sdt2str(buffer, "INIT: XOS System initialization complete at "
			"%z%T:%m%a on %w, %D-%3n-%l\r\n", &dt);
    svcIoOutString(conhndl, buffer, 0);
    cfgqab.qab_buffer2 = (char *)&statechar; // Set system state to normal
    svcIoQueue(&cfgqab);

	// Need to check if LOGIN required on TRM0S1: and not launch SHELL if req'd

    if (statechar.initial.value == 0x00534559L)
    {
		runqab.qab_buffer1 = "XOSCMD:SHELL.RUN";
		runqab.qab_option = R_SESSION;
		runparm.devlist.desp = PAR_SET|REP_STR;
		devlist.dlstdin.src = conhndl;	// Set up the device list
		devlist.dlstdout.src = conhndl;
		devlist.dlstderr.src = conhndl;
		devlist.dlstdtrm.src = conhndl | 0xC0000000L;
		runparm.devlist.bfrlen = runparm.devlist.strlen = 4*16;
		runparm.arglist.buffer = "XOSCMD:SHELL ";
		runparm.arglist.bfrlen = runparm.arglist.strlen = 12;
		if ((rtnval = svcIoRun(&runqab)) < 0 ||
				(rtnval = runqab.qab_error) < 0)
		{
			svcSysErrMsg(rtnval, 3, msgbfr); // Get error message string
			sprintf(buffer, "? INIT: Error loading command processor "
					"(XOSCMD:SHELL.RUN)\r\n        %s\r\n\r\n", msgbfr);
			svcIoOutString(conhndl, buffer, 0);
		}
	}
	svcIoClose(conhndl, 0);				// Free up screen 0 on the console if
										//   we still have it
    svcSchSuspend(NULL, 0xFFFFFFFFL);	// Wait for something to happen
}   



int dorun(
	char *prg,
	char *tail)

{

	runqab.qab_buffer1 = prg;
	runqab.qab_option = R_CHILDTERM;
	runparm.devlist.desp = PAR_SET|REP_STR;
	devlist.dlstdin.src = conhndl;	// Set up the device list
	devlist.dlstdout.src = conhndl;
	devlist.dlstderr.src = conhndl;
	devlist.dlstdtrm.src = conhndl;
	runparm.devlist.bfrlen = runparm.devlist.strlen = 4*16;
	runparm.arglist.buffer = tail;
	runparm.arglist.bfrlen = runparm.arglist.strlen = strlen(tail);
	if ((rtnval = svcIoRun(&runqab)) < 0 ||
			(rtnval = runqab.qab_error) < 0)
	{
		svcSysErrMsg(rtnval, 3, msgbfr); // Get error message string
		return (FALSE);
	}
	return (TRUE);
}



// Signal routine called once each second

void onceasecond(void)

{
/*	time_s dt;

	if (--mincnt < 0)
	{
		mincnt = 60;
		svcSysDateTime(T_GTXDTTM, &dt);
		if (logdate < dt.date)
			rolllog(logdate, "Date changed");
	}
	if (logdev > 0 && logcommit)
	{
		svcIoCommit(logdev);
		logcommit = FALSE;
	}
*/
}

// Function to service message available interrupt

void message(void)

{
    for (;;)
    {
        switch (msgbfr[0])
        {
		 case MT_SYMBREQ:				// Symbiont request
			symbiontreq();
			break;

///		 case MT_SYSLOG:				// Data for system log
///			syslogdata();
///			break;

		 case MT_TERMDATA:				// Data from idle terminal
			termdata();
			break;
		}
		msginpqab.qab_vector = 0;
		msginpparm.srcname.bfrlen = 64;
		if (svcIoQueue(&msginpqab) >= 0)
		{
			msginpqab.qab_vector = VECT_MESSAGE;
			if (msginpqab.qab_status & QSTS_DONE)
				continue;
		}
		break;
    }
}										//   was written to it

// Function to store 32 bit value as 8 hex digits
/*
char *storehex(pnt, value)
char *pnt;
ulong value;

{
    int  cnt;
    char val;

    cnt = 8;
    while (--cnt >= 0)
    {
        value = rotate32(value, -4);
        val = (value & 0xF);
        if (val > 9)
            val += 'A'-'0'-10;
        *pnt++ = val + '0';
    }
    return (pnt);
}
*/
// Function to send response

void response(header, msg)
char *header;
char *msg;

{
    strmov(strmov(strmov(msgbfr, header), "INIT: "), msg);
    svcIoOutBlockP(msginpqab.qab_handle, msgbfr, strlen(msgbfr), &msgoutparm);
}
