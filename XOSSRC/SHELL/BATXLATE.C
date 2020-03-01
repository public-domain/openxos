//------------------------------------------------------------------------------
//
//  BATXLATE.C - Translate string routine for BATCH
//
//  Written by Bruce R. Nevins
// 
//  Edit History:
//  -------------
//  05/13/91(brn) - Modified to support extended batch
//  06/13/91(brn) - Fix bug leaving digit in string for % args
//  03/08/94(brn) - Add %* command to copy the entire alrgument list
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
#include <XOSERR.H>
#include <XOSSVC.H>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

/*
 * Function to process Translate a string,
 *  srcstr to dststr, expanding all % variables.
 */

struct firstblk * batxlate(struct firstblk *dststr, struct firstblk *srcstr)
{
    register char tempchr;
    register char chr;
    char envstr[128];
    int temp;
    char envname[LINESIZE+2];
    char *tempstr;
    char  templine[LINESIZE+2];


    chr = blkgetc(srcstr);
    while ((chr != '\0') && isspace(chr))
    {
        dststr = blkputc(dststr, chr);
        chr = blkgetc(srcstr);
    }

    if (chr == '@')
    {
        chr = blkgetc(srcstr);
        tempnoecho = TRUE;
    }
    else
        tempnoecho = FALSE;

    while (chr != '\0')
    {
        while (!(chr == '%' || chr == '\0'))
        {
            dststr = blkputc(dststr, chr);
            chr = blkgetc(srcstr);
        }

        if (chr == '\0')        /* If EOL we are done */
        {
            dststr = blkputc(dststr, chr);
            return (dststr);
        }

/*
 * Here if % is seen
 */
        chr = blkgetc(srcstr);
        if (isdigit(chr))
        {
            tempstr = templine;

	    if (extend_batch)
	    {
        	while (isdigit(chr))        /* Gather all the digits */
        	{
                    *tempstr++ = chr;
                    chr = blkgetc(srcstr);/* Get the next possible digit */
        	}
	    }
	    else
	    {
		*tempstr++ = chr;
        	chr = blkgetc(srcstr);
	    }

            *tempstr = '\0';            /* End the string */

            temp = atoi(templine);
            if (temp < argcounter)
            {
/* printf("argpointer[%d] = %s\n", temp, argpointer[temp]); */
                tempstr = argpointer[temp];
                while ((tempchr = *tempstr++) != '\0')
                    dststr = blkputc(dststr, tempchr);
            }
        }
        else
        {
            if (chr == '*')
            {
		for (temp = 1; temp < argcounter; temp++)
		{
                    tempstr = argpointer[temp];

                    while ((tempchr = *tempstr++) != '\0')
                	dststr = blkputc(dststr, tempchr);

		    if ((temp + 1) != argcounter)
			dststr = blkputc(dststr, ' ');
		}
                chr = blkgetc(srcstr);
	    }
            else if (chr != '%')
            {
                tempstr = envname;
                while (!(chr == '%' || chr == '\0'))
                {
                    *tempstr++ = chr;
                    chr = blkgetc(srcstr);
                }
                *tempstr = '\0';        /* End the string */
                /* Do environment call here */
		if ((temp = svcSysFindEnv(0, envname, NULL, envstr, sizeof(envstr),
			NULL)) >= 0)
		{
		    temp = 0;
		    while ((chr = envstr[temp++]) != '\0')
		        dststr = blkputc(dststr, chr);
		}
                chr = blkgetc(srcstr);
            }
            else
            {
                dststr = blkputc(dststr, chr);
                chr = blkgetc(srcstr);
            }
          }
    }
    dststr = blkputc(dststr, chr);
    return (dststr);
}

