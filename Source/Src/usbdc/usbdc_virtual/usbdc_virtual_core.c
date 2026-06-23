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
 * Module Name: usbdc_virtual_core.c
 * Purpose: The USB DC Virtual Core prgoram, Handle all the activity of Virtual
 *          USB DC,is the interface between Host & Virtual Root HUB,Virtual HID
 *          the other virtual device.
 * Author: Jack Wang
 * Date:
 * Notes:
 *=============================================================================
 */

/* INCLUDE FILE SECTION 							*/
#include <string.h>
#include <stdlib.h>
#include "project_include.h"

/* NAMING CONSTANT DECLARATIONS 					*/
#define	HW_CONFIG_FLASH_OFFSET_MAX	(sizeof(HWConfigData) / FLASH_WORD_LEN)

/* GLOBAL VARIABLES DECLARATIONS 					*/
u16_t_u8_t		StatusInfo;
HWConfigData	USBDC_VIRTUAL_HwConfig;
U8_T			USBDC_VIRTUAL_SerialByPort;
#if (SYSTEM_EXTENDER_SUPPORT)
U8_T XDATA		USBDC_PassthroughTempory[USB_HC_MAX_DEVICE][USBDC_PORT_MAX][8];
#else
U8_T XDATA		USBDC_PassthroughTempory[64];
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Setup Up Token Parser Usage
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
U8_T			Request_No;
U8_T			Type_Recipient;
U8_T			wValue_H;
U8_T			wValue_L;
U8_T			wIndex_H;
U8_T			wIndex_L;
U8_T			Port_Protocol[USBDC_PORT_MAX];
U8_T			TASK_USBDC_VIRTUAL_PassThrough_Setup_Wait_ID;

/* LOCAL VARIABLES DECLARATIONS 					*/
/* LOCAL SUBPROGRAM DECLARATIONS 					*/
void USBDC_VIRTUAL_Setup_DataStageOut(U8_T devinx,U8_T port,U8_T handle_type);
void USBDC_VIRTUAL_Fill_Device_Reg(U8_T devinx,U8_T active_port,U8_T address);
void USBDC_VIRTUAL_Control_Endp_Stalled(U8_T devinx,U8_T port);
void USBDC_VIRTUAL_Control_Statue_In(U8_T devinx,U8_T port);

RESULT USBDC_Standard_SetConfiguration(U8_T devinx,U8_T port);
RESULT USBDC_Standard_Feature_Control(U8_T devinx,U8_T port,FlagStatus state);
RESULT USBDC_Standard_SetInterface(U8_T devinx,U8_T port);
RESULT USBDC_Class_SetIdle(U8_T devinx);
RESULT USBDC_Standard_GetConfiguration(U8_T devinx,U8_T port);
RESULT USBDC_Standard_GetStatus(U8_T devinx,U8_T port);
RESULT USBDC_Standard_GetInterface(U8_T devinx,U8_T port);
void USBDC_VIRTUAL_Endp_Stall_Control(U8_T port,U8_T devinx,U8_T endp_addr,FlagStatus);
RESULT USBDC_Standard_GetDeviceDescriptor(U8_T devinx,U8_T port);
RESULT USBDC_Class_GetDescriptor(U8_T devinx,U8_T port);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Request Function Declartion
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
RESULT USBDC_Class_GetProtocol(U8_T devinx,U8_T port);
RESULT USBDC_Class_SetProtocol(U8_T devinx,U8_T port);
RESULT USBDC_Class_GetReport(U8_T devinx,U8_T port);
RESULT USBDC_Class_SetReport(U8_T devinx,U8_T port);
void TASK_USBDC_VIRTUAL_PassThrough_Setup_Wait(void);
U8_T USBDC_Standard_GetDeviceDescriptor_Inx(U8_T devinx);
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
/* EXTERNAL SUBPROGRAM DECLARATIONS 			*/ 

/*
 * ----------------------------------------------------------------------------
 * Function Name: USBDC_VIRTUAL_Init
 * Purpose: Initial the usb dc system  
 * Params:  NONE
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------
 */
void USBDC_VIRTUAL_Init(void)
{
#if (SYSTEM_STORAGE_FLASH)
	U16_T offset;
	
	/*01.Read related config of virtual device from flash */
	for (offset = 0; offset < HW_CONFIG_FLASH_OFFSET_MAX; offset++)
	{
		FLASH_InfoWordRead8Byte(0x00000000 + (offset * FLASH_WORD_LEN), ((U8_T *)&USBDC_VIRTUAL_HwConfig) + (offset * FLASH_WORD_LEN));
	}
	
	// Get serial number
	if (USBDC_VIRTUAL_HwConfig.serialNum[0] <= HUB_SIZ_STRING_SERIAL)
	{
#if (PROJECT_USB_GENERIC_HID_ENABLE)			
		memcpy(VHID_StringSerial, USBDC_VIRTUAL_HwConfig.serialNum, USBDC_VIRTUAL_HwConfig.serialNum[0]);
#endif /* #if (PROJECT_USB_GENERIC_HID_ENABLE) */

		memcpy(HUB_StringSerial, USBDC_VIRTUAL_HwConfig.serialNum, USBDC_VIRTUAL_HwConfig.serialNum[0]);
		offset = USBDC_VIRTUAL_HwConfig.serialNum[0] - 2;
		USBDC_VIRTUAL_SerialByPort = USBDC_VIRTUAL_HwConfig.serialNum[offset];
	}
	else
	{
		USBDC_VIRTUAL_SerialByPort = HUB_StringSerial[8];
	}
	
	// Get product string
#if (PROJECT_USB_GENERIC_HID_ENABLE)
	if (USBDC_VIRTUAL_HwConfig.vHIDProductString[0] <= VHID_SIZ_STRING_PRODUCT)
	{
		memcpy(VHID_StringProduct, USBDC_VIRTUAL_HwConfig.vHIDProductString, USBDC_VIRTUAL_HwConfig.vHIDProductString[0]);
	}
#endif /* #if (PROJECT_USB_GENERIC_HID_ENABLE) */

	if (USBDC_VIRTUAL_HwConfig.vHUBProductString[0] <= HUB_SIZ_STRING_PRODUCT)
	{
		memcpy(HUB_StringProduct, USBDC_VIRTUAL_HwConfig.vHUBProductString, USBDC_VIRTUAL_HwConfig.vHUBProductString[0]);
	}
#endif
	
	/*02.Reserved a device in the Device Table for virtual Hub */
	memset(&USBDC_Device[USBDC_VHUB_DEVINX],0x00,sizeof(USBDC_DeviceTypeDef));
	USBDC_Device[USBDC_VHUB_DEVINX].DevIdx |= DC_USED_MASK;

	/*03.Init the Hub & HID device*/
	USBDC_VHub_Init(); //The virtual Hub will used index0,Endp 0,Endp 1

#if (PROJECT_USB_GENERIC_HID_ENABLE)
	USBDC_VHid_Init(); //The virtual HID will used index1,Endp 0,Endp 1,Endp 2
#endif 	/* #if (PROJECT_USB_GENERIC_HID_ENABLE)	*/

#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
	USBDC_VMsc_Init(); //The virtual HID will used index1,Endp 0,Endp 1,Endp 2
#endif

	/*04.Assing device into USBDC Register */ 
	USBDC_VIRTUAL_Fill_Device_Reg(USBDC_VHUB_DEVINX,UDC_PORT_ALL_ACT,0x00);	//Port0,Port1,Port2,Port3 Active the root hub device

#if (PROJECT_USB_GENERIC_HID_ENABLE)
	USBDC_VIRTUAL_Fill_Device_Reg(USBDC_VHID_DEVINX,UDC_PORT_ALL_ACT,0x00);	//Port0,Port1,Port2,Port3 Active the root hub device
#endif /* #if (PROJECT_USB_GENERIC_HID_ENABLE) */

	USBDC_VIRTUAL_Active_Device(USBDC_VHUB_DEVINX,UDC_PORT_ALL_ACT);

	TASK_USBDC_VIRTUAL_PassThrough_Setup_Wait_ID = TASK_Create(TASK_USBDC_VIRTUAL_PassThrough_Setup_Wait);

#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("TASK_USBDC_VIRTUAL_PassThrough_Setup_Wait_ID=%bu\n\r",TASK_USBDC_VIRTUAL_PassThrough_Setup_Wait_ID);
#endif	
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: USBDC_VIRTUAL_Fill_Device_Reg
 * Purpose: Use Device Table, fill up DC Register
 * Params:  NONE
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------
 */
void USBDC_VIRTUAL_Fill_Device_Reg(U8_T devinx,U8_T active_port,U8_T address)
{
	U8_T pinx;
	U8_T ep;
	U8_T enable_bit;
	U8_T reg0;
	U8_T ep_dir;
	U8_T ep_dbuf=0;
	U8_T ep_iso;
	U8_T ep_pksize;
	U8_T prefetch[USBDC_PORT_MAX];
	U8_T index;

	for (pinx = 0; pinx < USBDC_PORT_MAX; pinx++)
	{
		prefetch[pinx] = 0;
		
		if (active_port & MOUNT_PORT[pinx])
		{
			for (ep=0; ep < USBDC_Device[devinx].EndpNum; ep++)
			{
				if ((ep == 0) || (USBDC_Device[devinx].EndpAddr[ep] & 0x0f))
				{
					//Check Double Buffer Flag
					if (USBDC_Device[devinx].EndpAddr[ep] & USBDC_TABLE_DOUBF_MASK)
					{
						if (ep != 0) //Control Endp will ingore the double buffer setting
						{
							ep_dbuf = DA_EP_CFGR_DBUF_SET;
						}
					}
					else
					{
						ep_dbuf = DA_EP_CFGR_DBUF_RESET;
					}
					
					//Check DIR
					if (USBDC_Device[devinx].EndpAddr[ep] & USBDC_TABLE_DIR_MASK) //OUT-0,IM-1
					{
						ep_dir = DA_EP_CFGR_DIR_IN;
					}
					else
					{
						ep_dir = DA_EP_CFGR_DIR_OUT;
						index=ep+1;
						if (index < USBDC_Device[devinx].EndpNum)
						{
							if ((USBDC_Device[devinx].EndpAddr[index] & 0x0f) == (USBDC_Device[devinx].EndpAddr[ep] & 0x0f))
							{
								continue;
							}	
						}		
					}
					
					//Check ISO
					if (USBDC_Device[devinx].EndpAddr[ep] & USBDC_TABLE_ISO_MASK) //OUT-0,IM-1
					{
						ep_iso = DA_EP_CFGR_ISOCH_SET;
					}
					else
					{
						ep_iso = DA_EP_CFGR_ISOCH_RESET;
					}

					//Check prefetch
					if ((ep_iso == DA_EP_CFGR_ISOCH_SET) && (ep_dir == DA_EP_CFGR_DIR_IN))
					{
						prefetch[pinx] = DC_IEPPR_ENABLE_SET | (devinx << 4) | (USBDC_Device[devinx].EndpAddr[ep] & 0x0f);
					}

					//---------------------------------------------
					//Test Only if the ISO port enable the endpoint
					enable_bit = DA_EP_CFGR_EPEN_SET;

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
					if (ep) // not include control endpoint pipe
					{
						if (USB_PDevice[devinx].DevClass == USB_AUDIO_CLASS)
						{
							if (ep_iso == DA_EP_CFGR_ISOCH_SET)
							{
								if (pinx != KVM_CurrentUSBAudio)
								{
									enable_bit = DA_EP_CFGR_EPEN_RESET;
								}
							}
						}
					}
#endif
					reg0 = (USBDC_Device[devinx].EndpAddr[ep] & 0x0f) | enable_bit | ep_dbuf | ep_dir | ep_iso;
					ep_pksize = USBDC_REGS_Get_Pkt_Size(ep_iso,USBDC_Device[devinx].EndpMaxSize[ep]);
					
#if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST))
					//Check Burst Mode
					if (USBDC_Device[devinx].EndpAddr[ep] & USBDC_TABLE_BURST_MASK) //
					{
						ep_pksize |= DA_EP_CFGR_BUREN_SET;
					}
#endif /* # if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST)) */
					
					//Write into Register
					USBDC_EPCFGR_Write(pinx,devinx,ep,reg0,ep_pksize);
					USBDC_REGS_Endp_Reset(pinx, devinx, ep);
				}
			}
			
			//-------------------------------------------------
			//2.Assign IDCFGR->Device Address
			USBDC_REGS_Address_Write(pinx,devinx,address);

			//-------------------------------------------------
			//3.Assign DCIEPPR->Prefetch
			if (prefetch[pinx])
			{
				USBDC_Write_Regs(pinx, DCIEPPR, &prefetch[pinx], 1);
			}
		}
		
		// Check Terminate condition
		active_port &= ~MOUNT_PORT[pinx];
		if (active_port == 0)
		{
			break;
		}
	}
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: USBDC_VIRTUAL_Active_Device
 * Purpose: Use Device Table, fill up DC Register
 * Params:  NONE
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------
 */
void USBDC_VIRTUAL_Active_Device(U8_T devinx,U8_T active_port)
{
	if (active_port & 0x01)
	{
		USBDC_HAL_Address_Active(0,devinx,SET);
	}
	if (active_port & 0x02)
	{
		USBDC_HAL_Address_Active(1,devinx,SET);
	}
	if (active_port & 0x04)
	{
		USBDC_HAL_Address_Active(2,devinx,SET);
	}
	if (active_port & 0x08)
	{
		USBDC_HAL_Address_Active(3,devinx,SET);
	}
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: USBDC_VIRTUAL_INNAK_Control
 * Purpose: Enable the IN NAE interrupt report feature
 * Params:  NONE
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------
 */
void USBDC_VIRTUAL_INNAK_Control(U8_T devinx,U8_T port,U8_T endpmask,U8_T value)
{
	U8_T in_nak=0;
	
	USBDC_Read_Regs(port,(DAyINAKE+devinx),&in_nak,1);
	if (value)
		in_nak |= endpmask;
	else
		in_nak &= ~endpmask;
	
	USBDC_Write_Regs(port,(DAyINAKE+devinx),&in_nak,1);
}


/*----------------------------------------------------------------------------
 * void USBDC_VIRTUAL_Setup_IN_Process(U8_T port,U8_T insr) 
 * Purpose : Setup Transfer Handle
 * Params  : none
 * Returns : none
 * Note    : none
 *----------------------------------------------------------------------------*/
void USBDC_VIRTUAL_Setup_IN_Process(U8_T port, U8_T devinx)  //in Complete
{
	U8_T td_id;

	if ((USBDC_Device[devinx].ControlState[port] == IN_DATA) || (USBDC_Device[devinx].ControlState[port] == LAST_IN_DATA))
	{
		/*PASSTHROUGH*/
		if (USBDC_Device[devinx].PassThroughState[port] == PASSTHROUGH_IDLE)
		{
			USBDC_VIRTUAL_Setup_DataStageIn(devinx,port,1);
		}
		else
		{
#if (SYSTEM_EXTENDER_TRANSMITTER)
			if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)	//if not extender, then 
			{
				USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_DC_SETUP_DATA_IN; // return  to HC setup data in mode
				TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_ExtenderT_PassThrough_Handle_ID,0x00,devinx,0,0);
			}
			else
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */
			{
				if (USBDC_Device[devinx].ControlState[port] == LAST_IN_DATA)
				{
					USBDC_Device[devinx].ControlState[port] = WAIT_STATUS_OUT;
				}

				if ((USB_PDevice[devinx].Hc.Control.State != CTRL_STATUS_OUT) &&
					(USB_PDevice[devinx].Hc.Control.State != CTRL_IDLE))
				{
					td_id = USB_PDevice[devinx].Hc.Control.TdNum;   // which host channel is used by device
					TASK_Active(TASK_TYPE_USB,TASK_USBHC_Handle_Control_ID,td_id,devinx,0,0);  // perform HC IN processing
				}
			}
		}
	}
	else if (USBDC_Device[devinx].ControlState[port] == WAIT_STATUS_IN)
	{		
		USBDC_Device[devinx].ControlState[port] = SETUP_COMPLETE;
#if (SYSTEM_EXTENDER_TRANSMITTER)
		if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)	//if not extender, then
		{
			if (USB_PDevice[devinx].Hc.PassThrough_State != PASSTHROUGH_IDLE)
			{
				ExtenderT_Device_Passthough_State[devinx] |= T_USB_PASSTHROUGH_DC_DONE_FLAG;
				ExtenderT_Passthrough_Done_Check(devinx);
			}
		}
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */
	}
}

