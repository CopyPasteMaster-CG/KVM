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
 * Module Name : usbhc_hub.c
 * Purpose     : The USB host hub class driver
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	<string.h>
#include	<stdio.h>
#include	"project_include.h"

/* STATIC VARIABLE DECLARATIONS */
U8_T	USBHC_HUB_Enumerate_Occupy_Flag;
U8_T	USBHC_HUB_Enumerate_Occupy_Devinx;
U16_T	USBHC_HUB_Feature; // feature
U8_T	USBHC_HUB_FeatureType; // feature
#if (USBHC_HUB_DEBUG_MODE)
U16_T   Debug_inx = 0;
#endif
static U8_T	usbhc_HUB_Port;
static U8_T usbhc_Address_Table;
U8_T	TASK_USBHC_HUB_Port_Reset_ID;

TASK_ActiveTable_TypeDef TASK_USBHC_HUB_Class_Enumerate_Task=
{
	TASK_TYPE_EVENT, // Task ID from 0~255
	0, // Task ID from 0~255
	0, // Task Wait for Semaphore to active
	0, // Task Event
	0, // Task transfer parameter
	0, // Task Interval time from 0~65535(ms/us)
	0, // Task Interval Reload
};

/* LOCAL SUBPROGRAM BODIES */
void TASK_USBHC_HUB_Class_Enumerate(void);
RESULT USBHC_Class_GetHubDesc(U8_T devinx,U8_T td_id);
RESULT USBHC_Class_HubFeature(U8_T devinx,U8_T td_id,U8_T usbhc_hub_port,U8_T usbhc_hub_featuretype,U16_T usbhc_hub_feature);
static RESULT USBHC_HUB_Status_Handle(U8_T devinx,U8_T portid);
static void usbhc_HUB_Control_Prepare(U8_T devinx, U8_T hub_port,U8_T hub_featuretype,U16_T hub_feature);
static U8_T usbhc_HUB_GetNewAddress(void);
static U8_T usbhc_Check_Enumerate_Available(U8_T,U8_T);
static void TASK_USBHC_HUB_Port_Reset(void);

/****************************************************************************
 * @brief  USBHC_HUB_Class_Enum_Init
 *         init the HUB Class Enumerate Task
 * @param  None
 * @retval None
 */
void USBHC_HUB_Class_Enum_Init(void)
{
	TASK_USBHC_HUB_Class_Enumerate_Task.Task_ID = TASK_Create(TASK_USBHC_HUB_Class_Enumerate);
	TASK_USBHC_HUB_Port_Reset_ID = TASK_Create(TASK_USBHC_HUB_Port_Reset);
	usbhc_Address_Table = 0;
	USBHC_HUB_Enumerate_Occupy_Flag = 0;
	USBHC_HUB_Enumerate_Occupy_Devinx = 0;
	
#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("TASK_USBHC_HUB_Class_Enumerate_Task.Task_ID=%bu\n\r",TASK_USBHC_HUB_Class_Enumerate_Task.Task_ID);
	printf("TASK_USBHC_HUB_Port_Reset_ID=%bu\n\r",TASK_USBHC_HUB_Port_Reset_ID);
#endif
}

/****************************************************************************
 * @brief  TASK_USBHC_HUB_Class_Enumerate
 *         Handles the USB Class Enumerate relative control machine
 * @param  none
 * @retval none
 */
void TASK_USBHC_HUB_Class_Enumerate(void)
{
	U8_T	devinx,td_id,hub_ports;
	RESULT	Result;

	devinx = TASK_Register0;

	//Skip task if the device is gone
	if ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00)  // Device is gone  
	{
		return;	
	}

