/*
 ******************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is an proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
 /*============================================================================
 * Module Name: extender.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */

/* INCLUDE FILE SECTION */
/* INCLUDE FILE DECLARATIONS */
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	"project_include.h"

#if (SYSTEM_EXTENDER_SUPPORT)
/* NAMING CONSTANT DECLARATIONS */

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

bit		Transmitter_Data_Toggle;	//The data toggle bit in transmitter side
bit		Receiver_Data_Toggle;		//The data toggle bit in receiver side
bit		Extender_Ready_Flag;
bit		EXTENDER_MSC_BurstOutWait_Flag = 0;

U8_T	Extender_Receive_Data_Parser_State;
U8_T	Extender_Receive_Cmd_Parser_State;

/* GLOBAL VARIABLES DECLARATIONS */
#ifdef EXTENDER_SOF_CNT_CHECK	
U8_T	EXTENDER_SofCount;
#endif
U8_T	Extender_Control_State;
U8_T	rs485_RcvrBufRing[UR2_RX_BUF_SIZE] _at_ UR2_RX_BUF_START_ADDR;
U8_T	EXTENDER_Transmit_OutP;
U8_T	EXTENDER_Transmit_InP;
U8_T	EXTENDER_Transmit_TimeOut_TaskActiveID;
U8_T	EXTENDER_Multi_TR_State;
idata	U16_T	Receive_Buffer_Start;
idata	U16_T	Receive_Buffer_End;
idata	U16_T	Receive_Buffer_Cnt;
U8_T	Extender_Passthrough_Buf[EXTENDER_PAGE_SIZ+sizeof(Extender_Packet_Header_Def)+2];
U8_T	Extender_Receive_Timeout_Active_ID;
Extender_Cmd_Token_Def	Receive_Cmd_Token;
Extender_Transimit_Def	Trainsmit_FIFO[EXTENDER_TRANSMIT_FIFO_MAX];
#if (EXTENDER_TRANSACTION_SERIAL_ID)
U8_T	Trainsmit_Table[EXTENDER_TRANSMIT_FIFO_MAX];
#endif

TASK_ActiveTable_TypeDef Extender_Transmit_Timeout_TASK=
{
	TASK_TYPE_INTERVAL_MS,
	0,		// Task ID from 0~255
	0,		// Task Wait for Semaphore to active
	0,		// Task Event
	0,		// Task transfer parameter
	150,	// Task Interval time from
	1000,	// Task Interval Reload
//	0,		// Task wait for period then perform
};

TASK_ActiveTable_TypeDef Extender_Receive_Timeout_TASK=
{
	TASK_TYPE_INTERVAL_MS,
	0,		// Task ID from 0~255
	0,		// Task Wait for Semaphore to active
	0,		// Task Event
	0,		// Task transfer parameter
	500,	// Task Interval time from
	500,	// Task Interval Reload
//	0,		// Task wait for period then perform
};

#if (SYSTEM_EXTENDER_MSC_SUPPORT)
#if (!SYSTEM_EXTENDER_RECEIVER)
EXTENDER_MSC_Buf_TypeDef	EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BUF_CNT];
U8_T						EXTENDER_MSC_BulkOutHead = 0;
U8_T						EXTENDER_MSC_BulkOutTail = 0;
U8_T						EXTENDER_MSC_BulkOutCount = 0;
#endif //#if (!SYSTEM_EXTENDER_RECEIVER)
EXTENDER_MSC_Buf_TypeDef	EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BUF_CNT];
U8_T						EXTENDER_MSC_BulkInHead = 0;
U8_T						EXTENDER_MSC_BulkInTail = 0;

U8_T						EXTENDER_MSC_StallFlag = 0;
U8_T						EXTENDER_MSC_Devinx = 0xFF;
//U8_T						EXTENDER_MSC_BurstWait = 0;
U16_T						EXTENDER_MSC_TimeOut = 0;

#if (!SYSTEM_USB_HC_BURST)
SWDMA_TypeDef				EXTENDER_MSC_SwdmaBulkOutPtr[EXTENDER_MSC_BUF_CNT];
SWDMA_TypeDef				EXTENDER_MSC_SwdmaBulkInPtr[EXTENDER_MSC_BUF_CNT];
#endif
#endif //#if (SYSTEM_EXTENDER_MSC_SUPPORT)


/* LOCAL VARIABLES DECLARATIONS */
static bit			etdr_Isr = 0;
static U8_T			edtr_tmpArray[3];
static const U8_T	extender_BaudText[15][6] = {"12M", "8M", "6M", "4M", "3M", "2M", "1M", "921K", "115K", "9600", "19200", "38400", "57600"};


/* LOCAL SUBPROGRAM DECLARATIONS */
void TASK_EXTENDER_Receive_TimeOut(void);
#ifdef EXTENDER_SOF_CNT_CHECK	
void EXTENDER_SOF_Check_Update(void);
#endif
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
/* EXTERNAL SUBPROGRAM DECLARATIONS */

