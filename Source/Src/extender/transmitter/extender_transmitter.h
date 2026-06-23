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
 * Module Name: extender_transmitter.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __EXTENDER_TRANSMITTER_H__
#define __EXTENDER_TRANSMITTER_H__

/* INCLUDE FILE DECLARATIONS 		  */
/* DEFINATION DECLARATIONS   		  */
#ifdef PWAYTEK_01
#define  DATA_TRANSFER_LED_GPIO					P0_7
#define  HOST_STATE_LED_GPIO					P0_6
#define  RECEIVER_STATE_LED_GPIO				P2_3
#endif

#ifdef PCT_MUA22
#define  DATA_TRANSFER_LED_GPIO					P0_5
#endif

#define EXTENDER_MSC_BulkInWaitMax				32
/*
$ Transmitter_Control_State
*/
#define   TRANSMITTER_RESTART_FLAG   		0x01
#define   TRANSMITTER_USB_PASSTHROUGH_FLAG 	0x02
#define   TRANSMITTER_USB_COMMAND_FLAG	 	0x04
#define   TRANSMITTER_ANSWER_WAIT_FLAG	 	0x08
#define   TRANSMITTER_CONNECT_FLAG 			0x10
#define   TRANSMITTER_KB_LED_FLAG   		0x20
#define   TRANSMITTER_HOST_LED_FLAG   		0x40

/*
$ ExtenderT_Device_Passthough_State
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
$ ExtenderT_Device_USB_Cmd_Send_State
*/
#define   T_USB_CMD_SET_PROTOCOL_FLAG 	 		0x01
#define   T_USB_PASSTHROUGH_INTR_OUT_FLAG		0x02
#define   T_MSC_RESET					 		0x03
#define   T_USB_CMD_PASSTHROUGH_FLAG			0x04
#define   T_MSC_IN_WAIT					 		0x05
#define   T_USB_INTERVAL					 	0x06
#define   T_DEVICE_TABLE					 	0x07



#define   RECEIVER_PLUG_OFF_CNT					2
/*-------------------------------------------------------
$ Transmitter_Wait_Answer_State
*/
//#define	 TRANSMITTER_WAIT_SOF_SEND_ANSWER		0x01


/*-------------------------------------------------------
$ Transmitter_Send_State Define
*/
#define  TRANSMITTER_SEND_WAIT					0x01
#define  TRANSMITTER_SEND_START					0x02
#define  TRANSMITTER_SEND_KB_LED				0x04
#define  TRANSMITTER_SEND_RESTART				0x08
#define  TRANSMITTER_SEND_USB_EVENT				0x10
#define  TRANSMITTER_SEND_COTROL_EVENT			0x20


#define  EXTENDERT_CMD_Q_MAX					32
typedef struct
{
	U8_T	bCmd;	
	U8_T    bIndex;
	U8_T    bValue;
	U8_T    bRequest;
	U8_T    bLen;
	U8_T    bData;
} ExtenderT_Cmd_Q_Typedef;



/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void EXTENDER_Transmitter_Receive_Handle(void);
void EXTENDER_Transmitter_Init(void);
void ExtenderT_Passthrough_Done_Check(U8_T devinx);
U8_T ExtenderT_Setup_Buffer_Malloc(U8_T pid,U8_T devinx);
void ExtenderT_Intr_Data_In_Handle(void);
void ExtenderT_Intr_In_Move_Wait_Handle(U8_T devinx,U8_T intt_id,U8_T endpidx);
void ExtenderT_KVM_KB_Led_Check(U8_T port);
void ExtenderT_USB_Set_Protocol(U8_T devinx,U8_T curinf,U8_T protocol_value);
void ExtenderT_USB_IntrOutProcess(U8_T devinx, U8_T endpinx);
void ExtenderT_HostLedCheck(U8_T pid);
void ExtenderT_HostLedControlReport(void);
void ExtenderT_USB_Send_Command(U8_T devinx,U8_T cmd,U8_T value);
void ExtenderT_Receiver_Data_Send(U8_T *header,U8_T control,U8_T bRequest,U8_T bIndex,U8_T bValue,U8_T dataLen,U8_T data_buf);
void ExtenderT_Receiver_Send_Cmd(U8_T *header,U8_T control,U8_T bRequest,U8_T bIndex,U8_T bValue,U16_T dataLen,U8_T *data_buf);
void ExtenderT_Reset_Remote_MSC(void);
void ExtenderT_MSC_In_Wait(U8_T,U8_T);
void ExtenderT_MSC_VHUB_ResetUpdate(void);
void ExtenderT_Send_Cmd_To_Receiver(U8_T cmd,U8_T request,U8_T index,U8_T value);
#ifdef USB_2
U8_T ExtenderT_Check_Devcie_MountedHost(U8_T devinx);
#endif

