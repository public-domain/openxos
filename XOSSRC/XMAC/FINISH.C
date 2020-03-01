//*************************************
// Function to finish assembly for XMAC
//*************************************
// Written by John Goltz
//*************************************

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
#include <XOS.H>
#include <SEARCH.H>
#include "XMAC.H"
#include "XMACEXT.H"

//***********************************
// Function: finish - Finish assembly
// Returned: Nothing
//***********************************

// This function scans the hash table and builds two lists of symbol table
//   entries, one for macros and one for symbols - entires for local symbols
//   and for psects are discarded

void finish(void)

{
    struct sy *sympnt;
    struct sy *macpnt;
    struct sy *sp;
    struct sy **hp;
    int    cnt;

    hp = hashtbl;
    cnt = HASHSIZE;
    sympnt = (struct sy *)&symhead;
    macpnt = (struct sy *)&machead;
    do
    {   if ((sp = *hp++) != NULL)	// Get start of hash list
	{
            do
	    {   if (sp->sy_type == SYT_SYM && sp->sy_name[0])
                {			// Local symbol or psect name?
		    if (sp->sy_name[0] & 0x80) // No - macro name?
		    {
			sp->sy_name[0] &= 0x7F; // Yes
			macpnt->sy_hash = sp; // Link to macro list
			macpnt = sp;
		    }
		    else
		    {
			sympnt->sy_hash = sp; // No - link to symbol list
			sympnt = sp;
			if (sp->sy_flag & SYF_UNDEF) // Undefined symbol?
			    ++undcnt;	// Yes - count it
			if (sp->sy_flag & SYF_MULT) // Multiply defined?
			    ++mltcnt;	// Yes - count it
		    }
		}
	    } 
	    while ((sp = sp->sy_hash) != NULL);
	}
    } 
    while (--cnt > 0);
    sympnt->sy_hash = 0;		// Clear final pointers in both lists
    macpnt->sy_hash = 0;
    symhead = heapsort(symhead, (int (*)(void *a, void *b, void *data))compsym,
            0);				// Sort the symbol table
}
