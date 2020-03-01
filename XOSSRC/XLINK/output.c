//*********************************************
// Routines to output to the RUN file for XLINK
//*********************************************
// Written by John Goltz
//*********************************************

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
#include <PROCARG.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSERRMSG.H>
#include "XLINK.H"
#include "XLINKEXT.H"

// Allocate local data 

struct outbhd *lastob;		// Address of header block for last buffer
				//   used
long   filetop;			// Offset to top of output file
short  bufsta;			// Buffer state

struct outbhd  outhead[4];	// The buffer header blocks

struct outbhd *bufnext[] =
{
    &outhead[3],&outhead[2],&outhead[3],&outhead[1],	//  0  1  2  3
    &outhead[2],&outhead[1],&outhead[3],&outhead[2],	//  4  5  6  7
    &outhead[3],&outhead[0],&outhead[2],&outhead[0],	//  8  9 10 11
    &outhead[3],&outhead[1],&outhead[3],&outhead[0],	// 12 13 14 15
    &outhead[1],&outhead[0],&outhead[2],&outhead[1],	// 16 17 18 19
    &outhead[2],&outhead[0],&outhead[1],&outhead[0]	// 20 21 22 23
};

char bf0sta[] =
{ 
    0, 1, 2, 3, 4, 5, 0, 1, 0, 0, 1, 1,
    2, 3, 2, 2, 3, 3, 4, 5, 4, 4, 5, 5
};
char bf1sta[] =
{ 
    6, 7, 6, 6, 7, 7, 6, 7, 8, 9,10,11,
    8, 8, 8, 9, 9, 9,10,10,10,11,11,11
};
char bf2sta[] =
{
    12,12,12,13,13,13,14,14,14,15,15,15,
    12,13,14,15,16,17,16,16,16,17,17,16
};
char bf3sta[] =
{
    18,18,19,19,18,19,20,20,21,21,20,21,
    22,22,23,23,22,23,18,19,20,21,22,23
};

//***************************************************************
// Function: putvarval - Output variable length value to RUN file
// Returned: Nothing
//***************************************************************

void putvarval(
    long value)

{
    if (value <= 0x7F)
        putbyte(value);
    else if (value <= 0x3FFF)
    {
        putbyte((value >> 8) | 0x80);
        putbyte(value);
    }
    else if (value <= 0x1FFFFF)
    {
        putbyte((value >> 16) | 0xC0);
        putbyte(value >> 8);
        putbyte(value);
    }
    else
    {
        putbyte((value >> 24) | 0xE0);
        putbyte(value >> 16);
        putbyte(value >> 8);
        putbyte(value);
    }
}

//********************************************
// Function: putlong - Output long to RUN file
// Returned: Nothing
//********************************************

void putlong(
    long value)

{
    short shrt;

    if (lowfirst)
    {
	shrt = value;
	putword(shrt);
	shrt = value >> 16;
	putword(shrt);
    }
    else
    {
	shrt = value >> 16;
	putword(shrt);
	shrt = value;
	putword(shrt);
    }
}

//********************************************
// Function: putword - Output word to RUN file
// Returned: Nothing
//********************************************

void putword(
    short value)

{
    if (lowfirst)
    {
	putbyte(value);
	putbyte(value >> 8);
    }
    else
    {
	putbyte(value >> 8);
	putbyte(value);
    }
}

//***************************************************
// Function: putbyte - Output single byte to RUN file
// Returned: Nothing
//***************************************************

void putbyte(
    short data)			// Byte to output

