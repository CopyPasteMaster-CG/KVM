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
 * Module Name : usbhc.h
 * Purpose     : A header file of usbhc.c
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

#ifndef __USB_HC_H__
#define __USB_HC_H__

/* INCLUDE FILE DECLARATIONS */
#include "usbhc_cfg.h"
#include "usbhc_core.h"

#define DESC_MAX							12
#define USBHC_TD_DEVICE_ADDR_MASK			0x7f
#define USBHC_TD_USED_MASK					0x80
#define Hc_PASSTHROUGH_BUF_SIZ				192

/* DATA STRUCTURE DECLARATIONS */
/* USB_PDevcie_State Define */
#define DEVCIE_PLUG_RESET_PROCESSING		0x01
#define DEVCIE_PLUG_IN_PROCESSING			0x10
#define DEVCIE_PLUG_IN						0x20
#define DEVCIE_PLUG_OUT_PROCESSING			0x40
#define DEVCIE_PLUG_OUT						0x80

/*--------------
$ Byte1 Define
----------------*/
#define ACTUAL_BYTE_MSB_MASK				0x03
#define COMPLETEION_CODE_MASK				0xf0
#define COMPLETE_NO_ERROR					0x00
#define COMPLETE_CRC						0x10
#define COMPLETE_BIT_STUFFING				0x20
#define COMPLETE_DATA_TOGGLE_MISMATCH		0x30
#define COMPLETE_STALL						0x40
#define COMPLETE_DEVICE_NOT_RESPONDING		0x50
#define COMPLETE_PID_CHECK_FAILURE			0x60
#define COMPLETE_UNEXPECTED_PID				0x70
#define COMPLETE_DATA_OVERRUN				0x80
#define COMPLETE_DATA_UNDERRUN				0x90
#define COMPLETE_DATA_NAKE					0xa0
#define COMPLETE_RESERVED					0xb0
#define COMPLETE_BUFFER_OVERRUN				0xc0
#define COMPLETE_BUFFER_UNDERRUN			0xd0
#define TD_ACTIVED_MASK						0x08
#define TD_TOGGLE_MASK						0x04
#define TD_TOGGLE_DATA0						0x00
#define TD_TOGGLE_DATA1						0x04
#define TD_ACTUAL_BYTE_MSB					0x03

/*--------------
$ Byte3 Define 
----------------*/
#define TD_ENDP_NUMBER_MASK					0xf0
#define TD_ENDP_NUMBER_SHIFT				<< 4
#define TD_ISTL_LAST_MASK					0x08
#define TD_SPEED_MSK						0x04
#define TD_SPEED_LOW						0x04
#define TD_SPEED_FULL						0x00
#define TD_MAXPKT_SIZE_MSB					0x03

/*--------------
$ Byte5 Define 
----------------*/
#define TD_ERROR_COUNT_MASK					0x30
#define TD_ERROR_COUNT_SHIFT				>> 4
#define TD_DIR_TOKEN_MASK					0x0C
#define TD_DIR_TOKEN_SETUP					0x00
#define TD_DIR_TOKEN_OUT					0x04
#define TD_DIR_TOKEN_IN						0x08
#define TD_DIR_TOKEN_RESERVED				0x0C
#define TD_TOTAL_BYTE_MSB					0x03

/*--------------
$ Byte5 Define 
----------------*/
#define TD_ATL_NAK_COUNT					0xff
#define TD_INT_POLLING_RATE_MASK			0xf0
#define TD_INT_POLLING_RATE_SHIFT			<< 4
#define TD_INT_STARTING_FRAME_MASK			0x0f
#define TD_ISTL_STARTING_FRAME_MASK			0xff

/*------------------------------*/
/* DATA STRUCTURE DECLARATIONS  */
/*------------------------------*/
/*
TD_HEADER_Struct()
*/
typedef struct
{
	U8_T	Byte0_Actual_Byte;
	U8_T	Byte1;
	U8_T	Byte2_MaxPKT_Size;
	U8_T	Byte3;
	U8_T	Byte4_Total_Byte;
	U8_T	Byte5;
	U8_T	Byte6_Function_Address;
	U8_T	Byte7;
} USBHC_TD_Header_Typedef;

/*
TD_ISTL_Struct()
*/
typedef struct
{
	USBHC_TD_Header_Typedef		TD;
	U8_T						Buf[USB_HC_ISTL_BUF_SIZE];
} ISTL_TDBuf_Typedef;

