//================================================
// Program: UDFREAD.C - Functions for reading UDFs
//================================================

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
#include <XOSSTR.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <ERRMSG.H>
#include "UDFREAD.H"

// udfOpen is called first to open the UDF.  udfGetRecord is called repeatedly
//   to read each record in the UDF.  The buffer passed to udfGetRecord must
//   be large enough to hold the entire record.  After a normal return from
//   udfGetRecord, udfGetBlock is called repeatedly to access each block in
//   the record.  After a good return from udfGetBlock, the blkpnt and blklen
//   items in the UDF structure point to the first data byte in the block and
//   specify the length of the block.  These value MUST NOT be modified by the
//   caller between calls to udfGetBlock.  A block can be re-read by saving
//   the values of the reccnt and blkpnt items in the UDF structure after the
//   return from udfGetRecord.  Restoring these values allows the blocks in
//   the record to be re-accessed by calling udfGetBlock.  When access to the
//   UDF is complete, it is closed by calling udfClose.

extern long  errno;

static int getbyte(UDF *udf);

//********************************************
// Function: udfOpen - Open a UDF for reading
// Returned: Address of UDF structure if OK or
//		null if error (errno is set)
//********************************************

UDF *udfOpen(
    char *name)

{
    UDF *udf;
    int  rtn;
    int  head2;

    if ((udf = malloc(sizeof(UDF))) == NULL)
	return (NULL);
    if ((udf->handle = svcIoOpen(O_IN, name, NULL)) < 0)
    {
	errno = -udf->handle;
	free(udf);
	return (NULL);
    }
    udf->offset = 0;
    udf->dskcnt = 0;
    udf->next = 0;
    if ((rtn = getbyte(udf)) < 0)
    {
	errno = -rtn;
	free(udf);
	return (NULL);
    }
    if ((head2 = getbyte(udf)) < 0)
    {
	errno = -head2;
	free(udf);
	return (NULL);
    }
    if (rtn != 0x81 || head2 != 0xC6)
    {
        errno = -ER_BDFMT;
	free(udf);
        return (NULL);
    }

    if ((rtn = getbyte(udf)) < 0)
    {
        errno = -rtn;
	free(udf);
        return (NULL);
    }
    if (rtn != 0xAA)
    {
        errno = -ER_BDFMT;
	free(udf);
        return (NULL);
    }
    return (udf);
}

//*****************************************************
// Function: udfClose - Close a UDF opened with udfOpen
// Returned: Nothing
//*****************************************************

void udfClose(
    UDF *udf)

{
    svcIoClose(udf->handle, 0);
    free(udf);
}

//*****************************************************
// Function: udfGetRecord - Read next record from UDF
// Returned: Record type (positive) if normal, 0 if end
//		of file, or -1 if error (errno is set)
//*****************************************************

int udfGetRecord(
    UDF   *udf,
    uchar *buffer,
    int    size)

{
    uchar *inpnt;
    int    incnt;
    int    data;

    if (udf->next < 0)
	return (0);
    udf->recpos = udf->offset - 1;
    inpnt = buffer;
    incnt = size;
    for (;;)
    {
	if ((data = getbyte(udf)) < 0 || data == 0xAA)
	{
	    if (data == 0xAA || data == ER_EOF)
		break;
	    errno = -data;
	    return (-1);
	}
	if (data == 0xAB)
	{
	    switch (data = getbyte(udf))
	    {
	     case 1:
		data = 0xAA;
		break;

	     case 2:
		data = 0xAB;
		break;

	     default:
		errno = - ER_BDPFX;
		return (-1);
	    }
	}
	if (--incnt > 0)
	    *inpnt++ = data;
	else if (incnt == 0)
	{
	    errno = -ER_REC2L;
	    return (-1);
	}
    }
    if ((udf->reccnt = inpnt - buffer - 1) < 0)
    {
	errno = -ER_REC2S;
	return (-1);
    }
    udf->blkpnt = buffer + 1;
    udf->blklen = 0;
    udf->next = data;
    return (buffer[0]);
}

//*******************************************************
// Function: udfGetBlock - Get next block in record
// Returned: Block type (positive) if normal, 0 if end of
//		record, or -1 if error (errno is set)
//*******************************************************

int udfGetBlock(
    UDF *udf)

{
    int type;
    int len;

    if (udf->reccnt == 0)
	return (0);
    udf->blkpnt += udf->blklen;
    if ((udf->reccnt -= 2) < 0)
    {
	errno = -ER_REC2S;
	return (-1);
    }
    type = *udf->blkpnt++;
    udf->hdrlen = 2;
    if ((len = *udf->blkpnt++) & 0x80)
    {
	if (udf->reccnt-- < 0)
	{
	    errno = -ER_REC2S;
	    return (-1);
	}
	len = ((len & 0x7F) << 8) + *udf->blkpnt++;
	udf->hdrlen++;
    }
    if ((udf->reccnt -= len) < 0)
    {
	errno = -ER_REC2S;
	return (-1);
    }
    udf->blklen = len;
    return (type);
}

static int getbyte(
    UDF *udf)

{
    if (--udf->dskcnt < 0)
    {
        if ((udf->dskcnt = svcIoInBlock(udf->handle, udf->dskbufr, 1024)) <= 0)
        {
	    if (udf->dskcnt == 0)
		udf->dskcnt = ER_EOF;
            return (udf->dskcnt);
        }
	udf->dskcnt--;
	udf->dskpnt = udf->dskbufr;
    }
    udf->offset++;
    return (*udf->dskpnt++);
}
