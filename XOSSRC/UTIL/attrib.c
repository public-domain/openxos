//--------------------------------------------------------------------------*
// ATTRIB.C - File attribute modification utility for Forte
//
// Written by: Tom Goltz
//
// Edit History:
// 04/15/91(tmg) - Created initial version
// 08/20/92(brn) - Change reference to global.h from local to library
// 05/12/94(brn) - Fixed command abbreviations
// 04/04/95(sao) - Added progasst package
// 05/13/95(sao) - Changed 'optional' indicators, added MUTE
// 05/16/95(sao) - Changed return codes to reflect XOSRTN
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
//  5Jun95 (fpj) - Re-ordered option table.
//--------------------------------------------------------------------------*

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
#include <XOSERR.H>
#include <XOSRTN.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <DIRSCAN.H>
#include <GLOBAL.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>

// Local defines

#define VERSION 3
#define EDITNO  5

#define REPLACE 1
#define ADD     2
#define REMOVE  3

int  comp(struct filespec *one, struct filespec *two);
void errormsg(char *, long);
int  nonopt(char *arg);
int  optattrib(arg_data *arg);
int  optowner(arg_data *arg);
int  optprot(arg_data *arg);
int  procfile(void);

#define AF(func) (int (*)(arg_data *))func

long  sorted = TRUE;        // Sort file list if TRUE
long  quiet = FALSE;        // No output unless error if TRUE
long  mute  = FALSE;		// Suppress all messages
long  confirm = FALSE;      // Confirm before changing attributes if TRUE

arg_spec options[] =
{   {"S*YSTEM"   , ASF_MINUS           , NULL,    optattrib ,~A_SYSTEM,"Modify (+=ON/-=OFF) the file's SYSTEM attribute."},
    {"S*YSTEM"   , ASF_PLUS            , NULL,    optattrib , A_SYSTEM,NULL},
    {"A*RCHIVE"  , ASF_MINUS           , NULL,    optattrib ,~A_ARCH,"Modify the file's ARCHIVE attribute."},
    {"A*RCHIVE"  , ASF_PLUS            , NULL,    optattrib , A_ARCH,NULL},
    {"H*IDDEN"   , ASF_MINUS           , NULL,    optattrib ,~A_HIDDEN,"Modify the file's HIDDEN attribute."},
    {"H*IDDEN"   , ASF_PLUS            , NULL,    optattrib , A_HIDDEN,NULL},
    {"R*EADONLY" , ASF_MINUS           , NULL,    optattrib ,~A_RDONLY,"Modify the file's READONLY attribute."},
    {"R*EADONLY" , ASF_PLUS            , NULL,    optattrib , A_RDONLY,NULL},
    {"P*ROTECT"  , ASF_VALREQ|ASF_LSVAL, NULL,    optprot   , 0, "Set the file protection levels."},
    {"O*WNER"    , ASF_VALREQ|ASF_LSVAL, NULL,    optowner  , 0, "Set the file owner value."},
    {"SO*RT"     , ASF_BOOL|ASF_STORE  , NULL,    &sorted   , TRUE,"Sort files before taking action."},
    {"C*ONFIRM"  , ASF_BOOL|ASF_STORE  , NULL,    &confirm  , TRUE,"Ask for confirmation before taking actioin."},
    {"Q*UIET"    , ASF_BOOL|ASF_STORE  , NULL,    &quiet    , TRUE,"Suppress all but error messages."},
    {"M*UTE"     , ASF_BOOL|ASF_STORE  , NULL,    &mute     , TRUE,"Suppress all messages."},
    {"H*ELP"     , 0                   , NULL, AF(opthelp) , 0, "This message."},
    {"?"         , 0                   , NULL, AF(opthelp) , 0, "This message."},
    {NULL        , 0                   , NULL, AF(NULL)     , 0,NULL}
};

char user[36];			// Specified user name
char ownername[36];		// Owner information from file

