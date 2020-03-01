//***********************************
// Routines to do first pass for XLIB
//***********************************
// Written by John Goltz
//***********************************

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
#include <TIME.H>
#include <XCSTRING.H>
#include <MALLOC.H>
#include <XCMALLOC.H>
#include <XOS.H>
#include "XLIB.H"
#include "XLIBEXT.H"

//**********************************************************************
// Function: passone - Do first pass for commands which output a library
//		(all commands except EXTRACT).  This function builds the
//		memory copy of the index section for the library to be
//		output, which also contains the location of each module
//		to copy to the output library.
// Returned: Nothing
//**********************************************************************

void passone(void)

{
    struct et *etp;
    int    sect;

    firstmod = 5;			// Initialize first module offset
    if (cmdindex != C_CREATE)		// Do we have an input library?
    {
	openobj(&inplib, libname);	// Yes - open it
	if (!inplib.obj_libflag)	// It must be a library!
	    fail(&inplib, "Main input file is not a library file");
	startmod(&inplib);		// Setup to read entry list module
	if (inplib.obj_language != 0xFF) // First module must be entry list
					 //   section
	    fail(&inplib, "First module in input library file is not an entry"
                    " list section");
        prevet = &ethead;
	while ((sect=startsec(&inplib)) == SC_INDEX)
            cpylibent(&inplib);
        if (sect != SC_EOM)
            fail(&inplib, "Illegal section in input library index module");

        etp = &ethead;
        while ((etp = etp->et_next) != NULL)
        {
            setblk(&inplib, etp->et_source);	// Position to start of module
            startmod(&inplib);		// Start reading module


            if ((sect = startsec(&inplib)) == SC_START)
                getsym(&inplib);	// Read module name
            else if (sect == MSSC_THEADR)
                getmssym(&inplib);
            else
                fail(&inplib, "First section in module in input library not a"
                        "start section");
            etp->et_modname = getspace(symsize + 1);
            strcpy(etp->et_modname, symbuf);
            etp->et_time.time = getlong(&inplib);
            etp->et_time.date = getlong(&inplib);
            if (etp->et_next == NULL)	// Is this the last module?
            {				// Yes - must read it to get size
                etp->et_modsize = symsize + 14 + inplib.obj_seccnt;
                while ((sect=startsec(&inplib)) != SC_EOM)
                    etp->et_modsize += inplib.obj_seccnt + 3;
            }
        }
        if (cmdindex == C_LIST)		// Listing only?
            return;			// Yes - finished here

        fputs("? XLIB: Function not implemented yet\n", stderr);
        exit(1);
    }

    prevet = &ethead;
    thisname = firstname;
    while (thisname)
    {
        do
        {   openobj(&inpfil, thisname); // Open input file
            if (inpfil.obj_libflag)	// Is it a library?
            {				// Yes
                startmod(&inpfil);	// Setup to read entry list module
                if (inpfil.obj_language != 0xFF) // First module must be
						 //   entry list section
                    fail(&inpfil, "First module in library file is not an"
                            " entry list section");
                while (inpfil.obj_sectype != SC_EOM) // End of module?
                {
//                  if (inpfil.obj_sectype != SC_ENTRY) // Entry section?
                        fail(&inpfil, "Illegal section type in library"
                                " entry list module");
                    cpylibent(&inpfil); // Copy entry to in-memory table
                }
            }
            else			// Input file is not a library
            {
                startmod(&inpfil);
                cpyfilent(&inpfil);	// Copy entry data for file
            }
            closeobj(&inpfil);		// Close the input file
        } while ((thisname = thisname->nb_next) != 0);
    }
}

//***************************************************************
// Function: cpylibent - Copy entry for next input library module
//		index into the in-memory index
// Returned: Nothing
//***************************************************************

void cpylibent(
    struct obj *obj)

{
    struct et *etp;
    char  *pnt;

    etp = (struct et *)getspace(obj->obj_seccnt + offsetof(et, et_entlist) + 1);
					// Get space for entry
    etp->et_source = getlong(obj);	// Get module offset
    etp->et_listsize = (int)(obj->obj_seccnt);
    pnt = etp->et_entlist;
    while (obj->obj_seccnt)		// Copy rest of section
        *pnt++ = getbyte(obj);
    prevet->et_modsize = etp->et_source - prevet->et_source;
    prevet->et_next = etp;
    prevet = etp;
    etp->et_next = NULL;
}

//***********************************************************************
// Function: cpyfilent - Setup in-memory index entry for non-library file
// Returned: Nothing
//***********************************************************************

void cpyfilent(
    struct obj *obj)

