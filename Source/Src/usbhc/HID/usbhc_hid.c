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
 * Module Name : usbhc_hid.c
 * Purpose     :  
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 *               HID Class Request(bRequest)
 *               [v].Get Report(0x01)
 *               [ ].Get Idle(0x02)
 *               [ ].Get Protocol(0x03)
 *               [v].Set Report(0x09)
 *               [v].Set Idle(0x0A)
 *               [v].Set Protocol(0x0B)
 *               [v].Stall 
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	<string.h>
#include	"project_include.h"

/* STATIC VARIABLE DECLARATIONS */ 
U8_T	TASK_USBHC_HID_Class_Enumerate_ID;
HID_Rpt_Parser_TypeDef	USBHC_HidKBParserTempory;
HID_Rpt_Table_TypeDef	USBHC_HidKBTableTempory[USB_HC_MAX_DEVICE];

/* LOCAL SUBPROGRAM BODIES */
static RESULT usbhc_Class_SetIdle(U8_T devinx,U8_T td_id,U8_T inf_id,U8_T duration);
static RESULT usbhc_Class_GetRptDesc(U8_T devinx,U8_T td_id,U8_T inf_id);
#if defined(HID_PARSER_CORE_KB)
extern void USBHC_HidParser_ReportDescription(U8_T devinx,U8_T inf_id);
#else
extern void USBHC_HidParser_ReportDescription(U8_T devinx);
#endif
static RESULT usbhc_Class_SetProtocol(U8_T devinx,U8_T td_id,U8_T inf_id,U8_T mode);

/**
  * @brief  HID_Class_Enum_Init
  *         init the HID Class Enumerate relate
  * @param  pdev: Selected device
  * @retval Status
  */
void HID_Class_Enum_Init(void)
{
	TASK_USBHC_HID_Class_Enumerate_ID = TASK_Create(TASK_USBHC_HID_Class_Enumerate);

#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("TASK_USBHC_HID_Class_Enumerate_ID=%bu\n\r",TASK_USBHC_HID_Class_Enumerate_ID);	
#endif	
}

/**
  * @brief  USBHC_HID_Interface_Check
  *          
  * @param  pdev: Selected device
  * @retval Status
  */
void USBHC_HID_Interface_Check(U8_T devinx,U8_T inf_id)
{
	//printf("Interface:%bu, Protocol:%02bx\n\r",inf_id,USB_PDevice[devinx].InfProtocol[inf_id]);
#ifndef HID_PARSER_CORE_KB
	if (USB_PDevice[devinx].InfProtocol[inf_id] == INF_KEYBOARD)
#endif
	{		
#ifndef HID_PARSER_CORE_KB		
		if ((USB_PDevice[devinx].Interval_Adjust & BIT7) == 0)		
#endif			
		{	
			//1_1.Keyboard Parser
			USB_PDevice[devinx].HID_Kb_Parser = &USBHC_HidKBParserTempory;
			memset((U8_T *)USB_PDevice[devinx].HID_Kb_Parser, 0, sizeof(HID_Rpt_Parser_TypeDef));
			USB_PDevice[devinx].HID_Kb_Parser->Desc_RemainLen  = USB_PDevice[devinx].HID_Rpt_Len[inf_id];
			USB_PDevice[devinx].HID_Kb_Parser->HID_DescriptorP = USB_PDevice[devinx].Hc.Control.Buf;
	
			USB_PDevice[devinx].HID_Kb_ParserTable = &USBHC_HidKBTableTempory[devinx];
			memset((U8_T *)USB_PDevice[devinx].HID_Kb_ParserTable, 0, sizeof(HID_Rpt_Table_TypeDef));
#if defined(HID_PARSER_CORE_KB)	
			USBHC_HidParser_ReportDescription(devinx,inf_id);
#else	
			USBHC_HidParser_ReportDescription(devinx);
#endif
			USB_PDevice[devinx].HID_Kb_ParserTable->PackageLen >>= 3;
#ifndef HID_PARSER_CORE_KB					
			USB_PDevice[devinx].Interval_Adjust |= BIT7;		
#endif			
		}	
	}
}

