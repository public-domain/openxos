//-----------------------------------------------------------------------
// ADDUNIT.C
// Program to add device unit to class
//
// Written by: John R. Goltz
//
// Edit History:
// 08/20/92(brn) - Add comment header
// 05/12/94(brn) - Fix spelling in command abbreviations
// 02/24/95(sao) - Add progasst update, added mute option
// 05/13/95(sao) - Changed 'optional' indicators from [] to {}
// 05/16/95(sao) - changed exit codes to reference XOSRTN defines
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
//-----------------------------------------------------------------------

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

// Command format:
//   ADDUNIT {-Q{UIET}} classname {name1=char1} {name2=char2} ...

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xos.h>
#include <xossvc.h>
#include <xosrtn.h>
#include <progarg.h>
#include <proghelp.h>
#include <xosstr.h>

#define VERSION 3
#define EDITNO  3

struct addchar
{   unsigned char type;
    unsigned char length;
    char  name[8];
    union
    {   long num;
        char text[32];
    } val;
};

struct addcharbfr
{   struct addcharbfr *next;
    union
    {   struct addchar p;
        char   s[1];
    } b;
}     *addcharbfr;

struct addchar *addbfr;
struct addchar *addchar;
char   classname[10];		// Buffer for class name

type_qab addqab =
{
    QFNC_WAIT|QFNC_CLASSFUNC,	// qab_func     - Function
    0,				// qab_status   - Returned status
    0,				// qab_error    - Error code
    0,				// qab_amount   - Amount
    0,				// qab_handle   - Device handle
    0,				// qab_vector   - Vector for interrupt
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    CF_ADDUNIT,			// qab_option   - Options or command
    0,				// qab_count    - Count
    classname, 0,		// qab_buffer1  - Pointer to file spec
    NULL, 0,			// qab_buffer2  - Unused
    NULL, 0			// qab_parm     - Pointer to parameter area
};

int    unitnum = -1;
struct addcharbfr *firstchar;
struct addcharbfr *lastchar = (struct addcharbfr *)(&firstchar);
long   quiet = FALSE;		// TRUE if no output except error messages
long   mute = FALSE;		// TRUE if no output at all

// The program information block

Prog_Info pib;

char   copymsg[] = "";
char   example[] = "{/options} class{:}UNIT=# {item1=value1 i2=v2{...}}}";
char   description[] = "This command is used to add physical hardware units "
    "to a device class.  This command creates a device unit associated with "
    "the specified device class and associates it with the specified hardware.";
char   prgname[] = "ADDUNIT";
char   envname[] = "^XOS^ADDUNIT^OPT";

struct addchar *nextchar(struct addchar *);
void   firstarg(char *);
int    charhave(arg_data *);
int    unithave(arg_data *);
void   init_Vars(void);

#define AF(func) (int (*)(arg_data *))func

arg_spec keywords[] =
{   {"U*NIT", ASF_NVAL          , NULL, unithave, 0, "Specifies unit to add"},
    {"*"    , ASF_LSVAL|ASF_NVAL, NULL, charhave, 0, "Anything else is a characteristic"},
    {NULL   , 0                 , NULL, AF(NULL), 0, NULL}
};

arg_spec options[] =
{
    {"H*ELP"  , 0, NULL, AF(opthelp), 0, "This message."},
    {"?"      , 0, NULL, AF(opthelp), 0, "This message."},
    {"Q*UIET" , ASF_BOOL|ASF_STORE, NULL, &quiet, TRUE, "No output, except error messages."},
    {"M*UTE"  , ASF_BOOL|ASF_STORE, NULL, &mute, TRUE, "No output, even error messages."},
    {NULL     , 0, NULL, AF(NULL)    , 0}
};

main(argc, argv)
int   argc;
char *argv[];

