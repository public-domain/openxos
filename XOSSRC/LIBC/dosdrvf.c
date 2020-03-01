//--------------------------------------------------------------------------*
// DOSDRVF.C - XOS utility for setting up DOS-equivilent disk names
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

type_qab dosnqab =
{   QFNC_WAIT|QFNC_DEVCHAR,	// qab_func
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    -1,				// qab_handle
    0,				// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    O_RAW|O_NOMOUNT|DCF_VALUES,	// qab_option
    0,				// qab_count
    NULL, 0,			// qab_buffer1
    (char *)&dosnchar, 0,	// qab_buffer2
    NULL, 0			// qab_parm
};

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

type_qab dcqab =
{   QFNC_WAIT|QFNC_DEVCHAR,	// qab_func
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    -1,				// qab_handle
    0,				// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    DCF_VALUES,			// qab_option
    0,				// qab_count
    NULL, 0,			// qab_buffer1
    (char *)&diskchar, 0,	// qab_buffer2
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

char     unit[11] = "_";
char    *unitname = &unit[1];

char     nonsdevmsg = TRUE;

static void assigndrv(char *unitname, const char *driveletter, int quiet);
static void error(long code, const char *name1, const char *name2);
static void fail(long code, char *name);
static void getunits(void);

int dosdrvf(
    const char *driveletter,
    char  quiet)

{
    long     rtn;
    int      cnt;
    char    *ptr;
    char    *dst;
    char     temp[12];
    char     drive2;
    char     more;
    char     drivecopy[3];              // Copy of drive letter

    if (strlen(driveletter) >= sizeof(drivecopy))
        return (0);

    strcpy(drivecopy, driveletter);

    if (setjmp(&jmpbuf))
        return (0);

    assigndrv("_F0:", "A:", quiet);
    assigndrv("_F1:", "B:", quiet);
    dosnqab.qab_option = O_RAW|DCF_VALUES;
    nonsdevmsg = FALSE;
    getunits();				// Get the initial list of disk units

    // Scan the disks and open all primary hard disk units to make sure the
    //   partitions are all set up

    ptr = namelist;
    cnt = numunits;
    while (--cnt >= 0)
    {
        if (ptr[0] == 'D')
        {
            strnmov(temp, ptr, 8);
            temp[8] = 0;
            dst = temp + 1;
            while(isdigit(*dst))
                ++dst;
            if (*dst == 0)
            {
                strmov(dst, ":");
                if ((rtn = svcIoDevParm(O_PHYS, temp, NULL)) < 0 &&
			rtn != ER_NTRDY)		
                    fail(rtn, temp);
            }
        }
        ptr += 8;
    }

    getunits();				// Get the list of units again (there
					//   may be more if we caused some
					//   partitions to be set up above)

    // Check if there's a second hard drive with a primary DOS partition
    //   If present, this will be assigned D:

    cnt = numunits;
    ptr = namelist;
    drive2 = FALSE;
    while (--cnt >= 0)
    {
	if (strncmp(ptr, "D1P1", 5) != 0)
	{
	    ptr += 8;
	    continue;
	}
        strncpy(unitname, ptr, 8);
        unitname[8] = '\0';
        strcat(unitname, ":");
        if ((dcqab.qab_handle = svcIoOpen(O_PHYS, unitname,
                NULL)) < 0)
            fail(dcqab.qab_handle, unitname);
        else
        {
            if ((rtn=svcIoQueue(&dcqab)) < 0 || (rtn=dcqab.qab_error) < 0)
                fail(rtn, unitname);
            svcIoClose(dcqab.qab_handle, 0);
        }
	if ((diskchar.partn.value & 0x80) == 0)	// Not an extended partition
	    drive2 = TRUE;
	break;
    }

    // Remove the floppy disk names from the list of disk names

    cnt = numunits;
    ptr = dst = namelist;
    do
    {
	printf("### unit = %s\n", ptr);

        if (ptr[0] != 'F')
        {
            strnmov(dst, ptr, 8);
            dst += 8;
        }
        else
            numunits--;
        ptr += 8;
    } while (--cnt > 0);
    if (numunits == 0)			// All finished if no hard disks
        exit(0);

    // Sort the list of hard disk names

    more = FALSE;
    do
    {
        more = FALSE;
        cnt = numunits;
        ptr = namelist;
        while (-- cnt > 0)
        {
            if (strncmp(ptr, ptr+8, 8) > 0)
            {
                strnmov(temp, ptr, 8);
                strnmov(ptr, ptr+8, 8);
                strnmov(ptr+8, temp, 8);
                more = TRUE;
            }
            ptr += 8;
        }
    } while (more);

    // Assign all DOS drive letters for hard disks

    cnt = numunits;
    ptr = namelist;

    printf("### num units = %d\n", cnt);

    while (--cnt >= 0)
    {
	strmov(strnmov(unitname, ptr, 8), ":");
	dsparms.devsts.value = 0;

	printf("### name = %s\n", unitname);

	if ((dcqab.qab_handle = svcIoOpen(O_PHYS, unitname, &dsparms)) < 0)
	{
	    if (dcqab.qab_handle != ER_NTRDY)
		fail(dcqab.qab_handle, unitname);
	}
	else
	{
	    if ((dsparms.devsts.value & DS_REMOVE) == 0)
	    {

		if ((rtn=svcIoQueue(&dcqab)) < 0 || (rtn=diqab.qab_error) < 0)
		    fail(rtn, unitname);
		svcIoClose(dcqab.qab_handle, 0);
		if (strncmp(diskchar.fstype.value, "DOS", 3) == 0 ||
			strncmp(diskchar.fstype.value, "DSS", 3) == 0)
		{			// DOS file structure?
		    switch (unitname[1]) // Yes - Drive unit number?
		    {
		     case '0':		// First hard drive
			assigndrv(unit, drivecopy, quiet);
			*drivecopy += 1; // Advance to next drive letter
			if (drive2 && unitname[3] == '1') // First partition?
			    *drivecopy += 1; // Leave a drive letter for the
			break;		     //   2nd drive

		     case '1':		// Second hard drive
			if (drive2 && unitname[3] == '1')
			{
			    assigndrv(unit, "D:", quiet);
			    drive2 = FALSE;
			    break;
			}
		     default:		// Here with all other drives, and
					//   most of the second drive
			assigndrv(unit, drivecopy, quiet);
			*drivecopy += 1;
			break;
		    }
		}
            }
        }
        ptr += 8;
    }

    // Now assign drive letters to non-DOS file structures

    ptr = namelist;
    while (--numunits >= 0)
    {
	strmov(strnmov(unitname, ptr, 8), ":");
	if ((dcqab.qab_handle = svcIoOpen(O_PHYS, unitname, NULL)) < 0)
	{
	    if (dcqab.qab_handle != ER_NTRDY)
		fail(dcqab.qab_handle, unitname);
	}
	else
        {
	    if ((dsparms.devsts.value & DS_REMOVE) == 0)
	    {
		if ((rtn=svcIoQueue(&dcqab)) < 0 || (rtn=diqab.qab_error) < 0)
		    fail(rtn, unitname);
		svcIoClose(dcqab.qab_handle, 0);


		if (strcmp(diskchar.fstype.value, "NONE") != 0 &&
			strncmp(diskchar.fstype.value, "DOS", 3) != 0 &&
			strncmp(diskchar.fstype.value, "DSS", 3) != 0)
		{
		    assigndrv(unit, drivecopy, quiet);
		    *drivecopy += 1;
		}
	    }
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
    char *unitname,
    const char *driveletter,
    int   quiet)

{
    long rtn;

    dosnqab.qab_buffer1 = unitname;
    dosnchar.dosname.value[0] = driveletter[0];
    if ((dosnqab.qab_handle = svcIoOpen(O_PHYS|O_NOMOUNT, unitname, NULL)) < 0)
        error(dosnqab.qab_handle, driveletter, unitname);
    else
    {
        if ((rtn = svcIoQueue(&dosnqab)) < 0 || (rtn = dosnqab.qab_error) < 0)
            error(rtn, driveletter, unitname);
        else if (!quiet)
        {
            char buffer[100];

            sprintf(buffer, "%s defined as %s\n", driveletter, unitname);
            message(DDML_INFO, buffer);
        }
        svcIoClose(dosnqab.qab_handle, 0);
    }
}

static void error(
    long  code,
    const char *name1,
    const char *name2)

{
    char *pnt;
    char  buffer[100];		// Buffer to receive error message

    if (!nonsdevmsg || code != ER_NSDEV)
    {
        pnt = strmov(buffer, "Error assigning DOS name %s to disk %s\n"
                "            %s\n");


        pnt = buffer + sprintf(buffer, "Error assigning DOS name %s to disk %s\n"
                    "            ", name1, name2);
        pnt += svcSysErrMsg(code, 3, pnt); // Get error message
        strmov(pnt, "\n");
        message(DDML_ERROR, buffer);
    }
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
    message(DDML_FINERROR, buffer);
    longjmp(&jmpbuf, 1);
}
