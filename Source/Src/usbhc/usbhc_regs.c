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
 * Module Name : usbhc_regs.c
 * Purpose     :
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	<string.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	"project_include.h"

/* STATIC VARIABLE DECLARATIONS */ 
/* LOCAL SUBPROGRAM DECLARATIONS */
/* LOCAL SUBPROGRAM BODIES */
 
/************************************************************************************************
* Function Name  : USBHC_Write_Regs(U8_T hccir,U8_T *hcdr,U8_T len)
* Description    : Write Regs
* Input          : None.
* Output         : None.
* Return         : None.
*************************************************************************************************/
void USBHC_Write_Regs(U8_T hccir,U8_T *hcdr,U8_T len)
{
	U8_T index;
	U8_T int2_bk;

	int2_bk = EINT2;
	EXTINT2_DISABLE;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	EXTINT3_DISABLE;
#endif //#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	for (index=0; index < len ;index++)
	{
		_USBHC_HCDR_SFR(hcdr[index]);
	}
	_USBHC_HCCIR_SFR(hccir);
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	EXTINT3_ENABLE;
#endif
	EINT2 = int2_bk;
}

#if (USBHC_INIT_DEBUG_MODE)
/************************************************************************************************
* Function Name  : USBHC_Read_Regs(U8_T hccir,U8_T *hcdr,U8_T len)
* Description    : Read Regs
* Input          : None.
* Output         : None.
* Return         : None.
*************************************************************************************************/
void USBHC_Read_Regs(U8_T hccir,U8_T *hcdr,U8_T len)
{
	U8_T	index;
	U8_T	int2_bk;

	int2_bk = EINT2;
	EXTINT2_DISABLE;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	EXTINT3_DISABLE; /* disable USBDC interrupt */
#endif //#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	_USBHC_HCCIR_SFR(hccir);
	for (index=0; index < len ;index++)
	{
		_USBHC_HCDR_READ_SFR(hcdr[index]);
	}
	EINT2 = int2_bk;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	EXTINT3_ENABLE;
#endif //#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
}

#if (USBHC_INIT_DEBUG_MODE)
/************************************************************************************************
* Function Name  : USBHC_Reg_Dump(U8_T hccir,U8_T len)
* Description    : Read Regs
* Input          : None.
* Output         : None.
* Return         : None.
*************************************************************************************************/
void USBHC_Reg_Dump(U8_T hccir,U8_T length)
{
	U8_T	content[16],index;

	USBHC_Read_Regs(hccir,content,length);
	//printf("HC_DUMP,Reg(0x%02x)=[",(U16_T)hccir);
	for (index=0; index < length ; index++)
	{
		if (index <(length-1))
			printf("%02x ",(U16_T)content[index]);
		else
			printf("%02x]\n\r",(U16_T)content[index]);
	}
}
#endif /* #if (USBHC_INIT_DEBUG_MODE) */
#endif

/* End of usbhc_regs.c */


