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
 * Module Name: usbdc_hal.c
 * Purpose: The USB DC Control & Managment program
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */
/* INCLUDE FILE SECTION 							*/
#include "string.h"
#include "project_include.h"

/* NAMING CONSTANT DECLARATIONS 					*/
/* GLOBAL VARIABLES DECLARATIONS 					*/
/* LOCAL VARIABLES DECLARATIONS 					*/
/* LOCAL SUBPROGRAM DECLARATIONS 					*/
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
/* EXTERNAL SUBPROGRAM DECLARATIONS 			*/ 

/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/
/* USB DC Hardware HAL Layer Library 
/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 * U16_T USBDC_HAL_Endp_Buf_Ptr()
 * Purpose : When Endp has get a Out transaction, then get the buffer content
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */
void USBDC_HAL_Endp_Buf_Ptr(U8_T **bufp,U8_T port,U8_T devinx,U8_T endpinx,U8_T dir)
{
	bit   double_buffer_flag;

	if (USBDC_Device[devinx].EndpAddr[endpinx] & USBDC_TABLE_DOUBF_MASK)
		double_buffer_flag = 1;
	else
		double_buffer_flag = 0;

	if (dir == USBDC_DIR_OUT)
	{
		*bufp = USBDC_EndpBufPtr[port][devinx][endpinx]+OUT_BUF_OFFSET;
	}
	else //Double Buffer
	{
		if (double_buffer_flag)
			*bufp = (USBDC_EndpBufPtr[port][devinx][endpinx]+OUT_BUF_OFFSET+USBDC_Device[devinx].EndpMaxSize[endpinx]);
		else
			*bufp = USBDC_EndpBufPtr[port][devinx][endpinx]+OUT_BUF_OFFSET;
	}

} 

/* ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 * U16_T USBDC_HAL_Endp_Buf_Length_Write()
 * Purpose : When Endp has get a Out transaction, then get the buffer content
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */
void USBDC_HAL_Endp_Buf_Length_Write(U8_T len,U8_T port,U8_T devinx,U8_T endpinx,U8_T dir)
{
	bit   double_buffer_flag;
	U8_T  *p;

	p =  USBDC_EndpBufPtr[port][devinx][endpinx];
	if (USBDC_Device[devinx].EndpAddr[endpinx] & USBDC_TABLE_DOUBF_MASK)
		double_buffer_flag = 1;
	else
		double_buffer_flag = 0;

	if (dir == USBDC_DIR_OUT)
	{
		*p = len;  //single or double buffer-byte0 is always the buffer length byte
	}
	else
	{
		if (double_buffer_flag)
			*(p+IN_BUF_LEN_OFFSET) = len;
		else
			*p = len;  //single buffer, the byte 0, is alsways buffer length byte
	}
}

/* ----------------------------------------------------------------------------
 * U16_T USBDC_HAL_Endp_Buf_Length_Read()
 * Purpose : When Endp has get a Out transaction, then get the buffer content
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
U8_T USBDC_HAL_Endp_Buf_Length_Read(U8_T port,U8_T devinx,U8_T endpinx,U8_T dir)
{
	bit   double_buffer_flag;
	U8_T  *p;
	U8_T  len = 0;

	p = USBDC_EndpBufPtr[port][devinx][endpinx];
	if (USBDC_Device[devinx].EndpAddr[endpinx] & USBDC_TABLE_DOUBF_MASK)
		double_buffer_flag = 1;
	else
		double_buffer_flag = 0;

	if (dir == USBDC_DIR_OUT)
	{
		len = *p;  //single buffer-byte0 is always the buffer length byte
	}
	else
	{
		if (double_buffer_flag)
			len = *(p+IN_BUF_LEN_OFFSET);
		else
			len = *p;  //single buffer-byte0 is always the buffer length byte
	}

	return len;
} 

/* ----------------------------------------------------------------------------
 * U16_T USBDC_HAL_Endp_In_Read(void)
 * Purpose : When Endp has get a Out transaction, then get the buffer content
 * Params  : none
 * Returns : none
 * Note    : none
 * ---------------------------------------------------------------------------- */
