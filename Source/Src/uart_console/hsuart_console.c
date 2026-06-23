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
 * Module Name : uart_console.c
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
#include    <ctype.h>
#include	<string.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	"project_include.h"

/* STATIC VARIABLE DECLARATIONS */
#ifdef HSUART_CONSOLE

//P A R T 1
#define    HUART_12M			0
#define    HUART_8M				1
#define    HUART_6M				2
#define    HUART_4M				3
#define    HUART_3M				4
#define    HUART_2M				5
#define    HUART_1M				6
#define    HUART_921K			7
#define    HUART_115K			8
#define    HUART_9600			9
#define    HUART_19200			10
#define    HUART_38400			11
#define    HUART_57600			12

#define CONSOLE_HUART_BAUD	HUART_9600

U16_T  HUART_BAUDRATE_96M[] = {
	0x0001, 	/* 12M	  */
	0x0001, 	/*  8M	  */
	0x0001, 	/*  6M	  */
	0x0001, 	/*  4M	  */
	0x0001, 	/*  3M	  */	
	0x0001, 	/*  2M	  */
	0x0001, 	/*  1M    */
	0x0004, 	/* 921600 */
	0x0020, 	/* 115200 */
	0x0064, 	/* 9600   */
	0x0032,		/* 19200   */
	0x0019,		/* 38400   */
	0x0011,		/* 57600   */
};
									
U16_T  HUART_BAUDRATE_48M[] = {
	0x0001, 	/* 12M	  */
	0x0001, 	/*  8M	  */
	0x0001, 	/*  6M	  */
	0x0001, 	/*  4M	  */
	0x0001, 	/*  3M	  */
	0x0001, 	/*  2M	  */
	0x0001, 	/*  1M    */
	0x0002, 	/* 921600 */
	0x0010, 	/* 115200 */
	0x0032, 	/* 9600   */
	0x0019,		/* 19200   */
	0x000C,		/* 38400   */
	0x0008,		/* 57600   */
};

U8_T  HUART_DPR_96M[] = {
//	0x06,		//16M(bps)
	0x08,		//12M(bps)
	0x0C,		//8M(bps)
	0x10,		//6M(bps)
	0x18,		//4M(bps)
	0x20,		//3M(bps)
	0x30,		//2M(bps)
	0x5E,		//1M(bps)
	0x1A,		//921600(bps)
	0x1A,		//115200(bps)
	0x64,		//9600(bps)
	0x64,		//19200
	0x64,		//38400
	0x64,		//57600
};

U8_T  HUART_DPR_48M[] = {
	0x04,		//12M(bps)
	0x06,		//8M(bps)
	0x08,		//6M(bps)
	0x0C,		//4M(bps)
	0x10,		//3M(bps)
	0x18,		//2M(bps)
	0x2F,		//1M(bps)
	0x1A,		//921600(bps)
	0x1A,		//115200(bps)
	0x64,		//9600(bps)
	0x64,		//19200
	0x64,		//38400
	0x64,		//57600
};


//P A R T 2 
#define STAGE_READ_WAIT			0
#define STAGE_READ_HANDLE		1
#define STAGE_SEND				2
#define STAGE_SEND_HANDLE		3
#define STAGE_SEND_WAIT			4
#define STAGE_SEND_WAIT			4

//------------------------------------------
//$ Console Behavior define
//------------------------------------------
#define AUTO_0A				1
#define CHAR_ECHO			1
#define CHAR_TERMINATE1		0x0d
#define CHAR_TERMINATE2		0x0a
#define AUTO_TOUPPER		1
#define AUTO_TOLOWER		0
#define UartStrMaxLength	10
#define UartStrMaxCnt		6

void UartConsole_PortSwitch_Handle(void);
void UartConsole_PowerSavingMode_Handle(void);
void UartConsole_AutoscanMode_Handle(void);
void UartConsole_AutoscanInterval_Handle(void);
void UartConsole_Help_Handle(void);
void UartConsole_Config_Handle(void);

