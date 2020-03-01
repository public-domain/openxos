//------------------------------------------------------------------------------
// batcmd.c - Batch command processor
//
// Written by Bruce R. Nevins
//
// Edit History:
// -------------
// 01/01/87(brn) - First version
// 05/17/89(brn) - Fixed change drive command to take only simple form,
//                  fixed file not found check to not be simple case
// 12/05/89(brn) - Remove MODE command
// 02/22/91(brn) - Fix Redirection and Pipe handling
// 05/22/91(brn) - Add CHDIR command definition
// 10/09/91(brn) - Fix change drive command to look at the string backwards
// 	    to support network drive names
// 01/21/92(brn) - Fix bug where childcnt was non zero if error occured
// 	    when storing the command
// 03/22/92(brn) - Make Illegal Command print under gcp_dosquirk otherwise
// 	    return real error.
// 07/05/92(brn) - Make batcmd take parameters for argc and argv from shell
// 07/23/92(brn) - Move batch file processing into BATBAT.C
// 07/25/92(brn) - Fix bug where temp buffer did not have room for null
// 	    string terminator.
// 07/30/92(brn) - Fix bug when specifying ..\foo to run a batch file
// 07/30/92(brn) - Make ? Invalid command print when not dosquirk
// 08/05/92(brn) - Turn off and on interrupts in key points to prevent ^C
// 	    from messing up the screen
// 08/05/92(brn) - Check for bat_ctlc after enabling software interrupts
// 08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
// 	   Save and restore file handles for redirection calls
// 08/19/92(brn) - Flush output for redirected output before run
// 12/26/92(brn) - Don't allow// or ? in a program name
// 03/01/93(brn) - Don't return to bat file unless extend batch or a CALL
// 02/22/94(brn) - Fix bug where not disabling interrupts before doing
// 		a program run.  This caused a race with process
// 		termination.
// 02/23/94(brn) - Changed the command table to use a command structure
// 03/09/94(brn) - Fix echoing when a blank command is entered under batch
// 		Finish command alias handling
// 05/24/95(brn) - Make batch files set SHELL to the batch file name while
// 		it is running
// 23May95 (fpj) - Changed code to output to cmderr instead of cmdout.
// 11/02/95(brn) - Check for errors in getkeyword
// 07/19/96(brn) - moved parameters and setup of argpointer and argcounter
//			from batcmd to shell
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

//#define DEBUG 0

#include <CTYPE.H>
#include <SETJMP.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <XOSERR.H>
#include <XOS.H>
#include <XOSTRM.H>
#include <XOSSVC.H>
#include <XOSRUN.H>
#include <GLOBAL.H>
#include <XOSSTR.H>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"
#include "SHELLJMP.H"

extern struct   runparm runparm;	/* Parameter list for RFNC_RUN */
extern type_qab runqab;			/* QAB for RFNC_RUN */

/*
 * Command table
 */

struct command dos_cmddsp[] =
{
	{"ALIAS",	cmdalias},
	{"BATOPT",	batopt},
	{"CD",		cmdcd},
	{"CHDIR",	cmdcd},
	{"CLS",		cmdcls},
	{"RESET",	cmdreset},
	{"EXIT",	CD(cmdexit)},
	{"LOGOUT",	cmdlogout},
	{"BYE",		cmdlogout},
	{"CALL",	batcall},
	{"ECHO",	batecho},
	{"FOR",		batfor},
	{"GOTO",	batgoto},
	{"HISTORY",	cmdhist},
	{"IF",		batif},
	{"JUMP",	batjump},
	{"PAUSE",	batpause},
	{"REM",		batrem},
	{"RETURN",	batreturn},
	{"SHIFT",	batshift},
	{"VER",		cmdver}
};

#define NUMCMDS sizeof(dos_cmddsp) / sizeof (struct command)

char   *ext;
char   *extbgn;
char  **extpnt;
char   *exttbl[] = {   ".RUN", ".BAT", ".EXE", ".COM", NULL};

struct vbofparm
{   byte4_parm vbof;
    char       end;
} vbofparm =
{   {PAR_SET|PAR_GET|REP_DECV, 4, IOPAR_VBOF, -2L}
};

