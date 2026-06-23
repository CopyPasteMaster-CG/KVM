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
 * Module Name: cascade_master.c
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

#if (SYSTEM_CASCADE_MASTER)
/* NAMING CONSTANT DECLARATIONS */
//For Slave Input Ready, use P2 for GPIO Interrupt,
const U16_T	SLAVE_MASK[] = {0x000f,0x00f0,0x0f00,0xf000};
const U8_T	InReadyPinShift[] = {4,4,4,0};
const SPI_InReadyPort_SFR[] = {MISC_GP2TR,MISC_GP2TR,MISC_GP2TR,MISC_GP2TR};
const SPI_InReadyPort_DER[] = {MISC_GP2DER,MISC_GP2DER,MISC_GP2DER,MISC_GP2DER};
const GPIO_TypeDef  *SPI_InReadyPort[] = { 
											GPIOC, //P2
											GPIOC, //P2
											GPIOC, //P2
											GPIOC  //P2											
										 };

const GPIO_PinTypeDef SPI_InReadyPin[] = {
											GPIO_Pin_6,
											GPIO_Pin_6,
											GPIO_Pin_2,											
											GPIO_Pin_4,											
										 };										 


#define  SPI_INREADY_MASK	(SPI_InReadyPin[0]|SPI_InReadyPin[1]|SPI_InReadyPin[2]|SPI_InReadyPin[3])		
#define  SLAVE_RESET_GPIO_PIN	P1_5
#define  SLAVE_IAP_GPIO_PIN		P1_4
/* GLOBAL VARIABLES DECLARATIONS */

//SPI Opcode
#if CCMST_ORIGINAL_1
bit		CascadeM_Broadcast_Flag;
#endif
bit		CascadeM_PassThrough_Buf_In_Used_Flag;
bit		CascadeM_Device_Mount_Start_Flag;
bit		CascadeM_Transmit_Processing_Flag;
bit     CaascaseM_System_Report_Ready_Flag = 0;
//U8_T 	CascadeM_Device_Mount_Index;
U8_T 	Master_Cmd_RBRS[]  = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40};
U8_T 	Master_Cmd_DRMEM[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x50};
U8_T 	Master_Cmd_DWMEM[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xD0};
Firmware_Version_Def  FirmwareVersion[4];
idata volatile U8_T 	CascadeM_Transmit_OutP;
idata volatile U8_T 	CascadeM_Transmit_InP;
U8_T 	CascadeM_Passthrough_State[USB_HC_MAX_DEVICE];
//U8_T 	CascadeM_Slave_Connect_State;
Cascade_Transimit_Def Trainsmit_FIFO[CASCADE_TRANSMIT_FIFO_MAX];

/*
For Device Mount Control 
*/
Cascade_Mount_Que_Def	CasecadeM_Mount[CASCADEM_MOUNT_QUEUE_MAX];
U8_T 	CascadeM_Mount_Startp;
U8_T 	CascadeM_Mount_Endp;
U8_T    CascadeM_Control_State;
U8_T    CascadeM_Console_State;
U8_T	TASK_CascadeM_USBHC_PassThrough_ID;

U8_T    Remote_Device_Report_Pup_Stack[USBDC_DEVICE_MAX];
U8_T	Remote_Devinx_Pup_Index;
U8_T	Remote_Devinx_Pup_Start;

U8_T    CascadeM_Salve_Cnt;
U8_T	CascadeM_Current_Slave;
U8_T    CascadeM_KVM_Max_port;
U8_T	CascadeM_Rx_Buf[CASCADEM_RX_BUF_SIZE]; 
Cascade_System_State     CascadeM_Sys_State;
Cascade_Data_Packet_Def  CascadeM_SPI_RBRS_Buf[MAXIMUM_SLAVE];
U8_T	TASK_CascadeM_DeviceMount_Control_ID;

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
#if (SYSTEM_CASCADE_AUDIO_BUF_CNT)
Cascade_ISO_Data_Def CADR_USB_Audio_Buf[SYSTEM_CASCADE_AUDIO_BUF_CNT];
volatile U8_T	CADR_USB_HC_Audio_Inx;
volatile U8_T	CCMST_AudioOutRdPtr;
volatile U8_T   CCMST_AudioOutWrPtr;
U8_T	CADR_USB_AudioBufAddr[SYSTEM_CASCADE_AUDIO_BUF_CNT][2];
bit     CCMST_AudioOutBusy;
#endif /* #if (SYSTEM_CASCADE_AUDIO_BUF_CNT) */
#endif /* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */

#if (PROJECT_USB_GENERIC_HID_ENABLE)
Cascade_GHID_Data_Def  Cascade_GHID_DataQ[2];
#endif /* #if (PROJECT_USB_GENERIC_HID_ENABLE) */

TASK_ActiveTable_TypeDef TASK_CascadeM_Devcie_Rereport_TASK=
{		
	TASK_TYPE_EVENT,
	0, 	// Task ID from 0~255
	0, 	// Task Wait for Semaphore to active
	0, 	// Task Event
	0, 	// Task transfer parameter
	0,  // Task Interval time from 
	0,  // Task Interval Reload
	//0, 	// Task wait for period then perform
}; 

TASK_ActiveTable_TypeDef Cascade_Transmit_Timeout_TASK=
{		
	TASK_TYPE_INTERVAL_MS,
	0, 	 // Task ID from 0~255
	0, 	 // Task Wait for Semaphore to active
	0, 	 // Task Event
	0, 	 // Task transfer parameter
	150, // Task Interval time from 
	1000,// Task Interval Reload
	//0, 	 // Task wait for period then perform
};

TASK_ActiveTable_TypeDef Cascade_Receive_Timeout_TASK=
{		
	TASK_TYPE_INTERVAL_MS,
	0, 	 // Task ID from 0~255
	0, 	 // Task Wait for Semaphore to active
	0, 	 // Task Event
	0, 	 // Task transfer parameter
	500,  // Task Interval time from 
	500,  // Task Interval Reload
	//0, 	 // Task wait for period then perform
};

U8_T CascadeM_Transmit_TimeOut_TaskActiveID;
U8_T CascadeM_Receive_TimeOut_TaskActiveID;
U8_T CascadeM_SystemPort_TaskID;

/* LOCAL VARIABLES DECLARATIONS */
static CASCADE_PacketHeader *pPACKET;

/* LOCAL SUBPROGRAM DECLARATIONS */
void  TASK_CASCADEM_Receive_TimeOut(void);
void  TASK_CASCADEM_Transmit_TimeOut(void);
void  CascadeM_Transmit_USB_HID_Setting(U8_T slave,U8_T devinx);
void  TASK_CascadeM_SystemPort_Report(void);
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
/* EXTERNAL SUBPROGRAM DECLARATIONS */

/*----------------------------------------------------------------------------
 * Function Name: Cascade_Master_Init
 * Purpose: initial the cascade spi master
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Master_Init(void)
{			
	U16_T	reg16b = 0;
	U8_T	reg8p = 0;
	
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
#if (SYSTEM_CASCADE_AUDIO_BUF_CNT)
	CascadeM_Audio_Device_State_Reset();	
#endif /* #if (SYSTEM_CASCADE_AUDIO_BUF_CNT) */
#endif /* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */	

#if (PROJECT_USB_GENERIC_HID_ENABLE)
	CascadeM_GHid_Device_Buf_Init();
#endif	
	
	CascadeM_Transmit_Processing_Flag = 0;
	Cascade_Transmit_Timeout_TASK.Task_ID = TASK_Create(TASK_CASCADEM_Transmit_TimeOut);		
	Cascade_Receive_Timeout_TASK.Task_ID  = TASK_Create(TASK_CASCADEM_Receive_TimeOut);
	TASK_CascadeM_DeviceMount_Control_ID  = TASK_Create(TASK_CascadeM_Device_Mount_Control);
	CascadeM_SystemPort_TaskID			  = TASK_Create(TASK_CascadeM_SystemPort_Report);

#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("Cascade_Transmit_Timeout_TASK=%d\n\r",(U16_T)Cascade_Transmit_Timeout_TASK.Task_ID);	
	printf("Cascade_Receive_Timeout_TASK=%d\n\r",(U16_T)Cascade_Receive_Timeout_TASK.Task_ID);		
#endif
		
	CascadeM_Transmit_TimeOut_TaskActiveID  = TASK_Run(&Cascade_Transmit_Timeout_TASK);
	CascadeM_Receive_TimeOut_TaskActiveID   = TASK_Run(&Cascade_Receive_Timeout_TASK);	
#if (TaskControlEnable)
	Task_Active_Table[CascadeM_Transmit_TimeOut_TaskActiveID].taskCtrl |= TASK_EVENT_SKIP;
	Task_Active_Table[CascadeM_Receive_TimeOut_TaskActiveID].taskCtrl |= TASK_EVENT_SKIP;		
#else	
	Task_Active_Table[CascadeM_Transmit_TimeOut_TaskActiveID].Task_Event |= TASK_EVENT_SKIP; //trans timeout not skip		
	Task_Active_Table[CascadeM_Receive_TimeOut_TaskActiveID].Task_Event |= TASK_EVENT_SKIP;		
#endif 	
	TASK_CascadeM_USBHC_PassThrough_ID	  = TASK_Create(TASK_CascadeM_USBHC_PassThrough);		
	
#if (SYSTEM_TASK_DUMP_SUPPORT)
	//printf("CascadeM_Connection_Check_TASK=%d\n\r",(U16_T)CascadeM_Connection_Check_TASK.Task_ID);		
	printf("TASK_CascadeM_USBHC_PassThrough_ID=%d\n\r",(U16_T)TASK_CascadeM_USBHC_PassThrough_ID);	
	printf("TASK_CascadeM_Devcie_Rereport_TASK=%d\n\r",(U16_T)TASK_CascadeM_Devcie_Rereport_TASK.Task_ID);		
#endif		

	Remote_Devinx_Pup_Index = 0;
	CascadeM_Transmit_OutP  = 0;
	CascadeM_Transmit_InP   = 0;		
	CascadeM_Console_State	= 0;				
	CascadeM_Device_Mount_Start_Flag = 0;
	
	memset(CascadeM_Passthrough_State,0x00,sizeof(CascadeM_Passthrough_State));
	memset(Trainsmit_FIFO,0x00,sizeof(Trainsmit_FIFO));						
	memset(&CascadeM_Sys_State,0x00,sizeof(CascadeM_Sys_State));
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	
	CascadeM_Control_State = 0; 	 
	CascadeM_KVM_Max_port  = 4;
	CascadeM_Salve_Cnt 	   = 0;
 
	/*Hardware SPI Releative Init                   */
	//--------------------------------------------------------------------------------------	
	//SPI_MstModeSetup(U8_T ctrlCmd, U8_T intrEnb, U8_T baudrate, U8_T opDelay, U8_T slvSel)	
	//--------------------------------------------------------------------------------------	
	SPI_MstModeSetup(
						0xb0, //SPICMR_MST_ENB|SPICMR_ASS_AUTO|SPICMR_SSOE_ENB(Master enable,slave_select_auto,enable_slave_select 
					 	(SPIMIER_SDTCFIE_ENB|SPIMIER_DMAERRIE_ENB), //SPIMIER_SDTCFIE_ENB|SPIMIER_DMAERRIE_ENB(DMA Transfer complete,Transfer DMA ERROR)
					 	SPI_24M, //SPI_24M
					 	0x00,
					 	SPI_SELECT_PIN_TO_SS_VALUE[0] // disable the decoder, and select the default slave address 0
					 );
	/*-----------------------------------------------
	/*DMA Setting for Master 						*/	
	SPI_MstDmaSetting(
						0x01,//0x00-SPI delay before sclk
					  	0x00 //0x00-SPI delay trnansmission
					 );
					 
	/*-----------------------------------------------
	/*Bounded the Receiver Ring Buffer				*/	
	reg16b = (U16_T)CascadeM_Rx_Buf;
	SPI_MstRegWrite(SDRSA, (U8_T *)&reg16b, 2);

    /*-----------------------------------------------*/
    //GPIO the interrupt setting
    CascadeM_Slave_Input_Ready_Init();
    
    CascadeM_Mount_Startp  = 0;
	CascadeM_Mount_Endp	= 0;

#if (SYSTEM_MSC_DEVICE_SUPPORT)	
	CascadeM_MSC_Init();
#endif
	
#if (SYSTEM_CASCADE_API_SUPPORT)
	Cascade_API_Init();
#endif

	memcpy(FirmwareVersion[0].Version,VERSION_STRING,5);
	memcpy(FirmwareVersion[0].Date   ,DATE_STRING	 ,10);	
	TASK_Active(TASK_TYPE_INTERVAL_MS,CascadeM_SystemPort_TaskID,0,0,900,0);

#if (SYSTEM_KVM_BOX_CASCADE_SUPPORT)
	CascadeBoxM_Init();
#endif	
}

/*----------------------------------------------------------------------------
 * Function Name: CascadeM_Transfer_Complete_Handle
 * Purpose: Subroutine to handle the SPI transmitter or receiver complete
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Transfer_Complete_Handle(void)
{	 
	if (CascadeM_Control_State & CASCADE_CONTROL_WAIT_LEN)
	{	
		CascadeM_Control_State &= ~CASCADE_CONTROL_WAIT_LEN;
		//Read in Rx buffer counter			
#if (TaskControlEnable)
		Task_Active_Table[CascadeM_Receive_TimeOut_TaskActiveID].taskCtrl |= TASK_EVENT_SKIP;
#else		
		Task_Active_Table[CascadeM_Receive_TimeOut_TaskActiveID].Task_Event |= TASK_EVENT_SKIP; // stop receiver time out counter			
#endif			
		CascadeM_Transmit_Update(); //go for next transmit		
	}	
	else if (CascadeM_Control_State & CASCADE_CONTROL_RECEIVE) //handle the spi read done processing
	{
		Task_Active_Table[CascadeM_Receive_TimeOut_TaskActiveID].Task_Reload.w = 10000; // next for 10 seconds
#if (TaskControlEnable)			
		Task_Active_Table[CascadeM_Receive_TimeOut_TaskActiveID].taskCtrl |= TASK_EVENT_SKIP;	
#else
		Task_Active_Table[CascadeM_Receive_TimeOut_TaskActiveID].Task_Event |= TASK_EVENT_SKIP;	
#endif		
		CascadeM_Receive_Complete_Handle();			
		CascadeM_Transmit_Update(); //Go for next transmit		
	}	
	else if (CascadeM_Control_State & CASCADE_CONTROL_TRANSMIT) //Handke the spi write done processing
	{
		Task_Active_Table[CascadeM_Transmit_TimeOut_TaskActiveID].Task_Reload.w = 10000; // next for 10 seconds
#if (TaskControlEnable)		
		Task_Active_Table[CascadeM_Transmit_TimeOut_TaskActiveID].taskCtrl |= TASK_EVENT_SKIP;			
#else
		Task_Active_Table[CascadeM_Transmit_TimeOut_TaskActiveID].Task_Event |= TASK_EVENT_SKIP;			
#endif
		CascadeM_Handle_After_Transmit_Complete();
		CascadeM_Transmit_Update(); //go for next transmit			
	}			
}

/*----------------------------------------------------------------------------
 * Function Name: CascadeM_Transmit_FIFO_Send
 * Purpose: After slave pull the inready pin low, master need use SPI bus
 *          to read data in.
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Slave_InRquest_Handle(U8_T pinstate)
{
	U8_T i;
	
	for (i=1; i<MAXIMUM_SLAVE; i++)
	{
		if (pinstate & SPI_InReadyPin[i])
		{			
			if (CascadeM_Salve_Cnt < i)
			{	
				CascadeM_Salve_Cnt = i;
				CascadeM_KVM_Max_port = (CascadeM_Salve_Cnt+1) << 2;    	
			}	
			
			CascadeM_SPI_RBRS_Buf[i].bCommand = CASCADE_SPI_READ;
			CascadeM_SPI_RBRS_Buf[i].iLen = 0;
			CascadeM_Cmd_Transmit((U8_T *)&CascadeM_SPI_RBRS_Buf[i], (CASCADE_CONTROL_RBRS|CASCADE_CONTROL_NONE_FREE), CASCADE_PACKET_HEADER_LEN, 10, i);

			/* Clear interrupt flag */
			pinstate &= ~SPI_InReadyPin[i];
			if (pinstate==0)
				break;						
		}				
	}
}


/*----------------------------------------------------------------------------
 * Function Name: Extender_Token_Transmit
 * Purpose: Task for check the transmit time out
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Cmd_Transmit(U8_T *buf,U8_T contrl_attr,U16_T len,U16_T timeout,U8_T target)
{
	bit isr_tmp;
	
	isr_tmp = EINT2;
	EINT2 = OFF;
	
	Trainsmit_FIFO[CascadeM_Transmit_InP].Control  = contrl_attr;
	Trainsmit_FIFO[CascadeM_Transmit_InP].Target   = target;	
	Trainsmit_FIFO[CascadeM_Transmit_InP].DataLen  = len;
	Trainsmit_FIFO[CascadeM_Transmit_InP].Buf      = buf;	
	Trainsmit_FIFO[CascadeM_Transmit_InP].ErrCnt   = 0;
	Trainsmit_FIFO[CascadeM_Transmit_InP].TimeOut  = timeout;

	CascadeM_Transmit_InP++;
	if (CascadeM_Transmit_InP >= CASCADE_TRANSMIT_FIFO_MAX)
		CascadeM_Transmit_InP = 0;
		
	EINT2 = isr_tmp;		
}

/*----------------------------------------------------------------------------
 * Function Name: CascadeM_Transmit_FIFO_Send
 * Purpose: Send out the SPI Transcation
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Transmit_FIFO_Send(void)
{
	U8_T tmp8;
	U16_T tmp16;	
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) || (SYSTEM_MSC_DEVICE_SUPPORT)
	CASCADE_PacketHeader *packet;	
#endif	

	/* Check SPI master is idle */
	if (CascadeM_Control_State & (CASCADE_CONTROL_TRANSMIT|CASCADE_CONTROL_WAIT_LEN|CASCADE_CONTROL_RECEIVE))	
	{
		return;
	}
	
	if (SPI_IsMstSpiBusRdy() == 0)
	{
		return;
	}	
	
	/* Process data read first */
	if (Trainsmit_FIFO[CascadeM_Transmit_OutP].Control & CASCADE_CONTROL_RBRS)
	{
		Trainsmit_FIFO[CascadeM_Transmit_OutP].Control &= ~CASCADE_CONTROL_RBRS;
		CascadeM_Current_Slave = Trainsmit_FIFO[CascadeM_Transmit_OutP].Target & 0x07;
		CascadeM_Control_State |= CASCADE_CONTROL_WAIT_LEN;
		tmp8 = (Trainsmit_FIFO[CascadeM_Transmit_OutP].Target-1) & 0x07;
		SPI_MstRegWrite(SPISSR, &SPI_SELECT_PIN_TO_SS_VALUE[tmp8], 1);
		CascadeM_SPI_SetOpCodeReg(Master_Cmd_RBRS,1);
		SPI_MstDmaRx(CascadeM_Rx_Buf,4,(SDCR_OPC_ENB | SDCR_DMA_GO |SDCR_ERDMA_EXC));
		Task_Active_Table[CascadeM_Receive_TimeOut_TaskActiveID].Task_Interval.w = Trainsmit_FIFO[CascadeM_Transmit_OutP].TimeOut;
#if (TaskControlEnable)
		Task_Active_Table[CascadeM_Receive_TimeOut_TaskActiveID].taskCtrl &= ~TASK_EVENT_SKIP;	
#else		
		Task_Active_Table[CascadeM_Receive_TimeOut_TaskActiveID].Task_Event &= ~TASK_EVENT_SKIP;	
#endif		
		return;
	}		

	CascadeM_Control_State |= CASCADE_CONTROL_TRANSMIT;
	/* Set slave selection value */
	if (Trainsmit_FIFO[CascadeM_Transmit_OutP].Target==CASCADEM_BROADCAST)
		tmp8 = 3;/* Select three device simultaneously for broadcast transmission */
	else
		tmp8 = (Trainsmit_FIFO[CascadeM_Transmit_OutP].Target-1) & 0x07;	
	SPI_MstRegWrite(SPISSR, &SPI_SELECT_PIN_TO_SS_VALUE[tmp8], 1);
	CascadeM_SPI_SetOpCodeReg(Master_Cmd_DWMEM,1);	
	tmp16 = Trainsmit_FIFO[CascadeM_Transmit_OutP].DataLen;
