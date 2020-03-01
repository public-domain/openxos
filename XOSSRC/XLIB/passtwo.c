//************************************
// Routines to do second pass for XLIB
//************************************
// Written by John Goltz
//************************************

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
#include <PROCARG.H>
#include <XOS.H>
#include <XOSERRMSG.H>
#include <XOSSVC.H>
#include "XLIB.H"
#include "XLIBEXT.H"

//***********************************
// Function: passtwo - Do second pass
// Returned: Nothing
//***********************************

void passtwo(void)

{
    struct et  *etp;
    struct obj *obj;
    char  *cpnt;
    long   lrtn;
    int    cnt;

    strcpy(strnmov(tempname, libname->nb_name, // Build name for output file
	    (int)(strchr(libname->nb_name, '.')-libname->nb_name)), ".TMP");
    if ((outfp = fopen(tempname, "wb")) == NULL)
	femsg(prgname, -errno, tempname);
    putbyte(0x84);			// Output file header
    putbyte(0x01);
    putbyte(0x81);			// Output header for entry list
    putbyte(0xFF);			//   module (low order byte first)
    etp = ethead.et_next;
    do
    {   putbyte(SC_INDEX);		     // Output section header for
	putword((int)etp->et_listsize + 4);  //   index section
        putlong(firstmod+etp->et_offset); // Output offset to module
	cnt = etp->et_listsize;
	cpnt = etp->et_entlist;
	while (--cnt >= 0)		// Output rest of section
	    putbyte(*cpnt++);
    } while ((etp = etp->et_next) != 0);
    putbyte(0);				// Output end of module "section"
    etp = ethead.et_next;
    do
    {   if (etp->et_name)		// Does this point to a file?
	{				// Yes - do we have it open now?
	    if (etp->et_name != inpfil.obj_name)
	    {				// No
                if (inpfil.obj_handle != 0) // Is a previous file still open?
                    closeobj(&inpfil);	// Yes - close it now
                openobj(&inpfil, etp->et_name); // Open next file
                if (etp->et_source)	// Is this from a library?
                    setblk(&inpfil, etp->et_source); // Yes
                obj = &inpfil;
            }
	}
	else				// Not a file - must position input
	{				//   library
	    setblk(&inplib, etp->et_source);
	    obj = &inplib;
	}
	startmod(obj);			// Set up to read module
        putbyte((obj->obj_modtype == MT_MS)? 0xC1: // Output module header
                ((lowfirst)? 0x81: 0x80));
        putbyte((unsigned)obj->obj_language);
        if (startsec(obj) !=		// Set up to read first section,
                ((obj->obj_modtype == MT_XOS)? SC_START: MSSC_THEADR))
            fail(obj, notstart);	//   must be a start section
        putbyte((obj->obj_modtype == MT_XOS)? SC_START: MSSC_THEADR);
					// Output section header
        putword((int)obj->obj_seccnt + 8); // We are adding 8 bytes here
        cnt = etp->et_modsize - 14 - obj->obj_seccnt;
        if (obj->obj_modtype == MT_XOS)
        {
            while (obj->obj_seccnt > 0)	// Copy module name
                putbyte((int)getbyte(obj));
            putlong(etp->et_time.time);	// Output creation date and time
            putlong(etp->et_time.date);
        }
        else
        {
            while (obj->obj_seccnt > 1)	// Copy module name
                putbyte((int)getbyte(obj));
            putlong(etp->et_time.time);	// Output creation date and time
            putlong(etp->et_time.date);
            getbyte(obj);		// Discard the checksum byte
            putbyte(0);			// Output dummy checksum byte
        }
	obj->obj_seccnt = etp->et_modsize; // Set count
	while (--cnt >= 0)
	    putbyte((int)getbyte(obj));	// Copy byte
	putbyte(0);			// Output EOM "section"
    } while ((etp = etp->et_next) != NULL);
    putbyte(0);				// Output EOF indication

    if (fclose(outfp) < 0)		// Close output file
	femsg(prgname, -errno, tempname);
    svcIoDelete(0, libname->nb_name, NULL); // Delete original file if there
    if ((lrtn=svcIoRename(0, tempname, libname->nb_name, NULL)) < 0)
					// And rename our output
	femsg(prgname, lrtn, libname->nb_name);
}

//**************************************************
// Function: putlong - Output long to output library
// Returned: Nothing
//**************************************************

void putlong(
    long value)

{
    putword((int)value);	// Output low order half
    putword((int)(value >> 16)); // Output high order half
}

//**********************************************************
// Function: putword - Output word (short) to output library
// Returned: Nothing
//**********************************************************

void putword(
    int value)

{
    putbyte(value);		// Output low order half
    putbyte(value >> 8);	// Output high order half
}

//*********************************************************
// Function: putbyte - Output byte (char) to output library
// Returned: Nothing
//*********************************************************

void putbyte(
    int value)

{
    if (putc(value, outfp) < 0)
	femsg(prgname, -errno, tempname);
}
