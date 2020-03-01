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

//Define accounting file record types

#define ACTR_LOGIN    1		// Login record
#define ACTR_LOGOUT   2		// Logout record
#define ACTR_FAIL     3		// Login failure
#define ACTR_BILLING  4		// Billing information

#define ACTR_CRDTCARD 32	// Credit card transaction (audit only)
#define ACTR_CREDIT   33	// Credit
#define ACTR_DEBIT    34	// Debit

// Define accounting record item types

#define ACTI_SESSION    1	// Session ID
#define ACTI_BGNDTTM    2	// Beginning date and time
#define ACTI_ENDDTTM    3	// Ending date and time
#define ACTI_USERNAME   4	// User name
#define ACTI_USERADDR   5	// User address
#define ACTI_PORTNAME   6	// Port name
#define ACTI_COMMADDR   7	// Comm processor address
#define ACTI_HOSTADDR   8	// Host address
#define ACTI_ACCESS     9	// Access class name
#define ACTI_NUMBER     10	// Number
#define ACTI_BILLSTATUS 11	// Billing status
#define ACTI_BILLUSAGE  12	// Billing usage amounts
#define ACTI_BILLEXTRA  13	// Billing extra amounts
#define ACTI_BILLCREDIT 14	// Billing credit
#define ACTI_BILLADJUST 15	// Billing adjustment
#define ACTI_SERVICE    32	// Service name
#define ACTI_CCCHARGE   33	// Credit card charge (audit only)
#define ACTI_CCAPPROVAL 34	// Credit card approval code
#define ACTI_CCNAME     35	// Name (first character is first name initial,
				//   remainder is last name)
#define ACTI_CCSTREET   36	// Street address
#define ACTI_CCCITY     37	// City
#define ACTI_CCSTATE    38	// State (2 characters)
#define ACTI_CCZIPCODE  39	// Zip code (5 or 9 characters)
