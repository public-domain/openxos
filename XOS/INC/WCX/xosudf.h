// Define UDF record types

#define UDF_USERNAME 1		// User name
#define UDF_PASSWORD 2		// Password
#define UDF_USERDESC 3		// User description
#define UDF_PROGRAM  4		// Initial program
#define UDF_HOMEDIR  5		// Home directory
#define UDF_AVLPRIV  6		// Available privileges
#define UDF_INLPRIV  7		// Initial active privileges
#define UDF_AVLMEM   8		// Available memory limits
#define UDF_INLMEM   9		// Initial active memory limits
#define UDF_AVLCPU   10		// Available CPU limits
#define UDF_INLCPU   11		// Initial active CPU limits
#define UDF_PSWDEXP  12		// Password expiration
#define UDF_USEREXP  13		// User expiration
#define UDF_FLAGS    14		// User flags
#define UDF_HISTORY  15		// Login history
#define UDF_DIRLIST  16		// User directory listing status
#define UDF_ACCESS   17		// Access class list
#define UDF_AVLSECT  18		// Available sections
#define UDF_INLSECT  19		// Initial sections
#define UDF_ALGPROG  20		// Initial Allegro program
#define UDF_ALGDIR   21		// Allegro home directory

#define UDF_BILLING  24		// Billing data
#define UDF_CRDTCARD 25		// Credit card information

#define UDF_USERID   32		// User ID
#define UDF_MAILNAME 33		// Mailing name
#define UDF_COMPANY  34		// Company name
#define UDF_ADDR1    35		// Address - line 1
#define UDF_ADDR2    36		// Address - line 2
#define UDF_ADDR3    37		// Address - line 3
#define UDF_ADDR4    38		// Address - line 4
#define UDF_CITY     39		// City
#define UDF_STATE    40		// State
#define UDF_ZIP      41		// Postal (ZIP) code
#define UDF_COUNTRY  42		// Country
#define UDF_PHONE    43		// Phone number

// Define bits for the UDF_DIRLIST value

#define DLST_INCLUDE  0x01	// Include user in the user directory
#define DLST_USERDESC 0x02	// Include user description in directory entry
#define DLST_ADDRESS  0x04	// Include address in directory entry
#define DLST_PHONE    0x08	// Include phone number in directory entry

// Define bits for the flags field in the UDF_BILLING item

#define BF_SETBITS    0x80	// OR in low order 7 bits of value
#define BF_ADMNDTL    0x02	// Administrative billing detail report request
#define BF_USERDTL    0x01	// User billing detail report request

// Define flag bits for the user request message

#define URQ1_DIRLIST  0x80000000	// Return user directory list status
#define URQ1_HISTORY  0x40000000	// Return login history
#define URQ1_USEREXP  0x20000000	// Return user expiration date/time
#define URQ1_PSWDEXP  0x10000000	// Return password expiration date/time
#define URQ1_AVLCPU   0x08000000	// Return available CPU limits
#define URQ1_INLCPU   0x04000000	// Return initial CPU limits
#define URQ1_AVLMEM   0x02000000	// Return available memory limits
#define URQ1_INLMEM   0x01000000	// Return initial memory limits
#define URQ1_AVLPRIV  0x00800000	// Return available priviledges
#define URQ1_INLPRIV  0x00400000	// Return initial priviledges
#define URQ1_HOMEDIR  0x00100000	// Return user directory
#define URQ1_PROGRAM  0x00080000	// Return initial program specification
#define URQ1_USERDESC 0x00040000	// Return user description
#define URQ1_PASSWORD 0x00020000	// Return password
#define URQ1_USERNAME 0x00010000	// Return user name
#define URQ1_UPDHIST  0x00000800	// Update log-in history record
#define URQ1_INACTIVE 0x00000200	// Make record inactive
#define URQ1_ACTIVE   0x00000100	// Make record active
#define URQ1_DELETE   0x00000040	// Delete record
#define URQ1_CREATE   0x00000020	// Create record
#define URQ1_UPDATE   0x00000010	// Update record
#define URQ1_UNLOCK   0x00000008	// Unlock record
#define URQ1_LOCK     0x00000004	// Lock record
#define URQ1_KEYUID   0x00000001	// Key is user ID (otherwise key is user
					//   name)

#define URQ2_PHONE    0x00200000	// Return phone number
#define URQ2_COUNTRY  0x00100000	// Return country
#define URQ2_ADDRESS  0x00080000	// Return mailing address
#define URQ2_COMPANY  0x00040000	// Return company name
#define URQ2_MAILNAME 0x00020000	// Return mailing name
#define URQ2_USERID   0x00010000	// Return user ID
#define URQ2_CRDTCARD 0x00000100	// Return credit card data
#define URQ2_BILLING  0x00000080	// Return billing data
#define URQ2_ALGDIR   0x00000010	// Return Allegro home directory
#define URQ2_ALGPROG  0x00000008	// Return initial Allegro program
#define URQ2_AVLSECT  0x00000004	// Return available sections
#define URQ2_INLSECT  0x00000002	// Return initial sections
#define URQ2_ACCESS   0x00000001	// Return access permissions

