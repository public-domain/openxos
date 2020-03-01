//------------------------------------------------------------------------------
//
//  batargs.c - Batch command argument list, sets up array pointed to by argv
//              from string pointed to by command.  Returns argument count
//              (argc)
// 
//  Written by Bruce R. Nevins
//
//  Edit History:
//  -------------
//  06/16/92(brn) - Created first version
//  08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
// 
//------------------------------------------------------------------------------

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
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

int batargs(char *args, char **argv[])
{
    char *cp;				/* Pointers for strings */
    union
    {   char  *c;
        char **cc;
    } tempstr;
    char  templine[LINESIZE+2];		/* Temp string of command list */
    char  chr;
    int   argc;				/* Count of arguments returned */

/*
 * Point to beginning of strings
 */
    tempstr.c = templine;
    cp = args;
    *argv = NULL;			/* Assume no block returned */
    argc = 0;				/* Assume no args are returned */

/*
 * Copy the command string without leading space
 */
    /* Remove space or HT at beinning */

    while ((chr = *cp++) == ' ' || chr == '\t')
        ;

    if (chr == '\0')
	return (argc);			/* No args just return */
    else
	argc++;

    *tempstr.c++ = chr;			/* Save first char */

    while ((chr = *cp++) != '\0')
    {
    	if(chr == ' ' || chr == '\t')
	{
	    while (((chr = *cp++) == ' ' || chr == '\t') && chr != '\0')
	        ;			/* Skip extra space */

	    if (chr != '\0')
	    {
	        argc++;			/* Just had an arg, count it */
	        *tempstr.c++ = ' ';	/* Add one space */
	    }
	    else
	    {
		break;			/* Done */
	    }
	}
        *tempstr.c++ = chr;
    }

    *tempstr.c = chr;	            	/* end string */


    if ((*argv = tempstr.cc = (char **)sbrk(strlen(templine) + 1 +
        (argc * sizeof(char *)))) == NULL) /* Make buffer for command */
    {
        fprintf(cmderr, "? Command line lost - out of memory\n");
	if (detached)
	    fflush(cmderr);
	return (0);
    }
    cp = tempstr.c + (argc * sizeof(char *)); /* Point to beginning of strings */
    strcpy(cp, templine);
	
    *tempstr.cc++ = cp;

    while ((chr = *cp++) != '\0')
    {
    	if(chr == ' ')
	{
	    *(cp - 1) = '\0';		/* End string for last ARGV */
	    *tempstr.cc++ = cp;		/* Point to next ARGV */
	}
    }
    return (argc);			/* Give caller number of args */
}
