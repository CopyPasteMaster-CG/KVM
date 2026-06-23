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
 * Module Name: usbdc_vhub.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */

/* INCLUDE FILE SECTION 							*/
#include <string.h>
#include "project_include.h"

/* NAMING CONSTANT DECLARATIONS 					*/

code  VHub_Port_Changed_Bit[] = 
{
	L_PortChange_Connection,
	L_PortChange_Enable,
	L_PortChange_Suspend,
	L_PortChange_OverCurrent,
	L_PortChange_Reset,
};

/* GLOBAL VARIABLES DECLARATIONS 					*/
U8_T  VHUB_MSC_Reset_Flag=0;
U8_T  VHUB_MSC_Reset_Port=0;
U8_T  VHUB_MSC_Reset_HubDevinx=0;
U8_T  VHUB_MSC_Reset_HubPort=0;
U8_T  HUB_Status[4]={0,0,0,0};	// for Interrupt Transfer used

/* Root Hub Device Mapping Table */
USBDC_Vritaul_Hub_Map_TypeDef USBDC_Virtual_Hub_Map[USBDC_DEVICE_MAX];

/* LOCAL VARIABLES DECLARATIONS 					*/
/* LOCAL SUBPROGRAM DECLARATIONS 					*/
RESULT USBDC_VHub_Port_ClearFeature(U8_T devinx,U8_T port,U8_T feature,U8_T hubport);  //Operation,PowerNumber
RESULT USBDC_VHub_Port_SetFeature(U8_T devinx,U8_T port,U8_T feature,U8_T hubport);  //Operation,PowerNumber
//void USBDC_VHub_Update_StatusChange(U8_T devinx,U8_T startport,U8_T end_port);
void USBDC_VHub_Hub_Status_Change_Report_Ack(U8_T devinx,U8_T port);
void USBDC_VHub_Hub_Status_Change_Report_Nak(U8_T devinx,U8_T port);
void USBDC_VHub_Reset_Attribute(U8_T port,U8_T devinx);
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
/* EXTERNAL SUBPROGRAM DECLARATIONS 			*/

/*
 * ----------------------------------------------------------------------------
 * Function Name: USBDC_VHub_Init
 * Purpose: Initial the usb dc system  
 * Params:  NONE
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------
 */
