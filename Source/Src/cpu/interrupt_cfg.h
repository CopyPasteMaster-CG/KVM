/*
 *********************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : interrupt_cfg.h
 * Purpose     : Configuration setting of interrupt module.
 * Author      : 
 * Date        : 
 * Notes       : 
 *================================================================================
 */
#ifndef INTERRUPT_CFG_H
#define INTERRUPT_CFG_H

/* INCLUDE FILE DECLARATIONS */


/* NAMING CONSTANT DECLARATIONS */
#define		INTR_ISR_INCLUDE_I2C			1
#if (SYSTEM_PS2_HOST_ENABLE)
#define		INTR_ISR_INCLUDE_PS2A			1
#define		INTR_ISR_INCLUDE_PS2B			1
#else
#define		INTR_ISR_INCLUDE_PS2A			0
#define		INTR_ISR_INCLUDE_PS2B			0
#endif //#if (SYSTEM_PS2_HOST_ENABLE)
#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
#define		INTR_ISR_INCLUDE_SPI_MASTER		1
#else
#define		INTR_ISR_INCLUDE_SPI_MASTER		0
#endif //#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
#define		INTR_ISR_INCLUDE_SPI_SLAVE		0
#ifdef PWAYTEK_01
#define		INTR_ISR_INCLUDE_GPIO0			0
#endif

#ifndef INTR_ISR_INCLUDE_GPIO0
#define		INTR_ISR_INCLUDE_GPIO0			1
#endif

#define		INTR_ISR_INCLUDE_GPIO2			0
#define		INTR_ISR_INCLUDE_MSTIMER		1
#define		INTR_ISR_INCLUDE_WATCHDOG		0
#define		INTR_ISR_INCLUDE_BUZZER			0
#if (SYSTEM_MSC_DEVICE_SUPPORT)
 #if (SYSTEM_USB_HC_BURST)
  #define		INTR_ISR_INCLUDE_SWDMA			0	// must be 0
 #else
  #define		INTR_ISR_INCLUDE_SWDMA			1
 #endif	/* #if (SYSTEM_USB_HC_BURST) */
#else
  #define		INTR_ISR_INCLUDE_SWDMA			0	// must be 0
#endif	/* #if (SYSTEM_MSC_DEVICE_SUPPORT) */

#if (SYSTEM_EXTENDER_TRANSMITTER)
 #define INTR_ISR_INCLUDE_USBHC				1  // USB Host   use External interrupt 2
#else
 #define INTR_ISR_INCLUDE_USBHC				1  // USB Host   use External interrupt 2                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               
#endif

#define		INTR_ISR_INCLUDE_USBDC		    1 // USB Device use External interrupt 3

#if (SYSTEM_HUART_SUPPORT)
	#define		INTR_ISR_INCLUDE_HSUR2		    	1
#else
	#if	defined(HSUART_CONSOLE) || defined(HSUART)
		#define		INTR_ISR_INCLUDE_HSUR2		    1
	#else
		#ifdef KMLOG
			#define		INTR_ISR_INCLUDE_HSUR2		1
		#else
			#define		INTR_ISR_INCLUDE_HSUR2		0
		#endif /* #ifdef KM_LOG	*/
	#endif
#endif

/* MACRO DECLARATIONS */


/* TYPE DECLARATIONS */


/* GLOBAL VARIABLES */


/* EXPORTED SUBPROGRAM SPECIFICATIONS */

#endif /* End of INTERRUPT_CFG_H */
