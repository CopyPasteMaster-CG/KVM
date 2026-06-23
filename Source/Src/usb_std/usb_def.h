/*
 *********************************************************************************
 *     Copyright (c) 2014   ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : usb_def.h
 * Purpose     :
 * Author      :
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_DEF_H
#define __USB_DEF_H

#define _ValBit(VAR,POS)	(VAR & (1 << POS))
#define _SetBit(VAR,POS)	(VAR |= (1 << POS))
#define _ClrBit(VAR,POS)	(VAR &= ((1 << POS)^255))

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef enum _RECIPIENT_TYPE
{
	DEVICE_RECIPIENT,     /*0x00 Recipient device */
	INTERFACE_RECIPIENT,  /*0x01 Recipient interface */
	ENDPOINT_RECIPIENT,   /*0x02 Recipient endpoint */
	OTHER_RECIPIENT       /*0x03*/
} RECIPIENT_TYPE;

typedef enum _STRING_TYPE
{
	STRING_LANG_ID,
	STRING_VENDOR_ID,
	STRING_PRODUCT_ID,
	STRING_SERIAL_ID
} STRING_TYPE;

typedef enum _STANDARD_REQUESTS
{
	GET_STATUS = 0,
	CLEAR_FEATURE,    //0x01
	RESERVED1,        //0x02
	SET_FEATURE,      //0x03
	RESERVED2,        //0x04
	SET_ADDRESS,      //0x05
	GET_DESCRIPTOR,   //0x06
	SET_DESCRIPTOR,   //0x07
	GET_CONFIGURATION,//0x08
	SET_CONFIGURATION,//0x09
	GET_INTERFACE,    //0x0a
	SET_INTERFACE,    //0x0b
	TOTAL_REQUEST,  /* Total number of Standard request */
	SYNCH_FRAME = 12
} STANDARD_REQUESTS;

typedef enum _CLASS_REQUESTS
{
	GET_REPORT = 0x01,
	GET_IDLE,
	GET_PROTOCOL,
	SET_REPORT = 0x09,
	SET_IDLE,
	SET_PROTOCOL,
} CLASS_REQUESTS;

/* Definition of "USBwValue" */
typedef enum _DESCRIPTOR_TYPE
{
	DEVICE_DESCRIPTOR = 1,
	CONFIG_DESCRIPTOR,
	STRING_DESCRIPTOR,
	INTERFACE_DESCRIPTOR,
	ENDPOINT_DESCRIPTOR,
	DEVICE_QUALIFIER_DESCRIPTOR,
	HID_DESCRIPTOR=0x21,
	HID_REPORT=0x22 
} DESCRIPTOR_TYPE;

/* Feature selector of a SET_FEATURE or CLEAR_FEATURE */
typedef enum _FEATURE_SELECTOR
{
	ENDPOINT_STALL,
	DEVICE_REMOTE_WAKEUP
} FEATURE_SELECTOR;

typedef union _USB_Setup
{
	U8_T d8[8];

	struct _SetupPkt_Struc
	{
		U8_T bmRequestType;
		U8_T bRequest;
		u16_t_u8_t wValue;
		u16_t_u8_t wIndex;
		u16_t_u8_t wLength;
	} b;
} 
USB_Setup_TypeDef;

typedef  struct  _DescHeader
{
	U8_T  bLength;
	U8_T  bDescriptorType;
} USBHC_DescHeader_t;

typedef enum _DCSETUP_CONTROL_STATE
{
	SETUP_IDLE,       /* 0 */
	SETTING_UP,       /* 1 */
	IN_DATA,          /* 2 */
	OUT_DATA,         /* 3 */
	LAST_IN_DATA,     /* 4 */
	LAST_OUT_DATA,    /* 5 */
	WAIT_STATUS_IN,   /* 6 */
	WAIT_STATUS_OUT,  /* 7 */
	STALLED,          /* 8 */
	ENDPx_PAUSE,      /* 9 */
	SETUP_COMPLETE,   /* 10*/   
} DCSETUP_CONTROL_STATE;    /* The state machine states of a control pipe */

