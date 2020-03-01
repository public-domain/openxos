//------------------------------------------------------------------------------
// CMDHIST.C
//
// Written by Bruce R. Nevins
// 
// Edit History:
// -------------
// 01/01/87(brn) - Created first version
// 08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
// 08/27/96(brn) - Add fflush if detached process
//------------------------------------------------------------------------------*

#include <STDIO.H>
#include <STDLIB.H>
#include <XOS.H>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

/*
 * Function to process HISTORY command
 */

int cmdhist(void)

{
    char *pnt;
    unsigned long newsize;
    register j;

    if ((pnt = firstarg) != NULL)
    {
        if (!singlearg)                 /* Must have one argument */
        {
            fprintf(cmderr, "? %s: Too many arguments\n", prgname);
	    if (detached)
		fflush(cmderr);
            return (TRUE);
        }
        newsize = atol(pnt);            /* Get the size of the new buffer */
        if (newsize > HISTMAX)
        {
            fprintf(cmderr, "? %s: History size too large, Maximum = %d\n",
			 prgname, HISTMAX);
	    if (detached)
		fflush(cmderr);
            return (TRUE);
        }
        
        if (newsize == 0)
        {
            fprintf(cmderr, "%% %s: Command history disabled\n", prgname);
	    if (detached)
		fflush(cmderr);
            for(;;)
            {
                if (hstfrst == NULL)    /* check if last one */
                    break;
                hsttmp = hstfrst->a.nextlst; /* Get next buffer to free */
                givechain(hstfrst);     /* Free buffer */
                hstfrst = hsttmp;       /* Move pointer */
            }
            hstlast = NULL;             /* Make sure last pointer is clear */
            histcnt = 0;                /* Clear count of blocks */
        }
        else
            while (newsize < histcnt)
            {
                hsttmp = hstfrst->a.nextlst; /* Point to new first block */
                hsttmp->a.lastlst = NULL; /* Clear last pointer in new block */
                givechain(hstfrst);     /* Give up old first block */
                hstfrst = hsttmp;       /* New first pointer */
                hsttmp = hstlast;       /* Point temp at last block */
                histcnt--;              /* Reduce count of blocks */
                topseen = FALSE;        /* Flag for top of command list */
                botseen = TRUE;         /* Flag for bottom of command list */
            }

        histlimit = newsize;    
    }
    else
    {
        if (hstfrst != NULL)
        {
            fprintf(cmdout, "Command History (%d):\n", histlimit);
            hsttmp = hstfrst;

            for(j = 1; j <= histcnt; j++)
            {
                fprintf(cmdout, "%d: ", j + histlow);
                blkinic(hsttmp);        /* Start from beginning */
                blkputs(hsttmp, cmderr);
                fputc('\n', cmderr);
                if ((hsttmp = hsttmp->a.nextlst) == NULL)
                    break;
            }
            hsttmp = hstlast;
            topseen = FALSE;    /* Flag for top of command list */
            botseen = TRUE;     /* Flag for bottom of command list */
	    if (detached)
		fflush(cmdout);
        }
        else if (histlimit == 0)
	{
            fprintf(cmderr, "Command history disabled\n");
	    if (detached)
		fflush(cmderr);
	}
	else
	{
            fprintf(cmderr, "Command History (%d):\n", histlimit);
	    if (detached)
		fflush(cmderr);
	}
    }
    return (TRUE);
}

