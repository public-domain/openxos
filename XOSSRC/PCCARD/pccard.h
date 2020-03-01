// Define debug level values (bit encoded)

#define DEBUG_DEVICE  0x01	// Report PC-card and device set up
#define DEBUG_TUPLES  0x02	// Report values while scanning CIS tuples
#define DEBUG_CONFIG  0x04	// Report configuration summary after CIS
				//   tuples are scanned

// Define device type codes

#define TYPE_DISK_HDKA  1


typedef struct idb     IDB;
typedef struct socblk  SOCBLK;
typedef struct typeblk TYPEBLK;

struct  idb
{   THDDATA tdb;
    IDB    *next;
    long    instance;
    long    pcchndl;
    char    pccname[20];
    int     numsoc;
    SOCBLK *soctbl[1];
};

struct socblk
{   int      num;
    TYPEBLK *curtype;
    TYPEBLK *firsttype;
};

struct typeblk
{   TYPEBLK *next;
    int      type;
    void   (*remove)(IDB *idb, SOCBLK *soc);
    char     name[20];
    char     dosnames[12];
    int      unit;
    int      index;
    int      ioreg;
    int      intreq;
    int      cfgreg;
    int      cfgval;
};

typedef struct intdata
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
    long  socket;
    IDB  *idb;
} INTDATA;


typedef struct adrent
{   int    addr;		// Base address
    int    length;		// Length of window
} ADRENT;
    
typedef struct cfgent
{   int    type;		// Card type (1 = IO, 2 = memory)
    int    cfgval;		// Value for configuration register
    int    decode;		// Number of address lines decoded
    int    width;		// Bus width
    int    numadrs;		// Number of address ranges defined
    ADRENT adrtbl[4];
} CFGENT;

typedef struct cfg
{   int    cfgoffset;		// CIS offset of the configuration register
    int    cardtype;		// Card type (from the FUNCID tuple)
    int    numcfgs;		// Number of configuration entries defined
    CFGENT cfgtbl[5];		// Table of configuration entries
} CFG;


extern CFG     cfg;
extern CFGENT  dftcfg;
extern CFGENT *curcfg;

extern char    bufr[];

extern char  class[];
extern char  type[];
extern char  dosnames[];
extern int   index;
extern int   unit;
extern int   ioreg;
extern int   intreq;
extern int   pccardsoc;
extern char  pccardname[];

void     devicedisk(IDB *idb, SOCBLK *soc);
void     fail(IDB *idb, long code, char *msg);
TYPEBLK *findtype(int type, SOCBLK *socblk);
int      getconfig(IDB *idb, int socket);
void    *getmem(int size);
void     logstring(char *str);
void     pccsetup(IDB *idb, SOCBLK *soc);
void     setupdisk(IDB *idb, SOCBLK *soc);
int      setupiocard(IDB *idb, int soc, CFGENT *cfgent, int ioreg1,
    int iosize1, int ioreg2, int iosize2, int intreq);
