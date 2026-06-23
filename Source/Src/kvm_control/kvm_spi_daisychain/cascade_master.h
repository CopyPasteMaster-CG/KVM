 /*
 ******************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*=============================================================================
 * Module Name: cascade_master.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __CASCADE_MASTER_H__
#define __CASCADE_MASTER_H__

/* INCLUDE FILE DECLARATIONS 		  */
/* DEFINATION DECLARATIONS   		  */
#define	CCMST_DEBUG_MODE		0
#define	CCMST_ORIGINAL_1		0

//#define   REMOTE_HOST_PORT			  	0x00
#define   CASCADEM_BROADCAST			0xff
#define   CASCADEM_IN_READY_MASK		0x54
//--------------------------------------------
//$ Define input ready pin
//--------------------------------------------
#define  SLAVE_INPUT_READY				0
//--------------------------------------------
//$ RL_Link_LED_State
//--------------------------------------------
#define   REMOTE_SELECT_LED			  	0x00
#define   LOCAL_SELECT_LED			  	0x01

//--------------------------------------------
//$CascadeM_Control_State Define
//
#define   SLAVE_CONNECTION_MASK  			0x01
//#define   SLAVE_EDID_MASK  					0x02
#define   MASTER_RESTART_MASK	  			0x04
#define   SLAVE_RESTART_MASK 				0x08

#define   SLAVE_INREADY_PROCESSING_MASK 	0x10
#define   MASTER_SEND_WAIT					0x20
#define   MASTER_SEND_START					0x40

//--------------------------------------------
//$Master_DMA_State Define
//
#define   SLAVE0_DMA_MASK  					0x02
#define   SLAVE1_DMA_MASK  					0x04
#define   SLAVE2_DMA_MASK  					0x08

//-----------------------------------------------
//$ CascadeM_Passthrough_State
//-----------------------------------------------
#define   CASCADEM_PASSTHROUGH_IDLE				0x00
#define   CASCADEM_PASSTHROUGH_SETUP_MASK		0x01
#define   CASCADEM_PASSTHROUGH_IN_MASK			0x02
#define   CASCADEM_PASSTHROUGH_IN_SEND_MASK		0x04
#define   CASCADEM_PASSTHROUGH_OUT_MASK			0x08
#define   CASCADEM_PASSTHROUGH_IN_PAGE_MASK		0x10
#define   CASCADEM_PASSTHROUGH_STATUS_IN_MASK	0x20
#define   CASCADEM_PASSTHROUGH_STATUS_OUT_MASK	0x40
#define   CASCADEM_PASSTHROUGH_SETUP_DONE_MASK	0x80

/*
$CascadeM_Console_State
*/
#define   CASCADEM_KVM_START					0x01
#define   CASCADEM_EDID_VALID					0x02
#define   CASCADEM_EDID_REPORT					0x04

/*
$CascadeM_Slave_Connect_State, Slave_Connect_State
*/
#define   CASCADEM_SLAVE0_CONNECT				0x01
#define   CASCADEM_SLAVE1_CONNECT				0x02
#define   CASCADEM_SLAVE2_CONNECT				0x04
#define   CASCADEM_SLAVE3_CONNECT				0x08
#define   CASCADEM_SLAVE4_CONNECT				0x10
#define   CASCADEM_SLAVE5_CONNECT				0x20
#define   CASCADEM_SLAVE6_CONNECT				0x40
#define   CASCADEM_SLAVE7_CONNECT				0x80




//-----------------------------------------------
//$ Sate Define
//-----------------------------------------------
#define	STATE_CONNECT
#define	STATE_SUSPEND
#define	STATE_REMOTE_WAKEUP

typedef struct _Cascade_Ctrl_Def
{
	U8_T  	Connect[2];					// Support 16 Ports	
	U8_T  	Suspend[2];					// Support 16 Ports	
	U8_T  	Resume[2];					// Support 16 Ports	
	U8_T    DeviceAttr[16][8];          // Support 16 ports x 8 devices
} Cascade_Ctrl_Def;


#define CASCADEM_MOUNT_QUEUE_MAX    16

