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
 * Module Name: cascade_slave.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */

/* INCLUDE FILE SECTION */
/* INCLUDE FILE DECLARATIONS */
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	"project_include.h"

#if (SYSTEM_CASCADE_SLAVE)
/* NAMING CONSTANT DECLARATIONS */
#define   CASCADES_RX_BUFFER_START_ADDRESS 		0x1000
#define   CASCADES_TX_BUFFER_START_ADDRESS 		(CASCADES_RX_BUFFER_START_ADDRESS+CASCADES_RX_BUF_MAX+1)

/* GLOBAL VARIABLES DECLARATIONS */

bit	Slave_PassThrough_Buf_In_Used_Flag;
bit CascadeS_KVM_State_Changed_Flag = 0;
bit CascadeS_Version_Report_Flag;

//Define the receiver ring buffer start address must start 0xXX00~0xYYFF
U8_T	CascadeS_Rx_Buf[CASCADES_RX_BUF_MAX+1]  _at_ CASCADES_RX_BUFFER_START_ADDRESS;
//Define the transmit ring buffer start address must start 0xXX00~0xYYFF
U8_T	CascadeS_Tx_Buf[CASCADES_TX_BUF_MAX+1]  _at_ CASCADES_TX_BUFFER_START_ADDRESS;

Firmware_Version_Def  FirmwareVersion;

CCSLV_CTRL CCSLV_Ctrl;

U8_T   	Slave_Control_State; 
U8_T   	CascadeS_Control_State;

Cascade_Device_Mapping_Def  Remote_DevMap[USBDC_DEVICE_MAX];
U8_T    Remote_Reserve_DevMap[USBDC_DEVICE_MAX];	
U8_T	TASK_USBHC_CascadeS_PassThrough_Handle_ID; 
U8_T  	Slave_Device_Passthrough_Flag;						//each bit standard 1 for device
U8_T  	Slave_Device_Passthough_State[USBDC_DEVICE_MAX];

//For Rx Ring Buffer Handle
//U16_T  	CascadeS_Rx_Buf_Startp;
//U16_T  	CascadeS_Rx_Buf_Data_Startp;
//U16_T  	CascadeS_Rx_Buf_Endp;
//U16_T  	CascadeS_Rx_Buf_Length;
//U16_T  	CascadeS_Rx_Buf_Available_Count;

//For Tx Ring Buffer Handle
//U16_T  	CascadeS_Tx_Buf_Startp;
//U16_T  	CascadeS_Tx_Buf_Endp;
//U16_T  	CascadeS_Tx_Buf_Length;
//U16_T  	CascadeS_Tx_Buf_Available_Count;

//U8_T  	CascadeS_Page[CASCADES_PAGE_LENGTH]; //
U8_T  	CascadeS_Rx_Buf_Last_Avail=USBDC_DEVICE_MAX;
//U16_T  	CascadeS_Rx_Len,CascadeS_Tx_Len;
//U16_T  	Receive_BufferP;

CascadeS_System_State	CascadeS_KVM_State;

//For SPI slave Transmit fifo table handle
Cascade_Transimit_Def Trainsmit_FIFO[CASCADE_TRANSMIT_FIFO_MAX];
U8_T 	CascadeS_Transmit_OutP;
U8_T 	CascadeS_Transmit_InP;

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
#if (SYSTEM_CASCADE_AUDIO_BUF_CNT)
/*------------------------------------------------------------------------------------
$This is for support audio device, for none HC case:
 [1].The DC Endpoint buffer will assign to a struct table, which contain 2 section.
     A.Output Section => start from index 0, 
     B.Input  Section => start form index 3
     The total payload for memory useage will be:(192+6)*6=1188
*/
//volatile U8_T		 CADR_USB_CascadeS_Buffer_Out; //transmit index
Cascade_ISO_Data_Def CADR_USB_Audio_Buf[SYSTEM_CASCADE_AUDIO_BUF_CNT];
volatile U8_T		 CADR_USB_CascadeS_Audio_Inx; //transmit index
volatile U8_T		 CADR_USB_CascadeS_Audio_Outx;
volatile U8_T		 CADR_USB_DC_Audio_Inx; //transmit index
volatile U8_T		 CADR_USB_DC_Audio_Outx;
//volatile U8_T		 CADR_USB_DC_Buffer_In;
U8_T	             CADR_USB_AudioBufAddr[SYSTEM_CASCADE_AUDIO_BUF_CNT][2];
U8_T	USBDC_Audio_In_Interface;
U8_T	USBDC_Audio_Out_Interface;
bit		CascadeS_ISO_DC_IN_Sending_Flag;		
#if (CASCADES_AUDIO_BUFFING_MODE)
bit		CascadeS_AudioIn_Buffering_Start_Flag;
#endif /* #if (CASCADEM_AUDIO_BUFFING_MODE) */

/*    FOR NONE HC DESIGNED    */
#endif /* #if (SYSTEM_CASCADE_AUDIO_BUF_CNT) */
#endif /* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */

/* LOCAL VARIABLES DECLARATIONS */
static U8_T CCSLV_PACKET_BUF[CASCADES_PAGE_LENGTH];

/* LOCAL SUBPROGRAM DECLARATIONS */
U8_T CascadeS_Retrive_Port(U8_T mountport);
void CascadeS_KVM_Led_Control_Handle(CASCADE_PacketHeader *dp);
void CascadeS_API_Device_Config_Setting_Handle(CASCADE_PacketHeader *Data_Token);
void CascadeS_Virtual_Device_Suspend_Check(void);
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
/* EXTERNAL SUBPROGRAM DECLARATIONS */

/*
 *--------------------------------------------------------------------------------
 * Function Name: CascadeS_Slave_Init()
 * Purpose : 
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */ 
void CascadeS_Slave_Init(void)
{
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	U16_T reg16b;
	U8_T  reg8p;
#endif	
	
	memset(&CCSLV_Ctrl, 0, sizeof(CCSLV_CTRL));
	
	CASCADE_SLAVE_INPUT_GPIO = 1;
	Slave_Control_State = 0;		
	memset(&CascadeS_KVM_State,0x00,sizeof(CascadeS_KVM_State));
	CascadeS_Transmit_OutP  = 0;
	CascadeS_Transmit_InP   = 0;	
	CascadeS_Version_Report_Flag = 0;

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
#if (SYSTEM_CASCADE_AUDIO_BUF_CNT)	
	CADR_USB_DC_Audio_Inx  			 = SYSTEM_CASCADE_AUDIO_IN_START;
	CADR_USB_DC_Audio_Outx 			 = SYSTEM_CASCADE_AUDIO_OUT_START;	
	CADR_USB_CascadeS_Audio_Inx  	 = SYSTEM_CASCADE_AUDIO_IN_START;; 
    CADR_USB_CascadeS_Audio_Outx 	 = SYSTEM_CASCADE_AUDIO_OUT_START;;    
	//CADR_USB_CascadeS_Buffer_Out = 0;
	//CADR_USB_DC_Buffer_In		 = 0;
	for (reg8p = 0; reg8p < SYSTEM_CASCADE_AUDIO_BUF_CNT; reg8p++)
	{
		//Get buffer address		
		reg16b = (U16_T)(CADR_USB_Audio_Buf[reg8p].Buf);
		CADR_USB_AudioBufAddr[reg8p][0] = (U8_T)reg16b;
		CADR_USB_AudioBufAddr[reg8p][1] = (U8_T)(reg16b >> 8);
	}	
	CascadeS_AudioIn_Buffering_Start_Flag = 0;
	CascadeS_ISO_DC_IN_Sending_Flag = 0;
#endif /* #if (SYSTEM_CASCADE_AUDIO_BUF_CNT) */
#endif /* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */	
	
	memset((U8_T *)&Remote_DevMap,0x00,sizeof(Remote_DevMap));
	memset(Remote_Reserve_DevMap ,0x00,sizeof(Remote_Reserve_DevMap));
	//memset(CascadeS_Device_Passthough_State,0x00,sizeof(CascadeS_Device_Passthough_State));
	
	/* Link the Slave Virtual HUB with remote Root HUB */
	Remote_DevMap[USBDC_VHUB_DEVINX].Mapping   = 0x00;
	//Remote_DevMap[USBDC_VHUB_DEVINX].UpperInfo = USBDC_VHUB_PORT_NUM; //hub port counter 7
	Remote_Reserve_DevMap[USBDC_VHUB_DEVINX]   = USBDC_VHUB_DEVINX;  //the first is transmitter virutal root hub

#if (PROJECT_USB_GENERIC_HID_ENABLE)
	/* Link the Slave Virtual HUB with remote Root HUB */
	Remote_DevMap[USBDC_VHID_DEVINX].Mapping   = USBDC_VHID_DEVINX;
	//Remote_DevMap[USBDC_VHUB_DEVINX].UpperInfo = USBDC_VHUB_PORT_NUM; //hub port counter 7
	Remote_Reserve_DevMap[USBDC_VHID_DEVINX]   = USBDC_VHID_DEVINX;  //the first is transmitter virutal root hub
#endif /*#if ((PROJECT_USB_GENERIC_HID_ENABLE) */

	CascadeS_PassThrough_Buf_In_Used_Flag = 0;
	//Tansmitter_PassThrough_Buf_In_Used_Devinx = 0;
	//TASK_USBHC_CascadeS_PassThrough_ID				= TASK_Create(TASK_USBHC_CascadeS_PassThrough);
	//TASK_CascadeS_Devcie_Rereport_TASK.Task_ID		= TASK_Create(TASK_CascadeS_Devcie_Rereport);
	
	SPI_SlvModeSetup(SPICSR_SLV_ENB, SPISISR_SSTDC_CPL | SPISISR_SSRDC_CPL, 0, SPIS_24M);
	SPI_SetSlvDmaRingSetup(&CascadeS_Tx_Buf[0], &CascadeS_Tx_Buf[CASCADES_TX_BUF_MAX], &CascadeS_Rx_Buf[0], &CascadeS_Rx_Buf[CASCADES_RX_BUF_MAX]);
	SPI_SlvDmaTrigLevelSetting(0, 0, 0xff);
	SPI_SlvStart();

	printf("CCSLV: Set TX_SADDR=0x%08lx, TX_EADDR=0x%08lx, RX_SADDR=0x%08lx, RX_EADDR=0x%08lx\r\n"
		, (U32_T)&CascadeS_Tx_Buf[0]
		, (U32_T)&CascadeS_Tx_Buf[CASCADES_TX_BUF_MAX]
		, (U32_T)&CascadeS_Rx_Buf[0]
		, (U32_T)&CascadeS_Rx_Buf[CASCADES_RX_BUF_MAX]);

	//Do the KVM state transmit to host for identify  	
	CascadeS_Transmit_KVM_Version();
	CascadeS_Transmit_KVM_State(CascadeS_KVM_State.Connect_State, CascadeS_KVM_State.Power_State, CascadeS_KVM_State.Suspend_State);
	
#if (SYSTEM_MSC_DEVICE_SUPPORT)
	CascadeS_MSC_Init();
#endif	

#if (SYSTEM_KVM_BOX_CASCADE_SUPPORT)
	CascadeBoxS_Init();
#endif	
} 

