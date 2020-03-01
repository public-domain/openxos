//**********************
// IO routines for XLINK
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
#include <STRING.H>
#include <CTYPE.H>
#include <PROCARG.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>
#include "XLINK.H"
#include "XLINKEXT.H"

uchar mstypetbl[] =
{   SC_MTHEADR,		// 0x80 Module header
    SC_BAD,		// 0x81 Illegal
    SC_BAD,		// 0x82 Illegal
    SC_BAD,		// 0x83 Illegal
    SC_BAD,		// 0x84 Illegal
    SC_BAD,		// 0x85 Illegal
    SC_BAD,		// 0x86 Illegal
    SC_BAD,		// 0x87 Illegal
    SC_MCOMENT,		// 0x88 Comment
    SC_BAD,		// 0x89 Illegal
    SC_MMODEND,		// 0x8A 16-bit module end
    SC_MMODEND,		// 0x8B 32-bit module end
    SC_MEXTDEF,		// 0x8C External definition
    SC_BAD,		// 0x8D Illegal
    SC_MTYPDEF,		// 0x8E Type definition
    SC_BAD,		// 0x8F Illegal
    SC_MPUBDEF,		// 0x90 16-bit public definition
    SC_MPUBDEF,		// 0x91 32-bit public definition
    SC_MLOCSYM,		// 0x92 16-bit local symbols
    SC_MLOCSYM,		// 0x93 32-bit local symbols
    SC_MLINNUM,		// 0x94 16-bit source line number
    SC_MLINNUM,		// 0x95 32-bit source line number
    SC_MLNAMES,		// 0x96 Name list
    SC_BAD,		// 0x97 Illegal
    SC_MSEGDEF,		// 0x98 16-bit segment definition
    SC_MSEGDEF,		// 0x99 32-bit segment definition
    SC_MGRPDEF,		// 0x9A Group definition
    SC_BAD,		// 0x9B Illegal
    SC_MFIXUPP,		// 0x9C Fix up previous 16-bit data image
    SC_MFIXUPP,		// 0x9D Fix up previous 32-bit data image
    SC_BAD,		// 0x9E Illegal
    SC_BAD,		// 0x9F Illegal
    SC_MLEDATA,		// 0xA0 16-bit logical data image
    SC_MLEDATA,		// 0xA1 32-bit logical data image
    SC_MLIDATA,		// 0xA2 16-bit logical repeated (iterated) data image
    SC_MLIDATA,		// 0xA3 32-bit logical repeated (iterated) data image
    SC_BAD,		// 0xA4 Illegal
    SC_BAD,		// 0xA5 Illegal
    SC_BAD,		// 0xA6 Illegal
    SC_BAD,		// 0xA7 Illegal
    SC_BAD,		// 0xA8 Illegal
    SC_BAD,		// 0xA9 Illegal
    SC_BAD,		// 0xAA Illegal
    SC_BAD,		// 0xAB Illegal
    SC_BAD,		// 0xAC Illegal
    SC_BAD,		// 0xAD Illegal
    SC_BAD,		// 0xAE Illegal
    SC_BAD,		// 0xAF Illegal
    SC_MCOMDEF,		// 0xB0 Communal names definition
    SC_BAD,		// 0xB1 Illegal
    SC_BAD,		// 0xB2 Illegal
    SC_BAD,		// 0xB3 Illegal
    SC_MLOCEXD,		// 0xB4 External definition visible within module
			//	  only
    SC_MFLCEXD,		// 0xB5 Func ext definition visible within module
			//	  only
    SC_MLOCPUB,		// 0xB6 16-bit public definition visible within
			//	  module only
    SC_MLOCPUB,		// 0xB7 32-bit public definition visible within
			//	  module only
    SC_MLOCCOM,		// 0xB8 Communal name visible within module only
    SC_BAD,		// 0xB9 Illegal
    SC_BAD,		// 0xBA Illegal
    SC_BAD,		// 0xBB Illegal
    SC_BAD,		// 0xBC Illegal
    SC_BAD,		// 0xBD Illegal
    SC_BAD,		// 0xBE Illegal
    SC_BAD,		// 0xBF Illegal
    SC_BAD,		// 0xC0 Illegal
    SC_BAD,		// 0xC1 Illegal
    SC_MCOMDAT,		// 0xC2 Initialized communal data
    SC_MCOMDAT,		// 0xC3 Initialized communal data

	//@@@
	SC_MLINNUM,		// 0xC4 LINSYM16
	SC_MLINNUM,		// 0xC5 LINSYM32
};

