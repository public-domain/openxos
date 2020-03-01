// *--------------------------------------------*
// * SERVER.C					*
// * Program to send a command to a server	*
// *						*
// * Written by: John R. Goltz			*
// * Copyright (c) 1994, Saguaro Software, Ltd. *
// *						*
// *--------------------------------------------*

// Command format:
//	SERVER name {unit} arguments ....
// The command is sent to server "name".  If the second argument is numeric, it
//   is taken as a unit number.  Otherwise unit 1 is assumed.  Following
//   arguments are passed to the server as a command.

// This program must be linked with _MAINX which does not parse the command
//   line into separate items!

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <XCSTRING.H>
#include <TIME.H>
#include <CTYPE.H>
#include <ALLEGRO.H>
#include <ALGSVC.H>
#include <ALGERR.H>
#include <ALGMSG.H>
#include <ALGTIME.H>

extern char _mainx;

char  prgname[] = "SERVER";
long  ipmhandle = (long)&_mainx;	// Kludge to force loading of _mainx!
int   exitsts;
int   unitnum;
char  mbufr[256];
char  srvname[32];
char  dstname[32];

struct
{   byte4_parm  timeout;
    lngstr_parm dstname;
    char        end;
} outparm =
{   {PAR_SET|REP_DECV, 4, IOPAR_TIMEOUT   , 10*XT_SECOND},
    {PAR_SET|REP_STR , 0, IOPAR_DGRMTADDRS, dstname}
};

struct
{   byte4_parm  timeout;
    char        end;
} inpparm =
{   {PAR_SET|REP_DECV, 4, IOPAR_TIMEOUT, 10*XT_SECOND}
};

void main(
    img_data *argp)

{
    char *argpnt;
    char *symmsg;
    char *pnt;
    long  rtn;
    char  chr;

    argpnt = argp->cmdtail;
    while (((chr=*argpnt) != '\0') && !isspace(chr)) // Advance to first
        ++argpnt;				     //   whitespace character
    while (((chr=*argpnt) != '\0') && isspace(chr)) // Skip whitespace
        ++argpnt;

    pnt = srvname;
    while (((chr=*argpnt) != '\0') && !isspace(chr))
    {
        ++argpnt;
        *pnt++ = chr;
    }
    while (((chr=*argpnt) != '\0') && isspace(chr)) // Skip whitespace
        ++argpnt;
    if (isdigit(chr))
    {
        do
        {
            unitnum = unitnum * 10 + (chr & 0x0F);
            ++argpnt;
        } while (((chr=*argpnt) != 0) && isdigit(chr));
        while (((chr=*argpnt) != '\0') && isspace(chr)) // Skip whitespace
            ++argpnt;
    }
    else
        unitnum = 1;

    strupper(srvname);
    outparm.dstname.bfrlen = outparm.dstname.strlen =
            sprintf(dstname, "SYS^%s^%d", srvname, unitnum);
    if (chr == '\0')
    {
        fputs("? SERVER: Command error, correct usage is:\n"
              "            SERVER name {unit} arguments ...\n", stderr);
        exit(1);
    }
    if ((ipmhandle=svcIoOpen(O_IN|O_OUT, "IPM:", NULL)) < 0)
        femsg2(prgname, "Cannot open message device IPM:", ipmhandle, NULL);
    symmsg = argpnt - 1;		// Get address of our message
    *symmsg = MT_SRVCMD;		// Store message type
    if ((rtn=svcIoOutBlockP(ipmhandle, symmsg, strlen(symmsg), &outparm)) < 0)
    {
        if (rtn == ER_NTDEF)
        {
            fprintf(stderr, "? SERVER: Server %s is not running\n", srvname);
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
        if (mbufr[0] > 7)		// Valid message type?
        {
            fprintf(stderr, "? SERVER: Illegal message received, type = %d\n",
                    mbufr[0]);
            exit(1);
        }
        mbufr[(int)rtn] = '\0';		// Put null at end of message
        if ((mbufr[0] & 3) == 3)
        {
            exitsts |= 1;
            fprintf(stderr, "%s\n", &mbufr[1]);
        }
        else
            printf("%s\n", &mbufr[1]);
        if (mbufr[0] > 3)		// Are we finished?
            exit (exitsts);
    }
}
