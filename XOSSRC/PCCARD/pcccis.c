//=================================
// PCCCIS - CIS routines for PCCARD
// Written by John Goltz
//=================================

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

#include <STDIO.H>
#include <STDDEF.H>
#include <STDLIB.H>
#include <STRING.H>
#include <SIGNAL.H>
#include <CTYPE.H>
#include <XOS.H>
#include <XOSSTR.H>
#include <XOSTIME.H>
#include <ERRMSG.H>
#include <XOSSVC.H>
#include <XOSTRM.H>
#include <XOSACCT.H>
#include <XOSERR.H>
#include <XOSPROCARG.H>
#include <XOSTHREADS.H>
#include <XOSSERVERT.H>
#include "PCCARD.H"

static IDB   *cisidb;
static int    ciscnt;
static uchar *cispnt;


static int  getbyte(void);
static int  getvalue(int len);
static int  scancfgtbl(void);
static int  scanconfig(void);
static int  scaniospace(void);
static int  scanfuncid(void);
static int  skippower(void);
static int  skippowval(void);
static int  skiptiming(void);
static void truncated(void);


//*******************************************************************
// Function: getconfig - Get configuration information from a PC-card
// Returned: TRUE if normal, FALSE if error
//*******************************************************************

// This function reads the CIS data from the PC-card and decodes it to
//   obtain the configuration information.  The information obtained
//   consists of up to 5 configurations, with each configuration
//   specifying a configuration register value and up to two sets of
//   IO register or memory addresses.  The device type of the card and
//   the offset of the configuration register in CIS space is also returned.
//   For a PC-card to be usable, it must specify the configuration register
//   offset, be of the a generic type that a device specification has been
//   provided for and and and have at least one address range that is
//   compatable with the specified device configuration.  Note that we do
//   not worry about the interrupt specification given by the CIS data since
//   the PC-card controller allows us to use any interrupt for any card.

// NOTE: This function is NOT reentrent!  It counts on the fact that it
//   will only be executed from signal level and cannot be interrupt!  This
//   allows the use of static data areas, which considerablly reduces the
//   amount of argument passing needed.

int getconfig(
    IDB *idb,
    int  socket)

{
    uchar *pnt;
    int    cnt;
    int    rtn;
    int    type;
    int    size;
    uchar  cisdata[1024];

    static struct
    {	byte1_char  socket;
	lngstr_char cisdata;
	char        end;
    } chrlist =
    {	{PAR_SET|REP_DECV , 1, "SOCKET" , 0},
	{PAR_GET|REP_DATAS, 0, "CISDATA", NULL, 1024, 1024}
    };

    cisidb = idb;
    memset(&cfg, 0, sizeof(CFG));
    chrlist.socket.value = socket;
    chrlist.cisdata.buffer = cisdata;
    if ((rtn = svcIoDevChar(idb->pcchndl, &chrlist)) < 0)
    {
	if (rtn == ER_PDNAV)		// Card not present?
	    return (TRUE);		// Yes - give good return
	fail(idb, rtn, "Error getting PC-card CIS data"); // No - log the error
	return (FALSE);			// Give error return
    }
    pnt = cisdata;
    cnt = chrlist.cisdata.strlen;
    do
    {
	if (--cnt < 0)
	{
	    truncated();
	    return (FALSE);
	}
	if ((type = *pnt++) == 0xFF)
	{
	    if (srvDebugLevel & DEBUG_TUPLES)
	        logstring("END tuple");
	    break;
	}
	if (--cnt < 0)
	{
	    truncated();
	    return (FALSE);
	}
	if ((size = *pnt++) > 0)
	{
	    if ((cnt -= size) < 0)
	    {
		truncated();
		return (FALSE);
	    }
	    cispnt = pnt;
	    ciscnt = size;
	    switch (type)
	    {
	     case 0x1A:			// CONFIG tuple
		scanconfig();
		break;

	     case 0x1B:			// CFTABLE_ENTRY tuple
		scancfgtbl();
		break;

	     case 0x21:			// FUNCID tuple
		scanfuncid();
		break;
	    }
	    pnt += size;
	}
    } while (cnt > 0);

    if (srvDebugLevel & DEBUG_CONFIG)
    {
	int     acnt;
	ADRENT *apnt;

	sprintf(bufr, "*** Start of configuration dump\nCfg reg offset="
		"0x%02.2X, card type=%d\n  Found %d configs:", cfg.cfgoffset,
		cfg.cardtype,  cfg.numcfgs);
	logstring(bufr);
	curcfg = cfg.cfgtbl;
	cnt = cfg.numcfgs;
	do
	{
	    acnt = curcfg->numadrs;
	    sprintf(bufr, "  Cfg #%d:\n    Cfg val=0x%02.2X, width="
		    "0x%02.2X, decode=%d\n    Have %d address ranges",
		    curcfg - cfg.cfgtbl, curcfg->cfgval, curcfg->width,
		    curcfg->decode, acnt);
	    logstring(bufr);

	    apnt = curcfg->adrtbl;
	    while (--acnt >= 0)
	    {
		sprintf(bufr, "    Addr=0x%04.4X, length=%d", apnt->addr,
			apnt->length);
		logstring(bufr);

		apnt++;
	    }
	    curcfg++;
	} while (--cnt > 0);
	logstring("*** End of configuration dump");
    }
    return (TRUE);
}

