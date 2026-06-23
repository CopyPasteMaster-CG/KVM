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
 * Module Name: usbdc_virtual.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __USBDC_VIRTUAL_H__
#define __USBDC_VIRTUAL_H__

#define USBDC_VHUB_DEVINX		0	//Device index number of virtual HUB
#define USBDC_VHUB_ENDPINX		1	//Endpoint index number of virtual HUB

#if (PROJECT_USB_GENERIC_HID_ENABLE)
	#define USBDC_VHID_DEVINX			1   //Device index number of virtual HID
 #if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
	#define USBDC_VMSC_DEVINX			2   //Device index number of virtual MSC
  #ifdef KVM_EXTENDER_TRANSMITTER
	#define USBDC_VREMOTE_HUB_DEVINX	3   //Device start index number of user device
	#define USBDC_VIRTUAL_DEVINX		3   //Device start index number of user device
  #else
	#define USBDC_VIRTUAL_DEVINX		3   //Device start index number of user device
  #endif	
 #else
  #ifdef KVM_EXTENDER_TRANSMITTER
	#define USBDC_VREMOTE_HUB_DEVINX	2   //Device start index number of user device
	#define USBDC_VIRTUAL_DEVINX		2   //Device start index number of user device
  #else
	#define USBDC_VIRTUAL_DEVINX		2   //Device start index number of user device
  #endif
 #endif
#else
 #if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
	#define USBDC_VMSC_DEVINX			1   //Device Index number of virtual Generic HID
    #ifdef KVM_EXTENDER_TRANSMITTER
		#define USBDC_VREMOTE_HUB_DEVINX	2   //Device Index number of virtual Generic HID
		#define USBDC_VHID_DEVINX			2   //Device Index number of virtual Generic HID
		#define USBDC_VIRTUAL_DEVINX		3   //Device Index number of virtual Generic HID
  	#else
		#define USBDC_VHID_DEVINX			2   //Device Index number of virtual Generic HID
		#define USBDC_VIRTUAL_DEVINX		2   //Device Index number of virtual Generic HID
	 #endif
 #else		
	#ifdef KVM_EXTENDER_TRANSMITTER
		#define USBDC_VREMOTE_HUB_DEVINX	0   //Device Index number of virtual Remote Hub
		#define USBDC_VIRTUAL_DEVINX		1   //Device Index number of virtual Generic HID
		#define USBDC_VHID_DEVINX			0   //Device Index number of virtual Generic HID
	 #else
		#define USBDC_VIRTUAL_DEVINX		1   //Device Index number of virtual Generic HID
		#define USBDC_VHID_DEVINX			1   //Device Index number of virtual Generic HID
	#endif
 #endif
#endif

/* INCLUDE FILE DECLARATIONS */
/* DEFINATION DECLARATIONS */
 
typedef  struct _GetDesc 
{
	U8_T  bDescriptorType;
	U8_T  bLength;
	U8_T  *bDescriptor;
} GetDesc_TypeDef;

typedef struct _HWConfigData
{
	/* Notice: The size of this data structure must be multiple of 8 */
	U8_T	hwCfgFlag;
	U8_T	hwCfgMultiFun[3];
	U8_T	hwCfgUsbDisable;
	U8_T	hwCfgDbgPort;
	U8_T	hwCfgReserved[10];
	U8_T	idNumber[4];
	U8_T	reserved[4];
	U16_T	vendorID;
	U16_T	bldrHIdProductID;
	U16_T	vHIDProductID;
	U16_T	vHUBProductID;
	U8_T	manuString[64];
	U8_T	serialNum[32];
	U8_T	bldrHIDProductString[64];
	U8_T	vHIDProductString[64];
	U8_T	vHUBProductString[64];
	U8_T	iapGpioSetting;
	U8_T	iapLedGpioSetting;
	U8_T	reserved2[6];
	
} HWConfigData;

/*----------------------------------------------



/* NAMING CONSTANT DECLARATIONS */
/* GLOBAL VARIABLES */
extern HWConfigData	USBDC_VIRTUAL_HwConfig;
extern U8_T			USBDC_VIRTUAL_SerialByPort;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Setup Up Token Parser Usage
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
extern U8_T		Request_No;
extern U8_T		Type_Recipient;
extern U8_T		wValue_H;
extern U8_T		wValue_L;
extern U8_T		wIndex_H;
extern U8_T		wIndex_L;
extern U8_T		TASK_USBDC_VIRTUAL_PassThrough_Setup_Wait_ID;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */

/* EXPORTED GLOBAL VARIABLES */
void	USBDC_VIRTUAL_Setup_DataStageIn(U8_T devinx,U8_T port,U8_T handletype);
void	USBDC_VIRTUAL_Setup_IN_Process(U8_T port,U8_T devinx);
void	USBDC_VIRTUAL_Setup_OUT_Process(U8_T port,U8_T devinx);
void	USBDC_VIRTUAL_Init(void);
void	USBDC_VIRTUAL_Port_Clear(U8_T pid,U8_T start_devinx,U8_T end_devinx);

void	USBDC_VIRTUAL_Active_Device(U8_T devinx,U8_T active_port);
void	USBDC_VIRTUAL_Port_Status_Control(U8_T pid,U8_T flag,FlagStatus state);
void	USBDC_VIRTUAL_Create_DeviceDesc(U8_T devid,U8_T *device_desc);
void	USBDC_VIRTUAL_Create_ConfigDesc(U8_T devid,U8_T *config_desc);
void	USBDC_VIRTUAL_Control_Statue_In(U8_T devinx,U8_T port);
void	USBDC_VIRTUAL_Fill_Device_Reg(U8_T devinx,U8_T active_port,U8_T address);

RESULT	USBDC_Standard_GetStatus		 (U8_T devinx,U8_T port);
RESULT	USBDC_Standard_GetDeviceDescriptor(U8_T devinx,U8_T port);
RESULT	USBDC_Standard_SetConfiguration(U8_T devinx,U8_T port);
RESULT	USBDC_Standard_GetInterface(U8_T devinx,U8_T port);
RESULT	USBDC_Standard_SetAddress(U8_T devinx,U8_T port);

void	USBDC_VIRTUAL_Setup_Token(U8_T devinx,U8_T port);
//void	USBDC_VIRTUAL_Setup_DataStageIn(U8_T devinx,U8_T port,U8_T handletype);

RESULT	USBDC_Standard_GetDescData_Post(U8_T devinx,U8_T port,U8_T descid);
void	USBDC_VIRTUAL_Setup_Process(U8_T devinx,U8_T port);
void	USBDC_VIRTUAL_Endp_Stall_Control(U8_T port,U8_T devinx,U8_T endpinx,FlagStatus state);
void	USBDC_VIRTUAL_INNAK_Control(U8_T devinx,U8_T port,U8_T endpinx,U8_T value);
void	USBDC_VIRTUAL_SetCheck_Interval(U8_T port,U8_T devinx,U8_T endpinx);
#endif /* End of __USBDC_VIRTUAL_H__ */

/* End of usbdc_virtual.h */