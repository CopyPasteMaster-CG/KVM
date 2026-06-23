/*
 ******************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*=============================================================================
 * Module Name:dma.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */

#ifndef __DMA_H__
#define __DMA_H__


/* INCLUDE FILE DECLARATIONS */
/* NAMING CONSTANT DECLARATIONS */
/* DMA register definitions */
#define SW_DMA_CMD_REG			0x00
#define SW_DMA_STATUS_REG		0x01
#define SW_DMA_SOURCE_ADDR_REG	0x02
#define SW_DMA_TARGET_ADDR_REG	0x06
#define SW_DMA_BYTE_COUNT_REG	0x0A
#define	SW_MS_TIMER_REG			0x0C

/* Bit definitions: SW_DMA_CMD_REG */
#define DMA_CMD_GO				BIT0
#define DMA_COMPARE_SET			BIT1
#define	DMA_COMPLETE_INT_ENABLE	BIT3
#define	DMA_SA_IN_PROG_MEM		BIT4
#define DMA_TA_IN_PROG_MEM		BIT5
#define	DMA_RW_IN_INFO_PAGE		BIT7

/* Bit definitions: SW_DMA_STATUS_REG */
#define DMA_COMPLETE_STU		BIT0
#define DMA_COMPARE_ERR_STU		BIT1
#define DMA_ERROR_STU			BIT2

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
/*-------------------------------------------------------------*/
BOOL DMA_GrantXdata(U8_T XDATA*, U8_T XDATA*, U16_T);
void DMA_RegWrite(U8_T, U8_T XDATA*, U8_T);
BOOL DMA_CheckStatus(void);
BOOL DMA_DataToProgram(U32_T addrDest, U32_T addrSour, U16_T length);
#endif /* End of __DMA_H__ */


/* End of dma.h */