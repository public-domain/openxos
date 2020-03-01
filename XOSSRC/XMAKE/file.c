//*****************************************
// FILE.C - Routines for file I/O for XMAKE
//*****************************************

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
#include <XOSERR.H>
#include <XOSERRMSG.H>
#include "XMAKE.H"

///#define BSIZE 256
#define BSIZE 512


struct
{
    byte4_parm  mdate;
    char        end;
} fcparm =
{
    {(PAR_GET|REP_HEXV), 4, IOPAR_MDATE, 0},
    0
};

type_qab fcqab =
{
    QFNC_WAIT|QFNC_DEVPARM,	// qab_func    - Function
    0,				// qab_status  - Returned status
    0,				// qab_error   - Error code
    0,				// qab_amount  - Amount
    0,				// qab_handle  - Device handle
    0,				// qab_vector  - Vector for interrupt
    0, 0,			//             - Reserved
    0,				// qab_option  - Options or command
    0,				// qab_count   - Count
    NULL, 0,			// qab_buffer1 - Pointer to file spec
    NULL, 0,			// qab_buffer2 - Unused
    &fcparm, 0			// qab_parm    - Pointer to parameter area
};

char buffer[BSIZE];		// Place to build atoms
static char putback[BSIZE] = {0}; // Place to store returned atoms
       char svchr    = 0;	// Place to store returned characters
       char oldchr1  = 0;	// Place for getatom to return chars
       char oldchr2  = 0;	// Second place for getatom to return chr
static char lastatom = 0;	// 1st chr of last atom to be processed
static char lastnl   = '\n';	// Flag to indicate last chr was newline
static char last_phys = '\n';	// Last physical character read from file
static char sbody[10];		// Place to store body of special macro
static uchar *sq = (char *)NULL; // Pointer to special macro bodies

//************************************************
// Function: getcmd - Read a command from makefile
// Returned:
//************************************************

char *getcmd(void)

{
    char  c;
    char *atom;
    char *buffer = (char *)NULL;
    char *cmd    = (char *)NULL;
    char *ctmp;
    int   cntr   = BSIZE;

    while ((atom = getatom()) != (char *)NULL && *atom == '\t')
    {									// Read commands for this dependency
		ctmp = cmd;						// Set pointer for debug printout
		while ((c = readchr()) != EOF && c != '\n' && isspace(c))
			;							// Eat leading whitespace per bug #1051
		if (c != EOF && c != '\n')
		{
			do
			{
				if (cmd == (char *)NULL)
					ctmp = buffer = cmd = getmemory((long)BSIZE);
				if (cntr <= 0)
				{
					getmore((long)BSIZE, cmd);
					cntr = BSIZE;
				}
				*cmd++ = c;
				--cntr;
			} while ((c = readchr()) != '\n' && c != EOF);
		}
		if (cntr <= 1)					// If out of space
		{
			getmore((long)2, cmd);		// Get enough for terminating chr
			cntr += 2;
		}
		if (c != EOF)
		{
			lastatom = c;				// Set last atom
			*cmd++ = CR;				// End command with CR
			--cntr;
		}
		*cmd = 0;						// Terminate command properly
		if (OPT_debug)
			cmdprint((unsigned char *)ctmp); // Display command for debug stuff
	}
	ungetatom(atom);					// Put this atom back for now
	return (buffer);    
}

//*********************************************************
// Function: getmacro - Read a macro body from the Makefile
// Returned:
//*********************************************************

char *getmacro(void)

{
    char  c;
    char *buffer;
    char *cmd;
    int   cntr = BSIZE;

    buffer = cmd = getmemory((long)BSIZE);
    while ((c = readchr()) == ' ' || c == '\t')
		;
    if (c != '\n' && c != EOF)
		do
		{
			if (cntr <= 0)
			{
				getmore((long)BSIZE, cmd);
				cntr = BSIZE;
			}
			*cmd++ = c;
			--cntr;
		} while ((c = readchr()) != '\n' && c != EOF);
	if (c != EOF)
		lastatom = c;					// Set last atom
    if (cntr <= 0)
		getmore((long)1, cmd);
    *cmd = 0;							// Terminate macro properly
    return (buffer);    
}

// This function reads input from the MAKEFILE, and processes it into
//   'atoms' which are the basic elements of the MAKEFILE.
// The following items are considered to be atoms:
//
// + Any continous string of alphanumeric digits, possibly including
//   underlines, minus signs, periods, slashes, and colons not
//   followed by space or tab.  (Leading periods permitted.)
// + Newline
// + Tab, immediately following a Newline
// + ': '
// + ':: '
// + '='

