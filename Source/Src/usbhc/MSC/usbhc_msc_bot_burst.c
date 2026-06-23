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
 * Module Name : usbhc_msc_bot_burst.c
 * Purpose     : The USB host msc(mass storage class) class driver
 *               Current only support BBB(BOT-Bulk only transfer) 
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	<stdio.h>
#include	"project_include.h"

#if (SYSTEM_USB_HC_BURST)
#if (USB_HC_MSC_BOT_SUPPORT)

/* STATIC VARIABLE DECLARATIONS */
/* GLOBAL VARIABLE DECLARATIONS */

/* LOCAL SUBPROGRAM DECLARATIONS */
/* LOCAL SUBPROGRAM BODIES */
extern const U8_T	DA_CR[USBDC_DEVICE_MAX];

/* GLOBAL EXTERNAL DECLARATIONS */

/*******************************************************************************
* RESULT USBHC_MSC_BOT_Done_Handle(U8_T devinx, U8_T td_id, U8_T jump)
* Description    : handle MSC class out endp event
* Input          :  
* Output         : None.
* Return         : None
*******************************************************************************/
RESULT USBHC_MSC_BOT_Done_Handle(U8_T devinx, U8_T atl_id, U8_T jump)
{
	U8_T	pinpong_td;
	U16_T	len;

	if (USBHC_MSC_Device_Error_Condition_Check(devinx) ||
		USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_IDLE)
	{
		return USB_SUCCESS;
	}
	
	if (jump)
	{				
		//maintain data toggle bit
		if ((HCTD_Table.BULK[atl_id].TD.Byte1 & TD_TOGGLE_MASK) == TD_TOGGLE_DATA1) //data1
		{
			USB_PDevice[devinx].MSC->Toggle_In = 1;
		}
		else
		{
			USB_PDevice[devinx].MSC->Toggle_In = 0;
		}
		goto ERROR_HANDLE;
	}

	if ((HCTD_Table.BULK[atl_id].TD.Byte5 & TD_DIR_TOKEN_MASK) == TD_DIR_TOKEN_IN) // Handle the IN Token
	{
		/* Endp Buffer Direction IN */
		len = ((U16_T)(HCTD_Table.BULK[atl_id].TD.Byte1 & 0x03) << 8) + (U16_T)HCTD_Table.BULK[atl_id].TD.Byte0_Actual_Byte; //get input byte		
		
		/*BURST_DEBUG*/
		USB_PDevice[devinx].MSC->BURST_In_State |= USBHC_MSC_Burst_Done[atl_id]; // In TD is Done				

		//maintain data toggle bit
		if ((HCTD_Table.BULK[atl_id].TD.Byte1 & TD_TOGGLE_MASK) == TD_TOGGLE_DATA1) //data1
		{
			USB_PDevice[devinx].MSC->Toggle_In = 1;
		}
		else
		{
			USB_PDevice[devinx].MSC->Toggle_In = 0;
		}
        
		//1.Send Complete, reset the input & output index pointer
		if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_END) //the last data is csw IN,
		{
			/*BURST_DEBUG*/
			USBHC_MSC_Active_State &= ~(MSC_ACTIVE_TEST_UNIT_READY|MSC_ACTIVE_BUSY);
			USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_IDLE;
		}
		else if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_DATA_IN) //the last data is csw IN,
		{
			//maintainment the remained data length
			if (len % MSC_Class_Page_Size) /* all IN data has been transfer by MSC device */
			{
				if (USB_PDevice[devinx].MSC->Control.Total_Length < len)
				{
					USBHC_MSC_Active_State &= ~(MSC_ACTIVE_TEST_UNIT_READY|MSC_ACTIVE_BUSY);
					USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_IDLE;					
				}				
				USB_PDevice[devinx].MSC->Control.Total_Length = 0;					
			}
			else if (USB_PDevice[devinx].MSC->Control.Total_Length > len) 
			{
				USB_PDevice[devinx].MSC->Control.Total_Length -= len;
			}
			else
			{
				USB_PDevice[devinx].MSC->Control.Total_Length = 0;
			}
			
			if (USB_PDevice[devinx].MSC->BOT_State != MSC_BOT_IDLE)		
			{	
				if (USB_PDevice[devinx].MSC->Control.Total_Length == 0) //if the in is done
				{
					//Next shoud be CSW
					USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_CSW_REQUEST;
				}
			}	
		}

		//Check is there any DC wait for IN, if yes then wait
		pinpong_td = USBHC_MSC_GetNext_PingPongIn_TD(atl_id);		

