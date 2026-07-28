/*
 *********************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation    All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : kvm_hotkey.c
 * Purpose     : This module handles the kvm hotkey feature 
 *
 * Author      : Jack Wang
 * Date        :
 * Notes       : 
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
/* KEIL Header File Section */
#include	"stdio.h"
#include	"string.h"

/* System Relate Header File Seciton */
#include "project_include.h"

#if (PROJECT_KVM_CONSOLE_ENABLE)
#if (KVM_HOTKEY_SUPPORT)
/* NAMING CONSTANT DECLARATIONS */
CONST	KVM_HotkeyStruct HOTKEY_FORK_DEFAULT[]=
{
//#if (SYSTEM_STANDARD_HOTKEY)
//	HOTKEY_MAKE				,CHAR_GUI,
//#endif	
	HOTKEY_MAKE				,CHAR_SCROLL_LOCK,
	HOTKEY_BREAK			,CHAR_SCROLL_LOCK,
	HOTKEY_MAKE				,CHAR_SCROLL_LOCK,
	HOTKEY_BREAK			,CHAR_SCROLL_LOCK,
//#if (SYSTEM_STANDARD_HOTKEY)
//	HOTKEY_BREAK			,CHAR_GUI,
//#endif		
	HOTKEY_ACTIVE			,0x00,
	HOTKEY_FUNCTION			,0x00,
	HOTKEY_TERMINATE		,0x00
};

#if (SYSTEM_STANDARD_HOTKEY)
CONST	KVM_HotkeyStruct HOTKEY_FORK_CTRLD[]=
{
	//HOTKEY_MAKE				,CHAR_GUI,
	HOTKEY_MAKE				,CHAR_CONTROL,
	HOTKEY_BREAK			,CHAR_CONTROL,
	HOTKEY_MAKE				,CHAR_CONTROL,
	HOTKEY_BREAK			,CHAR_CONTROL,
	//HOTKEY_BREAK			,CHAR_GUI,
	HOTKEY_ACTIVE			,0x00,
	HOTKEY_FUNCTION			,0x00,
	HOTKEY_TERMINATE		,0x00
};
#endif


