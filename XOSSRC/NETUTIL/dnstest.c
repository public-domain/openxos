#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <TIME.H>
#include <PROCARG.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSTIME.H>
#include <XOSERRMSG.H>
#include <XOSNET.H>

#define VERSION 1
#define EDITNO  0

long  dev;
long  rtn;
char  netname[16] = "IPS0:";
char  prgname[] = "DNSTEST";
char  phyname[32];

int   dnamelen;
uchar dname[260];
char  ipaddr[20];

struct
{   text4_parm  class;
    byte4_parm  filoptn;
    lngstr_parm filspec;
    char        end;
} spcparms =
{   {PAR_SET|REP_TEXT, 4, IOPAR_CLASS  , "IPS"},
    {PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, FO_NOPREFIX|FO_XOSNAME},
    {PAR_GET|REP_STR , 0, IOPAR_FILSPEC, phyname, 0, sizeof(phyname), 0}
};

type_qab spcqab =
{   QFNC_WAIT|QFNC_SPECIAL,	// qab_open
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    0,				// qab_handle
    0,				// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    0,				// qab_option
    sizeof(dname),		// qab_count
    dname, 0,			// qab_buffer1
    NULL, 0,			// qab_buffer2
    &spcparms, 0		// qab_parm
};

int  deflt = IPSSF_FINDIPA;

int  havename(char *);
void help(void);
int  afunc(void);
int  cnamefunc(void);
int  ptrfunc(void);
int  mxfunc(void);

#define AF(func) (int (*)(arg_data *))func

arg_spec options[] =
{   {"A"    , 0, NULL, AF(afunc)     , 0},
    {"CNAME", 0, NULL, AF(cnamefunc) , 0},
    {"C"    , 0, NULL, AF(cnamefunc) , 0},
    {"MX"   , 0, NULL, AF(mxfunc)    , 0},
    {"M"    , 0, NULL, AF(mxfunc)    , 0},
    {"PTR"  , 0, NULL, AF(ptrfunc)   , 0},
    {"P"    , 0, NULL, AF(ptrfunc)   , 0},
    {"?"    , 0, NULL, AF(help)      , 0},
    {"HELP" , 0, NULL, AF(help)      , 0},
    {"H"    , 0, NULL, AF(help)      , 0},
    {NULL   , 0, NULL, AF(NULL)      , 0}
};


main(argc, argv)
int   argc;
char *argv[];

