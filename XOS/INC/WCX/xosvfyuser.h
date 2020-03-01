// Header file VFYUSER.H

typedef struct vudata
{   long   userlen;				// Length of the user name string
    char  *userpnt;				// Pointer to the user name string
    long   grouplen;			// Length of the group name string
    char  *grouppnt;			// Pointer to the group name string
    long   pswdlen;				// Length of the password string
    char  *pswdpnt;				// Pointer to the password string
    long   desclen;				// Length of the user description string
    char  *descpnt;				// Pointer to the user description string
    long   proglen;				// Length of the program name string
    char  *progpnt;				// Pointer to the program name string
    long   dirlen;				// Length of the home directory string
    char  *dirpnt;				// Pointer to the home directory string
    long   apvlen;				// Length of the available privileges string
    char  *apvpnt;				// Pointer to the available privileges string
    long   ipvlen;				// Length of the initial active privileges
								//   string
    char  *ipvpnt;				// Pointer to the initial active privileges
								//   string
    time_s pwddtm;				// Password expiration date/time
    time_s usrdtm;				// User expiration date/time
    long   wsavail;				// Available working set limit value
    long   tmavail;				// Available total memory limit value
    long   pmavail;				// Available protected mode memory limit value
    long   rmavail;				// Available real mode memory limit value
    long   omavail;				// Available overhead memory limit value
    long   wslimit;				// Initial working set limit value
    long   tmlimit;				// Initial total memory limit value
    long   pmlimit;				// Initial protected mode memory limit value
    long   rmlimit;				// Initial real mode memory limit value
    long   omlimit;				// Initial overhead memory limit value
    long   scavail;				// Available short-term CPU limit
    long   lcavail;				// Available long-term CPU limit
    long   pcavail;				// Available CPU priority
    long   sclimit;				// Initial short-term CPU limit
    long   lclimit;				// Initial long-term CPU limit
    long   pclimit;				// INitial CPU priority
    time_s lstglog;				// Date/time of last successful login
    time_s lstblog;				// Date/time of last failed login attempt
    long   numblog;				// Number of failed login attempts
    long   dirlist;				// User directory listing status
    long   asclen;				// Length of the available sections string
    char  *ascpnt;				// Pointer to the available sections string
    long   isclen;				// Length of the initial active sections string
    char  *iscpnt;				// Pointer to the initial active sections string
    long   xosproglen;			// Length of the XOS program string
    char  *xosprogpnt;			// Offset of the XOS program string
    long   xosdirlen;			// Length of the XOS directory string
    char  *xosdirpnt;			// Offset of the XOS directory string
    long   billinglen;			// Length of the billing information item
    char  *billingpnt;			// Offset of the billing information item
    long   crdtcardlen;			// Length of the credit card item
    char  *crdtcardpnt;			// Offset of the credit card item
    long   usridlen;			// Length of the user ID string
    char  *usridpnt;			// Offset of the user ID string
    long   mlnmlen;				// Length of the mailing name string
    char  *mlnmpnt;				// Offset of the mailing name string
    long   complen;				// Length of the company name string
    char  *comppnt;				// Offset of the company name string
    long   addr1len;			// Length of the address line 1 string
    char  *addr1pnt;			// Offset of the address line 1 string
    long   addr2len;			// Length of the address line 2 string
    char  *addr2pnt;			// Offset of the address line 2 string
    long   addr3len;			// Length of the address line 3 string
    char  *addr3pnt;			// Offset of the address line 3 string
    long   addr4len;			// Length of the address line 4 string
    char  *addr4pnt;			// Offset of the address line 4 string
    long   citylen;				// Length of the city string
    char  *citypnt;				// Offset of the city string
    long   statelen;			// Length of the state string
    char  *statepnt;			// Offset of the state string
    long   ziplen;				// Length of the ZIP-code string
    char  *zippnt;				// Offset of the ZIP-code string
    long   cntrylen;			// Length of the country string
    char  *cntrypnt;			// Offset of the country string
    long   phonelen;			// Length of the phone number string
    char  *phonepnt;			// Offset of the phone number string
    long   udfhandle;			// Handle for the UDF server device
    long   udfaddr;				// Network address for UDF server
    long   udfport;				// Network port for UDF server
    char  *error1;				// Pointer to first error message string
    char   error2[80];			// Buffer for second error message string
} VUDATA;

int verifyuser(char *username, char *access, long bits1, long bits2,
		long device, VUDATA *usrdata, char *buffer, int size);
int verifypassword(char *password, int pswdlen, char *verify, char *refdata,
		VUDATA *usrdata);