/*******************************************************************************
* Function Name  : USBDC_VIRTUAL_Setup_OUT_Process
* Description    : Process the OUT token on all default endpoint.
* Input          : None.
* Output         : None.
* Return         : Post0_Process.
*******************************************************************************/
void USBDC_VIRTUAL_Setup_OUT_Process(U8_T port, U8_T devinx) 
{
	if ((USBDC_Device[devinx].ControlState[port] == IN_DATA) || (USBDC_Device[devinx].ControlState[port] == LAST_IN_DATA))
	{
#if (SYSTEM_EXTENDER_TRANSMITTER)
		if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)	//if extender remote usb device
		{
			if (USB_PDevice[devinx].Hc.PassThrough_State != PASSTHROUGH_IDLE)
			{
				if (USBDC_Device[devinx].ControlState[port] == IN_DATA) //If host issue data complete in middle way.
				{
					if ((ExtenderT_Device_Passthough_State[devinx] & T_USB_PASSTHROUGH_REMOTE_DONE_FLAG) == 0x00)
					{
						ExtenderT_Device_Passthough_State[devinx] |= T_USB_PASSTHROUGH_STATUS_OUT_FLAG;
						ExtenderT_Device_Passthrough_Flag |= BIT_MASK[devinx];						
					}
					else
					{
						ExtenderT_Device_Passthough_State[devinx] |= T_USB_PASSTHROUGH_DC_DONE_FLAG; //data complete						
						ExtenderT_Passthrough_Done_Check(devinx);
					}
				}
				else
				{
					ExtenderT_Device_Passthough_State[devinx] |= T_USB_PASSTHROUGH_DC_DONE_FLAG; //data complete					
					USB_PDevice[devinx].Hc.Control.State = CTRL_STATUS_OUT;
					ExtenderT_Passthrough_Done_Check(devinx);
				}
			}
		}
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */
		USBDC_REGS_Endp_ControlSet(port,devinx,0,DA_CR_ENPX_IN_NAK_SET); //Asnwer NAK for next IN 		
		USBDC_Device[devinx].ControlState[port] = SETUP_COMPLETE; 
	}
	else if ((USBDC_Device[devinx].ControlState[port] == OUT_DATA) || (USBDC_Device[devinx].ControlState[port] == LAST_OUT_DATA))
	{
		USBDC_VIRTUAL_Setup_DataStageOut(devinx,port,1);
	}
	else if (USBDC_Device[devinx].ControlState[port] == WAIT_STATUS_OUT)
	{
		USBDC_REGS_Endp_ControlClear(port,devinx,0,DA_CR_BCLR_SET); // now can receive data		
		USBDC_Device[devinx].ControlState[port] = SETUP_COMPLETE;

#if (SYSTEM_EXTENDER_TRANSMITTER)
		if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)	//if extender remote usb device
		{
			if (USB_PDevice[devinx].Hc.PassThrough_State != PASSTHROUGH_IDLE)
			{
				ExtenderT_Device_Passthough_State[devinx] |= T_USB_PASSTHROUGH_DC_DONE_FLAG;
				ExtenderT_Passthrough_Done_Check(devinx);
			}
		}
