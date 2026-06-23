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
 * Module Name : usbhc_audio.c
 * Purpose     : The Audio handle program for USB HC interface 
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 * -------------------------------------------------------------------------------
 * 
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	<string.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	"project_include.h"

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
/* GLOBAL VARIABLE DECLARATIONS */
bit	USBHC_Audio_IN_Flag;
bit	USBHC_Audio_IN_Start_Flag;
bit	USBHC_Audio_Enumerate_Processing_Flag;
//bit USBHC_Audio_Switch_In_Processing_Flag;
bit USBHC_Audio_Hotkey_Sw_Flag;
U8_T	USBHC_Audio_Enumerate_Task_Id;
U8_T	USBHC_Audio_Device_Cnt;
U8_T	USBHC_Audio_In_Endpinx;
U8_T	USBHC_Audio_Out_Endpinx;
U8_T	USBHC_Audio_In_Interface;
U8_T	USBHC_Audio_Out_Interface;
U8_T	USBHC_Audio_Devinx;
U8_T    TASK_USBHC_Audio_Switch_ID;
U8_T    TASK_USBHC_Audio_Switch_Check_TaskID;

/* LOCAL SUBPROGRAM DECLARATIONS */
void USBHC_AUDIO_Umount_Control(U8_T);
void USBHC_AUDIO_Mount_Control(U8_T port);
void TASK_USBHC_Audio_Switch(void);
void TASK_USBHC_Audio_Switch_Check(void);
/* EXTERNAL GLOBAL VARIABLE DECLARATIONS */


/* LOCAL SUBPROGRAM BODIES */

/*----------------------------------------------------------------------------
 * void USBHC_AUDIO_Init(void)
 * Purpose : 1.Init Memory 
 * Params  : none
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------*/
void USBHC_AUDIO_Init(void)
{
	U8_T	reg;

	//Setting the ISO Sof offset Source
	reg = 0xa0;
	USBHC_Write_Regs(HC_SOF_OFFSET_REG,&reg,1);
	//Setting the ISO Sof Source
	reg = HC_SOF_SELECT_USPSE_SET;
	USBHC_Write_Regs(HC_SOF_SELECT_REG,&reg,1);	
	USBHC_Audio_Device_Cnt = 0;
	USBHC_Audio_IN_Flag = 0;
	USBHC_Audio_IN_Start_Flag = 0;
	USBHC_Audio_In_Endpinx = 1;
	USBHC_Audio_Out_Endpinx= 1;
	USBHC_Audio_Devinx = 0;	
	USBHC_Audio_Enumerate_Processing_Flag = 0;
	//USBHC_Audio_Switch_In_Processing_Flag = 0;
	USBHC_Audio_Hotkey_Sw_Flag			  = 0;	
	TASK_USBHC_Audio_Switch_ID = TASK_Create(TASK_USBHC_Audio_Switch);
	TASK_USBHC_Audio_Switch_Check_TaskID = TASK_Create(TASK_USBHC_Audio_Switch_Check);

#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("TASK_USBHC_Audio_Switch_ID=%bu\n\r",TASK_USBHC_Audio_Switch_ID);
#endif	

} /* End of USBHC_AUDIO_Init */

/*----------------------------------------------------------------------------
 * void USBHC_AUDIO_Sync(U8_T up)
 * Purpose : 1.Init Memory 
 * Params  : none
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------*/
void USBHC_AUDIO_Sync(U8_T up)
{
	U8_T	reg;
	
	reg = HC_SOF_SELECT_USPSE_SET | (up & 0x03);
	USBHC_Write_Regs(HC_SOF_SELECT_REG,&reg,1);	
}  

/*----------------------------------------------------------------------------
 * void USBHC_AUDIO_Sync_Disable(void)
 * Purpose : 1.Init Memory 
 * Params  : none
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------*/
void USBHC_AUDIO_Sync_Disable(void)
{
	U8_T	reg;
		
	reg = 0;
	USBHC_Write_Regs(HC_SOF_SELECT_REG,&reg,1);	
		
} 

/*----------------------------------------------------------------------------
 * void USBHC_AUDIO_Switch(U8_T next_usb_audio)
 * Purpose : 1.Init Memory 
 * Params  : none
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------*/
void USBHC_AUDIO_Switch(U8_T next_usb_audio)
{
	//1.Get releative information.		
	if (USBHC_Audio_Device_Cnt)
	{		
		if (KVM_CurrentUSBAudio != next_usb_audio) // changed active port id
		{						
			//printf("> KVM: USBHC Switch Audio to Target:%bu\n\r",next_usb_audio);
			USBHC_AUDIO_Sync_Disable();
			USBHC_AUDIO_Umount_Control(KVM_CurrentUSBAudio);
			TASK_USBHC_Device_Reset(KVM_CurrentUSBAudio,USBHC_Audio_Devinx);
			//USBHC_Audio_Switch_In_Processing_Flag = 1;
			KVM_NextAudio = next_usb_audio;
			KVM_CurrentUSBAudio = next_usb_audio;						
		}
	}

}