U16_T USBDC_HAL_Endp_Out_Buf_Copy(U8_T *buf,U8_T port,U8_T devinx,U8_T endpinx,U8_T isSetup)
{
	U16_T len,index;
	U8_T *p;

	len = USBDC_HAL_Endp_Buf_Length_Read(port,devinx,endpinx,USBDC_DIR_OUT);
	USBDC_HAL_Endp_Buf_Ptr(&p,port,devinx,endpinx,USBDC_DIR_OUT);

	if (isSetup && len > 8)
	{
		len = 8;
	}

	for (index = 0; index < len ; index++)
	{
		buf[index] = p[index];
	}

	return len;
}

/* ---------------------------------------------------------------------------- 
 * U16_T USBDC_HAL_Endp_In_Buf_Move(U8_T *buf,U8_T port,U8_T insr)
 * Purpose : When Endp has get a Out transaction, then get the buffer content
 * Params  : none
 * Returns : none
 * Note    : For Control Endp & Double Endp,The Byte0-Buffer0 Len,Byte1-Bufer1 Len
 *           For Single Endp,The first byte is always the lengt of buffer,(but still
 *           2 bytes length header)
 * ---------------------------------------------------------------------------- */
void USBDC_HAL_Endp_In_Buf_Move(U8_T *buf,U8_T len,U8_T port,U8_T devinx,U8_T endpinx)
{
	idata index;
	U8_T *p;
	
	//1.Get Device Index & Endp Index
	//2.Copy the Length Byte into IN buffer 
	USBDC_HAL_Endp_Buf_Ptr(&p,port,devinx,endpinx,USBDC_DIR_IN);
	for (index = 0; index < len ; index++)
	{
		p[index] = buf[index];
	}
	
	//3.Save Lenght into In buffer length byte
	USBDC_HAL_Endp_Buf_Length_Write(len,port,devinx,endpinx,USBDC_DIR_IN);
}

/* ---------------------------------------------------------------------------- 
 * U16_T USBDC_HAL_Address_Active(U8_T port,U8_T devinx,U8_T address)
 * Purpose : 
 * Params  : none
 * Returns : none
 * Note    : none 
 * ----------------------------------------------------------------------------*/
void USBDC_HAL_Address_Active(U8_T port,U8_T devinx,FlagStatus state)
{
	USBDC_REGS_Address_Active(port,devinx,state);
}

/*----------------------------------------------------------------------------
 * U16_T USBDC_HAL_Endp_Stalled(U8_T port,U8_T devinx,U8_T endpinx)
 * Purpose : When Endp has get a Out transaction, then get the buffer content
 * Params  : none
 * Returns : none
 * Note    : none 
 *----------------------------------------------------------------------------*/
void USBDC_HAL_Endp_Stalled(U8_T port,U8_T devinx,U8_T endpinx)
{
	USBDC_Device[devinx].Endp_Stall[port] |= (0x01 << endpinx);
	USBDC_REGS_Endp_ControlSet(port,devinx,endpinx,DA_CR_STALL_SET);
}

/*----------------------------------------------------------------------------
 * U16_T USBDC_HAL_Convert_Setup_Value
 * Purpose : convert the value,index,length into big endian byte order
 * Params  : none
 * Returns : none
 * Note    : none 
 *----------------------------------------------------------------------------*/
void USBDC_HAL_Convert_Setup_Value(U8_T *setup)
{
	U8_T temp0,temp1;
	
	temp0 = setup[2];
	temp1 = setup[3];
	setup[2] = temp1;
	setup[3] = temp0;
	
	temp0 = setup[4];
	temp1 = setup[5];
	setup[4] = temp1;
	setup[5] = temp0;
	
	temp0 = setup[6];
	temp1 = setup[7];
	setup[6] = temp1;
	setup[7] = temp0;
}

/*----------------------------------------------------------------------------
 * void USBDC_HAL_Port_Suspend
 * Purpose : Suspend the USB DC port
 * Params  : none
 * Returns : none
 * Note    : none 
 *----------------------------------------------------------------------------*/
void USBDC_HAL_Port_Suspend(U8_T port)
{
	U8_T reg;
	
	USBDC_Read_Regs(port, DCCR, &reg, 1);
	reg |= DC_CR_SUSP_SET;
	USBDC_Write_Regs(port, DCCR, &reg, 1);
}

/*----------------------------------------------------------------------------
 * void USBDC_HAL_Port_Resume
 * Purpose : Resume the USB DC port
 * Params  : none
 * Returns : none
 * Note    : none 
 *----------------------------------------------------------------------------*/
