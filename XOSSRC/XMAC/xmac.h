//*******************************
// Parameter definitions for XMAC
//*******************************
// Written by John Goltz
//*******************************

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

#define FALSE 0
#define TRUE  1

#define HASHSIZE  128			// Size of the symbol hash table
#define INCLMAX   6				// Maximum include nesting level
#define INCLFSSZ  100			// Maximum length for include file
								//   specification
#define SYMMAXSZ  64			// Maximum length for symbol name
#define DIGMAXSZ  20			// Maximum number of digits in a number
#define IFSTRSZ   60			// Maximum string length for .IF compares
#define BLBSIZE   200			// Size of binary listing buffer
#define ALBSIZE   100			// Size of alpha listing buffer
#define BLKSIZE   (sizeof(struct sy)) // Size of a memory block
#define LSTPGSZ   50			// Page length (excluding header) for listing
#define TITLESZ   67			// Maximum length for title text
#define SBTTLSZ   120			// Maximum length for subtitle text
#define ETBLSIZE  8				// Size of error message pointer table
#define CONDMAX   20			// Maximum nesting level for conditionals

#define offsetof(strt, label) (int)&(((struct strt *)0)->label)

// Define listing types

#define LT_NONE   0
#define LT_NORMAL 1
#define LT_CREF   2
#define LT_DEBUG  3

// Define value types for binary output

#define U_VALUE  0		// Unsigned value
#define S_VALUE  1		// Signed value

// Define error flag bits

#define ERR_Q  0x80000000L	// Questionable line
#define ERR_V  0x40000000L	// Value error
#define ERR_U  0x20000000L	// Undefined symbol
#define ERR_A  0x10000000L	// Addressing error
#define ERR_T  0x08000000L	// Value truncated
#define ERR_R  0x04000000L	// Address out of range
#define ERR_P  0x02000000L	// Phase error
#define ERR_M  0x01000000L	// Multipley defined symbol
#define ERR_O  0x00800000L	// Illegal opcode
#define ERR_C  0x00400000L	// Attribute conflict
#define ERR_X  0x00200000L	// Illegal expression
#define ERR_D  0x00100000L	// Reference to multipley defined symbol
#define ERR_K  0x00080000L	// Illegal angle bracket usage
#define ERR_S  0x00020000L	// Operation size error
#define ERR_E  0x00010000L	// .ERROR pseudo-op
#define ERR_I  0x00008000L	// Invalid segment or msect name
#define ERR_DN 0x00004000L	// Different segment or msect name
#define ERR_CE 0x00002000L	// Conditional error
#define ERR_CX 0x00001000L	// Conditional nesting too deep
#define ERR_CO 0x00000800L	// Open conditional at end of assembly
#define ERR_UM 0x00000400L	// Open macro or repeat definition at end of
				//   assembly
#define ERR_NA 0x00000200L	// Value is not an address
#define ERR_NP 0x00000100L	// No psect for code or data
#define ERR_IN 0x00000080L	// Illegal .INCLUD or .REQUIR file name
#define ERR_XX 0x00000001L	// No error (used to force listing for
				//   .PRINT
// Define processor type values (stored in ptype)

#define P_8086  2
#define P_80186 3
#define P_80286 4
#define P_80386 5
#define P_80486 6

// Define bits for pbits and ot_proc bytes

#define B_80486 0x10
#define B_80386 0x08
#define B_80286 0x04
#define B_80186 0x02
#define B_8086  0x01

// Define REL file section types

#define SC_EOF    0		// End of file
#define SC_INDEX  1		// Library index section
#define SC_START  2		// Start of module section
#define SC_SEG    3		// Segment definition section
#define SC_MSECT  4		// Msect definition section
#define SC_PSECT  5		// Psect definition section
#define SC_DATA   6		// Data section
#define SC_INTERN 7		// Internal symbol definition section
#define SC_LOCAL  8		// Local symbol definition section
#define SC_EXPORT 9		// Exported symbol definition section
#define SC_STRADR 10		// Starting address section
#define SC_DEBUG  11		// Debugger address section
#define SC_STACK  12		// Initial stack address section
#define SC_FILREQ 13		// File request section
#define SC_EXTERN 14		// External symbol definition section
#define SC_SHARED 15		// Shared library definition section
#define SC_IMPORT 16		// Imported symbol definition section

// Define polish operators

