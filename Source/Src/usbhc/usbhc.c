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
 * Module Name : usbhc.c
 * Purpose     : The USB host controller with 4 port root hub module driver. 
 *               It manages the TD buffer and handles the ISR. 
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 * -------------------------------------------------------------------------------
 *               Standard Request(bRequest)
 *               [v].Get Status(0x00)
 *               [v].Clear Feature(0x01)
 *               [v].Set Feature(0x03)
 *               [v].Set Address(0x05)               wValueH
 *               [v].Get Descriptor(0x06)----------> [v].Device(0x01)
 *               [v].Set Descriptor(0x07)            [v].Config(0x02)
 *               [v].Get Config(0x08)                [v].String(0x03)
 *               [v].Set Config(0x09)                [v].HID(0x021)  
 *               [v].Get Interface(0x0A)             [v].Report(0x22)
 *               [v].Set Interface(0x0B)             [ ].Physical(0x23)
 *               [ ].Sync Frame
 *               [v].Stall
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	<string.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	"project_include.h"

/* STATIC VARIABLE DECLARATIONS */ 
const U8_T					MOUNT_PORT[] = {UDC_PORT0_ACT,UDC_PORT1_ACT,UDC_PORT2_ACT,UDC_PORT3_ACT};
const U8_T				    MicrosoftString[] = {0x14,0x03,0x4D,0x00,0x69,0x00,0x63,0x00,0x72,0x00,0x6F,0x00,0x73,0x00,0x6F,0x00,0x66,0x00,0x74,0x00};
//---------------------- I M P O R T A N T -------------------------------------------------
HCTD_Table_Typedef			HCTD_Table _at_ 0x00000000; //This is for Hardware requirement
//------------------------------------------------------------------------------------------
USBHC_TD_Channel_TypeDef	HCTD_Channel_Table[USB_HC_ISTL_MAX+USB_HC_INTL_MAX+USB_HC_ATL_MAX]; // Store the device index
USB_PDevice_TypeDef			USB_PDevice[USB_HC_MAX_DEVICE+USB_HC_MAX_HUB];
USBHC_IntTransfer_TypeDef	HC_IntTransfer_Table[USB_HC_INTL_MAX];
USBHC_IsoTransfer_TypeDef	HC_IsoTransfer_Table[USB_HC_ISTL_MAX];

bit		TASK_USBHC_KVM_Switch_Check_Start_Flag;
U8_T	TASK_USBHC_Process_ID;
U8_T	TASK_USBHC_Enumerate_Handle_ID;
U8_T	TASK_USBHC_Scm_Handle_ID;
U8_T	TASK_USBHC_Scm_Handle_Start_ID;
U8_T	USBHC_Enumerate_New_Addr;
U8_T	USBHC_String_Idx_Fix;
U8_T	TASK_USBHC_KVM_Switch_Check_ID;
U8_T	TASK_USBHC_Resume_Command_ID;
U8_T	USBHC_KVM_Switch_Check_Devinx;
U8_T	TASK_USBHC_PassThrough_Handle_Start_ID;
U8_T	TASK_USBHC_PassThrough_Handle_ID;
U8_T    TASK_USBHC_Audio_DC_Mount_Handle_ID;
U8_T 	USB_PDevice_Reset_Tab[USB_HC_MAX_DEVICE+USB_HC_MAX_HUB];
U8_T 	USBHC_PDevice_ResetIndex;
U8_T    Hid_Data[64];
#ifdef SYNC
U8_T 	USBHC_PDevice_SyncInit[USB_HC_MAX_DEVICE];
#endif /* #ifdef SYNC */
idata  U16_T	MSC_Class_Page_Size;

/* LOCAL SUBPROGRAM DECLARATIONS */
void TASK_USBHC_KVM_Switch_Check(void);
RESULT USBHC_SetAddress(U8_T devinx,U8_T td_id,U8_T emustate);
RESULT USBHC_GetDevDesc(U8_T devinx,U8_T td_id,U8_T desc_len);
RESULT USBHC_GetCfgDesc(U8_T devinx,U8_T td_id,U16_T desc_len,U8_T emustate);
void USBHC_Memory_Fail_Msg(U8_T msgindex);
RESULT USBHC_SetConfiguration(U8_T devinx,U8_T td_id,U8_T emustate);
RESULT USBHC_SetDeviceFeature(U8_T devinx,U8_T td_id,U8_T feature,U8_T emustate);
RESULT USBHC_GetStringDesc(U8_T devinx,U8_T td_id,U8_T stringidx,U8_T desc_len,U8_T emu_process_id);
void TASK_USBHC_Scm_Handle_Start(void);
void TASK_USBHC_Resume_Command(void);
void TASK_USBHC_PassThrough_Handle_Start(void);
void TASK_USBHC_PassThrough_Handle(void);
RESULT USBHC_SCM_SetInterface(U8_T devinx,U8_T td_id,U8_T configuration);
RESULT USBHC_SCM_SetIdle(U8_T devinx,U8_T td_id,U8_T intf_id);
void USBHC_Set_Dev_Idle(U8_T devinx,U8_T intf);
RESULT USBHC_SCM_SetHubFeature(U8_T devinx,U8_T td_id,U8_T para2);
RESULT USBHC_SCM_ClearHubFeature(U8_T devinx,U8_T td_id,U8_T para2);
void USBHC_Enumerate_Condition_Release(U8_T devinx);
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
void TASK_USBHC_Audio_DC_Mount_Handle(void);
#endif
void USBHC_INTL_SkipMap_Control(U8_T devinx,U8_T value);
static void USBHC_Channel_Control_Attribute(U8_T devinx,U8_T td_id);
/* EXTERNAL SUBPROGRAM DECLARATIONS */
extern RESULT	USBHC_SCM_SetLedReport(U8_T devinx,U8_T td_id);
extern RESULT	USBHC_SCM_SetProtocol(U8_T devinx,U8_T td_id,U8_T inf,U8_T mode);
RESULT			USBHC_SCM_SetConfiguration(U8_T devinx,U8_T td_id,U8_T configuration);

/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
/* LOCAL SUBPROGRAM BODIES */

/*
 * ----------------------------------------------------------------------------
 * void USBHC_Init(void)
 * Purpose : 1.Init Memory
 *           2.Assign TD buffer address into register(0x31)
 *           3.Assign different TD buffer information into register
 *             a.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void USBHC_Init(void)
{
	//1.Memory Initial operation
	memset((U8_T *)&HCTD_Table,0x00,sizeof(HCTD_Table)); /* TD Table for HC hardware */
	memset((U8_T *)USB_PDevice,0x00,sizeof(USB_PDevice)); /* USB physical device table */
	memset((U8_T *)HC_IntTransfer_Table	,0x00,sizeof(HC_IntTransfer_Table));  /* TD interrupt Transfer Table */
	memset(HCTD_Channel_Table,0x00,sizeof(HCTD_Channel_Table)); /* Logical TD Channel Table */
	//2.Do the HC core hard Init
	USBHC_CORE_Init();
	
	TASK_USBHC_Process_ID = TASK_Create(TASK_USBHC_Process);
	TASK_USBHC_Enumerate_Handle_ID = TASK_Create(TASK_USBHC_Enumerate_Handle);
	TASK_USBHC_PassThrough_Handle_Start_ID = TASK_Create(TASK_USBHC_PassThrough_Handle_Start);
	TASK_USBHC_PassThrough_Handle_ID = TASK_Create(TASK_USBHC_PassThrough_Handle);
	TASK_USBHC_Scm_Handle_ID = TASK_Create(TASK_USBHC_Scm_Handle);
	TASK_USBHC_Scm_Handle_Start_ID = TASK_Create(TASK_USBHC_Scm_Handle_Start);
	TASK_USBHC_KVM_Switch_Check_ID = TASK_Create(TASK_USBHC_KVM_Switch_Check);
	TASK_USBHC_Resume_Command_ID = TASK_Create(TASK_USBHC_Resume_Command);
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	TASK_USBHC_Audio_DC_Mount_Handle_ID = TASK_Create(TASK_USBHC_Audio_DC_Mount_Handle);
#endif
	TASK_USBHC_KVM_Switch_Check_Start_Flag = 0;
	
	HID_Class_Enum_Init(); //HID Class Supported
	USBHC_KVM_Switch_Check_Devinx = 0;
	USBHC_HUB_Class_Enum_Init();
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	USBHC_AUDIO_Init();
#endif //#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)

#if (SYSTEM_MSC_DEVICE_SUPPORT)
	USBHC_MSC_Init();
#endif //#if (SYSTEM_MSC_DEVICE_SUPPORT)

#if (SYSTEM_EXTENDER_MSC_SUPPORT)
	Extender_MSC_Init(0);
#endif /*#if (SYSTEM_EXTENDER_MSC_SUPPORT) */

	USBHC_HidParser_Init();

#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("TASK_USBHC_Process_ID=%bu\n\r",TASK_USBHC_Process_ID);
	printf("TASK_USBHC_Enumerate_Handle_ID=%bu\n\r",TASK_USBHC_Enumerate_Handle_ID);
	printf("TASK_USBHC_PassThrough_Handle_Start_ID=%bu\n\r",TASK_USBHC_PassThrough_Handle_Start_ID);
	printf("TASK_USBHC_PassThrough_Handle_ID=%bu\n\r",TASK_USBHC_PassThrough_Handle_ID);
	printf("TASK_USBHC_Scm_Handle_ID=%bu\n\r",TASK_USBHC_Scm_Handle_ID);
	printf("TASK_USBHC_Scm_Handle_Start_ID=%bu\n\r",TASK_USBHC_Scm_Handle_Start_ID);
	printf("TASK_USBHC_KVM_Switch_Check_ID=%bu\n\r",TASK_USBHC_KVM_Switch_Check_ID);
	printf("TASK_USBHC_Resume_Command_ID=%bu\n\r",TASK_USBHC_Resume_Command_ID);
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	printf("TASK_USBHC_Audio_DC_Mount_Handle_ID=%bu\n\r",TASK_USBHC_Audio_DC_Mount_Handle_ID);
#endif
#endif	
} /* End of USBHC_Init */

/**
* @brief  TASK_USBHC_Process
*         USB Host core main state machine process
* @param  None 
* @retval None
*/
void TASK_USBHC_Process(void)
{
	U8_T	devinx = TASK_Register0;

	//Skip task if the device is gone
	if ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00)  // Device is gone  
	{
		return;
	}

TASK_USBHC_Process_Restart:
	switch (USB_PDevice[devinx].Hc.gState)
	{
		case HOST_DEV_RESET: // Root hub port reset:Hub Port Status Changed
			//Check Port enable & connect
			break;
		case HOST_DEV_RESET_WAIT: // Root hub port reset:Hub Port Status Changed
			//Check Port enable & connect
			break;
		case HOST_DEV_ATTACHED: // Device attached in root hub downstream port
			// Next step go for enumeration.
			USB_PDevice[devinx].Hc.gState = HOST_ENUMERATION; // start do Enumeration operation
			//USB_PDevice[devinx].Hc.EnumState = ENUM_SET_ADDR; // Do set addressing
			USB_PDevice[devinx].Hc.EnumState = ENUM_START; 
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;

		case HOST_ENUMERATION:
			if (USB_PDevice[devinx].Hc.EnumState == ENUM_IDLE) // Do set addressing
			{
				if ( USB_PDevice[devinx].Hc.Control.State == CTRL_FAIL)
				{
					printf("@@ENU@@ ERROR:Devinx:%bd,Skip Next Class Enumerate\n\r", devinx);
					USBHC_RootHub_Bus_Occupy_Flag = 0; //unlock the root hub connection enumeration
					USBHC_HUB_ReleaseEnumerateOperation();
					USB_PDevice[devinx].Hc.EnumState = HID_ENUM_IDLE;
					USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
					USBHC_Remove_Device(devinx, 0); //remove the device
				}
				else
				{
					USB_PDevice[devinx].Hc.gState = HOST_HID_CLASS_ENUMERATION;
					goto TASK_USBHC_Process_Restart;
				}
			}
			else
			{
				TASK_Active(TASK_TYPE_USB,TASK_USBHC_Enumerate_Handle_ID,0,devinx,0,0); // Generate the task
			}
			break;
		case HOST_HID_CLASS_ENUMERATION: // if any interface has claim the hid class, then get the report content now.
			USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_SET_IDLE;
			TASK_Active(TASK_TYPE_USB,TASK_USBHC_HID_Class_Enumerate_ID,0,devinx,0x00,0);  // Generate the task
			break;
		case HOST_CLASS_ENUMERATION:
			switch (USB_PDevice[devinx].DevClass)
			{
				case USB_HID_CLASS:
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)						
				case USB_AUDIO_CLASS:
#endif /* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */											
#if (SYSTEM_MSC_DEVICE_SUPPORT)
				case USB_MSC_CLASS:
#endif //#if (SYSTEM_MSC_DEVICE_SUPPORT)
					USB_PDevice[devinx].Hc.gState = HOST_ENUMLATE_TERMINATED;
					goto TASK_USBHC_Process_Restart;
				case USB_HUB_CLASS:					
					USB_PDevice[devinx].Hc.HUB_EnumState = HUB_ENUM_GET_DESC;
					USB_PDevice[devinx].HUB.HC_qt_Bk = HOST_CLASS_ENUMERATION;
					USB_Hub_Class_Enumerate_Fork(devinx);
					break;
				default:					
					/* Class not support */
					USBHC_Enumerate_Condition_Release(devinx);
					USB_PDevice[devinx].Hc.gState = HOST_CLASS_REQUEST;
					//printf("> Class(%02bx) not support\n\r", USB_PDevice[devinx].DevClass);
					break;
			}
			break;
		case HOST_ENUMLATE_TERMINATED:
			USB_PDevice[devinx].Hc.gState = HOST_CLASS_REQUEST; // Class maintain..........
			switch(USB_PDevice[devinx].DevClass)
			{
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)				
 				case USB_AUDIO_CLASS:	
#if (SYSTEM_EXTENDER_TRANSMITTER)
					if ((USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK) == 0x00) //if not extender remote device
					{
						//printf("Transmitter: Not support local AUDIO class device(%bu)\n\r",devinx);
						USBHC_Enumerate_Condition_Release(devinx);
						break;
					}	
#endif 	/* #if (SYSTEM_EXTENDER_TRANSMITTER) */						

#ifdef DEVICE_GPIO_DEFINE
					if (Audio_Device_Support_Gpio != DEVCIE_AUDIO_SUPPORT)
					{
						//printf("Audio Class not support(%bu)\n\r",devinx);
						USBHC_Enumerate_Condition_Release(devinx);
						break;
					}	
#endif				
					USBHC_Audio_Device_Cnt++;
					if (USBHC_Audio_Device_Cnt > SYSTEM_AUDIO_MAX)
					{
						//Error Message to Buffer
#if (KVM_BUZZER_SUPPORT)
						BUZZER_Script_Active(DeviceErrorSound);
#endif //#if (KVM_BUZZER_SUPPORT)
#if (SYSTEM_AUDIO_MAX)
						//printf("Too many audio device, skip mount\n\r");
#endif /* #if (SYSTEM_AUDIO_MAX) */
						USBHC_Enumerate_Condition_Release(devinx);
					}
					else
					{
#if (SYSTEM_EXTENDER_TRANSMITTER)						
						KVM_CurrentUSBAudio = KVM_CurrentHost;
#endif						
					    //if ((KVM_Flash.cSystemFlag0 & SYSTEM_AUDIO_REPORT_ALL_MASK) == 0x00)
					    {
#if (SYSTEM_EXTENDER_RECEIVER)							
					    	KVM_CurrentUSBAudio = KVM_CurrentHost;					    	
#endif							

#if (SYSTEM_KVM)
							if (USBHC_Audio_Hotkey_Sw_Flag)
							{	
								//printf("   USB Audio Hotkey Switchn\n\r");
								//USBHC_Audio_Switch_In_Processing_Flag = 0;
								USBHC_Audio_Hotkey_Sw_Flag = 0;
								KVM_CurrentUSBAudio = KVM_NextAudio;
							}
							else
							{	
								KVM_CurrentUSBAudio = KVM_CurrentHost;
							}	
#endif
						    USBHC_Active_New_Device(devinx,MOUNT_PORT[KVM_CurrentUSBAudio]);
						}						
						
#if (SYSTEM_EXTENDER_RECEIVER)
						ETDR_AudioOutHead = 0;
						ETDR_AudioOutTail = 0;
						ETDR_AudioOutHcDoingFlag = 0;
						ETDR_AudioInWaitFlag = 0;
						ETDR_AudioInHead = 0;
						ETDR_AudioInTail = 0;
						ETDR_AudioInTailSkipFlag = 0;
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */
					}
					break;
#endif //#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)					
				case USB_HUB_CLASS:
					USBHC_Active_New_Device(devinx,UDC_PORT_ALL_ACT);
					break;
				case USB_HID_CLASS:
					USB_PDevice[devinx].Hc.HID_EnumState = HID_ENUM_IDLE; // Enumeration Ternmination
					USBHC_Active_New_Device(devinx,UDC_PORT_ALL_ACT);
					if (USB_PDevice[devinx].DevAttr & DEVATTR_KB_CLASS_MASK) // if the this device contain the keyboard interface
					{
						if (USBHC_KB_Led_Check(devinx))
						{	
							USB_PDevice[devinx].Hc.gState   = HOST_SCM; //go to set the Set Report command
							USB_PDevice[devinx].Hc.ScmdStep = SCMD_SET_LED_REPORT;
							goto TASK_USBHC_Process_Restart; // for last command
						}	
					}
					break;
#if (SYSTEM_MSC_DEVICE_SUPPORT)
				case USB_MSC_CLASS:
#if (SYSTEM_EXTENDER_TRANSMITTER)		
					if ((USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK) == 0x00) //if not extender remote device
					{
						//printf("Transmitter: Not support local Mass Storage Class device(%bu)\n\r",devinx);
						USBHC_Enumerate_Condition_Release(devinx);
						break;
					}	
#endif 	/* #if (SYSTEM_EXTENDER_TRANSMITTER) */		

#ifdef DEVICE_GPIO_DEFINE
					if (MSC_Device_Support_Gpio != DEVCIE_MSC_SUPPORT)
					{
						//printf("Storage Class not support(%bu)\n\r",devinx);
						USBHC_Enumerate_Condition_Release(devinx);
						break;
					}	
