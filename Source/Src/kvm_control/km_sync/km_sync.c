/*
 ******************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is an proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
 /*============================================================================
 * Module Name: km_sync.c
 * Purpose:
 * Author:
 * Date:
 *=============================================================================
 */
/* 
*/

/* INCLUDE FILE SECTION */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "project_include.h"


#ifdef SYNC
/* NAMING CONSTANT DECLARATIONS */
#define DEFAULT_HOTKEY_HEADER				CHAR_CONTROL	
#define DEFAULT_HOTKEY_HEADER2				CHAR_CONTROL	

#define ACROSS_LED_INTERVAL					500
#define KVM_MS_LEFT_BTN						0x01
#define KVM_MS_RIGHT_BTN					0x02
#define KVM_MS_MIDDLE_BTN					0x04

// For KM_SYNC_State
#define KM_SYNC_NUM_LED_SET					0x01

/*
$ U8_T    KM_SYNC_Sync_MS_HotkeyState;
*/
#define KM_SYNC_MS_HOTKEY_SET				0x01
#define KM_SYNC_MS_DN_SET					0x02

#define KM_SYNC_MS_HOTKEY_LEFT				0x10
#define KM_SYNC_MS_HOTKEY_RIGHT				0x20

/*
$ U8_T    KM_SYNC_Sync_KB_HotkeyState;
*/
#define KM_SYNC_KB_HOST_SET					0x01
#define KM_SYNC_KB_REPEAT_SET				0x02
#define KM_SYNC_KB_ACROSS_SET				0x04
#define KM_SYNC_KB_DCRESET_SET				0x08

/*
$ U8_T    KM_SYNC_Hotkey_Function[0]
*/
#define  KB_HOTKEY_DUAL_HEADER_MODE			0xf0
#define  KB_HOTKEY_ABSOLUTE_MODE			1		
#define  KB_HOTKEY_RELATIVE_MODE			2		
#define  KB_HOTKEY_STORAGE_MODE				3		
#define  KB_HOTKEY_RECORD_MODE				4		
#define  KB_HOTKEY_RECORD_STOP_MODE			5		
#define  KB_HOTKEY_PLAY_MODE				6		
#define  KB_HOTKEY_PLAY_STOP_MODE			7		
#define  KB_HOTKEY_STOP_MODE				8		

#define  KB_HOTKEY_SYNC_MODE				9		
#define  KB_HOTKEY_JUMP_MODE				10		
#define  KB_HOTKEY_ACROSS_MODE				11	
#define  KB_HOTKEY_REPEAT_CLS_ALL_MODE		12		
#define  KB_HOTKEY_REPEAT_SET_KEY_MODE		13		
#define  KB_HOTKEY_REPEAT_CLS_KEY_MODE		14		
#define  KB_HOTKEY_DC_RESET_MODE			15
#define  KB_HOTKEY_REPEAT_KEY_MODE			16
#define  KB_HOTKEY_VPID_CHANGER_MODE		17
#define  KB_HOTKEY_SYNC_PORT_ADD_MODE		18		
#define  KB_HOTKEY_SYNC_PORT_CLEAR_MODE		19
#define  KB_HOTKEY_IAP_MODE					20
#define  KB_HOTKEY_ROAMING_MAP				21
#define  KB_HOTKEY_ROAMING_MAP2				22

#define  ROAMING_MAP2_KEY					CHAR_M

/* GLOBAL VARIABLES DECLARATIONS */
bit		KM_SYNC_HOTKEY_Skip_Flag=0;
bit		KM_SYNC_HOTKEY_Clear_Flag=0;
bit		KM_SYNC_HOTKEY_Reset_Flag=0;
U8_T	KM_SYNC_HOTKEY_Modifiers = 0;
U8_T    KM_SYNC_Roaming_Control=SYNC_ROAMING_AUTO;
U8_T	KM_SYNC_Roaming_DelayCnt=COORDINATE_COUNTER; 
U8_T    KM_SYNC_Roaming_Edge=EDGE_ROAMING_LEFT|EDGE_ROAMING_RIGHT|EDGE_ROAMING_TOP|EDGE_ROAMING_BOTTOM;
U8_T    KM_SYNC_WORKING_MODE=0;
U8_T    KM_SYNC_Init_Led = KVM_LED_NUM_LOCK;
U8_T    KM_SYNC_HOTKEY_Reset_Cnt=0;
U8_T    KM_Hotkey_Leading_Key=CHAR_SCROLL_LOCK;
U8_T    KM_HOTKEY_LED = 0;
U8_T    KM_HOTKEY_Active_Valid_Cnt=0;
U8_T    Mouse_Btn;
U8_T  	*MouseXb,*MouseYb;	
U8_T  	Coordinate_Cnt[4]={0,0,0,0};
U8_T    KM_SYNC_Hotkey_Function[2];
U8_T    KM_SYNC_Hotkey_ScanCode[2];
U8_T	KM_SYNC_HOTKEY_Active_Valid_Cnt=0;
U8_T	KM_SYNC_Hotkey_Q_Size;
//struct _KM_SYNC_HOTKEY	KM_SYNC_KB_Hotkey_Tab[4];
U8_T	KM_SYNC_KbLed_Set_State;
U8_T	KM_SYNC_KbLed_Init_State;
U8_T	KM_SYNC_KbLed = 0;
U8_T    KM_SYNC_Sync_MS_HotkeyInx;
U8_T    KM_SYNC_Sync_MS_HotkeyState;
U8_T    KM_SYNC_Sync_KB_HotkeyInx;
U8_T    KM_SYNC_Sync_KB_HotkeyState;
U8_T    KM_SYNC_Sync_KB_HotkeyChar;
U8_T    KM_SYNC_Sync_KB_Repeat = 0;
U8_T    KM_SYNC_Sync_KB_Repeat_Devinx = 0;
U8_T    KM_SYNC_Sync_KB_RepeatActive[KVM_MAX_PORT];
//U8_T    KM_SYNC_Sync_KB_RepeatActive_Bk;
U8_T    KM_SYNC_Sync_KB_RepeatTable[KM_SYNC_REPEAT_SIZ] = {0,0,0,0,0,0,0,0};
U8_T    KM_SYNC_Sync_KB_HID_RepeatTableMake[KVM_MAX_PORT][KM_SYNC_REPEAT_SIZ];
U8_T    KM_SYNC_Sync_KB_HID_RepeatTableBreak[KM_SYNC_REPEAT_SIZ];
U8_T    KM_SYNC_Sync_Host;
U8_T    KM_SYNC_Absolute_Coordinate[6];
U8_T 	TASK_KM_SYNC_Across_Led_ID;
U8_T 	TASK_KM_SYNC_Across_Led_ActiveID;
//U8_T	TASK_KM_SYNC_Active_Delay_ID;
U8_T	TASK_KM_SYNC_DN_Active_Delay_ID;
U8_T	TASK_KM_SYNC_DCReset_ID;
U8_T    TAKS_KM_SYNC_RepeatKeySend_ID;
//U8_T    TASK_KM_HOTKEY_LED_INDICATE_ID;
U8_T	Hotkey_Queue[8];
U8_T    KM_SYNC_PortSetting = 0x00; //port 0~3 is sync
U8_T    MouseZ;
U16_T 	MouseX= (X_MAX >> 1);
U16_T 	MouseY= (Y_MAX >> 1);
U16_T 	Coordinate_State = 0;	
//---------------------------------------------------------------------------
//For Roaming mode control
API_Roaming_Mapping_Def  *Roaming[API_ROAMING_TABLE_MAX]={0,0,0,0,0};
U8_T	Current_Roaming_Table = 0; //default pointer 0
U8_T	Current_Roaming_Direction;
U16_T	ROAMING_CROSS_MARGIN_X[4] ={(X_MAX-100),(X_MAX-100),100		   ,100};  
U16_T	ROAMING_CROSS_MARGIN_Y[4] ={(Y_MAX-100),100        ,(Y_MAX-100),100};

#ifdef VPID_CHANGER
U16_T	VPID_Changer_Seed;
U8_T	VPID_Changer_State=0;
#endif
/* LOCAL VARIABLES DECLARATIONS */
/* LOCAL SUBPROGRAM DECLARATIONS */
void TASK_KM_SYNC_Across_Led(void);
void KM_SYNC_ModeSync_Control(U8_T syncmode);
void TASK_KM_SYNC_Active_Delay(void);
U8_T KM_SYNC_Search_Repeat_Table(U8_T table,U8_T keycode);
void KM_SYNC_Repeat_Key_Handle(U8_T keytype,U8_T keycode);
void KM_SYNC_Send_KB_Hid_Key(U8_T port,U8_T usbcode);
void TASK_KM_SYNC_DN_Active_Delay(void);
void TASK_KM_SYNC_DCReset(void);
void KM_SYNC_Reset_Upsteam(void);
void KM_Power_Led_Control(U8_T led_contorl);
void KM_SYNC_Send_VHID_Key(U8_T keytype,U8_T keycode);
void KM_SYNC_KB_Hotkey_Function_3_ThirdKeyCheck(U8_T keytype,U8_T keycode);
void TAKS_KM_SYNC_RepeatKeySend(void);
void TASK_KM_HOTKEY_Active_Led_Indicate(void);
void KM_SYNC_Force_Port_Jump(U8_T port);
void KM_SYNC_Port_Add(U8_T port);
void KM_SYNC_Port_Clear(U8_T port);
void TASK_KM_SYNC_DCReset(void);
U8_T KM_SYNC_SyncMode_State(void);
void KM_SYNC_Set_SyncMode(void);
void KM_SYNC_Manufacturer_Default(void);
void KM_SYNC_Handle_SetRepeatKey(U8_T keycode);
void KM_Sync_VPid_Change_Handle(U8_T port_mask);
void KM_SYNC_Stop_KB_Report(void);
U8_T KM_SYNC_Check_Digit_Function(U8_T keycode);
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
/* EXTERNAL SUBPROGRAM DECLARATIONS */

