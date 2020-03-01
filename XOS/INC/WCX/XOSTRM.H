//------------------------------------------------------------------------------
//
//  XOSTRM.H - User-mode terminal definitions
//
//  Edit History:
//  -------------
//  09/07/92(brn) - Add comment Header
//   6Jun95 (fpj) - Resynchronized with XOS.PAR.
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

#ifndef _XOSTRM_H_
#define _XOSTRM_H_

# include <_prolog.h>

// Define bits for the function bits for the svcTrmDspMode system call

#define DM_NOCLEAR  0x00008000	// Do not clear screen when changing mode

#define DM_NOFONT   0x00004000	// Do not initialize text mode character
				//   font memory when changing mode to text mode
#define DM_NOADJUST 0x00002000	// Do not force text character height to match
				//   standard font size (8, 14, or 16 pixels)
#define DM_USEDATA  0x00000800	// Use data block values to set parameters

#define DM_RTNDATA  0x00000400	// Return current parameter values in data block

#define DM_NOCHANGE 0x00000200	// Do not change display state, but recalulate
				//   and return parameters
#define DM_BIOSMODE 0x00000100  // Do BIOS style mode set (low order 7 bits
				//   contains BIOS mode value, bit 7 suppresses

// Define mode values (low order 8 bits of the display bits argumemt)

#define DM_TEXT    1		// Text (default is 80x24)
#define DM_HCG     2		// High color (RGB: 5-6-5) 16-bit graphics
#define DM_TCG     3		// True color (XRGB: 8-8-8-8) 32-bit graphics
#define DM_MONOG   4		// Monochrome graphics
#define DM_MIXED   5		// Combined text and monochrome graphics
#define DM_SETBASE 0xFE		// Set base mode values from current mode
#define DM_USEBASE 0xFF		// Change display mode to match base mode if
				//   current mode is different

// Define offsets in the display modes data block

typedef struct trmmodes
{   char dm_textres;		// (1) Text resolution
    char dm_actpage;		// (1) Active BIOS display page
    char dm_prifont;		// (1) Primary font
    char dm_secfont;		// (1) Secondary font
    long dm_columns;		// (4) Number of character columns
    long dm_rows;		// (4) Number of character rows
    long dm_horiz;		// (4) Graphics horizontal resolution
    long dm_vert;		// (4) Graphics vertical resolution
    long dm_disptype;		// (4) Display type
    long dm_curtype;		// (4) Cursor mode
} TRMMODES;

#define	dm_SIZE	 32		// Size of block (allow room for expansion!)

// Define values for the dm_textres field

#define DM_TEXT8   1		// 8 pixel high (CGA style) text
#define DM_TEXT14  2		// 14 pixel high (EGA style) text
#define DM_TEXT16  3		// 16 pixel high (VGA style) text

// Define bits in the third special keyboard code byte

#define SPC3_LFTALT 0x40	// Left ALT key pressed for character
#define SPC3_LFTCTL 0x20	// Left control key pressed for character
#define SPC3_LFTSHF 0x10	// Left shift key pressed for character
#define SPC3_RHTALT 0x04	// Right ALT key pressed for character
#define SPC3_RHTCTL 0x02	// Right control key pressed for character
#define SPC3_RHTSHF 0x01	// Right shift key pressed for character

// Define bits in the fourth special keyboard code byte

#define SPC4_RELEASE 0x80	// Key-release code
#define SPC4_CAPKEY  0x40	// Caps-lock key pressed for character
#define SPC4_NUMKEY  0x20	// Num-lock key pressed for chacacter
#define SPC4_SCLKEY  0x10	// Scroll-lock key pressed for character
#define SPC4_CAPLCK  0x04	// Caps-lock state true for character
#define SPC4_NUMLCK  0x02	// Num-lock state true for chacacter
#define SPC4_SCLLCK  0x01	// Scroll-lock state true for character

// Define bits for the value of IOPAR_CINPMODE and IOPAR_SINPMODE - these
// bits are also stored in tdb_imode and tdb_imodec

#define TIM_SCNALL  0x40000000	// Return all scan codes from keyboard
#define TIM_SCNCODE 0x20000000	// Return full scan codes from keyboard for
				//   all keys
#define TIM_PCSCNC  0x10000000	// Return PC mode scan codes (keyboard mode 1)
#define TIM_PC101   0x08000000	// Return extended 101/102-key keyboard scan
				//   code
#define TIM_PCDOS   0x04000000	// Filter PC scan codes for DOS (changes E0
				//   prefix
#define TIM_ANSI7   0x02000000	// Return ANSI 7 bit mode special codes
#define TIM_ANSI8   0x01000000	// Return ANSI 8 bit mode special codes
#define TIM_NOCO    0x00800000	// Control-O is not special on input
#define TIM_NOCC    0x00400000	// Control-C is not special on input
#define TIM_ALTPAD  0x00200000	// Suppress special handling of keypad digits

#define TIM_NOCOMB  0x00010000	// Do not combine forwarding groups in image
				//   mode
