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
 * Module Name: usbdc_vhid_desc.c
 * Purpose:
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
#if (PROJECT_USB_GENERIC_HID_ENABLE)

U8_T VHID_DeviceDescriptor[VHID_SIZ_DEVICE_DESC] =
{
	0x12,							/*bLength */
	USB_DEVICE_DESCRIPTOR_TYPE,		/*bDescriptorType*/
	0x10,							/*bcdUSB Low*/
	0x01,							/*bcdUSB High*/
	0x00,							/*bDeviceClass*/
	0x00,							/*bDeviceSubClass*/
	0x00,							/*bDeviceProtocol*/
	0x08,							/*bMaxPacketSize40*/
	0x95, 0x0b,						/*idVendor (0x0b95)*/
	0x06, 0x68,						/*idProduct = 0x6801*/ 
	0x02, 0x00,						/*bcdDevice rel. 2.00*/
	1,								/*Index of string descriptor describing
                                      manufacturer */
	2,								/*Index of string descriptor describing
                                      product*/
	3,								/*Index of string descriptor describing the
                                      device serial number */
	0x01							/*bNumConfigurations*/
}; /* VHID_DeviceDescriptor */


#if (VHID_INTERFACE_NUM == 3)
/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
U8_T VHID_ConfigDescriptor[VHID_SIZ_CONFIG_DESC] =
{
	0x09,								/* bLength: Configuration Descriptor size */
	USB_CONFIGURATION_DESCRIPTOR_TYPE,	/* bDescriptorType: Configuration */
	VHID_SIZ_CONFIG_DESC, 0x00,			/* wTotalLength: Bytes returned */
	0x03,								/* bNumInterfaces: 3 interface */
	0x01,								/* bConfigurationValue: Configuration value */
	0x00,								/* iConfiguration: Index of string descriptor describing
                                           the configuration*/
	0xa0,								/* bmAttributes: Bus powered */
	0x01,								/* MaxPower 2 mA: this current is used for detecting Vbus */
	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//  Descriptor of Custom HID interface 1 
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	0x09,							/* bLength: Interface Descriptor size */
	USB_INTERFACE_DESCRIPTOR_TYPE,	/* bDescriptorType: Interface descriptor type */
	0x00,							/* bInterfaceNumber: Number of Interface */
	0x00,							/* bAlternateSetting: Alternate setting */
	0x01,							/* bNumEndpoints */
	0x03,							/* bInterfaceClass: HID */
	0x01,							/* bInterfaceSubClass : 1=BOOT, 0=no boot */
	0x01,							/* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
	0,								/* iInterface: Index of string descriptor */

	/******************** Descriptor of Custom HID HID ********************/
	0x09,							/* bLength: HID Descriptor size */
	HID_DESCRIPTOR_TYPE,			/* bDescriptorType: HID */
	0x10, 0x01,						/* bcdHID: HID Class Spec release number */
	0x00,							/* bCountryCode: Hardware target country */
	0x01,							/* bNumDescriptors: Number of HID class descriptors to follow */
	0x22,							/* bDescriptorType */
	VHID_SIZ_REPORT_DESC, 0x00,		/* wItemLength: Total length of Report descriptor */
	
	/******************** Descriptor of Custom HID endpoints ******************/
	0x07,							/* bLength: Endpoint Descriptor size */
	USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType: */
	0x81,							/* bEndpointAddress: Endpoint Address (IN) */
	0x03,							/* bmAttributes: Interrupt endpoint */
	0x08, 0x00,						/* wMaxPacketSize: 8 Bytes max */
	0x01,							/* bInterval: Polling Interval (1 ms) */
	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//  Descriptor of Custom HID interface 2 
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	
	0x09,							/* bLength: Interface Descriptor size */
	USB_INTERFACE_DESCRIPTOR_TYPE,	/* bDescriptorType: Interface descriptor type */
	0x01,							/* bInterfaceNumber: Number of Interface */
	0x00,							/* bAlternateSetting: Alternate setting */
	0x01,							/* bNumEndpoints */
	0x03,							/* bInterfaceClass: HID */
	0x01,							/* bInterfaceSubClass : 1=BOOT, 0=no boot */
	0x02,							/* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
	0x00,							/* iInterface: Index of string descriptor */
	
	/******************** Descriptor of Custom HID HID ********************/
	0x09,							/* bLength: HID Descriptor size */
	HID_DESCRIPTOR_TYPE,			/* bDescriptorType: HID */
	0x10, 0x01,						/* bcdHID: HID Class Spec release number */
	0x00,							/* bCountryCode: Hardware target country */
	0x01,							/* bNumDescriptors: Number of HID class descriptors to follow */
	0x22,							/* bDescriptorType */
	VHID_SIZ_REPORT2_DESC, 0x00,	/* wItemLength: Total length of Report descriptor */
	
	/******************** Descriptor of Custom HID endpoints ******************/
	0x07,							/* bLength: Endpoint Descriptor size */
	USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType: */
	0x82,							/* bEndpointAddress: Endpoint Address (IN) 2*/
	0x03,							/* bmAttributes: Interrupt endpoint */
	0x07, 0x00,						/* wMaxPacketSize: 7 Bytes max */
	0x01,							/* bInterval: Polling Interval (10 ms) */
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//  Descriptor of Custom HID interface 3 
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	
	0x09,							/* bLength: Interface Descriptor size */
	USB_INTERFACE_DESCRIPTOR_TYPE,	/* bDescriptorType: Interface descriptor type */
	0x02,							/* bInterfaceNumber: Number of Interface */
	0x00,							/* bAlternateSetting: Alternate setting */
	0x01,							/* bNumEndpoints */
	0x03,							/* bInterfaceClass: HID */
	0x01,							/* bInterfaceSubClass : 1=BOOT, 0=no boot */
	0x02,							/* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
	0x00,							/* iInterface: Index of string descriptor */
	
	/******************** Descriptor of Custom HID HID ********************/
	0x09,							/* bLength: HID Descriptor size */
	HID_DESCRIPTOR_TYPE,			/* bDescriptorType: HID */
	0x10, 0x01,						/* bcdHID: HID Class Spec release number */
	0x00,							/* bCountryCode: Hardware target country */
	0x01,							/* bNumDescriptors: Number of HID class descriptors to follow */
	0x22,							/* bDescriptorType */
	VHID_SIZ_REPORT3_DESC, 0x00,	/* wItemLength: Total length of Report descriptor */

	/******************** Descriptor of Custom HID endpoints ******************/
	0x07,							/* bLength: Endpoint Descriptor size */
	USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType: */
	0x83,							/* bEndpointAddress: Endpoint Address (IN) 2*/
	0x03,							/* bmAttributes: Interrupt endpoint */
	0x07, 0x00,						/* wMaxPacketSize: 7 Bytes max */
	0x01,							/* bInterval: Polling Interval (10 ms) */
} ; /* VHID_ConfigDescriptor */
#endif // (VHID_INTERFACE_NUM == 3)

