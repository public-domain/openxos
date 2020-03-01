//---------------------------------------------------------------------
// LPRT.C
//
// Written by: John R. Goltz
//
// Edit History:
// 08/02/91(brn) - Add error handling for fprintf
// 08/05/91(brn) - Add help and correct name when options are specified
//---------------------------------------------------------------------

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

#include <CTYPE.H>
#include <ERRNO.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <TIME.H>
#include <XCSTRING.H>
#include <MALLOC.H>

//#include <XCMALLOC.H>

#include <PROCARG.H>
#include <XOSTIME.H>
#include <XOS.H>
#include <XOSERMSG.H>
#include <XOSSVC.H>
#include <XOSRTN.H>
#include <XOSERR.H>
#include <LOWIO.H>

struct inname
{   struct inname *next;
    char   name[2];
};
struct inname *firstfile;
struct inname *lastfile = (struct inname *)&firstfile;
int    majedt = 1;		/* major edit number */
int    minedt = 2;		/* minor edit number */
int    pagesize;
int    pos;
int    xpfcnt;
int    leftmar;
int    downamnt;
long   pagenuma;
long   pagenumb;
long   pagenumg;
long   startpg;
long   finishpg = 0x7FFFFFFEL;
int    widthv = 80;
char  *spnt;
char  *dpnt;
char  *pntc;
char  *pntx;
char  *linepnt;
char  *inpname;
char  outname[50] = "LZPRT:";
char  *xpfpnt;
char **args;
char **argp;
FILE  *infile;
FILE  *outfile;
FILE  *gblfile;
long   linenum = 0;
int    linecnt;
char   lsflag;			/* TRUE if want landscape format */
char   numflag;			/* TRUE if want line numbers */
char   hdrflag;			/* TRUE if want page headers */
char   gblflag;			/* TRUE if want global page numbers */
char   firsttime = TRUE;	/* TRUE for first input file */
char   crstr[102];		/* Buffer for copyright notice */
char   xpfbfr[180];
char   inbfr[258];
char   outbfr[1200] = "(";
char    copymsg[] = "";		/* Our Copyright notice */
char   prgname[] = "LPRT";
char   envname[] = "^XOS^LPRT^OPT";
char   optchar1 = '-';			/* option character 1 */
char   optchar2 = '/';			/* option character 2 */

/* Summary of the Postscript items (items flagged with E are external, that
 *   is, they are generated as the listing is produced, those flagged with
 *   I are internal, they are only referenced by prolog code):
 *	(string) l	E Fucntion to display a line consisting of string
 *	num s		E Function to skip num lines
 *	f		E Function to terminate a page
 *	b		I Function reset m and p for new page
 *	m		I Stores horizontal position of current line
 *	p		I Stores vertical position of current line
 *	q		E Function to terminate entire listing
 */

char   phdr[] =
"\4%%!PS\n"
"/l {/p p -50 add def show m p moveto} def\n"
"/s {/p exch -50 mul p add def m p moveto} def\n"
"/f {restore gsave showpage grestore b} def\n"
"/b {save /m %d def /p 3200 def m p moveto} def\n"
"/q {restore} def\n"
"0.24 0.24 scale\n"
"/Courier findfont [3960 1.65 %d mul sub %d div 0 0 47 0 0]\n"
"makefont setfont b\n";

char   lshdr[] =
"\4%%!PS\n"
"/l {/p p -33 add def show m p moveto /iu true def} def\n"
"/s {/p exch -33 mul p add def m p moveto} def\n"
"/f {e {/lp exch def initclip /m 1665 def /p -235 def /e false def\n"
" /iu false def 1650 -200 moveto 3240 -200 lineto 3240 -2400 lineto\n"
" 1650 -2400 lineto closepath clip newpath}\n"
" {/rp exch def d b} ifelse\n"
" m p moveto} def\n"
"/b {initclip 60 -200 moveto  60 -2400 lineto 1650 -2400 lineto 1650 -200\n"
" lineto closepath gsave stroke grestore clip newpath /m 75 def /p -235 def\n"
" /e true def} def\n"
"/d {initclip xf setfont 60 -2440 moveto lp show lps -2440 moveto lbl show\n"
" iu {/m 1665 def 1650 -200 moveto 3240 -200 lineto 3240 -2400 lineto\n"
" 1650 -2400 lineto stroke rp stringwidth pop 3235 exch sub\n"
" -2440 moveto rp show} if%s\n"
" nf setfont restore gsave showpage grestore save} def\n"
"/q {e not {d} if restore} def\n"
"/lbl (%s) def /lp () def /rp () def\n"
"0.24 0.24 scale 90 rotate\n"
"/nf /Courier findfont [2600 %d div 0 0 34 0 0] makefont def\n"
"/xf /Helvetica findfont 40 scalefont def\n"
"/sf /Symbol findfont 30 scalefont def\n"
"/cf /Helvetica findfont 30 scalefont def\n%s"
"xf setfont /lps 1650 lbl stringwidth pop 2 div sub def\n"
"/pl (Page ) stringwidth pop def\n"
"nf setfont 2.0 setlinewidth save b m p moveto\n";

