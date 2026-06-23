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
 * Module Name : uart.c
 * Purpose     : The UART module driver. It manages the character
 *               buffer and handles the ISR.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	"ax6800x.h"
#include	"types.h"
#include	"uart.h"
#include	"cpu_cfg.h"
#include	"cpu.h"
#include	"string.h"
#include	"project_include.h"

/* STATIC VARIABLE DECLARATIONS */
U8_T	uart0_TxBuf[MAX_TX_UART0_BUF_SIZE];
U16_T	uart0_TxHead = 0;
U16_T	uart0_TxTail = 0;
U16_T	uart0_TxCount = 0;
U8_T	uart0_TxFlag = 0;
U8_T	uart0_RxBuf[MAX_RX_UART0_BUF_SIZE];
U16_T	uart0_RxHead = 0;
U16_T	uart0_RxTail = 0;
U16_T	uart0_RxCount = 0;

/* LOCAL SUBPROGRAM DECLARATIONS */
static void		uart0_ISR(void);
static void		uart0_Init(void);

/* LOCAL SUBPROGRAM BODIES */
/*
 * ----------------------------------------------------------------------------
 * static void uart_ISR(void)
 * Purpose : UART0 interrupt service routine. For sending out, it puts data
 *           from software buffer into hardware serial buffer register (SBUF0).
 *           For receiving, it gets data from hardware serial buffer register
 *           (SBUF0) and stores into software buffer.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
static void uart0_ISR(void) interrupt UR0_VECTOR
{
	if (RI0)
	{
		//if (uart0_RxCount != MAX_RX_UART0_BUF_SIZE) 
		{
			uart0_RxBuf[uart0_RxTail] = SBUF0;
			//uart0_RxCount++;
			uart0_RxTail++;
			uart0_RxTail &= MAX_RX_UART0_MASK;
			ISR_FIFO[ISR_FIFO_Wp].ISR_Type = ISR_UART0;
			//ISR_FIFO[ISR_FIFO_Wp].Data = cpu_Gpio0IntrStatus;
			if (++ISR_FIFO_Wp >= ISR_FIFO_DEPTH)
			{
				ISR_FIFO_Wp = 0;
			}	
		}
		RI0 = 0;
	} /* End of if(RI0) */

	if (TI0)
	{
		uart0_TxTail++;
		uart0_TxTail &= MAX_TX_UART0_MASK;
		uart0_TxCount--;
		if (uart0_TxCount > 0)
		{
			SBUF0 = uart0_TxBuf[uart0_TxTail];
		}
		else
			uart0_TxFlag = 0;

		TI0 = 0;
	} /* End of if(TI0) */
}

/*
 * ----------------------------------------------------------------------------
 * static void uart0_Init(void)
 * Purpose : Setting operation mode of UART0 and initiating the global values.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
static void uart0_Init(void)
{
//	U8_T	sysClk = 0;
//	U16_T	i;

	ES0 = 0;
	TR1 = 0;

	uart0_TxHead = 0;
	uart0_TxTail = 0;
	uart0_TxCount = 0;
	uart0_TxFlag = 0;
	uart0_RxHead = 0;
	uart0_RxTail = 0;
	uart0_RxCount = 0;
	memset(uart0_TxBuf,0x00,sizeof(uart0_TxBuf));
	memset(uart0_RxBuf,0x00,sizeof(uart0_RxBuf));
	//for (i=0 ; i<MAX_TX_UART0_BUF_SIZE ; i++)
	//	uart0_TxBuf[i] = 0;
	//for (i=0 ; i<MAX_RX_UART0_BUF_SIZE ; i++)
	//	uart0_RxBuf[i] = 0;

	// Initialize TIMER1 for standard 8051 UART clock.
		// Initialize TIMER1 for standard 8051 UART clock.
	PCON  = SMOD0_;		// Enable BaudRate doubler.(Baudrate=38400)
	//PCON  = 0;		// Enable BaudRate doubler.(Baudrate=9600)
	SM00  = 0;
	SM01  = 1;			// Use serial port 0 in mode 1 with 8-bits data.
	REN0  = 1;			// Enable UART0 receiver.
	TMOD  = T1_M1_;		// Use timer 1 in mode 2, 8-bit counter with auto-reload.

//	sysClk = CSREPR & SCS_96M;
	switch (CSREPR & SCS_96M)
	{
		case SCS_48M :
			CKCON |= T1M_;
			TH1 = 0xF3;		// Baud rate = 9600 @ 48MHz.
			break;
		case SCS_96M :
			CKCON |= T1M_;
			TH1 = 0xF3;		// Baud rate = 38400 @ 96MHz.
			//TH1 = 0xE6;		// Baud rate = 19200 @ 96MHz.
			break;
		default :
			TH1 = 0xE6;		// Baud rate = 9600 @ 96MHz.
			break;
	}

	ES0 = 1;				// Enable serial port Interrupt request
	TR1 = 1;				// Run Timer 1
	TI0 = 0;

} /* End of uart0_Init */

/*
 * ----------------------------------------------------------------------------
 * S8_T UART0_PutChar(S8_T c)
 * Purpose : UART0 output function. This function puts one byte data into the
 *           software character buffer.
 * Params  : c - one byte character.
 * Returns : c - one byte character.
 * Note    : none
 * ----------------------------------------------------------------------------
 */
S8_T UART0_PutChar(S8_T c)
{
	U16_T	count = 0;

	do
	{
		ES0	= 0;
		count = uart0_TxCount;
		ES0	= 1;
	} while (count == MAX_TX_UART0_BUF_SIZE);
	uart0_TxBuf[uart0_TxHead] = c;
	ES0	= 0;
	uart0_TxCount++;
	ES0	= 1;
	uart0_TxHead++;
	uart0_TxHead &= MAX_TX_UART0_MASK;

	if (!uart0_TxFlag)
	{
		uart0_TxFlag = 1;
		SBUF0 = uart0_TxBuf[uart0_TxTail];
	}

	return c;
}

/* EXPORTED SUBPROGRAM BODIES */

/*
 * ----------------------------------------------------------------------------
 * S8_T putchar(S8_T c)
 * Purpose : UART putchar function. This function is the entry of putting
 *           characters into software buffer of system's UART ports,
 *           UART0 and UART1. 
 * Params  : c - one byte character to be put.
 * Returns : ch - the same character to be replied.
 * Note    : The default UART port is UART0.
 * ----------------------------------------------------------------------------
 */
S8_T putchar(S8_T c)
{
	S8_T	ch = 0;
	ch = UART0_PutChar(c);

	return ch;
}

/*
 * ----------------------------------------------------------------------------
 * void UART_Init()
 * Purpose : UART initial function. It will call a real initial function
 *           corresponding to the used UART port.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void UART_Init(void)
{
	uart0_Init();
}

/* End of uart.c */

