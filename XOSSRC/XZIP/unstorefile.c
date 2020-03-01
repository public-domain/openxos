//==============================================
// UNSTOREFILE.C
// Unstore file from a ZIP file (part of XUNZIP)
//
// Written by: John Goltz
//
//==============================================

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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <xoserr.h>
#include <xostrm.h>
#include <xosrtn.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <xoserrmsg.h>
#include <progarg.h>
#include <proghelp.h>
#include <global.h>
#include <xosstr.h>
#include <xoszipfile.h>
#include <zlib.h>
#include "xunzip.h"


long unstorefile(
    int xfer)

{
    long amount;
    long rtn;

	crcvalue = 0;
	while (cdheader.comsize > 0)
	{
		amount = (cdheader.comsize < bufrsize) ? cdheader.comsize :
				bufrsize;
		cdheader.comsize -= amount;
		if ((rtn = svcIoInBlockP(zipdev, inbufr, amount, &inparms)) !=
				amount)
		{
			if (rtn >= 0)
				rtn = ER_EOF;
			errtext = "Error reading data for";
			errname = filename;
			return (rtn);
		}
		inparms.pos.value += amount;
		if (xfer)
		{
			if ((rtn = svcIoOutBlock(filedev, inbufr, amount)) != amount)
			{
				if (rtn >= 0)
					rtn = ER_ICMIO;
				errtext = "Error writing";
				errname = dstname;
				return (rtn);
			}
		}
		crcvalue = crc32(crcvalue, inbufr, amount);
	}
	return ((crcvalue == cdheader.crc32) ? 0 : 1);
}
