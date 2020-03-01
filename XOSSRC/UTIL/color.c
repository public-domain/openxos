//------------------------------------------------------------------------------
//  COLOR.C - Command to set or display display colors
//
//  Written by: John R. Goltz
//
//  Edit History:
//  ------------
//  08/20/92(brn) - Add comment header
//  05/12/94(brn) - Fix command abbreviations
//  02/28/95(sao) - Fixed quiet mode, added mute mode, added progarg pkg
//                                       changed Color selections from keywords too options.
//  05/13/95(sao) - Changed 'optional' indicators
//  05/16/95(sao) - Changed exit codes to reflect ALGRTN
//  05/16/95(sao) - Changed exit codes to reflect ALGRTN
//  18May95 (fpj) - Fixed bug which allowed background colors with values > 7 --
//                  this caused the screen to blink.
//  18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                  optusage() to opthelp().
//  26May95 (fpj) - Re-worked code to use errmsg(); removed ability for users
//                  to set foreground and background color or fill to same
//                  value; changed code to use terminal attributes structure
//                  in ALGTRM.H; fixed bug which printed suboption descriptions
//                  incorrectly.
//------------------------------------------------------------------------------

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
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSRTN.H>
#include <XOSTRM.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <XOSERRMSG.H>


#define VERSION 3
#define EDITNO  6

#define ERRBITS             (EM_IQM | EM_NAME | EM_CODE | EM_TEXT | EM_EXIT)                                        // Bits to use for errmsg() calls

int gotGSwitch( arg_data *);
void display(long func, char *msg);
void getarg(char *argv[]);
void printsubopt(const char *subopt);


TRMATTRIB current_colors  = { -1, -1, -1, -1, 0 };
TRMATTRIB set_colors      = { -1, -1, -1, -1, 0 };

long  value = -1;
char  havecolor;                // TRUE if any color value specified
long  gswitch;                  // TRUE if changing graphics colors
long  quiet = FALSE;            // TRUE if /QUIET seen
long  mute = FALSE;             // TRUE if /MUTE seen


SubOpts color[] =
{
    { "BLA*CK",     "[0] Low intensity Black" },
    { "BLU*E",      "[1] Low intensity Blue" },
    { "GRE*EN",     "[2] Low intensity Green" },
    { "C*YAN",      "[3] Low intensity Cyan" },
    { "R*ED",       "[4] Low intensity Red" },
    { "V*IOLET",    "[5] Low intensity Violet" },
    { "BR*OWN",     "[6] Low intensity Brown" },
    { "W*HITE",     "[7] Low intensity White" },
    { "GRA*Y",      "[8] Low intensity Gray" },
    { "BBLU*E",     "[9] High intensity Blue" },
    { "BG*REEN",    "[10] High intensity Green" },
    { "BC*YAN",     "[11] High intensity Cyan" },
    { "BR*ED",      "[12] High intensity Red" },
    { "BV*IOLET",   "[13] High intensity Violet" },
    { "Y*ELLOW",    "[14] High intensity Yellow" },
    { "BW*HITE",    "[15] High intensity White" },
    { NULL,         NULL }
};


SubOpts attrb[] =
{
    { "B*LINK",         "Output should blink" },
    { "NOB*LINK",       "Output should not blink" },
    { "U*NDERLINE",     "Output should be underlined" },
    { "NOU*NDERLINE",   "Output should not be underlined" },
    { "X*OR",           "Output should be inverted" },
    { "NOX*OR",         "Output should not be inverted" },
    { NULL,             NULL }
};

arg_spec keywords[] =
{
    {"BGC", ASF_VALREQ|ASF_XSVAL|ASF_NVAL|ASF_STORE, &color,
        &set_colors.backgnd, 0, NULL},
    {"FGF", ASF_VALREQ|ASF_XSVAL|ASF_NVAL|ASF_STORE, &color,
        &set_colors.forefill, 0, NULL},
    {"BGF", ASF_VALREQ|ASF_XSVAL|ASF_NVAL|ASF_STORE, &color,
        &set_colors.backfill, 0, NULL},
    {"FGC", ASF_VALREQ|ASF_XSVAL|ASF_NVAL|ASF_STORE, &color,
        &set_colors.foregnd, 0,
        "Sets the foreground color.  Either the color or color number may be "
        "entered.  BGC, FGF and BGF all use the same color list"},
    {"ATR", ASF_VALREQ|ASF_XSVAL|ASF_STORE, &attrb, &value , 0,
        "Display attributes" },
    {NULL      , 0, NULL, AF(NULL)   , 0, NULL}
};