/*
 *--------------------------------------------------------------------------------
 * Function Name: Cascades_Receive_Handle()
 * Purpose : After receiver from spi master, and DMA into receiver buffer
 *           handle the receiver buffer.
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */ 
void CascadeS_Receive_Handle(void)
{
	bit	skip_read=0;
	U16_T packet_len, rx_data_len, tmp16, rdptr;	
	CASCADE_PacketHeader *packet = CCSLV_PACKET_BUF;	
	
	/* Check RX data length */
	rx_data_len = SPI_GetSlvRxDmaAvailCount();
	while (rx_data_len >= CASCADE_PACKET_HEADER_LEN)
	{
		/* Dequeue packet header from RX buffer ring */	
		SPI_SlvRegRead(SRBRP, (U8_T*)&tmp16, 2);
		rdptr = tmp16 - CASCADES_RX_BUFFER_START_ADDRESS;
		for (tmp16=0; tmp16<CASCADE_PACKET_HEADER_LEN; tmp16++)
		{
			CCSLV_PACKET_BUF[tmp16] = CascadeS_Rx_Buf[rdptr++];
			if (rdptr > CASCADES_RX_BUF_MAX)
				rdptr = 0;
		}	
#if CCSLV_DEBUG_MODE	
		printf("CCSLV: RX=%02bx-%u-%02bx-%02bx-%02bx\r\n"
			, packet->bCommand
			, packet->iLen
			, packet->bRequest
			, packet->bIndex
			, packet->bValue
			);
#endif
		packet_len = CASCADE_PACKET_HEADER_LEN + packet->iLen;
		if (packet_len > rx_data_len)
		{			
			printf("CCSLV: RX partial data, len=%d/%d\r\n", rx_data_len, packet_len);
			printf("CCSLV: RX=%02bx-%u-%02bx-%02bx-%02bx\r\n"
			, packet->bCommand
			, packet->iLen
			, packet->bRequest
			, packet->bIndex
			, packet->bValue
			);
						
			if (packet->bCommand == 0)
			{				
				rx_data_len -= CASCADE_PACKET_HEADER_LEN;							
				if ((rdptr + rx_data_len) > CASCADES_RX_BUF_MAX)
				{
					tmp16 = CASCADES_RX_BUF_MAX - rdptr + 1;
					rdptr = rx_data_len;								
				}					
				else
				{
					rdptr += rx_data_len;				
				}		
				tmp16 = rdptr + CASCADES_RX_BUFFER_START_ADDRESS;
				SPI_SlvRegWrite(SRBRP, (U8_T*)&tmp16, 2);								
			}					
			return;
		}		
		
		/* Dequeue payload from RX buffer ring */
		if (packet->iLen)
		{
			if ((rdptr + packet->iLen) > CASCADES_RX_BUF_MAX)
			{
				tmp16 = CASCADES_RX_BUF_MAX - rdptr + 1;
				DMA_GrantXdata(packet->buf, &CascadeS_Rx_Buf[rdptr], tmp16);
				DMA_GrantXdata(packet->buf+tmp16, &CascadeS_Rx_Buf[0], packet->iLen-tmp16);
				rdptr = packet->iLen-tmp16;								
			}
			else
			{
				DMA_GrantXdata(packet->buf, &CascadeS_Rx_Buf[rdptr], packet->iLen);
				rdptr += packet->iLen;				
			}
			
#if CCSLV_DEBUG_MODE	
			printf("CCSLV: RX=%08lx\r\n", *((U32_T*)packet->buf));	
#endif			
		}			
		tmp16 = rdptr + CASCADES_RX_BUFFER_START_ADDRESS;
		SPI_SlvRegWrite(SRBRP, (U8_T*)&tmp16, 2);
		rx_data_len-= packet_len;
							
		/* Cascade commands decode */
		if (packet->bCommand <= CASCADE_SYSTEM_CMD)
		{		
			CascadeS_Receive_System_Handle(packet);
		}
		else if (packet->bCommand <= CASCADE_USB_CMD)
		{
			CascadeS_Receive_USB_Handle(packet);
		}
		else if	(packet->bCommand <= CASCADE_KVM_CMD)
		{		
			CascadeS_Receive_KVM_Handle(packet);			
		}
		else
		{
			printf("CCSLV: RX unsupported command(0x%02bx)...\r\n", packet->bCommand);	
			Disp_Str((U8_T*)packet, packet_len);
		}
	}
} /* End of CascadeS_Receive_Handle() */

/*
 *--------------------------------------------------------------------------------
 * Function Name: CascadeS_Transfer_Complete_Handle()
 * Purpose : None.
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */  
void CascadeS_Transfer_Complete_Handle(U8_T spis_state)
{	
	/* SPI slave TX DMA complete handling */
	if (spis_state & SPISISR_SSTDC_CPL)
	{
		CASCADE_SLAVE_INPUT_GPIO = 1;	
		if (SPI_GetSlvTxDmaVacantCount() != CASCADES_TX_BUF_MAX)
		{	
			CASCADE_SLAVE_INPUT_GPIO = 0;
		}
#if 0//CCSLV_DEBUG_MODE		
		printf("CCSLV: TX done, Ready pin=%bu\r\n", (U8_T)CASCADE_SLAVE_INPUT_GPIO);
#endif		
	}
	
	/* SPI slave RX DMA complete handling */	
	if (spis_state & SPISISR_SSRDC_CPL)
	{
		CascadeS_Receive_Handle();
	}
} /* End of CascadeS_Transfer_Complete_Handle() */

/*
 *--------------------------------------------------------------------------------
 * Function Name: CascadeS_Transmit_FIFO_Send()
 * Purpose : None.
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void CascadeS_Transmit_FIFO_Send(void)
{
	U16_T tmp16, wrptr;
	U8_T tmp8;
#if CCSLV_DEBUG_MODE		
	CASCADE_PacketHeader *packet;
#endif		
	/* Wating for TX buffer is idle */
	SPI_SlvRegRead(SSSR, &tmp8, 1);
	if (tmp8 & SSSR_SSDTIP_DMATX)
	{
		return;
	}

	/* Check TX available memory space */
	if (Trainsmit_FIFO[CascadeS_Transmit_OutP].DataLen >= SPI_GetSlvTxDmaVacantCount())
	{
		return;
	}

	/* Eequeue packet into TX buffer ring */
	SPI_SlvRegRead(STBWP, (U8_T*)&tmp16, 2);
	wrptr = tmp16 - CASCADES_TX_BUFFER_START_ADDRESS;
	if ((wrptr + Trainsmit_FIFO[CascadeS_Transmit_OutP].DataLen) > CASCADES_TX_BUF_MAX)
	{
		tmp16 = CASCADES_TX_BUF_MAX - wrptr + 1;
		DMA_GrantXdata(&CascadeS_Tx_Buf[wrptr], Trainsmit_FIFO[CascadeS_Transmit_OutP].Buf, tmp16);
		DMA_GrantXdata(&CascadeS_Tx_Buf[0], Trainsmit_FIFO[CascadeS_Transmit_OutP].Buf+tmp16, Trainsmit_FIFO[CascadeS_Transmit_OutP].DataLen-tmp16);
		wrptr = Trainsmit_FIFO[CascadeS_Transmit_OutP].DataLen - tmp16;
	}
	else
	{
		DMA_GrantXdata(&CascadeS_Tx_Buf[wrptr], Trainsmit_FIFO[CascadeS_Transmit_OutP].Buf, Trainsmit_FIFO[CascadeS_Transmit_OutP].DataLen);
		wrptr += Trainsmit_FIFO[CascadeS_Transmit_OutP].DataLen;	
	}
#if CCSLV_DEBUG_MODE		
	packet = (CASCADE_PacketHeader*)Trainsmit_FIFO[CascadeS_Transmit_OutP].Buf;
	printf("CCSLV: TX[%bu]=%02bx-%u-%02bx-%02bx-%02bx\r\n"
		, CascadeS_Transmit_OutP
		, packet->bCommand
		, packet->iLen
		, packet->bRequest
		, packet->bIndex
		, packet->bValue
		);
		
	if (packet->iLen)
	{
		printf("CCSLV: TX[%bu]=%08lx\r\n", CascadeS_Transmit_OutP, *((U32_T*)packet->buf));	
	}
#endif	
	tmp16 = wrptr + CASCADES_TX_BUFFER_START_ADDRESS;
	SPI_SlvRegWrite(STBWP, (U8_T*)&tmp16, 2);
	
	if ((Trainsmit_FIFO[CascadeS_Transmit_OutP].Control & CASCADE_CONTROL_NONE_FREE) == 0x00)
	{
		malloc_free(Trainsmit_FIFO[CascadeS_Transmit_OutP].Buf);
	}
	
	if (Trainsmit_FIFO[CascadeS_Transmit_OutP].Control & CASCADE_CONTROL_CLEAR_BUF) //need to handle the burst out handle
	{
		CascadeS_MSC_BurstOutComplete();
	}	
	CascadeS_Transmit_OutP++;
	if (CascadeS_Transmit_OutP >= CASCADE_TRANSMIT_FIFO_MAX)
		CascadeS_Transmit_OutP = 0;
	
	/* Notify the cascade master to read data */
	if (CASCADE_SLAVE_INPUT_GPIO)
	{
		CASCADE_SLAVE_INPUT_GPIO = 0;
#if 0//CCSLV_DEBUG_MODE			
		printf("CCSLV: Notify master!\r\n");
#endif		
	}
} /* End of CascadeS_Transmit_FIFO_Send() */