#endif	/* #if (SYSTEM_EXTENDER_TRANSMITTER) */
	}
	/* Unexpect state, STALL the endpoint */
	else if (USBDC_Device[devinx].ControlState[port] != SETUP_COMPLETE)
	{
		USBDC_REGS_Endp_ControlClear(port,devinx,0,DA_CR_BCLR_SET); // now can receive data
	}
}

 
/*******************************************************************************
* Function Name  : DataStageIn.
* Description    : Data stage of a Control Read Transfer.
* Input          : handletype=0->prepare,1->send out complete
* Output         : None.
* Return         : None.
*******************************************************************************/
void USBDC_VIRTUAL_Setup_DataStageIn(U8_T devinx,U8_T port,U8_T handletype)
{
	U8_T  *DataBuffer;
	U16_T Length;

	/* No more data to send and empty packet */
	if (USBDC_Device[devinx].PassThroughState[port] != PASSTHROUGH_IDLE &&
		USB_PDevice[devinx].Hc.PassThrough_State == PASSTHROUGH_DC_SETUP_DATA_IN)
	{
		Length = USB_PDevice[devinx].Hc.PControl.Data_Length;
#if (SYSTEM_EXTENDER_TRANSMITTER)
		if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)	//if is extender remode usb device
		{
			//Length = USB_PDevice[devinx].Hc.PControl.Data_Length;
			DataBuffer = USBDC_Device[devinx].Control_EndpBuf[port]+USB_PDevice[devinx].Hc.PControl.Current_Length;
		}
		else
#endif	/* #if (SYSTEM_EXTENDER_TRANSMITTER) */
		{            			
			DataBuffer = USBDC_Device[devinx].Control_EndpBuf[port];
		}
	}
	else
	{
		Length = USBDC_Device[devinx].Ctrl_TotalByte[port] - USBDC_Device[devinx].Ctrl_CurrentByte[port];
		DataBuffer = USBDC_Device[devinx].Control_EndpBuf[port]+USBDC_Device[devinx].Ctrl_CurrentByte[port];
	}

	if (Length == 0)
	{
		/* No more data to send and empty packet */
		if (handletype == 1) // Send Complete
		{
			if (USBDC_Device[devinx].ControlState[port] == LAST_IN_DATA)
			{
				USBDC_Device[devinx].ControlState[port] = WAIT_STATUS_OUT;
				return;
			}
			else 
			{
				if (USBDC_Device[devinx].ControlState[port] == IN_DATA)
				{
					USBDC_Device[devinx].ControlState[port] = LAST_IN_DATA;
					USBDC_HAL_Endp_Buf_Length_Write(0,port,devinx,0,USBDC_DIR_IN);
					goto USB_VIRTUAL_Setup_DataStageIn_Post;
				}
			}
		}
	}

	if (Length > USBDC_Device[devinx].EndpMaxSize[0])
	{
		Length = USBDC_Device[devinx].EndpMaxSize[0];
	}

	USBDC_HAL_Endp_In_Buf_Move(DataBuffer,Length,port,devinx,0);
	USBDC_Device[devinx].Ctrl_CurrentByte[port] += Length;

	if (Length == USBDC_Device[devinx].EndpMaxSize[0])
	{
		USBDC_Device[devinx].ControlState[port] = IN_DATA;
	}
	else
	{
		USBDC_Device[devinx].ControlState[port] = LAST_IN_DATA;
	}
USB_VIRTUAL_Setup_DataStageIn_Post:
	USBDC_REGS_Endp_ControlSet(port,devinx,0,DA_CR_BVLD_SET);
}

/*******************************************************************************
* Function Name  : USBDC_VIRTUAL_Setup_DataStageOut_Handle.
* Description    : Data stage of a Control Write Transfer.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
RESULT USBDC_VIRTUAL_Setup_DataStageOut_Handle(U8_T devinx,U8_T port)
{
	U8_T				led_inx;
	U8_T				led_value;
	U8_T				report_type;
	U8_T				report_id;
	U8_T				report_inf;
	USB_Setup_TypeDef	*Setup;
	
	Setup = &USBDC_Device[devinx].Setup[port];
	Request_No = Setup->b.bRequest;
	Type_Recipient = Setup->b.bmRequestType & (REQUEST_TYPE | RECIPIENT);

	if ((Type_Recipient & REQUEST_TYPE) == CLASS_REQUEST) /* Class Interface Request*/
	{
		if (Type_Recipient != (CLASS_REQUEST | OTHER_RECIPIENT))
		{
			switch(Request_No)
			{
				case SET_REPORT:  //0x09
					report_type = Setup->b.wValue.bw.lsb;
					report_id   = Setup->b.wValue.bw.msb;
					report_inf  = Setup->b.wIndex.bw.msb;
					if (report_type == 0x02) // standard led command
					{
						if (report_id >= 0x02)
						{
							return USB_SUCCESS;
						}
#if (PROJECT_USB_GENERIC_HID_ENABLE)
						if (devinx == USBDC_VHID_DEVINX)
						{
							led_inx = 0;
						}
						else
#endif
						{
							led_inx = 0;
							if ((USBDC_Device[devinx].Current_Protocol[port] & (0x01 << report_inf))== 0x00) // boot protocol
							{
								if (Setup->b.wLength.bw.msb > 1)
									led_inx = 1;
							}
						}						
						led_value = *(USBDC_Device[devinx].Control_EndpBuf[port]+led_inx) & 0x07;		

						if (KVM_HostLed[port] != led_value)
						{
							KVM_HostLed[port] = led_value;
#ifdef SYNC
							//check keyboard sync setting
							if (KVM_Flash.cSystemFlag2 & (SYSTEM_ALL_SYNC_MASK | SYSTEM_KB_SYNC_MASK))	
							{								
								//memset(KVM_HostLed,led_value,sizeof(KVM_HostLed));									
								if (KM_SYNC_KbLed != led_value)
								{	
									//printf("p=%bu,d:%bu,set led=%02bx\n\r",port,devinx,led_value);
									KM_SYNC_KbLed = led_value;
									KVM_CONSOLE_Keyboard_Led_Control(led_value);									
								}																		
							}
							else
							{
								if (KVM_CurrentHost == port)
								{
									KVM_CONSOLE_Keyboard_Led_Control(led_value);
#if (SYSTEM_EXTENDER_TRANSMITTER)								 
									ExtenderT_KVM_KB_Led_Check(port);
#endif	//#if (SYSTEM_EXTENDER_TRANSMITTER)
								}
							}								
#else
							if (KVM_CurrentHost == port)
							{
								KVM_CONSOLE_Keyboard_Led_Control(led_value);
#if (SYSTEM_EXTENDER_TRANSMITTER)								 
								ExtenderT_KVM_KB_Led_Check(port);
#endif	//#if (SYSTEM_EXTENDER_TRANSMITTER)
							}
#endif /* #ifdef SYNC */							
						}

#ifdef SYNC						
						if (devinx == USBDC_VHID_DEVINX)
						{	
							KM_SYNC_Start(port);
						}	
#endif												
						
					}
					break;
			}
			
		}
	}
	return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : USBDC_VIRTUAL_Setup_DataStageOut.
* Description    : Data stage of a Control Write Transfer.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USBDC_VIRTUAL_Setup_DataStageOut(U8_T devinx,U8_T port,U8_T handle_type)
{
	U8_T	*DataBuffer;
	U16_T	Length;
	bit		hc_passthroug_flag=0;

	if (handle_type == 0) // Out Stage Init
	{
		USBDC_Device[devinx].Ctrl_TotalByte[port] = USBDC_Device[devinx].Setup[port].b.wLength.w;
		USBDC_Device[devinx].Ctrl_CurrentByte[port] = 0;
		if (USBDC_Device[devinx].Ctrl_TotalByte[port] <= USBDC_Device[devinx].EndpMaxSize[0])
		{
			USBDC_Device[devinx].ControlState[port] = LAST_OUT_DATA;
		}
		else
		{
			USBDC_Device[devinx].ControlState[port] = OUT_DATA;
		}
		/*if Not in PassThrough Mode Out Mode */
		if (USBDC_Device[devinx].PassThroughState[port] == PASSTHROUGH_IDLE)
		{
#if (SYSTEM_EXTENDER_SUPPORT)
			USBDC_Device[devinx].Control_EndpBuf[port] = USBDC_PassthroughTempory[devinx][port];
#else
			USBDC_Device[devinx].Control_EndpBuf[port] = &USBDC_PassthroughTempory[0];
#endif
		}
	}
	else
	{
		Length = USBDC_HAL_Endp_Buf_Length_Read(port,devinx,0,USBDC_DIR_OUT); // out data length
		if (USBDC_Device[devinx].PassThroughState[port])
		{
			USB_PDevice[devinx].Hc.PControl.Data_Length=Length;
		}

		if (Length ==0) 
		{
			if (USBDC_Device[devinx].PassThroughState[port])
			{
				/*PASSTHROUGH*/
				hc_passthroug_flag = 1;
				USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_HC_SETUP_DATA_OUT;
			}
		}
		else
		{
			if ((USBDC_Device[devinx].ControlState[port] != OUT_DATA) && (USBDC_Device[devinx].ControlState[port] != LAST_OUT_DATA))
			{
				USBDC_REGS_Endp_ControlClear(port,devinx,0,DA_CR_BCLR_SET); // Clear the OUT buffre flag
				USBDC_VIRTUAL_Control_Endp_Stalled(devinx,port);
				return;
			}
			
			if (USBDC_Device[devinx].PassThroughState[port])
			{
#if (SYSTEM_EXTENDER_TRANSMITTER)
				if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)	//if extender device
				{
					DataBuffer = USB_PDevice[devinx].Hc.PControl.Buf+USB_PDevice[devinx].Hc.PControl.Current_Length;
				}
				else
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */
				{
					DataBuffer = USBDC_Device[devinx].Control_EndpBuf[port];
				}
			}
			else
			{
				DataBuffer = USBDC_Device[devinx].Control_EndpBuf[port]+USBDC_Device[devinx].Ctrl_CurrentByte[port];
			}

			USBDC_HAL_Endp_Out_Buf_Copy(DataBuffer,port,devinx,0,0);
			
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			//Logitech Unifying wireless mouse handle
			if ((USB_PDevice[devinx].DevAttr & DEVATTR_UNIFYING_MASK) &&
				(USBDC_Device[devinx].DevAttr[port] & UNIFYING_RPT_CHECK_MASK))
			{
				USBDC_Unifying_SetReport_Data_Check(port,devinx,DataBuffer);
			}
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			
			if (USBDC_Device[devinx].ControlState[port] == OUT_DATA)
			{
				if (USBDC_Device[devinx].Ctrl_TotalByte[port] < (USBDC_Device[devinx].Ctrl_CurrentByte[port] + Length)) // this over flow
				{
					USBDC_REGS_Endp_ControlClear(port,devinx,0,DA_CR_BCLR_SET); // Clear the OUT buffre flag
					USBDC_VIRTUAL_Control_Endp_Stalled(devinx,port);
					/*PassThroug*/
					return;
				}
				else
				{
					USBDC_Device[devinx].Ctrl_CurrentByte[port] += Length;
					Length = USBDC_Device[devinx].Ctrl_TotalByte[port]-USBDC_Device[devinx].Ctrl_CurrentByte[port];
					/*PASSTHROUGH*/
					if (USBDC_Device[devinx].PassThroughState[port] == PASSTHROUGH_IDLE) 
					{
						USBDC_REGS_Endp_ControlClear(port,devinx,0,DA_CR_BCLR_SET); // Clear the OUT buffre flag
					}
					else
					{
						/*PASSTHROUGH*/
						hc_passthroug_flag = 1;
						//Special handle for Logitech unifying wirelsee mouse
					}
					if (Length <= USBDC_Device[devinx].EndpMaxSize[0])
					{
						USBDC_Device[devinx].ControlState[port] = LAST_OUT_DATA;
					}
				}
			}
			else if (USBDC_Device[devinx].ControlState[port] == LAST_OUT_DATA)// should put STALL
			{
				// The Last package
				USBDC_Device[devinx].ControlState[port] = WAIT_STATUS_IN;
				USBDC_REGS_Endp_ControlClear(port,devinx,0,DA_CR_BCLR_SET); // Clear the OUT buffre flag
				
				//Enable endpoint buffer in condition
				USBDC_HAL_Endp_Buf_Length_Write(0,port,devinx,0,USBDC_DIR_IN);

				if (USBDC_Device[devinx].PassThroughState[port] == PASSTHROUGH_IDLE) 
				{
					USBDC_REGS_Endp_ControlSet(port,devinx,0,DA_CR_BVLD_SET); // STATUS IN
					if (devinx != USBDC_VHUB_DEVINX )
					{
						USBDC_VIRTUAL_Setup_DataStageOut_Handle(devinx,port);
					}
				}
				else
				{
					/*PASSTHROUGH*/
					hc_passthroug_flag = 1;
				}
			}
			else
			{
				USBDC_VIRTUAL_Control_Endp_Stalled(devinx,port);
			}
			
			/*PASSTHROUGH*/
			if (hc_passthroug_flag)
			{
				USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_OUT;   
				USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_HC_SETUP_DATA_OUT;
#if (SYSTEM_EXTENDER_TRANSMITTER)
				if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)	//if extender device
				{
					TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_ExtenderT_PassThrough_Handle_ID,0,devinx,0,0);  // Generate the Setup Send command
				}
				else
#endif	/*#if (SYSTEM_EXTENDER_TRANSMITTER)	*/
				{
					TASK_Active(TASK_TYPE_USB,TASK_USBHC_Handle_Control_ID,USB_PDevice[devinx].Hc.Control.TdNum,devinx,0,0);  // Generate the Setup Send command
				}
			}
		}
	}
}

