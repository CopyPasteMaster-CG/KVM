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
 * Module Name: hardware_ps2.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __HARDWARE_PS2_H__
#define __HARDWARE_PS2_H__

/* INCLUDE FILE DECLARATIONS */
#include "system_cfg.h"
/* DEFINATION DECLARATIONS */
#define PS2A_CR					0x00	// PS2A Control Register
#define PS2A_SR					0x01	// PS2A Status Register
#define PS2A_DR					0x02	// PS2A Data Register
#define PS2B_CR					0x04	// PS2B Control Register
#define PS2B_SR					0x05	// PS2B Status Register
#define PS2B_DR					0x06	// PS2B Data Register
#define PS2_DIVR				0x10	// Inhibit Interval Divider Register
#define PS2_TRSFTIMOR			0x11	// Transfer Time-out Register
#define PS2_RESPTIMOR			0x12	// Response Clock Time-out Register
#define PS2_WSR					0x13	// PS2 Wakeup Status Register

/* PS2A_CR & PS2B_CR */
#define PS2CR_RX_BUSY			BIT0
#define PS2CR_CLK_HIGH			BIT1
#define PS2CR_HOST_ENB			BIT2
#define PS2CR_RX_IE				BIT3
#define PS2CR_TX_ACT			BIT4
#define PS2CR_INH_FORCE			BIT5
#define PS2CR_INH_RELEASE		0
#define PS2CR_WE				BIT6
#define PS2CR_TX_IE				BIT7

/* PS2A_SR & PS2B_SR */
#define PS2SR_RX_STATE_MASK		0x03
#define PS2SR_RX_COMPLETE		0
#define PS2SR_RX_TIME_OUT		BIT0
#define PS2SR_RX_PARITY_ERR		BIT1
#define PS2SR_RX_FORMAT_ERR		(BIT1|BIT0)
#define PS2SR_RX_INTR			BIT3

#define PS2SR_TX_STATE_MASK		0x30
#define PS2SR_TX_COMPLETE		0
#define PS2SR_RSP_TIME_OUT		BIT4
#define PS2SR_TX_TIME_OUT		BIT5
#define PS2SR_TX_INTR			BIT7

/* PS2_WSR */
#define PS2WSR_PS2A_WKUP		BIT0
#define PS2WSR_PS2B_WKUP		BIT1

/* NAMING CONSTANT DECLARATIONS */
 
/* GLOBAL VARIABLES */ 
extern PS2_ISR_TypeDef	HPS2_ISR_FIFO[HPS2_ISR_FIFO_DEPTH_MAX];
extern U8_T		HPS2_ISR_FIFO_WP,PS2_ISR_FIFO_RP;
extern U8_T		TASK_HW_PS2_ISR_HANDLE_ID;
extern U8_T		HPS2_CR_BK[SYSTEM_HARDWARE_PS2_PORT_MAX];

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void HW_PS2_Transmit(PS2Port_TypeDef PS2x,U8_T ps2data);
void HW_PS2_Init(void);
void HW_PS2_Enable(PS2Port_TypeDef PS2x);
//void HW_PS2_Disable(PS2Port_TypeDef PS2x);
void HW_PS2_ISR_Handle(void); 
#endif /* End of __EXTI_H__ */

/* End of exti.h */