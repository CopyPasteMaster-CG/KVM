 /*
 ******************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*=============================================================================
 * Module Name: gpio.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __GPIO_H__
#define __GPIO_H__

/* INCLUDE FILE DECLARATIONS */


/* NAMING CONSTANT DECLARATIONS */
/** @defgroup GPIO_pins_define 
  * @{
  */

/* define the GPIO Port * GPIO Pin data byte
   this should be 8/16 bits width */
typedef U8_T GPIO_PinTypeDef;

/* define the GPIO Port width value */
#define GPIO_PORT_WIDTH					8

/* define the GPIO Pin bit value 
   this should be 8/16 bits width */
#define GPIO_Pin_0					((GPIO_PinTypeDef)0x01)  /*!< Pin 0 selected */
#define GPIO_Pin_1					((GPIO_PinTypeDef)0x02)  /*!< Pin 1 selected */
#define GPIO_Pin_2					((GPIO_PinTypeDef)0x04)  /*!< Pin 2 selected */
#define GPIO_Pin_3					((GPIO_PinTypeDef)0x08)  /*!< Pin 3 selected */
#define GPIO_Pin_4					((GPIO_PinTypeDef)0x10)  /*!< Pin 4 selected */
#define GPIO_Pin_5					((GPIO_PinTypeDef)0x20)  /*!< Pin 5 selected */
#define GPIO_Pin_6					((GPIO_PinTypeDef)0x40)  /*!< Pin 6 selected */
#define GPIO_Pin_7					((GPIO_PinTypeDef)0x80)  /*!< Pin 7 selected */
#define GPIO_Pin_8					((GPIO_PinTypeDef)0x0100)  /*!< Pin 8 selected */
#define GPIO_Pin_9					((GPIO_PinTypeDef)0x0200)  /*!< Pin 9 selected */
#define GPIO_Pin_10					((GPIO_PinTypeDef)0x0400)  /*!< Pin 10 selected */
#define GPIO_Pin_11					((GPIO_PinTypeDef)0x0800)  /*!< Pin 11 selected */
#define GPIO_Pin_12					((GPIO_PinTypeDef)0x1000)  /*!< Pin 12 selected */
#define GPIO_Pin_13					((GPIO_PinTypeDef)0x2000)  /*!< Pin 13 selected */
#define GPIO_Pin_14					((GPIO_PinTypeDef)0x4000)  /*!< Pin 14 selected */
#define GPIO_Pin_15					((GPIO_PinTypeDef)0x8000)  /*!< Pin 15 selected */
#define GPIO_Pin_All				((GPIO_PinTypeDef)0xFFFF)  /*!< All pins selected */

/* DATA STRUCTURE */

typedef U8_T  data GPIO_TypeDef;


/* GLOBAL VARIABLES */
#define GPIOA_BASE				(0x80)  // for 8051 P0 address
#define GPIOB_BASE				(0x90)  // for 8051 P1 address
#define GPIOC_BASE				(0xa0)  // for 8051 P2 address
#define GPIOD_BASE				(0xb0)  // for 8051 P3 address

#define GPIOA				((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB				((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC				((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD				((GPIO_TypeDef *) GPIOD_BASE)

#define PORT0				((GPIO_TypeDef *) GPIOA_BASE)
#define PORT1				((GPIO_TypeDef *) GPIOB_BASE)
#define PORT2				((GPIO_TypeDef *) GPIOC_BASE)
#define PORT3				((GPIO_TypeDef *) GPIOD_BASE)


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void  GPIO_SetOneBit(GPIO_TypeDef* GPIOx, GPIO_PinTypeDef GPIO_Pin,U8_T value);		/* Sets one single bit of the selected data port */
U8_T  GPIO_GetPinValue(GPIO_TypeDef* GPIOx, GPIO_PinTypeDef GPIO_Pin);
#endif /* End of __GPIO_H__ */

/* End of gpio.h */