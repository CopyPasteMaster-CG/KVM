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
 * Module Name: usbdc_vhid.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 *=============================================================================
  Offset Field         Size Value Description 
 ------ ------------- ---- -----------------------------------------------------
 0      bmRequestType   1  Bit-Map D7 Data Phase Transfer Direction
                              0 = Host to Device
                              1 = Device to Host
                           D6..5 Type
                              0 = Standard
                              1 = Class
                              2 = Vendor
                              3 = Reserved
                           D4..0 Recipient
                              0 = Device
                              1 = Interface
                              2 = Endpoint
                              3 = Other
                              4..31 = Reserved
          
 
 */

/* INCLUDE FILE SECTION 							*/
#include <string.h>
#include "project_include.h"

#if (PROJECT_USB_GENERIC_HID_ENABLE)
/* NAMING CONSTANT DECLARATIONS 					*/
code  U8_T USBDC_UPORT[]={BIT0,BIT1,BIT2,BIT3};

/* GLOBAL VARIABLES DECLARATIONS 					*/
U8_T USBDC_HID_ENPx_Data_Wait_Flag[3];
U8_T USBDC_HID_Inttid[3]; //for interface & endpoint 1,2,3
#ifdef VPID_CHANGER
USBHC_DevDesc_TypeDef Ran_Desc[USBDC_PORT_MAX];
#endif
/* LOCAL VARIABLES DECLARATIONS 					*/
/* LOCAL SUBPROGRAM DECLARATIONS 					*/
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
/* EXTERNAL SUBPROGRAM DECLARATIONS 			*/

/*
 * ----------------------------------------------------------------------------
 * Function Name: USBDC_VHid_Init
 * Purpose: Initial the usb virtual generic hid
 * Params:  NONE
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------
 */
void USBDC_VHid_Init(void)
{
	U8_T					pdevinx,port=0;
	USBHC_DevDesc_TypeDef	*devsc;
	
	/*01.Initial Data Read Flag  */
#if (PROJECT_USB_GENERIC_HID_ENABLE)
	USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_KB_ENDP_NUM] = 0x0f;
	USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_MS_ENDP_NUM] = 0x0f;
#endif
	
	/*02.Generate the Device Table                            */
	USB_HAL_Alloc_Free_VDevice(&pdevinx);
	USBDC_HAL_Malloc_Device_Endpx(pdevinx,0,0,USBDC_TABLE_DOUBF_MASK,USBDC_VHID_MAX_ENDP0_SIZE);
	USBDC_HAL_Malloc_Device_Endpx(pdevinx,1,1,USBDC_TABLE_DIR_IN_MASK,USBDC_VHID_MAX_ENDP1_SIZE);
	USBDC_HAL_Malloc_Device_Endpx(pdevinx,2,2,USBDC_TABLE_DIR_IN_MASK,USBDC_VHID_MAX_ENDP2_SIZE);
#if (VHID_INTERFACE_NUM==2)	
	USBDC_Device[USBDC_VHID_DEVINX].EndpNum	= 3;     //Contain 2 endpoint buffers
#endif	

#if (VHID_INTERFACE_NUM==3)	
	USBDC_HAL_Malloc_Device_Endpx(pdevinx,3,3,USBDC_TABLE_DIR_IN_MASK,USBDC_VHID_MAX_ENDP3_SIZE);
	USBDC_Device[USBDC_VHID_DEVINX].EndpNum	= 4;     //Contain 2 endpoint buffers
#endif		
	memset(&USBDC_Device[USBDC_VHID_DEVINX].VirHubNum[0],USBDC_VHID_PORT_NUM+1,USBDC_PORT_MAX);

	//PD table Handle
	USB_HAL_Alloc_Free_PDevice(&pdevinx, 0xFF); //Get a free device table in USB Devcie Table
	//Init PVID/VID of virtual HID device descriptor
	devsc = (USBHC_DevDesc_TypeDef *) &VHID_DeviceDescriptor;
	
	#ifdef VPID_CHANGER
	for (port=0; port < USBDC_PORT_MAX; port++)
	{
		memcpy(&Ran_Desc[port],devsc,sizeof(USBHC_DevDesc_TypeDef));		
	}		
	#endif
#if (SYSTEM_STORAGE_FLASH)
	//From flash
	devsc->idVendor = USBDC_VIRTUAL_HwConfig.vendorID;
	devsc->idProduct = USBDC_VIRTUAL_HwConfig.vHIDProductID;
#endif
	//printf("[Virtual HID] VID = %04x, PID = %04x\n\r",
	//		Big_Endian_16_Convert(devsc->idVendor),
	//		Big_Endian_16_Convert(devsc->idProduct));

	USBDC_VIRTUAL_Create_DeviceDesc(pdevinx,VHID_DeviceDescriptor);
	