/**
  * @brief  USBH_HandleControl
  *         Handles the USB control transfer state machine
  * @param  pdev: Selected device
  * @retval Status
  */
void TASK_USBHC_HID_Class_Enumerate(void)
{
	idata U8_T	devinx,inf_id,td_id;
	RESULT		Result=USB_ERROR;

	devinx = TASK_Register0;
	inf_id = TASK_Event;
	td_id = USB_PDevice[devinx].Hc.Control.TdNum;
	TASK_Destory_Current();

	//Skip task if the device is gone
	if ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00)  // Device is gone  
	{
		return;
	}
	
TASK_USBHC_HID_Enumerate_Handle_Restart:
	switch (USB_PDevice[devinx].Hc.HID_EnumState)
	{
		case HID_ENUM_SET_IDLE: //Start to Get device description
			Result = usbhc_Class_SetIdle(devinx,td_id,inf_id,0);
			break;
		case HID_ENUM_GET_RPT_DESC:
			Result = usbhc_Class_GetRptDesc(devinx,td_id,inf_id);
			break;
//#ifdef HIKDVR		
//		case HID_ENUM_SET_PROTOCOL: //Start to Get device description
//			Result = usbhc_Class_SetProtocol(devinx,td_id,inf_id,0);
//			break;
//#endif		
		default:
			break;
	}

	if ((USB_PDevice[devinx].Hc.Control.State == CTRL_FAIL) ||
		(USB_PDevice[devinx].Hc.HID_EnumState == HID_ENUM_IDLE) ||
		(USB_PDevice[devinx].Hc.HID_EnumState == HID_ENUM_FAIL))
	{
		USBHC_CORE_Free_TD(td_id);
		if (USB_PDevice[devinx].Hc.HID_EnumState == HID_ENUM_IDLE)
		{
			//HID Report Parser
			USBHC_HID_Interface_Check(devinx,inf_id);

			inf_id++;
			if (inf_id < (USB_PDevice[devinx].InterfaceNum & USBHC_DEVICE_INTERFACENUM_MASK))
			{
				USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_SET_IDLE;
				USB_PDevice[devinx].Hc.Control.InfAddr = inf_id;
				goto TASK_USBHC_HID_Enumerate_Handle_Restart;
			}
		}

		USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_IDLE;
		USB_PDevice[devinx].Hc.Control.Buf = NULL;
		USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
		USB_PDevice[devinx].Hc.gState = HOST_CLASS_ENUMERATION; // next go other Class Maintain.
		TASK_Active(TASK_TYPE_USB,TASK_USBHC_Process_ID,0x00,devinx,0,0); // Generate the task
		return;
	}

	if (Result == USB_SUCCESS)
	{
		goto TASK_USBHC_HID_Enumerate_Handle_Restart;
	}
	else if (Result == USB_STALLED_WAIT)
	{
		TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_USBHC_HID_Class_Enumerate_ID, inf_id, devinx, 2000, 0);  // Generate the task
	}
}

/**
* @brief  USBHC_Classreq_SetIdle
*         Set Idle State. 
* @param  pdev: Selected device
* @param  duration: Duration for HID Idle request
* @param  reportID : Targetted report ID for Set Idle request
* @retval USBH_Status : Response for USB Set Idle request
*/
USBH_Status USBHC_Classreq_SetIdle (U8_T devinx,U8_T td_id,U8_T duration,U8_T reportId,U8_T interface)
{
	//1.TD Header init
	HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE;
	HCTD_Channel_Table[td_id].Total_Length = 0; // no data stage
	HCTD_Channel_Table[td_id].Current_Length = 0;
	HCTD_Channel_Table[td_id].Endp_Size = USBHC_MAX_PACKET_SIZE;
	//2.Setup Command Buffer prepare
	USB_PDevice[devinx].Hc.Control.Setup.b.bmRequestType = USB_H2D | USB_REQ_RECIPIENT_INTERFACE |USB_REQ_TYPE_CLASS;
	USB_PDevice[devinx].Hc.Control.Setup.b.bRequest = USB_HID_SET_IDLE;
	USB_PDevice[devinx].Hc.Control.Setup.b.wValue.w = Big_Endian_16_Convert((duration << 8 ) | reportId);
	USB_PDevice[devinx].Hc.Control.Setup.b.wIndex.w = Big_Endian_16_Convert(interface);
	USB_PDevice[devinx].Hc.Control.Setup.b.wLength.w = 0;

	return USBHC_CtlReq(devinx,td_id);
}