//P A R T 1 -----------------------------
static U8_T		hsur2IntrEnbType;
static U8_T		hsur2FifoCtrl;
static U16_T	hsur2ErrCount;

U8_T	hsuart_TxBuf[MAX_TX_HSUART_BUF_SIZE];
U16_T	hsuart_TxHead = 0;
U16_T	hsuart_TxTail = 0;
volatile U16_T	hsuart_TxCount = 0;
U8_T	hsuart_TxFlag = 0;
U8_T	hsuart_RxBuf[MAX_RX_HSUART_BUF_SIZE];
U16_T	hsuart_RxHead = 0;
U16_T	hsuart_RxTail = 0;
U16_T	hsuart_RxCount = 0;

static const U8_T	Console_BaudText[15][6] = {"12M", "8M", "6M", "4M", "3M", "2M", "1M", "921K", "115K", "9600", "19200", "38400", "57600"};

//P A R T 2 -----------------------------
bit		UartConsole_R_Queue = 0;
U8_T	UartConsole_Buf[MAX_RX_HSUART_BUF_SIZE];
U16_T	UartConsole_RxHead  = 0;
U16_T	UartConsole_RxTail  = 0;
U16_T	UartConsole_RxCount = 0;
U16_T	UartConsole_P = 0;
U8_T    UartConsole_ProcessID;
U8_T    UartConsole_ProcessStage;
U8_T    UartConsole_MsgLen;
U8_T	UartStr[UartStrMaxCnt][UartStrMaxLength];
U16_T   WordP;
U8_T	WordCnt,WordLen;

UartConsole_Handle_TypeDef  Console_Handle_Tab[]=
{
	0,"PSW"		,1,UartConsole_PortSwitch_Handle,
	0,"AS"		,1,UartConsole_AutoscanMode_Handle,
	0,"ASI"		,1,UartConsole_AutoscanInterval_Handle,
	0,"PWSM" 	,1,UartConsole_PowerSavingMode_Handle,
	0,"HELP"	,0,UartConsole_Help_Handle,
	0,"?"		,0,UartConsole_Help_Handle,
	0,"CONFIG"	,0,UartConsole_Config_Handle,
};

#define UART_CONSOLE_TAB_MAX	(sizeof(Console_Handle_Tab) /  sizeof(UartConsole_Handle_TypeDef))
		
#define CMD_GET_USERNAME		0
#define CMD_GET_PASSWORD		1
#define CMD_SEND_USERNAME		2
#define CMD_SEND_PASSWORD		3
#define CMD_SEND_SSID			4
#define CMD_SEND_CONFIG			5		

/* LOCAL SUBPROGRAM DECLARATIONS */
BOOL HSUART_PutStr(char *str);
BOOL HSUR2_RegWrite(U8_T addr, U8_T *ptData, U8_T dataLen);
BOOL HSUR2_RegRead(U8_T addr, U8_T *ptData, U8_T dataLen);

void UartConsole_StageStart(void);		
U8_T UartConsole_GetNextWord(void);
void UartConsole_Prompt(void);
/* LOCAL SUBPROGRAM BODIES */
/*
 * ----------------------------------------------------------------------------
 * void HSUART_Init()
 * Purpose : HSUART initial function. It will call a real initial function
 *           corresponding to the used HSUART port.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void HSUART_Init(void)
{
	U16_T	baud;
	U8_T	reg8b;
	
	//Do the software & memory init
	hsuart_TxHead = 0;
	hsuart_TxTail = 0;
	hsuart_TxCount = 0;
	hsuart_TxFlag = 0;
	hsuart_RxHead = 0;
	hsuart_RxTail = 0;
	hsuart_RxCount = 0;
	memset(hsuart_TxBuf,0x00,sizeof(hsuart_TxBuf));
	memset(hsuart_RxBuf,0x00,sizeof(hsuart_RxBuf));
		
	/* Baudrate = 12M for RS232 */
	if (CPU_SysClk == SCS_96M)
	{
		reg8b = HUART_DPR_96M[CONSOLE_HUART_BAUD];
		baud = HUART_BAUDRATE_96M[CONSOLE_HUART_BAUD];
	}
	else
	{
		reg8b = HUART_DPR_48M[CONSOLE_HUART_BAUD];
		baud = HUART_BAUDRATE_48M[CONSOLE_HUART_BAUD];
	}

	HSUR2_RegWrite(HSDPR, &reg8b, 1); //setup the clock rate
	//For Extender Setting
	//Data Bit 8, Stop bit 1,None parity
	//Interupt-Receive when receive buffer level changed
	//         Line status 
	//         Receive buffer full
	//         Transmitte complete
	//         DMA error 
	//         Flow character received complete

