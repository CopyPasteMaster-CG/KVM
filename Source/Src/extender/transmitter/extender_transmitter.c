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
 * Module Name: extender_transmitter.c
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


#if (SYSTEM_EXTENDER_TRANSMITTER)
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
bit	Tansmitter_PassThrough_Buf_In_Used_Flag;
bit Remote_Link_LED_Flash_Flag;
bit Remote_Link_LED_Flash_State;
bit Remote_Link_LED_State;
bit Transmitter_MSC_Bulk_Out_Flag=0;
bit	Transmitter_MSC_In_Wait_Flag=0;
bit EXTENDER_MSC_BurstOutWait_Clear_Flag=0;
bit Transmitter_System_Set_Flag=0;
#ifdef  DEVICE_KEEP
bit     ExtenderT_Device_Keep_Flag=0;
#endif
#ifdef DEVICE_KEEP
#define ExtenderT_Start_Mask			0x01

U8_T	ExtenderT_Control_Flag = 0;
U8_T    ExtenderT_Report_Restart_Cnt = 0;
U8_T	ExtenderT_Device_Mount_Change;
Extender_Device_Keep_Def	DeviceKeepTable[USB_HC_MAX_DEVICE-1];
#endif 
U8_T Tansmitter_PassThrough_Buf_In_Used_Devinx;
U8_T Receiver_Plug_Off_Cnt=0;
#if (SYSTEM_EXTENDER_MSC_SUPPORT)
#if (!SYSTEM_EXTENDER_RECEIVER)
//Extender_Data_Packet_Def	*pToken[EXTENDER_MSC_BUF_CNT];
#endif /* #if (!SYSTEM_EXTENDER_RECEIVER) */
#endif /* if (SYSTEM_EXTENDER_MSC_SUPPORT) */
/* GLOBAL VARIABLES DECLARATIONS */
U8_T Transmitter_Ack_Token[]=
{
	EXTENDER_ACK_TOKEN,
	EXTENDER_RECEIVER_ADDR,
	0x00,
};

U8_T Transmitter_Stall_Token[]=
{
	EXTENDER_STALL_TOKEN,
	EXTENDER_RECEIVER_ADDR,
	0x00,
};

U8_T Transmitter_Nak_Token[]=
{
	EXTENDER_NAK_TOKEN,
	EXTENDER_RECEIVER_ADDR,
	0x00,
};

U8_T	Transmitter_Control_State;
U8_T	Transmitter_Send_State;
U8_T	*Trnasmitter_Send_Buf;
U8_T	Trnasmitter_EDID[EDID_MAX];
U8_T	Transmitter_Plug_Off_Cnt;
//U8_T	Transmitter_Wait_Answer_State;
U8_T	ExtenderT_Device_USB_Cmd_Send_State;

Extender_Device_Mapping_Def		Remote_DevMap[USBDC_DEVICE_MAX];
U8_T	Remote_Reserve_DevMap[USBDC_DEVICE_MAX];
U8_T	TASK_USBHC_ExtenderT_PassThrough_Handle_ID;
U8_T	Data_End_Token[sizeof(Extender_Data_Packet_Def)+2] =
		{
			EXTENDER_DATA0_TOKEN,  //Token
			0x00,                  //Len MSB
			0x00,                  //Len MSB
			EXTENDER_DATA_END,     //bRequest
			0x00,                  //bIndex
			0x00,                  //bValue
			0x00,                  //CRC0
			0x00,                  //CRC1
		};

U8_T	ExtenderT_Device_Passthrough_Flag; //each bit standard 1 device
U8_T	ExtenderT_Device_Passthough_State[USBDC_DEVICE_MAX];
U8_T	TASK_ExtenderT_Connection_Check_ActiveID;
U8_T	TASK_Receiver_RemoteLed_Control_ActiveID;
U8_T	TASK_ExtenderR_Actived_Led_Flash_Active_ID;
U8_T	ExtenderT_Cmd_Q_Inx;
ExtenderT_Cmd_Q_Typedef		ExtenderT_Cmd_Q[EXTENDERT_CMD_Q_MAX];
Extender_Data_Packet_Def	*Trnasmitter_Send_Data_TokenP;
Extender_Cmd_Handle_Def		Extender_Setup;
Extender_Data_Packet_Def	Extender_Data_Wait;
U8_T						*EXTENDER_MSC_BulkInWait[EXTENDER_MSC_BulkInWaitMax];
U8_T						EXTENDER_MSC_BulkInWaitHead = 0;
U8_T						EXTENDER_MSC_BulkInWaitTail = 0;
U16_T						start_hold;

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
U8_T	ETDR_AudioOutBuf[EXTENDER_AUDIO_OUT_BUF_CNT][EXTENDER_AUDIO_OUT_BUF_SIZE] _at_ 0x5000;
U8_T	ETDR_AudioOutHead = 0;
U8_T	ETDR_AudioOutTail = 0;
U8_T	ETDR_AudioInBuf[EXTENDER_AUDIO_IN_BUF_CNT][EXTENDER_AUDIO_IN_BUF_SIZE] _at_ 0x5800;
U8_T	ETDR_AudioInHead = 0;
U8_T	ETDR_AudioInTail = 0;
U8_T	ETDR_AudioOutTailSkipFlag = 0;
U8_T	ETDR_AudioOutBufAddr[EXTENDER_AUDIO_OUT_BUF_CNT][2];
U8_T	ETDR_AudioInDcDoingFlag = 0;
U8_T	ETDR_AudioInBufAddr[EXTENDER_AUDIO_IN_BUF_CNT][2];
static bit etdrT_isr;
U8_T	burstout_cnt;
#endif

#ifdef USB_2
U8_T  Device_Host_Mount[KVM_MAX_PORT];
#endif /* #ifdef USB_2 */

#ifdef PLUG_DETECT
#define RJ45_PLUG_OFF			1
#define RJ45_PLUG_IN			0
#define HDMI_PLUG_OFF			0
#define HDMI_PLUG_IN			1

U8_T    UART_Disable_Flag = 0;
#ifdef RJ45_PLUG_DETECT
U8_T	RJ45_PLUG_SATE 		= PLUG_OFF;
U8_T	RJ45_Connect_Cnt=0;
U8_T	RJ45_UART_QuietCnt;
#endif /* #ifdef RJ45_PLUG_DETECT */
U8_T	HDMI_PLUG_SATE 		= PLUG_OFF;
U8_T	HDMI_UART_QuietCnt;
U8_T	TASK_ExtenderT_PlugDetect_ActiveID;
U8_T	TASK_ExtenderT_Uart_Disable_ActiveID=0;
#endif
#ifdef  DEVICE_KEEP
Extender_Device_Keep_Def	DeviceKeepTable[USB_HC_MAX_DEVICE-1];
#endif /* #ifdef  DEVICE_KEEP */

TASK_ActiveTable_TypeDef ExtenderT_Connection_Check_TASK=
{
	TASK_TYPE_INTERVAL_MS,
	0,	// Task ID from 0~255
	0,	// Task Wait for Semaphore to active
	0,	// Task Event
	0,	// Task transfer parameter
#if (HUART_BAUD <= HUART_4M)	
	100,// Task Interval time from
	100,// Task Interval Reload
#else	
	#if (HUART_BAUD <= HUART_921K)		
	200,// Task Interval time from
	200,// Task Interval Reload
	#else
	300,// Task Interval time from
	300,// Task Interval Reload
	#endif
#endif		
//	0,	// Task wait for period then perform
};

TASK_ActiveTable_TypeDef ExtenderT_Plug_Off_Handle_TASK=
{
	TASK_TYPE_EVENT,
	0,	// Task ID from 0~255
	0,	// Task Wait for Semaphore to active
	0,	// Task Event
	0,	// Task transfer parameter
	0,	// Task Interval time from
	0,	// Task Interval Reload
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
TASK_ActiveTable_TypeDef TASK_ExtenderT_PlugDetect_TASK=
{
	TASK_TYPE_INTERVAL_MS,
	0,	// Task ID from 0~255
	0,	// Task Wait for Semaphore to active
	0,	// Task Event
	0,	// Task transfer parameter
	40,	// Task Interval time from
	40,	// Task Interval Reload
};

TASK_ActiveTable_TypeDef TASK_ExtenderT_UartDisable_TASK=
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

/* LOCAL VARIABLES DECLARATIONS */
//static U8_T etdr_T_485FirstFlag = 5
static U8_T etdr_T_MountSkipFlag = 0;
//static U8_T etdr_T_DescriptorSkipFlag = 0;
static U8_T etdr_T_WaitRestartAckFlag = 1;

/* LOCAL SUBPROGRAM DECLARATIONS */
void Extender_Transmitter_Command_Token_Handle(void);
void Extender_Transmitter_EDID_Handle(Extender_Cmd_Handle_Def *setup);
void TRANSMITTER_Restart_Report(void);
void TRANSMITTER_WaitEvent_Ack_Handle(void);
void Transmitter_Receiver_Restart_Handle(void);
U8_T Transmitter_Receiver_Mount_Handle(Extender_Cmd_Handle_Def *setup);
void Transmitter_Receiver_Unmount_Handle(Extender_Cmd_Handle_Def *setup);
void Transmitter_Receiver_Device_Descriptor_Handle(Extender_Cmd_Handle_Def *setup);
void TRANSMITTER_Transmit_End(void);
void TASK_USBHC_ExtenderT_PassThrough(void);
void ExtenderT_Setup_Status_Stage(U8_T devinx,U8_T direction);
void ExtenderT_Setup_Token_Stage(U8_T devinx);
void ExtenderT_Setup_Data_Out_Stage(U8_T devinx);
RESULT USBHC_ExtenderT_PassThrough_Setup_Token_Parser(U8_T devinx);
void ExtenderT_Passthrough_Done_Check(U8_T devinx);
void ExtenderT_Setup_Data_In_Enable(U8_T devinx);
void TASK_Extender_Transmitter_Plug_Check(void);
void TASK_ExtenderT_Plug_Off_Handle(void);
void ExtenderT_Clean_USB_Device(void);
void ExtenderT_Virtual_Device_Suspend_Check(void);
void TASK_Receiver_RemoteLed_Control(void);
void TASK_ExtenderR_Actived_Led_Flash(void);
void ExtenderT_Intr_Data_In_Handle(void);
void ExtenderT_KVM_KB_Led_Send(void);
void ExtenderT_SOF_Token_Received_Handle(void);
void TRANSMITTER_Receive_Parser_State_Reset(void);
void ExtenderT_Send_Command_Q(U8_T index);
void Extender_Transmitter_Port_Switch_Handle(U8_T newport);
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
void ExtenderT_Audio_Data_Out_Stage(U8_T devinx);
void ExtenderT_Audio_Data_In_Stage(void);
#endif
void TASK_USBHC_ExtenderT_PassThrough_Process(U8_T devinx);
void ExtenderT_PS2_Data_Handle(void);
void TASK_Transmitter_PlugDetect(void);
void TASK_ExtenderT_Uart_Disable(void);
#ifdef PLUG_DETECT
void TASK_Transmitter_PlugDetect(void);
void TASK_ExtenderT_Uart_Disable(void);
void ExtenderT_Check_Uart_Disable(void);
#endif

/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */

/* EXTERNAL SUBPROGRAM DECLARATIONS */

/*----------------------------------------------------------------------------
 * Function Name: EXTENDER_Transmitter_Init
 * Purpose: initial the extender transmitter
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void EXTENDER_Transmitter_Init(void)
{
	Transmitter_Control_State = 0x00;
	Transmitter_Send_State = 0;

	memset((U8_T *)&Remote_DevMap,0x00,sizeof(Remote_DevMap));
	memset(Remote_Reserve_DevMap ,0x00,sizeof(Remote_Reserve_DevMap));
	memset(ExtenderT_Device_Passthough_State,0x00,sizeof(ExtenderT_Device_Passthough_State));

	/* Link the Transmitter Virtual HUB with remote Root HUB */
	Remote_DevMap[USBDC_VHUB_DEVINX].Mapping   = 0x00;
	Remote_DevMap[USBDC_VHUB_DEVINX].UpperInfo = USBDC_VHUB_PORT_NUM; //hub port counter 7
	Remote_Reserve_DevMap[USBDC_VHUB_DEVINX]   = USBDC_VHUB_DEVINX;  //the first is transmitter virutal root hub

	Transmitter_Control_State |= TRANSMITTER_KB_LED_FLAG;
	Transmitter_Send_State |= TRANSMITTER_SEND_WAIT; //send restart event to receiver

	TASK_USBHC_ExtenderT_PassThrough_Handle_ID		= TASK_Create(TASK_USBHC_ExtenderT_PassThrough);
	ExtenderT_Connection_Check_TASK.Task_ID			= TASK_Create(TASK_Extender_Transmitter_Plug_Check);
#ifndef PWAYTEK_01
	TASK_Receiver_RemoteLed_Control_TASK.Task_ID	= TASK_Create(TASK_Receiver_RemoteLed_Control);
#endif

#if (EXTENDER_LED_FLASH_INDICATE)	
	TASK_ExtenderR_Actived_Led_Flash_TASK.Task_ID	= TASK_Create(TASK_ExtenderR_Actived_Led_Flash);
#endif	

#ifdef PLUG_DETECT
	TASK_ExtenderT_PlugDetect_TASK.Task_ID			= TASK_Create(TASK_Transmitter_PlugDetect);
	TASK_ExtenderT_UartDisable_TASK.Task_ID			= TASK_Create(TASK_ExtenderT_Uart_Disable);
#endif

#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("TASK_USBHC_ExtenderT_PassThrough_Handle_ID=%d\n\r",(U16_T)TASK_USBHC_ExtenderT_PassThrough_Handle_ID);
	printf("ExtenderT_Connection_Check_TASK=%d\n\r",(U16_T)ExtenderT_Connection_Check_TASK.Task_ID);
	printf("TASK_Receiver_RemoteLed_Control_TASK=%d\n\r",(U16_T)TASK_Receiver_RemoteLed_Control_TASK.Task_ID);
#if (EXTENDER_LED_FLASH_INDICATE)		
	printf("TASK_ExtenderR_Actived_Led_Flash_TASK=%d\n\r",(U16_T)TASK_ExtenderR_Actived_Led_Flash_TASK.Task_ID);
#endif	
#endif

	TASK_ExtenderT_Connection_Check_ActiveID	= TASK_Run(&ExtenderT_Connection_Check_TASK);

#ifdef PLUG_DETECT
	TASK_ExtenderT_PlugDetect_ActiveID = TASK_Run(&TASK_ExtenderT_PlugDetect_TASK);
#endif	
	
	
#ifndef PWAYTEK_01
	TASK_Receiver_RemoteLed_Control_ActiveID	= TASK_Run(&TASK_Receiver_RemoteLed_Control_TASK);
#endif

#if (EXTENDER_LED_FLASH_INDICATE)	
	TASK_ExtenderR_Actived_Led_Flash_Active_ID	= TASK_Run(&TASK_ExtenderR_Actived_Led_Flash_TASK);
#endif	
	Task_Active_Table[TASK_ExtenderR_Actived_Led_Flash_Active_ID].Task_Event |= TASK_EVENT_SKIP;

	Tansmitter_PassThrough_Buf_In_Used_Flag = 0;
	Tansmitter_PassThrough_Buf_In_Used_Devinx = 0;
	Remote_Link_LED_Flash_Flag = 0;
	Remote_Link_LED_Flash_State= 0;
	Remote_Link_LED_State = 0;
//	Transmitter_Wait_Answer_State = 0;
	ExtenderT_Device_USB_Cmd_Send_State = 0;
	memset(KVM_HostLed,0x00,sizeof(KVM_HostLed));
	ExtenderT_Cmd_Q_Inx = 0;

#ifdef USB_2
	memset(Device_Host_Mount,0x00,sizeof(Device_Host_Mount));
#endif /* #ifdef USB_2 */

#if (PROJECT_USB_GENERIC_HID_ENABLE)
	Remote_Reserve_DevMap[USBDC_VHID_DEVINX] = USBDC_VHID_DEVINX;
#endif
}

/*----------------------------------------------------------------------------
 * Function Name: EXTENDER_Transmitter_Receive_Handle
 * Purpose: The function for handle the data send from receiver
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void EXTENDER_Transmitter_Receive_Handle(void)
{
	U8_T	token_found;
	//U16_T   index;

	start_hold = Receive_Buffer_Start;
	EXTENDER_Get_Total_Receive_Len();
	/*
	if (etdr_T_485FirstFlag > 2)
	{
		EXTENDER_Update_Receive_Index(Receive_Buffer_Cnt);
		Receive_Buffer_Cnt = 0;
		etdr_T_485FirstFlag--;
		return;
	}
	else if (etdr_T_485FirstFlag)
	{
		etdr_T_485FirstFlag--;
	}
	*/
	if (Receive_Buffer_Cnt >= 2000)
	{
		printf("R:%d\n\r",Receive_Buffer_Cnt);
	}


	Remote_Link_LED_Flash_Flag = 1;
	Task_Active_Table[TASK_ExtenderR_Actived_Led_Flash_Active_ID].Task_Event &= ~TASK_EVENT_SKIP; //stop disconnect led flashing

Transmitter_Receive_Handle_Restart:
	//1.The first trnasaction mnust be a valid token header(3 bytes,CMD+ADDR+CRC)
	if (EXTENDER_Multi_TR_State & EXTENDER_RECEIVE_START) //if command has been received
	{
		token_found = 1;
	}
	else
	{
		if (Receive_Buffer_Cnt < sizeof(Extender_Cmd_Token_Def))
		{
			//printf("TS_CNT=%d\n\r",(U16_T)Receive_Buffer_Cnt);
			//EXTENDER_Update_Receive_Index(Receive_Buffer_Cnt);
			//Receive_Buffer_Cnt = 0;
			return;
		}

		token_found = Extender_Receive_Cmd_Token_Check(Receive_Buffer_Cnt,&Receive_Cmd_Token,EXTENDER_TRANSMITTER_ADDR);
	}

	if (token_found == 0)
	{
		/* For GUNLOT Porject       */
		/*
		printf("UART_DEBUG:[ ");
			for (index = 0; index < Receive_Buffer_Cnt; index++)
			{
				printf("%02bx ",rs485_RcvrBufRing[Receive_Buffer_Start+index]);
			}
			printf("\n\r");
		*/
		/*------------------------- */
		return;
	}    

	if ((EXTENDER_Multi_TR_State & EXTENDER_RECEIVE_START) == 0x00)
	{
		memset((U8_T *)&Extender_Setup,0x00,sizeof(Extender_Setup));
		EXTENDER_Multi_TR_State |= (EXTENDER_RECEIVE_WAIT|EXTENDER_RECEIVE_START);  /* Wait for end token */
		memcpy((U8_T *)&Extender_Setup.Cmd_Token,(U8_T *)&Receive_Cmd_Token,sizeof(Extender_Cmd_Token_Def));
		if (Receive_Cmd_Token.bCmd_Token == EXTENDER_INT_TOKEN)
			Extender_Setup.iRemain_Len = sizeof(Extender_Data_Int_Packet_Def)+2;
		else
			Extender_Setup.iRemain_Len = sizeof(Extender_Data_Packet_Def)+2;
		Extender_Receive_Data_Parser_State = 0;
	}

	Transmitter_Control_State |= TRANSMITTER_CONNECT_FLAG;
#ifdef PWAYTEK_01
	RECEIVER_STATE_LED_GPIO = LED_ON;
