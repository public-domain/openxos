//****************************************
// Routines to generate load map for XLINK
//****************************************
// Written by John Goltz
//****************************************

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <TIME.H>
#include <CTYPE.H>
#include <ERRNO.H>
#include <PROCARG.H>
#include <XOS.H>
#include <XCMALLOC.H>
#include <XOSTIME.H>
#include <XOSSVC.H>
#include <XOSERRMSG.H>
#include "XLINK.H"
#include "XLINKEXT.H"

int    fisctrm(FILE *stream);

extern struct sy *sympnt;

// Allocate local data

struct sy dmysym;
int    pagenum;			// Map page number
int    linecnt;			// Line counter
char   hvdnmpo;			// TRUE if any map output has been done
char   timebfr[32];		// Buffer for date and time string

// Table of msect type names 

char *typename[] = {"None", "Data", "Code", "Stack", "Combined"};

// Function to output main part of map if it is wanted

void mapout(void)

{
    time_d timeloc;
    struct sd *sgd;
    char  *str;

    svcSysDateTime(2, &timeloc);	// Get current data and time
    ddt2str(timebfr, "%H:%m %W %D-%3n-%y", (time_dz *)&timeloc);
					// Change to string
    if (strcmp(mapfsp, "#") == 0)
        mapfp = stdout;
    else
    {
        if (mapfsp == NULL)
            mapfsp = usefif(mapext);
        if ((mapfp = fopen(mapfsp, "w")) == NULL)
            femsg(prgname, -errno, mapfsp);
    }
    mapflag = !fisctrm(mapfp);		// Remember if map is to console
    pagenum = 0;
    linecnt = 0;
    mapnline();
    fputs(">>> File header data", mapfp);
    mapnline();
    mapnline();
    fputs("    RUN file version:  1", mapfp);
    mapnline();
    switch (ptype)
    {
     case P_LOW:
        str = "Unspecified, low order first";
        break;

     case P_Z80:
        str = "Z80";
        break;

     case P_8086:
        str = "8086";
        break;

     case P_80186:
        str = "80186";
        break;

     case P_80286:
        str = "80286";
        break;

     case P_80386:
        str = "80386";
        break;

     case P_HIGH:
        str = "Unspecified, high order first";
        break;

     case P_68000:
        str = "68000";
        break;

     default:
        str = "??????";
        break;
    }
    fprintf(mapfp, "    Processor type: (0x%2.2X) %s", ptype, str);
    mapnline();
    switch (itype)
    {
     case I_ALONE:
        str = "Stand-alone";
        break;

     case I_XOSUSER:
        str = "XOS user";
        break;

     case I_MSDOS:
        str = "MSDOS";
        break;

     case I_XOS8086:
        str = "XOS 8086 mode";
        break;

     case I_XOSLKE:
        str = "XOS Loadable Kernel Extension";
        break;

     default:
        str = "??????";
        break;
    }
    fprintf(mapfp, "    Image type:     (0x%2.2X) %s", itype, str);
    mapnline();
    if (strpsd)
        fprintf(mapfp, "    Start address:   0x%8.8lX%c (Msect: %s)",
                stradr, (strpsd->pd_msd->md_flag&MA_ADDR)? ' ': '*',
                strpsd->pd_msd->md_sym->sy_name+1);
    else
        fputs("    No start address", mapfp);
    mapnline();
    if (dbgpsd)
        fprintf(mapfp, "    Debug address:   0x%8.8lX%c (Msect: %s)",
                dbgadr, (dbgpsd->pd_msd->md_flag&MA_ADDR)? ' ': '*',
                dbgpsd->pd_msd->md_sym->sy_name+1);
    else
        fputs("    No debug address", mapfp);
    mapnline();
    if (stkpsd)
        fprintf(mapfp, "    Stack address:   0x%8.8lX%c (Msect: %s)",
                stkadr, (stkpsd->pd_msd->md_flag&MA_ADDR)? ' ': '*',
                stkpsd->pd_msd->md_sym->sy_name+1);
    else
        fputs("    No stack address", mapfp);
    mapnline();
    mapnline();
    if (abspsd.pd_slh == NULL)		// Do we have any absolute symbols?
    {
        fputs(">>> No absolute symbols defined", mapfp); // No
		mapnline();
    }
    else
    {
		maptline(5);			// Require 5 lines on this page
		fputs(">>> Absolute symbols", mapfp); // Yes - list them
		mapnline();
		mapnline();
		map1psect(&abspsd, ' ');
    }
    mapnline();
    if ((sgd=sgdhead) != NULL)		// Point to first segment
    {
        do
        {   map1seg(sgd);
        } while ((sgd=sgd->sd_next) != NULL);
    }
}

