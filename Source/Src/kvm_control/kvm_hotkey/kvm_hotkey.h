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
 * Module Name : kvm_hotkey.h
 * Purpose     : A header file of KVM Hotkey handle function
 * Author      : Jack Wang
 * Date        :
 * Notes       : 
 * no message
 *
 *================================================================================
 */
#ifndef __KVM_HOTKEY_H
#define __KVM_HOTKEY_H

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
 
/* MACRO DECLARATIONS */

/* TYPE DECLARATIONS */
typedef enum
{
	HOTKEY_MAKE     		= 0x00,
	HOTKEY_BREAK    		= 0x01,
	HOTKEY_MODIFIER_MAKE 	= 0x02,
	HOTKEY_MODIFIER_BREAK 	= 0x03,
	HOTKEY_ACTIVE   		= 0xf0,
	HOTKEY_FUNCTION 		= 0xf1,
	HOTKEY_TERMINATE		= 0xff
} Hotkey_TypeDef;

typedef enum
{
	FUNCTION_KEY    = 0x01,
	FUNCTION_VALUE  = 0x02,
} Function_TypeDef;

//typedef struct _KVM_FunctionStruct
//{
//	U8_T  Type;    // Keycode,Value
//	U8_T  FunctionID;
//	U8_T  Value;
//	U8_T  Parameter0;
//	U8_T  Parameter1;
//	U8_T  Parameter2;
//	U8_T  Parameter3;
//	U8_T  Parameter4;
//} KVM_FunctionStruct,*PKVM_FunctionStruct;

typedef struct _KVM_HotkeyStruct
{
	U8_T  Keytype;
	U8_T  Keycode;
} KVM_HotkeyStruct,*PKVM_HotkeyStruct;

/*-----------------------------------------------------------------------------------*/
typedef struct
{
	U8_T 	UsagePage;							// Task type
	U8_T 	CodeLSB;							// Task ID from 0~255
	U8_T 	CodeMSB;							// Task transfer parameter
} PS2_2_USB_MultiMediaTypeDef;
/*-----------------------------------------------------------------------------------*/
#define HOTKEY_FUN_ENABLE							0x01
#define HOTKEY_FUN_DISABLE							0x00

/* Define the Function ID */
#define HOTKEY_OPCODE_MASK						0x1f
#define HOTKEY_FUN_PORT_SW_NUMERIC					0x01
#define HOTKEY_FUN_PORT_SW_NONUM					0x02
#define HOTKEY_FUN_USBAUDIO_SW						0x03
#define HOTKEY_FUN_AUDIO_SW							0x04
#define HOTKEY_FUN_BUZZER_CONTROL_TOGGLE			0x05
#define HOTKEY_FUN_AUTOSCAN_CONTROL					0x06
#define HOTKEY_FUN_AUTOSCAN_INTERVAL				0x07
#define HOTKEY_FUN_HUB_SW							0x08
#define HOTKEY_FUN_PORT_RESET						0x09
#define HOTKEY_FUN_AUTOAUDIO_SW						0x0A
#define HOTKEY_FUN_MSC_SW							0x0B
#define HOTKEY_FUN_FLASH_DEFAULT					0x0C
#define HOTKEY_FUN_EDID_REREAD						0x0D
#define HOTKEY_FUN_RAPID_SW							0x0E
#define HOTKEY_FUN_PEN_CONTROL						0x0F
#define HOTKEY_FUN_PEN_SW							0x10
#define HOTKEY_FUN_MSC_CONTRL						0x11
#define HOTKEY_FUN_USB_AUDIO_SW						0x12
#define HOTKEY_FUN_PLUGIN_JUMP_CONTROL				0x13
#define HOTKEY_FUN_PORT_JUMP_POWER_CONTROL			0x14
#define HOTKEY_FUN_POWER_SAVING_CONTROL				0x15
#define HOTKEY_FUN_PLUGIN_JUMP_MODE					0x16
#define HOTKEY_FUN_USBAUDIO_REPORT_CONTROL			0x17
#define HOTKEY_FUN_UPDATE_FIRMWARE					0x20

#define HOTKEY_FUN_TRANSMITTER_PORT_SW				0x18
#define HOTKEY_FUN_R_DELAY_PLUS						0x19
#define HOTKEY_FUN_G_DELAY_PLUS						0x1A
#define HOTKEY_FUN_B_DELAY_PLUS						0x1B
#define HOTKEY_FUN_R_DELAY_MINUS					0x1C
#define HOTKEY_FUN_G_DELAY_MINUS					0x1D
#define HOTKEY_FUN_B_DELAY_MINUS					0x1E

