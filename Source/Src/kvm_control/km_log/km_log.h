/*
 *********************************************************************************
 *     Copyright (c) 2018	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : km_log.h
 * Purpose     : 
 * Author      : Jack Wang
 * Date        :
 * Notes       : 
 * no message
 *
 *================================================================================
 */
#ifndef __KM_LOG_H
#define __KM_LOG_H

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define MAX_TX_REMOTE_BUF_SIZE	512
#define MAX_RX_REMOTE_BUF_SIZE	256

#define MAX_TX_HSUART_MASK		(MAX_TX_REMOTE_BUF_SIZE - 1)
#define MAX_RX_HSUART_MASK		(MAX_RX_REMOTE_BUF_SIZE - 1)
#define MAX_RX_REMOTE_Q_SIZE	32
#define MAX_TX_REMOTE_Q_SIZE	32

#define REMOTE_CONSOLE_WAIT		0
#define REMOTE_CONSOLE_SEND		1
#define REMOTE_CONSOLE_SCREEN_2_WINDOWS   0
#define REMOTE_CONSOLE_SCREEN_1_WINDOWS   1

#define ROAMING_LEFT			0
#define ROAMING_RIGHT			1
#define ROAMING_TOP				2
#define ROAMING_BOTTOM			3

/*---------------------------------------------
$ bType
*/
#define KMLOG_KEY_MAKE				0x00
#define KMLOG_KEY_BREAK				0x01
/*
#define  HID_USAGE_PAGE_07_MAKE       0x00   // Normal key make
#define  HID_USAGE_PAGE_07_BREAK      0x01
#define  MODIFIER_MAKE                0x02
#define  MODIFIER_BREAK               0x03
#define  HID_USAGE_PAGE_01_MAKE       0x04  // System Key make(Power,Sleep,Wakeup)
#define  HID_USAGE_PAGE_01_BREAK      0x05
#define  HID_USAGE_PAGE_0C_MAKE       0x06  // Multi-Media Key 
#define  HID_USAGE_PAGE_0C_BREAK      0x07
*/
/*---------------------------------------------
$ bUsagePage
*/
#define KMLOG_KEY_USAGE_SYSTEM		0x01
#define KMLOG_KEY_USAGE_NORMAL		0x07
#define KMLOG_KEY_USAGE_CONSUMER	0x0c


//-----------------------------------------------
typedef struct _KmLog_Kb_Def
{
	U8_T  bType; 			//0-make,1-break
	U8_T  bUsagePage;		//01,07,0c
	U8_T  bH_Byte;			//High byte for keycode
	U8_T  bL_Byte;			//Low byte for keycode
} Kmlog_Kb_Packet_Def;

typedef struct _KmLog_Ms_Def
{	
	U8_T  bButton; 		//0-make,1-break
	U8_T  bX;			//
	U8_T  bY;			//
	U8_T  bZ;			//
} Kmlog_Ms_Packet_Def;

typedef struct _KmLog_MsA_Def
{	
	U8_T  bButton; 		//0-make,1-break
	U16_T iX;			//
	U16_T iY;			//
	U8_T  bZ;			//
} Kmlog_MsA_Packet_Def;
 
/*---------------------------------------------
$ bDataType
*/
#define KMLOG_HEADER0				0xfa
#define KMLOG_TYPE_KB				0x01
#define KMLOG_TYPE_MS				0x02
#define KMLOG_TYPE_SYSTEM			0x03
#define KMLOG_TYPE_MSA				0x04
#define KMLOG_TYPE_REPEAT_KEY		0x05

typedef struct _KmLog_Packet_Def
{	
	U8_T  bHeader; 		//fixed for 0xfa 	
	U8_T  bDataType;  	//				
	U8_T  bPayload[6];  //			
} Kmlog_Data_Packet_Def;



/*---------------------------------------------
$ bRequest
*/
#define KMLOG_REQ_RESET				0x01
//#define KMLOG_REQ_KBLED				0x02
#define KMLOG_REQ_KBLED_STATE		0x02
#define KMLOG_KBLED_SET				0x03
#define KMLOG_REQ_HOTKEY			0x04
	#define KMLOG_HOTKEY_ABSOLUTE		0x01
	#define KMLOG_HOTKEY_RELATIVE		0x02
	#define KMLOG_HOTKEY_STORAGE		0x03
	#define KMLOG_HOTKEY_RECORD_START	0x04
	#define KMLOG_HOTKEY_RECORD_STOP	0x05
	#define KMLOG_HOTKEY_PLAY_START		0x06
	#define KMLOG_HOTKEY_PLAY_STOP		0x07
	#define KMLOG_HOTKEY_STOP			0x08
	#define KMLOG_HOTKEY_FUN9			0x09
	#define KMLOG_HOTKEY_FUNA			0x0a