#if (VHID_INTERFACE_NUM == 2)
/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
U8_T VHID_ConfigDescriptor[VHID_SIZ_CONFIG_DESC] =
{
	0x09,								/* bLength: Configuration Descriptor size */
	USB_CONFIGURATION_DESCRIPTOR_TYPE,	/* bDescriptorType: Configuration */
	VHID_SIZ_CONFIG_DESC, 0x00,			/* wTotalLength: Bytes returned */
	0x023,								/* bNumInterfaces: 3 interface */
	0x01,								/* bConfigurationValue: Configuration value */
	0x00,								/* iConfiguration: Index of string descriptor describing
                                           the configuration*/
	0xa0,								/* bmAttributes: Bus powered */
	0x01,								/* MaxPower 2 mA: this current is used for detecting Vbus */
	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//  Descriptor of Custom HID interface 1 
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	0x09,							/* bLength: Interface Descriptor size */
	USB_INTERFACE_DESCRIPTOR_TYPE,	/* bDescriptorType: Interface descriptor type */
	0x00,							/* bInterfaceNumber: Number of Interface */
	0x00,							/* bAlternateSetting: Alternate setting */
	0x01,							/* bNumEndpoints */
	0x03,							/* bInterfaceClass: HID */
	0x01,							/* bInterfaceSubClass : 1=BOOT, 0=no boot */
	0x01,							/* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
	0,								/* iInterface: Index of string descriptor */
	
	/******************** Descriptor of Custom HID HID ********************/
	0x09,							/* bLength: HID Descriptor size */
	HID_DESCRIPTOR_TYPE,			/* bDescriptorType: HID */
	0x10, 0x01,						/* bcdHID: HID Class Spec release number */
	0x00,							/* bCountryCode: Hardware target country */
	0x01,							/* bNumDescriptors: Number of HID class descriptors to follow */
	0x22,							/* bDescriptorType */
	VHID_SIZ_REPORT_DESC, 0x00,		/* wItemLength: Total length of Report descriptor */
	
	/******************** Descriptor of Custom HID endpoints ******************/
	0x07,							/* bLength: Endpoint Descriptor size */
	USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType: */
	0x81,							/* bEndpointAddress: Endpoint Address (IN) */
	0x03,							/* bmAttributes: Interrupt endpoint */
	0x08, 0x00,						/* wMaxPacketSize: 8 Bytes max */
	0x01,							/* bInterval: Polling Interval (1 ms) */
	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//  Descriptor of Custom HID interface 2 
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	
	0x09,							/* bLength: Interface Descriptor size */
	USB_INTERFACE_DESCRIPTOR_TYPE,	/* bDescriptorType: Interface descriptor type */
	0x01,							/* bInterfaceNumber: Number of Interface */
	0x00,							/* bAlternateSetting: Alternate setting */
	0x01,							/* bNumEndpoints */
	0x03,							/* bInterfaceClass: HID */
	0x01,							/* bInterfaceSubClass : 1=BOOT, 0=no boot */
	0x02,							/* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
	0x00,							/* iInterface: Index of string descriptor */
	
	/******************** Descriptor of Custom HID HID ********************/
	0x09,							/* bLength: HID Descriptor size */
	HID_DESCRIPTOR_TYPE,			/* bDescriptorType: HID */
	0x10, 0x01,						/* bcdHID: HID Class Spec release number */
	0x00,							/* bCountryCode: Hardware target country */
	0x01,							/* bNumDescriptors: Number of HID class descriptors to follow */
	0x22,							/* bDescriptorType */
	VHID_SIZ_REPORT2_DESC, 0x00,	/* wItemLength: Total length of Report descriptor */
	
	/******************** Descriptor of Custom HID endpoints ******************/
	0x07,							/* bLength: Endpoint Descriptor size */
	USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType: */
	0x82,							/* bEndpointAddress: Endpoint Address (IN) 2*/
	0x03,							/* bmAttributes: Interrupt endpoint */
	0x07, 0x00,						/* wMaxPacketSize: 7 Bytes max */
	0x01,							/* bInterval: Polling Interval (10 ms) */
} ; /* VHID_ConfigDescriptor */
#endif // (VHID_INTERFACE_NUM == 2)