CONST	U8_T HOTKEY_DESCRIPTOR_DEFAULT[]=
{
	//-----------------------------------
	// Ctrl Key On/OFF
	//-----------------------------------
	9,		//Function Length
	HOTKEY_FUN_ALTERNAMTE_CONTROL,			// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_4,	// OP Code
	CHAR_C,CHAR_T,CHAR_R,CHAR_L,
	HOTKEY_OP_SHORT_VALUE|HOTKEY_LEN_2,		// OP Code
	0x00,	// Minimum 
	0x01,	// Maximum
	//-----------------------------------
	// Port Switch
	//-----------------------------------
	4,		//Function Length
	HOTKEY_FUN_PORT_SW_NUMERIC,				// Function Code
	HOTKEY_OP_SHORT_VALUE+HOTKEY_LEN_2,		// OP Code
	0x01,	// Minimum 
	KVM_MAX_PORT,	// Maximum
	
	//-----------------------------------
	// 1. Single Screen Mode
	//-----------------------------------
	3,		//Function Length
	HOTKEY_FUN_MS_ROAMING_MODE,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE+HOTKEY_LEN_1,		// OP Code
	CHAR_F1,	// Minimum
	//-----------------------------------
	// 2. One Big Two Small Mode
	//-----------------------------------
	3,		//Function Length
	HOTKEY_FUN_KVM_ONE_2_MODE,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE+HOTKEY_LEN_1,		// OP Code
	CHAR_F3,	// Minimum
	//-----------------------------------
	// 3. Dual Screen Mode
	//-----------------------------------
	3,		//Function Length
	HOTKEY_FUN_KVM_PBP_MODE,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE+HOTKEY_LEN_1,		// OP Code
	CHAR_F2,	// Minimum
	//-----------------------------------
	// 4. Four Screen Mode
	//-----------------------------------
	3,		//Function Length
	HOTKEY_FUN_KVM_4WIN_MODE,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE+HOTKEY_LEN_1,		// OP Code
	CHAR_F4,	// Minimum
	//-----------------------------------
	// 5. One Big Three Small Mode
	//-----------------------------------
	3,		//Function Length
	HOTKEY_FUN_KVM_ONE_3_MODE,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE+HOTKEY_LEN_1,		// OP Code
	CHAR_F5,	// Minimum
	//-----------------------------------
	// 6. return last mode
	//-----------------------------------
	3,		//Function Length
	HOTKEY_FUN_RETURN_LAST_MODE,			// Function Code
	HOTKEY_OP_FUNCTION0_CODE+HOTKEY_LEN_1,		// OP Code
	CHAR_Z,			// Minimum 	
	//-----------------------------------	
	//  3.Mouse Relative Coordinates Mode
	//-----------------------------------
	3,		//Function Length
	HOTKEY_FUN_MS_RELATIVE_MODE,			// Function Code
	HOTKEY_OP_FUNCTION0_CODE+HOTKEY_LEN_1,		// OP Code
	CHAR_F9,			// Minimum 	
	//-----------------------------------	
	//  4.Mouse Absolute Coordinates Mode
	//-----------------------------------
	3,		//Function Length
	HOTKEY_FUN_MS_ABSOLUTE_MODE,			// Function Code
	HOTKEY_OP_FUNCTION0_CODE+HOTKEY_LEN_1,		// OP Code
	CHAR_F10,			// Minimum 	
	//-----------------------------------
	// Port Jump by Power Control
	//-----------------------------------
	// #if (ENABLE_HOTKEY_FUN_POWER_JUMP_SW)
	// 7,		//Function Length
	// HOTKEY_FUN_PORT_JUMP_POWER_CONTROL,			// Function Code
	// HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code
	// CHAR_P,CHAR_W,
	// HOTKEY_OP_SHORT_VALUE+HOTKEY_LEN_2,		// OP Code
	// 0x00,	// Minimum 
	// 0x01,	// Maximum
	// #endif //#if (ENABLE_HOTKEY_FUN_POWER_JUMP_SW)
	//-----------------------------------
	// Port Plugin jump Control
	//-----------------------------------
	// #if (ENABLE_HOTKEY_FUN_PLUGIN_JUMP_SW)
	// 9,		//Function Length
	// HOTKEY_FUN_PLUGIN_JUMP_CONTROL,			// Function Code
	// HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_4,	// OP Code
	// CHAR_P,CHAR_L,CHAR_U,CHAR_G,
	// HOTKEY_OP_SHORT_VALUE+HOTKEY_LEN_2,		// OP Code
	// 0x00,	// Minimum 
	// 0x01,	// Maximum
	// #endif //#if (ENABLE_HOTKEY_FUN_PLUGIN_JUMP_SW)
	//-----------------------------------
	// Port Plugin jump Control
	//-----------------------------------
	#if (ENABLE_HOTKEY_FUN_PLUGIN_JUMP_SW)
	7,		//Function Length
	HOTKEY_FUN_PLUGIN_JUMP_MODE,			// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code
	CHAR_P,CHAR_G,
	HOTKEY_OP_SHORT_VALUE+HOTKEY_LEN_2,		// OP Code
	0x00,	// Minimum(only jump one time)
	0x01,	// Maximum(always jump)
	#endif //#if (ENABLE_HOTKEY_FUN_PLUGIN_JUMP_SW)								
	//-----------------------------------
	// Combo Audio AutoSwitch Control(Receiver)
	//-----------------------------------	
	#if (ENABLE_HOTKEY_FUN_AUDIO_SW) || (ENABLE_HOTKEY_FUN_USBAUDIO_SW)	 
	7,		//Function Length
	HOTKEY_FUN_AUTOAUDIO_SW,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code
	CHAR_A,CHAR_A,							// 'AA'
	HOTKEY_OP_SHORT_VALUE|HOTKEY_LEN_2,		// Control Value
	0x00,	// Minimum 
	0x01,	// Maximum	 
	#endif  /* #if (ENABLE_HOTKEY_FUN_AUDIO_SW) || (ENABLE_HOTKEY_FUN_USBAUDIO_SW) */
	//-----------------------------------
	// Audio Switch
	//-----------------------------------
	#if (ENABLE_HOTKEY_FUN_AUDIO_SW)
	6,		//Function Length
	HOTKEY_FUN_AUDIO_SW,					// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_1,	// OP Code
	KVM_HOTKEY_DEFAULT_AUDIOSW,				// 'A'
	HOTKEY_OP_SHORT_VALUE|HOTKEY_LEN_2,		// OP Code
	0x01,	// Minimum 
	0x03,	// Maximum
	#endif //#if (ENABLE_HOTKEY_FUN_AUDIO_SW)
	//-----------------------------------
	// USB Audio Switch
	//-----------------------------------
	#if (ENABLE_HOTKEY_FUN_USBAUDIO_SW)	
	7,		//Function Length
	HOTKEY_FUN_USBAUDIO_SW,					// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code
	CHAR_U,KVM_HOTKEY_DEFAULT_AUDIOSW,		// 'UA'
	HOTKEY_OP_SHORT_VALUE|HOTKEY_LEN_2,		// OP Code
	0x01,	// Minimum 
	KVM_MAX_PORT,	// Maximum	
	#endif //#if (ENABLE_HOTKEY_FUN_USBAUDIO_SW)
	
	//--------------------------------------------
	// Transmitter Combo Audio AutoSwitch Control
	//---------------------------------------------
	#if (ENABLE_HOTKEY_T_AUDIO_AUTO_SW)
	8,		//Function Length
	HOTKEY_FUN_T_USBAUDIO_AUTOSW,			// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_3,	// OP Code
	CHAR_T,CHAR_A,CHAR_A,					// 'TAA'
	HOTKEY_OP_SHORT_VALUE|HOTKEY_LEN_2,		// Control Value
	0x00,	// Minimum 
	0x01,	// Maximum
	#endif
	//-----------------------------------
	// Transmitter USB Audio Switch
	//-----------------------------------
	#if (ENABLE_HOTKEY_T_USBAUDIO_SW)
	7,		//Function Length
	HOTKEY_FUN_T_USBAUDIO_SW,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code
	CHAR_T,CHAR_A,							// 'TA'
	HOTKEY_OP_SHORT_VALUE|HOTKEY_LEN_2,		// OP Code
	0x01,	// Minimum 
	KVM_MAX_PORT,	// Maximum
	#endif //#if (ENABLE_HOTKEY_T_USBAUDIO_SW)	
	
	//-----------------------------------
	// Buzzer Control(Toggle)
	//-----------------------------------	 
	// #if (ENABLE_HOTKEY_FUN_BUZZER_CONTROL)	
	// 6,		//Function Length
	// HOTKEY_FUN_BUZZER_CONTROL_TOGGLE,		// Function Code
	// HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_1,	// OP Code+Length
	// KVM_HOTKEY_DEFAULT_BUZZER,				// 'B'
	// HOTKEY_OP_SHORT_VALUE|HOTKEY_LEN_2,		// OP Code
	// 0x00,	// Minimum
	// 0x01,	// Maximum
	// #endif /* #if (ENABLE_HOTKEY_FUN_BUZZER_CONTROL) */
	// //-----------------------------------
	// // AutoScan Control
	// //-----------------------------------
	// #if (ENABLE_HOTKEY_FUN_AUTOSCAN_CONTROL)
	// 3,		//Function Length
	// HOTKEY_FUN_AUTOSCAN_CONTROL,			// Function Code
	// HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_1,	// OP Code+Length. Start
	// KVM_HOTKEY_DEFAULT_AUTOSCAN,			// 'S'
	// #endif //#if (ENABLE_HOTKEY_FUN_AUTOSCAN_CONTROL)
	// //-----------------------------------
	// // AutoScan Interval Control
	// //-----------------------------------
	// #if (ENABLE_HOTKEY_FUN_AUTOSCAN_INTERVAL)
	// 11,		//Function Length
	// HOTKEY_FUN_AUTOSCAN_INTERVAL,			// Function Code
	// HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_1,	// OP Code+Length  Interval
	// CHAR_I,									// 'I'
	// HOTKEY_OP_LONG_VALUE|HOTKEY_LEN_4,		// OP Code+Length  Interval
	// 0x05,0x00,								// Minimum-5
	// 0xE7,0x03,								// Maximum-999
	// HOTKEY_OP_TERNIMATE|HOTKEY_LEN_2,		// OP Code+Length  Interval
	// CHAR_RETURN,							//
	// CHAR_PAD_ENTER,							//
	// #endif //#if (ENABLE_HOTKEY_FUN_AUTOSCAN_INTERVAL)
	//-----------------------------------
	// Port Reset
	//-----------------------------------
	#if (ENABLE_HOTKEY_FUN_PORT_RESET)
	7,										//Function Length	 
	HOTKEY_FUN_PORT_RESET,					// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code
	CHAR_R,									// 'R'
	CHAR_S,									// 'S' ==>Port,Reset
	HOTKEY_OP_SHORT_VALUE|HOTKEY_LEN_2,		// OP Code
	0x01,									// Minimum
	KVM_MAX_PORT,							// Maximum
	#endif //#if (ENABLE_HOTKEY_FUN_PORT_RESET)
	//-----------------------------------
	// MSC Support
	//-----------------------------------
	#if (ENABLE_HOTKEY_FUN_MSC_CONTROL)
	8,		//Function Length
	HOTKEY_FUN_MSC_CONTRL,					// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_3,	// OP Code-Start
	CHAR_M,CHAR_S,CHAR_C,					// 'MSC'
	HOTKEY_OP_SHORT_VALUE|HOTKEY_LEN_2,		// OP Code
	0x00,	// Minimum
	0x01,	// Maximum
	#endif
	//-----------------------------------
	// MSC Switch
	//-----------------------------------
	// #if (ENABLE_HOTKEY_FUN_MSC_SW)
	// 6,		//Function Length
	// HOTKEY_FUN_MSC_SW,						// Function Code
	// HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_1,	// OP Code-Start
	// CHAR_M,									// 'M'
	// HOTKEY_OP_SHORT_VALUE|HOTKEY_LEN_2,		// OP Code
	// 0x01,	// Minimum
	// KVM_MAX_PORT,	// Maximum
	// #endif
	
	// //-----------------------------------
	// // Transmitter MSC Switch
	// //-----------------------------------
	// #if (ENABLE_HOTKEY_T_MSC_SW)
	// 7,		//Function Length
	// HOTKEY_FUN_T_MSC_SW,						// Function Code
	// HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code-Start
	// CHAR_T,CHAR_M,									// 'TM'
	// HOTKEY_OP_SHORT_VALUE|HOTKEY_LEN_2,		// OP Code
	// 0x01,	// Minimum
	// KVM_MAX_PORT,	// Maximum
	// #endif
	//-----------------------------------
	// Flash with Default Value
	//-----------------------------------
	#if (ENABLE_HOTKEY_FUN_FLASH_DEFAULT)
	10,		//Function Length
	HOTKEY_FUN_FLASH_DEFAULT,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_5,	// OP Code-Start
	CHAR_F,CHAR_L,CHAR_A,CHAR_S,CHAR_H,		// 'FLASH'
	HOTKEY_OP_TERNIMATE|HOTKEY_LEN_2,		// OP Code+Length  Interval
	CHAR_RETURN,							//
	CHAR_PAD_ENTER,							//
	#endif
	//-----------------------------------
	// EDID Reread
	//-----------------------------------
	#if (ENABLE_HOTKEY_FUN_EDID_REREAD)
	9,		//Function Length
	HOTKEY_FUN_EDID_REREAD,					// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_4,	// OP Code-Start
	CHAR_E,CHAR_D,CHAR_I,CHAR_D,			// 'EDID'
	HOTKEY_OP_TERNIMATE|HOTKEY_LEN_2,		// OP Code+Length  Interval
	CHAR_RETURN,							//
	CHAR_PAD_ENTER,							//
	#endif
	//-----------------------------------
	// Rapid Switch
	//-----------------------------------
	#if (ENABLE_HOTKEY_FUN_RAPID_SW)
	4,		//Function Length
	HOTKEY_FUN_RAPID_SW,					// Function Code
	HOTKEY_OP_FUNCTION5_CODE|HOTKEY_LEN_2,	// OP Code+Length  Interval
	CHAR_RETURN,							//
	CHAR_PAD_ENTER,							//
	#endif
	//-----------------------------------
	// USB PEN Driver ON/OFF Control
	//-----------------------------------
	#if (ENABLE_HOTKEY_FUN_PEN_CONTROL)
	8,		//Function Length
	HOTKEY_FUN_PEN_CONTROL,					// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_3,	// OP Code-Start
	CHAR_P,CHAR_E,CHAR_N,					// 'PEN'
	HOTKEY_OP_SHORT_VALUE|HOTKEY_LEN_2,		// OP Code
	0x00,	// Minimum
	0x01,	// Maximum
	#endif
	//-----------------------------------
	// USB PEN Drive Switch
	//-----------------------------------
	#if (ENABLE_HOTKEY_FUN_PEN_SW)
	6,		//Function Length
	HOTKEY_FUN_PEN_SW,						// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_1,	// OP Code-Start
	CHAR_P,									// 'P'
	HOTKEY_OP_SHORT_VALUE|HOTKEY_LEN_2,		// OP Code
	0x01,	// Minimum
	KVM_MAX_PORT,	// Maximum
	#endif
	//-----------------------------------
	// USB PowerSaving Control
	//-----------------------------------
	#if (ENABLE_HOTKEY_FUN_PWSAVING_CONTROL)
	7,		//Function Length
	HOTKEY_FUN_POWER_SAVING_CONTROL,		// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code-Start
	CHAR_P,CHAR_S,					// 'PS'
	HOTKEY_OP_SHORT_VALUE|HOTKEY_LEN_2,		// OP Code
	0x00,	// Minimum
	0x01,	// Maximum
	#endif
	//-----------------------------------
	// Firmware Upgrade
	//-----------------------------------
	6,		//Function Length
	HOTKEY_FUN_UPDATE_FIRMWARE,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_1,	// OP Code-Start
	CHAR_U,									// 'U'
	HOTKEY_OP_TERNIMATE|HOTKEY_LEN_2,		// OP Code+Length  Interval
	CHAR_RETURN,							//
	CHAR_PAD_ENTER,
	
#ifdef RTC
	#if (ENABLE_HOTKEY_FUN_RTC_CONTROL)
	7,		//Function Length
	HOTKEY_FUN_RTC_CONTROL,					// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_3,	// OP Code-Start
	CHAR_R,CHAR_T,CHAR_C,					// 'RTC'
	HOTKEY_OP_SHORT_VALUE|HOTKEY_LEN_2,		// OP Code
	0x00,	// Minimum
	0x01,	// Maximum
	#endif
#endif

#if (EXTENDER_ISL59920_SUPPORT)	
	//-----------------------------------
	// ISL59920 R -> Delay
	//-----------------------------------
	4,		//Function Length
	HOTKEY_FUN_R_DELAY_PLUS,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code-Start
	CHAR_R,CHAR_R_ARROW,
	
	4,		//Function Length
	HOTKEY_FUN_R_DELAY_PLUS,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code-Start
	CHAR_R,CHAR_PAD_6_RIGHT,								
	
	//-----------------------------------
	// ISL59920 G+ Delay
	//-----------------------------------
	4,		//Function Length
	HOTKEY_FUN_G_DELAY_PLUS,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code-Start
	CHAR_G,CHAR_R_ARROW,
	
	4,		//Function Length
	HOTKEY_FUN_G_DELAY_PLUS,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code-Start
	CHAR_G,CHAR_PAD_6_RIGHT,
						
	//-----------------------------------
	// ISL59920 B+ Delay
	//-----------------------------------
	4,		//Function Length
	HOTKEY_FUN_B_DELAY_PLUS,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code-Start
	CHAR_B,CHAR_R_ARROW,
	
	4,		//Function Length
	HOTKEY_FUN_B_DELAY_PLUS,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code-Start
	CHAR_B,CHAR_PAD_6_RIGHT,
	
	//-----------------------------------
	// ISL59920 R- Delay
	//-----------------------------------
	4,		//Function Length
	HOTKEY_FUN_R_DELAY_MINUS,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code-Start
	CHAR_R,CHAR_L_ARROW,
	
	4,		//Function Length
	HOTKEY_FUN_R_DELAY_MINUS,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code-Start
	CHAR_R,CHAR_PAD_4_LEFT,								
	//-----------------------------------
	// ISL59920 G- Delay
	//-----------------------------------
	4,		//Function Length
	HOTKEY_FUN_G_DELAY_MINUS,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code-Start
	CHAR_G,CHAR_L_ARROW,
	
	4,	
	HOTKEY_FUN_G_DELAY_MINUS,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code-Start
	CHAR_G,CHAR_PAD_4_LEFT,
	
	//-----------------------------------
	// ISL59920 B- Delay
	//-----------------------------------
	4,		//Function Length
	HOTKEY_FUN_B_DELAY_MINUS,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code-Start
	CHAR_B,CHAR_L_ARROW,
	
	4,		//Function Length
	HOTKEY_FUN_B_DELAY_MINUS,				// Function Code
	HOTKEY_OP_FUNCTION0_CODE|HOTKEY_LEN_2,	// OP Code-Start
	CHAR_B,CHAR_PAD_4_LEFT,
						
#endif /* #if (EXTENDER_ISL59920_SUPPORT) */ 
	//-----------------------------------
	// Function Ternimate
	//-----------------------------------
	HOTKEY_DESCRIPTOR_TERNIMATE,			//Function terminste
};