#endif
	Transmitter_Plug_Off_Cnt = 0; //reset the plug check counter

	//2.Command Token handle
	switch (Receive_Cmd_Token.bCmd_Token)
	{
		case EXTENDER_INT_TOKEN:
		case EXTENDER_CMD_TOKEN:
			//printf("(R:%d)",Receive_Buffer_Cnt);
			if (Receive_Buffer_Cnt)
			{
				Extender_Transmitter_Command_Token_Handle();
			}
			break;
		case EXTENDER_SOF_TOKEN: //need to answer ack or nak
			/*
			if (Transmitter_Restart_End_Flag == 0)
			{				
				Tansmitter_Restart_Cnt++;
				if (Tansmitter_Restart_Cnt > 250)
				{
					Transmitter_Restart_End_Flag = 1;
				}	
			}
			else
			*/
			{	
				ExtenderT_SOF_Token_Received_Handle();
			}	
			break;
		case EXTENDER_ACK_TOKEN:
/*			
#if (SYSTEM_EXTENDER_MSC_SUPPORT)
			if (EXTENDER_MSC_BurstWait)
			{
				EXTENDER_MSC_BurstWait = 0;
			}
#endif
*/
		default:
			Extender_Receive_Parser_State_Reset();
			break;
	}

	if ((Extender_Receive_Cmd_Parser_State & EXTENDER_CMD_TOKEN_LEN_SHORT) ||
		(Extender_Receive_Data_Parser_State & EXTENDER_DATA_PACKET_LEN_SHORT))
	{
		// cmd length not enough
		Extender_Receive_Start_TimeOut_Check();
		return;
	}

	Extender_Receive_Stop_TimeOut_Check();

	if (Receive_Buffer_Cnt != 0)
	{
		//Current not finish
		goto Transmitter_Receive_Handle_Restart;
	}

	if (Extender_Control_State & EXTENDER_CONTROL_TRANSMIT_DONE)
	{
		Task_Active_Table[EXTENDER_Transmit_TimeOut_TaskActiveID].Task_Reload.w = 10000; // next for 1 seconds
		EXTENDER_Transmit_Update();
	}
}

/*----------------------------------------------------------------------------
 * Function Name: Extender_Transmitter_Command_Token_Handle
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void Extender_Transmitter_Command_Token_Handle(void)
{
	U8_T	*answer;
	bit		ack_answer=1;
	U8_T	devinx;
	U8_T	remote_devinx;
	U8_T    ack_skip=0;

	if (Extender_Receive_Data_Token_Parser(&Extender_Setup))
	{
		//get remote device index,then change inot local device index
		if (Extender_Receive_Data_Parser_State & EXTENDER_DATA_PACKET_PARSER_ERR)
		{
			if ((Extender_Receive_Data_Parser_State & EXTENDER_DATA_PACKET_CRC_OK) == 0x00)
			{
				printf("CRC Check Sum Error");
			}
		}

		if (Extender_Setup.Cmd_Token.bCmd_Token == EXTENDER_INT_TOKEN)
		{
			ExtenderT_Intr_Data_In_Handle();
#if (KVM_EXTENDER_HID_NO_ACK)
			ack_skip=1;
#endif
		}
		else
		{
			//printf("[%03bu][%d]",Extender_Setup.Data_Token.bTid,Receive_Buffer_Start);
			remote_devinx = Extender_Setup.Data_Token.bIndex;
			devinx = Remote_Reserve_DevMap[remote_devinx];

			if ((Extender_Setup.Data_Token.bRequest == EXTENDER_USB_MSC_DATA_IN) ||
				(Extender_Setup.Data_Token.bRequest ==EXTENDER_USB_MSC_CSW))
			{
#if (SYSTEM_MSC_DEVICE_SUPPORT)
				ExtenderT_MSC_BurstDataInHandle(&Extender_Setup);
#endif
			}
			else if (Extender_Setup.Data_Token.bRequest == EXTENDER_USB_ISO_IN)
			{
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
				ExtenderT_Audio_Data_In_Stage();
				ack_skip = 1;
#endif
			}
			else
			{
				switch(Extender_Setup.Data_Token.bRequest)
				{
#ifdef DEVICE_KEEP
					/*
					case EXTENDER_USB_VPID:
						if (ExtenderT_Device_Keep_Flag)
						{
							Transmitter_Check_Remote_NewDevice(&Extender_Setup);								
						}
						break;
					*/
					case EXTENDER_T_DEVICE_TABLE:							
						Transmitter_Check_Device_Table(&Extender_Setup);
						break;
#ifdef RTC					
					case EXTENDER_RTC_CONTORL:							
						KVM_Flash.cSystemFlag2 =  Extender_Setup.Data_Token.bValue;
						KVM_RTC_Control();
						break;
#endif					
					/*
					case EXTENDER_T_DEVICE_LIST:
						if (Extender_Setup.Data_Token.bIndex)
						{	
							printf("DEVICE Keep Start\n\r");
							ExtenderT_Device_Keep_Flag = 1;
							Transmitter_Clear_DeviceKeep_Flag();
						}		
						else
						{	
							printf("DEVICE Keep End\n\r");
							ExtenderT_Device_Keep_Flag = 0;
							Transmitter_Clear_Old_DeviceKeep();
						}
						break;
					*/
#endif					
					case EXTENDER_R_MSC_REDEVINX:
#if (SYSTEM_MSC_DEVICE_SUPPORT)
						//olddevinx = Extender_Setup.Data_Token.bIndex;
						Remote_DevMap[USBHC_MSC_Devinx].Mapping = (USBHC_DEVICE_USED_MASK|Extender_Setup.Data_Token.bValue);
						USBHC_MSC_VHUB_ResetUpdate();
						//ExtenderT_MSC_VHUB_ResetUpdate();
#endif
						break;
					case EXTENDER_R_MSC_RESET_ENABLE:
	#if (SYSTEM_MSC_DEVICE_SUPPORT)
						USBHC_MSC_VHUB_ResetUpdate();
						//ExtenderT_MSC_VHUB_ResetUpdate();
#endif
						break;
					case EXTENDER_USB_INTR_IN:
						ExtenderT_Intr_Data_In_Handle();
#if (KVM_EXTENDER_HID_NO_ACK)
						ack_skip=1;
#endif
						break;
					case EXTENDER_KVM_EDID:
						Extender_Transmitter_EDID_Handle(&Extender_Setup);
						break;
					case EXTENDER_R_RESTART:
						etdr_T_WaitRestartAckFlag = 0;
						Transmitter_Receiver_Restart_Handle();
						break;
					case EXTENDER_USB_REMOUNT:
					case EXTENDER_USB_MOUNT:
						//printf("MountCommand:%02bx\n\r",Extender_Setup.Data_Token.bRequest);
						ack_answer = Transmitter_Receiver_Mount_Handle(&Extender_Setup);
						break;
					case EXTENDER_USB_UNMOUNT:
						Transmitter_Receiver_Unmount_Handle(&Extender_Setup);
						break;
					case EXTENDER_USB_DESCRIPTOR:
						//if (!etdr_T_DescriptorSkipFlag)
						//{
							Transmitter_Receiver_Device_Descriptor_Handle(&Extender_Setup);
						//}
						break;
					case EXTENDER_USB_DATA_OUT_STAGE:
						if (USB_PDevice[devinx].Hc.Control.State == CTRL_DATA_OUT_WAIT) //This should put into Transmitter Receiver Handle first
						{
							TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_ExtenderT_PassThrough_Handle_ID,CTRL_DATA_OUT,devinx,0,0); //Generate the task, for nex
						}
						else
						{
							printf("  !!! STATE No Excepted\n\r");
						}
						ack_skip=1;
						break;
					case EXTENDER_USB_DATA_IN_STAGE:
						if (USB_PDevice[devinx].Hc.PassThrough_State == PASSTHROUGH_HC_SETUP_DATA_IN)
						{
							if (USB_PDevice[devinx].Hc.Control.State == CTRL_DATA_IN) //This should put into Transmitter Receiver Handle first
							{
								//DC are waiting for data receive
								//DC buffer avaiable ....							
								USB_PDevice[devinx].Hc.PControl.Total_Length = Extender_Setup.Data_Token.iLen;
								USB_PDevice[devinx].Hc.PControl.Current_Length = 0; // new page start							
								if (USB_PDevice[devinx].Hc.PControl.Total_Length)
								{
									Extender_CopyDataFromRxBuffer(Extender_Setup.iRecBuf_Index,USB_PDevice[devinx].Hc.PControl.Total_Length,USB_PDevice[devinx].Hc.PControl.Buf);
								}
	
								/* Send Data Page to DC */
								USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_DC_SETUP_DATA_IN; //Next is DC data in
								TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_ExtenderT_PassThrough_Handle_ID,CTRL_DATA_IN,devinx,0,0); //Generate the task, for next
							}
							else
							{
								ack_answer = 0;
								printf("total_length=%d\n\r",(U16_T)USB_PDevice[devinx].Hc.PControl.Total_Length);
								printf("Setup linkage broken ...(%02x)\n\r",(U16_T)USB_PDevice[devinx].Hc.Control.State);
								printf("(%02x)",(U16_T)USB_PDevice[devinx].Hc.Control.State);
							}
						}
						else
						{
							printf(" <T>:Devinx:%d,Transmitter not Pass-Through DataIN mode(%02x)\n\r",(U16_T)devinx,
							(U16_T)USB_PDevice[devinx].Hc.PassThrough_State);
						}
						break;
					case EXTENDER_USB_SETUP_DONE: //answer IN or OUT to DC
#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)
						printf(" <T>->[PASSTHROUGH_SETUP_DONE]\n\r");
#endif /*#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)*/
						ExtenderT_Device_Passthough_State[devinx] |= T_USB_PASSTHROUGH_REMOTE_DONE_FLAG;
						//printf("A(%bx-%bx)",USB_PDevice[devinx].Hc.Control.State,USB_PDevice[devinx].Hc.PassThrough_State);
						if ((USB_PDevice[devinx].Hc.Control.State == CTRL_DATA_OUT_WAIT) //This should put into Transmitter Receiver Handle first
							|| (USB_PDevice[devinx].Hc.PassThrough_State == PASSTHROUGH_HC_SETUP_STATUS_IN))
						{
							TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_ExtenderT_PassThrough_Handle_ID,CTRL_DATA_OUT,devinx,0,0); //Generate the task, for nex
						}
						else
						{
							ExtenderT_Passthrough_Done_Check(devinx);
						}
						ack_skip=1;
						break;
					case EXTENDER_USB_STATUS_STALL:
						ExtenderT_Device_Passthough_State[devinx] |= (T_USB_PASSTHROUGH_REMOTE_DONE_FLAG | T_USB_PASSTHROUGH_DC_DONE_FLAG);
						ExtenderT_Passthrough_Done_Check(devinx);
						USBDC_HAL_Endp_Stalled(USB_PDevice[devinx].Hc.PControl.UPID,devinx,0);
						break;
#if (SYSTEM_EXTENDER_MSC_SUPPORT)
					//case EXTENDER_USB_MSC_DATA_IN:
					//case EXTENDER_USB_MSC_CSW:
#if (SYSTEM_USB_HC_BURST)
						//ExtenderT_MSC_BurstDataInHandle(&Extender_Setup);
#else
						//ExtenderT_MSC_BulkDataInHandle(&Extender_Setup);
#endif //#if (SYSTEM_USB_HC_BURST)
						//break;
					case EXTENDER_USB_MSC_STALL:
						/* For STALL, the bValue is used to indicate the IN or OUT direction replied from the extender receiver */
						EXTENDER_MSC_StallFlag = Extender_Setup.Data_Token.bValue;
						ExtenderT_MSC_DcBulkInStallHandle(devinx);
						break;
#endif //#if (SYSTEM_EXTENDER_MSC_SUPPORT)

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
					//case EXTENDER_USB_ISO_IN:
					//	ExtenderT_Audio_Data_In_Stage();
					//	ack_skip = 1;
						//if (Receive_Buffer_Cnt)
						//{
							//printf("B:%d",Receive_Buffer_Cnt);
						//}
						//break;
#endif
					case EXTENDER_KVM_T_PORT_SW: //switch port
						Extender_Transmitter_Port_Switch_Handle(Extender_Setup.Data_Token.bValue);
						break;
					case EXTENDER_KVM_MSX_SIZE: //define the msc class page size					
						MSC_Class_Page_Size = (Extender_Setup.Data_Token.bIndex << 8) + Extender_Setup.Data_Token.bValue;
						printf("MSC Size=%d\n\r",MSC_Class_Page_Size);					
						break;
					case EXTENDER_KVM_T_SYSTEM: //switch port
						ExtenderT_System_Control(Extender_Setup.Data_Token.bValue);
						break;
					case EXTENDER_KVM_T_AUDIO_SW:
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
						ExtenderT_Audio_SW_Handle(Extender_Setup.Data_Token.bValue);
#endif
						break;
					case EXTENDER_USB_T_AUDIO_AUTOSW:
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
						ExtenderT_Audio_AutoSW_Handle(Extender_Setup.Data_Token.bIndex);
#endif
						break;
					case EXTENDER_USB_T_MSC_SW:
						ExtenderT_Msc_SW_Handle(Extender_Setup.Data_Token.bValue);
						break;
#if (SYSTEM_EXTENDER_MSC_SUPPORT)						
					case EXTENDER_USB_MSC_DATA_OUT:						
						ExtenderT_MSC_BurstOutComplete();
						break;	
#endif						
					default:
						ack_answer=0;
						break;
				}
			}
		}

		if (ack_answer)
		{
			if (Extender_Receive_Data_Parser_State & EXTENDER_DATA_PACKET_SECTION_OK)
			{
				if (ack_skip==0)
				{
					ExtenderT_SOF_Token_Received_Handle();
				}
				Extender_Receive_Parser_State_Reset();
				return;
			}
			else
			{
				printf("T:NAK(%02bx)\n\r",Extender_Setup.Data_Token.bRequest);				
				Extender_Receive_Parser_State_Reset();
				return;
/*				
#if (KVM_EXTENDER_HID_SHORT_PACKET)
				if (Extender_Setup.Data_Token.bRequest != EXTENDER_USB_INTR_IN)
				{
					answer = Transmitter_Nak_Token;
				}
#else
				answer = Transmitter_Nak_Token;
#endif
*/
			}
		}
		else
		{	
			//answer = Transmitter_Stall_Token;
			printf("STALL\n\r");
			Extender_Receive_Parser_State_Reset();
			return;
		}
		Extender_Receive_Parser_State_Reset();
		Extender_Token_Transmit(answer,(EXTENDER_CONTROL_TRANSMIT_NO_WAIT|EXTENDER_CONTROL_NONE_FREE),sizeof(Extender_Cmd_Token_Def),1000,1);
	}
}

/*----------------------------------------------------------------------------
 * Function Name: Extender_Transmitter_EDID_Handle
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void Extender_Transmitter_EDID_Handle(Extender_Cmd_Handle_Def *setup)
{
	//1.Copy edid table into Trnasmitter_EDID
#if (TRANSMITTER_CMD_DEBUG)
	printf(" (T)->[EDID Table]\n\r");
#endif /* if (TRANSMITTER_CMD_DEBUG) */

	EXTENDER_CopyRxDmaToApp(setup->iRecBuf_Index,EDID_MAX,Trnasmitter_EDID);
	//2.Copy edid table into EDID Table
#if (SYSETM_CONSOLE_EDID_CONTROL_ENABLE)
	//if (KVM_CurrentHost == 0)
	{
		DMA_GrantXdata(ConsoleEdidTable,Trnasmitter_EDID,EDID_MAX);
//#ifndef MCU_TYPE_AX68002
		HW_EDID_EnableSlave();
//#endif
	}
#endif
}

/*----------------------------------------------------------------------------
 * Function Name: Transmitter_Receiver_Restart_Handle
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void Transmitter_Receiver_Restart_Handle(void)
{
	Transmitter_Data_Toggle = TRANSMITTER_DATA_TOGGLE0;
	Receiver_Data_Toggle    = RECEIVER_DATA_TOGGLE0;
//#if (TRANSMITTER_CMD_DEBUG)
	printf("[(T)<-Receiver Restart][S:0x%02bx]\n\r", Transmitter_Control_State);
//#endif  /* #if (TRANSMITTER_CMD_DEBUG) */
	Tansmitter_PassThrough_Buf_In_Used_Flag = 0; //release the extender passthrough in used flag
	Tansmitter_PassThrough_Buf_In_Used_Devinx = 0;
	Transmitter_Control_State |= TRANSMITTER_CONNECT_FLAG;
#ifdef DEVICE_KEEP
#else	
	ExtenderT_Clean_USB_Device();
#endif	
	Extender_Receive_Data_Parser_State = EXTENDER_DATA_PACKET_SECTION_OK;
	Transmitter_Control_State |= (TRANSMITTER_KB_LED_FLAG | TRANSMITTER_HOST_LED_FLAG);
	Transmitter_Send_State |= TRANSMITTER_SEND_WAIT;
	//need to test this feature
	Transmitter_Control_State &= ~TRANSMITTER_RESTART_FLAG;
}

/*----------------------------------------------------------------------------
 * Function Name: Transmitter_Receiver_Mount_Handle
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */

U8_T Transmitter_Receiver_Mount_Handle(Extender_Cmd_Handle_Def *setup)
{
	U8_T  free_deviceid;
	U8_T  remote_upper_devinx;
	U8_T  remote_upper_port;
	U8_T  remote_devinx;
	U8_T  upper_devinx;

//#if (TRANSMITTER_CMD_DEBUG)
	printf(" (T)->Mount [RemoteDevice:%d(%02x),",(U16_T)setup->Data_Token.bValue,(U16_T)setup->Data_Token.bIndex);
//#endif /* #if (TRANSMITTER_CMD_DEBUG) */
	//1.Allocate a new device for this device
/*	
#ifdef DEVICE_KEEP
	//1.Check the current device table first
	etdr_T_DescriptorSkipFlag = 0;
	if (etdr_T_MountSkipFlag)
	{	
		etdr_T_MountSkipFlag = 0;
		printf("skip device\n\r");
		etdr_T_DescriptorSkipFlag = 1;
		return 1;
	}	
	//if (ExtenderT_Device_Keep_Flag)
	//{
	//	if (Transmitter_Check_Remote_NewDevice(setup))
	//		return 1;
	//}		
#endif	// #ifdef DEVICE_KEEP
*/
	if (USB_HAL_Alloc_Free_PDevice(&free_deviceid, 0xff))
	{
//#if (TRANSMITTER_CMD_DEBUG)
		printf("New TDevinx:%d]\n\r",(U16_T)free_deviceid);
//#endif /* #if (TRANSMITTER_CMD_DEBUG) */
		remote_devinx		= setup->Data_Token.bValue;
		remote_upper_devinx = (setup->Data_Token.bIndex & 0xf0) >> 4;
		remote_upper_port	= setup->Data_Token.bIndex & 0x0f;
		upper_devinx        = Remote_Reserve_DevMap[remote_upper_devinx];		
		//printf("T_devinx=%d,T_upper_devinx:%d,T_upper_hub_port:%d\n\r",(U16_T)free_deviceid,(U16_T)upper_devinx,(U16_T)remote_upper_port);

		Remote_DevMap[free_deviceid].Mapping = (USBHC_DEVICE_USED_MASK|remote_devinx);
		Remote_DevMap[free_deviceid].UpperInfo = setup->Data_Token.bIndex;
		Remote_Reserve_DevMap[remote_devinx] = free_deviceid;
		USB_PDevice[free_deviceid].DevAttr |= DEVATTR_REMOTE_DEV_MASK;

		USB_PDevice[free_deviceid].UpperHubDevinx = upper_devinx; //store the upper layer hub devinx
//		upper_devinx = setup->Data_Token.bIndex & 0x0f;
		USB_PDevice[free_deviceid].Hub_NbrPorts = remote_upper_port USBHC_ROOTHUBPORT_PORT_NUM_SHIFT; // upper layer connect port number
		etdr_T_MountSkipFlag = 0;
		return 1;
	}
	else
	{
		etdr_T_MountSkipFlag = 1;
		printf("No free device index!!!\n\r");
		return 0;
	}
}