#if (VHID_INTERFACE_NUM == 1)
/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
U8_T VHID_ConfigDescriptor[VHID_SIZ_CONFIG_DESC] =
{
	0x09,								/* bLength: Configuration Descriptor size */
	USB_CONFIGURATION_DESCRIPTOR_TYPE,	/* bDescriptorType: Configuration */
	VHID_SIZ_CONFIG_DESC, 0x00,			/* wTotalLength: Bytes returned */
	0x01,								/* bNumInterfaces: 2 interface */
	0x01,								/* bConfigurationValue: Configuration value */
	0x00,								/* iConfiguration: Index of string descriptor describing
                                           the configuration*/
	0xA0,								/* bmAttributes: Bus powered */
	0x32,								/* MaxPower 100 mA: this current is used for detecting Vbus */
	
#if (VHID_ENUMERATE_KB)
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//  Descriptor of Custom HID interface 1 
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	0x09,							/* bLength: Interface Descriptor size */
	USB_INTERFACE_DESCRIPTOR_TYPE,	/* bDescriptorType: Interface descriptor type */
	0x00,							/* bInterfaceNumber: Number of Interface */
	0x00,							/* bAlternateSetting: Alternate setting */
	0x01,							/* bNumEndpoints */
	0x03,							/* bInterfaceClass: HID */
	0x01,							/* bInterfaceSubClass : 1=BOOT, 0=no boot */
	0x01,							/* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
	0,								/* iInterface: Index of string descriptor */
	/******************** Descriptor of Custom HID HID ********************/
	0x09,							/* bLength: HID Descriptor size */
	HID_DESCRIPTOR_TYPE,			/* bDescriptorType: HID */
	0x10, 0x01,						/* bcdHID: HID Class Spec release number */
	0x00,							/* bCountryCode: Hardware target country */
	0x01,							/* bNumDescriptors: Number of HID class descriptors to follow */
	0x22,							/* bDescriptorType */
	VHID_SIZ_REPORT_DESC, 0x00,		/* wItemLength: Total length of Report descriptor */
		
	/******************** Descriptor of Custom HID endpoints ******************/
	0x07,							/* bLength: Endpoint Descriptor size */
	USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType: */

	0x81,							/* bEndpointAddress: Endpoint Address (IN) */
	0x03,							/* bmAttributes: Interrupt endpoint */
	0x08, 0x00,						/* wMaxPacketSize: 16 Bytes max */
	0x03,							/* bInterval: Polling Interval (10 ms) */
#endif

#if (VHID_ENUMERATE_MOUSE)
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//  Descriptor of Custom HID interface 2 
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~     
	0x09,							/* bLength: Interface Descriptor size */
	USB_INTERFACE_DESCRIPTOR_TYPE,	/* bDescriptorType: Interface descriptor type */
	0x00,							/* bInterfaceNumber: Number of Interface */
	0x00,							/* bAlternateSetting: Alternate setting */
	0x01,							/* bNumEndpoints */
	0x03,							/* bInterfaceClass: HID */
	0x01,							/* bInterfaceSubClass : 1=BOOT, 0=no boot */
	0x02,							/* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
	0x00,							/* iInterface: Index of string descriptor */
	
	/******************** Descriptor of Custom HID HID ********************/
	0x09,							/* bLength: HID Descriptor size */
	HID_DESCRIPTOR_TYPE,			/* bDescriptorType: HID */
	0x10, 0x01,						/* bcdHID: HID Class Spec release number */
	0x00,							/* bCountryCode: Hardware target country */
	0x01,							/* bNumDescriptors: Number of HID class descriptors to follow */
	0x22,							/* bDescriptorType */
	VHID_SIZ_REPORT2_DESC,0x00,		/* wItemLength: Total length of Report descriptor */
	
	/******************** Descriptor of Custom HID endpoints ******************/
	0x07,							/* bLength: Endpoint Descriptor size */
	USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType: */
	0x81,							/* bEndpointAddress: Endpoint Address (IN) 2*/
	0x03,							/* bmAttributes: Interrupt endpoint */
	0x08, 0x00,						/* wMaxPacketSize: 16 Bytes max */	
	0x03,							/* bInterval: Polling Interval (10 ms) */
#endif
} ; /* VHID_ConfigDescriptor */
#endif // (VHID_INTERFACE_NUM == 1)
   
