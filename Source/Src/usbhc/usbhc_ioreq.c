/*
 ******************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is an proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
 /*============================================================================
 * Module Name: usbhc_ioreq.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */
#include	"project_include.h"

/* STATIC VARIABLE DECLARATIONS */
/* LOCAL SUBPROGRAM DECLARATIONS */ 
static void usbhc_Ctrl_TD_Submit(U8_T td_offset);
static U8_T usbhc_Ctrl_TD_Header(U8_T devinx,U8_T td_id);
static U16_T usbhc_Total_Byte_Fix(U8_T devinx, U16_T length);

/* LOCAL SUBPROGRAM BODIES */
/**
  * @brief  USBHC_CtlReq
  *         USBHC_CtlReq active a control task   
  * @param  devinx: Selected device
  * @param  td_id : Selected host channel  
  * @retval Status
  */
USBH_Status USBHC_CtlReq (U8_T 	devinx,U8_T td_id)
{
	USB_PDevice[devinx].Hc.Control.State = CTRL_SETUP;
	TASK_Active(TASK_TYPE_USB,TASK_USBHC_Handle_Control_ID,td_id,devinx,0,0); // Generate the Setup Send command
	return USBH_OK;
}

/**
  * @brief  usbhc_Ctrl_TD_Header
  *         copy TD header 8 bytes for Host Hannel & Device Table
  * @param  devinx: Selected device
  * @param  td_id : Selected host channel
  * @retval td_offet
  */ 
static U8_T usbhc_Ctrl_TD_Header(U8_T devinx,U8_T td_id)
{
	USBHC_TD_Header_Typedef	*td_header;
	U8_T					td_offset;

	td_offset = USBHC_CORE_TD_Offset(td_id);

	//1.Get td_header address
	td_header = (USBHC_TD_Header_Typedef *) &(HCTD_Table.ATL[td_offset].TD);
	//2.Fill up TD Header Content -------------------------------------------------------------
	//2-0.Actual Byte
	td_header->Byte0_Actual_Byte = 0;
	//2-1.(1)Toggle=>Data0
	//    (2)Active=>1
	//    (3)Complete code=>0x0f
	
	//2-2.(1)Toggle=>Data0
	//    (2)Active=>1
	//    (3)Complete code=>0x0f
	td_header->Byte2_MaxPKT_Size = HCTD_Channel_Table[td_id].Endp_Size;
	//2-3.(1)Speed
	//    (2)Endpoint Number
	td_header->Byte3 = (HCTD_Channel_Table[td_id].Attri & CHANNEL_ATRI_ENDPADDR_MASK) << 4;
	if (USB_PDevice[devinx].InterfaceNum & USBHC_DEVICE_SPEED_MASK) /* low speed device check */
		td_header->Byte3 |= TD_SPEED_LOW;
	//2-4.(1)Total Bytes
	td_header->Byte4_Total_Byte = HCTD_Channel_Table[td_id].TD_Length;
	//2-5.(1)Error Count(ingore)
	//    (2)Dir Token
	//    (3)Total Byte MSD
	td_header->Byte5  = ((HCTD_Channel_Table[td_id].TD_Length & 0x0300) >>8) | TD_DIR_TOKEN_SETUP;
	//2-6.(1)Device Address
	td_header->Byte6_Function_Address = USB_PDevice[devinx].Addr & USBHC_DEVICE_ADDR_MASK;
	//2-7.(1)Starting frame
	td_header->Byte7 = 0xff; // Nake Count
	
	td_header->Byte1 = COMPLETEION_CODE_MASK | TD_ACTIVED_MASK;
	return td_offset;
}

/**
  * @brief  usbhc_Ctrl_TD_Submit
  *         Submit the TD by setting SkipMap,DoneMap,ATLATV registers
  * @param  td_offet : Selected ATL TD offset number
  * @retval None
  */
static void usbhc_Ctrl_TD_Submit(U8_T td_offset)
{
	//1.Reset the DoneMap Flag
	USBHC_CORE_Clear_TD_DoneMap(USBHC_TD_ATL,td_offset);
	//2.Clear skip bit
	USBHC_CORE_Set_TD_SkipMap(USBHC_TD_ATL,td_offset,0x00); // clear the skip map
}

/**
  * @brief  USBHC_CtlSendSetup
  *         Sends the Setup Packet to the Device
  * @param  pdev: Selected device
  * @param  buff: Buffer pointer from which the Data will be send to Device
  * @param  hc_num: Host channel Number
  * @retval Status
  */
