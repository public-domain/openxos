// Structure for file specification blocks

typedef struct fs FS;
struct  fs
{   FS   *next;			// Pointer to next independent file spec
    short pathlen;		// Length of path part of file specification
    char  spec[4];		// File specification
};

struct inparms
{   byte4_parm pos;
    char       end;
};

extern struct inparms inparms;

extern CDH   cdheader;
extern LFH   lfheader;

extern long  crcvalue;

extern long  zipdev;
extern long  filedev;
extern char *inbufr;
extern char *outbufr;
extern long  bufrsize;

extern char  filename[512];
extern char  dstname[512];

extern char *errtext;
extern char *errname;

extern long  totalsize;
extern long  totalcomsize;
extern long  numfiles;

void *getmem(size_t size);
long  inflatefile(int xfer);
long  unstorefile(int xfer);
