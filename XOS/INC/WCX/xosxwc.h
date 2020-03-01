#pragma aux XWCCALL "*" parm [] reverse routine modify [EAX EBX ECX EDX] \
	value [EAX];


typedef struct _xwccon
{   void *pnt;
} XWCCON;

typedef struct _xwcwin
{   void *pnt;
} XWCWIN;


typedef void XWCREDRAW(XWCCON *con, XWCWIN *win, int xpos, int ypos, int width,
    int height);
#pragma aux (XWCCALL) XWCREDRAW;

typedef long XWCEVENT(XWCCON *con, XWCWIN *win, long type, long arg1, long arg2,
    long arg3, long arg4);
#pragma aux (XWCCALL) XWCEVENT;

XWCCON *xwcInit(int prot, char*addr, int port, int level, int vect);
#pragma aux (XWCCALL) xwcInit;


XWCWIN *xwcWinCreateButton(XWCWIN *parent, long xpos, long ypos, long xsize,
    long ysize, void *parms, XWCREDRAW redraw, XWCEVENT event, long evmask,
    char *text, long length);
#pragma aux (XWCCALL) xwcWinCreateButton;

XWCWIN *xwcWinCreateContainer(XWCWIN *parent, long xpos, long ypos, long xsize,
    long ysize, void *parms, XWCREDRAW redraw, XWCEVENT event, long evmask);
#pragma aux (XWCCALL) xwcWinCreateContainer;

XWCWIN *xwcWinCreateEdit(XWCWIN *parent, long xpos, long ypos, long xsize,
    long ysize, void *parms, XWCREDRAW redraw, XWCEVENT event, long evmask,
    char *text, long length, long bits);
#pragma aux (XWCCALL) xwcWinCreateEdit;

XWCWIN *xwcWinCreateList(XWCWIN *parent, long xpos, long ypos, long xsize,
    long ysize, void *parms, XWCREDRAW redraw, XWCEVENT event, long evmask);
#pragma aux (XWCCALL) xwcWinCreateList;

XWCWIN *xwcWinCreateResponse(XWCWIN *parent, long xpos, long ypos, long xsize,
    long ysize, void *parms, XWCREDRAW redraw, XWCEVENT event, long evmask,
    char *text, long length);
#pragma aux (XWCCALL) xwcWinCreateResponse;

void   xwcWinDestroy(XWCWIN *win);
#pragma aux (XWCCALL) xwcWinDestroy;