#ifdef CONSOLE_RS232
	printf("@RS-232 @ %s bps\n\r", &Console_BaudText[CONSOLE_HUART_BAUD][0]);
	HSUR2_Setup(baud,
				(HSLCR_CHAR_8|HSLCR_STOP_10),
				(HSIER_RDI_ENB|HSIER_TFEI_ENB|HSIER_RLSI_ENB|HSIER_RBRFI_ENB),
				(HSFCR_HSUART_ENB|HSFCR_FIFOE|HSFCR_RFR|HSFCR_TFR|HSFCR_TRIG_01),
				0);
#else // For RS485 Mode
	printf("@RS-485 @ %s bps\n\r", &Console_BaudText[CONSOLE_HUART_BAUD][0]);
	HSUR2_Setup(baud,
				(HSLCR_CHAR_8|HSLCR_STOP_10),
				(HSIER_RDI_ENB|HSIER_TFEI_ENB|HSIER_RLSI_ENB|HSIER_RBRFI_ENB),
				(HSFCR_HSUART_ENB|HSFCR_FIFOE|HSFCR_RFR|HSFCR_TFR|HSFCR_TRIG_01),
				(HSMCR_RTS|HSMCR_RS485_ENB|HSMCR_DEREC_STPHD));
#endif //#if (SYSTEM_EXTENDER_RS232_MODE)

	/* Enable HSUR interrupt */
	HSUR2_Start();
}

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
		/*
		UR2CIR = HSRBWP;
		reg16b = (U16_T)UR2DR;
		reg16b |= (U16_T)UR2DR << 8;
		UR2DR = (U8_T)reg16b;
		UR2DR = (U8_T)(reg16b >> 8);
		UR2CIR = HSRBRP;
		ptHsur2RxDmaRingSwRead = (U8_T XDATA*)reg16b;
		*/
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

