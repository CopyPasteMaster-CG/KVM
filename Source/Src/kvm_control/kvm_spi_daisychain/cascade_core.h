 /*
 ******************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*=============================================================================
 * Module Name: cascade_core.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __CASCADE_H__
#define __CASCADE_H__

/* INCLUDE FILE DECLARATIONS 		  */
/* DEFINATION DECLARATIONS   		  */

#define NO_VALUE							0
#define NO_INDEX							0
#define NO_REQUEST							0
#define NO_REQ  							0
#define NO_CTRL_ATTR						0
#define NO_CTRL								0
#define NO_DATA								NULL
#define NO_LEN								0
#define BOXCASCADE_REQ						0xee


//#define CASCADE_PASSTHROUGH_BUF_MALLOC	  	0
#define SINGLE_MODULE_HOST_PORTS			4
#define CASCADE_HOST_PORT_START				4
#define CASCADE_PAGE_SIZ					512
#define SPI_NOT_USED		          		0
#define SPI_ROLE_MASTER             		1
#define SPI_ROLE_SLAVE	          			2
#define KVM_HOST_IDLE	          			0
#define KVM_HOST_ACTIVE	          			1
#define KVM_BTN_PRESS	          			1
#define KVM_BTN_RELEASE          			0
#define MAXIMUM_SLAVE						4
#define MAXIMUM_UNIT						2 //support 2 * 8 ports

#define CASCADE_MSC_BURST_SIZE	384 //256 burst data
#define CASCADE_MSC_BUF_CNT		2
#define CASCADE_MSC_BUF_SIZE	386//(CASCADE_MSC_BURST_SIZE + 2) //(256+2), 256 burst data and the first 2 byte burst length
#define CASCADE_MSC_BUF_MASK	(CASCADE_MSC_BUF_CNT -1)



//SPI Baud Divider
#if (MCU_REAL_CHIP) //96M
#define    SPI_24M			1
#define    SPI_16M			2
#define    SPI_12M			3
#define    SPI_6M			7
#define    SPI_4M		   11
#define    SPI_1M		   47
#else //48M
#define    SPI_24M			0
#define    SPI_16M			1
#define    SPI_12M			2
#define    SPI_6M			3
#define    SPI_4M		    5
#define    SPI_1M		   23
#endif


#define CASCADE_DMA_LENGTH			  	 	24    		//Support the maximun
#define CASCADE_TRANSMIT_FIFO_MAX   		48    		//Support the maximun
#define CASCADE_RX_TRIGGER_LEVEL			1024
#define CASCADE_RX_TIMEOUT_GAP				16			// 16 characters time out gap for receiver buffer ring
#define CASCADE_RX_BUF_START_ADDR			0x6000		/* The low byte of this start address must be 0x00 */
#define	CASCADEM_RX_BUF_SIZE				1024//1536	/* The RX ring size must be a multiple of 256bytes */

#define CASCADE_TRANSMITTER_ADDR       		0x70
#define CASCADE_RECEIVER_ADDR          		0x7F


#define CASCADE_SEND_MAX		          	5	
#define CASCADE_RECEIVE_TIME_OUT			100
//-------------------------------------
// Extender Transmitt FIFO structure
//-------------------------------------
#define CASCADE_CONTROL_IDLE           		0x00
#define CASCADE_CONTROL_TRANSMIT     		0x01
#define CASCADE_CONTROL_WAIT_LEN	 		0x02
#define CASCADE_CONTROL_TRANSMIT_NO_WAIT	0x04 //The transmit no need to wait
#define CASCADE_CONTROL_NONE_FREE      		0x08 //buffer should not free after send complete
#define CASCADE_CONTROL_RECEIVE	    		0x10
#define CASCADE_CONTROL_CLEAR_BUF			0x20
#define CASCADE_CONTROL_DATA_FOLLOW	 		0x40  //The transmit no need to wait
#define CASCADE_CONTROL_RBRS		 		0x80  //


//COMMAND Define section ----------------------------
#define   CASCADE_TOKEN_MASK				0x0f

#define   CASCADE_OUT_TOKEN        			0xe1
#define   CASCADE_IN_TOKEN        			0x69
#define   CASCADE_SOF_TOKEN        			0xa5
#define   CASCADE_CMD_TOKEN      			0x2d

#define   CASCADE_DATA0_TOKEN      			0xc3
#define   CASCADE_DATA1_TOKEN      			0x4b