/**
* @brief  USBH_Get_HID_ReportDescriptor
*         Issue report Descriptor command to the device. Once the response 
*         received, parse the report descriptor and update the status.
* @param  pdev   : Selected device
* @param  Length : HID Report Descriptor Length
* @retval USBH_Status : Response for USB HID Get Report Descriptor Request
*/
USBH_Status USBHC_Classreq_GetRptDesc (U8_T devinx,U8_T td_id,U8_T interface,U16_T length)
{
	//1.TD Parameter
	HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE;
	HCTD_Channel_Table[td_id].Total_Length = length; // no data stage
	HCTD_Channel_Table[td_id].Current_Length = 0;
	//2.Descriptor Read
	return USBHC_GetDescriptor(devinx,
							   td_id,
							   USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_STANDARD,
							   USB_DESC_HID_REPORT,
							   interface,
							   length);
}

/**
* @brief  USBHC_Classreq_SetReport
*         Issues Set Report 
* @param  pdev: Selected device
* @param  reportType  : Report type to be sent
* @param  reportID    : Targetted report ID for Set Report request
* @param  reportLen   : Length of data report to be send
* @param  reportBuff  : Report Buffer
* @retval USBH_Status : Response for USB Set Idle request
*/
USBH_Status USBHC_Classreq_SetReport (U8_T devinx,U8_T td_id,U8_T interface,U8_T reportId,U8_T reportType,U16_T reportLen,U8_T *rptbuf)
{
	//1.HC Channel Control
	HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE;
	HCTD_Channel_Table[td_id].Total_Length = reportLen; // no data stage
	HCTD_Channel_Table[td_id].Current_Length = 0;
	//2.Setup Command Buffer prepare
	USB_PDevice[devinx].Hc.Control.Setup.b.bmRequestType = USB_H2D|USB_REQ_RECIPIENT_INTERFACE|USB_REQ_TYPE_CLASS;
	USB_PDevice[devinx].Hc.Control.Setup.b.bRequest = USB_HID_SET_REPORT;
	USB_PDevice[devinx].Hc.Control.Setup.b.wValue.w = (reportType << 8 ) | reportId;
	USB_PDevice[devinx].Hc.Control.Setup.b.wIndex.w = interface;
	USB_PDevice[devinx].Hc.Control.Setup.b.wLength.w= reportLen;
	USBDC_HAL_Convert_Setup_Value((U8_T *)&USB_PDevice[devinx].Hc.Control.Setup);

	USB_PDevice[devinx].Hc.Control.Buf = rptbuf;

	return USBHC_CtlReq(devinx,td_id);
}

/**
* @brief  USBHC_Classreq_SetProtocol
*         Issues Set Protocol command
* @param  devinx: Selected device
* @param  td_id    : TD id
* @param  intf_id  : Interface ID
* @param  mode     : Protocol mode value(Muse 0/1)
* @retval USBH_Status : Response for USB Set Idle request
*/
USBH_Status USBHC_Classreq_SetProtocol (U8_T devinx,U8_T td_id,U8_T intf_id,U8_T mode)
{
	//1.HC Channel Control
	HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE;
	HCTD_Channel_Table[td_id].Total_Length = 0; // no data stage
	HCTD_Channel_Table[td_id].Current_Length = 0;
	//2.Setup Command Buffer prepare
	USB_PDevice[devinx].Hc.Control.Setup.b.bmRequestType = USB_H2D|USB_REQ_RECIPIENT_INTERFACE|USB_REQ_TYPE_CLASS;
	USB_PDevice[devinx].Hc.Control.Setup.b.bRequest = USB_HID_SET_PROTOCOL;
	USB_PDevice[devinx].Hc.Control.Setup.b.wValue.w = mode;
	USB_PDevice[devinx].Hc.Control.Setup.b.wIndex.w = intf_id;
	USB_PDevice[devinx].Hc.Control.Setup.b.wLength.w= 0;
	USBDC_HAL_Convert_Setup_Value((U8_T *)&USB_PDevice[devinx].Hc.Control.Setup);

	return USBHC_CtlReq(devinx,td_id);
}

