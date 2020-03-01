//--------------------------------------------------------------------------*
// LABEL.C
// A disk volume label utility for XOS
//
// Written by: Tom Goltz
//
// Edit History:
// 04/16/91 (tmg) created initial version
// 08/20/92(brn) - Add comment block at start of file
// 05/06/94(brn) - Fixed bad paramter index and change handling of LABEL
//			operations in general
// 04/01/95(sao) - Added progasst package
// 05/14/95(sao) - Changed description, added mute option
// 05/16/95(sao) - Changed exit codes to reflect XOSRTN
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
//--------------------------------------------------------------------------*

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
#include <CTYPE.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSRTN.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <GLOBAL.H>
#include <XOSERRMSG.H>

#define VERSION 3
#define EDITNO  3
#define AF(func) (int (*)(arg_data *))func

long quiet=FALSE;
long mute=FALSE;

arg_spec options[] =
{
    {"Q*UIET" , ASF_BOOL|ASF_STORE,    NULL, &quiet,    TRUE, "No output, except error messages."},
    {"M*UTE"  , ASF_BOOL|ASF_STORE,    NULL, &mute ,    TRUE, "No output, except error messages."},
    {"H*ELP"  , 0,                     NULL, AF(opthelp), 0, "This message."},
    {"?"      , 0,                     NULL, AF(opthelp), 0, "This message."},
    {NULL     , 0,                     NULL, AF(NULL)    , 0, NULL}
};

char prgname[] = "LABEL";
char  envname[] = "^XOS^LABEL^OPT";
char copymsg[] = "";
char  example[] = "{/option} {device{:}} {label}";
char  description[] = "This command is used to change the volume label " \
    "of a disk.  The volume label is displayed at the top of DOS-compatible " \
    "directory listings and is used as information for the user only.  When " \
    "the command is executed, it will display the current volume label for " \
    "the specified drive or the current drive if no drive is specified.  " \
    "It will then prompt for the volume label.  You can type in a new " \
    "volume label, or press ENTER to keep the current label or delete the " \
    "label.  The program will ask 'Delete current volume label (Y/N)?'.  " \
    "Press Y to delete the label or N to keep the same label.";
Prog_Info pib;

struct
{
    byte2_parm  attr;
    char        end;
} delparm =
{
    {(PAR_SET|REP_HEXV), 2, IOPAR_SRCATTR, A_LABEL},
    0
};

struct
{
    byte2_parm  filattr;
    char        end;
} newparm =
{
    {(PAR_SET|REP_HEXV), 2, IOPAR_FILATTR, A_LABEL},
    0
};

void labelask(char *device, char *label);
void getdefault(char *device);
void getlabel(char *device, char *buffer);
int nonOpt(char *);

char  label[32]="";        // Current volume label
char  device[32]="";
long dnm=FALSE;
long lbl=FALSE;