/*----------------------------------------------------------------------------
 * Function Name: CascadeS_Send_Transmit
 * Purpose: Send out the transmit
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeS_Send_Transmit(U8_T *buf,U8_T contrl_attr,U16_T len)
{			 		
	bit isr_tmp;
	
	isr_tmp = EINT3;
	EINT3 = OFF;
	
	Trainsmit_FIFO[CascadeS_Transmit_InP].Control  = contrl_attr;	
	Trainsmit_FIFO[CascadeS_Transmit_InP].DataLen  = len; // 3 bytes for transmitter
	Trainsmit_FIFO[CascadeS_Transmit_InP].Buf      = buf;	
	//Trainsmit_FIFO[CascadeS_Transmit_InP].ErrCnt   = 0;
	//Trainsmit_FIFO[CascadeS_Transmit_InP].TimeOut  = timeout; //10ms time out
	
	//Add the FIFO pointer
	CascadeS_Transmit_InP++;
	if (CascadeS_Transmit_InP >= CASCADE_TRANSMIT_FIFO_MAX)
		CascadeS_Transmit_InP = 0;
	
	EINT3 = isr_tmp;		
}

/*
 *--------------------------------------------------------------------------------
 * Function Name: CascadeS_Transmit_KVM_State()
 * Purpose : None.
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void CascadeS_Transmit_KVM_State(U8_T connect_status, U8_T power_status, U8_T suspend_status)
{
	CASCADE_PacketHeader *packet;
	
	packet = Cascade_BuildPacket(CASCADE_KVM_STATE, connect_status, power_status, suspend_status, NULL, 0);
	if (packet)
	{
		CascadeS_Send_Transmit((U8_T*)packet, 0, CASCADE_PACKET_HEADER_LEN + packet->iLen);
		//printf("SLAVE_STATE:");
		//Disp_Str((U8_T*)packet,CASCADE_PACKET_HEADER_LEN + packet->iLen);
	}	
	Cascade_KVM_State_Changed_Flag = 0;
}/* End of CascadeS_Transmit_KVM_State */

/*----------------------------------------------------------------------------
 * Function Name: CascadeS_Transmit_KVM_Version
 * Purpose: Send out the KVM version information
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeS_Transmit_KVM_Version(void)
{
	U8_T  buf[15];	
	CASCADE_PacketHeader *packet;
	
    memcpy(&buf[0],VERSION_STRING,5);
	memcpy(&buf[5],DATE_STRING,10);
	packet = Cascade_BuildPacket(CASCADE_SC_VERSION, 0, 0, 0, buf, 15);
	if (packet)
	{
		CascadeS_Send_Transmit((U8_T*)packet, 0, CASCADE_PACKET_HEADER_LEN + packet->iLen);
	}			
}

/*
 *--------------------------------------------------------------------------------
 * Function Name: CascadeS_KVM_ReportButtonStatus()
 * Purpose : None.
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void CascadeS_KVM_ReportButtonStatus(U8_T upstream_port, U8_T button_status)
{
	CASCADE_PacketHeader *packet;
	
	packet = Cascade_BuildPacket(CASCADE_KVM_BTN, upstream_port, upstream_port, button_status, NULL, 0);
	if (packet)
	{	
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
		USBDC_INT_DISABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
		CascadeS_Send_Transmit((U8_T*)packet, 0, CASCADE_PACKET_HEADER_LEN + packet->iLen);
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
		USBDC_INT_ENABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
#if 0//CCSLV_DEBUG_MODE	
		printf("ReportButtonStatus[%bu][%bu][%bu], ready=%bu-%bu\n\r", slave_id, upstream_port, button_status, (U8_T)KVM_System_Not_Ready_Flag, (U8_T)CASCADE_SLAVE_INPUT_GPIO);
#endif		
	}
//	printf("packet=%08lx\r\n", (U32_T)packet);	
}/* End of CascadeS_KVM_ReportButtonStatus */

/*
 *--------------------------------------------------------------------------------
 * Function Name: CascadeS_Transmit_KVM_KB_Led()
 * Purpose : None.
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void CascadeS_Transmit_KVM_KB_Led(U8_T port, U8_T led)
{
//	U16_T  len;
	
//	len = Cascade_Set_Data_Transmit_Header(0,NULL,NULL);	 
//	if (len == 0)
//	{	
//		return;
//	} 
	
//	Cascade_Data_TokenP->bCommand = CASCADE_KVM_KB_LED;	
//	Cascade_Data_TokenP->bIndex	= port;	
//	Cascade_Data_TokenP->bValue	= led;	
//#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
//	USBDC_INT_DISABLE;
//#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
//	CascadeS_Send_Transmit(Cascade_Transmit_Send_Buf,0,len);
//#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
//	USBDC_INT_ENABLE;
//#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	

	CASCADE_PacketHeader *packet;
	
	packet = Cascade_BuildPacket(CASCADE_KVM_KB_LED, 0, port, led, NULL, 0);
	if (packet)
	{
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
		USBDC_INT_DISABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
		CascadeS_Send_Transmit((U8_T*)packet, 0, CASCADE_PACKET_HEADER_LEN + packet->iLen);
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
		USBDC_INT_ENABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */		
	}
}/* End of CascadeS_Transmit_KVM_KB_Led */

/*----------------------------------------------------------------------------
 * Function Name: CascadeS_Intr_In_Move_Wait_Handle
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void CascadeS_Intr_In_Move_Wait_Handle(U8_T port,U8_T devinx,U8_T intt_id,U8_T endpidx)
{
	U8_T  *buf;
	U8_T  endp_bit;
	U8_T  len;

	//if (KVM_CurrentHost == KVM_HOST_IDLE_STATE)
	//	return;
		
	buf = HC_IntTransfer_Table[intt_id].PassControl.Buf;
	len = buf[0];

	endp_bit = (0x01 << endpidx);
	USBDC_EndpBufPtr[KVM_CurrentHost][devinx][endpidx][0] = len;
	memcpy((USBDC_EndpBufPtr[KVM_CurrentHost][devinx][endpidx]+2),(buf+1),len);
	USBDC_REGS_Endp_ControlSet(port,devinx,endpidx,DA_CR_BVLD_SET); // inform data move
	USBDC_Device[devinx].EndpIntrAttr |= endp_bit; // after Endp send out the interrutp need to clear this bit	
	USBDC_Device[devinx].EndpIntrWait &= ~endp_bit; //one buffer content is waiting
}

/*----------------------------------------------------------------------------
 * Function Name: CascadeS_Receive_USB_Handle
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
void CascadeS_Receive_USB_Handle(CASCADE_PacketHeader *packet)
{	
	U8_T port;
	U8_T devinx;
	U8_T remote_devinx;
			
	remote_devinx = packet->bIndex;
	devinx = Remote_Reserve_DevMap[remote_devinx];
	port = packet->bValue & 0x03;
	switch(packet->bCommand)
	{
		case CASCADE_USB_CONFIGURATION:
			CascadeS_API_Device_Config_Setting_Handle(packet);
			break;
		case CASCADE_USB_INTR_IN:						
			CascadeS_ReceiveInterruptInData(packet); 
			break;		
		case CASCADE_USB_REMOUNT:
		case CASCADE_USB_MOUNT:			
			CascadeS_USB_Device_Mount_Handle(packet);			
			break;
		case CASCADE_USB_UNMOUNT:
			CascadeS_USB_Device_Unmount_Handle(packet);			
			break;
		case CASCADE_USB_DESCRIPTOR:			
			CascadeS_Device_Descriptor_Handle(packet);			
			break;
		case CASCADE_USB_DESCRIPTOR_END:
			CascadeS_Device_Descriptor_End_Handle(packet);			
			break;			
		case CASCADE_USB_DATA_OUT_STAGE:		 
			if (USB_PDevice[devinx].Hc.Control.State == CTRL_DATA_OUT_WAIT) //This should put into Transmitter Receiver Handle first
			{				
				TASK_Active(TASK_TYPE_EVENT,TASK_CascadeS_USBHC_PassThrough_ID,0,devinx,0,0); //Generate the task, for nex
			}			
			break;
		case CASCADE_USB_DATA_IN_STAGE:			
			if (USB_PDevice[devinx].Hc.PassThrough_State == PASSTHROUGH_HC_SETUP_DATA_IN)
			{
				if (USB_PDevice[devinx].Hc.Control.State == CTRL_DATA_IN) //This should put into Transmitter Receiver Handle first
				{	//DC are waiting for data receive
					//DC buffer avaiable ....
					USB_PDevice[devinx].Hc.PControl.Total_Length = packet->iLen;
					USB_PDevice[devinx].Hc.PControl.Current_Length = 0; // new page start
					if (USB_PDevice[devinx].Hc.PControl.Total_Length)
					{
						 
//						CascadeS_CopyRxDmaToApp(CascadeS_Rx_Buf_Startp,
//												USB_PDevice[devinx].Hc.PControl.Total_Length+CASCADE_PACKET_HEADER_LEN,
//												USB_PDevice[devinx].Hc.PControl.Buf);
						DMA_GrantXdata((U8_T XDATA*)USB_PDevice[devinx].Hc.PControl.Buf, (U8_T XDATA*)packet, USB_PDevice[devinx].Hc.PControl.Total_Length+CASCADE_PACKET_HEADER_LEN);
					}

					// Send Data Page to DC 
					USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_DC_SETUP_DATA_IN; //Next is DC data in
					TASK_Active(TASK_TYPE_EVENT,TASK_CascadeS_USBHC_PassThrough_ID,0x00,devinx,0,0); //Generate the task, for next
				}
				else
				{
					//ack_answer = 0;
					printf("total_length=%d\n\r",(U16_T)USB_PDevice[devinx].Hc.PControl.Total_Length);
					printf("Setup linkage broken ...(%02x)\n\r",(U16_T)USB_PDevice[devinx].Hc.Control.State);
					printf("(%02x)",(U16_T)USB_PDevice[devinx].Hc.Control.State);
				}
			}
			else
			{
				printf(" <T>:Devinx:%d,Transmitter not Pass-Through DataIN mode(%02x)\n\r",(U16_T)devinx,
				(U16_T)USB_PDevice[devinx].Hc.PassThrough_State);
			}			
			break;
		case CASCADE_USB_SETUP_DONE: //answer IN or OUT to DC
#if (CASCADES_USB_PASSTHROUGH_DEBUG_MODE)
			printf(" <S>->[PASSTHROUGH_SETUP_DONE]\n\r");
#endif //#if (TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE)
			CascadeS_Device_Passthough_State[devinx] |= T_USB_PASSTHROUGH_REMOTE_DONE_FLAG;

			if ((USB_PDevice[devinx].Hc.Control.State == CTRL_DATA_OUT_WAIT) //This should put into Transmitter Receiver Handle first
				|| (USB_PDevice[devinx].Hc.PassThrough_State == PASSTHROUGH_HC_SETUP_STATUS_IN))
			{
				//printf("PASS\n\r");
				TASK_Active(TASK_TYPE_EVENT,TASK_CascadeS_USBHC_PassThrough_ID,0x00,devinx,0,0); //Generate the task, for nex
			}
			else
			{
				CascadeS_PassThrough_Done_Check(devinx,port);
			}			
			break;
		case CASCADE_USB_STATUS_STALL:	
			//printf("devinx:%d,port:%d STALL\n\r",(U16_T)devinx,(U16_T)port);		
			CascadeS_Device_Passthough_State[devinx] |= (T_USB_PASSTHROUGH_REMOTE_DONE_FLAG | T_USB_PASSTHROUGH_DC_DONE_FLAG);		 
			CascadeS_PassThrough_Done_Check(devinx,port);
			USBDC_HAL_Endp_Stalled(port,devinx,0);			
			break;
#if (SYSTEM_MSC_DEVICE_SUPPORT)
		case CASCADE_USB_MSC_DATA_OUT:
			CascadeS_MSC_BurstOutComplete();
			break;
		case CASCADE_USB_MSC_DATA_IN:
		case CASCADE_USB_MSC_CSW:
			//printf("Data_in:%02x-%d\n\r",(U16_T)packet->bCommand,packet->iLen);
			CascadeS_MSC_BurstDataInHandle(packet);
			break;
		case CASCADE_USB_MSC_STALL:
			/* For STALL, the bValue is used to indicate the IN or OUT direction replied from the extender receiver */
			//printf("MSC_STALL\n\r");
			Cascade_MSC_StallFlag = packet->bValue;
			CascadeS_MSC_DcBulkInStallHandle(devinx);
			break;