/*******************************************************************************
* Function Name  : USBDC_VIRTUAL_Control_Endp_Stalled.
* Description    : Set or enable a specific feature of Device.
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
void USBDC_VIRTUAL_Control_Endp_Stalled(U8_T devinx,U8_T port)
{	
	USBDC_Device[devinx].ControlState[port] = STALLED; // stall the quest
	USBDC_REGS_Endp_ControlSet(port,devinx,0,DA_CR_STALL_SET);
}

/*******************************************************************************
* Function Name  : USBDC_VIRTUAL_Control_Statue_In.
* Description    : Prepare a IN Ack with 0 length 
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
void USBDC_VIRTUAL_Control_Statue_In(U8_T devinx,U8_T port)
{
	USBDC_HAL_Endp_Buf_Length_Write(0,port,devinx,0,USBDC_DIR_IN);
	USBDC_REGS_Endp_ControlSet(port,devinx,0,DA_CR_BVLD_SET);
}

/*******************************************************************************
* Function Name  : USBDC_Standard_SetConfiguration.
* Description    : Set or enable a specific feature of Device.
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT USBDC_Standard_SetConfiguration(U8_T devinx,U8_T port)
{
	if ((wValue_L <= USB_PDevice[devinx].TotalConfiguration) ||
		(USB_PDevice[devinx].idVendor == 0x0408 && USB_PDevice[devinx].idProduct == 0x3001)) /* For Acer T231H touch screen */
	{
		if (wValue_L != USB_PDevice[devinx].ConfigurationValue)
		{
			//Check PD configuration value
			/*Handle HC Protocol Setting */
			if ((port == KVM_CurrentHost) && (devinx >= USBDC_VIRTUAL_DEVINX)) // active port
			{
				USB_PDevice[devinx].ConfigurationValue = wValue_L;				
				USBHC_Set_Dev_Configuration(devinx,wValue_L);					
			}
		}

		USBDC_Device[devinx].Current_Configuration[port] = wValue_L;

		return USB_SUCCESS;
	}
	return USB_UNSUPPORT;
}


/*******************************************************************************
* Function Name  : USBDC_Standard_SetAddress
* Description    : Set or enable a specific feature of Device.
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT USBDC_Standard_SetAddress(U8_T devinx,U8_T port)
{
	if (wValue_L > 127)   /* Device Address should be 127 or less*/
	{
		USBDC_VIRTUAL_Control_Endp_Stalled(devinx,port);
		return USB_UNSUPPORT;
	}
	else
	{
		USBDC_Device[devinx].DevAddr[port] = wValue_L;
		USBDC_Device[devinx].ISR_Handle[port] |= ISR_HANDLE_SET_ADDRESS; // need isr to handle the STATUS IN Status
		if (port == KVM_CurrentHost)        
		{
			USBDC_Device[devinx].DevAttr[port] &= ~RESET_ONCE_WHEN_SW_MASK; //if target port has been assign reset
		}

#if (SYSTEM_EXTENDER_TRANSMITTER)
		USBDC_Device[devinx].EndpIntrAttr = 0;
		USBDC_Device[devinx].EndpIntrWait = 0;
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */
	}

	return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : USBDC_Standard_Feature_Control.
* Description    : Set or Clear a specific feature of Device/Interface/endpoint
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT USBDC_Standard_Feature_Control(U8_T devinx,U8_T port,FlagStatus state)
{
	RESULT	Result = USB_UNSUPPORT;
	U8_T	endpidx;
	U8_T	endpaddr;

	switch (Type_Recipient)
	{
		case DEVICE_RECIPIENT: //0-Device, remote wakeup
			if (state == SET)
			{
				USBDC_Device[devinx].Feature[port] |= FEATURE_REMOTE;
				USBDC_UpPortState[port] |= USBDC_ROOTHUB_REMOTE_WAKEUP;
			}
			else
			{
				USBDC_Device[devinx].Feature[port] &= ~FEATURE_REMOTE;
				USBDC_UpPortState[port] &= ~USBDC_ROOTHUB_REMOTE_WAKEUP;
			}
			Result = USB_SUCCESS;
			break;
		case ENDPOINT_RECIPIENT: //Endpoint, STALLED
			USBDC_Search_EndpAddr(devinx,USBDC_Device[devinx].Setup[port].b.wIndex.bw.msb,&endpidx);
			endpaddr = USBDC_Device[devinx].Setup[port].b.wIndex.bw.msb & ~0x80;
			if (endpaddr <= 0x07)
			{
				if (state == SET)
				{
					USBDC_VIRTUAL_Endp_Stall_Control(port,devinx,endpidx,state);
					USBDC_Device[devinx].Endp_Stall[port] |= (0x01 << endpidx);
				}
				else
				{
					if ((USBDC_Device[devinx].Endp_Stall[port] & (0x01 << endpidx)) == 0)
					{
						return USB_UNSUPPORT;
					}
					USBDC_VIRTUAL_Endp_Stall_Control(port,devinx,endpidx,state);
					USBDC_Device[devinx].Endp_Stall[port] &= ~(0x01 << endpidx);

#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
					if (devinx == USBDC_VMSC_DEVINX)
					{
						USBDC_MSC_Stall_Clear(devinx);
					}
#endif
				}
			}
#if (SYSTEM_EXTENDER_MSC_SUPPORT)
#if (SYSTEM_EXTENDER_TRANSMITTER)
			if ((devinx == EXTENDER_MSC_Devinx) && (USB_PDevice[devinx].DevClass == USB_MSC_CLASS))
			{
				Result = USB_WAIT;
				if ((Request_No == CLEAR_FEATURE) && (USBDC_Device[devinx].Setup[port].b.wValue.w == 0x00))
				{
					//should create a IN after passthrough complete
					if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_STALL)
					{
						USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_CSW_REQUEST;
					}
				}
			}
			else
#endif //#if (SYSTEM_EXTENDER_TRANSMITTER)
#endif //#if (SYSTEM_EXTENDER_MSC_SUPPORT)
#if (SYSTEM_MSC_DEVICE_SUPPORT)
			if ((devinx == USBHC_MSC_Devinx) && (USB_PDevice[devinx].DevClass == USB_MSC_CLASS))
			{
				Result = USB_WAIT;
				if ((Request_No == CLEAR_FEATURE) && (USBDC_Device[devinx].Setup[port].b.wValue.w == 0x00))
				{
					//should create a IN after passthrough complete
					USBHC_MSC_State |= MSC_STATE_CHECK_BULK_IN;
				}
			}
			else
#endif
			{
				Result = USB_SUCCESS;
			}
			break; //Endpoint, STALLED
	}
	return Result;
}

/*******************************************************************************
* Function Name  : USBDC_Standard_SetInterface.
* Description    : This routine is called to set the interface.
*                  Then each class should configure the interface them self.
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT USBDC_Standard_SetInterface(U8_T devinx,U8_T port)
{
	/*Test if the specified Interface and Alternate Setting are supported by
	the application Firmware*/
	if (devinx==0) // check hub first
	{
		if ((wIndex_L > 0) || (wValue_L > 0))
			return USB_UNSUPPORT;
	}

	if (wValue_L != USBDC_Device[devinx].Current_AlternateSetting[port][wIndex_L])
	{
		USBDC_Device[devinx].Current_AlternateSetting[port][wIndex_L]  = wValue_L;
		
		if (USB_PDevice[devinx].DevClass == USB_AUDIO_CLASS)
		{
			if (port == KVM_CurrentUSBAudio)
			{
				USB_PDevice[devinx].Current_AlternateSetting[wIndex_L] = wValue_L;
				USBHC_Set_Dev_Interface(devinx,wIndex_L,wValue_L); // interface,alternate setting				
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
#if (SYSTEM_EXTENDER_RECEIVER)	|| (SYSTEM_KVM)							
				if ((wIndex_L == HC_IsoTransfer_Table[USB_HC_ISTL_OUT_MAX].InterfaceIdx) &&
					(wValue_L == 0x01) &&
					(HC_IsoTransfer_Table[USB_HC_ISTL_OUT_MAX].Used_DeviceID & USBHC_DEVICE_USED_MASK))
				{
					EXTINT3_DISABLE;
					USBHC_AUDIO_In_Start();
					EXTINT3_ENABLE;
				}
#endif  /*	#if (SYSTEM_EXTENDER_RECEIVER) */									
#endif	/*	#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */		
			}
		}
		else if (port == KVM_CurrentHost)	
		{
			USB_PDevice[devinx].Current_AlternateSetting[wIndex_L] = wValue_L;
			USBHC_Set_Dev_Interface(devinx,wIndex_L,wValue_L); // interface,alternate setting
		}
	}

	return USB_SUCCESS;
}