/* STATIC VARIABLE DECLARATIONS */
bit		HOTKEY_OP_Content_Ternimate_Flag;
bit		HOTKEY_Function_Match_Flag;
bit		HOTKEY_Function_Continue_Flag;
bit		HOTKEY_Operation_Continue_Flag;
bit		HOTKEY_FirstKeyTimeOutCheck_Flag;

KVM_HotkeyStruct *HOTKEY_Function_Hotkey_Table;

U8_T	HOTKEY_Function_CheckTableID;
U8_T	HOTKEY_Funciton_Code		= 0;
U8_T	HOTKEY_Function_Modifiers	= 0;
U8_T 	HOTKEY_Funciton_Len			= 0;
U8_T	HOTKEY_Funciton_End_Index;
U8_T 	HOTKEY_Operation_Code		= 0;
U8_T 	HOTKEY_Operation_Len		= 0;
U8_T 	HOTKEY_Op_Content_Index		= 0;
U8_T	HOTKEY_Parser_State			= 0;
U8_T 	HOTKEY_Descritpr_Index		= 0;
U8_T 	HOTKEY_Descritpr_Index_Hold;
U8_T 	HOTKEY_Fork_Table_Index;			/* Hotkey Function ID  */
U8_T	KVM_Hotkey_Serial			= 0;
U8_T	HOTKEY_Control_Status;				/* Hotkey Status */
U8_T	HOTKEY_Control_Status_Last;
U16_T	HOTKEY_OP_Minmmum_Value;
U16_T	HOTKEY_OP_Maxmmum_Value;
U8_T	HOTKEY_ExOperation_Valid_Cnt;		/* at least one operation is valid */
static U8_T	hotkey_CheckValueFlag;

