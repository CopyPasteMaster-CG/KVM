/*
 *********************************************************************************
 *     Copyright (c) 2010   ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : hsuart2.h
 * Purpose     : A header file of UART2 module.
 *               It defines all globe parameters and extend API functions.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 *================================================================================
 */
#ifndef HSUART2_H
#define HSUART2_H

/* INCLUDE FILE DECLARATIONS */


/* NAMING CONSTANT DECLARATIONS */


/* TYPE DECLARATIONS */


/* GLOBAL VARIABLES */
extern 	U16_T	rs485_MstRxAvailCount;


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
BOOL	HSUR2_RegWrite(U8_T addr, U8_T *ptData, U8_T dataLen);
BOOL	HSUR2_RegRead(U8_T addr, U8_T *ptData, U8_T dataLen);
void	HSUR2_IntrEntryFunc(void);
void	HSUR2_Setup(U16_T divisor, U8_T lCtrl, U8_T intEnb, U8_T fCtrl, U8_T mCtrl);
void	HSUR2_Start(void);
//void	HSUR2_ErrorRecovery(void);
BOOL	HSUR2_RxDmaControlInit(void);
//BOOL	HSUR2_RxDmaSwReadPointerUpdated(U16_T lenCount);
//U16_T	HSUR2_GetRxDmaAvailCount(void);
BOOL	HSUR2_TxDmaTransmit(U8_T *ptTx, U16_T length);
//void	HSUR2_WakeUpEvent(void);
void	HSUR2_CopyRxDmaToApp(U8_T *pAppData, U16_T appFreeLen);

#endif /* End of HSUART2_H */
