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
 * Module Name : usb_kernal.h
 * Purpose     : 
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

#ifndef __USB_KERNAL_H__
#define __USB_KERNAL_H__

/--------------------------------
//USB information structure
//--------------------------------
//typedef struct _USB_Virtual_Control
//{
//	U8_T  HubNum[USBDC_PORT_MAX];			// Virtual Hub Port,bit0-indicate port0 ......
//	U8_T  DevAddr[USBDC_PORT_MAX];			// Virtual Device Address for PC from 0~127
//	U8_T  Feature[USBDC_PORT_MAX];			// bit0-Suspend
											// bit5-Remote  
											// bit6-Bus Power
//	U8_T  Configuration[USBDC_PORT_MAX];	// Configuration for each port
//	U8_T  Interface[USBDC_PORT_MAX];		// Interface Setting Value
//	U8_T  Alternate[USBDC_PORT_MAX];		// Alternate Setting value
//	U8_T  Protocol[USBDC_PORT_MAX];			// each bit standard for eeach port
											// 0-report 1-boot

//	U8_T  ISR_Handle[USBDC_PORT_MAX];
//	U8_T  *Control_EndpBuf[USBDC_PORT_MAX];	//each endpoint buffer address 	
//	U16_T Ctrl_TotalByte[USBDC_PORT_MAX];	//Current Transfer-total byte to received or send
//	U16_T Ctrl_CurrentByte[USBDC_PORT_MAX];	//Current Transfer-how many byte has been received or sended
//	DCSETUP_CONTROL_STATE  ControlState[USBDC_PORT_MAX];
//	USB_Setup_TypeDef  Setup[USBDC_PORT_MAX];
//} USB_Virtual_Control;

#endif /* End of __USB_KERNAL_H__ */