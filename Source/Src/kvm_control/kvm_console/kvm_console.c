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
 * Module Name : kvm_console.c
 * Purpose     : This module handles the kvm console & control function include
 *               1).Hotkey Control
 *               2).KVM switch control
 *               3).LED & Button access & KVM control
 *               4).Autoscan control
 *               5).Mouse/Keyboard control(only Active Port)
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
/* NAMING CONSTANT DECLARATIONS */
#if (PROJECT_KVM_CONSOLE_ENABLE)
#ifdef MCU_TYPE_AX68002
code GPIO_TypeDef	*VGA_CONTROL_PORT[] = {PORT2,PORT2};
code GPIO_PinTypeDef VGA_CONTROL_PIN[] =
{
	GPIO_Pin_0,
	GPIO_Pin_1,
	GPIO_Pin_2,
	GPIO_Pin_3,
	GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3
};
#endif

#ifdef MCU_TYPE_AX68004
code GPIO_TypeDef	*VGA_CONTROL_PORT[] = {PORT1,PORT1,PORT1,PORT1};
code GPIO_PinTypeDef VGA_CONTROL_PIN[] =
{
	GPIO_Pin_0,
	GPIO_Pin_1,
	GPIO_Pin_2,
	GPIO_Pin_3,
	GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3
};
#endif

#ifdef MCU_TYPE_AX68002	
code GPIO_TypeDef	*SELECT_LED_PORT[]={PORT2,PORT2};	
code GPIO_PinTypeDef SELECT_LED[] =
{
	GPIO_Pin_0,
	GPIO_Pin_2,	
};	
#else	
code GPIO_TypeDef	*SELECT_LED_PORT[]={PORT2,PORT2,PORT2,PORT2};	
code GPIO_PinTypeDef SELECT_LED[] =
{
	GPIO_Pin_0,
	#if (PCB_TEST_BOARD)
	GPIO_Pin_4,
	GPIO_Pin_2,
	#else
	GPIO_Pin_2,
	GPIO_Pin_4,
	#endif
	GPIO_Pin_6,
	GPIO_Pin_0|GPIO_Pin_2|GPIO_Pin_4|GPIO_Pin_6
};
#endif 

#ifdef MCU_TYPE_AX68002	
code GPIO_TypeDef	*HOST_LED_PORT[]={PORT2,PORT2};	
code GPIO_PinTypeDef HOST_LED[] =
{
	GPIO_Pin_1,
	GPIO_Pin_3,	
};
#else
code GPIO_TypeDef	*HOST_LED_PORT[]={PORT2,PORT2,PORT2,PORT2};	
code GPIO_PinTypeDef HOST_LED[] =
{
	GPIO_Pin_1,
	#if (PCB_TEST_BOARD)
	GPIO_Pin_5,
	GPIO_Pin_3,
	#else
	GPIO_Pin_3,
	GPIO_Pin_5,
	#endif
	GPIO_Pin_7,	
};
#endif /* 
/* STATIC VARIABLE DECLARATIONS */
#if (SYSTEM_PS2_HOST_ENABLE)
bit	TASK_KVM_CONSOLE_PS2_KBLed_Control_Flag;
#endif
bit 	KVM_Plug_In_Jump_Flag;
bit 	KVM_System_Not_Ready_Flag;
bit		KVM_Event_Control_Flag;
#if (SYSTEM_PS2_HOST_ENABLE)
bit		KVM_PS2_HotkeySwitch_Flag;
#endif

#define VS4210_1_Mode		1	//Single Monitor Mode
#define VS4210_4_Mode0		2	//4 equal Monitor Mode

#define KVM_PIP_MODE_COUNT    4
#define KVM_ONE_2_MODE_COUNT  3
#define KVM_PBP_MODE_COUNT    2
#define KVM_4WIN_MODE_COUNT   2
#define KVM_ONE_3_MODE_COUNT  4

U8_T	KVM_VS4210_Mode=VS4210_1_Mode;
U8_T	cModifiers;
U8_T	KVM_HostLed[KVM_MAX_PORT];
U8_T	KVM_CurrentHost,KVM_CurrentAudio,KVM_CurrentMSC;

U8_T KVM_Cycle_PIP_Mode=0;
U8_T KVM_Cycle_ONE_2_Mode=0;
U8_T KVM_Cycle_PBP_Mode=0;
U8_T KVM_Cycle_4WIN_Mode=0;
U8_T KVM_Cycle_ONE_3_Mode=0;

U8_T	KVM_CurrentUSBAudio;
U8_T	KVM_NextHost,KVM_NextAudio,KVM_NextAnalogAudio;
//U8_T	KVM_Mouse_Disable_Flag;
U8_T	TASK_KVM_EVENT_CONTROL_ID;
#if (SYSTEM_PS2_HOST_ENABLE)
U8_T	cPS2_KB_PressCnt;
#endif
U8_T	cUSB_KB_PressCnt; // PS/2 key press and release counter 
U8_T	TASK_KVM_AUTOSCAN_ID;
U8_T	TASK_KVM_AUTOSCAN_ACTIVE_ID;
U8_T	TASK_KVM_CONSOLE_PS2_KBLed_Control_ID;
U8_T	TASK_KVM_VGA_Switch_ID;
U8_T	TASK_KVM_ACCESSORY_ID;
U16_T	KVM_Control_Status;

//---------------------------
// For Autoscan 
U16_T	iAutoScanSecCnt;
U8_T	cAutoScanCurrentHost;
U8_T	cAutoScanNextHost;
//---------------------------

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) && (SYSTEM_SWITCH_AUDIO_DELAY)
U8_T KVM_Console_Switch_Audio_Delay_Task_ID;     
U8_T KVM_Console_Switch_Audio_Delay_Task_ActiveID=0;
#endif

static U8_T kvmVgaSwitchCnt;

struct	_KVM_Flash KVM_Flash;
struct	_KVM_Flash KVM_FlashTemporary;

TASK_ActiveTable_TypeDef	TASK_KVM_Switch_Event_Control_Task =
{
	TASK_TYPE_INTERVAL_MS, // Task Type
	0, // Task ID from 0~255
	0, // Task Wait for Semaphore to active
	0, // Task Event
	0, // Task transfer parameter
	KVM_SW_INTERVAL_TIME, // Task Interval time from 0~65535(ms/us)
	KVM_SW_INTERVAL_TIME, // Task Interval Reload
};

#if (KVM_BUTTON_SUPPORT) && (KVM_BUTTON_RELEASE_DELAY_INTERVAL)	
TASK_ActiveTable_TypeDef TASK_KVM_Button_Release_Delay_Task = 
{
	TASK_TYPE_INTERVAL_MS, // Task Type
	0, // Task ID from 0~255
	0, // Task Wait for Semaphore to active
	0, // Task Event
	0, // Task transfer parameter
	KVM_BUTTON_RELEASE_DELAY_INTERVAL, // Task Interval time from 0~65535(ms/us)
	100, // Task Interval Reload
};
#endif

/* LOCAL SUBPROGRAM DECLARATIONS */
//void	KVM_Select_Led_Control(U8_T,U8_T);
void	KVM_Host_Led_Control(U8_T,U8_T);
void	TASK_KVM_CONSOLE_PS2_KBLed_Control(void);
void 	TASK_KVM_VGA_Switch_Control(void);
void 	TASK_KVM_Accessory(void);
U8_T 	KVM_GetUsbDcEndp(U8_T devIndex, U8_T infIndex, U8_T endpIndex);

/* LOCAL SUBPROGRAM BODIES */
U8_T	KVM_Check_SOF_Count;
#if (KVM_BUTTON_SUPPORT) && (KVM_BUTTON_RELEASE_DELAY_INTERVAL)	
void 	TASK_KVM_Button_Release_Delay(void);
#endif

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) && (SYSTEM_SWITCH_AUDIO_DELAY)
void TASK_KVM_CONSOLE_Switch_Port_Audio_Control(void);
#endif

extern U8_T		TASK_KVM_HotkeyFork_Timeout_ID;

/*
 *--------------------------------------------------------------------------------
 * void KVM_Console_Init(void)
 * Purpose : KVM Console Init procedure( should read from some where? Flash?)
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void KVM_Console_Init(void)
{
	U8_T	index;

#if (SYSTEM_STORAGE_DEVICE_SUPPORT)
	STORAGE_Init();

	#if (SYSTEM_STORAGE_FLASH)
		STORAGE_SoftwareConfigurationCheck(0);
	#endif //End of (SYSTEM_STORAGE_FLASH)

#else
	/* KVM Console control relative */
	/* Autoscan interval time */
	KVM_Flash.AutoScanInterval = 5; // 0-1 defautl 5 second
	KVM_Flash.AutoScanStopCode = 0xff; // any key
	KVM_Flash.AutoScanStop_JumpCode = CHAR_SPACE;
	/* Control relative */
	KVM_Flash.cSystemFlag0 = SYSTEM_BEEPER_MASK; // 3 System Flag
	KVM_Flash.cSystemFlag1 = 0; // 4 System Flag2
	KVM_Flash.PowerSavingMode = SYSTEM_POWER_STOP_MODE; 	// Stop Mode
#endif //#if (SYSTEM_STORAGE_DEVICE_SUPPORT)

#ifdef KM104U
	KVM_Flash.cSystemFlag2 |= (SYSTEM_ALL_SYNC_MASK | SYSTEM_KB_SYNC_MASK | SYSTEM_MS_SYNC_MASK);
	if (KVM_Flash.cSystemFlag3 & KM_HOTKEY_MASK)
		KM_SYNC_Sync_KB_HotkeyChar = CHAR_PAD_STAR;
	else
		KM_SYNC_Sync_KB_HotkeyChar = CHAR_SCROLL_LOCK;
	//printf("HotkeyValue=%02bx\n\r",KM_SYNC_Sync_KB_HotkeyChar);
#endif
	
#ifdef KMLOG
	KVM_Flash.cSystemFlag2 &= ~(SYSTEM_ALL_SYNC_MASK | SYSTEM_KB_SYNC_MASK | SYSTEM_MS_SYNC_MASK);	
#endif	

#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
	if (KVM_Flash.cSystemFlag1 & SYSTEM_PEN_DRIVE_WRITE_MASK)
		scsi_Flash_Write_Protect_Flag = 1;
	else
		scsi_Flash_Write_Protect_Flag = 0;