#define HOTKEY_FUN_TX_BUZZER_CONTROL				0x1F
#define HOTKEY_FUN_T_USBAUDIO_SW					0x21
#define HOTKEY_FUN_T_USBAUDIO_AUTOSW				0x22
#define HOTKEY_FUN_T_MSC_SW							0x23
#define HOTKEY_FUN_ALTERNAMTE_CONTROL				0x24
#define HOTKEY_FUN_RTC_CONTROL						0x25
#define HOTKEY_FUN_MS_SWITCH						0x26

#define HOTKEY_FUN_MS_ROAMING_MODE					0x27
#define HOTKEY_FUN_MS_SYNC_MODE						0x28
#define HOTKEY_FUN_MS_RELATIVE_MODE					0x29
#define HOTKEY_FUN_MS_ABSOLUTE_MODE					0x30

#define HOTKEY_FUN_ALL_SYNC							0x2D
#define HOTKEY_FUN_KB_SYNC							0x2E
#define HOTKEY_FUN_MS_SYNC							0x2F



#define HOTKEY_OPCODE_LEN_MASK					0xe0
#define HOTKEY_OPCODE_LEN_SHIFT					>> 5

#define HOTKEY_LEN_1								0x20
#define HOTKEY_LEN_2								0x40
#define HOTKEY_LEN_3								0x60
#define HOTKEY_LEN_4								0x80
#define HOTKEY_LEN_5								0xA0
#define HOTKEY_LEN_6								0xC0
#define HOTKEY_LEN_7								0xE0

/* Define the Operation ID */
#define HOTKEY_OP_FUNCTION0_CODE					0x00 // code is and
#define HOTKEY_OP_FUNCTION1_CODE					0x01 // code is and
#define HOTKEY_OP_FUNCTION2_CODE					0x02 // code is and
#define HOTKEY_OP_FUNCTION3_CODE					0x03 // code is and
#define HOTKEY_OP_FUNCTION4_CODE					0x04 // code is and

#define HOTKEY_OP_SHORT_VALUE						0x05
#define HOTKEY_OP_LONG_VALUE						0x06
#define HOTKEY_OP_KEY_TO_VALUE						0x07

#define HOTKEY_OP_FUNCTION5_CODE					0x08 // code is or

#define HOTKEY_OP_TERNIMATE							0x1e
#define HOTKEY_FUN_TERNIMATE						0x1f

//--------------------------------------------------------
//$ HOTKEY_Parser_State
#define HOTKEY_PARSER_IDLE							0x00
#define HOTKEY_PARSER_FUNCTION_CODE_STAGE			0x01
#define HOTKEY_PARSER_OPCODE_STAGE					0x02
#define HOTKEY_PARSER_OPCODE_DONE					0x03
#define HOTKEY_PARSER_OP_CONTENT_STAGE				0x04
#define HOTKEY_PARSER_OP_TERNIMATE_STAGE			0x05

//--------------------------------------------------------
/* Define the keycode for CHARACTER */
#define CHAR_A										0x04
#define CHAR_B										0x05
#define CHAR_C										0x06
#define CHAR_D										0x07
#define CHAR_E										0x08
#define CHAR_F										0x09
#define CHAR_G										0x0a
#define CHAR_H										0x0b
#define CHAR_I										0x0c
#define CHAR_J										0x0d
#define CHAR_K										0x0e
#define CHAR_L										0x0f
#define CHAR_M										0x10
#define CHAR_N										0x11
#define CHAR_O										0x12
#define CHAR_P										0x13
#define CHAR_Q										0x14
#define CHAR_R										0x15
#define CHAR_S										0x16
#define CHAR_T										0x17
#define CHAR_U										0x18
#define CHAR_V										0x19
#define CHAR_W										0x1a
#define CHAR_X										0x1b
#define CHAR_Y										0x1c
#define CHAR_Z										0x1d
#define CHAR_1										0x1e
#define CHAR_2										0x1f
#define CHAR_3										0x20
#define CHAR_4										0x21
#define CHAR_5										0x22
#define CHAR_6										0x23
#define CHAR_7										0x24
#define CHAR_8										0x25
#define CHAR_9										0x26
#define CHAR_0										0x27
#define CHAR_RETURN									0x28
#define CHAR_ESCAPE									0x29
#define CHAR_BACKSPACE								0x2a
#define CHAR_TAB									0x2b
#define CHAR_SPACE									0x2C
#define CHAR_TILDE									0x35
#define CHAR_CAPS_LOCK								0x39
#define CHAR_SCROLL_LOCK							0x47
#define CHAR_NUM_LOCK								0x53
#define CHAR_F1										0x3a
#define CHAR_F2										0x3b
#define CHAR_F3										0x3c
#define CHAR_F4										0x3d
#define CHAR_F5										0x3e
#define CHAR_F6										0x3f
#define CHAR_F7										0x40
#define CHAR_F8										0x41
#define CHAR_F9										0x42
#define CHAR_F10									0x43
#define CHAR_F11									0x44
#define CHAR_F12									0x45

