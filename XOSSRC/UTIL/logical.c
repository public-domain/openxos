//--------------------------------------------------------------------------*
// Logical.c
// Command to set or display logical names
//
// Written by: John R. Goltz
//
// Edit History:
// 08/20/92(brn) - Add comment block at start of file
// 05/08/94(brn) - Document new options in the help command
// 04/02/95(sao) - added progasst package
// 05/14/95(sao) - chaged description, added mute option
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

// Command format:
//	LOGICAL {-l{evel}=n} {-o{nly}}
//		Displays all logical names
//   or
//	LOGICAL devname: {-l{evel}=n} {-o{nly}}
//		Displays specified logical names, wild cards are valid
//   or
//	LOGICAL devname: = {-l{evel}=n} {-q{uiet}}
//		Deletes a logical name
//   or
//	LOGICAL devname: = definition {-l{evel}=n} {-q{uiet}}
//		Defines a logical name

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSRTN.H>
#include <XOSERR.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>

#define VERSION 3
#define EDITNO  3

long  syslevel;		// 0xC000 if -SYSTEM seen
long  subst;		// 0x10000000 if defining a substituted name
long  setbits;
long  clrbits;
char *defpnt;		// Pointer to definition string
char  havevalue;
char  kwbfr[34];	// Keyword buffer
long  editflag = FALSE; // Not in edit mode
long  renflag  = FALSE; // Not renaming a variable
long  quiet=FALSE;        // TRUE if no output wanted
long  mute=FALSE;         // TRUE if no output wanted
char  haveeq;		// TRUE if = specified

void asserror(char *, char *, long);
void display(char *);
//int  edithave(arg_data *);
void editlogical(char *);
void getname(char *bfr);
int  levelhave(arg_data *);
//int  helphave(void);
//void help_print(char *help_string, int state, int newline);
int  namehave(arg_data *);
int  onlyhave(arg_data *);
//int  quiethave(arg_data *);
void renamelogical(char *, char *);
//int  renhave(arg_data *);
int  sethave(arg_data *);
int  clrhave(arg_data *);

#define AF(func) (int (*)(arg_data *))func

arg_spec keywords[] =
{   {"*",  ASF_LSVAL, NULL, namehave, 0, NULL},
    {NULL, 0        , NULL, NULL    , 0, NULL}
};

arg_spec options[] =
{   {"E*DIT"    , ASF_BOOL|ASF_STORE, NULL, &editflag , TRUE, "Edit the selected logical name." },
    {"RE*NAME"  , ASF_BOOL|ASF_STORE, NULL, &renflag  , TRUE, "Rename the selected logical name."},
    {"SU*BST"   , ASF_NEGATE, NULL,    sethave  , 0x40000000L, "Make a substitute logical name."},
    {"RO*OTED"  , ASF_NEGATE, NULL,    sethave  , 0x20000000L, "Make a rooted logical name."},
    {"SY*STEM"  , 0, NULL, levelhave, 0xC000L, "Operate on a system level logical name."},
    {"SE*SSION" , 0, NULL, levelhave, 0, "Operate on a session level logical name."},
    {"Q*UIET"   , ASF_BOOL|ASF_STORE, NULL, &quiet, TRUE, "Suppress all but error messages."},
    {"M*UTE"    , ASF_BOOL|ASF_STORE, NULL, &mute, TRUE, "Suppress all but error messages."},
    {"H*ELP"    , 0, NULL, AF(opthelp), 0, "This message."},
    {"?"        , 0, NULL, AF(opthelp), 0, "This message."},
    {NULL       , 0, NULL,    NULL     , 0, NULL}
};


