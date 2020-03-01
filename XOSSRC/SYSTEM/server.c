// *--------------------------------------------*
// * SERVER.C					*
// * Program to send a command to a server	*
// *						*
// * Written by: John R. Goltz			*
// *--------------------------------------------*

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

// Command format:
//	SERVER name {unit} arguments ....
// The command is sent to server "name".  If the second argument is numeric, it
//   is taken as a unit number.  Otherwise unit 1 is assumed.  Following
//   arguments are passed to the server as a command.

// This program must be linked with _MAINALT which does not parse the command
//   line into seperate items!

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <TIME.H>
#include <CTYPE.H>
#include <XOS.H>
#include <XOSSTR.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSERRMSG.H>
#include <XOSTIME.H>

#define VERSION 2
#define EDITNUM 0

char *argpnt;
char  prgname[] = "SERVER";
long  ipmhandle = 0;
FILE *logstream = NULL;
int   exitsts;
int   unitnum;
char  srvname[32];
char  dstname[32];
char  logfile[260] = {0};
char  syslog = 0;

struct
{   byte4_parm  timeout;
    lngstr_parm dstname;
    char        end;
} outparm =
{   {PAR_SET|REP_DECV, 4, IOPAR_TIMEOUT    , 30*XT_SECOND},
    {PAR_SET|REP_STR , 0, IOPAR_MSGRMTADDRS, dstname}
};

struct
{   byte4_parm  timeout;
    char        end;
} inpparm =
{   {PAR_SET|REP_DECV, 4, IOPAR_TIMEOUT, 30*XT_SECOND}
};

long value;

void  getatom(char *bufr, int size, char *erritm);
char *getval(char *pnt);
void  putinlog(char *label, int type, char *msg);
int   skipws(void);


void mainalt(
    img_data *argp)

{
    char *symmsg;
    char *pnt;
    long  rtn;
    char  chr;
    char  mbufr[512];

    printf("SERVER: Version %d.%d\n", VERSION, EDITNUM);
    argpnt = argp->cmdtail;
    while (((chr=*argpnt) != '\0') && !isspace(chr)) // Advance to first
		++argpnt;				     				 //   whitespace character
    skipws();							// Skip whitespace and check for options
    pnt = srvname;
    while (((chr=*argpnt) != '\0') && !isspace(chr))
    {
		++argpnt;
		*pnt++ = chr;
    }
    chr = skipws();						// Skip whitespace and check for options
    if (isdigit(chr))
    {
		do
		{
			unitnum = unitnum * 10 + (chr & 0x0F);
			++argpnt;
		} while (((chr=*argpnt) != 0) && isdigit(chr));
		skipws();						// Skip whitespace and check for options
    }
    else
        unitnum = 1;
    strupper(srvname);
    if (logfile[0] != 0)
    {
		if ((logstream = fopen(logfile, "w")) == NULL)
		{
			printf("% SERVER: Warning: Could not open log file, logging disabled");
		}

    }
    outparm.dstname.bfrlen = outparm.dstname.strlen = sprintf(dstname,
			"SYS^%s^%d", srvname, unitnum);
    if (chr == '\0')
    {
		fputs("? SERVER: Command error, correct usage is:\n"
				"            SERVER name {unit} command arguments ...\n",
				stderr);
		exit(1);
    }
    putinlog("Cmd", 0, argpnt);
    if ((ipmhandle=svcIoOpen(O_IN|O_OUT, "IPM:", NULL)) < 0)
		femsg2(prgname, "Cannot open message device IPM:", ipmhandle, NULL);
    symmsg = argpnt - 1;				// Get address of our message
    *symmsg = MT_SRVCMD;				// Store message type
    if ((rtn=svcIoOutBlockP(ipmhandle, symmsg, strlen(symmsg), &outparm)) < 0)
    {
		if (rtn == ER_NTDEF)
		{
			fprintf(stderr, "? SERVER: Server %s (unit %d) is not running\n",
			srvname, unitnum);
			exit(1);
		}
		else
			femsg2(prgname, "Error sending message to server", rtn, NULL);
    }
    for (;;)
    {
        if ((rtn=svcIoInBlockP(ipmhandle, mbufr, sizeof(mbufr), &inpparm))
                < 0)
            femsg2(prgname, "Error receiving response from server", rtn, NULL);
        if (mbufr[0] > 7)				// Valid message type?
        {
            fprintf(stderr, "? SERVER: Illegal message received, type = %d\n",
                    mbufr[0]);
            exit(1);
        }
        mbufr[rtn++] = (mbufr[0] == MT_INTRMDSTS) ? '\r': '\n';
										// Add LF if normal message or CR if
										//   status message
        mbufr[rtn] = 0;					// Put null at end

		if (mbufr[1] == '{')			// Have special stuff?
		{
			pnt = mbufr + 2;
			while ((chr = *pnt++) != '}')
			{
				if (chr == 'T')			// Time-out value?
				{
					pnt = getval(pnt);
					outparm.timeout.value = value * XT_SECOND;
				}
				else
				{
					fputs("? SERVER: Illegal format special data received\n",
					stderr);
					exit(1);
				}
			}
			rtn = strmov(mbufr + 1, pnt) - mbufr;
		}
		if (mbufr[0] == MT_INTRMDERR || mbufr[0] == MT_FINALERR)
		{
			exitsts |= 1;

			if (mbufr[1] == 1)
				fprintf(stderr, "\x1B[K%s", mbufr+2);
			else
				fputs(&mbufr[1], stderr);
		}
		else
		{
			if (mbufr[1] == 1)
				printf("\x1B[K%s", mbufr+2);
			else
				fputs(&mbufr[1], stdout);
		}
		mbufr[rtn - 1] = 0;
		putinlog("Rsp", mbufr[0], mbufr+1);
		if (mbufr[0] >= MT_FINALMSG)	// Are we finished?
			exit (exitsts);				// Yes
	}
}


