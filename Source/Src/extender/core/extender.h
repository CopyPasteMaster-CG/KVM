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
 * Module Name: extender.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __EXTENDER_H__
#define __EXTENDER_H__

/* INCLUDE FILE DECLARATIONS 		  */
#define EXTENDER_PASSTHROUGH_BUF_MALLOC	  0
#define EXTENDER_TRANSACTION_SERIAL_ID	  0

/* NAMING CONSTANT DECLARATIONS */
#define    HUART_12M			0
#define    HUART_8M				1
#define    HUART_6M				2
#define    HUART_4M				3
#define    HUART_3M				4
#define    HUART_2M				5
#define    HUART_1M				6
#define    HUART_921K			7
#define    HUART_115K			8
#define    HUART_9600			9
#define    HUART_19200			10
#define    HUART_38400			11
#define    HUART_57600			12

#ifndef HUART_BAUD
#if (SYSTEM_EXTENDER_RS232_MODE)
	#ifdef BR19200
		#define    HUART_BAUD			HUART_19200
	#else
		#ifdef BR921K
			#define    HUART_BAUD			HUART_921K
		#else		
			#ifdef BR12M
				#define    HUART_BAUD			HUART_12M
			#else
				#ifdef BR6M
					#define    HUART_BAUD			HUART_6M
				#else
					#define    HUART_BAUD			HUART_115K
				#endif
			#endif
		#endif
	#endif
#else //For RS-485 Mode
	#ifdef BR19200
		#define    HUART_BAUD			HUART_19200
	#else
		#ifdef BR6M
			#define    HUART_BAUD			HUART_6M
		#else
			#ifdef BR8M
				#define    HUART_BAUD			HUART_8M
			#else
				#ifdef BR921K
					#define    HUART_BAUD			HUART_921K
				#else
					#define    HUART_BAUD			HUART_12M
				#endif
			#endif	
		#endif
	#endif
#endif
#endif

/* DEFINATION DECLARATIONS   		  */
//#define EXTENDER_DC_PULLUP_GPIO			P2_3
#define EXTENDER_DMA_LENGTH				24			//Support the maximun
#define EXTENDER_TRANSMIT_FIFO_MAX   	192			//Support the maximun
#define UR2_RX_TRIGGER_LEVEL			1024
#define UR2_RX_TIMEOUT_GAP				16			// 16 characters time out gap for receiver buffer ring
#define UR2_RX_BUF_START_ADDR			0x6000		/* The low byte of this start address must be 0x00 */
#define	UR2_RX_BUF_SIZE					(256 * 8)   //2048, The RX ring size must be a multiple of 256bytes */

#define EXTENDER_TRANSMITTER_ADDR       0x70
#define EXTENDER_RECEIVER_ADDR          0x7F

#define EXTENDER_PAGE_SIZ		        512
#define EXTENDER_SEND_MAX				5
#if (SYSTEM_EXTENDER_RS232_MODE)
#define EXTENDER_RECEIVE_TIME_OUT		3000
#else
#define EXTENDER_RECEIVE_TIME_OUT		3000
#endif
#define EXTENDER_HOST_PORT				0
//-------------------------------------
// Extender Transmitt FIFO structure
//-------------------------------------
#define EXTENDER_CONTROL_IDLE           	0x00
#define EXTENDER_CONTROL_TRANSMIT     		0x01
#define EXTENDER_CONTROL_TRANSMIT_DONE 		0x02
#define EXTENDER_CONTROL_TRANSMIT_NO_WAIT	0x04  //The transmit no need to wait
#define EXTENDER_CONTROL_NONE_FREE      	0x08 //buffer should not free after send complete
#define EXTENDER_CONTROL_DATA_FOLLOW	 	0x10  //The transmit no need to wait

#define EXTENDER_CONTROL_CLEAR_BUF		 	0x20  //The transmit need to clear the application buffer
//#define EXTENDER_CONTROL_URGENT			 	0x80  //The transmit must be send without overwrite

