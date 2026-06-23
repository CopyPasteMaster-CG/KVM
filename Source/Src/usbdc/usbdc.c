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
 * Module Name: usbdc.c
 * Purpose: The USB DC Control & Managment program
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */
/*
 
*/
/* INCLUDE FILE SECTION */
#include <stdlib.h>
#include <string.h>
#include "project_include.h"

/* NAMING CONSTANT DECLARATIONS */
/* GLOBAL VARIABLES DECLARATIONS */
volatile USBDC_Index_TypeDef	USBDC_Index[USBDC_PORT_MAX][USBDC_DEVICE_MAX][USBDC_ENDP_MAX];
volatile U8_T					USBDC_UpPortState[USBDC_PORT_MAX];
volatile USBDC_DeviceTypeDef	USBDC_Device[USBDC_DEVICE_MAX];
xdata U8_T						*USBDC_EndpBufPtr[USBDC_PORT_MAX][USBDC_DEVICE_MAX][USBDC_ENDP_MAX];
U8_T XDATA						USB_Dynamic_Memory[USB_DYNAMIC_MEMORY_SIZE];
U8_T							TASK_USBDC_Get_EndpInterval_ID;
volatile USBDC_ISR_FIFO_TypeDef	USBDC_ISR_FIFO[USBDC_ISR_FIFO_DEPTH]; /*ISR FIFO Buffer*/
idata volatile U8_T				USBDC_ISRQ_Rp,USBDC_ISRQ_Wp;
U8_T    USBDC_Resume_Task_ActiveID[USBDC_PORT_MAX];
#ifdef RESUME_DELAY	
U8_T    USBDC_Resume_Timer_ActiveID[USBDC_DEVICE_MAX];
U8_T    VDevice_Keep_Quite_Flag[USBDC_PORT_MAX];
U8_T    DC_Keep_Quite_Flag;
U8_T    TASK_DC_Keep_Quite_TASKID;
U8_T    TASK_DC_Keep_Quite_Active_TaskID[USBDC_PORT_MAX];
U8_T    TASK_VDevice_Keep_Quite_TASKID;
#endif
U8_T	TASK_DC_Reset_Complete_TaskID;		

/* LOCAL VARIABLES DECLARATIONS 					*/
/* LOCAL SUBPROGRAM DECLARATIONS 					*/
void USBDC_Port_Regs_Clear(U8_T pid);
void USBDC_Port_GoSuspend(U8_T pid);
void USBDC_Port_Reset(U8_T pid);
void USBDC_CORE_Init(void);
void USBDC_CORE_Start(void);
void USBDC_Maintain_MSC_PDevice(U8_T pid);
void TASK_USBDC_Get_EndpInterval(void);
void TASK_VDevice_Keep_Quite(void);
void TASK_DC_Keep_Quite(void);
U8_T USBDC_Virtual_HubPort_Alloc(U8_T devinx);
void TASK_DC_Reset_Complete(void);


/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
extern bit			USBHC_MSC_DC_Buffer_Wait_Processing_Flag;
#if (ENABLE_HOTKEY_FUN_PLUGIN_JUMP_SW)
extern bit 			KVM_Plug_In_Jump_Flag;
#endif /* #if (ENABLE_HOTKEY_FUN_PLUGIN_JUMP_SW) */
extern const U8_T	DA_CR[USBDC_DEVICE_MAX];

/* EXTERNAL SUBPROGRAM DECLARATIONS 			*/ 
extern void DATAST_Move_Intr_Buffer(U8_T KVM_CurrentHost,U8_T devinx,U8_T endpidx,U8_T intt_id);

/*
 * ----------------------------------------------------------------------------
 * Function Name: USBDC_Init
 * Purpose: Initial the usb dc system  
 * Params:  NONE
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------
 */
void USBDC_Init(void)
{
	init_mempool(&USB_Dynamic_Memory,sizeof(USB_Dynamic_Memory)); 
	
	/*01.Initial Index & Endpoint Buffer                      */
	memset(USBDC_Index 					,0x00,sizeof(USBDC_Index));
	memset(USBDC_UpPortState 			,USBDC_ROOTHUB_SUSPEND_MASK,sizeof(USBDC_UpPortState));	 // default every port is in suspend 
	memset(USBDC_Resume_Task_ActiveID	,0x00,sizeof(USBDC_Resume_Task_ActiveID));
	
#ifdef RESUME_DELAY			
	memset(USBDC_Resume_Timer_ActiveID	,0x00,sizeof(USBDC_Resume_Timer_ActiveID));
	memset(VDevice_Keep_Quite_Flag		,0x00,sizeof(VDevice_Keep_Quite_Flag));
	DC_Keep_Quite_Flag = 0;
	TASK_VDevice_Keep_Quite_TASKID = TASK_Create(TASK_VDevice_Keep_Quite);
	TASK_DC_Keep_Quite_TASKID = TASK_Create(TASK_DC_Keep_Quite);
	memset(TASK_DC_Keep_Quite_Active_TaskID,0x00,sizeof(TASK_DC_Keep_Quite_Active_TaskID));
#endif
	/*02.USBDC Register Initial                               */ 
	USBDC_CORE_Init();

	/*----------------------------------------------*/
	/*03.Generate Virtual Hub & HID Endpoint Buffer */
	USBDC_VIRTUAL_Init();

	/*----------------------------------------------*/
	/*04.Start the usb dc core sytem                */
	USBDC_CORE_Start();	 
	
	TASK_DC_Reset_Complete_TaskID = TASK_Create(TASK_DC_Reset_Complete);
}

/*
 * ----------------------------------------------------------------------------
 * void usbdc_ISR(void)
 * Purpose : USBDC interrupt service routine. 
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
#if (INTR_ISR_INCLUDE_USBDC)
void usbdc_ISR(void) interrupt INT3_VECTOR
{
	idata U8_T pId, pIsr, pInsr, pEsmr;
	idata U8_T port_act;
	bit   dcisr_skip_flag;
	idata U8_T devinx,endpinx;
	idata U8_T dr,cir;	
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	idata U8_T reg;	
#endif		
	//Get Interrupt status
	port_act = UDCSR;

REDO:
	/* Clear isr skip flag */
	dcisr_skip_flag = 0;
#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
	PowerSavingInhibitFlag = 1;