/*----------------------------------------------------------------------------
 * void TASK_USBHC_Audio_Switch(void)
 * Purpose : 1.Init Memory 
 * Params  : none
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------*/
void TASK_USBHC_Audio_Switch(void)
{		
	//printf("TASK Audio Switch\n\r");
	if (USBHC_Audio_Device_Cnt)
	{
		if (KVM_CurrentUSBAudio != KVM_CurrentHost) // changed active port id
		{			
			//USBHC_AUDIO_Switch(KVM_CurrentHost);
			USBHC_KVM_USB_Audio_Port_Sw(KVM_CurrentHost);
		}
						
		//if (KVM_CurrentUSBAudio != KVM_CurrentHost) // changed active port id
		if (KVM_CurrentUSBAudio != KVM_NextAudio) // changed active port id
		{						
			//USBHC_AUDIO_Switch(KVM_CurrentHost);
			KVM_CurrentUSBAudio = KVM_NextAudio;				
			//printf("Set Current USB Audio:%bu\n\r",KVM_CurrentUSBAudio);
		}	
	}	
	//USBHC_Audio_Switch_In_Processing_Flag = 0;
	TASK_Destory_Current();	
}


/*------------------------------------------------------------------------------
 void TASK_USBHC_Audio_Switch_Check(void)
 @brief  After KVM switching complete,check the value with curretn port setting
         [1].Protocol
         [2].COnfiguration 
 @param  $port:current active kvm port number
 @retval None
--------------------------------------------------------------------------------*/
void TASK_USBHC_Audio_Switch_Check(void)
{
	if (USBHC_Audio_Device_Cnt)
	{
		if ((USBHC_RootHub_Bus_Occupy_Flag == 0) && (USBHC_HUB_Enumerate_Occupy_Flag==0))
    	{	
			USBHC_AUDIO_Switch(TASK_Register0);				
		}
		else
		{
			return;		
		}
	}
	TASK_Destory_Current();				
}

/*----------------------------------------------------------------------------
 * void USBHC_AUDIO_In_Start(void)
 * Purpose : Start the Audio Service
 * Params  : none
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------*/
//#if (SYSTEM_AUDIO_IN_SUPPORT)
void USBHC_AUDIO_In_Start(void)
{
	//Check the Audio In Flag
	if (USBHC_Audio_IN_Flag)
	{
		//Fill the TD header
		HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte0_Actual_Byte = 0;
		HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte2_MaxPKT_Size = USB_HC_ISTL_BUF_SIZE - 2;
		HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte3 &= 0xFC;
		HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte4_Total_Byte = USB_HC_ISTL_BUF_SIZE - 2; //get the byte will not over the bufer size
		HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte5 &= 0xFC;
		HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte7 = 1;
		HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte1 |= TD_ACTIVED_MASK;
		
		//Clear Done Map
		EXTINT2_DISABLE;
		_USBHC_HCDR_SFR(USB_HC_ISTL_IN_DONE_MASK);
		_USBHC_HCCIR_SFR(HC_ISTL_TDDONE_MAP_REG);
		
		//Clear Skip Map
		TD_ISTL_Skip_Map &= ~(USB_HC_ISTL_IN_DONE_MASK);
		_USBHC_HCDR_SFR(TD_ISTL_Skip_Map);
		_USBHC_HCCIR_SFR(HC_ISTL_TDSKIP_MAP_REG);
		EXTINT2_ENABLE;
		
		USBHC_Audio_IN_Start_Flag = 1;
	}
}
//#endif /* End of SYSTEM_AUDIO_IN_SUPPORT */

/*----------------------------------------------------------------------------
 * void USBHC_AUDIO_In_Stop(void)
 * Purpose : Stop the Audio Service
 * Params  : none
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------*/
void USBHC_AUDIO_In_Stop(U8_T pid)
{
	if ((pid == KVM_CurrentUSBAudio) &&
		(HC_IsoTransfer_Table[USB_HC_ISTL_OUT_MAX].Used_DeviceID & USBHC_DEVICE_USED_MASK))
	{
		EXTINT3_DISABLE;
		HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte1 &= ~TD_ACTIVED_MASK;
		EXTINT3_ENABLE;
	}
	
	USBHC_Audio_IN_Start_Flag = 0;
}

