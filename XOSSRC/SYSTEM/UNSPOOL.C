/****************************************************************/
/* Program: UNSPOOL - program to unspool spooled data		*/
/*								*/
/* Edit history:        					*/
/*								*/
/* 13Apr95 (fpj)  Fixed bug in call to svcSchAlarm().   	*/
/****************************************************************/

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

// This program is invoked using the following arguments:
//	UNSPOOL PID UNIT=n OUTPUT=dev DIRECT=dirspec DIGITS=m
// Where:
//	PID    = PID of invoking process
//	UNIT   = Unspool device unit number
//	OUTPUT = Name of the output device
//	DIRECT = Full file specification for the spooling directory
//	CLOSE  = Close timeout value in seconds (default = 0, which means no
//		   close timeout)
//	DIGITS = Number of digits to use for sequence number (default = 3)

// Note that only one positional argument (the invoking PID) is used.  It
// MUST be present and MUST be the first argument.  This argument is
// normally provided by SYMBIONT when UNSPOOL is invoked using the SYMBIONT
// command.  The remaining arguments, which are all keyword arguments, may be
// specified in any order.  All arguments except DIGITS must be specified.

// For example,
//
//	UNSPOOL 60003 UNIT=1 OUTPUT=LPT1: DIRECT=C:\SPOOL\ DIGITS=3

// would use output device LPT1: for data written to spooled device SPL1:.  The
// name of the spool would be SPOOL1 using the directory C:\SPOOL\.  The
// generated name would contain a 3 digit sequence number.  No close timeout
// would be used.

#include <STDIO.H>
#include <STDLIB.H>
#include <CTYPE.H>
#include <STRING.H>
#include <TIME.H>
#include <XCSTRING.H>
#include <MALLOC.H>
#include <XCMALLOC.H>
#include <PROCARG.H>
#include <XOSERR.H>
#include <XOSSVC.H>
#include <XOS.H>
#include <XOSNET.H>
#include <XOSVECT.H>
#include <XOSMSG.H>
#include <XOSTRM.H>
#include <XOSSYSP.H>
#include <XOSTIME.H>

#define VECT_MESSAGE  50
#define VECT_OUTPUT   51
#define VECT_CLOCK    52

#define BUFRSIZE    8192

struct pid
{   char  header;
    ulong pid;
};

struct msg			// General IPC message
{   uchar type;			//   Message type
    char  kind;			//   Name kind
    char  data[160];		//   Data
};

char reqname[64];

struct msg msg;			// Message buffer

struct rspparms
{   lngstr_parm reqname;
    char        end;
} rspparms =
{   {PAR_SET|REP_STR, 0xFF, IOPAR_MSGRMTADDRS, reqname, 0, 64, 64}
};

type_qab rspqab =
{   QFNC_OUTBLOCK,		// qab_func    - Function
    0,				// qab_status  - Returned status
    0,				// qab_error   - Error code
    0,				// qab_amount  - Amount transfered
    0,				// qab_handle  - Device handle
    0,				// qab_vector  - Vector for interrupt
    0, 0,			//             - Reserved
    0,				// qab_option  - Options
    sizeof(msg),		// qab_count   - Amount to transfer
    (char far *)(&msg), 0,	// qab_buffer1 - Pointer to first buffer
    NULL, 0,			// qab_buffer2 - Pointer to second buffer
    (char far *)(&rspparms), 0	//qab_parm   - Pointer to parameter list
};

type_qab msgqab =
{   QFNC_INBLOCK,		// qab_func    - Function
    0,				// qab_status  - Returned status
    0,				// qab_error   - Error code
    0,				// qab_amount  - Amount transfered
    0,				// qab_handle  - Device handle
    VECT_MESSAGE,		// qab_vector  - Vector for interrupt
    0, 0,			//             - Reserved
    0,				// qab_option  - Options
    sizeof(msg),		// qab_count   - Amount to transfer
    (char far *)(&msg), 0,	// qab_buffer1 - Pointer to first buffer
    NULL, 0,			// qab_buffer2 - Pointer to second buffer
    NULL, 0			// qab_parm    - Pointer to parameter list
};

type_qab outqab =
{   QFNC_OUTBLOCK,		// qab_func    - Function
    0,				// qab_status  - Returned status
    0,				// qab_error   - Error code
    0,				// qab_amount  - Amount transfered
    0,				// qab_handle  - Device handle
    VECT_OUTPUT,		// qab_vector  - Vector for interrupt
    0, 0,			//             - Reserved
    0,				// qab_option  - Options
    0,				// qab_count   - Amount to transfer
    NULL, 0,			// qab_buffer1 - Pointer to first buffer
    NULL, 0,			// qab_buffer2 - Pointer to second buffer
    NULL, 0			// qab_parm    - Pointer to parameter list
};


