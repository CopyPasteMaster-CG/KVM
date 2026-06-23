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
 * Module Name: usbdc_vendor.c
 * Purpose: The USB DC Vendor Command Control & Managment program
 * Author:
 * Date:
 * Notes:
 *============================================================================= 
 Offset Field         Size Value Description 
 ------ ------------- ---- -----------------------------------------------------
 Value  => LSB=Command,MSB=Paramter
 	0x01 => Report
 	     MSB--------------------------------------
 		 0x01=> Boot Loader Version Number Report
 		 0x02=> KVM Firmware System Version Number Report
 		 0x03=> Ative Port Report
 		 0x04=> Host Status Report => INDEX=PortID
 	
 	0x02 => System Control
 	 	 MSB--------------------------------------
 	 	 0x01=> System Configuration Table Read
 	 	 0x02=> AutoScan Start
 	 	 0x03=> AutoScan Stop
 		 0x04=> Autoscan Inverval Setting
 		 0x05=> Beeper Control => INDEX=ON/OFF
 		 0x06=> Beeper Sound   => INDEX=Counter
 		 0x07=> Pen Drive Enable => INDEX=Port Number
 		 0x08=> Pen Drive Disable
 		 0x09=> MSC Devcie Enable => INDEX=Port Number
 		 0x0a=> MSC Devcie Disable
 		 0x0b=> Audio+Port Combo Switch => INDEX=ON/OFF
 		 .
 		 .
 		 .
 		 .
 		 .
 		 
 	0x03 => Switch Control
 	     MSB--------------------------------------
 		 0x01=> KVM Active Port Switch	
 		 0x02=> Audio Switch	
 		 0x03=> MSC Switch
 		 0x04=> PEN Drive Switch		
 		 
 		 
 	0x04 => Register Read
 		 MSB--------------------------------------
 		 0x01=> Register Address
 	
 	0x05 => Register Write	 	
 	     MSB--------------------------------------
 		 0x01=> Register Address   
 
 */
/*
 
*/
/* INCLUDE FILE SECTION 							*/
#include	<string.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<absacc.h>
#include	"project_include.h"

#if (SYSTEM_VENDOR_COMMAND_SUPPORT)
/* NAMING CONSTANT DECLARATIONS 					*/
/* GLOBAL VARIABLES DECLARATIONS 					*/
U8_T  KVM_Vendor_State[10];
/* LOCAL VARIABLES DECLARATIONS 					*/
/* LOCAL SUBPROGRAM DECLARATIONS 					*/ 
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS 			*/
/* EXTERNAL SUBPROGRAM DECLARATIONS 				*/

/*******************************************************************************
* Function Name  : USBDC_Vendor_Command_Handle.
* Description    : Handle the Vendor Command
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
RESULT USBDC_Vendor_Command_Handle(U8_T devinx,U8_T port)
{
#ifdef RuntimeCodeAt12KH
	U8_T indx;
#endif
	RESULT result=USB_UNSUPPORT;
	
	switch(wValue_L) //command
	{
		case VENDOR_REPORT:
			switch(wValue_H)
			{
#ifdef RuntimeCodeAt12KH
				case REPROT_BOOTLOADER_VERSION:
					for (indx=0; indx < 10; indx++)
						KVM_Vendor_State[indx] = FCVAR(U8_T, 0x2000 + indx);

					USBDC_Device[devinx].Ctrl_TotalByte[port]  = 10; //only answer one byte 
					USBDC_Device[devinx].Control_EndpBuf[port] = KVM_Vendor_State; //Get a memory to store data
					USBDC_Device[devinx].Ctrl_CurrentByte[port] = 0;
					result=USB_SUCCESS;
					break;
#endif
				case REPROT_SYSTEM_VERSION:
					break;
				case REPROT_ACTIVE_PORT:
					USBDC_Device[devinx].Ctrl_TotalByte[port]  = 4; //only answer one byte 
					USBDC_Device[devinx].Control_EndpBuf[port] = KVM_Vendor_State; //Get a memory to store data
					USBDC_Device[devinx].Control_EndpBuf[port][0]= KVM_CurrentHost;
					#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
					USBDC_Device[devinx].Control_EndpBuf[port][1]= KVM_CurrentAudio;
					#else
					USBDC_Device[devinx].Control_EndpBuf[port][1]= 0xff; //not support
					#endif
					
					#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
					USBDC_Device[devinx].Control_EndpBuf[port][2]=  KVM_CurrentUSBAudio;
					#else
					USBDC_Device[devinx].Control_EndpBuf[port][2]= 0xff; //not support
					#endif
					
					#if (SYSTEM_MSC_DEVICE_SUPPORT)
					USBDC_Device[devinx].Control_EndpBuf[port][3]= KVM_CurrentMSC;
					#else
					USBDC_Device[devinx].Control_EndpBuf[port][3]= 0xff;
					#endif
					
					#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
					USBDC_Device[devinx].Control_EndpBuf[port][4]= KVM_CurrentPEN;
					#else
					USBDC_Device[devinx].Control_EndpBuf[port][4]= 0xff;
					#endif
					
					USBDC_Device[devinx].Ctrl_CurrentByte[port] = 0;
					result=USB_SUCCESS;
					break;
				case REPROT_HOST_STATE:
					break;			
			}
			break;
		case VENDOR_SYSTEM_CONTROL:
			break;
		case VENDOR_SWITCH_CONTROL:
			break;
		case VENDOR_REGISTER_READ:
			break;			
		case VENDOR_REGISTER_WRITE:
			break;			
	}
	return result;
}
#endif /* End  of SYSTEM_VENDOR_COMMAND_SUPPORT */ 

/* End of usbdc_vendor.c */