TASK_USBHC_HUB_Enumerate_Handle_Restart:
	td_id = USB_PDevice[devinx].Hc.Control.TdNum;
	switch (USB_PDevice[devinx].Hc.HUB_EnumState)
	{
		case HUB_ENUM_GET_DESC: //Start to Get device description
			Result = USBHC_Class_GetHubDesc(devinx,td_id);
			break;
		case HUB_SET_FEATURE: //Start to Get device description
		case HUB_CLEAR_FEATURE: //Start to Get device description
		case HUB_GET_PORT_STATUS: //Start to Get device description
			Result = USBHC_Class_HubFeature(devinx,td_id,usbhc_HUB_Port,USBHC_HUB_FeatureType,USBHC_HUB_Feature);
			if (Result == USB_SUCCESS)
			{
				if (USB_PDevice[devinx].HUB.HubStatus)
				{
					if ((USB_PDevice[devinx].Hc.HUB_EnumState == HUB_SET_FEATURE) &&
						(USBHC_HUB_Feature == PORT_POWER))
					{
						hub_ports = USB_PDevice[devinx].Hub_NbrPorts & USBHC_HUBPORT_MASK;
						usbhc_HUB_Port++;
						if (usbhc_HUB_Port >= hub_ports) //the power on init stage is complete, go next
						{
							USB_PDevice[devinx].HUB.HubStatus = HUB_POWER_PORT;
							USB_PDevice[devinx].Hc.HUB_EnumState = HUB_ENUM_IDLE;
						}
					}
				}
				else
				{
					if (USB_PDevice[devinx].Hc.HUB_EnumState == HUB_GET_PORT_STATUS)
					{
						Result = USBHC_HUB_Status_Handle(devinx,usbhc_HUB_Port);
						if (Result == USB_BUSY)
						{
							Result = USB_SUCCESS; // go for next command
						}
						else
						{
							if (Result != USB_WAIT) // Important
							{
								USB_PDevice[devinx].Hc.HUB_EnumState = HUB_ENUM_IDLE;
							}
						}
					}
					else if (USB_PDevice[devinx].Hc.HUB_EnumState == HUB_CLEAR_FEATURE)
					{
						usbhc_HUB_Control_Prepare(devinx, usbhc_HUB_Port,GET_STATUS,0x00); // get port status
						USB_PDevice[devinx].Hc.HUB_EnumState = HUB_GET_PORT_STATUS;
					}
					else if (USB_PDevice[devinx].Hc.HUB_EnumState == HUB_SET_FEATURE)
					{
						if (USBHC_HUB_Feature == PORT_RESET) //no further operation.
						{
							USB_PDevice[devinx].Hc.HUB_EnumState = HUB_ENUM_IDLE;
						}
					}
				}
			}
			break;
		default:
			break;
	}

	if ((USB_PDevice[devinx].Hc.Control.State == CTRL_FAIL) ||
		(USB_PDevice[devinx].Hc.HUB_EnumState == HUB_ENUM_IDLE) ||
		(USB_PDevice[devinx].Hc.HUB_EnumState == HUB_ENUM_FAIL))
	{
		USBHC_CORE_Free_TD(td_id);
		USB_PDevice[devinx].Hc.HUB_EnumState = HUB_ENUM_IDLE;
		USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE; 
		USB_PDevice[devinx].Hc.gState = USB_PDevice[devinx].HUB.HC_qt_Bk;
		if (USB_PDevice[devinx].Hc.gState == HOST_CLASS_ENUMERATION)
		{
			USB_PDevice[devinx].Hc.gState = HOST_ENUMLATE_TERMINATED;
		}
		TASK_Active(TASK_TYPE_USB,TASK_USBHC_Process_ID,0x00,devinx,0,0); // Generate the task
		return;
	}

	if (Result == USB_SUCCESS)
		goto TASK_USBHC_HUB_Enumerate_Handle_Restart;
}

/**
* @brief  USBHC_Classreq_GetHubDesc
*         Issue Get Hub Descriptor command to the device. Once the response 
*         received, parse the report descriptor and update the status.
* @param  pdev   : Selected device
* @param  Length : HUB Descriptor Table Length
* @retval USBHC_Status : Response for USB HID Get Report Descriptor Request
*/
USBH_Status USBHC_Classreq_GetHubDesc (U8_T devinx,U8_T td_id,U16_T length)
{
	//1.TD Parameter
	HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE;
	HCTD_Channel_Table[td_id].Total_Length = length; // no data stage
	HCTD_Channel_Table[td_id].Current_Length = 0;
	//2.Descriptor Read
	return USBHC_GetDescriptor(devinx,
							   td_id,
							   USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_CLASS,
							   0x2900,
							   0x00,
							   length);
}

