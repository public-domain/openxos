/***********************************************/
/*                                             */
/* INITTERM.C - Terminal data handler for INIT */
/*                                             */
/***********************************************/
/*                                             */
/* Written by John Goltz                       */
/*                                             */
/***********************************************/

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
#include <ctype.h>
#include <time.h>
#include <xcstring.h>
#include <utility.h>
#include <xossvc.h>
#include <xos.h>
#include <xostrm.h>
#include <xostime.h>
#include <xosmsg.h>
#include <xosusr.h>
#include "init.h"

//**************************************************
// Function: termdata - Handle terminal data message
// Returned: Nothing
//**************************************************

// Format of the terminal data message:
//  Offset    Use
//     0    Message type (MT_TERMDATA)
//     1    Reason - 0 = data, 1 = carrier detect, 2 = ring detect
//     2    Data byte if reason = 1, unused if reason = 0
//     3    Rate detect type (0 = fixed rate)
//     4    First byte of terminal name

void termdata(void)

{
    long  dev;
    long  rtn;
    char *pnt;
    char  name1[50];
    char  name2[50];
    char  buffer[200];
    char  chr;

    msgbfr[(int)(msginpqab.qab_amount)] = ':';
    msgbfr[(int)(msginpqab.qab_amount)+1] = '\0';
    if ((dev=svcIoOpen(O_PHYS|O_IN|O_OUT, &msgbfr[4], NULL)) < 0)
        return;

    if (msgbfr[1] == 0)			// If have data
    {
        if (msgbfr[2] != '\r')		// It must be CR!
        {
            svcIoClose(dev, 0);	// Else ignore this!
            return;
        }
    }
    svcTrmFunction(dev, TF_CLROUT);
    svcIoOutStringP(dev, welcome, 0, &trmclrparms); // Output initial banner
    svcTrmFunction(dev, TF_CLRINP);	// Clear terminal input buffers

    pwqab.qab_handle = dev;
    if (svcIoQueue(&pwqab) >= 0 && pwqab.qab_error >= 0 && pwbufr[0] != 0)
    {
        runqab.qab_buffer1 = "XOSSYS:LOGIN.RUN";
        runparm.arglist.buffer = "CMD:SHELL ";
    }
    else
    {
        if (prgmbufr[0] == 0)
            strmov(prgmbufr, "SHELL");
        pnt = prgmbufr;
        while ((chr = *pnt++) != 0 && chr != ':')
            ;
        pnt = name1;
        if (chr == 0)
            pnt = strmov(pnt, "CMD:");
        runparm.arglist.buffer = name1;
        runparm.arglist.bfrlen = runparm.arglist.strlen = strlen(name1);
        strmov(pnt, prgmbufr);                        
        strmov(strmov(name2, name1), ".RUN");
        runqab.qab_buffer1 = name2;
    }
    runqab.qab_option = R_SESSION;
    runparm.devlist.desp = PAR_SET|REP_STR;
    runparm.devlist.bfrlen = runparm.devlist.strlen = 4*16;
    devlist.dlstdin.src = dev;
    devlist.dlstdout.src = dev;
    devlist.dlstderr.src = dev;
    devlist.dlstdtrm.src = dev | 0xC0000000L;
    runparm.arglist.bfrlen = runparm.arglist.strlen = 14;
    runparm.arglist.buffer = "CMD:SHELL ";
    runparm.arglist.bfrlen = runparm.arglist.strlen = 12;
    if ((rtn=svcIoRun(&runqab)) < 0 || (rtn=runqab.qab_error) < 0)
    {
        char errbufr[100];

        svcSysErrMsg(rtn, 3, errbufr);	// Get error message string
        rtn = sprintf(buffer, STR_MT_SYSLOG"---INIT    ----Error loading "
				"command processor %s for %s\n%s", name2, &msgbfr[4], errbufr);
		*(long *)&buffer[12] = initpid;	// Log the error
		svcSysLog(buffer, rtn);
        sprintf(buffer, "? INIT: Error loading command processor %s for %s\r\n"
                        "        %s\r\n\r\n",
                name2, &msgbfr[4], errbufr);
        svcIoOutString(dev, buffer, 0); // Tell the user
        svcIoClose(dev, 0);
    }
    else
	{
        rtn = sprintf(buffer, STR_MT_SYSLOG"---PROCCRTD----Level 1 process "
				"created using %s for %s", name2, &msgbfr[4]);
		*(long *)&buffer[12] = runqab.qab_amount & 0xFFFF0FFF;
		svcSysLog(buffer, rtn);
	}
}
