// Very simple program to test keypad input using KPDADRV

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <XOS.H>
#include <XOSTRM.H>
#include <XOSSVC.H>
#include <XOSERMSG.H>

char prgname[] = "KPTEST";

struct
{   byte4_parm clrim;
    byte4_parm setim;
    char       end;
} trmparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0xFFFFFFFE},
    {PAR_SET|REP_HEXV, 4, IOPAR_TRMSINPMODE, TIM_IMAGE}
};

void main(void)

{
    long handle;
    long rtn;
    char buffer[32];

    if ((handle = svcIoOpen(O_IN|O_PARTIAL, "TRM99S1:", NULL)) < 0)
	femsg2(prgname, "Error opening TRM99S1:", handle, NULL);

    if ((rtn = svcIoInBlockP(handle, NULL, 0, &trmparms)) < 0)
	femsg2(prgname, "Error setting input mode", handle, NULL);

    for (;;)
    {
	if ((rtn = svcIoInBlock(handle, buffer, 10)) < 0)
	    femsg2(prgname, "Error doing input", handle, NULL);
	if (rtn == 0)
	    printf("Null input\n");
	else
	    printf("Data: %.*s\n", rtn, buffer);
    }
}