// Function to generate map for a segment

void map1seg(
    struct sd *sgd)

{
    struct md *msd;

    maptline(10);			// Require 10 lines here
    fprintf(mapfp, ">>> Segment #%d, name: %s", sgd->sd_num,
            sgd->sd_sym->sy_name+1);
    mapnline();
    if (sgd->sd_linked == NULL)
        fputs("    Linked segment: None", mapfp);
    else
        fprintf(mapfp, "    Linked segment: #%d (%s)", sgd->sd_linked->sd_num,
                sgd->sd_linked ->sd_sym->sy_name+1);
    mapnline();
    fprintf(mapfp, "    Segment type: %s", typename[sgd->sd_type]);
    mapnline();
    fprintf(mapfp, "    Privilege level: %d", sgd->sd_priv);
    mapnline();
    fputs("    Attributes:", mapfp);
    if (sgd->sd_flag &
            (SA_LARGE|SA_32BIT|SA_CONF|SA_WRITE|SA_READ|SA_EXTEND|SA_SHARE))
    {
        if (sgd->sd_flag & SA_LARGE)
            fputs(" Large", mapfp);
        if (sgd->sd_flag & SA_32BIT)
            fputs(" 32-bit", mapfp);
        if (sgd->sd_flag & SA_CONF)
            fputs(" Conformable", mapfp);
        if (sgd->sd_flag & SA_WRITE)
            fputs(" Writable", mapfp);
        if (sgd->sd_flag & SA_READ)
            fputs(" Readable", mapfp);
        if (sgd->sd_flag & SA_EXTEND)
            fputs(" Extendable", mapfp);
        if (sgd->sd_flag & SA_SHARE)
            fputs(" Sharable", mapfp);
    }
    else
        fputs(" None", mapfp);
    mapnline();
    if (sgd->sd_select == 0)
        fputs("    No selector specified", mapfp);
    else
        fprintf(mapfp, "    Segment selector: %4.4X", (ushort)(sgd->sd_select));
    mapnline();
    if (sgd->sd_addr == 0xFFFFFFFFL)
        fputs("    No address or modulus specified", mapfp);
    else
        fprintf(mapfp, "    %s: %8.8lX", (sgd->sd_flag & SA_ADDR)?
                "Address": "Modulus", sgd->sd_addr);
    mapnline();
    mapnline();
    if ((msd = sgd->sd_head) != NULL) // Point to first msect
    {
        do
        {   map1msect(msd);
        } while ((msd = msd->md_next) != NULL);
    }
}

// Function to generate map for 1 msect

void map1msect(
    struct md *msd)

{
    struct pd *psd;