typedef enum _DCENDPx_STATE
{
	DCENDPx_DATAIDLE,       /* 1 */
	DCENDPx_DATAING,        /* 2 */
	DCENDPx_DATAERR,        /* 3 */
	DCENDPx_DATA_COMPLETE,  /* 4 */
} DCENDPx_STATE;    /* The state machine states of a endpx pipe */

/* Following states are used for EnumerationState */
typedef enum {
	ENUM_IDLE = 0,
	ENUM_GET_DEV_SPEED,				/*  1 */
	ENUM_GET_DEV_DESC,				/*  2 */
	ENUM_SET_ADDR,					/*  3 */	
	ENUM_GET_CFG_DESC,				/*  4 */
	ENUM_GET_FULL_CFG_DESC,			/*  5 */
	ENUM_SET_CONFIGURATION,			/*  6 */
	ENUM_SET_REMOTEWAKEUP,			/*  7 */
	ENUM_GET_MICROSOFTOS_STRING,	/*  8 */
	ENUM_GET_LANGID_STRING,
	ENUM_GET_VENDOR_STRING,
	ENUM_GET_PRODUCT_STRING,
	ENUM_GET_SERIALNUM_STRING,
	ENUM_GET_MSOS_STRING,
	ENUM_GET_FULL_LANGID_STRING,
	ENUM_GET_FULL_VENDOR_STRING,
	ENUM_GET_FULL_PRODUCT_STRING,
	ENUM_GET_FULL_SERIALNUM_STRING,
	ENUM_GET_FULL_MSOS_STRING,
	ENUM_SET_PROTOCOL,
	ENUM_DEV_CONFIGURED,
	ENUM_TEST_SET_REPORT,
	ENUM_TEST_GET_REPORT,
	ENUM_VENDOR_IN_TEST,
	ENUM_START,
	ENUM_GET_DEV_DESC0,
	ENUM_NO_EVENT,
	ENUM_FAIL
} ENUM_State;  

typedef enum {
	HID_ENUM_IDLE = 0,
	HID_ENUM_SET_PROTOCOL,
	HID_ENUM_GET_PROTOCOL,
	HID_ENUM_SET_REPORT,
	HID_ENUM_GET_REPORT,
	HID_ENUM_SET_IDLE,
	HID_ENUM_GET_RPT_DESC,
	HID_ENUM_GET_RPT_DESC_WAIT,
	HID_ENUM_FAIL,
	HID_ENUM_NO_EVENT
} HID_ENUM_State;

typedef enum {
	HUB_ENUM_IDLE = 0,
	HUB_ENUM_GET_DESC,    //Get HUB Descriptor
	HUB_GET_PORT_STATUS,  //Get Port Status
	HUB_SET_FEATURE,
	HUB_CLEAR_FEATURE,
	HUB_SET_FEATURE_POWER,
	HUB_SET_FEATURE_RESET,
	HUB_CLS_FEATURE_CONNECT,
	HUB_CLS_FEATURE_RESET,
	HUB_ENUM_FAIL,
	HUB_ENUM_NO_EVENT
} HUB_ENUM_State;

typedef enum{
	SCMD_IDLE = 0,
	//~~~~~~~~~
	//HID_CLASS
	//~~~~~~~~~
	/*0x01*/SCMD_SET_LED_REPORT,		
	/*0x02*/SCMD_SET_UNIFYING_REPORT,
	/*0x03*/SCMD_SET_UNIFYING_HID,
	/*0x04*/SCMD_GET_REPORT,
	/*0x05*/SCMD_GET_STATUS,
	/*0x06*/SCMD_SET_IDLE,
	/*0x07*/SCMD_GET_IDLE,
	/*0x08*/SCMD_GET_PROTOCOL,
	/*0x09*/SCMD_SET_PROTOCOL,
	/*0x0a*/SCMD_SET_FEATURE,
	/*0x0b*/SCMD_CLEAR_FEATURE,
	/*0x0c*/SCMD_SET_HUB_FEATURE,
	/*0x0d*/SCMD_CLEAR_HUB_FEATURE,
	/*0x0e*/SCMD_SET_CONFIGURATION,
	/*0x0f*/SCMD_SET_INTERFACE,
	//~~~~~~~~~
	//VENDOR
	//~~~~~~~~~
	SCMD_SINGLE_PORT=0x0f,
	SCMD_VENDOR,
	SCMD_FAIL,
	SCMD_COMPLETE,
} SINGLE_CMD_State;