/*
TD_INTL_Struct()
*/
typedef struct
{
	USBHC_TD_Header_Typedef		TD;
	U8_T						Buf[USB_HC_INTL_BUF_SIZE];
} INTL_TDBuf_Typedef;

/*
TD_ATL_Struct()
*/
typedef struct
{
	USBHC_TD_Header_Typedef		TD;
	U8_T						Buf[USB_HC_ATL_BUF_SIZE];
} ATL_TDBuf_Typedef;

/*
TD_BULK_Struct()
*/
#if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST))
typedef struct
{
	USBHC_TD_Header_Typedef		TD;
	U8_T						Buf[USB_HC_BULK_BUF_SIZE];
} BULK_TDBuf_Typedef;
#endif /* #if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST)) */
/*
TD_Table_Struct()
*/
typedef struct _HCTD_Table
{
	/*1.ISTL=Isochronous */
	ISTL_TDBuf_Typedef	ISTL[USB_HC_ISTL_MAX];
	/*2.INTL=Interrupt */
	INTL_TDBuf_Typedef	INTL[USB_HC_INTL_MAX];
#if ((SYSTEM_MSC_DEVICE_SUPPORT) && (SYSTEM_USB_HC_BURST))
	/*3.ATL=Control+Bulk */
	ATL_TDBuf_Typedef	ATL[USB_HC_CONTROL_MAX];
    /*4.BULK=Bulk Burst */
    BULK_TDBuf_Typedef	BULK[USB_HC_BULK_MAX];
#else
	ATL_TDBuf_Typedef	ATL[USB_HC_ATL_MAX];
#endif
} HCTD_Table_Typedef;

/** @defgroup USBD_CORE_Exported_Defines
  * @{
  */ 
#define BOT_IDLE							0 /* Idle state */
#define BOT_DATA_OUT						1 /* Data Out state */
#define BOT_DATA_IN							2 /* Data In state */
#define BOT_LAST_DATA_IN					3 /* Last Data In Last */
#define BOT_SEND_DATA						4 /* Send Immediate data */


/* CSW Status Definitions */
#define CSW_CMD_PASSED						0x00
#define CSW_CMD_FAILED						0x01
#define CSW_PHASE_ERROR						0x02

/* BOT Status */
#define BOT_STATE_NORMAL					0
#define BOT_STATE_RECOVERY					1
#define BOT_STATE_ERROR						2

#define DIR_IN								0
#define DIR_OUT								1
#define BOTH_DIR							2

/*
$bmFlags
*/
#define BOT_DIR_IN							0x80

/*---------------------------------------
MSC BOT CBW & CSW Struct Define
----------------------------------------*/
typedef struct _MSC_BOT_CBW
{
	U32_T	dSignature;
	U32_T	dTag;
	U32_T	dDataLength;
	U8_T	bmFlags;
	U8_T	bLUN;
	U8_T	bCBLength;
	U8_T	CB[16];
} MSC_BOT_CBW_TypeDef;

typedef struct _MSC_BOT_CSW
{
	U32_T	dSignature;
	U32_T	dTag;
	U32_T	dDataResidue;
	U8_T	bStatus;
} MSC_BOT_CSW_TypeDef;

typedef struct _MSC_BOT_CONTROL
{
	MSC_BOT_CBW_TypeDef		cbw;
	MSC_BOT_CSW_TypeDef		csw;
} MSC_BOT_Control_TypeDefine;

typedef struct _USBHC_Bulk
{
	U8_T	Rp;
	U8_T	Wp;
	U8_T	Cnt;
} USBHC_Bulk_TypeDef;

typedef struct _USBHC_MSC_Ctrl
{
#if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST))
	U16_T		Data_Length; // Current data package length
#else
	U8_T		Data_Length; // Current data package length
#endif	
	U32_T		Current_Length; // Current Total Data length
	U32_T		Total_Length; // Total Data length
	U8_T		DcBufFullCnt;
	CTRL_State	State; // control transfer stage
} USBHC_MSC_Ctrl_TypeDef;

/*----------------------------------------------
$ Attr
-----------------------------------------------*/
#define MSC_PROTOCOL_MASK					0x03
#define MSC_BOT_PROTOCOL					0x00
#define MSC_CBI_PROTOCOL					0x01
#define MSC_PORT_MASK						0x0c
#define MSC_ENDPINX_MASK					0x70
#define MSC_OCCUPY_MODE_MASK				0x80
#define MSC_OCCUPY_MODE_SET					0x80
#define MSC_OCCUPY_MODE_RESET				0x00