U16_T	HOTKEY_Funciton_PerformValue[5];
U8_T	HOTKEY_Funciton_PerformValueIndex;

#if  (KVM_HOTKEY_TIMEOUT_CHECK)
U8_T	HOTKEY_Active_Valid_Cnt;
#endif

U8_T	HOTKEY_LED;
U8_T    HOTKEY_Function_Code_Stack[20];
U8_T    HOTKEY_Function_Code_Cnt;
U8_T    HOTKEY_Function_Code_Index;

/*-------------------------------------------*/
/* For Task control usage */
U8_T	TASK_HOTKEY_LED_INDICATE_ID;
U8_T	TASK_KVM_LED_Indicate_ActiveID;
U8_T	TASK_KVM_HotkeyFork_Timeout_ID;
/*-------------------------------------------*/

/* LOCAL SUBPROGRAM DECLARATIONS */
void KVM_Hostkey_LED_Indicate(void); 
void HOTKEY_Default_Initial(void);
void TASK_KVM_HotkeyFork_Timeout(void);
static U8_T hotkey_KeyCodeMatch(U8_T keyCode, U8_T matchCode);

/* LOCAL SUBPROGRAM BODIES */
/*
 *--------------------------------------------------------------------------------
 * void HOTKEY_Init(void)
 * Purpose : KVM Console hotkey feature init procedure
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void HOTKEY_Init(void)
{
	/* Hotkey Function Key Setting */
	/* Hotkey Fork Key Setting */

#if (SYSTEM_STORAGE_FLASH)    
#else    
	/*1-Copy default Hotkey Fork sequence descriptor table */
	memcpy((void *)KVM_Flash.Hotkey_Active_Descriptor,(void *)HOTKEY_FORK_DEFAULT,sizeof(HOTKEY_FORK_DEFAULT));
	/*2-Copy default Hotkey Descriptor table */
	printf("HK fork:\n\r");
	for (i = 0; i < 8; i++)
	{
		printf("%bx %bx\n\r",
			KVM_Flash.Hotkey_Active_Descriptor[i].Keytype,
			KVM_Flash.Hotkey_Active_Descriptor[i].Keycode);
	}
	if (HOTKEY_DESCRIPTOR_DEFAULT  > sizeof(KVM_Flash.HotKey_Descriptor))
	{	
	    //printf("> ERROR(Table size=%d, over size:%d)\n\r",HOTKEY_DESCRIPTOR_DEFAULT,sizeof(KVM_Flash.HotKey_Descriptor));
	}	
    else
	    memcpy((void *)KVM_Flash.HotKey_Descriptor,(void *)HOTKEY_DESCRIPTOR_DEFAULT,sizeof(HOTKEY_DESCRIPTOR_DEFAULT));
#endif    
	HOTKEY_Default_Initial();	

	HOTKEY_Fork_Table_Index			= 0;	/* Hotkey Table process index */
	HOTKEY_Control_Status			= 0;	/* Hotkey Status */
	HOTKEY_Control_Status_Last		= 0;
	HOTKEY_Function_Continue_Flag	= 0;
	HOTKEY_Operation_Continue_Flag	= 0;
	HOTKEY_FirstKeyTimeOutCheck_Flag= 0;
	HOTKEY_Function_CheckTableID 	= 0;
	hotkey_CheckValueFlag = 0;	
}

/*
 *--------------------------------------------------------------------------------
 * void HOTKEY_Default_Initial(void)
 * Purpose : internal subprgram fork by STORAGE_Default_Initial()
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void HOTKEY_Default_Initial(void)
{
	/*1-Copy default Hotkey Fork sequence descriptor table */
	memcpy((void *)KVM_Flash.Hotkey_Active_Descriptor,(void *)HOTKEY_FORK_DEFAULT,sizeof(HOTKEY_FORK_DEFAULT));
 	/*2-Copy default Hotkey Descriptor table */
 	memcpy((void *)KVM_Flash.HotKey_Descriptor,(void *)HOTKEY_DESCRIPTOR_DEFAULT,sizeof(HOTKEY_DESCRIPTOR_DEFAULT));
}

