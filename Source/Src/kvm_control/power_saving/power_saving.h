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
 * Module Name : power_saving.h
 * Purpose     : A header file of KVM Power Saving Handle function
 * Author      : Jack Wang
 * Date        :
 * Notes       : 
 * no message
 *
 *================================================================================
 */
#ifndef __KVM_POWERSAVING_H
#define __KVM_POWERSAVING_H

/* INCLUDE FILE DECLARATIONS */


/* GLOBAL VARIABLES */
extern bit	PowerSavingStartFlag;
extern bit	PowerSavingInhibitFlag;
extern bit	PowerSavingExitFlag;
extern U8_T	PowerSavingExitSate;

/* Subfunction Varable Declared */
void KVM_PowerSaving_Init(void);
void KVM_PowerSaving_Exit(void);
#endif /* End of __KVM_POWERSAVING_H */




