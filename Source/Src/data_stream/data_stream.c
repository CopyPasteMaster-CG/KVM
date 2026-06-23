/*
 ************************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ************************************************************************************
 */
 
/*================================================================================
 * Module Name : data_stream.c
 * Purpose     : handle keyboard & mouse data streaming
 * Author      : Jack Wang
 * Date        :
 * Notes       : None
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include <stdio.h>
#include <string.h>

#include "project_include.h"

#if (SYSETM_DATA_STREAM_CONTROL_ENABLE)  
/*$GLOBAL DEFINATION DECLARATIONS*/
/*$GLOBAL CONTANT DECLARATIONS*/
/*$GLOBAL VARIABLES DECLARATIONS*/ 
//---------------------------------------------------------------------------------------
// GENERIC USB QUEUE
U8_T TASK_DATAST_USB_Virtual_HID_ID;
//---------------------------------------------------------------------------------------
/*$STATIC VARIABLE DECLARATIONS  */
/*$LOCAL SUBPROGRAM DECLARATIONS */
void TASK_DATAST_USB_Virtual_HID_Output_Handle(void);
void DATAST_Move_Intr_Buffer(U8_T currentHost,U8_T devinx,U8_T endpidx,U8_T intt_id);

/*
 * -------------------------------------------------------------------------------
 * void    DATAST_Init(void)
 * Purpose : Initializes the Data stream control 
 *       
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */
void DATAST_Init(void)
{
	//1.Genteric HID device init declartion
#if (PROJECT_USB_GENERIC_HID_ENABLE)
	DATAST_Reset_GenericQueue();
	TASK_DATAST_PS2_Generic_KB_ActiveFlag = 0;
	TASK_DATAST_PS2_Generic_MS_ActiveFlag = 0;

	DATAST_Generic_USB_MS_Report[0] = 0; // the report content is not valid
	TASK_DATAST_PS2_Generic_KB_ID  = TASK_Create(TASK_DATAST_PS2_Generic_KB_Output_Handle);
	TASK_DATAST_PS2_Generic_MS_ID  = TASK_Create(TASK_DATAST_PS2_Generic_MS_Output_Handle);
	TASK_DATAST_PS2_Generic_MS_ID2 = TASK_DATAST_PS2_Generic_MS_ID;
#endif

	//2.Virtual USB device init declartion.	
	TASK_DATAST_USB_Virtual_HID_ID = TASK_Create(TASK_DATAST_USB_Virtual_HID_Output_Handle);	

#if (SYSTEM_TASK_DUMP_SUPPORT)
#if (PROJECT_USB_GENERIC_HID_ENABLE)
	printf("TASK_DATAST_PS2_Generic_KB_ID=%bu\n\r",TASK_DATAST_PS2_Generic_KB_ID);
	printf("TASK_DATAST_PS2_Generic_MS_ID=%bu\n\r",TASK_DATAST_PS2_Generic_MS_ID);
#endif
	//2.Virtual USB device init declartion.
	printf("TASK_DATAST_USB_Virtual_HID_ID=%bu\n\r",TASK_DATAST_USB_Virtual_HID_ID);
#endif	
} /* End of DATAST_Init */

/***************************************************************************************************/
/*
**  Function Name: TASK_DATAST_PS2_Virtual_HID_Output_Handle(void)
**  Parameter    : 
**  Description  : 
**
**
*/
void TASK_DATAST_USB_Virtual_HID_Output_Handle(void)
{
	U8_T intt_id,devinx;	
	bit  wait_ms_event=0;
	U8_T currenthost;
	U8_T targethost;
	U8_T wp,rp;
	U8_T passwait;
#ifndef ONEHID	
	U8_T infid,endp_id,endp_addr,endpidx;
#endif	
	U8_T hid_inf;

	targethost = (TASK_Register0 & 0xc0) >> 6;
	hid_inf = TASK_Event & 0x0f;
	intt_id = TASK_Register0 & 0x3f;
	devinx = HC_IntTransfer_Table[intt_id].Used_DeviceID & USBHC_TD_DEVICE_ADDR_MASK;

	//printf("<dp=%bu,%bu>",targethost,TASK_Active_ID);
#ifdef ONEHID	
	if ((devinx < USBDC_VHID_DEVINX) || ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00)) 