/*----------------------------------------------------------------------------
 * CTRL_State USBHC_Class_GetHubDesc(U8_T devinx,U8_T td_id)
 * Purpose : Perform the set address operation of enumearation
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
RESULT USBHC_Class_GetHubDesc(U8_T devinx,U8_T td_id)
{
	RESULT					Result=USB_ERROR;
	U8_T					buf_len;
	USBHC_HUBDec_Typedef	*phubdec;
	
	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		if (USBHC_Control_TD_Init(devinx,&td_id) == USBH_OK)
		{
			//1.Get dynamic buffer memory
			USB_PDevice[devinx].Hc.Control.Buf = m_malloc(9,13); /* allocate 9 bytes */
			HCTD_Channel_Table[td_id].Buf = USB_PDevice[devinx].Hc.Control.Buf; // also assign to host channel buffer
			if (USB_PDevice[devinx].Hc.Control.Buf == NULL)
			{
				USBHC_CORE_Free_TD(td_id);
				USBHC_Memory_Fail_Msg(0x04);
				USB_PDevice[devinx].Hc.HUB_EnumState = HUB_ENUM_FAIL;
			}
			else
			{
				//Send out Setup Control Token
				USBHC_Classreq_GetHubDesc(devinx,td_id,0x08);
				Result=USB_BUSY;
			}
		}
		else
		{
			//printf("Hub: Failed to allocate TD\r\n");
			TASK_Wait_Current();
		}
	}
	else
	{
		//parser descri_ptor
		if ((USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE) ||
		(USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED))
		{
			//Copy HUB Descriptor into PUSB Table
			USB_PDevice[devinx].Desc[HUB_DESC].Len = 9; // store the length
			USB_PDevice[devinx].Desc[HUB_DESC].Ptr = USB_PDevice[devinx].Hc.Control.Buf;
			USB_PDevice[devinx].Desc[HUB_DESC].Ptr[8] = 0xFF;
			USB_PDevice[devinx].Hc.Control.Buf = NULL;
			//Parser Hub Descriptor
			phubdec = (USBHC_HUBDec_Typedef *)USB_PDevice[devinx].Desc[HUB_DESC].Ptr;
			USB_PDevice[devinx].Hub_NbrPorts |= phubdec->bNbrPorts;
			buf_len = sizeof(USBHC_HubReportTypeDef) * phubdec->bNbrPorts;
			USB_PDevice[devinx].HUB.ReportState = m_malloc(buf_len,14);
			memset((U8_T *)USB_PDevice[devinx].HUB.ReportState,0x00,buf_len);
			//Prepare next enumeration operation.
			USBHC_HUB_Feature = PORT_POWER;
			USBHC_HUB_FeatureType= SET_FEATURE;
			usbhc_HUB_Port = 0;
			USB_PDevice[devinx].HUB.HubStatus = HUB_INIT_POWER;
			USB_PDevice[devinx].Hc.HUB_EnumState = HUB_SET_FEATURE;
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
			Result=USB_SUCCESS;
		}
		else
		{
			//Due to have to release the buffer, so need put the satement here
			//proces alos if the State is not COMPLETE
			malloc_free(USB_PDevice[devinx].Hc.Control.Buf);
			USB_PDevice[devinx].Hc.Control.Buf = 0;
		}
	}
	return Result;
}