#if CCMST_DEBUG_MODE
	packet = (CASCADE_PacketHeader*)Trainsmit_FIFO[CascadeM_Transmit_OutP].Buf;
	printf("CCMST: TX[%03bu/0x%02bx][%bu/%bu]=%02bx-%u-%02bx-%02bx-%02bx\r\n"
			, tmp8, Trainsmit_FIFO[CascadeM_Transmit_OutP].Target
			, CascadeM_Transmit_OutP, CascadeM_Transmit_InP
			, packet->bCommand
			, packet->iLen
			, packet->bRequest
			, packet->bIndex
			, packet->bValue
			);
	if (packet->iLen)
	{
		printf("CCMST: TX[%bu]=%08lx\r\n", CascadeM_Transmit_OutP, *((U32_T*)packet->buf));
	}
#endif
	
#if (SYSTEM_MSC_DEVICE_SUPPORT)	
	/* For MSC class process only */
	if (Trainsmit_FIFO[CascadeM_Transmit_OutP].Control & CASCADE_CONTROL_CLEAR_BUF)
	{		
		packet = (CASCADE_PacketHeader *)Trainsmit_FIFO[CascadeM_Transmit_OutP].Buf;		
		tmp8  = packet->bValue;
		memcpy(&Cascade_MSC_BulkInBuf[packet->bRequest].bTD_Header, &HCTD_Table.BULK[tmp8].TD, sizeof(USBHC_TD_Header_Typedef));
		memcpy(&HCTD_Table.BULK[tmp8].TD.Byte2_MaxPKT_Size, packet, CASCADE_PACKET_HEADER_LEN);
		SPI_MstDmaTx(&HCTD_Table.BULK[tmp8].TD.Byte2_MaxPKT_Size, tmp16, (SDCR_OPC_ENB | SDCR_DMA_GO |SDCR_ETDMA_EXC));
	}		
	else
#endif		
	{	
		SPI_MstDmaTx(Trainsmit_FIFO[CascadeM_Transmit_OutP].Buf, tmp16, (SDCR_OPC_ENB | SDCR_DMA_GO |SDCR_ETDMA_EXC));
	}	
	Task_Active_Table[CascadeM_Transmit_TimeOut_TaskActiveID].Task_Interval.w = 10000; // set the trans timeout	
}

/*----------------------------------------------------------------------------
 * Function Name: CascadeM_Transmit_Update
 * Purpose: 
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Transmit_Update(void)
{				
	CascadeM_Control_State &= ~(CASCADE_CONTROL_TRANSMIT|CASCADE_CONTROL_RECEIVE); //clear the transmitter flag	
 
	//Clear malloc buffer if needed
	if ((Trainsmit_FIFO[CascadeM_Transmit_OutP].Control & CASCADE_CONTROL_NONE_FREE) == 0x00)
	{
		malloc_free(Trainsmit_FIFO[CascadeM_Transmit_OutP].Buf);
	}
	
	//Go for Next FIFO Transmit
	CascadeM_Transmit_OutP++;
	if (CascadeM_Transmit_OutP >= CASCADE_TRANSMIT_FIFO_MAX)
		CascadeM_Transmit_OutP = 0;
		
}
/*
 *--------------------------------------------------------------------------------
 * Function Name: CascadeM_Receive_Complete_Handle()
 * Purpose : 
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */  
void CascadeM_Receive_Complete_Handle(void)
{
	U16_T packet_len, rx_data_len;
	CASCADE_PacketHeader *packet;
	
	SPI_MstRegRead(SDRBC, (U8_T *)&rx_data_len, 2);
	rx_data_len++;

	packet = (CASCADE_PacketHeader *) (&CascadeM_Rx_Buf[0]);
	
	while (rx_data_len >= CASCADE_PACKET_HEADER_LEN)
	{
		packet_len = packet->iLen + CASCADE_PACKET_HEADER_LEN;
		if (packet_len > rx_data_len)
		{
			printf("CCMST: RX partial data, len=%d/%d\r\n", rx_data_len, packet_len);
 	 		return;
		}
#if CCMST_DEBUG_MODE	
		printf("CCMST: RX[0x%02bx]=%02bx-%u-%02bx-%02bx-%02bx\r\n"
			, Trainsmit_FIFO[CascadeM_Transmit_OutP].Target
			, packet->bCommand
			, packet->iLen
			, packet->bRequest
			, packet->bIndex
			, packet->bValue);
			
		if (packet->iLen)
		{
			printf("CCMST: RX=%08lx\r\n", *((U32_T*)packet->buf));
		}					
#endif	
		if (packet->bCommand <= CASCADE_SYSTEM_CMD)
		{
			CascadeM_Receive_System_Command_Handle(packet);		 
		}
		else if (packet->bCommand <= CASCADE_USB_CMD)
		{
			CascadeM_Receive_USB_Command_Handle(packet);			
		}
		else if	(packet->bCommand <= CASCADE_KVM_CMD)
		{		
			CascadeM_Receive_KVM_Command_Handle(packet);		 
		}
		else
		{
			printf("CCMST: RX unsupported command...\r\n");
			Disp_Str((U8_T*)&packet, packet_len);
		}
		
		if (rx_data_len >= packet_len)
			rx_data_len -= packet_len;

		/* Process next packet */
		packet = (CASCADE_PacketHeader *)(packet->buf + packet->iLen);
	}
} /* End of CascadeM_Receive_Complete_Handle() */

/*--------------------------------------------------------------------------------------
 * Function Name: void CascadeM_Slave_Input_Ready_Init(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *--------------------------------------------------------------------------------------*/
void CascadeM_Slave_Input_Ready_Init(void)
{
	U8_T index;
	U8_T interrupt5;
	U8_T trigger;
	U8_T GPIO_wakeup[2];
	U8_T regvalue0,regvalue1;

	EXTINT4_DISABLE;
	interrupt5 = EINT5; // keep the orginal value
	EXTINT5_DISABLE;
	
	//1.Read in the current GPIO_wakeup_setting.
	_MISC_CIR_SFR(MISC_GPWER);
	_MISC_DR_SFR(GPIO_wakeup[0]);	
	_MISC_DR_SFR(GPIO_wakeup[1]);	
		
	//2.Settle SPI Slave Input Ready Interrupt 
	//Use P0 as the input port	
	GPIO_wakeup[0] = CASCADEM_IN_READY_MASK; //use ping 0~3
	_MISC_DR_SFR(GPIO_wakeup[0]); //allow P0 pin 0~3 to wakeup
	_MISC_DR_SFR(GPIO_wakeup[1]); //use used		
	_MISC_CIR_SFR(MISC_GPWER);
	
	regvalue0 = 0x00;
	_MISC_DR_SFR(regvalue0);
	_MISC_CIR_SFR(SPI_InReadyPort_DER[0]);

	for (index=1;index < 4; index++)
	{
		//Pull up the input interrupt pin first
		GPIO_SetOneBit(SPI_InReadyPort[index], SPI_InReadyPin[index], 1);
		//0.Clear De-bounce time
		/*
		_MISC_CIR_SFR(SPI_InReadyPort_DER[index]);
		_MISC_DR_READ_SFR(regvalue0);
		regvalue0 &= ~BIT_MASK[SPI_InReadyPin[index]];
		_MISC_DR_SFR(regvalue0);
		_MISC_CIR_SFR(SPI_InReadyPort_DER[index]);
		*/
		//1.Set Falling edge trigger
		//1-0.Backup old value
		_MISC_CIR_SFR(SPI_InReadyPort_SFR[index]);
		_MISC_DR_READ_SFR(regvalue0);
		_MISC_DR_READ_SFR(regvalue1);
		
		//2-0.assign new value
		if (SPI_InReadyPin[index] < GPIO_Pin_4) // pin0~pin3
		{
			regvalue0 &= ~(BTN_TRIGGER_MASK << InReadyPinShift[index]);
			trigger = regvalue0 | (BTN_TRIGGER_FALLING << InReadyPinShift[index]);
			regvalue0 = trigger;
		}
		else
		{
			regvalue1 &= ~(BTN_TRIGGER_MASK << InReadyPinShift[index]);
			trigger = regvalue1 | (BTN_TRIGGER_FALLING << InReadyPinShift[index]);
			regvalue1 = trigger;
		}
		_MISC_DR_SFR(regvalue0);
		_MISC_DR_SFR(regvalue1);
		_MISC_CIR_SFR(SPI_InReadyPort_SFR[index]);
	}
	//2.Enable INTER4
	EXTINT4_ENABLE;
	EINT5 = interrupt5;
}


/*----------------------------------------------------------------------------
 * Function Name: TASK_CascadeM_Receive_TimeOut
 * Purpose: Task for check the receive time out
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void TASK_CASCADEM_Receive_TimeOut(void)
{
#if (TaskControlEnable)
	Task_Active_Table[CascadeM_Receive_TimeOut_TaskActiveID].taskCtrl |= TASK_EVENT_SKIP;
#else	
	Task_Active_Table[CascadeM_Receive_TimeOut_TaskActiveID].Task_Event |= TASK_EVENT_SKIP;
#endif	
}

/*----------------------------------------------------------------------------
 * Function Name: CascadeM_KVM_Console_Init
 * Purpose: Cascade Console KVM init
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_KVM_Console_Init(void)
{		
	CascadeM_Console_State |= CASCADEM_KVM_START;
	
	if (KVM_Active_State == 0x00) //so far not host has report connect
	{
		//Check the Power Jump setting
		if (KVM_Flash.cSystemFlag1 & SYSTEM_PLUGIN_JUMP_MASK)
		{
		}
		else
		{
			//CascadeM_CurrentHost = 0; //start from first port			
#if (SYSTEM_CASCADE_API_SUPPORT==0)			
			KVM_CurrentHost = 0x00;			 
			CascadeM_KVM_Port_Jump(0x00);
#endif			
		}		
	}	
	
#if (SYSTEM_CASCADE_API_SUPPORT)
	//Report the Port number to Console 
	//Cascade_API_Transmit_SystemPort();
	CCAPI_KVM_ReportUpstreamPortStatus();
#endif /* #if (SYSTEM_CASCADE_API_SUPPORT) */	
}

/*----------------------------------------------------------------------------
 * U8_T CascadeM_PassThrough_Setup_Buffer_Malloc(U8_T pid,U8_T devinx)
 * Purpose :  
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
U8_T CascadeM_PassThrough_Setup_Buffer_Malloc(U8_T pid,U8_T devinx)
{
	U16_T data_len;
	
#if (CASCADEM_PASSTHROUGH_BUF_MALLOC)
	U16_T malloc_len;
#endif /* #if (CASCADEM_PASSTHROUGH_BUF_MALLOC) */
	
	memcpy(&USB_PDevice[devinx].Hc.Control.Setup ,&USBDC_Device[devinx].Setup[pid],sizeof(USB_Setup_TypeDef));
	memcpy(&USB_PDevice[devinx].Hc.PControl.Setup,&USBDC_Device[devinx].Setup[pid],sizeof(USB_Setup_TypeDef));	
	if (USB_PDevice[devinx].Hc.Control.Setup.b.wLength.w != 0) /* with data stage */
	{
		if (USB_PDevice[devinx].Hc.PControl.Total_Length > CASCADE_PAGE_SIZ)
		{
			data_len = CASCADE_PAGE_SIZ;
			USB_PDevice[devinx].Hc.PControl.Total_Length = CASCADE_PAGE_SIZ;
		}
		else
		{
			if (USB_PDevice[devinx].Hc.PControl.Total_Length < sizeof(USB_Setup_TypeDef))
				data_len = sizeof(USB_Setup_TypeDef);
			else	
				data_len = USB_PDevice[devinx].Hc.PControl.Total_Length;
		}
		
#if (CASCADEM_PASSTHROUGH_BUF_MALLOC)		
		malloc_len = data_len+CASCADE_PACKET_HEADER_LEN;
		CascadeM_PassThrough_Buf_In_Used_Flag = 0;		
		USB_PDevice[devinx].Hc.PControl.Buf = m_malloc(malloc_len,37);		
		//printf("P=%ld\n\r",(U32_T)USB_PDevice[devinx].Hc.PControl.Buf);
		if (USB_PDevice[devinx].Hc.PControl.Buf == NULL)
		{
			printf(" <M>:Passthrough Setup malloc Fail\n\r");
			return 0;
		}				
#else
		//memroy alloc
		Tansmitter_PassThrough_Buf_In_Used_Devinx = devinx;
		CascadeM_PassThrough_Buf_In_Used_Flag = 1;
		USB_PDevice[devinx].Hc.PControl.Buf = CascadeM_Passthrough_Buf;
#endif /* #if (CASCADEM_PASSTHROUGH_BUF_MALLOC) */
		
		//Set Pointer to Data Segment.		
		USBDC_Device[devinx].Control_EndpBuf[pid] = USB_PDevice[devinx].Hc.PControl.Buf+CASCADE_PACKET_HEADER_LEN;
	}
	else
	{
		USB_PDevice[devinx].Hc.PControl.Buf = 0; //no data
	}
	return 1;
}

/*----------------------------------------------------------------------------
 * void CascadeM_PassThrough_Setup_Data_Out_Enable(U8_T devinx)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void CascadeM_PassThrough_Setup_Data_Out_Enable(U8_T slave,U8_T port,U8_T devinx)
{
#if CCMST_ORIGINAL_1
	U16_T	len;	
	CASCADE_PacketHeader  *token;

	len = USB_PDevice[devinx].Hc.PControl.Total_Length + CASCADE_PACKET_HEADER_LEN;
#if (CASCADEM_CMD_DEBUG)
	printf(" (M)->[Setup_Data_Out_Enable Transmit(Len:%d)]\n\r",USB_PDevice[devinx].Hc.PControl.Total_Length);
#endif /*#if (RECEIVER_CMD_DEBUG)*/
	token = (CASCADE_PacketHeader *) (&USB_PDevice[devinx].Hc.PControl.Buf);
	token->bCommand = CASCADE_USB_DATA_OUT_STAGE;	
	token->iLen	   	= USB_PDevice[devinx].Hc.PControl.Total_Length;	
	token->bRequest	= 0;	
	token->bIndex	= devinx;	
	token->bValue	= port & 0x03;	
	CascadeM_Transmit_Processing_Flag = 1;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_DISABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */
	CascadeM_Cmd_Transmit(USB_PDevice[devinx].Hc.PControl.Buf,CASCADE_CONTROL_NONE_FREE,len,100,slave);			
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)		
	USBHC_INT_ENABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */	
	CascadeM_Transmit_Processing_Flag = 0;
#else
	pPACKET = (CASCADE_PacketHeader *)(&USB_PDevice[devinx].Hc.PControl.Buf);
	pPACKET->bCommand = CASCADE_USB_DATA_OUT_STAGE;	
	pPACKET->iLen = USB_PDevice[devinx].Hc.PControl.Total_Length;	
	pPACKET->bRequest = 0;	
	pPACKET->bIndex	= devinx;	
	pPACKET->bValue	= port & 0x03;
	CascadeM_Cmd_Transmit((U8_T*)pPACKET, CASCADE_CONTROL_NONE_FREE, CASCADE_PACKET_HEADER_LEN + pPACKET->iLen, 100, slave);
#endif	
}

/*----------------------------------------------------------------------------
 * Function Name: void CascadeM_PassThrough_Setup_Data_In(U8_T devinx)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void CascadeM_PassThrough_Setup_Data_In(U8_T slave,U8_T port,U8_T devinx)
{
#if CCMST_ORIGINAL_1
	U16_T	len;
	//U8_T	*buf;
	CASCADE_PacketHeader  *token;

	len = USB_PDevice[devinx].Hc.PControl.Total_Length+CASCADE_PACKET_HEADER_LEN;
	 
	//if (len)
	{
#if (RECEIVER_CMD_DEBUG)
		printf(" (R)->[Setup Data IN Transmit(%d)]\n\r",len);
#endif /*#if (RECEIVER_CMD_DEBUG)*/

		token = (CASCADE_PacketHeader *) (&USB_PDevice[devinx].Hc.PControl.Buf);
		token->bCommand = CASCADE_USB_DATA_IN_STAGE;	
		token->iLen	   	= USB_PDevice[devinx].Hc.PControl.Total_Length;	
		token->bRequest	= 0;	
		token->bIndex	= devinx;	
		token->bValue	= port & 0x03;	
		CascadeM_Transmit_Processing_Flag = 1;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
		USBHC_INT_DISABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */		
		CascadeM_Cmd_Transmit(USB_PDevice[devinx].Hc.PControl.Buf,CASCADE_CONTROL_NONE_FREE,len,100,slave);				
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
		USBHC_INT_ENABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */					
		CascadeM_Transmit_Processing_Flag = 0;
	}
#else
	pPACKET = (CASCADE_PacketHeader *)(&USB_PDevice[devinx].Hc.PControl.Buf);
	pPACKET->bCommand = CASCADE_USB_DATA_IN_STAGE;	
	pPACKET->iLen = USB_PDevice[devinx].Hc.PControl.Total_Length;	
	pPACKET->bRequest = 0;	
	pPACKET->bIndex	= devinx;	
	pPACKET->bValue	= port & 0x03;
	CascadeM_Cmd_Transmit((U8_T*)pPACKET, CASCADE_CONTROL_NONE_FREE, CASCADE_PACKET_HEADER_LEN + pPACKET->iLen, 100, slave);
