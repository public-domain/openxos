//------------------------------------------------------------------------------
// CMDALIAS.C - ALIAS command handler for SHELL
//
// Written by Bruce R. Nevins
//
// Edit History:
// -------------
// 03/16/89(brn) - Created first version
// 08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
// 03/09/94(brn) - Finish code
// 04/20/94(brn) - Modify to use batnkeyw and to handle options
// 08/27/96(brn) - Add fflush if detached process
//------------------------------------------------------------------------------*

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

#include <CTYPE.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <XOS.H>
#include <XOSSTR.H>

#include "BATCHEXT.H"
#include "SHELL.H"
#include "SHELLEXT.H"

// Define all our static data
static void delalias(void);

static char cmdname[] = "ALIAS";	// Our option name

static char help_text[] =
	    "   List all:      ALIAS\n"
	    "   List 'name':   ALIAS name\n"
	    "   Delete 'name': ALIAS name=\n"
	    "   ADD 'name':    ALIAS name=command {batch style arguments}\n";

static int quiet, help;

static struct OPTLIST
{
    char *option;	// The option name
    int  *bool;		// The option boolean variable
    int   value;	// The value to set bool
} optlist[] =
{
    {"/?",       &help,  TRUE},
    {"/HELP",    &help,  TRUE},
    {"/HEL",     &help,  TRUE},
    {"/H",       &help,  TRUE},
    {"/NOQ",     &quiet, FALSE},
    {"/NOQUIET", &quiet, FALSE},
    {"/Q",       &quiet, TRUE},
    {"/QUI",     &quiet, TRUE},
    {"/QUIET",   &quiet, TRUE},
    {NULL,       NULL,   NULL}
};

/*
 * Function to process ALIAS command
 */

int cmdalias(void)

