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
 * Module Name: extender_receiver.c
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

#if (SYSTEM_EXTENDER_RECEIVER)
/* NAMING CONSTANT DECLARATIONS */
#ifdef PLUG_DETECT
#define RJ45_PLUG		P2_3
#define HDIM_HOTPLUG	P2_2		

#define PLUG_OFF	0
#define PLUG_WAIT	1
#define PLUG_IN		2
#define RJ45_UART_QUIET		25 

#define UART_RJ45_DISABLE_MASK	0x01
#define UART_HDMI_DISABLE_MASK	0x02
#define UART_DISABLE_TIME		10000
#endif
/* GLOBAL VARIABLES DECLARATIONS */
bit     ExtednerR_Skip_PassthroughFlag=0;
bit		Remote_Link_LED_State;
bit		Remote_Link_LED_Flash_Flag;
bit		Remote_Link_LED_Flash_State;
bit     MSC_In_Wait_Flag=0;
bit     ExtenderR_Skip_TimeoutUpdate=0;
bit   	Transmitter_Restart_Flag = 0;
#ifdef  DEVICE_KEEP
bit     ExtenderR_Device_Keep_Flag=0;
U8_T	ExtenderT_Device_Mount_Change=0;
Extender_Device_Keep_Def	DeviceKeepTable[USB_HC_MAX_DEVICE-1];
#endif

U8_T	ExtenderR_TD_Skip_Active_Flag;
U8_T    Remote_Mount_State=0;
U8_T    ExtenderR_Audio_In_Cnt=0;
U8_T	Transmitter_Restart_Cnt=0;

#ifdef PLUG_DETECT
#define RJ45_PLUG_OFF			1
#define RJ45_PLUG_IN			0
#define HDMI_PLUG_OFF			0
#define HDMI_PLUG_IN			1

U8_T    UART_Disable_Flag = 0;
U8_T	RJ45_PLUG_SATE 		= PLUG_OFF;
U8_T	RJ45_Connect_Cnt=0;
U8_T	RJ45_UART_QuietCnt;
U8_T	HDMI_PLUG_SATE 		= PLUG_OFF;
U8_T	HDMI_UART_QuietCnt;
U8_T	TASK_ExtenderR_PlugDetect_ActiveID;
U8_T	TASK_ExtenderR_Uart_Disable_ActiveID=0;
#endif

#if (PROJECT_USB_GENERIC_HID_ENABLE)
PassThrough_TypeDef	ExtenderR_PS2_PassThrough[2];
U8_T ExtenderR_PS2_KB_Queue[PS2_KB_BUF_MAX_CNT];  /* Byte1-Modifiers,Byte2-Reverse,Byte3~8=>Data */
U8_T ExtenderR_PS2_MS_Queue[PS2_MS_BUF_MAX_CNT];  /* Byte1-Modifiers,Byte2-Reverse,Byte3~8=>Data */
U8_T *ExtenderR_PS2_ExtenderR[2];  /* Byte1-Modifiers,Byte2-Reverse,Byte3~8=>Data */

#if (KVM_EXTENDER_HID_SHORT_PACKET)
U8_T ExtenderR_PS2KB_Transfer_Queue[sizeof(Extender_Packet_Int_Header_Def)+PS2_KB_TRANS_BUF_MAX_CNT+2];  /* Byte1-Modifiers,Byte2-Reverse,Byte3~8=>Data */
U8_T ExtenderR_PS2MS_Transfer_Queue[sizeof(Extender_Packet_Int_Header_Def)+PS2_MS_TRANS_BUF_MAX_CNT+2];   /* Byte0-Package Length,Byte1-Page ID,Byte2~BYte5 Data */
#else
U8_T ExtenderR_PS2KB_Transfer_Queue[sizeof(Extender_Packet_Header_Def)+PS2_KB_TRANS_BUF_MAX_CNT+2];  /* Byte1-Modifiers,Byte2-Reverse,Byte3~8=>Data */
U8_T ExtenderR_PS2MS_Transfer_Queue[sizeof(Extender_Packet_Header_Def)+PS2_MS_TRANS_BUF_MAX_CNT+2];  /* Byte0-Package Length,Byte1-Page ID,Byte2~BYte5 Data */
#endif /*  #if (KVM_EXTENDER_HID_SHORT_PACKET) */
#endif /* #if (PROJECT_USB_GENERIC_HID_ENABLE) && (SYSTEM_HARDWARE_PS2_ENABLE) */
U8_T Recevier_Cmd_Token[]=
{
	EXTENDER_CMD_TOKEN,
	EXTENDER_TRANSMITTER_ADDR,
	0x00,
};

U8_T Recevier_Int_Token[]=
{
	EXTENDER_INT_TOKEN,
	EXTENDER_TRANSMITTER_ADDR,
	0x00,
};

/* GLOBAL VARIABLES DECLARATIONS */
U8_T Recevier_Sof_Token[]=
{
	EXTENDER_SOF_TOKEN,
	EXTENDER_TRANSMITTER_ADDR,
	0x00,
};

U8_T Recevier_Ack_Token[]=
{
	EXTENDER_ACK_TOKEN,
	EXTENDER_TRANSMITTER_ADDR,
	0x00,
};

U8_T	TASK_ExtenderR_Connection_Check_ActiveID;
U8_T	TASK_Receiver_RemoteLed_Control_ActiveID;
U8_T	Receiver_Control_State;
U8_T	Receiver_Passthrough_State[USB_HC_MAX_DEVICE];
U8_T	Transmitter_Plug_Off_Cnt = 0;
U8_T	*Receiver_Send_Buf;
U8_T	TASK_USBHC_ExtenderR_PassThrough_ID;
U8_T	TASK_ExtenderR_Actived_Led_Flash_Active_ID;
U8_T	Remote_Devinx_Pup_Index;
U8_T	Remote_Devinx_Pup_Start;
U8_T	*Data_Transmit_Buf;
U8_T	Remote_Device_Report_Pup_Stack[USB_HC_MAX_DEVICE+USB_HC_MAX_HUB];

Extender_Data_Packet_Def	*Receiver_Send_Data_TokenP;
Extender_Cmd_Handle_Def		Extender_Setup;

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
U8_T	ETDR_AudioOutBuf[EXTENDER_AUDIO_OUT_BUF_CNT][EXTENDER_AUDIO_OUT_BUF_SIZE] _at_ 0x5000;
U8_T	ETDR_AudioOutHead = 0;
U8_T	ETDR_AudioOutTail = 0;
U8_T	ETDR_AudioInBuf[EXTENDER_AUDIO_IN_BUF_CNT][EXTENDER_AUDIO_IN_BUF_SIZE] 	  _at_ 0x5800;
U8_T	ETDR_AudioInHead = 0;
U8_T	ETDR_AudioInTail = 0;
U8_T	ETDR_AudioOutHcDoingFlag = 0;
U8_T	ETDR_AudioInWaitFlag = 0;
U8_T	ETDR_AudioInTailSkipFlag = 0;
#endif
static bit	etdrR_isr;
U8_T	burstout_cnt;

TASK_ActiveTable_TypeDef ExtenderR_Connection_Check_TASK=
{
	TASK_TYPE_INTERVAL_MS,
	0,	// Task ID from 0~255
	0,	// Task Wait for Semaphore to active
	0,	// Task Event
	0,	// Task transfer parameter
	RECEIVER_CHECK_DELAY_TIME+TRANSMITTER_CHECK_TIME,	// Task Interval time from
	TRANSMITTER_CHECK_TIME,	// Task Interval Reload
//	0,	// Task wait for period then perform
};

TASK_ActiveTable_TypeDef TASK_Receiver_RemoteLed_Control_TASK=
{
	TASK_TYPE_INTERVAL_MS,
	0,	// Task ID from 0~255
	0,	// Task Wait for Semaphore to active
	0,	// Task Event
	0,	// Task transfer parameter
	1000,// Task Interval time from
	1000,// Task Interval Reload
//	0,	// Task wait for period then perform
};

#if (EXTENDER_LED_FLASH_INDICATE)
TASK_ActiveTable_TypeDef TASK_ExtenderR_Actived_Led_Flash_TASK=
{
	TASK_TYPE_INTERVAL_MS,
	0,	// Task ID from 0~255
	0,	// Task Wait for Semaphore to active
	0,	// Task Event
	0,	// Task transfer parameter
	50,	// Task Interval time from
	50,	// Task Interval Reload
//	0,	// Task wait for period then perform
};
#endif

#ifdef PLUG_DETECT
TASK_ActiveTable_TypeDef TASK_ExtenderR_PlugDetect_TASK=
{
	TASK_TYPE_INTERVAL_MS,
	0,	// Task ID from 0~255
	0,	// Task Wait for Semaphore to active
	0,	// Task Event
	0,	// Task transfer parameter
	40,	// Task Interval time from
	40,	// Task Interval Reload
};

TASK_ActiveTable_TypeDef TASK_ExtenderR_UartDisable_TASK=
{
	TASK_TYPE_INTERVAL_MS,
	0,	// Task ID from 0~255
	0,	// Task Wait for Semaphore to active
	0,	// Task Event
	0,	// Task transfer parameter
	UART_DISABLE_TIME,	// Task Interval time from
	UART_DISABLE_TIME,	// Task Interval Reload
};
#endif

TASK_ActiveTable_TypeDef TASK_ExtenderR_Devcie_Rereport_TASK=
{
	//TASK_TYPE_EVENT,
	TASK_TYPE_INTERVAL_MS,
	0,	// Task ID from 0~255
	0,	// Task Wait for Semaphore to active
	0,	// Task Event
	0,	// Task transfer parameter
	500,	// Task Interval time from
	500,	// Task Interval Reload
//	0,	// Task wait for period then perform
};

/* LOCAL VARIABLES DECLARATIONS */
U8_T	edtr_R_DeviceMount[USB_HC_MAX_DEVICE] = {0};

/* LOCAL SUBPROGRAM DECLARATIONS */
void TASK_Extender_Receiver_Plug_Check(void);
U8_T Extender_Receiver_Cmd_Responsed_Handle(U16_T len);
void ExtenderR_Responsed_Received_Data_Parser(void);
void Receiver_Restart_Control(void);
void TASK_Receiver_RemoteLed_Control(void);
void ExtenderR_Device_Descriptor_Send(U8_T devinx);
void TASK_USBHC_ExtenderR_PassThrough(void);
void ExtenderR_Passthrough_Setup_Data_Out_Handle(void);
void ExtenderR_Passthrough_Intr_Out_Handle(void);
void ExtenderR_PassThrough_Setup_End(U8_T devinx);
void ExtenderR_Passthrough_Setup_Data_In_Enable(void);
void ExtenderR_Setup_Stall(U8_T devinx);
void ExtenderR_Passthrough_Setup_Token_Handle(void);
void ExtenderR_Setup_Data_In(U8_T devinx);
void ExtenderR_Setup_Data_Out_Enable(U8_T devinx);
void TASK_ExtenderR_Actived_Led_Flash(void);
void ExtenderR_Remote_Device_Restart_Create(void);
void ExtenderR_Passthrough_Setup_Status_Out_Handle(void);
void ExtenderR_PassThrough_Setup_Condition_Reset(U8_T);
void TASK_ExtenderR_Devcie_Rereport(void);
void ExtenderR_Clear_Remote_Passthrough_Condition(void);
void ExtenderR_Passthrough_KVM_KB_Led_Handle(void);
void ExtenderR_Clear_IntrBuffer_Wait(void);
void ExtenderR_Clear_Intr_Send_OK(void);
void ExtenderR_Transimiiter_PlugIn_Set(void);
void TASK_ExtenderR_Devcie_Mount(void);
void ExtenderR_Transmitter_Data_Send(U8_T control,U8_T request,U8_T index,U8_T value,U16_T data_len,U8_T *data_buf);
void ExtenderR_USB_Command_Handle(U8_T devinx,U8_T scm,U8_T *buf);
void ExtenderR_Interrupt_DataTypeTransfer_Check(U8_T index);
#ifdef PLUG_DETECT
void TASK_Receiver_PlugDetect(void);
void TASK_ExtenderR_Uart_Disable(void);
void ExtenderR_Check_Uart_Disable(void);
#endif

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
void ExtenderR_Passthrough_Audio_Data_Out_Handle(void);
#endif
void Dump_Receiver_Buff(void);
void ExtenderR_Transmit_MSC_Size_Setting(void);
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
/* EXTERNAL SUBPROGRAM DECLARATIONS */
extern RESULT USBDC_VIRTUAL_Setup_DataStageOut_Handle(U8_T devinx,U8_T port);

