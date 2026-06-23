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
 * Module Name: edid_hardware.c
 * Purpose: User hardware to implement the edid feature
 * Author:
 * Date:
 *=============================================================================
 */
 
/* INCLUDE FILE SECTION */
#include <stdio.h>
#include <string.h>

/* System Relate Header File Seciton */
#include "project_include.h"

#if (SYSTEM_HWEDID_CONTROL_ENABLE)
/* NAMING CONSTANT DECLARATIONS */
/* GLOBAL VARIABLES DECLARATIONS */

/* LOCAL VARIABLES DECLARATIONS */
/* LOCAL SUBPROGRAM DECLARATIONS */
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
/* EXTERNAL SUBPROGRAM DECLARATIONS */ 

/*
 * ----------------------------------------------------------------------------
 * Function Name: HW_EDID_Init(void)
 * Purpose: Initial the hardware EDID 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void HW_EDID_Init(void)
{
	U8_T int_reg[2];

	/* Pre-scale Clock */
	/* get system clock */
	switch (CPU_GetSysClk())
	{
		case SCS_48M :
			int_reg[0]= 0x00;
			int_reg[1]= I2C_STD_48M;
			break;
		case SCS_96M :
		default:
			int_reg[0]= 0x00;
			int_reg[1]= I2C_STD_96M;
			break;
	}
	
	I2C_RegWrite(I2CCPR,int_reg,2);
	int_reg[0] = 0x20;
	I2C_RegWrite(I2CSTOI,int_reg,1);

	/* Enable Master & AUto Detection Plug Mode */
	HW_EDID_PlugAutoDetectMode(0,ENABLE);
	
	/* Enable Slave EDID feature */
	HW_EDID_EnableSlave();
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: HW_EDID_PlugAutoDetectMode(void)
 * Purpose: Enable/Disable the Plug Auto Detect hardware feature
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void HW_EDID_PlugAutoDetectMode(U8_T dtime,FunctionalState control)
{
	U8_T int_reg;

	if (control == DISABLE)
	{
		int_reg = 0;
		I2C_RegWrite(I2CMCR,&int_reg, 1);
	}
	else
	{
		/* Setup Plug Auto Detected Time */
		int_reg = dtime; // default is 0->8ms
		I2C_RegWrite(I2CADPTI, &int_reg, 1);

		/* Setup I2C Master mode */
		/* Enable Master & AUto Detection Plug Mode */
		int_reg= I2CMCR_ADP_ENB;
		I2C_RegWrite(I2CMCR,&int_reg, 1);

		/* Setup I2C Master interrupt register */
		// Read back the interrupt setting value
		EXTINT4_DISABLE;
		_I2C_CIR_SFR(I2CMIER);
		_I2C_DR_READ_SFR(int_reg);
		EXTINT4_ENABLE;
		
		int_reg &= ~I2CMIER_RLEDIE; // stop ALE IE
		int_reg |= I2CMIER_PSCIE;   // Enable Autodect plug IE
		
		I2C_RegWrite(I2CMIER,&int_reg, 1);
	}
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: HW_EDID_ReloadAutoMode(void)
 * Purpose: Initial the hardware EDID 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void HW_EDID_ReloadAutoMode(void)
{
	U8_T int_reg;

	/* Setup I2C Master mode */
	/* Enable Master & AUto Detection Plug Mode */
	int_reg= I2CMCR_RLE_EDID;
	I2C_RegWrite(I2CMCR,&int_reg, 1);

	/* Setup I2C Master interrupt register */
	// Read back the interrupt setting value
	EXTINT4_DISABLE;
	_I2C_CIR_SFR(I2CMIER);
	_I2C_DR_READ_SFR(int_reg);
	EXTINT4_ENABLE;

	int_reg &= ~I2CMIER_PSCIE;	// stop auto plug 
	int_reg |= I2CMIER_RLEDIE;	// Enable ReloadIE

	I2C_RegWrite(I2CMIER,&int_reg, 1);
}

 /*
 * ----------------------------------------------------------------------------
 * Function Name: HW_EDID_EnableSlave(void)
 * Purpose: Initial the hardware EDID 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void HW_EDID_EnableSlave(void)
{
	U8_T int_reg;

	/* Setup I2C Master mode */
	/* Enable Master & Auto Detection Plug Mode */
#ifdef MCU_TYPE_AX68002	
	int_reg = 0x03;
#endif

#ifdef MCU_TYPE_AX68004	
	int_reg = 0x0F;
#endif	

	int_reg |= I2CSCR_SLV_GLITCH_FLT;

	I2C_RegWrite(I2CSCR,&int_reg, 1);
} 

/*
 *--------------------------------------------------------------------------------
 * void I2C_RegWrite(U8_T regAddr, U8_T *pRegData, regLen)
 * Purpose : Write the I2C interface indirectly through I2C's SFR.
 * Params  : regAddr - I2C register address.
 *           *pRegData - a pointer store the data.
 *           regLen - register length in byte count.
 * Returns : none
 * Note    : none
 *--------------------------------------------------------------------------------
 */
void I2C_RegWrite(U8_T regAddr, U8_T *pRegData, U8_T regLen)
{
	BIT oldEintBit = EINT4;

	EXTINT4_DISABLE;
	while (regLen --)
		I2CDR = *(pRegData + regLen);
	I2CCIR = regAddr;
	EINT4 = oldEintBit;
}

#endif // (SYSTEM_HWEDID_CONTROL_ENABLE)

/* End of edid_hardware.c */