/** ----------------------------------------------------------------------------
 * Function Name: KM_SYNC_Init(void)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void KM_SYNC_Init(void)
{
	U8_T intt_id;
	
	//For Absolute mouse usage
	MouseXb = (U8_T *)&MouseX; 
	MouseYb = (U8_T *)&MouseY;
	
	KM_SYNC_KbLed_Set_State = 0;
	KM_SYNC_KbLed_Init_State = 0;
	KM_SYNC_Sync_MS_HotkeyState = 0;
	KM_SYNC_Sync_MS_HotkeyInx  = 0;
	KM_SYNC_Sync_Host = 0x00; //all pc is avaiable in sync mode
	KM_SYNC_Sync_KB_HotkeyInx = 0;
    KM_SYNC_Sync_KB_HotkeyState = 0;
	//KM_SYNC_Sync_KB_HotkeyTable = 0;	
	memset(KM_SYNC_Sync_KB_RepeatActive,0,sizeof(KM_SYNC_Sync_KB_RepeatActive));
	memset(KM_SYNC_Sync_KB_HID_RepeatTableMake,0,sizeof(KM_SYNC_Sync_KB_HID_RepeatTableMake));
	memset(KM_SYNC_Sync_KB_HID_RepeatTableBreak,0,sizeof(KM_SYNC_Sync_KB_HID_RepeatTableBreak));
	KM_SYNC_Hotkey_Function[0] = 0;
	
	intt_id = USB_PDevice[USBDC_VHID_DEVINX].EndpType[0][0] & 0x1f;
	memset(HC_IntTransfer_Table[intt_id].PassControl.SyncWp,0x00,USBDC_PORT_MAX);		
	memset(HC_IntTransfer_Table[intt_id].PassControl.SyncRp,0x00,USBDC_PORT_MAX);		
	
	intt_id = USB_PDevice[USBDC_VHID_DEVINX].EndpType[1][0] & 0x1f;	
	memset(HC_IntTransfer_Table[intt_id].PassControl.SyncWp,0x00,USBDC_PORT_MAX);		
	memset(HC_IntTransfer_Table[intt_id].PassControl.SyncRp,0x00,USBDC_PORT_MAX);			
	
	TASK_KM_SYNC_Across_Led_ID = TASK_Create(TASK_KM_SYNC_Across_Led);
	//TASK_KM_SYNC_Active_Delay_ID = TASK_Create(TASK_KM_SYNC_Active_Delay);
	TASK_KM_SYNC_DN_Active_Delay_ID = TASK_Create(TASK_KM_SYNC_DN_Active_Delay);
	TAKS_KM_SYNC_RepeatKeySend_ID = TASK_Create(TAKS_KM_SYNC_RepeatKeySend);
	TASK_KM_SYNC_DCReset_ID = TASK_Create(TASK_KM_SYNC_DCReset);
	TASK_KM_SYNC_Across_Led_ActiveID = 0;
	
	//if (KVM_Flash.cSystemFlag2 & SYSTEM_ALL_SYNC_MASK)
	//{
	//	KM_SYNC_KM_SyncModeLed(LED_ON);				
	//}		
	
	//KM_Power_Led_Control(LED_ON);
	//TASK_KM_HOTKEY_LED_INDICATE_ID = TASK_Create(TASK_KM_HOTKEY_Active_Led_Indicate);
	
	//Please define the init key value in API_Init()
	//KVM_Flash.cSystemFlag2 &= ~(SYSTEM_ALL_SYNC_MASK | SYSTEM_KB_SYNC_MASK | SYSTEM_MS_SYNC_MASK | SYSTEM_MS_DN_MASK);	
	//KVM_Flash.cSystemFlag3 |= (MPASS_ACROSS_MODE | SYSTEM_MOUSE_SW_MASK);	
}

/** ----------------------------------------------------------------------------
 * Function Name: KM_SYNC_KB_Led_Send(U8_T port,U8_T led_id)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void KM_SYNC_KB_Led_Send(U8_T port,U8_T led_id)
{
	U8_T i;
	
	for (i=0; i < 3; i++)
	{
		if (led_id & BIT_MASK[i])
		{ 
			switch (BIT_MASK[i])
			{	
				case KVM_LED_NUM_LOCK:
					//printf("Number LED Toggle\n\r");
					KM_SYNC_Send_KB_Hid_Key(port,CHAR_NUM_LOCK);
					//DATAST_Keyboard_Send(port,HID_USAGE_PAGE_07_MAKE,CHAR_NUM_LOCK);
					//DATAST_Keyboard_Send(port,HID_USAGE_PAGE_07_BREAK,CHAR_NUM_LOCK);				
					break;
				case KVM_LED_CAPS_LOCK:
					//printf("CAPS LED Toggle\n\r");
					KM_SYNC_Send_KB_Hid_Key(port,CHAR_CAPS_LOCK);
					//DATAST_Keyboard_Send(port,HID_USAGE_PAGE_07_MAKE,CHAR_CAPS_LOCK);
					//DATAST_Keyboard_Send(port,HID_USAGE_PAGE_07_BREAK,CHAR_CAPS_LOCK);				
					break;
				case KVM_LED_SCROLL_LOCK:
					//printf("Scroll LED Toggle\n\r");
					KM_SYNC_Send_KB_Hid_Key(port,CHAR_SCROLL_LOCK);
					//DATAST_Keyboard_Send(port,HID_USAGE_PAGE_07_MAKE,CHAR_SCROLL_LOCK);
					//DATAST_Keyboard_Send(port,HID_USAGE_PAGE_07_BREAK,CHAR_SCROLL_LOCK);				
					break;
			}	
		}					
	}		
}	
/**----------------------------------------------------------------------------
 * Function Name: KM_SYNC_Start(U8_T led,U8_T port)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ---------------------------------------------------------------------------*/
void KM_SYNC_Start(U8_T port)
{	
	U8_T i;
	
	//1.Check current LED setting
	//printf("SYNC LED,Set=%02bx,Port=%bu,Led=%02bx\n\r",KM_SYNC_KbLed_Set_State,port,KVM_HostLed[port]);
	if (KVM_Flash.cSystemFlag2 & (SYSTEM_ALL_SYNC_MASK | SYSTEM_KB_SYNC_MASK))
	{			 
		if ((KM_SYNC_KbLed_Set_State & BIT_MASK[port]) == 0x00) // not in LED set state
		{
			//printf("Init0:%02bx",KM_SYNC_Init_Led);
			if (KVM_HostLed[port] != KM_SYNC_Init_Led) // only Numner LED is on				
			{														
				for (i=0; i < 3; i++)
				{
					if (KM_SYNC_Init_Led & BIT_MASK[i])
					{							
						if ((KVM_HostLed[port] & BIT_MASK[i]) == 0)
						{
							KM_SYNC_KB_Led_Send(port,BIT_MASK[i]);
						}
					}	
					else
					{
						if (KVM_HostLed[port] & BIT_MASK[i])
						{
							KM_SYNC_KB_Led_Send(port,BIT_MASK[i]);
						}
					}						
				}						
			}			
		}	
		KM_SYNC_KbLed_Set_State |= BIT_MASK[port]; //setting done
		//KM_SYNC_System_Ready_Led();
	}	
}