#define   CASCADE_ACK_TOKEN      			0xd2
#define   CASCADE_NAK_TOKEN      			0x5a
#define   CASCADE_STALL_TOKEN      			0x1e

//-------------------
//$bCommand Define
//-------------------
//System 0x00~0x0f
#define   CASCADE_SYSTEM_CMD				0x0f  //
#define   CASCADE_SC_EDID					0x01  //Update edid table for monitor
#define   CASCADE_SC_SUSPEND				0x02  //perform power saving mode
#define   CASCADE_MASTER_RESTART			0x03  //indicate the master has been restart
#define   CASCADE_SUSPEND					0x04
#define   CASCADE_SWITCH_REMOTE				0x05
#define   CASCADE_SWITCH_LOCAL				0x06
#define   CASCADE_WAIT_RELEASE_CHECK		0x07
#define   CASCADE_UART0_CHANNEL				0x08
#define   CASCADE_SC_CONFIGURATION			0x09  //Software configuration data
#define   CASCADE_SC_VERSION				0x0A  //Software version report
#define   CASCADE_BOX_REDIRECT				0x0B  //Redirect the followed data to Console


//USB 0x10~0x4f
#define   CASCADE_USB_CMD					0x4f  //Transmitter side has been reset

#define   CASCADE_USB_SUSPEND				0x10  //perform USB bus suspend in DC port
#define   CASCADE_USB_REMOTE_WAKE			0x11  //perform USB bus remote wakeup in DC port
#define   CASCADE_USB_MOUNT					0x12  //Mount a USB device
#define   CASCADE_USB_UNMOUNT				0x13  //Unmount a USB device
#define   CASCADE_USB_DESCRIPTOR			0x14  //USB Descriptor End
#define   CASCADE_USB_DEV_DESP				0x15  //USB devcie descriptor
#define   CASCADE_USB_CONF_DESP				0x16  //USB devcie configuration descriptor
#define   CASCADE_USB_INF_DESP				0x17  //USB devcie interface descriptor
#define   CASCADE_USB_ENDP_DESP				0x18  //USB devcie endpoint descriptor
#define   CASCADE_USB_HID_DESP				0x19  //USB device interface HID descriptor
#define   CASCADE_USB_HUB_DESP				0x1a  //USB HUB device descriptor
#define   CASCADE_USB_DESCRIPTOR_END		0x1b  //USB Descriptor End
#define   CASCADE_USB_SETUP_STAGE			0x1c
#define   CASCADE_USB_DATA_IN_STAGE			0x1d
#define   CASCADE_USB_DATA_OUT_STAGE		0x1e
#define   CASCADE_USB_STATUS_STAGE			0x1f

#define   CASCADE_USB_SETUP_DONE			0x20
#define   CASCADE_USB_STATUS_IN				0x21
#define   CASCADE_USB_STATUS_OUT			0x22
#define   CASCADE_USB_STATUS_STALL			0x23
#define   CASCADE_USB_REMOUNT				0x24
#define   CASCADE_USB_INTR_IN				0x25
#define   CASCADE_USB_INTR_OUT				0x26
#define   CASCADE_USB_BULK_IN				0x27
#define   CASCADE_USB_BULK_OUT				0x28
#define   CASCADE_USB_ISO_IN				0x29
#define   CASCADE_USB_ISO_OUT				0x2a
#define   CASCADE_USB_MSC_CBW				0x2b
#define   CASCADE_USB_MSC_DATA_IN			0x2c
#define   CASCADE_USB_MSC_DATA_OUT			0x2d
#define   CASCADE_USB_MSC_CSW				0x2e
#define   CASCADE_USB_MSC_STALL				0x2f
#define   CASCADE_USB_INTERVAL				0x30
#define   CASCADE_USB_PROTOCOL_SETTING		0x31
#define   CASCADE_USB_ALTERNATE_SETTING		0x32
#define   CASCADE_USB_CONFIGURATION			0x33
#define   CASCADE_USB_MSC_DATA_OUT_ENABLE	0x34

#define   CASCADE_USB_DEVICE_SWITCH			0x3A

//#define   CASCADE_USB_SET_PROTOCOL			0x40