void USBDC_VHub_Init(void)
{
	U8_T					pdevinx,buflen;
	USBHC_DevDesc_TypeDef	*devsc;

	/* Initialize Virtual Hub */
	USBDC_Device[USBDC_VHUB_DEVINX].VHub = m_malloc(sizeof(VHub_ReportTypeDef),19); // create the hub report struct
	memset(USBDC_Device[USBDC_VHUB_DEVINX].VHub,0x00,sizeof(VHub_ReportTypeDef));
	USBDC_Device[USBDC_VHUB_DEVINX].EndpNum = 2;  //Contain 2 endpoint buffers
	USBDC_HAL_Malloc_Device_Endpx(USBDC_VHUB_DEVINX,0,0,USBDC_TABLE_DOUBF_MASK,USBDC_VHUB_MAX_ENDP0_SIZE);
	USBDC_HAL_Malloc_Device_Endpx(USBDC_VHUB_DEVINX,1,1,USBDC_TABLE_DIR_MASK  ,USBDC_VHUB_MAX_ENDP1_SIZE);
	
	/*Alloc Free Physical Device Table index */
	USB_HAL_Alloc_Free_PDevice(&pdevinx, 0xFF); //Get a free device table in USB Devcie Table
	
	//Create the Hub Relative control buffer
	buflen = sizeof(USBHC_HubReportTypeDef) * USBDC_VHUB_PORT_NUM;
	USB_PDevice[pdevinx].HUB.ReportState = m_malloc(buflen,20);
	USB_PDevice[pdevinx].HUB.ReportEndpInx = USBDC_VHUB_ENDPINX;
	
	//Init PVID/VID of virtual HUB device descriptor
	devsc = (USBHC_DevDesc_TypeDef *)&HUB_DeviceDescriptor;
#if (SYSTEM_STORAGE_FLASH)
	//From flash
	devsc->idVendor = USBDC_VIRTUAL_HwConfig.vendorID;
	devsc->idProduct = USBDC_VIRTUAL_HwConfig.vHUBProductID;
#endif
    printf("\n\r");
	printf("[Virtual HUB] VID = %04x, PID = %04x\n\r",
			Big_Endian_16_Convert(devsc->idVendor),
			Big_Endian_16_Convert(devsc->idProduct));

	USBDC_VIRTUAL_Create_DeviceDesc(pdevinx,HUB_DeviceDescriptor);
	
	//Hub Descriptor .......
	USB_PDevice[pdevinx].Desc[HUB_DESC].Len = HUB_SIZ_DESCRIPTOR;
	USB_PDevice[pdevinx].Desc[HUB_DESC].Ptr = HUB_Descriptor;
	
	//Langid
	USB_PDevice[pdevinx].Desc[LANG_ID].Len = HUB_SIZ_STRING_LANGID;
	USB_PDevice[pdevinx].Desc[LANG_ID].Ptr = HUB_StringLangID;
	//Vendier ID String
	USB_PDevice[pdevinx].Desc[VENDOR_ID].Len = HUB_SIZ_STRING_VENDOR;
	USB_PDevice[pdevinx].Desc[VENDOR_ID].Ptr = HUB_StringVendor;	  
	//Product String
	USB_PDevice[pdevinx].Desc[PRODUCT_ID].Len = HUB_StringProduct[0];
	USB_PDevice[pdevinx].Desc[PRODUCT_ID].Ptr = HUB_StringProduct;	  
	//Serial String
	USB_PDevice[pdevinx].Desc[SERIAL_ID].Len = HUB_StringSerial[0];
	USB_PDevice[pdevinx].Desc[SERIAL_ID].Ptr = HUB_StringSerial;
	
	USB_PDevice[pdevinx].Hub_NbrPorts        = USBDC_VHUB_PORT_NUM; //assign port number
	USBDC_VIRTUAL_Create_ConfigDesc(pdevinx,HUB_ConfigDescriptor);
	
	/*Generate the USB Devcie Attribute */
	USB_PDevice[pdevinx].Hid_SetIdle_Allow  = 0x00; // not allow set idle command
	USB_PDevice[pdevinx].ConfigurationValue = 0x01; // Set current configuration Value

	memset(USBDC_Virtual_Hub_Map,0x00,sizeof(USBDC_Virtual_Hub_Map));
}