#endif		

	/* Hotkey Indicate Control */
	for (index = 0; index < KVM_MAX_PORT; index++)
	{
		KVM_HostLed[index] = 0;
		KVM_Select_Led_Control(index,LED_OFF);
		KVM_Host_Led_Control(index,LED_OFF);
	}

	KVM_CurrentHost = 0;
	KVM_CurrentAudio = 0;
	KVM_CurrentUSBAudio = 0;
	KVM_CurrentMSC = 0x00;	
	KVM_Select_Led_Control(KVM_CurrentHost,LED_ON);
	KVM_Event_Control_Flag = 0;
	KVM_Control_Status = KVM_IDLE_STATUS;
 	KVM_Plug_In_Jump_Flag = 0;

	/* keyboad press counter */
#if (SYSTEM_PS2_HOST_ENABLE)
	cPS2_KB_PressCnt = 0; // PS/2 key press and release counter
#endif
	cUSB_KB_PressCnt = 0; // USB key press and release counter
//	KVM_CONSOLE_KB_Led_Inhibit_Flag = 0;
#if (SYSTEM_PS2_HOST_ENABLE)
	KVM_Mouse_Disable_Flag = 0;
	TASK_KVM_CONSOLE_PS2_KBLed_Control_Flag = 0;
	KVM_PS2_HotkeySwitch_Flag = 0;
#endif
	KVM_System_Not_Ready_Flag = 1;
	kvmVgaSwitchCnt = 0;
	/* Other */
	KVM_Check_SOF_Count = 0;

	/* do Hotkey init */
#if (KVM_HOTKEY_SUPPORT)	
	HOTKEY_Init();
	TASK_HOTKEY_LED_INDICATE_ID = TASK_Create(TASK_HOTKEY_Active_Led_Indicate);
	TASK_KVM_HotkeyFork_Timeout_ID = TASK_Create(TASK_KVM_HotkeyFork_Timeout);
#endif /* #if (KVM_HOTKEY_SUPPORT) */
	TASK_KVM_EVENT_CONTROL_ID = TASK_Create(TASK_KVM_Event_Control);
	TASK_KVM_Switch_Event_Control_Task.Task_ID = TASK_Create(TASK_KVM_Switch_Event_Control);
	TASK_KVM_AUTOSCAN_ID = TASK_Create(TASK_KVM_Autoscan_Event_Control);
//#ifdef KM104U	
	TASK_KVM_VGA_Switch_ID = TASK_Create(TASK_KVM_VGA_Switch_Control);
//#endif	
	TASK_KVM_ACCESSORY_ID = TASK_Create(TASK_KVM_Accessory);

#if (KVM_BUTTON_SUPPORT) && (KVM_BUTTON_RELEASE_DELAY_INTERVAL)	
	TASK_KVM_Button_Release_Delay_Task.Task_ID = TASK_Create(TASK_KVM_Button_Release_Delay);
#endif
	
#if (SYSTEM_PS2_HOST_ENABLE)
	TASK_KVM_CONSOLE_PS2_KBLed_Control_ID = TASK_Create(TASK_KVM_CONSOLE_PS2_KBLed_Control);
#endif //#if (SYSTEM_PS2_HOST_ENABLE)

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) && (SYSTEM_SWITCH_AUDIO_DELAY)
	KVM_Console_Switch_Audio_Delay_Task_ID = TASK_Create(TASK_KVM_CONSOLE_Switch_Port_Audio_Control);
#endif

#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("TASK_HOTKEY_LED_INDICATE_ID=%bu\n\r",TASK_HOTKEY_LED_INDICATE_ID);
	printf("TASK_KVM_HotkeyFork_Timeout_ID=%bu\n\r",TASK_KVM_HotkeyFork_Timeout_ID);
	printf("TASK_KVM_EVENT_CONTROL_ID=%bu\n\r",TASK_KVM_EVENT_CONTROL_ID);
	printf("TASK_KVM_Switch_Event_Control_Task.Task_ID=%bu\n\r",TASK_KVM_Switch_Event_Control_Task.Task_ID);
	printf("TASK_KVM_AUTOSCAN_ID=%bu\n\r",TASK_KVM_AUTOSCAN_ID);
	printf("TASK_KVM_VGA_Switch_ID=%bu\n\r",TASK_KVM_VGA_Switch_ID);
	printf("TASK_KVM_ACCESSORY_ID=%bu\n\r",TASK_KVM_ACCESSORY_ID);
#if (SYSTEM_PS2_HOST_ENABLE)
	printf("TASK_KVM_CONSOLE_PS2_KBLed_Control_ID=%bu\n\r",TASK_KVM_CONSOLE_PS2_KBLed_Control_ID);
#endif //#if (SYSTEM_PS2_HOST_ENABLE) 
#endif

#ifndef HID_PARSER_CORE_MS
	KVM_Flash.cSystemFlag2 |= SYSTEM_MS_DN_MASK;
#endif	
}

/*
 *--------------------------------------------------------------------------------
 * void KVM_CheckPS2Modifiers(U8_T,U8_T)
 * Purpose : check kvm modifier key
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
#if (SYSTEM_PS2_HOST_ENABLE)
void KVM_CheckPS2Modifiers(U8_T keytype,U8_T keycode)
{
	U8_T  cmodifier;
	
	cmodifier = 0;
	if ( keytype <= KB_TYPE_BREAK_F0 )
	{
		switch (keycode)
		{
			case 0x11 : // left alt
				cmodifier = 0x04;
				break;
			case 0x12 : // left shift
				cmodifier = 0x02;
				break;
			case 0x14 : // left ctrl cModifiers == 0x02
				cmodifier = 0x01;
				break;
			case 0x59 : // right shift
				cmodifier = 0x20;
				break;
		}
	}
	else
	{
		if ( keytype <= KB_TYPE_BREAK_E0 )
		{
			switch (keycode)
			{
				case 0x27 : // right GUI
					cmodifier = 0x80;	
					break;
				case 0x1f : // left GUI
					cmodifier = 0x08;	
					break;
				case 0x11 : // right alt
					cmodifier = 0x40;	
					break;
				case 0x14 : // right ctrl
					cmodifier = 0x10;
					break;
			}
		}
	}
	
	if ( (keytype == KB_TYPE_MAKE_E0) || (keytype == KB_TYPE_MAKE_F0) )
		cModifiers |= cmodifier;
	else
		cModifiers &= ~cmodifier;
}

/*
 *--------------------------------------------------------------------------------
 * void KVM_Hostkey_LED_PS2_Convert(U8_T kvm_led)
 * Purpose : check convert usb led format to ps/2 format
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */ 
U8_T KVM_Hostkey_LED_PS2_Convert(U8_T kvm_led)
{
	U8_T	led = 0;
	
	if (kvm_led & KVM_LED_NUM_LOCK)
	{
		led |= KVM_LED_PS2_NUM_LOCK;
	}
	
	if (kvm_led & KVM_LED_CAPS_LOCK)
	{
		led |= KVM_LED_PS2_CAPS_LOCK;
	}
	
	if (kvm_led & KVM_LED_SCROLL_LOCK)
	{
		led |= KVM_LED_PS2_SCROLL_LOCK;
	}
	
	return led;
}
#endif

/*
 *--------------------------------------------------------------------------------
 * void KVM_Select_Led_Control(U8_T port,U8_T led_control)
 * Purpose : Control the Select LED Port
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void KVM_Select_Led_Control(U8_T port,U8_T led_control)
{
	GPIO_SetOneBit(SELECT_LED_PORT[port],SELECT_LED[port],led_control);
}

#if (KVM_HOTKEY_SUPPORT)
/*
 *--------------------------------------------------------------------------------
 * void KVM_Autoscan_KB_Handle(U8_T codetype,U8_T keycode)
 * Purpose : 
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void KVM_Autoscan_KB_Handle(U8_T codetype,U8_T keycode)
{
	bit	result = 0;

	if (KVM_Control_Status & KVM_AUTOSCAN_MASK)
	{
		if ((codetype & 0x01)) // key break type
		{
			if (KVM_Flash.AutoScanStop_JumpCode && (KVM_Flash.AutoScanStop_JumpCode == keycode))
			{
				KVM_AutoscanStop_Jump(1,cAutoScanCurrentHost);
				result = 1;
			}
			else
			{
				if ((KVM_Flash.AutoScanStopCode & keycode) == keycode)
				{
					result = 1;
				}
			}
			
			if (result)
			{
				KVM_Autoscan_Mode_Contorl(0);
				//KVM_Control_Status &= ~KVM_AUTOSCAN_MASK; // clear the autoscan flag
				//Task_Active_Table[TASK_KVM_AUTOSCAN_ACTIVE_ID].Task_Interval.w = 1; //wait for 1 ms
			}
		}
	}
}
#endif /* KVM_HOTKEY_SUPPORT */
/*
 *--------------------------------------------------------------------------------
 * void KVM_Host_Led_Control(U8_T port,U8_T led_control)
 * Purpose : Control the Select LED Port
 * Params  : None
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void KVM_Host_Led_Control(U8_T port,U8_T led_control)
{		
#ifdef SYNC_SELECT_LED_ONLY		
	port = 0;
	led_control = 0;			
#else
	GPIO_SetOneBit(HOST_LED_PORT[port],HOST_LED[port],led_control);
#endif
	
#ifdef SYNC
	KM_SYNC_KM_SyncModeLed(LED_ON);
#endif
}

/*
 *--------------------------------------------------------------------------------
 * void LIB_Get_Bit_Status(void)
 * Purpose : Handle the hotkey mode deactive control
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
U8_T LIB_Get_Bit_Status(U8_T *table, U8_T index)
{
	U8_T	base_byte, search_bit;

	base_byte = index >> 3;	//8 bit as unit
	search_bit = 0x01 << (index & 0x07);	//the compare bit

	if (table[base_byte] & search_bit)
	{
		return 1;
	}
	return 0;
}
/*
 *--------------------------------------------------------------------------------
 * void KVM_GetNextConnect(U8_T currenthost,U8_T *nexthost)
 * Purpose : 
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
U8_T KVM_GetNextConnect(U8_T currenthost,U8_T *nexthost)
{
	U8_T	index;

	// look backward
	for (index = (currenthost+1); index < KVM_MAX_PORT; index++)
	{
		if (USBDC_UpPortState[index] & USBDC_ROOTHUB_ATTACHED_MASK)
		{
			*nexthost = index;
			return 1;
		}
	}

	// look forward
	for (index = 0; index < currenthost; index++)
	{
		if (USBDC_UpPortState[index] & USBDC_ROOTHUB_ATTACHED_MASK)
		{
			*nexthost = index;
			return 1;
		}
	}

	*nexthost = currenthost;
	return 0;
}

/*
 *--------------------------------------------------------------------------------
 * void KVM_GetNextAutoScanHost(U8_T currenthost,U8_T *nexthost)
 * Purpose : 
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
U8_T KVM_GetNextAutoScanHost(U8_T currenthost,U8_T *nexthost)
{
	/* if Free Scan Mode */
	if ((KVM_Flash.cSystemFlag0 & SYSTEM_AUTOSCAN_MODE_MASK) == SYSTEM_AUTOSCAN_MODE_FREE)
	{
		*nexthost = currenthost+1;
		if (*(nexthost) >= KVM_MAX_PORT)
		{
			*nexthost = 0;
		}
		return 1;
	}
	/* if Scan Mode with Power status */
	if ((KVM_Flash.cSystemFlag0 & SYSTEM_AUTOSCAN_MODE_MASK) == SYSTEM_AUTOSCAN_MODE_POWER)
	{
		if (KVM_GetNextConnect(currenthost,nexthost))
		{
			return 1;
		}
	}
	return 0;
}

