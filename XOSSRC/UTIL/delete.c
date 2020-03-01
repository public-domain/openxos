//------------------------------------------------------------------------
// DELETE.C - File delete utility for XOS
//
// Revision history
// ??/??/?? 1.0 - (???) Initial development
// ??/??/?? 3.0 - (???) Updated to 32 bit
// 03/08/95 3.1 - (SAO) Added progasst package
// 03/13/95 3.2 - (sao) Changed 'option' indicator
// 03/16/95 -.- - (sao) Changed exit codes to reflect ALGRTN
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp(). Changed setup for dirscan() so
//                 that wildcards are NOT the default - this fixed problem
//                 with deleting "xxx.*" when "xxx" was specified.
//  5Jun95 (fpj) - Re-ordered option table.
//
//------------------------------------------------------------------------

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
#include <XOSERR.H>
#include <XOSRTN.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSTRM.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <DIRSCAN.H>
#include <GLOBAL.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>

// Local defines

#define VERSION 3
#define EDITNO  4

void chkpause(void);
int  comp(struct filespec *one, struct filespec *two);
void dofiles(void);
void errormsg(char *, long);
void helpprint(char *helpstring, int state, int newline);
int  hvellip(void);
int  nonopt(char *arg);
int  optconfirm(arg_data *arg);
int  optquiet(arg_data *arg);
int  optsort(arg_data *arg);
int  procfile(void);
int  prompt(char *msg);


long  sorted = TRUE;            // Sort file list if TRUE
long  quiet = FALSE;            // No output unless error if TRUE
long  mute = FALSE;             // Suppress all mesages
long  confirm = FALSE;          // Confirm before deleting if TRUE

arg_spec options[] =
{
    {"S*ORT",      ASF_BOOL|ASF_STORE, NULL, &sorted , TRUE, "Sort the file list."},
    {"C*ONFIRM",   ASF_BOOL|ASF_STORE, NULL, &confirm, TRUE, "Require confirmation before deleting.  This option over-rides both quiet and mute."},
    {"Q*UIET",     ASF_BOOL|ASF_STORE, NULL, &quiet  , TRUE, "Only errors are reported."},
    {"M*UTE" ,     ASF_BOOL|ASF_STORE, NULL, &mute   , TRUE, "No messages.  Error code reported on exit."},
    {"H*ELP",      0, NULL, AF(opthelp) , 0, "This message."},
    {"?",          0, NULL, AF(opthelp) , 0, "This message."},
    {NULL,         0, NULL, AF(NULL)     , 0, NULL}
};

struct
{   byte4_parm  filoptn;
    lngstr_parm filspec;
    byte2_parm  srcattr;
    byte2_parm  filattr;
    byte4_parm  dirhndl;
    char        end;
} fileparm =
{   {PAR_SET|REP_HEXV, 4 , IOPAR_FILOPTN , FO_XOSNAME|FO_NODENUM|FO_NODENAME|
            FO_RXOSNAME|FO_PATHNAME|FO_FILENAME|FO_VERSION|FO_ATTR},
    {PAR_GET|REP_STR , 0 , IOPAR_FILSPEC , NULL, 0, FILESPCSIZE+10, 0},
    {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR , A_NORMAL},
    {PAR_GET|REP_HEXV, 2 , IOPAR_FILATTR , 0},
    {PAR_SET|REP_HEXV, 4 , IOPAR_DIRHNDL , 0},
};

DIRSCANDATA dsd =
{   (DIRSCANPL *)&fileparm,	// parmlist - Address of parameter list
    FALSE,			// repeat   - TRUE if should do repeated search
    FALSE,			// dfltwild - TRUE if want wildcard as default
    QFNC_DEVPARM,		// function - Value for low byte of QAB_FUNC
    DSSORT_ASCEN,		// sort     - Directory sort order
    (int (*)(DIRSCANDATA *))hvellip,
				// hvellip  - Report use of ellipsis
    (int (*)(DIRSCANDATA *))procfile,
				// func     - Function called for each match
    errormsg			// error    - Function called on error
};

int   maxname;			// Maximum name length
int   left;			// Number of lines left on screen
int   total;			// Total number of files deleted

// Misc. variables

Prog_Info pib;
char  copymsg[] = "";
#ifndef DELETE
char  prgname[] = "DELETE";
char  envname[] = "^XOS^DELETE^OPT";
char  example[] = "{/options} file-list";
char  description[] = "The file(s) specified are deleted.  Once they are " \
    "removed, they can no longer be accessed, and the space they occupied " \
    "on the disk will be reused by XOS.  Wildcard and ellipsis " \
    "characters are allowed.  DELETE can accept more than one filename or " \
    "wildcard per command line.";