#if (VHID_INTERFACE_NUM==3)	
	//HID Report0 Descriptor
	USB_PDevice[pdevinx].Desc[HID0_REPORT].Len = sizeof(VHID_ReportDescriptor);
	USB_PDevice[pdevinx].Desc[HID0_REPORT].Ptr = VHID_ReportDescriptor;
	//HID Report1 Descriptor
	USB_PDevice[pdevinx].Desc[HID1_REPORT].Len = sizeof(VHID_Report2Descriptor);
	USB_PDevice[pdevinx].Desc[HID1_REPORT].Ptr = VHID_Report2Descriptor;
	//HID Report2 Descriptor
	USB_PDevice[pdevinx].Desc[HID2_REPORT].Len = sizeof(VHID_Report3Descriptor);
	USB_PDevice[pdevinx].Desc[HID2_REPORT].Ptr = VHID_Report3Descriptor;
#endif
	
#if (VHID_INTERFACE_NUM==2)	
	//HID Report0 Descriptor
	USB_PDevice[pdevinx].Desc[HID0_REPORT].Len = sizeof(VHID_ReportDescriptor);
	USB_PDevice[pdevinx].Desc[HID0_REPORT].Ptr = VHID_ReportDescriptor;
	//HID Report1 Descriptor
	USB_PDevice[pdevinx].Desc[HID1_REPORT].Len = sizeof(VHID_Report2Descriptor);
	USB_PDevice[pdevinx].Desc[HID1_REPORT].Ptr = VHID_Report2Descriptor;
#endif

#if (VHID_INTERFACE_NUM==1)
	#if (VHID_ENUMERATE_KB)
		USB_PDevice[pdevinx].Desc[HID0_REPORT].Len = sizeof(VHID_ReportDescriptor);
		USB_PDevice[pdevinx].Desc[HID0_REPORT].Ptr = VHID_ReportDescriptor;
	#endif
	
	#if (VHID_ENUMERATE_MS)		
		USB_PDevice[pdevinx].Desc[HID0_REPORT].Len = sizeof(VHID_Report2Descriptor);
		USB_PDevice[pdevinx].Desc[HID0_REPORT].Ptr = VHID_Report2Descriptor;
	#endif
#endif

	//Langid
	USB_PDevice[pdevinx].Desc[LANG_ID].Len = VHID_SIZ_STRING_LANGID;
	USB_PDevice[pdevinx].Desc[LANG_ID].Ptr = VHID_StringLangID;
	//Vendier ID String
	USB_PDevice[pdevinx].Desc[VENDOR_ID].Len = VHID_SIZ_STRING_VENDOR;
	USB_PDevice[pdevinx].Desc[VENDOR_ID].Ptr = VHID_StringVendor;	  
	//Product String
	USB_PDevice[pdevinx].Desc[PRODUCT_ID].Len = VHID_StringProduct[0];
	USB_PDevice[pdevinx].Desc[PRODUCT_ID].Ptr = VHID_StringProduct;	  
	//Serial String
	USB_PDevice[pdevinx].Desc[SERIAL_ID].Len = VHID_StringSerial[0];
	USB_PDevice[pdevinx].Desc[SERIAL_ID].Ptr = VHID_StringSerial;
	
	USBDC_VIRTUAL_Create_ConfigDesc(pdevinx,VHID_ConfigDescriptor);
	
	/* Generate the USB Devcie Attribute */
#if (VHID_INTERFACE_NUM == 1)
	USB_PDevice[pdevinx].Hid_SetIdle_Allow  = 0x01; //only allow keyboard interface to setidle
#endif
#if (VHID_INTERFACE_NUM == 2)
	USB_PDevice[pdevinx].Hid_SetIdle_Allow  = 0x03; //only allow keyboard/mouse interface to setidle
#endif
#if (VHID_INTERFACE_NUM == 3)
	USB_PDevice[pdevinx].Hid_SetIdle_Allow  = 0x07; //only allow keyboard/mouse interface to setidle