char *getatom(void)						// Function to read an atom from
										//   makefile
{
    char c = 0;
    char *atom = buffer;				// Pointer into character buffer

    if (*putback != 0)					// If an atom has been put back
    {
        strcpy(buffer, putback);		// Copy it out of putback buffer
        *putback = 0;					// Clear the putback buffer
        return (buffer);				// And return that atom
    }

    while (isspace(c = toupper(readchr())) && c != EOF && c != '\n' &&
			(lastatom != '\n' || c != '\t')); // Eat leading whitespace
    if (c == EOF)						// If at end of file
        return ((char *)NULL);			// No atom to return

    *atom++ = c;
    if (c == '=' || c == '\n' || (c == '\t' && lastatom == '\n'))
	{									// Tab and newline are complete atoms
		lastatom = c;
        *atom = 0;
        return (buffer);
    }
    lastatom = c;
    while (isspace(c = toupper(readchr())) == 0 && c != EOF)
    {
		if (c == ':')					// Must handle colon with care
		{
			if (isspace(c = toupper(readchr())) || c == ':')
			{			 				// If next chr is space or another colon
				if (*(atom - 1) == ':')
				{
					*atom++ = ':';		// Add this colon to atom
					if (isspace(c))		// Check if we stop!
					{
						oldchr1 = c;
						break;
					}
				}
				else
				{
					oldchr1 = ':';		// Save the colon for later
					break;				// That'll terminate this atom
				}
			}
			*atom++ = ':';
			if (c == EOF) 				// Check if this terminates atom
				return ((char *)NULL);
		}
		if (c == '=')
			break;
		*atom++ = c;					// Until whitespace
	}
	if (c != EOF)
		oldchr2 = c;					// Put this character back
	*atom = 0;							// Terminate string properly
	return (buffer);					// Return pointer to atom
}


void ungetatom(pnt)
char *pnt;
{
    if (pnt != (char *)NULL)	// Don't unget a null atom!
        strcpy(putback, pnt);	// Copy into putback buffer
}

static char fileread(void)
{
    char c;
    int redo;
    static int newline = FALSE;

    if (newline == TRUE)
	{
		newline = FALSE;
		++mline;						// Bump line count
	}

    if (invoke != (MACRO_NODE *)NULL)	// If macro expansion in progress
    {
		redo = 1;
		while (redo)
		{
			redo = 0;
			if (*(invoke->position) != 0) // If not at end of macro
				return (*(invoke->position++)); // Return next chr from macro
			invoke->position = (char *)NULL;	// Set this macro to inactive
			if (invoke->nested != (MACRO_NODE *)NULL)
			{							// If another macro nested
				invoke = invoke->nested; // Make this the active macro
				redo = 1;				// And do it all over again
			}
			else
				invoke = (MACRO_NODE *)NULL; // No macro active
		}
    }
    if (*linepntr == 0)
    {
		if (fgets(linebuffer, LINESIZE, makefile) == (char *)NULL)
			return (EOF);				// Indicate end-of-file or error
		linepntr = linebuffer;			// Reset to beginning of line buffer
    }
    c = *linepntr++;					// If macro doesn't return a chr,
										//   read one
    if (c == 0x1a)						// If extra ctrl-Z's
		c = EOF;						// Indicate end of file
    if (c == 0)							// If extra NULL's...
		c = EOF;						// Indicate end of file

    if (c != EOF)						// If not at end of file
		c = c & 0x7f;					// Strip the high bit from the character
    if (c == '\n')						// If a newline in makefile
		newline = TRUE;					// Count it next time a character is
										//   read
    return (c);
}

