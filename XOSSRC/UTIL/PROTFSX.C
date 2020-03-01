//---------------------------------------------------------------------
// PROTFS.C - Program to indicate that a DOS file system is protected
//
// Written by: John R. Goltz
//
// Edit History:
// 04/03/95(sao)  - Added progasst package
// 05/14/95(sao)  - Added mute
// 05/16/95(sao)  - Changed exit codes to reflect XOSRTN
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
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
#include <PROGARG.H>
#include <PROGHELP.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSRTN.H>
#include <XOSDISK.H>
#include <XOSERMSG.H>
#include <XOSSTR.H>

#define VERSION 3
#define EDITNO  3

#define offsetof(strct, member) ((long)(((struct strct *)(NULL))->member))

long hndl;
long prot;

char copymsg[] = "";
long quiet = FALSE;     // TRUE if /QUIET specified
long mute  = FALSE;     // TRUE if /MUTE specified
char prgname[] = "PROTFS";	// Name of this program
char envname[] = "^ALG^PROTFS";
char example[] = "drive-name:";
char description[] = "This is used to determine whether or not a DOS file system protected under XOS.";

char diskname[12];		// Name of disk

Prog_Info pib;

char yes;
char no;

struct
{   text8_parm class;
    char       end;
} opnparms =
{  {PAR_GET|REP_TEXT, 8, IOPAR_CLASS}
};

struct
{   byte1_parm sattr;
    byte1_parm fattr;
//  byte4_parm prot;
    char       end;
} protparm =
{   {PAR_SET|REP_HEXV, 1, IOPAR_SRCATTR, A_SYSTEM|A_HIDDEN|A_RDONLY},
    {PAR_SET|REP_HEXV, 1, IOPAR_FILATTR, A_SYSTEM|A_HIDDEN|A_RDONLY}
};

