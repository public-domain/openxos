//------------------------------------------------------------------------------
//
//  INICMD.C - Execute commands from filen named in environment variable
//
//  Written by Bruce R. Nevins
//
//  Edit History:
//  -------------
//  07/23/92(brn) - Changed to do commands inline
//  07/26/92(brn) - Preserve cmdin durring call to batbat
//  03/07/93(brn) - Close batch file on return
//  04/01/94(brn) - Fixed to not close cmdin if no batch file ran
//
//------------------------------------------------------------------------------

#include <CTYPE.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <XOSERR.H>
#include <XOSSVC.H>
#include <XOS.H>
#include <XOSTRM.H>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

void inicmd(char *env)
{
    char strbuf[256];           // String buffer
    char chr, *cp;
    FILE *old_cmdin;
    long old_stdin;

    if (expndarg != NULL)
        expndarg = givechain(expndarg); // Give up old command line

    strcpy(runname, (svcSysFindEnv(0, env, NULL, strbuf, 256, NULL) < 0)?
        "SHELLINI.BAT": strbuf);

    cp = runname;
    while ((chr = *cp++) != '\0')	// Build dummy arguments
	expndarg = blkputc(expndarg, chr);

    old_stdin = devlist.dlstdin.src;
    old_cmdin = cmdin;
    batbat(runname);
    if (cmdin != stdin)
	fclose(cmdin);			// If it equals stdin no batch file
    cmdin = old_cmdin;			//  ran, so there is no fie to close
    devlist.dlstdin.src = old_stdin;
    return;
}

