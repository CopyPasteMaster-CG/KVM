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
 * Module Name : edid_hardware.h
 * Purpose     : 
 * Author      : Jack Wang
 * Date        :
 * Notes       : 
 * no message
 *
 *================================================================================
 */
#ifndef __EDID_HARDWARE_H
#define __EDID_HARDWARE_H

/* INCLUDE FILE DECLARATIONS */
/* NAMING CONSTANT DECLARATIONS */


/* MACRO DECLARATIONS */

void HW_EDID_Init(void);
void HW_EDID_EnableSlave(void);
void HW_EDID_ReloadAutoMode(void);
void HW_EDID_PlugAutoDetectMode(U8_T,FunctionalState);
#endif /* End of __BUZZER_GPIO_H */

