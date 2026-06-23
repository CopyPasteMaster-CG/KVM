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
 * Module Name: ps2_host.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __PS2_H__
#define __PS2_H__

/* INCLUDE FILE DECLARATIONS */
#include "system_cfg.h"
/* DEFINATION DECLARATIONS */


/**
  * @brief  Max Number of byte count for output buffer
  */
#define PS2_PORT_MAX_NUM				(SYSTEM_HARDWARE_PS2_PORT_MAX)
#define PS2_KB_OUT_BUF_MAX				32
#define PS2_MS_OUT_BUF_MAX				24
#define PS2_RECEIVE_BUF_MAX				64
#define PS2_RESEND_CODE					0xfe
#define PS2A							0
#define PS2B							1
#define PS2C							2
#define PS2D							3
/**
  * @brief  PS2_Keyboard_Tx_Control
  */
#define PS2_KB_TX_DISABLE_FLAG			0x01
/**
  * @brief  define the keyboard package type
  */
#define KB_TYPE_MAKE_F0					0x00
#define KB_TYPE_BREAK_F0				0x01
#define KB_TYPE_MAKE_E0					0x02
#define KB_TYPE_BREAK_E0				0x03
#define KB_TYPE_MAKE_E1					0x04
#define KB_TYPE_BREAK_E1				0x05
/** 
  * @brief  PS/2 Config : Max Number
  */
#define PS2_FIFO_QUEUE_MAX				16 // MUST be 8 based
#define PS2_RECEIVE_TIME_MAX			25 //25 ms for receive procedure
#define PS2_TRANSMITT_TIME_MAX			30 //25 ms for receive procedure
#define PS2_RECEIVE_ACT_TIME_MAX		5 // 3 ms for after detecting clock low
#define PS2_RECEIVE_ACT_TIME_BAT		1500 // 1000 ms for after send out 0xff
#define PS2_TRANSMITT_ACT_TIME_MAX		3 // 3 ms for after detecting clock low 
/** 
  * @brief  PS/2 Port Status
  */
#define PS2_ERROR_STATUS_MASK			0x0300
#define PS2_IDLE						0x0001
#define PS2_ACT							0x0002
#define PS2_WRITE_FLAG					0x0004
#define PS2_RECEIVE_COMPLETE_FLAG		0x0008
#define PS2_TRANSMIT_COMPLETE_FLAG		0x0010
#define PS2_TRANSMIT_START_FLAG			0x0020
#define PS2_TRANSMIT_RESEND_FLAG		0x0040 // send out a 0xfe to device
#define PS2_SCRIPT_START_FLAG			0x0080 // Used script to write out data
#define PS2_RECEIVE_RESEND_FLAG			0x0100 // ask for resend
// Error Status Flag 
#define PS2_PARITY_ERR_FLAG				0x0100
#define PS2_TIME_OUT_FLAG				0x0200
#define PS2_MOUSE_READY_FLAG			0x0400
#define PS2_KB_READY_FLAG				0x0800
#define PS2_DEVICE_KB					0x1000 // Keyboard Type
#define PS2_DEVICE_MS					0x2000 // Mouse Type
#define PS2_DEVICE_BAT_RECEIVED			0x4000 // Mouse Type
#define PS2_DISABLE_FLAG				0x8000
#define PS2_TRANSMIT_SCRIPT_MASK		PS2_TRANSMIT_TYPE_FLAG // By Script or Normal

/* ISR FIFO Buffer Depth */
#define HPS2_ISR_FIFO_DEPTH_MAX			8

typedef struct _PS2_ISR_TypeDef
{
	U8_T	PID;
	U8_T	State;
	U8_T	Data; // receive data Q
} PS2_ISR_TypeDef;

/** 
  * @brief  PS/2 Port Control Status
  */
#define PS2_NONE						0x00
#define PS2_MOUSE						0x01
#define PS2_KB							0x02
#define PS2_CONFIG_WAIT					0x04
#define PS2_CONFIG						0x08
/** 
  * @brief  PS2 PS2_ScriptFlowControl
  */
