
#pragma aux XWSCALL "*" parm [] reverse routine modify [EAX EBX ECX EDX] \
	value [EAX];


typedef struct rgn    RGN;
typedef struct rsb    RSB;
typedef struct drw    DRW;
typedef struct win    WIN;
typedef struct edb_s  EDB;
typedef struct efte   EFTE;
typedef struct not    NOT;
typedef struct cur    CUR;
typedef struct par    PAR;
typedef struct fnb    FNB;
typedef struct fsb    FSB;
typedef struct fnt    FNT;
typedef struct gbm    GBM;
typedef struct fm     FM;
typedef struct rect   RECT;
typedef struct insets INSETS;
typedef struct bitmap BITMAP;


extern WIN *xws_ScreenWIN;
extern DRW  xws_ScreenDRW;
extern PAR  xws_ParContainerDefault;
extern PAR  xws_ParContainerPlain;
extern PAR  xws_ParListDefault;
extern PAR  xws_ParButtonDefault;
extern PAR  xws_ParEditDefault;

extern long xws_TrmHndl;

extern CUR *xws_CurFinger;
extern CUR *xws_CurPoint;
extern CUR *xws_CurMove;
extern CUR *xws_CurSizeNS;
extern CUR *xws_CurSizeEW;
extern CUR *xws_CurSizeNESW;
extern CUR *xws_CurSizeNWSE;

extern long xws_TotalMem; 
extern long xws_TotalBlk;
extern long xws_AllocCnt;
extern long xws_FreeCnt;


typedef void XWSREDRAW(WIN *win);
#pragma aux (XWSCALL) XWSREDRAW;

typedef long XWSEVENT(WIN *win, long type, long arg1, long arg2, long arg3,
    long arg4);
#pragma aux (XWSCALL) XWSEVENT;

typedef void XWSTIMER(void);


long   xwsDrawChar(WIN *win, long chr);
#pragma aux (XWSCALL) xwsDrawChar;

long   xwsDrawCharXY(WIN *win, long chr, long xpos, long ypos);
#pragma aux (XWSCALL) xwsDrawCharXY;

void   xwsDrawCircle(WIN *win, long xpos, long ypos, long radius);
#pragma aux (XWSCALL) xwsDrawCircle;

void    xwsBitmapFree(BITMAP *bitmap);
#pragma aux (XWSCALL) xwsBitmapFree;

void    xwsBitmapDestroy(BITMAP *bitmap);
#pragma aux (XWSCALL) xwsBitmapDestroy;

BITMAP *xwsBitmapLoadBMP(char *file, WIN *win, long bits);
#pragma aux (XWSCALL) xwsBitmapLoadBMP;

void   xwsCtlMain(void);
#pragma aux (XWSCALL) xwsCtlMain;

void   xwsCtlUpdate(void);
#pragma aux (XWSCALL) xwsCtlUpdate;

CUR   *xwsCurCreate(long xsize, long ysize, long xhot, long yhot, long *colors,
    long num ,uchar *patbm, uchar *maskbm);
#pragma aux (XWSCALL) xwsCurCreate;

void   xwsCurSetPos(long xpos, long ypos);
#pragma aux (XWSCALL) xwsCurSetPos;

long   xwsDrawLine(WIN *win, long x1, long y1, long x2, long y2);
#pragma aux (XWSCALL) xwsDrawLine;

long   xwsDrawText(WIN *win, char *text, long length);
#pragma aux (XWSCALL) xwsDrawText;

long   xwsDrawTextXY(WIN *win, char *text, long length, long xpos, long ypos);
#pragma aux (XWSCALL) xwsDrawTextXY;

long   xwsFillRect(WIN *win, long xpos, long ypos, long xsize, long ysize);
#pragma aux (XWSCALL) xwsFillRect;

int    xwsFontGetLength(FNT *fnt, char *text, long length);
#pragma aux (XWSCALL) xwsFontGetLength;

