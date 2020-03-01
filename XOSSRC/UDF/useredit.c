//==================================================================
// USEREDIT.C - Program to test USERSRV - this program generates UDP
//		request packets and sends them to USERSRV and then
//		waits for and displays the response
//==================================================================

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERRMSG.H>
#include <XOSENCPSWD.H>
#include <XOSUDF.H>
#include <XOSUDFUTIL.H>

#define MAJVER 2
#define MINVER 1

extern ulong swapword (ulong value);
#pragma aux swapword =	\
   "xchg al, ah"	\
    parm [EAX] value [EAX];

extern ulong swaplong (ulong value);
#pragma aux swaplong =	\
   "xchg al, ah"	\
   "ror  eax, 16"	\
   "xchg al, ah"	\
    parm [EAX] value [EAX];

extern int errno;

struct msg
{   char  type;
    char  subtype;
    short seq;
    ulong procid;
    ulong bits1;
    ulong bits2;
    char  data[500];
};

struct msg req;
struct msg resp;

GRPDATA grpdata;
char    group[20];

int   size;
long  udpdev;

uchar   linebufr[140];

typedef struct strdef
{   int    len;
    uchar *pnt;
    char   chg;
} STRDEF;

long  procid;
int   chngcnt;
int   length;
char *dpnt;
char  nullstr[2];
char  useuserid;		// TRUE if should use user ID as key

typedef struct memdef
{   long wslim;
    long pmlim;
    long rmlim;
    long omlin;
    char chg;
} MEMDEF;

typedef struct cpudef
{   long lim;
    char chg;
} CPUDEF;

typedef struct expdef
{   time_s val;
    char   chg;
} EXPDEF;

typedef struct disdef
{   time_s val;
    char   chg;
} HISDEF;

STRDEF dusername;
STRDEF dpassword;
STRDEF duserdesc;
STRDEF dprogram;
STRDEF dhomedir;
STRDEF davlpriv;
STRDEF dinlpriv;
MEMDEF davlmem;
MEMDEF dinlmem;
CPUDEF davlcpu;
CPUDEF dinlcpu;
EXPDEF dpswdexp;
EXPDEF duserexp;
HISDEF dhistory;
STRDEF daccess;
STRDEF davlsect;
STRDEF dinlsect;
STRDEF duserid;
STRDEF dmailname;
STRDEF dcompany;
STRDEF daddr1;
STRDEF daddr2;
STRDEF daddr3;
STRDEF daddr4;
STRDEF dcity;
STRDEF dstate;
STRDEF dzip;
STRDEF dcountry;
STRDEF dphone;

char *prompt[] =
{   NULL,		//		=  0 - Illegal
    "User name",	// UDF_USERNAME =  1 - User name
    "Password",		// UDF_PASSWORD =  2 - Password
    "User description",	// UDF_USERDESC =  3 - User description
    "Initial program",	// UDF_PROGRAM  =  4 - Initial program
    "Home directory",	// UDF_HOMEDIR  =  5 - Home directory
    "Available priv.",	// UDF_AVLPRIV  =  6 - Available privileges
    "Initial priv.",	// UDF_INLPRIV  =  7 - Initial privileges
    "Available memory",	// UDF_AVLMEM   =  8 - Allowed memory limits
    "Initial memory",	// UDF_INLMEM   =  9 - Initial active memory limits
    "Available CPU",	// UDF_AVLCPU   = 10 - Available CPU limits
    "Initial CPU",	// UDF_INLCPU   = 11 - Initial active CPU limits
    "Password exp.",	// UDF_PSWDEXP  = 12 - Password expiration
    "User exp.",	// UDF_USEREXP  = 13 - User expiration
    NULL,		//		= 14 - Illegal
    "Login history",	// UDF_HISTORY  = 15 - Login history
    "Directory list",	// UDF_DIRLIST	= 16 - Directory list status
    "Access",		// UDF_ACCESS	= 17 - Access permissions
    "Available sect.",	// UDF_AVLSECT	= 18 - Available sections
    "Initial sect.",	// UDF_INLSECT	= 19 - Initial sections
    NULL,		//		= 20 - Illegal
    NULL,		//		= 21 - Illegal
    NULL,		//		= 22 - Illegal
    NULL,		//		= 23 - Illegal
    NULL,		//		= 24 - Illegal
    NULL,		//		= 25 - Illegal
    NULL,		//		= 26 - Illegal
    NULL,		//		= 27 - Illegal
    NULL,		//		= 28 - Illegal
    NULL,		//		= 29 - Illegal
    NULL,		//		= 30 - Illegal
    NULL,		//		= 31 - Illegal
    "User ID",		// UDF_USERID   = 32 - User ID
    "Mailing name",	// UDF_MAILNAME = 33 - Mailing name
    "Company name",	// UDF_COMPANY  = 34 - Company name
    "Address (line 1)",	// UDF_ADDR1    = 35 - Address - line 1
    "Address (line 2)",	// UDF_ADDR2    = 36 - Address - line 2
    "Address (line 3)",	// UDF_ADDR3    = 37 - Address - line 3
    "Address (line 4)",	// UDF_ADDR4    = 38 - Address - line 4
    "City",		// UDF_CITY     = 39 - City
    "State",		// UDF_STATE    = 40 - State
    "Postal code",	// UDF_ZIP      = 41 - Postal (ZIP) code
    "Country",		// UDF_COUNTRY  = 42 - Country
    "Phone number"	// UDF_PHONE    = 43 - Phone number
};

void caccess(void);
void caddr1(void);
void caddr2(void);
void caddr3(void);
void caddr4(void);
void cavlcpu(void);
void cavlmem(void);
void cavlpriv(void);
void cavlsect(void);
void cbadtype(void);
void ccity(void);
void ccompany(void);
void ccountry(void);
void cdirlist(void);
void chistory(void);
void chngitems(STRDEF *str, char *msg);
void chngstr(STRDEF *str);
void chomedir(void);
void cinlcpu(void);
void cinlmem(void);
void cinlpriv(void);
void cinlsect(void);
void cmailname(void);
void cpassword(void);
void cphone(void);
void cprogram(void);
void cpswdexp(void);
void cstate(void);
void cuserexp(void);
void cuserid(void);
void cuserdesc(void);
void cusername(void);
void czip(void);

