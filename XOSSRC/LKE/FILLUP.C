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

long *pnt;
int   rtn;
int   disk;
int   num;
int   block;
int   amount;
int   block;
int   filecnt;
int   filesize;
int   left;
int   cnt;
char  buffer[512];
char  prgname[] = "FILLUP";


void main(
    int   argc,
    char *argv[])

{
    if (argc != 4)
    {
        fputs("? Command error, usage is:\n"
              "    FILLUP numfiles size filespec\n", stderr);
        exit(1);
    }

    filecnt = atoi(argv[1]);
    filesize = atoi(argv[2]);

    while (--filecnt >= 0)
    {
	sprintf(buffer, "%s%d", argv[3], num++);
	printf("File: %s", buffer);
	if ((disk = svcIoOpen(O_OUT|O_CREATE|O_TRUNCA, buffer,  NULL)) < 0)
	    femsg2(prgname, "Error opening output file", disk, buffer);
	block = 0;
	left = filesize;
	while (left > 0)
	{
	    if ((amount = left) > 512)
		amount = 512;
	    cnt = (512 - 128)/4;
	    pnt = (long *)(buffer + 128);
	    do
	    {
		*pnt++ = block;
	    } while (--cnt > 0);
	    if ((rtn = svcIoOutBlock(disk, buffer, amount)) < 0)
		femsg2(prgname, "Error writing output file", rtn, buffer);
	    left -= amount;
	    block++;
	}
	if ((rtn = svcIoClose(disk, 0)) < 0)
	    femsg2(prgname, "Error closing output file", rtn, buffer);
	fputs(", done\n", stdout);
    }
    fputs("All done\n", stdout);
}
