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
 * Module Name: usbdc_hal.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __USBDC_HAL_H__
#define __USBDC_HAL_H__

/* INCLUDE FILE DECLARATIONS */
/* DEFINATION DECLARATIONS */

#define USBDC_DIR_IN 		0x01
#define USBDC_DIR_OUT 		0x00


/* NAMING CONSTANT DECLARATIONS */
/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
/* EXPORTED GLOBAL VARIABLES */
//void	USBDC_HAL_Get_Devinx_Endpinx(U8_T *devinx,U8_T *endpinx,U8_T insr);
void	USBDC_HAL_Endp_Buf_Ptr(U8_T **bufp,U8_T port,U8_T devinx,U8_T endpinx,U8_T dir);
U16_T	USBDC_HAL_Endp_Out_Buf_Copy(U8_T *buf,U8_T port,U8_T devinx,U8_T endpinx,U8_T isSetup);
void	USBDC_HAL_Endp_In_Buf_Move(U8_T *buf,U8_T len,U8_T port,U8_T devinx,U8_T endpinx);
void	USBDC_HAL_Convert_Setup_Value(U8_T *setup);
//void	USBDC_HAL_Address_Assign(U8_T port,U8_T devinx,U8_T address);
void	USBDC_HAL_Address_Active(U8_T port,U8_T devinx,FlagStatus state);
void	USBDC_HAL_Endp_Stalled(U8_T port,U8_T devinx,U8_T endpinx);
void	USBDC_HAL_Malloc_Device_Endpx(U8_T devinx,U8_T endpinx,U8_T enpdaddr,U8_T endpatri,U16_T maxpackzie);
RESULT	USB_HAL_Alloc_Free_VDevice(U8_T *devinx);
U8_T	USBDC_HAL_Endp_Buf_Length_Read(U8_T port,U8_T devinx,U8_T endpinx,U8_T dir);
void	USBDC_HAL_Endp_Buf_Length_Write(U8_T len,U8_T port,U8_T devinx,U8_T endpinx,U8_T dir);
#endif /* End of __USBDC_MAIN__ */

/* End of usbdc_main.h */