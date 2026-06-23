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
 * Module Name: extender_receiver.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __EXTENDER_RECEIVER_H__
#define __EXTENDER_RECEIVER_H__

/* INCLUDE FILE DECLARATIONS 		  */
/* DEFINATION DECLARATIONS   		  */
#define  PS2_KB_BUF_MAX_CNT				128
#define  PS2_KB_TRANS_BUF_MAX_CNT		32
#define  PS2_MS_BUF_MAX_CNT				20
#define  PS2_MS_TRANS_BUF_MAX_CNT		 4

#define  RECEIVER_CHECK_DELAY_TIME		100

#ifdef PCT_MUA22
#define   DATA_TRANSFER_LED_GPIO		P0_5
#define   REMOTE_HOST_PORT			  	0x01
#else
#define   REMOTE_HOST_PORT			  	0x00
#endif
//--------------------------------------------
//$ RL_Link_LED_State
//--------------------------------------------
#define   REMOTE_SELECT_LED			  	0x00
#define   LOCAL_SELECT_LED			  	0x01

//--------------------------------------------
//$Receiver_Control_State Define
//
#define   TRANSMITTER_CONNECTION_MASK  	0x01
#define   TRANSMITTER_EDID_MASK  		0x02
#define   TRANSMITTER_PLUG_CHECK_MASK 	0x04
#define   RECEIVER_RESTART_MASK	  		0x08
#define   RECEIVER_SEND_WAIT			0x10
#define   RECEIVER_SEND_START			0x20
#define   TRANSMITTER_RESTART_MASK 		0x40

//-----------------------------------------------
//$ Receiver_Passthrough_State
//-----------------------------------------------
#define   RECEIVER_PASSTHROUGH_IDLE				0x00
#define   RECEIVER_PASSTHROUGH_SETUP_MASK		0x01
#define   RECEIVER_PASSTHROUGH_IN_MASK			0x02
#define   RECEIVER_PASSTHROUGH_IN_SEND_MASK		0x04
#define   RECEIVER_PASSTHROUGH_OUT_MASK			0x08
#define   RECEIVER_PASSTHROUGH_OUT_PAGE_MASK	0x10
#define   RECEIVER_PASSTHROUGH_STATUS_IN_MASK	0x20
#define   RECEIVER_PASSTHROUGH_STATUS_OUT_MASK	0x40
#define   RECEIVER_PASSTHROUGH_SETUP_DONE_MASK	0x80

#define   TRANSMITTER_PLUG_OFF_CNT				1

#if (SYSTEM_EXTENDER_RS232_MODE)
#define   TRANSMITTER_CHECK_TIME				100
#else
#ifdef BR19200
#define   TRANSMITTER_CHECK_TIME				200
#else
#define   TRANSMITTER_CHECK_TIME				1
#endif
#endif //SYSTEM_EXTENDER_RS232_MODE

