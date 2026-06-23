/*
 *********************************************************************************
 *     Copyright (c) 2014   ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : kvm_ir.c
 * Purpose     : The IR receiver handler program,use timer0 for counter 
 * Author      : 
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
*/

/* INCLUDE FILE SECTION 									*/
/* KEIL Header File Section */	
#include	"stdio.h"
#include	"string.h"

/* System Relate Header File Seciton 		*/
#include "project_include.h"

#if (SYSTEM_IR_DEVICE_SUPPORT)
/* NAMING CONSTANT DECLARATIONS */	
#define IR_KEY_CNT				6 
/* GLOBAL VARIABLES DECLARATIONS 					*/
volatile U8_T  IR_Control_State;
volatile U8_T  IR_MS_Cnt;
volatile U8_T  IR_Plus_Cnt;
volatile U8_T  IR_Buffer_Cnt;
volatile U8_T  IR_Buffer[8];
U8_T  IR_Edge_Setting[2];
//U8_T  IR_Key1_Numeric[4]={0x1c,0x0e,0x01,0x03};
//U8_T  IR_Key2_Numeric[4]={0x02,0x19,0x12,0x05};
U8_T  IR_Key1_Numeric[IR_KEY_CNT]={0x1c,0x0e,0x01,0x03,0x02,0x19};
U8_T  IR_Key2_Numeric[IR_KEY_CNT]={0xe3,0xf1,0xfe,0xfc,0xfd,0xe6};
U8_T  IR_Key3_Power     =0x1e;
U16_T IR_1MS_Counter,IR_2MS_Counter;
volatile U16_T IR_Plus_Timer0;
volatile U16_T IR_Plus_Timer1;
volatile U16_T IR_Plus_Timer_Dif;

/* LOCAL VARIABLES DECLARATIONS 					*/
/* LOCAL SUBPROGRAM DECLARATIONS 					*/
void KVM_IR_Timer0_Init(void);
void KVM_IR_Gpio_Init(void);
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
/* EXTERNAL SUBPROGRAM DECLARATIONS 			*/ 


/*
 * ----------------------------------------------------------------------------
 * Function Name: KVM_IR_Init
 * Purpose: Do the IR device control relative hardware & software control
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void KVM_IR_Init(void)
{
	/*1.Do the timer 0 init */
	KVM_IR_Timer0_Init();
	
	/*2.Do the GPIO init */
	KVM_IR_Gpio_Init();

	IR_Plus_Cnt     = 0;
	IR_Buffer_Cnt   = 0;
	IR_Buffer_Cnt   = 0;
	IR_MS_Cnt		= 0;
	IR_Control_State = IR_IDLE;
	
	switch (CSREPR & SCS_96M)
	{
		case SCS_48M :
			IR_1MS_Counter = 4000;
			IR_2MS_Counter = 8000;			
			break;
		case SCS_96M :
			IR_1MS_Counter = 8000;
			IR_2MS_Counter = 16000;			
			break;		
	}
	
}
/*
 * ----------------------------------------------------------------------------
 * Function Name: KVM_IR_Timer0_Init
 * Purpose: Initiate global values in the MS timer module.
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void KVM_IR_Timer0_Init(void)
{
	  
   // Stop the Timer First IE/TR
   // Keep the old setting for TCON
   //tr_flag = TR0;
   TR0 = 0; // disable timer0

   /* 1-CPU Clock division */			      
   //CKCON |= T0M_;      	   
   TMOD &= ~T0_CT_;	/* timer mode */ 	           	     
   /* 3-GATE -> GATE bit setting */			     
   TMOD &= ~T0_GATE_;  /* GATE disable */	 	      	           
   /* 4-C/T Mode */			 
   TMOD |= 0x01; // 16 bit timer mode          
   /* 5-THx,TL0 Setting */			 
   TH0 = 0x00; // timer start from 0x00
   TL0 = 0x00; 	  
   
   TR0 = 1; // disable timer0    	        
	  
} /* End of TIM_TimeBaseInit */