void USBHC_CtlSendSetup(U8_T devinx,U8_T td_id)
{ 
	U8_T	td_offset, index;

	//1.Fill up TD Header & get TD offset
	td_offset = usbhc_Ctrl_TD_Header(devinx,td_id);

	//2.Copy setup command into data payload
	for (index=0; index < USBHC_SETUP_PKT_SIZE ; index++)
	{
		HCTD_Table.ATL[td_offset].Buf[index] = USB_PDevice[devinx].Hc.Control.Setup.d8[index];
	}

	//3.Sumbit the TD operation
	usbhc_Ctrl_TD_Submit(td_offset);
}

/**
  * @brief  USBHC_CtlSendData
  *         Sends a data Packet to the Device
  * @param  pdev: Selected device
  * @param  buff: Buffer pointer from which the Data will be sent to Device
  * @param  length: Length of the data to be sent
  * @param  hc_num: Host channel Number
  * @retval Status
  */
USBH_Status USBHC_CtlSendData (U8_T devinx,U8_T td_id,U16_T length)
{
	U8_T	td,index;
	USBHC_TD_Header_Typedef	*td_header;

	//1.Get the TD address
	td = USBHC_CORE_TD_Offset(USB_PDevice[devinx].Hc.Control.TdNum);

	td_header = (USBHC_TD_Header_Typedef *) &(HCTD_Table.ATL[td].TD);
	//2.Fill up TD Header Content
	//2-0.Actual Byte
	td_header->Byte0_Actual_Byte = 0;
	//2-2.(1)Toggle=>Data0
	//    (2)Active=>1
	//    (3)Complete code=>0x0f
	//td_header->Byte2_MaxPKT_Size = USBHC_SETUP_PKT_SIZE;
	td_header->Byte2_MaxPKT_Size = HCTD_Channel_Table[td_id].Endp_Size;
	//2-3.(1)Speed
	//    (2)Endpoint Number
	td_header->Byte3 = USBHC_EP0_EP_NUM;
	if (USB_PDevice[devinx].InterfaceNum & USBHC_DEVICE_SPEED_MASK) // Low Speed Device
		td_header->Byte3 |= TD_SPEED_LOW;
	//2-4.(1)Total Bytes
	if (length > USBHC_MAX_PACKET_SIZE)
		length = USBHC_MAX_PACKET_SIZE;

	td_header->Byte4_Total_Byte = (length & 0x00ff);
	td_header->Byte5 = (length & 0x0300) >>8;
	//2-5.(1)Error Count(ingore)
	//    (2)Dir Token
	td_header->Byte5 |= TD_DIR_TOKEN_OUT;
	//2-6.(1)Device Address
	td_header->Byte6_Function_Address = USB_PDevice[devinx].Addr & USBHC_DEVICE_ADDR_MASK;
	//2-7.(1)Starting frame
	td_header->Byte7 = 0xff; // next first frame
	//3.Copy setup command into data payload
	for (index=0;index < length ; index++)
	{
		HCTD_Table.ATL[td].Buf[index] = USB_PDevice[devinx].Hc.Control.Buf[USB_PDevice[devinx].Hc.Control.Current_Length+index];
	}
	/* Final Step */ 
	//2-1.(1)Toggle=>Data0
	//    (2)Active=>1
	//    (3)Complete code=>0x0f
	if (length == 0)
		td_header->Byte1 = COMPLETEION_CODE_MASK | TD_ACTIVED_MASK | TD_TOGGLE_DATA1;
	else
		td_header->Byte1 |= COMPLETEION_CODE_MASK | TD_ACTIVED_MASK;
	//4.Reset the DoneMap Flag
	USBHC_CORE_Clear_TD_DoneMap(USBHC_TD_ATL,td);
	//4-1.Clear skip bit 
	USBHC_CORE_Set_TD_SkipMap(USBHC_TD_ATL,td,0x00); // clear the skip map

	return USBH_OK;
}

/**
  * @brief  USBHC_CtlReceiveData
  *         Receives the Device Response to the Setup Packet
  * @param  pdev: Selected device
  * @param  buff: Buffer pointer in which the response needs to be copied
  * @param  length: Length of the data to be received
  * @param  hc_num: Host channel Number
  * @retval Status. 
  */
