//-------------------------------------------------------------------------*
// PROGHELP.C
// Programmer's help utilities for XOS
//
// Written by: SA Ortmann
//
// Edit History:
// 12/09/94(sao) - Initial development
// 02/03/95(brn) - Clean up formatting and change default colors
// 02/06/95(sao) - Modified screen & kbd handling to support ANSI TRMs
// 02/23/95(sao) - Expanded display area, added 'more' indicators, prevent
//					scrolling into last page, added 'no color' (gpc_ANSIClr)
//					support, added support for variable length header area.
// 02/28/95(sao) - Added support for both an option and keyword table.
//					Fixed reported build date
// 05/15/95(sao) - Modified keyboard commands, eliminate WS support, Ctrl-C
//					breaks.
// 18May95 (fpj) - Changed name from progasst to proghelp.
// 08/28/96(brn) - Fix getTrmParms to set default values if no display
//-------------------------------------------------------------------------*

// ++++
// This software is in the public domain.  It may be freely copied and used
// for whatever purpose you see fit, including commerical uses.  Anyone
// modifying this software may claim ownership of the modifications, but not
// the complete derived code.  It would be appreciated if the authors were
// told what this software is being used for, but this is not a requirement.

//   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>

#include <XOS.H>
#include <XOSSVC.H>
#include <XOSTRM.H>
#include "PROGHELP.H"

// Local variables
static Prog_Info *__pib=NULL;

static long ansi_conv[8]={0,4,2,6,1,5,3,7};
static int Ansi=FALSE;
static int Ansi_Qual=FALSE;
static int Ansi_Ext=FALSE;
static int Ansi_Home=FALSE;
static int Ansi_Proc=FALSE;
static int esc=FALSE;
// static int Ctrl=FALSE;
static int Xos=FALSE;
static int OK=TRUE;
static int hdrlen;
static int scr_stat;
static int old_stat=(-1);
static STR_NODE *scroll_stop;

static long gpc_ANSIClr=TRUE;

static struct
{   byte4_parm  devsts;
    char        end;
} __prmprm =
{   {(PAR_GET|REP_HEXV), 4, IOPAR_DEVSTS, 0}
};


static struct
{   byte4_parm modec;
    byte4_parm modes;
    char       end;
} __ioparm =
{	{(PAR_SET|REP_HEXV), 4, IOPAR_TRMCINPMODE, 0xFFFFFFFF},
	{(PAR_SET|REP_HEXV), 4, IOPAR_TRMSINPMODE, TIM_PCSCNC|TIM_CHAR|TIM_IMAGE}
};

// Queued IO argument block, defined in XOS.H
static type_qab __pqab =
{
    QFNC_WAIT|QFNC_OUTBLOCK,	// qab_open
    0,				// qab_status
    0,				// qab_error
    0,				// qab_amount
    DH_STDOUT,			// qab_handle
    0,				// qab_vector
    0,	        		// qab_level    - Signal level for direct I/O
    0,  			// qab_prvlevel - Previous signal level (int.)
    0,				// qab_option
    0,				// qab_count
    NULL, 0,			// qab_buffer1
    NULL, 0,			// qab_buffer2
	&__prmprm, 0		// qab_parm
};

// ***********************************************************************
// HEAPSTR functions:  Maintain a heap based linked list of strings (no sort)
// ***********************************************************************

// give_line:  Returns the pointer to the current string
char *give_line(STR_CB *scb)
{
	return scb->curr->string;
};

// give_page:  Returns pointer to buffer; up to 'page' strings concatenated
char *give_page(STR_CB *scb)
{
    STR_NODE *t;
    int i=1;

    t=scb->curr;
	scr_stat=3;
	if ( t==scb->head )
	{
		scr_stat-=2;
	}
    strcpy(scb->buffer,t->string);
    t=t->next;
    while ( (i < scb->page) && (t != NULL) )
    {
		strcat(scb->buffer,t->string);
		i++;
		t=t->next;
    };
	if ( t==NULL || scb->curr==scroll_stop )
	{
		scr_stat-=1;
	}
    return scb->buffer;
};

// page_top:  jumps to top of list
void page_top(STR_CB *scb)
{
	scb->curr=scb->head;
	scb->pos=0;
};
void page_bottom(STR_CB *scb)
{
	scb->curr=scb->tail;
	scb->pos=scb->lines-1;
};