/*----------------------------------------------
$ Stage
-----------------------------------------------*/
#define MSC_BOT_IDLE						0x00
#define MSC_BOT_CBW_OUT						0x01
#define MSC_BOT_CBW_WAIT					0x02
#define MSC_BOT_CSW_IN						0x03
#define MSC_BOT_CSW_WAIT					0x04
#define MSC_BOT_DATA_OUT					0x05
#define MSC_BOT_DATA_OUT_LAST				0x06
#define MSC_BOT_DATA_IN						0x07
#define MSC_BOT_DATA_IN_LAST				0x08
#define MSC_BOT_CSW_REQUEST					0x09
#define MSC_BOT_OUT_END						0x0a
#define MSC_BOT_IN_END						0x0b
#define MSC_BOT_END							0x0c
#define MSC_BOT_STALL						0x0d
#define MSC_BOT_TOGGLE_OUT_MASK				0x01
#define MSC_BOT_TOGGLE_IN_MASK				0x02

typedef struct _USBHC_MSC_Device
{	
	U8_T					Switch_State;	// switch control & state
	U8_T					Switch_ErrCnt;	// switch control & state
	U8_T					Attr;			// Attribute
											// bit0-1 => Protocol type 00->BOT
											// bit2~3 => Port
											// bit7~4 => Endpinx
	U8_T					BOT_State;		// bit0-CBW WAIT Stage
											// bit1-CSW OUT Stage
											// bit2-DATA OUT Stage
											// bit3-DATA IN Stage
#if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST))
	U8_T					BURST_Out_State;
	U8_T					BURST_In_State;
#endif	
	MSC_BOT_Control_TypeDefine	*BOT;		// Contain 31 byte CBW command header, 13 bytes CSW status header
#if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST == 0))
	SWDMA_TypeDef			In_Buf[2];
	SWDMA_TypeDef			Out_Buf[2];
#endif
	U8_T					In_Endpinx;
	U8_T					In_EndpAddr;	// Bit 7~4 interface id
											// Bit 3~0 Endp id
#if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST == 0))
	U8_T					*In_Len[2];		// Support Max 8 TD,each bit standard for one TD
#endif
	U8_T					Out_Endpinx;
	U8_T					Out_EndpAddr;	// Bit 7~4 interface id 
											// Bit 3~0 Endp id
#if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST == 0))
	U8_T					*Out_Len[2];	// Support Max 8 TD,each bit standard for one TD
#endif
	U8_T					Host;			// connect to which host
	U8_T					Toggle_In;		// bit0-for OUT
											// bit1-for IN
	U8_T					Toggle_Out;		// bit0-for OUT
	USBHC_MSC_Ctrl_TypeDef	Control;
} USBHC_MSC_Device_TypeDef;

/*
USBHC_Device_Prop()
*/
#define USBHC_MAX_NUM_INTERFACES			6 /* Support max interface number ineach device */
#define USBHC_MAX_NUM_ENDP					6 /* Support max endpoint in eache interface */
#define USBHC_MAX_NUM_PORT					8 /* Support max downstreams in HUB Class */

/* Byte Addr */
#define USBHC_DEVICE_USED_MASK				0x80
#define USBHC_DEVICE_ADDR_MASK				0x7f

/*$ Byte InterfaceNum */
#define USBHC_DEVICE_SPEED_MASK				0x80
#define USBHC_DEVICE_INTERFACENUM_MASK		0x0f

/*$ Byte Hub_NbrPorts */
#define USBHC_ROOTHUBPORT_CONNECT_MASK		0x80
#define USBHC_ROOTHUBPORT_PORT_NUM_MASK		0x70 //if the USBHC_ROOTHUBPORT_CONNECT_MASK is not enable, then it is the layer number
#define USBHC_ROOTHUBPORT_PORT_NUM_SHIFT	<< 4
#define USBHC_ROOTHUBPORT_PORT_NUM_READ		>> 4
#define USBHC_HUBPORT_MASK					0x0f

#define DEVICE_DESC							0
#define CONFIG_DESC							1
#define LANG_ID								2
#define VENDOR_ID							3
#define PRODUCT_ID							4
#define SERIAL_ID							5
#define HID0_REPORT							6
#define HID1_REPORT							7
#define HID2_REPORT							8
#define HID3_REPORT							9
#define HUB_DESC							10
#define MICROSOFT_OS						11
#define HID_DESCRIPTOR						0x21

