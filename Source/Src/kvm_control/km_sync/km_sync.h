/*
 *********************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : km_sync.h
 * Purpose     : 
 * Author      : Jack Wang
 * Date        :
 * Notes       : 
 * no message
 *
 *================================================================================
 */
#ifndef __KM_SYNC_H
#define __KM_SYNC_H

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define API_ROAMING_TABLE_MAX			5
//This is for Absolute Mouse setting --------------
//#define POWER_LED_PIN		P2_5
#define X_MAX				VHID_X_MAX
#define Y_MAX				VHID_Y_MAX
#define RELATIVE_MODE		0
#define ABSOLUTE_MODE		1
#define X_LEFT_EDGE			0
#define X_RIGHT_EDGE		X_MAX
#define Y_TOP_EDGE			0
#define Y_BOTTOM_EDGE		Y_MAX
#define COORDINATE_COUNTER	1

#define EDGE_ROAMING_LEFT			0x01
#define EDGE_ROAMING_RIGHT			0x02
#define EDGE_ROAMING_TOP			0x04
#define EDGE_ROAMING_BOTTOM			0x08

#define SYNC_ROAMING_AUTO			0x01
#define SYNC_ROAMING_REPORT			0x00

#define KM_SYNC_REPEAT_SIZ	8
//------------------------------------------------- 
 
/* MACRO DECLARATIONS */
/* TYPE DECLARATIONS */
//-------------------------------------------------------------------------- 
// Structure declaration
//--------------------------------------------------------------------------
#define ROAMING_LEFT		0
#define ROAMING_RIGHT		1
#define ROAMING_UP			2
#define ROAMING_DOWN		3
#define ROAMING_TOP			2
#define ROAMING_BOTTOM		3

typedef struct _KM_SYNC_Roaming_Field {	
	U8_T Portid;	//Target portid	
	U8_T Latency;	//Mouse data package wait
} KM_SYNC_Roaming_Field;

typedef struct _KM_SYNC_Roaming_Direct {	
	KM_SYNC_Roaming_Field  Left;
	KM_SYNC_Roaming_Field  Right;
	KM_SYNC_Roaming_Field  Up;
	KM_SYNC_Roaming_Field  Down;
	void (* CallBack_FunctionP)(void); //user call back function pointer	
} KM_SYNC_Roaming_Direct;

typedef struct _KM_SYNC_Roaming {	
	KM_SYNC_Roaming_Direct  Item[4];	
} KM_SYNC_Roaming;

typedef struct _API_Roaming_Mapping_Def {		
	U8_T Sequence[4];
	KM_SYNC_Roaming Edge;	
} API_Roaming_Mapping_Def;
/* GLOBAL VARIABLES */
extern U8_T KM_SYNC_KbLed;
extern bit	KM_SYNC_HOTKEY_Skip_Flag;
extern U16_T MouseX;
extern U16_T MouseY;
extern U8_T  Mouse_Btn;
extern U8_T  MouseZ;
extern U8_T  *MouseXb,*MouseYb;	
extern U8_T  Coordinate_Cnt[4];
extern U8_T  Hid_Key_Parser_Flag;
extern U8_T  KM_SYNC_Sync_KB_HID_RepeatTableMake[KVM_MAX_PORT][8];
extern U8_T  KM_SYNC_Sync_KB_HID_RepeatTableBreak[8];
extern U16_T Coordinate_State;
extern bit	 KM_SYNC_HOTKEY_Clear_Flag;
extern U8_T  KM_Hotkey_Leading_Key;
extern U8_T  KM_SYNC_Sync_KB_HotkeyChar;
extern U8_T  TAKS_KM_SYNC_RepeatKeySend_ID;
extern U8_T  KM_SYNC_Sync_KB_Repeat;
extern U8_T  KM_SYNC_Sync_KB_RepeatActive[KVM_MAX_PORT];
extern U8_T  KM_SYNC_Sync_KB_RepeatTable[KM_SYNC_REPEAT_SIZ];
extern U8_T  KM_SYNC_Roaming_Edge;
extern U8_T  KM_SYNC_Roaming_Control;
extern U8_T  KM_SYNC_Roaming_DelayCnt;
extern API_Roaming_Mapping_Def  *Roaming[API_ROAMING_TABLE_MAX];
extern U8_T	Current_Roaming_Table; //default pointer 0
extern U8_T	Current_Roaming_Direction;
#ifdef VPID_CHANGER
extern U16_T VPID_Changer_Seed;
extern U8_T	 VPID_Changer_State;
#endif
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void KM_SYNC_Start(U8_T port);
void KM_SYNC_ClearStart(U8_T port);
void KM_SYNC_Init(void);
void KM_SYNC_ModeAccross_Control(U8_T ctrl);
void KM_SYNC_MS_Hotkey_Check(U8_T ms_btn);
void KM_SYNC_KB_Hotkey_Check(U8_T keytype,U8_T keycode);
void KM_SYNC_KM_SyncModeLed(U8_T led_ctrl);
void KM_SYNC_Select_Led_Control(U8_T port);
void KM_SYNC_KM_AcrossModeLed(U8_T port);
U8_T KM_SYNC_Port_Jump_Power(U8_T current_port,U8_T jumptype,U8_T call_fun);
void KM_SYNC_Abs_Mouse_Coordinate_Convert(U8_T *datacode);
void KM_SYNC_Mouse_Jump_State_Reset(void);
U8_T KM_SYNC_KB_Repeat_Active(U8_T port,U8_T usbcode);
void KM_SYNC_KB_Clear_Active(U8_T port,U8_T usbcode);
void KM_SYNC_Send_KB_Hid_Key(U8_T port,U8_T usbcode);
void KM_SYNC_Check_HID_Repeat_State(U8_T port);
void KM_SYNC_Repeat_Active_Force_Stop(U8_T devinx);
void KM_SYNC_Send_RepeatKey_Check(U8_T port);
void KM_SYNC_ModeSync_Control(U8_T syncmode);
void KM_SYNC_Mouse_Absolute_Mode(void);
void KM_SYNC_Mouse_Relative_Mode(void);
void KM_SYNC_Clear_Repeat_Table(void);
U8_T KM_SYNC_Search_Repeat_Table(U8_T table,U8_T keycode);
#if (KVM_HOTKEY_SUPPORT==0)
void HOTKEY_Handle(U8_T keytype,U8_T keycode);
#endif /* #if (KVM_HOTKEY_SUPPORT==0) */
void KM_SYNC_Send_VHID_Key(U8_T keytype,U8_T keycode);

#ifdef VPID_CHANGER
void KM_Sync_VPid_Change_Auto(U8_T port);
#endif /* #ifdef VIP_CHANGER */
U8_T KM_SYNC_Port_Setting(U8_T p);
U8_T KM_SYNC_SwitchMode_State(void);
U8_T KM_SYNC_SyncMode_State(void);
void KM_SYNC_ModeSwitch_Control(U8_T mode);
#endif /* End of __KM_SYNC_H */

