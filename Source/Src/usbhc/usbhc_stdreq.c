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
 * Module Name : usbhc_stdreq.c
 * Purpose     : The handler of HC interface standard request 
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 *               Standard Request(bRequest)
 *               [ ].Get Status(0x00)
 *               [ ].Clear Feature(0x01)
 *               [ ].Set Feature(0x03)
 *               [v].Set Address(0x05)               wValueH
 *               [v].Get Descriptor(0x06)----------> [v].Device(0x01)
 *               [ ].Set Descriptor(0x07)            [v].Config(0x02)
 *               [ ].Get Config(0x08)                [v].String(0x03)
 *               [v].Set Config(0x09)                [v].HID(0x021)  
 *               [ ].Get Interface(0x0A)             [v].Report(0x22)
 *               [ ].Set Interface(0x0B)             [ ].Physical(0x23)
 *               [ ].Sync Frame
 *               [ ].Stall
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	<string.h>
#include	<stdio.h>
#include	"project_include.h"


/* STATIC VARIABLE DECLARATIONS */ 
/* LOCAL SUBPROGRAM DECLARATIONS */
/* LOCAL SUBPROGRAM BODIES */


/**
* @brief  USBH_SetAddress
*         This command sets the address to the connected device
* @param  pdev: Selected device
* @param  DeviceAddress: Device address to assign
* @retval Status
*/
USBH_Status USBHC_Stdreq_SetAddress(U8_T devinx,U8_T td_id,U8_T DeviceAddress)
{
	//1.Fill up Host Channel Table
	HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE;
	HCTD_Channel_Table[td_id].Total_Length = 0; // no data stage
	HCTD_Channel_Table[td_id].Current_Length = 0;
	HCTD_Channel_Table[td_id].Endp_Size = USBHC_MAX_PACKET_SIZE;
	
	//2.Fill up Device Table Control Transfer relative field
	USB_PDevice[devinx].Hc.Control.EpN_MaxSize = USBHC_SETUP_PKT_SIZE;
	USB_PDevice[devinx].Hc.Control.Setup.b.bmRequestType = USB_H2D | USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD;
	USB_PDevice[devinx].Hc.Control.Setup.b.bRequest = USB_REQ_SET_ADDRESS;
	USB_PDevice[devinx].Hc.Control.Setup.b.wValue.w = Big_Endian_16_Convert(DeviceAddress);
	USB_PDevice[devinx].Hc.Control.Setup.b.wIndex.w = 0;
	USB_PDevice[devinx].Hc.Control.Setup.b.wLength.w = 0;
	
	return USBHC_CtlReq(devinx,td_id);
}

/**
* @brief  USBH_GetDescriptor
*         Issues Descriptor command to the device. Once the response received,
*         it parses the descriptor and updates the status.
* @param  pdev: Selected device
* @param  req_type: Descriptor type
* @param  value_idx: wValue for the GetDescriptr request
* @param  buff: Buffer to store the descriptor
* @param  length: Length of the descriptor
* @retval Status
*/
USBH_Status USBHC_GetDescriptor(U8_T devinx,U8_T td_id,U8_T req_type,U16_T value_idx,U16_T interface,U16_T length)
{
	U16_T	index;
	
	//2.Setup Token Parameter
	USB_PDevice[devinx].Hc.Control.Setup.b.bmRequestType = USB_D2H | req_type;
	USB_PDevice[devinx].Hc.Control.Setup.b.bRequest = USB_REQ_GET_DESCRIPTOR;
	USB_PDevice[devinx].Hc.Control.Setup.b.wValue.bw.lsb = value_idx & 0x00ff;
	USB_PDevice[devinx].Hc.Control.Setup.b.wValue.bw.msb = (value_idx & 0xff00) >> 8;
	if ((value_idx & 0xff00) == USB_DESC_STRING)
	{
		if ((value_idx & 0x00ff) == 0x00)
			index = 0x00;
		else
			index = 0x0409;
	}
	else
	{
		index = interface;
	}
	USB_PDevice[devinx].Hc.Control.Setup.b.wIndex.bw.lsb = index & 0x00ff;
	USB_PDevice[devinx].Hc.Control.Setup.b.wIndex.bw.msb = (index & 0xff00) >> 8;
	USB_PDevice[devinx].Hc.Control.Setup.b.wLength.bw.lsb = length & 0x00ff;
	USB_PDevice[devinx].Hc.Control.Setup.b.wLength.bw.msb = (length & 0xff00) >> 8;

	return USBHC_CtlReq(devinx,td_id);
}