USBH_Status USBHC_CtlReceiveData(U8_T devinx, U8_T td_id, U16_T length, U8_T status)
{
	U8_T						td;
	USBHC_TD_Header_Typedef		*td_header;

	//1.Get the TD address
	td = USBHC_CORE_TD_Offset(td_id);

	td_header = (USBHC_TD_Header_Typedef *) &(HCTD_Table.ATL[td].TD);
	//2.Fill up TD Header Content
	//2-0.Actual Byte
	td_header->Byte0_Actual_Byte = 0;
	//disp_str((U8_T *)td_header,8,1);
	//2-2.(1)Toggle=>Data0
	//    (2)Active=>1
	//    (3)Complete code=>0x0f
	td_header->Byte2_MaxPKT_Size = (USB_PDevice[devinx].Hc.Control.EpN_MaxSize & 0x00ff);
	//2-3.(1)Speed
	//    (2)Endpoint Number
	td_header->Byte3 = (U8_T)((USB_PDevice[devinx].Hc.Control.EpN_MaxSize & 0x0300) >> 8) | USBHC_EP0_EP_NUM;
	
	if (USB_PDevice[devinx].InterfaceNum & USBHC_DEVICE_SPEED_MASK) // Low Speed Device
	{	
		td_header->Byte3 |= TD_SPEED_LOW;
		td_header->Byte7 = 0x28; //Nake counter
	}
	else
	{
		td_header->Byte7 = 0xfe; // Nake counter
	}

	//2-4.(1)Total Bytes
	if (length >= USBHC_MAX_PACKET_SIZE)
	{
		length = USBHC_MAX_PACKET_SIZE;
	}
	else
	{
		length = usbhc_Total_Byte_Fix(devinx, length);
	}

	td_header->Byte4_Total_Byte = (length & 0x00ff);
	td_header->Byte5 = (length & 0x0300) >> 8;
	
	//2-5.(1)Error Count(ingore)
	//    (2)Dir Token
	td_header->Byte5 |= TD_DIR_TOKEN_IN;
	
	//2-6.(1)Device Address
	//2-7.(1)Starting frame
	td_header->Byte6_Function_Address &= ~0x80;
			
	//2-1.(1)Toggle=>Data0
	//    (2)Active=>1
	//    (3)Complete code=>0x0f
	if ((length == 0) || status) // status stage
	{
		td_header->Byte1 = COMPLETEION_CODE_MASK | TD_ACTIVED_MASK | TD_TOGGLE_DATA1;
	}
	else
	{
		td_header->Byte1 |= (COMPLETEION_CODE_MASK | TD_ACTIVED_MASK);
	}

	USBHC_CORE_Clear_TD_DoneMap(USBHC_TD_ATL,td);
	
	//3.Reset the Skip Flag  
	USBHC_CORE_Set_TD_SkipMap(USBHC_TD_ATL,td,0x00); // clear the skip map

	return USBH_OK;
}

/**
  * @brief  USBHC_InterruptReqData
  *         Receives or Send the Interrupt Transfer type data
  *         This subroutine will used the INTL_Num TD to check
  *         the td is been used or not,
  *
  * @retval Status. 
  */