#endif //#if (SYSTEM_CascadeS_MSC_SUPPORT)

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
		case CASCADE_USB_ISO_IN:			
			CascadeS_Audio_Data_In_Stage(packet);			
			break;
		case CASCADE_USB_ISO_OUT:
			CascadeS_Audio_Data_Out_Enable(packet);			
			break;
#endif
		case CASCADE_USB_DEVICE_SWITCH: //do the device switch
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)			
			if (packet->bIndex == USBHC_Audio_Devinx) 
			{ //audio device switch ?				
				USBHC_AUDIO_Switch(packet->bValue);
				break;
			}		
#endif		

#if (SYSTEM_MSC_DEVICE_SUPPORT)		
			if (packet->bIndex == USBHC_MSC_Devinx) 
			{ //MSC device switch ?
				//KVM_CurrentMSC_Next = packet->bValue);
				USBHC_MSC_Switch(packet->bValue);				
				break;
			}		
#endif				
			break;
		default:
			printf("CCSLV: RX Unknown USB command=0x%02bx\r\n", packet->bCommand);			
			break;	
	}
}

/*----------------------------------------------------------------------------
 * Function Name: CascadeS_Receive_KVM_Handle
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
void CascadeS_Receive_KVM_Handle(CASCADE_PacketHeader *packet)
{
	switch(packet->bCommand)
	{			
		case CASCADE_KVM_STATE:
			CascadeS_Transmit_KVM_State(CascadeS_KVM_State.Connect_State, CascadeS_KVM_State.Power_State, CascadeS_KVM_State.Suspend_State);
			break;			
		case CASCADE_KVM_KB_LED: //Host request the KVM state 					
			CascadeS_Transmit_KVM_KB_Led(packet->bIndex,KVM_HostLed[packet->bIndex]);								
			break;				
		case CASCADE_KVM_HOST_ACTIVE: //Host request the KVM state 
			CascadeS_KVM_Port_Control(packet);		
			break;
		case CASCADE_KVM_SELECT_LED: //Host request the KVM state 	
		case CASCADE_KVM_HOST_LED: //Host request the KVM state 	
		 	CascadeS_KVM_Led_Control_Handle(packet);				
			break;	
		case CASCADE_KVM_VGA_CONTROL:
#if (SYSTEM_CASCADE_VGA_CONTROL)			
			KVM_CONSOLE_Vga_Control(packet->bIndex, packet->bValue);
#endif			
			break;
		default:
			printf("CCSLV: RX Unknown KVM command=0x%02bx\r\n", packet->bCommand);			
			break;				
	}	
}	

/*----------------------------------------------------------------------------
 * Function Name: CascadeS_Receive_System_Handle
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
void CascadeS_Receive_System_Handle(CASCADE_PacketHeader *packet)
{
	switch(packet->bCommand)
	{			
		case CASCADE_SC_EDID: //Console send edid table
			CascadeS_System_EDID_Handle(packet);		
			break;
#if (SYSTEM_KVM_BOX_CASCADE_SUPPORT) 		
		case CASCADE_BOX_REDIRECT:			
			CascadeBoxS_Box_Redirect_Handle(packet);
			break;
#endif /* #if (SYSTEM_KVM_BOX_CASCADE_SUPPORT) */
		default:
			printf("CCSLV: RX Unknown System command=0x%02bx\r\n", packet->bCommand);			
			break;					
	}	
}		
/*----------------------------------------------------------------------------
 * Function Name: CascadeS_USB_Device_Mount_Handle
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void CascadeS_USB_Device_Mount_Handle(CASCADE_PacketHeader *Data_Token)
{
	U8_T  free_deviceid;		
	U8_T  remote_devinx;	

//#if (CASCADES_CMD_DEBUG)
	printf(" <S>->Mount [Remote:%d(%02x),",(U16_T)Data_Token->bIndex,(U16_T)Data_Token->bValue);
//#endif /* #if (TRANSMITTER_CMD_DEBUG) */

	//1.Allocate a new device for this device
	if (USB_HAL_Alloc_Free_PDevice(&free_deviceid))
	{
//#if (CASCADES_CMD_DEBUG)
		printf("New SlaveDevinx:%d]\n\r",(U16_T)free_deviceid);
//#endif /* #if (TRANSMITTER_CMD_DEBUG) */

		//Loging the Master device information.
		remote_devinx		= Data_Token->bIndex;
		//mountport			= Data_Token->bValue;
		//Keep real information for handling
		//remote_upper_devinx = (Data_Token->bIndex & 0xf0) >> 4;
		//remote_upper_port	= Data_Token->bIndex & 0x0f;
		
		//upper_devinx        = Remote_Reserve_DevMap[remote_upper_devinx];

		//printf(" <S> Devinx=%d,remote_device=%d\n\r",(U16_T)free_deviceid,(U16_T)remote_devinx);

		Remote_DevMap[free_deviceid].Mapping   = remote_devinx | USBHC_DEVICE_USED_MASK;
		Remote_DevMap[free_deviceid].MountPort = Data_Token->bValue;;
		//Remote_DevMap[free_deviceid].UpperInfo = setup->Data_Token.bIndex;
		Remote_Reserve_DevMap[remote_devinx] = free_deviceid;
		CascadeS_USBDC_Check_Upstream_Suspend_State(Data_Token->bValue);
		//USB_PDevice[free_deviceid].DevAttr |= DEVATTR_CascadeM_DEV_MASK;

		//USB_PDevice[free_deviceid].UpperHubDevinx = upper_devinx; //store the upper layer hub devinx
//		upper_devinx = setup->Data_Token.bIndex & 0x0f;
		//USB_PDevice[free_deviceid].Hub_NbrPorts = remote_upper_port USBHC_ROOTHUBPORT_PORT_NUM_SHIFT; // upper layer connect port number
		//etdr_T_MountSkipFlag = 0;
		//return 1;
	}
	else
	{
		//etdr_T_MountSkipFlag = 1;
		printf("No free device index!!!\n\r");
		//return 0;
	}
}

/*----------------------------------------------------------------------------
 * Function Name: CascadeS_System_EDID_Handle
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeS_System_EDID_Handle(CASCADE_PacketHeader *packet)
{
	//1.Copy edid table into Trnasmitter_EDID
#if (CASCADES_CMD_DEBUG)
	printf(" (S)->[EDID Table]\n\r");
#endif /* if (CASCADES_CMD_DEBUG) */

	//buf = CascadeS_Rx_Buf_Startp + CASCADES_RX_BUFFER_START_ADDRESS; //get the start pointer
	//((U8_T *)Data_Token)+CASCADE_PACKET_HEADER_LEN;
//	CascadeS_CopyRxDmaToApp(CascadeS_Rx_Buf_Data_Startp,EDID_TABLE_SIZE,CascadeM_EdidTable);

	DMA_GrantXdata(ConsoleEdidTable, packet->buf, packet->iLen);		
	
	//2.Copy edid table into EDID Table	
	HW_EDID_EnableSlave();	
}

/*----------------------------------------------------------------------------
 * Function Name: Transmitter_Receiver_Unmount_Handle
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void CascadeS_USB_Device_Unmount_Handle(CASCADE_PacketHeader *Data_Token)
{
	U8_T remote_devinx;
	U8_T devinx;

	remote_devinx = Data_Token->bIndex;
	devinx = Remote_Reserve_DevMap[remote_devinx];

//#if (CASCADES_CMD_DEBUG)
	printf(" (S)->Unmount [RemoteDevice:%d,T_Device:%d,port=%02x]\n\r",(U16_T)remote_devinx,(U16_T)devinx,(U16_T)Data_Token->bValue);
//#endif  /* #if (TRANSMITTER_CMD_DEBUG)	*/	

	if (USB_PDevice[devinx].Hc.gState == HOST_PASSTHROUGH)
		CascadeS_PassThrough_Buf_In_Used_Flag = 0;

	
	if (Remote_DevMap[devinx].Mapping & USBHC_DEVICE_USED_MASK)
	{
		USBHC_CORE_Clear_Device(devinx);		
		Remote_DevMap[devinx].Mapping = 0;
		Remote_DevMap[devinx].MountPort = 0;
		Remote_Reserve_DevMap[remote_devinx] = 0;		
		CascadeS_Virtual_Device_Suspend_Check();
	}
	
	CascadeS_Transmit_USB_Device_Unmount(remote_devinx);
}