/*******************************************************************************
* RESULT USBDC_Class_SetIdle(U8_T devinx)
* Description    : This routine is called to set the interface.
*                  Then each class should configure the interface them self.
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT USBDC_Class_SetIdle(U8_T devinx)
{
	U8_T	infidle;
	
	infidle = 0x01 << wIndex_L;
	if (USB_PDevice[devinx].Hid_SetIdle_Allow & infidle)
	{
		USB_PDevice[devinx].InfSetIdle[wIndex_L] = wValue_H; // store the idle counter
		return USB_SUCCESS;
	}
	
	return USB_UNSUPPORT;
}

/*******************************************************************************
* Function Name  : USBDC_Class_SetReport.
* Description    : Every device have to provide this feature
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT USBDC_Class_SetReport(U8_T devinx,U8_T port)
{
	U8_T report_type;
	U8_T report_id;
	
	report_type = USBDC_Device[devinx].Setup[port].b.wValue.bw.lsb;
	report_id   = USBDC_Device[devinx].Setup[port].b.wValue.bw.msb;
	
	if (report_type != 0x02) // standard led command
	{
		if (USB_PDevice[devinx].DevAttr & DEVATTR_TD2220_MASK)
		{
			if (report_type == 0x03 && report_id == 0x0C)
			{
				USBDC_Device[devinx].DevAttr[port] |= TD2220_FORMAT_C14;
				if (port == KVM_CurrentHost)
				{
					USB_PDevice[devinx].DevAttr |= TD2220_FORMAT_C14;
				}
				else
				{
					return USB_UNSUPPORT;
				}
			}
		}

		//Not Standard Class Report
		if (devinx >= USBDC_VIRTUAL_DEVINX) // this is virtual devcie base address
		{
			return USB_WAIT; // need to passthrough
		}
	}
	else
	{
		if (report_id >= 0x02)
		{
			//1.Special Handle for Logitech wireless receive serials
			if (USB_PDevice[devinx].DevAttr & DEVATTR_UNIFYING_MASK) //Logitech Unifying wirelesee hid device
			{
				if (USBDC_Unifying_SetReport_Cmd_Check(port,devinx,report_id) != USB_WAIT)
				{
					goto USBDC_Class_SetReport_Next;
				}
			}
			return  USB_WAIT;
		}
		else if (USBDC_Device[devinx].Setup[port].b.wLength.w >= 16)
		{
			return USB_WAIT;
		}
		
USBDC_Class_SetReport_Next:
		USBDC_Device[devinx].Ctrl_TotalByte[port]   = USBDC_Device[devinx].Setup[port].b.wLength.w;
		USBDC_Device[devinx].Ctrl_CurrentByte[port] = 0;
	}
	return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : USBDC_Class_GetProtocol.
* Description    : Every device have to provide this feature
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT USBDC_Class_GetProtocol(U8_T devinx,U8_T port)
{
	USBDC_Device[devinx].Ctrl_TotalByte[port] = USBDC_Device[devinx].Setup[port].b.wLength.w;
	USBDC_Device[devinx].Ctrl_CurrentByte[port] = 0;
	wValue_H = 0x01 << wIndex_L; // get interface bit position
	
	if (USBDC_Device[devinx].Current_Protocol[port] & wValue_H)  // boot protocol
	{
		Port_Protocol[port] = 0; // boot protocol
	}
	else
	{
		Port_Protocol[port] = 1; // report porotocol
	}
	USBDC_Device[devinx].Control_EndpBuf[port] = &Port_Protocol[port];
	return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : USBDC_Class_SetProtocol.
* Description    : Every device have to provide this feature
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT USBDC_Class_SetProtocol(U8_T devinx,U8_T port)
{
	U8_T	hcprotocol;
	U8_T	curinf;
	
	wValue_H = (0x01 << wIndex_L); // Interface number setting
	
	if (wValue_L == 0) // Boot protocol
	{
		USBDC_Device[devinx].Current_Protocol[port] |= wValue_H;   //Boot Protocol
		
		/* If host of upper system set boot protocol for interface 0, we set all
		   bootable interfaces to boot protocol.
		*/
		if (wIndex_L == 0)
		{
			for (curinf = 1; curinf < (USB_PDevice[devinx].InterfaceNum & USBHC_DEVICE_INTERFACENUM_MASK); curinf++)
			{
				if (USB_PDevice[devinx].InfClass[curinf] == USB_HID_CLASS &&
					USB_PDevice[devinx].InfSubClass[curinf])
				{
					USBDC_Device[devinx].Current_Protocol[port] |= (0x01 << curinf);
				}
			}
		}
	}
	else
	{
		USBDC_Device[devinx].Current_Protocol[port] &= ~wValue_H;  //Report Protocol
	}

	/*Handle HC Protocol Setting */
//#ifdef HIKDVR	
//	hcprotocol = 0;
//#else	
	if ((port == KVM_CurrentHost) && (devinx >= USBDC_VIRTUAL_DEVINX)) // active port
	{
		hcprotocol = USBDC_Device[devinx].Current_Protocol[port] & wValue_H;
		USB_PDevice[devinx].Current_Protocol &=	~wValue_H;
		USB_PDevice[devinx].Current_Protocol |=	hcprotocol;
		return USB_WAIT;
	}
//#endif	
	return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : USBDC_Class_GetReport.
* Description    : Every device have to provide this feature
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
 
RESULT USBDC_Class_GetReport(U8_T devinx,U8_T port)
{
	RESULT Result=USB_UNSUPPORT;
	U8_T report_type;
	U8_T report_id;

	report_type = USBDC_Device[devinx].Setup[port].b.wValue.bw.lsb;
	report_id   = USBDC_Device[devinx].Setup[port].b.wValue.bw.msb;

#if (PROJECT_USB_GENERIC_HID_ENABLE)
	if ( (devinx == USBDC_VHID_DEVINX) || ((report_id == 0) && (report_type == 0x02)))// for virtual HID device
	{
		Result=USBDC_VHid_Class_GetReport(devinx,port);
	}
	else //if (devinx == USBDC_VIRTUAL_DEVINX) // for virtual HID device
#endif /* #if (PROJECT_USB_GENERIC_HID_ENABLE)*/
	{
		if (devinx >= USBDC_VIRTUAL_DEVINX) // this is virtual devcie base address
		{
			Result = USB_WAIT; // need to passthrough
		}
	}
	return Result;
}
 
/*******************************************************************************
* Function Name  : USBDC_Standard_GetStatus.
* Description    : Copy the device request data to "StatusInfo buffer".
* Input          : - Length - How many bytes are needed.
* Output         : None.
* Return         : Return 0, if the request is at end of data block,
*                  or is invalid when "Length" is 0.
*******************************************************************************/
RESULT USBDC_Standard_GetStatus(U8_T devinx,U8_T port)
{
	U8_T Related_Endpoint;
	
	if ((USBDC_Device[devinx].Setup[port].b.wValue.w == 0) &&
		(USBDC_Device[devinx].Setup[port].b.wLength.w == 0x0002)&& 
		(wIndex_H == 0)
		)
	{
		//USBDC_VIRTUAL_Setup_Token(devinx,port);
	}
	else
	{
		return USB_UNSUPPORT;
	}
	/* Reset Status Information */
	StatusInfo.w = 0;

	if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT))
	{
		/*Get Device Status */
		/* Remote Wakeup enabled */
		if (_ValBit(USBDC_Device[devinx].Feature[port],5))
		{
			_SetBit(StatusInfo.bw.lsb, 1);
		}

		/* Bus-powered */
		if (_ValBit(USBDC_Device[devinx].Feature[port],6))
		{
			/* Self-powered */
			_SetBit(StatusInfo.bw.lsb, 0);
		}
		else /* Bus-powered */
		{
			_ClrBit(StatusInfo.bw.lsb, 0);
		}
	}
	/*Interface Status*/
	else if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
	{
	
	}
	/*Get EndPoint Status*/
	else if (Type_Recipient == (STANDARD_REQUEST | ENDPOINT_RECIPIENT))
	{
		Related_Endpoint = (wIndex_L & 0x0f);
		if (_ValBit(wIndex_L, 7)) 
		{
			/* IN endpoint */
			if (_ValBit(USBDC_Device[devinx].Endp_Stall[port],Related_Endpoint))
			{
				SetBit(StatusInfo.bw.lsb, 1); /* IN Endpoint stalled */
			}
		}
		else
		{
			/* OUT endpoint */
			if (_ValBit(USBDC_Device[devinx].Endp_Stall[port],Related_Endpoint))
			{
				SetBit(StatusInfo.bw.lsb, 1); /* OUT Endpoint stalled */
			}
		}
	}
	else
	{
		return USB_UNSUPPORT;
	}
	
	USBDC_Device[devinx].Ctrl_TotalByte[port]   = 2; // byte is 2
	USBDC_Device[devinx].Ctrl_CurrentByte[port] = 0;
	USBDC_Device[devinx].Control_EndpBuf[port] = (U8_T *)&StatusInfo;
	return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : Standard_GetConfiguration.
* Description    : Return the current configuration variable address.
* Input          : Length - How many bytes are needed.
* Output         : None.
* Return         : Return 1 , if the request is invalid when "Length" is 0.
*                  Return "Buffer" if the "Length" is not 0.
*******************************************************************************/
RESULT USBDC_Standard_GetConfiguration(U8_T devinx,U8_T port)
{
	if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT))
	{
		USBDC_Device[devinx].Ctrl_TotalByte[port]   = 1; // byte is 1
		USBDC_Device[devinx].Ctrl_CurrentByte[port] = 0;
		USBDC_Device[devinx].Control_EndpBuf[port] = (U8_T *)&USBDC_Device[devinx].Current_Configuration[port];
		return USB_SUCCESS;
	}

	return USB_UNSUPPORT;
}

