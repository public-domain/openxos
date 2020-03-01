//===================================
// PCCDISK - DISK routines for PCCARD
// Written by John Goltz
//===================================

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
/// #include <XOSTIME.H>
#include <ERRMSG.H>
#include <XOSSVC.H>
#include <XOSDISK.H>
/// #include <XOSTRM.H>
/// #include <XOSACCT.H>
#include <XOSERR.H>
#include <XOSPROCARG.H>
#include <XOSTHREADS.H>
#include <XOSSERVERT.H>
#include "PCCARD.H"


static void devicediskhdka(SOCBLK *soc);
static void removedisk(IDB *idb, SOCBLK *soc);
static void setupdiskdev(IDB *idb, SOCBLK *soc, TYPEBLK *dev, CFGENT *cfgent,
    int iosize1, int ioreg2, int iosize2);


//*****************************************************************
// Function: devicedisk - Process the DEVICE command for DISK class
// Returned: Nothing
//*****************************************************************

void devicedisk(
    IDB    *idb,
    SOCBLK *soc)

{
    if (strcmp(type, "HDKA") == 0)
	devicediskhdka(soc);
    else

    {
	sprintf(bufr, "Device type %s is not supported for class DISK", type);
	srvCmdErrorResp(0, bufr, NULL, srvMsgDst);
    }
    if (soc->curtype == 0)
	pccsetup(idb, soc);
}

//******************************************************
// Function: devicediskhdka - Process the DEVICE command
//		for HDKA type for DISK class
// Returned: Nothing
//******************************************************

static void devicediskhdka(
    SOCBLK *soc)

{
    TYPEBLK *typeblk;

    if (index < 0)
    {
	srvCmdErrorResp(0, "INDEX value not specified", NULL, srvMsgDst);
	return;
    }
    if (unit < 0)
    {
	srvCmdErrorResp(0, "UNIT value not specified", NULL, srvMsgDst);
	return;
    }
    if (ioreg < 0)
    {
	srvCmdErrorResp(0, "IOREG value not specified", NULL, srvMsgDst);
	return;
    }
    if (intreq < 0)
    {
	srvCmdErrorResp(0, "INT value not specified", NULL, srvMsgDst);
	return;
    }
    if ((typeblk = findtype(TYPE_DISK_HDKA, soc)) == NULL)
    {
	if ((typeblk = (TYPEBLK *)getmem(sizeof(TYPEBLK))) == NULL)
	    return;

	typeblk->type = TYPE_DISK_HDKA;
	typeblk->remove = removedisk;
	typeblk->next = soc->firsttype;
	soc->firsttype = typeblk;
    }
    sprintf(typeblk->name, "D%d:", unit);
    typeblk->index = index;
    typeblk->unit = unit;
    strcpy(typeblk->dosnames, dosnames);
    typeblk->ioreg = ioreg;
    typeblk->intreq = intreq;
    typeblk->cfgreg = typeblk->cfgval = 0;
    sprintf(bufr, "xPCCARD: Device associated with PC-card %s%d\n"
	    "        Class=DISK  Type=HDKA  Unit=%d  Name=%s\n"
	    "            IOreg=0x%04.4X  Int=%d\n            DOSnames=%s",
	    pccardname, pccardsoc, unit, typeblk->name, ioreg, intreq,
	    dosnames);
    bufr[0] = MT_FINALMSG;
    srvCmdResponse(bufr, srvMsgDst);
}

//****************************************************
// Function: setupdisk - Set up a disk PC-card for use
// Returned: Nothing
//****************************************************

// This function first determines if the disk PC-card has a configuration
//   which is compatable with the associated DISK/HDKA device and sets up
//   the PC-card controller if so.  If the associated device does not exist,
//   it is created.

void setupdisk(
    IDB    *idb,
    SOCBLK *soc)