//KVM	 0x50~0x6f
#define   CASCADE_KVM_CMD					0x6f
#define   CASCADE_KVM_STATE					0x50
#define   CASCADE_KVM_KB_LED				0x51
#define   CASCADE_KVM_HOST_ACTIVE			0x52
#define   CASCADE_KVM_AUTOSCAN_INTERVAL		0x53
#define   CASCADE_KVM_AUTOSCAN_START		0x54
#define   CASCADE_KVM_AUTOSCAN_STOP			0x55
#define   CASCADE_KVM_HOST_JUMP				0x56
#define   CASCADE_KVM_BTN					0x57
#define   CASCADE_KVM_INTERVAL				0x58
#define   CASCADE_KVM_HOST_LED				0x59
#define   CASCADE_KVM_SELECT_LED			0x5A

#define   CASCADE_KVM_VGA_CONTROL			0x5F

//Other
#define   TRANSFER_DATA_END					0xA3
#define   CASCADE_SPEED_TEST				0x3A
//#define   CASCADE_USB_DESCRIPTOR_END		0xff

//----------------------------------------------------
//SPI 0xB00xBF
//SPI Wait Read
#define   CASCADE_SPI_READ					0xB0
#define   CASCADE_SPI_RSR					0xB0
#define   CASCADE_SPI_RDR					0xB1
#define   CASCADE_SPI_RBRS					0xB2
#define   CASCADE_SPI_DRMEM					0xB3
#define   CASCADE_SPI_DWRMEM				0xB4

//SPI Wait Write
#define   CASCADE_SPI_CMDR					0xBA
#define   CASCADE_SPI_DWMEM					0xBB

//Data Toggle define ----------------------------------
#define   RECEIVER_DATA_TOGGLE  	    	0xf0
#define   TRANSMITTER_DATA_TOGGLE       	0xf1
#define   TRANSMITTER_DATA_TOGGLE0      	0x00
#define   TRANSMITTER_DATA_TOGGLE1      	0x01
#define   RECEIVER_DATA_TOGGLE0      		0x00
#define   RECEIVER_DATA_TOGGLE1      		0x01

//-----------------------------------------------------
//$ CASCADE_Multi_TR_State DEFINE
//
#define	CASCADE_SEND_IDLE					0x00
#define	CASCADE_SEND_WAIT					0x01
#define	CASCADE_SEND_START					0x02
#define	CASCADE_RECEIVE_WAIT				0x04
#define	CASCADE_RECEIVE_START				0x08
#define	CASCADE_RECEIVE_NEXT				0x10

//-------------------------------------------------------
//$ Cascade_Receive_Cmd_Parser_State Define
//-------------------------------------------------------
#define CMD_PARSER_MASK						0xf0
#define CASCADE_CMD_TOKEN_LEN_SHORT			0x01
#define CASCADE_DATA_PACKET_WAIT			0x02
#define CASCADE_CMD_TOKEN_PARSER_ERR		0x08

#define CASCADE_CMD_TOKEN_CMD_OK			0x10
#define CASCADE_CMD_TOKEN_ADDR_OK			0x20
#define CASCADE_CMD_TOKEN_CRC_OK			0x40
#define CASCADE_CMD_TOKEN_PARSER_OK			0x80


//-------------------------------------------------------
//$ Cascade_Receive_Data_Parser_State Define
//-------------------------------------------------------
#define DATA_PACKET_PARSER_MASK				0xf0

#define CASCADE_DATA_PACKET_LEN_SHORT		0x01
#define CASCADE_DATA_SECTION_PARSER_OK		0x02
#define CASCADE_DATA_PACKET_PARSER_ERR		0x08

#define CASCADE_DATA_PACKET_TOKEN_OK		0x10
#define CASCADE_DATA_TOKEN_PARSER_OK		0x20
#define CASCADE_DATA_PACKET_CRC_OK			0x40
#define CASCADE_DATA_PACKET_SECTION_OK		0x80


#define CASCADE_PACKET_HEADER_LEN			0x06
//This is Transmitter Handle for Casecade Transmitter
typedef struct _Cascade_Transimit
{
	U8_T  	Control;						// 
#if (SYSTEM_CASCADE_MASTER)	
	U8_T  	Target;							// Command Target
#endif /* #if (SYSTEM_CASCADE_MASTER) */	

	U16_T 	DataLen;						// FIFO Data payload buffer length											
	
#if (SYSTEM_CASCADE_MASTER)		
	U8_T  	ErrCnt;							// Error counter	
	U16_T  	TimeOut;						// Transmitter time out define(ms)
#endif
	
	U8_T  	*Buf;							// Transmitter Buffer pointer			
} Cascade_Transimit_Def;

