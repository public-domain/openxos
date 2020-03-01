////////////////////////////////////////////////////////////////////////////////
//
//  DISMOUNT.H - Header file for DISMOUNT utility
//
//  Copyright (c) 1995 Allegro Systems, Ltd. All rights reserved.
//
//  Edit history:
//  ------------
//  16Jun95 (fpj) - Original creation.
//
////////////////////////////////////////////////////////////////////////////////

#define PROGRAM     DISMOUNT            // Program name
#define VERSION     3                   // Program version number
#define EDIT        4                   // Program edit number

#define string(x)   stringx(x)
#define stringx(x)  #x

#define ERRBITS             (EM_IQM | EM_NAME | EM_CODE | EM_TEXT | EM_EXIT)
                                        // Bits to use for errmsg() calls

extern char  prgname[];

// Here are our configuration options

extern char *disk_name;                 // Name of disk to dismount

extern long quiet;                      // TRUE if /QUIET specified
extern long mute;                       // TRUE if /MUTE specified


// Function prototypes

extern void getarg(char *argv[]);