/*
 *--------------------------------------------------------------------------------
 * U8_T HOTKEY_Check_PS2_Key_Break(U8_T buftype)
 * Purpose : check break key status
 * Params  : None.
 * Returns : 0/1
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
#if (SYSTEM_PS2_HOST_ENABLE)
U8_T HOTKEY_Check_PS2_Key_Break(U8_T buftype)
{
	if (buftype == KB_TYPE_BREAK_F0)
	{
		return 1;
	}

	if (buftype == KB_TYPE_BREAK_E0) 
	{
		return 2;
	}

	if (buftype == KB_TYPE_BREAK_E1) 
	{
		return 3;
	}

	return 0;
}  
#endif
/*
 *--------------------------------------------------------------------------------
 * U8_T HOTKEY_Check_Numeric_Key(U8_T keycode,U16_T *value)
 * Purpose : check usb keycode, wheither is numeric key or keypad numeric key
 * Params  : $keycode-USB Scan Code
 *           $*value- USB scan code to a digit
 * Returns : 0/1- 1->is numeric key
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
U8_T HOTKEY_Check_Numeric_Key(U8_T keycode,U16_T *value)
{
	U8_T	keyvalue=0, newvalue;

	newvalue = *value;
	if ((keycode >= CHAR_1) && (keycode <= CHAR_0))
	{
		if (keycode < CHAR_0)
		{
			keyvalue = (keycode - CHAR_1) + 1;
		}

		goto NumericKeyChange;
	}
	else if ((keycode >= CHAR_PAD_1_END) && (keycode <= CHAR_PAD_0_INSERT))
	{
		if (keycode < CHAR_PAD_0_INSERT)
		{
			keyvalue = (keycode - CHAR_PAD_1_END) + 1;
		}

		goto NumericKeyChange;
	}

	return 0;

NumericKeyChange:

	*value = (newvalue << 3)+(newvalue << 1)+keyvalue; // value = value * 10 + value
	return 1;
}

/*
 *--------------------------------------------------------------------------------
 * U8_T hotkey_KeyCodeMatch(U8_T keyCode, U8_T matchCode)
 * Purpose : 
 * Params  : 
 *           
 * Returns : 
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
static U8_T hotkey_KeyCodeMatch(U8_T keyCode, U8_T matchCode)
{
	if ((matchCode >= CHAR_1) && (matchCode <= CHAR_0))
	{
		if (keyCode == matchCode)
			return 1;
		else if (keyCode == (matchCode+0x3B))
			return 1;
	}
	else if ((matchCode >= CHAR_PAD_1_END) && (matchCode <= CHAR_PAD_0_INSERT))
	{
		if (keyCode == matchCode)
			return 1;
		else if (keyCode == (matchCode-0x3B))
			return 1;
	}
	else if (keyCode == matchCode)
		return 1;

	return 0;
}

/*
 *--------------------------------------------------------------------------------
 * void HOTKEY_Handle(U8_T keytype,U8_T keycode)
 * Purpose : check kvm hotkey status
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void HOTKEY_Handle(U8_T keytype,U8_T keycode)
{	
	/* Check hotkey */
	/* [1].Hotkey already actived */\	
	//printf("t=%bx,c=%bx\n\r",keytype,keycode);

	// printf("HK in: type=%bx code=%bx idx=%bx status=%bx cMod=%bx table=%bx/%bx\n\r",
    // keytype,
    // keycode,
    // HOTKEY_Fork_Table_Index,
    // HOTKEY_Control_Status,
    // cModifiers,
    // HOTKEY_Function_Hotkey_Table[HOTKEY_Fork_Table_Index].Keytype,
    // HOTKEY_Function_Hotkey_Table[HOTKEY_Fork_Table_Index].Keycode);
	if (HOTKEY_Control_Status == KVM_HOTKEY_ACTIVE)
	{
		if (keytype & 0x01)		// KEY Break
		{
#if (EXTENDER_ISL59920_SUPPORT)			
			if (bOperateFlag)
			{
				ISL59920_Hotkey_Control(keycode);
				return;
			}
			else					
#endif  /* #if (EXTENDER_ISL59920_SUPPORT) */	
			{	
				//Store the functino code sequence
				HOTKEY_Function_Code_Stack[HOTKEY_Function_Code_Cnt] = keycode;			
				HOTKEY_Function_Code_Cnt++;				
				if (HOTKEY_Function_Hotkey_Table[HOTKEY_Fork_Table_Index].Keytype == HOTKEY_FUNCTION)	// now do the function
				{					
					HOTKEY_Function_Parser(keycode);
				}
				else if (HOTKEY_Function_Hotkey_Table[HOTKEY_Fork_Table_Index].Keytype == HOTKEY_TERMINATE)	// terminate the hotkey function
				{
					HOTKEY_Deactive();	// reset the
				}
			}	
		}
	}
	else 
	{
HOTKEY_LEADING_KEY_START:		
		/* Hotkey not active */
		if (HOTKEY_Control_Status == KVM_HOTKEY_IDLE)
		{
			if (HOTKEY_Function_CheckTableID == 0)
			{					
				HOTKEY_Function_Hotkey_Table = KVM_Flash.Hotkey_Active_Descriptor;
			}
			else
			{				
				HOTKEY_Function_Hotkey_Table = HOTKEY_FORK_CTRLD;//ѡ���ȼ����������б�
			}	
			/* start from normal condition */
			//Change the modifiers keyfirst */			
			if ((keycode >= 0xe4) && (keycode <= 0xe7))//���μ�ͳһ��������ݼ�ʱ�����?�ֱ�֧������ Ctrl��
			{				
				keycode = keycode - 0x04;
			}
				
			if (HOTKEY_Function_Hotkey_Table[HOTKEY_Fork_Table_Index].Keycode == keycode)	/* Keycode match */
			{
				if (HOTKEY_Function_Hotkey_Table[HOTKEY_Fork_Table_Index].Keytype == keytype) /* Keytype also match */
				{
					if (HOTKEY_Fork_Table_Index==0) // in the first begining
					{
						HOTKEY_Function_Modifiers = 0;
					}
					
					//Modifiers key judgement					
					if (keytype == HOTKEY_MAKE)
					{	
						if ((keycode >= 0xe0) && (keycode <= 0xe3))
						{
							HOTKEY_Function_Modifiers |=  0x11 << (keycode-0xe0);
						}
					}
					else if (keytype == HOTKEY_BREAK)
					{
						if ((keycode >= 0xe0) && (keycode <= 0xe3))
						{
							HOTKEY_Function_Modifiers &= ~(0x11 << (keycode-0xe0));
						}
					}	
					
					//printf("HOTKEY_Function_Modifiers=%02x,cModifiers=%02x\n\r",(U16_T)HOTKEY_Function_Modifiers,(U16_T)cModifiers);
					if ((cModifiers == (HOTKEY_Function_Modifiers & 0xf0)) || (cModifiers == (HOTKEY_Function_Modifiers & 0x0f)))
					{
						HOTKEY_Fork_Table_Index++;  // Function index add index
						if (HOTKEY_Fork_Table_Index==1) // in the first begining
						{
							HOTKEY_FirstKeyTimeOutCheck_Flag = 1;
							KVM_Hotkey_Serial++;
							TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_KVM_HotkeyFork_Timeout_ID,0,KVM_Hotkey_Serial,5000,5000); // for 5seconds, if the
						}

						if (HOTKEY_Function_Hotkey_Table[HOTKEY_Fork_Table_Index].Keytype == HOTKEY_ACTIVE)
						{
							printf("HOTKEY ACTIVE!\n\r");
							HOTKEY_Control_Status = KVM_HOTKEY_ACTIVE; /* hotkey actived */
							HOTKEY_FirstKeyTimeOutCheck_Flag = 0;
							HOTKEY_Fork_Table_Index++;	// for next operation */
							HOTKEY_LED = 0;
							HOTKEY_Function_Code_Index = 0; //From now on record the function code sequences.
							HOTKEY_Function_Code_Cnt   = 0;	
							HOTKEY_Function_CheckTableID = 0;
							/*------------------------------------------*/
							/*This is for Hotkey function parser 				*/
							HOTKEY_Funciton_Code 	       = 0;
							HOTKEY_Operation_Code        = 0;
							HOTKEY_Descritpr_Index       = 0; // first function code
							HOTKEY_Parser_State 	       = HOTKEY_PARSER_IDLE;
							HOTKEY_Function_Match_Flag   = 0;
							HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex] = 0;
							/*------------------------------------------*/
							#if (KVM_BUZZER_SUPPORT)
							BUZZER_Script_Active(Hotkey_Active_Sound);
							#endif
							/*------------------------------------------*/
							#if (KVM_HOTKEY_TIMEOUT_CHECK)
							HOTKEY_Active_Valid_Cnt		= 0;
							#endif

							TASK_KVM_LED_Indicate_ActiveID = TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_HOTKEY_LED_INDICATE_ID,0,0,HOTKEY_LED_INDICATE_TIME,HOTKEY_LED_INDICATE_TIME);
						}
					}
					else
					{
						//HOTKEY_Fork_Table_Index = 0; // the hotkey fork sequence check is restart from
						goto HOTKEY_LEADING_KEY_ALTERNAMTE_CHECK;
					}
				}
				else
				{
					//HOTKEY_Fork_Table_Index = 0;	// the hotkey fork sequence check is restart from 0
					goto HOTKEY_LEADING_KEY_ALTERNAMTE_CHECK;
				}
			}
			else
			{
				HOTKEY_Fork_Table_Index = 0;	// the hotkey fork sequence check is restart from 0
HOTKEY_LEADING_KEY_ALTERNAMTE_CHECK:
				if (KVM_Flash.cSystemFlag0 & SYSTEM_HOTKEY_ALTERNATE_MASK)
				{	
					if (HOTKEY_Function_CheckTableID == 0)
					{							
						HOTKEY_Function_CheckTableID = 1;
						goto HOTKEY_LEADING_KEY_START;
					}											
				}						
				HOTKEY_Function_CheckTableID = 0;				
			}
		}
	}