/**----------------------------------------------------------------------------
 * Function Name: KM_SYNC_ClearStart(U8_T port)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void KM_SYNC_ClearStart(U8_T port)
{		
	KM_SYNC_KbLed_Set_State  &= ~BIT_MASK[port];		
	KM_SYNC_KbLed_Init_State &= ~BIT_MASK[port];		
}

/**----------------------------------------------------------------------------
 * Function Name: KM_SYNC_Mouse_Absolute_Mode(void)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void KM_SYNC_Mouse_Absolute_Mode(void)
{
	if (KVM_Flash.cSystemFlag2 & SYSTEM_MS_DN_MASK)	
	{	
		TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_KM_SYNC_DN_Active_Delay_ID,0x00,0,100,1);	
	}	
}

/**----------------------------------------------------------------------------
 * Function Name: KM_SYNC_Mouse_Relative_Mode(void)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void KM_SYNC_Mouse_Relative_Mode(void)
{
	if ((KVM_Flash.cSystemFlag2 & SYSTEM_MS_DN_MASK) == 0x00)	
	{	
		TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_KM_SYNC_DN_Active_Delay_ID,0x00,1,100,1);
	}	
}
#ifdef SYNC_MS_HOTKEY
/**----------------------------------------------------------------------------
 * Function Name: void KM_SYNC_MS_Hotkey_Clear(void)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void KM_SYNC_MS_Hotkey_Clear(void)
{
	KM_SYNC_Sync_MS_HotkeyState &= ~(KM_SYNC_MS_HOTKEY_SET|KM_SYNC_MS_HOTKEY_LEFT|KM_SYNC_MS_HOTKEY_RIGHT);
	KM_SYNC_Sync_MS_HotkeyInx = 0;	
}
	
/**----------------------------------------------------------------------------
 * Function Name: KM_SYNC_MS_Hotkey_Check(USB_MS_Data_TypeDef *ms_data)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void KM_SYNC_MS_Hotkey_Check(U8_T ms_btn)
{
	////printf("[M %02bx,%02bx]",ms_btn,KM_SYNC_Sync_MS_HotkeyState);
	//check button
	if ((KM_SYNC_Sync_MS_HotkeyState & (KM_SYNC_MS_HOTKEY_SET))== 0)
	{			
		//check Mouse hotkey
		if (((ms_btn & 0x07) == KVM_MS_MIDDLE_BTN) && ((KVM_Flash.cSystemFlag2 & SYSTEM_ALL_SYNC_MASK) == 0x00))
		{			
			KM_SYNC_Sync_MS_HotkeyState |= KM_SYNC_MS_HOTKEY_SET;
			KM_SYNC_Sync_MS_HotkeyInx = 1;						
		}			
	}	
	else
	{
		if (KM_SYNC_Sync_MS_HotkeyState & KM_SYNC_MS_HOTKEY_SET)
		{	
			switch (KM_SYNC_Sync_MS_HotkeyInx)
			{
				case 1: //left button down
					if ((ms_btn & 0x07) == KVM_MS_LEFT_BTN)
					{
						break;
					}			
					
					if ((ms_btn & 0x07) == KVM_MS_RIGHT_BTN)
					{
						break;
					}
					
					if ((ms_btn & 0x07) == (KVM_MS_LEFT_BTN|KVM_MS_MIDDLE_BTN))
					{
						KM_SYNC_Sync_MS_HotkeyState |= KM_SYNC_MS_HOTKEY_LEFT;
						KM_SYNC_Sync_MS_HotkeyInx = 2;												
						break;
					}	
					else if ((ms_btn & 0x07) == (KVM_MS_RIGHT_BTN|KVM_MS_MIDDLE_BTN))
					{
						KM_SYNC_Sync_MS_HotkeyState |= KM_SYNC_MS_HOTKEY_RIGHT;
						KM_SYNC_Sync_MS_HotkeyInx = 2;						
						break;
					}
					KM_SYNC_MS_Hotkey_Clear();
					break;
				case 2://left/right button down+middle button down
					if ((ms_btn & 0x07) == (KVM_MS_LEFT_BTN|KVM_MS_MIDDLE_BTN))
					{
						if (KM_SYNC_Sync_MS_HotkeyState & KM_SYNC_MS_HOTKEY_LEFT)
							break;
					}			
					else if ((ms_btn & 0x07) == (KVM_MS_RIGHT_BTN|KVM_MS_MIDDLE_BTN))
					{
						if (KM_SYNC_Sync_MS_HotkeyState & KM_SYNC_MS_HOTKEY_RIGHT)
							break;
					}			
					
					if (KM_SYNC_Sync_MS_HotkeyState & KM_SYNC_MS_HOTKEY_LEFT)
					{	
						if ((ms_btn & 0x07) == KVM_MS_MIDDLE_BTN)
						{
							KM_SYNC_Sync_MS_HotkeyInx = 3; 
							break;
						}						
					}	
					else if (KM_SYNC_Sync_MS_HotkeyState & KM_SYNC_MS_HOTKEY_RIGHT)
					{	
						if ((ms_btn & 0x07) == KVM_MS_MIDDLE_BTN)
						{
							KM_SYNC_Sync_MS_HotkeyInx = 3; 
							break;
						}						
					}
					KM_SYNC_MS_Hotkey_Clear();
					break;
				case 3:
					if ((ms_btn & 0x07) == KVM_MS_MIDDLE_BTN)
					{					
						break;
					}
					
					if ((ms_btn & 0x07) == 0)
					{
						if (KM_SYNC_Sync_MS_HotkeyState & KM_SYNC_MS_HOTKEY_LEFT)
						{
							//if (KM_SYNC_Sync_MS_HotkeyState & KM_SYNC_MS_HOTKEY_LEFT)
							{ 
								KM_SYNC_ModeSwitch_Control(API_SWITCH_ENABLE);
								KM_SYNC_Port_Jump_Power(KVM_CurrentHost,ROAMING_LEFT,0);  									
							}								
						}
						else if (KM_SYNC_Sync_MS_HotkeyState & KM_SYNC_MS_HOTKEY_RIGHT)
						{
							//if (KM_SYNC_Sync_MS_HotkeyState & KM_SYNC_MS_HOTKEY_LEFT)
							{
								KM_SYNC_ModeSwitch_Control(API_SWITCH_ENABLE);
								KM_SYNC_Port_Jump_Power(KVM_CurrentHost,ROAMING_RIGHT,0);								
							}
						}							
					}
					KM_SYNC_MS_Hotkey_Clear();
					break;				
			}			
		}
	}		
}	
#endif /* #ifdef SYNC_MS_HOTKEY */
/**----------------------------------------------------------------------------
 * Function Name: KM_SYNC_KB_Hotkey_Clear(void)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void KM_SYNC_KB_Hotkey_Clear(void)
{	
	KM_SYNC_Sync_KB_HotkeyState = 0;
	KM_SYNC_Sync_KB_HotkeyInx = 0;
	//KM_SYNC_Sync_KB_HotkeyTable = 0;
	KM_SYNC_Hotkey_Function[0] = 0;
	KM_SYNC_Hotkey_Function[1] = 0;
	KM_SYNC_Hotkey_Function[2] = 0;
	KM_SYNC_HOTKEY_Skip_Flag = 0;
}	

/**----------------------------------------------------------------------------
 * Function Name: KM_SYNC_ModeAccross_Control(UT8_T ctrl)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void KM_SYNC_ModeAccross_Control(U8_T ctrl)
{
	if (ctrl) //mode on
	{
		if ((KVM_Flash.cSystemFlag3 & MPASS_ACROSS_MODE) == 0x00)
		{	
			KM_SYNC_ModeSync_Control(0);			
			KVM_Flash.cSystemFlag3 &= ~SYSTEM_MPASS_MASK;
			KVM_Flash.cSystemFlag3 |= (MPASS_ACROSS_MODE | SYSTEM_MOUSE_SW_MASK);								
			printf("Mouse Roaming Mode Start\n\r");
			KM_SYNC_KM_AcrossModeLed(KVM_CurrentHost);
			#if (KVM_BUZZER_SUPPORT)
			BUZZER_Script_Active(Hotkey_Active_Sound);
			#endif
		}	
	}		
	else
	{
		if (KVM_Flash.cSystemFlag3 & MPASS_ACROSS_MODE)
		{				
			KVM_Flash.cSystemFlag3 &= ~SYSTEM_MPASS_MASK;			
			printf("Mouse Roaming Mode Stop\n\r");
			#if (KVM_BUZZER_SUPPORT)
			BUZZER_Script_Active(Hotkey_Active_Sound);
			#endif
		}	
	}		
}

/**----------------------------------------------------------------------------
 * Function Name: KM_SYNC_KM_SyncModeLed(void)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void KM_SYNC_KM_SyncModeLed(U8_T led_ctrl)
{
	U8_T i;
	U8_T led=led_ctrl;
	
	for (i=0; i < KVM_MAX_PORT; i++)
	{
		if (led_ctrl == LED_ON)
		{	
			if (USBDC_UpPortState[i] & USBDC_ROOTHUB_ATTACHED_MASK)
			{					
				if (KVM_Flash.cSystemFlag2 & SYSTEM_ALL_SYNC_MASK)
				{
					if (KM_SYNC_PortSetting & BIT_MASK[i])					
						led = LED_OFF;
					else
						led = LED_ON;					
				}
				else
				{
					if (i != KVM_CurrentHost)
						led = LED_OFF;
				}					
				KVM_Select_Led_Control(i,led);				
			}	
			else
			{
				if (KVM_Flash.cSystemFlag2 & SYSTEM_ALL_SYNC_MASK)
					KVM_Select_Led_Control(i,LED_OFF);
				else
				{
					if (i != KVM_CurrentHost)
					{	
						KVM_Select_Led_Control(i,LED_OFF);
					}	
					else
					{
						KVM_Select_Led_Control(i,LED_ON);
					}						
				}					
			}				
		}	
		else
		{	
			KVM_Select_Led_Control(i,led_ctrl);
		}	
	}
}	

/**----------------------------------------------------------------------------
 * Function Name: KM_SYNC_Select_Led_Control(U8_T port)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void KM_SYNC_Select_Led_Control(U8_T port)
{	
	KM_SYNC_KM_SyncModeLed(LED_OFF);		
	KVM_Select_Led_Control(port,LED_ON);
}

/**----------------------------------------------------------------------------
 * Function Name: KM_SYNC_KM_AcrossModeLed(U8_T port)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void KM_SYNC_KM_AcrossModeLed(U8_T port)
{	
	KM_SYNC_Select_Led_Control(port);
	
	if (KVM_Flash.cSystemFlag3 & MPASS_ACROSS_MODE)
	{	
		if (TASK_KM_SYNC_Across_Led_ActiveID == 0)
		{
			TASK_KM_SYNC_Across_Led_ActiveID = 
				TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_KM_SYNC_Across_Led_ID,0,0,ACROSS_LED_INTERVAL,ACROSS_LED_INTERVAL)+1;
		}	
		else
		{
			Task_Active_Table[TASK_KM_SYNC_Across_Led_ActiveID-1].Task_Interval.w = ACROSS_LED_INTERVAL;
		}			
	}				
}	

U8_T KM_SYNC_SwitchMode_State(void)
{
	if ((KVM_Flash.cSystemFlag2 & SYSTEM_ALL_SYNC_MASK) || (KVM_Flash.cSystemFlag3 & MPASS_ACROSS_MODE))
		return 0;
	
	return 1;
}
/**----------------------------------------------------------------------------
 * Function Name: KM_SYNC_ModeSwitch_Control(U8_T syncmode)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void KM_SYNC_ModeSwitch_Control(U8_T mode)
{
	if (mode)
	{	 
		API_Active_Roaming_Mapping(SINGLE_SCREEN_MODE);
		if ((KVM_Flash.cSystemFlag2 & SYSTEM_ALL_SYNC_MASK) || (KVM_Flash.cSystemFlag3 & MPASS_ACROSS_MODE))
		{	
			printf("Switch Mode Start\n\r");				
			API_Set_Roaming_Mode(API_ROAMING_DISABLE);				
			API_Set_Sync_Mode(API_SYNC_DISABLE);
			//KM_SYNC_ModeAccross_Control(API_ACROSS_DISABLE);
			//KM_SYNC_ModeSync_Control(API_SYNC_DISABLE);
			//KVM_Flash.cSystemFlag2 &= ~(SYSTEM_ALL_SYNC_MASK | SYSTEM_KB_SYNC_MASK | SYSTEM_MS_SYNC_MASK);	
			//KVM_Flash.cSystemFlag3 &= ~MPASS_ACROSS_MODE;						
			//#if (KVM_BUZZER_SUPPORT)	
			//BUZZER_Script_Active(PortSwitch_Sound);	
			//#endif //#if (KVM_BUZZER_SUPPORT)					 
		}	
	}	
}	


/**----------------------------------------------------------------------------
 * Function Name: KM_SYNC_ModeSync_Control(U8_T syncmode)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void KM_SYNC_ModeSync_Control(U8_T syncmode)
{
	if (syncmode)
	{
		//if ((KVM_Flash.cSystemFlag2 & (SYSTEM_ALL_SYNC_MASK | SYSTEM_KB_SYNC_MASK | SYSTEM_MS_SYNC_MASK)) == 0x00)	
		{	
			printf("KM Sync Mode Start\n\r");
			KM_SYNC_PortSetting = 0; //clear all exception pid
			KVM_Flash.cSystemFlag2 |= (SYSTEM_ALL_SYNC_MASK | SYSTEM_KB_SYNC_MASK | SYSTEM_MS_SYNC_MASK);	
			KVM_Flash.cSystemFlag3 &= ~MPASS_ACROSS_MODE;			
			KM_SYNC_KM_SyncModeLed(LED_ON);
			#if (KVM_BUZZER_SUPPORT)	
			BUZZER_Script_Active(PortSwitch_Sound);	
			#endif //#if (KVM_BUZZER_SUPPORT)			
		}	
	}
	else
	{
		//if (KVM_Flash.cSystemFlag2 & (SYSTEM_ALL_SYNC_MASK | SYSTEM_KB_SYNC_MASK | SYSTEM_MS_SYNC_MASK))	
		{
			printf("KM Sync Mode Stop\n\r");
			KVM_Flash.cSystemFlag2 &= ~(SYSTEM_ALL_SYNC_MASK | SYSTEM_KB_SYNC_MASK | SYSTEM_MS_SYNC_MASK);			
			//KM_SYNC_KM_SyncModeLed(LED_OFF);
			KM_SYNC_Select_Led_Control(KVM_CurrentHost);
		}	
	}		
}	

/**----------------------------------------------------------------------------
 * Function Name: KM_SYNC_SyncMode_State(void)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
U8_T KM_SYNC_SyncMode_State(void)
{
	if (KVM_Flash.cSystemFlag2 & SYSTEM_ALL_SYNC_MASK)
		return 1;
	return 0;	
}

#ifdef SYNC_KB_HOTKEY
/**----------------------------------------------------------------------------
 * void KM_SYNC_KB_Hotkey_Function_0_StartCheck(U8_T keytype,U8_T keycode)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void KM_SYNC_KB_Hotkey_Function_0_StartCheck(U8_T keytype,U8_T keycode)
{
	if (keytype == HID_USAGE_PAGE_07_MAKE)
	{
		//1.Check Star key
		if ((keycode == CHAR_PAD_STAR) || (keycode == CHAR_SCROLL_LOCK) || (keycode == CHAR_PAD_PLUS) || (keycode == CHAR_PAD_MINUS))
		{				
			Hotkey_Queue[0] = keycode;
			Hotkey_Queue[1] = 0;
			KM_SYNC_HOTKEY_Clear_Flag = 1;
			KM_SYNC_HOTKEY_Skip_Flag = 1;							
			KM_SYNC_Sync_KB_HotkeyInx = 1;	
			KM_SYNC_Hotkey_Function[0] = 0;
			KM_SYNC_Hotkey_Function[1] = 0;
			KM_SYNC_Hotkey_Q_Size = 1;
			//if (keycode == CHAR_SCROLL_LOCK)
			//	KM_SYNC_HOTKEY_Skip_Flag = 0;
		}
		/*
		else if (keycode == CHAR_SCROLL_LOCK)		//2.Check Scroll Key			
		{
			Hotkey_Queue[0] = CHAR_SCROLL_LOCK;			
			KM_SYNC_Sync_KB_HotkeyInx = 1;
			KM_SYNC_HOTKEY_Skip_Flag = 0;
			KM_SYNC_HOTKEY_Clear_Flag = 1;
		}					 		
		else if (keycode == CHAR_PAD_PLUS)		//3.Add Sync port
		{
			Hotkey_Queue[0] = CHAR_PAD_PLUS;
			Hotkey_Queue[0] = CHAR_SCROLL_LOCK;			
			KM_SYNC_Sync_KB_HotkeyInx = 1;
			KM_SYNC_HOTKEY_Skip_Flag = 0;
			KM_SYNC_HOTKEY_Clear_Flag = 1;
		}			
		else if (keycode == CHAR_PAD_MINUS)		//4.Clear Sysnc Port
		{
		}
		*/	
	}					
}	

