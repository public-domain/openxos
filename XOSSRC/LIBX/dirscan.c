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
#include <XOSSTR.H>
#include <XOSERR.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <DIRSCAN.H>

static const char noelipmsg[] = "Device does not support ... searches";

static int   _dirscan(char *filespec, int recurse, DIRSCANDATA *dsd);

static char  *checkdfltext(char *filespec, char **elipp, DIRSCANDATA *dsd);

static int   compdir(DIRNAME *a, DIRNAME *b, DIRSCANDATA *dsd);
static int   doelip(char *filename, char *elip, DIRSCANDATA *dsd);
static void *dsgetmem(size_t size, DIRSCANDATA *dsd);
static int   parsestr(long error, DIRSCANDATA *dsd);
static int   level;

//**************************************************
// Function: dirscan - Scans directory and processes
//		all files in directory
// Returned: TRUE if normal, FALSE if error
//**************************************************

int dirscan(
    char        *filespec,
    DIRSCANDATA *dsd)

{
    RMTDATA *rmt;
    RMTDATA *next;
    int      rtn;

    // First initial our main data structure, allocate buffer, etc.

    dsd->changed = 0;
    dsd->devnamelen = 0;		// Length of devname string
    dsd->devname[0] = '\0';		// devname string

    if (dsd->parmlist->filoptn.value & (FO_NODENAME|FO_NODENUM))
    {
	if ((dsd->rmtdata.nodename = dsd->rmtdata.nodenamebase =
		(char *)dsgetmem(NODENAMESIZE, dsd)) == NULL)
	    return (FALSE);
	dsd->rmtdata.nodename[0] = 0;
    }
    dsd->rmtdata.nodenamelen = 0;

    dsd->rmtdata.rmtdevnamelen = 0;	// Length of rmtdevname string
    dsd->rmtdata.rmtdevname[0] = '\0';	// rmtdevname string

    if (dsd->parmlist->filoptn.value & FO_PATHNAME)
    {
	if ((dsd->pathname = dsd->pathnamebase =
		(char *)dsgetmem(PATHNAMESIZE, dsd)) == NULL)
	    return (FALSE);	
	dsd->pathname[0] = 0;
    }
    else
	dsd->pathname = dsd->pathnamebase = NULL;
    dsd->pathnamelen = 0;

    if (dsd->parmlist->filoptn.value & FO_PATHDOS)
    {
	if ((dsd->pathdos = dsd->pathdosbase =
		(char *)dsgetmem(PATHDOSSIZE, dsd)) == NULL)
	    return (FALSE);
	dsd->pathdos[0] = 0;
    }
    else
	dsd->pathdos = dsd->pathdosbase = NULL;
    dsd->pathdoslen = 0;

    if (dsd->parmlist->filoptn.value & FO_FILENAME)
    {
	if ((dsd->filename = dsd->filenamebase =
		(char *)dsgetmem(FILENAMESIZE, dsd)) == NULL)
	    return (FALSE);
	dsd->filename[0] = 0;
    }
    else
	dsd->filename = dsd->filenamebase = NULL;
    dsd->filenamelen = 0;

    if (dsd->parmlist->filoptn.value & FO_FILEDOS)
    {
	if ((dsd->filedos = dsd->filedosbase =
		(char *)dsgetmem(FILEDOSSIZE, dsd)) == NULL)
	    return (FALSE);
	dsd->filedos[0] = 0;
    }
    else
	dsd->filedos = dsd->filedosbase = NULL;
    dsd->filedoslen = 0;

    // Initialize the parameter lists we need

    if ((dsd->parmlist->filspec.buffer =
	    (char *)dsgetmem(FILSPECSIZE, dsd)) == NULL)
	return (FALSE);
    dsd->parmlist->filspec.bfrlen = FILSPECSIZE;

    dsd->diropnparm.filoptn.desp = PAR_SET|REP_HEXV;
    dsd->diropnparm.filoptn.size = 4;
    dsd->diropnparm.filoptn.index = IOPAR_FILOPTN;
    dsd->diropnparm.filoptn.value = dsd->parmlist->filoptn.value &
        (FO_XOSNAME|FO_DOSNAME|FO_VOLNAME|FO_NODENAME|FO_RXOSNAME|FO_RDOSNAME|
        FO_RVOLNAME|FO_PATHNAME|FO_FILENAME);
    dsd->diropnparm.filspec.desp = PAR_GET|REP_STR;
    dsd->diropnparm.filspec.index = IOPAR_FILSPEC;
    dsd->diropnparm.filspec.buffer = dsd->parmlist->filspec.buffer;
    dsd->diropnparm.filspec.bfrlen = dsd->parmlist->filspec.bfrlen;
    dsd->diropnparm.srcattr.desp = PAR_SET|REP_HEXV;
    dsd->diropnparm.srcattr.size = 2;
    dsd->diropnparm.srcattr.index = IOPAR_SRCATTR;
    dsd->diropnparm.srcattr.value = A_DIRECT;
    dsd->diropnparm.devsts.desp = PAR_GET|REP_HEXV;
    dsd->diropnparm.devsts.size = 4;
    dsd->diropnparm.devsts.index = IOPAR_DEVSTS;
    dsd->diropnparm.end = 0;

    dsd->dirposparm.iopos.desp = PAR_SET|REP_HEXV;
    dsd->dirposparm.iopos.size = 4;
    dsd->dirposparm.iopos.index = IOPAR_ABSPOS;
    dsd->dirposparm.iopos.value = 0;
    dsd->dirposparm.devsts.desp = PAR_GET|REP_HEXV;
    dsd->dirposparm.devsts.size = 4;
    dsd->dirposparm.devsts.index = IOPAR_DEVSTS;
    dsd->dirposparm.end = 0;

    dsd->dirsrchparm.filoptn.desp = PAR_SET|REP_HEXV;
    dsd->dirsrchparm.filoptn.size = 4;
    dsd->dirsrchparm.filoptn.index = IOPAR_FILOPTN;
    dsd->dirsrchparm.filoptn.value = FO_FILENAME;
    dsd->dirsrchparm.filspec.desp = PAR_GET|REP_STR;
    dsd->dirsrchparm.filspec.index = IOPAR_FILSPEC;
    dsd->dirsrchparm.filspec.buffer = dsd->parmlist->filspec.buffer;
    dsd->dirsrchparm.filspec.bfrlen = dsd->parmlist->filspec.bfrlen;
    dsd->dirsrchparm.srcattr.desp = PAR_SET|REP_HEXV;
    dsd->dirsrchparm.srcattr.size = 2;
    dsd->dirsrchparm.srcattr.index = IOPAR_SRCATTR;
    dsd->dirsrchparm.srcattr.value = A_DIRECT;
    dsd->dirsrchparm.dirhndl.desp = PAR_SET|REP_HEXV;
    dsd->dirsrchparm.dirhndl.size = 4;
    dsd->dirsrchparm.dirhndl.index = IOPAR_DIRHNDL;
    dsd->dirsrchparm.end = 0;

    dsd->dirqab.qab_func = QFNC_WAIT|QFNC_DEVPARM;
    dsd->dirqab.qab_vector = 0;
    dsd->dirqab.qab_option = O_REPEAT;
    dsd->dirqab.qab_buffer1x = 0;
    dsd->dirqab.qab_parm = &dsd->dirsrchparm;
    dsd->dirqab.qab_parmx = 0;

    dsd->fileqab.qab_func = QFNC_WAIT | (dsd->function);
    dsd->fileqab.qab_vector = 0;
    dsd->fileqab.qab_buffer1x = 0;
    dsd->fileqab.qab_parm = dsd->parmlist;
    dsd->fileqab.qab_parmx = 0;

    dsd->newfilespec = NULL;

    dsd->level = level = 0;

    // Do the work

    rtn = _dirscan(filespec, TRUE, dsd);

    // Give up the memory we allocated

    free((char near *)(dsd->parmlist->filspec.buffer));
    rmt = &(dsd->rmtdata);
    do
    {
	if (rmt->nodenamebase != NULL)
	{
	    if (rmt->nodename != rmt->nodenamebase)
		free(rmt->nodename);
	    free(rmt->nodenamebase);
	}
	next = rmt->next;
	if (rmt != &(dsd->rmtdata))
	    free(rmt);
	else
	    rmt->nodename = rmt->nodenamebase = NULL;
    } while ((rmt = next) != NULL);
    if (dsd->pathnamebase != NULL)
    {
	if (dsd->pathname != dsd->pathnamebase)
	    free(dsd->pathname);
	free(dsd->pathnamebase);
	dsd->pathname = dsd->pathnamebase = NULL;
    }
    if (dsd->pathdosbase != NULL)
    {
	if (dsd->pathdos != dsd->pathdosbase)
	    free(dsd->pathdos);
	free(dsd->pathdosbase);
	dsd->pathdos = dsd->pathdosbase = NULL;
    }
    if (dsd->filenamebase != NULL)
    {
	if (dsd->filename != dsd->filenamebase)
	    free(dsd->filename);
	free(dsd->filenamebase);
	dsd->filename = dsd->filenamebase = NULL;
    }
    if (dsd->filedosbase != NULL)
    {
	if (dsd->filedos != dsd->filedosbase)
	    free(dsd->filedos);
	free(dsd->filedosbase);
	dsd->filedos = dsd->filedosbase = NULL;
    }
    if (dsd->newfilespec != NULL && dsd->newfilespec != (char *)-1)
    {
	free(dsd->newfilespec);
	dsd->newfilespec = NULL;
    }
    return (rtn);
}