int main(int argc, char **argv)
{
    char  temp[64];
    long  rtn;

    argc=argc;
    reg_pib(&pib);

	init_Vars();

    global_parameter(TRUE);		// Check global configuration stuff

    argv++;
    progarg(argv, 0, options, NULL, nonOpt,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

    if (!lbl)           // Nothing specified on command line
    {
        if ( !dnm )
        {
            getdefault(device);     // Get default drive name
//            printf("#### Device is %s\n", device);
        }
        getlabel(device, label);    // Get current volume label
        labelask(device, label);
    }

    strcpy(temp, device);
    strcat(temp, "\*.*");
    svcIoDelete(0, temp, &delparm);    // Delete existing volume label

    if (*label != 0)			// Create new label?

    {
	strcat(label, "            ");	// Make sure plenty of spaces in label
	strncpy(label + 11, label + 12, 3);
	*(label + 9) = '.';
	strcpy(temp, device);
	strcat(temp, label);		// Build complete filename for label
    if ((rtn = svcIoOpen(O_CREATE|O_NOMOUNT|O_REQFILE, temp, &newparm)) < 0)
	    femsg(prgname, rtn, temp);
    svcIoClose(rtn, 0);
    }
    return (EXIT_NORM);
}

//***************************************************
// Function: init_Vars - Set up the program information block
// Returned: none
//***************************************************
void init_Vars(void){

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
}

//*******************************************************************
// Function: nonOpt - handle the user inputs of device name and label
// Returned: TRUE
//*******************************************************************
int nonOpt(char *inps)
{
    if ( !dnm )
    {
        dnm=TRUE;
        strcpy(device,inps);
        if ( strchr(inps,':')==NULL )
        {
            strcat(inps,":");
        }
    } else {
        lbl=TRUE;
        strcpy(label,inps);
    }
    return TRUE;
}

void labelask(char *device, char *label)
{
    char *ptr;

    printf("\nVolume in drive %s is %s\n\nVolume label (11 characters, ENTER for none)?", device, label);
    *label = '\0';			// Clear existing volume label
    fgets(label, 12, stdtrm);
    *(label + strlen(label) - 1) = 0;
    ptr = label;
    while (isspace(*ptr))
	++ptr;
    if (*ptr == 0)			// No new volume label?
    {
	fputs("Delete current volume label (Y/N)? ", stdout);
	fgets(label, 12, stdtrm);
	*(label + strlen(label) - 1) = 0;
	ptr = label;
	while (isspace(*ptr))		// Ignore leading whitespace on answer
	    ++ptr;
	switch (toupper(*ptr))
	{
	    case 'Y':
		*label = 0;
		return;
	    default:
		exit(EXIT_NORM);
	}
    }
    strcpy(label, ptr);			// Move to beginning of string
}

/*
 * Function to get current default device
 */

struct
{
    byte4_parm  filoptn;
    lngstr_parm filspec;
    char        end;
} deviceparm =
{
    {(PAR_SET|REP_HEXV), 4, IOPAR_FILOPTN, 0},
    {(PAR_GET|REP_STR ), 0, IOPAR_FILSPEC, NULL, 0, FILESPCSIZE, 0},
    0
};

void getdefault(char *device)
{
    type_qab deviceqab;
    long     rtn;

// Setup QAB values
    deviceqab.qab_func        = QFNC_WAIT|QFNC_PATH;
    deviceqab.qab_vector      = 0;
    deviceqab.qab_option      = 0;
    deviceqab.qab_buffer1     = "Z:";
    deviceqab.qab_buffer2     = NULL;
    deviceqab.qab_parm        = &deviceparm;

// Setup parameter values
    deviceparm.filoptn.value  = (gcp_dosdrive)? FO_NOPREFIX|FO_DOSNAME:
						FO_NOPREFIX|FO_XOSNAME;
    deviceparm.filspec.buffer = device;

// Get the device name
    if ((rtn = svcIoQueue(&deviceqab)) < 0 ||
	(rtn = deviceqab.qab_error) < 0)
	femsg(prgname, rtn, "Z:");	// Print error message and exit
// printf("Device = %s\n", device);
    return;
}

struct
{
    byte4_parm  filoptn;
    lngstr_parm filspec;
    char        end;
} labelparm =
{
    {(PAR_SET|REP_HEXV), 4, IOPAR_FILOPTN, FO_VOLNAME},
    {(PAR_GET|REP_STR ), 0, IOPAR_FILSPEC, NULL, 0, FILESPCSIZE, 0},
    0
};

/*
 * Function to get current volume label
 */

void getlabel(char *device, char *buffer)
{
    long     rtn;
    char     name[32];
    char     label[32];
    type_qab labelqab;

    strcpy(name, device);
    strcat(name, "\\*.*");

//    printf("#### Searching for %s\n", name);

    labelqab.qab_func         = QFNC_WAIT|QFNC_DEVPARM;
    labelqab.qab_vector       = 0;
    labelqab.qab_option       = 0;
    labelqab.qab_buffer1      = name;
    labelqab.qab_buffer2      = NULL;
    labelqab.qab_parm         = &labelparm;
    labelparm.filspec.buffer  = label;
    if ((rtn = svcIoQueue(&labelqab)) < 0 || (rtn=labelqab.qab_error) < 0)
    {
        *buffer = '\0';         // No volume label found
        femsg(prgname, rtn, NULL);
    } else {
        if ( ((int)label[0]&0xFF) == 0xE2 )
        {
            // Volume label found
            strcpy(buffer,label+1);
        } else {
            // No volume label found
            strcpy(buffer,label+1);
        }
    }
}