int  copyright(arg_data *);
int  downval(arg_data *);
void endpage(void);
void finish(void);
int  finishpage(arg_data *);
int  globalnum(arg_data *);
int  headers(void);
void help_print(char *help_string, int state, int newline);
int  landscape(void);
int  leftmargin(arg_data *);
int  nonopt(char *);
int  numbers(void);
void optusage(void);
int  output(arg_data *);
int  portrait(void);
int  startpage(arg_data *);
int  wid132(void);
int  wid80(void);
int  wid96(void);
int  width(arg_data *);

#define AF(foo) (int (*)(arg_data *))foo

arg_spec options[] =
{   {"?"        , 0                  , NULL, AF(optusage) , 0},
    {"80"       , 0                  , NULL, AF(wid80)    , 0},
    {"96"       , 0                  , NULL, AF(wid96)    , 0},
    {"132"      , 0                  , NULL, AF(wid132)   , 0},
    {"C"        , ASF_LSVAL          , NULL,    copyright , 0},
    {"COP"      , ASF_LSVAL          , NULL,    copyright , 0},
    {"COPYRIGHT", ASF_LSVAL          , NULL,    copyright , 0},
    {"D"        , ASF_VALREQ|ASF_NVAL, NULL,    downval   , 0},
    {"DOW"      , ASF_VALREQ|ASF_NVAL, NULL,    downval   , 0},
    {"DOWN"     , ASF_VALREQ|ASF_NVAL, NULL,    downval   , 0},
    {"G"        , ASF_VALREQ|ASF_NVAL, NULL,    globalnum , 0},
    {"GLO"      , ASF_VALREQ|ASF_NVAL, NULL,    globalnum , 0},
    {"GLOBAL"   , ASF_VALREQ|ASF_NVAL, NULL,    globalnum , 0},
    {"HEA"      , 0                  , NULL, AF(headers)  , 0},
    {"HEADER"   , 0                  , NULL, AF(headers)  , 0},
    {"H"        , 0                  , NULL, AF(optusage) , 0},
    {"HEL"      , 0                  , NULL, AF(optusage) , 0},
    {"HELP"     , 0                  , NULL, AF(optusage) , 0},
    {"L"        , 0                  , NULL, AF(landscape), 0},
    {"LAN"      , 0                  , NULL, AF(landscape), 0},
    {"LANDSCAPE", 0                  , NULL, AF(landscape), 0},
    {"N"        , 0                  , NULL, AF(numbers)  , 0},
    {"NUM"      , 0                  , NULL, AF(numbers)  , 0},
    {"NUMBER"   , 0                  , NULL, AF(numbers)  , 0},
    {"O"        , ASF_LSVAL          , NULL,    output    , 0},
    {"OUT"      , ASF_LSVAL          , NULL,    output    , 0},
    {"OUTPUT"   , ASF_LSVAL          , NULL,    output    , 0},
    {"P"        , 0                  , NULL, AF(portrait) , 0},
    {"POR"      , 0                  , NULL, AF(portrait) , 0},
    {"PORTRAIT" , 0                  , NULL, AF(portrait) , 0},
    {"W"        , ASF_VALREQ|ASF_NVAL, NULL,    width     , 0},
    {"WID"      , ASF_VALREQ|ASF_NVAL, NULL,    width     , 0},
    {"WIDTH"    , ASF_VALREQ|ASF_NVAL, NULL,    width     , 0},
    {"LMA"      , ASF_VALREQ|ASF_NVAL, NULL,    leftmargin, 0},
    {"LMARGIN"  , ASF_VALREQ|ASF_NVAL, NULL,    leftmargin, 0},
    {"STA"      , ASF_VALREQ|ASF_NVAL, NULL,    startpage , 0},
    {"START"    , ASF_VALREQ|ASF_NVAL, NULL,    startpage , 0},
    {"FIN"      , ASF_VALREQ|ASF_NVAL, NULL,    finishpage, 0},
    {"FINISH"   , ASF_VALREQ|ASF_NVAL, NULL,    finishpage, 0},
    {NULL       , 0                  , NULL,    NULL      , 0}
};