ERROR_HANDLE:
				
		if ((USB_PDevice[devinx].MSC->BURST_In_State & USBHC_MSC_Burst_Done[pinpong_td]) == 0x00) //if next has not been used
		{
			USBHC_MSC_BOT_Check_Next_In(devinx, atl_id);

			if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_DATA_IN)
			{
				if (USB_PDevice[devinx].MSC->Control.Total_Length > MSC_Class_Page_Size)
				{
					len = MSC_Class_Page_Size;
				}
				else
				{
					len = USB_PDevice[devinx].MSC->Control.Total_Length;
				}
				
				USBHC_MSC_PingPongIn_ID = pinpong_td +  USB_HC_MSC_START;				
				USBHC_MSC_Bulk_TD_Header(devinx, pinpong_td, MSC_BOT_DATA_IN, len);
				USB_PDevice[devinx].MSC->BURST_In_State |= USBHC_MSC_Burst_Used[pinpong_td]; // In TD in Used
			}
			else if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_CSW_REQUEST)
			{
				USBHC_MSC_BOT_Send_CSW_Request(devinx,pinpong_td);
			}
		}
		else
		{
			USB_PDevice[devinx].MSC->BURST_In_State |= BURST_IN_WAIT;
		}
	} // End of if ((HCTD_Table.ATL[atl_id].TD.Byte5 & TD_DIR_TOKEN_MASK) == TD_DIR_TOKEN_IN)
	else
	{
		/* Endp Buffer Direction OUT */
		USB_PDevice[devinx].MSC->BURST_Out_State &= ~(USBHC_MSC_Burst_Used[atl_id]|USBHC_MSC_Burst_Done[atl_id]); //clear the used and done flag first

		len = ((U16_T)(HCTD_Table.BULK[atl_id].TD.Byte1 & 0x03) << 8) + (U16_T)HCTD_Table.BULK[atl_id].TD.Byte0_Actual_Byte; //get input byte

		//maintain data toggle bit
		if ((HCTD_Table.BULK[atl_id].TD.Byte1 & TD_TOGGLE_MASK) == TD_TOGGLE_DATA1) //data1
		{
			USB_PDevice[devinx].MSC->Toggle_Out = 1;
		}
		else
		{
			USB_PDevice[devinx].MSC->Toggle_Out = 0;
		}

		//1.Check Page Complete condition
		if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_END) //is the last TD
		{
			USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_IDLE;
		}
		else // handle the remain package
		{
			if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_CSW_REQUEST)
			{				
				pinpong_td = USBHC_MSC_EnableNextBurstTD(MSC_BOT_DATA_IN);
				USBHC_MSC_BOT_Send_CSW_Request(devinx, pinpong_td);

			}
			else if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_DATA_IN)
			{
				pinpong_td = USBHC_MSC_PingPongIn_ID - USB_HC_MSC_START; //Get Current TD
				USB_PDevice[devinx].MSC->BURST_In_State |= USBHC_MSC_Burst_Used[pinpong_td];
				USBHC_MSC_Bulk_TD_Header(devinx, pinpong_td, MSC_BOT_DATA_IN, USB_PDevice[devinx].MSC->Control.Data_Length);
			}
			else //if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_DATA_OUT)
			{
				pinpong_td = USBHC_MSC_GetNext_PingPongOut_TD(atl_id);			
				if ((USB_PDevice[devinx].MSC->BURST_Out_State & USBHC_MSC_Burst_Done[pinpong_td])) //if next wait, then send it out
				{
					USBHC_MSC_BOT_Next_HC_Out(devinx, atl_id, pinpong_td);
				}
			}
		}
	}
	return USB_SUCCESS;
}

/****************************************************************************
 * void USBHC_MSC_BOT_DC_Data_Out_Handle(U8_T devinx)
 *
 * @brief  Handle the Data out of BOT
 * @param  
 * @retval none
 */
