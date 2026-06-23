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
void MULTIVIEW_Init(void);
void MULTIVIEW_Receive_Handle(void);
#endif /* End of __MULTIVIEW_H */




