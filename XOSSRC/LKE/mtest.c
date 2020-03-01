#include "stdio.h"
#include "xos.h"
#include "xossvc.h"
#include "xosvect.h"
#include "xoserrmsg.h"


typedef struct
{   long  svdEDI;
    long  svdESI;
    long  svdEBP;
    long  svdESP;
    long  svdEBX;
    long  svdEDX;
    long  svdECX;
    long  svdEAX;
    long  intEIP;
    long  intCS;
    long  intEFR;
    long  intGS;
    long  intFS;
    long  intES;
    long  intDS;
    short intcnt;
    short intnum;
	long  userdata;
	long  buttons;
	long  xval;
	long  yval;
	long  zval;
	long  time;
} INTDATA;

long xval;
long yval;
long zval;
char mname[20];
char prgname[] = "MTEST";

struct
{	byte4_parm vect;
	uchar      end;
} mouseparms =
{	{PAR_SET|REP_HEXV, 4, IOPAR_TRMHUVECT, 50}
};;

struct
{	text16_char mname;
	uchar       end;
} mousechars =
{	{PAR_GET|REP_TEXT, 16, "MOUSE"}
};


void mouseevent(INTDATA intdata);


void main()


{
	long rtn;

	// Get name of the mouse device for our terminal

	if ((rtn = svcIoDevChar(DH_STDTRM, &mousechars)) < 0)
		femsg2(prgname, "Error get name of mouse device", rtn, NULL);

	sprintf(mname, "%s:", mousechars.mname.value);

	// Access the mouse device to reset its state

	svcIoDevParm(0, mname, NULL);

	// Set the mouse signal vector

	if ((rtn = setvector(50, 0x04, mouseevent)) < 0)
		femsg2(prgname, "Error setting mouse signal vector", rtn, NULL);

	// Set the mouse vector number

	if ((rtn = svcIoOutBlockP(DH_STDTRM, NULL, 0, &mouseparms)) < 0)
		femsg2(prgname, "Error setting mouse signal number", rtn, NULL);

	fputs("\x1B[2J", stdtrm);

	svcSchSetLevel(0);
	while (TRUE)
		svcSchSuspend(NULL, 0xFFFFFFFF);
}



void mouseevent(
	INTDATA intdata)

{
	if ((xval += intdata.xval) < 0)
		xval = 0;
	else if (xval > 799)
		xval = 799;
	if ((yval += intdata.yval) < 0)
		yval = 0;
	else if (yval > 599)
		yval = 599;
	zval += intdata.zval;

	svcTrmCurPos(DH_STDTRM, -1, 0, 0);

	printf("%s %s %s %6d %6d %6d %6d %6d %6d\n", (intdata.buttons & 0x40) ?
			"L" : " ", (intdata.buttons & 0x20) ? "M" : " ",
			(intdata.buttons & 0x10) ? "R" : " ", xval, yval, zval,
			intdata.xval, intdata.yval, intdata.zval);

	svcTrmCurPos(DH_STDTRM, -1, xval/10, yval/10);

}
