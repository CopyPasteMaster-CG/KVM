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
 * Module Name : buzzer_hardware.h
 * Purpose     : 
 * Author      : Jack Wang
 * Date        :
 * Notes       : 
 * no message
 *
 *================================================================================
 */
#ifndef __BUZZER_HARDWARE_H
#define __BUZZER_HARDWARE_H

/* INCLUDE FILE DECLARATIONS */

/* NAMING CONSTANT DECLARATIONS */
#define BUZZER_ON					0
#define BUZZER_OFF					1
#define BUZZER_KHZ_MASK				0x07 // shkim mod 16.01.29 clear error fix
#define BUZZER_0_Khz				0x00
#define BUZZER_125_hz				0x01
#define BUZZER_250_hz				0x02
#define BUZZER_500_hz				0x03
#define BUZZER_1_Khz				0x04
#define BUZZER_2_Khz				0x05
#define BUZZER_4_Khz				0x06
#define BUZZER_8_Khz				0x07
#define BUZZER_Silence				0xf0
#define BUZZER_Terminate			0xff

#define BUZZER_PERIOD_MASK			0x38
#define BUZZER_2_Secs				0x00
#define BUZZER_1_Secs				0x08
#define BUZZER_500_ms				0x10
#define BUZZER_250_ms				0x18
#define BUZZER_125_ms				0x20


#define BUZZER_IE_MASK				0x10
#define BUZZER_IE_SET				0x10
#define BUZZER_IE_RESET				0x00

#define BUZZER_TSE_MASK				0x40
#define BUZZER_TSE_SET				0x40
#define BUZZER_TSE_RESET			0x00

#define BUZZER_SE_MASK				0x80
#define BUZZER_SE_SET				0x80
#define BUZZER_SE_RESET				0x00

typedef struct _Buzzer_ControlTypeDef
{
	U8_T	Frequency; // Interface address
	U16_T	Period; // TD Number
	U16_T	Keeptime; // TD Number
} Buzzer_ControlTypeDef;

/* MACRO DECLARATIONS */


/* GLOBAL VARIABLES */
extern code		Buzzer_ControlTypeDef Btn_Active_Sound[];
extern code		Buzzer_ControlTypeDef Hotkey_Active_Sound[];
extern code		Buzzer_ControlTypeDef Error__Sound[];
extern code		Buzzer_ControlTypeDef PortSwitch_Sound[];
extern code		Buzzer_ControlTypeDef AutoScan_Sound_Start[];
extern code		Buzzer_ControlTypeDef AutoScan_Sound_Stop[];
extern code		Buzzer_ControlTypeDef Hotkey_Stop_Sound[];
extern code		Buzzer_ControlTypeDef DeviceErrorSound[];
extern code		Buzzer_ControlTypeDef MSC_Switch_Sound[];
extern code		Buzzer_ControlTypeDef MSC_Switch_Error[];
extern code		Buzzer_ControlTypeDef Error_Sound[];
extern code		Buzzer_ControlTypeDef DeviceOverCurrentSound[];
#if (SYSTEM_EXTENDER_RECEIVER)
extern code 	Buzzer_ControlTypeDef Extender_Plug_In[];
extern code 	Buzzer_ControlTypeDef Extender_Plug_Out[];
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void BUZZER_Script_Active(Buzzer_ControlTypeDef *script);
void KVM_BUZZER_Init(void);

#endif /* End of __BUZZER_GPIO_H */



