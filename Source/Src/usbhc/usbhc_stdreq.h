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
 * Module Name : usbhc_stdreq.h
 * Purpose     :
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */ 

#ifndef __USBH_STDREQ_H
#define __USBH_STDREQ_H

/*Standard Feature Selector for clear feature command*/
#define FEATURE_SELECTOR_ENDPOINT			0x00
#define FEATURE_SELECTOR_DEVICE				0x01
#define INTERFACE_DESC_TYPE					0x04
#define ENDPOINT_DESC_TYPE					0x05
#define INTERFACE_DESC_SIZE					0x09
#define USBH_HID_CLASS						0x03


USBH_Status USBHC_Stdreq_SetAddress(U8_T devinx,U8_T td_id,U8_T DeviceAddress);
USBH_Status USBHC_Stdreq_GetDevDesc(U8_T devinx,U8_T td_id,U16_T length);
USBH_Status USBHC_GetDescriptor(U8_T devinx,U8_T td_id,U8_T req_type,U16_T value_idx,U16_T interface,U16_T length);
USBH_Status USBHC_Stdreq_GetCfgDesc(U8_T devinx,U8_T td_id,U16_T length);
USBH_Status USBHC_Stdreq_SetCfg(U8_T devinx,U8_T td_id,U8_T cfg_idx);
USBH_Status USBHC_Stdreq_GetStringDesc(U8_T devinx,U8_T td_id,U8_T string_idx,U8_T length);
USBH_Status USBHC_Stdreq_PatternCommand(U8_T devinx,U8_T td_id,U8_T *setup,U16_T length);
USBH_Status USBHC_Stdreq_SetFeature(U8_T devinx,U8_T td_id,U8_T receipient,U16_T wvalue,U16_T windex);
//USBH_Status USBHC_Stdreq_ClearFeature(U8_T devinx,U8_T td_id,U8_T receipient,U16_T wvalue,U16_T windex);
USBH_Status USBHC_Stdreq_SetInterface(U8_T devinx,U8_T td_id,U8_T wvalue,U8_T windex);

#endif /* __USBH_STDREQ_H */