#else
	if ((devinx < USBDC_VIRTUAL_DEVINX) || ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00)) 
#endif	
	{
//DATAST_USB_Virtual_HID_Output_Reset:	
#ifdef SYNC		
		if (targethost != KVM_CurrentHost)
		{	
			HC_IntTransfer_Table[intt_id].PassControl.SyncRp[targethost] = HC_IntTransfer_Table[intt_id].PassControl.Wp;
			HC_IntTransfer_Table[intt_id].PassControl.SyncWp[targethost] = HC_IntTransfer_Table[intt_id].PassControl.Wp;			
		}
		else					
#endif			
		{	
			HC_IntTransfer_Table[intt_id].PassControl.Rp = HC_IntTransfer_Table[intt_id].PassControl.Wp;
			//HC_IntTransfer_Table[intt_id].PassControl.Rp = 0; // check if there is data needed processed
		}			
		goto TASK_DATAST_USB_Virtual_HID_Output_Handle_Exit2;
	}

	/* 1.Check Task Ternimate Condition */
#ifdef SYNC
	passwait = HC_IntTransfer_Table[intt_id].PassControl.SyncState & BIT_MASK[targethost+4];
	if (targethost != KVM_CurrentHost)
	{	
		wp = HC_IntTransfer_Table[intt_id].PassControl.SyncWp[targethost];
		rp = HC_IntTransfer_Table[intt_id].PassControl.SyncRp[targethost];		
	}
    else
	{	
		wp = HC_IntTransfer_Table[intt_id].PassControl.Wp;
		rp = HC_IntTransfer_Table[intt_id].PassControl.Rp;		
	}
#else	
	passwait = HC_IntTransfer_Table[intt_id].PassControl.State & PASSTHROUGH_WAIT;
	wp = HC_IntTransfer_Table[intt_id].PassControl.Wp;
	rp = HC_IntTransfer_Table[intt_id].PassControl.Rp;		
#endif /* #ifdef SYNC */	
	if (rp == wp) // check if there is data needed processed
	{				
		goto TASK_DATAST_USB_Virtual_HID_Output_Handle_Exit2;
	}
	
	if (TASK_USBHC_KVM_Switch_Check_Start_Flag)
	{
		if (TASK_Type != TASK_TYPE_INTERVAL_MS)
			TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_DATAST_USB_Virtual_HID_ID,hid_inf,TASK_Register0,1,1); // now active the Generic KB Output Handle			
		return;
	}

#if (SYSTEM_EXTENDER_RECEIVER)
	if ((KVM_CurrentHost == REMOTE_HOST_PORT) && (ExtenderR_Check_Transmitter_Plug_State()==0))
	{		
		goto DATAST_USB_Virtual_HID_Output_Reset;		
	}
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */

#if (SYSTEM_EXTENDER_RECEIVER)
	if (KVM_CurrentHost == REMOTE_HOST_PORT)
	{
#ifdef SYNC		
		if (HC_IntTransfer_Table[intt_id].PassControl.SyncState & BIT_MASK[KVM_CurrentHost+4])
#else
		if (HC_IntTransfer_Table[intt_id].PassControl.State & PASSTHROUGH_WAIT)
#endif		
		{	
			wait_ms_event = 1;
		}
		else
		{
			ExtenderR_Interrupt_Transfer_In_Data_Handle(devinx,intt_id,hid_inf);
		}
		goto TASK_DATAST_USB_Virtual_HID_Output_Handle_Exit2;
	}
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */
	
#ifdef SYNC	
	currenthost = targethost;	