long  dirhandle;
long  filehandle;
uint  unitnum = -1;		// Spooled device unit number
int   numdigits;		// Number of sequence number digits in
				//   generated name
long  numdel;

char  outdev[32];		// Output device name
char  spoolname[16] = "IPM:SPL^"; // Spool name
#define spoolnamex (spoolname+8)
char  ourname[16];
char *prgname = "UNSPOOL";
char *dirspec;			// Directory specification
char *splspec;
int   dirlength;
char  delname[32];
char  clsname[32];
char  spldevname[16];

struct splparms
{   byte4_parm offset;
    byte4_parm dirhandle;
    char       end;
} splparms =
{   {PAR_SET|REP_DECV, 4, IOPAR_DIROFS , 0},
    {PAR_SET|REP_DECV, 4, IOPAR_DIRHNDL, 0}
};

struct addchars
{   byte4_char  unit;
    char        end;
} addchars =
{   {PAR_SET|REP_HEXV, 4, "UNIT", 0}
};

type_qab addqab =
{   QFNC_CLASSFUNC,		// qab_func    - Function
    0,				// qab_status  - Returned status
    0,				// qab_error   - Error code
    0,				// qab_amount  - Amount transfered
    0,				// qab_handle  - Device handle
    0,				// qab_vector  - Vector for interrupt
    0, 0,			//             - Reserved
    CF_ADDUNIT,			// qab_option  - Options
    0,				// qab_count   - Amount to transfer
    (char near *)"SPL:", 0,	// qab_buffer1 - Pointer to first buffer
    (char *)(&addchars), 0,	// qab_buffer2 - Pointer to second buffer
    NULL, 0			// qab_parm    - Pointer to parameter list
};

struct splchars
{   byte4_char  seqnum;
    lngstr_char splspec;
    lngstr_char clsname;
    text16_char clsmsg;
    byte4_char  clstime;
    char        end;
} splchars =
{   {PAR_SET|REP_HEXV, 4  , "SEQNUM" , 1},
    {PAR_SET|REP_STR , 0  , "SPLSPEC"},
    {PAR_SET|REP_STR , 0  , "CLSNAME", clsname},
    {PAR_SET|REP_TEXT, 16 , "CLSMSG" , ""},
    {PAR_SET|REP_DECV, 4  , "CLSTIME", 0}
};

type_qab charqab =
{   QFNC_DEVCHAR,		// qab_func    - Function
    0,				// qab_status  - Returned status
    0,				// qab_error   - Error code
    0,				// qab_amount  - Amount transfered
    0,				// qab_handle  - Device handle
    0,				// qab_vector  - Vector for interrupt
    0, 0,			//             - Reserved
    DCF_VALUES,			// qab_option  - Options
    0,				// qab_count   - Amount to transfer
    NULL, 0,			// qab_buffer1 - Pointer to first buffer
    (char *)(&splchars), 0,	// qab_buffer2 - Pointer to second buffer
    NULL, 0			// qab_parm    - Pointer to parameter list
};

struct queue
{   struct queue *next;
    char   name[20];
};

struct queue *current;
struct queue *headpnt;
struct queue *tailpnt;

void argerr(char *msg1, char* msg2);
long argvalue(char *str, int radix);
void clocksrv(void);
void command(void);
void fatalmsg(long code, char *msgstr, char *name);
void fileready(void);
void fixarg(char *arg);
int  haveclose(arg_data *);
int  havedirect(arg_data *);
int  havedigits(arg_data *);
int  haveoutput(arg_data *);
int  haveunit(arg_data *);
void messagesrv(void);
void outputsrv(void);
void response(int code);
void setuperr(long code, char *msgstr, char *name);
void startprint(void);

arg_spec keywords[] =
{   {"OUTPUT", ASF_VALREQ|ASF_LSVAL, NULL, haveoutput, 0},
    {"UNIT"  , ASF_VALREQ|ASF_NVAL , NULL, haveunit  , 0},
    {"DIRECT", ASF_VALREQ|ASF_LSVAL, NULL, havedirect, 0},
    {"DIGITS", ASF_VALREQ|ASF_NVAL , NULL, havedigits, 0},
    {"CLOSE" , ASF_VALREQ|ASF_NVAL , NULL, haveclose , 0},
    {NULL    , 0                   , NULL, NULL      , 0}
};

/********************************/
/* Function: main - main proram	*/
/* Returned: Never returns	*/
/********************************/