RESULT USBHC_MSC_BOT_DC_Data_Out_Handle(U8_T devinx)
{
	U8_T 	bulk_td;
	U8_T 	free_pingpong;
	U16_T	len;

	//Get the total length
	//Get necessary information about the transaction.
	//1.Get bulk td buffer id
	bulk_td = USBHC_MSC_PingPongOut_ID - USB_HC_MSC_START; //Get the BULK TD index
	
	//2.Get data package length
	USBHC_MSC_Get_Burst_Length_From_DC(devinx, bulk_td);
	len = USB_PDevice[devinx].MSC->Control.Data_Length;
	
    //3.Get data package buffer pointer    
	switch (USB_PDevice[devinx].MSC->BOT_State)
	{
		case MSC_BOT_IDLE: //wait for CBW
			//1.check the CBW valid condition
			if (USBHC_MSC_BOT_CBW_Decode(devinx, len, &HCTD_Table.BULK[bulk_td].Buf[2]) == USB_SUCCESS)
			{
				USBHC_MSC_Flush_DC_Buffer(KVM_CurrentMSC, devinx, USB_PDevice[devinx].MSC->In_Endpinx);
				//Send out CBW to HC first.
				USBHC_MSC_Bulk_TD_Header(devinx,bulk_td,MSC_BOT_DATA_OUT,USBHC_MSC_BOT_CBW_LENGTH); // active the first header,for cbw output
				USB_PDevice[devinx].MSC->BURST_Out_State |= (USBHC_MSC_Burst_Used[bulk_td] | USBHC_MSC_Burst_Done[bulk_td]);
				if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_DATA_OUT) //Generate the first page for ATL TD INPUT
				{					
					USBHC_MSC_Active_State |= MSC_ACTIVE_BUSY; 			//Transaction start
					len = USB_PDevice[devinx].MSC->Control.Data_Length;				
				}
				else if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_DATA_IN) 
				{					
					USBHC_MSC_Active_State |= MSC_ACTIVE_BUSY;		//Transaction start
					len = USBHC_MSC_BOT_CBW_LENGTH;
				}
				else //if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_CSW_REQUEST)
				{
					USBHC_MSC_Active_State |= MSC_ACTIVE_TEST_UNIT_READY;//Check Unit Ready
					len = USBHC_MSC_BOT_CSW_LENGTH;
				}

				free_pingpong = USBHC_MSC_EnableNextBurstTD(MSC_BOT_DATA_OUT);	//Assign next Burst TD to Data out
				USBHC_MSC_Set_DC_Burst_Length(free_pingpong, len);

				USB_PDevice[devinx].MSC->BURST_Out_State |= (BURST_COMPLETE | USBHC_MSC_Burst_Used[free_pingpong]);
			}
			else
			{
				USB_PDevice[devinx].MSC->BURST_Out_State = (BURST_ERROR | USBHC_MSC_Burst_Used[bulk_td]);
				return USB_ERROR;	// indicate the CBW command error
			}
			break;
		case MSC_BOT_DATA_OUT:
			//Get the DC burst data package length
			//Maintain the remain total length
			if (USB_PDevice[devinx].MSC->Control.Total_Length > len)
				USB_PDevice[devinx].MSC->Control.Total_Length -= len;
			else
				USB_PDevice[devinx].MSC->Control.Total_Length = 0;

			USB_PDevice[devinx].MSC->BURST_Out_State |= (USBHC_MSC_Burst_Used[bulk_td] | USBHC_MSC_Burst_Done[bulk_td]);

			//1.Check HC state,if the HC is not send another TD, then active current TD
			free_pingpong = USBHC_MSC_GetNext_PingPongOut_TD(bulk_td);			

			if ((USB_PDevice[USBHC_MSC_Devinx].MSC->BURST_Out_State & USBHC_MSC_Burst_Done[free_pingpong]) == 0x00)
			{
				USBHC_MSC_Bulk_TD_Header(devinx,bulk_td,MSC_BOT_DATA_OUT,len); // active the current TD header,for data output
				USB_PDevice[devinx].MSC->BURST_Out_State |= (BURST_COMPLETE | USBHC_MSC_Burst_Used[free_pingpong]); //Used & Complete
				USBHC_MSC_EnableNextBurstTD(MSC_BOT_DATA_OUT); //Change PingPong ID
				if (USB_PDevice[devinx].MSC->Control.Total_Length)
				{
					USBHC_MSC_BOT_Get_Next_Burst_Length(devinx); //set default length
					USBHC_MSC_Set_DC_Burst_Length(free_pingpong, USB_PDevice[devinx].MSC->Control.Data_Length);
				}
				else
				{
					USBHC_MSC_Set_DC_Burst_Length(free_pingpong, USBHC_MSC_BOT_CBW_LENGTH);
					USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_CSW_REQUEST; //Next,wait for CSW
				}
			}
			break;
		case MSC_BOT_CSW_REQUEST:
			break;
		default:
			//printf("UNKNOW Status=%02x,%02x\n\r",(U16_T)USB_PDevice[devinx].MSC->BOT_State,(U16_T)(U16_T)USB_PDevice[devinx].MSC->BURST_Out_State);
			break;
	}
	return USB_SUCCESS;
}

