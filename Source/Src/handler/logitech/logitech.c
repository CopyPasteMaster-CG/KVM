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
 * Module Name : logitech.c
 * Purpose     : The special handler for Logitech unifying wireless HID device
 *               Test Item:M185,T650,T400
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
U8_T UNIFYING_Switch_DJ_Command[15] ={0x20,0xff,0x80,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
U8_T UNIFYING_Switch_HID_Command[15]={0x20,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
U8_T UNIFYING_Switch_HIDPP_Command[7]={0x10,0x00,0x0f,0x20,0x09,0x00,0x00};
U8_T UNIFYING_Switch_HID0_Command[7] ={0x10,0x00,0x0f,0x20,0x00,0x00,0x00};
/* LOCAL SUBPROGRAM BODIES */

/*************************************************************************************
  * @brief  USBDC_Unifying_Set_Report_Check()
  *         Check the set report command
  *         0x21 0x09 0x02 0xyy ......
  * 
  *************************************************************************************/
RESULT USBDC_Unifying_SetReport_Cmd_Check(U8_T port,U8_T devinx,U8_T report_id)
{
	if (report_id == 0x10)
	{
		if (USBDC_Device[devinx].Setup[port].b.wLength.w == 7)
		{
			USBDC_Device[devinx].DevAttr[port] |= UNIFYING_RPT_CHECK_MASK;
		}
	}
	else if (report_id == 0x20) //DJ Mode
	{
		if (USBDC_Device[devinx].Setup[port].b.wLength.w == 15)
		{	
			USBDC_Device[devinx].DevAttr[port] |= UNIFYING_RPT_CHECK_MASK;
			if (port != KVM_CurrentHost)
			{
				return USB_SUCCESS;
			}
		}
	}
	return USB_WAIT;
}

/*************************************************************************************
  * @brief  USBDC_Unifying_Set_Report_Check()
  *         Check the set report command
  *         0x21 0x09 0x02 0xyy ......
  * 
  *************************************************************************************/
void USBDC_Unifying_SetReport_Data_Check(U8_T port,U8_T devinx,U8_T *DataBuffer)
{
	U8_T DJ_mode;

	USBDC_Device[devinx].DevAttr[port] &= ~UNIFYING_RPT_CHECK_MASK;
	if (memcmp(DataBuffer,UNIFYING_Switch_HID_Command,3)==0) //switch command
	{
		if(DataBuffer[3]) //if set to DJ
		{
			USBDC_Device[devinx].DevAttr[port] |= UNIFYING_RPT_MODE_MASK; //DJ Mode
			DJ_mode = UNIFYING_RPT_MODE_MASK;
		}
		else
		{
			USBDC_Device[devinx].DevAttr[port] &= ~UNIFYING_RPT_MODE_MASK; //HID Mode
			DJ_mode = 0;
		}
		
		if (USBDC_Device[devinx].PassThroughState[port])
		{
			USB_PDevice[devinx].DevAttr &= ~UNIFYING_RPT_MODE_DJ;
			USB_PDevice[devinx].DevAttr |= DJ_mode; //update HC mode
		}
	}
	else if ((DataBuffer[0]== 0x10)&& (DataBuffer[2]== 0x0f) && ((DataBuffer[3] & 0xf0) == 0x20))
	{
		if ((DataBuffer[4] & 0x0f) == 0x09) //=>HIDPP mode
		{
			USBDC_Device[devinx].DevAttr[port] |= UNIFYING_RPT_SIZE_MASK; //HIDPP Mode
			DJ_mode = UNIFYING_RPT_SIZE_MASK;
		}
		else
		{
			USBDC_Device[devinx].DevAttr[port] &= ~UNIFYING_RPT_SIZE_MASK; //HID Mode
			DJ_mode = 0;
		}
			
		if (USBDC_Device[devinx].PassThroughState[port])
		{
			USB_PDevice[devinx].DevAttr &= ~UNIFYING_RPT_SIZE_MASK;
			USB_PDevice[devinx].DevAttr |= DJ_mode; //update HC mode
		}

		USBDC_Device[devinx].Unifying_index = DataBuffer[1];
		USBDC_Device[devinx].Unifying_index |= (DataBuffer[3]&0x0f) << 4;
	}
}

/*************************************************************************************
  * void USBHC_Unifying_KVM_Switch_Check(U8_T port,U8_T devinx)
  * Description:Check the set report command
  * 
  * 
  *************************************************************************************/
void USBHC_Unifying_KVM_Switch_Check(U8_T port,U8_T devinx)
{
	U8_T DJ_mode;

	//Check host connection

	//1.Check HID Mode
	DJ_mode = USBDC_Device[devinx].DevAttr[port] & UNIFYING_RPT_SIZE_MASK;
	if ( DJ_mode != (USB_PDevice[devinx].DevAttr & UNIFYING_RPT_SIZE_MASK)) 
	{
		USB_PDevice[devinx].DevAttr &= ~UNIFYING_RPT_SIZE_MASK;
		USB_PDevice[devinx].DevAttr |= DJ_mode;
		USBHC_Set_Dev_Logitech_Unifying_HID(devinx,DJ_mode);
	}
	else
	{
		//1.Check DJ Mode
		DJ_mode = USBDC_Device[devinx].DevAttr[port] & UNIFYING_RPT_MODE_DJ;
		if ( DJ_mode != (USB_PDevice[devinx].DevAttr & UNIFYING_RPT_MODE_DJ)) 
		{
			USB_PDevice[devinx].DevAttr &= ~UNIFYING_RPT_MODE_DJ;
			USB_PDevice[devinx].DevAttr |= DJ_mode;
			USBHC_Set_Dev_Logitech_Unifying_Mode(devinx,DJ_mode);
		}
	}
}

/*************************************************************************************
  * U8_T USBHC_Unifying_Interrupt_In_Check(U8_T port,U8_T devinx,U8_T intt_id,U8_T *DJ_Buf,U8_T len,U8_T deviceid)
  * Description:Check the set report command
  * 
  * 
  *************************************************************************************/
U8_T USBHC_Unifying_Interrupt_In_Check(U8_T port,U8_T devinx,U8_T intt_id,U8_T *DJ_Buf,U8_T len,U8_T deviceid)
{
	if (HCTD_Table.INTL[intt_id].Buf[0] == 0x20) 
	{
		if (len == 15) //short DJ
		{
			if (deviceid==0x01) // keyboard
			{
				if (USBDC_Device[devinx].DevAttr[port] & UNIFYING_RPT_MODE_DJ)
				{
					if (HCTD_Table.INTL[intt_id].Buf[2] == 0x01) //keyboard	
					{
						DJ_Buf[0] = HCTD_Table.INTL[intt_id].Buf[3];
						DJ_Buf[1] = 0;
						memcpy(&DJ_Buf[2],&HCTD_Table.INTL[intt_id].Buf[4],6);
						USBHC_HidParser_Package_Handle(devinx,0,DJ_Buf,8,1);
						return 1;
					}
				}
			}
			else if (deviceid==0x02) //mouse
			{
				if (HCTD_Table.INTL[intt_id].Buf[2] == 0x02) //Mouse
				{
					if ((USBDC_Device[devinx].DevAttr[port] & (UNIFYING_RPT_SIZE_MASK|UNIFYING_RPT_MODE_MASK)) == 0x00)
					{
						DJ_Buf[0] = HCTD_Table.INTL[intt_id].Buf[2];
						memcpy(&DJ_Buf[1],&HCTD_Table.INTL[intt_id].Buf[3],7);
						return 1;
					}
				}
			}
		}
	}
	else if (HCTD_Table.INTL[intt_id].Buf[0] == 0x11) 
	{
		if (len == 20) //DJ mode
		{
			if ((USB_PDevice[devinx].DevAttr & (UNIFYING_RPT_SIZE_MASK|UNIFYING_CHANGE_HID_MASK))== 0x00)
			{
				if ((HCTD_Table.INTL[intt_id].Buf[1] != 0x00) && (HCTD_Table.INTL[intt_id].Buf[1] != 0xff) && (HCTD_Table.INTL[intt_id].Buf[3] == 0x00))
				{
					USB_PDevice[devinx].DevAttr |= UNIFYING_CHANGE_HID_MASK;
					USBDC_Device[devinx].Unifying_index  =  HCTD_Table.INTL[intt_id].Buf[1];
					USBDC_Device[devinx].Unifying_index |= 0x50;
					USBHC_Set_Dev_Logitech_Unifying_HID(devinx,0); 
					return 1;
				}
			}
		}
	}
	else if (HCTD_Table.INTL[intt_id].Buf[0] == 0x02) 
	{
		if (len == 8) //HID mode
		{
			if (USB_PDevice[devinx].DevAttr & UNIFYING_RPT_MODE_MASK) //DJ MOde
			{
				if ((USB_PDevice[devinx].DevAttr & UNIFYING_CHANGE_MODE_MASK) == 0x00)
				{
					USB_PDevice[devinx].DevAttr |= UNIFYING_CHANGE_MODE_MASK;
					USBHC_Set_Dev_Logitech_Unifying_Mode(devinx,UNIFYING_RPT_MODE_MASK);
				}
			}
			else if (USB_PDevice[devinx].DevAttr & UNIFYING_RPT_SIZE_MASK)
			{
				if ((USB_PDevice[devinx].DevAttr & UNIFYING_CHANGE_HID_MASK) == 0x00)
				{
					USB_PDevice[devinx].DevAttr |= UNIFYING_CHANGE_HID_MASK;
					USBDC_Device[devinx].Unifying_index  =  HCTD_Table.INTL[intt_id].Buf[1];
					USBDC_Device[devinx].Unifying_index |= 0x50;
					USBHC_Set_Dev_Logitech_Unifying_HID(devinx,UNIFYING_RPT_SIZE_MASK);
					return 1;
				}
			}
		}
	}

	return 0;
}

/* End of logitech.c */


