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
 * Module Name : mstimer.c
 * Purpose     : AX6800x provides a ms timer. This firmware can initial the ms-timer
 *               and get the time tick by applications.
 * Author      : 
 * Date        :
 * Notes       : 
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "ax6800x.h"
#include "types.h"
#include "interrupt.h"
#include "mstimer.h"
#include "dma.h"

/* NAMING CONSTANT DECLARATIONS */

/* GLOBAL VARIABLES DECLARATIONS */
idata U8_T	SWTIMER_Counter = 0;
idata U8_T	SWTIMER_ScCheckTimer256ms;
U8_T		SWTIMER_ScCheckTimerMinutes;
U8_T		SWTIMER_ScCheckTimerHours;

/* LOCAL VARIABLES DECLARATIONS */

/*
 * ----------------------------------------------------------------------------
 * void SWTIMER_Init(void)
 * Purpose: Initiate global values in the MS timer module.
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void SWTIMER_Init(void)
{
	SWTIMER_Counter = 0;
	SWTIMER_ScCheckTimer256ms = 0;
	SWTIMER_ScCheckTimerMinutes = 0;
	SWTIMER_ScCheckTimerHours = 0;

} /* End of SWTIMER_Init */

/*
 * ----------------------------------------------------------------------------
 * void SWTIMER_Start(void)
 * Purpose: Enable the interrupt and start software timer.
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void SWTIMER_Start(void)
{
	U8_T XDATA	temp[2];

	/* set software timer */
	temp[1] = (U8_T)SWTIMER_INTERVAL;
	temp[0] = (U8_T)(SWTIMER_INTERVAL >> 8) + SWTIMER_START + SWTIMER_INT_ENABLE;
	DMA_RegWrite(SWTIMER_REG, temp, 2);

	EXTINT5_ENABLE;	/* Enable INT5 interrupt for software timer. */

} /* End of SWTIMER_Start */

/* End of mstimer.c */