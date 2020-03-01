//-------------------------------------------------------------------------
// TYPE.C (TYPE and MORE)
// File display utility for XOS
//
// Written by: Bruce R. Nevins
//
// Edit History:
// 01/25/90(brn) - Clone from DIR
// 02/08/90(brn) - Cleaned up dead code, made PAUSE help line up, changed
//			default to NOPAUSE and NONAME, added check for
//			subdirectorys so we don't type them.
// 02/18/91(brn) - Fixed to not supply .* when no extension was given
// 03/28/91(brn) - Add global configuration support and auto screen size
//			handling.  Add MORE option to build more program.
// 04/21/91(brn) - Fix attempted typing of directory
// 05/18/91(brn) - Changed to support new ioinsinglep call.
// 06/26/91(brn) - Fixed display of XOS and DOS style error message
// 10/07/91(brn) - Truncate line that are too wide for the display
//		   Fix to not use wildcard lookups on full filenames
//		   Add support for network filenames
// 08/20/92(brn) - Change reference to global.h from local to library
// 02/22/94(brn) - Fixed Single character input mode change
// 05/12/94(brn) - Change version number for 32 bit
// 11/07/94(sao) - Fixed >132 char buffer over-run, changed fetch method
// 12/09/94(sao) - Changed initialization to use new procarg
// 12/27/94(sao) - Added new PROGASST (help) functions
// 12/30/94(sao) - Fixed bug in NOWRAP option (only 1st line didn't wrap)
// 03/01/95(sao) - Added progasst updates
// 05/16/95(sao) - Changed exit codes to reflect XOSRTN
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
//-------------------------------------------------------------------------

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
#include <TIME.H>
#include <CTYPE.H>
#include <SETJMP.H>
#include <XOSERR.H>
#include <XOSRTN.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSTRM.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <GLOBAL.H>
#include <XOSSTR.H>

// Local defines

#define SIZE 8*1024             // Size of buffer for returned filenames
#define FILNF -38

// Local function prototypes

struct  fndretrn *filescan(char *filename,
                          unsigned int mode,
                          struct fndretrn *prev);
struct  fndretrn *src_filesetup(char *filename);
int     comp(const void *a, const void *b);
void    dofiletype(struct  fndretrn *newfiles);
void    dorecurse(char fqfile[]);
int     pagecheck(int count);
void    interr(void);
void   *getmem(size_t size);
void    givemem(void *ptr);
char   *pathtrim(char pathstring[]);
void    xosmsg(char *arg, long  code);
int	nonopt(char *arg);

// Structure for file links

struct file_list
{
    struct file_list *next;
    char filename[FILESPCSIZE];
};

struct file_list *first_file_list;
struct file_list *last_file_list;
struct file_list *cur_file_list;

// Data structure returned from filescan

struct fndretrn
{
    struct fndretrn *next;      // next file block
    int    filattr;		// File attributes
    char   name[FILESPCSIZE];   // File name buffer
};

// Switch settings functions

extern long errno;

// Program control variables

long	page;			// Page flag
long	wrap;			// Wrap long lines
int     recurse;                // Do all lower subdirectories
long	names;			// List totals only, no file names
int     curline = 0;            // current line on the screen
char	prgname[]="TYPE";


#define AF(func) (int (*)(arg_data *))func

arg_spec options[] =
{   {"?"      , 0, NULL, AF(opthelp), 0, "This message"},
    {"H*ELP"   , 0, NULL, AF(opthelp), 0, "This message"},
    {"DOSQ*UIRK", ASF_BOOL|ASF_STORE, NULL, &gcp_dosquirk, TRUE, "If TRUE, DOS quirks will be in affect" },
    {"DOSD*RIVE", ASF_BOOL|ASF_STORE, NULL, &gcp_dosdrive, TRUE, "If TRUE, DOS drive IDs will be used" },
    {"N*AMES"  , ASF_BOOL|ASF_STORE, NULL, &names , TRUE, "If TRUE (NAMES), file names are displayed"},
    {"P*AUSE"  , ASF_BOOL|ASF_STORE, NULL, &page , TRUE, "If TRUE (PAUSE), output will be paged"},
    {"W*RAP"   , ASF_BOOL|ASF_STORE, NULL, &wrap  , TRUE, "If TRUE (WRAP), output will be wrapped on screen"},
    {NULL	  , 0, NULL, AF(NULL)	 , 0, 0}
};