/*
 *--------------------------------------------------------------------------------
 * void TASK_KVM_Autoscan_Event_Control(void)
 * Purpose : Handle the hotkey mode deactive control
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void TASK_KVM_Autoscan_Event_Control(void)
{
	if (KVM_Control_Status & KVM_AUTOSCAN_MASK)
	{
		iAutoScanSecCnt++;
		if ((iAutoScanSecCnt >> 1) >= KVM_Flash.AutoScanInterval)
		{
			//1.Changed the LED status
			iAutoScanSecCnt = 0; // reset the autoscan second counter
			if (KVM_GetNextAutoScanHost(cAutoScanCurrentHost,&cAutoScanNextHost))
			{
				KVM_Select_Led_Control(cAutoScanCurrentHost,LED_OFF);
				cAutoScanCurrentHost = cAutoScanNextHost;
				KVM_Select_Led_Control(cAutoScanCurrentHost,LED_ON);
				KVM_CONSOLE_Vga_Control(cAutoScanCurrentHost,VGA_ON);
			}
		}
		else
		{
			if (iAutoScanSecCnt & 0x01)
			{
				KVM_Select_Led_Control(cAutoScanCurrentHost,LED_OFF);
			}
			else
			{
				KVM_Select_Led_Control(cAutoScanCurrentHost,LED_ON);
			}
		}
	}
	else
	{
		KVM_Select_Led_Control(cAutoScanCurrentHost,LED_OFF);
		if ((KVM_Control_Status & KVM_PORT_SW_MASK) == 0x00) // if not in autoscan stop & jump mode
		{
			KVM_Select_Led_Control(KVM_CurrentHost,LED_ON);
			KVM_CONSOLE_Vga_Control(KVM_CurrentHost,VGA_ON);
			
#if (KVM_BUZZER_SUPPORT)
			BUZZER_Script_Active(AutoScan_Sound_Stop);
#endif
		}
		TASK_Destory_Current();
	}
}


/*
 *--------------------------------------------------------------------------------
 * U8_T KVM_GetUsbDcEndp(U8_T devIndex, U8_T infIndex, U8_T endpIndex)
 * Purpose : Get USB DC endp index
 * Params  : devIndex : USB_PDevice[] index.
 *           infIndex : USB HC interface number.
 *           endpIndex : USB HC endpoint number.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
U8_T KVM_GetUsbDcEndp(U8_T devIndex, U8_T infIndex, U8_T endpIndex)
{
	U8_T	intt_id;
	
	intt_id = USB_PDevice[devIndex].EndpType[infIndex][endpIndex] & 0x1f;
		
	return ((HC_IntTransfer_Table[intt_id].EndpIdx & USBDC_DEV_ENDPINX_MASK ) >> 4);
}

/*
 *--------------------------------------------------------------------------------
 * void TASK_KVM_Switch_Event_Control(void)
 * Purpose : Handle the hotkey mode deactive control
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void TASK_KVM_Switch_Event_Control(void)
{
	U8_T	device;
#if (SYSTEM_KVM)	
	U8_T	dev_index;
	U8_T 	inf_index;
	U8_T	end_index;
	U8_T	interfacenum;
	U8_T	endpidx;
#endif

#if (SYSTEM_HARDWARE_PS2_ENABLE) && (SYSTEM_KVM) && (!SYSTEM_EXTENDER_SUPPORT)
	// Check if any characters comes from PS2 keyboard
	if (cPS2_KB_PressCnt)
	{
		DATAST_Send_Kb_Break(KVM_CurrentHost,USBDC_VHID_DEVINX, USBDC_VHID_KB_ENDP_NUM, 0, 8);
	}
#endif


	// Check if any characters comes from USB keyboard 
#if (SYSTEM_KVM)
	if (cUSB_KB_PressCnt)
	{
		for (dev_index = 0; dev_index < USB_HC_MAX_DEVICE; dev_index++)		// polling USB_PDevice[] to find keyboard device
		{
			if (USB_PDevice[dev_index].HID_Kb_Press_Cnt == 0)
				continue;
				
			interfacenum = USB_PDevice[dev_index].InterfaceNum & USBHC_DEVICE_INTERFACENUM_MASK;
			for (inf_index = 0; inf_index < interfacenum; inf_index++) // Check Eache Interface
			{
				for (end_index=0; end_index < USB_PDevice[dev_index].EndpNum[inf_index]; end_index++) // Check all the endpoint in this interface
				{
					if (USB_PDevice[dev_index].InfProtocol[inf_index] == INF_KEYBOARD)
					{
						if ((USB_PDevice[dev_index].idVendor == A4TECH_IVENDOR) &&(USB_PDevice[dev_index].idProduct == A4TECH_IPORDUCT_GV800V))
						{
							endpidx = KVM_GetUsbDcEndp (dev_index, inf_index, end_index);
							DATAST_Send_Kb_Break(KVM_CurrentHost,dev_index, endpidx, 1, 13);
						}
						else
						{
							endpidx = KVM_GetUsbDcEndp (dev_index, inf_index, end_index);
							DATAST_Send_Kb_Break(KVM_CurrentHost,dev_index, endpidx, 0, 8);
						}
					}
					else
					{
						//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
						//1.Special handle for Gigabyte
						if ((USB_PDevice[dev_index].idVendor == 0x060b) &&(USB_PDevice[dev_index].idProduct == 0x2270))
						{
							if (USB_PDevice[dev_index].HID_Gigabyte_Kb_04_Cnt && USB_PDevice[dev_index].HID_Gigabyte_Kb_03_Flag == 1)  // already send report ID 03
							{
								endpidx = KVM_GetUsbDcEndp (dev_index, inf_index, end_index);
								DATAST_Send_Kb_Break(KVM_CurrentHost,dev_index, endpidx, 4, 8);
								USB_PDevice[dev_index].HID_Gigabyte_Kb_03_Flag = 0;
								continue;
							}									

							if (USB_PDevice[dev_index].HID_Gigabyte_Kb_03_Cnt && USB_PDevice[dev_index].HID_Gigabyte_Kb_03_Flag == 0) // first time send report ID 03
							{
								endpidx = KVM_GetUsbDcEndp (dev_index, inf_index, end_index);
								DATAST_Send_Kb_Break(KVM_CurrentHost,dev_index, endpidx, 3, 8);
								USB_PDevice[dev_index].HID_Gigabyte_Kb_03_Flag = 1;
							}
									
							if (USB_PDevice[dev_index].HID_Gigabyte_Kb_04_Cnt && USB_PDevice[dev_index].HID_Gigabyte_Kb_03_Flag == 1)
							{
								return;
							}
							USB_PDevice[dev_index].HID_Gigabyte_Kb_03_Flag = 0;
								
							if (USB_PDevice[dev_index].HID_Gigabyte_Kb_04_Cnt)  // only need to send report ID 04
							{
								endpidx = KVM_GetUsbDcEndp (dev_index, inf_index, end_index);
								DATAST_Send_Kb_Break(KVM_CurrentHost,dev_index, endpidx, 4, 8);
							}
						}
					}
				}
			}
		}
		cUSB_KB_PressCnt = 0;
	}
#endif /* #if (SYSTEM_HARDWARE_PS2_ENABLE) && (SYSTEM_KVM) && (!SYSTEM_EXTENDER_SUPPORT) */
	/* Check if previous KVM switching task still process */
	if (TASK_USBHC_KVM_Switch_Check_Start_Flag)
	{
		//TASK_Destory_Current();
		return;
	}

#if (PROJECT_USB_GENERIC_HID_ENABLE)
	/* Clear KB & Mouse output Queue */
	DATAST_Reset_GenericQueue();
#endif /* #if (PROJECT_USB_GENERIC_HID_ENABLE) */	
	
	/* Disable the old state for LED */
#if (SYSTEM_PS2_HOST_ENABLE)
	KVM_PS2_HotkeySwitch_Flag = 0;
#endif
	KVM_Control_Status &= ~KVM_PORT_SW_MASK;
	KVM_Select_Led_Control(KVM_CurrentHost,LED_OFF);
	
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) && (SYSTEM_SWITCH_AUDIO_DELAY)
	if (KVM_Flash.cSystemFlag1 & SYSTEM_AUDIOCOMBO_MASK)
	{	
		KVM_CONSOLE_Switch_Port_Audio_Control(KVM_CurrentHost);
	}	
#endif
	
	/* Update current host and check if the host needs to wake up */
	KVM_CurrentHost = TASK_Register0;

// #if (KVM_CONSOLE_DEBUG_MODE)
	printf("CG<P:%bu>\n\r",KVM_CurrentHost);
	//printf("DEBUG:KVM_Console: Port:%bu,switched(%bu)\n\r",KVM_CurrentHost,(U8_T)KM_SYNC_HOTKEY_Skip_Flag);
// #endif

	/* Update the new state for LED */
	KVM_Select_Led_Control(KVM_CurrentHost,LED_ON);
	KVM_CONSOLE_Vga_Control(KVM_CurrentHost,VGA_ON);
#if (SYSTEM_PS2_HOST_ENABLE)
	TASK_KVM_CONSOLE_PS2_KBLed_Control_Flag = 0;
#endif

	KVM_CONSOLE_Keyboard_Led_Control(KVM_HostLed[KVM_CurrentHost]);

#if (SYSTEM_EXTENDER_TRANSMITTER)
	ExtenderT_KVM_KB_Led_Check(KVM_CurrentHost);
#endif