#if (SYSTEM_EXTENDER_MSC_SUPPORT)
/* RS485 Extender Transmitter for MSC */
void ExtenderT_MSC_DcInBufClear(U8_T port,U8_T devinx,U8_T endpinx);
void ExtenderT_MSC_DcBulkInStallHandle(U8_T devinx);
#if (SYSTEM_USB_HC_BURST)
void ExtenderT_MSC_BurstDataOutStage(void);
void ExtenderT_MSC_BurstOutComplete(void);
U8_T ExtenderT_MSC_DcBurstOutHandle(U8_T devinx);
U8_T ExtenderT_MSC_BurstOutDataCheck(U8_T devinx);
void ExtenderT_MSC_DcBurstInHandle(U8_T devinx);
void ExtenderT_MSC_BurstDataInHandle(Extender_Cmd_Handle_Def *setup);
U8_T ExtenderT_MSC_BurstInDataCheck(U8_T devinx, U8_T *pBuf, U16_T len);
#else
void ExtenderT_MSC_BulkDataOutStage(void);
U8_T ExtenderT_MSC_DcBulkOutReceiveHandle(U8_T portId, U8_T devIndex, U8_T endpIndex);
U8_T ExtenderT_MSC_BulkOutDataCheck(U8_T port, U8_T devinx, U8_T *pBuf, U16_T len);
void ExtenderT_MSC_BulkDataInHandle(Extender_Cmd_Handle_Def *setup);
void ExtenderT_MSC_DcBulkInTransmitHandle(void);
U8_T ExtenderT_MSC_BulkInDataCheck(U8_T devinx, U8_T *pBuf, U16_T len);
#endif //#if (SYSTEM_USB_HC_BURST)
#endif //#if (SYSTEM_EXTENDER_MSC_SUPPORT)
void ExtenderT_Maintain_HostState_Led(U8_T ledconrol);
void ExtenderT_Buzzer_Control(U8_T value);
void ExtenderT_Audio_SW_Handle(U8_T newport);
void ExtenderT_Audio_AutoSW_Handle(U8_T value);
void ExtenderT_Msc_SW_Handle(U8_T newport);
void ExtenderT_System_Control(U8_T value);
void ExtenderT_Reset_Remote_MSC_Req(void);
void ExtenderT_MSC_BurstDataInSendToDC(Extender_Data_Packet_Def *setup,U8_T *pInPkt);
void ExtenderT_Passthrough_Condition_Reset(U8_T devinx);
void ExtenderT_Remote_Interval(U8_T devinx,U8_T value);
void ExtenderT_Remote_Interval_Req(U8_T devinx,U8_T enpdinx,U8_T interval);
#ifdef DEVICE_KEEP
void Transmitter_Check_Remote_NewDevice(Extender_Cmd_Handle_Def *setup);
void Transmitter_Clear_DeviceKeep_Flag(void);
void Transmitter_Clear_Old_DeviceKeep(void);
void Transmitter_Check_Device_Table(Extender_Cmd_Handle_Def *setup);
#endif
/* EXPORTED GLOBAL VARIABLES */
extern U8_T Remote_Device_Mapping[];
extern U8_T Transmitter_Control_State;
extern U8_T	TASK_USBHC_ExtenderT_PassThrough_Handle_ID;
extern U8_T ExtenderT_Device_Passthough_State[USBDC_DEVICE_MAX]; 
extern bit	Tansmitter_PassThrough_Buf_In_Used_Flag;  
extern U8_T ExtenderT_Device_Passthrough_Flag;						//each bit standard 1 device
extern U8_T Tansmitter_PassThrough_Buf_In_Used_Devinx;
extern U8_T	*EXTENDER_MSC_BulkInWait[EXTENDER_MSC_BulkInWaitMax];
extern U8_T	EXTENDER_MSC_BulkInWaitHead;
extern U8_T	EXTENDER_MSC_BulkInWaitTail;
extern U8_T	Remote_Reserve_DevMap[USBDC_DEVICE_MAX];
extern bit 	EXTENDER_MSC_BurstOutWait_Clear_Flag;
extern U8_T UART_Disable_Flag;
#endif /* End of __EXTENDER_TRANSMITTER_H__ */

/*
$ End of file extender_transmitter.h
*/