/*----------------------------------------------------------------------------
 * void CascadeS_Device_Descriptor_Handle(CASCADE_PacketHeader *Data_Token)
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
void CascadeS_Device_Descriptor_Handle(CASCADE_PacketHeader *packet)
{
	U16_T len;
	U8_T  descid;	
	U8_T  *buf;
	//U8_T  *data_buf;
	U8_T  remote_devinx;
	U8_T  devinx;
	U8_T  inf_id;
	//USBHC_HUBDec_Typedef	*phubdec;

	descid = packet->bValue;
	len    = packet->iLen;

#if (CASCADES_CMD_DEBUG)
	printf(" (S)->[Descriptor:%d,Len:%d,",(U16_T)packet->bValue,len);
#endif /*TRANSMITTER_CMD_DEBUG*/

	if (len != 0)
	{
		//printf("S0_");
		buf = m_malloc(len,55);
		if (buf != NULL)
		{
			remote_devinx = packet->bIndex;
			devinx = Remote_Reserve_DevMap[remote_devinx];
			
#if (CASCADES_CMD_DEBUG)
			printf("R:%d,T:%d]\n\r",(U16_T)remote_devinx,(U16_T)devinx);
#endif /*TRANSMITTER_CMD_DEBUG*/

			USB_PDevice[devinx].Desc[descid].Len = len;
			USB_PDevice[devinx].Desc[descid].Ptr = buf;

			//copy data into buffer
			//data_inx = CascadeS_Rx_Buf_Startp+CASCADE_PACKET_HEADER_LEN;
			//printf("Startp=%d\n\r",data_inx);
//			CascadeS_CopyRxDmaToApp(CascadeS_Rx_Buf_Data_Startp,len,buf);
			DMA_GrantXdata((U8_T XDATA*)buf, packet->buf, len);

			//Disp_Str(buf,len);
			
			switch(descid)
			{
				case DEVICE_DESC:
#if (CASCADES_CMD_DEBUG)
					printf(" <S>-[Parser Device Descriptor]\n\r");
					//Disp_Str((U8_T *)packet,CASCADE_PACKET_HEADER_LEN);
					//Disp_Str(USB_PDevice[devinx].Desc[descid].Ptr,USB_PDevice[devinx].Desc[descid].Len);
#endif /*CASCADES_CMD_DEBUG*/
					USBHC_Parser_DevDesc(devinx,USB_PDevice[devinx].Desc[descid].Ptr);
					break;
				case CONFIG_DESC:
#if (CASCADES_CMD_DEBUG)
					printf(" <S>-[Parser Configuration Descriptor]\n\r");
					//Disp_Str(USB_PDevice[devinx].Desc[descid].Ptr,USB_PDevice[devinx].Desc[descid].Len);
#endif /*CASCADES_CMD_DEBUG*/
					USBHC_Parser_ConfigDesc(devinx,len,USB_PDevice[devinx].Desc[descid].Ptr);
					break;
				case HID0_REPORT:
				case HID1_REPORT:
				case HID2_REPORT:
				case HID3_REPORT:
					inf_id = descid-HID0_REPORT;
					USB_PDevice[devinx].Hc.Control.Buf = buf; //assing hid report descritpro table to control pointer
#if (CASCADES_CMD_DEBUG)
					printf(" <S>-[Parser HID Report Descriptor for interface[%d]]\n\r",(U16_T)inf_id);
					//Disp_Str(USB_PDevice[devinx].Desc[descid].Ptr,USB_PDevice[devinx].Desc[descid].Len);
#endif /*TRANSMITTER_CMD_DEBUG*/
					//USBHC_HID_Interface_Check(devinx,inf_id);
					USB_PDevice[devinx].Hc.Control.Buf = 0x00; //assing hid report descritpro table to control pointer
					break;
				/*	
				case HUB_DESC:
					//Parser Hub Descriptor
					phubdec = (USBHC_HUBDec_Typedef *)USB_PDevice[devinx].Desc[HUB_DESC].Ptr;
					USB_PDevice[devinx].Hub_NbrPorts |= phubdec->bNbrPorts;
					len = sizeof(USBHC_HubReportTypeDef) * phubdec->bNbrPorts;
					USB_PDevice[devinx].HUB.ReportState = m_malloc(len);
					if (USB_PDevice[devinx].HUB.ReportState != NULL)
					{
						memset((U8_T *)USB_PDevice[devinx].HUB.ReportState,0x00,len);
					}
					else
					{
						printf("HUB Report Malloc Error !!\n\r");
					}
					break;
				*/					
			}
		}
		else
		{
			printf("Descriptor malloc fail !!!]\n\r");
		}
	}	
}

/*----------------------------------------------------------------------------
 * void CascadeS_Device_Descriptor_End_Handle(CASCADE_PacketHeader *Data_Token)
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *---------------------------------------------------------------------------*/
void CascadeS_Device_Descriptor_End_Handle(CASCADE_PacketHeader *Data_Token)
{
	U8_T  remote_devinx;
	U8_T  devinx;
	U8_T  mountport;	
		
	remote_devinx = Data_Token->bIndex;
	devinx = Remote_Reserve_DevMap[remote_devinx];
	mountport = Remote_DevMap[devinx].MountPort;
					
#if (CASCADES_CMD_DEBUG)	
	printf(" <S>-[Active T_Device:%d(%02x)]\n\r",(U16_T)devinx,(U16_T)mountport);
	//printf(" [R-%d,S-%d]\n\r",(U16_T)remote_devinx,(U16_T)devinx);
#endif /*TRANSMITTER_CMD_DEBUG*/
		//Check the device Class

	if (USB_PDevice[devinx].DevClass == USB_MSC_CLASS)
	{
#if (SYSTEM_MSC_DEVICE_SUPPORT)		
		KVM_CurrentMSC   =  CascadeS_Retrive_Port(mountport);		
		USBHC_MSC_Devinx = devinx;			
		//printf("S1_");
		USB_PDevice[devinx].MSC = (USBHC_MSC_Device_TypeDef *) m_malloc(sizeof(USBHC_MSC_Device_TypeDef),56);
		memset(USB_PDevice[devinx].MSC,0x00,sizeof(USBHC_MSC_Device_TypeDef));	
		//printf("S2_");
		USB_PDevice[devinx].MSC->BOT = (MSC_BOT_Control_TypeDefine *) m_malloc(sizeof(MSC_BOT_Control_TypeDefine),57);
		memset(USB_PDevice[devinx].MSC->BOT,0x00,sizeof(MSC_BOT_Control_TypeDefine));
		USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_IDLE;			
#endif //#if (SYSTEM_CascadeS_MSC_SUPPORT)					
	}
	
	if (USB_PDevice[devinx].DevClass == USB_AUDIO_CLASS)
	{
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
		KVM_CurrentUSBAudio = CascadeS_Retrive_Port(mountport);		
		USBHC_Audio_Devinx 	= devinx;				
#endif //#if (SYSTEM_CascadeS_MSC_SUPPORT)					
	}
				
	USBHC_Active_New_Device(devinx,mountport);
	
	CascadeS_Virtual_Device_Suspend_Check();	
	
	//Send out Descriptor_End message
	CascadeS_Transmit_Descriptor_End(remote_devinx);
}

