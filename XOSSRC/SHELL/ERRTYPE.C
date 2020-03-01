//------------------------------------------------------------------------------
//
//  ERRTYPE.C
//
//  Written by Bruce R. Nevins
//
//  Edit History:
//  -------------
//
//------------------------------------------------------------------------------

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

#include <STDIO.H>
#include <XOS.H>

#define SIZE 8*1024

char prgname[] = "TYPE";

main(argc, argv)
int   argc;
char *argv[];

{
    register char *buffer;
    int      size;
    int      dev;

    if ((buffer=sbrk(SIZE)) == NULL)
        femsg(prgname, NULL);
    while (--argc > 0)
    {
        if ((dev=openio(*(++argv), (long)O_IN, NULL)) < 0)
            femsg(prgname, *argv);
        do
        {
            if ((size = read(dev, buffer, SIZE)) < 0)
                femsg(prgname, *argv);
            if (size)
                write(STDERR, buffer, size);
        } while (size == SIZE);
        if (close(dev) < 0)
            femsg(prgname, *argv);
    }
    exit(0);
}
