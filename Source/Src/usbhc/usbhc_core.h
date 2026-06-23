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
 * Module Name : usbhc_core.h
 * Purpose     : 
 * Author      : 
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */
#ifndef USB_HC_CORE_H
#define USB_HC_CORE_H

/* INCLUDE FILE DECLARATIONS */
#include	"usb_def.h"
#include	"usbhc_def.h"

/* Hardware Relative Define */
/* Nameing Define */
/** @defgroup USBH_CORE_Exported_Defines
  * @{
  */
#define HID_CLASS						0x03
#define MSC_CLASS						0x08
#define HUB_CLASS						0x09
#define MSC_PROTOCOL					0x50
#define CBI_PROTOCOL					0x01
#define USBH_MAX_ERROR_COUNT			2
#define USBHC_TD_ISTL					0x00
#define USBHC_TD_INTL					0x01
#define USBHC_TD_ATL					0x02
#if ((SYSTEM_MSC_DEVICE_SUPPORT) && (SYSTEM_USB_HC_BURST))
#define USBHC_TD_BULK					0x03
#endif

/* MACRO DECLARATIONS */


/* TYPE DECLARATIONS */

/*
TD_Done_Struct()
*/
#define TDDONE_ATTR_TYPE_MASK			0x60
#define TDDONE_ATTR_TYPE_ISTL			0x00
#define TDDONE_ATTR_TYPE_INTL			0x20
#define TDDONE_ATTR_TYPE_ATL			0x40
#define TDDONE_ATTR_DEVINX_MASK			0x0F

typedef struct _HCTD_Done
{
	/*1.ISTL=Isochronous */
	U8_T	Attr;	//Bit7 UseFlag
					//Bit6~5 Done Type
					//00-ISTL
					//01-INTL
					//10-ATL/BULK
					//11-Reverse
	U8_T	TD_Num;	//Bit 4~0 TD Number
} HCTD_Done_Typedef;


/* GLOBAL VARIABLES */
extern volatile U8_T	USBHC_ISRQ_Rp,USBHC_ISRQ_Wp;
extern U8_T				TASK_USBHC_Handle_Control_ID;
extern U8_T				TD_INTL_Skip_Map[4],TD_ATL_Skip_Map[4],TD_INTL_Reload_Map[4];
extern U8_T				TD_ISTL_Skip_Map,TD_ISTL_Reload_Map;
extern U8_T				USBHC_ATL_Done_Q_Rp,USBHC_ATL_Done_Q_Wp;
extern const U8_T		BIT_MASK[];
extern U8_T				USBHC_Current_Total_Hub_Count;
extern U8_T				USBHC_Max_Device_Support;
extern U8_T				USBHC_Current_Total_Device_Count;
extern U8_T				USBHC_TD_Activity_Cnt;
extern bit     			USBHC_MSC_DismountFlag;
extern U8_T				USBHC_ErrorState;
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
#define USBHC_CORE_TD_Offset(x) (x-USB_HC_ATL_START)

void USBHC_CORE_Init(void);
U8_T USB_HAL_Alloc_Free_PDevice(U8_T *freeid, U8_T port);
U8_T USB_HAL_Check_PDevice_Status(U8_T port);
void USBHC_ISR_Handle(void);
U8_T USBHC_CORE_Alloc_TD(void);
void TASK_TASK_USBHC_CORE_Control_Transfer_Handle(void);
void USBHC_CORE_SetBit(U8_T *tab,U8_T index,U8_T value);
//U8_T USBHC_CORE_GetBit(U8_T *tab,U8_T index);
void TASK_USBHC_Done_Handle(void);
void TASK_USBHC_INTL_Done_Handle(U8_T);
void USBHC_CORE_Set_TD_SkipMap(U8_T tdtype,U8_T tdid,U8_T value);
//U8_T USBHC_CORE_Get_TD_SkipMap(U8_T tdtype,U8_T tdid);
void USBHC_CORE_Clear_TD_DoneMap(U8_T tdtype,U8_T tdid);
void USBHC_CORE_Clear_Device(U8_T devinx);
U8_T USBHC_CORE_Alloc_IntTransfer_Table(U8_T *freeid);
U8_T USBHC_CORE_QueryAlloc_IsoTransfer_Table(U8_T allocateID);
void USBHC_CORE_Free_TD(U8_T td_id);
void USBHC_CORE_Set_INTL_ReloadMap(U8_T tdid,U8_T value);
void USBHC_CORE_Suspend(void);
void USBHC_CORE_Resmue(void);
void USBHC_CORE_Build_ISTL_Polling(U8_T devinx);
void USBHC_CORE_Retrive_USB_State(U8_T td_type,U8_T atl_id,U8_T td_id);
void USBHC_CORE_BULK_Done_Handle(void);
void USBHC_CORE_INTL_Done_Handle(void);
void USBHC_CORE_Clear_TD_Register(U8_T td_id);
void USBHC_CORE_Change_IntTransfer_Interval(U8_T device,U8_T intfid,U8_T interval);
#endif /* End of USB_HC_CORE_H */
