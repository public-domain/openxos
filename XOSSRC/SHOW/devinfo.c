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
#include <XOSSINFO.H>
#include <XOSSTR.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSERRMSG.H>

#include "SHOW.H"

extern char prgname[];

int devinfo(void)
{
    devinfo_data *blk;
    char         *pnt;
    long          rtn;
    char          name[256];
    char          buffer[400];

    ++validcount;
    for (;;)
    {
        if ((rtn=svcSysGetInfo(GSI_DEV, 0, NULL, 0)) < 0)
            femsg(prgname, rtn, NULL);	// See how much room we need
        if ((blk = malloc((int)(rtn+10) * sizeof(devinfo_data))) == NULL)
        {								// Get space for the data
            fputs("? SHOW: Not enough memory available\n", stderr);
            exit(1);
        }
        if ((rtn=svcSysGetInfo(GSI_DEV, 0, blk, (rtn+10)*64)) < 0)
        {								// Get the data
            if (rtn == ER_DATTR)		// Error - was it truncated?
            {
                free(blk);				// Yes
                continue;				// Try again
            }
            else
                femsg(prgname, rtn, NULL);
        }
        break;
    }
    fputs("Class    Device   Type  Sesn Open Out In  Device\n"
          "Name     Name     Name  Proc Cnt  Cnt Cnt Information\n", stdout);
    name[255] = 0;
    while (--rtn >= 0)
    {
        pnt = buffer + sprintf(buffer, "%-8.8s", blk->cname);
        if (blk->dname[0] != '\0')
        {
            pnt += sprintf(pnt, " %-8.8s %-4.4s ", blk->dname, blk->tname);
            if (blk->gpid != 0)
                pnt += sprintf(pnt, " %-4ld", blk->gpid);
            else
				pnt = strmov(pnt, "     ");
			pnt += sprintf(pnt, " %-4d %-3d %-3d", blk->ocount, blk->outcnt,
		    blk->incnt);
            name[0] = 0;
            svcSysGetInfo(GSI_DEV, blk->dcbid, name, 255);
            if (name[0] != 0)
			{
				*pnt++ = ' ';
				pnt = strmov(pnt, name);
			}
        }
		*pnt++ = '\n';
		*pnt = 0;
		fputs(buffer, stdout);
        ++blk;
    }
    return (TRUE);
}
