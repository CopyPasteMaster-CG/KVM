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
 * Module Name : spi_flash.c
 * Purpose     : This module handles the SPI flash interface driver.
 * Author      : 
 * Date        :
 * Notes       :
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	<stdio.h>
#include	"project_include.h"
#include	"spim.h"
#include	"spi_flash.h"

#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)

/* STATIC VARIABLE DECLARATIONS */

/* GLOBAL VARIABLE DECLARATIONS */
FlashDescriptor		spiflsh_DescTable[1];
BOOL				spiflsh_IsFlashErase;

/* LOCAL SUBPROGRAM DECLARATIONS */
void	spiflsh_InitMxFlashDescriptor(U8_T flashId);

//static void	spiflsh_ReadID(U16_T *vendorId, U16_T *productId);
static void	spiflsh_WriteEnable(U8_T flashId);
static U8_T	spiflsh_ReadStatus(U8_T flashId);
static void	spiflsh_ReadData(U8_T flashId, U32_T flshStartAddr, U8_T* dmaRxPt, U16_T rxLen);
static void	spiflsh_WriteData(U8_T flashId, U32_T flshStartAddr, U8_T* dmaTxPt, U16_T txLen);
static void	spiflsh_SectorErase(U8_T flashId, U32_T flshStartAddr);
static BOOL	spiflsh_IsWriteInProcess(U8_T flashId);