/****************************************************************************
 * void USBHC_MSC_BOT_CBW_Decode(U8_T devinx,U16_T len,U8_T *cbw)
 *
 * @brief  Decode the CBW command and set the BOT state machine accordingtly  
 * @param  
 * @retval none
 */
RESULT USBHC_MSC_BOT_CBW_Decode(U8_T devinx,U16_T len,U8_T *buf)
{
	MSC_BOT_CBW_TypeDef* cbw = (MSC_BOT_CBW_TypeDef*)buf;

	//1.Check the CBW header length	& Signature First
	if ((len != USBHC_MSC_BOT_CBW_LENGTH) || (cbw->dSignature != USBHC_MSC_BOT_CBW_SIGNATURE))
	{
		USB_PDevice[devinx].MSC->BURST_Out_State = BURST_ERROR;
		return USB_ERROR; //indidate the CBW command error
	}

	//2.Check next BOT State
	USB_PDevice[devinx].MSC->Control.Total_Length = Endian_32_Convert(cbw->dDataLength);
	if (USB_PDevice[devinx].MSC->Control.Total_Length)
	{
		if (cbw->bmFlags & BOT_DIR_IN)
		{
			USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_DATA_IN; //Request data from HC
#if (USB_MSC_CLASS_DEBUG_MODE)			
			printf("R");
#endif
		}
		else
		{
			USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_DATA_OUT; //Send data to HC
#if (USB_MSC_CLASS_DEBUG_MODE)
			if (USB_PDevice[devinx].MSC->Control.Total_Length)
			{
				printf("\n\r");
			}	
			printf("W");
#endif
		}
		//For next Burst Mode Data Length
		if (USB_PDevice[devinx].MSC->Control.Total_Length > MSC_Class_Page_Size)
		{
			USB_PDevice[devinx].MSC->Control.Data_Length = MSC_Class_Page_Size;
		}
		else
		{
			USB_PDevice[devinx].MSC->Control.Data_Length = USB_PDevice[devinx].MSC->Control.Total_Length;
		}
#if (USB_MSC_CLASS_DEBUG_MODE)
		printf("(%ld)",USB_PDevice[devinx].MSC->Control.Total_Length);
#endif
	}
	else
	{
		USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_CSW_REQUEST; //wait HC issue a IN Token in Bulk endp IN
		USB_PDevice[devinx].MSC->Control.Data_Length = USBHC_MSC_BOT_CSW_LENGTH;
#if (USB_MSC_CLASS_DEBUG_MODE)
		printf("=");
#endif
	}
	return USB_SUCCESS;
}

/****************************************************************************
 * void USBHC_MSC_BOT_Send_CSW_Request(U8_T devinx,U8_T current_td)
 *
 * @brief : Send out the CSW request to device
 * @param  
 * @retval none
 */
void USBHC_MSC_BOT_Send_CSW_Request(U8_T devinx,U8_T current_td)
{
	USBHC_MSC_Bulk_TD_Header(devinx,current_td,MSC_BOT_DATA_IN,USBHC_MSC_BOT_CSW_LENGTH);
	USB_PDevice[devinx].MSC->BURST_In_State |= USBHC_MSC_Burst_Used[current_td]; // In TD in Used
	USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_END;
}

/****************************************************************************
 * void USBHC_MSC_BOT_Check_Next_In(U8_T devinx,U8_T atl_id)
 *
 * @brief : send current DC and enable next DC
 * @param  
 * @retval none
 */
