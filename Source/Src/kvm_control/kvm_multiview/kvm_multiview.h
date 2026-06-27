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
 * Module Name : kvm_multiview.h
 * Purpose     : The c header file for multiview chip
 * Author      : Jack Wang
 * Date        :
 * Notes       : 
 * no message
 *
 *================================================================================
 */
#ifndef __MULTIVIEW_H
#define __MULTIVIEW_H

/* INCLUDE FILE DECLARATIONS */

/* NAMING CONSTANT DECLARATIONS */

//#define MULTIVIEW_1_Mode		1	//Single Monitor Mode
//#define MULTIVIEW_4_Mode		0	//4 equal Monitor Mode

#define MULTIVIEW_1_Mode		SINGLE_SCREEN_MODE	//Single Monitor Mode
#define MULTIVIEW_4_Mode		FOUR_SCREEN_MODE	//4 equal Monitor Mode


#define KVM_MODE_PORT1_ONLY              0x01
#define KVM_MODE_PORT2_ONLY              0x02
#define KVM_MODE_PORT3_ONLY              0x03
#define KVM_MODE_PORT4_ONLY              0x04

#define KVM_MODE_TWO_SMALL_MAIN_PORT1    0x05
#define KVM_MODE_TWO_SMALL_MAIN_PORT2    0x06
#define KVM_MODE_TWO_SMALL_MAIN_PORT3    0x07

#define KVM_MODE_PORT12_ONLY             0x08
#define KVM_MODE_PORT34_ONLY             0x09
#define KVM_MODE_PORT_ALL                0x0A
#define KVM_MODE_PORT_ALL_SYNC           0x0B

#define KVM_MODE_THREE_SMALL_MAIN_PORT4  0x0C
#define KVM_MODE_THREE_SMALL_MAIN_PORT3  0x0D
#define KVM_MODE_THREE_SMALL_MAIN_PORT2  0x0E
#define KVM_MODE_THREE_SMALL_MAIN_PORT1  0x0F

#define KVM_CMD_SET_MODE     0x01  /* 设置KVM模式 */
#define KVM_CMD_GET_MODE     0x02  /* 查询当前模式 */
#define KVM_CMD_REPORT_MODE  0x82  /* 返回当前模式 */
#define KVM_CMD_ACK          0x80  /* 设置结果应答 */


typedef enum
{
    KVM_RX_HEAD1 = 0,
    KVM_RX_HEAD2,
    KVM_RX_CMD,
    KVM_RX_MODE,
    KVM_RX_CRC
} KVM_RX_STATE;

typedef struct
{
    KVM_RX_STATE state;
    U8_T cmd;
    U8_T mode;
} KVM_RX_DATA;





#define MULTIVIEW_Out_HDMIx				0
#define MULTIVIEW_Out_DislayModeA			4
#define MULTIVIEW_Out_DislayModeB			5
#define MULTIVIEW_Out_Audiox				6

#define MULTIVIEW_UP_DIRECTION				0
#define MULTIVIEW_DOWN_DIRECTION			1
#define MULTIVIEW_LEFT_DIRECTION			2
#define MULTIVIEW_RIGHT_DIRECTION			3

/* MACRO DECLARATIONS */

/* GLOBAL VARIABLES */
/* EXPORTED SUBPROGRAM SPECIFICATIONS */

void KVM_SET_mode(U8_T mode);
void MULTIVIEW_Init(void);
void MULTIVIEW_Receive_Handle(void);
#endif /* End of __MULTIVIEW_H */




