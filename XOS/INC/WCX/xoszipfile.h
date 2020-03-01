// Define structure for each local file header in the ZIP-file

typedef struct
{   ulong  id;					// Signature (0x04034B50)
    ushort extract;				// Version needed to extract
    ushort flag;				// Bit flags
    ushort method;				// Compression method
    ushort time;				// File modification time (DOS format)
    ushort date;				// File modification date (DOS format)
    ulong  crc32;				// CRC-32 for file
    ulong  comsize;				// Compressed size
    ulong  size;				// Uncompressed size
    ushort namelen;				// Length of the name field
    ushort extralen;			// Length of the extra field
} LFH;

#define ZIPLFHID 0x04034B50

// Define structure for each central directory header in the ZIP-file

typedef struct
{   ulong  id;					// Signature (0x02014B50)
    ushort madeby;				// Version this ZIP-file made by
    ushort extract;				// Version needed to extract
    ushort flag;				// Bit flags
    ushort method;				// Compression method
    ushort time;				// File modification time (DOS format)
    ushort date;				// File modification date (DOS format)
    ulong  crc32;				// CRC-32 for file
    ulong  comsize;				// Compressed size
    ulong  size;				// Uncompressed size
    ushort namelen;				// Length of the name field
    ushort extralen;			// Length of the extra field
    ushort commentlen;			// Length of the comment field
    ushort disknum;				// Disk number for lfh
    ushort intattr;				// Internal file attributes
    ulong  extattr;				// External file attributes
    ulong  lfh;					// Offset of local file header in ZIP-file
} CDH;

#define ZIPCDHID 0x02014B50

// Define structure for the end of central directory record in the ZIP-file

typedef struct
{   ulong  id;					// Signature (0x06054B50)
    ushort disknum;				// Number of this disk
    ushort startnum;			// Number of disk where central directory starts
    ushort enthere;				// Number of central directory entries on this
								//   disk
    ushort enttotal;			// Total number of central directory entries
    ulong  size;				// Size of the central directory
    ulong  start;				// Offset in ZIP-file of start of central
								//   directory
    ushort commentlen;			// Comment length
} ECD;

#define ZIPECDID 0x06054B50

// Define bits for the flag item in LFH

#define LFB_ENCRYPT  0x0001		// File is encrypted
#define LFB_LEVEL    0x0006		// Compression level
  #define LEVEL_NORM   0x0000	//   Normal compression
  #define LEVEL_MAX    0x0002	//   Maximum compression
  #define LEVEL_FAST   0x0004	//   Fast compression
  #define LEVEL_SUPER  0x0006	//   Super fast compression
#define LFB_DESC     0x0008		// Data descriptor is present
#define LFB_PATCHED  0x0020		// Compressed patched data (not supported)

// Define value for the method item in LFH

#define METHOD_STORE    0		// Stored (no compression)
#define METHOD_SHRUNK   1		// Shrunk
#define METHOD_REDUCE1  2		// Reduced with compression factor 1
#define METHOD_REDUCE2  3		// Reduced with compression factor 2
#define METHOD_REDUCE3  4		// Reduced with compression factor 3
#define METHOD_REDUCE4  5		// Reduced with compression factor 4
#define METHOD_IMPLODE  6		// Impleded
#define METHOD_TOKEN    7		// Reserved for tokenizing compression algorithm
#define METHOD_DEFLATE  8		// Deflated
#define METHOD_EDEFLATE 9		// Reserved for enhanced deflating
#define METHOD_PKDATE   10		// PKWARE date compression library imploding
