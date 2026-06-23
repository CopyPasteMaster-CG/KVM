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
 * Module Name : hsuart.h
 * Purpose     : A header file of uart.c includes UART0 and UART1 module.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

#ifndef __HSUART_H__
#define __HSUART_H__

/* INCLUDE FILE DECLARATIONS */
/* NAMING CONSTANT DECLARATIONS */
#define MAX_TX_HSUART_BUF_SIZE	256
#define MAX_TX_HSUART_MASK		(MAX_TX_HSUART_BUF_SIZE - 1)

#ifdef HSUART_CONSOLE
	#define MAX_RX_HSUART_BUF_SIZE	256
#else
	#define MAX_RX_HSUART_BUF_SIZE	256
#endif /* #ifdef HSUART_CONSOLE */

#define MAX_RX_HSUART_MASK		(MAX_RX_HSUART_BUF_SIZE - 1)

/* GLOBAL VARIABLES */
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void	HSUART_Init(void);
BOOL  	HSUART_PutStr(char *format); 
void 	HSUR2_IntrEntryFunc(void);
BOOL 	HSUART_PutStr(char *str);
BOOL 	HSUR2_RegWrite(U8_T addr, U8_T *ptData, U8_T dataLen);
BOOL 	HSUR2_RegRead(U8_T addr, U8_T *ptData, U8_T dataLen);
void 	HSUR2_Setup(U16_T divisor, U8_T lCtrl, U8_T intEnb, U8_T fCtrl, U8_T mCtrl);
void 	HSUR2_Start(void);
#endif /* End of __HSUART_H__ */

/* End of uart.h */