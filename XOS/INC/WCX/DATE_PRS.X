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
//  Copyright (c) 1994-1995 Allegro Systems, Ltd.  All rights reserved.
//
//
//  Written by: SA Ortmann
//
//  Edit History:
//  -------------
//   8Feb95 (sao) - Initial development
//
//------------------------------------------------------------------------------

#ifndef _DATE_PRS_H_
#define _DATE_PRS_H_

#ifndef _BASELINE_H_

#include <baseline.h>

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
