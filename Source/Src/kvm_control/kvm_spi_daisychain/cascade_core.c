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
 * Module Name: cascade_core.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */
/*
 
*/
/* INCLUDE FILE SECTION */
/* INCLUDE FILE DECLARATIONS */
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	"project_include.h"

#if (SYSTEM_CASCADE_SUPPORT)
/*---------------------------------------------------
$ define the default BuadRate
  ---------------------------------------------------*/
#define    SPI_DEFAULT_SPEED      SPI_24M  // 4/1 of the sytem clock
/* NAMING CONSTANT DECLARATIONS */
const U16_T BIT_MASK16[]={0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080, 0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000};

/* GLOBAL VARIABLES DECLARATIONS */
bit     Cascade_KVM_State_Changed_Flag;                                                    					   	     								   	

//Handle for Data pointer
Cascade_Data_Packet_Def	*Cascade_Data_TokenP;
U8_T	*Cascade_Transmit_Send_Buf;

#if (SYSTEM_MSC_DEVICE_SUPPORT)
Cascade_MSC_Buf_TypeDef	Cascade_MSC_BulkOutBuff[CASCADE_MSC_BUF_CNT];
U8_T	Cascade_MSC_BulkOutHead;
U8_T	Cascade_MSC_BulkOutTail;
U8_T	Cascade_MSC_BulkOutCount;

Cascade_MSC_Buf_TypeDef	Cascade_MSC_BulkInBuf[CASCADE_MSC_BUF_CNT];
U8_T	Cascade_MSC_BulkInHead;
U8_T	Cascade_MSC_BulkInTail;

U8_T	Cascade_MSC_StallFlag;
U8_T	Cascade_MSC_BurstWait;
bit		Cascade_BurstDataInFlowControl;
bit		Cascade_BurstDataOutFlowControl;
#endif
/* LOCAL VARIABLES DECLARATIONS */
#if (SYSTEM_CASCADE_MASTER)
#if (SYSETM_CONSOLE_EDID_CONTROL_ENABLE)
U8_T CascadeEdidPacket[CASCADE_PACKET_HEADER_LEN] _at_ (EDID_START_ADDR-CASCADE_PACKET_HEADER_LEN);
#endif
#endif

/* LOCAL SUBPROGRAM DECLARATIONS */

/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
/* EXTERNAL SUBPROGRAM DECLARATIONS */

/*----------------------------------------------------------------------------
 * Function Name: Cascade_Core_Init
 * Purpose: initial the cascade system  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void Cascade_Core_Init(void)
{
	//U16_T	baud;
	//U8_T	*pDmaInit, regDpr;
	//printf("CASCADE INIT\n\r");
	//---------------------------------------------
	//Do the software & memory init		
	Cascade_KVM_State_Changed_Flag	  = 0;                                                    
	
#if (SYSTEM_CASCADE_MASTER)
	CascadeM_Master_Init();	
#endif /* #ifdef SYSTEM_CASCADE_MASTER */
	//printf("CASCADE SLAVE INIT\n\r");	
#if (SYSTEM_CASCADE_SLAVE)
	CascadeS_Slave_Init();	
#endif /* #ifdef SYSTEM_CASCADE_SLAVE */		 	
	
	
}

/*----------------------------------------------------------------------------------
 * U16_T Cascade_Set_Data_Transmit_Header(U16_T datalen,U8_T *buf,U8_T *data_buf)
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *-----------------------------------------------------------------------------------*/
U16_T Cascade_Set_Data_Transmit_Header(U16_T datalen,U8_T *buf,U8_T *data_buf)
{
	U16_T len;

	len = CASCADE_PACKET_HEADER_LEN+datalen;

	if (buf == NULL)
	{	
		//printf("M0_");
		Cascade_Transmit_Send_Buf = m_malloc(len,39);		
	}	
	else
		Cascade_Transmit_Send_Buf = buf;
    
	if (Cascade_Transmit_Send_Buf != NULL)
	{	
		//Copy the data content
		if (buf == NULL)
		{
			memset(Cascade_Transmit_Send_Buf,0,CASCADE_PACKET_HEADER_LEN);
		}	
		
		//Copy the data content
		if (data_buf != NULL)
		{	
			//if (datalen > CASCADE_DMA_LENGTH)				
				DMA_GrantXdata((Cascade_Transmit_Send_Buf+CASCADE_PACKET_HEADER_LEN),data_buf,datalen);
			//else				
			//	memcpy((Cascade_Transmit_Send_Buf+CASCADE_PACKET_HEADER_LEN),data_buf,datalen);
		}	
		
		Cascade_Data_TokenP = 	(Cascade_Data_Packet_Def *) Cascade_Transmit_Send_Buf;
		return len;
	}

	return 0;
}

/*----------------------------------------------------------------------------------
 * Function Name: Cascade_BuildPacket
 * Purpose:  
 * Params:
 * Returns:
 * Note:
 *-----------------------------------------------------------------------------------
 */
CASCADE_PacketHeader *Cascade_BuildPacket(U8_T cmd, U8_T req, U8_T index, U8_T value, U8_T *pbuf, U16_T len)
{
	CASCADE_PacketHeader *packet;

	packet = (CASCADE_PacketHeader*) m_malloc(CASCADE_PACKET_HEADER_LEN + len,40);		
	if (packet!=NULL)
	{	
		packet->bCommand = cmd;
		packet->iLen = len;
		packet->bRequest = req;
		packet->bIndex = index;		
		packet->bValue = value;		

		if (len)
		{	
			DMA_GrantXdata(packet->buf, pbuf, len);
		}	
	}
	return packet;
}

/*----------------------------------------------------------------------------------
 * void Cascade_Compare_State(U8_T old_value,U8_T new_value)
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *-----------------------------------------------------------------------------------*/
void Cascade_Compare_State(U8_T old_value,U8_T new_value)
{
	if (new_value)
	{
		if (old_value ==0)
			Cascade_KVM_State_Changed_Flag = 1;
	}
	else
	{
		if (old_value)
			Cascade_KVM_State_Changed_Flag = 1;
	}				
}

/*--------------------------------------------------------------------------------
 * void Set_Table_Bit(U8_T *tab,U8_T index,U8_T value)
 * Purpose :  
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------*/ 
void Set_Table_Bit(U8_T *tab,U8_T index,U8_T value)
{
	U8_T byte,targetbit;
	
	byte = index >> 3; // every byte 8 bits
	targetbit = 0x01 << (index & 0x07);
	if (value)	 // bet bit
		tab[byte] |= targetbit;
	else
		tab[byte] &= ~targetbit;
}

/*--------------------------------------------------------------------------------
 * U8_T Get_Table_Bit(U8_T *tab,U8_T index)
 * Purpose :  
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *-------------------------------------------------------------------------------*/
U8_T Get_Table_Bit(U8_T *tab,U8_T index)
{
	U8_T byte,targetbit;
	
	byte = index >> 3; // every byte 8 bits
	targetbit = 0x01 << (index & 0x07);
	return(tab[byte] & targetbit);		
} 
#endif /* SYSTEM_CASCADE_SUPPORT */
/* End of cascade_core.c */
