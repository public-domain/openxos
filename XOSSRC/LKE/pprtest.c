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
long  ppr;
long  amount;
long  size;
long  total;
char  prgname[] = "PPRTEST";
char  pprname[] = "PPR1:";


uchar buffer[448*40/8+20];


void main(
    int   argc,
    char *argv[])

{
    uchar *pnt;
    int    cnt;


    cnt = (argc > 1) ? atoi(argv[1]) : 3;

    if ((ppr = svcIoOpen(O_OUT, pprname, NULL)) < 0)
        femsg2(prgname, "Error opening printer device", ppr, pprname);
    
    pnt = buffer;
    *pnt++ = 0x1D;
    *pnt++ = 0x45;
    *pnt++ = cnt;
    *pnt++ = 0x1B;
    *pnt++ = 0x2A;
    *pnt++ = 0x62;
    *pnt++ = 0x28;
    *pnt++ = 0x00;

    cnt = (448*40/8)/2;
    do
    {
	*pnt++ = 0xFF;
	*pnt++ = 0;
    } while (--cnt > 0);

    svcIoOutBlock(ppr, buffer, pnt - buffer);

///    svcIoOutBlock(ppr, buffer, pnt - buffer);
///    svcIoOutBlock(ppr, buffer, pnt - buffer);
///    svcIoOutBlock(ppr, buffer, pnt - buffer);


}