//********************************************************
// Function: opnobj - Open a OBJ file and read first block
// Returned: Nothing
//********************************************************

void opnobj( struct objblk *opnt )
{
    uchar byte;

    if (debugflag)
        printf("file: %s\n", opnt->obj_fsp);

    if ((objdd = svcIoOpen(O_IN, opnt->obj_fsp, 0)) < 0) // Open file
        femsg(prgname, objdd, opnt->obj_fsp); // If error

    curobj = opnt;
    objblk = -1;			// Read first block
    objcnt = 0;
    objbuf = objloc;			// Make sure section count does not
    seccnt = 127;			//   go to 0 here
    if ((byte = getbyte()) == 0x80)	// MS format OBJ file?
    {
        objtype = OT_MS;		// Yes
        modtype = OT_MS;
        lowfirst = TRUE;		// Indicate low-order byte first
        objpnt--;			// Put the byte back
        objcnt++;
    }
    else if (byte == 0x84)		// XOS format OBJ file?
    {
        objtype = OT_XOS;
        modtype = OT_XOS;
        byte = getbyte();		// Yes - get next byte
        if (byte > 1) 			// Legal value?
            badfmt();			// No
        libflag = (byte == 1);
    }
    else
        badfmt();			// Bad value - loose!
}

//***********************************************************
// Function: setobj - setup to read next module from OBJ file
// Returned: Nothing
//***********************************************************

void setobj(void)

{
    uint byte;

    if (objtype != OT_MS)		// MS type OBJ file?
    {
        seccnt = 127;			//   No
        byte = getbyte();		// Get next byte
        if (byte == 0xC1)		// MS type module?
        {
            lowfirst = TRUE;
            modtype = OT_MS;
	    if (msbuffer == NULL)
		msbuffer = (uchar *)getspace(1024);
        }
        else if (byte == 0x80)		// High order byte first XOS module?
        {
	    lowfirst = FALSE;		// Yes
            modtype = OT_XOS;
        }
        else if (byte == 0x81)		// No - low order byte first XOS
        {				//   module?
            lowfirst = TRUE;		// Yes
            modtype = OT_XOS;
        }
        else
            badfmt();			// No - error
        byte = getbyte();		// Get the processor type
        if (ptype == 0 && byte != 0xFF)	// Is this the first OBJ file?
            ptype = byte;		// Yes - it sets processor type for
    }					//   the image if one was given
    else
    {
        lowfirst = TRUE;
        if (ptype == 0)
            ptype = 5;
        if (msbuffer == NULL)
            msbuffer = (uchar *)getspace(1024);
    }
    seccnt = 0;				// Reset sector byte count
}

//************************************
// Function: closeobj - Close OBJ file
// Returned: Nothing
//************************************

void closeobj(void)

{
    long lrtn;

    if ((lrtn=svcIoClose(objdd, 0)) < 0) // Close the file
	femsg(prgname, lrtn, curobj->obj_fsp);
}

//******************************************************************
// Function: getsec - Advance to start of next section, read header,
//	setup the byte count, and return section type
// Returned: Section type
//******************************************************************

int getsec(void)

{
    if (seccnt != 0)			// Anything to skip?
	skpbyte(seccnt);		// Yes - skip it
    seccnt = 127;			// Ensure section count doesn't run
					//   out while reading the header
    if ((sectype = getbyte()) != 0)	// Get section type
    {
        sectypx = sectype;
        if (modtype == OT_XOS)		// XOS module?
        {
            if (sectype > SC_XOSMAX)	// Yes - legal section type?
                badsec();		// No!
        }
        else				// If MS module
        {
            if (sectype < MSSC_MIN || sectype > MSSC_MAX) // Legal record
							  //   type?
                badsec();		// No!
            else			// Yes - get internal type
                sectype = mstypetbl[sectype-MSSC_MIN];
            if (sectype != SC_MFIXUPP)	// Fix-up record?
                msdispatch();		// No - do any pending clean-up
        }
        seccnt = getword();		// Get section size
    }
    else
    {
        sectypx = 0;
        seccnt = 0;
    }
    return (sectype);
}

