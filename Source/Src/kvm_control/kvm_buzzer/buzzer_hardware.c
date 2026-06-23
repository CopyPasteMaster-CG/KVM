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
 * Module Name: buzzer_hardware.c
 * Purpose: User hardeare to control the buzzer
 * Author:
 * Date:
 * Modify :
 *          1.2013-04-17  Add Buffer Stop Register design
 *=============================================================================
 */

/* INCLUDE FILE SECTION */
#include <stdio.h>
#include <string.h>
#include "project_include.h"

#if (KVM_BUZZER_HARDWARE)
/* NAMING CONSTANT DECLARATIONS */
code Buzzer_ControlTypeDef	Btn_Active_Sound[] =
{
	BUZZER_250_hz,BUZZER_125_ms,130,
	BUZZER_Terminate,0,0
};

code Buzzer_ControlTypeDef	Hotkey_Active_Sound[] =
{
	BUZZER_250_hz,BUZZER_125_ms,130,
	BUZZER_Terminate,0,0
};

code Buzzer_ControlTypeDef	Hotkey_Stop_Sound[] =
{
	BUZZER_250_hz,BUZZER_125_ms,130,
	BUZZER_Terminate,0,0
};

code Buzzer_ControlTypeDef	Error_Sound[] =
{
	BUZZER_250_hz,BUZZER_125_ms,130,
	BUZZER_Silence,BUZZER_125_ms,50,
	BUZZER_250_hz,BUZZER_125_ms,130,
	BUZZER_Terminate,0,0
};

code Buzzer_ControlTypeDef	AutoScan_Sound_Start[] =
{
	BUZZER_2_Khz,BUZZER_125_ms,130,
	BUZZER_Silence,BUZZER_125_ms,100,
	BUZZER_250_hz,BUZZER_125_ms,130,
	BUZZER_Terminate,0,0
};

code Buzzer_ControlTypeDef	AutoScan_Sound_Stop[] =
{
	BUZZER_250_hz,BUZZER_125_ms,130,
	BUZZER_Silence,BUZZER_125_ms,100,
	BUZZER_2_Khz,BUZZER_125_ms,130,
	BUZZER_Terminate,0,0
};

code Buzzer_ControlTypeDef	PortSwitch_Sound[] =
{
	BUZZER_2_Khz,BUZZER_125_ms,130,
	BUZZER_Terminate,0,0
};

code Buzzer_ControlTypeDef	DeviceErrorSound[] =
{
	BUZZER_2_Khz,BUZZER_125_ms,1000,
	BUZZER_Terminate,0,0
};

code Buzzer_ControlTypeDef MSC_Switch_Sound[] =
{
	BUZZER_2_Khz,BUZZER_125_ms,200,
	BUZZER_Terminate,0,0
};

code Buzzer_ControlTypeDef MSC_Switch_Error[] =
{
	BUZZER_250_hz,BUZZER_125_ms,130,
	BUZZER_Silence,BUZZER_125_ms,100,
	BUZZER_250_hz,BUZZER_125_ms,130,
	BUZZER_Silence,BUZZER_125_ms,100,
	BUZZER_250_hz,BUZZER_125_ms,130,
	BUZZER_Silence,BUZZER_125_ms,100,
	BUZZER_Terminate,0,0
};

code Buzzer_ControlTypeDef	DeviceOverCurrentSound[] =
{
	BUZZER_2_Khz,BUZZER_1_Secs,1000,
	BUZZER_Silence,BUZZER_125_ms,500,
	BUZZER_Terminate,0,0
};

#if (SYSTEM_EXTENDER_RECEIVER)
code Buzzer_ControlTypeDef	Extender_Plug_In[] =
{
	BUZZER_2_Khz,BUZZER_250_ms,100,
	BUZZER_Silence,BUZZER_125_ms,150,	
	BUZZER_500_hz,BUZZER_250_ms,100,
	BUZZER_Terminate,0,0
};

code Buzzer_ControlTypeDef	Extender_Plug_Out[] =
{
	BUZZER_500_hz,BUZZER_250_ms,100,	
	BUZZER_Silence,BUZZER_125_ms,150,
	BUZZER_2_Khz,BUZZER_250_ms,100,		
	BUZZER_Terminate,0,0
};
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */

/* GLOBAL VARIABLES DECLARATIONS */
bit			Buzzer_Script_Start;
bit			BUZZER_OnFlag; 	//Buzzer active flag
bit			BUZZER_Type; 	//Buzzer control type,1-Sound,0-Silent
bit			bBUZZER_ForceFlag;
Buzzer_ControlTypeDef	*Buzzer_Current_Script;
U8_T		Buzzer_Index;
U8_T		BCR_BK,BIR_BK;
U8_T		BUZZER_Cnt; 	//Buzzer Counter
U8_T		TASK_BUZZER_SCRIPT_ID;
U16_T		BUZZER_Period,BUZZER_Timer;


