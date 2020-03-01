//++++
// This software is in the public domain.  It may be freely copied and used
// for whatever purpose you see fit, including commerical uses.  Anyone
// modifying this software may claim ownership of the modifications, but not
// the complete derived code.  It would be appreciated if the authors were
// told what this software is being used for, but this is not a requirement.

//   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>

#include <XOSSTR.H>
#include <XOS.H>
#include <XOSERMSG.H>
#include <XOSSVC.H>

long  scsi;
long  rtn;
long  temp;
char  isover[12];
char  ecmaver[12];
char  ansiver[12];
char  format[16];
char *aenc;
char *trmiop;
long  option;
char *removev;
char *device;
long  devindex;
long  devmod;
char *connect;
char *reladr;
long  buswidth;
char *sync;
char *linked;
char *queued;
char *reset;
uchar buffer[255];
char  prgname[] = "SCSIINQ";

struct sdfp
{   text8_parm class;
    char       end;
} sdfp =
{   {PAR_SET|REP_TEXT, 8, IOPAR_CLASS, "SCSI"}
};

char *contbl[] =
{   "Yes",
    "No",
    "X10",
    "None",
    "V00",
    "V01",
    "V10",
    "V11"
};

char *scsi2dev[] =
{   "Direct access (disk)",
    "Sequential access (tape)",
    "Printer",
    "Processor",
    "WORM",
    "CD-ROM",
    "Scanner",
    "Optical memory",
    "Medium changer",
    "Communications",
    "Pre-press (0A)",
    "Pre-press (0B)",
    "Reserved (0C)",
    "Reserved (0D)",
    "Reserved (0E)",
    "Reserved (0F)",
    "Reserved (10)",
    "Reserved (11)",
    "Reserved (12)",
    "Reserved (13)",
    "Reserved (14)",
    "Reserved (15)",
    "Reserved (16)",
    "Reserved (17)",
    "Reserved (18)",
    "Reserved (19)",
    "Reserved (1A)",
    "Reserved (1B)",
    "Reserved (1C)",
    "Reserved (1D)",
    "Reserved (1E)",
    "Unknown or no"
};

char *scsi1dev[] =
{   "SCSI 1 (00)",
    "SCSI 1 (01)",
    "SCSI 1 (02)",
    "SCSI 1 (03)",
    "SCSI 1 (04)",
    "SCSI 1 (05)",
    "SCSI 1 (06)",
    "SCSI 1 (07)",
    "SCSI 1 (08)",
    "SCSI 1 (09)",
    "SCSI 1 (0A)"
};

char *fmttbl[] =
{   "SCSI 1",
    "SCSI 1 (CCS)",
    "SCSI 2"
};

void main(
    int   argc,
    char *argv[])

{
    char devname[20];

    if (argc != 4)
    {
        fputs("? Command error, usage is:\n"
              "    SCSIINQ dev: target lun\n", stderr);
        exit(1);
    }
    if ((scsi = svcIoOpen(0, argv[1], NULL)) < 0)
        femsg(prgname, scsi, "Error opening SCSI device");
    option = 2 + (atoi(argv[2])<<8) + (atoi(argv[3])<<16);
    if ((rtn = svcIoSpecial(scsi, option, buffer, 255, &sdfp)) < 0)
        femsg(prgname, rtn, NULL);

    connect = contbl[buffer[0]>>6];

    temp = buffer[2]>>5;
    if (temp == 0)
        strcpy(isover, "None");
    else
        sprintf(isover, "%d", temp);

    temp = (buffer[2]>3) & 0x07;
    if (temp == 0)
        strcpy(ecmaver, "None");
    else
        sprintf(ecmaver, "%d", temp);

    temp = buffer[2] & 0x07;
    if (temp == 0)
        strcpy(ansiver, "None");
    else
        sprintf(ansiver, "%d", temp);

    temp = buffer[3] & 0x0F;
    if (temp <= 2)
        strcpy(format, fmttbl[temp]);
    else
        sprintf(format, "Reserved (%d)", temp);

    aenc = (buffer[3] & 0x80)? "Yes": "No";
    trmiop = (buffer[3] & 0x40)? "Yes": "No";
    removev = (buffer[1] & 0x80)? "Yes": "No";
    devindex = buffer[0] & 0x1F;
    devmod = buffer[1] & 0x7F;
    if (devindex != 0x1F || devmod == 0)
        device = scsi2dev[devindex];
    else
    {
        if (devmod <= 10)
            device = scsi1dev[devmod];
        else
        {
            sprintf(devname, "SCSI 1 (%02.2X)", devmod);
            device = devname;
        }
    }
    reladr = (buffer[7] & 0x80)? "Yes": "No";
    buswidth = 8;
    if (buffer[7] & 0x20)
        buswidth = 16;
    if (buffer[7] & 0x40)
        buswidth = 32;
    sync = (buffer[7] & 0x10)? "Yes": "No";
    linked = (buffer[7] & 0x08)? "Yes": "No";
    queued = (buffer[7] & 0x02)? "Yes": "No";
    reset = (buffer[7] & 0x01)? "Soft": "Hard";
    printf("Resp len  = %-4d     Resp fmt  = %s\n"
           "ANSI ver  = %-4s     ECMA ver  = %-4s     ISO ver = %s\n"
           "Removable = %-4s     Device    = %s device\n"
           "Connected = %-4s     AENC      = %-4s     TrmIOP  = %s\n"
           "RelAdr    = %-4s     Bus width = %-4d     Sync    = %s\n"
           "Linked    = %-4s     Queued    = %-4s     Reset   = %s\n"
           "Vendor    = \"%-8.8s\"\n"
           "Product   = \"%-16.16s\"\n"
           "Revision  = ", rtn, format, ansiver, ecmaver, isover, removev,
           device, connect, aenc, trmiop, reladr, buswidth, sync, linked,
           queued, reset, &buffer[8], &buffer[16]);
    if (buffer[32] > 0x20 && buffer[32] < 0x80 &&
            buffer[33] > 0x20 && buffer[33] < 0x80 &&
            buffer[34] > 0x20 && buffer[34] < 0x80 &&
            buffer[35] > 0x20 && buffer[35] < 0x80)
        printf("\"%-4.4s\"", &buffer[32]);
    else
        printf("%u.%u.%u.%u", buffer[32], buffer[33], buffer[34], buffer[35]);
    printf("\nOther     = \"%-20.20s\"\n", &buffer[36]);
    exit(0);
}