/*----------------------------------------------------------------------------
 * Function Name: CascadeS_Transmit_Descriptor_End
 * Purpose: Send out the KVM state
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeS_Transmit_Descriptor_End(U8_T remote_devinx)
{	 
	U16_T  len;
	
	len = Cascade_Set_Data_Transmit_Header(0,NULL,NULL);	 
	if (len == 0)
	{			
		return;
	} 		

#if (CASCADES_CMD_DEBUG)		
	printf("(S)Send DESCRIPTOR_END:%d\n\r",(U16_T)remote_devinx);		
#endif
				
	Cascade_Data_TokenP->bCommand = CASCADE_USB_DESCRIPTOR_END;	
	//Cascade_Data_TokenP->iLen	   	= 0;	
	//Cascade_Data_TokenP->bRequest	= 0;	
	Cascade_Data_TokenP->bIndex	= remote_devinx;	
	//Cascade_Data_TokenP->bValue	= 0;
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
	USBDC_INT_DISABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
	CascadeS_Send_Transmit(Cascade_Transmit_Send_Buf,0,len);
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
	USBDC_INT_ENABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
}

/*----------------------------------------------------------------------------
 * Function Name: CascadeS_Transmit_USB_Device_Unmount
 * Purpose: Send out the KVM state
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeS_Transmit_USB_Device_Unmount(U8_T remote_devinx)
{	 
	U16_T  len;
	
	len = Cascade_Set_Data_Transmit_Header(0,NULL,NULL);	 
	if (len == 0)
	{	
		//printf("<S> Send CASCADE_USB_UNMOUNT Fail\n\r");
		return;
	} 		
		
	//printf("(S)Send CASCADE_USB_UNMOUNT:%d\n\r",(U16_T)remote_devinx);		
				
	Cascade_Data_TokenP->bCommand = CASCADE_USB_UNMOUNT;	
	//Cascade_Data_TokenP->iLen	   	= 0;	
	//Cascade_Data_TokenP->bRequest	= 0;	
	Cascade_Data_TokenP->bIndex	= remote_devinx;	
	//Cascade_Data_TokenP->bValue	= 0;	
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
	USBDC_INT_DISABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
	CascadeS_Send_Transmit(Cascade_Transmit_Send_Buf,0,len);
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
	USBDC_INT_ENABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
}

/*----------------------------------------------------------------------------
 * Function Name: CascadeS_ReceiveInterruptInData
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeS_ReceiveInterruptInData(CASCADE_PacketHeader *packet)
{
	U8_T  devinx;
	U8_T  remote_devinx;
	U8_T  inf_inx;
	U8_T  intt_id;
	U8_T  endpidx;
	U8_T  *buf;
	U8_T  endp_bit;
	U8_T  wp;
	U8_T  rp;
//	U8_T  databyte;
	U8_T  remain_siz;
	U16_T len;
	U16_T tmp16;
	
	remote_devinx = packet->bIndex;
	devinx = Remote_Reserve_DevMap[remote_devinx];
	
	//printf("devinx=%bu\n\r");
	if (KVM_CurrentHost == 0xff)
	{			
		return;
	}
	
	//Check whether upstream port is connect?		
	if (USBDC_Device[devinx].DevAddr[KVM_CurrentHost] == 0x00)
	{		
		return;
	}

	if (USBDC_Check_Upstream_Suspend_State(KVM_CurrentHost,devinx))
	{
		return;
	}

	if (USBDC_Device[devinx].Feature[KVM_CurrentHost] & FEATURE_RESUME)
	{	
		return;
	}		
		
	len = packet->iLen;	
	inf_inx = packet->bRequest; //this interface	
	endpidx = packet->bValue; //this interface	
	intt_id = USBDC_Device[devinx].EndpType[endpidx] & 0x1f; // Get interrupt TD ID number
	//printf("devinx=%d,len=%d,inf=%d,endp=%d,intt_id=%d\n\r",(U16_T)len,(U16_T)inf_inx,(U16_T)endpidx,(U16_T)intt_id);	
	//Special handle for generic HID device
#if (PROJECT_USB_GENERIC_HID_ENABLE)
	if (devinx == USBDC_VHID_DEVINX)
	{				
		if (endpidx == USBDC_VHID_KB_ENDP_NUM)
		{
//			CascadeS_CopyRxDmaToApp(CascadeS_Rx_Buf_Data_Startp,len,DATAST_Generic_USB_KB_Queue);		
			DMA_GrantXdata((U8_T XDATA*)DATAST_Generic_USB_KB_Queue, packet->buf, len);
			TASK_DATAST_PS2_Generic_KB_ActiveFlag =1;
			TASK_Active(TASK_TYPE_EVENT,TASK_DATAST_PS2_Generic_KB_ID,0,len,0,0); // now aticev the Generic KB Output Handle
			//USBDC_HAL_Endp_In_Buf_Move(DATAST_Generic_USB_KB_Queue,8,KVM_CurrentHost,USBDC_VHID_DEVINX,USBDC_VHID_KB_ENDP_NUM);
			//USBDC_REGS_Endp_ControlSet(KVM_CurrentHost,USBDC_VHID_DEVINX,USBDC_VHID_KB_ENDP_NUM,DA_CR_BVLD_SET);
		}
		else
		{
//			CascadeS_CopyRxDmaToApp(CascadeS_Rx_Buf_Data_Startp,len,DATAST_Generic_USB_MS_Queue);						
			DMA_GrantXdata((U8_T XDATA*)DATAST_Generic_USB_MS_Queue, packet->buf, len);
			TASK_DATAST_PS2_Generic_MS_ActiveFlag =1;
			TASK_Active(TASK_TYPE_EVENT,TASK_DATAST_PS2_Generic_MS_ID,0,len,0,0); // now aticev the Generic KB Output Handle
		}
		return;		
	}	
#endif	
	if (USB_PDevice[devinx].InfProtocol[inf_inx] == INF_KEYBOARD) /* should be INF_MOUSE ? */
	{
CascadeS_InterruptTransferIN_Buf_Restart:
		wp = HC_IntTransfer_Table[intt_id].PassControl.Wp;
		rp = HC_IntTransfer_Table[intt_id].PassControl.Rp;

		//2_1.Check the buffer remain size(contain len bytes at least)
		if (wp == rp)
		{
			remain_siz = HC_IntTransfer_Table[intt_id].PassControl.BufSize;
		}
		else if (wp > rp)
		{
			remain_siz = (HC_IntTransfer_Table[intt_id].PassControl.BufSize-wp)+rp;
		}
		else
		{
			remain_siz = rp - wp;
		}

		//2_2.move out the data if the remain size is small than len
		if (remain_siz <= len)
		{
			//printf("CCSLV: KB buffre full, %bu<=%u, wp=%02bx, rp=%02bx\r\n", remain_siz, len, wp, rp);		
			USBHC_Remove_HID_Buffer(intt_id);
			goto CascadeS_InterruptTransferIN_Buf_Restart;
		}

		//2_3.move interrupt in buffer data into out queue buffer
		HC_IntTransfer_Table[intt_id].PassControl.Buf[wp] = len;
		wp++;
		if (wp >= HC_IntTransfer_Table[intt_id].PassControl.BufSize)
			wp = 0;
			
		for (tmp16=0; tmp16<packet->iLen; tmp16++)
		{
			HC_IntTransfer_Table[intt_id].PassControl.Buf[wp] = packet->buf[tmp16];
			wp++;
			if (wp >= HC_IntTransfer_Table[intt_id].PassControl.BufSize)
				wp = 0;
		}
		HC_IntTransfer_Table[intt_id].PassControl.Wp = wp;

		if ((HC_IntTransfer_Table[intt_id].PassControl.State & PASSTHROUGH_SET) == 0)
		{
			HC_IntTransfer_Table[intt_id].PassControl.State |= PASSTHROUGH_SET;
			TASK_Active(TASK_TYPE_EVENT,TASK_DATAST_USB_Virtual_HID_ID,0,intt_id,0,0);
		}	
	}
	else/* Is mouse protocol */
	{
		endp_bit = (0x01 << endpidx);
		//start_index = CascadeS_Rx_Buf_Startp+CASCADE_PACKET_HEADER_LEN;
		if ((USBDC_Device[devinx].EndpIntrAttr & endp_bit) == 0)
		{	
			buf = (USBDC_EndpBufPtr[KVM_CurrentHost][devinx][endpidx]+2);			
//			CascadeS_CopyRxDmaToApp(CascadeS_Rx_Buf_Data_Startp,len,buf);			
			DMA_GrantXdata((U8_T XDATA*)buf, packet->buf, len);
			USBDC_EndpBufPtr[KVM_CurrentHost][devinx][endpidx][0] = len;
			USBDC_REGS_Endp_ControlSet(KVM_CurrentHost,devinx,endpidx,DA_CR_BVLD_SET); // inform data move
			USBDC_Device[devinx].EndpIntrAttr |= endp_bit; // after Endp send out the interrutp need to clear this bit
		}
		else
		{			
			buf = HC_IntTransfer_Table[intt_id].PassControl.Buf;
			buf[0] = (len & 0x00ff);
//			CascadeS_CopyRxDmaToApp(CascadeS_Rx_Buf_Data_Startp,len,buf+1);			
			DMA_GrantXdata((U8_T XDATA*)buf+1, packet->buf, len);
			USBDC_Device[devinx].EndpIntrWait |= endp_bit; //one buffer content is waiting
		}
	}
}

/*----------------------------------------------------------------------------
 * void CascadeS_PassThrough_Done_Check(U8_T devinx,U8_T port)
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeS_PassThrough_Done_Check(U8_T devinx,U8_T port)
{
	if (CascadeS_Device_Passthough_State[devinx] & T_USB_PASSTHROUGH_REMOTE_DONE_FLAG)
	{
		if (CascadeS_Device_Passthough_State[devinx] & T_USB_PASSTHROUGH_DC_DONE_FLAG)
		{
			//printf("Pass Conditin Closed\n\r");
			USBDC_Device[devinx].PassThroughState[port] = PASSTHROUGH_IDLE;
			USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_IDLE;
#if (CASCADES_PASSTHROUGH_BUF_MALLOC)
			malloc_free(USB_PDevice[devinx].Hc.PControl.Buf);
			USB_PDevice[devinx].Hc.PControl.Buf = 0;
			//USB_PDevice[devinx].Hc.PControl.Buf -= sizeof(Extender_Data_Packet_Def); //jump inot position 0
			//malloc_free(USB_PDevice[devinx].Hc.Control.Buf);
#endif	/* #if (CascadeM_PASSTHROUGH_BUF_MALLOC) */
			USB_PDevice[devinx].Hc.Control.Buf = 0;
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
			USB_PDevice[devinx].Hc.gState = USB_PDevice[devinx].Hc.PassThrough_gState_bk;
			CascadeS_PassThrough_Buf_In_Used_Flag = 0;
			CascadeS_Device_Passthough_State[devinx] = 0;
		}
	}
}


