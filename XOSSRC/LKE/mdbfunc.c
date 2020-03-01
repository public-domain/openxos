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
#include <XOSSTR.H>
#include <XOS.H>
#include <XOSERMSG.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSTIME.H>
#include "MDB.H"


static void warn(char *msg, char *text);


//***********************************************************
// Function: sendblk - Send a data block and get the response
// Returned: Length of response block (positive) if normal or
//		negative error code if error
//***********************************************************

int sendblk(
    uchar *msg,
    int    length,
    uchar *resp,
    int    retry)

{
    int    cnt;
    int    size;
    int    rtn;
    uchar *pnt;
    uchar  chksum;

    static uchar ackmsg[] = {0x00};
    static uchar retmsg[] = {0xAA};

    cnt = length;
    pnt = msg;
    chksum = 0;
    do
    {
	chksum += *pnt++;
    } while (--cnt > 1);
    msg[length - 1] = chksum;
    do
    {
	if ((rtn = svcIoOutBlock(mdbhndl, msg, length)) < 0)
	    return (rtn);
	if ((size = svcIoInBlock(mdbhndl, resp, 64)) < 0)
	{
	    if (size != ER_NORSP && size != ER_LSTER)
		return (size);
	    if (size == ER_LSTER)
	    {
		warn(msg, "Data error!");
		svcSchSuspend(NULL, XT_SECOND/10);
	    }
	    else
		warn(msg, "No response!");
	    continue;
	}
	if (size == 1)
	{
	    if (resp[0] == 0x00)	// ACK?
		return (0);		// Yes
	    svcSchSuspend(NULL, XT_SECOND/10); // No - treat is like a NAK!
	    {
		char bufr[100];

		sprintf(bufr, "Non-ACK single byte response: %02.2X", resp[0]);
		warn(msg, resp);
	    }
	    continue;
	}
	cnt = size;
	pnt = resp;
	chksum = 0;
	do
	{
	    chksum += *pnt++;
	} while (--cnt > 1);
	if (resp[size - 1] != chksum)
	{
	    warn(msg, "CHK is wrong!");
	    msg = retmsg;
	    length = 1;
	    continue;
	}
	if ((rtn = svcIoOutBlock(mdbhndl, ackmsg, 1)) < 0)
	    return (rtn);
	return (size - 1);
    } while (--retry > 0);
    return (ER_NOACK);
}

static void warn(
    char *msg,
    char *text)

{
    printf("%s: %s\n", (msg[0] < 30) ? "Coin" : "Bill", text);
}
