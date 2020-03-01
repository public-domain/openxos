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

long  rtn;
long  disk;
long  amount;
char  prgname[] = "RDDISK";


char buffer[512];


void main(
    int   argc,
    char *argv[])

{
    if (argc != 2)
    {
        fputs("? Command error, usage is:\n"
              "    RDDISK filespec\n", stderr);
        exit(1);
    }

    if ((disk = svcIoOpen(O_IN|O_PHYS, argv[1], NULL)) < 0)
        femsg2(prgname, "Error opening input file", disk, argv[1]);
    for (;;)
    {
	if ((amount = svcIoInBlock(disk, buffer, 512)) <= 0)
	{
	    if (amount != ER_BDDBK)
		femsg2(prgname, "Error reading input file", amount, NULL);
	    if ((amount = svcIoSetPos(disk, 0, 0)) < 0)
		femsg2(prgname, "Error resettng input file", amount, NULL);
	}
    }
}