/**
* @brief  USBHC_Stdreq_GetDevDesc
*         Issue Get Device Descriptor command to the device. Once the response 
*         received, it parses the device descriptor and updates the status.
* @param  pdev: Selected device
* @param  dev_desc: Device Descriptor buffer address
* @param  pdev->host.Rx_Buffer: Receive Buffer address
* @param  length: Length of the descriptor
* @retval Status
*/
USBH_Status USBHC_Stdreq_GetDevDesc(U8_T devinx,U8_T td_id,U16_T length)
{
	//1.Host Channel Paramter
	HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE; //receive the length this time
	HCTD_Channel_Table[td_id].Current_Length = 0; // no data stage
	HCTD_Channel_Table[td_id].Total_Length = length; // data stage yes
	
	return USBHC_GetDescriptor(devinx,td_id,
	USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD,USB_DESC_DEVICE,0,length);
}

/**
* @brief  USBHC_Stdreq_GetCfgDesc
*         Issues Configuration Descriptor to the device. Once the response 
*         received, it parses the configuartion descriptor and updates the 
*         status.
* @param  pdev: Selected device
* @param  cfg_desc: Configuration Descriptor address
* @param  itf_desc: Interface Descriptor address
* @param  ep_desc: Endpoint Descriptor address
* @param  length: Length of the descriptor
* @retval Status
*/
USBH_Status USBHC_Stdreq_GetCfgDesc(U8_T devinx,U8_T td_id,U16_T length)
{
	//1.TD Parameter
	HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE; //receive the length this time
	HCTD_Channel_Table[td_id].Current_Length = 0; // no data tage
	HCTD_Channel_Table[td_id].Total_Length = length; // data stage yes
	
	//2.Descriptor Read
	return USBHC_GetDescriptor(devinx,td_id,
	USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD,USB_DESC_CONFIGURATION,0,length);
}

/**
* @brief  USBHC_Stdreq_SetCfg
*         The command sets the configuration value to the connected device
* @param  pdev: Selected device
* @param  cfg_idx: Configuration value
* @retval Status
*/
USBH_Status USBHC_Stdreq_SetCfg(U8_T devinx,U8_T td_id,U8_T cfg_idx)
{
	//1.Fill up Host Channel Table
	HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE;
	HCTD_Channel_Table[td_id].Total_Length = 0; // no data stage
	HCTD_Channel_Table[td_id].Current_Length = 0;
	HCTD_Channel_Table[td_id].Endp_Size = USBHC_MAX_PACKET_SIZE;
	USB_PDevice[devinx].Hc.Control.Setup.b.bmRequestType = USB_H2D | USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD;
	USB_PDevice[devinx].Hc.Control.Setup.b.bRequest = USB_REQ_SET_CONFIGURATION;
	USB_PDevice[devinx].Hc.Control.Setup.b.wValue.w = Big_Endian_16_Convert((U16_T)cfg_idx);
	USB_PDevice[devinx].Hc.Control.Setup.b.wIndex.w = 0;
	USB_PDevice[devinx].Hc.Control.Setup.b.wLength.w = 0;
	
	return USBHC_CtlReq(devinx,td_id);
}

/**
* @brief  USBHC_Stdreq_SetFeature
*         The command sets the feature to the connected device
* @param  pdev: Selected device
* @param  cfg_idx: Configuration value
* @retval Status
*/
USBH_Status USBHC_Stdreq_SetFeature(U8_T devinx,U8_T td_id,U8_T receipient,U16_T wvalue,U16_T windex)
{
	//1.Fill up Host Channel Table
	HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE;
	HCTD_Channel_Table[td_id].Total_Length = 0; // no data stage
	HCTD_Channel_Table[td_id].Current_Length = 0;
	HCTD_Channel_Table[td_id].Endp_Size = USBHC_MAX_PACKET_SIZE;

	USB_PDevice[devinx].Hc.Control.Setup.b.bmRequestType = USB_H2D | receipient | USB_REQ_TYPE_STANDARD;
	USB_PDevice[devinx].Hc.Control.Setup.b.bRequest = USB_REQ_SET_FEATURE;
	USB_PDevice[devinx].Hc.Control.Setup.b.wValue.w = Big_Endian_16_Convert(wvalue);
	USB_PDevice[devinx].Hc.Control.Setup.b.wIndex.w = Big_Endian_16_Convert(windex);
	USB_PDevice[devinx].Hc.Control.Setup.b.wLength.w = 0;
	
	return USBHC_CtlReq(devinx,td_id);
}