    maptline(10);			// Require 10 lines here
    fprintf(mapfp, ">>> Msect #%d, name: %s", msd->md_num,
            msd->md_sym->sy_name+1);
    mapnline();
    fprintf(mapfp, "    Loaded size: %8.8lX (%ld.)", msd->md_lsize,
            msd->md_lsize);
    mapnline();
    fprintf(mapfp, "    Total size:  %8.8lX (%ld.)", msd->md_tsize,
            msd->md_tsize);
    mapnline();
    fprintf(mapfp, "    Msect type: %s", typename[msd->md_type]);
    mapnline();
    fprintf(mapfp, "    Privilege level: %d", msd->md_priv);
    mapnline();
    fputs("    Attributes:", mapfp);
    if (msd->md_flag & (MA_CONF|MA_WRITE|MA_READ|MA_SHARE))
    {
        if (msd->md_flag & MA_CONF)
            fputs(" Conformable", mapfp);
        if (msd->md_flag & MA_WRITE)
            fputs(" Writable", mapfp);
        if (msd->md_flag & MA_READ)
            fputs(" Readable", mapfp);
        if (msd->md_flag & MA_SHARE)
            fputs(" Sharable", mapfp);
    }
    else
        fputs(" None", mapfp);
    mapnline();
    if (!(msd->md_flag & MA_ADDR) && msd->md_mod == 0xFFFFFFFFL)
        fputs("    No address or modulus specified", mapfp);
    else
        fprintf(mapfp, "    %s specified: %8.8lX",
                (msd->md_flag & MA_ADDR)? "Address": "Modulus",
                (msd->md_flag & MA_ADDR)? msd->md_addr: msd->md_mod);
    mapnline();
    if (msd->md_addr != 0xFFFFFFFFL)
        fprintf(mapfp, "    Address used: %8.8lX", msd->md_addr);
    else
        fputs("    No address used", mapfp);
    mapnline();
    mapnline();
    if ((psd = msd->md_head) != NULL)	// Point to first psect
    {
        do
        {   maptline(7);				// Require 7 lines here
            fprintf(mapfp, ">>> Psect name: %s", psd->pd_sym->sy_name+1);
            mapnline();
            fprintf(mapfp, "    Loaded size: %l8.8lX (%ld.)", psd->pd_lsize,
                    psd->pd_lsize);
            mapnline();
            fprintf(mapfp, "    Total size:  %l8.8lX (%ld.)", psd->pd_tsize,
                    psd->pd_tsize);
            mapnline();
            fprintf(mapfp, "    Attributes: %s",
                        (psd->pd_flag & PA_OVER)? "Overlayed": "Concatenated");
            mapnline();
            if (psd->pd_addr == 0xFFFFFFFFL && psd->pd_mod == 0xFFFFFFFFL)
                fputs("    No address or modulus specified", mapfp);
            else
                fprintf(mapfp, "    %s specified: %8.8lX",
                        (psd->pd_addr != 0xFFFFFFFFL)? "Address":"Modulus",
                        (psd->pd_addr != 0xFFFFFFFFL)?
                        psd->pd_addr:psd->pd_mod);
            mapnline();
            fprintf(mapfp, "    Address used: %8.8lX", psd->pd_reloc);
            mapnline();
            if (psd->pd_slh == NULL)	// Any symbols defined?
            {
                fputs("    No symbols defined", mapfp); // No
                mapnline();
            }
            else
            {				// Yes - list them
                mapnline();
                map1psect(psd, (msd->md_flag & MA_ADDR)? ' ': '*');
            }
            mapnline();
        } while ((psd = psd->pd_next) != NULL);
    }
}

int cmpsym(
    struct sy *a,
    struct sy *b,
    struct pd *psd);


// Function to output map data for a single psect

void map1psect(
    struct pd *psd,
    char   relflag)

{
    struct sy *thissym;
    char  *symfmt1a;
    char  *symfmt1b;
    char  *symfmt2a;
    char  *symfmt2b;
    char  *symfmt3a;
    char  *symfmt3b;
    int    symcnt;		// Symbol counter
    int    symmax;
    int    select;

    if (psd->pd_ssmax > 23+1)
    {
        fputs("      Value     Symbol", mapfp);
        symmax = 0;
        symfmt1b = "      %8.8lX%c %s";
        symfmt2b = " %4.4X:%8.8lX%c %s";
        symfmt3b = " ---- %8.8lX%c %s";
    }
    else if (psd->pd_ssmax > 10+1)
    {
        fputs("      Value     Symbol                       Value"
              "     Symbol", mapfp);
        symmax = 1;
        symfmt1a = "      %8.8lX%c %-23s";
        symfmt1b = "      %8.8lX%c %s";
        symfmt2a = " %4.4X:%8.8lX%c %-23s";
        symfmt2b = " %4.4X:%8.8lX%c %s";
        symfmt3a = " ---- %8.8lX%c %-23s";
        symfmt3b = " ---- %8.8lX%c %s";
    }
    else
    {
        fputs("      Value     Symbol          Value     Symbol"
              "          Value     Symbol", mapfp);
        symmax = 2;
        symfmt1a = "      %8.8lX%c %-10s";
        symfmt1b = "      %8.8lX%c %s";
        symfmt2a = " %4.4X:%8.8lX%c %-10s";
        symfmt2b = " %4.4X:%8.8lX%c %s";
        symfmt3a = " ---- %8.8lX%c %-10s";
        symfmt3b = " ---- %8.8lX%c %s";
    }
    symcnt = 0;
    if ((thissym = psd->pd_slh) != NULL)
    {
        thissym = heapsort(thissym, cmpsym, psd); // Sort symbols for the psect
        do
        {   if (--symcnt < 0)			// Room for more on this line?
            {
                symcnt = symmax;		// No - start new line
                mapnline();
            }
            if (thissym->sy_flag & SYF_ADDR)
            {
		if ((thissym->sy_flag & SYF_ABS) ||
			(select=thissym->sy_psd->pd_msd->md_sgd->sd_select) != 0)
		    fprintf(mapfp, (symcnt)? symfmt2a: symfmt2b,
			    (thissym->sy_flag & SYF_ABS) ?
			    ((ushort)(thissym->sy_sel)) : ((ushort)select),
			    thissym->sy_val.v, relflag, thissym->sy_name+1);
		else
		    fprintf(mapfp, (symcnt)? symfmt3a: symfmt3b,
			    thissym->sy_val.v, relflag, thissym->sy_name+1);
            }
            else
                fprintf(mapfp, (symcnt)? symfmt1a: symfmt1b,
                        thissym->sy_val.v, relflag, thissym->sy_name+1);
        } while ((thissym = thissym->sy_next) != NULL);
    }
    mapnline();			// End the last line
}

