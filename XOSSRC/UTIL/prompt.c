//--------------------------------------------------------------------------*
// Prompt.c
// DOS-compatable command to set prompt
//
// Written by: John R. Goltz
//
// Edit History:
// 08/20/92(brn) - Add comment box
// 13Dec94 (fpj) - Fixed code so that it works.
// 04/11/95(sao) - Added progasst package
// 05/16/95(sao) - Changed exit codes to reflect XOSRTN
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
//-------------------------------------------------------------------------*

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

// Command format:
//      PROMPT
//          Reset prompt string to default (i.e., "C>")
//  or
//      PROMPT string
//          Sets string as prompt

#include <stdio.h>
#include <string.h>

#include <XOS.H>
#include <XOSSVC.H>
#include <XOSRTN.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <XOSERRMSG.H>

#define VERSION 3
#define EDITNO  3

int nonOpt(char *);

SubOpts embed[] =
{
    {"$ ", "Dollar Sign"},
    {"_ ", "Carriage return/line feed"},
    {"b ", "vertical bar"},
    {"d ", "Current date"},
    {"e ", "Esc Character"},
    {"g ", "Greater than (>)"},
    {"h ", "Backspace"},
    {"l ", "Less than (<)"},
    {"n ", "Default drive"},
    {"q ", "Equal sign (=)"},
    {"t ", "Current time"},
    {"p ", "Current path"},
    {NULL,NULL}
};

arg_spec options[] =
{
    {"H*ELP"  , 0, NULL, AF(opthelp), 0, "This message."},
    {"?"      , 0, NULL, AF(opthelp), 0, "This message."},
    {"       ", 0, NULL, AF(NULL), 0, " "},
    {"      " , ASF_XSVAL, &embed, AF(NULL), 0, "The following special meaning characters may be embedded in the prompt string when preceded by an $."},
    {NULL     , 0, NULL, AF(NULL)    , 0, NULL}
};


Prog_Info pib;
char  copymsg[] = "";
char  example[] = "prompt_string";
char  description[] = "This command is provided for DOS compatibility.  " \
    "It is recommended that the SETENV command be used for the PROMPT " \
    "environment variable.  The command prompt can be completely " \
    "customized using the options listed below.  " \
    "All options listed below must be preceded by an $ character (e.g. " \
    "$p).  Standard delimiters (comma, equal sign, semicolon, space or " \
    "tab) must be preceded by a null character.  A null character is an " \
    "$ followed by any character not listed.";


char prgname[] = "PROMPT";
char envname[] = "^XOS^PROMPT";
char newPrompt[512]="";
char gotPrompt=FALSE;

int main(int argc, char *argv[])
{
    long rcode;
    long procflags = (svcSysGetPLevel(0) - 1) | FES_SESSION;

    argc=argc;
	reg_pib(&pib);

	init_Vars();

    argv++;
    progarg(argv, 0, options, NULL, nonOpt,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

    if (!gotPrompt)                       // Was a prompt string specified?
    {
        if ((rcode = svcSysDefEnv(procflags, "PROMPT", NULL)) < 0)
            femsg(prgname, rcode, "PROMPT"); // Reset prompt
    }
    else                                // Define the new prompt
    {
        if ((rcode = svcSysDefEnv(procflags, "PROMPT", newPrompt)) < 0)
            femsg(prgname, rcode, "PROMPT");
    }

    return(EXIT_NORM);
}

int nonOpt(char *inpstr)
{
    gotPrompt=TRUE;
    strcpy(newPrompt,inpstr);
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
};