void encodeitem(char *str);
void encodestr(char *str);

void paccess(void);
void paddr1(void);
void paddr2(void);
void paddr3(void);
void paddr4(void);
void pavlcpu(void);
void pavlmem(void);
void pavlpriv(void);
void pavlsect(void);
void pbadtype(void);
void pcity(void);
void pcompany(void);
void pcountry(void);
void pdirlist(void);
void phistory(void);
void phomedir(void);
void pinlcpu(void);
void pinlmem(void);
void pinlpriv(void);
void pinlsect(void);
void pmailname(void);
void ppassword(void);
void pphone(void);
void pprogram(void);
void ppswdexp(void);
void pstate(void);
void puserexp(void);
void puserid(void);
void puserdesc(void);
void pusername(void);
void putnull(STRDEF *data);
void pzip(void);



void eaccess(void);
void eaddr1(void);
void eaddr2(void);
void eaddr3(void);
void eaddr4(void);
void eavlcpu(void);
void eavlmem(void);
void eavlpriv(void);
void eavlsect(void);

void ecity(void);
void ecompany(void);
void ecountry(void);
void edirlist(void);
void ehistory(void);
void ehomedir(void);
void einlcpu(void);
void einlmem(void);
void einlpriv(void);
void einlsect(void);
void emailname(void);
void enull(void);
void ephone(void);
void eprogram(void);
void epswdexp(void);
void estate(void);
void euserexp(void);
void euserid(void);
void euserdesc(void);
void eusername(void);
void eutnull(STRDEF *data);
void ezip(void);


void showitems(char *label, STRDEF *str);
void update(void);

void (*typetbl[])(void) =
{   pbadtype,		//		=  0 - Illegal
    pusername,		// UDF_USERNAME =  1 - User name
    ppassword,		// UDF_PASSWORD =  2 - Password
    puserdesc,		// UDF_USERDESC =  3 - User description
    pprogram,		// UDF_PROGRAM  =  4 - Initial program
    phomedir,		// UDF_HOMEDIR  =  5 - Home directory
    pavlpriv,		// UDF_AVLPRIV  =  6 - Available privileges
    pinlpriv,		// UDF_INLPRIV  =  7 - Initial privileges
    pavlmem,		// UDF_AVLMEM   =  8 - Allowed memory limits
    pinlmem,		// UDF_INLMEM   =  9 - Initial active memory limits
    pavlcpu,		// UDF_AVLCPU   = 10 - Available CPU limits
    pinlcpu,		// UDF_INLCPU   = 11 - Initial active CPU limits
    ppswdexp,		// UDF_PSWDEXP  = 12 - Password expiration
    puserexp,		// UDF_USEREXP  = 13 - User expiration
    pbadtype,		//		= 14 - Illegal
    phistory,		// UDF_HISTORY  = 15 - Login history
    pdirlist,		// UDF_DIRLIST	= 16 - Directory list status
    paccess,		// UDF_ACCESS	= 17 - Access permissions
    pavlsect,		// UDF_AVLSECT	= 18 - Available sections
    pinlsect,		// UDF_INLSECT	= 19 - Initial sections
    pbadtype,		//		= 20 - Illegal
    pbadtype,		//		= 21 - Illegal
    pbadtype,		//		= 22 - Illegal
    pbadtype,		//		= 23 - Illegal
    pbadtype,		//		= 24 - Illegal
    pbadtype,		//		= 25 - Illegal
    pbadtype,		//		= 26 - Illegal
    pbadtype,		//		= 27 - Illegal
    pbadtype,		//		= 28 - Illegal
    pbadtype,		//		= 29 - Illegal
    pbadtype,		//		= 30 - Illegal
    pbadtype,		//		= 31 - Illegal
    puserid,		// UDF_USERID   = 32 - User ID
    pmailname,		// UDF_MAILNAME = 33 - Mailing name
    pcompany,		// UDF_COMPANY  = 34 - Company name
    paddr1,		// UDF_ADDR1    = 35 - Address - line 1
    paddr2,		// UDF_ADDR2    = 36 - Address - line 2
    paddr3,		// UDF_ADDR3    = 37 - Address - line 3
    paddr4,		// UDF_ADDR4    = 38 - Address - line 4
    pcity,		// UDF_CITY     = 39 - City
    pstate,		// UDF_STATE    = 40 - State
    pzip,		// UDF_ZIP      = 41 - Postal (ZIP) code
    pcountry,		// UDF_COUNTRY  = 42 - Country
    pphone		// UDF_PHONE    = 43 - Phone number
};