/*----------------------------------------------------------------------------
 * Function Name: EXTENDER_Init
 * Purpose: initial the extender system  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void EXTENDER_Init(void)
{
	U16_T	baud;
	U8_T	reg8b;

	//Do the software & memory init
#ifdef EXTENDER_SOF_CNT_CHECK		
	EXTENDER_SofCount = 0;
#endif	
	EXTENDER_Transmit_InP  = 0;
	EXTENDER_Transmit_OutP = 0;
	memset(Trainsmit_FIFO,0x00,sizeof(Trainsmit_FIFO));

	/* Baudrate = 12M for RS485 */
	if (CPU_SysClk == SCS_96M)
	{
		reg8b = HUART_DPR_96M[HUART_BAUD];
		baud = HUART_BAUDRATE_96M[HUART_BAUD];
	}
	else
	{
		reg8b = HUART_DPR_48M[HUART_BAUD];
		baud = HUART_BAUDRATE_48M[HUART_BAUD];
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

#if (SYSTEM_EXTENDER_RS232_MODE)
	printf("@RS-232 @ %s bps\n\r", &extender_BaudText[HUART_BAUD][0]);
	HSUR2_Setup(baud,
				(HSLCR_CHAR_8|HSLCR_STOP_10),
				(HSIER_RDI_ENB|HSIER_RLSI_ENB|HSIER_RBRFI_ENB|HSIER_DOEI_ENB|HSIER_TDCI_ENB),
				(HSFCR_HSUART_ENB|HSFCR_FIFOE|HSFCR_RFR|HSFCR_TFR|HSFCR_TRIG_08),
				0);
#else // For RS485 Mode
	printf("@RS-485 @ %s bps\n\r", &extender_BaudText[HUART_BAUD][0]);
	HSUR2_Setup(baud,
				(HSLCR_CHAR_8|HSLCR_STOP_10),
				(HSIER_RDI_ENB|HSIER_RLSI_ENB|HSIER_RBRFI_ENB|HSIER_DOEI_ENB|HSIER_TDCI_ENB),
				(HSFCR_HSUART_ENB|HSFCR_FIFOE|HSFCR_RFR|HSFCR_TFR|HSFCR_TRIG_08),
				(HSMCR_RTS|HSMCR_RS485_ENB|HSMCR_DEREC_STPHD));
#endif //#if (SYSTEM_EXTENDER_RS232_MODE)

	/* Configure to DMA mode only */
	if (HSUR2_RxDmaControlInit() == FALSE)
	{
		//printf("HSUR2 Rx DMA initial failed!!\n\r");
		return;
	}

	/* Enable Extender Transmitt time out check task */
	Extender_Transmit_Timeout_TASK.Task_ID = TASK_Create(TASK_EXTENDER_Transmit_TimeOut);
	Extender_Receive_Timeout_TASK.Task_ID  = TASK_Create(TASK_EXTENDER_Receive_TimeOut);

#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("Extender_Transmit_Timeout_TASK=%d\n\r",(U16_T)Extender_Transmit_Timeout_TASK.Task_ID);
	printf("Extender_Receive_Timeout_TASK=%d\n\r",(U16_T)Extender_Receive_Timeout_TASK.Task_ID);
#endif

	EXTENDER_Transmit_TimeOut_TaskActiveID = TASK_Run(&Extender_Transmit_Timeout_TASK);
	Extender_Receive_Timeout_Active_ID	   = TASK_Run(&Extender_Receive_Timeout_TASK);
	Task_Active_Table[Extender_Receive_Timeout_Active_ID].Task_Event |= TASK_EVENT_SKIP;

	Receive_Buffer_Start = 0;
	Receive_Buffer_End   = 0;
	EXTENDER_Multi_TR_State = 0;
	Transmitter_Data_Toggle = TRANSMITTER_DATA_TOGGLE0;
	Receiver_Data_Toggle    = RECEIVER_DATA_TOGGLE0;
	Extender_Receive_Data_Parser_State = 0;
	Extender_Receive_Cmd_Parser_State = 0;
	Extender_Ready_Flag = 0;

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) && (SYSTEM_EXTENDER_TRANSMITTER)
	#if (EXTENDER_AUDIO_OUT_BUF_CNT == EXTENDER_AUDIO_IN_BUF_CNT)
	for (reg8b = 0; reg8b < EXTENDER_AUDIO_OUT_BUF_CNT; reg8b++)
	{
		baud = 0x5000 + (U16_T)(EXTENDER_AUDIO_OUT_BUF_SIZE * reg8b) + sizeof(Extender_Data_Packet_Def);
		ETDR_AudioOutBufAddr[reg8b][0] = (U8_T)baud;
		ETDR_AudioOutBufAddr[reg8b][1] = (U8_T)(baud >> 8);		
		baud = 0x5800 + (U16_T)(EXTENDER_AUDIO_IN_BUF_SIZE * reg8b);
		ETDR_AudioInBufAddr[reg8b][0] = (U8_T)baud;
		ETDR_AudioInBufAddr[reg8b][1] = (U8_T)(baud >> 8);
	}
	#else	
	for (reg8b = 0; reg8b < EXTENDER_AUDIO_OUT_BUF_CNT; reg8b++)
	{
		baud = 0x5000 + (U16_T)(EXTENDER_AUDIO_OUT_BUF_SIZE * reg8b) + sizeof(Extender_Data_Packet_Def);
		ETDR_AudioOutBufAddr[reg8b][0] = (U8_T)baud;
		ETDR_AudioOutBufAddr[reg8b][1] = (U8_T)(baud >> 8);
	}
	for (reg8b = 0; reg8b < EXTENDER_AUDIO_IN_BUF_CNT; reg8b++)
	{
		baud = 0x5800 + (U16_T)(EXTENDER_AUDIO_IN_BUF_SIZE * reg8b);
		ETDR_AudioInBufAddr[reg8b][0] = (U8_T)baud;
		ETDR_AudioInBufAddr[reg8b][1] = (U8_T)(baud >> 8);
	}
	#endif
#endif

#if (SYSTEM_EXTENDER_RECEIVER)
	EXTENDER_Receiver_Init();
#endif

#if (SYSTEM_EXTENDER_TRANSMITTER)
	EXTENDER_Transmitter_Init();
#endif

	/* Enable HSUR interrupt */
	HSUR2_Start();
}

/*----------------------------------------------------------------------------
 * Function Name: Extender_Transmit_FIFO_Send
 * Purpose: Task to check the transmitter plug in 
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void Extender_Transmit_FIFO_Send(void)
{
	U8_T	temp8b;
	U16_T XDATA*	pTempShort = (U16_T XDATA*)&edtr_tmpArray[1];

	if (Extender_Ready_Flag==0)
	{
		return;
	}
	
	//2.Check transmitter communicating state
	if (Extender_Control_State & EXTENDER_CONTROL_TRANSMIT)
	{
		return; //Do next time
	}

	//03.Check Transmitter DMA
	/* if TxDMA still busy, give up */
	HSUR2_RegRead(HSDCR, &temp8b, 1);
	if (temp8b & HSDCR_STD)
	{	
		return;
	}

#ifdef KVM_EXTENDER_RECEIVER
	Remote_Link_LED_Flash_Flag = 1;
#endif

	Extender_Control_State |= EXTENDER_CONTROL_TRANSMIT;

#ifdef PLUG_DETECT
	if (UART_Disable_Flag)
	{
		 //Extender_Transmit_Complete_Handle();	
		 TASK_EXTENDER_Transmit_TimeOut();
		 return;
	}