#endif

	/* Check port number and get status */
	if (port_act & UDC_PORT0_ACT)
	{
		port_act &= ~UDC_PORT0_ACT;
		pId = 0;
		pIsr = DC0ISR;
		pInsr = DC0INSR;
		pEsmr = DC0ESMR;
	}
	else if (port_act & UDC_PORT1_ACT)
	{
		port_act &= ~UDC_PORT1_ACT;
		pId = 1;
		pIsr = DC1ISR;
		pInsr = DC1INSR;
		pEsmr = DC1ESMR;
	}
	else if (port_act & UDC_PORT2_ACT)
	{
		port_act &= ~UDC_PORT2_ACT;
		pId = 2;
		pIsr = DC2ISR;
		pInsr = DC2INSR;
		pEsmr = DC2ESMR;
	}
	else if (port_act & UDC_PORT3_ACT)
	{
		port_act &= ~UDC_PORT3_ACT;
		pId = 3;
		pIsr = DC3ISR;
		pInsr = DC3INSR;
		pEsmr = DC3ESMR;
	}
	else
	{
		return;
	}
	
	//2017-06-09 this code is for fixing the Skylake cup mother board resume issue, must can not deleted
	if (pIsr & DC_ISR_RWK_MASK)
	{
		if (pId==0)
		{
			_USBDC_DC0CIR_SFR(DCIER);			
			_USBDC_DC0DR_READ_SFR(dr);
			dr &= ~DC_IER_RSM_MASK;
			_USBDC_DC0DR_SFR(dr);			
			_USBDC_DC0CIR_SFR(DCIER);
		}			
		else if (pId==1)
		{
			_USBDC_DC1CIR_SFR(DCIER);			
			_USBDC_DC1DR_READ_SFR(dr);
			dr &= ~DC_IER_RSM_MASK;
			_USBDC_DC1DR_SFR(dr);			
			_USBDC_DC1CIR_SFR(DCIER);
		}
		else if (pId==2)
		{
			_USBDC_DC2CIR_SFR(DCIER);			
			_USBDC_DC2DR_READ_SFR(dr);
			dr &= ~DC_IER_RSM_MASK;
			_USBDC_DC2DR_SFR(dr);			
			_USBDC_DC2CIR_SFR(DCIER);
		}			
		else
		{
			_USBDC_DC3CIR_SFR(DCIER);			
			_USBDC_DC3DR_READ_SFR(dr);
			dr &= ~DC_IER_RSM_MASK;
			_USBDC_DC3DR_SFR(dr);			
			_USBDC_DC3CIR_SFR(DCIER);
		}				
	}	

	/* Check if any complete transaction */
	if (pIsr & DC_ISR_CT_SET)
	{
		devinx  = (pInsr & DC_INSR_DEVIND_MASK) DC_INSR_DEVIND_SHIFT;
		endpinx = (pInsr & DC_INSR_EPIND_MASK)  DC_INSR_EPIND_SHFIT;

		// Special Handle for Interrupt transfer interval time measure
		if (USBDC_Device[devinx].Interval_Check[pId] & BIT_MASK[endpinx]) //Still in measume period
		{
			USBDC_Device[devinx].EndpInterval0[pId][endpinx] = USBDC_Device[devinx].EndpInterval1[pId][endpinx];
			USBDC_Device[devinx].EndpInterval1[pId][endpinx] = SWTIMER_Counter;
			
			// The in-nake happened, need to retrigger
			if (pEsmr & DC_IESMR_IN_NAK)
			{
				dcisr_skip_flag = 1;
				cir = DAyINAKE+devinx;
				switch (pId)
				{
					case 0:
						_USBDC_DC0CIR_SFR(cir);
						_USBDC_DC0DR_READ_SFR(dr);
						dr |= BIT_MASK[endpinx];
						_USBDC_DC0DR_SFR(dr);
						_USBDC_DC0CIR_SFR(cir);
						break;
					case 1:
						_USBDC_DC1CIR_SFR(cir);
						_USBDC_DC1DR_READ_SFR(dr);
						dr |= BIT_MASK[endpinx];
						_USBDC_DC1DR_SFR(dr);
						_USBDC_DC1CIR_SFR(cir);
						break;
					case 2:
						_USBDC_DC2CIR_SFR(cir);
						_USBDC_DC2DR_READ_SFR(dr);
						dr |= BIT_MASK[endpinx];
						_USBDC_DC2DR_SFR(dr);
						_USBDC_DC2CIR_SFR(cir);
						break;
					case 3:
						_USBDC_DC3CIR_SFR(cir);
						_USBDC_DC3DR_READ_SFR(dr);
						dr |= BIT_MASK[endpinx];
						_USBDC_DC3DR_SFR(dr);
						_USBDC_DC3CIR_SFR(cir);
						break;
				}
			}
		}
		
		// Special Handle for audio device and not setup command
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
		if ((USBHC_Audio_Devinx == devinx) && (pId == KVM_CurrentUSBAudio) && (endpinx)) 
		{
			// Isochronous transfer type
			if ((USBDC_Device[devinx].EndpType[endpinx] & USB_EP_TYPE_MASK) == USB_EP_TYPE_ISOC)
			{
				if (USBDC_Device[devinx].EndpAddr[endpinx] & USBDC_TABLE_DIR_MASK) // IN
				{
#if (SYSTEM_EXTENDER_TRANSMITTER)
					ETDR_AudioInDcDoingFlag = 0;
					if (ETDR_AudioInHead != ETDR_AudioInTail)
					{
						USBDC_Index[KVM_CurrentUSBAudio][devinx][endpinx].Endp_Addr_Lsb = ETDR_AudioInBufAddr[ETDR_AudioInTail][0];
						USBDC_Index[KVM_CurrentUSBAudio][devinx][endpinx].Endp_Addr_Msb = ETDR_AudioInBufAddr[ETDR_AudioInTail][1];
			 			cir = DA_CR_BVLD_SET | (endpinx &0x07);
						switch (KVM_CurrentUSBAudio)
						{
							case 0:
								_USBDC_DC0DR_SFR(cir);
								_USBDC_DC0CIR_SFR(DA_CR[devinx]);
								break;
							case 1:
								_USBDC_DC1DR_SFR(cir);
								_USBDC_DC1CIR_SFR(DA_CR[devinx]);
								break;
							case 2:
								_USBDC_DC2DR_SFR(cir);
								_USBDC_DC2CIR_SFR(DA_CR[devinx]);
								break;
							case 3:
								_USBDC_DC3DR_SFR(cir);
								_USBDC_DC3CIR_SFR(DA_CR[devinx]);
								break;
						}
						ETDR_AudioInTail++;
						ETDR_AudioInTail &= EXTENDER_AUDIO_IN_BUF_MASK;
						ETDR_AudioInDcDoingFlag = 1;
					}
#else
					USBHC_AUDIO_In_Start();
#endif
				}
				else // OUT
				{
#if (SYSTEM_EXTENDER_TRANSMITTER)
					ETDR_AudioOutHead++;
					ETDR_AudioOutHead &= EXTENDER_AUDIO_OUT_BUF_MASK;
					if (ETDR_AudioOutHead == ETDR_AudioOutTail)
					{
						ETDR_AudioOutHead--; 
						ETDR_AudioOutHead &= EXTENDER_AUDIO_OUT_BUF_MASK;
						ETDR_AudioOutTailSkipFlag = 1;
					}
					USBDC_Index[KVM_CurrentUSBAudio][devinx][endpinx].Endp_Addr_Lsb = ETDR_AudioOutBufAddr[ETDR_AudioOutHead][0];
					USBDC_Index[KVM_CurrentUSBAudio][devinx][endpinx].Endp_Addr_Msb = ETDR_AudioOutBufAddr[ETDR_AudioOutHead][1];
#else					
					//if (USBHC_Audio_IN_Start_Flag)					
					HCTD_Table.ISTL[0].TD.Byte0_Actual_Byte = 0;
					HCTD_Table.ISTL[0].TD.Byte2_MaxPKT_Size = HCTD_Table.ISTL[0].Buf[0];
					HCTD_Table.ISTL[0].TD.Byte3 &= 0xFC;
					HCTD_Table.ISTL[0].TD.Byte4_Total_Byte = HCTD_Table.ISTL[0].Buf[0];
					HCTD_Table.ISTL[0].TD.Byte5 &= 0xFC;
					HCTD_Table.ISTL[0].TD.Byte1 |= TD_ACTIVED_MASK;

					// Clear Skip Map
					TD_ISTL_Skip_Map &= (~USB_HC_ISTL_OUT_DONE_MASK); // Do not skip ISL0
					//USBHC_INT_DISABLE;
					_USBHC_HCDR_SFR(TD_ISTL_Skip_Map);
					_USBHC_HCCIR_SFR(HC_ISTL_TDSKIP_MAP_REG);
					//USBHC_INT_ENABLE;					
#endif
					// Set the DC Buffer Ready
					cir = DA_CR_BCLR_SET | (endpinx & 0x07);
					switch (KVM_CurrentUSBAudio)
					{
						case 0:
							_USBDC_DC0DR_SFR(cir);
							_USBDC_DC0CIR_SFR(DA_CR[devinx]);
							break;
						case 1:
							_USBDC_DC1DR_SFR(cir);
							_USBDC_DC1CIR_SFR(DA_CR[devinx]);
							break;
						case 2:
							_USBDC_DC2DR_SFR(cir);
							_USBDC_DC2CIR_SFR(DA_CR[devinx]);
							break;
						case 3:
							_USBDC_DC3DR_SFR(cir);
							_USBDC_DC3CIR_SFR(DA_CR[devinx]);
							break;
					}
				}
				goto usbdc_ISR_End;
			}
		}
#endif /* End of SYSTEM_USBAUDIO_DEVICE_SUPPORT */

		// Handle the SetAddress Event
		// Set Address Status Stage done handle
		if (USBDC_Device[devinx].ISR_Handle[pId] & ISR_HANDLE_SET_ADDRESS) // If need isr handle
		{
			//Clear interval check flag 
			USBDC_Device[devinx].Interval_Detect[pId] = 0; //need to start check for all endpoint
			// Clear the ISR handle flag
			USBDC_Device[devinx].ISR_Handle[pId] &= ~ISR_HANDLE_SET_ADDRESS;

		// Enable the SOF auto sync 
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
			if (USBHC_Audio_Devinx == devinx) 
			{
				EXTINT2_DISABLE;
				reg = HC_SOF_SELECT_USPSE_SET | (pId & 0x03);				
				_USBHC_HCDR_SFR(reg);	
				_USBHC_HCCIR_SFR(HC_SOF_SELECT_REG);
				EXTINT2_ENABLE;
			}	
#endif			
			
			// Get new address
			dr  = USBDC_Device[devinx].DevAddr[pId] | DA_ID_CFGR_ADRACT_SET;
			cir = DA_ID_CFGR[devinx];
			
			// Set new address into hardware register
			if (pId == 0)
			{
				_USBDC_DC0DR_SFR(dr);
				_USBDC_DC0CIR_SFR(cir);
			} 
			else if (pId == 1)
			{
				_USBDC_DC1DR_SFR(dr);   
				_USBDC_DC1CIR_SFR(cir);
			}
			else if (pId == 2)
			{
				_USBDC_DC2DR_SFR(dr);
				_USBDC_DC2CIR_SFR(cir);
			}
			else
			{
				_USBDC_DC3DR_SFR(dr);
				_USBDC_DC3CIR_SFR(cir);
			}
		}
	}

	/* Check isr skip flag */
	if (dcisr_skip_flag == 0)
	{
		// Special handle for bus reset
		if (pIsr & DC_ISR_URST_MASK)
		{
			// Get reset status
			switch (pId)
			{
				case 0:
					_USBDC_DC0CIR_SFR(DCCR);
					_USBDC_DC0DR_READ_SFR(dr);
					break;
				case 1:
					_USBDC_DC1CIR_SFR(DCCR);
					_USBDC_DC1DR_READ_SFR(dr);
					break;
				case 2:
					_USBDC_DC2CIR_SFR(DCCR);
					_USBDC_DC2DR_READ_SFR(dr);
					break;
				case 3:
					_USBDC_DC3CIR_SFR(DCCR);
					_USBDC_DC3DR_READ_SFR(dr);
					break;
			}
			
			// Check if reset not yet
			if (dr & DC_CR_URST_MASK)
			{
				// Check if any other isr events
				if (!(pIsr &= ~DC_ISR_URST_MASK))
				{
					goto usbdc_ISR_End;
				}
			}
		}
		
		// Put status into ISR FIFO
		USBDC_ISR_FIFO[USBDC_ISRQ_Wp].Pid  = pId;
		USBDC_ISR_FIFO[USBDC_ISRQ_Wp].Isr  = pIsr;
		USBDC_ISR_FIFO[USBDC_ISRQ_Wp].Insr = pInsr;
		USBDC_ISR_FIFO[USBDC_ISRQ_Wp].Esmr = pEsmr;
		USBDC_ISRQ_Wp++;

		if (USBDC_ISRQ_Wp >= USBDC_ISR_FIFO_DEPTH)
		{
			USBDC_ISRQ_Wp = 0;
		}
	}

usbdc_ISR_End:

	/* Check if any port act did not process */
	if (port_act & UDC_PORT_ALL_ACT)
	{
		goto REDO;
	}

}