void USBDC_HAL_Port_Resume(U8_T port)
{
	U8_T reg;

	USBDC_Read_Regs(port, DCCR, &reg, 1);
	reg |= DC_CR_RESUM_SET;
	USBDC_Write_Regs(port, DCCR, &reg, 1);
}

/*----------------------------------------------------------------------------
 * void USBDC_HAL_Port_Check_SOF
 * Purpose : Check SOF received from the assigned USB DC port
 * Params  : none
 * Returns : none
 * Note    : none 
 *----------------------------------------------------------------------------*/
U8_T USBDC_HAL_Port_Check_SOF(U8_T port)
{
	U8_T reg[2];

	USBDC_Read_Regs(port, DCFMR, &reg, 2);
	
	return ((reg[1] & DC_FMR_LCK_MASK) ? 1 : 0);
}

#if (PROJECT_USB_GENERIC_HID_ENABLE)
/*----------------------------------------------------------------------------
 * void USB_HAL_Alloc_Free_VDevice
 * Purpose : get a free virtual devcie table index
 * Params  : none
 * Returns : none
 * Note    : none 
 *----------------------------------------------------------------------------*/
RESULT USB_HAL_Alloc_Free_VDevice(U8_T *devinx)
{
	U8_T index;
	
	for (index=0; index < USBDC_DEVICE_MAX ; index++)
	{
		if ((USBDC_Device[index].DevIdx & DC_USED_MASK) == 0)
		{
			*devinx = index;
			memset(&USBDC_Device[index],0x00,sizeof(USBDC_DeviceTypeDef));
			USBDC_Device[index].DevIdx |= DC_USED_MASK;
			return USB_SUCCESS;
		}
	}
	
	return USB_ERROR;
}
#endif /* PROJECT_USB_GENERIC_HID_ENABLE */

/*----------------------------------------------------------------------------
 * U16_T USBDC_HAL_Malloc_Device_Endpx
 * Purpose : Init the Endp Buffer & relative Registers
 * Params  : none
 * Returns : none
 * Note    : none 
 *----------------------------------------------------------------------------*/
