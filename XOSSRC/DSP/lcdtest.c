// Very simple program to test LCD display output using LCDADRV

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <XOS.H>
#include <XOSTRM.H>
#include <XOSSVC.H>
#include <XOSERMSG.H>

char prgname[] = "LCDTEST";

char str1[] = "This is a test\r\n";
char str2[] = "This is the second line";
char str3[] = "... This is more!\r\n";

void main(void)

{
    long handle;
    long rtn;

    if ((handle = svcIoOpen(O_OUT|O_IN|O_PARTIAL, "TRM99S1:", NULL)) < 0)
	femsg2(prgname, "Error opening TRM99S1:", handle, NULL);


    if ((rtn = svcIoOutBlock(handle, str1, sizeof(str1) - 1)) < 0)
	    femsg2(prgname, "Error doing output", handle, NULL);
    if ((rtn = svcIoOutBlock(handle, str2, sizeof(str2) - 1)) < 0)
	    femsg2(prgname, "Error doing output", handle, NULL);
    if ((rtn = svcIoOutBlock(handle, str3, sizeof(str3) - 1)) < 0)
	    femsg2(prgname, "Error doing output", handle, NULL);
}
