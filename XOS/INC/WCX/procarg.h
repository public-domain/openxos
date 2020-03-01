//------------------------------------------------------------------------------
//
// PROCARG.H - Prototypes and functions for PROCARG function for XOS
//
//  Written by: John R. Goltz
//
//  Edit History:
//  -------------
//  09/07/92(brn) - Add comment header
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

#ifndef _PROCARG_H
#define _PROCARG_H

// Define bits for flags (argument to PROCARG)

#define PAF_PATOM     0x8000	// Treat plus sign as an atom
								//  Note: Use of both PAF_PATOM and
								//	  PAF_PSWITCH not allowed,
								//	  PAF_PSWITCH has priority
#define PAF_EATQUOTE  0x4000	// Strip quotes from the strings
#define PAF_PSWITCH   0x1000	// Allow plus sign as a switch character
#define PAF_INDIRECT  0x0800	// Allow indirect files
#define PAF_ECHOINDIR 0x0400	// Echo indirect input to stdout

// Define structure which describes argument values

typedef struct argdata
{	char *name;					// Pointer to name of option or keyword
	union						// Value of option or keyword
	{	long  n;
		char *s;
		char  c[4];
    }     val;
	long  data;					// Data from option or keyword definition
	long  flags;				// Value description bits
	long  length;				// Length of string value
} arg_data;

// Define values for argdata.flags

#define ADF_NVAL  0x4000		// Numeric value present
#define ADF_SSVAL 0x2000		// Short string value present
#define ADF_LSVAL 0x1000		// Long string value present
#define ADF_XSVAL 0x0800		// String table index value present
#define ADF_NONE  0x0400		// No = after option or keyword
#define ADF_NULL  0x0200		// No value after =

// Define structure which specifies options and keywords

typedef struct argspec
{   char  *name;				// Name of option or keyword
    long   flags;				// Flag bits
    char **svalues;				// Pointer to string value table
    long (*func)(arg_data *);	// Pointer to function
    long   data;				// Data to pass to function
} arg_spec;

// Values for argspec.flags

#define ASF_VALREQ 0x8000		// A value is required
#define ASF_NVAL   0x4000		// Value may be numeric
#define ASF_SSVAL  0x2000		// Value may be a short string
#define ASF_LSVAL  0x1000		// Value may be a long string
#define ASF_PLUS   0x0100		// Restrict switch character to '+' only
#define ASF_MINUS  0x0080		// Restrict switch character to '-' only

#define PROCARGEBSZ 130

int procarg(char **apnt, unsigned int flags, arg_spec *optbl, arg_spec *kwtbl,
    int (*fnc)(char *), void (*uerrrtn)(char *, char *), int (*contfnc)(void),
    char *indirext);

#endif	// _PROCARG_H_