// page_fwd:  steps forward up to 'page' nodes
//	True - OK
//	False - failed (already at end)
int page_fwd(STR_CB *scb)
{
    int i=0;
	if ( scb->curr==scroll_stop )
	{
		return FALSE;
	}
    if ( scb->pos >= (scb->lines-1) ) return FALSE;
    while ( (i < scb->page) && (scb->curr != scroll_stop) && (scb->pos < (scb->lines-1)) )
    {
		i++;
		scb->curr = scb->curr->next;
		scb->pos += 1;
    };
    return TRUE;
};

// page_back:  steps backward up to 'page' nodes
//	True - OK
//	False - failed (already at begining)
int page_back(STR_CB *scb)
{
    int i=0;
    if ( scb->pos == 0 ) return FALSE;
    while ( (i < scb->page) && (scb->pos > 0) )
    {
	i++;
	scb->curr = scb->curr->prev;
	scb->pos -= 1;
    };
    return TRUE;
};

// line_fwd:  steps forward one node
//	True - OK
//	False - failed (already at end)
int line_fwd(STR_CB *scb)
{
	if ( scb->curr==scroll_stop )
	{
		return FALSE;
	};
    if ( scb->pos >= (scb->lines-1) ) return FALSE;
    scb->curr = scb->curr->next;
    scb->pos += 1;
    return TRUE;
};

// line_back:  steps backward one node
//	True - OK
//  False - failed (already at begining)
int line_back(STR_CB *scb)
{
    if ( scb->pos < 1 ) return FALSE;
    scb->curr = scb->curr->prev;
    scb->pos -= 1;
    return TRUE;
};

// empty:  Clears and frees all nodes in the list
//	True - OK
//	False - Invalid (NULL) pointer to list
int empty(STR_CB *scb)
{
    STR_NODE *t;

    // point to the start of list
    scb->curr=scb->head;

    // while there are more nodes in the list
    while ( scb->curr != NULL )
    {
	free(scb->curr->string);
	t=scb->curr->next;
	free(scb->curr);
	scb->curr=t;
    };
    if (scb->buffer != NULL)
	free(scb->buffer);

    scb->head=NULL;
    scb->tail=NULL;
    scb->lines=0;
    scb->pos=(-1);
    return TRUE;
};

// add_line:  Add node to list, allocates buffer if first node
//	True - OK
//	False - Bad string (NULL), bad STR_CB (NULL) or malloc failure
int add_line( STR_CB *scb, char *s )
{
    STR_NODE *t;

    // check to see that a string was passed
    if ( s==NULL )
	return FALSE;

    // allocate space for the node
    t = malloc(sizeof(STR_NODE));
    if ( t == NULL )
	return FALSE;

    // check if it's the first node
    if ( scb->head == NULL )
    {
	scb->lines= 0;
	scb->pos  = 0;
	scb->head = t;
	scb->tail = t;
	scb->curr = t;
	t->prev = NULL;
	scb->buffer = malloc(HELPBUFSZ);
    }
    else
    {
	t->prev = scb->tail;
	scb->tail->next = t;
	scb->tail = t;
    };
    t->next=NULL;

    // allocate space for the string
    t->string = malloc(strlen(s)+1);
    if ( t->string == NULL )
	return FALSE;

    // save a copy of the string and return
    strcpy( t->string, s );
    scb->lines += 1;
    return TRUE;
};

// ***********************************************************************
// Screen Handling functions
// ***********************************************************************

// getTrmParms:  gets the 'console' value and screen size, saves in __pib
// Returns TRUE - done
//		   FALSE - pure defaults (some problem)
int getTrmParms(void)
{
    TRMMODES data;

    int rv=TRUE;
    __pib->handle=STDTRM;

    // Get the screen height and width
    if (svcTrmDspMode(__pib->handle,DM_RTNDATA,&data) >= 0)
    {
	__pib->screen_width = data.dm_columns;
	__pib->screen_height = data.dm_rows;
	__pib->handle = STDTRM;
	__pib->console = TRUE;
    }
    else
    {
	__pib->handle = 0;
	__pib->screen_width = 80;
	__pib->screen_height = 25;
	__pib->page=0;
	__pib->console = FALSE;
	return (FALSE);			// Return the rest will fail also
    }

    // Get the current display page
    __pib->page=svcTrmDspPage(__pib->handle, -1);
    if ( __pib->page < 0 )
    {
	__pib->page=0;
	rv=FALSE;
    };

    // Save the current display colors
    svcTrmAttrib(__pib->handle,0x41,&__pib->old_color);

    if ((__pib->errno = svcIoQueue(&__pqab)) < 0 ||
	(rv = __pqab.qab_error) < 0)
    {
	__pib->console=FALSE;
        return (FALSE);
    }

    // __prmprm is a global instance of a structure containing a
    //    byte4_parm (devsts) [defined in XOS.H]
    //       initial values are GET|HEX,4,DEVSTS,0
    //    plus a character
    // modified by svcIoQueue call above
    if ((__prmprm.devsts.value & DS_CONTROL) == 0)
	__pib->console=FALSE;

    return rv;
};