void   xwsFontGetMetrics(FNT *fnt, FM *fm);
#pragma aux (XWSCALL) xwsFontGetMetrics;

FNT   *xwsFontLoad(char *name, long height, long width, long attr);
#pragma aux (XWSCALL) xwsFontLoad;

//@@@ CEY (pragma was for xwsInitS3A!!!)
void   xwsInitATIB(void);
#pragma aux (XWSCALL) xwsInitATIB;

void   xwsInitVesaA(void);
#pragma aux (XWSCALL) xwsInitVesaA;

void   xwsInitBegin(char *trmname, long vectbase, long horiz);
#pragma aux (XWSCALL) xwsInitBegin;

void   xwsInitCursor(void);
#pragma aux (XWSCALL) xwsInitCursor;

void   xwsInitLCDA(void);
#pragma aux (XWSCALL) xwsInitLCDA;

void   xwsInitMouse(void);
#pragma aux (XWSCALL) xwsInitMouse;

void   xwsInitS3A(void);
#pragma aux (XWSCALL) xwsInitS3A;

void   xwsInitScreen(void);
#pragma aux (XWSCALL) xwsInitScreen;

void   xwsInitStart(PAR *parms, XWSREDRAW redraw, XWSEVENT event, EDB *edb);
#pragma aux (XWSCALL) xwsInitStart;

long   xwsTimerStart(XWSTIMER, long data, long vector, long interval);
#pragma aux (XWSCALL) xwsTimerStart;

void   xwsTimerStop(long handle);
#pragma aux (XWSCALL) xwsTimerStop;

RGN   *xwsRgnCreateRect(long xpos, long ypos, long xsize, long ysize);
#pragma aux (XWSCALL) xwsRgnCreateRect;

void   xwsRgnDestroy(RGN *rgn);
#pragma aux (XWSCALL) xwsRgnDestroy;

void   xwsWinClear(WIN *win);
#pragma aux (XWSCALL) xwsWinClear;

WIN   *xwsWinCreateButton(WIN *parent, long xpos, long ypos, long xsize,
	    long ysize, PAR *parms, XWSREDRAW redraw, XWSEVENT event, long evmask,
	    EDB *edb, char *text, long length);
#pragma aux (XWSCALL) xwsWinCreateButton;

WIN   *xwsWinCreateContainer(WIN *parent, long xpos, long ypos, long xsize,
	    long ysize, PAR *parms, XWSREDRAW redraw, XWSEVENT event, long evmask,
	    EDB *edb);
#pragma aux (XWSCALL) xwsWinCreateContainer;

WIN   *xwsWinCreateEdit(WIN *parent, long xpos, long ypos, long xsize,
	    long ysize, PAR *parms, XWSREDRAW redraw, XWSEVENT event, long evmask,
	    EDB *edb, char *text, long length, long bits);
#pragma aux (XWSCALL) xwsWinCreateEdit;

WIN   *xwsWinCreateList(WIN *parent, long xpos, long ypos, long xsize,
	    long ysize, PAR *parms, XWSREDRAW redraw, XWSEVENT event, long evmask,
	    EDB *edb);
#pragma aux (XWSCALL) xwsWinCreateList;

WIN   *xwsWinCreateResponse(WIN *parent, long xpos, long ypos, long xsize,
	    long ysize, PAR *parms, XWSREDRAW redraw, XWSEVENT event, long evmask,
	    EDB *edb, char *text, long length);
#pragma aux (XWSCALL) xwsWinCreateResponse;

void   xwsWinDestroy(WIN *win);
#pragma aux (XWSCALL) xwsWinDestroy;

void   xwsWinDispBitmap(WIN *win, BITMAP *bitmap, long xpos, long ypos,
		long width, long height, long xrept, long yrept, long func);
#pragma aux (XWSCALL) xwsWinDispBitmap;

void   xwsWinDispText(WIN *win);
#pragma aux (XWSCALL) xwsWinDispText;

