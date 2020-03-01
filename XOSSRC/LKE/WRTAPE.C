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

#define SDF_TAPE_UNLOAD    1	// Unload tape
#define SDF_TAPE_REWIND    2	// Rewind tape
#define SDF_TAPE_FORMAT    3	// Format tape
#define SDF_TAPE_RETEN     4	// Retension tape
#define SDF_TAPE_WRITEFM   5	// Write filemarks
#define SDF_TAPE_WRITESM   6	// Write setmarks
#define SDF_TAPE_LOCK      7	// Lock/unlock tape mounting
#define SDF_TAPE_ERASEGAP  8	// Erase gap
#define SDF_TAPE_ERASEALL  9	// Erase gap
#define SDF_TAPE_SKPREC   10	// Skip records
#define SDF_TAPE_SKPFILE  11	// Skip filemarks
#define SDF_TAPE_CONFILE  12	// Skip to consective filemarks
#define SDF_TAPE_SKPSET   13	// Skip setmarks
#define SDF_TAPE_CONSET   14	// Skip to consective setmarks
#define SDF_TAPE_SKP2EOD  15	// Skip to end-of-data

long  rtn;
long  disk;
long  tape;
long  amount;
long  size;
long  total;
char  prgname[] = "WRTAPE";
char  tapename[] = "TAP1:";

struct sdfp
{   text8_parm class;
    char       end;
} sdfp =
{   {PAR_SET|REP_TEXT, 8, IOPAR_CLASS, "TAPE"}
};

char buffer[32*1024];


void finish(void);

void main(
    int   argc,
    char *argv[])

{
    if (argc != 2)
    {
        fputs("? Command error, usage is:\n"
              "    WRTAPE filespec\n"
              "  Output is to TAP1:\n", stderr);
        exit(1);
    }

    if ((tape = svcIoOpen(O_OUT, tapename, NULL)) < 0)
        femsg2(prgname, "Error opening tape device", tape, tapename);
    if ((disk = svcIoOpen(O_IN, argv[1], NULL)) < 0)
        femsg2(prgname, "Error opening input file", disk, argv[1]);

    if ((rtn = svcIoSpecial(tape, SDF_TAPE_REWIND, NULL, 0, &sdfp)) < 0)
        femsg2(prgname, "Error rewinding tape", rtn, tapename);

    for (;;)
    {
        if ((amount = svcIoInBlock(disk, buffer, 32*1024)) < 0)
        {
            if (amount != ER_EOF)
                femsg2(prgname, "Error reading input file", amount, argv[1]);
            finish();
        }
        size = amount;
        if (size & 0x1FF)
        {

            size = (size & ~0x1FF) + 0x200;

            printf("### amount = %d, %d\n", amount, size-amount);

            memset(buffer+amount, 0, size-amount);
        }
        if ((rtn = svcIoOutBlock(tape, buffer, size)) < 0)
            femsg2(prgname, "Error writing tape", rtn, tapename);
        total += rtn;
        if (amount < (32*1024))
            break;
    }
    finish();
}

void finish(void)

{
    if ((rtn = svcIoSpecial(tape, SDF_TAPE_WRITEFM, NULL, 1, &sdfp)) < 0)
        femsg2(prgname, "Error writing final filemark to tape", rtn, tapename);

    printf("All done, %d bytes written\n", total);
    exit(0);
}
