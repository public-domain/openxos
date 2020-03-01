//--------------------------------------------------------------------------
// DEFAULT.C
// Default parameter utility for XOS
//
// Original version written by Tom Goltz, current version written by
//   John GOltz
//
// Edit History:
// 12-Oct-91 (TMG) 0.1 - Created initial version
//  1-Feb-94       3.0 - Initial 32-bit version
// 30-Apr-94 (JRG) 3.1 - Rewrote to fix major problems
//  6-Mar-95 (SAO) 3.2 - Added progasst package
// 05/13/95  (sao) 3.3 - Changed 'option' indicator
// 05/16/95  (sao) -.- - Changed return codes to match XOSRTN
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
//--------------------------------------------------------------------------

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
#include <XOSERR.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSRTN.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>

#define VERSION 3
#define EDITNO  4

void nowild(char *msg);
int nonOpt(char *);
int optMute(arg_data *);
int optQuiet(arg_data *);
int optOther(arg_data *);
int optEdit(arg_data *);
int optDel(arg_data *);
void skipws(void);

arg_spec options[] =
{
    {"H*ELP"  , 0, NULL, AF(opthelp), 0, "This message."},
    {"?"      , 0, NULL, AF(opthelp), 0, "This message."},
    {"Q*UIET" , ASF_BOOL, NULL, optQuiet, TRUE, "No output, except error messages."},
    {"M*UTE"  , ASF_BOOL, NULL, optMute, TRUE, "No output, even error messages."},
    {"D*ELETE", 0, NULL, optDel, TRUE, "Delete the default options for the selected program."},
    {"E*DIT"  , 0, NULL, optEdit, TRUE, "Edit the default options for the selected program."},
    {"*"      , ASF_NVAL|ASF_LSVAL, NULL, optOther, TRUE, NULL },
    {NULL     , 0, NULL, AF(NULL)    , 0}
};


Prog_Info pib;
long  quiet;            // TRUE if should supress unneccessary output
long  mute;
char  edit;			// TRUE if should edit existing string
char  delete;			// TRUE if deleting name
char  wild;			// TRUE if wild-card seen in name
char  local=TRUE;
char  parmBufStarted=FALSE;
char  parmBuf[1024];
char  targProg[80]="";
char *program;			// Name of program to process
char *apnt;
char  prgname[] = "DEFAULT";
char  copymsg[] = "";
char  example[] = "{/options} CMDNAME {default options}";
char  description[] = "DEFAULT provides a convenient method for setting " \
    "or modifying the environment strings which specify the default " \
    "options for most XOS commands.  Options specified after the " \
    "CMDNAME argument are copied to the environment string ^XOS^CMDNAME^OPT " \
    "which will be used whenever the command CMDNAME is issued to set its " \
    "initial options.  Options specified before the CMDNAME argument apply " \
    "to this command itself, as described above.  If GLOBAL is specified as " \
    "the CMDNAME argument, special global default options are set which " \
    "apply to all commands.  The valid global options are /DOSQUIRK=ON|OFF " \
    "and /DOSDRIVE=ON|OFF.";

void main(
    int   argc,
    char *argv[])

