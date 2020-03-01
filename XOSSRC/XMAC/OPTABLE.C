//***********************
// Op-code table for XMAC
//***********************
// Written by John Goltz
//***********************

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
#include <XOS.H>
#include "XMAC.H"

// Format of each table entry is:
//	unsigned char  name[6];	Opcode (excluding first character)
//	void  (*E)(disp);	Address of routine for op-code
//	unsigned char val1;	Normal value for first byte
//	unsigned char val2;	First alternate value for first byte
//	unsigned char val3;	Second alternate value for first byte
//	unsigned char val4;	Normal value for second byte
//	unsigned char size;	Operation size for instruction
//	unsigned char flag;	Flag bits

// Define symbols used here for the processor bits

#define P0 (B_8086 |B_80186|B_80286|B_80386|B_80486)
#define P1 (B_80186|B_80286|B_80386|B_80486)
#define P2 (B_80286|B_80386|B_80486)
#define P3 (B_80386|B_80486)
#define P4 (B_80486)

// Define symbols used here for the flag bits

#define BO OF_BO
#define RF OF_RF
#define BS OF_BS
#define WS OF_WS
#define SS OF_SS
#define B3 OF_B3
#define MS OF_MS
#define RS OF_RS
#define MD OF_MD
#define RD OF_RD
#define XP OF_XP
#define NS OF_NS
#define DB OF_DB
#define SB OF_SB
#define BI OF_BI

struct optbl opctbla[] =
{{{'A','A',' ',' ',' ',' '},ionebyte,0x37,0x00,0x00,0x00,0,P0,0,0},
 {{'A','D',' ',' ',' ',' '},itwobyte,0xD5,0x0A,0x00,0x00,0,P0,0,0},
 {{'A','M',' ',' ',' ',' '},itwobyte,0xD4,0x0A,0x00,0x00,0,P0,0,0},
 {{'A','S',' ',' ',' ',' '},ionebyte,0x3F,0x00,0x00,0x00,0,P0,0,0},
 {{'D','C','B',' ',' ',' '},itwoopr ,0x10,0x80,0x10,0x14,1,P0,0,RD|MD|RS|MS|DB|SB},
 {{'D','C','L',' ',' ',' '},itwoopr ,0x11,0x81,0x10,0x15,4,P3,0,RD|MD|RS|MS|DB|SB},
 {{'D','C','W',' ',' ',' '},itwoopr ,0x11,0x81,0x10,0x15,2,P0,0,RD|MD|RS|MS|DB|SB},
 {{'D','D','B',' ',' ',' '},itwoopr ,0x00,0x80,0x00,0x04,1,P0,0,RD|MD|RS|MS|DB|SB},
 {{'D','D','L',' ',' ',' '},itwoopr ,0x01,0x81,0x00,0x05,4,P3,0,RD|MD|RS|MS|DB|SB},
 {{'D','D','W',' ',' ',' '},itwoopr ,0x01,0x81,0x00,0x05,2,P0,0,RD|MD|RS|MS|DB|SB},
 {{'N','D','B',' ',' ',' '},itwoopr ,0x20,0x80,0x20,0x24,1,P0,0,RD|MD|RS|MS|DB|SB},
 {{'N','D','L',' ',' ',' '},itwoopr ,0x21,0x81,0x20,0x25,4,P3,0,RD|MD|RS|MS|DB|SB},
 {{'N','D','W',' ',' ',' '},itwoopr ,0x21,0x81,0x20,0x25,2,P0,0,RD|MD|RS|MS|DB|SB},
 {{'R','P','L',' ',' ',' '},itwoopr ,0x63,0x00,0x00,0x00,2,P0,0,RD|MD|RS|NS}
};

struct optbl opctblb[] =
{{{'O','U','N','D',' ',' '},itwoopr ,0x62,0x00,0x00,0x00,2,P0,0,RD|MS},
 {{'S','F','L',' ',' ',' '},itwoopr ,0xBC,0x00,0x00,0x00,4,P3,0,RD|RS|MS|RF|XP},
 {{'S','F','W',' ',' ',' '},itwoopr ,0xBC,0x00,0x00,0x00,2,P3,0,RD|RS|MS|RF|XP},
 {{'S','R','L',' ',' ',' '},itwoopr ,0xBD,0x00,0x00,0x00,4,P3,0,RD|RS|MS|RF|XP},
 {{'S','R','W',' ',' ',' '},itwoopr ,0xBD,0x00,0x00,0x00,2,P3,0,RD|RS|MS|RF|XP},
 {{'S','W','A','P','L',' '},ioneopr ,0xC8,0x00,0x00,0x00,4,P4,0,RS|XP},
 {{'T','L',' ',' ',' ',' '},itwoopr ,0xA3,0xBA,0x20,0x00,4,P3,0,RD|MD|RS|BI|XP},
 {{'T','C','L',' ',' ',' '},itwoopr ,0xBB,0xBA,0x38,0x00,4,P3,0,RD|MD|RS|BI|XP},
 {{'T','C','W',' ',' ',' '},itwoopr ,0xBB,0xBA,0x38,0x00,2,P3,0,RD|MD|RS|BI|XP},
 {{'T','Z','L',' ',' ',' '},itwoopr ,0xB3,0xBA,0x30,0x00,4,P3,0,RD|MD|RS|BI|XP},
 {{'T','Z','W',' ',' ',' '},itwoopr ,0xB3,0xBA,0x30,0x00,2,P3,0,RD|MD|RS|BI|XP},
 {{'T','S','L',' ',' ',' '},itwoopr ,0xAB,0xBA,0x28,0x00,4,P3,0,RD|MD|RS|BI|XP},
 {{'T','S','W',' ',' ',' '},itwoopr ,0xAB,0xBA,0x28,0x00,2,P3,0,RD|MD|RS|BI|XP},
 {{'T','W',' ',' ',' ',' '},itwoopr ,0xA3,0xBA,0x20,0x00,2,P3,0,RD|MD|RS|BI|XP}
};

struct optbl opctblc[] =
{{{'A','L','L',' ',' ',' '},ibranch ,0x00,0x00,0xE8,0x00,0,P0,0,SS},
 {{'A','L','L','F','I',' '},ioneopr ,0xFF,0x00,0x00,0x18,0,P0,0,MS|SS},
 {{'A','L','L','F',' ',' '},icallf  ,0x00,0x00,0x00,0x00,0,P0,0,SS},
 {{'A','L','L','I',' ',' '},ioneopr ,0xFF,0x00,0x00,0x10,0,P0,0,SS|MS|RS},
 {{'B','W',' ',' ',' ',' '},ionebytx,0x98,0x00,0x00,0x00,2,P0,0,0},
 {{'L','C',' ',' ',' ',' '},ionebyte,0xF8,0x00,0x00,0x00,0,P0,0,0},
 {{'L','D',' ',' ',' ',' '},ionebyte,0xFC,0x00,0x00,0x00,0,P0,0,0},
 {{'L','R','B',' ',' ',' '},iclear  ,0x32,0x00,0x00,0x00,1,P0,0,0},
 {{'L','R','L',' ',' ',' '},iclear  ,0x33,0x00,0x00,0x00,4,P3,0,0},
 {{'L','R','W',' ',' ',' '},iclear  ,0x33,0x00,0x00,0x00,2,P0,0,0},
 {{'L','I',' ',' ',' ',' '},ionebyte,0xFA,0x00,0x00,0x00,0,P0,0,0},
 {{'L','Q',' ',' ',' ',' '},ionebytx,0x99,0x00,0x00,0x00,4,P0,0,0},
 {{'L','T','S',' ',' ',' '},itwobyte,0x0F,0x06,0x00,0x00,0,P0,0,0},
 {{'M','C',' ',' ',' ',' '},ionebyte,0xF5,0x00,0x00,0x00,0,P0,0,0},
 {{'M','P','B',' ',' ',' '},itwoopr ,0x38,0x80,0x38,0x3C,1,P0,0,RD|MD|RS|MS|DB|SB},
 {{'M','P','L',' ',' ',' '},itwoopr ,0x39,0x81,0x38,0x3D,4,P3,0,RD|MD|RS|MS|DB|SB},
 {{'M','P','S','B',' ',' '},istrins ,0xA6,0x00,0x00,0x00,1,P0,0,MD|MS},
 {{'M','P','S','L',' ',' '},istrins ,0xA7,0x00,0x00,0x00,4,P0,0,MD|MS},
 {{'M','P','S','W',' ',' '},istrins ,0xA7,0x00,0x00,0x00,2,P0,0,MD|MS},
 {{'M','P','W',' ',' ',' '},itwoopr ,0x39,0x81,0x38,0x3D,2,P0,0,RD|MD|RS|MS|DB|SB},
 {{'M','P','X','C','G','B'},itwoopr ,0xA6,0x00,0x00,0x00,1,P4,0,RD|MD|RS|XP},
 {{'M','P','X','C','G','W'},itwoopr ,0xA7,0x00,0x00,0x00,2,P4,0,RD|MD|RS|XP},
 {{'M','P','X','C','G','L'},itwoopr ,0xA7,0x00,0x00,0x00,4,P4,0,RD|MD|RS|XP},
 {{'W','D',' ',' ',' ',' '},ionebytx,0x99,0x00,0x00,0x00,2,P0,0,0},
 {{'W','L',' ',' ',' ',' '},ionebytx,0x98,0x00,0x00,0x00,4,P0,0,0}
};