{
    uchar *pnt;
    char  *num1;
    char  *num2;
    char  *num3;
    char  *num4;
    int    cnt;
    char   chr;

    if (--argc > 0)
    {
	argv++;
	procarg(argv, 0, options, NULL, havename,
		(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    if (dname[1] == 0)
    {
	fputs("? DNSTEST: No domain name specified\n", stderr);
	exit(1);
    }
    if ((spcqab.qab_handle = svcIoOpen(0, netname, NULL)) < 0)
	femsg2(prgname, "Error opening network device", spcqab.qab_handle,
		netname);

    num1 = dname + 1;			// See if we have an IP address
    while ((chr = *num1++) != 0)
    {
	if (chr != '.' && !isdigit(chr))
	    break;
    }
    if (chr == 0)			// Do we have an IP address?
    {					// Yes - convert it to an inaddr.arpa
	strncpy(ipaddr, dname + 1, 19);	//   name
	if ((num1 = strtok(ipaddr, ".")) == NULL ||
		(num2 = strtok(NULL, ".")) == NULL ||
		(num3 = strtok(NULL, ".")) == NULL ||
		(num4 = strtok(NULL, "")) == NULL)
	{
	    fputs("? DNSTEST: Illegal IP address specified\n", stderr);
	    exit(1);
	}
	sprintf(dname + 1, "%s.%s.%s.%s.in-addr.arpa", num4, num3, num2, num1);
	printf("Resolving %s\n", dname + 1);
        deflt = IPSSF_FINDPTR;
    }
    if (spcqab.qab_option == 0)
	spcqab.qab_option = deflt;
    dname[0] = 0;
    if ((rtn = svcIoQueue(&spcqab)) < 0 || (rtn = spcqab.qab_error) < 0)
	femsg2(prgname, "Error getting domain name mapping", rtn, NULL);

    switch (spcqab.qab_option)
    {
     case IPSSF_FINDIPA:
	if (spcqab.qab_amount == 0)
	{
	    fputs("No IP addresses defined\n", stderr);
	    exit (0);
	}
	printf("%d IP address%s defined:\n", spcqab.qab_amount,
		(spcqab.qab_amount != 1) ? "es" : "");
	pnt = dname;
	do
	{
	    if (pnt[0] != 4)
	    {
		fprintf(stderr, "? DNSTEST: IP address length is not 4 (%d)\n",
			pnt[0]);
		exit (1);
	    }
	    printf("    %d.%d.%d.%d\n", pnt[1], pnt[2], pnt[3], pnt[4]);
	    pnt += 5;
	} while (--(spcqab.qab_amount) > 0);
	break;

     case IPSSF_FINDCNAME:
	if (spcqab.qab_amount == 0)
	{
	    fputs("No canonical name defined\n", stderr);
	    exit (0);
	}
	printf("%d canonical name%s defined:\n", spcqab.qab_amount,
		(spcqab.qab_amount != 1) ? "s" : "");
	goto names;

     case IPSSF_FINDMAIL:
	if (spcqab.qab_amount == 0)
	{
	    fputs("No mail names defined\n", stderr);
	    exit (0);
	}
	printf("%d mail name%s defined:\n", spcqab.qab_amount,
		(spcqab.qab_amount != 1) ? "s" : "");
	goto names;

     case IPSSF_FINDPTR:
	if (spcqab.qab_amount == 0)
	{
	    fputs("No pointers defined\n", stderr);
	    exit (0);
	}
	printf("%d pointer%s defined:\n", spcqab.qab_amount,
		(spcqab.qab_amount != 1) ? "s" : "");
     names:
	pnt = dname;
	do
	{
	    cnt = *pnt++;
	    printf("    %s\n", pnt);
	    pnt += cnt;
	} while (--(spcqab.qab_amount) > 0);
	break;
    }
    exit (0);
}

int havename(
    char *arg)

{
    int   cnt;
    char *pnt;
    char  chr;

    pnt = arg;

    while ((chr = *pnt++) != '\0')	//First see if have device name
    {
        if (chr == ':')
        {
            if (*pnt == ':')
                break;
            pnt = netname;
            cnt = 14;
            do
            {   if (--cnt <= 0)
                {
                    fputs("? DNSTEST: Illegal device name specified\n", stderr);
                    return (FALSE);
                }
                chr = *arg++;
                *pnt++ = chr;
            } while (chr != ':');
            *pnt = '\0';
        }
    }
    if ((dnamelen = strlen(arg)) > 255)
    {
	fputs("? DNSTEST: Domain name is too long\n", stderr);
	return (FALSE);
    }
    strcpy(dname + 1, arg);
    return (TRUE);
}

int afunc(void)

{
    spcqab.qab_option = IPSSF_FINDIPA;
    return (TRUE);
}

int cnamefunc(void)

{
    spcqab.qab_option = IPSSF_FINDCNAME;
    return (TRUE);
}

int mxfunc(void)

{
    spcqab.qab_option = IPSSF_FINDMAIL;
    return (TRUE);
}

int ptrfunc(void)

{
    spcqab.qab_option = IPSSF_FINDPTR;
    return (TRUE);
}

void help(void)

{
    fprintf(stderr, "DNSTEST - version %d.%d\n\n", VERSION, EDITNO);
    fputs("A single argument must be given.  It must specify a domain name or "
	  "an IP\naddress.  If an IP address is given, it is converted to the "
	  "corresponding\nin-addr.arpa domain name.\n\n"
          "The following options may also be specified:\n"
	  "    /HELP or /?  Display this help message\n"
          "    /A           Obtain A records (IP addresses)\n"
	  "    /CNAME       Obtain CNAME record (canonical name)\n"
	  "    /MX          Obtain MX records (mail names)\n"
	  "    /PTR         Obtain PTR record (pointer name)\n"
	  "All options can be specified with one character or the complete "
	  "name.  If\nno option is specified, /A is assumed if a domain name "
	  "is given or /PTR is\nassumed if an IP address is given.\n", stderr);
    exit(1);
}