// Function to compare symbols for heapsort

int cmpsym(
    struct sy *a,
    struct sy *b,
    struct pd *psd)

{
    if (psd == &abspsd)
    {
	if (a->sy_sel != b->sy_sel)
	    return ((((ushort)(a->sy_sel)) > ((ushort)(b->sy_sel))) ? 1 : -1);
    }
    return (a->sy_val.v - b->sy_val.v);
}

// Funtion to output string to console and to map (string must NOT contain \n)

void mapconstr(str)
char *str;

{
    fputs(str, stderr);
    if (mapflag)
        fputs(str, mapfp);
}

// Function to start new line on console and for map

void xeline(void)

{
    putc('\n', stderr);
    if (mapflag)
	mapnline();
}

void xnline(void)

{
    putc('\n', stdout);
    if (mapflag)
	mapnline();
}

// Function to start new line for map

void mapnline(void)

{
    if (--linecnt >= 0)			// Need new page?
	putc('\n', mapfp);		// No - just start new line
    else
	mapnpage();
}

// Function to ensure that have n lines on current page

void maptline(lines)
int lines;

{
    if (linecnt < lines)		// Enough left?
	mapnpage();			// No - start new page
}

// Function to start new page for map

void mapnpage(void)

{
    if (hvdnmpo)			// Is this the first time?
	fputs("\n\f", mapfp);		// No - end previous page
    else
	hvdnmpo = TRUE;
    linecnt = MAPPGSZ;			// Yes - reset count
    fprintf(mapfp, "Load map    %s    XLINK - version %d.%-4d",
	    timebfr, version, editnum);
    fprintf(mapfp, "           Page %d\n\n\n", ++pagenum);
}

// Function to list undefined or multiply defined globals

void listgbl(
    int   cnt,			// Number to list
    char *msg,			// Header message
    int   stst)			// Bit to test in sy_flag

{
    char   s;
    int    symnum;
    int    symmax;
    int    symcnt;
    int    tblcnt;
    struct sy  *sym;
    struct sy **tblpnt;
    char  *cpnt;

    static char header[] = "? %d. %sdefined global symbol%c";

    if (cnt == 0)						// Anything to do here?
		return;							// No
    s = (cnt == 1)? '\0':'s';
    fprintf(stderr, header, cnt, msg, s); // Output header to terminal
    if (mapflag)						// And to map if should
		fprintf(mapfp, header, cnt, msg, s);
    symmax = 0;
    tblcnt = HASHSIZE;
    tblpnt = hashtbl;
    while (--tblcnt >= 0)				// Scan the hash table to find
    {									//   longest symbol to print
		if ((sym=*tblpnt++) != NULL)
		{
			do
			{
				if (sym->sy_flag & stst) // Should we print this symbol?
				{
					cpnt = sym->sy_name+1; // Yes - see how long it is
					symcnt = 1;
					while (*cpnt++ > 0)
						symcnt++;
					if (symmax < symcnt)
						symmax = symcnt;
				}
			} while ((sym=sym->sy_hash) != NULL);
		}
    }
    symnum = 79/(symmax+2) - 1;			// Number of symbols per line - 1
    symcnt = 0;
    tblcnt = HASHSIZE;
    tblpnt = hashtbl;
    while (--tblcnt >= 0)				// Scan the hash table
    {
		if ((sym=*tblpnt++) != NULL)
			do
			{   if (sym->sy_flag & stst) // Should we print this symbol?
				{
					if (--symcnt < 0)	// Need new line?
					{
						symcnt = symnum; // Yes
						xeline();
					}
                    cpnt = sym->sy_name+1; // Output symbol name
                    fprintf(stderr, "  %-*s", symmax, cpnt);
					if (mapflag)
                        fprintf(mapfp, "  %-*s", symmax, cpnt);
			}
	    } while ((sym=sym->sy_hash) != NULL);
    }
    xeline();
    if (mapfsp)
        mapnline();
}