arg_spec options[] =
{
    {"T*EXT"    , 0, NULL, &gotGSwitch,
        FALSE,  "Set colors for text screens"},
    {"G*RAPHICS", 0, NULL, &gotGSwitch,
        TRUE,  "Set colors for graphics screens"},
    {"M*UTE"    , ASF_BOOL|ASF_STORE, NULL, &mute,
        TRUE,  "Send no messages"},
    {"Q*UIET"   , ASF_BOOL|ASF_STORE, NULL, &quiet,
        TRUE,  "Send only error messages"},
    {"H*ELP"    , 0, NULL, opthelp,
        0,      "This message"},
    {"?"        , 0, NULL, opthelp,
        0,      "This message"},
    {NULL      , 0, NULL, AF(NULL)   , 0, NULL}
};

char  example[] = "{/options} {FGC=color {BGC=c {FGF=c {BGF=c {ATR=a}}}}}";
char  prgname[] = "COLOR";
char  envname[] = "^XOS^COLOR^OPT"; // The environment option name
char  copymsg[] = "";
char  descrip[] = "This command sets the default console display colors for "
    "text and graphics modes.  Foreground, background, fill colors and "
    "display attributes can be set independently.  The default colors are "
    "used when displaying text to the screen using normal text terminal "
    "output functions.  They have no effect when writing directly to the "
    " screen buffer or using graphics functions.  If the command is given "
    "with no arguments, the current default colors are displayed.  If the "
    "/TEXT (default) or /GRAPHICS options are specified, the indicated set"
    " of default colors are set to the values given.  Any attributes not "
    "given are not changed.";

Prog_Info pib;


//
//  Function: main() - Main entry for COLOR utility
//
//  Return: XOS exit status
//
//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    long  rtn;

#ifdef  __WATCOMC__

    (void)argc;                         // (define parameter as unused)

#endif

    getarg(++argv);                     // Parse command-line arguments

    // First, get current colors

    if ((rtn = svcTrmAttrib(DH_STDTRM, 0x43, &current_colors)) < 0)
    {
        if (!mute)
            errmsg(ERRBITS, NULL, rtn, "Cannot get current color information",
                    NULL);
        else
            return EXIT_SVCERR;
    }

    if (set_colors.foregnd == -1 && set_colors.backgnd == -1
            && set_colors.forefill == -1 && set_colors.backfill == -1)
    {                                   // Want current colors?
        if ( !quiet )                   // Yes, so display them and exit
        {
            display(0x41, "Text");
            display(0x42, "Graphics");
        };

        return EXIT_NORM;
    }

    if ( set_colors.foregnd > 15 )
    {
        if (!mute)
            errmsg(ERRBITS, NULL, rtn, "Invalid foreground color", NULL);
        else
            return EXIT_INVSWT;
    }

    if ( set_colors.backgnd > 7 )
    {
        if (!mute)
            errmsg(ERRBITS, NULL, rtn, "Invalid background color", NULL);
        else
            return EXIT_INVSWT;
    }

    if ( set_colors.forefill > 15 )
    {
        if (!mute)
            errmsg(ERRBITS, NULL, rtn, "Invalid foreground fill", NULL);
        else
            return EXIT_INVSWT;
    }

    if ( set_colors.backfill > 7 )
    {
        if (!mute)
            errmsg(ERRBITS, NULL, rtn, "Invalid background fill", NULL);
        else
            return EXIT_INVSWT;
    }

    // If foreground fill not specified, set equal to foreground color
    
    if ((set_colors.foregnd != -1) && (set_colors.forefill == -1))
        set_colors.forefill = set_colors.foregnd;

    // If background fill not specified, set equal to background color

    if ((set_colors.backgnd != -1) && (set_colors.backfill == -1))
        set_colors.backfill = set_colors.backgnd;

    // Make sure he doesn't try setting foreground and background colors
    //  the same

    if ( (set_colors.foregnd != -1 || set_colors.backgnd != -1)
            && ( ( set_colors.foregnd == set_colors.backgnd)
                || (set_colors.foregnd == current_colors.backgnd)
                || (set_colors.backgnd == current_colors.foregnd) ) )
    {
        if ( !mute )
            errmsg(ERRBITS, NULL, rtn, "Foreground and background colors"
                    " cannot be the same", NULL);
        else
            return EXIT_INVSWT;
    }

    // Make sure he doesn't try setting foreground and background fill
    //  the same

    if ( (set_colors.forefill != -1 || set_colors.backfill != -1)
            && ( ( set_colors.forefill == set_colors.backfill)
                || (set_colors.forefill == current_colors.backfill)
                || (set_colors.backfill == current_colors.forefill) ) )
    {
        if ( !mute )
            errmsg(ERRBITS, NULL, rtn, "Foreground and background fill"
                    " cannot be the same", NULL);
        else
            return EXIT_INVSWT;
    }
    rtn = svcTrmAttrib(DH_STDTRM, (gswitch) ? 0x82: 0x81, &set_colors);

    if (rtn < 0)
    {
        if (!mute)
            errmsg(ERRBITS, NULL, rtn, "Cannot change the base colors", NULL);
        else
            return EXIT_SVCERR;
    }

    if (!gswitch)
    {
        if ((rtn = svcTrmAttrib(DH_STDTRM, 0x83, &set_colors)) < 0)
        {
            if (!mute)
                errmsg(ERRBITS, NULL, rtn, "Cannot change the current colors",
                        NULL);
            else
                return EXIT_SVCERR;
        }
    }

    if ( rtn == 0 )
        return EXIT_NORM;
    else
        return EXIT_SVCERR;
}

