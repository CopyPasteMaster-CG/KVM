/*
 ******************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation All rights reserved.
 *
 *     This is an proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
 
 /*============================================================================
 * Module Name: usbdc_regs.c
 * Purpose: USB DC hardware configuration library
 * Author:
 * Date:
 * Notes: 
 *=============================================================================
 */
 
/* Includes ------------------------------------------------------------------*/
#include "project_include.h"

/* $ Private typedef -----------------------------------------------------------*/
const U8_T DA_EP_CFGR[USBDC_DEVICE_MAX] =
{
	0x00,//0x02,0x04,0x06,0x08,0x0a,0x0c,0x0e,
	0x10,//0x12,0x14,0x16,0x18,0x1a,0x1c,0x1e,
	0x20,//0x22,0x24,0x26,0x28,0x2a,0x2c,0x2e,
	0x30,//0x32,0x34,0x36,0x38,0x3a,0x3c,0x3e,
	0x40,//0x42,0x44,0x46,0x48,0x4a,0x4c,0x4e,
	0x50,//0x52,0x54,0x56,0x58,0x5a,0x5c,0x5e,
	0x60,//0x62,0x64,0x66,0x68,0x6a,0x6c,0x6e,
	0x70 //0x72,0x74,0x76,0x78,0x7a,0x7c,0x7e
};
	
const U8_T DA_ID_CFGR[USBDC_DEVICE_MAX] =
{
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87
};

const U8_T DA_CR[USBDC_DEVICE_MAX] =
{
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97
};
	
const U8_T DA_EP_SR[USBDC_DEVICE_MAX][USBDC_ENDP_MAX] =
{
	0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,
	0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
	0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,
	0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
	0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,
	0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
	0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,
	0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
};
/* $ Private define ------------------------------------------------------------*/
/* $ Private macro -------------------------------------------------------------*/
/* $ Private variables ---------------------------------------------------------*/
/* $ Extern variables ----------------------------------------------------------*/
/* $ Private function prototypes -----------------------------------------------*/

/* $$ Private functions Body ---------------------------------------------------*/
/*******************************************************************************
* Function Name  : USBDC_REGS_Get_Pkt_Size(U8_T ep_iso,U16_T epsize)
* Description    : Write Regs
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
U8_T USBDC_REGS_Get_Pkt_Size(U8_T ep_iso,U16_T epsize)
{
	if (ep_iso == DA_EP_CFGR_ISOCH_SET)
	{
		if (epsize <= 16)
			return ISO_ENDP_PKT_SIZE_16;
		else if(epsize <=64)	
			return ISO_ENDP_PKT_SIZE_64;
		else if(epsize <=128)
			return ISO_ENDP_PKT_SIZE_128;
		else if(epsize <=192)
			return ISO_ENDP_PKT_SIZE_192;
	}
	else
	{
		if (epsize <= 8)
			return ENDP_PKT_SIZE_8;
		else if(epsize <=16)	
			return ENDP_PKT_SIZE_16;
		else if(epsize <=32)	
			return ENDP_PKT_SIZE_32;
		else if(epsize <=64)	
		return ENDP_PKT_SIZE_64;
	}

	return ENDP_PKT_SIZE_8;
}

/************************************************************************************************
* Function Name  : USBDC_Write_Regs(U8_T port,U8_T dccir,U8_T *dcdr,U8_T len)
* Description    : Write Regs
* Input          : None.
* Output         : None.
* Return         : None.
*************************************************************************************************/
void USBDC_Write_Regs(U8_T port,U8_T dccir,U8_T *dcdr,U8_T len)
{
	U8_T index;
	U8_T int3_bk;
	
	int3_bk = EINT3;
#if (SYSTEM_MSC_DEVICE_SUPPORT)
	EXTINT5_DISABLE;
#endif
	
	USBDC_INT_DISABLE;
	
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	EXTINT2_DISABLE;
#endif
	switch (port)
	{
		case 0:
			for (index=0; index < len ;index++)
				_USBDC_DC0DR_SFR(dcdr[index]);
			
			_USBDC_DC0CIR_SFR(dccir);
			break;
		case 1:
			for (index=0; index < len ;index++)
				_USBDC_DC1DR_SFR(dcdr[index]);

			_USBDC_DC1CIR_SFR(dccir);
			break;
		case 2:
			for (index=0; index < len ;index++)
				_USBDC_DC2DR_SFR(dcdr[index]);

			_USBDC_DC2CIR_SFR(dccir);
			break;
		case 3:
			for (index=0; index < len ;index++)
				_USBDC_DC3DR_SFR(dcdr[index]);

			_USBDC_DC3CIR_SFR(dccir);
			break;
	}
	
	/*Enable the HC interrupt */
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	EXTINT2_ENABLE;
#endif
	
	EINT3 = int3_bk;
	
#if (SYSTEM_MSC_DEVICE_SUPPORT)
	EXTINT5_ENABLE;
#endif
}