#define PO_ADD        0x80	// Add
#define PO_SUB        0x81	// Subtract
#define PO_MUL        0x82	// Multiply
#define PO_DIV        0x83	// Divide
#define PO_SHF        0x84	// Shift
#define PO_AND        0x85	// And
#define PO_IOR        0x86	// Inclusive or
#define PO_XOR        0x87	// Exclusive or
#define PO_COM        0x88	// 1s complement
#define PO_NEG        0x89	// Negate (2s complement)
#define PO_PSELCP     0x92	// Push selector, current psect
#define PO_PSELAP     0x93	// Push selector, any psect
#define PO_PSELAM     0x94	// Push selector, any msect
#define PO_PSELAS     0x95	// Push selector, any segment
#define PO_POFSAM     0x96	// Push offset, any msect
#define PO_PSELSYM    0x97	// Push selector, external symbol
#define PO_STRAW      0x98	// Store word address
#define PO_STRAL      0x99	// Store long address
#define PO_STRUB      0x9A	// Store unsigned byte
#define PO_STRUW      0x9B	// Store unsigned word
#define PO_STRUL      0x9C	// Store unsigned long
#define PO_STRSB      0x9D	// Store signed byte
#define PO_STRSW      0x9E	// Store signed word
#define PO_STRSL      0x9F	// Store signed long
#define PO_PVAL       0xA0	// Push absolute value
#define PO_PVALREL    0xA8	// Push absolute value, relative
#define PO_POFSCP     0xB0	// Push relocated value, current psect
#define PO_POFSAP     0xC0	// Push relocated value, any psect
#define PO_POFSRELAP  0xC8	// Push relocated value, relative, any psect
#define PO_POFSSYM    0xF0	// Push symbol offset value
#define PO_POFSRELSYM 0xF8	// Push symbol offset value, relative

// Define value prefix bits which specify length of value

#define PX_0BV   0		// No value bytes
#define PX_1BV   1		// One value byte
#define PX_2BV   2		// Two value bytes
#define PX_4BV   3		// Four value bytes
#define PX_EX0   0		// Extend value with 0s
#define PX_EX1   4		// Extend value with 1s

// Define value types

#define VL_VAL   0		// General value
#define VL_BYTE  1		// Byte value
#define VL_WORD  2		// Word value
#define VL_LONG  3		// Long value
#define VL_VAR   4		// Variable length value
#define VL_BREL  5		// Byte relative value
#define VL_WREL  6		// Word relative value
#define VL_LREL  7		// Long relative value
#define VL_VREL  8		// Variable length relative value

// Define value kinds

#define VK_OFS 0		// Offset value
#define VK_ABS 1		// Absolute value
#define VK_ABA 2		// Absolute address
#define VK_EXT 3		// External value
#define VK_MSC 4		// Msect value
#define VK_SEG 5		// Segment value
#define VK_REG 6		// Register name
#define VK_SEL 7		// Selector value
#define VK_SLX 8		// Selector of external value

// Define operand types

#define OP_REG   0		// Register
#define OP_IMM   1		// Immediate value
#define OP_MEM   2		// Simple memory operand
#define OP_CM1   3		// Memory operand (single addressing byte)
#define OP_CM2   4		// Memory operand (two addressing bytes)

// Define listing type values

#define LT_NONE  0		// No data
#define LT_BYTE  1		// Byte
#define LT_WORD  2		// Word
#define LT_LONG  3		// Long
#define LT_WADR  4		// Word address
#define LT_LADR  5		// Long address

// Define register values