char readchr(void)
{
    char  c;
    char  mn[256];
    char *macname;				// Place to build macro names
    MACRO_NODE *msearch;		// Pointer to macro list

    if (oldchr1 != 0)					// If there's a returned chr waiting
    {
		c = oldchr1;					// Keep it for a sec...
		oldchr1 = 0;					// Clear the old chr from buffer
		return (c);						// And just return this one again
    }
    if (oldchr2 != 0)
    {
		c = oldchr2;					// Keep it for a sec...
		oldchr2 = 0;					// Clear the old chr from buffer
		return (c);						// And just return this one again
    }
    if (sq != (char *)NULL)				// If special macro body waiting
    {
		if (*sq != 0)
		{
			if (OPT_print)
				prtprint(*sq);
			return (*sq++);
		}
		else
			sq = (char *)NULL;
    }

    for (;;)
    {
		if (svchr == 0)					// If no chr leftover from last time
			c = fileread();				// Read another chr from file
		else
		{
			c = svchr;					// Use leftover chr
			svchr = 0;
		}

		if (c == '#')					// Until out of comments to eat
		{
			while ((fileread()) != '\n')
				;						// Eat a comment
			if (last_phys == '\n')		// If entire line was a comment
				continue;				// Go around the loop again
			else
				c = '\n';				// Otherwise pass through the newline
		}
		last_phys = c;

		if (c == '\\')					// If this might be an escaped newline
		{
			c = fileread();
			if (c != '\\')				// If we don't have an escaped backslash
			{
				if (c == '\n')			// If we escaped a newline
				{
					last_phys = c;		// Set this up for the comment eater
					while ((c = fileread()) == ' ' || c == '\t')
						;				// Eat all spaces and tabs following \n
					svchr = c;			// Save this chr for later
					if (OPT_print)
						prtprint(' ');
					return (' ');		// And return a single space
				}
				else
				{
					svchr = c;			// Not \\ and not newline, put it back
					if (OPT_print)
						prtprint('\\');
					return ('\\');		// And return backslash
				}
			}
			else
			{
				svchr = c;				// We escape only newline, just
				if (OPT_print)
					prtprint('\\');
				return ('\\');			// return this backslash
			}
		}

		if (c == '$')					// Check if macro invokation
		{
			if ((c = fileread()) != '$') // Second $ just passes through
			{
				macname = mn;
				if (c == '(')			// If beginning of long macro name
				{
					while ((c = toupper(fileread())) != ')' && c != EOF &&
							c != '\n')
					*macname++ = c;
					if (c == EOF)		// If improperly terminated by EOF
					{
						fprintf(stderr, "? XMAKE: %s @%d, Macro invocation "
								"unexpectedly terminated by end of file\n",
						xmakex, mline);
						showerror();
						exit(1);
					}
					if (c == '\n')		// If improperly terminated by EOL
					{
						fprintf(stderr, "? XMAKE: %s @%d, Macro invocation "
								"unexpectedly terminated by end of line\n",
								xmakex, mline);
						showerror();
						exit(1);
					}
				}
				else
					*macname++ = c;
				*macname = 0;			// Terminate macro name properly
				if (*mn == '*' || *mn == '@' /* || *mn == '<' */ ||
						*mn == '?' /* || *mn == '%' */)
				{						// If we have a magic macro
					sq = sbody;			// Set up pointer
					macname = mn + 1;
					if (*macname == 0)	// If no parameters at all
					{
						*sq++ = ' ' | 0x80; // Just indicate space
						*sq++ = 0;
						sq    = sbody;
						if (OPT_print)
						{
							prtprint(c = (*mn | 0x80));
							return (c);	// Return the macro character
						}
						else
							return (*mn | 0x80);
					}
					if (strlen(mn + 1) > 9)	// If string too long
					{
						fprintf(stderr, "? XMAKE: %s @%d, Too many parameters "
								"specified for internal macro $%c\n", xmakex,
								mline, *mn);
						showerror();
						exit(1);
					}
					while (*macname != 0 && *macname != ':')
										// Process the body
						*sq++ = *macname++;
					if (*macname == ':' && *(macname + 1) != 0)
										// If a separator specified
						*sq++ = *(macname + 1) + 0x80;
					else
						*sq++ = ((uchar)' ') + 0x80;
					*sq = 0;			// Terminate this properly
					sq = sbody;
					if (OPT_print)
					{
						prtprint(c = (*mn | 0x80));
						return (c);		// Return 1st chr with high bit set
					}
					else
						return (*mn | 0x80);
				}
				msearch = macpnt;		// Init search pointer
				while (msearch != (MACRO_NODE *)NULL &&
						strcmp(mn, msearch->token) != 0)
					msearch = msearch->next; // Search macro list for match
				if (msearch != (MACRO_NODE *)NULL) // If match for this macro
				{								   //   found
					if (msearch->position != (char *)NULL)
					{					// Macro may not self-invoke
						fprintf(stderr, "? XMAKE: %s @%d, Macro %s may not "
								"invoke itself\n", xmakex, mline, mn);
						showerror();
						exit(1);
					}		
					msearch->nested = invoke; // Link into active macro list
					invoke = msearch;
					msearch->position = msearch->value;
					continue;			// Go around and get another char
				}
				fprintf(stderr, "? XMAKE: Internal Error - Macro Not Found "
						"'%s'\n", mn);

				// At this point, we'd search the enviroment list if we don't
				//   decide to automatically build a macro list from the
				//   enviroment at startup

				continue;				// Need to get another character
			}
		}
		if (c == '\n')					// Check if last chr was newline
			lastnl = c;
		else
			lastnl = 0;
		if (OPT_print)
			prtprint(c);
		return (c);
    }
}


//**********************************************
// Function: getinfo - Return creation date/time
//		for file defined by FILE_NODE
// Returned: Nothing
//**********************************************

void getinfo(
    FILE_NODE *file)

{
    long rtn;

    fcqab.qab_buffer1 = file->fname;
    if ((rtn = svcIoQueue(&fcqab)) < 0 || (rtn = fcqab.qab_error) < 0)
    {
		if (rtn != ER_FILNF)
			femsg("XMAKE", rtn, file->fname);
		file->fdatetime = 0;
    }
    else
		file->fdatetime = fcparm.mdate.value;
}