// getHelpClr: gets help screen colors and saves in __pib
//	Attempts to read program specific color set
//		If not, attempts to read global color set
//			If not, sets hardcoded color scheme
// Returns	TRUE - program specific or global color scheme
//			FALSE - hardcoded color scheme
// NOTE:  both foreground and background colors must be in the range 0-8
int getHelpClr(void)
{
    char envstr[41];
    char clrbuf[41];
    int rv = TRUE;
    long h_fgc=(-1), h_bgc=(-1), b_fgc=(-1), b_bgc=(-1);

    sprintf(envstr,"^XOS^%s^HCLR",__pib->prgname);
    if(svcSysFindEnv(0, envstr, NULL, clrbuf, sizeof(clrbuf), NULL) > 0)
    {
	h_fgc=atol(strtok(clrbuf,",")) % 9;
	h_bgc=atol(strtok(NULL,",")) % 8;
	b_fgc=atol(strtok(NULL,",")) % 9;
	b_bgc=atol(strtok(NULL,",")) % 8;
    }
    else
    {
	strcpy(envstr,"^XOS^ALL^HCLR");
	if(svcSysFindEnv(0, envstr, NULL, clrbuf, sizeof(clrbuf), NULL) > 0)
	{
	    h_fgc=atol(strtok(clrbuf,",")) % 9;
	    h_bgc=atol(strtok(NULL,",")) % 8;
	    b_fgc=atol(strtok(NULL,",")) % 9;
	    b_bgc=atol(strtok(NULL,",")) % 8;
        };
    };

    // If there was a problem (some color not set or fg/bg pair that are eq)
    if ((h_fgc==(-1)) || (h_bgc==(-1)) || (b_fgc==(-1)) || (b_bgc==(-1)) ||
	(h_fgc==h_bgc) || (b_fgc==b_bgc))
    {
	h_fgc=1;	// header blue on
	h_bgc=7;	// White
	b_fgc=2;	// body green on
	b_bgc=0;	// black
	rv = FALSE;
    };
    __pib->hdr_color.fgc=h_fgc;
    __pib->hdr_color.bgc=h_bgc;
    __pib->hdr_color.fgf=h_fgc;
    __pib->hdr_color.bgf=h_bgc;
    __pib->hdr_color.atr=0;
    __pib->bdy_color.fgc=b_fgc;
    __pib->bdy_color.bgc=b_bgc;
    __pib->bdy_color.fgf=b_fgc;
    __pib->bdy_color.bgf=b_bgc;
    __pib->bdy_color.atr=0;

    return rv;
};

// clrlines: Clears an area
//				starting at (starat = y-sub-i)
//				for ( height = n ) lines, of width (width, starts at 0)
//				setting area to colors defined in cb
//	Returns: 0 - OK or XOS error code.
long clrlines(COLDATA *cb, int startat, int height, int width)
{
    int i;
    long rtn=0;
    char buffer[255];

    // set up a buffer to use as a 'clear' string
    memset(buffer,' ',width);
    buffer[width]='\0';

    // set the specified colors
    setcolors(cb);

    // move cursor to start of area to clear
//    svcTrmCurPos(STDOUT,0,0,startat);

    // clear the specified area
    for (i=0; i<height; i++)
    	printf("%c[%d;%dH%s",0x1b,i+startat,1,buffer);

    // move cursor back to start of cleared block
//    rtn = svcTrmCurPos(STDOUT,0,0,startat);
	printf("%c[%d;%dH",0x1b,startat,1);
    return rtn;
};