/*----------------------------------------------------------------------------
 * RESULT USBHC_Class_HubFeature(devinx,td_id,USBHC_HUB_Port,USBHC_HUB_FeatureType,USBHC_HUB_Feature
 * Purpose : Perform the HUB class Set Feature & Clear Feature command
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
RESULT USBHC_Class_HubFeature(U8_T devinx,U8_T td_id,U8_T usbhc_hub_port,U8_T usbhc_hub_featuretype,U16_T usbhc_hub_feature)
{
	RESULT Result;

	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		//1.TD Parameter
		HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE;
		HCTD_Channel_Table[td_id].Current_Length = 0;
		HCTD_Channel_Table[td_id].Total_Length = 0; // no data stage
		//2.Setup Token
		USB_PDevice[devinx].Hc.Control.Setup.b.bmRequestType = USB_H2D | OTHER_RECIPIENT | CLASS_REQUEST; //0x03-Port
		USB_PDevice[devinx].Hc.Control.Setup.b.bRequest = usbhc_hub_featuretype;
		USB_PDevice[devinx].Hc.Control.Setup.b.wValue.w = usbhc_hub_feature;
		USB_PDevice[devinx].Hc.Control.Setup.b.wIndex.w = usbhc_hub_port+1;
		USB_PDevice[devinx].Hc.Control.Setup.b.wLength.w = 0;
		//3.Check Get_Status Command 
		if (usbhc_hub_featuretype == GET_STATUS)
		{
			USB_PDevice[devinx].Hc.Control.Setup.b.bmRequestType |= USB_D2H;
			HCTD_Channel_Table[td_id].Buf = (U8_T *)(USB_PDevice[devinx].HUB.ReportState+usbhc_hub_port);
			USB_PDevice[devinx].Hc.Control.Setup.b.wLength.w = 4;
			HCTD_Channel_Table[td_id].Total_Length = 4; //Get 4 bytes back
		}
		//4.Send Setup Control Token
		USBDC_HAL_Convert_Setup_Value((U8_T *)&USB_PDevice[devinx].Hc.Control.Setup);
		USBHC_CtlReq(devinx,td_id);
		Result=USB_BUSY;
	}
	else
	{
		if (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED)
		{
			/* Handle device qualifier */
			if (USB_PDevice[devinx].Usb_2_0)
			{
				//1.TD Parameter
				HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE;
				HCTD_Channel_Table[td_id].Current_Length = 0;
				HCTD_Channel_Table[td_id].Total_Length = 10; // no data stage
		
				//2.Setup Token
				USB_PDevice[devinx].Hc.Control.Setup.b.bmRequestType = USB_D2H;
				USB_PDevice[devinx].Hc.Control.Setup.b.bRequest = USB_REQ_GET_DESCRIPTOR;
				USB_PDevice[devinx].Hc.Control.Setup.b.wValue.w = USB_DESC_DEVICE_QUALIFIER;
				USB_PDevice[devinx].Hc.Control.Setup.b.wIndex.w = 0;
				USB_PDevice[devinx].Hc.Control.Setup.b.wLength.w = 10;
		
				//3.Send Setup Control Token
				USBDC_HAL_Convert_Setup_Value((U8_T *)&USB_PDevice[devinx].Hc.Control.Setup);
				USBHC_CtlReq(devinx,td_id);
				Result=USB_BUSY;
			}
		}
		else
		{
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
			Result=USB_SUCCESS;
		}
	}
	return Result;
}

static void usbhc_HUB_Control_Prepare(U8_T devinx, U8_T hub_port, U8_T hub_featuretype,U16_T hub_feature)
{
	USBHC_HUB_Feature = hub_feature;
	USBHC_HUB_FeatureType= hub_featuretype;
	usbhc_HUB_Port = hub_port;
	USB_PDevice[devinx].HUB.HubStatus = HUB_POWER_PORT;
}

/*----------------------------------------------------------------------------
 * void USBHC_HUB_StatusChangeHandle(U8_T devinx,U8_T inttid)
 * Purpose : Handle the HUB Status Changed Interrupt responsed
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
void USBHC_HUB_StatusChangeHandle(U8_T devinx,U8_T intt_id)
{
	U8_T	hub_status_change;
	U8_T	maxport,td_id;
	U8_T	index;
	U8_T	portbit = 0x01;
	U8_T	enubit;

	hub_status_change = HCTD_Table.INTL[intt_id].Buf[0];
	maxport = USB_PDevice[devinx].Hub_NbrPorts & USBHC_HUBPORT_MASK;
	for (index=0; index < maxport; index++)
	{
		portbit <<= 1;
		if (hub_status_change & portbit) // indicate the port status changed
		{
			if (USB_PDevice[devinx].Hub_Enu_Port_Id)
			{
				enubit = (0x01 << USB_PDevice[devinx].Hub_Enu_Port_Id);
				if (enubit != portbit)
				{
					continue;
				}
			}
			//Prepare next enumeration operation.
			usbhc_HUB_Control_Prepare(devinx,index,GET_STATUS,0x00);
			USB_PDevice[devinx].Hc.HUB_EnumState = HUB_GET_PORT_STATUS;
			if (USBHC_Control_TD_Init(devinx,&td_id) == USBH_OK)
			{
				USB_PDevice[devinx].HUB.HC_qt_Bk = USB_PDevice[devinx].Hc.gState;
				USB_Hub_Class_Enumerate_Fork(devinx);
			}
			break;
		}
	}
}

/*----------------------------------------------------------------------------
 * RESULT usbhc_HUB_Status_Handle(U8_T devinx,U8_T portid)
 * Purpose : Handle the HUB Status Changed Interrupt responsed
 * Params  : none
 * Returns : RESULT
 * Note    : none
 * ----------------------------------------------------------------------------*/