/* LOCAL SUBPROGRAM BODIES */
/*
 *--------------------------------------------------------------------------------
 * void	initMxFlashDescriptor(U8_T flashId)
 * Purpose : Request a RDID command to MX25Lxxxx SPI flash for manufacturer ID and 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void	spiflsh_InitMxFlashDescriptor(U8_T flashId)
{
	U8_T	i;
	
	spiflsh_DescTable[flashId].wVendorId = MX8006_VENDOR_ID;
	spiflsh_DescTable[flashId].wProductId = MX8006_PRODUCT_ID;
	spiflsh_DescTable[flashId].wMaxSectorNum = MX8006_SECTOR_NUM;
	spiflsh_DescTable[flashId].bSectorSize = MX8006_SECTOR_SIZE;
	

	spiflsh_DescTable[flashId].opTable[SPI_WREN].bOpCodeLen = 1;
	spiflsh_DescTable[flashId].opTable[SPI_WREN].opCode[0] = MX8006_WREN;

	spiflsh_DescTable[flashId].opTable[SPI_RDSR].bOpCodeLen = 1;
	spiflsh_DescTable[flashId].opTable[SPI_RDSR].opCode[0] = MX8006_RDSR;

	spiflsh_DescTable[flashId].opTable[SPI_READ].bOpCodeLen = 4;
	spiflsh_DescTable[flashId].opTable[SPI_READ].opCode[0] = MX8006_RD;

	spiflsh_DescTable[flashId].opTable[SPI_WRITE].bOpCodeLen = 4;
	spiflsh_DescTable[flashId].opTable[SPI_WRITE].opCode[0] = MX8006_PP;

	spiflsh_DescTable[flashId].opTable[SPI_SE].bOpCodeLen = 4;
	spiflsh_DescTable[flashId].opTable[SPI_SE].opCode[0] = MX8006_SE;

	for (i = SPI_SE + 1; i < MAX_OP_SUPPORT; i++)
	{
		spiflsh_DescTable[flashId].opTable[i].bOpCodeLen = 0;
		spiflsh_DescTable[flashId].opTable[i].opCode[0] = 0;
	}

	spiflsh_DescTable[flashId].bStatusTable[FLASH_WIP] = MX8006_WIP;
	spiflsh_DescTable[flashId].bStatusTable[FLASH_WEL] = MX8006_WEL;
	
	for (i = FLASH_WEL + 1; i < MAX_FLASH_STATUS; i++)
	{
		spiflsh_DescTable[flashId].bStatusTable[i] = 0;
	}
}

/*
 *--------------------------------------------------------------------------------
 * spiflsh_ReadID(U16_T *vendorId, U16_T *productId)
 * Purpose : Request a RDID command to MX25Lxxxx SPI flash for manufacturer ID and product ID
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
//static void	spiflsh_ReadID(U16_T *vendorId, U16_T *productId)
//{
//	U8_T	readBuf[3];
//	U8_T	opCode = MX8006_RDID;
//
//	while (!SPI_IsMstSpiBusRdy()){}
//
//	SPI_SetOpCodeReg(&opCode, 1);
//	SPI_MstDmaRx(readBuf, 3, (SDCR_ETDMA_IDL | SDCR_ERDMA_EXC | SDCR_OPC_ENB | SDCR_DMA_GO));
//
//	while (!SPI_IsMstSpiBusRdy()){}
//
//	*vendorId = readBuf[0];
//	*productId = ((readBuf[1]<<8) | readBuf[2]);
//}

/*
 *--------------------------------------------------------------------------------
 * spiflsh_WriteEnable(U8_T flashId)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void	spiflsh_WriteEnable(U8_T flashId)
{
	while (!SPI_IsMstSpiBusRdy()){}

	SPI_SetOpCodeReg(&(spiflsh_DescTable[flashId].opTable[SPI_WREN].opCode[0]), spiflsh_DescTable[flashId].opTable[SPI_WREN].bOpCodeLen);
	SPI_MstSendOpOnly();

	while (!SPI_IsMstSpiBusRdy()){}
}


/*
 *--------------------------------------------------------------------------------
 * spiflsh_ReadStatus(U8_T flashId)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static U8_T	spiflsh_ReadStatus(U8_T flashId)
{
	U8_T	readStatus;

	while (!SPI_IsMstSpiBusRdy()){}

	SPI_SetOpCodeReg(&(spiflsh_DescTable[flashId].opTable[SPI_RDSR].opCode[0]), spiflsh_DescTable[flashId].opTable[SPI_RDSR].bOpCodeLen);
	SPI_MstDmaRx (&readStatus, 1, (SDCR_ETDMA_IDL | SDCR_ERDMA_EXC | SDCR_OPC_ENB | SDCR_DMA_GO));

	while (!SPI_IsMstSpiBusRdy()){}

	return readStatus;
}

/*
 *--------------------------------------------------------------------------------
 * spiflsh_ReadData(U8_T flashId, U32_T flshStartAddr, U8_T* dmaRxPt, U16_T rxLen)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void	spiflsh_ReadData(U8_T flashId, U32_T flshStartAddr, U8_T* dmaRxPt, U16_T rxLen)
{
	spiflsh_DescTable[flashId].opTable[SPI_READ].opCode[1] = (U8_T)(flshStartAddr >> 16);
	spiflsh_DescTable[flashId].opTable[SPI_READ].opCode[2] = (U8_T)(flshStartAddr >> 8);
	spiflsh_DescTable[flashId].opTable[SPI_READ].opCode[3] = (U8_T)(flshStartAddr);

	while (!SPI_IsMstSpiBusRdy()){}

	SPI_SetOpCodeReg(&(spiflsh_DescTable[flashId].opTable[SPI_READ].opCode[0]), spiflsh_DescTable[flashId].opTable[SPI_READ].bOpCodeLen);
	SPI_MstDmaRx (dmaRxPt, rxLen, (SDCR_ETDMA_IDL | SDCR_ERDMA_EXC | SDCR_OPC_ENB | SDCR_DMA_GO));

	while (!SPI_IsMstSpiBusRdy()){}
}

/*
 *--------------------------------------------------------------------------------
 * spiflsh_WriteData(U8_T flashId, U32_T flshStartAddr, U8_T* dmaTxPt, U16_T txLen)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void	spiflsh_WriteData(U8_T flashId, U32_T flshStartAddr, U8_T* dmaTxPt, U16_T txLen)
{
	spiflsh_DescTable[flashId].opTable[SPI_WRITE].opCode[1] = (U8_T)(flshStartAddr >> 16);
	spiflsh_DescTable[flashId].opTable[SPI_WRITE].opCode[2] = (U8_T)(flshStartAddr >> 8);
	spiflsh_DescTable[flashId].opTable[SPI_WRITE].opCode[3] = (U8_T)(flshStartAddr);

	while (!SPI_IsMstSpiBusRdy()){}

	SPI_SetOpCodeReg(&(spiflsh_DescTable[flashId].opTable[SPI_WRITE].opCode[0]), spiflsh_DescTable[flashId].opTable[SPI_WRITE].bOpCodeLen);
	SPI_MstDmaTx (dmaTxPt, txLen, (SDCR_ETDMA_EXC | SDCR_ERDMA_IDL | SDCR_OPC_ENB | SDCR_DMA_GO));
	
	while (!SPI_IsMstSpiBusRdy()){}
}

/*
 *--------------------------------------------------------------------------------
 * spiflsh_SectorErase(U8_T flashId, U32_T flshStartAddr)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void	spiflsh_SectorErase(U8_T flashId, U32_T flshStartAddr)
{
	spiflsh_DescTable[flashId].opTable[SPI_SE].opCode[1] = (U8_T)(flshStartAddr >> 16);
	spiflsh_DescTable[flashId].opTable[SPI_SE].opCode[2] = (U8_T)(flshStartAddr >> 8);
	spiflsh_DescTable[flashId].opTable[SPI_SE].opCode[3] = (U8_T)(flshStartAddr);
	
	while (!SPI_IsMstSpiBusRdy()){}

	SPI_SetOpCodeReg(&(spiflsh_DescTable[flashId].opTable[SPI_SE].opCode[0]), spiflsh_DescTable[flashId].opTable[SPI_SE].bOpCodeLen);
	SPI_MstSendOpOnly();
	
	while (!SPI_IsMstSpiBusRdy()){}
}

/*
 *--------------------------------------------------------------------------------
 * spiflsh_IsWriteInProcess(U8_T flashId)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static BOOL	spiflsh_IsWriteInProcess(U8_T flashId)
{
	U8_T	status;

	status = spiflsh_ReadStatus(flashId);

	if ((status & (spiflsh_DescTable[flashId].bStatusTable[FLASH_WIP])) == spiflsh_DescTable[flashId].bStatusTable[FLASH_WIP])
		return TRUE;
	else
		return FALSE;
}

/* EXPORTED SUBPROGRAM BODIES */
/*
 *--------------------------------------------------------------------------------
 * SPIFLSH_Init(void)
 * Purpose : Initial SPI flash table
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
//void SPIFLSH_Init(void)
//{
//	spiflsh_IsFlashErase = FALSE;
//	spiflsh_InitMxFlashDescriptor(0);
//	SPI_MstModeSetup(0xb0, 0x03, 0x01, 0x00, 0x06);
//	SPI_MstDmaSetting(0x00, 0x00);
//	
//	EXTINT4_ENABLE;
//}

/*
* -----------------------------------------------------------------------------
 * Function Name: SPIFLSH_read_handle
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
BOOL SPIFLSH_read_handle(U8_T flashId, U32_T flashAddr, U8_T* dmaRxPt, U8_T length)
{
	BOOL result;

	if (!spiflsh_IsWriteInProcess(flashId))
	{
		spiflsh_ReadData(flashId, flashAddr, dmaRxPt, (U16_T)length);
		result = TRUE;
	}
	else
	{
		result = FALSE;
	}

	return result;
}

/*
* -----------------------------------------------------------------------------
 * Function Name: SPIFLSH_write_handle
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
BOOL SPIFLSH_write_handle(U8_T flashId, U32_T flashAddr, U8_T* dmaTxPt, U16_T txLen)
{
	if (!spiflsh_IsWriteInProcess(flashId))
	{
		if ((flashAddr % ((U32_T)spiflsh_DescTable[flashId].bSectorSize << 10) == 0) &&
		(spiflsh_IsFlashErase == FALSE))
		{
			spiflsh_WriteEnable(flashId);
			spiflsh_SectorErase(flashId, flashAddr);
			spiflsh_IsFlashErase = TRUE;
			return FALSE;	
		}
		else
		{
			spiflsh_WriteEnable(flashId);
			spiflsh_WriteData(flashId, flashAddr, dmaTxPt, txLen);
			spiflsh_IsFlashErase = FALSE;
			return TRUE;
		}
	}
	else
		return FALSE;
}

#endif /* SYSTEM_USB_PEN_DRIVE_SUPPORT */

/* End of spi_flash.c */