// Function to output illegal relocatable expression message

void badrexp(void)

{
    mapconstr("Illegal relocatable expression");
    showfile(2);
    showplace(NULL);
}

// Function to output expression stack underflow message

void underflow(void)

{
    mapconstr("Expression stack underflow");
    showfile(2);
}

// Function to output expression stack overflow message

void overflow(void)

{
    mapconstr("Expression stack overflow");
    showfile(2);
}

// Function to output warning message if address or modulus specified
//   for Z80 or 8086 type processor

void warnaddr(msd)
struct md *msd;

{

    static char fmt[] = "Address or modulus specified for msect %s for";
    char  *name;

    name = msd->md_sym->sy_name+1;
    fprintf(stderr, fmt, name);
    if (mapflag)
        fprintf(mapfp, fmt, name);
    xeline();
    mapconstr("  processor which does not allow this");
    showfile(2);
}

// Function to output message to complain if absolute address is specified
//   as a starting, debug, or stack address

void absaddr(msg)
char *msg;

{
    static char fmt[] = "Absolute %s address specified";

    fprintf(stderr, fmt, msg);
    if (mapflag)
        fprintf(mapfp, fmt, msg);
    showfile(2);
}

// Function to output message to complain if multpile values are specified
//   as a starting, debug, or stack address

void multaddr(msg)
char *msg;

{
    static char fmt[] = "Multiple %s addresses specified";

    fprintf(stderr, fmt, msg);
    if (mapflag)
        fprintf(mapfp, fmt, msg);
    showfile(2);
}

// Function to output message to complain if no segment associated with
//   global symbol

void nosymseg(void)

{
    static char fmt[] = "No segment associated with symbol %s";
    char  *name;

    name = sympnt->sy_name+1;
    fprintf(stderr, fmt, name);
    if (mapflag)
        fprintf(mapfp, fmt, name);
    showfile(2);
}

// Function to output message to complain about missing or bad symbol
//   table psect

void nosyms(msg)
char *msg;

{
    static char fmt[] = "Psect _symbols_p not %s, no symbol table loaded";

    fprintf(stderr, fmt, msg);
    if (mapflag)
	fprintf(mapfp, fmt, msg);
    xeline();
    stbpsd = NULL;
    opsymsz = 0;
}

// Function to output truncation warning message

void truncate(
    int size)

{
    static char fmt[] = "Value truncated to %d. bits";

    fprintf(stderr, fmt, size);
    if (mapflag)
	fprintf(mapfp, fmt, size);
    showfile(2);
    showplace(NULL);
}

// Function to output message for relative reference to different address
//   space

void diffspace(sym)
struct sy *sym;

{
    mapconstr("Relative reference to different segment");
    showfile(2);
    showplace(sym);
}

// Function to report where in image an error occured

void showplace(sym)
struct sy *sym;

{
    long   place;
    char   reloc;
    static char fmt1[] = "symbol %s ";
    static char fmt2[] = "in psect %s at address %8.8lX%c";

    mapconstr("  ");
    if (sym != NULL)
    {
        fprintf(stderr, fmt1, sym->sy_name+1);
        if (mapflag)
            fprintf(mapfp, fmt1, sym->sy_name+1);
    }
    place = curpsd->pd_reloc + offset - curpsd->pd_offset;
    reloc = (curmsd->md_flag & MA_ADDR)? ' ': '*';
    fprintf(stderr, fmt2, curpsd->pd_sym->sy_name+1, place, reloc);
    if (mapflag)
	fprintf(mapfp, fmt2, curpsd->pd_sym->sy_name+1, place, reloc);
    xeline();
}