/*******************************************************************************
* Function Name  : USBDC_VHub_Setup_Process.
* Description    : process the hub setup token
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
RESULT USBDC_VHub_Setup_Process(U8_T devinx, U8_T port) 
{
	RESULT Result=USB_UNSUPPORT;
#if (SYSTEM_MSC_DEVICE_SUPPORT)		
	U8_T   pdevinx;
#endif	
	
	if (Type_Recipient == (CLASS_REQUEST | DEVICE_RECIPIENT)) //Devcie operation
	{
		if (Request_No == GET_DESCRIPTOR) // Get Hub Descriptor
		{
			Result = USBDC_Standard_GetDescData_Post(devinx,port,HUB_DESC);
		}
		else if (Request_No == GET_STATUS) // Get Hub Status
		{
			USBDC_Device[devinx].Ctrl_TotalByte[port]   = USBDC_Device[devinx].Setup[port].b.wLength.w;
			USBDC_Device[devinx].Ctrl_CurrentByte[port] = 0;
			USBDC_Device[devinx].Control_EndpBuf[port]  = HUB_Status;
			Result=USB_SUCCESS;
		}
	}
	else if (Type_Recipient == (CLASS_REQUEST | OTHER_RECIPIENT))
	{
		if (Request_No == GET_STATUS) // Get Hub Port Status
		{
			USBDC_Device[devinx].Ctrl_TotalByte[port]   = USBDC_Device[devinx].Setup[port].b.wLength.w;
			USBDC_Device[devinx].Ctrl_CurrentByte[port] = 0;			
			USBDC_Device[devinx].Control_EndpBuf[port] = (U8_T *)&USBDC_Device[devinx].VHub->PortStatus[port][wIndex_L-1];
#if (SYSTEM_MSC_DEVICE_SUPPORT)	
			pdevinx = USBDC_Device[devinx].VHub->PortStatus[port][wIndex_L-1].Devinx;
			if ((pdevinx == USBHC_MSC_Devinx) && (VHUB_MSC_Reset_Flag))
			{
				Result=USB_NOT_READY;
			}		
			else
#endif	/* #if (SYSTEM_MSC_DEVICE_SUPPORT)	*/			
			{
				Result=USB_SUCCESS;
			}	
		}
		else if (Request_No == CLEAR_FEATURE)  //CLEAR FEATURE for EndPoint
		{
			Result = USBDC_VHub_Port_ClearFeature(devinx,port,wValue_L,wIndex_L); //Operation,PowerNumber
		}
		/* SET FEATURE for EndPoint*/
		else if (Request_No == SET_FEATURE)
		{
			Result = USBDC_VHub_Port_SetFeature(devinx,port,wValue_L,wIndex_L);  //Operation,PowerNumber  
		}
	}
	
	return Result;
}

