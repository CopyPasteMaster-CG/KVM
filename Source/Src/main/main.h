/*
*******************************************************************************
 *  Copyright (c) 2014	ASIX Electronic Corporation    All rights reserved.
 *
 *  This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *  The copyright notice above does not evidence any actual or intended
 *  publication of such source code.
 ******************************************************************************
 */

/*=============================================================================
 * Module Name: main.h
 * Purpose	  : 
 * Author     : 
 * Date       :
 * Notes      : 
 * History    : 
 *                                                                                                                      
 *=============================================================================
*/
#ifndef _MAIN_H
#define _MAIN_H

/* INCLUDE FILE DECLARATIONS */

/* NAMING CONSTANT DECLARATIONS */
#define XDATA_BASE_ADDR		0x10000
#if (SYSTEM_EXTENDER_TRANSMITTER)
#define ISR_FIFO_DEPTH		128
#else
#define ISR_FIFO_DEPTH		32
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */
#define ISR_SWDMA_DEPTH		64

#ifdef DEVICE_GPIO_DEFINE
#define DEVICE_AUDIO_GPIO_PIN	P3_0
#define DEVICE_MSC_GPIO_PIN		P3_1

#define DEVCIE_AUDIO_SUPPORT	1
#define DEVCIE_NO_AUDIO_SUPPORT	0
#define DEVCIE_MSC_SUPPORT		1
#define DEVCIE_NO_MSC_SUPPORT	0
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ISR Event Define
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define ISR_BTN				0x01
#define ISR_EDID			0x02
#define ISR_I2C				0x03
#define ISR_SPI				0x04
#define ISR_HUART			0x05
#define ISR_HUART_RBDMA		0x06 /*Receiver ring buffer event */
#define ISR_BULK_OUT  		0x07
#define ISR_BULK_IN   		0x08 
#define ISR_IR				0x09
#define ISR_UART0			0x0A
#define ISR_UART1			0x0B

#define USBDC_ISR_BULK		0x10
#define USBDC_ISR_BULK_OUT	0x11
#define USBDC_ISR_BULK_IN 	0x12
#define USBDC_ISR_ISO_OUT	0x13
#define USBDC_ISR_ISO_IN	0x14

#define USBHC_ISR_ISO_OUT	0x20
#define USBHC_ISR_ISO_IN	0x21
#define USBHC_ISR_BULK		0x22

#define SWDMA_BULK_OUT		0x30
#define SWDMA_BULK_IN		0x31



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ISR State Define
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define ISR_STATE_SKIP      0x01 
typedef struct _ISR_TypeDef
{
	U8_T ISR_Type;	// ISR_TYPE
	U8_T State;
	U8_T Data;		// receive data Q
} ISR_TypeDef;


typedef union //Byte order=Small Endian
{
	U16_T w;

	struct BW
	{
		U8_T lsb;
		U8_T msb;
	} bw;
} u16_t_u8_t;

typedef union //Byte order=Big Endian
{
	U16_T w;

	struct BBW
	{		
		U8_T msb;
		U8_T lsb;
	} bbw;
} U16_T_U8_T;

typedef struct _SWDMA_TypeDef
{
	U8_T  lsb;
	U8_T  msb;
} SWDMA_TypeDef;
/* MACRO Definition */
/* TYPE DECLARATIONS */
/* GLOBAL VARIABLES */
extern ISR_TypeDef ISR_FIFO[ISR_FIFO_DEPTH];
extern U8_T ISR_FIFO_Rp,ISR_FIFO_Wp;
extern ISR_TypeDef ISR_FIFO[ISR_FIFO_DEPTH];
extern bit	Audio_Device_Support_Gpio;
extern bit	MSC_Device_Support_Gpio;
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void ISR_Init(void);
void malloc_free(void xdata *buf);
void xdata *m_malloc(U16_T len,U8_T id);
void	Disp_Str(U8_T *p,U16_T len);
#if (SYSTEM_TASK_DUMP_SUPPORT)
void Main_System_Halt(void);
#endif


#endif /* End of _MAIN_H */