// Function to output warning message for multiply defined symbols

void multdef(
    struct sy *sym,
    long   value,
    int    psn)

{
    char  *msg;
    struct pd *psd;
    char   msgx[50];

    static char fmt1[] = "  value = %8.8lX (%s)";

    varwmsg(sym, "Multiply defined", "defined"); // Output the first line
    if (psn == 0)			// If absolute value
	msg = "absolute";
    else 				// If for psect
    {
	psd = getpsd(psn);
        sprintf(msgx, "psect %s", psd->pd_sym->sy_name+1);
        msg = msgx;
    }
    fprintf(stderr, fmt1, value, msg);
    if (mapflag)
	    fprintf(mapfp, fmt1, value, msg);
    xeline();
}

// Function to output warning message for undefined or multiply defined symbols

void varwmsg(sym, msg1, msg2)
struct sy *sym;			// Pointer to symbol table entry
char  *msg1;
char  *msg2;

{
    static char fmt[] = "%s symbol %s %s";

    fprintf(stderr, fmt, msg1, sym->sy_name+1, msg2);
    if (mapflag)
	fprintf(mapfp, fmt, msg1, sym->sy_name+1, msg2);
    showfile(2);
}

// Function to report attribute conflict

void conflatr(atrib, blk, name)
char  *atrib;
char  *blk;
struct sy *name;

{
    static char fmt[] = "Conflicting attribute \"%s\" for %s %s";

    fprintf(stderr, fmt, atrib, blk, name->sy_name+1);
    if (mapflag)
	fprintf(mapfp, fmt, atrib, blk, name->sy_name+1);
    showfile(2);
}

// Function to report msect/segment conflict

void conflblk(blk1, name1, blk2, name2)
char  *blk1;
struct sy *name1;
char  *blk2;
struct sy *name2;

{
    static char fmt[] = "%s %s defined in conflicting %s %s";

    fprintf(stderr, fmt, blk1, name1->sy_name+1, blk2, name2->sy_name+1);
    if (mapflag)
	fprintf(mapfp, fmt, blk1, name1->sy_name+1, blk2, name2->sy_name+1);
    showfile(2);
}

// Function to report conflicting attribute

void conflitm(blk, thing, name)
char  *blk;
char  *thing;
struct sy *name;

{
    static char fmt[] = "Conflicting %s specified for %s %s";

    fprintf(stderr, fmt, thing, blk, name->sy_name+1);
    if (mapflag)
        fprintf(stderr, fmt, thing, blk, name->sy_name+1);
    showfile(2);
}

// Function to report address/modulus conflict

void confladdr(blk, name)
char  *blk;
struct sy *name;

{
    static char fmt[] = "Conflicting address or modulus for %s %s";

    fprintf(stderr, fmt, blk, name->sy_name+1);
    if (mapflag)
	fprintf(mapfp, fmt, blk, name->sy_name+1);
    showfile(2);
}

// Function to report conflict in segment selector value

void conflsel(name)
struct sy *name;

{
    static char fmt[] = "Conflicting selector value for segment %s";

    fprintf(stderr, fmt, name->sy_name+1);
    if (mapflag)
	fprintf(mapfp, fmt, name->sy_name+1);
    showfile(2);
}

// Function to report conflict in segment linking

void confllink(
    struct sy *name)

{
    static char fmt[] = "Conflicting segment linkage for segment %s";

    fprintf(stderr, fmt, name->sy_name+1);
    if (mapflag)
	fprintf(mapfp, fmt, name->sy_name+1);
    showfile(2);
}

// Function to report conflict in assigning an MS segment to an MS group

void conflmsseg(
    struct sy *name)

{
    static char fmt[]= "MS segment %s is part of more than one MS group";

    fprintf(stderr, fmt, name->sy_name+1);
    if (mapflag)
	fprintf(mapfp, fmt, name->sy_name+1);
    showfile(2);
}

// Function to output file and module names at end of error message

void showfile(
    int spaces)

