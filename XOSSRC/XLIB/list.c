//**************************************
// Routines to generate listing for XLIB
//**************************************
// Written by John Goltz
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

#include <STDIO.H>
#include <STDLIB.H>
#include <CTYPE.H>
#include <ERRNO.H>
#include <STRING.H>
#include <TIME.H>
#include <XCSTRING.H>
#include <XOSTIME.H>
#include <XOS.H>
#include <XOSERRMSG.H>
#include <XOSSVC.H>
#include "XLIB.H"
#include "XLIBEXT.H"

// Local data

int    linecnt;			// Line count
char   firsthead;		// TRUE if first header not output yet
FILE  *lstfp;
time_d ourtime;
char   lsttime[40];

//***************************************
// Function: list - Generate listing file
// Nothing
//***************************************

void list(void)

{
    struct et *etp;
    int    left;
    int    need;
    char  *pnt;

    if (listfsp)			// Was a listing file specified?
    {					// Yes
	if ((lstfp=fopen(listfsp, "w")) == NULL) // Open the file
	    femsg(prgname, -errno, listfsp);
    }
    else
	lstfp = stdout;			// No file specified - use stdout
    svcSysDateTime(2, &ourtime);	// Get current date and time
    ddt2str(lsttime, "%H:%m:%s %D-%3n-%2y", (time_dz *)&ourtime);
    linecnt = 0;
    firsthead = TRUE;
    pageno = 0;
    if ((etp = ethead.et_next) != 0)
    {
        char buffer[40];

	do
	{   chkhead(4);			// Output header if needed
            sdt2str(buffer, "%z%H:%m:%s %D-%3n-%2y", (time_sz *)&etp->et_time);
	    fprintf(lstfp, "Module:  %s\nCreated: %-33s Size: %ld\n"
                    "Entries:", etp->et_modname, buffer, etp->et_modsize);
	    left = 64;
            pnt = etp->et_entlist;
	    while ((pnt = entname(pnt)) != 0)// Get next entry name
	    {
		need = (symsize > 16)? 32: 16;
		if ((left -= need) < 0)	// Room for this symbol?
		{
		    left = 64 - need;	// Reset count
		    fputc('\n', lstfp);	// End current line
		    chkhead(1);		// Output header if needed
		    fputs("        ", lstfp); // Start next line
		}
		fprintf(lstfp, (need == 16)? " %-16s": " %-33s", symbuf); 
	    }
	    fputc('\n', lstfp);
	    if (--linecnt > 4)
	    {
		fputc('\n', lstfp);
		--linecnt;
	    }
	} while ((etp = etp->et_next) != 0);
    }
    fputc('\f', lstfp);			// Output final form-feed
    if (lstfp != stdout)		// Do we need to close the listing?
	if (fclose(lstfp) < 0)		// Yes - close it
	    femsg(prgname, -errno, listfsp); // If error
}

//***************************************************
// Function: chkhead - Generate page header if needed
// Returned: Nothing
//***************************************************

void chkhead(
    long amnt)

{
    if ((linecnt -= amnt) <= 0)		// Need header now?
    {
	linecnt = LISTPGSZ;
	if (firsthead)
	    firsthead = FALSE;
	else
	    fputc('\f', lstfp);
	fprintf(lstfp, "Library listing for file %-45s Page %d\nListed on %s\n\n",
		libname->nb_name, ++pageno, lsttime);
    }
}

//**************************************************************
// Function: entname - Get next entry name
// Returned: Pointer to name obtained if normal or NULL if error
//**************************************************************

char *entname(
    char *pnt)

{
    char *cpnt;
    char  chr;

    cpnt = symbuf;
    symsize = 0;
    if (*pnt)
    {
	while ((chr = *pnt++) != 0 && !(chr & 0x80))
	{
	    if (symsize <= SYMMAXSZ)
	    { 
		*cpnt++ = chr;
		++symsize;
	    }
	}
	if (chr)			// Normal termination?
	{
	    *cpnt++ = chr & 0x7F;	// Yes - store final character
	    ++symsize;
	}
	*cpnt = '\0';
	return (pnt);
    }
    else
	return (NULL);
}
