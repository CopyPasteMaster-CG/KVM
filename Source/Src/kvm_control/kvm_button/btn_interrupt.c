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
 * Module Name: btn_interrupt.c
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
#include "project_include.h"

extern U8_T  API_Current_Main_SCREEN;
#if (KVM_BUTTON_SUPPORT) && (KVM_BUTTON_HARDWARE)
/* NAMING CONSTANT DECLARATIONS */
#if (KVM_MULITY_BUTTON_TYPE)		
	#if (MCU_REAL_CHIP)
		#ifdef MCU_TYPE_AX68002
			const U8_T				BTN_PORT_SFR[] = {MISC_GP0TR, MISC_GP0TR,};
			const GPIO_TypeDef		*BTN_POLLING_PORT[] = {PORT0,PORT0,};
		#else
			const U8_T				BTN_PORT_SFR[] = {MISC_GP0TR, MISC_GP0TR, MISC_GP0TR, MISC_GP0TR,};
			const GPIO_TypeDef		*BTN_POLLING_PORT[] = {PORT0,PORT0,PORT0,PORT0,};
		#endif /* #ifdef MCU_TYPE_AX68002 */
			
		#if (PCB_TEST_BOARD)
			#ifdef MCU_TYPE_AX68002
				const GPIO_PinTypeDef	BTN_POLLING_PIN[] = {GPIO_Pin_0,GPIO_Pin_1,};
				const GPIO_PinTypeDef	BTN_POLLING_SHIFT[] = {0,2,4,6,};
			#else
				const GPIO_PinTypeDef	BTN_POLLING_PIN[] = {GPIO_Pin_0,GPIO_Pin_2,GPIO_Pin_1,GPIO_Pin_3,};
				const GPIO_PinTypeDef	BTN_POLLING_SHIFT[] = {0,4,2,6,};
			#endif
		#else
			const GPIO_PinTypeDef	BTN_POLLING_PIN[] = {GPIO_Pin_0,GPIO_Pin_1,GPIO_Pin_2,GPIO_Pin_3,};
			const GPIO_PinTypeDef	BTN_POLLING_SHIFT[] = {0,2,4,6,};
		#endif	
	#else
		const U8_T				BTN_PORT_SFR[] = {MISC_GP0TR, MISC_GP0TR, MISC_GP0TR, MISC_GP0TR,};	
		const GPIO_TypeDef		*BTN_POLLING_PORT[] = {PORT0,PORT0,PORT0,PORT0,};	
		const GPIO_PinTypeDef	BTN_POLLING_PIN[] = {GPIO_Pin_4,GPIO_Pin_5,GPIO_Pin_6,GPIO_Pin_7,};
		const GPIO_PinTypeDef	BTN_POLLING_SHIFT[] = {0,2,4,6,};
	#endif
#else
	const U8_T				BTN_PORT_SFR[] = {MISC_GP0TR};	
	const GPIO_TypeDef		*BTN_POLLING_PORT[] = {PORT0};
    const GPIO_PinTypeDef	BTN_POLLING_PIN[] = {GPIO_Pin_0};	
    const GPIO_PinTypeDef	BTN_POLLING_SHIFT[] = {0,2,4,6,};
#endif //#if (KVM_MULITY_BUTTON_TYPE)


/* GLOBAL VARIABLES DECLARATIONS */
U8_T	TASK_BTN_Polling_ID,TASK_BTN_Polling_Active_ID;
U8_T	TASK_BTN_Event_ID,TASK_BTN_Event_Active_ID;
bit		BTN_Processing_Flag;
extern U8_T	TASK_USBHC_Resume_Command_ID;
#ifdef BTN_RESET
U8_T TASK_BTN_Reset_ID;
U8_T TASK_BTN_Reset_ActiveID;
void TASK_BTN_Reset(void);
#endif
/* LOCAL VARIABLES DECLARATIONS */
/* LOCAL SUBPROGRAM DECLARATIONS */
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
/* EXTERNAL SUBPROGRAM DECLARATIONS */