#if (SYSETM_GPIO_EEPROM_ENABLE)
	GPIO_WriteEEprom(); 
#endif

#if (KVM_BUTTON_SUPPORT)
#if (KVM_BUTTON_RELEASE_DELAY_INTERVAL)
	TASK_Run(&TASK_KVM_Button_Release_Delay_Task);
#else
	BTN_Processing_Flag = 0;
#endif /*	#if (KVM_BUTTON_SUPPORT) */
#endif /* 	#if (KVM_BUTTON_RELEASE_DELAY_INTERVAL) */

	TASK_Destory_Current();

#if (KVM_BUZZER_SUPPORT)
	#ifdef SYNC
	if ((Coordinate_State & MOUSE_JUMP_MASK) == 0)
	#endif	
	{
		BUZZER_Script_Active(PortSwitch_Sound);
	}	
#endif //#if (KVM_BUZZER_SUPPORT)
	device = (USB_HC_MAX_DEVICE << 4) | USBDC_VIRTUAL_DEVINX;
	TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_KVM_Switch_Check_ID,0x00,device,KVM_CurrentHost,0); //Generate the task, for next Event check

#ifdef SYNC
#ifdef HID_PARSER_CORE_MS 
	KM_SYNC_Mouse_Jump_State_Reset();
#endif
#endif
}

/*--------------------------------------------------------------------------------
 * void TASK_KVM_Accessory(void)
 * Purpose : Check the system ready(for button) or SOF from upper system
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------*/
void TASK_KVM_Accessory(void)
{
	if (KVM_System_Not_Ready_Flag)
	{
		/* Set system ready */
		KVM_System_Not_Ready_Flag = 0;
		USBDC_Resume_Task_ActiveID[TASK_Register0] = 0;
	}
	else
	{
		/* Check SOF of assigned upstream port */
#if (SYSTEM_EXTENDER_RECEIVER)
		if (TASK_Register0 != REMOTE_HOST_PORT)
#endif //#if (SYSTEM_EXTENDER_RECEIVER)
		{
			if (USBDC_HAL_Port_Check_SOF(TASK_Register0))
			{
				//printf("> DC_Port:%bu,SOF detected\n\r",TASK_Register0);
				USBDC_Port_Normal(TASK_Register0);
				KVM_Check_SOF_Count = 0;
				USBDC_Resume_Task_ActiveID[TASK_Register0] = 0;
				TASK_Destory_Current();
			}
			else
			{
				if (++KVM_Check_SOF_Count >= 20)
				{
					KVM_Check_SOF_Count = 0;
					USBDC_Resume_Task_ActiveID[TASK_Register0] = 0;
					TASK_Destory_Current();
				}
			}
		}
	}
}

void KVM_Autoscan_Mode_Contorl(U8_T control)
{
	if (control) // autoscan mode start
	{
		if (KVM_Control_Status & KVM_AUTOSCAN_MASK)
			return;
		KVM_Control_Status |= KVM_AUTOSCAN_MASK;
		iAutoScanSecCnt = 0;
		cAutoScanCurrentHost = KVM_CurrentHost;
		TASK_KVM_AUTOSCAN_ACTIVE_ID = TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_KVM_AUTOSCAN_ID,0,0,500,500); // now aticev the process for time out event
#if (KVM_BUZZER_SUPPORT)
		BUZZER_Script_Active(AutoScan_Sound_Start);
#endif //#if (KVM_BUZZER_SUPPORT)		
	}
	else
	{
		if ((KVM_Control_Status & KVM_AUTOSCAN_MASK) == 0x00)
			return;
		
		KVM_Control_Status &= ~KVM_AUTOSCAN_MASK; // clear the autoscan flag
		Task_Active_Table[TASK_KVM_AUTOSCAN_ACTIVE_ID].Task_Interval.w = 1; //wait for 1 ms
	}		
}	
/*
 *--------------------------------------------------------------------------------
 * void TASK_KVM_Event_Control(void)
 * Purpose : Handle the hotkey mode deactive control
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void TASK_KVM_Event_Control(void)
{
	bit		system_config_changed = 0;
#if (KVM_HOTKEY_SUPPORT)
	U8_T	result = 0;
#endif

#if (ENABLE_HOTKEY_FUN_AUDIO_SW && SYSTEM_AUDIO_MAX != 0)	
	U8_T    tempi;
#endif
	
	if (KVM_Event_Control_Flag)
	{
#if (KVM_HOTKEY_SUPPORT)
		printf("HK exec: fun=%bx val=%d vidx=%bx\n\r",
			HOTKEY_Funciton_Code,
			HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex],
			HOTKEY_Funciton_PerformValueIndex);
		switch (HOTKEY_Funciton_Code)
		{
			#ifdef SYNC
			case  HOTKEY_FUN_MS_ROAMING_MODE:
				printf("HK exec: single screen cycle mode=%bu\n\r", KVM_Cycle_PIP_Mode);
				switch (KVM_Cycle_PIP_Mode)
				{
					case 0:
						KVM_SET_mode(KVM_MODE_PORT1_ONLY);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_PORT1_ONLY);
						break;

					case 1:
						KVM_SET_mode(KVM_MODE_PORT2_ONLY);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_PORT2_ONLY);
						break;

					case 2:
						KVM_SET_mode(KVM_MODE_PORT3_ONLY);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_PORT3_ONLY);
						break;

					case 3:
						KVM_SET_mode(KVM_MODE_PORT4_ONLY);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_PORT4_ONLY);
						break;

					default:
						KVM_Cycle_PIP_Mode = 0;
						KVM_SET_mode(KVM_MODE_PORT1_ONLY);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_PORT1_ONLY);
						break;
				}

				KVM_Cycle_PIP_Mode++;
				if (KVM_Cycle_PIP_Mode >= KVM_PIP_MODE_COUNT)
				{
					KVM_Cycle_PIP_Mode = 0;
				}
				break;

			case HOTKEY_FUN_KVM_ONE_2_MODE:
				printf("HK exec: one-two cycle mode=%bu\n\r", KVM_Cycle_ONE_2_Mode);
				switch (KVM_Cycle_ONE_2_Mode)
				{
					case 0:
						KVM_SET_mode(KVM_MODE_TWO_SMALL_MAIN_PORT1);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_TWO_SMALL_MAIN_PORT1);
						break;

					case 1:
						KVM_SET_mode(KVM_MODE_TWO_SMALL_MAIN_PORT2);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_TWO_SMALL_MAIN_PORT2);
						break;

					case 2:
						KVM_SET_mode(KVM_MODE_TWO_SMALL_MAIN_PORT3);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_TWO_SMALL_MAIN_PORT3);
						break;

					default:
						KVM_Cycle_ONE_2_Mode = 0;
						KVM_SET_mode(KVM_MODE_TWO_SMALL_MAIN_PORT1);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_TWO_SMALL_MAIN_PORT1);
						break;
				}

				KVM_Cycle_ONE_2_Mode++;
				if (KVM_Cycle_ONE_2_Mode >= KVM_ONE_2_MODE_COUNT)
				{
					KVM_Cycle_ONE_2_Mode = 0;
				}
				break;

			case HOTKEY_FUN_KVM_PBP_MODE:
				printf("HK exec: pbp cycle mode=%bu\n\r", KVM_Cycle_PBP_Mode);
				switch (KVM_Cycle_PBP_Mode)
				{
					case 0:
						KVM_SET_mode(KVM_MODE_PORT12_ONLY);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_PORT12_ONLY);
						break;

					case 1:
						KVM_SET_mode(KVM_MODE_PORT34_ONLY);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_PORT34_ONLY);
						break;

					default:
						KVM_Cycle_PBP_Mode = 0;
						KVM_SET_mode(KVM_MODE_PORT12_ONLY);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_PORT12_ONLY);
						break;
				}

				KVM_Cycle_PBP_Mode++;
				if (KVM_Cycle_PBP_Mode >= KVM_PBP_MODE_COUNT)
				{
					KVM_Cycle_PBP_Mode = 0;
				}
				break;

			case HOTKEY_FUN_KVM_4WIN_MODE:
				printf("HK exec: four-win cycle mode=%bu\n\r", KVM_Cycle_4WIN_Mode);
				switch (KVM_Cycle_4WIN_Mode)
				{
					case 0:
						KVM_SET_mode(KVM_MODE_PORT_ALL);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_PORT_ALL);
						break;

					case 1:
						KVM_SET_mode(KVM_MODE_PORT_ALL_SYNC);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_PORT_ALL_SYNC);
						break;

					default:
						KVM_Cycle_4WIN_Mode = 0;
						KVM_SET_mode(KVM_MODE_PORT_ALL);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_PORT_ALL);
						break;
				}

				KVM_Cycle_4WIN_Mode++;
				if (KVM_Cycle_4WIN_Mode >= KVM_4WIN_MODE_COUNT)
				{
					KVM_Cycle_4WIN_Mode = 0;
				}
				break;

			case HOTKEY_FUN_KVM_ONE_3_MODE:
				printf("HK exec: one-three cycle mode=%bu\n\r", KVM_Cycle_ONE_3_Mode);
				switch (KVM_Cycle_ONE_3_Mode)
				{
					case 0:
						KVM_SET_mode(KVM_MODE_THREE_SMALL_MAIN_PORT1);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_THREE_SMALL_MAIN_PORT1);
						break;

					case 1:
						KVM_SET_mode(KVM_MODE_THREE_SMALL_MAIN_PORT2);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_THREE_SMALL_MAIN_PORT2);
						break;

					case 2:
						KVM_SET_mode(KVM_MODE_THREE_SMALL_MAIN_PORT3);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_THREE_SMALL_MAIN_PORT3);
						break;

					case 3:
						KVM_SET_mode(KVM_MODE_THREE_SMALL_MAIN_PORT4);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_THREE_SMALL_MAIN_PORT4);
						break;

					default:
						KVM_Cycle_ONE_3_Mode = 0;
						KVM_SET_mode(KVM_MODE_THREE_SMALL_MAIN_PORT1);
						KVM_UART_SendFrame(KVM_CMD_SET_MODE,KVM_MODE_THREE_SMALL_MAIN_PORT1);
						break;
				}

				KVM_Cycle_ONE_3_Mode++;
				if (KVM_Cycle_ONE_3_Mode >= KVM_ONE_3_MODE_COUNT)
				{
					KVM_Cycle_ONE_3_Mode = 0;
				}
				break;

			case  HOTKEY_FUN_MS_SYNC_MODE:
				printf("HK exec: sync=%d\n\r", HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex]);
				KM_SYNC_ModeSync_Control(HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex]);								
				break;
			case HOTKEY_FUN_MS_RELATIVE_MODE:
				printf("HK exec: mouse relative\n\r");
				KM_SYNC_Mouse_Relative_Mode();
				break;
			case HOTKEY_FUN_MS_ABSOLUTE_MODE:
				printf("HK exec: mouse absolute\n\r");
				KM_SYNC_Mouse_Absolute_Mode();
				break;
			#endif
		//-----------------------------------------------------------------------
		// Port Switching Control 
		//-----------------------------------------------------------------------
			case HOTKEY_FUN_ALTERNAMTE_CONTROL:
				if (HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex])
				{
					KVM_Flash.cSystemFlag0 |= SYSTEM_HOTKEY_ALTERNATE_MASK;
				}
				else
				{
					KVM_Flash.cSystemFlag0 &= ~SYSTEM_HOTKEY_ALTERNATE_MASK;
				}
				break;
			case HOTKEY_FUN_PORT_SW_NUMERIC:
			case HOTKEY_FUN_PORT_SW_NONUM:
				KVM_NextHost = HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex]-1;				
				printf("HK exec: port jump value=%d next=%bx\n\r",
					HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex],
					KVM_NextHost);
				KVM_Console_Port_Jump(KVM_NextHost);
				/*
				if ((KVM_Control_Status & KVM_PORT_SW_MASK) == 0)
				{					
					if (KVM_CurrentHost != KVM_NextHost ) // changed active port id
					{						
						if (KVM_Power_Jump_Check(KVM_NextHost,0)) 
						{					
							KVM_Port_Jump(KVM_NextHost);									
						}
					}
				}
				*/
				break;
		//-----------------------------------------------------------------------
		// Audio Control & Switching
		//-----------------------------------------------------------------------