/*----------------------------------------------------------------------------
 * Function Name: EXTENDER_Receiver_Init
 * Purpose: initial the extender receiver
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void EXTENDER_Receiver_Init(void)
{
	Receiver_Control_State 	= 0x00;
	//Extender Task control
	ExtenderR_Connection_Check_TASK.Task_ID			= TASK_Create(TASK_Extender_Receiver_Plug_Check);
#ifndef PWAYTECH_01	
	TASK_Receiver_RemoteLed_Control_TASK.Task_ID	= TASK_Create(TASK_Receiver_RemoteLed_Control);
#endif
	
#ifdef PLUG_DETECT
	TASK_ExtenderR_PlugDetect_TASK.Task_ID			= TASK_Create(TASK_Receiver_PlugDetect);
	TASK_ExtenderR_UartDisable_TASK.Task_ID			= TASK_Create(TASK_ExtenderR_Uart_Disable);
#endif
	
#if (EXTENDER_LED_FLASH_INDICATE)	
	TASK_ExtenderR_Actived_Led_Flash_TASK.Task_ID	= TASK_Create(TASK_ExtenderR_Actived_Led_Flash);
#endif	
	TASK_USBHC_ExtenderR_PassThrough_ID				= TASK_Create(TASK_USBHC_ExtenderR_PassThrough);
	TASK_ExtenderR_Devcie_Rereport_TASK.Task_ID		= TASK_Create(TASK_ExtenderR_Devcie_Rereport);

#if (SYSTEM_TASK_DUMP_SUPPORT)
#if (EXTENDER_LED_FLASH_INDICATE)
	printf("ExtenderR_Connection_Check_TASK=%d\n\r",(U16_T)ExtenderR_Connection_Check_TASK.Task_ID);
#endif	
	printf("TASK_Receiver_RemoteLed_Control_TASK=%d\n\r",(U16_T)TASK_Receiver_RemoteLed_Control_TASK.Task_ID);
	printf("TASK_ExtenderR_Actived_Led_Flash_TASK=%d\n\r",(U16_T)TASK_ExtenderR_Actived_Led_Flash_TASK.Task_ID);
	printf("TASK_USBHC_ExtenderR_PassThrough_ID=%d\n\r",(U16_T)TASK_USBHC_ExtenderR_PassThrough_ID);
	printf("TASK_ExtenderR_Devcie_Rereport_TASK=%d\n\r",(U16_T)TASK_ExtenderR_Devcie_Rereport_TASK.Task_ID);
#endif
	TASK_ExtenderR_Connection_Check_ActiveID = TASK_Run(&ExtenderR_Connection_Check_TASK);
	
#ifdef PLUG_DETECT
	TASK_ExtenderR_PlugDetect_ActiveID = TASK_Run(&TASK_ExtenderR_PlugDetect_TASK);
#endif	

#ifndef PWAYTECH_01
	TASK_Receiver_RemoteLed_Control_ActiveID = TASK_Run(&TASK_Receiver_RemoteLed_Control_TASK);
#endif	

#if (EXTENDER_LED_FLASH_INDICATE)	
	TASK_ExtenderR_Actived_Led_Flash_Active_ID = TASK_Run(&TASK_ExtenderR_Actived_Led_Flash_TASK);
#endif	
	Task_Active_Table[TASK_ExtenderR_Actived_Led_Flash_Active_ID].Task_Event |= TASK_EVENT_SKIP;
	memset(Receiver_Passthrough_State,0x00,sizeof(Receiver_Passthrough_State));
	Remote_Link_LED_State = 0;
	Remote_Link_LED_Flash_Flag = 0;
	Remote_Link_LED_Flash_State= 0;
	Remote_Devinx_Pup_Index = 0;
	
#if (PROJECT_USB_GENERIC_HID_ENABLE) && (SYSTEM_HARDWARE_PS2_ENABLE)
	memset((U8_T *)ExtenderR_PS2_PassThrough,0x00,sizeof(ExtenderR_PS2_PassThrough));
	ExtenderR_PS2_PassThrough[0].Buf = ExtenderR_PS2_KB_Queue;
	ExtenderR_PS2_PassThrough[0].BufSize = sizeof(ExtenderR_PS2_KB_Queue);
	ExtenderR_PS2_PassThrough[1].Buf = ExtenderR_PS2_MS_Queue;
	ExtenderR_PS2_PassThrough[1].BufSize = sizeof(ExtenderR_PS2_MS_Queue);
	
	ExtenderR_PS2_ExtenderR[0] = ExtenderR_PS2KB_Transfer_Queue;
	ExtenderR_PS2_ExtenderR[1] = ExtenderR_PS2MS_Transfer_Queue;
#endif	
}

/*----------------------------------------------------------------------------
 * Function Name: EXTENDER_Receiver_Receive_Handle
 * Purpose: After Send out the transmit fifo, the receiver will go into 
 *          listen mode for receive the responsed from transmiiter, after 
 *          DMA generate the interrupt for receive time out, this function 
 *          will be perform to handle the command responsed
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void EXTENDER_Receiver_Receive_Handle(void)
{	
	EXTENDER_Get_Total_Receive_Len();
	 	 
	if (((Extender_Receive_Cmd_Parser_State & EXTENDER_CMD_TOKEN_LEN_SHORT) == 0)
	&& ((Extender_Receive_Data_Parser_State & EXTENDER_DATA_PACKET_LEN_SHORT) == 0)
	&& (Receive_Buffer_Cnt < sizeof(Extender_Cmd_Token_Def)))
	{		
		return;
	} 
	 
EXTENDER_Receiver_Receive_Handle_Restart:	
	//if ((Extender_Control_State & EXTENDER_CONTROL_TRANSMIT) == 0x00)	//no any command in transmit operation, so clear the receive
	//{	//skip the content		
	//	Receiver_Control_State &= ~TRANSMITTER_PLUG_CHECK_MASK;
	//	EXTENDER_Update_Receive_Index(Receive_Buffer_Cnt);
	//}
	//else
	{	
		if (Extender_Control_State & EXTENDER_CONTROL_TRANSMIT_DONE)
		{
			switch (Trainsmit_FIFO[EXTENDER_Transmit_OutP].Buf[0])
			{
				case EXTENDER_SOF_TOKEN:	
					//printf("a");
					Receiver_Control_State &= ~TRANSMITTER_PLUG_CHECK_MASK;
					if (Transmitter_Restart_Flag==0)
					{
						Transmitter_Restart_Cnt++;
#if (HUART_BAUD >= HUART_115K)						
						if (Transmitter_Restart_Cnt > 10)
#else							
						if (Transmitter_Restart_Cnt > 1000)
#endif							
						{
							Receiver_Restart_Control();
						}							
					}						
				case EXTENDER_CMD_TOKEN:				
					if (ExtenderR_Resonsed_Received_Handle() == 0)
					{						
						Extender_Receive_Parser_State_Reset();						
						goto EXTENDER_Receiver_Receive_Handle_Exit;
					}
					break;
				default:
EXTENDER_Receiver_Receive_Handle_Exit:
					EXTENDER_Update_Receive_Index(Receive_Buffer_Cnt);
					Receive_Buffer_Cnt = 0;
					break;
			}
		  
			if (Extender_Receive_Cmd_Parser_State & EXTENDER_CMD_TOKEN_LEN_SHORT)
			{	// cmd length not enough
				return;
			}

			if (Extender_Receive_Data_Parser_State & EXTENDER_DATA_PACKET_LEN_SHORT)
			{
				// data packet length not enough
				Extender_Receive_Start_TimeOut_Check();					
				return;
			}
			
			Extender_Receive_Stop_TimeOut_Check();

			if (Receive_Buffer_Cnt >= 3)
			{				
				goto EXTENDER_Receiver_Receive_Handle_Restart;
			}	
					
			if ((EXTENDER_Multi_TR_State & EXTENDER_RECEIVE_WAIT) == 0x00)
			{	//Add the FIFO pointer, go for next Transmitte
				EXTENDER_Transmit_Update();
			}		
		}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: TASK_Extender_Receiver_Plug_Check
 * Purpose: Task to check the transmitter plug in 
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void TASK_Extender_Receiver_Plug_Check(void)
{
	//Check the transmitter communicating state
	//If in communication, skip this time.
	if (Receiver_Control_State & TRANSMITTER_PLUG_CHECK_MASK)
	{		
		return; //Do next time
	}
#ifdef EXTENDER_SOF_CNT_CHECK	
	if (EXTENDER_SofCount >= EXTENDER_SOF_ENQ_MAX)
	{
		//printf("SOF:%bu\n\r",EXTENDER_SofCount);
		return;
	}		
	EXTENDER_SofCount ++;
#endif
	Receiver_Control_State |= TRANSMITTER_PLUG_CHECK_MASK; //start the check
	Extender_Token_Transmit(Recevier_Sof_Token,EXTENDER_CONTROL_NONE_FREE,3,100,1);	 // answer in 100 ms
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Resonsed_Received_Handle
 * Purpose: Task to check the transmitter plug in 
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
U8_T ExtenderR_Resonsed_Received_Handle(void)
{
	U8_T	token_found=0;
	U8_T	Receiver_Control_State_Bk;
	U8_T	*buf;
	U8_T	cmd;
	U8_T	devinx;
	U8_T	td_id;
	U16_T	period = TRANSMITTER_CHECK_TIME;
	Extender_Data_Packet_Def	*data_token;
	U8_T    err_flag=0;

	if (EXTENDER_Multi_TR_State & EXTENDER_RECEIVE_START) //if command has been received
	{
		token_found = 1;
	}
	else
	{
		token_found = Extender_Receive_Cmd_Token_Check(Receive_Buffer_Cnt,&Receive_Cmd_Token,EXTENDER_RECEIVER_ADDR);
	}
		
	if (token_found)
	{
		//Reset CMD Token parser result
		Receiver_Control_State_Bk = Receiver_Control_State;
		cmd = Receive_Cmd_Token.bCmd_Token;
		if (cmd == EXTENDER_NAK_TOKEN) //transmitter can't understand the command,
		{			
			//1.Transmitter still connection.
			Receiver_Control_State |= TRANSMITTER_CONNECTION_MASK;
			Transmitter_Plug_Off_Cnt = 0; //reset connector
			Extender_Receive_Parser_State_Reset();

			//2.Handke NAK, resend the Command again
			Trainsmit_FIFO[EXTENDER_Transmit_OutP].ErrCnt++;
			if (Trainsmit_FIFO[EXTENDER_Transmit_OutP].ErrCnt < EXTENDER_SEND_MAX) //if resend over 5 timmes			
			{				
				if (EXTENDER_Transmit_OutP == 0)
					EXTENDER_Transmit_OutP = (EXTENDER_TRANSMIT_FIFO_MAX -1);
				else	
					EXTENDER_Transmit_OutP--;								
			}			
			return 1;
		}
		else if (cmd == EXTENDER_ACK_TOKEN) //Transmitter need to send data
		{
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			//(1).Handle the process for send out but wait for transmitter responsded, then do necessary steps
			//(1-1).What is the request for receiver?
			if ((EXTENDER_Multi_TR_State & EXTENDER_RECEIVE_START) == 0x00)//it is the first time handle the ack token
			{
				if (Trainsmit_FIFO[EXTENDER_Transmit_OutP].DataLen > 3)
				{
					buf = Trainsmit_FIFO[EXTENDER_Transmit_OutP].Buf+sizeof(Extender_Cmd_Token_Def);
					data_token = (Extender_Data_Packet_Def *)(buf);
					devinx = data_token->bIndex & 0x0f; // target device index number
					switch(data_token->bRequest)
					{
						case EXTENDER_USB_DESCRIPTOR:
							if (data_token->bValue == EXTENDER_USB_DESCRIPTOR_END)
							{
								USBDC_Device[devinx].DevIdx |= USBDC_DEVIDX_INTR_SEND_OK_MASK;
							}	
							break;
						case EXTENDER_USB_INTR_IN:
							ExtenderR_Interrupt_DataTypeTransfer_Check(data_token->bIndex);							
							break;
						case EXTENDER_USB_DATA_IN_STAGE: //passthrough data in
							Receiver_Passthrough_State[devinx] &= ~RECEIVER_PASSTHROUGH_IN_SEND_MASK;//clear the page in data send flag
							if (USB_PDevice[devinx].Hc.PassThrough_State == PASSTHROUGH_HC_SETUP_DATA_IN)
							{
								if (USB_PDevice[devinx].Hc.Control.State == CTRL_DATA_IN)
								{
									td_id = USB_PDevice[devinx].Hc.Control.TdNum; // which host channel is used by device
									TASK_Active(TASK_TYPE_USB,TASK_USBHC_Handle_Control_ID,td_id,devinx,0,0);
								}
							}
							break;				
						case EXTENDER_USB_REMOUNT: //ask transmitter to remount device
							if (Remote_Devinx_Pup_Start < Remote_Devinx_Pup_Index)
							{
								TASK_Run(&TASK_ExtenderR_Devcie_Rereport_TASK);
							}
/*							
#ifdef DEVICE_KEEP
							else
							{	
								if (ExtenderR_Device_Keep_Flag)
								{
									ExtenderR_Device_Keep_Flag = 0;
									ExtenderR_Remote_Device_List_Contorl(0x00);	
								}									
							}		
								
#endif
*/							
							//break; //do not take the mark away, it is on purpose
						case EXTENDER_USB_MOUNT:
							devinx = data_token->bValue;
							USB_PDevice[devinx].Hc.Etdr_RejectMountFlag = 0;
							break;
						case EXTENDER_USB_UNMOUNT:
							devinx = data_token->bValue;							
							break;
						case EXTENDER_USB_SETUP_DONE: //passtrhoug data section is closed							
							USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_IDLE; //wait for next passthrough
							Receiver_Passthrough_State[devinx] = 0;
							malloc_free(USB_PDevice[devinx].Hc.Control.Buf); // not free this time, should wait for send out complete
							USB_PDevice[devinx].Hc.Control.Buf = 0;
							break;						
						default:
							break;
					}
				}													
			}
			
			if ((Receiver_Control_State & TRANSMITTER_CONNECTION_MASK) == 0x00) // device plug in
			{	
				printf("=> Transmitter Plug In\n\r");
				Receiver_Control_State |= TRANSMITTER_CONNECTION_MASK; // device plug in
				Transmitter_Restart_Flag = 0;
				Transmitter_Restart_Cnt = 0;
			}	
			Transmitter_Plug_Off_Cnt = 0; //reset connector

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			//(2).Handle the condition if has data section follow by ACK
			if (Extender_Receive_Cmd_Parser_State & EXTENDER_DATA_PACKET_WAIT) //parser has beeon done
			{
				if ((EXTENDER_Multi_TR_State & EXTENDER_RECEIVE_START) == 0x00)
				{	//start the data section
					memset((U8_T *)&Extender_Setup,0x00,sizeof(Extender_Setup));
					EXTENDER_Multi_TR_State |= (EXTENDER_RECEIVE_WAIT|EXTENDER_RECEIVE_START);
					memcpy((U8_T *)&Extender_Setup.Cmd_Token,(U8_T *)&Receive_Cmd_Token,sizeof(Extender_Cmd_Token_Def));
					Extender_Setup.iRemain_Len = sizeof(Extender_Data_Packet_Def) + 2;
					Extender_Receive_Data_Parser_State = 0;
				}
				//if still has data length
				if (Receive_Buffer_Cnt) //if still has data, then go to data section processing
				{					
					ExtenderR_Responsed_Received_Data_Parser(); //Retrive the first data if already has
				}
			}
			else
			{								
				if (Receive_Buffer_Cnt)
				{	
					Extender_Receive_Cmd_Parser_State |= EXTENDER_DATA_PACKET_WAIT;
				}				
			}	
		}
		else if (cmd == EXTENDER_STALL_TOKEN) //Transmitter need to send data
		{
			buf = Trainsmit_FIFO[EXTENDER_Transmit_OutP].Buf+sizeof(Extender_Cmd_Token_Def);
			data_token = (Extender_Data_Packet_Def *)buf;
			if ((data_token->bRequest == EXTENDER_USB_MOUNT) || (data_token->bRequest == EXTENDER_USB_REMOUNT))
			{
				devinx = data_token->bValue;
				USB_PDevice[devinx].Hc.Etdr_RejectMountFlag = 1;
#if (KVM_BUZZER_SUPPORT)
				BUZZER_Script_Active(DeviceErrorSound);
#endif //#if (KVM_BUZZER_SUPPORT)
			}

			Extender_Receive_Parser_State_Reset();						
			period = 100;
			Task_Active_Table[TASK_ExtenderR_Connection_Check_ActiveID].Task_Interval.w = period;		
		}	
		
		//This will caused the send out multi SOF token out, should fixed
		Task_Active_Table[TASK_ExtenderR_Connection_Check_ActiveID].Task_Interval.w = period;		
		//Receiver_Control_State &= ~TRANSMITTER_PLUG_CHECK_MASK; //start the plug check

		if (Receiver_Control_State & TRANSMITTER_CONNECTION_MASK)
		{
			if ((Receiver_Control_State_Bk & TRANSMITTER_CONNECTION_MASK) == 0x00)
			{
				Task_Active_Table[TASK_ExtenderR_Actived_Led_Flash_Active_ID].Task_Event &= ~TASK_EVENT_SKIP;
				USBDC_UpPortState[REMOTE_HOST_PORT] |= USBDC_ROOTHUB_ATTACHED_MASK;				
#if (KVM_BUZZER_SUPPORT)
				BUZZER_Script_Active(Extender_Plug_In);
#endif //#if (KVM_BUZZER_SUPPORT)
#ifdef PWAYTEK_01
				TRANSMITTER_STATE_LED_GPIO = LED_ON;				
#endif
			}

			//----------------------------------------------------------------------------
			//1.Some operation is happend when Transmitter is plug in, these operation has
			//(1).Receiver Restart(Power ON or Reset)
			//(2).Console monitor has been detected plug in or change
			//(3).Receiver Restart(Power ON or Reset)
			if ((Receiver_Control_State & RECEIVER_RESTART_MASK) == 0x00)
			{			
				Receiver_Restart_Control();
				Receiver_Control_State |= RECEIVER_RESTART_MASK;
			}				
						
			//(2).Monitor In or Re-plug in
#if (EXTENDER_EDID_SUPPORT)			
			if (EDID_ctrl.TaskState == CONSOLE_MONITOR_STATUS_CHK)
			{
				if ((Receiver_Control_State & TRANSMITTER_EDID_MASK) == 0x00)
				{					
					Externder_Receiver_EDID_Control();
					Receiver_Control_State |= TRANSMITTER_EDID_MASK;
				}
			}
#endif /* #if (EXTENDER_EDID_SUPPORT) */
		}
	}
	else
	{
		if ((Extender_Receive_Cmd_Parser_State & EXTENDER_CMD_TOKEN_LEN_SHORT) == 0)
		{			
			Extender_Receive_Parser_State_Reset();			
		}		
		return 0;
	}
	
	if (err_flag)
		return 0;
		
	return 1;
}

/*----------------------------------------------------------------------------
 * Function Name: Extender_Receiver_Transmit_TimeOut_Check
 * Purpose: Handle some speical command which need to handle after transmit
 *          time out event
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void Extender_Receiver_Transmit_TimeOut_Check(void)
{
	U8_T	*buf;
	U8_T	td_id;
	U8_T	token;
	U8_T	plug_off = 0;
	Extender_Data_Packet_Def *data_token;	

	Transmitter_Plug_Off_Cnt++;
	if (Transmitter_Plug_Off_Cnt >= TRANSMITTER_PLUG_OFF_CNT)
	{
		if (Receiver_Control_State & TRANSMITTER_CONNECTION_MASK)
		{
			Task_Active_Table[TASK_ExtenderR_Actived_Led_Flash_Active_ID].Task_Event |= TASK_EVENT_SKIP;
			USBDC_UpPortState[REMOTE_HOST_PORT] &= ~USBDC_ROOTHUB_ATTACHED_MASK;
			printf("=> Transmitter Plug Off\n\r");			
			ExtenderR_Audio_In_Cnt = 0;			
#ifdef DEVICE_KEEP
			Receiver_Control_State &= ~(TRANSMITTER_CONNECTION_MASK|TRANSMITTER_EDID_MASK|TRANSMITTER_RESTART_MASK);
#else			
			Receiver_Control_State &= ~(TRANSMITTER_CONNECTION_MASK|TRANSMITTER_EDID_MASK|RECEIVER_RESTART_MASK|TRANSMITTER_RESTART_MASK);
#endif			
			Extender_Receive_Data_Parser_State = 0;
			Extender_Receive_Cmd_Parser_State  = 0;
			Transmitter_Data_Toggle = TRANSMITTER_DATA_TOGGLE0;
			Receiver_Data_Toggle = RECEIVER_DATA_TOGGLE0;
			plug_off = 1;
			ExtenderR_Clear_IntrBuffer_Wait();
			ExtenderR_Clear_Intr_Send_OK();
			ExtenderR_Clear_Remote_Passthrough_Condition(); //Clear passthrough mode in remote_port device
			Remote_Mount_State = 0;			
#if (KVM_BUZZER_SUPPORT)
			BUZZER_Script_Active(Extender_Plug_Out);
#endif //#if (KVM_BUZZER_SUPPORT)

#if (SYSTEM_EXTENDER_MSC_SUPPORT)			
			if ((USBHC_MSC_Devinx != 0xff) && (KVM_CurrentMSC == REMOTE_HOST_PORT) && USBHC_MSC_Device_Cnt) 
			{	
				USBDC_Maintain_MSC_PDevice(KVM_CurrentMSC);
				Extender_MSC_Init(0);
			}	
#endif //#if (SYSTEM_EXTENDER_MSC_SUPPORT)

#ifdef PWAYTEK_01
			TRANSMITTER_STATE_LED_GPIO = LED_OFF;
#endif
		}
	}

	token = Trainsmit_FIFO[EXTENDER_Transmit_OutP].Buf[0];
	if (token == EXTENDER_SOF_TOKEN)
	{ 		
		//printf("[T]");	
		Receiver_Control_State &= ~TRANSMITTER_PLUG_CHECK_MASK; //start the check
		if (plug_off)
		{
			Task_Active_Table[TASK_ExtenderR_Connection_Check_ActiveID].Task_Interval.w = RECEIVER_CHECK_DELAY_TIME+TRANSMITTER_CHECK_TIME; //next go for 100ms check
		}
	}
	else if (token == EXTENDER_CMD_TOKEN)
	{		
		//Setup command has been send out
		buf = Trainsmit_FIFO[EXTENDER_Transmit_OutP].Buf+sizeof(Extender_Cmd_Token_Def);
		data_token = (Extender_Data_Packet_Def *)(buf);			
		//printf("\n\r\n\r [ERROR CMD TimeOut:bRequest(%bx)]\n\r\n\r",data_token->bRequest);		
		
		//Resend the Transcation .....
		Extender_Control_State &= ~EXTENDER_CONTROL_TRANSMIT;
		
		if (data_token->bRequest == EXTENDER_USB_INTR_IN)
		{
			td_id = (data_token->bIndex & 0xf0) >> 4;
			HC_IntTransfer_Table[td_id].PassControl.State &= ~PASSTHROUGH_WAIT; // clear the wait state
		}
		
		if (Receiver_Control_State & TRANSMITTER_CONNECTION_MASK)
		{	
			ExtenderR_Skip_TimeoutUpdate = 1;
		}	
	}

	//Clear the wait  state
//	EXTENDER_Multi_TR_State &= ~EXTENDER_SEND_WAIT; /* Setup token init */

#if (RECEIVER_CMD_DEBUG)
	printf(" (R):Transmit_Wait_TimeOut(%02x) !\n\r",(U16_T)Receiver_Control_State);
#endif /*(RECEIVER_CMD_DEBUG)*/
}

/*----------------------------------------------------------------------------
 * Function Name: Receiver_Set_Transmit_Header
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
U16_T Receiver_Set_Transmit_Header(U16_T maloc_len,U8_T *token_pattern)
{
	U16_T len;

	len = maloc_len+sizeof(Extender_Packet_Header_Def)+2;

	Receiver_Send_Buf = m_malloc(len,28); //256 edid table, setup token 3,data token,7

	if (Receiver_Send_Buf != NULL)
	{
		Receiver_Send_Data_TokenP = (Extender_Data_Packet_Def *) (Receiver_Send_Buf+sizeof(Extender_Cmd_Token_Def));
		if (token_pattern != NULL)
		{
			Externder_Packet_Header((Extender_Packet_Header_Def *) Receiver_Send_Buf,token_pattern);
		}
		return len;
	}
	return 0;
}

/*----------------------------------------------------------------------------
 * Function Name: Externder_Receiver_DeviceMount_Control 
 * Purpose: make the port connect status, in virtual hub port
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
U8_T Externder_Receiver_DeviceMount_Control(U8_T devinx,U8_T mounttype,U8_T brequst)
{
	U8_T	hubportnumber;
	U8_T	hubdevice;
	U8_T	pinformation;
	U16_T	len;
	U8_T    skip_mount=0;

	if (ExtenderR_Check_Transmitter_Plug_State() == 0)
	{
		if (USB_PDevice[devinx].DevClass != USB_HUB_CLASS)
		{
			if (mounttype == VDEV_UNMOUNT)
				edtr_R_DeviceMount[devinx] = 0;
			else
				edtr_R_DeviceMount[devinx] = 1;
		}
		return 0;
	}

	if (USB_PDevice[devinx].DevClass == USB_HUB_CLASS)
	{		
		return 1;
	}
	else
	{
		USBHC_Get_Upper_Device_Info(devinx,&hubdevice,&hubportnumber);
#if (RECEIVER_CMD_DEBUG)
		if (mounttype == VDEV_UNMOUNT)
		{
			printf("  R<-[UnMount");
		}
		else
		{
			printf("  R(%02bx)<-[Mount",Remote_Mount_State);
		}
		printf(" Devinx:%d,HubPort:%d]\n\r",(U16_T)devinx,(U16_T)hubportnumber);
#endif /* #if (RECEIVER_CMD_DEBUG) */
	}
