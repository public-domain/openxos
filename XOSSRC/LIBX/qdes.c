//====================================================//
// Module: QDES.C - Modified DES encryption functions //
//====================================================//

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

#include <STDIO.H>
#include <STRING.H>
#include <CTYPE.H>
#include <XOSQDES.H>
#include <XOSSWAP.H>

// Sofware DES functions
// written 12 Dec 1986 by Phil Karn, KA9Q; large sections adapted from
// the 1977 public-domain program by Jim Gillogly

// Permuted choice table (key)

static char pc1[] =
{   57, 49, 41, 33, 25, 17,  9,
     1, 58, 50, 42, 34, 26, 18,
    10,  2, 59, 51, 43, 35, 27,
    19, 11,  3, 60, 52, 44, 36,

    63, 55, 47, 39, 31, 23, 15,
     7, 62, 54, 46, 38, 30, 22,
    14,  6, 61, 53, 45, 37, 29,
    21, 13,  5, 28, 20, 12,  4
};

// Number left rotations of pc1

static char totrot[] = {1,2,4,6,8,10,12,14,15,17,19,21,23,25,27,28};

// Permuted choice key (table)

static char pc2[] =
{   14, 17, 11, 24,  1,  5,
     3, 28, 15,  6, 21, 10,
    23, 19, 12,  4, 26,  8,
    16,  7, 27, 20, 13,  2,
    41, 52, 31, 37, 47, 55,
    30, 40, 51, 45, 33, 48,
    44, 49, 39, 56, 34, 53,
    46, 42, 50, 36, 29, 32
};

// Combined S and P boxes