#endif
	//04.Transmitter DMA Setting
	edtr_tmpArray[0] = 0;
	*pTempShort = (U16_T)Trainsmit_FIFO[EXTENDER_Transmit_OutP].Buf;			
	HSUR2_RegWrite(HSTDAR, edtr_tmpArray, 3);
	HSUR2_RegWrite(HSTDBCR, (U8_T *)&Trainsmit_FIFO[EXTENDER_Transmit_OutP].DataLen, 2);	
	temp8b |= HSDCR_STD;
	HSUR2_RegWrite(HSDCR, &temp8b, 1);

	Task_Active_Table[EXTENDER_Transmit_TimeOut_TaskActiveID].Task_Interval.w = 10000;
}

/*----------------------------------------------------------------------------
 * Function Name: Extender_Token_Transmit
 * Purpose: Task for check the transmit time out
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void Extender_Token_Transmit(U8_T *buf,U8_T contrl_attr,U16_T len,U16_T timeout,U8_T cmd_token_header)
{	
	//U8_T index,index2,max;
	
	if (cmd_token_header)
	{
		if (contrl_attr & EXTENDER_CONTROL_DATA_FOLLOW)
			buf[1] |= DATA_WAIT_ACCESS_MASK;
		else
			buf[1] &= ~DATA_WAIT_ACCESS_MASK;
		
		if (buf[0] != EXTENDER_INT_TOKEN)
		{	
			buf[2] = buf[0] ^ buf[1]; 
		}	
	}
	 
	Trainsmit_FIFO[EXTENDER_Transmit_InP].Control  = contrl_attr;
	Trainsmit_FIFO[EXTENDER_Transmit_InP].DataLen  = len; // 3 bytes for transmitter	
	Trainsmit_FIFO[EXTENDER_Transmit_InP].Buf      = buf;
	Trainsmit_FIFO[EXTENDER_Transmit_InP].ErrCnt   = 0;	 
	Trainsmit_FIFO[EXTENDER_Transmit_InP].TimeOut  = timeout; //10ms time out
	//Add the FIFO pointer
	EXTENDER_Transmit_InP ++;
	if (EXTENDER_Transmit_InP >= EXTENDER_TRANSMIT_FIFO_MAX)
	{
		EXTENDER_Transmit_InP = 0;
	}	
	//EXTENDER_Transmit_InP &= (EXTENDER_TRANSMIT_FIFO_MAX -1);

		
	if (EXTENDER_Transmit_InP == EXTENDER_Transmit_OutP)
	{		
		printf(">ERROR:Transmit Overflow\n\r");		
		/*
		printf("Current=%bu\n\r",EXTENDER_Transmit_InP);		
		for (index=0; index < EXTENDER_TRANSMIT_FIFO_MAX ; index++)
		{
			printf("%03bu.",index);
			printf("%02bx ",Trainsmit_FIFO[index].Control);
			printf("%4d "  ,Trainsmit_FIFO[index].DataLen);
            if (Trainsmit_FIFO[index].DataLen <= 9)
                max = Trainsmit_FIFO[index].DataLen;
            else
                max = 9; 
			for (index2=0; index2 < max; index2++)
			{
				printf("%02bx ",Trainsmit_FIFO[index].Buf[index2]);
			}	
			printf("\n\r");			
			
		}
		*/
		//while (1);
		if (EXTENDER_Transmit_InP == 0)
			EXTENDER_Transmit_InP = (EXTENDER_TRANSMIT_FIFO_MAX -1);
		else
			EXTENDER_Transmit_InP --;		
		//EXTENDER_Transmit_InP &= (EXTENDER_TRANSMIT_FIFO_MAX -1);
	}
	
}

/*----------------------------------------------------------------------------
 * Function Name: EXTENDER_Transmit_Update
 * Purpose: 
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void EXTENDER_Transmit_Update(void)
{
	if (Extender_Control_State & (EXTENDER_CONTROL_TRANSMIT|EXTENDER_CONTROL_TRANSMIT_DONE)) //clear the transmitter flag
	{		
		Extender_Control_State &= ~(EXTENDER_CONTROL_TRANSMIT|EXTENDER_CONTROL_TRANSMIT_DONE); //clear the transmitter flag
		if ((Trainsmit_FIFO[EXTENDER_Transmit_OutP].Control & EXTENDER_CONTROL_NONE_FREE) == 0x00)
		{
			malloc_free(Trainsmit_FIFO[EXTENDER_Transmit_OutP].Buf);
		}
#ifdef EXTENDER_SOF_CNT_CHECK	
		EXTENDER_SOF_Check_Update();		
#endif		
		//Add the FIFO pointer
		EXTENDER_Transmit_OutP++;
		if (EXTENDER_Transmit_OutP >= EXTENDER_TRANSMIT_FIFO_MAX)
			EXTENDER_Transmit_OutP = 0;
		//EXTENDER_Transmit_OutP &= (EXTENDER_TRANSMIT_FIFO_MAX -1);
	}
}

/*----------------------------------------------------------------------------
 * Function Name: TASK_EXTENDER_Transmit_TimeOut
 * Purpose: Task for check the transmit time out
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void TASK_EXTENDER_Transmit_TimeOut(void)
{
	if (Extender_Ready_Flag==0)
	{
#if (SYSTEM_EXTENDER_RECEIVER)
		EXTENDER_Get_Total_Receive_Len();
		EXTENDER_Update_Receive_Index(Receive_Buffer_Cnt);
#endif
		Extender_Ready_Flag = 1; //extender is ready to send
	}

	Task_Active_Table[EXTENDER_Transmit_TimeOut_TaskActiveID].Task_Reload.w = 10000; // next for 1 seconds
	if ((Extender_Control_State & EXTENDER_CONTROL_TRANSMIT) == 0x00) //not in transmit operation
	{
		return;
	}

/*	
#if (SYSTEM_EXTENDER_RECEIVER && SYSTEM_EXTENDER_MSC_SUPPORT)
	if (EXTENDER_MSC_BurstWait)
	{
		printf("\n\rTASK_EXTENDER_Transmit_TimeOut\n\r");
		printf("MSC BurstWait: %bx\n\r", Extender_Control_State);
		return;
	}
#endif //#if (SYSTEM_EXTENDER_RECEIVER)
*/