#else
char  prgname[] = "ERASE";
char  envname[] = "^XOS^ERASE^OPT";
char  example[] = "{/options} file-list";
char  description[] = "The file(s) specified are deleted.  Once they are " \
    "removed, they can no longer be accessed, and the space they occupied " \
    "on the disk will be reused by XOS.  Wildcard and ellipsis " \
    "characters are allowed.  ERASE can accept more than one filename or " \
    "wildcard per command line.";
#endif
char *pathbufr;
long  grandtotal = 0;		// Total number of files processed
char  nameseen;			// TRUE if saw a file name on the cmd line
char  fileseen;			// TRUE if file found
char  doall;			// TRUE if should not prompt any more
char  ellipok;			// TRUE if ellipsis has been confirmed

struct filespec
{
    struct filespec *next;	// Must be first item in structure
    struct filespec *sort;	// For heapsort
    long   error;		// Error associated with file
    char   filespec[1];		// File specification
};

struct filespec *firstfile;	// Pointer to first file to process
struct filespec *lastfile;	// Pointer to last file to process

struct argstr
{
    struct argstr *next;
    char   argstr[1];		// Argument string
};

struct argstr *firstarg;
struct argstr *lastarg = (struct argstr *)&firstarg;

struct
{   byte4_parm  options;
    lngstr_parm spec;
    char        end;
} openparm =
{
    {(PAR_SET|REP_HEXV), 4, IOPAR_FILOPTN, FO_XOSNAME|FO_PATHNAME},
    {(PAR_GET|REP_STR),  0, IOPAR_FILSPEC, NULL, 0, FILESPCSIZE, 0},
    0
};

//**********************************
// Function: main - Main program
// Returned: 0 if normal, 1 if error
//**********************************

int main(
    int argc,
    char *argv[])