/*
 * ----------------------------------------------------------------------------
 * Function Name: KVM_IR_Gpio_Init
 * Purpose: Initial the GPIO interrupt control
 * Params : 
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void KVM_IR_Gpio_Init(void)
{
	U8_T interrupt5;
	U8_T regvalue0;
	#if (KVM_BUTTON_WAKEUP)
	U8_T regvalue1;	
	#endif
	
	EXTINT4_DISABLE;
	interrupt5 = EINT5; // keep the orginal value
	EXTINT5_DISABLE;
    //PCT case IR input is P14 
  	#if (KVM_BUTTON_WAKEUP)// Allow Button to wakeup Sleep Vinson		
  	_MISC_CIR_SFR(MISC_GPWER);	
  	_MISC_DR_READ_SFR(regvalue0);	 				
	_MISC_DR_READ_SFR(regvalue1); //alow pin 4 to wakeup									
  	
  	_MISC_DR_SFR(regvalue0);	 				
	_MISC_DR_SFR((regvalue1 | IR_PIN_MASK)); //alow pin 4 to wakeup								
	_MISC_CIR_SFR(MISC_GPWER);	
	#endif /* #if (KVM_BUTTON_WAKEUP) */
	
	/* Disable the debounce time */
	_MISC_CIR_SFR(MISC_GP0DER);
	_MISC_DR_READ_SFR(regvalue0);
	regvalue0 &= ~IR_PIN_MASK; //Clear debounce time
	_MISC_DR_SFR(regvalue0);
	_MISC_CIR_SFR(MISC_GP0DER);
	//1.Set Falling edge trigger
	//1-0.Backup old value
	_MISC_CIR_SFR(MISC_GP0TR);
	_MISC_DR_READ_SFR(IR_Edge_Setting[0]);
	_MISC_DR_READ_SFR(IR_Edge_Setting[1]);
	//_MISC_DR_READ_SFR(regvalue0);
	//_MISC_DR_READ_SFR(regvalue1);
	
	//2-0.assign new value
	/*
	//if (BTN_POLLING_PIN[0] < GPIO_Pin_4) // pin0~pin3
	{
		regvalue0 &= ~(BTN_TRIGGER_MASK << BTN_POLLING_SHIFT[0]);
		trigger = regvalue0 | (BTN_TRIGGER_FALLING << BTN_POLLING_SHIFT[0]);
		regvalue0 = trigger;		
	}	
	else	
	*/
	{
		IR_Edge_Setting[1] &= ~(BTN_TRIGGER_MASK << IR_TRIGGER_LEVEL_SHIFT);
		IR_Edge_Setting[1] = IR_Edge_Setting[1] | (BTN_TRIGGER_FALLING << IR_TRIGGER_LEVEL_SHIFT);
		//IR_Edge_Setting[1] = trigger;		
	}
	
	_MISC_DR_SFR(IR_Edge_Setting[0]);
	_MISC_DR_SFR(IR_Edge_Setting[1]);
	_MISC_CIR_SFR(MISC_GP0TR);
	 
	//2.Enable INTER4
	EXTINT4_ENABLE;
	EINT5 = interrupt5;
} /* End of TIM_Cmd */

/*
 * ----------------------------------------------------------------------------
 * Function Name: KVM_IR_Key_Handle
 * Purpose: Handle the IR input key
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void KVM_IR_Key_Handle(void)
{
	U8_T index;
	
	if (IR_Buffer_Cnt < 4)
		return;
	
	IR_Control_State = 0; //next is bit 1.
	
	printf("Counter=%bu\n\r",IR_Buffer_Cnt);			
	printf("Keycode=%02bx\n\r",IR_Buffer[2]);	
	if (IR_Buffer[2] != (~IR_Buffer[3]))
	{		
		//printf("Fail\n\r");
		return;
	}	
	
	//handle key left(next port)
	if (IR_Key1_Numeric[4] == IR_Buffer[2])
	{
		index = KVM_CurrentHost+1;
		if (index >= KVM_MAX_PORT)
		{
			index = 0;				
			goto KVM_IR_CONTROL_PORT_JUMP;
		}	
	}
	
	//handle key right(pre port)
	if (IR_Key1_Numeric[5] == IR_Buffer[2])
	{
		if (KVM_CurrentHost == 0)
		{
			index = KVM_MAX_PORT-1;
		}	
		else
		{
			index =  KVM_CurrentHost-1;
		}				
		goto KVM_IR_CONTROL_PORT_JUMP;
		
	}	
		
	
	for (index=0; index < 4 ;index++)
	{
		//handle key 1~4
		if (IR_Key1_Numeric[index] == IR_Buffer[2])
		{ //OUT1 handle			
			if (KVM_CurrentHost != index) // if the port is not active now
			{
KVM_IR_CONTROL_PORT_JUMP:				
				if (KVM_Power_Jump_Check(index,0))
				{			
					KVM_NextHost = index;
					printf("Index=%bu\n\r",index);
					KVM_Control_Status |= KVM_PORT_SW_MASK;
					TASK_KVM_Switch_Event_Control_Fork(index,KVM_SW_INTERVAL_TIME);
					KVM_Audio_Combo_Switch_Check(index);
				}
				return; // no more check about button down
			}				
		}
				
		if (IR_Key2_Numeric[index] == IR_Buffer[2])
		{ //OUT2						
			printf("OUT2:%bu\n\r");
			switch(index)
		    {
		    	case 0:
		    		GPIO_SetOneBit(GPIOA,GPIO_Pin_1,0);
					GPIO_SetOneBit(GPIOB,GPIO_Pin_3,0);
					GPIO_SetOneBit(GPIOA,GPIO_Pin_3,1);
		    		break;
		    	case 2:
		    		GPIO_SetOneBit(GPIOA,GPIO_Pin_1,0);
					GPIO_SetOneBit(GPIOB,GPIO_Pin_3,1);
					GPIO_SetOneBit(GPIOA,GPIO_Pin_3,1);
		    		break;
		    	case 1:
		    		GPIO_SetOneBit(GPIOA,GPIO_Pin_1,1);
					GPIO_SetOneBit(GPIOB,GPIO_Pin_3,0);
					GPIO_SetOneBit(GPIOA,GPIO_Pin_3,1);
		    		break;
		    	case 3:
		    		GPIO_SetOneBit(GPIOA,GPIO_Pin_1,1);
					GPIO_SetOneBit(GPIOB,GPIO_Pin_3,1);
					GPIO_SetOneBit(GPIOA,GPIO_Pin_3,1);
		    		break;			
		    }		
		    return;
		}	
	}
}

#endif /* #if (SYSTEM_IR_DEVICE_SUPPORT) */

/* End of kvm_ir.c */