#endif							
					USBHC_MSC_Device_Cnt++; // How may msc device has been attached					
					//if (USBHC_MSC_Device_Cnt > SYSTEM_MSC_MAX)
					if (USBHC_MSC_Devinx != 0xff)
					{
						//Error Message to Buffer
#if (KVM_BUZZER_SUPPORT)
						BUZZER_Script_Active(DeviceErrorSound);
#endif //#if (KVM_BUZZER_SUPPORT)
#if (SYSTEM_MSC_MAX)
						//printf("Too many msc device, skip mount\n\r");
#else
						//printf("Not support msc device, skip mount\n\r");
#endif
						USBHC_Enumerate_Condition_Release(devinx);
						break;
					}
					else
					{
						USB_PDevice[devinx].Hc.gState = HOST_CLASS_REQUEST;
						if (USBHC_MSC_Class_Verify(devinx) != USB_SUCCESS) //Check the protocol MSC used
						{
							//printf("> KVM: MSC class type device not supported\n\r");
							//Error Message to Buffer
#if (KVM_BUZZER_SUPPORT)
							BUZZER_Script_Active(MSC_Switch_Error);
#endif //#if (KVM_BUZZER_SUPPORT)
							USBHC_Enumerate_Condition_Release(devinx);
							break;
						}
						
						//Start handling MSC class device
						USBHC_MSC_Devinx = devinx;
												
						//There are 2 condition will caused the MSC device remount
						//1.MSC Switch
						//2.MSC Reset by DC port
						//Handle the Condition 1
						if (KVM_CurrentMSC_Next & BIT7)
						{						
							KVM_CurrentMSC_Next &= (~BIT7);
							KVM_CurrentMSC = KVM_CurrentMSC_Next;
						}
						else						
						{
#if (SYSTEM_EXTENDER_TRANSMITTER)								
							KVM_CurrentMSC = KVM_CurrentHost;
#endif

#if (SYSTEM_EXTENDER_RECEIVER)								
							KVM_CurrentMSC = REMOTE_HOST_PORT;
#endif						

#if (SYSTEM_KVM)
							KVM_CurrentMSC = KVM_CurrentHost;;
#endif								
						}
						
						//Handle the Condition 2
						if (USBHC_MSC_Reset_Flag)
						{						
							KVM_CurrentMSC = USBHC_MSC_Reset_DCPort;
						}
														
						USBHC_MSC_TD_Assign_Devinx(devinx);
						USB_PDevice[devinx].MSC = (USBHC_MSC_Device_TypeDef *) m_malloc(sizeof(USBHC_MSC_Device_TypeDef),5);
						USB_PDevice[devinx].MSC->BOT = (MSC_BOT_Control_TypeDefine *) m_malloc(sizeof(MSC_BOT_Control_TypeDefine),4);
					}
					USBHC_MSC_Bulk_State_Reset(devinx);
					USBHC_Active_New_Device(devinx,MOUNT_PORT[KVM_CurrentMSC]);
					break;
#endif //#if (SYSTEM_MSC_DEVICE_SUPPORT)
				default:					
					break;
			}
			break;
		case HOST_SCM:
			if (USB_PDevice[devinx].Hc.ScmdState == SCMD_IDLE) //The first Start
			{
				if ((USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK) == 0x00)	//if extender, then 
				{	
					TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_Scm_Handle_Start_ID,0,devinx,0,0); //Generate the task, for next Event check
				}
				else
				{
					USB_PDevice[devinx].Hc.gState = HOST_CLASS_REQUEST; // Class maintain..........
					USB_PDevice[devinx].Hc.ScmdState = SCMD_IDLE;
					break;
				}		
			}
			else
			{
				if ((USB_PDevice[devinx].Hc.ScmdState == SCMD_FAIL) || //The first Start
					(USB_PDevice[devinx].Hc.ScmdState == SCMD_COMPLETE))
				{
					USB_PDevice[devinx].Hc.gState = HOST_CLASS_REQUEST; // Class maintain..........
					USB_PDevice[devinx].Hc.ScmdState = SCMD_IDLE;
					goto TASK_USBHC_Process_Restart;
				}
			}
			break;
		case HOST_CLASS_REQUEST:
			break;
		default:
			break;
	}
}

/**
* @brief  USBHC_Control_TD_Init
*          
* @param  None 
* @retval None
*/
USBH_Status USBHC_Control_TD_Init(U8_T devinx,U8_T *rtn_tdid)
{
	U8_T td_id;
	
	td_id = USBHC_CORE_Alloc_TD(); //Get a free ATL TD
	if (td_id == 0)
	{
		//printf("Devinx:%bd,No ATL Free TD can used\n\r",devinx);
		return USBH_FAIL;
	}
	td_id--;

	memset(&HCTD_Channel_Table[td_id], 0x00, sizeof(USBHC_TD_Channel_TypeDef));
	HCTD_Channel_Table[td_id].Used_DeviceID = USBHC_TD_USED_MASK;
	USBHC_Channel_Control_Attribute(devinx,td_id);
	USB_PDevice[devinx].Hc.Control.TdNum = td_id;
	*rtn_tdid = td_id;
	return USBH_OK;
}

/**
* @brief  TASK_USBHC_Enumerate_Handle
*         USB Host core main state machine process
* @param  None 
* @retval None
*/
void TASK_USBHC_Enumerate_Handle(void)
{
	U8_T	devinx,td_id;
	RESULT	Result=USB_ERROR;
	U8_T	hubdevinx;
	USBHC_DevDesc_TypeDef	*devsc;

	devinx = TASK_Register0;
	td_id = USB_PDevice[devinx].Hc.Control.TdNum; // which host channel is used by device

	//Skip task if the device is gone
	if ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00)  // Device is gone  
	{
		return;
	}
	
TASK_USBHC_Enumerate_Handle_Restart:
	switch (USB_PDevice[devinx].Hc.EnumState)
	{
		case ENUM_START:
			if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
			{
				if (USBHC_Control_TD_Init(devinx,&td_id) == USBH_OK)
				{
					USB_PDevice[devinx].Hc.EnumState = ENUM_GET_DEV_DESC0;
					//USB_PDevice[devinx].Hc.EnumState = ENUM_SET_ADDR;
					//USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
					Result = USB_SUCCESS;
				}		
				else
				{
					USB_PDevice[devinx].Hc.EnumState = ENUM_FAIL;
				}				
			}		
			break;
		case ENUM_SET_ADDR: //Start to Get device description
			//Result = USBHC_SetAddress(devinx,td_id,ENUM_GET_CFG_DESC);
			Result = USBHC_SetAddress(devinx,td_id,ENUM_GET_DEV_DESC);
			break;
		case ENUM_GET_DEV_DESC0:			
		case ENUM_GET_DEV_DESC:
			Result = USBHC_GetDevDesc(devinx,td_id,0x12);
			break;
		case ENUM_GET_CFG_DESC:
			Result = USBHC_GetCfgDesc(devinx,td_id,0x12,ENUM_GET_FULL_CFG_DESC);
			break;
		case ENUM_GET_FULL_CFG_DESC:
			Result = USBHC_GetCfgDesc(devinx,td_id,USB_PDevice[devinx].CfgTotalLen,ENUM_GET_LANGID_STRING);
			break;
		case ENUM_GET_LANGID_STRING:
			devsc = (USBHC_DevDesc_TypeDef *)(USB_PDevice[devinx].Desc[DEVICE_DESC].Ptr);
			if ((devsc->iManufacturer == 0) && (devsc->iProduct == 0))
			{				
				USB_PDevice[devinx].Desc[LANG_ID].Len = 4; //no language id length
				USB_PDevice[devinx].Desc[LANG_ID].Ptr = m_malloc(4,6);
				USB_PDevice[devinx].Desc[LANG_ID].Ptr[0] = 0x04;
				USB_PDevice[devinx].Desc[LANG_ID].Ptr[1] = 0x03;
				USB_PDevice[devinx].Desc[LANG_ID].Ptr[2] = 0x09;
				USB_PDevice[devinx].Desc[LANG_ID].Ptr[3] = 0x04;
				USB_PDevice[devinx].Hc.EnumState = ENUM_GET_SERIALNUM_STRING;
				Result = USB_SUCCESS;
			}
			else
			{					
				Result = USBHC_GetStringDesc(devinx,td_id,0x00,0,ENUM_GET_FULL_LANGID_STRING); // langid
			}				
			break; 
		case ENUM_GET_FULL_LANGID_STRING:
			Result = USBHC_GetStringDesc(devinx,td_id,0x00,USB_PDevice[devinx].Desc[LANG_ID].Len,ENUM_GET_SERIALNUM_STRING); // langid
			break;
		case ENUM_GET_SERIALNUM_STRING:
			Result = USBHC_GetStringDesc(devinx,td_id,0x03,0,ENUM_GET_FULL_SERIALNUM_STRING); // Serial string
			break;
		case ENUM_GET_FULL_SERIALNUM_STRING:
			if (USB_PDevice[devinx].idVendor == 0x045E)  //Microsoft
			{
				USB_PDevice[devinx].Desc[VENDOR_ID].Len = 20;
				USB_PDevice[devinx].Desc[VENDOR_ID].Ptr = MicrosoftString;
				Result = USBHC_GetStringDesc(devinx,td_id,0x03,USB_PDevice[devinx].Desc[SERIAL_ID].Len,ENUM_GET_PRODUCT_STRING);//Serial string				
			}
			else
			{					
			    Result = USBHC_GetStringDesc(devinx,td_id,0x03,USB_PDevice[devinx].Desc[SERIAL_ID].Len,ENUM_GET_VENDOR_STRING);//Serial string
			}	
			break;
		case ENUM_GET_VENDOR_STRING:
			Result = USBHC_GetStringDesc(devinx,td_id,0x01,0,ENUM_GET_FULL_VENDOR_STRING); // Manufacturer string
			break;
		case ENUM_GET_FULL_VENDOR_STRING:
			Result = USBHC_GetStringDesc(devinx,td_id,0x01,USB_PDevice[devinx].Desc[VENDOR_ID].Len,ENUM_GET_PRODUCT_STRING);//Manufacturer string
			break; 
		case ENUM_GET_PRODUCT_STRING:
			Result = USBHC_GetStringDesc(devinx,td_id,0x02,0,ENUM_GET_FULL_PRODUCT_STRING); // Product string
			break;
		case ENUM_GET_FULL_PRODUCT_STRING:
			Result = USBHC_GetStringDesc(devinx,td_id,0x02,USB_PDevice[devinx].Desc[PRODUCT_ID].Len,ENUM_SET_CONFIGURATION); // Product string
			break;
		case ENUM_GET_MSOS_STRING:
//			Result = USBHC_GetStringDesc(devinx,td_id,0xee,0xff,ENUM_GET_FULL_MSOS_STRING);
			Result = USBHC_GetStringDesc(devinx,td_id,0xee,0,ENUM_GET_FULL_MSOS_STRING);
			break;
		case ENUM_GET_FULL_MSOS_STRING:
			Result = USBHC_GetStringDesc(devinx,td_id,0xee,USB_PDevice[devinx].Desc[MICROSOFT_OS].Len,ENUM_SET_CONFIGURATION); // Product string
			break;
		case ENUM_SET_CONFIGURATION:
#if (ENUMERATE_VENDOR_TEST)             
			Result = USBHC_SetConfiguration(devinx,td_id,ENUM_VENDOR_IN_TEST);
#else
			Result = USBHC_SetConfiguration(devinx,td_id,ENUM_SET_REMOTEWAKEUP);
#endif
			break;
#if (ENUMERATE_VENDOR_TEST)
		case ENUM_VENDOR_IN_TEST:
			Result = USBHC_GetVendorTest(devinx,td_id,ENUM_SET_REMOTEWAKEUP);
			break;
#endif
		case ENUM_SET_REMOTEWAKEUP:
			if (USB_PDevice[devinx].DevClass == USB_AUDIO_CLASS || USB_PDevice[devinx].DevClass == USB_MSC_CLASS) // => Fixed the 256 MB USB Drive issue.
			{
				USB_PDevice[devinx].Hc.EnumState = ENUM_IDLE;
				Result = USB_SUCCESS;
			}
			else
			{
				if (USB_PDevice[devinx].Attribute & FEATURE_REMOTE)
					Result = USBHC_SetDeviceFeature(devinx,td_id,DEVICE_REMOTE_WAKEUP,ENUM_IDLE);
				else
				{
					USB_PDevice[devinx].Hc.EnumState = ENUM_IDLE;
					Result = USB_SUCCESS;
				}
			}
			break;
		default:
			return;
	}

	if ((USB_PDevice[devinx].Hc.Control.State == CTRL_FAIL) ||  // the control condition is error, exception processing
		(USB_PDevice[devinx].Hc.EnumState == ENUM_IDLE) ||
		(USB_PDevice[devinx].Hc.EnumState == ENUM_FAIL))
	{
		USBHC_CORE_Free_TD(USB_PDevice[devinx].Hc.Control.TdNum);

		if (USB_PDevice[devinx].Hc.EnumState != ENUM_IDLE)
		{
			//if (USB_PDevice[devinx].Hc.EnumState == ENUM_FAIL)
			//	printf("@@ Terminate=>ENUM_FAIL\n\r");
			//else
			//	printf("@@ Terminate=>CTRL_ERROR\n\r");

			USB_PDevice[devinx].Hc.Control.State = CTRL_FAIL;
			USB_PDevice[devinx].Hc.EnumState = ENUM_IDLE;
		}
		//Handle the Special device Class			
		switch(USB_PDevice[devinx].DevClass)
		{
			//1.HUB class move to another device table zone.						
			case USB_HUB_CLASS:

#if (!SYSTEM_USBAUDIO_DEVICE_SUPPORT) || (!SYSTEM_MSC_DEVICE_SUPPORT) || defined(DEVICE_GPIO_DEFINE) || (SYSTEM_EXTENDER_TRANSMITTER)
DEVICE_MOVE_ANOTHER_ZONE:				
#endif			
				hubdevinx = USBHC_Clone_PDevice(devinx,0);
				if (hubdevinx)
				{
					devinx = hubdevinx;
				}
				else
				{
					USB_PDevice[devinx].Hc.Control.State = CTRL_FAIL;
					USB_PDevice[devinx].Hc.EnumState = ENUM_IDLE;
				}
				break;
			case USB_AUDIO_CLASS:				
#if (SYSTEM_EXTENDER_SUPPORT)				
#if (!SYSTEM_USBAUDIO_DEVICE_SUPPORT) || (SYSTEM_EXTENDER_TRANSMITTER)
				goto DEVICE_MOVE_ANOTHER_ZONE;								
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */			
#endif

#ifdef KVM
#if (!SYSTEM_USBAUDIO_DEVICE_SUPPORT)
				goto DEVICE_MOVE_ANOTHER_ZONE;								
#endif
#endif

#if (!SYSTEM_EXTENDER_TRANSMITTER)			
#ifdef DEVICE_GPIO_DEFINE			
				if (Audio_Device_Support_Gpio == DEVCIE_NO_AUDIO_SUPPORT)
				{	
					goto DEVICE_MOVE_ANOTHER_ZONE;
				}	
#endif /* #ifdef DEVICE_GPIO_DEFINE */
#endif 	/* #if (!SYSTEM_EXTENDER_TRANSMITTER) */			
				break;
			case USB_MSC_CLASS:
#if (SYSTEM_EXTENDER_SUPPORT)								
#if (!SYSTEM_EXTENDER_MSC_SUPPORT) || (SYSTEM_EXTENDER_TRANSMITTER)			
				goto DEVICE_MOVE_ANOTHER_ZONE;								
#endif	/* #if (SYSTEM_EXTENDER_MSC_SUPPORT) */			
#endif

#ifdef KVM
#if (!SYSTEM_MSC_DEVICE_SUPPORT)
				goto DEVICE_MOVE_ANOTHER_ZONE;								
#endif
#endif

#if (!SYSTEM_EXTENDER_TRANSMITTER)
#ifdef DEVICE_GPIO_DEFINE
				if (MSC_Device_Support_Gpio == DEVCIE_NO_MSC_SUPPORT)
				{	
					goto DEVICE_MOVE_ANOTHER_ZONE;
				}	
#endif /* #ifdef DEVICE_GPIO_DEFINE */
#endif 	/* #if (!SYSTEM_EXTENDER_TRANSMITTER) */							
				break;			
		}		

		TASK_Active(TASK_TYPE_USB,TASK_USBHC_Process_ID,0x00,devinx,0,0); // Generate the task
		return;
	}
	
	if (Result == USB_SUCCESS)
	{
		goto TASK_USBHC_Enumerate_Handle_Restart;
	}
}

/**
* @brief  USBHC_Parser_DevDesc 
*         Displays the message on LCD for device descriptor
* @param  DeviceDesc : device descriptor
* @retval None
*/
void USBHC_Parser_DevDesc(U8_T devinx,U8_T *DeviceDesc)
{
	USBHC_DevDesc_TypeDef	*hs;
	U8_T					td_id;
#if (USBHC_PARSER_DEBUG_MODE)
	U8_T index, len;
#endif //#if (USBHC_PARSER_DEBUG_MODE)
	
	hs = (USBHC_DevDesc_TypeDef *)DeviceDesc;
#if (USBHC_PARSER_DEBUG_MODE)
	len = hs->bLength;
#endif
	USB_PDevice[devinx].DevClass = hs->bDeviceClass;
	USB_PDevice[devinx].ControlEndpSize = hs->bMaxPacketSize;
	USB_PDevice[devinx].idVendor = Big_Endian_16_Convert(hs->idVendor);
	USB_PDevice[devinx].idProduct = Big_Endian_16_Convert(hs->idProduct);

	
	//printf("iVendor=%04x,iProduct=%04x\n\r",USB_PDevice[devinx].idVendor,USB_PDevice[devinx].idProduct);
	if (devinx < USBDC_VIRTUAL_DEVINX)
		return;

	/* Special handle for Logitech Unifying Wireless Mouse */
	if ((USB_PDevice[devinx].idVendor == 0x046d) && ((USB_PDevice[devinx].idProduct == 0xc52b) || ((USB_PDevice[devinx].idProduct == 0xc532))))
	{
		USB_PDevice[devinx].DevAttr |= DEVATTR_UNIFYING_MASK;// contain keyboard protocol.
	}
	else if ((USB_PDevice[devinx].idVendor == 0x0408) &&(USB_PDevice[devinx].idProduct == 0x3008)) /* Special handle for Viewsonic TD2220 */
	{
		USB_PDevice[devinx].DevAttr |= DEVATTR_TD2220_MASK;
	}
#if (SYSTEM_EXTENDER_TRANSMITTER)
	if ((USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK) == 0x00) //if not extender remote device
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */
	{
		td_id = USB_PDevice[devinx].Hc.Control.TdNum;
		HCTD_Channel_Table[td_id].Endp_Size = hs->bMaxPacketSize;
	}

	if (Big_Endian_16_Convert(hs->bcdUSB) >= 0x0200) // usb 2.0
	{
		USB_PDevice[devinx].Usb_2_0 = 1; // USB 2.0
	}
	
#if (USBHC_PARSER_DEBUG_MODE)
	if (devinx > 1)
	{
		printf("Device Descriptor :[");
		for (index=0; index < len ; index++)
		{
			if (index < (len-1))
				printf("%02x ",(U16_T)DeviceDesc[index]);
			else
				printf("%02x]\n\r",(U16_T)DeviceDesc[index]);
		}
		printf(" Class : %02x\n\r" ,(U16_T)(hs->bDeviceClass));
		printf(" EndpSize : %d\n\r" ,(U16_T)(hs->bMaxPacketSize));
		printf(" VID : %04Xh\n\r" , (U16_T)(hs->idVendor));
		printf(" PID : %04Xh\n\r" , (U16_T)(hs->idProduct));
	}
#endif //#if (USBHC_PARSER_DEBUG_MODE)
}