void (*chngtbl[])(void) =
{   cbadtype,		//		=  0 - Illegal
    cusername,		// UDF_USERNAME =  1 - User name
    cpassword,		// UDF_PASSWORD =  2 - Password
    cuserdesc,		// UDF_USERDESC =  3 - User description
    cprogram,		// UDF_PROGRAM  =  4 - Initial program
    chomedir,		// UDF_HOMEDIR  =  5 - Home directory
    cavlpriv,		// UDF_AVLPRIV  =  6 - Available privileges
    cinlpriv,		// UDF_INLPRIV  =  7 - Initial privileges
    cavlmem,		// UDF_AVLMEM   =  8 - Allowed memory limits
    cinlmem,		// UDF_INLMEM   =  9 - Initial active memory limits
    cavlcpu,		// UDF_AVLCPU   = 10 - Available CPU limits
    cinlcpu,		// UDF_INLCPU   = 11 - Initial active CPU limits
    cpswdexp,		// UDF_PSWDEXP  = 12 - Password expiration
    cuserexp,		// UDF_USEREXP  = 13 - User expiration
    cbadtype,		//		= 14 - Illegal
    chistory,		// UDF_HISTORY  = 15 - Login history
    cdirlist,		// UDF_DIRLIST	= 16 - Directory list status
    caccess,		// UDF_ACCESS	= 17 - Access permissions
    cavlsect,		// UDF_AVLSECT	= 18 - Available sections
    cinlsect,		// UDF_INLSECT	= 19 - Initial sections
    cbadtype,		//		= 20 - Illegal
    cbadtype,		//		= 21 - Illegal
    cbadtype,		//		= 22 - Illegal
    cbadtype,		//		= 23 - Illegal
    cbadtype,		//		= 24 - Illegal
    cbadtype,		//		= 25 - Illegal
    cbadtype,		//		= 26 - Illegal
    cbadtype,		//		= 27 - Illegal
    cbadtype,		//		= 28 - Illegal
    cbadtype,		//		= 29 - Illegal
    cbadtype,		//		= 30 - Illegal
    cbadtype,		//		= 31 - Illegal
    cuserid,		// UDF_USERID   = 32 - User ID
    cmailname,		// UDF_MAILNAME = 33 - Mailing name
    ccompany,		// UDF_COMPANY  = 34 - Company name
    caddr1,		// UDF_ADDR1    = 35 - Address - line 1
    caddr2,		// UDF_ADDR2    = 36 - Address - line 2
    caddr3,		// UDF_ADDR3    = 37 - Address - line 3
    caddr4,		// UDF_ADDR4    = 38 - Address - line 4
    ccity,		// UDF_CITY     = 39 - City
    cstate,		// UDF_STATE    = 40 - State
    czip,		// UDF_ZIP      = 41 - Postal (ZIP) code
    ccountry,		// UDF_COUNTRY  = 42 - Country
    cphone		// UDF_PHONE    = 43 - Phone number
};


void (*encodetbl[])(void) =
{   enull,		//		=  0 - Illegal
    eusername,		// UDF_USERNAME =  1 - User name
    enull,		// UDF_PASSWORD =  2 - Password
    euserdesc,		// UDF_USERDESC =  3 - User description
    eprogram,		// UDF_PROGRAM  =  4 - Initial program
    ehomedir,		// UDF_HOMEDIR  =  5 - Home directory
    eavlpriv,		// UDF_AVLPRIV  =  6 - Available privileges
    einlpriv,		// UDF_INLPRIV  =  7 - Initial privileges
    eavlmem,		// UDF_AVLMEM   =  8 - Allowed memory limits
    einlmem,		// UDF_INLMEM   =  9 - Initial active memory limits
    eavlcpu,		// UDF_AVLCPU   = 10 - Available CPU limits
    einlcpu,		// UDF_INLCPU   = 11 - Initial active CPU limits
    epswdexp,		// UDF_PSWDEXP  = 12 - Password expiration
    euserexp,		// UDF_USEREXP  = 13 - User expiration
    enull,		//		= 14 - Illegal
    ehistory,		// UDF_HISTORY  = 15 - Login history
    edirlist,		// UDF_DIRLIST	= 16 - Directory list status
    eaccess,		// UDF_ACCESS	= 17 - Access permissions
    eavlsect,		// UDF_AVLSECT	= 18 - Available sections
    einlsect,		// UDF_INLSECT	= 19 - Initial sections
    enull,		//		= 20 - Illegal
    enull,		//		= 21 - Illegal
    enull,		//		= 22 - Illegal
    enull,		//		= 23 - Illegal
    enull,		//		= 24 - Illegal
    enull,		//		= 25 - Illegal
    enull,		//		= 26 - Illegal
    enull,		//		= 27 - Illegal
    enull,		//		= 28 - Illegal
    enull,		//		= 29 - Illegal
    enull,		//		= 30 - Illegal
    enull,		//		= 31 - Illegal
    euserid,		// UDF_USERID   = 32 - User ID
    emailname,		// UDF_MAILNAME = 33 - Mailing name
    ecompany,		// UDF_COMPANY  = 34 - Company name
    eaddr1,		// UDF_ADDR1    = 35 - Address - line 1
    eaddr2,		// UDF_ADDR2    = 36 - Address - line 2
    eaddr3,		// UDF_ADDR3    = 37 - Address - line 3
    eaddr4,		// UDF_ADDR4    = 38 - Address - line 4
    ecity,		// UDF_CITY     = 39 - City
    estate,		// UDF_STATE    = 40 - State
    ezip,		// UDF_ZIP      = 41 - Postal (ZIP) code
    ecountry,		// UDF_COUNTRY  = 42 - Country
    ephone		// UDF_PHONE    = 43 - Phone number
};

char  prgname[] = "USEREDIT";

static void *getmem(int size);
static void updatestr(int type, STRDEF *value);

void main(void)