/*	
#ifdef DEVICE_KEEP		
	printf("vpiid go\n\r");
	ExtenderR_Transmitter_Data_Send(NO_CTRL,EXTENDER_USB_VPID,devinx,mounttype,
									4,
									(U8_T *)&USB_PDevice[devinx].idVendor);		

#endif	
*/	
	

#if (SYSTEM_MSC_DEVICE_SUPPORT)
	if ((mounttype == VDEV_MOUNT) && (devinx == USBHC_MSC_Devinx) && USBHC_MSC_Reset_Flag)
	{
		len = 1;
		skip_mount = 1;		
		ExtenderR_Remote_MscResetCheck(devinx);
	}	
	else
#endif		
	{	 
		len = Receiver_Set_Transmit_Header(0,Recevier_Cmd_Token);
	}
		
	if (len)
	{			
		if (skip_mount == 0)
		{	
			pinformation = hubportnumber + (hubdevice << 4);
			Extender_Packet_Data(Receiver_Send_Data_TokenP,0,0,brequst,pinformation,devinx);
			//printf("Trnsmitter mountype:%02bx,devinx=%bu,len=%d\n\r",mounttype,devinx,len);
			Extender_Token_Transmit(Receiver_Send_Buf,0x00,len,1000,1);
		}
			
		if (mounttype == VDEV_UNMOUNT)
		{
#if (RECEIVER_CMD_DEBUG)
			printf(" (R)<-[Device(%bu) Unmount_Transmit]\n\r",devinx);
#endif	/* #if (SYSTEM_EXTENDER_RECEIVER) */
			edtr_R_DeviceMount[devinx] = 0;
		}
		else
		{
#if (RECEIVER_CMD_DEBUG)
			printf(" (R)<-[Device(%bu) Mount_Transmit]\n\r",devinx);
#endif	/* #if (SYSTEM_EXTENDER_RECEIVER) */
			if (skip_mount == 0)
			{	
				ExtenderR_Device_Descriptor_Send(devinx);
			}
			else
			{				
				Remote_Mount_State |= BIT_MASK[devinx];
			}		
			edtr_R_DeviceMount[devinx] = 1;
		}
	}
	return 1;
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Device_Descriptor_Send 
 * Purpose: make the port connect status, in virtual hub port
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Device_Descriptor_Send(U8_T devinx)
{
	U8_T	index;	 
	//U8_T	*dev_desp;
	//U16_T	devdsp_len;
	//U16_T	len;

#if (RECEIVER_CMD_DEBUG)
	printf(" (R)<-[Device_Descriptor_Transmit]..");
#endif /* #if (RECEIVER_CMD_DEBUG) */
	
	for (index=0; index <= HUB_DESC ; index++)
	{
		if (USB_PDevice[devinx].Desc[index].Len)
		{			
#if (RECEIVER_CMD_DEBUG)
			printf("[(%d)-%d Bytes].",(U16_T)index,USB_PDevice[devinx].Desc[index].Len);
#endif /* #if (RECEIVER_CMD_DEBUG) */					 				
			ExtenderR_Transmitter_Data_Send(NO_CTRL,EXTENDER_USB_DESCRIPTOR,devinx,index,
										    USB_PDevice[devinx].Desc[index].Len,
										    USB_PDevice[devinx].Desc[index].Ptr);
		}
	}

	/* Desp End */
	ExtenderR_Transmitter_Data_Send(NO_CTRL,EXTENDER_USB_DESCRIPTOR,devinx,EXTENDER_USB_DESCRIPTOR_END,
									0,
									NULL);		
	Remote_Mount_State |= BIT_MASK[devinx];										
	
#if (RECEIVER_CMD_DEBUG)
	printf(" Done\n\r");
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */
}

/*----------------------------------------------------------------------------
 * Function Name: Externder_Receiver_EDID_Control 
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void Externder_Receiver_EDID_Control(void)
{
	//U16_T	len;

	if (ExtenderR_Check_Transmitter_Plug_State() == 0)
	{
		return;
	}
	
	ExtenderR_Transmitter_Data_Send(NO_CTRL,EXTENDER_KVM_EDID,0,0,
										    EDID_MAX,
										    ConsoleEdidTable);
	
	//len = Receiver_Set_Transmit_Header(EDID_MAX,Recevier_Cmd_Token);
	//if (len)
	//{
#if (RECEIVER_CMD_DEBUG)
		printf(" (R)->EDID_Transmit\n\r");
#endif /*#if (RECEIVER_CMD_DEBUG) */
	//	Extender_Packet_Data(Receiver_Send_Data_TokenP,EDID_MAX,ConsoleEdidTable,EXTENDER_KVM_EDID,0,0);
	//	Extender_Token_Transmit(Receiver_Send_Buf,0x00,len,1000,1);
	//}
}

/*----------------------------------------------------------------------------
 * Function Name: Receiver_Restart_Control 
 * Purpose:  
 * Params : 
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void Receiver_Restart_Control(void)
{
	//U16_T	len;

	if (ExtenderR_Check_Transmitter_Plug_State() == 0)
		return;
	
	ExtenderR_Transmitter_Data_Send(NO_CTRL,EXTENDER_R_RESTART,NO_INDEX,NO_VALUE,NO_LEN,NO_DATA);
		
	//len = Receiver_Set_Transmit_Header(0,Recevier_Cmd_Token);
	//if (len)
	//{
#if (RECEIVER_CMD_DEBUG)
		printf(" (R)->Receiver Restart:%d\n\r",len);
#endif /* #if (RECEIVER_CMD_DEBUG) */
		//Extender_Packet_Data(Receiver_Send_Data_TokenP,0,NULL,EXTENDER_R_RESTART,0,0);
		//Extender_Token_Transmit(Receiver_Send_Buf,0x00,len,1000,1); //report Reciver has been reset, need transmter init itself
	//}
//#ifdef DEVICE_KEEP
//	ExtenderR_Transmitter_Data_Send(NO_CTRL,EXTENDER_T_DEVICE_KEEP,NO_INDEX,NO_VALUE,NO_LEN,NO_DATA);
//#endif	
}

/*----------------------------------------------------------------------------
 * Function Name: Externder_Receiver_EDID_PlugOff 
 * Purpose:  
 * Params : 
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void Externder_Receiver_EDID_PlugOff(void)
{
	Receiver_Control_State &= ~TRANSMITTER_EDID_MASK;
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Responsed_Received_Data_Parser 
 * Purpose:  
 * Params : 
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Responsed_Received_Data_Parser(void)
{
	U8_T devinx;
	U8_T curinf;
	U8_T value;
	U8_T buf[8];
	
ExtenderR_Responsed_Received_Data_Parser_Restart:
	if (Extender_Receive_Data_Token_Parser(&Extender_Setup))
	{
		switch(Extender_Setup.Data_Token.bRequest)
		{
			case EXTENDER_DATA_END:
//EXTENDER_FORCE_ACK:			
#if (RECEIVER_CMD_DEBUG)
				printf("  R__[ E N D ]\n\r");
#endif /* #if (RECEIVER_CMD_DEBUG) */
				EXTENDER_Multi_TR_State &= ~(EXTENDER_RECEIVE_WAIT|EXTENDER_RECEIVE_START); /* Multi Data Receive stage reinit */
				//Extender_Token_Transmit(Recevier_Ack_Token,(EXTENDER_CONTROL_TRANSMIT_NO_WAIT|EXTENDER_CONTROL_NONE_FREE),3,1000,1);
				Extender_Receive_Parser_State_Reset();
				break;
#ifdef DEVICE_KEEP
			case EXTENDER_T_DEVICE_TABLE: //ask transmitter to remount device			
				ExtenderT_Device_Mount_Change = Extender_Setup.Data_Token.bIndex;
				//printf("MountChange=%02bx\n\r",ExtenderT_Device_Mount_Change);
				ExtenderR_Device_Table_Check();
				ExtenderR_Remote_Device_Restart_Create();
				break;
#endif					
			case EXTENDER_T_RESTART: //Transmitter restart											
				//if ((Receiver_Control_State & TRANSMITTER_RESTART_MASK) == 0x00)
				//{
//#if (KVM_BUZZER_SUPPORT)					
//					BUZZER_Script_Active(Extender_Plug_In);
//#endif					
				//	printf("Transmitter has been report restarted\n\r");
				//}
				//else
				{		
					Transmitter_Restart_Flag = 1;					
					Transmitter_Data_Toggle = TRANSMITTER_DATA_TOGGLE0;
					Receiver_Data_Toggle = RECEIVER_DATA_TOGGLE0;
//#if (RECEIVER_CMD_DEBUG)
					printf("> Transmitter Restart\n\r");
//#endif /* #if (RECEIVER_CMD_DEBUG) */
					//Clear passthrough mode in remote_port device
					ExtenderR_Clear_Remote_Passthrough_Condition();
//					edtr_R_RcvTransmitterRestartFlag = 1;
					Receiver_Control_State |= TRANSMITTER_RESTART_MASK;					
#ifdef DEVICE_KEEP
					ExtenderT_Device_Mount_Change = 0; //default send no device device
					if (Extender_Setup.Data_Token.bValue) //Transmitter has been connected
					{	
						ExtenderR_Device_Keep_Flag = 1;
						//ExtenderR_Remote_Device_List_Contorl(0x01);		
						ExtenderR_Device_Table_Transfer();
						break;
					}				
					else
					{
						ExtenderT_Device_Mount_Change = 0xfe; //default send all device
					}						
#endif			

					Receiver_Control_State &= ~TRANSMITTER_EDID_MASK;	
					//Need to Report All device in HC to Transmitter
					ExtenderR_Remote_Device_Restart_Create();
					
					ExtenderR_Transmit_System_Setting();
					
					ExtenderR_Transmit_MSC_Size_Setting();
#ifdef RTC					
					ExtenderR_Transmit_RTC_Contorl();
#endif					
				}
				break;
			case EXTENDER_USB_SETUP_STAGE: //Extender Setup Token Passthrough
#if (RECEIVER_CMD_DEBUG)
				printf("  R__[SETUP_STAGE](%d])\n\r",(U16_T)Receive_Buffer_Cnt);
#endif /* #if (RECEIVER_CMD_DEBUG) */				
				ExtenderR_Passthrough_Setup_Token_Handle();
				break;
			case EXTENDER_USB_DATA_OUT_STAGE: //Extender Setup Data Passthrough
#if (RECEIVER_CMD_DEBUG)
				printf("  R__[DATA_OUT]\n\r");
#endif /* #if (RECEIVER_CMD_DEBUG) */				
				ExtenderR_Passthrough_Setup_Data_Out_Handle();				
				break;
			case EXTENDER_USB_DATA_IN_STAGE: //Extender Setup Status Passthrough
#if (RECEIVER_CMD_DEBUG)				
				printf("  R__[DATA_IN]\n\r");
#endif /* #if (RECEIVER_CMD_DEBUG) */
				ExtenderR_Passthrough_Setup_Data_In_Enable();
				break;
			case EXTENDER_USB_STATUS_OUT:
#if (RECEIVER_CMD_DEBUG)
				printf("  R__[STATUS_OUT]\n\r");
#endif /* #if (RECEIVER_CMD_DEBUG) */
				ExtenderR_Passthrough_Setup_Status_Out_Handle();
				break;
			case EXTENDER_KVM_KB_LED:
#if (RECEIVER_CMD_DEBUG)
				printf("  R__[KB_LED](0x%02bx)\n\r",Extender_Setup.Data_Token.bIndex);
#endif /* #if (RECEIVER_CMD_DEBUG) */
				ExtenderR_Passthrough_KVM_KB_Led_Handle();
				break;
			case EXTENDER_USB_SET_PROTOCOL:
#if (RECEIVER_CMD_DEBUG) 
				printf("  R__[SET_PROTOCOL](D:0x%02bx)[%02bx]\n\r",Extender_Setup.Data_Token.bIndex,Extender_Setup.Data_Token.bValue);
#endif /* #if (RECEIVER_CMD_DEBUG) */
				devinx = Extender_Setup.Data_Token.bIndex;
				curinf = (Extender_Setup.Data_Token.bValue & 0xf0) >> 4;
				value  = Extender_Setup.Data_Token.bValue & 0x0f;
				USBHC_Set_Dev_Inf_Protocol(devinx,curinf,value,1);
				USBDC_Device[devinx].Current_Protocol[REMOTE_HOST_PORT] = 0; //Clear to 0 for next HOST reset
				break;
			case EXTENDER_USB_INTR_OUT:				
				ExtenderR_Passthrough_Intr_Out_Handle();
				break;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
			case EXTENDER_USB_ISO_OUT:			
				ExtenderR_Passthrough_Audio_Data_Out_Handle();
				break;
#endif

#if (SYSTEM_EXTENDER_MSC_SUPPORT)
			case EXTENDER_USB_MSC_CBW:
			case EXTENDER_USB_MSC_DATA_OUT:
#if (SYSTEM_USB_HC_BURST)				 
				ExtenderR_PassthroughMscBurstOutHandle();				
#else				
				ExtenderR_PassthroughMscBulkOutHandle();
#endif //#if (SYSTEM_USB_HC_BURST)				
				break;
			case EXTENDER_USB_MSC_IN_WAIT:		
				ExtenderR_MscInWaitHandle(Extender_Setup.Data_Token.bValue);		
				break;	
			case EXTENDER_USB_MSC_RESET:				
				ExtenderR_MscResetHandle();
				break;