/*
 * ----------------------------------------------------------------------------
 * void USBDC_ISR_Handle(void)
 * Purpose : USBDC interrupt service routine. 
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */ 
void USBDC_ISR_Handle(void)
{
	idata U8_T	isr,insr,esmr,pid,devinx,endpinx,intt_id,dr;
	bit			buf_clear = 1;
#if (SYSTEM_EXTENDER_TRANSMITTER)
	U8_T        endp_bit;
#endif
	U8_T        inf_id=0;
	U8_T		*pSrc, *pDest;
	U8_T		len, index;
	USBHC_TD_Header_Typedef*	pTdHeader;

#if (SYSTEM_EXTENDER_TRANSMITTER && SYSTEM_EXTENDER_MSC_SUPPORT)
	if (EXTENDER_MSC_BulkOutCount >= EXTENDER_MSC_BUF_MASK)
		return;
#endif
	// Get necessary value for processing and speed up
	pid  = USBDC_ISR_FIFO[USBDC_ISRQ_Rp].Pid;
	isr  = USBDC_ISR_FIFO[USBDC_ISRQ_Rp].Isr;
	insr = USBDC_ISR_FIFO[USBDC_ISRQ_Rp].Insr;
	esmr = USBDC_ISR_FIFO[USBDC_ISRQ_Rp].Esmr;
	devinx  = (insr & DC_INSR_DEVIND_MASK) DC_INSR_DEVIND_SHIFT;
	endpinx = (insr & DC_INSR_EPIND_MASK)  DC_INSR_EPIND_SHFIT;
	
	// Increase the ISR FIFO read counter
	if (++USBDC_ISRQ_Rp >= USBDC_ISR_FIFO_DEPTH)
	{
		USBDC_ISRQ_Rp = 0;
	}
	
	if (pid >= USBDC_PORT_MAX)
	{
		//printf("\n\r[> ??????????? PID ???????????:%bu]\n\r",pid);
		return;
	}		
	
	//1.Check the CT flag, endpoint transcation has successfully
	if ((isr & DC_ISR_CT_SET) && (USBDC_Device[devinx].DevIdx & DC_USED_MASK))
	{
		//Check the root hub state first
		//2017-1-9 Add by Jack
		//If host PC did not send a resume singal to HUB, that will cause the root hub and the device under this port are still in resume state, 
		//So need ot clear it.		
		if (USBDC_UpPortState[pid] & USBDC_ROOTHUB_RESUME_MASK)
		{							
			//printf("Port:%bu Normal_Resume\n\r",pid);
			USBDC_Port_Normal(pid);
			//USBDC_UpPortState[pid] &= ~(USBDC_ROOTHUB_SUSPEND_MASK|USBDC_ROOTHUB_RESUME_MASK);
			//USBDC_VIRTUAL_Port_Status_Control(pid,FEATURE_RESUME|FEATURE_SUSPEND,RESET);	
		}

		//1-2.Check IN/OUT/Setup Token
		if ((insr&DC_INSR_EPIND_MASK) == 0) // Control Endpoint
		{
			//1-3.Check Data Direction & Setup Token
			//1-3-1.IN Data Stage or Status Stage Complete Handle			
			//1-3-3.Setup Token Handle
			if (esmr & DC_IESMR_SETUP)
			{		
				if (USBDC_Device[devinx].PassThroughState[pid] != PASSTHROUGH_IDLE)					
				{				
					if ((USB_PDevice[devinx].Hc.Control.State == CTRL_STATUS_OUT) || 
						(USB_PDevice[devinx].Hc.Control.State == CTRL_STATUS_OUT_WAIT)	||
						(USB_PDevice[devinx].Hc.PassThrough_State == PASSTHROUGH_COMPLETE) )				
				   	{
				   		if (USBDC_Device[devinx].Control_Cnt[pid] < 0xa0)
				   		{	
				   			USBDC_Device[devinx].Control_Cnt[pid]++;				   			
					   		if (USBDC_ISRQ_Rp == 0)
					   		{
					   			USBDC_ISRQ_Rp = USBDC_ISR_FIFO_DEPTH-1;
					   		}	
					   		else
					   		{
					   			USBDC_ISRQ_Rp --;
					   		}
					   		return;
				   		}
				   		else
				   		{
				   			USBDC_Device[devinx].Control_Cnt[pid] = 0;				   							   		
					   	}						   		
				   	}	
				}																						
				//copy Setup Token Header and get length
				USBDC_HAL_Endp_Out_Buf_Copy((U8_T *)(&USBDC_Device[devinx].Setup[pid]),pid,devinx,endpinx, 1);				
				//Change the endian order of wValue,wIndex,wLength,USB is little, Keil C is big endian
				//clear the setup & out flag, allow out buffer to get data again
				USBDC_HAL_Convert_Setup_Value((U8_T *)&USBDC_Device[devinx].Setup[pid]);
				USBDC_VIRTUAL_Setup_Process(devinx,pid);
			}
			else
			{
				if ((insr&DC_INSR_DIR_MASK) == DC_INSR_DIR_IN)
				{
					USBDC_VIRTUAL_Setup_IN_Process(pid, devinx);
				}
				//1-3-2.Out Direction
				else
				{
					USBDC_VIRTUAL_Setup_OUT_Process(pid, devinx);
				}
			}
		} /* End of if ((insr&DC_INSR_EPIND_MASK) == 0) */
		else
		{
			//1-3.Check IN/OUT Transfer for other endpoint 
			if ((insr&DC_INSR_DIR_MASK) == DC_INSR_DIR_IN)
			{
				if (USB_PDevice[devinx].DevClass == USB_HUB_CLASS)
				{
					USBDC_VHub_Update_StatusChange(devinx,pid,pid,0);
				} 
#if (PROJECT_USB_GENERIC_HID_ENABLE)
				else if (devinx == USBDC_VHID_DEVINX) // for Virtual HID
				{
/*					
#ifdef SYNC
					if (endpinx == USBDC_VHID_KB_ENDP_NUM) //Keyboard IN Complete
					{	
						if (KVM_Flash.cSystemFlag2 & (SYSTEM_ALL_SYNC_MASK | SYSTEM_KB_SYNC_MASK))
						{
							goto USBDC_INTR_VDEV_SYNC;
						}
					}
					else if (KVM_Flash.cSystemFlag2 & (SYSTEM_ALL_SYNC_MASK | SYSTEM_MS_SYNC_MASK))
					{
						goto USBDC_INTR_VDEV_SYNC;
					}																	
#endif
*/					
#ifndef SYNC					
					if (pid == KVM_CurrentHost)  //if KVM active port
#endif						
					{
						if (endpinx == USBDC_VHID_KB_ENDP_NUM) //Keyboard IN Complete
						{
#ifdef SYNC					
							if ((TASK_DATAST_PS2_Generic_KB_ActiveFlag & BIT_MASK[pid])==0)
							{
								if (DATAST_Generic_KB_IN_WP[pid] != DATAST_Generic_KB_IN_RP[pid]) //There is still ne
								{
									TASK_DATAST_PS2_Generic_KB_ActiveFlag |= BIT_MASK[pid];									
									TASK_Active(TASK_TYPE_EVENT,TASK_DATAST_PS2_Generic_KB_ID,0,pid,0,0); // now aticev the Generic KB Output Handle
								}
								else
								{
									KM_SYNC_Send_RepeatKey_Check(pid);									
								}									
							}
#else							
							if (TASK_DATAST_PS2_Generic_KB_ActiveFlag==0)
							{
								if (DATAST_Generic_KB_IN_WP != DATAST_Generic_KB_IN_RP) //There is still ne
								{
									TASK_DATAST_PS2_Generic_KB_ActiveFlag=1;
									TASK_Active(TASK_TYPE_EVENT,TASK_DATAST_PS2_Generic_KB_ID,0,0,0,0); // now aticev the Generic KB Output Handle
								}
							}
#endif							
							USBDC_HID_ENPx_Data_Wait_Flag[endpinx] |= USBDC_UPORT[pid];
						}
						else if ((endpinx == USBDC_VHID_MS_ENDP_NUM) || (endpinx == USBDC_VHID_MS2_ENDP_NUM))  //Mouse IN Complete
						{
#ifdef SYNC	
							if ((TASK_DATAST_PS2_Generic_MS_ActiveFlag & BIT_MASK[pid])==0)
							{
								#if (DATAST_GENERIC_MS_IN_QUEUE)		
								if (DATAST_Generic_MS_IN_WP != DATAST_Generic_MS_IN_RP[pid]) //There is still ne
								#else
								if (DATAST_Generic_MS_IN_WP[pid] != DATAST_Generic_MS_IN_RP[pid]) //There is still ne
								#endif			
								{
									TASK_DATAST_PS2_Generic_MS_ActiveFlag |= BIT_MASK[pid];
									//printf("[T1]");
									TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_DATAST_PS2_Generic_MS_ID2,0,pid,1,1); // now aticev the Generic KB Output Handle
								}
							}
#else							
							if (TASK_DATAST_PS2_Generic_MS_ActiveFlag==0)
							{
								if (DATAST_Generic_MS_IN_WP != DATAST_Generic_MS_IN_RP) //There is still ne
								{
									TASK_DATAST_PS2_Generic_MS_ActiveFlag=1;
									TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_DATAST_PS2_Generic_MS_ID2,0,0,1,1); // now aticev the Generic MS Output Handle
								}
							}
#endif							
							USBDC_HID_ENPx_Data_Wait_Flag[endpinx] |= USBDC_UPORT[pid];
						}
					}
				}
#endif /* #if (PROJECT_USB_GENERIC_HID_ENABLE) */
				else if ((USBDC_Device[devinx].EndpType[endpinx] & USB_EP_TYPE_MASK) == USB_EP_TYPE_INTR) //Interrupt transfer type 
				{
					intt_id = USBDC_Device[devinx].EndpType[endpinx] & 0x0f;  // Get interrupt TD ID number						
					inf_id = HC_IntTransfer_Table[intt_id].InterfaceIdx;						
					if (KVM_CurrentHost == pid)
					{
						//intt_id = USBDC_Device[devinx].EndpType[endpinx] & 0x0f;  // Get interrupt TD ID number						
						//inf_id = HC_IntTransfer_Table[intt_id].InterfaceIdx;						
#if (SYSTEM_EXTENDER_TRANSMITTER)
						if (USB_PDevice[devinx].InfProtocol[inf_id] != INF_KEYBOARD) /* should be INF_MOUSE ? */
						{
							if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)
							{							
								endp_bit = (0x01 << endpinx);
								USBDC_Device[devinx].EndpIntrAttr &= ~endp_bit; //clear the current queue
								if (USBDC_Device[devinx].EndpIntrWait & endp_bit)
								{									
									ExtenderT_Intr_In_Move_Wait_Handle(devinx,intt_id,endpinx);
								}
							}
							else
							{
#ifdef SYNC								
								HC_IntTransfer_Table[curinf].PassControl.SyncState &= ~BIT_MASK[KVM_CurrentHost+4];
#else								
								HC_IntTransfer_Table[intt_id].PassControl.State &= ~(PASSTHROUGH_WAIT);
#endif								
								if (HC_IntTransfer_Table[intt_id].PassControl.Wp != HC_IntTransfer_Table[intt_id].PassControl.Rp) // check if there is data needed processed
								{
									DATAST_Move_Intr_Buffer(KVM_CurrentHost,devinx,endpinx,intt_id);
								}
							}
						}
						else
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */
						{
#ifdef SYNC
							HC_IntTransfer_Table[intt_id].PassControl.SyncState &= ~BIT_MASK[KVM_CurrentHost+4];
#else							
							HC_IntTransfer_Table[intt_id].PassControl.State &= ~(PASSTHROUGH_WAIT);							
#endif							
							if (HC_IntTransfer_Table[intt_id].PassControl.Wp != HC_IntTransfer_Table[intt_id].PassControl.Rp) // check if there is data needed processed
							{
								DATAST_Move_Intr_Buffer(KVM_CurrentHost,devinx,endpinx,intt_id);
							}
						}

						if ((USBDC_Device[devinx].Interval_Detect[pid] & (0x01 << endpinx)) == 0x00 )
						{
							USBDC_Device[devinx].Interval_Detect[pid] |= (0x01 << endpinx);
							USBDC_VIRTUAL_SetCheck_Interval(pid,devinx,endpinx);
							USBDC_VIRTUAL_INNAK_Control(devinx,pid,BIT_MASK[endpinx],1);
						}
					}
#ifdef SYNC
					else
					{
						if (USB_PDevice[devinx].InfProtocol[inf_id] != INF_KEYBOARD) /* it is MOUSE inteface */
						{
							if (KVM_Flash.cSystemFlag2 & SYSTEM_MS_SYNC_MASK)
							{	
USBDC_SEND_SYNC_DATA:																																			
								HC_IntTransfer_Table[intt_id].PassControl.SyncState &= ~BIT_MASK[pid+4];		
								if (HC_IntTransfer_Table[intt_id].PassControl.SyncWp[index] != HC_IntTransfer_Table[intt_id].PassControl.SyncRp[index]) // check if there is data needed processed										
								{	
									DATAST_Move_Intr_Buffer(index,devinx,endpinx,intt_id);
								}										
							}								
						}													
						else
						{
							if (KVM_Flash.cSystemFlag2 & SYSTEM_KB_SYNC_MASK)
							{
								goto USBDC_SEND_SYNC_DATA;									
							}									
						}								
					}							
#endif /*	#ifdef SYNC  */							
				}
#if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST))
				else if ((USBDC_Device[devinx].EndpType[endpinx] & USB_EP_TYPE_MASK) == USB_EP_TYPE_BULK) 
				{
#if (SYSTEM_EXTENDER_TRANSMITTER)
					if (EXTENDER_MSC_Devinx == devinx)
					{
						ExtenderT_MSC_DcBurstInHandle(devinx);
					}
#else
					if (USBHC_MSC_Devinx == devinx)
					{
						USBHC_MSC_DC_Burst_In_Handle(devinx);
					}
#endif //#if (SYSTEM_EXTENDER_TRANSMITTER)
				}
#endif
			}
			//1-3-2.Out Direction
			else
			{
				if (USB_PDevice[devinx].DevClass != USB_HUB_CLASS)
				{
#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
					if (devinx == USBDC_VMSC_DEVINX) // for Virtual HID
					{
						USBDC_VMSC_Endpx_Data_OUT_Complete(pid,devinx,endpinx); // the out endpinx=2
						buf_clear = 0; // not clear the data
					}
#endif

#if (SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT)
 #if (SYSTEM_USB_HC_BURST)
  #if (SYSTEM_EXTENDER_TRANSMITTER)
					if (EXTENDER_MSC_Devinx == devinx)
					{
						buf_clear = ExtenderT_MSC_DcBurstOutHandle(devinx);
					}
  #else
					if (USBHC_MSC_Devinx == devinx)
					{
						buf_clear = USBHC_MSC_DC_Burst_Out_Handle(devinx);
					}
  #endif //#if (SYSTEM_EXTENDER_TRANSMITTER)
 #else // Non (SYSTEM_USB_HC_BURST)
  #if (SYSTEM_EXTENDER_TRANSMITTER)
					/* Check For MSC BULK OUT transfer */
					if (EXTENDER_MSC_Devinx == devinx)
					{
						if ((USBDC_Device[devinx].EndpType[endpinx] & USB_EP_TYPE_MASK) == USB_EP_TYPE_BULK)
						{
							ExtenderT_MSC_DcBulkOutReceiveHandle(pid, devinx, endpinx);
						}
						buf_clear = 0;
					}
  #else
					if (USBHC_MSC_Devinx == devinx)
					{
						if (USBHC_MSC_DC_Buffer_Wait_Processing_Flag == 0) // in processing,the isr routine should check this flag
						{
							if (USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK)  // Device still exist
							{		
								USBHC_MSC_DC_Buffer_Wait_Processing_Flag=1;
								TASK_USBHC_MSC_DC_Out_Wait_Task.Task_Para = pid;
								TASK_USBHC_MSC_DC_Out_Wait_Task.Task_Interval.w = devinx;
								TASK_Run(&TASK_USBHC_MSC_DC_Out_Wait_Task);
							}
						}
						buf_clear = 0;
					}
  #endif /* (SYSTEM_EXTENDER_TRANSMITTER) */
 #endif /* (SYSTEM_USB_HC_BURST) */
#endif /* (SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) */

					if ((USBDC_Device[devinx].EndpType[endpinx] & USB_EP_TYPE_MASK) == USB_EP_TYPE_INTR)
					{
						if (pid == KVM_CurrentHost)
						{
#if (SYSTEM_EXTENDER_TRANSMITTER)
							if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)
							{
								ExtenderT_USB_IntrOutProcess(devinx, endpinx);
						 		buf_clear = 0;
							}
							else
#endif
							{
								// INTERRUPT OUT for non-extender path
								pSrc = USBDC_EndpBufPtr[pid][devinx][endpinx];
								len = *pSrc;
								pSrc += OUT_BUF_OFFSET; // data point
								intt_id = USBDC_Device[devinx].EndpType[endpinx] & 0x0f;
								pDest = HCTD_Table.INTL[intt_id].Buf;
								for (index = 0; index < len ; index++)
								{
									pDest[index] = pSrc[index];
								}

								pTdHeader = &(HCTD_Table.INTL[intt_id].TD);
								pTdHeader->Byte4_Total_Byte = len;
								pTdHeader->Byte5 &= 0xFC;
								pTdHeader->Byte1 |= TD_ACTIVED_MASK;

								USBHC_CORE_Set_TD_SkipMap(USBHC_TD_INTL,intt_id,0x00);

								buf_clear = 1;
							}
						}
					}

					if (buf_clear)
					{
						USBDC_REGS_Endp_ControlClear(pid,devinx,endpinx,DA_CR_BCLR_SET); // now can receive data
					}
				}
			}
		}
	}
	else if  (isr & DC_ISR_CT_SET)
	{
		USBDC_REGS_Endp_Reset(pid, devinx, endpinx);
	}
	
	//2.Check VBUS Status
	if (isr & DC_ISR_VCHG_SET)
	{
		USBDC_INT_DISABLE;
		switch (pid)
		{
			case 0:
				_USBDC_DC0CIR_SFR(DCCR);
				_USBDC_DC0DR_READ_SFR(insr);
				break;
			case 1:
				_USBDC_DC1CIR_SFR(DCCR);
				_USBDC_DC1DR_READ_SFR(insr);
				break;
			case 2:
				_USBDC_DC2CIR_SFR(DCCR);
				_USBDC_DC2DR_READ_SFR(insr);
				break;
			case 3:
				_USBDC_DC3CIR_SFR(DCCR);
				_USBDC_DC3DR_READ_SFR(insr);
				break;
		}
		USBDC_INT_ENABLE;

		if (!(insr & DC_CR_VBUS_MASK) &&
			(USBDC_UpPortState[pid] & USBDC_ROOTHUB_ATTACHED_MASK) == USBDC_ROOTHUB_ATTACHED_SET) //VBUS changed, and it is detached
		{
			//printf("> DC_Port:%bu, VBUS Disattached\n\r",pid);
			USBDC_UpPortState[pid] &= ~(USBDC_ROOTHUB_ATTACHED_MASK | USBDC_ROOTHUB_REMOTE_WAKEUP);
			USBDC_UpPortState[pid] |= USBDC_ROOTHUB_SUSPEND_MASK;
			USBDC_Device[devinx].DevAttr[pid] &= RESET_ONCE_WHEN_SW_MASK; //reset the port
#if (SYSTEM_EXTENDER_TRANSMITTER)
		#ifdef PCT_MUA22
			KVM_Host_Led_Control(pid,LED_OFF);
		#endif /* #ifdef PCT_MUA22	*/
			ExtenderT_HostLedCheck(pid);
		
#else
			KVM_Host_Led_Control(pid,LED_OFF);
			
#ifdef SYNC			
			//KM_SYNC_ClearStart(pid);
#endif
			
#endif
			//1.Should do the Clean processing 
#ifdef RESUME_DELAY
			VDevice_Keep_Quite_Flag[pid] = 0;
#endif			
			USBDC_Port_Regs_Clear(pid);
			USBDC_VIRTUAL_Port_Clear(pid,0,USBDC_DEVICE_MAX);
			USBDC_HAL_Address_Active(pid,0,SET);
#if (SYSTEM_MSC_DEVICE_SUPPORT) 			
			USBDC_Maintain_MSC_PDevice(pid);
#endif
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
			USBHC_AUDIO_In_Stop(pid);
#endif

			//2.Do the KVM Task
#if (ENABLE_HOTKEY_FUN_POWER_JUMP_SW)
			KVM_Power_Jump_Check(pid,1);
#endif /* #if (ENABLE_HOTKEY_FUN_POWER_JUMP_SW) */

#if (ENABLE_HOTKEY_FUN_PLUGIN_JUMP_SW)
			if ((KVM_Flash.cSystemFlag1 & SYSTEM_PLUGIN_JUMP_MODE) == 0)
			{
				/* Check all ports were unplugged ? */
				for (index = 0; index < KVM_MAX_PORT; index++)
				{
					if (USBDC_UpPortState[index] & USBDC_ROOTHUB_ATTACHED_MASK)
						break;
				}
				/* No Upstream port plugged, then reenabled the first jump function */
				if (index >= KVM_MAX_PORT)
					KVM_Plug_In_Jump_Flag = 0;
			}
#endif /* #if (ENABLE_HOTKEY_FUN_PLUGIN_JUMP_SW) */
			
#ifdef KMLOG
			KMLog_Send_Host_State();
#endif			
		}
		else if ((insr & DC_CR_VBUS_MASK) &&
				!(USBDC_UpPortState[pid] & USBDC_ROOTHUB_ATTACHED_MASK))
		{
			//printf("> DC_Port:%bu, VBUS Attached\n\r",pid);
			USBDC_UpPortState[pid] |= USBDC_ROOTHUB_ATTACHED_MASK;
#if (SYSTEM_EXTENDER_TRANSMITTER)
		#ifdef PCT_MUA22
			KVM_Host_Led_Control(pid, LED_ON);
		#endif	/* #ifdef PCT_MUA22 */
			ExtenderT_HostLedCheck(pid);		
#else
			KVM_Host_Led_Control(pid, LED_ON);
#endif

			//1.Do Reset First
			//Should do the VHub Reset
			USBDC_VHub_UpPort_Reset(USBDC_VHUB_DEVINX,pid);

			//2.Do the KVM Task
#if (ENABLE_HOTKEY_FUN_PLUGIN_JUMP_SW)
			KVM_PlugIn_Jump_Check(pid);
#endif /* #if (ENABLE_HOTKEY_FUN_PLUGIN_JUMP_SW) */

#ifdef KMLOG
			KMLog_Send_Host_State();
#endif						
		}
	}

	//3.Check Reset Status
	if (isr & DC_ISR_URST_MASK)
	{
		//--------------------------------------------------------------------------------------------------------
		//printf("> DC_Port:%bu, Reset\n\r",pid);
		USBDC_Virtual_Hub_Reset(pid,0);		
#ifdef SYNC		
		KM_SYNC_ClearStart(pid);		
#endif		
		
#ifdef KMLOG
		KMLog_Send_Host_State();
#endif		

#ifdef VPID_CHANGER		
		KM_Sync_VPid_Change_Auto(pid);		
#endif		
		
	}

	//4.Check Suspend Status
	if (isr & DC_ISR_USUS_MASK)
	{
		//printf("> DC_Port:%bu, Suspend\n\r",pid);
		USBDC_Port_GoSuspend(pid);
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
		USBHC_AUDIO_In_Stop(pid);
#endif
		
#ifdef KMLOG
		KMLog_Send_Host_State();
#endif					
	}

	//5.Check Resume Status(Resume by Host)
	if (isr & DC_ISR_RWK_MASK)
	{	
		//2017-06-09 this code is for fixing the Skylake cup mother board resume issue, must can not deleted		
		if (pid==0)
		{
			_USBDC_DC0CIR_SFR(DCIER);			
			_USBDC_DC0DR_READ_SFR(dr);
			dr |= DC_IER_RSM_MASK;
			_USBDC_DC0DR_SFR(dr);			
			_USBDC_DC0CIR_SFR(DCIER);
		}			
		else if (pid==1)
		{
			_USBDC_DC1CIR_SFR(DCIER);			
			_USBDC_DC1DR_READ_SFR(dr);
			dr |= DC_IER_RSM_MASK;
			_USBDC_DC1DR_SFR(dr);			
			_USBDC_DC1CIR_SFR(DCIER);
		}
		else if (pid==2)
		{
			_USBDC_DC2CIR_SFR(DCIER);			
			_USBDC_DC2DR_READ_SFR(dr);
			dr |= DC_IER_RSM_MASK;
			_USBDC_DC2DR_SFR(dr);			
			_USBDC_DC2CIR_SFR(DCIER);
		}			
		else
		{
			_USBDC_DC3CIR_SFR(DCIER);			
			_USBDC_DC3DR_READ_SFR(dr);
			dr |= DC_IER_RSM_MASK;
			_USBDC_DC3DR_SFR(dr);			
			_USBDC_DC3CIR_SFR(DCIER);
		}					
		//--------------------------------------------------------------------------------------------------------
				
		//printf("> DC_Port:%bu, Resume\n\r",pid);
		//TASK_Active(TASK_TYPE_INTERVAL_MS, TASK_KVM_ACCESSORY_ID, 0, pid, 1000, 1000);
		if (USBDC_Resume_Task_ActiveID[pid]==0)
		{				
			USBDC_Resume_Task_ActiveID[pid] = TASK_Active(TASK_TYPE_INTERVAL_MS, TASK_KVM_ACCESSORY_ID, 0, pid, 1000, 1000)+1;
			//printf("> Port:%bu,Accessory Start(%bu)\n\r",pid,USBDC_Resume_Task_ActiveID[pid]);
		}
		else
		{
			 //printf("  * Port:%bu,AccessoryTime Reset]\n\r",pid);
			 Task_Active_Table[USBDC_Resume_Task_ActiveID[pid]-1].Task_Interval.w = 1000;
		}		
		USBDC_VHub_Resume_HubPort_Device(pid);
#ifdef KMLOG
		KMLog_Send_Host_State();
#endif
	}

	//6.Check SOF interrupt for Audio
	if (isr & DC_ISER_SOF_SET)
	{
	}

}
#endif // End of (INTR_ISR_INCLUDE_USBDC)

