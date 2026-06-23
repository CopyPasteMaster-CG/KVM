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
 * Module Name : kvm_console.h
 * Purpose     : A header file of Console EDID program
 * Author      : Jack Wang
 * Date        :
 * Notes       : 
 * no message
 *
 *================================================================================
 */
#ifndef __KVM_CONSOLE_H
#define __KVM_CONSOLE_H

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
 
 
/* MACRO DECLARATIONS */

/* TYPE DECLARATIONS */
/*----------------------------------------------------------------------------------------------*/
#define  SYSTEM_POWER_STOP_MODE        		2    //
#define  SYSTEM_POWER_SLEEP_MODE       		3    //
//-------------------------------------------------------------------------- 
// Structure declaration
//--------------------------------------------------------------------------
//For VS4210 4 screen mode direction define
#define VS4210_UP				0
#define VS4210_DOWN				1
#define VS4210_LEFT				2
#define VS4210_RIGHT			3
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// cSystemFlag
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define  SYSTEM_BEEPER_MASK      			0x01    //0-OFF,1-ON
#define  SYSTEM_HOTKEY_ALTERNATE_MASK    	0x02    //0-Not Used Ctrl Key,1-Use CTRL Key
//#define  SYSTEM_CTRL_KEY_MASK    			0x02    //0-Not Used Ctrl Key,1-Use CTRL Key
#define  SYSTEM_AUTOSCAN_MODE_MASK    0x0c    //00-Free,01-Power,10-Tag
#define  SYSTEM_JUMP_MODE_MASK        0x10    //0-Jump Mode Free,1-Jump Mode Power
#define  SYSTEM_SECURITY_MASK         0x20    //0-Security mode off,1-Security mode on
#define  SYSTEM_CTRL_FAST_SWITCH_MASK 0x40    //1-Enable ctrlx2, shift to next port
#define  SYSTEM_AUDIO_REPORT_ALL_MASK  		0x80    //1-Report USB Audio to all computer,0-only report to current active

/*
$ SYSTEM_AUTOSCAN_MODE_MASK
*/
#define  SYSTEM_AUTOSCAN_MODE_FREE    0x00
#define  SYSTEM_AUTOSCAN_MODE_POWER   0x04
#define  SYSTEM_AUTOSCAN_MODE_TAG     0x08
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// cSystemFlag_1
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define  SYSTEM_AUDIOCOMBO_MASK  					0x01    //0-OFF,1-ON
#define  SYSTEM_PLUGIN_JUMP_MASK					0x02    //0-OFF,1-ON
#define  SYSTEM_MSC_NOSUPPORT_MASK  				0x04    //1-Not Support, 0- Support
#define  SYSTEM_MSC_OVER_HUB_NOSUPPORT_MASK			0x08    //1-Not Support, 0- Support
#define  SYSTEM_PEN_DRIVE_WRITE_MASK				0x10    //1-can not write,0-can write
#define  SYSTEM_PEN_DRIVE_ATTACH_MASK				0x20    //1-PEN DRIVER Attached 0-PEN DRIVER NOT ATTACHED
#define  SYSTEM_PLUGIN_JUMP_MODE		  			0x40    //0-Plug in jump only to the first,1-always jump to new plug in
#define  SYSTEM_POWERSAVING_MASK		  			0x80    //0-Not Support PowerSaving Mode,1-Support PowerSaving Mode

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// cSystemFlag_2
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define  SYSTEM_TX_BEEPER_MASK  					0x01    //0-OFF,1-ON
#define  SYSTEM_TX_AUDIOCOMBO_MASK 					0x02    //0-OFF,1-ON
#define  SYSTEM_TX_DEVICE_KEEP_MASK 				0x04    //0-OFF,1-ON
#define  SYSTEM_RTC_MASK 							0x08    //0-OFF,1-ON
#define  SYSTEM_ALL_SYNC_MASK 						0x10    //0-OFF,1-ON
#define  SYSTEM_KB_SYNC_MASK 						0x20    //0-OFF,1-ON
#define  SYSTEM_MS_SYNC_MASK 						0x40    //0-OFF,1-ON
#define  SYSTEM_MS_DN_MASK 							0x80    //0-OFF,1-ON
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// cSystemFlag_3
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define  SYSTEM_VHID_MODE_MASK		 				0x03    //Use the virtual HID device to tranfer the Keyboard and mouse data
#define  SYSTEM_VHID_MS_MASK 	        	        0x01
#define  SYSTEM_VHID_KB_MASK 			  	        0x02

#define  SYSTEM_MPASS_MASK				  	        0x0C	//0-Middle Button,1-Pass-Through Mode
#define  MPASS_DISABLE_MASK				  	        0x00	
#define  MPASS_BUTTON_MODE							0x04
#define  MPASS_ACROSS_MODE							0x08