USBH_Status USBHC_InterruptReqData(U8_T devinx,U8_T intt_id)
{
	USBHC_TD_Header_Typedef	*td_header;
	U8_T					endpaddr,td_offset,inf_id,endp_id;
	U16_T					length;

	//1.Check if need to alloc a new free INTL TD position.
	td_offset = HC_IntTransfer_Table[intt_id].INTL_Num;
	//2.Set the Skip Map First
	USBHC_CORE_Set_TD_SkipMap(USBHC_TD_INTL,td_offset,0x01); // set the skip map first
	
	//3.Fill up TD Header Conten
	td_header = (USBHC_TD_Header_Typedef *) &(HCTD_Table.INTL[td_offset].TD);
	//3-0.Actual Byte
	td_header->Byte0_Actual_Byte = 0;
	//2-2.(1)Toggle=>Data0
	//    (2)Active=>1
	//    (3)Complete code=>0x0f
	inf_id = HC_IntTransfer_Table[intt_id].InterfaceIdx;
	endp_id = HC_IntTransfer_Table[intt_id].EndpIdx & USBHC_PDEV_ENDPIDX_MASK;
	td_header->Byte2_MaxPKT_Size = (USB_PDevice[devinx].EndpSize[inf_id][endp_id] & 0x00ff);
	td_header->Byte3 = (USB_PDevice[devinx].EndpSize[inf_id][endp_id] & 0x0300) >> 8;
	//2-3.(1)Speed
	//    (2)Endpoint Number
	endpaddr = (USB_PDevice[devinx].EndpAddr[inf_id][endp_id] & ~0x80) & 0x0f;
	endpaddr = endpaddr << 4;
	td_header->Byte3 |= endpaddr;
	if (USB_PDevice[devinx].InterfaceNum & USBHC_DEVICE_SPEED_MASK) // Low Speed Device
		td_header->Byte3 |= TD_SPEED_LOW;
	length = USB_PDevice[devinx].EndpSize[inf_id][endp_id]; 
	//2-4.(1)Total Bytes	 
	if (length > USB_HC_INTL_BUF_SIZE)
		length = USB_HC_INTL_BUF_SIZE;
	td_header->Byte4_Total_Byte = length;
	td_header->Byte5 = (length & 0x0300) >>8;
	//2-5.(1)Error Count(ingore)
	//    (2)Dir Token
	if (USB_PDevice[devinx].EndpAddr[inf_id][endp_id] & 0x80) // Interrupt IN
		td_header->Byte5 |= TD_DIR_TOKEN_IN;
	else
		td_header->Byte5 |= TD_DIR_TOKEN_OUT;
	//2-6.(1)Device Address
	td_header->Byte6_Function_Address = USB_PDevice[devinx].Addr & USBHC_DEVICE_ADDR_MASK;
	//2-7.(1)Starting frame
#if (SYSTEM_EXTENDER_RS232_MODE && SYSTEM_EXTENDER_RECEIVER)
	if (td_header->Byte5 & TD_DIR_TOKEN_IN)
	{
/* Check Special Customer KVM Setting */
		switch (USB_PDevice[devinx].idVendor)
		{
			case 0x9966:
				td_header->Byte7 = 3;
				break;
			default:
				if (USB_PDevice[devinx].EndpInterval[inf_id][endp_id] < 8)
				{	
					td_header->Byte7 = 8;
				}	
				else
				{						
					td_header->Byte7 = USB_PDevice[devinx].EndpInterval[inf_id][endp_id];
				}	
				break;	
		}						
	}
	else
	{
		td_header->Byte7 = 1;
	}
#else
	if (td_header->Byte5 & TD_DIR_TOKEN_IN)
	{
		td_header->Byte7 = USB_PDevice[devinx].EndpInterval[inf_id][endp_id]; // This is the interval
	}
	else
	{
		td_header->Byte7 = 1;
	}
#endif
	
/*
#ifdef KMLOG
	if (td_header->Byte5 & TD_DIR_TOKEN_IN)
	{
		if (td_header->Byte7 < 10)
			td_header->Byte7 = 10;		
	}
#endif
*/
	
	if ((HC_IntTransfer_Table[intt_id].Control & USBHC_INTT_CON_TOGGLE_MASK) == USBHC_INTT_CON_DATA1)
		td_header->Byte1 = TD_TOGGLE_DATA1; // the first bit
	else
		td_header->Byte1 = TD_TOGGLE_DATA0; // the first bit
#if (SYSTEM_EXTENDER_TRANSMITTER)
	if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)	//if extender, then 
	{
		td_header->Byte1 |= COMPLETEION_CODE_MASK;
		//4-1.Clear skip bit
		USBHC_CORE_Set_TD_SkipMap(USBHC_TD_INTL,td_offset,0x01); // Set the skip map
		USBHC_CORE_Set_INTL_ReloadMap(td_offset,0x00); //set with NO AutoReload
	}
	else
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */
	{

#if defined(ONEHID) || defined(SYNC)
		if (devinx == USBDC_VHID_DEVINX)
		{
			//printf("SKIP\n\r");
			return USBH_OK;
		}	
#endif		
		td_header->Byte1 |= (COMPLETEION_CODE_MASK | TD_ACTIVED_MASK);
		//4-1.Clear skip bit
		if (td_header->Byte5 & TD_DIR_TOKEN_IN)
		{	
			USBHC_CORE_Set_TD_SkipMap(USBHC_TD_INTL,td_offset,0x00); // clear the skip map		
			USBHC_CORE_Set_INTL_ReloadMap(td_offset,0x01); //set with AutoReload
		}
		else
		{
			USBHC_CORE_Set_TD_SkipMap(USBHC_TD_INTL,td_offset,0x01); // Set the skip map		
			USBHC_CORE_Set_INTL_ReloadMap(td_offset,0x00); //set with no AutoReload
		}			
	}

	return USBH_OK;
}

