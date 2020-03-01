//--------------------------------------------------------------------------*
// SETENV.C
// Command to set or display environment strings
//
// Written by: John R. Goltz
//
// Edit History:
// 08/20/92(brn) - Added comment box
// 05/12/94(brn) - Fixed spelling of options
// 04/03/95(sao) - Added progasst package
// 04/14/95(sao) - Fixed quiet option (broke in prev. release)
// 04/15/95(sao) - Fixed quiet&delete combo (nulled defpnt in gotname)
// 05/14/95(sao) - Changed example, added mute
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

/* Command format:
	SETENV {PID=n.n} {SL=n} {PL=n} {SYS}
		Displays all environment strings
     or
	SETENV environmentname {PID=n.n} {SL=n} {PL=n} {SYS}
		Displays specified environment strings, wild cards are valid
     or
	SETENV environmentname = {PID=n.n} {SL=n} {PL=n} {SYS}
		Deletes an environment string
     or
	SETENV environmentname = definition {PID=n.n} {SL=n} {PL=n} {SYS}
		Defines an environment string
 */

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSRTN.H>
#include <XOSERR.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>

#define VERSION 3
#define EDITNO  6

#define uchar  unsigned char
#define ushort unsigned short
#define uint   unsigned int
#define ulong  unsigned long

char *defpnt="";           /* Pointer to definition string */
char  havevalue;
long   editflag = FALSE;     /* TRUE if in edit mode */
long   renflag  = FALSE;     /* TRUE if renaming a variable */
char  kwbfr[34];		/* Keyword buffer */
union
{   long val;
    struct
    {   short num;
        short seq;
    } pid;
} level;			/* Level for definition */

long  quiet=FALSE;            /* TRUE if no output wanted */
long  mute=FALSE;            /* TRUE if no output wanted */
char  havelevel;		/* TRUE if level specified */
char  haveeq;			/* TRUE if = specified */


void asserror(char *, char *, long);
void badvalue(char *sw);
void chkconflict(void);
void display(char *);
void displevel(char *);
void editenv(char *);
void getname(char *bfr);
int  getpid(char **pnta, short *vala);
int  namehave(arg_data *);
int  pidhave(arg_data *);
int  plhave(arg_data *);
void renameenv(char *, char *);
int  slhave(arg_data *);
int  syshave(void);

#define AF(func) (int (*)(arg_data *))func

arg_spec keywords[] =
{   {"*",  ASF_LSVAL, NULL, namehave, 0, NULL},
    {NULL, 0        , NULL, NULL    , 0, NULL}
};

arg_spec options[] =
{
    {"PID"     , ASF_VALREQ|ASF_LSVAL, NULL, AF(pidhave)  , 0, "Modify the environment for this process ID."},
    {"SL"      , ASF_VALREQ|ASF_NVAL , NULL, AF(slhave)   , 0, "Level relative to session for enviroment variable"},
    {"PL"      , ASF_VALREQ|ASF_NVAL , NULL, AF(plhave)   , 0, "Level relative to process for enviroment variable"},
    {"SYS"     , 0                   , NULL, AF(syshave)  , 0, "Modify the system environment list."},
    {"E*DIT"   , ASF_BOOL|ASF_STORE  , NULL, &editflag , 0, "Edit the value of the specified environment variable."},
    {"R*ENAME" , ASF_BOOL|ASF_STORE  , NULL, &renflag  , 0, "Rename the specified environment variable."},
    {"Q*UIET"  , ASF_BOOL|ASF_STORE  , NULL, &quiet, TRUE, "Suppress all but error messages."},
    {"M*UTE"   , ASF_BOOL|ASF_STORE  , NULL, &mute, TRUE, "Suppress all messages."},
    {"H*ELP"   , 0                   , NULL, AF(opthelp) , 0, "This message."},
    {"?"       , 0                   , NULL, AF(opthelp) , 0, "This message."},
    {NULL      , 0                   , NULL, AF(NULL)     , 0, NULL}
};

Prog_Info pib;
char  copymsg[] = "";
char  prgname[] = "SETENV";
char  envname[] = "^XOS^SETENV^OPT";
char  example[] = "{/options} {envname {={text}}}";
char  description[] = "This function manipulatees the XOS environment " \
    "variables.  Functionally, this command is equivalent to the DOS " \
    "compatible SET command but it conforms to the standard XOS " \
    "command syntax and provides additional features.";

main(argc, argv)
int   argc;
char *argv[];

