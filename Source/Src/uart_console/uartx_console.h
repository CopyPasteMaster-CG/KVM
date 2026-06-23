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
 * Module Name : uartx_console.h
 * Purpose     : A header file of uart.c includes UART0 and UART1 module.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

#ifndef __UART_CONSOLE_H__
#define __UART_CONSOLE_H__

/* INCLUDE FILE DECLARATIONS 		*/
/* NAMING CONSTANT DECLARATIONS 	*/
#ifdef HSUART_CONSOLE
	#define MAX_TX_UARTx_BUF_SIZE	MAX_TX_HSUART_BUF_SIZE	
	#define MAX_TX_UARTx_MASK 		MAX_TX_HSUART_MASK	
	#define MAX_RX_UARTx_BUF_SIZE	MAX_RX_HSUART_BUF_SIZE
	#define MAX_RX_UARTx_MASK		MAX_RX_HSUART_MASK
	#define UARTx_PutStr			HSUART_PutStr
	#define UARTx_TxBuf				hsuart_TxBuf
	#define UARTx_RxBuf				hsuart_RxBuf
#else
	#define MAX_TX_UARTx_BUF_SIZE	MAX_TX_UART0_BUF_SIZE	
	#define MAX_TX_UARTx_MASK 		MAX_TX_UART0_MASK	
	#define MAX_RX_UARTx_BUF_SIZE	MAX_RX_UART0_BUF_SIZE
	#define MAX_RX_UARTx_MASK		MAX_RX_UART0_MASK
	#define UARTx_PutStr			printf
	#define UARTx_TxBuf				uart0_TxBuf
	#define UARTx_RxBuf				uart0_RxBuf
#endif

#ifdef HSUART_CONSOLE
	#define Uartx_TxHead 	hsuart_TxHead
	#define Uartx_TxTail 	hsuart_TxTail
	#define Uartx_TxCount 	hsuart_TxCount
	#define Uartx_TxFlag 	hsuart_TxFlag
	#define Uartx_RxHead 	hsuart_RxHead
	#define Uartx_RxTail 	hsuart_RxTail
	#define Uartx_RxCount 	hsuart_RxCount
#else
	#define Uartx_TxHead 	uart0_TxHead
	#define Uartx_TxTai 	uart0_TxTail
	#define Uartx_TxCount 	uart0_TxCount
	#define Uartx_TxFlag 	uart0_TxFlag
	#define Uartx_RxHead 	uart0_RxHead
	#define Uartx_RxTail 	uart0_RxTail
	#define Uartx_RxCount 	uart0_RxCount
#endif
typedef struct _UartConsole_Handle
{
	U8_T	Operation; //0-Wait,1-Send
	U8_T    *WaitMsg;  
	U8_T    ParameterCnt;  
	void   (* HandleFunction)(void);	
} UartConsole_Handle_TypeDef;

/* GLOBAL VARIABLES */
extern bit	UartConsole_R_Queue;
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void UartConsole_Init(void);
void UartConsole_Recieve_Handle(void);
#endif /* End of __UART_CONSOLE_H__ */

/* End of uart.h */