//This is Transmitter Handle for Casecade Transmitter
typedef struct _Cascade_API_Transimit
{
	U8_T  	Control;						// 
	U16_T 	DataLen;						// FIFO Data payload buffer length													
	U8_T  	*Buf;							// Transmitter Buffer pointer			
} Cascade_API_Transimit_Def;
 
typedef struct _Firmware_Version_Def
{
	U8_T  	Version[5];						// 
	U8_T  	Date[10];						// 	
} Firmware_Version_Def;
 
 
//-------------------------------------------------------
//$ bTarget_Addr Define
//-------------------------------------------------------
#define TARGET_ADDRESS_MASK 		0x7f
#define DATA_WAIT_ACCESS_MASK 		0x80
 
typedef struct _Cascade_Cmd_Token_Def
{
	U8_T  bSpi_Op;
	U8_T  bSalve_Addr;			
} Cascade_Cmd_Token_Def;

/*
$Task_State Define Table
*/
#define CASCADEM_MOUNT_START_MASK		0x01
#define CASCADEM_MOUNT_TYPE_MASK		0x02 //1-Mount 0-Umount

/*
$Task_State Define Table
*/
#define  QUEUE_DEVINX_MASK				0x0f //Contain 16 FIFO queue

#define  QUEUE_TYPE_MASK				0x80 //Contain 16 FIFO queue
#define  QUEUE_TYPE_MOUNT				0x80 //Contain 16 FIFO queue
#define  QUEUE_TYPE_UNMOUNT				0x00 //Contain 16 FIFO queue
//---------------------------------------------------------------
#define CASCADEM_DEVCIE_MOUNT			0x01 
#define CASCADEM_DEVCIE_UNMOUNT			0x00 

typedef struct _Cascade_Mount_Ctrl
{
	U8_T	Task_State;		//Bit0-Mount/Umount 
	         				
	         				
	U8_T	Task_ID;		//mount+/umount Task ID
	U8_T    Task_Slave; 	//Current processing Slave ID
	U8_T    Task_Devinx; 	//Current processing Slave ID
	U8_T    Task_DespInx; 	//Task DespInx
	//U8_T    Queue[16]; 		//Contain 16 FIFO queue
	//U8_T    QueueInx;  		//
	//U8_T	Host[USBDC_DEVICE_MAX][MAXIMUM_UNIT];	//Mount to which host?
	U8_T	State[USBDC_DEVICE_MAX][MAXIMUM_SLAVE];	//each bit statnd for the state of each device
	U16_T	idVendor[USBDC_DEVICE_MAX];		/* Vendor ID (Assigned by USB Org) */
	U16_T	idProduct[USBDC_DEVICE_MAX];	/* Product ID (Assigned by Manufacturer) */
} Cascade_Mount_Ctrl;

typedef struct _Cascade_System_State
{
	U8_T  Connect_State[2];
	U8_T  Suspend_State[2];
	U8_T  Resume_State[2];
	U8_T  Power_State[2];
	U8_T  KVM_State[2];		
	
	Cascade_Mount_Ctrl Mounted;
} Cascade_System_State;

typedef struct _Cascade_Data_Packet_Def
{
	//Cascade_Cmd_Token_Def Token;
	U8_T  bCommand;	
	U16_T iLen;	
	U8_T  bRequest;	
	U8_T  bIndex;	
	U8_T  bValue;		
} Cascade_Data_Packet_Def; 

typedef struct _CASCADE_PacketHeader_
{
	U8_T  bCommand;	
	U16_T iLen;	
	U8_T  bRequest;	
	U8_T  bIndex;	
	U8_T  bValue;
	U8_T  buf[1];	
} CASCADE_PacketHeader; 

typedef struct _Cascade_Device_Mapping_Def
{
	U8_T Mapping;
	U8_T UpperInfo;
	U8_T MountPort;
} Cascade_Device_Mapping_Def;	

typedef struct _Cascade_ISO_Data_Def
{
	Cascade_Data_Packet_Def	Token;
	U8_T  					Buf[USB_HC_ISTL_BUF_SIZE];							
} Cascade_ISO_Data_Def;

typedef struct _Cascade_GHID_Data_Def
{
	Cascade_Data_Packet_Def	Token;
	U8_T  					Buf[8];							
} Cascade_GHID_Data_Def;