void   xwsWinGetBndBox(WIN *win, RECT *rect);
#pragma aux (XWSCALL) xwsWinGetBndBox;

uchar *xwsWinGetBuffer(WIN *win);
#pragma aux (XWSCALL) xwsWinGetBuffer;

void   xwsWinGetClient(WIN *win, RECT *rect);
#pragma aux (XWSCALL) xwsWinGetClient;

void   xwsWinGetInsets(WIN *win, INSETS *insets);
#pragma aux (XWSCALL) xwsWinGetInsets;

void   xwsWinGetOrigin(WIN *win, long *data);
#pragma aux (XWSCALL) xwsWinGetOrigin;

void  *xwsWinGetUser(WIN *win);
#pragma aux (XWSCALL) xwsWinGetUser;

void   xwsWinInvalidateAll(WIN *win);
#pragma aux (XWSCALL) xwsWinInvalidateAll;

void   xwsWinInvalidateRect(WIN *win, long xpos, long ypos, long width,
    long height);
#pragma aux (XWSCALL) xwsWinInvalidateRect;

void   xwsWinInvalidateRgn(WIN *win, RGN *rgn);
#pragma aux (XWSCALL) xwsWinInvalidateRgn;

WIN   *xwsWinMemory(long width, long height, long type, PAR *parms,
    XWSREDRAW redraw, XWSEVENT event, EDB *edb);
#pragma aux (XWSCALL) xwsWinMemory;

void   xwsWinMove(WIN *win, long xpos, long ypos, long width, long height);
#pragma aux (XWSCALL) xwsWinMove;

void   xwsWinRedrawBegin(WIN *win);
#pragma aux (XWSCALL) xwsWinRedrawBegin;

void   xwsWinRedrawEnd(WIN *win);
#pragma aux (XWSCALL) xwsWinRedrawEnd;

void   xwsWinSetBgColor(WIN *win, long bgcol);
#pragma aux (XWSCALL) xwsWinSetBgColor;

void   xwsWinSetFgColor(WIN *win, long fgcol);
#pragma aux (XWSCALL) xwsWinSetFgColor;

//@@@ CEY
long	xwsWinSetFocus( WIN * win, int nMode );
#pragma aux (XWSCALL) xwsWinSetFocus;

void   xwsWinSetFont(WIN *win, FNT *fnt);
#pragma aux (XWSCALL) xwsWinSetFont;

void   xwsWinSetOriginAbs(WIN *win, long xorg, long yorg);
#pragma aux (XWSCALL) xwsWinSetOriginAbs;

void   xwsWinSetOriginRel(WIN *win, long xorg, long yorg);
#pragma aux (XWSCALL) xwsWinSetOriginRel;

void   xwsWinSetSelItem(WIN *win, long item);
#pragma aux (XWSCALL) xwsWinSetSelItem;

void   xwsWinSetUser(WIN *win, void *user);
#pragma aux (XWSCALL) xwsWinSetUser;

void   xwsWinScrollRgn(WIN *win, RGN *rgn, long xdist, long ydist);
#pragma aux (XWSCALL) xwsWinScrollRgn;

void   xwsStoreTextAdd(WIN *win, char *text, long count, long xpos, long ypos);
#pragma aux (XWSCALL) xwsStoreTextAdd;

void   xwsStoreTextIns(WIN *win, char *text, long count, long xpos, long ypos,
    long line, long index);
#pragma aux (XWSCALL) xwsStoreTextIns;

void   xwsStoreTextNew(WIN *win, char *text, long count, long xpos, long ypos,
    long bits);
#pragma aux (XWSCALL) xwsStoreTextNew;

// Define structure for the EDB (Environment Data Block)

//@@@ CEY for C++...
struct edb_s
{
	long  numfonts;		// Number of fonts in the font table
    EFTE *fonttbl;		// Offset of the font table
    long  numcols;		// Number of colors in the color table
    long *coltbl;		// Offset of the color table
};