static long sp[8][64] =
{   0x00808200, 0x00000000, 0x00008000, 0x00808202,
    0x00808002, 0x00008202, 0x00000002, 0x00008000,
    0x00000200, 0x00808200, 0x00808202, 0x00000200,
    0x00800202, 0x00808002, 0x00800000, 0x00000002,
    0x00000202, 0x00800200, 0x00800200, 0x00008200,
    0x00008200, 0x00808000, 0x00808000, 0x00800202,
    0x00008002, 0x00800002, 0x00800002, 0x00008002,
    0x00000000, 0x00000202, 0x00008202, 0x00800000,
    0x00008000, 0x00808202, 0x00000002, 0x00808000,
    0x00808200, 0x00800000, 0x00800000, 0x00000200,
    0x00808002, 0x00008000, 0x00008200, 0x00800002,
    0x00000200, 0x00000002, 0x00800202, 0x00008202,
    0x00808202, 0x00008002, 0x00808000, 0x00800202,
    0x00800002, 0x00000202, 0x00008202, 0x00808200,
    0x00000202, 0x00800200, 0x00800200, 0x00000000,
    0x00008002, 0x00008200, 0x00000000, 0x00808002,
    0x40084010, 0x40004000, 0x00004000, 0x00084010,
    0x00080000, 0x00000010, 0x40080010, 0x40004010,
    0x40000010, 0x40084010, 0x40084000, 0x40000000,
    0x40004000, 0x00080000, 0x00000010, 0x40080010,
    0x00084000, 0x00080010, 0x40004010, 0x00000000,
    0x40000000, 0x00004000, 0x00084010, 0x40080000,
    0x00080010, 0x40000010, 0x00000000, 0x00084000,
    0x00004010, 0x40084000, 0x40080000, 0x00004010,
    0x00000000, 0x00084010, 0x40080010, 0x00080000,
    0x40004010, 0x40080000, 0x40084000, 0x00004000,
    0x40080000, 0x40004000, 0x00000010, 0x40084010,
    0x00084010, 0x00000010, 0x00004000, 0x40000000,
    0x00004010, 0x40084000, 0x00080000, 0x40000010,
    0x00080010, 0x40004010, 0x40000010, 0x00080010,
    0x00084000, 0x00000000, 0x40004000, 0x00004010,
    0x40000000, 0x40080010, 0x40084010, 0x00084000,
    0x00000104, 0x04010100, 0x00000000, 0x04010004,
    0x04000100, 0x00000000, 0x00010104, 0x04000100,
    0x00010004, 0x04000004, 0x04000004, 0x00010000,
    0x04010104, 0x00010004, 0x04010000, 0x00000104,
    0x04000000, 0x00000004, 0x04010100, 0x00000100,
    0x00010100, 0x04010000, 0x04010004, 0x00010104,
    0x04000104, 0x00010100, 0x00010000, 0x04000104,
    0x00000004, 0x04010104, 0x00000100, 0x04000000,
    0x04010100, 0x04000000, 0x00010004, 0x00000104,
    0x00010000, 0x04010100, 0x04000100, 0x00000000,
    0x00000100, 0x00010004, 0x04010104, 0x04000100,
    0x04000004, 0x00000100, 0x00000000, 0x04010004,
    0x04000104, 0x00010000, 0x04000000, 0x04010104,
    0x00000004, 0x00010104, 0x00010100, 0x04000004,
    0x04010000, 0x04000104, 0x00000104, 0x04010000,
    0x00010104, 0x00000004, 0x04010004, 0x00010100,
    0x80401000, 0x80001040, 0x80001040, 0x00000040,
    0x00401040, 0x80400040, 0x80400000, 0x80001000,
    0x00000000, 0x00401000, 0x00401000, 0x80401040,
    0x80000040, 0x00000000, 0x00400040, 0x80400000,
    0x80000000, 0x00001000, 0x00400000, 0x80401000,
    0x00000040, 0x00400000, 0x80001000, 0x00001040,
    0x80400040, 0x80000000, 0x00001040, 0x00400040,
    0x00001000, 0x00401040, 0x80401040, 0x80000040,
    0x00400040, 0x80400000, 0x00401000, 0x80401040,
    0x80000040, 0x00000000, 0x00000000, 0x00401000,
    0x00001040, 0x00400040, 0x80400040, 0x80000000,
    0x80401000, 0x80001040, 0x80001040, 0x00000040,
    0x80401040, 0x80000040, 0x80000000, 0x00001000,
    0x80400000, 0x80001000, 0x00401040, 0x80400040,
    0x80001000, 0x00001040, 0x00400000, 0x80401000,
    0x00000040, 0x00400000, 0x00001000, 0x00401040,
    0x00000080, 0x01040080, 0x01040000, 0x21000080,
    0x00040000, 0x00000080, 0x20000000, 0x01040000,
    0x20040080, 0x00040000, 0x01000080, 0x20040080,
    0x21000080, 0x21040000, 0x00040080, 0x20000000,
    0x01000000, 0x20040000, 0x20040000, 0x00000000,
    0x20000080, 0x21040080, 0x21040080, 0x01000080,
    0x21040000, 0x20000080, 0x00000000, 0x21000000,
    0x01040080, 0x01000000, 0x21000000, 0x00040080,
    0x00040000, 0x21000080, 0x00000080, 0x01000000,
    0x20000000, 0x01040000, 0x21000080, 0x20040080,
    0x01000080, 0x20000000, 0x21040000, 0x01040080,
    0x20040080, 0x00000080, 0x01000000, 0x21040000,
    0x21040080, 0x00040080, 0x21000000, 0x21040080,
    0x01040000, 0x00000000, 0x20040000, 0x21000000,
    0x00040080, 0x01000080, 0x20000080, 0x00040000,
    0x00000000, 0x20040000, 0x01040080, 0x20000080,
    0x10000008, 0x10200000, 0x00002000, 0x10202008,
    0x10200000, 0x00000008, 0x10202008, 0x00200000,
    0x10002000, 0x00202008, 0x00200000, 0x10000008,
    0x00200008, 0x10002000, 0x10000000, 0x00002008,
    0x00000000, 0x00200008, 0x10002008, 0x00002000,
    0x00202000, 0x10002008, 0x00000008, 0x10200008,
    0x10200008, 0x00000000, 0x00202008, 0x10202000,
    0x00002008, 0x00202000, 0x10202000, 0x10000000,
    0x10002000, 0x00000008, 0x10200008, 0x00202000,
    0x10202008, 0x00200000, 0x00002008, 0x10000008,
    0x00200000, 0x10002000, 0x10000000, 0x00002008,
    0x10000008, 0x10202008, 0x00202000, 0x10200000,
    0x00202008, 0x10202000, 0x00000000, 0x10200008,
    0x00000008, 0x00002000, 0x10200000, 0x00202008,
    0x00002000, 0x00200008, 0x10002008, 0x00000000,
    0x10202000, 0x10000000, 0x00200008, 0x10002008,
    0x00100000, 0x02100001, 0x02000401, 0x00000000,
    0x00000400, 0x02000401, 0x00100401, 0x02100400,
    0x02100401, 0x00100000, 0x00000000, 0x02000001,
    0x00000001, 0x02000000, 0x02100001, 0x00000401,
    0x02000400, 0x00100401, 0x00100001, 0x02000400,
    0x02000001, 0x02100000, 0x02100400, 0x00100001,
    0x02100000, 0x00000400, 0x00000401, 0x02100401,
    0x00100400, 0x00000001, 0x02000000, 0x00100400,
    0x02000000, 0x00100400, 0x00100000, 0x02000401,
    0x02000401, 0x02100001, 0x02100001, 0x00000001,
    0x00100001, 0x02000000, 0x02000400, 0x00100000,
    0x02100400, 0x00000401, 0x00100401, 0x02100400,
    0x00000401, 0x02000001, 0x02100401, 0x02100000,
    0x00100400, 0x00000000, 0x00000001, 0x02100401,
    0x00000000, 0x00100401, 0x02100000, 0x00000400,
    0x02000001, 0x02000400, 0x00000400, 0x00100001,
    0x08000820, 0x00000800, 0x00020000, 0x08020820,
    0x08000000, 0x08000820, 0x00000020, 0x08000000,
    0x00020020, 0x08020000, 0x08020820, 0x00020800,
    0x08020800, 0x00020820, 0x00000800, 0x00000020,
    0x08020000, 0x08000020, 0x08000800, 0x00000820,
    0x00020800, 0x00020020, 0x08020020, 0x08020800,
    0x00000820, 0x00000000, 0x00000000, 0x08020020,
    0x08000020, 0x08000800, 0x00020820, 0x00020000,
    0x00020820, 0x00020000, 0x08020800, 0x00000800,
    0x00000020, 0x08020020, 0x00000800, 0x00020820,
    0x08000800, 0x00000020, 0x08000020, 0x08020000,
    0x08020020, 0x08000000, 0x00020000, 0x08000820,
    0x00000000, 0x08020820, 0x00020020, 0x08000020,
    0x08020000, 0x08000800, 0x08000820, 0x00000000,
    0x08020820, 0x00020800, 0x00020800, 0x00000820,
    0x00000820, 0x00020020, 0x08000000, 0x08020800
};

