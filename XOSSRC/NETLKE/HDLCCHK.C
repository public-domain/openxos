#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <XOSIO.H>
#include <XOSSVC.H>
#include <XOSTIME.H>
#include <XOSERR.H>

long  hdlc;
FILE *loadfile;

char xidmsg[8] = {0xBF, 'F', 'T', '3', '2', 'A', '1', 0x77};
char testmsg[] = "\xF3This is a test message";
char msg1[] = {0, 0, 0x82};
char msg2[] = {0, 0, 0x96, 0x01, 0x01, 0xFF};
char msg3[] = {0, 0, 0x90};
char msg4[] = {0, 0, 0x00};

#define p(x) (((((x)<<1)^((x)<<2)^((x)<<3)^((x)<<4)^((x)<<5)^((x)<<6)^((x)<<7)\
    ^0x80)&0x80)|x)

uchar partbl[] =
{   p(0x00), p(0x01), p(0x02), p(0x03), p(0x04), p(0x05), p(0x06), p(0x07),
    p(0x08), p(0x09), p(0x0A), p(0x0B), p(0x0C), p(0x0D), p(0x0E), p(0x0F),
    p(0x10), p(0x11), p(0x12), p(0x13), p(0x14), p(0x15), p(0x16), p(0x17),
    p(0x18), p(0x19), p(0x1A), p(0x1B), p(0x1C), p(0x1D), p(0x1E), p(0x1F),
    p(0x20), p(0x21), p(0x22), p(0x23), p(0x24), p(0x25), p(0x26), p(0x27),
    p(0x28), p(0x29), p(0x2A), p(0x2B), p(0x2C), p(0x2D), p(0x2E), p(0x2F),
};
char hdlcname[] = "HDLC:";
char prgname[] = "HDLCTEST";

struct opnparms
{   byte4_parm addr;
    char       end;
} opnparms =
{   {PAR_SET|REP_HEXV, 1, IOPAR_NETRMTNETAS, 0x77}
};

struct
{   byte4_parm mode;
    char       end;
} bcparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_NETSMODE, 0x0800}
};

struct
{   byte4_parm mode;
    char       end;
} ftoparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_TIMEOUT, XT_SECOND/10}
};

struct
{   byte4_parm mode;
    char       end;
} stoparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_TIMEOUT, XT_SECOND*4}
};

struct
{   text8_parm class;
    char       end;
} sdfparms =
{   {PAR_SET|REP_TEXT, 8, IOPAR_CLASS, "HDLCA"}
};

type_qab qab =
{   0,				/* qab_func */
    0,				/* qab_status */
    0,				/* qab_error */
    0,				/* qab_amount */
    0,				/* qab_handle */
    0,				/* qab_vector */
    0, 0, 0,
    0,				/* qab_option */
    sizeof(xidmsg),		/* qab_count */
    NULL, 0,			/* qab_buffer1 */
    NULL, 0,			/* qab_buffer2 */
    NULL, 0			/* qab_parm */
};

void display(char *prefix, uchar *buffer, int size);
int  gethex(char *str);
int  getresp(char *buffer, int size, void *parms, int disp);
int  hexdig(int chr);
void loadsend(char *filename);
void sendmsg(char *msg, int size, short mode, void *parms, int disp);


int main(void)

{
    int   rtn;
    uchar buffer[100];

    // Open the HDLC device

    if ((hdlc = svcIoOpen(O_OUT|O_IN, hdlcname, (void far *)&opnparms)) < 0)
        femsg(prgname, hdlc, hdlcname);
    qab.qab_handle = hdlc;

    do
    {
        sendmsg(xidmsg, sizeof(xidmsg), QFNC_OOBD|QFNC_POLL, &bcparms, TRUE);
    } while ((rtn = getresp(buffer, 100, &ftoparms, TRUE)) == 0);

    if ((rtn = svcIoSpecial(hdlc, 2, NULL, 100, (void far *)&sdfparms)) < 0)
        femsg2(prgname, "Error connecting to instrument", rtn, hdlcname);
    fputs("Instrument connected\n", stdout);

//  sendmsg(testmsg, sizeof(testmsg)-1, QFNC_OOBD|QFNC_POLL, NULL, TRUE);
//  getresp(buffer, 100, &stoparms, TRUE);

    sendmsg(msg1, sizeof(msg1), QFNC_POLL, NULL, TRUE);
    getresp(buffer, 100, &stoparms, TRUE);

    sendmsg(msg2, sizeof(msg2), QFNC_POLL, NULL, TRUE);
    getresp(buffer, 100, &stoparms, TRUE);

    loadsend("IOMS2210.VLT");

    loadsend("S22CFG10.VLT");

    sendmsg(msg3, sizeof(msg3), QFNC_POLL, NULL, TRUE);
    fputs("Loaded code started\n", stdout);

//  svcSchSuspend(NULL, XT_SECOND*1);

    if ((rtn = svcIoSpecial(hdlc, 2, NULL, 100, (void far *)&sdfparms)) < 0)
        femsg2(prgname, "Error resetting connection to device", rtn, hdlcname);
    fputs("Connection reset\n", stdout);

    sendmsg(msg4, sizeof(msg4), QFNC_POLL, NULL, TRUE);
    getresp(buffer, 100, &stoparms, TRUE);

    sendmsg(msg4, sizeof(msg4), QFNC_POLL, NULL, TRUE);
    getresp(buffer, 100, &stoparms, TRUE);

    sendmsg(msg4, sizeof(msg4), QFNC_POLL, NULL, TRUE);
    getresp(buffer, 100, &stoparms, TRUE);

    sendmsg(testmsg, sizeof(testmsg)-1, QFNC_OOBD|QFNC_POLL, NULL, TRUE);
    getresp(buffer, 100, &stoparms, TRUE);

    if ((rtn = svcIoSpecial(hdlc, 3, NULL, 0, (void far *)&sdfparms)) < 0)
        femsg2(prgname, "Error disconnecting instrument", rtn, hdlcname);
    fputs("Instrument disconnected\n", stdout);

    fputs("Closing serial device\n", stdout);

    if ((rtn = svcIoClose(hdlc, 0)) < 0)
        femsg(prgname, rtn, hdlcname);

    return(0);    
}

