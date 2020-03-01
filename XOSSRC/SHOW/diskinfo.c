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
#include <XOS.H>
#include <XOSSINFO.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>
#include "SHOW.H"

extern char prgname[];
char  *unitnames;

struct dichar
{   text16_char dosname;
    text16_char volname;
    text4_char  unittype;
    text4_char  remove;
    text4_char  msensor;
    text8_char  fstype;
    byte4_char  cblksz;
    byte4_char  clssz;
    byte4_char  clusters;
    byte4_char  avail;
    byte4_char  cheads;
    byte4_char  csects;
    byte4_char  ccylns;
    byte1_char  partn;
    char        end;
} dichar =
{   {PAR_GET|REP_TEXT, 16, "DOSNAME"},
    {PAR_GET|REP_TEXT, 16, "VOLNAME"},
    {PAR_GET|REP_TEXT,  4, "UNITTYPE"},
    {PAR_GET|REP_TEXT,  4, "REMOVE"},
    {PAR_GET|REP_TEXT,  4, "MSENSOR"},
    {PAR_GET|REP_TEXT,  8, "FSTYPE"},
    {PAR_GET|REP_DECV,  4, "CBLKSZ"},
    {PAR_GET|REP_DECV,  4, "CLSSZ"},
    {PAR_GET|REP_DECV,  4, "CLUSTERS"},
    {PAR_GET|REP_DECV,  4, "AVAIL"},
    {PAR_GET|REP_DECV,  4, "CHEADS"},
    {PAR_GET|REP_DECV,  4, "CSECTS"},
    {PAR_GET|REP_DECV,  4, "CCYLNS"},
    {PAR_GET|REP_HEXV,  1, "PARTN"}
};

char diskcls[] = "DISK:";

type_qab diqab =
{   QFNC_WAIT|QFNC_CLASSFUNC,	// qab_func
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    -1,				// qab_handle
    0,				// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    CF_PUNITS,			// qab_option
    0,				// qab_count
    (char *)&diskcls, 0,	// qab_buffer1
    NULL, 0,			// qab_buffer2
    NULL, 0			// qab_parm
};


int diskinfo(void)

{
    long     rtn;
    long     avail;
    long     total;
    long     cnt;
    int      numunits;
    char    *ptr;
    char     unitname[10];
    char     partnc;

    ++validcount;
    if ((rtn=svcIoQueue(&diqab)) < 0 || (rtn=diqab.qab_error) < 0)
        femsg(prgname, rtn, NULL);

    numunits = diqab.qab_count = diqab.qab_amount;
    diqab.qab_buffer2 = unitnames = getspace(diqab.qab_count * 8);
    if ((rtn=svcIoQueue(&diqab)) < 0 || (rtn=diqab.qab_error) < 0)
        femsg(prgname, rtn, NULL);


    // Sort the list of disk names

    partnc = FALSE;
    do
    {
        partnc = FALSE;
        cnt = numunits;
        ptr = unitnames;
        while (-- cnt > 0)
        {
            if ((ptr[0] != ptr[8]) ? ((ptr[0] != 'F' && ptr[8] == 'F') ? TRUE :
		    (ptr[0] != 'F' && ptr[0] > ptr[8])) :
		    (strncmp(ptr+1, ptr+9, 7) > 0))
            {
                strnmov(unitname, ptr, 8);
                strnmov(ptr, ptr+8, 8);
                strnmov(ptr+8, unitname, 8);
                partnc = TRUE;
            }
            ptr += 8;
        }
    } while (partnc);

    fputs("Disk DOS Unit F M File    Ptn  Blk Clst       Total          Free\n"
          "name ltr type R S system  Tbl size size          KB            KB  "
            "Hd Sct   Cyl\n", stdout);

    diqab.qab_func = QFNC_WAIT|QFNC_DEVCHAR;
    diqab.qab_option = O_NOMOUNT|O_PHYS|CF_VALUES;
    diqab.qab_buffer1 = NULL;
    diqab.qab_buffer2 = (char *)&dichar;
    diqab.qab_parm = NULL;

    while (--numunits >= 0)
    {
        strncpy(unitname, unitnames, 8);
        unitname[8] = '\0';
        printf("%-6s", unitname);
        strcat(unitname, ":");
        if ((diqab.qab_handle = svcIoOpen(O_PHYS|O_NOMOUNT, unitname,
                NULL)) < 0)
            femsg(prgname, rtn, NULL);
	if ((rtn=svcIoQueue(&diqab)) < 0 || (rtn=diqab.qab_error) < 0)
	    femsg(prgname, rtn, NULL);
	svcIoClose(diqab.qab_handle, 0);
        printf(" %1.1s ", dichar.dosname.value);
	if (dichar.partn.value == (char)0xFF)
	    dichar.partn.value = 0;
        partnc = dichar.partn.value & 0x7F;
        printf("%-4.4s %s %-8.8s %c%c%5ld", dichar.unittype.value,
                (dichar.remove.value[0] == 'N') ? "F  " :
                ((dichar.msensor.value[0] == 'N') ?
                "R N" : "R Y"), (stricmp(dichar.fstype.value, "NOTMNTD") == 0 ||
		stricmp(dichar.fstype.value, "NONE") == 0) ? "" :
		dichar.fstype.value, (dichar.partn.value & 0x80) ? 'E' : ' ',
		(partnc) ? partnc+'0' : ' ', dichar.cblksz.value);
        if (stricmp(dichar.fstype.value, "NOTMNTD") == 0)
            fputs("    Not mounted                ", stdout);
	else if (stricmp(dichar.fstype.value, "NONE") == 0)
            fputs("    No file structure          ", stdout);
        else
        {
            total = dichar.clusters.value * dichar.clssz.value;
            avail = dichar.avail.value * dichar.clssz.value;
            printf("%3d%,12d.%c%,12d.%c", dichar.clssz.value, total/2,
                (total & 1)? '5': '0', avail/2, (avail & 1)? '5': '0');
        }
        printf("%4d%4d%6d\n", dichar.cheads.value, dichar.csects.value,
                dichar.ccylns.value);
        unitnames += 8;
    }
    return (TRUE);
}