/*----------------------------------------------------------------------------
 * Function Name: Transmitter_Receiver_Unmount_Handle
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void Transmitter_Receiver_Unmount_Handle(Extender_Cmd_Handle_Def *setup)
{
	U8_T remote_devinx;
	U8_T devinx;

	remote_devinx = setup->Data_Token.bValue;
	devinx = Remote_Reserve_DevMap[remote_devinx];

#if (TRANSMITTER_CMD_DEBUG)
	printf(" (T)->Unmount [RemoteDevice:%d,T_Device:%d]\n\r",(U16_T)setup->Data_Token.bValue,(U16_T)devinx);
#else
	printf("> UNMOUNT: (Upstream[Port=%d,HubDevinx=%d]) Devinx=%d\n\r",(U16_T)(USBDC_Device[devinx].VirHubNum[KVM_CurrentHost]-1),(U16_T)USB_PDevice[devinx].UpperHubDevinx,(U16_T)devinx);
#endif  /* #if (TRANSMITTER_CMD_DEBUG)	*/

	if (devinx == 0)
		return;
			
	if (USB_PDevice[devinx].Hc.gState == HOST_PASSTHROUGH)
		Tansmitter_PassThrough_Buf_In_Used_Flag = 0;

	if (Remote_DevMap[devinx].Mapping & USBHC_DEVICE_USED_MASK)
	{
#ifdef USB_2
		if (USB_PDevice[devinx].DevClass == USB_HID_CLASS)
		{
			for (remote_devinx = 0; remote_devinx < KVM_MAX_PORT ; remote_devinx++)
			{
				if ((USB_PDevice[devinx].MountPort & BIT_MASK[remote_devinx]))
				{
					Device_Host_Mount[remote_devinx] = 0;
					break;
				}
			}
		}
#endif
		Remote_DevMap[devinx].Mapping = 0;
		Remote_Reserve_DevMap[remote_devinx] = 0;		
		USBHC_CORE_Clear_Device(devinx);
		ExtenderT_Virtual_Device_Suspend_Check();
	}
}

/*----------------------------------------------------------------------------
 * Function Name: Transmitter_Set_Data_Transmit_Header
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
U16_T Transmitter_Set_Data_Transmit_Header(U16_T datalen,U8_T *buf)
{
	U16_T len;

	len = sizeof(Extender_Data_Packet_Def)+2+datalen;

	if (buf == NULL)
		Trnasmitter_Send_Buf = m_malloc(len,28);
	else
		Trnasmitter_Send_Buf = buf;

	if (Trnasmitter_Send_Buf != NULL)
	{
		Trnasmitter_Send_Data_TokenP = 	(Extender_Data_Packet_Def *) Trnasmitter_Send_Buf;
		return len;
	}

	return 0;
}

/*----------------------------------------------------------------------------
 * Function Name: TRANSMITTER_Restart_Report
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void TRANSMITTER_Restart_Report(void)
{
	U16_T	len;
#ifdef  DEVICE_KEEP			
	U8_T	value;
#endif	
	
	len = Transmitter_Set_Data_Transmit_Header(0,NULL);
	if (len)
	{
		//ExtenderT_Control_Flag |= ExtenderT_Start_Mask;
#ifdef  DEVICE_KEEP		
		value = ExtenderT_Control_Flag & ExtenderT_Start_Mask;
		Extender_Packet_Data(Trnasmitter_Send_Data_TokenP,0,0,EXTENDER_T_RESTART,0,value);
#else		
		Extender_Packet_Data(Trnasmitter_Send_Data_TokenP,0,0,EXTENDER_T_RESTART,0,0);
#endif		
//#if (TRANSMITTER_CMD_DEBUG)
		printf("[Transmitter Restart]\n\r");
//#endif /*#if (TRANSMITTER_CMD_DEBUG)*/
		Extender_Token_Transmit(Trnasmitter_Send_Buf,EXTENDER_CONTROL_TRANSMIT_NO_WAIT,len,1000,0);
		Transmitter_Control_State |= TRANSMITTER_RESTART_FLAG;
	}
}

/*----------------------------------------------------------------------------
 * Function Name: void TRANSMITTER_Transmit_End
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void TRANSMITTER_Transmit_End(void)
{
	U16_T	len;

	len = sizeof(Data_End_Token);
	Extender_Packet_Data((Extender_Data_Packet_Def *)Data_End_Token,0,0,EXTENDER_DATA_END,0,0);
#if (TRANSMITTER_CMD_DEBUG)
	printf(" (T)<-Transmitter End\n\r");
#endif
	Extender_Token_Transmit(Data_End_Token,(EXTENDER_CONTROL_TRANSMIT_NO_WAIT|EXTENDER_CONTROL_NONE_FREE),len,1000,0); //report start
	Transmitter_Send_State &= ~(TRANSMITTER_SEND_WAIT|TRANSMITTER_SEND_START); // clear the wait flag
//	Transmitter_Wait_Answer_State |= TRANSMITTER_WAIT_SOF_SEND_ANSWER;
}

/*----------------------------------------------------------------------------
 * Function Name: Transmitter_Receiver_Device_Descriptor_Handle
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void Transmitter_Receiver_Device_Descriptor_Handle(Extender_Cmd_Handle_Def *setup)
{
	U16_T len;	
	U8_T  descid;
	U8_T  active_port;
	U8_T  *buf;
	U8_T  remote_devinx;
	U8_T  devinx;
	U8_T  inf_id;
	USBHC_HUBDec_Typedef	*phubdec;

	descid = setup->Data_Token.bValue;
	len = setup->Data_Token.iLen;

#if (TRANSMITTER_CMD_DEBUG)
	printf(" (T)->[Descriptor:%d,Len:%d,",(U16_T)setup->Data_Token.bValue,len);
#endif /*TRANSMITTER_CMD_DEBUG*/

	if (len != 0)
	{
		buf = m_malloc(len,29);
		if (buf != NULL)
		{
			remote_devinx = setup->Data_Token.bIndex;
			devinx = Remote_Reserve_DevMap[remote_devinx];

#if (TRANSMITTER_CMD_DEBUG)
			printf("R:%d,T:%d]\n\r",(U16_T)remote_devinx,(U16_T)devinx);
#endif /*TRANSMITTER_CMD_DEBUG*/

			USB_PDevice[devinx].Desc[descid].Len = len;
			USB_PDevice[devinx].Desc[descid].Ptr = buf;

			//copy data into buffer
			Extender_CopyDataFromRxBuffer(setup->iRecBuf_Index,len,buf);

			switch(descid)
			{
				case DEVICE_DESC:
#if (TRANSMITTER_CMD_DEBUG)
					printf(" <T>-[Parser Device Descriptor]\n\r");
#endif /*TRANSMITTER_CMD_DEBUG*/
					USBHC_Parser_DevDesc(devinx,USB_PDevice[devinx].Desc[descid].Ptr);
					break;
				case CONFIG_DESC:
#if (TRANSMITTER_CMD_DEBUG)
					printf(" <T>-[Parser Configuration Descriptor]\n\r");
#endif /*TRANSMITTER_CMD_DEBUG*/
					USBHC_Parser_ConfigDesc(devinx,len,USB_PDevice[devinx].Desc[descid].Ptr);
					break;
				case HID0_REPORT:
				case HID1_REPORT:
				case HID2_REPORT:
				case HID3_REPORT:
					inf_id = descid-HID0_REPORT;
					USB_PDevice[devinx].Hc.Control.Buf = buf; //assing hid report descritpro table to control pointer
#if (TRANSMITTER_CMD_DEBUG)
					printf(" <T>-[Parser HID Report Descriptor for interface[%d]]\n\r",(U16_T)inf_id);
#endif /*TRANSMITTER_CMD_DEBUG*/
					USBHC_HID_Interface_Check(devinx,inf_id);
					USB_PDevice[devinx].Hc.Control.Buf = 0x00; //assing hid report descritpro table to control pointer
					break;
				case HUB_DESC:
					//Parser Hub Descriptor
					phubdec = (USBHC_HUBDec_Typedef *)USB_PDevice[devinx].Desc[HUB_DESC].Ptr;
					USB_PDevice[devinx].Hub_NbrPorts |= phubdec->bNbrPorts;
					len = sizeof(USBHC_HubReportTypeDef) * phubdec->bNbrPorts;
					USB_PDevice[devinx].HUB.ReportState = m_malloc(len,30);
					if (USB_PDevice[devinx].HUB.ReportState != NULL)
					{
						memset((U8_T *)USB_PDevice[devinx].HUB.ReportState,0x00,len);
					}
					else
					{
						printf("HUB Report Malloc Error !!\n\r");
					}
					break;
			}
		}
		else
		{
			printf("Descriptor malloc fail !!!]\n\r");
		}
	}
	else if (descid == EXTENDER_USB_DESCRIPTOR_END) /* EXTENDER_USB_DESCRIPTOR_END */
	{
#if (TRANSMITTER_CMD_DEBUG)
		printf(" <T>-[Active T_Device:%d]\n\r",(U16_T)devinx);
#endif /*TRANSMITTER_CMD_DEBUG*/
		//Check the device Class
		if (USB_PDevice[devinx].DevClass == USB_MSC_CLASS)
		{
#if (SYSTEM_EXTENDER_MSC_SUPPORT)
			USBHC_MSC_Devinx = devinx;
			EXTENDER_MSC_Devinx = devinx;
			KVM_CurrentMSC = KVM_CurrentHost;
			USB_PDevice[devinx].MSC = (USBHC_MSC_Device_TypeDef *) m_malloc(sizeof(USBHC_MSC_Device_TypeDef),31);
			memset(USB_PDevice[devinx].MSC,0x00,sizeof(USBHC_MSC_Device_TypeDef));
			USB_PDevice[devinx].MSC->BOT = (MSC_BOT_Control_TypeDefine *) m_malloc(sizeof(MSC_BOT_Control_TypeDefine),32);
			memset(USB_PDevice[devinx].MSC->BOT,0x00,sizeof(MSC_BOT_Control_TypeDefine));
			USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_IDLE;
			USBHC_MSC_Bulk_State_Reset(USBHC_MSC_Devinx);
			EXTENDER_MSC_BulkInWaitHead = EXTENDER_MSC_BulkInWaitTail = 0;
#endif //#if (SYSTEM_EXTENDER_MSC_SUPPORT)
			active_port = MOUNT_PORT[KVM_CurrentMSC];
		}
		else
		{
#ifdef USB_2
			if (USB_PDevice[devinx].DevClass == USB_HID_CLASS)
			{
				for (descid = 0; descid < KVM_MAX_PORT ; descid++)
				{
					if (Device_Host_Mount[descid] == 0)
					{
						Device_Host_Mount[descid]=1;
						active_port = BIT_MASK[descid];
						break;
					}
				}
			}
			else
			{
				active_port = UDC_PORT_ALL_ACT;
			}
#else
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
			if (USB_PDevice[devinx].DevClass == USB_AUDIO_CLASS)
			{
				USBHC_Audio_Devinx = devinx;
				KVM_CurrentUSBAudio = KVM_CurrentHost;
				active_port = MOUNT_PORT[KVM_CurrentUSBAudio];
				USBHC_Audio_Device_Cnt++;
			}
			else
#endif /* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */
			{
				active_port = UDC_PORT_ALL_ACT;
			}
#endif /* #ifdef USB_2 */
		}
		//printf("active_port=0x%02bx\n\r",active_port);		
		USBHC_Active_New_Device(devinx,active_port);
		ExtenderT_Virtual_Device_Suspend_Check();
#ifdef VIRTUAL_HUB_RESET		
		USBDC_Virtual_Hub_Reset(KVM_CurrentHost,1);
#endif		
	}
}

/*----------------------------------------------------------------------------
 * Function Name: TASK_USBHC_ExtenderT_PassThrough
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void TASK_USBHC_ExtenderT_PassThrough(void)
{
	U8_T	devinx, pid;
//	U8_T	ctrl_state;
	U16_T	remain_len;
	RESULT	Result = USB_BUSY;
	bit		terminate=0;

	if (TASK_Type == TASK_TYPE_INTERVAL_MS)
	{
		TASK_Destory_Current();
	}
	//Get paramter transfer
	devinx = TASK_Register0;
//	ctrl_state = TASK_Register1;
	pid = USB_PDevice[devinx].Hc.PControl.UPID; // which host channel is used by device
	//Skip task if the device is gone
	if ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00) // Device is gone
	{
		return;
	}

#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)
	printf(">> TASK_EXTENDERT_PassThrough_Handle(%d)-(%02x):\n\r",(U16_T)devinx,(U16_T)USB_PDevice[devinx].Hc.PassThrough_State);
#endif //#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)

	switch (USB_PDevice[devinx].Hc.PassThrough_State)
	{
		case PASSTHROUGH_SETUP_START: //Send A control setup to HC
#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)
			printf("   (1).SETUP_TOKEN Stage\n\r");
#endif //#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)
			/* convert back to normal USB order */
			/* Task Fork => Send out Setup Token */
			Result = USBHC_ExtenderT_PassThrough_Setup_Token_Parser(devinx);
			break;
		case PASSTHROUGH_DC_SETUP_DATA_IN: //After passthrough Setup token send complete
PASSTHROUGH_DC_SETUP_DATA_IN_RESEND:
			if (USB_PDevice[devinx].Hc.Control.State == CTRL_DATA_IN) //This should put into Transmitter Receiver Handle first
			{	//new data page
				remain_len = USB_PDevice[devinx].Hc.PControl.Total_Length - USB_PDevice[devinx].Hc.PControl.Current_Length;
				if (remain_len > USBDC_Device[devinx].EndpMaxSize[0]) //send one data packet out
					USB_PDevice[devinx].Hc.PControl.Data_Length = USBDC_Device[devinx].EndpMaxSize[0];
				else
					USB_PDevice[devinx].Hc.PControl.Data_Length = remain_len;

				USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_IN_WAIT; // wait for DC complete
				USBDC_VIRTUAL_Setup_DataStageIn(devinx,pid,1); //Set DC Buffer ready register
			}
			else if (USB_PDevice[devinx].Hc.Control.State == CTRL_DATA_IN_WAIT) // dc data in complete
			{
				if (USBDC_Device[devinx].ControlState[pid] == LAST_IN_DATA) // if this is the last
				{
					// no more data
					USB_PDevice[devinx].Hc.Control.State = CTRL_STATUS_OUT; // Next is status out
					break;
				}
				else
				{
					USB_PDevice[devinx].Hc.PControl.Current_Length += USB_PDevice[devinx].Hc.PControl.Data_Length;
					USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_IN;
					if (USB_PDevice[devinx].Hc.PControl.Current_Length < USB_PDevice[devinx].Hc.PControl.Total_Length)
					{
						// Still in DC Data IN Page Data transmit mode
						goto PASSTHROUGH_DC_SETUP_DATA_IN_RESEND;
					}
					else
					{
						if (USB_PDevice[devinx].Hc.PControl.Current_Length == USB_PDevice[devinx].Hc.PControl.Total_Length &&
							USB_PDevice[devinx].Hc.PControl.Data_Length == USBDC_Device[devinx].EndpMaxSize[0])
						{
							goto PASSTHROUGH_DC_SETUP_DATA_IN_RESEND;
						}

						//Page data send to DC complet, issue next data from remote usb device
						ExtenderT_Device_Passthough_State[devinx] |= T_USB_PASSTHROUGH_DATA_IN_ENABLE_FLAG; //First data page is ready to receive
						ExtenderT_Device_Passthrough_Flag |= BIT_MASK[devinx];
						USB_PDevice[devinx].Hc.PassThrough_State   = PASSTHROUGH_HC_SETUP_DATA_IN;
					}
				}
			}
			else
			{
				printf("ExtendT control state %bx\r\n", USB_PDevice[devinx].Hc.Control.State);
			}
#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)
			printf("   (2).ExtenderT_HC_Data_IN=>Total:%d,Current:%d,Data=%d\n\r",USB_PDevice[devinx].Hc.PControl.Total_Length,
			USB_PDevice[devinx].Hc.PControl.Current_Length, USB_PDevice[devinx].Hc.PControl.Data_Length);
#endif //#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)
			break;
		case PASSTHROUGH_HC_SETUP_DATA_OUT: //Setup Data out Stage
			/*update length first */
			if (USB_PDevice[devinx].Hc.Control.State == CTRL_DATA_OUT)
			{
				//new data page
				USB_PDevice[devinx].Hc.PControl.Current_Length += USB_PDevice[devinx].Hc.PControl.Data_Length;
				if (USB_PDevice[devinx].Hc.PControl.Current_Length < USB_PDevice[devinx].Hc.PControl.Total_Length)
				{
					USBDC_REGS_Endp_ControlClear(pid,devinx,0,DA_CR_BCLR_SET); // Clear the OUT buffre flag
				}
				else
				{
					USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_OUT_WAIT; // wait transaction complete
					ExtenderT_Device_Passthough_State[devinx] |= T_USB_PASSTHROUGH_DATA_OUT_FLAG;
					ExtenderT_Device_Passthrough_Flag |= BIT_MASK[devinx];
				}
			}
			else if (USB_PDevice[devinx].Hc.Control.State == CTRL_DATA_OUT_WAIT)
			{
				if (USBDC_Device[devinx].ControlState[pid] == WAIT_STATUS_IN) //should let DC answer status in
				{
					USBDC_REGS_Endp_ControlSet(pid,devinx,0,DA_CR_BVLD_SET); // STATUS IN
				}
				else
				{
					remain_len = USBDC_Device[devinx].Ctrl_TotalByte[pid]-USBDC_Device[devinx].Ctrl_CurrentByte[pid];
					if (remain_len > EXTENDER_PAGE_SIZ)
						remain_len = EXTENDER_PAGE_SIZ;

					if (remain_len)
					{
						USB_PDevice[devinx].Hc.PControl.Total_Length = remain_len;
						USB_PDevice[devinx].Hc.PControl.Current_Length = 0;
						USBDC_REGS_Endp_ControlClear(pid,devinx,0,(DA_CR_SCLR_SET|DA_CR_BCLR_SET)); // now can receive data
						USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_OUT; // wait transaction complete
					}
				}
			}
#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)
			printf("   (2).ExtenderT_HC_Data_Out:Total:%d,Current:%d,Data=%d\n\r",USB_PDevice[devinx].Hc.PControl.Total_Length,
			USB_PDevice[devinx].Hc.PControl.Current_Length, USB_PDevice[devinx].Hc.PControl.Data_Length);
#endif //#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)
			break;
		case PASSTHROUGH_HC_SETUP_STATUS_IN:
			USBDC_EndpBufPtr[pid][devinx][0][IN_BUF_LEN_OFFSET] = 0;
			USBDC_REGS_Endp_ControlSet(pid,devinx,0,DA_CR_BVLD_SET); // STATUS IN
			break;
		default:
			break;
	}

	if (Result == USB_SUCCESS)
	{
#if (USB_PASSTHROUGH_DEBUG_MODE)
		printf("@@ Done=>TASK_USBHC_PassThrough Complete\n\r");
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
		terminate = 1;
	}
	else if (Result == USB_ERROR)
	{
		USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_FAIL;
#if (USB_PASSTHROUGH_DEBUG_MODE)
		printf("@@ Ternimate=>PASSTHROUGH_FAIL\n\r");
#endif
	}
	else
	{
		terminate = 0;
	}

	if (terminate)
	{
		//printf("terminate\n\r");
		USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
#if (USB_PASSTHROUGH_DEBUG_MODE)
		printf("@P@ TD_Free:%d\n\r",(U16_T)USB_PDevice[devinx].Hc.Control.TdNum);
#endif
		USB_PDevice[devinx].Hc.gState = USB_PDevice[devinx].Hc.PassThrough_gState_bk;
	}
}

/*----------------------------------------------------------------------------
 * CTRL_State USBHC_ExtenderT_PassThrough_Setup_Token_Parser(U8_T devinx)
 * Purpose : Parser the setup command, to descide next step, IN? OUT?
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
RESULT USBHC_ExtenderT_PassThrough_Setup_Token_Parser(U8_T devinx)
{
	U8_T	direction;


	direction = (USB_PDevice[devinx].Hc.Control.Setup.b.bmRequestType & USB_REQ_DIR_MASK); /* data direction */
	//Check the status,prevent the last passthroug not compelte
	if (ExtenderT_Device_Passthough_State[devinx] & ~(T_USB_PASSTHROUGH_DC_DONE_FLAG|T_USB_PASSTHROUGH_REMOTE_DONE_FLAG))
	{
		ExtenderT_Device_Passthough_State[devinx] |= (T_USB_PASSTHROUGH_DC_DONE_FLAG|T_USB_PASSTHROUGH_REMOTE_DONE_FLAG);
		ExtenderT_Passthrough_Done_Check(devinx); //clear all the relative
	}

	ExtenderT_Device_Passthough_State[devinx] = T_USB_PASSTHROUGH_SETUP_FLAG;
	ExtenderT_Device_Passthrough_Flag |= BIT_MASK[devinx];