#else	
	currenthost = KVM_CurrentHost;	
#endif	/* #ifdef SYNC	 */
	/*@@Check Port Suspend & Wakeup condition */
#ifdef ONEHID
	if (USBDC_Check_Upstream_Suspend_State(currenthost,USBDC_VHID_DEVINX))
#else	
	if (USBDC_Check_Upstream_Suspend_State(currenthost,devinx))
#endif			
	{
		wait_ms_event = 1;	
		goto TASK_DATAST_USB_Virtual_HID_Output_Handle_Exit2;
	}
	
	// if buffer has been filled and wait for send to host
#ifdef ONEHID	
	if ((USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_KB_ENDP_NUM] & USBDC_UPORT[currenthost]) == 0)
#else			
	if (passwait)
#endif		
	{
		wait_ms_event = 1;		
		goto TASK_DATAST_USB_Virtual_HID_Output_Handle_Exit2;
	}

#ifdef ONEHID	
	if (USBDC_Device[USBDC_VHID_DEVINX].Feature[currenthost] & FEATURE_RESUME)
#else		
	if (USBDC_Device[devinx].Feature[currenthost] & FEATURE_RESUME)
#endif	
	{
		wait_ms_event = 1;	
		goto TASK_DATAST_USB_Virtual_HID_Output_Handle_Exit2;
	}

	/*@@Check address */
#ifdef ONEHID		
	if ((USBDC_Device[USBDC_VHID_DEVINX].DevAddr[currenthost] == 0x00) || 
		((USBDC_Device[USBDC_VHID_DEVINX].Feature[currenthost] & (FEATURE_SUSPEND|FEATURE_REMOTE)) == FEATURE_SUSPEND))	
#else
	if ((USBDC_Device[devinx].DevAddr[currenthost] == 0x00) || 
		((USBDC_Device[devinx].Feature[currenthost] & (FEATURE_SUSPEND|FEATURE_REMOTE)) == FEATURE_SUSPEND))
#endif	/* #ifdef ONEHID */	
	{
#if (SYSTEM_EXTENDER_RECEIVER)		
DATAST_USB_Virtual_HID_Output_Reset:
#endif				
#ifdef SYNC
		HC_IntTransfer_Table[intt_id].PassControl.SyncRp[targethost] = HC_IntTransfer_Table[intt_id].PassControl.Wp;
		HC_IntTransfer_Table[intt_id].PassControl.SyncWp[targethost] = HC_IntTransfer_Table[intt_id].PassControl.Wp;

		if (targethost == KVM_CurrentHost)
		{	
			HC_IntTransfer_Table[intt_id].PassControl.Rp = HC_IntTransfer_Table[intt_id].PassControl.Wp;			
		}
#else	
		HC_IntTransfer_Table[intt_id].PassControl.Wp = 0;
		HC_IntTransfer_Table[intt_id].PassControl.Rp = 0;		
#endif /* #ifdef SYNC */
		goto TASK_DATAST_USB_Virtual_HID_Output_Handle_Exit2;		
	}

	/* 2.Process HID output */
#ifdef ONEHID
	if (USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_KB_ENDP_NUM] & USBDC_UPORT[currenthost])
	{		
		USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_KB_ENDP_NUM] &= ~USBDC_UPORT[currenthost];		
	#if (SYSTEM_EXTENDER_RECEIVER) && (PROJECT_USB_GENERIC_HID_ENABLE)
		if (KVM_CurrentHost != REMOTE_HOST_PORT)			
	#endif	/* #if (SYSTEM_EXTENDER_RECEIVER) && (PROJECT_USB_GENERIC_HID_ENABLE) */			
		{
			//USBDC_HAL_Endp_In_Buf_Move(DATAST_Generic_USB_KB_Queue,8,KVM_CurrentHost,USBDC_VHID_DEVINX,USBDC_VHID_KB_ENDP_NUM);
			//USBDC_REGS_Endp_ControlSet(KVM_CurrentHost,USBDC_VHID_DEVINX,USBDC_VHID_KB_ENDP_NUM,DA_CR_BVLD_SET);
			DATAST_Move_Intr_Buffer(currenthost,USBDC_VHID_DEVINX,USBDC_VHID_KB_ENDP_NUM,intt_id);
			wait_ms_event = 1;
		}
	}
