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
 * Module Name: usbdc_vhub_desc.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_HUB_DESC_H
#define __USB_HUB_DESC_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported CONSTants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/

#define HID_DESCRIPTOR_TYPE		0x21
#define HUB_SIZ_HID_DESC		0x09
#define HUB_OFF_HID_DESC		0x12
#define HUB_OFF_HID2_DESC		0x2b

#define HUB_SIZ_DEVICE_DESC		18
#define HUB_SIZ_CONFIG_DESC		25
#define HUB_SIZ_DESCRIPTOR		9
#define HUB_SIZ_STRING_LANGID	4
#define HUB_SIZ_STRING_VENDOR	34
#define HUB_SIZ_STRING_PRODUCT	64
#define HUB_SIZ_STRING_SERIAL	32

#define STANDARD_ENDPOINT_DESC_SIZE		0x09

/* Exported functions ------------------------------------------------------- */
extern U8_T	HUB_DeviceDescriptor[HUB_SIZ_DEVICE_DESC];
extern CODE U8_T HUB_ConfigDescriptor[HUB_SIZ_CONFIG_DESC];
extern CODE U8_T HUB_StringLangID[HUB_SIZ_STRING_LANGID];
extern CODE U8_T HUB_StringVendor[HUB_SIZ_STRING_VENDOR];
extern U8_T XDATA HUB_StringProduct[HUB_SIZ_STRING_PRODUCT];
extern U8_T XDATA HUB_StringSerial[HUB_SIZ_STRING_SERIAL];
extern CODE U8_T HUB_Descriptor[HUB_SIZ_DESCRIPTOR];
#endif /* __USB_DESC_H */