// Misc. variables

    char   envname[41];		// The environment option name
    uint   fndattr;		// Find attribute
    char  *pnt;			// Pointer to current argv
    char  *wildcard;		// Pointer to wildcard string
    char   device[FILESPCSIZE];	// Current device name
    char   path[FILESPCSIZE];	// Current path
    long   dircount = 0;	// Count of nested directories
    long   rtn;			// Return value from XOS SVC
    int    nameseen;		// We saw a file name on the cmd line
    struct fndretrn *found;	// Pointer to the matching file struct

#ifndef snglparm
struct
{   byte4_parm modeo;
    byte4_parm modes;
    byte4_parm modec;
    char       end;
} snglparm =
{   {(PAR_GET|REP_HEXV), 4, IOPAR_TRMCINPMODE, 0},
    {(PAR_SET|REP_HEXV), 4, IOPAR_TRMSINPMODE, TIM_IMAGE},
    {(PAR_SET|REP_HEXV), 4, IOPAR_TRMCINPMODE, TIM_ECHO}
};
#endif

struct
{   byte4_parm modec;
    byte4_parm modeo;
    char       end;
} snglparm2 =
{   {(PAR_SET|REP_HEXV), 4, IOPAR_TRMCINPMODE, 0xFFFFFFFF},
    {(PAR_SET|REP_HEXV), 4, IOPAR_TRMSINPMODE, 0}
};

struct
{   byte4_parm  options;
    lngstr_parm spec;
    char        end;
} dirparm =
{   {(PAR_SET|REP_HEXV), 4, IOPAR_FILOPTN, FO_XOSNAME|FO_NODENUM|FO_RXOSNAME|
            FO_PATHNAME|FO_FILENAME|FO_VERSION},
    {(PAR_GET|REP_STR ), 0, IOPAR_FILSPEC, NULL, 0, SIZE, 0}
};

struct
{   byte4_parm  options;
    lngstr_parm spec;
    byte4_parm  dirhndl;
    byte2_parm  srcattr;
    byte2_parm  filattr;
    byte4_parm  length;
    char        end;
} fileparm =
{   {(PAR_SET|REP_HEXV), 4, IOPAR_FILOPTN, FO_XOSNAME|FO_NODENUM|FO_RXOSNAME|
            FO_PATHNAME|FO_FILENAME|FO_VERSION|FO_ATTR},
    {(PAR_GET|REP_STR ), 0, IOPAR_FILSPEC, NULL, 0, SIZE, 0},
    {(PAR_SET|REP_HEXV), 4, IOPAR_DIRHNDL, 0},
    {(PAR_SET|REP_HEXV), 2, IOPAR_SRCATTR, 0},
    {(PAR_GET|REP_HEXV), 2, IOPAR_FILATTR, 0},
    {(PAR_GET|REP_DECV), 4, IOPAR_LENGTH , 0}
};

type_qab fileqab =
{
    QFNC_WAIT|QFNC_DEVPARM,	// qab_open
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    0,				// qab_handle
    0,				// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    O_REPEAT,			// qab_option
    0,				// qab_count
    NULL, 0,			// qab_buffer1
    NULL, 0,			// qab_buffer2
    &fileparm, 0		// qab_parm
};

Prog_Info pib;

void main(
    int argc,
    char *argv[])