#define EXTENDER_SOF_ENQ_MAX				1


//COMMAND Define section ----------------------------
//#define   EXTENDER_TOKEN_MASK			0x0f

//#define   EXTENDER_OUT_TOKEN        	0xe1
#define   EXTENDER_INT_TOKEN       		0x69  //Interface token
#define   EXTENDER_SOF_TOKEN        	0xa5
#define   EXTENDER_CMD_TOKEN      		0x2d

#define   EXTENDER_DATA0_TOKEN      	0xc3
#define   EXTENDER_DATA1_TOKEN      	0x4b

#define   EXTENDER_ACK_TOKEN      		0xd2
#define   EXTENDER_NAK_TOKEN      		0x5a
#define   EXTENDER_STALL_TOKEN      	0x1e

//-------------------
//$Request Define
//-------------------
//System 0x00~0x0f
#define   EXTENDER_T_RESTART			0x01  //Transmitter side has been reset
#define   EXTENDER_T_PORT_REPORT		0x02  //Transmitter side port report
#define   EXTENDER_R_RESTART			0x03
#define   EXTENDER_R_MSC_REDEVINX		0x04
#define   EXTENDER_R_MSC_RESET_ENABLE	0x05
#define   EXTENDER_T_LED_CONTROL		0x09
#define   EXTENDER_T_DEVICE_LIST		0x0A
#define   EXTENDER_T_DEVICE_TABLE		0x0B
#define   EXTENDER_RTC_CONTORL			0x0C
//#define   EXTENDER_T_DEVICE_LIST		0x0B
//#define   EXTENDER_T_DEVICE_CLEAR		0x0C


//USB 0x10~0x3f
#define   EXTENDER_USB_MOUNT			0x10
#define   EXTENDER_USB_UNMOUNT			0x11
#define   EXTENDER_USB_DESCRIPTOR		0x12
#define   EXTENDER_USB_SETUP_STAGE		0x18
#define   EXTENDER_USB_DATA_IN_STAGE	0x19
#define   EXTENDER_USB_DATA_OUT_STAGE	0x1a
#define   EXTENDER_USB_SETUP_DONE		0x1c
#define   EXTENDER_USB_STATUS_OUT		0x1e
#define   EXTENDER_USB_STATUS_STALL		0x1f
#define   EXTENDER_USB_REMOUNT			0x20
#define   EXTENDER_USB_INTR_IN			0x21
#define   EXTENDER_USB_INTR_OUT			0x22
#define   EXTENDER_USB_ISO_IN			0x25
#define   EXTENDER_USB_ISO_OUT			0x26
#define   EXTENDER_USB_T_AUDIO_SW		0x27
#define   EXTENDER_USB_T_AUDIO_AUTOSW	0x28
#define   EXTENDER_USB_T_MSC_SW			0x29
#define   EXTENDER_USB_MSC_STALL		0x2A
#define   EXTENDER_USB_MSC_RESET		0x2B
#define   EXTENDER_USB_INTERVAL			0x2C
#define   EXTENDER_USB_VPID				0x2D


/* This section is for MSC define */
//if (command & EXTENDER_USB_MSC_CMD_MASK == EXTENDER_USB_MSC_CMD_MASK)
//then the crc check will be ingored
#define   EXTENDER_USB_CRC_SKIP_MASK	0x50

#define   EXTENDER_USB_MSC_DATA_IN		0x50
#define   EXTENDER_USB_MSC_DATA_OUT		0x51
#define   EXTENDER_USB_MSC_CBW			0x52
#define   EXTENDER_USB_MSC_CSW			0x53

#define   EXTENDER_USB_SET_PROTOCOL		0x40
#define   EXTENDER_USB_COMMAND			0x41
#define   EXTENDER_USB_MSC_IN_WAIT		0x42