struct optbl opctbld[] =
{{{'A','A',' ',' ',' ',' '},ionebyte,0x27,0x00,0x00,0x00,0,P0,0,0},
 {{'A','S',' ',' ',' ',' '},ionebyte,0x2F,0x00,0x00,0x00,0,P0,0,0},
 {{'E','C','B',' ',' ',' '},iincdec ,0xFE,0x00,0x00,0x08,1,P0,0,0},
 {{'E','C','L',' ',' ',' '},iincdec ,0xFF,0x48,0x00,0x08,4,P3,0,0},
 {{'E','C','W',' ',' ',' '},iincdec ,0xFF,0x48,0x00,0x08,2,P0,0,0},
 {{'I','V','B',' ',' ',' '},ioneopr ,0xF6,0x00,0x00,0x30,1,P0,0,MS|RS},
 {{'I','V','L',' ',' ',' '},ioneopr ,0xF7,0x00,0x00,0x30,4,P3,0,MS|RS},
 {{'I','V','W',' ',' ',' '},ioneopr ,0xF7,0x00,0x00,0x30,2,P0,0,MS|RS}
};

struct optbl opctble[] =
{{{'N','T','E','R',' ',' '},ientrins,0xC8,0x00,0x00,0x00,0,P1,0,0},
 {{'S','C',' ',' ',' ',' '},ionebyte,0xD8,0xF8,0x00,0x00,0,P0,0,0}
};