/**
* @brief  USBHC_Classreq_SetProtocol
*         Issues Set Protocol command
* @param  devinx: Selected device
* @param  td_id    : TD id
* @param  intf_id  : Interface ID
* @param  mode     : Protocol mode value(Muse 0/1)
* @retval USBH_Status : Response for USB Set Idle request
*/
//USBH_Status USBHC_Classreq_SetConfigurtion(U8_T devinx,U8_T td_id,U8_T intf_id,U8_T mode)
//{
	//1.HC Channel Control
//	HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE;
//	HCTD_Channel_Table[td_id].Total_Length = 0; // no data stage
//	HCTD_Channel_Table[td_id].Current_Length = 0;
	//2.Setup Command Buffer prepare
//	USB_PDevice[devinx].Hc.Control.Setup.b.bmRequestType = USB_H2D|USB_REQ_RECIPIENT_INTERFACE|USB_REQ_TYPE_CLASS;
//	USB_PDevice[devinx].Hc.Control.Setup.b.bRequest = USB_HID_SET_PROTOCOL;
//	USB_PDevice[devinx].Hc.Control.Setup.b.wValue.w = mode;
//	USB_PDevice[devinx].Hc.Control.Setup.b.wIndex.w = intf_id;
//	USB_PDevice[devinx].Hc.Control.Setup.b.wLength.w= 0;
//	USBDC_HAL_Convert_Setup_Value((U8_T *)&USB_PDevice[devinx].Hc.Control.Setup);

//	return USBHC_CtlReq(devinx,td_id);
//}

/*----------------------------------------------------------------------------
 * CTRL_State usbhc_Class_SetIdle(U8_T devinx,U8_T td_id,U8_T duration))
 * Purpose : Perform the set address operation of enumearation
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
static RESULT usbhc_Class_SetIdle(U8_T devinx,U8_T td_id,U8_T inf_id,U8_T duration)
{
	RESULT	Result=USB_ERROR;
	
	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{	
		if (USB_PDevice[devinx].InfClass[inf_id] == USB_HID_CLASS)	//if has hid report, need to set idle command support
		{
			if (USBHC_Control_TD_Init(devinx,&td_id) == USBH_OK)
			{
				USBHC_Classreq_SetIdle(devinx,td_id,duration,0,inf_id);
				Result=USB_BUSY;
			}
			else
			{
				USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_FAIL;
			}
		}
		else
		{
			USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_IDLE;
			Result=USB_SUCCESS;
		}
	}
	else
	{
		if ((USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE) || (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED))
		{
			if (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED)
			{
				Result=USB_STALLED_WAIT;
			}
			else
			{
				if (USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE)
				{
					USB_PDevice[devinx].Hid_SetIdle_Allow |= (0x01 << inf_id);
				}

				Result=USB_SUCCESS;
			}

			USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_GET_RPT_DESC;
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
		}
		else
		{
			USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_FAIL;
		}
	}
	return Result;
}

/*----------------------------------------------------------------------------
 * CTRL_State usbhc_Class_GetRptDesc(U8_T devinx,U8_T td_id)
 * Purpose : Perform the set address operation of enumearation
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
RESULT usbhc_Class_GetRptDesc(U8_T devinx,U8_T td_id,U8_T inf_id)
{
	RESULT	Result=USB_ERROR;
	U16_T	rpt_len;

	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		//1.Get dynamic buffer memory
		rpt_len = USB_PDevice[devinx].HID_Rpt_Len[inf_id];
		if (rpt_len)
		{
			USB_PDevice[devinx].Hc.Control.Buf = m_malloc(rpt_len,16); /* allocate 18 bytes */
			HCTD_Channel_Table[td_id].Buf = USB_PDevice[devinx].Hc.Control.Buf; // also assign to host channel buffer
			if (USB_PDevice[devinx].Hc.Control.Buf != NULL)
			{
				//2.Set maximum package size
				USBHC_Classreq_GetRptDesc(devinx,td_id,inf_id,rpt_len);
				Result=USB_BUSY;
			}
			else
			{
				USBHC_Memory_Fail_Msg(0x04);
				// Terminate the Task .........
				// should do something
				USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_FAIL;
			}
		}
		else
		{
			USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_IDLE;
			Result=USB_SUCCESS;
		}
	}
	else
	{
		//parser descri_ptor
		if (USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE)
		{
			USB_PDevice[devinx].Desc[HID0_REPORT+inf_id].Len = USB_PDevice[devinx].HID_Rpt_Len[inf_id];
			USB_PDevice[devinx].Desc[HID0_REPORT+inf_id].Ptr = USB_PDevice[devinx].Hc.Control.Buf;
			USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_IDLE;
//#ifdef HIKDVR			
//			USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_SET_PROTOCOL; //HID_ENUM_IDLE;
//#else
			USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_IDLE;
//#endif			
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
			Result=USB_SUCCESS;
		}
		else
		{
			//Due to have to release the buffer, so need put the satement here
			//proces alos if the State is not COMPLETE
			malloc_free(USB_PDevice[devinx].Hc.Control.Buf);
			USB_PDevice[devinx].Hc.Control.Buf = 0;
			USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_FAIL;
		}
	}
	return Result;
}