/**----------------------------------------------------------------------------
 * U8_T KM_SYNC_KB_Hotkey_Check_Valid(U8_T keycode)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
U8_T KM_SYNC_KB_Hotkey_Check_Valid(U8_T keycode)
{
	Hotkey_Queue[2] = 0;
	if ((keycode >= CHAR_PAD_1_END) && (keycode <= CHAR_PAD_9_PGUP))
	{	
		Hotkey_Queue[2] = '1'+(keycode-CHAR_PAD_1_END);						
	}
	else if (keycode == CHAR_PAD_0_INSERT)
	{	
		Hotkey_Queue[2] = '0';
	}			
	else if ((keycode >= CHAR_1) && (keycode <= CHAR_9))
	{	
		Hotkey_Queue[2] = '1'+(keycode-CHAR_1);						
	}			
	else if (keycode == CHAR_0)
	{	
		Hotkey_Queue[2] = '0';
	}			
	else if ((keycode >= CHAR_A) && (keycode <= CHAR_Z))
	{	
		Hotkey_Queue[2] = 'A'+(keycode-CHAR_A);
	}	
	
	return Hotkey_Queue[2];
}

/**----------------------------------------------------------------------------
 * void KM_SYNC_KB_Hotkey_Function_Perform(void)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void KM_SYNC_KB_Hotkey_Function_Perform(void)
{
	//Handle 2 bytes command
	if (KM_SYNC_Sync_KB_HotkeyInx == 2)
	{
		if (KM_SYNC_Hotkey_Q_Size == 2) //2 bytes command
		{
			printf("[Fun=%02bx]",KM_SYNC_Hotkey_Function[0]);
			switch(KM_SYNC_Hotkey_Function[0])
			{
				case KB_HOTKEY_JUMP_MODE:
					KM_SYNC_Force_Port_Jump(KM_SYNC_Hotkey_Function[1]);
					break;
				case KB_HOTKEY_SYNC_PORT_ADD_MODE:
					KM_SYNC_Port_Add(KM_SYNC_Hotkey_Function[1]);
					break;
				case KB_HOTKEY_SYNC_PORT_CLEAR_MODE:
					KM_SYNC_Port_Clear(KM_SYNC_Hotkey_Function[1]);
					break;
				case KB_HOTKEY_SYNC_MODE:
					KM_SYNC_Set_SyncMode();
					break;
				case KB_HOTKEY_ACROSS_MODE:					
					printf("Single Screen Roaming Mode\n\r");
					API_Set_Roaming_Mode(API_ROAMING_ENABLE);					
					API_Active_Roaming_Mapping(SINGLE_SCREEN_MODE);
					break;
				case KB_HOTKEY_ROAMING_MAP:
					printf("Quad Screen Roaming Mode\n\r");
					API_Set_Roaming_Mode(API_ROAMING_ENABLE);					
					API_Active_Roaming_Mapping(QUAD_SCREEN_MODE);					
					break;
				case KB_HOTKEY_VPID_CHANGER_MODE:
					KM_Sync_VPid_Change_Handle(0xff);
					break;
				case KB_HOTKEY_ABSOLUTE_MODE:
					API_Set_Mouse_Mode(API_ABSOLUTE_MODE);
					break;
				case KB_HOTKEY_RELATIVE_MODE:
					API_Set_Mouse_Mode(API_RELATIVE_MODE);
					break;
				case KB_HOTKEY_DC_RESET_MODE:
					CPU_SoftReboot();
					//KM_SYNC_Manufacturer_Default();					
					break;
				case KB_HOTKEY_IAP_MODE:
					KVM_CONSOLE_IAP_Mode();
					break;
				case KB_HOTKEY_REPEAT_KEY_MODE:
					KM_SYNC_Handle_SetRepeatKey(Hotkey_Queue[1]);										
					break;
				case KB_HOTKEY_ROAMING_MAP2:
					API_Roaming_Mapping2_Handle(KM_SYNC_Hotkey_Function[1]);						
					break;
			}			
		}			
	}		
}	

U8_T KM_SYNC_Check_Digit_Function(U8_T keycode)
{
	if ((keycode >= CHAR_PAD_1_END) && (keycode <= CHAR_PAD_4_LEFT)) //Port Jump
	{		
		if (KM_SYNC_Hotkey_Function[0] != KB_HOTKEY_ROAMING_MAP2)
			KM_SYNC_Hotkey_Function[0] = 0;	
		
		if (Hotkey_Queue[0] == CHAR_PAD_STAR)
		{	
			if (KM_SYNC_Hotkey_Function[0] != KB_HOTKEY_ROAMING_MAP2)
				KM_SYNC_Hotkey_Function[0] = KB_HOTKEY_JUMP_MODE;
		}
		else if (Hotkey_Queue[0] == CHAR_PAD_PLUS)
		{	
			KM_SYNC_Hotkey_Function[0] = KB_HOTKEY_SYNC_PORT_ADD_MODE;
		}
		else if (Hotkey_Queue[0] == CHAR_PAD_MINUS)
		{	
			KM_SYNC_Hotkey_Function[0] = KB_HOTKEY_SYNC_PORT_CLEAR_MODE;
		}
		
		if (KM_SYNC_Hotkey_Function[0])
		{	
			KM_SYNC_HOTKEY_Skip_Flag = 1;						
			KM_SYNC_Hotkey_Function[1] = keycode-CHAR_PAD_1_END;
			//KM_SYNC_KB_Hotkey_Function_Perform();				
			return 1;
		}							
	}			
	return 0;
}
/**--------------------------------------------------------------------------------
 * void KM_SYNC_KB_Hotkey_Function_1_SecondKeyMakeCheck(U8_T keytype,U8_T keycode)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * -------------------------------------------------------------------------------*/
void KM_SYNC_KB_Hotkey_Function_1_SecondKeyMakeCheck(U8_T keytype,U8_T keycode)
{		
	if (keytype == HID_USAGE_PAGE_07_MAKE)
	{		
		Hotkey_Queue[1] = keycode;
		KM_SYNC_Hotkey_Q_Size++;
		KM_SYNC_Sync_KB_HotkeyInx = 2;
		
		if (Hotkey_Queue[0] == CHAR_PAD_STAR)
		{
			KM_SYNC_HOTKEY_Skip_Flag = 1;
			if (KM_SYNC_Check_Digit_Function(keycode) == 0)
			{	
				switch (keycode)
				{
					case ROAMING_MAP2_KEY:
						KM_SYNC_Hotkey_Function[0] = KB_HOTKEY_ROAMING_MAP2;
						break;
					case CHAR_O:
						KM_SYNC_Hotkey_Function[0] = KB_HOTKEY_ROAMING_MAP;
						break;
					case CHAR_PAD_0_INSERT:
						KM_SYNC_Hotkey_Function[0] = KB_HOTKEY_SYNC_MODE;
						break;
					case CHAR_S:
						KM_SYNC_Hotkey_Function[0] = KB_HOTKEY_ACROSS_MODE;
						break;
					case CHAR_V:
						KM_SYNC_Hotkey_Function[0] = KB_HOTKEY_VPID_CHANGER_MODE;
						break;
					case CHAR_F5:
						KM_SYNC_Hotkey_Function[0] = 0;
						KM_SYNC_Hotkey_Function[1] = CHAR_F5;
						break;
					case CHAR_F6:
						if (KM_SYNC_Hotkey_Function[1] == CHAR_F5)
						{	
							KM_SYNC_Hotkey_Function[0] = KB_HOTKEY_IAP_MODE;
							KM_SYNC_Hotkey_Function[1] = CHAR_F6;
						}
						else
						{
							KM_SYNC_Hotkey_Function[0] = 0;
						}						
						break;
					case CHAR_F9:
						KM_SYNC_Hotkey_Function[0] = KB_HOTKEY_ABSOLUTE_MODE;
						break;
					case CHAR_F10:
						KM_SYNC_Hotkey_Function[0] = KB_HOTKEY_RELATIVE_MODE;
						break;
					case CHAR_F11:
						KM_SYNC_Hotkey_Function[0] = KB_HOTKEY_DC_RESET_MODE;
						break;
					default:
						KM_SYNC_Hotkey_Function[0] = 0; //no function
				}
			}				 
			
			if (KM_SYNC_Hotkey_Function[0])
			{	
				KM_SYNC_KB_Hotkey_Function_Perform();				
			}	
		}
		else if ((Hotkey_Queue[0] == CHAR_PAD_PLUS) || (Hotkey_Queue[0] == CHAR_PAD_MINUS))
		{
			KM_SYNC_Check_Digit_Function(keycode);
			
			if (KM_SYNC_Hotkey_Function[0])
			{	
				KM_SYNC_KB_Hotkey_Function_Perform();				
			}	
		}			
		else if (Hotkey_Queue[0] == CHAR_SCROLL_LOCK)
		{ // header scroll
			if (((keycode >= CHAR_A) && (keycode <= CHAR_Z)) || (keycode == CHAR_ESCAPE) || (keycode == CHAR_TAB))
			{	
				KM_SYNC_Hotkey_Function[0] = keycode;
				KM_SYNC_HOTKEY_Skip_Flag = 1;
				KM_SYNC_Hotkey_Function[0] = KB_HOTKEY_REPEAT_KEY_MODE;	
				KM_SYNC_KB_Hotkey_Function_Perform();				
			}			
		}			
		else
		{			
			KM_SYNC_Hotkey_Function[0] = 0;			
			KM_SYNC_HOTKEY_Skip_Flag = 1;
		}			
	}
	else
	{ // first key break make
		if (keytype == HID_USAGE_PAGE_07_BREAK)
		{	
			if ((KM_SYNC_Hotkey_Function[0] == 0x00) && (Hotkey_Queue[0] == keycode))
			{
				printf("[Send : %02bx]\n\r",keycode);
				KM_SYNC_KB_Hotkey_Clear();
				KM_SYNC_Send_VHID_Key(HID_USAGE_PAGE_07_MAKE,keycode);
				KM_SYNC_Send_VHID_Key(HID_USAGE_PAGE_07_BREAK,keycode);
			}				
		}		
		//else
		{
			KM_SYNC_KB_Hotkey_Clear();			
		}					
	}
}	

/**--------------------------------------------------------------------------------
 * void KM_SYNC_KB_Hotkey_Function_2_SecondKeyBreakCheck(U8_T keytype,U8_T keycode)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * -------------------------------------------------------------------------------*/
void KM_SYNC_KB_Hotkey_Function_2_SecondKeyBreakCheck(U8_T keytype,U8_T keycode)
{			
	if (keytype == HID_USAGE_PAGE_07_BREAK) 
	{ //could be first/second key break		
		if (Hotkey_Queue[0] == keycode) //if release the leading key
		{
			Hotkey_Queue[0]= 0; 
			KM_SYNC_Sync_KB_HotkeyInx = 3; //wait for final key release			
			KM_SYNC_Hotkey_Q_Size--;
		}			
		
		if (Hotkey_Queue[1] == keycode)
		{						
			KM_SYNC_Sync_KB_HotkeyInx = 1; //wait next key release				
			Hotkey_Queue[1]= 0;		
			KM_SYNC_Hotkey_Q_Size--;						
		}		
	}		
	else
	{//key make					
		KM_SYNC_Hotkey_Q_Size++;
		KM_SYNC_Sync_KB_HotkeyInx = 3;			
		Hotkey_Queue[2]=keycode;		
	}		
}	

/**--------------------------------------------------------------------------------
 * void KM_SYNC_KB_Hotkey_Function_3_ThirdKeyCheck(U8_T keytype,U8_T keycode)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * -------------------------------------------------------------------------------*/
void KM_SYNC_KB_Hotkey_Function_3_ThirdKeyCheck(U8_T keytype,U8_T keycode)
{
	U8_T i;
	
	if (keytype == HID_USAGE_PAGE_07_BREAK)
	{				
		KM_SYNC_Hotkey_Q_Size--;
		if (KM_SYNC_Hotkey_Q_Size == 0)
		{	
			KM_SYNC_KB_Hotkey_Clear();
		}	
		else if (KM_SYNC_Hotkey_Q_Size == 1)
		{
			KM_SYNC_Sync_KB_HotkeyInx = 1; //wait next key release				
			Hotkey_Queue[1]= 0;					
		}			
		for (i=0; i < 2; i++)
		{
			if (Hotkey_Queue[i] == keycode)
			{	
				Hotkey_Queue[i] = 0;
				break;
			}	
		}
	}
	else
	{
		KM_SYNC_Hotkey_Q_Size++;
	}		
}

/**----------------------------------------------------------------------------
 * Function Name: KM_SYNC_MS_Hotkey_Check(USB_MS_Data_TypeDef *ms_data)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
 
void KM_SYNC_KB_Hotkey_Check(U8_T keytype,U8_T keycode)
{
	//U8_T i,tb;
	
	printf("[K %02bx %02bx]",keytype,keycode);
	printf("[S_index=%bu,size=%bu]",KM_SYNC_Sync_KB_HotkeyInx,KM_SYNC_Hotkey_Q_Size);
	
	//Check Modifies key status
	if ((keycode >= 0xe0) && (keycode <= 0xe7))
	{
		if (keytype == HID_USAGE_PAGE_07_MAKE)
		{
			KM_SYNC_HOTKEY_Modifiers |= BIT_MASK[keycode-0xe0];
		}			
		else if (keytype == HID_USAGE_PAGE_07_BREAK)
		{
			KM_SYNC_HOTKEY_Modifiers &= ~BIT_MASK[keycode-0xe0];
		}			
	}		
			
	//2.Hotkey check start	
	if (KM_SYNC_Sync_KB_HotkeyInx == 0) //the first start
	{
		//1.Repeat Key check
		if (KM_SYNC_HOTKEY_Modifiers == 0)
		{	
			KM_SYNC_Repeat_Key_Handle(keytype,keycode);
		}	
		KM_SYNC_KB_Hotkey_Function_0_StartCheck(keytype,keycode);		
	}		
	else if (KM_SYNC_Sync_KB_HotkeyInx == 1) //check the second key
	{
		KM_SYNC_KB_Hotkey_Function_1_SecondKeyMakeCheck(keytype,keycode);									
	}		
	else if (KM_SYNC_Sync_KB_HotkeyInx == 2) //check the second key release
	{
		KM_SYNC_KB_Hotkey_Function_2_SecondKeyBreakCheck(keytype,keycode);
	}		
	else if (KM_SYNC_Sync_KB_HotkeyInx >= 3) //check the second key release
	{
		KM_SYNC_KB_Hotkey_Function_3_ThirdKeyCheck(keytype,keycode);
	}		
	//printf("[Fun=%02bx]\n\r",KM_SYNC_Hotkey_Function[0]);
}

#endif /* #ifdef SYNC_KB_HOTKEY */ 
/**----------------------------------------------------------------------------
 * Function Name: TASK_KM_SYNC_Across_Led(void)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void TASK_KM_SYNC_Across_Led(void)
{	
	if (KVM_Flash.cSystemFlag3 & MPASS_ACROSS_MODE)
	{
		//KM_SYNC_KM_SyncModeLed(LED_OFF);
		if (GPIO_GetPinValue(SELECT_LED_PORT[KVM_CurrentHost],SELECT_LED[KVM_CurrentHost])) //if off
		{	
			GPIO_SetOneBit(SELECT_LED_PORT[KVM_CurrentHost],SELECT_LED[KVM_CurrentHost],LED_ON);
		}
		else
		{
			GPIO_SetOneBit(SELECT_LED_PORT[KVM_CurrentHost],SELECT_LED[KVM_CurrentHost],LED_OFF);
		}									
	}		
	else
	{
		TASK_KM_SYNC_Across_Led_ActiveID = 0;
		TASK_Destory_Current();
	}		
}

/*----------------------------------------------------------------------------
 * void TASK_KM_SYNC_Active_Delay(void)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------- */