//HOTKEY_Handle_Exit:
	KVM_Autoscan_KB_Handle(keytype,keycode);
}

/*
 *--------------------------------------------------------------------------------
 * void HOTKEY_OP_Content_Parser(U8_T keycode)
 * Purpose : check kvm hotkey status
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
U8_T HOTKEY_OP_Content_Parser(U8_T keycode)
{
	U8_T index;

	HOTKEY_OP_Content_Ternimate_Flag = 0; // defautl flag is reseted
	switch(HOTKEY_Operation_Code)
	{
		// Keycode and .............................
		// Keycode must match one by one
		case HOTKEY_OP_FUNCTION0_CODE:
		case HOTKEY_OP_FUNCTION1_CODE:
		case HOTKEY_OP_FUNCTION2_CODE:
		case HOTKEY_OP_FUNCTION3_CODE:
		case HOTKEY_OP_FUNCTION4_CODE:
			if (hotkey_KeyCodeMatch(keycode, KVM_Flash.HotKey_Descriptor[HOTKEY_Descritpr_Index+HOTKEY_Op_Content_Index]))
			{
				HOTKEY_Op_Content_Index++;
				if (HOTKEY_Op_Content_Index < HOTKEY_Operation_Len)
				{
					HOTKEY_Operation_Continue_Flag = 1; // stay in this operation
				}
				else
				{
					HOTKEY_Operation_Continue_Flag = 0; // skip for next op or funciton
				}

				HOTKEY_OP_Content_Ternimate_Flag = 1; //Parser terminate, wait for next key
				return 1;
			}
			break;

		// Keycode or .............................
		// keycode only match one of the content
		case HOTKEY_OP_TERNIMATE:
		case HOTKEY_FUN_TERNIMATE:
		case HOTKEY_OP_FUNCTION5_CODE:
			if (HOTKEY_Funciton_PerformValueIndex)
				HOTKEY_Funciton_PerformValueIndex--;

			if ((HOTKEY_Operation_Code == HOTKEY_OP_TERNIMATE) || (HOTKEY_Operation_Code == HOTKEY_FUN_TERNIMATE))
			{
				if (HOTKEY_ExOperation_Valid_Cnt==0)
					break;
			}

			for (index=0; index < HOTKEY_Operation_Len ;index++)
			{
				if (hotkey_KeyCodeMatch(keycode, KVM_Flash.HotKey_Descriptor[HOTKEY_Descritpr_Index+index]))
				{
					HOTKEY_OP_Content_Ternimate_Flag = 1; // skip for next op or funciton
					return 1;
				}
			}
			break;

		// Keycode+Value...........................
		case HOTKEY_OP_KEY_TO_VALUE:
			if (keycode == KVM_Flash.HotKey_Descriptor[HOTKEY_Descritpr_Index]) // keycode match
			{
				HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex] = KVM_Flash.HotKey_Descriptor[HOTKEY_Descritpr_Index+1]; // keycode match
				HOTKEY_OP_Content_Ternimate_Flag = 1; // skip for next op or funciton
				return 1;
			}
			break;
	
		// Character value.........................
		case HOTKEY_OP_SHORT_VALUE:
			if (HOTKEY_Op_Content_Index == 0)
			{
				HOTKEY_Op_Content_Index++;
				HOTKEY_OP_Minmmum_Value = KVM_Flash.HotKey_Descriptor[HOTKEY_Descritpr_Index];
				HOTKEY_OP_Maxmmum_Value = KVM_Flash.HotKey_Descriptor[HOTKEY_Descritpr_Index+1];
				HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex] = 0;
			}
	
			if (HOTKEY_Check_Numeric_Key(keycode,&HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex]))
			{
				if (HOTKEY_OP_Maxmmum_Value <= 10 ) // if the max digit is only one digit, then terminate it
					HOTKEY_OP_Content_Ternimate_Flag = 1; // skip for next op or funciton

				hotkey_CheckValueFlag = 1;
				return 1;
			}
			else
			{
				HOTKEY_Operation_Continue_Flag   = 0;
				HOTKEY_OP_Content_Ternimate_Flag = 1; // skip for next op or funciton
				HOTKEY_Function_Match_Flag = 0;
				HOTKEY_Function_Continue_Flag = 0;							
			}
			break;
		
		// Integerr value..........................
		case HOTKEY_OP_LONG_VALUE:
			if (HOTKEY_Op_Content_Index == 0)
			{
				HOTKEY_Op_Content_Index++;
				HOTKEY_OP_Minmmum_Value = KVM_Flash.HotKey_Descriptor[HOTKEY_Descritpr_Index]+(KVM_Flash.HotKey_Descriptor[HOTKEY_Descritpr_Index+1] << 8);
				HOTKEY_OP_Maxmmum_Value = KVM_Flash.HotKey_Descriptor[HOTKEY_Descritpr_Index+2]+(KVM_Flash.HotKey_Descriptor[HOTKEY_Descritpr_Index+3] << 8);
				HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex] = 0;
			}

			if (HOTKEY_Check_Numeric_Key(keycode,&HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex]))
			{
				hotkey_CheckValueFlag = 1;
				HOTKEY_Operation_Continue_Flag = 1;
				HOTKEY_OP_Content_Ternimate_Flag = 1; // skip for next op or funciton
				return 1;
			}
			else
			{
				HOTKEY_Operation_Continue_Flag   = 0;
				HOTKEY_OP_Content_Ternimate_Flag = 1; // skip for next op or funciton
			}
			break;
	}

	return 0;
}

/*
 *--------------------------------------------------------------------------------
 * void HOTKEY_Function_Parser(U8_T keycode)
 * Purpose : check kvm hotkey status
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void HOTKEY_Function_Parser(U8_T keycode)
{
	/*1.Reset the vaild time out value */
	U8_T  Current_Check=1;