U8_T VHID_ReportDescriptor[] =
{
	0x05, 0x01,			//; usage page (generic desktop)
	0x09, 0x06,			//; usage (keyboard)
	0xA1, 0x01,			//; collection (application)
	0x05, 0x07,			//; usage page( key codes)
	0x19, 0xE0,			//; usage minimum (224)
	0x29, 0xE7,			//; usage maximum (231)
	0x15, 0x00,			//; logical minimum (0)
	0x25, 0x01,			//; logical maximum (1)
	0x75, 0x01,			//; report size (1 bit)
	0x95, 0x08,			//; report count (8 bytes) // 10
	0x81, 0x02,			//; input (data, variable, absolute)
	
	0x95, 0x01,			//; report count (1 byte)
	0x75, 0x08,			//; report size (8 bits)
	0x81, 0x01,			//; input (CODEant)
	
	0x95, 0x05,			//; report count (5)
	0x75, 0x01,			//; report size (1)
	0x05, 0x08,			//; usage page (LEDs)
	0x19, 0x01,			//; usage minimum (1)
	0x29, 0x05,			//; usage maximum (5)
	0x91, 0x02,			//; output (data, variable, absolute) //20
	
	0x95, 0x01,			//; report count (1)
	0x75, 0x03,			//; report size (3)
	0x91, 0x01,			//; output (CODEant)
	
	0x95, 0x06,			//; report count (6)
	0x75, 0x08,			//; report size (8)
	0x15, 0x00,			//; logical minimum (0)
	0x25, 0x68,			//; logical maximum (101)
	0x05, 0x07,			//; usage page (key codes)
	0x19, 0x00,			//; usage minimum (0)
	0x29, 0x68,			//; usage maximum (101) //30
	0x81, 0x00,			//; input (data, array)
	0xC0				//; end collection
}; /* VHID_ReportDescriptor */

