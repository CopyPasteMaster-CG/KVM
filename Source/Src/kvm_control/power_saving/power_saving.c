/*
 ******************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is an proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
 /*============================================================================
 * Module Name: power_saving.c
 * Purpose: KVM Power saving mode control program
 * Author:
 * Date:
 *=============================================================================
 */

/* INCLUDE FILE SECTION */
#include <stdio.h>
#include <string.h>
#include "project_include.h"


#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
/* NAMING CONSTANT DECLARATIONS */
/* GLOBAL VARIABLES DECLARATIONS */
bit	PowerSavingStartFlag;
bit	PowerSavingInhibitFlag;
bit	PowerSavingExitFlag;
U8_T	PowerSavingExitSate;
U8_T	TASK_KVM_PowerSaving_Check_ID;
U8_T	TASK_KVM_PowerSaving_Start_ID;

/* LOCAL VARIABLES DECLARATIONS */

/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
extern U8_T	TASK_USBHC_Resume_Command_ID;


/* LOCAL SUBPROGRAM DECLARATIONS */
void TASK_KVM_PowerSaving_Start(void);
void TASK_KVM_PowerSaving_Check(void);

/* EXTERNAL SUBPROGRAM DECLARATIONS */

/*
 * ----------------------------------------------------------------------------
 * Function Name: KVM_PowerSaving_Init(void)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void KVM_PowerSaving_Init(void)
{
	PowerSavingStartFlag = 0;
	PowerSavingExitFlag = 0;
	PowerSavingExitSate = 0;
	PowerSavingInhibitFlag = 0;

	TASK_KVM_PowerSaving_Check_ID = TASK_Create(TASK_KVM_PowerSaving_Check);
	TASK_KVM_PowerSaving_Start_ID = TASK_Create(TASK_KVM_PowerSaving_Start);
	TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_KVM_PowerSaving_Check_ID,0,0,SYSTEM_POWER_SAVING_TIMER,SYSTEM_POWER_SAVING_TIMER); //after 100ms, start power on all port
#if (POWER_SAVING_DEBUG_MODE)
	if (KVM_Flash.PowerSavingMode)
	{
		printf("\n\r> Default Power Saving Mode = %s\n\r", (KVM_Flash.PowerSavingMode == SYSTEM_POWER_STOP_MODE) ? "STOP" : "DEEP SLEEP");
	}
	else
	{
		printf("\n\r> Default Power Saving Mode = OFF\n\r");
	}
#endif

#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("TASK_KVM_PowerSaving_Check_ID=%bu\n\r",TASK_KVM_PowerSaving_Check_ID);
	printf("TASK_KVM_PowerSaving_Start_ID=%bu\n\r",TASK_KVM_PowerSaving_Check_ID);
#endif
}

/*--------------------------------------------------------------------------------
 * void TASK_KVM_PowerSaving_Check(void)
 * Purpose : Check weither get into power saving mode
 * Params  : None 
 * Returns : None
 * Note    : The length can not large than 0xff
 *--------------------------------------------------------------------------------
 */
void TASK_KVM_PowerSaving_Check(void)
{
	if (PowerSavingInhibitFlag)
	{
		PowerSavingInhibitFlag = 0;
	}
	else
	{
		if ((KVM_Flash.cSystemFlag1 & SYSTEM_POWERSAVING_MASK) == 0) //if not support powersaving mode
		{
			return;
		}	
		/* Need to Check DC Suspend Status */
		if (USBDC_PortSuspendCheck() == 0)
		{
			return;
		}
#if (SYSETM_CONSOLE_EDID_CONTROL_ENABLE)
		/* Need to check I2C bus status is whether idle or not */
		if (EDID_ctrl.TaskState != CONSOLE_MONITOR_PLUG_PAUSED)
		{
			EDID_ctrl.TaskPauseRequest = 1;//Pause any communication on I2C bus
			return;
		}
#endif
		if (PowerSavingStartFlag == 0)
		{
#if (POWER_SAVING_DEBUG_MODE2) || (POWER_SAVING_DEBUG_MODE)
			printf("\n\r");
			printf("> *******************************************\n\r");
			if (KVM_Flash.PowerSavingMode)
			{
				printf("> * Power Saving Mode(%s) Start... \n\r", (KVM_Flash.PowerSavingMode == SYSTEM_POWER_STOP_MODE) ? "STOP" : "DEEP SLEEP");
			}
			else
			{
				printf("> * Power Saving Mode(OFF) Start...\n\r");
			}
			printf("> *******************************************\n\r");
#endif
			PowerSavingStartFlag = 1;
			USBHC_CORE_Suspend();
			TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_KVM_PowerSaving_Start_ID,0,0,200,200); //after 100ms, start power on all port
		}
	}
}

/*--------------------------------------------------------------------------------
 * void TASK_KVM_PowerSaving_Start(void)
 * Purpose : Enter Low Power Mode
 * Params  : None 
 * Returns : None
 * Note    : The length can not large than 0xff
 *--------------------------------------------------------------------------------
 */
void TASK_KVM_PowerSaving_Start(void)
{
	if (PowerSavingInhibitFlag)
	{
		PowerSavingStartFlag = 0;
#if (POWER_SAVING_DEBUG_MODE)
		printf(">Power Saving Mode Stop by interrupt...\n\r");
#endif
	}
	else
	{
		CPU_SetPowerSaveMode(KVM_Flash.PowerSavingMode);
#if (SYSETM_CONSOLE_EDID_CONTROL_ENABLE)
		/* Clear EDID puase flag to continue the process of EDID task */
		EDID_ctrl.TaskPauseRequest = 0;
#endif
	}
	TASK_Destory_Current();
}

/*--------------------------------------------------------------------------------
 * void KVM_PowerSaving_Exit(void)
 * Purpose : Check weither get into power saving mode
 * Params  : None 
 * Returns : None
 * Note    : The length can not large than 0xff
 *--------------------------------------------------------------------------------
 */
void KVM_PowerSaving_Exit(void)
{
	EXTINT6_DISABLE;
	PowerSavingExitFlag = 0;
#if (POWER_SAVING_DEBUG_MODE)
	printf("INT6 incoming, state = %02bx\n\r", PowerSavingExitSate);
#endif
	EXTINT6_ENABLE;
	
#if (POWER_SAVING_DEBUG_MODE2) || (POWER_SAVING_DEBUG_MODE)
	if (PowerSavingStartFlag)
	{
		if (KVM_Flash.PowerSavingMode)
		{
			printf("> Power Saving Mode(%s)Exit by Event:%02x\n\r", (KVM_Flash.PowerSavingMode == SYSTEM_POWER_STOP_MODE) ? "STOP" : "DEEP SLEEP", (U16_T)PowerSavingExitSate);
		}
		else
		{
			printf("> Power Saving Mode(OFF)Exit by Event:%02x\n\r", (U16_T)PowerSavingExitSate);
		}
	}
#endif
	
	if ((PowerSavingExitSate & UHC_WKUP) == 0) // wake up from other interface
	{
#if (POWER_SAVING_DEBUG_MODE)
		printf("Resume HC\n\r");
#endif
		USBHC_CORE_Resmue();
	}
	TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_Resume_Command_ID,0x00,USBDC_VIRTUAL_DEVINX,0,0); //Generate the task, for next Event check
	PowerSavingStartFlag = 0;
}

#endif //#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)

/* End of power_saving.c */
