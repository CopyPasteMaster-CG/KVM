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
 * Module Name : spim.c
 * Purpose     : This file handles the SPI master driver.
 * Author      :
 * Date        :
 * Notes       :
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	"project_include.h"
#include	"spim.h"

#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)

/* GLOBAL VARIABLE DECLARATIONS */

/* STATIC VARIABLE DECLARATIONS */
SPI_MST_INST_T	spi_Mst;

/* LOCAL SUBPROGRAM DECLARATIONS */

/* LOCAL SUBPROGRAM BODIES */

/* EXPORTED SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * void SPI_MstRegWrite(U8_T addr, U8_T *ptData, U8_T dataLen)
 * Purpose : Write data bytes into a register with interrupt locked.
 * Params  : addr - An index address to SPI register.
 *           ptData - A pointer to indicate the register data.
 *           dataLen - A number of bytes to indicate how many bytes will be written.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void SPI_MstRegWrite(U8_T addr, U8_T *ptData, U8_T dataLen)
{
	EXTINT4_DISABLE;
	while (dataLen--)
	{
		SPIDR = *(ptData + dataLen);
	}
	SPICIR = addr;
	EXTINT4_ENABLE;
}

/*
 *--------------------------------------------------------------------------------
 * void SPI_MstRegRead(U8_T addr, U8_T *ptData, U8_T dataLen)
 * Purpose : Read data bytes from a register with interrupt locked.
 * Params  : addr - An index address to SPI register.
 *           ptData - A pointer to indicate the register data.
 *           dataLen - A number of bytes to indicate how many bytes will be read.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void SPI_MstRegRead(U8_T addr, U8_T *ptData, U8_T dataLen)
{
	EXTINT4_DISABLE;
	SPICIR = addr;
	while (dataLen--)
	{
		*(ptData + dataLen) = SPIDR;
	}
	EXTINT4_ENABLE;
}

/*
 *--------------------------------------------------------------------------------
 * void SPI_MstFlagEnb(U8_T flag)
 * Purpose : Enable spi_Mst.ActFlag.
 * Params  : flag - The flag vaule to enable.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void SPI_MstFlagEnb(U8_T flag)
{
	EXTINT4_DISABLE;
	spi_Mst.ActFlag |= flag;
	EXTINT4_ENABLE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL SPI_MstFlagChk(U8_T flag)
 * Purpose : disable spi_Mst.ActFlag.
 * Params  : flag - The flag vaule to disable.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
BOOL SPI_MstFlagChk(U8_T flag)
{
	U8_T	flagValue;
	
	EXTINT4_DISABLE;
	flagValue = spi_Mst.ActFlag & flag;
	EXTINT4_ENABLE;
	
	return flagValue;
}

/*
 *--------------------------------------------------------------------------------
 * void SPI_MstModeSetup(U8_T ctrlCmd, U8_T intrEnb, U8_T baudrate, U8_T opDelay, U8_T slvSel)
 * Purpose : Setup the SPI module and initiate SPI function.
 * Params  : ctrlCmd - control register value.
 *           intrEnb - interrupt source enabled bit.
 *           baudrate - a parameter to generate baudrate.
 *           opDelay - a parameter about the delay time after OP-Code in burst-type transfer.
 *           slvSel - slave selection bit.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void SPI_MstModeSetup(U8_T ctrlCmd, U8_T intrEnb, U8_T baudrate, U8_T opDelay, U8_T slvSel)
{
	spi_Mst.IntrEnb = 0;
	spi_Mst.IntrStatus = 0;
	spi_Mst.ActFlag = 0;
	
	/* Setup SPI mode */
	SPI_MstRegWrite(SPICMR, (U8_T *)&ctrlCmd, 1);
	
	/* Enable intertupe flag type of SPI */
	spi_Mst.IntrEnb = intrEnb;
	SPI_MstRegWrite(SPIMIER, &intrEnb, 1);

	/* SPI baud rate selection */
	SPI_MstRegWrite(SPIBRR, &baudrate, 1);
	/* SPI Delay After Op-code*/
	SPI_MstRegWrite(SDAOF, &opDelay, 1);
	/* SPI slave select */
	SPI_MstRegWrite(SPIMCSR, &slvSel, 1);
}