//
//  Function: gotGSwitch() - Set up the graphic switch variable
//
//  Return: TRUE
//
//------------------------------------------------------------------------------

int gotGSwitch( arg_data *arg )
{
    gswitch = arg->data;

    return TRUE;
}

//
//  Function: init_Vars() - Set up the program information block
//
//  Return: (n/a)
//
//------------------------------------------------------------------------------

void getarg(char *argv[])
{
    char  strbuf[256];
    char *envpnt[2];

    reg_pib(&pib);

    // Set defaults of control variables

    quiet = FALSE;
    mute = FALSE;

    // set Program Information Block variables

    pib.opttbl = options;               // Load the option table
    pib.kwdtbl = keywords;
    pib.majedt = VERSION;               // major edit number
    pib.minedt = EDITNO;                // minor edit number
    pib.copymsg = copymsg;
    pib.prgname = prgname;
    pib.desc = descrip;
    pib.example = example;
    pib.build = __DATE__;
    pib.errno = 0;
    getTrmParms();
    getHelpClr();

    if (svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
    {
        envpnt[0] = strbuf;
        envpnt[1] = NULL;

        progarg(envpnt, 0, options, NULL, NULL, NULL, NULL, NULL);
    }

    progarg(argv, 0, options, keywords, NULL, NULL, NULL, NULL);

    if ( mute )
        quiet = TRUE;
};


//
//  Function: display() - Display colors
//
//  Return: (n/a)
//
//------------------------------------------------------------------------------

void display(
    long func,
    char *msg)
{
    long      rtn;
    TRMATTRIB colors;

    if ((rtn = svcTrmAttrib(DH_STDTRM, func, &colors)) < 0)
    {
        if (!mute)
            errmsg(ERRBITS, NULL, rtn, (func & 0x01) ?
                "Cannot get text color information" :
                "Cannot get graphics color information", NULL);
        else
            exit(EXIT_SVCERR);
    }


    printf("%s color values:\n    FGC=", msg);

    printsubopt(color[colors.foregnd].option);

    fputs("  BGC=", stdout);

    printsubopt(color[colors.backgnd].option);

    fputs("  FGF=", stdout);

    printsubopt(color[colors.forefill].option);

    fputs("  BGF=", stdout);

    printsubopt(color[colors.backfill].option);

    fputc('\n', stdout);
}


//
//  Function: printsubopt() - Print sub-option string, avoiding special chrs.
//
//  Return: (n/a)
//
//------------------------------------------------------------------------------

void printsubopt(const char *subopt)
{
    char buffer[80];
    char *p = buffer;

    do
    {
        if (*subopt != '*' && *subopt != '|')
            *p++ = *subopt;

    } while (*subopt++);

    fputs(buffer, stdout);
}
