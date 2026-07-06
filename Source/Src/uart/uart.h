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
 * Module Name : uart.h
 * Purpose     : A header file of uart.c includes UART0 and UART1 module.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

#ifndef __UART_H__
#define __UART_H__

/* INCLUDE FILE DECLARATIONS */
/* NAMING CONSTANT DECLARATIONS */
#define MAX_TX_UART0_BUF_SIZE	512
#define MAX_TX_UART0_MASK		(MAX_TX_UART0_BUF_SIZE - 1)
#ifdef UART_CONSOLE
#define MAX_RX_UART0_BUF_SIZE	256
#else
#define MAX_RX_UART0_BUF_SIZE	16
#endif
#define MAX_RX_UART0_MASK		(MAX_RX_UART0_BUF_SIZE - 1)

#define MAX_RX_UART1_BUF_SIZE	256
#define MAX_RX_UART1_MASK		(MAX_RX_UART1_BUF_SIZE - 1)

/* GLOBAL VARIABLES */
extern U8_T	uart0_TxBuf[MAX_TX_UART0_BUF_SIZE];
extern U16_T	uart0_TxHead;
extern U16_T	uart0_TxTail;
extern U16_T	uart0_TxCount;
extern U8_T		uart0_TxFlag;
extern U8_T		uart0_RxBuf[MAX_RX_UART0_BUF_SIZE];
extern U16_T	uart0_RxHead;
extern U16_T	uart0_RxTail;
extern U16_T	uart0_RxCount;
extern U8_T		uart1_RxBuf[MAX_RX_UART1_BUF_SIZE];
extern U16_T	uart1_RxHead;
extern U16_T	uart1_RxTail;
extern U16_T	uart1_RxCount;
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void	UART_Init(void);
void	UART1_Polling_Receive_Handle(void);
S8_T	putchar(S8_T);
S8_T	UART0_PutChar(S8_T c);
S8_T	HSUART1_PutChar(S8_T c);
S8_T	HSUART_PutChar(S8_T c);
BOOL	HSUART_PutData(U8_T *buf, U8_T len);
S8_T	UART0_NoBlockGetkey (void);

#endif /* End of __UART_H__ */

/* End of uart.h */