typedef struct _Cascade_Mount_Queue
{
	U8_T  	Devinx;						// Devinx index
	U8_T  	MountType;					// Bit
	U16_T  	MountPort;					// Support 16 Ports		
} Cascade_Mount_Que_Def;


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void CascadeM_Master_Init(void);
void CascadeM_Receive_Complete_Handle(void);
U8_T Cascade_Receive_Cmd_Token_Check(U16_T len,Cascade_Cmd_Token_Def *cmd_token,U8_T update,U8_T addr);
U8_T CascadeM_Resonsed_Received_Handle(void);
void Cascade_Master_Transmit_TimeOut_Check(U8_T token);
U8_T Cascade_Master_DeviceMount_Control(U8_T devinx,U8_T mounttype,U8_T brequst);
void Cascade_Master_EDID_Control(void);
void Cascade_Master_EDID_PlugOff(void);
void CascadeM_Remote_Link_Led_Flash(U8_T); 
U8_T CascadeM_Setup_Buffer_Malloc(U8_T devinx);
U8_T CascadeM_Check_Transmitter_Plug_State(void);
U8_T CascadeM_Malloc_Intr_Buffer(U8_T td_id,U8_T endpsize);
void CascadeM_Transmit_Intr_Endp_In_Data(U8_T devinx,U8_T td_id,U8_T data_len,U8_T API_Only);
void CascadeM_Interrupt_Transfer_In_Data_Handle(U8_T devinx,U8_T intt_id);
void CascadeM_Transmit_FIFO_Send(void);
void CascadeM_Transfer_Complete_Handle(void);
void CascadeM_Slave_InRquest_Handle(U8_T pinstate);
void CascadeM_Transmit_Update(void);
void CascadeM_Receive_Complete_Handle(void);
void CascadeM_Cmd_Transmit(U8_T *buf,U8_T contrl_attr,U16_T len,U16_T timeout,U8_T target);
void CascadeM_Slave_Input_Ready_Init(void);
void CascadeM_KVM_Console_Init(void);
void CascadeM_KVM_Btn_Handle(U8_T hostid);
U8_T CascadeM_PassThrough_Setup_Buffer_Malloc(U8_T pid,U8_T devinx);
void CascadeM_PassThrough_Setup_Data_Out_Enable(U8_T slave,U8_T port,U8_T devinx);
void TASK_CascadeM_Device_Mount_Control(void);
#if CCMST_ORIGINAL_1
U8_T CascadeM_Check_Valid_Slave(U16_T port_type);
#else
U8_T CascadeM_GetSlaveIdFromPortMap(U16_T port_map);
#endif
U8_T CascadeM_Check_System_Ready(void);
U8_T CascadeM_Check_Mount_Valid_State(U8_T slave,U8_T devinx,U8_T mounttype,U16_T *mountport);
void CascadeM_Maintain_Mount_State(U8_T slave,U8_T devinx,U8_T mounttype,U16_T mountport);
void CascadeM_USB_Device_Check_Interval(U8_T slave,U8_T port,U8_T devinx);
void CascadeM_PassThrough_Setup_Data_In(U8_T slave,U8_T port,U8_T devinx);
void CascadeM_PassThrough_Setup_Condition_Reset(U8_T devinx,U8_T port);
void CascadeM_PassThrough_Setup_End(U8_T slave,U8_T port,U8_T devinx);
void CascadeM_PassThrough_Setup_Stall(U8_T slave,U8_T port,U8_T devinx);
void CascadeM_Device_Mount_Control(U8_T devinx,U8_T mount_type,U16_T mount_port);
void CascadeM_Receive_System_Command_Handle(CASCADE_PacketHeader *dp);
void CascadeM_Receive_KVM_Command_Handle(CASCADE_PacketHeader *dp);
void CascadeM_Receive_USB_Command_Handle(CASCADE_PacketHeader *dp);
void CascadeM_Console_EDID_PlugOff(void);
void CascadeM_Console_Monitor_PlugIn(void);
void CascadeM_EDID_Control(void);
void CascadeM_USB_Device_Mount_Transmit(U8_T slave,U8_T devinx,U8_T mounttype,U16_T mountport);
void CascadeM_USB_Device_Descriptor_Transmit(U8_T slave,U8_T devinx,U16_T mountport,U8_T target);
void CascadeM_Check_Next_Mount_Event(void);
void CascadeM_Transmit_DataPack(U8_T slave,U8_T control,U8_T command,U8_T request,U8_T index,U8_T value,U16_T data_len,U8_T *datap); 
//void CascadeM_CopyRxDmaToApp(U16_T start,U16_T appGetLen,U8_T *pAppData);
void CascadeM_Passthrough_Setup_Data_Out_Handle(CASCADE_PacketHeader *token);
void CascadeM_Setup_Data_Out_Enable(U8_T port,U8_T devinx);
void CascadeM_Setup_Data_In(U8_T slave,U8_T port,U8_T devinx);
void CascadeM_Passthrough_Setup_Status_Out_Handle(CASCADE_PacketHeader *token);
void CascadeM_Passthrough_Intr_Out_Handle(CASCADE_PacketHeader *token);
void CascadeM_Rx_Device_Informtaion(CASCADE_PacketHeader *token);
void CascadeM_Passthrough_Setup_Data_In_Enable(CASCADE_PacketHeader *token);
void CascadeM_SPI_SetOpCodeReg(U8_T *opCode, U8_T opLen);
void CascadeM_Handle_After_Transmit_Complete(void);
void CascadeM_KVM_KB_Led_Handle(U8_T port,U8_T kb_led);
void CascadeM_Audio_Device_State_Reset(void);
void CascadeM_Slave_DeviceMount_Control(U8_T upid,U8_T devinx,U8_T bcmd);
void CascadeM_Passthrough_Audio_Data_Out_Handle(CASCADE_PacketHeader *token);
void CascadeM_Audio_Data_Out_Buf_Check(void);
void CascadeM_GHid_Device_Buf_Init(void);
void CascadeM_GHID_Intr_Data_Transmit(U8_T endpid,U8_T length,U8_T *buf);
void CascadeM_Setup_Command_Handle(U8_T devinx,USB_Setup_TypeDef *setup);
void TASK_CascadeM_USBHC_PassThrough(void);
U8_T CascadeM_UpPort_State(U8_T hostid);
void CascadeM_Audio_ISR_Handle(void);
void CascadeM_Passthrough_HidInterval_Handle(CASCADE_PacketHeader *token);
RESULT CascadeM_Passthrough_Protocol_Setting_Handle(U8_T hcperform,U8_T slave,U8_T port_devinx,U8_T intf,U8_T value);
void CascadeM_Passthrough_Alternate_Setting_Handle(CASCADE_PacketHeader *token);
void CascadeM_Passthrough_Configuration_Handle(CASCADE_PacketHeader *token);
void CascadeM_USB_Device_Switch(U8_T slave,U8_T target_port,U8_T devinx);
void CascadeM_USB_Maintain_Connect_State(U8_T hostid,U8_T value);
void CascadeM_USB_Maintain_Power_State(U8_T hostid,U8_T value);
void CascadeM_USB_Maintain_Suspend_State(U8_T hostid,U8_T value);
void CascadeM_USB_Maintain_KVM_State(U8_T hostid,U8_T value);
void DATAST_API_Transmit_Hid_Data(U8_T devinx,U8_T intt_id,U8_T *buf,U8_T length);
void CascadeM_Transmit_API_Mount_State(U8_T devinx,U8_T mounttype);
void CascadeM_Box_Redirect_Handle(CASCADE_PacketHeader *dp);
void TASK_CASCADEM_Transmit_TimeOut(void);
U16_T CascadeM_Retrive_MountPort_Value(U8_T devinx);
void CascadeM_Slave_Chip_Reset(void);
void CascadeM_USB_Maintain_KVM_Connect_State(U8_T hostid,U8_T value);
/* EXPORTED GLOBAL VARIABLES */
extern U8_T     CascadeM_Console_State;
extern idata volatile  U8_T		CascadeM_Transmit_OutP;
extern idata volatile  U8_T		CascadeM_Transmit_InP;
extern U8_T 	CascadeM_Control_State;
extern U8_T		TASK_CascadeM_USBHC_PassThrough_ID;
extern bit  	Remote_Link_LED_Flash_Flag;
extern U8_T		CascadeM_Resend_Cnt;
extern Cascade_System_State     CascadeM_Sys_State;
extern const GPIO_TypeDef  	 *SPI_InReadyPort[];
extern const GPIO_PinTypeDef SPI_InReadyPin[];
extern TASK_ActiveTable_TypeDef TASK_CascadeM_Slave_InRquest_Handle_TASK;
extern U8_T 	CascadeM_Mount_Startp;
extern U8_T 	CascadeM_Mount_Endp;
extern Cascade_Mount_Que_Def	CasecadeM_Mount[CASCADEM_MOUNT_QUEUE_MAX];
extern bit 		task_debug_flag;
extern bit		CascadeM_Transmit_Processing_Flag;
//extern U8_T 	CascadeM_Slave_Connect_State;
extern U8_T 	CascadeM_Passthrough_State[];
extern U8_T     CascadeM_Salve_Cnt;
extern U8_T     CascadeM_KVM_Max_port;
extern Cascade_Data_Packet_Def   CascadeM_SPI_RBRS_Buf[MAXIMUM_SLAVE];
extern U8_T 	Master_Cmd_RBRS[];
extern U8_T 	Master_Cmd_DRMEM[];
extern U8_T 	Master_Cmd_DWMEM[];
extern U8_T		TASK_CascadeM_DeviceMount_Control_ID;
//extern U16_T	CascadeM_Rx_Data_Inx;
extern U8_T		CascadeM_Current_Slave;
extern U8_T  	CascadeM_Rx_Buf[];
extern Firmware_Version_Def  FirmwareVersion[4];

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
#if (SYSTEM_CASCADE_AUDIO_BUF_CNT)
extern Cascade_ISO_Data_Def CADR_USB_Audio_Buf[SYSTEM_CASCADE_AUDIO_BUF_CNT];
extern volatile U8_T	CADR_USB_HC_Audio_Inx;
extern volatile U8_T	CCMST_AudioOutRdPtr;
extern volatile U8_T	CCMST_AudioOutWrPtr;
extern bit  CCMST_AudioOutBusy;
extern volatile U8_T	CADR_USB_CascadeM_Audio_Inx;
#endif /* #if (SYSTEM_CASCADE_AUDIO_BUF_CNT) */
#endif /* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */
#endif /* End of __CASCADE_MASTER_H__ */

/*
$ End of file cascade_master.h
*/