/**
* @brief  USBHC_CfgDesc_Parser_Len 
*         Displays the message on LCD for device descriptor
* @param  DeviceDesc : device descriptor
* @retval None
*/
void USBHC_CfgDesc_Parser_Len(U8_T devinx,U8_T *CfgDesc)
{
	USBHC_CfgDesc_TypeDef	*hs;

	hs = (USBHC_CfgDesc_TypeDef *)CfgDesc;
	USB_PDevice[devinx].CfgTotalLen = Big_Endian_16_Convert(hs->wTotalLength);
#if (USBHC_PARSER_DEBUG_MODE)
	printf("Configuration Descriptor Total Length=[%d]\n\r",USB_PDevice[devinx].CfgTotalLen);
#endif //#if (USBHC_PARSER_DEBUG_MODE)
}

/**
* @brief  Big_Endian_16_Convert 
*         Displays the message on LCD for device descriptor
* @param  DeviceDesc : device descriptor
* @retval None
*/
U16_T Big_Endian_16_Convert(U16_T big_edin)
{
	return (((big_edin & 0xff00) >> 8) + ((big_edin & 0x00ff) << 8));
}

#if ((SYSTEM_USB_PEN_DRIVE_SUPPORT) || (SYSTEM_MSC_DEVICE_SUPPORT) || (SYSTEM_EXTENDER_MSC_SUPPORT))
/**
* @brief  Big_Endian_32_Convert 
*         Convert the samll_endian U32_T to big endian U32_T
* @param  DeviceDesc : device descriptor
* @retval None
*/
U32_T Endian_32_Convert(U32_T endian)
{
	U32_T convert;
	
	convert = ((endian & 0xff000000) >> 24);
	convert += ((endian & 0x00ff0000) >>  8);
	convert += ((endian & 0x0000ff00) <<  8);
	convert += ((endian & 0x000000ff) << 24);
	
	return convert;
}
#endif /* (SYSTEM_USB_PEN_DRIVE_SUPPORT) || (SYSTEM_MSC_DEVICE_SUPPORT) */

/**
* @brief  USBHC_Parser_ConfigDesc 
*         Displays the message on LCD for device descriptor
* @param  DeviceDesc : device descriptor
* @retval None
*/
U8_T USBHC_Parser_ConfigDesc(U8_T devinx,U16_T total_len,U8_T *buf) 
{
	bit		alternate_endp_flag;		
	U8_T	endpcnt=0, inf_class;
	U8_T	curinf=0xFF, endpinx;	
	U16_T	pos = 0, endpsize;
	USBHC_CfgDesc_TypeDef		*pcfg;
	USBHC_InterfaceDesc_TypeDef	*pif;
	USBHC_EpDesc_TypeDef		*pep;
	USBHC_DescHeader_t			*pdesc;
	USBHC_HIDDesc_TypeDef		*phid;
	
#if (USBHC_PARSER_DEBUG_MODE)
	U16_T	len, index, curlen;
	U16_T	packagecnt;

	if (devinx > 1)
	{
		len = USB_PDevice[devinx].CfgTotalLen;
		printf("Configuration Descriptor(%d):\n\r",(U16_T)len);
		printf("{\n\r");
		endpsize = USB_PDevice[devinx].ControlEndpSize;	
		curlen = 0;
		while (1)
		{
			if (len > endpsize)
				packagecnt = endpsize;
			else
				packagecnt = len;
			printf("    ");
			for (index=0; index < packagecnt ; index++)
			{
				printf("%02bx ", buf[curlen]);
				curlen++;
			}
			printf("\n\r");
			len -= packagecnt;
			if (len==0)
				break;
		}
		printf("}\n\r");
	}
#endif //#if (USBHC_PARSER_DEBUG_MODE)
	while ( 1 )
	{
		pdesc = (USBHC_DescHeader_t *)(buf+pos);				
		switch ( pdesc->bDescriptorType )
		{
			case USB_DESC_TYPE_CONFIGURATION : // 0x02
				pcfg = (USBHC_CfgDesc_TypeDef *)(buf+pos);	
				if (pcfg->bNumInterfaces >= USBHC_MAX_NUM_INTERFACES)
				{
					//printf("Too many interfaces (%bd)\n\r", pcfg->bNumInterfaces);
					pcfg->bNumInterfaces = USBHC_MAX_NUM_INTERFACES;
					return 0;
				}
				USB_PDevice[devinx].InterfaceNum |= pcfg->bNumInterfaces;
				//Assing current active configuration
				if (USB_PDevice[devinx].ConfigurationValue == 0) // if current configuration is not assign, default to 1
				{
					USB_PDevice[devinx].ConfigurationValue = pcfg->bConfigurationValue;
				}
				//Check the total configuration value
				if (pcfg->bConfigurationValue > USB_PDevice[devinx].TotalConfiguration)
					USB_PDevice[devinx].TotalConfiguration = pcfg->bConfigurationValue;
				if (pcfg->bMaxPower > 0x32) //modify the power
				{
					pcfg->bMaxPower = 0x32;
				}
				memset(USBDC_Device[devinx].Feature,(pcfg->bmAttributes | 0x80),sizeof(USBDC_Device[devinx].Feature));
				USB_PDevice[devinx].Attribute = pcfg->bmAttributes;
				break;

			case USB_DESC_TYPE_INTERFACE: // 0x04
				pif = (USBHC_InterfaceDesc_TypeDef *)(buf+pos);
				if ( pif->bInterfaceNumber >= USBHC_MAX_NUM_INTERFACES )
					return 0;

				if (pif->bAlternateSetting == 0)
				{
					endpcnt = 0;
					curinf++;
				}
				else if (curinf == 0xFF)
				{
					curinf = 0;
				}
				inf_class = pif->bInterfaceClass;
				if (inf_class == 0) // if local class is zero
				{
					USB_PDevice[devinx].InfClass[curinf] = USB_PDevice[devinx].DevClass;
				}
				else
				{
					USB_PDevice[devinx].InfClass[curinf] = inf_class;
					if (USB_PDevice[devinx].DevClass == 0x00)
					{
						USB_PDevice[devinx].DevClass = inf_class;
					}
				}
				
				//need to keep the bigst alternate setting.
				if (pif->bAlternateSetting > USB_PDevice[devinx].InfAltValue[curinf])
				{
					USB_PDevice[devinx].InfAltValue[curinf] = pif->bAlternateSetting;
				}
				
				USB_PDevice[devinx].InfSubClass[curinf] = pif->bInterfaceSubClass;
				USB_PDevice[devinx].InfProtocol[curinf] = pif->bInterfaceProtocol;
				if ( pif->bNumEndpoints > USB_PDevice[devinx].EndpNum[curinf])
				{
					USB_PDevice[devinx].EndpNum[curinf] = pif->bNumEndpoints;
				}

				/*Check if weither contain keyboard protcol*/
				if (pif->bInterfaceProtocol == INF_KEYBOARD)
				{
					USB_PDevice[devinx].DevAttr |= DEVATTR_KB_CLASS_MASK; 
#ifdef SYNC
					if (KVM_Flash.cSystemFlag2 & (SYSTEM_ALL_SYNC_MASK | SYSTEM_KB_SYNC_MASK))
					{	
						USB_PDevice[devinx].HID_Kb_LED_Value[1] = KM_SYNC_KbLed;
					}
					else
#endif					
					{
						USB_PDevice[devinx].HID_Kb_LED_Value[1] = KVM_HostLed[KVM_CurrentHost];
					}	
				}
				break;
			case USB_DESC_TYPE_HID_DESCRIPTOR: // 0x21
				phid = (USBHC_HIDDesc_TypeDef *)(buf+pos);
				USB_PDevice[devinx].HID_Rpt_Len[curinf] = Big_Endian_16_Convert(phid->wItemLength);
				break;
			case USB_DESC_TYPE_ENDPOINT: // 0x05
				if ( endpcnt <= USBHC_MAX_NUM_ENDP )
				{
					pep = (USBHC_EpDesc_TypeDef *)(buf+pos);
					//Search the Endpoint Address that has been in table
					alternate_endp_flag = 0;				
					for (endpinx=0; endpinx < endpcnt; endpinx++)
					{				
						if ((USB_PDevice[devinx].EndpAddr[curinf][endpinx]&0x7F) == (pep->bEndpointAddress&0x7F))
						{
							/* 2015-12-14 Modify add by jack*/
							if (USB_PDevice[devinx].InfAltValue[curinf])
							{	
								alternate_endp_flag = 1;
							}	
							break;
						}
					}
					endpsize =  Big_Endian_16_Convert(pep->wMaxPacketSize);
					//Check the Isochronous max package size, the bigest is 192 bytes.
					if (USB_PDevice[devinx].InfClass[curinf] == USB_AUDIO_CLASS)
					{
						if (USB_PDevice[devinx].DevClass != USB_AUDIO_CLASS)
						{
							USB_PDevice[devinx].DevClass = USB_AUDIO_CLASS;
						}
						
						if (endpsize > (USB_HC_ISTL_BUF_SIZE-2))
						{
							endpsize = (USB_HC_ISTL_BUF_SIZE-2);
							//chnage the interface setting.
							pep->wMaxPacketSize = Big_Endian_16_Convert(endpsize);
						}
					}					

					if (alternate_endp_flag)
					{
						if (pep->bEndpointAddress != USB_PDevice[devinx].EndpAddr[curinf][endpinx])
						{
							//printf("Warring,Alternating Interface:%bx,Endp:%bx Address_old(%bx),New(%bx) conflict\n\r",
							//	pif->bInterfaceNumber, endpinx,	USB_PDevice[devinx].EndpAddr[curinf][endpinx],
							//	pep->bEndpointAddress);
								
							if (USB_PDevice[devinx].DevClass != USB_HID_CLASS) 
							{	
								USB_PDevice[devinx].EndpAddr[curinf][endpinx] = pep->bEndpointAddress;
								USB_PDevice[devinx].EndpSize[curinf][endpinx] = endpsize;
								if (USB_PDevice[devinx].DevClass == USB_HUB_CLASS) 
									USB_PDevice[devinx].EndpInterval[curinf][endpinx] = 0xf0;
								else
									USB_PDevice[devinx].EndpInterval[curinf][endpinx] = pep->bInterval;
								USB_PDevice[devinx].EndpType[curinf][endpinx] = (pep->bmAttributes & 0x03) << 5;
							}	
						}
						else
						{
							if (endpsize > USB_PDevice[devinx].EndpSize[curinf][endpinx])
								USB_PDevice[devinx].EndpSize[curinf][endpinx] = endpsize;
						}
					}
					else
					{						
						USB_PDevice[devinx].EndpAddr[curinf][endpcnt] = pep->bEndpointAddress;
						USB_PDevice[devinx].EndpSize[curinf][endpcnt] = endpsize;
						if (USB_PDevice[devinx].DevClass == USB_HUB_CLASS) 
							USB_PDevice[devinx].EndpInterval[curinf][endpcnt] = 0xff;
						else
						{	
							USB_PDevice[devinx].EndpInterval[curinf][endpcnt] = pep->bInterval;
#if (SYSTEM_EXTENDER_TRANSMITTER)
#ifdef USB_2						
							if (USB_PDevice[devinx].DevClass == USB_HID_CLASS) 
							{										
								pep->bInterval = 10; //10ms
							}	
#endif /* #ifdef (USB_2) */
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */
						}	
						USB_PDevice[devinx].EndpType[curinf][endpcnt] = (pep->bmAttributes & 0x03) << 5;
					}
					if (pif->bInterfaceClass == USB_HUB_CLASS)
					{
						pep->bInterval = 250;
						USB_PDevice[devinx].HUB.ReportEndpInx = pep->bEndpointAddress & ~0x80; //for IN only
					}
				}
				if (alternate_endp_flag == 0)
					endpcnt++;
				break;
		}
		pos += pdesc->bLength;		
		if ( pos >= total_len )
			break;
	}
#if (USBHC_PARSER_DEBUG_MODE)
	if ((devinx >= 1) && (total_len != 0x12))
	{		
		printf("Device Table[%bu] interfaces %bx\n\r",devinx, curinf);
		printf("{\n\r");
		printf("  Addr=0x%bx\n\r",(USB_PDevice[devinx].Addr & USBHC_DEVICE_ADDR_MASK));
		printf("  DevClass=0x%bx\n\r",USB_PDevice[devinx].DevClass);
		printf("  ControlEndpSize=%bu\n\r",USB_PDevice[devinx].ControlEndpSize);
		printf("  CfgTotalLen=%bu\n\r",USB_PDevice[devinx].CfgTotalLen);
		printf("  InterfaceNum=%bu\n\r",(USB_PDevice[devinx].InterfaceNum & USBHC_DEVICE_INTERFACENUM_MASK));
		printf("  ConfigurationValue=%bu\n\r",USB_PDevice[devinx].ConfigurationValue);
		printf("  TotalConfiguration=%bu\n\r",USB_PDevice[devinx].TotalConfiguration);
		for (curinf=0; curinf < (USB_PDevice[devinx].InterfaceNum & USBHC_DEVICE_INTERFACENUM_MASK) ; curinf++)
		{
			printf("    Inf:%bu -------------------------\n\r",curinf);
			printf("    {\n\r");
			printf("      InfClass=0x%02bx\n\r",USB_PDevice[devinx].InfClass[curinf]);
			printf("      InfSubClass=0x%02bx\n\r",USB_PDevice[devinx].InfSubClass[curinf]);
			printf("      Protocol=0x%bx\n\r",USB_PDevice[devinx].InfProtocol[curinf]);
			printf("      Alternate=%bx\n\r",USB_PDevice[devinx].InfAltValue[curinf]);
			printf("      EndpNum=0x%02bx\n\r",USB_PDevice[devinx].EndpNum[curinf]);
			printf("      HID_Rpt_Len=%u\n\r",USB_PDevice[devinx].HID_Rpt_Len[curinf]);
			for (endpcnt=0; endpcnt < USB_PDevice[devinx].EndpNum[curinf] ; endpcnt++)
			{
				printf("      EndPoint:%bu -------------------------\n\r",endpcnt);
				printf("      {\n\r");
				printf("        EndpAddr[%bu]=0x%02bx\n\r",endpcnt,USB_PDevice[devinx].EndpAddr[curinf][endpcnt]);
				printf("        EndpSize[%bu]=0x%02bx\n\r",endpcnt,USB_PDevice[devinx].EndpSize[curinf][endpcnt]);
				printf("        EndpInterval[%bu]=%02bx\n\r",endpcnt,USB_PDevice[devinx].EndpInterval[curinf][endpcnt]);
				printf("        EndpType[%bu]=0x%02bx\n\r",endpcnt,USB_PDevice[devinx].EndpType[curinf][endpcnt]);
				printf("      }\n\r");
			}
			printf("    }\n\r");
		}
	printf("}\n\r");
	}
#endif //#if (USBHC_PARSER_DEBUG_MODE)
	return 1;
}

/** ----------------------------------------------------------------------------
 * void USBHC_Channel_Control_Attribute(void)
 * Purpose : 1.Init Channel by device attribute
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 **/
static void USBHC_Channel_Control_Attribute(U8_T devinx,U8_T td_id)
{
	//1.Update devinx
	HCTD_Channel_Table[td_id].Used_DeviceID |= devinx; // assign this td to this channel
	//2.Update device speed
	HCTD_Channel_Table[td_id].Attri = CHANNEL_ATRI_TYPE_CONTROL | USBHC_EP0_EP_NUM;
	if (USB_PDevice[devinx].InterfaceNum & USBHC_DEVICE_SPEED_MASK) // Low Speed Device
	{
		HCTD_Channel_Table[td_id].Attri |= CHANNEL_ATRI_SPEED_LOW;
	}
	//3.Update devinx
	//4.Update Endpoint Address
	//5.Update Endpoint Size
	HCTD_Channel_Table[td_id].Endp_Size = USB_PDevice[devinx].ControlEndpSize;
}

/*----------------------------------------------------------------------------
 * CTRL_State USBHC_SetAddress(U8_T devinx,U8_T td_id,U8_T emu_process_id))
 * Purpose : Perform the set address operation of enumearation
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
RESULT USBHC_SetAddress(U8_T devinx,U8_T td_id,U8_T emu_process_id)
{
	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		//if (USBHC_Control_TD_Init(devinx,&td_id) == USBH_OK)
		{
			USBHC_Stdreq_SetAddress(devinx,td_id,USBHC_Enumerate_New_Addr);
			return USB_BUSY;
		}
		//else
		//{
		//	USB_PDevice[devinx].Hc.EnumState = ENUM_FAIL;
		//}
	}
	else
	{
		if (USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE)
		{
			USB_PDevice[devinx].Addr = USBHC_Enumerate_New_Addr | USBHC_DEVICE_USED_MASK;
			USB_PDevice[devinx].Hc.EnumState = emu_process_id;
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
			return USB_SUCCESS;
		}
		else
		{
			USB_PDevice[devinx].Hc.EnumState = ENUM_FAIL;
		}
	}
	return USB_ERROR;
}

/*----------------------------------------------------------------------------
 * CTRL_State USBHC_PassThrough_Send_Setup_Token(U8_T devinx,U8_T td_id)
 * Purpose : Perform the set address operation of enumearation
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
RESULT USBHC_PassThrough_Send_Setup_Token(U8_T devinx,U8_T td_id)
{
	RESULT	Result;
	U16_T	data_len;
	
	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		data_len = USB_PDevice[devinx].Hc.PControl.Setup.b.wLength.w;
		/* convert back to normal USB order */
		USBDC_HAL_Convert_Setup_Value((U8_T *)&USB_PDevice[devinx].Hc.PControl.Setup);
		USBHC_Stdreq_PatternCommand(devinx,td_id,(U8_T *)&USB_PDevice[devinx].Hc.PControl.Setup,data_len);
		Result=USB_BUSY;
	}
	else
	{
		if (USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE || (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED))
		{
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
			Result=USB_SUCCESS;
		}
		else
		{
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
			Result=USB_UNSUPPORT;
		}
	}
	return Result;
}