int skipws(void)

{
    char keyword[16];
    char chr;

	for (;;)
	{
		if ((chr = *argpnt) == '-' || chr == '/')
		{
			argpnt++;
			getatom(keyword, 12, "Option name");
			strupper(keyword);
			if (strcmp(keyword, "LOG") == 0 || strcmp(keyword, "LOGFILE") == 0)
			{
				while (((chr=*argpnt++) != '\0') && isspace(chr))
					;
				if (chr != '=')
				{
					fputs("? SERVER: No file specified for /LOGFILE\n", stderr);
					exit(1);
				}
				getatom(logfile, 256, "Log file specification");
			}
			else if (strcmp(keyword, "SYS") == 0 ||
					strcmp(keyword, "SYSLOG") == 0)
				syslog = TRUE;
			else
			{
				fprintf(stderr, "? SERVER: Illegal option %s specified\r",
						keyword);
				exit(1);
			}
		}
		else
		{
			if (isspace(chr))
				argpnt++;
			else
				return (chr);
		}
	}
}

void getatom(
    char *bufr,
    int   size,
    char *erritm)

{
    char chr;

	while (((chr=*argpnt) != '\0') && isspace(chr)) // Skip whitespace
		++argpnt;
	while ((chr = *argpnt++) != 0 && chr != '=' && !isspace(chr))
	{
		if (--size >= 0)
			*bufr++ = chr;
		else
		{
			fprintf(stderr, "? SERVER: %s is too long", erritm);
			exit(1);
		}
	}
	*bufr = 0;
	argpnt--;
}

char *getval(
    char *pnt)

{
    char chr;

    value = 0;

	while ((chr = *pnt) != 0 && isdigit(chr))
	{
		pnt++;
		value = value * 10 + (chr & 0x0F);
	}
	return (pnt);
}

void putinlog(
    char *label,
    int   type,
    char *msg)

{
    char *pntm;
    char *pntb;
    int   pfxsize;
    char  chr;
    char  bufr[300];

	if ((syslog && type != MT_INTRMDSTS) || logstream != NULL)
	{
		if (msg[0] == 1)
			msg++;
		pfxsize = sprintf(bufr + 12, "%.16s(%d) %s: ", srvname, unitnum, label);
		pntb = bufr + 12 + pfxsize;
		pntm = msg;
		while ((chr = *pntm++) != 0)
		{
			*pntb++ = chr;
			if (chr == '\n')
			{
				memcpy(pntb, bufr + 12, pfxsize);
				pntb += pfxsize;
			}
		}
		if (logstream != NULL)
		{
			strmov(pntb, "\n");
			fputs(bufr + 12, logstream);
		}
		if (syslog && type != MT_INTRMDSTS)
		{
			*pntb = 0;
			pntm = pntb = (bufr + 12);
			do
			{
				if ((chr = *pntb) == 0 || chr == '\n')
				{
					memcpy(pntm - 12, "    SERVER  ", 12);
					*pntb = 0;
					svcSysLog(pntm - 12, pntb - pntm + 12);
					pntm = pntb + 1;
				}
				pntb++;
			} while (chr != 0);
		}
    }
}
