//**********************
// IO routines for XLIB
//**********************
// Written by John Goltz
//**********************

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
#include <STRING.H>
#include <TIME.H>
#include <XOS.H>
#include <XOSERRMSG.H>
#include <XOSSVC.H>
#include "XLIB.H"
#include "XLIBEXT.H"

//*************************************************************
// Function: openobj - Open a OBJ file and read the first block
// Returned: Nothing
//*************************************************************

void openobj(
    struct obj     *obj,
    struct nameblk *namepnt)

{
    uchar byte;

    if ((obj->obj_handle=svcIoOpen(O_IN, namepnt->nb_name, NULL)) < 0)
					// Open input OBJ file
	femsg(prgname, obj->obj_handle, namepnt->nb_name); // If error
    obj->obj_name = namepnt;
    obj->obj_block = -1;		// Read first block
    obj->obj_count = 0;
    obj->obj_buf = obj->obj_loc;
    obj->obj_seccnt = 127;		// Make sure section count does not
					//   go to 0 here
    if ((byte = getbyte(obj)) == 0x84)	// Is it the right format?
    {
        byte = getbyte(obj);		// Yes - get next byte
        if (byte > 1) 			// Legal value?
            badfmt(obj);			// No
        obj->obj_libflag = byte;	// Remember if library file
        obj->obj_modtype = MT_XOS;
    }
    else if (byte == 0x80)		// MS module start?
    {
        (obj->obj_pnt)--;		// Yes - put back the first byte
        (obj->obj_count)++;
        obj->obj_modtype = MT_MS;
    }
    else
	badfmt(obj);			// No - loose!
}

//*************************************************************
// Function: startmod - Setup to read next module from OBJ file
// Returned: Nothing
//*************************************************************

void startmod(
    struct obj *obj)

{
    uchar byte;

    if (obj->obj_modtype == MT_XOS)
    {
        obj->obj_seccnt = 127;
        byte = getbyte(obj);		// Get next byte
        if (byte == 0x80)		// High order byte first?
            lowfirst = FALSE;		// Yes
        else if (byte == 0x81)		// No - low order byte first?
            lowfirst = TRUE;		// Yes
        else
            badfmt(obj);		// No - error
        obj->obj_language = getbyte(obj); // Remember the language byte
    }
    else
    {
        lowfirst = TRUE;
        obj->obj_language = 0;
    }
    obj->obj_seccnt = 0;
}

//************************************
// Function: closeobj - Close OBJ file
// Returned: Nothing
//************************************

void closeobj(
    struct obj *obj)

{
    long lrtn;

    if ((lrtn=svcIoClose(obj->obj_handle, 0)) < 0) // Close the file
	femsg(prgname, lrtn, obj->obj_name->nb_name);
    obj->obj_handle = 0;		// Remember that it is closed
    obj->obj_name = NULL;
}

//********************************************************************
// Function: startsec - Advance to start of next section, read header,
//		and setup the byte count
// Returned: Section type
//********************************************************************

int startsec(
    struct obj *obj)

{
    int type;

    if (obj->obj_seccnt != 0)		// Anything to skip?
	skpbyte(obj, obj->obj_seccnt);	// Yes - skip it
    obj->obj_seccnt = 127;		// Ensure section count doesn't run
					//   out while reading the header
    if ((type = (int)getbyte(obj)) != 0) // Get section type
    {
	if (type > SC_MAX && type < MSSC_MIN && type > MSSC_MAX)
					// Legal type?
	    badsec(obj);		// No!
	obj->obj_seccnt = getword(obj);	// Yes - get section size
    }
    return (type);
}

//*******************************************
// Function: getbyte - Get byte from OBJ file
// Returned: Value of byte
//*******************************************

int getbyte(
    struct obj *obj)