#if (SYSTEM_EXTENDER_RECEIVER)
	Extender_Receiver_Transmit_TimeOut_Check();
#endif 	/* #if (SYSTEM_EXTENDER_RECEIVER) */

	Extender_Receive_Data_Parser_State = 0;
	Extender_Receive_Cmd_Parser_State = 0;

#if (SYSTEM_EXTENDER_RECEIVER)
	if (ExtenderR_Skip_TimeoutUpdate)
	{
		ExtenderR_Skip_TimeoutUpdate = 0;
		return;
	}	
#endif
	EXTENDER_Transmit_Update();
}

/*----------------------------------------------------------------------------
 * Function Name: TASK_EXTENDER_Receive_TimeOut
 * Purpose: Task for check the receive time out
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void TASK_EXTENDER_Receive_TimeOut(void)
{
//	etdr_Isr = EA;
//	EA = 0;
//	if (Receive_Buffer_Start == Receive_Buffer_End)
//	{
//		EA = etdr_Isr;
		//printf("RECEIVE TIME OUT:%x\n\r", Receive_Buffer_Cnt);
		EXTENDER_Update_Receive_Index(Receive_Buffer_Cnt);
		Extender_Receive_Data_Parser_State = 0;
		Extender_Receive_Cmd_Parser_State  = 0;
		EXTENDER_Multi_TR_State			   = 0;
//	}
//	else
//	{
//		EA = etdr_Isr;
//	}

	Task_Active_Table[Extender_Receive_Timeout_Active_ID].Task_Event |= TASK_EVENT_SKIP;
}

/*----------------------------------------------------------------------------
 * Function Name: Extender_Transmit_Complete_Handle
 * Purpose: Handler for transmit completes
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void Extender_Transmit_Complete_Handle(void)
{
#if (SYSTEM_EXTENDER_RECEIVER)	
	Extender_Data_Packet_Def	*data_token;
#if (KVM_EXTENDER_HID_NO_ACK)	
#if (KVM_EXTENDER_HID_SHORT_PACKET==0)	
	Extender_Data_Packet_Def	*data_token;
#endif	
	U8_T	*buf;
#endif

#if (KVM_EXTENDER_HID_SHORT_PACKET)	
	Extender_Data_Int_Packet_Def	*data_int_token;
	Extender_Cmd_Token_Def			*cmd_token;
#endif	
#endif
	
	if (Extender_Control_State & EXTENDER_CONTROL_TRANSMIT)
	{
#if (SYSTEM_EXTENDER_RECEIVER)
		if (Trainsmit_FIFO[EXTENDER_Transmit_OutP].Control & EXTENDER_CONTROL_CLEAR_BUF)
		{
			Trainsmit_FIFO[EXTENDER_Transmit_OutP].Control &= ~EXTENDER_CONTROL_CLEAR_BUF;
			// Updated buffer and Check the buffer if wait to send
#if (SYSTEM_EXTENDER_MSC_SUPPORT)
			ExtenderR_PassthroughMscBurstInComplete();
#endif	
		}
		
#if (KVM_EXTENDER_HID_NO_ACK)
		if (Trainsmit_FIFO[EXTENDER_Transmit_OutP].DataLen > 3)
		{
			buf = Trainsmit_FIFO[EXTENDER_Transmit_OutP].Buf+sizeof(Extender_Cmd_Token_Def);
#if (KVM_EXTENDER_HID_SHORT_PACKET)							
			cmd_token = (Extender_Cmd_Token_Def *)Trainsmit_FIFO[EXTENDER_Transmit_OutP].Buf;					
			if (cmd_token->bCmd_Token == EXTENDER_INT_TOKEN)
			{				
				data_int_token = (Extender_Data_Int_Packet_Def *)(buf);				
				ExtenderR_Interrupt_DataTypeTransfer_Check(data_int_token->bIndex);
			}
#else
			 
			data_token = (Extender_Data_Packet_Def *)(buf);			
			if (data_token->bRequest == EXTENDER_USB_INTR_IN)
			{
				ExtenderR_Interrupt_DataTypeTransfer_Check(data_token->bIndex);
			}	
#endif			
			data_token = (Extender_Data_Packet_Def *)(buf);	
			
			if (data_token->bRequest == EXTENDER_USB_ISO_IN)
			{
				if (ExtenderR_Audio_In_Cnt)
					ExtenderR_Audio_In_Cnt--;				
			}				
		}										
#endif /* #if (KVM_EXTENDER_HID_NO_ACK) */

#endif //#if (SYSTEM_EXTENDER_RECEIVER)

#if (SYSTEM_EXTENDER_TRANSMITTER)
		if (Trainsmit_FIFO[EXTENDER_Transmit_OutP].Control & EXTENDER_CONTROL_CLEAR_BUF)
		{
			Trainsmit_FIFO[EXTENDER_Transmit_OutP].Control &= ~EXTENDER_CONTROL_CLEAR_BUF;
			// Updated buffer and Check the buffer if wait to send
		}
				 
#endif //#if (SYSTEM_EXTENDER_TRANSMITTER)
			
		if (Trainsmit_FIFO[EXTENDER_Transmit_OutP].Control & EXTENDER_CONTROL_TRANSMIT_NO_WAIT)
		{			
			Task_Active_Table[EXTENDER_Transmit_TimeOut_TaskActiveID].Task_Reload.w = 10000; // next for 10 seconds
			EXTENDER_Transmit_Update(); //go for next transmit							
		}
		else
		{
			Extender_Control_State |= EXTENDER_CONTROL_TRANSMIT_DONE;			   
			Task_Active_Table[EXTENDER_Transmit_TimeOut_TaskActiveID].Task_Reload.w = Trainsmit_FIFO[EXTENDER_Transmit_OutP].TimeOut;
			Task_Active_Table[EXTENDER_Transmit_TimeOut_TaskActiveID].Task_Interval.w = Trainsmit_FIFO[EXTENDER_Transmit_OutP].TimeOut;
		}		
	}
}

