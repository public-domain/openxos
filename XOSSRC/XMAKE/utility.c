//****************************************
// UTILITY.C - Utility functions for XMAKE
//****************************************

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
#include <XCSTRING.H>
#include <MALLOC.H>
#include <XCMALLOC.H>
#include <CTYPE.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERRMSG.H>
#include <XOSTIME.H>
#include "XMAKE.H"

static char dummy[2] = ".";	// Dummy extension for default rule search

static char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
              		 "Aug", "Sep", "Oct", "Nov", "Dec"};

static int last_prt = FALSE;	// Flag for print option's display of fmacros

extern char oldchr1;		// Place for getatom to return chars
extern char oldchr2;		// Second place for getatom to return chr

void prtprint(
	unsigned char chr)

{
    if (chr != (unsigned char)EOF)	// Don't print the EOF character
    {
		if (chr >= 0x80 && !last_prt)
		{
			printf("$(%c", chr & 0x7f);
			last_prt = TRUE;
		}
		else if (chr >= 0x80 && last_prt)
		{
			printf(":%c)", chr & 0x7f);
			last_prt = FALSE;
		}
		else
			putchar(chr);
    }
}


void cmdprint(
    unsigned char *string)

{
    putchar('\t');
    while (*string != 0)
    {
		if (*string > 0x80)				// If beginning of special macro
		{
			printf("$(%c", *string++ & 0x7f); // Show macro type
			while (*string < 0x80)		// Print macro options
				putchar(*string++);
			printf(":%c)", *string++ & 0x7f); // Print final part of macro
		}
		else
			putchar(*string++);			// Otherwise just display the chr
    }
    putchar('\n');
}


void showerror(void)

{
    int x = 1;

    fputs(linebuffer, stderr);			// First, show the line
    if (oldchr1 != 0)
		++x;
    if (oldchr2 != 0)
		++x;
    spaces(stderr, (int)(linepntr - linebuffer) - x);
    fputs("^\n", stderr);
}

void syntax(void)
{
    fprintf(stderr, "? XMAKE: %s @%d, Syntax Error\n", makex, mline);
    showerror();
    exit(1);
}


//*****************************************
// Function: getmemory - Get a memory block
// Returned: Address of block allocated
//*****************************************

char *getmemory(
    ulong size)

{
    char *temp;

    if ((temp = (char *)sbrk(size)) == (char *)(-1))
    {
		fputs("? XMAKE: Unable to allocate memory\n", stderr);
		exit(-2);
    }
    return (temp);
}


//******************************************
// Function: getmore - Extend a memory block
// Returend: Address of block
//******************************************

char *getmore(
    ulong size,
    char *place)

{
    char *temp;

    place = place;

    if ((temp = (char *)sbrk(size)) == (char *)(-1))
    {
		fputs("? XMAKE: Unable to allocate memory\n", stderr);
		exit(-2);
    }
///	if (place + 1 != temp)
///	{
///		fputs("? XMAKE: System Error - Non contigous memory allocation\n",
///				stderr);				// this happens in getcmd()
///		exit(-2);
///	}

    return (temp);
}


int inchr(
    char *string,
    char  chr)

{
    char c;

    while ((c = *string++) != 0 && c != chr) // Until end of string or success
		;
    if (c == 0)							// If not found
		return (FALSE);					// Indicate we failed
    return (TRUE);						// Otherwise the character was found
}										//   in string


int instr(
    char *string1,
    char *string2)

{
    char *s1, *s2;

    s2 = string2;
    while(*string1 != 0)
    {
		s1 = string1;
		while (toupper(*s1) == toupper(*s2) && *s1 != 0)
		{
			++s1;
			++s2;
		}
		if ((*s1 == 0 && *s2 == 0) || *s2 == 0 || toupper(*s1) == toupper(*s2))
			return (1);					// Must have a match
		s2 = string2;
		++string1;
    }
    return (0);							// No match found
}


//**************************************
// Function: fatal - Exit on fatal error
// Returend: Never returns
//**************************************