//KVM 0x80~0x8f
#define   EXTENDER_KVM_EDID				0x80
#define   EXTENDER_KVM_KB_LED			0x81
#define   EXTENDER_KVM_T_PORT_SW		0x82    //Transmitter port switch
#define   EXTENDER_KVM_T_BUZZER			0x83    //Transmitter buzzer control
#define   EXTENDER_KVM_T_SYSTEM			0x84    //Transmitter buzzer control
#define   EXTENDER_KVM_T_AUDIO_SW		0x85    //Transmitter port switch
#define   EXTENDER_KVM_PS2_DATA			0x86    //PS2 data package,bValue=0-keyboard,1-mouse
#define   EXTENDER_KVM_MSX_SIZE			0x87    //Define the msc stoarge page sizeo for transmitter

//Other
#define   EXTENDER_DATA_END				0xA3
#define   EXTENDER_USB_DESCRIPTOR_END	0xff

//Data Toggle define ----------------------------------
//#define   RECEIVER_DATA_TOGGLE  	    0xf0
//#define   TRANSMITTER_DATA_TOGGLE     0xf1
#define   TRANSMITTER_DATA_TOGGLE0      0x00
#define   TRANSMITTER_DATA_TOGGLE1      0x01
#define   RECEIVER_DATA_TOGGLE0      	0x00
#define   RECEIVER_DATA_TOGGLE1      	0x01

//-----------------------------------------------------
//$ EXTENDER_Multi_TR_State DEFINE
//
#define	EXTENDER_SEND_IDLE				0x00
#define	EXTENDER_SEND_WAIT				0x01
#define	EXTENDER_SEND_START				0x02
#define	EXTENDER_RECEIVE_WAIT			0x04
#define	EXTENDER_RECEIVE_START			0x08
#define	EXTENDER_RECEIVE_NEXT			0x10

//-------------------------------------------------------
//$ Extender_Receive_Cmd_Parser_State Define
//-------------------------------------------------------
#define CMD_PARSER_MASK						0xf0
#define EXTENDER_CMD_TOKEN_LEN_SHORT		0x01
#define EXTENDER_DATA_PACKET_WAIT			0x02
#define EXTENDER_CMD_TOKEN_PARSER_ERR		0x08

#define EXTENDER_CMD_TOKEN_CMD_OK			0x10
#define EXTENDER_CMD_TOKEN_ADDR_OK			0x20
#define EXTENDER_CMD_TOKEN_CRC_OK			0x40
#define EXTENDER_CMD_TOKEN_PARSER_OK		0x80


//-------------------------------------------------------
//$ Extender_Receive_Data_Parser_State Define
//-------------------------------------------------------
#define DATA_PACKET_PARSER_MASK				0xf0

#define EXTENDER_DATA_PACKET_LEN_SHORT		0x01
#define EXTENDER_DATA_SECTION_PARSER_OK		0x02
#define EXTENDER_DATA_PACKET_PARSER_ERR		0x08

#define EXTENDER_DATA_PACKET_TOKEN_OK		0x10
#define EXTENDER_DATA_TOKEN_PARSER_OK		0x20
#define EXTENDER_DATA_PACKET_CRC_OK			0x40
#define EXTENDER_DATA_PACKET_SECTION_OK		0x80

typedef struct _Extender_Transimit
{
	U8_T  	Control;							// 
	U16_T 	DataLen;							// FIFO Data payload buffer length
	U8_T  	*Buf;								// Transmitter Buffer pointer											
	U8_T  	ErrCnt;								// Error counter
	U16_T  	TimeOut;							// Transmitter time out define(ms)	
} Extender_Transimit_Def;
 
 
//-------------------------------------------------------
//$ bTarget_Addr Define
//-------------------------------------------------------
#define TARGET_ADDRESS_MASK 		0x7f
#define DATA_WAIT_ACCESS_MASK 		0x80
 
typedef struct _Extender_Cmd_Token_Def
{
	U8_T  bCmd_Token;			
	U8_T  bTarget_Addr;	
	U8_T  bCmd_CRC;
	
} Extender_Cmd_Token_Def;