{
    long   rtn;
    int    type;
    uchar *pnt;
    uchar *text;
    uchar  chr;

    procid = svcSysGetPid();;
    req.type = MT_UDF;
    req.subtype = 1;
    req.seq = 1234;
    req.procid = procid;

    banner();
    fputs("\nUser Name: ", stdout);
    fgets(linebufr, 140, stdin);

    text = linebufr;
    while ((chr = *text) != 0 && isspace(chr))
        text++;
    if (chr == '!')
    {
        useuserid = TRUE;
        text++;
    }
    else
        useuserid = FALSE;
    if (getgroup(text, &grpdata) < 0)	// Get the group data
    {
        printf("? USEREDIT: Group %s is not defined\n", group);
        exit(1);
    }
    if ((udpdev = svcIoOpen(O_OUT|O_IN, grpdata.udpdev, NULL)) < 0)
        femsg2(prgname, "Cannot open UDP device", udpdev, NULL);
    pnt = req.data;
    *pnt++ = grpdata.grplen;		// Store group name in the request
    memcpy(pnt, grpdata.grpname, grpdata.grplen);
    pnt += grpdata.grplen;
    *pnt++ = grpdata.usrlen;		// Store key in the request
    memcpy(pnt, grpdata.usrname, grpdata.usrlen);
    pnt += grpdata.usrlen;
    req.bits1 = swaplong((useuserid)? (0xFFFFFF00|URQ1_KEYUID): 0xFFFFFF00);
    req.bits2 = 0xFFFFFFFF;
    if ((rtn = reqresp(udpdev, grpdata.addr, grpdata.port, &req,
            pnt - (char *)&req, &resp, sizeof(resp))) < 0)
					// Send request and get response
        femsg2(prgname, "Error obtaining information from UDF server",
                rtn, NULL);
    req.bits1 = swaplong((useuserid)? (URQ1_UPDATE|URQ1_KEYUID): URQ1_UPDATE);
    req.bits2 = 0;
    if (resp.subtype == 3)
    {
        fputs("User not found, create new user record? ", stdout);
        fgets(linebufr, 140, stdin);
        if (toupper(linebufr[0]) != 'Y')
            exit(1);
        req.bits1 = swaplong((useuserid)?
                (URQ1_CREATE|URQ1_KEYUID): URQ1_CREATE);
    }
    else if (resp.subtype == 4)
    {
        fputs("? Incorrect password\n", stdout);
        exit(1);
    }

    dusername.pnt = nullstr;
    dusername.len = 0;
    dusername.chg = FALSE;
    duserdesc.pnt = nullstr;
    duserdesc.len = 0;
    duserdesc.chg = FALSE;
    dprogram.pnt = nullstr;
    dprogram.len = 0;
    dprogram.chg = FALSE;
    dhomedir.pnt = nullstr;
    dhomedir.len = 0;
    dhomedir.chg = FALSE;
    davlpriv.pnt = nullstr;
    davlpriv.len = 0;
    davlpriv.chg = FALSE;
    dinlpriv.pnt = nullstr;
    dinlpriv.len = 0;
    dinlpriv.chg = FALSE;
    daccess.pnt = nullstr;
    daccess.len = 0;
    davlsect.pnt = nullstr;
    davlsect.len = 0;
    davlsect.chg = FALSE;
    dinlsect.pnt = nullstr;
    dinlsect.len = 0;
    dinlsect.chg = FALSE;
    duserid.pnt = nullstr;
    duserid.len = 0;
    duserid.chg = FALSE;
    dmailname.pnt = nullstr;
    dmailname.len = 0;
    dmailname.chg = FALSE;
    dcompany.pnt = nullstr;
    dcompany.len = 0;
    dcompany.chg = FALSE;
    daddr1.pnt = nullstr;
    daddr1.len = 0;
    daddr1.chg = FALSE;
    daddr2.pnt = nullstr;
    daddr2.len = 0;
    daddr2.chg = FALSE;
    daddr3.pnt = nullstr;
    daddr3.len = 0;
    daddr3.chg = FALSE;
    daddr4.pnt = nullstr;
    daddr4.len = 0;
    daddr4.chg = FALSE;
    dcity.pnt = nullstr;
    dcity.len = 0;
    dcity.chg = FALSE;
    dstate.pnt = nullstr;
    dstate.len = 0;
    dstate.chg = FALSE;
    dzip.pnt = nullstr;
    dzip.len = 0;
    dzip.chg = FALSE;
    dcountry.pnt = nullstr;
    dcountry.len = 0;
    dzip.chg = FALSE;
    dphone.pnt = nullstr;
    dphone.len = 0;
    dphone.chg = FALSE;
    chngcnt = 0;

    if (!(req.bits1 & swaplong(URQ1_CREATE)))
    {
        rtn -= 16;
        dpnt = resp.data;
        while (rtn >= 2)
        {
            type = *dpnt++;
            length = *dpnt++;
            if ((rtn -= (length + 2)) < 0)
            {
                fputs("**** Record extends beyond end of datagram!\n", stdout);
                exit(1);
            }
            if (type <= (sizeof(typetbl)/sizeof(void *)))
                typetbl[type]();
            dpnt += length;
        }
        if (rtn != 0)
        {
            printf("**** Have %d extra byte%s at end of datagram\n",
                    rtn, (rtn != 1)? "s": "");
            exit(1);
        }
        putnull(&dusername);
        putnull(&duserdesc);
        putnull(&dprogram);
        putnull(&dhomedir);
        putnull(&davlpriv);
        putnull(&dinlpriv);
		putnull(&daccess);
        putnull(&davlsect);
        putnull(&dinlsect);
        putnull(&duserid);
        putnull(&dmailname);
        putnull(&dcompany);
        putnull(&daddr1);
        putnull(&daddr2);
        putnull(&daddr3);
        putnull(&daddr4);
        putnull(&dcity);
        putnull(&dstate);
        putnull(&dzip);
        putnull(&dcountry);
        putnull(&dphone);
    }
    else
    {
        if (useuserid)
        {
            duserid.pnt = grpdata.usrname;
            duserid.len = grpdata.usrlen;
            duserid.chg = TRUE;
        }
        else
        {
            dusername.pnt = grpdata.usrname;
            dusername.len = grpdata.usrlen;
            dusername.chg = TRUE;
        }
        chngcnt++;
    }
    for (;;)
    {
	banner();
	printf("               Group: %s\n", grpdata.grpname);
        dusername.pnt[dusername.len] = 0;
        printf(" 1         User name: %s\n", dusername.pnt);
        printf(" 2          Password: %s\n", (dpassword.len != 0)? "*": "");
        duserdesc.pnt[duserdesc.len] = 0;
        printf(" 3  User description: %s\n", duserdesc.pnt);
        dprogram.pnt[dprogram.len] = 0;
        printf(" 4   Initial program: %s\n", dprogram.pnt);
        dhomedir.pnt[dhomedir.len] = 0;
        printf(" 5    Home directory: %s\n", dhomedir.pnt);
        showitems(" 6   Available priv.", &davlpriv);
        showitems(" 7     Initial priv.", &dinlpriv);
        printf(" 8    Available mem.:\n");
        printf(" 9      Initial mem.:\n");
        printf("10     Available CPU:\n");
        printf("11       Initial CPU:\n");
        printf("12     Password exp.:\n");
        printf("13         User exp.:\n");
        printf("14        User flags:\n");
        printf("15     Login history:\n");
        printf("16 Directory listing:\n");
		showitems("17      Access perm.", &daccess);
        showitems("18   Available sect.", &davlsect);
        showitems("19     Initial sect.", &dinlsect);
        duserid.pnt[duserid.len] = 0;
        printf("32           User ID: %s\n", duserid.pnt);
        dmailname.pnt[dmailname.len] = 0;
        printf("33      Mailing name: %s\n", dmailname.pnt);
        dcompany.pnt[dcompany.len] = 0;
        printf("34      Company name: %s\n", dcompany.pnt);
        daddr1.pnt[daddr1.len] = 0;
        printf("35  Address (line 1): %s\n", daddr1.pnt);
        daddr2.pnt[daddr2.len] = 0;
        printf("36  Address (line 2): %s\n", daddr2.pnt);
        daddr3.pnt[daddr3.len] = 0;
        printf("37  Address (line 3): %s\n", daddr3.pnt);
        daddr4.pnt[daddr4.len] = 0;
        printf("38  Address (line 4): %s\n", daddr4.pnt);
        dcity.pnt[dcity.len] = 0;
        printf("39              City: %s\n", dcity.pnt);
        dstate.pnt[dstate.len] = 0;
        printf("40             State: %s\n", dstate.pnt);
        dzip.pnt[dzip.len] = 0;
        printf("41       Postal code: %s\n", dzip.pnt);
        dcountry.pnt[dcountry.len] = 0;
        printf("42           Country: %s\n", dcountry.pnt);
        dphone.pnt[dphone.len] = 0;
        printf("43      Phone number: %s\n", dphone.pnt);

        for (;;)
        {
            fputs("\nItem to change: ", stdout);
            fgets(linebufr, 140, stdin);
            if (linebufr[0] == '\n' || linebufr[0] == 0)
            {
                if (chngcnt == 0)
                    fputs("\nNo changes made\n", stdout);
                else
                    update();
                exit (0);
            }
            type = atoi(linebufr);
            if (type >= (sizeof(chngtbl)/sizeof(void *)) ||
                    prompt[type] == NULL)
            {
                fputs("? Illegal item number\n", stdout);
                continue;
            }
            printf("\n%s: ", prompt[type]);

            encodetbl[type]();

            fgets(linebufr, 140, stdin);
            pnt = linebufr;
            while ((chr = *pnt) != 0 && chr != '\n')
                pnt++;
            *pnt = 0;
            chngtbl[type]();
            break;
        }
    }
}

