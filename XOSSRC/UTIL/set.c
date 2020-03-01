//--------------------------------------------------------------------------*
// SET.C
// DOS-compatable command to set or display environment strings
//
// Written by: Tom Goltz
//
// Edit History:
// 08/20/92(brn) - Add comment header
// 10/09/92(brn) - Fix Path handling to be DOS syntax compatable
// 10/15/92(brn) - Add help_print and real help screen
// 05/12/94(brn) - Fix command abbreviations and version number for 32 bit
// 04/??/95(sao) - Added progasst package
// 05/14/94(sao) - Changed example
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
#include <XOSERRMSG.H>
#include <XOSSTR.H>

#define VERSION 3
#define EDITNO  3

char *defpnt;			/* Pointer to definition string	*/
char *namepnt;			/* Pointer to name string	*/
char  haveeq;			/* TRUE if = specified		*/

void help_print(char *help_string, int state, int newline);
void display(char *);
int  namehave(arg_data *);
void helphave(void);

#define AF(func) (int (*)(arg_data *))func

arg_spec keywords[] =
{   {"*",  ASF_LSVAL, NULL, namehave, 0, NULL},
    {NULL, 0        , NULL, NULL    , 0, NULL}
};

char  prgname[] = "SET";
char  copymsg[] = "";
Prog_Info pib;
char  example[] = "{/option} {envname={text}}";
char  description[] = "This command is provided for compatibility with " \
    "DOS.  It does not conform to the standard XOS command syntax.  " \
    "It is fully compatible with the DOS SET command.  SETENV is the " \
    "equivalent preferred XOS command.  If no arguments are " \
    "specified, all environment variables are listed.  If an argument " \
    "is specified, it must be an environment variable name and must be " \
    "followed by an equal sign.  There must be no space between the " \
    "environment variable name and the equal sign.  If there is, the " \
    "trailing space(s) become part of the name.";

arg_spec options[] =
{
    {"H*ELP", 0, NULL, AF(opthelp), 0, "This message."},
    {"?",     0, NULL, AF(opthelp), 0, "This message."},
    {NULL,    0, NULL, AF(NULL)    , 0, NULL}
};

int     majedt = 3;		/* major edit number */
int     minedt = 0;		/* minor edit number */

main(argc, argv)
int   argc;
char *argv[];
{
    char buffer[512];
    char  *ptr, *dst;
    long   rtn;

	reg_pib(&pib);

	init_Vars();

    if (argc > 1)
    {
	++argv;
    progarg(argv, PAF_EATQUOTE, options, keywords, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    if (!haveeq)			/* Have = ? */
    {
	if (namepnt == 0)
	    display("*");		/* No - display something */
	else
	    fputs("Syntax error\n", stderr);
	exit(EXIT_NORM);
    }
    if (strcmp(namepnt, "PATH") == 0)	/* Defining a new search path? */
    {
	dst = buffer;
	*buffer = 0;
	ptr = defpnt;

	if (strncmp(ptr, "Z:,", 3) != 0)
	    dst = strmov(buffer, "Z:,");

	while (*ptr)
	{
	    if (*ptr == ';')		/* End of a device name? */
	    {
		if (isalnum(*(ptr-1)))
		    *dst++ = '\\';	/* Add \ to the last path name */

		ptr++;		/* Skip the ; */
		*dst++ = ',';	/* replace with a , */
	    }
	    else if (*ptr == ',')
	    {
		if (isalnum(*(ptr-1)))
		    *dst++ = '\\';	/* Add \ to the last path name */
		*dst++ = *ptr++;	/* Just copy the char */
	    }
	    else
	    {
		*dst++ = *ptr++;	/* Just copy the char */
	    }
	}
	if (isalnum(*(ptr-1)))
	    *dst++ = '\\';	/* Add \ to the path name */
	*dst   = '\0';
    if ((rtn = svcIoDefLog(0, 0, "CMD:", buffer)) < 0)
 	    femsg(prgname, rtn, "CMD:");
    }
    else if (svcSysDefEnv(FES_SESSION, namepnt, defpnt) < 0 &&
		defpnt == NULL)
    {
	fputs("Syntax error\n", stderr);
	exit(EXIT_INVSWT);
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
    if (namepnt)
    {
        fputs("Syntax error\n", stderr);
        exit(EXIT_INVSWT);
    }
    haveeq = ((arg->flags & ADF_NONE) == 0);
    namepnt = arg->name;
    strupper(namepnt);		/* Convert the name to upper case */
    defpnt = arg->val.s;
    return (TRUE);
}


/*
 * Function to display environment strings
 */

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
    char   prt_buffer[512];
    char   found[132];
    char  *ptr, *dst;

    if ((rtn = svcIoFindLog(0, "CMD:", found, buffer, 512,
            NULL)) >= 0)
    {
        rtn = 0;

	rtn &= 0xFFFF;
	ptr = buffer;

	if (strncmp(ptr, "Z:,", 3) == 0)
	    ptr += 3;			/* Skip the DSK name	*/
	fputs("PATH=", stdout);

	dst = prt_buffer;
	while (*ptr)
	{
	    if (*ptr == '\\' || *ptr == ':')	/* End of a device name? */
	    {
		if (*(ptr+1) == ',')
		{
		    if (*ptr == '\\')
			ptr++;		/* Skip the trailing \ */
		    else
			*dst++ = *ptr++;

		    if (*ptr == '\0')	/* If null don't add ; */
			continue;

		    ptr++;		/* Skip the , */
		    *dst++ = ';';	/* replace with a ; */
		}
		else
		{
		    *dst++ = *ptr++;
		    continue;
		}
	    }
	    else
	    {
		*dst++ = *ptr++;	/* Just copy the char */
	    }
	}
	*dst   = '\0';
	fputs(prt_buffer, stdout);
	fputc('\n', stdout);
    }

    cnt = 0;
    while ((rtn=svcSysFindEnv(FES_SESSION, name, found, buffer, 512,
            &cnt)) >= 0)
        printf("%s=%s\n", found, buffer); /* Display name and definition */
    exit(EXIT_NORM);
}

/*
 * help_print
 * Print help option entries
 */
void help_print(char *help_string, int state, int newline)
{
    char str_buf[132];

    strcpy(str_buf, help_string);
    if (state)
        strcat(str_buf, " *");

    if (newline)
    {
        fprintf(stderr, "%s\n", str_buf);
    }
    else
        fprintf(stderr, "%-38s", str_buf);

}