U8_T VHID_Report2Descriptor[] =
{
	0x05, 0x01,			//; usage page (generic desktop)
	0x09, 0x02,			//; usage (mouse)
	0xA1, 0x01,			//; collection (application)
	
	0x85, 0x01,			//; report id (1)
	0x09, 0x01,			//; usage (pointer)
	0xA1, 0x01,			//; collection (linked)
	0x05, 0x09,			//; usage page (buttons)
	0x19, 0x01,			//; usage minimum (1)
	0x29, 0x05,			//; 3usage maximum (3)
	0x15, 0x00,			//; logical minimum (0) //10
	0x25, 0x01,			//; logical maximum (1)
	0x75, 0x01,			//; report size (1)
	0x95, 0x05,			//; 3 report count (3 bytes)		
	0x81, 0x02,			//; input (3 button bits)
	
	0x75, 0x03,			//; 5 report size (5)
	0x95, 0x01,			//; report count (1)	
	0x81, 0x01,			//; input (CODEant 5 bit padding)
	
	0x05, 0x01,			//; usage page (generic desktop)
	0x09, 0x30,			//; usage (X)
	0x09, 0x31,			//; usage (Y) //20
	0x09, 0x38,			//; usage (wheel)
	0x15, 0x81,			//; logical minimum (-127)
	0x25, 0x7F,			//; logical maximum (127)
	0x75, 0x08,			//; report size (8)
	0x95, 0x03,			//; report count (3)
	0x81, 0x06,			//; input (3 position bytes X & Y & wheel)
	0xC0, 0xC0,			//; end collection, end collection

	0x05, 0x01,			//; USAGE_PAGE (Generic Desktop)
	0x09, 0x80,			//; USAGE (System Control)
	0xa1, 0x01,			//; COLLECTION (Application) //30
	0x85, 0x02,			//; report id (2)
	0x05, 0x01,			//; USAGE_PAGE (Generic Desktop)
	0x19, 0x81,			//; USAGE_MINIMUM (System Power Down)
	0x29, 0x83,			//; USAGE_MAXIMUM (System Wake Up)
	0x15, 0x00,			//; LOGICAL_MINIMUM (0)
	0x25, 0x01,			//; LOGICAL_MAXIMUM (1)
	0x95, 0x03,			//; REPORT_COUNT (3)
	0x75, 0x01,			//; REPORT_SIZE (1)
	0x81, 0x06,			//; INPUT (Data,Var,rel)
	0x95, 0x01,			//; report count (1 byte) //40
	0x75, 0x05,			//; report size (5 bits)
	0x81, 0x01,			//; input (CODEant)
	0xc0,
	
	0x05, 0x0C,			/*usage page (Multimedia KB) */
	0x09, 0x01,			/* Usage (Pointer)  =======  Hut1.1 Table 5*/
	0xa1, 0x01,			/*collection (application) */
	0x85, 0x03,			/* ???  Interface#2 ????*/
	0x19, 0x00,			/*Usage minimum (0x00) */
	0x2a, 0x3c, 0x02,	/*Usage minimum (0x023c) */
	0x15, 0x00,			/*logical minimum (0) */
	0x26, 0x3c, 0x02,	/*logical maximum (0x023c) */
	0x75, 0x10,			/*report size (16)*/
	0x95, 0x01,			/*report count (1) */
	0x81, 0x00,			/*input (Data, Array) */
	0xC0,				/*end collection*/
}; /* VHID_ReportDescriptor */
	
