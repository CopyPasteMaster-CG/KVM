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
 * Module Name: usbdc_vhid_desc.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_VHID_DESC_H
#define __USB_VHID_DESC_H
/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported CONSTants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#ifndef VHID_X_MAX
#define VHID_X_MAX				1024
#endif
#define VHID_X_MAX_MSB			(VHID_X_MAX & 0xff00) >> 8
#define VHID_X_MAX_LSB			(VHID_X_MAX & 0x00ff)

#ifndef VHID_Y_MAX
#define VHID_Y_MAX				768
#endif
#define VHID_Y_MAX_MSB			(VHID_Y_MAX & 0xff00) >> 8
#define VHID_Y_MAX_LSB			(VHID_Y_MAX & 0x00ff)


#define VHID_SIZ_DEVICE_DESC	18

#if (VHID_INTERFACE_NUM==1)
  #define VHID_SIZ_CONFIG_DESC 	34
#endif 

#if (VHID_INTERFACE_NUM==2)
  #define VHID_SIZ_CONFIG_DESC	59
#endif

#if (VHID_INTERFACE_NUM==3)
  #define VHID_SIZ_CONFIG_DESC	84
#endif

#define VHID_DESCRIPTOR_TYPE	0x21
#define VHID_SIZ_HID_DESC		0x09
#define VHID_OFF_HID_DESC		0x12
#define VHID_OFF_HID2_DESC		0x2b

#define VHID_SIZ_REPORT_DESC		63
#define VHID_SIZ_REPORT2_DESC		110
#define VHID_SIZ_REPORT3_DESC		93

#define VHID_SIZ_STRING_LANGID		4
#define VHID_SIZ_STRING_VENDOR		34
#define VHID_SIZ_STRING_PRODUCT		64
#define VHID_SIZ_STRING_SERIAL		32

/* Exported functions ------------------------------------------------------- */
extern U8_T VHID_DeviceDescriptor[VHID_SIZ_DEVICE_DESC];
extern U8_T VHID_ConfigDescriptor[VHID_SIZ_CONFIG_DESC];
extern U8_T VHID_ReportDescriptor[VHID_SIZ_REPORT_DESC];
extern U8_T VHID_Report2Descriptor[VHID_SIZ_REPORT2_DESC];
extern U8_T VHID_Report3Descriptor[VHID_SIZ_REPORT3_DESC];
extern U8_T VHID_StringLangID[VHID_SIZ_STRING_LANGID];
extern U8_T VHID_StringVendor[VHID_SIZ_STRING_VENDOR];
extern U8_T XDATA VHID_StringProduct[VHID_SIZ_STRING_PRODUCT];
extern U8_T XDATA VHID_StringSerial[VHID_SIZ_STRING_SERIAL];
#endif /* __USB_VHID_DESC_H */
