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

long mdbhndl;
long rtn;
char coinr[] = {0x08, 0x00};
char billr[] = {0x30, 0x00};

char coins[] = {0x09, 0x00};
char bills[] = {0x31, 0x00};

char coint[] = {0x0C, 0x00, 0x07, 0x00, 0x07, 0x00};
char billt[] = {0x34, 0x00, 0x0F, 0x00, 0x0F, 0x00};

char coinp[] = {0x0B, 0x00};
char billp[] = {0x33, 0x00};

char billrtn[] = {0x35, 0x00, 0x00};

char prgname[] = "MDBTEST";


void main(void)

{
    uchar *pnt;
    int    cnt;
    uchar  resp[64];

    if ((mdbhndl = svcIoOpen(O_IN|O_OUT, "MDBA0:", NULL)) < 0)
	femsg2(prgname, "Error opening MDB device MDBA0", mdbhndl, NULL);

    svcSchSuspend(NULL, XT_SECOND);

    if ((rtn = sendblk(coinr, sizeof(coinr), resp, 8)) < 0)
	femsg2(prgname, "Error sending data to MDBA0", rtn, NULL);

    svcSchSuspend(NULL, XT_SECOND);

    if ((rtn = sendblk(billr, sizeof(billr), resp, 8)) < 0)
	femsg2(prgname, "Error sending data to MDBA0", rtn, NULL);

    svcSchSuspend(NULL, XT_SECOND);

    if ((rtn = sendblk(coins, sizeof(coins), resp, 8)) < 0)
	femsg2(prgname, "Error sending data to MDBA0", rtn, NULL);

    svcSchSuspend(NULL, XT_SECOND);

    if ((rtn = sendblk(bills, sizeof(bills), resp, 8)) < 0)
	femsg2(prgname, "Error sending data to MDBA0", rtn, NULL);

    svcSchSuspend(NULL, XT_SECOND);

    if ((rtn = sendblk(coint, sizeof(coint), resp, 8)) < 0)
	femsg2(prgname, "Error sending data to MDBA0", rtn, NULL);

    svcSchSuspend(NULL, XT_SECOND);

    if ((rtn = sendblk(billt, sizeof(billt), resp, 8)) < 0)
	femsg2(prgname, "Error sending data to MDBA0", rtn, NULL);

    svcSchSuspend(NULL, XT_SECOND);

    if ((rtn = sendblk(bills, sizeof(bills), resp, 8)) < 0)
	femsg2(prgname, "Error sending data to MDBA0", rtn, NULL);

    for (;;)
    {
	svcSchSuspend(NULL, XT_SECOND/20);

	if ((rtn = sendblk(coinp, sizeof(coinp), resp, 2)) < 0)
	    errmsg(0x70, NULL, rtn, "Coin: Error polling device", NULL);
	else if (rtn > 0)
	{
	    printf("Coin: (%d) %02.2X", rtn, resp[0]);
	    cnt = rtn;
	    pnt = resp + 1;
	    while (--cnt > 0)
		printf(" %02.2X", *pnt++);
	    printf("\n");
	}

	svcSchSuspend(NULL, XT_SECOND/20);

	if ((rtn = sendblk(billp, sizeof(billp), resp, 2)) < 0)
	    errmsg(0x70, NULL, rtn, "Bill: Error polling device", NULL);
	else if (rtn > 0)
	{
	    printf("Bill: (%d) %02.2X", rtn, resp[0]);
	    cnt = rtn;
	    pnt = resp + 1;
	    while (--cnt > 0)
		printf(" %02.2X", *pnt++);
	    printf("\n");

	    if ((resp[0] & 0xF0) == 0x90)
	    {
		billrtn[1] ^= 0x01;
		sendblk(billrtn, sizeof(billrtn), resp, 6);
	    }
	}
    }
}
