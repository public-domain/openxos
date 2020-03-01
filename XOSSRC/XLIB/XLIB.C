//**********************
// Main program for XLIB
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
#include <XCSTRING.H>
#include <MALLOC.H>
#include <XCMALLOC.H>
#include <PROCARG.H>
#include <XOS.H>
#include <XOSSVC.H>
#include "XLIB.H"
#include "XLIBEXT.H"

// 3.0 - 29-Dec-93
//	Changed to 32-bit code.
// 3.1 - 26-May-94
//	Added support for imported entires.
// 3.2 - 29-Nov-94
//	Fixed bug in handling single input OBJ file, was getting confused
//	about file being open after pass 1.

#define VERSION   3		// Version number
#define EDITNUM   2		// Edit number

// Allocate global data

char   prgname[] = "XLIB";	// Program name for error messages
char   envname[] = "^XOS^XLIB^OPT"; // The environment option name
int    cmdindex;		// Command index
int    symsize;			// Length of symbol in symbuf
int    outdd;			// Device descriptor for output file
int    pageno;			// Listing page number
int    headsz;			// Size of header in output file
long   firstmod;		// Offset to first module
long   modoffset;		// Offset to current module
char  *membtm;			// Start of dynmanic memory area
struct obj inplib;		// Data for reading main input library
struct obj inpfil;		// Data for reading input files
struct nameblk *firstname;	// Pointer to first OBJ data block
struct nameblk *prevname = (struct nameblk *)&firstname;
struct nameblk *thisname;
struct nameblk *libname;	// Pointer to name block for library
struct et       ethead;		// Pointer to first entry table entry
struct et      *prevet;		// Pointer to previous entry table entry
char  *listfsp;			// Pointer to list file specification
FILE  *outfp;			// Output file pointer
struct _mab memblk =
{   0x400000, 0x400000, 0x400000};
char   cmdline;			// TRUE if processing command line arguments
char   lowfirst;		// TRUE if byte order is low order byte first
char   havlname;		// TRUE if listing output specified
char   nocmd;			// TRUE if command options not allowed
char   briefflag;
char   verboseflag;
char   quietflag;
char   symbuf[SYMMAXSZ+2];	// Symbol name buffer
char   tempname[FNAMESZ+2];	// Name of actual output file
char   notstart[] = "First section in OBJ file not start section";

// Define stuff for procarg

int  arghave(char *arg);
int  listhave(arg_data *arg);
int  createhave(void);
int  updatehave(void);
int  extracthave(void);
int  briefhave(arg_data *arg);
int  verbosehave(arg_data *arg);
int  quiethave(arg_data *arg);
void helphave(void);

arg_spec options[] =
{
    {"L"        , ASF_LSVAL, NULL,                      listhave   , 0},
    {"LIS"      , ASF_LSVAL, NULL,                      listhave   , 0},
    {"LIST"     , ASF_LSVAL, NULL,                      listhave   , 0},
    {"CRE"      , 0        , NULL, (int (*)(arg_data *))createhave , 0},
    {"CREATE"   , 0        , NULL, (int (*)(arg_data *))createhave , 0},
    {"UPD"      , 0        , NULL, (int (*)(arg_data *))updatehave , 0},
    {"UPDATE"   , 0        , NULL, (int (*)(arg_data *))updatehave , 0},
    {"EXT"      , 0        , NULL, (int (*)(arg_data *))extracthave, 0},
    {"EXTRACT"  , 0        , NULL, (int (*)(arg_data *))extracthave, 0},
    {"NOVERBOSE", 0        , NULL,                      briefhave  , FALSE},
    {"NOV"      , 0        , NULL,                      briefhave  , FALSE},
    {"VERBOSE",   0        , NULL,                      verbosehave, TRUE},
    {"V"      ,   0        , NULL,                      verbosehave, TRUE},
    {"NOQUIET",   0        , NULL,                      quiethave  , FALSE},
    {"NOQ",       0        , NULL,                      quiethave  , FALSE},
    {"QUIET",     0        , NULL,                      quiethave  , TRUE},
    {"Q",         0        , NULL,                      quiethave  , TRUE},
    {"HELP",      0        , NULL, (int (*)(arg_data *))helphave   , 0},
    {"H",         0        , NULL, (int (*)(arg_data *))helphave   , 0},
    {"?",         0        , NULL, (int (*)(arg_data *))helphave   , 0},
    {NULL,        0        , NULL,                      NULL       , 0}
};

//***************************************
// Function: main - Main program for XLIB
// Returned: 0 if normal, 1 if error
//***************************************

main(
    int    argc,
    char **argv)