struct
{   byte4_parm  filoptn;
    lngstr_parm filspec;
    byte2_parm  srcattr;
    byte2_parm  filattr;
    byte4_parm  dirhndl;
    byte4_parm  prot;
    lngstr_parm owner;
    char        end;
} fileparm =
{   {PAR_SET|REP_HEXV, 4 , IOPAR_FILOPTN , FO_XOSNAME|FO_NODENUM|FO_NODENAME|
            FO_RXOSNAME|FO_PATHNAME|FO_FILENAME|FO_VERSION|FO_ATTR},
    {PAR_GET|REP_STR , 0 , IOPAR_FILSPEC , NULL, 0, FILESPCSIZE+10, 0},
    {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR , A_NORMAL|A_DIRECT|A_HIDDEN|A_SYSTEM},
    {PAR_GET|REP_HEXV, 2 , IOPAR_FILATTR , 0},
    {PAR_SET|REP_HEXV, 4 , IOPAR_DIRHNDL , 0},
    {PAR_GET|REP_HEXV, 4 , IOPAR_PROT    , 0},
    {PAR_GET|REP_STR , 0 , IOPAR_OWNER   , ownername, 0, 36, 36}
};

DIRSCANDATA dsd =
{   (DIRSCANPL *)&fileparm,	// parmlist - Address of parameter list
    FALSE,			// repeat   - TRUE if should do repeated search
    TRUE,			// dfltwild - TRUE if want wildcard as default
    QFNC_DEVPARM,		// function - Value for low byte of QAB_FUNC
    DSSORT_ASCEN,		// sort     - Directory sort order
    NULL,			// hvellip  - Function called if ellipsis given
    (int (*)(DIRSCANDATA *))procfile,
				// func     - Function called for each match
    errormsg			// error    - Function called on error
};

struct protdata
{   uchar spc;
    uchar new;
    uchar val;
    uchar add;
    uchar rmv;
};

struct protdata network = {FALSE, FALSE, 0, 0, 0xFF};
struct protdata world   = {FALSE, FALSE, 0, 0, 0xFF};
struct protdata group   = {FALSE, FALSE, 0, 0, 0xFF};
struct protdata owner   = {FALSE, FALSE, 0, 0, 0xFF};

char  modify;			// Modify attributes if TRUE
char  protected;		// TRUE if have protected file system
char  firsttime;
int   ownerlen;
int   pagewidth  = 80;		// Width of display in columns
int   pageremain = 80;		// Remaining space on current line

// Misc. variables

Prog_Info pib;
char  copymsg[] = "";
char  example[] = "{+|-options} {/options} filelist";
char  description[] = "ATTRIB is a DOS-style command with some XOS " \
    "extensions.  It is used to look at or change any of the file " \
    "attributes which are part of the directory entry for each file.  " \
    "Files may be specified using wildcard or ellipsis specifications.";
char  prgname[] = "ATTRIB";
char  envname[] = "^XOS^ATTRIB^OPT";

int   setattribute =  0;	// File attributes to set
int   clrattribute = -1;	// File attributes to clear

long  grandtotal = 0;		// Total number of files processed
int   nameseen;			// We saw a file name on the cmd line

struct filespec
{
    struct filespec *next;	// Must be first item in structure
    struct filespec *sort;	// For heapsort
    long   error;		// Error associated with file
    long   protect;		// File protection value
    char   setattrib;		// Attributes to set
    char   clrattrib;		// Attributes to clear
    char   attributes;		// Current file attributes
    char   owner[32];
    char   filespec[1];		// File specification
};

struct
{   text4_char  protect;
    char        end;
} protectchar =
{  {PAR_GET|REP_TEXT, 4 , "PROTECT", 0}
};

struct filespec *firstfile;	// Pointer to first file to process
struct filespec *lastfile;	// Pointer to last file to process

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
    char  *envargs[2];
    char   strbuf[256];		// String buffer
    char   filspec[FILESPCSIZE+10];

	reg_pib(&pib);

	init_Vars();


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

    if (argc < 2)
	{			// Have any arguments?
		if ( !mute )
			fprintf(stderr,"? ERROR %s:  NO PARAMETERS SPECIFIED\n",prgname);
		exit(EXIT_INVSWT);
	}

    ++argv;
    progarg(argv, PAF_PSWITCH, options, NULL, nonopt,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

	if ( mute )
		quiet=TRUE;

    if (!nameseen)			// Must have something specified
	{
		if ( !mute )
			fprintf(stderr,"? ERROR %s:  NO FILESPEC PROVIDED\n",prgname);
		exit(EXIT_INVSWT);
	}

    return (EXIT_NORM);
}

//************************************************
// Function: nonopt - Process a file specification
// Returned: TRUE if OK, false if error
//************************************************

int nonopt(
    char *arg)