#if	(SYSTEM_AUDIO_DEVICE_SUPPORT) || (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
#if (ENABLE_HOTKEY_FUN_AUTOAUDIO_SW)
			case HOTKEY_FUN_AUTOAUDIO_SW: //Auto Switch Audio when port switching				
				if (HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex])
				{
					KVM_Flash.cSystemFlag1 |= SYSTEM_AUDIOCOMBO_MASK;
				}
				else
				{
					KVM_Flash.cSystemFlag1 &= ~SYSTEM_AUDIOCOMBO_MASK;
				}

				//After the combo switch mode is on,check the current audio portid
				if (KVM_Flash.cSystemFlag1 & SYSTEM_AUDIOCOMBO_MASK)
				{
					KVM_Audio_Combo_Switch_Check(KVM_CurrentHost);					
				}
				system_config_changed = 1;				
				break;
#endif //#if (ENABLE_HOTKEY_FUN_AUTOAUDIO_SW)

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
#if (ENABLE_HOTKEY_FUN_USBAUDIO_SW) 
			case HOTKEY_FUN_USBAUDIO_SW: //USB Audio device Switch Control
				if ((KVM_Flash.cSystemFlag1 & SYSTEM_AUDIOCOMBO_MASK) == 0)
				{
					KVM_NextAudio = HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex]-1;
					USBHC_Audio_Hotkey_Sw_Flag = 1;
					USBHC_AUDIO_Switch(KVM_NextAudio);
				}
				break;
#endif //#if (ENABLE_HOTKEY_FUN_USBAUDIO_SW)
#endif //#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
#if (ENABLE_HOTKEY_FUN_USBAUDIO_REPORT)
			case HOTKEY_FUN_USBAUDIO_REPORT_CONTROL: //USB Audio device report rule control
				if (HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex] == 0) 
				{
					if (KVM_Flash.cSystemFlag0 & SYSTEM_AUDIO_REPORT_ALL_MASK)
					{
						USBHC_AUDIO_ReportModeChanged_Mount_Control(USBAUDIO_REPORT_SINGLE);
						KVM_Flash.cSystemFlag0 &= ~SYSTEM_AUDIO_REPORT_ALL_MASK;
						system_config_changed = 1;
					}							
				}	
				else if (HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex] == 1)
				{	
					if ((KVM_Flash.cSystemFlag0 & SYSTEM_AUDIO_REPORT_ALL_MASK) == 0x00)
					{	
						USBHC_AUDIO_ReportModeChanged_Mount_Control(USBAUDIO_REPORT_ALL); 
						KVM_Flash.cSystemFlag0 |= SYSTEM_AUDIO_REPORT_ALL_MASK;
						system_config_changed = 1;
					}	
				}	
				break;
#endif //#if (ENABLE_HOTKEY_FUN_USBAUDIO_REPORT)
#endif //#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)

#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
#if (ENABLE_HOTKEY_FUN_AUDIO_SW && SYSTEM_AUDIO_MAX != 0)
			case HOTKEY_FUN_AUDIO_SW: //Analog Switch Control
				//BUZZER_Adjust_Factor = HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex]-1
				//Btn_Active_Sound.Keeptime = BUZZER_Factor_Table[BUZZER_Adjust_Factor];
				//printf("Curent Buzzer Setting = %bu\n\r",BUZZER_Adjust_Factor);
				tempi = HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex]-1;
				KVM_Audio_Switch(tempi);
				break;
#endif //#if (ENABLE_HOTKEY_FUN_AUDIO_SW)
#endif //#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
#endif //#if ((SYSTEM_USBAUDIO_DEVICE_SUPPORT) || (SYSTEM_AUDIO_DEVICE_SUPPORT)

		//-----------------------------------------------------------------------
		// Buzzer Control
		//-----------------------------------------------------------------------
#if (ENABLE_HOTKEY_FUN_BUZZER_CONTROL)
			case HOTKEY_FUN_BUZZER_CONTROL_TOGGLE:
				if (HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex]) // USB PEN Drive is Disattached
				{
					printf("Buzzer ON\n\r");
					KVM_Flash.cSystemFlag0 |= SYSTEM_BEEPER_MASK;
#if (KVM_BUZZER_SUPPORT)
					BUZZER_Script_Active(Hotkey_Active_Sound);
#endif //#if (KVM_BUZZER_SUPPORT)
				}
				else
				{
					printf("Buzzer OFF\n\r");
					KVM_Flash.cSystemFlag0 &= ~SYSTEM_BEEPER_MASK;
				}
				system_config_changed = 1;				
				break;
#endif //#if (ENABLE_HOTKEY_FUN_BUZZER_CONTROL)

		//-----------------------------------------------------------------------
		// Autoscan Control & Interval
		//-----------------------------------------------------------------------
			case HOTKEY_FUN_AUTOSCAN_CONTROL:
				printf("HK exec: autoscan start\n\r");
				KVM_Autoscan_Mode_Contorl(1);		
				//KVM_Control_Status |= KVM_AUTOSCAN_MASK;
				//iAutoScanSecCnt = 0;
				//cAutoScanCurrentHost = KVM_CurrentHost;
				//TASK_KVM_AUTOSCAN_ACTIVE_ID = TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_KVM_AUTOSCAN_ID,0,0,500,500); // now aticev the process for time out event
#if (KVM_BUZZER_SUPPORT)
				//BUZZER_Script_Active(AutoScan_Sound_Start);
#endif //#if (KVM_BUZZER_SUPPORT)
				break;
			case HOTKEY_FUN_AUTOSCAN_INTERVAL:				
				KVM_Flash.AutoScanInterval = HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex];
				printf("HK exec: autoscan interval=%d\n\r", KVM_Flash.AutoScanInterval);
				system_config_changed = 1;
				break;

		//-----------------------------------------------------------------------
		// Switch High Speed HUB
		//-----------------------------------------------------------------------
#if (ENABLE_HOTKEY_FUN_HUB_SW)
			case HOTKEY_FUN_HUB_SW:
				break;
#endif //#if (ENABLE_HOTKEY_FUN_HUB_SW)
		
		//-----------------------------------------------------------------------
		// Upstream port reset control
		//-----------------------------------------------------------------------
#if (ENABLE_HOTKEY_FUN_PORT_RESET)
			case HOTKEY_FUN_PORT_RESET:
				break;
#endif //#if (ENABLE_HOTKEY_FUN_PORT_RESET)

		//-----------------------------------------------------------------------
		// Power Jump Setting control
		//-----------------------------------------------------------------------
#if (ENABLE_HOTKEY_FUN_POWER_JUMP_SW)
			case HOTKEY_FUN_PORT_JUMP_POWER_CONTROL:
				if (HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex])
				{
					KVM_Flash.cSystemFlag0 |= (SYSTEM_JUMP_MODE_MASK|SYSTEM_AUTOSCAN_MODE_POWER);
					KVM_Power_Jump_Check(KVM_CurrentHost,1); 
				}
				else
				{
					KVM_Flash.cSystemFlag0 &= ~(SYSTEM_JUMP_MODE_MASK|SYSTEM_AUTOSCAN_MODE_POWER);
				}
				system_config_changed = 1;
				break;
#endif //#if (ENABLE_HOTKEY_FUN_POWER_JUMP_SW)
		
		//-----------------------------------------------------------------------
		// Plug In Jump Setting control
		//-----------------------------------------------------------------------
#if (ENABLE_HOTKEY_FUN_PLUGIN_JUMP_SW)
			case HOTKEY_FUN_PLUGIN_JUMP_CONTROL:
				if (HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex])
				{
					KVM_Flash.cSystemFlag1 |= SYSTEM_PLUGIN_JUMP_MASK;
				}
				else
				{
					KVM_Flash.cSystemFlag1 &= ~SYSTEM_PLUGIN_JUMP_MASK;
				}
				system_config_changed = 1;
				break;
#endif //#if (ENABLE_HOTKEY_FUN_PLUGIN_JUMP_SW)
		
		//-----------------------------------------------------------------------
		// Plug In Jump Mode Control
		//-----------------------------------------------------------------------
#if (ENABLE_HOTKEY_FUN_PLUGIN_JUMP_SW)
			case HOTKEY_FUN_PLUGIN_JUMP_MODE:
				if (HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex])
				{
					KVM_Flash.cSystemFlag1 |= SYSTEM_PLUGIN_JUMP_MODE;
					if ((USBDC_UpPortState[KVM_CurrentHost] & USBDC_ROOTHUB_ATTACHED_MASK) == 0X00)
					{
						KVM_Jump_Next_Power_Port(KVM_CurrentHost);	
					}	
				}
				else
				{
					KVM_Flash.cSystemFlag1 &= ~SYSTEM_PLUGIN_JUMP_MODE;
				}
				system_config_changed = 1;
				break;