main(argc, argv)
int   argc;
char *argv[];

{
    long  rtn;
    char *pnt;

    if (argc < 2)			// Was a process ID specified?
        exit(1);			// No - just exit quietly since there
					//   is no way we can complain!
    argv++;				// Probably - process arguments

    strncpy(reqname, *argv++, 64);	// Get requester's name
    if (strcmp(reqname, "0") == 0)
        reqname[0] = 0;
    procarg(argv, 0, NULL, keywords, (int (*)(char *))NULL, argerr,
            (int (*)(void))NULL, NULL);	// Process other arguments

    if (unitnum == (uint)-1)
        setuperr(0, "No unit number specified", NULL);
    if (outdev[0] == '\0')
        setuperr(0, "No output device specified", NULL);
    if (dirspec == NULL)
        setuperr(0, "No directory specified", NULL);
    sprintf(spoolnamex, "S%02.2d", unitnum);
    if ((rspqab.qab_handle = msgqab.qab_handle =
            svcIoOpen(O_OUT|O_IN, spoolname, NULL)) < 0)
    {					// If error, try to use a blank name
        spoolnamex[0] = 0;		//   to get a startup message back
        rspqab.qab_handle = svcIoOpen(O_OUT, spoolname, NULL);
        setuperr(msgqab.qab_handle, "Cannot set up to handle messages", NULL);
    }
    if ((outqab.qab_buffer1 = malloc(BUFRSIZE)) == NULL)
        setuperr(0, "Cannot allocate output buffer", NULL);
    setvector(VECT_MESSAGE, 0x84, messagesrv); // Set message vector
    setvector(VECT_OUTPUT, 0x84, outputsrv); // Set output done vector
    if ((rtn = svcIoQueue(&msgqab)) < 0)
        setuperr(rtn, "Error starting message input", NULL);

//  printf("#### outdev: %s\n", outdev);

    if ((outqab.qab_handle = svcIoOpen(O_OUT, outdev, NULL)) < 0)
        setuperr(outqab.qab_handle, "Error opening output device", NULL);

    pnt = dirspec;
    while (*pnt != '\0')
        ++pnt;
    if (pnt[-1] != '\\')
        *pnt = '\\';

//  printf("#### dirspec: %s\n", dirspec);

    if ((dirhandle = svcIoOpen(O_ODF, dirspec, NULL)) < 0)
        setuperr(dirhandle, "Error opening spool directory", NULL);
    sprintf(ourname, "Unspool_%02.2d", unitnum);
    svcSysSetPName(ourname);

    sprintf(delname, "S%02.2d_*.*", unitnum);
    splparms.dirhandle.value = dirhandle;
    if ((numdel = svcIoDelete(O_REPEAT, delname, &splparms)) < 0)
        numdel = 0;
    sprintf(&((char *)(&msg))[1], "UNSPOOL: %ld file%s deleted from %s",
            numdel, (numdel == 1)? "": "s", dirspec);
    response(MT_INTRMDMSG);

    splchars.splspec.buffer = (char far *)getspace(dirlength+16);
    sprintf((char *)splchars.splspec.buffer, "%sS%02.2d_#4.TMP", dirspec,
            unitnum);
    sprintf(clsname, "S%02.2d_#4.SPL", unitnum);
    sprintf(splchars.clsmsg.value, "SPL^S%02.2d", unitnum);
    addchars.unit.value = unitnum;
    if (((rtn = svcIoQueue(&addqab)) < 0 || (rtn = addqab.qab_error) < 0) &&
            rtn != ER_DUADF)
        setuperr(rtn, "Error defining unit for spool device", NULL);    
    sprintf(spldevname, "SPL%d:", unitnum);
    if ((charqab.qab_handle = svcIoOpen(O_PHYS, spldevname, 0)) < 0)
        setuperr(charqab.qab_handle, "Error opening spool device", NULL);

    if ((rtn = svcIoQueue(&charqab)) < 0 || (rtn = charqab.qab_error) < 0)
        setuperr(rtn, "Error setting characteristics for spool device", NULL);
    svcIoClose(charqab.qab_handle, 0);

    // Here with all preliminary stuff done - tell the invoking process that
    //   we are running

    sprintf(&((char *)(&msg))[1], "UNSPOOL: SPL%d: set up to spool to %s using "
            "directory %s", unitnum, outdev, dirspec);
    response(MT_FINALMSG);		// Send final response to invoking
					//   process - NOTE: if invoked as
					//   a symbiont, no errors can be
					//   reported after this!

    // Now we scan the spool directory to see if there are any files we care
    //   about - we care about files which start with our 4 character name
    //   which have an extension of TMP or SPL


// qqqqqqqqqqq

    setvector(VECT_CLOCK, 0x82, clocksrv); // Set up the every-ten-seconds
					   //   routine
    svcSchAlarm(4, 0, VECT_CLOCK, 0, 0, ((long)XT_SECOND)*10);
    svcSchSetLevel(0);			// Allow all software interrupts now
    for  (;;)				// Wait for something to happen
        svcSchSuspend(NULL, -1);
}

