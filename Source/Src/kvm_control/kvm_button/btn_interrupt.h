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
 * Module Name : btn_interrupt.h
 * Purpose     :
 * Author      :
 * Date        :
 * Notes       : 
 * no message
 *
 *================================================================================
 */
#ifndef __BTN_INTERRUPT_H
#define __BTN_INTERRUPT_H

/* INCLUDE FILE DECLARATIONS */


/* GLOBAL VARIABLES */
#ifdef BTN_RESET	
#define BTN_RESET_WAIT_TIME			 5000
#endif
extern bit		BTN_Processing_Flag;
extern U8_T		TASK_BTN_Polling_ID;
extern U8_T		TASK_BTN_Event_ID,TASK_BTN_Event_Active_ID;
extern const    U8_T	BTN_PORT_SFR[];
extern const 	GPIO_PinTypeDef	BTN_POLLING_SHIFT[];

//daniel
#define PORT0_ACTIVE 0x0
#define PORT1_ACTIVE 0x1
#define PORT2_ACTIVE 0x2
#define PORT3_ACTIVE 0x3

#define PORT12_ACTIVE 0x4
#define PORT34_ACTIVE 0x5
#define PORT_ALL_ACTIVE 0x8
#define PORT_ALL_SYNC 0x0F




/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void KVM_BTN_Init(void);
void TASK_BTN_Event_Control(void);
void TASK_BTN_Polling(void);

#endif /* End of __BTN_POLLING_H */




