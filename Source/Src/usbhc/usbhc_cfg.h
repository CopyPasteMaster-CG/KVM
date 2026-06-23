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
 * Module Name : usbhc_cfg.h
 * Purpose     : Configuration setting of USB device interface
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */
#ifndef USB_HC_CFG_H
#define USB_HC_CFG_H
// USB HC SRF define
//sfr USBHC_INTERRUPT_STATUS	= 0xc4;
//sfr USBHC_OINTERRUPT_STATUS	= 0xc5;
#define USBHC_INTERRUPT_STATUS	HCIS

#define USBHC_VECTOR						2
#ifndef LED_ON
#define LED_ON								0
#define LED_OFF								1
#endif

/* INCLUDE FILE DECLARATIONS */
/* Hardware Relative Define */
#ifdef MCU_TYPE_AX68002
#define	USB_HC_MAX_PORT						2 //Max Down stream ports counter
#else
#define	USB_HC_MAX_PORT						4 //Max Down stream ports counter
#endif

//#if (SYSTEM_AUDIO_IN_SUPPORT)
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
#define USB_HC_ISTL_OUT_MAX				1 //Support 2=>ISTL0,ISTL1(for Speaker & Micphone)
#define USB_HC_ISTL_OUT_DONE_MASK		0x01
#define USB_HC_ISTL_IN_DONE_MASK		(0x01 << USB_HC_ISTL_OUT_MAX)
#define USB_HC_ISTL_IN_MAX				1 //Support 2=>ISTL0,ISTL1(for Speaker & Micphone)
#define USB_HC_ISTL_MAX					(USB_HC_ISTL_OUT_MAX+USB_HC_ISTL_IN_MAX) //Support 2=>ISTL0,ISTL1(for Speaker & Micphone)
#else
#define USB_HC_ISTL_OUT_MAX				1 //Support 2=>ISTL0,ISTL1(for Speaker & Micphone)
#define USB_HC_ISTL_OUT_DONE_MASK		0x01
#define USB_HC_ISTL_IN_DONE_MASK		(0x01 << USB_HC_ISTL_OUT_MAX)
#define USB_HC_ISTL_IN_MAX				1 //Support 2=>ISTL0,ISTL1(for Speaker & Micphone)
#define USB_HC_ISTL_MAX					(USB_HC_ISTL_OUT_MAX+USB_HC_ISTL_IN_MAX) //Support 2=>ISTL0,ISTL1(for Speaker & Micphone)
#endif //#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)

#if (SYSTEM_MSC_DEVICE_SUPPORT)
 #if (SYSTEM_USB_HC_BURST)
  #define USB_HC_INTL_MAX						14 //Support 14 INTL buffer
  #define	USB_HC_CONTROL_MAX					8  //Support 8 ATL
  #define	USB_HC_BULK_MAX						USBHC_MSC_TD_SIZ //Support 4 BULK Burst TD,2 for IN, 2 for OUT
  #define	USB_HC_ATL_MAX						(USB_HC_CONTROL_MAX+USB_HC_BULK_MAX) //Support 8 ATL+? BULK
 #else
  #define USB_HC_INTL_MAX						16 //Support 16 INTL buffer
  #define	USB_HC_ATL_MAX						(8+USBHC_MSC_TD_SIZ) //Support 8 ATL+? BULK
 #endif // #if (SYSTEM_USB_HC_BURST)
#else
  #define	USB_HC_BULK_MAX						0
  #define	USB_HC_ATL_MAX						8	//Support 8 ATL
  #define USB_HC_INTL_MAX						16	//Support 16 INTL buffer
#endif	/* #if (SYSTEM_MSC_DEVICE_SUPPORT) */

#define USB_HC_TD_MAX						(USB_HC_ISTL_MAX+USB_HC_INTL_MAX+USB_HC_ATL_MAX) //Support 16 INTL buffer
#define USB_HC_MAX_DEVICE					8 //Support total 8 device connectted
#define USB_HC_MAX_HUB						4 //Support total 8 device connectted
#define USB_HC_MAX_ISR_FIFO_DEPTH			64 //Support 64 ISR FiFO layers
#define DONE_QUEUE_MAX						64
#if (SYSTEM_MSC_DEVICE_SUPPORT)
#define BULK_DONE_QUEUE_MAX					32
#endif	/* End of #if (SYSTEM_MSC_DEVICE_SUPPORT) */
#define ATL_DONE_QUEUE_MAX					32
// The TD look up start address
#define USB_HC_ISTL_START					0
#define USB_HC_INTL_START					USB_HC_ISTL_MAX
#define USB_HC_ATL_START					(USB_HC_ISTL_MAX+USB_HC_INTL_MAX)
#define USB_HC_INTL_BUF_SIZE				64 //Interrupt block size(should be 64)
#if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST))
#define USB_HC_SINGLE_BULK_BUF_SIZE			64 //Control & Bulk block size
#endif
#define USB_HC_INTL_TOTAL_BUF_SIZE			((USB_HC_INTL_BUF_SIZE+8)*USB_HC_INTL_MAX) //Interrupt block size
#define USB_HC_INTL_BUF_SIZE_MSB			((USB_HC_INTL_TOTAL_BUF_SIZE) >> 8) //Interrupt buffer size define
#define USB_HC_INTL_BUF_SIZE_LSB			((USB_HC_INTL_TOTAL_BUF_SIZE) & 0x00ff) //Interrupt buffer size define
#define USB_HC_INTL_BLK_SIZE_MSB			((USB_HC_INTL_BUF_SIZE & 0x0300) >> 8) //Interrupt block size define
#define USB_HC_INTL_BLK_SIZE_LSB			((USB_HC_INTL_BUF_SIZE) & 0xff) //Interrupt block size define