#else		
	infid = HC_IntTransfer_Table[intt_id].InterfaceIdx;
	endp_id = HC_IntTransfer_Table[intt_id].EndpIdx & USBHC_PDEV_ENDPIDX_MASK;	
	endp_addr = USB_PDevice[devinx].EndpAddr[infid][endp_id];			
	USBDC_Search_EndpAddr(devinx,endp_addr,&endpidx);
		
	DATAST_Move_Intr_Buffer(currenthost,devinx,endpidx,intt_id);
#endif /* #ifdef ONEHID */

TASK_DATAST_USB_Virtual_HID_Output_Handle_Exit2:
#ifdef SYNC
	if (targethost != KVM_CurrentHost)
	{	
		wp = HC_IntTransfer_Table[intt_id].PassControl.SyncWp[targethost];
		rp = HC_IntTransfer_Table[intt_id].PassControl.SyncRp[targethost];
	}
    else
#endif /* #ifdef SYNC */
	{
		wp = HC_IntTransfer_Table[intt_id].PassControl.Wp;
		rp = HC_IntTransfer_Table[intt_id].PassControl.Rp;
	}	
	
	if (wp != rp) //There is still needed to handle queue 	
	{
		if (wait_ms_event)
		{
			if (TASK_Type != TASK_TYPE_INTERVAL_MS)
			{
				TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_DATAST_USB_Virtual_HID_ID,hid_inf,TASK_Register0,1,1); // now active the Generic KB Output Handle
			}
		}
		else
		{
			if (TASK_Type == TASK_TYPE_INTERVAL_MS)
			{
				TASK_Destory_Current(); //Now destory this task
			}
			TASK_Active(TASK_TYPE_EVENT,TASK_DATAST_USB_Virtual_HID_ID,hid_inf,TASK_Register0,0,0); // now aticev the Virtual HID Output Handle
		}
	}
	else
	{
		TASK_Destory_Current(); //Now destory this task
#ifdef SYNC				
		HC_IntTransfer_Table[intt_id].PassControl.SyncState &= ~BIT_MASK[targethost];				
#else		
		HC_IntTransfer_Table[intt_id].PassControl.State &= ~PASSTHROUGH_SET;
#endif
	}
}

/***************************************************************************************************/
/*
**  void DATAST_Move_Intr_Buf_Move(U8_T intt_id,U8_T *transmit_buf)
**  Parameter    : 
**  Description  : 
**
**
*/
U8_T DATAST_Move_Intr_Buf_Move(U8_T intt_id,U8_T *transmit_buf,U8_T currenthost)
{
	U8_T length,index;
	idata U8_T rp;
	
#ifdef SYNC	
	if (currenthost != KVM_CurrentHost)
		rp = HC_IntTransfer_Table[intt_id].PassControl.SyncRp[currenthost];
	else	
		rp = HC_IntTransfer_Table[intt_id].PassControl.Rp;
#else	
	currenthost = KVM_CurrentHost;
	rp = HC_IntTransfer_Table[intt_id].PassControl.Rp;		
#endif /* #ifdef SYNC */
	
	length = HC_IntTransfer_Table[intt_id].PassControl.Buf[rp];
	rp ++;
	if (rp >= HC_IntTransfer_Table[intt_id].PassControl.BufSize)
		rp = 0;
	 
	for (index=0;index < length ;index++)
	{
		transmit_buf[index] = HC_IntTransfer_Table[intt_id].PassControl.Buf[rp];
		rp++;
		if (rp >= HC_IntTransfer_Table[intt_id].PassControl.BufSize)
			rp = 0;
	}
#ifdef SYNC	
	HC_IntTransfer_Table[intt_id].PassControl.SyncRp[currenthost] = rp; // update the RP pointer 	
	if (currenthost == KVM_CurrentHost)	
	{	
		HC_IntTransfer_Table[intt_id].PassControl.Rp = rp; // update the RP pointer 				
	}	
#else	
	HC_IntTransfer_Table[intt_id].PassControl.Rp = rp; // update the RP pointer 
#endif /* #ifdef SYNC */
	
	return length;
}