/*----------------------------------------------------------------------------
 * void USBHC_AUDIO_Umount_Control(U8_T port)
 * Purpose : Start the Audio Service
 * Params  : none
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------*/
void USBHC_AUDIO_Umount_Control(U8_T port)
{		
	U8_T 	upper_devinx;
	U8_T 	hubportnumber;

	//1.Get releative information.
	USBHC_Get_Upper_Device_Info(USBHC_Audio_Devinx,&upper_devinx,&hubportnumber);

	//2.Dismount the device from virtual DC
	USBDC_Virtual_Hub_DeviceMount_Control(upper_devinx,hubportnumber,VDEV_UNMOUNT,MOUNT_PORT[port],USBHC_Audio_Devinx);
	
	//3.Disable the address now
	USBDC_REGS_Device_Reset(port,USBHC_Audio_Devinx);			
}

/*----------------------------------------------------------------------------
 * void USBHC_AUDIO_Mount_Control(U8_T upper_devinx,U8_T hubportnumber)
 * Purpose : Start the Audio Service
 * Params  : none
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------*/
void USBHC_AUDIO_Mount_Control(U8_T port)
{	
	U8_T 	upper_devinx;
	U8_T 	hubportnumber;

	USBHC_Get_Upper_Device_Info(USBHC_Audio_Devinx,&upper_devinx,&hubportnumber);

	(USB_PDevice[upper_devinx].HUB.ReportState+hubportnumber)->Devinx = USBHC_Audio_Devinx;
	USBDC_Virtual_Hub_DeviceMount_Control(upper_devinx, hubportnumber, VDEV_MOUNT, MOUNT_PORT[port],USBHC_Audio_Devinx);
}

#if (ENABLE_HOTKEY_FUN_USBAUDIO_REPORT && SYSTEM_AUDIO_MAX)
#if (KVM_HOTKEY_SUPPORT)
/*----------------------------------------------------------------------------
 * void USBHC_AUDIO_ReportModeChanged_Mount_Control(U8_T reportmode)
 * Purpose : usb audio device mounting control
 * Params  : none
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------*/
void USBHC_AUDIO_ReportModeChanged_Mount_Control(U8_T reportmode)
{	
	U8_T index;	
	U8_T mount_port;
	U8_T port_active;
	U8_T roothubport;

	if (USBHC_Audio_Device_Cnt) //if usb audio device has been plug in
	{	
		if (reportmode == USBAUDIO_REPORT_ALL)
		{
			roothubport = (USB_PDevice[USBHC_Audio_Devinx].Hub_NbrPorts & USBHC_ROOTHUBPORT_PORT_NUM_MASK) USBHC_ROOTHUBPORT_PORT_NUM_READ;
			port_active = (roothubport << 4 ) | USB_PDevice[USBHC_Audio_Devinx].UpperHubDevinx;
			USBHC_Audio_Enumerate_Processing_Flag = 1;		
			mount_port = UDC_PORT_ALL_ACT;
			mount_port &= ~MOUNT_PORT[KVM_CurrentUSBAudio]; //not mount curent audio

			USBHC_Audio_Enumerate_Task_Id = TASK_Active(TASK_TYPE_INTERVAL_MS,
														TASK_USBHC_Audio_DC_Mount_Handle_ID,
														mount_port,
														port_active,
														0,
														2000);  //every 2 seconds check the idle state

		}
		else
		{
			//2.Cancle all the mount except current active one
			for (index = 0; index < KVM_MAX_PORT; index++)
			{
				if (index != KVM_CurrentUSBAudio) //only umount the port is not current active
				{
					USBHC_AUDIO_Umount_Control(index);
				}
			}
		}
	}
}
#endif /* KVM_HOTKEY_SUPPORT */
#endif /* ENABLE_HOTKEY_FUN_USBAUDIO_REPORT && SYSTEM_AUDIO_MAX */

/*---------------------------------------------------------------------------------
 * void USBHC_AUDIO_In_Control_Check(U8_T devinx,U8_T intf_id,U8_T configuration)
 * Purpose :  
 * Params  : none
 * Returns : none
 * Note    : none
 *--------------------------------------------------------------------------------*/
void USBHC_AUDIO_In_Control_Check(U8_T devinx,U8_T intf_id,U8_T configuration)
{
	if (devinx == USBHC_Audio_Devinx)
	{
		if (intf_id == USBHC_Audio_In_Interface)
		{
			if (configuration == 0) //turn off the IN
			{
				USBHC_AUDIO_In_Stop(KVM_CurrentUSBAudio);
			}
			else
			{		
				USBHC_AUDIO_In_Start();
			}				
		}	
	}		
}

/*----------------------------------------------------------------------------
 * void USBHC_KVM_USB_Audio_Port_Sw(U8_T newport)
 * Purpose : 
 * Params  : none
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------*/
void USBHC_KVM_USB_Audio_Port_Sw(U8_T newport)
{
	TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_USBHC_Audio_Switch_Check_TaskID,0,newport,1,1);	
}
#endif /* End of SYSTEM_USBAUDIO_DEVICE_SUPPORT */

/* End of usbhc_audio.c */