// setcolors: Changes current color pallete
void setcolors(COLDATA *cb)
{
//    svcTrmAttrib(STDOUT,0x81,cb);
//    svcTrmAttrib(STDOUT,0x83,cb);
	if ( gpc_ANSIClr && (cb->fgc != cb->bgc) )
		printf("%c[%d;%dm",0x1b,30+ansi_conv[cb->fgc],40+ansi_conv[cb->bgc]);
};

// bld_sopts:  build sub-option (XSVAL) display list
void bld_sopts( SubOpts *hb, int dflt )
{
    char temp[SOBUFSZ];
    char junk[20];
    int c, cc, i, si, sol, solMax, sodMax;

    SubOpts *soTemp=hb;
    c=0;
    si=0;
    solMax=0;
    sodMax=0;

    // Find the Max sizes for the option and description fields
    while ( soTemp->option != NULL )
    {
	sol = strlen(soTemp->option);
	if ( sol>solMax )
	    solMax=sol;

	sol = strlen(soTemp->desc);
	if ( sol>sodMax )
	    sodMax=sol;

	si++;
	soTemp=hb+si;
    };
    solMax++;
    sodMax++;

    // Determine how many columns to break sub options into
    if ( (solMax+sodMax) <= ((__pib->screen_width-8)/4) )
	cc=4;
    else
	if ( (sodMax+solMax) > ((__pib->screen_width-8)/2)	)
	    cc=1;
	else
	    if ( (sodMax+solMax) > ((__pib->screen_width-8)/3)	)
		cc=2;
	    else
		cc=3;

    // resest the pointer and counters
    soTemp=hb;
    si=0;
    memset(temp,' ',SOBUFSZ);
    *(temp)='\t';
    i=1;
    while ( soTemp->option != NULL )
    {
	if (si==dflt)
	{
	    sprintf(junk,"%c[%dm",27,1);
	    memcpy(temp+i,junk,strlen(junk));
	    i += strlen(junk);
	};
	memcpy(temp+i,soTemp->option,strlen(soTemp->option));
	i += solMax;
	if (si==dflt)
	{
	    sprintf(junk,"%c[%dm",27,2);
	    memcpy(temp+i,junk,strlen(junk));
	    i += strlen(junk);
        };
        memcpy(temp+i,soTemp->desc,strlen(soTemp->desc));
	i += sodMax;
	si++;
	c++;
	if ( c >= cc )
	{
	    strcpy(temp+i,"\n");
	    add_line( &__pib->scb, temp); // save the built up string
	    memset(temp,' ',SOBUFSZ);
	    *(temp)='\t';
	    i=1;
	    c=0;
	};
	soTemp=hb+si;
    };
    if ( i > 1 )
    {
	strcpy(temp+i,"\n");
	add_line( &__pib->scb,temp); // save the built up string
    };
};