{
    TYPEBLK *dev;
    CFGENT  *cpnt;
    ADRENT  *apnt;
    int      addr;
    int      length;
    int      ccnt;
    int      acnt;

    if ((dev = findtype(TYPE_DISK_HDKA, soc)) == NULL)
    {					// Find the associated disk device
	soc->curtype = 0;		// If none, say no card!
	if (srvDebugLevel & DEBUG_DEVICE)
	{
	    sprintf(bufr, "No associated DISK/HDKA device for disk card for "
		    "%s%d", idb->pccname, soc->num);
	    logstring(bufr);
	}
	return;
    }

    // First scan the configurations to see if we have an exact match for the
    //   specified base IO register (ATA disks normally specify two IO ranges.
    //   We assume they are for the primary and secondary IDE controller
    //   configurations and check for a match on either range, since we're not
    //   sure which is which.  We assume the device will not specify the funny
    //   second address!)

    ccnt = cfg.numcfgs;
    cpnt = cfg.cfgtbl;
    while (--ccnt >= 0)
    {
	acnt = cpnt->numadrs;
        apnt = cpnt->adrtbl;
	while (--acnt >= 0)
	{
	    if (apnt->addr == dev->ioreg)
	    {
		if (cpnt->numadrs > 1)
		{
		    if (apnt == cpnt->adrtbl)
		    {
			addr = cpnt->adrtbl[1].addr;
			length = cpnt->adrtbl[1].length;
		    }
		    else
		    {
			addr = cpnt->adrtbl[0].addr;
			length = cpnt->adrtbl[0].length;
		    }
		}
		else
		    addr = length = 0;
		setupdiskdev(idb, soc, dev, cpnt, apnt->length, addr, length);
		return;
	    }
	    apnt++;
	}
	cpnt++;
    }

    // If get here we did not find an exact address match - now see if
    //   we have a configuration with partial decoding that we can use

    ccnt = cfg.numcfgs;
    cpnt = cfg.cfgtbl;;
    while (--ccnt >= 0)
    {
	if (cpnt->decode >= 4 && cpnt->decode <= 6)
	{
	    length = 0x01 << cpnt->decode;
	    if (dev->ioreg % length == 0)
	    {
		setupdiskdev(idb, soc, dev, cpnt, length, 0, 0);
		return;
	    }
	}
	cpnt++;
    }

    // If get here we could not find a usable configuration

    soc->curtype = 0;
}

//************************************************************
// Function: setupdiskdev - Low level set up for a disk device
// Returned: Nothing
//************************************************************

static void setupdiskdev(
    IDB     *idb,
    SOCBLK  *soc,
    TYPEBLK *dev,
    CFGENT  *cfgent,
    int      iosize1,
    int      ioreg2,
    int      iosize2)

{
    static struct
    {   text8_parm class;
	uchar      end;
    } spcparms =
    {	{PAR_SET|REP_TEXT, 8, IOPAR_CLASS, "DISK"}
    };
    static char classname[] = "DISK:";
    static struct
    {	byte4_char unit;
	text4_char type;
	byte4_char index;
	byte4_char ioreg;
	byte4_char intreq;
	char       end;
    } addchrs =
    {	{PAR_SET|REP_DECV, 4, "UNIT"},
	{PAR_SET|REP_TEXT, 4, "TYPE" , "HDKA"},
	{PAR_SET|REP_DECV, 4, "INDEX"},
	{PAR_SET|REP_DECV, 4, "IOREG"},
	{PAR_SET|REP_DECV, 4, "INT"}
    };
    static type_qab addqab =
    {	QFNC_WAIT|QFNC_CLASSFUNC,//qab_func     - Function
	0,			// qab_status   - Returned status
	0,			// qab_error    - Error code
	0,			// qab_amount   - Amount
	0,			// qab_handle   - Device handle
	0,			// qab_vector   - Vector for interrupt
	0,	        	// qab_level    - Signal level for direct I/O
	0,  			// qab_prvlevel - Previous signal level (int.)
	CF_ADDUNIT,		// qab_option   - Options or command
	0,			// qab_count    - Count
	classname, 0,		// qab_buffer1  - Pointer to file spec
	&addchrs, 0,		// qab_buffer2  - Unused
	NULL, 0			// qab_parm     - Pointer to parameter list
    };
    static struct
    {	text16_char pccard;
	text4_char  ready;
	char        end;
    } readychrs =
    {	{PAR_SET|REP_TEXT, 16, "PCCARD", "xxx"},
	{PAR_SET|REP_TEXT,  4, "READY" , "YES"}
    };

    static struct
    {	text4_char dosname;
	char       end;
    } doschrs =
    {	{PAR_SET|REP_TEXT, 4, "DOSNAMEx", "x"}
    };

    char *pnt;
    int   rtn;
    int   dskhndl;
    int   unit;
    char  chr;

    // First set up the PC-card controller

    if (!setupiocard(idb, soc->num, cfgent, dev->ioreg, iosize1, ioreg2,
	    iosize2, dev->intreq))
	return;

    // Second, set up the disk device - first try to open it to see if it
    //   already exists

    if (srvDebugLevel & DEBUG_DEVICE)
    {
	sprintf(bufr, "Disk set up: Unit=%d, Index=%d, IOreg=0x%X, Int=%d",
		dev->unit, dev->index, dev->ioreg, dev->intreq);
	logstring(bufr);
    }
    if ((dskhndl = svcIoOpen(O_RAW|O_NOMOUNT, dev->name, NULL)) == ER_NSDEV)
    {					// No such device - try to add the
	addchrs.unit.value = dev->unit;	//   device
	addchrs.index.value = dev->index;
	addchrs.ioreg.value = dev->ioreg;
	addchrs.intreq.value = dev->intreq;
	if ((rtn = svcIoQueue(&addqab)) < 0 || (rtn = addqab.qab_error) < 0)
	{
	    sprintf(bufr, "Error adding disk unit %d for PC-card controller %s",
		    dev->unit, idb->pccname);
	    fail(idb, rtn, bufr);
	    return;
	}
	dskhndl = svcIoOpen(O_RAW|O_NOMOUNT, dev->name, NULL);
    }					// Try to open the disk again






    if (dskhndl > 0)			// Does disk exist?
    {					// Yes - dismount it (need do this in
					//   case someone tried to access the
					//   disk while it was missing and it
					//   was mounted as non-file structured)
	if ((rtn = svcIoSpecial(dskhndl, DSF_DISMOUNT, NULL, 0, &spcparms)) < 0)
	{
	    sprintf(bufr, "Error dismounting disk unit %d for PC-card "
		    "controller %s", dev->unit, idb->pccname);
	    fail(idb, rtn, bufr);
	    return;
	}
    }
    else				// Else if had an unexpected error
    {
	sprintf(bufr, "Error opening disk unit %d for PC-card controller %s",
		dev->unit, idb->pccname);
	fail(idb, dskhndl, bufr);
	return;
    }

    // Here with the disk device open - now associate the disk with the PC-card
    //   controller and clear it's not ready state

    sprintf(readychrs.pccard.value, "%s%d", idb->pccname, soc->num);
    if ((rtn = svcIoDevChar(dskhndl, &readychrs)) < 0)
    {
	sprintf(bufr, "Error clearing not-ready state for disk unit %d for PC-card "
		"controller %s", dev->unit, idb->pccname);
	fail(idb, rtn, bufr);
    }

    // Set up any DOS names specified for the disk

    pnt = dev->dosnames;
    unit = '1';
    while ((chr = *pnt++) != 0)
    {
	doschrs.dosname.name[7] = unit++;
	doschrs.dosname.value[0] = chr;
	if ((rtn = svcIoDevChar(dskhndl, &doschrs)) < 0)
	{
	    sprintf(bufr, "Error setting DOS name for disk unit %d for PC-card "
		    "controller %s", dev->unit, idb->pccname);
	    fail(idb, rtn, bufr);
	}
    }
    svcIoClose(dskhndl, 0);

    // Now access the disk again to mount it

    if ((svcIoDevParm(O_PHYS, dev->name, NULL)) < 0)
    {
	sprintf(bufr, "Error mounting disk unit %d for PC-card controller %s",
		dev->unit, idb->pccname);
	fail(idb, dskhndl, bufr);
    }
    soc->curtype = dev;
}

