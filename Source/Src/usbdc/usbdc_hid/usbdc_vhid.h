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
 * Module Name: usbdc_vhid.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
 
/* INCLUDE FILE DECLARATIONS */
/* DEFINATION DECLARATIONS */ 
#ifndef __USBDC_VHID_H__
#define __USBDC_VHID_H__

#define USBDC_VHID_MAX_ENDP0_SIZE	8   //Contorl Endp
#define USBDC_VHID_MAX_ENDP1_SIZE	8   //Interrupt Endp1
#define USBDC_VHID_MAX_ENDP2_SIZE	8   //Interrupt Endp2
#define USBDC_VHID_MAX_ENDP3_SIZE	8   //Interrupt Endp2
#define USBDC_VMSC_MAX_ENDP3_SIZE	64  //Interrupt Endp2
#define USBDC_VMSC_MAX_ENDP4_SIZE	64  //Interrupt Endp2

//This is for testing................................
#define VHID_INTERFACE_NUM			3 // only support 1 interface,kb or ms
#if (VHID_INTERFACE_NUM ==1)
	#define VHID_ENUMERATE_KB		1
	#define VHID_ENUMERATE_MOUSE	0
#endif

#if (VHID_INTERFACE_NUM ==2) || (VHID_INTERFACE_NUM ==3)
	#define USBDC_VHID_KB_ENDP_NUM	1   //Interrupt Endp1
	#define USBDC_VHID_MS_ENDP_NUM	2   //Interrupt Endp2
	#define USBDC_VHID_MS2_ENDP_NUM	3   //Interrupt Endp3
#endif

#if (VHID_INTERFACE_NUM ==1)
	#if (VHID_ENUMERATE_KB)
		#define USBDC_VHID_KB_ENDP_NUM	1   //Interrupt Endp1
		#define USBDC_VHID_MS_ENDP_NUM	2   //Interrupt Endp2
	#endif
	
	#if (VHID_ENUMERATE_MOUSE)
		#define USBDC_VHID_KB_ENDP_NUM	2   //Interrupt Endp1
		#define USBDC_VHID_MS_ENDP_NUM	1   //Interrupt Endp2
	#endif
	
#endif
/*----------------------------------------------

/* NAMING CONSTANT DECLARATIONS */
/* GLOBAL VARIABLES */
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
#if (PROJECT_USB_GENERIC_HID_ENABLE)
extern U8_T USBDC_HID_ENPx_Data_Wait_Flag[3];
extern code U8_T USBDC_UPORT[];
#endif

extern U8_T USBDC_HID_Inttid[3];
/* EXPORTED GLOBAL VARIABLES */
void		USBDC_VHid_Init(void);
#if (PROJECT_USB_GENERIC_HID_ENABLE)
RESULT		USBDC_VHid_Valid_Check(U8_T port,U8_T devinx);
FlagStatus	USBDC_VHid_Resume_Check(U8_T port,U8_T devinx);
void		USBDC_VHid_Reset(void);
RESULT		USBDC_VHid_Class_GetReport(U8_T devinx,U8_T port);
#endif /* #if (PROJECT_USB_GENERIC_HID_ENABLE) */

#endif /* End of __USBDC_VHID_H__ */

/* End of usbdc_main.h */