//---------------------------------------------------------------------
// PROTFS.C - Program to indicate that a DOS file system is protected
//
// Written by: John R. Goltz
//
// Edit History:
//---------------------------------------------------------------------

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
#include <PROCARG.H>
#include <MALLOC.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSRTN.H>
#include <ERRMSG.H>
#include <XOSDISK.H>

#define VERSION 3
#define EDITNO  0

#define offsetof(strct, member) ((long)(((struct strct *)(NULL))->member))

long hndl;
long prot;

struct dirent *dpnt;
struct dirent *dblock;

char copymsg[] = "Copyright (c) 1991-1994 by Saguaro Software, Ltd.";
char quiet = FALSE;		// TRUE if /QUIET specified
char prgname[] = "PROTFS";	// Name of this program
char diskname[12];		// Name of disk

char yes;
char no;

struct dirent
{   char  name[8];
    char  ext[3];
    char  attrib;
    long  prot;
    char  resrvd[5];
    char  protcs;
    short time;
    short date;
    short cluster;
    long  length;
};

struct
{   text8_parm  class;
    char        end;
} opnparms =
{  {PAR_GET|REP_TEXT, 8, IOPAR_CLASS}
};

struct
{   text8_char fstype;
    text4_char protect;
    byte4_char rootblk;
    byte4_char rootsize;
    char       end;
} diskchars =
{  {PAR_GET|REP_TEXT, 8, "FSTYPE"},
   {PAR_GET|REP_TEXT, 4, "PROTECT"},
   {PAR_GET|REP_DECV, 4, "ROOTBLK"},
   {PAR_GET|REP_DECV, 4, "ROOTSIZE"}
};

type_qab charqab =
{   QFNC_WAIT|QFNC_DEVCHAR,	// qab_func    - Function
    0,				// qab_status  - Returned status
    0,				// qab_error   - Error code
    0,				// qab_amount  - Amount transfered
    0,				// qab_handle  - Device handle
    0, 0, 0,			// qab_vector  - Vector for interrupt
    DCF_VALUES,			// qab_option  - Options or command
    0,				// qab_count   - Count for transfer
    NULL, 0,			// qab_buffer1 - Pointer to first data buffer
    &diskchars,	0,		// qab_buffer2 - Pointer to second data buffer
    NULL, 0			// qab_parm    - Pointer to parameter area
};

struct
{   byte4_parm pos;
    char       end;
} posparms =
{  {PAR_SET|REP_DECV, 4, IOPAR_ABSPOS, 0}
};


struct
{
    text8_parm  class;
    char        end;
} dismparm =
{
    {(PAR_SET|REP_TEXT), 8, IOPAR_CLASS, "DISK"},
    0
};

type_qab dismqab =
{
    QFNC_WAIT|QFNC_SPECIAL,	// qab_open
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    0,				// qab_handle
    0,				// qab_vector
    0, 0,
    DS_DISMOUNT,		// qab_option
    0,				// qab_count
    NULL, 0,			// qab_buffer1
    NULL, 0,			// qab_buffer2
    &dismparm, 0		// qab_parm
};

int  findroot(void);
void helpprint(char *help_string, int state, int newline);
int  hvdisk(char *arg);
int  optquiet(arg_data *arg);
void optusage(void);

// Switch settings functions

#define OPT(name) ((int (*)(arg_data *))name)

arg_spec options[] =
{   {"?"        , 0, NULL, OPT(optusage), 0},
    {"HELP"     , 0, NULL, OPT(optusage), 0},
    {"HEL"      , 0, NULL, OPT(optusage), 0},
    {"H"        , 0, NULL, OPT(optusage), 0},
    {"QUIET"    , 0, NULL,     optquiet , TRUE},
    {"QUI"      , 0, NULL,     optquiet , TRUE},
    {"Q"        , 0, NULL,     optquiet , TRUE},
    {"NOQUIET"  , 0, NULL,     optquiet , FALSE},
    {"NOQUI"    , 0, NULL,     optquiet , FALSE},
    {"NOQ"      , 0, NULL,     optquiet , FALSE},
    {NULL       , 0, NULL,     NULL     , 0}
};

//******************************
// Function: main - Main program
// Returned: Exit status
//******************************

int main(
    int   argc,
    char *argv[])