{
    if (--obj->obj_count < 0)		// Anything in buffer now?
    {
	++obj->obj_block;		// No - bump block number
	readblk(obj);			// Read next block
	obj->obj_count = 511;		// Reset count
	obj->obj_pnt = obj->obj_buf;	// And pointer
    }
    if (--obj->obj_seccnt < 0)		// Reduce section count
	endofsec(obj);
    return ((int)((uchar)*obj->obj_pnt++));
}

//*******************************************
// Function: getword - Get word from OBJ file
// Returned: Value of word
//*******************************************

int getword(
    struct obj *obj)

{
    uint first;
    uint second;

    first = getbyte(obj);		// Get first byte
    second = getbyte(obj);		// Get second byte
    if (lowfirst)			// Low order byte first?
	return (first + (second << 8)); // Yes
    else
	return (second + (first << 8)); // No
}

//*******************************************
// Function: getlong - Get long from OBJ file
// Returned: Value of long
//*******************************************

int getlong(
    struct obj *obj)

{
    ulong first;
    ulong second;

    first = getword(obj);		// Get first word
    second = getword(obj);		// Get second word
    if (lowfirst)			// Low order byte first?
	return (first + (second << 16)); // Yes
    else
	return (second + (first << 16)); // No
}

//************************************************
// Function: getsym - Get symbol from XOS OBJ file
// Returned: Nothing
//************************************************

void getsym(
    struct obj *obj)

{
    int   cnt;
    char *pnt;

    pnt = symbuf;			// Point to symbol buffer
    cnt = SYMMAXSZ;
    while (!((*pnt=getbyte(obj)) & 0x80)) // Get symbol character
    {
	if (--cnt <= 0)			// Symbol too long?
	    fail(obj, "Symbol too long in file");
	++pnt;
    }
    *pnt++ &= 0x7F;			// Remove high bit
    *pnt = '\0';			// End it with a null
    symsize = (int)(pnt - symbuf);
}

//*************************************************
// Function: getmssym - Get symbol from MS OBJ file
// Returned: Nothing
//*************************************************

void getmssym(
    struct obj *obj)

{
    int   cnt;
    char *pnt;

    pnt = symbuf;			// Point to symbol buffer
    cnt = getbyte(obj);			// Get length of name
    if (cnt > SYMMAXSZ)
	fail(obj, "Symbol too long in file");
    symsize = cnt;
    while (--cnt >= 0)
        *pnt++ = getbyte(obj);
    *pnt = '\0';			// End it with a null
}

//*********************************************************
// Function: getitem - Get variable size item from OBJ file
// Returned: Value of item
//*********************************************************

int getitem(
    struct obj *obj,
    uchar  size)	// Size and extension flag:
			//   000 = 0 bytes, value = 0
			//   001 = 1 byte, 0 extension
			//   010 = 2 bytes, 0 extension
			//   011 = 4 bytes
			//   100 = 0 bytes, value = 0xFFFFFFFF
			//   101 = 1 byte, 1 extension
			//   110 = 2 bytes, 1 extension
			//   111 = 4 bytes
{
    switch (size & 0x03)
    {
    case 0:
	if (size & 0x04)
	    return (0xFFFFFFFFL);
	else
	    return (0L);
    case 1:
	if (size & 0x04)
	    return (getbyte(obj) | 0xFFFFFF00L);
	else
	    return (getbyte(obj));
    case 2:
	if (size & 0x04)
	    return (getword(obj) | 0xFFFF0000L);
	else
	    return (getword(obj));
    case 3:
	    return (getlong(obj));
    }
    return (0);
}

//***********************************************************************
// Function: getnumber - Get variable length XOS number from the OBJ file
// Returned: Value of number
//***********************************************************************

int getnumber(
    struct obj *obj)

{
    int value;

    value = getbyte(obj);		// Get first byte
    if (!(value & 0x80L))		// 8 bit value?
        return (value);			// Yes - return it
    value = ((value &0x7F) << 8) + getbyte(obj); // No - get next 8 bits
    if (!(value & 0x4000L))		// 16 bit value?
        return (value);			// Yes - return it