void USBHC_MSC_BOT_Check_Next_In(U8_T devinx,U8_T atl_id)
{
	U8_T  op_state;

	/*Enable DC Buffer ready for IN */
	HCTD_Table.BULK[atl_id].Buf[0] = HCTD_Table.BULK[atl_id].TD.Byte0_Actual_Byte;	//get input low byte
	HCTD_Table.BULK[atl_id].Buf[1] = (HCTD_Table.BULK[atl_id].TD.Byte1 & 0x03);		//get input high byte
	USBHC_MSC_EnableCurrentBurstTD(MSC_BOT_DATA_IN,atl_id);
	USBHC_MSC_UpstreamInTD = atl_id + USB_HC_MSC_START;		
	
	op_state = DA_CR_BVLD_SET | (USB_PDevice[devinx].MSC->In_Endpinx & 0x07);	
	EXTINT2_DISABLE;
	USBDC_INT_DISABLE;
	switch (KVM_CurrentMSC)
	{
		case 0:
			_USBDC_DC0DR_SFR(op_state);
			_USBDC_DC0CIR_SFR(DA_CR[devinx]);
			break;
		case 1:
			_USBDC_DC1DR_SFR(op_state);
			_USBDC_DC1CIR_SFR(DA_CR[devinx]);
			break;
		case 2:
			_USBDC_DC2DR_SFR(op_state);
			_USBDC_DC2CIR_SFR(DA_CR[devinx]);
			break;
		case 3:
			_USBDC_DC3DR_SFR(op_state);
			_USBDC_DC3CIR_SFR(DA_CR[devinx]);
			break;
	}
	USBDC_INT_ENABLE;
	EXTINT2_ENABLE;
}

/****************************************************************************
 * void USBHC_MSC_CSW_After_Control(U8_T devinx)
 *
 * @brief : Special handle for after host send out a stall clear feature command
 *          the MSC device need send back a CSW of the result. Then assign the
 *          proper td id to Next CBW DC transaction.
 * @param  
 * @retval none
 */
void USBHC_MSC_CSW_After_Control(U8_T devinx)
{
	U8_T current_td;

	current_td = USBHC_MSC_EnableNextBurstTD(MSC_BOT_DATA_IN);
	USBHC_MSC_BOT_Send_CSW_Request(devinx,current_td);
}

/*******************************************************************************
* void USBHC_MSC_BOT_Get_Next_Burst_Length(U8_T devinxd)
* Description    : Calcute next Burst transcation for DC/HC
* Input          :  
* Output         : None.
* Return         : None
*******************************************************************************/ 
void USBHC_MSC_BOT_Get_Next_Burst_Length(U8_T devinx)
{
	U16_T len;

	//For next Burst Mode Data Length
	if (USB_PDevice[devinx].MSC->Control.Total_Length > MSC_Class_Page_Size)
	{
		len = MSC_Class_Page_Size;
	}
	else
	{
		len = USB_PDevice[devinx].MSC->Control.Total_Length;
	}

	USB_PDevice[devinx].MSC->Control.Data_Length = len;
}

/*******************************************************************************
* void USBHC_MSC_BOT_Next_HC_Out(U8_T devinx,U8_T current_td,U8_T next_td)
* Description    : Send out the HC
* Input          :
* Output         : None.
* Return         : None
*******************************************************************************/
void USBHC_MSC_BOT_Next_HC_Out(U8_T devinx,U8_T current_td,U8_T next_td)
{
	U8_T endpinx;

	endpinx = USB_PDevice[devinx].MSC->Out_Endpinx;
	//1.Next Send out the Done Bulk TD
	USBHC_MSC_Get_Burst_Length_From_DC(devinx,next_td);
	USBHC_MSC_Bulk_TD_Header(devinx,next_td,MSC_BOT_DATA_OUT,USB_PDevice[devinx].MSC->Control.Data_Length); // active the first header,for cbw output
		
	//2.Need Assing Current to DC
	USBHC_MSC_EnableNextBurstTD(MSC_BOT_DATA_OUT); //Assign next Burst TD to Data out
	USB_PDevice[devinx].MSC->BURST_Out_State |= USBHC_MSC_Burst_Used[current_td]; //if next td has been done by dc

	if (USB_PDevice[devinx].MSC->Control.Total_Length)
	{
		USBHC_MSC_BOT_Get_Next_Burst_Length(devinx); //set default length
		USBHC_MSC_Set_DC_Burst_Length(current_td, USB_PDevice[devinx].MSC->Control.Data_Length);
	}
	else
	{
		USBHC_MSC_Set_DC_Burst_Length(current_td, USBHC_MSC_BOT_CBW_LENGTH);

		USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_CSW_REQUEST; //Next,wait for CSW
	}
	USBDC_REGS_Endp_ControlClear(KVM_CurrentMSC,devinx,endpinx,DA_CR_BCLR_SET); // for the out can receive data again
}

#endif /* End of USB_HC_MSC_BOT_SUPPORT */
#endif /* #if (SYSTEM_USB_HC_BURST) */

/* End of usbhc_msc_bot_burst.c */


