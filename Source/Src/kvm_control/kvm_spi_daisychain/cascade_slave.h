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
 * Module Name: cascade_slave.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __CASCADE_SLAVE_H__
#define __CASCADE_SLAVE_H__

/* INCLUDE FILE DECLARATIONS 		  */
/* DEFINATION DECLARATIONS   		  */

	/* Configuration */
#define   CCSLV_DEBUG_MODE		0
//--------------------------------------------
//$CascadeS_Control_State Define
//
#define   CASCADES_AUDIO_TRNASMIT_MASK 	0x01


/*
$ Slave_Control_State
*/
#define   CASCADES_PAGE_LENGTH			1024
#define   CASCADE_SLAVE_INPUT_GPIO		P3_7
#define   CASCADES_DMA_LENGTH			24    		//Support the maximun

#define   CASCADES_RX_BUF_MAX           0x07FF
#define   CASCADES_TX_BUF_MAX           0x01FF

#define   SLAVE_RESTART_FLAG   			0x01
#define   SLAVE_USB_PASSTHROUGH_FLAG 	0x02
#define   SLAVE_USB_COMMAND_FLAG	 	0x04
#define   SLAVE_ANSWER_WAIT_FLAG	 	0x08
#define   SLAVE_CONNECT_FLAG 			0x10
#define   SLAVE_KB_LED_FLAG   			0x20

/*
$ CascadeS_Device_Passthough_State
*/
#define   T_USB_PASSTHROUGH_SETUP_FLAG  		0x01
#define   T_USB_PASSTHROUGH_DATA_IN_FLAG 		0x02
#define   T_USB_PASSTHROUGH_DATA_OUT_FLAG 		0x04
#define   T_USB_PASSTHROUGH_DATA_IN_ENABLE_FLAG 0x08
#define   T_USB_PASSTHROUGH_STATUS_IN_FLAG 		0x10
#define   T_USB_PASSTHROUGH_STATUS_OUT_FLAG 	0x20
#define   T_USB_PASSTHROUGH_DC_DONE_FLAG 	 	0x40
#define   T_USB_PASSTHROUGH_REMOTE_DONE_FLAG  	0x80


/*
$ CascadeS_Device_USB_Cmd_Send_State
*/
#define   T_USB_CMD_SET_PROTOCOL_FLAG 	 		0x01



#define   MASTER_PLUG_OFF_CNT					   5
/*-------------------------------------------------------
$ Slave_Wait_Answer_State
*/
#define	 SLAVE_WAIT_SOF_SEND_ANSWER		0x01


/*-------------------------------------------------------
$ Slave_Send_State Define
*/
#define  SLAVE_SEND_WAIT					0x01
#define  SLAVE_SEND_START					0x02
#define  SLAVE_SEND_KB_LED					0x04
#define  SLAVE_SEND_RESTART					0x08
#define  SLAVE_SEND_USB_EVENT				0x10
#define  SLAVE_SEND_COTROL_EVENT			0x20


/* SPI Slave speed setting */
#define SPIS_24M		0x04
#define SPIS_16M		0x06
#define SPIS_12M		0x08
#define SPIS_8M			0x0c
#define SPIS_6M			0x10
#define SPIS_4M			0x18
#define SPIS_2M			0x30
#define SPIS_1M			0x60

typedef struct _CCSLV_CTRL_
{
	U8_T TxTriggerCnt;
} CCSLV_CTRL;

