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


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void KVM_BTN_Init(void);
void TASK_BTN_Event_Control(void);
void TASK_BTN_Polling(void);

#endif /* End of __BTN_POLLING_H */