/*
 * ----------------------------------------------------------------------------
 * void HSUR2_IntrEntryFunc(void)
 * Purpose : HSUART interrupt service routine. For sending out, it puts data
 *           from software buffer into hardware serial buffer register (SBUF0).
 *           For receiving, it gets data from hardware serial buffer register
 *           (SBUF0) and stores into software buffer.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
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
	
	if ((intrStatus & HSIIR_RLS_INTR) == HSIIR_RLS_INTR)
	{
		hsur2_ReadLsr();
		return;
	}
	//else if (intrStatus == HSIIR_DMAS_INTR)
	if (intrStatus == HSIIR_DMAS_INTR)
	{ /* DMA interrupt flag */
		UR2CIR = HSDSR;
		ur2DmaIntrStatus = UR2DR;
		
		//Assing to ISR handle 
		ISR_FIFO[ISR_FIFO_Wp].ISR_Type = ISR_HUART;
		ISR_FIFO[ISR_FIFO_Wp].Data = ur2DmaIntrStatus;				
		
		ISR_FIFO_Wp++;
		if (ISR_FIFO_Wp >= ISR_FIFO_DEPTH)
			ISR_FIFO_Wp = 0;			
		return;
	}	
	//else if ((intrStatus == HSIIR_RD_TI_INTR) || (intrStatus == HSIIR_RD_TRIG_INTR))	
	if ((intrStatus & HSIIR_RD_TI_INTR) == HSIIR_RD_TRIG_INTR)	
	{ /* Receiver Timeout */		
		// Assing to ISR handle 		
		UR2CIR = HSRBR;								
		hsuart_RxBuf[hsuart_RxTail] = UR2DR;
		
		hsuart_RxTail++;
		hsuart_RxTail &= MAX_RX_HSUART_MASK;
		ISR_FIFO[ISR_FIFO_Wp].ISR_Type = ISR_UART1;
		//ISR_FIFO[ISR_FIFO_Wp].Data = intrStatus;
		if (++ISR_FIFO_Wp >= ISR_FIFO_DEPTH)
		{
			ISR_FIFO_Wp = 0;
		}	
	}
	
	if ((intrStatus & HSIIR_TFE_INTR) == HSIIR_TFE_INTR)
	{ /* Transmitte complete */				
		if (hsuart_TxCount > 0)
		{
			hsuart_TxTail++;
			hsuart_TxTail &= MAX_TX_HSUART_MASK;		
			hsuart_TxCount--;
			if (hsuart_TxCount > 0)			
			{	
				UR2DR = hsuart_TxBuf[hsuart_TxTail];
				UR2CIR = HSTHR;						
			}
			else
			{
				hsuart_TxFlag = 0;
			}				
		}
		else
		{	
			hsuart_TxFlag = 0;
		}	
	}
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
	//HSUART_PutStr("IER=%02x\r",(U16_T)hsur2IntrEnbType);
	HSUR2_RegWrite(HSIER, &hsur2IntrEnbType, 1);
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
	//hsur2FifoCtrl = fCtrl | HSFCR_RSTOP; // Enable the RSTOP bit to stop the data characters into RX FIFO.
	hsur2FifoCtrl = fCtrl; // Enable the RSTOP bit to stop the data characters into RX FIFO.
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
 * ----------------------------------------------------------------------------
 * S8_T HSUART_PutChar(S8_T c)
 * Purpose : HSUART output function. This function puts one byte data into the
 *           software character buffer.
 * Params  : c - one byte character.
 * Returns : c - one byte character.
 * Note    : none
 * ----------------------------------------------------------------------------
 */
S8_T HSUART_PutChar(S8_T c)
{
	U16_T	count = 0;	
	
	do
	{	
		count = hsuart_TxCount;			
	} while (count == MAX_TX_HSUART_MASK);		
	hsuart_TxBuf[hsuart_TxHead] = c;	
	hsuart_TxCount++;		
	hsuart_TxHead++;
	hsuart_TxHead &= MAX_TX_HSUART_MASK;

	if (!hsuart_TxFlag)
	{	
		hsuart_TxFlag = 1;
		EINT4 = 0;	
		UR2DR = hsuart_TxBuf[hsuart_TxTail];;	
		UR2CIR = HSTHR;	
		EINT4 = 1;	
	}	
	return c;
}

