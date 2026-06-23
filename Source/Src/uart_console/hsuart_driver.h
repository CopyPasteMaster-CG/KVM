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
 * Module Name : hsuart_driver.h
 * Purpose     : A header file of uart.c includes UART1 module.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

#ifndef __HSUART_DRIVER_H__
#define __HSUART_DRIVER_H__

/* INCLUDE FILE DECLARATIONS */
/* NAMING CONSTANT DECLARATIONS */
 
/* GLOBAL VARIABLES */
extern U8_T		hsuart_TxBuf[];
extern U16_T	hsuart_TxHead;
extern U16_T	hsuart_TxTail;
extern U16_T	hsuart_TxCount;
extern U8_T		hsuart_TxFlag;
extern U8_T		hsuart_RxBuf[];
extern U16_T	hsuart_RxHead;
extern U16_T	hsuart_RxTail;
extern U16_T	hsuart_RxCount;
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
S8_T HSUART_PutChar(S8_T c);
#endif /* End of __HSUART_DRIVER_H__ */

/* End of uart.h */