//*******************************************************
// Function: scanconfig - Process the CONFIG (0x1A) tuple
// Returned: TRUE if normal, FALSE if error
//*******************************************************

// This tuple provides general configuration information.  The only thing
//   we care about here is the address of the configuration register.

static int scanconfig(void)

{
    int   val;
    int   value;
    int   radrbytes;
    char *pnt;

    if ((val = getbyte()) < 0)
	return (FALSE);
    radrbytes = val & 0x03;
    if ((val = getbyte()) < 0)
	return (FALSE);
    pnt = (char *)&value;
    value = 0;
    do
    {
	if ((val = getbyte()) < 0)
	    return (FALSE);
	*pnt++ = val;
    } while (--radrbytes >= 0);
    cfg.cfgoffset = value;

    if (srvDebugLevel & DEBUG_TUPLES)
    {
	sprintf(bufr, "Cfg offset=0x%X", value);
	logstring(bufr);
    }
    return (TRUE);
}

//**************************************************************
// Function: scancfgtbl - Process the CFTABLE_ENTRY (0x1B) tuple
// Returned: TRUE if normal, FALSE if error
//**************************************************************

// This tuple provides information about one configuration.  We are only
//   interested in the configuratin register value, the card type (IO or
//   memory), and the IO register specification (including the number of
//   address lines decoded.  Note that there may be more than one entry
//   for each configuration register value that differ only in items that
//   we don't care about (like voltage values).

static int scancfgtbl(void)

{
    int   val;
    int   cnt;
    int   cfgval;
    int   options;
    uchar dfltvals;

    // First byte is always there and specifies if the second byte is there

    if ((val = getbyte()) < 0)
	return (FALSE);
    cfgval = val & 0x3F;		// Get CIS offset for the configuration
					//   register
    curcfg = cfg.cfgtbl;		     // See if we already know about
    cnt = sizeof(cfg.cfgtbl)/sizeof(CFGENT); //   this configuration
    do
    {
	if (curcfg->cfgval == 0 || curcfg->cfgval == cfgval)
	    break;
	curcfg++;
    } while (--cnt > 0);

    if (cnt <= 0)			// To many configurations?
	return (TRUE);			// Yes - just quietly ignore this one!

    dfltvals = FALSE;			// Assume not setting default values
    if (val & 0x40)			// Right?
    {
	dfltvals = TRUE;		// No - clear current defaults
	memset(&dftcfg, 0, sizeof(CFGENT));
    }
    *curcfg = dftcfg;
    curcfg->cfgval = cfgval;		// Store the CIS offset of the
					//   configuration register

    if (srvDebugLevel & DEBUG_CONFIG)
    {
	sprintf(bufr, "Cfg #%d: cfg reg=0x%X", curcfg - cfg.cfgtbl, cfgval);
	logstring(bufr);
    }
    if (val & 0x80)			// Is the second byte there?
    {
	if ((val = getbyte()) < 0)	// Yes - skip it
	    return (FALSE);
    }

    // "Third" byte is always there and specifies which of the following
    //   items are there

    if ((options = getbyte()) < 0)
	return (FALSE);
    switch (options & 0x03)		// Dispatch on power options
    {
     case 0x03:				// Vcc, Vpp1, and Vpp2 descriptions
	skippower();

     case 0x02:				// Vcc and Vpp descriptions
	skippower();

     case 0x01:				// Vcc description only
	skippower();
	break;
    }
    if (options & 0x04)			// Have timing description?
	skiptiming();			// Yes
    if (options & 0x08)			// Have IO space description?
	scaniospace();			// Yes
    if (dfltvals)
	dftcfg = *curcfg;
    return (TRUE);
}

//***********************************************************************
// Function: scaniospace - Process I/O space data for CFTABLE_ENTRY tuple
// Returned: TRUE if normal, FALSE if error
//***********************************************************************

static int scaniospace(void)