    value = ((value & 0x3FFF) << 8) + getbyte(obj); // Get next 8 bits
    if (!(value & 0x200000L))		// 24 bit value?
        return (value);			// Yes - return it
    return (((value & 0x1FFFFF) << 8) + getbyte(obj));
}

//*********************************************************************
// Function: getmsnum - Get variable length MS number from the OBJ file
// Returned: Value of number
//*********************************************************************

int getmsnum(
    struct obj *obj)

{
    int value;

    value = getbyte(obj);		// Get first byte
    if (!(value & 0x80L))		// 8 bit value?
        return (value);			// Yes - return it
    return (((value &0x7F) << 8) + getbyte(obj)); // No - return 16 bits
}

//***********************************************
// Function: skpbyte - Skip bytes in the OBJ file
// Returned: Nothing
//***********************************************

void skpbyte(
    struct obj *obj,
    int    amnt)

{
    if ((obj->obj_seccnt -= amnt) < 0)	// Reduce section count
	endofsec(obj);
    if ((obj->obj_count -= amnt) > 0)	// Need to go past end of block?
	obj->obj_pnt += amnt;		// No - just adjust pointer and count
    else
    {
	amnt = -obj->obj_count;		// Yes - get amount from end of block
	obj->obj_block += amnt/512 + 1;	// Adjust block number
	readblk(obj);			// Read the block
	amnt %= 512;			// Get offset in block
	obj->obj_count = 512 - amnt;
	obj->obj_pnt = obj->obj_buf + amnt;
    }
}

//***********************************************************************
// Function: setblk - Setup to start reading OBJ file at offset specified
// Returned: Nothing
//***********************************************************************

void setblk(
    struct obj *obj,
    long   offset)

{
    long block;

    block = offset >> 9;		// Get block number
    if (block != obj->obj_block)	// Is this block in our buffer now?
    {
	obj->obj_block = block;		// No - read it
	readblk(obj);
    }
    offset &= 0x1FF;			// Get offset in block
    obj->obj_pnt = obj->obj_buf + (int)offset; // Set pointer
    obj->obj_count = 512 - offset;	// Set count
}

//******************************************************
// Function: readblk - Read block from disk for OBJ file
// Returend: Nothing
//******************************************************

void readblk(
    struct obj *obj)

{
    long lrtn;

    if (((lrtn=svcIoSetPos(obj->obj_handle, obj->obj_block<<9,0)) < 0)
            || ((lrtn=svcIoInBlock(obj->obj_handle, obj->obj_buf,512)) < 0))
	femsg(prgname, lrtn, obj->obj_name->nb_name);
    if (lrtn == 0)
        fail(obj, "Unexpected eof");
}

//************************************************************
// Function: endofsec - Report unexpected end of section error
// Returned: Nothing
//************************************************************

void endofsec(
    struct obj *obj)

{
    fail(obj, "Unexpected end of section");
}

//***********************************************
// Function: badsec - Report illegal section type
// Returend: Never returns
//***********************************************

void badsec(
    struct obj *obj)

{
    fail(obj, "Illegal section type");
}

//************************************************************
// Function: badfmt - Report illegal format in OBJ file errors
// Returned: Never returns
//************************************************************

void badfmt(
    struct obj *obj)

{
    fail(obj, "Illegal OBJ file format");
}

//*********************************************************
// Function: fail - Report fatal errors with simple message
// Returned: Never returns
//*********************************************************

void fail(
    struct obj *obj,
    char  *msg)

{
    fprintf(stderr, "? XLIB: %s, file %s\n", msg,obj->obj_name->nb_name);
    loose();
}

//*****************************************************************
// Function: loose - Clean up loose ends before exiting after error
// Returned: Never returns
//*****************************************************************

void loose(void)

{
    exit(1);
}