#define RG_AL   0		// Register AL (8 bit)
#define RG_AH   1		// Register AH (8 bit)
#define RG_BL   2		// Register BL (8 bit)
#define RG_BH   3		// Register BH (8 bit)
#define RG_CL   4		// Register CL (8 bit)
#define RG_CH   5		// Register CH (8 bit)
#define RG_DL   6		// Register DL (8 bit)
#define RG_DH   7		// Register DH (8 bit)
#define RG_AX   8		// Register AX (16 bit)
#define RG_BX   9		// Register BX (16 bit)
#define RG_CX   10		// Register CX (16 bit)
#define RG_DX   11		// Register DX (16 bit)
#define RG_SI   12		// Register SI (16 bit)
#define RG_DI   13		// Register DI (16 bit)
#define RG_BP   14		// Register BP (16 bit)
#define RG_SP   15		// Register SP (16 bit)
#define RG_EAX  16		// Register EAX (32 bit)
#define RG_EBX  17		// Register EBX (32 bit)
#define RG_ECX  18		// Register ECX (32 bit)
#define RG_EDX  19		// Register EDX (32 bit)
#define RG_ESI  20		// Register ESI (32 bit)
#define RG_EDI  21		// Register EDI (32 bit)
#define RG_EBP  22		// Register EBP (32 bit)
#define RG_ESP  23		// Register ESP (32 bit)
#define RG_CS   24		// Register CS (16 bit)
#define RG_DS   25		// Register DS (16 bit)
#define RG_SS   26		// Register SS (16 bit)
#define RG_ES   27		// Register ES (16 bit)
#define RG_FS   28		// Register FS (16 bit)
#define RG_GS   29		// Register GS (16 bit)
#define RG_PC   30		// Register PC (16 or 32 bit)
#define RG_FR   31		// Register FR (16 or 32 bit, flag register)
#define RG_LDTR 32		// Register LDTR (16 bit, system level)
#define RG_MSW  33		// Register MSW (16 or 32 bit, system level)
#define RG_TR   34		// Register TR (16 bit, system level)
#define RG_GDTR 35		// Register GDTR (48 bit, system level)
#define RG_IDTR 36		// Register IDTR (48 bit, system level)
#define RG_CR0  37		// Register CR0 (32 bit, system level)
#define RG_CR2  38		// Register CR2 (32 bit, system level)
#define RG_CR3  39		// Register CR3 (32 bit, system level)
#define RG_DR0  40		// Register DR0 (32 bit, system level)
#define RG_DR1  41		// Register DR1 (32 bit, system level)
#define RG_DR2  42		// Register DR2 (32 bit, system level)
#define RG_DR3  43		// Register DR3 (32 bit, system level)
#define RG_DR6  44		// Register DR6 (32 bit, system level)
#define RG_DR7  45		// Register DR7 (32 bit, system level)
#define RG_TR6  46		// Register TR6 (32 bit, system level)
#define RG_TR7  47		// Register TR7 (32 bit, system level)
#define RG_ST0  48		// Register ST0 (FPU TOS)
#define RG_ST1  49		// Register ST1 (FPU TOS - 1)
#define RG_ST2  50		// Register ST2 (FPU TOS - 2)
#define RG_ST3  51		// Register ST3 (FPU TOS - 3)
#define RG_ST4  52		// Register ST4 (FPU TOS - 4)
#define RG_ST5  53		// Register ST5 (FPU TOS - 5)
#define RG_ST6  54		// Register ST6 (FPU TOS - 6)
#define RG_ST7  55		// Register ST7 (FPU TOS - 7)

// Define bits for the regbits table

#define RB_DAT   0x80		// Register is valid as general data operand
#define RB_F16I  0x40		// Register is valid as first 16 bit index
#define RB_F32I  0x20		// Register is valid as first 32 bit index
#define RB_S32I  0x10		// Register is valid as second 32 bit index
#define RB_SEG   0x08		// Register is a segment register
#define RB_CNTL  0x04		// Register is a processor control register
#define RB_FLOAT 0x02		// Register is an FPU register

// Define type for general pointer

typedef union
{   long  *l;
    short *s;
    char  *c;
} genpnt;

// Define type for general location

typedef union
{   long  l;
    short s;
    char  c;
} genloc;

// Define structure for local symbol value

struct locsym
{   int   ls_flag;		// Always 0 to indicate local symbol
    long  ls_blk;		// Local symbol block number (lsbnum)
    long  ls_val;		// Local symbol value
};

// Define structure for pseudo-op attributes data

struct atributes
{   char  at_name[6];
    void (*at_func)(void *);
};

// Define structure for symbol table entry (WARNING: Order of items through
//   sy_flag must match corresponding items in the segment, msect, and psect
//   name blocks!)

struct sy
{   struct   sy *sy_hash;		// Hash list link
    struct   sy *sy_sort;		// Pointer for heapsort
    char     sy_name[SYMMAXSZ];		// Symbol name
    union
    {   long         val;		// Value
        struct ab   *ab;
        struct body *body;
    }        sy_val;
    int      sy_psect;			// Psect number
    char     sy_type;			// Symbol table entry type
    char     sy_xxxx;			// Not used
    int      sy_flag;			// Flag bits
    int      sy_symnum;			// Symbol number
    struct   rf *sy_ref;		// Pointer to start of reference list
};

