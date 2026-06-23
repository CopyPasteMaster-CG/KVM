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
 * Module Name : kvm_vendorhandle.h
 * Purpose     : A header file of kvm_vendorhandle.c
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

#ifndef __USB_KVM_VENDOR_H__
#define __USB_KVM_VENDOR_H__

/* 
$ INCLUDE FILE DECLARATIONS 
*/
#include "usbhc.h" 
/*
$ STRUCTURE DECLARATIONS
*/

#define TEST_STATE_TEST_MODE_MASK     0x0007
  #define TEST_STATE_TEST_NONE        0x0000
  #define TEST_STATE_TEST_CONTROL     0x0001
  #define TEST_STATE_TEST_BULK        0x0002
  #define TEST_STATE_TEST_INT         0x0003
  #define TEST_STATE_TEST_ISO         0x0004
  #define TEST_STATE_TEST_BUS         0x0005

#define TEST_STATE_BUS_MODE_MASK      0x0038
  #define TEST_STATE_BUS_NORMAL       0x0000
  #define TEST_STATE_BUS_SUSPEND      0x0008
  #define TEST_STATE_BUS_RESUME       0x0010
  #define TEST_STATE_BUS_REWAKEUP     0x0018

#define TEST_STATE_READY_MASK         0x0040
  #define TEST_STATE_READY_SET        0x0040
  #define TEST_STATE_READY_RESET      0x0000

#define TEST_STATE_PAUSED_MASK        0x0080
  #define TEST_STATE_PAUSED_SET       0x0080
  #define TEST_STATE_PAUSED_RESET     0x0000

#define TEST_STATE_MASK               0x0f00
  #define TEST_STATE_IDLE             0x0000
  #define TEST_STATE_COMPLETE         0x0100
  #define TEST_STATE_RUNNING          0x0200
  #define TEST_STATE_FAIL             0x0300
  #define TEST_STATE_DONE             0x0f00
  
#define TEST_STATE_MODE_MASK          0x8000
  #define TEST_STATE_SINGLE           0x0000
  #define TEST_STATE_BATCH            0x8000
  
typedef struct _USBHC_Test
{
	U16_T  State;          // Root Hub Port Control State
                           // Bit0~2 Test Transfer Type
                           //    000-not assign
                           //    001(0x01)-controler transfer type
                           //    010(0x02)-Bulk transfer type
                           //    011(0x03)-Interrupt transfer type
                           //    100(0x04)-Isochronous transfer type
                           // Bit3~5 Bus Status
                           //    000(0x00)-Normal
                           //    001(0x01)-Suspend
                           //    010(0x02)-Resume
                           //    011(0x03)-Remoate Wakeup Enable
                           // Bit6- Device Ready Flag, if ready then start test
                           // Bit7- Device Paused Flag, stop test tempany
                           // Bit8~11-Bus Control Statue
                           //   0000->Idle
                           //   0001->Complete
                           //   0002->Fail
                           //
                           //
                           // Bit15-Single Mode,Batch Mode
    U16_T PackageSize;
    U16_T TotalLength;
    U16_T CurrentLength;
    U16_T CurrentPackCnt;
    U16_T TotalPackCnt;
    U8_T  Interval;
    U8_T  *Buf;
} USBHC_Test_TypeDef;

/*
$ EXTERNAL GOLBAL VARIABLE DECLARATIONS
*/ 
extern U8_T TASK_USBHC_TEST_Class_Handle_ID;
/*
$ SUBPROGRAM DECLARATIONS
*/
void USBHC_TEST_Class_Init(void);
void TASK_USBHC_TEST_Class_Handle(void);

USBH_Status USBHC_TEST_Set_Report(U8_T  devinx,
                                  U8_T  td_id,
                                  U8_T  reportType,
                                  U8_T  reportId,
                                  U16_T reportLen,
                                  U8_T  *reportBuff);
                                  
#endif /* End of __KVM_VENDOR_H__ */
/* End of kvm_vendorhandle.h */