#endif //#if (SYSTEM_EXTENDER_MSC_SUPPORT)
			case EXTENDER_T_LED_CONTROL:
				//edtr_RPlugOffFlag = 0;				
				ExtenderR_PassthroughHostLedControlHandle(Extender_Setup.Data_Token.bValue);
				break;			
			case EXTENDER_USB_COMMAND:												
				Extender_CopyDataFromRxBuffer(Extender_Setup.iRecBuf_Index,Extender_Setup.Data_Token.iLen,buf);
				ExtenderR_USB_Command_Handle(Extender_Setup.Data_Token.bIndex,
											 Extender_Setup.Data_Token.bValue,  //USB command
											 buf);
				break;
			case EXTENDER_USB_INTERVAL:
				//printf("R interval:index=%bx,value=%bu\n\t",Extender_Setup.Data_Token.bIndex,Extender_Setup.Data_Token.bValue);
				#if (HUART_BAUD < HUART_115K)					
				ExtenderR_USB_Interval_Handle(Extender_Setup.Data_Token.bIndex,Extender_Setup.Data_Token.bValue);
				#endif
				break;				
			default:
				printf("[!!UNKNOW!!]\n\r");
				Extender_Receive_Parser_State_Reset();
				break;
		}

		if (EXTENDER_Multi_TR_State & EXTENDER_RECEIVE_WAIT)
		{		
			if (Receive_Buffer_Cnt >= sizeof(Extender_Data_Packet_Def)+2)
			{				
				goto ExtenderR_Responsed_Received_Data_Parser_Restart;
			}
			else
			{
//				printf("RC:%x\n\r", Receive_Buffer_Cnt);
				Extender_Receive_Data_Parser_State |= EXTENDER_DATA_PACKET_LEN_SHORT;
			}
		}
	}
	else
	{		 
		if (Extender_Receive_Data_Parser_State & EXTENDER_DATA_PACKET_PARSER_ERR)
		{
			//printf("error(%02bx)\n\r",Extender_Receive_Data_Parser_State);
			if ((Extender_Receive_Data_Parser_State & EXTENDER_DATA_PACKET_CRC_OK) == 0x00)
			{
				//printf("Data Parser Error,Termimate the processing\n\r");
				Extender_Receive_Parser_State_Reset();
			}
		}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: TASK_Receiver_RemoteLed_Control 
 * Purpose:  
 * Params : 
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void TASK_Receiver_RemoteLed_Control(void)
{	
	//if (KVM_CurrentHost == REMOTE_HOST_PORT) //Remote site
	{
		if ((Receiver_Control_State & TRANSMITTER_CONNECTION_MASK) == 0x00)
		{
			if (Remote_Link_LED_State)
			{				
#ifndef PCT_MUA22
				KVM_Host_Led_Control(REMOTE_HOST_PORT, LED_ON);
#endif				
				//KVM_Select_Led_Control(0,LED_ON);
				Remote_Link_LED_State = 0;
			}
			else
			{
#ifndef PCT_MUA22							
				KVM_Host_Led_Control(REMOTE_HOST_PORT, LED_OFF);
#endif								
				//KVM_Select_Led_Control(0,LED_OFF);
				Remote_Link_LED_State = 1;
			}
			
#ifdef PCT_MUA22							
			KVM_Host_Led_Control(REMOTE_HOST_PORT, LED_OFF);
#endif											
		}
		else
		{
#ifdef PCT_MUA22							
			KVM_Host_Led_Control(REMOTE_HOST_PORT, LED_ON);
#endif											
		}	
		
	}
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Passthrough_Setup_Token_Handle(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Passthrough_Setup_Token_Handle(void)
{
	U8_T	devinx;
	U8_T	direction;
	
	devinx = Extender_Setup.Data_Token.bIndex;
	
	//---------------------------------------------------------
	//1.Check passthrough last condition, did it complete?
	if (USB_PDevice[devinx].Hc.PassThrough_State)
	{
		//printf("Last Passthroug not complete:%x\n\r",(U16_T)USB_PDevice[devinx].Hc.PassThrough_State);		
		ExtenderR_PassThrough_Setup_Condition_Reset(devinx);
	}

	//---------------------------------------------------------
	//2.Copy setup token to output buffer
	Extender_CopyRxDmaToBuf(Extender_Setup.iRecBuf_Index,8,(U8_T *)&USBDC_Device[devinx].Setup[REMOTE_HOST_PORT]);
	Receiver_Passthrough_State[devinx] = RECEIVER_PASSTHROUGH_SETUP_MASK; //indicate the setup passthrough is start

	//---------------------------------------------------------
	//3.Check the direction
	direction = (USBDC_Device[devinx].Setup[REMOTE_HOST_PORT].b.bmRequestType & USB_REQ_DIR_MASK); /* data direction */
	if (direction == USB_D2H) /*Device to Host,Data Direction is IN*/
	{
		Receiver_Passthrough_State[devinx] |= RECEIVER_PASSTHROUGH_IN_MASK;
	}
	else
	{
		Receiver_Passthrough_State[devinx] |= RECEIVER_PASSTHROUGH_OUT_MASK;
	}
//#if 0
	/* Handle the SET_REPORT */
/*	
	Request_No = USBDC_Device[devinx].Setup[REMOTE_HOST_PORT].b.bRequest;
	Type_Recipient = USBDC_Device[devinx].Setup[REMOTE_HOST_PORT].b.bmRequestType & (REQUEST_TYPE | RECIPIENT);
	if ((Type_Recipient & REQUEST_TYPE) == CLASS_REQUEST)
	{
		if (Request_No == SET_REPORT)
		{
			USBDC_Class_SetReport(devinx,REMOTE_HOST_PORT);
		}
	}
*/	
//#else
	/* Handle the Setup Token process */
	//USBDC_VIRTUAL_Setup_Process(devinx, REMOTE_HOST_PORT);
//#endif
	//---------------------------------------------------------
	//4.Fork the PassThrough process task queue
	//Disp_Str((U8_T *)&USBDC_Device[devinx].Setup[REMOTE_HOST_PORT],8);		
	if (USB_PDevice[devinx].Hc.PassThrough_State == PASSTHROUGH_IDLE)
	{
		memcpy((U8_T *)&USB_PDevice[devinx].Hc.PControl.Setup,(U8_T *)&USBDC_Device[devinx].Setup[REMOTE_HOST_PORT],8);
		USBDC_Device[devinx].PassThroughState[REMOTE_HOST_PORT] = PASSTHROUGH_SETUP_START; /* Indicate the device is in PassThrough Mode */
 		TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_PassThrough_Handle_Start_ID,0x00,devinx,REMOTE_HOST_PORT,0);  // Generate the task, for next check
	}
	else
	{
		TASK_Active(TASK_TYPE_EVENT,TASK_USBDC_VIRTUAL_PassThrough_Setup_Wait_ID,0x00,devinx,REMOTE_HOST_PORT,0);
	}		
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Passthrough_Setup_Data_Out_Handle(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Passthrough_Setup_Data_Out_Handle(void)
{
	U8_T	devinx;
	U16_T	len;	
 
	len = Extender_Setup.Data_Token.iLen;
	
	if (len==0)
	{
		//printf("   !! DATA out is length 0\n\r");
		return;	
	}	
		
	devinx = Extender_Setup.Data_Token.bIndex;
	
	if (USB_PDevice[devinx].Hc.Control.Buf == NULL)
	{
		//printf("Devinx=%bu,SKip OUT\n\r",devinx);
		//Dump_Receiver_Buff();		
		//ExtednerR_Skip_PassthroughFlag = 1;
		//USB_PDevice[devinx].Hc.Control.Buf = Extender_Passthrough_Buf;
		return;
		
	}	
	//len = Extender_Setup.Data_Token.iLen;
	Extender_CopyDataFromRxBuffer(Extender_Setup.iRecBuf_Index,len,USB_PDevice[devinx].Hc.Control.Buf);
	USB_PDevice[devinx].Hc.PControl.Total_Length = len;
	USB_PDevice[devinx].Hc.PControl.Current_Length = 0; //reset current length
	USB_PDevice[devinx].Hc.PControl.Data_Length = len;
	if (len > USB_HC_ATL_BUF_SIZE)
		USB_PDevice[devinx].Hc.PControl.Data_Length = USB_HC_ATL_BUF_SIZE;

	/* Handle the SETUP token command process */
	ExtenderR_Passthrough_SetupOutProcess(REMOTE_HOST_PORT, devinx);

	Receiver_Passthrough_State[devinx] |= RECEIVER_PASSTHROUGH_OUT_PAGE_MASK; //out data page in
	USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_OUT;
	TASK_Active(TASK_TYPE_USB,TASK_USBHC_Handle_Control_ID,USB_PDevice[devinx].Hc.Control.TdNum,devinx,0,0);
}

/*******************************************************************************
* Function Name  : ExtenderR_Passthrough_SetupOutProcess
* Description    : Process the OUT token on all default endpoint.
* Input          : None.
* Output         : None.
* Return         : Post0_Process.
*******************************************************************************/
void ExtenderR_Passthrough_SetupOutProcess(U8_T port, U8_T devinx)
{
	U8_T	*pDataBuf;
	U16_T	Length;
	
	if ((USBDC_Device[devinx].ControlState[port] == IN_DATA) || (USBDC_Device[devinx].ControlState[port] == LAST_IN_DATA))
	{
		USBDC_Device[devinx].ControlState[port] = SETUP_COMPLETE; 
	}
	else if ((USBDC_Device[devinx].ControlState[port] == OUT_DATA) || (USBDC_Device[devinx].ControlState[port] == LAST_OUT_DATA))
	{
//		USBDC_VIRTUAL_Setup_DataStageOut(devinx,port,1);
		Length = USB_PDevice[devinx].Hc.PControl.Data_Length; // out data length
		if (Length == 0) 
		{
			if (USBDC_Device[devinx].PassThroughState[port])
			{
				/*PASSTHROUGH*/
				USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_HC_SETUP_DATA_OUT;
			}
		}
		else
		{
			if ((USBDC_Device[devinx].ControlState[port] != OUT_DATA) && (USBDC_Device[devinx].ControlState[port] != LAST_OUT_DATA))
			{
				return;
			}
			
			pDataBuf = USB_PDevice[devinx].Hc.Control.Buf;	
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			//Logitech Unifying wireless mouse handle
			if ((USB_PDevice[devinx].DevAttr & DEVATTR_UNIFYING_MASK) &&
				(USBDC_Device[devinx].DevAttr[port] & UNIFYING_RPT_CHECK_MASK))
			{
				USBDC_Unifying_SetReport_Data_Check(port, devinx, pDataBuf);
			}
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			
			if (USBDC_Device[devinx].ControlState[port] == OUT_DATA)
			{
				if (USBDC_Device[devinx].Ctrl_TotalByte[port] < (USBDC_Device[devinx].Ctrl_CurrentByte[port] + Length)) // this over flow
				{
					return;
				}
				else
				{
					USBDC_Device[devinx].Ctrl_CurrentByte[port] += Length;
					Length = USBDC_Device[devinx].Ctrl_TotalByte[port]-USBDC_Device[devinx].Ctrl_CurrentByte[port];
					/*PASSTHROUGH*/
					if (USBDC_Device[devinx].PassThroughState[port] == PASSTHROUGH_IDLE) 
					{
						USBDC_REGS_Endp_ControlClear(port,devinx,0,DA_CR_BCLR_SET); // Clear the OUT buffre flag
					}
					else
					{
						/*PASSTHROUGH*/
					}
					if (Length <= USBDC_Device[devinx].EndpMaxSize[0])
					{
						USBDC_Device[devinx].ControlState[port] = LAST_OUT_DATA;
					}
				}
			}
			else if (USBDC_Device[devinx].ControlState[port] == LAST_OUT_DATA)// should put STALL
			{
				// The Last package
				USBDC_Device[devinx].ControlState[port] = WAIT_STATUS_IN;
				if (USBDC_Device[devinx].PassThroughState[port] == PASSTHROUGH_IDLE) 
				{
					if (devinx != USBDC_VHUB_DEVINX )
					{
						USBDC_VIRTUAL_Setup_DataStageOut_Handle(devinx,port);
					}
				}
				else
				{
					/*PASSTHROUGH*/
				}
			}
			else
			{
				//printf("STALL\n\r");
				USBDC_Device[devinx].ControlState[port] = STALLED; // stall the quest
			}
		}
	}
	else if (USBDC_Device[devinx].ControlState[port] == WAIT_STATUS_OUT)
	{
		USBDC_Device[devinx].ControlState[port] = SETUP_COMPLETE;
	}
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Passthrough_Intr_Out_Handle(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Passthrough_Intr_Out_Handle(void)
{
	U8_T	devinx = Extender_Setup.Data_Token.bIndex;
	U8_T	endpinx = Extender_Setup.Data_Token.bValue;
	U8_T	len = (U8_T)Extender_Setup.Data_Token.iLen;
	U8_T	tdOffset = USBDC_Device[devinx].EndpType[endpinx] & 0x0f;
	U8_T*	pBuf = HCTD_Table.INTL[tdOffset].Buf;
	USBHC_TD_Header_Typedef*	pTdHeader = &(HCTD_Table.INTL[tdOffset].TD);

#if (SYSTEM_EA_CONTROL)
	etdrR_isr = EA;
	EA = 0;
#endif
	Extender_CopyDataFromRxBuffer(Extender_Setup.iRecBuf_Index, len, pBuf);
#if (SYSTEM_EA_CONTROL)	
	EA = etdrR_isr;
#endif	

	//pTdHeader->Byte0_Actual_Byte = 0;
	//pTdHeader->TD.Byte2_MaxPKT_Size = len;
	//pTdHeader->Byte3 &= 0xFC;
	pTdHeader->Byte4_Total_Byte = len;
	pTdHeader->Byte5 &= 0xFC;
	pTdHeader->Byte1 |= TD_ACTIVED_MASK;

	//USBHC_CORE_Clear_TD_DoneMap(USBHC_TD_INTL, tdOffset);
	USBHC_CORE_Set_TD_SkipMap(USBHC_TD_INTL,tdOffset,0x00);
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Passthrough_KVM_KB_Led_Handle(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Passthrough_KVM_KB_Led_Handle(void)
{
	U8_T	led;

	led = Extender_Setup.Data_Token.bIndex;	
	if (KVM_HostLed[REMOTE_HOST_PORT] != led)
	{
		KVM_HostLed[REMOTE_HOST_PORT] = led;
		if (KVM_CurrentHost == REMOTE_HOST_PORT)
		{			 
			KVM_CONSOLE_Keyboard_Led_Control(KVM_HostLed[REMOTE_HOST_PORT]);
		}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Passthrough_Setup_Status_Out_Handle(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Passthrough_Setup_Status_Out_Handle(void)
{
	U8_T	devinx;

	if (USB_PDevice[devinx].Hc.Control.State != CTRL_IDLE)
	{
		if (USB_PDevice[devinx].Hc.Control.State != CTRL_STATUS_OUT_WAIT)
		{
			devinx = Extender_Setup.Data_Token.bIndex;
			USB_PDevice[devinx].Hc.Control.State = CTRL_STATUS_OUT;
			TASK_Active(TASK_TYPE_USB,TASK_USBHC_Handle_Control_ID,USB_PDevice[devinx].Hc.Control.TdNum,devinx,0,0);
		}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Passthrough_Setup_Data_In_Enable(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Passthrough_Setup_Data_In_Enable(void)
{
	U8_T devinx;

	devinx = Extender_Setup.Data_Token.bIndex;
	if (Receiver_Passthrough_State[devinx] & RECEIVER_PASSTHROUGH_IN_MASK)
	{
		Receiver_Passthrough_State[devinx] |= RECEIVER_PASSTHROUGH_IN_SEND_MASK;
	}
	else
	{
		if (Receiver_Passthrough_State[devinx] == RECEIVER_PASSTHROUGH_IDLE)
		{
			//No data, just send 0 length data page out
			devinx = Extender_Setup.Data_Token.bIndex;
			USB_PDevice[devinx].Hc.PControl.Total_Length = 0;
			ExtenderR_Setup_Data_In(devinx);
		}
		//else
		//{
		//	printf("Remote Passthrough state error,state=%02x\n\r",(U16_T)Receiver_Passthrough_State[devinx]);
		//}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: void TASK_USBHC_ExtenderR_PassThrough(void)
 * Purpose: USB Host core passthrough control main state machine process
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void TASK_USBHC_ExtenderR_PassThrough(void)
{
	U8_T	devinx, td_id;
	RESULT	Result = USB_BUSY;
	U16_T	remain_len;

	devinx = TASK_Register0;
	td_id = USB_PDevice[devinx].Hc.Control.TdNum; // which host channel is used by device
	
	if (TASK_Type == TASK_TYPE_INTERVAL_MS)
    {
    	TASK_Destory_Current();
	}
	
	//Skip task if the device is gone or remote is gone
	if (((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00) ||// Device is gone
		(ExtednerR_Skip_PassthroughFlag))
	{
		if (USB_PDevice[devinx].Hc.PassThrough_State)
			ExtenderR_PassThrough_Setup_Condition_Reset(devinx);
		return;
	}

#if (USB_PASSTHROUGH_DEBUG_MODE)
	printf(">> TASK_EXTENDERT_PassThrough(%d)-(%d):\n\r",(U16_T)devinx,(U16_T)USB_PDevice[devinx].Hc.PassThrough_State);
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
TASK_USBHC_ExtenderR_PassThrough_Restart:
	switch (USB_PDevice[devinx].Hc.PassThrough_State)
	{
		case PASSTHROUGH_SETUP_START: //Send A control setup to HC
#if (RECEIVER_USB_PASSTHROUGH_DEBUG_MODE)
			printf("   (1).ExtenderR_SETUP Stage\n\r");			
#endif //#if (RECEIVER_USB_PASSTHROUGH_DEBUG_MODE)
			/* convert back to normal USB order */
			/* Task Fork => Send out Setup Token */
			Result = USBHC_PassThrough_Send_Setup_Token(devinx,td_id);
			/* For No Data Stage */
			if (Result == USB_SUCCESS) // need to ACK DC
			{
				/* Check the data length */
				USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_DC_SETUP_STATUS_IN;
				goto TASK_USBHC_ExtenderR_PassThrough_Restart;
			}
			else if (Result == USB_UNSUPPORT)
			{
				USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_DC_SETUP_STALL;
				goto TASK_USBHC_ExtenderR_PassThrough_Restart;
			}
			break;
		case PASSTHROUGH_SETUP_START_DONE: //After passthrough Setup token send complete
#if (RECEIVER_USB_PASSTHROUGH_DEBUG_MODE)
			printf("SETUP Token Done\n\r");			
#endif //#if (RECEIVER_USB_PASSTHROUGH_DEBUG_MODE)
			//wait for first page data in		
			USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_HC_SETUP_DATA_OUT;
			ExtenderR_Setup_Data_Out_Enable(devinx);
			break;
		case PASSTHROUGH_DC_SETUP_DATA_IN: //Setup Data Stage
			//update length first
			//If not allow to send data, then need to wait
			//Check the transmiter is ready to receive page data?
			if ((Receiver_Passthrough_State[devinx] & RECEIVER_PASSTHROUGH_IN_SEND_MASK) == 0x00)
			{
				TASK_Wait_Current(); //wait for next
				return;
			}
			USB_PDevice[devinx].Hc.PControl.Current_Length += USB_PDevice[devinx].Hc.PControl.Data_Length;

#if (RECEIVER_USB_PASSTHROUGH_DEBUG_MODE)
			printf("(R)SETUP_DC_DATA_IN(d-%d, c-%d, t-%d)\n\r",(U16_T)USB_PDevice[devinx].Hc.PControl.Data_Length,
			(U16_T)USB_PDevice[devinx].Hc.PControl.Current_Length, (U16_T)USB_PDevice[devinx].Hc.PControl.Total_Length);
#endif //#if (RECEIVER_USB_PASSTHROUGH_DEBUG_MODE)
			if (USB_PDevice[devinx].Hc.PControl.Data_Length == 0)
			{
				if (USB_PDevice[devinx].Hc.PControl.Current_Length)
				{
					if(USB_PDevice[devinx].Hc.Control.State == CTRL_STATUS_OUT)
					{
						// Do nothing and go for the next CTRL_STATUS_OUT checking
					}
					else
					{
						USB_PDevice[devinx].Hc.PControl.Total_Length = USB_PDevice[devinx].Hc.PControl.Current_Length;
						ExtenderR_Setup_Data_In(devinx);
						Receiver_Passthrough_State[devinx] &= ~RECEIVER_PASSTHROUGH_IN_SEND_MASK;
						USB_PDevice[devinx].Hc.PControl.Current_Length = 0;
						USB_PDevice[devinx].Hc.PControl.Total_Length   = 0;
						HCTD_Channel_Table[td_id].Total_Length = HCTD_Channel_Table[td_id].Current_Length; // not complete
						TASK_Wait_Current(); //wait for next data page in, then send out last page
						return;
					}
				}
			}
			//hanlde the actual byte is small than total byte,and can not modify by 8
			if(USB_PDevice[devinx].Hc.Control.State == CTRL_STATUS_OUT)
			{
				USB_PDevice[devinx].Hc.PControl.Total_Length = USB_PDevice[devinx].Hc.PControl.Current_Length;
				HCTD_Channel_Table[td_id].Total_Length = HCTD_Channel_Table[td_id].Current_Length; // not complete
			}

			if ((USB_PDevice[devinx].Hc.PControl.Current_Length == USB_PDevice[devinx].Hc.PControl.Total_Length)) //page full
			{	//if page full,send back to Remote Transmitter
				ExtenderR_Setup_Data_In(devinx);
				USB_PDevice[devinx].Hc.PControl.Current_Length = 0;
				if (HCTD_Channel_Table[td_id].Total_Length != HCTD_Channel_Table[td_id].Current_Length) // not complete
				{
					USB_PDevice[devinx].Hc.PControl.Total_Length = HCTD_Channel_Table[td_id].Total_Length - HCTD_Channel_Table[td_id].Current_Length;
					if (USB_PDevice[devinx].Hc.PControl.Total_Length > EXTENDER_PAGE_SIZ)
					{
						USB_PDevice[devinx].Hc.PControl.Total_Length = EXTENDER_PAGE_SIZ; //page length
					}
					USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_HC_SETUP_DATA_IN; // next wait for HC data in
					break;
				}
			}
			TASK_Active(TASK_TYPE_USB,TASK_USBHC_Handle_Control_ID,td_id,devinx,0,0); // perform the ctrl command
			//after send out complete, then retriggle the data in operation
			break;
		case PASSTHROUGH_DC_SETUP_STATUS_IN: //DC side Setup Status Stage
#if (RECEIVER_USB_PASSTHROUGH_DEBUG_MODE)
			printf(" @P@ DC Status IN R\n\r");
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
			ExtenderR_PassThrough_Setup_End(devinx);
#if (RECEIVER_USB_PASSTHROUGH_DEBUG_MODE)
			printf("@P_IN@ PassThrough Complete R\n\r");
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
				TASK_Active(TASK_TYPE_USB,TASK_USBHC_Handle_Control_ID,USB_PDevice[devinx].Hc.Control.TdNum,devinx,0,0);  // Generate the Setup Send command
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
					ExtenderR_Setup_Data_Out_Enable(devinx);
					break;
				}
				goto TASK_USBHC_ExtenderR_PassThrough_Restart;
			}
			break;
		case PASSTHROUGH_DC_SETUP_STALL: //DC side Setup Status Stage
			Result = USB_SUCCESS;
			ExtenderR_Setup_Stall(devinx);
			break;
		case PASSTHROUGH_COMPLETE: //PassThroug Complete
#if (RECEIVER_USB_PASSTHROUGH_DEBUG_MODE)
			printf("@P@ PassThrough Complete \n\r");
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
			//Check the total length
			Result = USB_SUCCESS;
			ExtenderR_PassThrough_Setup_End(devinx);
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
#if (SYSTEM_USB_HC_BURST)
						USBHC_MSC_CSW_After_Control(devinx);
#endif //#if (SYSTEM_USB_HC_BURST)
					}
				}
			}
		}
#endif //#if (SYSTEM_MSC_DEVICE_SUPPORT)
#if (USB_PASSTHROUGH_DEBUG_MODE)
		printf("@@ Done=>TASK_USBHC_PassThrough Complete\n\r");
		printf("\n\r@P@ TD_Free:%d\n\r\n\r",(U16_T)USB_PDevice[devinx].Hc.Control.TdNum);
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
		USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
		USBHC_CORE_Free_TD(USB_PDevice[devinx].Hc.Control.TdNum);
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
 * Function Name: void ExtenderR_Setup_Data_In(U8_T devinx)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Setup_Data_In(U8_T devinx)
{
	U16_T	len;
	U8_T	*buf;

	len = USB_PDevice[devinx].Hc.PControl.Total_Length+sizeof(Extender_Packet_Header_Def)+2;
	buf = Data_Transmit_Buf+sizeof(Extender_Cmd_Token_Def);
	//if (len)
	{
#if (RECEIVER_CMD_DEBUG)
		printf(" (R)->[Setup Data IN Transmit(%d-%d)]\n\r",len,USB_PDevice[devinx].Hc.PControl.Total_Length);
#endif /*#if (RECEIVER_CMD_DEBUG)*/
		Externder_Packet_Header((Extender_Packet_Header_Def *) Data_Transmit_Buf,Recevier_Cmd_Token);
		Extender_Packet_Data((Extender_Data_Packet_Def *)buf,USB_PDevice[devinx].Hc.PControl.Total_Length,NULL,EXTENDER_USB_DATA_IN_STAGE,devinx,0);
		Extender_Token_Transmit(Data_Transmit_Buf,EXTENDER_CONTROL_NONE_FREE,len,1000,1);
	}
}

/*----------------------------------------------------------------------------
 * Function Name: void ExtenderR_Setup_Data_Out_Enable(U8_T devinx)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Setup_Data_Out_Enable(U8_T devinx)
{
	//U16_T	len;
	
	ExtenderR_Transmitter_Data_Send(EXTENDER_CONTROL_TRANSMIT_NO_WAIT,EXTENDER_USB_DATA_OUT_STAGE,
									devinx,
									NO_VALUE,
									NO_LEN,
									NO_DATA);

	//len = Receiver_Set_Transmit_Header(0,Recevier_Cmd_Token);
	//if (len)
	//{
#if (RECEIVER_CMD_DEBUG)
		printf(" (R)->[Setup_Data_Out_Enable Transmit(%d)]\n\r",len);
#endif /*#if (RECEIVER_CMD_DEBUG)*/
	//	Extender_Packet_Data(Receiver_Send_Data_TokenP,0,NULL,EXTENDER_USB_DATA_OUT_STAGE,devinx,0);
	//	Extender_Token_Transmit(Receiver_Send_Buf,EXTENDER_CONTROL_TRANSMIT_NO_WAIT,len,1000,1);
	//}
}

/*----------------------------------------------------------------------------
 * Function Name: void ExtenderR_PassThrough_Setup_End(U8_T devinx)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_PassThrough_Setup_End(U8_T devinx)
{
	//U16_T	len;

	ExtenderR_Transmitter_Data_Send(EXTENDER_CONTROL_TRANSMIT_NO_WAIT,
									EXTENDER_USB_SETUP_DONE,
									devinx,
									NO_VALUE,
									NO_LEN,
									NO_DATA);
	//len = Receiver_Set_Transmit_Header(0,Recevier_Cmd_Token);
	//if (len)
	//{
#if (RECEIVER_CMD_DEBUG)
		printf(" (R)->[Setup_Done_Transmit(%d)]\n\r",len);
#endif /*#if (RECEIVER_CMD_DEBUG)*/
	//	Extender_Packet_Data(Receiver_Send_Data_TokenP,0,NULL,EXTENDER_USB_SETUP_DONE,devinx,0);
	//	Extender_Token_Transmit(Receiver_Send_Buf,EXTENDER_CONTROL_TRANSMIT_NO_WAIT,len,1000,1);
	//}	

	ExtenderR_PassThrough_Setup_Condition_Reset(devinx);
}

/*----------------------------------------------------------------------------
 * Function Name: void ExtenderR_PassThrough_Setup_Condition_Reset(U8_T devinx)
 * Purpose: Reset all the passthrough coditioin
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_PassThrough_Setup_Condition_Reset(U8_T devinx)
{
	/* Free all buffer & flag */
	USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
	USB_PDevice[devinx].Hc.gState = USB_PDevice[devinx].Hc.PassThrough_gState_bk;
	USBDC_Device[devinx].PassThroughState[REMOTE_HOST_PORT] = PASSTHROUGH_IDLE;
	malloc_free(USB_PDevice[devinx].Hc.Control.Buf);
	USB_PDevice[devinx].Hc.Control.Buf = 0;
	USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_IDLE;
}

/*----------------------------------------------------------------------------
 * Function Name: void ExtenderR_Setup_Stall(U8_T devinx)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Setup_Stall(U8_T devinx)
{
	//U16_T	len;
	
	ExtenderR_Transmitter_Data_Send(NO_CTRL,
									EXTENDER_USB_STATUS_STALL,
									devinx,
									NO_VALUE,
									NO_LEN,
									NO_DATA);
	
	//len = Receiver_Set_Transmit_Header(0,Recevier_Cmd_Token);
	//if (len)
	//{
	//	Extender_Packet_Data(Receiver_Send_Data_TokenP,0,NULL,EXTENDER_USB_STATUS_STALL,devinx,0);
	//	Extender_Token_Transmit(Receiver_Send_Buf,0x00,len,2000,1);
	//}
	Receiver_Passthrough_State[devinx] = 0;
	malloc_free(USB_PDevice[devinx].Hc.Control.Buf); // not free this time, should wait for send out complete
	ExtenderR_PassThrough_Setup_Condition_Reset(devinx);
}

#if (EXTENDER_LED_FLASH_INDICATE)	
/*----------------------------------------------------------------------------
 * Function Name: void TASK_ExtenderR_Actived_Led_Flash(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void TASK_ExtenderR_Actived_Led_Flash(void)
{
	if (Remote_Link_LED_Flash_Flag)
	{
		if (Receiver_Control_State & TRANSMITTER_CONNECTION_MASK)
		{
			Remote_Link_LED_Flash_Flag = 0;
			if (Remote_Link_LED_Flash_State==0)
			{
				Remote_Link_LED_Flash_State = 1;
				//KVM_Select_Led_Control(0,LED_OFF);
#if defined(PWAYTEK_01) || defined(PCT_MUA22)
				DATA_TRANSFER_LED_GPIO = LED_OFF;
#else				
				KVM_Host_Led_Control(REMOTE_HOST_PORT, LED_OFF);
#endif				
			}
			else
			{
				Remote_Link_LED_Flash_State = 0;
				//KVM_Select_Led_Control(0,LED_ON);
#if defined(PWAYTEK_01) || defined(PCT_MUA22)
				DATA_TRANSFER_LED_GPIO = LED_ON;
#else								
				KVM_Host_Led_Control(REMOTE_HOST_PORT, LED_ON);
#endif				
			}
		}
	}
	else
	{
#if defined(PWAYTEK_01) || defined(PCT_MUA22)
	   DATA_TRANSFER_LED_GPIO = LED_OFF;
#endif		
	}	
}
#endif

/*----------------------------------------------------------------------------
 * void ExtenderR_Device_ReCreate_Sub(U8_T devinx)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void ExtenderR_Device_ReCreate_Sub(U8_T devinx)
{
	if (devinx >= USBDC_VIRTUAL_DEVINX)
	{
		if ((edtr_R_DeviceMount[devinx] == 1) || (USB_PDevice[devinx].DevClass == USB_HUB_CLASS))
		{
			if ((USB_PDevice[devinx].DevClass == USB_MSC_CLASS) && (USBHC_MSC_DismountFlag))
			{						
				//printf("Device:%bu, MSC Skip Mount\n\r",devinx);
			}
			else
			{	
				if (USB_PDevice[devinx].DevClass != USB_HUB_CLASS)
				{
#ifdef DEVICE_KEEP
					if ((ExtenderT_Device_Mount_Change & BIT_MASK[devinx]) == 0x00) //if no need to report to transmiiter
					{
						ExtenderT_Device_Mount_Change &= ~BIT_MASK[devinx];
						return;
					}						
#endif					
					if (devinx >= 8)
					{
						return;
					}	
				}
				//printf("Report device:%bu\n\r",devinx);
				Remote_Device_Report_Pup_Stack[Remote_Devinx_Pup_Index] = devinx;
				Remote_Devinx_Pup_Index++;
			}	
		}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: void ExtenderR_Remote_Device_Restart_Create(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void ExtenderR_Remote_Device_Restart_Create(void)
{	
	U8_T segment_start;
#ifndef HIKDVR 	
	U8_T segment_end;
	U8_T hubports;
	U8_T hubdevinx;		
	U8_T index;
#endif	
	//scan the root hub port
	//1.Get root Hub first
	Remote_Devinx_Pup_Index = 0;
	segment_start = 0;
	
#ifdef HIKDVR
	ExtenderR_Device_ReCreate_Sub(USBDC_VIRTUAL_DEVINX);
	//goto ExtenderR_Remote_Device_Restart_Create_End; 
#else	
	for (index=0; index < USB_HC_MAX_PORT ; index++)
	{
		//Any device connect?		
		ExtenderR_Device_ReCreate_Sub(USBHC_RootHub_Devinx[index]);		
	}

	segment_end = Remote_Devinx_Pup_Index;

	//2.Check these device under hub
ExtenderR_Remote_Device_Restart_Create_Restart:
	for (index=segment_start; index < segment_end ; index++)
	{
		hubdevinx = Remote_Device_Report_Pup_Stack[index];
		if (USB_PDevice[hubdevinx].DevClass == USB_HUB_CLASS) //find hub class
		{
			hubports = USB_PDevice[hubdevinx].Hub_NbrPorts & USBHC_HUBPORT_MASK;
			/*record every devinx in the hub*/
			for (index=0; index < hubports; index++)
			{
				ExtenderR_Device_ReCreate_Sub((USB_PDevice[hubdevinx].HUB.ReportState+index)->Devinx);				
			}
		}
	}

	if (Remote_Devinx_Pup_Index > segment_end) //find new device, go scan again
	{
		segment_start = segment_end;
		segment_end = Remote_Devinx_Pup_Index;
		goto ExtenderR_Remote_Device_Restart_Create_Restart;
	}
#endif

	//-----------------------
	//the final report
	//-----------------------
	if (Remote_Devinx_Pup_Index)
	{		
		Remote_Devinx_Pup_Start=0;
		TASK_Run(&TASK_ExtenderR_Devcie_Rereport_TASK);
	}
/*	
#ifdef DEVICE_KEEP
	else
	{
		printf("No Device keep\n\r");
		ExtenderR_Remote_Device_List_Contorl(0x00); // no any device, so start the device keep operation
	}		
#endif
*/	
}

#ifdef DEVICE_KEEP
/*----------------------------------------------------------------------------
 * Function Name: void ExtenderR_Device_Table_Check(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void ExtenderR_Device_Table_Check(void)
{
	U8_T index;
	U8_T dev_end = USB_HC_MAX_DEVICE;

#ifdef HIKDVR
	dev_end = 2;
#endif	
	for (index=1; index < dev_end; index++)
	{
		if (USB_PDevice[index].Addr & USBHC_DEVICE_USED_MASK) //if device used
		{
			if ((ExtenderT_Device_Mount_Change & BIT_MASK[index]) == 0)
			{			
				USBDC_Device[index].DevIdx |= USBDC_DEVIDX_INTR_SEND_OK_MASK;
			}				
		}			
	}	
}	
/*----------------------------------------------------------------------------
 * Function Name: void ExtenderR_Device_Table_Transfer(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void ExtenderR_Device_Table_Transfer(void)
{
	U8_T  index;
	U8_T  devinx;
	U8_T  device_end=USB_HC_MAX_DEVICE;
	
#ifdef HIKDVR
	device_end = 2; // only transfer the virtual generic hid device
#endif
	for (index=1; index < device_end; index++)
	{
		devinx = index-1;
		if (USB_PDevice[index].Addr & USBHC_DEVICE_USED_MASK) //if device used
		{
			DeviceKeepTable[devinx].ivendor  = USB_PDevice[index].idVendor;
			DeviceKeepTable[devinx].iproduct = USB_PDevice[index].idProduct;			
		}
		else
		{
			DeviceKeepTable[devinx].ivendor = 0;
			DeviceKeepTable[devinx].iproduct = 0;
		}			
	}
	ExtenderR_Transmitter_Data_Send(NO_CTRL,EXTENDER_T_DEVICE_TABLE,NO_CTRL,NO_VALUE,sizeof(DeviceKeepTable),(U8_T *)DeviceKeepTable);
}
/*----------------------------------------------------------------------------
 * Function Name: void ExtenderR_Remote_Device_List_Contorl(U8_T control)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void ExtenderR_Remote_Device_List_Contorl(U8_T control)
{
	//Contorl :
	//1. 0x01 : Start
	//2. 0x00 : End
	ExtenderR_Transmitter_Data_Send(NO_CTRL,EXTENDER_T_DEVICE_LIST,control,NO_VALUE,NO_LEN,NO_DATA);
}
#endif
/*----------------------------------------------------------------------------
 * Function Name: U8_T ExtenderR_Setup_Buffer_Malloc(U8_T devinx)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
U8_T ExtenderR_Setup_Buffer_Malloc(U8_T devinx)
{
#if (EXTENDER_PASSTHROUGH_BUF_MALLOC)
	U16_T len;
#endif

	USB_PDevice[devinx].Hc.PControl.Total_Length = USBDC_Device[devinx].Setup[REMOTE_HOST_PORT].b.wLength.w;
	if (USB_PDevice[devinx].Hc.PControl.Total_Length > EXTENDER_PAGE_SIZ)
	{
		USB_PDevice[devinx].Hc.PControl.Total_Length = EXTENDER_PAGE_SIZ; //page length
	}
#if (EXTENDER_PASSTHROUGH_BUF_MALLOC)
	len = USB_PDevice[devinx].Hc.PControl.Total_Length+sizeof(Extender_Packet_Header_Def)+2;
	USBDC_Device[devinx].Control_EndpBuf[REMOTE_HOST_PORT] = m_malloc(len,29);
	if (USBDC_Device[devinx].Control_EndpBuf[REMOTE_HOST_PORT] == NULL)
	{
		//printf(" <R>:Setup Malloc Fail !!\n\r");
		return 0;
	}
#else
	USBDC_Device[devinx].Control_EndpBuf[REMOTE_HOST_PORT] = Extender_Passthrough_Buf;
#endif
	Data_Transmit_Buf = USBDC_Device[devinx].Control_EndpBuf[REMOTE_HOST_PORT];
	USBDC_Device[devinx].Control_EndpBuf[REMOTE_HOST_PORT] += sizeof(Extender_Packet_Header_Def);
	return 1;
}

/*----------------------------------------------------------------------------
 * Function Name: U8_T TASK_ExtenderR_Devcie_Rereport(U8_T devinx)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void TASK_ExtenderR_Devcie_Rereport(void)
{
	U8_T devinx;
	
	TASK_Destory_Current();
	devinx = Remote_Device_Report_Pup_Stack[Remote_Devinx_Pup_Start];	
	Externder_Receiver_DeviceMount_Control(devinx,VDEV_MOUNT,EXTENDER_USB_REMOUNT);
	Remote_Devinx_Pup_Start++;
	if (Remote_Devinx_Pup_Start < Remote_Devinx_Pup_Index)
	{
		if (USB_PDevice[devinx].DevClass == USB_HUB_CLASS)
		{
			TASK_Run(&TASK_ExtenderR_Devcie_Rereport_TASK);
		}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: void ExtenderR_Clear_Remote_Passthrough_Condition(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void ExtenderR_Clear_Remote_Passthrough_Condition(void)
{
	U8_T index;

	for (index=USBDC_VHID_DEVINX; index < USBDC_DEVICE_MAX ; index++)
	{
		if (USB_PDevice[index].Addr & USBHC_DEVICE_USED_MASK)
		{
			if (USBDC_Device[index].PassThroughState[REMOTE_HOST_PORT])
			{
				ExtenderR_PassThrough_Setup_Condition_Reset(index);
				//printf("Clear Devinx:%d\n\r",(U16_T)index);
			}
		}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: void ExtenderR_Check_Transmitter_Plug_State(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
U8_T ExtenderR_Check_Transmitter_Plug_State(void)
{
	return (Receiver_Control_State & TRANSMITTER_CONNECTION_MASK);
}

/*----------------------------------------------------------------------------
 * Function Name: void ExtenderR_Malloc_Intr_Buffer(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
/* 
U8_T ExtenderR_Malloc_Intr_Buffer(U8_T td_id,U8_T endpsize)
{
	U16_T len;

	len = endpsize+sizeof(Extender_Packet_Header_Def)+2;
	HC_IntTransfer_Table[td_id].ExtenderR = m_malloc(len);
	if (HC_IntTransfer_Table[td_id].ExtenderR != NULL)
	{
		return 1;
	}
	printf("Intr Buf Malloc Error\n\r");
	return 0;
}
*/
/*----------------------------------------------------------------------------
 * Function Name: void ExtenderR_Transmit_Intr_Endp_In_Data()
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Transmit_Intr_Endp_In_Data(U8_T devinx,U8_T td_id,U8_T vhid_inf,U8_T data_len)
{
	U16_T	len;
	U8_T	*buf;
	U8_T	intf_id;
	U8_T	endpidx;
#if (KVM_EXTENDER_HID_SHORT_PACKET)	
	Extender_Packet_Int_Header_Def  *header;
#endif	

#ifndef HIKDVR
	vhid_inf = 0;
#endif	
	if (ExtenderR_Check_Transmitter_Plug_State()==0) //check the transmitter connect condition.
	{
		HC_IntTransfer_Table[td_id].PassControl.Wp = 0;
		HC_IntTransfer_Table[td_id].PassControl.Rp = 0;
		HC_IntTransfer_Table[td_id].PassControl.State &= ~PASSTHROUGH_WAIT;
		return;
	}

	HC_IntTransfer_Table[td_id].PassControl.State |= PASSTHROUGH_WAIT; //data in processing of sending
	
#if (KVM_EXTENDER_HID_SHORT_PACKET)
	len = data_len+sizeof(Extender_Packet_Int_Header_Def)+2;
	header = (Extender_Packet_Int_Header_Def *) HC_IntTransfer_Table[td_id].ExtenderR;
#else
	len = data_len+sizeof(Extender_Packet_Header_Def)+2;
#endif	
	buf = HC_IntTransfer_Table[td_id].ExtenderR+sizeof(Extender_Cmd_Token_Def);
	intf_id = (HC_IntTransfer_Table[td_id].InterfaceIdx << 4);
	endpidx = (HC_IntTransfer_Table[td_id].EndpIdx & USBDC_DEV_ENDPINX_MASK ) >> 4;
	intf_id |= endpidx;

	//Check if there is a data packet in queue?
	//if (len)
	{
#ifdef HIKDVR
		//printf("%bu",vhid_inf);	
		devinx |= ( td_id << 4);		
#else		
		devinx |= (td_id << 4);		
#endif
		
#if (KVM_EXTENDER_HID_SHORT_PACKET)		
		Externder_Packet_Header((Extender_Packet_Header_Def *) HC_IntTransfer_Table[td_id].ExtenderR,Recevier_Int_Token);		 
		header->Cmd_Token.bCmd_CRC = data_len;				 
		Extender_Packet_Int_Data((Extender_Data_Int_Packet_Def *)buf,data_len,NULL,devinx,intf_id);					
#else
		Externder_Packet_Header((Extender_Packet_Header_Def *) HC_IntTransfer_Table[td_id].ExtenderR,Recevier_Cmd_Token);
		Extender_Packet_Data((Extender_Data_Packet_Def *)buf,data_len,NULL,EXTENDER_USB_INTR_IN,devinx,intf_id);
#endif

#if (KVM_EXTENDER_HID_NO_ACK)		
		Extender_Token_Transmit(HC_IntTransfer_Table[td_id].ExtenderR,EXTENDER_CONTROL_NONE_FREE|EXTENDER_CONTROL_TRANSMIT_NO_WAIT,len,100,1);
#else
		Extender_Token_Transmit(HC_IntTransfer_Table[td_id].ExtenderR,EXTENDER_CONTROL_NONE_FREE,len,100,1);
#endif		
	}
}

/*----------------------------------------------------------------------------
 * Function Name: void ExtenderR_Interrupt_Transfer_In_Data_Handle(U8_T devinx,U8_T intt_id)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Interrupt_Transfer_In_Data_Handle(U8_T devinx,U8_T intt_id,U8_T vhid_inf)
{
	U8_T	length;
	//idata U8_T	rp;
	U8_T	*p;

#if (KVM_EXTENDER_HID_SHORT_PACKET)
	p = HC_IntTransfer_Table[intt_id].ExtenderR+sizeof(Extender_Packet_Int_Header_Def);
#else
	p = HC_IntTransfer_Table[intt_id].ExtenderR+sizeof(Extender_Packet_Header_Def);
#endif	
	
	length = DATAST_Move_Intr_Buf_Move(intt_id,p,REMOTE_HOST_PORT);
	/*
	rp = HC_IntTransfer_Table[intt_id].PassControl.Rp;
	length = HC_IntTransfer_Table[intt_id].PassControl.Buf[rp];

	rp ++;
	if (rp >= HC_IntTransfer_Table[intt_id].PassControl.BufSize)
		rp = 0;

	for (index=0;index < length ;index++)
	{
		p[index] = HC_IntTransfer_Table[intt_id].PassControl.Buf[rp];
		rp++;
		if (rp >= HC_IntTransfer_Table[intt_id].PassControl.BufSize)
			rp = 0;
	}

	HC_IntTransfer_Table[intt_id].PassControl.Rp = rp; // update the RP pointer
	*/
#ifdef HIKDVR
	printf("%bu",vhid_inf);
	ExtenderR_Transmit_Intr_Endp_In_Data(devinx,intt_id,vhid_inf,length);
#else
	vhid_inf = 0;
	ExtenderR_Transmit_Intr_Endp_In_Data(devinx,intt_id,0,length);
#endif	

#if (VIRTUAL_USB_HID_DEBUG)
	datainq_flag = 1;
	if (datainq_flag) // content has changed
	{
		printf("HIDQ=[ ");
		for (index=0; index < length ; index++)
		{
			printf("%02x ",(U16_T)p[index]);
		}
		printf("]\n\r");
	}
#endif
}

/*----------------------------------------------------------------------------
 * Function Name: void ExtenderR_Clear_IntrBuffer_Wait(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Clear_IntrBuffer_Wait(void)
{
	U8_T index;

	if (KVM_CurrentHost == REMOTE_HOST_PORT)
	{
		for (index = 0; index < USB_HC_INTL_MAX; index++)
		{
			//if (HC_IntTransfer_Table[index].Used_DeviceID & USBHC_DEVICE_USED_MASK)
			//{
			//	continue; // go next port
			//}
			HC_IntTransfer_Table[index].PassControl.State &= ~PASSTHROUGH_WAIT; //data in processing of sending
		}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: void ExtenderR_Clear_Intr_Send_OK(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Clear_Intr_Send_OK(void)
{
	U8_T index;

	for (index = 0; index < USBDC_DEVICE_MAX; index++)
	{
		USBDC_Device[index].DevIdx &= ~USBDC_DEVIDX_INTR_SEND_OK_MASK;
	}
}
 
/*----------------------------------------------------------------------------
 * Function: void ExtenderR_PassthroughHostLedControlHandle(U8_T ledStatus)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
void ExtenderR_PassthroughHostLedControlHandle(U8_T ledStatus)
{
#ifdef PCT_MUA22
	ledStatus = 0;
#else
	if (ledStatus == LED_ON)
	{
		KVM_Select_Led_Control(REMOTE_HOST_PORT, LED_ON); //Orange LED
	}
	else
	{	
		KVM_Select_Led_Control(REMOTE_HOST_PORT, LED_OFF); //Orange LED
	}
#endif	
}

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Passthrough_Audio_Data_Out_Handle(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Passthrough_Audio_Data_Out_Handle(void)
{
	U8_T	len;

	len = Extender_Setup.Data_Token.iLen;

	Extender_CopyDataFromRxBuffer(Extender_Setup.iRecBuf_Index, len, ETDR_AudioOutBuf[ETDR_AudioOutHead]);
	ETDR_AudioOutHead++;
	ETDR_AudioOutHead &= EXTENDER_AUDIO_OUT_BUF_MASK;

	if (ETDR_AudioOutHead == ETDR_AudioOutTail)
	{
		ETDR_AudioOutHead--;
		ETDR_AudioOutHead &= EXTENDER_AUDIO_OUT_BUF_MASK;
	}

	if (ETDR_AudioOutHcDoingFlag == 0)
	{
		if (((ETDR_AudioOutHead - ETDR_AudioOutTail) & EXTENDER_AUDIO_OUT_BUF_MASK) >= EXTENDER_AUDIO_OUT_WATER_LEVEL)
		{
			DMA_GrantXdata(HCTD_Table.ISTL[0].Buf, ETDR_AudioOutBuf[ETDR_AudioOutTail], len);

			HCTD_Table.ISTL[0].TD.Byte0_Actual_Byte = 0;
			HCTD_Table.ISTL[0].TD.Byte2_MaxPKT_Size = HCTD_Table.ISTL[0].Buf[0];
			HCTD_Table.ISTL[0].TD.Byte3 &= 0xFC;
			HCTD_Table.ISTL[0].TD.Byte4_Total_Byte = HCTD_Table.ISTL[0].Buf[0];
			HCTD_Table.ISTL[0].TD.Byte5 &= 0xFC;
			HCTD_Table.ISTL[0].TD.Byte1 |= TD_ACTIVED_MASK;
			TD_ISTL_Skip_Map &= ~0x01;
			USBHC_INT_DISABLE;	
			//Clear Done Map
			_USBHC_HCDR_SFR(0x01);
			_USBHC_HCCIR_SFR(HC_ISTL_TDDONE_MAP_REG);

			//Clear Skip Map
			_USBHC_HCDR_SFR(TD_ISTL_Skip_Map);
			_USBHC_HCCIR_SFR(HC_ISTL_TDSKIP_MAP_REG);
			USBHC_INT_ENABLE;
			
			ETDR_AudioOutTail++;
			ETDR_AudioOutTail &= EXTENDER_AUDIO_OUT_BUF_MASK;

			ETDR_AudioOutHcDoingFlag = 1;
		}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Audio_Data_In_Stage
 * Purpose:  
 * Params:
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Audio_Data_In_Stage(void)
{
	U8_T XDATA*	pbuf;
	U8_T		data_len;
	U8_T		len;
	Extender_Data_Packet_Def XDATA*	header;
	
	if (ExtenderR_Audio_In_Cnt > 5) //avoid over flow the transmitter buffer
	{ // Skip the audio IN
		//ETDR_AudioInTail++;	
		//ETDR_AudioInTail &= EXTENDER_AUDIO_IN_BUF_MASK;
		return;
	}	
	
	ExtenderR_Audio_In_Cnt++;
		
	pbuf = (U8_T XDATA*)&ETDR_AudioInBuf[ETDR_AudioInTail][0];
	pbuf[0] = EXTENDER_CMD_TOKEN;
	pbuf[1] = EXTENDER_TRANSMITTER_ADDR;
	pbuf[2] = pbuf[0] ^ pbuf[1];

	header = (Extender_Data_Packet_Def XDATA*)&pbuf[EXTENDER_COMMAND_TOKEN_LEN];

	data_len = pbuf[EXTENDER_PACKET_HEADER_DATA_OFFSET] +2;
	len = data_len + EXTENDER_PACKET_HEADER_DATA_OFFSET;

	if (Transmitter_Data_Toggle == 0)
	{
		header->bData_Token = EXTENDER_DATA0_TOKEN;
		Transmitter_Data_Toggle = 1;
	}
	else
	{
		header->bData_Token = EXTENDER_DATA1_TOKEN;
		Transmitter_Data_Toggle = 0;
	}

	header->iLen = data_len;
	header->bRequest = EXTENDER_USB_ISO_IN;
	header->bIndex = USBHC_Audio_Devinx;
	header->bValue = 0;
#if (EXTENDER_TRANSACTION_SERIAL_ID)	
	Trainsmit_Table[Extender_Current_TransId] = EXTENDER_Transmit_InP;
	header->bTid   = Extender_Current_TransId;
	Extender_Current_TransId++;
	if (Extender_Current_TransId >= EXTENDER_TRANSMIT_FIFO_MAX)
		Extender_Current_TransId = 0;
#endif		
		
	//header->bTid   = EXTENDER_Transmit_InP;			
	Extender_Token_Transmit (pbuf, (EXTENDER_CONTROL_TRANSMIT_NO_WAIT|EXTENDER_CONTROL_NONE_FREE), len, 1000, 0);
	if (ETDR_AudioInTailSkipFlag)
	{
		ETDR_AudioInTailSkipFlag = 0;
		ETDR_AudioInTail++;
	}
	else
	{
		ETDR_AudioInTail++;
	}
	ETDR_AudioInTail &= EXTENDER_AUDIO_IN_BUF_MASK;
}

/*----------------------------------------------------------------------------
 * Function Name: TASK_ExtenderR_AudioInWait(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
/* 
void TASK_ExtenderR_AudioInWait(void)
{
	if (ETDR_AudioInWaitFlag == 1)
	{
		ETDR_AudioInWaitFlag = 0;
		if ((USB_PDevice[USBHC_Audio_Devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00) // Device is gone
		{
			return;
		}

		TD_ISTL_Skip_Map &= ~(USB_HC_ISTL_IN_DONE_MASK);   // Skip ISTL3

		//4.Enable the TD Buffer
		HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte0_Actual_Byte = 0;
		HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte2_MaxPKT_Size = USB_HC_ISTL_BUF_SIZE - 2;
		HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte3 &= 0xFC;
		HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte4_Total_Byte = USB_HC_ISTL_BUF_SIZE-2; //get the byte will not over the bufer size
		HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte5 &= 0xFC;
		HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte1 |= TD_ACTIVED_MASK;
		HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte7  = 0;

		//Clear Done Map
		USBHC_INT_DISABLE;
		_USBHC_HCDR_SFR(USB_HC_ISTL_IN_DONE_MASK);
		_USBHC_HCCIR_SFR(HC_ISTL_TDDONE_MAP_REG);

		//Clear Skip Map
		_USBHC_HCDR_SFR(TD_ISTL_Skip_Map);
		_USBHC_HCCIR_SFR(HC_ISTL_TDSKIP_MAP_REG);
		USBHC_INT_ENABLE;
	}
}
*/
#endif

#if (SYSTEM_EXTENDER_MSC_SUPPORT)
#if (SYSTEM_USB_HC_BURST)
/*----------------------------------------------------------------------------
 * Function: void ExtenderR_PassthroughMscBurstOutHandle(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
void ExtenderR_PassthroughMscBurstOutHandle(void)
{
	U16_T	iLen;
	U8_T	tdBulk;	
	
	if (USBHC_MSC_Device_Error_Condition_Check(USBHC_MSC_Devinx))
	{
		return;
	}	
	
	//------------------------------------------------------------------------------------------------
	// @@ This should not happened, because there is a traffic control mechanism, if buffer full
	//    then no EXTENDER_USB_MSC_DATA_OUT will responsed to Transmitter, 
	//
	//if (USB_PDevice[USBHC_MSC_Devinx].MSC->BURST_Out_State & USBHC_MSC_Burst_Used[tdBulk])	
	if (USBHC_MSC_PingPongOut_UseState(USBHC_MSC_PingPongOut_ID)) //Check current pingpong td used state
	{		
		//printf("!\n\r");
		return;
	}
	//------------------------------------------------------------------------------------------------
	tdBulk = USBHC_MSC_PingPongOut_ID - USB_HC_MSC_START; //Get the BULK TD index	
	iLen = Extender_Setup.Data_Token.iLen;	
	Extender_CopyDataFromRxBuffer(Extender_Setup.iRecBuf_Index, iLen, HCTD_Table.BULK[tdBulk].Buf);
	
	//Check next pingpong buffer, if in used, then this td need wait for done....
	ExtenderR_TD_Skip_Active_Flag = USBHC_MSC_Next_PingPongOut_UseState(USBHC_MSC_PingPongOut_ID);
	
	//Trigger HC to send TD to devices
	USBHC_MSC_Bulk_TD_Header(USBHC_MSC_Devinx, tdBulk, MSC_BOT_DATA_OUT, (iLen-2));
	
	ExtenderR_TD_Skip_Active_Flag = 0; //very important, do not delete it, for TD_Header is used in normal MSC control, so
	                                   //the ExtenderR_TD_Skip_Active_Flag will block active flag and skip map
	
	//HC TD burst out handle
	if (ExtenderR_MSC_BurstDataOutCheck(USBHC_MSC_Devinx) == USB_SUCCESS)
	{
		USBHC_MSC_PingPongOut_ID = USBHC_MSC_GetNext_PingPongOut(USBHC_MSC_PingPongOut_ID);			
	}	
	//EXTENDER_MSC_BurstWait = 1;	
	
	//Handle the MSC out page issue
	tdBulk = USBHC_MSC_PingPongOut_ID - USB_HC_MSC_START; //Get the BULK TD index	
	if (USB_PDevice[USBHC_MSC_Devinx].MSC->BURST_Out_State & USBHC_MSC_Burst_Used[tdBulk])
	{				
		EXTENDER_MSC_BurstOutWait_Flag = 1;		
	}	
	else
	{		
		EXTENDER_MSC_BurstOutWait_Flag = 0;	
		ExtenderR_Transmit_MSC_Out_Ready();
	}	
}

/*----------------------------------------------------------------------------
 * Function: RESULT ExtenderR_MSC_BurstDataOutCheck(U8_T devinx)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
RESULT ExtenderR_MSC_BurstDataOutCheck(U8_T devinx)
{
	U8_T 	bulk_td;
	U8_T    *burst_buf;
	U16_T	len;
	RESULT	Result=USB_SUCCESS;

	//1.Get bulk td buffer id
	bulk_td = USBHC_MSC_PingPongOut_ID - USB_HC_MSC_START; //Get the BULK TD index
	//2.Get data package length
	USBHC_MSC_Get_Burst_Length_From_DC(devinx,bulk_td);
	len = USB_PDevice[devinx].MSC->Control.Data_Length;
	//3.Get data package buffer pointer
	burst_buf = &HCTD_Table.BULK[bulk_td].Buf[2];	//Buffer Pointer

    //4.Get data package buffer pointer
	switch (USB_PDevice[devinx].MSC->BOT_State)
	{
		case MSC_BOT_IDLE: //wait for CBW
			//1.check the CBW valid condition	
			Result= USBHC_MSC_BOT_CBW_Decode(devinx,len,burst_buf);			 
			if (Result == USB_SUCCESS)
			{
				burstout_cnt = 0; //for debug												
				//USBHC_MSC_Set_CurrentPingPongOut_Used(devinx);				
				if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_DATA_OUT) //Generate the first page for ATL TD INPUT
				{
					USBHC_MSC_Active_State |= MSC_ACTIVE_BUSY; 			//Transaction start
				}
				else if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_DATA_IN) 
				{
					USBHC_MSC_Active_State |= MSC_ACTIVE_BUSY;		//Transaction start
				}
				else //if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_CSW_REQUEST)
				{
					USBHC_MSC_Active_State |= MSC_ACTIVE_TEST_UNIT_READY;//Check Unit Ready
				}
				//USBHC_MSC_PingPongOut_ID = USBHC_MSC_GetNext_PingPongOut(USBHC_MSC_PingPongOut_ID);				 
			}
			else
			{
				//printf("CBW_Decode Error");
				return USB_ERROR;	// indicate the CBW command error
			}
			break;
		case MSC_BOT_DATA_OUT:
			//Get the DC burst data package length
			//Maintain the remain total length			
			USBHC_MSC_Set_CurrentPingPongOut_Used(devinx);
			if (USB_PDevice[devinx].MSC->Control.Total_Length > len)
			{	
				USB_PDevice[devinx].MSC->Control.Total_Length -= len;
			}	
			else
			{
				USB_PDevice[devinx].MSC->Control.Total_Length = 0;
			}							
			 
			if (USB_PDevice[devinx].MSC->Control.Total_Length == 0)
			{
				USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_CSW_REQUEST; //Next,wait for CSW
			}
			//USBHC_MSC_PingPongOut_ID = USBHC_MSC_GetNext_PingPongOut(USBHC_MSC_PingPongOut_ID);			
			break;
		default:
			//printf("R_UNKNOW Status=%02bx,%02bx\n\r",USB_PDevice[devinx].MSC->BOT_State,USB_PDevice[devinx].MSC->BURST_Out_State);
			Result=USB_ERROR;
			break;
	}
	return Result;
}

/*----------------------------------------------------------------------------
 * Function: RESULT ExtenderR_MSC_Burst_Done_Handle(U8_T devinx, U8_T atl_id, U8_T jump)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
RESULT ExtenderR_MSC_Burst_Done_Handle(U8_T devinx, U8_T atl_id, U8_T jump)
{
	U8_T	pinpong_td, req;
	U8_T    toggle_bit;
	U16_T	len;

	if (USBHC_MSC_Device_Error_Condition_Check(devinx))
	{
		return USB_SUCCESS;
	}

	if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_IDLE)
	{
		return USB_SUCCESS;
	}
	
	//This is only for Bulk In
	if (jump)
	{
		//maintain data toggle bit
		USB_PDevice[devinx].MSC->Toggle_In = 0;
		if ((HCTD_Table.BULK[atl_id].TD.Byte1 & TD_TOGGLE_MASK) == TD_TOGGLE_DATA1) //data1
		{
			USB_PDevice[devinx].MSC->Toggle_In = 1;
		}
		goto ERROR_HANDLE;
	}
	

	if ((HCTD_Table.BULK[atl_id].TD.Byte5 & TD_DIR_TOKEN_MASK) == TD_DIR_TOKEN_IN) // Handle the IN Token
	{	
		/* Endp Buffer Direction IN */
		USB_PDevice[devinx].MSC->BURST_In_State |= USBHC_MSC_Burst_Done[atl_id]; // In TD is Done
		
		len  = HCTD_Table.BULK[atl_id].TD.Byte0_Actual_Byte; //get input byte
		len += ((HCTD_Table.BULK[atl_id].TD.Byte1 & 0x03) << 8); //get input byte		
		
		//printf("<%d/%d>",(U16_T)USB_PDevice[devinx].MSC->Control.Total_Length,len);		
		//maintain data toggle bit
		if ((HCTD_Table.BULK[atl_id].TD.Byte1 & TD_TOGGLE_MASK) == TD_TOGGLE_DATA1) //data1
		{
			USB_PDevice[devinx].MSC->Toggle_In = 1;
		}
		else
		{
			USB_PDevice[devinx].MSC->Toggle_In = 0;
		}
		
		//1.Send Complete, reset the input & output index pointer
		if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_END) //the last data is csw IN,
		{
			req = EXTENDER_USB_MSC_CSW;
			USBHC_MSC_Active_State &= ~(MSC_ACTIVE_TEST_UNIT_READY|MSC_ACTIVE_BUSY);
			USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_IDLE;
//			printf("O_CSW_DONE\n\r");
		}
		else if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_DATA_IN) //the last data is csw IN,
		{
			req = EXTENDER_USB_MSC_DATA_IN;
			//Maintain the remained data length
			if (len % MSC_Class_Page_Size) /* all IN data has been transfer by MSC device */
			{
				if (USB_PDevice[devinx].MSC->Control.Total_Length < len)
				{
					//printf("<%d/%d>",(U16_T)USB_PDevice[devinx].MSC->Control.Total_Length,len);		
					USBHC_MSC_Active_State &= ~(MSC_ACTIVE_TEST_UNIT_READY|MSC_ACTIVE_BUSY);
					USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_IDLE;					
				}									
				USB_PDevice[devinx].MSC->Control.Total_Length = 0;
			}
			else if (USB_PDevice[devinx].MSC->Control.Total_Length > len) 
			{
				USB_PDevice[devinx].MSC->Control.Total_Length -= len;
			}
			else
			{
				USB_PDevice[devinx].MSC->Control.Total_Length = 0;
			}
			
			if (USB_PDevice[devinx].MSC->BOT_State != MSC_BOT_IDLE)
			{	
				if (USB_PDevice[devinx].MSC->Control.Total_Length == 0) //if the in is done
				{
					//Next should be CSW				 
					USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_CSW_REQUEST;
				}
			}	
		}
		
		// Copy the HC burst in data to RS485 buffer for transmitting
		//printf("[IH:%bu:%bu]",EXTENDER_MSC_BulkInHead,EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].bufUsingFlag);
		USBHC_MSC_Length_To_Buf(HCTD_Table.BULK[atl_id].Buf,len);
		//HCTD_Table.BULK[atl_id].Buf[0] = (U8_T)(len & 0x00FF);
		//HCTD_Table.BULK[atl_id].Buf[1] = (U8_T)((len & 0x0300) >> 8);
		
		DMA_GrantXdata(EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].bBuf, HCTD_Table.BULK[atl_id].Buf, (len + 2));

		EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].payloadLen = (len + 2);
		EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].reqType = req;
		EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].devIndex = devinx;
		
		
		EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].bufUsingFlag = 1;
		USBHC_MSC_Length_To_Buf(EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].bBuf,len);
		//EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].bBuf[0] = (U8_T)(len & 0x00FF);
		//EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].bBuf[1] = (U8_T)((len & 0x0300) >> 8);
		EXTENDER_MSC_BulkInHead++;
		EXTENDER_MSC_BulkInHead &= EXTENDER_MSC_BUF_MASK;
		
		//Check is there any buffer wait for IN, if yes then wait
		pinpong_td = USBHC_MSC_GetNext_PingPongIn_TD(atl_id);
		
		//if (atl_id == (USB_HC_MSC_BURST_IN_TD0 - USB_HC_MSC_START))
		//	pinpong_td = USB_HC_MSC_BURST_IN_TD1 - USB_HC_MSC_START;
		//else
		//	pinpong_td = USB_HC_MSC_BURST_IN_TD0 - USB_HC_MSC_START;
			