#define CHAR_INSERT									0x49
#define CHAR_HOME									0x4a
#define CHAR_PGUP									0x4b
#define CHAR_DEL									0x4c
#define CHAR_END									0x4d
#define CHAR_PGDN									0x4e
#define CHAR_R_ARROW								0x4f
#define CHAR_L_ARROW								0x50
#define CHAR_D_ARROW								0x51
#define CHAR_U_ARROW								0x52

#define CHAR_PAD_SLASH								0x54
#define CHAR_PAD_STAR								0x55
#define CHAR_PAD_MINUS								0x56
#define CHAR_PAD_PLUS								0x57
#define CHAR_PAD_ENTER								0x58
#define CHAR_PAD_1_END								0x59
#define CHAR_PAD_2_DOWN								0x5a
#define CHAR_PAD_3_PGDN								0x5b
#define CHAR_PAD_4_LEFT								0x5c
#define CHAR_PAD_5									0x5d
#define CHAR_PAD_6_RIGHT							0x5e
#define CHAR_PAD_7_HOME								0x5f
#define CHAR_PAD_8_UP								0x60
#define CHAR_PAD_9_PGUP								0x61
#define CHAR_PAD_0_INSERT							0x62
#define CHAR_PAD_DELETE								0x63

//-------------------------------------------------------
//Define Modifiers Keys Value
//-------------------------------------------------------
#define CHAR_CONTROL								0xe0
#define CHAR_SHIFT									0xe1
#define CHAR_ALT									0xe2
#define CHAR_GUI									0xe3

#define CHAR_LCONTROL								0xe4
#define CHAR_LSHIFT									0xe5
#define CHAR_LALT									0xe6
#define CHAR_LGUI									0xe7

#define CHAR_REPEAT									0xfe

/* Define the default hotkey value */
#define HOTKEY_NOTUSED                  0x00
#define KVM_HOTKEY_DEFAULT_AUDIOSW      CHAR_A  //'A'
#define KVM_HOTKEY_DEFAULT_AUDIOMUTE    HOTKEY_NOTUSED  // not used
#define KVM_HOTKEY_DEFAULT_HUBSW        HOTKEY_NOTUSED 	// 'H'
#define KVM_HOTKEY_DEFAULT_MASSSW       CHAR_M // 'M'
#define KVM_HOTKEY_DEFAULT_AUTOSCAN     CHAR_S // 'S'
#define KVM_HOTKEY_DEFAULT_SCANINTERVAL CHAR_I // 'I'
#define KVM_HOTKEY_DEFAULT_BUZZER       CHAR_B // 'B'
#define KVM_HOTKEY_DEFAULT_EDID       	CHAR_E // 'E'
#define KVM_HOTKEY_DEFAULT_RESET      	CHAR_R // 'R'
#define KVM_HOTKEY_DEFAULT_FLASH      	CHAR_F // 'F'

#define KVM_HOTKEY_DEFAULT_OSD          HOTKEY_NOTUSED // not used
#define HOTKEY_VALUE_OVERRUN            0xffff
#define HOTKEY_DESCRIPTOR_TERNIMATE     0xff

/*
$ HOTKEY_Control_Status
*/
#define KVM_HOTKEY_IDLE					0x00
#define KVM_HOTKEY_ACTIVE				0x01

/*
$ Macro Function
*/

/* GLOBAL VARIABLES */
extern U8_T 	TASK_HOTKEY_LED_INDICATE_ID;
extern U8_T 	TASK_KVM_LED_Indicate_ActiveID;
extern U8_T 	HOTKEY_Funciton_Code;
extern U8_T 	HOTKEY_Funciton_Len;
extern U8_T		HOTKEY_Funciton_End_Index;
extern U8_T 	HOTKEY_Operation_Code;
extern U8_T 	HOTKEY_Operation_Len;
extern U8_T 	HOTKEY_Op_Content_Index;
extern U16_T	HOTKEY_Funciton_PerformValue[5];
extern U8_T		HOTKEY_Funciton_PerformValueIndex;
extern U8_T	HOTKEY_Control_Status;  /* Hotkey Status */
extern U8_T	HOTKEY_Control_Status_Last;

void HOTKEY_Init(void);
void HOTKEY_Function_Parser(U8_T);
void HOTKEY_Deactive(void);
void TASK_HOTKEY_Active_Led_Indicate(void);
void TASK_KVM_HotkeyFork_Timeout(void);
void HOTKEY_Handle(U8_T,U8_T);
void HOTKEY_Default_Initial(void);
U8_T HOTKEY_Check_PS2_Key_Break(U8_T buftype);


#endif /* End of __KVM_HOTKEY_H */
