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
 * Module Name: usbhc_ioreq.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */

#ifndef __USBH_IOREQ_H
#define __USBH_IOREQ_H

#include "usbhc_def.h"

#define USBHC_SETUP_PKT_SIZE			8
#define USBHC_EP0_EP_NUM				0
#define USBHC_MAX_PACKET_SIZE			USB_HC_ATL_BUF_SIZE

#define HEADER_TOGGLE_DATA0				0
#define HEADER_TOGGLE_DATA1				1
#define HEADER_TOGGLE_BY_CHANNEL		2 //Used the Data Toggle set in Host Channel Table
#define HEADER_TOGGLE_BY_TD				3 //Used the Data Toggle set in TD Header

void USBHC_CtlSendSetup (U8_T devinx,U8_T td_id);
USBH_Status USBHC_CtlReq (U8_T devinx,U8_T td_id);
USBH_Status USBHC_CtlReceiveData(U8_T devinx,U8_T td_id,U16_T length,U8_T status);
USBH_Status USBHC_CtlSendData (U8_T devinx,U8_T td_id,U16_T length);
USBH_Status USBHC_InterruptReqData(U8_T devinx,U8_T intt_id);
USBH_Status USBHC_Istl_ReqData(U8_T devinx,U8_T intt_id);

#endif /* __USBH_IOREQ_H */