ERROR_HANDLE:
		if (((USB_PDevice[devinx].MSC->BURST_In_State & USBHC_MSC_Burst_Done[pinpong_td]) == 0x00) &&  
			(MSC_In_Wait_Flag == 0))	
		{
			//USBHC_MSC_BOT_Check_Next_In(devinx, atl_id);
			//Send the Bulk In data to the Extender Transmitter
			USBHC_MSC_UpstreamInTD = atl_id + USB_HC_MSC_START;
			ExtenderR_PassthroughMscBurstInHandle();
			
			if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_DATA_IN)
			{
				//printf("HC:%d\n\r",(U16_T)pinpong_td);
				if (USB_PDevice[devinx].MSC->Control.Total_Length > MSC_Class_Page_Size)
				{
					len = MSC_Class_Page_Size;
				}
				else
				{
					len = USB_PDevice[devinx].MSC->Control.Total_Length;
				}
				
				USBHC_MSC_PingPongIn_ID = pinpong_td + USB_HC_MSC_START;
				USBHC_MSC_Bulk_TD_Header(devinx, pinpong_td, MSC_BOT_DATA_IN, len);
				USB_PDevice[devinx].MSC->BURST_In_State |= USBHC_MSC_Burst_Used[pinpong_td]; // In TD in Used
			}
			else if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_CSW_REQUEST)
			{		
				USBHC_MSC_BOT_Send_CSW_Request(devinx,pinpong_td);
			}
		}
		else
		{			
			USB_PDevice[devinx].MSC->BURST_In_State |= BURST_IN_WAIT;
		}
	}
	else // Endp Buffer Direction OUT
	{
		if (EXTENDER_MSC_BurstOutWait_Flag)
		{			
			ExtenderR_Transmit_MSC_Out_Ready();	
		}		
		/* Endp Buffer Direction OUT */
		USB_PDevice[devinx].MSC->BURST_Out_State &= ~(USBHC_MSC_Burst_Used[atl_id]); //clear the used and done flag first
		
		//len  = HCTD_Table.BULK[atl_id].TD.Byte0_Actual_Byte; //get input byte
		//len += ((HCTD_Table.BULK[atl_id].TD.Byte1 & 0x03) << 8); //get input byte

		//maintain data toggle bit, after burst out, the data toggle bit will be store into TD, 
		//need to get the correct data bit	
		if ((HCTD_Table.BULK[atl_id].TD.Byte1 & TD_TOGGLE_MASK) == TD_TOGGLE_DATA1) //data1
		{
			USB_PDevice[devinx].MSC->Toggle_Out = 1;
			toggle_bit = TD_TOGGLE_DATA1;
		}
		else
		{
			USB_PDevice[devinx].MSC->Toggle_Out = 0;
			toggle_bit = TD_TOGGLE_DATA0;
		}
				 
		//1.Check Page Complete condition
		if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_END) //is the last TD
		{
			USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_IDLE;
		}
		else // handle the remain package
		{
			pinpong_td = USBHC_MSC_GetNext_PingPongOut_TD(atl_id);
			//if (atl_id == (USB_HC_MSC_BURST_OUT_TD0 - USB_HC_MSC_START))			
			//	pinpong_td = USB_HC_MSC_BURST_OUT_TD1 - USB_HC_MSC_START;
			//else
			//	pinpong_td = USB_HC_MSC_BURST_OUT_TD0 - USB_HC_MSC_START;
			if (EXTENDER_MSC_BurstOutWait_Flag)
			{	
				if ((USB_PDevice[devinx].MSC->BURST_Out_State & USBHC_MSC_Burst_Used[pinpong_td]))
				{
					HCTD_Table.BULK[pinpong_td].TD.Byte1 = (COMPLETEION_CODE_MASK | TD_ACTIVED_MASK | toggle_bit);
					USBHC_CORE_SetBit(TD_ATL_Skip_Map,(pinpong_td+USB_HC_MSC_START),0x00);
					USBHC_Write_Regs(HC_ATL_TDSKIP_MAP_REG, TD_ATL_Skip_Map, 4);
				}
				EXTENDER_MSC_BurstOutWait_Flag = 0;
			}		
			else
			{	
				if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_CSW_REQUEST)
				{
					pinpong_td = USBHC_MSC_EnableNextBurstTD(MSC_BOT_DATA_IN);
					USBHC_MSC_BOT_Send_CSW_Request(devinx, pinpong_td);
				}
				else if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_DATA_IN)
				{
					//pinpong_td = USBHC_MSC_PingPongIn_ID - USB_HC_MSC_START; //Get Current TD
					pinpong_td = USBHC_MSC_EnableNextBurstTD(MSC_BOT_DATA_IN);
					USB_PDevice[devinx].MSC->BURST_In_State |= USBHC_MSC_Burst_Used[pinpong_td];
					USBHC_MSC_Bulk_TD_Header(devinx, pinpong_td, MSC_BOT_DATA_IN, USB_PDevice[devinx].MSC->Control.Data_Length);
				}
			}				
		}
	}
	return USB_SUCCESS;
}