void TASK_KM_SYNC_Active_Delay(void)
{		
	KM_SYNC_ModeSync_Control(0);						
	KM_SYNC_ModeAccross_Control(0);
	
#if (KVM_BUZZER_SUPPORT)	
	if ( KVM_CurrentHost == TASK_Register0)
	{
		BUZZER_Script_Active(PortSwitch_Sound);
	}	
#endif //#if (KVM_BUZZER_SUPPORT)	
	
	KVM_Port_Jump(TASK_Register0);		
	TASK_Destory_Current();
}

/*----------------------------------------------------------------------------
 * void TASK_KM_SYNC_DN_Active_Delay(void)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------- */
void TASK_KM_SYNC_DN_Active_Delay(void)
{		
	if (TASK_Register0 == 0) //SYNC Mode
	{	
		printf("Go Absolute Mode\n\r");
		//KM_SYNC_ModeSync_Control(1);	
		KVM_Flash.cSystemFlag2 &= ~SYSTEM_MS_DN_MASK; //cancel the DN mode						
	}
	else
	{	
		//active the DN mode
		if (KVM_Flash.cSystemFlag3 & MPASS_ACROSS_MODE)
		{
			KM_SYNC_Select_Led_Control(KVM_CurrentHost);
		}	
		
		if ((KVM_Flash.cSystemFlag2 & SYSTEM_MS_DN_MASK) == 0x00)						
		{
			KVM_Flash.cSystemFlag2 |= SYSTEM_MS_DN_MASK;
			//KVM_Flash.cSystemFlag3 &= ~(MPASS_ACROSS_MODE | SYSTEM_MOUSE_SW_MASK);	
			printf("MSHotkey : Go Relative Mode\n\r");
		}															
	}	
	TASK_Destory_Current();
}

/*----------------------------------------------------------------------------
 * U8_T KM_SYNC_Check_KB_Repeat_Port(void)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------- */
U8_T KM_SYNC_Check_KB_Repeat_Port(void)
{
#if (REPEAT_MODE == 0) //only use one
	return KVM_CurrentHost;
#else
	if (KM_SYNC_SyncMode_State())
		return 0;
	else
		return KVM_CurrentHost;
#endif	/* #if (REPEAT_MODE == 0) */
}	

/*----------------------------------------------------------------------------
 * void KM_SYNC_KB_Check_Repeat(U8_T usbcode)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------- */
U8_T KM_SYNC_KB_Repeat_Active(U8_T p,U8_T usbcode)
{	
	U8_T i;
	
	//p = KM_SYNC_Check_KB_Repeat_Port();
	//check the repeat keycode table first
	i = KM_SYNC_Search_Repeat_Table(KM_SYNC_Sync_KB_Repeat,usbcode);
	if (i != 0xff) //this keycode is in table
	{			
#if (REPEAT_MODE == 0) //only use one
		KM_SYNC_Sync_KB_RepeatActive[p] |=  BIT_MASK[i];	
#else
		if (KM_SYNC_Sync_KB_RepeatActive[p] & BIT_MASK[i])
		{
			KM_SYNC_KB_Clear_Active(p,usbcode);
			return 0;
		}
		else
		{	
			KM_SYNC_Sync_KB_RepeatActive[p] |= BIT_MASK[i];		
		}	
#endif
		

		printf("Set Repate Active[%bu](%02bx):%bu\n\r",p,KM_SYNC_Sync_KB_RepeatActive,i);
		for (i=2; i < 8;i++)
		{
			if (KM_SYNC_Sync_KB_HID_RepeatTableMake[p][i] == 0x00)
			{	
				KM_SYNC_Sync_KB_HID_RepeatTableMake[p][i] = usbcode;
				break;
			}	
		}
		return 1;
	}		
	return 0;
}

/*----------------------------------------------------------------------------
 * void KM_SYNC_KB_Check_Repeat(U8_T usbcode)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------- */
void KM_SYNC_KB_Clear_Active(U8_T port,U8_T usbcode)
{
	U8_T i;
	
	 
	i = KM_SYNC_Search_Repeat_Table(KM_SYNC_Sync_KB_Repeat,usbcode);
	
	if (i != 0xff)
	{		
		KM_SYNC_Sync_KB_RepeatActive[port] &= ~BIT_MASK[i];
		//printf("Clear Repate Active(%02bx):%bu\n\r",KM_SYNC_Sync_KB_RepeatActive,i);
		for (i=2; i < 8;i++)
		{
			if (KM_SYNC_Sync_KB_HID_RepeatTableMake[port][i] == usbcode)
			{	
				KM_SYNC_Sync_KB_HID_RepeatTableMake[port][i] = 0x00;
				break;
			}	
		}

		//if in sync mode	
		if ((KM_SYNC_SyncMode_State()) && (port == 0))
		{
			memcpy(&KM_SYNC_Sync_KB_RepeatActive[1],&KM_SYNC_Sync_KB_RepeatActive[0],KVM_MAX_PORT-1);
			memcpy(KM_SYNC_Sync_KB_HID_RepeatTableMake[1],KM_SYNC_Sync_KB_HID_RepeatTableMake[0],(KVM_MAX_PORT-1) << 3);
		}			
	}	
}

/*----------------------------------------------------------------------------
 * void KM_SYNC_Send_KB_Hid_Key(U8_T port,U8_T usbcode)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------- */
void KM_SYNC_Send_KB_Hid_Key(U8_T port,U8_T usbcode)
{
	DATAST_Keyboard_Send(port,HID_USAGE_PAGE_07_MAKE,usbcode);
	DATAST_Keyboard_Send(port,HID_USAGE_PAGE_07_BREAK,usbcode);				
}	

/*----------------------------------------------------------------------------
 * void KM_SYNC_Send_VHID_Key(U8_T keytype,U8_T keycode)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------- */
void KM_SYNC_Send_VHID_Key(U8_T keytype,U8_T keycode)
{
	U8_T port,maxport;
	
		 
	if (KVM_Flash.cSystemFlag2 & (SYSTEM_ALL_SYNC_MASK | SYSTEM_KB_SYNC_MASK))
	{	
		port = 0;
		maxport = KVM_MAX_PORT;
	}	
	else
	{	
		port = KVM_CurrentHost;
		maxport = KVM_CurrentHost+1;
	}
	
	for ( ;port < maxport ; port++)
	{
		//KM_SYNC_Send_KB_Hid_Key(port,CHAR_REPEAT);
		DATAST_Keyboard_Send(port,keytype,keycode);
		//DATAST_Keyboard_Send(port,HID_USAGE_PAGE_07_BREAK,CHAR_REPEAT);				
	}	
}	


/*----------------------------------------------------------------------------
 * void KM_SYNC_Repeat_Key_Handle(U8_T keytype,U8_T keycode)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------- */
void KM_SYNC_Repeat_Key_Handle(U8_T keytype,U8_T keycode)
{
	U8_T port,maxport;
	
	port = KM_SYNC_Check_KB_Repeat_Port();
	if (keytype == HID_USAGE_PAGE_07_MAKE)
	{
		if (KM_SYNC_KB_Repeat_Active(port,keycode)) //if trigger the repeat active operation.
		{
			if (KM_SYNC_Sync_KB_RepeatActive[port])
			{	
				KM_SYNC_Sync_KB_Repeat_Devinx = Kb_devinx;				
				
				if (KVM_Flash.cSystemFlag2 & SYSTEM_ALL_SYNC_MASK)
				{	
					port = 0;
					maxport = KVM_MAX_PORT;
				}	
				else
				{	
					port = KVM_CurrentHost;
					maxport = KVM_CurrentHost+1;
				}
				
				for ( ;port < maxport ; port++)
				{
					if (KVM_Flash.cSystemFlag2 & SYSTEM_ALL_SYNC_MASK)
					{
						if (port)
						{
							KM_SYNC_Sync_KB_RepeatActive[port] = KM_SYNC_Sync_KB_RepeatActive[0];
							memcpy(KM_SYNC_Sync_KB_HID_RepeatTableMake[port],KM_SYNC_Sync_KB_HID_RepeatTableMake[0],8);
						}	
						
						if ((KM_SYNC_PortSetting & BIT_MASK[port]) == 0x00)
							KM_SYNC_Send_KB_Hid_Key(port,CHAR_REPEAT);						
					}		
					else
					{
						printf("Fork Port:%bu",port);
						KM_SYNC_Send_KB_Hid_Key(port,CHAR_REPEAT);
					}	
					//DATAST_Keyboard_Send(port,HID_USAGE_PAGE_07_MAKE,CHAR_REPEAT);
					//DATAST_Keyboard_Send(port,HID_USAGE_PAGE_07_BREAK,CHAR_REPEAT);				
				}	
			}	
		}					
	}
	else if (keytype == HID_USAGE_PAGE_07_BREAK)
	{
		#if (REPEAT_MODE == 0)
		KM_SYNC_KB_Clear_Active(port,keycode);
		#endif
	}		
}	

U8_T KM_SYNC_Port_One_3(void)
{
	U8_T nextport;
	U16_T one_quarter;
	U16_T three_quarter;
	
	one_quarter = (Y_MAX >> 2);
	three_quarter = ((Y_MAX * 3) >> 2);
	
	if (MouseY < one_quarter)
	{
		nextport = Roaming[Current_Roaming_Table]->Sequence[1];
		printf("CG:三等分上\r\n");
	}					
	else if (MouseY < three_quarter)
	{
		nextport = Roaming[Current_Roaming_Table]->Sequence[2];
		printf("CG:三等分中\r\n");
	}
	else
	{
		nextport = Roaming[Current_Roaming_Table]->Sequence[3];
		printf("CG:三等分下\r\n");
	}
	
	return nextport;
}

U8_T KM_SYNC_Port_One_2(void)
{
	if (MouseY < (Y_MAX >> 1))
	{
		printf("CG:二等分上 %d\r\n", (Y_MAX >> 1));
		return Roaming[Current_Roaming_Table]->Sequence[1];
	}
	else
	{
		printf("CG:二等分下 %d\r\n", (Y_MAX >> 1));
		return Roaming[Current_Roaming_Table]->Sequence[2];
	}
	
}
/*----------------------------------------------------------------------------
 * U8_T KM_SYNC_Port(U8_T current_port,U8_T direction)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : 获取下一次要跳转的端口，在这里加了对一大三小的屏幕的特殊处理
 *---------------------------------------------------------------------------- */
// U8_T KM_SYNC_Port(U8_T current_port,U8_T direction)
// {
// 	U8_T nextport;
	
