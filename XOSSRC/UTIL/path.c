//==========================================================================
// Path.c
// DOS-compatible command to set command search path
//
// Written by: John R. Goltz
//
// Edit History:
// 08/20/92(brn) - Add comment header
// 09/30/92(brn) - Fix to handle dos syntax
// 14Dec94 (fpj) - Re-wrote code so that it works on XOS.
// 04/03/95(sao) - Added progasst package
// 05/14/95(sao) - Changed example
// 05/16/95(sao) - Changed exit codes to reflect XOSRTN
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
//==========================================================================

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
#include <STRING.H>
#include <CTYPE.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSRTN.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>

#define SYS_DELIM_CHAR      ','         // System path list delimiter
#define SYS_DELIM_STR       ","         // System path list delimiter
#define SYS_DELIM_LEN       (sizeof(SYS_DELIM_STR) - 1)
                                        // Length of system delimiter string

#define USER_DELIM_CHAR     ';'         // User path list delimiter
#define USER_DELIM_STR      ";,"        // User path list delimiters

#define DSK_DEVICE          "Z:"        // Default (DSK:) device
#define DSK_DEV_LEN         (sizeof(DSK_DEVICE) - 1)
                                        // Length of default device string

#define DEVICE_DELIM_CHAR   ':'         // Device delimiter
#define PATH_DELIM_CHAR     '\\'        // Path delimiter

static void getpath(void);
static void setpath(char *usrlist);
int nonOpt(char *);

#define VERSION 3
#define EDITNO  3

arg_spec options[] =
{
    {"H*ELP"  , 0, NULL, AF(opthelp), 0, "This message."},
    {"?"      , 0, NULL, AF(opthelp), 0, "This message."},
    {NULL     , 0, NULL, AF(NULL)    , 0}
};


Prog_Info pib;
char  prgname[] = "PATH";
char  envname[] = "^XOS^PATH";
char  copymsg[] = "";
char  example[] = "{new path}|{;}";
char  description[] = "This function is provided for DOS compatiblity.  " \
    "As such, it behaves exactly the same as the equivalent DOS command.  " \
    "The XOS logical name CMD: performs the same function for the " \
    "standard XOS environment, but adds certain features not found " \
    "in the PATH command.  It is recommended that the LOGICAL command be " \
    "used to redefine the CMD: logical name instead of using the PATH " \
    "command.  Typing PATH with no parameters will display the current " \
    "path.  Typing PATH; will remove the existing path specification.";
char  newpath[512]="";
char  gotPath=FALSE;

//  Function: main() - Start of PATH utility.
//
//  Return: 0 for success.
//
//  Command format:
//
//      PATH            (to display current path)
//
//      PATH list       (to set a command search path list)
//
//      PATH ;          (to disable the command search path list)
//
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    // FIXME: need to add code to check for /HELP and /?
    //        and print something useful.

    argc=argc;
	reg_pib(&pib);

	init_Vars();

    argv++;
    progarg(argv, 0, options, NULL, nonOpt,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

    if ( !gotPath )                       // Was a path specified?
        getpath();                      // No, so print it
    else
        setpath(newpath);               // Yes, so set it

    return (EXIT_NORM);
}

int nonOpt(char *inpstr)
{
    strcpy(newpath,inpstr);
    gotPath=TRUE;
    return TRUE;
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
}

//  Function: getpath() - Get the current path list and display it.
//
//  Return: (n/a)
//
////////////////////////////////////////////////////////////////////////////////

static void getpath(void)
{
    long   rcode;                       // SVC return code
    char   *nextpath;                   // Next path to process
    char   syslist[BUFSIZ];             // System path list
    char   usrlist[BUFSIZ];             // User path list
    char   *puser = usrlist;            // Pointer to user path list

    //  Find out what the system path list is

    rcode = svcIoFindLog(0, "CMD:", NULL, syslist, sizeof(syslist), 0);

    if (rcode < 0)                      // This better work (or else!)
    {
        femsg(prgname, rcode, "CMD:");  // Print error and exit
    }

    nextpath = strtok(syslist, SYS_DELIM_STR);
                                        // Get first path in list

    //
    //  Copy system path list to user path list, converting commas to
    //   semi-colons and suppressing trailing slashes after path names.
    //

    while (nextpath != NULL)
    {
        if (!strnicmp(nextpath, DSK_DEVICE, DSK_DEV_LEN))
            nextpath += DSK_DEV_LEN;

        if (*nextpath != '\0')
        {
            puser = strmov(puser, nextpath);
                                        // Add this path to user list

            if (*(puser - 1) == PATH_DELIM_CHAR)
            {
                --puser;                // Skip the trailing slash
            }

            *puser++ = USER_DELIM_CHAR; // Add semi-colon as delimiter
        }
        nextpath = strtok(NULL, SYS_DELIM_STR);
                                        // Get next path in list
    }
    *--puser = '\0';                    // Delete trailing semi-colon

    printf("PATH=%s\n", usrlist[0] != '\0' ? usrlist : "<none>");
}


//  Function: setpath() - Set the current path list.
//
//  Return: (n/a)
//
////////////////////////////////////////////////////////////////////////////////

void setpath(
    char *usrlist)                      // User path list
{
    long   rcode;                       // Returned code
    char   *nextpath = strtok(usrlist, USER_DELIM_STR);
                                        // Next path to process
    char   syslist[BUFSIZ] = DSK_DEVICE SYS_DELIM_STR "   ";
                                        // System path list
    char   *psystem = syslist + DSK_DEV_LEN + SYS_DELIM_LEN;
                                        // Pointer to system path list

    //
    //  Copy user path list to system path list, converting commas to
    //  semi-colons and ensuring trailing slashes after path names.
    //

    while (nextpath != NULL)
    {
        if (*nextpath != '\0')          // Skip null-length paths
        {
            psystem = strmov(psystem, nextpath);
                                        // Add this path to system list

            //  If last character copied was neither a slash nor a
            //  colon, then we have to add a colon.

            if (*(psystem - 1) != PATH_DELIM_CHAR
                    && *(psystem - 1) != DEVICE_DELIM_CHAR)
            {
                *psystem++ = PATH_DELIM_CHAR;
            }                           // Ensure slash at end of path

            *psystem++ = SYS_DELIM_CHAR; // Add comma as delimiter

            nextpath = strtok(NULL, USER_DELIM_STR);
                                        // Get next path in user list
        }
    }
    *--psystem = '\0';                  // Clear last semi-colon

    //
    //  See if the user specified Z: - if not, then point to the start
    //  of the path list so that we include it, else just start with
    //  what the user gave us. Note that the code below will do the
    //  right thing if the user specified a null list on the command
    //  line (e.g., "PATH ;").
    //

    if (strnicmp(syslist, syslist + DSK_DEV_LEN + SYS_DELIM_LEN, DSK_DEV_LEN))
    {
        psystem = syslist;
    }
    else
        psystem = syslist + DSK_DEV_LEN + SYS_DELIM_LEN;

    //  Now do the actual change to the logical name

    if ((rcode = svcIoDefLog(0, 0, "CMD:", psystem)) < 0)
        femsg(prgname, rcode, "CMD:");
}