/************************************************************************************************
* Function Name  : USBDC_Read_Regs(U8_T port,U8_T endpx,U8_T dccir,U8_T *dcdr,U8_T len)
* Description    : Write Regs
* Input          : None.
* Output         : None.
* Return         : None.
*************************************************************************************************/
void USBDC_Read_Regs(U8_T port,U8_T dccir,U8_T *dcdr,U8_T len)
{
	U8_T index;
	U8_T int3_bk;
	
	int3_bk = EINT3;
#if (SYSTEM_MSC_DEVICE_SUPPORT)
	EXTINT5_DISABLE;	   
#endif
	
	USBDC_INT_DISABLE;
	
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	EXTINT2_DISABLE;		   
#endif
	switch (port)
	{
		case 0:
			_USBDC_DC0CIR_SFR(dccir);
			for (index=0; index < len ;index++)
				_USBDC_DC0DR_READ_SFR(dcdr[index]);
			break;
		case 1:
			_USBDC_DC1CIR_SFR(dccir);
			for (index=0; index < len ;index++)
				_USBDC_DC1DR_READ_SFR(dcdr[index]);
			break;
		case 2:
			_USBDC_DC2CIR_SFR(dccir);
			for (index=0; index < len ;index++)
				_USBDC_DC2DR_READ_SFR(dcdr[index]);
			break;
		case 3:
			_USBDC_DC3CIR_SFR(dccir);
			for (index=0; index < len ;index++)
				_USBDC_DC3DR_READ_SFR(dcdr[index]);
			break;
	}
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	EXTINT2_ENABLE;
#endif
	
	EINT3 = int3_bk;
	
#if (SYSTEM_MSC_DEVICE_SUPPORT)
	EXTINT5_ENABLE;
#endif
}


/*******************************************************************************
* Function Name  : USBDC_IDCFGR(U8_T port,U8_T devinx,U8_T address)
* Description    : Assing the device Address
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USBDC_REGS_Address_Write(U8_T port,U8_T devinx,U8_T address)
{
	U8_T reg;
	U8_T cir;
	
	cir = DA_ID_CFGR[devinx];
	USBDC_Read_Regs(port,cir,&reg,1);
	
	reg &= ~DA_ID_CFGR_DEVADR_MASK;
	reg |= address;
	
	USBDC_Write_Regs(port,cir,&reg,1);
}

/*******************************************************************************
* Function Name  : USBDC_REGS_Address_Active(U8_T port,U8_T devinx,U8_T address)
* Description    : Assing the device Address
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USBDC_REGS_Address_Active(U8_T port,U8_T devinx,FlagStatus state)
{
	U8_T reg;
	U8_T cir;
	
	cir = DA_ID_CFGR[devinx]; 
	USBDC_Read_Regs(port,cir,&reg,1);
	
	if (state == RESET)
		reg &= ~DA_ID_CFGR_ADRACT_MASK;
	else
		reg |= DA_ID_CFGR_ADRACT_MASK;
	
	USBDC_Write_Regs(port,cir,&reg,1);
#if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST))
#if (SYSTEM_EXTENDER_TRANSMITTER)
	if ((state == SET) && (devinx == EXTENDER_MSC_Devinx))
#else
	if ((state == SET) && (devinx == USBHC_MSC_Devinx))
#endif //#if (SYSTEM_EXTENDER_TRANSMITTER)
	{
		cir = DA_CR[devinx];
		reg = DA_CR_BCLR_SET + USB_PDevice[devinx].MSC->Out_Endpinx;
		USBDC_Write_Regs(port,cir,&reg,1);
	}
#endif
}

/*******************************************************************************************
* Function Name  : USBDC_EPCFGR_Write(U8_T port,U8_T devinx,U8_T ep,U8_T reg0,U8_T reg1)
* Description    : Configures the USB hardware
* Input          : None.
* Output         : None.
* Return         : None.
********************************************************************************************/
void USBDC_EPCFGR_Write(U8_T port,U8_T devinx,U8_T ep,U8_T reg0,U8_T reg1)
{
	U8_T cir;
	U8_T dr[2];
	
	dr[0] = reg0;
	dr[1] = reg1;
	cir = DA_EP_CFGR[devinx] + (ep << 1);
	USBDC_Write_Regs(port,cir,dr,2);
	
#if (USBDC_DEBUG_MODE)
	USBDC_Reg_Dump(port,cir,2);
#endif
}