// Define structure for general argument lists

struct ab
{   struct   ab *ab_next;	// Pointer to next argument list
    char     ab_data[BLKSIZE-2*sizeof(char *)];
    struct   ab *ab_link;	// Link to next block in this list
};

// Define structure for repeat control block (ICB)

struct ic
{   struct ic *ic_link;		// Link to next level ICB
    char  *ic_body;		// Pointer to beginning of body
    char  *ic_cbp;		// Current block pointer
    char  *ic_sbp;		// Saved block pointer
    char   ic_cbc;		// Current block count
    char   ic_sbc;		// Saved block count
    union
    {   struct ab *arg;		// Pointer to beginning of argument list
	char *argc;		// Pointer to .IRPC argument string
	long   cnt;		// Repeat counter
    } ic_;
    char   ic_type;		// Block type
    char   ic_gac;		// Generated argument maximum number
};

// Define values for ic_type

#define ICT_MAC   0		// Macro
#define ICT_REPT  1		// Repeat
#define ICT_IRP   2		// IRP
#define ICT_IRPC  3		// IRPC

// Define values for special inserted characters

#define CC_IMA  0200		// Insert macro argument
#define CC_IMX  0201		// Insert macro argument with generated
				//   symbol if null
#define CC_G1S  0202		// Insert first character of generated
				//   symbol
#define CC_G2S  0203		// Insert second character of generated
				//   symbol
#define CC_G3S  0204		// Insert third character of generated symbol
#define CC_G4S  0205		// Insert fourth character of generated
				//   symbol
#define CC_IIA  0206		// Insert IRP argument
#define CC_ICA  0207		// Insert IRPC argument
#define CC_EMB  0210		// End of macro body
#define CC_ERB  0211		// End of repeat body
#define CC_EIB  0212		// End of IRP body
#define CC_ECB  0213		// End of IRPC body
#define CC_EAG  0214		// End of macro or IRP argument

// Define structure for body block

struct body
{   char   bb_data[BLKSIZE-sizeof(char *)]; // Data
    struct body *bb_link;	// Pointer to next body block
};

// Define structure for reference list initial block

struct rf
{   union			// Pointer
    {   int  *ps;
	int **pps;
    } rf_pnt;
    int rf_cnt;			// Item count
    int rf_data[2];		// First data pair
};

// Define structure for the .REQUIR list blocks

struct reqblk
{   struct reqblk *next;
    int    size;
    char   name[4];
};

// Define structure for op-code table entry

struct optbl
{   char  ot_name[6];		// Name of op-code (less first character)
    void (*ot_dsp)(struct optbl *tbl); // Pointer to routine for type
    char  ot_val1;
    char  ot_val2;
    char  ot_val3;
    char  ot_val4;
    char  ot_size;
    char  ot_proc;
    short ot_xxx;
    long  ot_flag;
};

// Define structure for pseudo-op table entry

struct pstbl
{   char  pt_name[6];
    void (*pt_dsp)(void);
};

struct cptbl
{   char  pt_name[6];
    int (*pt_dsp)(void);
};

// Define structure for if table entry

struct iftbl
{   char  if_name[3];
    void (*if_dsp)(void);
};

// Define bits for ot_flag byte

#define OF_BO 0x4000	// 8-bit relative operand value allowed
#define OF_RD 0x2000	// Two operand instruction can have register as
			//   destination
#define OF_RF 0x1000	// First operand is register operand
#define OF_BS 0x0800	// Source operand is a byte
#define OF_WS 0x0400	// Source operand is a word
#define OF_SS 0x0200	// Need operand prefix for stack item size selection
#define OF_MS 0x0100	// Two operand instruction can have memory field as
			//   source
#define OF_B3 0x0080	// Branch instruction has 80386 only 16 bit from
#define OF_RS 0x0040	// Source memory field can be register
#define OF_MD 0x0020	// Two operand instruction can have memory field as
			//   destination
#define OF_XP 0x0010	// 30386 prefix byte needed
#define OF_NS 0x0008	// Do not use operand size prefix byte
#define OF_BI 0x0004	// Immediate value is always single byte
#define OF_DB 0x0002	// Instruction uses direction bit
#define OF_SB 0x0001	// Instruction uses size bit

// Define values for sy_type