// 	switch (direction)
// 	{	
// 		case ROAMING_LEFT:
// 			nextport = Roaming[Current_Roaming_Table]->Edge.Item[current_port].Left.Portid;			
// 			break;
// 		case ROAMING_RIGHT:
// 			nextport = Roaming[Current_Roaming_Table]->Edge.Item[current_port].Right.Portid;
// 			break;
// 		case ROAMING_TOP:
// 			if ((Current_Roaming_Table == ONE_3_SCREEN_MODE) && (current_port == API_Current_Main_SCREEN))
// 			{
// 				nextport = KM_SYNC_Port_One_3();				
// 			}
// 			else
// 			{	
// 				nextport = Roaming[Current_Roaming_Table]->Edge.Item[current_port].Up.Portid;
// 			}	
// 			break;
// 		case ROAMING_BOTTOM:
// 			if ((Current_Roaming_Table == ONE_3_SCREEN_MODE) && (current_port == API_Current_Main_SCREEN))
// 			{
// 				nextport = KM_SYNC_Port_One_3();				
// 			}
// 			else
// 			{
// 				nextport = Roaming[Current_Roaming_Table]->Edge.Item[current_port].Down.Portid;
// 			}	
// 			break;
// 	}	
// 	return nextport;
// }
 



U8_T KM_SYNC_Port(U8_T current_port,U8_T direction)
{
	U8_T nextport;
	
	switch (direction)
	{	
		case ROAMING_LEFT:
			nextport = Roaming[Current_Roaming_Table]->Edge.Item[current_port].Left.Portid;		
			break;

		case ROAMING_RIGHT:
			if ((Current_Roaming_Table == ONE_2_SCREEN_MODE) && (current_port == API_Current_Main_SCREEN))
			{
				nextport = KM_SYNC_Port_One_2();
				printf("CG:ONE_2 right move, main port:%bu, MouseY:%d\n\r",current_port,MouseY);
			}
			else if ((Current_Roaming_Table == ONE_3_SCREEN_MODE) && (current_port == API_Current_Main_SCREEN))
			{
				nextport = KM_SYNC_Port_One_3();	
				printf("CG:右 一大三小，主窗口 %bu\n",current_port);	
			}
			else
			{
				nextport = Roaming[Current_Roaming_Table]->Edge.Item[current_port].Right.Portid;
				
			}	
			break;

		case ROAMING_TOP:
			nextport = Roaming[Current_Roaming_Table]->Edge.Item[current_port].Up.Portid;
			break;

		case ROAMING_BOTTOM:
			nextport = Roaming[Current_Roaming_Table]->Edge.Item[current_port].Down.Portid;
			break;
	}	
	return nextport;
}



/*---------------------------------------------------------------------------------
 * U8_T KM_SYNC_Power_Status_Check(U8_T current_port,U8_T direction,U8_T call_fun)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : 跳转前检查函数，查看端口是否连接，并调用数组里面的回调函数
 *-------------------------------------------------------------------------------- */
U8_T KM_SYNC_Power_Status_Check(U8_T current_port,U8_T direction,U8_T call_fun)
{
	U8_T nextport,current_port_hold;
	U8_T screen_position;
	
	current_port_hold = current_port;
	
KM_SYNC_Power_Status_Check_Start:	
	nextport = KM_SYNC_Port(current_port,direction);		
	if (USBDC_UpPortState[nextport] & USBDC_ROOTHUB_ATTACHED_MASK)
	{
		if (current_port_hold != nextport)
		{
			if (call_fun)
			{ // call user define call back function
				if (Roaming[Current_Roaming_Table]->Edge.Item[current_port].CallBack_FunctionP)
				{
					Current_Roaming_Direction = direction;
					Roaming[Current_Roaming_Table]->Edge.Item[current_port].CallBack_FunctionP();
				}					
			}				
			return nextport+1;
		}	
		else
			return 0;
	}				
	else
	{ //judge the corss move operation in quad screen mode
		if (Current_Roaming_Table == QUAD_SCREEN_MODE)
		{			
			screen_position = Roaming[Current_Roaming_Table]->Sequence[current_port];
			//printf("[D:%bu,S:%bu,X=%d,Y=%d]",direction,screen_position,MouseX,MouseY);
			switch (screen_position)
			{
				case 0: //Left Up 
					if ((direction == ROAMING_RIGHT) || (direction == ROAMING_BOTTOM))
					{	
						if ((MouseX >= ROAMING_CROSS_MARGIN_X[0]) &&  
							(MouseY >= ROAMING_CROSS_MARGIN_Y[0]))
						{
							return nextport+1;
						}
					}													
					break;
				case 1: //Left Down
					if ((direction == ROAMING_RIGHT) || (direction == ROAMING_TOP))
					{	
						if ((MouseX>= ROAMING_CROSS_MARGIN_X[1]) &&  
							(MouseY <= ROAMING_CROSS_MARGIN_Y[1]))
						{
							return nextport+1;
						}
					}
					break;
				case 2: //Right Up
					if ((direction == ROAMING_LEFT) || (direction == ROAMING_BOTTOM))
					{	
						if ((MouseX <= ROAMING_CROSS_MARGIN_X[2]) &&  
							(MouseY >= ROAMING_CROSS_MARGIN_Y[2]))
						{
							return nextport+1;
						}
					}
					break;
				case 3: //Right Down
					if ((direction == ROAMING_LEFT) || (direction == ROAMING_TOP))
					{	
						if ((MouseX <= ROAMING_CROSS_MARGIN_X[3]) &&  
							(MouseY <= ROAMING_CROSS_MARGIN_Y[3]))
						{
							return nextport+1;
						}
					}
					break;
			}							
		}			
	}		
	current_port = nextport;
	if (current_port_hold != current_port)
	{
		goto KM_SYNC_Power_Status_Check_Start;		
	}	
	
	return 0;
		
	/*
	if (direction) // look forward
	{		
		for (i=0; i < (KVM_MAX_PORT-1) ; i++)
		{			
			nextport = KM_SYNC_Port(current_port,1);			
			if (USBDC_UpPortState[nextport] & USBDC_ROOTHUB_ATTACHED_MASK)
			{
				return nextport+1;
			}				
			current_port = nextport;
		}				
		return 0;
	}
	else
	{
		for (i=0; i < (KVM_MAX_PORT-1) ; i++)
		{			
			nextport = KM_SYNC_Port(current_port,0);			
			if (USBDC_UpPortState[nextport] & USBDC_ROOTHUB_ATTACHED_MASK)
			{
				return nextport+1;
			}				
			current_port = nextport;
		}				
		return 0;
	}				
	*/	
}

/*----------------------------------------------------------------------------
 * void KM_SYNC_Mouse_Jump_State_Reset(void)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------- */
void KM_SYNC_Mouse_Jump_State_Reset(void)
{
	Coordinate_Cnt[0] = Coordinate_Cnt[1] = 0;		
	Coordinate_Cnt[2] = Coordinate_Cnt[3] = 0;		
	Coordinate_State &= ~0x0f;		
	if (Coordinate_State & MOUSE_JUMP_MASK)
	{
		if (Coordinate_State & ACROSS_LEFT_MASK)
		{				
			MouseX = X_RIGHT_EDGE;
		}
		
		if (Coordinate_State & ACROSS_RIGHT_MASK)
		{
			MouseX = X_LEFT_EDGE;
		}
		
		
		if (Coordinate_State & ACROSS_TOP_MASK)
		{
			MouseY = Y_BOTTOM_EDGE;
		}					
		if (Coordinate_State & ACROSS_BOTTOM_MASK)
		{
			MouseY = Y_TOP_EDGE;
		}
		
	}		
	Coordinate_State &= ~(MOUSE_JUMP_MASK|ACROSS_RIGHT_MASK|ACROSS_LEFT_MASK|ACROSS_TOP_MASK|ACROSS_BOTTOM_MASK|ROAMING_REPORT_MASK);
}

/*----------------------------------------------------------------------------
 * U8_T KM_SYNC_Port_Jump_Power(U8_T current_port,U8_T jumptype,U8_T call_fun);
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------- */
U8_T KM_SYNC_Port_Jump_Power(U8_T current_port,U8_T jumptype,U8_T call_fun)
{
	U8_T newport;

	Coordinate_Cnt[0] = 0;					
	Coordinate_State |= MOUSE_JUMP_MASK;	
	
	newport = KM_SYNC_Power_Status_Check(current_port,jumptype,call_fun);	
	if (newport)
	{		
		newport--;
		//KVM_Port_Jump(newport-1);
		//if (KVM_CurrentHost != newport)
		{
			KVM_Control_Status |= KVM_PORT_SW_MASK; 
			// Prevent PS/2 stick after port switch */
#if (SYSTEM_PS2_HOST_ENABLE)
			KVM_PS2_HotkeySwitch_Flag = 1; //it is hotkey switch
#endif			
			KVM_NextHost = newport;
			TASK_KVM_Switch_Event_Control_Fork(KVM_NextHost,30); 
		}
		return 1;
	}			
	else
	{
		KM_SYNC_Mouse_Jump_State_Reset();
	}		
	return 0;
}

U8_T KM_SYNC_Port_Across_Jump(U8_T current_host)
{
	#ifdef AX68002
	current_host++;
	if (current_host >= KVM_MAX_PORT)
		current_host = 0;
	#else
	current_host += 2;
	if (current_host >= KVM_MAX_PORT)
		current_host -= KVM_MAX_PORT;
	#endif
	
	//printf("[C:%bu]",current_host);
	if (USBDC_UpPortState[current_host] & USBDC_ROOTHUB_ATTACHED_MASK)
	{			
		Coordinate_State |= MOUSE_JUMP_MASK;
		KVM_Control_Status |= KVM_PORT_SW_MASK; 
		// Prevent PS/2 stick after port switch */
#if (SYSTEM_PS2_HOST_ENABLE)
		KVM_PS2_HotkeySwitch_Flag = 1; //it is hotkey switch
#endif					 
		TASK_KVM_Switch_Event_Control_Fork(current_host,10); 		
		return current_host+1;
	}
	return 0;
}	
/*----------------------------------------------------------------------------------
 * void KM_SYNC_Abs_Mouse_Coordinate_Convert(U8_T *datacode)
 * Purpose : convert the relative mouse data int absolute data
 * Params  :  
 * Returns : none
 * Note    : none
 *-----------------------------------------------------------------------------------*/