/* ----------------------------------------------------------------------------
 * void USBDC_Port_Regs_Clear(void)
 * Purpose : USBDC Port register clear,after get reset interrupt, clear all the 
 *           device's registers
 * Params  : none
 * Returns : none
 * Note    : none
 * ---------------------------------------------------------------------------- */
void USBDC_Port_Regs_Clear(U8_T pid)
{
	U8_T	devinx,endpinx;

	for (devinx = 0; devinx < USBDC_DEVICE_MAX; devinx ++)
	{
		USBDC_REGS_Device_Reset(pid, devinx);
		for (endpinx = 0; endpinx < USBDC_ENDP_MAX; endpinx ++)
		{
			USBDC_REGS_Endp_Reset(pid, devinx, endpinx);
		}
	}
	
	/* Also clear all PASSTHROUGH_WAIT flag at HC side 
	   after clear DC endpoint FIFOs at active port */
	if (KVM_CurrentHost == pid)
	{
#ifdef SYNC
USBDC_Port_Regs_Clear_Start:
#endif		
		for (devinx = 0; devinx < USB_HC_INTL_MAX; devinx ++)
		{
			if (HC_IntTransfer_Table[devinx].Used_DeviceID & USBHC_DEVICE_USED_MASK)
			{	
#ifdef SYNC					
				HC_IntTransfer_Table[devinx].PassControl.SyncState &= ~BIT_MASK[pid+4];				
#else
				HC_IntTransfer_Table[devinx].PassControl.State &= ~(PASSTHROUGH_WAIT);
#endif				
			}	
		}
	}
#ifdef SYNC
	else
	{
		goto USBDC_Port_Regs_Clear_Start;
	}
#endif	
}