#define TIM_DEFER   0x00008000	// Do not wake up process immediately on image
#define TIM_ILFACR  0x00000200	// Insert LF after CR in input stream
#define TIM_RCRBLF  0x00000100	// Remove CR before LF in input stream
#define TIM_OVER    0x00000080	// Initial input edit mode is overstrike
#define TIM_DEBUG   0x00000040	// Debug mode input
#define TIM_CHAR    0x00000020	// Character mode input
#define TIM_IMAGE   0x00000010	// Image mode input
#define TIM_XIMAGE  0x00000008	// Special image mode input
#define TIM_HALFDUP 0x00000004	// Half duplex operation
#define TIM_ECHO    0x00000002	// Echo input

// Define bits for the value of IOPAR_COUTMODE and IOPAR_SOUTMODE - these
// bits are also stored in tdb_omode and tdb_omodec

#define TOM_ANSICM  0x04000000	// Do ANSI character mapping
#define TOM_PLP1    0x02000000	// Output PLP packets (old style, no
				//  XON or XOFF)
#define TOM_PLP2    0x01000000  // Output PLP packets (new style,
				//  XON and XOFF)
#define TOM_PLP     0x02000000	// Output PLP packets
#define TOM_ICRBLF  0x00000200	// Insert CR before LF is output
#define TOM_IMAGE   0x00000010	// Image mode output

// Define bits for the value of IOPAR_SPMODEM

#define SPMC_RI     0x0800	// Ring indicate change (read only, cleared when
				//   read)
#define SPMC_CD     0x0400	// Carrier detect (read only)
#define SPMC_CTS    0x0200	// Clear to send (read only)
#define SPMC_DSR    0x0100	// Data set ready (read only)
#define SPMC_LOOP   0x0010	// Loopback
#define SPMC_RTS    0x0002	// Ready to send
#define SPMC_DTR    0x0001	// Data terminal ready

// Define attribute extension bits

#define	DAX_XOR  	0x01	// XOR write (graphics mode only)

// Define terminal special device functions

#define TSF_SETCCI  1		// Set up control-C interrupt
#define TSF_SETCPI  2		// Set up control-P interrupt

// Define functions for the svcTrmFunc SVC

#define TF_RESET   0		// Reset terminal data to defaults
#define TF_ENECHO  1		// Enable echoing
#define TF_DSECHO  2		// Disable echoing
#define TF_CLRINP  3		// Clear input buffer
#define TF_CLROUT  4		// Clear output buffer
#define TF_ENBOUT  5		// Enable output

// Define special internal scan code values

