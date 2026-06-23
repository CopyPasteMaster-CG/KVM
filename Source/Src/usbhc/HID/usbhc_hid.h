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
 * Module Name : usbhc_hid.h
 * Purpose     : A header file of usbhc_hid.c
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

#ifndef __USB_HC_HID_H__
#define __USB_HC_HID_H__

/* INCLUDE FILE DECLARATIONS */

extern U8_T		TASK_USBHC_HID_Class_Enumerate_ID;

void HID_Class_Enum_Init(void);
void TASK_USBHC_HID_Class_Enumerate(void);
USBH_Status USBHC_Classreq_SetIdle(U8_T devinx,U8_T td_id,U8_T duration,U8_T reportId,U8_T interface);
USBH_Status USBHC_Classreq_GetRptDesc (U8_T devinx,U8_T td_id,U8_T interface,U16_T length);
USBH_Status USBHC_Classreq_SetReport (U8_T devinx,U8_T td_id,U8_T interface,U8_T reportId,U8_T reportType,U16_T reportLen,U8_T *rptbuf);
void USBHC_CORE_Build_INTL_Polling(U8_T);
USBH_Status USBHC_Classreq_SetProtocol(U8_T devinx,U8_T td_id,U8_T inf_id,U8_T mode);
RESULT USBHC_SCM_SetUnifyingReport(U8_T devinx,U8_T td_id,U8_T mode);
RESULT USBHC_SCM_SetUnifyingHID(U8_T devinx,U8_T td_id,U8_T mode);
void USBHC_HID_Interface_Check(U8_T devinx,U8_T inf_id);
#endif /* End of __USB_HC_HID_H__ */
/* End of uart.h */