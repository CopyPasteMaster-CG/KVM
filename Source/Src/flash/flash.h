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
 * Module Name : flash.h
 * Purpose     :
 * Author      : Robin Lee
 * Date        : 
 * Notes       :
 *================================================================================
 */
#ifndef __FLASH_H__
#define __FLASH_H__

/* INCLUDE FILE DECLARATIONS */
#include	"system_cfg.h"

#if (SYSTEM_STORAGE_FLASH)
/* NAMING CONSTANT DECLARATIONS */
/* Flash Register */
#define FLH_FCCR1					0x00 //Flash Controller Command Register1
#define FLH_FCCR2					0x01 //Flash Controller Command Register2
#define FLH_FWATR					0x02 //Flash Write Access Time Register
#define FLH_FAR						0x04 //Flash Address Register
#define FLH_FETOR					0x06 //Flash Erase Time Out Register
#define FLH_FWTOR					0x07 //Flash Write Time Out Register
#define FLH_FCSR					0x08 //Flash Controller Status Register
/* FCCR1 Bit Definition */
#define FCCR1_FC_GO					BIT0
#define FCCR1_INFO_PAGE				BIT1
#define FCCR1_CNDX					BIT2
#define FCCR1_FCMD_WORD_READ		0x00
#define FCCR1_FCMD_WORD_WRITE		BIT5
#define FCCR1_FCMD_SECTOR_ERASE		(BIT6+BIT4)
#define FCCR1_FCMD_MASS_ERASE		(BIT6+BIT5)
/* FCCR2 Bit Definition */
#define FCCR2_FCF_INTEN				BIT0
#define FCCR2_FCS_INTEN				BIT1
/* FCSR Bit Definition */
#define FCSR_FC_CS					BIT0
#define FCSR_FC_TO					BIT1
#define FCSR_WD_NR					BIT2
#define FCSR_SW_PWC_TO				BIT3
#define FCSR_DCHK_FAIL				BIT4
#define FCSR_WRITE_ERASE_BUSY		BIT7
#define FCSR_ERROR_OCCURRED			(FCSR_FC_TO|FCSR_WD_NR|FCSR_SW_PWC_TO|FCSR_DCHK_FAIL)
/* FCISR */
#define FCISR_ERROR_OCCURRED		(PWC_TO|PWD_NR|SDPWC_TO|PWD_CHKF)

#define FLASH_SECTOR_SIZE			0x1000 // 4K Bytes
#define FLASH_WORD_LEN				8
#define FLASH_SECTOR_ADDR_GAP		(FLASH_SECTOR_SIZE / FLASH_WORD_LEN)

/* MACRO DECLARATIONS */
/* TYPE DECLARATIONS */
/* GLOBAL VARIABLES */
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void	FLASH_Setup(void);
void	FLASH_RegWrite(U8_T addr, U8_T *ptData, U8_T dataLen);
void	FLASH_RegRead(U8_T addr, U8_T *ptData, U8_T dataLen);
BOOL	FLASH_CheckCmdComplete(void);
BOOL	FLASH_SectorErase(void);
BOOL	FLASH_InfoWordRead8Byte(U32_T addr, U8_T *pData);
#else
#define FLH_FWATR					0x02 //Flash Write Access Time Register
#endif

#endif /* End of __FLASH_H__ */