//--------------------------------------------------------------------------*
// DEVCHAR.C
// Command to list or change device characteristics
//
// Written by: John R. Goltz
//
// Edit History:
// 08/20/92(brn) - Add comment header
// 05/12/94(brn) - Fix command abbreviations
// 03/08/95(sao) - Add progasst package
// 04/25/95(sao) - Changed 'brief' switch back to 'verbose'
// 05/13/95(sao) - Changed 'optional' id
// 05/16/95(sao) - Changed exit codes to reflect ALGRTN
// 05/17/95(sao) - Made 'brief' option do something (again).
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
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

// Command format:
//   DEVCHAR {-Q{UIET}} devname {name1=parm1} {name2=parm2} ...

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <XOSTIME.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSRTN.H>
#include <XOSERR.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>

#define VERSION 3
#define EDITNO  6

struct parm
{   uchar type;
    uchar length;
    char  name[8];
    char  far *infopnt;
    short xxx1;
    short infosize;
    short xxx2;
    union
    {   struct
        {   long low;
            long high;
        }      num;
        char   far *pnt;
        time_s dtv;
        char   ss[8];
    } val;
    short size;
    short amount;
};

struct parmbfr
{   struct parmbfr *next;
    union
    {   struct parm p;
        char   s[1];
    } b;
};

#if defined(SYSCHAR)
  char devname[] = "SYSTEM:";
#else
  char devname[10];		// Buffer for given device name
#endif
char   realname[16];		// Buffer for real device name

struct
{   byte4_parm  options;
    lngstr_parm name;
    char        end;
} opnparm =
{   {(PAR_SET|REP_HEXV), 4, IOPAR_FILOPTN, FO_PHYNAME|FO_XOSNAME},
    {(PAR_GET|REP_STR ), 0, IOPAR_FILSPEC, realname, 0, 16, 0}
};

type_qab charqab =
{
#if defined(CLSCHAR) || defined(SYSCHAR)
    QFNC_WAIT|QFNC_CLASSFUNC,
#else
    QFNC_WAIT|QFNC_OPEN,
#endif
    0,				// qab_status   - Returned status
    0,				// qab_error    - Error code
    0,				// qab_amount   - Amount
    0,				// qab_handle   - Device handle
    0,				// qab_vector   - Vector for interrupt
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    O_PHYS|O_NOMOUNT,		// qab_option   - Options or command
    0,				// qab_count    - Count
    devname, 0,			// qab_buffer1  - Pointer to file spec
    NULL, 0,			// qab_buffer2  - Unused
    NULL, 0			// qab_parm     - Pointer to parameter area
};

extern int (*_sprintfpnt)(int, char **);

ulong  parmblk[32];
ulong *ppnt = parmblk+2;
char  *xpfpnt;
char  *buffer;
int    bufmax;
int    outsize;
int    charcnt;
int    chartotal;
int    labelsize;
char  *charpnt;
char  *parmlist;
struct parm    *parmpnt;
struct parmbfr *parmbfr;
struct parmbfr *firstparm;
struct parmbfr *lastparm = (struct parmbfr *)(&firstparm);
long   brief=FALSE;
long   quiet=FALSE;           // TRUE if no output wanted
long   mute=FALSE;           // TRUE if brief output wanted
char   errbfr[PROGASSTEBSZ]; // procarg error buffer


Prog_Info pib;
char  copymsg[] = "";

#if defined(CLSCHAR)
  char prgname[] = "CLSCHAR";
  char envname[] = "^XOS^CLSCHAR^OPT";
char  example[] = "{/option} class{:} char1=value1 ...";
  char description[] = "This command is used to modify and display " \
    "class characteristics for any device class in the system.  Refer " \
    "to chapter 5 of the XOS Command Reference Manual for a " \
    "description of classes and class characteristics.  The first " \
    "argument specifies the device class.  It may optionally be terminated " \
    "with a colon.  This argument is required and may not have a value.  " \
    "Each additional argument specifies a characteristic.  If no " \
    "characteristics are specified, the current values of all " \
    "characteristics for the device class are displayed.  If one or " \
    "more characteristics are specified with values, the characteristics " \
    "are set to the values specified.  The values of all characteristics " \
    "specified (with or without a new value being specified) are " \
    "displayed (unless /QUIET was specified).";