#define INF_NONE							0x00
#define INF_KEYBOARD						0x01
#define INF_MOUSE							0x02

/*
$DevAttr Define 
*/
#define DEVATTR_ENUMERLATE_ERR_MASK			0x80
#define DEVATTR_KB_CLASS_MASK				0x01
#define DEVATTR_UNIFYING_MASK				0x02 //Logitech Unifying Wireless Mouse
#define DEVATTR_TD2220_MASK					0x04 //Viewsonic TD2220 optical toucher
#define DEVATTR_UNIFYING_DJ					0x10 //Must used this, same defined with DC												 
//#if (SYSTEM_EXTENDER_SUPPORT)
#define DEVATTR_REMOTE_DEV_MASK				0x80  //device in extender receiver site
//#endif /* (SYSTEM_EXTENDER_SUPPORT) */
/*
$HID_Kb_State Define Table
*/
#define HID_KB_BUF_USED_FLAG				0x01
#define HID_KB_LED_OUT_USED_FLAG			0x02

/*
$Reset_Issue
*/
#define RESET_HC_EVENT						0x01 //Caused by HC, this is always reset when switch
#define RESET_DC_EVENT						0x02 //This is caused by DC, could be reset only one time
#define RESET_WHEN_SWITCH_EVENT				0x04 //This is caused by DC, could be reset only one time
#define RESET_ONCE_WHEN_SWITCH_EVENT		0x08 //This is caused by DC, could be reset only one time

#define DEVICE_KEEP_DONE_MASK				0x01	
/*
$HID_Parser_State
*/
#define PARSER_START_MASK			0x01 //0-Parser Init,1-Parser already start
typedef struct _USBHC_HID_Parser
{
	U8_T					HID_Kb_LED_Value[2];	// This contain the hid keyboard led value
	HID_Rpt_Parser_TypeDef	*HID_Kb_Parser;			// HID_Rpt_Parser_TypeDef Pointer
	HID_Rpt_Table_TypeDef	*HID_Kb_ParserTable;	// Support 2 interface keyboard parser
} USBHC_HIDParser_TypeDefine;