/************************************************/
/* Function: argerr - process procarg errors	*/
/* Returned: Nothing				*/
/************************************************/

void argerr(
    char *msg1, char* msg2)

{
    char *pnt;
    char  buffer[300];

    pnt = strmov(buffer, msg1);
    if (msg2 != NULL)
        strmov(pnt, msg1);
    setuperr(0, buffer, NULL);
}

/********************************************************/
/* Function: haveoutput - process OUTPUT keyword	*/
/* Returned: Nothing					*/
/********************************************************/

int haveoutput(
    arg_data *arg)

{
    fixarg(arg->val.s);			// Get output device name
    strmov(strnmov(outdev, arg->val.s, 30), ":");
    return (TRUE);
}

/************************************************/
/* Function: haveunit - process UNIT keyword	*/
/* Returned: Nothing				*/
/************************************************/

int haveunit(
    arg_data *arg)

{
    unitnum = (int)(arg->val.n);
    if (unitnum > 99)
        setuperr(0, "Illegal unit number specified", NULL);
    return (TRUE);
}

/********************************************************/
/* Function: havedirect - process DIRECT keyword	*/
/* Returned: Nothing					*/
/********************************************************/

int havedirect(
    arg_data *arg)

{
    dirlength = arg->length+2;
    dirspec = getspace(dirlength);	// Allocate space for string
    strcpy(dirspec, arg->val.s);	// Copy the string
    return (TRUE);
}

/********************************************************/
/* Function: havedigits - process DIGITS keyword	*/
/* Returned: Nothing					*/
/********************************************************/

int  havedigits(
    arg_data *arg)

{
    numdigits = (int)(arg->val.n);
    return (TRUE);
}

/************************************************/
/* Function: haveclose - process CLOSE keyword	*/
/* Returned: Nothing				*/
/************************************************/

int  haveclose(
    arg_data *arg)

{
    splchars.clstime.value = arg->val.n;
    return (TRUE);
}

/********************************************************/
/* Function: clocksrv - Software interrupt routine	*/
/*		executed every ten seconds		*/
/* Returned: Nothing					*/
/********************************************************/

void clocksrv(void)

{
}

/********************************************************/
/* Function: messagesrv - Software interrupt routine	*/
/*		for message available			*/
/* Returned: Nothing					*/
/********************************************************/

void messagesrv(void)

{
    long   rtn;

    while (msgqab.qab_status & QSTS_DONE)
    {
        if (msgqab.qab_error >= 0 && msgqab.qab_amount >= 3)
        {				// Yes - make sure no errors and that
					//   message contains all of the
					//   header
            if (msg.type == MT_UNSPLREADY)
                fileready();
            else if (msg.type == MT_UNSPLCMD)
                command();
        }
        msgqab.qab_count = sizeof(msg);
        msgqab.qab_vector = 0;		// Restart message input (suppress
        if ((rtn = svcIoQueue(&msgqab)) < 0) //   interrupt if have another
        {				      //   message immediately)
            return;
        }
        msgqab.qab_vector = VECT_MESSAGE;
    }
}

/********************************************************/
/* Function: fileready - Process file ready message	*/
/* Returned: Nothing					*/
/********************************************************/

void fileready(void)

{
    struct queue *pnt;

    if (msgqab.qab_amount > 19)		// Ignore message if too long
        return;
    msg.data[(int)(msgqab.qab_amount)-2] = '\0'; // Make sure have final null

//  printf("#### fileready: kind: %c, name: %s\n", msg.kind, msg.data);

    if (msg.kind == 'n')
    {
        if ((pnt = malloc(sizeof(struct queue))) == NULL)
        {
            return;
        }
        strcpy(pnt->name, msg.data);
        if (tailpnt == NULL)		// Link into our queue
            headpnt = pnt;
        else
            tailpnt->next = pnt;
        tailpnt = pnt;
        if (current == NULL)		// Start output if idle
            startprint();
    }
}

/************************************************/
/* Function: command - Process command message	*/
/* Returned: Nothing				*/
/************************************************/

void command(void)

{
}

/************************************************/
/* Function: startprint - Start printing a file	*/
/* Returned: Nothing				*/
/************************************************/

