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
 * Module Name : storage.h
 * Purpose     : 
 * Author      : 
 * Date        :
 * Notes       : 
 * no message
 *
 *================================================================================
 */
#ifndef __STORAGE_H
#define __STORAGE_H

/* INCLUDE FILE DECLARATIONS */
#include "types.h"
/* MACRO DECLARATIONS */

/* DEFINE DECLARATIONS */
#define STORAGE_START_ADDRESS			0x01F000									// From last 124 KBytes
#define STORAGE_START_PRIMARY_ADDRESS	STORAGE_START_ADDRESS						// From last 124 KBytes
#define STORAGE_START_SECONDARY_ADDRESS	(STORAGE_START_PRIMARY_ADDRESS + 0x400)		// From last 125 KBytes
#define STORAGE_START_THIRDARY_ADDRESS	(STORAGE_START_SECONDARY_ADDRESS + 0x400)	// From last 126 KBytes
#define STORAGE_START_FOURTH_ADDRESS	(STORAGE_START_THIRDARY_ADDRESS + 0x400)	// From last 127 KBytes

/*----------------
$ STORAGE_Status
----------------*/
#define STORAGE_STATE_IDLE				0x00
#define STORAGE_STATE_READ_START		0x01
#define STORAGE_STATE_READ_ERROR		0x02
#define STORAGE_STATE_WRITE_START		0x04
#define STORAGE_STATE_WRITE_ERROR		0x08


/* TYPE DECLARATIONS */
/* GLOBAL VARIABLES */
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void STORAGE_Init(void);
void TASK_STORAGE_Control(void);
#if (SYSTEM_STORAGE_FLASH)
void STORAGE_Init_Read(U16_T,U8_T *,U8_T);
#endif
void STORAGE_Default_Initial(void);
void STORAGE_Write(U16_T,U8_T *);
U16_T STORAGE_CRC_Calculate(U8_T *);
void STORAGE_SoftwareConfigurationCheck(U8_T houseKeeping);
#endif /* End of __STORAGE_H */