#endif	
}

/*----------------------------------------------------------------------------
 * CascadeM_Passthrough_Setup_Token_Handle(CASCADE_PacketHeader *Data_TokenP)
 * Purpose: handle the passthrough 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Passthrough_Setup_Token_Handle(CASCADE_PacketHeader *token)
{
	U8_T	devinx;
	U8_T    port;
	U8_T    slave;		
	U8_T    *data_buf;	
	
	data_buf  = (U8_T *)(token);
	data_buf += CASCADE_PACKET_HEADER_LEN;	
	slave  = CascadeM_Current_Slave;
	devinx = token->bIndex;
	port   = token->bValue + (CascadeM_Current_Slave << 2);		
	//---------------------------------------------------------
	//1.Check passthrough last condition, is it completed?
	if (USB_PDevice[devinx].Hc.PassThrough_State)		
	{
		if (USB_PDevice[devinx].Hc.PControl.UPID == port)
		{	
			printf("UPID:%d,Last Passthroug not complete:%x\n\r",(U16_T)USB_PDevice[devinx].Hc.PControl.UPID,(U16_T)USB_PDevice[devinx].Hc.PassThrough_State);
			CascadeM_PassThrough_Setup_Condition_Reset(devinx,port);
		}	
	}

	//---------------------------------------------------------
	//2.Copy setup token to output buffer	
	memcpy((U8_T *)&USBDC_Device[devinx].Setup[port],data_buf,sizeof(USB_Setup_TypeDef));
	//printf("Setup=>");
	//Disp_Str((U8_T *)&USBDC_Device[devinx].Setup[port],8);
	USBDC_Device[devinx].PassThroughState[port] = PASSTHROUGH_SETUP_START; //port start passthrough service.			
	//4.Fork the PassThrough process task queue	
	TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_PassThrough_Handle_Start_ID,0x00,devinx,port,0);
}

/*----------------------------------------------------------------------------
 * void CascadeM_PassThrough_Setup_Stall(U8_T slave,U8_T port,U8_T devinx)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void CascadeM_PassThrough_Setup_Stall(U8_T slave,U8_T port,U8_T devinx)
{
#if CCMST_ORIGINAL_1
	U16_T len;	
  	 	
	len = Cascade_Set_Data_Transmit_Header(0,NULL,NULL);		
	//Trigger the Transmit		
	Cascade_Data_TokenP->bCommand  = CASCADE_USB_STATUS_STALL;		
	Cascade_Data_TokenP->iLen	   	= 0;	
	Cascade_Data_TokenP->bRequest	= 0;	
	Cascade_Data_TokenP->bIndex	= devinx;	
	Cascade_Data_TokenP->bValue	= port & 0x03; //mount port
	
	CascadeM_Transmit_Processing_Flag = 1;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_DISABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */		
	CascadeM_Cmd_Transmit(Cascade_Transmit_Send_Buf,
						  (CASCADE_CONTROL_TRANSMIT),
						  len,
						  100,
						  slave);	 //report start
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_ENABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */									  
	CascadeM_Transmit_Processing_Flag = 0;
						  
#if (CASCADEM_CMD_DEBUG)
		printf(" (R)->[Setup_STALL_Transmit(%d)]\n\r",len);
		Disp_Str((U8_T *)Cascade_Data_TokenP,len);
#endif /*#if (RECEIVER_CMD_DEBUG)*/					  
#else
	pPACKET = Cascade_BuildPacket(CASCADE_USB_STATUS_STALL, 0, devinx, port & 0x03, 0, 0);
	if (pPACKET)
	{
		CascadeM_Cmd_Transmit((U8_T*)pPACKET, CASCADE_CONTROL_TRANSMIT, CASCADE_PACKET_HEADER_LEN + pPACKET->iLen, 100, slave);
	}
#endif						  		
	CascadeM_PassThrough_Setup_Condition_Reset(devinx,port);
}

/*----------------------------------------------------------------------------
 * void CascadeM_PassThrough_Setup_End(U8_T slave,U8_T devinx)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_PassThrough_Setup_End(U8_T slave,U8_T port,U8_T devinx)
{
#if CCMST_ORIGINAL_1
	U16_T len;	
  	 	
	len = Cascade_Set_Data_Transmit_Header(0,NULL,NULL);		
	//Trigger the Transmit		
	Cascade_Data_TokenP->bCommand  = CASCADE_USB_SETUP_DONE;		
	//Cascade_Data_TokenP->iLen	   	= 0;	
	//Cascade_Data_TokenP->bRequest	= 0;	
	Cascade_Data_TokenP->bIndex	= devinx;	
	Cascade_Data_TokenP->bValue	= (port & 0x03); //mount port
	
	CascadeM_Transmit_Processing_Flag = 1;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_DISABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */	
	CascadeM_Cmd_Transmit(Cascade_Transmit_Send_Buf,
						  (CASCADE_CONTROL_TRANSMIT),
						  len,
						  100,
						  slave);	 //report start
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_ENABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */									  
	CascadeM_Transmit_Processing_Flag = 0;
							  
#if (CASCADEM_CMD_DEBUG)
		printf(" (R)->[Setup_Done_Transmit(%d)]\n\r",len);
#endif /*#if (RECEIVER_CMD_DEBUG)*/					  
#else
	pPACKET = Cascade_BuildPacket(CASCADE_USB_SETUP_DONE, 0, devinx, port & 0x03, 0, 0);
	if (pPACKET)
	{
		CascadeM_Cmd_Transmit((U8_T*)pPACKET, CASCADE_CONTROL_TRANSMIT, CASCADE_PACKET_HEADER_LEN + pPACKET->iLen, 100, slave);
	}
#endif							  		
	CascadeM_PassThrough_Setup_Condition_Reset(devinx,port);
	//printf("[%d]\n\r",Audio_Data_Cnt);
	//printf("End(%d,%02x)\n\r",(U16_T)devinx,(U16_T)USB_PDevice[devinx].Hc.Control.State );
}

/*------------------------------------------------------------------------------------------
 * Function Name: void CascadeM_PassThrough_Setup_Condition_Reset(U8_T devinx,U8_T port)
 * Purpose: Reset all the passthrough coditioin
 * Params :
 * Returns: NONE
 * Note:
 *-------------------------------------------------------------------------------------------
 */
void CascadeM_PassThrough_Setup_Condition_Reset(U8_T devinx,U8_T port)
{
	/* Free all buffer & flag */
	USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
	USB_PDevice[devinx].Hc.gState = USB_PDevice[devinx].Hc.PassThrough_gState_bk;
	USBDC_Device[devinx].PassThroughState[port] = PASSTHROUGH_IDLE;
	//printf("L_P=%ld\n\r",(U32_T)USB_PDevice[devinx].Hc.PControl.Buf);
	malloc_free(USB_PDevice[devinx].Hc.PControl.Buf);		
	USB_PDevice[devinx].Hc.PControl.Buf = 0;
	USB_PDevice[devinx].Hc.Control.Buf = 0;
	USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_IDLE;
}

/*----------------------------------------------------------------------------
 * void CascadeM_Receive_USB_Command_Handle(void)
 * Purpose: handle the passthrough 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Receive_USB_Command_Handle(CASCADE_PacketHeader *packet)
{	
	//U8_T    devinx;
	U8_T	td_id;
	//U8_T	curinf,value;
	
	switch(packet->bCommand)
	{
		case CASCADE_USB_UNMOUNT: //umount device done			
		case CASCADE_USB_DESCRIPTOR_END: //mount device done
			if (CascadeM_Sys_State.Mounted.Task_ID) //if the mount control task still in processing
			{	
#if (TaskControlEnable)			
				Task_Active_Table[CascadeM_Sys_State.Mounted.Task_ID].taskCtrl &= ~(TASK_EVENT_SKIP|TASK_EVENT_DESTORY); //continue task
#else
				Task_Active_Table[CascadeM_Sys_State.Mounted.Task_ID].Task_Event &= ~(TASK_EVENT_SKIP|TASK_EVENT_DESTORY); //continue task
#endif				
			}				
			break;
		case CASCADE_USB_INTR_IN:
			td_id = (packet->bIndex & 0xf0) >> 4;
			HC_IntTransfer_Table[td_id].PassControl.State &= ~PASSTHROUGH_WAIT; // clear the wait state
			if (HC_IntTransfer_Table[td_id].PassControl.Wp != HC_IntTransfer_Table[td_id].PassControl.Rp) //There is still needed to handle queue
			{	//if still has data in queue
				if ((HC_IntTransfer_Table[td_id].PassControl.State & PASSTHROUGH_SET) == 0x00) //task has been in running
				{					
					TASK_Active(TASK_TYPE_EVENT,TASK_DATAST_USB_Virtual_HID_ID,0,td_id,0,0); // now active the Generic KB Output Handle
				}
			}
			break;					
										
		case CASCADE_USB_SETUP_STAGE: //Extender Setup Token Passthrough
#if (CASCADEM_CMD_DEBUG)
			printf("  M_[SETUP]\n\r");
#endif /* #if (CASCADEM_CMD_DEBUG) */			
			CascadeM_Passthrough_Setup_Token_Handle(packet);
			break;
			
		case CASCADE_USB_DATA_OUT_STAGE: //Extender Setup Data Passthrough
#if (CASCADEM_CMD_DEBUG)
			printf("  M_[DATA_OUT]\n\r");
#endif /* #if (CASCADEM_CMD_DEBUG) */
			CascadeM_Passthrough_Setup_Data_Out_Handle(packet);			
			break;
			
		case CASCADE_USB_DATA_IN_STAGE: //Extender Setup Status Passthrough
#if (CASCADEM_CMD_DEBUG)
			printf("  M_[DATA_IN]\n\r");
#endif /* #if (CASCADEM_CMD_DEBUG) */
			CascadeM_Passthrough_Setup_Data_In_Enable(packet);
			break;
			
		case CASCADE_USB_STATUS_OUT:
#if (CASCADEM_CMD_DEBUG)
			printf("  M_[STATUS_OUT]\n\r");
#endif /* #if (CASCADEM_CMD_DEBUG) */
			CascadeM_Passthrough_Setup_Status_Out_Handle(packet);
			break;			 					
			
		case CASCADE_USB_INTR_OUT:
			CascadeM_Passthrough_Intr_Out_Handle(packet);
			break;
			
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
		case CASCADE_USB_ISO_OUT:			
			CascadeM_Passthrough_Audio_Data_Out_Handle(packet);			
			break;
#endif
#if (SYSTEM_MSC_DEVICE_SUPPORT)
		case CASCADE_USB_MSC_DATA_IN:
			//Burst data in flow control
			Cascade_BurstDataInFlowControl = 0;
			CascadeM_PassthroughMscBurstInHandle();
			break;
		case CASCADE_USB_MSC_CBW:
		case CASCADE_USB_MSC_DATA_OUT:
			CascadeM_PassthroughMscBurstOutHandle(packet);
			break;			
#endif //#if (SYSTEM_DEVICE_MSC_SUPPORT)

		case CASCADE_USB_INTERVAL: //usb hid interval time			 
			CascadeM_Passthrough_HidInterval_Handle(packet);
			break;
		case CASCADE_USB_PROTOCOL_SETTING: //usb hid interval time			 
			CascadeM_Passthrough_Protocol_Setting_Handle(1,CascadeM_Current_Slave,packet->bIndex,packet->bRequest,packet->bValue);			
			break;
		case CASCADE_USB_ALTERNATE_SETTING: //usb hid interval time			 
			CascadeM_Passthrough_Alternate_Setting_Handle(packet);
			break;		
		case CASCADE_USB_CONFIGURATION: //usb hid interval time			 
			CascadeM_Passthrough_Configuration_Handle(packet);
			break;			
		default:
			printf("CCMST: RX Unknown KVM command=0x%02bx\r\n", packet->bCommand);			
			break;
	}		
}

/*----------------------------------------------------------------------------
 * void CascadeM_Receive_System_Command_Handle(void)
 * Purpose: handle the passthrough 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Receive_System_Command_Handle(CASCADE_PacketHeader *dp)
{	
	switch(dp->bCommand)
	{		
		case CASCADE_SC_VERSION: //the slave infromation for KVM
			//printf("Slave:%bu,report version\n\r",CascadeM_Current_Slave);
			memcpy(FirmwareVersion[CascadeM_Current_Slave].Version,dp->buf      							    ,sizeof(FirmwareVersion[0].Version));
			memcpy(FirmwareVersion[CascadeM_Current_Slave].Date   ,&(dp->buf[sizeof(FirmwareVersion[0].Version)]),sizeof(FirmwareVersion[0].Date));			 
			//Disp_Str(FirmwareVersion[CascadeM_Current_Slave].Version,15);
			break;		
#if (SYSTEM_KVM_BOX_CASCADE_SUPPORT)			
		case CASCADE_BOX_REDIRECT:
			CascadeM_Box_Redirect_Handle(dp);			
			break;	
#endif			
		default:
			break;
	}
}

/*----------------------------------------------------------------------------
 * void CascadeM_Receive_KVM_Command_Handle(void)
 * Purpose: handle the passthrough 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Receive_KVM_Command_Handle(CASCADE_PacketHeader *packet)
{
	U8_T    start_hostid;
	U8_T    index;
	U8_T	hostid;
	U8_T	bit_mask;	
	U8_T    kvm_bit;
	
	switch(packet->bCommand)
	{
		case CASCADE_KVM_STATE: //the slave infromation for KVM						
			start_hostid = (CascadeM_Current_Slave << 2);			
			//printf("Slave[%d]_State:",(U16_T)start_hostid);
			//Disp_Str((U8_T *)packet,6);
			//Reset the state........			
			for (index=0; index < 4; index++)
			{ 				
				hostid = start_hostid+index;
				//Handle the Connect State
				bit_mask = packet->bRequest & BIT_MASK[index];
				CascadeM_USB_Maintain_Connect_State(hostid,bit_mask);
				
				//Handle the KVM Connect State				
				kvm_bit  = 0x10 << index;
				bit_mask = packet->bRequest & kvm_bit;
				CascadeM_USB_Maintain_KVM_Connect_State(hostid,bit_mask);
				
				//Handle the Power State 														
				bit_mask = packet->bIndex & BIT_MASK[index];
				CascadeM_USB_Maintain_Power_State(hostid,bit_mask);
				
				//Handle the Suspend State 														
				bit_mask = packet->bValue & BIT_MASK[index];
				CascadeM_USB_Maintain_Suspend_State(hostid,bit_mask);				
			}
			
			//printf("Connect:");
			//Disp_Str(CascadeM_Sys_State.Connect_State,10);
			
#if (SYSTEM_CASCADE_API_SUPPORT)
			CCAPI_KVM_ReportUpstreamPortStatus();
#endif			
			break;
			
#if (KVM_BUTTON_SUPPORT)
		case CASCADE_KVM_BTN: // host press the button or release button		
#if (SYSTEM_CASCADE_API_SUPPORT)
			//hostid =  (CascadeM_Current_Slave << 2) + packet->bIndex;
			CCAPI_KVM_ReportButtonStatus(CascadeM_Current_Slave, packet->bIndex, packet->bValue);

#else
			if (packet->bValue == KVM_BTN_PRESS)
			{
				hostid = (CascadeM_Current_Slave << 2) + packet->bIndex;
				CascadeM_KVM_Btn_Handle(hostid);
			}
#endif			
			break;
#endif
			
		case CASCADE_KVM_KB_LED: // host press the button or release button
#if (CASCADEM_CMD_DEBUG)
				printf("  M_[KB_LED](p:%d,0x%02bx)\n\r",(U16_T)packet->bIndex,packet->bValue);
#endif /* #if (RECEIVER_CMD_DEBUG) */
				CascadeM_KVM_KB_Led_Handle(packet->bIndex,packet->bValue);
				break;
			break;
		default:
			printf("CCMST: RX Unknown KVM command=0x%02bx\r\n", packet->bCommand);			
			break;
	}
}

/*----------------------------------------------------------------------------
 * Function Name: CascadeM_EDID_Control 
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
void CascadeM_EDID_Control(void)
{
#if CCMST_ORIGINAL_1
	U16_T	len;
	
	len = sizeof(CascadeM_EdidTable);
	Cascade_Data_TokenP = (Cascade_Data_Packet_Def *)CascadeM_EdidTable;
	
	//Trigger the Transmit		
	Cascade_Data_TokenP->bCommand = CASCADE_SC_EDID;		
	Cascade_Data_TokenP->iLen	  = len - CASCADE_PACKET_HEADER_LEN;	
	Cascade_Data_TokenP->bRequest = 0;	
	Cascade_Data_TokenP->bIndex	  = 0;	
	Cascade_Data_TokenP->bValue	  = 0; //mount port
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_DISABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */				
	CascadeM_Transmit_Processing_Flag = 1;
	CascadeM_Cmd_Transmit(Cascade_Transmit_Send_Buf,
						  (CASCADE_CONTROL_TRANSMIT),
						  len,
						  100,
						  CASCADEM_BROADCAST);	 //report start	
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_ENABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */									  
	CascadeM_Transmit_Processing_Flag = 0;					  
#if (CASCADEM_CMD_DEBUG)
		printf(" <M>->EDID_Transmit:%d\n\r",len);
#endif /*#if (RECEIVER_CMD_DEBUG) */
#else
	pPACKET = (CASCADE_PacketHeader*)CascadeEdidPacket;
	pPACKET->bCommand = CASCADE_SC_EDID;		
	pPACKET->iLen = EDID_MAX;	
	pPACKET->bRequest = 0;	
	pPACKET->bIndex	= 0;	
	pPACKET->bValue	= 0;
	CascadeM_Cmd_Transmit((U8_T*)pPACKET, CASCADE_CONTROL_TRANSMIT, CASCADE_PACKET_HEADER_LEN + pPACKET->iLen, 100, CASCADEM_BROADCAST);