#endif //#if (ENABLE_HOTKEY_FUN_PLUGIN_JUMP_SW)
		//-----------------------------------------------------------------------	
		// MSC Devcie Control
		//-----------------------------------------------------------------------
#if (SYSTEM_EXTENDER_MSC_SUPPORT)
#if (ENABLE_HOTKEY_T_MSC_SW)
			case HOTKEY_FUN_T_MSC_SW:				
				if (USBHC_MSC_Device_Cnt) // if msc has plug in					
				{									
					ExtenderR_Transmitter_MscSw_Control((U8_T)HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex]);	
				}	
				break;
#endif	/* #if (EXTENDER_USB_T_MSC_SW) */	

#if (ENABLE_HOTKEY_FUN_MSC_CONTROL && SYSTEM_MSC_MAX != 0) //if system suport the msc device
			case HOTKEY_FUN_MSC_CONTRL:
				if (HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex])
				{
					KVM_Flash.cSystemFlag1 &= ~SYSTEM_MSC_NOSUPPORT_MASK;
				}
				else
				{
					KVM_Flash.cSystemFlag1 |= SYSTEM_MSC_NOSUPPORT_MASK;
				}
				system_config_changed = 1;				
				break;
#endif
#endif //#if (SYSTEM_EXTENDER_MSC_SUPPORT)

#if (ENABLE_HOTKEY_FUN_MSC_SW)
			case HOTKEY_FUN_MSC_SW:
				result = HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex] - 1;
				if (KVM_CurrentMSC != result) // changed active port id
				{						
					USBHC_MSC_Port_Switch(USBHC_MSC_Devinx, result);
				}				
				break;
#endif //#if (ENABLE_HOTKEY_FUN_MSC_SW)

		
		//-----------------------------------------------------------------------
		// Power Saving Mode Control
		//-----------------------------------------------------------------------
#if (ENABLE_HOTKEY_FUN_PWSAVING_CONTROL)
			case HOTKEY_FUN_POWER_SAVING_CONTROL:
				if (HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex])
				{	
					KVM_Flash.cSystemFlag1 |= SYSTEM_POWERSAVING_MASK;
				}
				else
				{
					KVM_Flash.cSystemFlag1 &= ~SYSTEM_POWERSAVING_MASK;
				}
				//printf("Power Saving Mode=%s\n\r",(KVM_Flash.cSystemFlag1 & SYSTEM_POWERSAVING_MASK) ? "Y" : "N");
				system_config_changed = 1;		
				break;
#endif
		
		//-----------------------------------------------------------------------
		// Pen Drive Control
		//-----------------------------------------------------------------------
#if (ENABLE_HOTKEY_FUN_PEN_SW)
			case HOTKEY_FUN_PEN_SW:
#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
				if ((KVM_Flash.cSystemFlag1 & SYSTEM_PEN_DRIVE_ATTACH_MASK) == 0x00)
				{
					break; // not vaild port number
				}
				result = HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex] - 1;
				USBDC_VMSC_Port_Switch(result);
#endif /* (SYSTEM_USB_PEN_DRIVE_SUPPORT) */
				break;
#endif // #if (ENABLE_HOTKEY_FUN_PEN_SW)

#if (ENABLE_HOTKEY_FUN_PEN_CONTROL)
			case HOTKEY_FUN_PEN_CONTROL:
#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
				if (HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex] == 0) // USB PEN Drive is Disattached
				{
					USBDC_VMSC_Stop();
				}
				else if (HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex] == 1) // USB PEN Drive is attached
				{
					//2013-12-19 ----------------------------------------------------------
					//Avoid more than one device active
					if (KVM_Flash.cSystemFlag1 & SYSTEM_PEN_DRIVE_ATTACH_MASK)
					{
#if (KVM_BUZZER_SUPPORT)
						BUZZER_Script_Active(Error_Sound);
#endif //#if (KVM_BUZZER_SUPPORT)
					}					
					else
					{
						KVM_CurrentPEN = KVM_CurrentHost;
						USBDC_VMSC_Start();
					}
				}
#endif // #if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
				break;
#endif // #if (ENABLE_HOTKEY_FUN_PEN_CONTROL)
		
		//-----------------------------------------------------------------------
		// Flash Storage Control
		//-----------------------------------------------------------------------
#if (ENABLE_HOTKEY_FUN_FLASH_DEFAULT)
			case HOTKEY_FUN_FLASH_DEFAULT:
#if (SYSTEM_STORAGE_DEVICE_SUPPORT)
				STORAGE_Default_Initial();
				system_config_changed = 1;
#endif //#if (SYSTEM_STORAGE_DEVICE_SUPPORT)
				break;
#endif //#if (ENABLE_HOTKEY_FUN_FLASH_DEFAULT)
		
		//-----------------------------------------------------------------------
		// Control EDID Control
		//-----------------------------------------------------------------------
#if (SYSETM_CONSOLE_EDID_CONTROL_ENABLE)
#if (ENABLE_HOTKEY_FUN_EDID_REREAD)
			case HOTKEY_FUN_EDID_REREAD:
				//printf("EDID Reread\n\r");
				//EDID_ctrl.EdidReloadRequest++;
				break;
#endif //(ENABLE_HOTKEY_FUN_EDID_REREAD)
#endif //(SYSETM_CONSOLE_EDID_CONTROL_ENABLE)
		
		//-----------------------------------------------------------------------
		// Port Switching Control
		//-----------------------------------------------------------------------
#if (ENABLE_HOTKEY_FUN_RAPID_SW)
			case HOTKEY_FUN_RAPID_SW:
				KVM_Jump_Next_Power_Port(KVM_CurrentHost);
				break;
#endif //#if (ENABLE_HOTKEY_FUN_RAPID_SW)

			case HOTKEY_FUN_UPDATE_FIRMWARE:							
				FirmwareUpgradeFlag[0] = 'i';
				FirmwareUpgradeFlag[1] = 'a';
				FirmwareUpgradeFlag[2] = 'p';				
				CPU_SoftReboot();
				break;
				
#if  (SYSTEM_EXTENDER_RECEIVER)
#if (ENABLE_HOTKEY_FUN_T_PORT_SW)
			case HOTKEY_FUN_TRANSMITTER_PORT_SW:				
				ExtenderR_Transmitter_Port_Switch((U8_T)HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex]);				
				break; 
#endif	
#endif /* SYSTEM_EXTENDER_RECEIVER */

#if  (SYSTEM_EXTENDER_RECEIVER)
#if (ENABLE_HOTKEY_FUN_T_BUZZER_CONTROL)
			case HOTKEY_FUN_TX_BUZZER_CONTROL:				
				ExtenderR_Transmitter_Buzzer_Control((U8_T)HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex]);				
				break; 				
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */

#if (ENABLE_HOTKEY_T_AUDIO_AUTO_SW)
			case HOTKEY_FUN_T_USBAUDIO_AUTOSW:
				ExtenderR_Transmitter_AudioComboSw_Control((U8_T)HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex]);				
				break;
#endif /* #if (ENABLE_HOTKEY_T_AUDIO_AUTO_SW) */

#if (ENABLE_HOTKEY_T_USBAUDIO_SW)
			case HOTKEY_FUN_T_USBAUDIO_SW:
				if ((KVM_Flash.cSystemFlag2 & SYSTEM_TX_AUDIOCOMBO_MASK) == 0x00) //if transmitter is not is auto sw mode
				{
					ExtenderR_Transmitter_AudioSw_Control((U8_T)HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex]);	
				}	
				break;
#endif	/* #if (ENABLE_HOTKEY_T_USBAUDIO_SW) */			
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */

#if  (SYSTEM_EXTENDER_RECEIVER)
#if (EXTENDER_ISL59920_SUPPORT)
			case HOTKEY_FUN_R_DELAY_PLUS:
				ISL59920_Color_Delay_Setting(R_COLOR,OPERATION_PLUS);								
				break;
			case HOTKEY_FUN_G_DELAY_PLUS:
				ISL59920_Color_Delay_Setting(G_COLOR,OPERATION_PLUS);												
				break;
			case HOTKEY_FUN_B_DELAY_PLUS:
				ISL59920_Color_Delay_Setting(B_COLOR,OPERATION_PLUS);																
				break;
			case HOTKEY_FUN_R_DELAY_MINUS:
				ISL59920_Color_Delay_Setting(R_COLOR,OPERATION_MINUS);																				
				break;			
			case HOTKEY_FUN_G_DELAY_MINUS:
				ISL59920_Color_Delay_Setting(G_COLOR,OPERATION_MINUS);																								
				break;			
			case HOTKEY_FUN_B_DELAY_MINUS:
				ISL59920_Color_Delay_Setting(B_COLOR,OPERATION_MINUS);								
				break;					
#endif  /* #if (EXTENDER_ISL59920_SUPPORT) */	
			
#if (ENABLE_HOTKEY_FUN_RTC_CONTROL)
			case HOTKEY_FUN_RTC_CONTROL:
				if (HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex])
				{	
					KVM_Flash.cSystemFlag2 |= SYSTEM_RTC_MASK;
				}	
				else
				{	
					KVM_Flash.cSystemFlag2 &= ~SYSTEM_RTC_MASK;
				}
				system_config_changed = 1;
				ExtenderR_Transmit_RTC_Contorl();
				break;
#endif
			
#endif  /* #if (SYSTEM_EXTENDER_RECEIVER) */	
#ifdef SYNC
			case HOTKEY_FUN_ALL_SYNC:
			case HOTKEY_FUN_KB_SYNC:
			case HOTKEY_FUN_MS_SYNC:
				// system_config_changed = KM_SYNC_Setting(HOTKEY_Funciton_Code,HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex]);								
				break;					
#endif
			
#ifdef MSWITCH //Mouse switch mode			
			case HOTKEY_FUN_MS_SWITCH:
				if (HOTKEY_Funciton_PerformValue[HOTKEY_Funciton_PerformValueIndex])
				{	
					KVM_Flash.cSystemFlag3 |= SYSTEM_MOUSE_SW_MASK;
				}	
				else
				{	
					KVM_Flash.cSystemFlag3 &= ~SYSTEM_MOUSE_SW_MASK;
				}
				system_config_changed = 1;
				break;
#endif			
			default:
//				printf("[XXX]");
				break;
		}
#endif /* #if (KVM_HOTKEY_SUPPORT) */

