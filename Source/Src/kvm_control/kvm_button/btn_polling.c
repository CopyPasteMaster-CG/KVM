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
 * Module Name: btn_polling.c
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

#if (KVM_BUTTON_SUPPORT) && !(KVM_BUTTON_HARDWARE)
/* NAMING CONSTANT DECLARATIONS */
#define  BTN_POLLING_TIMER		35
#define  BTN_RESET_TINMER		150
#define  BTN_MAX_PORT			2


#if (KVM_MULITY_BUTTON_TYPE)		
	#define  BTN_AUTOSCAN_MASK		0x03
	
	//#ifdef MCU_TYPE_AX68004	
	const GPIO_TypeDef		*BTN_POLLING_PORT[] = {PORT1,PORT1};
	const GPIO_PinTypeDef	BTN_POLLING_PIN[] = {GPIO_Pin_4,GPIO_Pin_5};	
	//#endif /* #ifdef MCU_TYPE_AX68004 */		
#else	
	const GPIO_TypeDef		*BTN_POLLING_PORT[] = {PORT1};
    const GPIO_PinTypeDef	BTN_POLLING_PIN[] = {GPIO_Pin_6};
#endif //#if (KVM_MULITY_BUTTON_TYPE)

/* GLOBAL VARIABLES DECLARATIONS */
bit		BTN_Processing_Flag;
bit		BTN_Autscan_Flag;
U8_T	BTN_Status[2];
U8_T	TASK_BTN_Polling_ID,TASK_BTN_Polling_Active_ID;
//U8_T	TASK_BTN_Event_ID,TASK_BTN_Event_Active_ID;

#ifdef BTN_RESET
U8_T Btn_Reset_Cnt=0;
U8_T Btn_Reset_idx;
//U8_T TASK_BTN_Reset_ActiveID;
void TASK_BTN_Reset(void);
#endif
/* LOCAL VARIABLES DECLARATIONS */
/* LOCAL SUBPROGRAM DECLARATIONS */
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
extern U8_T	TASK_USBHC_Resume_Command_ID;
/* EXTERNAL SUBPROGRAM DECLARATIONS */

/*
 * ----------------------------------------------------------------------------
 * Function Name: KVM_BTN_Init(void)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * -----------------------------------------------------------------------------
 */
void KVM_BTN_Init(void)
{
	U8_T p[2]={0,0}; // 0 for Port0, 1 for port 2
	//U8_T index;

	BTN_Autscan_Flag    = 0;
	BTN_Processing_Flag = 0;	
#if (KVM_MULITY_BUTTON_TYPE)
	#if (BTN_MAX_PORT == 4)
		BTN_Status[0] = 0x0f; //default button up 	
	#else
		#if (BTN_MAX_PORT == 3)
			BTN_Status[0] = 0x07;
		#else
			BTN_Status[0] = 0x03;
		#endif
	#endif
#else	
	BTN_Status[0] = 0x01;
#endif
	BTN_Status[1] = BTN_Status[0];

#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
#if (KVM_BUTTON_WAKEUP)// Allow Button to wakeup Sleep Vinson
	_MISC_CIR_SFR(MISC_GPWER);
	_MISC_DR_READ_SFR(p[0]);
	_MISC_DR_READ_SFR(p[1]);
#if (KVM_MULITY_BUTTON_TYPE)	
	for (index=0; index < BUTTON_MAX_PORT; index++)
#else
	index =0;
#endif	
	{
		if (BTN_POLLING_PORT[index] == PORT0)
		{
			p[0] |= BTN_POLLING_PIN[index];
		}
		if (BTN_POLLING_PORT[index] == PORT2)
		{
			p[1] |= BTN_POLLING_PIN[index];
		}
	}		
	EXTINT6_DISABLE;
	_MISC_DR_SFR(p[0]); //allow pin 0 to wakeup
	_MISC_DR_SFR(p[1]); //allow pin 1 to wakeup	
	_MISC_CIR_SFR(MISC_GPWER);
	EXTINT6_ENABLE;
#endif //#if (KVM_BUTTON_WAKEUP)
#endif /* #if (SYSTEM_POWER_SAVING_MODE_SUPPORT) */
		
	TASK_BTN_Polling_ID = TASK_Create(TASK_BTN_Polling);
//#ifdef BTN_RESET
//	TASK_BTN_Reset_ID = TASK_Create(TASK_BTN_Reset);
//	TASK_BTN_Reset_ActiveID = 0;
//#endif	
	TASK_BTN_Polling_Active_ID = TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_BTN_Polling_ID,0,0,BTN_POLLING_TIMER,BTN_POLLING_TIMER);
	
#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("TASK_BTN_Polling_ID=%bu\n\r",TASK_BTN_Polling_ID);
#endif	
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: BTN_Event_Handle(void)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */

 //CG2026613
void BTN_Event_Handle(U8_T index)
{
	//U8_T newport,current_host;
	
	if (BTN_Processing_Flag) // no any button down
		return;
	
#if (KVM_MULITY_BUTTON_TYPE)
	/*
	if (BTN_Autscan_Flag & 	(index <= 1))
	{
		if ((BTN_Status[0] & BTN_AUTOSCAN_MASK) == BTN_AUTOSCAN_MASK)
		{
			BTN_Autscan_Flag = 0;			
		}			
		return;
	}
	*/	
#endif
	/*
	if (KVM_Control_Status & KVM_AUTOSCAN_MASK)
	{
		KVM_AutoscanStop_Jump(1,index);
		KVM_Control_Status &= ~KVM_AUTOSCAN_MASK; // clear the autoscan flag
		Task_Active_Table[TASK_KVM_AUTOSCAN_ACTIVE_ID].Task_Interval.w = 1; //wait for 1 ms		
	}
	else
	*/
	{	
#if (KVM_BUTTON_WAKEUP)					
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
			if (index == 0)  // port switch
			{
				KM_SYNC_ModeSwitch_Control(API_SWITCH_ENABLE);
				KM_SYNC_Port_Jump_Power(KVM_CurrentHost,ROAMING_RIGHT,0);
			}				
			else if (index == 1) //Sync Mode Control
			{
				if (KM_SYNC_SyncMode_State())
					API_Set_Sync_Mode(API_SYNC_DISABLE);
				else
					API_Set_Sync_Mode(API_SYNC_ENABLE);
			}
		}
	}
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
	U8_T	index=0;
	U8_T    press_cnt=0;
		
	//----------------------------------------------
	//1.Get Button status from	
	//----------------------------------------------
	//if (BTN_Processing_Flag == 0) // no any button down	 