{
    struct outbhd *obpnt;
    char  *str;
    long   oset;
    long   obase;
    long   lrtn;
    short  cnt;

    if (outdd)				// Only do this if doing output
    {
        oset = offset;
        obase = oset & ~0x3FF;
        if ((obpnt=lastob) == NULL || obase!=obpnt->ob_base)
        {				// Same buffer as before?
            cnt = 4;			// No - see if have right buffer in
            obpnt = outhead;		//   memory
            while (--cnt >= 0)
            {
                if (obpnt->ob_inuse && obase == obpnt->ob_base)
                    break;
                ++obpnt;
            }
            if (cnt < 0)		// Did we find it?
            {
                obpnt = bufnext[bufsta]; // No - reallocate a buffer
                bufsta = obpnt->ob_sta[bufsta]; // Update state
                if (obpnt->ob_inuse)	// Is this buffer in use now?
                    outblk(obpnt);	// Yes - write it out
                obpnt->ob_inuse = TRUE;
                obpnt->ob_base = obase;	// Store base for buffer
                if (obase < filetop)	// Is this block in the file now?
                {			// Yes - read the block
                    if (((lrtn=svcIoSetPos(outdd, obase, 0)) < 0)
                            || ((lrtn=svcIoInBlock(outdd, obpnt->ob_buf,
                            1024)) < 0))
                        femsg(prgname, lrtn, curobj->obj_fsp);
                    if (lrtn == 0)
                        fail("Unexpected eof");
                    obpnt->ob_size = lrtn;
                    if (lrtn != 1024L)	// Did we get less than a block?
                    {
                        str = obpnt->ob_buf + (int)lrtn;
                        lrtn = 1024L - lrtn; // Yes - clear the excess
                        if (lrtn & 1)
                        {
                            *str++ = 0;
                            lrtn -= 1;
                        }
                        if (lrtn & 2)
                        {
                            *str++ = 0;
                            *str++ = 0;
                            lrtn -= 2;
                        }
                        if (lrtn)
                            clrbuf((long *)str, (int)lrtn);
                    }
                }
                else
                {
                    obpnt->ob_size = 0;	// No - clear the buffer
                    clrbuf((long *)(obpnt->ob_buf), 1024);
                }
            }
            lastob = obpnt;
        }
        oset &= 0x3FF;
        *(obpnt->ob_buf + (int)oset) = data; // Store data in buffer
        if (obpnt->ob_size <= oset)	// Did we extend this buffer?
            obpnt->ob_size = oset + 1;	// Yes - adjust size
    }
    offset++;
}

//*******************************************
// Function: outclose - Close the output file
// Returned: Nothing
//*******************************************

void outclose(void)

{
    struct outbhd *obpnt;
    struct outbhd *lowpnt;
    long   lowbase;
    long   lrtn;
    short  cnt;

    for (;;)
    {
	cnt = 4;			// Output lowest buffer in memory
	obpnt = outhead;
	lowbase = 0x7FFFFFFFL;
	while (--cnt >= 0)
	{
	    if (obpnt->ob_inuse && obpnt->ob_base < lowbase)
	    {
		lowbase = obpnt->ob_base;
		lowpnt = obpnt;
	    }
	    ++obpnt;
	}
	if (lowbase != 0x7FFFFFFFL)	// Did we find one to output?
	{
	    outblk(lowpnt);		// Yes - output it
	    lowpnt->ob_inuse = FALSE;
	}
	else
	    break;			// No - all output is done
    }
    if ((lrtn=svcIoClose(outdd, 0)) < 0) // Close the output file
	femsg(prgname, lrtn, outfsp);
}

//********************************
// Function: outblk - Output block
// Returned: Nothing
//********************************

void outblk(obpnt)
struct outbhd *obpnt;

{
    long offset;
    long amnt;
    long lrtn;

    offset = obpnt->ob_base;
    if (offset > filetop)		// Are we doing output beyond EOF?
    {					// Yes - extend file by writting
					//   blocks which contain all 0s
	if ((lrtn=svcIoSetPos(outdd, filetop, 0)) < 0)
	    femsg(prgname, lrtn, outfsp);
	while (offset > filetop)
	{
	    amnt = 512 - (filetop & 0x1FF);
	    if ((lrtn=svcIoOutBlock(outdd, (char *)auxloc, (int)amnt)) != amnt)
            {
                if (lrtn >= 0)
                    lrtn = ER_ICMIO;
		femsg(prgname, lrtn, outfsp);
            }
	    filetop += amnt;
	}
    }
    if (((lrtn=svcIoSetPos(outdd, offset, 0)) < 0) ||
	    ((lrtn=svcIoOutBlock(outdd, obpnt->ob_buf, obpnt->ob_size))
            != obpnt->ob_size))
    {
        if (lrtn >= 0)
            lrtn = ER_ICMIO;
	femsg(prgname, lrtn, outfsp);
    }
    if (offset+obpnt->ob_size > filetop)
	filetop = offset + obpnt->ob_size;
}
