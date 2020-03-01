//------------------------------------------------------------------------------
// CMDCD.C - Routines for CD command
// 
// Written by Bruce R. Nevins
// 
// Edit History:
// -------------
// 11/16/89(brn) - Remove support for / in path
// 03/02/90(brn) - Make CD command show dos or XOS device name
// 10/09/91(jrg) - Change to support Z: instead of DSK: for DOS compatibility
// 03/22/92(brn) - Add use of global.c module like all the utils
// 08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
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

#include <STDIO.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <GLOBAL.H>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

struct pathparm pathparm =
{   {(PAR_SET|REP_HEXV), 4, IOPAR_FILOPTN, 0},
    {(PAR_GET|REP_STR ), 0, IOPAR_FILSPEC, NULL, 0, FILESPCSIZE, 0}
};

/*
 * Function to process CD command
 */

int cmdcd(void)
{
    uchar   *pnt;
    uchar   *pnt2;
    long     rtn;
    type_qab pathqab;

    char buffer[FILESPCSIZE];	/* Buffer for current path string */
    char dev[FILESPCSIZE];	/* Device Name */
    unsigned char path[FILESPCSIZE]; /* Path Name */

    dev[0] = '\0';
    path[0] = '\0';

    if ((pnt = (unsigned char *)firstarg) != NULL)
    {
        if (!singlearg)			/* Must have one argument */
        {
            fprintf(cmderr, "? Too many arguments\n");
	    if (detached)
		fflush(cmderr);
            return (TRUE);
        }
        pathparm.filoptn.value = 0;	/* Assume do not want display value */
        pnt = (unsigned char *)firstarg;/* Find end of argument */
        while (*pnt++)
            ;
        pnt -= 2;			/* Backup to last character */
        if (*pnt == ':')		/* Want to just display path? */
            pathparm.filoptn.value = (gcp_dosdrive)? FO_DOSNAME|FO_PATHNAME:
                    FO_VOLNAME|FO_PATHNAME; // Yes
        else if (*pnt != '\\')		/* No - need slash at end? */
        {
            ++pnt;
            *pnt++ = '\\';		/* Yes */
            *pnt = '\0';
        }
    }
    else				/* No - show current path for */
    {					/*   default disk */
        firstarg = "Z:";
        pathparm.filoptn.value = (gcp_dosdrive)? FO_DOSNAME|FO_PATHNAME:
                FO_VOLNAME|FO_PATHNAME;
    }
    pathqab.qab_func = QFNC_WAIT|QFNC_PATH;
    pathqab.qab_vector = 0;
    pathqab.qab_option = 0;
    pathqab.qab_buffer1 = firstarg;
    pathqab.qab_buffer2 = NULL;
    pathqab.qab_parm = &pathparm;
    pathparm.filspec.buffer = buffer;
    if ((rtn=svcIoQueue(&pathqab)) < 0 || (rtn = pathqab.qab_error) < 0)
    {					/* Do path SVC */
        cmnderr(NULL, rtn);		/* If error */
        return (TRUE);
    }
    if (pathparm.filoptn.value != 0)	/* OK - display result if we should */
    {
        pnt = (unsigned char *)buffer;
        if (*pnt == FS_RXOSNAME || *pnt == FS_RVOLNAME || *pnt == FS_RDOSNAME
		|| *pnt == FS_XOSNAME || *pnt == FS_VOLNAME
		|| *pnt == FS_DOSNAME)
        {
            pnt++;
            pnt2 = (unsigned char *)dev;
            while (*pnt != '\0' && *pnt != FS_PATHNAME)
                *(pnt2++) = *(pnt++);
            *pnt2 = '\0';
        }
        if (*pnt == FS_PATHNAME)
        {
            pnt++;
            pnt2 = path;
            while (*pnt != '\0')
                *(pnt2++) = *(pnt++);
            *pnt2 = '\0';
        }
        fprintf(cmdout, "%s%s\n", dev, path);	/* Print the path */
	if (detached)
	    fflush(cmdout);
    }
    return (TRUE);
}