struct optbl opctblf[] =
{{{'2','X','M','1',' ',' '},itwobyte,0xD9,0xF0,0x00,0x00,0,P0,0,0},
 {{'4','X','4',' ',' ',' '},itwobyte,0xD9,0xF1,0x00,0x00,0,P0,0,0},
 {{'A','B','S',' ',' ',' '},itwobyte,0xD9,0xE1,0x00,0x00,0,P0,0,0},
 {{'A','D','D',' ',' ',' '},itwooprf,0xD8,0xC0,0xDE,0x00,0,P0,0,DB},
 {{'A','D','D','F',' ',' '},ioneopr ,0xD8,0x00,0x00,0x00,0,P0,0,MS},
 {{'A','D','D','D',' ',' '},ioneopr ,0xDC,0x00,0x00,0x00,0,P0,0,MS},
 {{'A','D','D','L',' ',' '},ioneopr ,0xDA,0x00,0x00,0x00,0,P0,0,MS},
 {{'A','D','D','P',' ',' '},itwooprf,0xDE,0xC0,0xDE,0x00,0,P0,0,0},
 {{'A','D','D','Q',' ',' '},ioneopr ,0xDE,0x00,0x00,0x00,0,P0,0,MS},
 {{'B','L','D',' ',' ',' '},ioneopr ,0xDF,0x00,0x00,0x20,0,P0,0,MS},
 {{'B','S','T','P',' ',' '},ioneopr ,0xDF,0x00,0x00,0x30,0,P0,0,MS},
 {{'C','H','S',' ',' ',' '},itwobyte,0xD9,0xE0,0x00,0x00,0,P0,0,0},
 {{'C','L','E','X',' ',' '},ithrbyte,0x9B,0xDB,0xE2,0x00,0,P0,0,0},
 {{'C','O','M',' ',' ',' '},itwooprf,0xD8,0xD0,0xD8,0x00,0,P0,0,0},
 {{'C','O','M','D',' ',' '},ioneopr ,0xDC,0x00,0x00,0x10,0,P0,0,MS},
 {{'C','O','M','F',' ',' '},ioneopr ,0xD8,0x00,0x00,0x10,0,P0,0,MS},
 {{'C','O','M','L',' ',' '},ioneopr ,0xDA,0x00,0x00,0x10,0,P0,0,MS},
 {{'C','O','M','P',' ',' '},itwooprf,0xD8,0xD8,0xD8,0x00,0,P0,0,0},
 {{'C','O','M','P','D',' '},ioneopr ,0xDC,0x00,0x00,0x18,0,P0,0,MS},
 {{'C','O','M','P','F',' '},ioneopr ,0xD8,0x00,0x00,0x18,0,P0,0,MS},
 {{'C','O','M','P','L',' '},ioneopr ,0xDA,0x00,0x00,0x18,0,P0,0,MS},
 {{'C','O','M','P','P',' '},itwobyte,0xDE,0xD9,0x00,0x00,0,P0,0,0},
 {{'C','O','M','P','Q',' '},ioneopr ,0xDE,0x00,0x00,0x18,0,P0,0,MS},
 {{'C','O','M','Q',' ',' '},ioneopr ,0xDE,0x00,0x00,0x10,0,P0,0,MS},
 {{'C','O','S',' ',' ',' '},itwobyte,0xD9,0xFF,0x00,0x00,0,P3,0,MS},
 {{'D','E','C','S','T','P'},itwobyte,0xD9,0xF6,0x00,0x00,0,P0,0,0},
 {{'D','I','S','I',' ',' '},ithrbyte,0x9B,0xDB,0xE1,0x00,0,P0,0,0},
 {{'D','I','V',' ',' ',' '},itwooprf,0xD8,0xF8,0xDE,0x00,0,P0,0,DB},
 {{'D','I','V','D',' ',' '},ioneopr ,0xDC,0x00,0x00,0x30,0,P0,0,MS},
 {{'D','I','V','F',' ',' '},ioneopr ,0xD8,0x00,0x00,0x30,0,P0,0,MS},
 {{'D','I','V','L',' ',' '},ioneopr ,0xDA,0x00,0x00,0x30,0,P0,0,MS},
 {{'D','I','V','P',' ',' '},itwooprf,0xDE,0xF8,0xDE,0x00,0,P0,0,0},
 {{'D','I','V','Q',' ',' '},ioneopr ,0xDE,0x00,0x00,0x30,0,P0,0,MS},
 {{'D','I','V','R',' ',' '},itwooprf,0xD8,0xF0,0xDE,0x00,0,P0,0,DB},
 {{'D','I','V','R','D',' '},ioneopr ,0xDC,0x00,0x00,0x38,0,P0,0,MS},
 {{'D','I','V','R','F',' '},ioneopr ,0xD8,0x00,0x00,0x38,0,P0,0,MS},
 {{'D','I','V','R','L',' '},ioneopr ,0xDE,0x00,0x00,0x38,0,P0,0,MS},
 {{'D','I','V','R','Q',' '},ioneopr ,0xDA,0x00,0x00,0x38,0,P0,0,MS},
 {{'D','I','V','R','P',' '},itwooprf,0xDE,0xF0,0xDE,0x00,0,P0,0,0},
 {{'E','N','I',' ',' ',' '},ithrbyte,0x9B,0xDB,0xE0,0x00,0,P0,0,MS},
 {{'F','R','E','E',' ',' '},ioneoprf,0xDD,0xC0,0x00,0x00,0,P0,0,0},
 {{'I','N','C','S','T','P'},itwobyte,0xD9,0xF7,0x00,0x00,0,P0,0,0},
 {{'I','N','I','T',' ',' '},ithrbyte,0x9B,0xDB,0xE3,0x00,0,P0,0,0},
 {{'L','D',' ',' ',' ',' '},ioneoprf,0xD9,0xC0,0x00,0x00,0,P0,0,RS},
 {{'L','D','1',' ',' ',' '},itwobyte,0xD9,0xE8,0x00,0x00,0,P0,0,0},
 {{'L','D','C','W',' ',' '},ioneopr ,0xD9,0x00,0x00,0x28,0,P0,0,MS},
 {{'L','D','D',' ',' ',' '},ioneopr ,0xDD,0x00,0x00,0x00,0,P0,0,MS},
 {{'L','D','E','N','V',' '},ioneopr ,0xD9,0x00,0x00,0x20,0,P0,0,MS},
 {{'L','D','F',' ',' ',' '},ioneopr ,0xD9,0x00,0x00,0x00,0,P0,0,MS},
 {{'L','D','L',' ',' ',' '},ioneopr ,0xDB,0x00,0x00,0x00,0,P0,0,MS},
 {{'L','D','L','2','E',' '},itwobyte,0xD9,0xEA,0x00,0x00,0,P0,0,0},
 {{'L','D','L','2','T',' '},itwobyte,0xD9,0xE9,0x00,0x00,0,P0,0,0},
 {{'L','D','L','G','2',' '},itwobyte,0xD9,0xEC,0x00,0x00,0,P0,0,0},
 {{'L','D','L','N','2',' '},itwobyte,0xD9,0xED,0x00,0x00,0,P0,0,0},
 {{'L','D','P','I',' ',' '},itwobyte,0xD9,0xEB,0x00,0x00,0,P0,0,0},
 {{'L','D','Q',' ',' ',' '},ioneopr ,0xDF,0x00,0x00,0x00,0,P0,0,MS},
 {{'L','D','X',' ',' ',' '},ioneopr ,0xDB,0x00,0x00,0x28,0,P0,0,MS},
 {{'L','D','Z',' ',' ',' '},itwobyte,0xD9,0xEE,0x00,0x00,0,P0,0,0},
 {{'M','U','L',' ',' ',' '},itwooprf,0xD8,0xC8,0xDE,0x00,0,P0,0,DB},
 {{'M','U','L','D',' ',' '},ioneopr ,0xDC,0x00,0x00,0x08,0,P0,0,MS},
 {{'M','U','L','F',' ',' '},ioneopr ,0xD8,0x00,0x00,0x08,0,P0,0,MS},
 {{'M','U','L','L',' ',' '},ioneopr ,0xDA,0x00,0x00,0x08,0,P0,0,MS},
 {{'M','U','L','P',' ',' '},itwooprf,0xDE,0xC8,0xDE,0x00,0,P0,0,0},
 {{'M','U','L','Q',' ',' '},ioneopr ,0xDE,0x00,0x00,0x08,0,P0,0,MS},
 {{'N','C','L','E','X',' '},itwobyte,0xDB,0xE2,0x00,0x00,0,P0,0,MS},
 {{'N','D','I','S','I',' '},ithrbyte,0x9B,0xDB,0xE1,0x00,0,P0,0,MS},
 {{'N','E','N','I',' ',' '},ithrbyte,0x9B,0xDB,0xE0,0x00,0,P0,0,MS},
 {{'N','I','N','I','T',' '},itwobyte,0xDB,0xE3,0x00,0x00,0,P0,0,0},
 {{'N','O','P',' ',' ',' '},itwobyte,0xD9,0xD0,0x00,0x00,0,P0,0,0},
 {{'N','S','A','V','E',' '},ioneopr ,0xDD,0x00,0x00,0x30,0,P0,0,MS},
 {{'N','S','T','C','W',' '},ioneopr ,0xD9,0x00,0x00,0x38,0,P0,0,MS},
 {{'N','S','T','E','N','V'},ioneoprw,0xD9,0x00,0x00,0x30,0,P0,0,MS},
 {{'N','S','T','S','W',' '},ifstsw  ,0x00,0x00,0x00,0x00,0,P0,0,0},
 {{'P','A','T','A','N',' '},itwobyte,0xD9,0xF3,0x00,0x00,0,P0,0,0},
 {{'P','R','E','M',' ',' '},itwobyte,0xD9,0xF8,0x00,0x00,0,P0,0,0},
 {{'P','R','E','M','1',' '},itwobyte,0xD9,0xF5,0x00,0x00,0,P3,0,0},
 {{'P','T','A','N',' ',' '},itwobyte,0xD9,0xF2,0x00,0x00,0,P0,0,0},
 {{'R','I','C','H','O','P'},itwobyte,0xDD,0xFC,0x00,0x00,0,P2,0,0},
 {{'R','I','N','E','A','R'},itwobyte,0xDF,0xFC,0x00,0x00,0,P2,0,0},
 {{'R','I','N','T','2',' '},itwobyte,0xDB,0xFC,0x00,0x00,0,P2,0,0},
 {{'R','N','D','I','N','T'},itwobyte,0xD9,0xFC,0x00,0x00,0,P0,0,0},
 {{'R','S','T','O','R',' '},ioneopr ,0xDD,0x00,0x00,0x20,0,P0,0,MS},
 {{'R','S','T','P','M',' '},itwobyte,0xDB,0xF4,0x00,0x00,0,P2,0,0},
 {{'S','A','V','E',' ',' '},ioneoprw,0xDD,0x00,0x00,0x30,0,P0,0,MS},
 {{'S','B','P','0',' ',' '},itwobyte,0xDB,0xE8,0x00,0x00,0,P2,0,0},
 {{'S','B','P','1',' ',' '},itwobyte,0xDB,0xEB,0x00,0x00,0,P2,0,0},
 {{'S','B','P','2',' ',' '},itwobyte,0xDB,0xEC,0x00,0x00,0,P2,0,0},
 {{'S','C','A','L','E',' '},itwobyte,0xD9,0xFD,0x00,0x00,0,P0,0,0},
 {{'S','E','T','P','M',' '},itwobyte,0xDB,0xE4,0x00,0x00,0,P2,0,0},
 {{'S','I','N',' ',' ',' '},itwobyte,0xD9,0xFE,0x00,0x00,0,P3,0,0},
 {{'S','I','N','C','O','S'},itwobyte,0xDB,0xFB,0x00,0x00,0,P3,0,0},
 {{'S','Q','R','T',' ',' '},itwobyte,0xD9,0xFA,0x00,0x00,0,P0,0,0},
 {{'S','T',' ',' ',' ',' '},ioneoprf,0xDD,0xD0,0x00,0x00,0,P0,0,RS},
 {{'S','T','C','W',' ',' '},ioneoprw,0xD9,0x00,0x00,0x38,0,P0,0,MS},
 {{'S','T','E','N','V',' '},ioneoprw,0xD9,0x00,0x00,0x30,0,P0,0,MS},
 {{'S','T','F',' ',' ',' '},ioneopr ,0xD9,0x00,0x00,0x10,0,P0,0,MS},
 {{'S','T','D',' ',' ',' '},ioneopr ,0xDD,0x00,0x00,0x10,0,P0,0,MS},
 {{'S','T','L',' ',' ',' '},ioneopr ,0xDB,0x00,0x00,0x10,0,P0,0,MS},
 {{'S','T','P',' ',' ',' '},ioneoprf,0xDD,0xC8,0x00,0x00,0,P0,0,RS},
 {{'S','T','P','F',' ',' '},ioneopr ,0xDB,0x00,0x00,0x18,0,P0,0,MS},
 {{'S','T','P','D',' ',' '},ioneopr ,0xDD,0x00,0x00,0x18,0,P0,0,MS},
 {{'S','T','P','L',' ',' '},ioneopr ,0xDF,0x00,0x00,0x18,0,P0,0,MS},
 {{'S','T','P','O',' ',' '},ioneopr ,0xDF,0x00,0x00,0x38,0,P0,0,MS},
 {{'S','T','P','Q',' ',' '},ioneopr ,0xDB,0x00,0x00,0x18,0,P0,0,MS},
 {{'S','T','P','X',' ',' '},ioneopr ,0xDB,0x00,0x00,0x38,0,P0,0,MS},
 {{'S','T','Q',' ',' ',' '},ioneopr ,0xDF,0x00,0x00,0x10,0,P0,0,MS},
 {{'S','T','S','W',' ',' '},ifstsw  ,0x01,0x00,0x00,0x00,0,P0,0,0},
 {{'S','U','B',' ',' ',' '},itwooprf,0xD8,0xE8,0xDE,0x00,0,P0,0,DB},
 {{'S','U','B','D',' ',' '},ioneopr ,0xDC,0x00,0x00,0x20,0,P0,0,MS},
 {{'S','U','B','F',' ',' '},ioneopr ,0xD8,0x00,0x00,0x20,0,P0,0,MS},
 {{'S','U','B','L',' ',' '},ioneopr ,0xDA,0x00,0x00,0x20,0,P0,0,MS},
 {{'S','U','B','P',' ',' '},itwooprf,0xDE,0xE8,0xDE,0x00,0,P0,0,0},
 {{'S','U','B','Q',' ',' '},ioneopr ,0xDE,0x00,0x00,0x20,0,P0,0,MS},
 {{'S','U','B','R',' ',' '},itwooprf,0xD8,0xE0,0xDE,0x00,0,P0,0,DB},
 {{'S','U','B','R','D',' '},ioneopr ,0xDC,0x00,0x00,0x28,0,P0,0,MS},
 {{'S','U','B','R','F',' '},ioneopr ,0xD8,0x00,0x00,0x28,0,P0,0,MS},
 {{'S','U','B','R','L',' '},ioneopr ,0xDA,0x00,0x00,0x28,0,P0,0,MS},
 {{'S','U','B','R','P',' '},itwooprf,0xDE,0xE0,0xDE,0x00,0,P0,0,0},
 {{'S','U','B','R','Q',' '},ioneopr ,0xDE,0x00,0x00,0x08,0,P0,0,MS},
 {{'T','S','T',' ',' ',' '},itwobyte,0xD9,0xE4,0x00,0x00,0,P0,0,0},
 {{'T','S','T','P',' ',' '},itwobyte,0xD9,0xE6,0x00,0x00,0,P3,0,0},
 {{'U','C','O','M',' ',' '},ioneoprf,0xDD,0xE0,0x00,0x00,0,P3,0,0},
 {{'U','C','O','M','P',' '},ioneoprf,0xDD,0xE8,0x00,0x00,0,P3,0,0},
 {{'U','C','O','M','P','P'},itwobyte,0xDA,0xE9,0x00,0x00,0,P3,0,0},
 {{'W','A','I','T',' ',' '},ionebyte,0x9B,0x00,0x00,0x00,0,P0,0,0},
 {{'X','A','M',' ',' ',' '},itwobyte,0xD9,0xE5,0x00,0x00,0,P0,0,0},
 {{'X','C','H',' ',' ',' '},ioneoprf,0xD9,0xC8,0x00,0x00,0,P0,0,0},
 {{'X','T','R','A','C','T'},itwobyte,0xD9,0xF4,0x00,0x00,0,P0,0,0},
 {{'Y','L','2','X',' ',' '},itwobyte,0xD9,0xF1,0x00,0x00,0,P0,0,0},
 {{'Y','L','2','X','P','1'},itwobyte,0xD9,0xF9,0x00,0x00,0,P0,0,0}
};

