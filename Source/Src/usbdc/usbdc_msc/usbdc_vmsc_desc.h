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
 * Module Name: usbdc_vmsc_desc.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_VMSC_DESC_H
#define __USB_VMSC_DESC_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported CONSTants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/

#define VMSC_SIZ_DEVICE_DESC		18
#define VMSC_SIZ_CONFIG_DESC		32
#define VMSC_DESCRIPTOR_TYPE		0x21
#define VMSC_SIZ_STRING_PRODUCT		46
#define VMSC_SIZ_STRING_SERIAL		14

/* Exported functions ------------------------------------------------------- */
extern CODE U8_T	VMSC_DeviceDescriptor[VMSC_SIZ_DEVICE_DESC];
extern CODE U8_T	VMSC_ConfigDescriptor[VMSC_SIZ_CONFIG_DESC];
extern CODE U8_T	VMSC_StringProduct[VMSC_SIZ_STRING_PRODUCT];
extern CODE U8_T	VMSC_StringSerial[VMSC_SIZ_STRING_SERIAL];
#endif /* __USB_VMSC_DESC_H */