#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
/*----------------------------------------------------------------------------
 * void CascadeS_Audio_Data_In_Buf_Check(void)
 * Purpose: 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void CascadeS_Audio_Data_In_Buf_Check(void)
{
	U16_T len;	
  	 	
  	if (CascadeS_ISO_DC_IN_Sending_Flag)
  		return;
  			
	//if (CADR_USB_DC_Audio_Inx != CADR_USB_CascadeS_Audio_Inx) //check the waiting buffer
	{					
		CascadeS_ISO_DC_IN_Sending_Flag = 1;		
		//copy				 		 
		USBDC_Index[KVM_CurrentUSBAudio][USBHC_Audio_Devinx][USBHC_Audio_In_Endpinx].Endp_Addr_Lsb = CADR_USB_AudioBufAddr[CADR_USB_CascadeS_Audio_Inx][0];
		USBDC_Index[KVM_CurrentUSBAudio][USBHC_Audio_Devinx][USBHC_Audio_In_Endpinx].Endp_Addr_Msb = CADR_USB_AudioBufAddr[CADR_USB_CascadeS_Audio_Inx][1];
		len = DA_CR_BVLD_SET | (USBHC_Audio_In_Endpinx &0x07);
		switch (KVM_CurrentUSBAudio)
		{
			case 0:
				_USBDC_DC0DR_SFR(len);
				_USBDC_DC0CIR_SFR(DA_CR[USBHC_Audio_Devinx]);
				break;
			case 1:
				_USBDC_DC1DR_SFR(len);
				_USBDC_DC1CIR_SFR(DA_CR[USBHC_Audio_Devinx]);
				break;
			case 2:
				_USBDC_DC2DR_SFR(len);
				_USBDC_DC2CIR_SFR(DA_CR[USBHC_Audio_Devinx]);
				break;
			case 3:
				_USBDC_DC3DR_SFR(len);
				_USBDC_DC3CIR_SFR(DA_CR[USBHC_Audio_Devinx]);
				break;
		}
		
		CADR_USB_CascadeS_Audio_Inx++;
		if (CADR_USB_CascadeS_Audio_Inx >= SYSTEM_CASCADE_AUDIO_BUF_CNT)
			CADR_USB_CascadeS_Audio_Inx = SYSTEM_CASCADE_AUDIO_IN_START;
	}
}
/*----------------------------------------------------------------------------
 * void CascadeS_Audio_Data_Out_Enable(CASCADE_PacketHeader *Data_Token)
 * Purpose: Send out next ISO out, if aviable.
 * Params:
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeS_Audio_Data_Out_Enable(CASCADE_PacketHeader *Data_Token)
{
	U16_T len;		
	U8_T  outp;
	
	//Clear the buffer out send flag
	outp = Data_Token->bValue;
	//CADR_USB_CascadeS_Buffer_Out &= ~BIT_MASK[outp];			
																				
	//Check wheither the buffer table is full....	

	while (CADR_USB_DC_Audio_Outx != CADR_USB_CascadeS_Audio_Outx)
	{		
		len =  CADR_USB_Audio_Buf[CADR_USB_CascadeS_Audio_Outx].Buf[0]+CASCADE_PACKET_HEADER_LEN+2;
		printf("len=%d\n\r",(U16_T)len);
		CADR_USB_Audio_Buf[CADR_USB_CascadeS_Audio_Outx].Token.bCommand = CASCADE_USB_ISO_OUT;						
		CADR_USB_Audio_Buf[CADR_USB_CascadeS_Audio_Outx].Token.iLen     = CADR_USB_Audio_Buf[CADR_USB_CascadeS_Audio_Outx].Buf[0]+2;
		CADR_USB_Audio_Buf[CADR_USB_CascadeS_Audio_Outx].Token.bRequest = 0;
		CADR_USB_Audio_Buf[CADR_USB_CascadeS_Audio_Outx].Token.bIndex   = Data_Token->bIndex; //devinx
		CADR_USB_Audio_Buf[CADR_USB_CascadeS_Audio_Outx].Token.bValue   = CADR_USB_CascadeS_Audio_Outx;					
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
		USBDC_INT_DISABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */		
		CascadeS_Send_Transmit((U8_T *)&CADR_USB_Audio_Buf[CADR_USB_CascadeS_Audio_Outx],CASCADE_CONTROL_NONE_FREE,len);
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
		USBDC_INT_ENABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */								
		CADR_USB_CascadeS_Audio_Outx++; //go for next OUT
		if (CADR_USB_CascadeS_Audio_Outx >= SYSTEM_CASCADE_AUDIO_IN_START)
			CADR_USB_CascadeS_Audio_Outx = 0;
		//CADR_USB_CascadeS_Buffer_Out |= BIT_MASK[CADR_USB_CascadeS_Audio_Outx];			
	};
}
/*----------------------------------------------------------------------------
 * void CascadeS_Audio_Data_In_Stage(CASCADE_PacketHeader *Data_Token)
 * Purpose: Handle the ISO input data packet for Master 
 * Params:
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeS_Audio_Data_In_Stage(CASCADE_PacketHeader *packet)
{
	U8_T		len;

	//Check the buffer valid condition,	
	if ((CADR_USB_DC_Audio_Inx == CADR_USB_CascadeS_Audio_Inx) && (CascadeS_ISO_DC_IN_Sending_Flag == 0)) //indicate the TD buffer is ready for data fillup
	{	 
		//printf("S");
		//CascadeS_ISO_DC_IN_Sending_Flag = 1;
		//CascadeS_Audio_Data_In_Buf_Check(packet);			
#if (CASCADES_AUDIO_BUFFING_MODE)	
		if (CascadeS_AudioIn_Buffering_Start_Flag==0)
#endif			
		{
			CascadeS_Audio_Data_In_Buf_Check();			
		}						
#if (CASCADES_AUDIO_BUFFING_MODE)					
		//CascadeS_AudioOut_Buffering_Start_Flag = 1;
		//printf("1_Buffering Start\n\r");
#endif
	}
	else
	{	
		len = packet->iLen;				
//		CascadeS_CopyRxDmaToApp(CascadeS_Rx_Buf_Data_Startp,len,CADR_USB_Audio_Buf[CADR_USB_CascadeS_Audio_Inx].Buf);
		DMA_GrantXdata((U8_T XDATA*)CADR_USB_Audio_Buf[CADR_USB_CascadeS_Audio_Inx].Buf, packet->buf, len);
		
		CADR_USB_CascadeS_Audio_Inx++;
		if (CADR_USB_CascadeS_Audio_Inx >= SYSTEM_CASCADE_AUDIO_BUF_CNT)
			CADR_USB_CascadeS_Audio_Inx = SYSTEM_CASCADE_AUDIO_IN_START;
			
		if ((CascadeS_ISO_DC_IN_Sending_Flag == 0) &&(CascadeS_AudioIn_Buffering_Start_Flag==0))
		{
			CascadeS_Audio_Data_In_Buf_Check();	
		}	
		else
		{
#if (CASCADES_AUDIO_BUFFING_MODE)
			if (CascadeS_AudioIn_Buffering_Start_Flag)
			{	
				if (CADR_USB_CascadeS_Audio_Inx == (SYSTEM_CASCADE_AUDIO_BUF_CNT-1))
				{
					CascadeS_AudioIn_Buffering_Start_Flag = 0; //cancel the buffering flag
					CascadeS_Audio_Data_In_Buf_Check();						
				}	
			}					
#endif	
		}	
	}		
}

/*----------------------------------------------------------------------------
 * void CascadeS_Setup_Command_Handle(U8_T devinx,USB_Setup_TypeDef *setup)
 * Purpose:  
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeS_Setup_Command_Handle(U8_T devinx,USB_Setup_TypeDef *setup)
{
	U8_T Request_No;
	U8_T Type_Recipient;	
	//Do Little endian convert to Big endian
	U8_T wValue_L;	
	U8_T wIndex_L;
	
	
	Request_No = setup->b.bRequest;
	Type_Recipient = setup->b.bmRequestType & (REQUEST_TYPE | RECIPIENT);
	wValue_L = setup->b.wValue.bw.lsb;	
	wIndex_L = setup->b.wIndex.bw.lsb;	

	if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))  /* Interface Receivent */
	{
			/*SET INTERFACE*/
		switch (Request_No)
		{
			case SET_INTERFACE:
				//Check the Audio Set Interface handle
#if (CASCADES_AUDIO_BUFFING_MODE)				
				if (USBHC_Audio_Devinx == devinx)
				{
					//printf("Devinx:%d,Set Interface:%d,Value=%d\n\r",(U16_T)devinx,(U16_T)wIndex_L,(U16_T)(U16_T)wValue_L);
					//printf("intf=%d\n\r",(U16_T)USBDC_Audio_In_Interface);
					if (wIndex_L == USBDC_Audio_In_Interface)
					{
						if (wValue_L == 0) //turn of the out
						{
							//printf("0_Buffering Stop\n\r");
							CascadeS_AudioIn_Buffering_Start_Flag = 0;							
							CADR_USB_CascadeS_Audio_Inx   = SYSTEM_CASCADE_AUDIO_IN_START;							
							CADR_USB_CascadeS_Audio_Inx    = SYSTEM_CASCADE_AUDIO_IN_START;	
							//CascadeS_ISO_DC_IN_Sending_Flag = 0;
						}
						else
						{
							CascadeS_AudioIn_Buffering_Start_Flag = 1; //start for buffering
							//printf("0_Buffering Start\n\r");
						}		
					}
				}				
#endif					
				break;
		}
	}				
}
#endif

/*----------------------------------------------------------------------------
 * Function Name: CascadeS_USB_IntrOutProcess(U8_T devinx, U8_T endpinx)
 * Purpose:  
 * Params:
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void CascadeS_Intr_Data_Out_Handle(U8_T pid,U8_T devinx, U8_T endpinx)
{	 	
	CASCADE_PacketHeader *Data_Token;
	U8_T	*pSrc;
	U8_T	len;
	U8_T	intt_id;	
	U8_T	remote_devinx;
	
	pSrc = USBDC_EndpBufPtr[pid][devinx][endpinx];
	len = *pSrc;	
	intt_id = USBDC_Device[devinx].EndpType[endpinx] & 0x1f;
	DMA_GrantXdata(HC_IntTransfer_Table[intt_id].CascadeDataBuf+CASCADE_PACKET_HEADER_LEN,pSrc,len+2);	
	remote_devinx = Remote_DevMap[devinx].Mapping & ~USBHC_DEVICE_USED_MASK;		 			
					
	Data_Token->bCommand = CASCADE_USB_INTR_OUT;	
	Data_Token->iLen	   	= len;	
	Data_Token->bRequest	= pid;	
	Data_Token->bIndex		= remote_devinx;	
	Data_Token->bValue 		= endpinx;
	
	len += CASCADE_PACKET_HEADER_LEN;
	CascadeS_Send_Transmit(HC_IntTransfer_Table[intt_id].CascadeDataBuf,CASCADE_CONTROL_NONE_FREE,len);
}

/**--------------------------------------------------------------------------------
 * void KVM_CONSOLE_Keyboard_Led_Control(void)
 * Purpose : Handle the hotkey mode deactive control
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------*/
void CascadeS_KVM_CONSOLE_Keyboard_Led_Control(U8_T port,U8_T kbled)
{	
	CascadeS_Transmit_KVM_KB_Led(port,kbled); // this will boardcast the led to USB HC	
}

/**--------------------------------------------------------------------------------
 * void CascadeS_Change_IntTransfer_Interval(U8_T devinx,U8_T endpinx,U8_T interval)
 * Purpose : Handle the hotkey mode deactive control
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------*/
void CascadeS_Change_IntTransfer_Interval(U8_T devinx,U8_T endpinx,U8_T interval)
{
	U16_T  len;
	U8_T   remote_devinx;
	
	remote_devinx = Remote_DevMap[devinx].Mapping & ~USBHC_DEVICE_USED_MASK;
	len = Cascade_Set_Data_Transmit_Header(0,NULL,NULL);	 			
	Cascade_Data_TokenP->bCommand  = CASCADE_USB_INTERVAL;	
	Cascade_Data_TokenP->iLen	    = 0;	
	Cascade_Data_TokenP->bRequest	= endpinx;	
	Cascade_Data_TokenP->bIndex	= remote_devinx;	
	Cascade_Data_TokenP->bValue	= interval;	
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
	USBDC_INT_DISABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
	CascadeS_Send_Transmit(Cascade_Transmit_Send_Buf,0,len);
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
	USBDC_INT_ENABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
}

/**--------------------------------------------------------------------------------
 * void CascadeS_Change_Protocol_Setting(U8_T port,U8_T devinx,U8_T intf,U8_T value)
 * Purpose : Handle the hotkey mode deactive control
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------*/
void CascadeS_Change_Protocol_Setting(U8_T port,U8_T devinx,U8_T intf,U8_T value)
{
	U16_T  len;
	U8_T   remote_devinx;
	
	remote_devinx = Remote_DevMap[devinx].Mapping & ~USBHC_DEVICE_USED_MASK;
	len = Cascade_Set_Data_Transmit_Header(0,NULL,NULL);	 			
	Cascade_Data_TokenP->bCommand  = CASCADE_USB_PROTOCOL_SETTING;	
	Cascade_Data_TokenP->iLen	    = 0;	
	Cascade_Data_TokenP->bRequest	= intf;	
	Cascade_Data_TokenP->bIndex	= remote_devinx | (port << 4);	
	Cascade_Data_TokenP->bValue	= value;	
	//printf("<S>SetProtocol,");
	//Disp_Str(Cascade_Transmit_Send_Buf,6);
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
	USBDC_INT_DISABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
	CascadeS_Send_Transmit(Cascade_Transmit_Send_Buf,0,len);
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
	USBDC_INT_ENABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
}

