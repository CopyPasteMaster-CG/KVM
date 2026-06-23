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
 * Module Name : a4_tech.h
 * Purpose     : A header file of a4_tech.c
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

#ifndef __USB_A4_TECH_H__
#define __USB_A4_TECH_H__

/* INCLUDE FILE DECLARATIONS */
/* GLOBAL VARABLE DECLARATIONS */
extern U16_T A4TECH_IVENDOR;
extern U16_T A4TECH_IPORDUCT_GV800V;
/* EXTERNAL SUB-FUNCTION DECLARATIONS */
U8_T A4_TECH_Check_Protocol(U8_T devinx,U8_T port,U8_T intf_bit);
#endif /* End of __USB_A4_TECH_H__ */
/* End of a4_tech.h */