#endif
}
/*----------------------------------------------------------------------------
 * void TASK_CascadeM_Device_Mount_Control(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
void TASK_CascadeM_Device_Mount_Control(void)
{	
#if CCMST_ORIGINAL_1
	U8_T  host_valid;
#endif	
	U8_T  devinx;
	U8_T  mounttype;
	U16_T mountport;	
	U8_T  remove_mask;

	if ((CascadeM_Check_System_Ready() == 0x00) || (CaascaseM_System_Report_Ready_Flag == 0))
	{	
       // printf("Mount skip\n\r");		
		TASK_Wait_Current();		
		return;
	}	
	
TASK_CascadeM_DeviceMount_Control_Start:	
	//Get the parameter		
	devinx 	  = TASK_Register0;	
	mounttype = TASK_Register1 & VDEV_MOUNT_MASK;	
	remove_mask = TASK_Register1 & VDEV_REMOVE;
	mountport = Task_Active_Table[TASK_Active_ID].Task_Reload.w;
	
#if CCMST_ORIGINAL_1				
	if (CascadeM_Sys_State.Mounted.Task_ID)	
	{				
		if (TASK_Active_ID != CascadeM_Sys_State.Mounted.Task_ID) //if not same task, then wait
		{						
			TASK_Wait_Current();				
			return;
		}	

		/* Check Next Slave ID */			
		CascadeM_Sys_State.Mounted.Task_Slave = CascadeM_Check_Valid_Slave(mountport);
		if (CascadeM_Sys_State.Mounted.Task_Slave == 0)
		{							
			CascadeM_Sys_State.Mounted.Task_ID = 0; // reset the book in id
			CascadeM_Check_Next_Mount_Event();						
			return; //skip all the mount operation
		}	
		else
		{				
			CascadeM_Sys_State.Mounted.Task_Slave--;
			host_valid = CascadeM_Check_Mount_Valid_State(CascadeM_Sys_State.Mounted.Task_Slave,devinx,mounttype,&Task_Active_Table[TASK_Active_ID].Task_Reload.w);				
			if (host_valid ==0)
			{				
				goto TASK_CascadeM_DeviceMount_Control_Start;
			}			
		}	
	}
	else
	{			
		CascadeM_Sys_State.Mounted.Task_ID =  TASK_Active_ID;	//register the task id		
		//Check the mount or umount state		
		//CascadeM_Sys_State.Mounted.Task_DespInx = 0; //start from first descritptor
		CascadeM_Sys_State.Mounted.Task_Devinx  = devinx; //devinx

		CascadeM_Sys_State.Mounted.Task_Slave	= CascadeM_Check_Valid_Slave(mountport);
		if (CascadeM_Sys_State.Mounted.Task_Slave == 0)
		{					
			CascadeM_Sys_State.Mounted.Task_ID = 0; // reset the book in id
			CascadeM_Check_Next_Mount_Event();			
			return; //skip all the mount operation
		}	
		else
		{				 			
			CascadeM_Sys_State.Mounted.Task_Slave--;									 
			host_valid = CascadeM_Check_Mount_Valid_State(CascadeM_Sys_State.Mounted.Task_Slave,devinx,mounttype,&Task_Active_Table[TASK_Active_ID].Task_Reload.w);						 
			if (host_valid ==0)
			{			
				goto TASK_CascadeM_DeviceMount_Control_Start;
			}			
		}			
		//where is the first slave
	}		
#else
	if (CascadeM_Sys_State.Mounted.Task_ID==0)
	{
		CascadeM_Sys_State.Mounted.Task_ID =  TASK_Active_ID;
		CascadeM_Sys_State.Mounted.Task_Devinx  = devinx;		
	}
	else if (TASK_Active_ID != CascadeM_Sys_State.Mounted.Task_ID)
	{						
		TASK_Wait_Current();		
		return;
	}		
	
	CascadeM_Sys_State.Mounted.Task_Slave = CascadeM_GetSlaveIdFromPortMap(mountport);
	if (CascadeM_Sys_State.Mounted.Task_Slave == 0xff)
	{
		CascadeM_Sys_State.Mounted.Task_ID = 0;
		CascadeM_Check_Next_Mount_Event();		
		return;
	}	
	else if (CascadeM_Check_Mount_Valid_State(CascadeM_Sys_State.Mounted.Task_Slave,devinx,mounttype,&Task_Active_Table[TASK_Active_ID].Task_Reload.w)==0)
	{					
		goto TASK_CascadeM_DeviceMount_Control_Start;
	}
#endif

	if ((mounttype & VDEV_MOUNT_MASK) == VDEV_MOUNT)
	{
		if (CascadeM_Sys_State.Mounted.State[devinx][CascadeM_Sys_State.Mounted.Task_Slave])
		{
			//printf("Slave:%bu,Devinx:%bu,Skip Mount\n\r",CascadeM_Sys_State.Mounted.Task_Slave,devinx);
			Task_Active_Table[TASK_Active_ID].Task_Reload.w &= ~SLAVE_MASK[CascadeM_Sys_State.Mounted.Task_Slave];
			CascadeM_Sys_State.Mounted.Task_ID = 0; // reset the book in id
			CascadeM_Check_Next_Mount_Event();
			return; //skip all the mount operation			
		}			
	}		
	//printf("Slave:%bu,devinx mountsate=%02bx\n\r",CascadeM_Sys_State.Mounted.Task_Slave,CascadeM_Sys_State.Mounted.State[devinx][CascadeM_Sys_State.Mounted.Task_Slave]);	
	//printf("Devinx:%bu, Mount=%02bx,mountport=%04x\n\r",devinx,mounttype|remove_mask,mountport);	
	
	CascadeM_Maintain_Mount_State(CascadeM_Sys_State.Mounted.Task_Slave,devinx,mounttype,mountport);
	CascadeM_USB_Device_Mount_Transmit(CascadeM_Sys_State.Mounted.Task_Slave,devinx,mounttype|remove_mask,mountport);	
	
	if ((mounttype & VDEV_MOUNT_MASK) == VDEV_MOUNT)
	{			 		
		CascadeM_USB_Device_Descriptor_Transmit(CascadeM_Sys_State.Mounted.Task_Slave,devinx,mountport,0);			
	}	
	
	//Wait for slave responsed					  			
	Task_Active_Table[TASK_Active_ID].Task_Reload.w &= ~SLAVE_MASK[CascadeM_Sys_State.Mounted.Task_Slave];
#if (TaskControlEnable)
	Task_Active_Table[TASK_Active_ID].taskCtrl |= TASK_EVENT_SKIP;
#else
	Task_Active_Table[TASK_Active_ID].Task_Event |= TASK_EVENT_SKIP; //task go to sleep, wait for wakeup
#endif		
	//Maintain the mount/umount condition table
	//CascadeM_Maintain_Mount_State(CascadeM_Sys_State.Mounted.Task_Slave,devinx,mounttype,mountport);
	//printf("maintain:%02x\n\r",(U16_T)CascadeM_Sys_State.Mounted.State[devinx][CascadeM_Sys_State.Mounted.Task_Slave]);
	TASK_Wait_Current();	
	//CascadeM_Sys_State.Mounted.State[devinx][CascadeM_Sys_State.Mounted.Task_Slave];							
}

/*----------------------------------------------------------------------------
 * void CascadeM_Device_Mount_Control(U8_T devinx,U8_T mount_type,U16_T mount_port) 
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
void CascadeM_Device_Mount_Control(U8_T devinx,U8_T mount_type,U16_T mount_port)	
{	
	if (CascadeM_Sys_State.Mounted.Task_ID == 0)
	{			
	 
		TASK_Active(TASK_TYPE_EVENT,TASK_CascadeM_DeviceMount_Control_ID,0,devinx,mount_type,mount_port);			
	}
	else
	{		 
		CasecadeM_Mount[CascadeM_Mount_Startp].Devinx	= devinx;
		CasecadeM_Mount[CascadeM_Mount_Startp].MountType = mount_type;
		CasecadeM_Mount[CascadeM_Mount_Startp].MountPort = mount_port;	
		CascadeM_Mount_Startp++;
		if (CascadeM_Mount_Startp >= CASCADEM_MOUNT_QUEUE_MAX)	
		{
			CascadeM_Mount_Startp=0;
		}	
	}		
}
#if CCMST_ORIGINAL_1
/*----------------------------------------------------------------------------
 * U8_T  CascadeM_Check_Valid_Slave(U16_T port_type)
 * Purpose: 
 * Params :
 * Returns:
 * Note:
 *---------------------------------------------------------------------------*/
U8_T CascadeM_Check_Valid_Slave(U16_T mountport)
{
	U8_T 	index;
	U16_T 	bit_mask;
	
	for (index=0; index <= CascadeM_Salve_Cnt; index++)
	{
		bit_mask = 0x000f << (index << 2);
		if (mountport & bit_mask)
			return (index+1); //the value will must large then 1
	}
	return 0;
}
#else
/*----------------------------------------------------------------------------
 * U8_T  CascadeM_GetSlaveIdFromPortMap(U16_T port_map)
 * Purpose: 
 * Params :
 * Returns:
 * Note:
 *---------------------------------------------------------------------------*/
U8_T CascadeM_GetSlaveIdFromPortMap(U16_T port_map)
{
	U8_T i, slave_id;

	for (i=0; i < CASCADE_KVM_MAX_PORT; i++)
	{
		if (BIT_MASK16[i] & port_map)
		{
			slave_id = i>>2;
			if (slave_id <= CascadeM_Salve_Cnt)
				return slave_id;/* Return valid slave id */
		}
	}
	return 0xff;
}
#endif
/*----------------------------------------------------------------------------
 * U8_T  CascadeM_Check_System_Ready(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
U8_T CascadeM_Check_System_Ready(void)
{
	return (CascadeM_Console_State & CASCADEM_KVM_START);		
}

/*----------------------------------------------------------------------------
 * U16_T CascadeM_Get_Mount_State(U8_T slave,U8_T devinx)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
U16_T CascadeM_Get_Mount_State(U8_T slave,U8_T devinx)
{
	U16_T value;
	
	value = (CascadeM_Sys_State.Mounted.State[devinx][slave] << (slave<<2));
	
	return value;
}

/*----------------------------------------------------------------------------
 * U16_T CascadeM_Get_Mount_State(U8_T slave,U8_T devinx)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
U8_T CascadeM_Check_Mount_Valid_State(U8_T slave,U8_T devinx,U8_T mounttype,U16_T *mountport)
{
	U8_T  valid=0;
	U8_T  value;
	U8_T  start_bit;
	U16_T new_bit;
	U8_T  index;
	U16_T port_value;
	U8_T  host;
	
	port_value = *mountport;
	host = (slave << 2);
	value = port_value >> host;	 
	start_bit = 0x01;
	
	for (index=0; index < 4; index++)
	{
		if (start_bit & CascadeM_Sys_State.Mounted.State[devinx][slave]) //if old value is mount
		{
			if ((mounttype & VDEV_MOUNT_MASK) == VDEV_MOUNT)
			{	
				if (value & start_bit) //command to mount, so skip it.
				{
					new_bit = 0x01 << (host+index);
					port_value &= ~new_bit; // clear the opreation
				}
				else
					valid = 1;
			}	
			else
			{
				valid = 1;
			}		
		}
		else
		{ //if old value is umount
			if ((mounttype & VDEV_MOUNT_MASK)== VDEV_UNMOUNT)
			{	
				if ((value & start_bit) == 0x00) //command to mount, so skip it.
				{
					new_bit = 0x01 << (host+index);
					port_value &= ~new_bit; // clear the opreation
				}
				else
				 	valid = 1;
			}	
			else
			{
				valid = 1;
			}	
				
		}	
		start_bit <<= 1;	
	}

	//maintain the mountport value	
	*mountport = port_value;	
	return valid;
}

/*----------------------------------------------------------------------------
 * U8_T CascadeM_Check_System_Ready(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
void CascadeM_Maintain_Mount_State(U8_T slave,U8_T devinx,U8_T mounttype,U16_T mountport)
{
	U8_T 	bit_mask;
	U8_T    start_bit;
	U8_T	start_index;
	U8_T	index;
		
	start_index = (slave << 2);	
	start_bit = (mountport >> start_index) & 0x000f;	
	for (index=0; index < 4; index++)
	{
		bit_mask = 0x01 << index;
		if (bit_mask & start_bit)			
		{
			if ((mounttype & VDEV_MOUNT_MASK)== VDEV_UNMOUNT)
			{	
				CascadeM_Sys_State.Mounted.State[devinx][slave] &= ~bit_mask;
			}
			else
			{
				CascadeM_Sys_State.Mounted.State[devinx][slave] |= bit_mask;
			}		
		}		
	} 
}

/*----------------------------------------------------------------------------
 * void CascadeM_USB_Device_Mount_Transmit(U8_T slave,U8_T devinx,U8_T mounttype,U16_T mountport)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
void CascadeM_USB_Device_Mount_Transmit(U8_T slave,U8_T devinx,U8_T mounttype,U16_T mountport)
{
#if CCMST_ORIGINAL_1
	bit   desp_flag = 0;			
	U16_T len;
	U8_T  mounthost;
	
	len = Cascade_Set_Data_Transmit_Header(0,NULL,NULL);		
	
	//Trigger the Transmit			
	mounthost = (mountport >> (slave << 2)) & 0x000f;		
	
	if ((mounttype & VDEV_MOUNT_MASK) == VDEV_MOUNT)
	{	
		Cascade_Data_TokenP->bCommand  = CASCADE_USB_MOUNT;	
	}	
	else
	{		
		Cascade_Data_TokenP->bCommand  = CASCADE_USB_UNMOUNT;			
	}	
	//Cascade_Data_TokenP->iLen	   	= 0;	
	Cascade_Data_TokenP->bRequest	= mounttype & VDEV_REMOVE;
	Cascade_Data_TokenP->bIndex		= devinx;	
	Cascade_Data_TokenP->bValue		= mounthost; //mount port
		
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_DISABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */	
	CascadeM_Transmit_Processing_Flag = 1;
	CascadeM_Cmd_Transmit(Cascade_Transmit_Send_Buf,
						  (CASCADE_CONTROL_TRANSMIT),
						  len,
						  100,
						  slave);	 //report start
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_ENABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */									  
	CascadeM_Transmit_Processing_Flag = 0;
	
#if (CASCADEM_CMD_DEBUG)
	if ((mounttype & VDEV_MOUNT_MASK) == VDEV_MOUNT)
		printf("(M) Mount");
	else
		printf("(M) Unmount");
	printf("[(Slave:%d,Devinx:%d)-Port:%02x]\n\r",(U16_T)slave,(U16_T)devinx,(U16_T)mounthost);
#endif /* #if (RECEIVER_CMD_DEBUG) */
#else
	U8_T  mounthost;
	
	mounthost = (mountport >> (slave << 2)) & 0x000f;
	pPACKET = Cascade_BuildPacket((mounttype & VDEV_MOUNT_MASK) ? CASCADE_USB_MOUNT:CASCADE_USB_UNMOUNT, mounttype & VDEV_REMOVE, devinx, mounthost, 0, 0);
	if (pPACKET)
	{
		CascadeM_Cmd_Transmit((U8_T*)pPACKET, CASCADE_CONTROL_TRANSMIT, CASCADE_PACKET_HEADER_LEN + pPACKET->iLen, 100, slave);
	}
#endif
	//Send out to console 
#if (SYSTEM_CASCADE_API_SUPPORT)
	CascadeM_Transmit_API_Mount_State(devinx,mounttype);
#endif	
}

/*----------------------------------------------------------------------------
 * void CascadeM_USB_Device_Descriptor_Transmit(U8_T slave,U8_T devinx,U8_T mounttype,U16_T mountport)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
void CascadeM_USB_Device_Descriptor_Transmit(U8_T slave,U8_T devinx,U16_T mountport,U8_T taget)
{
#if CCMST_ORIGINAL_1
	U8_T  desp_flag = 0;			
	U8_T  *dev_desp;
	U8_T  index;	
	U16_T devdsp_len,len;
	U8_T  mounthost;
	
	mounthost = (mountport >> (slave << 2)) & 0x000f;	
	for (index=0; index < HUB_DESC ; index++)
	{
		if (USB_PDevice[devinx].Desc[index].Len)
		{			
#if (CASCADEM_CMD_DEBUG)
			printf("(M) Desp[(%d)-%d Bytes]\n\r",(U16_T)index,USB_PDevice[devinx].Desc[index].Len);			
#endif /* #if (RECEIVER_CMD_DEBUG) */			
			desp_flag 	= 1;
			devdsp_len  = USB_PDevice[devinx].Desc[index].Len;
			dev_desp    = USB_PDevice[devinx].Desc[index].Ptr;									
			len = Cascade_Set_Data_Transmit_Header(devdsp_len,dev_desp,NULL);			
			
			//Trigger the Transmit
			Cascade_Data_TokenP->bCommand  = CASCADE_USB_DESCRIPTOR;	
			Cascade_Data_TokenP->iLen	   = devdsp_len;	
			Cascade_Data_TokenP->bRequest  = mounthost;	
			Cascade_Data_TokenP->bIndex	= devinx;	
			Cascade_Data_TokenP->bValue	= index;	//Descriptor ID
			
#if (CASCADEM_CMD_DEBUG)			
			//Disp_Str(USB_PDevice[devinx].Desc[index].Ptr,USB_PDevice[devinx].Desc[index].Len+CASCADE_PACKET_HEADER_LEN);
#endif			
			CascadeM_Transmit_Processing_Flag = 1;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
				USBHC_INT_DISABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */						
			if (taget==0)
			{	

				CascadeM_Cmd_Transmit(Cascade_Transmit_Send_Buf,
									  (CASCADE_CONTROL_TRANSMIT|CASCADE_CONTROL_NONE_FREE),
									  len,
									  100,
									  slave);	 //report start
			}
#if (SYSTEM_CASCADE_API_SUPPORT)
			else
			{	
				//if ((USB_PDevice[devinx].bVirtual_Attr & VIRTUAL_ATTR_MASK) == 0x00)	
				{	
					//if ((USB_PDevice[devinx].bVirtual_Attr & VIRTUAL_ATTR_CONSOLE_REPORT) == 0x00)
					{
						Cascade_API_Cmd_Transmit(Cascade_Transmit_Send_Buf,
												 CASCADE_CONTROL_NONE_FREE,
												 len);
					}							 
				}
			}						 
#endif
								  
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
			USBHC_INT_ENABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */											  
			CascadeM_Transmit_Processing_Flag = 0;					  
			//Wait for slave responsed					  			
		}
	}
	
	//Handle the descriptor terminate and wait for responsed from slave
	if (desp_flag)
	{
		if (taget==0)
		{
			CascadeM_Transmit_DataPack(CascadeM_Sys_State.Mounted.Task_Slave,
									   CASCADE_USB_DESCRIPTOR_END,
									   NO_REQUEST,
									   devinx,
									   NO_VALUE,
									   NO_CTRL_ATTR);	
									   
			CascadeM_Transmit_USB_HID_Setting(CascadeM_Sys_State.Mounted.Task_Slave,devinx);							  		  													   
		}
		
#if (SYSTEM_CASCADE_API_SUPPORT)
		else
		{	
			//if ((USB_PDevice[devinx].bVirtual_Attr & VIRTUAL_ATTR_MASK) == 0x00)	
			{
				//if ((USB_PDevice[devinx].bVirtual_Attr & VIRTUAL_ATTR_CONSOLE_REPORT) == 0x00)
				{		
					Cascade_API_Transmit_DataPack(CASCADE_USB_DESCRIPTOR_END,
												  NO_REQUEST,
												  devinx,
												  NO_VALUE,
												  NO_CTRL_ATTR);										  
					USB_PDevice[devinx].bVirtual_Attr |= VIRTUAL_ATTR_CONSOLE_REPORT; // has been repor to console already
					//printf("Done,devinx:%bx[%bx]\n\r",devinx,USB_PDevice[devinx].bVirtual_Attr);
				}	
				
				//Cascade_API_Cmd_Transmit(Cascade_Transmit_Send_Buf,0,len);
			}
		}								
#endif 									   		 
	}