struct optbl opctblh[] =
{{{'L','T',' ',' ',' ',' '},ionebyte,0xF4,0x00,0x00,0x00,0,P0,0,0}
};

struct optbl opctbli[] =
{{{'D','I','V','B',' ',' '},ioneopr ,0xF6,0x00,0x00,0x38,1,P0,0,MS|RS},
 {{'D','I','V','L',' ',' '},ioneopr ,0xF7,0x00,0x00,0x38,4,P3,0,MS|RS},
 {{'D','I','V','W',' ',' '},ioneopr ,0xF7,0x00,0x00,0x38,2,P0,0,MS|RS},
 {{'M','U','L','B',' ',' '},ioneopr ,0xF6,0x00,0x00,0x28,1,P0,0,RS|MS},
 {{'M','U','L','L',' ',' '},iimul   ,0xF7,0x00,0x00,0x00,4,P3,0,0},
 {{'M','U','L','W',' ',' '},iimul   ,0xF7,0x00,0x00,0x00,2,P0,0,0},
 {{'N','B',' ',' ',' ',' '},iinout  ,0xE4,0xEC,0x00,0x00,1,P0,0,0},
 {{'N','C','B',' ',' ',' '},iincdec ,0xFE,0x00,0x00,0x00,1,P0,0,0},
 {{'N','C','L',' ',' ',' '},iincdec ,0xFF,0x40,0x00,0x00,4,P3,0,0},
 {{'N','C','W',' ',' ',' '},iincdec ,0xFF,0x40,0x00,0x00,2,P0,0,0},
 {{'N','L',' ',' ',' ',' '},iinout  ,0xE5,0xED,0x00,0x00,4,P3,0,0},
 {{'N','S','B',' ',' ',' '},istrins ,0x6C,0x00,0x00,0x00,1,P0,0,MD},
 {{'N','S','L',' ',' ',' '},istrins ,0x6D,0x00,0x00,0x00,4,P3,0,MD},
 {{'N','S','W',' ',' ',' '},istrins ,0x6D,0x00,0x00,0x00,2,P0,0,MD},
 {{'N','T',' ',' ',' ',' '},iintins ,0xCD,0x00,0x00,0x00,0,P0,0,0},
 {{'N','T','3',' ',' ',' '},ionebyte,0xCC,0x00,0x00,0x00,0,P0,0,0},
 {{'N','T','O',' ',' ',' '},ionebyte,0xCE,0x00,0x00,0x00,0,P0,0,0},
 {{'N','V','D',' ',' ',' '},itwobyte,0x0F,0x08,0x00,0x00,0,P4,0,0},
 {{'N','V','L','P','G',' '},ioneopr ,0x01,0x00,0x00,0x38,0,P4,0,MS|XP},
 {{'N','W',' ',' ',' ',' '},iinout  ,0xE5,0xED,0x00,0x00,2,P0,0,0},
 {{'R','E','T',' ',' ',' '},ionebytx,0xCF,0x00,0x00,0x00,0,P0,0,0},
 {{'R','E','T','L',' ',' '},ionebytx,0xCF,0x00,0x00,0x00,4,P0,0,0},
 {{'R','E','T','W',' ',' '},ionebytx,0xCF,0x00,0x00,0x00,2,P0,0,0}
};

