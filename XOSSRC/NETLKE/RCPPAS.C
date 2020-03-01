#include <stdio.h>
#include <stdlib.h>

#include <ALLEGRO.H>
#include <ALGSVC.H>
#include <ALGTIME.H>
#include <ALGERR.H>

char rcpname[] = "RCP0:*::";
char prgname[] = "RCPPAS";
long rcpdev;

struct opnparms
{   byte4_parm lclport;
    byte4_parm rmtaddr;
    byte4_parm rmtport;
    char       end;
} opnparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_NETLCLPORT , 900},
    {PAR_GET|REP_HEXV, 4, IOPAR_NETRMTNETAR, 0},
    {PAR_GET|REP_HEXV, 4, IOPAR_NETRMTPORTR, 0}
};

int main(void)

{
    if ((rcpdev = svcIoOpen(O_OUT|O_IN, rcpname, (void far *)&opnparms)) < 0)
        femsg(prgname, rcpdev, rcpname);

    printf("Have connection from %d.%d.%d.%d.%d\n", opnparms.rmtaddr.value & 0xFF,
            (opnparms.rmtaddr.value>>8)&0xFF, (opnparms.rmtaddr.value>>16)&0xFF,
            (opnparms.rmtaddr.value>>24)&0xFF, opnparms.rmtport.value);

    svcSchSuspend(NULL, XT_SECOND*10);

    printf("Closing connection\n");
    if ((rcpdev = svcIoClose(rcpdev, 0)) < 0)
        femsg(prgname, rcpdev, rcpname);
    printf("Connection closed\n");

    return(0);    
}