#define PS2_SCP_IDLE					0x00
#define PS2_SCP_PENDING					0x01
#define PS2_SCP_START					0x02
#define PS2_SCP_CMD						0x04
#define PS2_SCP_CMD_RPN					0x08
#define PS2_SCP_PARA1					0x10
#define PS2_SCP_PARA1_RPN				0x20
#define PS2_SCP_END						0x80
/** 
  * @brief  define Exti PS/2 Port
  */
typedef enum
{
#if (SYSTEM_HARDWARE_PS2_ENABLE)
	HPS2A = 0,
	HPS2B,
#endif
	EPS2A = SYSTEM_HARDWARE_PS2_PORT_MAX,
	EPS2B,
	EPS2C,
	EPS2D
} PS2Port_TypeDef;

/** 
  * @brief  PS2_Script_Check_Next_Operation 
  */
typedef enum
{
	Script_Cmd		= 0x01,
	Script_CmdRpn	= 0x02,
	Script_Para1	= 0x04,
	Script_Para1Rpn	= 0x08,
	Script_Para2	= 0x10,
	Script_Para2Rpn	= 0x20
} PS2ScriptTransmit_TypeDef;

/** 
  * @brief  Script Table Data Structure
  */
typedef struct _PS2_CfgScriptStruct
{
	U8_T	Cmd;
	U8_T	Cmd_RpnByteCnt;
	U8_T	Parameter1;
	U8_T	Para1_RpnByteCnt;
} PS2_CfgScriptStruct,*PS2_CfgScriptStructP;

/** 
  * @brief  Script Control Data Structure
  */
typedef struct _PS2_CfgCtlStruct
{
	U8_T	Script;			//0-type check,1-Cfg
	U8_T	*ScriptPtr;		//script table pointer
	U8_T	ScriptCnt;		//total command line quantity
	U8_T	Cmdindex;		//Command Index
	U8_T	RpnCnt;			//responsed counter
	U8_T	ResendCnt;		//responsed counter
	U8_T	Receive_Error;	//responsed error cndition, such like time out/pariry error
	U8_T	FlowControl;	//Current Timeout counter
} PS2_CfgCtlStruct,*PS2_CfgCtlStructP;

/** 
  * @brief  Script Table id
  */
#define TYPE_CHECK_SCRIPT				0x01
#define CONFIG_SCRIPT_MS				0x02
#define CONFIG_SCRIPT_KB				0x03
#define CONFIG_SCRIPT_KB_F3				0x04
#define CONFIG_SCRIPT_KB_LED			0x05
#define SCRIPT_RESEND					0x06
#define SCRIPT_ENABLE					0x07
#define EXTI_SCRIPT_DELAY				0x80


/* NAMING CONSTANT DECLARATIONS */