U8_T VHID_Report3Descriptor[] =
{		
	//Below is for mouse pass through mode control
	0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
	0x09, 0x02,        // Usage (Mouse)
	0xA1, 0x01,        // Collection (Application)
	0x85, 0x04,        //   Report ID (4)
	0x09, 0x01,        //   Usage (Pointer)
	0xA1, 0x00,        //   Collection (Physical)
	0x05, 0x09,        //     Usage Page (Button)
	0x19, 0x01,        //     Usage Minimum (0x01)
	0x29, 0x05,        //     Usage Maximum (0x05)
	0x15, 0x00,        //     Logical Minimum (0)
	0x25, 0x01,        //     Logical Maximum (1)
	0x75, 0x01,        //     Report Size (1)
	0x95, 0x05,        //     Report Count (5)
	0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x75, 0x01,        //     Report Size (1)
	0x95, 0x03,        //     Report Count (3)
	0x81, 0x01,        //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x75, 0x08,        //     Report Size (8)
	0x95, 0x01,        //     Report Count (1)
	0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
	0x09, 0x38,        //     Usage (Wheel)
	0x15, 0x81,        //     Logical Minimum (-127)
	0x25, 0x7F,        //     Logical Maximum (127)
	0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
	0xC0,              //   End Collection
	0x09, 0x01,        //   Usage (Pointer)
	0xA1, 0x00,        //   Collection (Physical)
	0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
	0x09, 0x30,        //     Usage (X)
	0x15, 0x00,        //     Logical Minimum (0)
	//0x26, 0x00, 0x0f,  //     Logical Maximum (3840)
	0x26,VHID_X_MAX_LSB,VHID_X_MAX_MSB, //     Logical Maximum (????)
	0x35, 0x00,        //     Physical Minimum (0)
	//0x46, 0x00, 0x0f,  //     Physical Maximum (3840)
	0x46, VHID_X_MAX_LSB,VHID_X_MAX_MSB,  //     Physical Maximum (????)
	0x75, 0x10,        //     Report Size (16)
	0x95, 0x01,        //     Report Count (1)
	0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x09, 0x31,        //     Usage (Y)
	0x15, 0x00,        //     Logical Minimum (0)
	//0x26, 0x70, 0x08,  //     Logical Maximum (2160)
	0x26, VHID_Y_MAX_LSB,VHID_Y_MAX_MSB,  //     Logical Maximum (????)
	0x35, 0x00,        //     Physical Minimum (0)
	0x46, VHID_Y_MAX_LSB,VHID_Y_MAX_MSB,  //     Physical Maximum (????)
	//0x46, 0x70, 0x08,  //     Physical Maximum (2160)
	0x75, 0x10,        //     Report Size (16)
	0x95, 0x01,        //     Report Count (1)
	0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0xC0, 0xC0,        //     End Collection
	
	/*
	//Top level collection for use in Single-Touch Digitizer device mode.
	//-------------------------------------------------------------------
	//Format for single-touch digitizer input reports using this report descriptor:
	//byte[0] = Report ID = SINGLE_TOUCH_DATA_REPORT_ID
	//byte[1] = contains bit fields for various input information typically generated by an input pen or human finger. '1' is the active value (ex: pressed), '0' is the non active value
	//		bit0 = Tip switch. At the end of a pen input device would normally be a pressure senstive switch.  Asserting this performs an operation analogous to a "left click" on a mouse
	//		bit2 = In range indicator.  
	//		bit3 though bit 7 = Pad bits.  Values not used for anything.
	//byte[2] = Pad byte.  Value not used for anything.
	//byte[3] = X coordinate LSB value of contact point
	//byte[4] = X coordinate MSB value of contact point
	//byte[5] = Y coordinate LSB value of contact point
	//byte[6] = Y coordinate MSB value of contact point
    0x05, 0x0D,             // USAGE_PAGE (Digitizers)          
    0x09, 0x02,             // USAGE (Pen)                      
    0xA1, 0x01,             // COLLECTION (Application)         
    0x85, SINGLE_TOUCH_DATA_REPORT_ID, 	//   REPORT_ID (Pen digitizer/single touch)  //To send a HID input report containing single-touch digitizer data, use the SINGLE_TOUCH_DATA_REPORT_ID report ID
    0x09, 0x20,             //   USAGE (Stylus)                 
    0xA1, 0x00,             //   COLLECTION (Physical)          
    0x09, 0x42, 			//     USAGE (Tip Switch)           //(byte[1] bit 0)
    0x09, 0x32, 			//     USAGE (In Range)             //(byte[1] bit 1)
    0x15, 0x00,             //     LOGICAL_MINIMUM (0)          
    0x25, 0x01,             //     LOGICAL_MAXIMUM (1)          
    0x75, 0x01,             //     REPORT_SIZE (1)              //1-bit		
    0x95, 0x02,             //     REPORT_COUNT (2)             //two
    0x81, 0x02, 			//     INPUT (Data,Var,Abs)         //Makes two, 1-bit IN packet fields (byte[1] bits 0-4)) for (USAGE) tip sw, barrel sw, invert sw, in range sw.  Send '1' here when switch is active.  Send '0' when switch not active.
    0x95, 0x0E, 			//     REPORT_COUNT (14)            //fourteen
    0x81, 0x03, 			//     INPUT (Cnst,Var,Abs)         //Makes fourteen, 1-bit IN packet fields (byte[1] bits 5-7, and byte[2] all bits) with no usage.  These are pad bits that don't contain useful data.
    0x05, 0x01,             //     USAGE_PAGE (Generic Desktop)
    0x26, 0xC0, 0x12,     //       LOGICAL_MAXIMUM (4800)         
    0x75, 0x10,           //       REPORT_SIZE (16)             
    0x95, 0x01,             //     REPORT_COUNT (1)   			//one
    0x55, 0x0E,           //       UNIT_EXPONENT (-2)       //10^(-2)           
    0x65, 0x33,           //       UNIT (Inches, English Linear)  //But exponent -2, so Physical Maximum is in 10’s of mils.                 
    0x09, 0x30,           //       USAGE (X)                    
    0x35, 0x00,           //       PHYSICAL_MINIMUM (0)         
    0x46, 0x40, 0x06,     //       PHYSICAL_MAXIMUM (0x640 = 1600)     //1600 * 10^(-2) = 16 inches X-dimension        
    0x81, 0x02,           //       INPUT (Data,Var,Abs)           //Makes one, 16-bit field for X coordinate info.  Valid values from: 0-4800    
    0x26, 0xB8, 0x0B,     //       LOGICAL_MAXIMUM (3000)             //16:10 aspect ratio (X:Y) 
    0x46, 0xE8, 0x03,     //       PHYSICAL_MAXIMUM (0x3E8 = 1000)    //1000 * 10^(-2) = 10 inches Y-dimension       
    0x09, 0x31,           //       USAGE (Y)       
    0x81, 0x02,             //     INPUT (Data,Var,Abs)		    //Makes one, 16-bit field for Y coordinate info.  Valid values from: 0-3000   
    0xC0,                   //   END_COLLECTION (Physical)                
    0xC0                    // END_COLLECTION (Application) 
	*/
}; /* VHID_ReportDescriptor */


