/*
******************************************************************************
 *     Copyright (c) 2012	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*================================================================================
 * Module Name : exti_ps2.c
 * Purpose     : Use External Interrupt to access and control PS/2 device 
 * Author      : Jack Wang
 * Date        :
 * Notes       : None
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include <stdio.h>
#include <string.h>
#include	"ax22000.h"
#include	"types.h"
#include	"interrupt_cfg.h"
#include	"system_cfg.h"
#include	"uart.h"
#include	"gpio.h"
#include	"ps2_host.h"
#include	"exti.h"
#include	"exti_ps2.h"
#include	"printf.h"

#if (SYSTEM_EXTI_PS2_ENABLE)  
/* GLOBAL DEFINATION DECLARATIONS */
/* GLOBAL CONTANT DECLARATIONS    */                                                   
/* GLOBAL VARIABLES DECLARATIONS  */ 

#if ( SYSTEM_EXTI_PS2_PORT_MAX == 1 ) 
      GPIO_TypeDef 		*PS2_CLK_PORT[]  = {PS2A_CLK_PORT};
      GPIO_PinTypeDef  PS2_CLK_PIN[]   = {PS2A_CLK_PIN };
      GPIO_TypeDef 		*PS2_DATA_PORT[] = {PS2A_DATA_PORT};
      GPIO_PinTypeDef  PS2_DATA_PIN[]  = {PS2A_DATA_PIN};
#endif

#if ( SYSTEM_EXTI_PS2_PORT_MAX == 2 ) 
      GPIO_TypeDef 		*PS2_CLK_PORT[]  = {PS2A_CLK_PORT ,PS2B_CLK_PORT};
      GPIO_PinTypeDef  PS2_CLK_PIN[]   = {PS2A_CLK_PIN  ,PS2B_CLK_PIN };
      GPIO_TypeDef 		*PS2_DATA_PORT[] = {PS2A_DATA_PORT,PS2B_DATA_PORT};
      GPIO_PinTypeDef  PS2_DATA_PIN[]  = {PS2A_DATA_PIN ,PS2B_DATA_PIN };
#endif

/* STATIC VARIABLE DECLARATIONS  */
/* LOCAL SUBPROGRAM DECLARATIONS */

/*
 * ----------------------------------------------------------------------------
 * void EXTI_PS2_Init(void)
 * Purpose : Initializes the PS/2 relative EXTIx interrupt peripheral and GPIO
 * Params  : none
 * Returns : none
 * Note    : 
 * ----------------------------------------------------------------------------
 */
void EXTI_PS2_Init(void)
{			
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;	
		
	    
	/*2.GPIO Init */
	    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	    
			#if ( SYSTEM_EXTI_PS2_PORT_MAX == 1 ) 
			      GPIO_InitStruct.GPIO_Pin = PS2_CLK_PIN[0]; 			
			      GPIO_Init(PS2_CLK_PORT[0],&GPIO_InitStruct);
			      
			      GPIO_InitStruct.GPIO_Pin = PS2_DATA_PIN[0]; 
			      GPIO_Init(PS2_DATA_PORT[0],&GPIO_InitStruct);
			      //GPIO_SetOneBit(PS2_CLK_PORT[0], PS2_CLK_PIN[0]); // pull High
			#endif
			
			#if ( SYSTEM_EXTI_PS2_PORT_MAX == 2 ) 			      
			      GPIO_InitStruct.GPIO_Pin = PS2_CLK_PIN[1]; 			      
			      GPIO_Init(PS2_CLK_PORT[1],&GPIO_InitStruct);
			      
			      GPIO_InitStruct.GPIO_Pin = PS2_DATA_PIN[1]; 
			      GPIO_Init(PS2_DATA_PORT[1],&GPIO_InitStruct);
			#endif			
	
	/*3.Exti Init */
			EXTI_InitStruct.EXTI_Mode    = EXTI_Mode_Interrupt;			
	   	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;	        
	   
	   	EXTI_Init(EXTI0,&EXTI_InitStruct);
	   	EXTI_ITConfig(EXTI0,EXTI_IT_GLOBAL_FLAG,ENABLE); // enable interrupt
	   	
	   	#if ( SYSTEM_EXTI_PS2_PORT_MAX == 2 ) 			      
	   	EXTI_Init(EXTI1,&EXTI_InitStruct);	
	   	EXTI_ITConfig(EXTI1,EXTI_IT_GLOBAL_FLAG,ENABLE);
	   	#endif	   		   		   	
} /* End of EXTI_PS2_Init */
 
/*
 * ----------------------------------------------------------------------------
 * void EXTI_PS2_Disable
 * Purpose : Set the PS2 port to disable condition: EXTIx interupt disable, the 
 *           clock is low
 * Params  : PS2x: Select the PS2 port peripheral. 
 *           this parameter can be one of the following values:
 *           EPS2A, EPS2B
 * Returns : None
 * Note    :  
 * ----------------------------------------------------------------------------
 */ 
void EXTI_PS2_Disable(PS2Port_TypeDef PS2x)
{  
	GPIO_InitTypeDef GPIO_InitStruct;
  
  /*1. Disable the interrupt first */
  if (PS2x == EPS2A)
  {
  	  _EXTI0_INTERRUPT_RESET();
  }
  else if (PS2x == EPS2B)
  {     	  
  	  _EXTI1_INTERRUPT_RESET();
	}	 
	
	/*2. Pull low the clock to inhibit input */
	GPIO_InitStruct.GPIO_Pin = PS2_CLK_PIN[PS2x]; 			
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD; // Out open drain	
  GPIO_Init(PS2_CLK_PORT[PS2x],&GPIO_InitStruct);			      			      
  GPIO_ResetOneBit(PS2_CLK_PORT[PS2x], PS2_CLK_PIN[PS2x]); // Pull Low			
} /* End of EXTI_PS2_Disable */

