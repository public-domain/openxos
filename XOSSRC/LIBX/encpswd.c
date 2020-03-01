//========================================================
// ENCPSWD - Function to encode password value for storage
// Written by John Goltz
//========================================================

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
#include <STDDEF.H>
#include <STRING.H>
#include <CTYPE.H>
#include <XOS.H>
#include <XOSSTR.H>
#include <XOSTIME.H>
#include <XOSSVC.H>
#include <XOSCRYPT.H>

// This function generates the 40 bit encoded password value.  This value is
//   formatted as follows:

//	Offset  Size    Description
//	   0      8   Salt value
//	   8     32   Encoded password

//   The salt value is an 8 byte random number.  It is used as the first 8
//   bytes of the message encrypted to give the encoded password.

// Weak key table - Includes the known DES weak, semi-weak, and possibly
//   weak keys.  Note that the values are all stored with the parity bit
//   (the low order bit) set to 0 since this is now we generate our keys)


long weakkeys[] =
{   0x00000000, 0x00000000,	// Weak keys
    0xFEFEFEFE, 0xFEFEFEFE,
    0x1E1E1E1E, 0x1E1E1E1E,
    0xE0E0E0E0, 0xE0E0E0E0,

    0x00FE00FE, 0x00FE00FE,	// Semi-weak keys
    0x1EE01EE0, 0x0EF00EF0,
    0x00E000E0, 0x00F000F0,
    0x1EFE1EFE, 0x0EFE0EFE,
    0x001E001E, 0x000E000E,
    0xE0FEE0FE, 0xF0FEF0FE,

    0xFE00FE00, 0xFE00FE00,
    0xE01EE01E, 0xF00EF00E,
    0xE000E000, 0xF000F000,
    0xFE1EFE1E, 0xFE0EFE0E,
    0x1E001E00, 0x0E000E00,
    0xFEE0FEE0, 0xFEF0FEF0,

    0x1E1E0000, 0x0E0E0000,	// Possibly weak keys
};

extern char genhashkey[];

//*****************************************************************
// Function: encodepassword - Generate encoded password value
// Returned: TRUE if OK, FALSE if password is unacceptable (results
//		in a "weak" initial encryption key
//*****************************************************************


int encodepassword(
    char *password,				// Password
    char *buffer)				// Encoded value (40 bytes)

{
    time_s datetime;
    char  *cpnt;
    char  *bpnt;
    int    notweak;						// TRUE if key is not weak
    int    cnt;
    char   chr;

    extern char main[];

    notweak = TRUE;
    svcSysDateTime(1, &datetime);		// Get current time of day
    cpnt = main + ((datetime.time >> 4) & 0xFFF);
    bpnt = buffer;						// Store 8 non-zero, non-0xFF random
    cnt = 8;							//   bytes
    do
    {
		while ((chr = *cpnt++) == 0 || chr == 0xFF)
			;
		*bpnt++ = chr;
    } while (--cnt > 0);

    qdesGenHash32(password, buffer, buffer + 8); // Hash the password
    if (checkweak(buffer + 32))			// Make sure last 8 bytes are not a
		notweak = FALSE;				//   weak key
    return (notweak);
}

//*************************************************
// Function: checkweak - Check for weak keys
// Returned: TRUE if key is weak, FALSE if not weak
//*************************************************

int checkweak(
    char *key)

{
    long *lpnt;
    int   cnt;

    lpnt = (long *)weakkeys;			// Check for a weak key
    cnt = sizeof(weakkeys)/8;
    do
    {
		if (lpnt[0] == ((long *)key)[0] || lpnt[1] == ((long *)key)[0])
			return (TRUE);
		lpnt += 2;
    } while (--cnt > 0);
    return (FALSE);
}