void USBHC_HUB_Remove_Downstream_Device(U8_T hubdevinx,U8_T portid,U16_T change_event,U8_T event_type)
{
	U8_T downstream,address;
	
	if ((USB_PDevice[hubdevinx].HUB.ReportState+portid)->Devinx)
	{
#if (USBHC_HUB_DEBUG_MODE)    
		printf(">KVM :(%d)HubDev:%bu,P:%bu,",Debug_inx++,hubdevinx,portid+1);
		if (event_type) //Change Event
			printf("Chagne Event=0x%02x, ",change_event);
		else
			printf("Satate Lose=0x%04x, ",change_event);
		printf("remove device\n\r");	
#else
		change_event = 0;
		event_type = 0;	
#endif			

		downstream = (USB_PDevice[hubdevinx].HUB.ReportState+portid)->Devinx;					
		address = USB_PDevice[downstream].Addr & USBHC_DEVICE_ADDR_MASK;
		USBHC_HUB_ReleaseAddress(address);
		TASK_USBHC_HUB_RemoveDevinx_Fork(downstream);
		(USB_PDevice[hubdevinx].HUB.ReportState+portid)->Devinx = 0;
	}	
}	

/*----------------------------------------------------------------------------
 * RESULT USBHC_HUB_Status_Handle(U8_T devinx,U8_T portid)
 * Purpose : Handle the HUB Status Changed Interrupt responsed
 * Params  : none
 * Returns : RESULT
 * Note    : none
 * ----------------------------------------------------------------------------*/