/* ----------------------------------------------------------------------------
 * void USBDC_Port_GoSuspend(void)
 * Purpose : USBDC Port suspend operation
 * Params  : none
 * Returns : none
 * Note    : none
 * ---------------------------------------------------------------------------- */
void USBDC_Port_GoSuspend(U8_T pid)
{
#ifdef RESUME_DELAY		
	DC_Keep_Quite_Flag |= BIT_MASK[pid];
#endif	
	USBDC_UpPortState[pid] |= USBDC_ROOTHUB_SUSPEND_MASK;
	USBDC_UpPortState[pid] &= ~USBDC_ROOTHUB_RESUME_MASK;
	
#ifdef RESUME_DELAY		
	if (TASK_DC_Keep_Quite_Active_TaskID[pid] == 0)
		TASK_DC_Keep_Quite_Active_TaskID[pid] = TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_DC_Keep_Quite_TASKID,0,pid,VBUS_RESUME_KEEP_QUITE,VBUS_RESUME_KEEP_QUITE); // now active the Generic KB Output Handle		
#endif	
	// Set All Port into Suspend
	USBDC_VIRTUAL_Port_Status_Control(pid,FEATURE_SUSPEND,SET);
	USBDC_HAL_Port_Suspend(pid);
}

/* ----------------------------------------------------------------------------
 * void USBDC_PortSuspendCheck(void)
 * Purpose : Check all upstream port suspend status 
 * Params  : none
 * Returns : none
 * Note    : none
 * ---------------------------------------------------------------------------- */
U8_T USBDC_PortSuspendCheck(void)
{
	U8_T	pidx;

	for (pidx = 0; pidx < USBDC_PORT_MAX; pidx ++)
	{
		if ((USBDC_UpPortState[pidx] & USBDC_ROOTHUB_SUSPEND_MASK) == 0)
		{
			return 0;
		}
	}
	return 1;
}

/* ----------------------------------------------------------------------------
 * void USBDC_WakeupUpstreamHost(U8_T pid)
 * Purpose : Wakeup assingned upstream port if host entered into suspend mode
 * Params  : none
 * Returns : none
 * Note    : none
 * ---------------------------------------------------------------------------- */
void USBDC_WakeupUpstreamHost(U8_T pid)
{
	if ((USBDC_UpPortState[pid] & USBDC_ROOTHUB_REMOTE_WAKEUP) &&
		(USBDC_UpPortState[pid] & USBDC_ROOTHUB_SUSPEND_MASK))
	{
		USBDC_Port_Resume(pid);
	}
}

/* ----------------------------------------------------------------------------
 * void USBDC_Port_Resume(void)
 * Purpose : USBDC Port resume operation
 * Params  : none
 * Returns : none
 * Note    : none
 * ---------------------------------------------------------------------------- */ 
void USBDC_Port_Resume(U8_T pid)
{	
	USBDC_UpPortState[pid] |= USBDC_ROOTHUB_RESUME_MASK;
	USBDC_UpPortState[pid] &= ~USBDC_ROOTHUB_SUSPEND_MASK;
	USBDC_VIRTUAL_Port_Status_Control(pid,FEATURE_RESUME,SET);
	USBDC_VIRTUAL_Port_Status_Control(pid,FEATURE_SUSPEND,RESET);
	USBDC_HAL_Port_Resume(pid);
}

/* ----------------------------------------------------------------------------
 * void USBDC_Port_Normal(void)
 * Purpose : USBDC Port resume operation
 * Params  : none
 * Returns : none
 * Note    : none
 * ---------------------------------------------------------------------------- */ 
void USBDC_Port_Normal(U8_T pid)
{
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	U8_T reg;
#endif	

	if (USBDC_UpPortState[pid] & (USBDC_ROOTHUB_SUSPEND_MASK | USBDC_ROOTHUB_RESUME_MASK))
	{
		USBDC_UpPortState[pid] &= ~(USBDC_ROOTHUB_SUSPEND_MASK|USBDC_ROOTHUB_RESUME_MASK);
		USBDC_VIRTUAL_Port_Status_Control(pid,FEATURE_RESUME|FEATURE_SUSPEND,RESET);

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
		if (pid == KVM_CurrentUSBAudio)
		{
			if (HC_IsoTransfer_Table[USB_HC_ISTL_OUT_MAX].Used_DeviceID & USBHC_DEVICE_USED_MASK)
			{
				EXTINT3_DISABLE;
				USBHC_AUDIO_In_Start();
				EXTINT3_ENABLE;
			}
		
			if (HC_IsoTransfer_Table[0].Used_DeviceID & USBHC_DEVICE_USED_MASK)
			{
				reg = (KVM_CurrentUSBAudio & 0x03);
				USBHC_Write_Regs(HC_SOF_SELECT_REG, &reg, 1);
				reg = HC_SOF_SELECT_USPSE_SET | (KVM_CurrentUSBAudio & 0x03);
				USBHC_Write_Regs(HC_SOF_SELECT_REG, &reg, 1);
			}
		}
#endif
		
		#ifdef KMLOG
		KMLog_Send_Host_State();
		#endif
	}	
}

/* ----------------------------------------------------------------------------
 * void USBDC_Port_Reset(void)
 * Purpose : USBDC Port reset operation:only clear the status
 * Params  : none
 * Returns : none
 * Note    : none
 * ---------------------------------------------------------------------------- */ 
void USBDC_Port_Reset(U8_T pid)
{
	USBDC_UpPortState[pid] &= ~(USBDC_ROOTHUB_SUSPEND_MASK|USBDC_ROOTHUB_RESUME_MASK);
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: USBDC_CORE_Init
 * Purpose: Initial the usb dc core system  
 * Params:  NONE
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------
 */
void USBDC_CORE_Init(void)
{
	U32_T iaddr;

	USBDC_ISRQ_Rp = 0;
	USBDC_ISRQ_Wp = 0;
	/*------------------------------------------*/
	/*01.Foruce USB bus Reset */
	_USBDC_DC0DR_SFR(DC_CR_FSE0_SET);
	_USBDC_DC0CIR_SFR(DCCR);

	_USBDC_DC1DR_SFR(DC_CR_FSE0_SET);
	_USBDC_DC1CIR_SFR(DCCR);

	_USBDC_DC2DR_SFR(DC_CR_FSE0_SET);
	_USBDC_DC2CIR_SFR(DCCR);

	_USBDC_DC3DR_SFR(DC_CR_FSE0_SET);
	_USBDC_DC3CIR_SFR(DCCR);

	/*------------------------------------------*/
	/*02.Write Index Table Address to Registr  */
	/*02_1. Assign Upstream Port 0 */
	iaddr = (U32_T)(&USBDC_Index[0][0][0]);
	DC0DR = iaddr & 0x000000ff;
	DC0DR = (iaddr & 0x0000ff00) >> 8;
	DC0DR = 0x00;
	DC0CIR = INDBASR;

	/*02_2. Assign Upstream Port 1 */
	iaddr = (U32_T)(&USBDC_Index[1][0][0]);
	DC1DR = iaddr & 0x000000ff;
	DC1DR = (iaddr & 0x0000ff00) >> 8;
	DC1DR = 0x00;
	DC1CIR = INDBASR;

	/*02_3. Assign Upstream Port 2 */
	iaddr   = (U32_T)(&USBDC_Index[2][0][0]);
	DC2DR = iaddr & 0x000000ff;
	DC2DR = (iaddr & 0x0000ff00) >> 8;
	DC2DR = 0x00;
	DC2CIR = INDBASR;

	/*02_4. Assign Upstream Port 3 */
	iaddr = (U32_T)(&USBDC_Index[3][0][0]);
	DC3DR = iaddr & 0x000000ff;
	DC3DR 	= (iaddr & 0x0000ff00) >> 8;
	DC3DR = 0x00;
	DC3CIR 	= INDBASR;
}

/**----------------------------------------------------------------------------
 * Function Name: USBDC_CORE_Start
 * Purpose: Enable the Interrupt and release the Froce SE0 bus condition
 * Params:  NONE
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void USBDC_CORE_Start(void)
{
	U8_T  reg0;

	/*--------------------------------------------*/
	// Enable all interrupt flag,except ESOF & SOF
	/*04.Enable interrupt for every upstream port */
	reg0 = (0xff & ~(DC_IER_ESOF_SET | DC_IER_SOF_SET));
	USBDC_Write_Regs(0,DCIER,&reg0,1);
	USBDC_Write_Regs(1,DCIER,&reg0,1);
#ifndef MCU_TYPE_AX68002
	USBDC_Write_Regs(2,DCIER,&reg0,1);
	USBDC_Write_Regs(3,DCIER,&reg0,1);
#endif

	/*---------------------------------------------------------*/
	/*05.Release BUS Reset condistion,enable Device controller */
	reg0 = DC_CR_DCEN_SET|DC_CR_SRSM_SET;
	USBDC_Write_Regs(0,DCCR,&reg0,1);
	USBDC_Write_Regs(1,DCCR,&reg0,1);
#ifndef MCU_TYPE_AX68002
	USBDC_Write_Regs(2,DCCR,&reg0,1);
	USBDC_Write_Regs(3,DCCR,&reg0,1);
#endif

	/*06.Enable Interrupt 3 */
#if (INTR_ISR_INCLUDE_USBDC)
	EXTINT3_ENABLE;
#endif

	TASK_USBDC_Get_EndpInterval_ID = TASK_Create(TASK_USBDC_Get_EndpInterval);

#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("TASK_USBDC_Get_EndpInterval_ID=%bu\n\r",TASK_USBDC_Get_EndpInterval_ID);
#endif	
}

