//--------------------------------------------------------------------------*
// DOSDRIVEF.C - XOS utility for setting up DOS-equivilent disk names
//
// Written by: Tom Goltz
//
// Edit History:
// 05/12/94(brn) - Fix command abbreviations and version number for 32 bit
//--------------------------------------------------------------------------*

// ++++
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
#include <CTYPE.H>
#include <SETJMP.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <DOSDRIVE.H>
#include <XOSSTR.H>

struct dosnchar
{   text4_char dosname;
    char       end;
} dosnchar =
{   {PAR_SET|REP_TEXT, 4, "DOSNAME", ""}
};

char diskcls[] = "DISK:";

struct
{
    text4_char unittype;
    text8_char fstype;
    byte1_char partn;
    char      end;
} diskchar =
{   {PAR_GET|REP_TEXT, 4, "UNITTYPE"},
    {PAR_GET|REP_TEXT, 8, "FSTYPE"},
    {PAR_GET|REP_HEXV, 1, "PARTN"}
};

struct drivedata
{
    char name[10];		// Device name of this drive
    char type;			// Partition type
    struct drivedata *next;	// Pointer to next drive
};

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


static struct
{   byte4_parm devsts;
    char       end;
} dsparms =
{   {PAR_GET|REP_HEXV, 4, IOPAR_DEVSTS}
};

jmp_buf  jmpbuf;

char    *namelist;
int      numunits;

int      driveltr = 'C';
char     unitname[16] = "_";


static void assigndrv(char *unitname, int letter, int quiet);
static void assigndrv2(long hndl, char *unitname, int letter, int quiet);
static void error(long  code, char *name, int letter);
static void fail(long code, char *name);
static void getunits(void);

// This function assigns DOS drive letters to XOS disks as follows:
//	F0: ==> A
//	F1: ==> B
//   Drive letters starting with C as assigned in the following order:
//	Primary DOS partition on Dn: drives, n = 0, 1, 2, ....
//	Primary DOS partition on Sn: drives, n = 0, 1, 2, ....
//	Secondary DOS partitions on Dn: drives, n = 0, 1, 2, ....
//	Secondary DOS partitions on Sn: drives, n = 0, 1, 2, ....
//   Removable drives are skipped.  Existing DOS drive letter assignments
//     are ignored; it is best to execute this function before any manual
//     DOS drive letter assignments are made.
// This is approximately the same way DOS/Windows assigns drive letters, but
//   may not match exactly for unusual configurations!



int dosdrivef(
    int quiet)

{
    long  rtn;
    long  hndl;
    int   cnt;
    char *ptr;
    char *dst;
    char  more;

    if (setjmp(&jmpbuf))
        return (1);

    // First, assign the two standard floppy drive letters

    assigndrv("_F0:", 'A', quiet);
    assigndrv("_F1:", 'B', quiet);

    // Get a list of all disks now known to the system and open all
    //   non-removable primary units.  This ensures that the system knows
    //   about all partitions

    getunits();				// Get the list of disk units
    ptr = namelist;
    cnt = numunits;
    while (--cnt >= 0)
    {
        if (ptr[0] == 'D' || ptr[0] == 'S')
        {
            strnmov(unitname + 1, ptr, 8);
            unitname[9] = 0;
            dst = unitname + 2;
            while(isdigit(*dst))
                ++dst;
            if (*dst == 0)
            {
////		printf("### Scan 1: unit = %s\n", unitname);

		strmov(dst, ":");
		if ((rtn = svcIoDevParm(O_PHYS|O_NOMOUNT, unitname, &dsparms))
			>= 0 && (dsparms.devsts.value & DS_REMOVE) == 0)
		    svcIoDevParm(O_PHYS, unitname, NULL);
            }
        }
        ptr += 8;
    }
    getunits();				// Get the list of units again (there
					//   may be more if we caused some
					//   partitions to be set up above)

    // We now have a complete list of disk units.  First we check each unit
    //   to determine if it is removable and, if so, remove it from the list.
    //   If it is not removable, we check to see if it is a primary DOS
    //   partition.  If so, we assign it a drive letter and remove it from the
    //   list.

    more = FALSE;			// Sort the list of hard disk names
    do
    {
        more = FALSE;
        cnt = numunits;
        ptr = namelist;
        while (-- cnt > 0)
        {
            if (strncmp(ptr, ptr + 8, 8) > 0)
            {
                strnmov(unitname + 1, ptr, 8);
                strnmov(ptr, ptr + 8, 8);
                strnmov(ptr + 8, unitname + 1, 8);
                more = TRUE;
            }
            ptr += 8;
        }
    } while (more);

    ptr = namelist;
    cnt = numunits;
    while (--cnt >= 0)
    {
        if (ptr[0] == 'D' || ptr[0] == 'S')
        {
            strmov(strnmov(unitname + 1, ptr, 8), ":");

//////	    printf("### Scan 2: unit = %s\n", unitname);

	    if ((rtn = svcIoDevParm(O_PHYS|O_NOMOUNT, unitname, &dsparms)) < 0)
		warn(rtn, "Error ascessing disk", unitname);
	    else
	    {
		if ((dsparms.devsts.value & DS_REMOVE) != 0)
		    ptr[0] = 'X';
		else
		{
		    if ((hndl = svcIoOpen(O_PHYS, unitname, NULL))
			    < 0)
			warn(hndl, "Error opening disk", unitname);
		    else
		    {
			if ((rtn = svcIoDevChar(hndl, &diskchar)) < 0)
			    warn(rtn, "Error geting characteristics for disk",
				    unitname);
			else
			{
			    if (strncmp(diskchar.fstype.value, "DOS", 3)
				    != 0 && strncmp(diskchar.fstype.value,
				    "DSS", 3) != 0)
				ptr[0] = 'X';
			    else if ((diskchar.partn.value & 0x80) == 0)
			    {
				assigndrv2(hndl, unitname, driveltr++, quiet);
				ptr[0] = 'X';
			    }
			}
			svcIoClose(hndl, 0);
		    }
		}
            }
        }
        ptr += 8;
    }

    // We have letters assigned to all primary DOS partitions.  Now assign
    //   letters to secondary DOS partitions.  We have removed all D or S
    //   names which do not correspond to secondary DOS paritions, so this
    //   is easy.

    ptr = namelist;
    cnt = numunits;
    while (--cnt >= 0)
    {
        if (ptr[0] == 'D' || ptr[0] == 'S')
        {
            strmov(strnmov(unitname + 1, ptr, 8), ":");
////	    printf("### Scan 3: unit = %s\n", unitname);
	    assigndrv(unitname, driveltr++, quiet);
        }
        ptr += 8;
    }
    return(0);
}