/* USB String Descriptors (optional) */
U8_T VHID_StringLangID[VHID_SIZ_STRING_LANGID] =
{
	VHID_SIZ_STRING_LANGID,
	USB_STRING_DESCRIPTOR_TYPE,
	0x09,
	0x04
}; /* LangID = 0x0409: U.S. English */

U8_T VHID_StringVendor[VHID_SIZ_STRING_VENDOR] =
{
	VHID_SIZ_STRING_VENDOR, /* Size of Vendor string */
	USB_STRING_DESCRIPTOR_TYPE,  /* bDescriptorType*/
	/* Manufacturer: "ASIX Electronics" */
	'A', 0, //1
	'S', 0, //2
	'I', 0, //3
	'X', 0, //4
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

U8_T XDATA VHID_StringProduct[VHID_SIZ_STRING_PRODUCT] =
{
	42,          /* bLength */
	USB_STRING_DESCRIPTOR_TYPE,      /* bDescriptorType */
	'V', 0, //1
	'i', 0, //2
	'r', 0, //3
	't', 0, //4
	'u', 0, //5
	'a', 0, //6
	'l', 0, //7
	' ', 0, //8
	'H', 0, //9
	'I', 0, //10
	'D', 0, //11
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

U8_T XDATA VHID_StringSerial[VHID_SIZ_STRING_SERIAL] =
{
	10,           /* bLength */
	USB_STRING_DESCRIPTOR_TYPE,      /* bDescriptorType */
	'A', 0, 'S', 0, 'I', 0, 'X', 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0
};

#endif  /* #if (PROJECT_USB_GENERIC_HID_ENABLE) */

/* End of usbdc_vhid_desc.c */

