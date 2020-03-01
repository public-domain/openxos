#define uchar  unsigned char
#define ushort unsigned short
#define uint   unsigned int
#define ulong  unsigned long

#define defbuffer ((char *)0x300000)

#define LKEML_INFO     1
#define LKEML_ERROR    2
#define LKEML_FININFO  3
#define LKEML_FINERROR 4

typedef struct lkeargs
{   ulong  lla_state;		// us State
    ulong  lla_value;		//  s Returned value
    ulong  lla_doffset;		//  s Offset for data msect
    ulong  lla_dsize;		//  s Final size for data msect
    ulong  lla_dcount;		// u  Size of data msect data
    void   far *lla_ddata;	// u  Address of data for data msect
    ushort filler1;
    ulong  lla_coffset;		//  s Offset for code msect
    ulong  lla_csize;		//  s Final size for code msect
    ulong  lla_ccount;		// u  Size of code msect data
    void   far *lla_cdata;	// u  Address of data for code msect
    ushort filler2;
    ulong  lla_init;		// u  Offset of initialization routine
    void   far *lla_char;	// u  Address of characteristics list
    ushort filler3;
    ulong  lla_xcount;		// u  Size of exported symbol table data
    void   far *lla_xdata;	// u  Address of exported symbol table data
    ushort filler4;
    ulong  lla_soffset;		//  s Offset for debugger symbol table msect
    ulong  lla_ssize;		//  s Final size for debugger symbol table msect
    ulong  lla_scount;		// u  Size of debugger symbol table data
    void   far *lla_sdata;	// u  Address of debugger symbol table data
    ushort filler5;
} LKEARGS;

// Following functions must be defined by the caller of lkeloadf

void  message(int level, char *text);

// Following functions are defined in LKELOADF.C

void  decput(ulong value);
char *getmem(long size);
int   lkeloadf(int quiet, struct lkechar *addbfr);
void  strput(char *str);