void fatal(
	char *msg,
	char *string,
	int  code)

{
    if (string == (char *)NULL)
		fprintf(stderr, "\n? XMAKE: %s\n", msg);
    else
		fprintf(stderr, "\n? XMAKE: %s%s\n", msg, string);
    exit(code);
}


// Function to search tree for node

TREE_NODE *tsearch(
	char      *target,			// String to search for
	TREE_NODE *search)			// Pointer to node list to search

{
    while (search != (TREE_NODE *)NULL)	// Until out of targets
    {
        if (strcmp(search->file->fname, target) == 0)
            return (search);			// Found match - return node ptr
        search = search->next;			// Point to next node in list
    }
    return ((TREE_NODE *)NULL);			// No match found
}


//*****************************************
// Function: fcompare - Compare two strings
// Returned:
//*****************************************

int fcompare(
    char *str1,
    char *str2)

{
    char chr1;					// Place to keep chr during compare
    char chr2;

    while (*str1 != 0 && *str2 != 0) // Until end of either string
    {
        if (toupper(chr1 = *str1) != toupper(chr2 = *str2) ||
	    (chr1 == '\\' && chr2 != '/') || (chr1 == '/' && chr2 != '\\'))
										// If these characters don't match
            return (FALSE);				// Indicate failure
        ++str1;							// Bump pointer to next character
        ++str2;							//  On both strings
    }
    if (*str1 == '.' && *str2 == 0)		// Is this really needed?
		return (TRUE);
    if (*str1 != *str2)					// If both strings didn't end
        return (FALSE);					// Indicate failure
    return(TRUE);						// Otherwise indicate success
}


// Function to search tree for target

TREE_NODE *firstsrch(
	char      *target,			// String to search for
	TREE_NODE *search)			// Pointer to node list to search

{
    while (search != (TREE_NODE *)NULL)	// Until out of targets
    {
        if (fcompare(search->file->fname, target))
            return (search);			// Found match - return node ptr
        search = search->next;			// Point to next node in list
    }
    return ((TREE_NODE *)NULL);			// No match found
}


//*********************************************
// Function: inslash - Test for slash in string
// Returned:
//*********************************************

int inslash(
    char *string)

{
    char c;

    while ((c = *string++) != 0)
	if (c == '\\' || c == '/')			// If slash found
	    return (1);						// Return true for a slash
    return (0);							// Indicate no slashes found
}


//*******************************************
// Function: research - Search for dependents
// Returned: Nothing
//*******************************************

void research(
    TREE_NODE *start,		// Pointer to beginning of search
    TREE_NODE *dlist,		// Pointer to list of targets
    int        level)

{
    TREE_NODE *tpnt;		// Temp ptr into tree
    int        oldlevel;

    if (level >= 255)
    {
		fputs("\n? XMAKE: Too many levels of dependencies\n", stderr);
		exit(-1);
    }
    while (start != (TREE_NODE *)NULL)
    {
		if (start->level && start->level < level)
		{
			fprintf(stderr, "\n? XMAKE: Infinite loop detected containing %s\n",
					start->file->fname);
			exit(-1);
		}
		oldlevel = start->level;		// Keep track of what it was
		start->level = level;
		if (start->tree_list != (TREE_NODE *)NULL) // Move down and do it again
			research(start->tree_list, dlist, level + 1); // Search this arm
		else
			start->level = oldlevel;	// If not a target!
		if ((tpnt = tsearch(start->file->fname, dlist)) != (TREE_NODE *)NULL)
		{
			start->tree_list = tpnt->tree_list;
			start->shell_list = tpnt->shell_list;
			if (start->tree_list)
				research(start->tree_list, dlist, level + 1); // Try this for a
															  //   dependency
				start->level = oldlevel; // Not a target!
		}
		if (start->side != (TREE_NODE *)NULL) // If a :: with more than 1 entry
			research(start->side, dlist, level + 1);
		start->level = oldlevel;		// Restore before we go on
		if (level > 1)					// If not at first level
			start = start->next;
		else
	    	start = (TREE_NODE *)NULL;	// Process only the first name
    }
}


