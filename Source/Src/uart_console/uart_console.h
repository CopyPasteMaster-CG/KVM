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

#ifndef __UART_CONSOLE_H__
#define __UART_CONSOLE_H__

/* INCLUDE FILE DECLARATIONS */
/* NAMING CONSTANT DECLARATIONS */
#define UART_CONSOLE_WAIT		0
#define UART_CONSOLE_SEND		1
#define UART_CONSOLE_SCREEN_2_WINDOWS   0
#define UART_CONSOLE_SCREEN_1_WINDOWS   1

typedef struct _UartConsole_Handle
{
	U8_T	Operation; //0-Wait,1-Send
	U8_T    *WaitMsg;  
	U8_T    ParameterCnt;  
	void   (* HandleFunction)(void);	
} UartConsole_Handle_TypeDef;

/* GLOBAL VARIABLES */
extern U8_T		uart0_TxBuf[];
extern U16_T	uart0_TxHead;
extern U16_T	uart0_TxTail;
extern U16_T	uart0_TxCount;
extern U8_T		uart0_TxFlag;
extern U8_T		uart0_RxBuf[];
extern U16_T	uart0_RxHead;
extern U16_T	uart0_RxTail;
extern U16_T	uart0_RxCount;
extern bit		UartConsole_R_Queue;
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void UartConsole_Init(void);
void UartConsole_Recieve_Handle(void);
#endif /* End of __UART_CONSOLE_H__ */

/* End of uart.h */