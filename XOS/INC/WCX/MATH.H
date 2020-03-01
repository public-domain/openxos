//------------------------------------------------------------------------------
//
//  MATH.H - ANSI C header file for math functions
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

#ifndef _MATH_H_
#define _MATH_H_

# include <_prolog.h>

//#define HUGE_VAL                        // Unrepresentable object

//
// Function prototypes
//
double pow( double x, double y );

char *ecvt( double value, int numdigits, int *dec_ptr, int *sign_ptr );
char *fcvt( double value, int numdigits, int *dec_ptr, int *sign_ptr );



extern double acos(double x);

extern double asin(double x);

extern double atan(double x);

extern double atan2(double y, double x);

extern double ceil(double x);

extern double cos(double x);

extern double cosh(double x);

extern double exp(double x);

extern double fabs(double x);

extern double floor(double x);

extern double fmod(double x, double y);

extern double frexp(double value, int *exp);

extern double ldexp(double x, int exp);

extern double log(double x);

extern double log10(double x);

extern double modf(double value, double *iptr);

extern double pow(double x, double y);

extern double sin(double x);

extern double sinh(double x);

extern double sqrt(double x);

extern double tan(double x);

extern double tanh(double x);

# include <_epilog.h>

#endif  // _MATH_H_