typedef enum {
	PASSTHROUGH_IDLE = 0,

	/*Control Transfer Type*/
	PASSTHROUGH_SETUP_START,			//01
	PASSTHROUGH_SETUP_START_DONE,		//02
	PASSTHROUGH_DC_SETUP_DATA_IN,		//03
	PASSTHROUGH_DC_SETUP_DATA_OUT,		//04
	PASSTHROUGH_HC_SETUP_DATA_IN,		//05
	PASSTHROUGH_HC_SETUP_DATA_OUT,		//06
	PASSTHROUGH_HC_SETUP_DATA_OUT_DONE,	//07
	PASSTHROUGH_DC_SETUP_STATUS_IN,		//08
	PASSTHROUGH_DC_SETUP_STATUS_OUT,	//09
	PASSTHROUGH_HC_SETUP_STATUS_IN,		//10
	PASSTHROUGH_HC_SETUP_STATUS_OUT,	//11
	PASSTHROUGH_DC_SETUP_STALL,			//12
	PASSTHROUGH_HC_SETUP_STALL,			//13
	PASSTHROUGH_SETUP_TERMINATE,		//14 not normal length control ternimate.(physical leggth is less then total length)

	/*Interrupt Transfer Type*/
	PASSTHROUGH_INT_DATA_IN,			//15
	PASSTHROUGH_INT_DATA_OUT,			//16

	/*Bulk Transfer Type*/
	PASSTHROUGH_BULK_DATA_IN,			//17
	PASSTHROUGH_BULK_DATA_OUT,			//18
	PASSTHROUGH_FAIL,					//19
	PASSTHROUGH_COMPLETE,				//20
	PASSTHROUGH_STALLED,				//21
	PASSTHROUGH_NO_EVENT				//22
} PASSTHROUGH_State;

typedef enum {
	CLASS_ENUM_IDLE = 0,
	CLASS_ENUM_HUB,
	CLASS_ENUM_HID,
	CLASS_ENUM_MSD,
	CLASS_ENUM_TEST,
	CLASS_ENUM_FAIL,
} CLASS_ENUM_State;


/* Following states are used for CtrlXferStateMachine */
typedef enum {
	/*00*/ CTRL_IDLE =0,
	/*01*/ CTRL_SETUP,
	/*02*/ CTRL_SETUP_WAIT,
	/*03*/ CTRL_DATA_IN,
	/*04*/ CTRL_DATA_IN_WAIT,
#if 0
	/*05*/ CTRL_DATA_LAST_IN,
	/*06*/ CTRL_DATA_LAST_IN_WAIT,
#endif
	/*05*/ CTRL_DATA_OUT,
	/*06*/ CTRL_DATA_OUT_WAIT,
	/*07*/ CTRL_STATUS_IN,
	/*08*/ CTRL_STATUS_IN_WAIT,
	/*09*/ CTRL_STATUS_OUT,
	/*0a*/ CTRL_STATUS_OUT_WAIT,
	/*0b*/ CTRL_ERROR,
	/*0c*/ CTRL_STALLED,
	/*0d*/ CTRL_COMPLETE,
	/*0e*/ CTRL_FAIL
} CTRL_State; 

typedef struct _HD_Ctrl
{
	U8_T InfAddr;				// Interface address
	U8_T TdNum;					// TD Number
	U8_T EpN_MaxSize;			// Endpoint Buffer Max size
	U8_T *Buf;					// Endpoint Buffer Pointer(For OUT=>Send, For IN=>Read Buffer)
	U16_T Current_Length;		// Current Total Data length
	U16_T Total_Length;			// Total Data length
	U8_T  ErrorCnt;
	U16_T NakeCnt;               // Contorl nake counter
	CTRL_State State;			// control transfer stage
	USB_Setup_TypeDef Setup;	// 8  bytes setup command header, for send

} USBHC_Ctrl_TypeDef;