#ifdef HIKDVR
/*----------------------------------------------------------------------------
 * CTRL_State usbhc_Class_SetProtocol(U8_T devinx,U8_T td_id,U8_T duration))
 * Purpose : Perform the set address operation of enumearation
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
/*
static RESULT usbhc_Class_SetProtocol(U8_T devinx,U8_T td_id,U8_T inf_id,U8_T mode)	 
{
	RESULT	Result=USB_ERROR;
	
	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{	
		if (USB_PDevice[devinx].InfClass[inf_id] == USB_HID_CLASS)	//if has hid report, need to set idle command support
		{
			if (USBHC_Control_TD_Init(devinx,&td_id) == USBH_OK)
			{
				USBHC_Classreq_SetProtocol(devinx,td_id,inf_id,mode);
				Result=USB_BUSY;
			}
			else
			{
				USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_FAIL;
			}
		}
		else
		{
			USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_IDLE;
			Result=USB_SUCCESS;
		}
	}
	else
	{
		if ((USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE) || (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED))
		{
			if (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED)
			{
				Result=USB_STALLED_WAIT;
			}
			else
			{
				memset(USBDC_Device[devinx].Current_Protocol,0xff,USBDC_PORT_MAX);				
				Result=USB_SUCCESS;
			}

			USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_IDLE;
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
		}
		else
		{
			USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_FAIL;
		}
	}
	return Result;
}
*/
#endif