// Define structure for the EFTE (Environment Font Table Entry)

struct efte
{
	char *name;			// Name of the font
    FNT  *fnt;			// FNT object for the font
};

// Define the PAR structure which is used to specify parameters when
//   creating windows

struct par
{   long   zorder;				// Z-order value
    ushort textsize;			// Text size
    ushort textwidth;			// Text width
    ushort textfont;			// Text font
    ushort textattr;			// Text attributes
    long   leftmargin;			// Left margin
    long   rightmargin;			// Right margin
    long   topmargin;			// Top margin
    long   btmmargin;			// Bottom margin
    long   lspace;				// leading
    long   pspace;				// Paragraph spacing
    long   fgcolorn;			// Normal foreground (text) color
    long   fgcolora;			// Alternate foreground (text) color
    long   fgcolorx;			// Extra foreground (text) color
    long   bgcolorn;			// Normal background color
    long   bgcolora;			// Alternate background color
    long   brdcolor;			// Border color
    long   hilcolorn;			// Normal highlight color
    long   hilcolora;			// Alternate highlight color
    long   shdcolorn;			// Nornal shading color
    long   shdcolora;			// Alternate shading color
    long   brdwidtho;			// Outer border width
    long   brdwidthc;			// Center border width
    long   brdwidthi;			// Inner border width
    long   bits1;				// Window bits 1
    long   bits2;				// Window bits 2
    long   bits3;				// Window bits 3
    long   bits4;				// Window bits 4
    long   xradius;				// X radius
    long   yradius;				// Y radius
    long   poffset;				// Pressed button offset
    long   orient;				// Orientation
    ushort format;				// Format
    ushort inc1size;			// Increment 1 button size
    ushort inc2size;			// Increment 2 button size
    ushort digits;				// Number of digits
    long   maxvalue;			// Maximum value
    long   minvalue;			// Minimum value
    long   inc1value;			// Increment 1 value
    long   inc2value;			// INcrement 2 value
    CUR   *cursor;				// Offset of CUR for cursor for window
    long   sbxsize;				// Scroll bar X size
    long   sbysize;				// Scroll bar Y size
    long   sbxbutsize;			// Scroll bar X button size
    long   sbybutsize;			// Scroll bar Y button size
    long   sbshdwidth;			// Scroll bar shading width
    long   sbfgcolorn;			// Scroll bar normal forground (text) color
    long   sbfgcolora;			// Scroll bar alternate forgournd (text) color
    long   sbbgcolorn;			// Scroll bar normal background color
    long   sbbgcolora;			// Scroll bar alternate background color
    long   sbbarcolor;			// Scroll bar bar color
    long   sbhilcolorn;			// Scroll bar normal highlight color
    long   sbhilcolora;			// Scroll bar alternate highlight color
    long   sbshdcolorn;			// Scroll bar normal shadow color
    long   sbshdcolora;			// Scroll bar alternate shadow color
    long   actfunc;				// Activation function
    long   actwindow;			// Activation window
    long   titleSize;			// Title bar size
    long   titleBits;			// Title bar bits
    long   titleText;			// Title bar text
    long   bgbitmapn;			// Normal background bitmap
    long   bgbitmapa;			// Alternate background bitmap
    long   brdbitmap;			// Border bitmap
};

// Define the FM (Font Metrics) structure

struct fm
{   char  name[32];		// Name of font
    long  flags;		// Flag bits
    long  height;		// Height (ascent + descent)
    long  avgwidth;		// Average width
    long  maxwidth;		// Maximum width
    long  ascent;		// Ascent
    long  descent;		// Descent
    long  inleading;		// Internal leading
    long  exleading;		// External leading
    long  attr;			// Attributes
    long  underpos;		// Underscore position
    long  strikepos;		// Strike-through position
};

// Define bits for the flags item in FM

#define FMBITS_TTF    0x00000004 // True-type font
#define FMBITS_MONO   0x00000002 // Monospaced font
#define FMBITS_ITALIC 0x00000001 // Italic font

