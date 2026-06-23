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
 * Module Name: usbdc_vmsc.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
 
/* INCLUDE FILE DECLARATIONS */
/* DEFINATION DECLARATIONS */ 
#ifndef __USBDC_MSC_H__
#define __USBDC_MSC_H__

#define USBDC_VMSC_MAX_ENDP1_SIZE		64
#define USBDC_VMSC_MAX_ENDP2_SIZE		64

#define	USB_MSC_REQ_GML		0xFE
#define	USB_MSC_REQ_BOMSR	0xFF

#define	CBW_LENGTH		31
#define	CSW_LENGTH		13
#define	CBW_SIGNATURE	0x55534243	// 55425355h (little endian)
#define	CSW_SIGNATURE	0x55534253	// 55425355h (little endian)


#define CSW_STATUS_PASS			0
#define	CSW_STATUS_FAIL			1
#define CSW_STATUS_PHASE_ERROR	2

#define CBW_FLAG_IN		0x80
#define CBW_FLAG_OUT	0x00
/*
$ USBDC_MSC_State
*/

#define USBDC_MSC_IDLE					0x00
#define USBDC_MSC_DATA_OUT_WAIT 		0x01   // MSC Port Swtich 
#define USBDC_MSC_DATA_IN_WAIT			0x02   // MSC Port Swtich 
#define USBDC_MSC_WAIT_UNATTACH			0x04   // wait the Virtual HUB report back to host the device is disattached
#define USBDC_MSC_SWITCH_MASK			0x40   // MSC Port Swtich 
#define USBDC_MSC_ACTIVE_MASK			0x80   // MSC Port Active

/* NAMING CONSTANT DECLARATIONS */
typedef enum _MSC_STATE {
	MSC_IDLE = 0,
	MSC_CBW,
	MSC_DATA_IN,
	MSC_CMD_RESPONSED,
	MSC_DATA_OUT,
	MSC_CSW,
	MSC_CSW_BULKIN_STALL,
	MSC_BULKIN_STALL,	
	MSC_END,
	MSC_OP_NOT_SUPPORT,
} MSC_STATE;

/* STRUCTURE TYPE DECLARATIONS */

typedef struct
{
	U32_T	dCBWSignature;
	U32_T	dCBWTag;
	U32_T	dCBWDataTransferLength;
	U8_T	bCBWFlags;
	U8_T	bCBWLUN;
	U8_T	bCBWCBLength;
	U8_T	CBWCB[16];
} MSC_Cbw;

typedef struct
{
	U32_T	dCSWSignature;
	U32_T	dCSWTag;
	U32_T	dCSWDataResidue;
	U8_T	bCSWStatus;	
} MSC_Csw;

/* GLOBAL VARIABLES */
/* EXPORTED GLOBAL VARIABLES */
extern U8_T  KVM_CurrentPEN;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void	USBDC_VMSC_Stop(void);
RESULT	USBDC_VMSC_Setup_Process(U8_T devinx,U8_T port);
U8_T	USBDC_MSC_Bulk_In_Send(U8_T devinx,U8_T endpinx,U8_T datalen,U8_T *buf);
U8_T	USBDC_MSC_Media_Write(U8_T *txDmaPt,U8_T txLen);
void	USBDC_VMSC_Endpx_Data_IN_Handle(U8_T port,U8_T devinx,U8_T endpinx);
void	USBDC_VMSC_Endpx_Data_OUT_Complete(U8_T port,U8_T devinx,U8_T endpinx);
void	USBDC_VMsc_Init(void);
void	USBDC_VMSC_Port_Switch(U8_T newport);
void	USBDC_VMSC_Start(void);
void	USBDC_MSC_Stall_Clear(U8_T devinx);
void	USBDC_MSC_State_Reset(void);
#endif /* End of __USBDC_MSC_H__ */

/* End of usbdc_main.h */