typedef struct _Cascade_MSC_Buf_TypeDef
{
	U8_T					bufUsingFlag;
	U8_T					devIndex;
	U16_T					payloadLen;
	//U32_T					TotalLen;
	U8_T					reqType;
	Cascade_Data_Packet_Def	Data_Token;
#if (SYSTEM_CASCADE_MASTER)	
	//U8_T  					bTDID; //store the TD							
	U8_T  					bTD_Header[8]; //store the TD							
#endif
#if (SYSTEM_CASCADE_SLAVE)	
	U8_T  					bBuf[USB_HC_BULK_BUF_SIZE];							
#endif
} Cascade_MSC_Buf_TypeDef;

/* $_EXPORTED SUBPROGRAM SPECIFICATIONS_$ */
void  Cascade_Transmit_FIFO_Send(void);
void  Cascade_Core_Init(void);
void  Cascade_Transmit_Complete_Handle(void);
void  Cascade_Token_Transmit(U8_T *buf,U8_T contrl_attr,U16_T len,U16_T timeout,U8_T cmd_token_header);
void  Cascade_Retrive_Receive_Buf(U8_T *rbuf,U16_T len);
void  Cascade_Token_Header(U8_T *token,U8_T *buf);
void  Cascade_CRC_Calculate(U16_T start,U16_T offset,U16_T len,U8_T *buf,U8_T *crc,U8_T crctype,U8_T buftype);
void  Cascade_CopyRxDmaToBuf(U16_T start,U16_T len,U8_T *rbuf);
void  Cascade_Receive_Parser_State_Reset(void);
void  Cascade_Receive_Start_TimeOut_Check(void);
void  Cascade_Receive_Stop_TimeOut_Check(void);
void  Cascade_CopyDataFromRxBuffer(U16_T start,U16_T len,U8_T *rbuf);
void  Cascade_Active_Led_Control(U8_T led_control);
void  Cascade_Link_Led_Control(U8_T led_control);
void  Cascade_Host_Led_Control(U8_T led_control);
void  Cascade_HUB_Reset_Control(U8_T reset_control);
U8_T  Cascade_Receive_Cmd_Token_Check(U16_T len,Cascade_Cmd_Token_Def *cmd_token,U8_T update_flag,U8_T addr);
U16_T Cascade_Set_Data_Transmit_Header(U16_T datalen,U8_T *buf,U8_T *databuf);
CASCADE_PacketHeader *Cascade_BuildPacket(U8_T cmd, U8_T req, U8_T index, U8_T value, U8_T *pbuf, U16_T len);
void  Cascade_Compare_State(U8_T old_value,U8_T new_value);
U8_T  Get_Table_Bit(U8_T *tab,U8_T index);
void  Set_Table_Bit(U8_T *tab,U8_T index,U8_T value);

/* $_EXPORTED GLOBAL VARIABLES_$ */
extern const U16_T BIT_MASK16[];
extern  Cascade_Transimit_Def Trainsmit_FIFO[CASCADE_TRANSMIT_FIFO_MAX];
extern  Cascade_Data_Packet_Def	*Cascade_Data_TokenP;
extern  U8_T	*Cascade_Transmit_Send_Buf;
extern 	bit     Cascade_KVM_State_Changed_Flag;
#if (SYSTEM_CASCADE_MASTER)
extern	U8_T CascadeEdidPacket[CASCADE_PACKET_HEADER_LEN];
#endif
#if (SYSTEM_MSC_DEVICE_SUPPORT)
extern  Cascade_MSC_Buf_TypeDef	Cascade_MSC_BulkOutBuff[CASCADE_MSC_BUF_CNT];
extern  U8_T	Cascade_MSC_BulkOutHead;
extern  U8_T	Cascade_MSC_BulkOutTail;
extern  U8_T	Cascade_MSC_BulkOutCount;

extern  Cascade_MSC_Buf_TypeDef	Cascade_MSC_BulkInBuf[CASCADE_MSC_BUF_CNT];
extern  U8_T	Cascade_MSC_BulkInHead;
extern  U8_T	Cascade_MSC_BulkInTail;

extern  U8_T	Cascade_MSC_StallFlag;
extern  U8_T	Cascade_MSC_BurstWait;
extern 	bit		Cascade_BurstDataInFlowControl;
extern 	bit		Cascade_BurstDataOutFlowControl;
#endif

#endif /* #ifndef __CASCADE_H__ */