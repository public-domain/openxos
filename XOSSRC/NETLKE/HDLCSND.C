#include <STDIO.H>
#include <STDLIB.H>
#include <XOSIO.H>
#include <XOSSVC.H>
#include <XOSTIME.H>

long hdlc;

char msg1[] = "\xAFTHIS IS A TEST123456789abcdefghijklmn";
char msg2[] = "SOME MORE TESTING";

char hdlcname[] = "HDLC:";
char prgname[] = "HDLCTEST";

struct opnparms
{   byte4_parm addr;
    char       end;
} opnparms =
{   {PAR_SET|REP_HEXV, 1, IOPAR_NETRMTNETAS, 0x77}
};

type_qab oobdqab =
{   QFNC_WAIT|QFNC_OOBD|QFNC_OUTBLOCK,
				/* qab_open */
    0,				/* qab_status */
    0,				/* qab_error */
    0,				/* qab_amount */
    0,				/* qab_handle */
    0,				/* qab_vector */
    0, 0, 0,
    1,				/* qab_option */
    0,				/* qab_count */
    NULL, 0,			/* qab_buffer1 */
    NULL, 0,			/* qab_buffer2 */
    (void far *)&opnparms, 0	/* qab_parm */
};

int main(void)

{
    int rtn;
    int count;

    if ((hdlc = svcIoOpen(O_OUT|O_IN, hdlcname, (void far *)&opnparms)) < 0)
        femsg(prgname, hdlc, hdlcname);
    oobdqab.qab_handle = hdlc;

    count = 100;
    do
    {
        oobdqab.qab_count = sizeof(msg1);
        oobdqab.qab_buffer1 = (char far *)msg1;
        if ((rtn = svcIoQueue((type_qab far *)&oobdqab)) < 0 ||
                (rtn = oobdqab.qab_error) < 0)
            femsg(prgname, rtn, hdlcname);
        svcSchSuspend(NULL, 100*XT_MILLISEC);
    } while (--count > 0);

    if ((rtn = svcIoClose(hdlc, 0)) < 0)
        femsg(prgname, rtn, hdlcname);

    return(0);    
}