#if  (KVM_HOTKEY_TIMEOUT_CHECK)
	HOTKEY_Active_Valid_Cnt = 0;
#endif

HOTKEY_Function_Parser_Resset:
	if (Current_Check==0)
	{
		HOTKEY_Parser_State = HOTKEY_PARSER_FUNCTION_CODE_STAGE;
		HOTKEY_Function_Code_Index = 0;
	}

HOTKEY_Function_Parser_Restart:	
	/*2.Get the Function Code first */
	if (Current_Check==0)
	{
		keycode = HOTKEY_Function_Code_Stack[HOTKEY_Function_Code_Index];
		HOTKEY_Function_Code_Index++;
	}

	while ((HOTKEY_Control_Status == KVM_HOTKEY_ACTIVE) && (HOTKEY_Function_Match_Flag == 0))
	{
		if (KVM_Flash.HotKey_Descriptor[HOTKEY_Descritpr_Index] == HOTKEY_DESCRIPTOR_TERNIMATE) // Ternimate code
		{
			HOTKEY_Deactive();
			break;
		}
		else
		{
			switch (HOTKEY_Parser_State)
			{
				case HOTKEY_PARSER_IDLE:
					HOTKEY_Parser_State = HOTKEY_PARSER_FUNCTION_CODE_STAGE;

				case HOTKEY_PARSER_FUNCTION_CODE_STAGE:
					//1.Get Function Length
					HOTKEY_Funciton_Len = KVM_Flash.HotKey_Descriptor[HOTKEY_Descritpr_Index];
					HOTKEY_Descritpr_Index++;

					//2.Get Function Code
					HOTKEY_Funciton_Code = KVM_Flash.HotKey_Descriptor[HOTKEY_Descritpr_Index];
					HOTKEY_Descritpr_Index_Hold = HOTKEY_Descritpr_Index;
					HOTKEY_Funciton_End_Index = HOTKEY_Descritpr_Index+HOTKEY_Funciton_Len;
					HOTKEY_Descritpr_Index++;
					HOTKEY_Function_Match_Flag= 0; // default function match flag is reset
					HOTKEY_Parser_State = HOTKEY_PARSER_OPCODE_STAGE;
					HOTKEY_Function_Continue_Flag = 0;
					/* Reset the function value table */
					HOTKEY_Funciton_PerformValueIndex = 0; // start from first value
					HOTKEY_ExOperation_Valid_Cnt = 0;					

				case HOTKEY_PARSER_OPCODE_STAGE:
					if (HOTKEY_Descritpr_Index >= HOTKEY_Funciton_End_Index) //Current Function Table maximan Length
					{
						HOTKEY_Parser_State = HOTKEY_PARSER_FUNCTION_CODE_STAGE;						
						break;
					}

					HOTKEY_Operation_Code = KVM_Flash.HotKey_Descriptor[HOTKEY_Descritpr_Index] & HOTKEY_OPCODE_MASK;
					HOTKEY_Operation_Len = (KVM_Flash.HotKey_Descriptor[HOTKEY_Descritpr_Index] & HOTKEY_OPCODE_LEN_MASK) HOTKEY_OPCODE_LEN_SHIFT;
					HOTKEY_Op_Content_Index = 0;
					HOTKEY_Descritpr_Index++; // opcode content
					HOTKEY_Parser_State = HOTKEY_PARSER_OP_CONTENT_STAGE;
					HOTKEY_Operation_Continue_Flag = 0;

				case HOTKEY_PARSER_OP_CONTENT_STAGE:
					if (HOTKEY_OP_Content_Parser(keycode))  // if opcode match with this content
					{
						HOTKEY_ExOperation_Valid_Cnt++;
						if (HOTKEY_OP_Content_Ternimate_Flag) // if operation has been match & no more keycode expect
						{
							if (HOTKEY_Operation_Continue_Flag == 0)
							{
								HOTKEY_Descritpr_Index += HOTKEY_Operation_Len; // next opcode
								if (hotkey_CheckValueFlag)
								{
									if ((HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex] < HOTKEY_OP_Minmmum_Value) ||
										(HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex] > HOTKEY_OP_Maxmmum_Value))
									{
										HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex] = HOTKEY_VALUE_OVERRUN;
									}
								}
							}
							if (HOTKEY_Descritpr_Index >= HOTKEY_Funciton_End_Index) // if the op content is terminated
							{
								// then cancel the hotkey mode and perform the KVM operation
								// create task --------------
								if (HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex] != HOTKEY_VALUE_OVERRUN)
								{
									printf("HK match: fun=%bx val=%d idx=%bx\n\r",
										HOTKEY_Funciton_Code,
										HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex],
										HOTKEY_Funciton_PerformValueIndex);
									TASK_Active(TASK_TYPE_EVENT,TASK_KVM_EVENT_CONTROL_ID,0,0,0,0); // now aticev the process for time out event
									KVM_Event_Control_Flag = 1;
								}
								else
								{
									printf("HK value overrun: fun=%bx idx=%bx\n\r",
										HOTKEY_Funciton_Code,
										HOTKEY_Funciton_PerformValueIndex);
								}

								HOTKEY_Function_Match_Flag = 1;
								HOTKEY_Deactive();
								break;
							}
							else
							{
								HOTKEY_Function_Continue_Flag = 1;
								if (HOTKEY_Operation_Continue_Flag == 0)
								{
									HOTKEY_Parser_State = HOTKEY_PARSER_OPCODE_STAGE;
									HOTKEY_Funciton_PerformValueIndex++;
								}

								if (Current_Check ==0) //if in check hotkey queue mode
								{
									if (HOTKEY_Function_Code_Index == HOTKEY_Function_Code_Cnt)
									{	
										return;
									}	

									goto HOTKEY_Function_Parser_Restart;
								}
								else	
								    return;
							}
						}
					}
					else
					{
						if (HOTKEY_Function_Match_Flag)  // match fail,cancel the hotkey processing
						{							
							HOTKEY_Function_Match_Flag = 0; //clear the match flag
						}
						else 
						{
							if (HOTKEY_Function_Continue_Flag)
							{
								if ((HOTKEY_Operation_Code == HOTKEY_OP_SHORT_VALUE) || (HOTKEY_Operation_Code == HOTKEY_OP_LONG_VALUE))
								{
									goto FUNCTION_PARSER_CONTINUE;
								}
								else
								{
									HOTKEY_Function_Continue_Flag = 0;
									HOTKEY_Function_Match_Flag = 0;
									goto FUNCTION_PARSER_CONTINUE;
								}
							}
							else
							{
FUNCTION_PARSER_CONTINUE:
								if (HOTKEY_Function_Continue_Flag)
								{											
									HOTKEY_Descritpr_Index += HOTKEY_Operation_Len;   // next opcode
								}
								else
								{		
									HOTKEY_Descritpr_Index = HOTKEY_Descritpr_Index_Hold + HOTKEY_Funciton_Len;   // next opcodeHOTKEY_Operation_Len;  
								}
								
								HOTKEY_Parser_State = HOTKEY_PARSER_OPCODE_STAGE; // process next opecode for next keycode
								if (HOTKEY_Descritpr_Index >= HOTKEY_Funciton_End_Index) // if the op content is terminated
								{
									HOTKEY_Parser_State = HOTKEY_PARSER_FUNCTION_CODE_STAGE; // Check the next possible function
								}
								else
								{
									HOTKEY_Funciton_PerformValueIndex++;
								}
							}
						}

						if ((HOTKEY_Function_Match_Flag==0) && (HOTKEY_Function_Continue_Flag==0))
						{
							Current_Check = 0; //check the hotkey queue
							goto HOTKEY_Function_Parser_Resset;
						}	
					}
					break;
				default:
					break;
			}
		}
	}
}

