// C definitions for the server support module
// This header requires that PROCARG.H have been previously included

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

extern char srvname[];		// Process name for the server (SYSDIS name)
extern char prgname[];		// Program name for the server (filename)

extern FILE *srvDebugStream;	// Stream for debug file output
extern char  srvDebugLevel;	// Debug level

extern char *srvLogName;	// File for log file
extern long  srvLogHandle;	// Handle for log file
extern char  srvLogLevel;	// Logging level
extern char *srvMsgDst;		// Message destination string for responses
extern long  srvUnitNum;	// Server unit number

typedef struct srvdata
{   THDDATA thddata;
} SRVDATA;

typedef struct msgstate
{   char rmt[64];
} MSGSTATE;

// Functions defined in the program that are called by the server routines

void srvFinishCmd(void);	// Called after the server command message is
				//   processed
void srvMessage(char *msg, int size);
				// Called for all non-server command messages
void srvSetup1(void);		// Called before command-line args are processed
void srvSetup2(void);		// Called just before suspend/after command-line
void srvSetupCmd(void);		// Called before server command message is
				//   processed


// Prototypes for functions defined within the server

// Function to call to initialize the server manager

long srvInitialize(char *args, SRVDATA *membase, long vectbase, long numthrds,
    long majver, long minver, long editnum);

// Function to call when unable to set up a required vector

void srvVectFail(long err);

// Function to call when unable to initialize the periodic alarm

void srvAlarmFail(long err);

// Function used in procarg dispatch tables

void srvFncCommand(arg_data *);

// Function called by procarg to process errors

void srvCmdError(char *, char *);

// Function to return an error response to the SERVER program
// First byte of msg is a magic value that is specified in XOSMSG.H

void srvCmdErrorResp(long code, char *msg1, char *msg2, char *dst);
void srvCmdErrorRespLog(long code, char *msg1, char *msg2, char *dst);

// Function to return a response to the SERVER program
// First byte of msg is a magic value that is specified in XOSMSG.H

void srvCmdResponse(char *msg, char *dst);

// Function to return a response to the SERVER program and to write the
//   response to the system log
// First byte of msg is a magic value that is specified in XOSMSG.H

void srvCmdResponseLog(char *msg, char *dst);

// Function to write a message to the system log

void srvLogSysLog(long code, char *msg);

// Function to call to report a fatal set up error

void srvSetupFail(long code, char *msg1, char *msg2);
