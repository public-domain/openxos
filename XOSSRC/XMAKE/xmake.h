/*
 * Name:
 *	XMAKE.h
 */

#define VERSION 3
#define EDITNO  3

#define PREREQ_MAGIC 123
#define FILE_MAGIC   543
#define SHELL_MAGIC  678
#define TARG_MAGIC   987
#define SYMBOL_MAGIC 653

#define MAXLIN     132
#define SYMLEN     39
#define MAXTARGETS 12
#define CMDSIZE    4000			// Max length of command line generated
#define OPTSIZE    2000			// Max length of command options
#define LINESIZE   512			// Max length of input line

/*
 * Data block definitions for dependency tree, rule list, and file list
 */

typedef struct file_node
{
    char    *fname;
    unsigned long fdatetime;
    struct  file_node *chain;
} FILE_NODE;

typedef struct tree_node
{
    struct tree_node  *next;		/* Ptr to next dependency in list     */
    FILE_NODE         *file;		/* Ptr to filenode for target         */
    struct tree_node  *tree_list;	/* Ptr to list of dependents          */
    struct tree_node  *tree_add;	/* Ptr for adding dependents          */
    struct tree_node  *side;		/* Ptr to 2nd double-colon dependency */
    struct tree_node  *side_add;	/* Ptr for adding 2nd :: dependency   */
    char              *shell_list;	/* Ptr to cmds for this dependency    */
    char               type;		/* Type of dependency                 */
    int                level;		/* Tree level where this node was used*/
} TREE_NODE;

typedef struct rule_node
{
    struct rule_node *next;
    char *srctype;
    char *desttype;
    char *command;
} RULE_NODE;

typedef struct macro_node
{
    char    *token;		/* Name of macro                        */
    char    *value;		/* Body of macro                        */
    int     lock;		/* Lock flag to prevent re-definition   */
    int     vsize;		/* Size of body of macro                */
    struct  macro_node *nested;	/* Pointer for macro invokation list    */
    char    *position;		/* Pointer into this macro when invoked */
    struct  macro_node *next;	/* Pointer to next macro in macro list  */
} MACRO_NODE;

struct opttbl			/* Structure for options table          */
{   char op_name[11];		/* Name of option                       */
    void (*op_rout)(void);	/* Pointer to function for option       */
};

/* AMAKE global variables   */

extern char *linebuffer;	/* Buffer for each line read from Makefile  */
extern char *linepntr;		/* Pointer into Makefile buffer             */

extern FILE        *makefile;	/* FILE ptr for Makefile                    */
extern char        *xmakex;		/* New name for the makefile                */
extern TREE_NODE   *treepnt;	/* Pointer to dependency tree               */
extern TREE_NODE   *treeadd;	/* Tail pointer to dependency tree          */
extern FILE_NODE   *filepnt;	/* Pointer to list of file nodes            */
extern RULE_NODE   *rulepnt;	/* Pointer to default rule list             */
extern RULE_NODE   *ruleadd;	/* Tail pointer to default rule list        */
extern MACRO_NODE  *macpnt;	/* Pointer to macro list                    */
extern MACRO_NODE  *invoke;	/* List of currently invoked macros         */
extern int          mline;	/* Current input line number in Makefile    */

extern int OPT_print;		/* Print out macro and target defs         */
extern int OPT_noexec;		/* Print out commands, but don't exec      */
extern int OPT_precious;	/* Targets are not to be deleted           */
extern int OPT_envlock;		/* Don't allow re-def of enviroment macros */
extern int OPT_debug;		/* debug mode - Display all actions        */
extern int OPT_silent;		/* Silent mode - don't print out commands  */
extern int OPT_ignore;		/* Ignore error codes returned by commands */
extern int OPT_question;	/* Question mode.  Do nothing, set status  */
extern int OPT_cont;		/* Continue if possible.                   */
extern char *default_cmd;	/* Pointer to default command string       */

extern int CMD_silent;		/* Silent mode for this command            */
extern int CMD_ignore;		/* Ignore errors returned for this command */

/*
 * File.c
 */

char *getatom(void);		/* Function to read an atom from MAKEFILE   */
void ungetatom(char *);
char *getcmd(void);		/* Function to read a command from MAKEFILE */
char *getmacro(void);		/* Function reads a macro body frm MAKEFILE */
char readchr(void);		/* Function to read a char from MAKEFILE    */
void getinfo(FILE_NODE *);	/* Return creation date/time for file       */

/*
 * Tree.c
 */

void buildtree(void);
void depend(char *, char);
void macrodef(char *);
void defltrule(char *);
void dotcmd(char *);
void dot_default(void);
FILE_NODE *fnodemake(char *);
FILE_NODE *filesrch(char *);

/*
 * Switch.c
 */

char *switchparse(int, char **);
void chkoptn(void);
struct opttbl *srchtbl(char *, struct opttbl *, int);
char *optfile(char *);
char *getfsp(char *);
char peekchar(void);
void getcx(char *);
void skipsp(void);
void badcmd(void);

/*
 * Utility.c
 */

void prtprint(unsigned char);
void cmdprint(unsigned char *);
void showerror(void);
void syntax(void);
char *getmemory(unsigned long);
char *getmore(unsigned long, char *);
int  inchr(char *, char);
int  instr(char *, char *);
void fatal(char *, char *, int);
TREE_NODE *tsearch(char *, TREE_NODE *);
int  fcompare(char *, char *);
TREE_NODE *firstsrch(char *, TREE_NODE *);
int  inslash(char *);
void research(TREE_NODE *, TREE_NODE *, int);
int  treewalk(TREE_NODE *, int);
void makethis(TREE_NODE *);
void spaces(FILE *, int);
void showtime(long);

/*
 * FORTERUN.C
 */

void cntlcsrv(void);
void runprog(char *, char *, TREE_NODE *);
void parsecmd(char *, char *, char *, TREE_NODE *);
int  cmdcopy(char *, unsigned char *, TREE_NODE *, int, int);

/*
 * ASM functions
 */

unsigned long fctime(char *);	/* Get file creation time	*/
unsigned long cdatetime(void);	/* Get current time		*/