/*----------------------------------------------------------------------------
 * Function Name: Extender_Retrive_Receive_Buf
 * Purpose: Handler for transmit completes
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void Extender_Retrive_Receive_Buf(U8_T *rbuf,U8_T len)
{
	//U8_T index;
	U16_T start;

	start = Receive_Buffer_Start;
	Receive_Buffer_Start = Extender_CopyRxDmaToBuf(start,len,rbuf);
	//for(index=0; index < len ; index++)
	//{
	//	rbuf[index] = rs485_RcvrBufRing[Receive_Buffer_Start];
	//	Receive_Buffer_Start++;
	//	Receive_Buffer_Start &= (UR2_RX_BUF_SIZE -1);
	//}
}

/*----------------------------------------------------------------------------
 * Function Name: Extender_CopyRxDmaToBuf
 * Purpose: Handler for transmit completes
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
U16_T Extender_CopyRxDmaToBuf(U16_T start,U8_T len,U8_T *rbuf)
{
	idata U8_T index;

	for(index=0; index < len ; index++)
	{
		rbuf[index] = rs485_RcvrBufRing[start];
		start++;
		start &= (UR2_RX_BUF_SIZE -1);
	}
	
	return start;
}

/*----------------------------------------------------------------------------
 * Function Name: Extender_Receive_Cmd_Token_Check
 * Purpose: The function will check the token pattern
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
U8_T Extender_Receive_Cmd_Token_Check(U16_T len,Extender_Cmd_Token_Def *cmd_token,U8_T addr)
{
	bit		loop_break_flag = 0;
	U16_T	index;
	U8_T	token;
	U8_T	token_bit;
	U8_T	token_addr;
	U8_T	result = 0;

	for (index = 0; index < len; index++)
	{		 
		//Check Token command format
		if ((Extender_Receive_Cmd_Parser_State & EXTENDER_CMD_TOKEN_CMD_OK) == 0x00)
		{
			token = rs485_RcvrBufRing[Receive_Buffer_Start];
			token_bit = token & 0xf0;
			token_bit = ((~token_bit) >> 4);
			if (token_bit == (token & 0x0f)) //token byte
			{
				cmd_token->bCmd_Token = token;
				cmd_token->bCmd_CRC = token;
				Extender_Receive_Cmd_Parser_State = EXTENDER_CMD_TOKEN_CMD_OK;
				result = 1;
			}
		}
		else
		{
			//Check Token address valid
			if ((Extender_Receive_Cmd_Parser_State & EXTENDER_CMD_TOKEN_ADDR_OK) == 0x00)
			{
				token_addr = rs485_RcvrBufRing[Receive_Buffer_Start];
				if ((token_addr & TARGET_ADDRESS_MASK) == addr)
				{
					cmd_token->bTarget_Addr = token_addr;
					cmd_token->bCmd_CRC ^= token_addr;
					Extender_Receive_Cmd_Parser_State |= EXTENDER_CMD_TOKEN_ADDR_OK;
					result = 1;
					if (token_addr & DATA_WAIT_ACCESS_MASK) //there is another data packet after this command
					{
						Extender_Receive_Cmd_Parser_State |= EXTENDER_DATA_PACKET_WAIT; //parser has beeon done
					}
				}
				else
				{
					Extender_Receive_Cmd_Parser_State = 0; //clear the token ok flag
					result = 0;
					token = token_addr;					
				}
			}
			else
			{
				if (cmd_token->bCmd_Token == EXTENDER_INT_TOKEN)
				{
					cmd_token->bCmd_CRC = rs485_RcvrBufRing[Receive_Buffer_Start]; //ingore the CRC check, this will be the length byte					
					goto COMMAND_TOKEN_HEADER_CRC;
				}
				else
				{			
					if (cmd_token->bCmd_CRC == rs485_RcvrBufRing[Receive_Buffer_Start])
					{
COMMAND_TOKEN_HEADER_CRC:						
						Extender_Receive_Cmd_Parser_State &= EXTENDER_DATA_PACKET_WAIT;
						Extender_Receive_Cmd_Parser_State |= EXTENDER_CMD_TOKEN_PARSER_OK; //parser has beeon done
						result = 1;
						loop_break_flag = 1;
					}
					else
					{
						Extender_Receive_Cmd_Parser_State = 0;
						result=0;
					}
				}
			}
		}

		//Adjust the Cnt & Index
		if (Receive_Buffer_Cnt)
			Receive_Buffer_Cnt--;

		Receive_Buffer_Start++;
		Receive_Buffer_Start &= (UR2_RX_BUF_SIZE -1);

		if (loop_break_flag)
		{
			break;
		}
	}

	if (result)
	{
		if ((Extender_Receive_Cmd_Parser_State & EXTENDER_CMD_TOKEN_PARSER_OK)== 0x00)
		{
#if (SYSTEM_EXTENDER_TRANSMITTER)
			if (Extender_Receive_Cmd_Parser_State == EXTENDER_CMD_TOKEN_CMD_OK)
			{
				if ((token != EXTENDER_CMD_TOKEN) || (token != EXTENDER_SOF_TOKEN) || (token != EXTENDER_ACK_TOKEN))
				{
					Extender_Receive_Cmd_Parser_State = 0;
					return 0;
				}
			}
#endif
#if (SYSTEM_EXTENDER_RECEIVER)
			if (Extender_Receive_Cmd_Parser_State == EXTENDER_CMD_TOKEN_CMD_OK)
			{
				if ((token != EXTENDER_ACK_TOKEN) || (token != EXTENDER_NAK_TOKEN) || (token != EXTENDER_STALL_TOKEN))
				{
					Extender_Receive_Cmd_Parser_State = 0;
					return 0;
				}
			}
#endif
			result = 0;
			Extender_Receive_Cmd_Parser_State |= EXTENDER_CMD_TOKEN_LEN_SHORT;
			Extender_Receive_Start_TimeOut_Check();
		}
	}

	return result;
}

/*----------------------------------------------------------------------------
 * Function Name: Extender_Receive_Data_Token_Parser
 * Purpose: The function will check the token pattern
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
U8_T Extender_Receive_Data_Token_Parser(Extender_Cmd_Handle_Def *setup)
{
	U16_T data_len;

Extender_Recive_Data_Parser_Find_DATA_TOKEN:
	
	//1.Handle the frist byte => Data Token byte or bytes			
	if ((Extender_Receive_Data_Parser_State & EXTENDER_DATA_PACKET_TOKEN_OK) == 0x00)
	{
		if (Receive_Buffer_Cnt == 0)
			return 0;
					
		setup->iData_Token_Index = Receive_Buffer_Start;
		Receive_Buffer_Cnt--;
		if (setup->Cmd_Token.bCmd_Token == EXTENDER_INT_TOKEN)
		{			
			Extender_Retrive_Receive_Buf((U8_T *)&(setup->Data_Token.bIndex),1); //retrive first byte and check about the data token ....			
			setup->iRemain_Len = sizeof(Extender_Data_Int_Packet_Def)+1; //atleaet need to received data length
		}
		else
		{			
			//2.Handle the frist byte => Data Token byte or bytes
			Extender_Retrive_Receive_Buf((U8_T *)&(setup->Data_Token.bData_Token),1); //retrive first byte and check about the data token ....
			if (setup->Data_Token.bData_Token != EXTENDER_DATA0_TOKEN)
			{
				if (setup->Data_Token.bData_Token != EXTENDER_DATA1_TOKEN)
				{
					//since data token is not correct,then go next byte
					if (Receive_Buffer_Cnt)
					{
						goto Extender_Recive_Data_Parser_Find_DATA_TOKEN;
					}
					else
					{
						setup->iRemain_Len = sizeof(Extender_Data_Packet_Def)+2;
						//printf("data_token_err:%d\n\r",Receive_Buffer_Cnt);
						return 0;
					}
				}
			}
			setup->iRemain_Len = sizeof(Extender_Data_Packet_Def)+1; //atleaet need to received data length
		}
		Extender_Receive_Data_Parser_State = EXTENDER_DATA_PACKET_TOKEN_OK; //clear all other codition.		
	}

	//this prevent the input is not long enough
	//What if it is time out? (transmitter reset or cat5 wire plug off)
	if (Receive_Buffer_Cnt < setup->iRemain_Len)
	{
		Extender_Receive_Data_Parser_State |= EXTENDER_DATA_PACKET_LEN_SHORT;
		return 0;
	}

	// Data section process
	// How to make sure the data token is correct?
	if ((Extender_Receive_Data_Parser_State & EXTENDER_DATA_TOKEN_PARSER_OK) == 0x00)
	{
		Extender_Receive_Data_Parser_State |= EXTENDER_DATA_TOKEN_PARSER_OK;
		if (setup->Cmd_Token.bCmd_Token == EXTENDER_INT_TOKEN)
		{
			Extender_Retrive_Receive_Buf((U8_T *)&(setup->Data_Token.bValue),1);			
			setup->Data_Token.iLen = setup->Cmd_Token.bCmd_CRC;
			setup->iRecBuf_Index = Receive_Buffer_Start; //the start position for Data Segment					
			setup->iRemain_Len = setup->Cmd_Token.bCmd_CRC+2; // at leaset 2 bytes remai
			Receive_Buffer_Cnt--;
		}
		else
		{			
			Extender_Retrive_Receive_Buf((U8_T *)&(setup->Data_Token.iLen),(sizeof(Extender_Data_Packet_Def)-1));		

			setup->iRecBuf_Index = Receive_Buffer_Start; //the start position for Data Segment
			Receive_Buffer_Cnt = Receive_Buffer_Cnt - (sizeof(Extender_Data_Packet_Def)-1);
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
#if (SYSTEM_EXTENDER_RECEIVER)
			if (setup->Data_Token.bRequest == EXTENDER_USB_ISO_OUT)
#else /* #if (SYSTEM_EXTENDER_TRANSMITTER) */
			if (setup->Data_Token.bRequest == EXTENDER_USB_ISO_IN)
