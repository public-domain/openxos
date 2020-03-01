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
#include <XOSSTR.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSUDF.H>
#include <XOSERRMSG.H>

extern ulong swapword (ulong value);
#pragma aux swapword =	\
   "xchg al, ah"	\
    parm [EAX] value [EAX];

long  udf;

long  offset;
int   lvl1size;
int   lvl1type;
int   lvl2size;
int   lvl2hdrsz;
int   lvl2left;
int   lvl2type;

uchar *buffer;
uchar *bfrpnt;

char prgname[] = "UDFDUMP";

extern long errno;

struct priv
{   char *name;
    int   byte;
    char  bit;
} priv[] =
{  {"DETACH"  , 0, 0x01},	// May detach process
   {"NEWSES"  , 0, 0x02},	// May create new session
   {"MEMLOCK" , 0, 0x04},	// Can lock memory pages in place
   {"NOSWAP"  , 0, 0x08},	// Can lock memory pages in memory
   {"PORTIO"  , 1, 0x01},	// May directly access IO ports
   {"SHAREDEV", 1, 0x02},	// May share any device
   {"ALLIO"   , 1, 0x04},	// May do restricted IO operations
   {"SYSENV"  , 2, 0x01},	// May change system level environment strings
   {"SESENV"  , 2, 0x02},	// May change session level environment strings
   {"SYSLOG"  , 2, 0x04},	// May change system level logical names
   {"SYSADMIN", 3, 0x01},	// System administrator privileges
   {"GRPADMIN", 3, 0x02},	// Group administrator privileges
   {"OPER"    , 3, 0x04},	// Operator privileges
   {"CHNGUSER", 4, 0x01},	// May change user name
   {"ALLPROC" , 4, 0x02},	// May kill or interrupt any process
   {"FBYPASS" , 4, 0x04},	// Bypass all file access checking
   {"FREADALL", 4, 0x08},	// Bypass file read access checking
   {"FUSESYS" , 4, 0x10},	// Access files using system protection
   {"FSPECIFY", 4, 0x20},	// Can specify user name for file access
   {"READPHY" , 5, 0x01},	// Can read physical memory
   {"WRITEPHY", 5, 0x02},	// Can write physical memory
   {"READKER" , 5, 0x04},	// Can read kernel memory
   {"WRITEKER", 5, 0x08},	// Can write kernel memory
   {"SCRNSYM" , 5, 0x10},	// Can execute screen symbiont functions
   {"LKELOAD" , 5, 0x20}	// Can execute LKE load functions
};

void  failleft(void);
ulong getvarval(void);
void  showstring(char *head);
void  showpriv(char *head);
void  showmem(char *head);
void  showdatetime(char *head);
void  showflags(void);
void  showhistory(void);

void main(
   int   argc,
   char *argv[])