/*******************************************************************************
* Function Name  : USBDC_VHub_Port_ClearFeature.
* Description    :  
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
RESULT USBDC_VHub_Port_ClearFeature(U8_T devinx,U8_T port,U8_T feature,U8_T hubport)  //Operation,PowerNumber
{
	U8_T vdevinx;

	hubport--;
	if (feature == PORT_ENABLE) // port to clear the enable, so disalbe the port now
	{
		vdevinx = USBDC_Device[devinx].VHub->PortStatus[port][hubport].Devinx;

		if (vdevinx)
		{
			USBDC_HAL_Address_Active(port, vdevinx, RESET);
		}
		USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortStatusL &= ~L_PortStatus_Enable;
	}

	if (feature >= 16 && feature <= 20)
	{
		feature -= 16;
		USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortChangeL &= ~VHub_Port_Changed_Bit[feature];
	}
	USBDC_VHub_Update_StatusChange(devinx,port,port,0);
	return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : USBDC_VHub_Port_SetFeature.
* Description    :  
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
RESULT USBDC_VHub_Port_SetFeature(U8_T devinx,U8_T port,U8_T feature,U8_T hubport)  //Operation,PowerNumber
{
	RESULT	Result = USB_SUCCESS;
	U8_T	pdevinx;
	
	hubport--;
	
	switch (feature)
	{
		case PORT_SUSPEND:	// how to figure out the deviceindex?
			//this should suspend all the virtual device which is connect on this HUB PORT
			pdevinx = USBDC_Device[devinx].VHub->PortStatus[port][hubport].Devinx;
			//printf("> DCPort(%bu),VHub_Port(%bu),Device=%bu : Suspend\n\r",port,hubport,pdevinx);
			USBDC_Device[pdevinx].Feature[port] |= FEATURE_SUSPEND; //this device go suspend

            //USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortStatusL &= ~L_PortStatus_Enable;
			USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortStatusL |= L_PortStatus_Suspend;
#ifdef RESUME_DELAY
			//Special Handle for Reaume...
			VDevice_Keep_Quite_Flag[port]  |= BIT_MASK[pdevinx];
			USBDC_Resume_Timer_ActiveID[pdevinx] = 
					TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_VDevice_Keep_Quite_TASKID,port,pdevinx,VDEVICE_RESUME_KEEP_QUITE,VDEVICE_RESUME_KEEP_QUITE); //after 1000ms check overcurrent again
#endif		
			break;
		case PORT_POWER:
			if ((USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortStatusH & H_PortStatus_Power) == 0) // if current is no power
			{
				// Power ON => Connect Changed
				USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortStatusH |= H_PortStatus_Power;
				USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortStatusL &= ~L_PortStatus_Suspend;
				if (USBDC_Device[devinx].VHub->PortStatus[port][hubport].Devinx)
				{
					if ((USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortStatusL & L_PortStatus_Connection) == 0x00)
					{
						USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortStatusL |= L_PortStatus_Connection;
					}
				}
				
				if (USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortStatusL & L_PortStatus_Connection) // if current is connect
				{
					USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortStatusH &= ~(H_PortStatus_HighSpeed|H_PortStatus_LowSpeed);  //Force Full Speed device /*Importtant*/
					USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortChangeL |= L_PortChange_Connection; //report connection change
				}
			}
			break;
		case PORT_RESET:
			//Should Report Enable & Connection & ResetComplete_Changed
			USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortChangeL |= L_PortChange_Reset; //report connection change
			if (USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortStatusH & H_PortStatus_Power) // if current is power on
			{
				if (USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortStatusL & L_PortStatus_Connection) // if current is connection
				{
					USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortStatusL |= L_PortStatus_Enable;  //Force Full Speed device /*Importtant*/					
					USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortStatusL &= ~L_PortStatus_Suspend;
					//---------------------------------------
					/* Check hub port connection condition */
					pdevinx = USBDC_Device[devinx].VHub->PortStatus[port][hubport].Devinx;
					if (pdevinx) // if device present
					{						
						USBDC_REGS_Address_Active(port, pdevinx, RESET);
						USBDC_REGS_Address_Write(port,pdevinx,0x00);
						USBDC_VIRTUAL_Active_Device(pdevinx, MOUNT_PORT[port]);
#if (PROJECT_USB_GENERIC_HID_ENABLE)
						if (pdevinx == USBDC_VHID_DEVINX)
						{
							USBDC_VHid_Reset();
						}
						else							
#endif /* #if (PROJECT_USB_GENERIC_HID_ENABLE) */
						{
#if (SYSTEM_MSC_DEVICE_SUPPORT)						
						if ((USB_PDevice[pdevinx].DevClass == USB_MSC_CLASS) && (pdevinx == USBHC_MSC_Devinx))
						{												
							USBHC_MSC_Bulk_State_Reset(USBHC_MSC_Devinx);
#if (SYSTEM_EXTENDER_RECEIVER)							
							Extender_MSC_Init(1);
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */								
							VHUB_MSC_Reset_Flag = 1;
						}													
#endif

#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
						else if (pdevinx == USBDC_VMSC_DEVINX)
						{
							USBDC_MSC_State_Reset();
						}						
#endif
						}	
					}
					
					/*Inital The Protocol value */
					USBDC_VHub_Reset_Attribute(port,pdevinx);
				}
			}
			break;
	}
	
	//Handle the VHUB MSC Reset Flag
#if (SYSTEM_MSC_DEVICE_SUPPORT)
	if (VHUB_MSC_Reset_Flag)
	{		
		VHUB_MSC_Reset_Port = port;
		VHUB_MSC_Reset_HubDevinx = devinx; //keep hub devinx
		VHUB_MSC_Reset_HubPort = hubport;		
		if (USB_PDevice[pdevinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)
		{			
#if (SYSTEM_EXTENDER_TRANSMITTER)							
			ExtenderT_Reset_Remote_MSC_Req();			
#endif	
		}
		else
		{			
			USBHC_MSC_Reset_DCPort= port;
			USBHC_MSC_ResetDevinx = USBHC_MSC_Devinx;			
			USBDC_Maintain_MSC_PDevice(KVM_CurrentMSC);		
			USBHC_MSC_Reset_Flag  = 1;
		}				
		USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortChangeL &= ~L_PortChange_Reset; //report connection change				
	}
	else	
#endif									
	{			
		USBDC_VHub_Update_StatusChange(devinx,port,port,0);
	}	
	
	return Result;
}
 
