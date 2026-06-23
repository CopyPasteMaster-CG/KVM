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
 * Module Name : usbhc_msc_bot_burst.h
 * Purpose     : A header file of usbhc_msc_bot_burst.c
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

#ifndef __USB_HC_MSC_BOT_BURST_H__
#define __USB_HC_MSC_BOT_BURST_H__


/** @defgroup USBHC_MSC_BOT_Exported_Defines
  * @{
  */ 
#define USBHC_MSC_BOT_CBW_SIGNATURE			0x55534243
//#define USBHC_MSC_BOT_CBW_TAG				0x20304050
#define USBHC_MSC_BOT_CSW_SIGNATURE			0x55534253
#define USBHC_MSC_BOT_CBW_LENGTH			31
#define USBHC_MSC_BOT_CSW_LENGTH			13

/* CSW Status Definitions */

//#define USBHC_MSC_DIR_IN			0
//#define USBHC_MSC_DIR_OUT			1
//#define USBHC_MSC_BOTH_DIR			2

#define USB_REQ_BOT_RESET			0xFF
#define USB_REQ_GET_MAX_LUN			0xFE

  
/* Global Varable Declartion */
//extern 	bit 	 USBHC_MSC_Done_Middleway_Flag;
//extern 	U16_T USBHC_MSC_Done_Middleway;

/* Functin Declartion */
RESULT USBHC_MSC_BOT_Done_Handle(U8_T devinx, U8_T td_id, U8_T jump);
RESULT USBHC_MSC_BOT_CBW_Decode(U8_T devinx,U16_T len,U8_T *buf);
RESULT 	USBHC_MSC_BOT_DC_Data_Out_Handle(U8_T devinx);
void 	USBHC_MSC_BOT_Send_CSW_Request(U8_T devinx,U8_T current_td);
void 	USBHC_MSC_BOT_Check_Next_In(U8_T devinx,U8_T atl_id);
void 	USBHC_MSC_CSW_After_Control(U8_T endpinx);
void 	USBHC_MSC_BOT_Get_Next_Burst_Length(U8_T devinx);
void 	USBHC_MSC_BOT_Next_HC_Out(U8_T devinx,U8_T current_td,U8_T next_td);
#endif /* End of __USB_HC_MSC_BOT_BURST_H__ */
/* End of usbhc_msc_bot.h */