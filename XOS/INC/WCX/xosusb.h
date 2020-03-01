// Define class specific USB IO parameters (all parameters are settable only)

#define IOPAR_USB_ADDRESS  0x8001	// Specify address
#define IOPAR_USB_ENDPNT   0x8002	// Specify endpoint
#define IOPAR_USB_PKTSIZE  0x8003	// Packet size
#define IOPAR_USB_XFERTYPE 0x8004	// Transfer type
#define IOPAR_USB_SETUP    0x8005	// Setup token contents
#define IOPAR_USB_PKTRATE  0x8006	// Packet rate
#define IOPAR_USB_INPSIZE  0x8007	// Total input transaction size

// Define buffer format for the IOPAR_USB_SETUP IO parameter

typedef struct
{	uchar  reqtype;
	uchar  request;
	ushort value;
	ushort index;
	ushort length;
} SETUP;

// Define values for the IOPAR_USB_XFERTYPE IO parameter

#define USB_XFERTYPE_CNTRL 1	// Control transfer
#define USB_XFERTYPE_INT   2	// Interrupt transfer
#define USB_XFERTYPE_BULK  3	// Bulk transfer
#define USB_XFERTYPE_ISOCH 4	// Isochronous transfer (not supported yet)

// Define USB special device functions

#define SDF_USB_START       0	// Start controller
#define SDF_USB_STOP        1	// Stop controller
#define SDF_USB_STATUS      2	// Request status
#define SDF_USB_RESETPORT   3	// Reset port
#define SDF_USB_ENABLEPORT  4	// Enable port
#define SDF_USB_DISABLEPORT 5	// Disable port
#define SDF_USB_BGNINTINPUT 6	// Begin interrupt input
#define SDF_USB_ENDINTINPUT 7	// End interrupt input