/*----------------------------------------------------------------------------
 * Function: void ExtenderR_PassthroughMscBurstInHandle(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
void ExtenderR_PassthroughMscBurstInHandle(void)
{
	U8_T	*pDataBuf, *pCmdToken, *pDataToken;
	U16_T	dataLen, extLen;
	U8_T	extIndex, extReq, extVal;
	Extender_Data_Packet_Def  *pToken;

	if (EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].bufUsingFlag == 0)
	{
		//printf("No Bulk In data\n\r");
		return;
	}
	extVal = EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].serial;
	pCmdToken = (U8_T *)(&(EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].Cmd_Token));
	pDataToken = (U8_T *)(&(EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].Data_Token));
	pToken = (Extender_Data_Packet_Def *) pDataToken;
	pDataBuf = EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].bBuf;
	extIndex = EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].devIndex;
	dataLen = EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].payloadLen;
	extReq = EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].reqType;
	extLen = dataLen + sizeof(Extender_Packet_Header_Def)+2;
	
	/*
	if (extLen == 0)
	{
		printf("EXTENDER_MSC_BulkInBuf Length Error\n\r");
		return;
	}
	*/
		
	if (extReq == EXTENDER_USB_MSC_STALL)
	{
		USBHC_MSC_State |= MSC_STATE_CHECK_BULK_IN;
		extVal = EXTENDER_MSC_StallFlag;
		EXTENDER_MSC_StallFlag = 0;
	}
	
	// Fill the Command Token
	*(pCmdToken + 0) = Recevier_Cmd_Token[0];
	*(pCmdToken + 1) = Recevier_Cmd_Token[1];
	*(pCmdToken + 2) = Recevier_Cmd_Token[0] ^ Recevier_Cmd_Token[1];
	
	// Fill the Data Token	
	Extender_Packet_Data((Extender_Data_Packet_Def *)pDataToken, dataLen, NULL, extReq, extIndex, extVal);
	Extender_Token_Transmit(pCmdToken, EXTENDER_CONTROL_CLEAR_BUF, extLen, 3000, 1);
}