//*******************************************
// Function: getbyte - Get byte from OBJ file
// Returned: Value
//*******************************************

ulong getbyte(void)

{
    if (--objcnt < 0)			// Anything in buffer now?
    {
	++objblk;			// No - bump block number
	objamnt = readblk();		// Read next block
        if ((objcnt = objamnt - 1) < 0)
            fail("Unexpected eof");
	objpnt = objbuf;		// And pointer
    }
    if (seccnt == 0)			// Reduce section count
	endofsec();
    --seccnt;
    return (*objpnt++);
}

//*******************************************
// Function: getbyte - Get word from OBJ file
// Returned: Value
//*******************************************

ulong getword(void)

{
    ulong first;
    ulong second;

    first = getbyte();			// Get first byte
    second = getbyte();			// Get second byte
    if (lowfirst)			// Low order byte first?
	return (first + (second << 8));	// Yes
    else
	return (second + (first << 8));	// No
}

//*******************************************
// Function: getbyte - Get long from OBJ file
// Returned: Value
//*******************************************

ulong getlong(void)

{
    ulong first;
    ulong second;

    first = getword();			// Get first word
    second = getword();			// Get second word
    if (lowfirst)			// Low order byte first?
	return (first + (second << 16)); // Yes
    else
	return (second + (first << 16)); // No
}

//************************************************************
// Function: getnumber - Get XOS format variable length number
// Returned: Value
//************************************************************

ulong getnumber(void)

{
    ulong value;

    value = getbyte();			// Get first byte
    if (!(value & 0x80L))		// 8 bit value?
        return (value);			// Yes - return it
    value = ((value&0x7F) << 8) + getbyte(); // No - get next 8 bits

    if (!(value & 0x4000L))		// 16 bit value?
        return (value);			// Yes - return it
    value = ((value&0x3FFF) << 8) + getbyte(); // Get next 8 bits

    if (!(value & 0x200000L))		// 24 bit value?
        return (value);			// Yes - return it
    return (((value&0x1FFFFF) << 8) + getbyte());
}

//**********************************************************
// Function: getmsnum - Get MS format variable length number
// Returned: Value
//**********************************************************

ulong getmsnum(void)

{
    ulong value;

    value = getbyte();			// Get first byte
    if (!(value & 0x80))		// 8 bit value?
        return (value);			// Yes - return it
    return (((value&0x7F) << 8) + getbyte()); // No - return 16-bit value
}

//*************************************************************
// Function: getmsxnum - Get MS extended format variable length
//		number (used when defining communal items)
// Returned: Value
//*************************************************************

ulong getmsxnum(void)

{
    ulong value;

    value = getbyte();			// Get first byte
    switch (value)
    {
     case 0x81:
        return (getword());

     case 0x84:
        value = getword();
        return (value + (getword()<<16));

     case 0x88:
        return (getlong());

     default:
        return (value);
    }
}

//********************************************
// Function: getsym - Get symbol from OBJ file
// Returned: Nothing
//********************************************

void getsym(void)

{
    int   cnt;
    char *pnt;

    pnt = symbuf + 1;			// Point to symbol buffer
    cnt = SYMMAXSZ;
    while (!((*pnt=getbyte()) & 0x80))	// Get symbol character
    {
	if (--cnt <= 0)			// Symbol too long?
	    fail("Symbol name is too long");
	++pnt;
    }
    pnt[0] &= 0x7F;
    pnt[1] = '\0';
    symsize = pnt - symbuf + 1;
}

//*************************************************************
// Function: getmssym - Get MS format symbol name from OBJ file
// Returned: Nothing
//*************************************************************

void getmssym(void)

