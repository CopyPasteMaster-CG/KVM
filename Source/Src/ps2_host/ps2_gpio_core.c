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

#if (SYSTEM_PS2_HOST_ENABLE) && (SYSTEM_GPIO_PS2_ENABLE)
/* NAMING CONSTANT DECLARATIONS */	
#define PS2A_CLK_GPIO_PIN	P0_0
#define PS2A_DAT_GPIO_PIN	P0_1
#define PS2B_CLK_GPIO_PIN	P0_2
#define PS2B_DAT_GPIO_PIN	P0_3
#define PS2A_Clk_Bit		0x01
#define PS2A_Data_Bit		0x02
#define PS2B_Clk_Bit		0x04
#define PS2B_Data_Bit		0x08

const U8_T				PS2_GPIO_SFR[] 	 = {MISC_GP0TR, MISC_GP0TR};
const GPIO_TypeDef		*PS2_GPIO_PORT[] = {PORT0,PORT0,};
const GPIO_PinTypeDef	PS2_GPIO_PIN[] 	 = {GPIO_Pin_0,GPIO_Pin_2};
const GPIO_PinTypeDef	PS2_GPIO_SHIFT[] = {0,4};

#define PS2_GPIO_MAX    sizeof(PS2_GPIO_SFR)
/* GLOBAL VARIABLES DECLARATIONS */
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
void PS2_Init(void)
{
	U8_T index,regvalue1;
	U8_T interrupt5;
	U8_T trigger,regvalue0;

	EXTINT4_DISABLE;
	interrupt5 = EINT5; // keep the orginal value
	EXTINT5_DISABLE;
	
#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)	 
	_MISC_CIR_SFR(MISC_GPWER);
	_MISC_DR_READ_SFR(regvalue0);
	_MISC_DR_READ_SFR(regvalue1);
	
	for (index=0; index < PS2_GPIO_MAX; index++) 
	{
		if (PS2_GPIO_PORT[index] == PORT0)
		{
			p[0] |= PS2_GPIO_PIN[index];
		}
		if (PS2_GPIO_PORT[index] == PORT2)
		{
			p[1] |= PS2_GPIO_PIN[index];
		}
	}		
	EXTINT6_DISABLE;
	_MISC_DR_SFR(p[0]); //allow pin 0 to wakeup
	_MISC_DR_SFR(p[1]); //allow pin 1 to wakeup	
	_MISC_CIR_SFR(MISC_GPWER);
	EXTINT6_ENABLE;	 
#endif
 	
	for (index=0;index < PS2_GPIO_MAX; index++)
	{
		GPIO_SetOneBit(PS2_GPIO_PORT[index],PS2_GPIO_PIN[index],1);
		//1.Set Falling edge trigger
		//1-0.Backup old value
		_MISC_CIR_SFR(PS2_GPIO_SFR[index]);
		_MISC_DR_READ_SFR(regvalue0);
		_MISC_DR_READ_SFR(regvalue1);
		
		//2-0.assign new value
		if (PS2_GPIO_PIN[index] < GPIO_Pin_4) // pin0~pin3
		{
			regvalue0 &= ~(BTN_TRIGGER_MASK << PS2_GPIO_SHIFT[index]);
			trigger = regvalue0 | (BTN_TRIGGER_FALLING << PS2_GPIO_SHIFT[index]);
			regvalue0 = trigger;
		}
		else
		{
			regvalue1 &= ~(BTN_TRIGGER_MASK << PS2_GPIO_SHIFT[index]);
			trigger = regvalue1 | (BTN_TRIGGER_FALLING << PS2_GPIO_SHIFT[index]);
			regvalue1 = trigger;
		}
		_MISC_DR_SFR(regvalue0);
		_MISC_DR_SFR(regvalue1);
		_MISC_CIR_SFR(PS2_GPIO_SFR[index]);
		 
		//3.Clear the de-bounce time
		if (PS2_GPIO_SFR[index] == MISC_GP0TR)
		{
			de_bounce_sfr = MISC_GP0DER;
		}	
		else
		{
			de_bounce_sfr = MISC_GP2DER;
		}	
		//Read Interrupt
		_MISC_CIR_SFR(de_bounce_sfr);
		_MISC_DR_READ_SFR(regvalue0);
		regvalue0 &= ~PS2_GPIO_PIN[index]; //Clear debounce time
		_MISC_DR_SFR(regvalue0);
		_MISC_CIR_SFR(de_bounce_sfr);
	
	}
	//2.Enable INTER4
	EXTINT4_ENABLE;
	EINT5 = interrupt5;
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
void PS2_Gpio_ISR_Handle(GPIO_TypeDef *gpio_port,U8_T pvalue)
{
	U8_T regvalue1;	
	U8_T regvalue0;
	U8_T pid=0;
	
	//Read PS2A			
	if (gpio_port == PS2_GPIO_PORT[pid])
	{
		if (pvalue & PS2_GPIO_PIN[pid]) //pin happened
		{
			goto PS2_Gpio_ISR_Handle_2;
		}			
	}		
	pid++;
	goto PS2_Gpio_ISR_Handle_3;
PS2_Gpio_ISR_Handle_2:	
	if (pid==0)
		PS2A_CLK_GPIO_PIN = 1;
	else
		PS2B_CLK_GPIO_PIN = 1;
	_MISC_CIR_SFR(PS2_GPIO_SFR[pid]);
	_MISC_DR_READ_SFR(regvalue0);
	_MISC_DR_READ_SFR(regvalue1);							
	if (regvalue0 & (BTN_TRIGGER_FALLING << PS2_GPIO_SHIFT[pid]))
	{									
		regvalue0 &= ~(BTN_TRIGGER_MASK << PS2_GPIO_SHIFT[pid]);				
		regvalue0  |= (BTN_TRIGGER_RISING << PS2_GPIO_SHIFT[pid]);								
		ISR_FIFO[ISR_FIFO_Wp].State = 0;
	}	
	else
	{ //Rising						
		regvalue0 &= ~(BTN_TRIGGER_MASK << PS2_GPIO_SHIFT[pid]);
		regvalue0  |= (BTN_TRIGGER_FALLING << PS2_GPIO_SHIFT[pid]);															
		ISR_FIFO[ISR_FIFO_Wp].State = 1;
	}
	_MISC_DR_SFR(regvalue0);
	_MISC_DR_SFR(regvalue1);
	_MISC_CIR_SFR(PS2_GPIO_SFR[pid]);
#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
	PowerSavingInhibitFlag = 1;
#endif //#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)	
	//2. Comsoite the PS2 data byte by check the interrupt counter
PS2_Gpio_ISR_Handle_3:	
	pid++;
	if (pid <= PS2_GPIO_MAX)
		goto PS2_Gpio_ISR_Handle_2;
	
	return;
}

/*
 * ----------------------------------------------------------------------------
 * void PS2_Gpio_Core_Clk_Int_Ctrl(U8_T ps2port,U8_T control)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void PS2_Gpio_Core_Clk_Int_Ctrl(U8_T ps2port,U8_T control)
{
	U8_T regvalue0,regvalue1,trigger;
	
	//GPIO_SetOneBit(PS2_GPIO_PORT[ps2port],PS2_GPIO_PIN[ps2port],1);
	//1.Set Falling edge trigger
	//1-0.Backup old value
	_MISC_CIR_SFR(PS2_GPIO_SFR[ps2port]);
	_MISC_DR_READ_SFR(regvalue0);
	_MISC_DR_READ_SFR(regvalue1);
		
	//2-0.assign new value
	if (PS2_GPIO_PIN[ps2port] < GPIO_Pin_4) // pin0~pin3
	{		
		regvalue0 &= ~(BTN_TRIGGER_MASK << PS2_GPIO_SHIFT[ps2port]);
		if (control)
		{	
			trigger = regvalue0 | (BTN_TRIGGER_FALLING << PS2_GPIO_SHIFT[ps2port]);
		}	
		regvalue0 = trigger;
	}
	else
	{
		regvalue1 &= ~(BTN_TRIGGER_MASK << PS2_GPIO_SHIFT[ps2port]);
		if (control)
		{
			trigger = regvalue1 | (BTN_TRIGGER_FALLING << PS2_GPIO_SHIFT[ps2port]);
		}	
		regvalue1 = trigger;
	}
	_MISC_DR_SFR(regvalue0);
	_MISC_DR_SFR(regvalue1);
	_MISC_CIR_SFR(PS2_GPIO_SFR[ps2port]);
}

/*
 * ----------------------------------------------------------------------------
 * void PS2_Gpio_Clk_Int_Ctrl(U8_T ps2port,U8_T pin_bit_value)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void PS2_Gpio_Clk_Int_Ctrl(U8_T ps2port,U8_T pin_bit_value)
{
	if (ps2port == PS2A)
	{
		if (pin_bit_value & PS2A_Clk_Bit) // enable interrupt
		{
			PS2_Gpio_Core_Clk_Int_Ctrl(PS2A,1);
		}			
		else
		{
			PS2_Gpio_Core_Clk_Int_Ctrl(PS2A,0);
		}			
	}
	
	if (ps2port == PS2B)
	{
		if (pin_bit_value & PS2B_Clk_Bit) // enable interrupt
		{
			PS2_Gpio_Core_Clk_Int_Ctrl(PS2B,1);
		}			
		else
		{
			PS2_Gpio_Core_Clk_Int_Ctrl(PS2B,0);
		}			
	}
	
}	
	
/*
 * ----------------------------------------------------------------------------
 * void PS2_Gpio_Pin_Control(U8_T ps2_port,U8_T pin_bit_value)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void PS2_Gpio_Pin_Control(U8_T ps2_port,U8_T pin_bit_value)
{
	if (ps2_port & PS2A_Clk_Bit)
	{
		if (pin_bit_value & PS2A_Clk_Bit) // pull high
		{
			PS2A_CLK_GPIO_PIN = 1;
			//enable the edge low interrupt
			PS2_Gpio_Clk_Int_Ctrl(PS2A,PS2A_Clk_Bit);
		}
		else
		{
			PS2A_CLK_GPIO_PIN = 0;
			//disable the interrupt
			PS2_Gpio_Clk_Int_Ctrl(PS2A,0);
		}			
	}
	
	if (ps2_port & PS2B_Clk_Bit)
	{
		if (pin_bit_value & PS2B_Clk_Bit) // pull high
		{
			PS2B_CLK_GPIO_PIN = 1;
			//enable the edge low interrupt
			PS2_Gpio_Clk_Int_Ctrl(PS2B,PS2B_Clk_Bit);
		}
		else
		{
			PS2B_CLK_GPIO_PIN = 0;
			//disable the interrupt
			PS2_Gpio_Clk_Int_Ctrl(PS2B,0);
		}			
	}	
	
	if (ps2_port & PS2A_Data_Bit)
	{
		if (pin_bit_value & PS2A_Data_Bit) // pull high
		{
			PS2A_DATA_GPIO_PIN = 1;			
		}
		else
		{
			PS2A_DATA_GPIO_PIN = 0;			
		}			
	}
	
	if (ps2_port & PS2B_Data_Bit)
	{
		if (pin_bit_value & PS2B_Data_Bit) // pull high
		{
			PS2B_DATA_GPIO_PIN = 1;			
		}
		else
		{
			PS2B_DATA_GPIO_PIN = 0;			
		}			
	}
}	

#endif //#if (KVM_BUTTON_HARDWARE)

/* End of btn_interrupt.c */
