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
 * Module Name: usbdc_vhub_desc.c
 * Purpose: The USB DC USB Pen Drive MSC Class program
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */

/* Includes ------------------------------------------------------------------*/
#include "project_include.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* USB Standard Device Descriptor */

U8_T HUB_DeviceDescriptor[HUB_SIZ_DEVICE_DESC] =
{
	0x12,						/*bLength */
	USB_DEVICE_DESCRIPTOR_TYPE,	/*bDescriptorType*/
	0x01,						/*bcdUSB Low*/
	0x01,						/*bcdUSB High*/
	0x09,						/*bDeviceClass*/
	0x00,						/*bDeviceSubClass*/
	0x00,						/*bDeviceProtocol*/
	0x08,						/*bMaxPacketSize40*/
	0x95,						/*idVendor (0x0b95)*/
	0x0b,
	0x04,						/*idProduct = 0x6804*/ // KVM Autotype
	0x68,
	0x12,						/*bcdDevice rel. 2.00*/
	0x00,
	1,							/*Index of string descriptor describing manufacturer */
	2,							/*Index of string descriptor describing product*/
	3,							/*Index of string descriptor describing the device serial number */
	0x01						/*bNumConfigurations*/
}; /* HUB_DeviceDescriptor */


/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
CODE U8_T HUB_ConfigDescriptor[HUB_SIZ_CONFIG_DESC] =
{
	0x09,								/* bLength: Configuration Descriptor size */
	USB_CONFIGURATION_DESCRIPTOR_TYPE,	/* bDescriptorType: Configuration */
	HUB_SIZ_CONFIG_DESC, 0x00,			/* wTotalLength: Bytes returned */
	0x01,								/* bNumInterfaces: 1 interface */
	0x01,								/* bConfigurationValue: Configuration value */
	0x00,								/* iConfiguration: Index of string descriptor describing
                                           the configuration*/
	0xe0,								/* bmAttributes: Bus powered,Remote Wakeup Support */
	0x32,								/* MaxPower 100 mA: this current is used for detecting Vbus */
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//  Descriptor of Custom HID interface 1 
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	0x09,							/* bLength: Interface Descriptor size */
	USB_INTERFACE_DESCRIPTOR_TYPE,	/* bDescriptorType: Interface descriptor type */
	0x00,							/* bInterfaceNumber: Number of Interface */
	0x00,							/* bAlternateSetting: Alternate setting */
	0x01,							/* bNumEndpoints */
	0x09,							/* bInterfaceClass: HUB */
	0x00,							/* bInterfaceSubClass : 0-Full Speed HUBs */
	0x00,							/* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
	0,								/* iInterface: Index of string descriptor */
	
	/******************** Descriptor of Endpoints ******************/
	0x07,							/* bLength: Endpoint Descriptor size */
	USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType: */
	0x81,							/* bEndpointAddress: Endpoint Address (IN) */
	0x03,							/* bmAttributes: Interrupt endpoint */
	0x01, 0x00,						/* wMaxPacketSize: 1 Bytes max */
	0xff,							/* bInterval: Polling Interval (255 ms) */
};

/* USB String Descriptors (optional) */
CODE U8_T HUB_StringLangID[HUB_SIZ_STRING_LANGID] =
{
	HUB_SIZ_STRING_LANGID,
	USB_STRING_DESCRIPTOR_TYPE,
	0x09,
	0x04
}; /* LangID = 0x0409: U.S. English */

CODE U8_T HUB_StringVendor[HUB_SIZ_STRING_VENDOR] =
{
	HUB_SIZ_STRING_VENDOR,		/* Size of Vendor string */
	USB_STRING_DESCRIPTOR_TYPE,	/* bDescriptorType*/
	
	' ', 0, //1
	'M', 0, //2
	'T', 0, //3
	'K', 0, //4
	' ', 0, //5
	'E', 0, //6
	'l', 0, //7
	'e', 0, //8
	'c', 0, //9
	't', 0, //10
	'r', 0, //11
	'o', 0, //12
	'n', 0, //13
	'i', 0, //14
	'c', 0, //15
	's', 0, //16
};

U8_T XDATA HUB_StringProduct[HUB_SIZ_STRING_PRODUCT] =
{
	42,		/* bLength */
	USB_STRING_DESCRIPTOR_TYPE,	/* bDescriptorType */
	
	'V', 0, //1
	'i', 0, //2
	'r', 0, //3
	't', 0, //4
	'u', 0, //5
	'a', 0, //6
	'l', 0, //7
	' ', 0, //8
	'H', 0, //9
	'U', 0, //10
	'B', 0, //11
	' ', 0, //12
	'V', 0, //13
	'e', 0, //14
	'r', 0, //15
	':', 0, //16
	'0', 0, //17
	'.', 0, //18
	'0', 0, //19
	'1', 0  //20
};
U8_T XDATA HUB_StringSerial[HUB_SIZ_STRING_SERIAL] =
{
	10,		/* bLength */
	USB_STRING_DESCRIPTOR_TYPE,	/* bDescriptorType */
	'M', 0, 'T', 0, 'K', 0, ' ', 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0
};

/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
CODE U8_T HUB_Descriptor[HUB_SIZ_DESCRIPTOR] =
{
	0x09,						// bLength: Configuration Descriptor size 
	USB_HUB_DESCRIPTOR_TYPE,	// bDescriptorType: Configuration 
	USBDC_VHUB_PORT_NUM,		// bNumberOfPorts
	0x10,						// wHubCharacteristics LSB
	0x00,						// wHubCharacteristics MSB
	0x32,						// bPowerOnToPowerGood, unit 2ms 
	0x01,						// bHubContrCurrent  100mA
	0x00,						// DeviceRemovable 
	0xff,						// PortPowerCtrlMask : All bit should be 1.
};

/* End of usbdc_vhub_desc.c */