/**----------------------------------------------------------------------------
 * Function Name: USBDC_VirtualDevice_Create(U8_T devinx)
 * Purpose: create a virtual devcie in dc side base on hc information
 * Params :$devinx:the hc devinx, the virtual vdevice_id is same with hc
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
U8_T USBDC_VirtualDevice_Create(U8_T devinx)
{
	U8_T					roothubport, direction,index,skip;
	U8_T					infinx, endpinx, endpnum, infnum;
	USBHC_CfgDesc_TypeDef	*dc_cfg;
	U8_T					virtual_port;

	//~~~~~~~~~~~~~~~~~~~~~~~
	//1.Check the device ID
	//~~~~~~~~~~~~~~~~~~~~~~~
	if (USBDC_Device[devinx].DevIdx & DC_USED_MASK) // if device has been occupy by other device
	{
		//printf(">>> KVM ERROR: VDevinx:%bx been used, skip Mount\n\r", devinx);
		return 0;
	}

	if (devinx >= USB_HC_MAX_DEVICE)
	{
//		printf("USBDC_VirtualDevice_Create: devinx=%bx >= USB_HC_MAX_DEVICE\n\r", devinx);
		return 1;
	}
	else
	{
#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
		if (devinx != USBDC_VMSC_DEVINX)
		{
			// get a new port for virtual device
			virtual_port = USBDC_Virtual_HubPort_Alloc(devinx);
			if (virtual_port == 0)
			{
				//printf("No free port id of root hub...\n\r");
				return 0;
			}
			virtual_port--;			
			USBDC_Virtual_Hub_Map[virtual_port].HubPort_Devinx = devinx;
			USBDC_Virtual_Hub_Map[devinx].Devinx_HubPort = virtual_port;
		}
		else
		{
			virtual_port = USBDC_VMSC_PORT_NUM;
		}
#else
		// get a new port for virtual device
		virtual_port = USBDC_Virtual_HubPort_Alloc(devinx);
		if (virtual_port == 0)
		{
			//printf("No free port id of root hub...\n\r");
			return 0;
		}
		virtual_port--;
		USBDC_Virtual_Hub_Map[virtual_port].HubPort_Devinx = devinx;
		USBDC_Virtual_Hub_Map[devinx].Devinx_HubPort = virtual_port;
#endif
	}

	//Check the HUB Class
	if (USB_PDevice[devinx].DevClass == USB_HUB_CLASS)
	{
		USBDC_Device[devinx].VHub = m_malloc(sizeof(VHub_ReportTypeDef),27); // create the hub report struct
		if (!USBDC_Device[devinx].VHub)
		{
			//printf(">>> KVM ERROR: VDevinx:%bx VHub = Null, skip Mount\n\r", devinx);
			return 0;
		}
	}
	USBDC_Device[devinx].DevIdx |= DC_USED_MASK;

	//Check the upper layer HUB port
	roothubport = (USB_PDevice[devinx].Hub_NbrPorts & USBHC_ROOTHUBPORT_PORT_NUM_MASK) USBHC_ROOTHUBPORT_PORT_NUM_READ;
#if (SYSTEM_MSC_DEVICE_SUPPORT)
	if (devinx == USBHC_MSC_Devinx)
		USBHC_HubPortNumber = roothubport;
#endif
	memset(USBDC_Device[devinx].VirHubNum, (roothubport + 1), sizeof(USBDC_Device[devinx].VirHubNum));
	(USB_PDevice[USB_PDevice[devinx].UpperHubDevinx].HUB.ReportState+roothubport)->Devinx = devinx;

	//Get EndpNum & Address & MaxSize
	USBDC_Device[devinx].EndpAddr[0] = USBDC_TABLE_DOUBF_MASK; // Control Endp address
	USBDC_Device[devinx].EndpMaxSize[0] = USB_PDevice[devinx].ControlEndpSize;

	//Get Interface number
	endpnum = 1;
	infnum = USB_PDevice[devinx].InterfaceNum & USBHC_DEVICE_INTERFACENUM_MASK;

	//Get every Endpoint buffer pipe basic information in each interface.
	//???? if the endp is also in & out?	
	for (infinx = 0; infinx < infnum; infinx ++)
	{
		for (endpinx = 0; endpinx < USB_PDevice[devinx].EndpNum[infinx]; endpinx++)
		{			
			USBDC_Device[devinx].EndpAddr[endpnum] = USB_PDevice[devinx].EndpAddr[infinx][endpinx] & ~0x80;		
			if (USB_PDevice[devinx].EndpAddr[infinx][endpinx] & USB_EP_DIR_IN) // IN
			{
				USBDC_Device[devinx].EndpAddr[endpnum] |= USBDC_TABLE_DIR_IN_MASK;				
			}						
			USBDC_Device[devinx].EndpMaxSize[endpnum] = USB_PDevice[devinx].EndpSize[infinx][endpinx];
			USBDC_Device[devinx].EndpType[endpnum] = USB_PDevice[devinx].EndpType[infinx][endpinx];
			
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)				
			if (USB_PDevice[devinx].DevClass == USB_AUDIO_CLASS)
			{
				if ((USBDC_Device[devinx].EndpType[endpnum] & USB_EP_TYPE_MASK) == USB_EP_TYPE_ISOC)
				{					
					if (USB_PDevice[devinx].EndpAddr[infinx][endpinx] & USB_EP_DIR_IN) // IN
					{				
						USBHC_Audio_In_Interface = infinx;
					}
					else
					{					
						USBHC_Audio_Out_Interface = infinx;
					}		
				}	
			}	
#endif			
			endpnum++;
		}
	}

	//Log the total endp numbers
	USBDC_Device[devinx].EndpNum = endpnum;

	//Build DC Endp register table
	//Check the HUB Class
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	if (USB_PDevice[devinx].DevClass == USB_AUDIO_CLASS)
	{
		USBHC_Audio_IN_Flag = 0;
		USBHC_Audio_Devinx = devinx;
	}
#endif //#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)

	for (endpinx = 0; endpinx < endpnum; endpinx ++)
	{
		if (USBDC_Device[devinx].EndpAddr[endpinx] & USBDC_TABLE_DOUBF_MASK)
		{
			direction = USBDC_TABLE_DOUBF_MASK;
		}
		else if (USBDC_Device[devinx].EndpAddr[endpinx] & USBDC_TABLE_DIR_IN_MASK) // direct IN
		{
			direction = USBDC_TABLE_DIR_IN_MASK;
		}
		else
		{
			direction = USBDC_TABLE_DIR_OUT_MASK;
		}

		if ((USBDC_Device[devinx].EndpType[endpinx] & USB_EP_TYPE_MASK) == USB_EP_TYPE_ISOC)
		{
			direction |= USBDC_TABLE_ISO_MASK;
			USBDC_Device[devinx].EndpMaxSize[endpinx] = USB_HC_ISTL_BUF_SIZE - 2;
		}
#if (SYSTEM_EXTENDER_MSC_SUPPORT)
#if (SYSTEM_EXTENDER_TRANSMITTER)
		/* Extender Transmitter MSC double buffer */
		if (devinx == EXTENDER_MSC_Devinx)
		{
			if ((USBDC_Device[devinx].EndpType[endpinx] & USB_EP_TYPE_MASK) == USB_EP_TYPE_BULK)
			{
				if ((direction & USBDC_TABLE_DIR_MASK) == USBDC_TABLE_DIR_IN_MASK)
				{
					direction |= USBDC_TABLE_DOUBF_MASK;
				}
				if ((direction & USBDC_TABLE_DIR_MASK) == USBDC_TABLE_DIR_OUT_MASK)
				{
					direction |= USBDC_TABLE_DOUBF_MASK;
				}
			}
		}
#endif //#if (SYSTEM_EXTENDER_TRANSMITTER)
#endif //#if (SYSTEM_EXTENDER_MSC_SUPPORT)

		//This is for MSC class
#if ((SYSTEM_MSC_DEVICE_SUPPORT) && (SYSTEM_USB_HC_BURST == 0))
		if (devinx == USBHC_MSC_Devinx)
		{
			if ((USBDC_Device[devinx].EndpType[endpinx] & USB_EP_TYPE_MASK) == USB_EP_TYPE_BULK)
			{
				//if IN=> only one buffer
	#if (USB_HC_MSC_DC_IN_DBUF)
				if ((direction & USBDC_TABLE_DIR_MASK) == USBDC_TABLE_DIR_IN_MASK)
					direction |= USBDC_TABLE_DOUBF_MASK;
	#endif

	#if (USB_HC_MSC_DC_OUT_DBUF)
				if ((direction & USBDC_TABLE_DIR_MASK) == USBDC_TABLE_DIR_OUT_MASK)
					direction |= USBDC_TABLE_DOUBF_MASK;
	#endif
			}
		}
#endif
		if (USBDC_Device[devinx].EndpAddr[endpinx] || (endpinx ==0))
		{			
			//Add 2015-12-14 */
			skip = 0;
			if ((USBDC_Device[devinx].EndpAddr[endpinx] & USBDC_TABLE_DIR_IN_MASK) == 0x00) // direct OUT
			{
				index=endpinx+1;
				if (index < endpnum)
				{
					if ((USBDC_Device[devinx].EndpAddr[index] & 0x0f) == (USBDC_Device[devinx].EndpAddr[endpinx] & 0x0f))
					{
						skip = 1;			
					}	
				}	
			}	
			
			if (skip == 0)
			{					
			    USBDC_HAL_Malloc_Device_Endpx(devinx,endpinx,(USBDC_Device[devinx].EndpAddr[endpinx] & 0x0f),direction,USBDC_Device[devinx].EndpMaxSize[endpinx]);
		    }	
	    }
	}
	
	//Set Feature
	dc_cfg = (USBHC_CfgDesc_TypeDef *)USB_PDevice[devinx].Desc[CONFIG_DESC].Ptr;
	memset(USBDC_Device[devinx].Feature,(dc_cfg->bmAttributes | 0x80),sizeof(USBDC_Device[devinx].Feature));

	//Active the device in the DC
	USBDC_VIRTUAL_Fill_Device_Reg(devinx,UDC_PORT_ALL_ACT,0x00);	//Port0,Port1,Port2,Port3 Active the root hub device  
	return 1;
}

/**----------------------------------------------------------------------------
 * Function Name: USBDC_Virtual_Hub_DeviceMount_Control
 * Purpose: make the port connect status, in virtual hub port
 * Params :$devinx:the hc devinx, the virtual vdevice_id is same with hc
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void USBDC_Virtual_Hub_DeviceMount_Control(U8_T devinx,U8_T vhub_port,U8_T mounttype,U8_T active_port,U8_T mount_dev)
{
	U8_T pinx;
	U8_T hubPort;

	for (pinx = 0; pinx < USBDC_PORT_MAX; pinx++)
	{
		if (active_port & MOUNT_PORT[pinx])
		{						
			//2.Mount device
			if (mounttype == VDEV_MOUNT)
			{
				if ((USBDC_Device[devinx].VHub->PortStatus[pinx][vhub_port].wPortStatusH & H_PortStatus_Power)) // if current is power
				{
					USBDC_Device[devinx].VHub->PortStatus[pinx][vhub_port].wPortStatusL |= L_PortStatus_Connection;
					USBDC_Device[devinx].VHub->PortStatus[pinx][vhub_port].wPortStatusH &= ~(H_PortStatus_HighSpeed|H_PortStatus_LowSpeed);  //Force Full Speed device /*Importtant*/
					USBDC_Device[devinx].VHub->PortStatus[pinx][vhub_port].wPortChangeL |= L_PortChange_Connection; //report connection change
				}
				USBDC_Device[devinx].VHub->PortStatus[pinx][vhub_port].Devinx = mount_dev; //has device mount
				//Wakeup the upstream DC port......				
				USBDC_VHub_Update_StatusChange(devinx,pinx,pinx,1);
			}
			else
			{
				USBDC_Device[devinx].VHub->PortStatus[pinx][vhub_port].wPortStatusL &= ~(L_PortStatus_Connection|L_PortStatus_Enable);
				USBDC_Device[devinx].VHub->PortStatus[pinx][vhub_port].wPortStatusH &= ~(H_PortStatus_HighSpeed|H_PortStatus_LowSpeed);  //Force Full Speed device /*Importtant*/
				USBDC_Device[devinx].VHub->PortStatus[pinx][vhub_port].wPortChangeL |= (L_PortChange_Connection|L_PortChange_Enable); //report connection change
				USBDC_Device[devinx].VHub->PortStatus[pinx][vhub_port].Devinx = 0; //Device Gone
				//NOT Wakeup the upstream DC port......				
				USBDC_VHub_Update_StatusChange(devinx,pinx,pinx,0);
			}
		}
	}
	//Maintain the map of mount host 
	if (mounttype == VDEV_UNMOUNT)
	{
		(USB_PDevice[devinx].HUB.ReportState+vhub_port)->Devinx = 0;
		hubPort = USB_PDevice[mount_dev].Hub_NbrPorts USBHC_ROOTHUBPORT_PORT_NUM_READ;
		(USB_PDevice[USB_PDevice[mount_dev].UpperHubDevinx].HUB.ReportState+hubPort)->Devinx = 0;
		USB_PDevice[mount_dev].MountPort &= ~active_port;
	}
	else
		USB_PDevice[mount_dev].MountPort |= active_port;					
	USB_PDevice[mount_dev].Mounted   = mounttype;
	
}