{
    char  strbuf[256];		// String buffer
    char *foo[2];

    // Set defaults

    reg_pib(&pib);
    init_vars( argv[0] );

    // Allocate a bunch of space

    dirparm.spec.buffer = fileparm.spec.buffer = getmem(SIZE);
    first_file_list = NULL;
    last_file_list = NULL;
    cur_file_list = NULL;

    // Check Global Configuration Parameters (Checks DOSQUIRK and DRIVE)

    global_parameter(TRUE);

    if(svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
    {
	foo[0] = strbuf;
	foo[1] = '\0';
	progarg(foo, 0, options, NULL, (int (*)(char *))NULL,
	(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    if (argc >= 2)
    {
	++argv;
	progarg(argv, 0, options, NULL, nonopt, (void (*)(char *, char *))NULL,
		(int (*)(void))NULL, NULL);
	}
	else
	    exit(EXIT_FNF);		// If file not found, return

	if (gcp_dosdrive)
	{
	    dirparm.options.value = FO_DOSNAME|FO_NODENUM|FO_NODENAME|
		    FO_RDOSNAME|FO_PATHNAME|FO_FILENAME|FO_VERSION;
	    fileparm.options.value = FO_DOSNAME|FO_NODENUM|FO_NODENAME|
		    FO_RDOSNAME|FO_PATHNAME|FO_FILENAME|FO_VERSION|FO_ATTR;
	}

    if (!nameseen && !found)
    {
        if ( gcp_dosquirk )
            printf("File not found\n");
        else
            xosmsg("",FILNF);
		exit(EXIT_FNF);			// if file not found, return
					//   with error
    }

    cur_file_list = first_file_list;
    do
    {
	if (strpbrk(cur_file_list->filename, "#*?") != NULL)
	{
	    found = filescan(cur_file_list->filename, fndattr, found);
	    if (nameseen && found)
		dofiletype(found);	// Start printing matching files

	    if (!found)
	    {
		if (gcp_dosquirk)
		    printf("File not found\n");
		else
		    xosmsg("", FILNF);
		exit(EXIT_FNF);		// if file not found, return
					//   with error
	    } // end of  IF NOT FOUD

	    if (recurse)
		dorecurse(cur_file_list->filename);
	} // END OF SPECIAL CHAR HANDLING
	else
	{
	    found = src_filesetup(cur_file_list->filename);
	    if (found)
	    {
		dofiletype(found);	// Start printing matching files
	    }
	    else
	    {
		if (gcp_dosquirk)
		    printf("File not found\n");
		else
		    xosmsg("", FILNF);
		exit(EXIT_FNF);		// if file not found, return
					//   with error
	    } // END OF ELSE (i.e. file not found)
	}
    } while ((cur_file_list = cur_file_list->next) != NULL);
    exit(EXIT_NORM);                    // Return with no error
}

// init_vars
// Initialize program information block (pib) and local variables

void init_vars( char *name )

{

    // Set defaults of control variables

    found = 0;				// Start with found = 0
    nameseen = FALSE;			// Assume no name was seen
#ifndef MORE
    page = FALSE;			// Page default
#else
    page = TRUE;			// Page default
#endif
    wrap = TRUE;			// Wrap long lines
    recurse = FALSE;			// Do all lower subdirectories
    names = FALSE;			// List totals only, no file names
    fndattr = (A_NORMAL | A_DIRECT);	// Start looking for all files
                                        //   and subdirectories

    // Set Program Information Block variables

    pib.opttbl=options; 		// Load the option table
    pib.majedt = 3; 			// Major edit number
    pib.minedt = 3; 			// Minor edit number
    pib.copymsg = "";
    pib.prgname = strupper(name);
    sprintf(envname,"^XOS^%s^OPT",strupper(name));
#ifndef MORE
    pib.desc = "This command copies the file(s) specified to the sessions's "
	    "controlling terminal.  Output will be paused, pending "
	    "instructions from the operator, at each screen page.  Wildcard "
	    "and ellipsis filenames are allowed.  Note that the only "
	    "difference between this command and the TYPE command is the "
	    "initial option defaults.";
#else
    pib.desc = "This command copies the file(s) specified to the sessions's "
	    "controlling terminal.  Wildcard and ellipsis filenames are "
	    "allowed.  Note that the only difference between this command and "
	    "the MORE command is the initial option defaults.";
#endif
    pib.example = " {/option} {/NOoption} file_list";
    pib.build = __DATE__;
    errno = 0;
    getHelpClr();
    getTrmParms();
}

// dofiletype
// print all matching filenames

void dofiletype(struct  fndretrn *newfiles)

{
    int    i;			// Counters
    int    line_size;		// Number of lines in current string
    int    filenum;		// Number of files found
    int    eof_seen;		// ^Z seen in string
    int    eol_pos;		// Length of incoming line
    int    line_cont;		// There is more to the line than read
    struct fndretrn **sortarry;	// Pointer to array of pointers
    struct fndretrn *fndtmp;	// Temp pointer
    char   lcl_name[FILESPCSIZE]; // Local File name buffer
    char   cur_line[135];	// Current line buffer
    FILE  *rdfile;		// File handle to use for the read
    char  *cp;			// Character pointer

    filenum = 0;
    fndtmp = newfiles;
    while (fndtmp)
    {
	filenum++;
	fndtmp = fndtmp->next;
    }

    sortarry = getmem(sizeof(struct fndretrn *) * filenum);

    fndtmp = newfiles;
    for (i = 0; i < filenum; i++)
    {
	sortarry[i] = fndtmp;
	fndtmp = fndtmp->next;
    }

    qsort((void *)sortarry, (size_t)filenum, (size_t)sizeof(void *), comp);

    for (i = 0; i < filenum; i++)
    {
	if ((sortarry[i]->filattr & A_DIRECT) != 0)
	{
	    strcat(sortarry[i]->name, "\\");
	    if ((strcmp(sortarry[i]->name, ".\\") == 0) ||
		    (strcmp(sortarry[i]->name, "..\\") == 0))
	    {
		if (fndattr == (A_NORMAL | A_DIRECT)) // skip on subdir only
		    continue;
	    }
	}
    }

    for (i = 0; i < filenum; i++)
    {
	if ((sortarry[i]->filattr & A_DIRECT) != 0)
	    continue;

	strcpy(lcl_name, device);
	strcat(lcl_name, path);
	strcat(lcl_name, sortarry[i]->name);

        if ((rdfile = fopen(lcl_name, "r")) !=NULL)
        {
            if ( ftell(rdfile)== (-1) )
            {
                fclose(rdfile);
                exit(EXIT_FNF);
            }
            if (names)
            {
                printf("\n\33[7m[%s]\33[0m\n", lcl_name);
                pagecheck(1);
            }
            eof_seen = FALSE;
		errno = 0;		// Start with no error condition
            while (fgets(cur_line, pib.screen_width-1, rdfile) != NULL &&
		    !eof_seen)
	    {				// Be sure that the string is null
					//   terminated
		cur_line[pib.screen_width]=0;
                if ((cp = strchr(cur_line, '\x1a')) != NULL)
                {
					// Found a ctrl-Z, end of file marker
					eof_seen = TRUE;
                    if (cp == cur_line)
                    {
					// EOF if first character in the buffer, no print
			break;
		    }
		    else
					// Adjust end of string
			*cp = '\0';
		}

		// If there isn't an '\n', see if one is needed

		line_cont = FALSE;
                if (strchr(cur_line,'\n') == NULL)
                {
					// No newline in buffer
		    line_cont = TRUE;
		    eol_pos=strlen(cur_line);
                    if (eol_pos < pib.screen_width)
                    {
			cur_line[eol_pos+1] = 0;
			cur_line[eol_pos] = '\n';
		    }
		}

		// Buffer is now cleaned up, go ahead and print it

		line_size = 1;
		if (pagecheck(line_size) == FALSE)
		    break;
		fprintf(stdout, "%s", cur_line);

                if (!wrap && line_cont)
                {
					// No wrap and there's more to the line,
					//   keep reading until flushed
		    while (line_cont && !eof_seen
			    && fgets(cur_line, sizeof(cur_line), rdfile)
			    != NULL)
		    {
					// Check for '\n' and Ctrl-Z
			if ((cp = strchr(cur_line, '\x1a')) != NULL)
			    eof_seen = TRUE;
			if ((cp = strchr(cur_line, '\n')) != NULL)
			    line_cont = FALSE;
		    }
		}
		memset(cur_line,0,pib.screen_width);
	    } // END OF WHILE FGETS

            if (errno != 0 && errno != -ER_EOF)
            {
		xosmsg(lcl_name, -errno);
		exit(EXIT_INVSWT);
	    }
	    fclose(rdfile);
	} // END OF IF FILE OPEN OK
	else
	    xosmsg(lcl_name, -errno);

	// ALL DONE

	while (newfiles)
	{
	    fndtmp = newfiles->next;
	    givemem(newfiles);		// Give up all storage used
	    newfiles = fndtmp;
	}
	givemem(sortarry);
    }
}

// filescan - scan a file directory for matching entries
//   with the FCB information

struct fndretrn *filescan(
    char    *filename,
    unsigned int mode,
    struct   fndretrn *prev)

{
    int      i;			// Index for string arrays
    long     dirdev;		// Directory handle
    struct   fndretrn *fndchn;	// Current block to fill
    struct   fndretrn *fndret;	// First block in chain of blocks
    unsigned char *cp;		// Character pointer

    fileparm.srcattr.value = mode;	// Set our search mode
					// Set our return string options
    fndchn = 0;				// Make sure we start with no blocks
    fndret = 0;				// Ditto

    cp = (unsigned char *)(dirparm.spec.buffer);
					// Point to device and path name
    *cp = '\0';				// In case we don't get a string

    if ((dirdev = svcIoOpen(O_ODF, filename, &dirparm)) < 0)
    {
        xosmsg(filename, dirdev);
        return (prev);
    }
    fileparm.dirhndl.value = dirdev;	// Use the directory handle to read

    // Save the device and path name

    if (*cp == FS_XOSNAME || *cp == FS_DOSNAME || *cp == FS_VOLNAME)
    {
        cp++;                   	// Point to start
        i = 0;                  	// Index into path string
        while (*cp != '\0' && *cp < FS_MIN)
        {
            if (*cp == FS_ESC)
                cp++;
            device[i++] = *cp++;
        }
        device[i] = '\0';
    }

    if (*cp == FS_NODENAME || *cp == FS_NODENUM || *cp == FS_RXOSNAME ||
	    *cp == FS_RDOSNAME || *cp == FS_RVOLNAME || *cp == FS_PATHNAME)
    {
        cp++;                   	// Point to start
        i = 0;                  	// Index into path string
        while (*cp != '\0' && (*cp < FS_MIN || *cp == FS_NODENAME
		|| *cp == FS_NODENUM || *cp == FS_RXOSNAME ||
		*cp == FS_RDOSNAME || *cp == FS_RVOLNAME || *cp == FS_PATHNAME))
        {
            if (*cp == FS_ESC)
            {
                cp++;
                path[i++] = *cp++;
            }
            else if (*cp < FS_MIN)
                path[i++] = *cp++;
            else
                cp++;
        }
        path[i] = '\0';
    }
    do
    {
	fileqab.qab_buffer1 = filename;
	if ((dirdev = svcIoQueue(&fileqab)) < 0 ||
		(dirdev = fileqab.qab_error) < 0)
	{
	    if (dirdev != ER_FILNF || (dirdev == ER_FILNF && !recurse))
		if (!gcp_dosquirk)
		    xosmsg(filename, dirdev);

	    if ((rtn = svcIoClose(fileparm.dirhndl.value, 0)) < 0)
		xosmsg(filename, rtn);	// Must close the open directory
	    return (prev);
	}

	cp = (unsigned char *)(fileparm.spec.buffer);
					// Point to device and path name
	while (*cp != '\0')
	{
#ifdef DEBUG
	    printf("*cp = |%s|\n", cp);
#endif
	    // Save the device and path name

	    if (*cp == FS_XOSNAME || *cp == FS_DOSNAME || *cp == FS_VOLNAME)
            {
                cp++;			// Point to start
                i = 0;			// Index into device string
                while (*cp != '\0' && *cp < FS_MIN)
                {
                    if (*cp == FS_ESC)
                        cp++;
                    device[i++] = *cp++;
                }
                device[i] = '\0';
            }
            else if (*cp == FS_NODENUM  || *cp == FS_NODENAME ||
		    *cp == FS_RXOSNAME || *cp == FS_RDOSNAME ||
		    *cp == FS_RVOLNAME || *cp == FS_PATHNAME)
            {
                cp++;               // Point to start
                i = 0;              // Index into path string

                while (*cp != '\0' && (*cp < FS_MIN ||
			*cp == FS_NODENUM  || *cp == FS_NODENAME ||
			*cp == FS_RXOSNAME || *cp == FS_DOSNAME  ||
			*cp == FS_RVOLNAME || *cp == FS_PATHNAME))
                {
                    if (*cp == FS_ESC)
                    {
                        cp++;
                        path[i++] = *cp++;
                    }
                    else if (*cp < FS_MIN)
                        path[i++] = *cp++;
                    else
                        cp++;
                }
                path[i] = '\0';
            }
            else if (*cp == FS_FILENAME)
            {
#ifdef DEBUG
                printf("*cp = |%s|\n", cp);
#endif
                cp++;
                i = 0;
                if (fndchn == 0)
                {
                    fndret=getmem(sizeof(struct fndretrn));
					// Start the beginning ptr
                    fndchn=fndret;	// Start the first block
                }
                else
                {
                    fndchn->next=getmem(sizeof(struct fndretrn));
                    fndchn=fndchn->next; // Make this the new end of list
                }
                while (*cp != '\0' && *cp < FS_MIN)
                {
                    if (gcp_dosquirk && *cp == '.')
                    {
                        if (i == 0)
                        {
                            if (*(cp + 1) == '.')
                                fndchn->name[i++] = *cp++;
                            cp++;
                            continue;
                        }

                        if (i >= 9)
                            fndchn->name[i++] = ' ';
                        else
                        {
                            for (;i < 9;)
                                fndchn->name[i++] = ' ';
                        }
                        cp++;
                    }
                    else
                        fndchn->name[i++] = *cp++;
                }
                fndchn->name[i] = '\0';     // Make a C string
                fndchn->next = prev;        // Make sure link is clear
#ifdef DEBUG
                printf("fndchn->name = |%s|\n", fndchn->name);
#endif
            }
            else if (*cp == FS_ATTR)
            {
                fndchn->filattr = *++cp; // Store the attribute

                // Must handle second attribute byte

                cp++;                   // Skip to next byte
                cp++;                   // Skip to next byte
            }
            else if (*cp == FS_VERSION)
            {
                cp++;

                fndchn->name[i++] = ';';
                while (*cp != '\0' && *cp < FS_MIN)
                {
                    fndchn->name[i++] = *cp++;
                }
                fndchn->name[i] = '\0'; // Make a C string
            }
            else
            {
	        cp++;		// Skip bad prefix
                while (*cp != '\0' && (*cp < FS_MIN || *cp == FS_ESC))
                {
                    cp++;
                    if (*cp == FS_ESC)
                    {
                        cp++;
                    }
                }
            }
        }
    } while (fileqab.qab_amount & 0x80000000L);

    if (dirdev < 0 && (dirdev != ER_FILNF ||
                (!recurse && fndret == 0)))
        xosmsg(filename, dirdev);

    if ((rtn = svcIoClose(fileparm.dirhndl.value, 0)) < 0)
            xosmsg(filename, rtn);            // Must close the open directory

    if (fndret == 0)
        return (prev);
    else
        return (fndret);
}

// src_filesetup - setup a single file name for a copy

struct fndretrn *src_filesetup(
    char *filename)

{
    struct fndretrn  *fndret;   // First block in chain of blocks
    char *cp1, *cp2;            // Character pointer

    fndret = 0;

    // Save the device and path name

    if ((cp1 = strrchr(filename, ':')) != NULL)
    {
        *strnmov(device, filename, (size_t)(cp1 - filename) + 1) = 0;
        cp1++;
    }
    else
        cp1 = filename;

    if ((cp2 = strrchr(cp1, '\\')) != NULL ||
	    (cp2 = strrchr(cp1, ']')) != NULL)
        strncpy(path, cp1, (size_t)(cp2 - cp1) + 1);
    else
        cp2 = cp1;

    fndret=getmem(sizeof(struct fndretrn));
                                        // Start the beginning ptr
    if ((cp2 = strrchr(cp1, '\\')) != NULL ||
	    (cp2 = strrchr(cp1, ']')) != NULL)
        strcpy(fndret->name, cp2 + 1);
    else
        strcpy(fndret->name, cp1);

    return (fndret);
}

// comp
// compare two filenames in the structure

int comp(
    const void *a,
    const void *b)

{
    char *afile, *bfile;

    afile = (*(struct fndretrn **)a)->name;
    bfile = (*(struct fndretrn **)b)->name;
    return (strcmp(afile, bfile));
}

// dorecurse
// do recursive list of files

void dorecurse(
    char fqfile[])

{
    int     i;			// Counters
    int     filenum;		// Number of files found
    char   *cp;                 // Misc character pointer
    char    nextfile[FILESPCSIZE]; // File buffer
    char    newpath[FILESPCSIZE]; // Path buffer
    struct  fndretrn **sortarry; // Pointer to array of pointers
    struct  fndretrn *fndtmp;	// Temp pointer
    struct  fndretrn *newfiles;	// New dirs to do?

    filenum = 0;
    strcpy(nextfile, device);	// Get current file device
    strcat(nextfile, path);	// Get current file path
    strcpy(newpath, nextfile);
    strcat(nextfile, "*.*");

    newfiles = filescan(nextfile, A_DIRECT, NULL);
    if (newfiles == NULL)
        return;

    fndtmp = newfiles;
    filenum = 0;
    while (fndtmp != NULL)
    {
        filenum++;
        fndtmp = fndtmp->next;
    }

    sortarry = getmem(sizeof(char *) * filenum);

    fndtmp = newfiles;
    for (i = 0; i < filenum; i++)
    {
        sortarry[i] = fndtmp;
        if ((sortarry[i]->filattr & A_DIRECT) != 0)
            strcat(sortarry[i]->name, "\\");

        fndtmp = fndtmp->next;
    }

    qsort((void *)sortarry, (size_t)filenum,
        (size_t)sizeof(void *), comp);

    for (i = 0; i < filenum; i++)
    {
        if (((sortarry[i]->filattr & A_DIRECT) != 0) &&
                (strcmp(sortarry[i]->name, ".\\") != 0) &&
                (strcmp(sortarry[i]->name, "..\\") != 0))
        {
            strcpy(nextfile, newpath);
            strcat(nextfile, sortarry[i]->name);
            if (((sortarry[i]->filattr & A_DIRECT) != 0) &&
        	    (strcmp(sortarry[i]->name, ".\\") != 0) &&
            	(strcmp(sortarry[i]->name, "..\\") != 0))
            {
                strcpy(nextfile, newpath);
                strcat(nextfile, sortarry[i]->name);
                if (((cp = strrchr(fqfile, '\\')) != NULL) ||
                    ((cp = strrchr(fqfile, ':'))  != NULL))
                {
                    if (cp[1] == '\0')
                    {
        	            strcat(nextfile, wildcard);
                    }
                    else
                    {
                        strcat(nextfile, ++cp);
                    }
                }
                else
                {
                    strcat(nextfile, fqfile);
                }			// pointer to file name to find
            }
            fndtmp = filescan(nextfile, fndattr, 0);
            if (fndtmp)
                dofiletype(fndtmp);
                        // Start printing matching files
            dorecurse(nextfile);
        }
    }

    while (newfiles)
    {
        fndtmp = newfiles->next;
        givemem(newfiles);              // Give up all storage used
        newfiles = fndtmp;
    }
    givemem(sortarry);
    return;
}

// pagecheck
// check the page count

int pagecheck(
    int count)			// Amount to increment before checking

{
    long temp;			// Temp int for keyboard status
    int more;
    char chr;

    curline += count;			// Add the new count
	if (pib.console && page && (curline >= pib.screen_height - 1))
    {
        more = TRUE;
        while (more == TRUE)
        {
	    if (gcp_dosquirk)
		printf("-- More --");
	    else
		printf("\33[7m-- More -- ^C, G, H, N, Q, <Enter> or "
			"<Space>\33[0m");
            temp = svcIoInSingleP(DH_STDTRM, &snglparm);

	    if (gcp_dosquirk)
	    {
		curline = 0;
		break;
	    }
            more = FALSE;		// Assume good input
            chr = (char)temp & 0x7f;	// Just 7 bits worth
            switch (chr)
            {
	     case 'N':
	     case 'n':
                curline = 0;
	        printf("\r\33[K");
		return (FALSE);

             case 'q':			// Quit!, don't print the rest
             case 'Q':
             case 0x03:
                printf("\r\33[K");
                exit(EXIT_NORM);

             case 'g':			// Don't ask for any more
             case 'G':
                page = FALSE;
                break;

             case '\r':			// Enter just does one line
             case ' ':			// Do another screen full
                curline = 0;
                break;

	     case 'h':
	     case '?':
             default:			// Tell him this is wrong
                printf("\r\33[K");
                printf("\33[7m ^C      - Exit                    \33[0m\n");
                printf("\33[7m  G      - Go, don't ask for -MORE-\33[0m\n");
                printf("\33[7m  H or ? - Help, this message      \33[0m\n");
                printf("\33[7m  N      - Skip to next file       \33[0m\n");
                printf("\33[7m  Q      - Quit program            \33[0m\n");
                printf("\33[7m <Enter> - Next screen             \33[0m\n");
                printf("\33[7m <Space> - Next screen             \33[0m\n");
                more = TRUE;            // Loop back and check again
            }
        }
        printf("\r\33[K");
    }
    return (TRUE);
}

// interr

void interr(void)

{
    fprintf(stderr, "? %s: Internal error - string parse failure\n",
			pib.prgname);
    exit(EXIT_INTERR);
}

// getmem
// Get memory with malloc, exit with error if out of memory

void *getmem(
    size_t size)

{
    void *ptr;

    ptr = malloc(size);
    if (ptr == NULL)
    {
	fprintf(stderr, "? %s: Not enough memory available\n", pib.prgname);
        exit(EXIT_MALLOC);
    }
#ifdef DEBUG
    fprintf(stderr, "DBUG - getmem returned with %lx\n", ptr);
#endif // DEBUG
    return (ptr);
}

// givemem
// Give up memory malloc, exit with error if out of memory

void givemem(
    void *ptr)

{
#ifdef DEBUG
    fprintf(stderr, "DBUG - givemem called with %lx\n", ptr);
#endif // DEBUG

    free(ptr);
}

// Function to display ALLEGRO error messages

void xosmsg(
    char *arg,
    long  code)

{
    char buffer[80];            // Buffer to receive error message

    svcSysErrMsg(code, 3, buffer);	// Get error message
	fprintf(stderr, "\n? %s: %s", pib.prgname, buffer);
					// Output error message
    if (*arg != '\0')                   // Have returned name?
        fprintf(stderr, "; %s\n", arg);
    else
        fprintf(stderr, "\n");
}

// Function to process non-option input

int nonopt(
    char *arg)

{
    char *cp;			// misc character pointers
    int   j;

    pnt = arg;
    nameseen = TRUE;
    if (first_file_list == NULL)
    {
	first_file_list = getmem(sizeof(struct file_list));
	last_file_list = first_file_list;
    }
    else
    {
	last_file_list->next = getmem(sizeof(struct file_list));
	last_file_list = last_file_list->next;
    }
    last_file_list->next = NULL;
    j = strlen(pnt);
    cp = &pnt[j - 1];
    strcpy(last_file_list->filename, pnt);

    if (*cp == '\\' || *cp == ':')
    {
        strcat(last_file_list->filename, wildcard);
    }
    if ((cp = strstr(last_file_list->filename, "...\\")) != NULL ||
        (cp = strstr(last_file_list->filename, "...\\")) != NULL)
    {
        strcpy((char *)cp, ((char *)cp + 4));
        recurse = TRUE;
    }
    if (strchr(last_file_list->filename, '.') == NULL)
        strcat(last_file_list->filename, ".");
    return (TRUE);
}