void showitems(
    char   *label,
    STRDEF *str)

{
    uchar *pnts;
    uchar *pntd;
    int    cnt;
    uchar  chr;
    char   delimit;
    char   buffer[2000];

    cnt = 2000;
    pntd = buffer;
    pnts = str->pnt;
    delimit = 0;

    while ((chr = *pnts++) != 0)
    {
        if (chr & 0x80)
        {
            *pntd++ = chr & 0x7F;
             delimit = ',';
        }
        else
        {
            if (delimit != 0)
            {
                *pntd++ = delimit;
                delimit = 0;
            }
            *pntd++ = chr;
        }
    }
    *pntd = 0;
    printf("%s: %s\n", label, buffer);
}

void putnull(
    STRDEF *data)

{
    if (data->len != 0)
        data->pnt[data->len] = 0;
}


// Function: pbadtype - Process illegal records

void pbadtype(void)

{

}

// Function: pusername - Process UDF_USERNAME records (user name)

void pusername(void)

{
    dusername.len = length;
    dusername.pnt = dpnt;
}

// Function: ppassword - Process UDF_PASSWORD records (password)

void ppassword(void)

{
    dpassword.len = length;
    dpassword.pnt = dpnt;
}

// Function: puserdesc - Process UDF_USERDESC records (user description)

void puserdesc(void)

{
    duserdesc.len = length;
    duserdesc.pnt = dpnt;
}

// Function: pprogram - Process UDF_PROGRAM records (initial program)

void pprogram(void)

{
    dprogram.len = length;
    dprogram.pnt = dpnt;
}

// Function: phomedir - Process UDF_HOMEDIR records (home directory)

void phomedir(void)

{
    dhomedir.len = length;
    dhomedir.pnt = dpnt;
}

// Function: palvpriv - Process UDF_AVLPRIV records (available priviledges)

void pavlpriv(void)

{
    davlpriv.len = length;
    davlpriv.pnt = dpnt;
}

// Function: pinlpriv - Process UDF_INLPRIV records (initial priviledges)

void pinlpriv(void)

{
    dinlpriv.len = length;
    dinlpriv.pnt = dpnt;
}

// Function: pavlmem - Process UDF_ALWMEM records (available memory limits)

void pavlmem(void)

{

}

// Function: pinlmem - Process UDF_INLMEM records (initial memory limits)

void pinlmem(void)

{

}

// Function: palwcpu - Process UDF_AVLCPU records (available CPU limits)

void pavlcpu(void)

{

}

// Function: pinlcpu - Process UDF_INLCPU records (initial CPU limits)

void pinlcpu(void)

{

}

// Function: ppswdexp - Process UDF_PSWDEXP records (password expiration)

void ppswdexp(void)

{

}

// Function: puserexp - Process UDF_USEREXP records (user expiration)

void puserexp(void)

{

}

// Function: phistory - Process UDF_HISTORY records (login history)

void phistory(void)

{

}

// Function: pdirlist - Process UDF_DIRLIST records (directory list state)

void pdirlist(void)

{

}

// Function: paccess - Process UDF_ACCESS records (access permissions)

void paccess(void)

