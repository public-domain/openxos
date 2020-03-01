//------------------------------------------------------------------------------
// SHELL.C - Command decoder for XOS
// 
// Written by John R. Goltz and Bruce R. Nevins
// 
// Edit History:
// -------------
// 01/01/87(jrg) - Created first version
// 05/17/89(brn) - Fixed change drive command to take only simple form,
//                  fixed file not found check to not be simple case
// 12/05/89(brn) - Remove MODE command, remove screenmode variable,
// 	    		remove initscrn call, make cntlccnt a long
// 02/24/90(brn) - Remove cntlccnt and cntlcflg
// 03/02/90(brn) - Add support for ENV variable for dos or xos device names
// 05/22/91(brn) - Add CHDIR command definition
// 10/09/91(brn) - Fix change drive command to look at the string backwards
// 	    		to support network drive names
// 01/13/92(jrg) - Added support to scan for the right extension when being
// 	    		used of a network to UNIX
// 01/21/92(brn) - Fix bug where childcnt was non zero if error occured
// 	    		when storing the command
// 03/22/92(brn) - Add use of global.c module like all the utils
// 03/22/92(brn) - Make Illegal Command print under gcp_dosquirk otherwise
// 	    		return real error.
// 04/19/92(brn) - Merge Batch and Shell into one program
// 07/05/92(brn) - Make shell send batcmd argc and argv collected from
// 			input string.
// 07/20/92(brn) - Make all batch command files be processed internally
// 08/05/92(brn) - Change quit to bat_ctlc
// 08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
// 08/18/92(brn) - Move data into CMDDATA.C
// 11/02/95(brn) - Check for errors in getkeyword
// 07/19/96(brn) - moved parameters and setup of argpointer and argcounter
//			from batcmd to shell
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
#include <SETJMP.H>
#include <SIGNAL.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <XOSERR.H>
#include <XOSSVC.H>
#include <XOS.H>
#include <XOSTRM.H>
#include <GLOBAL.H>
#include <XOSSTR.H>
#include "SHELL.H"
#include "BATCHEXT.H"
#include "SHELLEXT.H"
#include "SHELLJMP.H"

/*
 * Main program
 */
int     argcounter;			/* Batch arg counter */
char   **argpointer;			/* Batch arg pointers */


void mainalt(
    img_data *args)

{
    int    argc;		/* Argument count */
    int    rtn;			// svc return value
    char   initialize;		/* Flag if initialize is done */
    char **argv;		/* Pointer to the array of pointers */
    char  *pnt;			/* Pointer into filename string */
    struct firstblk *newcmd;	/* New command to execute */

    initialize = FALSE;                 /* Show init not done */
    newcmd = NULL;			/* Start clear */
    devlist.dlstdin.src = STDIN;
    devlist.dlstdin.cmd = O_IN;
    setvector(VECT_HNGUP, 4, hungup);	/* Setup to take hung-up signal */
    setvector(VECT_CHILD, 4, childgone); /* Setup vector for child died */
					 /*   signal */
    setvector(VECT_CNTC, 4, cntlcsrv);	/* Setup cntl-C signal vector */
    resetscrn();			// Set screen to known state
    hsttmp = hstlast;                   /* Point temp at last block */
    topseen = FALSE;                    /* Flag for top of command list */
    botseen = TRUE;                     /* Flag for bottom of command list */

#ifdef ttttttt
    if (!svc_iospecfunc(STDTRM, TRMCLASS, 2, NULL))
    {                                   /* Do we still have a terminal? */
        cmdlogout();                    /* No - log off now! */
    }
#endif

/*
 * Here we initialize the default command ALIAS'
 */
    argc = 0;
    while ((pnt = alias_defaults[argc++]) != NULL)
    {
	while (*pnt != '\0')
	{
	    argbase = blkputc(argbase, *pnt++);
	}
	argbase = blkputc(argbase, *pnt++);
	if (!getkeyword(argbase))
	    continue;

	cmdalias();
	argbase = givechain(argbase);
    }

/*
 * Here we check to see if the BATCH file has been pre-opened
 * for us by the OS
 */
    _iob[0].iob_handle = STDFV;
    _iob[0].iob_flag = _F_READ|_F_ASCII;
    cmdin = &_iob[0];			/* Set up command in */
    if (_fsetup(&_iob[0]) == NULL)	/* Do standard stuff if failed */
    {
/*
 * No pre-opened file was found, see if we have a file name to use
 */
	argc = batargs(args->cmdtail, &argv); /* Setup the argument list */
        _iob[0].iob_flag = 0;		/* Make available again */

	cmdin = stdin;			/* No file was found, just take */
        if (argc >= 2)			/*  commands from STDIN */
	{
            argv++;			/* Adjust to point to "Command name" */
            argc--;			/* Adjust to reduce the count correctly */
            strcpy(filname, argv[0]);

            strupper(filname);
	    pnt = filname;

            while (*pnt != '\0')
            {
                newcmd = blkputc(newcmd, *pnt++);
            }
            newcmd = blkputc(newcmd, '\0');	/* End the line */

            blkinic(newcmd);		/* Start command at beginning */
	    initialize = setjmp(rstbuf); /* Setup for restarts */
	    if (!initialize)
            {
	        svcSchSetLevel(0);	/* Enable software interrupts */
                dointcmd(newcmd);
            }
	    cmdexit();
	}
    }
    else
    {
	batch = TRUE;
	argc = batargs(args->cmdtail, &argv); /* Setup the argument list */
    }

    svcMemChange(args, 0, 0);		/* Give up the argument msect */
    if ((rtn = svcIoInBlock(STDIN, 0, 0)) == ER_BDDVH)
    {
fprintf(stderr, "stdin not setup\n");
fflush(stderr);
	detached = TRUE;		// Running under the batchserver
    }
    initialize = setjmp(rstbuf);        /* Setup for restarts */
    hsttmp = hstlast;                   /* Point temp at last block */
    topseen = FALSE;                    /* Flag for top of command list */
    botseen = TRUE;                     /* Flag for bottom of command list */

    svcSchSetLevel(0);			/* Enable software interrupts */

    if (!initialize)			/* Only do this the first time */
    {
	if (!batch)
            inicmd("SHELLINI");		/* Execute from the environment string */

	echo = TRUE;
/*
 * Check Global Configuration Parameters
 */
	global_parameter(TRUE);
    }

    argcounter = argc;
    argpointer = argv;

    
    while (TRUE)
    {
	if (cmdin != stdin && feof(cmdin))
	    break;

	if (batch && bat_ctlc)
	    break;

	if ((cmdin == stdin) && bat_ctlc)
	{
	    bat_ctlc = FALSE;
	}

        batcmd();			/* Process the command */
    }
    cmdexit();
}   

