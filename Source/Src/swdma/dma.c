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
 * Module Name : dma.c
 * Purpose     : Provides a DMA that is used by software.
 *               This module handles all DMA functions 
 * Author      : 
 * Date        :
 * Notes       : 
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "ax6800x.h"
#include "types.h"
#include "dma.h"

/* GLOBAL VARIABLE DECLARATIONS */
/* STATIC VARIABLE DECLARATIONS */
static bit dma_isr;

/* LOCAL SUBPROGRAM BODIES */

/*
 * ----------------------------------------------------------------------------
 * void DMA_RegRead(U8_T regaddr, U8_T XDATA* pbuf, U8_T length)
 * Purpose: Read SW DMA register.
 * Params : regaddr: A register index address.
 *          pbuf: A pointer to indicate the register data.
 *          length: How many bytes will be read. 
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void DMA_RegRead(U8_T regaddr, U8_T XDATA* pbuf, U8_T length)
{
	dma_isr = EA;
	EA = 0;
	DCIR = regaddr;
	while (length--)
		pbuf[length] = DDR;
	EA = dma_isr;

} /* End of DMA_ReadReg */

/*
 * ----------------------------------------------------------------------------
 * void DMA_RegWrite(U8_T regaddr, U8_T XDATA* pbuf, U8_T length)
 * Purpose: Write SW DMA register.
 * Params : regaddr: A register index address.
 *          pbuf: A pointer to indicate the register data.
 *          length: How many bytes will be written. 
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void DMA_RegWrite(U8_T regaddr, U8_T XDATA* pbuf, U8_T length)
{
	dma_isr = EA;
	EA = 0;
	while (length--)
		DDR = pbuf[length];
	DCIR = regaddr;
	EA = dma_isr;

} /* End of DMA_RegWrite */

/*
 * ----------------------------------------------------------------------------
 * void DMA_Start(void)
 * Purpose: Start software-dma module by enabling the interrupt 5.
 * Params :
 * Returns:
 * Note   :
 * ----------------------------------------------------------------------------
 */
//void DMA_Start(void)
//{
//	EINT5 = 1;
//} /* End of DMA_Start */

/*
 * ----------------------------------------------------------------------------
 * BOOL DMA_CheckStatus(void)
 * Purpose: Wait software-dma to complete, and check if dma error happened or not?
 * Params : None.
 * Returns: 0 : dma error had happened.
 *			1 :	dma had completed.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
BOOL DMA_CheckStatus(void)
{
	U8_T XDATA	temp;

	while (1)
	{
		DMA_RegRead(SW_DMA_STATUS_REG, &temp, 1);
		if (temp & DMA_ERROR_STU)
		{
			return FALSE;
		}
		else if (temp & DMA_COMPLETE_STU)
			break;
	}
	return TRUE;

} /* End of DMA_CheckStatus */

/*
 * ----------------------------------------------------------------------------
 * BOOL DMA_DataToProgram(U32_T addrDest, U32_T addrSour, U16_T length)
 * Purpose: This function is responsible to copy data from external data memory
 *          to program memory. It will check DMA status automatically
 *          after DMA begin copying.
 * Params : addrDest¡GThe destination address in program memory.
 *          addrSour¡GThe source address in external data memory.
 *          length¡GThe total length that S/W DMA has need to copy.
 * Returns: 0 : DMA error had happened
 *			1 :	DMA had completed.
 * Note   :
 * ----------------------------------------------------------------------------
 */
BOOL DMA_DataToProgram(U32_T addrDest, U32_T addrSour, U16_T length)
{
	U32_T	cmd32b = 0;
	U8_T	cmd8b = 0;

    if (length == 0)
        return FALSE;

	cmd32b = (U32_T)addrSour << 8;
	DMA_RegWrite(SW_DMA_SOURCE_ADDR_REG, (U8_T *)&cmd32b, 3);

	cmd32b = (U32_T)addrDest << 8;
	DMA_RegWrite(SW_DMA_TARGET_ADDR_REG, (U8_T *)&cmd32b, 3);

	DMA_RegWrite(SW_DMA_BYTE_COUNT_REG, (U8_T *)&length, 2);

	cmd8b = (DMA_CMD_GO | DMA_TA_IN_PROG_MEM);
	DMA_RegWrite(SW_DMA_CMD_REG, (U8_T *)&cmd8b, 1);

	return (DMA_CheckStatus());
}

//#if (SYSTEM_EXTENDER_SUPPORT)
/*
 * ----------------------------------------------------------------------------
 * Function Name: DMA_GrantXdata
 * Purpose: Use sw-dma doing memory copy. The scope of source and destination
 *				target must	be smaller than 64k sram memory.
 *			If the scope of either source or destination target is large than
 *				64k memory,	the programmers can call DMA_Grant() instead of
 *				this function.
 *			DMA_GrantXdata() can run faster than DMA_Grant() function.
 * Params: 
 * Returns: A address of XDATA memory that point to the next address of
 *			the end address of the scope of destination target.
 * Note:
 * ----------------------------------------------------------------------------
 */
BOOL DMA_GrantXdata(U8_T XDATA* pdest, U8_T XDATA* psour, U16_T length)
{
	volatile U8_T XDATA	temp[3];
	U16_T XDATA*	 	ptempshort = (U16_T XDATA*)&temp[1];
//	U8_T 				dmatype = 0;
	U8_T				bitEa, result;

	if (!length || pdest == psour)
	{
		return FALSE;
	}

	bitEa = EA;
	EA = OFF;

	/* set source address indirect register */
	*ptempshort = psour;
	DDR = temp[2];
	DDR = temp[1];
	DDR = 0; /* temp[0] is always 0 */
	DCIR = SW_DMA_SOURCE_ADDR_REG;

	/* set target address indirect register */
	*ptempshort = pdest;
	DDR = temp[2];
	DDR = temp[1];
	DDR = 0; /* temp[0] is always 0 */
	DCIR = SW_DMA_TARGET_ADDR_REG;

	/* set length */
	*ptempshort = length;
	DDR = temp[2];
	DDR = temp[1];
	DCIR = SW_DMA_BYTE_COUNT_REG;

	DDR = DMA_CMD_GO;
	DCIR = SW_DMA_CMD_REG;

	/* check software dma completed. */
	result = DMA_CheckStatus();
	EA = bitEa;
	
	return (result);
	
} /* DMA_GrantXdata */
//#endif /* #if (SYSTEM_EXTENDER_SUPPORT) */
/* End of dma.c */