{
    char      *envargs[2];
    struct    argstr *nextarg;
    char      strbuf[256];	// String buffer
    char      filspec[FILESPCSIZE+10];

	reg_pib(&pib);

	init_Vars();

    pathbufr = getspace(1024);
    fileparm.filspec.buffer = filspec;

    // Check global configuration parameters and enviroment variable

    global_parameter(TRUE);
    if (gcp_dosquirk)
	quiet = TRUE;
    if (gcp_dosdrive)
        fileparm.filoptn.value ^= (FO_DOSNAME|FO_XOSNAME|FO_RDOSNAME|
                FO_RXOSNAME);

    if(svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
    {
	envargs[0] = strbuf;
	envargs[1] = '\0';
    progarg(envargs, 0, options, NULL, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    // Process the command line

    ++argv;
    progarg(argv, PAF_PSWITCH, options, NULL, nonopt,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    if (argc < 2 || firstarg==NULL)			// Have any arguments?
	{
		if (!mute) fprintf(stderr,"? ERROR %s: A file specification must be provided.\n",prgname);
		exit(EXIT_FNF);
	}
    else
    {
        left=pib.screen_height-2;
        if (!confirm)
            dsd.function = QFNC_DELETE;
        do
        {
            nameseen = TRUE;
            fileseen = FALSE;
            dirscan(firstarg->argstr, &dsd); // Scan the directory
            if (!fileseen)
            {
                dofiles();
                if ( !mute )
                    fprintf(stdout, "? DELETE: File not found, %s\n",
                        firstarg->argstr);
                exit(EXIT_FNF);
            }
            nextarg = firstarg->next;
            free(firstarg);
        } while ((firstarg = nextarg) != NULL);
    }
    dofiles();
    --left;
    chkpause();
    if ( !mute && !quiet )
        printf("%d file%s deleted\n", total, (total != 1)? "s": "");
    return (0);
}

//***************************************************
// Function: init_Vars - Set up the program information block
// Returned: none
//***************************************************
void init_Vars(void){

	// set Program Information Block variables
	pib.opttbl=options; 		// Load the option table
    pib.kwdtbl=NULL;
	pib.build=__DATE__;
	pib.majedt = VERSION; 			// major edit number
	pib.minedt = EDITNO; 			// minor edit number
	pib.copymsg=copymsg;
	pib.prgname=prgname;
	pib.desc=description;
	pib.example=example;
	pib.errno=0;
	getTrmParms();
	getHelpClr();
};

//************************************************
// Function: nonopt - Process a file specification
// Returned: TRUE if OK, does not return if error
//************************************************

int nonopt(
    char *arg)

{
    struct argstr *thisarg;

    if ((thisarg = (struct argstr *)malloc(sizeof(struct argstr) +
            strlen(arg) + 1)) == NULL)
    {
        if ( !mute )
            fputs("? DELETE: Not enough memory available\n", stderr);
        exit(EXIT_MALLOC);
    }
    strmov(thisarg->argstr, arg);
    thisarg->next = NULL;
    lastarg->next = thisarg;
    lastarg = thisarg;
    return (TRUE);
}

//**************************************************
// Function: dofiles - Process list of deleted files
// Returned: Nothing
//**************************************************

void dofiles(void)

{
    int    num;
    int    cnt;
    int    fill;
    char  *pnt;
    struct filespec *nextfile;
    char   linbufr[160];

    if (firstfile == NULL)
        return;

    if (sorted)
        firstfile = heapsort(firstfile,
                (int (*)(void *a, void *b, void *d))comp, NULL);


    if (confirm)
    {
        char namebufr[1024];

        pnt = strmov(namebufr, pathbufr);
        do
        {
            strmov(pnt, firstfile->filespec);
            switch(prompt(namebufr))
            {
             case 0:
                fputs(" skipped\n", stdout);
                break;

             case 1:
                if ((num = svcIoDelete(0, namebufr, NULL)) < 0)
                    femsg2(prgname, "Error deleting file", num, NULL);
                fputs(" deleted\n", stdout);
                break;
            }
            nextfile = firstfile->next;
            free(firstfile);
        } while ((firstfile = nextfile) != NULL);
    }
    else
    {
        if ( !mute && !quiet )
        {
            chkpause();

            printf("Following file%s deleted from %s\n",
                    (firstfile->next != NULL)? "s": "", pathbufr);
            maxname += 2;
            num = (pib.screen_width - 3)/maxname;  // Calculate number of names per line
            cnt = 0;
            pnt = NULL;
            do
            {
                if (--cnt <= 0)
                {
                    if (pnt != NULL)
                    {
                        strmov(pnt, "\n");
                        chkpause();
                        fputs(linbufr, stdout);
                    }
                    pnt = strmov(linbufr, "  ");
                    cnt = num;
                    fill = 0;
                }
                while (--fill >= 0)
                    *pnt++ = ' ';
                pnt = strmov(pnt, firstfile->filespec);
                fill = maxname - strlen(firstfile->filespec);
                nextfile = firstfile->next;
                free(firstfile);
            } while ((firstfile = nextfile) != NULL);
            strmov(pnt, "\n");
            chkpause();
            fputs(linbufr, stdout);
            maxname = 4;
        }
    }
}


//*******************************************
// Function: hvellip - Confirm use of ellipis
// Returned: TRUE
//*******************************************

int hvellip(void)

{
    if (ellipok || quiet || mute)
        return (1);
    fputs("% DELETE: Ellipsis delete selected, are you sure [Y/N]? ", stderr);
    for (;;)
    {
        switch (toupper(getch()))
        {
         case 'Y':
            fputs("\n", stderr);
            ellipok = TRUE;
            return (1);

         case 'N':
            fputs("\n", stderr);
            exit(EXIT_NORM);
        }
    }
}

//**************************************************************
// Function: procfile - Function called by dirscan for each file
// Returned: TRUE if should continue, FALSE if should terminate
//**************************************************************

int procfile(void)

{
    char  *pnt;
    struct filespec *thisfile;
    int    left;

    if (dsd.changed)
    {
        dofiles();

        pnt = strmov(pathbufr, dsd.devname);
        left = 1022-dsd.devnamelen;
        pnt = strnmov(pnt, dsd.rmtdata.nodename, left);
        if ((left -= dsd.rmtdata.nodenamelen) > 0)
        {
            pnt = strnmov(pnt, dsd.rmtdata.rmtdevname, left);
            if ((left -= dsd.rmtdata.rmtdevnamelen) > 0)
                pnt = strnmov(pnt, dsd.pathname, left);
        }
        *pnt = '\0';			// Make sure have null at end
        dsd.changed = 0;
    }

    fileseen = TRUE;

    if (dsd.error < 0 &&
            (dsd.error != ER_FILAD && dsd.error != ER_FBFER &&
            dsd.error != ER_FBFER && dsd.error != ER_FBWER &&
            dsd.error != ER_BUSY))
    {
        char buffer[100];

        if ( !mute )
            errormsg(buffer, dsd.error);
        return (FALSE);
    }

    if (dsd.filenamelen == 0)		// Finished here if no name
        return (TRUE);

    ++total;
    if ((thisfile = (struct filespec *)malloc(sizeof(struct filespec) +
            dsd.filenamelen + 1)) == NULL)
    {
        if ( !mute )
            fputs("? DELETE: Not enough memory available\n", stderr);
        exit(EXIT_MALLOC);
    }
    if (maxname < dsd.filenamelen)
        maxname = dsd.filenamelen;

    strmov(thisfile->filespec, dsd.filename);
    if (firstfile == NULL)
        firstfile = thisfile;
    else
        lastfile->next = thisfile;
    lastfile = thisfile;
    thisfile->next = NULL;
    return (TRUE);
}

//****************************************************
// Function: comp - Compare two filenames for heapsort
// Returned: Negative if a < b
//	     Zero if a == b
//           Positive if a > b
//****************************************************

int comp(
    struct filespec *one,
    struct filespec *two)

{
    char *pnt1;
    char *pnt2;
    char  chr1;
    char  chr2;

//    if (revsort)			/* Want reverse order sort? */
//    {
//        struct filespec *temp;
//
//        temp = one;
//        one = two;
//        two = temp;
//    }

    pnt1 = one->filespec;
    pnt2 = two->filespec;
    while (((chr1 = *pnt1++) == (chr2 = *pnt2++)) && chr1 != 0)
        ;
    if (chr1 == '\\' || chr1 == ':')
        chr1 = 0;
    if (chr2 == '\\' || chr2 == ':')
        chr2 = 0;
    return (chr1 - chr2);
}

//**********************************************
// Function: prompt - Ask about deleting a file
// Returned: 1 if should delete, 0 if should not
//**********************************************

int prompt(
    char *msg)

{
    char display;

    if (doall)
    {
        fputs(msg, stderr);
        return (1);
    }
    display = TRUE;
    for (;;)
    {
        if (display)
        {
            fprintf(stderr, "Delete %s [Y,N,G,Q,?] ?", msg);
            display = FALSE;
        }
        switch (toupper(getch()))
        {
         case 'G':
            doall = TRUE;
         case 'Y':
            return (1);

         case 'N':
            return (0);

         case 'Q':
            fputs("\n", stderr);
            exit(EXIT_NORM);

         case '?':
            fputs("\n\n  Enter one of the following single characters:\n"
                    "    Y = Yes, delete the file\n"
                    "    N = No, do not delete the file\n"
                    "    G = Go, delete this and all following files\n"
                    "    Q = Quit, do not delete this or any following files\n"
                    "    ? = Display this text\n\n", stderr);
            display = TRUE;
        }
    }
}

//********************************************
// Function: chkpause - Check for screen pause
// Returned: Nothing
//********************************************

void chkpause(void)

{
    char msg;

    if ( mute || quiet )
        return;

    if (!pib.console || --left > 0)
        return;
    msg = TRUE;
    for (;;)
    {
        if (msg)
        {
            fputs("\33[7m-MORE- G, H, ?, <Enter> or <Space>\33[0m", stdout);
            msg = FALSE;
        }
        switch (toupper((getch()) & 0x7F))
        {
         case 'G':			// Don't ask for any more
            pib.console = FALSE;
         case '\r':
         case ' ':			// Do another screen full
            fputs("\r\33[K", stdout);
            left = pib.screen_height - 2;
            return;

         case 'H':
         case '?':
            fputs("\r\33[K\33[7m"
                    "  G      - Go, don't ask for -MORE-\n"
                    "  H or ? - Help, this message      \n"
                    " <Enter> - Next screen             \n"
                    " <Space> - Next screen             \n", stdout);
            msg = TRUE;
            break;
        }
    }
}

//*******************************************
// Function: errormsg - Display error message
// Returned: Nothing
//*******************************************

void errormsg(arg, code)
char *arg;
long  code;
{
    char buffer[80];		// Buffer to receive error message

    fileseen = TRUE;
    if ( mute )
        return;
    svcSysErrMsg(code, 3, buffer); // Get error message
    fprintf(stderr, "\n? %s: %s", prgname, buffer);
    if (arg != NULL && *arg != '\0')	// Have returned name?
	fprintf(stderr, ", file %s", arg); // Yes - output it too
    fputc('\n', stderr);
}