#define  SYSTEM_HID_MOUSE_MODE_MASK  				0x10    //0-Relative,1-Absolute
#define  SYSTEM_MOUSE_SW_MASK 		 				0x20
#define  KM_HOTKEY_MASK								0x40
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define  AUTOSCAN_STOP_CODE      0xff
#define  AUTOSCAN_STOP_AND_JUMP  CHAR_SPACE

#define R_COLOR 		0
#define G_COLOR 		1
#define B_COLOR 		2

struct _KVM_OSD {
	U8_T  CurrentUser;         // 1  User ID
	U8_T  MenuStartX;          // 2
	U8_T  MenuStartY;          // 3
	U8_T  BannerTime;          // 4
	U8_T  BannerStartX;        // 5
	U8_T  BannerStartY;        // 6
	U8_T  MenuLanguge;         // 7              
	U8_T  SecurityTime;    	   // 8  
	U8_T  ScreenSaveTime;      // 9 
	U8_T  ScanTag[4];          // 10,11,12,13            
	U8_T  DVR_Flag[4];         // 14,15,16,17  00-None PS2_DVR,01-PS2_DVR	    	  
	U8_T  UserControl[32];     // 18~50 User Port control
	U8_T  UserPassword[10][6]; // 51~110 User Password
	U8_T  UserName[10][14];    // 111~250 Adminstrator,User1~9
	// Server Name Parameter -----------------------------------------------------------
	U8_T  ServerName[16][20];  // 251~570 Server Name
};

typedef struct
{
	U8_T    KB_HotkeyLead_1; 			//All hotkey key
	U8_T    KB_HotkeyLead_2; 			//only for Repeat Setting
	U8_T    KB_Repeat;		
	U8_T    KB_RepeatTable[8];
	U8_T    KB_HID_RepeatTableMake[8];
	U8_T    KB_HID_RepeatTableBreak[8];
} KM_SYNC_Typedef;

typedef struct {
	U16_T  iVid;         // 1  User ID
	U16_T  iPid;         // 2	
} KVM_VPid_Typedef;

struct _KVM_Flash {
	// System Parameter ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	U16_T CRC_Check;
	U16_T Total_Length;
	U16_T AutoScanInterval;    		// 0-1 autoscan interval timer 5-999   0-1
	U8_T  AutoScanStopCode;    		// 2
	U8_T  AutoScanStop_JumpCode; 	// 3
	U8_T  cSystemFlag0;        		// 5-System Flag1
	U8_T  cSystemFlag1;        		// 6-System Flag2
	U8_T  PowerSavingMode;	        // 7:2-Not Stop Clock 3-Stop Clock

	/* Hotkey Start Key Sequence Setting----------------------*/
	KVM_HotkeyStruct  Hotkey_Active_Descriptor[20];  //8~47 20x2=40 bytes
	U8_T  HotKey_Descriptor[250]; // 48~176 Hotkey Function Descriptor
	U8_T  Color_Delay[3];
	U8_T  cSystemFlag2;        		// Store the Extender parameter
	U8_T  cSystemFlag3;        		// Store the Extender parameter
	U16_T Version;
	U8_T  VPid_Change_State;	
	KVM_VPid_Typedef VPid[4]; 
	U8_T  Useless[3];
	//U8_T  Edid_Table[256];		
};

/*
$ KVM_Led 
*/
#define KVM_LED_NUM_LOCK		0x01
#define KVM_LED_CAPS_LOCK		0x02
#define KVM_LED_SCROLL_LOCK		0x04
#define KVM_LED_COMPOSE			0x08
#define KVM_LED_KANA			0x10

#define KVM_LED_PS2_NUM_LOCK		0x02
#define KVM_LED_PS2_CAPS_LOCK		0x04
#define KVM_LED_PS2_SCROLL_LOCK		0x01

/*
$ KVM_Control_Status
*/
#define KVM_IDLE_STATUS                 0x0000
#define KVM_PORT_SW_MASK                0x0001
//#define KVM_AUDIO_SW_MASK               0x0002
//#define KVM_USB_SW_MASK                 0x0004
//#define KVM_HUB_SW_MASK                 0x0008
//#define KVM_AUDIO_AUTO_MASK             0x0010
//#define KVM_BUZZER_MASK                 0x0020
//#define KVM_EDID_MASK                   0x0040
//#define KVM_RESET_MASK                  0x0080
//#define KVM_CASCADE_JUMP_MASK           0x0100
//#define KVM_DAISY_JUMP_MASK             0x0200
#define KVM_AUTOSCAN_MASK               0x0400
//#define KVM_AUTOSCAN_INTERVAL_MASK      0x0800


/*
$ KVM Select LED Control
*/
/*
#if (MCU_REAL_CHIP)
	#ifdef MCU_TYPE_AX68002
		#define VGA_CONTROL_PORT               GPIOC
		#define SELECT_LED_PORT                GPIOC
		#define HOST_LED_PORT                  GPIOC
	#else
		#define VGA_CONTROL_PORT               GPIOB
		#define SELECT_LED_PORT                GPIOC
		#define HOST_LED_PORT                  GPIOC
	#endif
#else
	#define VGA_CONTROL_PORT               		GPIOA
	#define SELECT_LED_PORT                		GPIOB
	#define HOST_LED_PORT                  		GPIOB
#endif
*/

