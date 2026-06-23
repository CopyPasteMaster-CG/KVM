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
/*=============================================================================
 * Module Name:timer.h
 * Purpose: Asix 22xxx timer control(Timer0,Timer1,Timer2)
 * Author:
 * Date:
 * Notes:
 * $Log: mstimer.h,v $
 * no message
 *
 *=============================================================================
 */

#ifndef __IR_H__
#define __IR_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"



/* NAMING CONSTANT DECLARATIONS */
#define IR_IDLE				0x00
#define IR_START			0x01
#define IR_WAIT_HIGH		0x02
#define IR_END				0x80
#define IR_PIN_MASK			0x40
#define IR_TRIGGER_LEVEL_SHIFT		4
#define IR_GPIO_PIN			P0_6

/* GLOBAL VARIABLES */
extern volatile U8_T  IR_Control_State;
extern volatile U8_T  IR_MS_Cnt;
extern volatile U8_T  IR_Plus_Cnt;
extern volatile U8_T  IR_Buffer_Cnt;
extern volatile U8_T  IR_Buffer[8];
extern U8_T  IR_Edge_Setting[2];
extern volatile U16_T IR_Plus_Timer0;
extern volatile U16_T IR_Plus_Timer1;
extern volatile U16_T IR_Plus_Timer_Dif;
extern U16_T IR_1MS_Counter,IR_2MS_Counter;


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void KVM_IR_Init(void);
void KVM_IR_Key_Handle(void);
//---------------------------------------------------------------
#endif /* End of __IR_H__ */


/* End of timer.h */