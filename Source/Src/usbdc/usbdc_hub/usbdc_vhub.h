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
 * Module Name: usbdc_vhub.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __USBDC_VHUB_H__
#define __USBDC_VHUB_H__

#define USBDC_VHUB_MAX_ENDP0_SIZE	8	//5 Port Virtual HUB
#define USBDC_VHUB_MAX_ENDP1_SIZE	1	//5 Port Virtual HUB

#define USBDC_VHUB_PORT_NUM			7	//7 Port Virtual HUB
										//Port0~Port3 mapping to HC
#define USBDC_VHID_PORT_NUM			5	//The virtual HID use Port number 5
#define USBDC_VMSC_PORT_NUM			6	//The virtual MSC use Port number 6

//#if (SYSTEM_EXTENDER_SUPPORT)
//  #define USBDC_VREMOTE_HUB_PORT_NUM	6
//#endif /* #if (SYSTEM_EXTENDER_SUPPORT) */

/* INCLUDE FILE DECLARATIONS */
/* DEFINATION DECLARATIONS */
/* NAMING CONSTANT DECLARATIONS */
typedef struct _DC_Virtual_Hub_Map_TypeDef
{
	U8_T HubPort_Devinx;
	U8_T Devinx_HubPort;
} USBDC_Vritaul_Hub_Map_TypeDef;

extern USBDC_Vritaul_Hub_Map_TypeDef USBDC_Virtual_Hub_Map[USBDC_DEVICE_MAX];

/* GLOBAL VARIABLES */
/* EXPORTED GLOBAL VARIABLES */
extern U8_T  VHUB_MSC_Reset_Flag;
extern U8_T  VHUB_MSC_Reset_Port;
extern U8_T  VHUB_MSC_Reset_HubDevinx;
extern U8_T  VHUB_MSC_Reset_HubPort;
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void	USBDC_VHub_Init(void);
RESULT	USBDC_VHub_Setup_Process(U8_T devinx,U8_T port);
void	USBDC_VHub_UpPort_Reset(U8_T devinx,U8_T pid);
void	USBDC_VHub_Update_StatusChange(U8_T devinx,U8_T startport,U8_T end_port,U8_T resume);
void 	USBDC_VHub_Resume_Port(U8_T port,U8_T target_devinx);
void 	USBDC_VHub_Resume_HubPort_Device(U8_T dcport);	
#endif /* End of __USBDC_VHUB_H__ */

/* End of usbdc_vhub.h */