/*
 * ----------------------------------------------------------------------------
 * S8_T *HSUART_PutStr(S8_T *format,...)  
 * Purpose : HSUART initial function. It will call a real initial function
 *           corresponding to the used HSUART port.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
BOOL HSUART_PutStr(char *str)
{               
    while(*str != 0x00) 
    {          
        HSUART_PutChar(*str);
		str++;
    }      
	
	return 1;
}    



// P A R T 2 -------------------------------------------------------------------
/*
 * ----------------------------------------------------------------------------
 * void UartConsole_Recieve_Handle(void)
 * Purpose : UART0 console receive handle 
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void UartConsole_Init(void)
{
	
	//TASK_UART_Console_Receive_Handle_ID = TASK_Create(TASK_UART_Console_Receive_Handle);
	//TASK_UART_Console_Receive_Handle_ActiveID = 0;		
	UartConsole_ProcessID = 0;
	UartConsole_StageStart();		
	HSUART_Init();
}		

/*
 * ----------------------------------------------------------------------------
 * void UartConsole_StageStart(void)
 * Purpose : start a new uart console event 
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void UartConsole_StageStart(void)
{	
	UartConsole_RxCount = 0;
	UartConsole_ProcessStage = STAGE_READ_WAIT;			
}

/*
 * ----------------------------------------------------------------------------
 * void UartConsole_NextStage(void)
 * Purpose : start a new uart console event 
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void UartConsole_NextStage(void)
{
	UartConsole_ProcessID++;
	if (UartConsole_ProcessID >= UART_CONSOLE_TAB_MAX)
	{
		UartConsole_ProcessID = 0;
	}	
	else
	{
		UartConsole_StageStart();
	}			
}

/*
 * ----------------------------------------------------------------------------
 * void UartConsole_Recieve_Handle(void)
 * Purpose : UART0 console receive handle 
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void UartConsole_Recieve_Handle(void)
{	
	bit terminate=0;
	U8_T  cmd_get=0;
	U8_T  uart_data;
	
	//printf("(%d-%d)\n\r",UartConsole_RxTail,hsuart_RxTail);		
	while (UartConsole_RxTail != hsuart_RxTail)	
	{		
		//if (uart0_RxCount != MAX_RX_HSUART_BUF_SIZE) 
		{

#if (AUTO_TOUPPER)
			UartConsole_Buf[UartConsole_RxCount]=toupper(hsuart_RxBuf[hsuart_RxHead]);		
#else
	#if (AUTO_TOLOWER)
			UartConsole_Buf[UartConsole_RxCount]=tolower(hsuart_RxBuf[hsuart_RxHead]);		
	#else
			UartConsole_Buf[UartConsole_RxCount]=hsuart_RxBuf[hsuart_RxHead];		
	#endif
#endif						

			if ((UartConsole_Buf[UartConsole_RxCount] == CHAR_TERMINATE1) ||(UartConsole_Buf[UartConsole_RxCount] == CHAR_TERMINATE2))			
			{		
				terminate=1;				
//#if (CHAR_ECHO) && (AUTO_0A) 							
//				HSUART_PutStr("\n");			
//#endif			
			}	
			
			#if (CHAR_ECHO)
			//HSUART_PutStr("%c-%bx",(U8_T)UartConsole_Buf[UartConsole_RxCount],(U8_T)UartConsole_Buf[UartConsole_RxCount]);
			if (UartConsole_Buf[UartConsole_RxCount] != CHAR_TERMINATE2)	
			{				
				if (UartConsole_Buf[UartConsole_RxCount] == CHAR_TERMINATE1)	
				{
					HSUART_PutStr("\n\r");
				}	
				else
				{
					HSUART_PutChar(UartConsole_Buf[UartConsole_RxCount]);
				}					
			}			
			#endif					
			
			uart_data = UartConsole_Buf[UartConsole_RxCount];
			UartConsole_RxCount++; //add the receive buffer lenght
			
			if (UartConsole_RxCount >=  MAX_RX_UART0_BUF_SIZE)
				UartConsole_RxCount = 0;
			//----------------------------------------------------------
			//update the ring buffer end pointer for RX ring buffer
			hsuart_RxHead++;
			hsuart_RxHead &= MAX_RX_HSUART_MASK;			
			
			UartConsole_RxTail++;			
			UartConsole_RxTail &= MAX_RX_HSUART_MASK;					
			
			if (terminate)
			{				
				if (uart_data != CHAR_TERMINATE2)
				{				
					WordCnt = 0;	
					WordP = 0;				
					if (UartConsole_GetNextWord())
					{
						for (UartConsole_ProcessID = 0 ; UartConsole_ProcessID < UART_CONSOLE_TAB_MAX ; UartConsole_ProcessID++)
						{
							UartConsole_MsgLen = (U8_T)strlen(Console_Handle_Tab[UartConsole_ProcessID].WaitMsg);
							if (UartConsole_MsgLen == WordLen)
							{
								if (memcmp(UartConsole_Buf,Console_Handle_Tab[UartConsole_ProcessID].WaitMsg,WordLen) == 0) //if string is same
								{
									cmd_get = 1;
									if (Console_Handle_Tab[UartConsole_ProcessID].HandleFunction)
									{	
										Console_Handle_Tab[UartConsole_ProcessID].HandleFunction();
									}	
									break;
								}	
							}	
						}
						
						if (cmd_get == 0)
						{
							HSUART_PutStr("??\n\r");
							UartConsole_Prompt();
						}						
							
					}			
					else
					{
						#if !(CHAR_ECHO)
						HSUART_PutStr("\n\r"); //responsed command
						#endif
						UartConsole_Prompt();
					}					
				}	
				UartConsole_RxCount = 0;								
			}	
		}		
	} /* End of if(RI0) */
}