void KM_SYNC_Abs_Mouse_Coordinate_Convert(U8_T *datacode)
{	
	U16_T t;
	U8_T edge_check=0;
	U8_T middle_btn;
	//U8_T newport;
		
	middle_btn = datacode[0] & 0x04;
	//1.Check X coordinate	
	if (Mouse_Data.b.wX.w & 0x8000) //negative	
	{	
		Coordinate_State &= ~X_RIGHT_MASK;
		Coordinate_State |= X_LEFT_MASK;
		//X_Right = 0;
		//X_Left = 1;	
		//t = (~datacode[1])+1; 
		t = (~Mouse_Data.b.wX.w)+1; 
		//datacode[2] &= ~0x80;
		if (MouseX >= t)
		{	
			MouseX -= t;				
			//if (MouseX == 0)
			//	MouseX = 1;								
		}	
		else
			MouseX = 0;					
	}
	else
	{			
		Coordinate_State &= ~X_LEFT_MASK;
		Coordinate_State |= X_RIGHT_MASK;
		//X_Left = 0;
		//X_Right = 1;
		//MouseX += datacode[1];
		MouseX += Mouse_Data.b.wX.w;
		if (MouseX >= X_MAX)
			MouseX = X_MAX;
	}
			
	//2.Check Y coordinate	
	if (Mouse_Data.b.wY.w & 0x8000) //negative
	{				
		Coordinate_State &= ~Y_BOTTOM_MASK;
		Coordinate_State |= Y_TOP_MASK;		
		t = (~Mouse_Data.b.wY.w)+1; 		
		if (MouseY >= t)
		{	
			MouseY -= t;			
		}	
		else
			MouseY = 0;			
	}
	else
	{		
		Coordinate_State &= ~Y_TOP_MASK;
		Coordinate_State |= Y_BOTTOM_MASK;		
		MouseY += Mouse_Data.b.wY.w;
		if (MouseY >= Y_MAX)
			MouseY = Y_MAX;
	}
	
	#ifdef API
	API_Send_Mouse_Absolute();
	#endif  /* #ifdef API */ 
	//3.Move data into right position
	
	//datacode[1] = datacode[3]; //copy Z coordinate data	
	//datacode[2] = MouseXb[1]; //copy X LSB data
	//datacode[3] = MouseXb[0]; //copy X MSB data
	//datacode[4] = MouseYb[1]; //copy Y LSB data
	//datacode[5] = MouseYb[0]; //copy Y MSB data
	
// Check MPass mode control
	//1.Check Upper direction
	/*
	if (Coordinate_State & Y_TOP_MASK)
	{
		if ((middle_btn) && (KVM_Flash.cSystemFlag3 & SYSTEM_MOUSE_SW_MASK))
		{			
			if ((KVM_Flash.cSystemFlag3 & SYSTEM_MPASS_MASK) != MPASS_ACROSS_MODE)
			{						 
				Coordinate_Cnt[2]++;		
				if (Coordinate_Cnt[2] >= COORDINATE_COUNTER)
				{
					KVM_Flash.cSystemFlag3 &= ~SYSTEM_MPASS_MASK;
					KVM_Flash.cSystemFlag3 |= MPASS_ACROSS_MODE;
					Coordinate_Cnt[2] = 0;
					//printf("Mouse Across Mode Start(Absolute)\n\r");
					#if (KVM_BUZZER_SUPPORT)
					BUZZER_Script_Active(Hotkey_Active_Sound);
					#endif
					//MouseX = X_RIGHT_EDGE;
					//DATAST_Mouse_Port_Jump_Power(KVM_CurrentHost,0);																
				}					
			}
		}
		else
		{
			Coordinate_Cnt[2] = 0;
		}			
	}		
	else
	{ 
		Coordinate_Cnt[2] = 0;
	}

	//2.Check bottom direction
	if (Coordinate_State & Y_BOTTOM_MASK)
	{
		if ((middle_btn) && (KVM_Flash.cSystemFlag3 & SYSTEM_MOUSE_SW_MASK))
		{
			if ((KVM_Flash.cSystemFlag3 & SYSTEM_MPASS_MASK) != MPASS_BUTTON_MODE)
			{	
				Coordinate_Cnt[3]++;		
				if (Coordinate_Cnt[3] >= COORDINATE_COUNTER)
				{
					KVM_Flash.cSystemFlag3 &= ~SYSTEM_MPASS_MASK;
					KVM_Flash.cSystemFlag3 |= MPASS_BUTTON_MODE;
					Coordinate_Cnt[3] = 0;					
					//printf("Mouse Middle Button Mode Start(Relative)\n\r");
					#if (KVM_BUZZER_SUPPORT)
					BUZZER_Script_Active(Error_Sound);
					#endif
					//MouseX = X_RIGHT_EDGE;
					//DATAST_Mouse_Port_Jump_Power(KVM_CurrentHost,0);																
				}			
			}	
		}		
		else
		{
			Coordinate_Cnt[3] = 0;
		}			
	}
	else
	{
		Coordinate_Cnt[3] = 0;
	}	
	*/	
// Check Margin
	//1.Check the middle button operation	

	#ifdef KMLOG
	if (KM_Check_Skip_Condition())
	{	
		return;
	}	
	#endif
		
	if (((KVM_Flash.cSystemFlag3 & SYSTEM_MPASS_MASK) == MPASS_ACROSS_MODE) && (KVM_Flash.cSystemFlag3 & SYSTEM_MOUSE_SW_MASK))
	{		
		//printf("[%d:%d]",MouseX,MouseY);
		//1.Check Left Edge
		if (KM_SYNC_Roaming_Edge & EDGE_ROAMING_LEFT)
		{	
			if (MouseX == X_LEFT_EDGE)
			{
				if ((Coordinate_State & MOUSE_JUMP_MASK)==0)
				{	
					Coordinate_Cnt[0]++;		
					if (Coordinate_Cnt[0] >= Roaming[Current_Roaming_Table]->Edge.Item[KVM_CurrentHost].Left.Latency)
					{				
						//printf("Left Trigger\n\r");	
						if (KM_SYNC_Port_Jump_Power(KVM_CurrentHost,ROAMING_LEFT,1))
						{
							MouseX = X_RIGHT_EDGE;
							Coordinate_State |= ACROSS_LEFT_MASK;
							//MouseX = X_LEFT_EDGE;
						}					
					}
				}	
			}
			else
			{
				Coordinate_Cnt[0] = 0;
			}
		}	
		//2.Check Left Edge
		if (KM_SYNC_Roaming_Edge & EDGE_ROAMING_RIGHT)
		{	
			if (MouseX == X_RIGHT_EDGE)
			{
				if ((Coordinate_State & MOUSE_JUMP_MASK)==0)
				{
					Coordinate_Cnt[1]++;
					if (Coordinate_Cnt[1] >= Roaming[Current_Roaming_Table]->Edge.Item[KVM_CurrentHost].Right.Latency)
					{				
						//printf("Right Trigger\n\r");	
						printf(
							"Before Jump: Host=%bu Dir=%bu X=%u Y=%u State=%02bX\r\n",
							KVM_CurrentHost,
							ROAMING_RIGHT,
							MouseX,
							MouseY,
							Coordinate_State
						);						 
						if (KM_SYNC_Port_Jump_Power(KVM_CurrentHost,ROAMING_RIGHT,1))
						{
							//KMLog_Send_Roaming_Report(KVM_CurrentHost,ROAMING_RIGHT);
							Coordinate_State |= ACROSS_RIGHT_MASK;
							MouseX = X_RIGHT_EDGE;
							//MouseX = X_LEFT_EDGE;
							printf(
								"Right Jump Success: Host=%bu X=%u Y=%u State=%02bX\r\n",
								KVM_CurrentHost,
								MouseX,
								MouseY,
								Coordinate_State
							);
						}								 				
					}		
				}	
			}
			else
			{
				Coordinate_Cnt[1] = 0;
			}
		}	
		//3.Check Upper Edge
		if (KM_SYNC_Roaming_Edge & EDGE_ROAMING_TOP)
		{
			if (MouseY == Y_TOP_EDGE)
			{
				if ((Coordinate_State & MOUSE_JUMP_MASK)==0)
				{
					Coordinate_Cnt[2]++;
					if (Coordinate_Cnt[2] >= Roaming[Current_Roaming_Table]->Edge.Item[KVM_CurrentHost].Up.Latency)
					{															
						if (KM_SYNC_Port_Jump_Power(KVM_CurrentHost,ROAMING_TOP,1))
						{					
							//KMLog_Send_Roaming_Report(KVM_CurrentHost,ROAMING_TOP);
							Coordinate_State |= ACROSS_TOP_MASK;
							MouseY = Y_BOTTOM_EDGE;						
						}											
					}		
				}	
			}					
			else
			{
				Coordinate_Cnt[2] = 0;
			}				 
		}
		//4.Check Bottom Edge
		if (KM_SYNC_Roaming_Edge & EDGE_ROAMING_BOTTOM)
		{
			if (MouseY == Y_BOTTOM_EDGE)
			{
				if ((Coordinate_State & MOUSE_JUMP_MASK)==0)
				{
					Coordinate_Cnt[3]++;
					if (Coordinate_Cnt[3] >= Roaming[Current_Roaming_Table]->Edge.Item[KVM_CurrentHost].Down.Latency)
					{				
						//printf("Bottom Trigger\n\r");										
						if (KM_SYNC_Port_Jump_Power(KVM_CurrentHost,ROAMING_BOTTOM,1))
						{
							//KMLog_Send_Roaming_Report(KVM_CurrentHost,ROAMING_BOTTOM);
							Coordinate_State |= ACROSS_BOTTOM_MASK;
							MouseY = Y_BOTTOM_EDGE;				
						}											
					}		
				}	
			}					
			else
			{
				Coordinate_Cnt[3] = 0;
			}
		}
	}	
	
	datacode[1] = datacode[3]; //copy Z coordinate data	
	datacode[2] = MouseXb[1]; //copy X LSB data
	datacode[3] = MouseXb[0]; //copy X MSB data
	datacode[4] = MouseYb[1]; //copy Y LSB data
	datacode[5] = MouseYb[0]; //copy Y MSB data
}		 

/*----------------------------------------------------------------------------------
 * U8_T KM_SYNC_Search_Repeat_Table(U8_T table,U8_T keycode)	
 * Purpose : 
 * Params  :  
 * Returns : none
 * Note    : none
 *-----------------------------------------------------------------------------------*/
U8_T KM_SYNC_Search_Repeat_Table(U8_T table,U8_T keycode)	
{	
	U8_T tb,i;
	
	tb = table;				
	for (i=0; (i < KM_SYNC_REPEAT_SIZ) && tb;i++)
	{									    
		if (KM_SYNC_Sync_KB_RepeatTable[i] == keycode)
		{
			return i;			
		}						
		tb &= ~BIT_MASK[i];
	}
	
	return 0xff;
}

/*----------------------------------------------------------------------------------
 * U8_T KM_SYNC_Check_HID_Repeat_State(U8_T table,U8_T keycode)	
 * Purpose : convert the relative mouse data int absolute data
 * Params  :  
 * Returns : none
 * Note    : none
 *-----------------------------------------------------------------------------------*/
void KM_SYNC_Check_HID_Repeat_State(U8_T port)	
{	
	if (KM_SYNC_Sync_KB_RepeatActive[port])
	{
		KM_SYNC_Send_KB_Hid_Key(port,CHAR_REPEAT);	
	}		
}

/*----------------------------------------------------------------------------
 * void KM_SYNC_Repeat_Active_Force_Stop(U8_T keytype,U8_T keycode)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------- */
void KM_SYNC_Repeat_Active_Force_Stop(U8_T devinx)	
{
	//printf("Device_Remove=%bu\n\r",devinx);
	if (devinx == KM_SYNC_Sync_KB_Repeat_Devinx)
	{
		//printf("Force Stop repeat");
		memset(KM_SYNC_Sync_KB_RepeatActive,0,sizeof(KM_SYNC_Sync_KB_RepeatActive));
		memset(KM_SYNC_Sync_KB_HID_RepeatTableMake,0x00,sizeof(KM_SYNC_Sync_KB_HID_RepeatTableMake));
		KM_SYNC_Sync_KB_Repeat_Devinx = 0;
	}		
}

/*----------------------------------------------------------------------------
 * void KM_SYNC_Reset_Upsteam(U8_T keytype,U8_T keycode)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------- */ 
void KM_SYNC_Reset_Upsteam(void)
{
	U8_T uport;
	
	//1.Check in Sync Mode?
	//KM_Power_Led_Control(LED_OFF);
	if (KVM_Flash.cSystemFlag2 & SYSTEM_ALL_SYNC_MASK)
	{		
		for (uport=0; uport < USBDC_PORT_MAX ; uport++)
		{
			if (USBDC_UpPortState[uport] & USBDC_ROOTHUB_ATTACHED_MASK)
			{
				TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_KM_SYNC_DCReset_ID,0,uport,(100*(uport+1)),1); // now active the Generic KB Outpu
			}				
		}
	}		
	else
	{
		TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_KM_SYNC_DCReset_ID,0,KVM_CurrentHost,200,1); // now active the Generic KB Outpu
	}		
}
 
/*----------------------------------------------------------------------------
 * void TASK_KM_SYNC_DCReset(void)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------- */
/* 
void TASK_KM_SYNC_DCReset(void)
{	
	USBDC_Virtual_Hub_Reset(TASK_Register0,1);
	//KM_Power_Led_Control(LED_ON);
	TASK_Destory_Current();
}
*/
/*
void KM_Power_Led_Control(U8_T led_control)
{
	if (led_control == LED_ON)
		POWER_LED_PIN = 0;
	else
		POWER_LED_PIN = 1;
}	
*/
#if (KVM_HOTKEY_SUPPORT==0)
void HOTKEY_Handle(U8_T keytype,U8_T keycode)
{	
	/* Check hotkey */
	/* [1].Hotkey already actived */
	printf("HK ==0enter: t=%bx c=%bx\n\r",
    keytype, keycode);
	#ifdef SYNC_KB_HOTKEY
	KM_SYNC_KB_Hotkey_Check(keytype,keycode);
	#else
	keytype = 0;
	keycode = 0;
	#endif /* #ifSYNC_KB_HOTKEY */
}	
#endif
void KVM_SYNC_Init_Led_Set(U8_T port,U8_T led_value)
{
	if (KM_SYNC_KbLed_Set_State & BIT_MASK[port])
	{	
		KM_SYNC_Init_Led = led_value;
	}	
}