struct optbl opctblj[] =
{{{'A',' ',' ',' ',' ',' '},ibranch ,0x77,0x00,0x0F,0x87,0,P0,0,B3|BO},
 {{'A','E',' ',' ',' ',' '},ibranch ,0x73,0x00,0x0F,0x83,0,P0,0,B3|BO},
 {{'B',' ',' ',' ',' ',' '},ibranch ,0x72,0x00,0x0F,0x82,0,P0,0,B3|BO},
 {{'B','E',' ',' ',' ',' '},ibranch ,0x76,0x00,0x0F,0x86,0,P0,0,B3|BO},
 {{'C',' ',' ',' ',' ',' '},ibranch ,0x72,0x00,0x0F,0x82,0,P0,0,B3|BO},
 {{'E',' ',' ',' ',' ',' '},ibranch ,0x74,0x00,0x0F,0x84,0,P0,0,B3|BO},
 {{'G',' ',' ',' ',' ',' '},ibranch ,0x7F,0x00,0x0F,0x8F,0,P0,0,B3|BO},
 {{'G','E',' ',' ',' ',' '},ibranch ,0x7D,0x00,0x0F,0x8D,0,P0,0,B3|BO},
 {{'L',' ',' ',' ',' ',' '},ibranch ,0x7C,0x00,0x0F,0x8C,0,P0,0,B3|BO},
 {{'L','E',' ',' ',' ',' '},ibranch ,0x7E,0x00,0x0F,0x8E,0,P0,0,B3|BO},
 {{'M','P',' ',' ',' ',' '},ibranch ,0xEB,0x00,0xE9,0x00,0,P0,0,BO},
 {{'M','P','F',' ',' ',' '},ijumpf  ,0x00,0x00,0x00,0x00,0,P0,0,0},
 {{'M','P','F','I','L',' '},ioneopr ,0xFF,0x00,0x00,0x28,4,P0,0,MS},
 {{'M','P','F','I','W',' '},ioneopr ,0xFF,0x00,0x00,0x28,2,P0,0,MS},
 {{'M','P','I','L',' ',' '},ioneopr ,0xFF,0x00,0x00,0x20,4,P0,0,MS|RS},
 {{'M','P','I','W',' ',' '},ioneopr ,0xFF,0x00,0x00,0x20,2,P0,0,MS|RS},
 {{'N','C',' ',' ',' ',' '},ibranch ,0x73,0x00,0x0F,0x83,0,P0,0,B3|BO},
 {{'N','E',' ',' ',' ',' '},ibranch ,0x75,0x00,0x0F,0x85,0,P0,0,B3|BO},
 {{'N','O',' ',' ',' ',' '},ibranch ,0x71,0x00,0x0F,0x81,0,P0,0,B3|BO},
 {{'N','P',' ',' ',' ',' '},ibranch ,0x7B,0x00,0x0F,0x8B,0,P0,0,B3|BO},
 {{'N','S',' ',' ',' ',' '},ibranch ,0x79,0x00,0x0F,0x89,0,P0,0,B3|BO},
 {{'N','Z',' ',' ',' ',' '},ibranch ,0x75,0x00,0x0F,0x85,0,P0,0,B3|BO},
 {{'O',' ',' ',' ',' ',' '},ibranch ,0x70,0x00,0x0F,0x80,0,P0,0,B3|BO},
 {{'P',' ',' ',' ',' ',' '},ibranch ,0x7A,0x00,0x0F,0x8A,0,P0,0,B3|BO},
 {{'P','E',' ',' ',' ',' '},ibranch ,0x7A,0x00,0x0F,0x8A,0,P0,0,B3|BO},
 {{'P','O',' ',' ',' ',' '},ibranch ,0x7B,0x00,0x0F,0x8B,0,P0,0,B3|BO},
 {{'R','E','G','Z',' ',' '},iloop   ,0xE3,0x00,0x00,0x00,0,P0,0,0},
 {{'S',' ',' ',' ',' ',' '},ibranch ,0x78,0x00,0x0F,0x88,0,P0,0,B3|BO},
 {{'Z',' ',' ',' ',' ',' '},ibranch ,0x74,0x00,0x0F,0x84,0,P0,0,B3|BO}
};

struct optbl opctbll[] =
{{{'A','H','F',' ',' ',' '},ionebyte,0x9F,0x00,0x00,0x00,0,P0,0,0},
 {{'A','R','L',' ',' ',' '},itwoopr ,0x02,0x00,0x00,0x00,4,P3,0,RD|RS|MS|XP|RF},
 {{'A','R','W',' ',' ',' '},itwoopr ,0x02,0x00,0x00,0x00,2,P0,0,RD|RS|MS|XP|RF},
 {{'D','S','L',' ',' ',' '},itwoopr ,0xC5,0x00,0x00,0x00,4,P3,0,RD|MS},
 {{'D','S','W',' ',' ',' '},itwoopr ,0xC5,0x00,0x00,0x00,2,P0,0,RD|MS},
 {{'E','A','L',' ',' ',' '},itwoopr ,0x8D,0x00,0x00,0x00,4,P3,0,RD|MS},
 {{'E','A','W',' ',' ',' '},itwoopr ,0x8D,0x00,0x00,0x00,2,P0,0,RD|MS},
 {{'E','A','V','E',' ',' '},ionebytx,0xC9,0x00,0x00,0x00,0,P1,0,0},
 {{'E','S','L',' ',' ',' '},itwoopr ,0xC4,0x00,0x00,0x00,4,P3,0,RD|MS},
 {{'E','S','W',' ',' ',' '},itwoopr ,0xC4,0x00,0x00,0x00,2,P0,0,RD|MS},
 {{'F','S','L',' ',' ',' '},itwoopr ,0xB4,0x00,0x00,0x00,4,P3,0,RD|MS|XP},
 {{'F','S','W',' ',' ',' '},itwoopr ,0xB4,0x00,0x00,0x00,2,P3,0,RD|MS|XP},
 {{'G','D','T','L',' ',' '},ioneopr ,0x01,0x00,0x00,0x10,4,P3,0,RD|MS|XP},
 {{'G','D','T','W',' ',' '},ioneopr ,0x01,0x00,0x00,0x10,2,P2,0,RD|MS|XP},
 {{'G','S','L',' ',' ',' '},itwoopr ,0xB5,0x00,0x00,0x00,4,P3,0,RD|MS|XP},
 {{'G','S','W',' ',' ',' '},itwoopr ,0xB5,0x00,0x00,0x00,2,P3,0,RD|MS|XP},
 {{'I','D','T','L',' ',' '},ioneopr ,0x01,0x00,0x00,0x18,4,P3,0,MS|XP},
 {{'I','D','T','W',' ',' '},ioneopr ,0x01,0x00,0x00,0x18,2,P2,0,MS|XP},
 {{'L','D','T',' ',' ',' '},ioneopr ,0x00,0x00,0x00,0x10,2,P0,0,MS|RS|XP|NS},
 {{'M','S','W',' ',' ',' '},ioneopr ,0x01,0x00,0x00,0x30,2,P0,0,MS|RS|XP|NS},
 {{'O','C','K',' ',' ',' '},ionebyte,0xF0,0x00,0x00,0x00,0,P0,0,0},
 {{'O','D','S','B',' ',' '},istrins ,0xAC,0x00,0x00,0x00,1,P0,0,MS},
 {{'O','D','S','L',' ',' '},istrins ,0xAD,0x00,0x00,0x00,4,P3,0,MS},
 {{'O','D','S','W',' ',' '},istrins ,0xAD,0x00,0x00,0x00,2,P0,0,MS},
 {{'O','O','P',' ',' ',' '},iloop   ,0xE2,0x00,0x00,0x00,0,P0,0,0},
 {{'O','O','P','E',' ',' '},iloop   ,0xE1,0x00,0x00,0x00,0,P0,0,0},
 {{'O','O','P','N','E',' '},iloop   ,0xE0,0x00,0x00,0x00,0,P0,0,0},
 {{'O','O','P','N','Z',' '},iloop   ,0xE0,0x00,0x00,0x00,0,P0,0,0},
 {{'O','O','P','Z',' ',' '},iloop   ,0xE1,0x00,0x00,0x00,0,P0,0,0},
 {{'S','L','L',' ',' ',' '},itwoopr ,0x03,0x00,0x00,0x00,4,P3,0,RD|RS|MS|XP|RF},
 {{'S','L','W',' ',' ',' '},itwoopr ,0x03,0x00,0x00,0x00,2,P0,0,RD|RS|MS|XP|RF},
 {{'S','S','L',' ',' ',' '},itwoopr ,0xB2,0x00,0x00,0x00,4,P3,0,RD|MS|XP},
 {{'S','S','W',' ',' ',' '},itwoopr ,0xB2,0x00,0x00,0x00,2,P3,0,RD|MS|XP},
 {{'T','R',' ',' ',' ',' '},ioneopr ,0x00,0x00,0x00,0x18,2,P0,0,RS|MS|XP|NS}
};