#endif
			{
				setup->iRemain_Len = setup->Data_Token.iLen;
			}
			else
#endif //#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
			{
				setup->iRemain_Len = setup->Data_Token.iLen+2; // at leaset 2 bytes remain
			}
		}
	}

	//Check the data valid
	if (Receive_Buffer_Cnt < setup->iRemain_Len)
	{
		Extender_Receive_Data_Parser_State |= EXTENDER_DATA_PACKET_LEN_SHORT;		
		return 0; //wait for next time
	}

	//Data section processing	
	EXTENDER_Update_Receive_Index(setup->Data_Token.iLen);	

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
#if (SYSTEM_EXTENDER_RECEIVER)
	if (setup->Data_Token.bRequest == EXTENDER_USB_ISO_OUT)
#else /* #if (SYSTEM_EXTENDER_TRANSMITTER) */
	if (setup->Data_Token.bRequest == EXTENDER_USB_ISO_IN)
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */
	{
		Extender_Receive_Data_Parser_State = (EXTENDER_DATA_PACKET_CRC_OK|EXTENDER_DATA_PACKET_SECTION_OK);
	}
	else
#endif //#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	{
		//CRC byte processing
		Extender_Retrive_Receive_Buf(setup->bData_CRC,2);
		Receive_Buffer_Cnt -= 2; //2 byte crc

		/* Check the crc */
		data_len = setup->Data_Token.iLen;
		if (setup->Cmd_Token.bCmd_Token == EXTENDER_INT_TOKEN)
		{			
			data_len += sizeof(Extender_Data_Int_Packet_Def);
		}
		else
		{
			data_len += sizeof(Extender_Data_Packet_Def);
		}		
		if ((setup->Data_Token.bRequest & 0xf0) == EXTENDER_USB_CRC_SKIP_MASK)
		{				
			edtr_tmpArray[0] = setup->bData_CRC[0];
			edtr_tmpArray[1] = setup->bData_CRC[1];			
		}
		else	
		{	
#ifdef BR19200				
			if (setup->Cmd_Token.bCmd_Token == EXTENDER_INT_TOKEN)
			{	
				edtr_tmpArray[0] = setup->bData_CRC[0];
				edtr_tmpArray[1] = setup->bData_CRC[1];				
			}
			else	
#endif			
			{		
				Externder_CRC_Calculate(setup->iData_Token_Index,data_len,rs485_RcvrBufRing,edtr_tmpArray,0);	//crc 2 byte,Ring Buffer
			}	
		}
		/* CRC Check */
		if ((setup->bData_CRC[0] == edtr_tmpArray[0]) && ((setup->bData_CRC[1] == edtr_tmpArray[1])))
		{			
			Extender_Receive_Data_Parser_State = (EXTENDER_DATA_PACKET_CRC_OK|EXTENDER_DATA_PACKET_SECTION_OK);
		}
		//else
		//{
		//	printf("CE[%02bx.%02bx,%02bx.%02bx]\n\r",setup->bData_CRC[0],setup->bData_CRC[1],edtr_tmpArray[0],edtr_tmpArray[1]);												
		//}		 	
	}
	return 1;
}