void TAKS_KM_SYNC_RepeatKeySend(void)
{	
	KM_SYNC_Check_HID_Repeat_State(TASK_Register0);	
	TASK_Destory_Current();
}	

void KM_SYNC_Send_RepeatKey_Check(U8_T port)
{
	if (KM_SYNC_Sync_KB_RepeatActive[port])
	{	
		TASK_Active(TASK_TYPE_INTERVAL_MS,TAKS_KM_SYNC_RepeatKeySend_ID,0,port,48,48); // now active the Generic KB Output Handle
	}
}

/*
 * ----------------------------------------------------------------------------
 * void      TASK_KM_HOTKEY_Active_Led_Indicate
 * Purpose : This task will indidate the hostkey status(Active or Deactive)
 *           also will do the hotkey valid time out check if enable timeout
 *           check mode.
 * Params  : NONE
 * Returns : NONE
 * Note    : NONE
 * ----------------------------------------------------------------------------
 */
void TASK_KM_HOTKEY_Active_Led_Indicate(void)
{
	
	if (KM_HOTKEY_LED != 0)
	{
		KM_HOTKEY_LED = 0;
	}
	else
	{
		KM_HOTKEY_LED = 0x07;
	}
 
	KM_HOTKEY_Active_Valid_Cnt++;
	if (KM_HOTKEY_Active_Valid_Cnt >= 10)
	{
		KM_HOTKEY_LED = KVM_HostLed[KVM_CurrentHost];		
		KVM_CONSOLE_Keyboard_Led_Control(KM_HOTKEY_LED);
		KM_HOTKEY_Active_Valid_Cnt	= 0;	
		TASK_Destory_Current();
	}	

	#if (KVM_HOTKEY_KB_LED)
	KVM_CONSOLE_Keyboard_Led_Control(KM_HOTKEY_LED);
	#endif
}

void KM_SYNC_Clear_Repeat_Table(void)
{
	memset(KM_SYNC_Sync_KB_RepeatTable,0x00,KM_SYNC_REPEAT_SIZ);
}

/*----------------------------------------------------------------------------
 * void KM_SYNC_Force_Port_Jump(U8_T port)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------- */
void KM_SYNC_Force_Port_Jump(U8_T port)
{				
	//Cancel the sync mode	
	KM_SYNC_ModeSync_Control(API_SYNC_DISABLE);
	KM_SYNC_ModeSwitch_Control(API_SWITCH_ENABLE);
	KVM_Console_Port_Jump(port);
	//Cancel the roaming mode
	//KM_SYNC_ModeAccross_Control(0);										
	//Do the port jump	
	//if (KVM_CurrentHost == port)
	//{	
	//	return;		
	//}
	
	//KVM_Port_Jump(port);	
	
}

/*-----------------------------------------------------------------------------
 * void KM_SYNC_Port_Add(U8_T port)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *----------------------------------------------------------------------------*/
void KM_SYNC_Port_Add(U8_T port)
{
	if (KM_SYNC_PortSetting & BIT_MASK[port])
	{
		printf("Sync Port Add:%bu\n\r",port);
		KM_SYNC_PortSetting &= ~BIT_MASK[port];
		KM_SYNC_KM_SyncModeLed(LED_ON);
	}	
}

/*-----------------------------------------------------------------------------
 * void KM_SYNC_Port_Clear(U8_T port)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *----------------------------------------------------------------------------*/
void KM_SYNC_Port_Clear(U8_T port)
{
	if ((KM_SYNC_PortSetting & BIT_MASK[port]) == 0)
	{			
		printf("Sync Port Clear:%bu\n\r",port);
		KM_SYNC_PortSetting |= BIT_MASK[port];		
		KM_SYNC_KM_SyncModeLed(LED_ON);
	}			
}

/*----------------------------------------------------------------------------
 * void TASK_KM_SYNC_DCReset(void)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------- */
void TASK_KM_SYNC_DCReset(void)
{	
	USBDC_Virtual_Hub_Reset(TASK_Register0,1);
	//KM_Power_Led_Control(LED_ON);
	TASK_Destory_Current();
}

/*----------------------------------------------------------------------------
 * void KM_SYNC_Set_SyncMode(void);
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------- */
void KM_SYNC_Set_SyncMode(void)
{
	U8_T mode;
	
	mode = !KM_SYNC_SyncMode_State();	
	//if turn on the sync mode, then stop all keyboard repeat 
	if (mode == API_SYNC_ENABLE)
		KM_SYNC_Stop_KB_Report();
	API_Set_Sync_Mode(mode);	
}	

//------------------------------------------------------------------------------------------
//Source code for VPID Change, for user to add
//------------------------------------------------------------------------------------------
/*-------------------------------------------------------------------------------
 * void KM_SYNC_Get_Ran(U8_T serial)
 * Purpose : get the radmon number for VID,PID useage
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------*/
void KM_SYNC_Get_Ran(U8_T serial)
{		
#ifdef VPID_CHANGER	
	srand(VPID_Changer_Seed+(123*serial)+128);
#else
	serial = 0;
#endif	
}	

/*-------------------------------------------------------------------------------
 * void KM_Sync_VPid_Change_Auto(U8_T port)
 * Purpose : get the radmon number for VID,PID useage
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------*/
void KM_Sync_VPid_Change_Auto(U8_T port)
{			
#ifdef VPID_CHANGER	
	if ((Ran_Desc[port].idVendor != 0x0b95) && (Ran_Desc[port].idVendor != 0xffff))//if not asix id
	{
		Ran_Desc[port].idVendor  = KVM_Flash.VPid[port].iVid;
		Ran_Desc[port].idProduct = KVM_Flash.VPid[port].iPid;
	}		
	else
	{	
		KM_SYNC_Get_Ran((port+13));
		Ran_Desc[port].idVendor = rand();
		
		KM_SYNC_Get_Ran(port+28);
		Ran_Desc[port].idProduct = rand();
		
		KVM_Flash.VPid_Change_State |= BIT_MASK[port];
		KVM_Flash.VPid[port].iVid = Ran_Desc[port].idVendor;
		KVM_Flash.VPid[port].iPid = Ran_Desc[port].idProduct;
		KVM_System_Config_Write();
	}
#else
	port = 0;
#endif	
}

/*------------------------------------------------------------------------------------------
 * void KM_Sync_VPid_Change_Handle(U8_T port_mask)
 * Purpose : After use the hotkey to call vid,pid change, user need call this to send to slave ax68004  
 * 			 This function need user to create different VID, PID in each port
 *           then store into the Ran_Desc[port].idVendor = ????,
 *           Ran_Desc[port].idProduct = ????, after that, then call KM_SYNC_Reset_Upsteam
 *           to reset each upstream port.
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------*/
void KM_Sync_VPid_Change_Handle(U8_T port_mask)
{
	//U16_T vid,pid;
#ifdef VPID_CHANGER	
	U8_T  i,serial=0;
		
	//Generate the VID, PID then Fill vid pid into Ran_Desc[port].idVendor & Ran_Desc[port].idProduct		
	for (i=0; i < 4 ; i++)
	{
		serial++;
		KM_SYNC_Get_Ran(serial);
		Ran_Desc[i].idVendor = rand();
		serial++;
		KM_SYNC_Get_Ran(serial);
		Ran_Desc[i].idProduct = rand();
		KVM_Flash.VPid_Change_State |= BIT_MASK[i];
		KVM_Flash.VPid[i].iVid = Ran_Desc[i].idVendor;
		KVM_Flash.VPid[i].iPid = Ran_Desc[i].idProduct;
	}
				
	KM_SYNC_Reset_Upsteam();	
	
	KVM_System_Config_Write();
#endif	
	port_mask = 0;
}

/*-----------------------------------------------------------------------------------------------------
 * void KM_SYNC_Manufacturer_Default(void)
 * Purpose : 
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *------------------------------------------------------------------------------------------------------*/
void KM_SYNC_Manufacturer_Default(void)
{		
	printf("Manufacturer Default\n\r");
	API_Active_Roaming_Mapping(SINGLE_SCREEN_MODE);	
	API_Set_Mouse_Mode(API_ABSOLUTE_MODE);
	API_Set_Sync_Mode(API_SYNC_ENABLE);		
	KM_SYNC_Force_Port_Jump(0);
	
	KM_SYNC_Sync_KB_Repeat = 0;
	memset(KM_SYNC_Sync_KB_RepeatActive,0,sizeof(KM_SYNC_Sync_KB_RepeatActive));
	KM_SYNC_HOTKEY_Modifiers = 0;
	memset(KM_SYNC_Sync_KB_RepeatTable,0x00,sizeof(KM_SYNC_Sync_KB_RepeatTable));
}

void KM_SYNC_Stop_KB_Report(void)
{
	printf("Stop Sync Repeat Status\n\r");
	memset(KM_SYNC_Sync_KB_RepeatActive,0x00,sizeof(KM_SYNC_Sync_KB_RepeatActive));
	memset(KM_SYNC_Sync_KB_HID_RepeatTableMake,0x00,sizeof(KM_SYNC_Sync_KB_HID_RepeatTableMake));
}
 
/*-----------------------------------------------------------------------------------------------------
 * void KM_SYNC_Handle_SetRepeatKey(U8_T keycode)
 * Purpose : 
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *------------------------------------------------------------------------------------------------------*/
void KM_SYNC_Handle_SetRepeatKey(U8_T keycode)
{
	U8_T i,tb,p;
	
	//Hotkey_Queue[1] = 0x00;
	//KM_SYNC_Sync_KB_HotkeyInx = 1; //wait next key in
	//p = KM_SYNC_Check_KB_Repeat_Port();
	if (keycode == CHAR_ESCAPE) //cancel all repeat set
	{
		printf("Clear All repeat key:%bu\n\r",p);
		KM_SYNC_Stop_KB_Report();
		KM_SYNC_Sync_KB_Repeat = 0;		
		//memset(KM_SYNC_Sync_KB_RepeatActive,0x00,sizeof(KM_SYNC_Sync_KB_RepeatActive));		
		memset(KM_SYNC_Sync_KB_RepeatTable,0x00,sizeof(KM_SYNC_Sync_KB_RepeatTable));		
	}
	else if (keycode == CHAR_TAB)
	{		
		if (KM_SYNC_SyncMode_State()) //if in sync, then clear all port repeat ative status
		{	
			//printf("Stop Sync Repeat Status\n\r");
			KM_SYNC_Stop_KB_Report();
			//memset(KM_SYNC_Sync_KB_RepeatActive,0x00,sizeof(KM_SYNC_Sync_KB_RepeatActive));
			//memset(KM_SYNC_Sync_KB_HID_RepeatTableMake,0x00,sizeof(KM_SYNC_Sync_KB_HID_RepeatTableMake));
		}	
		else
		{
			printf("Stop Port:%bu Repeat Status\n\r",KVM_CurrentHost);
			KM_SYNC_Sync_KB_RepeatActive[KVM_CurrentHost] = 0;		
			memset(KM_SYNC_Sync_KB_HID_RepeatTableMake[KVM_CurrentHost],0x00,8);			
		}			
	}		
	else	
	{						
		i=KM_SYNC_Search_Repeat_Table(KM_SYNC_Sync_KB_Repeat,keycode); //check the repeat keycode table
		if (i == 0xff)	//if not in the repeat keycode table	
		{ // add repeat key
			if (KM_SYNC_Sync_KB_Repeat != 0xff) //if not full
			{					
				tb = ~KM_SYNC_Sync_KB_Repeat;
				i=KM_SYNC_Search_Repeat_Table(tb,0x00); //check remain space for make key
				if (i != 0xff)								
				{					
					printf("Set repeat key=%02bx\n\r",keycode);							
					KM_SYNC_Sync_KB_Repeat |= BIT_MASK[i];
					KM_SYNC_Sync_KB_RepeatTable[i] = keycode;							 				
				}
			}	
		}									
	}			
}	

U8_T KM_SYNC_Port_Setting(U8_T p)
{
	if (KM_SYNC_PortSetting & BIT_MASK[p])
		return 1;
	return 0;
}

#endif /* #ifdef SYNC */
/* End of btn_interrupt.c */
