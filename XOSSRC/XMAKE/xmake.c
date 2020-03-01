//**************************************
// XMAKE - A programming utility for XOS
//**************************************

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

// Revision history:
//
// v1.7 Fixed fact that date value was being treated as a signed number,
//      and some wildly impossible dates caused incorrect behavior.
//      Changed code so that only files actually used in the build will
//      have the creation date obtained, instead of obtaining creation
//      date for everything in the makefile.
//
// v3.2 Fixed bug #1051 - Added code the remove leading whitespace from
//      command strings.  Previously make became invisably unhappy with
//      the command if such was present.

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <XOSVECT.H>
#include <XOSSVC.H>
#include "XMAKE.h"

// static char buildtime[] = "Compiled: "
//			  __DATE__	/* Time this .EXE was compiled */
//			  " "
//			  __TIME__;

char *linebuffer;		/* Buffer for each line read from Makefile */
char *linepntr;			/* Pointer into Makefile buffer            */

TREE_NODE  *treepnt = (TREE_NODE *)NULL;  /* Pointer to dependency tree       */
TREE_NODE  *treeadd = (TREE_NODE *)NULL;  /* Tail pointer to dependency tree  */
FILE_NODE  *filepnt = (FILE_NODE *)NULL;  /* Pointer to list of file nodes    */
RULE_NODE  *rulepnt = (RULE_NODE *)NULL;  /* Pointer to default rule list     */
RULE_NODE  *ruleadd = (RULE_NODE *)NULL;  /* Tail pointer to default rule list*/
MACRO_NODE *macpnt  = (MACRO_NODE *)NULL; /* Pointer to macro list            */
MACRO_NODE *invoke  = (MACRO_NODE *)NULL; /* List of currently invoked macros */

char *default_cmd   = (char *)NULL; /* Pointer to default command string       */

int CMD_ignore   = FALSE;	/* Ignore errors for this command only     */
int CMD_silent   = FALSE;	/* Execute single command silently         */

int OPT_print    = FALSE;	/* Print out macro and target defs         */
int OPT_noexec   = FALSE;	/* Print out commands, but don't exec      */
int OPT_precious = FALSE;	/* Targets are not to be deleted           */
int OPT_envlock  = FALSE;	/* Don't allow re-def of enviroment macros */
int OPT_debug    = FALSE;	/* debug mode - Display all actions        */
int OPT_silent   = FALSE;	/* Execute all commands silently           */
int OPT_ignore   = FALSE;	/* Ignore error codes returned by commands */
int OPT_question = FALSE;	/* Question mode.  Do nothing, set status  */
int OPT_cont     = FALSE;	/* Continue if possible.                   */

char *xmakex = (char *)NULL;	// New name for the makefile
int   mline = 1;				// Current input line in Makefile
FILE *makefile = (FILE *)NULL;	// FILE pointer for MAKEFILE
char *target;					// Target for make operation


main(
	int   argc,
	char *argv[])

{
    TREE_NODE *d_tree;			// Pointer to dependency tree 
    int ques = TRUE;			// Flag for question mode result

    fprintf(stderr, "XMAKE - version %d.%d\n", VERSION, EDITNO);
    target = switchparse(argc, argv);	// Process any switches
    if (xmakex == (char *)NULL)			// If name not re-defined
		xmakex = "MAKEFILE.MAK";		// Define default name
    if (makefile == (FILE *)NULL)
		makefile = fopen(xmakex, "r");
    if (makefile == (FILE *)NULL)
    {
		if ((makefile = fopen("MAKEFILE", "r")) == (FILE *)NULL)
		{
			fprintf(stderr, "? XMAKE: Unable to open MAKEFILE.MAK or "
					"MAKEFILE\n", xmakex);
			exit(1);
		}
    }
    linepntr = linebuffer = getmemory(LINESIZE + 1);
    *linebuffer = '\0';					// Init line buffer for makefile read

    setvector(VECT_CNTC, 0x84, cntlcsrv); // Setup to take cntl-C interrupt
    svcSchSetLevel(0);

    buildtree();
    if (target == (char *)NULL)			// If no target specified
		target = treepnt->file->fname;	//  Use the first dependency
    if ((d_tree = firstsrch(target, treepnt)) != (TREE_NODE *)NULL)
    {
		research(d_tree, treepnt, 1);
		if (!OPT_question)				// If operating in question mode
		{
			if (treewalk(d_tree, 1) == FALSE) // If this is up to date
				fprintf(stderr, "%s is up to date\n", d_tree->file->fname);
		}
		else
		{
		    if (treewalk(d_tree, 1) == FALSE)
				fprintf(stderr, "%s is up to date\n", d_tree->file->fname);
		    else
	    	{
				fprintf(stderr, "%s is not up to date\n", d_tree->file->fname);
				ques = FALSE;
		    }
		}
    }
    else
    {
		fprintf(stderr, "? XMAKE: No dependencies found for %s\n", target);
		exit(1);
    }
    if (OPT_question && ques == FALSE)	// If in question mode, and out-of-date */
		exit(1);
    exit(0);
}