/*----------------------------------------------------------------------------
 * CTRL_State USBHC_GetDevDesc(U8_T devinx,U8_T td_id,U8_T desc_len)
 * Purpose : Perform the set address operation of enumearation
 * Params  : $devinx: USBPD device id
 * 				   $td_id: The HC ATL TD number should be used
 *           $desc_len : the length for device descriptor
 * Returns : none
 * Note    : none
 *----------------------------------------------------------------------------
 */
RESULT USBHC_GetDevDesc(U8_T devinx,U8_T td_id,U8_T desc_len)
{
	RESULT	Result=USB_ERROR;
	
	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{		
		//0.Check the buffer pointer
		if (USB_PDevice[devinx].Hc.EnumState == ENUM_GET_DEV_DESC) //the second time polling
		{ //since this is the second time polling so need to release the first time buffer					
			malloc_free(USB_PDevice[devinx].Desc[DEVICE_DESC].Ptr);
		}			
		//1.Get dynamic buffer memory				
		USB_PDevice[devinx].Hc.Control.Buf = m_malloc(desc_len,7); /* allocate 18 bytes */
		//USB_PDevice[devinx].Hc.Control.Buf = m_malloc(USBHC_MAX_PACKET_SIZE,7);
		HCTD_Channel_Table[td_id].Buf = USB_PDevice[devinx].Hc.Control.Buf; // also assign to host channel buffer
		if (USB_PDevice[devinx].Hc.Control.Buf != NULL)
		{
			//2.Set maximum package size
			USB_PDevice[devinx].Hc.Control.EpN_MaxSize = USBHC_MAX_PACKET_SIZE;
			USBHC_Stdreq_GetDevDesc(devinx,td_id,desc_len);
			Result=USB_BUSY;
		}
		else
		{
			USBHC_Memory_Fail_Msg(0x00);			
			// Terminate the Task .........
			USB_PDevice[devinx].Hc.EnumState = ENUM_FAIL;
		}
	}
	else
	{
		if (USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE || (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED))
		{
			USBHC_Parser_DevDesc(devinx,USB_PDevice[devinx].Hc.Control.Buf);
			//2018-11-16 jack
			//USB_PDevice[devinx].Hc.EnumState = ENUM_GET_CFG_DESC;
			if (USB_PDevice[devinx].Hc.EnumState == ENUM_GET_DEV_DESC)
				USB_PDevice[devinx].Hc.EnumState = ENUM_GET_CFG_DESC;
			else
				USB_PDevice[devinx].Hc.EnumState = ENUM_SET_ADDR;
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;			
			USB_PDevice[devinx].Desc[DEVICE_DESC].Len = desc_len;			
			USB_PDevice[devinx].Desc[DEVICE_DESC].Ptr = USB_PDevice[devinx].Hc.Control.Buf;
			USB_PDevice[devinx].Hc.Control.Buf = 0; //clear the buffer pointer
			// Update Channel information.....
			HCTD_Channel_Table[USB_PDevice[devinx].Hc.Control.TdNum].Endp_Size = USB_PDevice[devinx].ControlEndpSize;
			USB_PDevice[devinx].Hc.Control.EpN_MaxSize = USB_PDevice[devinx].ControlEndpSize;
			Result=USB_SUCCESS;			
		}
		else
		{
			//Due to have to release the buffer, so need put the satement here
			//proces alos if the State is not COMPLETE
			malloc_free(USB_PDevice[devinx].Hc.Control.Buf);
			USB_PDevice[devinx].Hc.Control.Buf = 0;			
			USB_PDevice[devinx].Hc.EnumState = ENUM_FAIL;			
		}
	}
	return Result;
}
 
/*----------------------------------------------------------------------------
 * CTRL_State USBHC_GetCfgDesc(U8_T devinx,U8_T td_id,U8_T desc_len)
 * Purpose : Perform the set address operation of enumearation
 * Params  : $devinx: USBPD device id
 * 				   $td_id: The HC ATL TD number should be used
 *           $desc_len : the length for device descriptor
 * Returns : none
 * Note    : none
 *----------------------------------------------------------------------------
 */
RESULT USBHC_GetCfgDesc(U8_T devinx,U8_T td_id,U16_T desc_len,U8_T emustate)
{
	RESULT Result=USB_ERROR;
	
	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		// get standard configuration descriptor
		USB_PDevice[devinx].Hc.Control.Buf = m_malloc(desc_len,8); //* allocate 9 bytes 
		HCTD_Channel_Table[td_id].Buf = USB_PDevice[devinx].Hc.Control.Buf; // also assign to host channel buffer
		if (USB_PDevice[devinx].Hc.Control.Buf != NULL)
		{
			//2.Set maximum package size
			USBHC_Stdreq_GetCfgDesc(devinx,td_id,desc_len);
			Result=USB_BUSY;
		}
		else
		{
			// Terminate the Task .........
			// should do something
			USBHC_Memory_Fail_Msg(0x01);
			USB_PDevice[devinx].Hc.EnumState = ENUM_FAIL;
		}
	}
	else
	{
		if (USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE || (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED))
		{
			USBHC_CfgDesc_Parser_Len(devinx,USB_PDevice[devinx].Hc.Control.Buf);
			if ((desc_len == 0x12) && (USB_PDevice[devinx].CfgTotalLen != 0x12))
			{
				USBHC_Parser_ConfigDesc(devinx,0x12,USB_PDevice[devinx].Hc.Control.Buf);
				malloc_free(USB_PDevice[devinx].Hc.Control.Buf);
				USB_PDevice[devinx].Hc.Control.Buf = 0;
			}
			else // it is full
			{			
				if (USBHC_Parser_ConfigDesc(devinx,USB_PDevice[devinx].CfgTotalLen,USB_PDevice[devinx].Hc.Control.Buf) == 0) //parser error
				{
					Result = USB_ERROR;
					goto USBHC_GetCfgDesc_Free;
				}		
				USB_PDevice[devinx].Desc[CONFIG_DESC].Len = desc_len;
				USB_PDevice[devinx].Desc[CONFIG_DESC].Ptr = USB_PDevice[devinx].Hc.Control.Buf;
				USB_PDevice[devinx].Hc.Control.Buf = 0;
			}
			USB_PDevice[devinx].Hc.EnumState = emustate;
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
			Result=USB_SUCCESS;
		}
		else
		{
USBHC_GetCfgDesc_Free:
			malloc_free(USB_PDevice[devinx].Hc.Control.Buf);
			USB_PDevice[devinx].Hc.Control.Buf = 0;
			USB_PDevice[devinx].Hc.EnumState = ENUM_FAIL;
		}
	}
	return Result;
}

/*----------------------------------------------------------------------------
 * CTRL_State USBHC_SetConfiguration(U8_T devinx,U8_T td_id,U8_T emustate)
 * Purpose : Perform the set configuration operation of enumearation
 * Params  : $devinx: USBPD device id
 * 				   $td_id: The HC ATL TD number should be used 
 * Returns : none
 * Note    : none
 *----------------------------------------------------------------------------
 */
RESULT USBHC_SetConfiguration(U8_T devinx,U8_T td_id,U8_T emustate)
{
	RESULT	Result;
	
	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		USBHC_Stdreq_SetCfg(devinx,td_id,USB_PDevice[devinx].ConfigurationValue);
		Result = USB_BUSY;
	}
	else
	{
		if (USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE || (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED))
		{
			USB_PDevice[devinx].Hc.EnumState = emustate;
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
			Result = USB_SUCCESS;
		}
		else
		{
			USB_PDevice[devinx].Hc.EnumState = ENUM_FAIL;
			Result = USB_ERROR;
		}
	}	
	
	return Result;
}

#if (ENUMERATE_VENDOR_TEST)
/*----------------------------------------------------------------------------
 * CTRL_State USBHC_GetVendorTest(U8_T devinx,U8_T td_id,U8_T emustate)
 * Purpose : Perform the set configuration operation of enumearation
 * Params  : $devinx: USBPD device id
 * 				   $td_id: The HC ATL TD number should be used 
 * Returns : none
 * Note    : none
 *----------------------------------------------------------------------------*/
RESULT USBHC_GetVendorTest(U8_T devinx,U8_T td_id,U8_T emustate)
{
	RESULT	Result=USB_ERROR;
	
	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		//USBHC_Stdreq_SetCfg(devinx,td_id,USB_PDevice[devinx].ConfigurationValue);
		//1.Fill up Host Channel Table
		USB_PDevice[devinx].Hc.Control.Buf  = m_malloc(16,9); /* allocate 16 bytes */
		HCTD_Channel_Table[td_id].TD_Length = USBHC_SETUP_PKT_SIZE;
		HCTD_Channel_Table[td_id].Total_Length = 16; // no data stage
		HCTD_Channel_Table[td_id].Current_Length = 0;
		HCTD_Channel_Table[td_id].Endp_Size = USBHC_MAX_PACKET_SIZE;
		USB_PDevice[devinx].Hc.Control.Setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_VENDOR;
		USB_PDevice[devinx].Hc.Control.Setup.b.bRequest = 0x00;
		USB_PDevice[devinx].Hc.Control.Setup.b.wValue.w = 0x00;
		USB_PDevice[devinx].Hc.Control.Setup.b.wIndex.w = 0x04;
		USB_PDevice[devinx].Hc.Control.Setup.b.wLength.w = 16;
		
		USBHC_CtlReq(devinx,td_id);
		Result=USB_BUSY;
	}
	else
	{
		if (USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE || (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED))
		{
			USB_PDevice[devinx].Hc.EnumState = emustate;
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
			Result=USB_SUCCESS;
		}
		else
		{
			USB_PDevice[devinx].Hc.EnumState = ENUM_FAIL;
		}
		malloc_free(USB_PDevice[devinx].Hc.Control.Buf);
		USB_PDevice[devinx].Hc.Control.Buf = NULL;
	}
	return Result;
}
#endif

/*----------------------------------------------------------------------------
 * CTRL_State USBHC_SetDeviceFeature(U8_T devinx,U8_T td_id,U8_T feature,U8_T emustate)
 * Purpose : Perform the remote wakeup enable command in enumearation stage
 * Params  : $devinx: USBPD device id
 * 				   $td_id: The HC ATL TD number should be used 
 * Returns : none
 * Note    : none
 *----------------------------------------------------------------------------
 */
RESULT USBHC_SetDeviceFeature(U8_T devinx,U8_T td_id,U8_T feature,U8_T emustate)
{
	RESULT	Result;
	
	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		USBHC_Stdreq_SetFeature(devinx,td_id,
								USB_REQ_RECIPIENT_DEVICE,//Device
								(U16_T)feature, //DEVICE_REMOTE_WAKEUP(Feature Selector)
								0); //0 for Device
		Result = USB_BUSY;
	}
	else
	{
		if ((USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE) || (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED))
		{
			USB_PDevice[devinx].Hc.EnumState = emustate;
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
			Result = USB_SUCCESS;
		}
		else
		{
			USB_PDevice[devinx].Hc.EnumState = ENUM_FAIL;
			Result = USB_ERROR;
		}
	}
	return Result;
}

/*----------------------------------------------------------------------------
 * CTRL_State USBHC_GetStringDesc(U8_T devinx,U8_T td_id,U8_T stringidx,U8_T desc_len,U8_T emu_process_id)
 * Purpose : Perform the set configuration operation of enumearation
 * Params  : $devinx: USBPD device id
 * 				   $td_id: The HC ATL TD number should be used 
 * Returns : none
 * Note    : none
 *----------------------------------------------------------------------------
 */
RESULT USBHC_GetStringDesc(U8_T devinx,U8_T td_id,U8_T stringidx,U8_T desc_len,U8_T emu_process_id)
{
	RESULT					Result=USB_ERROR;
	USBHC_DevDesc_TypeDef	*devsc;
	U8_T					valid=1,*pr;

	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		//2016-12-27 fxied, stringidx should between from 1 to 3
		if ((stringidx >= 1) && (stringidx <= 3))
		{			
			devsc = (USBHC_DevDesc_TypeDef *)(USB_PDevice[devinx].Desc[DEVICE_DESC].Ptr);
			pr = &(devsc->iManufacturer);
			valid = pr[stringidx-1]; // start from iManufacturer	
			if (valid)
			{
				if (valid != stringidx) // if the stringidx is different from index, change to stringidx
				{
					stringidx = valid;
				}
			}
		}
		USBHC_String_Idx_Fix = stringidx; // now keep the last idx
		// get standard configuration descriptor
		if (valid)
		{
			if (desc_len == 0)
			{
				desc_len = 8;
			}

			malloc_free(USB_PDevice[devinx].Hc.Control.Buf);
			USB_PDevice[devinx].Hc.Control.Buf = m_malloc(desc_len,10); //* allocate 9 bytes
			HCTD_Channel_Table[td_id].Buf = USB_PDevice[devinx].Hc.Control.Buf; // also assign to host channel buffer
			if (USB_PDevice[devinx].Hc.Control.Buf != NULL)
			{
				//2.Set maximum package size
				USBHC_Stdreq_GetStringDesc(devinx,td_id,stringidx,desc_len);
				Result=USB_BUSY;
			}
			else
			{
				// Terminate the Task .........
				USBHC_Memory_Fail_Msg(0x02);
				USB_PDevice[devinx].Hc.EnumState = ENUM_FAIL;
			}
		}
		else
		{
			// Not Support this string descriptor
			if (stringidx != 0xee)
			{
				stringidx += LANG_ID;
			}
			else
			{
				stringidx = MICROSOFT_OS;
			}
			USB_PDevice[devinx].Hc.EnumState = emu_process_id; // perfoom next
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
			USB_PDevice[devinx].Desc[stringidx].Len = 0; // not leng
			Result=USB_SUCCESS;
		}
	}
	else
	{
		if (stringidx != 0xee)
		{
			USBHC_String_Idx_Fix = stringidx + LANG_ID;
		}
		else
		{
			USBHC_String_Idx_Fix = MICROSOFT_OS;
		}
		if (USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE)
		{
			//Get string index
			USB_PDevice[devinx].Desc[USBHC_String_Idx_Fix].Len = USB_PDevice[devinx].Hc.Control.Buf[0];
			if (desc_len == 0)
			{
				malloc_free(USB_PDevice[devinx].Hc.Control.Buf);
			}
			else // it is full
			{
				USB_PDevice[devinx].Desc[USBHC_String_Idx_Fix].Ptr = USB_PDevice[devinx].Hc.Control.Buf;
			}
			USB_PDevice[devinx].Hc.Control.Buf = 0;
			USB_PDevice[devinx].Hc.EnumState = emu_process_id;
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
			Result=USB_SUCCESS;
		}
		else
		{
			if (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED)
			{
				USB_PDevice[devinx].Hc.EnumState = emu_process_id; // perfoom next
				USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
				Result=USB_SUCCESS;
			}
			USB_PDevice[devinx].Desc[USBHC_String_Idx_Fix].Len = 0;
			malloc_free(USB_PDevice[devinx].Hc.Control.Buf);
			USB_PDevice[devinx].Hc.Control.Buf = 0;
		}
	}
	return Result;
}

/*----------------------------------------------------------------------------
 * void USBHC_Memory_Fail_Msg(U8_T msgindex)
 * Purpose : Print out the UART message when memory alloc fault
 * Params  : $msgindex: the process id 
 * Returns : none
 * Note    : none
 *----------------------------------------------------------------------------
 */
void USBHC_Memory_Fail_Msg(U8_T msgindex)
{
	printf("USBHC Error(%d):Memory Alloc Failure\n\r",(U16_T)msgindex);
}

/*----------------------------------------------------------------------------
 * void USBHC_InterruptTransfer_Mouse_Out(U8_T devinx,U8_T *buf,U8_T len,U8_T endpidx,U8_T kvm_host)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 *----------------------------------------------------------------------------*/
void USBHC_InterruptTransfer_Mouse_Out(U8_T devinx,U8_T *buf,U8_T len,U8_T endpidx,U8_T kvm_host)
{
	if (USBDC_Check_Endp_Buffer_Valid(kvm_host,devinx,endpidx))
	{
		USBDC_EndpBufPtr[kvm_host][devinx][endpidx][0] = len;
		memcpy((USBDC_EndpBufPtr[kvm_host][devinx][endpidx]+2),buf,len);
		USBDC_REGS_Endp_ControlSet(kvm_host,devinx,endpidx,DA_CR_BVLD_SET); // inform data move
	}
}

/*----------------------------------------------------------------------------
 * void USBHC_InterruptTransfer_HID_Mouse(U8_T *buf,U8_T buf_type,U8_T len)
 * Purpose : 
 * Params  : 
 * Returns : none
 * Note    : none
 *----------------------------------------------------------------------------*/
void USBHC_InterruptTransfer_HID_Mouse(U8_T *buf,U8_T buf_type)
{			
	DATAST_Append_Generic_MS_IN_Q(buf_type,buf); // normal mouse package	
#if	(SYSTEM_EXTENDER_RECEIVER)
#else		
	DATAST_PS2_MS_Handle(1);
#endif	/* #if	(SYSTEM_EXTENDER_RECEIVER) */	
}

/*----------------------------------------------------------------------------
 * void USBHC_InterruptTransfer_Mouse(U8_T devinx,U8_T inttid,U8_T *buf,U8_T len)
 * Purpose : Print out the UART message when memory alloc fault
 * Params  : $msgindex: the process id 
 * Returns : none
 * Note    : none
 *----------------------------------------------------------------------------
 */
