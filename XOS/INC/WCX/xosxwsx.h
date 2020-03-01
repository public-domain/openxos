typedef struct tb TB;
typedef struct tl TL;

extern uchar screenbufr[];	// Offset of the display screen buffer
extern uchar rgnbufr[];		// Offset of the temporary region buffer

#define RGNBSZ 0x40000		// Maximum size of the temporary region buffer


typedef FNT *XWSFNTLOAD(char *name, long height, long width, long attr);
#pragma aux (XWSCALL) XWSFNTLOAD;

typedef void XWSFNTUNLOAD(FNT *fnt);
#pragma aux (XWSCALL) XWSFNTUNLOAD;

typedef GBM *XWSFNTGETGLYPH(FNT *fnt, int chr);
#pragma aux (XWSCALL) XWSFNTGETGLYPH;


XWSFNTLOAD xwsftfload;
XWSFNTLOAD xwsbmfload;

long  dispchar(WIN *win, FNT *fnt, long chr, long xpos, long ypos, long fgcol,
    long bgcol);
#pragma aux (XWSCALL) dispchar;

void  xwsfree(void *);
void *xwsmalloc(int size);

FNB  *xwsmakefnb(char *name, int extra);
FSB  *xwsgetfsb(FNB *fnb, int height, int width);


// Define vectors (offset from our base vector)

#define VECT_CTLP  0
#define VECT_MOUSE 1
#define VECT_IPM   2

// Define structure for the RGN (region object)

struct rgn
{   ulong  id;			// ID = 'RGN*'
    ushort user;
    uchar  type;		// Region type
    uchar  res1[1];		// Reserved
    long   size;		// Total size of RGN (bytes)
    long   usecnt;		// Use count (number of users - 1)
    long   ycount;		// Number of Y index table entries
    long   left;		// Left side X position
    long   right;		// Right side X position
    long   top;			// Top Y position
    long   bottom;		// Bottom Y position
    RSB   *ytable[1];		// Start of the Y index table
};

// Define structure for each RSB (Region Segment Block)

struct rsb
{   long left;			// Left side
    long right;			// Right side
};

// Define values for rgn.type

#define RGNTYPE_NULL     0	// Null region
#define RGNTYPE_SIMPLE   1	// Simple region
#define RGNTYPE_COMPLEX  2	// Complex region

// Define structure for the DRW (drawing context object)

struct drw
{   ulong  id;			// ID
    uchar  type;		// Type
    uchar  disable;		// Non-zero if disabled
    uchar  res1[2];		// Reserved
    uchar *buffer;		// Offset of start of display buffer
    long   bufsize;		// Size of buffer (bytes)
    long   width;		// Width (pixels)
    long   height;		// Height (pixels)
    long   pixelspan;		// Span between lines (pixels)
    long   bytespan;		// Span between lines (bytes)
    long   pixelsize;		// Pixel size (bytes)
    long   waitidle;		// Offset of wait until idle routine
    long   rgnfill;		// Offset of region fill routine
    long   rgncopy;		// Offset of region copy routine
};

// Define values for drw_type

#define DRWTYPE_DISPLAY 1	// Physical display
#define DRWTYPE_MEMORY  2	// Memory

// Define structure for common part of the WIN (window object)

struct winbas
{   ulong  id;			// ID
    ushort user;
    ushort xxx;
    DRW   *drw;			// Pointer to DRW for window
    long   offset;		// Offset of window in its display buffer
    WIN   *parent;		// Offset of WIN for parent window
    WIN   *fchild;		// Offset of WIN for first child window
    WIN   *sibling;		// Offset of WIN for next sibling window
    uchar  type;		// Window type
    uchar  moving;
    uchar  redrawflg;
    uchar  xxx1;
    long   xpos;		// X position in parent window
    long   ypos;		// Y position in parent window
    long   width;		// Width
    long   height;		// Height
    long   scnxpos;		// X position on screen
    long   scnypos;		// Y position on screen
    long   zorder;		// Z-order (larger value is toward viewer)
    long   xorg;
    long   yorg;
    long   xclient;		// Width of client area
    long   yclient;		// Height of client area
    long   xleft;		// Width of left border area
    long   xright;
    long   ytop;		// Height of top border area
    long   ybottom;
    long   bits1;		// Window bits 1
    long   bits2;		// Window bits 2
    long   bits3;		// Window bits 3
    long   bits4;		// Window bits 4
    long   evmask;		// Event mask bits
    long   status;		// Window status bits
    long   cursor;		// Offset of CUR for cursor for window

    long   mvxofs;		// X offset for cursor when dragging window
    long   mvyofs;		// Y offset for cursor when dragging window

    WIN   *redrawnext;		// Offset of next WIN in the redraw list

    RGN   *basergn;		// Offset of RGN for base region for window (this
				//   region covers the entire window)
    RGN   *clientrgn;		// Offset of RGN for client region for window
    RGN   *userrgn;		// Offset of RGN for user specified clipping
				//   region (this is initially a copy of
				//   basclientrgn and is always a subset of
				//   basclientrgn)
    RGN   *maskrgn;		// Offset of RGN for currently visible part of
				//   the window (this does not exclude the
				//   parts not in the clipping region)

    RGN   *cliprgn;		// Offset of current clipping region RGN (this
				//   is the intersection of basmaskrgn and
				//   basuserrgn - it is used to clip
				//   everything drawn to the window)
    RGN   *redrawrgn;		// Offset of RGN for current redraw region

    RGN   *savcliprgn;		// Offset of saved clipping region RGN when in
				//   redraw mode
    WIN   *prevmodal;		// Previous modal window (bit 31 set to indicate
				//   previous focus, not modal window)
    long   texthead;
    long   textlast;
    TL    *bashlbgnline;
    long   bashlbgnpos;
    ushort bashlbgnchar;
    ushort bashlbgnindex;
    TL    *bashlendline;
    long   bashlendpos;
    ushort bashlendchar;
    ushort bashlendindex;

