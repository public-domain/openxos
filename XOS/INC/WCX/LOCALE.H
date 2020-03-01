//------------------------------------------------------------------------------
//
//  LOCALE.H - ANSI C header file for localization functions
//
//  Edit History:
//  -------------
//  17Aug94 (fpj) - Original creation.
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

#ifndef _LOCALE_H_
#define _LOCALE_H_

#ifndef _BASELINE_H_

#include <baseline.h>

#endif // _BASELINE_H_


struct lconv
{
    char *decimal_point;                /* "." in "C" locale */
    char *thousands_sep;                /* "" in "C" locale */
    char *grouping;                     /* "" in "C" locale */
    char *int_curr_symbol;              /* "" in "C" locale */
    char *currency_symbol;              /* "" in "C" locale */
    char *mon_decimal_point;            /* "" in "C" locale */
    char *mon_thousands_sep;            /* "" in "C" locale */
    char *mon_grouping;                 /* "" in "C" locale */
    char *positive_sign;                /* "" in "C" locale */
    char *negative_sign;                /* "" in "C" locale */
    char int_frac_digits;               /* CHAR_MAX in "C" locale */
    char frac_digits;                   /* CHAR_MAX in "C" locale */
    char p_cs_precedes;                 /* CHAR_MAX in "C" locale */
    char p_sep_by_space;                /* CHAR_MAX in "C" locale */
    char n_cs_precedes;                 /* CHAR_MAX in "C" locale */
    char n_sep_by_space;                /* CHAR_MAX in "C" locale */
    char p_sign_posn;                   /* CHAR_MAX in "C" locale */
    char n_sign_posn;                   /* CHAR_MAX in "C" locale */
};

#define NULL    _NULL

// #define LC_ALL
// #define LC_COLLATE
// #define LC_CTYPE
// #define LC_MONETARY
// #define LC_NUMERIC
// #define LC_TIME

//
// Function prototypes
//

extern struct lconv *localeconv(void);

extern char *setlocale(int category, const char *locale);

#endif  // _LOCALE_H_