/*******************************************************************************
* Function Name  : USBDC_Standard_GetInterface.
* Description    : Return the Alternate Setting of the current interface.
* Input          : Length - How many bytes are needed.
* Output         : None.
* Return         : Return 0, if the request is invalid when "Length" is 0.
*                  Return "Buffer" if the "Length" is not 0.
*******************************************************************************/
RESULT USBDC_Standard_GetInterface(U8_T devinx,U8_T port)
{
	if ((Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
		&& (USBDC_Device[devinx].Current_Configuration[port] != 0) && (USBDC_Device[devinx].Setup[port].b.wValue.w == 0)
		&& (wIndex_H == 0) && (USBDC_Device[devinx].Setup[port].b.wLength.w == 0x0001)
		)
	{
		USBDC_Device[devinx].Ctrl_TotalByte[port]   = 1; // byte is 1
		USBDC_Device[devinx].Ctrl_CurrentByte[port] = 0;
		USBDC_Device[devinx].Control_EndpBuf[port] = (U8_T *)&USBDC_Device[devinx].Current_AlternateSetting[port];
		return USB_SUCCESS;
	}
	return USB_UNSUPPORT;
}

/*******************************************************************************
* Function Name  : USBDC_VIRTUAL_Port_Clear.
* Description    : Return the Alternate Setting of the current interface.
* Input          : Length - How many bytes are needed.
* Output         : None.
* Return         : Return 0, if the request is invalid when "Length" is 0.
*                  Return "Buffer" if the "Length" is not 0.
*******************************************************************************/
void USBDC_VIRTUAL_Port_Clear(U8_T pid,U8_T start_devinx,U8_T end_devinx)
{
	U8_T index;	
	
	for (index=start_devinx; index < end_devinx ; index++)
	{
	/*In case during control transfering, the host pc go reset by user*/
		if (USBDC_Device[index].PassThroughState[pid] != PASSTHROUGH_IDLE)
		{
			malloc_free(USBDC_Device[index].Control_EndpBuf[pid]);
			USBDC_Device[index].Control_EndpBuf[pid] = 0;
			USBDC_Device[index].PassThroughState[pid]     = PASSTHROUGH_IDLE;
		}
		else if ((USBDC_Device[index].ControlState[pid] == OUT_DATA) || (USBDC_Device[index].ControlState[pid] == LAST_OUT_DATA))
		{
			malloc_free(USBDC_Device[index].Control_EndpBuf[pid]);
			USBDC_Device[index].Control_EndpBuf[pid] = 0;
		}
		
#if (SYSTEM_EXTENDER_TRANSMITTER)
		USBDC_Device[index].EndpIntrAttr = 0;
		USBDC_Device[index].EndpIntrWait = 0;
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */

		USBDC_Device[index].DevAddr[pid]=0;
		USBDC_Device[index].Current_Configuration[pid]=0;
		USBDC_Device[index].Current_Protocol[pid]=0;
		USBDC_Device[index].Feature[pid] &= BIT6;
		USBDC_Device[index].Endp_Stall[pid]=0;
		USBDC_Device[index].ISR_Handle[pid]=0;
		USBDC_Device[index].ControlState[pid]=0;
		USBDC_Device[index].Ctrl_TotalByte[pid]=0;
		USBDC_Device[index].Ctrl_CurrentByte[pid]=0;
		USBDC_Device[index].DevAttr[pid] &= RESET_ONCE_WHEN_SW_MASK;
		USBDC_Device[index].Reset_Issue[pid]=0;
		memset(USBDC_Device[index].Current_AlternateSetting,0x00,sizeof(USBDC_Device[index].Current_AlternateSetting));
	}
}

/*******************************************************************************
* Function Name  : USBDC_VIRTUAL_Port_Status_Control.
* Description    : Return the Alternate Setting of the current interface.
* Input          : Length - How many bytes are needed.
* Output         : None.
* Return         : Return 0, if the request is invalid when "Length" is 0.
*                  Return "Buffer" if the "Length" is not 0.
*******************************************************************************/
void USBDC_VIRTUAL_Port_Status_Control(U8_T pid,U8_T flag,FlagStatus state)
{
	U8_T index;
	
	for (index=0; index < USBDC_DEVICE_MAX ; index++)
	{
		if (state == RESET)
			USBDC_Device[index].Feature[pid] &= ~flag;
		else
			USBDC_Device[index].Feature[pid] |= flag;
	}
}

/*******************************************************************************
* Function Name  : USBDC_VIRTUAL_Endp_Stall_Control.
* Description    : Return the Alternate Setting of the current interface.
* Input          : Length - How many bytes are needed.
* Output         : None.
* Return         : Return 0, if the request is invalid when "Length" is 0.
*                  Return "Buffer" if the "Length" is not 0.
*******************************************************************************/
void USBDC_VIRTUAL_Endp_Stall_Control(U8_T port,U8_T devinx,U8_T endpinx,FlagStatus state)
{
	if (state == SET)
	{
		USBDC_REGS_Endp_ControlSet(port,devinx,endpinx,DA_CR_STALL_SET);
	}
	else
	{
		USBDC_REGS_Endp_ControlClear(port,devinx,endpinx,DA_CR_STALL_SET);
	}
}

/*******************************************************************************
* Function Name  : USBDC_VIRTUAL_Create_DeviceDesc.
* Description    : Generate the downstream port USB device description and 
*                  relate parameter in the "physical device table"
* Input          :  
* Output         : None.
* Return         : Return 0, if the request is invalid when "Length" is 0.
*                  Return "Buffer" if the "Length" is not 0.
*******************************************************************************/
void USBDC_VIRTUAL_Create_DeviceDesc(U8_T devid,U8_T *device_desc)
{
	USB_PDevice[devid].Desc[DEVICE_DESC].Len = device_desc[0];
	USB_PDevice[devid].Desc[DEVICE_DESC].Ptr = device_desc;
	USBHC_Parser_DevDesc(devid,device_desc);
}

/*******************************************************************************
* Function Name  : USBDC_VIRTUAL_Create_ConfigDesc.
* Description    : Generate the downstream port USB configuration description and 
*                  relate parameter in the "physical device table"
* Input          :  
* Output         : None.
* Return         : Return 0, if the request is invalid when "Length" is 0.
*                  Return "Buffer" if the "Length" is not 0.
*******************************************************************************/
void USBDC_VIRTUAL_Create_ConfigDesc(U8_T devid,U8_T *config_desc)
{
	USBHC_CfgDesc_TypeDef *conf_desc;
	
	conf_desc = (USBHC_CfgDesc_TypeDef *)(config_desc);
	USB_PDevice[devid].Desc[CONFIG_DESC].Len = Big_Endian_16_Convert(conf_desc->wTotalLength);
	USB_PDevice[devid].Desc[CONFIG_DESC].Ptr = config_desc;
	USB_PDevice[devid].CfgTotalLen = USB_PDevice[devid].Desc[CONFIG_DESC].Len; // should kill this after it
	USBHC_Parser_ConfigDesc(devid,USB_PDevice[devid].Desc[CONFIG_DESC].Len,config_desc);
}

/*******************************************************************************
* Function Name  : USBDC_VIRTUAL_Setup_Token.
* Description    : Proceed the processing of setup request with data stage.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USBDC_VIRTUAL_Setup_Token(U8_T devinx,U8_T port)
{
	Request_No = USBDC_Device[devinx].Setup[port].b.bRequest;
	Type_Recipient = USBDC_Device[devinx].Setup[port].b.bmRequestType & (REQUEST_TYPE | RECIPIENT);
	
	//Do Little endian convert to Big endian
	wValue_L = USBDC_Device[devinx].Setup[port].b.wValue.bw.msb;
	wValue_H = USBDC_Device[devinx].Setup[port].b.wValue.bw.lsb;
	
	wIndex_L = USBDC_Device[devinx].Setup[port].b.wIndex.bw.msb;
	wIndex_H = USBDC_Device[devinx].Setup[port].b.wIndex.bw.lsb;
}

/*******************************************************************************
* Function Name  : USBDC_Standard_GetDeviceDescriptor.
* Description    : Copy the device request data to "StatusInfo buffer".
* Input          : - Length - How many bytes are needed.
* Output         : None.
* Return         : Return 0, if the request is at end of data block,
*                  or is invalid when "Length" is 0.
*******************************************************************************/
RESULT USBDC_Standard_GetDeviceDescriptor(U8_T devinx,U8_T port)
{
	RESULT Result=USB_UNSUPPORT;
	U8_T  strinx;

	switch (wValue_H)
	{
		case DEVICE_DESCRIPTOR:  //0x01
		case CONFIG_DESCRIPTOR:  //0x02
			Result = USBDC_Standard_GetDescData_Post(devinx,port,(wValue_H-1));
			break;
		case STRING_DESCRIPTOR:  //0x03
			if (wValue_L <= 3) // language type
			{
				strinx = USBDC_Standard_GetDeviceDescriptor_Inx(devinx);				
				//-----------------------------------------------------------------------------------------------------------
				Result = USBDC_Standard_GetDescData_Post(devinx,port,strinx+2);					
			}
			else
			{
				//Special Handling for HUB class, due to hub class could be in init processing, so need to put
				//into local answer
				if (devinx < USBDC_VIRTUAL_DEVINX) 
				{
					if (wValue_L > 3)
					{
						Result=USB_UNSUPPORT;
					}
					else
					{	
						Result = USBDC_Standard_GetDescData_Post(devinx,port,(wValue_L+2));
					}	
				}
				else
				{
					//Result = USB_WAIT;
					Result = USBDC_Standard_GetDescData_Post(devinx,port,(wValue_L+2));
				}
			}
			break;
		case DEVICE_QUALIFIER_DESCRIPTOR:
		default:
			if (devinx < USBDC_VIRTUAL_DEVINX)
				Result = USB_UNSUPPORT;
			else
				Result = USB_WAIT;
			break;
	}
	return Result;
}

/*******************************************************************************
* Function Name  : USBDC_Class_GetDescriptor.
* Description    : Copy the device request data to "StatusInfo buffer".
* Input          : - Length - How many bytes are needed.
* Output         : None.
* Return         : Return 0, if the request is at end of data block,
*                  or is invalid when "Length" is 0.
*******************************************************************************/
RESULT USBDC_Class_GetDescriptor(U8_T devinx,U8_T port)	 	  
{
	RESULT Result=USB_UNSUPPORT;
	
	if (wValue_H == USB_DESC_TYPE_HID_REPORT)	// GET HID REPORT
	{
		Result = USBDC_Standard_GetDescData_Post(devinx,port,(wIndex_L+HID0_REPORT));
	}
	else if (wValue_H == USB_DESC_TYPE_HID_DESCRIPTOR)	// GET HID DESCRIPTOR
	{
		if (devinx >= USBDC_VIRTUAL_DEVINX)
			Result = USB_WAIT;
		else
			Result = USBDC_Standard_GetDescData_Post(devinx,port,USB_DESC_TYPE_HID_DESCRIPTOR);
	}
	else
	{
		if (devinx >= USBDC_VIRTUAL_DEVINX)
			Result = USB_WAIT;
	}
	
	return Result;
}

/*******************************************************************************
* Function Name  : USBDC_Standard_GetDescData_Post.
* Description    : Post the data to GetDescriptor command
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
RESULT USBDC_Standard_GetDescData_Post(U8_T devinx,U8_T port,U8_T descid)
{
#if (PROJECT_USB_GENERIC_HID_ENABLE)
	if ((descid == USB_DESC_TYPE_HID_DESCRIPTOR) && (devinx == USBDC_VHID_DEVINX))
	{
		USBDC_Device[devinx].Setup[port].b.wLength.w = 0x09;
		USBDC_Device[devinx].Ctrl_TotalByte[port]   = USBDC_Device[devinx].Setup[port].b.wLength.w;
		USBDC_Device[devinx].Ctrl_CurrentByte[port] = 0;
		if (wIndex_L == 0)
			USBDC_Device[devinx].Control_EndpBuf[port]  = (USB_PDevice[devinx].Desc[CONFIG_DESC].Ptr+18);
		else if (wIndex_L == 1)
			USBDC_Device[devinx].Control_EndpBuf[port]  = (USB_PDevice[devinx].Desc[CONFIG_DESC].Ptr+43);
		return USB_SUCCESS;
	}
#endif /* #if (PROJECT_USB_GENERIC_HID_ENABLE) */

	if ((USB_PDevice[devinx].Desc[descid].Len == 0) || (USB_PDevice[devinx].Desc[descid].Ptr == NULL))
	{
		if (devinx >= USBDC_VIRTUAL_DEVINX)
		{
			return USB_WAIT;
		}
		else
		{
			return USB_UNSUPPORT;
		}
	}
	
	if (descid == (0xee+2))
	{
		descid = MICROSOFT_OS;
	}

#if (SYSTEM_EXTENDER_TRANSMITTER)
 #if (EXTENDER_T_HID_REPORT_PASSTHROUGH)
	if (devinx < USBDC_VIRTUAL_DEVINX)
	{
		if (USBDC_Device[devinx].Setup[port].b.wLength.w > USB_PDevice[devinx].Desc[descid].Len)
		{
			USBDC_Device[devinx].Setup[port].b.wLength.w = USB_PDevice[devinx].Desc[descid].Len;
		}
	}
 #else
	if (USBDC_Device[devinx].Setup[port].b.wLength.w > USB_PDevice[devinx].Desc[descid].Len)
	{
		USBDC_Device[devinx].Setup[port].b.wLength.w = USB_PDevice[devinx].Desc[descid].Len;
	}
 #endif
#else
	if (USBDC_Device[devinx].Setup[port].b.wLength.w > USB_PDevice[devinx].Desc[descid].Len)
	{
		USBDC_Device[devinx].Setup[port].b.wLength.w = USB_PDevice[devinx].Desc[descid].Len;
	}
#endif /*#if (SYSTEM_EXTENDER_TRANSMITTER)*/

	USBDC_Device[devinx].Ctrl_TotalByte[port]   = USBDC_Device[devinx].Setup[port].b.wLength.w;
	USBDC_Device[devinx].Ctrl_CurrentByte[port] = 0;

	if (descid == SERIAL_ID)
	{
		USB_PDevice[devinx].Desc[descid].Ptr[USB_PDevice[devinx].Desc[descid].Len - 2] = USBDC_VIRTUAL_SerialByPort + port;
		USBDC_Device[devinx].Control_EndpBuf[port]  = USB_PDevice[devinx].Desc[descid].Ptr;
	}
	else
	{
#if (SYSTEM_EXTENDER_TRANSMITTER)
		if ((USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK) && (devinx >=USBDC_VREMOTE_HUB_DEVINX))
		{
#if (EXTENDER_T_HID_REPORT_PASSTHROUGH)
			if ((descid >= HID0_REPORT) && (descid <= HID3_REPORT))
			{
				return USB_WAIT; // start passthrough
			}
			else
#endif /* #if (EXTENDER_T_HID_REPORT_PASSTHROUGH) */
			{
				if (USBDC_Device[devinx].Setup[port].b.wLength.w > USB_PDevice[devinx].Desc[descid].Len)
				{
					USBDC_Device[devinx].Setup[port].b.wLength.w = USB_PDevice[devinx].Desc[descid].Len;
					USBDC_Device[devinx].Ctrl_TotalByte[port] = USB_PDevice[devinx].Desc[descid].Len;
				}
				USBDC_Device[devinx].Control_EndpBuf[port]  = USB_PDevice[devinx].Desc[descid].Ptr;
			}
		}
		else
#endif	//#if (SYSTEM_EXTENDER_TRANSMITTER)
		{
			#ifdef VPID_CHANGER
			if ((descid == DEVICE_DESC) && (devinx == USBDC_VHID_DEVINX))
			{				
				USBDC_Device[devinx].Control_EndpBuf[port]  = (U8_T *) &Ran_Desc[port];							
			}				
			else
			#endif	
			{	
				USBDC_Device[devinx].Control_EndpBuf[port]  = USB_PDevice[devinx].Desc[descid].Ptr;
			}				
		}
	}

	return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : USBDC_VIRTUAL_Setup_Process.
* Description    : Post the data to GetDescriptor command
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USBDC_VIRTUAL_Setup_Process(U8_T devinx,U8_T port)
{
	RESULT Result = USB_UNSUPPORT;
	bit    passthrough_mode = 0;

	USBDC_VIRTUAL_Setup_Token(devinx,port);

	if ((Type_Recipient & REQUEST_TYPE) == STANDARD_REQUEST) /* Standard Device Request*/
	{
		if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT)) /* Devcie Rceivent */
		{
			switch(Request_No)
			{
				case SET_CONFIGURATION:
					Result = USBDC_Standard_SetConfiguration(devinx,port);				
					break;
				case SET_ADDRESS:
					Result = USBDC_Standard_SetAddress(devinx,port);
					break;
				case SET_FEATURE:
					Result = USBDC_Standard_Feature_Control(devinx,port,SET);
					break;
				case CLEAR_FEATURE:
					Result = USBDC_Standard_Feature_Control(devinx,port,RESET);
					break;
				case GET_DESCRIPTOR:
					Result = USBDC_Standard_GetDeviceDescriptor(devinx,port);
					break;
				case SET_DESCRIPTOR: // this should pass through
					Result = USB_WAIT;	
					break;
				case GET_STATUS:
					Result = USBDC_Standard_GetStatus(devinx,port);
					break;
				case GET_CONFIGURATION:
					Result = USBDC_Standard_GetConfiguration(devinx,port);
					break;
			}
		}
		else if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))  /* Interface Receivent */
		{
			/* SET INTERFACE */
			switch (Request_No)
			{
				case SET_INTERFACE:
					Result = USBDC_Standard_SetInterface(devinx,port);
					break;
				case GET_INTERFACE:
					Result = USBDC_Standard_GetInterface(devinx,port);
					break;
				case GET_DESCRIPTOR:
					Result = USBDC_Class_GetDescriptor(devinx,port);
					break;
				case SET_FEATURE:
					break;
				case CLEAR_FEATURE:
					break;
				case GET_STATUS:
					break;
			}
		}
		else if (Type_Recipient == (STANDARD_REQUEST | ENDPOINT_RECIPIENT)) /* Devcie Rceivent */
		{
			/* SET ENDPOINT */
			switch (Request_No)
			{
				case GET_STATUS:
					break;
				case SET_FEATURE:
					Result = USBDC_Standard_Feature_Control(devinx,port,SET);
					break;
				case CLEAR_FEATURE:
					Result = USBDC_Standard_Feature_Control(devinx,port,RESET);
					break;
				default:
					//printf("Unknow request num\n\r");
					break;
			}
		}
	}
	else if ((Type_Recipient & REQUEST_TYPE) == CLASS_REQUEST) /* Class Interface Request*/
	{
		if (Type_Recipient != (CLASS_REQUEST | OTHER_RECIPIENT))
		{
			switch(Request_No)
			{
				case GET_REPORT:  //0x01
					if (_ValBit(USBDC_Device[devinx].Setup[port].b.bmRequestType, 7)) // Device to Host
						Result = USBDC_Class_GetReport(devinx,port);
					else
					{
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
						if (USBHC_Audio_Devinx == devinx)
						{
							if (port != KVM_CurrentUSBAudio)
							{
								Result = USB_SUCCESS;
								break;
							}
						}
#endif
						Result = USB_WAIT; //passthrough to Device
					}
					break;
				case GET_IDLE:    //0x02
					break;
				case GET_PROTOCOL://0x03 
					Result = USBDC_Class_GetProtocol(devinx,port);
					break;
				case SET_REPORT:  //0x09
					Result = USBDC_Class_SetReport(devinx,port);
					break;
				case SET_IDLE:    //0x0a
						Result = USBDC_Class_SetIdle(devinx);
					break;
				case SET_PROTOCOL://0x0b
					Result = USBDC_Class_SetProtocol(devinx,port);
					break;
				default:
					if ((USB_PDevice[devinx].DevClass != USB_HUB_CLASS)	&&
						(devinx >= USBDC_VIRTUAL_DEVINX))
					{
						Result = USB_WAIT; // Passthrough
					}
					else
					{
#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
						if (devinx == USBDC_VMSC_DEVINX)
						{
							Result = USBDC_VMSC_Setup_Process(devinx,port);
						}
#endif
					}
					break;
			}
		}
	}
	else if ((Type_Recipient & REQUEST_TYPE) == VENDOR_REQUEST) /* Vendor Request */
	{
#if (SYSTEM_VENDOR_COMMAND_SUPPORT)
		if (devinx == USBDC_VHUB_DEVINX) // this is ASIX Vendor Command
		{
			Result = USBDC_Vendor_Command_Handle(devinx,port);			
		}
		else 
#endif
		{
			if (devinx >= USBDC_VIRTUAL_DEVINX) // this is virtual devcie base address
			{
				Result = USB_WAIT; // will pass through to device 
			}
			else
			{
				Result = USB_UNSUPPORT;
			}
		}
	}

	//Check the Data stage
	if (USBDC_Device[devinx].Setup[port].b.wLength.w == 0)
	{
		/* Setup with no data stage */
		/* Consider the passthrough, clear the out buffer ready flag here */
		USBDC_REGS_Endp_ControlClear(port,devinx,0,(DA_CR_SCLR_SET|DA_CR_BCLR_SET)); // now can receive data

		if ((Result != USB_SUCCESS) && (Result != USB_WAIT))
		{
			if (USB_PDevice[devinx].DevClass == USB_HUB_CLASS)
			{
				Result = USBDC_VHub_Setup_Process(devinx,port);
			}

			/*Error*/	 
			if (Result == USB_UNSUPPORT)
			{
				goto USBDC_SETUP_STALL;
			}
		}

		USBDC_Device[devinx].ControlState[port] = WAIT_STATUS_IN;/* After no data stage SETUP */
		if (Result == USB_WAIT) // passthrough state
		{
			/* Start the Passthroug procedure */
			TASK_Active(TASK_TYPE_EVENT,TASK_USBDC_VIRTUAL_PassThrough_Setup_Wait_ID,0x00,devinx,port,0);  // Generate the task, for next check
			/* USB wait for HC answer */
		}
		else
		{
			USBDC_VIRTUAL_Control_Statue_In(devinx,port);
		}
	}
	else
	{
		/* Setup with data stage */
		if (Result == USB_UNSUPPORT)
		{
			if (USB_PDevice[devinx].DevClass == USB_HUB_CLASS)
			{
				Result = USBDC_VHub_Setup_Process(devinx,port);
			}

			if (Result == USB_UNSUPPORT)
			{
				goto USBDC_SETUP_STALL;
			}
		}

		if (Result == USB_WAIT)
		{
			/* Start the Passthroug procedure */
			TASK_Active(TASK_TYPE_EVENT,TASK_USBDC_VIRTUAL_PassThrough_Setup_Wait_ID,0x00,devinx,port,0);  // Generate the task, for next check  
			passthrough_mode = 1;
			/* USB wait for HC answer */
		}
		else
		{
#if (SYSTEM_EXTENDER_TRANSMITTER)
			//if (USB_PDevice[devinx].Hc.gState == HOST_PASSTHROUGH)
			if (USBDC_Device[devinx].PassThroughState[port] != PASSTHROUGH_IDLE)
			{
				printf("\n\r P:%bu,DC_Force_Clear Passthrough \n\r",port);			
				USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
				USBDC_Device[devinx].PassThroughState[port] = PASSTHROUGH_IDLE;
				USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_IDLE;
				malloc_free(USB_PDevice[devinx].Hc.Control.Buf);
				USB_PDevice[devinx].Hc.Control.Buf = 0;
				USB_PDevice[devinx].Hc.gState = USB_PDevice[devinx].Hc.PassThrough_gState_bk;
				if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)
				{
					Tansmitter_PassThrough_Buf_In_Used_Flag = 0;
				}
				else
					USBHC_CORE_Free_TD(USB_PDevice[devinx].Hc.Control.TdNum);
			}
#else
			if ((USB_PDevice[devinx].Hc.gState == HOST_PASSTHROUGH) && (USBDC_Device[devinx].PassThroughState[port] != PASSTHROUGH_IDLE))
			{
				//printf("*****\n\r");
				//printf("P:%bu,D:%bu,DC:%02bx,HC:%02bx,DC-PA:%02bx,HC-PA:%02bx\n\r",port,
				//													   devinx,
				//													   USBDC_Device[devinx].ControlState[port],
				//													   USB_PDevice[devinx].Hc.Control.State,
				//													   USBDC_Device[devinx].PassThroughState[port],
				//													   USB_PDevice[devinx].Hc.PassThrough_State);
				
				USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
				USBHC_CORE_Free_TD(USB_PDevice[devinx].Hc.Control.TdNum);
				USBDC_Device[devinx].PassThroughState[port] = PASSTHROUGH_IDLE;
				USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_IDLE;
				malloc_free(USB_PDevice[devinx].Hc.Control.Buf);
				USB_PDevice[devinx].Hc.Control.Buf = 0;
				USB_PDevice[devinx].Hc.gState = USB_PDevice[devinx].Hc.PassThrough_gState_bk;
			}
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */
		}

		// Judge the data direction
		if (_ValBit(USBDC_Device[devinx].Setup[port].b.bmRequestType, 7))
		{
			// Device ==> Host
			/*Consider the passthrough, clear the out buffer ready flag here */
			USBDC_REGS_Endp_ControlClear(port,devinx,0,(DA_CR_SCLR_SET|DA_CR_BCLR_SET)); // now can receive data

			if (passthrough_mode == 0)
			{
				if (Result != USB_NOT_READY)
				{	
					USBDC_VIRTUAL_Setup_DataStageIn(devinx,port,0);
				}	
			}
		}
		else
		{
			// Host ==> Device
			/*Consider if data out, the out package will follow after setup very shortly, HC is possible that no time to 
			  send out the setup token, so dealy the out buffer complete flag*/
			if (passthrough_mode == 0) 
			{
				/*Consider the passthrough, clear the out buffer ready flag here */
				USBDC_REGS_Endp_ControlClear(port,devinx,0,(DA_CR_SCLR_SET|DA_CR_BCLR_SET)); // now can receive data
			}
			USBDC_VIRTUAL_Setup_DataStageOut(devinx,port,0);
		}
	}
		
