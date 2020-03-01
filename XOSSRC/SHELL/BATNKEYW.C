//------------------------------------------------------------------------------
//
//  BATNKEYW.C
//
//  Written by Bruce R. Nevins
//
//  Edit History:
//  -------------
//  01/01/87(brn) - Created first version
//  03/28/94(brn) - made / and = keywords that get returned
//
//------------------------------------------------------------------------------

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

#include <CTYPE.H>
#include <STDIO.H>
#include <XOS.H>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

// Function to return next keyword for batch commands
//  where outstr is the string pointer for where to place the
//  keyword found in inblk.
// Returns: TRUE if keyword found, FALSE if none found.
//
int batnkeyw(register char *outstr, register struct firstblk *inblk)
{
    char chr;

    chr = blkgetc(inblk);
    if (chr == '\0')
    {
        return (FALSE);
    }

/*
 * Skip any white space or delimiters
 */
    while (isspace(chr) ||
           chr == ';' ||
           chr == ',')
        chr = blkgetc(inblk);           // Get next character

    if (chr == '\0')
    {
        return (FALSE);
    }

    if (chr == '=')			// = is to be returned alone
    {
	*outstr++ = chr;
	*outstr = '\0';
	return (TRUE);
    }

    if (chr == '/')			// = is to be returned alone
    {
	while (!(isspace(chr) ||	// Store an alnum string
            chr == '\0'  ||
            chr == ';'   ||
            chr == ','))
	{
	    *outstr++ = chr;
	    chr = blkgetc(inblk);
	}
	*outstr = '\0';
	return (TRUE);
    }
    while (!(isspace(chr) ||		// Store an alnum string
            chr == '\0'||
            chr == ';' ||
            chr == '=' ||
	    chr == '/' ||
            chr == ','))
    {
        *outstr++ = chr;
        chr = blkgetc(inblk);
    }
    *outstr = '\0';             	// Mark end of line
    blkungetc(inblk, chr);		// Put back for next time
    return (TRUE);
}