static RESULT USBHC_HUB_Status_Handle(U8_T devinx,U8_T portid)
{
	U8_T	portchange,free_deviceid;
	U16_T	portstatus;
	
	portchange = (USB_PDevice[devinx].HUB.ReportState+portid)->wPortChangeL;
	portstatus = (USB_PDevice[devinx].HUB.ReportState+portid)->wPortStatusL+((USB_PDevice[devinx].HUB.ReportState+portid)->wPortStatusH <<8); //change edian

    //printf("Devinx:%bu,portid:%bu,Change:%02bx,Status:%04x\n\r",devinx,portid,portchange,(U16_T)portstatus);
	/*1.Check Port Change Status */
	/*01.Connect Changed */
	if (portchange & L_PortChange_Connection)
	{
		if ((portstatus & (HUB_PORT_PORTSTATUS_ENABLE|HUB_PORT_PORTSTATUS_POWER|HUB_PORT_PORTSTATUS_CONNECT)) == ((HUB_PORT_PORTSTATUS_CONNECT|HUB_PORT_PORTSTATUS_POWER)))		
		{
			USBHC_HUB_Remove_Downstream_Device(devinx,portid,(U16_T)L_PortChange_Connection,1);			
		}
		usbhc_HUB_Control_Prepare(devinx,portid,CLEAR_FEATURE,C_PORT_CONNECT);
		USB_PDevice[devinx].Hc.HUB_EnumState = HUB_CLEAR_FEATURE;
		
//#if (SYSTEM_EXTENDER_RECEIVER)
//		if (KVM_CurrentHost != REMOTE_HOST_PORT)
//#else /* SYSTEM_EXTENDER_TRANSMITTER */
		//if (KVM_CurrentHost == 0) /* DC port-0 */
//#endif
//		{
//			USBDC_WakeupUpstreamHost(KVM_CurrentHost);
//		}
		return USB_BUSY;
	}
	
	/*02.Reset Changed*/
	if (portchange & L_PortChange_Reset)
	{
		usbhc_HUB_Control_Prepare(devinx,portid,CLEAR_FEATURE,C_PORT_RESET);
		USB_PDevice[devinx].Hc.HUB_EnumState = HUB_CLEAR_FEATURE;
		return USB_BUSY;
	}
	
	/*04.Enable Changed*/
	if (portchange & L_PortChange_Enable)
	{
		//2106-12-19 Jack, for the device has fail, so need to clear port also        
		USBHC_HUB_Remove_Downstream_Device(devinx,portid,(U16_T)L_PortChange_Enable,1);		
		usbhc_HUB_Control_Prepare(devinx,portid,CLEAR_FEATURE,C_PORT_ENABLE);
		USB_PDevice[devinx].Hc.HUB_EnumState = HUB_CLEAR_FEATURE;
		return USB_BUSY;
	}
	/*2.Port Status Handle*/
	if (portstatus & HUB_PORT_PORTSTATUS_CONNECT) //if device present
	{
		if ((portstatus & (HUB_PORT_PORTSTATUS_ENABLE|HUB_PORT_PORTSTATUS_POWER)) != ((HUB_PORT_PORTSTATUS_ENABLE|HUB_PORT_PORTSTATUS_POWER)))
		{
			USBHC_HUB_Remove_Downstream_Device(devinx,portid,(U16_T)(HUB_PORT_PORTSTATUS_ENABLE|HUB_PORT_PORTSTATUS_POWER),0);									
		}

		if ((portstatus & HUB_PORT_PORTSTATUS_ENABLE) == 0x00) //if device not enable
		{
			if (USBHC_Current_Total_Device_Count < USB_HC_MAX_DEVICE)
			{
				USB_PDevice[devinx].Hub_Enu_Port_Id = portid + 1;
				TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_USBHC_HUB_Port_Reset_ID,devinx,portid,10,10);
				return USB_WAIT;
			}
			else
			{
#if (KVM_BUZZER_SUPPORT)
				BUZZER_Script_Active(DeviceErrorSound);
#endif //#if (KVM_BUZZER_SUPPORT)
				//printf("> KVM ERROR:No Free device, Skip operation\n\r");
			}
		}
		else //need to enumeration .......
		{
			if (portstatus & HUB_PORT_PORTSTATUS_POWER) //if device connect+enable+power
			{
				// do the device enumeration
				if (USB_PDevice[devinx].Hc.ResetCnt > 0)
				{
					USB_PDevice[devinx].Hc.ResetCnt = 0;
					if (USB_HAL_Alloc_Free_PDevice(&free_deviceid, USB_PDevice[devinx].RootHubPort))
					{
						//log the devinx into hub table						
						(USB_PDevice[devinx].HUB.ReportState+portid)->Devinx = free_deviceid;
						
						//Assign device address
						USBHC_Enumerate_New_Addr = usbhc_HUB_GetNewAddress(); // for HUB Layer, start from 0x08~0x0f
						//Store device speed.
						if (portstatus & HUB_PORT_PORTSTATUS_LOWSPEED) // Low Speed
							USB_PDevice[free_deviceid].InterfaceNum |= USBHC_DEVICE_SPEED_MASK;  // store speed type

						//Assign device attatched task
						USB_PDevice[free_deviceid].Hc.gState = HOST_DEV_ATTACHED; // Do the root hub reset process
						USB_PDevice[free_deviceid].UpperHubDevinx = devinx; //store the upper layer hub devinx
						USB_PDevice[free_deviceid].Hub_NbrPorts =  portid USBHC_ROOTHUBPORT_PORT_NUM_SHIFT; // store the port number
						TASK_Active(TASK_TYPE_USB,TASK_USBHC_Process_ID,0,free_deviceid,0,0); // Generate the task
					}
					//else
					//{
					//	printf("> KVM ERROR:No Free device, Skip operation\n\r");
					//}
				}
				else
				{
					TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_USBHC_HUB_Port_Reset_ID,devinx,portid,100,100);
					return USB_WAIT;
				}
			}
			return USB_SUCCESS;
		}
	}
	else
	{		
		if ((portstatus & HUB_PORT_PORTSTATUS_POWER) == 0x00)//if device connect+enable+power
		{				
			free_deviceid = (portid << 4) | PORT_POWER;
			USB_PDevice[devinx].Hc.ScmdStep = SCMD_SET_HUB_FEATURE;			
			TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_Scm_Handle_Start_ID,0,devinx,free_deviceid,0);				
		}
		else
		{		
			if ((portstatus & HUB_PORT_PORTSTATUS_ENABLE) == 0x00) //if device disable, no device present
			{
				USBHC_HUB_Remove_Downstream_Device(devinx,portid,HUB_PORT_PORTSTATUS_ENABLE,0);				
				//Check this port has been assigned a device ?
				//free_deviceid = (USB_PDevice[devinx].HUB.ReportState+portid)->Devinx;
				//if ((USB_PDevice[free_deviceid].Addr&USBHC_DEVICE_USED_MASK) && free_deviceid >= USBDC_VIRTUAL_DEVINX)
				//{
				//	(USB_PDevice[devinx].HUB.ReportState+portid)->Devinx = 0; // clear					
				//	portchange = USB_PDevice[free_deviceid].Addr & USBHC_DEVICE_ADDR_MASK;
				//	USBHC_HUB_ReleaseAddress(portchange);
				//	TASK_USBHC_HUB_RemoveDevinx_Fork(free_deviceid);
				//}
			}
		}
	}
	
	//2016-12-27 add Jack
	//05.Overcurrent Change Handle
	if (portchange & L_PortChange_OverCurrent)
	{		
		//printf("> KVM: HubDevinx:%bu, Port:%bu Overcurrent\n\r",devinx,portid);
		//1.Remove device
		USBHC_HUB_Remove_Downstream_Device(devinx,portid,(U16_T)L_PortChange_OverCurrent,1);
		//2.Clear Overcurrent Change event
		usbhc_HUB_Control_Prepare(devinx,portid,CLEAR_FEATURE,C_PORT_OVER_CURRENT);
		USB_PDevice[devinx].Hc.HUB_EnumState = HUB_CLEAR_FEATURE;
		return USB_BUSY;
	}
	
	//2016-12-27 add Jack
	//06.Power State Change Handle
	if ((portstatus & HUB_PORT_PORTSTATUS_POWER) == 0x00) //if port not in power
	{				
		usbhc_HUB_Control_Prepare(devinx,portid,SET_FEATURE,PORT_POWER);
		USB_PDevice[devinx].Hc.HUB_EnumState = HUB_SET_FEATURE;
		return USB_BUSY;
	}
	return USB_SUCCESS;
}

