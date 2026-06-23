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
 * Module Name : usbhc_msc_burst.h
 * Purpose     : A header file of usbhc_msc.c
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

#ifndef __USB_HC_MSC_BURST_H__
#define __USB_HC_MSC_BURST_H__

/* INCLUDE FILE DECLARATIONS */

/* Define Area */ 
#define MSC_BULK_ONLY_TRANSPORT				0x50
/* USBHC_MSC_Td Control */
/* Bit0~4 => ATL TD Number
   Bit7~5 => TD  Control 
      000 => IDLE
      001 =>  
*/

//#define MSC_TD_ACTIVE		0x01
//#define MSC_TD_IDLE			0x00

/* 
$ USBHC_MSC_Active_State;
*/
#define MSC_ACTIVE_IDLE						0x00
#define MSC_ACTIVE_START					0x01
#define MSC_ACTIVE_BUSY						0x02
#define MSC_ACTIVE_SWITCH_START				0x04
#define MSC_ACTIVE_SWITCH_WAIT1				0x08
#define MSC_ACTIVE_SWITCH_WAIT2				0x10
#define MSC_ACTIVE_TEST_UNIT_READY			0x20
#define MSC_ACTIVE_TIME_UP					0x40
#define MSC_ACTIVE_REPORT_UMOUNT			0x80

/* 
$ USBHC_MSC_State
*/
//#define MSC_STATE_IDLE						0x00
//#define MSC_STATE_PAGE_FULL					0x01
//#define MSC_STATE_WAIT						0x02
#define MSC_STATE_TERNIMATE					0x04
#define MSC_STATE_BULK_IN					0x08
#define MSC_STATE_CHECK_BULK_IN				0x10
//#define MSC_STATE_WAIT_START				0x20
//#define MSC_STATE_DEVICE_IDLE				0x40
//#define MSC_STATE_DMA_WRITE_WAIT			0x80



/* NAMING CONSTANT DECLARATIONS */

#define BURST_IDLE 	    	0x00
#define BURST_0_USED   		0x01
#define BURST_1_USED    	0x02
#define BURST_0_DONE  		0x04
#define BURST_1_DONE   		0x08
#define BURST_IN_WAIT  		0x10
#define BURST_OUT_WAIT 		0x20
#define BURST_COMPLETE 		0x40
#define BURST_ERROR     	0x80



//#define USBHC_MSC_TD_START_INX				(USB_HC_MSC_START)

/** @defgroup USBHC_MSC_CORE_Exported_Defines
  * @{
  */
//#define MSC_BURST_OUT			0x00 //from Host to HC
//#define MSC_BURST_IN			0x01 //from Host to HC

 
/* Global Varable Declartion */
extern U8_T	USBHC_MSC_Device_Cnt;   // How may msc device has been attached 
extern U8_T	USBHC_MSC_Devinx;					//Devinx for each MSC device
extern U8_T	USBHC_MSC_State;
extern U8_T	TASK_USBHC_MSC_Idle_Check_ID;
extern U8_T	USBHC_MSC_Active_State;
extern U8_T	KVM_CurrentMSC_Next;
extern U8_T	TASK_USBHC_MSC_Idle_Check_Active_ID;
extern U8_T	USBHC_HubPortNumber;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//For Burst Mode Used
extern U8_T USBHC_MSC_PingPongOut_ID;  	//store current active OUT TD id
extern U8_T USBHC_MSC_PingPongIn_ID;  	//store current active IN TD id
extern U8_T	USBHC_MSC_UpstreamInTD;
extern U8_T USBHC_MSC_Burst_Used[4];
extern U8_T USBHC_MSC_Burst_Done[4];
//extern U8_T 						USBHC_MSC_DC_In_ID; 		//store current active DC TD id
extern U8_T	USBHC_MSC_ResetDevinx;		//Devinx for each MSC device
extern U8_T	USBHC_MSC_Reset_DCPort;
extern U8_T USBHC_MSC_Reset_Flag;
extern U8_T	USBHC_MSC_CurrentOut_TD;
extern U8_T	USBHC_MSC_NextOut_TD;


/* Functin Declartion */
void	USBHC_MSC_Init(void);
RESULT	USBHC_MSC_Class_Verify(U8_T devinx);
void	USBHC_MSC_Bulk_TD_Header(U8_T devinx,U8_T atl_id,U8_T direction,U16_T total_byte);
void	USBHC_MSC_Bulk_State_Reset(U8_T devinx);
void	USBHC_MSC_TD_Assign_Devinx(U8_T devinx);
void	USBHC_MSC_Port_Switch(U8_T,U8_T);
void	USBHC_MSC_Switch_Stage_1(void);
void	USBHC_MSC_Umount_Report(void);
void	USBHC_MSC_Flush_DC_Buffer(U8_T port,U8_T devinx,U8_T endpinx);
void	USBHC_MSC_Bulk_Clear_Pipe(U8_T devinx, U8_T pipeDir);
void 	USBHC_MSC_VHUB_ResetUpdate(void);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//For Burst Mode Used
U8_T 	USBHC_MSC_DC_Burst_Out_Handle(U8_T devinx);
U8_T 	USBHC_MSC_DC_Burst_In_Handle(U8_T devinx);
U8_T 	USBHC_MSC_EnableNextBurstTD(U8_T burst_state);
void 	USBHC_MSC_Get_Burst_Length_From_DC(U8_T devinx,U8_T bulk_td);
void 	USBHC_MSC_Set_DC_Burst_Length(U8_T burst_td,U16_T burst_length);
U8_T 	USBHC_MSC_Device_Error_Condition_Check(U8_T devinx);
void 	USBHC_MSC_EnableCurrentBurstTD(U8_T burst_state,U8_T td_id);
U8_T 	USBHC_MSC_GetNext_PingPongOut(U8_T current_pingpong);
U8_T 	USBHC_MSC_GetNextOut_TD(U8_T current_td);
U8_T    USBHC_MSC_PingPongOut_UseState(U8_T pingpong_id);
U8_T 	USBHC_MSC_Next_PingPongOut_UseState(U8_T pingpon_id);
void 	USBHC_MSC_Length_To_Buf(U8_T *buf,U16_T iLen);
void 	USBHC_MSC_Set_CurrentPingPongOut_Used(U8_T devinx);
U8_T 	USBHC_MSC_GetNext_PingPongIn_TD(U8_T pingpong_id);
U8_T 	USBHC_MSC_GetNext_PingPongOut_TD(U8_T bulk_td);
#endif /* End of __USB_HC_MSC_BURST_H__ */
/* End of usbhc_msc.h */