void startprint(void)

{
    long rtn;

    while (headpnt != NULL)
    {
        current = headpnt;
        if ((headpnt = headpnt->next) == NULL)
            tailpnt = NULL;
        if ((filehandle = svcIoOpen(O_IN|O_OUT, current->name, &splparms)) < 0)
        {
//          printf("#### startprint: open error %ld\n", filehandle);
            current = NULL;
            continue;
        }
        if ((outqab.qab_count = svcIoInBlock(filehandle, outqab.qab_buffer1,
                BUFRSIZE)) <= 0)
        {
//          printf("#### startprint: input error %ld\n", outqab.qab_count);
            svcIoClose(filehandle, C_DELETE);
            current = NULL;
            continue;
        }
//      printf("#### output count = %ld\n", outqab.qab_count);
        if ((rtn = svcIoQueue(&outqab)) < 0)
        {
//          printf("#### startprint: output error %ld\n", rtn);
            svcIoClose(filehandle, C_DELETE);
            current = NULL;
            continue;
        }
        return;
    }
}

/********************************************************/
/* Function: outputsrv - Software interrupt routine	*/
/*		for printer output done			*/
/* Returned: Nothing					*/
/********************************************************/

void outputsrv(void)

{
    if (outqab.qab_error < 0)		// Output error?
    {					// Yes
//      printf("#### outputsrv: output error %ld\n", outqab.qab_error);
        svcSchSuspend(NULL, XT_SECOND*5); // Wait for 5 seconds!
//      printf("#### output count = %ld\n", outqab.qab_count);
        svcIoQueue(&outqab);		// Try the output again
        return;
    }

    if ((outqab.qab_count = svcIoInBlock(filehandle, outqab.qab_buffer1,
            BUFRSIZE)) <= 0)
    {
//      printf("#### outputsrv: input error %ld\n", outqab.qab_count);
        svcIoClose(filehandle, C_DELETE);
        current = NULL;
        startprint();
    }
    else
    {
        if (svcIoQueue(&outqab) < 0)
        {
            svcIoClose(filehandle, C_DELETE);
            current = NULL;
            startprint();
        }
    }
}

/********************************************************/
/* Function: setuperr - report error during setup while	*/
/*		can still talk to the invoking process	*/
/* Returned: Never returns				*/
/********************************************************/

void setuperr(
    long  code,		// Error code
    char *msgstr,	// Message string
    char *name)		// Device name

{
    fatalmsg(code, msgstr, name);
    response(MT_FINALERR);
    exit(1);
}

void fatalmsg(
    long  code,		// Error code
    char *msgstr,	// Message string
    char *name)		// Device name

{
    char *pnt;

    pnt = strmov(strmov(&((char *)(&msg))[1], "? UNSPOOL: "), msgstr);
    if (name != NULL)
        pnt = strmov(pnt, name);
    if (code != 0)
    {
        pnt = strmov(pnt, "\n           ");
        pnt = pnt + (int)svcSysErrMsg(code, 3, pnt);
    }
}

/************************************************************************/
/* Function: response - send response to invoking process during	*/
/*		startup - response message is taken from msg		*/
/* Returned: Nothing							*/
/************************************************************************/

void response(
    int code)		// Response code

{
    if (reqname[0] == 0)
        fprintf(stderr, "Response = %d\n%s\n", code, ((char *)(&msg))+1);
    else
    {
        if (rspqab.qab_handle != 0 ||
                (rspqab.qab_handle = svcIoOpen(O_OUT|O_IN, "IPM:", NULL)) > 0)
        {
            msg.type = code;
            rspqab.qab_count = strlen((char *)(&msg));
            svcIoQueue(&rspqab);
        }
    }
}

/********************************************************/
/* Function: argvalue - Get value from argument string	*/
/* Returned: Value					*/
/********************************************************/

long argvalue(
    char *str,
    int radix)

{
    long value;
    char chr;

    value = 0;
    while (isxdigit(chr=*str++))
    {
        if (chr > '9')
            chr += 9;
        chr &= 0xF;
        if (chr >= radix)
            return(-1);
        value = value * radix + chr;
    }
    if (chr != '\0')
        return (-1);
    return (value);
}

/************************************************************************/
/* Function: fixarg - Fix up device name argument - name is converted	*/
/*		to upper case and any trailing colon is removed		*/
/* Returned: Nothing							*/
/************************************************************************/

void fixarg(
    char *arg)

{
    strupper(arg);			// Convert to upper case
    while (*arg != '\0' && *arg != ':')	// Remove colon from end of name
        ++arg;
    *arg = '\0';
}
