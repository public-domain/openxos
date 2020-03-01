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

//-----------------------------------------------------------------
//
//  XOSXTRM.H - Kernel-mode include file for terminal class devices
//
//-----------------------------------------------------------------

// Define structure for the screen symbiont message which reports system shift
//   character

typedef struct tagssalmchar
{   uchar  func;		// Function
    uchar  xxx;			// Not used
    uchar  pnum;		// Physical screen number (primary unit number)
    uchar  snum;		// Virtual screen number (secondary unit number)
    long   xtdb;		// Exec offset of terminal TDB
    long   dspmode;		// Mode bits and value
    ushort chr;			// Character
    ushort bits;		// Keyboard state bits (low 16 bits from
} SSALMCHAR;			//   tdb_keysts0)


// Define structure for the screen symbiont message which reports virtual
//   screen status

typedef struct tagssstatus
{   uchar func;			// Function
    uchar xxx;
    uchar pnum;			// Physical screen number (primary unit number)
    uchar snum;			// Virtual screen number (secondary unit number)
    long  xtdb;			// Exec offset of terminal TDB
    long  dspmode;		// Mode bits and value
    uchar status;		// Status: 0 = Idle, 1 = In use
} SSSTATUS;

// Define structure for the screen symbiont message used to register a program
//   to handle screen switches

typedef struct tagssregprg
{   uchar func;			// Function = MT_SSREGPRG
    uchar xxx;
    uchar pnum;			// Physical screen number (primary unit number)
    uchar snum;			// Virtual screen number (secondary unit numer)
    uchar save;			// Non-zero if screen symbiont should save
} SSREGPRG;			//   screen when switching

// Define structure for the screen symbiont message sent to a registered
//   program by the screen symbiont to report an action

typedef struct tagssaction
{   uchar  func;		// Function = MT_SSACTION
    uchar  xxx;
    uchar  pnum;		// Physical screen number (primary unit number)
    uchar  snum;		// Virtual screen number (secondary unit numer)
    ushort achar;		// Character (0 means this is a registration
				//   acknowlegement)
    ushort abits;		// Keyboard state bits (low 16 bits from
} SSACTION;			//   tdb_keysts0)

// Define structure for the screen symbiont message which is sent by a
//   registered program to report an action

typedef struct tagssreport
{   uchar func;			// Function = MT_SSREPORT
    uchar xxx;
    uchar pnum;			// Physical screen number (primary unit number)
    uchar snum;			// Virtual screen number (secondary unit numer)
    uchar action;		// Action (0 means clear system shift state,
} SSREPORT;			//   non-zero is virtual screen number to
				//   switch to)