{
    int     val;
    int     rcnt;
    int     acnt;
    int     adrsize;
    int     lensize;
    int     address;
    int     length;
    ADRENT *apnt;

    if ((val = getbyte()) < 0)
	return (FALSE);
    cfg.numcfgs++;
    curcfg->type = 1;
    curcfg->decode = val & 0x1F;	// Get number of address lines decoded
    curcfg->width = val & 0x60;		// Get bus width code
    if (srvDebugLevel & DEBUG_TUPLES)
    {
	sprintf(bufr, "  IO def: decode=%d, width=%d", curcfg->decode,
		curcfg->width);
	logstring(bufr);
    }

    // This overrides all previous (default) IO space values, so we clear the
    //   address range configurations here

    curcfg->numadrs = 0;
    memset(curcfg->adrtbl, 0, sizeof(curcfg->adrtbl));
    if (val & 0x80)			// Have range bytes following?
    {
	if ((val = getbyte()) < 0)	// Yes
	    return (FALSE);
	rcnt = val & 0x0F;		// Get number of ranges present
	adrsize = (val >> 4) & 0x03;	// Get size of the address value
	lensize = val >> 6;		// Get size of the length value
	do
	{
	    if ((address = getvalue(adrsize)) < 0) // Get address value
		return (FALSE);
	    if ((length = getvalue(lensize)) < 0) // Get length value
		return (FALSE);
	    apnt = curcfg->adrtbl;
	    acnt = sizeof(curcfg->adrtbl)/sizeof(ADRENT);
	    do
	    {
		if (apnt->addr == 0)	// Unused entry?
		    break;		// Yes - use it
		if (apnt->addr == address && apnt->length == length)
		{			// Duplicate entry?
	    	    if (srvDebugLevel & DEBUG_TUPLES)
			logstring("Duplicate address item");
		    return (TRUE);	// Yes - ignore it
		}
		apnt++;
	    } while (--acnt > 0);
	    if (acnt <= 0)		// Too many address ranges?
	    {				// Yes - just quietly skip it!
		if (srvDebugLevel & DEBUG_TUPLES)
		    logstring("Too many addresses!");
		return (TRUE);
	    }
	    apnt->addr = address;	// No - store the new entry
	    apnt->length = length + 1;
	    if (srvDebugLevel & DEBUG_TUPLES)
	    {
		sprintf(bufr, "    Addr range #%d: addr=0x%X, length=%d",
			curcfg->numadrs, address, length + 1);
		logstring(bufr);
	    }
	    curcfg->numadrs++;		// Count the new entry

	} while (--rcnt >= 0);
    }
    return (TRUE);
}

//**************************************************************
// Function: skippower - Skip power data for CFTABLE_ENTRY tuple
// Returned: TRUE if normal, FALSE if error
//**************************************************************

static int skippower(void)

{
    int bits;

    if ((bits = getbyte()) < 0)
	return (FALSE);
    if (bits & 0x01 && !skippowval())
	return (FALSE);
    if (bits & 0x02 && !skippowval())
	return (FALSE);
    if (bits & 0x04 && !skippowval())
	return (FALSE);
    if (bits & 0x08 && !skippowval())
	return (FALSE);
    if (bits & 0x10 && !skippowval())
	return (FALSE);
    if (bits & 0x20 && !skippowval())
	return (FALSE);
    if (bits & 0x40 && !skippowval())
	return (FALSE);
    return (TRUE);
}

//*********************************************************************
// Function: skippowval - Skip power data value for CFTABLE_ENTRY tuple
// Returned: TRUE if normal, FALSE if error
//*********************************************************************

static int skippowval(void)

{
    int val;

    if ((val = getbyte()) < 0)
	return (FALSE);
    while (val & 0x80)			// Need another value byte?
    {
	if ((val = getbyte()) < 0)
	    return (FALSE);
    }
    return (TRUE);
}

//****************************************************************
// Function: skiptiming - Skip timing data for CFTABLE_ENTRY tuple
// Returned: TRUE if normal, FALSE if error
//****************************************************************

static int skiptiming(void)

{
    // ???????

    return (TRUE);
}

//************************************************
// Function: scanfuncid - Process the FUNCID tuple
// Returned: Nothing
//************************************************

static int scanfuncid(void)

{
    int val;

    if ((val = getbyte()) < 0)
	return (FALSE);
    cfg.cardtype = val;
    return (TRUE);
}

//*********************************************************
// Function: getvalue - Get variable length value for tuple
// Returned: Value (0 extended) or -1 if at end of tuple
//*********************************************************

static int getvalue(
    int len)

{
    int rtn;
    int tmp;

    switch (len)
    {
     case 1:
	return (getbyte());

     case 2:
	if ((rtn = getbyte()) < 0)
	    return (-1);
	if ((tmp = getbyte()) < 0)
	    return (-1);
	return ((tmp << 8) + rtn);

     case 3:
	if ((rtn = getbyte()) < 0)
	    return (-1);
	if ((tmp = getbyte()) < 0)
	    return (-1);
	rtn += (tmp << 8);
	if ((tmp = getbyte()) < 0)
	    return (-1);
	rtn += (tmp << 16);
	if ((tmp = getbyte()) < 0)
	    return (-1);
	return ((tmp << 24) + rtn);
    }
    return (0);
}

//*******************************************************************
// Function: getbyte - Get next byte for tuple
// Returned: Value of next byte (0 extended) or -1 if at end of tuple
//*******************************************************************

static int getbyte(void)

{
    if (--ciscnt >= 0)
	return (*cispnt++);
    fail(cisidb, 0, "Tuple data truncated");
    return (-1);
}

//*******************************************************
// Function: truncated - Report error for truncated tuple
// Returned: Never returns
//*******************************************************

static void truncated(void)

{
    fail(cisidb, 0, "Data tuple truncated");
    exit(1);
}