{
    static char fmt[] = "%*sin file %s[%s]";

    errorflg = TRUE;			// Indicate fatal error
    xeline();
    fprintf(stderr, fmt, spaces, "", curobj->obj_fsp, modname);
    if (mapflag)
        fprintf(mapfp, fmt, spaces, "", curobj->obj_fsp, modname);
    xeline();
}

/************************************************/
/* Function: dosummary - Produce summary output	*/
/* Returned: Nothing				*/
/************************************************/

void dosummary(void)

{
    int    temp;
    int    hcnt;
    int    ucnt;
    int    num;
    int    len;
    int    max;
    struct sy  *sym;
    struct sy **hpnt;

    static char fmthash[] = "Hash table usage: %d/%d; "
            "string length: ave=%d.%03.3d, max=%d";
    static char fmtstbl[] = "Symbol table: pass one: %d, %d;"
            " pass two: %d, %d";
    static char fmtsfile[] = "Symbol file: %d, %d";
    static char fmtseg[]   = "Segments:  ref: %-6d def: %d";
    static char fmtmsect[] = "Msects:    ref: %-6d def: %d";
    static char fmtpsect[] = "Psects:    ref: %-6d def: %d";
    static char fmtsym[]   = "Symbols:   ref: %-6d def: %d";
    static char fmtnames[] = "MS names:  ref: %d";

    if (mapflag)
    {
        fputs(">>> Summary", mapfp);
        mapnline();
        mapnline();
    }
    hpnt = hashtbl;			// Determine hash table usage
    hcnt = HASHSIZE;
    ucnt = 0;
    temp = 0;
    max = 0;
    do
    {   if ((sym=*hpnt++) != NULL)
	{
	    ++ucnt;
            len = 0;
	    do
            {   ++temp;
                ++len;
	    } while ((sym=sym->sy_hash) != NULL);
            if (len > max)
                max = len;
	}
    } while (--hcnt > 0);
    temp  = (temp * 1000) / ucnt;
    hcnt = temp % 1000;
    temp /= 1000;
    printf(fmthash, ucnt, HASHSIZE, temp, hcnt, max);
    if (mapflag)
        fprintf(mapfp, fmthash, ucnt, HASHSIZE, temp, hcnt, max);
    xnline();

    if (opsymsz)
    {
        temp = (stboffset - stbpsd->pd_offset);
        printf(fmtstbl, stbnum, stbsize, stbtotal, temp);
        if (mapflag)
            fprintf(mapfp, fmtstbl, stbnum, stbsize, stbtotal, temp);
        xnline();
    }

    if (symdd)
    {
        printf(fmtsfile, symtotal, symlength);
        if (mapflag)
            fprintf(mapfp, fmtsfile, symtotal, symlength);
        xnline();
    }

    if (segnummem.mab_mempnt != SEGNUMBASE)
    {
        
        num = (int)(segnummem.mab_mempnt - SEGNUMBASE)/4;
        printf(fmtseg, num, seggbn);
        if (mapflag)
            fprintf(mapfp, fmtseg, num, seggbn);
        xnline();
    }

    if (msectnummem.mab_mempnt != MSECTNUMBASE)
    {
        num = (int)(msectnummem.mab_mempnt - MSECTNUMBASE)/4;
        printf(fmtmsect, num, mscgbn);
        if (mapflag)
            fprintf(mapfp, fmtmsect, num, mscgbn);
        xnline();
    }

    if (psectnummem.mab_mempnt != PSECTNUMBASE)
    {
        num = (int)(psectnummem.mab_mempnt - PSECTNUMBASE)/4;
        printf(fmtpsect, num, pscgbn);
        if (mapflag)
            fprintf(mapfp, fmtmsect, num, pscgbn);
        xnline();
    }

    if (symnummem.mab_mempnt != SYMNUMBASE)
    {
        num = (int)(symnummem.mab_mempnt - SYMNUMBASE)/4;
        printf(fmtsym, num, symtblsize);
        if (mapflag)
            fprintf(mapfp, fmtsym, num, symtblsize);
        xnline();
    }

    if (lnamesmem.mab_mempnt != LNAMESBASE)
    {
        num = (int)(lnamesmem.mab_mempnt - LNAMESBASE)/4;
        printf(fmtnames, num);
        if (mapflag)
            fprintf(mapfp, fmtnames, num);
        xnline();
    }
}