/*
void USBHC_InterruptTransfer_Mouse(U8_T devinx,U8_T intt_id,U8_T *buf,U8_T len)
{	
	U8_T    kvm_host;		
	U8_T    skip_flag;	
	U8_T    buf_type=0;
	U8_T    check_devinx=USBDC_VHID_DEVINX;
	//U8_T    mouse_mode;
	
	kvm_host = KVM_CurrentHost;			
#ifdef HID_PARSER_CORE_MS	
	//if (((KVM_Flash.cSystemFlag3 & SYSTEM_MPASS_MASK) == MPASS_ACROSS_MODE)	&& (KVM_Flash.cSystemFlag3 & SYSTEM_MOUSE_SW_MASK))
	if ((KVM_Flash.cSystemFlag3 & SYSTEM_MPASS_MASK) == MPASS_ACROSS_MODE) //Mouse Passthrough accross mode
	{
		USBHC_HidParser_Package_Handle(devinx,HC_IntTransfer_Table[intt_id].InterfaceIdx,buf,len,0);			
	}
	else
	{	
		memcpy(Hid_Data,buf,len);	
		USBHC_HidParser_Package_Handle(devinx,HC_IntTransfer_Table[intt_id].InterfaceIdx,Hid_Data,len,0);
	}	
	
	if (hid_data_valid_flag==0)
	{		
		return;
	}	
		
	//if ((KVM_Flash.cSystemFlag3 & SYSTEM_MPASS_MASK) == MPASS_ACROSS_MODE) //Mouse Passthrough accross mode
	//if ((KVM_Flash.cSystemFlag2 & SYSTEM_MPASS_MASK) == MPASS_ACROSS_MODE) //Mouse Passthrough accross mode
	//{			
	DATAST_Generic_Mouse_Data(&Mouse_Data,&buf_type,1);		
	//}	
	//else
	//{		
	//	DATAST_Generic_Mouse_Data(&Mouse_Data,&buf_type,0);	
	//}		
#endif // #ifdef HID_PARSER_CORE_MS

	//@@Check Port Suspend & Wakeup condition 
#ifndef ONE_HID		
    if (buf_type != GENERIC_USAGE_ABS_MAKE)	
		check_devinx = devinx;	
#endif
	
	skip_flag = USBDC_Check_Upstream_Suspend_State(kvm_host,check_devinx);	
	
	// 2.Process HID output 
	if (skip_flag == 0)
	{					
		if (KVM_Check_MS_Skip_Condition() == 0)
		{
#ifdef ONEHID			
			USBHC_InterruptTransfer_HID_Mouse(DATAST_Generic_USB_MS_Report,buf_type);
#else			
			if (buf_type == GENERIC_USAGE_ABS_MAKE)	
			{
				USBHC_InterruptTransfer_HID_Mouse(DATAST_Generic_USB_MS_Report,buf_type);		
				return;
			}						
#endif	// #ifdef ONEHID 
		}
	}	 
}
*/
#ifdef SYNC
/*----------------------------------------------------------------------------
 * void USBHC_VirtualHID_MouseSync_XY0(void)
 * Purpose : Set Mouse position in x=0,y=0
 * Params  : $msgindex: the process id 
 * Returns : none
 * Note    : none
 *----------------------------------------------------------------------------
 */
void USBHC_VirtualHID_MouseSync_XY0(void)
{	
	U8_T    kvm_host;	
	//U8_T    skip_flag;
	U8_T    mbuf[4]={0x00,1,1,0};
	
	kvm_host = KVM_CurrentHost;		
	
	/*@@Check Port Suspend & Wakeup condition */
	//skip_flag = USBDC_Check_Upstream_Suspend_State(kvm_host,USBDC_VHID_DEVINX);	
	
	/* 2.Process HID output */
	//if (skip_flag == 0)
	{	
		//if (KVM_Check_MS_Skip_Condition() == 0)
		{			
			DATAST_Append_Generic_MS_IN_Q(GENERIC_USAGE_07_MAKE,mbuf); // normal mouse package
			DATAST_PS2_MS_Handle(1);
		}
	}	 
}
#endif

#ifdef SYNC	
void USBHC_InterruptTrnasferIN_Sync(U8_T sync_device_mask,U8_T port,U8_T intt_id,U8_T vhid_inf)
{
	U8_T para;
	
	if (KVM_Flash.cSystemFlag2 & sync_device_mask)
	{
		if (KM_SYNC_Port_Setting(port) == 0)
		{	
USBHC_InterruptTrnasferIN_Sync_Out:			
			if ((HC_IntTransfer_Table[intt_id].PassControl.SyncState & BIT_MASK[port]) == 0)
			{																			
				HC_IntTransfer_Table[intt_id].PassControl.SyncState |= BIT_MASK[port];
				para = intt_id | (port << 6); 
				//printf("[p:%bu]",port);
				TASK_Active(TASK_TYPE_EVENT,TASK_DATAST_USB_Virtual_HID_ID,vhid_inf,para,0,0); // now active the Virtual HID Output Handle																
				return;
			}	
		}		
	}	
	else
	{
		if (port == KVM_CurrentHost)
		{
			goto USBHC_InterruptTrnasferIN_Sync_Out;
		}			
	}		
	HC_IntTransfer_Table[intt_id].PassControl.SyncRp[port] = HC_IntTransfer_Table[intt_id].PassControl.SyncWp[port];	
}	
#endif
/*----------------------------------------------------------------------------
 * voi USBHC_InterruptTransferIN_Buf(U8_T intt_id,U8_T devinx,U8_T len)
 * Purpose : Process HC interrupt tranfer IN buffer
 * Params  : none
 * Returns : none
 * Note    : none
 *----------------------------------------------------------------------------
 */
U8_T USBHC_InterruptTransferIN_Buf(U8_T intt_id,U8_T devinx,U8_T len)
{
	U8_T	infid,remain_siz;
	bit		bufskip_flag=0;
	U8_T	wp,databyte,rp;
	U8_T	DJ_Buf[8];	
	U8_T    endp_id;
	U8_T    vhid_inf = USBDC_VHID_MS_ENDP_NUM;	
	U8_T	buf_type=0;
	U8_T    sync_device_mask;
	//U8_T	vhid_data_len=4;
 

#if (SYSTEM_EXTENDER_RECEIVER) && !defined(ONEHID)	
	if ((KVM_CurrentHost == REMOTE_HOST_PORT) && USB_PDevice[devinx].Hc.Etdr_RejectMountFlag)
	{		
		HC_IntTransfer_Table[intt_id].PassControl.Wp = 0;
	    HC_IntTransfer_Table[intt_id].PassControl.Rp = 0;
		return 0; 
	}		
#endif

	/* Check interrupt class type, HID or HUB or other...... */
	infid = HC_IntTransfer_Table[intt_id].InterfaceIdx;
	endp_id = HC_IntTransfer_Table[intt_id].EndpIdx & USBHC_PDEV_ENDPIDX_MASK;
	
#ifdef SYNC
	if (USBHC_PDevice_SyncInit[devinx] & BIT_MASK[infid])
	{
		USBHC_PDevice_SyncInit[devinx] &= ~BIT_MASK[infid];
		HC_IntTransfer_Table[intt_id].PassControl.SyncState = 0;		
		memset(HC_IntTransfer_Table[intt_id].PassControl.SyncRp,HC_IntTransfer_Table[intt_id].PassControl.Rp,KVM_MAX_PORT);		
	}		
#endif /* #ifdef SYNC */	
	
	
#if (SYSTEM_EXTENDER_RECEIVER)
//#if (SYSTEM_EXTENDER_RS232_MODE)
#if (HUART_BAUD == HUART_115K)
	if (KVM_CurrentHost == REMOTE_HOST_PORT) 
	{			
		if ((USB_PDevice[devinx].Interval_Adjust & BIT_MASK[infid]) == 0)
		{
			if (USB_PDevice[devinx].EndpInterval[infid][endp_id] < 8)	 
			{									
				HCTD_Table.INTL[intt_id].TD.Byte7 = 8;
				USB_PDevice[devinx].Interval_Adjust |= BIT_MASK[infid];				
			}		
		}
	}			
#endif /* #if (HUART_BAUD == HUART_115K) */	

#if (HUART_BAUD == HUART_19200)  
	if (KVM_CurrentHost == REMOTE_HOST_PORT) 
	{			
		if ((USB_PDevice[devinx].Interval_Adjust & BIT_MASK[infid]) == 0)
		{
			if (USB_PDevice[devinx].EndpInterval[infid][endp_id] < 15)	 
			{					
				HCTD_Table.INTL[intt_id].TD.Byte7 = 15;
				USB_PDevice[devinx].Interval_Adjust |= BIT_MASK[infid];				
			}		
		}
	}			
#endif /* #if (HUART_BAUD == HUART_115K) */	
//#endif /* if (SYSTEM_EXTENDER_RS232_MODE) */	
#endif	

	if (USB_PDevice[devinx].DevClass == USB_HUB_CLASS)
	{
		USBHC_HUB_StatusChangeHandle(devinx,intt_id);
	}
	else
	{
		//1_1.Check HID keyboard interface
		//printf("D:%bu,infid:%bu,len=%bu,",devinx,infid,len);
		//Disp_Str(HCTD_Table.INTL[intt_id].Buf,len);
		if (USB_PDevice[devinx].InfProtocol[infid] == INF_KEYBOARD)
		{
			sync_device_mask = SYSTEM_KB_SYNC_MASK;
#if (SYSTEM_EXTENDER_RECEIVER) && defined(ONEHID)			
			vhid_inf = USBDC_VHID_KB_ENDP_NUM;
#endif			
			//1_1.Keyboard Report Data Parser
			if (USB_PDevice[devinx].HID_Kb_Buf != NULL)
			{
				if (KVM_Flash.cSystemFlag3 & SYSTEM_VHID_KB_MASK)	
				{			
					printf("USB KB rpt parsed\n\r");	
					USBHC_HidParser_Package_Handle(devinx,infid,HCTD_Table.INTL[intt_id].Buf,len,0);					
					return 0;
				}
				else
				{					
					memcpy(Hid_Data,HCTD_Table.INTL[intt_id].Buf,len);	
					USBHC_HidParser_Package_Handle(devinx,infid,Hid_Data,len,0);
					#ifdef SYNC
					if (KM_SYNC_HOTKEY_Skip_Flag)
					{	
						if (KM_SYNC_HOTKEY_Clear_Flag)
						{
							KM_SYNC_HOTKEY_Clear_Flag = 0;
							memset(HCTD_Table.INTL[intt_id].Buf,0x00,len);
						}							
						else
						{								
							bufskip_flag=1;
						}	
					}	
					#endif
				}					
			}
			//1_2.Hotkey bufer skip condition
			if (KVM_Check_KB_Skip_Condition())
			{				
				bufskip_flag=1;				
			}
			
			#ifdef KMLOG
			if (KM_Check_Skip_Condition())
			{	
				return 1;
			}	
			#endif
//#ifdef ONE_HID
			//if ((KVM_Flash.cSystemFlag3 & SYSTEM_MPASS_MASK) == MPASS_ACROSS_MODE)
			if (KVM_Flash.cSystemFlag3 & SYSTEM_VHID_KB_MASK)
			{	
				//if ( (bufskip_flag == 1) || (bufskip_flag == 0))
				//{				
				HC_IntTransfer_Table[intt_id].PassControl.Wp = 0;
				HC_IntTransfer_Table[intt_id].PassControl.Rp = 0;				
				return 0;
				//}	
			}	
//#endif	
		}
		else
		{	
			sync_device_mask = SYSTEM_MS_SYNC_MASK;
			//1_2.Mouse Report Data Parser	
#ifdef HID_PARSER_CORE_MS 
			//if (((KVM_Flash.cSystemFlag3 & SYSTEM_MPASS_MASK) == MPASS_ACROSS_MODE)	&& (KVM_Flash.cSystemFlag3 & SYSTEM_MOUSE_SW_MASK))
			if ((KVM_Flash.cSystemFlag2 & SYSTEM_MS_DN_MASK) == 0) //Absolute coordination
			{				
				USBHC_HidParser_Package_Handle(devinx,HC_IntTransfer_Table[intt_id].InterfaceIdx,HCTD_Table.INTL[intt_id].Buf,len,0);
				if (hid_data_valid_flag==0)
				{			
					return 0;
				}				
				
				//to hide the mouse cursor
				#ifdef SYNC
				if (Coordinate_State & MOUSE_JUMP_MASK)
				{	
					return 0;
				}
				#endif				
				DATAST_Generic_Mouse_Data(&Mouse_Data,&buf_type,1);		
			}		
			else			
			{					
				memcpy(Hid_Data,HCTD_Table.INTL[intt_id].Buf,len);			
				USBHC_HidParser_Package_Handle(devinx,HC_IntTransfer_Table[intt_id].InterfaceIdx,Hid_Data,len,0);				
				DATAST_Generic_Mouse_Data(&Mouse_Data,&buf_type,0);	
			}			
			#ifdef KMLOG
			if (KM_Check_Skip_Condition())
			{	
				return 1;
			}	
			#endif
			/*
			if (buf_type == GENERIC_USAGE_ABS_MAKE)
			{
				if ((KVM_Flash.cSystemFlag2 & SYSTEM_MS_DN_MASK) == 0) //Absolute coordination
				{
					vhid_data_len = 6;
				}			
			}
*/			
#endif /* #ifdef HID_PARSER_CORE_MS */					
			
#ifdef ONEHID			
			USBHC_InterruptTransfer_HID_Mouse(DATAST_Generic_USB_MS_Report,buf_type);
			return;
#else			
			if ((KVM_Flash.cSystemFlag2 & SYSTEM_MS_DN_MASK) == 0) //Absolute coordination
			{				
				//printf("M");
				//Disp_Str(DATAST_Generic_USB_MS_Report,6);
				USBHC_InterruptTransfer_HID_Mouse(DATAST_Generic_USB_MS_Report,buf_type);		
				return 0;
			}						
#endif	/* #ifdef ONEHID */										
			if (USB_PDevice[devinx].DevAttr & DEVATTR_UNIFYING_MASK) //Logitech Unifying wirelesee hid device
			{
				if (USBHC_Unifying_Interrupt_In_Check(KVM_CurrentHost,devinx,intt_id,DJ_Buf,len,INF_KEYBOARD))
				{
					if (KVM_Check_KB_Skip_Condition())
					{
						bufskip_flag=1;
					}
					goto USBHC_InterruptTransferIN_Buf_Restart;
				}
				if (USBHC_Unifying_Interrupt_In_Check(KVM_CurrentHost,devinx,intt_id,DJ_Buf,len,INF_MOUSE))
				{
#if (SYSTEM_EXTENDER_RECEIVER)
					if (KVM_CurrentHost == REMOTE_HOST_PORT)
					{
						memcpy(HCTD_Table.INTL[intt_id].Buf,DJ_Buf,8);
						len = 8;
						goto USBHC_InterruptTransferIN_Buf_Restart;
					}
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */
					//USBHC_InterruptTransfer_Mouse(devinx,1,DJ_Buf,8);
					//return 1;
				}
			}
#if (SYSTEM_EXTENDER_RECEIVER)
			if (KVM_CurrentHost == REMOTE_HOST_PORT)
			{
				goto USBHC_InterruptTransferIN_Buf_Restart;
			}
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */						
			//USBHC_InterruptTransfer_Mouse(devinx,intt_id,HCTD_Table.INTL[intt_id].Buf,len);
			//return 1; // return OK
			
		}

USBHC_InterruptTransferIN_Buf_Restart:
#if (SYSTEM_EXTENDER_RECEIVER)
		if (KVM_CurrentHost == REMOTE_HOST_PORT)
		{
#ifdef ONEHID			
			if (ExtenderR_Check_Transmitter_Plug_State()==0)
#else			
			if ((ExtenderR_Check_Transmitter_Plug_State()==0) ||
			   ((USBDC_Device[devinx].DevIdx & USBDC_DEVIDX_INTR_SEND_OK_MASK)==0x00)) //check the transmitter connect condition.
#endif			
			{							
				HC_IntTransfer_Table[intt_id].PassControl.Wp = 0;
			    HC_IntTransfer_Table[intt_id].PassControl.Rp = 0;
				return 0;
			}
		}
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */

		//2.Move buffer out
		if (bufskip_flag ==0)
		{			 						
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
			if (remain_siz < (len+1))
			{
				//remove the data byte
				//What is needed to be done? remove the first old one or skip the current?
				USBHC_Remove_HID_Buffer(intt_id);
				goto USBHC_InterruptTransferIN_Buf_Restart;
			}
			
			//2_3.move interrupt in buffer data into out queue buffer
			HC_IntTransfer_Table[intt_id].PassControl.Buf[wp] = len;
			wp++;
			if (wp >= HC_IntTransfer_Table[intt_id].PassControl.BufSize)
				wp = 0;
			for (databyte =0; databyte < len ; databyte++)
			{
				HC_IntTransfer_Table[intt_id].PassControl.Buf[wp] = HCTD_Table.INTL[intt_id].Buf[databyte];
				wp++;
				if (wp >= HC_IntTransfer_Table[intt_id].PassControl.BufSize)
					wp = 0;
			}
			HC_IntTransfer_Table[intt_id].PassControl.Wp = wp;
#ifdef SYNC
			memset(HC_IntTransfer_Table[intt_id].PassControl.SyncWp,wp,USBDC_PORT_MAX);			
#endif	/* #ifdef SYNC */		
			//***************************************************
			// Check The HID buffer Task State
			// Data Stream Control
			//***************************************************
			
#if (SYSTEM_EXTENDER_RECEIVER)				
USBHC_INTERRUPT_DATA_OUTPUT:
#endif			
				
#if defined(ONEHID)				
	#if (SYSTEM_EXTENDER_RECEIVER)				
			if (KVM_CurrentHost != REMOTE_HOST_PORT)
			{	
				if (USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_KB_ENDP_NUM] & USBDC_UPORT[KVM_CurrentHost])
				{		
					TASK_Active(TASK_TYPE_EVENT,TASK_DATAST_USB_Virtual_HID_ID,0,intt_id,0,0); // now active the Virtual HID Output Handle
				}
			}
			else
			{	
				if ((HC_IntTransfer_Table[intt_id].PassControl.State & PASSTHROUGH_SET) == 0)
				{
					HC_IntTransfer_Table[intt_id].PassControl.State |= PASSTHROUGH_SET;
					intt_id |= (KVM_CurrentHost << 6);
					TASK_Active(TASK_TYPE_EVENT,TASK_DATAST_USB_Virtual_HID_ID,vhid_inf,intt_id,0,0); // now active the Virtual HID Output Handle
				}	
			}	
	#else
			if (USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_KB_ENDP_NUM] & USBDC_UPORT[KVM_CurrentHost])
			{	
				intt_id |= (KVM_CurrentHost << 6);	
				TASK_Active(TASK_TYPE_EVENT,TASK_DATAST_USB_Virtual_HID_ID,vhid_inf,intt_id,0,0); // now active the Virtual HID Output Handle
			}
	#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */						
#endif	/* #if defined(ONEHID) */			
			
#ifdef SYNC			
			endp_id = 0;
			infid = KVM_MAX_PORT;
#else
			endp_id = KVM_CurrentHost;
			infid = KVM_CurrentHost+1;
#endif			
			for (;endp_id < infid; endp_id++)
			{
				//if (endp_id != KVM_CurrentHost)
				{//mouse or keyboard			
					USBHC_InterruptTrnasferIN_Sync(sync_device_mask,endp_id,intt_id,vhid_inf);
				}	
			}			
		}
	}
	return 1;
}

/**
* @brief  USBHC_State_Machine_Busy_Check
*         USB Host core singole command state machine process
* @param  None 
* @retval None
*/
U8_T USBHC_State_Machine_Busy_Check(U8_T devinx)
{
	if ((USB_PDevice[devinx].Hc.Control.State != CTRL_IDLE) && (USB_PDevice[devinx].Hc.Control.State != CTRL_COMPLETE))
	{
		return 1;
	}
	
	if (USB_PDevice[devinx].Hc.EnumState)
	{
		return 2;
	}
	
	if (USB_PDevice[devinx].Hc.HID_EnumState)
	{
		return 3;
	}
	
	if (USB_PDevice[devinx].Hc.HUB_EnumState)
	{
		return 4;
	}
	
	if (USB_PDevice[devinx].Hc.ScmdState)
	{
		return 5;
	}
	
	if (USB_PDevice[devinx].Hc.PassThrough_State)
	{
		return 6;
	}
	
	//2017-05-22 Add by Jack, for if the PC enumlate the keyboard very fast and send the led command 
	if (USB_PDevice[devinx].Hc.gState == HOST_CLASS_ENUMERATION) // next go other Class Maintain.	
	{
		return 7;
	}
		
	return 0;
}

