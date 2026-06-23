/*
 ************************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ************************************************************************************
 */
 
/*================================================================================
 * Module Name : kvm_rtc.c
 * Purpose     : handle real time clock
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
#include "project_include.h"

#ifdef RTC  
/*$GLOBAL DEFINATION DECLARATIONS*/
#define RTC_DEBUG_DISPLAY			1
/*$GLOBAL CONTANT DECLARATIONS*/
/*$GLOBAL VARIABLES DECLARATIONS*/
struct _KVM_RTCC KVM_Rtc;
struct _KVM_RTC_Alarm  KVM_RTC_Alarm;
//---------------------------------------------------------------------------------------
// GENERIC USB QUEUE
U8_T TASK_KVM_RTC_Handle_ID;
U8_T TASK_KVM_RTC_Handle_ActiveID;
//---------------------------------------------------------------------------------------
/*$STATIC VARIABLE DECLARATIONS  */
/*$LOCAL SUBPROGRAM DECLARATIONS */
void KVM_RTC_Alarm_Handle(void);
void TASK_KVM_RTC_Handle(void);

/*
 * -------------------------------------------------------------------------------
 * void KVM_RTC_Init(void)
 * Purpose : Initializes the rtc control 
 *       
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */
void KVM_RTC_Init(void)
{
	memset(&KVM_Rtc,0x00,sizeof(KVM_Rtc));
	TASK_KVM_RTC_Handle_ActiveID = 0;
	//1.Genteric HID device init declartion
	TASK_KVM_RTC_Handle_ID  = TASK_Create(TASK_KVM_RTC_Handle);		
	 
	//After 4 hours need to reset extender transmitter
	KVM_RTC_Alarm.iCountdown = DEFAULT_RTC_TIMER; // 4 housrs, unit is minutes	
	KVM_RTC_Alarm.iReload = KVM_RTC_Alarm.iCountdown;
	KVM_RTC_Alarm.Task_FunctionP = KVM_RTC_Alarm_Handle;
	
} /* End of KVM_RTC_Init */

/***************************************************************************************************/
/*
**  Function Name: KVM_RTC_Control(void)
**  Parameter    : 
**  Description  : 
**
**
*/
void KVM_RTC_Control(void)
{
	if (KVM_Flash.cSystemFlag2 & SYSTEM_RTC_MASK)
		KVM_RTC_Start();
	else
		KVM_RTC_Stop();
}	

/***************************************************************************************************/
/*
**  Function Name: KVM_RTC_Start(void)
**  Parameter    : 
**  Description  : 
**
**
*/
void KVM_RTC_Start(void)
{	
	//printf("RTC Start\n\r");
	if (KVM_Flash.cSystemFlag2 & SYSTEM_RTC_MASK)
	{
		KVM_RTC_Alarm.iCountdown = DEFAULT_RTC_TIMER;
		if (TASK_KVM_RTC_Handle_ActiveID == 0)
		{	
			//printf("RTC GO\n\r");
			TASK_KVM_RTC_Handle_ActiveID = TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_KVM_RTC_Handle_ID,0,0,1,1000)+1; //for every sec active the real time clock					
		}	
		else
		{			
			Task_Active_Table[TASK_KVM_RTC_Handle_ActiveID-1].Task_Interval.w = 1000;
		}			
	}		
}

/***************************************************************************************************/
/*
**  Function Name: KVM_RTC_Start(void)
**  Parameter    : 
**  Description  : 
**
**
*/
void KVM_RTC_Stop(void)
{	
	//printf("RTC Stop\n\r");
	if ((KVM_Flash.cSystemFlag2 & SYSTEM_RTC_MASK) == 0)
	{
		if (TASK_KVM_RTC_Handle_ActiveID)
		{
			Task_Active_Table[TASK_KVM_RTC_Handle_ActiveID-1].Task_Interval.w = 1;			
		}		
	}		
}

/***************************************************************************************************/
/*
**  Function Name: TASK_KVM_RTC_Handle_ID(void)
**  Parameter    : 
**  Description  : 
**
**
*/
void TASK_KVM_RTC_Handle(void)
{
	if ((KVM_Flash.cSystemFlag2 & SYSTEM_RTC_MASK) == 0)
	{
		TASK_Destory_Current();
		TASK_KVM_RTC_Handle_ActiveID = 0;
		return;
	}
	
	if (KVM_Rtc.cSecond < 59) //
	{
		KVM_Rtc.cSecond++;
	}		
	else
	{
		KVM_Rtc.cSecond = 0;
		if (KVM_Rtc.cMinute < 59)
		{
			KVM_Rtc.cMinute++;
		}			
		else
		{
			KVM_Rtc.cMinute=0;
			if (KVM_Rtc.cHour < 23)
			{
				KVM_Rtc.cHour++;
			}				
			else
			{
				KVM_Rtc.cHour = 0;				
			}				
		}
		
		//Alarm minitue handle
		if (KVM_RTC_Alarm.iCountdown)
		{	
			KVM_RTC_Alarm.iCountdown--;
		}
		
		if (KVM_RTC_Alarm.iCountdown == 0)		
		{
			KVM_RTC_Alarm.iCountdown = KVM_RTC_Alarm.iReload;
			if (KVM_RTC_Alarm.Task_FunctionP)
				KVM_RTC_Alarm.Task_FunctionP();
		}	
		
#if (RTC_DEBUG_DISPLAY)
		printf("%02bu:%02bu:%02bu\n\r",KVM_Rtc.cHour,KVM_Rtc.cMinute,KVM_Rtc.cSecond);
#endif 		
	}		
}

/***************************************************************************************************/
/*
**  Function Name: KVM_RTC_Alarm_Handle(void)
**  Parameter    : 
**  Description  : 
**
**
*/
void KVM_RTC_Alarm_Handle(void)
{
	
}
#endif /* End of RTC */

/* End of kvm_rtc.c */