Prog_Info pib;
char  copymsg[] = "";
char  prgname[] = "LOGICAL";
char  envname[] = "^XOS^LOGICAL^OPT";
char  example[] = "{/option} name{={definition}}";
char  description[] = "This command provides the means for controlling " \
    "logical device names.  XOS provides a complete system of logical " \
    "names which are used for many system functions.  See chapter 2, page " \
    "14 of the 'XOS Command Reference Manual' for a complete " \
    "discussion of logical names.  If no name is specified, all logical " \
    "names at the indicated level (SESISON or SYSTEM) are displayed.  If " \
    "a logical name is specified without a value (no '=' after the name), " \
    "the current value of the logical name is displayed.  If a logical " \
    "name is specified with a value, the value is changed.  If a null " \
    "value is specified (an '=' after the name but no value), the logical " \
    "name is deleted.  If a logical name is to be defined as a \"search " \
    "list\" logical, the names in the list are seperated with commas.  " \
    "The entire list must be enclosed in double quotes.";

int main(
    int   argc,
    char *argv[])

{
    long  rtn;
    char  strbuf[512];
    char *foo[2];

    argc = argc;

	reg_pib(&pib);

	init_Vars();

    if(svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
    {
	foo[0] = strbuf;
	foo[1] = '\0';
    progarg(foo, 0, options, NULL, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    setbits = clrbits = 0;
    ++argv;
    progarg(argv, PAF_EATQUOTE, options, keywords, (int (*)(char *))NULL,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

	// Added code to resolve conflicts between quiet and mute
	if ( mute == TRUE )
		quiet=TRUE;

    // Here with all arguments processed - now do what we need to do

    if (!haveeq || editflag)		// Have = ?
    {					// No
        if (renflag)
        {
			if ( !mute )
            fputs("? LOGICAL: Two names not specified for rename\n", stderr);
            exit(EXIT_INVSWT);
        }
	if (editflag)			// Do we edit an existing string?
        {
            if (defpnt != NULL)
            {
				if ( !mute )
                fputs("LOGICAL: Warning - new definition ignored\n", stderr);
                defpnt = NULL;
            }
	    editlogical(kwbfr);
        }
        display((kwbfr[0])? kwbfr: "*:"); // No - display something
    }

    if (renflag)			// Rename an existing name?
	renamelogical(kwbfr, defpnt);
    if ((rtn=svcIoDefLog(syslevel, subst, kwbfr, defpnt)) < 0)
        asserror((defpnt==NULL)?"deleting logical name":
                "defining logical name", kwbfr, rtn);
    if (!quiet)
    {
        if (defpnt == NULL)
            printf("Logical name \"%s\" deleted at ", kwbfr);
        else
            printf("Logical name \"%s\" defined as \"%s\" at ", kwbfr, defpnt);
        fputs((syslevel)? "system level\n": "session level\n", stdout);
    }
    return (0);
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
		if ( !mute )
        fputs("? LOGICAL: More than one logical name specified\n", stderr);
        exit(EXIT_INVSWT);
    }
    haveeq = ((arg->flags & ADF_NONE) == 0);
    strcpy(kwbfr, arg->name);
	if ( haveeq )
    	defpnt = arg->val.s;
	else
		defpnt = NULL;
    return (TRUE);
}

// Function to process /SUBST and /ROOTED switches

int sethave(
    arg_data *arg)

{
    if ( arg->flags & ASF_NEGATE )
    {
        clrhave(arg);
    } else {
        subst |= arg->data;
        setbits |= arg->data;
        clrbits &= ~(arg->data);
    }
    return (TRUE);
}

// Function to process /NOSUBST and /NOROOTED switches

int clrhave(
    arg_data *arg)

{
    subst &= !(arg->data);
    clrbits |= arg->data;
    setbits &= ~(arg->data);
    return (TRUE);
}

// Function to process /Session and /System switches


// Function to edit existing logical device names
int  levelhave(arg_data *arg)
{
    if ( strncmp(arg->name,"SE",2)==0 )
        syslevel=0;
    else
        syslevel=0xC000L;
    return 0;
}

void editlogical(
    char *name)

{
    char buffer[512];
    char namebfr[512];
    char device[12];
    long rtn;

    if (name == NULL)
    {
		if ( !mute )
        fputs("? LOGICAL: No name specified\n", stderr);
        exit(EXIT_INVSWT);
    }

    if ((rtn = svcIoFindLog(syslevel, name, device, buffer, 512, NULL)) < 0)
	asserror("searching for name", name, rtn);

//    printf("#### rtn = %08.8X, set = %08.8X, clr = %08.8X\n", rtn, setbits, clrbits);

    subst = ((rtn & 0xFFFF0000) | setbits) & ~clrbits;

    if (renflag)			// Rename variable with edit?
    {
	if ( !quiet )
	fprintf(stdtrm, "LOGICAL: Rename %s to ", name);
    if ((rtn = svcTrmWrtInB(STDTRM, name, strlen(name))) < 0)
	    femsg(prgname, rtn, name);
        getname(namebfr);		// Get revised string
	renamelogical(name, namebfr);
    }

	if ( !quiet )
    fprintf(stdtrm, "LOGICAL: %s = ", name);
    if ((rtn = svcTrmWrtInB(STDTRM, buffer, strlen(buffer))) < 0)
	femsg(prgname, rtn, name);

    getname(buffer);			// Get revised string
    if ((rtn=svcIoDefLog(syslevel, subst, name, buffer)) < 0)
	asserror("defining logical name", name, rtn);
    if (!quiet)
        printf("Logical name \"%s\" defined as \"%s\" at %s level\n", name,
                buffer, (syslevel)? "system": "session");
    exit(EXIT_NORM);
}

// Function to get revised string

void getname(
    char *bfr)

{
    char chr;

    fgets(bfr, 512, stdtrm);		// Get revised string
    while ((chr = *bfr++) != '\0')
    {
        if (chr == '\n')
        {
            bfr[-1] = '\0';
            return;
        }
    }
}

// Function to rename existing logical names

void renamelogical(
    char *oldname,
    char *newname)

{
    long  rtn;
    char  buffer[512];
    char  device[12];

    if (oldname == NULL || newname == NULL)
    {
		if ( !mute )
        fputs("? LOGICAL: Two names not specified for rename\n", stderr);
        exit(EXIT_INVSWT);
    }
    if ((rtn = svcIoFindLog(syslevel, oldname, device, buffer, 512, NULL)) < 0)
	asserror("searching for name", oldname, rtn);
    strupper(newname);
    if ((rtn=svcIoDefLog(syslevel, 0, newname, buffer)) < 0)
	asserror("defining logical name", newname, rtn);
    if ((rtn=svcIoDefLog(syslevel, 0, oldname, NULL)) < 0)
	asserror("deleting logical name", oldname, rtn);
	if ( !quiet )
		printf("Logical %s has been renamed %s\n",oldname,newname);
    exit(EXIT_NORM);
}

// Function to display logical names

void display(
    char *name)

{
    long   rtn;
    long   cnt;
    struct def
    {   struct def *next;
        int    length;
        char   name[2];
    };
    char   buffer[512];
    char   device[12];
    char   headflg;

    cnt = 0;
    headflg = FALSE;
    while ((rtn=svcIoFindLog(syslevel, name, device, buffer, 512,
            &cnt)) >= 0)
    {
        if(!headflg)
        {
			if ( !quiet )
            printf("Matching logical names defined at %s level:\n",
                    (syslevel)? "system": "session");
            headflg = TRUE;
        }
		if ( !quiet )
        printf("%c%c   %s = %s\n", (rtn & 0x40000000L)?'s':'-',
                (rtn & 0x20000000L)?'r':'-', device, buffer);
    }
    if (rtn == ER_NSDEV)
    {
        if (!headflg)
			if ( !quiet )
            printf("? No matching logical names defined at %s level\n",
                    (syslevel)? "system": "session");
    }
    else
        asserror("searching for name", name, rtn);
    exit(EXIT_NORM);
}

// Function to report fatal error

void asserror(
    char *msg,
    char *name,
    long  code)

{
    char buffer[100];

    svcSysErrMsg(code, 3, buffer); // Get error message string
	if ( !mute )
    fprintf(stderr, "? LOGICAL: Error %s %s\n"
                    "           %s\n", msg, name, buffer);
    exit(EXIT_SVCERR);
}
