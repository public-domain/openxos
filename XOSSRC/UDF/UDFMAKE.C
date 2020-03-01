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
#include <XOSSTR.H>
#include <XOS.H>
#include <XOSERRMSG.H>
#include <XOSSVC.H>

#define VERSION 3
#define EDITNUM 0

#define LINEMAX 256

extern ulong swapword (ulong value);
#pragma aux swapword =	\
   "xchg al, ah"	\
    parm [EAX] value [EAX];

FILE *src;
long  dst;
int   linenum;

int  usernamesz;
int  passwordsz;
int  userdescsz;
int  programsz;
int  homedirsz;

char  username[68];
char  password[68];
char  userdesc[132];
char  program[260];
char  homedir[260];

char  line[LINEMAX+2];

uchar *outbufr;
uchar *outpnt;

char prgname[] = "UDFMAKE";
char header[2] = {0x81, 0xC6};

extern long errno;

void blockout(int type, int size, char *str);
int  getstring(char *string, int size);

void main(
   int   argc,
   char *argv[])

{
    long rtn;

    printf("UDFMAKE: Version %d.%d\n", VERSION, EDITNUM);
    if (argc != 3)
    {
        fputs("? UDFMAKE: Command error, usage is:\n"
		"             UDFMAKE inputfile outputfile\n", stderr);
	exit(1);
    }
    if ((src = fopen(argv[1], "r")) == NULL)
        femsg2(prgname, "Error opening input file", -errno, argv[1]);
    if ((dst = svcIoOpen(O_IN|O_OUT|O_TRUNCA|O_CREATE, argv[2], NULL)) < 0)
        femsg2(prgname, "Error creating output file", dst, argv[2]);
    if ((rtn = svcIoOutBlock(dst, header, 2)) < 0)
        femsg2(prgname, "Error writting to output file", rtn, argv[2]);

    outbufr = (char *)getspace(4000);

    while (fgets(line, LINEMAX, src) != 0)
    {
        ++linenum;
        switch(toupper(line[0]))
        {
         case 'U':
            dumpuser();
            usernamesz = getstring(username, 64);
            break;

         case 'P':
            passwordsz = getstring(password, 64);
            break;

         case 'D':
            userdescsz = getstring(userdesc, 128);
            break;

         case 'R':
            programsz = getstring(program, 256);
            break;

         case 'H':
            homedirsz = getstring(homedir, 256);
            break;

         case '\0':
         case '\n':
            break;

         default:
            fprintf(stderr, "? UDFMAKE: Illegal line type \"%c\" at line %d\n",
                    line[0], linenum);
            exit(1);
        }
    }
    dumpuser();
}

void dumpuser(void)

{
    long rtn;
    int  len;

    outbufr[0] = 0xAA;
    outbufr[1] = 1;
    outpnt = outbufr + 2;
    if (usernamesz != 0)
    {
        blockout(1, usernamesz, username);
        blockout(2, passwordsz, password);
        blockout(3, userdescsz, userdesc);
        blockout(4, programsz, program);
        blockout(5, homedirsz, homedir);
        len = outpnt - outbufr;
        if ((rtn = svcIoOutBlock(dst, outbufr, len)) < 0)
            femsg2(prgname, "Error writting to output file", rtn, NULL);
    }
    usernamesz = 0;
    passwordsz = 0;
    userdescsz = 0;
    programsz = 0;
    homedirsz = 0;
}

int getstring(
    char *string,
    int   size)

{
    char *pnt;
    int   len;

    pnt = line + 1;
    while (isspace(*pnt))
        pnt++;
    len = strlen(pnt);
    if (len > 0)
    {
        if (pnt[len-1] == '\n')
            --len;
        if (len > size)
        {
            fprintf(stderr, "? UDFMAKE: String too long for line type \"%c\" "
                    "at line %d\n", line[0], linenum);
            exit(1);
        }
        strncpy(string, pnt, len);
    }
    string[len] = 0;
    return (len);
}

void blockout(
    int  type,
    int  size,
    char *str)

{
    int   len;

    if (size != 0)
    {
        len = size + 2;
        *outpnt++ = type;
        if (size > 127)
        {
            *outpnt++ = (size >> 8) | 0x80;
            ++len;
        }
        *outpnt++ = size;
        outpnt = strnmov(outpnt, str, size);
    }
}