struct
{   byte1_parm sattr;
    byte1_parm fattr;
//  byte4_parm prot;
    char       end;
} rmvparm =
{   {PAR_SET|REP_HEXV, 1, IOPAR_SRCATTR, A_SYSTEM|A_HIDDEN|A_RDONLY},
    {PAR_SET|REP_HEXV, 1, IOPAR_FILATTR, 0}
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
{   QFNC_WAIT|QFNC_DEVCHAR,	// qab_func     - Function
    0,				// qab_status   - Returned status
    0,				// qab_error    - Error code
    0,				// qab_amount   - Amount transfered
    0,				// qab_handle   - Device handle
    0,  			// qab_vector   - Vector for interrupt
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    DCF_VALUES,			// qab_option   - Options or command
    0,				// qab_count    - Count for transfer
    NULL, 0,			// qab_buffer1  - Pointer to first data buffer
    (void far *)&diskchars,	// qab_buffer2  - Pointer to second data buffer
    NULL, 0			// qab_parm     - Pointer to parameter area
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
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    DSF_DISMOUNT,		// qab_option
    0,				// qab_count
    NULL, 0,			// qab_buffer1
    NULL, 0,			// qab_buffer2
    &dismparm, 0		// qab_parm
};

int  hvdisk(char *arg);

// Switch settings functions

#define OPT(name) ((int (*)(arg_data *))name)

arg_spec options[] =
{   {"?"      , 0, NULL, OPT(opthelp), 0, "This message."},
    {"H*ELP"  , 0, NULL, OPT(opthelp), 0, "This message."},
    {"Q*UIET" , ASF_BOOL|ASF_STORE, NULL, &quiet , TRUE, "Suppress all but error messages."},
    {"M*UTE"  , ASF_BOOL|ASF_STORE, NULL, &mute  , TRUE, "Suppress all messages."},
    {NULL     , 0, NULL,     NULL     , 0, NULL}
};

//******************************
// Function: main - Main program
// Returned: Exit status
//******************************

int main(
    int   argc,
    char *argv[])

{
    long rtn;
    char protname[32];

	reg_pib(&pib);

	init_Vars();

    if (argc >= 2)
    {
	++argv;
    progarg(argv, PAF_EATQUOTE, options, NULL, hvdisk,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    if (!quiet && !mute)
        printf("PROTFS - version %d.%d\n", VERSION, EDITNO);

    if (diskname[0] == '\0')
    {
		if ( !mute )
		fprintf(stderr,"? ERROR %s:  No drive name specified\n",prgname);
		exit(EXIT_INVSWT);
    }

    if ((hndl = svcIoOpen(O_IN|O_OUT|O_PHYS, diskname, &opnparms)) < 0)
        femsg2(prgname, "Error opening disk", hndl, diskname);

    if (strcmp(opnparms.class.value, "DISK") != 0)
    {
		if ( !mute )
        fprintf(stderr, "? PROTFS: Device %s is not a disk\n", diskname);
        exit(EXIT_INVDRV);
    }

    if ((rtn = svcIoDevChar(hndl, &diskchars)) < 0)
        femsg2(prgname, "Error getting characteristics for disk", rtn,
                diskname);

    if (strncmp(diskchars.fstype.value, "DOS", 3) != 0 &&
            strncmp(diskchars.fstype.value, "DSS", 3) != 0)
    {
		if ( !mute )
        fprintf(stderr, "? PROTFS: Disk %s does not contain a DOS file "
                "system\n", diskname);
        exit(EXIT_INVDRV);
    }

    if (!yes && !no)
    {
		if ( !mute && !quiet )
        printf("Disk %s is %sprotected\n", diskname,
                (diskchars.protect.value[0] == 'Y')? "": "not ");
        exit(EXIT_NORM);
    }
    strmov(strmov(protname, diskname), "PROTECT.DIR");

    if (yes)
    {
        if (diskchars.protect.value[0] == 'Y')
        {
			if ( !mute )
            fprintf(stderr, "? PROTFS: Disk %s is already protected\n",
                    diskname);
            exit(EXIT_INVSWT);
        }

        if ((rtn = svcIoDevParm(O_CREATE|O_FAILEX, protname, &protparm))
                < 0)
        {
            if (rtn == ER_FILEX)
            {
				if ( !mute )
                fprintf(stderr, "? PROTFS: Unexpected PROTECT.DIR entry found"
                        " in root directory on\n"
                        "          disk %s.  It must be removed before"
                        " disk can be protected.\n", diskname);
                exit(EXIT_INVSWT);
            }
            else
                femsg2(prgname, "Error creating PROTECT.DIR in root directory",
                        rtn, NULL);
        }
    }
    else
    {
        if (diskchars.protect.value[0] != 'Y')
        {
			if ( !mute )
            fprintf(stderr, "? PROTFS: Disk %s is not protected\n",
                    diskname);
            exit(EXIT_INVSWT);
        }
        if ((rtn = svcIoDevParm(0, protname, &rmvparm)) < 0)
        {
            if (rtn == ER_FILNF)
            {
				if ( !mute )
                fprintf(stderr, "? PROTFS: Did not find expected PROTECT.DIR"
                        " entry in the root directory for\n"
                        "          disk %s.  The file structure on this disk"
                        " may be damaged.\n", diskname);
                exit(EXIT_INVDRV);
            }
            else
                femsg2(prgname, "Error removing PROTECT.DIR from root "
                        "directory", rtn, NULL);
        }
        if ((rtn = svcIoClose(rtn, C_DELETE)) < 0)
            femsg2(prgname, "Error removing PROTECT.DIR from root directory",
                    rtn, NULL);
    }

    return (EXIT_NORM);
}

//***************************************************
// Function: init_Vars - Set up the program information block
// Returned: none
//***************************************************
void init_Vars(void)
{
	// set Program Information Block variables
	pib.opttbl=options; 		// Load the option table
    pib.kwdtbl=NULL;
	pib.build=__DATE__;
	pib.majedt = VERSION; 			// major edit number
	pib.minedt = EDITNO; 			// minor edit number
	pib.copymsg=copymsg;
	pib.prgname=prgname;
	pib.desc=description;
	pib.example=example;
	pib.errno=0;
	getTrmParms();
	getHelpClr();
};

int hvdisk(
    char *arg)

{
    char *pnt;

    if (diskname[0])
    {
        if (yes || no)
        {
			if ( !mute )
            fputs("? PROTFS: Too many arguments\n", stderr);
            exit(EXIT_INVSWT);
        }
        strupr(arg);
        if (strcmp(arg, "Y") == 0 || strcmp(arg, "YES") == 0)
            yes = TRUE;
        else if (strcmp(arg, "N") == 0 || strcmp(arg, "NO") == 0)
            no = TRUE;
        else
        {
			if ( !mute )
            fprintf(stderr, "? PROTFS: Invalid second argument \"%s\"\n"
                            "          Must be YES (Y) or NO (N)\n", arg);
            exit(EXIT_INVSWT);
        }
        return (TRUE);
    }
    if (strlen(arg) > 10)
    {
		if ( !mute )
        fputs("? PROTFS: Illegal disk name\n", stderr);
        exit(EXIT_INVDRV);
    }
    pnt = strmov(diskname, strupr(arg)); // Copy disk name
    if (pnt[-1] != ':')
        *pnt = ':';
    return (TRUE);
}
