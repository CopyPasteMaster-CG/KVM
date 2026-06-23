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
 * Module Name : logitech.h
 * Purpose     : A header file of logitech.c
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

#ifndef __USB_LOGITECH_H__
#define __USB_LOGITHEC_H__

/* INCLUDE FILE DECLARATIONS */
/* GLOBAL VARABLE DECLARATIONS */ 
extern U8_T UNIFYING_Switch_DJ_Command[];
extern U8_T UNIFYING_Switch_HID_Command[];
extern U8_T UNIFYING_Switch_HIDPP_Command[];
extern U8_T UNIFYING_Switch_HID0_Command[];
/* EXTERNAL SUB-FUNCTION DECLARATIONS */

RESULT USBDC_Unifying_SetReport_Cmd_Check(U8_T port,U8_T devinx,U8_T report_id);
void USBDC_Unifying_SetReport_Data_Check(U8_T port,U8_T devinx,U8_T *DataBuffer);
void USBHC_Unifying_KVM_Switch_Check(U8_T port,U8_T devinx);
U8_T USBHC_Unifying_Interrupt_In_Check(U8_T port,U8_T devinx,U8_T intt_id,U8_T *DJ_Buf,U8_T len,U8_T deviceid);
#endif /* End of __USB_LOGITECH_H__ */
/* End of uart.h */