void USBDC_HAL_Malloc_Device_Endpx(U8_T devinx,U8_T endpinx,U8_T enpdaddr,U8_T endpatri,U16_T maxpackzie)
{
	U8_T  up;
	U8_T  *bf;
	U16_T bf_len;
	U32_T iaddr;
	
	//1.Caculate Data buffer length
	if (endpatri & USBDC_TABLE_DOUBF_MASK)
	{
		bf_len = (maxpackzie << 1) + 2; // for Control 
		
		//Only work with DC can assign the iso buffer into HC TD Buffer
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
		if (endpatri & USBDC_TABLE_ISO_MASK)
		{
			bf_len += 8 ; // for ISO In & Out Endpoint buffer
		}
#endif
	}
	else
		bf_len = maxpackzie + 2; // for Control 

	//3.Malloc buffer 
	USBDC_Device[devinx].EndpMaxSize[endpinx]     = maxpackzie;	  
	USBDC_Device[devinx].EndpAddr[endpinx]        = (enpdaddr & 0x0f) | endpatri;   // Control Endpoint Buffer0
	
	//3.Fill up USBDC Index Table & Register
	for (up=0; up < USBDC_PORT_MAX; up++)
	{
		//2.Malloc buffer 
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
		if (endpatri & USBDC_TABLE_ISO_MASK)
		{
			if (endpatri & USBDC_TABLE_DIR_IN_MASK) // IN
			{
				bf =  HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].Buf;
				USBHC_Audio_In_Endpinx = endpinx;
				USBHC_Audio_IN_Flag = 1; //inform the audio in
			}
			else // OUT
			{
#if (SYSTEM_EXTENDER_TRANSMITTER)
				// HC port-0
				bf = ETDR_AudioOutBuf[0] + sizeof(Extender_Data_Packet_Def);
				ETDR_AudioOutHead = 0;
				ETDR_AudioOutTail = 0;
				ETDR_AudioOutTailSkipFlag = 0;
				ETDR_AudioInHead = 0;
				ETDR_AudioInTail = 0;
				ETDR_AudioInDcDoingFlag = 0;
#else
				bf =  HCTD_Table.ISTL[0].Buf;
#endif
				USBHC_Audio_Out_Endpinx = endpinx;
			}
		}
		else
#endif
		{
#if ((SYSTEM_MSC_DEVICE_SUPPORT) || (SYSTEM_USB_PEN_DRIVE_SUPPORT) || (SYSTEM_EXTENDER_MSC_SUPPORT))
			if (USB_PDevice[devinx].DevClass == USB_MSC_CLASS)
			{
				bf = 0;
#if (SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT)
#if (SYSTEM_USB_HC_BURST)
#if (SYSTEM_EXTENDER_TRANSMITTER)
				if (devinx == EXTENDER_MSC_Devinx)
				{
					if (up > 0)
					{
						bf = USBDC_EndpBufPtr[0][devinx][endpinx];
					}
					else
					{
						if ((USBDC_Device[devinx].EndpType[endpinx] & USB_EP_TYPE_MASK) == USB_EP_TYPE_BULK) // enable the Burst Mode endpoint
						{
							// For Bulk Burst Mode
							if (endpatri & USBDC_TABLE_DIR_IN_MASK) // IN
							{
								USB_PDevice[devinx].MSC->In_Endpinx  = endpinx;
								USB_PDevice[devinx].MSC->In_EndpAddr = enpdaddr;
								bf = EXTENDER_MSC_BulkInBuf[0].bBuf;
							}
							else // OUT
							{
								USB_PDevice[devinx].MSC->Out_Endpinx = endpinx;
								USB_PDevice[devinx].MSC->Out_EndpAddr = enpdaddr;
								bf = EXTENDER_MSC_BulkOutBuf[0].bBuf;
							}
							//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
							//indicate Burst Mode used
							USBDC_Device[devinx].EndpAddr[endpinx] |= USBDC_TABLE_BURST_MASK; //indicate the burst mode
							//End of Burst Mode used
							//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
							//Fill Up the length for Burst Mode
							bf[0] = (U8_T)(EXTENDER_MSC_BURST_SIZE & 0x00FF);
							bf[1] = (U8_T)(EXTENDER_MSC_BURST_SIZE >> 8);
						}
						else
						{
							// not for Bulk transfer
							bf = m_malloc(bf_len,21);
						}
					}
				}
#else //For (SYSTEM_EXTENDER_RECEIVER)
				if (devinx == USBHC_MSC_Devinx)
				{
					if (up > 0)
					{
						bf = USBDC_EndpBufPtr[0][devinx][endpinx];
					}
					else
					{
						if ((USBDC_Device[devinx].EndpType[endpinx] & USB_EP_TYPE_MASK) == USB_EP_TYPE_BULK) // enable the Burst Mode endpoint
						{
							// For Bulk Burst Mode
							if (endpatri & USBDC_TABLE_DIR_IN_MASK) // IN
							{
								USBHC_MSC_PingPongIn_ID = USB_HC_MSC_BURST_IN_TD0; // start from first In TD
								USB_PDevice[devinx].MSC->In_Endpinx  = endpinx;
								USB_PDevice[devinx].MSC->In_EndpAddr = enpdaddr;
								bf = HCTD_Table.BULK[USB_HC_MSC_BURST_IN_TD0 - USB_HC_MSC_START].Buf;
							}
							else // OUT
							{
								USBHC_MSC_PingPongOut_ID = USB_HC_MSC_BURST_OUT_TD0; // start from first Out TD
								USB_PDevice[devinx].MSC->Out_Endpinx = endpinx;
								USB_PDevice[devinx].MSC->Out_EndpAddr = enpdaddr;
								bf = HCTD_Table.BULK[USB_HC_MSC_BURST_OUT_TD0 - USB_HC_MSC_START].Buf;
							}
							//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
							//indicate Burst Mode used
							USBDC_Device[devinx].EndpAddr[endpinx] |= USBDC_TABLE_BURST_MASK; //indicate the burst mode
							//End of Burst Mode used
							//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
							//Fill Up the length for Burst Mode
							bf[0] = USB_HC_BULK_BLK_SIZE_LSB;
							bf[1] = USB_HC_BULK_BLK_SIZE_MSB;
						}
						else
						{
							// not for Bulk transfer
							bf = m_malloc(bf_len,22);
						}
					}
				}
#endif //#if (SYSTEM_EXTENDER_TRANSMITTER)
#else // Non (SYSTEM_USB_HC_BURST)
#if (SYSTEM_EXTENDER_TRANSMITTER)
				if (devinx == EXTENDER_MSC_Devinx)
#else
				if (devinx == USBHC_MSC_Devinx)
#endif
				{
					if (up > 0)
					{
						bf = USBDC_EndpBufPtr[0][devinx][endpinx];
					}
					else
					{
						bf = m_malloc(bf_len,23);
					}
				}
#endif /*(SYSTEM_USB_HC_BURST)*/
#endif //#if (SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT)
	
#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
				if (devinx == USBDC_VMSC_DEVINX)
				{
					if (up > 0)
					{
						bf = USBDC_EndpBufPtr[0][devinx][endpinx];
					}
					else
					{
						bf = m_malloc(bf_len,24);
					}
				}

#endif //End of (SYSTEM_USB_PEN_DRIVE_SUPPORT)
			}
			else
			{
				bf = m_malloc(bf_len,25);
			}
#else
			bf  = m_malloc(bf_len,26);
#endif // End of (SYSTEM_MSC_DEVICE_SUPPORT)  || (SYSTEM_USB_PEN_DRIVE_SUPPORT)
		}
		
		//3-1 Fill up index table
		if (bf != NULL)
		{
			iaddr = bf;
			USBDC_Index[up][devinx][endpinx].Endp_Addr_Lsb = iaddr & 0x00ff;  // endpoint0 address
			USBDC_Index[up][devinx][endpinx].Endp_Addr_Msb = (iaddr & 0xff00) >> 8;  // endpoint0 address
			USBDC_EndpBufPtr[up][devinx][endpinx] = bf;
			
#if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST == 0))
			if ((up == 0) && (USB_PDevice[devinx].DevClass == USB_MSC_CLASS))
			{
				if ((USBDC_Device[devinx].EndpType[endpinx] & USB_EP_TYPE_MASK) == USB_EP_TYPE_BULK) // enable the endpoint
				{
					if (endpatri & USBDC_TABLE_DIR_IN_MASK) // IN
					{
						iaddr = bf+2;
						USB_PDevice[devinx].MSC->In_Buf[0].lsb = iaddr & 0x00ff;
						USB_PDevice[devinx].MSC->In_Buf[0].msb = (iaddr & 0xff00) >> 8;
						
						iaddr = bf+maxpackzie+2;
						USB_PDevice[devinx].MSC->In_Buf[1].lsb = iaddr & 0x00ff;
						USB_PDevice[devinx].MSC->In_Buf[1].msb = (iaddr & 0xff00) >> 8;
						
						USB_PDevice[devinx].MSC->In_Len[0] = bf;
						USB_PDevice[devinx].MSC->In_Len[1] = bf+1;
						USB_PDevice[devinx].MSC->In_Endpinx= endpinx;
						USB_PDevice[devinx].MSC->In_EndpAddr= enpdaddr;
					}
					else // OUT
					{
						iaddr = bf+2;
						USB_PDevice[devinx].MSC->Out_Buf[0].lsb = iaddr & 0x00ff;;
						USB_PDevice[devinx].MSC->Out_Buf[0].msb = (iaddr & 0xff00) >> 8;
						
						iaddr = bf+maxpackzie+2;
						USB_PDevice[devinx].MSC->Out_Buf[1].lsb = iaddr & 0x00ff;
						USB_PDevice[devinx].MSC->Out_Buf[1].msb = (iaddr & 0xff00) >> 8;
						
						USB_PDevice[devinx].MSC->Out_Len[0] = bf;
						USB_PDevice[devinx].MSC->Out_Len[1] = bf+1;
						USB_PDevice[devinx].MSC->Out_Endpinx= endpinx;
						USB_PDevice[devinx].MSC->Out_EndpAddr= enpdaddr;
					}
				}
			}
#endif // End of (SYSTEM_MSC_DEVICE_SUPPORT)
		}
		//else
		//{
		//	printf("!!! ERROR !!!==>Memory Alloc Error,DevInx:%d,Port:%d,Endpinx:%d \n\r",(U16_T)devinx,(U16_T)up,(U16_T)endpinx);
		//}
	}
}

/* End of usbdc_hal.c */
