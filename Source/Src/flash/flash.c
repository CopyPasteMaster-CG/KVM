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
 * Module Name : flash.c
 * Purpose     :
 * Author      : 
 * Date        : 
 * Notes       : Flash driver to read/write flash in IAP. In all API functions,
 *               the Flash address is byte address and will be converted to word
 *               address in the API function.
 *================================================================================
 */
/* INCLUDE FILE DECLARATIONS */
#include	<absacc.h>
#include	"project_include.h"

#if (SYSTEM_STORAGE_FLASH)
/* GLOBAL VARIABLE DECLARATIONS */
U8_T	flashIntrStatus = 0;
U8_T	flashIntrEnbFlag = 0;

/* STATIC VARIABLE DECLARATIONS */
/* LOCAL SUBPROGRAM DECLARATIONS */
static bit flash_isr;

/* LOCAL SUBPROGRAM BODIES */

/* EXPORTED SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * void FLASH_Setup(void)
 * Purpose : Flash interrupt server routine for programming and erasing.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void FLASH_Setup(void)
{
	flashIntrEnbFlag = 0;
	FLASH_RegWrite(FLH_FCCR2, &flashIntrEnbFlag, 1);
}

/*
 *--------------------------------------------------------------------------------
 * void FLASH_RegWrite(U8_T addr, U8_T *ptData, U8_T dataLen)
 * Purpose: Write data bytes into a register with interrupt locked.
 * Params : addr: an index address to Flash register.
 *          ptData: a pointer to indicate the register data.
 *          dataLen:A number of bytes to indicate how many bytes will be written.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void FLASH_RegWrite(U8_T addr, U8_T *ptData, U8_T dataLen)
{
	flash_isr = EA;
	EA = 0;
	while (dataLen --)
	{
		FDR = *(ptData + dataLen);
	}
	FCIR = addr;
	EA = flash_isr;
}

/*
 *--------------------------------------------------------------------------------
 * void FLASH_RegRead(U8_T addr, U8_T *ptData, U8_T dataLen)
 * Purpose: Read data bytes from a register with interrupt locked.
 * Params : addr: an index address to Flash register.
 *          ptData: a pointer to indicate the register data.
 *          dataLen:A number of bytes to indicate how many bytes will be read.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void FLASH_RegRead(U8_T addr, U8_T *ptData, U8_T dataLen)
{
	flash_isr = EA;
	EA = 0;
	FCIR = addr;
	while (dataLen --)
	{
		*(ptData + dataLen) = FDR;
	}

	EA = flash_isr;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL FLASH_CheckCmdComplete(void)
 * Purpose : Check the Flash status after triggering a Flash access.
 * Params  : 
 * Returns : TRUE (1) is success ; FALSE (0) is failure with error.
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL FLASH_CheckCmdComplete(void)
{
	U8_T	reg8b;

	/* Check the Flash status */
	while (1)
	{
		FLASH_RegRead(FLH_FCSR, &reg8b, 1);
		if (reg8b & FCSR_FC_CS)
			break;
	}
	if (reg8b & FCSR_ERROR_OCCURRED)
		return FALSE;
	
	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL FLASH_SectorErase(void)
 * Purpose : Erase the flash for a sector.
 * Params  : sectAddr: flash address that is a multiple of 0x2000 (8K).
 *           sectSize: total sectors to be erased.
 * Returns : TRUE (1) is success ; FALSE (0) is failure with error.
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL FLASH_SectorErase(void)
{
	U16_T	addrRealFlash;
	U8_T	reg8b;

	/* Convert the byte mode address to the word mode address.
	/* A word address 0x0000 indicates a byte address 0x0000~0x0007, 0x0001 for 0x0008~0x000F. */
	addrRealFlash = (STORAGE_START_ADDRESS / FLASH_WORD_LEN);
	FLASH_RegWrite(FLH_FAR, (U8_T *)&addrRealFlash, 2);
	
	/* Trigger the GO bit to erase Flash sector */
	reg8b = (FCCR1_FC_GO | FCCR1_FCMD_SECTOR_ERASE);
	FLASH_RegWrite(FLH_FCCR1, &reg8b, 1);
	/* Check the Flash status */
	if (FLASH_CheckCmdComplete() == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL FLASH_InfoWordRead8Byte(U32_T addr, U8_T *pData)
 * Purpose : Flash interrupt server routine for programming and erasing.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL FLASH_InfoWordRead8Byte(U32_T addr, U8_T *pData)
{
	U16_T	i, addrRealFlash;
	U8_T	reg8b;

	/* Check the Flash address must be a word mode address (1 word = 8 bytes) */
	if (addr & 0x07)
	{
		return FALSE;
	}
	
	/* Convert the byte mode address to the word mode address.
	/* A word address 0x0000 indicates a byte address 0x0000~0x0007, 0x0001 for 0x0008~0x000F. */
	addrRealFlash = (U16_T)(addr / FLASH_WORD_LEN);
	FLASH_RegWrite(FLH_FAR, (U8_T *)&addrRealFlash, 2);
	
	/* Trigger the GO bit to read Flash */
	reg8b = (FCCR1_FC_GO | FCCR1_FCMD_WORD_READ | FCCR1_INFO_PAGE);
	FLASH_RegWrite(FLH_FCCR1, &reg8b, 1);
	
	/* Check the Flash status */
	if (FLASH_CheckCmdComplete() == FALSE)
	{
		return FALSE;
	}

	/* Fill a page data 256 bytes into the Flash HW buffer */
	flash_isr = EA;
	EA = 0;
	for (i = 0 ; i < FLASH_WORD_LEN ; i ++)
	{
		*(pData + i) = FCDP;
	}
	EA = flash_isr;
	return TRUE;
}
#endif

/* End of flash.c */