//**************************************************
// Function: removedisk - Remove a DISK class device
// Returned: Nothing
//**************************************************

static void removedisk(
    IDB    *idb,
    SOCBLK *soc)

{
    static struct 
    {	text8_parm class;
	char       end;
    } diskparm =
    {	{PAR_SET|REP_TEXT, 8, IOPAR_CLASS, "DISK"},
    };

    static type_qab dskqab =
    {	QFNC_WAIT|QFNC_SPECIAL,	// qab_open
	0,			// qab_status
	0,			// qab_error
	0,			// qab_amount
	0,			// qab_handle
	0,			// qab_vector
	0,			// qab_level
	0,			// qab_prvlevel
	DSF_DISMOUNT,		// qab_option
	0,			// qab_count
	NULL, 0,		// qab_buffer1
	NULL, 0,		// qab_buffer2
	&diskparm, 0		// qab_parm
    };

    long     rtn;
    TYPEBLK *dev;

    dev = soc->curtype;
    if ((dskqab.qab_handle = svcIoOpen(O_RAW|O_NOMOUNT, dev->name, NULL)) < 0)
    {
	sprintf(bufr, "Error opening disk for dismount, disk unit %d for "
		"PC-card controller %s", dev->unit, idb->pccname);
	fail(idb, dskqab.qab_handle, bufr);
    }
    if ((rtn = svcIoQueue(&dskqab)) < 0 || (rtn = dskqab.qab_error) < 0)
    {
	sprintf(bufr, "Error dismounting disk unit %d for PC-card controller "
		"%s", dev->unit, idb->pccname);
	fail(idb, rtn, bufr);
    }
    svcIoClose(dskqab.qab_handle, 0);
    if (srvDebugLevel & DEBUG_DEVICE)
    {
	sprintf(bufr, "Disk unit %d (%s) dismounted for %s", dev->unit,
		dev->name, idb->pccname);
	logstring(bufr);
    }
}