#if (USBDC_DEBUG_MODE)
/*
* @brief  USBDC_Reg_Dump
*         USB Host core main state machine process
* @param  None 
* @retval None
*/
void USBDC_Reg_Dump(U8_T port,U8_T address,U8_T length)
{
	U8_T  content[16],index;

	USBDC_Read_Regs(port,address,content,length);

	printf("DUMP[%d],Reg(0x%02x)=[",(U16_T)port,(U16_T)address);
	for (index=0; index < length ; index++)
	{
		if (index <(length-1))
			printf("%02x ",(U16_T)content[index]);
		else	
			printf("%02x]\n\r",(U16_T)content[index]);
	}
}
#endif

/* ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 * void USBDC_REGS_Endp_ControlClear(U8_T port,U8_T devinx,U8_T endpinx,U8_T flag)
 * Purpose : When Endp has get a Out transaction, then get the buffer content
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */
void USBDC_REGS_Endp_ControlClear(U8_T port,U8_T devinx,U8_T endpinx,U8_T flag)
{
	idata U8_T reg;
	idata U8_T stall_flag = 0x00;
	
	//1.Get Endpoint Stall Status (0xa0)
	if (endpinx)
	{
		USBDC_Read_Regs(port,DA_EP_SR[devinx][endpinx],&reg,1);
		if (reg & DA_EP_STALL) // if the endp is stalled
		{
			stall_flag = DA_CR_STALL_SET;
		}
	}
	
	//2.Update the content
	reg = endpinx | stall_flag;
	
	if (flag & DA_CR_STALL_SET)
		reg &= ~DA_CR_STALL_SET;

	if (flag & DA_CR_SCLR_MASK)
		reg |= DA_CR_SCLR_MASK;
	
	if (flag & DA_CR_BCLR_MASK)
		reg |= DA_CR_BCLR_MASK;
	
	//3.Write back the register
	USBDC_Write_Regs(port,DA_CR[devinx],&reg,1);
} 

/* ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 * void USBDC_REGS_Endp_ControlSet(U8_T port,U8_T devinx,U8_T endpinx,U8_T flag)
 * Purpose : When Endp has get a Out transaction, then get the buffer content
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */
void USBDC_REGS_Endp_ControlSet(U8_T port,U8_T devinx,U8_T endpinx,U8_T flag)
{
	U8_T reg;
	U8_T stall_flag = 0x00;

	//1.Get Control Register content_0x90
	if (endpinx)
	{
		USBDC_Read_Regs(port,DA_EP_SR[devinx][endpinx],&reg,1);
		if (reg & DA_EP_STALL) // if the endp is stalled
		{
			stall_flag = DA_CR_STALL_SET;
		}
	}
	
	//2.Update the content
	reg = (flag | (endpinx &0x07) | stall_flag);
	//3.Write back the register
	USBDC_Write_Regs(port,DA_CR[devinx],&reg,1);
} 

/* ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 * void USBDC_REGS_Endp_Reset(U8_T port,U8_T devinx,U8_T endpinx)
 * Purpose :
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */
void USBDC_REGS_Endp_Reset(U8_T port,U8_T devinx,U8_T endpinx)
{
	U8_T reg;
	
	reg = endpinx | DA_CR_SCLR_MASK | DA_CR_BCLR_MASK;
	USBDC_Write_Regs(port,DA_CR[devinx],&reg,1);
}

/* ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 * void USBDC_REGS_Device_Reset(U8_T port,U8_T devinx)
 * Purpose : 
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */
void USBDC_REGS_Device_Reset(U8_T port,U8_T devinx)
{
	U8_T reg;
	
	reg = 0x00;
	USBDC_Write_Regs(port,DA_ID_CFGR[devinx],&reg,1);
}

/* End of usbdc_regs.c */