/**--------------------------------------------------------------------------------
 * void CascadeS_Change_Alternate_Setting(U8_T devinx,U8_T intf,U8_T value)
 * Purpose : Handle the hotkey mode deactive control
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------*/
void CascadeS_Change_Alternate_Setting(U8_T devinx,U8_T intf,U8_T value)
{
	U16_T  len;
	U8_T   remote_devinx;
	
	remote_devinx = Remote_DevMap[devinx].Mapping & ~USBHC_DEVICE_USED_MASK;
	len = Cascade_Set_Data_Transmit_Header(0,NULL,NULL);	 			
	Cascade_Data_TokenP->bCommand  = CASCADE_USB_ALTERNATE_SETTING;	
	Cascade_Data_TokenP->iLen	    = 0;	
	Cascade_Data_TokenP->bRequest	= intf;	
	Cascade_Data_TokenP->bIndex	= remote_devinx;	
	Cascade_Data_TokenP->bValue	= value;	
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
	USBDC_INT_DISABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
	CascadeS_Send_Transmit(Cascade_Transmit_Send_Buf,0,len);
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
	USBDC_INT_ENABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
}

/**--------------------------------------------------------------------------------
 * void CascadeS_Change_Configuration_Setting(U8_T devinx,U8_T value)
 * Purpose : Handle the hotkey mode deactive control
 * Params  : NONE
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------*/
void CascadeS_Change_Configuration_Setting(U8_T devinx,U8_T value)
{
	U16_T  len;
	U8_T   remote_devinx;
	
	remote_devinx = Remote_DevMap[devinx].Mapping & ~USBHC_DEVICE_USED_MASK;
	len = Cascade_Set_Data_Transmit_Header(0,NULL,NULL);	 			
	Cascade_Data_TokenP->bCommand  = CASCADE_USB_CONFIGURATION;	
	Cascade_Data_TokenP->iLen	    = 0;	
	Cascade_Data_TokenP->bRequest	= 0;	
	Cascade_Data_TokenP->bIndex	= remote_devinx;	
	Cascade_Data_TokenP->bValue	= value;	
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
	USBDC_INT_DISABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
	CascadeS_Send_Transmit(Cascade_Transmit_Send_Buf,0,len);
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
	USBDC_INT_ENABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
}
/**-------------------------------------------------------------------------------------------
 * U8_T CascadeS_USBDC_Check_Upstream_Suspend_State(U8_T port,U8_T devinx,U8_T remote)
 * Purpose: Measure the endpoint interval time
 * Params : NONE
 * Returns: 
 * Note:
 *-----------------------------------------------------------------------------------------*/
void CascadeS_USBDC_Check_Upstream_Suspend_State(U8_T mount_port)
{ 
	U8_T index;
	
	for (index=0; index < 4;index++)
	{	
		if (mount_port & BIT_MASK[index])
		{	
			if (USBDC_UpPortState[index] & USBDC_ROOTHUB_ATTACHED_MASK)
			{
				//need to wake up host	
				if (USBDC_UpPortState[index] & USBDC_ROOTHUB_SUSPEND_MASK)
				{	
					//printf("Wakeup:%d\n\r",(U16_T)index);	
					USBDC_Port_Resume(index);
					mount_port &= ~BIT_MASK[index];
					if (mount_port ==0)
						return;
				}	
			}
		}	
	}
}

/*----------------------------------------------------------------------------
 * Function Name: CascadeS_Transmit_KVM_KB_Led
 * Purpose: Send out the kb led state
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeS_Transmit_MSC_Burst_In_Enable(U8_T port,U8_T devinx)
{	
	U16_T  len;
	
	len = Cascade_Set_Data_Transmit_Header(0,NULL,NULL);	 
	if (len == 0)
	{	
		//printf("<S> Send Burst IN Enable\n\r");
		return;
	} 
	
	//token = (CASCADE_PacketHeader *)buf;			 				
	Cascade_Data_TokenP->bCommand = CASCADE_USB_MSC_DATA_IN;	
	//Cascade_Data_TokenP->iLen	   	= 0;	
	//Cascade_Data_TokenP->bRequest	= 0;	
	Cascade_Data_TokenP->bIndex	= devinx;	
	Cascade_Data_TokenP->bValue	= port;	
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
	USBDC_INT_DISABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
	CascadeS_Send_Transmit(Cascade_Transmit_Send_Buf,0,len);
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
	USBDC_INT_ENABLE;
#endif /* #if (SYSTEM_AUDIO_DEVICE_SUPPORT) */	
}

/*----------------------------------------------------------------------------
 * U8_T CascadeS_Retrive_Port(U8_T mountport)
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
U8_T CascadeS_Retrive_Port(U8_T mountport)
{
	U8_T  index;
	
	for (index=0; index < 4 ; index++)
	{
		if (mountport & MOUNT_PORT[index])
		{
			return index;
		}	
	}
	
	return 0;
}

/*----------------------------------------------------------------------------
 * void CascadeS_Port_Connect_State(U8_T port,U8_T value)
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeS_Port_Connect_State(U8_T port,U8_T value)
{
	U8_T old_value;
	
	//printf("Connect:%bx-%bx\n\r",port,value);
	old_value = CascadeS_KVM_State.Connect_State & BIT_MASK[port];
	CascadeS_KVM_State.Connect_State &= ~BIT_MASK[port];
	if (value)
		CascadeS_KVM_State.Connect_State |= BIT_MASK[port];
	Cascade_Compare_State(old_value,value);	
}

/*----------------------------------------------------------------------------
 * void CascadeS_Port_KVM_Connect_State(U8_T port,U8_T value)
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeS_Port_KVM_Connect_State(U8_T port,U8_T value)
{
	U8_T old_value;
	U8_T kvm_bit;
	
	//printf("Connect:%bx-%bx\n\r",port,value);
	kvm_bit = BIT_MASK[port+4]; //MSB nibble(high nibble part)
	old_value = CascadeS_KVM_State.Connect_State & kvm_bit;
	CascadeS_KVM_State.Connect_State &= ~kvm_bit;
	if (value)
		CascadeS_KVM_State.Connect_State |= kvm_bit;
	Cascade_Compare_State(old_value,value);	
}

/*----------------------------------------------------------------------------
 * void CascadeS_Port_Power_State(U8_T port,U8_T value)
 * Purpose: Send  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeS_Port_Power_State(U8_T port,U8_T value)
{
	U8_T old_value;
	
	//printf("Power:%bx-%bx\n\r",port,value);	
	old_value = CascadeS_KVM_State.Power_State & BIT_MASK[port];		
	CascadeS_KVM_State.Power_State &= ~BIT_MASK[port];
		
	if (value)	
	{		
		CascadeS_KVM_State.Power_State |= BIT_MASK[port];
	}	
	else
	{
		CascadeS_KVM_State.Connect_State &= ~BIT_MASK[port];
		CascadeS_KVM_State.Suspend_State &= ~BIT_MASK[port];
		CascadeS_KVM_State.Connect_State &= ~(BIT_MASK[port] << 4);	//kvm state
	}
	Cascade_Compare_State(old_value,value);	
}

/*----------------------------------------------------------------------------
 * void CascadeS_Port_Suspend_State(U8_T port,U8_T value)
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeS_Port_Suspend_State(U8_T port,U8_T value)
{
	U8_T old_value;
	
	//printf("Suspend:%bx-%bx\n\r",port,value);	
	old_value = CascadeS_KVM_State.Suspend_State & BIT_MASK[port];	
	CascadeS_KVM_State.Suspend_State &= ~BIT_MASK[port];
	
	if (value)
		CascadeS_KVM_State.Suspend_State |= BIT_MASK[port];
		
	Cascade_Compare_State(old_value,value);
}

/*----------------------------------------------------------------------------
 * void CascadeS_KVM_Led_Control_Handle(CASCADE_PacketHeader *dp) 
 * Purpose: 
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void CascadeS_KVM_Led_Control_Handle(CASCADE_PacketHeader *dp) 	
{	
	
	if (dp->bCommand == CASCADE_KVM_SELECT_LED)
	{
		//KVM_CONSOLE_Select_Led_Control(dp->bIndex,dp->bValue);
	}		
	else
	{
		//KVM_Host_Led_Control(dp->bIndex,dp->bValue);
	}										
}

/*-------------------------------------------------------------------------------------
 * void CascadeS_API_Device_Config_Setting_Handle(CASCADE_PacketHeader *Data_Token)
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *------------------------------------------------------------------------------------*/
void CascadeS_API_Device_Config_Setting_Handle(CASCADE_PacketHeader *Data_Token)
{	
	U8_T  devinx;
	U8_T  remote_devinx;
	
	remote_devinx = Data_Token->bIndex;
	devinx = Remote_Reserve_DevMap[remote_devinx];				
#if (CASCADES_CMD_DEBUG)	
	printf(" <S>-[Device(%bx),Configure,IDLE:(%02x),Config(%02x)]\n\r",devinx,(U16_T)Data_Token->bRequest,(U16_T)Data_Token->bValue);
	//printf(" [R-%d,S-%d]\n\r",(U16_T)remote_devinx,(U16_T)devinx);
#endif /*TRANSMITTER_CMD_DEBUG*/
	USB_PDevice[devinx].Hid_SetIdle_Allow  = Data_Token->bRequest;
	USB_PDevice[devinx].ConfigurationValue = Data_Token->bValue;	
}

/*-------------------------------------------------------------------------------------
 * void CascadeS_Virtual_Device_Suspend_Check(CASCADE_PacketHeader *Data_Token)
 * Purpose:  
 * Params:  NONE
 * Returns: NONE
 * Note:
 *------------------------------------------------------------------------------------*/
void CascadeS_Virtual_Device_Suspend_Check(void)
{
	U8_T index;
	
	for (index=0; index < KVM_MAX_PORT ; index++)
	{
		if (USBDC_UpPortState[index] & USBDC_ROOTHUB_SUSPEND_MASK)
		{
			if (USBDC_UpPortState[index] & USBDC_ROOTHUB_ATTACHED_MASK)
			{
				USBDC_Port_Resume(index);
			}
		}	
	}
}		
#endif /* SYSTEM_CASCADE_SLAVE */
/* End of cascade_slave.c */