{
    long   rtn;
    char   strbuf[512];
    char  *foo[2];

    argc = argc;

    reg_pib(&pib);

	init_Vars();

    if((rtn=svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL)) > 0)
    {
	foo[0] = &strbuf[(short)rtn+1];
	foo[1] = '\0';
    progarg(foo, 0, options, keywords, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    ++argv;
    progarg(argv, PAF_EATQUOTE, options, keywords, (int (*)(char *))NULL,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

    /* Here with all arguments processed - now do what we need to do */

    if (level.val == 0)
        level.val = FES_SESSION;

    if (!haveeq || editflag)		/* Have = ? */
    {
	if (editflag)			/* No - do we edit an existing string? */
        {
            if (defpnt != NULL)
            {
                if (!mute) fputs("? SETENV: Warning - new definition ignored\n", stderr);
                defpnt = NULL;
            }
	    editenv(kwbfr);
        }
        display((kwbfr[0])? kwbfr: "*"); /* No - display something */
    }

    if (renflag)		/* Rename an existing variable?	*/
	renameenv(kwbfr, defpnt);

    if ((rtn=svcSysDefEnv(level.val, kwbfr, defpnt)) < 0)
    {
        asserror((defpnt==NULL)?"deleting environment string":
            "defining environment string", kwbfr, rtn);
    }

    if (!quiet && !mute)
    {
	if (defpnt == NULL)
	    printf("String \"%s\" deleted", kwbfr);
	else
	    printf("String \"%s\" defined as \"%s\"", kwbfr, defpnt);
        displevel("\n");
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


int namehave(
    arg_data *arg)

{

    if (kwbfr[0])
    {
        if (!mute) fputs("? SETENV: More than one environment string specified\n", stderr);
        exit(EXIT_INVSWT);
    }
    haveeq = ((arg->flags & ADF_NONE) == 0);
    strcpy(kwbfr, arg->name);
    if ( (arg->flags & ADF_NULL) == 0 )
        defpnt = arg->val.s;
    else
        defpnt = NULL;
    return (TRUE);
}

/*
 * Function to process /PL switch
 */

int plhave(
    arg_data *arg)

{
    chkconflict();
    level.val = 0x4000 | arg->val.n;
    return (TRUE);
}

/*
 * Function to process /SL switch
 */

int slhave(
    arg_data *arg)

{
    chkconflict();
    level.val = 0x8000 | arg->val.n;
    return (TRUE);
}

/*
 * Function to process /PID switch
 */

int pidhave(
    arg_data *arg)

{
    char *pnt;

    chkconflict();

    //printf("### %s\n", arg->val.s);

    pnt = arg->val.s;
    if (getpid(&pnt, &level.pid.seq) != '.' ||
            getpid(&pnt, &level.pid.num) != '\0' || level.val == 0 ||
            (level.pid.num & 0xFF00) != 0)
    {
        if (!mute) fputs("? SETENV: Illegal process ID specified for /PID\n", stderr);
        exit(EXIT_INVSWT);
    }
    return (TRUE);
}

int getpid(
    char **pnta,
    short *vala)

{
    char *pnt;
    long  val;
    char  chr;

    val = 0;				/* Initialize value */
    pnt = *pnta;			/* Copy his pointer */
    while (isdigit(chr = *pnt++))	/* Loop while have digits */
    {
        val *= 10;			/* Add in next digit */
        val += (chr & 0xF);
        if ((val & 0xFFFF0000L) != 0)	/* Is value too big? */
        {
            chr = -1;			/* Yes - return illegal stopper */
            break;
        }
    }
    *vala = (short)val;			/* Give him the value */
    *pnta = pnt;			/* Update his pointer */
    return (chr);			/* Return the stopper character */
}

/*
 * Function to process /SYS switch
 */

int syshave(void)

{
    chkconflict();
    level.val = 0xC000;
    return (TRUE);
}

/*
 * Function to check for level conflicts
 */

void chkconflict(void)

{
    if (level.val != 0)
    {
        if (!mute) fputs("SETENV: More than one level specified\n", stderr);
        exit(EXIT_INVSWT);
    }
}

/*
 * Function to edit existing enviroment strings
 */

void editenv(name)
char *name;
{
    char  found[132];
    char  buffer[512];
    char  namebfr[512];
    long  rtn;
    char *ptr;

    if (name == NULL)
    {
        if (!mute) fputs("? SETENV: No name specified\n", stderr);
        exit(EXIT_INVSWT);
    }
    if ((rtn = svcSysFindEnv(level.val, name, found, buffer, 512, NULL)) < 0)
	asserror("searching for name", name, rtn);
    ptr = &buffer[(int)rtn + 1];
    if (renflag)			/* Rename variable with edit?	*/
    {
		if (!mute && !quiet) fprintf(stdtrm, "SETENV: Rename %s to ", name);
	    if ((rtn = svcTrmWrtInB(STDTRM, name, strlen(name))) < 0)
		    femsg(prgname, rtn, name);
		getname(namebfr);		/* Get revised string	*/
		renameenv(name, namebfr);
    }
    fprintf(stdtrm, "SETENV: %s = ", name);
    if ((rtn = svcTrmWrtInB(STDTRM, ptr, strlen(ptr))) < 0)
	femsg(prgname, rtn, name);
    getname(buffer);			/* Get revised string	*/
    if ((rtn=svcSysDefEnv(level.val, name, buffer)) < 0)
	asserror("defining environment string", name, rtn);
    exit(EXIT_NORM);
}

/*
 * Function to get revised string
 */

void getname(
    char *bfr)

{
    char chr;

    fgets(bfr, 512, stdtrm);		/* Get revised string */
    while ((chr = *bfr++) != '\0')
    {
        if (chr == '\n')
        {
            bfr[-1] = '\0';
            return;
        }
    }
}


/*
 * Function to rename existing enviroment variable
 */

void renameenv(oldname, newname)
char *oldname;
char *newname;
{
    long  rtn;
    char  found[132];
    char  buffer[512];
    char *ptr;

    if (oldname == NULL || newname == NULL)
    {
        if (!mute) fputs("? SETENV: Two names not specified for rename\n", stderr);
        exit(EXIT_INVSWT);
    }
    if ((rtn = svcSysFindEnv(level.val, oldname, found, buffer, 512, NULL)) < 0)
	asserror("searching for name", oldname, rtn);
    ptr = &buffer[(int)rtn + 1];
    strupper(newname);
    if ((rtn=svcSysDefEnv(level.val, newname, ptr)) < 0)
	asserror("defining environment string", newname, rtn);
    if ((rtn=svcSysDefEnv(level.val, oldname, NULL)) < 0)
	asserror("deleting environment string", oldname, rtn);
    exit(EXIT_NORM);
}

/*
 * Function to display environment strings
 */

void display(name)
char *name;
{
    long   rtn;
    long   cnt;
    struct def
    {   struct def *next;
        int    length;
        char   name[2];
    };
    char   heading;
    char   found[132];
    char   buffer[512];

    heading = TRUE;
    cnt = 0;
    while ((rtn=svcSysFindEnv(level.val, name, found, buffer, 512, &cnt)) >= 0)
    {
        if (heading)
        {
            heading = FALSE;
            if (!mute && !quiet) fputs("Matching environment strings defined", stdout);
            displevel(":\n");
        }
        printf("    %s = %s\n", found, buffer);
    }
    if (rtn == ER_NTDEF)
    {
        if (cnt == 0)
        {
            if (!mute && !quiet) fputs("No matching environment strings defined", stdout);
            displevel("\n");
        }
    }
    else
        asserror("searching for name", name, rtn);
    exit(EXIT_NORM);
}

void displevel(
    char *fin)

{
    int plvl;

    if ((level.val & 0xC000) == 0)
        if (!mute && !quiet) printf(" for process %u.%d", level.pid.seq, level.pid.num);
    else if (level.val == 0xC000)
        if (!mute && !quiet) fputs(" at system level", stdout);
    else
    {
        if (!mute && !quiet) printf(" at %s level", (level.pid.num & 0x8000)? "session": "process");
        if ((plvl = level.pid.num & 0xFF) != 0)
            if (!mute && !quiet) printf(" %s %d", (level.pid.num & 0x8000)? "plus": "minus", plvl);
    }
    if (!mute && !quiet) fputs(fin, stdout);
}

/*
 * Function to report bad argument value
 */

void badvalue(sw)
char *sw;

{
    if (!mute) fprintf(stderr, "? SETENV: Illegal value for switch %s\n", sw);
    exit(EXIT_INVSWT);
}

/*
 * Function to report fatal error
 */

void asserror(msg, name, code)
char *msg;
char *name;
long  code;

{
    char buffer[100];

    svcSysErrMsg(code, 3, buffer); /* Get error message string */
    if (!mute) fprintf(stderr, "? SETENV: Error %s %s\n"
                    "          %s\n", msg, name, buffer);
    exit(EXIT_SVCERR);
}