typedef struct _HCP_Ctrl
{
	U8_T  *Buf;					// Endpoint Buffer Pointer(For OUT=>Send, For IN=>Read Buffer)
	U16_T Current_Length;		// Current Total Data length
	U16_T Data_Length;			// Current data package length
	U16_T Total_Length;			// Total Data length
	U8_T  ErrorCnt;
	CTRL_State State;			// control transfer stage
	U8_T UPID;					// upstream port
	USB_Setup_TypeDef Setup;	// 8  bytes setup command header, for send
} USBHC_PassCtrl_TypeDef;

/* Following states are used for gState */
typedef enum {
	HOST_IDLE=0,
	/*0x01*/ HOST_ROOT_HUB_PORT_RESET,
	/*0x02*/ HOST_HUB_PORT_RESET,
	/*0x03*/ HOST_DEV_RESET,
	/*0x04*/ HOST_DEV_RESET_WAIT,
	/*0x05*/ HOST_DEV_ATTACHED,  
	/*0x06*/ HOST_DEV_DISCONNECTED,  
	/*0x07*/ HOST_DETECT_DEVICE_SPEED,
	/*0x08*/ HOST_ENUMERATION, 
	/*0x09*/ HOST_HID_CLASS_ENUMERATION,
	/*0x0a*/ HOST_CLASS_ENUMERATION,
	/*0x0b*/ HOST_ENUMLATE_TERMINATED,
	/*0x0c*/ HOST_SCM,
	/*0x0d*/ HOST_PASSTHROUGH,
	/*0x0e*/ HOST_TEST,
	/*0x0f*/ HOST_CLASS_REQUEST,
	/*0x10*/ HOST_CLASS,
	/*0x11*/ HOST_CTRL_XFER,
	/*0x12*/ HOST_USR_INPUT,
	/*0x13*/ HOST_SUSPENDED,
	/*0x14*/ HOST_ERROR_STATE
}HOST_State; 

typedef struct _HUBDescriptor {	// 集線器描述元
	U8_T bLength;
	U8_T bDescriptorType;		// should be 0x29
	U8_T bNbrPorts;
	U8_T wHubCharacteristicsL;
	U8_T wHubCharacteristicsH;
	U8_T bPwOn2PwrGood;			// 以2ms 為單位
	U8_T bHubontrCurrent;
	U8_T DeviceRemovable;
	U8_T PortPwrCtrlMask;
} USBHC_HUBDec_Typedef,*PUSBHC_HUBDec_Typedef;

//--------------------------------
//$ISO_State
//--------------------------------
//#define ISO_PINPON_MASK		0x01 //in pinponmode?
//#define ISO_TD_MASK			0x02 //0-ISTL0,1-ISTL1 

typedef struct _Host_TypeDef
{
	//Device Control state -----------------------------------------
	USBHC_Ctrl_TypeDef Control;				/* Control Transfer transmitt Control Structure */
	HOST_State gState;						/* Host State Machine Value */
	U8_T ResetCnt;							/* Reset counter */
	ENUM_State EnumState;					/* Class Enumeration state Machine */
//	U8_T EnumerateErr;						/* Class Enumeration state Machine */
	HID_ENUM_State HID_EnumState;			/* HID Enumeration state Machine   */
	HUB_ENUM_State HUB_EnumState;			/* HUB Enumeration state Machine   */
	HUB_ENUM_State HUB_EnumPort;			/* HUB Enumeration state Machine   */
	SINGLE_CMD_State ScmdState;				/* Single command  state Machine   */
	SINGLE_CMD_State ScmdStep;				/* Single command  Perform Commandstate Machine */
	PASSTHROUGH_State PassThrough_State;	/* PASS Through state Machine      */
	PASSTHROUGH_State PassThrough_Step;		/* Next PASS Through state Machine */
	HOST_State PassThrough_gState_bk;		/* When go into Passthrough mode, the QState backup */
	USBHC_PassCtrl_TypeDef 	PControl;		/* Pass Control Structure */
	U8_T ErrorCnt;							/* Enumeration state Machine */
	U8_T HandleState;						/* Different Handle Stage */
#ifdef KVM_EXTENDER_RECEIVER
	U8_T Etdr_RejectMountFlag;
#endif
} USBHC_HOST, *pUSBHC_HOST;

