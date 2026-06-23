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
 * Module Name: usbdc_vmsc_desc.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */

/* Includes ------------------------------------------------------------------*/
#include "project_include.h"

#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* USB Standard Device Descriptor */

CODE U8_T VMSC_DeviceDescriptor[VMSC_SIZ_DEVICE_DESC] =
{
	0x12,						/*bLength */
	USB_DEVICE_DESCRIPTOR_TYPE,	/*bDescriptorType*/
	0x10,						/*bcdUSB Low*/
	0x01,						/*bcdUSB High*/
	0x00,						/*bDeviceClass*/
	0x00,						/*bDeviceSubClass*/
	0x00,						/*bDeviceProtocol*/
	0x08,						/*bMaxPacketSize40*/
	0x0b, 0x95,					/*idVendor (0x950b)*/
	0x02, 0x0c,					/*idProduct = 0x0ca*/ 
	0x00, 0x02,					/*bcdDevice rel. 2.00*/
	1,							/*Index of string descriptor describing
                                   manufacturer */
	2,							/*Index of string descriptor describing
                                   product*/
	3,							/*Index of string descriptor describing the
                                   device serial number */
	0x01						/*bNumConfigurations*/
}; /* VMSC_DeviceDescriptor */



/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
CODE U8_T VMSC_ConfigDescriptor[VMSC_SIZ_CONFIG_DESC] =
{
	0x09,								/* bLength: Configuration Descriptor size */
	USB_CONFIGURATION_DESCRIPTOR_TYPE,	/* bDescriptorType: Configuration */
	VMSC_SIZ_CONFIG_DESC, 0x00,			/* wTotalLength: Bytes returned */
	
	0x01,								/* bNumInterfaces: 1 interface */
	0x01,								/* bConfigurationValue: Configuration value */
	0x00,								/* iConfiguration: Index of string descriptor describing
                                 the configuration*/
	0x80,								/* bmAttributes: Bus powered */
	0x32,								/* MaxPower 100 mA: this current is used for detecting Vbus */

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//  Descriptor of Custom STORAGE interface  
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~     
	0x09,								/* bLength: Interface Descriptor size */
	USB_INTERFACE_DESCRIPTOR_TYPE,		/* bDescriptorType: Interface descriptor type */
	0x00,								/* bInterfaceNumber: Number of Interface */
	0x00,								/* bAlternateSetting: Alternate setting */
	0x02,								/* bNumEndpoints */
	0x08,								/* bInterfaceClass: MSC */
	0x06,								/* bInterfaceSubClass : 0x06=SCSI transparent command set */
	0x50,								/* nInterfaceProtocol : 0x50=Bulk-Only Transport */
	0x00,								/* iInterface: Index of string descriptor */

	/******************** Descriptor of Storage endpoints ******************/
	/* Endpoint 1 */
	0x07,							/* bLength: Endpoint Descriptor size */
	USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType: */
	0x81,							/* bEndpointAddress: Endpoint Address (IN) 1*/
	0x02,							/* bmAttributes: Bulk endpoint */
	0x40, 0x00,						/* wMaxPacketSize: 64 Bytes max */
	0x00,							/* bInterval: Polling Interval (10 ms) */
	0x07,							/* bLength: Endpoint Descriptor size */
	USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType: */
	0x02,							/* bEndpointAddress: Endpoint Address (OUT) 2*/
	0x02,							/* bmAttributes: Bulk endpoint */
	0x40, 0x00,						/* wMaxPacketSize: 64 Bytes max */
	0x00,							/* bInterval: Polling Interval (10 ms) */ 
} ; /* VMSC_ConfigDescriptor */ 

CODE U8_T VMSC_StringProduct[VMSC_SIZ_STRING_PRODUCT] =
{
	VMSC_SIZ_STRING_PRODUCT,		/* bLength */
	USB_STRING_DESCRIPTOR_TYPE,		/* bDescriptorType */
	'U', 0, //1
	'S', 0, //2
	'B', 0, //3
	' ', 0, //4
	'P', 0, //5
	'e', 0, //6
	'n', 0, //7
	' ', 0, //8
	'D', 0, //9
	'r', 0, //10
	'i', 0, //11
	'v', 0, //12
	'e', 0, //13
	' ', 0, //14
	'V', 0, //15
	'e', 0, //16
	'r', 0, //17
	':', 0, //18
	'0', 0, //19
	'.', 0, //20
	'0', 0, //21
	'1', 0  //22
};

CODE U8_T VMSC_StringSerial[VMSC_SIZ_STRING_SERIAL] =
{
	VMSC_SIZ_STRING_SERIAL,						/* bLength */
	USB_STRING_DESCRIPTOR_TYPE,					/* bDescriptorType */
	'0', 0, '0', 0, '0', 0,'0', 0,'0', 0,'0', 
};
#endif

/* End of usbdc_vmsc_desc.c */