/*----------------------------------------------------------------------------
 * Function Name: EXTENDER_Update_Receive_Index
 * Purpose: After Send out the transmit fifo, the receiver will go into 
 *          listen mode for receive the responsed from transmiiter, after 
 *          DMA generate the interrupt for receive time out, this function 
 *          will be perform to handle the command responsed
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void EXTENDER_Update_Receive_Index(U16_T len)
{
    Receive_Buffer_Start += len;
	Receive_Buffer_Start &= (UR2_RX_BUF_SIZE -1);
	Receive_Buffer_Cnt -= len;
}


/*
 *--------------------------------------------------------------------------------
 * void HSUR2_CopyRxDmaToApp(U8_T *pAppData, U16_T appGetLen)
 * Purpose: If the up-layer application has a memory resource as a RX buffer,
 *          this function can help to copy the received data block from the firmware
 *          receiving buffer ring to up-layer buffer by a SW DMA function. After
 *          completing the DMA copying, the function, HSUR2_RxDmaSwReadPointerUpdated,
 *          will be called to release the RX buffer ring. This function alwaye need
 *          a real memory resource at the up-layer application. But the application
 *          can also use other API functions to get the RX buffer ring status
 *          and can release the RX buffer ring.
 * Params : pAppData : A point to indicate the up-layer application data buffer that
 *                     storing received characters.
 *          appFreeLen : The maximum length of the up-layer application data buffer.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void EXTENDER_CopyRxDmaToApp(U16_T start,U16_T appGetLen,U8_T *pAppData)
{
	U16_T	tempCount1, tempCount2;

	if (appGetLen == 0)
		return;

	if ((start + appGetLen) > UR2_RX_BUF_SIZE)
	{
		/* copy data to application from pRxCur to pRxEnd */
		tempCount1 = UR2_RX_BUF_SIZE - start;

		if (tempCount1 < EXTENDER_DMA_LENGTH)
			Extender_CopyRxDmaToBuf(start,(U8_T)tempCount1,pAppData);
		else
		{
			DMA_GrantXdata(pAppData,&rs485_RcvrBufRing[start], tempCount1);
		}
		/* copy data to application for last length from pRxStart */
		tempCount2 = appGetLen - tempCount1;
		pAppData += tempCount1;
		if (tempCount2)
		{
			if (tempCount2 < EXTENDER_DMA_LENGTH)
				Extender_CopyRxDmaToBuf(0,(U8_T)tempCount2,pAppData);
			else
			{
				DMA_GrantXdata(pAppData, &rs485_RcvrBufRing[0], tempCount2);
			}
		}
	}
	else
	{
		DMA_GrantXdata(pAppData,&rs485_RcvrBufRing[start],appGetLen);
	}
}

/**----------------------------------------------------------------------------
 * Function Name: Extender_Packet_Data_Phase2 
 * Purpose:  
 * Params :$devinx:the hc devinx, the virtual vdevice_id is same with hc
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
U16_T Extender_Packet_Data_Phase2(U8_T packet_len,U8_T *header,U8_T *data_buf,U16_T data_len)
{
	U16_T	len;
	
	if (data_len && (data_buf != NULL))
	{
		DMA_GrantXdata((header+packet_len),data_buf,data_len);
	}

	//CRC
	len = data_len + packet_len;
	Externder_CRC_Calculate(0,(U16_T)len,header,edtr_tmpArray,1); //offset is 0,2 byte crc,None Ring buffer type	

	//CRC write	
	header[len]= edtr_tmpArray[0];
	len++;
	header[len]= edtr_tmpArray[1];

	return (len+1);
}

#if (SYSTEM_EXTENDER_RECEIVER)
/**----------------------------------------------------------------------------
 * Function Name: Externder_Packet_Header 
 * Purpose:  
 * Params :$devinx:the hc devinx, the virtual vdevice_id is same with hc
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void Externder_Packet_Header(Extender_Packet_Header_Def *header,U8_T *token)
{
	header->Cmd_Token.bCmd_Token 	= token[0];
	header->Cmd_Token.bTarget_Addr 	= token[1];
	header->Cmd_Token.bCmd_CRC = token[0] ^ token[1];
}


/*----------------------------------------------------------------------------
 * Function Name: Extender_Packet_Int_Data 
 * Purpose: Transfer the edid to remote transmitter
 * Params :$devinx:the hc devinx, the virtual vdevice_id is same with hc
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
U8_T Extender_Packet_Int_Data(Extender_Data_Int_Packet_Def *header,U8_T data_len,U8_T *data_buf,U8_T bIndex,U8_T bValue)
{
	header->bIndex = bIndex;
	header->bValue = bValue;

	return ((U8_T)Extender_Packet_Data_Phase2(sizeof(Extender_Data_Int_Packet_Def),(U8_T *)header,data_buf,(U16_T)data_len));
}	
#endif /* SYSTEM_EXTENDER_RECEIVER */

