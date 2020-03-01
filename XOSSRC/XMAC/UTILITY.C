//**************************
// Utility routines for XMAC
//**************************
// Written by John Goltz
//**************************

//++++
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
#include <CTYPE.H>
#include <STRING.H>
#include <XOS.H>
#include <MALLOC.H>
#include <XCMALLOC.H>
#include "XMAC.H"
#include "XMACEXT.H"

// Link to global data

extern int symsize;		// Length of symbol name in symbuf
extern struct sy *hashtbl[];	// The symbol hash table
extern struct fb *freelist;	// Memory block free list head pointer

// Allocate local data

int curhash;				// Current hash table index

//*******************************************
// Function: getblock - Allocate memory block
// Returned: Pointer to block
//*******************************************

struct fb *getblock(void)

{
    struct fb *temp;
    int    cnt;

    if (freelist == NULL)		// If have something on the freelist
    {					// If free list is empty
	temp = (struct fb *)getspace((long)(100*BLKSIZE));
        freelist = temp;		// Get some more blocks
        cnt = 100;
        while (--cnt >= 0)
        {
            temp->fb_next = temp + 1;
            temp++;
        }
        (temp-1)->fb_next = NULL;
    }
    temp = freelist;			// Get block from freelist
    freelist = temp->fb_next;
    temp->fb_next = NULL;
    return (temp);
}

//******************************************
// Function: givblock - Give up memory block
// Returned: Nothing
//******************************************

void givblock(
    struct fb *block)		// Pointer to block to give up

{
    block->fb_next = freelist;		// Put block on the free list
    freelist = block;
}

//****************************************************
// Function: looksym - Look for symbol in symbol table
// Returned: Pointer to symbol table entry
//****************************************************

struct sy *looksym(void)

{
    struct sy *pnt;
    char  *pnt1;
    char  *pnt2;
    int    cnt;
    char   indexx;

    cnt = symsize;
    pnt1 = symbuf.nsym;
    indexx = 0;
    while (--cnt >= 0)
    {
	indexx <<= 1;
	if (indexx < 0)
	    ++indexx;
	indexx ^= *pnt1++;
    }
    curhash = indexx & 0x7F;
    pnt = (struct sy *)(&hashtbl[curhash]);// Point to hash table entry
    while ((pnt = pnt->sy_hash) != NULL) // Scan down the hash chain
    {
	cnt = symsize;			// Check this entry
	pnt1 = symbuf.nsym;
	pnt2 = pnt->sy_name;
	while (--cnt >= 0)
	{
	    if (*pnt1++ != *pnt2++)
		break;
	}
	if (cnt < 0 && (*pnt2 == '\0' || symsize == SYMMAXSZ))
	    return (pnt);		// If this is it
    }
    return NULL;			// Not found if get here
}

//**************************************************
// Function: entersym - Enter symbol in symbol table
// Returned: Pointer to symbol table entry
//**************************************************

struct sy *entersym(void)

{
    struct sy *pnt;
    char  *pnt1;
    char  *pnt2;
    int    cnt;

    pnt = (struct sy *)getblock();	// Get block for symbol table entry
    pnt->sy_hash = hashtbl[curhash];	// Link into hash list
    hashtbl[curhash] = pnt;
    cnt = symsize;			// Copy symbol to entry
    pnt1 = symbuf.nsym;
    pnt2 = pnt->sy_name;
    while (--cnt >= 0)
	*pnt2++ = *pnt1++;
    cnt = SYMMAXSZ - symsize;		// Set remainder to nulls
    while (--cnt >= 0)
	*pnt2++ = '\0';
    pnt->sy_symnum = 0;
    symnum++;
    return (pnt);			// Finished
}

//******************************************
// Function: givlist - Give up argument list
// Returned: Nothing
//******************************************

void givlist(
    struct ab *block)

{
    struct ab *save;

    while (block)
    {
	save = block->ab_link;
	givblock((struct fb *)block);
	block = save;
    }
}

//***********************************************************
// Function: chkpsect - Check that a psect has been specified
// Returned: Nothing
//***********************************************************

void chkpsect(void)

{
    if (ptype == 0)			// Processor type specified?
        ptype = P_8086;			// No - assume 8086
    if (curpsd == NULL || curmsd == NULL || cursgd == NULL)
    {					// Psect specified?
        errsub(errnpmsg, ERR_NP);	// No - fatal error!
        skiprest();
        listline();
        fputs("? XMAC: Fatal error - cannot continue\n", stderr);
        exit(1);
    }
}

//**************************************************************
// Function: getsegnumps - Get segment number given psect number
// Returned: Segment number
//**************************************************************

int getsegnumps(
    int psect)

{
    struct pd *psd;

    if ((psd=pshead) != NULL)
    {
        do
        {   if (psd->pd_psn == psect)
                return (psd->pd_mdb->md_sdb->sd_sgn);
        } while ((psd=psd->pd_next) != NULL);
    }
    return (0xFFFF);
}

//**************************************************************
// Function: getsegnumms - Get segment number given msect number
// Returned: Segment number
//**************************************************************

int getsegnumms(
    int msect)

{
    struct md *msd;

    if ((msd=mshead) != NULL)
    {
        do
        {   if (msd->md_msn == msect)
                return (msd->md_sdb->sd_sgn);
        } while ((msd=msd->md_next) != NULL);
    }
    return (0xFFFF);
}

//**********************************
// Function: errorq - Report Q error
// Returned: Nothing
//**********************************

void errorq(void)

{
    errsub(errqmsg, ERR_Q);
}

//**********************************
// Function: errorc - Report C error
// Returned: Nothing
//**********************************

void errorc(void)

{
    errsub(errcmsg, ERR_C);
}

//**********************************
// Function: errorx - Report X error
// Returned: Nothing
//**********************************

void errorx(void)

{
    errsub(errxmsg, ERR_X);
}

//************************************
// Function: errorna - Report NA error
// Returned: Nothing
//************************************

void errorna(void)

{
    errsub(errnamsg, ERR_NA);
}

//********************************
// Function: errsub - Report error
// Returned: Nothing
//********************************


void errsub(msg, flag)
char *msg;
long  flag;

{
    if (errcnt < ETBLSIZE && !(errflag & flag))
    {
        *errpnt++ = msg;		// Store address of message in table
        ++errcnt;
        errflag |= flag;		// And set flag for error
    }
}
