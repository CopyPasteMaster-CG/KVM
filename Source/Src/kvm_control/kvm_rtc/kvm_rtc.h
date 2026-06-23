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
 * Module Name : kvm_rtc.h
 * Purpose     : A header file of real time clock
 * Author      : Jack Wang
 * Date        :
 * Notes       : 
 * no message
 *
 *================================================================================
 */
#ifndef __KVM_RTC_H
#define __KVM_RTC_H

/* INCLUDE FILE DECLARATIONS */

/* NAMING CONSTANT DECLARATIONS */
#define  DEFAULT_RTC_TIMER	2 * 60  // 2 Hours 
 
/* MACRO DECLARATIONS */

/* TYPE DECLARATIONS */
/*----------------------------------------------------------------------------------------------*/
struct _KVM_RTCC {
	U16_T iYear;
	U8_T  cMonth;
	U8_T  cDay;
	U8_T  cHour;
	U8_T  cMinute;
	U8_T  cSecond;
};


struct _KVM_RTC_Alarm {
	U16_T iCountdown;
	U16_T iReload;
	void (* Task_FunctionP)(void); 	// Task Function Pointer
};

/*
$ Macro Function
*/

/* GLOBAL VARIABLES */


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void KVM_RTC_Init(void);
void KVM_RTC_Control(void);
void KVM_RTC_Start(void);
void KVM_RTC_Stop(void);
#endif /* End of __KVM_RTC_H */