//	ExtenderT_Device_Passthough_State[devinx] |= T_USB_PASSTHROUGH_SETUP_FLAG;

	if (USB_PDevice[devinx].Hc.Control.Setup.b.wLength.w != 0) /* with data stage */
	{
		if (direction == USB_D2H) /*Device to Host,Data Direction is IN*/
		{
			USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_HC_SETUP_DATA_IN;
			USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_IN;
			ExtenderT_Device_Passthough_State[devinx] |= T_USB_PASSTHROUGH_DATA_IN_ENABLE_FLAG; //First data page is ready to receive
		}
		else
		{
			/* Host to Device */
			USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_HC_SETUP_DATA_OUT;
			USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_OUT_WAIT;
		}
	}
	else
	{
		USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_HC_SETUP_STATUS_IN;
	}

#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)
	printf("^^EXTENDERT_SETUP(0x%02x)^^,",(U16_T)USB_PDevice[devinx].Hc.PassThrough_State);
//	Disp_Str((U8_T *)&USB_PDevice[devinx].Hc.PControl.Setup,8);
#endif //#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)

	/* convert back to normal USB order */
	USBDC_HAL_Convert_Setup_Value((U8_T *)&USB_PDevice[devinx].Hc.PControl.Setup);
	return USB_BUSY;
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_Setup_Token_Stage
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Setup_Token_Stage(U8_T devinx)
{
	U8_T	*buf;
	U8_T	token_len;
	U8_T	remote_devinx;
	U16_T	len;

	token_len = sizeof(USB_Setup_TypeDef);
	len = Transmitter_Set_Data_Transmit_Header(token_len,NULL);
	if (len)
	{
		buf = (U8_T *)&USB_PDevice[devinx].Hc.Control.Setup;
		remote_devinx = Remote_DevMap[devinx].Mapping & ~USBHC_DEVICE_USED_MASK;
		Extender_Packet_Data(Trnasmitter_Send_Data_TokenP,token_len,buf,EXTENDER_USB_SETUP_STAGE,remote_devinx,0);
		Extender_Token_Transmit(Trnasmitter_Send_Buf,EXTENDER_CONTROL_TRANSMIT_NO_WAIT,len,1000,0);
		ExtenderT_Device_Passthough_State[devinx] &= ~T_USB_PASSTHROUGH_SETUP_FLAG;
	}
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_Setup_Data_Out_Stage
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Setup_Data_Out_Stage(U8_T devinx)
{
	U8_T	*buf;
	U8_T	remote_devinx;
	U16_T	data_len;
	U16_T	len;

	buf = USB_PDevice[devinx].Hc.PControl.Buf - sizeof(Extender_Data_Packet_Def);
	data_len = USB_PDevice[devinx].Hc.PControl.Current_Length;
	len = Transmitter_Set_Data_Transmit_Header(data_len,buf);
	if (len)
	{
		remote_devinx = Remote_DevMap[devinx].Mapping & ~USBHC_DEVICE_USED_MASK;
		Extender_Packet_Data((Extender_Data_Packet_Def *)buf,data_len,NULL,EXTENDER_USB_DATA_OUT_STAGE,remote_devinx,0);
#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)
		printf(" <T>->[Transmitter Data Out:%d\]\n\r",(U16_T)data_len);
#endif /* #if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE) */
		Extender_Token_Transmit(buf,(EXTENDER_CONTROL_TRANSMIT_NO_WAIT|EXTENDER_CONTROL_NONE_FREE),len,1000,0);
		ExtenderT_Device_Passthough_State[devinx] &= ~T_USB_PASSTHROUGH_DATA_OUT_FLAG;
	}
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_Setup_Status_Stage
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Setup_Data_In_Enable(U8_T devinx)
{
	U16_T	len;
	U8_T	remote_devinx;

	len = Transmitter_Set_Data_Transmit_Header(0,NULL);
	if (len)
	{
		remote_devinx = Remote_DevMap[devinx].Mapping & ~USBHC_DEVICE_USED_MASK;
		Extender_Packet_Data(Trnasmitter_Send_Data_TokenP,0,NULL,EXTENDER_USB_DATA_IN_STAGE,remote_devinx,0);
#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)
		printf(" <T>->[Transmitter Setup In Enable:%d]\n\r",len);
#endif /*#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)*/
		Extender_Token_Transmit(Trnasmitter_Send_Buf,EXTENDER_CONTROL_TRANSMIT_NO_WAIT,len,1000,0);
		ExtenderT_Device_Passthough_State[devinx] &= ~T_USB_PASSTHROUGH_DATA_IN_ENABLE_FLAG;
	}
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_Setup_Status_Stage
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Setup_Status_Stage(U8_T devinx,U8_T direction)
{
	U16_T	len;
	U8_T	remote_devinx;

	len = Transmitter_Set_Data_Transmit_Header(0,NULL);
	if (len)
	{
		remote_devinx = Remote_DevMap[devinx].Mapping & ~USBHC_DEVICE_USED_MASK;
		Extender_Packet_Data(Trnasmitter_Send_Data_TokenP,0,0,direction,remote_devinx,0);
#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)
		printf(" <T>->[Transmitter Status Stage:%02x]\n\r",(U16_T)direction);
#endif /*#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)*/
		Extender_Token_Transmit(Trnasmitter_Send_Buf,EXTENDER_CONTROL_TRANSMIT_NO_WAIT,len,1000,0);
		if (direction == EXTENDER_USB_STATUS_OUT)
		{
			ExtenderT_Device_Passthough_State[devinx] |= T_USB_PASSTHROUGH_DC_DONE_FLAG; //data complete
			ExtenderT_Passthrough_Done_Check(devinx);
		}
		ExtenderT_Device_Passthough_State[devinx] &= ~(T_USB_PASSTHROUGH_STATUS_OUT_FLAG|T_USB_PASSTHROUGH_STATUS_IN_FLAG);
	}
}

void ExtenderT_Passthrough_Condition_Reset(U8_T devinx)
{
	USBDC_Device[devinx].PassThroughState[USB_PDevice[devinx].Hc.PControl.UPID] = PASSTHROUGH_IDLE;
	USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_IDLE;
#if (EXTENDER_PASSTHROUGH_BUF_MALLOC)
	USB_PDevice[devinx].Hc.PControl.Buf -= sizeof(Extender_Data_Packet_Def); //jump inot position 0
	malloc_free(USB_PDevice[devinx].Hc.Control.Buf);
#endif	/* #if (EXTENDER_PASSTHROUGH_BUF_MALLOC) */
	USB_PDevice[devinx].Hc.Control.Buf = 0;
	USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
	USB_PDevice[devinx].Hc.gState = USB_PDevice[devinx].Hc.PassThrough_gState_bk;
	Tansmitter_PassThrough_Buf_In_Used_Flag = 0;
	ExtenderT_Device_Passthough_State[devinx] = 0;
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_Passthrough_Done_Check
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Passthrough_Done_Check(U8_T devinx)
{
	if (ExtenderT_Device_Passthough_State[devinx] & T_USB_PASSTHROUGH_REMOTE_DONE_FLAG)
	{
		if (ExtenderT_Device_Passthough_State[devinx] & T_USB_PASSTHROUGH_DC_DONE_FLAG)
		{
			ExtenderT_Passthrough_Condition_Reset(devinx);
		}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: TASK_Extender_Transmitter_Plug_Check
 * Purpose: Task to check the transmitter plug in
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void TASK_Extender_Transmitter_Plug_Check(void)
{
	if (Transmitter_Control_State & TRANSMITTER_CONNECT_FLAG)
	{
		Transmitter_Plug_Off_Cnt++;
		if (Transmitter_Plug_Off_Cnt > RECEIVER_PLUG_OFF_CNT)
		{
			printf("Receiver Plug Off(%bu)\n\r",++Receiver_Plug_Off_Cnt);
			etdr_T_WaitRestartAckFlag = 1;
#ifdef  DEVICE_KEEP					
			ExtenderT_Report_Restart_Cnt = 0;
#endif			
			Transmitter_Plug_Off_Cnt=0;
			Transmitter_Control_State &= ~(TRANSMITTER_CONNECT_FLAG|TRANSMITTER_RESTART_FLAG);
#ifdef DEVICE_KEEP
#else				
			ExtenderT_Clean_USB_Device();
#endif			
			Transmitter_Send_State |= TRANSMITTER_SEND_WAIT;

			Extender_Receive_Cmd_Parser_State  = 0;
			Extender_Receive_Data_Parser_State = 0;
			//etdr_T_485FirstFlag = 5;
#ifdef PWAYTEK_01
			RECEIVER_STATE_LED_GPIO = LED_OFF;
#endif
			Task_Active_Table[TASK_ExtenderR_Actived_Led_Flash_Active_ID].Task_Event |= TASK_EVENT_SKIP;
		}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_Clean_USB_Device
 * Purpose: Task to check the transmitter plug in
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Clean_USB_Device(void)
{
	U8_T index;
	U8_T devinx;

	for (index = USBDC_VREMOTE_HUB_DEVINX; index < USBDC_DEVICE_MAX; index++)
	{
		if (Remote_DevMap[index].Mapping & USBHC_DEVICE_USED_MASK)
		{
			printf("Clean Devinx:%bx\n\r",index);
			//Should remove this device
			devinx = Remote_DevMap[index].Mapping & ~USBHC_DEVICE_USED_MASK;
			Remote_DevMap[index].Mapping = 0;
			USBHC_CORE_Clear_Device(index);
			Remote_Reserve_DevMap[devinx] = 0; //clear reverse mapping
		}
	}

#ifdef USB_2
	memset(Device_Host_Mount,0x00,sizeof(Device_Host_Mount));
#endif
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_Virtual_Device_Suspend_Check
 * Purpose: This will wakeup all device
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Virtual_Device_Suspend_Check(void)
{
	U8_T index;

	for (index=USBDC_VREMOTE_HUB_DEVINX; index < USBDC_DEVICE_MAX ; index++)
	{
		if (USBDC_Device[index].Feature[KVM_CurrentHost] & FEATURE_SUSPEND)//this device go suspend
		{
			USBDC_VHub_Resume_Port(KVM_CurrentHost,index);
		}
	}

	//The final step, resume the root hub if in suspend mode
	/*Check Virtual Root HUB suspend */
	if (USBDC_UpPortState[KVM_CurrentHost] & USBDC_ROOTHUB_ATTACHED_MASK)
	{
		if (USBDC_UpPortState[KVM_CurrentHost] & USBDC_ROOTHUB_SUSPEND_MASK)
		{
			USBDC_Port_Resume(KVM_CurrentHost);
		}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_Setup_Buffer_Malloc
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
U8_T ExtenderT_Setup_Buffer_Malloc(U8_T pid,U8_T devinx)
{
	U16_T data_len;
#if (EXTENDER_PASSTHROUGH_BUF_MALLOC)
	U16_T malloc_len;
#endif /* #if (EXTENDER_PASSTHROUGH_BUF_MALLOC) */

	memcpy(&USB_PDevice[devinx].Hc.Control.Setup,&USBDC_Device[devinx].Setup[pid],sizeof(USB_Setup_TypeDef));
	if (USB_PDevice[devinx].Hc.Control.Setup.b.wLength.w != 0) /* with data stage */
	{
		if (USB_PDevice[devinx].Hc.PControl.Total_Length > EXTENDER_PAGE_SIZ)
		{
			data_len = EXTENDER_PAGE_SIZ;
			USB_PDevice[devinx].Hc.PControl.Total_Length = EXTENDER_PAGE_SIZ;
		}
		else
		{
			data_len = USB_PDevice[devinx].Hc.PControl.Total_Length;
		}

#if (EXTENDER_PASSTHROUGH_BUF_MALLOC)
		malloc_len = data_len+sizeof(Extender_Data_Packet_Def)+2;
		Tansmitter_PassThrough_Buf_In_Used_Flag = 0;
		USB_PDevice[devinx].Hc.PControl.Buf = m_malloc(malloc_len,33);
		if (USB_PDevice[devinx].Hc.PControl.Buf == NULL)
		{
			printf(" <T>:Passthrough Setup malloc Fail\n\r");
			return 0;
		}
#else
		//memroy alloc
		Tansmitter_PassThrough_Buf_In_Used_Devinx = devinx;
		if (Tansmitter_PassThrough_Buf_In_Used_Flag == 0)
		{
			Tansmitter_PassThrough_Buf_In_Used_Flag = 1;
		}
		USB_PDevice[devinx].Hc.PControl.Buf = Extender_Passthrough_Buf;
#endif /* #if (EXTENDER_PASSTHROUGH_BUF_MALLOC) */

		//Set Pointer to Data Segment.
		USB_PDevice[devinx].Hc.PControl.Buf += sizeof(Extender_Data_Packet_Def); //jump inot data body
		USBDC_Device[devinx].Control_EndpBuf[pid] = USB_PDevice[devinx].Hc.PControl.Buf;
	}
	else
	{
		USB_PDevice[devinx].Hc.PControl.Buf = 0; //no data
	}
	return 1;
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
		if (Transmitter_Control_State & TRANSMITTER_CONNECT_FLAG)
		{
			Remote_Link_LED_Flash_Flag = 0;
			if (Remote_Link_LED_Flash_State==0)
			{
				Remote_Link_LED_Flash_State = 1;
#if defined(PCT_MUA22) || defined(PWAYTEK_01)
				DATA_TRANSFER_LED_GPIO = LED_OFF;
#else
				KVM_Host_Led_Control(0, LED_OFF);
#endif /* #ifdef PWAYTEK_01 */
			}
			else
			{
				Remote_Link_LED_Flash_State = 0;
#if defined(PCT_MUA22) || defined(PWAYTEK_01)
				DATA_TRANSFER_LED_GPIO = LED_ON;
#else
				KVM_Host_Led_Control(0, LED_ON);
#endif	/* #ifdef PCT_MUA22 */
			}
		}
	}
}
#endif

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
	//if (KVM_CurrentHost == 0) //Remote site
	{
		if ((Transmitter_Control_State & TRANSMITTER_CONNECT_FLAG) == 0x00)
		{
			if (Remote_Link_LED_State)
			{
				//printf("REMOTE LED ON\n\r");
#ifdef PCT_MUA22
				DATA_TRANSFER_LED_GPIO = LED_ON;
#else
				KVM_Host_Led_Control(0, LED_ON);
#endif /* #ifdef PCT_MUA22 */
				Remote_Link_LED_State = 0;
			}
			else
			{
				//printf("REMOTE LED OFF\n\r");
#ifdef PCT_MUA22
				DATA_TRANSFER_LED_GPIO = LED_OFF;
#else
				KVM_Host_Led_Control(0, LED_OFF);
#endif /* #ifdef PCT_MUA22 */
				Remote_Link_LED_State = 1;
			}
		}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_Intr_Data_In_Handle
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Intr_Data_In_Handle(void)
{
	U8_T  currenthost;
	U8_T  devinx;
	U8_T  remote_devinx;
	U8_T  inf_inx;
	U8_T  intt_id;
	U16_T len;
	U16_T start_index;
	U8_T  endpidx;
	U8_T  *buf;
	U8_T  endp_bit;
	U8_T  wp;
	U8_T  rp;
	U8_T  databyte;
	U8_T  remain_siz;
	U8_T  skip_flag;
#ifdef HIKDVR			
	U8_T  vhid_inf;
#endif		
		
	remote_devinx = (Extender_Setup.Data_Token.bIndex & 0x0f);
	
#ifdef HIKDVR		
	devinx = USBDC_VHID_DEVINX;	
	vhid_inf = (Extender_Setup.Data_Token.bIndex & 0xf0) >> 4;
#else
	devinx = Remote_Reserve_DevMap[remote_devinx];		
	//Error condition check	
	if ((remote_devinx == 0) || (remote_devinx >= USB_HC_MAX_DEVICE) || (devinx == 0))
	{
		return;
	}				
		
#if (PROJECT_USB_GENERIC_HID_ENABLE)	
	if (devinx == USBDC_VHID_DEVINX)
	{
		ExtenderT_PS2_Data_Handle();
		return;
	}
#endif /* #if (PROJECT_USB_GENERIC_HID_ENABLE)	*/
#endif /* #ifdef HIKDVR */
	
	currenthost = KVM_CurrentHost;
#ifdef USB_2
	if (USB_PDevice[devinx].DevClass == USB_HID_CLASS)
	{
		for (wp = 0; wp < KVM_MAX_PORT ; wp++)
		{
			if (USB_PDevice[devinx].MountPort & BIT_MASK[wp])
			{
				currenthost = wp;
				break;
			}
		}
	}
#endif
	len = Extender_Setup.Data_Token.iLen;
	inf_inx = (Extender_Setup.Data_Token.bValue & 0xf0) >> 4; //this interface

	endpidx = Extender_Setup.Data_Token.bValue & 0x0f; //this interface
	intt_id = USBDC_Device[devinx].EndpType[endpidx] & 0x1f; // Get interrupt TD ID number	
	//printf("D=%bu-%bu,l=%d\n\r",remote_devinx,devinx,len);	
	
	skip_flag = USBDC_Check_Upstream_Suspend_State(currenthost,devinx);		
	
	//Check whether upstream port is connect?
	if (USBDC_Device[devinx].DevAddr[currenthost] == 0x00)
	{
		return;
	}
	
#ifdef HIKDVR		
	//skip_flag = USBDC_Check_Upstream_Suspend_State(currenthost,devinx);		
	
	/* 2.Process HID output */
	if (skip_flag == 0)
	{		
		if (len < 8)		
		{			
			vhid_inf = 1;
			Extender_CopyDataFromRxBuffer(Extender_Setup.iRecBuf_Index,len,HCTD_Table.INTL[1].Buf);			
		}
		else
		{			
			vhid_inf = 0;
			Extender_CopyDataFromRxBuffer(Extender_Setup.iRecBuf_Index,len,HCTD_Table.INTL[0].Buf);			
		}			
		USBHC_InterruptTransferIN_Buf(vhid_inf,devinx,(U8_T)len);
		return;
	}		
#endif	
	/*
	if (USBDC_Check_Upstream_Suspend_State(currenthost,devinx))
	{
		return;
	}

	if (USBDC_Device[devinx].Feature[currenthost] & FEATURE_RESUME)
	{
		return;
	}
 	*/

	//len = Extender_Setup.Data_Token.iLen;
	//inf_inx = (Extender_Setup.Data_Token.bValue & 0xf0) >> 4; //this interface

	//endpidx = Extender_Setup.Data_Token.bValue & 0x0f; //this interface
	//intt_id = USBDC_Device[devinx].EndpType[endpidx] & 0x1f; // Get interrupt TD ID number	
	if (USB_PDevice[devinx].InfProtocol[inf_inx] == INF_KEYBOARD) /* should be INF_MOUSE ? */
	{
ExtenderT_InterruptTransferIN_Buf_Restart:
		wp = HC_IntTransfer_Table[intt_id].PassControl.Wp;
		rp = HC_IntTransfer_Table[intt_id].PassControl.Rp;

		//2_1.Check the buffer remain size(contain len bytes at least)
		if (wp == rp)
		{
			remain_siz = HC_IntTransfer_Table[intt_id].PassControl.BufSize;
		}
		else if (wp > rp)
		{
			remain_siz = (HC_IntTransfer_Table[intt_id].PassControl.BufSize-wp)+rp;
		}
		else
		{
			remain_siz = rp - wp;
		}

		//2_2.move out the data if the remain size is small than len
		if (remain_siz < (len+1))
		{
			USBHC_Remove_HID_Buffer(intt_id);
			goto ExtenderT_InterruptTransferIN_Buf_Restart;
		}

		//2_3.move interrupt in buffer data into out queue buffer
		HC_IntTransfer_Table[intt_id].PassControl.Buf[wp] = len;
		wp++;
		if (wp >= HC_IntTransfer_Table[intt_id].PassControl.BufSize)
			wp = 0;

		start_index = Extender_Setup.iRecBuf_Index;
		for (databyte =0; databyte < len ; databyte++)
		{
			HC_IntTransfer_Table[intt_id].PassControl.Buf[wp] = rs485_RcvrBufRing[start_index];
			wp++;
			if (wp >= HC_IntTransfer_Table[intt_id].PassControl.BufSize)
				wp = 0;
			start_index++;
			start_index &= (UR2_RX_BUF_SIZE -1);
		}

		HC_IntTransfer_Table[intt_id].PassControl.Wp = wp;

		if ((HC_IntTransfer_Table[intt_id].PassControl.State & PASSTHROUGH_SET) == 0)
		{
			HC_IntTransfer_Table[intt_id].PassControl.State |= PASSTHROUGH_SET;
			TASK_Active(TASK_TYPE_EVENT,TASK_DATAST_USB_Virtual_HID_ID,0,intt_id,0,0); // now aticev the Virtual HID Output Handle
		}
	}
	else
	{
		//USBHC_InterruptTransfer_Mouse(devinx,intt_id,HCTD_Table.INTL[intt_id].Buf,len);
		/*@@Check Port Suspend & Wakeup condition */
		//USBDC_Check_Upstream_Suspend_State(currenthost,devinx);
		 
		endp_bit = (0x01 << endpidx);
		start_index = Extender_Setup.iRecBuf_Index;

		if ((USBDC_Device[devinx].EndpIntrAttr & endp_bit) == 0)
		{
			buf = (USBDC_EndpBufPtr[currenthost][devinx][endpidx]+2);
			Extender_CopyDataFromRxBuffer(start_index,len,buf);
			USBDC_EndpBufPtr[currenthost][devinx][endpidx][0] = len;
			USBDC_REGS_Endp_ControlSet(currenthost,devinx,endpidx,DA_CR_BVLD_SET); // inform data move
			USBDC_Device[devinx].EndpIntrAttr |= endp_bit; // after Endp send out the interrutp need to clear this bit
		}
		else
		{
			buf = HC_IntTransfer_Table[intt_id].PassControl.Buf;
			buf[0] = (len & 0x00ff);
			Extender_CopyDataFromRxBuffer(start_index,len,buf+1);
			USBDC_Device[devinx].EndpIntrWait |= endp_bit; //one buffer content is waiting
//			USBDC_REGS_Endp_ControlSet(EXTENDER_HOST_PORT,devinx,endpidx,DA_CR_BVLD_SET); // inform data move
		}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_Intr_In_Move_Wait_Handle
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Intr_In_Move_Wait_Handle(U8_T devinx,U8_T intt_id,U8_T endpidx)
{
	U8_T  *buf;
	U8_T  endp_bit;
	U8_T  len;
	U8_T  currenthost;
#ifdef USB_2
	U8_T  index;
#endif

	buf = HC_IntTransfer_Table[intt_id].PassControl.Buf;
	len = buf[0];

	endp_bit = (0x01 << endpidx);
	currenthost = KVM_CurrentHost;
#ifdef USB_2
	if (USB_PDevice[devinx].DevClass == USB_HID_CLASS)
	{
		for (index = 0; index < KVM_MAX_PORT ; index++)
		{
			if (USB_PDevice[devinx].MountPort & BIT_MASK[index])
			{
				currenthost = index;
				break;
			}
		}
	}
#endif
	USBDC_EndpBufPtr[currenthost][devinx][endpidx][0] = len;
	memcpy((USBDC_EndpBufPtr[currenthost][devinx][endpidx]+2),(buf+1),len);
	USBDC_REGS_Endp_ControlSet(currenthost,devinx,endpidx,DA_CR_BVLD_SET); // inform data move
	USBDC_Device[devinx].EndpIntrAttr |= endp_bit; // after Endp send out the interrutp need to clear this bit
	USBDC_Device[devinx].EndpIntrWait &= ~endp_bit; //one buffer content is waiting
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_KVM_KB_Led_Send
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_KVM_KB_Led_Send(void)
{
	U16_T len;

	len = Transmitter_Set_Data_Transmit_Header(0,NULL);
	if (len)
	{
		Extender_Packet_Data(Trnasmitter_Send_Data_TokenP,0,0,EXTENDER_KVM_KB_LED,KVM_HostLed[KVM_CurrentHost],0);
#if (TRANSMITTER_CMD_DEBUG)
		printf(" (T)<-[Transmitter KB_LED(%02bx]\n\r",KVM_HostLed[KVM_CurrentHost]);
#endif /*#if (TRANSMITTER_CMD_DEBUG)*/
		Extender_Token_Transmit(Trnasmitter_Send_Buf,EXTENDER_CONTROL_TRANSMIT_NO_WAIT,len,1000,0);
		Transmitter_Control_State &= ~TRANSMITTER_KB_LED_FLAG;
	}
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_KVM_KB_Led_Check
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_KVM_KB_Led_Check(U8_T port)
{
#ifdef USB_2
	KVM_HostLed[KVM_CurrentHost] = KVM_HostLed[port];
#else
	if (port == KVM_CurrentHost)
#endif
	{
		Transmitter_Control_State |= TRANSMITTER_KB_LED_FLAG;
		Transmitter_Send_State |= TRANSMITTER_SEND_WAIT;
	}
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_SOF_Token_Received_Handle
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_SOF_Token_Received_Handle(void)
{
	U8_T	devinx, state;
	U8_T    audio_flag=0;
	U8_T    start_cnt=15;
	bit		display=0;

 	//KVM_Select_Led_Control(1,LED_OFF);
 	//P2_2 = 0;
	// whether Receiver had received RESTART command		
	
	if (etdr_T_WaitRestartAckFlag)
	{
		#if (HUART_BAUD	<= HUART_115K)		
		if (++etdr_T_WaitRestartAckFlag >= start_cnt)
		#else
		if (++etdr_T_WaitRestartAckFlag >= 30)
		#endif	
		{
			etdr_T_WaitRestartAckFlag = 0;
			Transmitter_Control_State &= ~TRANSMITTER_RESTART_FLAG;
			Transmitter_Send_State |= TRANSMITTER_SEND_WAIT;
		}
	}	
	
	/*
	if (Transmitter_System_Set_Flag==0)
	{	
		if (++ExtenderT_Report_Restart_Cnt > 5)
		{
			ExtenderT_Report_Restart_Cnt = 0;
			ExtenderT_Control_Flag &= ~ExtenderT_Start_Mask;
			TRANSMITTER_Restart_Report();
		}			
		
	}	
	*/
	//if ((Transmitter_Control_State & TRANSMITTER_RESTART_FLAG) == 0x00)
	//{
		//etdr_T_WaitRestartAckFlag = 1;
		//TRANSMITTER_Restart_Report();
	//	Transmitter_Send_State |= TRANSMITTER_SEND_WAIT;
	//}

	//if (etdr_T_485FirstFlag)
	//{
	//	Extender_Token_Transmit(Transmitter_Ack_Token,(EXTENDER_CONTROL_TRANSMIT_NO_WAIT|EXTENDER_CONTROL_NONE_FREE),3,1000,1);
	//}
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	#if (SYSTEM_EXTENDER_MSC_SUPPORT)
	if ((Transmitter_Send_State & TRANSMITTER_SEND_WAIT) || ExtenderT_Device_Passthrough_Flag
	|| (ETDR_AudioOutHead != ETDR_AudioOutTail) || (EXTENDER_MSC_BulkOutCount))
	#else
	if ((Transmitter_Send_State & TRANSMITTER_SEND_WAIT) || ExtenderT_Device_Passthrough_Flag
	|| (ETDR_AudioOutHead != ETDR_AudioOutTail))
	#endif //#if (SYSTEM_EXTENDER_MSC_SUPPORT)
#else
	#if (SYSTEM_EXTENDER_MSC_SUPPORT)
	else if ((Transmitter_Send_State & TRANSMITTER_SEND_WAIT) || ExtenderT_Device_Passthrough_Flag
	|| (EXTENDER_MSC_BulkOutCount))
	#else
	else if ((Transmitter_Send_State & TRANSMITTER_SEND_WAIT) || ExtenderT_Device_Passthrough_Flag)
	#endif //#if (SYSTEM_EXTENDER_MSC_SUPPORT)
#endif //#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	{
		//(1).ACK Token as the data sequence header
		Extender_Token_Transmit(Transmitter_Ack_Token,(EXTENDER_CONTROL_TRANSMIT_NO_WAIT|EXTENDER_CONTROL_NONE_FREE|EXTENDER_CONTROL_DATA_FOLLOW),3,1000,1); //ACK

		//(2).Data Body
		if (etdr_T_WaitRestartAckFlag == 0)
		{	
			if ((Transmitter_Control_State & TRANSMITTER_RESTART_FLAG) == 0x00)
			{
				//etdr_T_WaitRestartAckFlag = 1;
				//if (++ExtenderT_Report_Restart_Cnt > 2 ) 
				{	
					printf("Report Restart\n\r");
					TRANSMITTER_Restart_Report();
				}	
			}
		}

		if (Transmitter_Control_State & TRANSMITTER_KB_LED_FLAG)
		{
#if (SYSTEM_EXTENDER_TRANSMITTER)
			Transmitter_Control_State &= ~TRANSMITTER_KB_LED_FLAG;
			ExtenderT_KVM_KB_Led_Send();
#endif
		}

		if (Transmitter_Control_State & TRANSMITTER_HOST_LED_FLAG)
		{
			Transmitter_Control_State &= ~TRANSMITTER_HOST_LED_FLAG;
			ExtenderT_HostLedControlReport();
		}

		//Check Passthrough Commands
		if (ExtenderT_Device_Passthrough_Flag) //device wait for passthrough
		{
			for (devinx=USBDC_VIRTUAL_DEVINX; devinx < USBDC_DEVICE_MAX ; devinx++)
			{
				if (ExtenderT_Device_Passthough_State[devinx])
				{
					state = ExtenderT_Device_Passthough_State[devinx];

					if (state & T_USB_PASSTHROUGH_SETUP_FLAG) // indicate send out setup token
					{
						ExtenderT_Setup_Token_Stage(devinx);
					}

					if (ExtenderT_Device_Passthough_State[devinx])
					{
						if (state & T_USB_PASSTHROUGH_DATA_OUT_FLAG) // indicate read in setup page data out.
						{
							ExtenderT_Setup_Data_Out_Stage(devinx);
						}

						if (ExtenderT_Device_Passthough_State[devinx])
						{
							if (state & T_USB_PASSTHROUGH_DATA_IN_ENABLE_FLAG)
							{
								ExtenderT_Setup_Data_In_Enable(devinx);
							}

							if (ExtenderT_Device_Passthough_State[devinx])
							{
								if (state & T_USB_PASSTHROUGH_STATUS_OUT_FLAG)
								{
									ExtenderT_Setup_Status_Stage(devinx,EXTENDER_USB_STATUS_OUT);
								}
							}
						}
					}
				}
				ExtenderT_Device_Passthrough_Flag &= ~BIT_MASK[devinx];

				if (ExtenderT_Device_Passthrough_Flag == 0x00)
					break;
			}
		}

		//Check for the device command send out q
		if (ExtenderT_Cmd_Q_Inx)
		{
			for (devinx=0; devinx < ExtenderT_Cmd_Q_Inx ; devinx++)
			{
				ExtenderT_Send_Command_Q(devinx);
			}
			ExtenderT_Cmd_Q_Inx = 0;
		}

#if (SYSTEM_EXTENDER_MSC_SUPPORT)
		if (EXTENDER_MSC_BulkOutCount)
		{
#if (SYSTEM_USB_HC_BURST)
			ExtenderT_MSC_BurstDataOutStage();
#else
			ExtenderT_MSC_BulkDataOutStage();
#endif
		}
#endif /* #if (SYSTEM_EXTENDER_MSC_SUPPORT) */
		//(3).Transmitter End Token, The final step
		//if (Transmitter_MSC_Bulk_Out_Flag == 0)

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
		// Audio OUT data in queue
		if (ETDR_AudioOutHead != ETDR_AudioOutTail)
		{
			//EA = etdrT_isr;
			ExtenderT_Audio_Data_Out_Stage(USBHC_Audio_Devinx);
		}


		if (ETDR_AudioOutHead != ETDR_AudioOutTail)
		{
			ExtenderT_Audio_Data_Out_Stage(USBHC_Audio_Devinx);
		}
#endif

		{
			TRANSMITTER_Transmit_End();
		}
	}
	else
	{
		Extender_Token_Transmit(Transmitter_Ack_Token,(EXTENDER_CONTROL_TRANSMIT_NO_WAIT|EXTENDER_CONTROL_NONE_FREE),3,1000,1);
	}

	Extender_Receive_Parser_State_Reset();
	//KVM_Select_Led_Control(1,LED_ON);
 	//P2_2 = 1;
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_USB_Set_Protocol(U8_T devinx,U8_T curinf,U8_T protocol_value)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_USB_Set_Protocol(U8_T devinx,U8_T curinf,U8_T protocol_value)
{
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bCmd = T_USB_CMD_SET_PROTOCOL_FLAG;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bIndex = Remote_DevMap[devinx].Mapping & ~USBHC_DEVICE_USED_MASK;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bValue = (curinf << 4) | protocol_value;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bLen  = 0;
	ExtenderT_Cmd_Q_Inx++;
	Transmitter_Send_State |= TRANSMITTER_SEND_WAIT;
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_USB_Send_Command(U8_T devinx,U8_T curinf,U8_T protocol_value)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_USB_Send_Command(U8_T devinx,U8_T req,U8_T value)
{
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bCmd = T_USB_CMD_PASSTHROUGH_FLAG;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bRequest = EXTENDER_USB_COMMAND;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bIndex = Remote_DevMap[devinx].Mapping & ~USBHC_DEVICE_USED_MASK;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bValue = value;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bLen   = 1;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bData  = req;
	ExtenderT_Cmd_Q_Inx++;
	Transmitter_Send_State |= TRANSMITTER_SEND_WAIT;
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_USB_IntrOutProcess(U8_T devinx, U8_T endpinx)
 * Purpose:
 * Params:
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_USB_IntrOutProcess(U8_T devinx, U8_T endpinx)
{
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bCmd = T_USB_PASSTHROUGH_INTR_OUT_FLAG;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bIndex = devinx;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bValue = endpinx;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bLen  = 0;
	ExtenderT_Cmd_Q_Inx++;
	Transmitter_Send_State |= TRANSMITTER_SEND_WAIT;
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_Send_Command_Q(U8_T index)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Send_Command_Q(U8_T index)
{
	U8_T*	point;
	U16_T	len;
	U8_T	dataLen;
	U8_T	bIndex = ExtenderT_Cmd_Q[index].bIndex;
	U8_T	bValue = ExtenderT_Cmd_Q[index].bValue;
	U8_T    bRequest = ExtenderT_Cmd_Q[index].bRequest;

	switch (ExtenderT_Cmd_Q[index].bCmd)
	{
#ifdef  DEVICE_KEEP
		case T_DEVICE_TABLE:
			//Extender_Packet_Data(Trnasmitter_Send_Data_TokenP,0,0,EXTENDER_T_DEVICE_LIST,bIndex,bValue);
			//Extender_Token_Transmit(Trnasmitter_Send_Buf,EXTENDER_CONTROL_TRANSMIT_NO_WAIT,len,1000,0);
			ExtenderT_Receiver_Send_Cmd(NULL,EXTENDER_CONTROL_TRANSMIT_NO_WAIT,EXTENDER_T_DEVICE_TABLE,ExtenderT_Device_Mount_Change,NO_VALUE,NO_LEN,NO_DATA);
			break;
#endif		
		case T_MSC_IN_WAIT:
#if (SYSTEM_MSC_DEVICE_SUPPORT)
			ExtenderT_MSC_In_Wait(bIndex,bValue);
#endif
			break;
		case T_USB_INTERVAL:
			ExtenderT_Remote_Interval(bIndex,bValue);
			break;
		case T_MSC_RESET:
#if (SYSTEM_MSC_DEVICE_SUPPORT)
			ExtenderT_Reset_Remote_MSC();
#endif
			break;
		case T_USB_CMD_SET_PROTOCOL_FLAG:
			len = Transmitter_Set_Data_Transmit_Header(0,NULL);
			if (len)
			{
				Extender_Packet_Data(Trnasmitter_Send_Data_TokenP,0,0,EXTENDER_USB_SET_PROTOCOL,bIndex,bValue);
				Extender_Token_Transmit(Trnasmitter_Send_Buf,EXTENDER_CONTROL_TRANSMIT_NO_WAIT,len,1000,0);
			}
			break;
		case T_USB_PASSTHROUGH_INTR_OUT_FLAG:
			point = USBDC_EndpBufPtr[KVM_CurrentHost][bIndex][bValue];
			dataLen = *point;
			point += OUT_BUF_OFFSET; // data point
			bIndex = Remote_DevMap[bIndex].Mapping & ~USBHC_DEVICE_USED_MASK;
			len = Transmitter_Set_Data_Transmit_Header(dataLen, NULL);
			if (len)
			{
				Extender_Packet_Data(Trnasmitter_Send_Data_TokenP, dataLen, point, EXTENDER_USB_INTR_OUT, bIndex, bValue);
				Extender_Token_Transmit(Trnasmitter_Send_Buf, EXTENDER_CONTROL_TRANSMIT_NO_WAIT, len, 1000, 0);
			}
			USBDC_REGS_Endp_ControlClear(KVM_CurrentHost, ExtenderT_Cmd_Q[index].bIndex, bValue, DA_CR_BCLR_SET);
			break;
		case T_USB_CMD_PASSTHROUGH_FLAG:
			ExtenderT_Receiver_Data_Send(NO_HEADER,EXTENDER_CONTROL_TRANSMIT_NO_WAIT,bRequest,bIndex,bValue,ExtenderT_Cmd_Q[index].bLen,ExtenderT_Cmd_Q[index].bData);
			break;
	}
}

/*----------------------------------------------------------------------------
 * Function: ExtenderT_HostLedCheck(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
void ExtenderT_HostLedCheck(U8_T pid)
{
#ifdef PCT_MUA22
	pid = 0;
#else
	if (USBDC_UpPortState[pid] & USBDC_ROOTHUB_ATTACHED_MASK)
	{
#ifdef PWAYTEK_01
		ExtenderT_Maintain_HostState_Led(LED_ON);
#else
		KVM_Select_Led_Control(pid, LED_ON); //Orange LED
#endif	/* #ifdef PWAYTEK_01 */
	}
	else
	{
#ifdef PWAYTEK_01
		ExtenderT_Maintain_HostState_Led(LED_OFF);
#else
		KVM_Select_Led_Control(pid, LED_OFF); //Orange LED
#endif
	}
#endif /* #ifdef PWAYTEK_01 */
	Transmitter_Control_State |= TRANSMITTER_HOST_LED_FLAG;
	Transmitter_Send_State |= TRANSMITTER_SEND_WAIT;
}

/*----------------------------------------------------------------------------
 * Function: ExtenderT_HostLedControlReport(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
void ExtenderT_HostLedControlReport(void)
{
	U16_T	len;
	U8_T	bLedStatus;

	if (USBDC_UpPortState[KVM_CurrentHost] & USBDC_ROOTHUB_ATTACHED_MASK)
		bLedStatus = LED_ON;
	else
		bLedStatus = LED_OFF;

	len = Transmitter_Set_Data_Transmit_Header(0,NULL);
	if (len)
	{
		Extender_Packet_Data(Trnasmitter_Send_Data_TokenP, 0, NULL, EXTENDER_T_LED_CONTROL, 0, bLedStatus);
//		printf("EXTENDER_T_LED_CONTROL\n\r");
		Extender_Token_Transmit(Trnasmitter_Send_Buf, EXTENDER_CONTROL_TRANSMIT_NO_WAIT, len, 1000, 0);
	}
}

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_Audio_Data_Out_Stage
 * Purpose:
 * Params:
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Audio_Data_Out_Stage(U8_T devinx)
{
	U8_T XDATA*	buf;
	U8_T		remote_devinx;
	U8_T		data_len;
	U8_T		len;
	U8_T		offset;

	Extender_Data_Packet_Def XDATA*	header;

	buf = (U8_T XDATA*)&ETDR_AudioOutBuf[ETDR_AudioOutTail][0];
	header = (Extender_Data_Packet_Def XDATA*)buf;

	offset = sizeof(Extender_Data_Packet_Def);
	data_len = ETDR_AudioOutBuf[ETDR_AudioOutTail][offset] +2;
	len = data_len + offset;

	remote_devinx = Remote_DevMap[devinx].Mapping & ~USBHC_DEVICE_USED_MASK;
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
	header->bRequest = EXTENDER_USB_ISO_OUT;
	header->bIndex = remote_devinx;
	header->bValue = 0;
#if (EXTENDER_TRANSACTION_SERIAL_ID)
	Trainsmit_Table[Extender_Current_TransId] = EXTENDER_Transmit_InP;
	header->bTid   = Extender_Current_TransId;
	Extender_Current_TransId++;
	if (Extender_Current_TransId >= EXTENDER_TRANSMIT_FIFO_MAX)
		Extender_Current_TransId = 0;
#endif

	Extender_Token_Transmit(buf, (EXTENDER_CONTROL_TRANSMIT_NO_WAIT|EXTENDER_CONTROL_NONE_FREE), len, 1000, 0);
#if (SYSTEM_EA_CONTROL)
	etdrT_isr = EA;
	EA = 0;
#endif
	if (ETDR_AudioOutTailSkipFlag)
	{
		ETDR_AudioOutTailSkipFlag = 0;
		ETDR_AudioOutTail += 2;
	}
	else
	{
		ETDR_AudioOutTail++;
	}
#if (SYSTEM_EA_CONTROL)
	EA = etdrT_isr;
#endif
	ETDR_AudioOutTail &= EXTENDER_AUDIO_OUT_BUF_MASK;
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_Audio_Data_In_Stage
 * Purpose:
 * Params:
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Audio_Data_In_Stage(void)
{
	U8_T		len;
	U8_T XDATA*	pBuf;
	
	
	//2017-08-25 
	if (USBHC_Audio_Devinx == 0) 
	{
		return;
	}		
	
	if (USBDC_Device[USBHC_Audio_Devinx].DevAddr[KVM_CurrentUSBAudio] == 0x00)
	{
		return;
	}

	len = Extender_Setup.Data_Token.iLen;
	if (len > 192)
	{
		len = 192;
		printf("ERROR Audio IN LEN erro\n\r");
	}
	pBuf = ETDR_AudioInBuf[ETDR_AudioInHead];
#if (SYSTEM_EA_CONTROL)
	etdrT_isr = EA;
	EA = 0;
#endif
	ETDR_AudioInHead++;
	ETDR_AudioInHead &= EXTENDER_AUDIO_IN_BUF_MASK;
	if (ETDR_AudioInHead == ETDR_AudioInTail)
	{
		ETDR_AudioInHead--;
		ETDR_AudioInHead &= EXTENDER_AUDIO_IN_BUF_MASK;
#if (SYSTEM_EA_CONTROL)
		EA = etdrT_isr;
#endif
		return;
	}

	Extender_CopyDataFromRxBuffer(Extender_Setup.iRecBuf_Index, len, pBuf);

	if (!ETDR_AudioInDcDoingFlag)
	{
#if (SYSTEM_EA_CONTROL)
		EA = etdrT_isr;
#endif
		if (((ETDR_AudioInHead - ETDR_AudioInTail) & EXTENDER_AUDIO_IN_BUF_MASK) >= EXTENDER_AUDIO_IN_WATER_LEVEL)
		{
			USBDC_Index[KVM_CurrentUSBAudio][USBHC_Audio_Devinx][USBHC_Audio_In_Endpinx].Endp_Addr_Lsb = ETDR_AudioInBufAddr[ETDR_AudioInTail][0];
			USBDC_Index[KVM_CurrentUSBAudio][USBHC_Audio_Devinx][USBHC_Audio_In_Endpinx].Endp_Addr_Msb = ETDR_AudioInBufAddr[ETDR_AudioInTail][1];
			len = DA_CR_BVLD_SET | (USBHC_Audio_In_Endpinx &0x07);
			switch (KVM_CurrentUSBAudio)
			{
				case 0:
					_USBDC_DC0DR_SFR(len);
					_USBDC_DC0CIR_SFR(DA_CR[USBHC_Audio_Devinx]);
					break;
				case 1:
					_USBDC_DC1DR_SFR(len);
					_USBDC_DC1CIR_SFR(DA_CR[USBHC_Audio_Devinx]);
					break;
				case 2:
					_USBDC_DC2DR_SFR(len);
					_USBDC_DC2CIR_SFR(DA_CR[USBHC_Audio_Devinx]);
					break;
				case 3:
					_USBDC_DC3DR_SFR(len);
					_USBDC_DC3CIR_SFR(DA_CR[USBHC_Audio_Devinx]);
					break;
			}

			ETDR_AudioInTail++;
			ETDR_AudioInTail &= EXTENDER_AUDIO_IN_BUF_MASK;
			ETDR_AudioInDcDoingFlag = 1;
		}
	}
#if (SYSTEM_EA_CONTROL)
	else
	{
		EA = etdrT_isr;
	}
#endif
}
#endif

#if (SYSTEM_EXTENDER_MSC_SUPPORT)
/*----------------------------------------------------------------------------
 * Function: void ExtenderT_MSC_DcInBufClear(U8_T port,U8_T devinx,U8_T endpinx)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
void ExtenderT_MSC_DcInBufClear(U8_T port,U8_T devinx,U8_T endpinx)
{
	endpinx = (DA_CR_IN_CLEAR | (endpinx & 0x07));
	switch (port)
	{
		case 0:
			_USBDC_DC0DR_SFR(endpinx);
			_USBDC_DC0CIR_SFR(DA_CR[devinx]);
			break;
		case 1:
			_USBDC_DC1DR_SFR(endpinx);
			_USBDC_DC1CIR_SFR(DA_CR[devinx]);
			break;
		case 2:
			_USBDC_DC2DR_SFR(endpinx);
			_USBDC_DC2CIR_SFR(DA_CR[devinx]);
			break;
		case 3:
			_USBDC_DC3DR_SFR(endpinx);
			_USBDC_DC3CIR_SFR(DA_CR[devinx]);
			break;
	}
}

/*----------------------------------------------------------------------------
 * Function: void ExtenderT_MSC_DcBulkInStallHandle(U8_T devinx)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
void ExtenderT_MSC_DcBulkInStallHandle(U8_T devinx)
{
	U8_T	endpinx;

	/* Stall packet does not have any data payload, can ignore the data payload copy to the EXTENDER_MSC_BulkInBuf[].bBuf).
	   skip the Extender_CopyDataFromRxBuffer() */
	if (EXTENDER_MSC_StallFlag == TD_DIR_TOKEN_OUT)
	{
		endpinx = USB_PDevice[devinx].MSC->Out_Endpinx;
		USB_PDevice[devinx].MSC->Toggle_Out = 0;
	}
	else
	{
		endpinx = USB_PDevice[devinx].MSC->In_Endpinx;
		USB_PDevice[devinx].MSC->Toggle_In = 0;
	}
//	printf("T_Stall\n\r");
	USBDC_HAL_Endp_Stalled(KVM_CurrentMSC, devinx, endpinx);

	USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_STALL;

	EXTENDER_MSC_StallFlag = 0;
}

#if (SYSTEM_USB_HC_BURST)
/*----------------------------------------------------------------------------
 * Function: ExtenderT_MSC_BulkDataOutStage(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
void ExtenderT_MSC_BurstDataOutStage(void)
{
	Extender_Data_Packet_Def	*pToken;
	U16_T						dataLen, crcLen, extLen;
	U8_T						extRequest, remoteIndex, crc16[2];

	Transmitter_MSC_Bulk_Out_Flag = 0;
	if (EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutTail].bufUsingFlag == 0)
	{
		return;
	}
	/*
	if (EXTENDER_MSC_BurstWait)
	{
		return;
	}
	*/
	pToken = &(EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutTail].Data_Token);
	dataLen = EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutTail].payloadLen;
	extRequest = EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutTail].reqType;
	extLen = Transmitter_Set_Data_Transmit_Header(dataLen, (U8_T *)pToken);

	if (extLen)
	{
		remoteIndex = Remote_DevMap[EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutTail].devIndex].Mapping & ~USBHC_DEVICE_USED_MASK;

		/* Fill the extender data token header */
		if (Transmitter_Data_Toggle==0)
		{
			pToken->bData_Token = EXTENDER_DATA0_TOKEN; //data pid
			Transmitter_Data_Toggle=1;
		}
		else
		{
			pToken->bData_Token = EXTENDER_DATA1_TOKEN; //data pid
			Transmitter_Data_Toggle=0;
		}
		pToken->iLen = dataLen;
		pToken->bRequest = extRequest;
		pToken->bIndex = remoteIndex; // for device index
		pToken->bValue = EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutTail].serial; // for port id
		//pToken->bTid   = EXTENDER_Transmit_InP;
		//Calculate CRC16
		crcLen = dataLen + sizeof(Extender_Data_Packet_Def);
		if ((extRequest & 0xf0) != EXTENDER_USB_CRC_SKIP_MASK)
		{
			Externder_CRC_Calculate(0, crcLen, (U8_T *)pToken, crc16, 1);
		}
		EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutTail].bBuf[dataLen + 0] = crc16[0];
		EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutTail].bBuf[dataLen + 1] = crc16[1];
		Extender_Token_Transmit((U8_T *)pToken,
								(EXTENDER_CONTROL_TRANSMIT_NO_WAIT|EXTENDER_CONTROL_NONE_FREE|EXTENDER_CONTROL_CLEAR_BUF),
								 extLen,
								 100,
								 0);
		if (EXTENDER_MSC_BulkOutCount)
			EXTENDER_MSC_BulkOutCount--;
		//EXTENDER_MSC_BurstWait = 1;
	}
	Transmitter_MSC_Bulk_Out_Flag = 1;
}

/*----------------------------------------------------------------------------
 * Function: void ExtenderT_MSC_BurstOutComplete(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
void ExtenderT_MSC_BurstOutComplete(void)
{
	U8_T	*pPacket, devinx, endpinx;
	U16_T	iLen;
	U32_T	addrBuf;

	devinx = EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutTail].devIndex;
	/* Updated the ExtenderT buffer after sending to ExtenderR */
	//printf("[%bu]",EXTENDER_MSC_BulkOutTail);
	EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutTail].bufUsingFlag = 0;
	EXTENDER_MSC_BulkOutTail ++;
	EXTENDER_MSC_BulkOutTail &= EXTENDER_MSC_BUF_MASK;	
	EXTENDER_MSC_BurstOutWait_Clear_Flag = 0; //receiver allow out again	

	//Handle Condition 1-The transmitter part:is there any buffer are waiting to send oust
	if (EXTENDER_MSC_BurstOutWait_Flag) //receiver allow out again	
	{
		if (EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutTail].bufUsingFlag) //the buffer is ready now
		{
			EXTENDER_MSC_BulkOutCount++;			
			Transmitter_Send_State |= TRANSMITTER_SEND_WAIT; 
			EXTENDER_MSC_BurstOutWait_Clear_Flag = 1; //wait host send back the EXTENDER_USB_MSC_DATA_OUT
		}	
	}	
	
	//Handle Condition 2- the DC part:is there any data are waint in DC for moving to transmitter buffer? 
	if (USB_PDevice[devinx].MSC->BURST_Out_State & BURST_OUT_WAIT)
	{
		USB_PDevice[devinx].MSC->BURST_Out_State &= ~BURST_OUT_WAIT;

		if (EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].bufUsingFlag == 0)
		{
			pPacket = EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].bBuf;
			addrBuf = pPacket;
			endpinx  = USB_PDevice[devinx].MSC->Out_Endpinx & 0x0F;
			USBDC_Index[KVM_CurrentMSC][devinx][endpinx].Endp_Addr_Lsb = addrBuf & 0x00FF;  // endpoint address
			USBDC_Index[KVM_CurrentMSC][devinx][endpinx].Endp_Addr_Msb = (addrBuf & 0xFF00) >> 8;  // endpoint address
			USBDC_EndpBufPtr[KVM_CurrentMSC][devinx][endpinx] = pPacket;

			//5.Set the burst length
			if (USB_PDevice[devinx].MSC->Control.Total_Length)
			{
				if (USB_PDevice[devinx].MSC->Control.Total_Length > MSC_Class_Page_Size)
				{
					USB_PDevice[devinx].MSC->Control.Data_Length = MSC_Class_Page_Size;
				}
				else
				{
					USB_PDevice[devinx].MSC->Control.Data_Length = USB_PDevice[devinx].MSC->Control.Total_Length;
				}
			}
			else
			{
				USB_PDevice[devinx].MSC->Control.Data_Length = USBHC_MSC_BOT_CBW_LENGTH;
				USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_CSW_REQUEST; //Next,wait for CSW
			}
			iLen = USB_PDevice[devinx].MSC->Control.Data_Length;
			//pPacket[0] = (U8_T)(iLen & 0x00FF);
			//pPacket[1] = (U8_T)((iLen & 0x0300) >> 8);
			USBHC_MSC_Length_To_Buf(pPacket,iLen);
			
			
			USBDC_REGS_Endp_ControlClear(KVM_CurrentMSC,devinx,endpinx,DA_CR_BCLR_SET); // for the out can receive data again
		}
		else
		{
			printf("BURST_OUT_WAIT Error\n\r");
		}
	}
}

/*----------------------------------------------------------------------------
 * Function: U8_T ExtenderT_MSC_DcBurstOutHandle(U8_T devinx)
 * Purpose :
 * Params  :
 * Returns : 0-Can not receive the data.
 * Note    :
 *----------------------------------------------------------------------------
 */
U8_T ExtenderT_MSC_DcBurstOutHandle(U8_T devinx)
{
	//Check extender transmitter MSC buffer
	if (EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].bufUsingFlag)
	{
		printf("!!!T_MSC_BurstOut Full\n\r");
		return FALSE;
	}

	if ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00)
	{
		 printf("MSC ERROR:Device not exist\n\r");
		 return FALSE;
	}

	if ((USBDC_UpPortState[KVM_CurrentMSC] & USBDC_ROOTHUB_ATTACHED_MASK) == 0x00)
	{
		printf("MSC ERROR:Not attached\n\r");
		return FALSE;
	}

	// Check the Buffer Data to process
	ExtenderT_MSC_BurstOutDataCheck(devinx);


	if (USB_PDevice[devinx].MSC->BURST_Out_State & (BURST_ERROR|BURST_COMPLETE))
	{
		//if burst handle is error or complete
		USB_PDevice[devinx].MSC->BURST_Out_State &= ~(BURST_ERROR|BURST_COMPLETE); //clear the complete & error flag
		return 1; // return 1 to clear the DC out buffer for receiving.
	}
	return 0;
}

/*----------------------------------------------------------------------------
 * Function: U8_T ExtenderT_MSC_BurstOutDataCheck(U8_T devinx)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
U8_T ExtenderT_MSC_BurstOutDataCheck(U8_T devinx)
{
	MSC_BOT_CBW_TypeDef		*pCbw;
	U16_T					iLen;
	U8_T					*pPacket, pktNum, endpinx, extReqType;
	U32_T					addrBuf;

	pPacket = EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].bBuf;
	iLen = (U16_T)pPacket[0] & 0x00FF;
	iLen += (U16_T)(pPacket[1] & 0x03) << 8;
	pktNum = (pPacket[1] & 0xFC) >> 2;
	if (pktNum > 1)
		iLen = ((pktNum - 1) * USB_HC_SINGLE_BULK_BUF_SIZE) + iLen;

	USB_PDevice[devinx].MSC->Control.Data_Length = iLen;

	pCbw = (MSC_BOT_CBW_TypeDef *)(pPacket + 2);

	switch (USB_PDevice[devinx].MSC->BOT_State)
	{
		case MSC_BOT_IDLE: //wait for CBW
			//1.Check the CBW block data length
			if ((iLen != USBHC_MSC_BOT_CBW_LENGTH) || (pCbw->dSignature != USBHC_MSC_BOT_CBW_SIGNATURE))
			{
				USB_PDevice[devinx].MSC->BURST_Out_State = BURST_ERROR;
				printf("MSC ERROR:CBW Decode Error\n\r");
				Extender_MSC_Init(0);
				USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_IDLE;
				break;
			}
			ExtenderT_MSC_DcInBufClear(KVM_CurrentMSC, devinx, USB_PDevice[devinx].MSC->In_Endpinx);
			USB_PDevice[devinx].MSC->Control.Total_Length = Endian_32_Convert(pCbw->dDataLength);
			USB_PDevice[devinx].MSC->Control.Current_Length = 0;
			//2.Fill the RS485 buffer header to send out
			EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].bufUsingFlag = 1;
			EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].devIndex = devinx;
			EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].payloadLen = iLen + 2;
			EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].reqType = EXTENDER_USB_MSC_CBW;
			EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].serial = burstout_cnt=0;
			 
			//EXTENDER_MSC_BulkOutCount ++;			

			//3.Get the burst data length for next burst
			if (USB_PDevice[devinx].MSC->Control.Total_Length)
			{
				USB_PDevice[devinx].MSC->Control.Data_Length = 0;
				if (USB_PDevice[devinx].MSC->Control.Total_Length > MSC_Class_Page_Size)
				{
					USB_PDevice[devinx].MSC->Control.Data_Length = MSC_Class_Page_Size;
				}
				else
				{
					USB_PDevice[devinx].MSC->Control.Data_Length = USB_PDevice[devinx].MSC->Control.Total_Length;
				}

				if (pCbw->bmFlags & BOT_DIR_IN)
				{					
					USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_DATA_IN; //Data-In from the device to the host
					iLen = USBHC_MSC_BOT_CBW_LENGTH;
				}
				else
				{
					if (USB_PDevice[devinx].MSC->Control.Total_Length)
					{							
						burstout_cnt++; //for debug	
					}	
					USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_DATA_OUT; //Data-Out from host to the device
					iLen = USB_PDevice[devinx].MSC->Control.Data_Length;
				}
			}
			else
			{
				USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_CSW_REQUEST; //wait HC issue a IN Token in Bulk endp IN
				USB_PDevice[devinx].MSC->Control.Data_Length = USBHC_MSC_BOT_CBW_LENGTH;
				iLen = USBHC_MSC_BOT_CBW_LENGTH;
			}

			//4.Configure the next buffer to DC endpoint buffer for data out
			EXTENDER_MSC_BulkOutHead ++;
			EXTENDER_MSC_BulkOutHead &= EXTENDER_MSC_BUF_MASK;
			if (EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].bufUsingFlag == 0)
			{
				pPacket = EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].bBuf;
				addrBuf = pPacket;
				endpinx  = USB_PDevice[devinx].MSC->Out_Endpinx & 0x0F;
				USBDC_Index[KVM_CurrentMSC][devinx][endpinx].Endp_Addr_Lsb = addrBuf & 0x00FF;  // endpoint address
				USBDC_Index[KVM_CurrentMSC][devinx][endpinx].Endp_Addr_Msb = (addrBuf & 0xFF00) >> 8;  // endpoint address
				USBDC_EndpBufPtr[KVM_CurrentMSC][devinx][endpinx] = pPacket;
				
				//5.Set the burst length
				if (iLen > MSC_Class_Page_Size)
				{
					iLen = MSC_Class_Page_Size;
				}
				//pPacket[0] = (U8_T)(iLen & 0x00FF);
				//pPacket[1] = (U8_T)((iLen & 0x0300) >> 8);	
				USBHC_MSC_Length_To_Buf(pPacket,iLen);			
				USB_PDevice[devinx].MSC->BURST_Out_State |= BURST_COMPLETE;													
			}
			else
			{
				USB_PDevice[devinx].MSC->BURST_Out_State |= BURST_OUT_WAIT;
			}
			
			extReqType = EXTENDER_USB_MSC_CBW;
			if (EXTENDER_MSC_BurstOutWait_Clear_Flag == 0) //if receiver is read to receive out
			{
				EXTENDER_MSC_BulkOutCount++;			
				Transmitter_Send_State |= TRANSMITTER_SEND_WAIT; 
				EXTENDER_MSC_BurstOutWait_Clear_Flag = 1; //wait host send back the EXTENDER_USB_MSC_DATA_OUT
			}
			else
			{ //else just put this into wait condition.
				EXTENDER_MSC_BurstOutWait_Flag = 1;
			}						
			break;
		case MSC_BOT_DATA_OUT:
			//1.Maintain total data length
			if (USB_PDevice[devinx].MSC->Control.Total_Length > iLen)
				USB_PDevice[devinx].MSC->Control.Total_Length -= iLen;
			else
				USB_PDevice[devinx].MSC->Control.Total_Length = 0;
//			printf("T_Burst Out Len = %lx\n\r", USB_PDevice[devinx].MSC->Control.Total_Length);
			//2.Fill the RS485 buffer header to send out
			EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].bufUsingFlag = 1;
			EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].devIndex = devinx;
			EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].payloadLen = iLen + 2;
			EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].reqType = EXTENDER_USB_MSC_DATA_OUT;
			EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].serial = burstout_cnt;
			burstout_cnt++;
			//EXTENDER_MSC_BulkOutCount ++;

			//3.Get the burst data length for next burst
			if (USB_PDevice[devinx].MSC->Control.Total_Length)
			{
				if (USB_PDevice[devinx].MSC->Control.Total_Length > (U16_T)MSC_Class_Page_Size)
				{
					USB_PDevice[devinx].MSC->Control.Data_Length = (U16_T)MSC_Class_Page_Size;
				}
				else
				{
					USB_PDevice[devinx].MSC->Control.Data_Length = USB_PDevice[devinx].MSC->Control.Total_Length;
				}
			}
			else
			{
				USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_CSW_REQUEST; //wait HC issue a IN Token in Bulk endp IN
				USB_PDevice[devinx].MSC->Control.Data_Length = USBHC_MSC_BOT_CSW_LENGTH;
			}			
			//4.Configure the next buffer to DC endpoint buffer for data out
			EXTENDER_MSC_BulkOutHead ++;
			EXTENDER_MSC_BulkOutHead &= EXTENDER_MSC_BUF_MASK;
			if (EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].bufUsingFlag == 0)
			{
				pPacket = EXTENDER_MSC_BulkOutBuf[EXTENDER_MSC_BulkOutHead].bBuf;
				addrBuf = pPacket;
				endpinx  = USB_PDevice[devinx].MSC->Out_Endpinx & 0x0F;
				USBDC_Index[KVM_CurrentMSC][devinx][endpinx].Endp_Addr_Lsb = addrBuf & 0x00FF;  // endpoint address
				USBDC_Index[KVM_CurrentMSC][devinx][endpinx].Endp_Addr_Msb = (addrBuf & 0xFF00) >> 8;  // endpoint address
				USBDC_EndpBufPtr[KVM_CurrentMSC][devinx][endpinx] = pPacket;

				//5.Set the burst length
				iLen = USB_PDevice[devinx].MSC->Control.Data_Length;
				if (iLen > MSC_Class_Page_Size)
				{
					iLen = MSC_Class_Page_Size;
				}
				//pPacket[0] = (U8_T)(iLen & 0x00FF);
				//pPacket[1] = (U8_T)((iLen & 0x0300) >> 8);
				USBHC_MSC_Length_To_Buf(pPacket,iLen);
				USB_PDevice[devinx].MSC->BURST_Out_State |= BURST_COMPLETE;
			}
			else
			{
				USB_PDevice[devinx].MSC->BURST_Out_State |= BURST_OUT_WAIT;				
			}
			
			if (EXTENDER_MSC_BurstOutWait_Clear_Flag == 0) //if receiver is read to receive out
			{
				EXTENDER_MSC_BulkOutCount++;			
				Transmitter_Send_State |= TRANSMITTER_SEND_WAIT; 
				EXTENDER_MSC_BurstOutWait_Clear_Flag = 1; //wait host send back the EXTENDER_USB_MSC_DATA_OUT
			}
			else
			{ //else just put this into wait condition.
				EXTENDER_MSC_BurstOutWait_Flag = 1;
			}			
			extReqType = EXTENDER_USB_MSC_DATA_OUT;
			//Transmitter_Send_State |= TRANSMITTER_SEND_WAIT;			
			break;
		case MSC_BOT_DATA_IN:
			break;
		default:
			printf("Burst OUT_UNKNOW Status=%02x\n\r",(U16_T)USB_PDevice[devinx].MSC->BOT_State);
			break;
	}

	return extReqType;
}

/*----------------------------------------------------------------------------
 * Function: void ExtenderT_MSC_DcBurstInHandle(U8_T devinx)
 * Purpose :
 * Params  :
 * Returns : 0-Can not receive the data.
 * Note    :
 *----------------------------------------------------------------------------
 */
void ExtenderT_MSC_DcBurstInHandle(U8_T devinx)
{
	U8_T	*pInPkt, endpinx;
	U32_T	bufAddr;
	U8_T    remote_devinx;

	if (EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].bufUsingFlag == 0)
	{
		printf("Bulk In DC Error\n\r");
		return;
	}

	EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].bufUsingFlag = 0;
	//printf("(A:%bu)",EXTENDER_MSC_BulkInTail);
	EXTENDER_MSC_BulkInTail ++;
	EXTENDER_MSC_BulkInTail &= EXTENDER_MSC_BUF_MASK;

	if (USB_PDevice[devinx].MSC->BURST_In_State & BURST_IN_WAIT)
	{
		USB_PDevice[devinx].MSC->BURST_In_State &= ~BURST_IN_WAIT;

		//5.Configure the current burst in buffer to DC endpoint buffer
		pInPkt = EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].bBuf;
		endpinx  = USB_PDevice[devinx].MSC->In_Endpinx & 0x0F;
		bufAddr = (U32_T)pInPkt;
		USBDC_Index[KVM_CurrentMSC][devinx][endpinx].Endp_Addr_Lsb = bufAddr & 0x00FF;
		USBDC_Index[KVM_CurrentMSC][devinx][endpinx].Endp_Addr_Msb = (bufAddr & 0xFF00) >> 8;
		USBDC_EndpBufPtr[KVM_CurrentMSC][devinx][endpinx] = pInPkt;

		//6.Inform the DC to move data
		USBDC_REGS_Endp_ControlSet(KVM_CurrentMSC, devinx, endpinx, DA_CR_BVLD_SET);
	}
	else
	{
		if (EXTENDER_MSC_BulkInWaitHead != EXTENDER_MSC_BulkInWaitTail)
		{//means there is one Bulk in data wait for handle
			if (EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].bufUsingFlag == 0x00)
			{
				pInPkt = EXTENDER_MSC_BulkInWait[EXTENDER_MSC_BulkInWaitTail]+sizeof(Extender_Data_Packet_Def);
				ExtenderT_MSC_BurstDataInSendToDC((Extender_Data_Packet_Def *)EXTENDER_MSC_BulkInWait[EXTENDER_MSC_BulkInWaitTail],pInPkt);
				//printf("Clean Wait Tail(%bu/%bu)\n\r",EXTENDER_MSC_BulkInWaitTail,EXTENDER_MSC_BulkInWaitHead);
				malloc_free(EXTENDER_MSC_BulkInWait[EXTENDER_MSC_BulkInWaitTail]);
				EXTENDER_MSC_BulkInWaitTail++;
				if (EXTENDER_MSC_BulkInWaitTail == EXTENDER_MSC_BulkInWaitHead)
				{
					EXTENDER_MSC_BulkInWaitHead = EXTENDER_MSC_BulkInWaitTail = 0;
					if (Transmitter_MSC_In_Wait_Flag)
					{
						remote_devinx = Remote_DevMap[USBHC_MSC_Devinx].Mapping & ~USBHC_DEVICE_USED_MASK;
						ExtenderT_Send_Cmd_To_Receiver(T_MSC_IN_WAIT,EXTENDER_USB_MSC_IN_WAIT,remote_devinx,0);
						Transmitter_MSC_In_Wait_Flag = 0;
					}
				}
			}
		}
		else
		{
			EXTENDER_MSC_BulkInWaitHead = EXTENDER_MSC_BulkInWaitTail = 0;
		}
	}
}

