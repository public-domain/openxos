//--------------------------------------------------------------------------*
// RMVDRIVEF.C - XOS utility for setting up DOS-equivilent disk names
//
// Written by: Tom Goltz, John Goltz
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
#include <RMVDRIVE.H>
#include <XOSSTR.H>

struct dosnchar
{   text4_char dosname;
    text4_char dosname1;
    char       end;
} dosnchar =
{   {PAR_SET|REP_TEXT, 4, "DOSNAME" , ""},
    {PAR_SET|REP_TEXT, 4, "DOSNAME1", ""}
};

char diskcls[] = "DISK:";

char model[64];

struct
{
    text4_char  unittype;
    lngstr_char model;
    char        end;
} diskchar =
{   {PAR_GET|REP_TEXT, 4, "UNITTYPE"},
    {PAR_GET|REP_STR , 0, "MODEL", model, sizeof(model) - 1, sizeof(model) - 1},
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

// This function assigns DOS drive letters to removeable disks. The first
//   letter to be used for each type of disk (CDROM, Syquest, ZIP, or LS-120)
//   and the maximum number of disks of each type to check must be specified
//   by the caller.  A single letter is assigned to the base disk and the
//   same letter is assigned to the first partition on the disk.  If it is
//   desired to assigned letters to additional partitions, this must be done
//   manually using the DEVCHAR command.

int rmvdrivef(
    int      quiet,
    RMVSPEC *spec)

{
    TYPSPEC *pnt;
    char    *ptr;
    long     rtn;
    long     hndl;
    int      cnt;
    char     more;

    if (setjmp(&jmpbuf))
        return (1);

    diqab.qab_count = 0;
    diqab.qab_buffer2 = NULL;
    if ((rtn=svcIoQueue(&diqab)) < 0 || (rtn=diqab.qab_error) < 0)
        fail(rtn, NULL);
    numunits = diqab.qab_count = diqab.qab_amount;

    // Get the list of unit names

    diqab.qab_buffer2 = namelist = getspace(diqab.qab_count * 8);
    if ((rtn=svcIoQueue(&diqab)) < 0 || (rtn=diqab.qab_error) < 0)
        fail(rtn, NULL);

    // We now have a complete list of disk units.  We check each unit to
    //   determine if it is removable base unit, if it is, we assign it a
    //   drive letter if its type matches a type we have a letter specified
    //   for

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
        if ((ptr[0] == 'D' || ptr[0] == 'S') && strchr(ptr + 2, 'P') == NULL)
        {
            strmov(strnmov(unitname + 1, ptr, 8), ":");
	    if ((hndl = svcIoOpen(O_PHYS|O_NOMOUNT, unitname, &dsparms)) < 0)
		warn(hndl, "Error ascessing disk", unitname);
	    else
	    {
		if ((rtn = svcIoDevChar(hndl, &diskchar)) < 0)
		    error(rtn, unitname, pnt->ltr);
		if (*((long *)(diskchar.unittype.value)) == 'mrdC')
		    pnt = &(spec->cd);
		else if (*((long *)(diskchar.unittype.value)) == '3DHU')
		    pnt = &(spec->ls);
		else 
		{
		    strupr(model);
		    if (strstr(model, "SYQUEST") != NULL)
			pnt = &(spec->syq);
		    else if (strstr(model, "ZIP") != NULL)
			pnt = &(spec->zip);
		    else
			pnt = &(spec->oth);
		}
		if (pnt->ltr != 0 && --(pnt->max) >= 0 &&
			(dsparms.devsts.value & DS_REMOVE) != 0)
		{
		    dosnchar.dosname.value[0] = dosnchar.dosname1.value[0] =
			    pnt->ltr;
		    if ((rtn = svcIoDevChar(hndl, &dosnchar)) < 0)
			error(rtn, unitname, pnt->ltr);
		    else
		    {
			if (!quiet)
			{
			    char buffer[100];

			    rtn = strlen(unitname) - 1;
			    sprintf(buffer, "%c: defined as %s or %*.*sP1:\n",
				    pnt->ltr, unitname, rtn, rtn, unitname);
			    rmvdrivemessage(RDML_INFO, buffer);
			}
			pnt->ltr++;
		    }
		}
		svcIoClose(hndl, 0);
            }
        }
        ptr += 8;
    }
    return(0);
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
	rmvdrivemessage(RDML_ERROR, buffer);
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
    rmvdrivemessage(RDML_ERROR, buffer);
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
    rmvdrivemessage(RDML_FINERROR, buffer);
    longjmp(&jmpbuf, 1);
}
