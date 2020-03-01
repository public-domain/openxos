//------------------------------------------------------------------------------
//
//  ERRMSG.H - Definitions for error message function
//
//  Copyright (c) 1994-1995 Allegro Systems, Ltd.  All rights reserved.
//
//
//  Edit History:
//  -------------
//  18May95 (fpj) - Added comment block.
//
//------------------------------------------------------------------------------

#ifndef _ERRMSG_H_
#define _ERRMSG_H_

#ifndef _BASELINE_H_

#include <baseline.h>

#endif // _BASELINE_H_


/* Definitions for ERRMSG.C */

#define EM_INL  0x0001	/* Include initial NL */
#define EM_IQM  0x0002	/* Include initial ? */
#define EM_IPC  0x0004	/* Include initial % */
#define EM_NAME 0x0008	/* Include program name */
#define EM_CODE 0x0010	/* Include error code */
#define EM_TEXT 0x0020	/* Include error message text */
#define EM_FNL  0x0040	/* Include final NL */
#define EM_EXIT 0x8000	/* Terminate after displaying message */

extern void errmsg(int bits, char *buffer, long code, char *msg1, char *msg2);

extern void femsg(char *name, long code, char *file);

extern void femsg2(char *name, char *msg, long code, char *file);

#endif  // _ERRMSG_H_
