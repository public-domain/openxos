//***************************
// Utility routines for XLINK
//***************************
// Written by John Goltz
//***************************

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <PROCARG.H>
#include <XOS.H>
#include "XLINK.H"
#include "XLINKEXT.H"

// Allocate local data

int curhash;			// Current hash table index

//*************************************************************
// Function: looksym - Look for symbol in symbol table
// Returned: Pointer to symbol table entry or NULL if not found
//*************************************************************

struct sy *looksym(void)

{
    int    cnt;
    struct sy *pnt;
    short  hindex;
    short *hpnt;
    char  *pnt1;
    char  *pnt2;

    cnt = (symsize+1)/2;
    hpnt = (short *)symbuf;
    hindex = 0;
    while (--cnt >= 0)
    {
	hindex <<= 1;
	if (hindex < 0)
	    ++hindex;
	hindex ^= *hpnt++;
    }
    curhash = hindex & HASHMASK;
    if ((pnt = hashtbl[curhash]) != NULL)
    {
        do
        {
            cnt = symsize + 1;		// Check this entry
            pnt1 = symbuf;
            pnt2 = pnt->sy_name;
            while (--cnt >= 0)
            {
                if (*pnt1++ != *pnt2++)
                    break;
            }
            if (cnt < 0 && (pnt->sy_mod == NULL || pnt->sy_mod == curmod))
                return (pnt);
        } while ((pnt=pnt->sy_hash) != NULL);
    }
    return (NULL);			// Not found if get here
}

//*************************************************************
// Function: looklocsym - Look for local symbol in symbol table
// Returned: Pointer to symbol table entry or NULL if not found
//*************************************************************

struct sy *looklocsym(void)

{
    int    cnt;
    struct sy *pnt;
    short  hindex;
    short *hpnt;
    char  *pnt1;
    char  *pnt2;

    cnt = (symsize+1)/2;
    hpnt = (short *)symbuf;
    hindex = 0;
    while (--cnt >= 0)
    {
	hindex <<= 1;
	if (hindex < 0)
	    ++hindex;
	hindex ^= *hpnt++;
    }
    curhash = hindex & HASHMASK;
    if ((pnt = hashtbl[curhash]) != NULL)
    {
        do
        {
            cnt = symsize + 1;		// Check this entry
            pnt1 = symbuf;
            pnt2 = pnt->sy_name;
            if (pnt->sy_mod == curmod)
            {
                while (--cnt >= 0)
                {
                    if (*pnt1++ != *pnt2++)
                        break;
                }
                if (cnt < 0)
                    return (pnt);
            }
        } while ((pnt=pnt->sy_hash) != NULL);
    }
    return (NULL);			// Not found if get here
}

//***************************************************
// Function: enetersym - Enter symbol in symbol table
// Returned: Pointer to symbol table entry
//***************************************************

struct sy *entersym(void)

{
    struct sy *pnt;

    pnt = (struct sy *)getspace((long)(symsize + 1 + offsetof(sy, sy_name)));
					// Get memory for symbol table entry
    pnt->sy_hash = hashtbl[curhash];	// Link into hash list
    pnt->sy_mod = NULL;
    pnt->sy_export = NULL;
	pnt->sy_weak = NULL;
    hashtbl[curhash] = pnt;
    strcpy(pnt->sy_name, symbuf);	// Copy symbol name to entry
    return (pnt);			// Finished
}

//***************************************************
// Function: getpsd - Get address of psect data block
// Returned: Pointer to psect data block
//***************************************************

struct pd *getpsd(
    int local)			// Local psect number

{
    if (local > curmod->mb_psectnummax)	// Valid psect number?
    {					// No - fail!
        char buf[50];

        sprintf(buf, "Illegal psect number %d", local);
	fail(buf);
    }
    return (curmod->mb_psectnumtbl[local-1]); // Yes - return address of
}					      //   psect data block

//***************************************************
// Function: getmsd - Get address of msect data block
// Returned: Pointer to msect data block
//***************************************************

struct md *getmsd(
    int local)			// Local msect number

{
    if (local > curmod->mb_msectnummax)	// Valid msect number?
    {					// No - fail
        char buf[50];

        sprintf(buf, "Illegal msect number %d", local);
	fail(buf);
    }
    return (curmod->mb_msectnumtbl[local-1]); // Yes - return address of
}					      //   msect data block

//*****************************************************
// Function: getsgd - Get address of segment data block
// Returned: Pointer to segment data block
//*****************************************************

struct sd *getsgd(
    int local)			// Local segment number

{
    if (local > curmod->mb_segnummax)	// Valid segment number?
    {					// No - fail!
        char buf[50];

        sprintf(buf, "Illegal segment number %d", local);
	fail(buf);
    }
    return (curmod->mb_segnumtbl[local-1]); // Yes - return address of
}					    //   segment data block

//*************************************
// Function: clrbuf - Clear disk buffer
// Returned: Nothing
//*************************************

void clrbuf(
    long *buffer,
    int   size)

{
    size >>= 2;
    do
    {   *buffer++ = 0;
    } while (--size > 0);
}

//*************************************************************
// Function: vallength - Get number of bytes required for value
// Returned: Number of bytes
//*************************************************************

int vallength(
    long value)

{
    int rtn;

    rtn = 0;
    if (value == 0)
        return (0);
    if ((value & 0xFFFFFF00) == 0 || (value & 0xFFFFFF00) == 0xFFFFFF00)
        return (1);
    if ((value & 0xFFFF0000) == 0 || (value & 0xFFFF0000) == 0xFFFF0000)
        return (2);
    return (4);
}

//***************************************
// Function: outname - Output name string
// Returned: Nothing
//***************************************

void outname(
    char *pnt)

{
    char chr;

    for (;;)
    {
        chr = *pnt++;
        if (*pnt == 0)
            break;
        putbyte(chr);
    }
    putbyte(chr | 0x80);
}