{
    struct dirent block[16];
    long   rtn;
    int    cnt;
    int    dcnt;

    if (argc >= 2)
    {
		++argv;
		procarg(argv, PAF_EATQUOTE, options, NULL, hvdisk,
				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    if (!quiet)
        printf("PROTFS - version %d.%d\n", VERSION, EDITNO);

    if (diskname[0] == '\0')
    {
        fputs("? PROTFS: No disk specified\n", stderr);
        exit(1);
    }
    if ((hndl = svcIoOpen(O_IN|O_OUT|O_PHYS, diskname, &opnparms)) < 0)
        femsg2(prgname, "Error opening disk", hndl, diskname);

    if (strcmp(opnparms.class.value, "DISK") != 0)
    {
        fprintf(stderr, "? PROTFS: Device %s is not a disk\n", diskname);
        exit(1);
    }
    if ((rtn = svcIoDevChar(hndl, &diskchars)) < 0)
        femsg2(prgname, "Error getting characteristics for disk", rtn,
                diskname);
    if (strncmp(diskchars.fstype.value, "DOS", 3) != 0 &&
            strncmp(diskchars.fstype.value, "DSS", 3) != 0)
    {
        fprintf(stderr, "? PROTFS: Disk %s does not contain a DOS file "
                "system\n", diskname);
        exit(1);
    }
    if (!yes && !no)
    {
        printf("Disk %s is %sprotected\n", diskname,
                (diskchars.protect.value[0] == 'Y')? "": "not ");   
        exit(0);
    }
    dblock = block;
    if (yes)
    {
        if (diskchars.protect.value[0] == 'Y')
        {
            fprintf(stderr, "? PROTFS: Disk %s is already protected\n",
                    diskname);
            exit(1);
        }

        if (findroot())
        {
            fprintf(stderr, "? PROTFS: Found unexpected PROTECT.XOS directory"
                    " entry on disk %s\n", diskname);
            exit(1);
        }

        // Here with unprotected disk which we want to protect.  We scan the
        //   entire root directory looking for an empty slot for the
        //   PROTECT.XOS entry.

        posparms.pos.value = diskchars.rootblk.value * 512 - 512;
        dcnt = diskchars.rootsize.value;
        do
        {
            posparms.pos.value += 512;
            if((rtn = svcIoInBlockP(hndl, (char *)block, 512, &posparms)) < 0)
                femsg2(prgname, "Error reading root directory", rtn, diskname);
            dpnt = block;
            cnt = 16;
            do
            {
                if (dpnt->name[0] == 0 || dpnt->name == 0xE5)
                    break;
                dpnt++;
            } while (--cnt > 0);
        } while (cnt == 0 && --dcnt > 0);
        if (dcnt == 0)
        {
            fprintf(stderr, "? PROTFS: The root directory of disk"
                    " %s is full.  This disk cannot be\n"
                    "          protected unless a file is deleted"
                    " from the root directory.\n", diskname);
            exit(1);
        }

        // Here with a free slot in the first block of the root directory for
        //   the ROOT entry.  Now create the PROTECT.XOS entry

        strmov(dpnt->name, "PROTECT XOS\x07");
        prot = 0x1010181B;
        dpnt->prot = prot;
        dpnt->protcs = 0x55 ^ (prot) ^ (prot>>8) ^ (prot>>16) ^ (prot>>24);
        dpnt->date = 0;
        dpnt->time = 0;
        dpnt->length = 0;
        dpnt->cluster = 0;
    }
    else
    {
        if (diskchars.protect.value[0] != 'Y')
        {
            fprintf(stderr, "? PROTFS: Disk %s is not protected\n",
                    diskname);
            exit(1);
        }
        if (!findroot())
        {
            fprintf(stderr, "? PROTFS: Did not find expected PROTECT.XOS entry"
                    " in the root directory for\n"
                    "          disk %s.  The file structure on this disk is"
                    " probably damaged.\n", diskname);
            exit(1);
        }
        else
            dpnt->name[0] = 0xFFFFFFE5;
    }

    if((rtn = svcIoOutBlockP(hndl, (char *)block, 512, &posparms)) < 0)
        femsg2(prgname, "Error writing root directory", rtn, diskname);

    dismqab.qab_handle = hndl;
    if ((rtn = svcIoQueue(&dismqab)) < 0 || (rtn = dismqab.qab_error) < 0)
        fprintf(stderr, "? PROTFS: Warning: Unable to dismount disk %s, change"
                " in protection status\n",
                "          will not take effect until system is rebooted.\n",
                diskname);
    else
    {
	svcIoClose(hndl, 0);
	if ((hndl = svcIoOpen(O_IN|O_OUT|O_PHYS, diskname, NULL)) < 0)
	    femsg2(prgname, "Error re-opening disk", hndl, diskname);
	if ((rtn = svcIoDevChar(hndl, &diskchars)) < 0)
	    femsg2(prgname, "Error getting characteristics for disk", rtn,
		    diskname);
	printf("Disk %s now is %sprotected\n", diskname,
		(diskchars.protect.value[0] == 'Y')? "": "not ");
    }
    return (0);
}

//*********************************************************************
// Function: findroot - Find root directory entry in the root directory
// Returned: TRUE if found entry, FALSE if not there (does not return
//		if other error)
//*********************************************************************

int findroot(void)

{
    long rtn;
    int  cnt;
    int  dcnt;

    posparms.pos.value = diskchars.rootblk.value * 512 - 512;
    dcnt = diskchars.rootsize.size;
    do
    {
        posparms.pos.value += 512;
        if((rtn = svcIoInBlockP(hndl, (char *)dblock, 512, &posparms)) < 0)
            femsg2(prgname, "Error reading root directory", rtn, diskname);
        dpnt = dblock;
        cnt = 16;
        do
        {
            if (strncmp(dpnt->name, "PROTECT XOS\x07", 12) == 0)
            {
                if (dpnt->cluster != 0 || dpnt->length != 0)
                {
                    fprintf(stderr, "? PROTFS: Root directory PROTECT.XOS entry on"
                            " disk %s is invalid\n", diskname);
                    exit(1);
                }
                return (TRUE);
            }
            dpnt++;
        } while (--cnt > 0);
    } while (--dcnt > 0);
    return (FALSE);
}

//*************************************************
// Function: helpprint - Prints help option entries
// Returned: Nothing
//*************************************************

void helpprint(char *helpstring, int state, int newline)

{
    char str_buf[132];

    strcpy(str_buf, helpstring);
    if (state)
        strcat(str_buf, " *");

    if (newline)
    {
        fprintf(stderr, "%s\n", str_buf);
    }
    else
        fprintf(stderr, "%-38s", str_buf);

}

void optusage(void)

{
    fprintf(stderr, "\n%s version %d.%d (%s) %s\n\n", prgname,
			VERSION, EDITNO, __DATE__, copymsg);
    fprintf(stderr, "%s {/option} {...\}file_specs\n", prgname);
    fprintf(stderr, "\nOptions:\n");
    helpprint(" HELP or ? - this message", FALSE, TRUE);
    helpprint(" {NO}QUIET   - Supress routine output messages", quiet, TRUE);
    fprintf(stderr, "\nA * shows this option is the current default.\n");
    fprintf(stderr, "All options and values may be abbreviated to 1 or 3 letters.\n");
    exit(EXIT_INVSWT);          // Return as if invalid option
}

int optquiet(
    arg_data *arg)

{
    quiet = arg->data;
    return (TRUE);
}

int hvdisk(
    char *arg)

{
    char *pnt;

    if (diskname[0])
    {
        if (yes || no)
        {
            fputs("? PROTFS: Too many arguments\n", stderr);
            exit(1);
        }
        strupr(arg);
        if (strcmp(arg, "Y") == 0 || strcmp(arg, "YES") == 0)
            yes = TRUE;
        else if (strcmp(arg, "N") == 0 || strcmp(arg, "NO") == 0)
            no = TRUE;
        else
        {
            fprintf(stderr, "? PROTFS: Invalid second argument \"%s\"\n"
                            "          Must be YES (Y) or NO (N)\n", arg);
            exit(1);
        }
        return (TRUE);
    }
    if (strlen(arg) > 10)
    {
        fputs("? PROTFS: Illegal disk name\n", stderr);
        exit(1);
    }
    pnt = strmov(diskname, strupr(arg)); // Copy disk name
    if (pnt[-1] != ':')
        *pnt = ':';
    return (TRUE);
}