/*
 * ----------------------------------------------------------------------------
 * void      TASK_HOTKEY_Active_Led_Indicate
 * Purpose : This task will indidate the hostkey status(Active or Deactive)
 *           also will do the hotkey valid time out check if enable timeout
 *           check mode.
 * Params  : NONE
 * Returns : NONE
 * Note    : NONE
 * ----------------------------------------------------------------------------
 */
void TASK_HOTKEY_Active_Led_Indicate(void)
{
	if (HOTKEY_Control_Status & KVM_HOTKEY_ACTIVE) /* hotkey actived */
	{
		if (HOTKEY_LED != 0)
		{
			HOTKEY_LED = 0;
		}
		else
		{
			HOTKEY_LED = KVM_LED_SCROLL_LOCK;
		}

#if (KVM_HOTKEY_TIMEOUT_CHECK)  // do the hotkey valid time out check
#if (EXTENDER_ISL59920_SUPPORT)
		if (bOperateFlag == 0)		
#endif  /* #if (EXTENDER_ISL59920_SUPPORT) */	
		{
			HOTKEY_Active_Valid_Cnt++;
			if (HOTKEY_Active_Valid_Cnt >= KVM_HOTKEY_VALID_TIME)
			{
				HOTKEY_Deactive(); // reset the
				Task_Active_Table[TASK_Active_ID].Task_Reload.w = 1; //Next 1 ms send out the LED
			}
		}	
#endif
	}
	else
	{	// restore the LED status
		HOTKEY_LED = KVM_HostLed[KVM_CurrentHost];
		TASK_Destory_Current();
	}

	#if (KVM_HOTKEY_KB_LED)
	KVM_CONSOLE_Keyboard_Led_Control(HOTKEY_LED);
	#endif
}

/*--------------------------------------------------------------------------------
 * void HOTKEY_Deactive(void)
 * Purpose : Handle the hotkey mode deactive control
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------*/
void HOTKEY_Deactive(void)
{
	HOTKEY_Control_Status = KVM_HOTKEY_IDLE;
	HOTKEY_Control_Status_Last = KVM_HOTKEY_IDLE;
	HOTKEY_Fork_Table_Index=0; /* for next operation */
	hotkey_CheckValueFlag = 0;
	HOTKEY_Function_CheckTableID = 0;

	HOTKEY_LED =  KVM_HostLed[KVM_CurrentHost];
	Task_Active_Table[TASK_KVM_LED_Indicate_ActiveID].Task_Interval.w = 1; // next 1 ms, restore the LED status of keyboard
	/*------------------------------------------*/
#if (KVM_BUZZER_SUPPORT)
	BUZZER_Script_Active(Hotkey_Stop_Sound);
#endif
	/*------------------------------------------*/
}

/*--------------------------------------------------------------------------------
 * void TASK_KVM_HotkeyFork_Timeout(void)
 * Purpose : After user presee the first key which match with hotkey forking, then
 *           use 5 seconds count down to check weither user press second key, if not
 *           then cancel the hotkey forking processing.
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------*/
void TASK_KVM_HotkeyFork_Timeout(void)
{
	if (TASK_Register0 == KVM_Hotkey_Serial)
	{
		if (HOTKEY_FirstKeyTimeOutCheck_Flag)  // Function index add index
		{
			HOTKEY_Fork_Table_Index = 0;
		}
	}
	TASK_Destory_Current();
}

#endif /* KVM_HOTKEY_SUPPORT */
#endif /* End of PROJECT_KVM_CONSOLE_ENABLE */

/* End of kvm_hotkey.c */