/*----------------------------------------------------------------------------
 * U8_T usbhc_HUB_GetNewAddress(void)
 * Purpose : This subroutine will get a new address for device in the hub
 *           address scrope is from 8~16
 * Params  : none
 * Returns : RESULT
 * Note    : none
 * ----------------------------------------------------------------------------*/
static U8_T usbhc_HUB_GetNewAddress(void)
{
	U8_T	index;

	for (index=0; index < 8 ; index++)
	{
		if (_ValBit(usbhc_Address_Table, index) == 0x00)
		{
			return index + 8;
		}
	}
	return 0;
}

/*----------------------------------------------------------------------------
 * U8_T USBHC_HUB_LogNewAddress(void)
 * Purpose : After the new address has been enumerat, then log the address 
 *           prevent reused
 * Params  : none
 * Returns : RESULT
 * Note    : none
 * ----------------------------------------------------------------------------*/
void USBHC_HUB_LogNewAddress(void)
{
	U8_T index;	
	
	if (USBHC_Enumerate_New_Addr >= 8)
	{
		index = USBHC_Enumerate_New_Addr-8;
		_SetBit(usbhc_Address_Table,index);
	}
}

/*----------------------------------------------------------------------------
 * U8_T USBHC_HUB_LogNewAddress(void)
 * Purpose : After the new address has been enumerated, then log the address 
 *           prevent reused
 * Params  : none
 * Returns : RESULT
 * Note    : none
 * ----------------------------------------------------------------------------*/
void USBHC_HUB_ReleaseAddress(U8_T addr)
{	
	if (addr >= 8)
	{
		addr -= 8;
		_ClrBit(usbhc_Address_Table,addr);
	}
}

/*----------------------------------------------------------------------------
 * U8_T USBHC_HUB_ReleaseEnumerateOperation(void)
 * Purpose : After the new device under HUB has been enumerated, this will 
 *           release the hub bus occupy condition, so next new devcie 
 *           under HUB can go to enumeration.
 * Params  : none
 * Returns : RESULT
 * Note    : none
 * ----------------------------------------------------------------------------*/
void USBHC_HUB_ReleaseEnumerateOperation(void)
{
	//if (USBHC_HUB_Enumerate_Occupy_Flag)
	{
		USBHC_HUB_Enumerate_Occupy_Flag = 0;
	}
}

/*----------------------------------------------------------------------------
 * U8_T TASK_USBHC_HUB_RemoveDevinx(U8_T devinx)
 * Purpose : This subroutine will remove the devcie under HUB,
 *          
 * Params  : none
 * Returns : RESULT
 * Note    : none
 * ----------------------------------------------------------------------------*/