#define LED_OFF						0x01
#define LED_ON						0x00

#define VGA_OFF						0x00
#define VGA_ON						0x01
/*
$ Macro Function
*/

/* GLOBAL VARIABLES */

extern U8_T cModifiers;
#if (SYSTEM_PS2_HOST_ENABLE)
extern U8_T cPS2_KB_PressCnt;
#endif
extern U8_T	cUSB_KB_PressCnt;
extern U8_T KVM_HostLed[KVM_MAX_PORT];
//extern U8_T KVM_HostConnectStatus[KVM_MAX_PORT];
extern U8_T KVM_CurrentHost,KVM_CurrentAudio,KVM_CurrentMSC,KVM_CurrentUSBAudio;
extern U8_T KVM_NextHost,KVM_NextAudio,KVM_NextMSC,KVM_NextAnalogAudio;
extern struct _KVM_Flash KVM_Flash;
extern struct _KVM_Flash KVM_FlashTemporary;
extern U8_T TASK_KVM_EVENT_CONTROL_ID;
extern bit  KVM_Event_Control_Flag;
extern U8_T TASK_KVM_AUTOSCAN_ID;
extern U16_T KVM_Control_Status;
extern U8_T  TASK_KVM_CONSOLE_KEYBOARD_LED_CONTROL_ID;
#if (SYSTEM_PS2_HOST_ENABLE)
extern bit   KVM_PS2_HotkeySwitch_Flag;
#endif
extern U8_T	TASK_KVM_AUTOSCAN_ACTIVE_ID;
extern U8_T TASK_KVM_ACCESSORY_ID;
extern bit 	KVM_System_Not_Ready_Flag;
extern bit 	KVM_Plug_In_Jump_Flag;
//extern U8_T KVM_USB_Keycode;
//extern U8_T KVM_USB_Keytype; 

extern code GPIO_PinTypeDef SELECT_LED[];
extern code GPIO_TypeDef	*SELECT_LED_PORT[];


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
extern void KVM_Console_Init(void);
//extern void KVM_Check_KeyboardControl(U8_T,U8_T);
#if (SYSTEM_PS2_HOST_ENABLE)
extern void KVM_CheckPS2Modifiers(U8_T,U8_T);
//extern void KVM_PS2_Convert_USB(U8_T,U8_T,U8_T *);
#endif
//extern void KVM_Check_Hotkey(U8_T,U8_T);
//extern void KVM_HotkeyActive_Indicate(void);
#if (SYSTEM_PS2_HOST_ENABLE)
extern U8_T KVM_Hostkey_LED_PS2_Convert(U8_T);
#endif
extern void TASK_KVM_Event_Control(void);
extern void TASK_KVM_Switch_Event_Control(void);
//extern void TASK_KVM_Switch_Audio_Control(void);
//extern void TASK_KVM_Switch_MSC_Control(void);
extern void TASK_KVM_Autoscan_Event_Control(void);
extern void KVM_Autoscan_KB_Handle(U8_T,U8_T);
extern U8_T KVM_Check_KB_Skip_Condition(void);
void KVM_CONSOLE_Vga_Control(U8_T port,U8_T vga_control);
void KVM_Host_Led_Control(U8_T,U8_T);
void KVM_PlugIn_Jump_Check(U8_T pid);
U8_T KVM_Power_Jump_Check(U8_T pid,U8_T checknext); 
extern void KVM_CONSOLE_Keyboard_Led_Control(U8_T kbled);
void TASK_KVM_Switch_Event_Control_Fork(U8_T next_port,U8_T interval);
void KVM_Jump_Next_Power_Port(U8_T pid);
U8_T KVM_Check_MS_Skip_Condition(void);
void KVM_Audio_Combo_Switch_Check(U8_T newport);
void KVM_Audio_Switch(U8_T newport);
void KVM_AutoscanStop_Jump(U8_T jumpflag,U8_T newport);
void KVM_Select_Led_Control(U8_T,U8_T);
void KVM_Port_Jump(U8_T newport);
U8_T KVM_Console_Port_Jump(U8_T new_port);
U8_T KVM_Power_Status_Check(U8_T start_pid,U8_T end_pid);
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) && (SYSTEM_SWITCH_AUDIO_DELAY)	 
void KVM_CONSOLE_Switch_Port_Audio_Control(U8_T);
#endif
#ifdef HDMI_KVM
void KVM_CONSOLE_HPD_Control(U8_T vga_control);
#endif
void KVM_Autoscan_Mode_Contorl(U8_T control);

#ifdef SYNC
U8_T KM_SYNC_Setting(U8_T fun_code,U16_T value);
#endif

void KVM_Console_Mouse_Cross_Jump(U8_T direction);
void KVM_CONSOLE_IAP_Mode(void);
void KVM_System_Config_Write(void);
#endif /* End of __KVM_CONSOLE_H */