USBDC_SETUP_STALL:
	if (Result == USB_UNSUPPORT)
	{
		/*Consider the passthrough, clear the out buffer ready flag here */
		USBDC_REGS_Endp_ControlClear(port,devinx,0,(DA_CR_SCLR_SET|DA_CR_BCLR_SET)); // now can receive data
		USBDC_HAL_Endp_Stalled(port,devinx,0);
	}
}

/*******************************************************************************
* void TASK_USBDC_VIRTUAL_PassThrough_Setup_Wait(void)
* Description    : create a passthroug control
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void TASK_USBDC_VIRTUAL_PassThrough_Setup_Wait(void)
{
	U8_T devinx;
	U8_T port;
	
	devinx = TASK_Register0;
	port   = TASK_Register1; // command from whicp upstream port

	//Skip task if the device is gone			 
	if ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00)  // Device is gone  
	{
		//printf("Devinx:%bx,PassThroug wait cancel\n\r", devinx);
		return;	
	}	 
	
	if (USB_PDevice[devinx].Hc.PassThrough_State)
	{
		if ((USBDC_Device[devinx].ControlState[port] == WAIT_STATUS_IN) ||
			(USBDC_Device[devinx].ControlState[port] == WAIT_STATUS_OUT))
		{			
#if (SYSTEM_EXTENDER_TRANSMITTER)
			if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)	//if not extender, then
			{
				if (USB_PDevice[devinx].Hc.PassThrough_State != PASSTHROUGH_IDLE)
				{
					ExtenderT_Passthrough_Condition_Reset(devinx);					
					goto VIRTUAL_PassThrough_Setup_Wait_Jump;
				}
			}
#else
   			TASK_Wait_Current();
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */	
		}
		else
		{	
			TASK_Wait_Current();
		}	
	}
	else
	{
#if (SYSTEM_EXTENDER_TRANSMITTER)
VIRTUAL_PassThrough_Setup_Wait_Jump:
		if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)//if remote device
		{
			if (Tansmitter_PassThrough_Buf_In_Used_Flag)
			{
				TASK_Wait_Current();
				return;
			}
			Tansmitter_PassThrough_Buf_In_Used_Flag = 1;
		}
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */

		memcpy((U8_T *)&USB_PDevice[devinx].Hc.PControl.Setup,(U8_T *)&USBDC_Device[devinx].Setup[port],8);
		USBDC_Device[devinx].PassThroughState[port] = PASSTHROUGH_SETUP_START; /* Indicate the device is in PassThrough Mode */
		TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_PassThrough_Handle_Start_ID,0x00,devinx,port,0);  // Generate the task, for next check
	}
}