void bld_opts()
{
    arg_spec *hb;
    char junk[80];
    char temp[SOBUFSZ];
    int sz, szMax, descOffset, neg, spcl, offset, i, si, hlen, subopts;
    int j, helpMax, pass2;
    long tflag;

    szMax=0;

	pass2=FALSE;
	if ( __pib->opttbl!=NULL )
	{
    	hb=__pib->opttbl;
	} else {
		if ( __pib->kwdtbl!=NULL )
		{
			hb=__pib->kwdtbl;
			pass2=TRUE;
		}
		return;
	};

    // Calculate the maximum option/input/value size
    while ( hb->name != NULL )
    {
		if ( hb->help_str!=NULL )
		{
	    	sz=strlen(hb->name)+1;
		    if ( hb->flags & (ASF_NVAL|ASF_NVAL16|ASF_NVAL8|ASF_SSVAL|
			      ASF_LSVAL|ASF_XSVAL))
				sz += 10;
	    	if ( (hb->flags & ASF_STORE) &&
			 (hb->flags & (ASF_NVAL|ASF_NVAL16|ASF_NVAL8)))
		    {
				sprintf(junk,"%d",*(long *)(hb->func.t));
				sz += (strlen(junk)+2);
		    };
		    if ( (hb->flags & ASF_STORE) &&
			 (hb->flags & (ASF_LSVAL|ASF_SSVAL)))
	    	{
				sz += (strlen((char *)(hb->func.t))+2);
            };
            if ( sz > szMax )
				szMax=sz;
		}
		hb++;
		if ( hb->name==NULL )
		{
			if ( !pass2 )
			{
				pass2=TRUE;
				if ( __pib->kwdtbl!=NULL )
				{
					hb=__pib->kwdtbl;
				}
			}
		}
    }

    descOffset=szMax+4;

	pass2=FALSE;
	if ( __pib->opttbl!=NULL )
	{
    	hb=__pib->opttbl;
	    add_line( &__pib->scb,"OPTIONS:\n");
	} else {
		if ( __pib->kwdtbl!=NULL )
		{
			hb=__pib->kwdtbl;
			pass2=TRUE;
    		add_line( &__pib->scb,"KEYWORDS:\n");
		}
		return;
	};

    while ( hb->name != NULL )
    {
		if ( hb->help_str!=NULL )
		{
		    sz=0;
		    neg=FALSE;
		    spcl=0;
		    memset(temp,' ',SOBUFSZ);
		    tflag= hb->flags & (ASF_BOOL|ASF_FLAG|ASF_STORE|ASF_NEGATE);
		    if ( tflag & (ASF_NEGATE|ASF_FLAG|ASF_BOOL) )
		    {
				// The negate option is enabled
				if ( tflag & ASF_BOOL  )
				    neg=TRUE;
				if ( tflag & ASF_STORE )
				{
				    if ( ((tflag & ASF_BOOL) && ( *(long *)(hb->func.t)==0 )))
				    {
						// turn on highlight and bump offset and spcl
						sprintf(junk,"%c[%dm",27,1);
						memcpy(temp+sz,junk,strlen(junk));
						spcl+=strlen(junk);
						sz+=strlen(junk);
				    };
		        };
				memcpy(temp+sz,"{NO}",4);
				sz += 4;
				//Turn on highlighting and bump offset and spcl
				sprintf(junk,"%c[%dm",27,1);
				memcpy(temp+sz,junk,strlen(junk));
				spcl+=strlen(junk);
				sz+=strlen(junk);
		    }
		    else
		    {
				// This option can't be negated
				memcpy(temp,"    ",4);
				sz += 4;
		    }
		    memcpy(temp+sz,hb->name,strlen(hb->name));
		    sz += strlen(hb->name);
		    // Turn higlight off and bump size and spcl
		    sprintf(junk,"%c[%dm",27,2);
	        memcpy(temp+sz,junk,strlen(junk));
		    spcl+=strlen(junk);
		    sz+=strlen(junk);

		    // Add the input type string
            if ( hb->flags & ASF_XSVAL )
                subopts=TRUE;
            else
                subopts=FALSE;
		    tflag=( hb->flags & (ASF_BOOL|ASF_NVAL|ASF_NVAL16|ASF_NVAL8|
					 ASF_FLAG|ASF_SSVAL|ASF_LSVAL|ASF_XSVAL));
		    switch (tflag)
		    {
			    case ASF_LSVAL :
					memcpy(temp+sz,"{=string} ",10);
					sz += 10;
					break;
		    	case ASF_SSVAL :
					memcpy(temp+sz,"{=S_Str} ",9);
					sz += 9;
					break;
			    case ASF_XSVAL :
					memcpy(temp+sz,"{=list} ",8);
					sz += 8;
					break;
			    case ASF_NVAL :
					memcpy(temp+sz,"{=long} ",8);
					sz += 8;
					break;
			    case ASF_NVAL16 :
					memcpy(temp+sz,"{=short} ",9);
					sz += 9;
					break;
			    case ASF_NVAL8 :
					memcpy(temp+sz,"{=char} ",8);
					sz += 8;
					break;
                case ASF_BOOL :
                case ASF_FLAG :
                    break;
                default :
                    if ( tflag > 0 )
                        memcpy(temp+sz,"{=value} ",9);
                    break;
		    }

		    // Add the current value if applicable
		    if ( !neg && (hb->flags & ASF_STORE) )
		    {
				switch (tflag)
				{
				case ASF_SSVAL :
					sprintf(junk,"[%s]",(char *)(hb->func.t));
					memcpy(temp+sz,junk,strlen(junk));
					sz += strlen(junk);
	        		break;
				case ASF_LSVAL :
				    sprintf(junk,"[%s]",(char *)(hb->func.t));
				    memcpy(temp+sz,junk,strlen(junk));
				    sz += strlen(junk);
					break;
	            case ASF_NVAL8 :
			    	sprintf(junk,"[%d]",*(char *)(hb->func.t));
			    	memcpy(temp+sz,junk,strlen(junk));
			    	sz += strlen(junk);
			    	break;
				case ASF_NVAL16 :
			    	sprintf(junk,"[%d]",*(short *)(hb->func.t));
			    	memcpy(temp+sz,junk,strlen(junk));
			    	sz += strlen(junk);
			    	break;
				case ASF_FLAG :
			    	sprintf(junk,"[0x%08X]",*(long *)(hb->func.t));
			   		memcpy(temp+sz,junk,strlen(junk));
			    	sz += strlen(junk);
			    	break;
				case ASF_NVAL :
			    	sprintf(junk,"[%ld]",*(long *)(hb->func.t));
			    	memcpy(temp+sz,junk,strlen(junk));
			    	sz += strlen(junk);
			    	break;
				}
			}

		    // start adding the help string
		    offset=0;
		    sz = descOffset + spcl;
		    hlen=strlen(hb->help_str);
		    helpMax=__pib->screen_width - sz;
		    si=0;
		    while ( offset < hlen )
		    {
				strncpy(temp+sz,hb->help_str+offset,helpMax);
				*(temp+sz+helpMax)='\0';
				if ( offset+helpMax < hlen )
				{
				    i=helpMax-1;
				    while ( *(temp+sz+i) != ' ' && *(temp+sz+i) != '\n' )
			    	{
						i--;
						offset--;
				    }
				    *(temp+sz+i)='\0';
				}
				offset += helpMax;
				j=strlen(temp);
				*(temp+j)='\n';
				*(temp+j+1)='\0';
				add_line( &__pib->scb,temp);
				memset(temp,' ',SOBUFSZ);
				sz=descOffset;
		    }
		    if ( subopts )
		    {
				if ( hb->flags & ASF_STORE )
				    bld_sopts(hb->svalues.s,*(long *)(hb->func.t));
				else
			    	bld_sopts(hb->svalues.s,-1);
			}
		}
		hb++;
		if ( hb->name==NULL )
		{
			if ( !pass2 )
			{
				pass2=TRUE;
				if ( __pib->kwdtbl!=NULL )
				{
					hb=__pib->kwdtbl;
    				add_line( &__pib->scb,"\n");
    				add_line( &__pib->scb,"KEYWORDS:\n");
				}
			}
		}

    }
};