{
    long rtn;
    int  lvl1hdr;


    if (argc != 2)
    {
       fputs("? UDFDUMP: Command error, usage is:\n"
             "           UDFDUMP filespec\n", stderr);
       exit(1);
    }

    if ((udf = svcIoOpen(O_IN, argv[1], NULL)) < 0)
        femsg2(prgname, "Error opening input file", udf, NULL);


    buffer = (char *)getspace(4000);

    if ((lvl1size = svcIoInBlock(udf, buffer, 2)) < 0)
        femsg2(prgname, "Error reading input file", lvl1size, NULL);
    if (buffer[0] != 0x81 || buffer[1] != 0xC5)
    {
        fprintf(stderr, "? UDFDUMP: File header is incorrect\n");
        exit(1);
    }
    offset = 2;
    for (;;)
    {
        if ((lvl1size = svcIoInBlock(udf, buffer, 3)) < 0)
        {
            if (lvl1size == ER_EOF)
                exit(0);
            femsg2(prgname, "Error reading input file", lvl1size, NULL);
        }
        if (lvl1size != 3)
        {
            fprintf(stderr, "? UDFDUMP: EOF while reading level one header at"
                    " %ld\n", offset);
            exit(1);

        }
        lvl1type = buffer[0];
        lvl1size = buffer[1];
        if (lvl1size & 0x80)
        {
            lvl1size = ((lvl1size << 8) + buffer[2]) & 0x7FFF;
            bfrpnt = buffer;
            rtn = lvl1size;
            lvl1hdr = 3;
        }
        else
        {
            bfrpnt = buffer + 1;
            buffer[0] = buffer[2];
            rtn = lvl1size - 1;
            lvl1hdr = 2;
        }

        if ((rtn = svcIoInBlock(udf, bfrpnt, rtn)) < 0)
            femsg2(prgname, "Error reading input file", lvl1size, NULL);
        printf("*** Level 1: (%2d) User description (%d+%d bytes at %ld)\n",
                lvl1type, lvl1hdr, lvl1size, offset);
        offset += 3;
        bfrpnt = buffer;
        while (lvl1size > 0)
        {
            if ((lvl2type = *bfrpnt++) == 0)
            {
                bfrpnt += lvl1size - 1;
                offset += lvl1size;
                break;
            }
            lvl2size = *bfrpnt++;
            lvl2hdrsz = 2;
            if (lvl2size & 0x80)
            {
                lvl2size = ((lvl2size & 0x7F) << 8) | *bfrpnt++;
                lvl2hdrsz++;
            }
            lvl1size -= lvl2hdrsz;

            if (lvl2size > lvl1size)
            {
                printf("    Level 2: Level 1 block is too small to contain"
                        " level 2 block at %ld\n", offset);
                lvl2size = lvl1size;
            }
            else
            {
                switch (lvl2type)
                {
                 case UDF_USERNAME:
                    showstring("( 1) User name");
                    break;

                 case UDF_PASSWORD:
                    showstring("( 2) Password");
                    break;

                 case UDF_USERDESC:
                    showstring("( 3) User description");
                    break;

                 case UDF_PROGRAM:
                    showstring("( 4) Initial program");
                    break;

                 case UDF_HOMEDIR:
                    showstring("( 5) Home directory");
                    break;

                 case UDF_AVLPRIV:
                    showpriv("( 6) Allowed privileges");
                    break;

                 case UDF_INLPRIV:
                    showpriv("( 7) Initial active privileges");
                    break;

                 case UDF_AVLMEM:
                    showmem("( 8) Allowed memory limits");
                    break;

                 case UDF_INLMEM:
                    showmem("( 9) Initial active memory limits");
                    break;

                 case UDF_AVLCPU:
                    showmem("(10) Allowed CPU limits");
                    break;

                 case UDF_INLCPU:
                    showmem("(11) Initial active CPU limits");
                    break;

                 case UDF_PSWDEXP:
                    showdatetime("(12) Password expiration");
                    break;

                 case UDF_USEREXP:
                    showdatetime("(13) User expiration");
                    break;

                 case UDF_FLAGS:
                    showflags();
                    break;

                 case UDF_HISTORY:
                    showhistory();
                    break;

                 case UDF_USERID:
                    showstring("(32) User ID");
                    break;

                 case UDF_MAILNAME:
                    showstring("(33) Mailing name");
                    break;

                 case UDF_COMPANY:
                    showstring("(34) Company");
                    break;

                 case UDF_ADDR1:
                    showstring("(35) Address - line 1");
                    break;

                 case UDF_ADDR2:
                    showstring("(36) Address - line 2");
                    break;

                 case UDF_ADDR3:
                    showstring("(37) Address - line 3");
                    break;

                 case UDF_ADDR4:
                    showstring("(38) Address - line 4");
                    break;

                 case UDF_CITY:
                    showstring("(39) City");
                    break;

                 case UDF_STATE:
                    showstring("(40) State");
                    break;

                 case UDF_ZIP:
                    showstring("(41) Postal (ZIP) code");
                    break;

                 case UDF_COUNTRY:
                    showstring("(42) Country");
                    break;

                 case UDF_PHONE:
                    showstring("(43) Phone number");
                    break;

                 default:
                    printf("    Level 2: Illegal block type %d\n", lvl2type);
                    break;
                }
            }
            bfrpnt += lvl2size;
            offset += (lvl2size + lvl2hdrsz);
            lvl1size -= lvl2size;
        }

    }
}


void showstring(
    char *head)

{
    printf("    Level 2: %s (%d+%d bytes at %ld)\n        %*.*s\n", head,
            lvl2hdrsz, lvl2size, offset, lvl2size, lvl2size, bfrpnt);
}

void showpriv(
    char *head)

{
    int    cnt;
    char  *bpnt;
    struct priv *tpnt;
    char   buffer[300];

    bpnt = buffer;
    tpnt = priv;
    cnt = sizeof(priv)/sizeof(struct priv);
    do
    {
        if (tpnt->byte >= lvl2size)
            break;
        if ((bfrpnt[tpnt->byte] & tpnt->bit) != 0)
        {
            bpnt = strmov(bpnt, tpnt->name);
            *bpnt++ = ' ';
        }
        tpnt++;
    } while (--cnt > 0);
    if (bpnt == buffer)
        strcpy(buffer, "** none **");
    else
        bpnt[-1] = 0;
    printf("    Level 2: %s (%d+%d bytes at %ld)\n        %s\n", head,
            lvl2hdrsz, lvl2size, offset, buffer);

}

void showmem(
    char *head)

{
    char  *bpnt;
    char  *npnt;
    char   buffer[120];
    static char names[] = "TM\0OM\0PM\0RM\0WS";

    bpnt = buffer;
    npnt = names;
    lvl2left = lvl2size;    
    do
    {
        bpnt += sprintf(bpnt, "%s=%d ", npnt, getvarval());
        npnt += 3;
    } while (lvl2left > 0);
    if (bpnt == buffer)
        strcpy(buffer, "** none **");
    else
        bpnt[-1] = 0;
    printf("    Level 2: %s (%d+%d bytes at %ld)\n        %s\n", head,
            lvl2hdrsz, lvl2size, offset, buffer);
}

void showdatetime(
    char *head)

{


    head = head;
}

void showflags(void)

{

}

void showhistory(void)

{

}

ulong getvarval(void)

{
    ulong value;

    if (--lvl2left < 0)
        failleft();
    value = *bfrpnt++;
    if ((value & 0x80) != 0)
    {
        if (--lvl2left < 0)
            failleft();
        if (((value = ((value & 0x7F) << 8) + *bfrpnt++) & 0x4000) != 0)
        {
            if (--lvl2left < 0)
                failleft();
            if (((value = ((value & 0x3FFF) << 8) + *bfrpnt++) & 0x200000) != 0)
            {
                if (--lvl2left < 0)
                    failleft();
                value = ((value & 0x1FFFFF) << 8) + *bfrpnt++;
            }
        }
    }
    return (value);
}

void failleft(void)

{
    fprintf(stderr, "? UDFDUMP: Level two block is too small to contain "
            "variable length value at %ld\n", offset);
    exit(1);

}