/**
  * @brief  USBHC_Istl_ReqData
  *         Used fixed ISTL TD id and fill up the necessary infromation
  *
  * @retval Status. 
  */
USBH_Status USBHC_Istl_ReqData(U8_T devinx,U8_T intt_id)
{
	USBHC_TD_Header_Typedef	*td_header;
	U8_T					endpaddr,inf_id,endp_id;

	//3.Fill up TD Header Conten
	td_header = (USBHC_TD_Header_Typedef *) &(HCTD_Table.ISTL[intt_id].TD);
	//3-0.Actual Byte
	td_header->Byte0_Actual_Byte = 0;
	//2-2.(1)Toggle=>Data0
	//    (2)Active=>1
	//    (3)Complete code=>0x0f
	inf_id  = HC_IsoTransfer_Table[intt_id].InterfaceIdx;
	endp_id = HC_IsoTransfer_Table[intt_id].EndpIdx & USBHC_PDEV_ENDPIDX_MASK;
	td_header->Byte2_MaxPKT_Size = (USB_PDevice[devinx].EndpSize[inf_id][endp_id] & 0x00ff);
	td_header->Byte3 = (USB_PDevice[devinx].EndpSize[inf_id][endp_id] & 0x0300) >> 8;
	//2-3.(1)Speed
	//    (2)Endpoint Number
	endpaddr = (USB_PDevice[devinx].EndpAddr[inf_id][endp_id] & ~0x80) & 0x0f;
	endpaddr = endpaddr << 4;
	td_header->Byte3 |= (endpaddr); //ISO
	if (USB_PDevice[devinx].InterfaceNum & USBHC_DEVICE_SPEED_MASK) // Low Speed Device
		td_header->Byte3 |= TD_SPEED_LOW;

	//2-5.(1)Error Count(ingore)
	//    (2)Dir Token
	if (USB_PDevice[devinx].EndpAddr[inf_id][endp_id] & 0x80) // Isochronous IN
	{
		td_header->Byte5 &= ~TD_DIR_TOKEN_MASK;
		td_header->Byte5 |= TD_DIR_TOKEN_IN;
	}
	else
	{
		td_header->Byte5 &= ~TD_DIR_TOKEN_MASK;
		td_header->Byte5 |= TD_DIR_TOKEN_OUT;
	}
	//2-6.(1)Device Address
	td_header->Byte6_Function_Address = USB_PDevice[devinx].Addr & USBHC_DEVICE_ADDR_MASK;
	
	//2-7.(1)Starting frame
	td_header->Byte7 = 1; // This is the interval
	
	// Final Step
	//2-1.(1)Toggle=>Data0
	//    (2)Active=>1
	//    (3)Complete code=>0x0f
//	if ((HC_IntTransfer_Table[intt_id].Control & USBHC_INTT_CON_TOGGLE_MASK) == USBHC_INTT_CON_DATA1)
//	{
//		td_header->Byte1 = TD_TOGGLE_DATA1; // the first bit
//	}
//	else
	{
		td_header->Byte1 = TD_TOGGLE_DATA0; // the first bit
	}
	td_header->Byte1 |= COMPLETEION_CODE_MASK;

	return USBH_OK;
}

/**
  * @brief  usbhc_Total_Byte_Fix(U8_T devinx,U16_T length)
  *         Used fixed Control TD TotalByte, must equal the
  *         max package size times factor. 
  *
  * @retval Status. 
  */
static U16_T usbhc_Total_Byte_Fix(U8_T devinx, U16_T length)
{
	U8_T	remain;
	U16_T	total_byte_fix;

	if (length <= (U16_T)USB_PDevice[devinx].Hc.Control.EpN_MaxSize)
	{
		return ((U16_T)USB_PDevice[devinx].Hc.Control.EpN_MaxSize);
	}
		
	remain = (U8_T)(length % (U16_T)USB_PDevice[devinx].Hc.Control.EpN_MaxSize);

	if (remain)
	{
		total_byte_fix = (length - remain) + USB_PDevice[devinx].Hc.Control.EpN_MaxSize;
	}
	else
	{
		total_byte_fix = length;
	}
	
	return total_byte_fix;
}

/* End of usbhc_ioreq.c */