//***********************************************
// Function: treewalk - Process a dependency tree
// Returned:
//***********************************************

int treewalk(
    TREE_NODE *tree,
    int        level)		// Number of levels of recursion

{
    TREE_NODE *t_list = tree->tree_list; // Temporary ptr for requirements
    int update        = FALSE;	// Flag for dependency update
    int outofdate     = FALSE;	// Flag for up-to-date message

    if (t_list == (TREE_NODE *)NULL)	// If no dependency specified at all
	outofdate = update = TRUE;	// This file needs updating
    else
    {
        while (t_list != (TREE_NODE *)NULL) // Until we run out
        {
            if (t_list->tree_list != (TREE_NODE *)NULL) // If this has a
							//   requirement

                treewalk(t_list, level + 1);
	    if (t_list->file != NULL && t_list->file->fdatetime == 0xFFFFFFFFL)	/* Creation time/date not yet obtained */
		getinfo(t_list->file);	 // Get it!
	    if (t_list->file->fdatetime == 0)
	    {
	        fprintf(stderr, "? XMAKE: Dependant file %s not found\n",
			t_list->file->fname);
	        exit(1);
	    }
	    if (tree->file->fdatetime == 0xFFFFFFFFL)
		getinfo(tree->file);	// Get creation date/time for this file
            if (t_list->file == NULL || tree->file->fdatetime <
		    t_list->file->fdatetime)
            {				// If no dependent, or out of date
	        if (OPT_debug)
	        {
		    spaces(stdout, level);
		    printf("Out-of-date: %s", tree->file->fname);
		    showtime(tree->file->fdatetime);
		    if (strlen(tree->file->fname) > 12 ||
			    strlen(t_list->file->fname) > 12)
		        fputs("\n\t", stdout);
		    printf(" -> %s", t_list->file->fname);
		    showtime(t_list->file->fdatetime);
		    fputc('\n', stdout);
	        }
                outofdate = update = TRUE; // This file needs updating
            }
            else if (OPT_debug)
	    {
	        spaces(stdout, level);
	        printf("Up-to-date: %s", tree->file->fname);
	        showtime(tree->file->fdatetime);
	        if (strlen(tree->file->fname) > 12 ||
			strlen(t_list->file->fname) > 12)
		    fputs("\n\t", stdout);
	        printf(" -> %s", t_list->file->fname);
	        showtime(t_list->file->fdatetime);
	        fputc('\n', stdout);
	    }
            t_list = t_list->next;	// Advance to next requirement
        }
    }
    if (update)
        makethis(tree);			// Update this dependency
    if (tree->side != (TREE_NODE *)NULL) // If a second double-colon specified
	if (treewalk(tree->side, level) != FALSE)
	    outofdate = TRUE;
    return (outofdate);			// Return flag for up-to-date msg
}


void makethis(
    TREE_NODE *tree)			// Pointer to the item to make