/*
 * ----------------------------------------------------------------------------
 * void UartConsole_SetSSID_Handle(void)
 * Purpose : send out set ssid command
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
U8_T UartConsole_GetNextWord(void)
{
	U16_T startp;
	
	
	startp = WordP;
	for (; WordP < UartConsole_RxCount ; WordP++)
	{
		if ((UartConsole_Buf[WordP] == ' ') || (UartConsole_Buf[WordP] == CHAR_TERMINATE1) ||  (UartConsole_Buf[WordP] == CHAR_TERMINATE2))
		{
			if (WordP != startp) //skip the space
			{				
				WordLen = WordP - startp;				
				//Copy String
				if(WordCnt < (UartStrMaxCnt-1))
				{
					UartStr[WordCnt][0] = WordLen; //keep word lenght
					if (WordLen < UartStrMaxLength)						
						memcpy(&UartStr[WordCnt][1],&UartConsole_Buf[startp],WordLen); //copy word content
					else
						memcpy(&UartStr[WordCnt][1],&UartConsole_Buf[startp],UartStrMaxLength); //copy word content
					UartStr[WordCnt][WordLen+1] = 0; //word terminate					
				}			
				WordCnt++;
				return 1;
			}				
			else
			{
				if (UartConsole_Buf[WordP] == ' ')
				{
					startp++; //move the start pointer to next byte
				}					
			}				
		}					
	}
	return 0;
}



//-------------------------------------------------------------------------------------------------------------
//
//  UART  COMMAND HANDLE SECTION
//
//-------------------------------------------------------------------------------------------------------------
void UartConsole_Ack(void)
{
	HSUART_PutStr(">OK\n\r"); //responsed command
	UartConsole_Prompt();
}

void UartConsole_Err(void)
{
	HSUART_PutStr(">ERROR\n\r"); //responsed command
	UartConsole_Prompt();
}

void UartConsole_Prompt(void)
{
	HSUART_PutStr(">"); //responsed command
}

U8_T UartConsole_GetParamter(U8_T para_cnt)
{
	U8_T wordcnt;
	
	for (wordcnt=0; wordcnt < para_cnt ; wordcnt++)
	{
		if (UartConsole_GetNextWord() == 0)
		{
			break;
		}				
	}
	
	return wordcnt;
}	

/*
 * ----------------------------------------------------------------------------
 * void UartConsole_Help_Handle(void)
 * Purpose : Transmitter Screen Mode Setting
 * Params  : none
 * Returns : none
 * Note    : Mode 0 1/2- 2 Screen with 1 2
 *			 Mode 1 
 * ----------------------------------------------------------------------------
 */
void UartConsole_Help_Handle(void)
{
	HSUART_PutStr(">Command\n\r");
	HSUART_PutStr(" [1].portsw 1/4\n\r");
	HSUART_PutStr("     port switch control\n\r");
	HSUART_PutStr("     1/4-port id\n\r");	
	HSUART_PutStr(" [2].pwsm 2/3\n\r");
	HSUART_PutStr("     power saving mode\n\r");
	HSUART_PutStr("     2-stop mode\n\r");
	HSUART_PutStr("     3-sleep mode\n\r");	
	HSUART_PutStr(" [3].as 0/1\n\r");
	HSUART_PutStr("     autoscan mode\n\r");
	HSUART_PutStr("     0-stop\n\r");
	HSUART_PutStr("     1-start\n\r");	
	HSUART_PutStr(" [4].asi 5~999\n\r");
	HSUART_PutStr("     autoscan interval time\n\r");
	HSUART_PutStr("     5~999 seconds\n\r");	
	UartConsole_Prompt();
}