#define SYT_SYM  1		// Symbol or macro name
#define SYT_PSC  2		// Psect
#define SYT_MSC  3		// Msect
#define SYT_SEG  4		// Segment

// Define flag bits for sy_flag

#define SYF_EXPORT 0x1000	// Exported symbol
#define SYF_IMPORT 0x0800	// Imported symbol
#define SYF_LABEL  0x0400	// Label
#define SYF_EXTERN 0x0200	// External symbol
#define SYF_UNDEF  0x0100	// Undefined

// Following 4 bits must match the symbol flag bits in the OBJ file
//   symbol table entry

#define SYF_ABS    0x0080	// Absolute symbol
#define SYF_ENTRY  0x0040	// Entry symbol
#define SYF_ADDR   0x0020	// Address
#define SYF_SUP    0x0010	// Suppressed

#define SYF_INTERN 0x0008	// Internal symbol
#define SYF_MULT   0x0004	// Multipily defined
#define SYF_USED   0x0002	// Used symbol
#define SYF_REG    0x0001	// Register

// Define union for symbol buffer

union sybf
{   char   nsym[SYMMAXSZ];	// Normal symbol
    struct locsym lsym;		// Local symbol
};

// Define structure for value of expression

// WARNING: This is byte order dependent - this definition is for
//   low order byte first machines!!

struct vl
{   union
    {   int  vl_vi;
        long vl_vl;
	struct
	{   short ls;
	    short hs;
	} vl_vs;
	struct
	{   char lslc;
            char lshc;
            char hslc;
            char hshc;
	} vl_vc;
    } vl_val;			// Numeric value
    struct sy *vl_pnt;		// Pointer to symbol table entry for value
    int    vl_psect;		// Psect number or selector for value
    char   vl_type;		// Value type
    char   vl_size;		// Value size
    char   vl_kind;		// Value kind
    char   vl_undef;		// TRUE if undefined value
};

// Define structure for operand data returned by getopr

// WARNING: This is byte order dependent - this definition is for
//   low order byte first machines!!

struct op
{   struct vl op_val;		// Numeric value of operand
    uchar op_type;		// Operand type
    uchar op_size;		// Operand size
    uchar op_modrm;		// Value for mod-r/m byte
    uchar op_sib;		// Value for sib byte
    uchar op_reg;		// Operand register value
    uchar op_apfx;		// Address size prefix byte value
    uchar op_nab;		// Number of address bytes needed
    uchar op_seg;		// Segment prefix byte value
    uchar op_asize;		// Address size
};

// Define structure of the segment data block

struct sd
{   struct sd *sd_next;		// Pointer to next segment data block
    struct sn *sd_nba;		// Pointer to segment name block
    int    sd_sgn;		// Segment number
    char   sd_atrb;		// Segment attributes
    char   sd_flag;		// Segment flag bits
    char   sd_type;		// Segment type
    char   sd_priv;		// Priviledge level
    struct sd *sd_linked;	// Pointer to linked segment
    long   sd_mod;		// Physical address modulus
    long   sd_addr;		// Physical address
    int    sd_select;		// Segment selector value
};

// Define bits for sd_atrb

#define SA_LARGE  0x80		// Large segment
#define SA_32BIT  0x40		// 32 bit segment
#define SA_CONF   0x20		// Conformable segment
#define SA_READ   0x10		// Readable segment
#define SA_WRITE  0x08		// Writable segment

// Define bits for sd_flag

#define SF_EXTEND 0x80		// Entendable segment
#define SF_LINKED 0x10		// Linked segment
#define SF_ADDR   0x08		// Address specified for segment

// Define values for sd_type

#define ST_DATA  1
#define ST_CODE  2
#define ST_STACK 3
#define ST_COMB  4

// Define structure of the msect data block

struct md
{   struct md *md_next;		// Pointer to next msect data block
    struct mn *md_nba;		// Pointer to msect name block
    int    md_msn;		// Msect number
    char   md_atrb;		// Msect attributes
    char   md_flag;		// Msect flag bits
    char   md_xxx[2];
    int    md_sgn;		// Segment number
    long   md_max;		// Maximum address space needed
    long   md_addr;		// Msect address
    long   md_mod;		// Msect modulus
    struct sd *md_sdb;		// Address of data block for segment
};

// Define bits for md_atrb

#define MA_READ  0x10		// Readable msect
#define MA_WRITE 0x08		// Writable msect