#else/**/
	U8_T i, desc_sent=0, mounthost;
	
	mounthost = (mountport >> (slave << 2)) & 0x000f;
	
#if (CASCADEM_CMD_DEBUG)
	printf("(M) Slave:%bu,Devinx:%bu,Descirptor\n\r",slave,devinx);			
#endif /* #if (RECEIVER_CMD_DEBUG) */			
	
	USB_PDevice[devinx].SlaveMount |= BIT_MASK[slave];
	
	/* Send device descriptor */
	for (i=0; i<HUB_DESC; i++)
	{
		if (USB_PDevice[devinx].Desc[i].Len==0)
			continue;

		pPACKET = (CASCADE_PacketHeader*)USB_PDevice[devinx].Desc[i].Ptr;
		pPACKET->bCommand = CASCADE_USB_DESCRIPTOR;	
		pPACKET->iLen = USB_PDevice[devinx].Desc[i].Len;	
		pPACKET->bRequest = (taget==0) ? mounthost:0x00;	
		pPACKET->bIndex	= devinx;	
		pPACKET->bValue	= i;
		
		desc_sent = 1;
		if (taget==0)
		{		
			CascadeM_Cmd_Transmit((U8_T*)pPACKET, (CASCADE_CONTROL_TRANSMIT|CASCADE_CONTROL_NONE_FREE), CASCADE_PACKET_HEADER_LEN + pPACKET->iLen, 100, slave);
		}
#if (SYSTEM_CASCADE_API_SUPPORT)			
		else
		{
			Cascade_API_Cmd_Transmit((U8_T*)pPACKET, CASCADE_CONTROL_NONE_FREE, CASCADE_PACKET_HEADER_LEN + pPACKET->iLen);			
		}
#endif			
	}
	
	/* End of device descriptor sendinng */
	if (desc_sent)
	{
		if (taget==0)
		{
			pPACKET = Cascade_BuildPacket(CASCADE_USB_DESCRIPTOR_END, 0, devinx, 0, 0, 0);
			if (pPACKET)
			{
				CascadeM_Cmd_Transmit((U8_T*)pPACKET, CASCADE_CONTROL_TRANSMIT, CASCADE_PACKET_HEADER_LEN + pPACKET->iLen, 100, CascadeM_Sys_State.Mounted.Task_Slave);
			}						  									   
			
			CascadeM_Transmit_USB_HID_Setting(CascadeM_Sys_State.Mounted.Task_Slave, devinx);							  		  													   
		}
#if (SYSTEM_CASCADE_API_SUPPORT)
		else
		{	
			pPACKET = Cascade_BuildPacket(API_USB_DESCRIPTOR_END, 0, devinx, 0, 0, 0);
			if (pPACKET)
			{			
				Cascade_API_Cmd_Transmit((U8_T*)pPACKET, 0, CASCADE_PACKET_HEADER_LEN + pPACKET->iLen);			
				USB_PDevice[devinx].bVirtual_Attr |= VIRTUAL_ATTR_CONSOLE_REPORT; // has been repor to console already
			}
		}								
#endif 									   		 
	}
#endif		
}	

/*----------------------------------------------------------------------------
 * void CascadeM_USB_Device_Check_IntervalU8_T slave,U8_T port,U8_T devinx)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
void CascadeM_USB_Device_Check_Interval(U8_T slave,U8_T port,U8_T devinx)
{	
#if CCMST_ORIGINAL_1
	U16_T len;		
	
	len = Cascade_Set_Data_Transmit_Header(0,NULL,NULL);		
	//Trigger the Transmit		
	Cascade_Data_TokenP->bCommand  = CASCADE_KVM_INTERVAL;		
	//Cascade_Data_TokenP->iLen	   	= 0;	
	//Cascade_Data_TokenP->bRequest	= 0;	
	Cascade_Data_TokenP->bIndex	= devinx;	
	Cascade_Data_TokenP->bValue	= port; //mount port
	CascadeM_Transmit_Processing_Flag = 1;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_DISABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */	
	CascadeM_Cmd_Transmit(Cascade_Transmit_Send_Buf,
						  CASCADE_CONTROL_TRANSMIT,
						  len,
						  100,
						  slave);	 //report start
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_ENABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */									  
	CascadeM_Transmit_Processing_Flag = 0;
#if (CASCADEM_CMD_DEBUG)
	printf("(M) Check Inerval[(Slave:%d,Devinx:%d)-Port:%02x]\n\r",(U16_T)slave,(U16_T)devinx,(U16_T)port);
#endif /* #if (RECEIVER_CMD_DEBUG) */
#else
	pPACKET = Cascade_BuildPacket(CASCADE_KVM_INTERVAL, 0, devinx, port, 0, 0);
	if (pPACKET)
	{
		CascadeM_Cmd_Transmit((U8_T*)pPACKET, 0, CASCADE_PACKET_HEADER_LEN + pPACKET->iLen, 100, slave);
	}
#endif
}

/*----------------------------------------------------------------------------
 * Function Name: CascadeM_Console_Monitor_PlugIn 
 * Purpose:  
 * Params : 
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */ 
#ifdef NO_CONSOLE
void CascadeM_Console_Monitor_PlugIn(void)
{
	CascadeM_EDID_Control();
	CascadeM_Console_State |= CASCADEM_EDID_VALID;	
}
#endif
/*----------------------------------------------------------------------------
 * Function Name: CascadeM_Check_Next_Mount_Event 
 * Purpose:  
 * Params : 
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void CascadeM_Check_Next_Mount_Event(void)
{
	
	if (CascadeM_Mount_Startp != CascadeM_Mount_Endp)
	{ // still have device to need mount/umount		
		TASK_Active(TASK_TYPE_EVENT,TASK_CascadeM_DeviceMount_Control_ID,0,
					CasecadeM_Mount[CascadeM_Mount_Endp].Devinx,
					CasecadeM_Mount[CascadeM_Mount_Endp].MountType,
					CasecadeM_Mount[CascadeM_Mount_Endp].MountPort);
					
		CascadeM_Mount_Endp++;
		if (CascadeM_Mount_Endp >= CASCADEM_MOUNT_QUEUE_MAX)	
		{
			CascadeM_Mount_Endp=0;
		}
	}
	/*	
	else
	{
		//devinx 	  = TASK_Register0;	
		//mounttype = TASK_Register1;		
		//mountport = Task_Active_Table[TASK_Active_ID].Task_Reload.w;
		if ((USB_PDevice[TASK_Register0].bVirtual_Attr & VIRTUAL_ATTR_MASK) == 0x00)	
		{		
			//printf("mountype=%bx,devinx:%bx[%bx]\n\r",mounttype,mount_dev,USB_PDevice[mount_dev].bVirtual_Attr);		
			cascade_port  = CascadeM_Sys_State.Mounted.State[TASK_Register0][0];			 
			cascade_port |= CascadeM_Sys_State.Mounted.State[TASK_Register0][1] << 4;
			cascade_port |= ((CascadeM_Sys_State.Mounted.State[TASK_Register0][2] << 8) | (CascadeM_Sys_State.Mounted.State[TASK_Register0][3] << 12));						
			Cascade_API_Transmit_USB_AttachChanged(TASK_Register0,0,TASK_Register1,cascade_port);				
		}					
		//Send to Console for mount information.
		//printf("Mount No Queue\n\r");
	}	
	*/
}
 
/*----------------------------------------------------------------------------
 * void CascadeM_Transmit_DataPack(U8_T slave,U8_T command,U8_T request,U8_T index,U8_T value)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
void CascadeM_Transmit_DataPack(U8_T slave,U8_T control,U8_T command,U8_T request,U8_T index,U8_T value,U16_T data_len,U8_T *datap)
{	
	pPACKET = Cascade_BuildPacket(command,request,index,value,datap,data_len);
	if (pPACKET)
	{
		CascadeM_Cmd_Transmit((U8_T*)pPACKET,control,CASCADE_PACKET_HEADER_LEN+pPACKET->iLen,100,slave);
	}
}
 

/*----------------------------------------------------------------------------------------------
 * void CascadeM_Transmit_Intr_Endp_In_Data(U8_T devinx,U8_T td_id,U8_T data_len,U8_T API_Only)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------------------------*/
void CascadeM_Transmit_Intr_Endp_In_Data(U8_T devinx,U8_T td_id,U8_T data_len,U8_T API_Only)
{		
#if CCMST_ORIGINAL_1	
	CASCADE_PacketHeader *packet;
	U8_T	slave_id;
#if (SYSTEM_CASCADE_API_SUPPORT)
	bit		data_send_flag=0;
#endif
	
	slave_id = KVM_CurrentHost >> 2;
	//Avoid sending data to slave before the device is ready
	if (API_Only==0)
	{
		if ((USB_PDevice[devinx].SlaveMount & BIT_MASK[slave]) == 0x00)
		{	
			//printf("s");
			return;
		}	
	}		
	//printf("g");
	packet = (CASCADE_PacketHeader *)HC_IntTransfer_Table[td_id].CascadeDataBuf;	
	
	packet->bCommand  	= CASCADE_USB_INTR_IN;
	packet->iLen	   	= data_len;		
	packet->bRequest	= HC_IntTransfer_Table[td_id].InterfaceIdx;	
	packet->bIndex		= devinx;		
	packet->bValue		= (HC_IntTransfer_Table[td_id].EndpIdx & USBDC_DEV_ENDPINX_MASK) >> 4;
	
	//slave_id = KVM_CurrentHost >> 2;
	CascadeM_Transmit_Processing_Flag = 1;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_DISABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */		
	if (API_Only==0)
	{	
		CascadeM_Cmd_Transmit((U8_T *)packet, (CASCADE_CONTROL_TRANSMIT | CASCADE_CONTROL_NONE_FREE), CASCADE_PACKET_HEADER_LEN + packet->iLen, 100, slave_id);
	}
	
#if (SYSTEM_CASCADE_API_SUPPORT)
	//Check the Keyboard & Mouse Disable flag first
	if (API_Only)
	{	
		if (USB_PDevice[devinx].InfProtocol[packet->bRequest] == INF_KEYBOARD)
		{
			if (Cascade_API_HID_Keyboard_Dislabe_Flag == 0)
			{				
			   	data_send_flag = 1;
			}   
		}
		else
		{
			if (Cascade_API_HID_Mouse_Dislabe_Flag == 0)
			{	
			   data_send_flag = 1;
			}
		}		
		
		if (data_send_flag)
		{
			if ((USB_PDevice[devinx].HID_Control & BIT_MASK[packet->bRequest]) == 0)
			{	
				Cascade_API_Cmd_Transmit((U8_T *)packet, CASCADE_CONTROL_NONE_FREE, CASCADE_PACKET_HEADER_LEN + packet->iLen);
			}									  	  
		}
	}					  
#endif	/* #if (SYSTEM_CASCADE_API_SUPPORT) */
						  
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_ENABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */									  
	CascadeM_Transmit_Processing_Flag = 0;		
#else /* #if CCMST_ORIGINAL_1 */
	U8_T	slave_id;
	
	slave_id = KVM_CurrentHost >> 2;
	//Avoid sending data to slave before the device is ready
	if (API_Only==0)
	{
		if ((USB_PDevice[devinx].SlaveMount & BIT_MASK[slave_id]) == 0x00)
		{				
			return;
		}	
	}	
	
	//if (slave_id)
	//	printf("g");
		
	pPACKET = (CASCADE_PacketHeader *)HC_IntTransfer_Table[td_id].CascadeDataBuf;	
	pPACKET->bCommand = CASCADE_USB_INTR_IN;
	pPACKET->iLen = data_len;		
	pPACKET->bRequest = HC_IntTransfer_Table[td_id].InterfaceIdx;	
	pPACKET->bIndex	= devinx;		
	pPACKET->bValue	= (HC_IntTransfer_Table[td_id].EndpIdx & USBDC_DEV_ENDPINX_MASK) >> 4;
	
	if (API_Only==0)
	{	
		CascadeM_Cmd_Transmit((U8_T *)pPACKET, (CASCADE_CONTROL_TRANSMIT|CASCADE_CONTROL_NONE_FREE), CASCADE_PACKET_HEADER_LEN + pPACKET->iLen, 100, KVM_CurrentHost >> 2);
	}
	
#if (SYSTEM_CASCADE_API_SUPPORT)
	//Check the Keyboard & Mouse Disable flag first
	if (API_Only)
	{	
		if (((USB_PDevice[devinx].InfProtocol[pPACKET->bRequest]==INF_KEYBOARD) && (Cascade_API_HID_Keyboard_Dislabe_Flag==0)) ||
			((USB_PDevice[devinx].InfProtocol[pPACKET->bRequest]==INF_MOUSE) && (Cascade_API_HID_Mouse_Dislabe_Flag==0)))
		{
			if ((USB_PDevice[devinx].HID_Control & BIT_MASK[pPACKET->bRequest]) == 0)
			{	
				Cascade_API_Cmd_Transmit((U8_T *)pPACKET, CASCADE_CONTROL_NONE_FREE, CASCADE_PACKET_HEADER_LEN + pPACKET->iLen);
			}									  	  
		}
	}					  
#endif	/* #if (SYSTEM_CASCADE_API_SUPPORT) */
#endif				  
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
//void CascadeM_CopyRxDmaToApp(U16_T start,U16_T appGetLen,U8_T *pAppData)
//{
//	if (appGetLen == 0)
//		return;
		
//	DMA_GrantXdata(pAppData,&CascadeM_Rx_Buf[start],appGetLen);		
//}

/*----------------------------------------------------------------------------
 * void CascadeM_Passthrough_Setup_Data_Out_Handle(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void CascadeM_Passthrough_Setup_Data_Out_Handle(CASCADE_PacketHeader *packet)
{
	U8_T	devinx;	
	U8_T    port;
	U8_T    slave;		
//	U8_T    *data_buf;	
	U16_T	len;
	
//	data_buf= ((U8_T *)(packet))+sizeof(USB_Setup_TypeDef);
	slave  	= CascadeM_Current_Slave;
	devinx 	= packet->bIndex;
	port   	= packet->bValue + (CascadeM_Current_Slave << 2);	
	len    	= packet->iLen;
	 	
//	CascadeM_CopyRxDmaToApp(CascadeM_Rx_Data_Inx,len,USB_PDevice[devinx].Hc.Control.Buf); 		
	DMA_GrantXdata(USB_PDevice[devinx].Hc.Control.Buf, packet->buf, len);
	USB_PDevice[devinx].Hc.PControl.Total_Length = len;
	USB_PDevice[devinx].Hc.PControl.Current_Length = 0; //reset current length
	USB_PDevice[devinx].Hc.PControl.Data_Length = len;
	if (len > USB_HC_ATL_BUF_SIZE)
		USB_PDevice[devinx].Hc.PControl.Data_Length = USB_HC_ATL_BUF_SIZE;

	//CascadeM_Passthrough_State[devinx] |= CASCADEM_PASSTHROUGH_OUT_PAGE_MASK; //out data page in
	USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_OUT;
	
	//Fork Control transfer 
	TASK_Active(TASK_TYPE_USB,TASK_USBHC_Handle_Control_ID,USB_PDevice[devinx].Hc.Control.TdNum,devinx,0,0);
}

/*----------------------------------------------------------------------------
 * void CascadeM_Setup_Data_Out_Enable(U8_T port,U8_T devinx)
 * Purpose: Ask slave to send next out data page
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Setup_Data_Out_Enable(U8_T port,U8_T devinx)
{	
#if CCMST_ORIGINAL_1
	U8_T    slave;
	U8_T    upid;
	CASCADE_PacketHeader *Data_TokenP;

	//Set basic information for slave kvm
	slave = port >> 2;
	upid  = port & 0x03;
	
	//Set basic information for slave kvm
	//len = CASCADE_PACKET_HEADER_LEN;
	//HC_IntTransfer_Table[td_id].PassControl.State |= PASSTHROUGH_WAIT; //data in processing of sending
	Data_TokenP = (CASCADE_PacketHeader *)USB_PDevice[devinx].Hc.PControl.Buf;	
	
	Data_TokenP->bCommand  	= CASCADE_USB_DATA_OUT_STAGE;
	Data_TokenP->iLen	   	= 0;		
	Data_TokenP->bRequest	= 0;	
	Data_TokenP->bIndex		= devinx;		
	Data_TokenP->bValue		= upid;
	CascadeM_Transmit_Processing_Flag = 1;	
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_DISABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */			
	CascadeM_Cmd_Transmit((U8_T *)Data_TokenP,
						  (CASCADE_CONTROL_TRANSMIT|CASCADE_CONTROL_NONE_FREE),
						  CASCADE_PACKET_HEADER_LEN,
						  100,
						  slave);	 //report start	
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_ENABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */									  
	CascadeM_Transmit_Processing_Flag = 0;
#if (CASCADEM_CMD_DEBUG)
		printf(" (M)->[Next Setup_Data_Out_Enable Transmit]\n\r");
#endif /*#if (CASCADEM_CMD_DEBUG)*/			
#else
	pPACKET = (CASCADE_PacketHeader *)USB_PDevice[devinx].Hc.PControl.Buf;
	pPACKET->bCommand = CASCADE_USB_DATA_OUT_STAGE;
	pPACKET->iLen = 0;
	pPACKET->bRequest = 0;	
	pPACKET->bIndex = devinx;		
	pPACKET->bValue	= port & 0x03;	
	CascadeM_Cmd_Transmit((U8_T *)pPACKET, (CASCADE_CONTROL_TRANSMIT|CASCADE_CONTROL_NONE_FREE), pPACKET->iLen + CASCADE_PACKET_HEADER_LEN, 100, (port>>2));
#endif
}