/**
* @brief  TASK_USBHC_Scm_Handle
*         USB Host core singole command state machine process
* @param  None 
* @retval None
*/
void TASK_USBHC_Scm_Handle_Start(void)
{
	U8_T	devinx, td_id, para2;

	devinx = TASK_Register0;
	para2 = TASK_Register1;
	
	/* Skip task if the device is gone */
	if ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00)  // Device is gone  
	{	
		return;
	}
	
	/* Check if HC state machine not idle now */	
	if (USBHC_State_Machine_Busy_Check(devinx))
	{
		TASK_Wait_Current();
		return;
	}
	
	/* Get Free TD of ATL */
	if (USBHC_Control_TD_Init(devinx,&td_id) != USBH_OK)
	{
		TASK_Wait_Current();
		return;
	}

	/* Now Occupy the SCM State Machine */
	USB_PDevice[devinx].Hc.ScmdState = USB_PDevice[devinx].Hc.ScmdStep;
	USB_PDevice[devinx].Hc.gState = HOST_SCM;
	TASK_Active(TASK_TYPE_USB, TASK_USBHC_Scm_Handle_ID, 0, devinx, para2, 0); //Generate the task, for next Event check
}

/**
* @brief  TASK_USBHC_Scm_Handle
*         USB Host core singole command state machine process
* @param  None 
* @retval None
*/
void TASK_USBHC_Scm_Handle(void)
{
	U8_T	devinx,td_id,intf_id,mode,para2;
	RESULT	Result;

	devinx = TASK_Register0;
	para2 = TASK_Register1;
	td_id = USB_PDevice[devinx].Hc.Control.TdNum; // which host channel is used by device

	//Skip task if the device is gone
	if ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00)  // Device is gone  
	{	
		return;	
	}
	
	switch (USB_PDevice[devinx].Hc.ScmdStep)
	{
		case SCMD_SET_LED_REPORT:
			Result = USBHC_SCM_SetLedReport(devinx,td_id);
			break;
		case SCMD_SET_UNIFYING_REPORT:
			Result = USBHC_SCM_SetUnifyingReport(devinx,td_id,para2);
			break;
		case SCMD_SET_UNIFYING_HID: //HID++ mode or HID mode
			Result = USBHC_SCM_SetUnifyingHID(devinx,td_id,para2);
			break;
		case SCMD_SET_PROTOCOL:
			mode = para2 & 0x0f;
			intf_id = (para2 & 0xf0) >> 4;
			Result = USBHC_SCM_SetProtocol(devinx,td_id,intf_id,mode);
			break;
		case SCMD_SET_CONFIGURATION:
			Result = USBHC_SCM_SetConfiguration(devinx,td_id,para2);
			break;
		case SCMD_SET_INTERFACE:
			Result = USBHC_SCM_SetInterface(devinx,td_id,para2);
			break;
		case SCMD_SET_HUB_FEATURE:			
			Result = USBHC_SCM_SetHubFeature(devinx,td_id,para2);
			break;
		case SCMD_CLEAR_HUB_FEATURE:			
			Result = USBHC_SCM_ClearHubFeature(devinx,td_id,para2);
			break;
		case SCMD_CLEAR_FEATURE:
			break;
		case SCMD_SET_IDLE:
			Result = USBHC_SCM_SetIdle(devinx,td_id,para2);
			break;
	}

	if (Result == USB_SUCCESS)
	{
		USB_PDevice[devinx].Hc.ScmdState = SCMD_COMPLETE;
	}
	else if (Result == USB_ERROR)
	{
		USB_PDevice[devinx].Hc.ScmdState = SCMD_FAIL;
		
		/* The Kworld TouchPad has a bug, it doese support suspend request */
		if (USB_PDevice[devinx].Hc.ScmdStep == SCMD_SET_IDLE)
		{
			/*Need to reset device */
			//1.unplug device first.
			//2.remove device
			USB_PDevice[devinx].Hc.EnumState = HID_ENUM_IDLE;
			USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
			USBHC_Remove_Device(devinx, 0);
			//printf("Cmd Error, Remove devinx:%bd\n\r", devinx);
			//3.do the reset(for root hub or other hub)
		}
	}
	else
	{
		return;
	}

	USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
	USBHC_CORE_Free_TD(USB_PDevice[devinx].Hc.Control.TdNum);
	TASK_Active(TASK_TYPE_USB,TASK_USBHC_Process_ID,0x00,devinx,0,0);  // Generate the task to go into HC main control
}

/*------------------------------------------------------------------------------
 U8_T USBHC_KB_Led_Brocast(U8_T led)
 @brief  brocast the led value to all USB keyboard device
 @param  $devinx:
          $total_len:receive description table length
          $buf:the buffer which can received
 @retval None
--------------------------------------------------------------------------------*/
void USBHC_KB_Led_Brocast(U8_T led)
{
	U8_T	devinx;
	
	for (devinx=USBDC_VIRTUAL_DEVINX; devinx < USB_HC_MAX_DEVICE ; devinx++) // start from first devinx
	{
		if (USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK)
		{			
			if (USB_PDevice[devinx].DevAttr & DEVATTR_KB_CLASS_MASK)				
			{
				if (USBHC_KB_Led_Check(devinx))
				{
					if ((USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK) == 0x00)
					{	
						if (USB_PDevice[devinx].HID_Kb_LED_Value[1] != led)
						{
							USB_PDevice[devinx].HID_Kb_LED_Value[1] = led;
							USB_PDevice[devinx].Hc.ScmdStep = SCMD_SET_LED_REPORT;
							TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_Scm_Handle_Start_ID,0,devinx,SCMD_SET_LED_REPORT,0);
						}
					}
				}	
			}         
		}
	}
}

/*------------------------------------------------------------------------------
 U8_T TASK_USBHC_KVM_Switch_Check(U8_T port)
 @brief  After KVM switching complete,check the value with curretn port setting
         [1].Protocol
         [2].COnfiguration 
 @param  $port:current active kvm port number
 @retval None
--------------------------------------------------------------------------------*/
void TASK_USBHC_KVM_Switch_Check(void)
{
	U8_T	port, index;
	U8_T	devinx, curinf, intf_bit, protocol_value, max_device;

	max_device = (TASK_Register0 & 0xf0) >> 4;
	devinx = (TASK_Register0 & 0x0f);
	port = TASK_Register1;
	TASK_USBHC_KVM_Switch_Check_Start_Flag = 1;
	
	if (USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) // if the device is in used
	{
		//2.Special Handle for some devices compatible issue
		if (USB_PDevice[devinx].DevAttr & DEVATTR_UNIFYING_MASK) //Logitech Unifying wirelesee hid device
		{
			USBHC_Unifying_KVM_Switch_Check(port,devinx);
		}
		else if (USB_PDevice[devinx].DevAttr & DEVATTR_TD2220_MASK) //Viewsonic TD2220
		{
			if ((USB_PDevice[devinx].DevAttr & TD2220_FORMAT_MASK) != (USBDC_Device[devinx].DevAttr[port] & TD2220_FORMAT_MASK))
			{
				USBHC_Remove_Device(devinx, 1);
			}
		}
		
		//3.Handle Protocol, Alternate Setting
#ifndef ONEHID			
		for (curinf=0; curinf < (USB_PDevice[devinx].InterfaceNum & USBHC_DEVICE_INTERFACENUM_MASK) ; curinf++)
		{
			//3.1 Check Protocol for HID Class
			if (USB_PDevice[devinx].InfClass[curinf] == USB_HID_CLASS) // if it is hid class
			{
				intf_bit = 0x01 << curinf;
				protocol_value = USBDC_Device[devinx].Current_Protocol[port] & intf_bit;
				if (protocol_value != (USB_PDevice[devinx].Current_Protocol & intf_bit))
				{
					USBHC_Set_Dev_Inf_Protocol(devinx,curinf,protocol_value,1);
				}
			}
			//3.2 Check Alternating Setting for Audio Class
			if (USB_PDevice[devinx].DevClass != USB_AUDIO_CLASS)
			{
				if (USBDC_Device[devinx].Current_AlternateSetting[port][curinf] != USB_PDevice[devinx].Current_AlternateSetting[curinf])
				{
					USB_PDevice[devinx].Current_AlternateSetting[curinf] = USBDC_Device[devinx].Current_AlternateSetting[port][curinf];
					USBHC_Set_Dev_Interface(devinx,curinf,USB_PDevice[devinx].Current_AlternateSetting[curinf]); // interface,alternate setting
				}
			}
		}
#endif		
		//4. Check Configuration Setting
		if (USB_PDevice[devinx].ConfigurationValue != USBDC_Device[devinx].Current_Configuration[port])
		{
			if (USBDC_Device[devinx].Current_Configuration[port])
			{
				USB_PDevice[devinx].ConfigurationValue = USBDC_Device[devinx].Current_Configuration[port];
				USBHC_Set_Dev_Configuration(devinx,USBDC_Device[devinx].Current_Configuration[port]);
			}
		}
		//5. Handle the intval setting
		if ((devinx >= USBDC_VHID_DEVINX) && (USB_PDevice[devinx].DevClass == USB_HID_CLASS))
		{
			USBDC_Device[devinx].Interval_Detect[port] = 0; //need to start check endpoint
			for (index=1;index < USBDC_Device[devinx].EndpNum ;index++)
			{
				if ((USBDC_Device[devinx].EndpType[index] & USB_EP_TYPE_MASK) == USB_EP_TYPE_INTR) //Interrupt transfer type
				{
					intf_bit = USBDC_Get_Endp_Interval(port,devinx,index);
					if (intf_bit)
					{
						USBHC_CORE_Change_IntTransfer_Interval(devinx,index,intf_bit);
					}
					curinf = USBDC_Device[devinx].EndpType[index] & 0x0f;
#ifdef SYNC
					HC_IntTransfer_Table[curinf].PassControl.SyncState &= ~BIT_MASK[port+4];
#else					
					HC_IntTransfer_Table[curinf].PassControl.State &= ~(PASSTHROUGH_WAIT);
#endif					
				}
			}
		}
		//6. Handle the transmitter issue
#if (SYSTEM_EXTENDER_TRANSMITTER)
		USBDC_Device[devinx].EndpIntrAttr = 0; //reset the mouse
#endif		
	}

TASK_USBHC_KVM_Switch_Check_Next:
	if (++devinx < max_device)
	{
		if (USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) // if the device is in used
		{
			protocol_value = (max_device << 4) + devinx;
			TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_KVM_Switch_Check_ID,0x00,protocol_value,port,0); //Generate the task, for next Event check
		}
		else
		{
			goto TASK_USBHC_KVM_Switch_Check_Next;
		}
	}
	else
	{
		TASK_USBHC_KVM_Switch_Check_Start_Flag = 0;
	}
}

/*------------------------------------------------------------------------------
 U8_T TASK_USBHC_Resume_Command(U8_T port)
 @brief  After USB HC resume, this program will perform command broadcast
 @param  
 @retval None
--------------------------------------------------------------------------------*/
void TASK_USBHC_Resume_Command(void)
{
	U8_T	devinx, curinf, intf_bit;
	
	devinx = TASK_Register0;
	if (USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) // if the device is in used
	{
		//Check HID Class device
		intf_bit = 0x01;
		for (curinf = 0; curinf < (USB_PDevice[devinx].InterfaceNum & USBHC_DEVICE_INTERFACENUM_MASK); curinf++)
		{
			if (USB_PDevice[devinx].InfClass[curinf] == USB_HID_CLASS) // if it is hid class
			{
				//check the protocol setting.
				if (USB_PDevice[devinx].Hid_SetIdle_Allow & intf_bit)
				{
					USBHC_Set_Dev_Idle(devinx,curinf);
				}
			}
			intf_bit <<= 1;
		}
	}

ResumeNextDevice:
	if (++devinx < (USB_HC_MAX_DEVICE+USB_HC_MAX_HUB))
	{
		if (!(USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK))
			goto ResumeNextDevice;

		TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_Resume_Command_ID,0x00,devinx,0,0); //Generate the task, for next Event check
	}
}

/*------------------------------------------------------------------------------
 void USBHC_Set_Dev_Inf_Protocol(U8_T devinx,U8_T curinf,U8_T protocol_value,U8_T cmd_type)
 @brief  Subroutine for setting HC devcie interface protocol setting
 @param  $devinx:device index
         $curinf:interface number
         $protocol_value
 @retval None
--------------------------------------------------------------------------------*/
void USBHC_Set_Dev_Inf_Protocol(U8_T devinx,U8_T curinf,U8_T protocol_value,U8_T cmd_type)
{
	U8_T intf_bit;
#if (SYSTEM_EXTENDER_TRANSMITTER)	
	U8_T protocol_hold;
#endif	
	
#ifdef ONEHID
	if (devinx > USBDC_VHID_DEVINX)
	{
		return;
	}		
#endif	
	
	intf_bit = 0x01 << curinf;
	if (protocol_value)
	{
		USB_PDevice[devinx].Current_Protocol |= intf_bit;
		protocol_value = curinf << 4; // if boot protocol then the wvalue is 0	
	}
	else
	{
		USB_PDevice[devinx].Current_Protocol &= ~intf_bit;
		protocol_value = 0x01 | (curinf << 4); // if report protocol then the wvalue is 1		
	}
	
	if (devinx >= USBDC_VIRTUAL_DEVINX)
	{	
		if (cmd_type == 1)
		{
			//Special Handle for A4 Teck keyboard G800V
#if (SYSTEM_EXTENDER_TRANSMITTER)
			if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)//if remote device
			{			
				protocol_value = (curinf << 4) | (protocol_hold & 0x0f);
				ExtenderT_USB_Set_Protocol(devinx,curinf,protocol_value);
			}
			else
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */
			{
				USB_PDevice[devinx].Hc.ScmdStep = SCMD_SET_PROTOCOL;
				TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_Scm_Handle_Start_ID,0,devinx,protocol_value,0);
			}
		}
	}
}

/*------------------------------------------------------------------------------
 void USBHC_Set_Dev_Logitech_Unifying_Mode(U8_T devinx,U8_T report_value)
 @brief  Subroutine for setting HC devcie interface protocol setting 
 @param  $devinx:device index
         $curinf:interface number
         $protocol_value
 @retval None
--------------------------------------------------------------------------------*/
void USBHC_Set_Dev_Logitech_Unifying_Mode(U8_T devinx,U8_T report_value)
{
	if (devinx >= USBDC_VIRTUAL_DEVINX)
	{
#if (SYSTEM_EXTENDER_TRANSMITTER)
		if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)//if remote device
		{
			ExtenderT_USB_Send_Command(devinx,SCMD_SET_UNIFYING_REPORT,report_value);
		}
		else
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */
		{
			USB_PDevice[devinx].Hc.ScmdStep = SCMD_SET_UNIFYING_REPORT;
			TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_Scm_Handle_Start_ID,0,devinx,report_value,0);
		}	
	}
}

/*------------------------------------------------------------------------------
 void USBHC_Set_Dev_Logitech_Unifying_HID(U8_T devinx,U8_T report_value)
 @brief  Subroutine for setting HC devcie interface protocol setting 
 @param  $devinx:device index
         $curinf:interface number
         $protocol_value
 @retval None
--------------------------------------------------------------------------------*/
void USBHC_Set_Dev_Logitech_Unifying_HID(U8_T devinx,U8_T hid_value)
{
	if (devinx >= USBDC_VIRTUAL_DEVINX)
	{
#if (SYSTEM_EXTENDER_TRANSMITTER)
		if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)//if remote device
		{
			ExtenderT_USB_Send_Command(devinx,SCMD_SET_UNIFYING_HID,hid_value);
		}
		else
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */		
		{
			USB_PDevice[devinx].Hc.ScmdStep = SCMD_SET_UNIFYING_HID;
			TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_Scm_Handle_Start_ID,0,devinx,hid_value,0);
		}	
	}
}

/*------------------------------------------------------------------------------
 void USBHC_Set_Dev_Configuration(U8_T devinx,U8_T configuration_value)
 @brief  Subroutine for setting HC devcie configuration value
 @param  $devinx:device index
         $configuration_value
 @retval None
--------------------------------------------------------------------------------*/
void USBHC_Set_Dev_Configuration(U8_T devinx,U8_T configuration_value)
{
	if (devinx >= USBDC_VIRTUAL_DEVINX)
	{		
#if (SYSTEM_EXTENDER_TRANSMITTER)
		if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)//if remote device
		{	
			ExtenderT_USB_Send_Command(devinx,SCMD_SET_CONFIGURATION,configuration_value);
		}
		else
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */	
		{		
			USB_PDevice[devinx].Hc.ScmdStep = SCMD_SET_CONFIGURATION;
			TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_Scm_Handle_Start_ID,0,devinx,configuration_value,0);
		}	
	}
}

/*------------------------------------------------------------------------------
 void USBHC_Set_Dev_Idle(U8_T devinx,U8_T configuration_value)
 @brief  Subroutine for setting HC devcie configuration value
 @param  $devinx:device index
         $configuration_value
 @retval None
--------------------------------------------------------------------------------*/
void USBHC_Set_Dev_Idle(U8_T devinx,U8_T intf)
{
	if (devinx >= USBDC_VIRTUAL_DEVINX)
	{
#if (SYSTEM_EXTENDER_TRANSMITTER)
		if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)//if remote device
		{
			ExtenderT_USB_Send_Command(devinx,SCMD_SET_IDLE,intf);
		}
		else
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */	
		{		
			USB_PDevice[devinx].Hc.ScmdStep = SCMD_SET_IDLE;
			TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_Scm_Handle_Start_ID,0,devinx,intf,0);
		}	
	}
}