struct lmparms
{   byte4_parm clrp;
    byte4_parm setp;
    char       end;
} lmparms =
{   {PAR_SET|REP_DECV, 4, IOPAR_TRMCINPMODE,
            0xFFFFFFFFL&(~(TIM_ECHO|TIM_ILFACR))},
    {PAR_SET|REP_DECV, 4, IOPAR_TRMSINPMODE, TIM_ECHO|TIM_ILFACR}
};

void batcmd(void)
{
    register int value0;
    char    *pnt1, *tempstr;
    long     runrtn;                    /* Return from run */
    register char  chr;
    int      name_seen;			/* Program name was seen */
    int	     i, n;			/* Misc counter */
    int      old_echo;			/* Previous value of echo */
    FILE    *old_cmdin;			/* FD for Batch Command input */
    long     old_stdin;			/* FD for Batch Standard input */
    FILE    *old_cmdout;		/* FD for Batch Standard output */
    FILE    *old_cmderr;		/* FD for Batch Standard error */
    int      temp_childcnt;		/* Holder for child number */

    devlist.dlstdtrm.src = STDTRM;

    svcSchSetLevel(8);			/* No software interrupts now */
    if (cmdin == stdin)
	doprompt(FIRST);
    svcSchSetLevel(0);			/* Software interrupts now */

/*
 * Save previous cmd i/o values
 */
    old_cmdin = cmdin;
    old_cmdout = cmdout;
    old_cmderr = cmderr;

// Setup for abort batch run
    setjmp(batrstbuf);			// Setup for restarts
    if (bat_ctlc)			// If ^C seen just return
    {
	if (cmdin != old_cmdin)
	{
            fclose(cmdin);
            cmdin = old_cmdin;
	}
	if (cmderr != old_cmderr)
	{
            fclose(cmderr);
            cmderr = old_cmderr;	// Redir on stderr does
            cmdout = old_cmdout;	//  stdout to match
	}
	if (cmdout != old_cmdout)
	{
            fclose(cmdout);
            cmdout = old_cmdout;
	}
	if (cmdout != stdout)
	{
	    fflush(cmdout);
	    _fsetup(cmdout);
	    fseek(cmdout, 0L, SEEK_END);// Must be sure to be at end
					//  of file
	}

	return;
    }

// Get the command
    if (!getcmd())                      /* Get command */
    {
        linecnt = 0;                    /* If error, discard rest of line */
	if (cmdin != stdin && echo && !tempnoecho)
	    fputc('\n', cmdout);

        return;
    }
    svcSchSetLevel(8);			/* No software interrupts now */
    blkinic(argbase);                   /* Point to beginning of string */

    if (expndarg != NULL)
        expndarg = givechain(expndarg); /* Give up old command line */

    expndarg = batxlate(expndarg, argbase);
                                        /* Expand the command line arguments */
    blkinic(expndarg);                  /* Point to beginning of string */

    if (!getkeyword(expndarg))		// Get first command atom
	return;

    strupper(keyword);
    if (tempnoecho)
    {
        skipprompt = TRUE;              /* Skip the next prompt */
    }
    if (cmdin != stdin)
	doprompt(FIRST);

/*
 * Get the keyword and make upper case
 */
    blkinic(expndarg);			// Point to beginning of string
    if (!getkeyword(expndarg))		// Get first command atom
	return;

    strupper(keyword);			/* Convert to upper case */

/*
 * Check if history search
 */
    if (*keyword == '!')		/* If history search */
    {
        cmdhstsrch();			/* Search the command history */
	svcSchSetLevel(0);		/* Software interrupts now */
	if (cmdin != old_cmdin)
	{
	    fclose(cmdin);
	    cmdin = old_cmdin;
	}
	if (cmderr != old_cmderr)
	{
	    fclose(cmderr);
	    cmderr = old_cmderr;		/* Redir on stderr does */
	    cmdout = old_cmdout;		/*  stdout to match */
	    devlist.dlstdout.src = cmdout->iob_handle;
	    devlist.dlstderr.src = cmderr->iob_handle;
	}
	if (cmdout != old_cmdout)
	{
	    fclose(cmdout);
	    cmdout = old_cmdout;
	    devlist.dlstdout.src = cmdout->iob_handle;
	}
        return;
    }

/*
 * Copy new command into the history buffer (Only if from STDIN)
 */
    if (histlimit != 0 && cmdin == stdin && !tempnohist)
    {
        blkinic(argbase);		/* Point to beginning of string */
        if (hstlast == NULL)
        {
            hstlast = blkcpy(hstlast, argbase);
            hstfrst = hstlast;
            hsttmp = hstlast;
            hstlast->a.lastlst = NULL;
            hstlast->a.nextlst = NULL;
            histcnt = 1;		/* Initialize the history count */
        }
        else
        {
            blkinic(argbase);		/* Point to beginning of string */
            blkinic(hstlast);		/* Point to beginning of string */
            if (blkstrcmp(argbase, hstlast) != 0)
            {
                blkinic(argbase);	/* Point to beginning of string */
                hstlast->a.nextlst = blkcpy(hstlast->a.nextlst, argbase);
                hsttmp = hstlast->a.nextlst;
                hsttmp->a.lastlst = hstlast;
                hstlast = hsttmp;	/* New last pointer */
                hstlast->a.nextlst = NULL;
					/* No next pointer */
                if (hstfrst == NULL)
                    hstfrst = hstlast;
                if (++histcnt > histlimit)
                {
                    histcnt--;
    		    histlow++;
                    hsttmp = hstfrst->a.nextlst;
					/* Point to new first block */
                    hsttmp->a.lastlst = NULL;
					/* Clear last pointer in new block */
                    givechain(hstfrst); /* Give up old first block */
                    hstfrst = hsttmp;   /* New first pointer */
                    hsttmp = hstlast;   /* Point temp at last block */
                }
            }
        }
        topseen = FALSE;		/* Flag for top of command list */
        botseen = TRUE;			/* Flag for bottom of command list */
    }
    else
	tempnohist = FALSE;		/* Reenable history if available */
/*
 * Check if change drive
 */
    if ((pnt1 = strrchr(keyword, ':')) != NULL && firstarg == NULL)
    {
        if (*++pnt1 == '\0')
        {
            cmdchngdrv(keyword);
	    svcSchSetLevel(0);		/* Software interrupts now */
	    if (cmdin != old_cmdin)
	    {
		fclose(cmdin);
		cmdin = old_cmdin;
	    }
	    if (cmderr != old_cmderr)
	    {
		fclose(cmderr);
		cmderr = old_cmderr;		/* Redir on stderr does */
		cmdout = old_cmdout;		/*  stdout to match */
		devlist.dlstdout.src = cmdout->iob_handle;
		devlist.dlstderr.src = cmderr->iob_handle;
	    }
	    if (cmdout != old_cmdout)
	    {
		fclose(cmdout);
		cmdout = old_cmdout;
		devlist.dlstdout.src = cmdout->iob_handle;
	    }
            return;
        }
    }

/*
 * Do any alias mappings
 */
    if (cmdalssrch())			/* Search the Alias List */
	return;				/* We did the ALIASed command */

/*
 * Check table of commands
 */
    blkinic(expndarg);			/* Point to beginning of string */

    if (simple && (value0=srchatom(keyword,
                  dos_cmddsp, NUMCMDS))
                        >= 0)           /* Search the command table */
    {
        if (pipeseen)
        {
            fprintf(cmderr, "? Internal commands cannot use pipes\n");
	    if (detached)
		fflush(cmderr);
            pipeseen = FALSE;		/* Flag not doing a pipe */
	    svcSchSetLevel(0);		/* Software interrupts now */
	    if (cmdin != old_cmdin)
	    {
		fclose(cmdin);
		cmdin = old_cmdin;
	    }
	    if (cmderr != old_cmderr)
	    {
		fclose(cmderr);
		cmderr = old_cmderr;		/* Redir on stderr does */
		cmdout = old_cmdout;		/*  stdout to match */
		devlist.dlstdout.src = cmdout->iob_handle;
		devlist.dlstderr.src = cmderr->iob_handle;
	    }
	    if (cmdout != old_cmdout)
	    {
		fclose(cmdout);
		cmdout = old_cmdout;
		devlist.dlstdout.src = cmdout->iob_handle;
	    }
            return;
        }

	if (echo && !tempnoecho && cmdin != stdin)
        {
            blkinic(expndarg);          /* Point to beginning of string */
            blkputs(expndarg, cmderr);  /* Print the string */
            fputc('\n', cmderr);
        }

	old_stdin = devlist.dlstdin.src;
        if ((*dos_cmddsp[value0].fnc)() == TRUE)/* Dispatch to command */
	{
	    if (cmdin != old_cmdin)
	    {
		fclose(cmdin);
		cmdin = old_cmdin;
	    }
	    if (cmderr != old_cmderr)
	    {
		fclose(cmderr);
		cmderr = old_cmderr;		/* Redir on stderr does */
		cmdout = old_cmdout;		/*  stdout to match */
		devlist.dlstdout.src = cmdout->iob_handle;
		devlist.dlstderr.src = cmderr->iob_handle;
	    }
	    if (cmdout != old_cmdout)
	    {
		fclose(cmdout);
		cmdout = old_cmdout;
		devlist.dlstdout.src = cmdout->iob_handle;
	    }
	    devlist.dlstdin.src = old_stdin;
            return;
	}
    }

    if (*keyword == ':')                /* If label */
    {
        batlabel();
	svcSchSetLevel(0);		/* Software interrupts now */
	if (cmdin != old_cmdin)
	{
	    fclose(cmdin);
	    cmdin = old_cmdin;
	}
	if (cmderr != old_cmderr)
	{
	    fclose(cmderr);
	    cmderr = old_cmderr;		/* Redir on stderr does */
	    cmdout = old_cmdout;		/*  stdout to match */
	    devlist.dlstdout.src = cmdout->iob_handle;
	    devlist.dlstderr.src = cmderr->iob_handle;
	}
	if (cmdout != old_cmdout)
	{
	    fclose(cmdout);
	    cmdout = old_cmdout;
	    devlist.dlstdout.src = cmdout->iob_handle;
	}
        return;
    }

    if (*keyword == '\x1a')             /* If ^Z (Old DOS end of File) */
    {
	svcSchSetLevel(0);		/* Software interrupts now */
	if (cmdin != old_cmdin)
	{
	    fclose(cmdin);
	    cmdin = old_cmdin;
	}
	if (cmderr != old_cmderr)
	{
	    fclose(cmderr);
	    cmderr = old_cmderr;		/* Redir on stderr does */
	    cmdout = old_cmdout;		/*  stdout to match */
	    devlist.dlstdout.src = cmdout->iob_handle;
	    devlist.dlstderr.src = cmderr->iob_handle;
	}
	if (cmdout != old_cmdout)
	{
	    fclose(cmdout);
	    cmdout = old_cmdout;
	    devlist.dlstdout.src = cmdout->iob_handle;
	}
        return;				/*  just ignore it */
    }

    if (echo && !tempnoecho && cmdin != stdin)
    {
        blkinic(expndarg);      /* Start from beginning */
        blkputs(expndarg, cmderr);      /* Print the string */
        fputc('\n', cmderr);
    }

    extbgn = NULL;

    extpnt = exttbl;
    if (simple)
        strcpy(extbgn = strmov(strmov(runname, dftdev), keyword),
                *extpnt);
    else
    {
	if ((pnt1 = strrchr(keyword, '\\')) == NULL)
	    pnt1 = keyword;

        if (strchr(pnt1, '.') != NULL)
            strcpy(runname, keyword);
        else
	{
            strcpy((extbgn = strmov(runname, keyword)), *extpnt);
	}
    }

    if (strpbrk(runname, "?*") != NULL)
    {
	if (gcp_dosquirk)
            fputs("Bad command or filename\n", cmdout);
	else
	    fputs("? Invalid command\n", cmdout);
	if (detached)
	    fflush(cmdout);
	return;
    }

    blkinic(expndarg);			/* Point to beginning of string */
    i = ((int)blklen(expndarg) + 2);	/* Get length of string for malloc */
					/* + 2 (NULL and possible space */
    if ((pnt1 = malloc(i)) == NULL) /* Make buffer for command */
    {
        fprintf(cmderr, "? Command lost - out of memory\n");
	if (detached)
	    fflush(cmderr);
        free(pnt1);			/* Give up any extra memory */
	svcSchSetLevel(0);		/* Software interrupts now */
	if (cmdin != old_cmdin)
	{
	    fclose(cmdin);
	    cmdin = old_cmdin;
	}
	if (cmderr != old_cmderr)
	{
	    fclose(cmderr);
	    cmderr = old_cmderr;		/* Redir on stderr does */
	    cmdout = old_cmdout;		/*  stdout to match */
	    devlist.dlstdout.src = cmdout->iob_handle;
	    devlist.dlstderr.src = cmderr->iob_handle;
	}
	if (cmdout != old_cmdout)
	{
	    fclose(cmdout);
	    cmdout = old_cmdout;
	    devlist.dlstdout.src = cmdout->iob_handle;
	}
        return;
    }
    tempstr = pnt1;			/* Temp pointer for string */

/*
 * Copy the command string without leading space
 */
    while (isspace(chr = blkgetc(expndarg)))    /* Remove space at beinning */
        ;

    i = strlen(keyword);
/*  strnset(pnt1, 0, i); */
    for (n = 0; n < i; n++)
        *(pnt1+n) = 0;

    *tempstr++ = chr;
    name_seen = FALSE;

    while ((chr = blkgetc(expndarg)) != '\0' &&
            chr != '>' && chr != '<')   // FIXME when pipes work! - FPJ, 23May95
//          chr != '>' && chr != '<' && chr != '|')
    {
        if (!name_seen && (strnicmp(keyword, pnt1, i) == 0))
        {
    	    if(!isspace(chr))
	    {
	        *tempstr++ = ' ';
	    }
	    name_seen = TRUE;
	}

        *tempstr++ = chr;
    }
    *tempstr = '\0';            /* end string */


/*
 * Build the run block
 */
#ifdef DEBUG
fprintf(stderr, "call devlist.dlstdin.src  = %8.8lx\n", devlist.dlstdin.src);
fprintf(stderr, "call devlist.dlstdout.src = %8.8lx\n", devlist.dlstdout.src);
fprintf(stderr, "call devlist.dlstderr.src = %8.8lx\n", devlist.dlstderr.src);
fprintf(stderr, "call cmdin->iob_handle = %8.8lx\n", cmdin->iob_handle);
fprintf(stderr, "call cmdout->iob_handle = %8.8lx\n", cmdout->iob_handle);
fprintf(stderr, "call cmderr->iob_handle = %8.8lx\n", cmderr->iob_handle);
	if (detached)
	    fflush(stderr);
#endif

    if (cmdout != stdout)
    {
	fflush(cmdout);

	vbofparm.vbof.value = -2L;
	svcIoInBlockP(cmdout->iob_handle, NULL, 0, &vbofparm);
    }

    old_stdin = devlist.dlstdin.src;
    runparm.arglist.buffer = pnt1;
    runparm.arglist.bfrlen = runparm.arglist.strlen = strlen(pnt1);
    svcSchSetLevel(8);	/* No software interrupts now */
    runqab.qab_amount = -1;
    if (detached)
	runparm.devlist.buffer = &devlist_det;
    else
	runparm.devlist.buffer = &devlist;

    if (((runrtn = svcIoRun(&runqab)) < 0 ||
	 (runrtn = runqab.qab_error) < 0) &&
          extbgn != NULL &&
	  runrtn == ER_FILNF)
    {
        while ((ext = *++extpnt) != NULL)
        {
            strcpy(extbgn, ext);
	    if (strcmp(ext, ".BAT") == 0)
	    {
		old_echo = echo;
		svcSchSetLevel(0);	/* Software interrupts now */
		old_stdin = devlist.dlstdin.src;
		svcSysSetPName(keyword);
		if (bat_ctlc || (batbat(runname) == TRUE))
		{
		    free(pnt1);		/* Give up any extra memory */
		    echo = TRUE;
		    if (cmdin != old_cmdin)
		    {
			if (extend_batch)
			{
			    fclose(cmdin);
			    cmdin = old_cmdin;	/* Implied CALL of bat file */
			}
			else
			{
			    if (old_cmdin != stdin)
				fclose(old_cmdin); /* Don't return to old bat */
			    else
			    {
				fclose(cmdin);
				cmdin = old_cmdin;	/* Implied CALL of bat file */
			    }
			}
		    }
		    if (cmderr != old_cmderr)
		    {
			fclose(cmderr);
			cmderr = old_cmderr; /* Redir on stderr does */
			cmdout = old_cmdout; /*  stdout to match */
			devlist.dlstdout.src = cmdout->iob_handle;
			devlist.dlstderr.src = cmderr->iob_handle;
		    }
		    if (cmdout != old_cmdout)
		    {
			fclose(cmdout);
			cmdout = old_cmdout;
			devlist.dlstdout.src = cmdout->iob_handle;
		    }
		    echo = old_echo;
		    devlist.dlstdin.src = old_stdin;
		    svcSysSetPName(prgname);
		    return;
		}
                svcSysSetPName(prgname);
		svcSchSetLevel(8);	/* No software interrupts now */
	    }
	    else
	    {
		svcSchSetLevel(8);	/* No software interrupts now */
		old_stdin = devlist.dlstdin.src;
runqab.qab_amount = -1;
        	if (((runrtn = svcIoRun(&runqab)) >= 0 &&
                     (runrtn = runqab.qab_error) >= 0) ||
                      runrtn != ER_FILNF)
                    break;
		devlist.dlstdin.src = old_stdin;
	    }
        }
    }
    devlist.dlstdin.src = old_stdin;
    if (detached)
    {
	fflush(cmdout);			// Flush buffer and reset for end
	_fsetup(cmdout);		//  of the file
	fseek(cmdout, 0L, SEEK_END);	// Must be sure to be at end
	fflush(cmderr);			// Flush buffer and reset for end
	_fsetup(cmderr);		//  of the file
	fseek(cmderr, 0L, SEEK_END);	// Must be sure to be at end
    }
#ifdef DEBUG
fprintf(stderr, "return devlist.dlstdin.src  = %8.8lx\n", devlist.dlstdin.src);
fprintf(stderr, "return devlist.dlstdout.src = %8.8lx\n", devlist.dlstdout.src);
fprintf(stderr, "return devlist.dlstderr.src = %8.8lx\n", devlist.dlstderr.src);
fprintf(stderr, "return cmdin->iob_handle = %8.8lx\n", cmdin->iob_handle);
fprintf(stderr, "return cmdout->iob_handle = %8.8lx\n", cmdout->iob_handle);
fprintf(stderr, "return cmderr->iob_handle = %8.8lx\n", cmderr->iob_handle);
	if (detached)
	    fflush(stderr);
#endif
    if (runrtn < 0)
    {
        if (runrtn == ER_FILNF || runrtn == ER_DIRNF)
	    if (gcp_dosquirk)
        	fputs("Bad command or filename\n", cmdout);
	    else
		fputs("? Invalid command\n", cmdout);
        else
            fileerr(runname, runrtn);
        errrtn = runrtn;		/* save any return value */
        pipeseen = FALSE;
	if (detached)
	    fflush(cmdout);
        svcSchSetLevel(0);		/* Allow software interrupts now */
    }
    else
    {
	temp_childcnt = ++childcnt;
        cproc[temp_childcnt - 1].c_state = C_RUN;   /* Remember its state */
        cproc[temp_childcnt - 1].c_pid = runqab.qab_amount & 0xFFFF0FFFL;
        svcSchSetLevel(0);		/* Allow software interrupts now */
        if (!pipeseen)			/* Don't wait if doing a pipe */
	{
            svcSchSuspend(&childcnt, 0xFFFFFFFFL);
            errrtn = cproc[temp_childcnt - 1].c_status;	/* Save return value from child */
	    resetscrn();
	}
    }
    if ((batch || local_batch) && error_pause && (errrtn != 0))
    {
	errpause();			// If Pause batch on error
    }

    if (cmdin != old_cmdin)
    {
        fclose(cmdin);
        cmdin = old_cmdin;
    }
    if (cmderr != old_cmderr)
    {
        fclose(cmderr);
        cmderr = old_cmderr;		/* Redir on stderr does */
        cmdout = old_cmdout;		/*  stdout to match */
    }
    if (cmdout != old_cmdout)
    {
        fclose(cmdout);
        cmdout = old_cmdout;
    }
    if (cmdout != stdout)
    {
	fflush(cmdout);			// Clear our buffer
	_fsetup(cmdout);
	fseek(cmdout, 0L, SEEK_END);	/* Must be sure to be at end of file */
    }

    devlist.dlstdin.src = old_stdin;
    devlist.dlstdout.src = cmdout->iob_handle;
    devlist.dlstderr.src = cmderr->iob_handle;
    free(pnt1);				/* Give up any extra memory */

    if ((runrtn > 0) && (runrtn & 0xF000) == (IT_BATCH*0x1000))
        exit(0);			/* This is to be compatible with MS-DOS */

    return;
}