/*-----------------------------------------------------------------------------------
 * void ExtenderT_MSC_BurstDataInSendToDC(Extender_Data_Packet_Def *setup,U8_T *pInPkt)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *-----------------------------------------------------------------------------------*/
void ExtenderT_MSC_BurstDataInSendToDC(Extender_Data_Packet_Def *setup,U8_T *pInPkt)
{
	U16_T	iLen, dataLen;
	U8_T	bReq, devinx, remote_devinx, endpinx;
	U32_T	bufAddr;

	iLen = setup->iLen;
	bReq = setup->bRequest;
	remote_devinx = setup->bIndex;
	devinx = Remote_Reserve_DevMap[remote_devinx];
	//pInPkt = EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].bBuf;
	dataLen = iLen - 2;

	//2.Check the burst bulk in data
	/*
	EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].bBuf[0] is the burst length low byte;
	EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].bBuf[1] is the burst length high byte;
	*/
	ExtenderT_MSC_BurstInDataCheck(devinx, (pInPkt + 2), dataLen);

	//3.Updated the Bulk In buffer information
	EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].payloadLen = iLen;
	EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].reqType = bReq;
	EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].devIndex = devinx;
	EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].bufUsingFlag = 1;	
	EXTENDER_MSC_BulkInHead ++;
	EXTENDER_MSC_BulkInHead &= EXTENDER_MSC_BUF_MASK;
	//4.Check the next Burst In buffer is busy??
	if (EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].bufUsingFlag)
	{		
		USB_PDevice[devinx].MSC->BURST_In_State |= BURST_IN_WAIT;
	}
	else
	{
		//5.Configure the current burst in buffer to DC endpoint buffer
		pInPkt = EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].bBuf;
		endpinx  = USB_PDevice[devinx].MSC->In_Endpinx & 0x0F;
		bufAddr = (U32_T)pInPkt;
		USBDC_Index[KVM_CurrentMSC][devinx][endpinx].Endp_Addr_Lsb = bufAddr & 0x00FF;
		USBDC_Index[KVM_CurrentMSC][devinx][endpinx].Endp_Addr_Msb = (bufAddr & 0xFF00) >> 8;
		USBDC_EndpBufPtr[KVM_CurrentMSC][devinx][endpinx] = pInPkt;

		//6.Inform the DC to move data		
		USBDC_REGS_Endp_ControlSet(KVM_CurrentMSC, devinx, endpinx, DA_CR_BVLD_SET);
	}
}