#endif

	USB_PDevice[pdevinx].ConfigurationValue = 0x01; // Set current configuration Value	 
	
	(USB_PDevice[USBDC_VHUB_DEVINX].HUB.ReportState+USBDC_VHID_PORT_NUM)->Devinx  = pdevinx;
	//Assing Port device index into root hub port information.
	USBDC_Device[USBDC_VHUB_DEVINX].VHub->PortStatus[0][USBDC_VHID_PORT_NUM].Devinx = USBDC_VHID_DEVINX;
	USBDC_Device[USBDC_VHUB_DEVINX].VHub->PortStatus[1][USBDC_VHID_PORT_NUM].Devinx = USBDC_VHID_DEVINX;
	USBDC_Device[USBDC_VHUB_DEVINX].VHub->PortStatus[2][USBDC_VHID_PORT_NUM].Devinx = USBDC_VHID_DEVINX;
	USBDC_Device[USBDC_VHUB_DEVINX].VHub->PortStatus[3][USBDC_VHID_PORT_NUM].Devinx = USBDC_VHID_DEVINX;
	USBDC_Virtual_Hub_Map[USBDC_VHID_PORT_NUM].HubPort_Devinx = pdevinx;
	USBDC_Virtual_Hub_Map[pdevinx].Devinx_HubPort = USBDC_VHID_PORT_NUM;

#if (SYSTEM_EXTENDER_RECEIVER) || (SYSTEM_EXTENDER_TRANSMITTER)
	#ifdef ONEHID
		USBHC_CORE_Build_INTL_Polling(pdevinx);
	#endif /* #ifdef ONEHID */
#else
	#ifdef SYNC	
		USBHC_CORE_Build_INTL_Polling(pdevinx);
	#endif /* #ifdef SYNC */
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) || (SYSTEM_EXTENDER_TRANSMITTER) */
}

#if (PROJECT_USB_GENERIC_HID_ENABLE)
/*******************************************************************************
* Function Name  : USBDC_VHid_Class_GetReport.
* Description    : for SETUP command only support Set Report,Get 
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
RESULT USBDC_VHid_Class_GetReport(U8_T devinx,U8_T port)
{ 
	RESULT Result=USB_UNSUPPORT;

	if (wValue_H==0x03) //EP0_IN
	{
		if (wIndex_L == 0x01) // interface 1=>Mouse Interface
		{
			USBDC_Device[devinx].Ctrl_TotalByte[port] = 3; // report back 3 byte for standard mouse report
			USBDC_Device[devinx].Ctrl_CurrentByte[port] = 0;
			USBDC_Device[devinx].Control_EndpBuf[port] = &DATAST_Generic_USB_MS_Report[1];
		}
		Result=USB_SUCCESS;
	}
	else if (wValue_H==0x01) //INT_IN
	{
		USBDC_Device[devinx].Ctrl_TotalByte[port] = 0; // report back 3 byte for standard mouse report
		USBDC_Device[devinx].Ctrl_CurrentByte[port] = 0;
		USBDC_Device[devinx].Control_EndpBuf[port] = &DATAST_Generic_USB_MS_Report[1];
		Result=USB_SUCCESS; // return nothing
	}
	
	return Result;
}

/*******************************************************************************
* Function Name  : USBDC_VHid_Resume_Check(U8_T port,U8_T devinx)
* Description    : Will check the HID device is ready for sending ot data
* Input          :  
* Output         : None.
* Return         : None
*******************************************************************************/
FlagStatus USBDC_VHid_Resume_Check(U8_T port,U8_T devinx)
{
	if ((USBDC_Device[devinx].Feature[port] & FEATURE_SUSPEND)== FEATURE_SUSPEND)
	{		
		if ((USBDC_Device[devinx].Feature[port] &FEATURE_REMOTE)== FEATURE_REMOTE)
		{
			USBDC_Port_Resume(port);
		}
		return SET;
	}

	if (USBDC_Device[devinx].Feature[port] & FEATURE_RESUME)
	{
		return SET;
	}

	return RESET;
}

/*******************************************************************************
* RESULT USBDC_VHid_Valid_Check(U8_T port,U8_T devinx)
* Description    : Will check the HID device is ready for sending ot data
* Input          :  
* Output         : None.
* Return         : None
*******************************************************************************/
RESULT USBDC_VHid_Valid_Check(U8_T port,U8_T devinx)
{
	if (USBDC_Device[devinx].DevAddr[port] == 0x00)
	{
		return USB_ERROR;
	}
	
	if ((USBDC_Device[devinx].Feature[port] & (FEATURE_SUSPEND|FEATURE_REMOTE)) == FEATURE_SUSPEND)
	{
		return USB_ERROR;
	}
	
	return USB_SUCCESS;
}

/*******************************************************************************
* void USBDC_VHid_Reset(void)
* Description    :
* Input          :  
* Output         : None.
* Return         : None
*******************************************************************************/
void USBDC_VHid_Reset(void)
{
	DATAST_Reset_GenericQueue(); // this will do the queue reset
}
#endif /* #if (SYSTEM_HARDWARE_PS2_ENABLE) */

#endif /* #if (PROJECT_USB_GENERIC_HID_ENABLE) */

/* End of usbdc_vhid.c */
