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
 * Module Name: usbdc_cfg.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __USBDC_CFG_H__
#define __USBDC_CFG_H__

/* INCLUDE FILE DECLARATIONS */
/* DEFINATION DECLARATIONS */
//#define  USBDC_IN_NAK					0  //Use IN-Nak for testing

#ifdef MCU_TYPE_AX68002
  #define	USBDC_PORT_MAX				2 //Max Up stream ports counter
#else
  #define	USBDC_PORT_MAX				4 //Max Up stream ports counter
#endif
	
//#ifdef MCU_TYPE_AX68008
//  #define	USBDC_PORT_MAX				8 //Max Up stream ports counter
//#endif
	
//#ifdef MCU_TYPE_AX68016
//  #define	USBDC_PORT_MAX				16 //Max Up stream ports counter
//#endif

#define  USBDC_DEVICE_MAX				8
#define  USBDC_ENDP_MAX					8
#define  USBDC_ISR_FIFO_DEPTH			64
#define  USBDC_VHID_DEFAULT_CONNECT		1

#define  USBDC_DOUBLE_BUF				1
#define  USBDC_SINGLE_BUF				0


#define  USBDC_CONTROL_TRAFFIC_BLOCK	0  /* This option will block the Status Stages Data.*/


#if defined(DEVICE_GPIO_DEFINE)
#define  USB_DYNAMIC_MEMORY_SIZE		(256*22) //5632
#else
#define  USB_DYNAMIC_MEMORY_SIZE		(256*24) //6144
#endif

/* NAMING CONSTANT DECLARATIONS */
/* CONSTANT VARIABLE DECLARATIONS */
/* GLOBAL VARIABLES */
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
/* EXPORTED GLOBAL VARIABLES */
#endif /* End of __USBDC_CFG_H__ */

/* End of usbdc_main.h */