/* GLOBAL VARIABLES */
extern volatile U8_T	PS2_Clock_Cnt[PS2_PORT_MAX_NUM]; // PS2 port clock counter
#if (SYSTEM_EXTENDER_SUPPORT)
extern volatile U16_T	PS2_Status[PS2_PORT_MAX_NUM+2]; // PS2 Port Status
#else
extern volatile U16_T	PS2_Status[PS2_PORT_MAX_NUM]; // PS2 Port Status
#endif
#if (SYSTEM_EXTENDER_SUPPORT)
/* PS/2 Mouse Device Handle Buffer */
extern  U8_T	PS2_Mouse_Package_Index[PS2_PORT_MAX_NUM+2]; // Store the mouse device package index
extern  U8_T	PS2_Mouse_Package[PS2_PORT_MAX_NUM+2][4]; // Store the mouse device package Buffer, each mouse device has 4 byte buffer
/* PS/2 Keyboard Device Handle Buffer */
extern  U8_T	PS2_Keyboard_Package[PS2_PORT_MAX_NUM+2][5]; // Store the keyboard device package Buffer, each mouse device has 4 byte buffer
extern  U8_T	PS2_Keyboard_Package_Index[PS2_PORT_MAX_NUM+2]; // Store the keyboard device package index
#else
/* PS/2 Mouse Device Handle Buffer */
extern  U8_T	PS2_Mouse_Package_Index[PS2_PORT_MAX_NUM]; // Store the mouse device package index
extern  U8_T	PS2_Mouse_Package[PS2_PORT_MAX_NUM][4]; // Store the mouse device package Buffer, each mouse device has 4 byte buffer
/* PS/2 Keyboard Device Handle Buffer */
extern  U8_T	PS2_Keyboard_Package[PS2_PORT_MAX_NUM][5]; // Store the keyboard device package Buffer, each mouse device has 4 byte buffer
extern  U8_T	PS2_Keyboard_Package_Index[PS2_PORT_MAX_NUM]; // Store the keyboard device package index
#endif
extern volatile U8_T	PS2_Control[PS2_PORT_MAX_NUM]; // PS2 B port time out counter,unit ms
extern volatile U8_T	PS2_TASK_Timeout_ID[PS2_PORT_MAX_NUM]; // contain the ps2 time out task ID (IN Period_MS Table)
/*
  Script Control relative variable 
*/
extern U8_T		PS2_Mouse_Type[PS2_PORT_MAX_NUM]; // if the port is mouse, store the mouse type information
extern U8_T		PS2_Input_Buf[PS2_PORT_MAX_NUM]; // PS2 Input Buffer
extern U8_T		PS2_RQ_Cnt[PS2_PORT_MAX_NUM]; // the receive counter
extern U8_T		PS2_Receive_FIFO_Queue[PS2_PORT_MAX_NUM][PS2_FIFO_QUEUE_MAX]; // Store the ps2 intput into Q
extern U8_T		PS2_Transmit_Buf[PS2_PORT_MAX_NUM]; // PS2 Transmit Buffer
extern U8_T		TASK_PS2_TIMEOUT_CHECK_ID;
extern U8_T		PS2_ReceiveBuf[PS2_RECEIVE_BUF_MAX];
extern U8_T		PS2_Receive_RP[PS2_PORT_MAX_NUM],PS2_Receive_WP[PS2_PORT_MAX_NUM],PS2_Receive_Port[8];
extern U8_T		HPS2_ISR_FIFO_WP,HPS2_ISR_FIFO_RP;
extern U8_T		PS2_KB_Balance_Cnt[PS2_PORT_MAX_NUM]; // the receive counter

void PS2_TimeOut_Check(void);
U8_T PS2_FIFO_Pup(PS2Port_TypeDef);
void PS2_Transmit_Start_Bit(PS2Port_TypeDef);
void PS2_Script_Cfg_Start(PS2Port_TypeDef,PS2_CfgCtlStruct *);
//U8_T PS2_GenerateParity(U8_T);
void PS2_Script_Status_Complete_Handle(PS2Port_TypeDef);
U8_T PS2_Script_Cmd_Handle(PS2Port_TypeDef);
void PS2_Init(void);
void PS2_Script_Check_TimeOut(PS2Port_TypeDef PS2x);
void PS2_Script_F4_Enable(PS2Port_TypeDef PS2x);
//void PS2_Script_F5_Disable(PS2Port_TypeDef PS2x);
void PS2_Process_Mouse_Package(PS2Port_TypeDef,U8_T);
void PS2_Process_Keyboard_Package(PS2Port_TypeDef,U8_T);
extern void PS2_Keyboad_LED_Script(PS2Port_TypeDef,U8_T);
extern void TASK_PS2_TimeOut_Check(void);
extern void TASK_Event_PS2_Handle(void);
void PS2_FIFO_Push(PS2Port_TypeDef PS2x,U8_T PS2_Byte);
void PS2_Port_Recover(PS2Port_TypeDef PS2x);
void PS2H_Read_Regs(U8_T regIndex, U8_T *pRegData, U8_T regLen);
U8_T HOTKEY_Check_PS2_Key_Break(U8_T buftype);
#endif /* End of __PS2_H__ */

/* End of ps2_host.h */