/*----------------------------------------------------------------------------
 * Function: void ExtenderR_PassthroughMscBurstInComplete(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
void ExtenderR_PassthroughMscBurstInComplete(void)
{
	U8_T	devinx;
	U8_T	bulk_td;
	
	//printf("C:%bu,%d",EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].serial,EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].payloadLen);
	devinx = EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].devIndex;
	// Clear the MSC buffer
	EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].bufUsingFlag = 0;
	EXTENDER_MSC_BulkInTail ++;
	EXTENDER_MSC_BulkInTail &= EXTENDER_MSC_BUF_MASK;
	
	//Handle the Burst IN Event and Clear the used and done flag first
	bulk_td = USBHC_MSC_UpstreamInTD - USB_HC_MSC_START;
	USB_PDevice[devinx].MSC->BURST_In_State &= ~(USBHC_MSC_Burst_Used[bulk_td]|USBHC_MSC_Burst_Done[bulk_td]);
	
	//Buffer handle
	if (MSC_In_Wait_Flag == 0)
	{			
		ExtenderR_MSC_Burst_In_Wait_Check(devinx);
	}	
}

/*----------------------------------------------------------------------------
 * ExtenderR_MSC_Burst_In_Wait_Check(U8_T devinx)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
void ExtenderR_MSC_Burst_In_Wait_Check(U8_T devinx)
{
	U8_T	bulk_td, next_td;
	U16_T	len;			
	
	//Handle the Burst IN Event and Clear the used and done flag first
	bulk_td = USBHC_MSC_UpstreamInTD - USB_HC_MSC_START;
		
	if (USB_PDevice[devinx].MSC->BURST_In_State & BURST_IN_WAIT)
	{
		USB_PDevice[devinx].MSC->BURST_In_State &= ~BURST_IN_WAIT;		
		next_td = USBHC_MSC_GetNext_PingPongIn_TD(bulk_td);
		/*		
		//if (bulk_td == (USB_HC_MSC_BURST_IN_TD0 - USB_HC_MSC_START))
		//	next_td = USB_HC_MSC_BURST_IN_TD1 - USB_HC_MSC_START;
		//else
		//	next_td = USB_HC_MSC_BURST_IN_TD0 - USB_HC_MSC_START;
		*/
		if ((USB_PDevice[devinx].MSC->BURST_In_State & USBHC_MSC_Burst_Done[next_td])) //if next is done by HC
		{
			//printf("CW:%bu\n\r",next_td);
			//Send the Bulk In data to the Extender Transmitter
			USBHC_MSC_UpstreamInTD = next_td + USB_HC_MSC_START;
			ExtenderR_PassthroughMscBurstInHandle();
			
			if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_DATA_IN)
			{
				if (USB_PDevice[devinx].MSC->Control.Total_Length > MSC_Class_Page_Size)
				{
					len = MSC_Class_Page_Size;
				}
				else
				{
					len = USB_PDevice[devinx].MSC->Control.Total_Length;
				}
				USBHC_MSC_PingPongIn_ID = bulk_td + USB_HC_MSC_START;
				USBHC_MSC_Bulk_TD_Header(devinx, bulk_td, MSC_BOT_DATA_IN, len);
				USB_PDevice[devinx].MSC->BURST_In_State |= USBHC_MSC_Burst_Used[bulk_td]; // In TD in Used
			}
			else if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_CSW_REQUEST)
			{
				USBHC_MSC_PingPongIn_ID = bulk_td + USB_HC_MSC_START;
				USBHC_MSC_BOT_Send_CSW_Request(devinx,bulk_td);
			}
		}
		//else
		//{
		//	printf("NO DONE\n\r");
		//}	
	}
}

/*----------------------------------------------------------------------------
 * Function: void ExtenderR_PassthroughMscBulkInStall(U8_T devinx, U8_T direction)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
void ExtenderR_PassthroughMscBulkInStall(U8_T devinx, U8_T direction, U8_T atl_id)
{
	U8_T	pinpong_td;

	EXTENDER_MSC_StallFlag = direction;
	if (EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].bufUsingFlag)
	{
		//printf("Stall IN_FULL\n\r");
		return;
	}

	EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].payloadLen = 0;
	EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].reqType = EXTENDER_USB_MSC_STALL;
	EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].devIndex = devinx;
	//printf("d2");
	EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].bufUsingFlag = 1;
	EXTENDER_MSC_BulkInHead ++;
	EXTENDER_MSC_BulkInHead &= EXTENDER_MSC_BUF_MASK;

//	printf("R_Stall\n\r");
	USB_PDevice[devinx].MSC->BURST_In_State |= USBHC_MSC_Burst_Done[atl_id]; // In TD is Done
	//Check is there any buffer wait for IN, if yes then wait
	pinpong_td = USBHC_MSC_GetNext_PingPongIn_TD(atl_id);
	/*
	//if (atl_id == (USB_HC_MSC_BURST_IN_TD0 - USB_HC_MSC_START))
	//	pinpong_td = USB_HC_MSC_BURST_IN_TD1 - USB_HC_MSC_START;
	//else
	//	pinpong_td = USB_HC_MSC_BURST_IN_TD0 - USB_HC_MSC_START;
	*/
	if ((USB_PDevice[devinx].MSC->BURST_In_State & USBHC_MSC_Burst_Done[pinpong_td]) == 0x00) //if next has not been used
	{
		//USBHC_MSC_BOT_Check_Next_In(devinx, atl_id);
		//Send the Bulk In data to the Extender Transmitter
		USBHC_MSC_UpstreamInTD = atl_id + USB_HC_MSC_START;
		ExtenderR_PassthroughMscBurstInHandle();
	}
	else
	{
//		printf("R_Stall in wait\n\r");
		USB_PDevice[devinx].MSC->BURST_In_State |= BURST_IN_WAIT;
	}
}

#endif //#if (SYSTEM_USB_HC_BURST)
#endif //#if (SYSTEM_EXTENDER_MSC_SUPPORT)

/*----------------------------------------------------------------------------
 * void ExtenderR_Transmitter_Port_Switch(U8_T transmitter_port)
 * Purpose: send command to transmitter to switch the port in transmitter
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Transmitter_Port_Switch(U8_T transmitter_port)
{
	transmitter_port--;
	//printf("Transmitter Switch Port:%bu\n\r",transmitter_port);
	ExtenderR_Transmitter_Data_Send(NO_CTRL,EXTENDER_KVM_T_PORT_SW,NO_INDEX,transmitter_port,NO_LEN,NO_DATA);
}

/*----------------------------------------------------------------------------
 * void ExtenderR_Transmitter_Buzzer_Control(U8_T value)
 * Purpose: Send command to control trnsmitter buzzer
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Transmitter_Buzzer_Control(U8_T value)
{	
	if (value)
		KVM_Flash.cSystemFlag2 |= SYSTEM_TX_BEEPER_MASK;
	else
		KVM_Flash.cSystemFlag2 &= ~SYSTEM_TX_BEEPER_MASK;
	ExtenderR_Transmit_System_Setting();
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Transmitter_AudioComboSw_Control
 * Purpose: initial the extender receiver
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Transmitter_AudioComboSw_Control(U8_T value)
{	
	if (value)
		KVM_Flash.cSystemFlag2 |= SYSTEM_TX_AUDIOCOMBO_MASK;
	else
		KVM_Flash.cSystemFlag2 &= ~SYSTEM_TX_AUDIOCOMBO_MASK;
	ExtenderR_Transmit_System_Setting();
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Transmitter_AudioSw_Control
 * Purpose: 
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Transmitter_AudioSw_Control(U8_T value)
{		
	//if (value <= KVM_MAX_PORT)
	{	
		value--;					
		ExtenderR_Transmitter_Data_Send(NO_CTRL,EXTENDER_KVM_T_AUDIO_SW,NO_INDEX,value,NO_LEN,NO_DATA);		
	}	
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Transmitter_MscSw_Control
 * Purpose: 
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Transmitter_MscSw_Control(U8_T value)
{		
	//if (value <= KVM_MAX_PORT)
	{	
		value--;					
		ExtenderR_Transmitter_Data_Send(NO_CTRL,EXTENDER_USB_T_MSC_SW,NO_INDEX,value,NO_LEN,NO_DATA);		
	}	
}
/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Transmitter_Data_Send
 * Purpose: initial the extender receiver
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Transmitter_Data_Send(U8_T control,U8_T request,U8_T index,U8_T value,U16_T data_len,U8_T *data_buf)
{
	U16_T len;
	
	/* Desp End */
	len = Receiver_Set_Transmit_Header(data_len,Recevier_Cmd_Token);
	if (len)
	{
		Extender_Packet_Data(Receiver_Send_Data_TokenP,data_len,data_buf,request,index,value);
		Extender_Token_Transmit(Receiver_Send_Buf,control,len,1000,1);
	}
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Transmitter_Data_Send
 * Purpose: initial the extender receiver
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_USB_Command_Handle(U8_T devinx,U8_T value,U8_T *buf)
{
	USB_PDevice[devinx].Hc.ScmdStep = *buf;	
	TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_Scm_Handle_Start_ID,0,devinx,value,0);
}