/*******************************************************************************
* Function Name  : USBDC_VHub_Update_StatusChange.
* Description    : Move the Hub Status Change Report data into endp buffer 
* Input          :  
* Output         : None.
* Return         : None
*******************************************************************************/
void USBDC_VHub_Update_StatusChange(U8_T devinx,U8_T startport,U8_T end_port,U8_T resume)
{
	U8_T	port;
	U8_T	hubport;
	bit		status_changed;
	
	for (port = startport; port <= end_port ; port++)
	{
		//1. status change
		if (resume)
		{	
			USBDC_WakeupUpstreamHost(port);
		}
		
		USBDC_Device[devinx].VHub->HubChanged[port] = 0;
		status_changed = 0;
		for (hubport = 0; hubport < USBDC_VHUB_PORT_NUM; hubport++)
		{
			if (USBDC_Device[devinx].VHub->PortStatus[port][hubport].wPortChangeL)
			{
				status_changed = 1;
				USBDC_Device[devinx].VHub->HubChanged[port] |= (0x01 << (hubport+1));
			}
		}
		
		if (status_changed) 
		{
			USBDC_VHub_Hub_Status_Change_Report_Ack(devinx,port);
		}
		else
		{
			USBDC_VHub_Hub_Status_Change_Report_Nak(devinx,port);
		}
	}
}

/*******************************************************************************
* Function Name  : USBDC_VHub_Hub_Status_Change_Report.
* Description    : Move the Hub Status Change Report data into endp buffer 
* Input          :  
* Output         : None.
* Return         : None
*******************************************************************************/
void USBDC_VHub_Hub_Status_Change_Report_Ack(U8_T devinx,U8_T port)
{
	U8_T endpinx;

	endpinx = USB_PDevice[devinx].HUB.ReportEndpInx;
	USBDC_HAL_Endp_In_Buf_Move(&USBDC_Device[devinx].VHub->HubChanged[port],1,port,devinx,endpinx);
	USBDC_REGS_Endp_ControlSet(port,devinx,endpinx,DA_CR_BVLD_SET); //Asnwer ack for next IN 
}

/*******************************************************************************
* Function Name  : USBDC_VHub_Hub_Status_Change_Report.
* Description    : Move the Hub Status Change Report data into endp buffer 
* Input          :  
* Output         : None.
* Return         : None
*******************************************************************************/
void USBDC_VHub_Hub_Status_Change_Report_Nak(U8_T devinx,U8_T port)
{
	U8_T endpinx;

	endpinx = USB_PDevice[devinx].HUB.ReportEndpInx;
	USBDC_HAL_Endp_In_Buf_Move(&USBDC_Device[devinx].VHub->HubChanged[port],1,port,devinx,endpinx);
	USBDC_REGS_Endp_ControlSet(port,devinx,endpinx,DA_CR_ENPX_IN_NAK_SET); //Asnwer NAK for next IN
}

/*******************************************************************************
* Function Name  : USBDC_VHub_UpPort_Reset.
* Description    : Move the Hub Status Change Report data into endp buffer 
* Input          :  
* Output         : None.
* Return         : None
*******************************************************************************/
void USBDC_VHub_UpPort_Reset(U8_T devinx,U8_T pid)
{
	U8_T index;
	U8_T hubport = USBDC_VHUB_PORT_NUM;
	U8_T hubdev; // start for DC root hub
	U8_T DownStreamMap[7];
	U8_T map = 0;
	U8_T current_map = 0; // max 5 hub device
	
	//Clear Current Upstream Port
RESCAN_DOWNSTREAM_PORT:
	for (index = 0; index < hubport; index++)
	{
		memset(&USBDC_Device[devinx].VHub->PortStatus[pid][index],0x00,(sizeof(USBDC_VHubReportTypeDef)-1));
		hubdev = USBDC_Device[devinx].VHub->PortStatus[pid][index].Devinx;
		if (hubdev)  // if the donwstream port is connect with HUB
		{
			if (USB_PDevice[hubdev].DevClass == USB_HUB_CLASS) // if hub class,then clear status
			{
				DownStreamMap[map] = hubdev;
				map++;
			}
		}
	}
	USBDC_Device[devinx].VHub->HubChanged[pid] = 0x00;
	
	if (current_map != map)
	{
		devinx = DownStreamMap[current_map];
		current_map++;
		
		hubport = USB_PDevice[devinx].Hub_NbrPorts & USBHC_HUBPORT_MASK;
		goto RESCAN_DOWNSTREAM_PORT;
	}
}


