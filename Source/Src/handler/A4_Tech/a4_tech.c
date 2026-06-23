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
 * Module Name : a4_tech.c
 * Purpose     : The special handler for Logitech unifying wireless HID device
 *               Test Item:G800V
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 *               The unifying has wireless mouse, touch pad, touch pad mouse has used
 *               logitech unifying protocol, basically are device into HID and DJ mode
 *               this program are the collection of handler program of unifying.
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	<string.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	"project_include.h"

/* STATIC VARIABLE DECLARATIONS */

/* LOCAL SUBPROGRAM DECLARATIONS */
U16_T A4TECH_IVENDOR=0x09DA;
U16_T A4TECH_IPORDUCT_GV800V=0x90C0;

/* LOCAL SUBPROGRAM BODIES */
/*************************************************************************************
 * U8_T A4_TECH_Check_Protocol(U8_T devinx,U8_T port,U8_T intf_bit)
 * Description : Check the special handler, for A4 Tech G800V keyboard, can not change
 *               into report mode from boot mode, so need to reset it on the HC.
 *
 * 
 *************************************************************************************/
/*
U8_T A4_TECH_Check_Protocol(U8_T devinx,U8_T port,U8_T intf_bit)
{
	if ((USB_PDevice[devinx].idVendor == A4TECH_IVENDOR) && (USB_PDevice[devinx].idProduct == A4TECH_IPORDUCT_GV800V))
	{
		if (USB_PDevice[devinx].Current_Protocol & intf_bit)  //current boot protocl on Physical HC devcie
		{
			if (USBDC_Device[devinx].Current_Protocol[port] & intf_bit)  //current report_protocl on Virtual DC devcie
			{
				//=>Need to reset the device
				return 1;
			}
		}
	}
	return 0;
}
*/

/* End of a4_tech.c */