{
    char *command = tree->shell_list; // Pointer to command to make this
    char filespec[CMDSIZE];		// Area to build the command in
    char fileopt[OPTSIZE];		// Area to build command options in
    char *d_type;				// Pointer to dependent file type
    char *t_type;				// Pointer to target file type
    char *x;					// Working pointer to filenames
    RULE_NODE *d_search;		// Pointer for searching for rules
    long rtn;

    if (command == (char *)NULL)		// If no command specified for this
    {
		x = tree->tree_list->file->fname; // Get dependent filename
		while (*x != 0 && *x != '.')// Advance to period
			++x;
		if (*x != '.')					// If a period wasn't found
			d_type = dummy;				// Point to just a period
		else
			d_type = x;
		x = tree->file->fname;			// Get target filename
		while (*x != 0 && *x != '.')	// Advance to period
			++x;
		if (*x == '.')					// If a period was found
			t_type = x;					// Point to it
		else
			t_type = dummy + 1;			// Otherwise point to a null string

		d_search = rulepnt;				// Set up to search list of rules
		if (OPT_debug)
			printf("\nSearching for %s%s rule\n", d_type, t_type);
		while (d_search != (RULE_NODE *)NULL) // Search until out of rules
		{
			if (strcmp(d_type, d_search->srctype) == 0 &&
					strcmp(t_type, d_search->desttype) == 0)
				break;
			else
				d_search = d_search->next;
		}
		if (d_search != (RULE_NODE *)NULL) // If match found
			command = d_search->command; // Get commands in rule
		else
		{
			if (default_cmd != (char *)NULL)
			{
				command = default_cmd;
				if (OPT_debug)
					fputs("Using .DEFAULT commands\n", stderr);
			}
			else
			{
				fprintf(stderr, "? XMAKE: No default rule found for %s%s and "
						"no .DEFAULT commands specified while building target "
						"%s\n", d_type, t_type, tree->file->fname);
				exit(1);
			}
		}
    }
    while (*command != 0)				// Until all commands processed
    {
		parsecmd(command, filespec, fileopt, tree); // Parse single command,
						    						//   expanding macros
        runprog(filespec, fileopt, tree); // Exec this command
		while (*command != 0 && *command != CR)
			++command;					// Advance to next command or end
		if (*command == CR)
			++command;
    }
    if (!OPT_noexec)					// Only if we actually did something
		getinfo(tree->file);			// Update the creation date for target
    if (tree->file->fdatetime == 0xFFFFFFFFL || OPT_noexec)
    {
		if ((rtn = svcSysDateTime(T_GTDDTTM, &tree->file->fdatetime)) < 0)
			femsg("XMAKE", rtn, NULL);
    }									// If target wasn't created by cmd
}


void parsecmd(
    char      *source,			// CR-terminated cmd string to parse
    char      *cmd,				// Command to pass to the exec fnctn
    char      *options,			// Option string to pass to command
    TREE_NODE *depend)			// Pointer to current dependency
{
    int   i;
    char *search;				// Pointer for command searches

    CMD_ignore = FALSE;
    CMD_silent = FALSE;

    if (*source == '-')					// If ignore error for this cmd
    {
		CMD_ignore = TRUE;
        ++source;
        if (*source == '@')				// If this command exec's silently
		{
			CMD_silent = TRUE;
			++source;
		}
    }
    else if (*source == '@')			// These options might be spec'ed in
    {									//   reverse order
		CMD_silent = TRUE;
		++source;
		if (*source == '-')
		{
			CMD_ignore = TRUE;
			++source;
		}
    }
    search = source;					// Init pointer for command searches
    i = 0;

    if (*source == 0 || *source == CR)
		return;
    while (isspace(*search & 0x7f) == 0)
    {
		++i;
		++search;						// Until first whitespace character
    }
    if (cmdcopy(cmd, (unsigned char *)source, depend, i, CMDSIZE) == EOF)
	{									// Number of characters to copy
			fprintf(stderr, "\n? XMAKE: %s @%s, Command \"%s\" is too long\n",
				makex, mline, source);
		exit(1);
    }
    cmdcopy(options, (unsigned char *)source, depend, -1, OPTSIZE);
										// Copy command options as well
}

void spaces(
    FILE *device,
    int   count)

{
    while (--count >= 0)
	fputc(' ', device);
}

//************************************************
// Function: showtime - Display file creation date
// Returned: Nothing
//************************************************

void showtime(
    long fdatetime)		// The packed creation date/time for a file

{
    if (fdatetime != 0)
	printf("(%d-%s-%d %d:%2.2d:%2.2d)",
		(int)((fdatetime >> 16) & 0x1F),
		months[(int)((fdatetime >> 21) & 0xF) - 1],
		(int)(((fdatetime >> 25) & 0x3F) + 80),
		(int)((fdatetime >> 11) & 0x1f), (int)((fdatetime >> 5) & 0x3f),
		(int)((fdatetime & 0x1f)) * 2);
    else
	fputs("(nonexistent)", stderr);
}
