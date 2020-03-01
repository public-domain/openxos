// Very simple program to test font loading - A standard font is loaded
//   every 5 seconds to allow checking loading fonts to other than the
//   current screen.

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <XOS.H>
#include <XOSTRM.H>
#include <XOSERMSG.H>
#include <XOSSVC.H>
#include <XOSTIME.H>

char prgname[] = "FONTLOAD";

long font = 3;


void main(void)

{
    long rtn;

    for (;;)
    {
	font ^= 0x02;
	if ((rtn = svcTrmLdStdFont(DH_STDTRM, 0, font, 0, 256)) < 0)
	    femsg2(prgname, "Error loading font", rtn, NULL);
	svcSchSuspend(NULL, XT_SECOND*5);
    }
}
