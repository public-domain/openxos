#define FILEBUFR (64*1024)
#define OUTBUFR (64*1024)

// Structure for file specification blocks

typedef struct _fs FS;
struct  _fs
{   FS   *next;			// Pointer to next FS
	uchar wild;
    char  name[4];		// File name
};

typedef struct _ps PS;
struct _ps
{	PS   *next;			// Pointer to next PS
	FS   *file;			// Pointer to first FS for this PS
	FS   *exclude;		// Pointer to first exclude FS for this PS
	uchar notrel;		// Path is not relative
	char  path[4];		// Path specification
};

typedef struct _fi FI;
struct _fi
{	FI    *next;		// Pointer to next FI
	FI    *link;		// Link used when sorting
	ushort skip;
	uchar  attrib;
	uchar  xxx;
	long   offset;
	LFH    lfh;
	char   spec[4];
};

typedef struct
{   byte4_parm pos;
    uchar      end;
} ZFOUTPARMS;

extern ZFOUTPARMS zfoutparms;

extern char  prgname[];

extern FI   *fipnt;

extern long  level;

extern char *filebufr;
extern char *outbufr;

extern long  zfhndl;
extern long  ifhndl;

extern long  totalsize;
extern long  totalcomsize;
extern long  numfiles;

void *getmem(size_t size);
int   deflatefile(void);
void  fileerror(char *msg, long code);
long  ratio(long num1, long num2);
int   storefile(void);