/**----------------------------------------------------------------------------
 * Function Name: USBDC_Free_VDevice_Memory(U8_T devinx)
 * Purpose: free the memory alloc in this virtual device
 * Params :$devinx:the hc devinx, the virtual vdevice_id is same with hc
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void USBDC_Free_VDevice_Memory(U8_T devinx)
{
	U8_T	endpinx, endpnum, up;
	bit		free_endpbuf;

	//1.Check the used flag
	if ((USBDC_Device[devinx].DevIdx & DC_USED_MASK) == 0x00) // if device never be used
	{
		return;
	}

	//2.Clear DC register
	//2-1.disable device address
	//2-2.disable endp   address
	USBDC_Free_VDevice(devinx);

	//3.Clear DC Hub buffer memory
	if (USB_PDevice[devinx].DevClass == USB_HUB_CLASS)
	{
		malloc_free(USBDC_Device[devinx].VHub);
		USBDC_Device[devinx].VHub = 0;
	}

	//4.Clear Endp buffer memroy
	endpnum = USBDC_Device[devinx].EndpNum;
	for (up=0; up <  USBDC_PORT_MAX; up ++)
	{
		for (endpinx=0; endpinx < endpnum ; endpinx ++)
		{
			if ((USBDC_Device[devinx].EndpType[endpinx] & USB_EP_TYPE_MASK) != USB_EP_TYPE_ISOC)
			{
				free_endpbuf = 1;
#if (SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT)
#if (SYSTEM_USB_HC_BURST)
#if (SYSTEM_EXTENDER_TRANSMITTER)
				if (devinx == EXTENDER_MSC_Devinx)
#else
				if (devinx == USBHC_MSC_Devinx)
#endif //#if (SYSTEM_EXTENDER_TRANSMITTER)
				{					
					if ((USBDC_Device[devinx].EndpType[endpinx] & USB_EP_TYPE_MASK) == USB_EP_TYPE_BULK)
					{
						//BURST mode:the memory is not form damynic memory,so skip 
						free_endpbuf = 0;
					}
					else
					{
						if (up)
							free_endpbuf = 0;
					}
				}
#else // Non-Burst
#if (SYSTEM_EXTENDER_TRANSMITTER)
				if (devinx == EXTENDER_MSC_Devinx)
#else
				if (devinx == USBHC_MSC_Devinx)
#endif
				{
					if (up)
						free_endpbuf = 0;
				}
#endif /* (SYSTEM_USB_HC_BURST) */
#endif /* (SYSTEM_MSC_DEVICE_SUPPORT) */
#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
				if (devinx == USBDC_VMSC_DEVINX)
				{
					if (up)
						free_endpbuf = 0;
				}
#endif
				if (free_endpbuf)
				{
					if (USBDC_Device[devinx].EndpMaxSize[endpinx])
					{
						malloc_free(USBDC_EndpBufPtr[up][devinx][endpinx]);
						USBDC_EndpBufPtr[up][devinx][endpinx] = 0;
					}
				}
			}
		}
	}

	//5.Clear Virtual device table
	memset(&USBDC_Device[devinx],0x00,sizeof(USBDC_DeviceTypeDef)); // if device has been occupy by other device
}

/**----------------------------------------------------------------------------
 * Function Name: USBDC_Free_VDevice(U8_T devinx)
 * Purpose: reset the relative DC register & make device unmount.
 * Params :$devinx:the hc devinx
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------*/
void USBDC_Free_VDevice(U8_T devinx)
{
	U8_T pinx, einx, reg0[2], cir, hubportnumber;

	printf("clear_dc_a:%bu ",devinx);
	//Clear DC register
	for (pinx = 0; pinx < USBDC_PORT_MAX; pinx ++)
	{
		if ((USB_PDevice[devinx].MountPort & BIT_MASK[pinx]) == 0x00)
		{
			continue;
		}	
			
		//1.disable device address
		USBDC_HAL_Address_Active(pinx, devinx, RESET);
		
		//2.disable endp address
		for (einx = 0; einx < USBDC_Device[devinx].EndpNum; einx ++)
		{
			cir = DA_EP_CFGR[devinx] + (einx << 1);
			USBDC_Read_Regs(pinx, cir, reg0, 2);
			if (reg0[0] & DA_EP_CFGR_EPEN_SET)  // if the endp has been assign
			{
				reg0[0] &= ~DA_EP_CFGR_EPEN_SET;
				USBDC_Write_Regs(pinx, cir, reg0, 2);
			}
			
			USBDC_REGS_Endp_Reset(pinx, devinx, einx);
		}

		//3.Unmount device
		hubportnumber = USBDC_Virtual_Hub_Map[devinx].Devinx_HubPort;

		//portmask = 0x01 << pinx;
		cir = USBDC_VHUB_DEVINX; // if hub omit, then must the root hub to handle
		printf("clear_dc_b:%bu ",cir);
		if (USB_PDevice[cir].Addr & USBHC_DEVICE_USED_MASK) // if device is valid
		{
#if (SYSTEM_EXTENDER_MSC_SUPPORT)
			if ((EXTENDER_MSC_Devinx == devinx) && (pinx != KVM_CurrentMSC))
			{
				continue;
			}
#endif
#if (SYSTEM_MSC_DEVICE_SUPPORT)		
			if (USBHC_MSC_Devinx == devinx)
			{
				if (pinx != KVM_CurrentMSC)
				{	
					continue;
				}
				else
				{	
					if (USBHC_MSC_Reset_Flag) //if is caused by DC site
					{
						continue;
					}
				}			
			}							
#endif
#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)		
			if ((USBDC_VMSC_DEVINX == devinx) && (pinx != KVM_CurrentPEN))
			{
				continue;
			}
#endif
			
#ifdef ONEHID
			if (devinx > USBDC_VHID_DEVINX)
			{
				continue;
			}		
#endif			
			printf("clear_dc_c:%bu ",pinx);
			USBDC_Virtual_Hub_DeviceMount_Control(cir, hubportnumber, VDEV_UNMOUNT, BIT_MASK[pinx], devinx);
		}
	}
}

/**----------------------------------------------------------------------------
 * Function Name: USBDC_Search_EndpAddr(U8_T devidx,U8_Tendpaddr,U8_T *endpidx)
 * Purpose: used the endpaddr to retrive the EndpIndex id
 * Params :$devidx:the dc devinx
 *         $endpaddr:the target endpaddr
 *         $endpidx:the pointer for endpidx for return
 * Returns: FlagStatus- Set=1,Reset=0
 * Note:
 *----------------------------------------------------------------------------*/
FlagStatus USBDC_Search_EndpAddr(U8_T devidx,U8_T endpaddr,U8_T *endpidx)
{
	U8_T endpinx;

	if (endpaddr & 0x80)
	{	
		endpaddr &= ~0x80;
		endpaddr |= USBDC_TABLE_DIR_IN_MASK;
	}	
	
	for (endpinx = 1; endpinx < USBDC_Device[devidx].EndpNum; endpinx ++)
	{			
		if ((USBDC_Device[devidx].EndpAddr[endpinx] & (0x0f | USBDC_TABLE_DIR_IN_MASK) ) == endpaddr)	
		{			
			*endpidx = endpinx;  //endpidx 
			return SET;          //result found
		}
	}
	return RESET;    //result not found
}

 
#if (SYSTEM_MSC_DEVICE_SUPPORT)
/**----------------------------------------------------------------------------
 * void USBDC_Maintain_MSC_PDevice(void)
 * Purpose: when in DC isr, if the MSC bulk out can not move to TD buffer
 *          this function will keep on monitoring,once the td buffer is free
 *          then copy the buffer content into TD buffer.
 * Params : NONE
 * Returns: 
 * Note:
 *----------------------------------------------------------------------------*/ 
void USBDC_Maintain_MSC_PDevice(U8_T pid)
{
	U8_T index;

	if ((pid == KVM_CurrentMSC) && USBHC_MSC_Device_Cnt)
	{
		USBHC_MSC_Bulk_State_Reset(USBHC_MSC_Devinx);
		//Scan the ATL TD,skip the
		for (index = (USB_HC_ATL_START + USB_HC_MSC_START); index < (USB_HC_ATL_START+USB_HC_ATL_MAX); index++)
		{
			USBHC_CORE_Clear_TD_Register(index);
		}
		if (USBHC_MSC_DismountFlag ==0)
		{	
			TASK_USBHC_Device_Reset(pid,USBHC_MSC_Devinx);
		}	
	}
}
#endif //end of SYSTEM_MSC_DEVICE_SUPPORT)

/**----------------------------------------------------------------------------
 * U8_T USBDC_Check_Endp_Buffer_Valid(U8_T port,U8_T devinx,U8_T endpinx)
 * Purpose: Check the Bulk Buffer Valid id
 * Params : NONE
 * Returns: 
 * Note:
 *----------------------------------------------------------------------------*/
U8_T USBDC_Check_Endp_Buffer_Valid(U8_T port,U8_T devinx,U8_T endpinx)
{
	U8_T esmr,buf_cnt=0,daepcfgr[2];
	U8_T double_buf,dir;
		   
	//1.Check the Double Buffer
	esmr = DA_EP_CFGR[devinx]+(endpinx << 1);
	USBDC_Read_Regs(port,esmr,&daepcfgr,2);
	double_buf	= daepcfgr[0] & DA_EP_CFGR_DBUF_MASK;
	dir			= daepcfgr[0] & DA_EP_CFGR_DIR_MASK;
	
	//2.Check the Buffer valid id
	USBDC_Read_Regs(port,DA_EP_SR[devinx][endpinx],&esmr,1);

	if (double_buf)
	{
		buf_cnt = esmr & (DC_IESMR_BAVA0|DC_IESMR_BAVA1);
		if (dir == DA_EP_CFGR_DIR_OUT)
		{
			if (buf_cnt == 0x00)
				return 0;

			if (buf_cnt == (DC_IESMR_BAVA0|DC_IESMR_BAVA1))
				buf_cnt = 2;
			else
				buf_cnt = 1;

			buf_cnt <<= 4;
			if (esmr & DC_IESMR_CBUF) //Buffer 1 avaiable
			{
				if (esmr & DC_IESMR_BAVA1) // Buffer is empty, then skip
				{
					return (2|buf_cnt) ;
				}
			}
			else 
			{	
				if (esmr & DC_IESMR_BAVA0)
				{
					return (1|buf_cnt);
				}
			}
		}
		else
		{
			if (buf_cnt == (DC_IESMR_BAVA0|DC_IESMR_BAVA1))
				return 0;

			if (buf_cnt == 0x00)
				buf_cnt = 2;
			else
				buf_cnt = 1;

			buf_cnt <<= 4;
			if (esmr & DC_IESMR_CBUF) //Buffer 1 avaiable
			{
				if ((esmr & DC_IESMR_BAVA1) == 0x00) // Buffer is empty, then skip
				{
					return ((buf_cnt|2)) ;
				}
			}
			else 
			{
				if ((esmr & DC_IESMR_BAVA0) == 0x00)
				{
					return ((buf_cnt|1));
				}
			}
		}
	}
	else
	{
		if (dir == DA_EP_CFGR_DIR_OUT)
		{
			if ((esmr & DC_IESMR_BAVA0) == 0x00)
			{
				return 0x00;
			}
			else
				return 0x11;
		}
		else
		{
			if ((esmr & DC_IESMR_BAVA0) == 0x00)
			{
				return 0x11;
			}
			else
				return 0x00;
		}
	}
	return 0;
} 

