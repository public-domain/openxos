//------------------------------------------------------------------------
// PROGASST.H
// Prototypes and functions for PROGASST functions for XOS
//
// Written by: SA Ortmann
//
// Edit History:
// 12/15/94(SAO) - Initial development
//
//------------------------------------------------------------------------

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

#ifndef _PROGASST_H
#define _PROGASST_H

#ifndef TRUE
#define TRUE 1
#define FALSE !TRUE
#endif

#ifndef _PROGARG_H
#include "progarg.h"
#endif

// ***********************************************************************
//                  HEAPSTR header info
// ***********************************************************************
// string control block structure
typedef struct strnode {
    struct strnode *prev;
    struct strnode *next;
    char *string;
} STR_NODE;

typedef struct strctrlb {
    int total;
    int pos;            // the current position in the structure
    int page;           // the size (in lines) of a page
    int lines;          // the number of lines (nodes) in use
    STR_NODE *head;     // pointer to head node
    STR_NODE *tail;     // pointer to tail node
    STR_NODE *curr;     // pointer to current node
    char *buffer;       // large character buffer, used by give_page
} STR_CB;

// function prototypes
int line_fwd(STR_CB *);         // steps forward one node
int line_back(STR_CB *);        // steps back one node
int page_fwd(STR_CB *);         // jumps 'page' nodes forward
int page_back(STR_CB *);        // jumps 'page' nodes back
int add_line(STR_CB *, char *);   // adds new node to end of list
int empty(STR_CB *);            // clears and frees all nodes in list
char *give_line(STR_CB *);      // return string from current node
char *give_page(STR_CB *);      // return * to buffer with up to page strings

// ***********************************************************************
//					HELP(optusage) header info
// ***********************************************************************
#define AF(func) (int (*)(arg_data *))func
#define SOBUFSZ 200
#define HELPBUFSZ 5*1024

typedef struct col_data{
    long fgc;
    long bgc;
    long fgf;
    long bgf;
    long atr;
} COLDATA;

typedef struct proginfo{
    int     console;            // TRUE if console is output device
    int     majedt;             // major edit number;
    int     minedt;             // minor edit number;
    long    errno;              // last error number
	long	screen_width;		// Should be set from svcIoTrmMode result
	long	screen_height;		// Should be set from svcIoTrmMode result
	long	handle; 			// The terminal device handle
	long	page;				// The display page from handle
	COLDATA old_color;			// The color scheme on entry
	COLDATA hdr_color;			// The color scheme for help header
	COLDATA bdy_color;			// The color scheme for help body
	STR_CB scb; 				// Heap string control block
    char   *copymsg;            // Pointer to the Copyright notice string
    char   *prgname;            // Pointer to the Program name string
    char   *build;              // Pointer to the Program build date string
    char   *desc;               // Pointer to the Program description
    char   *example;            // Pointer to the Program example string
    arg_spec    *opttbl;        // Pointer to the option table
	arg_spec	*kwdtbl;		// Pointer to the keyword table
}Prog_Info;

void reg_pib(Prog_Info *user_pib);	// Must be called before optusage
int optusage(); 	//

#endif  // _PROGASST_H_