struct optbl opctblm[] =
{{{'O','V','B',' ',' ',' '},imovins ,0x88,0xC6,0xB0,0xA2,1,P0,0,DB},
 {{'O','V','L',' ',' ',' '},imovins ,0x89,0xC7,0xB8,0xA3,4,P3,0,DB},
 {{'O','V','S',' ',' ',' '},istrins ,0xA5,0x00,0x00,0x00,2,P0,0,MD|MS},
 {{'O','V','S','B',' ',' '},istrins ,0xA4,0x00,0x00,0x00,1,P0,0,MD|MS},
 {{'O','V','S','L',' ',' '},istrins ,0xA5,0x00,0x00,0x00,4,P3,0,MD|MS},
 {{'O','V','S','W',' ',' '},istrins ,0xA5,0x00,0x00,0x00,2,P2,0,MD|MS},
 {{'O','V','W',' ',' ',' '},imovins ,0x89,0xC7,0xB8,0xA3,2,P0,0,DB},
 {{'O','V','X','B','L',' '},itwoopr ,0xBE,0x00,0x00,0x00,4,P3,0,RD|RS|MS|XP|BS|RF},
 {{'O','V','X','B','W',' '},itwoopr ,0xBE,0x00,0x00,0x00,2,P3,0,RD|RS|MS|XP|BS|RF},
 {{'O','V','X','W','L',' '},itwoopr ,0xBF,0x00,0x00,0x00,4,P3,0,RD|RS|MS|XP|WS|RF},
 {{'O','V','X','W','W',' '},imovins ,0x89,0xC7,0xB8,0xA3,2,P3,0,DB},
 {{'O','V','Z','B','L',' '},itwoopr ,0xB6,0x00,0x00,0x00,4,P3,0,RD|RS|MS|XP|BS|RF},
 {{'O','V','Z','B','W',' '},itwoopr ,0xB6,0x00,0x00,0x00,2,P3,0,RD|RS|MS|XP|BS|RF},
 {{'O','V','Z','W','L',' '},itwoopr ,0xB7,0x00,0x00,0x00,4,P3,0,RD|RS|MS|XP|WS|RF},
 {{'O','V','Z','W','W',' '},imovins ,0x89,0xC7,0xB8,0xA3,2,P3,0,DB},
 {{'U','L','B',' ',' ',' '},ioneopr ,0xF6,0x00,0x00,0x20,1,P0,0,MS|RS},
 {{'U','L','L',' ',' ',' '},ioneopr ,0xF7,0x00,0x00,0x20,4,P3,0,MS|RS},
 {{'U','L','W',' ',' ',' '},ioneopr ,0xF7,0x00,0x00,0x20,2,P0,0,MS|RS}
};

struct optbl opctbln[] =
{{{'E','G','B',' ',' ',' '},ioneopr ,0xF6,0x00,0x00,0x18,1,P0,0,MS|RS},
 {{'E','G','L',' ',' ',' '},ioneopr ,0xF7,0x00,0x00,0x18,4,P3,0,MS|RS},
 {{'E','G','W',' ',' ',' '},ioneopr ,0xF7,0x00,0x00,0x18,2,P0,0,MS|RS},
 {{'O','P',' ',' ',' ',' '},ionebyte,0x90,0x00,0x00,0x00,0,P0,0,0},
 {{'O','T','B',' ',' ',' '},ioneopr ,0xF6,0x00,0x00,0x10,1,P0,0,MS|RS},
 {{'O','T','L',' ',' ',' '},ioneopr ,0xF7,0x00,0x00,0x10,4,P3,0,MS|RS},
 {{'O','T','W',' ',' ',' '},ioneopr ,0xF7,0x00,0x00,0x10,2,P0,0,MS|RS}
};

struct optbl opctblo[] =
{{{'R','B',' ',' ',' ',' '},itwoopr ,0x08,0x80,0x08,0x0C,1,P0,0,RD|MD|MS|RS|DB|SB},
 {{'R','L',' ',' ',' ',' '},itwoopr ,0x09,0x81,0x08,0x0D,4,P3,0,RD|MD|MS|RS|DB|SB},
 {{'R','W',' ',' ',' ',' '},itwoopr ,0x09,0x81,0x08,0x0D,2,P0,0,RD|MD|MS|RS|DB|SB},
 {{'U','T','B',' ',' ',' '},iinout  ,0xE6,0xEE,0x00,0x00,1,P0,0,0},
 {{'U','T','L',' ',' ',' '},iinout  ,0xE7,0xEF,0x00,0x00,4,P3,0,0},
 {{'U','T','S','B',' ',' '},istrins ,0x6E,0x00,0x00,0x00,1,P0,0,MS},
 {{'U','T','S','L',' ',' '},istrins ,0x6F,0x00,0x00,0x00,4,P3,0,MS},
 {{'U','T','S','W',' ',' '},istrins ,0x6F,0x00,0x00,0x00,2,P0,0,MS},
 {{'U','T','W',' ',' ',' '},iinout  ,0xE7,0xEF,0x00,0x00,2,P0,0,0}
};

struct optbl opctblp[] =
{{{'O','P','A','L',' ',' '},ionebytx,0x61,0x00,0x00,0x00,4,P3,0,0},
 {{'O','P','A','W',' ',' '},ionebytx,0x61,0x00,0x00,0x00,2,P0,0,0},
 {{'O','P','F',' ',' ',' '},ionebytx,0x9D,0x00,0x00,0x00,0,P0,0,0},
 {{'O','P','F','L',' ',' '},ionebytx,0x9D,0x00,0x00,0x00,4,P3,0,0},
 {{'O','P','F','W',' ',' '},ionebytx,0x9D,0x00,0x00,0x00,2,P0,0,0},
 {{'O','P','L',' ',' ',' '},ipushpop,0x8F,0x58,0x07,0x00,4,P3,0,0},
 {{'O','P','W',' ',' ',' '},ipushpop,0x8F,0x58,0x07,0x00,2,P0,0,0},
 {{'U','S','H','A','L',' '},ionebytx,0x60,0x00,0x00,0x00,4,P3,0,0},
 {{'U','S','H','A','W',' '},ionebytx,0x60,0x00,0x00,0x00,2,P0,0,0},
 {{'U','S','H','F',' ',' '},ionebytx,0x9C,0x00,0x00,0x00,0,P0,0,0},
 {{'U','S','H','F','L',' '},ionebytx,0x9C,0x00,0x00,0x00,4,P3,0,0},
 {{'U','S','H','F','W',' '},ionebytx,0x9C,0x00,0x00,0x00,2,P0,0,0},
 {{'U','S','H','L',' ',' '},ipushpop,0xFF,0x50,0x06,0x30,4,P3,0,0},
 {{'U','S','H','W',' ',' '},ipushpop,0xFF,0x50,0x06,0x30,2,P0,0,0}
};

