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
 * Module Name:mstimer.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */

#ifndef __MSTIMER_H__
#define __MSTIMER_H__

/* INCLUDE FILE DECLARATIONS */
#include "mstimer_cfg.h"


/* NAMING CONSTANT DECLARATIONS */
#define SWTIMER_COUNT_SECOND	(1000 / SWTIMER_INTERVAL) /* timer expired count per second*/

/* SFR 0x94 definitions*/
#define SWTIMER_EXPIRED			BIT1

/* TIMER register definitions */
#define SWTIMER_REG				0x0C

/* Bit definitions: SWTIMER_REG */
#define SWTIMER_START			BIT4
#define SWTIMER_RESET			BIT5
#define SWTIMER_INT_ENABLE		BIT7

/* GLOBAL VARIABLES */
extern idata U8_T	SWTIMER_Counter;
extern bit			SWTIMER_Flag;
extern idata U8_T	SWTIMER_ScCheckTimer256ms;
extern U8_T			SWTIMER_ScCheckTimerMinutes;
extern U8_T			SWTIMER_ScCheckTimerHours;


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
//---------------------------------------------------------------
void SWTIMER_Init(void);
void SWTIMER_Start(void);
#endif /* End of __MSTIMER_H__ */


/* End of mstimer.h */