#if (SYSTEM_STORAGE_DEVICE_SUPPORT)
		if (system_config_changed)
		{
			STORAGE_Write(sizeof(KVM_Flash),(U8_T *)&KVM_Flash);
		}
#endif //#if (SYSTEM_STORAGE_DEVICE_SUPPORT)		
		KVM_Event_Control_Flag = 0;
	}
}

/*
 *--------------------------------------------------------------------------------
 * void KVM_Check_KB_Skip_Condition(void)
 * Purpose : Handle the hotkey mode deactive control
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
U8_T KVM_Check_KB_Skip_Condition(void)
{
#if (PROJECT_KVM_CONSOLE_ENABLE)
	/*1.Check Autoscan Status */
	if (KVM_Control_Status & KVM_AUTOSCAN_MASK)
		return 1;
	
	/*2.Check Hotkey Status Status */
#if (KVM_HOTKEY_SUPPORT)
	if (HOTKEY_Control_Status & KVM_HOTKEY_ACTIVE) /* hotkey actived */
	{
		if(HOTKEY_Control_Status_Last & KVM_HOTKEY_ACTIVE)
			return 1;
		else
			HOTKEY_Control_Status_Last |= KVM_HOTKEY_ACTIVE;
	}
#endif /* #if (KVM_HOTKEY_SUPPORT) */
#endif /* #if (PROJECT_KVM_CONSOLE_ENABLE) */
	return 0;
}

/*
 *--------------------------------------------------------------------------------
 * void KVM_Check_MS_Skip_Condition(void)
 * Purpose : Handle the hotkey mode deactive control
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
U8_T KVM_Check_MS_Skip_Condition(void)
{
#if (PROJECT_KVM_CONSOLE_ENABLE)
	/*1.Check Autoscan Status */
	if (KVM_Control_Status & KVM_AUTOSCAN_MASK)
		return 1;
#endif /* #if (PROJECT_KVM_CONSOLE_ENABLE) */

	//if (LIB_Get_Bit_Status(&KVM_Mouse_Disable_Flag,KVM_CurrentHost) == 0x01)
	//	return 1;

	return 0;
}

/*
 *--------------------------------------------------------------------------------
 * void KVM_CONSOLE_Vga_Control(U8_T port,U8_T led_control)
 * Purpose : Control the current VGA Port
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void KVM_CONSOLE_Vga_Control(U8_T port,U8_T vga_control)
{
#ifdef KMLOG
	vga_control = port = 0;
#else	
	#ifdef KM104U	
		port = 0;
		vga_control = 0;
	#else	
		#ifdef KMLOG
		port = 0;
		vga_control = 0;
		
		#else //none KMLOG
		#ifdef MCU_TYPE_AX68002		
			GPIO_SetOneBit(VGA_CONTROL_PORT[0],GPIO_Pin_4,0);
			GPIO_SetOneBit(VGA_CONTROL_PORT[1],GPIO_Pin_5,0);  
		#else
			U8_T	index;
			
			//1.Clear All the VGA port
			for (index = 0; index < KVM_MAX_PORT; index++)
			{
				GPIO_SetOneBit(VGA_CONTROL_PORT[index],VGA_CONTROL_PIN[index],1);
			}
		#endif
			
			//2.Enable the target VGA port
			if (vga_control == VGA_ON)
			{
				TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_KVM_VGA_Switch_ID,0,port,50,15);  //switch vga port to on after 50 ms
				kvmVgaSwitchCnt++;
			}
		#endif	
	#endif	/* #ifdef KM104U */
#endif	/* #ifdef KMLOG */	
}

/*
 *--------------------------------------------------------------------------------
 * void KVM_CONSOLE_Keyboard_Led_Control(void)
 * Purpose : Handle the hotkey mode deactive control
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void KVM_CONSOLE_Keyboard_Led_Control(U8_T kbled)
{
#if (SYSTEM_PS2_HOST_ENABLE)
	U8_T	led,dindex;
	
	/* Handle PS/2 device led command */
	for (dindex=0; dindex < PS2_PORT_MAX_NUM ; dindex++)
	{
		if (PS2_Control[dindex] == PS2_KB)
		{
			led = ((KVM_Hostkey_LED_PS2_Convert(kbled)) << 4) | dindex;
			TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_KVM_CONSOLE_PS2_KBLed_Control_ID,0,led,1,1); //after 100ms, start power on all port
		}
	}
#endif

	USBHC_KB_Led_Brocast(kbled); // this will boardcast the led to USB HC
}

/*--------------------------------------------------------------------------------
 * void TASK_KVM_CONSOLE_PS2_KBLed_Control(void)
 * Purpose : Handle the hotkey mode deactive control
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------
 */
#if (SYSTEM_PS2_HOST_ENABLE)
void TASK_KVM_CONSOLE_PS2_KBLed_Control(void)
{
	U8_T	ps2inx;
	U8_T	led;

	if (TASK_KVM_CONSOLE_PS2_KBLed_Control_Flag == 0)
	{
		led = (TASK_Register0 & 0xf0) >> 4; // get the target ps2 port
		ps2inx = TASK_Register0 & 0x0f; // get the target ps2 port

		if (PS2_KB_Balance_Cnt[ps2inx] == 0)
		{
			TASK_KVM_CONSOLE_PS2_KBLed_Control_Flag=1;
			PS2_Keyboad_LED_Script(ps2inx,led);
			TASK_Destory_Current();
		}
	}
}
#endif

/*--------------------------------------------------------------------------------
 * void TASK_KVM_Switch_Event_Control_Fork(U8_T devinx,U8_T interval)
 * Purpose : Reset the Bulk Handle status
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------s*/
void TASK_KVM_Switch_Event_Control_Fork(U8_T next_port,U8_T interval)
{
	TASK_KVM_Switch_Event_Control_Task.Task_Para = next_port;
	TASK_KVM_Switch_Event_Control_Task.Task_Interval.w = interval;
	TASK_Run(&TASK_KVM_Switch_Event_Control_Task);
}

/*--------------------------------------------------------------------------------
 * void TASK_KVM_VGA_Switch_Control(void)
 * Purpose : The task to switch on the vga port
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------
 */
void TASK_KVM_VGA_Switch_Control(void)
{
#ifdef MCU_TYPE_AX68002
	#ifdef PCT_MUA22
	if (TASK_Register0)
	{
		GPIO_SetOneBit(VGA_CONTROL_PORT[TASK_Register0], GPIO_Pin_4, 1);
		GPIO_SetOneBit(VGA_CONTROL_PORT[TASK_Register0], GPIO_Pin_5, 0);
	}
	else
	{
		GPIO_SetOneBit(VGA_CONTROL_PORT[TASK_Register0], GPIO_Pin_4, 0);
		GPIO_SetOneBit(VGA_CONTROL_PORT[TASK_Register0], GPIO_Pin_5, 1);		
	}
	#else	  
    #if (KVM_2_PORT_HDMI)
	if (TASK_Register0)
	{
		GPIO_SetOneBit(VGA_CONTROL_PORT[TASK_Register0], GPIO_Pin_4, 0);
		GPIO_SetOneBit(VGA_CONTROL_PORT[TASK_Register0], GPIO_Pin_5, 1);
		GPIO_SetOneBit(VGA_CONTROL_PORT[TASK_Register0], GPIO_Pin_6, 0);
		GPIO_SetOneBit(VGA_CONTROL_PORT[TASK_Register0], GPIO_Pin_7, 0);
	}
	else
	{
		GPIO_SetOneBit(VGA_CONTROL_PORT[TASK_Register0], GPIO_Pin_4, 1);
		GPIO_SetOneBit(VGA_CONTROL_PORT[TASK_Register0], GPIO_Pin_5, 1);
		GPIO_SetOneBit(VGA_CONTROL_PORT[TASK_Register0], GPIO_Pin_6, 0);
		GPIO_SetOneBit(VGA_CONTROL_PORT[TASK_Register0], GPIO_Pin_7, 0);
	}
	#else
	if (TASK_Register0)
	{
		GPIO_SetOneBit(VGA_CONTROL_PORT[TASK_Register0], GPIO_Pin_4, 0);
		GPIO_SetOneBit(VGA_CONTROL_PORT[TASK_Register0], GPIO_Pin_5, 1);
	}
	else
	{
		GPIO_SetOneBit(VGA_CONTROL_PORT[TASK_Register0], GPIO_Pin_4, 1);
		GPIO_SetOneBit(VGA_CONTROL_PORT[TASK_Register0], GPIO_Pin_5, 0);
	}
	#endif
    #endif
#else	
	GPIO_SetOneBit(VGA_CONTROL_PORT[TASK_Register0],VGA_CONTROL_PIN[TASK_Register0],0);
#endif
   	TASK_Destory_Current();
}		

/*--------------------------------------------------------------------------------
 * void KVM_Port_Jump(U8_T newport)
 * Purpose : This function will jump the kvm port 
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------*/
void KVM_Port_Jump(U8_T newport)
{
 	if (KVM_CurrentHost != newport)
	{
		KVM_Control_Status |= KVM_PORT_SW_MASK; 
		// Prevent PS/2 stick after port switch */
#if (SYSTEM_PS2_HOST_ENABLE)
		KVM_PS2_HotkeySwitch_Flag = 1; //it is hotkey switch
#endif
#if (PROJECT_USB_GENERIC_HID_ENABLE)
		#ifndef SYNC		
		DATAST_Generic_KB_IN_WP = DATAST_Generic_KB_IN_RP = 0;
		#endif
#endif /* #if (PROJECT_USB_GENERIC_HID_ENABLE) */
		KVM_NextHost = newport;
		TASK_KVM_Switch_Event_Control_Fork(KVM_NextHost,KVM_SW_INTERVAL_TIME);
#if (!SYSTEM_SWITCH_AUDIO_DELAY)		
		KVM_Audio_Combo_Switch_Check(KVM_NextHost);
#endif		
	}
}

/*--------------------------------------------------------------------------------
 * void KVM_KM_Port_Jump(U8_T newport)
 * Purpose : This function will jump the km(keyboard and mouse) port 
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------*/
void KVM_KM_Port_Jump(U8_T newport)
{
 	if (KVM_CurrentHost != newport)
	{
		KVM_Control_Status |= KVM_PORT_SW_MASK; 
		// Prevent PS/2 stick after port switch */
#if (SYSTEM_PS2_HOST_ENABLE)
		KVM_PS2_HotkeySwitch_Flag = 1; //it is hotkey switch
#endif
		
#if (PROJECT_USB_GENERIC_HID_ENABLE)
		#ifndef SYNC
		DATAST_Generic_KB_IN_WP = DATAST_Generic_KB_IN_RP = 0;
		#endif
#endif /* #if (PROJECT_USB_GENERIC_HID_ENABLE) */
		KVM_NextHost = newport;
		TASK_KVM_Switch_Event_Control_Fork(KVM_NextHost,KVM_SW_INTERVAL_TIME);
#if (!SYSTEM_SWITCH_AUDIO_DELAY)		
		KVM_Audio_Combo_Switch_Check(KVM_NextHost);
#endif		
	}
}