{
    char *apnt[2];
    char  strbuf[256];

    fprintf(stderr, "XLIB - version %d.%d\n", VERSION, EDITNUM);
    if (argc < 2)
        nooper();
    inplib.obj_loc = (char *)getspace(512);
    inpfil.obj_loc = (char *)getspace(512);
    if(svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
    {
	apnt[0] = strbuf;
	apnt[1] = '\0';
	procarg(apnt, PAF_INDIRECT, options, NULL, (int (*)(char *))NULL,
                (void (*)(char *, char*))NULL, (int (*)(void))NULL, ".DFT");
    }
    ++argv;
    procarg(argv, PAF_INDIRECT|PAF_ECHOINDIR, options, NULL, arghave,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, ".CMD");
    if (cmdindex == 0)
    {
        if (havlname)
            cmdindex = C_LIST;
        else
            nooper();
    }
    passone();				// Do first pass
    if (havlname)			// Want listing?
	list();				// Yes - do listing
    if (cmdindex != C_LIST)		// Listing only?
	passtwo();			// No - do second pass
    return (0);
}

//***************************************************
// Function: arghave - Process command line arguments
// Returned: Nothing
//***************************************************

int arghave(char *arg)

{
    thisname = (struct nameblk *)getfsp(offsetof(nameblk, nb_name), arg,
            ".OBJ");			// Get file specification
    thisname->nb_next = 0;		// Initialize the name block
    thisname->nb_size = 0;
    thisname->nb_offset = 0;
    if (libname == NULL)
        libname = thisname;
    else
    {
        prevname->nb_next = thisname;	// Link to previous name block
        prevname = thisname;
    }
    return (TRUE);
}

//******************************************
// Function: listhave - Process /LIST option
// Returned: Nothing
//******************************************

int listhave(arg_data *arg)

{
    havlname = TRUE;			// Remember that -LIST was given
    if (arg->flags & ADF_LSVAL)
        listfsp = getfsp(0, arg->val.s, ".LST"); // Get file specification
    return (TRUE);
}

//**********************************************
// Function: createhave - Process /CREATE option
// Returned: Nothing
//**********************************************

int createhave(void)

{
    chkconflict();
    cmdindex = C_CREATE;
    return (TRUE);
}

//**********************************************
// Function: updatehave - Process /UPDATE option
// Returned: Nothing
//**********************************************

int updatehave(void)

{
    chkconflict();
    cmdindex = C_UPDATE;
    return (TRUE);
}

//************************************************
// Function: extracthave - Process /EXTRACT option
// Returned: Nothing
//************************************************

int extracthave(void)

{
    chkconflict();
    cmdindex = C_EXTRACT;
    return (TRUE);
}

//************************************************
// Function: briefhave - Process /{NO}BRIEF option
// Returned: Nothing
//************************************************

int briefhave(arg_data *arg)

{
    briefflag = arg->data;
    return (TRUE);
}

//****************************************************
// Function: verbosehave - Process /{NO}VERBOSE option
// Returned: Nothing
//****************************************************

int verbosehave(arg_data *arg)

{
    verboseflag = arg->data;
    return (TRUE);
}

//************************************************
// Function: quiethave - Process /{NO}QUIET option
// Returned: Nothing
//************************************************

int quiethave(arg_data *arg)

{
    quietflag = arg->data;
    return (TRUE);
}

//******************************************
// Function: helphave - Process /HELP option
// Returned: Nothing
//******************************************

void helphave(void)

{
    fputs("? XLIB: /HELP not implemented yet!\n", stderr);
    exit(1);
}

//*********************************************
// Function: nooper - Report no operation error
// Returned: Never returns
//*********************************************

void nooper(void)

{
    fputs("? XLIB: No operation specified\n", stderr);
    exit(1);
}

//******************************************************
// Function: chkconflict - Check for conflicting options
// Returned: Nothing (does not return if conflict
//******************************************************

void chkconflict(void)

{
    if (cmdindex != 0)
    {
        fputs("? XLIB: Conflicting options specified\n", stderr);
        exit(1);
    }
}

//******************************************
// Function: getfsp - Get file specification
// Returned: Pointer to file specificaton
//******************************************

char *getfsp(
    int   offset,		// Offset from start of block for name
    char *str,			// File specification string
    char *dftext)		// Default extension

{
    struct nameblk *blk;
    char  *fsp;
    int    size;
    char   haveext;

    size = strlen(str);			// Get length of file spec
    haveext = (strchr(str, '.') != NULL); // See if have extension
    blk = (struct nameblk *)getspace(offset+size+((haveext)? 1: 5));
					// Get memory for OBJ block
    fsp = ((char *)blk) + offset;	// Point to place for file spec
    strcpy(fsp, str);			// Copy file spec
    if (!haveext)			// Have extension?
        strcpy(fsp + size, dftext);	// No - add default
    strupr(fsp);			// Make sure all upper case
    return ((char *)blk);
}

//****************************************************
// Function: getelmem - Allocate memory for entry list
// Returned: Pointer to memory allocated
//****************************************************

void *getelmem(
    int size)

{
    void *temp;

    if ((temp = sbrkxx(size, &memblk)) == NULL)
    {
        fputs("? XLIB: Cannot allocate memory for entry list\n", stderr);
        exit(1);
    }
    return (temp);
}