/*----------------------------------------------------------------------------
 * void CascadeM_Passthrough_Setup_Data_In_Enable(CASCADE_PacketHeader *token)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Passthrough_Setup_Data_In_Enable(CASCADE_PacketHeader *token)
{
	U8_T 	devinx;
	U8_T    upid;
	U8_T    slave;		
	//U8_T    *data_buf;	
	//U16_T	len;

	slave  	= CascadeM_Current_Slave;
	devinx 	= token->bIndex;
	upid   	= token->bValue + (CascadeM_Current_Slave << 2);	
	
	//printf("###  devinx=%d,state=(%02x)\n\r",(U16_T)devinx,(U16_T)CascadeM_Passthrough_State[devinx]);
	if (CascadeM_Passthrough_State[devinx] & CASCADEM_PASSTHROUGH_IN_MASK)
	{ //HC already in IN State
		if (CascadeM_Passthrough_State[devinx] & CASCADEM_PASSTHROUGH_IN_PAGE_MASK)
		{
			//printf("IN_Data are wait ....\n\r");
			CascadeM_Setup_Data_In(slave,token->bValue,devinx);	
			//CascadeM_Passthrough_State[devinx] |= CASCADEM_PASSTHROUGH_IN_SEND_MASK;
		}	
		else
		{
			//printf("0mark send rquest(%02x) ....\n\r",(U16_T)CascadeM_Passthrough_State[devinx]);
			CascadeM_Passthrough_State[devinx] |= CASCADEM_PASSTHROUGH_IN_SEND_MASK; //Host require sending
		}	
	}
	else
	{
		if (CascadeM_Passthrough_State[devinx] == CASCADEM_PASSTHROUGH_IDLE)
		{
			//No data, just send 0 length data page out
			//devinx = Extender_Setup.Data_Token.bIndex;
			CascadeM_Passthrough_State[devinx] |= CASCADEM_PASSTHROUGH_IN_SEND_MASK;			
			//printf("1mark send rquest(%02x) ....\n\r",(U16_T)CascadeM_Passthrough_State[devinx]);
			//printf("GO DATA in\n\r");
			//USB_PDevice[devinx].Hc.PControl.Total_Length = 0;			
			//CascadeM_Setup_Data_In(slave,token->bValue,devinx);
		}
		else
		{
			printf("S:%d,P:%d,D:%d,Passthrough state error=%02x\n\r",(U16_T)slave,(U16_T)upid,(U16_T)devinx,(U16_T)CascadeM_Passthrough_State[devinx]);
		}
	}
}


/*----------------------------------------------------------------------------
 * CascadeM_Setup_Data_In(U8_T slave,U8_T port,U8_T devinx)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------- */
void CascadeM_Setup_Data_In(U8_T slave,U8_T port,U8_T devinx)
{
#if CCMST_ORIGINAL_1
	U16_T	len;	
	CASCADE_PacketHeader *Data_TokenP;
		
	Data_TokenP = (CASCADE_PacketHeader *)USB_PDevice[devinx].Hc.PControl.Buf;	
	
	len = USB_PDevice[devinx].Hc.PControl.Total_Length+CASCADE_PACKET_HEADER_LEN;
	Data_TokenP->bCommand  	= CASCADE_USB_DATA_IN_STAGE;
	Data_TokenP->iLen	   	= USB_PDevice[devinx].Hc.PControl.Total_Length;		
	Data_TokenP->bRequest	= 0;	
	Data_TokenP->bIndex		= devinx;		
	Data_TokenP->bValue		= port & 0x03;
	CascadeM_Transmit_Processing_Flag = 1;		
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_DISABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */		
	CascadeM_Cmd_Transmit(USB_PDevice[devinx].Hc.PControl.Buf,
						  (CASCADE_CONTROL_TRANSMIT|CASCADE_CONTROL_NONE_FREE),
						  len,
						  1000,
						  slave);	 //report start	
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_ENABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */									  
	CascadeM_Transmit_Processing_Flag = 0;		
#if (CASCADEM_CMD_DEBUG)
		printf(" (M)->[Setup Data IN Transmit(%d)]\n\r",len);
#endif /*#if (RECEIVER_CMD_DEBUG)*/	
#else
	pPACKET = (CASCADE_PacketHeader *)USB_PDevice[devinx].Hc.PControl.Buf;
	pPACKET->bCommand = CASCADE_USB_DATA_IN_STAGE;
	pPACKET->iLen = USB_PDevice[devinx].Hc.PControl.Total_Length;
	pPACKET->bRequest = 0;	
	pPACKET->bIndex = devinx;		
	pPACKET->bValue	= port & 0x03;	
	CascadeM_Cmd_Transmit((U8_T*)pPACKET, (CASCADE_CONTROL_TRANSMIT|CASCADE_CONTROL_NONE_FREE), pPACKET->iLen + CASCADE_PACKET_HEADER_LEN, 1000, slave);	 //report start		
#endif
}

/*------------------------------------------------------------------------------------
 * void CascadeM_Passthrough_Setup_Status_Out_Handle(CASCADE_PacketHeader *token)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *-----------------------------------------------------------------------------------*/
void CascadeM_Passthrough_Setup_Status_Out_Handle(CASCADE_PacketHeader *token)
{
	U8_T	devinx;

	if (USB_PDevice[devinx].Hc.Control.State != CTRL_IDLE)
	{
		if (USB_PDevice[devinx].Hc.Control.State != CTRL_STATUS_OUT_WAIT)
		{
			devinx =token->bIndex;
			USB_PDevice[devinx].Hc.Control.State = CTRL_STATUS_OUT;
			TASK_Active(TASK_TYPE_USB,TASK_USBHC_Handle_Control_ID,USB_PDevice[devinx].Hc.Control.TdNum,devinx,0,0);
		}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: CascadeM_Passthrough_Intr_Out_Handle(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void CascadeM_Passthrough_Intr_Out_Handle(CASCADE_PacketHeader *token)
{
	U8_T	devinx;
	U8_T	endpinx;
	U8_T	len;
	U8_T	tdOffset;
	U8_T*	pBuf;
	USBHC_TD_Header_Typedef*	pTdHeader;
	
	devinx 		= token->bIndex;
	endpinx 	= token->bValue;
	len 		= (U8_T)token->iLen;
	tdOffset 	= USBDC_Device[devinx].EndpType[endpinx] & 0x0f;
	pBuf 		= HCTD_Table.INTL[tdOffset].Buf;
	pTdHeader 	= &(HCTD_Table.INTL[tdOffset].TD);
	
//	CascadeM_CopyRxDmaToApp(CascadeM_Rx_Buf_Inx,len,pBuf);
	DMA_GrantXdata(pBuf, token->buf, len);			
		
	pTdHeader->Byte4_Total_Byte = len;
	pTdHeader->Byte5 &= 0xFC;
	pTdHeader->Byte1 |= TD_ACTIVED_MASK;

	USBHC_CORE_Set_TD_SkipMap(USBHC_TD_INTL,tdOffset,0x00);
}
 
 
/*----------------------------------------------------------------------------
 *void CascadeM_Handle_After_Transmit_Complete(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Handle_After_Transmit_Complete(void)
{
	U8_T *data_buf;
	U8_T devinx;
	U8_T upid;
	U8_T td_id;
	CASCADE_PacketHeader *data_token;
	
	data_buf   = Trainsmit_FIFO[CascadeM_Transmit_OutP].Buf+CASCADE_PACKET_HEADER_LEN;
	data_token = (CASCADE_PacketHeader *)Trainsmit_FIFO[CascadeM_Transmit_OutP].Buf;
	devinx = data_token->bIndex & 0x0f; // target device index number
	switch(data_token->bRequest)
	{		 
		case CASCADE_USB_DATA_IN_STAGE: //passthrough data in,since the buffer is clear, so can do next data in
			CascadeM_Passthrough_State[devinx] &= ~CASCADEM_PASSTHROUGH_IN_SEND_MASK;//clear the page in data send flag
			if (USB_PDevice[devinx].Hc.PassThrough_State == PASSTHROUGH_HC_SETUP_DATA_IN)
			{
				if (USB_PDevice[devinx].Hc.Control.State == CTRL_DATA_IN)
				{
					td_id = USB_PDevice[devinx].Hc.Control.TdNum; // which host channel is used by device
					TASK_Active(TASK_TYPE_USB,TASK_USBHC_Handle_Control_ID,td_id,devinx,0,0);
				}
			}
			break;				
		case CASCADE_USB_SETUP_DONE: //passtrhoug data section is closed
			upid = data_token->bValue + (Trainsmit_FIFO[CascadeM_Transmit_OutP].Target << 2);
			CascadeM_PassThrough_Setup_Condition_Reset(devinx,upid);	
			//USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_IDLE; //wait for next passthrough
			//Receiver_Passthrough_State = 0;
			//malloc_free(USB_PDevice[devinx].Hc.Control.Buf); // not free this time, should wait for send out complete
			//USB_PDevice[devinx].Hc.Control.Buf = 0;
			break;		
		default:
			break;
	}
	
#if (SYSTEM_MSC_DEVICE_SUPPORT)			
	if (Trainsmit_FIFO[CascadeM_Transmit_OutP].Control & CASCADE_CONTROL_CLEAR_BUF)
	{		
		upid   = data_token->bRequest;
		td_id  = data_token->bValue;
		memcpy(&HCTD_Table.BULK[td_id].TD, &Cascade_MSC_BulkInBuf[upid].bTD_Header, sizeof(USBHC_TD_Header_Typedef));
		CascadeM_PassthroughMscBurstInComplete();
	}						
#endif			
}					

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
/*----------------------------------------------------------------------------
 * void CascadeM_Passthrough_Audio_Data_Out_Handle(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Passthrough_Audio_Data_Out_Handle(CASCADE_PacketHeader *token)
{
	U8_T tmp8;
	
	/* Enqueue data */
	DMA_GrantXdata((U8_T *)&CADR_USB_Audio_Buf[CCMST_AudioOutWrPtr], (U8_T*)token, token->iLen + CASCADE_PACKET_HEADER_LEN);
		
	CCMST_AudioOutWrPtr++;
	if (CCMST_AudioOutWrPtr >= SYSTEM_CASCADE_AUDIO_IN_START)
    	CCMST_AudioOutWrPtr = SYSTEM_CASCADE_AUDIO_OUT_START;
				
	/* Reactive audio TD if need */
	if (CCMST_AudioOutBusy==0)
	{
		CCMST_AudioOutBusy = 1;	
					
		tmp8 = CADR_USB_Audio_Buf[CCMST_AudioOutRdPtr].Token.iLen;
		DMA_GrantXdata(HCTD_Table.ISTL[0].Buf, CADR_USB_Audio_Buf[CCMST_AudioOutRdPtr].Buf, tmp8);
		HCTD_Table.ISTL[0].TD.Byte0_Actual_Byte = 0;
		HCTD_Table.ISTL[0].TD.Byte2_MaxPKT_Size = tmp8-2;
		HCTD_Table.ISTL[0].TD.Byte3 &= 0xFC;
		HCTD_Table.ISTL[0].TD.Byte4_Total_Byte = tmp8-2;
		HCTD_Table.ISTL[0].TD.Byte5 &= 0xFC;
		HCTD_Table.ISTL[0].TD.Byte1 |= TD_ACTIVED_MASK;

		/* Clear skip map */
		TD_ISTL_Skip_Map &= ~0x01;
		_USBHC_HCDR_SFR(TD_ISTL_Skip_Map);
		_USBHC_HCCIR_SFR(HC_ISTL_TDSKIP_MAP_REG);
		
		CCMST_AudioOutRdPtr++;
		if (CCMST_AudioOutRdPtr >= SYSTEM_CASCADE_AUDIO_IN_START)
			CCMST_AudioOutRdPtr = SYSTEM_CASCADE_AUDIO_OUT_START;		
	}
}

/*----------------------------------------------------------------------------
 * void CascadeM_PassThrough_Setup_Stall(U8_T slave,U8_T port,U8_T devinx)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Audio_Data_Out_Enable(CASCADE_PacketHeader *token)
{
#if CCMST_ORIGINAL_1
	U16_T len;	
  	 	
	len = Cascade_Set_Data_Transmit_Header(0,NULL,NULL);		
	//Trigger the Transmit		
	Cascade_Data_TokenP->bCommand  = CASCADE_USB_ISO_OUT;		
	Cascade_Data_TokenP->iLen	   	= 0;	
	Cascade_Data_TokenP->bRequest	= 0;	
	Cascade_Data_TokenP->bIndex	= token->bIndex;	
	Cascade_Data_TokenP->bValue	= token->bValue; //mount port
	
	CascadeM_Transmit_Processing_Flag = 1;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_DISABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */	
	CascadeM_Cmd_Transmit(Cascade_Transmit_Send_Buf,
						  (CASCADE_CONTROL_TRANSMIT),
						  len,
						  100,
						  CascadeM_Current_Slave);	 //report start
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_ENABLE;
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */									  
	CascadeM_Transmit_Processing_Flag = 0;
						  
#if (CASCADEM_CMD_DEBUG)
		printf(" (M)->[Audio Out Enable(%d)]\n\r",CascadeM_Current_Slave);
		Disp_Str((U8_T *)Cascade_Data_TokenP,len);
#endif /*#if (RECEIVER_CMD_DEBUG)*/					  
#else
	pPACKET = Cascade_BuildPacket(CASCADE_USB_ISO_OUT, 0, token->bIndex, token->bValue, 0, 0);
	if (pPACKET)
	{
		CascadeM_Cmd_Transmit((U8_T*)pPACKET, CASCADE_CONTROL_TRANSMIT, CASCADE_PACKET_HEADER_LEN + pPACKET->iLen, 100, CascadeM_Current_Slave);
	}
#endif						  		
	CascadeM_PassThrough_Setup_Condition_Reset(token->bIndex, KVM_CurrentUSBAudio);
}

/*----------------------------------------------------------------------------
 * void CascadeM_Audio_Data_Out_Buf_Check(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Audio_Data_Out_Buf_Check(void)
{
	U16_T len;	
  	 	
	if (CCMST_AudioOutRdPtr != CCMST_AudioOutWrPtr) //check the waiting buffer
	{
		len = CADR_USB_Audio_Buf[CCMST_AudioOutRdPtr].Token.iLen;	
		
		DMA_GrantXdata(HCTD_Table.ISTL[0].Buf,
					   CADR_USB_Audio_Buf[CCMST_AudioOutRdPtr].Buf,
					   len);						   			 					
			
		CCMST_AudioOutBusy = 1;		
		len -= 2;			
		HCTD_Table.ISTL[0].TD.Byte0_Actual_Byte = 0;
		HCTD_Table.ISTL[0].TD.Byte2_MaxPKT_Size = len;
		HCTD_Table.ISTL[0].TD.Byte3 		   &= 0xFC;
		HCTD_Table.ISTL[0].TD.Byte4_Total_Byte 	= len;
		HCTD_Table.ISTL[0].TD.Byte5            &= 0xFC;
		HCTD_Table.ISTL[0].TD.Byte1 		   |= TD_ACTIVED_MASK;
		TD_ISTL_Skip_Map &= ~0x01;
		
		USBHC_INT_DISABLE;
		//Clear Done Map
		_USBHC_HCDR_SFR(0x01);
		_USBHC_HCCIR_SFR(HC_ISTL_TDDONE_MAP_REG);

		//Clear Skip Map
		_USBHC_HCDR_SFR(TD_ISTL_Skip_Map);
		_USBHC_HCCIR_SFR(HC_ISTL_TDSKIP_MAP_REG);
		USBHC_INT_ENABLE;		
		
		CCMST_AudioOutRdPtr++;
		if (CCMST_AudioOutRdPtr >= SYSTEM_CASCADE_AUDIO_IN_START)
			CCMST_AudioOutRdPtr = SYSTEM_CASCADE_AUDIO_OUT_START;		
	}
}
#endif /* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */

/*
 *--------------------------------------------------------------------------------
 * void CascadeM_SPI_SetOpCodeReg(U8_T *opCode, U8_T opLen)
 * Purpose : Setting Op-code to SPI Transfer Op-code Field(STOF) register.
 * Params  : opCode - SPI Op-code.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void CascadeM_SPI_SetOpCodeReg(U8_T *opCode, U8_T opLen)
{
	U8_T	stol = opLen - 1;
	//U8_T	opReverse[8] = {0};
	//U8_T	i;

	//for(i = 0; i < opLen; i++)
	//	opReverse[8 - i - 1] = opCode[i];
	
	SPI_MstRegWrite(STOL, &stol, 1);
	SPI_MstRegWrite(STOF, opCode, 8);
}

/*----------------------------------------------------------------------------
 * void CascadeM_KVM_KB_Led_Handle(U8_T kb_led)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void CascadeM_KVM_KB_Led_Handle(U8_T upid,U8_T kb_led)
{	
	U8_T    port;
	
	
    port = upid + (CascadeM_Current_Slave << 2);        
    if (port == KVM_CurrentHost)
    {	    	
		if (KVM_HostLed[KVM_CurrentHost] != kb_led)
		{			
#if (SYSTEM_CASCADE_API_SUPPORT)
			if (Cascade_API_HID_Disable_Flag==0)
#endif																	
			{							
				KVM_CONSOLE_Keyboard_Led_Control(kb_led);	
			}				
		}
	}		
	
	KVM_HostLed[port] = kb_led;		
	
	
#if (SYSTEM_CASCADE_API_SUPPORT)	
	Cascade_API_Transmit_KVM_KB_Led(port,kb_led);
#endif			
}

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
#if (SYSTEM_CASCADE_AUDIO_BUF_CNT)
void CascadeM_Audio_Device_State_Reset(void)
{	
	U8_T  reg8p;
	U16_T reg16b;
	 
	CADR_USB_HC_Audio_Inx     	 = SYSTEM_CASCADE_AUDIO_IN_START;

	/* Reset Audio out buffer */
	CCMST_AudioOutBusy = 0;	
	CCMST_AudioOutRdPtr = SYSTEM_CASCADE_AUDIO_OUT_START;	
	CCMST_AudioOutWrPtr = SYSTEM_CASCADE_AUDIO_OUT_START;	
	
	for (reg8p = 0; reg8p < SYSTEM_CASCADE_AUDIO_BUF_CNT; reg8p++)
	{
		reg16b = (U16_T)(CADR_USB_Audio_Buf[reg8p].Buf);
		CADR_USB_AudioBufAddr[reg8p][0] = (U8_T)reg16b;
		CADR_USB_AudioBufAddr[reg8p][1] = (U8_T)(reg16b >> 8);
	}	
}
#endif /* #if (SYSTEM_CASCADE_AUDIO_BUF_CNT) */
#endif /* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */

/*----------------------------------------------------------------------------
 * U8_T CascadeM_DeviceMount_Control(U8_T upid,U8_T devinx,U8_T bcmd)
 * Purpose: make the port connect status, in virtual hub port
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Slave_DeviceMount_Control(U8_T upid,U8_T devinx,U8_T bcmd)
{
#if CCMST_ORIGINAL_1
	U8_T    slave;

	if (USB_PDevice[devinx].DevClass == USB_HUB_CLASS)
	{
		return;
	}
	else
	{
		//USBHC_Get_Upper_Device_Info(devinx,&hubdevice,&hubportnumber);
#if (CASCADEM_CMD_DEBUG)
		if (bcmd == CASCADE_USB_UNMOUNT)
		{
			printf("  <M>->[UnMount");
		}
		else
		{
			printf("  <M>->[Mount");
		}
		printf(" Devinx:%d]\n\r",(U16_T)devinx);
#endif /* #if (RECEIVER_CMD_DEBUG) */
	}

	slave = upid >> 2;
	CascadeM_Transmit_DataPack(slave,bcmd,0,devinx,0,0);