/**
* @brief  USBHC_Stdreq_SetFeature
*         The command sets the feature to the connected device
* @param  pdev: Selected device
* @param  cfg_idx: Configuration value
* @retval Status
*/
//USBH_Status USBHC_Stdreq_ClearFeature(U8_T devinx,U8_T td_id,U8_T receipient,U16_T wvalue,U16_T windex)
//{
	//1.Fill up Host Channel Table
//	HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE;
//	HCTD_Channel_Table[td_id].Total_Length = 0; // no data stage
//	HCTD_Channel_Table[td_id].Current_Length = 0;
//	HCTD_Channel_Table[td_id].Endp_Size = USBHC_MAX_PACKET_SIZE;

//	USB_PDevice[devinx].Hc.Control.Setup.b.bmRequestType = USB_H2D | receipient | USB_REQ_TYPE_STANDARD;
//	USB_PDevice[devinx].Hc.Control.Setup.b.bRequest = USB_REQ_CLEAR_FEATURE;
//	USB_PDevice[devinx].Hc.Control.Setup.b.wValue.w = Big_Endian_16_Convert(wvalue);
//	USB_PDevice[devinx].Hc.Control.Setup.b.wIndex.w = Big_Endian_16_Convert(windex);
//	USB_PDevice[devinx].Hc.Control.Setup.b.wLength.w = 0;
	
//	return USBHC_CtlReq(devinx,td_id);
//}

/**
* @brief  USBH_Get_StringDesc
*         Issues string Descriptor command to the device. Once the response 
*         received, it parses the string descriptor and updates the status.
* @param  pdev: Selected device
* @param  string_index: String index for the descriptor
* @param  buff: Buffer address for the descriptor
* @param  length: Length of the descriptor
* @retval Status
*/
USBH_Status USBHC_Stdreq_GetStringDesc(U8_T devinx,U8_T td_id,U8_T string_idx,U8_T length)
{
	//1.Fill up Host Channel Table
	HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE;
	HCTD_Channel_Table[td_id].Total_Length = length; // no data stage
	HCTD_Channel_Table[td_id].Current_Length = 0;
	
	//2.Descriptor Read
	return USBHC_GetDescriptor(devinx,td_id,
	USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD,USB_DESC_STRING | string_idx,0,length);
}

/**
* @brief  USBHC_Stdreq_SetInterface
*         The command sets the feature to the connected device
* @param  pdev: Selected device
* @param  cfg_idx: Configuration value
* @retval Status
*/
USBH_Status USBHC_Stdreq_SetInterface(U8_T devinx,U8_T td_id,U8_T wvalue,U8_T windex)
{
	//1.Fill up Host Channel Table
	HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE;
	HCTD_Channel_Table[td_id].Total_Length = 0; // no data stage
	HCTD_Channel_Table[td_id].Current_Length = 0;
	HCTD_Channel_Table[td_id].Endp_Size = USBHC_MAX_PACKET_SIZE;
	
	USB_PDevice[devinx].Hc.Control.Setup.b.bmRequestType = USB_H2D | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_STANDARD;
	USB_PDevice[devinx].Hc.Control.Setup.b.bRequest = USB_REQ_SET_INTERFACE;
	USB_PDevice[devinx].Hc.Control.Setup.b.wValue.w = Big_Endian_16_Convert((U16_T)wvalue);
	USB_PDevice[devinx].Hc.Control.Setup.b.wIndex.w = Big_Endian_16_Convert((U16_T)windex);
	USB_PDevice[devinx].Hc.Control.Setup.b.wLength.w = 0;
	
	return USBHC_CtlReq(devinx,td_id);
}

/**
* @brief  USBHC_Stdreq_PatternCommand
* @param  devinx: Selected device
* @param  td_id: 
* @param  setup: setup packet
* @param  length: Length of the setup packet
* @retval Status
*/
USBH_Status USBHC_Stdreq_PatternCommand(U8_T devinx,U8_T td_id,U8_T *setup,U16_T length)
{
	//USBH_Status status;
	//1.Fill up Host Channel Table
	HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE;
	HCTD_Channel_Table[td_id].Total_Length = length; // no data stage
	HCTD_Channel_Table[td_id].Current_Length = 0;
	
	//2.Setup Token Parameter
	memcpy((U8_T *)(&USB_PDevice[devinx].Hc.Control.Setup),setup,8);
	USB_PDevice[devinx].Hc.Control.State = CTRL_SETUP;
	TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_USBHC_Handle_Control_ID,td_id,devinx,1,1); // Generate the Setup Send command
	return USBH_OK;	
}

/* End of usbhc_stdreq.c */

