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
 * Module Name: usbhc_def.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */


#ifndef USBHC_DEF_H
#define USBHC_DEF_H

#ifndef USBHC_NULL
#define USBHC_NULL							((void *)0)
#endif

#ifndef FALSE
#define FALSE								0
#endif

#ifndef TRUE
#define TRUE								1
#endif

#define _ValBit(VAR,POS)					(VAR & (1 << POS))
#define SetBit(VAR,POS)						(VAR |= (1 << POS))
#define ClrBit(VAR,POS)						(VAR &= ((1 << POS)^255))
#define LE16(addr)							(((u16)(*((u8 *)(addr)))) + (((u16)(*(((u8 *)(addr)) + 1))) << 8))

#define USB_LEN_DESC_HDR					0x02
#define USB_LEN_DEV_DESC					0x12
#define USB_LEN_CFG_DESC					0x09
#define USB_LEN_IF_DESC						0x09
#define USB_LEN_EP_DESC						0x07
#define USB_LEN_OTG_DESC					0x03
#define USB_LEN_SETUP_PKT					0x08
/* bmRequestType :D7 Data Phase Transfer Direction */
#define USB_REQ_DIR_MASK					0x80
#define USB_H2D								0x00
#define USB_D2H								0x80
/* bmRequestType D6..5 Type */
#define USB_REQ_TYPE_STANDARD				0x00
#define USB_REQ_TYPE_CLASS					0x20
#define USB_REQ_TYPE_VENDOR					0x40
#define USB_REQ_TYPE_RESERVED				0x60
/* bmRequestType D4..0 Recipient */
#define USB_REQ_RECIPIENT_DEVICE			0x00
#define USB_REQ_RECIPIENT_INTERFACE			0x01
#define USB_REQ_RECIPIENT_ENDPOINT			0x02
#define USB_REQ_RECIPIENT_OTHER				0x03
/* Table 9-4. Standard Request Codes  */
/* bRequest , Value */
#define USB_REQ_GET_STATUS					0x00
#define USB_REQ_CLEAR_FEATURE				0x01
#define USB_REQ_SET_FEATURE					0x03
#define USB_REQ_SET_ADDRESS					0x05
#define USB_REQ_GET_DESCRIPTOR				0x06
#define USB_REQ_SET_DESCRIPTOR				0x07
#define USB_REQ_GET_CONFIGURATION			0x08
#define USB_REQ_SET_CONFIGURATION			0x09
#define USB_REQ_GET_INTERFACE				0x0A
#define USB_REQ_SET_INTERFACE				0x0B
#define USB_REQ_SYNCH_FRAME					0x0C
/* Table 9-5. Descriptor Types of USB Specifications */
#define USB_DESC_TYPE_DEVICE						1
#define USB_DESC_TYPE_CONFIGURATION					2
#define USB_DESC_TYPE_STRING						3
#define USB_DESC_TYPE_INTERFACE						4
#define USB_DESC_TYPE_ENDPOINT						5
#define USB_DESC_TYPE_DEVICE_QUALIFIER				6
#define USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION		7
#define USB_DESC_TYPE_INTERFACE_POWER				8
#define USB_DESC_TYPE_HID_DESCRIPTOR				0x21
#define USB_DESC_TYPE_HID_REPORT					0x22
#define USB_DEVICE_DESC_SIZE						18
#define USB_CONFIGURATION_DESC_SIZE					9
#define USB_HID_DESC_SIZE							9
#define USB_INTERFACE_DESC_SIZE						9
#define USB_ENDPOINT_DESC_SIZE						7
/* Descriptor Type and Descriptor Index */
/* Use the following values when calling the function USBHC_GetDescriptor */
#define USB_DESC_DEVICE						((USB_DESC_TYPE_DEVICE << 8) & 0xFF00)
#define USB_DESC_CONFIGURATION				((USB_DESC_TYPE_CONFIGURATION << 8) & 0xFF00)
#define USB_DESC_STRING						((USB_DESC_TYPE_STRING << 8) & 0xFF00)
#define USB_DESC_INTERFACE					((USB_DESC_TYPE_INTERFACE << 8) & 0xFF00)
#define USB_DESC_ENDPOINT 					((USB_DESC_TYPE_INTERFACE << 8) & 0xFF00)
#define USB_DESC_DEVICE_QUALIFIER			((USB_DESC_TYPE_DEVICE_QUALIFIER << 8) & 0xFF00)
#define USB_DESC_OTHER_SPEED_CONFIGURATION	((USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION << 8) & 0xFF00)
#define USB_DESC_INTERFACE_POWER			((USB_DESC_TYPE_INTERFACE_POWER << 8) & 0xFF00)
#define USB_DESC_HID_REPORT					((USB_DESC_TYPE_HID_REPORT << 8) & 0xFF00)
#define USB_DESC_HID						((USB_DESC_TYPE_HID << 8) & 0xFF00)
#define USB_EP_TYPE_MASK					0x60
#define USB_EP_TYPE_CTRL					0x00
#define USB_EP_TYPE_ISOC					0x20
#define USB_EP_TYPE_BULK					0x40
#define USB_EP_TYPE_INTR					0x60
#define USB_EP_TD_MASK						0xf8 //0~31 td number
#define USB_EP_DIR_OUT						0x00
#define USB_EP_DIR_IN						0x80
#define USB_EP_DIR_MSK						0x80
/* Interface Descriptor field values for HID Boot Protocol */
#define HID_BOOT_CODE						0x01
#define HID_KEYBRD_BOOT_CODE				0x01
#define HID_MOUSE_BOOT_CODE					0x02
/* As per USB specs 9.2.6.4 :Standard request with data request timeout: 5sec
   Standard request with no data stage timeout : 50ms */