// Define flag bits for the user response message

#define URS1_DIRLIST  0x80000000	// User directory list status returned
#define URS1_HISTORY  0x40000000	// Login history returned
#define URS1_USEREXP  0x20000000	// User expiration date/time returned
#define URS1_PSWDEXP  0x10000000	// Password expiration date/time
					//   returned
#define URS1_AVLCPU   0x08000000	// Available CPU limits returned
#define URS1_INLCPU   0x04000000	// Initial CPU limits returned
#define URS1_AVLMEM   0x02000000	// Available memory limits returned
#define URS1_INLMEM   0x01000000	// Initial memory limits returned
#define URS1_AVLPRIV  0x00800000	// Available priviledges returned
#define URS1_INLPRIV  0x00400000	// Initial priviledges returned
#define URS1_HOMEDIR  0x00100000	// User directory returned
#define URS1_PROGRAM  0x00080000	// Initial program specification
					//   returned
#define URS1_USERDESC 0x00040000	// User description returned
#define URS1_PASSWORD 0x00020000	// Password returned
#define URS1_USERNAME 0x00010000	// User name returned
#define URS1_UPDHIST  0x00000800	// Log-in history record updated
#define URS1_INACTIVE 0x00000200	// Record is inactive
#define URS1_ACTIVE   0x00000100	// Record is active
#define URS1_DELETE   0x00000040	// Record has been deleted
#define URS1_CREATE   0x00000020	// Record has been created
#define URS1_UPDATE   0x00000010	// Record has been updated
#define URS1_LOCKED   0x00000004	// Record is locked
#define URS1_CKDPSWD  0x00000002	// Password checked

#define URS2_PHONE    0x00200000	// Phone number returned
#define URS2_COUNTRY  0x00100000	// Country returned
#define URS2_ADDRESS  0x00080000	// Mailing address returned
#define URS2_COMPANY  0x00040000	// Company name returned
#define URS2_MAILNAME 0x00020000	// Mailing name returned
#define URS2_USERID   0x00010000	// User ID returned
#define URS2_CRDTCARD 0x00000100	// Credit card data returned
#define URS2_BILLING  0x00000080	// Billing data returned
#define URS2_ALGDIR   0x00000010	// Allegro home directory returned
#define URS2_ALGPROG  0x00000008	// Initial Allegro program returned
#define URS2_AVLSECT  0x00000004	// Available sections returned
#define URS2_INLSECT  0x00000002	// Initial sections returned
#define URS2_ACCESS   0x00000001	// Access permissions returned

// Define message sub-types (stored in second byte of message)

#define UDFM_DATAREQ 1		// Data request (sent to server, may include
				//   data records)
#define UDFM_OK      2		// Normal response (sent by server, may include
				//   data records)
#define UDFM_NOTFND  3		// Record not found response (sent by server)
#define UDFM_EXISTS  4		// Record exists response (sent by server)
#define UDFM_BADPWD  5		// Incorrect password response (sent by server)
#define UDFM_INUSE   6		// New user name or user ID is already in use
				//   (sent by server)
#define UDFM_NOINX   7		// No indexable field specified for new entry
				//   (sent by server)
#define UDFM_ERROR   8		// Message format error response (sent by
				//   server)
#define UDFM_MSGFMT  9		// IO or other kernel reported error (sent by
				//   server, includes error code only)
#define UDFM_UDFFMT  10		// UDF format error response (Sent by server)
#define UDFM_SERVER  11		// Server logic error (sent by server, includes
				//   error code only)
#define UDFM_UDFLNG  12		// UDF record is too long (sent by server)
#define UDFM_RSPLNG  13		// Response message is too long (sent by server)
#define UDFM_COMPRS  14		// UDF is being compressed (sent by server)
#define UDFM_BILACT  15		// Billing is active

#define UDFM_BILLREQ 32		// Billing request (sent to server)
#define UDFM_BILLSTP 33		// Stop billing (sent to server)
#define UDFM_BILLACK 34		// Bill user acknowledgement (sent to server)
#define UDFM_BILLUDN 35		// Bill user done (sent to server)
#define UDFM_BILLUPD 36		// Billing update (sent to server)

#define UDFM_BILLACP 48		// Billing request accepted (sent by server)
#define UDFM_BILLUSR 49		// Bill user (sent by server)
#define UDFM_BILLUDA 50		// Bill user done acknowledgement (sent by
				//   server)
#define UDFM_BILLEND 51		// Bill user end (send by server)
