/*
 *********************************************************************************
 *     Copyright (c) 2011   ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : hsuart2.c
 * Purpose     : UART2 module is implemented to support external UART2
 *               peripherals via accessing UART2 registers. This module
 *               handles the UART2 data transmission and reception on the serial
 *               bus and supports auto-software and auto-hardware flow control
 *               functions.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
//#include	<stdio.h>
#include	"project_include.h"
#ifndef KMLOG
#if (SYSTEM_HUART_SUPPORT) 
/* GLOBAL VARIABLES DECLARATIONS */
U16_T	rs485_MstRxAvailCount = 0;


/* STATIC VARIABLE DECLARATIONS */
static U8_T		hsur2IntrEnbType;
static U8_T		hsur2FifoCtrl;
//static U8_T		hsur2ErrBlocking;
static U16_T	hsur2ErrCount;
//static U32_T		hsur2RxTxOperationErr;
//static U32_T		hsur2RingFull;
//static U8_T		hsur2RxDataAvail;
static U8_T	 XDATA* ptHsur2RxDmaRingStart;
static U8_T	 XDATA* ptHsur2RxDmaRingEnd;
static U8_T	 XDATA* ptHsur2RxDmaRingSwRead;
//static U8_T		hsur2TxDmaComplete;


/* LOCAL SUBPROGRAM DECLARATIONS */
static void		hsur2_ReadLsr(void);
static void		hsur2_RxDmaCharTimeoutEvent(void);
/* LOCAL SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * static void hsur_ReadLsr(void)
 * Purpose: Read the Line Status Register to record the error events in a counter.
 *          And begin the error recovering if happening many errors.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
static void hsur2_ReadLsr(void)
{
	U8_T	lineStatus;
	U8_T	fifoCtrl;
	U16_T	reg16b;
	
	UR2CIR = HSLSR;
	lineStatus = UR2DR;

	if (lineStatus & HSLSR_OE_OVER)
	{
		hsur2ErrCount ++;
		// Overrun Error
	}
	else if (lineStatus & HSLSR_PE_ERROR)
	{
		hsur2ErrCount ++;
		// Parity Error
	}
	else if (lineStatus & HSLSR_FE_ERROR)
	{
		hsur2ErrCount ++;
		// Framing Error
	}
	else if (lineStatus & HSLSR_BI_INT)
	{
		hsur2ErrCount ++;
		// Break Interrupt Occured
	}
	else if (lineStatus & HSLSR_FERR_ERROR)
	{
		hsur2ErrCount ++;
		// Mixing Error
	}

	/* For Error handling before data synchrony */
	if (hsur2ErrCount)
	{
		/* Enable the Receiver FIFO STOP (RSTOP) bit to stop data characters transfer into RX FIFO.
		   And Reset Rx FIFO and TX FIFO */
		fifoCtrl = (hsur2FifoCtrl | HSFCR_RSTOP | HSFCR_RFR | HSFCR_TFR);
		UR2DR = fifoCtrl;
		UR2CIR = HSFCR;
		/* Read the error data pointer */
		UR2CIR = HSRBDEP;
		reg16b = (U16_T)UR2DR;
		reg16b |= (U16_T)UR2DR << 8;
		/* Flush all characters in the RX buffer ring by HSRBRP = HSRBWP when UART2 is in DMA mode */
		UR2CIR = HSRBWP;
		reg16b = (U16_T)UR2DR;
		reg16b |= (U16_T)UR2DR << 8;
		UR2DR = (U8_T)reg16b;
		UR2DR = (U8_T)(reg16b >> 8);
		UR2CIR = HSRBRP;
//		ptHsur2RxDmaRingSwRead = ((U32_T)ptHsur2RxDmaRingStart & 0xFF0000) | (U32_T)reg16b;
		ptHsur2RxDmaRingSwRead = (U8_T XDATA*)reg16b;

		/* Disable the Receiver FIFO STOP (RSTOP) bit to re-start RX FIFO */
		fifoCtrl = (hsur2FifoCtrl | HSFCR_RFR | HSFCR_TFR);
		fifoCtrl &= ~HSFCR_RSTOP;
		UR2DR = fifoCtrl;
		UR2CIR = HSFCR;

		if (hsur2ErrCount >= 100)
		{
//			hsur2ErrBlocking = 1;
			/* Disable the UART2 interrupt */
			UR2DR = 0;
			UR2CIR = HSIER;
		}
	}
}