typedef struct _USBHC_Device
{
	//~~~~~~~~~~~~~~~~~~~~~~~
	//1.Unique Part
	//~~~~~~~~~~~~~~~~~~~~~~~
	U8_T		Addr;					// Device address, assigned by HC
										// For device connect with Root Hub Port is start from 0x01~0x04
										// for device connect with Hub->Root Hub is start from 0x11~0x14
										// for device connect with hub->Hub->Root Hub is start from 0x21~0x24
	U8_T		RootHubPort;			// HC root Hub port
	U8_T		DevAttr;				// bit 0~bit 2 - current keyboard led value
										// bit3 - keyboard sub class
	U16_T		idVendor;				// Vendor ID
	U16_T		idProduct;				// Product ID
//	U8_T 		stallRetryCnt;			// Received STALL and retry for the device responding lately.
	U8_T		timeOutRetryCnt;
	
	//~~~~~~~~~~~~~~~~~~~~~~~
	//2.Devcie Description
	//~~~~~~~~~~~~~~~~~~~~~~~
	//2_1.Device Description
	U8_T		ControlEndpSize;		// Control Endpoint buffer size
	U8_T		DevClass;				// Class Declare
	//2_2.Description Table Array
	Desc_Table_TypeDefine		Desc[DESC_MAX];			// Different Description
	//2_3.Contorl
	U8_T		ConfigurationValue;		// Current Configuration value
	U8_T		Hid_SetIdle_Allow;		// HID class, each interface set_idle setting
	U8_T		TotalConfiguration;		// Totao1 Configuration value
	U8_T		Current_Protocol;		// 0-report 1-boot, each bit standard for one interface.
	//2_4.Configuration Description
	U8_T		InterfaceNum;
	U8_T		InfClass[USBHC_MAX_NUM_INTERFACES];		// Class ID for each interface
	U8_T		InfSubClass[USBHC_MAX_NUM_INTERFACES];	// Boot interface subclass
	U8_T		InfSetIdle[USBHC_MAX_NUM_INTERFACES];	// HID class, each interface set_idle setting
	U8_T		InfProtocol[USBHC_MAX_NUM_INTERFACES];	// Interface protocol- 0 None, 1 Keyboard, 2 Mouse, 3-255 Reserved.
	U8_T		Current_AlternateSetting[USBHC_MAX_NUM_INTERFACES];	// the current alternate setting in each interface.
	U8_T		Attribute;	// Bit 6: self-power (1) bus-power (0)
							// Bit 5: remote-wakeup (1) no-remote-wakeup (0)
	//Interface Description
	U8_T		InfAltValue[USBHC_MAX_NUM_INTERFACES];	// Alternate setting for each interface
	U8_T		EndpNum[USBHC_MAX_NUM_INTERFACES];		// Endpointer number in each interface(include endp 0)
	//Endpoint buffer pipe Description
	U8_T		EndpAddr[USBHC_MAX_NUM_INTERFACES][USBHC_MAX_NUM_ENDP];	//Endpoint address ID, not includ the control point
	U8_T		EndpSize[USBHC_MAX_NUM_INTERFACES][USBHC_MAX_NUM_ENDP];	//Store the endpoint pipe size
	U8_T		EndpType[USBHC_MAX_NUM_INTERFACES][USBHC_MAX_NUM_ENDP];	//bit 0~2 , transfer type, bit 7~3 , TD id number
	U8_T		EndpInterval[USBHC_MAX_NUM_INTERFACES][USBHC_MAX_NUM_ENDP];	//Store the endpoint polling interval

	//~~~~~~~~~~~~~~~~~~~~~~~
	//3.Control Part
	//~~~~~~~~~~~~~~~~~~~~~~~
	U16_T						CfgTotalLen;							// Full Configuration Descriptor
	U16_T						HID_Rpt_Len[USBHC_MAX_NUM_INTERFACES];	// Class ID for each interface
	U8_T						UpperHubDevinx;			// upper layer hub devinx
	U8_T						Hub_NbrPorts;			// 1.Bit 7:Root Hub Port Connection
														// 2.Bit 5~4:Root Hub Port Number, if connect directly
														// 3.Bit 3~0:Hub Port Number,if port Class is 0x09, then this is Hub
	U8_T						Hub_Enu_Port_Id;		// 1.The Enumerating port id
	USBHC_HOST					Hc;						// Host control structure
	U8_T						Mounted;				// Mount flag
	U8_T						MountPort;				// Mount host flag
	U8_T						Usb_2_0;				// USB 2.0
	
	//~~~~~~~~~~~~~~~~~~~~~~~
	//4.HID parser
	//~~~~~~~~~~~~~~~~~~~~~~~
	U8_T						HID_Kb_LED_Value[2];	// This contain the hid keyboard led value
	U8_T						HID_Kb_Buf_InfEndp;		// bit8-hid keyboard used,bit4~6 intf number,bit3~0 endpoint buffer number
	U8_T						HID_Kb_Press_Cnt;
	U8_T						HID_Gigabyte_Kb_03_Cnt;
	U8_T						HID_Gigabyte_Kb_04_Cnt;
	U8_T						HID_Gigabyte_Kb_03_Flag;		
	U8_T						*HID_Kb_Buf;			// Onlye support one keyboard buffer for 8 bytes
	U8_T						*HID_Kb_Buf2;			// Onlye support one keyboard buffer for 8 bytes
	HID_Rpt_Parser_TypeDef		*HID_Kb_Parser;			// HID_Rpt_Parser_TypeDef Pointer
	HID_Rpt_Table_TypeDef		*HID_Kb_ParserTable;	// HID_Rpt_Parser_TypeDef Pointer
//#if defined(HID_PARSER_CORE_KB)	
	U8_T						HID_Parser_State;		// bit0 -> ParserStart
	U8_T						*HID_Report_Table;
	U8_T						*HID_LED_Item;
//#endif
	//~~~~~~~~~~~~~~~~~~~~~~~
	//5.HUB Relative
	//~~~~~~~~~~~~~~~~~~~~~~~
	USBHC_HubTypeDef			HUB;
	
	//~~~~~~~~~~~~~~~~~~~~~~~
	//6.MSC Relative
	//~~~~~~~~~~~~~~~~~~~~~~~
	USBHC_MSC_Device_TypeDef	*MSC;
	
	//~~~~~~~~~~~~~~~~~~~~~~~
	//7.Passthrough port
	//~~~~~~~~~~~~~~~~~~~~~~~
	U8_T  Passthrough_uPort;
	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//8.Interval adjust for extender application
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	U8_T  Interval_Adjust;
	U8_T  DeviceKeep;	
} USB_PDevice_TypeDef;

