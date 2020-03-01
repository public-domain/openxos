//------------------------------------------------------------------------------
// batopt.c - Extended batch routine for BATCH
//
// Written by Bruce R. Nevins
//
// Edit History:
// -------------
// 05/13/91(brn) - Created first version
// 08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
// 02/21/94(brn) - Add control C prompt argument
// 03/23/94(brn) - Changed name to batopt from batxosba and change keyowrd
//                 handling.
// 03/28/94(brn) - Add QUIET and NOQUIET options and set errrtn to value
//                 of bool when displaying value of argument (even if
//                 QUIET is set)
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
#include <STRING.H>
#include <XOS.H>
#include <XOSSTR.h>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

static char cmdname[] = "BATOPT";	// Our option name

static struct ARGLIST
{
    char *argument;	// The argument name
    char *desc;		// The argument description
    int  *bool;		// The argument boolean variable
} arglist[] =
{
    {"EXTEND", "(Extended Batch)           ", &extend_batch},
    {"CCP", "   (Batch Control C prompting)", &ctrlc_prompt},
    {"ECHO", "  (Batch command echoing)    ", &echo},
    {"ERRP", "  (Pause Batch on error)     ", &error_pause},
    {NULL, NULL, NULL}
};

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

// Function to process BATOPT command

int batopt(void)

{
    char tempbuf[LINESIZE+2];
    int temp;

    quiet = FALSE;			// Always assume not QUIET
    help  = FALSE;			// See if he wants help

    blkinic(expndarg);                  // Start from beginning of string
    batnkeyw(tempbuf, expndarg);        // Skip past the command

// Get the first argument or option
    if (!batnkeyw(tempbuf, expndarg))
    {
	temp = 0;
	while (arglist[temp].argument != NULL)
	{
	    fprintf(cmdout, " %s %s = %s\n", arglist[temp].argument,
		    arglist[temp].desc, *arglist[temp].bool ? "ON" : "OFF");
	    temp++;
	}
	if (detached)
	    fflush(cmdout);
        return (TRUE);
    }

    while (TRUE)
    {
	if (tempbuf[0] == '/')
	{
	    temp = 0;
	    while (optlist[temp].option != NULL)
	    {
		if (stricmp(tempbuf, optlist[temp].option) == 0)
		    break;
	    	temp++;
	    }

	    if (optlist[temp].option == NULL)
	    {
		fprintf(cmderr, "? %s: Illegal option \"%s\"\n", cmdname, tempbuf);
		if (detached)
		    fflush(cmderr);
	    }
	    else
	    {
		*optlist[temp].bool = optlist[temp].value;
	    }

	    if (help)
	    {
		printf("%% %s: {/{?}{HELP}{{NO}QUIET}} {{CCP|ECHO|EXTEND}{={ON|OFF}}\n",
				cmdname);
		if (detached)
		    fflush(stdout);
		help = FALSE;
	    }

	    if (!batnkeyw(tempbuf, expndarg))
	        break;
	}

	temp = 0;
	while (arglist[temp].argument != NULL)
	{
	    if (stricmp(tempbuf, arglist[temp].argument) == 0)
		break;
	    temp++;
	}

	if (arglist[temp].argument == NULL)
	{
	    fprintf(cmderr, "? %s: Illegal argument \"%s\"\n", cmdname,
				tempbuf);
	    if (detached)
		fflush(cmderr);
	    if (!batnkeyw(tempbuf, expndarg))
		break;
	    continue;
	}
	if (!batnkeyw(tempbuf, expndarg))	// If no keyword just display state
	{
	    if (!quiet)
	    {
		fprintf(cmdout, " %s (%s) = %s\n", arglist[temp].argument,
		    arglist[temp].desc, *arglist[temp].bool ? "ON" : "OFF");
		if (detached)
		    fflush(cmdout);
	    }
	    errrtn = *arglist[temp].bool;	// Set error level to bool
	    return (TRUE);
	}
	if (strcmp("=", tempbuf) == 0)
	{
	    if (!batnkeyw(tempbuf, expndarg))
	    {
	        fprintf(cmderr, "? %s: %s Argument value not specified\n",
			cmdname, arglist[temp].argument);
		if (detached)
		    fflush(cmderr);
	        return (TRUE);
	    }
	    if (stricmp("ON", tempbuf) == 0)
	        *arglist[temp].bool = TRUE;
	    else if (stricmp("OFF", tempbuf) == 0)
	        *arglist[temp].bool = FALSE;
	    else
	    {
	        fprintf(cmderr, "? %s: %s Argument \"%s\" invalid\n",
			cmdname, arglist[temp].argument, tempbuf);
		if (detached)
		    fflush(cmderr);
		if (!batnkeyw(tempbuf, expndarg))
		    break;
	        continue;
	    }

	}
	else
	{
	    if (!quiet)
	    {
		fprintf(cmderr, " %s (%s) = %s\n", arglist[temp].argument,
		    arglist[temp].desc, *arglist[temp].bool ? "ON" : "OFF");
		if (detached)
		    fflush(cmderr);
	    }
	    errrtn = *arglist[temp].bool;	// Set error level to bool
	    continue;
	}
	if (!batnkeyw(tempbuf, expndarg))
	    break;
    }

    return (TRUE);
}