void bld_help()
{
    char temp[SOBUFSZ];
    int i, l, offset;

    offset=0;
    if ( __pib->desc != NULL )
    {
		l=strlen(__pib->desc);
		while (offset<l)
		{
		    strncpy(temp,__pib->desc+offset,__pib->screen_width-1);
		    *(temp+__pib->screen_width-1)='\0';
		    if ( (offset+__pib->screen_width-1) < l )
		    {
		    	i=__pib->screen_width-1;
		    	while ( *(temp+i) != ' ' && *(temp+i) != '\n' )
			{
	            	    i--;
			    offset--;
			};
			*(temp+i)='\0';
		    };
		    offset += __pib->screen_width;
		    i=strlen(temp);
		    *(temp+i)='\n';
		    *(temp+i+1)='\0';
		    add_line( &__pib->scb,temp);
		};
    };
    add_line( &__pib->scb,"\n");
};

void draw_stat()
{
	char u=' ';
	char d=' ';
    // Set up instruction line
	if ( scr_stat & 2 )
	{
		u='<';
	}
	if ( scr_stat & 1 )
	{
		d='>';
	}
    clrlines(&__pib->hdr_color,__pib->screen_height-1,1,__pib->screen_width);
    printf("                             ^C or Q to quit  [%c%c]",u,d);
    setcolors(&__pib->bdy_color);
	old_stat=scr_stat;
}