#elif defined(SYSCHAR)
  char prgname[] = "SYSCHAR";
  char envname[] = "^XOS^SYSCHAR^OPT";
char  example[] = "{/option} char1=value1 ...";
  char description[] = "This command is used to modify and display " \
    "class characteristics for any device class in the system.  Refer " \
    "to chapter 5 of the XOS Command Reference Manual for a " \
    "description of device classes and their characteristics.  Each " \
    "argument specifies a SYSTEM class characteristic.  If no " \
    "characteristics are specified, the current values of all " \
    "characteristics for the class are displayed.  If one or " \
    "more characteristics are specified with values, the characteristics " \
    "are set to the values specified.  The values of all characteristics " \
    "specified (with or without a new value being specified) are " \
    "displayed (unless /QUIET was specified).";
#else
  char prgname[] = "DEVCHAR";
  char envname[] = "^XOS^DEVCHAR^OPT";
  char example[] = "{/option} device{:} char1=value ...";
  char description[] = "This command is used to modify and display " \
    "device characteristics for any device unit in the system.  Refer " \
    "to chapter 6 of the XOS Command Reference Manual for a " \
    "description of devices and device characteristics.  The first " \
    "argument specifies the device.  It may optionally be terminated " \
    "with a colon.  This argument is required and may not have a value.  " \
    "Each additional argument specifies a characteristic.  If no " \
    "characteristics are specified, the current values of all " \
    "characteristics for the device unit are displayed.  If one or " \
    "more characteristics are specified with values, the characteristics " \
    "are set to the values specified.  The values of all characteristics " \
    "specified (with or without a new value being specified) are " \
    "displayed (unless /QUIET was specified).";
#endif


struct parm *nextparm(struct parm *);
#if !defined(SYSCHAR)
  void firstarg(char *);
#endif
// int  briefhave(arg_data *);
int  charhave(arg_data *);
void chkbuffer(int type, int length);
void dcharerr(long, struct parm *);
void fatal(long);
// void helphave(void);
// int  quiethave(arg_data *);
void xprintf (const char *, ...);

#define AF(func) (int (*)(arg_data *))func

arg_spec keywords[] =
{   {"*" , ASF_LSVAL, NULL, charhave, 0},
    {NULL, 0        , NULL, NULL    , 0}
};

int havebrief( arg_data *arg);

arg_spec options[] =
{
    {"H*ELP"    , 0                 , NULL, AF(opthelp) , 0   ,
            "This message."},
    {"?"        , 0                 , NULL, AF(opthelp) , 0   ,
            "This message."},
    {"V*ERBOSE" , ASF_NEGATE        , NULL,    havebrief, TRUE,
            "Detailed output."},
    {"Q*UIET"   , ASF_BOOL|ASF_STORE, NULL,   &quiet    , TRUE,
            "No output, except error messages."},
    {"M*UTE"    , ASF_BOOL|ASF_STORE, NULL,   &mute     , TRUE,
            "No output, even error messages."},
    {NULL}
};

main(argc, argv)
int   argc;
char *argv[];

