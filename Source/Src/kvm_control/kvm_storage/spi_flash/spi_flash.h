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
 * Module Name : spi_flash.h
 * Purpose     : This module handles the SPI flash interface driver.
 * Author      : 
 * Date        :
 * Notes       :
 *
 *================================================================================
 */
#ifndef SPIFLASH_H
#define SPIFLASH_H

/* INCLUDE FILE DECLARATIONS */


/* NAMING CONSTANT DECLARATIONS */
#define		MX8006_WREN	0x06
#define		MX8006_RDID	0x9F
#define		MX8006_RDSR	0x05
#define		MX8006_WRSR	0x01
#define		MX8006_RD	0x03
#define		MX8006_FSRD	0x0B
#define		MX8006_SE	0x20
#define		MX8006_BE	0xD8
#define		MX8006_CE	0x60
#define		MX8006_PP	0x02
#define		MX8006_DP	0xB9
#define		MX8006_RDP	0xAB
#define		MX8006_RES	0xAB
#define		MX8006_REMS	0x90

#define		MX8006_SRWD	BIT7
#define		MX8006_BP2	BIT4
#define		MX8006_BP1	BIT3
#define		MX8006_BP0	BIT2
#define		MX8006_WEL	BIT1
#define		MX8006_WIP	BIT0


#define MX8006_VENDOR_ID			0xC2
#define	MX8006_PRODUCT_ID			0x2014
#define	MX8006_SECTOR_NUM			0x100
#define	MX8006_SECTOR_SIZE			4 // 4KB

#define	MAX_OP_SUPPORT			8
#define MAX_FLASH_STATUS		8

enum SPI_CMD_IND
{
	SPI_WREN = 0,

	SPI_RDSR,
	SPI_READ,
	SPI_WRITE,
	SPI_SE,
};

enum	FLASH_STATUS
{
	FLASH_WIP = 0,
	FLASH_WEL,
};

/* TYPE DECLARATIONS */
typedef struct
{
U8_T	bOpCodeLen;
U8_T	opCode[7];	// max command is 5 bytes
} OpCodeEntry;

typedef struct
{
U16_T	wVendorId;	// Manufact ID
U16_T	wProductId;	// Device ID
U16_T	wMaxSectorNum;	// If max sector number over 256, need 2 bytes
U8_T	bSectorSize;	// unit is KB
OpCodeEntry	opTable[MAX_OP_SUPPORT];
U8_T	bStatusTable[MAX_FLASH_STATUS];
} FlashDescriptor;

/* GLOBAL VARIABLES */
extern BOOL	spiflsh_IsFlashErase;
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
//void	SPIFLSH_Init(void);
BOOL SPIFLSH_read_handle(U8_T flashId, U32_T	flashAddr, U8_T* dmaRxPt, U8_T length);
BOOL SPIFLSH_write_handle(U8_T flashId, U32_T flashAddr, U8_T* dmaTxPt, U16_T txLen);
void	spiflsh_InitMxFlashDescriptor(U8_T flashId);
#endif /* End of SPIFLASH_H */
