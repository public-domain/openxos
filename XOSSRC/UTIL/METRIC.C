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
#include <XCSTRING.H>
#include <ERRNO.H>
#include <XOS.H>
#include <XOSTRM.H>
#include <XOSSVC.H>
#include <XOSERMSG.H>

FILE *outfile;
char *outname;
long  prtr;
long  rtn;
int   cnt;
int   code;

char buffer[2000];
char prgname[] = "METRIC";
char head[] =
    "\003 \004 .24 .24 scale\n"
    "/%s findfont 1000 scalefont dup setfont\n"
    "/x 1 string def\n"
    "/d\n"
    "{   exch 50 string cvs print print\n"
    "} def\n"
    "/m\n"
    "{   dup ( ) d x exch 0 exch put\n"
    "    x stringwidth pop 100.0 mul 0.5 add cvi (\\n) d flush\n"
    "} def\n"
    "(\\n@@@\\n) print flush\n"
    "dup /FontName get (\\n) d\n"
    "dup /f exch /FontMatrix get 3 get 100 mul def\n"
    "dup /FontInfo get\n"
    "dup /UnderlinePosition known\n"
    "{   dup /UnderlinePosition get f mul 0.5 add cvi ( ) d\n"
    "}\n"
    "{   (999999\\n) print\n"
    "} ifelse\n"
    "dup /UnderlineThickness known\n"
    "{   /UnderlineThickness get f mul 0.5 add cvi (\\n) d\n"
    "}\n"
    "{   pop (999999\\n) print\n"
    "} ifelse flush\n";

char cmd[] =
    "%d m\n";
char badcmd[] =
    "? METRIC: Command error, usage is:\n"
    "            METRIC {prtr} font file\n"
    "          Where:\n"
    "            prtr = Printer device (default is COM2:)\n"
    "            font = Postscript name for font\n"
    "            file = Output file specification\n";

main (argc, argv)
int   argc;
char *argv[];

{
    if (argc < 3 || argc > 4)
    {
        fputs(badcmd, stderr);
        exit(1);
    }
    if (argc == 3)
        argv[0] = "COM2:";
    else
        argv++;
    strupper(argv[0]);
    strupper(argv[2]);

    if ((prtr = svcIoOpen(O_IN|O_OUT, argv[0], NULL)) < 0)
        femsg(prgname, prtr, argv[0]);
    svcTrmFunction(prtr, 2);		/* Turn of echoing */
    svcTrmFunction(prtr, 3);		/* Clear input buffer */
    sprintf(buffer, head, argv[1]);	/* Put font name into prolog */
    svcIoOutString(prtr, buffer, 0);	/* Send prolog to printer */
    for (;;)				/* Discard anything it sends back! */
    {
        if ((cnt = svcIoInBlock(prtr, buffer, 100)) < 0)
            femsg(prgname, cnt, "TRM2:");
        buffer[cnt] = 0;
        if (cnt == 5 && strcmp(buffer, "@@@\r\n") == 0)
            break;
    }
    getresp();				/* Get font name back from printer */
    if (strcmp(buffer, argv[1]) != 0)	/* Is it the one we want? */
    {					/* No - complain and fail */
        fprintf(stderr, "? METRIC: Requested font %s not available\n", argv[1]);
        exit(1);
    }
    outname = getspace(strlen(argv[2]+6));
    strmov(strmov(outname, argv[2]), ".FM");
    if ((outfile = fopen(outname, "w")) == NULL) /* OK - open the output file */
        femsg(prgname, -errno, outname);
    fprintf(outfile, "%s\n", argv[1]);	/* Put font name on first line */
    getresp();				/* Get underline position and width */
    fprintf(outfile, "%s\n", buffer);	/* Output it */
    for (code = 0; code < 256; ++ code)	/* Request width of each character */
    {
        sprintf(buffer, cmd, code);	/* Construct command */
        if ((rtn = svcIoOutString(prtr, buffer, 0)) < 0) /* Send it */
            femsg(prgname, rtn, "TRM2:");
        getresp();
        putchar('.');
        fprintf(outfile, "%s\n", buffer); /* Output the string */
    }
    if ((rtn = svcIoOutString(prtr, "\4", 0)) < 0) /* Send it */
        femsg(prgname, rtn, "TRM2:");
    svcIoClose(0, prtr);
    putchar('\n');
    fclose(outfile);
}

/*
 * Function to get response and output it to the metric file
 */

getresp()

{
    register char *pnt;

    if ((cnt = svcIoInBlock(prtr, buffer, 100)) < 0) /* Get response */
        femsg(prgname, cnt, "TRM2:");
    pnt = buffer;			/* Remove control characters */
    while (*pnt != '\r' && *pnt != '\n')
        pnt++;
    *pnt = '\0';
}