#else
	if (USB_PDevice[devinx].DevClass == USB_HUB_CLASS)
		return;
		
	pPACKET = Cascade_BuildPacket(bcmd, 0, devinx, 0, 0, 0);
	if (pPACKET)
	{
		CascadeM_Cmd_Transmit((U8_T*)pPACKET, CASCADE_CONTROL_TRANSMIT, CASCADE_PACKET_HEADER_LEN + pPACKET->iLen, 100, upid >> 2);
	}	
#endif		 
}

#if (PROJECT_USB_GENERIC_HID_ENABLE)
/*----------------------------------------------------------------------------
 * void CascadeM_GHid_Device_Buf_Init(void)
 * Purpose:  
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_GHid_Device_Buf_Init(void)
{
	memset(Cascade_GHID_DataQ,0x00,sizeof(Cascade_GHID_DataQ));
	
	Cascade_GHID_DataQ[0].Token.bCommand = CASCADE_USB_INTR_IN;	
	Cascade_GHID_DataQ[0].Token.bRequest = 0; //interface	
	Cascade_GHID_DataQ[0].Token.bIndex	 = USBDC_VHID_DEVINX;		
	Cascade_GHID_DataQ[0].Token.bValue	 = USBDC_VHID_KB_ENDP_NUM;
	
	Cascade_GHID_DataQ[1].Token.bCommand = CASCADE_USB_INTR_IN;	
	Cascade_GHID_DataQ[1].Token.bRequest = 1;	
	Cascade_GHID_DataQ[1].Token.bIndex	 = USBDC_VHID_DEVINX;		
	Cascade_GHID_DataQ[1].Token.bValue	 = USBDC_VHID_MS_ENDP_NUM;
}

/*----------------------------------------------------------------------------
 * void CascadeM_GHID_Intr_Data_Transmit(U8_T endpid,U8_T length)
 * Purpose:  
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_GHID_Intr_Data_Transmit(U8_T endpid,U8_T length,U8_T *buf)
{
	U8_T index;
	U8_T slave;
	
	if (endpid == USBDC_VHID_KB_ENDP_NUM)
	{	
		index = 0;
	}	
	else
	{	
		index = 1;
	}
	
	memcpy(Cascade_GHID_DataQ[index].Buf,buf,length);
	slave = KVM_CurrentHost >> 2;
	
	Cascade_GHID_DataQ[index].Token.iLen = length;
		
	CascadeM_Cmd_Transmit((U8_T *)&Cascade_GHID_DataQ[index],
						  (CASCADE_CONTROL_TRANSMIT),
						  length + CASCADE_PACKET_HEADER_LEN,
						  100,
						  slave);	 //report start
}
#endif /* #if (PROJECT_USB_GENERIC_HID_ENABLE) */

/*----------------------------------------------------------------------------
 * void CascadeM_Setup_Command_Handle(U8_T devinx,USB_Setup_TypeDef *setup)
 * Purpose:  
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Setup_Command_Handle(U8_T devinx,USB_Setup_TypeDef *setup)
{
	U8_T Request_No;
	U8_T Type_Recipient;	
	//Do Little endian convert to Big endian
	U8_T wValue_L;	
	U8_T wIndex_L;
	
	
	Request_No = setup->b.bRequest;
	Type_Recipient = setup->b.bmRequestType & (REQUEST_TYPE | RECIPIENT);
	wValue_L = setup->b.wValue.bw.lsb;	
	wIndex_L = setup->b.wIndex.bw.lsb;	

	if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))  /* Interface Receivent */
	{
		switch (Request_No)
		{
		case SET_INTERFACE:
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)				
			if (devinx == USBHC_Audio_Devinx)
			{
				if (wIndex_L == USBHC_Audio_Out_Interface)
				{
					if (wValue_L == 0)
					{
						/* Clear Audio tempory buffer and stop Audio OUT stream */
						CCMST_AudioOutWrPtr = SYSTEM_CASCADE_AUDIO_OUT_START;	
						CCMST_AudioOutRdPtr = SYSTEM_CASCADE_AUDIO_OUT_START;						
						CCMST_AudioOutBusy = 0;
					}
					printf("SetOutIntf! buffering %s, op=%02bx\n\r", (wValue_L==0) ? "Stop":"Start", CCMST_AudioOutRdPtr);					
				}
					
				if (wIndex_L == USBHC_Audio_In_Interface)
				{
					if (wValue_L == 0) //turn of the out
					{
						USBHC_AUDIO_In_Start(0x00);
					}
					else
					{		
						USBHC_AUDIO_In_Start(TD_ACTIVED_MASK);
					}	
					printf("SetInIntf! buffering %s\n\r", (wValue_L==0) ? "Stop":"Start");									
				}	
			}				
#else
		devinx = 1; // just disable the warrning message
#endif					
		break;
		}
	}				
}

U8_T CascadeM_UpPort_State(U8_T hostid)
{
	U8_T byte_mask,bit_mask;
	U8_T result;		
	
	byte_mask = hostid >> 3;
	bit_mask  = 0x01 << (hostid & 0x07);
		
	result = (CascadeM_Sys_State.Connect_State[byte_mask] & bit_mask) ? 1:0;
				
	return	result;
}

/*----------------------------------------------------------------------------
 * void TASK_CascadeM_USBHC_PassThrough(void)
 * Purpose: Cascade port USB device passthrough control
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void TASK_CascadeM_USBHC_PassThrough(void)
{
	U8_T	devinx, td_id;
	RESULT	Result = USB_BUSY;
	U16_T	remain_len;
	U8_T    slave,port;

	//Get basic patameter for processing
	devinx = TASK_Register0;
	port   = USB_PDevice[devinx].Hc.PControl.UPID; // which host channel is used by device
	//port   = TASK_Register1;
	slave  = port >> 2;	
	td_id  = USB_PDevice[devinx].Hc.Control.TdNum; // which host channel is used by device

	//Skip task if the device is gone or remote is gone
	if ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00) // Device is gone
	{
		if (USB_PDevice[devinx].Hc.PassThrough_State)
		{	
			//USBHC_PassThrough_Setup_Condition_Reset(devinx);
			/* Free all buffer & flag */
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
			USB_PDevice[devinx].Hc.gState = USB_PDevice[devinx].Hc.PassThrough_gState_bk;
			USBDC_Device[devinx].PassThroughState[port] = PASSTHROUGH_IDLE;
			USB_PDevice[devinx].Hc.Control.Buf = 0;
			USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_IDLE;
		}	
		return;
	}

#if (CASCADEM_USB_PASSTHROUGH_DEBUG_MODE)
	printf(">> TASK_CascadeM_PassThrough(P:%d)-(D:%d)-(S%d):\n\r",(U16_T)port,(U16_T)devinx,(U16_T)USB_PDevice[devinx].Hc.PassThrough_State);
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)

TASK_CascadeM_USBHC_PassThrough_Restart:
	switch (USB_PDevice[devinx].Hc.PassThrough_State)
	{
		case PASSTHROUGH_SETUP_START: //Send A control setup to HC
#if (CASCADEM_USB_PASSTHROUGH_DEBUG_MODE)
			printf("   (1).CascadeM_SETUP Stage\n\r");
#endif //#if (RECEIVER_USB_PASSTHROUGH_DEBUG_MODE)			
			/* Send out Setup Token */
			Result = USBHC_PassThrough_Send_Setup_Token(devinx,td_id);
			/* For No Data Stage */
			if (Result == USB_SUCCESS) // need to ACK DC
			{
				/* Check the data length */
				USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_DC_SETUP_STATUS_IN;
				goto TASK_CascadeM_USBHC_PassThrough_Restart;
			}
			else if (Result == USB_UNSUPPORT)
			{
				//printf("stall1");
				USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_DC_SETUP_STALL;
				goto TASK_CascadeM_USBHC_PassThrough_Restart;
			}
			break;
		case PASSTHROUGH_SETUP_START_DONE: //After passthrough Setup token send complete
#if (CASCADEM_USB_PASSTHROUGH_DEBUG_MODE)
			printf("SETUP Token Done\n\r");
#endif //#if (CASCADEM_USB_PASSTHROUGH_DEBUG_MODE)
			//wait for first page data in
			USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_HC_SETUP_DATA_OUT;
			CascadeM_Setup_Data_Out_Enable(port,devinx);
			//CascadeM_PassThrough_Setup_Data_Out_Enable(slave,port,devinx);
			break;
		case PASSTHROUGH_DC_SETUP_DATA_IN: //Setup Data Stage
			//update length first
			//If not allow to send data, then need to wait
			//Check the transmiter is ready to receive page data?
			/*		 
			if ((CascadeM_Passthrough_State[devinx] & CASCADEM_PASSTHROUGH_IN_SEND_MASK) == 0x00)
			{
				printf("(pw)");
				TASK_Wait_Current(); //wait for next
				return;
			}	
			*/
			
			USB_PDevice[devinx].Hc.PControl.Current_Length += USB_PDevice[devinx].Hc.PControl.Data_Length;

#if (CASCADEM_USB_PASSTHROUGH_DEBUG_MODE)
			printf("(R)SETUP_DC_DATA_IN(d-%d/c-%d/t-%d)\n\r",(U16_T)USB_PDevice[devinx].Hc.PControl.Data_Length,
															 (U16_T)USB_PDevice[devinx].Hc.PControl.Current_Length, 
															 (U16_T)USB_PDevice[devinx].Hc.PControl.Total_Length);
#endif //#if (CASCADEM_USB_PASSTHROUGH_DEBUG_MODE)
			if (USB_PDevice[devinx].Hc.PControl.Data_Length == 0)
			{
				if (USB_PDevice[devinx].Hc.PControl.Current_Length)
				{
					//printf("devinx=%d,Slave stop data in\n\r",(U16_T)devinx);
					//USB_PDevice[devinx].Hc.PControl.Total_Length = USB_PDevice[devinx].Hc.PControl.Current_Length;
					//CascadeM_PassThrough_Setup_Data_In(slave,port,devinx);																				
					//USB_PDevice[devinx].Hc.PControl.Current_Length = 0;
					//USB_PDevice[devinx].Hc.PControl.Total_Length   = 0;
					HCTD_Channel_Table[td_id].Total_Length = HCTD_Channel_Table[td_id].Current_Length; // not complete
					//TASK_Wait_Current(); //wait for next data page in, then send out last page					
					//return;
				}
			}
			//hanlde the actual byte is small than total byte,and can not modify by 8
			if((USB_PDevice[devinx].Hc.Control.State == CTRL_STATUS_OUT) ||(USB_PDevice[devinx].Hc.Control.State == CTRL_STATUS_OUT_FORCE))
			{					
				if (USB_PDevice[devinx].Hc.Control.State == CTRL_STATUS_OUT_FORCE)
				{	
					printf("STATSU OUT(%02x)\n\r",(U16_T)CascadeM_Passthrough_State[devinx]);
					printf("T:%d,C:%d\n\r",(U16_T)USB_PDevice[devinx].Hc.PControl.Total_Length,
										   (U16_T)USB_PDevice[devinx].Hc.PControl.Current_Length);
				}				
				USB_PDevice[devinx].Hc.PControl.Total_Length = USB_PDevice[devinx].Hc.PControl.Current_Length;
				HCTD_Channel_Table[td_id].Total_Length = HCTD_Channel_Table[td_id].Current_Length; // not complete
				
			}

			if ((USB_PDevice[devinx].Hc.PControl.Current_Length == USB_PDevice[devinx].Hc.PControl.Total_Length)) //page full
			{	//if page full,send back to Remote Transmitter				
				//CascadeM_PassThrough_Setup_Data_In(slave,port,devinx);
				if (CascadeM_Passthrough_State[devinx] & CASCADEM_PASSTHROUGH_IN_SEND_MASK)
				{
					//printf("Send_IN_Data page...\n\r");
					CascadeM_Setup_Data_In(slave,port,devinx);	
				}	
				else
				{	
					CascadeM_Passthrough_State[devinx] |= CASCADEM_PASSTHROUGH_IN_PAGE_MASK; //no wait for IN request
				}	
				
				USB_PDevice[devinx].Hc.PControl.Current_Length = 0;
				
				if (HCTD_Channel_Table[td_id].Total_Length != HCTD_Channel_Table[td_id].Current_Length) // not complete
				{
					USB_PDevice[devinx].Hc.PControl.Total_Length = HCTD_Channel_Table[td_id].Total_Length - HCTD_Channel_Table[td_id].Current_Length;
					if (USB_PDevice[devinx].Hc.PControl.Total_Length > CASCADE_PAGE_SIZ)
					{
						USB_PDevice[devinx].Hc.PControl.Total_Length = CASCADE_PAGE_SIZ; //page length
					}
					USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_HC_SETUP_DATA_IN; // next wait for HC data in
					break;
				}
			}
			if (USB_PDevice[devinx].Hc.Control.State == CTRL_STATUS_OUT_FORCE)
			{
				USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_COMPLETE; // next wait for HC data in
				goto TASK_CascadeM_USBHC_PassThrough_Restart;
			}	
			else	
				TASK_Active(TASK_TYPE_USB,TASK_USBHC_Handle_Control_ID,0,devinx,0,0); // perform the ctrl command
			//after send out complete, then retriggle the data in operation
			break;
		case PASSTHROUGH_DC_SETUP_STATUS_IN: //DC side Setup Status Stage
#if (CASCADEM_USB_PASSTHROUGH_DEBUG_MODE)
			printf(" @P@ DC Status IN\n\r");
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
			CascadeM_PassThrough_Setup_End(slave,port,devinx);
#if (CASCADEM_USB_PASSTHROUGH_DEBUG_MODE)
			printf("@P_IN@ PassThrough Complete \n\r");
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
			Result = USB_SUCCESS; // ternimate
			break;
		case PASSTHROUGH_HC_SETUP_DATA_OUT: //Setup Data Stage
			if (USB_PDevice[devinx].Hc.Control.State == CTRL_DATA_OUT)
			{
				remain_len = USB_PDevice[devinx].Hc.PControl.Total_Length - USB_PDevice[devinx].Hc.PControl.Current_Length;
				if (remain_len > USB_HC_ATL_BUF_SIZE)
					remain_len = USB_HC_ATL_BUF_SIZE;

				USB_PDevice[devinx].Hc.PControl.Data_Length = remain_len;
				TASK_Active(TASK_TYPE_USB,TASK_USBHC_Handle_Control_ID,0,devinx,0,0);  // Generate the Setup Send command
			}
			break;
		case PASSTHROUGH_HC_SETUP_DATA_OUT_DONE:
			USB_PDevice[devinx].Hc.PControl.Data_Length = HCTD_Channel_Table[td_id].ActualByte;
			USB_PDevice[devinx].Hc.PControl.Current_Length += USB_PDevice[devinx].Hc.PControl.Data_Length;

			if (USB_PDevice[devinx].Hc.Control.State == CTRL_DATA_OUT) //still has data to send out
			{
				USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_HC_SETUP_DATA_OUT;
				if (USB_PDevice[devinx].Hc.PControl.Total_Length == USB_PDevice[devinx].Hc.PControl.Current_Length) // out page done
				{
					CascadeM_PassThrough_Setup_Data_Out_Enable(slave,port,devinx);
					break;
				}
				goto TASK_CascadeM_USBHC_PassThrough_Restart;
			}
			break;
		case PASSTHROUGH_DC_SETUP_STALL: //DC side Setup Status Stage
			Result = USB_SUCCESS;
			CascadeM_PassThrough_Setup_Stall(slave,port,devinx);
			break;
		case PASSTHROUGH_COMPLETE: //PassThroug Complete
#if (CASCADEM_USB_PASSTHROUGH_DEBUG_MODE)
			printf("@P@ PassThrough Complete \n\r");
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
			//Check the total length
			Result = USB_SUCCESS;
			CascadeM_PassThrough_Setup_End(slave,port,devinx);
			break;
	}

	if (Result == USB_SUCCESS)
	{
#if (SYSTEM_MSC_DEVICE_SUPPORT)
		if (devinx == USBHC_MSC_Devinx)
		{
			if (USBHC_MSC_State & MSC_STATE_CHECK_BULK_IN)
			{
				if ((USB_PDevice[devinx].Hc.PControl.Setup.b.bmRequestType == 0x02) &&
					(USB_PDevice[devinx].Hc.PControl.Setup.b.bRequest == CLEAR_FEATURE) &&
					(USB_PDevice[devinx].Hc.PControl.Setup.b.wValue.w == 0x0000))
				{
					//should create a IN after passthrough complete
					USBHC_MSC_State &= ~MSC_STATE_CHECK_BULK_IN;						
					if (USBHC_MSC_State & MSC_STATE_BULK_IN)
					{
						USBHC_MSC_State &= ~MSC_STATE_BULK_IN;
						USBHC_MSC_CSW_After_Control(devinx);						
					}
				}
			}
		}
#endif //#if (SYSTEM_MSC_DEVICE_SUPPORT)
#if (USB_PASSTHROUGH_DEBUG_MODE)
		printf("@@ Done=>TASK_USBHC_PassThrough Complete\n\r");
		printf("\n\r@P@ TD_Free:%d\n\r\n\r",(U16_T)td_id);
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
		USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
		USBHC_CORE_Free_TD(td_id);
		USB_PDevice[devinx].Hc.gState = USB_PDevice[devinx].Hc.PassThrough_gState_bk;
	}
	else if (Result == USB_ERROR)
	{
		USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_FAIL;
#if (USB_PASSTHROUGH_DEBUG_MODE)
		printf("@@ Ternimate=>PASSTHROUGH_FAIL\n\r");
#endif
	}
}
 
