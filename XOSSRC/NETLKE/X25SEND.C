#include <STDIO.H>
#include <STDLIB.H>
#include <ALLEGRO.H>
#include <ALGSVC.H>
#include <ALGTIME.H>
#include <ALGERMSG.H>

#define MSGSIZE  500
#define BUFRSIZE 520

long x25dev;
long total;
long time;
long average;
char prgname[] = "X25SEND";
char outbuffer[BUFRSIZE];

int setvector(int vec, int level, void (*func)());

void second(void);

void main(void)

{
    long rtn;

    setvector(70, 4, second);
    fputs("Calling 123456789\n", stdout);
    if ((x25dev = svcIoOpen(O_OUT|O_IN, "XA10:123456789::", NULL)) < 0)
        femsg2(prgname, "Error opening X.25 device", x25dev, NULL);
    fputs("Call established\n", stdout);
    svcSchAlarm(4, 0, 70, 0, 0, XT_SECOND);
    svcSchSetLevel(0);
    for (;;)
    {
        if ((rtn = svcIoOutBlock(x25dev, outbuffer, MSGSIZE)) < 0)
            femsg2(prgname, "Error outputting to X.25 device", rtn, NULL);
        outbuffer[0]++;
        total += 500;
    }
}

void second(void)

{
    average = (average * 3)/4 + total/4;
    total = 0;
    printf(" %7d\r", average);
}