{
    daccess.len = length;
    daccess.pnt = dpnt;
}

// Function: palvsect - Process UDF_AVLSECT records (available sections)

void pavlsect(void)

{
    davlsect.len = length;
    davlsect.pnt = dpnt;
}

// Function: pinlsect - Process UDF_INLSECT records (initial sections)

void pinlsect(void)

{
    dinlsect.len = length;
    dinlsect.pnt = dpnt;
}

// Function: puserid - Process UDF_USERID records (user ID)

void puserid(void)

{
    duserid.len = length;
    duserid.pnt = dpnt;
}

// Function: pmailname - Process UDF_MAINNAME records (mailing name)

void pmailname(void)

{
    dmailname.len = length;
    dmailname.pnt = dpnt;
}

// Function: pcompany - Process UDF_COMPANY records (company name)

void pcompany(void)

{
    dcompany.len = length;
    dcompany.pnt = dpnt;
}

// Function: paddr1 - Process UDF_ADDR1 records (address line 1)

void paddr1(void)

{
    daddr1.len = length;
    daddr1.pnt = dpnt;
}

// Function: paddr2 - Process UDF_ADDR2 records (address line 2)

void paddr2(void)

{
    daddr2.len = length;
    daddr2.pnt = dpnt;
}

// Function: paddr3 - Process UDF_ADDR3 records (address line 3)

void paddr3(void)

{
    daddr3.len = length;
    daddr3.pnt = dpnt;
}

// Function: paddr4 - Process UDF_ADDR4 records (address line 4)

void paddr4(void)

{
    daddr4.len = length;
    daddr4.pnt = dpnt;
}

// Function: pcity - Process UDF_CITY records (city)

void pcity(void)

{
    dcity.len = length;
    dcity.pnt = dpnt;
}

// Function: pstate - Process UDF_STATE records (state)

void pstate(void)

{
    dstate.len = length;
    dstate.pnt = dpnt;
}

// Function: pzip - Process UDF_ZIP records (postal (ZIP) code)

void pzip(void)

{
    dzip.len = length;
    dzip.pnt = dpnt;
}

// Function: pcountry - Process UDF_COUNTRY records (country)

void pcountry(void)

{
    dcountry.len = length;
    dcountry.pnt = dpnt;
}

// Function: pphone - Process UDF_PHONE records (phone number)

void pphone(void)

{
    dphone.len = length;
    dphone.pnt = dpnt;
}

// Function: cbadtype - change illegal records

void cbadtype(void)

{

}

// Function: cusername - Change UDF_USERNAME records (user name)

void cusername(void)

{
    chngstr(&dusername);
}

// Function: cpassword - Change UDF_PASSWORD records (password)

void cpassword(void)

{
    char *text;

    chngstr(&dpassword);
    if (dpassword.len != 0)
    {
		text = dpassword.pnt;
		dpassword.pnt = (char *)getmem(40);
		dpassword.len = 40;
		if (!encodepassword(text, dpassword.pnt))
			fputs("Warning: Password generated a weak encryption key!\n",
					stderr);
		free(text);
    }
}

// Function: cuserdesc - Change UDF_USERDESC records (user description)

void cuserdesc(void)

{
    chngstr(&duserdesc);
}

// Function: cprogram - Change UDF_PROGRAM records (initial program)

void cprogram(void)

{
    chngstr(&dprogram);
}

// Function: chomedir - Change UDF_HOMEDIR records (home directory)

void chomedir(void)

{
    chngstr(&dhomedir);
}

// Function: calvpriv - Change UDF_AVLPRIV records (available priviledges)

void cavlpriv(void)

{
    chngitems(&davlpriv, "priviledge");
}

// Function: cinlpriv - Change UDF_INLPRIV records (initial priviledges)

void cinlpriv(void)

{
    chngitems(&dinlpriv, "priviledge");
}

// Function: cavlmem - Change UDF_ALWMEM records (available memory limits)

void cavlmem(void)

{

}

// Function: cinlmem - Change UDF_INLMEM records (initial memory limits)

void cinlmem(void)

{

}

// Function: calwcpu - Change UDF_AVLCPU records (available CPU limits)

void cavlcpu(void)

{

}

// Function: cinlcpu - Change UDF_INLCPU records (initial CPU limits)

void cinlcpu(void)

{

}

// Function: cpswdexp - Change UDF_PSWDEXP records (password expiration)

void cpswdexp(void)

{

}

// Function: cuserexp - Change UDF_USEREXP records (user expiration)

void cuserexp(void)

{

}

// Function: chistory - Change UDF_HISTORY records (login history)

void chistory(void)

{

}

// Function: cdirlist - Change UDF_DIRLIST records (directory list state)

void cdirlist(void)

{

}

// Function: caccess - Change UDF_ACCESS records (available sections)

void caccess(void)

{
    chngitems(&daccess, "access");
}

// Function: calvsect - Change UDF_AVLSECT records (available sections)

void cavlsect(void)

{
    chngitems(&davlsect, "section");
}

// Function: cinlsect - Change UDF_INLSECT records (initial sections)

void cinlsect(void)

{
    chngitems(&dinlsect, "section");
}

// Function: cuserid - Change UDF_USERID records (user ID)

void cuserid(void)

{
    chngstr(&duserid);
}

// Function: cmailname - Change UDF_MAINNAME records (mailing name)

void cmailname(void)

{
    chngstr(&dmailname);
}

// Function: ccompany - Change UDF_COMPANY records (company name)

void ccompany(void)

{
    chngstr(&dcompany);
}

// Function: caddr1 - Change UDF_ADDR1 records (address line 1)

void caddr1(void)

{
    chngstr(&daddr1);
}

// Function: caddr2 - Change UDF_ADDR2 records (address line 2)

void caddr2(void)

{
    chngstr(&daddr2);
}

// Function: caddr3 - Change UDF_ADDR3 records (address line 3)

void caddr3(void)

{
    chngstr(&daddr3);
}