/**----------------------------------------------------------------------------
 * U8_T USBDC_Get_Endp_Interval(U8_T port,U8_T devinx,U8_T endpinx)
 * Purpose: Calculate the endpoint interval time base on interval0, interval1
 * Params : NONE
 * Returns: 
 * Note:
 *----------------------------------------------------------------------------*/
U8_T USBDC_Get_Endp_Interval(U8_T port,U8_T devinx,U8_T endpinx)
{
	U8_T interval,interval0,interval1;

	interval0 = USBDC_Device[devinx].EndpInterval0[port][endpinx];
	interval1 = USBDC_Device[devinx].EndpInterval1[port][endpinx];

	if (interval0 > interval1)
	{
		interval = (0xff - interval0) + interval1 + 1;
	}
	else
	{
		interval = interval1 - interval0;
	}

	return interval;
}

/**----------------------------------------------------------------------------
 * void TASK_USBDC_Get_EndpInterval(void)
 * Purpose: Measure the endpoint interval time
 * Params : NONE
 * Returns: 
 * Note:
 *----------------------------------------------------------------------------*/
void TASK_USBDC_Get_EndpInterval(void)
{
	U8_T port,devinx,endpinx;
	U8_T interval;	 

	//Get the parameters first
	port	= TASK_Event  & 0x0f;
	devinx	= (TASK_Register0 & 0xf0) >> 4;
	endpinx	= TASK_Register0 & 0x0f;

	//Check the device
	interval = USBDC_Get_Endp_Interval(port,devinx,endpinx);
	if (interval == 0) //do next time
	{
		return;
	}

	USBDC_Device[devinx].Interval_Check[port] &= ~(0x01 << endpinx);
	USBDC_VIRTUAL_INNAK_Control(devinx,port,BIT_MASK[endpinx],0);
	TASK_Destory_Current();

	if (port == KVM_CurrentHost)
	{		
		USBHC_CORE_Change_IntTransfer_Interval(devinx,endpinx,interval);
	}
	//Should dislable the IN_NAK
}

/**----------------------------------------------------------------------------
 * void USBDC_Check_Upstream_Suspend_State(U8_T port,U8_T devinx)
 * Purpose: Measure the endpoint interval time
 * Params : NONE
 * Returns: 
 * Note:
 *----------------------------------------------------------------------------*/
U8_T USBDC_Check_Upstream_Suspend_State(U8_T port,U8_T devinx)
{ 
	U8_T port_suspend_flag=0;
	
	//Check the upstream is in resume mode
	if (USBDC_Resume_Task_ActiveID[port])
	{
		//printf("S");
		return 1;
	}
	
	/* Check Virtual Device suspend state */		
#ifdef RESUME_DELAY	
	if ((VDevice_Keep_Quite_Flag[port] & BIT_MASK[devinx]) == 0)
#endif
	{	
		if ((USBDC_Device[devinx].Feature[port] & FEATURE_SUSPEND)== FEATURE_SUSPEND)
		{
			//need to wake up host
			if ((USBDC_Device[devinx].Feature[port] & FEATURE_REMOTE)== FEATURE_REMOTE)
			{
				//printf("a");
				port_suspend_flag=1;
				USBDC_VHub_Resume_Port(port,devinx);									
			}
		}
	}
#ifdef RESUME_DELAY		
	else
	{
		//printf("b");
		port_suspend_flag=2;
	}		
#endif	
	/*Check Virtual Root HUB suspend */
#ifdef RESUME_DELAY		
	if ((DC_Keep_Quite_Flag & BIT_MASK[port]) == 0x00)		
#endif		
	{	
		if (USBDC_UpPortState[port] & USBDC_ROOTHUB_ATTACHED_MASK)
		{
			if (USBDC_UpPortState[port] & USBDC_ROOTHUB_SUSPEND_MASK)
			{
				USBDC_Port_Resume(port);				
				//printf("c");
				return 2;
			}
		}
	}	
#ifdef RESUME_DELAY	
	else
	{
		//printf("d");
		port_suspend_flag=3;
	}		
#endif	
	return port_suspend_flag;	
}

/*----------------------------------------------------------------------------
 * U8_T USBDC_Virtual_HubPort_Alloc(U8_T devinx)
 * Purpose: Measure the endpoint interval time
 * Params : NONE
 * Returns: 
 * Note:
 *----------------------------------------------------------------------------*/
U8_T USBDC_Virtual_HubPort_Alloc(U8_T devinx)
{
	U8_T index;
	
#if (SYSTEM_MSC_DEVICE_SUPPORT)	
	if ((USB_PDevice[devinx].DevClass == USB_MSC_CLASS) && (USBHC_MSC_Reset_Flag))
	{				 
		return (VHUB_MSC_Reset_HubPort + 1);
	}
	else
#else
	devinx = 0;
#endif		
	{	
		for (index = 0; index < USBDC_VHUB_PORT_NUM; index++)
		{
			if (USBDC_Virtual_Hub_Map[index].HubPort_Devinx) //device has use this table		
			{
				continue; // go next port
			}
			
			return index+1;						
		}
	}
	return 0; // not find
}

/*----------------------------------------------------------------------------
 * void USBDC_Virtual_Hub_Reset(U8_T pid)
 * Purpose: Measure the endpoint interval time
 * Params : NONE
 * Returns: 
 * Note:
 *----------------------------------------------------------------------------*/
void USBDC_Virtual_Hub_Reset(U8_T pid,U8_T force)
{
	/*01.Foruce USB bus Reset */
	if (force)
	{	
		switch(pid)
		{
			case 0:	
				_USBDC_DC0DR_SFR(DC_CR_FSE0_SET);
				_USBDC_DC0CIR_SFR(DCCR);
				break;
			case 1:	
				_USBDC_DC1DR_SFR(DC_CR_FSE0_SET);
				_USBDC_DC1CIR_SFR(DCCR);
				break;
			case 2:
				_USBDC_DC2DR_SFR(DC_CR_FSE0_SET);
				_USBDC_DC2CIR_SFR(DCCR);
				break;
			case 3:
				_USBDC_DC3DR_SFR(DC_CR_FSE0_SET);
				_USBDC_DC3CIR_SFR(DCCR);
		}
	}
	
	/*02.Handle the virtual HUB part */
	if (USBDC_UpPortState[pid] & USBDC_ROOTHUB_ATTACHED_MASK)
	{
#ifdef RESUME_DELAY			
		//For Resume Delay		
		DC_Keep_Quite_Flag &= ~BIT_MASK[pid];
		VDevice_Keep_Quite_Flag[pid] = 0;
#endif		
		USBDC_Port_Reset(pid);
		USBDC_Port_Regs_Clear(pid);
		USBDC_VIRTUAL_Port_Clear(pid,0,USBDC_DEVICE_MAX);
		USBDC_VHub_UpPort_Reset(USBDC_VHUB_DEVINX,pid);
		USBDC_HAL_Address_Active(pid,0,SET);
		#ifdef KMLOG
		KMLog_Send_Host_State();
		#endif
	}	
	
	/* Clear the virutal HUB relative */		
	/*01.Foruce USB bus Reset */
	if (force)
	{		
		TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_DC_Reset_Complete_TaskID,0,pid,200,100);				
		/*
		switch(pid)
		{
			case 0:	
				_USBDC_DC0DR_SFR(DC_CR_DCEN_SET|DC_CR_SRSM_SET);
				_USBDC_DC0CIR_SFR(DCCR);
				break;
			case 1:	
				_USBDC_DC1DR_SFR(DC_CR_DCEN_SET|DC_CR_SRSM_SET);
				_USBDC_DC1CIR_SFR(DCCR);
				break;
			case 2:
				_USBDC_DC2DR_SFR(DC_CR_DCEN_SET|DC_CR_SRSM_SET);
				_USBDC_DC2CIR_SFR(DCCR);
				break;
			case 3:
				_USBDC_DC3DR_SFR(DC_CR_DCEN_SET|DC_CR_SRSM_SET);
				_USBDC_DC3CIR_SFR(DCCR);
		}
		*/
	}
}

#ifdef RESUME_DELAY	
/*----------------------------------------------------------------------------
 * void TASK_VDevice_Keep_Quite(void)
 * Purpose: Measure the endpoint interval time
 * Params : NONE
 * Returns: 
 * Note:
 *----------------------------------------------------------------------------*/
void TASK_VDevice_Keep_Quite(void)
{
	U8_T  devinx,port;
	
	devinx = TASK_Register0;
	port = TASK_Event & 0x0f;
	
	if (USBDC_UpPortState[KVM_CurrentHost] & USBDC_ROOTHUB_SUSPEND_MASK)
	{
		//printf("DCPort(%bu) Suspend,Go quite period\n\r",KVM_CurrentHost);				
	}		
	else
	{
		//printf("Clear Port(%bu) Devinx(%bu), Keep quite flag\n\r",port,devinx);		
		VDevice_Keep_Quite_Flag[port] &= ~BIT_MASK[devinx]; 
	}			
	if (TASK_DC_Keep_Quite_Active_TaskID[port] == 0)
		TASK_DC_Keep_Quite_Active_TaskID[port] = TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_DC_Keep_Quite_TASKID,0,port,VBUS_RESUME_KEEP_QUITE,VBUS_RESUME_KEEP_QUITE); // now active the Generic KB Output Handle	
	TASK_Destory_Current();	
}	

/*----------------------------------------------------------------------------
 * void TASK_DC_Keep_Quite(void)
 * Purpose: Measure the endpoint interval time
 * Params : NONE
 * Returns: 
 * Note:
 *----------------------------------------------------------------------------*/
void TASK_DC_Keep_Quite(void)
{
	U8_T port,index;
	
	port = TASK_Register0;

	DC_Keep_Quite_Flag &= ~BIT_MASK[port];		
	for (index=1; index < USBDC_DEVICE_MAX; index++)
	{
		VDevice_Keep_Quite_Flag[port] &= ~BIT_MASK[index]; 
	}	
	//printf("DCPort(%bu), VBUS quite terminated\n\r",port);						
	TASK_DC_Keep_Quite_Active_TaskID[port] = 0;
	TASK_Destory_Current();	
}
#endif

/*----------------------------------------------------------------------------
 * void TASK_DC_Reset_Complete(void)
 * Purpose: 
 * Params : NONE
 * Returns: 
 * Note:
 *----------------------------------------------------------------------------*/
void TASK_DC_Reset_Complete(void)
{
	TASK_Destory_Current();
	switch(TASK_Register0)
	{
		case 0:	
			_USBDC_DC0DR_SFR(DC_CR_DCEN_SET|DC_CR_SRSM_SET);
			_USBDC_DC0CIR_SFR(DCCR);
			break;
		case 1:	
			_USBDC_DC1DR_SFR(DC_CR_DCEN_SET|DC_CR_SRSM_SET);
			_USBDC_DC1CIR_SFR(DCCR);
			break;
		case 2:
			_USBDC_DC2DR_SFR(DC_CR_DCEN_SET|DC_CR_SRSM_SET);
			_USBDC_DC2CIR_SFR(DCCR);
			break;
		case 3:
			_USBDC_DC3DR_SFR(DC_CR_DCEN_SET|DC_CR_SRSM_SET);
			_USBDC_DC3CIR_SFR(DCCR);
	}		
}	
/* End of usbdc.c */