// Define bits for md_flag

#define MF_ADDR  0x08		// Address specified

// Define structure of the psect data block

struct pd
{   struct pd *pd_next;		// Pointer to next psect data block
    struct pn *pd_nba;		// Pointer to psect name block
    int    pd_psn;		// Psect number
    char   pd_atrb;		// Psect attributes
    char   pd_flag;		// Psect flag bits
    char   pd_xxx[2];
    int    pd_msn;		// Msect number
    long   pd_ofs;		// Current offset in psect
    long   pd_tsize;		// Total size of psect
    long   pd_lsize;		// Loaded size of psect
    long   pd_addr;		// Address or modulus for psect
    struct md *pd_mdb;		// Address of data block for msect
};

// Define bits for pd_atrb

#define PA_OVER  0x40		// Overlayed psect

// Define bits for pd_flag

#define PF_ADDR  0x08		// Address specified

// Define structure for segment name block

struct sn
{   struct sn *sn_hash;		// Hash list link
    struct sn *sn_sort;
    char   sn_name[SYMMAXSZ];	// Psect name
    struct sd *sn_dba;		// Pointer to psect data block
    int    sn_sgn;		// Segment number
    char   sn_type;		// Symbol table entry type (always = SYT_SEG)
    char   sn_xxx[3];
    int    sn_flag;		// Flag bits
};

// Define structure for msect name block

struct mn
{   struct mn *mn_hash;		// Hash list link
    struct mn *mn_sort;
    char   mn_name[SYMMAXSZ];	// Psect name
    struct md *mn_dba;		// Pointer to msect data block
    int    mn_msn;		// Msect number
    char   mn_type;		// Symbol table entry type (always = SYT_MSC)
    char   mn_xxx[3];
    int    mn_flag;		// Flag bits
};

// Define structure for psect name block

struct pn
{   struct pn *pn_hash;		// Hash list link
    struct pn *pn_sort;
    char   pn_name[SYMMAXSZ];	// Psect name
    struct pd *pn_dba;		// Pointer to psect data block
    int    pn_psn;		// Psect number
    char   pn_type;		// Symbol table entry type (always = SYT_PSC)
    char   pn_xxx[3];
    int    pn_flag;		// Flag bits
};

// Define structure for block on memory free list

struct fb
{   struct fb *fb_next;		// Link to next free list block
    char   fb_xxxx[BLKSIZE - sizeof(struct fb *)];
};

// Define structure for source block

struct sb
{   struct sb *sb_next;		// Link to next source block
    char   sb_fsp[2];		// File specification
};
#define SB_fsp sizeof(struct sb *) // Offset for file specification in sb

// Function prototypes

