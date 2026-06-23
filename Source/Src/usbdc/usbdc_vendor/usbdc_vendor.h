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
 * Module Name: usbdc_vendor.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
 
/* INCLUDE FILE DECLARATIONS */
/* DEFINATION DECLARATIONS */ 
#ifndef __USBDC_VENDOR_H__
#define __USBDC_VENDOR_H__

#define VENDOR_REPORT  							0x01
		#define REPROT_BOOTLOADER_VERSION		0x01
		#define REPROT_SYSTEM_VERSION			0x02
		#define REPROT_ACTIVE_PORT				0x03
		#define REPROT_HOST_STATE				0x04
		
#define VENDOR_SYSTEM_CONTROL					0x02
#define VENDOR_SWITCH_CONTROL					0x03
#define VENDOR_REGISTER_READ					0x04
#define VENDOR_REGISTER_WRITE					0x05

/*External Subfunction Declartion */
RESULT USBDC_Vendor_Command_Handle(U8_T devinx,U8_T port);
#endif /* End of __USBDC_VENDOR_H__ */

/* End of usbdc_vendor.h */