{
    struct filespec *thisfile;
    union
    {   byte1_parm  *n1;
        byte4_parm  *n4;
        lngstr_parm *s;
        char        *e;
    } setpnt;
    union
    {   long l;
        char s[4];
    } letters;
    char   setparms[40];
    ulong  protection;
    ulong  newprot;
    long   newattrib;
    long   rtn;
    char   newowner;
    char   attributes;
    char   value;

    firstfile = lastfile = NULL;
    nameseen = TRUE;
    firsttime = TRUE;
    ownerlen = 1;
    dirscan(arg, &dsd);			// Scan the directory
    if (firstfile == NULL)
    {
        if ( !mute )
			fprintf(stdout, "? ATTRIB: File not found, %s\n", arg);
        exit(EXIT_FNF);
    }

    if (sorted)
        firstfile = heapsort(firstfile,
                (int (*)(void *a, void *b, void *d))comp, NULL);
    thisfile = firstfile;
    do
    {
        attributes = thisfile->attributes;
        protection = thisfile->protect;

        newattrib = (attributes | setattribute) & clrattribute;

        newprot = protection;
        if (network.spc)
        {
            value = (network.new)? network.val: (protection/FP_NETWORK);
            value |= network.add;
            value &= network.rmv;
            newprot &= ~(0xFF * (ulong)FP_NETWORK);
            newprot |= value * (ulong)FP_NETWORK;
        }
        if (world.spc)
        {
            value = (world.new)? world.val: (protection/FP_WORLD);
            value |= world.add;
            value &= world.rmv;
            newprot &= ~(0xFF * (ulong)FP_WORLD);
            newprot |= value * (ulong)FP_WORLD;
        }
        if (group.spc)
        {
            value = (group.new)? group.val: (protection/FP_GROUP);
            value |= group.add;
            value &= group.rmv;
            newprot &= ~(0xFF * (ulong)FP_GROUP);
            newprot |= value * (ulong)FP_GROUP;
        }
        if (owner.spc)
        {
            value = (owner.new)? owner.val: (protection/FP_OWNER);
            value |= owner.add;
            value &= owner.rmv;
            newprot &= ~(0xFF * (ulong)FP_OWNER);
            newprot |= value * (ulong)FP_OWNER;
        }

        newowner = ((user[0] != 0) && (strcmp(user, thisfile->owner) != 0));

        if ((attributes != newattrib) || (protection != newprot) || newowner)
        {
            setpnt.e = setparms;
            setpnt.n1->desp = PAR_SET|REP_HEXV;
            setpnt.n1->size = 1;
            setpnt.n1->index = IOPAR_SRCATTR;
            setpnt.n1->value = (char)(A_NORMAL|A_DIRECT|A_HIDDEN|A_SYSTEM);
            setpnt.n1++;
            if (attributes != newattrib)
            {
                setpnt.n1->desp = PAR_SET|REP_HEXV;
                setpnt.n1->size = 1;
                setpnt.n1->index = IOPAR_FILATTR;
                setpnt.n1->value = (char)newattrib;
                setpnt.n1++;
            }
            if (newprot != protection)
            {
                setpnt.n4->desp = PAR_SET|PAR_GET|REP_HEXV;
                setpnt.n4->size = 4;
                setpnt.n4->index = IOPAR_PROT;
                setpnt.n4->value = newprot;
                setpnt.n4++;
            }
            if (newowner)
            {
                setpnt.s->desp = PAR_SET|PAR_GET|REP_STR;
                setpnt.s->size = 0;
                setpnt.s->index = IOPAR_OWNER;
                setpnt.s->buffer = thisfile->owner;
                setpnt.s->bfrlen = 36;
                setpnt.s->strlen = 36;
                setpnt.s++;
            }
            *setpnt.e = 0;
            if ((rtn = svcIoDevParm(0, thisfile->filespec, setparms)) < 0)
                femsg2(prgname, "Error changing file attributes", rtn,
                        thisfile->filespec);
            attributes = newattrib;
//            protection = newprot;
        }

        if (confirm || (!quiet && !mute) )
        {
            if (protected)
            {
                letters.l = (attributes & A_DIRECT)? 'C?SA': 'WERX';
                printf("  %c%c%c %c%c%c%c%c %c%c%c%c%c %c%c%c%c%c %c%c%c%c%c "
                    "%-*.*s  %s\n", (attributes & A_ARCH)? 'A': '-',
                    (attributes & A_HIDDEN)? 'H': '-',
                    (attributes & A_SYSTEM)? 'S': '-',
                    (protection & (FP_NETWORK*FP_EXEC))? letters.s[0]: '-',
                    (protection & (FP_NETWORK*FP_READ))? letters.s[1]: '-',
                    (protection & (FP_NETWORK*FP_EXTEND))? letters.s[2]: '-',
                    (protection & (FP_NETWORK*FP_WRITE))? letters.s[3]: '-',
                    (protection & (FP_NETWORK*FP_MODIFY))? 'M': '-',
                    (protection & (FP_WORLD*FP_EXEC))? letters.s[0]: '-',
                    (protection & (FP_WORLD*FP_READ))? letters.s[1]: '-',
                    (protection & (FP_WORLD*FP_EXTEND))? letters.s[2]: '-',
                    (protection & (FP_WORLD*FP_WRITE))? letters.s[3]: '-',
                    (protection & (FP_WORLD*FP_MODIFY))? 'M': '-',
                    (protection & (FP_GROUP*FP_EXEC))? letters.s[0]: '-',
                    (protection & (FP_GROUP*FP_READ))? letters.s[1]: '-',
                    (protection & (FP_GROUP*FP_EXTEND))? letters.s[2]: '-',
                    (protection & (FP_GROUP*FP_WRITE))? letters.s[3]: '-',
                    (protection & (FP_GROUP*FP_MODIFY))? 'M': '-',
                    (protection & (FP_OWNER*FP_EXEC))? letters.s[0]: '-',
                    (protection & (FP_OWNER*FP_READ))? letters.s[1]: '-',
                    (protection & (FP_OWNER*FP_EXTEND))? letters.s[2]: '-',
                    (protection & (FP_OWNER*FP_WRITE))? letters.s[3]: '-',
                    (protection & (FP_OWNER*FP_MODIFY))? 'M': '-',
                    ownerlen, ownerlen, thisfile->owner, thisfile->filespec);
            }
            else
                printf("  %c%c%c%c  %s\n", (attributes & A_ARCH)? 'A': '-',
                        (attributes & A_HIDDEN)? 'H': '-',
                        (attributes & A_RDONLY)? 'R': '-',
                        (attributes & A_SYSTEM)? 'S': '-', thisfile->filespec);
        }

    } while ((thisfile = thisfile->next) != NULL);

    return (TRUE);
}

