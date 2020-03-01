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
#include <XCSTRING.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSERMSG.H>

long disk;

char prgname[] = "WRTDSK";

struct
{   byte4_parm block;
    char       end;
} outparms =
{   {PAR_SET|REP_DECV, 4, IOPAR_ABSPOS, 0}
};

long buffer[128];

void main(
    int   argc,
    char *argv[])

{
    long rtn;
    long block;

    if (argc != 3)
    {
        fputs("? Command error, usage is:\n"
              "    WRTDSK disk: begin\n", stderr);
        exit(1);
    }

    if ((disk = svcIoOpen(O_PHYS|O_OUT, argv[1], NULL)) < 0)
        femsg2(prgname, "Error opening disk", disk, argv[1]);

    block = atoi(argv[2]);
    outparms.block.value = block * 512;
    for (;;)
    {
        block = outparms.block.value/512;
        buffer[0] = block;
        if ((rtn = svcIoOutBlockP(disk, (char *)buffer, 512, &outparms)) < 0)
            femsg2(prgname, "Error writing disk", rtn, argv[1]);
        printf("%8d\r", block);
        outparms.block.value += 512;
    }
}