#define DATA_STAGE_TIMEOUT					5000
#define NODATA_STAGE_TIMEOUT				100
/** @defgroup USBH_HID_CORE_Exported_Defines
  * @{
  */ 
#define USB_HID_REQ_GET_REPORT				0x01
#define USB_HID_GET_IDLE					0x02
#define USB_HID_GET_PROTOCOL				0x03
#define USB_HID_SET_REPORT					0x09
#define USB_HID_SET_IDLE					0x0A
#define USB_HID_SET_PROTOCOL				0x0B

#define USBHC_CONFIGURATION_DESCRIPTOR_SIZE	(USB_CONFIGURATION_DESC_SIZE\
											+ USB_INTERFACE_DESC_SIZE\
											+ (USBHC_MAX_NUM_ENDPOINTS * USB_ENDPOINT_DESC_SIZE))

#define CONFIG_DESC_wTOTAL_LENGTH			(ConfigurationDescriptorData.ConfigDescfield.\
											ConfigurationDescriptor.wTotalLength)

typedef struct _DeviceDescriptor
{
	U8_T	bLength;
	U8_T	bDescriptorType;
	U16_T	bcdUSB;			/* USB Specification Number which device complies too */
	U8_T	bDeviceClass;	/* If equal to Zero, each interface specifies its own class
							code if equal to 0xFF, the class code is vendor specified.
							Otherwise field is valid Class Code.*/
	U8_T	bDeviceSubClass;
	U8_T	bDeviceProtocol;
	U8_T	bMaxPacketSize;
	U16_T	idVendor;			/* Vendor ID (Assigned by USB Org) */
	U16_T	idProduct;			/* Product ID (Assigned by Manufacturer) */
	U16_T	bcdDevice;			/* Device Release Number */
	U8_T	iManufacturer;		/* Index of Manufacturer String Descriptor */
	U8_T	iProduct;			/* Index of Product String Descriptor */
	U8_T	iSerialNumber;		/* Index of Serial Number String Descriptor */
	U8_T	bNumConfigurations;	/* Number of Possible Configurations */
} USBHC_DevDesc_TypeDef;

typedef struct _ConfigurationDescriptor
{
	U8_T	bLength;
	U8_T	bDescriptorType;
	U16_T	wTotalLength;			/* Total Length of Data Returned */
	U8_T	bNumInterfaces;			/* Number of Interfaces */
	U8_T	bConfigurationValue;	/* Value to use as an argument to select this configuration */
	U8_T	iConfiguration;			/* Index of String Descriptor Describing this configuration */
	U8_T	bmAttributes;			/* D7 Bus Powered , D6 Self Powered, D5 Remote Wakeup , D4..0 Reserved (0) */
	U8_T	bMaxPower;				/* Maximum Power Consumption */
} USBHC_CfgDesc_TypeDef;

typedef struct _HIDDescriptor
{
	U8_T	bLength;
	U8_T	bDescriptorType;
	U16_T	bcdHID;					/* indicates what endpoint this descriptor is describing */
	U8_T	bCountryCode;			/* specifies the transfer type. */
	U8_T	bNumDescriptors;		/* specifies the transfer type. */
	U8_T	bReportDescriptorType;	/* Maximum Packet Size this endpoint is capable of sending or receiving */
	U16_T	wItemLength;			/* is used to specify the polling interval of certain transfers. */
} USBHC_HIDDesc_TypeDef;

typedef struct _InterfaceDescriptor
{
	U8_T	bLength;
	U8_T	bDescriptorType;
	U8_T	bInterfaceNumber;
	U8_T	bAlternateSetting;		/* Value used to select alternative setting */
	U8_T	bNumEndpoints;			/* Number of Endpoints used for this interface */
	U8_T	bInterfaceClass;		/* Class Code (Assigned by USB Org) */
	U8_T	bInterfaceSubClass;		/* Subclass Code (Assigned by USB Org) */
	U8_T	bInterfaceProtocol;		/* Protocol Code */
	U8_T	iInterface;				/* Index of String Descriptor Describing this interface */
} USBHC_InterfaceDesc_TypeDef;

typedef struct _EndpointDescriptor
{
	U8_T	bLength;
	U8_T	bDescriptorType;
	U8_T	bEndpointAddress;	/* indicates what endpoint this descriptor is describing */
	U8_T	bmAttributes;		/* specifies the transfer type. */
	U16_T	wMaxPacketSize;		/* Maximum Packet Size this endpoint is capable of sending or receiving */
	U8_T	bInterval;			/* is used to specify the polling interval of certain transfers. */
} USBHC_EpDesc_TypeDef;

#endif