/*----------------------------------------------------------------------------
 * Function: void ExtenderT_MSC_BurstDataInHandle(Extender_Cmd_Handle_Def *setup)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
void ExtenderT_MSC_BurstDataInHandle(Extender_Cmd_Handle_Def *setup)
{
	U16_T	iLen;
	U16_T   buf_len;
	U8_T	*pInPkt;
	U8_T    remote_devinx,devinx;

	iLen = setup->Data_Token.iLen;

	//There are 2 condition need put the data into wait queue
	//1.The wait queue has bee activated.
	//2.there is no buffer for DC output.
	remote_devinx = setup->Data_Token.bIndex;
	devinx = Remote_Reserve_DevMap[remote_devinx];
	USB_PDevice[devinx].MSC->Control.Current_Length += (iLen-2);
	
	//printf("[%02bu]I=%d\n\r",setup->Data_Token.bValue,(U16_T)USB_PDevice[devinx].MSC->Control.Current_Length);
	if ((EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].bufUsingFlag) || (EXTENDER_MSC_BulkInWaitHead!=EXTENDER_MSC_BulkInWaitTail))
	{
		//printf("(MSC_ERROR):IN_FULL(H:%bu,T:%bu)(%d)[%bu]\n\r",EXTENDER_MSC_BulkInHead,EXTENDER_MSC_BulkInTail,setup->Data_Token.iLen,EXTENDER_MSC_BulkInWaitHead);
		buf_len = sizeof(Extender_Data_Packet_Def)+iLen;
		EXTENDER_MSC_BulkInWait[EXTENDER_MSC_BulkInWaitHead] = m_malloc(buf_len,34);
		if (EXTENDER_MSC_BulkInWait[EXTENDER_MSC_BulkInWaitHead] != NULL)
		{
			memcpy(EXTENDER_MSC_BulkInWait[EXTENDER_MSC_BulkInWaitHead],(U8_T *)(&setup->Data_Token),sizeof(Extender_Data_Packet_Def));
			//Disp_Str(EXTENDER_MSC_BulkInWait[EXTENDER_MSC_BulkInWaitHead],sizeof(Extender_Data_Packet_Def));
			pInPkt = EXTENDER_MSC_BulkInWait[EXTENDER_MSC_BulkInWaitHead]+sizeof(Extender_Data_Packet_Def);
			Extender_CopyDataFromRxBuffer(setup->iRecBuf_Index,iLen,pInPkt);
			EXTENDER_MSC_BulkInWaitHead++;
			if (Transmitter_MSC_In_Wait_Flag == 0)
			{
				 ExtenderT_Send_Cmd_To_Receiver(T_MSC_IN_WAIT,EXTENDER_USB_MSC_IN_WAIT,remote_devinx,1);
				 Transmitter_MSC_In_Wait_Flag = 1;
			}
		}
		return;
	}
	//1.Retrieve the burst in data from RS485
	pInPkt = EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].bBuf;
	//ExtenderT_MSC_BurstDataInSendToDC(&setup->Data_Token,pInPkt);
	//return;

	//iLen = setup->Data_Token.iLen;
	//bReq = setup->Data_Token.bRequest;
	//remote_devinx = Extender_Setup.Data_Token.bIndex;
	//remote_devinx = setup->Data_Token.bIndex;
	//devinx = Remote_Reserve_DevMap[remote_devinx];
	//pInPkt = EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInHead].bBuf;
	//dataLen = iLen - 2;
	//printf("(bi:%d)",iLen);
	Extender_CopyDataFromRxBuffer(setup->iRecBuf_Index, iLen, pInPkt);

	//2.Check the burst bulk in data
	/*
	EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].bBuf[0] is the burst length low byte;
	EXTENDER_MSC_BulkInBuf[EXTENDER_MSC_BulkInTail].bBuf[1] is the burst length high byte;
	*/
	ExtenderT_MSC_BurstDataInSendToDC(&setup->Data_Token,pInPkt);
}

