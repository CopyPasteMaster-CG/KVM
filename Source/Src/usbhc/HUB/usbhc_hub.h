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
 * Module Name : usbhc_hub.h
 * Purpose     : A header file of usbhc_hub.c
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

#ifndef __USB_HC_HUB_H__
#define __USB_HC_HUB_H__

//#define USBHC_HUB_MAX_PORT				10
#define SET_FEATURE						0x03
#define CLEAR_FEATURE					0x01

/* INCLUDE FILE DECLARATIONS */
/* Define Area */
/*
$USBHC_HUB_Stage
*/
//#define HUB_INIT_IDLE					0x00
#define HUB_POWER_PORT					0x00
#define HUB_INIT_POWER					0x01
//#define HUB_INIT_PORT_STATUS			0x02 // Get all Port status
//#define HUB_INIT_COMPLETE				0xff


/* Global Varable Declartion */
extern U8_T			TASK_USBHC_HUB_Class_Enumerate_ID;
extern U8_T			TASK_USBHC_HUB_RemoveDevinx_ID;
extern TASK_ActiveTable_TypeDef	TASK_USBHC_HUB_Class_Enumerate_Task;
extern U8_T			TASK_USBHC_HUB_Port_Reset_ID;
extern U8_T			USBHC_HUB_Enumerate_Occupy_Flag;


/* Functin Declartion */
void USBHC_HUB_Class_Enum_Init(void);
void USBHC_HUB_StatusChangeHandle(U8_T devinx,U8_T inttid);
void USBHC_HUB_ReleaseAddress(U8_T addr);
void TASK_USBHC_HUB_RemoveDevinx(void);
void USB_Hub_Class_Enumerate_Fork(U8_T devinx);
RESULT USBHC_Class_HubFeature(U8_T devinx,U8_T td_id,U8_T usbhc_hub_port,U8_T usbhc_hub_featuretype,U16_T usbhc_hub_feature);
#endif /* End of __USB_HC_HUB_H__ */
/* End of usbhc_hub.h */