typedef struct _Extender_Data_Packet_Def
{
	//Extender_Cmd_Token_Def Token;
	U8_T  bData_Token;	
	U16_T iLen;	
	U8_T  bRequest;	
	U8_T  bIndex;	
	U8_T  bValue;		
#if (EXTENDER_TRANSACTION_SERIAL_ID)	
	//U8_T  bTid;		
#endif	
} Extender_Data_Packet_Def;

typedef struct _Extender_Data_Int_Packet_Def
{	
	//U8_T iLen;		
	U8_T bIndex;	
	U8_T bValue;		
} Extender_Data_Int_Packet_Def;

#define  NO_CTRL		0
#define  NO_INDEX		0
#define  NO_VALUE		0
#define  NO_LEN			0
#define  NO_DATA		NULL
#define  NO_HEADER		NULL

typedef struct _Extender_Cmd_Handle_Def
{
//	U8_T  bState;
//	U16_T iCur_Len;	
	U16_T iRemain_Len;	
	U16_T iRecBuf_Index;		/* the index of receive buffering */
	U16_T iData_Token_Index;		/* the index of receive buffering */
	U8_T  bData_CRC[2];	
	Extender_Cmd_Token_Def	 Cmd_Token;
	Extender_Data_Packet_Def Data_Token;
} Extender_Cmd_Handle_Def;

typedef struct _Extender_Packet_Header_Def
{
	Extender_Cmd_Token_Def	 Cmd_Token;
	Extender_Data_Packet_Def Data_Token;	
} Extender_Packet_Header_Def;

typedef struct _Extender_Packet_Int_Header_Def
{
	Extender_Cmd_Token_Def	 	 Cmd_Token;
	Extender_Data_Int_Packet_Def Data_Token;	
} Extender_Packet_Int_Header_Def;

#define EXTENDER_PACKET_HEADER_DATA_OFFSET	(sizeof (Extender_Packet_Header_Def))
#define EXTENDER_COMMAND_TOKEN_LEN			(sizeof (Extender_Cmd_Token_Def))

typedef struct _Extender_Device_Mapping_Def
{
	U8_T Mapping;
	U8_T UpperInfo;
} Extender_Device_Mapping_Def;	

#ifdef DEVICE_KEEP
typedef struct _Extender_Device_Keep_Def
{
	U16_T ivendor;
	U16_T iproduct;
} Extender_Device_Keep_Def;	
#endif /* #ifdef DEVICE_KEEP */

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
  #define EXTENDER_AUDIO_OUT_BUF_CNT			4
  #define EXTENDER_AUDIO_IN_BUF_CNT				4
  #define EXTENDER_AUDIO_OUT_BUF_MASK			(EXTENDER_AUDIO_OUT_BUF_CNT -1)
  #define EXTENDER_AUDIO_IN_BUF_MASK			(EXTENDER_AUDIO_IN_BUF_CNT -1)
 #if (SYSTEM_EXTENDER_RECEIVER)
  #define EXTENDER_AUDIO_OUT_BUF_SIZE			194	//(192 + 2)
  #define EXTENDER_AUDIO_IN_BUF_SIZE			204	//(192 + 2 + sizeof(Extender_Cmd_Token_Def) + sizeof(Extender_Data_Packet_Def) + 1 pad)
  #define EXTENDER_AUDIO_OUT_WATER_LEVEL		2
 #else // (SYSTEM_EXTENDER_TRANSMITTER)
  #define EXTENDER_AUDIO_OUT_BUF_SIZE			200	//(192 + 2 + sizeof(Extender_Data_Packet_Def))
  #define EXTENDER_AUDIO_IN_BUF_SIZE			194	//(192 + 2)
  #define EXTENDER_AUDIO_IN_WATER_LEVEL			2
 #endif
#endif

#if (SYSTEM_EXTENDER_MSC_SUPPORT)