/*----------------------------------------------------------------------------
 * void CascadeM_Audio_ISR_Handle(void)
 * Purpose:
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
void CascadeM_Audio_ISR_Handle(void)
{
	U8_T len;
	U8_T slave;
	
	switch(USBHC_Audio_ISR[ISR_USBHC_Audio_Rp].ISR_Type)
	{
		case USBHC_ISR_ISO_OUT:
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
			if (CCMST_AudioOutBusy == 0)
			{	
				CascadeM_Audio_Data_Out_Buf_Check();
			}
#endif			
			break;
		case USBHC_ISR_ISO_IN:			
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
			len = HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte0_Actual_Byte;
			if (len) //start copy data			
			{			
				//copy data out
				DMA_GrantXdata(CADR_USB_Audio_Buf[CADR_USB_HC_Audio_Inx].Buf,HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].Buf,len+2);
				len += 2;	
				slave = KVM_CurrentUSBAudio >> 2;			
				CADR_USB_Audio_Buf[CADR_USB_HC_Audio_Inx].Token.bCommand = CASCADE_USB_ISO_IN;
				CADR_USB_Audio_Buf[CADR_USB_HC_Audio_Inx].Token.iLen	 = len;
				CADR_USB_Audio_Buf[CADR_USB_HC_Audio_Inx].Token.bRequest = 0;
				CADR_USB_Audio_Buf[CADR_USB_HC_Audio_Inx].Token.bIndex	 = USBHC_Audio_Devinx;
				CADR_USB_Audio_Buf[CADR_USB_HC_Audio_Inx].Token.bValue	 = KVM_CurrentUSBAudio & 0x03;
				CascadeM_Cmd_Transmit((U8_T *) &CADR_USB_Audio_Buf[CADR_USB_HC_Audio_Inx],
						  			  (CASCADE_CONTROL_TRANSMIT|CASCADE_CONTROL_NONE_FREE),
						  			  len+CASCADE_PACKET_HEADER_LEN,
						  			  100,
						  			  slave);	 //report start	
						
				//Add the FIFO pointer
				CADR_USB_HC_Audio_Inx++;
				if (CADR_USB_HC_Audio_Inx >= SYSTEM_CASCADE_AUDIO_BUF_CNT)
					CADR_USB_HC_Audio_Inx = SYSTEM_CASCADE_AUDIO_IN_START;
				
			}	
			USBHC_AUDIO_In_Start(TD_ACTIVED_MASK);
			//CascadeM_Audio_In_Enable();	 //enable the intput first
#endif			
			break;	
	}	
}
#endif
/*-------------------------------------------------------------------------------
 * void CascadeM_Passthrough_HidInterval_Handle(CASCADE_PacketHeader *token)
 * Purpose:
 * Params :
 * Returns: NONE
 * Note:
 *-------------------------------------------------------------------------------*/
void CascadeM_Passthrough_HidInterval_Handle(CASCADE_PacketHeader *token)
{		
	USBHC_CORE_Change_IntTransfer_Interval(token->bIndex,
										   token->bRequest,
										   token->bValue);	
}

/*------------------------------------------------------------------------------------------------------------------
 * RESULT CascadeM_Passthrough_Protocol_Setting_Handle(U8_T slave,U8_T port_devinx,U8_T intf,U8_T value) 
 * Purpose:
 * Params :
 * Returns: NONE
 * Note:
 *------------------------------------------------------------------------------------------------------------------*/
RESULT CascadeM_Passthrough_Protocol_Setting_Handle(U8_T hcperform,U8_T slave,U8_T port_devinx,U8_T intf,U8_T value)
{		
	U8_T port;
	U8_T devinx;
	U8_T value_mask;
	U8_T hcprotocol;
	
	port  = (port_devinx >> 4) + (slave << 2);
	devinx = (port_devinx & ~0xf0);		
	value_mask = 0x01 << intf; 
	if (value==0) // Boot protocol
	    hcprotocol = value_mask;
	else    
		hcprotocol = 0;
	
	//Update DC device current protocol setting
	//printf("Pass_HC:%bx,P:%bx,D:%bx,intf:%bx,protocol:%bx\n\r",hcperform,port,devinx,intf,value);
	
	USBDC_Device[devinx].Current_Protocol[port] &= ~value_mask;		
	USBDC_Device[devinx].Current_Protocol[port] |= hcprotocol;
	hcperform=0;
	if (port == KVM_CurrentHost) //if port is under actived
	{		
		if (hcprotocol != (USB_PDevice[devinx].Current_Protocol & value_mask))		
		{			
			USB_PDevice[devinx].Current_Protocol &=	~value_mask;
			USB_PDevice[devinx].Current_Protocol |=	hcprotocol;			
			if (USB_PDevice[devinx].bVirtual_Attr & VIRTUAL_ATTR_MASK)
			{											
				//printf("API Transmit Set_Protocol\n\r");
#if (SYSTEM_CASCADE_API_SUPPORT)
				Cascade_API_Transmit_HID_Protocol(devinx,intf,hcprotocol);					
#endif /* #if (SYSTEM_CASCADE_API_SUPPORT) */
			}	
			else
			{	
				//printf("HC record\n\r");
				//USB_PDevice[devinx].Current_Protocol &=	~value_mask;
				//USB_PDevice[devinx].Current_Protocol |=	hcprotocol;												
				//printf("Pass_HC Set Protocol\n\r");
				USBHC_Set_Dev_Inf_Protocol(port,devinx,intf,hcprotocol);				
				return USB_WAIT;
			}
		}
		/*
		else		
		{
			if (hcprotocol != (USB_PDevice[devinx].Current_Protocol & value_mask))
			{	
				printf("Pass_HC Set Protocol\n\r");
				USBHC_Set_Dev_Inf_Protocol(port,devinx,intf,hcprotocol);
			}	
		}
		*/		
	}	
	//if (perform==0)		
	return USB_SUCCESS;		
}

/*-----------------------------------------------------------------------------------
 * void CascadeM_Passthrough_Alternate_Setting_Handle(CASCADE_PacketHeader *token)
 * Purpose:
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------------*/
void CascadeM_Passthrough_Alternate_Setting_Handle(CASCADE_PacketHeader *token)
{	
	
	if (token->bValue != USB_PDevice[token->bIndex].Current_AlternateSetting[token->bRequest])
	{
		USB_PDevice[token->bIndex].Current_AlternateSetting[token->bRequest] = token->bValue;
		USBHC_Set_Dev_Interface(token->bIndex,token->bRequest,token->bValue); // interface,alternate setting
	}
}

/*---------------------------------------------------------------------------------
 * void CascadeM_Passthrough_Configuration_Handle(CASCADE_PacketHeader *token)
 * Purpose:
 * Params :
 * Returns: NONE
 * Note:
 *--------------------------------------------------------------------------------*/
void CascadeM_Passthrough_Configuration_Handle(CASCADE_PacketHeader *token)
{
	if (USB_PDevice[token->bIndex].ConfigurationValue != token->bValue)
	{
		if (token->bValue)
		{
			USB_PDevice[token->bIndex].ConfigurationValue = token->bValue;
			USBHC_Set_Dev_Configuration(token->bIndex,token->bValue);
		}
	}
}

/*----------------------------------------------------------------------------
 * void CascadeM_USB_Device_Check_IntervalU8_T slave,U8_T port,U8_T devinx)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
/*
void CascadeM_USB_Device_Switch(U8_T slave,U8_T target_port,U8_T devinx)
{	
	U16_T len;		
	
	len = Cascade_Set_Data_Transmit_Header(0,NULL,NULL);		
	//Trigger the Transmit		
	Cascade_Data_TokenP->bCommand  = CASCADE_USB_DEVICE_SWITCH;		
	//Cascade_Data_TokenP->iLen	   	= 0;	
	//Cascade_Data_TokenP->bRequest	= 0;	
	Cascade_Data_TokenP->bIndex	= devinx;	
	Cascade_Data_TokenP->bValue	= target_port; //mount port
	CascadeM_Transmit_Processing_Flag = 1;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_DISABLE;
#endif	// #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) 	
//	CascadeM_Cmd_Transmit(Cascade_Transmit_Send_Buf,
						  CASCADE_CONTROL_TRANSMIT,
						  len,
						  100,
						  slave);	 //report start
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	USBHC_INT_ENABLE;
#endif	// #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) 									  
	CascadeM_Transmit_Processing_Flag = 0;
#if (CASCADEM_CMD_DEBUG)
	printf("(M) Device Switch[(Slave:%d,Devinx:%d)-Port:%02x]\n\r",(U16_T)slave,(U16_T)devinx,(U16_T)port);
#endif // #if (RECEIVER_CMD_DEBUG)

#if (SYSTEM_CASCADE_API_SUPPORT)
	CascadeM_Transmit_API_Mount_State(devinx,VDEV_MOUNT);
#endif
}
*/
/*----------------------------------------------------------------------------
 * void CascadeM_USB_Maintain_Connect_State(U8_T hostid,U8_T value)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
void CascadeM_USB_Maintain_Connect_State(U8_T hostid,U8_T value)
{
	U8_T	byte_mask,bit_mask;
	U8_T    old_value;
	
	//printf("Maintain Connect State:hostid=%bu,Value=%bx\n\r",hostid,value);		
	byte_mask = hostid >> 3;
	bit_mask  = BIT_MASK[hostid & 0x07];
	old_value = CascadeM_Sys_State.Connect_State[byte_mask] & bit_mask;
	CascadeM_Sys_State.Connect_State[byte_mask] &= ~bit_mask;
	if (value)
	{	
		CascadeM_Sys_State.Connect_State[byte_mask] |= bit_mask;	
	}		
	//printf("State=%02bx\n\r",CascadeM_Sys_State.Connect_State[0]);	
	Cascade_Compare_State(old_value,value);
}

/*----------------------------------------------------------------------------
 * void CascadeM_USB_Maintain_KVM_Connect_State(U8_T hostid,U8_T value)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
void CascadeM_USB_Maintain_KVM_Connect_State(U8_T hostid,U8_T value)
{
	U8_T	byte_mask,bit_mask;
	U8_T    old_value;
	
	//printf("Maintain KVM State:hostid=%bu,Value=%bx\n\r",hostid,value);
	byte_mask = hostid >> 3;
	bit_mask  = BIT_MASK[hostid & 0x07];
	old_value = CascadeM_Sys_State.KVM_State[byte_mask] & bit_mask;
	CascadeM_Sys_State.KVM_State[byte_mask] &= ~bit_mask;
	if (value)
	{	
		CascadeM_Sys_State.KVM_State[byte_mask] |= bit_mask;					 
	}
#if (SYSTEM_KVM_BOX_CASCADE_SUPPORT)			
	else
	{
		if (old_value) //if kvm has been conntected
		{	
			//printf("  Slvel >>HOST:%bu, KVM Off, Plug Off\n\r",hostid);
			CascadeBoxM_Master_Attched_Control(hostid,0);				
		}	
	}		
#endif /* #if (SYSTEM_KVM_BOX_CASCADE_SUPPORT) */			
	
	//printf("After byte:%bu,KVM_State=%02bx\n\r",byte_mask,CascadeM_Sys_State.KVM_State[byte_mask]);
	
	Cascade_Compare_State(old_value,value);
}

/*----------------------------------------------------------------------------
 * void CascadeM_USB_Maintain_Power_State(U8_T hostid,U8_T value)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
void CascadeM_USB_Maintain_Power_State(U8_T hostid,U8_T value)
{
	U8_T	byte_mask,bit_mask;
	U8_T    old_value;
		
	byte_mask = hostid >> 3;
	bit_mask  = BIT_MASK[hostid & 0x07];
	old_value = CascadeM_Sys_State.Power_State[byte_mask] & bit_mask;
	CascadeM_Sys_State.Power_State[byte_mask] &= ~bit_mask;
	
	//printf("Power State:p=%bu,value=%bu,oldstate=0x%02bx,",hostid,value,CascadeM_Sys_State.Power_State[byte_mask]);
	
	if (value)
		CascadeM_Sys_State.Power_State[byte_mask] |= bit_mask;			
	else
	{
		CascadeM_Sys_State.Connect_State[byte_mask] &= ~bit_mask;		
		CascadeM_Sys_State.KVM_State[byte_mask]     &= ~bit_mask;				
		CascadeM_Sys_State.Suspend_State[byte_mask] &= ~bit_mask;			
	}	
	//printf("newstate=0x%02bx\n\r",CascadeM_Sys_State.Power_State[byte_mask]);
	Cascade_Compare_State(old_value,value);	
}

/*----------------------------------------------------------------------------
 * void CascadeM_USB_Maintain_Suspend_State(U8_T hostid,U8_T value)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
void CascadeM_USB_Maintain_Suspend_State(U8_T hostid,U8_T value)
{
	U8_T	byte_mask,bit_mask;
	U8_T    old_value;
	
	byte_mask = hostid >> 8;
	bit_mask  = BIT_MASK[hostid & 0x07];	
	old_value = CascadeM_Sys_State.Suspend_State[byte_mask] & bit_mask;
	CascadeM_Sys_State.Suspend_State[byte_mask] &= ~bit_mask;
	
	if (value)
		CascadeM_Sys_State.Suspend_State[byte_mask] |= bit_mask;			
	
	Cascade_Compare_State(old_value,value);	
}


/***************************************************************************************************/
/*
** void DATAST_API_Transmit_Hid_Data(U8_T devinx,U8_T intt_id,U8_T *buf,U8_T length)
**  Parameter    : send virtual HID keyboard break report into dc endpoint buffer
**  Description  : 
**
**
*/ 
#if (SYSTEM_CASCADE_API_SUPPORT)
void DATAST_API_Transmit_Hid_Data(U8_T devinx,U8_T intt_id,U8_T *buf,U8_T length)
{
	U8_T *p;
	U8_T index;
	
	p = HC_IntTransfer_Table[intt_id].CascadeDataBuf+CASCADE_PACKET_HEADER_LEN;
	 
	//if byte is too long, then use DMA copy			
	
	for (index=0;index < length ;index++)
	{
		p[index] = buf[index];		
	}
	
#ifndef PS2_HOST		
	if ((USB_PDevice[devinx].bVirtual_Attr & VIRTUAL_ATTR_MASK) == 0x00)	
#endif		
	{	
		CascadeM_Transmit_Intr_Endp_In_Data(devinx,intt_id,length,1);		
	}
}
#endif

/*----------------------------------------------------------------------------
 * Function Name: TASK_CascadeM_Transmit_TimeOut
 * Purpose: Task for check the transmit time out
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void TASK_CASCADEM_Transmit_TimeOut(void)
{	
			
	Task_Active_Table[CascadeM_Transmit_TimeOut_TaskActiveID].Task_Reload.w = 10000; // next for 1 seconds	
}


/*----------------------------------------------------------------------------
 * void CascadeM_Transmit_USB_HID_Setting(U8_T devinx)
 * Purpose: Task for check the transmit time out
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Transmit_USB_HID_Setting(U8_T slave,U8_T devinx)
{	
#if CCMST_ORIGINAL_1
	 CascadeM_Transmit_DataPack(slave,
								CASCADE_USB_CONFIGURATION,
								USB_PDevice[devinx].Hid_SetIdle_Allow,
								devinx,
								USB_PDevice[devinx].ConfigurationValue,
								NO_CTRL_ATTR);	
#else
	pPACKET = Cascade_BuildPacket(CASCADE_USB_CONFIGURATION, USB_PDevice[devinx].Hid_SetIdle_Allow, devinx, USB_PDevice[devinx].ConfigurationValue, 0, 0);
	if (pPACKET)
	{
		CascadeM_Cmd_Transmit((U8_T*)pPACKET, CASCADE_CONTROL_TRANSMIT, CASCADE_PACKET_HEADER_LEN + pPACKET->iLen, 100, slave);
	}
#endif								
}

/*----------------------------------------------------------------------------
 * void CascadeM_Transmit_API_Mount_State(U8_T devinx,U8_T mounttype)
 * Purpose: 
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Transmit_API_Mount_State(U8_T devinx,U8_T mounttype)
{
	U16_T cascade_port;
	
#ifndef PS2_HOST	
	if ((USB_PDevice[devinx].bVirtual_Attr & VIRTUAL_ATTR_MASK) == 0x00)	
#endif		
	{		
		//printf("mountype=%bx,devinx:%bx[%bx]\n\r",mounttype,mount_dev,USB_PDevice[mount_dev].bVirtual_Attr);		
		cascade_port  = CascadeM_Sys_State.Mounted.State[devinx][0];			 
		cascade_port |= CascadeM_Sys_State.Mounted.State[devinx][1] << 4;
		cascade_port |= ((CascadeM_Sys_State.Mounted.State[devinx][2] << 8) | (CascadeM_Sys_State.Mounted.State[devinx][3] << 12));						
#if (SYSTEM_CASCADE_API_SUPPORT)		
		Cascade_API_Transmit_USB_AttachChanged(devinx,0,mounttype,cascade_port);	
#else		
		mounttype = 0;
#endif /* #if (SYSTEM_CASCADE_API_SUPPORT) */		
	}					
}

/*----------------------------------------------------------------------------
 * U16_T  CascadeM_Retrive_MountPort_Value(U8_T devinx)
 * Purpose: 
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
U16_T CascadeM_Retrive_MountPort_Value(U8_T devinx)
{
	U16_T cascade_port;
	
	cascade_port  = CascadeM_Sys_State.Mounted.State[devinx][0];			 
	cascade_port |= CascadeM_Sys_State.Mounted.State[devinx][1] << 4;
	cascade_port |= ((CascadeM_Sys_State.Mounted.State[devinx][2] << 8) | (CascadeM_Sys_State.Mounted.State[devinx][3] << 12));				
	
	return cascade_port;
}

/*----------------------------------------------------------------------------
 * U16_T  CascadeM_Retrive_MountPort_Value(U8_T devinx)
 * Purpose: 
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void TASK_CascadeM_SystemPort_Report(void)
{
	printf("Report System Port=%bu\n\r",CascadeM_KVM_Max_port);
	CaascaseM_System_Report_Ready_Flag = 1;
#if (SYSTEM_CASCADE_API_SUPPORT)	
	Cascade_API_Transmit_SystemPort();
#endif /* #if (SYSTEM_CASCADE_API_SUPPORT) */	
	TASK_Destory_Current();
}

/*----------------------------------------------------------------------------
 * void CascadeM_Slave_Chip_Reset(void)
 * Purpose: Reset the slave Chip via the reset control gpio pin
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeM_Slave_Chip_Reset(void)
{
	U8_T index;
	
	
    SLAVE_IAP_GPIO_PIN = 1;
	//MASTER_RESET_GPIO_PIN = LOW;
	for (index = 0; index < 125 ; index++)
	{
		SLAVE_RESET_GPIO_PIN = 0;
	}
	SLAVE_RESET_GPIO_PIN = 1;	
}

/*----------------------------------------------------------------------------
 * void CascadeM_Box_Redirect_Handle(CASCADE_PacketHeader *dp)
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
#if (SYSTEM_KVM_BOX_CASCADE_SUPPORT)
void CascadeM_Box_Redirect_Handle(CASCADE_PacketHeader *dp)
{	
	CASCADE_PacketHeader *packet;
	CASCADE_PacketHeader *payload;			
				
	payload = (CASCADE_PacketHeader *)dp->buf;		
	packet  = Cascade_BuildPacket(payload->bCommand,payload->bRequest,payload->bIndex,payload->bValue,payload->buf,payload->iLen);		      
	
	if (packet)
	{	
		packet->bIndex = dp->bIndex + (CascadeM_Current_Slave << 2);  
		Cascade_API_Cmd_Transmit((U8_T*)packet, 0,dp->iLen);
		//printf("BOX Redirect,");
		//Disp_Str((U8_T *)packet,dp->iLen);
	}
}
#endif
#endif /* SYSTEM_CASCADE_MASTER */
/* End of cascade_master.c */