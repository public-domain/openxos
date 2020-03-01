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
#include <XOS.H>
#include <XOSSINFO.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include "SHOW.H"

extern char prgname[];
extern int  infocnt;
char  *type[] =
{   "",
    "Class driver",
    "Device driver",
    "File system",
    "DOS extension",
    "XOS extension"
};
#define MAXTYPE (sizeof(type)/sizeof(char *)-1)

int lkeinfo(void)
{
    lkeinfo_data *blk;
    char          version[32];
    char          level[32];

    ++validcount;
    blk = (lkeinfo_data *)getinfo(GSI_LKE, sizeof(lkeinfo_data));
    fputs("LKE name          Version    Level       LKE type\n", stdout);
    while (--infocnt >= 0)
    {
        sprintf(version, "%d.%d.%d", blk->version.major,
                blk->version.minor, blk->version.edit);
        sprintf(level, "%d.%d", blk->level.major, blk->level.minor);
        if (blk->type > MAXTYPE)
            blk->type = 0;
        if (blk->symo != 0)
            printf("%-16.16s  %-11s% -11s %s\n      CO=%08.8lX CS=%-6ld  "
                    "DO=%08.8lX DS=%-6ld SO=%08.8lX SS=%ld\n", blk->name,
                    version, level, type[(int)(blk->type)], blk->codeo,
                    blk->codes, blk->datao, blk->datas, blk->symo, blk->syms);
        else
            printf("%-16.16s  %-11s% -11s %s\n      CO=%08.8lX CS=%-6ld  "
                    "DO=%08.8lX DS=%ld\n", blk->name, version, level,
                    type[(int)(blk->type)], blk->codeo, blk->codes, blk->datao,
                    blk->datas);
        ++blk;
    }
    return (TRUE);
}