{
    char  *sep;
    char  *fmt;
    uchar *pnt;
    long  *vpnt;
    long   value;
    long   rtn;
    int    cnt;
    int    type;
    int    rep;
    int    size;
    int    fsize;
    int    curpos;
    int    spaces;
    int    needed;
    char   label[100];

    char   strbuf[512];
    char  *foo[2];

    reg_pib(&pib);

    // Set defaults of control variables

    quiet=FALSE;
    mute=FALSE;

    // Set Program Information Block variables

    pib.opttbl=options; 		// Load the option table
    pib.kwdtbl=NULL;
    pib.build=__DATE__;
    pib.majedt = VERSION; 		// Major edit number
    pib.minedt = EDITNO; 		// Minor edit number
    pib.copymsg=copymsg;
    pib.prgname=prgname;
    pib.desc=description;
    pib.example=example;
    pib.errno=0;
    getTrmParms();
    getHelpClr();

    if(svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
    {
	foo[0] = strbuf;
	foo[1] = '\0';
	progarg(foo, 0, options, NULL, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    if (argc !=0)
        ++argv;
    progarg(argv, PAF_EATQUOTE, options, keywords, (int (*)(char *))NULL,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

    if (devname[0] == '\0')		// Was a device specified?
    {					// No - fail
        if (!mute)
            fprintf(stderr, "? %s: No "
  #if defined(CLSCHAR) || defined (SYSCHAR)
                    "class"
  #else
                    "device"
  #endif
                    " specified\n", prgname);
        exit(EXIT_INVSWT);
    }

    // Here with all arguments processed - now do what we need to do

#if !(defined(CLSCHAR) || defined(SYSCHAR))
    if (!quiet && !mute)
        printf("Physical device name is _%s\n", realname+1);
#endif
    if (charcnt == 0)			// Any characteristics specified?
    {					// No - report all characteristics
					// First see how much space we need
        charqab.qab_option = DCF_SIZEIP;
        if ((rtn = svcIoQueue(&charqab)) < 0 || (rtn = charqab.qab_error) < 0)
        {
            if (rtn != ER_IFDEV)
                fatal(rtn);
            else
                charqab.qab_amount = 0;
        }
        if (charqab.qab_amount == 0)
        {
            fputs("No characteristics defined for "
  #if defined(CLSCHAR) || defined (SYSCHAR)
                    "class\n",
  #else
                    "device\n",
  #endif
                    stdout);
            exit(EXIT_NORM);
        }
        firstparm = (struct parmbfr *)getspace(charqab.qab_amount +
                sizeof(struct parmbfr *)+4); // Allocate space
        firstparm->next = NULL;		// Get names and types for all
	charqab.qab_option = DCF_ALLIP;	//   characteristics for device
        charqab.qab_count = charqab.qab_amount;
        charqab.qab_buffer2 = (char *)&(firstparm->b);
        if ((rtn = svcIoQueue(&charqab)) < 0 || (rtn = charqab.qab_error) < 0)
            fatal(rtn);
        parmpnt = (struct parm *)&(firstparm->b);

        while ((type = parmpnt->type) != 0)
        {
            parmpnt->type |= (PAR_GET|PAR_INFO); // Indicate should get value
            type &= 0x0F;
            if (type == REP_STR || type == REP_DATAS) // String value?
            {				// Yes - allocate space for buffer
                needed = parmpnt->size;
                parmpnt->val.pnt = getspace(parmpnt->size);
            }
            else
                needed = parmpnt->length;
            chkbuffer(type, needed);
            if (!brief && parmpnt->infosize)
            {
                parmpnt->infopnt = getspace(parmpnt->infosize);
                if ((parmpnt->infosize + 2) > labelsize)
                    labelsize = parmpnt->infosize + 2;
            }
            parmpnt = nextparm(parmpnt);
        }
    }

    //  When we get here, we have a list of characteristics buffers pointed
    //    to by firstparm - first we allocate a single buffer large enough
    //    for all of the characteristics items and copy them to that single
    //    buffer

    curpos = spaces = 0;
    if (firstparm->next != NULL)	// Have more than one item?
    {					// Yes
        parmbfr = firstparm;
        charpnt = getspace(chartotal + 4);
        parmlist = charpnt;
        do
        {
            size = parmbfr->b.p.length + 22;
            memcpy(parmlist, parmbfr->b.s, size);
            parmlist += size;
        } while ((parmbfr = parmbfr->next) != NULL);
        *parmlist = '\0';
    }
    else
        charpnt = firstparm->b.s;

    charqab.qab_option = DCF_VALUES;
    charqab.qab_buffer2 = charpnt;

    // Get and/or set the characteristics values

    if ((rtn = svcIoQueue(&charqab)) < 0 || (rtn = charqab.qab_error) < 0)
        dcharerr(rtn, (struct parm *)charpnt); // If error
    if (!quiet)				// Tell him about it if we should
    {
        buffer = getspace(labelsize + bufmax + 20);
        while ((type = ((struct parm *)(charpnt))->type) != 0)
        {
            parmpnt = (struct parm *)charpnt;
            xpfpnt = buffer;
            outsize = 0;
            if (!brief && parmpnt->infopnt)
                sprintf(label, " (%s)", parmpnt->infopnt);
            else
                label[0] = '\0';
            xprintf("%-8.8s%-*s = ", parmpnt->name, labelsize, label);
            type &= 0x0F;
            if (type == REP_STR)
                xprintf("%.*s", parmpnt->amount, parmpnt->val.pnt);
            else if (type == REP_DATAS)
            {
                cnt = parmpnt->amount;
                xprintf("(size = %d)", cnt);
                pnt = (uchar *)parmpnt->val.pnt;
                do
                {   xprintf(" %02.2X", *pnt++);
                } while (--cnt > 0);
            }
            else if (type == REP_TEXT)
                xprintf("%.*s", parmpnt->length, parmpnt->val.ss);
            else if (type <= REP_BINV)
            {
                switch (parmpnt->length)
                {
                 case 0:
                    value = 0;
                    goto longval;

                 case 1:
                    value = (uchar)(parmpnt->val.num.low);
                    goto longval;

                 case 2:
                    value = (ushort)(parmpnt->val.num.low);
                    goto longval;

                 case 3:
                    value = parmpnt->val.num.low & 0xFFFFFFL;
                    goto longval;

                 case 4:
                    value = parmpnt->val.num.low;
                 longval:
                    rep = (parmpnt->type & 0x0F);
                    xprintf((rep == REP_DECV) ? "%ld" :
                           (rep == REP_HEXV) ? "0x%lX" : "0%lo", value);
                    break;

                 case 5:
                    value = (uchar)(parmpnt->val.num.high);
                    goto extval;

                 case 6:
                    value = (ushort)(parmpnt->val.num.high);
                    goto extval;

                 case 7:
                    value = parmpnt->val.num.high & 0xFFFFFFL;
                    goto extval;

                 case 8:
                    value = parmpnt->val.num.high;
                 extval:
                    rep = (parmpnt->type & 0x0F);
                    xprintf((rep == REP_DECV) ? "%ld:%ld" :
                            (rep == REP_HEXV) ? "0x%lX:0x%lX" :
			    "0%lo:0%lo", parmpnt->val.num.low, value);
                    break;

                 default:
                    cnt = parmpnt->length/4;
                    vpnt = (long *)&(parmpnt->val);
                    for (;;)
                    {   xprintf("%d", *vpnt++);
                        if (--cnt <= 0)
                            break;
                        xprintf(",");
                    }
                    break;
                }
            }
            else if (type == REP_VERN)
            {
                int major;
                int minor;
                int edit;

                if (parmpnt->length <= 1)
                {
                    major = ((uchar *)(&parmpnt->val))[0];
                    minor = edit = 0;
                }
                else if (parmpnt->length == 2)
                {
                    major = ((uchar *)(&parmpnt->val))[1];
                    minor = ((uchar *)(&parmpnt->val))[0];
                    edit = 0;
                }
                else if (parmpnt->length == 3)
                {
                    major = ((uchar *)(&parmpnt->val))[2];
                    minor = ((uchar *)(&parmpnt->val))[1];
                    edit = ((uchar *)(&parmpnt->val))[0];
                }
                else
                {
                    major = ((uchar *)(&parmpnt->val))[3];
                    minor = ((uchar *)(&parmpnt->val))[2];
                    edit = ((ushort *)(&parmpnt->val))[0];
                }
                xprintf("%d.%d", major, minor);
                if (edit != 0)
                    xprintf(".%d", edit);
            }
            else if (type == REP_TIME)
            {
                time_s timeval;
                char   buffer[30];

                timeval.date = 0;
                timeval.time = parmpnt->val.num.low;
                sdt2str(buffer, "%z%H:%m:%s", (time_sz *)&timeval);
                xprintf("%s", buffer);
            }
            else if (type == REP_DATE)
            {
                time_s timeval;
                char   buffer[30];

                timeval.date = parmpnt->val.num.low;
                timeval.time = 0;
                sdt2str(buffer, "%z%D-%3n-%y", (time_sz *)&timeval);
                xprintf("%s", buffer);
            }
            else if (type == REP_DT)
            {
                char buffer[30];

                if ((parmpnt->val.num.low | parmpnt->val.num.high) == 0)
                    xprintf("None");
                else
                {
                    sdt2str(buffer, "%z%H:%m:%s %D-%3n-%y",
			    (time_sz *)&(parmpnt->val.dtv));
                    xprintf("%s", buffer);
                }
            }
            else
            {
                if (type == REP_DECB)
                {
                    sep = ".";
                    fmt = "%d";
                }
                else if (type == REP_HEXB)
                {
                    sep = "-";
                    fmt = "%02.2X";
                }
                else
                {
                    sep = "/";
                    fmt = "%o";
                }
                cnt = parmpnt->length;
                pnt = (uchar *)(&parmpnt->val);
                while (--cnt >= 0)
                {
                    xprintf(fmt, *pnt++);
                    if (cnt > 0)
                        xprintf(sep);
                }
            }
            if (!brief && !quiet && !mute)
                printf("  %s\n", buffer);
            else
            {
                fsize = (outsize <= 25)? 26: (outsize <=51)? 52: 78;
                if ((curpos + fsize) > 80)
                {
                    if (!quiet && !mute)
                        fputs("\n ", stdout);
                    curpos = fsize;
                }
                else
                {
                    if (!quiet && !mute)
                        printf("%*s ", spaces, "");
                    curpos += fsize;
                }
                spaces = fsize - outsize - 1;
                if (!quiet && !mute)
                    fputs(buffer, stdout);
            }
            charpnt = (char *)nextparm(parmpnt);
        }
	if (brief)
	    puts("");
    }
    return(EXIT_NORM);
}

int havebrief(
    arg_data *arg)

{
    brief = (arg->flags & ASF_NEGATE) ? TRUE : FALSE;
    return (TRUE);
}

// Function to advance to next characteristic

struct parm *nextparm(
    struct parm *parm)

{
    return ((struct parm *)((char *)parm + (parm->length) + 22));
}

// Function to process first argument

#if !defined(SYSCHAR)

void firstarg(
    char *arg)

{
#if !defined(CLSCHAR)
    long  rtn;
#endif
    char *pntn;
    char *device;
    int   cnt;
    char  chr;

    strupr(arg);
    device = arg;
    pntn = devname;
    cnt = 8;
    while ((chr = *arg++) != '\0' && chr != '=' && chr != ':')
    {
        if (--cnt < 0)
        {
            if (!mute)
                fprintf(stderr, "? %s: "
#if defined(CLSCHAR) || defined(SYSCHAR)
                        "Class"
#else
                        "Device"
#endif
                        " name %s is too long\n", prgname);
            exit(EXIT_INVSWT);
        }
        *pntn++ = chr;
    }
    if (chr != '\0' && (chr != ':' || *arg != '\0'))
    {
        if (!mute)
            fprintf(stderr, "? %s: Illegal "
#if defined(CLSCHAR) || defined(SYSCHAR)
                    "class"
#else
                    "device"
#endif
                    " name %s specified\n", prgname, device);

        exit(EXIT_INVSWT);
    }
    *pntn = ':';

#if !defined(CLSCHAR)
    charqab.qab_parm = &opnparm;
    if ((rtn = svcIoQueue(&charqab)) < 0 || (rtn = charqab.qab_error) < 0)
        fatal(rtn);
    charqab.qab_func = QFNC_WAIT|QFNC_DEVCHAR;
    charqab.qab_parm = NULL;
#endif
}

#endif

// Function to process characteristic name

int charhave(
    arg_data *arg)

{
    char  *pnt;
    uchar *bpnt;
    long   rtn;
    ulong  value;
    ulong  bval;
    ulong  radix;
    int    type;
    int    size;
    int    vsize;
    int    needed;
    int    cnt;
    char   chr;
    char   list;

#ifndef SYSCHAR
    if (*devname == '\0')
    {
	firstarg(arg->name);		// Process device name
	return (TRUE);
    }
#endif

    strupr(arg->name);			// Convert name to upper case
    if (strlen(arg->name) > 8)
    {
        if (!mute)
            fprintf(stderr, "? %s: Characteristic name %s is too long for"
#if defined(CLSCHAR) || defined(SYSCHAR)
                    "class"
#else
                    "device"
#endif
                    " %s\n", prgname, arg->name, devname);
        exit(EXIT_INVSWT);
    }
    parmbfr = (struct parmbfr *)getspace(64);
    memset(parmbfr, 0, 64);
    lastparm->next = parmbfr;
    lastparm = parmbfr;
    parmbfr->next = NULL;
    strmov(parmbfr->b.p.name, arg->name);
    charqab.qab_option = DCF_TYPEIP;
    charqab.qab_buffer2 = (char *)&(parmbfr->b.p);
    if ((rtn = svcIoQueue(&charqab)) < 0 || (rtn = charqab.qab_error) < 0)
        dcharerr(rtn, &(parmbfr->b.p)); // If error
    ++charcnt;

    type = parmbfr->b.p.type & 0x0F;

    if (type == REP_STR || type == REP_DATAS)
    {
        needed = parmbfr->b.p.size;
        parmbfr->b.p.val.pnt = getspace(needed);
    }
    else
        needed = parmbfr->b.p.length;
    chkbuffer(type, needed);
    parmbfr->b.p.type |= (PAR_GET|PAR_INFO);
    if (!brief && parmbfr->b.p.infosize)
    {
        parmbfr->b.p.infopnt = getspace(parmbfr->b.p.infosize);
        if ((parmbfr->b.p.infosize + 2) > labelsize)
            labelsize = parmbfr->b.p.infosize + 2;
    }
    if (!(arg->flags & ADF_NONE))	// Have a value to set?
    {
        parmbfr->b.p.type |= (PAR_SET|PAR_INFO); // Yes
        if ((parmbfr->b.p.type & 0xF) >= REP_TEXT) // Need string value?
        {
            if (arg->flags & ADF_LSVAL)
                vsize = arg->length;
            else
            {
                vsize = 0;
                arg->val.s = "";
            }
            if (type == REP_STR || type == REP_DATAS)
                size = parmbfr->b.p.size - 1;
            else
                size = parmbfr->b.p.length;
            if (vsize > size)		// Is our value too long?
            {
                if (!mute)
                    fprintf(stderr, "? %s: String value for characteristic"
                            " %s is too long for "
#if defined(CLSCHAR) || defined(SYSCHAR)
                            "class"
#else
                            "device"
#endif
                            " %s\n", prgname, arg->name, devname);
                exit(EXIT_INVSWT);
            }
            if (type == REP_STR || type == REP_DATAS)
            {
                strncpy((char near *)(parmbfr->b.p.val.pnt), arg->val.s, vsize);
                parmbfr->b.p.amount = vsize;
            }
            else
                strncpy(parmbfr->b.p.val.ss, arg->val.s, vsize);
        }
        else				// If need numeric value
        {
            value = 0;
            if ((vsize = arg->length) != 0)
            {
                list = FALSE;
                radix = 10;
                pnt = arg->val.s;
                while ((chr = *pnt++) != 0)
                {
                    if (chr == '.')
                    {
                        list = TRUE;
                        break;
                    }
                    if (chr == '-')
                    {
                        radix = 16;
                        list = TRUE;
                        break;
                    }
                }
                pnt = arg->val.s;

                if (!list)
                {
                    if (vsize > 0 && *pnt == '0')
                    {
                        pnt++;
                        vsize--;
                        radix = 8;
                        if (vsize > 0 && toupper(*pnt) == 'X')
                        {
                            pnt++;
                            vsize--;
                            radix = 16;
                        }
                    }
                    while ((chr = *pnt++) != 0)
                    {
                        if (chr == '.' || chr == '-')
                            break;
                        if (isxdigit(chr))
                        {
                            if (chr > 'A')
                                chr += 9;
                            chr &= 0x0F;
                            if (chr < radix)
                            {
                                value *= radix;
                                value += chr;
                                continue;
                            }
                        }
                    }
                }
                else
                {
                    bpnt = (uchar *)&value;
                    cnt = 4;
                    do
                    {
                        bval = 0;
                        while ((chr = *pnt++) != 0)
                        {
                            if (chr == '.' || chr == '-')
                                break;
                            if (isxdigit(chr))
                            {
                                if (chr > 'A')
                                    chr += 9;
                                chr &= 0x0F;
                                if (chr < radix)
                                {
                                    bval *= radix;
                                    bval += chr;
                                    continue;
                                }
                            }
                            illnumval(arg);
                        }
                        if (bval & 0xFFFFFF00)
                            illnumval(arg);
                        *bpnt++ = (uchar)bval;
                    } while (--cnt > 0 && chr != 0);
                }
                if (chr != 0)
                    illnumval(arg);
            }
            vsize = (value & 0xFFFF0000L)? 4:
                    (value & 0xFFFFFF00L)? 2:
                    (value != 0)? 1: 0;
            if (vsize > parmbfr->b.p.length)
            {
                if (!mute)
                    fprintf(stderr, "? %s: Numeric value for characteristic "
                            "%s is out of range for "
#if defined(CLSCHAR) || defined(SYSCHAR)
                            "class"
#else
                            "device"
#endif
                            " %s\n", prgname, arg->name, devname);
                exit(EXIT_INVSWT);
            }
            parmbfr->b.p.val.num.low = value;
            parmbfr->b.p.val.num.high = 0;
        }
    }
    chartotal += (parmbfr->b.p.length) + 22;
    parmbfr->b.s[parmbfr->b.p.length + 22] = 0;
    return (TRUE);
}

void illnumval(
    arg_data *arg)

{
    fprintf(stderr, "? %s: Illegal numeric value given for characteristic %s "
            "for "
#if defined(CLSCHAR) || defined(SYSCHAR)
            "class"
#else
            "device"
#endif
            " %s\n", prgname, arg->name, devname);
    exit(EXIT_INVSWT);
}


void xprintf(
    const char *format, ...)

{
    long rtn;

    if ((rtn = vsprintf(xpfpnt, format, (va_list)((&format)+1))) < 0)
        femsg2(prgname, "Error outputting to stdout", rtn, NULL);
    outsize += rtn;
    xpfpnt += rtn;
}

// Function to determine size of buffer needed for value

void chkbuffer(
    int type,
    int length)

{
    int need;

    need = 0;
    switch (type)
    {
     case REP_DECV:
     case REP_HEXV:
     case REP_OCTV:
        need = length * 4;
        break;
     case REP_BINV:
     case REP_DECB:
     case REP_HEXB:
     case REP_OCTB:
        need = length * 5;
        break;
     case REP_VERN:
     case REP_TIME:
     case REP_DATE:
     case REP_DT:
        need = 20;
        break;
     case REP_DATAS:
        need = 20 + length * 3;
        break;
     case REP_TEXT:
     case REP_STR:
        need = length;
    }
    if (bufmax < need)
        bufmax = need;
}

// Function to report error getting or setting characteristic value

void dcharerr(
    long   code,
    struct parm *parm)

{
    char errmsg[100];

    svcSysErrMsg(code, 3, errmsg); // Get error message string
    do					// Scan to find bad characteristic
    {
        if (parm->type & PAR_ERROR)
        {
            if (!mute)
                fprintf(stderr, "? %s: Error %s characteristic %-0.8s for "
#if defined(CLSCHAR) || defined(SYSCHAR)
                        "class"
#else
                        "device"
#endif
                        " %s\n           %s\n", prgname,
                    (parm->type & PAR_SET)? "setting": "getting", parm->name,
                    devname, errmsg);
            exit(EXIT_INVSWT);
        }
    } while ((parm = nextparm(parm))->type != 0);
    if (!mute)
        fprintf(stderr, "? %s: Error getting or setting characteristic for "
#if defined(CLSCHAR) || defined(SYSCHAR)
                "class"
#else
                "device"
#endif
                " %s\n           %s\n", prgname, devname, errmsg);
    exit(EXIT_INVSWT);
}

void fatal(
    long rtn)

{
    char buffer[50];

    sprintf(buffer,
#if defined(CLSCHAR) || defined(SYSCHAR)
            "class"
#else
            "device"
#endif
            " %s", devname);
    femsg(prgname, rtn, buffer);
}
