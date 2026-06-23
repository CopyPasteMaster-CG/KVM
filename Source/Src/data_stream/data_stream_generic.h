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
 * Module Name: data_stream_generic.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __DATA_STREAM_GENERIC_H__
#define __DATA_STREAM_GENERIC_H__

#define  GENERIC_KB_NOT_FOUND         0xff
#define  HID_KEY_KEYCODE_MASK			0x01		
#define  HID_KEY_COORDINATE_MASK		0x02		
#define  HID_KEY_SYSTEM_MASK			0x04		
#define  HID_KEY_CONSUMER_MASK			0x08		
#define  HID_KEY_BUTTON_MASK			0x10

#define  X_LEFT_MASK					0x0001
#define  X_RIGHT_MASK					0x0002
#define  Y_TOP_MASK						0x0004
#define  Y_BOTTOM_MASK					0x0008
#define  MOUSE_JUMP_MASK				0x0010
#define  MPASS_MODE_MASK				0x0020
#define  ACROSS_LEFT_MASK				0x0040
#define  ACROSS_RIGHT_MASK				0x0080
#define  ACROSS_TOP_MASK				0x0100
#define  ACROSS_BOTTOM_MASK				0x0200
#define  ROAMING_REPORT_MASK			0x0400

/* INCLUDE FILE DECLARATIONS */
/* DEFINATION DECLARATIONS */
#define  HID_USAGE_PAGE_07_MAKE       0x00   // Normal key make
#define  HID_USAGE_PAGE_07_BREAK      0x01
#define  MODIFIER_MAKE                0x02
#define  MODIFIER_BREAK               0x03
#define  HID_USAGE_PAGE_01_MAKE       0x04  // System Key make(Power,Sleep,Wakeup)
#define  HID_USAGE_PAGE_01_BREAK      0x05
#define  HID_USAGE_PAGE_0C_MAKE       0x06  // Multi-Media Key 
#define  HID_USAGE_PAGE_0C_BREAK      0x07

#define  EXTENDER_PS2_KEYBOARD		  0x00	
#define  EXTENDER_PS2_MOUSE			  0x01	
/*-------------------------------------------------------*/
#ifdef SYNC
#define  DATAST_GENERIC_KB_IN_MAX     64
#else
#define  DATAST_GENERIC_KB_IN_MAX     128
#endif
#define  DATAST_GENERIC_MS_IN_MAX     4


//$$Queue Format:
//    Byte 0:Bit 7  -> 1=USB, 0=PS/2 
//    Byte 0:Bit 6~4-> USB key type|PS/2 key type
//           Bit 3~0-> Length
//
//
#define  DATAST_PACKAGE_DATA_TYPE    0x80
#define  DATAST_PACKAGE_DATA_USB     0x80
#define  DATAST_PACKAGE_DATA_PS2     0x00

#define  DATAST_PACKAGE_CODE_TYPE        0x70
#define  GENERIC_USAGE_07_MAKE           0x00  // Normal key make
	#define  GENERIC_USAGE_07_BREAK      0x10
	#define  GENERIC_MODIFIER_MAKE       0x20
	#define  GENERIC_MODIFIER_BREAK      0x30
	#define  GENERIC_USAGE_01_MAKE       0x40  // System Key make(Power,Sleep,Wakeup)
	#define  GENERIC_USAGE_01_BREAK      0x50
	#define  GENERIC_USAGE_0C_MAKE       0x60  // Multi-Media Key 
	#define  GENERIC_USAGE_0C_BREAK      0x70
	#define  GENERIC_USAGE_ABS_MAKE      0x80  // Absolute Mouse Data

	#define  GENERIC_PS2_MAKE_F0         0x00
	#define  GENERIC_PS2_BREAK_F0        0x10
	#define  GENERIC_PS2_MAKE_E0         0x20
	#define  GENERIC_PS2_BREAK_E0        0x30
	#define  GENERIC_PS2_MAKE_E1         0x40
	#define  GENERIC_PS2_BREAK_E1        0x50