/*******************************************************************************
* void USBDC_VHub_Reset_Attribute(U8_T port,U8_T devinx)
* Description    : This function will check all the device protocol setting
*                  value,if different with initial state,then reset to 0
* Input          :  
* Output         : None.
* Return         : None
*******************************************************************************/
void USBDC_VHub_Reset_Attribute(U8_T port,U8_T devinx)
{
	U8_T inf_inx,inf_bit;
	
	/* Reset the Protcol,Configuration & Alternateing setting */
	USBDC_VIRTUAL_Port_Clear(port,devinx,devinx+1);
	if (port == KVM_CurrentHost)  // only port is in active then check the protocol setting
	{
		for (inf_inx=0 ; inf_inx < USB_PDevice[devinx].InterfaceNum ;inf_inx++)
		{
			inf_bit = 0x01 << inf_inx;
			if (USB_PDevice[devinx].InfClass[inf_inx] == USB_HID_CLASS) // if it is hid class
			{				
				if ((USB_PDevice[devinx].Current_Protocol & inf_bit) != 0) //if in boot protocol, then reset it
				{									
					USBHC_Set_Dev_Inf_Protocol(devinx,inf_inx,0,1);
				}
			}
		}
	}
}

/*******************************************************************************
* void USBDC_VHub_Resume_Port
* Description    : This function will resume the device & make the upperhub device
*                  send out suspend changed status to host
* Input          :  
* Output         : None.
* Return         : None
*******************************************************************************/
void USBDC_VHub_Resume_Port(U8_T port,U8_T target_devinx)
{
	U8_T upperdevinx;
	U8_T upperhubport;
	
	if (USB_PDevice[target_devinx].Addr & USBHC_DEVICE_USED_MASK)
	{	
		if (USBDC_Device[target_devinx].Feature[port] & FEATURE_SUSPEND)//this device go suspend															 
		{
			//check upper layer devinx & class
			//upperdevinx = USB_PDevice[target_devinx].UpperHubDevinx;
			//upperhubport = USBDC_Virtual_Hub_Map[target_devinx].Devinx_HubPort;
			USBHC_Get_Upper_Device_Info(target_devinx,&upperdevinx,&upperhubport);
			USBDC_Device[target_devinx].Feature[port] &= ~FEATURE_SUSPEND;//this device go suspend															 			     
			USBDC_Device[upperdevinx].VHub->PortStatus[port][upperhubport].wPortChangeL |= L_PortChange_Suspend;			
			USBDC_VHub_Update_StatusChange(upperdevinx,port,port,1);
		}
	}		     
}

/*******************************************************************************
* void USBDC_VHub_Resume_HubPort_Device(U8_T dcport)
* Description    : This function will clear the suspend status of all 
*                  downstream port device
* Input          :  
* Output         : None.
* Return         : None
*******************************************************************************/
void USBDC_VHub_Resume_HubPort_Device(U8_T dcport)
{
	U8_T port;
	
	for (port=0; port <= USBDC_VHUB_PORT_NUM ; port++)
	{
		USBDC_Device[USBDC_VHUB_DEVINX].VHub->PortStatus[dcport][port].wPortStatusL &= ~L_PortStatus_Suspend;
	}	
}	
/* End of usbdc_vhub.c */