#define ISC_F1      0x01	// F1 key
#define ISC_F2      0x02	// F2 key
#define ISC_F3      0x03	// F3 key
#define ISC_F4      0x04	// F4 key
#define ISC_F5      0x05	// F5 key
#define ISC_F6      0x06	// F6 key
#define ISC_F7      0x07	// F7 key
#define ISC_F8      0x08	// F8 key
#define ISC_F9      0x09	// F9 key
#define ISC_F10     0x0A	// F10 key
#define ISC_F11     0x0B	// F11 key
#define ISC_F12     0x0C	// F12 key
#define ISC_ESC     0x10	// Escape
#define ISC_INS     0x11	// Insert
#define ISC_HOME    0x12	// Home
#define ISC_PGUP    0x13	// Page up
#define ISC_DELETE  0x14	// Delete
#define ISC_END     0x15	// End
#define ISC_PGDN    0x16	// Page down
#define ISC_UPARW   0x17	// Up arrow
#define ISC_LTARW   0x18	// Left arrow
#define ISC_DNARW   0x19	// Down arrow
#define ISC_RTARW   0x1A	// Right arrow
#define ISC_KP0     0x20	// Keypad 0
#define ISC_KPINS   0x20	// Keypad insert
#define ISC_KP1     0x21	// Keypad 1
#define ISC_KPEND   0x21	// Keypad end
#define ISC_KP2     0x22	// Keypad 2
#define ISC_KPDNARW 0x22	// Keypad down arrow
#define ISC_KP3     0x23	// Keypad 3
#define ISC_KPPGDN  0x23	// Keypad page down
#define ISC_KP4     0x24	// Keypad 4
#define ISC_KPLTARW 0x24	// Keypad left arrow
#define ISC_KP5     0x25	// Keypad 5
#define ISC_KP6     0x26	// Keypad 6
#define ISC_KPRTARW 0x26	// Keypad right arrow
#define ISC_KP7     0x27	// Keypad 7
#define ISC_KPHOME  0x27	// Keypad home
#define ISC_KP8     0x28	// Keypad 8
#define ISC_KPUPARW 0x28	// Keypad up arrow
#define ISC_KP9     0x29	// Keypad 9
#define ISC_KPPGUP  0x29	// Keypad page up
#define ISC_KPPER   0x2A	// Keypad .
#define ISC_KPSLSH  0x2B	// Keypad /
#define ISC_KPSTAR  0x2C	// Keypad *
#define ISC_KPMINUS 0x2D	// Keypad - (!!!)
#define ISC_KPPLUS  0x2E	// Keypad +
#define ISC_KPENT   0x2F	// Keypad enter
#define ISC_ENT     0x38	// Enter
#define ISC_LF      0x39	// Line feed
#define ISC_BS      0x3A	// Backspace
#define ISC_HT      0x3B	// Tab
#define ISC_DEL     0x3C	// DEL
#define ISC_GRAVE   0x40	// Grave or ~
#define ISC_1       0x41	// 1 or !
#define ISC_2       0x42	// 2 or @
#define ISC_3       0x43	// 3 or #
#define ISC_4       0x44	// 4 or $
#define ISC_5       0x45	// 5 or %
#define ISC_6       0x46	// 6 or ^
#define ISC_7       0x47	// 7 or &
#define ISC_8       0x48	// 8 or *
#define ISC_9       0x49	// 9 or (
#define ISC_0       0x4A	// 0 or )
#define ISC_MINUS   0x4B	// - or _
#define ISC_EQUAL   0x4C	// = or +
#define ISC_Q       0x4D	// Q
#define ISC_W       0x4E	// W
#define ISC_E       0x4F	// E
#define ISC_R       0x50	// R
#define ISC_T       0x51	// T
#define ISC_Y       0x52	// Y
#define ISC_U       0x53	// U
#define ISC_I       0x54	// I
#define ISC_O       0x55	// O
#define ISC_P       0x56	// P
#define ISC_LBRKT   0x57	// [ or {
#define ISC_RBRKT   0x58	// ] or }
#define ISC_BSLSH   0x59	// \ or |
#define ISC_A       0x5A	// A
#define ISC_S       0x5B	// S
#define ISC_D       0x5C	// D
#define ISC_F       0x5D	// F
#define ISC_G       0x5E	// G
#define ISC_H       0x5F	// H
#define ISC_J       0x60	// J
#define ISC_K       0x61	// K
#define ISC_L       0x62	// L
#define ISC_SEMI    0x63	// ;
#define ISC_QUOTE   0x64	// ' or "
#define ISC_Z	    0x65	// Z
#define ISC_X       0x66	// X
#define ISC_C       0x67	// C
#define ISC_V       0x68	// V
#define ISC_B       0x69	// B
#define ISC_N       0x6A	// N
#define ISC_M       0x6B	// M
#define ISC_COMMA   0x6C	// , or <
#define ISC_PER     0x6D	// . or >
#define ISC_SLSH    0x6E	// / or ?
#define ISC_SPACE   0x6F	// Space
#define ISC_SCLLCK  0x73	// Scroll lock
#define ISC_NUMLCK  0x74	// Num lock
#define ISC_CAPLCK  0x75	// Caps lock
#define ISC_LFTSHF  0x76	// Left shift
#define ISC_RHTSHF  0x77	// Right shift
#define ISC_LFTALT  0x78	// Left Alt
#define ISC_RHTALT  0x79	// Right Alt
#define ISC_LFTCTL  0x7A	// Left control
#define ISC_RHTCTL  0x7B	// Right control
#define ISC_PRTSCN  0x7C	// Print screen
#define ISC_BREAK   0x7D	// Break
#define ISC_PAUSE   0x7E	// Pause
#define ISC_SYSSHF  0x7F	// System shift

// Define special device functions for PCON devices

#define PCSF_SETBUF   1		// Set up display buffer
#define PCSF_MODBUF   2		// Modify display mapping
#define PCSF_MODESET  3		// Report display mode set complete
#define PCSF_DISPTYPE 4		// Set display type
#define PCSF_FONTSET  5		// Set font related parameters
#define PCSF_STREAMMD 6		// Set to stream mode

// Define bits stored in the keyboard input buffers

#define KB_IMAGE   0x8000	// Image data
#define KB_CAPKEY  0x4000	// Caps-lock key pressed for character
#define KB_NUMKEY  0x2000	// Num-lock key pressed for chacacter
#define KB_SCLKEY  0x1000	// Scroll-lock key pressed for character
#define KB_CAPLCK  0x0400	// Caps-lock state true for character
#define KB_NUMLCK  0x0200	// Num-lock state true for chacacter
#define KB_SCLLCK  0x0100	// Scroll-lock state true for character
#define KB_LFTALT  0x0040	// Left ALT key pressed for character
#define KB_LFTCTL  0x0020	// Left control key pressed for character
#define KB_LFTSHF  0x0010	// Left shift key pressed for character
#define KB_RHTALT  0x0004	// Right ALT key pressed for character
#define KB_RHTCTL  0x0002	// Right control key pressed for character
#define KB_RHTSHF  0x0001	// Right shift key pressed for character

// Define structure for data for the svcTrmAttrib SVC

typedef struct trmattrib
{   long foregnd;	        // Foreground color
    long backgnd;	        // Background color
    long forefill;	        // Foreground fill color
    long backfill;	        // Background fill color
    long attrbits;	        // Extended attribute bits
} TRMATTRIB;

# include <_epilog.h>

#endif	// _XOSTRM_H_