#define  DATAST_PACKAGE_LENGTH_MASK      0x0f
/*-------------------------------------------------------*/
extern U8_T TASK_DATAST_PS2_Generic_KB_ID;
extern U8_T DATAST_Generic_USB_MS_Report[6]; /* The first byte indicate the buffer is valid */

#ifdef SYNC
extern U8_T DATAST_Generic_KB_IN_WP[KVM_MAX_PORT],DATAST_Generic_KB_IN_RP[KVM_MAX_PORT];  /* Byte1-Modifiers,Byte2-Reverse,Byte3~8=>Data */
#if (DATAST_GENERIC_MS_IN_QUEUE)
extern U8_T DATAST_Generic_MS_IN_WP,DATAST_Generic_MS_IN_RP[8];  /* Byte1-Modifiers,Byte2-Reverse,Byte3~8=>Data */
#else
extern U8_T DATAST_Generic_MS_IN_WP[KVM_MAX_PORT],DATAST_Generic_MS_IN_RP[KVM_MAX_PORT];  /* Byte1-Modifiers,Byte2-Reverse,Byte3~8=>Data */
#endif
#else
extern U8_T DATAST_Generic_KB_IN_WP,DATAST_Generic_KB_IN_RP;  /* Byte1-Modifiers,Byte2-Reverse,Byte3~8=>Data */
extern U8_T DATAST_Generic_MS_IN_WP,DATAST_Generic_MS_IN_RP;  /* Byte1-Modifiers,Byte2-Reverse,Byte3~8=>Data */
#endif

extern bit  DATAST_Generic_KB_Change_Flag;
extern U8_T TASK_DATAST_PS2_Generic_KB_ActiveFlag;
extern U8_T TASK_DATAST_PS2_Generic_MS_ActiveFlag;
//extern bit  DATAST_Generic_MS_Change_Flag;
extern U8_T TASK_DATAST_PS2_Generic_KB_ID;
extern U8_T TASK_DATAST_PS2_Generic_MS_ID;
extern U8_T TASK_DATAST_PS2_Generic_MS_ID2;
extern U8_T TASK_DATAST_USB_Virtual_HID_ID;
extern U16_T MouseX,MouseY;
extern U8_T  Hid_Key_Parser_Flag;


extern void DATAST_PS2_Convert_To_USB(U8_T,U8_T,U8_T *,U8_T *,U8_T *);
extern U8_T DATAST_TranslateE0(U8_T);
extern U8_T DATAST_TranslateF0(U8_T);
extern void DATAST_PS2_KB_Handle(U8_T,U8_T,U8_T *);
extern void DATAST_Init(void);
#ifdef SYNC
extern void DATAST_Append_Generic_KB_In_Queue(U8_T,U8_T,U8_T);
#else
extern void DATAST_Append_Generic_KB_In_Queue(U8_T,U8_T);
#endif
extern void TASK_DATAST_PS2_Generic_KB_Output_Handle(void);
extern U8_T DATAST_Generic_MS_Port_Mask[DATAST_GENERIC_MS_IN_MAX];


void DATAST_Reset_GenericQueue(void);
void DATAST_Check_Generic_KB_Active_Task(U8_T);
void DATAST_Check_Generic_MS_Active_Task(U8_T);
void DATAST_PS2_MS_Handle(U8_T data_type);
void DATAST_Append_Generic_MS_IN_Q(U8_T datatype,U8_T *datacode);
void TASK_DATAST_PS2_Generic_MS_Output_Handle(void);
void DATAST_PS2_Generic_Dumy_USB(U8_T intf);
U8_T DATAST_Generic_Mouse_Data(USB_MS_Data_TypeDef *ms_data,U8_T *buf_type,U8_T mouse_type);
void DATAST_Keyboard_Send(U8_T port,U8_T usb_keytype,U8_T usb_keycode);
#endif /* End of __DATA_STREAM_H__ */

/* End of data_stream_generic.h */