{
    char  *pnt1;
    char  *pnt2;
    long   rtn;
    int    cnt;
    int    size;
    char   strbuf[512];
    char  *foo[2];

	reg_pib(&pib);

	init_Vars();

    if(svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
    {
	foo[0] = strbuf;
	foo[1] = '\0';
	progarg(foo, 0, options, NULL, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    if (argc !=0)
        ++argv;
    progarg(argv, 0, options, keywords, (int (*)(char *))NULL,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

    if ( mute )
        quiet=TRUE;

    if (classname[0] == '\0')		// Was a class specified?
    {					// No - fail
        if ( !mute )
            fputs("? ADDUNIT: No class specified\n", stderr);
        exit(EXIT_INVSWT);
    }

    if (unitnum == -1)			// Did he give us a unit number?
    {
        if (!mute)
            fprintf(stderr, "? ADDUNIT: No unit number specified for "
                    "class %s\n", classname);
        exit(EXIT_INVSWT);
    }

    //  When we get here, we have a list of characteristics buffers pointed
    //    to by firstchar - first we copy these into a single buffer

    addcharbfr = (struct addcharbfr *)(&firstchar);
    size = 0;				// First see how much space we need
    while ((addcharbfr = addcharbfr->next) != NULL)
        size += addcharbfr->b.p.length + 10;

    addbfr = (struct addchar *)getspace(size+8);
    addcharbfr = (struct addcharbfr *)(&firstchar);
    pnt1 = (char *)addbfr;
    while ((addcharbfr = addcharbfr->next) != NULL)
    {
        cnt = addcharbfr->b.p.length + 10;
        pnt2 = (char *)&(addcharbfr->b.p);
        do
        {
            *pnt1++ = *pnt2++;
        } while (--cnt > 0);
    }
    *pnt1 = 0;

    addqab.qab_buffer2 = (char *)addbfr;
    if ((rtn = svcIoQueue(&addqab)) < 0 || (rtn = addqab.qab_error) < 0)
    {
        char errmsg[100];

        svcSysErrMsg(rtn, 3, errmsg);  // Get error message string
        addchar = addbfr;
        do				// Scan to find bad characteristic
        {
            if (addchar->type & PAR_ERROR)
            {
                if (!mute)
                    fprintf(stderr,"? ADDUNIT: Error adding unit %d to "
                            "class %s\n           (error associated with "
                            "characteristic %0.8s)\n           %s\n", unitnum,
                            classname, addchar->name, errmsg);
                exit(EXIT_INVSWT);
            }
        }
            while ((addchar = nextchar(addchar))->type != 0)
                ;
            if ( !mute )
                fprintf(stderr, "? ADDUNIT: Error adding unit %d to class %s\n"
                        "           %s\n", unitnum, classname, errmsg);
        exit(EXIT_INVSWT);
    }
    if ( !quiet && !mute ) // Tell him about it if we should
        printf("ADDUNIT: Unit %d added to device class %s\n", unitnum,
                classname);
    return(EXIT_NORM);
}

//***********************************************************
// Function: init_Vars - Set up the program information block
// Returned: none
//***********************************************************

void init_Vars(void)

{
    // Set defaults of control variables

    quiet=FALSE;
    mute=FALSE;

    // set Program Information Block variables

    pib.opttbl=options; 		// Load the option table
    pib.kwdtbl=keywords;
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
}

//***************************************************
// Function: addchar - Advance to next characteristic
// Returned: Pointer to data for next characteristic
//***************************************************

struct addchar *nextchar(
    struct addchar *addchar)

{
    return ((struct addchar *)((char *)addchar + (addchar->length) + 10));
}

//********************************************
// Function: firstarg - Process first argument
// Returned: Nothing
//********************************************

void firstarg(
    char *arg)

{
    char *pntn;
    char *class;
    int   cnt;
    char  chr;

    strupper(arg);
    class = arg;
    pntn = classname;
    cnt = 8;
    while ((chr = *arg++) != '\0' && chr != '=' && chr != ':')
    {
        if (--cnt < 0)
        {
            if (!mute)
                fprintf(stderr, "? ADDUNIT: Class name %s is too long\n",
                        class);
            exit(EXIT_INVSWT);
        }
        *pntn++ = chr;
    }
    if (chr != '\0' && (chr != ':' || *arg != '\0'))
    {
		if ( !mute )
        	fprintf(stderr, "? ADDUNIT: Illegal class name %s specified\n", class);
        exit(EXIT_INVSWT);
    }
    *pntn = ':';
}

//*****************************************
// Function: unithave - Process unit number
// Returned: TRUE, does not return if error
//*****************************************

int unithave(
    arg_data *arg)

{
    if (unitnum != -1)			// Already have a unit number?
    {
        if (!mute)
            fprintf(stderr, "? ADDUNIT: More than one unit number specified for"
                    " class %s\n", classname);
        exit(EXIT_INVSWT);
    }
    if (arg->val.n > 999)
    {
        if ( !mute )
            fprintf(stderr, "? ADDUNIT: Invalid unit number specified for"
                    " class %s\n", classname, stderr);
        exit(EXIT_INVSWT);
    }
    unitnum = (int)(arg->val.n);	// Remember the unit number
    charhave(arg);          // Finish processing it
    return (TRUE);
}

//*********************************************************
// Function: charhave - Process general characteristic name
// Returned: TRUE, does not return if error
//*********************************************************

int charhave(
    arg_data *arg)

{
    int    vsize;
    strupper(arg->name);            // Convert name to upper case
    if ( *classname == '\0')   // Classname not yet specified

    {
        firstarg(arg->name);
        return (TRUE);
    }
    if (arg->flags & (ADF_NONE|ADF_NULL)) // Have a value?
    {
	if ( !mute )
            fprintf(stderr, "? ADDUNIT: No value given for characteristic %s\n",
                arg->name);
        exit(EXIT_INVSWT);
    }
    if (strlen(arg->name) > 8)
    {
        if (!mute)
            fprintf(stderr, "? ADDUNIT: Characteristic name %s is too long\n",
                    arg->name);
        exit(EXIT_INVSWT);
    }
    addcharbfr = (struct addcharbfr *)getspace(64);
    lastchar->next = addcharbfr;
    lastchar = addcharbfr;
    strncpy(addcharbfr->b.p.name, arg->name, 8); // Copy characteristic name

    if (arg->flags & ADF_LSVAL)
    {
        addcharbfr->b.p.type = PAR_SET|REP_TEXT;
        if (arg->flags & ADF_LSVAL)
            vsize = strlen(arg->val.s);
        else
        {
            vsize = 0;
            arg->val.s = "";
        }
        if (vsize > 32)     // Is our value too long?
        {
            if ( !mute )
                fprintf(stderr, "? ADDUNIT: String value for characteristic"
                            " %s is too long\n", arg->name);
            exit(EXIT_INVSWT);
        }
        addcharbfr->b.p.length = vsize;
        strcpy(addcharbfr->b.p.val.text, arg->val.s);
    }
    else
    {
        addcharbfr->b.p.type = PAR_SET|REP_DECV;
        addcharbfr->b.p.val.num = arg->val.n;
        addcharbfr->b.p.length = 4;
    }
    return (TRUE);
}