    long   curxpos;		// Current X drawing/text position
    long   scurypos;		// Current Y drawing/text position
    long   curfgcol;		// Current drawing/text foreground color
    long   curbgcol;		// Current drawing/text background color
    FNT   *scurfnt;		// Offset of FNT for current font

    short  textfont;
    short  textsize;
    short  textwidth;
    short  xxxx;

    long   textcoln;
    long   textcola;
    long   bgcoln;		// Normal background color
    long   bgcola;		// Alternate background color
    long   bgbmn;		// Normal background bitmap
    long   bgbma;		// Alternate background bitmap
    long   brdrwo;		// Outer border width
    long   brdrwc;		// Center border width
    long   brdrwi;		// Inter border width
    long   shdcoln;		// Normal border shadow color
    long   shdcola;		// Alternate border shadow color
    long   hilcoln;		// Normal border highlight color
    long   hilcola;		// Alternate border highlight color
    long   brdrcol;		// Center border color
    long   brdrbm;		// Center border bitmap
    uchar  pressofs;
    uchar  yyyy[3];
    NOT   *fnot;		// First notify object
    EDB   *edb;
    long   usrredraw;		// Offset of user redraw function
    long   sysevent;		// Offset of system event function
    long   usrevent;		// Offset of user event function
};

struct win
{
    struct winbas bas;
};

// Define values for type in WIN

#define WINTYPE_CONTAINER 1
#define WINTYPE_BUTTON    2
#define WINTYPE_CHECKBOX  3
#define WINTYPE_LIST      4
#define WINTYPE_SELECT    5
#define WINTYPE_VALUE     6
#define WINTYPE_RESPONSE  7
#define WINTYPE_EDIT      8


// Define structure for the NOT (notify object)

struct not
{   ulong id;			// ID
    void *not;			// The notifier object
    NOT  *nextnot;		// Next notify object for same notifier
    NOT  *nextwin;		// Next notify object for same window
    WIN  *win;			// This window object
    NOT **head;			// Offset of head pointer for this notify list
};

// Define structure for the CUR (cursor object)

struct cur
{   ulong  id;			// ID
    ushort user;
    ushort xxx;
    long   xsize;		// X size
    long   ysize;		// Y size
    long   xhot;		// X hot-spot position
    long   yhot;		// Y hot-spot position
    long   color1;		// First color
    long   color2;		// Second color
    uchar *patbm;		// Pointer to pattern bitmap
    uchar *maskbm;		// Pointer to mask bitmap
};

// Define structure for the FNB (Font Name Block)

struct fnb
{   ulong id;			// ID
    FNB    *next;		// Offset of next FNB
    FSB    *fbmfsb;		// Offset of first bit-mapped font FSB
    FSB    *fftfsb;		// Offset of first FreeType font FSB

    TT_Face ftface;

    int     ftfcnt;
    uchar   ftcmap;		// FreeType character map index
    uchar   file;		// Offset of file name from font name
    char    name[1];		// Font name
};

// Define structure for the FSB (Font Size Block)

struct fsb
{   ulong id;			// ID
    FSB  *next;			// Offset of next FSB for same font
    FNB  *fnb;
    FNT  *ffnt;			// Offset of first FNT
    ulong height;		// Height of font (pixels)
    ulong width;		// Average width of font (pixels)
};

// Define structure for the FNT (font object)

struct fnt
{   ulong  id;			// ID
    ushort user;
    ushort xxx;
    FNT   *next;		// Pointer to next FNT from same font size
    FSB   *fsb;			// Pointer to FSB
    long   ascent;		// Ascent (pixels)
    long   descent;		// Descent (pixels)
    long   inleading;		// Internal leading (pixels)
    long   exleading;		// External leading (pixels)
    long   height;		// Height (pixels) (must be ascent + descent
				//   + inleading)
    long   bmheight;		// Bit-map height
    long   bmascent;		// Bit-map ascent
    long   maxwidth;		// Maximum width
    long   avgwidth;		// Average width
    long   attr;		// Attributes
    union
    {
	ulong      length;	// Length of the data to load
	TT_CharMap ftcharmap;
    };
    union
    {
	ulong       offset;	// Offset in file of data to load
	TT_Instance ftinstance;
    };    
    long   underpos;		// Underscore position
    long   undersize;		// Underscore position
    long   strikepos;		// Strike-out position
    long   strikesize;		// Strike-out size
    union
    {
	ulong    numchars;	// Number of characters
	TT_Glyph ftglyph;
    };
    uchar  firstchar;		// First character
    uchar  lastchar;		// Last character
    uchar  dfltchar;		// Default character
    uchar  flags;		// Flag bits:
				//   Bit 0: 1 if italic font
				//   Bit 1: 1 if monospaced font
				//   Other bits are reserved and must be 0
    long   loadcnt;		// Load count
    GBM  **table;		// Pointer to glyph pointer table (NULL if font
				//   not loaded)
    XWSFNTUNLOAD   *unldfont;	// Function to unload font
    XWSFNTGETGLYPH *getglyph;	// Function to get character glyph
    union
    {
	char  filename[1];	// Name of the font file
	void *ftgtbl[256];
    };
};



struct gbm
{   ushort width;
    ushort advance;
    ushort kerning;
    uchar  data[2];
};

extern FNB   *xwsfirstfnb;

extern TRMMODES xwstrmdata;

extern ushort xwsscnnum;
extern ushort xwstrmnum;
extern long   xwsvectbase;
extern CUR   *xwscursorcur;
extern DRW    xwsscreendrw;

void xwsupdatecursor(void);