{
    int   cnt;
    char *pnt;

    if ((cnt = getbyte()) > SYMMAXSZ)
        fail("Symbol too long");
    symsize = cnt + 1;
    pnt = symbuf + 1;
    while (--cnt >= 0)
        *pnt++ = getbyte();
    *pnt = 0;
}

//*********************************************************
// Function: getitem - Get variable size item from OBJ file
// Returned: Value
//*********************************************************

ulong getitem(
    char size)			// Size and extension flag: 
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
	    return (0);
     case 1:
	if (size & 0x04)
	    return (getbyte() | 0xFFFFFF00L);
	else
	    return (getbyte());
     case 2:
	if (size & 0x04)
	    return (getword() | 0xFFFF0000L);
	else
	    return (getword());
     case 3:
	    return (getlong());
    }
    return (0);				// Should never get here!
}

//************************************************
// Function: skipbyte - Skip bytes in the OBJ file
// Returned: Nothing
//************************************************

void skpbyte(
    int amnt)

{
    if ((seccnt -= amnt) < 0)
        endofsec();
    setblk((objblk<<9) + (objpnt - objbuf) + amnt);
}

//***************************************************
// Function: setblk - Setup to start reading OBJ file
// Returned: Nothing
//***************************************************

void setblk(
    long offset)

{
    long block;

    block = offset >> 9;		// Get block number
    offset &= 0x1FF;			// Get offset in block
    if (block != objblk)		// Is this block in our buffer now?
    {
	objblk = block;			// No - read it
	objamnt = readblk();
    }
    if ((objcnt = objamnt - offset) < 0)
        fail("Unexpected eof");
    objpnt = objbuf + (int)offset;	// Set pointer
}

//******************************************************
// Function: readblk - Read block from disk for OBJ file
// Returned: Number of bytes read
//******************************************************

int readblk(void)

{
    long rtn;

    if (((rtn=svcIoSetPos(objdd, objblk<<9, 0)) < 0)
            || ((rtn=svcIoInBlock(objdd, (char *)objbuf, 512)) <= 0))
    {
        if (rtn == 0 || rtn == ER_EOF)
            fail("Unexpected eof");
        else
            femsg(prgname, rtn, curobj->obj_fsp);
    }
    return (rtn);
}

//*****************************************************
// Function: usefif - Setup name of first input file as
//		name of an output file
// Returned: Pointer to name
//*****************************************************

char *usefif(
    char *ext)

{
    long  cnt;
    char  chr;
    char *pnt;

    if (firstname == NULL)
    {
        pnt = firstobj->obj_fsp;	// Point to name of first input file
        cnt = 0;			// See how long it is
        while ((chr=*pnt++) != '\0' && chr != '.')
	    ++cnt;
        pnt = getspace(cnt + 5);	// Get memory for file specification
        firstext = strnmov(pnt, firstobj->obj_fsp, (int)cnt);
        firstname = pnt;
    }
    strmov(firstext, ext);
    return (firstname);
}

//************************************************************
// Function: endofsec - Report unexpected end of section error
// Returned: Never returns
//************************************************************

void endofsec(void)

{
    fail("Unexpected end of section");
}

//***********************************************
// Function: badsec - Report illegal section type
// Returned: Never returns
//***********************************************

void badsec(void)

{
    char buf[40];

    sprintf(buf, "Illegal section type %d", sectypx);
    fail(buf);
}

//************************************************************
// Function: badfmt - Report illegal format in OBJ file errors
// Returned: Never returnes
//************************************************************

void badfmt(void)

{
    fail("Illegal format");
}

//*********************************************************
// Function: fail - Report fatal errors with simple message
// Returned: Nothing
//*********************************************************

void fail(msg)
char *msg;

{
    char fmt[] = "? XLINK: %s at offset 0x%lX";

    fprintf(stderr, fmt, msg, ((long)objblk) * 512 + (long)(objpnt-objbuf));
    if (mapflag)
        fprintf(mapfp, fmt, msg, ((long)objblk) * 512 + (long)(objpnt-objbuf));
    showfile(9);
    exit(1);
}