#if (SYSTEM_USB_HC_BURST)
#if (SYSTEM_EXTENDER_RS232_MODE)
#define EXTENDER_MSC_BURST_SIZE	USB_MSC_CLASS_PAGE_SIZ //max burst data
#else //For RS-485 Mode
#define EXTENDER_MSC_BURST_SIZE	USB_MSC_CLASS_PAGE_SIZ //max burst data
#endif //#if (SYSTEM_EXTENDER_RS232_MODE)
#define EXTENDER_MSC_BUF_CNT	2
#define EXTENDER_MSC_BUF_SIZE	(EXTENDER_MSC_BURST_SIZE + 2) //(256+2), 256 burst data and the first 2 byte burst length
#endif //#if (SYSTEM_USB_HC_BURST)

#define EXTENDER_MSC_BUF_MASK	(EXTENDER_MSC_BUF_CNT -1)

typedef struct _Extender_MSC_Buf_TypeDef
{
	U8_T						bufUsingFlag;
	U8_T						devIndex;
	U16_T						payloadLen;
	U8_T						reqType;
	U8_T						serial;
	Extender_Cmd_Token_Def		Cmd_Token;
	Extender_Data_Packet_Def	Data_Token;
	U8_T						bBuf[EXTENDER_MSC_BUF_SIZE];
	U8_T						bCRC[2];
} EXTENDER_MSC_Buf_TypeDef;
extern U8_T						EXTENDER_MSC_Devinx;
extern U8_T						EXTENDER_MSC_StallFlag;

#if (!SYSTEM_EXTENDER_RECEIVER)
extern EXTENDER_MSC_Buf_TypeDef	EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BUF_CNT];
extern U8_T						EXTENDER_MSC_BulkOutHead;
extern U8_T						EXTENDER_MSC_BulkOutTail;
extern U8_T						EXTENDER_MSC_BulkOutCount;
#endif //#if (!SYSTEM_EXTENDER_RECEIVER)

extern EXTENDER_MSC_Buf_TypeDef	EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BUF_CNT];
extern U8_T						EXTENDER_MSC_BulkInHead;
extern U8_T						EXTENDER_MSC_BulkInTail;
extern U8_T						EXTENDER_MSC_BurstWait;
extern U16_T					EXTENDER_MSC_TimeOut;
#endif //#if (SYSTEM_EXTENDER_MSC_SUPPORT)

extern U8_T						EXTENDER_SofCount;


/* $_EXPORTED SUBPROGRAM SPECIFICATIONS_$ */
void Extender_Transmit_FIFO_Send(void);
void EXTENDER_Init(void);
void Extender_Transmit_Complete_Handle(void);
void TASK_EXTENDER_Transmit_TimeOut(void);
void Extender_Token_Transmit(U8_T *buf,U8_T contrl_attr,U16_T len,U16_T timeout,U8_T cmd_token_header);
void Extender_Retrive_Receive_Buf(U8_T *rbuf,U8_T len);
void EXTENDER_Transmit_Update(void); 
U8_T Extender_Receive_Cmd_Token_Check(U16_T len,Extender_Cmd_Token_Def *cmd_token,U8_T addr);
U8_T Extender_Receive_Data_Token_Parser(Extender_Cmd_Handle_Def *setup);
void EXTENDER_Update_Receive_Index(U16_T len);
void EXTENDER_CopyRxDmaToApp(U16_T start,U16_T appGetLen,U8_T *pAppData);
void Externder_Token_Header(U8_T *token,U8_T *buf);