void page_up()
{
	if ( page_back( &__pib->scb ))
	{
    	clrlines(&__pib->bdy_color, hdrlen, __pib->screen_height-(hdrlen+1), __pib->screen_width);
    	printf("%c[%d;%dH%s",0x1b,hdrlen,1,give_page( &__pib->scb ));
		if ( old_stat!=scr_stat )
		{
			draw_stat();
		};
	};
};
void page_down()
{
	if ( scr_stat & 1 || scroll_stop==__pib->scb.curr )
	if ( page_fwd( &__pib->scb ) )
	{
   		clrlines(&__pib->bdy_color, hdrlen, __pib->screen_height-(hdrlen+1), __pib->screen_width);
   		printf("%c[%d;%dH%s",0x1b,hdrlen,1,give_page( &__pib->scb ));
		if ( old_stat!=scr_stat )
		{
			draw_stat();
		};
	};
};
void line_up()
{
	if ( line_back( &__pib->scb ) )
	{
    	clrlines(&__pib->bdy_color, hdrlen, __pib->screen_height-(hdrlen+1), __pib->screen_width);
	    printf("%c[%d;%dH%s",0x1b,hdrlen,1,give_page( &__pib->scb ));
		if ( old_stat!=scr_stat )
		{
			draw_stat();
		};
	};
};
void line_down()
{
	if ( scr_stat & 1 || scroll_stop==__pib->scb.curr )
	if ( line_fwd( &__pib->scb ) )
	{
    	clrlines(&__pib->bdy_color, hdrlen, __pib->screen_height-(hdrlen+1), __pib->screen_width);
	    printf("%c[%d;%dH%s",0x1b,hdrlen,1,give_page( &__pib->scb ));
		if ( old_stat!=scr_stat )
		{
			draw_stat();
		};
	};
};

int input_wto(char *buffer, long b_sz, long ms)
{
	struct
	{   byte4_parm modec;
	    byte4_parm modes;
		byte4_parm modet;
	    char       end;
	} __ioparm =
	{	{(PAR_SET|REP_HEXV), 4, IOPAR_TRMCINPMODE, 0xFFFFFFFF},
		{(PAR_SET|REP_HEXV), 4, IOPAR_TRMSINPMODE, TIM_PCSCNC|TIM_IMAGE},
		{(PAR_SET|REP_HEXV), 4, IOPAR_TIMEOUT, 0x0}
	};

	type_qab qab;

	qab.qab_func=QFNC_INBLOCK|QFNC_WAIT;
	qab.qab_handle=STDIN;
	qab.qab_count=b_sz;
	qab.qab_buffer1=buffer;
	qab.qab_parm=&__ioparm;
	__ioparm.modet.value=ms*50;

	ms=0;

	svcIoQueue(&qab);
	return qab.qab_amount;
}



void Ansi_Key(long key)
{
	esc=FALSE;
	// Second chararacter in sequence MUST (?) be a '['
	if ( !Ansi_Qual )
	{
		if ( key==0x5b )
		{
			Ansi_Qual=TRUE;
			Ansi_Ext=FALSE;
			return;
		} else {
			// this was an escape followed by some other key
			esc=TRUE;
			Ansi=FALSE;
			return;
		};
	};
	if ( Ansi_Ext )
	{
		if ( key==0x7e )
		{
			if ( !Ansi_Proc )
			{
				// This was an unadorned x31 (Ins) or x32 (home)
				Ansi=FALSE;
				Ansi_Qual=FALSE;
				Ansi_Home=FALSE;
				Ansi_Ext=FALSE;
				return;
			};
		} else {
// printf("<%ld>",key);
			switch (key)
			{
				// This is an extended key (function key)
				case 0x37 :
					// F2
					OK=FALSE;
					break;
			};
			Ansi_Proc=TRUE;
			return;
        };
	};
	if ( key==0x7e )
	{
		Ansi=FALSE;
		Ansi_Qual=FALSE;
		Ansi_Home=FALSE;
		Ansi_Ext=FALSE;
		Ansi_Proc=FALSE;
		return;
	};
	// Third character may or may not complete it
	switch (key)
	{
		case 0x41 :
			// Up Arrow
			line_up();
			Ansi=FALSE;
			Ansi_Qual=FALSE;
			return;
		case 0x42 :
			// Down Arrow
			line_down();
			Ansi=FALSE;
			Ansi_Qual=FALSE;
			return;
		case 0x33 :
			// Page Up
			page_up();
			break;
		case 0x36 :
			// Page down
			page_down();
            break;
    };
	Ansi_Proc=TRUE;
};

void Xos_Key(long key)
{
	switch (key)
	{
		case 0x48 :
			// Line Up
			line_up();
			break;
		case 0x50 :
			// Line Down
			line_down();
			break;
		case 0x3c :
			// F2
			OK=FALSE;
            break;
		case 0x49 :
			// Prev Page
			page_up();
            break;
		case 0x51 :
			// Next Page
			page_down();
            break;
	};
	Xos=FALSE;
};