// Define event types

#define EVENT_MOUSEMV 1		// Mouse movement
#define EVENT_MOUSEBT 2		// Mouse button
#define EVENT_KEYMAKE 3		// Key pressed event
#define EVENT_KEYRELS 4		// Key released event
#define EVENT_MOVE    8		// Window has been moved
#define EVENT_RESIZE  9		// Window size has been changed
#define EVENT_DESTROY 10	// Window is about to be destroyed
#define EVENT_CHILD   12	// Child window event
#define EVENT_BMLOAD  16	// Bitmap load update

// Define event mask bits

#define EVMASK_MOUSEMV 0x00000001 // Mouse movement
#define EVMASK_MOUSEBT 0x00000002 // Mouse button
#define EVMASK_KEYMAKE 0x00000004 // Key pressed event
#define EVMASK_KEYRELS 0x00000008 // Key released event
#define EVMASK_MOVE    0x00000100 // Window has been moved
#define EVMASK_RESIZE  0x00000200 // Window size has been changed
#define EVMASK_DESTROY 0x00000400 // Window is about to be destroyed
#define EVMASK_CHILD   0x00001000 // Child window event
#define EVMASK_BMLOAD  0x00010000 // Bitmap load update

// Define argument 1 values for EVENT_MOUSEBT events

#define MOUSEBT_LDN 1		// Left mouse button pressed
#define MOUSEBT_LUP 2		// Left mouse button released
#define MOUSEBT_CDN 3		// Center mouse button pressed
#define MOUSEBT_CUP 4		// Center mouse button released
#define MOUSEBT_RDN 5		// Right mouse button pressed
#define MOUSEBT_RUP 6		// Right mouse button released

// Define bits for argument 3 for EVENT_KEYMAKE and EVENT_KEYRELS events

#define KBE3_RELEASE 0x8000	// Key-release code
#define KBE3_CAPKEY  0x4000	// Caps-lock key pressed for character
#define KBE3_NUMKEY  0x2000	// Num-lock key pressed for chacacter
#define KBE3_SCLKEY  0x1000	// Scroll-lock key pressed for character
#define KBE3_CAPLCK  0x0400	// Caps-lock state true for character
#define KBE3_NUMLCK  0x0200	// Num-lock state true for chacacter
#define KBE3_SCLLCK  0x0100	// Scroll-lock state true for character
#define KBE3_LFTALT  0x0040	// Left ALT key pressed for character
#define KBE3_LFTCTL  0x0020	// Left control key pressed for character
#define KBE3_LFTSHF  0x0010	// Left shift key pressed for character
#define KBE3_RHTALT  0x0004	// Right ALT key pressed for character
#define KBE3_RHTCTL  0x0002	// Right control key pressed for character
#define KBE3_RHTSHF  0x0001	// Right shift key pressed for character

// Define argument 1 values for EVENT_CHILD events

#define CHILD_BUTSGL  1		// Button single click
#define CHILD_BUTDBL  2		// Button double click
#define CHILD_LSTSEL  3		// List item selected
#define CHILD_LSTENT  4		// List item entered
#define CHILD_LSTREL  5		// List item released
#define CHILD_LSTDBL  6		// List item double clicked

// Define the RECT structure which is used to specify the position and size
//   of a rectangle

struct rect
{   long  xpos;
    long  ypos;
    long  width;
    long  height;
};

// Define the INSETS structure which is used to specify the widths of window
//   borders

struct insets
{   long  left;
    long  right;
    long  top;
    long  bottom;
};

#define FNTA_XBOLD  0x1000	// Extra bold
#define FNTA_BOLD   0x0800	// Bold
#define FNTA_DBOLD  0x0400	// Demi-bold
#define FNTA_NORMAL 0x0200	// Normal
#define FNTA_LIGHT  0x0100	// Light
#define FNTA_ITALIC 0x0001	// Italic