/*******************************************************************************
* void USBDC_VIRTUAL_SetCheck_Interval(U8_T port,U8_T devinx,U8_T endpinx)
* Description    : create a passthroug control
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USBDC_VIRTUAL_SetCheck_Interval(U8_T port,U8_T devinx,U8_T endpinx)
{
	U8_T dev_endp;
	
	USBDC_Device[devinx].EndpInterval0[port][endpinx] = 0;
	USBDC_Device[devinx].EndpInterval1[port][endpinx] = 0;
	USBDC_Device[devinx].Interval_Check[port] |= (0x01 << endpinx);
	dev_endp = (devinx << 4) | endpinx;
	TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_USBDC_Get_EndpInterval_ID,port,dev_endp,500,500);  //after 200ms check the result
}

/*******************************************************************************
* U8_T USBDC_Standard_GetDeviceDescriptor_Inx(U8_T devinx)
* Description    : search the string index for Iman,Iproduct,iserial
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
U8_T USBDC_Standard_GetDeviceDescriptor_Inx(U8_T devinx)
{
	U8_T strinx,*pr,strinxfix;
	USBHC_DevDesc_TypeDef *devsc;

	//search the first index
	//Fixed if the string index error
	//-----------------------------------------------------------------------------------------------------------
	strinxfix = wValue_L;
	if (wValue_L) 
	{		
		devsc = (USBHC_DevDesc_TypeDef *)(USB_PDevice[devinx].Desc[DEVICE_DESC].Ptr);
		pr = &(devsc->iManufacturer);			
		for (strinx = 1; strinx <= 3; strinx++)
		{				
			if (pr[strinx-1] == wValue_L)
			{
				strinxfix = strinx;
				break;
			}						
		}							
	}	
	return strinxfix;

}
/* End of usbdc_virtual_core.c */