/*
 * ----------------------------------------------------------------------------
 * Function Name: KVM_BTN_Init(void)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void KVM_BTN_Init(void)
{

	U8_T index,regvalue1;
	U8_T interrupt5;
	U8_T trigger,regvalue0;

	BTN_Processing_Flag = 0;

	EXTINT4_DISABLE;
	interrupt5 = EINT5; // keep the orginal value
	EXTINT5_DISABLE;
	
#if (KVM_BUTTON_HARDWARE)	
#if (KVM_BUTTON_WAKEUP)// Allow Button to wakeup Sleep Vinson
	#if (KVM_MULITY_BUTTON_TYPE)
		#if (MCU_REAL_CHIP)
			#ifdef MCU_TYPE_AX68002
			_MISC_DR_SFR(0x03); //alow pin 0~1 to wakeup
			#else
			_MISC_DR_SFR(0x0f); //alow pin 0~3 to wakeup
			#endif
		#else
			_MISC_DR_SFR(0xf0); //alow pin 4~7 to wakeup
		#endif /* #if (MCU_REAL_CHIP) */
	#else
		_MISC_DR_SFR(0x01); //alow pin 0 to wakeup
	#endif /* #if (KVM_MULITY_BUTTON_TYPE) */
	
	_MISC_DR_SFR(0x00);
	_MISC_CIR_SFR(MISC_GPWER);
#endif //#if (KVM_BUTTON_WAKEUP)
#endif /* KVM_BUTTON_HARDWARE  */

#if (KVM_MULITY_BUTTON_TYPE)	
	for (index=0;index < KVM_MAX_PORT ; index++)
#else
	index = 0;	
#endif	
	{
		GPIO_SetOneBit(BTN_POLLING_PORT[index],BTN_POLLING_PIN[index],1);
		//1.Set Falling edge trigger
		//1-0.Backup old value
		_MISC_CIR_SFR(BTN_PORT_SFR[index]);
		_MISC_DR_READ_SFR(regvalue0);
		_MISC_DR_READ_SFR(regvalue1);
		
		//2-0.assign new value
		if (BTN_POLLING_PIN[index] < GPIO_Pin_4) // pin0~pin3
		{
			regvalue0 &= ~(BTN_TRIGGER_MASK << BTN_POLLING_SHIFT[index]);
			//trigger = regvalue0 | (BTN_TRIGGER_FALLING << BTN_POLLING_SHIFT[index]);
			trigger = regvalue0 | (BTN_TRIGGER_MASK << BTN_POLLING_SHIFT[index]);	//daniel
			regvalue0 = trigger;
		}
		else
		{
			regvalue1 &= ~(BTN_TRIGGER_MASK << BTN_POLLING_SHIFT[index]);
			trigger = regvalue1 | (BTN_TRIGGER_FALLING << BTN_POLLING_SHIFT[index]);
			regvalue1 = trigger;
		}
		_MISC_DR_SFR(regvalue0);
		_MISC_DR_SFR(regvalue1);
		_MISC_CIR_SFR(BTN_PORT_SFR[index]);
		
#if (KVM_BUTTON_DE_BOUNCE_DISALBE)
		//3.Clear the de-bounce time
		if (BTN_PORT_SFR[index] == MISC_GP0TR)
		{
			de_bounce_sfr = MISC_GP0DER;
		}	
		else
		{
			de_bounce_sfr = MISC_GP2DER;
		}	
		_MISC_CIR_SFR(de_bounce_sfr);
		_MISC_DR_READ_SFR(regvalue0);
		regvalue0 &= ~BTN_POLLING_PIN[index]; //Clear debounce time
		_MISC_DR_SFR(regvalue0);
		_MISC_CIR_SFR(de_bounce_sfr);
#endif		
	}
	//2.Enable INTER4
	EXTINT4_ENABLE;
	EINT5 = interrupt5;

	TASK_BTN_Polling_ID = TASK_Create(TASK_BTN_Polling);