struct optbl opctblr[] =
{{{'C','L','B',' ',' ',' '},ishrot  ,0xD0,0xD2,0xC0,0x10,1,P0,0,BI},
 {{'C','L','L',' ',' ',' '},ishrot  ,0xD1,0xD3,0xC1,0x10,4,P3,0,BI},
 {{'C','L','W',' ',' ',' '},ishrot  ,0xD1,0xD3,0xC1,0x10,2,P0,0,BI},
 {{'C','R','B',' ',' ',' '},ishrot  ,0xD0,0xD2,0xC0,0x18,1,P0,0,BI},
 {{'C','R','L',' ',' ',' '},ishrot  ,0xD1,0xD3,0xC1,0x18,4,P3,0,BI},
 {{'C','R','W',' ',' ',' '},ishrot  ,0xD1,0xD3,0xC1,0x18,2,P0,0,BI},
 {{'E','C','M','P','S','B'},istrins ,0xA6,0xF3,0x00,0x00,1,P0,0,MD|MS},
 {{'E','C','M','P','S','L'},istrins ,0xA7,0xF3,0x00,0x00,4,P3,0,MD|MS},
 {{'E','C','M','P','S','W'},istrins ,0xA7,0xF3,0x00,0x00,2,P0,0,MD|MS},
 {{'E','S','C','A','S','B'},istrins ,0xAE,0xF3,0x00,0x00,1,P0,0,MD},
 {{'E','S','C','A','S','L'},istrins ,0xAF,0xF3,0x00,0x00,4,P3,0,MD},
 {{'E','S','C','A','S','W'},istrins ,0xAF,0xF3,0x00,0x00,2,P0,0,MD},
 {{'E','T',' ',' ',' ',' '},iret    ,0xC3,0xC2,0x00,0x00,0,P0,0,0},
 {{'E','T','F',' ',' ',' '},iret    ,0xCB,0xCA,0x00,0x00,0,P0,0,0},
 {{'E','T','F','L',' ',' '},iret    ,0xCB,0xCA,0x00,0x00,4,P0,0,0},
 {{'E','T','F','W',' ',' '},iret    ,0xCB,0xCA,0x00,0x00,2,P0,0,0},
 {{'E','T','L',' ',' ',' '},iret    ,0xC3,0xC2,0x00,0x00,4,P0,0,0},
 {{'E','T','W',' ',' ',' '},iret    ,0xC3,0xC2,0x00,0x00,2,P0,0,0},
 {{'I','N','S','W',' ',' '},istrins ,0x6D,0xF3,0x00,0x00,2,P0,0,MD},
 {{'I','N','S','B',' ',' '},istrins ,0x6C,0xF3,0x00,0x00,1,P0,0,MD},
 {{'I','N','S','L',' ',' '},istrins ,0x6D,0xF3,0x00,0x00,4,P3,0,MD},
 {{'L','O','D','S','B',' '},istrins ,0xAC,0xF3,0x00,0x00,1,P0,0,MS},
 {{'L','O','D','S','L',' '},istrins ,0xAD,0xF3,0x00,0x00,4,P3,0,MS},
 {{'L','O','D','S','W',' '},istrins ,0xAD,0xF3,0x00,0x00,2,P0,0,MS},
 {{'M','O','V','S','B',' '},istrins ,0xA4,0xF3,0x00,0x00,1,P0,0,MD|MS},
 {{'M','O','V','S','L',' '},istrins ,0xA5,0xF3,0x00,0x00,4,P3,0,MD|MS},
 {{'M','O','V','S','W',' '},istrins ,0xA5,0xF3,0x00,0x00,2,P0,0,MD|MS},
 {{'N','C','M','P','S','B'},istrins ,0xA6,0xF2,0x00,0x00,1,P0,0,MD|MS},
 {{'N','C','M','P','S','L'},istrins ,0xA7,0xF2,0x00,0x00,4,P3,0,MD|MS},
 {{'N','C','M','P','S','W'},istrins ,0xA7,0xF2,0x00,0x00,2,P0,0,MD|MS},
 {{'N','S','C','A','S','B'},istrins ,0xAE,0xF2,0x00,0x00,1,P0,0,MD},
 {{'N','S','C','A','S','L'},istrins ,0xAF,0xF2,0x00,0x00,4,P3,0,MD},
 {{'N','S','C','A','S','W'},istrins ,0xAF,0xF2,0x00,0x00,2,P0,0,MD},
 {{'O','L','B',' ',' ',' '},ishrot  ,0xD0,0xD2,0xC0,0x00,1,P0,0,BI},
 {{'O','L','L',' ',' ',' '},ishrot  ,0xD1,0xD3,0xC1,0x00,4,P3,0,BI},
 {{'O','L','W',' ',' ',' '},ishrot  ,0xD1,0xD3,0xC1,0x00,2,P0,0,BI},
 {{'O','R','B',' ',' ',' '},ishrot  ,0xD0,0xD2,0xC0,0x08,1,P0,0,BI},
 {{'O','R','L',' ',' ',' '},ishrot  ,0xD1,0xD3,0xC1,0x08,4,P3,0,BI},
 {{'O','R','W',' ',' ',' '},ishrot  ,0xD1,0xD3,0xC1,0x08,2,P0,0,BI},
 {{'O','U','T','S','B',' '},istrins ,0x6E,0xF3,0x00,0x00,1,P0,0,MS},
 {{'O','U','T','S','L',' '},istrins ,0x6F,0xF3,0x00,0x00,4,P3,0,MS},
 {{'O','U','T','S','W',' '},istrins ,0x6F,0xF3,0x00,0x00,2,P0,0,MS},
 {{'S','T','O','S','B',' '},istrins ,0xAA,0xF3,0x00,0x00,1,P0,0,MD},
 {{'S','T','O','S','L',' '},istrins ,0xAB,0xF3,0x00,0x00,4,P3,0,MD},
 {{'S','T','O','S','W',' '},istrins ,0xAB,0xF3,0x00,0x00,2,P0,0,MD}
};