void TASK_USBHC_HUB_RemoveDevinx(void)
{
	U8_T	devinx,hubports,hubdevinx,index,addr;

	devinx = TASK_Register0;
	TASK_Destory_Current();
	
	if ((devinx < USBDC_VIRTUAL_DEVINX) ||
		(devinx >= (USB_HC_MAX_DEVICE+USB_HC_MAX_HUB)) ||
		!(USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK)) // this should not happened
	{
		printf("HUB_RemoveDevinx: ERROR(%bu,%02bx)\r\n",devinx,USB_PDevice[devinx].Addr);
		return;
	}
	printf("> KVM:Devinx=%bx Class %bx,CLEAR register ..\n\r", devinx, USB_PDevice[devinx].DevClass);	
	if (USB_PDevice[devinx].DevClass == USB_HUB_CLASS)
	{
		hubports = USB_PDevice[devinx].Hub_NbrPorts & USBHC_HUBPORT_MASK;
		/*record every devinx in the hub*/
		for (index=0; index < hubports; index++)
		{
			hubdevinx = (USB_PDevice[devinx].HUB.ReportState+index)->Devinx;
			if (hubdevinx >= USBDC_VIRTUAL_DEVINX)
			{
				(USB_PDevice[devinx].HUB.ReportState+index)->Devinx = 0;
				TASK_USBHC_HUB_RemoveDevinx_Fork(hubdevinx);
				addr = USB_PDevice[hubdevinx].Addr & USBHC_DEVICE_ADDR_MASK;
				USBHC_HUB_ReleaseAddress(addr);
			}
		}
	}

	USBHC_CORE_Clear_Device(devinx);
}

/*----------------------------------------------------------------------------
 * U8_T usbhc_Check_Enumerate_Available(U8_T devinx)
 * Purpose : This subroutine will check theu bus condition for root HUB & HUB
 *          
 * Params  : none
 * Returns : RESULT
 * Note    : none
 * ----------------------------------------------------------------------------*/
static U8_T usbhc_Check_Enumerate_Available(U8_T devinx,U8_T hubportid)
{
	if (USBHC_HUB_Enumerate_Occupy_Flag) /*if no other hub port in enumearation opeartion */
	{
		//printf("hubportid=%bu,USB_PDevice[%bu].Hc.HUB_EnumPort=%bu\n\r",hubportid,devinx,USB_PDevice[devinx].Hc.HUB_EnumPort);
		if (devinx == USBHC_HUB_Enumerate_Occupy_Devinx)
		{	
			if (hubportid != USB_PDevice[devinx].Hc.HUB_EnumPort) 
				return 0;
		}
		else
		{
			return 0;
		}			
	}
	
	if (USBHC_RootHub_Bus_Occupy_Flag) /*if no other hub port in enumearation opeartion */
	{
		return 0;
	}
	return 1;	
}

/*
 * -------------------------------------------------------------------------------------------
 * void TASK_USBHC_HUB_Port_Reset(void)
 * Purpose : Do the Hub port Reset Feature
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------
 */
static void TASK_USBHC_HUB_Port_Reset(void)
{
	U8_T devinx,portid;
	
	devinx = TASK_Event & TASK_EVENT_SERIAL_MASK; // get Port ID
	portid = TASK_Register0; // get Port ID

	if (USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) // if the device is in used
	{
		if (usbhc_Check_Enumerate_Available(devinx,portid))
		{
			USBHC_HUB_Enumerate_Occupy_Devinx = devinx;
			USBHC_HUB_Enumerate_Occupy_Flag=1; 
			USB_PDevice[devinx].Hc.HUB_EnumPort = portid;
			USB_PDevice[devinx].Hc.ResetCnt++;
			usbhc_HUB_Control_Prepare(devinx,portid,SET_FEATURE,PORT_RESET);
			USB_PDevice[devinx].Hc.HUB_EnumState = HUB_SET_FEATURE;

			portid = USB_PDevice[devinx].Hc.Control.TdNum;
			if (((HCTD_Channel_Table[portid].Used_DeviceID & USBHC_TD_USED_MASK) == 0) ||
				((HCTD_Channel_Table[portid].Used_DeviceID & USBHC_TD_DEVICE_ADDR_MASK) != devinx))
			{
				if (USBHC_Control_TD_Init(devinx,&portid) != USBH_OK)
					return;
			}
			USB_Hub_Class_Enumerate_Fork(devinx);
			TASK_Destory_Current();
		}
	}
	else
	{
		TASK_Destory_Current();
	}
}

/*-------------------------------------------------------------------------------------------
 * void USB_Hub_Class_Enumerate_Fork
 * Purpose : for saving memroy, using this function 
 * Params  :  
 * Returns : none
 * Note    : none
 *-------------------------------------------------------------------------------------------*/ 
void USB_Hub_Class_Enumerate_Fork(U8_T devinx)
{
	TASK_USBHC_HUB_Class_Enumerate_Task.Task_Para = devinx;
//	TASK_USBHC_HUB_Class_Enumerate_Task.Task_Interval.w = intt_id;
	TASK_Run(&TASK_USBHC_HUB_Class_Enumerate_Task);
}

/* End of usbhc_hub.c */
