#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ALLEGRO.H>
#include <ALGSVC.H>
#include <ALGTIME.H>
#include <ALGERR.H>

char rcpname[] = "RCP0:101::";
char prgname[] = "RCPACT";
long rcpdev;

struct opnparms
{   byte4_parm lclport;
    byte4_parm rmtaddr;
    byte4_parm rmtport;
    char       end;
} opnparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_NETRMTPORTS, 900},
    {PAR_GET|REP_HEXV, 4, IOPAR_NETRMTNETAR, 0},
    {PAR_GET|REP_HEXV, 4, IOPAR_NETRMTPORTR, 0}
};

int main(void)

{
    long rtn;
    char buffer[100];

    if ((rcpdev = svcIoOpen(O_OUT|O_IN, rcpname, (void far *)&opnparms)) < 0)
        femsg2(prgname, "Error opening RCP device", rcpdev, rcpname);

    printf("Have connected to %d.%d.%d.%d.%d\n", opnparms.rmtaddr.value & 0xFF,
            (opnparms.rmtaddr.value>>8)&0xFF, (opnparms.rmtaddr.value>>16)&0xFF,
            (opnparms.rmtaddr.value>>24)&0xFF, opnparms.rmtport.value);

    memset(buffer, 0, 100);
    buffer[0] = 10;
    do
    {
        printf("Sending buffer #%d\n", buffer[0]);
        if ((rtn = svcIoOutBlock(rcpdev, buffer, 100)) < 0)
            femsg2(prgname, "Error doing RCP output", rtn, rcpname);
        if (rtn != 100)
            printf("Output size of %d is wrong, should be 100\n", rtn);
    } while (--buffer[0] >= 0);

    printf("Closing connection\n");
    if ((rcpdev = svcIoClose(rcpdev, 0)) < 0)
        femsg(prgname, rcpdev, rcpname);
    printf("Connection closed\n");

    return(0);    
}
