typedef struct fontheader
{   long   id1;			// 'BMF*'
    long   id2;			// 0xFFFF0000
    ushort version;		// High byte major version, low byte minor
				//   version (currently 1.0)
    ushort glyphtbl;		// Offset in file of start of the glyph
				//   pointer table (currently always xxx)
    long   length;		// Length of file
    char   copyright[64];	// Copyright notice
    char   name[32];		// Name of font
    long   flags;		// Flag bits:
				//   Bit 0: 1 if italic font
				//   Bit 1: 1 if monospaced font
				//   Other bits are reserved and must be 0
    ushort ascent;		// Ascent (pixels)
    ushort descent;		// Descent (pixels)
    ushort inleading;		// Internal leading (pixels)
    ushort exleading;		// External leading (pixels)
    ushort height;		// Height (pixels) (must be ascent + descent
				//   + inleading)
    ushort maxwidth;		// Maximum width
    ushort avgwidth;		// Average width
    ushort attr;		// Attributes
    ushort underpos;		// Underscore position
    ushort strikepos;		// Strike-out position
    uchar  firstchar;		// First character
    uchar  numchars;		// Number of characters
    uchar  dfltchar;		// Default character
    uchar  reserved1;		// Reserved, must be 0
    long   reserved2;		// Reserved, must be 0
} FONTHEADER;

#define BMF_ID1     '*FMB'
#define BMF_ID2     0xFFFF0000
#define BMF_VERSION 0x0100

// Define values for attr in FONTHEADER (must be the same as the FNTA values)

#define BMFA_XBOLD  0x1000	// Extra bold
#define BMFA_BOLD   0x0800	// Bold
#define BMFA_DBOLD  0x0400	// Demi-bold
#define BMFA_NORMAL 0x0200	// Normal
#define BMFA_LIGHT  0x0100	// Light
#define BMFA_ITALIC 0x0001	// Italic