struct optbl opctbls[] =
{{{'A','H','F',' ',' ',' '},ionebyte,0x9E,0x00,0x00,0x00,0,P0,0,0},
 {{'A','L','B',' ',' ',' '},ishrot  ,0xD0,0xD2,0xC0,0x20,1,P0,0,BI},
 {{'A','L','L',' ',' ',' '},ishrot  ,0xD1,0xD3,0xC1,0x20,4,P3,0,BI},
 {{'A','L','W',' ',' ',' '},ishrot  ,0xD1,0xD3,0xC1,0x20,2,P0,0,BI},
 {{'A','R','B',' ',' ',' '},ishrot  ,0xD0,0xD2,0xC0,0x38,1,P0,0,BI},
 {{'A','R','L',' ',' ',' '},ishrot  ,0xD1,0xD3,0xC1,0x38,4,P3,0,BI},
 {{'A','R','W',' ',' ',' '},ishrot  ,0xD1,0xD3,0xC1,0x38,2,P0,0,BI},
 {{'B','B','B',' ',' ',' '},itwoopr ,0x18,0x80,0x18,0x80,1,P0,0,RD|MD|MS|RS|DB|SB},
 {{'B','B','L',' ',' ',' '},itwoopr ,0x19,0x81,0x18,0x81,4,P3,0,RD|MD|MS|RS|DB|SB},
 {{'B','B','W',' ',' ',' '},itwoopr ,0x19,0x81,0x18,0x81,2,P0,0,RD|MD|MS|RS|DB|SB},
 {{'C','A','S','B',' ',' '},istrins ,0xAE,0x00,0x00,0x00,1,P0,0,MD},
 {{'C','A','S','L',' ',' '},istrins ,0xAF,0x00,0x00,0x00,4,P3,0,MD},
 {{'C','A','S','W',' ',' '},istrins ,0xAF,0x00,0x00,0x00,2,P0,0,MD},
 {{'E','T','A',' ',' ',' '},ioneopr ,0x97,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','A','E',' ',' '},ioneopr ,0x93,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','B',' ',' ',' '},ioneopr ,0x92,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','B','E',' ',' '},ioneopr ,0x96,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','C',' ',' ',' '},ioneopr ,0x92,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','E',' ',' ',' '},ioneopr ,0x94,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','G',' ',' ',' '},ioneopr ,0x9F,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','G','E',' ',' '},ioneopr ,0x9D,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','L',' ',' ',' '},ioneopr ,0x9C,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','L','E',' ',' '},ioneopr ,0x9E,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','N','C',' ',' '},ioneopr ,0x93,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','N','E',' ',' '},ioneopr ,0x95,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','N','O',' ',' '},ioneopr ,0x91,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','N','P',' ',' '},ioneopr ,0x9B,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','N','S',' ',' '},ioneopr ,0x99,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','N','Z',' ',' '},ioneopr ,0x95,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','O',' ',' ',' '},ioneopr ,0x90,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','P',' ',' ',' '},ioneopr ,0x9A,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','P','E',' ',' '},ioneopr ,0x9A,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','P','O',' ',' '},ioneopr ,0x9B,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','S',' ',' ',' '},ioneopr ,0x98,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'E','T','Z',' ',' ',' '},ioneopr ,0x94,0x00,0x00,0x00,1,P3,0,MS|RS|XP},
 {{'G','D','T',' ',' ',' '},ioneopr ,0x01,0x00,0x00,0x00,0,P0,0,MS|XP},
 {{'H','L','B',' ',' ',' '},ishrot  ,0xD0,0xD2,0xC0,0x20,1,P0,0,BI},
 {{'H','L','D','L',' ',' '},idblshf ,0xA5,0xA4,0x00,0x00,4,P3,0,BI|XP},
 {{'H','L','D','W',' ',' '},idblshf ,0xA5,0xA4,0x00,0x00,2,P3,0,BI|XP},
 {{'H','L','L',' ',' ',' '},ishrot  ,0xD1,0xD3,0xC1,0x20,4,P3,0,BI},
 {{'H','L','W',' ',' ',' '},ishrot  ,0xD1,0xD3,0xC1,0x20,2,P0,0,BI},
 {{'H','R','B',' ',' ',' '},ishrot  ,0xD0,0xD2,0xC0,0x28,1,P0,0,BI},
 {{'H','R','D','L',' ',' '},idblshf ,0xAD,0xAC,0x00,0x00,4,P3,0,BI|XP},
 {{'H','R','D','W',' ',' '},idblshf ,0xAD,0xAC,0x00,0x00,2,P3,0,BI|XP},
 {{'H','R','L',' ',' ',' '},ishrot  ,0xD1,0xD3,0xC1,0x28,4,P3,0,BI},
 {{'H','R','W',' ',' ',' '},ishrot  ,0xD1,0xD3,0xC1,0x28,2,P0,0,BI},
 {{'I','D','T',' ',' ',' '},ioneopr ,0x01,0x00,0x00,0x08,0,P0,0,MS|XP},
 {{'L','D','T',' ',' ',' '},ioneopr ,0x00,0x00,0x00,0x00,2,P0,0,MS|RS|XP|NS},
 {{'M','S','W',' ',' ',' '},ioneopr ,0x01,0x00,0x00,0x20,2,P0,0,MS|RS|XP|NS},
 {{'T','C',' ',' ',' ',' '},ionebyte,0xF9,0x00,0x00,0x00,0,P0,0,0},
 {{'T','D',' ',' ',' ',' '},ionebyte,0xFD,0x00,0x00,0x00,0,P0,0,0},
 {{'T','I',' ',' ',' ',' '},ionebyte,0xFB,0x00,0x00,0x00,0,P0,0,0},
 {{'T','O','S','B',' ',' '},istrins ,0xAA,0x00,0x00,0x00,1,P0,0,MD},
 {{'T','O','S','L',' ',' '},istrins ,0xAB,0x00,0x00,0x00,4,P3,0,MD},
 {{'T','O','S','W',' ',' '},istrins ,0xAB,0x00,0x00,0x00,2,P0,0,MD},
 {{'T','R',' ',' ',' ',' '},ioneopr, 0x00,0x00,0x00,0x08,2,P0,0,MS|RS|XP|NS},
 {{'U','B','B',' ',' ',' '},itwoopr ,0x28,0x80,0x28,0x2C,1,P0,0,RD|MD|MS|RS|DB|SB},
 {{'U','B','L',' ',' ',' '},itwoopr ,0x29,0x81,0x28,0x2D,4,P3,0,RD|MD|MS|RS|DB|SB},
 {{'U','B','W',' ',' ',' '},itwoopr ,0x29,0x81,0x28,0x2D,2,P0,0,RD|MD|MS|RS|DB|SB}
};

struct optbl opctblt[] =
{{{'E','S','T','B',' ',' '},itwoopr ,0x84,0xF6,0x00,0xA8,1,P0,0,RD|MD|MS|RS},
 {{'E','S','T','L',' ',' '},itwoopr ,0x85,0xF7,0x00,0xA9,4,P3,0,RD|MD|MS|RS},
 {{'E','S','T','W',' ',' '},itwoopr ,0x85,0xF7,0x00,0xA9,2,P0,0,RD|MD|MS|RS}
};

struct optbl opctblv[] =
{{{'E','R','R',' ',' ',' '},ioneopr ,0x00,0x00,0x00,0x20,2,P0,0,MS|RS|XP|NS},
 {{'E','R','W',' ',' ',' '},ioneopr ,0x00,0x00,0x00,0x28,2,P0,0,MS|RS|XP|NS}
};

struct optbl opctblw[] =
{{{'A','I','T',' ',' ',' '},ionebyte,0x9B,0x00,0x00,0x00,0,P0,0,0},
 {{'B','I','N','V','D',' '},itwobyte,0x0F,0x09,0x00,0x00,0,P4,0,0}
};

struct optbl opctblx[] =
{{{'C','H','G','B',' ',' '},ixchg   ,0x86,0x00,0x00,0x00,1,P0,0,0},
 {{'C','H','G','L',' ',' '},ixchg   ,0x87,0x90,0x00,0x00,4,P3,0,0},
 {{'C','H','G','W',' ',' '},ixchg   ,0x87,0x90,0x00,0x00,2,P0,0,0},
 {{'A','D','D','B',' ',' '},itwoopr ,0xC0,0x00,0x00,0x00,1,P4,0,RD|RF|MD|RS|XP},
 {{'A','D','D','W',' ',' '},itwoopr ,0xC1,0x00,0x00,0x00,2,P4,0,RD|RF|MD|RS|XP},
 {{'A','D','D','L',' ',' '},itwoopr ,0xC1,0x00,0x00,0x00,4,P4,0,RD|RF|MD|RS|XP},
 {{'L','A','T',' ',' ',' '},ionebyte,0xD7,0x00,0x00,0x00,0,P0,0,0},
 {{'O','R','B',' ',' ',' '},itwoopr ,0x30,0x80,0x30,0x34,1,P0,0,RD|MD|MS|RS|DB|SB},
 {{'O','R','L',' ',' ',' '},itwoopr ,0x31,0x81,0x30,0x35,4,P3,0,RD|MD|MS|RS|DB|SB},
 {{'O','R','W',' ',' ',' '},itwoopr ,0x31,0x81,0x30,0x35,2,P0,0,RD|MD|MS|RS|DB|SB}
};

struct optbl *opcpnt[] =
{   opctbla, opctblb, opctblc, opctbld, opctble, opctblf, NULL,
    opctblh, opctbli, opctblj, NULL   , opctbll, opctblm, opctbln,
    opctblo, opctblp, NULL   , opctblr, opctbls, opctblt, NULL,
    opctblv, opctblw, opctblx, NULL   , NULL
};

#define opsize(tbl) sizeof(tbl)/sizeof(struct optbl)

int opcsize[] =
{   opsize(opctbla), opsize(opctblb), opsize(opctblc), opsize(opctbld),
    opsize(opctble), opsize(opctblf), 0              , opsize(opctblh),
    opsize(opctbli), opsize(opctblj), 0              , opsize(opctbll),
    opsize(opctblm), opsize(opctbln), opsize(opctblo), opsize(opctblp),
    0              , opsize(opctblr), opsize(opctbls), opsize(opctblt),
    0              , opsize(opctblv), opsize(opctblw), opsize(opctblx),
    0              , 0
};