// Function: caddr4 - Change UDF_ADDR4 records (address line 4)

void caddr4(void)

{
    chngstr(&daddr4);
}

// Function: ccity - Change UDF_CITY records (city)

void ccity(void)

{
    chngstr(&dcity);
}

// Function: cstate - Change UDF_STATE records (state)

void cstate(void)

{
    chngstr(&dstate);
}

// Function: czip - Change UDF_ZIP records (postal (ZIP) code)

void czip(void)

{
    chngstr(&dzip);
}

// Function: ccountry - Change UDF_COUNTRY records (country)

void ccountry(void)

{
    chngstr(&dcountry);
}

// Function: cphone - Change UDF_PHONE records (phone number)

void cphone(void)

{
    chngstr(&dphone);
}

//**********************************************************************
// Function: chngitems - Change items with a string value which consists
//		of a list of item names
// Returned: Nothing
//**********************************************************************

void chngitems(
    STRDEF *str,
    char   *msg)

{
    uchar *pnts;
    uchar *pntd;
    uchar  chr;

    pnts = pntd = linebufr;

    for (;;)
    {
        while ((chr = *pnts) != 0 && (chr == '+' || chr == ',' || isspace(chr)))
            pnts++;
        if (pntd != linebufr)
            pntd[-1] |= 0x80;
        if (chr == 0)
            break;
        if (!isalpha(chr))
        {
            fprintf(stderr, "? USEREDIT: Illegal %s name specified\n", msg);
            exit(1);
        }
        while (isalnum(chr = *pnts++) || chr == '^' || chr == '_')
            *pntd++ = toupper(chr);
    }
    *pntd = 0;
    chngstr(str);
}

//*********************************************************
// Function: chngstr - Change item with simple string value
// Returned: Nothing
//*********************************************************

void chngstr(
    STRDEF *str)

{
    int   size;

    if (str->chg)
        free(str->pnt);
    if ((size = strlen(linebufr)) == 0)
    {
        str->pnt = nullstr;
        str->len = 0;
    }
    else
    {
		str->pnt = (char *)getmem(size + 1);
		memcpy(str->pnt, linebufr, size + 1);
		str->pnt[size] = 0;
		str->len = size;
    }
    str->chg = TRUE;
    chngcnt++;
}

// Function: enull - Encode null record

void enull(void)

{

}

// Function: eusername - Encode UDF_USERNAME records (user name)

void eusername(void)

{
    encodestr(dusername.pnt);
}

// Function: euserdesc - Encode UDF_USERDESC records (user description)

void euserdesc(void)

{
    encodestr(duserdesc.pnt);
}

// Function: eprogram - Encode UDF_PROGRAM records (initial program)

void eprogram(void)

{
    encodestr(dprogram.pnt);
}

// Function: ehomedir - Encode UDF_HOMEDIR records (home directory)

void ehomedir(void)

{
    encodestr(dhomedir.pnt);
}

// Function: ealvpriv - Encode UDF_AVLPRIV records (available priviledges)

void eavlpriv(void)

{
    encodeitems(davlpriv.pnt);
}

// Function: einlpriv - Encode UDF_INLPRIV records (initial priviledges)

void einlpriv(void)

{
    encodeitems(dinlpriv.pnt);
}

// Function: eavlmem - Encode UDF_ALWMEM records (available memory limits)

void eavlmem(void)

{

}

// Function: einlmem - Encode UDF_INLMEM records (initial memory limits)

void einlmem(void)

{

}

// Function: ealwcpu - Encode UDF_AVLCPU records (available CPU limits)

void eavlcpu(void)

{

}

// Function: einlcpu - Encode UDF_INLCPU records (initial CPU limits)

void einlcpu(void)

{

}

// Function: epswdexp - Encode UDF_PSWDEXP records (password expiration)

void epswdexp(void)

{

}

// Function: euserexp - Encode UDF_USEREXP records (user expiration)

void euserexp(void)

{

}

// Function: ehistory - Encode UDF_HISTORY records (login history)

void ehistory(void)

{

}

// Function: edirlist - Encode UDF_DIRLIST records (directory list state)

void edirlist(void)

{

}

// Function: eaccess - Encode UDF_ACCESS records (available sections)

void eaccess(void)

{
    encodeitems(daccess.pnt);
}

// Function: ealvsect - Encode UDF_AVLSECT records (available sections)

void eavlsect(void)

{
    encodeitems(davlsect.pnt);
}

// Function: einlsect - Encode UDF_INLSECT records (initial sections)

void einlsect(void)

{
    encodeitems(dinlsect.pnt);
}

// Function: euserid - Encode UDF_USERID records (user ID)

void euserid(void)

{
    encodestr(duserid.pnt);
}

// Function: emailname - Encode UDF_MAINNAME records (mailing name)

void emailname(void)

{
    encodestr(dmailname.pnt);
}

// Function: ecompany - Encode UDF_COMPANY records (company name)

void ecompany(void)

{
    encodestr(dcompany.pnt);
}

// Function: eaddr1 - Encode UDF_ADDR1 records (address line 1)

void eaddr1(void)

{
    encodestr(daddr1.pnt);
}

// Function: eaddr2 - Encode UDF_ADDR2 records (address line 2)

void eaddr2(void)

{
    encodestr(daddr2.pnt);
}

// Function: eaddr3 - Encode UDF_ADDR3 records (address line 3)

void eaddr3(void)

{
    encodestr(daddr3.pnt);
}

// Function: eaddr4 - Encode UDF_ADDR4 records (address line 4)

void eaddr4(void)

{
    encodestr(daddr4.pnt);
}

// Function: ecity - Encode UDF_CITY records (city)

void ecity(void)

{
    encodestr(dcity.pnt);
}

// Function: estate - Encode UDF_STATE records (state)

void estate(void)

{
    encodestr(dstate.pnt);
}

// Function: ezip - Encode UDF_ZIP records (postal (ZIP) code)

void ezip(void)