/* LOCAL VARIABLES DECLARATIONS */
/* LOCAL SUBPROGRAM DECLARATIONS */
void TASK_BUZZER_Script(void);
static void BUZZER_Set_Freqency_Time(U8_T frequency,U8_T time);
static void BUZZER_Control(U8_T);
static void Buzzer_Script_Parser(Buzzer_ControlTypeDef *script,U8_T btn_index);
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
/* EXTERNAL SUBPROGRAM DECLARATIONS */
 
/*
 * ----------------------------------------------------------------------------
 * Function Name: KVM_BUZZER_Init(void)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void KVM_BUZZER_Init(void)
{
	U8_T	interrupt4;

#if (INTR_ISR_INCLUDE_BUZZER)
	BCR_BK = BUZZER_500_hz | BUZZER_125_ms;
	BIR_BK = BUZZER_IE_SET;
#else
	BCR_BK = BUZZER_500_hz | BUZZER_125_ms;
	BIR_BK = 0;
#endif

	EXTINT5_DISABLE;
	interrupt4 = EINT4; // keep the orginal value
	EXTINT4_DISABLE;

	_MISC_DR_SFR(BCR_BK);
	_MISC_CIR_SFR(MISC_BCR);

	_MISC_DR_SFR(BIR_BK);
	_MISC_CIR_SFR(MISC_BISR);

	EXTINT5_ENABLE;
	EINT4 = interrupt4;

	BUZZER_OnFlag = 0; 
	bBUZZER_ForceFlag = 0;

	BUZZER_Control(BUZZER_ON);
	Buzzer_Script_Start = 0;

	TASK_BUZZER_SCRIPT_ID = TASK_Create(TASK_BUZZER_Script);
	
#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("TASK_BUZZER_SCRIPT_ID=%bu\n\r",TASK_BUZZER_SCRIPT_ID);
#endif	
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: BUZZER_Set_Freqency_Time(U8_T frequency,U8_T time)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void BUZZER_Set_Freqency_Time(U8_T frequency,U8_T time)
{
	U8_T	interrupt4;

	EXTINT5_DISABLE;
	interrupt4 = EINT4; // keep the orginal value
	EXTINT4_DISABLE;

	BCR_BK &= ~(BUZZER_KHZ_MASK|BUZZER_PERIOD_MASK);

	BCR_BK |= (frequency| time);

	_MISC_DR_SFR(BCR_BK);
	_MISC_CIR_SFR(MISC_BCR);

	EXTINT5_ENABLE;
	EINT4 = interrupt4;
}
/*
 * ----------------------------------------------------------------------------
 * Function Name: BUZZER_Control(U8_T)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void BUZZER_Control(U8_T state)
{
	U8_T interrupt4;

	EXTINT5_DISABLE;
	interrupt4 = EINT4; // keep the orginal value
	EXTINT4_DISABLE; 

	if (state == BUZZER_ON)
	{
		BCR_BK |= BUZZER_SE_MASK;
		BCR_BK &= ~BUZZER_TSE_SET;
	}
	else
	{
		BCR_BK |= BUZZER_TSE_SET;
		BCR_BK &= ~BUZZER_SE_MASK;
	}
	_MISC_DR_SFR(BCR_BK);
	_MISC_CIR_SFR(MISC_BCR);

	EXTINT5_ENABLE;
	EINT4 = interrupt4;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: Buzzer_Script_Parser(U8_T)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void Buzzer_Script_Parser(Buzzer_ControlTypeDef *script,U8_T btn_index)
{
	if ((script+btn_index)->Frequency == BUZZER_Terminate)
	{
		Buzzer_Script_Start = 0;
		return;
	}
	if ((script+btn_index)->Frequency != BUZZER_Silence)
	{
		#if (KVM_BUZZER_TYPE_Frequency)	
			BUZZER_Set_Freqency_Time((script+btn_index)->Frequency,(script+btn_index)->Period);	//zbb
		#else		
			BUZZER_Set_Freqency_Time(0x00,(script+btn_index)->Period);
		#endif	
	
		BUZZER_Control(BUZZER_ON);
	}
	else
	{
		BUZZER_Control(BUZZER_OFF);
	}
	TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_BUZZER_SCRIPT_ID,0,0,(script+btn_index)->Keeptime,(script+btn_index)->Keeptime); // now aticev the process for time out event
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: BUZZER_Script_Active(U8_T)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void BUZZER_Script_Active(Buzzer_ControlTypeDef *script)
{
	//1.Check Buzzer active condition
	if (bBUZZER_ForceFlag == 0)
	{
		if ((KVM_Flash.cSystemFlag0 & SYSTEM_BEEPER_MASK) == 0)
		{
			return;
		}
	}

	if (Buzzer_Script_Start)
	{
		return;
	}

	Buzzer_Index = 0;
	Buzzer_Current_Script = script;

	Buzzer_Script_Start = 1;
	Buzzer_Script_Parser(Buzzer_Current_Script,Buzzer_Index);
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: TASK_BUZZER_Script(void)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void TASK_BUZZER_Script(void)
{
	BUZZER_Control(BUZZER_OFF);
	Buzzer_Index++;
	Buzzer_Script_Parser(Buzzer_Current_Script,Buzzer_Index);
	TASK_Destory_Current();
}

#endif

/* End of buzzer_hardware.c */