/*
 * ----------------------------------------------------------------------------
 * void EXTI_PS2_Enable
 * Purpose : Set the PS2 port to disable condition: EXTIx interupt disable, the 
 *           clock is low
 * Params  : PS2x: Select the PS2 port peripheral. 
 *           this parameter can be one of the following values:
 *           EPS2A, EPS2B
 * Returns : None
 * Note    :  
 * ----------------------------------------------------------------------------
 */ 
void EXTI_PS2_Enable(PS2Port_TypeDef PS2x)
{  
	GPIO_InitTypeDef GPIO_InitStruct;    
	
	/*1. Pull Up the clock to prohibit input */
	GPIO_InitStruct.GPIO_Pin = PS2_CLK_PIN[PS2x]; 			
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING; // Out open drain	
  GPIO_Init(PS2_CLK_PORT[PS2x],&GPIO_InitStruct);			      			      
  //GPIO_SetOneBit(PS2_CLK_PORT[PS2x], PS2_CLK_PIN[PS2x]); // Pull Up
  
  /*2. Enabl the interrupt */
  if (PS2x == EPS2A)
  {
  	  _EXTI0_Int_Flag_Reset(); // reset IE0
  	  _EPS2A_Clock_High();  	  
  	  _EXTI0_INTERRUPT_SET();
  }
  else if (PS2x == EPS2B)
  {     	  
  	  _EXTI1_Int_Flag_Reset(); // reset IE1
  	  _EPS2B_Clock_High();  	  
  	  _EXTI1_INTERRUPT_SET();
	}	 
} /* End of EXTI_PS2_Enable */

/*
 * ----------------------------------------------------------------------------
 * void EXTI_PS2_DeInit
 * Purpose : Set the PS2 port to deinit condition: EXTIx interupt disable, the 
 *           clock is high
 * Params  : PS2x: Select the PS2 port peripheral. 
 *           this parameter can be one of the following values:
 *           EPS2A, EPS2B
 * Returns : None
 * Note    :  
 * ----------------------------------------------------------------------------
 */  
void EXTI_PS2_DeInit(PS2Port_TypeDef PS2x)
{  
	GPIO_InitTypeDef GPIO_InitStruct;
  
  /*1. Disable the interrupt first */
  if (PS2x == EPS2A)
  {
  	  EXTI_ITConfig(EXTI0,EXTI_IT_GLOBAL_FLAG,DISABLE);
  }
  else if (PS2x == EPS2B)
  {     	  
  	  EXTI_ITConfig(EXTI1,EXTI_IT_GLOBAL_FLAG,DISABLE);		  
	}	 
	
	/*2. Pull HIGH the clock to prohibit input */
	GPIO_InitStruct.GPIO_Pin = PS2_CLK_PIN[PS2x]; 			
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING; // Out open drain	
  GPIO_Init(PS2_CLK_PORT[PS2x],&GPIO_InitStruct);			      			      
  GPIO_SetOneBit(PS2_CLK_PORT[PS2x], PS2_CLK_PIN[PS2x]); // Pull HIGH			
} /* End of EXTI_PS2_DeInit */

/*
 * ----------------------------------------------------------------------------
 * FlagStatus EXTI_PS2_Pin_Control
 * Purpose : Control PS/2 port pin high/low
 * Params  : PS2x: Select the EXTI peripheral. 
 *           this parameter can be one of the following values:
 *           EPS2A, EPS2B
 * @param  @ClockPinControl: specifies the line status for clock line
 *         This parameter can be one of the following values:
 *         PS2_Clock_Low,PS2_Clock_High
 *
 *         @DataPinControl: specifies the line status for data line
 *         This parameter can be one of the following values:
 *         PS2_Data_Low,PS2_Data_High
 *            
 * Returns : The received data.(include the address)
 * Note    :  
 * ----------------------------------------------------------------------------
 */ 
void EXTI_PS2_Pin_Control(PS2Port_TypeDef PS2x,PS2PinControl_TypeDef ClockPinControl,PS2PinControl_TypeDef DataPinControl)
{    	
	
  // Do the clock line control
  if ( ClockPinControl == PS2_Line_Low )
  {  	 
  	 GPIO_ResetOneBit(PS2_CLK_PORT[PS2x], PS2_CLK_PIN[PS2x]); // Pull low  	 
  }
  else if ( ClockPinControl == PS2_Line_High )
  {
  	 GPIO_SetOneBit(PS2_CLK_PORT[PS2x], PS2_CLK_PIN[PS2x]); // Pull HIGH			
  }
  
  // Do the Data line control
  if ( DataPinControl == PS2_Line_Low )
  {
  	 GPIO_ResetOneBit(PS2_DATA_PORT[PS2x], PS2_DATA_PIN[PS2x]); // Pull HIGH			
  }
  else if ( DataPinControl == PS2_Line_High )
  {
  	 GPIO_SetOneBit(PS2_DATA_PORT[PS2x], PS2_DATA_PIN[PS2x]); // Pull HIGH			
  }  			
} /* End of EXTI_PS2_Pin_Control */  

#endif /* End of SYSTEM_EXTI_PS2_ENABLE == 1 */ 

/* End of exti_ps2.c */