#ifdef BTN_RESET
	TASK_BTN_Reset_ID = TASK_Create(TASK_BTN_Reset);
	TASK_BTN_Reset_ActiveID = 0;
#endif	

#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("TASK_BTN_Polling_ID=%bu\n\r",TASK_BTN_Polling_ID);
#endif	
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: TASK_BTN_Polling(void)
 * Purpose: Detect the Button Pin low and become high(complete cycle)
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void TASK_BTN_Polling(void)
{
	U8_T	btnstate,index=0;
	U8_T    edge_state;
	U8_T 	btn_state = 0;	//zbb
#ifdef KMLOG
	U8_T    KM_Btn_State = 0;
#endif
	btnstate = TASK_Register0;
	edge_state = TASK_Event & 0x0f;

	

#ifdef BTN_RESET			
	//printf("Edge_State:%bu\n\r",edge_state);
	if (edge_state) //Rising Edge 
	{	
		if (TASK_BTN_Reset_ActiveID)
		{
			TASK_Destory_Task(TASK_BTN_Reset_ActiveID-1);
			TASK_BTN_Reset_ActiveID = 0;
		}	
		//return;
	}							
#endif	

	//zbb
	if (BTN_Processing_Flag == 0) // no any button down
	{
		#if 0
		for (index = 0; index < KVM_MAX_PORT; index++)
		{
			if (btnstate & BTN_POLLING_PIN[index]) // button pressed
			{
				btn_state |= BIT_MASK[index];	//zbb
			}
		}
		#endif
		if (GPIO_GetPinValue(PORT0, GPIO_Pin_0))
		{
			btn_state |= 0x01;
		}

		if (GPIO_GetPinValue(PORT0, GPIO_Pin_1))
		{
			btn_state |= 0x02;
		}

		if (GPIO_GetPinValue(PORT0, GPIO_Pin_2))
		{
			btn_state |= 0x04;
		}

		if (GPIO_GetPinValue(PORT0, GPIO_Pin_3))
		{
			btn_state |= 0x08;
		}

		//check the btn_state
		printf("---- btn_state:%bx ----\n\r", btn_state);
		switch(btn_state)
		{
			// case PORT0_ACTIVE:
			// 	API_Set_Roaming_Mode(API_ROAMING_DISABLE);
			// 	API_Set_Sync_Mode(API_SYNC_DISABLE);
			// 	KVM_Console_Port_Jump(3);
			// 	break;
			// case PORT1_ACTIVE:
			// 	API_Set_Roaming_Mode(API_ROAMING_DISABLE);
			// 	API_Set_Sync_Mode(API_SYNC_DISABLE);
			// 	KVM_Console_Port_Jump(2);
			// 	break;
			// case PORT2_ACTIVE:
			// 	API_Set_Roaming_Mode(API_ROAMING_DISABLE);
			// 	API_Set_Sync_Mode(API_SYNC_DISABLE);
			// 	KVM_Console_Port_Jump(1);
			// 	break;
			// case PORT3_ACTIVE:
			// 	API_Set_Roaming_Mode(API_ROAMING_DISABLE);
			// 	API_Set_Sync_Mode(API_SYNC_DISABLE);
			// 	KVM_Console_Port_Jump(0);
			// 	break;
			// case PORT12_ACTIVE:
			// 	printf("PORT1_2 Screen Roaming Mode\n\r");
			// 	API_Set_Roaming_Mode(API_ROAMING_ENABLE);					
			// 	API_Active_Roaming_Mapping(TWO_SCREEN_MODE);
			// 	KVM_Console_Port_Jump(3);
			// 	break;
			// case PORT34_ACTIVE:
			// 	printf("PORT3_4 Screen Roaming Mode\n\r");
			// 	API_Set_Roaming_Mode(API_ROAMING_ENABLE);					
			// 	API_Active_Roaming_Mapping(TWO_SCREEN_MODE);
			// 	KVM_Console_Port_Jump(1);
			// 	break;
			// case PORT_ALL_ACTIVE:
			// 	printf("PORT_ALL Screen Roaming Mode\n\r");
			// 	API_Set_Roaming_Mode(API_ROAMING_ENABLE);					
			// 	API_Active_Roaming_Mapping(QUAD_SCREEN_MODE);
			// 	KVM_Console_Port_Jump(3);
			// 	break;
			// case PORT_ALL_SYNC:
			// 	printf("PORT_ALL Screen SYNC Mode\n\r");
			// 	API_Set_Roaming_Mode(API_ROAMING_DISABLE);					
			// 	API_Active_Roaming_Mapping(QUAD_SCREEN_MODE);
			// 	API_Set_Sync_Mode(API_SYNC_ENABLE);
			// 	break;
			// default:
			// 	break;

			// case PORT0_ACTIVE:				
			// 	API_SET_ONE_3_SCREEN_MODE(0);
			// 	API_Current_Main_SCREEN = 3;
			// 	KVM_Console_Port_Jump(3);
			// 	break;
			// case PORT1_ACTIVE:					
			// 	API_SET_ONE_3_SCREEN_MODE(1);
			// 	API_Current_Main_SCREEN = 2;
			// 	KVM_Console_Port_Jump(2);
			// 	break;
			// case PORT2_ACTIVE:				
			// 	API_SET_ONE_3_SCREEN_MODE(2);
			// 	API_Current_Main_SCREEN = 1;
			// 	KVM_Console_Port_Jump(1);
			// 	break;
			// case PORT3_ACTIVE:				
			// 	API_SET_ONE_3_SCREEN_MODE(3);
			// 	API_Current_Main_SCREEN = 0;
			// 	KVM_Console_Port_Jump(0);
			// 	break;
			// case PORT12_ACTIVE:
			// 	printf("PORT1_2 Screen Roaming Mode\n\r");
			// 	API_Set_Roaming_Mode(API_ROAMING_ENABLE);					
			// 	API_Active_Roaming_Mapping(TWO_SCREEN_MODE);
			// 	KVM_Console_Port_Jump(3);
			// 	break;
			// case PORT34_ACTIVE:
			// 	printf("PORT3_4 Screen Roaming Mode\n\r");
			// 	API_Set_Roaming_Mode(API_ROAMING_ENABLE);					
			// 	API_Active_Roaming_Mapping(TWO_SCREEN_MODE);
			// 	KVM_Console_Port_Jump(1);
			// 	break;
			// case PORT_ALL_ACTIVE:
			// 	printf("PORT_ALL Screen Roaming Mode\n\r");
			// 	API_Set_Roaming_Mode(API_ROAMING_ENABLE);					
			// 	API_Active_Roaming_Mapping(QUAD_SCREEN_MODE);
			// 	KVM_Console_Port_Jump(3);
			// 	break;
			// case PORT_ALL_SYNC:
			// 	printf("PORT_ALL Screen SYNC Mode\n\r");
			// 	API_Set_Roaming_Mode(API_ROAMING_DISABLE);					
			// 	API_Active_Roaming_Mapping(QUAD_SCREEN_MODE);
			// 	API_Set_Sync_Mode(API_SYNC_ENABLE);
			// 	break;
			// default:
			// 	break;




			// case PORT0_ACTIVE:				
			// 	API_SET_ONE_2_SCREEN_MODE(1);
			// 	API_Current_Main_SCREEN = 3;
			// 	KVM_Console_Port_Jump(3);
			// 	break;
			// case PORT1_ACTIVE:					
			// 	API_SET_ONE_2_SCREEN_MODE(2);
			// 	API_Current_Main_SCREEN = 2;
			// 	KVM_Console_Port_Jump(2);
			// 	break;
			// case PORT2_ACTIVE:				
			// 	API_SET_ONE_2_SCREEN_MODE(3);
			// 	API_Current_Main_SCREEN = 1;
			// 	KVM_Console_Port_Jump(1);
			// 	break;
			// case PORT3_ACTIVE:				
			// 	API_SET_ONE_3_SCREEN_MODE(3);
			// 	API_Current_Main_SCREEN = 0;
			// 	KVM_Console_Port_Jump(0);
			// 	break;
			// case PORT12_ACTIVE:
			// 	printf("PORT1_2 Screen Roaming Mode\n\r");
			// 	API_Set_Roaming_Mode(API_ROAMING_ENABLE);					
			// 	API_Active_Roaming_Mapping(TWO_SCREEN_MODE);
			// 	KVM_Console_Port_Jump(3);
			// 	break;
			// case PORT34_ACTIVE:
			// 	printf("PORT3_4 Screen Roaming Mode\n\r");
			// 	API_Set_Roaming_Mode(API_ROAMING_ENABLE);					
			// 	API_Active_Roaming_Mapping(TWO_SCREEN_MODE);
			// 	KVM_Console_Port_Jump(1);
			// 	break;
			// case PORT_ALL_ACTIVE:
			// 	printf("PORT_ALL Screen Roaming Mode\n\r");
			// 	API_Set_Roaming_Mode(API_ROAMING_ENABLE);					
			// 	API_Active_Roaming_Mapping(QUAD_SCREEN_MODE);
			// 	KVM_Console_Port_Jump(3);
			// 	break;
			// case PORT_ALL_SYNC:
			// 	printf("PORT_ALL Screen SYNC Mode\n\r");
			// 	API_Set_Roaming_Mode(API_ROAMING_DISABLE);					
			// 	API_Active_Roaming_Mapping(QUAD_SCREEN_MODE);
			// 	API_Set_Sync_Mode(API_SYNC_ENABLE);
			// 	break;
			// default:
			// 	break;




// 	API_SET_ONE_3_SCREEN_MODE(0);
			// 	API_Current_Main_SCREEN = 3;
			// 	KVM_Console_Port_Jump(3);
			// 	break;
			// case PORT1_ACTIVE:					
			// 	API_SET_ONE_3_SCREEN_MODE(1);
			// 	API_Current_Main_SCREEN = 2;
			// 	KVM_Console_Port_Jump(2);
			// 	break;
			// case PORT2_ACTIVE:				
			// 	API_SET_ONE_3_SCREEN_MODE(2);
			// 	API_Current_Main_SCREEN = 1;
			// 	KVM_Console_Port_Jump(1);
			// 	break;
			// case PORT3_ACTIVE:				
			// 	API_SET_ONE_3_SCREEN_MODE(3);
			// 	API_Current_Main_SCREEN = 0;
			// 	KVM_Console_Port_Jump(0);






			case PORT0_ACTIVE:				
				API_SET_ONE_2_SCREEN_MODE(1);
				API_Current_Main_SCREEN = 3;
				KVM_Console_Port_Jump(3);
				break;
			case PORT1_ACTIVE:					
				API_SET_ONE_2_SCREEN_MODE(2);
				API_Current_Main_SCREEN = 2;
				KVM_Console_Port_Jump(2);
				break;
			case PORT2_ACTIVE:				
				API_SET_ONE_2_SCREEN_MODE(3);
				API_Current_Main_SCREEN = 1;
				KVM_Console_Port_Jump(1);
				break;
			case PORT3_ACTIVE:				
				API_SET_ONE_3_SCREEN_MODE(3);
				API_Current_Main_SCREEN = 0;
				KVM_Console_Port_Jump(0);
				break;
			case PORT12_ACTIVE:
				API_SET_ONE_3_SCREEN_MODE(0);
				API_Current_Main_SCREEN = 3;
				KVM_Console_Port_Jump(3);
				break;
			case PORT34_ACTIVE:
				API_SET_ONE_3_SCREEN_MODE(1);
				API_Current_Main_SCREEN = 2;
				KVM_Console_Port_Jump(2);
				break;
			case PORT_ALL_ACTIVE:
				API_SET_ONE_3_SCREEN_MODE(2);
				API_Current_Main_SCREEN = 1;
				KVM_Console_Port_Jump(1);
				break;
			case PORT_ALL_SYNC:
				API_SET_ONE_3_SCREEN_MODE(3);
				API_Current_Main_SCREEN = 0;
				KVM_Console_Port_Jump(0);
				break;
			default:
				break;




		}

		return;
	}

	
	if (BTN_Processing_Flag == 0) // no any button down
	{
#if (KVM_MULITY_BUTTON_TYPE)
		for (index = 0; index < KVM_MAX_PORT; index++)
#endif
		{
			if (btnstate & BTN_POLLING_PIN[index]) // button pressed
			{
#ifdef KMLOG
				KM_Btn_State |= BIT_MASK[index];
#endif
				
#ifdef BTN_RESET				
				if (TASK_BTN_Reset_ActiveID == 0)
				{			
					TASK_BTN_Reset_ActiveID = 
						TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_BTN_Reset_ID,0,index,BTN_RESET_WAIT_TIME,1)+1;
				}	
				else
				{
					Task_Active_Table[TASK_BTN_Reset_ActiveID-1].Task_Para = index;
					Task_Active_Table[TASK_BTN_Reset_ActiveID-1].Task_Interval.w = BTN_RESET_WAIT_TIME;
				}	
#endif		
			
#if (KVM_CONSOLE_DEBUG_MODE)
				printf("BTN:%bx Pressed\n\r", index);
#endif
				if (KVM_Control_Status & KVM_AUTOSCAN_MASK)
				{
					KVM_AutoscanStop_Jump(1,index);
					KVM_Control_Status &= ~KVM_AUTOSCAN_MASK; // clear the autoscan flag
					Task_Active_Table[TASK_KVM_AUTOSCAN_ACTIVE_ID].Task_Interval.w = 1; //wait for 1 ms
					return; // no more check about button down
				}
				else
				{
#if (!KVM_MULITY_BUTTON_TYPE)
	                index = KVM_CurrentHost + 1;
					if (index >= KVM_MAX_PORT)
                        index = 0;
#endif
#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)					
					if (KVM_CurrentHost == index) // if the port is not active now
					{
						if (PowerSavingStartFlag)
						{
							USBHC_CORE_Resmue();
							TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_Resume_Command_ID,0x00,USBDC_VIRTUAL_DEVINX,0,0); //Generate the task, for next Event check
							PowerSavingStartFlag = 0;
						}
						USBDC_WakeupUpstreamHost(index);
					}
					else
#endif						
					{
						#ifdef SYNC
						KM_SYNC_ModeSync_Control(0);
						#endif
						KVM_Console_Port_Jump(index);						
						return; // no more check about button down
					}
				}
			}
		}
#ifdef KMLOG
		KMLog_Send_Button_State(KM_Btn_State);
#endif		
	}	
}

//#ifdef BTN_RESET
/*
 * ----------------------------------------------------------------------------
 * Function Name: void TASK_BTN_Reset(void)
 * Purpose: Detect the Button Pin low and become high(complete cycle)
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void TASK_BTN_Reset(void)
{
	U8_T btn_id;
	
	
	btn_id = TASK_Register0;
	
	if (GPIO_GetPinValue(BTN_POLLING_PORT[btn_id],BTN_POLLING_PIN[btn_id]) == 0)
	{
		FirmwareUpgradeFlag[0] = 'i';
		FirmwareUpgradeFlag[1] = 'a';
		FirmwareUpgradeFlag[2] = 'p';		
		CPU_SoftReboot();		
	}	
	
	TASK_Destory_Current();
	
}
//#endif

#endif //#if (KVM_BUTTON_HARDWARE)

/* End of btn_interrupt.c */