void main(argc, argv)
int   argc;
char *argv[];

{
    time_d    ctm;
    file_info info;
    char     *cpnt[2];
    char      chr;
    char      ftimestr[60];
    char      ctimestr[60];
    char      filestr[120];
    char      envbuf[256];
    char      crline[200];

    if(svcSysFindEnv(0, envname, NULL, envbuf, sizeof(envbuf), NULL) > 0)
    {
	cpnt[0] = envbuf;
	cpnt[1] = '\0';
	procarg(cpnt, 0, options, NULL, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    if (argc >= 2)
    {
	++argv;
	procarg(argv, 0, options, NULL, nonopt,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    if ((lastfile = firstfile) == NULL)
    {
        fputs("? LPRT: No input file specified\n", stderr);
        exit(1);
    }
    if (lsflag)				/* Landscape mode? */
    {					/* Yes */
        if (startpg != 0 && (startpg & 1) == 0) /* Make sure starting on */
            startpg--;				/*   odd page */
        if (finishpg != 0 && (finishpg & 1) != 0) /* Make sure finishing */
            finishpg++;				  /*   on even page */
    }

    if ((outfile = fopen(outname, "w")) == NULL) /* Open the output file */
        femsg(prgname, -errno, outname);
    if (gblflag)
    {
        if (pagenumg == 0)
        {
            if ((gblfile = fopen("LPRT.GPN", "r")) == NULL)
                femsg(prgname, -errno, "LPRT.GPN");
            fgets(filestr, 40, gblfile);
            pagenumg = atol(filestr);
            fclose(gblfile);
        }
        printf("Starting global page number = %ld\n", pagenumg);
    }
    do
    {
        if ((infile = fopen(lastfile->name, "r"))  == NULL)
            femsg(prgname, -errno, lastfile->name);
        if (firsttime)
        {
            firsttime = FALSE;
            if (lsflag)
            {
                if (fgetfinfo(infile, &info) < 0)
                    femsg(prgname, -errno, lastfile->name);
                sdt2str(ftimestr, "%H:%m %D-%3n-%y", (time_sz *)&info.create);
                svcSysDateTime(T_GTDDTTM, &ctm);
                ddt2str(ctimestr, "%H:%m %D-%3n-%y", (time_dz *)&ctm);
                sprintf(filestr, "%s   created: %s   listed: %s",
			lastfile->name, ftimestr, ctimestr);
                if (crstr[0] != '\0')
                    sprintf(crline, "/crs ( %d, %.100s  "
                            "All rights reserved.) def\n"
                            " /crp cf setfont crs stringwidth pop 2 div "
                            "1570 exch sub def\n", ctm.tmx_year, crstr);
                else
                    crline[0] = '\0';
                if (fprintf(outfile, lshdr, (crstr[0] != 0)?
                        " crp -190 moveto cf setfont (Copyright ) show"
                        " sf setfont (\\343) show\n cf setfont"
                        " crs show\n": "", filestr, widthv, crline) < 0)
                    femsg(prgname, -errno, outname);
            }
            else
            {
                if (fprintf(outfile, phdr, leftmar+100, leftmar, widthv) < 0)
                    femsg(prgname, -errno, outname);
                if (downamnt != 0)
                {
                    if (fprintf(outfile, "0 %d translate\n", -downamnt) < 0)
                        femsg(prgname, -errno, outname);
                }
            }
            pagesize = hdrflag? 54: 63;
            linecnt = 0;
            pagenuma = pagenumb = 1;
            linepnt = NULL;
        }
        while ((linepnt != NULL) || fgets(inbfr, 256, infile) != NULL)
        {
            if (hdrflag && linecnt == 0) /* Need page header? */
                fprintf(outfile, "3 s (%-*.*s Page %ld) l 2 s\n", /* Yes */
                        widthv-12, widthv-12, lastfile->name, pagenuma++);
            if (numflag)		/* Want line numbers? */
            {
                linenum++;		/* Yes - bump line count */
                sprintf(&outbfr[1], "%5ld   ", linenum);
                pos = 8;		/* Put number in buffer */
                dpnt = &outbfr[9];
            }
            else			/* If do not want line numbers */
            {
                pos = 0;
                dpnt = &outbfr[1];
            }
            spnt = (linepnt != NULL)? linepnt: inbfr;
            linepnt = NULL;
            do
            {   if ((chr = *spnt++) == '\0'  || chr == '\n')
                    break;
                if ((chr & 0x80) != 0 || chr == ('C'-0x40) ||
                        chr == ('D'-0x40)) /* Special character? */
                {			/* Yes */
                    *dpnt++ = '\\';
                    *dpnt++ = '0' + ((chr >> 6) & 03);
                    *dpnt++ = '0' + ((chr >> 3) & 07);
                    *dpnt++ = '0' + (chr & 07);
                    pos += 3;
                }
                else if (chr == '(' || chr == ')' || chr == '\\')
                {
                    *dpnt++ = '\\';
                    *dpnt++ = chr;
                    ++pos;
                }
                else if (chr == '\t')
                {
                    do
                    {   *dpnt++ = ' ';
                    } while ((++pos & 0x07) != 0);
                }
                else if (chr == '\f')	/* Form feed? */
                {
                    linecnt = 100;	/* Yes - force new page */
                    if (*spnt != '\0')	/* Have more on this line? */
                        linepnt = spnt;	/* Yes - use it */
                    break;
                }
                else			/* Not form feed */
                {
                    *dpnt++ = chr;	/* Store character in output file */
                    ++pos;		/* Bump position on line */
                }
            } while (pos < 255);	/* Continue if room for more in line */
            ++linecnt;			/* Count the line */
            if (pos != 0 || linecnt < pagesize) /* Output all lines except */
            {					/*   empty lines at the end */
                *dpnt = '\0';			/*   of a page */
                fprintf(outfile, "%s) l\n", outbfr);
            }
            if (linecnt >= pagesize)	/* At end of page now? */
            {
                linecnt = 0;		/* Yes - tell the printer */
                endpage();
            }
        }
        if ((errno || fclose(infile) < 0) && errno != -ER_EOF)
            femsg(prgname, -errno, lastfile->name);
    } while ((lastfile = lastfile->next) != NULL);
    if (linecnt != 0)			/* Have partial unprinted page at */
					/*   end? */
        endpage();			/* Yes - print last page */
    finish();
}

void finish(void)

{
    fprintf(outfile, "q\n\4");		/* End it all */
    if (fclose(outfile) < 0)
        femsg(prgname, -errno, outname);
    if (gblflag)
    {
        if ((gblfile = fopen("LPRT.GPN", "rw")) == NULL)
            femsg(prgname, -errno, "LPRT.GPN");
        if (fprintf(gblfile, "%ld\n", pagenumg) == EOF)
            femsg(prgname, -errno, "LPRT.GPN");
        fclose(gblfile);
    }
    exit(0);
}

/****************************************************************/
/* Function: endpage - Output end of page string to printer	*/
/* Returned: Nothing						*/
/****************************************************************/

void endpage(void)

{
    if (lsflag)
    {
        if (gblflag)
        {
            if (pagenumb & 1)
                fprintf(outfile, "(Page %ld  \\(%ld\\)) f\n", pagenumb,
                        pagenumg++);
            else
                fprintf(outfile, "(\\(%ld\\)  Page %ld) f\n", pagenumg++,
                        pagenumb);
        }
        else
            fprintf(outfile, "(Page %d) f\n", pagenumb);
    }
    else
        fprintf(outfile, "f\n");
    pagenumb++;
    if (pagenumb > finishpg)
    {
        if (fclose(infile) < 0)
            femsg(prgname, -errno, lastfile->name);
        finish();
    }
}

/*
 * Function called for non-option argument string
 */

int nonopt(
    char *arg)

{
    int    len;
    struct inname *pnt;

    strupr(arg);
    len = strlen(arg);
    pnt = (struct inname *)getspace(len+6);
    strcpy(pnt->name, arg);
    pnt->next = NULL;
    lastfile->next = pnt;
    lastfile = pnt;
    return (TRUE);
}

void optusage(void)

{
    fprintf(stderr, "\n%s version %d.%d (%s) %s\n\n", prgname,
			majedt, minedt, __DATE__, copymsg);

    fprintf(stderr, "%s {{%c}{%c}option{=val}{=val}} {file_specs}\n",
                 prgname, optchar1, optchar2);
    fprintf(stderr, "Options:\n");
    help_print(" 80        - Page width 80", widthv == 80, TRUE);
    help_print(" 96        - Page width 96", widthv == 96, TRUE);
    help_print(" 132       - Page width 132", widthv == 132, TRUE);
    help_print(" COPYRIGHT - Change copyright notice", FALSE, TRUE);
    help_print(" FINISH    - Specify final page", FALSE, TRUE);
    help_print(" GLOBAL    - Print global page number", gblflag, TRUE);
    help_print(" HEADER    - Printer page headers", hdrflag, TRUE);
    help_print(" HELP or ? - This message", FALSE, TRUE);
    help_print(" LANDSCAPE - Print landscape, 2 pages side by side", lsflag, TRUE);
    help_print(" LMARGIN   - Specify left margin (1/10 inches)", FALSE, TRUE);
    help_print(" NUMBER    - Print line numbers", numflag, TRUE);
    help_print(" OUTPUT    - Output file name (default = LZPRT:)", FALSE, TRUE);
    help_print(" PORTRAIT  - Print portrait, 1 page", !lsflag, TRUE);
    help_print(" START     - Specify starting page", FALSE, TRUE);
    help_print(" WIDTH     - Specify custom page width", FALSE, TRUE);
    fprintf(stderr, "A * shows this option is the current default.\n");
    fprintf(stderr, "All options and values may be abbreviated to 1 or 3 letters.\n");
    fprintf(stderr, "Set the environment variable %s to change any default"
           " option.\n", envname);
    exit(EXIT_INVSWT);          	/* Return as if invalid option */
}

/*
 * help_print
 * Print help option entries
 */
void help_print(char *help_string, int state, int newline)
{
    char str_buf[132];

    strcpy(str_buf, help_string);
    if (state)
        strcat(str_buf, " *");

    if (newline)
    {
        fprintf(stderr, "%s\n", str_buf);
    }
    else
        fprintf(stderr, "%-38s", str_buf);

}

int copyright(
    arg_data *arg)

{
    char *pnt1;
    char *pnt2;
    int   cnt;
    char  chr;

    pnt1 = arg->val.s;
    pnt2 = crstr;
    cnt = sizeof(crstr) - 2;
    do
    {
        if ((chr = *pnt1++) != '"')
        {
            if (chr == '_')
                chr = ' ';
            *pnt2++ = chr;
        }
    } while (chr != '\0' && --cnt > 0);
    *pnt2 = '\0';
    return (TRUE);
}

/*
 * Function for -80 switch
 */

int wid80(void)

{
    widthv = 80;
    return (TRUE);
}

/*
 * Function for -96 switch
 */

int wid96(void)

{
    widthv = 96;
    return (TRUE);
}

/*
 * Function for -132 switch
 */

int wid132(void)

{
    widthv = 132;
    return (TRUE);
}

/*
 * Function for -WIDTH switch
 */

int width(
    arg_data *arg)

{
    widthv = (int)arg->val.n;
    return (TRUE);
}

/*
 * Function for -START switch
 */

int startpage(
    arg_data *arg)

{
    startpg = (int)arg->val.n;
    return (TRUE);
}

/*
 * Function for -FINISH switch
 */

int finishpage(
    arg_data *arg)

{
    finishpg = (int)arg->val.n;
    return (TRUE);
}

/*
 * Function for -LMARGIN switch
 */

int leftmargin(
    arg_data *arg)

{
    leftmar = (int)arg->val.n * 30;
    return (TRUE);
}

/*
 * Function for -DOWN switch
 */

int downval(
    arg_data *arg)

{
    downamnt = (int)arg->val.n * 3;
    return (TRUE);
}

/*
 * Function for -GLOBAL switch
 */

int globalnum(
    arg_data *arg)

{
    pagenumg = arg->val.n;
    gblflag = TRUE;
    return (TRUE);
}

/*
 * Function for -L, -LAN, or -LANDSCAPE switches
 */

int landscape(void)

{
    lsflag = TRUE;
    return (TRUE);
}

/*
 * Function for -P, -POR, or -PORTRAIT switches
 */

int portrait(void)

{
    lsflag = FALSE;
    return (TRUE);
}

/*
 * Function for -N, -NUM, or -NUMBER switches
 */

int numbers(void)

{
    numflag = TRUE;
    return (TRUE);
}

/*
 * Function to process -O, -OUT, or -OUTPUT options
 */

int output(
    arg_data *arg)

{
    if (arg->length > 48)
    {
        fputs("LPRT: Output specification is too long\n", stderr);
        exit(1);
    }
    strmov(outname, arg->val.s);
    return (TRUE);
}

/*
 * Function for -H, -HEA, or -HEADER switches
 */

int headers(void)

{
    hdrflag = TRUE;
    return (TRUE);
}
