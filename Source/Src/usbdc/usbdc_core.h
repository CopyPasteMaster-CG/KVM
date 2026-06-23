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
 * Module Name: usbdc_core.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __USBDC_CORE_H__
#define __USBDC_CORE_H__

/* INCLUDE FILE DECLARATIONS */
/* DEFINATION DECLARATIONS */
#define IN_BUF_LEN_OFFSET 1
#define OUT_BUF_OFFSET 2

/*----------------------------------------------
$ The Index to Endp Buffer Address Structure
*/
typedef struct
{  
	U8_T Endp_Addr_Lsb;
	U8_T Endp_Addr_Msb;
} USBDC_Index_TypeDef;


typedef struct OneDescriptor
{
	U8_T  *Descriptor;
	U16_T Descriptor_Size;
}
ONE_DESCRIPTOR, *PONE_DESCRIPTOR;


typedef enum _RESULT
{
	USB_SUCCESS = 0,    /* Process successfully */
	USB_ERROR,
	USB_UNSUPPORT,
	USB_WAIT,           /* USB wait for pass through */
	USB_BUSY,           /* Still processing */
	USB_STALLED_WAIT,
	USB_STALLED,
	USB_NOT_READY		/* The process has not been finished, endpoint will be
                         NAK to further request */
} RESULT;


/* NAMING CONSTANT DECLARATIONS */
/* GLOBAL VARIABLES */
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
extern  xdata volatile USBDC_Index_TypeDef		USBDC_Index[USBDC_PORT_MAX][USBDC_DEVICE_MAX][USBDC_ENDP_MAX];
extern  xdata U8_T								*USBDC_EndpBufPtr[USBDC_PORT_MAX][USBDC_DEVICE_MAX][USBDC_ENDP_MAX];
extern  idata volatile U8_T						USBDC_ISRQ_Rp;
extern  idata volatile U8_T						USBDC_ISRQ_Wp;	/*ISR FIFO Pointer*/
extern	U8_T									USBHC_Current_Total_Device_Count;

/* EXPORTED GLOBAL VARIABLES */
void USBDC_CORE_Init(void);
void USBDC_ISR_Handle(void);
void USBDC_Port_Resume(U8_T pid);
void USBDC_CORE_Start(void);
#endif /* End of __USBDC_MAIN__ */

/* End of usbdc_main.h */