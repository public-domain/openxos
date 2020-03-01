#define VECT_MESSAGE 50
#define VECT_SECOND  51

#define ulong  unsigned long
#define ushort unsigned short
#define uchar  unsigned char

struct cfgchar
{   lngstr_char sysname;
    byte4_char  memtotal;
    char        end;
};

struct statechar
{
    byte4_char sysstate;
    byte4_char initial;
    char       end;
};

struct trmclrparms
{   byte4_parm cinpmode;
    byte4_parm sinpmode;
    byte4_parm coutmode;
    char       end;
};

struct pwchar
{   lngstr_char password;
    lngstr_char program;
    char        end;
};

struct msgparm
{   lngstr_parm srcname;
    char        end;
};

struct intdata
{   long  svdEDI;
    long  svdESI;
    long  svdEBP;
    long  svdESP;
    long  svdEBX;
    long  svdEDX;
    long  svdECX;
    long  svdEAX;
    long  intGS;
    long  intFS;
    long  intES;
    long  intDS;
    long  intEIP;
    long  intCS;
    long  intEFR;
    short intcnt;
    short intnum;
    long  pid;
    long  term;
    long  pEIP;
    long  pCS;
    long  pEFR;
    long  pos;
    long  pseg;
    long  data;
};

struct devitem
{   long  src;
    long  dst;
    long  bits;
    long  xxx;
};

struct devlist
{   struct devitem dlstdin;
    struct devitem dlstdout;
    struct devitem dlstderr;
    struct devitem dlstdtrm;
};

struct runparm
{   lngstr_parm arglist;
    lngstr_parm devlist;
    char        end;
};

struct vectdata
{   short size;
    char  level;
    char  vector;
};

struct ratedata
{   char  mask;
    char  match;
    long  rate;
    short delay;
};

extern int      mincnt;
extern ulong    initpid;
extern long     conhndl;
extern long     logdev;
extern long     logdate;
extern uchar    logcommit;
extern time_sz  bgndt;
extern long     rtnval;
extern char     dttmbfr[];
extern struct   devlist devlist;
extern char     envlist[];
extern char     welcome[];
extern char     childtext[];
extern struct   ratedata ratedata1[];
extern int      numrates1;
extern struct   ratedata ratedata2[];
extern int      numrates2;
extern char     prgname[];
extern char     imsg0[];
extern char     sysname[];
extern char     startupdone;
extern char     ipmerrmsg[];
extern char     msgbfr[];
extern char     srcbfr[];
extern struct   cfgchar   cfgchar;
extern struct   statechar statechar;
extern type_qab cfgqab;
extern struct   trmclrparms trmclrparms;
extern char     pwbufr[];
extern char     prgmbufr[];
extern type_qab pwqab;
extern struct   msgparm   msginpparm;
extern struct   msgparm   msgoutparm;
extern type_qab msginpqab;
extern type_qab runqab;
extern struct   runparm   runparm;


void   childgone(struct intdata);
void   initlog(void);
void   message(void);
void   onceasecond(void);
int    openlog(ulong pid, char *prgname);
void   putinlog(ulong pid, char *label, char *text);
void   response(char *header, char *msg);
char  *storehex(char *pnt, ulong value);
void   symbiontreq(void);
void   syslogdata(void);
void   termdata(void);
void   termreq(void);
void   userreq(void);
void   uf_logerr(char *);
void   uf_reply(char *, int);
FILE  *openuf(struct user_header *);
ulong  search4uf(FILE *, struct user *, struct user *);
void   ufnotfound(void);
uchar *passwd_garble(unsigned char *);
void   rolllog(long date, char *text);
long   timechg(time_s *, time_s *);
void   uf_sendmsg(char *, int);
void   uferror(unsigned long, char *);
long   writeuf(FILE *, struct user *, struct user_header *);