void   absbyte(long value);
void   absword(long value);
int    adrerr(void);
void   asmline(void);
void   assemble(void);
void   badopcode(void);
void   binbyte(char value);
void   bindata(char value);
void   binfin(void);
void   bininit(void);
void   binlong(long value);
void   binnumber(ulong value);
void   binsyms(char type, int fbit);
void   binval(long value, char prefix);
void   binword(long value);
void   binxsm(char *str);
int    blkifs(void);
void   bumpofs(int amnt);
void   chkadr(void);
int    chkarg(char argval, char chr);
int    chkendx(void);
int    chkend(char chr);
void   chkhead(void);
void   chkpsect(void);
void   chkrel(struct vl *value);
int    chksym(char chr);
int    chksys(char chr);
void   chrstring(void (*func)(long c));
int    cmpifs(void);
void   cntlreg(int dbit, int creg, int dreg);
int    compsym(struct sy *sym1, struct sy *sym2);
void   copytext(char *buffer ,int size);
int    cpendc(void);
int    cpif(void);
int    cpiff(void);
int    cpift(void);
int    cpiftf(void);
char  *defspec(char *fsp, char *none, char *ext);
void   doblk(int size);
void   eatifs(void);
void   endins(void);
void   endsect(void);
struct sy *entersym(void);
void   entint(int flags, int clear);
void   errorc(void);
void   errorq(void);
void   errorna(void);
void   errorx(void);
void   errsub(char *msg, long flag);
void   exprsn(void);
void   extimpsect(int type, int flags);
struct sy *findsym(int def);
void   finish(void);
void   general(struct vl *value, char store);
void   getabs(void);
char   getadg(char chr);
void   getals(void);
long   getanum(int radix);
struct ab *getargs(void);
int    getaval(ulong *value);
struct fb *getblock(void);
void   getconcat(void);
void   getdmy(char chr, int genflg);
struct sb *getfsp(char *arg, int offset, char *dftext);
int    getifx(void);
int    getinx(void);
struct sy *getnxs(void);
int    getopr(struct op *opd, struct optbl *opt, int rmask);
int    getopr1(struct op *opd, struct optbl *opt, int rmask);
int    getoprm(struct op *opd, struct optbl *opt);
int    getscale(struct op *opd);
int    getsegnumms(int msect);
int    getsegnumps(int psect);
int    getsrc(void);
int    getsym(char chr);
int    getsyq(void);
void   givblock(struct fb *block);
void   givlist(struct ab *block);
void   givprm(void);
void   lowerchr(long chr);
int    havearg(char *);
void   helpprint(char *helpstring, int state, int newline);
void   hndlatr(struct atributes *tbl, int size, struct pd *block);
void   ibranch(struct optbl *opt);
void   icallf(struct optbl *opt);
void   iclear(struct optbl *opt);
void   idblshf(struct optbl *opt);
void   ientrins(struct optbl *opt);
void   ifb(void);
void   ifdef(void);
void   ifdif(void);
int    ifend(char chr);
void   ifeq(void);
void   iffalse(void);
void   ifge(void);
void   ifgt(void);
void   ifidn(void);
void   ifle(void);
void   iflt(void);
void   ifnb(void);
void   ifndf(void);
void   ifne(void);
void   ifp1(void);
void   ifp2(void);
void   ifstsw(struct optbl *opt);
void   iftrue(void);
void   iimul(struct optbl *opt);
void   iimul3op(struct op *dopd, struct op *sopd, struct op *iopd,
  struct optbl *opt);