/*----------------------------------------------------------------------------
 * Function Name: Extender_Packet_Data 
 * Purpose: Transfer the edid to remote transmitter
 * Params :$devinx:the hc devinx, the virtual vdevice_id is same with hc
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
U16_T Extender_Packet_Data(Extender_Data_Packet_Def *header,U16_T data_len,U8_T *data_buf,U8_T bRequest,U8_T bIndex,U8_T bValue)
{
	if (Transmitter_Data_Toggle==0)
	{
		header->bData_Token = EXTENDER_DATA0_TOKEN; //data pid
		Transmitter_Data_Toggle=1;
	}
	else
	{
		header->bData_Token = EXTENDER_DATA1_TOKEN; //data pid
		Transmitter_Data_Toggle=0;
	}

	header->iLen = data_len;
	header->bRequest = bRequest;
	header->bIndex = bIndex;
	header->bValue = bValue;
	
	return (Extender_Packet_Data_Phase2(sizeof(Extender_Data_Packet_Def),(U8_T *)header,data_buf,(U16_T)data_len));
}

/*----------------------------------------------------------------------------
 * Function Name: Externder_CRC_Calculate 
 * Purpose: Transfer the edid to remote transmitter
 * Params :$devinx:the hc devinx, the virtual vdevice_id is same with hc
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void Externder_CRC_Calculate(U16_T start,U16_T len,U8_T *buf,U8_T *crc,U8_T buftype)
{
	idata U16_T		index;	
	idata U16_T		bufinx = start;

	crc[0] = buf[bufinx];
	bufinx++;
	if (!buftype)
		bufinx &= (UR2_RX_BUF_SIZE -1);
	crc[1] = buf[bufinx];
	bufinx++;
	if (!buftype)
		bufinx &= (UR2_RX_BUF_SIZE -1);

	for (index = 2; index < len; index++)
	{
		if (index & 0x01)
			crc[1] ^= buf[bufinx];
		else
			crc[0] ^= buf[bufinx];

		bufinx++;
		if (!buftype)
			bufinx &= (UR2_RX_BUF_SIZE -1);
	}
}

/*----------------------------------------------------------------------------
 * Function Name: EXTENDER_Get_Total_Receive_Len 
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void EXTENDER_Get_Total_Receive_Len(void)
{
	EXTINT4_DISABLE;
	if (Receive_Buffer_End > Receive_Buffer_Start)
	{
		Receive_Buffer_Cnt = Receive_Buffer_End - Receive_Buffer_Start;	
	}	
	else
	{
		if (Receive_Buffer_End == Receive_Buffer_Start)
		{	
			Receive_Buffer_Cnt = 0;		   
		}
		else
		{
			Receive_Buffer_Cnt = (UR2_RX_BUF_SIZE - Receive_Buffer_Start) + Receive_Buffer_End;	
		}	    
	}
	EXTINT4_ENABLE;
}

/*----------------------------------------------------------------------------
 * Function Name: Extender_Receive_Parser_State_Reset
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void Extender_Receive_Parser_State_Reset(void)
{
	EXTENDER_Multi_TR_State &= ~(EXTENDER_RECEIVE_START|EXTENDER_RECEIVE_WAIT);
	Extender_Receive_Cmd_Parser_State  = 0; //clear the command token,so can go next command
	Extender_Receive_Data_Parser_State = 0; //clear the command token,so can go next command
}

/*----------------------------------------------------------------------------
 * Function Name: vod Extender_Receive_Start_TimeOut_Check(void)
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void Extender_Receive_Start_TimeOut_Check(void)
{
	Task_Active_Table[Extender_Receive_Timeout_Active_ID].Task_Interval.w = EXTENDER_RECEIVE_TIME_OUT;
	Task_Active_Table[Extender_Receive_Timeout_Active_ID].Task_Event &= ~TASK_EVENT_SKIP;
}

/*----------------------------------------------------------------------------
 * Function Name: vod Extender_Receive_Stop_TimeOut_Check(void)
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void Extender_Receive_Stop_TimeOut_Check(void)
{
	Task_Active_Table[Extender_Receive_Timeout_Active_ID].Task_Event |= TASK_EVENT_SKIP;
}

/*----------------------------------------------------------------------------
 * Function Name: Extender_CopyDataFromRxBuffer
 * Purpose: Handler for transmit completes
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void Extender_CopyDataFromRxBuffer(U16_T start,U16_T len,U8_T *rbuf)
{
	if (len < EXTENDER_DMA_LENGTH)
		Extender_CopyRxDmaToBuf(start,(U8_T)len,rbuf);
	else
		EXTENDER_CopyRxDmaToApp(start,len,rbuf);
}

#if (SYSTEM_EXTENDER_MSC_SUPPORT)
/*
 *----------------------------------------------------------------------------
 * Function: Extender_MSC_Init
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
void Extender_MSC_Init(U8_T keep_devinx)
{
	U8_T				index;
	
#if (!SYSTEM_EXTENDER_RECEIVER)
	if (keep_devinx==0)
	{	
		EXTENDER_MSC_Devinx = 0xFF;
	}	
	EXTENDER_MSC_BulkOutHead = 0;
	EXTENDER_MSC_BulkOutTail = 0;
	EXTENDER_MSC_BulkOutCount = 0;
	for (index=0; index < EXTENDER_MSC_BUF_CNT; index ++)
	{
		EXTENDER_MSC_BulkOutBuf[index].bufUsingFlag = 0;
		EXTENDER_MSC_BulkOutBuf[index].devIndex = 0;
		EXTENDER_MSC_BulkOutBuf[index].payloadLen = 0;
		EXTENDER_MSC_BulkOutBuf[index].reqType = 0;
	}
#else
	keep_devinx = 0;
#endif //#if (!SYSTEM_EXTENDER_RECEIVER)

	EXTENDER_MSC_BulkInHead = 0;
	EXTENDER_MSC_BulkInTail = 0;
	for (index=0; index < EXTENDER_MSC_BUF_CNT; index ++)
	{
		EXTENDER_MSC_BulkInBuf[index].bufUsingFlag = 0;
		EXTENDER_MSC_BulkInBuf[index].devIndex = 0;
		EXTENDER_MSC_BulkInBuf[index].payloadLen = 0;
		EXTENDER_MSC_BulkInBuf[index].reqType = 0;
	}
	
	EXTENDER_MSC_StallFlag = 0;
	//EXTENDER_MSC_BurstWait = 0;
	EXTENDER_MSC_TimeOut = 0;
	EXTENDER_MSC_BurstOutWait_Flag = 0;
}
#endif //#if (SYSTEM_EXTENDER_MSC_SUPPORT)

#ifdef EXTENDER_SOF_CNT_CHECK	
void EXTENDER_SOF_Check_Update(void)
{
	if (Trainsmit_FIFO[EXTENDER_Transmit_OutP].Buf[0] == EXTENDER_SOF_TOKEN)
	{
		EXTENDER_SofCount--;
	}
}
#endif

#endif /* SYSTEM_EXTENDER_RECEIVER */
/* End of Extender.c */