//***************************************************
// Function: _dirscan - Internal function for dirscan
// Returned: TRUE if normal, FALSE if error
//***************************************************

static int _dirscan(
    char        *filespec,
    int          recurse,
    DIRSCANDATA *dsd)

{
    char *elip;
    long  rtn;
    char  chr;

    dsd->namepnt = (char *)(dsd->parmlist->filspec.buffer);
    dsd->parmlist->filspec.buffer[0] = '\0'; // In case we don't get a string
    if ((elip = strstr(filespec, "...\\")) != NULL)
    {
        if (elip != filespec && elip[-1] != '\\' && elip[-1] != ':')
        {
            dsd->errormsg("Illegal use of ... in file specification", 0);
            return (FALSE);
        }

        if (dsd->hvellip != NULL)
            (dsd->hvellip)(dsd);	// Tell caller we have ellipsis

        strcpy(elip, elip+4);		// Remove elipisis from name

        // Here if have valid ellipsis somewhere in the path - if it is not
        //   the last item we never want to list any files at the level of
        //   the ellipsis - thus we just directly call the ellipsis processor

        if (strchr(elip, '\\'))		// Any directries after the ...?
        {				// Yes - must do this level of search
            chr = *elip;		// Truncate file specification after
            *elip = '\0';		//   directory to search here
            if ((rtn = svcIoOpen(O_ODF, filespec, &dsd->diropnparm)) < 0)
            {				// Open the directory
                *elip = chr;		// If error
                dsd->errormsg(filespec, rtn);
                return (FALSE);
            }
            dsd->parmlist->dirhndl.value = rtn;
            *elip = chr;		// Restore file specification
            if (!parsestr(0, dsd))	// Store device name and path
                return (FALSE);

	    if ((filespec = checkdfltext(filespec, &elip, dsd)) == NULL)
		return (FALSE);

            return (doelip(filespec, elip, dsd));
        }
        recurse = TRUE;
    }

    // We get here if we do not have ellipsis or if the ellipsis is the last
    //   thing in the path

    if ((rtn = svcIoOpen(O_ODF, filespec, &dsd->diropnparm)) < 0)
    {					// Open the directory
        dsd->errormsg(filespec, rtn); // If error, display message
        return (recurse);
    }

    if ((filespec = checkdfltext(filespec, &elip, dsd)) == NULL)
	return (FALSE);

    dsd->parmlist->dirhndl.value = rtn;
    if (!parsestr(0, dsd))		// Store device name and path
        return (FALSE);
    dsd->fileqab.qab_buffer1 = filespec;
    dsd->fileqab.qab_option = 0;

    if (dsd->repeat)			// Should we do this the quick way?
    {					// Yes
        dsd->fileqab.qab_option |= O_REPEAT;
        do
        {
            dsd->parmlist->filspec.buffer[0] = '\0';
            if ((rtn = svcIoQueue(&dsd->fileqab)) < 0) // Collect some names
            {
                dsd->errormsg(filespec, rtn);
                svcIoClose(dsd->parmlist->dirhndl.value, 0);
                return (FALSE);
            }
            if (dsd->fileqab.qab_error == ER_FILNF)
                break;
            dsd->namepnt = (char *)(dsd->parmlist->filspec.buffer);
					// Point to device and path name
            if (!parsestr(dsd->fileqab.qab_error, dsd))
                return (FALSE);
        } while (dsd->fileqab.qab_amount & 0x80000000L);
    }
    else				// Get full file info
    {
        for (;;)
        {
            if ((rtn = svcIoQueue(&dsd->fileqab)) < 0)
            {
                dsd->errormsg(filespec, rtn);
                return (FALSE);
            }
            if (dsd->fileqab.qab_error == ER_FILNF) // Normal end of scan?
                break;			// Yes
            dsd->namepnt = (char *)(dsd->parmlist->filspec.buffer);
            if (dsd->fileqab.qab_error < 0 && dsd->namepnt[0] == '\0')
            {				// Have error with no file spec?
                dsd->errormsg("", dsd->fileqab.qab_error); // Yes - report it
                return (FALSE);
            }
            if (!parsestr(dsd->fileqab.qab_error, dsd))
                return (FALSE);
        }
    }

    if (elip != NULL)
        doelip(filespec, elip, dsd);
    svcIoClose(dsd->parmlist->dirhndl.value, 0);
    dsd->parmlist->dirhndl.value = 0;
    return (TRUE);
} 