static int bytebit[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

static void decqdes(char *block, uchar keysched[16][8]);
static void encqdes(char *block, uchar keysched[16][8]);
static long f(ulong r, uchar subkey[8]);

//***************************************************************
// Function: qdesSetKey - Set up key for encryption or decryption
//				(initializes key schedule array)
// Returned: Nothing
//***************************************************************

void qdesSetKey(
    char      *key,
	QDESSTATE *qst)

{
    char pc1m[56];				// Place to modify pc1 into
    char pcr[56];				// Place to rotate pc1 into
    int  i;
    int  j;
    int  l;
    int  m;

    memset(qst->keysched, 0, 128);		// Clear the key schedule
    j = 0;
    do
    {									// Convert pc1 to bits of key
		l = pc1[j] - 1;					// Integer bit location
		m = l & 0x07;					// Find bit
		pc1m[j] = (key[l>>3] & bytebit[m])? 1: 0;
										// Find which key byte l is in and which
										//   bit of that byte and store 1-bit
    } while (++j < 56);					//   result
    i = 0;
    do
    {									// Key chunk for each iteration
		j = 0;
		do
		{								// Rotate pc1 the right amount
			pcr[j] = pc1m[(l = j + totrot[i]) <(j < 28? 28: 56) ? l: l - 28];
										// Rotate left and right halves
		} while (++j < 56);				//   independently

		j = 0;
		do
		{								// Select bits individually
										// Check bit that goes to keysched[j]
			if (pcr[pc2[j] - 1])
			{							// Mask it in if it's there
				l = j % 6;
				qst->keysched[i][j/6] |= bytebit[l] >> 2;
			}
		} while (++j < 48);
    } while (++i < 16);
}

//*********************************************************
// Function: qdesReset - Reset encryption/decryption stream
// Returned: Nothing
//*********************************************************

void qdesReset(
	QDESSTATE *qst)

{
    *((long *)(qst->cv)) = *((long *)(qst->cv + 4)) = 0;
}										// Clear the CBC vector

//**********************************************************
// Function: qdesEncBlock - Encrypt a block of data in place
// Returned: Length of encrypted block
//**********************************************************

// Buffer must be large enough to accept the resulting encrypted
//   block.  The length may be rounded up mod 8.

int qdesEncBlock(
    char      *data,
    int        size,
	QDESSTATE *qst)

{
    int   cnt;

    cnt = size = (size + 7) >> 3;		// Get number of 8 byte blocks
	do									//   (round up)
	{
		*((ulong *)(data + 0)) ^= *((ulong *)(qst->cv + 0));
		*((ulong *)(data + 4)) ^= *((ulong *)(qst->cv + 4));
										// Chain in last cipher word
		encqdes(data, qst->keysched);	// Encrypt 8-byte block
		*((ulong *)(qst->cv + 0)) = *((ulong *)(data + 0));
		*((ulong *)(qst->cv + 4)) = *((ulong *)(data + 4));
		data += 8;						// Bump the data pointer
    } while (--cnt > 0);				// Continue if more to do
    return (size);
}

//**********************************************************
// Function: qdesDecBlock - Decrypt a block of data in place
// Returned: Nothing
//**********************************************************

// Size of block must be a multiple of 8

void qdesDecBlock(
    char      *data,
    int        size,
	QDESSTATE *qst)

{
    char  cvtmp[8];

    size >>= 3;							// Get number of blocks
    do
    {		
		*((ulong *)(cvtmp + 0)) = *((ulong *)(data + 0)); // Save the
		*((ulong *)(cvtmp + 4)) = *((ulong *)(data + 4)); //   cyphertext
		decqdes(data, qst->keysched);	// Decrypt the block
		*((ulong *)(data + 0)) ^= *((ulong *)(qst->cv + 0));
		*((ulong *)(data + 4)) ^= *((ulong *)(qst->cv + 4));
		*((ulong *)(qst->cv + 0)) = *((ulong *)(cvtmp + 0));
		*((ulong *)(qst->cv + 4)) = *((ulong *)(cvtmp + 4));
		data += 8;
    } while (--size > 0);
}

//*******************************************************
// Function: qdesGenHash32 - Generate a 32-byte hash code
// Returned: Nothing
//*******************************************************

// This function encrypts a 32-byte message (hashmsg) using the specified
//   32-byte password as a key.  The first 8 bytes of the message are encrypted
//   using the first 8 bytes of the password as a key, the second 8 bytes of
//   the message are encrypted using the second 8 bytes of the password as a
//   key, etc.  All password bytes are shifed one byte to the left.  CBC mode
//   is used with chaining between blocks even though the key is changed.

void qdesGenHash32(
   char *password,				// Pointer to the password string
   char *salt,					// Pointer to the 8 byte salt value
   char *data)					// Pointer to 32 byte data buffer

{
	QDESSTATE qstate;
    char     *kpnt;
    char     *ppnt;
    int       cnt;
    char      chr;
    uchar     keys[32];

    static const uchar pswdmsg[24] =
	{	0xFF, 0x11, 0x00, 0x51, 0x99, 0xFF, 0x77, 0x32,
		0x00, 0x99, 0xFF, 0x62, 0x27, 0x00, 0x11, 0x00,
		0x88, 0x28, 0x31, 0x77, 0xAA, 0xC9, 0x00, 0x00
    };
	static const uchar pswdmask[32] =
	{	0x36, 0xF6, 0x04, 0x00, 0x74, 0x0E, 0x36, 0x80,
		0x04, 0xA8, 0x10, 0x74, 0xF0, 0xBA, 0x02, 0x8A,
		0x66, 0x20, 0x36, 0x84, 0xFE, 0x00, 0x42, 0xA8,
		0x16, 0x14, 0x10, 0x02, 0xE0, 0x90, 0x72, 0x98
    };

    if (password == NULL || password[0] == 0) // Null password?
		password = "null password";		// Yes - make it useable!
	*((ulong *)(data + 0)) = *((ulong *)(salt + 0)); // Set up the message
	*((ulong *)(data + 4)) = *((ulong *)(salt + 0)); //   we will encrypt
    memcpy(data + 8, pswdmsg, 24);
    memcpy(keys, pswdmask, 32);			// Initialize the keys from our data
    ppnt = password;					//   and his password
    kpnt = keys;
    cnt = 32;
    do
	{
		if ((chr = *ppnt++) == 0)
		{
			chr = 0x29;
			ppnt = password;
		}
		*kpnt++ ^= (chr << 1);
    } while (--cnt > 0);
    qdesReset(&qstate);
	qdesSetKey(keys + 0, &qstate);
	qdesEncBlock(data + 0, 8, &qstate);
	qdesSetKey(keys + 8, &qstate);
	qdesEncBlock(data + 8, 8, &qstate);
	qdesSetKey(keys + 16, &qstate);
	qdesEncBlock(data + 16, 8, &qstate);
	qdesSetKey(keys + 24, &qstate);
	qdesEncBlock(data + 24, 8, &qstate);
}

//********************************************************
// Function: encqdes - In-place encryption of 64-bit block
// Returned: Nothing
//********************************************************

static void encqdes(
    char *block,
    uchar keysched[16][8])

{
    int  num;
    long tmp;

    ((long *)block)[0] = swaplong(((long *)block)[0]);
    ((long *)block)[1] = swaplong(((long *)block)[1]);
    num = 0;							// Do the 16 rounds
    do
    {
		((long *)block)[0] ^= f(((long *)block)[1], keysched[num++]);
		((long *)block)[1] ^= f(((long *)block)[0], keysched[num++]);
    } while (num < 16);
    tmp = ((long *)block)[0];			// Swap left and right halves
    ((long *)block)[0] = swaplong(((long *)block)[1]);
    ((long *)block)[1] = swaplong(tmp);
}

//********************************************************
// Function: decqdes - In-place decryption of 64-bit block
// Returned: Nothing
//********************************************************

static void decqdes(
    char *block,
    uchar keysched[16][8])

{
    int  num;
    long tmp;

    tmp = swaplong(((long *)block)[0]);	// Swap left and right halves
    ((long *)block)[0] = swaplong(((long *)block)[1]);	
    ((long *)block)[1] = tmp;
    num = 15;							// Do the 16 rounds in reverse order
    do
    {
		((long *)block)[1] ^= f(((long *)block)[0], keysched[num--]);
		((long *)block)[0] ^= f(((long *)block)[1], keysched[num--]);
    } while (num >= 0);
		((long *)block)[0] = swaplong(((long *)block)[0]);
		((long *)block)[1] = swaplong(((long *)block)[1]);
}

//**************************************************************
// Function: f - The nonlinear function f(r,k), the heart of DES
// Returned: ???
//**************************************************************

static long f(
    ulong r,					// 32 bits
    uchar subkey[8])			// 48-bit key for this round

{
    ulong rval;
    ulong rt;

#ifdef TRACE
    uchar *cp;
    int    i;

    printf("f(%08lx, %02x %02x %02x %02x %02x %02x %02x %02x) = ", r,
			subkey[0], subkey[1], subkey[2], subkey[3], subkey[4], subkey[5],
			subkey[6], subkey[7]);
#endif

    // Run E(R) ^ K through the combined S & P boxes
    // This code takes advantage of a convenient regularity in
    // E, namely that each group of 6 bits in E(R) feeding
    // a single S-box is a contiguous segment of R.

    rt = (r >> 1) | ((r & 1)? 0x80000000: 0);
    rval = sp[0][((rt >> 26) ^ subkey[0]) & 0x3f] |
			sp[1][((rt >> 22) ^ subkey[1]) & 0x3f] |
			sp[2][((rt >> 18) ^ subkey[2]) & 0x3f] |
			sp[3][((rt >> 14) ^ subkey[3]) & 0x3f] |
			sp[4][((rt >> 10) ^ subkey[4]) & 0x3f] |
			sp[5][((rt >> 6) ^ subkey[5]) & 0x3f] |
			sp[6][((rt >> 2) ^ subkey[6]) & 0x3f];
    rt = (r << 1) | ((r & 0x80000000)? 1: 0);
    rval |= sp[7][(rt ^ subkey[7]) & 0x3f];

#ifdef TRACE
    printf(" %08lx\n",rval);
#endif

    return (rval);
}
