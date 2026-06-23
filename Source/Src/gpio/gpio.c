/*
******************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *     
 *     CHIP : 8051
 ******************************************************************************
 */
/*================================================================================
 * Module Name : gpio.c
 * Purpose     : The gpio HAL module driver. It manages the GPIO port control
 * Author      : Jack Wang
 * Date        :
 * Notes       : None.
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	"ax6800x.h"
#include	"types.h"
#include	"gpio.h"
#include	"absacc.h"

/* GLOBAL VARIABLES DECLARATIONS */


/* STATIC VARIABLE DECLARATIONS */
 

/* LOCAL SUBPROGRAM DECLARATIONS */
 
/* LOCAL SUBPROGRAM BODIES */

/*
 * ----------------------------------------------------------------------------
 * void GPIO_SetOneBit 
 * Purpose : Sets one single bit of the selected data port.
 * Params  : GPIOx: where x can be (A..D) to select the GPIO peripheral.
 *           GPIO_Pin: specifies the port bits to be written.
 * Returns : none
 * Note    : 
 * ----------------------------------------------------------------------------
 */
void GPIO_SetOneBit(GPIO_TypeDef* GPIOx, GPIO_PinTypeDef GPIO_Pin, U8_T value)
{
	if ( GPIOx == GPIOA ) // P0
	{
		switch ( GPIO_Pin ) 
		{
			case GPIO_Pin_0 :
				P0_0 = value;
				break;
			case GPIO_Pin_1 :
				P0_1 = value;
				break;
			case GPIO_Pin_2 :
				P0_2 = value;
				break;
			case GPIO_Pin_3 :
				P0_3 = value;
				break;
			case GPIO_Pin_4 :
				P0_4 = value;
				break;
			case GPIO_Pin_5 :
				P0_5 = value;
				break;
			case GPIO_Pin_6 :
				P0_6 = value;
				break;
			case GPIO_Pin_7 :
				P0_7 = value;
				break;
		}
	}
	else if ( GPIOx == GPIOB ) // P1
	{
		switch ( GPIO_Pin ) 
		{
			case GPIO_Pin_0 :
				P1_0 = value;
				break;
			case GPIO_Pin_1 :
				P1_1 = value;
				break;
			case GPIO_Pin_2 :
				P1_2 = value;
				break;
			case GPIO_Pin_3 :
				P1_3 = value;
				break;
			case GPIO_Pin_4 :
				P1_4 = value;
				break;
			case GPIO_Pin_5 :
				P1_5 = value;
				break;
			case GPIO_Pin_6 :
				P1_6 = value;
				break;
			case GPIO_Pin_7 :
				P1_7 = value;
				break;
		}
	}
	else if ( GPIOx == GPIOC ) // P2
	{
		switch ( GPIO_Pin ) 
		{
			case GPIO_Pin_0 :
				P2_0 = value;
				break;
			case GPIO_Pin_1 :
				P2_1 = value;
				break;
			case GPIO_Pin_2 :
				P2_2 = value;
				break;
			case GPIO_Pin_3 :
				P2_3 = value;
				break;
			case GPIO_Pin_4 :
				P2_4 = value;
				break;
			case GPIO_Pin_5 :
				P2_5 = value;
				break;
			case GPIO_Pin_6 :
				P2_6 = value;
				break;
			case GPIO_Pin_7 :
				P2_7 = value;
				break;
		}
	}
	else if ( GPIOx == GPIOD ) // P3
	{
		switch ( GPIO_Pin ) 
		{
			case GPIO_Pin_0 :
				P3_0 = value;
				break;
			case GPIO_Pin_1 :
				P3_1 = value;
				break;
			case GPIO_Pin_2 :
				P3_2 = value;
				break;
			case GPIO_Pin_3 :
				P3_3 = value;
				break;
			case GPIO_Pin_4 :
				P3_4 = value;
				break;
			case GPIO_Pin_5 :
				P3_5 = value;
				break;
			case GPIO_Pin_6 :
				P3_6 = value;
				break;
			case GPIO_Pin_7 :
				P3_7 = value;
				break;
		}
	}
} /* End of GPIO_SetOneBit */

/*
 * ----------------------------------------------------------------------------
 * void GPIO_GetPinValue 
 * Purpose : Get one single or mutiple bit of the selected data port.
 * Params  : GPIOx: where x can be (A..D) to select the GPIO peripheral.
 *           GPIO_Pin: specifies the port bits to be written.
 * Returns : none
 * Note    : 
 * ----------------------------------------------------------------------------
 */
U8_T GPIO_GetPinValue(GPIO_TypeDef* GPIOx, GPIO_PinTypeDef GPIO_Pin)
{
	U8_T p;	
	
	if ( GPIOx == GPIOA ) // P0
	{
		p = P0;
	}
    else if ( GPIOx == GPIOB ) // P1
	{
		p = P1;
	}
	else if ( GPIOx == GPIOC ) // P2
	{
		p = P2;
	}
	else if ( GPIOx == GPIOD ) // P3
	{
		p = P3;
	}
	
	return (p & GPIO_Pin);
}	

/* End of gpio.c */