/*----------------------------------------------------------------------------
 * void ExtenderR_Interrupt_DataTypeTransfer_Check(U8_T index)
 * Purpose: 
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Interrupt_DataTypeTransfer_Check(U8_T index)
{
	U8_T td_id;
	U8_T devinx;
	
	devinx = index & 0x0f;
	td_id = (index & 0xf0) >> 4;
#if (SYSTEM_EXTENDER_RECEIVER) && (PROJECT_USB_GENERIC_HID_ENABLE)	
	if (devinx == USBDC_VHID_DEVINX) // if generic
	{		
		ExtenderR_PS2_PassThrough[td_id].State &= ~PASSTHROUGH_SET;
	}
	else	 
#endif		
	{
		HC_IntTransfer_Table[td_id].PassControl.State &= ~PASSTHROUGH_WAIT; // clear the wait state
		if (HC_IntTransfer_Table[td_id].PassControl.Wp != HC_IntTransfer_Table[td_id].PassControl.Rp) //There is still needed to handle queue
		{	//if still has data in queue
			if ((HC_IntTransfer_Table[td_id].PassControl.State & PASSTHROUGH_SET) == 0x00)
			{
				TASK_Active(TASK_TYPE_EVENT,TASK_DATAST_USB_Virtual_HID_ID,0,td_id,0,0); // now active the Generic KB Output Handle
			}
		}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Transmit_Intr_Endp_In_Data()
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Transmit_System_Setting(void)
{
#ifdef DEVICE_KEEP
	KVM_Flash.cSystemFlag2 |= SYSTEM_TX_DEVICE_KEEP_MASK;
#else
	KVM_Flash.cSystemFlag2 &= ~SYSTEM_TX_DEVICE_KEEP_MASK;
#endif	
	
	ExtenderR_Transmitter_Data_Send(NO_CTRL,EXTENDER_KVM_T_SYSTEM,NO_INDEX,KVM_Flash.cSystemFlag2,NO_LEN,NO_DATA);
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Transmit_MSC_Size_Setting(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Transmit_MSC_Size_Setting(void)
{
	U8_T bindex;
	U8_T bvalue;
	
	bindex = (MSC_Class_Page_Size & 0xff00) >> 8;
	bvalue = MSC_Class_Page_Size & 0x00ff;
	ExtenderR_Transmitter_Data_Send(NO_CTRL,EXTENDER_KVM_MSX_SIZE,bindex,bvalue,NO_LEN,NO_DATA);
}

#if (SYSTEM_MSC_DEVICE_SUPPORT) 
/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_MscInWaitHandle()
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_MscInWaitHandle(U8_T value)
{	
	//printf("In Wait Flag:%bu\n\r",value);
	if (value)
	{
		MSC_In_Wait_Flag = 1;
	}
	else
	{
		MSC_In_Wait_Flag = 0;
		//ExtenderR_MSC_Burst_In_Wait_Check(USBHC_MSC_Devinx);
	}			
}
/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_MscResetHandle()
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_MscResetHandle(void)
{
	U8_T devinx;
	
	devinx = Extender_Setup.Data_Token.bIndex;
	if (devinx == USBHC_MSC_Devinx)
	{
		//printf("Remote Reset : %bu\n\r",USBHC_MSC_ResetDevinx);
		USBHC_MSC_ResetDevinx = USBHC_MSC_Devinx;
		USBHC_MSC_Reset_Flag = 1;
		VHUB_MSC_Reset_Port = REMOTE_HOST_PORT;
		USBHC_MSC_Reset_DCPort = REMOTE_HOST_PORT;
		VHUB_MSC_Reset_HubDevinx = USBDC_VHUB_DEVINX; //always to Virtual HUB devinx
		VHUB_MSC_Reset_HubPort = USBDC_Virtual_Hub_Map[devinx].Devinx_HubPort;				
		//printf("reset port=%bu\n\r",VHUB_MSC_Reset_HubPort);
		USBDC_Maintain_MSC_PDevice(KVM_CurrentMSC);
		//Extender_MSC_Init(1);
	}	
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Remote_MscResetCheck()
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Remote_MscResetCheck(U8_T devinx)
{	
	if (USBHC_MSC_Reset_Flag)
	{
		if (devinx != USBHC_MSC_ResetDevinx)
		{						
			ExtenderR_Transmitter_Data_Send(NO_CTRL,EXTENDER_R_MSC_REDEVINX,USBHC_MSC_ResetDevinx,devinx,NO_LEN,NO_DATA);
		}	
		else
		{			
			ExtenderR_Transmitter_Data_Send(NO_CTRL,EXTENDER_R_MSC_RESET_ENABLE,devinx,NO_VALUE,NO_LEN,NO_DATA);
		}	
		USBHC_MSC_Reset_Flag = 0;
	}				
}

#endif  /* #if (SYSTEM_MSC_DEVICE_SUPPORT) */

#ifdef PWAYTEK_01
/*----------------------------------------------------------------------------
 * void ExtenderR_Maintain_DeviceState_Led(U8_T index)
 * Purpose: 
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Maintain_DeviceState_Led(void)
{	
#if (SYSTEM_EXTENDER_RECEIVER)	
	if (USBHC_Max_Device_Support <= (8-USBHC_Current_Total_Device_Count))	
	{			
		DEVICE_STATE_LED_GPIO = LED_OFF;
	}	
	else
	{		
		DEVICE_STATE_LED_GPIO = LED_ON;
	}	
#endif	
}
#endif										 

/* 
void Dump_Receiver_Buff(void)
{
	U16_T start,templ;
	
	start = Receive_Buffer_Start;
	if (Receive_Buffer_Start < 250 )
	{
		templ = 250 - start;
		start = UR2_RX_BUF_SIZE - templ; 
	}
	else
	{			
		start -= 250;
	}	
	
	for (templ = 0; templ < 250 ; templ++)
	{
		printf("%02bx ",rs485_RcvrBufRing[start]);
		start++;
		if (start >= UR2_RX_BUF_SIZE)
		{
			start = 0;
		}	
	}
	printf("\n\r");
}
*/

#if ((PROJECT_USB_GENERIC_HID_ENABLE) && (SYSTEM_HARDWARE_PS2_ENABLE))
/*----------------------------------------------------------------------------
 * voi ExtenderR_PS2_DataTransfer(U8_T intt_id,U8_T devinx,U8_T len)
 * Purpose : Process HC interrupt tranfer IN buffer
 * Params  : none
 * Returns : none
 * Note    : none
 *----------------------------------------------------------------------------
 */ 
void ExtenderR_PS2_DataTransfer(U8_T intt_id,U8_T *data_buf,U8_T data_len)
{
	U8_T	remain_siz; 
	U8_T	wp,databyte;	
	U8_T	*p,*buf;
	idata U8_T	rp;
	U8_T	index;	
	U16_T   len;
	U8_T    max_cnt;
#if (KVM_EXTENDER_HID_SHORT_PACKET)	
	Extender_Packet_Int_Header_Def  *header;
#endif		

	//1.Check Transmitter plugger status
	//intt_id--; //interface number subtract 1		
	if (ExtenderR_Check_Transmitter_Plug_State()==0)		
	{				
		ExtenderR_PS2_PassThrough[intt_id].State &= ~PASSTHROUGH_SET;
		ExtenderR_PS2_PassThrough[intt_id].Rp = 0;
	    ExtenderR_PS2_PassThrough[intt_id].Wp = 0;
		return;
	}

	//2.Move buffer out
//ExtenderR_PS2_DataTransfer_Restart:		
	wp = ExtenderR_PS2_PassThrough[intt_id].Wp;
	rp = ExtenderR_PS2_PassThrough[intt_id].Rp;
	//2_1.Check the buffer remain size(contain len bytes at least)
	if (wp == rp)
	{
		remain_siz = ExtenderR_PS2_PassThrough[intt_id].BufSize;
	}
	
	else if (wp > rp)
	{
		remain_siz = (ExtenderR_PS2_PassThrough[intt_id].BufSize-wp)+rp;
	}
	else
	{
		remain_siz = rp - wp;
	}
	//2_2.move out the data if the remain size is small than len
	if (remain_siz < (data_len+1))
	{
		//JUST ingoire it
		return;
		//remove the data byte
		//What is needed to be done? remove the first old one or skip the current?
		//ExtenderRUSBHC_Remove_HID_Buffer(intt_id);
		//rp = ExtenderR_PS2_PassThrough[intt_id].Rp;	
		/*
		length = ExtenderR_PS2_PassThrough[intt_id].Buf[rp]+1;
		if (ExtenderR_PS2_PassThrough[intt_id].Wp > ExtenderR_PS2_PassThrough[intt_id].Rp)
		{
			ExtenderR_PS2_PassThrough[intt_id].Rp += (length);
		}
		else
		{
			//Get the end of this buffer
			newrp = ExtenderR_PS2_PassThrough[intt_id].BufSize-ExtenderR_PS2_PassThrough[intt_id].Rp;
			//Get hte data package end position from the index 0
			if ((length) > newrp) //if length is large than end postion
			{
				ExtenderR_PS2_PassThrough[intt_id].Rp = (length) - newrp;
			}
			else
			{
				ExtenderR_PS2_PassThrough[intt_id].Rp += (length);
				if (ExtenderR_PS2_PassThrough[intt_id].Rp >= ExtenderR_PS2_PassThrough[intt_id].BufSize)
					ExtenderR_PS2_PassThrough[intt_id].Rp -= ExtenderR_PS2_PassThrough[intt_id].BufSize;
			}
		}						
		goto ExtenderR_PS2_DataTransfer_Restart;
		*/
	}
	
	//2_3.move interrupt in buffer data into out queue buffer
	//ExtenderR_PS2_PassThrough[intt_id].Buf[wp] = len;
	//wp++;
	//if (wp >= ExtenderR_PS2_PassThrough[intt_id].BufSize)
	//	wp = 0;
	for (databyte =0; databyte < data_len ; databyte++)
	{
		ExtenderR_PS2_PassThrough[intt_id].Buf[wp] = data_buf[databyte];
		wp++;
		if (wp >= ExtenderR_PS2_PassThrough[intt_id].BufSize)
			wp = 0;
	}
	ExtenderR_PS2_PassThrough[intt_id].Wp = wp;
	
	//***************************************************
	// Check The HID buffer Task State
	// Data Stream Control
	//***************************************************

	//2.Check the In Transmitter condition			
	if (ExtenderR_PS2_PassThrough[intt_id].State & PASSTHROUGH_SET)
	{
		return;
	}					
	
	//3.Data transfer in processing
	ExtenderR_PS2_PassThrough[intt_id].State |= PASSTHROUGH_SET;

	//4.Data transmit
#if (KVM_EXTENDER_HID_SHORT_PACKET)		
	p = ExtenderR_PS2_ExtenderR[intt_id]+sizeof(Extender_Packet_Int_Header_Def);	
#else
	p = ExtenderR_PS2_ExtenderR[intt_id]+sizeof(Extender_Packet_Header_Def);	
#endif	
	rp = ExtenderR_PS2_PassThrough[intt_id].Rp;
	wp = ExtenderR_PS2_PassThrough[intt_id].Wp;
	
	if (intt_id == EXTENDER_PS2_KEYBOARD)
		max_cnt = PS2_KB_TRANS_BUF_MAX_CNT;
	else
		max_cnt = PS2_MS_TRANS_BUF_MAX_CNT;
		
	index = 0;	
    while (rp != wp)
    { 
    	p[index] =  ExtenderR_PS2_PassThrough[intt_id].Buf[rp];
		rp ++;
		if (rp >= ExtenderR_PS2_PassThrough[intt_id].BufSize)
			rp = 0;
		index++;
		if (index >= max_cnt)
			break;			
	}
	ExtenderR_PS2_PassThrough[intt_id].Rp = rp; // update the RP pointer
	
	p = ExtenderR_PS2_ExtenderR[intt_id];
		
#if (KVM_EXTENDER_HID_SHORT_PACKET)	
	len = index+sizeof(Extender_Packet_Int_Header_Def)+2;
	header = (Extender_Packet_Int_Header_Def *) p;
#else	
	len = index+sizeof(Extender_Packet_Header_Def)+2;
#endif	

	buf = p+sizeof(Extender_Cmd_Token_Def);

#if (KVM_EXTENDER_HID_SHORT_PACKET)		
	Externder_Packet_Header((Extender_Packet_Header_Def *)p,Recevier_Int_Token);		 
	header->Cmd_Token.bCmd_CRC = data_len;				 	
	databyte = USBDC_VHID_DEVINX | (intt_id << 4);
	Extender_Packet_Int_Data((Extender_Data_Int_Packet_Def *)buf,index,NULL,databyte,intt_id);						
#else
	databyte = USBDC_VHID_DEVINX | (intt_id << 4);
	Externder_Packet_Header((Extender_Packet_Header_Def *) p,Recevier_Cmd_Token);
	Extender_Packet_Data((Extender_Data_Packet_Def *)buf,index,NULL,EXTENDER_KVM_PS2_DATA,databyte,intt_id);
#endif	
	//Check if there is a data packet in queue?		 
	//Externder_Packet_Header((Extender_Packet_Header_Def *) data_buf,Recevier_Cmd_Token);
	//Extender_Packet_Data((Extender_Data_Packet_Def *)buf,data_len,NULL,EXTENDER_KVM_PS2_DATA,USBDC_VHID_DEVINX,intt_id);

#if (KVM_EXTENDER_HID_NO_ACK)		
	Extender_Token_Transmit(p,EXTENDER_CONTROL_NONE_FREE|EXTENDER_CONTROL_TRANSMIT_NO_WAIT,len,100,1);
#else
	Extender_Token_Transmit(p,EXTENDER_CONTROL_NONE_FREE,len,100,1);
#endif		
	
#if (VIRTUAL_USB_HID_DEBUG)
	datainq_flag = 1;
	if (datainq_flag) // content has changed
	{
		printf("HIDQ=[ ");
		for (index=0; index < length ; index++)
		{
			printf("%02x ",(U16_T)p[index]);
		}
		printf("]\n\r");
	}
#endif /* #if (VIRTUAL_USB_HID_DEBUG) */
}
#endif

/*----------------------------------------------------------------------------
 * void ExtenderR_USB_Interval_Handle(U8_T index,U8_T value)
 * Purpose : Change the host interrupt transfer interval
 * Params  : none
 * Returns : none
 * Note    : none
 *----------------------------------------------------------------------------
 */ 
void ExtenderR_USB_Interval_Handle(U8_T index,U8_T value)
{
	U8_T devinx;
	U8_T endpinx;
	
	devinx = index & 0x0f;
	endpinx = (index & 0xf0) >> 4;
	
	USBHC_CORE_Change_IntTransfer_Interval(devinx,endpinx,value);
	
}

#if (SYSTEM_EXTENDER_MSC_SUPPORT) 
/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Transmit_MSC_Out_Ready(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Transmit_MSC_Out_Ready(void)
{	
	ExtenderR_Transmitter_Data_Send(NO_CTRL,EXTENDER_USB_MSC_DATA_OUT,NO_INDEX,burstout_cnt,NO_LEN,NO_DATA);
	burstout_cnt++;
}

#endif /* #if (SYSTEM_EXTENDER_MSC_SUPPORT) */

#ifdef PLUG_DETECT
/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Transmit_MSC_Out_Ready(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void TASK_Receiver_PlugDetect(void)
{	
	U8_T  RS45_plug;
	U8_T  HDMI_plug;
	
	//1.Check the RJ45 Plug		
	RS45_plug = RJ45_PLUG;
	HDMI_plug = HDIM_HOTPLUG;
	//printf("(%bu,%bu)",RS45_plug,HDMI_plug);
	if (RS45_plug == RJ45_PLUG_OFF)	 // Plug Off
	{
		if (RJ45_PLUG_SATE != PLUG_OFF)
		{	
			if (RJ45_Connect_Cnt <= 5) //need to detect contineu for 5 times
			{	
				RJ45_Connect_Cnt++;
			}
			else
			{		
				printf("RJ45 Plug off,quiet start\n\r");				
				RJ45_PLUG_SATE = PLUG_OFF;
				UART_Disable_Flag |= UART_RJ45_DISABLE_MASK;
				ExtenderR_Check_Uart_Disable();
			}	
		}	
	}
	else
	{
		RJ45_Connect_Cnt = 0;
		if (RJ45_PLUG_SATE == PLUG_OFF)
		{
			printf("RJ45 Plug In,quiet start\n\r");
			RJ45_PLUG_SATE = PLUG_IN;					
			UART_Disable_Flag |= UART_RJ45_DISABLE_MASK;
			ExtenderR_Check_Uart_Disable();
		}			
	}
	
	//2.Check the HDMI HOTPLUG
	if (HDMI_plug == HDMI_PLUG_OFF)	
	{
		if (HDMI_PLUG_SATE == PLUG_IN)
		{	
			printf("HDMI Plug off,quiet start\n\r");
			HDMI_PLUG_SATE = PLUG_OFF;
			UART_Disable_Flag |= UART_HDMI_DISABLE_MASK;
			ExtenderR_Check_Uart_Disable();
		}	
	}
	else
	{
		if (HDMI_PLUG_SATE == PLUG_OFF)
		{
			printf("HDMI Plug In,quiet start\n\r");
			HDMI_PLUG_SATE = PLUG_IN;			
			UART_Disable_Flag |= UART_HDMI_DISABLE_MASK;
			ExtenderR_Check_Uart_Disable();
		}	
		
	}		
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderR_Transmit_MSC_Out_Ready(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Check_Uart_Disable(void)
{
	if (TASK_ExtenderR_Uart_Disable_ActiveID ==0)
	{	
		TASK_ExtenderR_Uart_Disable_ActiveID = TASK_Run(&TASK_ExtenderR_UartDisable_TASK);
	}
	else
	{
		Task_Active_Table[TASK_ExtenderR_Uart_Disable_ActiveID].Task_Interval.w = UART_DISABLE_TIME;
	}		
}
/*----------------------------------------------------------------------------
 * Function Name: TASK_ExtenderR_Uart_Disable(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void TASK_ExtenderR_Uart_Disable(void)
{
	//printf("Quiet End\n\r");	
	UART_Disable_Flag = 0;
	TASK_Destory_Current();
	TASK_ExtenderR_Uart_Disable_ActiveID = 0;
}	
#endif /* #ifdef PLUG_DETECT */

#if (ENABLE_HOTKEY_FUN_RTC_CONTROL)
/*----------------------------------------------------------------------------
 * Function Name: void ExtenderR_Transmit_RTC_Contorl(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderR_Transmit_RTC_Contorl(void)
{
	printf("RTC Send:%02bx\n\r",KVM_Flash.cSystemFlag2);	
	ExtenderR_Transmitter_Data_Send(NO_CTRL,EXTENDER_RTC_CONTORL,NO_INDEX,KVM_Flash.cSystemFlag2,NO_LEN,NO_DATA);
	KVM_RTC_Control();		
}
#endif

#endif /* SYSTEM_EXTENDER_RECEIVER */
/* End of extender_receiver.c */