{
    long   cnt;
    long   rtn;
    char  *cpnt;
    char   envname[48];
    char   found[64];
    char   chr;

	reg_pib(&pib);

	init_Vars();

    argc = argc;

    if (argc !=0)
        ++argv;
    progarg(argv, 0, options, NULL, nonOpt,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

    if (strlen(targProg) > 32)
    {
        if ( !mute )
            fprintf(stderr, "? DEFAULT: Program name \"%s\" is too long\n",
                program);

        exit(EXIT_INVSWT);
    }
    cpnt = targProg;
    while ((chr = *cpnt++) != 0)
    {
        if (chr == '*' || chr == '?' || chr == '{' || chr == '}')
        {
            wild = TRUE;
            break;
        }
    }

    if (targProg != NULL)
        strmov(strmov(strmov(envname, "^XOS^"), targProg), "^OPT");


    if (delete && edit)
    {
        if ( !mute )
        fputs("? DEFAULT: Command conflict: both /DELETE and /EDIT given\n",
                stderr);
        exit(EXIT_INVSWT);
    }

    if (delete)				// Delete existing options?
    {
        if (parmBuf[0] != 0)
        {
            if ( !mute )
            fputs("? DEFAULT: Command conflict: both /DELETE and value given\n",
                    stderr);
            exit(EXIT_INVSWT);
        }

        if (targProg == NULL)
        {
            if ( !mute )
            fputs("? DEFAULT: No program specified with /DELETE\n", stderr);
            exit(EXIT_INVSWT);
        }

        nowild("/DELETE");

        if ((rtn = svcSysDefEnv(FES_SESSION, envname, NULL)) < 0)
        {
            if (rtn == ER_NTDEF)    // Not defined?
            {
                if ( !mute && !quiet )
                    fprintf(stderr, "No default options now defined "
                            "for %s\n", targProg);
                exit(EXIT_NORM);
            }
            else
            {
                sprintf(parmBuf, "Error deleting environment string %s",
                        envname);
                femsg2(prgname, parmBuf, rtn, NULL);
            }
        }
        else
            if (!quiet && !mute)
                printf("Default options for %s deleted\n", targProg);
        exit(EXIT_NORM);
    }

    if (edit)				// Edit existing string?
    {
        if (parmBuf[0] != 0)
        {
            if ( !mute )
                fputs("? DEFAULT: Command conflict: both /EDIT and value given\n",
                    stderr);
            exit(EXIT_INVSWT);
        }
        if (targProg == NULL)
        {
            if ( !mute )
                fputs("? DEFAULT: No program specified with /EDIT\n", stderr);
            exit(EXIT_INVSWT);
        }
        nowild("/EDIT");
        if ((rtn=svcSysFindEnv(FES_SESSION, envname, NULL, parmBuf,
                    sizeof(parmBuf), NULL)) > 0)
        {
            if ((rtn = svcTrmWrtInB(STDTRM, parmBuf, rtn)) < 0)
            femsg2(prgname, "Error writing string to terminal buffer", rtn,
                            NULL);
            if ((rtn = svcIoInBlock(STDTRM, parmBuf, sizeof(parmBuf))) < 0)
            femsg2(prgname, "Error reading edited string from terminal"
                            " buffer", rtn, NULL);
            parmBuf[rtn] = 0;
                cpnt = parmBuf;
                while ((chr = *cpnt) != 0 && chr != '\n' && chr != '\r')
                    ++cpnt;
                *cpnt = 0;
        }
        else
        {
            if ( !mute && !quiet )
                printf("No existing default options for %s\n", targProg);
            exit(EXIT_INVSWT);
        }
    }

    // Here if neither /EDIT or /DELETE seen


    if (targProg[0] == '\0')
    {
        cnt = 0;
		if ( !quiet && !mute )
        printf("Current default options:\n");
        while (svcSysFindEnv(0, "^XOS^*^OPT", found, parmBuf, sizeof(parmBuf),
                &cnt) >= 0)
        {
            *(found + strlen(found) - 4) = 0; // Remove the ^OPT from end
			if ( !quiet && !mute )
            printf("  %-10.10s%s\n", found+5, parmBuf);
        }
        exit(EXIT_NORM);
    }
    if (parmBuf[0] == 0)         // Was a new value specified?
    {					// No - display current value
        cnt = 0;

        do
        {
            if ((rtn=svcSysFindEnv(FES_SESSION, envname, found, parmBuf,
                    sizeof(parmBuf), &cnt)) > 0)
            {
                *(found + strlen(found) - 4) = 0; // Remove the ^OPT from end
				if ( !quiet && !mute )
                printf("Default options for %s are \"%s\"\n", found+5, parmBuf);
            }
            else
            {
                if (rtn != ER_NTDEF)
                {
                    sprintf(parmBuf, "Error searching for environment string %s",
                            envname);
                    femsg2(prgname, parmBuf, rtn, NULL);
                }
                if (cnt == 0)
					if ( !quiet && !mute )
                    printf("No default options defined for %s\n", targProg);
                exit(EXIT_NORM);
            }

        } while (wild);
        exit(EXIT_NORM);
    }

    // Here if a new value was specified
    nowild("value");
    if ((rtn = svcSysDefEnv(FES_SESSION, envname, parmBuf)) < 0)
    {
        sprintf(parmBuf, "Error defining environment string %s", envname);
        femsg2(prgname, parmBuf, rtn, NULL);
    }
    else
        if (!quiet && !mute)
            printf("Default options for %s defined\n", targProg);
    exit(EXIT_NORM);
}

//***************************************************
// Function: init_Vars - Set up the program information block
// Returned: none
//***************************************************
void init_Vars(void){

	// Set defaults of control variables
	quiet=FALSE;
	mute=FALSE;

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




void nowild(
    char *str)

{
    if (wild)
    {
        if ( !mute )
        fprintf(stderr, "? DEFAULT: Wild card characters not allowed with %s\n",
            str);
        exit(EXIT_INVSWT);
    }
}

int nonOpt(char *s)
{
    if ( local )
    {
        strcpy(targProg,strupper(s));
        local=FALSE;
        return TRUE;
    } else {
        if ( !mute )
        {
            fprintf(stderr,"Two program names were given. [%s] and [%s]\n",targProg,s);
        }
        exit(EXIT_INVSWT);
    };
    return FALSE;
}

int optOther(arg_data *arg)
{
    char temp[80];
    // Add the option to the target programs' list
    if ( parmBufStarted )
    {
        sprintf(temp," /%s",arg->name);
        strcat(parmBuf,temp);
    } else {
        sprintf(parmBuf,"/%s",arg->name);
        parmBufStarted=TRUE;
    }
    if ( !(arg->flags & ADF_NONE) && !(arg->flags & ADF_NULL) )
    {
        if ( arg->flags & ADF_NVAL )
        {
            sprintf(temp,"=%ld",arg->val.n);
            strcat(parmBuf,temp);
            return TRUE;
        };
        if (arg->flags & ADF_LSVAL)
        {
            sprintf(temp,"=%s",arg->val.s);
            strcat(parmBuf,temp);
            return TRUE;
        }
    }
    return TRUE;
}


int optQuiet(arg_data *arg)
{

    if ( local )
    {
        if ( !(arg->flags & ASF_NEGATE) )
        {
            quiet=TRUE;
        } else {
            quiet=FALSE;
        }
    } else {
        optOther(arg);
    }
    return TRUE;
}

int optMute(arg_data *arg)
{

    if ( local )
    {
        if ( !(arg->flags & ASF_NEGATE) )
        {
            mute=TRUE;
        } else {
            mute=FALSE;
        }
    } else {
        // Add the 'mute' option to the target programs' list
        optOther(arg);
    }
    return TRUE;
}


int optDel(arg_data *arg)

{

    if ( local )
    {
        if ( !(arg->flags & ASF_NEGATE) )
        {
            delete=TRUE;
        } else {
            delete=FALSE;
        }
    } else {
        // Add the 'mute' option to the target programs' list
        optOther(arg);
    }
    return TRUE;
}

int optEdit(arg_data *arg)

{

    if ( local )
    {
        if ( !(arg->flags & ASF_NEGATE) )
        {
            edit=TRUE;
        } else {
            edit=FALSE;
        }
    } else {
        // Add the 'mute' option to the target programs' list
        optOther(arg);
    }
    return TRUE;
}

void skipws(void)

{
    while ((*apnt != 0) && isspace(*apnt))
        ++apnt;
}