/***************************************************************************************************/
/*
**  Function Name: DATAST_Move_Intr_Buffer(U8_T KVM_CurrentHost,U8_T devinx,U8_T endpidx,U8_T intt_id)
**  Parameter    : move virtual HID device interrupt tranfer buffer into dc endpoint buffer
**  Description  : 
**
**
*/   
void DATAST_Move_Intr_Buffer(U8_T currentHost,U8_T devinx,U8_T endpidx,U8_T intt_id)
{
	U8_T length;
#if (VIRTUAL_USB_HID_DEBUG)
	U8_T index;
#endif

	/* 2-2.Get DC endp Buffer Pointer */
	//get length
	if (USBDC_Check_Endp_Buffer_Valid(currentHost,devinx,endpidx))
	{		
		length = DATAST_Move_Intr_Buf_Move(intt_id,USBDC_EndpBufPtr[currentHost][devinx][endpidx]+OUT_BUF_OFFSET,currentHost);
		//printf("C:%bu,Dev:%bu,End:%bu,L:%bu,",currentHost,devinx,endpidx);
		//Disp_Str(USBDC_EndpBufPtr[currentHost][devinx][endpidx]+OUT_BUF_OFFSET,length);
		USBDC_EndpBufPtr[currentHost][devinx][endpidx][0] = length;		
		USBDC_REGS_Endp_ControlSet(currentHost,devinx,endpidx,DA_CR_BVLD_SET);  // inform data move
#ifdef SYNC		
		HC_IntTransfer_Table[intt_id].PassControl.SyncState |= BIT_MASK[currentHost+4];		
#else		
		HC_IntTransfer_Table[intt_id].PassControl.State |= PASSTHROUGH_WAIT;
#endif		
		USBDC_Device[devinx].EndpType[endpidx] = intt_id | USB_EP_TYPE_INTR;  // store td & transfer type into DC		
	}

#if (VIRTUAL_USB_HID_DEBUG)
	datainq_flag = 1;
	if (datainq_flag) // content has changed
	{
		printf("HIDQ=[ ");
		for (index=0; index < length ; index++)
		{
			printf("%02x ",(U16_T)p[index]);
		}
		printf("]\n\r");

	}
#endif
}

/***************************************************************************************************/
/*
**  Function Name: DATAST_Send_Kb_Break(U8_T currentHost,U8_T devinx, U8_T endpidx, U8_T reportId, U8_T reportLen)
**  Parameter    : send virtual HID keyboard break report into dc endpoint buffer
**  Description  : 
**
**
*/   
void DATAST_Send_Kb_Break(U8_T currentHost,U8_T devinx, U8_T endpidx, U8_T reportId, U8_T reportLen)
{
	U8_T	*p;
	U8_T	index;
	
	p = USBDC_EndpBufPtr[currentHost][devinx][endpidx]+OUT_BUF_OFFSET;
	for (index = 1;index < reportLen ;index++)
	{
		p[index] = 0;
	}
	p[0] = reportId;
	
	USBDC_EndpBufPtr[currentHost][devinx][endpidx][0] = reportLen; // length
	USBDC_REGS_Endp_ControlSet(currentHost,devinx,endpidx,DA_CR_BVLD_SET);  // inform data move
}

#endif /* End of SYSETM_DATA_STREAM_CONTROL_ENABLE */

/* End of data_stream.c */