/*
 * ----------------------------------------------------------------------------
 * void UartConsole_Config_Handle(void)
 * Purpose : Transmitter Screen Mode Setting
 * Params  : none
 * Returns : none
 * Note    : Mode 0 1/2- 2 Screen with 1 2
 *			 Mode 1 
 * ----------------------------------------------------------------------------
 */
void UartConsole_Config_Handle(void)
{	 
	UartConsole_Prompt();
}

/*
 * ----------------------------------------------------------------------------
 * void UartConsole_PortSwitch_Handle(void)
 * Purpose : port switch handle
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void UartConsole_PortSwitch_Handle(void)
{
	U8_T wordcnt;
	U8_T pid;
	
	HSUART_PutStr("Port Switch Handle\n\r");	
	wordcnt=UartConsole_GetParamter(1);
		
	if (wordcnt == 0)
	{
		UartConsole_Ack();		
		KVM_Jump_Next_Power_Port(KVM_CurrentHost);
	}		
	else
	{
		pid = atoi(&UartStr[1][1]);
		if ((pid <= KVM_MAX_PORT) && (pid > 0))
		{	
			UartConsole_Ack();		
			KVM_NextHost = pid-1;
			KVM_Console_Port_Jump(KVM_NextHost);
		}	
		else
		{
			UartConsole_Err();		
		}			
	}			
}	

/*
 * ----------------------------------------------------------------------------
 * void UartConsole_PowerSavingMode_Handle(void)
 * Purpose : Power Saving handle
 *			 SYSTEM_POWER_STOP_MODE = 2
 *			 SYSTEM_POWER_SLEEP_MODE= 3
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void UartConsole_PowerSavingMode_Handle(void)
{
	U8_T wordcnt;	
	U8_T  pid;
	
	HSUART_PutStr("Power Saving Mode Handle\n\r");	
	wordcnt=UartConsole_GetParamter(1);
	
	if (wordcnt == 0)
	{
		UartConsole_Err();		
	}		
	else
	{
		pid = atoi(&UartStr[1][1]);
		if (pid == SYSTEM_POWER_STOP_MODE)
		{
			KVM_Flash.PowerSavingMode = SYSTEM_POWER_STOP_MODE; 	// Stop Mode
		}
		else if (pid == SYSTEM_POWER_SLEEP_MODE)
		{	
			KVM_Flash.PowerSavingMode = SYSTEM_POWER_SLEEP_MODE; 	// Deep Sleep Mode
		}	
		else			
		{
			UartConsole_Err();
			return;
		}
		UartConsole_Ack();		
	}			
}

void UartConsole_AutoscanMode_Handle(void)
{
	U8_T wordcnt;	
	U8_T  pid;
	
	HSUART_PutStr("Autoscan mode control\n\r");	
	wordcnt=UartConsole_GetParamter(1);
	if (wordcnt == 0)
	{
		UartConsole_Err();		
	}		
	else
	{
		pid = atoi(&UartStr[1][1]);
		KVM_Autoscan_Mode_Contorl(pid);			
		UartConsole_Ack();		
	}				
}

void UartConsole_AutoscanInterval_Handle(void)
{
	U8_T wordcnt;	
	U16_T  pid;
	
	HSUART_PutStr("Autoscan mode interval\n\r");	
	wordcnt=UartConsole_GetParamter(1);
	if (wordcnt == 0)
	{
		UartConsole_Err();		
	}		
	else
	{
		pid = atoi(&UartStr[1][1]);
		if ((pid > 0) && (pid <= 999))
		{
			KVM_Flash.AutoScanInterval = pid;
			STORAGE_Write(sizeof(KVM_Flash),(U8_T *)&KVM_Flash);
		}			
	}	
}	
#endif  /* #ifdef HSUART_CONSOLE */
/* End of hsuart_console.c */