#define KMLOG_REQ_DEVICE_STATE		0x05
#define KMLOG_REQ_VERSION_NUMBER	0x06
#define KMLOG_REQ_VERSION_DATE		0x07
#define KMLOG_MONITOR_EDGE			0x08
#define KMLOG_HOST_STATE			0x09
#define KMLOG_HOST_CONNECT_SET		0x09
#define KMLOG_LED_SET				0x0A
#define KMLOG_GET_MOUSE_ABS			0x0B
#define KMLOG_BUTTON_STATE			0x0B
#define KMLOG_MOUSE_MODE_SET		0x0C
#define KMLOG_MOUSE_ROAMING_REPORT	0x0C
#define KMLOG_CLS_ROAMING_EVENT		0x0D

typedef struct _KmLog_Sys_Def
{		
	U8_T  bRequest;		//01-Reset,02-KB Led	
	U8_T  bValue[3];
} Kmlog_Sys_Packet_Def;

/*
$$ OpCode define 
*/
#define REMOTE_ACK		0x06
#define REMOTE_NAK		0xF9

typedef struct _KMLog_Format
{
	U8_T	Start;  //0xff
	U8_T    OpCode; //Operation function code
	U8_T    Payload[9];
	U8_T    CRC[2];
} KMLog_Format_TypeDef;

typedef struct _Keyboard_Format
{
	U8_T	Modifier;	
	U8_T	Key[8];	
} Keyboard_Format_TypeDef;

typedef struct _Mouse_Format
{
	U8_T	Btn;	
	U8_T	X;	
	U8_T	Y;	
	U8_T	Z;	
} Mouse_Format_TypeDef;

typedef struct _KMLog_Fun_Handle
{		
	void   (* HandleFunction)(void);	
} KMLog_Fun_Handle_TypeDef;

/* GLOBAL VARIABLES */
extern U8_T		hsuart_TxBuf[];
extern U16_T	hsuart_TxHead;
extern U16_T	hsuart_TxTail;
extern volatile U16_T	hsuart_TxCount;
extern U8_T		hsuart_TxFlag;
extern U8_T		hsuart_RxBuf[];
extern U16_T	hsuart_RxHead;
extern U16_T	hsuart_RxTail;
extern U16_T	hsuart_RxCount;
extern bit		KMLog_R_Queue;
extern U8_T		OperationMode;
extern U8_T		KMLog_Hotkey[3];
extern U16_T	AutoMode_Timer; //5 Seconds
extern U8_T		KMLog_TASK_Send_Hotkey_ID;
extern U8_T		KMLog_TxBuf[MAX_TX_REMOTE_Q_SIZE];
extern U8_T		KM_Host_Connect_State;
extern Kmlog_Data_Packet_Def	*Receive_Data;
extern Kmlog_Data_Packet_Def	Km_Data;
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void KMLog_Recieve_Handle(void);
void HSUR2_IntrEntryFunc(void);
S8_T HSUART_PutChar(S8_T c);
void KMLOG_Init(void);
void HOTKEY_Handle(U8_T keytype,U8_T keycode);
void KMLog_Send_Keyboard(U8_T datatype,U8_T datausage,U16_T datacode);
void KMLog_Send_Mouse_Relative(U8_T *msdata);
void KMLog_Send_System(U8_T *buf,U8_T funcode,U8_T subfuncode,U8_T payload,U8_T payload1);
void TASK_KMLog_Send_Hotkey_Fork(U8_T fun,U8_T payload);
void KMLog_Send_Monitor_Edge_Jump(U8_T jumpdirection);
U8_T KM_Check_Skip_Condition(void);
void KMLog_Send_Host_State(void);
void KMLog_Send_Button_State(U8_T btnstate);
void KMLog_Send_Roaming_Report(U8_T port,U8_T direction);
void KMLog_Handle_Host_Connect_Set(void);
void KMLog_Send_Mouse_Absolute(void);
BOOL REMOTE_PutStr(char *str,U8_T len);
#endif /* End of __KM_LOG_H */