/*
USBHC_Device_Polling()
*/
/*$ Control */
#define USBHC_INTT_CON_NORMAL_MASK			0x00
#define USBHC_INTT_CON_SKIP_MASK			0x01
#define USBHC_INTT_CON_STALL_MASK			0x02
#define USBHC_INTT_CON_TOGGLE_MASK			0x04
#define USBHC_INTT_CON_DATA0				0x00
#define USBHC_INTT_CON_DATA1				0x04
#define USBHC_INTT_CON_ERROR_MASK			0x08
#define USBHC_INTT_CON_TIME_CALIBRATE_START	0x10
#define USBHC_INTT_CON_TIME_CALIBRATE_DONE	0x20

/*$ Attrib */
#define USBHC_INTT_ATTRI_TOGGLE_MASK 		0x01
#define USBHC_INTT_ATTRI_TOGGLE_DATA0		0x00
#define USBHC_INTT_ATTRI_TOGGLE_DATA1		0x01
#define USBHC_INTT_ATTRI_DIR_MASK			0x02
#define USBHC_INTT_ATTRI_DIR_IN				0x00
#define USBHC_INTT_ATTRI_DIR_OUT			0x02
#define USBHC_INTT_ATTRI_SPEED_MASK			0x04
#define USBHC_INTT_ATTRI_LOW_SPEED			0x00
#define USBHC_INTT_ATTRI_FULL_SPEED			0x04
#define USBHC_PDEV_ENDPIDX_MASK				0x0f
#define USBDC_DEV_ENDPINX_MASK				0xf0

typedef struct _USBHC_IntTransfer_Struct 
{
	U8_T	Used_DeviceID;		// Used Flag & Device ID
	U8_T	InterfaceIdx;		// Which interface in device
	U8_T	EndpIdx;			// Which endpoint in device	
	U8_T	EndpAddr;			// EndpAddress
	U8_T	Control;			// Control Status
	U8_T	INTL_Num;			// INTL TD Number
	//U8_T	NewInterval;		// new interval to be changed
	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//Ring Buffer Structure
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~
	PassThrough_TypeDef PassControl;	
#if (SYSTEM_EXTENDER_RECEIVER)
	U8_T	*ExtenderR;			// For Extender Receiver used 
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */
} USBHC_IntTransfer_TypeDef;  

//-------------------------------------
//Control Define for Isochronous
#define ISTL_IDLE							0x00
#define ISTL_ACTIVE							0x01
#define ISTL_PINPON							0x02 // Pinpon active flag
#define ISTL_PINPON_DO						0x04 // Next PinPon process id

#define ISO_CONTROL_TOGGLE					0x01

typedef struct _USBHC_IsoTransfer_Struct 
{
	U8_T	Used_DeviceID;		// Used Flag & Device ID
	U8_T	InterfaceIdx;		// Which interface in device
	U8_T	EndpIdx;			// Which endpoint in device	
	U8_T	EndpAddr;			// EndpAddress
} USBHC_IsoTransfer_TypeDef;

typedef struct _Host_ISR_EventQ_TypeDef
{
	//Device Control state -----------------------------------------
	U8_T	ISR_Active;			// bit0-Interrupt,bit1-Operation Interrupt
	U8_T	ISR_State;
	U8_T	OP_ISR_State;	
} USBHC_ISRFIFO_TypeDef;

//typedef struct _USBHC_INTLQ_TypeDef
//{
	//Device Control state -----------------------------------------
//	U8_T	Rp;					// bit0-Interrupt,bit1-Operation Interrupt
//	U8_T	Wp;
//	U8_T	MaxPackage;
//} USBHC_INTLQ_TypeDef;

/** @defgroup USBH_CORE_Exported_Types
  * @{
  */
typedef enum
{
	/*0x00*/	USBH_IDLE = 0,
	/*0x01*/	USBH_OK,
	/*0x02*/	USBH_DONE,
	/*0x03*/	USBH_TIMEOUT,
	/*0x04*/	USBH_NAKE,
	/*0x05*/	USBH_BUSY,
	/*0x06*/	USBH_FAIL,
	/*0x07*/	USBH_ERROR,
	/*0x08*/	USBH_NOT_SUPPORTED, // maybe not support
	/*0x09*/	USBH_UNRECOVERED_ERROR,
	/*0x0a*/	USBH_ERROR_SPEED_UNKNOWN,
	/*0x0b*/	USBH_STALL,
	/*0x0c*/	USBH_DATA_TOGGLE_ERR,
	/*0x0d*/	USBH_APPLY_DEINIT
}USBH_Status;