static void getunits(void)

{
    long rtn;

    diqab.qab_count = 0;
    diqab.qab_buffer2 = NULL;
    if ((rtn=svcIoQueue(&diqab)) < 0 || (rtn=diqab.qab_error) < 0)
        fail(rtn, NULL);
    numunits = diqab.qab_count = diqab.qab_amount;

    // Get the list of unit names

    diqab.qab_buffer2 = namelist = getspace(diqab.qab_count * 8);
    if ((rtn=svcIoQueue(&diqab)) < 0 || (rtn=diqab.qab_error) < 0)
        fail(rtn, NULL);
}

static void assigndrv(
    char *name,
    int   letter,
    int   quiet)

{
    long hndl;

    if ((hndl = svcIoOpen(O_PHYS|O_NOMOUNT, name, NULL)) < 0)
	error(hndl, name, letter);
    else
    {
	assigndrv2(hndl, name, letter, quiet);
	svcIoClose(hndl, 0);
    }
}

static void assigndrv2(
    long  hndl,
    char *name,
    int   letter,
    int   quiet)

{
    long rtn;

    dosnchar.dosname.value[0] = letter;
    if ((rtn = svcIoDevChar(hndl, &dosnchar)) < 0)
	error(rtn, name, letter);
    else
    {
	if (!quiet)
	{
	    char buffer[100];

	    sprintf(buffer, "%c: defined as %s\n", letter, name);
	    dosdrivemessage(DDML_INFO, buffer);
	}
    }
}

static void error(
    long  code,
    char *name,
    int   letter)

{
    char *pnt;
    char  buffer[100];		// Buffer to receive error message

    if (code != ER_NSDEV)
    {
	pnt = strmov(buffer, "Error assigning DOS name %s to disk %s\n"
		"            %s\n");
	pnt = buffer + sprintf(buffer, "Error assigning DOS name %c: to disk "
		"%s\n            ", letter, name);
	pnt += svcSysErrMsg(code, 3, pnt); // Get error message
	strmov(pnt, "\n");
	dosdrivemessage(DDML_ERROR, buffer);
    }
}

static void warn(
    long  code,
    char *msg,
    char *disk)

{
    char *pnt;
    char  buffer[100];

    pnt = buffer + sprintf(buffer, "%s %s\n", msg, disk);
    pnt += svcSysErrMsg(code, 3, pnt); // Get error message
    strmov(pnt, "\n");
    dosdrivemessage(DDML_ERROR, buffer);
}

void fail(
    long  code,
    char *name)

{
    char *pnt;
    char  buffer[100];

    pnt = buffer + svcSysErrMsg(code, 3, buffer); // Get error message
    if (name != NULL)
	strmov(strmov(strmov(pnt, "; "), name), "\n");
    dosdrivemessage(DDML_FINERROR, buffer);
    longjmp(&jmpbuf, 1);
}