/*--------------------------------------------------------------------------------
 * void KVM_PlugIn_Jump_Check(U8_T pid)
 * Purpose : This function will check the PlugIn Switch flag, and decide jump 
 *           or stay
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------*/
void KVM_PlugIn_Jump_Check(U8_T pid) 
{
	if (KVM_Flash.cSystemFlag1 & SYSTEM_PLUGIN_JUMP_MASK)
	{
       	//Check plug in jump mode
		if ((KVM_Flash.cSystemFlag1 & SYSTEM_PLUGIN_JUMP_MODE) == 0)  //only jump one time to the first plug in
		{
			if (KVM_Plug_In_Jump_Flag)
			{
				return;
			}
		}
		KVM_Plug_In_Jump_Flag = 1;
		KVM_Port_Jump(pid);
	}
}	

/*--------------------------------------------------------------------------------
 * U8_T KVM_Power_Status_Check(U8_T start_pid,U8_T end_pid)
 * Purpose : This function will check the port vbus status
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------*/
U8_T KVM_Power_Status_Check(U8_T start_pid,U8_T end_pid)
{	
	while (start_pid < end_pid)
	{
		if (USBDC_UpPortState[start_pid] & USBDC_ROOTHUB_ATTACHED_MASK)
		{
			return (start_pid + 1);
		}
		start_pid++;
	}
	return 0;
}

/*--------------------------------------------------------------------------------
 * U8_T KVM_Jump_Next_Power_Port(U8_T pid)
 * Purpose : This function will check the port vbus status
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------*/
void KVM_Jump_Next_Power_Port(U8_T pid)
{	
	U8_T newport;
	
	if (KVM_Flash.cSystemFlag0 & SYSTEM_JUMP_MODE_MASK) //jump by power
	{	
		newport = KVM_Power_Status_Check(pid+1,KVM_MAX_PORT);
		if (newport) //check backward
		{
			KVM_Port_Jump(newport-1);
		}							
		else
		{
			newport = KVM_Power_Status_Check(0,pid); //check forward
			if (newport)
			{
				KVM_Port_Jump(newport-1);						
			}
		}
	}
	else
	{
		KVM_NextHost = pid+1;
		if (KVM_NextHost >= KVM_MAX_PORT)
			KVM_NextHost = 0;				
		KVM_Port_Jump(KVM_NextHost);					
	}		
}

/*--------------------------------------------------------------------------------
 * U8_T KVM_Power_Jump_Check(U8_T pid)
 * Purpose : This function will check the Power Jump Switch flag, and decide jump 
 * 			 or stay
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------*/
U8_T KVM_Power_Jump_Check(U8_T pid,U8_T checknext) 
{
	U8_T port_attached;
		
	if (KVM_Flash.cSystemFlag0 & SYSTEM_JUMP_MODE_MASK) //jump by power
	{			
		port_attached = USBDC_UpPortState[pid] & USBDC_ROOTHUB_ATTACHED_MASK;		
		if (checknext)
		{	
			if ((port_attached == 0x00) && (pid == KVM_CurrentHost))
			{
				KVM_Jump_Next_Power_Port(pid);												
			}
	    }
	    else
	    {
	    	if (port_attached == 0)
	    	{
#if (KVM_BUZZER_SUPPORT)
				BUZZER_Script_Active(Error_Sound);
#endif //#if (KVM_BUZZER_SUPPORT) 	
	    	}		    		
	    	return port_attached;
	    }	
		return 0;
	}
	else
	{
		return 1;
	}			
}	

/*--------------------------------------------------------------------------------
 * void KVM_Audio_Combo_Switch_Check(void)
 * Purpose : This function will check the Audio Combo Switch condition
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------*/
void KVM_Audio_Combo_Switch_Check(U8_T newport)
{
	if (KVM_Flash.cSystemFlag1 & SYSTEM_AUDIOCOMBO_MASK) //if audio combo switch has been setting
	{
		if (KVM_CurrentUSBAudio != newport)
		{
			KVM_NextAudio = newport;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) 
			USBHC_AUDIO_Switch(newport);
#endif
		}
	
		if (KVM_CurrentAudio != newport)		
		{		
			KVM_NextAudio = newport;
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
			KVM_Audio_Switch(newport);
#endif			
		}
	}
}

/*--------------------------------------------------------------------------------
 * void KVM_AutoscanStop_Jump(U8_T jumpflag,U8_T newport)
 * Purpose : Control the port jump operation during autoscan period
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------*/
void KVM_AutoscanStop_Jump(U8_T jumpflag,U8_T newport)
{
	if (jumpflag && (KVM_CurrentHost != newport))
	{	
		KVM_Port_Jump(newport);		
	}
}

#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
/*--------------------------------------------------------------------------------
 * void KVM_AnalogAudioControl(U8_T newport)
 * Purpose : This function will switch the analog audio by gpio pin
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------*/
void KVM_AnalogAudioControl(U8_T newport)
{
	/* Need Add code here */
	
	newport = 0;
}

/*--------------------------------------------------------------------------------
 * void KVM_Audio_Switch(U8_T newport)
 * Purpose : This function will switch the analog audio 
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------*/
void KVM_Audio_Switch(U8_T newport)
{
	if (KVM_NextAnalogAudio != newport)
	{
		KVM_NextAnalogAudio = newport;		
		KVM_CurrentAudio = newport;		 
		KVM_AnalogAudioControl(KVM_NextAnalogAudio);		
	}
}
#endif //#if (SYSTEM_AUDIO_DEVICE_SUPPORT)




#if (KVM_BUTTON_SUPPORT) && (KVM_BUTTON_RELEASE_DELAY_INTERVAL)	
/*--------------------------------------------------------------------------------
 * void TASK_KVM_Button_Release_Delay(void)
 * Purpose : 
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------*/
void TASK_KVM_Button_Release_Delay(void)
{
	BTN_Processing_Flag = 0;	
	TASK_Destory_Current();
}
#endif /* #if (KVM_BUTTON_SUPPORT) && (KVM_BUTTON_RELEASE_DELAY_INTERVAL)		*/

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) && (SYSTEM_SWITCH_AUDIO_DELAY)
/*
 *--------------------------------------------------------------------------------
 * void KVM_CONSOLE_Switch_Port_Audio_Control(U8_T newport)
 * Purpose : 
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */	 
void KVM_CONSOLE_Switch_Port_Audio_Control(U8_T oldport)
{
	//printf("+-+->  SW_AUDIO: OLD=%bu,",oldport);
	if (KVM_Console_Switch_Audio_Delay_Task_ActiveID == 0)
	{
		//printf("+-+->  SW_AUDIO: Oringial OLD=%bu,",oldport);
		KVM_Console_Switch_Audio_Delay_Task_ActiveID = 
			TASK_Active(TASK_TYPE_INTERVAL_MS,KVM_Console_Switch_Audio_Delay_Task_ID,0,oldport,SWITCH_PORT_DELAY_AUDIO_CNT,SWITCH_PORT_DELAY_AUDIO_CNT);									
		//printf("NEW\n\r");		
	}
	else
	{
		Task_Active_Table[KVM_Console_Switch_Audio_Delay_Task_ActiveID].Task_Interval.w = SWITCH_PORT_DELAY_AUDIO_CNT;			
		//printf("+-+->  SW_AUDIO: OLD=%bu\n\r",oldport);
	}	
}

/*
 *--------------------------------------------------------------------------------
 * void TASK_KVM_CONSOLE_Switch_Port_Audio_Control(void)
 * Purpose : 
 * Params  : NONE
 * Returns : None.
 * Note    : None.22
 *--------------------------------------------------------------------------------
 */
void TASK_KVM_CONSOLE_Switch_Port_Audio_Control(void)
{	
	//printf("+-+->  SW_AUDIO: DONE: Current=%bu,Old=%bu\n\r",KVM_CurrentHost,KVM_CurrentUSBAudio);
	USBHC_AUDIO_Switch(KVM_CurrentHost);
	//KVM_Audio_Combo_Switch_Check(KVM_CurrentHost);	
	TASK_Destory_Current();
	KVM_Console_Switch_Audio_Delay_Task_ActiveID = 0;
}
#endif /* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) && (SYSTEM_SWITCH_AUDIO_DELAY) */

/**--------------------------------------------------------------------------------
 * U8_T KVM_Console_Port_Jump(U8_T new_port)
 * Purpose : 
 * Params  : 
 *           
 *           
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------*/
U8_T KVM_Console_Port_Jump(U8_T new_port)
{	

	printf("CurrentHost=%bu, JumpPort=%bu\r\n", KVM_CurrentHost, new_port);
	if ((KVM_Control_Status & KVM_PORT_SW_MASK) == 0)
	{					
		if (KVM_CurrentHost != new_port ) // changed active port id
		{						
			if (KVM_Power_Jump_Check(new_port,0)) 
			{					
				KVM_Port_Jump(new_port);		
				MouseY = Y_MAX/2;
				MouseX = X_MAX/2;	
				return 1;
			}
		}
	}
	return 0;
}

/**--------------------------------------------------------------------------------
 * void KVM_CONSOLE_IAP_Mode(void)
 * Purpose : 
 * Params  : 
 *           
 *           
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------*/
void KVM_CONSOLE_IAP_Mode(void)
{		
	FirmwareUpgradeFlag[0] = 'i';
	FirmwareUpgradeFlag[1] = 'a';
	FirmwareUpgradeFlag[2] = 'p';						
	CPU_SoftReboot();		
}	

/**--------------------------------------------------------------------------------
 * void KVM_System_Config_Write(void)
 * Purpose : 
 * Params  : 
 *           
 *           
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------*/
void KVM_System_Config_Write(void)
{
#if (SYSTEM_STORAGE_DEVICE_SUPPORT)	
	printf("flash write\n\r");
	STORAGE_Write(sizeof(KVM_Flash),(U8_T *)&KVM_Flash);
#endif	
}
#endif /* End of PROJECT_KVM_CONSOLE_ENABLE */


/* End of kvm_console.c */