/*------------------------------------------------------------------------------
 void USBHC_Set_Dev_Interface(U8_T devinx,U8_T configuration_value)
 @brief  Subroutine for setting HC devcie configuration value
 @param  $devinx:device index
         $configuration_value
 @retval None
--------------------------------------------------------------------------------*/
void USBHC_Set_Dev_Interface(U8_T devinx,U8_T interface,U8_T alternate)
{
	U8_T	intf;

	if (devinx >= USBDC_VIRTUAL_DEVINX)
	{
#if (SYSTEM_EXTENDER_TRANSMITTER)							
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)		
		if (devinx == USBHC_Audio_Devinx)
		{
			if (interface == USBHC_Audio_In_Interface)
			{
				if (alternate) //turn ob the IN
				{				
					ETDR_AudioInHead = 0;
					ETDR_AudioInTail = 0;
					ETDR_AudioInDcDoingFlag = 0;							
				}	
			}	
		}
#endif /* if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	*/			
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */
		
		intf = interface << 4 | alternate;
#if (SYSTEM_EXTENDER_TRANSMITTER)
		if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)//if remote device
		{						
			ExtenderT_USB_Send_Command(devinx,SCMD_SET_INTERFACE,intf);
		}
		else
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */	
		{		
			USB_PDevice[devinx].Hc.ScmdStep = SCMD_SET_INTERFACE;			
			TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_Scm_Handle_Start_ID,0,devinx,intf,0);
		}	
	}
}

/*----------------------------------------------------------------------------
 * RESULT USBHC_SCM_SetConfiguration(U8_T devinx,U8_T td_id,U8_T mode)
 * Purpose : Perform the set address operation of enumearation
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
RESULT USBHC_SCM_SetConfiguration(U8_T devinx,U8_T td_id,U8_T configuration)
{
	RESULT	Result;

	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		USBHC_Stdreq_SetCfg(devinx,td_id,configuration);
		Result = USB_BUSY;
	}
	else
	{
		if ((USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE) || (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED))
		{
			Result = USB_SUCCESS;
		}
		else
		{
			Result = USB_ERROR;
		}
	}
	return Result;
}

/*----------------------------------------------------------------------------
 * RESULT USBHC_SCM_SetInterface(U8_T devinx,U8_T td_id,U8_T mode)
 * Purpose : Perform the set address operation of enumearation
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
RESULT USBHC_SCM_SetInterface(U8_T devinx,U8_T td_id,U8_T configuration)
{
	RESULT	Result;
	U8_T	intf_id;
	
	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		intf_id = (configuration & 0xf0) >> 4;
		configuration &= 0x0f;
		USBHC_Stdreq_SetInterface(devinx,td_id,configuration,intf_id);
		Result = USB_BUSY;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)		
		USBHC_AUDIO_In_Control_Check(devinx,intf_id,configuration);
#endif		
	}
	else
	{
		if ((USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE) || (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED))
		{
			Result = USB_SUCCESS;
		}
		else
		{
			Result = USB_ERROR;
		}
	}
	return Result;
}

#if (!SYSTEM_EXTENDER_SUPPORT)
/*******************************************************************************
* void USBHC_PassThrough_TD_Filled(U8_T devinx,U8_T port)
* Purpose : create a passthroug control
* Input   : None.
* Output  : None.
* Return  : None.
*******************************************************************************/
void USBHC_PassThrough_TD_Filled(U8_T devinx,U8_T port)
{
	USBDC_Device[devinx].PassThroughState[port] = PASSTHROUGH_SETUP_START; /* Indicate the device is in PassThrough Mode */
	/* Inital Total lenght & Current length */
	USBDC_Device[devinx].Ctrl_TotalByte[port] = USBDC_Device[devinx].Setup[port].b.wLength.w;
	USBDC_Device[devinx].Ctrl_CurrentByte[port] = 0;
	USBDC_Device[devinx].Control_EndpBuf[port] = m_malloc(USBDC_Device[devinx].EndpMaxSize[0],11); // only alloc the control endpoint buffer
	/* Update PassThrough Control Data Length field */
	USB_PDevice[devinx].Hc.PControl.Total_Length = USBDC_Device[devinx].Setup[port].b.wLength.w;
	USB_PDevice[devinx].Hc.PControl.Current_Length = 0;
	USB_PDevice[devinx].Hc.PControl.Data_Length = 0;
}
#endif /* (!SYSTEM_EXTENDER_SUPPORT) */

/*----------------------------------------------------------------------------
 * RESULT USBHC_SCM_SetIdle(U8_T devinx,U8_T td_id,U8_T intf_id)
 * Purpose : Perform the set idle operation of hid class device
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
RESULT USBHC_SCM_SetIdle(U8_T devinx,U8_T td_id,U8_T intf_id)
{
	RESULT	Result=USB_SUCCESS;
	 
	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{	
		USBHC_Classreq_SetIdle(devinx, td_id, USB_PDevice[devinx].InfSetIdle[intf_id], 0x00, intf_id);
		Result = USB_BUSY;
	}
	else
	{
		if ((USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE) || (USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED))
		{
			Result = USB_SUCCESS;
		}
		else
		{
			Result = USB_ERROR;
		}
	}
 
	return Result;
} 

/*******************************************************************************
* Function Name: TASK_USBHC_PassThrough_Handle_Start.
* Description  : Traffic control of PassThrough Command transmitt
* Input        :  
* Output       : None.
* Return       : None
*******************************************************************************/
void TASK_USBHC_PassThrough_Handle_Start(void)
{
	U8_T	devinx, pid;
	U8_T	td_id;
#if (SYSTEM_EXTENDER_SUPPORT)
	U8_T	passthrough_task_id;
#endif /* (SYSTEM_EXTENDER_SUPPORT) */
	
	devinx = TASK_Register0;
	pid = TASK_Register1; // command from whicp upstream port
	
	//Skip task if the device is gone
	if ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00)  // Device is gone  
	{
		return;	
	}
	
	/*Check the interrupt ternimal condition */
	if (USBDC_Device[devinx].PassThroughState[pid] == PASSTHROUGH_SETUP_START) /*The device is not in PassThrough Mode, then skip it*/
	{
		/* if the control endp pipe is not ready */
		if (USBHC_State_Machine_Busy_Check(devinx))
		{
#if (SYSTEM_EXTENDER_SUPPORT)
			TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_PassThrough_Handle_Start_ID,0x00,devinx,pid,0); // Generate the task, for next check
#else
			TASK_Wait_Current();
#endif
			return;
		}

#if (SYSTEM_EXTENDER_TRANSMITTER)
		if ((USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK) == 0x00)	//if not extender, then
#endif /* (SYSTEM_EXTENDER_TRANSMITTER) */
		{
			/*Get Free TD of ATL */
			if (USBHC_Control_TD_Init(devinx,&td_id) != USBH_OK)
			{
				TASK_Wait_Current();
				return;
			}
		}
		USB_PDevice[devinx].Hc.PassThrough_gState_bk = USB_PDevice[devinx].Hc.gState; //backup the qState
		
		//Occupy the PassThrough State Machine 
		USB_PDevice[devinx].Hc.gState = HOST_PASSTHROUGH; // goto passthrough command mode
		USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_SETUP_START; // Send Setup Token Out

		//Update Upstream port id 
		USB_PDevice[devinx].Hc.PControl.UPID = pid;
#if (SYSTEM_EXTENDER_SUPPORT)
		// Inital Total lenght & Current length 
		USBDC_Device[devinx].Ctrl_TotalByte[pid] = USBDC_Device[devinx].Setup[pid].b.wLength.w;
		USBDC_Device[devinx].Ctrl_CurrentByte[pid] = 0;
		// Update PassThrough Control Data Length field 
		USB_PDevice[devinx].Hc.PControl.Total_Length = USBDC_Device[devinx].Setup[pid].b.wLength.w;
		USB_PDevice[devinx].Hc.PControl.Current_Length = 0;
		USB_PDevice[devinx].Hc.PControl.Data_Length = 0;

#if (SYSTEM_EXTENDER_TRANSMITTER)
		if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)
		{
			if (ExtenderT_Setup_Buffer_Malloc(pid,devinx) == 0)
				return;
			passthrough_task_id = TASK_USBHC_ExtenderT_PassThrough_Handle_ID;
		}
		else
#endif // (SYSTEM_EXTENDER_TRANSMITTER) 
		{
#if (SYSTEM_EXTENDER_RECEIVER)
			if (pid == REMOTE_HOST_PORT)
			{
				ExtenderR_Setup_Buffer_Malloc(devinx);
				passthrough_task_id = TASK_USBHC_ExtenderR_PassThrough_ID;
			}
			else
#endif // SYSTEM_EXTENDER_RECEIVER
			{
				USBDC_Device[devinx].Control_EndpBuf[pid] = m_malloc(USBDC_Device[devinx].EndpMaxSize[0],12); // only alloc the control endpoint buffer
				passthrough_task_id = TASK_USBHC_PassThrough_Handle_ID;
			}

			if (USBDC_Device[devinx].Control_EndpBuf[pid] == NULL)
			{
				//printf(" USBHC_PASSTHROUGH Setup Malloc Fail !!\n\r");
				return;
			}	
				
			USB_PDevice[devinx].Hc.Control.Buf = USBDC_Device[devinx].Control_EndpBuf[pid];
			HCTD_Channel_Table[td_id].Buf = USBDC_Device[devinx].Control_EndpBuf[pid]; // Assing Buffer
		}
		
		TASK_Active(TASK_TYPE_EVENT,passthrough_task_id,0x00,devinx,0,0); //Generate the task, for next Event check
#else // For Non- SYSTEM_EXTENDER_SUPPORT
		USBHC_PassThrough_TD_Filled(devinx,pid);
		USB_PDevice[devinx].Hc.Control.Buf = USBDC_Device[devinx].Control_EndpBuf[pid];
		HCTD_Channel_Table[td_id].Buf = USBDC_Device[devinx].Control_EndpBuf[pid]; // Assing Buffer
		TASK_Active(TASK_TYPE_USB,TASK_USBHC_PassThrough_Handle_ID,0x00,devinx,0,0); //Generate the task, for next Event check
#endif // (SYSTEM_EXTENDER_SUPPORT) 
	}
}

/**
* void TASK_USBHC_PassThrough_Handle(U8_T devinx,U8_T pid)
* @brief  USB Host core passthroug control main state machine process
* @param  None 
* @retval None
*/
void TASK_USBHC_PassThrough_Handle(void)
{
	U8_T	devinx, td_id, pid;
	RESULT	Result = USB_BUSY;
	bit		terminate = 1;

	if (TASK_Type == TASK_TYPE_INTERVAL_MS)
	{
		 TASK_Destory_Current();
	}	

	devinx = TASK_Register0;
	td_id = USB_PDevice[devinx].Hc.Control.TdNum; // which host channel is used by device
	pid = USB_PDevice[devinx].Hc.PControl.UPID; // which host channel is used by device
	//Skip task if the device is gone
	if ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00 ||
		USB_PDevice[devinx].Hc.PassThrough_State == PASSTHROUGH_IDLE)  // Device is gone  
	{	
		if (USB_PDevice[devinx].Hc.Control.TdNum)
		{
			//printf("Free TD:%bu\n\r",USB_PDevice[devinx].Hc.Control.TdNum);
			USBHC_CORE_Free_TD(USB_PDevice[devinx].Hc.Control.TdNum);			
		}						
		return;
	}
	
#if (USB_PASSTHROUGH_DEBUG_MODE)
	printf(">> TASK_USBHC_PassThrough_Handle(%d)-(%d):\n\r",(U16_T)devinx,(U16_T)USB_PDevice[devinx].Hc.PassThrough_State);
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
TASK_USBHC_PassThrough_Handle_Restart:
	switch (USB_PDevice[devinx].Hc.PassThrough_State)
	{
		case PASSTHROUGH_SETUP_START: //Send A control setup to HC			
#if (USB_PASSTHROUGH_DEBUG_MODE)
			printf("   (1).SETUP_TOKEN Stage\n\r");
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
			/* convert back to normal USB order */
			/* Task Fork => Send out Setup Token */
			Result = USBHC_PassThrough_Send_Setup_Token(devinx,td_id);
			/* For No Data Stage */
			if (Result == USB_SUCCESS) // need to ACK DC
			{
				/* Check the data length */
				USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_DC_SETUP_STATUS_IN;
				goto TASK_USBHC_PassThrough_Handle_Restart;
			}
			else if (Result == USB_UNSUPPORT) 
			{
				USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_DC_SETUP_STALL;
				goto TASK_USBHC_PassThrough_Handle_Restart;
			}
			break;
		case PASSTHROUGH_SETUP_START_DONE: //After passthrough Setup token send complete
#if (USB_PASSTHROUGH_DEBUG_MODE)
			printf("SETUP Token Done\n\r");
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
			USBDC_REGS_Endp_ControlClear(pid,devinx,0,(DA_CR_SCLR_SET|DA_CR_BCLR_SET)); // now can receive data
			break;
		case PASSTHROUGH_DC_SETUP_DATA_IN: //Setup Data Stage
			/*update length first */
#if (USB_PASSTHROUGH_DEBUG_MODE)
			USB_PDevice[devinx].Hc.PControl.Current_Length += USB_PDevice[devinx].Hc.PControl.Data_Length;
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
			USBDC_VIRTUAL_Setup_DataStageIn(devinx,pid,1);	/*Set DC Buffre ready register*/
			if (USB_PDevice[devinx].Hc.Control.State == CTRL_STATUS_OUT)
			{
				TASK_Active(TASK_TYPE_USB,TASK_USBHC_Handle_Control_ID,td_id,devinx,0,0);
			}
#if (USB_PASSTHROUGH_DEBUG_MODE)
			printf("SETUP_DATA_IN Stage(%d/%d)\n\r",(U16_T)USB_PDevice[devinx].Hc.PControl.Current_Length,
			(U16_T)(U16_T)USB_PDevice[devinx].Hc.PControl.Total_Length);
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
			break;
		case PASSTHROUGH_HC_SETUP_DATA_OUT_DONE: //Setup Data Stage
			USBDC_REGS_Endp_ControlClear(pid,devinx,0,DA_CR_BCLR_SET); // Clear the OUT buffre flag
#if (USB_PASSTHROUGH_DEBUG_MODE)
			printf("SETUP_DATA_OUT_DONE Stage(%d/%d)\n\r",(U16_T)USB_PDevice[devinx].Hc.PControl.Current_Length,
			(U16_T)(U16_T)USB_PDevice[devinx].Hc.PControl.Total_Length);
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
			break;
		case PASSTHROUGH_DC_SETUP_STATUS_IN: //DC side Setup Status Stage
#if (USB_PASSTHROUGH_DEBUG_MODE)
			printf(" @P@ DC Status IN\n\r");
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
			USBDC_VIRTUAL_Control_Statue_In(devinx,pid);
			Result = USB_SUCCESS;
#if (USB_PASSTHROUGH_DEBUG_MODE)
			printf("@P_IN@ PassThrough Complete \n\r");
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
			break;
		case PASSTHROUGH_DC_SETUP_STALL: //DC side Setup Status Stage
			USBDC_HAL_Endp_Stalled(pid,devinx,0);
			Result = USB_SUCCESS;
			break;
		case PASSTHROUGH_COMPLETE: //PassThroug Complete
#if (USB_PASSTHROUGH_DEBUG_MODE)
			printf("@P@ PassThrough Complete \n\r");
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)
			Result = USB_SUCCESS; // ternimate
			break;
	}
	
	if (Result == USB_SUCCESS)
	{
#if (USB_PASSTHROUGH_DEBUG_MODE)
		printf("@@ Done=>TASK_USBHC_PassThrough Complete\n\r");
#endif //#if (USB_PASSTHROUGH_DEBUG_MODE)

#if (SYSTEM_MSC_DEVICE_SUPPORT)
		if (devinx == USBHC_MSC_Devinx)
		{
			if (USBHC_MSC_State & MSC_STATE_CHECK_BULK_IN)
			{
				if ((USB_PDevice[devinx].Hc.PControl.Setup.b.bmRequestType == ENDPOINT_RECIPIENT) &&
					(USB_PDevice[devinx].Hc.PControl.Setup.b.bRequest == CLEAR_FEATURE) &&
					(USB_PDevice[devinx].Hc.PControl.Setup.b.wValue.w == 0x0000))
				{
					//should create a IN after passthrough complete
					USBHC_MSC_State &= ~MSC_STATE_CHECK_BULK_IN;
					if (USBHC_MSC_State & MSC_STATE_BULK_IN)
					{
						USBHC_MSC_State &= ~MSC_STATE_BULK_IN;
#if (SYSTEM_USB_HC_BURST)
						USBHC_MSC_CSW_After_Control(devinx);
#endif /* (SYSTEM_USB_HC_BURST) */
					}
				}
			}
		}
#endif //#if (SYSTEM_MSC_DEVICE_SUPPORT)
	}
	else if (Result == USB_ERROR)
	{
		USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_FAIL;
#if (USB_PASSTHROUGH_DEBUG_MODE)
		printf("@@ Ternimate=>PASSTHROUGH_FAIL\n\r");
#endif
	}
	else
	{
		terminate = 0;
	}

	if (terminate)
	{
		USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
#if (USB_PASSTHROUGH_DEBUG_MODE)
		printf("@P@ TD_Free:%d\n\r",(U16_T)USB_PDevice[devinx].Hc.Control.TdNum);
#endif		
		USBHC_CORE_Free_TD(USB_PDevice[devinx].Hc.Control.TdNum);
		USBDC_Device[devinx].PassThroughState[pid] = PASSTHROUGH_IDLE;
		USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_IDLE;
		malloc_free(USB_PDevice[devinx].Hc.Control.Buf);
		USB_PDevice[devinx].Hc.Control.Buf = 0;
		USB_PDevice[devinx].Hc.gState = USB_PDevice[devinx].Hc.PassThrough_gState_bk;
	}
}

/*******************************************************************************
* void USBHC_Remove_Device(U8_T devinx,  U8_T portReset)
* Description: Remove the virtual device in HC & DC
* Input      :  
* Output     : None.
* Return     : None
*******************************************************************************/
void USBHC_Remove_Device(U8_T devinx, U8_T portReset)
{
	U8_T	upperlayer, port, para;
	
	/*Check the upper layer */
	upperlayer = USB_PDevice[devinx].UpperHubDevinx;
	port = USB_PDevice[devinx].Hub_NbrPorts USBHC_ROOTHUBPORT_PORT_NUM_READ;
	(USB_PDevice[upperlayer].HUB.ReportState+port)->Devinx = 0; // clear
	USBHC_HUB_ReleaseAddress(USB_PDevice[devinx].Addr & USBHC_DEVICE_ADDR_MASK);	
	TASK_USBHC_HUB_RemoveDevinx_Fork(devinx);
	if (portReset)
	{
		if (upperlayer == USBDC_VHUB_DEVINX) //it is root hub
		{		
			USBHC_RootHub_Reset_Port(port,150);
		}
		else
		{		
			para = (port << 4) | PORT_RESET;
			USB_PDevice[upperlayer].Hc.ScmdStep = SCMD_SET_HUB_FEATURE;
			//printf("HubFeature:Reset,devinx=%bd,port=%bd\n\r", devinx, port);
			TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_Scm_Handle_Start_ID,0,upperlayer,para,0);		
		}
	}
}