typedef enum
{
	HANDLE_IDLE = 0,
	HANDLE_CONTROL,
}Handle_State;

#define TD_CHANNEL_DEVICE_ADDRESS_MASK		0x7f
#define CHANNEL_COMPLETE_MASK				0xf0
#define CHANNEL_TOGGLE_DATA_MASK			0x01
#define CHANNEL_TOGGLE_DATA0				0x00
#define CHANNEL_TOGGLE_DATA1				0x01
#define CHANNEL_ATRI_SPEED_MASK				0x40
#define CHANNEL_ATRI_SPEED_LOW				0x40
#define CHANNEL_ATRI_SPEED_FULL				0x00
#define CHANNEL_ATRI_TYPE_MASK				0x30
#define CHANNEL_ATRI_TYPE_CONTROL			0x00
#define CHANNEL_ATRI_TYPE_IN				0x10
#define CHANNEL_ATRI_TYPE_OUT				0x20
#define CHANNEL_ATRI_ENDPADDR_MASK			0x0f

#ifdef HC_TIMEOUT
#define USBHC_DATAIN_NAKE_MAX				HC_TIMEOUT
#else
#define USBHC_DATAIN_NAKE_MAX				5000
#endif
typedef struct _HC_TD_Channel_TypeDef
{
	//Device Control state -----------------------------------------
	U8_T		Used_DeviceID;		// bit8-Used,bit0~6 device address
	U8_T		CompleteCode;
	U8_T		Attri;				// bit 7-Direction
									// bit 6-Speed
									// bit 5~4-Transfer Type 00-Control,01-Interrupt,10-Bulk,11-Isochronous
									// bit 3~0-Endpoint Address
	U8_T		*Buf;				// Buffer Store
	U16_T		Endp_Size;			// Endpoint Buffer Max size
	U16_T		Current_Length;		// Current Total Data length
	U16_T		TD_Length;			// data length for handle this time
	U16_T		Total_Length;		// Data Process Data Total Length
	U16_T		ActualByte;
	USBH_Status	USB_State;
} USBHC_TD_Channel_TypeDef;

//typedef struct _Host_Done_HandleQ_TypeDef
//{
	//Device Control state -----------------------------------------
//	U8_T	ISR_Type;				//ISTL,INTL,ATL
//	U8_T	DoneMap[4];
//} USBHC_ISRDone_TypeDef;

/* NAMING CONSTANT DECLARATIONS */