#if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST))
	#if (SYSTEM_EXTENDER_RS232_MODE)
		#define USB_HC_BULK_BUF_SIZE				130 //first 2 bytes reserved, 512 byte for burst page
		#define USB_HC_BURST_PAGE_SIZE				128 //burst page size define
	#else //For RS-485 Mode
		#if (SYSTEM_EXTENDER_SUPPORT)
			#define USB_HC_BULK_BUF_SIZE			(USB_MSC_CLASS_PAGE_SIZ+2) //first 2 bytes reserved, 384 byte for burst page
			#define USB_HC_BURST_PAGE_SIZE			USB_MSC_CLASS_PAGE_SIZ //burst page size define
		#endif
		
		#ifdef KVM			
			#define USB_HC_BULK_BUF_SIZE			514 //first 2 bytes reserved, 384 byte for burst page
			#define USB_HC_BURST_PAGE_SIZE			512 //burst page size define
		#endif /* #ifdef KVM  */
		
	#endif //#if (SYSTEM_EXTENDER_RS232_MODE)
	
	#define USB_HC_BULK_TOTAL_BUF_SIZE			((USB_HC_BULK_BUF_SIZE+8)* USB_HC_BULK_MAX) //BULK block size
	#define USB_HC_BULK_BLK_SIZE_MSB			(((USB_HC_BULK_BUF_SIZE-2) & 0x0300) >> 8) // Bulk block size define
	#define USB_HC_BULK_BLK_SIZE_LSB			((USB_HC_BULK_BUF_SIZE-2) & 0x00ff) //Bulk block size define
#else
	#define USB_HC_BULK_BUF_SIZE				0
	#define USB_HC_BURST_PAGE_SIZE				0
	#define USB_HC_BULK_TOTAL_BUF_SIZE			0
#endif //#if (SYSTEM_MSC_DEVICE_SUPPORT)


#define USB_HC_ATL_BUF_SIZE					64 //Control & Bulk block size
#if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST))
#define USB_HC_ATL_TOTAL_BUF_SIZE			(((USB_HC_ATL_BUF_SIZE+8)*USB_HC_ATL_MAX)+(USB_HC_BULK_TOTAL_BUF_SIZE)) //Control block size
#else
#define USB_HC_ATL_TOTAL_BUF_SIZE			((USB_HC_ATL_BUF_SIZE+8)*USB_HC_ATL_MAX) //Control block size
#endif
#define USB_HC_ATL_BUF_SIZE_MSB				((USB_HC_ATL_TOTAL_BUF_SIZE) >> 8) //Control & Bulk buffer size define
#define USB_HC_ATL_BUF_SIZE_LSB				((USB_HC_ATL_TOTAL_BUF_SIZE) & 0x00ff) //Control & Bulk buffer size define
#define USB_HC_ATL_BLK_SIZE_MSB				((USB_HC_ATL_BUF_SIZE & 0x0300) >> 8) //Control & Bulk block size define
#define USB_HC_ATL_BLK_SIZE_LSB				((USB_HC_ATL_BUF_SIZE) & 0xff) //Control & Bulk block size define
#define USB_HC_ISTL_BUF_SIZE				194 //Isochronous block size define
#define USB_HC_ISTL_TOTAL_BUF_SIZE			((USB_HC_ISTL_BUF_SIZE+8)*USB_HC_ISTL_MAX) //ISTL Buffer size(should be 1032)
#define USB_HC_ISTL_BUF_SIZE_MSB			((USB_HC_ISTL_TOTAL_BUF_SIZE) >> 8) //Isochronous block size define
#define USB_HC_ISTL_BUF_SIZE_LSB			((USB_HC_ISTL_TOTAL_BUF_SIZE) & 0x00ff) //Isochronous block size define
#define USB_HC_ISTL_BLK_SIZE_MSB			(((USB_HC_ISTL_BUF_SIZE-2) & 0x0300) >> 8) //Isochronous block size define
#define USB_HC_ISTL_BLK_SIZE_LSB			((USB_HC_ISTL_BUF_SIZE-2) & 0xff) //Isochronous block size define


/* MACRO DECLARATIONS */


/* TYPE DECLARATIONS */


/* GLOBAL VARIABLES */


/* EXPORTED SUBPROGRAM SPECIFICATIONS */


#endif /* End of USB_HC_CFG_H */
