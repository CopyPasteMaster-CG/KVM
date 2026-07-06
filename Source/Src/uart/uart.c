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
U8_T	uart1_RxBuf[MAX_RX_UART1_BUF_SIZE];
U16_T	uart1_RxHead = 0;
U16_T	uart1_RxTail = 0;
U16_T	uart1_RxCount = 0;

#define HSUR1_CIR				UR2CIR
#define HSUR1_DR				UR2DR

/* LOCAL SUBPROGRAM DECLARATIONS */
static void		uart0_ISR(void);
static void		uart1_ISR(void);
static void		uart0_Init(void);
static void		uart1_Init(void);
static void		hsur1_Write(U8_T addr, U8_T *ptData, U8_T dataLen);
static void		hsur1_Init(void);

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

static void uart1_ISR(void) interrupt UR1_VECTOR
{
	if (RI1)
	{
		uart1_RxBuf[uart1_RxTail] = SBUF1;
		uart1_RxTail++;
		uart1_RxTail &= MAX_RX_UART1_MASK;
		ISR_FIFO[ISR_FIFO_Wp].ISR_Type = ISR_UART1;
		ISR_FIFO[ISR_FIFO_Wp].Data = uart1_RxBuf[(uart1_RxTail - 1) & MAX_RX_UART1_MASK];
		if (++ISR_FIFO_Wp >= ISR_FIFO_DEPTH)
		{
			ISR_FIFO_Wp = 0;
		}
		RI1 = 0;
	}

	if (TI1)
	{
		TI1 = 0;
	}
}

static void hsur1_Write(U8_T addr, U8_T *ptData, U8_T dataLen)
{
	while (dataLen--)
	{
		HSUR1_DR = *(ptData + dataLen);
	}
	HSUR1_CIR = addr;
}

static void hsur1_Init(void)
{
	U8_T reg8b;
	U16_T baud;
	U8_T dll;
	U8_T dlh;

	reg8b = 0;
	hsur1_Write(HSIER, &reg8b, 1);

	reg8b = 0x1A;
	hsur1_Write(HSDPR, &reg8b, 1);

	baud = 0x0020;
	reg8b = HSLCR_DLAB_ENB;
	hsur1_Write(HSLCR, &reg8b, 1);
	dll = (U8_T)(baud & 0x00FF);
	dlh = (U8_T)(baud >> 8);
	hsur1_Write(HSDLLR, &dll, 1);
	hsur1_Write(HSDLHR, &dlh, 1);

	reg8b = (HSLCR_CHAR_8 | HSLCR_STOP_10);
	hsur1_Write(HSLCR, &reg8b, 1);

	reg8b = 0;
	hsur1_Write(HSMCR, &reg8b, 1);
	hsur1_Write(HSDCR, &reg8b, 1);

	reg8b = (HSFCR_HSUART_ENB | HSFCR_FIFOE | HSFCR_RFR | HSFCR_TFR | HSFCR_TRIG_01);
	hsur1_Write(HSFCR, &reg8b, 1);

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

static void uart1_Init(void)
{
	U8_T hwcfg[8];

	ES1 = 0;
	ET2 = 0;
	TR2 = 0;
	T2CON = 0;
	T2IF = 0;

	uart1_RxHead = 0;
	uart1_RxTail = 0;
	uart1_RxCount = 0;
	memset(uart1_RxBuf, 0x00, sizeof(uart1_RxBuf));

	P0_0 = 1;
	P0_1 = 1;
	SCON1 = 0x50;

	RCLK = 1;
	TCLK = 1;
	CKCON |= T2M_;

	if ((CSREPR & SCS_96M) == SCS_96M)
	{
		RLDH = 0xFF;
		RLDL = 0xE6;
	}
	else
	{
		RLDH = 0xFF;
		RLDL = 0xF3;
	}

	TH2 = RLDH;
	TL2 = RLDL;
	RI1 = 0;
	TI1 = 0;
	ES1 = 0;
	TR2 = 1;

	// if (FLASH_InfoWordRead8Byte(0x00000000, hwcfg))
	// {
	// 	printf("HWCFG[001]=%02bx MP0_10_PSEL=%bu\r\n",
	// 		hwcfg[1],
	// 		(U8_T)(hwcfg[1] & BIT0));
	// 	if ((hwcfg[1] & BIT0) == 0)
	// 	{
	// 		printf("WARN: P00/P01 still GPIO, set HWCFG[001].bit0=1 for RXD1/TXD1\r\n");
	// 	}
	// }
	hsur1_Init();
}

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

/*
 * ----------------------------------------------------------------------------
 * S8_T HSUART1_PutChar(S8_T c)
 * Purpose : HSUART1 output function. This function sends one byte data by
 *           polling the transmitter FIFO status.
 * Params  : c - one byte character.
 * Returns : c - one byte character.
 * Note    : none
 * ----------------------------------------------------------------------------
 */
S8_T HSUART1_PutChar(S8_T c)
{
	U8_T lineStatus;

	do
	{
		HSUR1_CIR = HSLSR;
		lineStatus = HSUR1_DR;
	} while ((lineStatus & HSLSR_TFIFO_EMPTY) == 0);

	HSUR1_DR = (U8_T)c;
	HSUR1_CIR = HSTHR;

	return c;
}

#ifndef HSUART
S8_T HSUART_PutChar(S8_T c)
{
	return HSUART1_PutChar(c);
}
#endif

/*
 * ----------------------------------------------------------------------------
 * BOOL HSUART_PutData(U8_T *buf, U8_T len)
 * Purpose : HSUART1 output function. This function sends a data buffer.
 * Params  : buf - data buffer pointer.
 *           len - data length.
 * Returns : TRUE.
 * Note    : none
 * ----------------------------------------------------------------------------
 */
BOOL HSUART_PutData(U8_T *buf, U8_T len)
{
	U8_T i;

	for (i = 0; i < len; i++)
	{
		HSUART1_PutChar(buf[i]);
	}

	return TRUE;
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
	uart1_Init();
}

void UART1_Polling_Receive_Handle(void)
{
	U8_T lineStatus;
	U8_T rxData;

	if (RI1)
	{
		uart1_RxBuf[uart1_RxTail] = SBUF1;
		uart1_RxTail++;
		uart1_RxTail &= MAX_RX_UART1_MASK;
		RI1 = 0;
	}

	HSUR1_CIR = HSLSR;
	lineStatus = HSUR1_DR;

	while (lineStatus & HSLSR_DR_DATA)
	{
		HSUR1_CIR = HSRBR;
		rxData = HSUR1_DR;
		uart1_RxBuf[uart1_RxTail] = rxData;
		uart1_RxTail++;
		uart1_RxTail &= MAX_RX_UART1_MASK;

		HSUR1_CIR = HSLSR;
		lineStatus = HSUR1_DR;
	}
}

/* End of uart.c */