{
    encodestr(dzip.pnt);
}

// Function: ecountry - Encode UDF_COUNTRY records (country)

void ecountry(void)

{
    encodestr(dcountry.pnt);
}

// Function: ephone - Encode UDF_PHONE records (phone number)

void ephone(void)

{
    encodestr(dphone.pnt);
}

//************************************************************************
// Function: encodeitems - Encode items with a string value which consists
//		of a list of item names
// Returned: Nothing
//************************************************************************

void encodeitems(
    char *str)

{
    char *bpnt;
    char  chr;
    char  needcomma;
    char  bufr[300];

    needcomma = FALSE;
    bpnt = bufr;
    while ((chr = *str++) != 0)
    {
	if (needcomma)
	{
	    *bpnt++ = ',';
	    needcomma = FALSE;
	}
	if ((chr & 0x80) == 0)
	    *bpnt++ = chr;
	else
	{
	    *bpnt++ = chr & 0x7F;
	    needcomma = TRUE;
	}
    }
    *bpnt = 0;
    encodestr(bufr);
}

//***********************************************************
// Function: encodestr - Encode item with simple string value
// Returned: Nothing
//***********************************************************

void encodestr(
    char *str)

{
    int rtn;

    if ((rtn = strlen(str)) != 0)
	if ((rtn = svcTrmWrtInB(STDTRM, str, rtn)) < 0)
	    femsg2(prgname, "Error writing to terminal buffer\n", rtn, NULL);
}

//****************************************************
// Function: update - Generate and send update message
// Returned: Nothing
//****************************************************

void update(void)

{
    long rtn;

    req.type = MT_UDF;
    req.subtype = 1;
    req.seq = 3456;
    req.procid = procid;
    dpnt = req.data;

    *dpnt++ = grpdata.grplen;		// Store group name
    memcpy(dpnt, grpdata.grpname, grpdata.grplen);
    dpnt += grpdata.grplen;
    *dpnt++ = grpdata.usrlen;		// Store user name
    memcpy(dpnt, grpdata.usrname, grpdata.usrlen);
    dpnt += grpdata.usrlen;
    updatestr(UDF_USERNAME, &dusername); // UDF_USERNAME = 1 - User name
    updatestr(UDF_PASSWORD, &dpassword); // UDF_PASSWORD = 2 - Passowrd
    updatestr(UDF_USERDESC, &duserdesc); // UDF_USERDESC = 3 - User description
    updatestr(UDF_PROGRAM, &dprogram);	// UDF_PROGRAM = 4 - Initial program
    updatestr(UDF_HOMEDIR, &dhomedir);	// UDF_HOMEDIR = 5 - Home directory
    updatestr(UDF_AVLPRIV, &davlpriv);	// UDF_AVLPRIV  =  6 - Available privileges
    updatestr(UDF_INLPRIV, &dinlpriv);	// UDF_INLPRIV  =  7 - Initial privileges
    {

    }
    if (davlmem.chg)			// UDF_AVLMEM   =  8 - Allowed memory limits
    {

    }
    if (dinlmem.chg)			// UDF_INLMEM   =  9 - Initial active memory limits
    {

    }
    if (davlcpu.chg)			// UDF_AVLCPU   = 10 - Available CPU limits
    {

    }
    if (dinlcpu.chg)			// UDF_INLCPU   = 11 - Initial active CPU limits
    {

    }
    if (dpswdexp.chg)			// UDF_PSWDEXP  = 12 - Password expiration
    {

    }
    if (duserexp.chg)			// UDF_USEREXP  = 13 - User expiration
    {

    }
    if (dhistory.chg)			// UDF_HISTORY  = 15 - Login history
    {

    }

    updatestr(UDF_ACCESS, &daccess);	// UDF_ACCESS   = 17 - Access permissions
    updatestr(UDF_AVLSECT, &davlsect);	// UDF_AVLSECT  = 18 - Available sections
    updatestr(UDF_INLSECT, &dinlsect);	// UDF_INLSect  = 19 - Initial sections

    updatestr(UDF_USERID, &duserid);
    updatestr(UDF_MAILNAME, &dmailname);
    updatestr(UDF_COMPANY, &dcompany);
    updatestr(UDF_ADDR1, &daddr1);
    updatestr(UDF_ADDR2, &daddr2);
    updatestr(UDF_ADDR3, &daddr3);
    updatestr(UDF_ADDR4, &daddr4);
    updatestr(UDF_CITY, &dcity);
    updatestr(UDF_STATE, &dstate);
    updatestr(UDF_ZIP, &dzip);
    updatestr(UDF_COUNTRY, &dcountry);
    updatestr(UDF_PHONE, &dphone);

    if ((rtn = reqresp(udpdev, grpdata.addr, grpdata.port, &req,
            dpnt - (char *)&req, &resp, sizeof(resp))) < 0)
					// Send request and get response
        femsg2(prgname, "Error sending update to UDF server",
                rtn, NULL);

    if (resp.subtype == 2)
        fputs("\nRecord updated\n", stdout);
    else
    {
        printf("\nError indicated: %d\n", resp.subtype);
    }
}

//***************************************
// Function: banner - Display banner line
// Returned: Nothing
//***************************************

void banner(void)

{
    printf("\x1B[f\x1B[JUseredit - version %d.%d\n\n", MAJVER, MINVER);
}

//*********************************************************************
// Function: updatestr - Generate update record for simple string value
// Returned: Nothing
//*********************************************************************

static void updatestr(
    int     type,
    STRDEF *value)

{
    if (value->chg)
    {
        *dpnt++ = type;
        *dpnt++ = value->len;
        memcpy(dpnt, value->pnt, value->len);
        dpnt += value->len;
    }
}

static void *getmem(
    int size)

{
    void *pnt;

    if ((pnt = malloc(size)) == NULL)
    {
	fputs("? USEREDIT: Cannot allocate memory\n", stderr);
	exit(1);
    }
    return (pnt);
}
