// cksum - Program to test the TAMSERVER file checksum function

#include <CTYPE.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>

#include <XOSIO.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSSTR.H>

long dev;
long rtn;
long data[3];
char rmtname[140] = "XFP1:";
char prgname[] = "CKSUM";

main(argc, argv)
int   argc;
char *argv[];

{
    if (argc != 2)
    {
        fputs("? GET: Command error, correct usage is:\n"
              "         CKSUM filespec\n", stderr);
        exit(1);
    }
    strupper(argv[1]);
    strmov(&rmtname[5], argv[1]);
    if ((dev = svcIoOpen(O_IN, rmtname, NULL)) < 0)
        femsg2(prgname, "Error opening remote file", dev, rmtname);
    data[0] = 0;
    if ((rtn = svc_iospecfunc(dev, CLS_XFP/0x100, 3, data)) < 0)
        femsg2(prgname, "Error getting file checksum", rtn, rmtname);
    printf("File checksum = %08.8lX, length = %ld\n", data[1], data[2]);
    svcIoClose(dev, 0);
}