/*----------------------------------------------------------------------------
 * Function: U8_T ExtenderT_MSC_BurstInDataCheck(U8_T devinx, U8_T *pBuf, U16_T len)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *----------------------------------------------------------------------------
 */
U8_T ExtenderT_MSC_BurstInDataCheck(U8_T devinx, U8_T *pBuf, U16_T len)
{
	MSC_BOT_CSW_TypeDef		*pCsw;
	U8_T					extReqType = 0;

	pCsw = (MSC_BOT_CSW_TypeDef *)pBuf;
	switch (USB_PDevice[devinx].MSC->BOT_State)
	{
		case MSC_BOT_CSW_REQUEST:
			if (len == USBHC_MSC_BOT_CSW_LENGTH)
			{
				if (pCsw->dSignature == USBHC_MSC_BOT_CSW_SIGNATURE)
				{
					USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_IDLE;
				}
			}
			else
			{
				USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_END;
			}
			extReqType = EXTENDER_USB_MSC_CSW;
			break;
		case MSC_BOT_DATA_IN:
			if (len == USBHC_MSC_BOT_CSW_LENGTH)
			{
				if (pCsw->dSignature == USBHC_MSC_BOT_CSW_SIGNATURE)
				{
					USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_IDLE;
				}
			}
			else
			{
				//Check the remained data length
				USB_PDevice[devinx].MSC->Control.Total_Length -= len; //maintain total data length
				if (USB_PDevice[devinx].MSC->Control.Total_Length == 0)
					USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_CSW_REQUEST;
			}
			extReqType = EXTENDER_USB_MSC_DATA_IN;
			break;
		default:
			printf("BurstIn UNKNOW Status=%bx\n\r",(U16_T)USB_PDevice[devinx].MSC->BOT_State);
			break;
	}

	return extReqType;
}

#endif //#if (SYSTEM_USB_HC_BURST)
#endif //#if (SYSTEM_EXTENDER_MSC_SUPPORT)


/*----------------------------------------------------------------------------
 * void Extender_Transmitter_Port_Switch_Handle(U8_T newport)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *---------------------------------------------------------------------------- */