/* EXPORTED SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR2_RegWrite(U8_T addr, U8_T *ptData, U8_T dataLen)
 * Purpose: Write data bytes into a register with interrupt locked.
 * Params : addr:An index address to UART2 register.
 *          ptData:A pointer to indicate the register data.
 *          dataLen:A number of bytes to indicate how many bytes will be written.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
BOOL HSUR2_RegWrite(U8_T addr, U8_T *ptData, U8_T dataLen)
{
	U8_T	oldEintBit = EINT4;

	EINT4 = 0;
	while (dataLen--)
	{
		UR2DR = *(ptData + dataLen);
	}
	UR2CIR = addr;
	EINT4 = oldEintBit;

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR2_RegRead(U8_T addr, U8_T *ptData, U8_T dataLen)
 * Purpose: Read data bytes from a register with interrupt locked.
 * Params : addr:An index address to UART2 register.
 *          ptData:A pointer to indicate the register data.
 *          dataLen:A number of bytes to indicate how many bytes will be read.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
BOOL HSUR2_RegRead(U8_T addr, U8_T *ptData, U8_T dataLen)
{
	U8_T	oldEintBit = EINT4;

	EINT4 = 0;
	UR2CIR = addr;
	while (dataLen--)
	{
		*(ptData + dataLen) = UR2DR;
	}
	EINT4 = oldEintBit;

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
          * void HSUR2_IntrEntryFunc(void)
 * Purpose: An interrupt service rounine that should be included in MCPU peripheral
 *          interface interrupt service routine within the AX220xx MCPU module.
 *          It will be trigger by interrupt events that be incicated in the HSIIR register.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR2_IntrEntryFunc(void)
{
	U8_T	intrStatus;
	U8_T	ur2DmaIntrStatus;
	U16_T	temp16b = 0;
//	U8_T	temp8b[2];

	UR2CIR = HSIIR;
	intrStatus = UR2DR;
	intrStatus &= 0x1F;

	if (intrStatus == HSIIR_RLS_INTR)
	{
		hsur2_ReadLsr();
	}
	else if (intrStatus == HSIIR_DMAS_INTR)
	{ /* DMA interrupt flag */
		UR2CIR = HSDSR;
		ur2DmaIntrStatus = UR2DR;
		
		//Assing to ISR handle 
		ISR_FIFO[ISR_FIFO_Wp].ISR_Type = ISR_HUART;
		ISR_FIFO[ISR_FIFO_Wp].Data = ur2DmaIntrStatus;				
		
		ISR_FIFO_Wp++;
		if (ISR_FIFO_Wp >= ISR_FIFO_DEPTH)
			ISR_FIFO_Wp = 0;			
	}	
	else if ((intrStatus == HSIIR_RD_TI_INTR) || (intrStatus == HSIIR_RD_TRIG_INTR))	
	{ /* Receiver Timeout */
		/* Only for DMA buffer ring mode */
		// Assing to ISR handle 
		ISR_FIFO[ISR_FIFO_Wp].ISR_Type = ISR_HUART_RBDMA;
		UR2CIR = HSRBABC;
		ISR_FIFO[ISR_FIFO_Wp].Data  = UR2DR;				
		ISR_FIFO[ISR_FIFO_Wp].State = UR2DR;					
		
		rs485_MstRxAvailCount = ISR_FIFO[ISR_FIFO_Wp].Data + (ISR_FIFO[ISR_FIFO_Wp].State << 8);		
		ptHsur2RxDmaRingSwRead += rs485_MstRxAvailCount;
		if (ptHsur2RxDmaRingSwRead > ptHsur2RxDmaRingEnd)
			ptHsur2RxDmaRingSwRead = ptHsur2RxDmaRingStart + (ptHsur2RxDmaRingSwRead - ptHsur2RxDmaRingEnd - 1);
			
		//For applicaton control ------------------------------------
		Receive_Buffer_End = (U16_T)ptHsur2RxDmaRingSwRead - UR2_RX_BUF_START_ADDR;
//		Receive_Buffer_End += rs485_MstRxAvailCount;
//		if (Receive_Buffer_End >= UR2_RX_BUF_SIZE)
//			Receive_Buffer_End -= UR2_RX_BUF_SIZE; 
		//-----------------------------------------------------------	
		
		//
		//$Update software read pointer for receiver buffer ring		
		//
		temp16b = (U16_T)ptHsur2RxDmaRingSwRead;
		UR2DR = (U8_T)temp16b;
		UR2DR = (U8_T)(temp16b >> 8);
//		temp8b[0] = temp32b & 0x000000ff; 	    
//		temp8b[1] = (temp32b & 0x0000ff00) >> 8; 	    		
//		UR2DR = temp8b[0];
//		UR2DR = temp8b[1];
		UR2CIR = HSRBRP;
		
		ISR_FIFO_Wp++;
		if (ISR_FIFO_Wp >= ISR_FIFO_DEPTH)
			ISR_FIFO_Wp =  0;					
	}
	else if (intrStatus == HSIIR_TFE_INTR)
	{
		/* This status will not be occurred in Tx DMA mode */
	}	
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR_Setup(U16_T divisor, U8_T lCtrl, U8_T intEnb, U8_T fCtrl, U8_T mCtrl)
 * Purpose: Use this function can initialize the UART2 basic mode. It will configure
 *          the baudrate, data bus type, interrupt source, FIFO setup and modem status.
 * Params : divisor : A 16-bit Divisor Latch value to calaulate the baudrate.
 *          lCtrl : A value to Line Control Register.
 *          intEnb : The interrupt source type of Interrupt Enable Register.
 *          fCtrl : A value to FIFO Control Register.
 *          mCtrl : A value to Modem Control Register.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR2_Setup(U16_T divisor, U8_T lCtrl, U8_T intEnb, U8_T fCtrl, U8_T mCtrl)
{
	U8_T	lineCtrl, dll, dlh;

	hsur2IntrEnbType = intEnb;
	/* Set UART2 Baudrate */
	lineCtrl = HSLCR_DLAB_ENB;
	HSUR2_RegWrite(HSLCR, &lineCtrl, 1);
	dll = (U8_T)(divisor & 0x00FF);
	dlh = (U8_T)((divisor & 0xFF00) >> 8);
	HSUR2_RegWrite(HSDLLR, &dll, 1);
	HSUR2_RegWrite(HSDLHR, &dlh, 1);
	lineCtrl &= ~HSLCR_DLAB_ENB;
	HSUR2_RegWrite(HSLCR, &lineCtrl, 1);
	/* Set Line Control Register */
	lCtrl &= ~HSLCR_DLAB_ENB;
	HSUR2_RegWrite(HSLCR, &lCtrl, 1);
	/* Set FIFO Control Register */
	hsur2FifoCtrl = fCtrl | HSFCR_RSTOP; // Enable the RSTOP bit to stop the data characters into RX FIFO.
	hsur2FifoCtrl &= ~(HSFCR_RFR | HSFCR_TFR);
	HSUR2_RegWrite(HSFCR, &fCtrl, 1);
	/* Set Modem Control Register */
	HSUR2_RegWrite(HSMCR, &mCtrl, 1);
	/* Disable DMA Mode */
	dll = 0;
	HSUR2_RegWrite(HSDCR, &dll, 1);
	/* Initial Variables */
	hsur2ErrCount = 0;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR2_RxDmaControlInit(void)
 * Purpose: Initialize the UART2 RX buffer ring in DMA mode. It will configure the
 *          ring start/end address, receiving data trig level and receiving timeout gap.
 * Params : ptRxRingStart : A pointer to indicate the RX buffer ring start address.
 *                          The low byte of this must be 0x00.
 *          ptRxRingEnd : A pointer to indicate the RX buffer ring end address.
 *                        The low byte of this must be 0xFF.
 *          rxRingOccupancyTrigLevel : RX ring trigger level to notice driver with numbers of RX data.
 *          rxRingTimeoutGap : To tell hardware how many character times to wait before reporting timeout in RX buffer ring.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
BOOL HSUR2_RxDmaControlInit(void)
{
	U8_T	temp[3] = {0};
	U16_T XDATA*	pTempShort = (U16_T XDATA*)&temp[1];

	if ((U8_T)rs485_RcvrBufRing != 0)
		return FALSE;

	ptHsur2RxDmaRingStart = (U16_T)rs485_RcvrBufRing;
	*pTempShort = (U16_T)ptHsur2RxDmaRingStart;
	HSUR2_RegWrite(HSRBSP, (U8_T *)&temp[0], 2);

	ptHsur2RxDmaRingEnd = (U16_T)(rs485_RcvrBufRing + (UR2_RX_BUF_SIZE - 1));
	*pTempShort = (U16_T)ptHsur2RxDmaRingEnd;
	HSUR2_RegWrite(HSRBEP, (U8_T *)&temp[0], 2);

	ptHsur2RxDmaRingSwRead = (U16_T)rs485_RcvrBufRing;
	*pTempShort = (U16_T)ptHsur2RxDmaRingSwRead;
	HSUR2_RegWrite(HSRBRP, (U8_T *)&temp[1], 2);

	*pTempShort = UR2_RX_TRIGGER_LEVEL;
	HSUR2_RegWrite(HSRBOTL, (U8_T *)&temp[1], 2);

	*pTempShort = UR2_RX_TIMEOUT_GAP;
	HSUR2_RegWrite(HSRBRTG, (U8_T *)&temp[1], 2);

	/* Enable Receive Buffer Ring */
	HSUR2_RegRead(HSDCR, &temp[0], 1);
	temp[0] |= HSDCR_RBRE_ENB;
	HSUR2_RegWrite(HSDCR, &temp[0], 1);

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR2_Start(void)
 * Purpose: Start function will enable the interrupt sources that be configured
 *          in the setup function. This function should be closely called before
 *          running application tasks.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR2_Start(void)
{
	/* Enable the UART2 interrupt */
	//printf("IER=%02x\r",(U16_T)hsur2IntrEnbType);
	HSUR2_RegWrite(HSIER, &hsur2IntrEnbType, 1);
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR2_ErrorRecovery(void)
 * Purpose: Checking the UART2 Line Status Register value of errors and
 *          re-enable interrupts of receiving and line status.
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
//void HSUR2_ErrorRecovery(void)
//{
//	U8_T	valLsr;
//	U8_T	valFcr;
//	U16_T	reg16b;

//	while (hsur2ErrBlocking)
//	{
//		printf("hsur2ErrBlocking = 1\n\r");
//		HSUR2_RegRead(HSLSR, &valLsr, 1);
//		if (valLsr & HSLSR_DR_DATA)
//		{
			/* Enable the Receiver FIFO STOP (RSTOP) bit to stop data characters transfer into RX FIFO.
			   And Reset Rx FIFO and TX FIFO */
//			valFcr = (hsur2FifoCtrl | HSFCR_RSTOP | HSFCR_RFR | HSFCR_TFR);
//			UR2DR = valFcr;
//			UR2CIR = HSFCR;
			/* Read the error data pointer */
//			UR2CIR = HSRBDEP;
//			reg16b = (U16_T)UR2DR;
//			reg16b |= (U16_T)UR2DR << 8;
			/* Flush all characters in the RX buffer ring by HSRBRP = HSRBWP when UART2 is in DMA mode */
//			UR2CIR = HSRBWP;
//			reg16b = (U16_T)UR2DR;
//			reg16b |= (U16_T)UR2DR << 8;
//			UR2DR = (U8_T)reg16b;
//			UR2DR = (U8_T)(reg16b >> 8);
//			UR2CIR = HSRBRP;
//			ptHsur2RxDmaRingSwRead = ((U32_T)ptHsur2RxDmaRingStart & 0xFF0000) | (U32_T)reg16b;
			/* Disable the Receiver FIFO STOP (RSTOP) bit to re-start RX FIFO */
//			valFcr = (hsur2FifoCtrl | HSFCR_RFR | HSFCR_TFR);
//			valFcr &= ~HSFCR_RSTOP;
//			UR2DR = valFcr;
//			UR2CIR = HSFCR;
//		}
//		else
//		{
//			printf("clear hsur2ErrBlocking = 0\n\r");
//			EA = 0;
//			hsur2ErrBlocking = 0;
//			hsur2ErrCount = 0;
			/* Enable the UART2 interrupt again */
//			UR2DR = hsur2IntrEnbType;
//			UR2CIR = HSIER;
//			EA = 1;
//		}
//	}
//}

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR2_RxDmaSwReadPointerUpdated(U16_T lenCount)
 * Purpose: release the RX buffer ring memory that have been retrieved.
 * Params : lenCount : A length that indicates how many valid data can be retrieved.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
//BOOL HSUR2_RxDmaSwReadPointerUpdated(U16_T lenCount)
//{
//	U32_T	temp32b = 0;
//	U8_T	temp8b = 0;

//	ptHsur2RxDmaRingSwRead = ptHsur2RxDmaRingSwRead + lenCount;

//	if (ptHsur2RxDmaRingSwRead > ptHsur2RxDmaRingEnd)
//		ptHsur2RxDmaRingSwRead = ptHsur2RxDmaRingStart + (ptHsur2RxDmaRingSwRead - ptHsur2RxDmaRingEnd - 1);

//	temp32b = ((U32_T)ptHsur2RxDmaRingSwRead - 0x010000);
//	temp32b <<= 16;

//	HSUR2_RegWrite(HSRBRP, (U8_T *)&temp32b, 2);
			
	/* Re-enable the RxDMA interrupt */
	/*
	HSUR2_RegRead(HSIER, &temp8b, 1);
	temp8b |= HSIER_RDI_ENB;
	HSUR2_RegWrite(HSIER, &temp8b, 1);
	*/
		
//	return TRUE;
//}

/*
 *--------------------------------------------------------------------------------
 * U16_T HSUR2_GetRxDmaAvailCount(void)
 * Purpose: The number of valid data bytes in Receive Buffer Ring.
 *          Check this to determine how many bytes have been received.
 *          This function will directly read a register HSRBABC (0x20) to return.
 * Params : None.
 * Returns: available bytes, HSRBABC (0x20).
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
//U16_T HSUR2_GetRxDmaAvailCount(void)
//{
//	U16_T	reg16b;

//	if ((!hsur2RxDataAvail) && (!hsur2RingFull))
//	{
//		return 0;
//	}

	/* update the HSRBABC to get the available count of Rx ring*/
//	HSUR2_RegRead(HSRBABC, (U8_T *)&reg16b, 2);

//	return reg16b;
//}

/*
 *--------------------------------------------------------------------------------
 * void HSUR2_WakeUpEvent(void)
 * Purpose: An interrupt service rounine that should be included in MCPU
 *          Wake-Up interrupt service routine within the AX220xx MCPU module.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
//void HSUR2_WakeUpEvent(void)
//{
//	U8_T	reg8b;

//	HSUR2_RegRead(HSDSR, &reg8b, 1);
//}
#endif /* #if (SYSTEM_HUART_SUPPORT) */
#endif /* #ifndef KMLOG */
/* End of hsuart2.c */