//*******************************************************
// Function: parsestr - Parse returned file specification
//		into device path and name parts
// Returned: TRUE if normal, FALSE if error
//*******************************************************

// This routine assumes that the items in the returned string will be in
//   the following order (items on the same line are equivilant and it is
//   assumed that at most one from each line will be seen, except that the
//   node name and remote device name items can be repeated as a pair any
//   number of times).  The FS_FILENAME and FS_VERSION items are combined
//   to give the file name.
//	FS_RTDNAME  FS_XOSNAME  FS_DOSNAME  FS_VOLNAME	>  Device name
//	FS_NODENAME FS_NODENUM				>  Node name
//	FS_RXOSNAME FS_RDOSNAME FS_RVOLNAME		>  Remote device name
//	.........
//	FS_PATHNAME					> Path name (long)
//	FS_FILENAME					> File name (long)
//	FS_FILEDOS					> File name (8x3)
//	FS_VERSION
//	FS_ATTR 					> Attribute byte
//   If any item preceeding the item just processed is seen, this terminates
//   the current item and the current item is returned.  If a new device name
//   or path element is seen, it replaces the current value for that element
//   and clears all following elements to a null value.  Thus a new FS_RXOSNAME
//   value will replace the current FS_RXOSNAME value and set the FS_PATHNAME
//   value to null.  The FS_XOSNAME and FS_NODENAME value will be unaffected.

