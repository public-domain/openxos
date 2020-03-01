//------------------------------------------------------------------------------
//
// DATE_PRS
// A date and time parser utility
// Acceptable date formats are:
//	US		mm/dd/[[yy]yy]
//	INTL	dd.mm.[[yy]yy]
//	TEXT	dd[-]mmm[-][[yy]yy]  or
//			mmm[-]dd[-][[yy]yy]
//	PACK	yymmdd (all six values must be present)
//	TIME	hh:mm[:ss][A|P]
//
// note: 	If the year is not included, current year is used
//			if century is not included, current century is used
//
//  Written by: SA Ortmann
//
//  Edit History:
//  -------------
//   8Feb95 (sao) - Initial development
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

#ifndef _DATE_PRS_H_
#define _DATE_PRS_H_

#ifndef _BASELINE_H_

#include <baseline.h>
#include <ALGTIME.H>

#endif // _BASELINE_H_


#define INVDTFORMAT	1
#define INVMONTH	2
#define INVDAY  	3
#define INVDTRANGE	4
#define INVTMMIN	5
#define INVTMHOUR	6
#define INVTMSECOND	7
#define INVTMFORMAT	8

extern int dt_parse(char *inp_str, time_s *dt );

// the following routine is included for test purposes only
void date_prt( time_d * );

#endif  // _DATE_PRS_H_