/*----------------------------------------------------------------------------
 * CTRL_State USBHC_SCM_SetLedReport(U8_T devinx,U8_T td_id)
 * Purpose : Perform the set address operation of enumearation
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
RESULT USBHC_SCM_SetLedReport(U8_T devinx,U8_T td_id)
{
	RESULT	Result=USB_ERROR;
	U8_T	len,start,inf,pageid;
#if defined(HID_PARSER_CORE_KB)	
	ParserCore_Led_TypeDef *parserled;	
#endif
	
	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		//what is the interface id?
		inf = (USB_PDevice[devinx].HID_Kb_Buf_InfEndp & 0xf0) >>4;
		inf = (0x01 << inf);
#if defined(HID_PARSER_CORE_KB)	
		parserled = (ParserCore_Led_TypeDef *) USB_PDevice[devinx].HID_LED_Item;	
		pageid = parserled->Page_ID;
#else
		pageid = USB_PDevice[devinx].HID_Kb_ParserTable->Attr_PageID & HID_RPT_PAGEID_MASK;
#endif			
		if ( pageid && ((USB_PDevice[devinx].Current_Protocol & inf) == 0x00)) //must not in boot protocol
		{
			start = 0;	
			len = 2;
			USB_PDevice[devinx].HID_Kb_LED_Value[0] = pageid;
		}
		else
		{
			start = 1;
			len = 1;
		}
		USBHC_Classreq_SetReport(devinx,td_id,0x00,0x00,0x02,len,&USB_PDevice[devinx].HID_Kb_LED_Value[start]);
		Result=USB_BUSY;
	}
	else
	{
		if ((USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE) || (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED))
		{
			Result=USB_SUCCESS;
		}
		USB_PDevice[devinx].Hc.Control.Buf = NULL; // clear the buffer pointer
	}
	return Result;
}

/*----------------------------------------------------------------------------
 * CTRL_State USBHC_SCM_SetUnifyingReport(U8_T devinx,U8_T td_id,U8_T mode)
 * Purpose : Perform the set address operation of enumearation
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
RESULT USBHC_SCM_SetUnifyingReport(U8_T devinx,U8_T td_id,U8_T mode)
{
	RESULT	Result=USB_ERROR;
	U8_T	*buf;
	
	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		if (mode == UNIFYING_RPT_MODE_DJ)
		{
			buf = UNIFYING_Switch_DJ_Command;
		}
		else
		{
			buf = UNIFYING_Switch_HID_Command;
		}
		USBHC_Classreq_SetReport(devinx,td_id,0x02,0x20,0x02,15,buf);
		Result=USB_BUSY;
	}
	else
	{
		if ((USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE) || (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED))
		{
			Result=USB_SUCCESS;
			USB_PDevice[devinx].DevAttr &= ~UNIFYING_CHANGE_MODE_MASK;
		}
		USB_PDevice[devinx].Hc.Control.Buf = NULL; // clear the buffer pointer
	}
	return Result;
}

/*----------------------------------------------------------------------------
 * RESULT USBHC_SCM_SetUnifyingHID(U8_T devinx,U8_T td_id,U8_T mode)
 * Purpose : Perform the set address operation of enumearation
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
RESULT USBHC_SCM_SetUnifyingHID(U8_T devinx,U8_T td_id,U8_T mode)
{
	RESULT	Result=USB_ERROR;
	U8_T	*buf;
	
	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		if (mode == UNIFYING_RPT_MODE_LONG)
		{
			buf = UNIFYING_Switch_HIDPP_Command;
		}
		else
		{
			buf = UNIFYING_Switch_HID0_Command;
		}
		buf[3] |= (USBDC_Device[devinx].Unifying_index & 0xf0) >> 4; //software id
		buf[1] = USBDC_Device[devinx].Unifying_index & 0x0f; //index
		USBHC_Classreq_SetReport(devinx,td_id,0x02,0x10,0x02,7,buf);
		Result=USB_BUSY;
	}
	else
	{
		if ((USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE) || (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED))
		{
			Result=USB_SUCCESS;	 
			USB_PDevice[devinx].DevAttr &= ~UNIFYING_CHANGE_HID_MASK;
		}
		USB_PDevice[devinx].Hc.Control.Buf = NULL; // clear the buffer pointer
	}
	return Result;
}
/*----------------------------------------------------------------------------
 * CTRL_State USBHC_SCM_SetProtocol(U8_T devinx,U8_T td_id,U8_T inf,U8_T mode)
 * Purpose : Perform the set address operation of enumearation
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
RESULT USBHC_SCM_SetProtocol(U8_T devinx,U8_T td_id,U8_T inf_id,U8_T mode)
{
	RESULT	Result=USB_ERROR;

	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		USBHC_Classreq_SetProtocol(devinx,td_id,inf_id,mode);
		Result=USB_BUSY;
	}
	else
	{
		if ((USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE) || (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED))
		{
			Result=USB_SUCCESS;
		}
	}
	return Result;
}

/* End of usbhc_hid.c */