/* GLOBAL VARIABLES */
extern HCTD_Table_Typedef		HCTD_Table; //This is for Hardware requirement
extern U8_T						TASK_USBHC_Process_ID;
extern USB_PDevice_TypeDef		USB_PDevice[USB_HC_MAX_DEVICE+USB_HC_MAX_HUB];
extern USBHC_TD_Channel_TypeDef	HCTD_Channel_Table[USB_HC_ISTL_MAX+USB_HC_INTL_MAX+USB_HC_ATL_MAX]; // Store the device index
extern volatile USBHC_ISRFIFO_TypeDef	USBHC_ISR_FIFO[USB_HC_MAX_ISR_FIFO_DEPTH]; //keep 64 depth isr q status
extern U8_T						USBHC_Enumerate_New_Addr;
extern U8_T						USBHC_RootHub_Port;
extern USBHC_IntTransfer_TypeDef		HC_IntTransfer_Table[USB_HC_INTL_MAX];
extern USBHC_IsoTransfer_TypeDef		HC_IsoTransfer_Table[USB_HC_ISTL_MAX];
extern bit			TASK_USBHC_KVM_Switch_Check_Start_Flag;
extern U8_T			TASK_USBHC_Enumerate_Handle_ID;
extern U8_T			TASK_USBHC_Scm_Handle_ID;
extern U8_T			TASK_USBHC_PassThrough_Handle_Start_ID;
extern U8_T			TASK_USBHC_PassThrough_Handle_ID;
extern U8_T			TASK_USBHC_KVM_Switch_Check_ID;
//extern U8_T			USBHC_AUDIO_Device_Cnt;
//extern U8_T			USBHC_MSC_Device_Cnt;
extern const U8_T	MOUNT_PORT[];
extern U8_T    		TASK_USBHC_Audio_DC_Mount_Handle_ID;
extern U8_T			TASK_USBHC_Scm_Handle_Start_ID;
extern idata U16_T		MSC_Class_Page_Size;
#ifdef SYNC
extern U8_T 	USBHC_PDevice_SyncInit[USB_HC_MAX_DEVICE];
#endif /* #ifdef SYNC */
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void USBHC_Init(void);
void TASK_USBHC_Process(void);
void TASK_USBHC_Enumerate_Handle(void);
void USBHC_Reg_Read(U8_T *buf,U8_T address,U8_T length);
void USBHC_Reg_Write(U8_T *buf,U8_T address,U8_T length);
void USBHC_Parser_DevDesc(U8_T devinx,U8_T *DeviceDesc);
void USBHC_CfgDesc_Parser_Len(U8_T devinx,U8_T *CfgDesc);
void USBHC_CfgDesc_Parser(U8_T devinx,U8_T *CfgDesc);
U16_T Big_Endian_16_Convert(U16_T big_edin);
U8_T USBHC_Parser_ConfigDesc(U8_T devinx,U16_T total_len,U8_T *buf);
USBH_Status USBHC_Control_TD_Init(U8_T devinx,U8_T *rtn_tdid);
void USBHC_Memory_Fail_Msg(U8_T msgindex);
U8_T USBHC_InterruptTransferIN_Buf(U8_T intt_id,U8_T devinx,U8_T len);
void TASK_USBHC_Scm_Handle(void);
void USBHC_KB_Led_Brocast(U8_T led);
void USBDC_VIRTUAL_Start_PassThrough(U8_T devinx,U8_T port);
void TASK_USBHC_CORE_Control_Transfer_Handle(void);
void USBHC_Set_Dev_Inf_Protocol(U8_T devinx,U8_T curinf,U8_T protocol_value,U8_T cmd_type);
void USBHC_Set_Dev_Configuration(U8_T devinx,U8_T configuration_value);
void USBHC_Set_Dev_Inf_Alternation(U8_T devinx,U8_T curinf,U8_T alternate_value);
void USBHC_Set_Dev_Interface(U8_T devinx,U8_T interface,U8_T alternate);
void USBHC_HUB_LogNewAddress(void);
void USBHC_HUB_ReleaseEnumerateOperation(void);
void USBHC_Remove_Device(U8_T devinx, U8_T portReset);
U32_T Endian_32_Convert(U32_T endian);
void TASK_USBHC_HUB_RemoveDevinx_Fork(U8_T devinx);
void USBHC_Set_Dev_Logitech_Unifying_Mode(U8_T devinx,U8_T report_value);
void USBHC_Set_Dev_Logitech_Unifying_HID(U8_T devinx,U8_T report_value);
void TASK_USBHC_Device_Reset(U8_T,U8_T);
RESULT USBHC_GetVendorTest(U8_T devinx,U8_T td_id,U8_T emustate);
RESULT USBHC_PassThrough_Send_Setup_Token(U8_T devinx,U8_T td_id);
void USBHC_Active_New_Device(U8_T devinx,U8_T mount_port);
void USBHC_Remove_HID_Buffer(U8_T intt_id);
U8_T USBHC_Clone_PDevice(U8_T old_devinx,U8_T direction);
void USBHC_Get_Upper_Device_Info(U8_T devinx,U8_T *upper_dev,U8_T *upper_port);
void TASK_USBHC_PassThrough_Handle_Go(U8_T devinx,U8_T pid);
void USBHC_InterruptTransfer_Mouse_DVR(U8_T *buf,U8_T len);
void USBHC_InterruptTD_Transfer_Index_Init(U8_T inttid);
#ifdef SYNC
void USBHC_VirtualHID_MouseSync_XY0(void);
#endif /* #ifdef SYNC */
void USBHC_InterruptTransfer_HID_Mouse(U8_T *buf,U8_T buf_type);
U8_T USBHC_KB_Led_Check(U8_T devinx);

#ifdef VPID_CHANGER
extern USBHC_DevDesc_TypeDef Ran_Desc[USBDC_PORT_MAX];
#endif

#endif /* End of __USB_HC_H__ */
/* End of uart.h */
