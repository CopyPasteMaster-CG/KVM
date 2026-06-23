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
 * Module Name : usbhc_roothub.h
 * Purpose     : A header file of usbdc_roothub.c
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

#ifndef __USB_HC_ROOTHUB_H__
#define __USB_HC_ROOTHUB_H__

/* INCLUDE FILE DECLARATIONS */ 
/* DATA STRUCTURE DECLARATIONS */
/* NAMING CONSTANT DECLARATIONS */  
#define USBHC_ROOTHUB_PORT_POWER_ON_STAGE       0x01
#define USBHC_ROOTHUB_PORT_POWER_WAIT_STAGE     0x02
#define USBHC_ROOTHUB_PORT_ENUMERATE_WAIT_STAGE 0x03 //wait for host assing a new address to the port

#define HUB_OP_IDLE   0x00
#define HUB_OP_RESET  0x01
#define HUB_OP_ENABLE 0x02

/* $$ GLOBAL VARIABLES $$ */
extern bit USBHC_RootHub_Bus_Occupy_Flag;
extern U8_T USBHC_RootHub_State_Report[USB_HC_MAX_PORT],USBHC_RootHub_StatusChanged_Report[USB_HC_MAX_PORT];
extern U8_T TASK_USBHC_StatusChanged_Handle_ID;
extern U8_T USBHC_RootHub_State_Hold[USB_HC_MAX_PORT];
extern TASK_ActiveTable_TypeDef TASK_USBHC_HUB_RemoveDevinx_Task;
extern U8_T USBHC_RootHub_ResetCnt;
extern U8_T USBHC_RootHub_Bus_Occupy_ID;
extern U8_T USBHC_RootHub_Devinx[USB_HC_MAX_PORT];
/* $$ EXPORTED SUBPROGRAM SPECIFICATIONS $$ */
void TASK_USBHC_RootHub_PowerON(void);
void USBHC_RootHub_Init(void);
void USBHC_RootHub_StatusChanged_Handle(void);
void USBHC_RootHub_Reset_Port(U8_T port,U16_T reset_timer);

#endif /* End of __USB_HC_H__ */
/* End of uart.h */