static int parsestr(
    long         error,		// Error code from directory scan
    DIRSCANDATA *dsd)		// Pointer to data structure

{
    char *cx;
    char *pnt;
    int   size;
    char  chr;

    dsd->error = error;
    while ((chr = *dsd->namepnt++) != '\0')
    {
        switch ((uchar)chr)
        {
         case FS_XOSNAME:
         case FS_DOSNAME:
         case FS_VOLNAME:
            if (dsd->devnamelen != 0)	// Do we have a device name now?
            {				// Yes - see if it has changed
                cx = dsd->namepnt;
                pnt = dsd->devname;
                size = 0;
                while ((chr = *dsd->namepnt) != '\0' &&
                        ((((uchar)chr) == FS_ESC) || (((uchar)chr) < FS_MIN)))
                {
                    dsd->namepnt++;
                    if (((uchar)chr) == FS_ESC)
                        chr = *dsd->namepnt++;
                    if (*pnt++ == chr)
                        ++size;
                    else
                    {
                        size = 0;
                        break;
                    }
                }
                if (dsd->devnamelen == size) // Is it the same?
                    break;		// Yes - continue
                dsd->namepnt = cx;	// No - process the new name
            }
            if (dsd->changed)		// Have any other unreported changes?
            {
                if (!dsd->func(dsd))	// Yes - call the caller's function
                    return (FALSE);
                dsd->changed = 0;
            }
            dsd->devnamelen = 0;
            dsd->rmtdata.nodenamelen = 0;
            dsd->rmtdata.rmtdevnamelen = 0;
            dsd->pathnamelen = 0;
            if (dsd->elippos != NULL)
            {
                dsd->errormsg(noelipmsg, 0);
                return (FALSE);
            }
            pnt = dsd->devname;
            while ((chr = *dsd->namepnt) != '\0' && ((((uchar)chr) == FS_ESC) ||
                    (((uchar)chr) < FS_MIN)))
            {
                dsd->namepnt++;
                if (((uchar)chr) == FS_ESC)
                    chr = *dsd->namepnt++;
                if (++dsd->devnamelen < 12)
                    *pnt++ = chr;
            }
            *pnt = '\0';
            dsd->changed |= DSCHNG_DEVNAME; // Indicate device changed
            break;

         case FS_NODENAME:
         case FS_NODENUM:
            if (dsd->rmtdata.nodenamelen != 0)
            {
                cx = dsd->namepnt;
                pnt = dsd->rmtdata.nodename;
                size = 0;
                while ((chr = *dsd->namepnt) != '\0' &&
                        ((((uchar)chr) == FS_ESC) || (((uchar)chr) < FS_MIN)))
                {
                    dsd->namepnt++;
                    if (((uchar)chr) == FS_ESC)
                        chr = *dsd->namepnt++;
                    if (*pnt++ == chr)
                        ++size;
                    else
                    {
                        size = 0;
                        break;
                    }
                }
                if (dsd->rmtdata.nodenamelen == size)
                    break;
                dsd->namepnt = cx;
            }
            if (dsd->changed &		// Have any other unreported changes?
                    (DSCHNG_NODENAME|DSCHNG_RDEVNAME|DSCHNG_PATHNAME))
            {
                if (!dsd->func(dsd))	// Yes - call the caller's function
                    return (FALSE);
                dsd->changed = 0;
            }
            dsd->rmtdata.nodenamelen = 0;
            dsd->rmtdata.rmtdevnamelen = 0;
            dsd->pathnamelen = 0;
            if (dsd->elippos != NULL)
            {
                dsd->errormsg(noelipmsg, 0);
                return (FALSE);
            }
            pnt = dsd->rmtdata.nodename;
            while ((chr = *dsd->namepnt) != '\0' &&
                    ((((uchar)chr) == FS_ESC) || (((uchar)chr) < FS_MIN)))
            {
                dsd->namepnt++;
                if (((uchar)chr) == FS_ESC)
                    chr = *dsd->namepnt++;
                if (++dsd->rmtdata.nodenamelen < 128)
                    *pnt++ = chr;
            }
            *pnt = '\0';
            dsd->changed |= DSCHNG_NODENAME; // Indicate node name changed
            break;

         case FS_RXOSNAME:
         case FS_RDOSNAME:
         case FS_RVOLNAME:
            if (dsd->rmtdata.rmtdevnamelen != 0)
            {				// Do we have a remote name now?
                cx = dsd->namepnt;	// Yes - first see if its the same
                pnt = dsd->rmtdata.rmtdevname;
                size = 0;
                while ((chr = *dsd->namepnt) != '\0' &&
                        ((((uchar)chr) == FS_ESC) || (((uchar)chr) < FS_MIN)))
                {
                    dsd->namepnt++;
                    if (((uchar)chr) == FS_ESC)
                        chr = *dsd->namepnt++;
                    if (*pnt++ == chr)
                        ++size;
                    else
                    {
                        size = 0;
                        break;
                    }
                }
                if (dsd->rmtdata.rmtdevnamelen == size)
                    break;
                dsd->namepnt = cx;
            }
            if (dsd->changed &		// Have any other unreported changes?
                    (DSCHNG_RDEVNAME|DSCHNG_PATHNAME))
            {
                if (!dsd->func(dsd))	// Yes - call the caller's function
                    return (FALSE);
                dsd->changed = 0;
            }
            dsd->rmtdata.rmtdevnamelen = 0;
            dsd->pathnamelen = 0;
            if (dsd->elippos != NULL)
            {
                dsd->errormsg(noelipmsg, 0);
                return (FALSE);
            }
            pnt = dsd->rmtdata.rmtdevname;
            while ((chr = *dsd->namepnt) != '\0' &&
                    ((((uchar)chr) == FS_ESC) || (((uchar)chr) < FS_MIN)))
            {
                dsd->namepnt++;
                if (((uchar)chr) == FS_ESC)
                    chr = *dsd->namepnt++;
                if (++dsd->rmtdata.rmtdevnamelen < 64)
                    *pnt++ = chr;
            }
            *pnt = '\0';
            dsd->changed |= DSCHNG_RDEVNAME; // Indicate remote device
            break;			     //   changed

         case FS_PATHNAME:
            if (dsd->pathnamelen != 0)
            {
                cx = dsd->namepnt;
                pnt = dsd->pathname;
                size = 0;
                while ((chr = *dsd->namepnt) != '\0' &&
                        ((((uchar)chr) == FS_ESC) || (((uchar)chr) < FS_MIN)))
                {
                    dsd->namepnt++;
                    if (((uchar)chr) == FS_ESC)
                        chr = *dsd->namepnt++;
                    if (*pnt++ == chr)
                        ++size;
                    else
                    {
                        size = 0;
                        break;
                    }
                }
                if (dsd->pathnamelen == size)
                    break;
                dsd->namepnt = cx;
            }
            if (dsd->changed & DSCHNG_PATHNAME)
            {				// Have any other unreported changes?
                if (!dsd->func(dsd))	// Yes - call the caller's function
                    return (FALSE);
                dsd->changed = 0;
            }
            dsd->pathnamelen = 0;
            if (dsd->elippos != NULL)
            {
                dsd->errormsg(noelipmsg, 0);
                return (FALSE);
            }
            pnt = dsd->pathname;
            while ((chr = *dsd->namepnt) != '\0' &&
                    ((((uchar)chr) == FS_ESC) || (((uchar)chr) < FS_MIN)))
            {
                dsd->namepnt++;
                if (((uchar)chr) == FS_ESC)
                    chr = *dsd->namepnt++;
                if (++dsd->pathnamelen < 512)
                    *pnt++ = chr;
            }
            *pnt = '\0';
            dsd->changed |= DSCHNG_PATHNAME; // Indicate path changed
            break;

         case FS_FILENAME:
            pnt = dsd->filename;
            size = 0;
            while ((chr = *dsd->namepnt) != '\0' &&
                    ((uchar)chr < FS_MIN || (uchar)chr == FS_ESC))
            {
                dsd->namepnt++;
                if ((uchar)chr == FS_ESC)
                    chr = *dsd->namepnt++;
                *pnt++ = chr;
                ++size;
            }
            if ((uchar)chr == FS_VERSION)
            {
                dsd->namepnt++;
                *pnt++ = ';';
                ++size;
                while ((chr = *dsd->namepnt) != '\0' &&
                        ((uchar)chr < FS_MIN || (uchar)chr == FS_ESC))
                {
                    dsd->namepnt++;
                    if ((uchar)chr == FS_ESC)
                        chr = *dsd->namepnt++;
                    *pnt++ = chr;
                    ++size;
                }
            }
            *pnt = '\0';
            dsd->filenamelen = size;
	    break;

         case FS_FILEDOS:
            pnt = dsd->filedos;
            size = 0;
            while ((chr = *dsd->namepnt) != '\0' &&
                    ((uchar)chr < FS_MIN || (uchar)chr == FS_ESC))
            {
                dsd->namepnt++;
                if ((uchar)chr == FS_ESC)
                    chr = *dsd->namepnt++;
                *pnt++ = chr;
                ++size;
            }
            *pnt = '\0';
            dsd->filedoslen = size;
	    break;

	 case FS_ATTR:
	    dsd->attr = (dsd->namepnt[1] << 8) + (uchar)(dsd->namepnt[0]);
	    dsd->namepnt += 2;
            dsd->error = (chr == '\0')? error: 0;
	    dsd->level = level;
            if (!dsd->func(dsd))	// Call the caller's function
	        return (FALSE);
	    dsd->changed = 0;
            dsd->filenamelen = dsd->filedoslen = 0;
	    if (dsd->filename != NULL)
		dsd->filename[0] = '\0';
	    if (dsd->filedos != NULL)
		dsd->filedos[0] = '\0';
	    break;

         default:
            {
                char bfx[64];

                sprintf(bfx, "Internal error processing name in dirscan,"
                        " char = %X\n", (uchar)chr);
                dsd->errormsg(bfx, 0);
            }
            return (FALSE);
        }
    }
    return (TRUE);
}