//***************************************************
// Function: init_Vars - Set up the program information block
// Returned: none
//***************************************************
void init_Vars(void)
{
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
// Function: optattrib - Process attribute options
// Returned: TRUE always
//************************************************

int optattrib(
    arg_data *arg)

{
    if (arg->data > 0)
    {
        setattribute |= arg->data;
        clrattribute |= arg->data;
    }
    else
    {
        setattribute &= arg->data;
        clrattribute &= arg->data;
    }
    modify = TRUE;
    return (TRUE);
}

//*********************************************************************
// Function: optprot - Process options which set file protection values
// Returned: TRUE always
//*********************************************************************

// Protecton values are specified as X:ABC, where X is N (network), W (world),
//   G (group), or O (owner) to specify the protection field, and A, B, or C
//   specifies the access allowed as follows:
//     File access Letter  Directory access Letter
//     None          N     None               N
//     Execute       X     Access file        A
//     Read          R     Search             S
//     Extend        E
//     Write         W     Create file        C
//     Modify attr   M     Modify attr        M
//   A letter can be preceeded with - to remove the access right or + to add
//   the access right to the current rights.  If no - or + preceeds the
//   specifications, the current rights are overwritten.  Multiple fields can
//   be specified in one option value with or without seperating spaces.  There
//   can be no spaces within the specification of a field.

int optprot(
    arg_data *arg)

{
    char  *pnt;
    struct protdata *protpnt;
    char   chr;
    char   func;
    char   value;

    pnt = arg->val.s;
    while ((chr = toupper(*pnt++)) != 0)
    {
        switch (chr)
        {
         case 'N':
            protpnt = &network;
            network.spc = TRUE;
            network.new = FALSE;
            network.val = 0;
            network.add = 0;
            network.rmv = 0xFF;
            break;

         case 'W':
            protpnt = &world;
            world.spc = TRUE;
            world.new = FALSE;
            world.val = 0;
            world.add = 0;
            world.rmv = 0xFF;
            break;

         case 'G':
            protpnt = &group;
            group.spc = TRUE;
            group.new = FALSE;
            group.val = 0;
            group.add = 0;
            group.rmv = 0xFF;
            break;

         case 'O':
            protpnt = &owner;
            owner.spc = TRUE;
            owner.new = FALSE;
            owner.val = 0;
            owner.add = 0;
            owner.rmv = 0xFF;
            break;

         default:
            if (!mute)
				fprintf(stderr, "? ATTRIB: Illegal protection field specification"
                    " %c\n          Valid values are N (network), W (world),"
                    " G (group), or O (owner)\n", chr);
            exit(EXIT_INVSWT);
        }
        if (*pnt++ != ':')		// Must have colon next
        {
            if (!mute)
				fprintf(stderr, "? ATTRIB: Protection field specification %c not"
                    " followed by colon\n", chr);
            exit(EXIT_INVSWT);
        }
        if (*pnt == 0)
        {
            if (!mute)
				fprintf(stderr, "? ATTRIB: No access specified for protection"
                    " field %c\n", chr);
            exit(EXIT_INVSWT);
        }

        func = REPLACE;
        while ((chr = toupper(*pnt)) != 0 && pnt[1] != ':')
        {
            pnt++;
            switch (chr)
            {
             case '+':
                func = ADD;
                continue;

             case '-':
                func = REMOVE;
                continue;

             case 'N':
                value = 0;
                break;

             case 'X':
             case 'A':
                value = FP_EXEC;
                break;

             case 'R':
             case 'S':
                value = FP_READ;
                break;

             case 'E':
                value = FP_EXTEND;
                break;

             case 'W':
             case 'C':
                value = FP_WRITE;
                break;

             case 'M':
                value = FP_MODIFY;
                break;
            }
            switch (func)
            {
             case REPLACE:
                protpnt->new = TRUE;
                protpnt->val = value;
                func = ADD;
                break;

             case ADD:
                protpnt->add |= value;
                protpnt->rmv |= value;
                break;

             case REMOVE:
                protpnt->add &= ~value;
                protpnt->rmv &= ~value;
            }
        }
    }
    return (TRUE);
}

//******************************************
// Function: optowner - Process OWNER option
// Returned: TRUE always
//******************************************

int optowner(
    arg_data *arg)

{
    strnmov(user, arg->val.s, 32);
    return (TRUE);
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
    char   buffer[1024];

    pnt = strmov(buffer, dsd.devname);
    left = 1022-dsd.devnamelen;
    pnt = strnmov(pnt, dsd.rmtdata.nodename, left);
    if ((left -= dsd.rmtdata.nodenamelen) > 0)
    {
        pnt = strnmov(pnt, dsd.rmtdata.rmtdevname, left);
        if ((left -= dsd.rmtdata.rmtdevnamelen) > 0)
        {
            pnt = strnmov(pnt, dsd.pathname, left);
            if ((left -= dsd.pathnamelen) > 0)
                pnt = strnmov(pnt, dsd.filename, left);
        }
    }
    *pnt = '\0';			// Make sure have null at end

    if (dsd.error < 0 &&
            (dsd.error != ER_FILAD && dsd.error != ER_FBFER &&
            dsd.error != ER_FBFER && dsd.error != ER_FBWER &&
            dsd.error != ER_BUSY))
    {
        errormsg(buffer, dsd.error);
        return (FALSE);
    }
    if (firsttime)
    {
        firsttime = FALSE;
        protected = FALSE;
        if (svcIoDevChar(fileparm.dirhndl.value, &protectchar) >= 0)
            protected = (protectchar.protect.value[0] == 'Y');
    }

    if (dsd.filenamelen == 0)		// Finished here if no name
        return (TRUE);

    thisfile = (struct filespec *)getspace(sizeof(struct filespec) +
            strlen(buffer) + 1);
    strmov(thisfile->filespec, buffer);
    thisfile->setattrib = setattribute;
    thisfile->clrattrib = clrattribute;
    thisfile->error = dsd.error;
    thisfile->attributes = dsd.attr;
    thisfile->protect = fileparm.prot.value;
    strnmov(thisfile->owner, ownername, 32);
    if (fileparm.owner.strlen > ownerlen)
        ownerlen = fileparm.owner.strlen;
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

//*******************************************
// Function: errormsg - Display error message
// Returned: Nothing
//*******************************************

void errormsg(arg, code)
char *arg;
long  code;
{
    char buffer[80];		// Buffer to receive error message

    svcSysErrMsg(code, 3, buffer); // Get error message
    if (!mute)
		fprintf(stderr, "\n? %s: %s", prgname, buffer);
    if (arg != NULL && *arg != '\0')	// Have returned name?
		if (!mute)
		{
			fprintf(stderr, ", file %s", arg); // Yes - output it too
    		fputc('\n', stderr);
		}
}