/*
 *--------------------------------------------------------------------------------
 * void SPI_MstDmaTx(U8_T *txStrAddr, U16_T txByteCnt, U8_T cmd)
 * Purpose : Excute SPI master TX DMA.
 * Params  : txstrAddr - a pointer store the TX DMA start address.
 *           txbyteCnt - Tx DMA byte count.
 *           cmd - DMA command register value.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void SPI_MstDmaTx(U8_T *txStrAddr, U16_T txByteCnt, U8_T cmd)
{
	U16_T	reg16b = 0;
	
	SPI_MstFlagEnb(SPI_DMA_TXRX_CPL);
	txByteCnt--;
	reg16b = (U16_T)txStrAddr;	
	SPI_MstRegWrite(SDTSA, (U8_T *)&reg16b, 2);
	SPI_MstRegWrite(SDTBC, (U8_T *)&txByteCnt, 2);
	SPI_MstRegWrite(SDCR, &cmd, 1);
}

/*
 *--------------------------------------------------------------------------------
 * void SPI_MstDmaRx(U8_T *rxStrAddr, U16_T rxByteCnt, U8_T cmd)
 * Purpose : Excute SPI master RX DMA.
 * Params  : rxstrAddr - a pointer sotre the RX DMA start address.
 *           rxbyteCnt - Rx DMA byte coutn.
 *           cmd - DMA command register value.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void SPI_MstDmaRx(U8_T *rxStrAddr, U16_T rxByteCnt, U8_T cmd)
{
	U16_T	reg16b = 0;
	
	SPI_MstFlagEnb(SPI_DMA_TXRX_CPL);
	rxByteCnt--;
	reg16b = (U16_T)rxStrAddr;
	SPI_MstRegWrite(SDRSA, (U8_T *)&reg16b, 2);
	SPI_MstRegWrite(SDRBC, (U8_T *)&rxByteCnt, 2);
	SPI_MstRegWrite(SDCR, &cmd, 1);
}

/*
 *--------------------------------------------------------------------------------
 * SPI_MstSendOpOnly()
 * Purpose : Only send opcode
 * Params  : None
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void SPI_MstSendOpOnly()
{
	U8_T	cmd = (SDCR_OPC_ENB | SDCR_DMA_GO);

	SPI_MstRegWrite(SDCR, &cmd, 1);
}

/*
 *--------------------------------------------------------------------------------
 * void SPI_SetOpCodeReg(U8_T *opCode, U8_T opLen)
 * Purpose : Setting Op-code to SPI Transfer Op-code Field(STOF) register.
 * Params  : opCode - SPI Op-code.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void SPI_SetOpCodeReg(U8_T *opCode, U8_T opLen)
{
	U8_T	stol = opLen - 1;
	U8_T	opReverse[8] = {0};
	U8_T	i;

	for(i = 0; i < opLen; i++)
		opReverse[8 - i - 1] = opCode[i];
	
	SPI_MstRegWrite(STOL, &stol, 1);
	SPI_MstRegWrite(STOF, opReverse, 8);
}


/*
 *--------------------------------------------------------------------------------
 * void SPI_MstDmaSetting(U8_T dbs, U8_T dt)
 * Purpose : Setup SPI Master DMA Options.
 * Params  : dbs - SPI Delay Before SCLK.
 *			 dt - SPI Delay Transmission.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void SPI_MstDmaSetting(U8_T dbs, U8_T dt)
{
	SPI_MstRegWrite(SPIDS, &dbs, 1);
	SPI_MstRegWrite(SPIDT, &dt, 1);
}

/*
 *--------------------------------------------------------------------------------
 * BOOL SPI_IsMstSpiBusRdy(void)
 * Purpose : Check the state of SPI bus.
 * Params  : None.
 * Returns : TRUE - the SPI bus is ready. or FALSE.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
BOOL SPI_IsMstSpiBusRdy(void)
{
	U8_T dmaCommand = 0;

	if (spi_Mst.IntrEnb & SPIMIER_SDTCFIE_ENB)
	{
		return !(SPI_MstFlagChk(SPI_DMA_TXRX_CPL));
	}

	SPI_MstRegRead(SDCR, &dmaCommand, 1);

	return !(dmaCommand & SDCR_DMA_GO);
}

#endif /* (SYSTEM_USB_PEN_DRIVE_SUPPORT) */

/* End of spim.c */