void Extender_Transmitter_Port_Switch_Handle(U8_T newport)
{
	KVM_NextHost = newport;

	if ((KVM_Control_Status & KVM_PORT_SW_MASK) == 0)
	{
		if (KVM_CurrentHost != KVM_NextHost ) // changed active port id
		{
			if (KVM_Power_Jump_Check(KVM_NextHost,0))
			{
				KVM_Control_Status |= KVM_PORT_SW_MASK;
#if (SYSTEM_PS2_HOST_ENABLE)
				// Prevent PS/2 stick after port switch */
				KVM_PS2_HotkeySwitch_Flag = 1; //it is hotkey switch
#endif

#if (PROJECT_USB_GENERIC_HID_ENABLE)
				DATAST_Generic_KB_IN_WP = DATAST_Generic_KB_IN_RP = 0;
#endif	/*#if (PROJECT_USB_GENERIC_HID_ENABLE) */
				/* Fork KVM_Swith_Task */
				TASK_KVM_Switch_Event_Control_Fork(KVM_NextHost);
				KVM_Audio_Combo_Switch_Check(KVM_NextHost);
			}
		}
	}
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_Receiver_Data_Send
 * Purpose: initial the extender receiver
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Receiver_Data_Send(U8_T *header,U8_T control,U8_T bRequest,U8_T bIndex,U8_T bValue,U8_T dataLen,U8_T data_buf)
{
	U16_T len;

	len = Transmitter_Set_Data_Transmit_Header(dataLen,header);
	if (len)
	{
		Extender_Packet_Data(Trnasmitter_Send_Data_TokenP, dataLen, &data_buf, bRequest, bIndex, bValue);
		Extender_Token_Transmit(Trnasmitter_Send_Buf,control, len, 1000, 0);
		//Disp_Str(Trnasmitter_Send_Buf,len);
	}
}

#ifdef PWAYTEK_01
/*----------------------------------------------------------------------------
 * void ExtenderT_Maintain_HostState_Led(U8_T index)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Maintain_HostState_Led(U8_T led_control)
{
	HOST_STATE_LED_GPIO = led_control;
}
#endif


#ifdef USB_2
/*----------------------------------------------------------------------------
 * U8_T void ExtenderT_Check_Devcie_MountedHost(U8_T devinx)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
U8_T ExtenderT_Check_Devcie_MountedHost(U8_T devinx)
{
	U8_T index;

	if (USB_PDevice[devinx].DevClass == USB_HID_CLASS)
	{
		for (index = 0; index < KVM_MAX_PORT ; index++)
		{
			if (USB_PDevice[devinx].MountPort & BIT_MASK[index])
			{
				return index;
			}
		}
	}

	return 0;
}
#endif

/*----------------------------------------------------------------------------
 * U8_T void ExtenderT_Buzzer_Control(U8_T devinx)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Buzzer_Control(U8_T value)
{
	if (value) // USB PEN Drive is Disattached
	{
		KVM_Flash.cSystemFlag0 |= SYSTEM_BEEPER_MASK;
#if (KVM_BUZZER_SUPPORT)
		BUZZER_Script_Active(Hotkey_Active_Sound);
#endif //#if (KVM_BUZZER_SUPPORT)
	}
	else
	{
		KVM_Flash.cSystemFlag0 &= ~SYSTEM_BEEPER_MASK;
	}

#if (SYSTEM_STORAGE_DEVICE_SUPPORT)
	STORAGE_Write(sizeof(KVM_Flash),(U8_T *)&KVM_Flash);
#endif //#if (SYSTEM_STORAGE_DEVICE_SUPPORT)

}


/*----------------------------------------------------------------------------
 * U8_T void ExtenderT_System_Control(U8_T devinx)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_System_Control(U8_T value)
{
	//Check Buzzer On/Off
	if (value & SYSTEM_TX_BEEPER_MASK)
	{
		KVM_Flash.cSystemFlag0 |= SYSTEM_BEEPER_MASK;
#if (KVM_BUZZER_SUPPORT)
		BUZZER_Script_Active(Hotkey_Active_Sound);
#endif //#if (KVM_BUZZER_SUPPORT)
	}
	else
	{
		KVM_Flash.cSystemFlag0 &= ~SYSTEM_BEEPER_MASK;
	}

	//Check USB Audio Combo Switch On/Off
	if (value & SYSTEM_TX_AUDIOCOMBO_MASK)
	{
		KVM_Flash.cSystemFlag1 |= SYSTEM_AUDIOCOMBO_MASK;
		KVM_Audio_Combo_Switch_Check(KVM_CurrentHost);
	}
	else
	{
		KVM_Flash.cSystemFlag1 &= ~SYSTEM_AUDIOCOMBO_MASK;
	}

#if (SYSTEM_STORAGE_DEVICE_SUPPORT)
	STORAGE_Write(sizeof(KVM_Flash),(U8_T *)&KVM_Flash);
#endif //#if (SYSTEM_STORAGE_DEVICE_SUPPORT)
	
	Transmitter_System_Set_Flag = 1;
#ifdef DEVICE_KEEP		
	ExtenderT_Control_Flag |= ExtenderT_Start_Mask;
#endif	
}

/*----------------------------------------------------------------------------
 * U8_T void ExtenderT_Audio_SW_Handle(U8_T devinx)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
void ExtenderT_Audio_SW_Handle(U8_T newport)
{
	if (newport != KVM_CurrentUSBAudio)
	{
		if ((KVM_Flash.cSystemFlag1 & SYSTEM_AUDIOCOMBO_MASK) == 0)
		{
			KVM_NextAudio = newport;
			USBHC_AUDIO_Switch(KVM_NextAudio);
		}
	}
}

/*----------------------------------------------------------------------------
 * U8_T void ExtenderT_Audio_AutoSW_Handle(U8_T devinx)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Audio_AutoSW_Handle(U8_T value)
{
	if (value) // audio auto switch ON
	{
		KVM_Flash.cSystemFlag1 |= SYSTEM_AUDIOCOMBO_MASK;
	}
	else
	{
		KVM_Flash.cSystemFlag1 &= ~SYSTEM_AUDIOCOMBO_MASK;
	}

	//After the combo switch mode is on,check the current audio portid
	if (KVM_Flash.cSystemFlag1 & SYSTEM_AUDIOCOMBO_MASK)
	{
		KVM_Audio_Combo_Switch_Check(KVM_CurrentHost);
	}
}
#endif /* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */
/*----------------------------------------------------------------------------
 * U8_T void ExtenderT_Msc_SW_Handle(U8_T devinx)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Msc_SW_Handle(U8_T newport)
{
	
#if (SYSTEM_MSC_DEVICE_SUPPORT)
	if (KVM_CurrentMSC != newport) // changed active port id
	{
		if (USBHC_MSC_Devinx != 0xff)
		{
			//printf("Switch MSC to port:%bu\n\r",newport);
			USBHC_MSC_Port_Switch(USBHC_MSC_Devinx, newport);
		}
	}
#else
	newport = 0;
#endif
}

/*----------------------------------------------------------------------------
 * U8_T void ExtenderT_Receiver_Send_Cmd
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Receiver_Send_Cmd(U8_T *header,U8_T control,U8_T bRequest,U8_T bIndex,U8_T bValue,U16_T dataLen,U8_T *data_buf)
{
	U16_T len;

	len = Transmitter_Set_Data_Transmit_Header(dataLen,header);
	if (len)
	{
		Extender_Packet_Data(Trnasmitter_Send_Data_TokenP, dataLen,data_buf,bRequest,bIndex,bValue);
		Extender_Token_Transmit(Trnasmitter_Send_Buf, control, len, 1000, 0);		
	}
	/*
	len = Transmitter_Set_Data_Transmit_Header(0,NULL);
	if (len)
	{
		Extender_Packet_Data(Trnasmitter_Send_Data_TokenP,0,NULL,bRequest,bIndex,bValue);

		printf("[MSC_Reset:%02bx]\n\r",bRequest);

		Extender_Token_Transmit(Trnasmitter_Send_Buf,EXTENDER_CONTROL_TRANSMIT_NO_WAIT,len,1000,0);
		//Transmitter_Control_State |= TRANSMITTER_RESTART_FLAG;
	}
	*/
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_USB_Set_Protocol(U8_T devinx,U8_T curinf,U8_T protocol_value)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Send_MSC_Reset(U8_T devinx)
{
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bCmd = T_USB_CMD_SET_PROTOCOL_FLAG;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bIndex = Remote_DevMap[devinx].Mapping & ~USBHC_DEVICE_USED_MASK;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bValue = 0;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bLen   = 0;
	ExtenderT_Cmd_Q_Inx++;
	Transmitter_Send_State |= TRANSMITTER_SEND_WAIT;
}

/*----------------------------------------------------------------------------
 * void ExtenderT_Reset_Remote_MSC(void)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
#if (SYSTEM_MSC_DEVICE_SUPPORT)
void ExtenderT_Reset_Remote_MSC(void)
{
	U8_T devinx;

	devinx = Remote_DevMap[USBHC_MSC_Devinx].Mapping & ~USBHC_DEVICE_USED_MASK;
	ExtenderT_Receiver_Send_Cmd(NULL,EXTENDER_CONTROL_TRANSMIT_NO_WAIT,EXTENDER_USB_MSC_RESET,devinx,NO_VALUE,NO_LEN,NO_DATA);
}
#endif

/*----------------------------------------------------------------------------
 * void ExtenderT_MSC_In_Wait(U8_T bindex,U8_T bvalue)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
#if (SYSTEM_MSC_DEVICE_SUPPORT)
void ExtenderT_MSC_In_Wait(U8_T bindex,U8_T bvalue)
{
	ExtenderT_Receiver_Send_Cmd(NULL,EXTENDER_CONTROL_TRANSMIT_NO_WAIT,EXTENDER_USB_MSC_IN_WAIT,bindex,bvalue,NO_LEN,NO_DATA);
}
#endif

/*----------------------------------------------------------------------------
 * void ExtenderT_Reset_Remote_MSC_Req(void)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
#if (SYSTEM_MSC_DEVICE_SUPPORT)
void ExtenderT_Reset_Remote_MSC_Req(void)
{
	U8_T devinx;

	devinx = Remote_DevMap[USBHC_MSC_Devinx].Mapping & ~USBHC_DEVICE_USED_MASK;
	ExtenderT_Send_Cmd_To_Receiver(T_MSC_RESET,EXTENDER_USB_MSC_RESET,devinx,0);
}
#endif

/*------------------------------------------------------------------------------
 * void ExtenderT_USB_Send_Command(U8_T devinx,U8_T curinf,U8_T protocol_value)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void ExtenderT_Send_Cmd_To_Receiver(U8_T cmd,U8_T request,U8_T index,U8_T value)
{
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bCmd 	  = cmd;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bRequest = request;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bIndex   = index;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bValue	  = value;
	ExtenderT_Cmd_Q[ExtenderT_Cmd_Q_Inx].bLen     = 0;

	ExtenderT_Cmd_Q_Inx++;
	Transmitter_Send_State |= TRANSMITTER_SEND_WAIT;
}

/*------------------------------------------------------------------------------
 * void ExtenderT_PS2_Data_Handle(U8_T intf)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
#if (PROJECT_USB_GENERIC_HID_ENABLE)
void ExtenderT_PS2_Data_Handle(void)
{	
	U16_T len,start_index;
	U8_T  index;
	U8_T  intf;

	intf = Extender_Setup.Data_Token.bValue;
	len = Extender_Setup.Data_Token.iLen;
	start_index = Extender_Setup.iRecBuf_Index;
	if (intf == EXTENDER_PS2_KEYBOARD)
	{
		for (index=0; index < len; index++)
		{
			PS2_Process_Keyboard_Package(PS2_PORT_MAX_NUM+EXTENDER_PS2_KEYBOARD,rs485_RcvrBufRing[start_index]);
			start_index++;
			start_index &= (UR2_RX_BUF_SIZE -1);
		}
	}
	else
	{
		for (index=0; index < len; index++)
		{
			PS2_Mouse_Package[PS2_PORT_MAX_NUM+EXTENDER_PS2_MOUSE][index] = rs485_RcvrBufRing[start_index];
			start_index++;
			start_index &= (UR2_RX_BUF_SIZE -1);
		}
		PS2_Mouse_Package_Index[PS2_PORT_MAX_NUM+EXTENDER_PS2_MOUSE] = 0; // clear index first, for next package
		DATAST_Append_Generic_MS_IN_Q(GENERIC_USAGE_07_MAKE,PS2_Mouse_Package[PS2_PORT_MAX_NUM+EXTENDER_PS2_MOUSE]); // normal mouse package
		DATAST_PS2_MS_Handle();
	}
}
#endif

/*----------------------------------------------------------------------------
 * void ExtenderT_Remote_Interval_Req(void)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Remote_Interval_Req(U8_T devinx,U8_T enpdinx,U8_T interval)
{
	U8_T remote_devinx;
	remote_devinx = Remote_DevMap[devinx].Mapping & ~USBHC_DEVICE_USED_MASK;
	remote_devinx |= (enpdinx << 4);
	//printf("Set Interval:%bu,Enpdinx=%bu,Interval=%bu\n\r",devinx,enpdinx,interval);
	ExtenderT_Send_Cmd_To_Receiver(T_USB_INTERVAL,EXTENDER_USB_INTERVAL,remote_devinx,interval);
}

/*----------------------------------------------------------------------------
 * void ExtenderT_Remote_Interval(U8_T devinx,U8_T value)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Remote_Interval(U8_T devinx,U8_T value)
{	
	U8_T remote_devinx;

	remote_devinx  = Remote_DevMap[devinx&0x0f].Mapping & ~USBHC_DEVICE_USED_MASK;
	remote_devinx |= (devinx & 0xf0); //this is endpinx
	//printf("TR devinx:%bx,value=%bu\n\r",remote_devinx,value);
	ExtenderT_Receiver_Send_Cmd(NULL,EXTENDER_CONTROL_TRANSMIT_NO_WAIT,EXTENDER_USB_INTERVAL,remote_devinx,value,NO_LEN,NO_DATA);
}

#ifdef PLUG_DETECT
/*----------------------------------------------------------------------------
 * Function Name: TASK_Transmitter_PlugDetect(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void TASK_Transmitter_PlugDetect(void)
{	
#ifdef RJ45_PLUG_DETECT		
	U8_T  RS45_plug;
#endif /* #ifdef RJ45_PLUG_DETECT	 */		
	U8_T  HDMI_plug;

#ifdef RJ45_PLUG_DETECT	
	//1.Check the RJ45 Plug		
	RS45_plug = RJ45_PLUG;	
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
				ExtenderT_Check_Uart_Disable();
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
#endif /* #ifdef RJ45_PLUG_DETECT	 */	
	//2.Check the HDMI HOTPLUG
	HDMI_plug = HDIM_HOTPLUG;
	//printf("%bu",HDMI_plug);
	if (HDMI_plug == HDMI_PLUG_OFF)	
	{
		if (HDMI_PLUG_SATE == PLUG_IN)
		{	
			printf("HDMI Plug off,quiet start\n\r");
			HDMI_PLUG_SATE = PLUG_OFF;
			UART_Disable_Flag |= UART_HDMI_DISABLE_MASK;
			ExtenderT_Check_Uart_Disable();
		}	
	}
	else
	{
		if (HDMI_PLUG_SATE == PLUG_OFF)
		{
			printf("HDMI Plug In,quiet start\n\r");
			HDMI_PLUG_SATE = PLUG_IN;			
			UART_Disable_Flag |= UART_HDMI_DISABLE_MASK;
			ExtenderT_Check_Uart_Disable();
		}			
	}		
}

/*----------------------------------------------------------------------------
 * Function Name: ExtenderT_Check_Uart_Disable(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void ExtenderT_Check_Uart_Disable(void)
{
	if (TASK_ExtenderT_Uart_Disable_ActiveID ==0)
	{	
		TASK_ExtenderT_Uart_Disable_ActiveID = TASK_Run(&TASK_ExtenderT_UartDisable_TASK);
	}
	else
	{
		Task_Active_Table[TASK_ExtenderT_Uart_Disable_ActiveID].Task_Interval.w = UART_DISABLE_TIME;
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
void TASK_ExtenderT_Uart_Disable(void)
{
	printf("Quiet End\n\r");	
	UART_Disable_Flag = 0;
	TASK_Destory_Current();
	TASK_ExtenderT_Uart_Disable_ActiveID = 0;
}	
#endif /* #ifdef PLUG_DETECT */

#ifdef DEVICE_KEEP
/*----------------------------------------------------------------------------
 * void Transmitter_Clear_Old_DeviceKeep(void)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void Transmitter_Clear_Old_DeviceKeep(void)
{
	U8_T  index;
	U8_T  remote_devinx;
	
	for (index = 1; index < USB_HC_MAX_DEVICE; index++)
	{
		if ((USB_PDevice[index].Addr & USBHC_DEVICE_USED_MASK) && (USB_PDevice[index].DevAttr & DEVATTR_REMOTE_DEV_MASK))
		{		
			//printf("devinx=%bu,devicekeep=%02bx\n\r",index,USB_PDevice[index].DeviceKeep);
			if ((USB_PDevice[index].DeviceKeep & DEVICE_KEEP_DONE_MASK) == 0x00)				
			{
				printf("Device Keep:Clean Devinx:%bx\n\r",index);
				//Should remove this device
				remote_devinx = Remote_DevMap[index].Mapping & ~USBHC_DEVICE_USED_MASK;
				Remote_DevMap[index].Mapping = 0;
				USBHC_CORE_Clear_Device(index);
				Remote_Reserve_DevMap[remote_devinx] = 0; //clear reverse mapping
			}		
			else
			{
				USB_PDevice[index].DeviceKeep &= ~DEVICE_KEEP_DONE_MASK; //clear the flag
			}				
		}
	}			
}	
/*----------------------------------------------------------------------------
 * void Transmitter_Clear_DeviceKeep_Flag(void)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void Transmitter_Clear_DeviceKeep_Flag(void)
{		
	U8_T  index;
	

	for (index = 1; index < USB_HC_MAX_DEVICE; index++)
	{
		if (USB_PDevice[index].Addr & USBHC_DEVICE_USED_MASK)
		{			
			USB_PDevice[index].DeviceKeep &= ~DEVICE_KEEP_DONE_MASK;				
		}
	}			
}	

/*----------------------------------------------------------------------------
 * U8_T Transmitter_Check_Remote_NewDevice(Extender_Cmd_Handle_Def *setup)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void Transmitter_Check_Remote_NewDevice(Extender_Cmd_Handle_Def *setup)
{	
	U16_T ivp_id[2];
	U8_T  devinx,remote_devinx;
		
	if (setup->Data_Token.bValue == VDEV_MOUNT)
	{	
		EXTENDER_CopyRxDmaToApp(setup->iRecBuf_Index,4,(U8_T *) &ivp_id);
		printf("RemoteDevinx=%bu,Vid=%04x,Pid=%04x\n\r",setup->Data_Token.bIndex,ivp_id[0],ivp_id[1]);
		remote_devinx		= setup->Data_Token.bIndex;		
		devinx = Remote_Reserve_DevMap[remote_devinx];
		printf("devinx=%bu\n\r",devinx);
		etdr_T_MountSkipFlag = 0;
		if (devinx != 0)
		{	
			if (USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK)
			{
				if (memcmp((U8_T *)&USB_PDevice[devinx].idVendor,(U8_T *)&ivp_id,4) == 0x00)
				{			
					USB_PDevice[devinx].DeviceKeep |= DEVICE_KEEP_DONE_MASK;	
					etdr_T_MountSkipFlag = 1;
					return;
				}					
			}
		}			
	}	
}	

/*----------------------------------------------------------------------------
 * U8_T Transmitter_Check_Remote_NewDevice(Extender_Cmd_Handle_Def *setup)
 * Purpose:
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void Transmitter_Check_Device_Table(Extender_Cmd_Handle_Def *setup)
{	
	//U16_T ivp_id[2];
	U8_T  devinx,remote_devinx,index;
	
	
	if (setup->Data_Token.iLen)
	{		
		EXTENDER_CopyRxDmaToApp(setup->iRecBuf_Index,setup->Data_Token.iLen,(U8_T *)DeviceKeepTable);
		ExtenderT_Device_Mount_Change = 0xfe; //set all device need to report
		
		for (index=0; index < (USB_HC_MAX_DEVICE-1); index++)
		{
			if (DeviceKeepTable[index].ivendor) //remote devinx exist.
 			{
				printf("remote:%bu,vid=%04x,pid=%04x,",index+1,DeviceKeepTable[index].ivendor,DeviceKeepTable[index].iproduct);
				remote_devinx = index+1;			
				devinx = Remote_Reserve_DevMap[remote_devinx];
				//USB_PDevice[devinx].DeviceKeep &= ~DEVICE_KEEP_DONE_MASK; //default need to remove the device
				if (devinx != 0) // if device is in transmitter already, than compare vid pid
				{					
					USB_PDevice[devinx].DeviceKeep &= ~DEVICE_KEEP_DONE_MASK; //default remove device
					if (memcmp((U8_T *)&USB_PDevice[devinx].idVendor,(U8_T *)&DeviceKeepTable[index].ivendor,4) == 0x00) //if same device
					{
						printf("skip\
						n\r");
						ExtenderT_Device_Mount_Change &= ~BIT_MASK[index+1]; //remote device no need report again
						USB_PDevice[devinx].DeviceKeep |= DEVICE_KEEP_DONE_MASK; // not remove device						
					}						
					else
					{
						//check if it is remote device			
						printf("new\n\r");
					}						
				}
			}
			else
			{ //since remote not exist, then no report
				printf("Devinx:%bu,no report[%04x]\n\r",index+1,USB_PDevice[index+1].idVendor);
				ExtenderT_Device_Mount_Change &= ~BIT_MASK[index+1];			
			}				
		}
		//report device map back to receiver
		printf("MountChange=%02bx\n\r",ExtenderT_Device_Mount_Change);
		//ExtenderT_Send_Cmd_To_Receiver(T_USB_INTERVAL,EXTENDER_USB_INTERVAL,remote_devinx,interval);
		//ExtenderT_Receiver_Send_Cmd(NULL,EXTENDER_CONTROL_TRANSMIT_NO_WAIT,EXTENDER_T_DEVICE_TABLE,ExtenderT_Device_Mount_Change,NO_VALUE,NO_LEN,NO_DATA);
		ExtenderT_Send_Cmd_To_Receiver(T_DEVICE_TABLE,EXTENDER_T_DEVICE_TABLE,ExtenderT_Device_Mount_Change,NO_VALUE);
		//clear the device which not same with receiver
		Transmitter_Clear_Old_DeviceKeep();	
	}
}	
#endif /* #ifdef DEVICE_KEEP */		
#endif /* SYSTEM_EXTENDER_TRANSMITTER */
/* End of extender_transmitter.c */