{
    register j;
    register char  chr;
    struct firstblk *aliasnew = NULL;
                                // Pointer to command alias list beginning
    char acmd[LINESIZE];
    char temp[LINESIZE];

    quiet = help = FALSE;		// Default of NOQUIET and NOHELP
    blkinic(argbase);			// Point to beginning of string

// Skip past ALIAS command
    batnkeyw(acmd, argbase);		// Skip past the command

// Get the first argument or option
    if (!batnkeyw(acmd, argbase))
    {
        if (aliasfrst != NULL)		// List the current alias's
        {
            fprintf(cmdout, "%% %s: Command Alias List: (%d) defined\n",
				cmdname, aliascnt);
            aliastmp = aliasfrst;

            for(j = aliascnt; j > 0; j--)
            {
		fputs("  ", cmdout);
                blkinic(aliastmp);        // Start from beginning
                blkputs(aliastmp, cmdout);
                fputc('\n', cmdout);
                if ((aliastmp = aliastmp->a.nextlst) == NULL)
                    break;
            }
	    errrtn = 0;			// Command alias's are defiined
	    if (detached)
		fflush(cmdout);
        }
	else
	{
	    if (!quiet)
        	fprintf(cmderr, "%% %: No Command Alias' defined\n", cmdname);
	    errrtn = 1;			// No command alias's are defined
	    if (detached)
		fflush(cmderr);
	}
        return (TRUE);
    }

    while (TRUE)
    {
	if (acmd[0] == '/')
	{
	    j = 0;
	    while (optlist[j].option != NULL)
	    {
		if (stricmp(acmd, optlist[j].option) == 0)
		    break;
	    	j++;
	    }

	    if (optlist[j].option == NULL)
	    {
		fprintf(cmderr, "? %s: Illegal option \"%s\"\n", cmdname,
				acmd);
		if (detached)
		    fflush(cmderr);
	    }
	    else
	    {
		*optlist[j].bool = optlist[j].value;
	    }

	    if (help)
	    {
	        fprintf(cmderr, "%% %s:Command syntax\n%s",
			    cmdname, help_text);
		if (detached)
		    fflush(cmderr);
		help = FALSE;
	    }

	    if (!batnkeyw(acmd, argbase))
	        break;
	}

// Get the ALIASed command
	j = 0;
	while (((chr = acmd[j++]) != '\0') && !isspace(chr))
	{
	    aliasnew = blkputc(aliasnew, chr);
	}
	blkputc(aliasnew, '=');

	if (batnkeyw(temp, argbase))
	    chr = temp[0];
	else
	    chr = '\0';

// Find the command in the list, Need to search with = to get the whole
//  command.

	aliastmp = aliasfrst;
	j = strlen(acmd);
	while (aliastmp != NULL)
	{
	    blkinic(aliastmp);
	    blkinic(aliasnew);
	    if (blkstrnicmp(aliastmp, aliasnew, j + 1) == 0)
	    {
		break;
	    }
	    aliastmp = aliastmp->a.nextlst;
	}

// Must be a = or a NULL

	if (chr == '\0')		// List the current ALIAS value
	{
	    if (aliastmp == NULL)
	    {
		if (!quiet)
		{
		    fprintf(cmderr, "? %s: (%s), not defined\n",
					cmdname, acmd);
		    if (detached)
			fflush(cmderr);
		}
		aliasnew = givechain(aliasnew); // Give up the search string
		errrtn = 1;		// No command alias's are defined
		return (TRUE);
	    }
	    errrtn = 0;			// Command alias's are defined
	    if (!quiet)
	    {
		blkinic(aliastmp);
		blkputs(aliastmp, cmdout);
		fputc('\n', cmdout);
		if (detached)
		    fflush(cmdout);
	    }
	    aliasnew = givechain(aliasnew); // Give up the search string
	    return (TRUE);
	}
	else if (chr == '=')		// Define new ALIAS value
	{
	    do
	    {
		chr = blkgetc(argbase);	// Get the first real character
	    } while ((chr != '\0') && isspace(chr));

	    if (chr == '\0')
	    {
		if (aliastmp == NULL)
		{
		    if (!quiet)
		    {
			fprintf(cmderr, "? %s: Cannot delete (%s), not defined\n",
					cmdname, acmd);
			if (detached)
			    fflush(cmderr);
		    }
		    errrtn = 1;		// No command alias's are defined
		}
		else
		{
		    fprintf(cmderr, "? %s: (%s), Deleted\n",
					cmdname, acmd);
		    if (detached)
			fflush(cmderr);
		    delalias();
		    errrtn = 0;		// Command alias's are defiined
		}
		aliasnew = givechain(aliasnew); // Give up the search string
		return (TRUE);
	    }

	    if (aliastmp != NULL)	// If the definition exist remove it
		delalias();

	    blkinic(aliasnew);
	    for (j = 0; j < (strlen(acmd)+1); j++)
	    {
		blkgetc(aliasnew);
	    }
	    blkputc(aliasnew, chr);	// Save first char
	    blkcat(aliasnew, argbase);	// Append the definition

// Place into alias list

            if (aliaslast == NULL)
            {
        	aliasfrst = aliasnew;
        	aliaslast = aliasnew;
        	aliastmp = aliasnew;
        	aliasnew->a.lastlst = NULL;
        	aliasnew->a.nextlst = NULL;
        	aliascnt = 1;		// Initialize the alias count
            }
            else
            {
        	aliastmp = aliasnew;
	        aliastmp->a.lastlst = aliaslast;
		aliaslast->a.nextlst = aliastmp;
        	aliaslast = aliastmp;	// New last pointer

	        aliaslast->a.nextlst = NULL;// No next pointer
		aliascnt++;
            }
	}
	else
	{
	    if (!quiet)
	    {
		fprintf(cmderr, "? %s: syntax error\n%s",
			    cmdname, help_text);
		if (detached)
		    fflush(cmderr);
	    }
	    aliasnew = givechain(aliasnew); // Give up the search string
	    return(TRUE);
	}
	break;
    }

    return (TRUE);
}

// Remove the alias pointed to by aliastmp from the
//  alias list

static void delalias(void)
{
// Unlink from packet in front of us

    if (aliastmp->a.lastlst != NULL)
	aliastmp->a.lastlst->a.nextlst = aliastmp->a.nextlst;
    else
	aliasfrst = aliastmp->a.nextlst;

// Unlink from packet after us

    if (aliastmp->a.nextlst != NULL)
	aliastmp->a.nextlst->a.lastlst = aliastmp->a.lastlst;
    else
	aliaslast = aliastmp->a.lastlst;

    aliastmp = givechain(aliastmp);
    aliascnt--;				// Drop the count of ALIAS'
}