void Ctrl_Key(long key)
{
	switch ( key )
	{
		case 0x12 :
//			page_up();
			break;
		case 0x05 :
//			line_up();
			break;
		case 0x18 :
//			line_down();
			break;
		case 0x03 :
			OK=FALSE;		// break on Ctrl-C
//			page_down();
			break;
		case '\n' :
		case '\r' :
			page_down();
			break;
	};
};

void key_cmd(long key)
{
	if (Ansi)
	{
		Ansi_Key(key);
		return;
	};
	if (Xos)
	{
 		Xos_Key(key);
		return;
	};
	if (key==0x1b)
	{		// Escape sequence  ANSI 7 bit
		Ansi=TRUE;
		return;
	};
	if (key==0x0)
	{		// XOS sequence
		Xos=TRUE;
		return;
	};
	if (key<0x20 || key==0x7f)
	{		// Control key
		Ctrl_Key(key);
		return;
	} else {
		if (key>0x7f)				// Unknown key
			return;
	};
	// Normal (printable) key processing
	if ( key=='q' || key=='Q' )
		OK=FALSE;
	if ( key== ' ' )
		page_down();
};



void opthelp(void)
{
	char header[512];
	char version[10];
    char temp[90];
    long key, old_crs;
	char *pC, *pOldC;
	int i;

    old_crs=svcTrmCurType(__pib->handle,-256);

	// Determine terminal type
	printf("%c[c",0x1b);
	i=input_wto(header,100,500);
	if ( i>0 )
	{
		if ( strcmp(header+1,"[?62c")==0 )
		{
			// It's an XOS console
			gpc_ANSIClr=TRUE;
		} else {
			gpc_ANSIClr=FALSE;
		};

	} else {
		gpc_ANSIClr=FALSE;
	};

    // Setup header
    setcolors(&__pib->hdr_color);
    clrlines(&__pib->hdr_color, 0, __pib->screen_height, __pib->screen_width);

    sprintf(version,"%d.%d",__pib->majedt,__pib->minedt);
    sprintf(header,"                              Help on %s\n",__pib->prgname);
    if ( strlen(__pib->example)== 0 )
    {
		sprintf(temp,"%-10sv%-6s[%s]  %s", __pib->prgname, version, __pib->build, __pib->copymsg);
		strcat(header,temp);
    }
    else
    {
		sprintf(temp,"%s %s\n",__pib->prgname,__pib->example);
		strcat(header,temp);
		sprintf(temp,"%-10sv%-6s[%s]  %s", "", version, __pib->build, __pib->copymsg);
		strcat(header,temp);
    };

	// figure out how big the header is
	pC=header;
	pOldC=header;
	hdrlen=0;
	while ( pC != NULL )
	{
		pC=strchr(pC,'\n');
		hdrlen++;
		if ( pC != NULL )
		{
			hdrlen += (int)((pC-pOldC)/80);
			pOldC=pC;
			pC++;
		}
	}
	hdrlen++;
	printf("%s",header);


    // Set up and clear body of help
    setcolors(&__pib->bdy_color);
    clrlines(&__pib->bdy_color, hdrlen, __pib->screen_height-(hdrlen+1), __pib->screen_width);
    __pib->scb.page=__pib->screen_height-(hdrlen+1);

    bld_help();
    bld_opts();

	// Mark the last line to scroll down to
	page_bottom(&__pib->scb);
	page_back(&__pib->scb);
	scroll_stop=__pib->scb.curr;
	page_top(&__pib->scb);
	scroll_stop=scroll_stop->next;

    printf("%c[%d;%dH%s",0x1b,hdrlen,1,give_page( &__pib->scb ));

	if ( old_stat!=scr_stat )
	{
		draw_stat();
	}

    while(OK)
    {
		key = svcIoInSingleP(DH_STDTRM, &__ioparm);
		key_cmd(key);
    };
    empty( &__pib->scb );
    svcTrmCurType(__pib->handle,old_crs);
    setcolors(&__pib->old_color);
    clrlines(&__pib->old_color,0,__pib->screen_height,__pib->screen_width);
    exit(0);
}

void reg_pib( Prog_Info *user_pib )
{
    __pib=user_pib;
    __pib->scb.head=NULL;
    __pib->scb.buffer=NULL;
	__pib->kwdtbl=NULL;
	__pib->opttbl=NULL;
};