//***************************************************
// Function: sendmsg - Send message to the instrument
// Returned: Nothing
//***************************************************

void sendmsg(
    char *msg,
    int   size,
    short mode,
    void *parms,
    int   disp)

{
    long rtn;

    if (!(mode & QFNC_OOBD))
        msg[0] = msg[1] = partbl[size];
    if (disp)
        display("Xmtd", msg, size);
    qab.qab_func = (QFNC_WAIT|QFNC_OUTBLOCK) | mode;
    qab.qab_buffer1 = (char far *)msg;
    qab.qab_count = size;
    qab.qab_parm = (void far *)parms;
    if ((rtn = svcIoQueue((type_qab far *)&qab)) < 0 ||
            (rtn = qab.qab_error) < 0)
        femsg(prgname, rtn, hdlcname);
}

//*****************************************************
// Function: getresp - Get response from the instrument
// Returned: Length of response
//*****************************************************


int getresp(
    char *buffer,
    int   size,
    void *parms,
    int   disp)

{
    long rtn;

    qab.qab_func = QFNC_WAIT|QFNC_INBLOCK;
    qab.qab_buffer1 = (char far *)buffer;
    qab.qab_count = size;
    qab.qab_parm = (void far *)parms;
    if ((rtn = svcIoQueue((type_qab far *)&qab)) < 0 ||
            (rtn = qab.qab_error) < 0)
    {
        if (rtn == ER_NORSP)
        {
            if ((rtn = svcIoSpecial(hdlc, 1, NULL, 0, (void far *)&sdfparms))
                    < 0)
                femsg2(prgname, "Error reenabling output", rtn, hdlcname);
            return(0);
        }
        else
            femsg(prgname, rtn, hdlcname);
    }
    if (disp)
        display((qab.qab_status&QSTS_OOBD)? "Rcvd OOBD": "Rcvd data", buffer,
                qab.qab_amount);
    return (qab.qab_amount);
}

//************************************
// Function: display - Display message
// Returned: Nothing
//************************************

void display(
    char  *prefix,
    uchar *buffer,
    int    size)

{
    int    cnt;
    uchar *pnt;
    char  *dst;
    char   txtbfr[80];

    printf("%s length = %d\n", prefix, size);
    if (size != 0)
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
        } while (--size > 0);
        printf("  %s\n", txtbfr);
    }

}

//**************************************************
// Function: loadsend - Send load data to instrument
// Returned: Nothing
//**************************************************

void loadsend(
    char *filename)

{
    char *pnt;
    char *bp;
    int   cnt;
    char  buffer[200];
    uchar msg[100];

    if ((loadfile = fopen(filename, "r")) == NULL)
        femsg2(prgname, "Error opening load data file", -errno, filename);

    printf("Loading %s\n", filename);
    msg[2] = 0x94;
    while (fgets(buffer, 200, loadfile) != NULL)
    {
        if (gethex(buffer+7) != 0)
        {
            printf("\n");
            fclose(loadfile);
            return;
        }
        cnt = gethex(buffer+1);
        if (cnt > 16)
        {
            fprintf(stderr, "? HDLCCHK: Load data count %d is too big\n",
                    cnt);
            exit(1);
        }
        msg[3] = gethex(buffer+3);
        msg[4] = gethex(buffer+5);
        pnt = msg + 5;
        bp = buffer + 9;
        while (--cnt >= 0)
        {
            *pnt++ = gethex(bp);
            bp += 2;
        }
        printf("\r%04.4X ", (msg[3]<<8) + msg[4]);
        sendmsg(msg, pnt - msg, QFNC_POLL, NULL, FALSE);
        getresp(buffer, 100, &stoparms, FALSE);
//      svcSchSuspend(NULL, XT_SECOND/20);
    }
    femsg2(prgname, "Error reading load data file", -errno, filename);
}

//*********************************************************
// Function: gethex - Get 2 character hex value from string
// Returned: Value
//*********************************************************

int gethex(
    char *str)

{
    return ((hexdig(str[0]) << 4) + hexdig(str[1]));
}

int hexdig(
    int chr)

{
    if (chr >= 'A')
        chr += 9;
    return (chr & 0xF);
}