#if (KVM_MULITY_BUTTON_TYPE)
	for (index = 0; index < BTN_MAX_PORT; index++)
#endif
	{
		//printf("[%bu]",(U8_T)P1_6);
		if (GPIO_GetPinValue(BTN_POLLING_PORT[index],BTN_POLLING_PIN[index]) == 0)  //if button down			
		{
			press_cnt++;
#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
			PowerSavingInhibitFlag = 1;
#endif //#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)								
			BTN_Status[0] &= ~BIT_MASK[index];
#if (KVM_CONSOLE_DEBUG_MODE)
			printf("+BTN:%bx Pressed:%02bx\n\r", index,BTN_Status[0]);
#endif
			//1.handle the reset operation								
			#ifdef BTN_RESET			
			if (press_cnt==1)
			{
				if (Btn_Reset_Cnt == 0)
				{					
					Btn_Reset_idx = index;
				}	
				else
				{
					if (Btn_Reset_Cnt >= BTN_RESET_TINMER)
					{
						if (Btn_Reset_idx == index)
						{	
							TASK_BTN_Reset();
						}	
					}		
				}						
				Btn_Reset_Cnt++;
			}
			else
			{
				Btn_Reset_Cnt=0;					
			}					
			#endif
		}	
		else
		{
			BTN_Status[0] |= BIT_MASK[index];
//#if (KVM_CONSOLE_DEBUG_MODE)
			//printf("BTN:%bx Release\n\r", index);
//#endif				
		}						
	}	
 
	//----------------------------------------------
	//2.Process the Button operation
	//----------------------------------------------
	//printf("b1%02b.%02bx",BTN_Status[0],BTN_Status[1]);
	if (BTN_Status[0] != BTN_Status[1]) //if the button is different from old one
	{	
#if (KVM_MULITY_BUTTON_TYPE)
		for (index = 0; index < BTN_MAX_PORT; index++)
#else
		index = 0;	
#endif	
		{
			//printf("b2");
			if ((BTN_Status[1] & BIT_MASK[index]) == 0) //if old status is button down
			{
				//printf("b3");
				//Any process need to take care?				
				//1.handle the autoscan mode
				#if (KVM_MULITY_BUTTON_TYPE)
				/*
				if ((BTN_Status[0] & BTN_AUTOSCAN_MASK) == 0x00)
				{
					Task_Active_Table[TASK_BTN_Polling_Active_ID].Task_Interval.w = 1000;
					KVM_Autoscan_Mode_Contorl(1);
					BTN_Autscan_Flag = 1;
				}
				*/				
				#endif
				
				if ((BTN_Status[0] & BIT_MASK[index]) == 0) //if new status is still button down
				{	
#if (KVM_MULITY_BUTTON_TYPE)					
					continue;
#endif					
				}		
				else
				{ //new state is button up
#if (KVM_CONSOLE_DEBUG_MODE)
					printf("-BTN:%bx Release\n\r", index);
#endif
					#ifdef BTN_RESET
					Btn_Reset_Cnt = 0;
					#endif
										
					BTN_Event_Handle(index);			
				}					
			}

			if (BTN_Status[1] & BIT_MASK[index]) //if old status is button up
			{
				if (BTN_Status[0] & BIT_MASK[index]) //if new status is still button up
				{
#if (KVM_MULITY_BUTTON_TYPE)					
					continue;
#endif					
				}					
			}								
		}
		BTN_Status[1] = BTN_Status[0]; //store the button status
	}
 	
}

#ifdef BTN_RESET
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
	U8_T i,j;
	U16_T delay_cnt;
	
#if (KVM_BUZZER_SUPPORT)	
	BUZZER_Script_Active(PortSwitch_Sound);	
#endif //#if (KVM_BUZZER_SUPPORT)	
	TASK_Destory_Current();		
	for (i=0; i < KVM_MAX_PORT; i++)
	{
		KVM_Select_Led_Control(i,LED_OFF);		
	}
	
	for (delay_cnt=0; delay_cnt < 35000; delay_cnt++)
	{
		for (i=0; i < 5 ; i++)
		{
			j = j + 1;
		}	
	}
	
	KVM_Flash.cSystemFlag2 |= (SYSTEM_ALL_SYNC_MASK | SYSTEM_KB_SYNC_MASK | SYSTEM_MS_SYNC_MASK);
	KM_SYNC_KM_SyncModeLed(LED_ON);
	KM_SYNC_Sync_KB_Repeat = 0;
    memset(KM_SYNC_Sync_KB_RepeatActive,0,sizeof(KM_SYNC_Sync_KB_RepeatActive));
    memset(KM_SYNC_Sync_KB_RepeatTable,0x00,sizeof(KM_SYNC_Sync_KB_RepeatTable));
	//CPU_SoftReboot();
}
#endif

#endif //#if (KVM_BUTTON_HARDWARE)

/* End of btn_interrupt.c */