typedef struct _USBHC_HubReportTypeDef
{
	U8_T  wPortStatusL;
	U8_T  wPortStatusH;
	U8_T  wPortChangeL;
	U8_T  wPortChangeH;
	U8_T  Devinx;		//connect to which devinx
} USBHC_HubReportTypeDef;
  

typedef struct _HUSBHC_HubTypeDef
{
	USBHC_HubReportTypeDef *ReportState;	// Report & Status 
	U8_T  HubStatus;						// 1 byte hub status change
	U8_T  IntTD;							// Int TD Number
	U8_T  HC_qt_Bk;							// How to handle the port changed status way
	U8_T  ReportEndpInx;					// the endpoint id for Hub Status Change 
} USBHC_HubTypeDef;


typedef struct _Desc_Table
{
	U16_T  Len;
	U8_T  *Ptr;
} Desc_Table_TypeDefine;


/*
&State
*/

#define  PASSTHROUGH_IDLE				0x00 //DC is idle(in init state)
#define  PASSTHROUGH_SET_MASK			0x01 //DC is normatl, and accept data normally
#define  PASSTHROUGH_SET				0x01 //DC is normatl, and accept data normally
#define  PASSTHROUGH_REMOTE				0x02 //Remote
#define  PASSTHROUGH_WAIT				0x04 //Remote

#define  PASSTHROUGH_BUF_TYPE_MASK	0x0  //Buffer type 0x00-Interrupt TD Buffer
										 //				 0x08-Alloc memory
#define  PASSTHROUGH_BUF_TYPE_TD	0x00 //Buffer type 0x00-Interrupt TD Buffer
#define  PASSTHROUGH_BUF_TYPE_XDAT	0x08 //Buffer type 0x00-Interrupt TD Buffer

typedef struct _PassThrough_TypeDef
{  
  U8_T  Rp;           /* Read pointer for Interrupt pass through */
  U8_T  Wp;           /* Write pointer for Interrupt pass through */
#ifdef SYNC
  U8_T  SyncWp[KVM_MAX_PORT];    /* Write pointer for Interrupt pass through for sync mode */
  U8_T  SyncRp[KVM_MAX_PORT];    /* Write pointer for Interrupt pass through for sync mode */  
#endif	
  U8_T  BufSize;      /* Host State Machine Value,the maxinmum size should only 192 bytes 	*/
  U8_T  State;        /* PassThrough Control State 	*/
  U8_T  *Buf;         /* The ring buffer start pointer */
  U8_T  SyncState;    /* PassThrough Control State 	*/
} PassThrough_TypeDef;


/* Exported constants --------------------------------------------------------*/
/* Definition of "USBbmRequestType" */
#define REQUEST_TYPE      0x60  /* Mask to get request type */
#define STANDARD_REQUEST  0x00  /* Standard request */
#define CLASS_REQUEST     0x20  /* Class request */
#define VENDOR_REQUEST    0x40  /* Vendor request */

#define RECIPIENT         0x1F  /* Mask to get recipient */

#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05
#define USB_REPORT_DESCRIPTOR_TYPE              0x06
#define USB_HUB_DESCRIPTOR_TYPE                 0x29
#define HID_DESCRIPTOR_TYPE                     0x21


#define CLASS_OUT_SET_IDLE  0x0A

/*----------- supported classes -----------*/
#define USB_AUDIO_CLASS                 0x01
#define USB_CDC_CLASS                   0x02
#define USB_HID_CLASS                   0x03
#define USB_MSC_CLASS                   0x08
#define USB_HUB_CLASS                   0x09
#define USB_TEST_CLASS                  0x99
#define USB_VENDER_CLASS                0xFF


/*
$ USB Disable Stage
*/
//typedef enum {
//	USB_DISABLE_IDLE=0,      //0x00
//	USB_DISABLE_UMOUNT_DC,   //0x01
//	USB_DISABLE_DC_WAIT,     //0x02
//	USB_DISABLE_CLEAR_DC,    //0x03
//	USB_DISABLE_CLEAR_HC,    //0x04
//	USB_DISABLE_HC_RESET,    //0x05
//} USB_DISABLE_STAGE;	

#endif /* __USB_DEF_H */

/* End of usb_def.h */

