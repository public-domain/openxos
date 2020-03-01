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
#define URQ1_CHKPSWD  0x00000002	// Check password
#define URQ1_KEYUID   0x00000001	// Key is user ID (otherwise key is user
					//   name)

#define URQ2_PHONE    0x00200000	// Return phone number
#define URQ2_COUNTRY  0x00100000	// Return country
#define URQ2_ADDRESS  0x00080000	// Return mailing address
#define URQ2_COMPANY  0x00040000	// Return company name
#define URQ2_MAILNAME 0x00020000	// Return mailing name
#define URQ2_USERID   0x00010000	// Return user ID

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

// Define message sub-types (stored in second byte of message)

#define UDFM_REQ    1		// Request (sent to server, may include data
				//   records)
#define UDFM_OK     2		// Normal response (sent by server, may
				//   include data records)
#define UDFM_NOTFND 3		// Record not found response (sent by server)
#define UDFM_EXISTS 4		// Record exists response (sent by server)
#define UDFM_BADPWD 5		// Incorrect password response (sent by server)
#define UDFM_INUSE  6		// New user name or user ID is already in use
				//   (sent by server)
#define UDFM_NOINX  7		// No indexable field specified for new entry
				//   (sent by server)
#define UDFM_ERROR  8		// Message format error response (sent by
				//   server)
#define UDFM_MSGFMT 9		// IO or other kernel reported error (sent by
				//   server, includes error code only)
#define UDFM_UDFFMT 10		// UDF format error response (Sent by server)
#define UDFM_SERVER 11		// Server logic error (sent by server, includes
				//   error code only)