#ifdef PWAYTEK_01
#define  DATA_TRANSFER_LED_GPIO					P0_7
#define  DEVICE_STATE_LED_GPIO					P0_6
#define  TRANSMITTER_STATE_LED_GPIO				P2_3
#endif

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void EXTENDER_Receiver_Init(void);
void EXTENDER_Receiver_Receive_Handle(void);
U8_T ExtenderR_Resonsed_Received_Handle(void);
void Extender_Receiver_Transmit_TimeOut_Check(void);
U8_T Externder_Receiver_DeviceMount_Control(U8_T devinx,U8_T mounttype,U8_T brequst);
void Externder_Receiver_EDID_Control(void);
void Externder_Receiver_EDID_PlugOff(void);
void ExtenderR_Remote_Link_Led_Flash(U8_T); 
U8_T ExtenderR_Setup_Buffer_Malloc(U8_T devinx);
U8_T ExtenderR_Check_Transmitter_Plug_State(void);
U8_T ExtenderR_Malloc_Intr_Buffer(U8_T td_id,U8_T endpsize);
void ExtenderR_Transmit_Intr_Endp_In_Data(U8_T devinx,U8_T td_id,U8_T vhid_inf,U8_T data_len);
void ExtenderR_Interrupt_Transfer_In_Data_Handle(U8_T devinx,U8_T intt_id,U8_T vhid_inf);
void ExtenderR_PassthroughHostLedControlHandle(U8_T ledStatus);
void ExtenderR_Passthrough_SetupOutProcess(U8_T port, U8_T devinx);
void ExtenderR_Transmitter_Port_Switch(U8_T transmitter_port);
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
void TASK_ExtenderR_AudioInWait(void);
void ExtenderR_Audio_Data_In_Stage(void);
#endif
#if (SYSTEM_EXTENDER_MSC_SUPPORT)
#if (SYSTEM_USB_HC_BURST)
void ExtenderR_PassthroughMscBurstOutHandle(void);
void ExtenderR_PassthroughMscBurstOutRetry(void);
void ExtenderR_MscBurstOutDataHandle(void);
RESULT ExtenderR_MSC_BurstDataOutCheck(U8_T devinx);
RESULT ExtenderR_MSC_Burst_Done_Handle(U8_T devinx, U8_T atl_id, U8_T jump);
void ExtenderR_PassthroughMscBurstInHandle(void);
void ExtenderR_PassthroughMscBurstInComplete(void);
void ExtenderR_PassthroughMscBulkInStall(U8_T devinx, U8_T direction, U8_T atl_id);
void ExtenderR_MSC_BurstWaitHandle(void);
#endif //#if (SYSTEM_USB_HC_BURST)
#endif //#if (SYSTEM_EXTENDER_MSC_SUPPORT)
void ExtenderR_Interrupt_DataTypeTransfer_Check(U8_T index);
U8_T Extender_Packet_Int_Data(Extender_Data_Int_Packet_Def *header,U8_T data_len,U8_T *data_buf,U8_T bIndex,U8_T bValue);
void ExtenderR_Maintain_DeviceState_Led(void);
void ExtenderR_Transmitter_Buzzer_Control(U8_T value);
void ExtenderR_Transmit_System_Setting(void);
void ExtenderR_Transmitter_AudioComboSw_Control(U8_T value);
void ExtenderR_Transmitter_AudioSw_Control(U8_T value);
void ExtenderR_Transmitter_MscSw_Control(U8_T value);
void ExtenderR_MscResetHandle(void);
void ExtenderR_Remote_MscResetCheck(U8_T devinx);
void ExtenderR_MscInWaitHandle(U8_T value);
void ExtenderR_MSC_Burst_In_Wait_Check(U8_T devinx);
void ExtenderR_PS2_DataTransfer(U8_T intt_id,U8_T *data_buf,U8_T data_len);
void ExtenderR_USB_Interval_Handle(U8_T index,U8_T value);
void ExtenderR_Transmit_MSC_Out_Ready(void);
#ifdef DEVICE_KEEP
void ExtenderR_Remote_Device_List_Contorl(U8_T control);
void ExtenderR_Device_Table_Transfer(void);
void ExtenderR_Device_Table_Check(void);
#endif	
/* EXPORTED GLOBAL VARIABLES */
extern U8_T 	Receiver_Control_State;
extern U8_T		TASK_USBHC_ExtenderR_PassThrough_ID;
extern bit  	Remote_Link_LED_Flash_Flag;
extern U8_T		ExtenderR_Resend_Cnt;
extern U8_T		edtr_R_DeviceMount[];
extern U8_T     Remote_Mount_State;
extern U8_T     USBHC_MSC_Reset_Flag;
extern bit      MSC_In_Wait_Flag;
extern U8_T     Msc_In_Serial;
extern bit      ExtenderR_Skip_TimeoutUpdate;
extern U8_T		ExtenderR_Audio_In_Cnt;
extern U8_T		ExtenderR_TD_Skip_Active_Flag;
#ifdef PLUG_DETECT
extern U8_T    UART_Disable_Flag;
#endif  /* #ifdef PLUG_DETECT */
#endif /* End of __EXTENDER_RECEIVER_H__ */

#if (ENABLE_HOTKEY_FUN_RTC_CONTROL)
void ExtenderR_Transmit_RTC_Contorl(void);
#endif

/*
$ End of file extender_receiver.h
*/