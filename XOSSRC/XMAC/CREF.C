//*************************************************
// Function to manage CREF reference lists for XMAC
//*************************************************
// Written by John Goltz
//*************************************************

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
#include <STRING.H>
#include <CTYPE.H>
#include <XOS.H>
#include "XMAC.H"
#include "XMACEXT.H"

// Allocate local data

union
{
    int *ps;
    int **pps;
} refpnt;			// Pointer to reference list
int *refend;			// Pointer to end of reference list
int refcnt;			// Word count for reference list block

//************************************************************
// Function: refsym - Put entry in reference list if necessary
// Returned: Nothing
//************************************************************

void refsym(
    struct sy *sym,		// Pointer to symbol block
    int    def)			// TRUE if defining reference

{
    struct rf *ref;
    int    stmp;

    if (listtype == LT_CREF && (passtwo || prmflg)) // Do we need to do this?
    {
	if ((ref=sym->sy_ref) == 0)	// Yes - do we have a list now
	{				// No - start one
	    ref = (struct rf *)getblock();
	    sym->sy_ref = ref;
	    ref->rf_pnt.ps = ref->rf_data;
	    ref->rf_cnt = (BLKSIZE - 10)/2;
	}
	stmp = pagnum;			// Store page number in list
	if (def)
	    stmp |= 0x8000;
	refent(stmp, ref);
	stmp = lstseq;			// Store sequence number in list
	refent(stmp, ref);
    }
}

//*********************************************************
// Function: refent - Store single word in a reference list
// Returned: Nothing
//*********************************************************

void refent(
    long   word,
    struct rf *ref)

{
    struct rf *temp;

    if (--ref->rf_cnt < 0)		// More room in this block?
    {
	temp = (struct rf *)getblock();	// No, allocate another block
	*ref->rf_pnt.pps = (int *)temp;
	ref->rf_pnt.ps = (int *)temp;
	ref->rf_cnt = ((BLKSIZE - 4)/2) - 1;
    }
    *ref->rf_pnt.ps++ = word;		// Store word in block
}

//******************************************************************
// Function: setref - Setup to access reference if have one
// Returned: TRUE if have a reference list, FALSE if do not have one
//******************************************************************

int setref(
    struct sy *sym)		// Pointer to symbol table entry

{
    struct rf *ref;

    if ((ref=sym->sy_ref) != NULL)	// Do we have a reference list?
    {
	refpnt.ps = ref->rf_data;	// Initialize pointer
	refend = ref->rf_pnt.ps;
	refcnt = (BLKSIZE - 10)/2;	// Initialize count
	return (TRUE);			// Indicate have list
    }
    else
	return (FALSE);				// Indicate no list
}

//*************************************************
// Function: nextref - Get next reference list word
// Returned: Value
//*************************************************

int nextref(void)

{
    if (--refcnt < 0)			// More in current block?
    {
	if (!*refpnt.pps)		// NO - end of list here?
	    return (0);			// Yes
	else
	{
	    refpnt.ps = *refpnt.pps;	// No - advance to next block
	    refcnt = (BLKSIZE - 4)/2 - 1;
	}
    }
    else
	if (refpnt.ps == refend)	// End of list?
	    return (0);			// Yes
    return (*refpnt.ps++);		// Not end - return item
}