U16_T Extender_Packet_Data(Extender_Data_Packet_Def *header,U16_T data_len,U8_T *data_buf,U8_T bRequest,U8_T bIndex,U8_T bValue);
void Externder_Packet_Header(Extender_Packet_Header_Def *header,U8_T *token);
void Externder_CRC_Calculate(U16_T start,U16_T len,U8_T *buf,U8_T *crc,U8_T buftype);
void EXTENDER_Get_Total_Receive_Len(void);
U16_T Extender_CopyRxDmaToBuf(U16_T start,U8_T len,U8_T *rbuf);
void Extender_Receive_Parser_State_Reset(void);
void Extender_Receive_Start_TimeOut_Check(void);
void Extender_Receive_Stop_TimeOut_Check(void);
void Extender_CopyDataFromRxBuffer(U16_T start,U16_T len,U8_T *rbuf);
void Extender_Active_Led_Control(U8_T led_control);
void Extender_Link_Led_Control(U8_T led_control);
void Extender_Host_Led_Control(U8_T led_control);
void Extender_HUB_Reset_Control(U8_T reset_control);
#ifdef EXTENDER_SOF_CNT_CHECK	
void EXTENDER_SOF_Check_Update(void);
#endif

#ifdef DEVICE_KEEP
void ExtenderR_Remote_Device_List_Create(void);
#endif

/* $_EXPORTED GLOBAL VARIABLES_$ */
extern  bit		EXTENDER_MSC_BurstOutWait_Flag;
extern 	U8_T 	EXTENDER_Transmit_OutP;
extern	U8_T 	EXTENDER_Transmit_InP;
extern  U8_T    Extender_Control_State;
extern  Extender_Transimit_Def Trainsmit_FIFO[EXTENDER_TRANSMIT_FIFO_MAX];
extern	U8_T 	EXTENDER_Transmit_TimeOut_TaskActiveID;
extern 	U8_T	rs485_RcvrBufRing[];
extern 	idata	U16_T	Receive_Buffer_Start;
extern 	idata	U16_T	Receive_Buffer_End;
extern 	idata	U16_T	Receive_Buffer_Cnt;
extern 	bit     Transmitter_Data_Toggle;  //The data toggle bit in transmitter side
extern	bit     Receiver_Data_Toggle;     //The data toggle bit in receiver side		
extern  U8_T	EXTENDER_Multi_TR_State;
extern 	U8_T	Extender_Passthrough_Buf[];
extern  U8_T	Extender_Receive_Data_Parser_State;
extern  U8_T    Extender_Receive_Cmd_Parser_State;
extern  Extender_Cmd_Token_Def Receive_Cmd_Token;  // last receive command token
extern  U8_T	Extender_Receive_Timeout_Active_ID;
extern  U8_T	Trainsmit_Table[EXTENDER_TRANSMIT_FIFO_MAX];
extern  U8_T    Extender_Current_TransId;

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
extern U8_T ETDR_AudioOutBuf[EXTENDER_AUDIO_OUT_BUF_CNT][EXTENDER_AUDIO_OUT_BUF_SIZE];
extern U8_T ETDR_AudioInBuf[EXTENDER_AUDIO_IN_BUF_CNT][EXTENDER_AUDIO_IN_BUF_SIZE];
 #if (SYSTEM_EXTENDER_TRANSMITTER)
extern U8_T ETDR_AudioOutBufAddr[EXTENDER_AUDIO_OUT_BUF_CNT][2];
extern U8_T ETDR_AudioInBufAddr[EXTENDER_AUDIO_IN_BUF_CNT][2];
extern U8_T ETDR_AudioOutTailSkipFlag;
extern U8_T ETDR_AudioInDcDoingFlag;
 #endif
 #if (SYSTEM_EXTENDER_RECEIVER)
extern U8_T ETDR_AudioOutHcDoingFlag;
extern U8_T ETDR_AudioInWaitFlag;
extern U8_T ETDR_AudioInTailSkipFlag;
 #endif
extern U8_T ETDR_AudioOutHead;
extern U8_T ETDR_AudioOutTail;
extern U8_T ETDR_AudioInHead;
extern U8_T ETDR_AudioInTail;
#endif

#if (SYSTEM_EXTENDER_MSC_SUPPORT)
void Extender_MSC_Init(U8_T keep_devinx);
#endif //#if (SYSTEM_EXTENDER_MSC_SUPPORT)

#endif /* End of extender.h */