static char *checkdfltext(
    char        *filespec,
    char       **elipp,
    DIRSCANDATA *dsd)

{
    char *pnt;
    char *bgn;
    char  chr;

    if (dsd->newfilespec == NULL)
    {
	if (!(dsd->dfltwildext) || !(dsd->diropnparm.devsts.value & DS_NAMEEXT))
	{
	    dsd->newfilespec = (char *)-1;
	    return (filespec);
	}
	bgn = pnt = filespec;
	while ((chr = *pnt++) != 0)
	{
	    if (chr == ':' || chr == '\\')
		bgn = pnt;
	}
	while ((chr = *bgn++) != 0)
	{
	    if (chr == '.')
	    {
		dsd->newfilespec = (char *)-1;
		return (filespec);
	    }
	}
	if ((pnt = dsgetmem(strlen(filespec) + 3, dsd)) == NULL)
	    return (NULL);

	strmov(strmov(pnt, filespec), ".*");
	dsd->newfilespec = pnt;
	if (*elipp != NULL)
	    *elipp  = pnt + (*elipp - filespec);
	return (pnt);
    }
    return (filespec);
}

//*****************************************
// Function: doelip - do ellipsis search
// Returned: TRUE if normal, FALSE if error
//*****************************************

// The filespec argument string has had the ellipsis removed before this
//   function is called.  The pointer elip points to the character which
//   previously followed the ellipsis:
//   Given filespec:   C:\AAA\...\BBB\*.XXX
//   Argument string:  C:\AAA\BBB\*.XXX
//                     |      |
//                     |      elip points here
//                     filespec points here
//   The directory preceeding the ellipis is open and is the directory we
//   will search here.