typedef struct _CascadeS_System_State
{
	U8_T  Connect_State;
	U8_T  Suspend_State;
	U8_T  Resume_State;
	U8_T  Power_State;	
} CascadeS_System_State;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void 	CascadeS_Receive_Handle(void);
void 	CascadeS_Slave_Init(void);
U8_T 	CascadeS_Setup_Buffer_Malloc(U8_T pid,U8_T devinx);
void 	CascadeS_Intr_In_Move_Wait_Handle(U8_T port,U8_T devinx,U8_T intt_id,U8_T endpidx);
void	CascadeS_KVM_KB_Led_Check(U8_T port);
void	CascadeS_USB_Set_Protocol(U8_T devinx,U8_T curinf,U8_T protocol_value);
//void   	CascadeS_Rx_Buffer_Data_Check(void);
void   	CascadeS_Rx_Buffer_ReadP_Update(U16_T endp);
void   	CascadeS_Transfer_Complete_Handle(U8_T spis_state);
void   	CascadeS_Transmit_FIFO_Send(void);
//void 	CascadeS_Tx_Buffer_Wp_Update(U16_T endp);
//void 	CascadeS_CopyDataToTxBuffer(U16_T start,U16_T len,U8_T *buf);
void 	CascadeS_CopyBufToTxDMA(U16_T start,U16_T len,U8_T *buf);
//void 	CascadeS_DMAToTxBuf(U16_T start,U16_T appGetLen,U8_T *pAppData);
void 	CascadeS_Send_Transmit(U8_T *buf,U8_T contrl_attr,U16_T len);
void	CascadeS_Transmit_KVM_State(U8_T connect_status, U8_T power_status, U8_T suspend_status);
void 	CascadeS_Transmit_KVM_KB_Led(U8_T port,U8_T led);
void	CascadeS_KVM_ReportButtonStatus(U8_T upstream_port, U8_T button_status);
void 	CascadeS_Intr_In_Move_Wait_Handle(U8_T port,U8_T devinx,U8_T intt_id,U8_T endpidx);
void	CascadeS_Receive_USB_Handle(CASCADE_PacketHeader *packet);
void	CascadeS_Receive_System_Handle(CASCADE_PacketHeader *packet);
void	CascadeS_Receive_KVM_Handle(CASCADE_PacketHeader *packet);
void 	CascadeS_System_EDID_Handle(CASCADE_PacketHeader *packet);
//void 	CascadeS_CopyRxDmaToApp(U16_T start,U16_T appGetLen,U8_T *pAppData);
void 	CascadeS_USB_Device_Mount_Handle(CASCADE_PacketHeader *packet);
void 	CascadeS_USB_Device_Unmount_Handle(CASCADE_PacketHeader *Data_Token);
void 	CascadeS_Device_Descriptor_Handle(CASCADE_PacketHeader *Data_Token);
void 	CascadeS_Device_Descriptor_End_Handle(CASCADE_PacketHeader *Data_Token);
void 	CascadeS_Transmit_Descriptor_End(U8_T remote_devinx);
void 	CascadeS_Transmit_USB_Device_Unmount(U8_T remote_devinx);
void 	CascadeS_ReceiveInterruptInData(CASCADE_PacketHeader *packet);
void 	CascadeS_PassThrough_Done_Check(U8_T devinx,U8_T port);
//void 	CascadeS_CopyRxDmaToBuf(U16_T start,U8_T len,U8_T *rbuf);
void 	CascadeS_Audio_Data_In_Stage(CASCADE_PacketHeader *packet);
void 	CascadeS_Audio_Data_Out_Enable(CASCADE_PacketHeader *packet);
void 	CascadeS_Setup_Command_Handle(U8_T devinx,USB_Setup_TypeDef *setup);
void 	CascadeS_Intr_Data_Out_Handle(U8_T port,U8_T devinx, U8_T endpinx);
void 	CascadeS_KVM_CONSOLE_Keyboard_Led_Control(U8_T port,U8_T kbled);
void 	CascadeS_Audio_Data_In_Buf_Check(void);
void 	CascadeS_Change_IntTransfer_Interval(U8_T devinx,U8_T endpinx,U8_T interval);
void 	CascadeS_USBDC_Check_Upstream_Suspend_State(U8_T mount_port);
void 	CascadeS_Transmit_MSC_Burst_In_Enable(U8_T port,U8_T devinx);
void 	CascadeS_MSC_BurstDataInFlowHandle(U8_T devinx);
void 	CascadeS_Change_Protocol_Setting(U8_T port,U8_T devinx,U8_T intf,U8_T value);
void 	CascadeS_Change_Alternate_Setting(U8_T devinx,U8_T intf,U8_T value);
void 	CascadeS_Change_Configuration_Setting(U8_T devinx,U8_T value);
void 	CascadeS_Port_Connect_State(U8_T port,U8_T value);
void 	CascadeS_Port_Power_State(U8_T port,U8_T value);
void 	CascadeS_Port_Suspend_State(U8_T port,U8_T value);
void 	CascadeS_Transmit_KVM_Version(void);
void 	CascadeS_Box_Transfer_To_Master(U8_T pid,U8_T devinx);
void 	CascadeS_Port_KVM_Connect_State(U8_T port,U8_T value);
void 	HW_EDID_EnableSlave(void);
/* EXPORTED GLOBAL VARIABLES */
extern	CCSLV_CTRL	CCSLV_Ctrl;
extern 	U8_T 	Remote_Device_Mapping[];
extern  Cascade_Device_Mapping_Def  Remote_DevMap[USBDC_DEVICE_MAX];
extern  U8_T    Remote_Reserve_DevMap[USBDC_DEVICE_MAX];	
extern 	U8_T 	Slave_Control_State;
extern 	U8_T	TASK_USBHC_CascadeS_PassThrough_Handle_ID;
extern 	U8_T 	CascadeS_Device_Passthough_State[USBDC_DEVICE_MAX]; 
extern 	bit		Slave_PassThrough_Buf_In_Used_Flag;  
extern 	U8_T 	CascadeS_Device_Passthrough_Flag;						//each bit standard 1 device
extern 	U8_T 	Slave_PassThrough_Buf_In_Used_Devinx;
extern 	bit		Slave_PassThrough_Buf_In_Used_Flag;
extern 	Cascade_Transimit_Def Trainsmit_FIFO[CASCADE_TRANSMIT_FIFO_MAX];
extern 	U8_T 	CascadeS_Transmit_OutP;
extern  U8_T 	CascadeS_Transmit_InP;
extern  CascadeS_System_State	CascadeS_KVM_State;
extern  Firmware_Version_Def  FirmwareVersion;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
#if (SYSTEM_CASCADE_AUDIO_BUF_CNT)
extern 	Cascade_ISO_Data_Def CADR_USB_Audio_Buf[SYSTEM_CASCADE_AUDIO_BUF_CNT];
extern 	U8_T	CADR_USB_AudioBufAddr[SYSTEM_CASCADE_AUDIO_BUF_CNT][2];
extern	volatile U8_T	CADR_USB_CascadeS_Audio_Inx; //transmit index
extern  volatile U8_T	CADR_USB_CascadeS_Audio_Outx;
extern	volatile U8_T	CADR_USB_DC_Audio_Inx; //transmit index
extern	volatile U8_T	CADR_USB_DC_Audio_Outx;
extern	volatile U8_T	CADR_USB_CascadeS_Buffer_Out;
extern	volatile U8_T	CADR_USB_DC_Buffer_In;
extern  U8_T	USBDC_Audio_In_Interface;
extern  U8_T	USBDC_Audio_Out_Interface;
extern  bit		CascadeS_ISO_DC_IN_Sending_Flag;
#if (CASCADES_AUDIO_BUFFING_MODE)
extern  bit		CascadeS_AudioIn_Buffering_Start_Flag;
//extern 	U16_T  	CascadeS_Rx_Buf_Startp;
//extern  U16_T  	CascadeS_Rx_Buf_Data_Startp;
#endif /* #if (CASCADEM_AUDIO_BUFFING_MODE) */
#endif /* #if (SYSTEM_CASCADE_AUDIO_BUF_CNT) */
#endif /* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */
#endif /* End of __CASCADE_SLAVE_H__ */

/*
$ End of file cascade_slave.h
*/