{
    struct    et *etp;
    char     *pnt;
    file_info fileinfo;
    long      modsize;
    int       namesize;
    int       sect;
    int       header;
    char      modname[SYMMAXSZ+2];

    if (((sect=startsec(obj)) != SC_START) && (sect != MSSC_THEADR))
	fail(obj, notstart);		// Start section must be first
    modsize = obj->obj_seccnt+2+3+8+1;	// 2 = Module header
					// 3 = Start section header
					// 8 = Date and time to be added
					//       to start section
					// 1 = Module trailer
    if (obj->obj_modtype == MT_XOS)
        getsym(obj);			// Get module name
    else
        getmssym(obj);
    strnmov(modname, symbuf, symsize);	// Save it for later
    namesize = symsize;
    etp = NULL;


    if (obj->obj_modtype == MT_XOS)
    {
        while ((sect=startsec(obj)) != SC_EOM)
        {
            modsize += (obj->obj_seccnt + 3);
            if ((sect == SC_INTERN || sect == SC_IMPORT) &&
                    obj->obj_seccnt != 0)
            {
                do
                {
                    header = getbyte(obj);
                    if (sect == SC_INTERN)
                    {
                        getitem(obj, header);
                        if (header & SYF_ADDR) // Address?
                        {
                            if (header & SYF_ABS) // Yes - get psect value or
                                getword(obj);     //   number
                            else
                                getnumber(obj);
                        }		// Not address - if not indicated as
                        else		//   ABS eat the extra byte which
                        {		//   old version of XMAC inserted!
                            if (!(header & SYF_ABS))
                                getbyte(obj);
                        }
                    }
                    getsym(obj);	// Get symbol name
                    if (header & 0x40)
                    {
                        if (etp == NULL)
                        {
                            etp = (struct et *)getelmem(offsetof(et, et_entlist)
                                    + symsize);
                            prevet->et_next = etp; // Link it into our list
                            prevet = etp;
                            etp->et_name = obj->obj_name;
                            etp->et_next = NULL;
                            etp->et_source = 0; // Indicate not a library
                            etp->et_listsize = 0;
                            getfinfo(obj->obj_handle, &fileinfo);
					// Get date and time for file
                            etp->et_time = fileinfo.create;
                            pnt = etp->et_entlist;
                        }
                        else
                            pnt = (char *)getelmem(symsize);
                        pnt = strnmov(pnt, symbuf, symsize);
                        *(pnt-1) |= 0x80;
                        etp->et_listsize += symsize; // Add in its size
                    }
                } while (obj->obj_seccnt > 0);
            }
        }
    }
    else
    {
        while ((sect=startsec(obj)) != MSSC_MODEND && sect != MSSC_386END)
        {
            modsize += (obj->obj_seccnt + 3);
            if ((sect == MSSC_PUBDEF || sect == MSSC_PUB386) &&
                    obj->obj_seccnt > 1)
            {
                if ((getmsnum(obj) == 0) & (getmsnum(obj) == 0))
                    getword(obj);	// Skip group and segment numbers
                do
                {
                    getmssym(obj);	// Get symbol name
                    if (sect == MSSC_PUBDEF)
                        getword(obj);
                    else
                        getlong(obj);
                    getmsnum(obj);	// Skip type index
                    if (etp == NULL)
                    {
                        etp = (struct et *)getelmem(offsetof(et, et_entlist)
                                + symsize);
                        prevet->et_next = etp; // Link it into our list
                        prevet = etp;
                        etp->et_name = obj->obj_name;
                        etp->et_next = NULL;
                        etp->et_source = 0; // Indicate not a library
                        etp->et_listsize = 0;
                        getfinfo(obj->obj_handle, &fileinfo);
					// Get date and time for file
                        etp->et_time = fileinfo.create;
                        pnt = etp->et_entlist;
                    }
                    else
                        pnt = (char *)getelmem(symsize);
                    pnt = strnmov(pnt, symbuf, symsize);
                    *(pnt-1) |= 0x80;
                    etp->et_listsize += symsize; // Add in its size
                } while (obj->obj_seccnt > 1);
            }
        }
    }
    if (etp != NULL)
    {
        pnt = getelmem(namesize + 4);
        *pnt++ = '\0';
        etp->et_modname = pnt;
        etp->et_namesize = namesize;
        strnmov(pnt, modname, namesize); // Copy module name
        etp->et_offset = modoffset;
        etp->et_modsize = modsize;	// Store module size
        firstmod += etp->et_listsize + 7; // Update first module
        modoffset += modsize;	// Update offset for next module
    }
    else
        printf("%% XLIB: No entries specified for module %s,\n"
               "        module not placed in library\n", symbuf);
}