/*----------------------------------------------------------------------------
 * RESULT USBHC_SCM_SetHubFeature(U8_T devinx,U8_T td_id,U8_T mode)
 * Purpose : Perform the set address operation of enumearation
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
RESULT USBHC_SCM_SetHubFeature(U8_T devinx,U8_T td_id,U8_T para2)
{
	RESULT	Result=USB_ERROR;
	U8_T	portid,feature;
	
	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		portid  = (para2 & 0xf0) >> 4;
		feature = para2 & 0x0f;
		USBHC_Class_HubFeature(devinx,td_id,portid,SET_FEATURE,feature);
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
/*----------------------------------------------------------------------------
 * RESULT USBHC_SCM_ClearHubFeature(U8_T devinx,U8_T td_id,U8_T mode)
 * Purpose : Perform the set address operation of enumearation
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
RESULT USBHC_SCM_ClearHubFeature(U8_T devinx,U8_T td_id,U8_T para2)
{
	RESULT	Result=USB_ERROR;
	U8_T	portid,feature;
	
	if (USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE)
	{
		portid  = (para2 & 0xf0) >> 4;
		feature = para2 & 0x0f;
		USBHC_Class_HubFeature(devinx,td_id,portid,CLEAR_FEATURE,feature);
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
/*----------------------------------------------------------------------------
 * RESULT USBHC_Enumerate_Condition_Release(U8_T devinx)
 * Purpose : Release the consition when in hc emulation stage.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
void USBHC_Enumerate_Condition_Release(U8_T devinx)
{
	U8_T	upper_devinx;
	
	//Release Root Hub Enumeration.
	if ((USB_PDevice[devinx].Addr & 0x7f) <= 4)	
	{		 
		 USBHC_RootHub_Bus_Occupy_Flag = 0;
	}	
	else
	{
		//Check the upper class whether is HUB
		upper_devinx = USB_PDevice[devinx].UpperHubDevinx;
		if (USB_PDevice[upper_devinx].DevClass == USB_HUB_CLASS)
		{
			if (upper_devinx >= USBDC_VIRTUAL_DEVINX)
			{
				USB_PDevice[upper_devinx].Hub_Enu_Port_Id = 0;
			}
		}
	}
	USBHC_HUB_LogNewAddress();
	USBHC_HUB_ReleaseEnumerateOperation();
}

/*----------------------------------------------------------------------------
 * void USBHC_Active_New_Device(U8_T devinx,U8_T mount_port)
 * Purpose : After enumerate in Host port, then do the report new device
 *           plug in operation.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
void USBHC_Active_New_Device(U8_T devinx,U8_T mount_port)
{
	U8_T	roothubport;
	U8_T    upperdevice;
	U8_T    hcportid;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	U8_T    port_active;
#endif
	
	/* Skip task if the device is gone */
	if ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00)  // Device is gone  
	{			
		//printf("Mount:Devinx=0,Skip\n\r");
		return;	
	}
		
	/* Create the vritual devcie */
	if (USBDC_VirtualDevice_Create(devinx) == 0)
	{
		return;
	}
	
	/* Build up the HID Polling Table & Interrupt TD Table */
#if (SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT)
	if (USB_PDevice[devinx].DevClass != USB_MSC_CLASS)
	{
		USBHC_CORE_Build_INTL_Polling(devinx);
	}
#else
	USBHC_CORE_Build_INTL_Polling(devinx);
#endif //#if (SYSTEM_MSC_DEVICE_SUPPORT)

	/* Build up the Isochronous Table & ISTL TD Table */
	USBHC_CORE_Build_ISTL_Polling(devinx);

	/* Release Root Hub Enumeration */
#if (SYSTEM_EXTENDER_TRANSMITTER)
	if ((USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK) == 0x00)
#endif /* (SYSTEM_EXTENDER_TRANSMITTER) */
	{
		USBHC_Enumerate_Condition_Release(devinx);
	}

	/* Get upper device info */
	if (USB_PDevice[devinx].DevClass == USB_HUB_CLASS)
	{
		return;
	}

	/* Check DVR Setting */
#ifdef ONEHID
	if (devinx > USBDC_VHID_DEVINX)
	{			 					
		printf("> SKIP: (DC:HubP_Id=%bu,HubDevinx=%bu)(Devinx=%bx,HubPortId=%bu,Class=0x%02bx)(DC_Mask=0x%02bx)\n\r", roothubport, upperdevice, devinx,hcportid,USB_PDevice[devinx].DevClass,mount_port);
		return;
	}		
#endif
	hcportid = (USB_PDevice[devinx].Hub_NbrPorts & USBHC_ROOTHUBPORT_PORT_NUM_MASK) USBHC_ROOTHUBPORT_PORT_NUM_READ;
	USBHC_Get_Upper_Device_Info(devinx, &upperdevice, &roothubport);
	printf("> MOUNT: (DC:HubP_Id=%bu,HubDevinx=%bu)(Devinx=%bx,HubPortId=%bu,Class=0x%02bx)(DC_Mask=0x%02bx)\n\r", roothubport, upperdevice, devinx,hcportid,USB_PDevice[devinx].DevClass,mount_port);

	/* Mount the device */
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	if (devinx == USBHC_Audio_Devinx)
	{
#if (SYSTEM_EXTENDER_RECEIVER)
		if (mount_port & (~MOUNT_PORT[REMOTE_HOST_PORT]))
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */
		{
			port_active = (roothubport << 4 ) | upperdevice;
			USBHC_Audio_Enumerate_Processing_Flag = 1;
			USBHC_Audio_Enumerate_Task_Id = TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_USBHC_Audio_DC_Mount_Handle_ID,mount_port,port_active,0,2000);  //every 2 seconds check the idle state
		}
	}
	else
#endif //#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)		
	{
#if (SYSTEM_MSC_DEVICE_SUPPORT)
		if ((devinx == USBHC_MSC_Devinx) && (USBHC_MSC_Reset_Flag) && (VHUB_MSC_Reset_Flag))
		{
			USB_PDevice[devinx].MountPort |= mount_port;					
			USB_PDevice[devinx].Mounted    = VDEV_MOUNT;
			USBHC_MSC_VHUB_ResetUpdate();
			USBHC_MSC_Reset_Flag = 0;
		}
		else
#endif		
		{
			USBDC_Virtual_Hub_DeviceMount_Control(upperdevice,roothubport,VDEV_MOUNT,mount_port,devinx);
		}	
	}

#if (SYSTEM_EXTENDER_RECEIVER)
	if (USBHC_MSC_DismountFlag ==0)
	{	
		if (mount_port & MOUNT_PORT[REMOTE_HOST_PORT])
		{
			Externder_Receiver_DeviceMount_Control(devinx,VDEV_MOUNT,EXTENDER_USB_MOUNT);			
		}
	}	
	//else
	//{
	//	printf("MSC DismountFlagOn, Skip mount\n\r");
	//}	
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */
}

/*----------------------------------------------------------------------------
 * void USBHC_Remove_HID_Buffer(U8_T intt_id)
 * Purpose : After enumerate in Host port, then do the report new device9
 *           plug in operation.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
void USBHC_Remove_HID_Buffer(U8_T intt_id)
{
	U8_T	length,rp,newrp;
	
	rp = HC_IntTransfer_Table[intt_id].PassControl.Rp;	
	length = HC_IntTransfer_Table[intt_id].PassControl.Buf[rp]+1;
	if (HC_IntTransfer_Table[intt_id].PassControl.Wp > HC_IntTransfer_Table[intt_id].PassControl.Rp)
	{
		HC_IntTransfer_Table[intt_id].PassControl.Rp += (length);
#ifdef SYNC
		memset(HC_IntTransfer_Table[intt_id].PassControl.SyncRp,HC_IntTransfer_Table[intt_id].PassControl.Rp,KVM_MAX_PORT);
#endif		
	}
	else
	{
		//Get the end of this buffer
		newrp = HC_IntTransfer_Table[intt_id].PassControl.BufSize-HC_IntTransfer_Table[intt_id].PassControl.Rp;
		//Get hte data package end position from the index 0
		if ((length) > newrp) //if length is large than end postion
		{
			HC_IntTransfer_Table[intt_id].PassControl.Rp = (length) - newrp;
		}
		else
		{
			HC_IntTransfer_Table[intt_id].PassControl.Rp += (length);
			if (HC_IntTransfer_Table[intt_id].PassControl.Rp >= HC_IntTransfer_Table[intt_id].PassControl.BufSize)
				HC_IntTransfer_Table[intt_id].PassControl.Rp -= HC_IntTransfer_Table[intt_id].PassControl.BufSize;
		}
	}
}

/*-------------------------------------------------------------------------------
 * void TASK_USBHC_HUB_RemoveDevinx_Fork(U8_T devinx)
 * Purpose : The short verson of TASK_Active of TASK_USBHC_HUB_RemoveDevinx
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
void TASK_USBHC_HUB_RemoveDevinx_Fork(U8_T devinx)
{
	TASK_USBHC_HUB_RemoveDevinx_Task.Task_Para = devinx;
	TASK_Run(&TASK_USBHC_HUB_RemoveDevinx_Task);
}


/*-------------------------------------------------------------------------------
 * void TASK_USBHC_Device_Reset(U8_T port,U8_T devinx)
 * Purpose : The task which handle the device reset
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
void TASK_USBHC_Device_Reset(U8_T port,U8_T devinx)
{
	if (USB_PDevice[devinx].UpperHubDevinx == USBDC_VHUB_DEVINX) //this is the root hub
	{
		TASK_USBHC_HUB_RemoveDevinx_Fork(devinx); //clear the msc device information in DC & HC				
		if ((USBDC_Device[devinx].VirHubNum[port]) >= USBDC_VHUB_PORT_NUM) // this should not happened
		{
			//printf("VHub portNo is invalid.\n\r");
			return;
		}
		USBHC_RootHub_Bus_Occupy_ID	= USBDC_Device[devinx].VirHubNum[port]-1;
		USBHC_RootHub_Reset_Port(USBHC_RootHub_Bus_Occupy_ID,10);
		USBHC_RootHub_ResetCnt = 1;
		//1.Fill Up root hub port reset register
		USBHC_RootHub_Bus_Occupy_Flag = 1;
		//USBHC_RootHub_Bus_Occupy_ID = USBDC_Device[devinx].VirHubNum[port]-1;
		USB_PDevice[devinx].Addr = USBHC_DEVICE_USED_MASK; // reset the device
		USBHC_INTL_SkipMap_Control(devinx,1);
	}
	else
	{
#if 0
		// the downstream hub port
		USB_PDevice[devinx].Hub_Enu_Port_Id = USB_PDevice[devinx].Hub_NbrPorts USBHC_ROOTHUBPORT_PORT_NUM_READ;
		port = USB_PDevice[devinx].Hub_Enu_Port_Id;
		//printf("HubPort:%bx,reset perform\n\r",USB_PDevice[devinx].Hub_Enu_Port_Id);
		USB_PDevice[devinx].Hc.HUB_EnumState = HUB_ENUM_IDLE;
		TASK_Active(TASK_TYPE_INTERVAL_MS, TASK_USBHC_HUB_Port_Reset_ID, devinx, port, 10, 10);
#else      
		USBHC_Remove_Device(devinx, 1);
#endif
	}
}

/*-------------------------------------------------------------------------------
 * void USBHC_INTL_SkipMap_Control(U8_T devinx,U8_T value)
 * Purpose : The task which handle the device reset
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
void USBHC_INTL_SkipMap_Control(U8_T devinx,U8_T value)
{
	U8_T	index;
	
	for (index=0; index < USB_HC_INTL_MAX ; index++)
	{
		if (HC_IntTransfer_Table[index].Used_DeviceID & USBHC_TD_USED_MASK)
		{
			if ((HC_IntTransfer_Table[index].Used_DeviceID & USBHC_TD_DEVICE_ADDR_MASK) == devinx)
			{
				USBHC_CORE_SetBit(TD_INTL_Skip_Map,index,value);
			}
		}
	}
	USBHC_Write_Regs(HC_INTL_TDSKIP_MAP_REG,TD_INTL_Skip_Map,4);
}

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
/**
* @brief  TASK_USBHC_Audio_DC_Mount_Handle
*         Mount the upstream port
* @param  None 
* @retval None
*/
void TASK_USBHC_Audio_DC_Mount_Handle(void)
{
	U8_T mount_port;
	U8_T roothubport;
	U8_T port_active;
	U8_T devinx;
	U8_T uport;
	U8_T upperdevice;
	
	TASK_Destory_Current();
	
	mount_port  = TASK_Event & 0x0f;
	devinx 		= (TASK_Register0 & 0x0f);
	roothubport = (TASK_Register0 & 0xf0) >> 4;
    
	//Skip task if the device is gone
	if ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00)  // Device is gone  
	{
		USBHC_Audio_Enumerate_Processing_Flag = 0; // no more enumeration.
		return;	
	}    

	for (uport = 0; uport < KVM_MAX_PORT ; uport++)
	{
		port_active = (0x01 << uport);
	    if (mount_port & port_active) 
	    {
			USBHC_Get_Upper_Device_Info(USBHC_Audio_Devinx,&upperdevice,&roothubport);
			USBDC_Virtual_Hub_DeviceMount_Control(upperdevice,roothubport,VDEV_MOUNT,port_active,USBHC_Audio_Devinx);
			mount_port &= ~port_active;
			
UpstreamMount_Handle_Start:
			if (mount_port)
			{		
				uport++;						
				if (uport < KVM_MAX_PORT)
				{	
					port_active = (0x01 << uport);
					if (mount_port & port_active) 
					{	
						port_active = (roothubport << 4 ) | devinx;
						USBHC_Audio_Enumerate_Task_Id = TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_USBHC_Audio_DC_Mount_Handle_ID,mount_port,port_active,2000,2000);  //every 3 seconds check the idle state
						return;
					}
					else
					{
						goto UpstreamMount_Handle_Start;				
					}
				}	
			}
		}		
	}	
	USBHC_Audio_Enumerate_Processing_Flag = 0; // no more enumeration.
}
#endif //#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)

/**
* @brief  USBHC_Clone_PDevice
*         move the hub device to a new location
* @param  None 
* @retval None
*/
U8_T USBHC_Clone_PDevice(U8_T old_devinx,U8_T endp2direction)
{
	U8_T upperdevinx,upperhubport;
	U8_T index;
	U8_T start=USB_HC_MAX_DEVICE,end=USB_HC_MAX_DEVICE + USB_HC_MAX_HUB;
	
	if (endp2direction)
	{
		//check weather it is device 6 or 7 already
		if (old_devinx >= 6)
			return 1;
		start = 6;
		end = USB_HC_MAX_DEVICE;			
	}		
	
	for (index=start; index < end; index++)
	{
		if (USB_PDevice[index].Addr & USBHC_DEVICE_USED_MASK)
		{
			continue;
		}

		memcpy(&USB_PDevice[index],&USB_PDevice[old_devinx],sizeof(USB_PDevice_TypeDef));
		memset(&USB_PDevice[old_devinx],0x00,sizeof(USB_PDevice_TypeDef));
		
		if (endp2direction==0)
		{	
			USBHC_Current_Total_Device_Count--;
			USBHC_Current_Total_Hub_Count++;
		}	
#if (SYSTEM_EXTENDER_RECEIVER)			
#ifdef PWAYTEK_01		
		ExtenderR_Maintain_DeviceState_Led();
#endif		
#endif
		//Adjust the root hub devinx handle table
		upperdevinx = USB_PDevice[index].UpperHubDevinx; //get upperhub devinx
		upperhubport = (USB_PDevice[index].Hub_NbrPorts & USBHC_ROOTHUBPORT_PORT_NUM_MASK) USBHC_ROOTHUBPORT_PORT_NUM_READ; //get port
		if (upperdevinx == 0)
		{
			USBHC_RootHub_Devinx[upperhubport] = index;
		}
		(USB_PDevice[upperdevinx].HUB.ReportState+upperhubport)->Devinx = index;

		return index;
	}
#if (KVM_BUZZER_SUPPORT)
	BUZZER_Script_Active(DeviceErrorSound);
#endif //#if (KVM_BUZZER_SUPPORT)
	//printf("> KVM ERROR:No Free PDevice, Skip operation\n\r");
	
	return 0;
}

/**
* @brief  USBHC_Get_Upper_Device_Info
*
* @param  None 
* @retval None
*/
void USBHC_Get_Upper_Device_Info(U8_T devinx,U8_T *upper_dev,U8_T *upper_port)
{
	*upper_dev = 0;
	*upper_port = USBDC_Virtual_Hub_Map[devinx].Devinx_HubPort;	
}

/*----------------------------------------------------------------------------
 * void USBHC_PassThrough_Setup_Condition_Reset(U8_T devinx,U8_T pid)
 * Purpose: Reset all the passthrough coditioin
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void USBHC_PassThrough_Setup_Condition_Reset(U8_T devinx,U8_T pid)
{
	/* Free all buffer & flag */
	USB_PDevice[devinx].Hc.Control.State = CTRL_IDLE;
	USB_PDevice[devinx].Hc.gState = USB_PDevice[devinx].Hc.PassThrough_gState_bk;
	USBDC_Device[devinx].PassThroughState[pid] = PASSTHROUGH_IDLE;
	malloc_free(USB_PDevice[devinx].Hc.Control.Buf);
	USB_PDevice[devinx].Hc.Control.Buf = 0;
	USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_IDLE;
}

/*----------------------------------------------------------------------------
 * void USBHC_InterruptTD_Transfer_Index_Init(U8_T intt_id)
 * Purpose: Init the interrupt TD transver index
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void USBHC_InterruptTD_Transfer_Index_Init(U8_T intt_id)
{
	HC_IntTransfer_Table[intt_id].PassControl.Rp = 0; 
	HC_IntTransfer_Table[intt_id].PassControl.Wp = 0; 
#ifdef SYNC
	memset(HC_IntTransfer_Table[intt_id].PassControl.SyncRp,0x00,USBDC_PORT_MAX);
	memset(HC_IntTransfer_Table[intt_id].PassControl.SyncWp,0x00,USBDC_PORT_MAX);						
#endif					
}	
/*----------------------------------------------------------------------------
 * U8_T USBHC_KB_Led_Check(U8_T devinx)
 * Purpose: Check weither contain the LED output 
 * Params :
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
U8_T USBHC_KB_Led_Check(U8_T devinx)
{
#if defined(HID_PARSER_CORE_KB)		
	if (USB_PDevice[devinx].HID_LED_Item != NULL)	
	{			
		return 1;
	}		
	return 0;
#else
	HID_Rpt_Element *item;

	item = &USB_PDevice[devinx].HID_Kb_ParserTable->Item[HID_RPT_LED];
	if (item->Rpt_Size)
	{
		return 1;
	}		
	return 0;
#endif
}
/* End of usbhc.c */