static int doelip(
    char        *filespec,
    char        *elip,
    DIRSCANDATA *dsd)

{
    DIRNAME *next;
    DIRNAME *thisdir;
    DIRNAME *lastdir;
    DIRNAME *firstdir;
    char    *elipsrch;
    char    *elippnt;
    char    *p1;
    char    *p2;
    char    *pnt;
    long     rtn;
    int      elipsize;
    int      val;
    int      size;
    char     chr;
    char     matchdir[300];

    if ((elippnt = strrchr(filespec, ':')) != NULL)
        filespec = elippnt + 1;
    if ((elippnt = strchr(elip, '\\')) != NULL) // Have directory after ...?
    {					// Yes - must copy the name of the
					//   directory followiing ...
        if ((val = (int)(elippnt - elip)) > 64) // Make sure its not too long
        {
            char bfr[150];

            *elippnt = '\0';
            sprintf(bfr, "Directory name is too long: %.100s", elip);
            dsd->errormsg(bfr, 0);
            return (FALSE);
        }
        sprintf(matchdir, "%.*s", val, elip);
    }
    else
        matchdir[0] = '\0';		// Ellipis is last, nothing to match
    elipsize = elip - filespec;
    if ((elipsrch = dsgetmem(elipsize + 6, dsd)) == NULL)
        return (FALSE);
    strmov(strnmov(elipsrch, filespec, elipsize), "*");

    // Reset to start of directory we are searching and get devsts bits

    if ((rtn = svcIoInBlockP(dsd->parmlist->dirhndl.value, NULL, 0,
            &dsd->dirposparm)) < 0)
    {
        dsd->errormsg(elipsrch, rtn);
        return (FALSE);
    }
    dsd->dirsrchparm.dirhndl.value = dsd->parmlist->dirhndl.value;

    dsd->dirqab.qab_buffer1 = elipsrch;
    dsd->dirqab.qab_option = O_REPEAT;
    lastdir = (struct dirname *)&firstdir;
    firstdir = NULL;
    do
    {
        dsd->parmlist->filspec.buffer[0] = '\0';    // Collect a list of all
        if ((rtn = svcIoQueue(&dsd->dirqab)) < 0 || //   of the directories in
                (rtn = dsd->dirqab.qab_error) < 0)  //   the directory we are
        {					    //   searching
            if (rtn == ER_FILNF)
                break;
            dsd->errormsg(filespec, rtn);
            svcIoClose(dsd->dirsrchparm.dirhndl.value, 0);
            return (FALSE);
	}
	p1 = (char *)(dsd->dirsrchparm.filspec.buffer);
	while ((chr = *p1++) != '\0')
	{
	    if ((uchar)chr != FS_FILENAME)
	    {
		dsd->errormsg("Internal error processing ...", 0);
		return (FALSE);
	    }
	    p2 = p1;
	    size = 0;
	    while ((chr = *p1) != '\0' && (((uchar)chr) == FS_ESC ||
		    ((uchar)chr) < FS_MIN))
            {
                p1++;
                if (((uchar)chr) == FS_ESC)
                    p1++;
                ++size;
            }
            if (size <= 2 && p2[0] == '.' && (size == 1 || p2[1] == '.'))
                continue;		// Ignore if . or ..
            if ((thisdir = (struct dirname *)dsgetmem(sizeof(struct dirname) -
                    1 + size, dsd)) == NULL)
                return (FALSE);
            pnt = thisdir->name;
            while (--size >= 0)
            {
                if (((uchar)(chr = *p2++)) == FS_ESC)
                    chr = *p2++;
                *pnt++ = chr;
            }
            *pnt = 0;
            lastdir->next = thisdir;
            lastdir = thisdir;
            thisdir->next = NULL;
        }
    } while (dsd->dirqab.qab_amount & 0x80000000L);
    free(elipsrch);
    svcIoClose(dsd->parmlist->dirhndl.value, 0);

    // Now we have constructed a list of directories to search

    if (firstdir != NULL)		// Did we find any directries?
    {					// Yes
        if (dsd->sort != DSSORT_NONE)	// Should we sort the list?
            firstdir = heapsort(firstdir, // Yes
                    (int (*)(void *a, void *b, void *dsd))compdir, dsd);

        // Now we have a sorted list of all subdirectories.

        do				// Loop for each file
        {
            // If this directory matches the directory after the ellipsis,
            //   we process it without the ellipsis.  If we found the directory
            //   BBB in C:\AAA\ when given C:\AAA\...\BBB\*.XXX, we now
            //   process C:\AAA\BBB\*.XXX.  This will list the files in this
            //   directory.  We also process it again retaining the ellipsis
            //   in case there is more than one BBB in this branch of the
            //   directory tree, such as C:\AAA\BBB\QQQ\BBB\, which should
            //   also be listed!

            if (stricmp(matchdir, firstdir->name) == 0)
            {
                if ((elippnt = dsgetmem(strlen(dsd->devname) +
                        strlen(dsd->rmtdata.nodename) +
			strlen(dsd->rmtdata.rmtdevname) +
			strlen(filespec) + 2, dsd)) == NULL)
                    return (FALSE);
                strmov(strmov(strmov(strmov(elippnt, dsd->devname),
                        dsd->rmtdata.nodename), dsd->rmtdata.rmtdevname),
			filespec);
		level++;
                if (!_dirscan(elippnt, TRUE, dsd))
                {
                    free(elippnt);
                    return (FALSE);
                }
		level--;
                free(elippnt);
            }

            // Now we process each directory we found by constructing a new
            //   name which moves the ellipsis down one level.  If we found
            //   the directory CCC in C:\AAA\ when given C:\AAA\...\BBB\*.XXX,
            //   we now process C:\AAA\CCC\...\BBB\*.XXX

            if ((elippnt = dsgetmem(strlen(dsd->devname) +
                    strlen(dsd->rmtdata.nodename) +
		    strlen(dsd->rmtdata.rmtdevname) +
                    strlen(firstdir->name) + strlen(filespec) + 6, dsd))
                    == NULL)
                return (FALSE);
            strmov(strmov(strmov(strnmov(strmov(strmov(strmov(elippnt,
                    dsd->devname), dsd->rmtdata.nodename),
		    dsd->rmtdata.rmtdevname), filespec, (int)(elip - filespec)),
		    firstdir->name), "\\...\\"), elip);
            next = firstdir->next;
            free(firstdir);
	    level++;
            if (!_dirscan(elippnt, TRUE, dsd))
            {
		level--;
                free(elippnt);
                return (FALSE);
            }
	    level--;
            free(elippnt);
        } while ((firstdir = next) != NULL);
    }
    return (TRUE);
}

//**************************************************
// Function: compdir - called by heapsort to compare
//		two directory names
// Returned: Negative if a < b
//	     Zero if a == b
//           Positive if a > b
//**************************************************

static int compdir(
    DIRNAME     *a,
    DIRNAME     *b,
    DIRSCANDATA *dsd)

{
    return ((dsd->sort == DSSORT_ASCEN)?
            stricmp(a->name, b->name): stricmp(b->name, a->name));
}

//****************************************************
// Function: dsgetmem - Get memory with malloc, return
//		with error if out of memory
// Returned: Pointer to memory obtained
//****************************************************

static void *dsgetmem(
    size_t       size,
    DIRSCANDATA *dsd)

{
    void *ptr;

    ptr = malloc(size);
    if (ptr == NULL)
    {
        dsd->errormsg("Not enough memory available", 0);
        return (NULL);
    }

//  if (maxmem < _malloc_amount)
//      maxmem = _malloc_amount;

    return (ptr);
}