void   iincdec(struct optbl *opt);
void   iinout(struct optbl *opt);
void   iinoutop(int byte, struct optbl *opt);
void   iintins(struct optbl *opt);
void   ijumpf(struct optbl *opt);
void   iloop(struct optbl *opt);
void   imovins(struct optbl *opt);
int    incond(void);
int    indxexp(struct op *opd, int size);
void   insbody(int type, struct body *body, struct ab *argval);
void   invmac(struct sy *sym);
void   ionebyte(struct optbl *opt);
void   ionebytx(struct optbl *opt);
void   ioneopr(struct optbl *opt);
void   ioneoprf(struct optbl *opt);
void   ioneoprw(struct optbl *opt);
void   ipushpop(struct optbl *opt);
void   iret(struct optbl *opt);
void   irpdmy(void);
void   ishrot(struct optbl *opt);
void   istrins(struct optbl *opt);
void   ithrbyte(struct optbl *opt);
void   itwobyte(struct optbl *opt);
void   itwoopr(struct optbl *opt);
void   itwooprf(struct optbl *opt);
void   ixchg(struct optbl *opt);
void   level1(struct vl *value);
void   level2(struct vl *value);
void   level3(struct vl *value);
void   level4(struct vl *value);
void   level5(struct vl *value);
void   level6(struct vl *value);
void   level7(struct vl *value);
void   level8(struct vl *value);
void   level9(struct vl *value);
void   listadr(void);
void   listbin(void);
void   listchr(char chr);
void   listent(struct sy *sym, int mac, int endflag);
void   listesym(int cnt, char *fmt, int flag);
void   listfin(void);
void   listinit(void);
void   listline(void);
void   listlong(struct vl *value);
void   listnxl(void);
void   listrlc(uchar reloc);
void   listsel(struct vl *value);
void   liststr(char *str);
void   listtbl(struct sy *sym, char *head1, char *head2, char *fill, int mac);
void   listtoc(void);
void   listword(struct vl *value);
struct sy *looksym(void);
int    macarg(void);
void   macbody(char argval, char endval, int (*nestfnc)(void), char endchr);
int    macnest(void);
void   matraddr(struct md *msd);
void   matrmax(struct md *msd);
void   matrmod(struct md *msd);
void   matrread(struct md *msd);
void   matrwrite(struct md *msd);
void   movmem1(struct optbl *opt);
void   movmemimm(struct optbl *opt);
void   movmemreg(struct optbl *opt);
void   movrmmr(int dbit, struct op *ropd, struct op *mopd, struct optbl *opt);
void   movreg1(struct optbl *opt);
void   movregimm(struct optbl *opt);
void   movregmem(struct optbl *opt);
void   movregreg(struct optbl *opt);
struct sy *nchsty(void);
void   nextpage(void);
int    nextref(void);
char   nxthrtn(long value);
char   nxtnb0(char chr);
char   nxtnbc(void);
char   nxtchar(void);
char   nxtcrtn(char chr);
void   op2mem1(struct optbl *opt);
void   op2memimm(struct optbl *opt);
void   op2memreg(struct optbl *opt);
void   op2reg1(struct optbl *opt);
void   op2regimm(struct optbl *opt);
void   op2regmem(struct optbl *opt);
void   op2regreg(struct optbl *opt);
void   opcdout(long value, struct op *opd, struct optbl *opt);
void   oprdout(long extra, struct op *opd);
void   paddrw(void);
void   paddrl(void);
void   pseg(void);
void   pseg16(void);
void   pseg32(void);
void   palmex(void);
void   pascii(void);
void   pascil(void);
void   pasciu(void);
void   pasciz(void);
void   patraddr(struct pd *psd);
void   patrmod(struct pd *psd);
void   patrover(struct pd *psd);
void   pblkb(void);
void   pblkw(void);
void   pblkl(void);
void   pblmex(void);
void   pcot(void);
void   pcref(void);
void   pdebug(void);
void   peerror(void);
void   pendc(void);
void   pentry(void);
void   persym(void);
void   peven(void);
void   pexpb(void);
void   pexpw(void);
void   pexpl(void);
void   pexport(void);
void   pextern(void);
void   pif(void);
void   piff(void);
void   pift(void);
void   pimport(void);
void   pinclud(void);
void   pintern(void);
void   pirp(void);
void   pirpc(void);
void   plbex(void);
void   plist(void);
void   plnkseg(void);
void   plsb(void);
void   plsym(void);
void   pmacro(void);
void   pmsect(void);
void   pmod(void);
void   pnchar(void);
void   pncot(void);
void   pncref(void);
void   pngsym(void);
void   pnlbex(void);
void   pnlist(void);
void   pnlmex(void);
void   pnlsym(void);
void   pnrname(void);
void   podd(void);
void   ppage(void);
void   pparm(void);
void   pprint(void);
void   pprname(void);
void   pproc(void);
void   ppsect(void);
void   pradix(void);
void   prept(void);
void   prequir(void);
void   primary(struct vl *value);
void   proc80186(void);
void   proc80286(void);
void   proc80386(void);
void   proc80486(void);
void   proc8086(void);
void   psbttl(void);
void   pslmex(void);
void   pstack(void);
void   pstart(void);
void   pstk16(void);
void   pstk32(void);
void   pstype(void);
void   ptitle(void);
void   putaddrl(struct vl *value);
void   putaddrw(struct vl *value);
void   putbody(char chr);
void   putbyte(int type, struct vl *value);
void   putlong(int type, struct vl *value);
void   putsel(struct vl *value);
void   putword(int type, struct vl *value);
int    queerr(void);
void   refent(long word, struct rf *ref);
void   refsym(struct sy *sym, int def);
int    regexp(void);
int    reptnest(void);
void   resetlst(void);
void   satr16b(struct sd *sgd);
void   satr32b(struct sd *sgd);
void   satrext(struct sd *sgd);
void   satrcode(struct sd *sgd);
void   satrcomb(struct sd *sgd);
void   satrconf(struct sd *sgd);
void   satrdata(struct sd *sgd);
void   satrlarge(struct sd *sgd);
void   satraddr(struct sd *sgd);
void   satrmod(struct sd *sgd);
void   satrpriv(struct sd *sgd);
void   satrread(struct sd *sgd);
void   satrselect(struct sd *sgd);
void   satrstack(struct sd *sgd);
void   satrwrite(struct sd *sgd);
void   setbody(void);
int    setref(struct sy *sym);
void   sizerr(void);
void   skipcond(void);
void   skiprest(void);
struct sy *sorttbl(struct sy *sym);
struct sy *srchtbl(char *sym, void *tbl, int tblsize, int strsize, int entsize);
void   strsect(char type);
void   tblhead(void);
void   tblnxtl(void);
void   upperchr(long chr);
void   valueout(struct op *opd, struct optbl *opt);
void   valuesout(struct op *opd, struct optbl *opt);
void   wrtblk(void);
