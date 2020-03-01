#include <stdio.h>
#include <stdlib.h>

#include <XOSIO.H>
#include <XOSSVC.H>

long hdlc;
char hdlcname[] = "HDLC:";
char prgname[] = "HDLCRCV";

struct opnparms
{   byte4_parm addr;
    char       end;
} opnparms =
{   {PAR_SET|REP_HEXV, 1, IOPAR_NETRMTNETAS, 0x77}
};

void main(void)

{
    int    rtn;
    int    cnt;
    uchar *pnt;
    uchar *dst;
    uchar  buffer[300];
    uchar  txtbfr[80];

    if ((hdlc = svcIoOpen(O_IN, hdlcname, (void far *)&opnparms)) < 0)
        femsg(prgname, hdlc, hdlcname);

    for (;;)
    {
        if ((rtn = svcIoInBlock(hdlc, buffer, 300)) < 0)
            femsg(prgname, rtn, hdlcname);

        printf("Count = %d\n", rtn);
        if (rtn != 0)
        {
            pnt = buffer;
            dst = txtbfr;
            cnt = 16;
            do
            {
                if (--cnt < 0)
                {
                    printf("  %s\n", txtbfr);
                    dst = txtbfr;
                    cnt = 15;
                }
                dst += sprintf(dst, " %02.2X", *pnt++);
            } while (--rtn > 0);
            printf("  %s\n", txtbfr);
        }
    }
}
