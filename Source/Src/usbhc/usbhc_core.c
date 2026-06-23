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
 * Module Name : usbhc_core.c
 * Purpose     : The USB host controller core handle program
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	<string.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	"project_include.h" 

/* CONSTANT VARIABLE DECLARATIONS */ 
//const U8_T	BYTE_MASK[]={0x00,0x08,0x10,0x18};
const U8_T	BIT_MASK[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

const U8_T	USBHC_RootHub_Cmd_Reg[USB_HC_MAX_PORT] =
{
#ifdef MCU_TYPE_AX68002
	HC_ROOTHUB0_CON_REG,
	HC_ROOTHUB1_CON_REG
#else
	HC_ROOTHUB0_CON_REG,
	HC_ROOTHUB1_CON_REG,
	HC_ROOTHUB2_CON_REG,
	HC_ROOTHUB3_CON_REG
#endif
};

const U8_T	USBHC_RootHub_Status_Reg[USB_HC_MAX_PORT] =
{
#ifdef MCU_TYPE_AX68002
	HC_ROOTHUB0_STATE_REG,
	HC_ROOTHUB1_STATE_REG
#else
	HC_ROOTHUB0_STATE_REG,
	HC_ROOTHUB1_STATE_REG,
	HC_ROOTHUB2_STATE_REG,
	HC_ROOTHUB3_STATE_REG
#endif
};

/* STATIC VARIABLE DECLARATIONS */
U8_T	TASK_USBHC_Handle_Control_ID;
U8_T	TD_INTL_Skip_Map[4],TD_ATL_Skip_Map[4],TD_INTL_Reload_Map[4];
U8_T	TD_ATL_Done_Map[4];
U8_T	TD_INTL_Done_Map[4];
U8_T	USBHC_ErrorState;
U16_T   MSC_Nake_Cnt=0;

bit		USBHC_IntlDoneFlag = 0;
bit		USBHC_AtlDoneFlag = 0;
bit     USBHC_MSC_DismountFlag=0;

//---------------------------------------------
// Done Queue Handle 
// Bit7~5 Devinx(0~7)
// Bit4~0 TD_ID (0~31)
U8_T	USBHC_BULK_Done_Q[ATL_DONE_QUEUE_MAX];
U8_T	USBHC_ATL_Done_Q_Rp,USBHC_ATL_Done_Q_Wp;
U8_T	TD_ISTL_Skip_Map,TD_ISTL_Reload_Map;

//------------------------------------------------------------------------------------------------
//$ For ISR Handle
volatile USBHC_ISRFIFO_TypeDef		USBHC_ISR_FIFO[USB_HC_MAX_ISR_FIFO_DEPTH]; //keep 64 depth isr q status
volatile U8_T						USBHC_ISRQ_Rp,USBHC_ISRQ_Wp;
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//$ For HC Task Handle

//---------------------------------------------
// Maximum device Handle by other module
U8_T	USBHC_Current_Total_Device_Count = 0;
U8_T	USBHC_Max_Device_Support 		 = 7;
U8_T	USBHC_Current_Total_Hub_Count	 = 0;
//------------------------------------------------------------------------------------------------
//$ For HC TD Activity Monitor
U8_T	USBHC_TD_Activity_Cnt;
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
/* LOCAL SUBPROGRAM DECLARATIONS */
extern const U8_T	DA_CR[USBDC_DEVICE_MAX];
extern U8_T			USBHC_HUB_Enumerate_Occupy_Flag;
extern U8_T			USBHC_HUB_Enumerate_Occupy_Devinx;

/* LOCAL SUBPROGRAM BODIES */
static void usbhc_core_ATL_Done_Handle(U8_T devinx, U8_T atl_id);
void TASK_USBHC_CORE_BULK_Retry_Handle(void);
/*
 * ----------------------------------------------------------------------------
 * void USBHC_CORE_Init(void)
 * Purpose : 1.Assign TD buffer address into register(0x31)
 *           2.Assign different TD buffer information into register
 *             a.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void USBHC_CORE_Init(void)
{
	U8_T reg[4],index;

#if (SYSTEM_EXTENDER_TRANSMITTER)
  #if (!EXTENDER_T_HC_ENABLE)
	USBHC_ISRQ_Rp = 0;
	USBHC_ISRQ_Wp = 0;
	
	USBHC_ATL_Done_Q_Rp  = 0;
	USBHC_ATL_Done_Q_Wp  = 0;

	memset(TD_INTL_Skip_Map,0x00,sizeof(TD_INTL_Skip_Map));
	
	memset(TD_INTL_Reload_Map,0x00,sizeof(TD_INTL_Reload_Map));
	
	memset(TD_ATL_Skip_Map,0xff,sizeof(TD_ATL_Skip_Map));
	
	//6. Generate a task for handleing the irq_request  
	TASK_USBHC_Handle_Control_ID = TASK_Create(TASK_USBHC_CORE_Control_Transfer_Handle);
	
	USBHC_Current_Total_Device_Count = 0;
	
	return;
  #endif
#endif

	//--------------------------------------------------------------
	//1.Do the Host controller reset first
	//2.Move the HC to Operational mode
	reg[0] = HCFS_OPERATIONAL | HC_RWE_SET;
	USBHC_Write_Regs(HC_CONTROL_REG,reg,1);

	//--------------------------------------------------------------
	//2.Assign TD buffer start address into register
	//  default is 0x00, so just skip it
	reg[0] = 0x00;
	USBHC_Write_Regs(HC_BUF_START_ADDR_REG,reg,1);

	//--------------------------------------------------------------
	//3.Assign ISTL Buffer size
	reg[0] = USB_HC_ISTL_BUF_SIZE_LSB;
	reg[1] = USB_HC_ISTL_BUF_SIZE_MSB;
	USBHC_Write_Regs(HC_ISTL_BUF_SIZE_REG,reg,2);

	//3-2.Assign ISTL Block size
	reg[0] = USB_HC_ISTL_BLK_SIZE_LSB;
	reg[1] = USB_HC_ISTL_BLK_SIZE_MSB;
	USBHC_Write_Regs(HC_ISTL_BLK_SIZE_REG,reg,2);

	//--------------------------------------------------------------
	//3-1.Assign INTL Buffer size
	reg[0] = USB_HC_INTL_BUF_SIZE_LSB;
	reg[1] = USB_HC_INTL_BUF_SIZE_MSB;
	USBHC_Write_Regs(HC_INTL_BUF_SIZE_REG,reg,2);

	//--------------------------------------------------------------
	//3-2.Assign INTL Block size
	reg[0] = USB_HC_INTL_BLK_SIZE_LSB;
	reg[1] = USB_HC_INTL_BLK_SIZE_MSB;
	USBHC_Write_Regs(HC_INTL_BLK_SIZE_REG,reg,2);

	//--------------------------------------------------------------
	//3-3.Assign ATL Buffer size
	reg[0] = USB_HC_ATL_BUF_SIZE_LSB;
	reg[1] = USB_HC_ATL_BUF_SIZE_MSB;
	USBHC_Write_Regs(HC_ATL_BUF_SIZE_REG,reg,2);

	//--------------------------------------------------------------
	//3-4.Assign ATL Block size
	reg[0] = USB_HC_ATL_BLK_SIZE_LSB;
	reg[1] = USB_HC_ATL_BLK_SIZE_MSB;
	USBHC_Write_Regs(HC_ATL_BLK_SIZE0_REG,reg,2);
#if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST))
    reg[0] = USB_HC_BULK_BLK_SIZE_LSB;
	reg[1] = USB_HC_BULK_BLK_SIZE_MSB;
#endif	/* #if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST)) */
	USBHC_Write_Regs(HC_ATL_BLK_SIZE1_REG,reg,2);

	reg[0] = 0;
	reg[1] = 0;
	reg[2] = 0;
	reg[3] = 0;
#if ((SYSTEM_MSC_DEVICE_SUPPORT) && (SYSTEM_USB_HC_BURST))
	USBHC_CORE_SetBit(reg,USB_HC_MSC_BURST_OUT_TD0,1);
	USBHC_CORE_SetBit(reg,USB_HC_MSC_BURST_OUT_TD1,1);
	USBHC_CORE_SetBit(reg,USB_HC_MSC_BURST_IN_TD0,1);
	USBHC_CORE_SetBit(reg,USB_HC_MSC_BURST_IN_TD1,1);
#endif /* #if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST)) */
	USBHC_Write_Regs(HC_ATL_BLK_SIZE_INDEX_MAP_REG,reg,4);

	//--------------------------------------------------------------
	//4.Do the Root Hub Initial operation
	USBHC_RootHub_Init();

	//--------------------------------------------------------------
	//5.HC relative interrupt Enable
	//5-1.Reset the ISR Queue FIFO Pointer
	USBHC_ISRQ_Rp = 0;
	USBHC_ISRQ_Wp = 0;

	//5-2.Setting the ATL TD Done Threshold Count Register
	reg[0] = 0x01 & HC_ATL_TD_DONE_COUNT_MASK;
	USBHC_Write_Regs(HC_ATL_TD_DONE_COUNT_REG,reg,1);

	//5-3.Setting the ATL TD Done Threshold Time Out Register
	reg[0] = 0x00;
	USBHC_Write_Regs(HC_ATL_TD_DONE_TIMEOUT_REG,reg,1);

	//5-4.Setting INTL TD Skip Map Register
	memset(TD_INTL_Skip_Map,0x00,sizeof(TD_INTL_Skip_Map));
	for (index=0;index < USB_HC_INTL_MAX ;index++)
	{
		USBHC_CORE_SetBit(TD_INTL_Skip_Map,index,0x01); // set skip to true
	}

	//5-5.Setting INTL TD Relaod Map Register
	memset(TD_INTL_Reload_Map,0x00,sizeof(TD_INTL_Reload_Map));
	USBHC_Write_Regs(HC_INTL_TDSKIP_MAP_REG,TD_INTL_Skip_Map,4);

	//5-6.Setting ISTL TD Skip Map Register
	TD_ISTL_Skip_Map = 0xff;
	USBHC_Write_Regs(HC_ISTL_TDSKIP_MAP_REG,&TD_ISTL_Skip_Map,1);

	//5-7.Setting ISTL TD Relaod Map Register
	TD_ISTL_Reload_Map = 0x00;
	USBHC_Write_Regs(HC_ISTL_TD_RELOAD_REG,&TD_ISTL_Reload_Map,1);

	//5-5.Setting ATL TD Skip Map Register
	memset(TD_ATL_Skip_Map,0xff,sizeof(TD_ATL_Skip_Map));
	for (index=0;index < USB_HC_ATL_MAX ;index++)
	{
		USBHC_CORE_SetBit(TD_ATL_Skip_Map,index,0x01); // set skip to true
	}

	USBHC_Write_Regs(HC_ATL_TDSKIP_MAP_REG,TD_ATL_Skip_Map,4);

	//5-6.Setting INTL Last TD Register
	memset(reg,0x00,sizeof(reg));
	USBHC_CORE_SetBit(reg,(USB_HC_INTL_MAX-1),0x01); // set INTL last td
	USBHC_Write_Regs(HC_INTL_TDLAST_MAP_REG,reg,4);

	//5-a.Setting ISTL Last TD Register
	reg[0] = 0x00;
	USBHC_CORE_SetBit(reg,(USB_HC_ISTL_MAX-1),0x01); // set INTL last td
	USBHC_Write_Regs(HC_ISTL_TDLAST_MAP_REG,reg,1);

	//5-7.Setting ATL Last TD Register
	memset(reg,0x00,sizeof(reg));
	USBHC_CORE_SetBit(reg,(USB_HC_ATL_MAX-1),0x01); // set INTL last td
	USBHC_Write_Regs(HC_ATL_TDLAST_MAP_REG,reg,4);

	//5-8.Operation Interupt Enable
	//Enable (1).Overrun (2).Resume (3).P0~P3 Root Hub Port Status Change
	reg[0] = HC_OPINT_CON_SOE_ENABLE|HC_OPINT_CON_RDE_ENABLE|HC_OPINT_CON_RHSCE_ENABLE;
	USBHC_Write_Regs(HC_OPINT_CONTROL_REG,reg,1);

	//5-9.Interupt Enable
	//Enable (1).ISTL0 (2).ISTL1 (3).Operation Interrupt (4).Suspend (5).INTL (6).ATL
	reg[0] = HC_INT_CON_ISTL0_ENABLE|HC_INT_CON_OPR_ENABLE|HC_INT_CON_SUS_ENABLE|HC_INT_CON_INTL_ENABLE|HC_INT_CON_ATL_ENABLE;
	USBHC_Write_Regs(HC_INT_CONTROL_REG,reg,1);

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	reg[0] = HC_BUF_STATE_ISTL0_SET;
#else
	reg[0] = 0;
#endif 
	
	USBHC_INT_DISABLE;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	EXTINT3_DISABLE;
#endif

	USBHC_TD_Activity_Cnt = 0;
	
	reg[0] |= HC_BUF_STATE_ATLATV_SET|HC_BUF_STATE_INTLATV_SET;
	_USBHC_HCDR_SFR(reg[0]);
	_USBHC_HCCIR_SFR(HC_BUF_STATE_REG);
	USBHC_INT_ENABLE;

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	EXTINT3_ENABLE;
#endif

	//6. Generate a task for handleing the irq_request  
	TASK_USBHC_Handle_Control_ID = TASK_Create(TASK_USBHC_CORE_Control_Transfer_Handle);
	USBHC_ATL_Done_Q_Rp  = 0;
	USBHC_ATL_Done_Q_Wp  = 0;

	/*06.Enable Interrupt 2 */
#if (INTR_ISR_INCLUDE_USBHC)
	EXTINT2_ENABLE;
#endif

	USBHC_Current_Total_Device_Count = 0;

#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("TASK_USBHC_Handle_Control_ID=%bu\n\r",TASK_USBHC_Handle_Control_ID);	
#endif	
} /* End of USBHC_CORE_Init */

/* LOCAL SUBPROGRAM BODIES */
/*---------------------------------------------------------------------------------------
 * static void usbhc_ISR(void)
 * Purpose : ISR handle program
 *           The ISR entry interrupt is intr_UsbHostControllerIntr2ISR() use INT2_VECTOR
 * Params  : none
 * Returns : none
 * Note    : none
 *---------------------------------------------------------------------------------------*/
void usbhc_ISR(void) interrupt INT2_VECTOR
{
	idata U8_T int_state,op_state,portindex,portmask,op_state_bk; 
	idata U8_T	usbhc_IntrStateFlag;
#if ((SYSTEM_USBAUDIO_DEVICE_SUPPORT) && (SYSTEM_EXTENDER_RECEIVER))
	volatile U8_T XDATA	temp[2];
	U16_T XDATA*		ptempshort = (U16_T XDATA*)&temp[0];
	U8_T				len;
	U8_T XDATA*			pbuf;
#endif

	//Read In Interrupt Status
	int_state = USBHC_INTERRUPT_STATUS; // This SFR address

#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
 #if (PS2_WAKEUP_SUPPORT)
	if (!(int_state & HC_INT_STATE_SUS_MASK))
	{
		PowerSavingInhibitFlag = 1;
	}
 #endif
#endif

	//Check SOF Interrupt
	usbhc_IntrStateFlag = (int_state & ~(HC_INT_STATE_SOF_SET | HC_INT_STATE_ISTL0_SET));

	//Check Operational Interrupt
	if (int_state & HC_INT_STATE_OPR_SET)
	{
		//-------------------------------
		// Check OPR Interrupt 
		//-------------------------------
		//1.Read IN Operation Interrupt Status
		op_state = HCOIS;
		op_state_bk = op_state;
		USBHC_ISR_FIFO[USBHC_ISRQ_Wp].OP_ISR_State = op_state;

		//2.Check Operation Interrupt Status
		if (op_state & HC_OPINT_STATE_RHSC_MAP_MASK)
		{
			//1.Check Port0 status changed condition
			portmask = HC_ROOTHUB_STATUSCHANGED_PORT0_MASK;
			for (portindex = 0; portindex < USB_HC_MAX_PORT; portindex++)
			{
				if (op_state & portmask)
				{
					//(1).Read in the PORT Change status
					_USBHC_HCCIR_SFR(USBHC_RootHub_Status_Reg[portindex]); // write out the SFR address
					_USBHC_HCDR_READ_SFR(USBHC_RootHub_State_Report[portindex]); // read in status
					_USBHC_HCDR_READ_SFR(USBHC_RootHub_StatusChanged_Report[portindex]); // read in port status changed 

					//(2).Clear the Hub Status Changed
					_USBHC_HCDR_SFR(0x00);
					_USBHC_HCDR_SFR(USBHC_RootHub_StatusChanged_Report[portindex]);
					_USBHC_HCCIR_SFR(USBHC_RootHub_Cmd_Reg[portindex]);
				}

				op_state_bk &= ~portmask;
				if (op_state_bk == 0)
				{
					break;
				}

				portmask = portmask << 1; // notice this must check with SFR
			}
		}

		//Final.Clear OP_Int status
		HCOIS = op_state;
	}
	else
	{
		USBHC_TD_Activity_Cnt++;
	}

	//Special Handle for ISO handle
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	if (int_state & HC_INT_STATE_ISTL0_SET)
	{
		//Read the DoneMap
		_USBHC_HCCIR_SFR(HC_ISTL_TDDONE_MAP_REG);
		_USBHC_HCDR_READ_SFR(op_state_bk);

		//Handle the Isochronous Out Data Package
		if (op_state_bk & USB_HC_ISTL_OUT_DONE_MASK) //Isochronous In ISTL[0]
		{
#if (SYSTEM_EXTENDER_RECEIVER)			
			if (KVM_CurrentUSBAudio == REMOTE_HOST_PORT)
			{
				ETDR_AudioOutHcDoingFlag = 0;
				while (1)
				{
					if (ETDR_AudioOutHead != ETDR_AudioOutTail)
					{
						len = ETDR_AudioOutBuf[ETDR_AudioOutTail][0];
						if (len)
						{
							/* set source address indirect register */
							*ptempshort = (U16_T)ETDR_AudioOutBuf[ETDR_AudioOutTail];
							DDR = temp[1];
							DDR = temp[0];
							DDR = 0;
							DCIR = SW_DMA_SOURCE_ADDR_REG;

							/* set target address indirect register */
							*ptempshort = (U16_T)HCTD_Table.ISTL[0].Buf;
							DDR = temp[1];
							DDR = temp[0];
							DDR = 0;
							DCIR = SW_DMA_TARGET_ADDR_REG;

							/* set length */
							DDR = len +2;
							DDR = 0;
							DCIR = SW_DMA_BYTE_COUNT_REG;

							DDR = DMA_CMD_GO;
							DCIR = SW_DMA_CMD_REG;

							/* check software dma completed. */
							while (1)
							{
								DCIR = SW_DMA_STATUS_REG;
								temp[0] = DDR;
								if (temp[0] & DMA_ERROR_STU)
								{
									temp[1] = 0;
									break;
								}
								else if (temp[0] & DMA_COMPLETE_STU)
								{
									temp[1] = 1;
									break;
								}
							}

							ETDR_AudioOutTail++;
							ETDR_AudioOutTail &= EXTENDER_AUDIO_OUT_BUF_MASK;

							if (temp[1])
							{
								HCTD_Table.ISTL[0].TD.Byte0_Actual_Byte = 0;
								HCTD_Table.ISTL[0].TD.Byte2_MaxPKT_Size = len;
								HCTD_Table.ISTL[0].TD.Byte3 &= 0xFC;
								HCTD_Table.ISTL[0].TD.Byte4_Total_Byte = len;
								HCTD_Table.ISTL[0].TD.Byte5 &= 0xFC;
								HCTD_Table.ISTL[0].TD.Byte1 |= TD_ACTIVED_MASK;
								TD_ISTL_Skip_Map &= ~0x01;

								//Clear Done Map
								_USBHC_HCDR_SFR(0x01);
								_USBHC_HCCIR_SFR(HC_ISTL_TDDONE_MAP_REG);

								//Clear Skip Map
								_USBHC_HCDR_SFR(TD_ISTL_Skip_Map);
								_USBHC_HCCIR_SFR(HC_ISTL_TDSKIP_MAP_REG);
								ETDR_AudioOutHcDoingFlag = 1;
							}
							break;
						}
						else
						{
							ETDR_AudioOutTail++;
							ETDR_AudioOutTail &= EXTENDER_AUDIO_OUT_BUF_MASK;
						}
					}
					else
					{
						break;
					}
				} /* while(1) */
			} /* REMOTE_HOST_PORT */
			else
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */				
			{
				//Clear Done Map
				_USBHC_HCDR_SFR(USB_HC_ISTL_OUT_DONE_MASK);
				_USBHC_HCCIR_SFR(HC_ISTL_TDDONE_MAP_REG);
				
				//Set Skip map
				TD_ISTL_Skip_Map |= USB_HC_ISTL_OUT_DONE_MASK; // Skip ISL0
				_USBHC_HCDR_SFR(TD_ISTL_Skip_Map);
				_USBHC_HCCIR_SFR(HC_ISTL_TDSKIP_MAP_REG);
			}
		} /* USB_HC_ISTL_OUT_DONE_MASK */

		//Handle the Isochronous In Data Package
		if (op_state_bk & USB_HC_ISTL_IN_DONE_MASK) //Isochronous In ISTL[1]
		{
#if (SYSTEM_EXTENDER_RECEIVER)
			if (KVM_CurrentUSBAudio == REMOTE_HOST_PORT)
			{
				len = HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte0_Actual_Byte;
				if (len == 0)
				{
					if ((HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte1 & 0xf0) == 0x50)
					{
						ETDR_AudioInWaitFlag = 1;
					}
					else
					{
						TD_ISTL_Skip_Map &= ~(USB_HC_ISTL_IN_DONE_MASK);   // Skip ISTL3

						//4.Enable the TD Buffer
						//HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte0_Actual_Byte = 0;
						HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte2_MaxPKT_Size = USB_HC_ISTL_BUF_SIZE - 2;
						HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte3 &= 0xFC;
						HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte4_Total_Byte  = USB_HC_ISTL_BUF_SIZE-2;	//get the byte will not over the bufer size
						HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte5 &= 0xFC;
						HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte1 |= TD_ACTIVED_MASK;
						HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte7  = 0;

						//Clear Done Map
						_USBHC_HCDR_SFR(USB_HC_ISTL_IN_DONE_MASK);
						_USBHC_HCCIR_SFR(HC_ISTL_TDDONE_MAP_REG);

						//Clear Skip Map
						_USBHC_HCDR_SFR(TD_ISTL_Skip_Map);
						_USBHC_HCCIR_SFR(HC_ISTL_TDSKIP_MAP_REG);
					}
				}
				else
				{
					while (1)
					{
						pbuf = (U8_T XDATA*)ETDR_AudioInBuf[ETDR_AudioInHead];
						ETDR_AudioInHead++;
						ETDR_AudioInHead &= EXTENDER_AUDIO_IN_BUF_MASK;
						if (ETDR_AudioInHead == ETDR_AudioInTail)
						{
							ETDR_AudioInHead--; 
							ETDR_AudioInHead &= EXTENDER_AUDIO_IN_BUF_MASK;
							ETDR_AudioInTailSkipFlag = 1;
							break;
						}

						HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].Buf[0] = len;

						/* set source address indirect register */
						*ptempshort = (U16_T)HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].Buf;
						DDR = temp[1];
						DDR = temp[0];
						DDR = 0;
						DCIR = SW_DMA_SOURCE_ADDR_REG;

						/* set target address indirect register */
						*ptempshort = (U16_T)(pbuf + EXTENDER_PACKET_HEADER_DATA_OFFSET);
						DDR = temp[1];
						DDR = temp[0];
						DDR = 0;
						DCIR = SW_DMA_TARGET_ADDR_REG;

						/* set length */
						DDR = len +2;
						DDR = 0;
						DCIR = SW_DMA_BYTE_COUNT_REG;

						DDR = DMA_CMD_GO;
						DCIR = SW_DMA_CMD_REG;

						/* check software dma completed. */
						while (1)
						{
							DCIR = SW_DMA_STATUS_REG;
							temp[0] = DDR;
							if (temp[0] & DMA_ERROR_STU)
							{
								temp[1] = 0;
								break;
							}
							else if (temp[0] & DMA_COMPLETE_STU)
							{
								temp[1] = 1;
								break;
							}
						}
						if (!temp[1])
						{
							ETDR_AudioInHead--; 
							ETDR_AudioInHead &= EXTENDER_AUDIO_IN_BUF_MASK;
						}
						break;
					}	// while(1)

					TD_ISTL_Skip_Map &= ~(USB_HC_ISTL_IN_DONE_MASK);   // Skip ISTL3

					//4.Enable the TD Buffer
					HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte0_Actual_Byte = 0;
					HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte2_MaxPKT_Size = USB_HC_ISTL_BUF_SIZE - 2;
					HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte3 &= 0xFC;
					HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte4_Total_Byte  = USB_HC_ISTL_BUF_SIZE-2;	//get the byte will not over the bufer size
					HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte5 &= 0xFC;
					HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte1 |= TD_ACTIVED_MASK;
					HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte7  = 0;

					//Clear Done Map
					_USBHC_HCDR_SFR(USB_HC_ISTL_IN_DONE_MASK);
					_USBHC_HCCIR_SFR(HC_ISTL_TDDONE_MAP_REG);

					//Clear Skip Map
					_USBHC_HCDR_SFR(TD_ISTL_Skip_Map);
					_USBHC_HCCIR_SFR(HC_ISTL_TDSKIP_MAP_REG);
				}
			}
			else
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */
			{
				if (USBHC_Audio_IN_Start_Flag)
				{	
					//Get the length of data package
					HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].Buf[0] = HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte0_Actual_Byte;
					
					// If received length is 0 by isocronous IN, re-trigger the TD
					if (HCTD_Table.ISTL[USB_HC_ISTL_OUT_MAX].TD.Byte0_Actual_Byte == 0)
					{
						USBHC_AUDIO_In_Start();
					}
					
					//Set the DC Buffer Ready
					op_state = DA_CR_BVLD_SET | (USBHC_Audio_In_Endpinx & 0x07);
					switch (KVM_CurrentUSBAudio)
					{
						case 0:
							_USBDC_DC0DR_SFR(op_state);
							_USBDC_DC0CIR_SFR(DA_CR[USBHC_Audio_Devinx]);
							break;
						case 1:
							_USBDC_DC1DR_SFR(op_state);
							_USBDC_DC1CIR_SFR(DA_CR[USBHC_Audio_Devinx]);
							break;
						case 2:
							_USBDC_DC2DR_SFR(op_state);
							_USBDC_DC2CIR_SFR(DA_CR[USBHC_Audio_Devinx]);
							break;
						case 3:
							_USBDC_DC3DR_SFR(op_state);
							_USBDC_DC3CIR_SFR(DA_CR[USBHC_Audio_Devinx]);
							break;
					}
				}	
			}
		}
	}
#endif //#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)

	// check INTL Done Flag
	if (USBHC_IntlDoneFlag)
	{
		usbhc_IntrStateFlag &= ~HC_INT_STATE_INTL_SET;
	}
	else if (usbhc_IntrStateFlag & HC_INT_STATE_INTL_SET)
	{
		USBHC_IntlDoneFlag = 1;
	}

	// check ATL Done Flag
	if (USBHC_AtlDoneFlag)
	{
		usbhc_IntrStateFlag &= ~HC_INT_STATE_ATL_SET;
	}
	else if (usbhc_IntrStateFlag & HC_INT_STATE_ATL_SET)
	{
		USBHC_AtlDoneFlag = 1;
	}

	//------------------------------------------------------------------------
	// USBHC ISR FIFO Table Pointer Maintain
	//------------------------------------------------------------------------
	if (usbhc_IntrStateFlag)
	{
		USBHC_ISR_FIFO[USBHC_ISRQ_Wp].ISR_State = usbhc_IntrStateFlag;

		if (++USBHC_ISRQ_Wp >= USB_HC_MAX_ISR_FIFO_DEPTH)
		{
			USBHC_ISRQ_Wp = 0;
		}
	}

	//------------------------------------------------------------------------
	// TD HANDLE 
	//------------------------------------------------------------------------
	//** Final Step, clear the flag
	HCIS = int_state;
}

/*
 * -------------------------------------------------------------------------------------------
 * U8_T USB_HAL_Alloc_Free_PDevice(U8_T *freeid, U8_T port)
 * Purpose : Allocate PDevice resource
 * Params  : 
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------
 */
U8_T USB_HAL_Alloc_Free_PDevice(U8_T *freeid, U8_T port)
{
	U8_T	index;
	U8_T    start = 0,end=USB_HC_MAX_DEVICE;

#if (SYSTEM_EXTENDER_TRANSMITTER)	
#ifdef DEVICE_KEEP
	if (port == 0xee) // this is the remote virtual device
	{
		start = USB_HC_MAX_DEVICE;
		end   = USB_HC_MAX_DEVICE+USB_HC_MAX_HUB;
	}	
#endif	/* #ifdef DEVICE_KEEP */
#endif	/* #if (SYSTEM_EXTENDER_TRANSMITTER) */
	for (index = start; index < end; index++)
	{
		if (USB_PDevice[index].Addr & USBHC_DEVICE_USED_MASK)
		{
			continue; // go next port
		}
		*freeid = index;
		memset((U8_T *)&USB_PDevice[index],0x00,sizeof(USB_PDevice_TypeDef));
		USB_PDevice[index].Addr |= USBHC_DEVICE_USED_MASK;
		USB_PDevice[index].RootHubPort = port;
		if (index >= USBDC_VIRTUAL_DEVINX)
			USB_PDevice[index].ControlEndpSize = 8;
		USBHC_Current_Total_Device_Count++;
#if (SYSTEM_EXTENDER_RECEIVER)			
#ifdef PWAYTEK_01		
		ExtenderR_Maintain_DeviceState_Led();
#endif		
#endif
		return TRUE;
	}
#if (KVM_BUZZER_SUPPORT)
	BUZZER_Script_Active(DeviceErrorSound);
#endif //#if (KVM_BUZZER_SUPPORT)
	return FALSE;
}

/*
 * -------------------------------------------------------------------------------------------
 * void USB_HAL_Check_PDevice_Status(U8_T port)
 * Purpose : Check PDevice status
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------
 */
U8_T USB_HAL_Check_PDevice_Status(U8_T port)
{
	U8_T	index;

	for (index = 0; index < (USB_HC_MAX_DEVICE + USB_HC_MAX_HUB); index++)
	{
		if (USB_PDevice[index].Addr & USBHC_DEVICE_USED_MASK)
		{
			if (USB_PDevice[index].RootHubPort == port)
			{
				if (USB_PDevice[index].Hc.gState >= HOST_ENUMERATION)
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

/*
 * -------------------------------------------------------------------------------------------
 * void USBHC_CORE_Alloc_IntTransfer_Table(void)
 * Purpose : Handle the Interrupt status
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------
 */
U8_T USBHC_CORE_Alloc_IntTransfer_Table(U8_T *freeid)
{
	U8_T	index;

	for (index = 0; index < USB_HC_INTL_MAX; index++)
	{
		if (HC_IntTransfer_Table[index].Used_DeviceID & USBHC_DEVICE_USED_MASK)
		{
			continue; // go next port
		}
		*freeid = index;
		memset((U8_T *)&HC_IntTransfer_Table[index],0x00,sizeof(USBHC_IntTransfer_TypeDef));
		HC_IntTransfer_Table[index].Used_DeviceID = USBHC_DEVICE_USED_MASK;
		return TRUE;
	}

	return FALSE;
}

/*
 * -------------------------------------------------------------------------------------------
 * void USBHC_CORE_Change_IntTransfer_Interval(void)
 * Purpose : Handle the Interrupt status
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------
 */
void USBHC_CORE_Change_IntTransfer_Interval(U8_T devinx, U8_T endpinx, U8_T interval)
{
	U8_T	index, source_endpinx;
	
#ifdef SYNC
	if (devinx)
		return;
#endif
	
#if (SYSTEM_EXTENDER_TRANSMITTER)
	if (USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK)//if remote device
	{
#if (HUART_BAUD <= HUART_921K)		
		//printf("R Devinx:%bu,endpinx=%bu,interval:%bu\n\r",devinx,endpinx,interval);
		ExtenderT_Remote_Interval_Req(devinx,endpinx,interval);
#endif
		return;
	}	
#endif
	for (index = 0; index < USB_HC_INTL_MAX; index++)
	{
		if (HC_IntTransfer_Table[index].Used_DeviceID & USBHC_DEVICE_USED_MASK)
		{
			if ((HC_IntTransfer_Table[index].Used_DeviceID & 0x7f) == devinx)
			{
				source_endpinx = (HC_IntTransfer_Table[index].EndpIdx & USBDC_DEV_ENDPINX_MASK ) >> 4;
				if (source_endpinx == endpinx)
				{
					//get right td id,change the interval					
					HCTD_Table.INTL[index].TD.Byte7 = interval;
				}
			}
		}
	}
}

/*
 * -------------------------------------------------------------------------------------------
 * U8_T USBHC_CORE_QueryAlloc_IsoTransfer_Table(U8_T allocateID)
 * Purpose : alloc the needed istl td id
 * Params  : U8_T allocateID
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------
 */
U8_T USBHC_CORE_QueryAlloc_IsoTransfer_Table(U8_T allocateID)
{
	if (allocateID < USB_HC_ISTL_MAX)
	{
		if ((HC_IsoTransfer_Table[allocateID].Used_DeviceID & USBHC_DEVICE_USED_MASK) == 0)
		{
			memset((U8_T *)&HC_IsoTransfer_Table[allocateID],0x00,sizeof(USBHC_IsoTransfer_TypeDef));
			HC_IsoTransfer_Table[allocateID].Used_DeviceID = USBHC_DEVICE_USED_MASK;
			return TRUE;
		}
	}

	return FALSE;
}
/*
 * -------------------------------------------------------------------------------------------
 * void USBHC_CORE_Set_TD_SkipMap(void)
 * Purpose : Handle the Skip Map Setting
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------
 */
void USBHC_CORE_Set_TD_SkipMap(U8_T tdtype,U8_T tdid,U8_T value)
{
	if (tdtype == USBHC_TD_INTL)
	{
		USBHC_CORE_SetBit(TD_INTL_Skip_Map,tdid,value);
		USBHC_Write_Regs(HC_INTL_TDSKIP_MAP_REG,TD_INTL_Skip_Map,4);
	}
	else if (tdtype == USBHC_TD_ATL)
	{
		USBHC_CORE_SetBit(TD_ATL_Skip_Map,tdid,value);
		USBHC_Write_Regs(HC_ATL_TDSKIP_MAP_REG,TD_ATL_Skip_Map,4);
	}
	else
	{
		USBHC_CORE_SetBit(&TD_ISTL_Skip_Map,tdid,value);
		USBHC_Write_Regs(HC_ISTL_TDSKIP_MAP_REG,&TD_ISTL_Skip_Map,1);
	}
}

/*
 * -------------------------------------------------------------------------------------------
 * U8_T USBHC_CORE_Get_TD_SkipMap(void)
 * Purpose : Handle the Skip Map Setting
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------
 */
//U8_T USBHC_CORE_Get_TD_SkipMap(U8_T tdtype,U8_T tdid)
//{
//	if (tdtype == USBHC_TD_INTL)
//	{
//		return USBHC_CORE_GetBit(TD_INTL_Skip_Map,tdid);
//	}
//	else if (tdtype == USBHC_TD_ATL)
//	{
//		return USBHC_CORE_GetBit(TD_ATL_Skip_Map,tdid);
//	}
//	else
//	{
//		return USBHC_CORE_GetBit(&TD_ISTL_Skip_Map,tdid);
//	}
//}

/*
 * -------------------------------------------------------------------------------------------
 * void USBHC_CORE_Clear_TD_DoneMap(void)
 * Purpose : Handle the Skip Map Setting
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------
 */
void USBHC_CORE_Clear_TD_DoneMap(U8_T tdtype,U8_T tdid) 
{
	U8_T done_map[4];

	done_map[0] = 0;
	done_map[1] = 0;
	done_map[2] = 0;
	done_map[3] = 0;

	USBHC_CORE_SetBit(done_map,tdid,0x01);

	if (tdtype == USBHC_TD_INTL)
	{
		USBHC_Write_Regs(HC_INTL_TDDONE_MAP_REG,done_map,4);
	}
	else if (tdtype == USBHC_TD_ATL)
	{
		USBHC_Write_Regs(HC_ATL_TDDONE_MAP_REG,done_map,4);
	}
	else
	{
		USBHC_Write_Regs(HC_ISTL_TDDONE_MAP_REG,done_map,1);
	}
}

/*
 * -------------------------------------------------------------------------------------------
 * void USBHC_CORE_Alloc_TD(void)
 * Purpose : Handle the Interrupt status
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------
 */
U8_T USBHC_CORE_Alloc_TD(void)
{
	U8_T	start_pos;

	start_pos = USB_HC_ATL_START;

	for (; start_pos < (USB_HC_ATL_START + USB_HC_ATL_MAX); start_pos++)
	{
		if ((HCTD_Channel_Table[start_pos].Used_DeviceID & USBHC_TD_USED_MASK) == 0)
		{
			return (start_pos + TRUE);
		}
	}

	return FALSE;
}

/*
 * -------------------------------------------------------------------------------------------
 * void USBHC_CORE_Free_TD(void)
 * Purpose : Handle the Interrupt status
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------
 */
void USBHC_CORE_Free_TD(U8_T td_id)
{
	U8_T devinx;

	if (HCTD_Channel_Table[td_id].Used_DeviceID | USBHC_TD_USED_MASK)
	{
		devinx = HCTD_Channel_Table[td_id].Used_DeviceID & USBHC_TD_DEVICE_ADDR_MASK;
		USB_PDevice[devinx].Hc.Control.TdNum = 0;
		HCTD_Channel_Table[td_id].Used_DeviceID = 0;
		HCTD_Channel_Table[td_id].USB_State = USBH_IDLE;
	}
}

/**
  * @brief  USBH_HandleControl
  *         Handles the USB control transfer state machine
  * @param  pdev: Selected device
  * @retval Status
  */
void TASK_USBHC_CORE_Control_Transfer_Handle(void)
{
	U8_T					devinx,td_id,td_offset;
	U8_T					direction;
	U16_T					index;
	USBH_Status				USB_Status = USBH_IDLE;
	bit						dc_passthrough=0;

    if (TASK_Type == TASK_TYPE_INTERVAL_MS)
    {
    	TASK_Destory_Current();
	}
	devinx = TASK_Register0;
	td_id = USB_PDevice[devinx].Hc.Control.TdNum;
	td_offset = USBHC_CORE_TD_Offset(td_id);

	//Skip task if the device is gone
	if (((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00) ||
		(USB_PDevice[devinx].Hc.Control.State == CTRL_IDLE))  // Device is gone
	{	
		return;	
	}
	
HC_ControlHandle_Restart:
	switch (USB_PDevice[devinx].Hc.Control.State)
	{
		case CTRL_SETUP:
			/* send a SETUP packet */
			USB_PDevice[devinx].Hc.Control.State = CTRL_SETUP_WAIT;
			USB_PDevice[devinx].Hc.HandleState   = HANDLE_CONTROL;
			USBHC_CtlSendSetup(devinx,td_id);
			USB_PDevice[devinx].timeOutRetryCnt = 0;
			break;

		case CTRL_SETUP_WAIT:
			/* SETUP Token send complete */
			/*Check the complete status */
			USB_Status = HCTD_Channel_Table[td_id].USB_State;
			if (USB_Status == USBH_DONE)
			{
				direction = (USB_PDevice[devinx].Hc.Control.Setup.b.bmRequestType & USB_REQ_DIR_MASK); /* data direction */
				if (USB_PDevice[devinx].Hc.Control.Setup.b.wLength.w != 0 ) /* with data stage */
				{
					if (direction == USB_D2H) /*Device to Host,Data Direction is IN*/
					{
						USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_IN;
						USB_PDevice[devinx].Hc.Control.NakeCnt = 0; // reset the nake counter
					}
					else 
					{	/*Host to Device,Data Direction is OUT*/
						USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_OUT;
						if (USB_PDevice[devinx].Hc.gState == HOST_PASSTHROUGH)
						{
							dc_passthrough = 1;
							USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_SETUP_START_DONE;
							break; // next is data out, need to wait for DC
						}
					}
					goto HC_ControlHandle_Restart;
				}
				else // No DATA stage  
				{
					/* Only Status IN */
					// Data Direction is OUT/
					USB_PDevice[devinx].Hc.Control.State = CTRL_STATUS_IN;
					USB_PDevice[devinx].Hc.Control.NakeCnt = 0;
					goto HC_ControlHandle_Restart;
				}
			}
			else if ((USB_Status== USBH_TIMEOUT) || (USB_Status== USBH_ERROR))
			{				
				USB_PDevice[devinx].Hc.Control.State = CTRL_ERROR;
				goto HC_ControlHandle_Restart; 
			}
			else if (USB_Status == USBH_STALL)
			{
				/* Control transfers completed, Exit the State Machine */
				USB_PDevice[devinx].Hc.Control.State = CTRL_STALLED; // Vinson ADD ... this is bug
				/*PASSTHROUGH*/
				if (USB_PDevice[devinx].Hc.gState == HOST_PASSTHROUGH)
				{
					USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_DC_SETUP_STALL;
					dc_passthrough = 1;
				}
			}
			break;
		case CTRL_DATA_IN:
			/* Issue an IN token */ 
			/*Determine the IN length */
#ifdef KVM_EXTENDER_RECEIVER
			//Check curent page remain length
            if ((USB_PDevice[devinx].Hc.gState == HOST_PASSTHROUGH) && (USB_PDevice[devinx].Hc.PControl.UPID == REMOTE_HOST_PORT))
			{				
				//Use Pdevice as the page buffer receiver
	   			index = USB_PDevice[devinx].Hc.PControl.Total_Length-USB_PDevice[devinx].Hc.PControl.Current_Length;
	   		}
	   		else
#endif /* #ifdef KVM_EXTENDER_RECEIVER */
			{
				index = HCTD_Channel_Table[td_id].Total_Length - HCTD_Channel_Table[td_id].Current_Length;
			}

			if (index > HCTD_Channel_Table[td_id].Endp_Size)
				index = HCTD_Channel_Table[td_id].Endp_Size;

			USBHC_CtlReceiveData (devinx, td_id, index, 0);
			USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_IN_WAIT;
			break;
		case CTRL_DATA_IN_WAIT:
			USB_Status = HCTD_Channel_Table[td_id].USB_State;
CTRL_DATA_IN_WAIT_RESTART:

			if (USB_Status == USBH_DONE)
			{
				/* Control data in package transfers completed, Exit the State Machine */
				// copy the buffer content from TD buffer
				td_offset = USBHC_CORE_TD_Offset(td_id);
#ifdef KVM_EXTENDER_RECEIVER
				//Check curent page remain length
	            if ((USB_PDevice[devinx].Hc.gState == HOST_PASSTHROUGH) && (USB_PDevice[devinx].Hc.PControl.UPID == REMOTE_HOST_PORT))
				{			
					//Use Pdevice as the page buffer receiver
		   			index = USB_PDevice[devinx].Hc.PControl.Total_Length-USB_PDevice[devinx].Hc.PControl.Current_Length;
		   		}
		   		else
#endif /* #ifdef KVM_EXTENDER_RECEIVER */
				{				
					index = HCTD_Channel_Table[td_id].Total_Length - HCTD_Channel_Table[td_id].Current_Length;
				}
				
				if (index > HCTD_Channel_Table[td_id].ActualByte)
					index = HCTD_Channel_Table[td_id].ActualByte;

				USB_PDevice[devinx].timeOutRetryCnt = 0;
				/*PASSTHROUGH*/
				if (USB_PDevice[devinx].Hc.gState == HOST_PASSTHROUGH)
				{
					//copy data into DC buffer
					USB_PDevice[devinx].Hc.PControl.Data_Length = index;
					for (direction=0; direction < index; direction++)
					{
#ifdef KVM_EXTENDER_RECEIVER
						if (USB_PDevice[devinx].Hc.PControl.UPID == REMOTE_HOST_PORT)
						{
							HCTD_Channel_Table[td_id].Buf[direction+(USB_PDevice[devinx].Hc.PControl.Current_Length)] = HCTD_Table.ATL[td_offset].Buf[direction];
						}
						else
#endif  /* #ifdef KVM_EXTENDER_RECEIVER */
						{
							HCTD_Channel_Table[td_id].Buf[direction] = HCTD_Table.ATL[td_offset].Buf[direction];
						}
					}
#if (USB_PASSTHROUGH_DEBUG_MODE)
					printf("Passtrhough Len=%d,",(U16_T)USB_PDevice[devinx].Hc.PControl.Data_Length);
#endif
					USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_DC_SETUP_DATA_IN; // inform the DC Buffer is ready to send
					dc_passthrough = 1;
				}
				else
				{
					for (direction=0; direction < index ; direction++)
					{
						HCTD_Channel_Table[td_id].Buf[HCTD_Channel_Table[td_id].Current_Length+direction] = HCTD_Table.ATL[td_offset].Buf[direction];
					}
				}

				// update the length
				HCTD_Channel_Table[td_id].Current_Length += index;
				
				if (HCTD_Channel_Table[td_id].Current_Length < HCTD_Channel_Table[td_id].Total_Length)
				{
					//check the last package is size is small than control endp package size
					if (HCTD_Channel_Table[td_id].ActualByte < USB_PDevice[devinx].ControlEndpSize)
					{
						USB_PDevice[devinx].Hc.Control.State = CTRL_STATUS_OUT;
					}
					else
					{
						USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_IN;
					}
				}
				else
				{
#if 0
					if ((dc_passthrough == 0) &&
						((HCTD_Channel_Table[td_id].Total_Length % USB_PDevice[devinx].ControlEndpSize) == 0))
					{
						USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_LAST_IN;
					}
					else
#endif
					{
						USB_PDevice[devinx].Hc.Control.State = CTRL_STATUS_OUT;
					}
				}

				if (dc_passthrough == 0)
				{
					goto HC_ControlHandle_Restart;
				}
			}
			else if (USB_Status == USBH_TIMEOUT)
			{
				if (++USB_PDevice[devinx].timeOutRetryCnt > 3)
				{
					USB_PDevice[devinx].Hc.Control.State = CTRL_ERROR;
					goto HC_ControlHandle_Restart;
				}

				USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_IN;
				goto HC_ControlHandle_Restart;
			}
			else if (USB_Status == USBH_ERROR)
			{
				USB_PDevice[devinx].Hc.Control.State = CTRL_ERROR;
				goto HC_ControlHandle_Restart;
			}
			else if (USB_Status == USBH_STALL)
			{
USBHC_CTRL_DATAIN_STALL1:
				/* Control transfers completed, Exit the State Machine */
				USB_PDevice[devinx].Hc.Control.State = CTRL_STALLED; // Vinson ADD ... this is bug 
				/*PASSTHROUGH*/
				if (USB_PDevice[devinx].Hc.gState == HOST_PASSTHROUGH)
				{
#if (USB_PASSTHROUGH_DEBUG_MODE)
					printf("@P@ CTRL_DATA_IN_WAIT->STALL\n\r");
#endif 
					USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_DC_SETUP_STALL;
					dc_passthrough = 1; 
				}
			}
			else if (USB_Status == USBH_NAKE) // resend the data In again
			{
				if (HCTD_Channel_Table[td_id].ActualByte) //since has data in, so just reset the USB_Status
				{
					USB_Status = USBH_DONE;
					goto CTRL_DATA_IN_WAIT_RESTART;
				}

				USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_IN;
				if (++USB_PDevice[devinx].Hc.Control.NakeCnt > USBHC_DATAIN_NAKE_MAX) 
				{
					USB_PDevice[devinx].Hc.Control.NakeCnt = 0; // reset the nake counter
				    goto USBHC_CTRL_DATAIN_STALL1;
				}   
				goto HC_ControlHandle_Restart;
			}
			else if (USB_Status == USBH_DATA_TOGGLE_ERR)// resend the data In again
			{
				USB_Status = USBH_DONE;
				goto CTRL_DATA_IN_WAIT_RESTART;
			}
			break;
#if 0
		case CTRL_DATA_LAST_IN:
			USBHC_CtlReceiveData (devinx, td_id, 0, 0);
			USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_LAST_IN_WAIT;
			break;
		case CTRL_DATA_LAST_IN_WAIT:
			USB_Status = HCTD_Channel_Table[td_id].USB_State;
			if (USB_Status == USBH_DONE)
			{
				USB_PDevice[devinx].timeOutRetryCnt = 0;
				USB_PDevice[devinx].Hc.Control.State = CTRL_STATUS_OUT;
				goto HC_ControlHandle_Restart;
			}
			else if (USB_Status == USBH_TIMEOUT ||
					 USB_Status == USBH_NAKE ||
					 USB_Status == USBH_DATA_TOGGLE_ERR)
			{								
				if (++USB_PDevice[devinx].timeOutRetryCnt > 1)
				{
					USB_PDevice[devinx].timeOutRetryCnt = 0;
					USB_PDevice[devinx].Hc.Control.State = CTRL_STATUS_OUT;
					goto HC_ControlHandle_Restart;
				}
				USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_LAST_IN;
				goto HC_ControlHandle_Restart;
			}
			else if (USB_Status == USBH_STALL)
			{				
				USB_PDevice[devinx].Hc.Control.State = CTRL_STALLED;
			}
			else
			{			
				USB_PDevice[devinx].timeOutRetryCnt = 0;
				USB_PDevice[devinx].Hc.Control.State = CTRL_STATUS_OUT;
				goto HC_ControlHandle_Restart;
			}
			break;
#endif
		case CTRL_DATA_OUT:
			/* Start DATA out transfer (only one DATA packet)*/
			if (USB_PDevice[devinx].Hc.gState == HOST_PASSTHROUGH)
			{
				index = USB_PDevice[devinx].Hc.PControl.Data_Length; // only do this package
#if (USB_PASSTHROUGH_DEBUG_MODE)
				/*PASSTHROUGH DEBUG*/
				printf(" @P_Out@Len=%d,",(U16_T)index);
#endif
			}
			else
			{
				index = HCTD_Channel_Table[td_id].Total_Length-HCTD_Channel_Table[td_id].Current_Length;
			}
			USBHC_CtlSendData(devinx,td_id,index);
			USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_OUT_WAIT;
			break;

		case CTRL_DATA_OUT_WAIT:
			USB_Status = HCTD_Channel_Table[td_id].USB_State;
			if (USB_Status == USBH_DONE)
			{
				/* If the Setup Pkt is sent successful, then change the state */
				HCTD_Channel_Table[td_id].Current_Length += HCTD_Channel_Table[td_id].ActualByte;
				if (HCTD_Channel_Table[td_id].Current_Length < HCTD_Channel_Table[td_id].Total_Length)
				{
					USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_OUT;
				}
				else
				{
					USB_PDevice[devinx].Hc.Control.State = CTRL_STATUS_IN;
				}

				if (USB_PDevice[devinx].Hc.gState == HOST_PASSTHROUGH)
				{
#if (USB_PASSTHROUGH_DEBUG_MODE)
	 				printf(" @P@ Data Out Done\n\r");
#endif
					if (USB_PDevice[devinx].Hc.Control.State == CTRL_DATA_OUT)
					{
						USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_HC_SETUP_DATA_OUT_DONE;
						dc_passthrough = 1;
#if (USB_PASSTHROUGH_DEBUG_MODE)
						printf(" @P@ HC Data OUT DONE\n\r");
#endif
					}
					else
					{
						USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_DC_SETUP_STATUS_IN;
#if (USB_PASSTHROUGH_DEBUG_MODE)
						printf(" @P@ HC Status IN\n\r");
#endif
						goto HC_ControlHandle_Restart;
					}
				}
				else
				{
					goto HC_ControlHandle_Restart;
				}
			}
			/* handle error cases */
			else if  (USB_Status == USBH_STALL) 
			{
				/* In stall case, return to previous machine state*/ 
				USB_PDevice[devinx].Hc.Control.State = CTRL_STALLED;
			}
			else if (USB_Status == USBH_NAKE)
			{
				/* Nack received from device */
				USB_PDevice[devinx].Hc.Control.State = CTRL_DATA_OUT;
				goto HC_ControlHandle_Restart;
			}
			else if (USB_Status == USBH_ERROR)
			{
				/* device error */
				USB_PDevice[devinx].Hc.Control.State = CTRL_ERROR;
				goto HC_ControlHandle_Restart;
			}
			break;

		case CTRL_STATUS_IN:
			/* Send 0 bytes out packet */
			USBHC_CtlReceiveData (devinx, td_id, 0, 1);
			USB_PDevice[devinx].Hc.Control.State = CTRL_STATUS_IN_WAIT;
			break;
		case CTRL_STATUS_IN_WAIT:
			USB_Status = HCTD_Channel_Table[td_id].USB_State;
			if (USB_Status == USBH_DONE)
			{
				/* Control transfers completed, Exit the State Machine */
				if (USB_PDevice[devinx].Hc.gState == HOST_PASSTHROUGH)
				{
					USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_DC_SETUP_STATUS_IN;//Inform DC status IN, then clear
				}
				USB_PDevice[devinx].Hc.Control.State = CTRL_COMPLETE;
			}
			else if ((USB_Status == USBH_ERROR) || (USB_Status == USBH_TIMEOUT))
			{
				USB_PDevice[devinx].Hc.Control.State = CTRL_ERROR;
				goto HC_ControlHandle_Restart;
			}
			else if (USB_Status == USBH_STALL)
			{
				/* Control transfers completed, Exit the State Machine */
				if (USB_PDevice[devinx].Hc.gState == HOST_PASSTHROUGH)
				{
					USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_DC_SETUP_STALL;
				}
				USB_PDevice[devinx].Hc.Control.State = CTRL_STALLED;
			}
			else if (USB_Status == USBH_NAKE) // resend the data In again
			{
				USB_PDevice[devinx].Hc.Control.State = CTRL_STATUS_IN;
				if (++USB_PDevice[devinx].Hc.Control.NakeCnt > USBHC_DATAIN_NAKE_MAX)
				{
					USB_PDevice[devinx].Hc.Control.NakeCnt = 0; // reset the nake counter
				    goto USBHC_CTRL_DATAIN_STALL1;
				}
				goto HC_ControlHandle_Restart;
			}
			else if (USB_Status == USBH_DATA_TOGGLE_ERR)// resend the data In again
			{
				/* Control transfers completed, Exit the State Machine */
				if (USB_PDevice[devinx].Hc.gState == HOST_PASSTHROUGH)
				{
					USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_DC_SETUP_STATUS_IN;//Inform DC status IN, then clear
				}
				USB_PDevice[devinx].Hc.Control.State = CTRL_COMPLETE;
			}
			// Perform Next Operation
			break;
		case CTRL_STATUS_OUT:
			USBHC_CtlSendData (devinx, td_id, 0);
			USB_PDevice[devinx].Hc.Control.State = CTRL_STATUS_OUT_WAIT;
			break;
		case CTRL_STATUS_OUT_WAIT:
			USB_Status = HCTD_Channel_Table[td_id].USB_State;
			if (USB_Status == USBH_DONE)
			{
				USB_PDevice[devinx].Hc.Control.State = CTRL_COMPLETE;
				/*PASSTHROUGH*/
				if (USB_PDevice[devinx].Hc.gState == HOST_PASSTHROUGH)
					USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_COMPLETE; //passthrough complete condition
			}
			else if (USB_Status == USBH_NAKE)
			{
				USB_PDevice[devinx].Hc.Control.State = CTRL_STATUS_OUT;
				goto HC_ControlHandle_Restart;
			}
			else if ((USB_Status == USBH_ERROR)  || (USB_Status == USBH_TIMEOUT))
			{
				if (++USB_PDevice[devinx].timeOutRetryCnt > 3)
				{
					USB_PDevice[devinx].Hc.Control.State = CTRL_ERROR;
					goto HC_ControlHandle_Restart;
				}
				USB_PDevice[devinx].Hc.Control.State = CTRL_STATUS_OUT;
				goto HC_ControlHandle_Restart;
			}
			break;
		case CTRL_ERROR:
			/* 
				After a halt condition is encountered or an error is detected by the 
				host, a control endpoint is allowed to recover by accepting the next Setup 
				PID; i.e., recovery actions via some other pipe are not required for control
				endpoints. For the Default Control Pipe, a device reset will ultimately be 
				required to clear the halt or error condition if the next Setup PID is not 
				accepted.
			*/
			
			//Skip task if the device is gone
			if ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00) 				
			{	
				USB_PDevice[devinx].Hc.Control.ErrorCnt = 0; /* Reset error count */
				USB_PDevice[devinx].Hc.Control.State = CTRL_FAIL;				
				break;
			}
			
			if ((++USB_PDevice[devinx].Hc.Control.ErrorCnt) <= USBH_MAX_ERROR_COUNT)
			{
				/* Do the transmission again, starting from SETUP Packet */
				USB_PDevice[devinx].Hc.Control.State = CTRL_SETUP;								
				TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_USBHC_Handle_Control_ID,td_id,devinx,500,500);  // perform HC IN processing
				//goto HC_ControlHandle_Restart;	
				return;
			}
			else
			{
				USB_PDevice[devinx].Hc.Control.ErrorCnt = 0; /* Reset error count */
				USB_PDevice[devinx].Hc.Control.State = CTRL_FAIL;				
			}
			break;
		default:
			break;
	}

	if ((USB_PDevice[devinx].Hc.Control.State == CTRL_COMPLETE) || \
		(USB_PDevice[devinx].Hc.Control.State == CTRL_FAIL) || \
		(USB_PDevice[devinx].Hc.Control.State == CTRL_STALLED)) 
	{
		USB_PDevice[devinx].Hc.Control.ErrorCnt = 0; /* Reset error count */
		USB_PDevice[devinx].Hc.HandleState = HANDLE_IDLE; // escape from control mode
		switch(USB_PDevice[devinx].Hc.gState)
		{
			case HOST_ENUMERATION:
				TASK_Active(TASK_TYPE_USB,TASK_USBHC_Enumerate_Handle_ID,0x00,devinx,0,0); // Generate the task
				break;
			case HOST_HID_CLASS_ENUMERATION:
				TASK_Active(TASK_TYPE_USB,TASK_USBHC_HID_Class_Enumerate_ID,USB_PDevice[devinx].Hc.Control.InfAddr,devinx,0,0);
				break;
			case HOST_CLASS_ENUMERATION:
				if (USB_PDevice[devinx].Hc.HUB_EnumState)
				{
					USB_Hub_Class_Enumerate_Fork(devinx);
				}
				break;
			case HOST_SCM:
				TASK_Active(TASK_TYPE_USB,TASK_USBHC_Scm_Handle_ID,0x00,devinx,0,0); 
				break;
			case HOST_PASSTHROUGH:
				if (USB_PDevice[devinx].Hc.Control.State != CTRL_COMPLETE)
				{
					USB_PDevice[devinx].Hc.PassThrough_State = PASSTHROUGH_DC_SETUP_STALL;
				}
#ifdef KVM_EXTENDER_RECEIVER
				if (USB_PDevice[devinx].Hc.PControl.UPID == REMOTE_HOST_PORT)
				{
					TASK_Active(TASK_TYPE_EVENT,TASK_USBHC_ExtenderR_PassThrough_ID,0x00,devinx,0,0); //Generate the task, for next Event check
				}
				else
				{
					TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_USBHC_PassThrough_Handle_ID,0x00,devinx,2,2); //Generate the task, for next Event check
				}
#else
				TASK_Active(TASK_TYPE_USB,TASK_USBHC_PassThrough_Handle_ID,0x00,devinx,0,0); //Generate the task, for next Event check
#endif
				return;
			case HOST_CLASS_REQUEST:
				if (USB_PDevice[devinx].Hc.HUB_EnumState)
				{
					USB_Hub_Class_Enumerate_Fork(devinx);
				}
				break;
		}
	}

	if (dc_passthrough)
	{
#ifdef KVM_EXTENDER_RECEIVER
		if (USB_PDevice[devinx].Hc.PControl.UPID == REMOTE_HOST_PORT)
		{	
	   		TASK_Active(TASK_TYPE_USB,TASK_USBHC_ExtenderR_PassThrough_ID,0x00,devinx,0,0); //Generate the task, for next Event check	   		
	   	}
	   	else
	   	{
	   	    TASK_Active(TASK_TYPE_USB,TASK_USBHC_PassThrough_Handle_ID,0x00,devinx,0,0); //Generate the task, for next Event check
	   	}		
#else	    
		TASK_Active(TASK_TYPE_USB,TASK_USBHC_PassThrough_Handle_ID,0x00,devinx,0,0); //Generate the task, for next Event check
#endif /* KVM_EXTENDER_RECEIVER */
	}
}

void USBHC_CORE_SetBit(U8_T *tab,U8_T index,U8_T value)
{
	U8_T byte,targetbit;
	
	byte = index >> 3; // every byte 8 bits
	targetbit = 0x01 << (index & 0x07);
	if (value)	 // bet bit
	{
		tab[byte] |= targetbit;
	}
	else
	{
		tab[byte] &= ~targetbit;
	}
}

/**
  * @brief  USBHC_CORE_GetBit
  *         Handles the USB control transfer state machine
  * @param  pdev: Selected device
  * @retval Status
  */
//U8_T USBHC_CORE_GetBit(U8_T *tab,U8_T index)
//{
//	U8_T byte,targetbit;
	
//	byte = index >> 3; // every byte 8 bits
//	targetbit = 0x01 << (index & 0x07);
//	if (tab[byte] & targetbit)
//		return TRUE;
//	else
//		return FALSE;
//}

/** ----------------------------------------------------------------------------
 * void USBHC_ISR_Handle(void)
 * Purpose: Handle USB HC isr event
 * Params:  NONE
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------*/
void USBHC_ISR_Handle(void)
{
	U8_T	int_state;
	U8_T	td_id,devinx,mapbyte,mapbit,target;
	U8_T	bitmask;
	U8_T	donemap;
	U8_T    endaddr = 0;
	bit		exit_flag;
	
	//Get interrupt status from sw fifo
	int_state = USBHC_ISR_FIFO[USBHC_ISRQ_Rp].ISR_State;

	//Check Operational Interrupt
	if (int_state & HC_INT_STATE_OPR_SET)
	{
		// Check Operation Interrupt Status
		if (USBHC_ISR_FIFO[USBHC_ISRQ_Rp].OP_ISR_State & HC_OPINT_STATE_RHSC_MAP_MASK) // port status change interrupt
		{
			// Check Portx status changed condition
			USBHC_RootHub_StatusChanged_Handle();
		}
	}

	//Increased the RP pointer
	if (++USBHC_ISRQ_Rp >= USB_HC_MAX_ISR_FIFO_DEPTH)
	{
		USBHC_ISRQ_Rp = 0;
	}
	//------------------------------------------------------------------------
	// TD HANDLE 
	//------------------------------------------------------------------------
	//Check ATL Done Interrupt
	if (int_state & HC_INT_STATE_ATL_SET)
	{
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
		EXTINT3_DISABLE;
#endif
		EXTINT2_DISABLE;
		USBHC_AtlDoneFlag = 0;
		_USBHC_HCCIR_SFR(HC_ATL_TDDONE_MAP_REG);
		_USBHC_HCDR_READ_SFR(TD_ATL_Done_Map[0]);
		_USBHC_HCDR_READ_SFR(TD_ATL_Done_Map[1]);
		_USBHC_HCDR_READ_SFR(TD_ATL_Done_Map[2]);
		_USBHC_HCDR_READ_SFR(TD_ATL_Done_Map[3]);
		EXTINT2_ENABLE;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
		EXTINT3_ENABLE;
#endif

		//Set Skip MAP First
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
		EXTINT3_DISABLE;
#endif
		USBHC_INT_DISABLE;
		_USBHC_HCDR_SFR(TD_ATL_Skip_Map[0] | TD_ATL_Done_Map[0]);
		_USBHC_HCDR_SFR(TD_ATL_Skip_Map[1] | TD_ATL_Done_Map[1]);
		_USBHC_HCDR_SFR(TD_ATL_Skip_Map[2] | TD_ATL_Done_Map[2]);
		_USBHC_HCDR_SFR(TD_ATL_Skip_Map[3] | TD_ATL_Done_Map[3]);
		_USBHC_HCCIR_SFR(HC_ATL_TDSKIP_MAP_REG);
		USBHC_INT_ENABLE;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
		EXTINT3_ENABLE;
#endif

		//Set Do Map
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
		EXTINT3_DISABLE;
#endif
		USBHC_INT_DISABLE;
		_USBHC_HCDR_SFR(TD_ATL_Done_Map[0]);
		_USBHC_HCDR_SFR(TD_ATL_Done_Map[1]);
		_USBHC_HCDR_SFR(TD_ATL_Done_Map[2]);
		_USBHC_HCDR_SFR(TD_ATL_Done_Map[3]);
		_USBHC_HCCIR_SFR(HC_ATL_TDDONE_MAP_REG);
		USBHC_INT_ENABLE;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
		EXTINT3_ENABLE;
#endif

		exit_flag = 0;
		for (mapbyte=0; mapbyte < 4 ; mapbyte++)
		{
			target = (mapbyte << 3);
			if (TD_ATL_Done_Map[mapbyte])
			{
				bitmask = 0x01;
				for (mapbit = 0; mapbit < 8; mapbit++)
				{
					if (TD_ATL_Done_Map[mapbyte] & bitmask) //if the TD is done
					{
						target = (mapbyte << 3) + mapbit;
						if ( target < USB_HC_ATL_MAX)
						{							
							if ((TD_ATL_Skip_Map[mapbyte] & bitmask) == 0)  //Get skip map
							{
								TD_ATL_Skip_Map[mapbyte] |= bitmask;
								//Check the Bulk Condition
								td_id  = target + USB_HC_ATL_START;
								devinx = HCTD_Channel_Table[td_id].Used_DeviceID & TD_CHANNEL_DEVICE_ADDRESS_MASK;
#if ((SYSTEM_MSC_DEVICE_SUPPORT) && (SYSTEM_USB_HC_BURST))
								if (target >= USB_HC_MSC_START) 
								{
									endaddr = (HCTD_Table.BULK[target-USB_HC_MSC_START].TD.Byte3 & 0xf0) >> 4;
								}									
								else
#endif
								{
									endaddr = (HCTD_Table.ATL[target].TD.Byte3 & 0xf0) >> 4;
								}
								
								if (endaddr == 0x00) // Control Endp
								{
									usbhc_core_ATL_Done_Handle(devinx, target);
								}
#if (SYSTEM_MSC_DEVICE_SUPPORT)
								else
								{
									if (devinx == USBHC_MSC_Devinx)
									{
										USBHC_BULK_Done_Q[USBHC_ATL_Done_Q_Wp] = (devinx << 5) | target;
										if (++USBHC_ATL_Done_Q_Wp >= BULK_DONE_QUEUE_MAX)
										{
											USBHC_ATL_Done_Q_Wp = 0;
										}
										if (USBHC_ATL_Done_Q_Wp == USBHC_ATL_Done_Q_Rp)
										{
											printf("ATL_Done_Queue is FULL!!!\r\n");
										}
									}
								}
#endif
							}
	
							if (exit_flag == 0)
							{
								TD_ATL_Done_Map[mapbyte] &= ~bitmask;
							}
							else 
							{
								break;
							}
						}
						
						if (TD_ATL_Done_Map[mapbyte] == 0)
						{
							break;
						}
					}
					//Check exit condition
					if ((target+1) >= USB_HC_ATL_MAX)
					{
						exit_flag = 1;
						break;
					}
					bitmask = bitmask << 1;
				}
			}
			
			if (exit_flag && ((target+1) >= USB_HC_ATL_MAX))
			{
				break;
			}
		}
	}

	//Check ISTL0 Done Interrupt
	//Check Suspend Interrupt
	if (int_state & HC_INT_STATE_SUS_SET)
	{
	}

	//Check INTL Done Interrupt
	if (int_state & HC_INT_STATE_INTL_SET)
	{
		EXTINT2_DISABLE;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
		EXTINT3_DISABLE;
#endif
		USBHC_IntlDoneFlag = 0;
		_USBHC_HCCIR_SFR(HC_INTL_TDDONE_MAP_REG);
		_USBHC_HCDR_READ_SFR(TD_INTL_Done_Map[0]);
		_USBHC_HCDR_READ_SFR(TD_INTL_Done_Map[1]);
		_USBHC_HCDR_READ_SFR(TD_INTL_Done_Map[2]);
		_USBHC_HCDR_READ_SFR(TD_INTL_Done_Map[3]);
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
		EXTINT3_ENABLE;
#endif
		EXTINT2_ENABLE;

		for (mapbyte = 0; mapbyte < 4; mapbyte++)
		{
			target = mapbyte << 3;
			donemap =TD_INTL_Done_Map[mapbyte];
			if (donemap)
			{
				for (mapbit = 0; mapbit < 8; mapbit++)
				{
					if (donemap & BIT_MASK[mapbit]) //if the TD is done
					{
						target = (mapbyte << 3) + mapbit;
						donemap &= ~BIT_MASK[mapbit];
						devinx = HC_IntTransfer_Table[target].Used_DeviceID & USBHC_TD_DEVICE_ADDR_MASK;

						//Process the INTL Input Data 
						if (((HCTD_Table.INTL[target].TD.Byte1 & COMPLETEION_CODE_MASK)==COMPLETE_NO_ERROR) || //receive success
							((HCTD_Table.INTL[target].TD.Byte1 & COMPLETEION_CODE_MASK)==COMPLETE_DATA_UNDERRUN)) //receive data under-size
						{
							//1-Take care the interrupt transfer IN direction 
							if ((HCTD_Table.INTL[target].TD.Byte5 & TD_DIR_TOKEN_MASK) == TD_DIR_TOKEN_IN)
							{ //copy data into buffer
								USBHC_InterruptTransferIN_Buf(target,devinx,HCTD_Table.INTL[target].TD.Byte0_Actual_Byte);
							}
						}

						//Must Clear the TD Actual Byte
						HCTD_Table.INTL[target].TD.Byte0_Actual_Byte = 0;
						//Active the TD again//
						if ((HCTD_Table.INTL[target].TD.Byte5 & TD_DIR_TOKEN_MASK) == TD_DIR_TOKEN_IN)
						{
							HCTD_Table.INTL[target].TD.Byte1 |= TD_ACTIVED_MASK;
						}
						else
						{
							USBHC_CORE_Set_TD_SkipMap(USBHC_TD_INTL, target, 0x01);
						}
					}

					if (donemap == 0x00)
					{
						break;
					}
				}
			}
			
			if ((target+1) >= USB_HC_INTL_MAX)
			{
				break;
			}
		}

		USBHC_Write_Regs(HC_INTL_TDDONE_MAP_REG,TD_INTL_Done_Map,4);  
	}
}
 

/** ----------------------------------------------------------------------------
 * void USBHC_CORE_Retrive_USB_State(U8_T td_type,U8_T atl_id,U8_T td_id)
 * Purpose: Retrive back the complete code
 * Params:  NONE
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------*/
void USBHC_CORE_Retrive_USB_State(U8_T td_type,U8_T atl_id,U8_T td_id)
{
	USBHC_TD_Header_Typedef *td_header;

	if (td_type == USBHC_TD_ATL)
	{
		td_header = &HCTD_Table.ATL[atl_id].TD;
	}
#if ((SYSTEM_MSC_DEVICE_SUPPORT) && (SYSTEM_USB_HC_BURST))
	else if (td_type == USBHC_TD_BULK)
	{
		atl_id = atl_id - USB_HC_MSC_START; //Get the Bulk Header
		td_header = &HCTD_Table.BULK[atl_id].TD;
	}	
#endif
	else
	{
		td_header = &HCTD_Table.INTL[atl_id].TD;
	}

	//1.Retrive the Complete code
	HCTD_Channel_Table[td_id].CompleteCode = td_header->Byte1 & COMPLETEION_CODE_MASK;
	switch (HCTD_Channel_Table[td_id].CompleteCode)	
	{
		case COMPLETE_NO_ERROR: //(0x00)
			HCTD_Channel_Table[td_id].USB_State = USBH_DONE;
			break;
		case COMPLETE_CRC: //(0x10)
		case COMPLETE_BIT_STUFFING://(0x20)
		case COMPLETE_PID_CHECK_FAILURE://(0x60)
		case COMPLETE_UNEXPECTED_PID://(0x70)
			//printf("HCERROR0=%02bx\n\r",HCTD_Channel_Table[td_id].CompleteCode);
			USBHC_ErrorState = HCTD_Channel_Table[td_id].CompleteCode;
			HCTD_Channel_Table[td_id].USB_State = USBH_ERROR;
			break;
		case COMPLETE_DATA_TOGGLE_MISMATCH://(0x30)
			HCTD_Channel_Table[td_id].USB_State = USBH_DATA_TOGGLE_ERR;
			break;
		case COMPLETE_STALL://(0x40)
			HCTD_Channel_Table[td_id].USB_State = USBH_STALL;
			break;
		case COMPLETE_DEVICE_NOT_RESPONDING://(0x50)
			HCTD_Channel_Table[td_id].USB_State = USBH_TIMEOUT;
			break;
		case COMPLETE_DATA_NAKE://(0xa0)
			HCTD_Channel_Table[td_id].USB_State = USBH_NAKE;
			break;
		case COMPLETE_DATA_UNDERRUN://(0x90)
			HCTD_Channel_Table[td_id].USB_State = USBH_DONE;
			break;
		case COMPLETE_DATA_OVERRUN://(0x80)
		case COMPLETE_BUFFER_OVERRUN://(0xc0)
		case COMPLETE_BUFFER_UNDERRUN://(0xd0)			
			//printf("HCERROR1=%02bx\n\r",HCTD_Channel_Table[td_id].CompleteCode);
			USBHC_ErrorState = HCTD_Channel_Table[td_id].CompleteCode;
			HCTD_Channel_Table[td_id].USB_State = USBH_ERROR;
			break;
	}

	//2.Retrive the actual byte
	HCTD_Channel_Table[td_id].ActualByte = td_header->Byte0_Actual_Byte;
	HCTD_Channel_Table[td_id].ActualByte += ((td_header->Byte1 & ACTUAL_BYTE_MSB_MASK) << 8);
}

/** ----------------------------------------------------------------------------
 * void usbhc_core_ATL_Done_Handle(U8_T devinx, U8_T atl_id)
 * Purpose: Process ATL TD done event
 * Params:  NONE
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------*/
static void usbhc_core_ATL_Done_Handle(U8_T devinx, U8_T atl_id)
{
	U8_T	td_id;

	//Skip task if the device is gone
	if ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00)  // Device is gone  
	{	
		return;	
	}
	
	td_id = atl_id + USB_HC_ATL_START;
	USBHC_CORE_Retrive_USB_State(USBHC_TD_ATL,atl_id,td_id);

	if (USB_PDevice[devinx].Hc.HandleState == HANDLE_CONTROL)
	{
		TASK_Active(TASK_TYPE_USB,TASK_USBHC_Handle_Control_ID,td_id,devinx,0,0); // Generate the task to handle the control 
	}
}

/** ----------------------------------------------------------------------------
 * void USBHC_CORE_Clear_TD_Register(U8_T td_id)
 * Purpose: After plug off form hc, clear the relative register in TD
 * Params:  NONE
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------*/
void USBHC_CORE_Clear_TD_Register(U8_T td_id)
{
	U8_T td;
	
	// Calculate the position in the td buffer table
	if (td_id < USB_HC_ISTL_MAX)
	{
		// clear ISTL TD
		USBHC_CORE_Set_TD_SkipMap(USBHC_TD_ISTL, td_id, 0x01);
	}
	else if (td_id < (USB_HC_ISTL_MAX + USB_HC_INTL_MAX))
	{
		// clear INTL TD
		td = td_id - USB_HC_ISTL_MAX; // this is the real td id

		//1.Set ATL skip table on
		USBHC_CORE_Set_TD_SkipMap(USBHC_TD_INTL, td, 0x01);
	}
	else
	{
		// clear ATL TD
		td = td_id - (USB_HC_ISTL_MAX + USB_HC_INTL_MAX); // this is the real td id

		//1.Set ATL skip table on
		USBHC_CORE_Set_TD_SkipMap(USBHC_TD_ATL, td, 0x01);
	}
}

/*
 * -------------------------------------------------------------------------------------------
 * void USBHC_CORE_Clear_Device(void)
 * Purpose : Clear device relative table & infromation
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------
 */
void USBHC_CORE_Clear_Device(U8_T devinx)
{
	U8_T index;
	U8_T prefetch = 0;
	bit  intl_chagned_flag = 0, vHubClear = 0;

	printf("Clear Start\b\r");
	//1.Clear device table
#if (SYSTEM_EXTENDER_TRANSMITTER)
	if ((USB_PDevice[devinx].DevAttr & DEVATTR_REMOTE_DEV_MASK) == 0x00) //if not extender remote device
#endif /*  (SYSTEM_EXTENDER_TRANSMITTER) */
	{
		if (USBHC_HUB_Enumerate_Occupy_Flag)
		{
			if (USBHC_HUB_Enumerate_Occupy_Devinx == devinx)
			{
				USBHC_HUB_Enumerate_Occupy_Flag = 0;
			}
		}
	}

#ifdef SYNC	
	KM_SYNC_Repeat_Active_Force_Stop(devinx);	
#endif	
	
    //1-1.Clear the DC State
	if ((devinx < USB_HC_MAX_DEVICE) &&
		(USBDC_Device[devinx].DevIdx & DC_USED_MASK))
	{			
		memset(USBDC_Device[devinx].PassThroughState,PASSTHROUGH_IDLE, USBDC_PORT_MAX);
		vHubClear = 1;
	}

	printf("clar_a ");
	//1-2.Clear the HC Control Pipe Buffer
	if (USB_PDevice[devinx].Hc.Control.Buf != 0)
	{
        printf("USBHC_CORE_Clear_Device():Dx=%bd,Cb=0x%lx Not NULL!!\n\r",devinx,(U32_T)USB_PDevice[devinx].Hc.Control.Buf);	
	}

#if (SYSTEM_EXTENDER_RECEIVER)
	if (USB_PDevice[devinx].DevClass != USB_HUB_CLASS)
	{
		if (USB_PDevice[devinx].Hc.Etdr_RejectMountFlag)
		{
			USB_PDevice[devinx].Hc.Etdr_RejectMountFlag = 0;
		}
		else
		{
			if ((USB_PDevice[devinx].DevClass == USB_MSC_CLASS) && USBHC_MSC_DismountFlag)
			{
				USBHC_MSC_DismountFlag = 0; //reset dismount flag
				//printf("DEBUG:Reset MSC DismountFlag\n\r");
			}
			else
			{			
				//if msc device is under remote reset condition
#if (SYSTEM_MSC_DEVICE_SUPPORT)			
				if ((USB_PDevice[devinx].DevClass == USB_MSC_CLASS) && USBHC_MSC_Reset_Flag)
				{
					//ExtenderR_Remote_MscResetCheck(devinx);
				}
				else
#endif					
				{		
#ifndef ONEHID					
					if (devinx < 8)
					{	
						Externder_Receiver_DeviceMount_Control(devinx,VDEV_UNMOUNT,EXTENDER_USB_UNMOUNT);
					}	
#endif					
				}	
			}	
			if (devinx < 8)
			{	
			    Remote_Mount_State &= ~BIT_MASK[devinx];
		    }
	    }
	}
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */
	printf("clar_b ");
	//1-final.Clear DC relative memory
	if (USB_PDevice[devinx].DevClass == USB_HUB_CLASS)
	{
		if (USBHC_Current_Total_Hub_Count)
			USBHC_Current_Total_Hub_Count--;
	}
	else
	{
		if (vHubClear)
		{
			USBDC_Free_VDevice_Memory(devinx);
		}
	}

	//2.Free the malloc buffer memory
	//2_1.Clear HC Descritpro Buffer
#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
	if (devinx == USBDC_VMSC_DEVINX) 
	{
		goto USBHC_CORE_Clear_Device_HID;
	}
#endif

	for (index=0; index < DESC_MAX ; index++)
	{
		if (USB_PDevice[devinx].Desc[index].Len)
		{
			malloc_free(USB_PDevice[devinx].Desc[index].Ptr);
		}
	}
	
	printf("clar_c ");
#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
USBHC_CORE_Clear_Device_HID:
#endif
	malloc_free(USB_PDevice[devinx].HID_Kb_Buf);
	malloc_free(USB_PDevice[devinx].HID_Kb_Buf2);
	malloc_free(USB_PDevice[devinx].HID_Kb_ParserTable);
	malloc_free(USB_PDevice[devinx].HUB.ReportState);

	if (USB_PDevice[devinx].DevClass == USB_MSC_CLASS)
	{
#if (SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT)
#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
		if (devinx != USBDC_VMSC_DEVINX) 
#endif
		{
#if (SYSTEM_EXTENDER_RECEIVER)
			if (USBHC_MSC_Device_Cnt) //over system limition.
			{	
			    USBHC_MSC_Device_Cnt--;
			}
#endif
		}
		
#if (SYSTEM_EXTENDER_RECEIVER) || (SYSTEM_KVM)
		if (devinx == USBHC_MSC_Devinx)
		{
			USBHC_MSC_Bulk_State_Reset(devinx);
			USBHC_MSC_Active_State &= ~MSC_ACTIVE_REPORT_UMOUNT; //if device has been remove, no answer
			USBHC_MSC_Devinx = 0xFF;
#if (SYSTEM_EXTENDER_RECEIVER)			
			KVM_CurrentMSC = REMOTE_HOST_PORT;
#else			
			KVM_CurrentMSC = 0;
#endif
			
			USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_IDLE;
		}
#endif //#if (SYSTEM_EXTENDER_RECEIVER)

#if (SYSTEM_EXTENDER_TRANSMITTER)
		if (devinx == EXTENDER_MSC_Devinx)
		{
			EXTENDER_MSC_Devinx = 0xFF;
			USBHC_MSC_Devinx = 0xFF;
			KVM_CurrentMSC = EXTENDER_HOST_PORT;
			USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_IDLE;
//			printf("T_MSC_Reset\n\r");
		}
#endif //#if (SYSTEM_EXTENDER_TRANSMITTER)

#if (SYSTEM_EXTENDER_MSC_SUPPORT)
		Extender_MSC_Init(0);
#endif
		if (USB_PDevice[devinx].MSC != NULL)
		{
			malloc_free(USB_PDevice[devinx].MSC->BOT); // Contain 31 byte CBW command header, 13 bytes CSW status header		
			malloc_free(USB_PDevice[devinx].MSC);
		}
#endif
	}
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	else if (USB_PDevice[devinx].DevClass == USB_AUDIO_CLASS)
	{
		if (USBHC_Audio_Device_Cnt)
		{
			USBHC_Audio_Device_Cnt--;
		}

		if (devinx == USBHC_Audio_Devinx)
		{
			//Disable prefetch anyway
			prefetch = DC_IEPPR_DISABLE_SET;
			for (index = 0; index < USBDC_PORT_MAX; index++)
			{
				USBDC_Write_Regs(index, DCIEPPR, &prefetch, 1);
			}

		    if (KVM_Flash.cSystemFlag0 & SYSTEM_AUDIO_REPORT_ALL_MASK)
		    {
				if (USBHC_Audio_Enumerate_Processing_Flag)
				{
					//Diable the mount opreation if the mount task is porcessing
					TASK_Destory_Task(USBHC_Audio_Enumerate_Task_Id);
					USBHC_Audio_Enumerate_Processing_Flag = 0;
				}
			}
			
			USBHC_Audio_Devinx = 0;
			KVM_CurrentUSBAudio = 0;
			USBHC_Audio_IN_Flag = 0;
			USBHC_Audio_IN_Start_Flag = 0;
		}
	}
#endif
	printf("clar_d ");
#if defined(HID_PARSER_CORE_KB)
	if (USB_PDevice[devinx].DevClass == USB_HID_CLASS)
	{
		USBHC_HidParser_Memory_Release(devinx);
	}	
#endif
	printf("clar_e ");
	//3.Scan ATL TD Table
	for (index=USB_HC_ATL_START; index < (USB_HC_ATL_START+USB_HC_ATL_MAX); index++)
	{
		if (HCTD_Channel_Table[index].Used_DeviceID & USBHC_TD_USED_MASK)
		{
			if ((HCTD_Channel_Table[index].Used_DeviceID & USBHC_TD_DEVICE_ADDR_MASK) == devinx)
			{
				HCTD_Channel_Table[index].Used_DeviceID = 0;
				USBHC_CORE_Clear_TD_Register(index);
			}
		}
	}

	//5.Scan INTL Polling Table
	for (index = 0; index < USB_HC_INTL_MAX; index++)
	{
		if (HC_IntTransfer_Table[index].Used_DeviceID & USBHC_TD_USED_MASK)
		{
			if ((HC_IntTransfer_Table[index].Used_DeviceID & USBHC_TD_DEVICE_ADDR_MASK) == devinx)
			{
				USBHC_CORE_Clear_TD_Register(index + USB_HC_ISTL_MAX);
				USBHC_CORE_SetBit(TD_INTL_Reload_Map, index, 0x00); // clear the interrupt auto relaod table
				//Check interrrupt Buffer

				if (HC_IntTransfer_Table[index].PassControl.BufSize == Hc_PASSTHROUGH_BUF_SIZ) // should release the buffer
				{
					malloc_free(HC_IntTransfer_Table[index].PassControl.Buf);
					HC_IntTransfer_Table[index].PassControl.Buf = 0;
				}
#if (SYSTEM_EXTENDER_RECEIVER)
				if (HC_IntTransfer_Table[index].ExtenderR != NULL)	// For Extender Receiver used
				{
					malloc_free(HC_IntTransfer_Table[index].ExtenderR);
					HC_IntTransfer_Table[index].ExtenderR = NULL;
				}
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */
				HC_IntTransfer_Table[index].Used_DeviceID = 0;
				intl_chagned_flag = 1;
			}
		}
	}

	if (intl_chagned_flag)
	{
		USBHC_Write_Regs(HC_INT_TD_RELOAD_REG,TD_INTL_Reload_Map,4); // in the final write into intl auto reload register
	}

	//6.Scan ISTL Polling Table
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	for (index = 0; index < USB_HC_ISTL_MAX; index++)
	{
		if ((HC_IsoTransfer_Table[index].Used_DeviceID & USBHC_TD_DEVICE_ADDR_MASK) == devinx)
		{
			HC_IsoTransfer_Table[index].Used_DeviceID = 0;
			USBHC_CORE_Clear_TD_Register(index);
			USBHC_CORE_SetBit(&TD_ISTL_Reload_Map, index, 0x00);  // clear the isochronous auto relaod table
			USBHC_Write_Regs(HC_ISTL_TD_RELOAD_REG, &TD_ISTL_Reload_Map, 1); // in the final write into istl auto reload register
		}
	}
#endif
	printf("clar_f ");
	//4.Clear Physical device table
#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
	if (devinx != USBDC_VMSC_DEVINX) 
#endif
	{
		if (USBHC_Current_Total_Device_Count && (devinx < USB_HC_MAX_DEVICE))
		{
			USBHC_Current_Total_Device_Count--;			
#if (SYSTEM_EXTENDER_RECEIVER)				
#ifdef PWAYTEK_01				
			ExtenderR_Maintain_DeviceState_Led();
#endif			
#endif
		}
		memset((U8_T *)&USB_PDevice[devinx],0x00,sizeof(USB_PDevice_TypeDef));
	}

	/*Special Handle for USB PEN DRIVE*/
#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
	else
	{ // PEN DRIVER STILL IN USED
		USB_PDevice[devinx].Addr |= USBHC_DEVICE_USED_MASK;
		return;
	}
#endif

	if (devinx < USB_HC_MAX_DEVICE && vHubClear)
	{
		index = USBDC_Virtual_Hub_Map[devinx].Devinx_HubPort;
		USBDC_Virtual_Hub_Map[index].HubPort_Devinx = 0;
		USBDC_Virtual_Hub_Map[devinx].Devinx_HubPort = 0;
	}

#if (SYSTEM_EXTENDER_TRANSMITTER) && (TRANSMITTER_CMD_DEBUG)
	printf(" (T)->TDevinx:%d,Clear Done\n\r",(U16_T)devinx);		
#endif  /* #if (TRANSMITTER_CMD_DEBUG)	*/

#if (SYSTEM_EXTENDER_TRANSMITTER) && defined(VIRTUAL_HUB_RESET) 			
	USBDC_Virtual_Hub_Reset(KVM_CurrentHost,1);
#endif
}

/**
* @brief  USBHC_CORE_Build_INTL_Polling
*         Check the device table, and issue the Interrupt TD 
* @param  pdev   : Selected device
* @param  Length : HID Report Descriptor Length
* @retval USBH_Status : Response for USB HID Get Report Descriptor Request
*/
void USBHC_CORE_Build_INTL_Polling(U8_T devinx)
{
	U8_T inf_index,end_index,free_id,interfacenum,endpinx;
	U16_T endpsiz;
#if (SYSTEM_EXTENDER_RECEIVER)	
	U16_T len;
#endif	

	interfacenum = USB_PDevice[devinx].InterfaceNum & USBHC_DEVICE_INTERFACENUM_MASK;
	for (inf_index=0; inf_index < interfacenum; inf_index++) // Check Eache Interface
	{
		for (end_index=0; end_index < USB_PDevice[devinx].EndpNum[inf_index]; end_index++) // Check all the endpoint in this interface
		{
			if ((USB_PDevice[devinx].EndpType[inf_index][end_index] & USB_EP_TYPE_MASK) == USB_EP_TYPE_INTR) //if endpoint is interrupt transfer
			{
				//1.Do the Keyboard Buffer Malloc
				if (USB_PDevice[devinx].InfClass[inf_index] == USB_HID_CLASS)
				{
					if (USB_PDevice[devinx].InfProtocol[inf_index] == INF_KEYBOARD)
					{
						if (USBHC_HidParser_Buf_Malloc(devinx,inf_index,end_index) == 0 )
						{
							//printf("> KVM ERROR:HID Parser Buffer Build FAIL,Dev=%d,Inf=%d\n\r",(U16_T)devinx,(U16_T)end_index);
							break;
						}
						USB_PDevice[devinx].HID_Kb_Press_Cnt = 0;
					}
					else
					{
						//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
						//1.Special handle for Gigabyte
						if ((USB_PDevice[devinx].idVendor == 0x060b) &&(USB_PDevice[devinx].idProduct == 0x2270))
						{
							if (USB_PDevice[devinx].InfProtocol[inf_index] == 0x00)
							{
								USB_PDevice[devinx].HID_Kb_Buf2 = m_malloc(16,1); //for page id 3 & page id 4
								if (!USB_PDevice[devinx].HID_Kb_Buf2)
									break;

								USB_PDevice[devinx].HID_Kb_Press_Cnt = 0;
								USB_PDevice[devinx].HID_Gigabyte_Kb_03_Cnt = 0;
								USB_PDevice[devinx].HID_Gigabyte_Kb_04_Cnt = 0;
								USB_PDevice[devinx].HID_Gigabyte_Kb_03_Flag = 0;
							}
						}
					}
				}

				//2.Build Interrupt TD
				if (USBHC_CORE_Alloc_IntTransfer_Table(&free_id))
				{
					HC_IntTransfer_Table[free_id].Used_DeviceID  |= devinx;
					HC_IntTransfer_Table[free_id].InterfaceIdx    = inf_index;
					HC_IntTransfer_Table[free_id].EndpIdx = end_index;
//					HC_IntTransfer_Table[free_id].ErrCnt = 0;
					HC_IntTransfer_Table[free_id].Control = 0;
					HC_IntTransfer_Table[free_id].INTL_Num = free_id;
					HC_IntTransfer_Table[free_id].EndpAddr = USB_PDevice[devinx].EndpAddr[inf_index][end_index];
					//Get the Endpinx 
					USBDC_Search_EndpAddr(devinx,HC_IntTransfer_Table[free_id].EndpAddr,&endpinx);
					if (devinx < USB_HC_MAX_DEVICE)
					{
						USBDC_Device[devinx].EndpType[endpinx]= (USB_EP_TYPE_INTR | (free_id & 0x0f));
					}
					endpinx <<= 4;
					HC_IntTransfer_Table[free_id].EndpIdx |= endpinx;

					//Store the TD number into Physical device table															
					USB_PDevice[devinx].EndpType[inf_index][end_index] |= (free_id & 0x1f);
					endpsiz = USB_PDevice[devinx].EndpSize[inf_index][end_index];

					if (USBHC_InterruptReqData(devinx,free_id) == USBH_OK)
					{
						//Create the upstream INTL buffer
						if (endpsiz <= 16) //if the interrupt is small than 16 bytes, then used the intl td buffer directly
						{
#if (SYSTEM_EXTENDER_TRANSMITTER)							
							if (USB_PDevice[devinx].InfClass[inf_index] == USB_HID_CLASS)
							{
								if (USB_PDevice[devinx].InfProtocol[inf_index] == INF_KEYBOARD)							
								{
									goto USBHC_HID_BUFFER_MALLOC;
								}
							}	
#endif								
							HC_IntTransfer_Table[free_id].PassControl.BufSize = USB_HC_INTL_BUF_SIZE - endpsiz; // buffer total size
							HC_IntTransfer_Table[free_id].PassControl.Buf = &HCTD_Table.INTL[free_id].Buf[endpsiz]; // buffer start pointer
							HC_IntTransfer_Table[free_id].PassControl.State = PASSTHROUGH_BUF_TYPE_TD;
						}
						else
						{
#if (SYSTEM_EXTENDER_TRANSMITTER)														
USBHC_HID_BUFFER_MALLOC:							
#endif	
							HC_IntTransfer_Table[free_id].PassControl.BufSize = Hc_PASSTHROUGH_BUF_SIZ; // buffer total size
							HC_IntTransfer_Table[free_id].PassControl.Buf = m_malloc(Hc_PASSTHROUGH_BUF_SIZ,2);
							if (HC_IntTransfer_Table[free_id].PassControl.Buf)
							{
								//printf("Malloc fail in Interrupt,Please Called\n\r");							
								HC_IntTransfer_Table[free_id].PassControl.State = PASSTHROUGH_BUF_TYPE_XDAT;
							}
						}
						USBHC_InterruptTD_Transfer_Index_Init(free_id);
						//HC_IntTransfer_Table[free_id].PassControl.Rp = 0; 
						//HC_IntTransfer_Table[free_id].PassControl.Wp = 0; 

#if (SYSTEM_EXTENDER_RECEIVER)
 						//ExtenderR_Malloc_Intr_Buffer(free_id,endpsiz); 						
						len = endpsiz+sizeof(Extender_Packet_Header_Def)+2;
						HC_IntTransfer_Table[free_id].ExtenderR = m_malloc(len,3);
						if (HC_IntTransfer_Table[free_id].ExtenderR == NULL)
						{
							return;
						}						
#endif	/* #if (SYSTEM_EXTENDER_RECEIVER) */

						if (USB_PDevice[devinx].InfClass[inf_index] == USB_HUB_CLASS)
						{
							USB_PDevice[devinx].HUB.IntTD = free_id;// offset TD id of Interrupt Transfer
						}
					}
				}
				else
				{
					USB_PDevice[devinx].HUB.IntTD = 0xff;// offset TD id of Interrupt Transfer
				}
			}
		}
		
		if (USBDC_VHID_DEVINX == devinx)
		{	
			USBDC_HID_Inttid[inf_index] = free_id;
		}	
	}
}

/*
 * -------------------------------------------------------------------------------------------
 * void USBHC_CORE_Set_INTL_ReloadMap(void)
 * Purpose : Set the INTL Auto Reload Map Register
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------
 */
void USBHC_CORE_Set_INTL_ReloadMap(U8_T tdid,U8_T value)
{
	USBHC_CORE_SetBit(TD_INTL_Reload_Map,tdid,value);
	USBHC_Write_Regs(HC_INT_TD_RELOAD_REG,TD_INTL_Reload_Map,4);
}

/*
 * -------------------------------------------------------------------------------------------
 * void USBHC_CORE_Suspend(void)
 * Purpose : 
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------
 */
void USBHC_CORE_Suspend(void)
{
	U8_T control_reg;

	EXTINT2_DISABLE;

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	EXTINT3_DISABLE;
#endif
	/*Read Back the Register00,first*/
	_USBHC_HCCIR_SFR(HC_CONTROL_REG);
	_USBHC_HCDR_READ_SFR(control_reg);
	control_reg &= ~HCFS_MASK;
	control_reg |= HCFS_SUSPEND;
	_USBHC_HCDR_SFR(control_reg);
	_USBHC_HCCIR_SFR(HC_CONTROL_REG);
	EXTINT2_ENABLE;

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)	
	EXTINT3_ENABLE;
#endif
}

/*-------------------------------------------------------------------------------------------
 * void USBHC_CORE_Resume(void)
 * Purpose : 
 * Params  :  
 * Returns : none
 * Note    : none
 *-------------------------------------------------------------------------------------------*/
void USBHC_CORE_Resmue(void)
{
	U8_T control_reg;

	EXTINT2_DISABLE;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	EXTINT3_DISABLE;
#endif
	/*Read Back the Register00,first*/
	_USBHC_HCCIR_SFR(HC_CONTROL_REG);
	_USBHC_HCDR_READ_SFR(control_reg);
	control_reg &= ~HCFS_MASK;
	control_reg |= HCFS_RESUME;
	_USBHC_HCDR_SFR(control_reg);
	_USBHC_HCCIR_SFR(HC_CONTROL_REG);
	EXTINT2_ENABLE;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	EXTINT3_ENABLE;
#endif

}

/*-------------------------------------------------------------------------------------------
 * void USBHC_CORE_Build_ISTL_Polling(U8_T devinx)
 * Purpose : build the istl tranfer table & ISTL0 relative setting
 * Params  :  
 * Returns : none
 * Note    : none
 *-------------------------------------------------------------------------------------------*/
void USBHC_CORE_Build_ISTL_Polling(U8_T devinx)
{
	U8_T	inf_index,end_index,td_id,interfacenum;

	interfacenum = USB_PDevice[devinx].InterfaceNum & USBHC_DEVICE_INTERFACENUM_MASK;
	for (inf_index=0; inf_index < interfacenum; inf_index++) // Check Eache Interface
	{
		for (end_index=0; end_index < USB_PDevice[devinx].EndpNum[inf_index]; end_index++) // Check all the endpoint in this interface
		{
			if ((USB_PDevice[devinx].EndpType[inf_index][end_index] & USB_EP_TYPE_MASK) == USB_EP_TYPE_ISOC) //if endpoint is interrupt transfer
			{
				/* Check out endpoint direction */
				if (USB_PDevice[devinx].EndpAddr[inf_index][end_index] & USB_EP_DIR_IN) // IN
				{
					td_id = 1; // ISTL[1] td for IN direction
				}
				else
				{				
					td_id = 0; // ISTL[0] td for OUT direction
				}
				
				/* Request an isochornous software table */
				if (USBHC_CORE_QueryAlloc_IsoTransfer_Table(td_id) == FALSE)
				{
					//printf("No free ISTL can used\n\r");
					return;
				}

				/* Fill the table */
				HC_IsoTransfer_Table[td_id].Used_DeviceID |= devinx;
				HC_IsoTransfer_Table[td_id].InterfaceIdx = inf_index;
				HC_IsoTransfer_Table[td_id].EndpIdx = end_index;
				HC_IsoTransfer_Table[td_id].EndpAddr = USB_PDevice[devinx].EndpAddr[inf_index][end_index];                
				/* Keep the table id in USB_PDevice[] */
				USB_PDevice[devinx].EndpType[inf_index][end_index] |= (td_id & 0x1f);
				
				/* Fill the ISTL TD */
				USBHC_Istl_ReqData(devinx, td_id);

				/* Clear the DoneMap */
				USBHC_CORE_Clear_TD_DoneMap(USBHC_TD_ISTL, td_id);
			}
		}
	}
}

/*-------------------------------------------------------------------------------------------
 * void USBHC_CORE_BULK_Done_Handle(U8_T devinx)
 * Purpose : Handle the BULK Done event
 * Params  :  
 * Returns : none
 * Note    : none
 *-------------------------------------------------------------------------------------------*/
#if (SYSTEM_MSC_DEVICE_SUPPORT)
void USBHC_CORE_BULK_Done_Handle(void)
{
	U8_T		atl_id;
	U8_T		bulk_id=0;
	U8_T		devinx;
	U8_T		td_id,endpinx;
#if (SYSTEM_USB_HC_BURST)
	U16_T		actual_byte;
#endif
	USBH_Status	USB_Status; 
	RESULT		Result=USB_SUCCESS;

	atl_id = USBHC_BULK_Done_Q[USBHC_ATL_Done_Q_Rp] & 0x1f;
	devinx = (USBHC_BULK_Done_Q[USBHC_ATL_Done_Q_Rp] & 0xe0) >> 5;
	
	//Skip task if the device is gone
	if ((USB_PDevice[devinx].Addr & USBHC_DEVICE_USED_MASK) == 0x00)  // Device is gone  
	{
		goto USBHC_MSC_Bulk_Done_Handle_Exit;
	}
	
	if (devinx == USBHC_MSC_Devinx)
	{
		td_id  = atl_id + USB_HC_ATL_START; 
#if (SYSTEM_USB_HC_BURST)
		bulk_id = atl_id - USB_HC_MSC_START;
		USBHC_CORE_Retrive_USB_State(USBHC_TD_BULK, atl_id, td_id);
#else
		bulk_id = atl_id;
		USBHC_CORE_Retrive_USB_State(USBHC_TD_ATL, atl_id, td_id);
#endif /*SYSTEM_USB_HC_BURST*/
		/*Check the complete status */
		USB_Status = HCTD_Channel_Table[td_id].USB_State;

USBHC_MSC_Bulk_Done_Handle_Restart:
		switch (USB_Status)
		{
			case USBH_DONE:
#if (SYSTEM_USB_HC_BURST)
  #if (SYSTEM_EXTENDER_RECEIVER && SYSTEM_EXTENDER_MSC_SUPPORT)
				if (KVM_CurrentMSC == REMOTE_HOST_PORT)
				{
					Result = ExtenderR_MSC_Burst_Done_Handle(devinx,bulk_id, 0);
				}
				else
  #endif //#if (SYSTEM_EXTENDER_RECEIVER)  
					Result = USBHC_MSC_BOT_Done_Handle(devinx, bulk_id, 0);
#endif
				MSC_Nake_Cnt= 0;
				
				if (Result == USB_WAIT)
					return;
				break;
			case USBH_NAKE:
				MSC_Nake_Cnt++;
				if (MSC_Nake_Cnt < 5000)
				{											
					HCTD_Table.BULK[bulk_id].TD.Byte1 |= (COMPLETEION_CODE_MASK | TD_ACTIVED_MASK);	
					USBHC_CORE_SetBit(TD_ATL_Skip_Map,(bulk_id+USB_HC_MSC_START),0x00);
					USBHC_Write_Regs(HC_ATL_TDSKIP_MAP_REG, TD_ATL_Skip_Map, 4);	
					break; 
				}
				else
				{
 					//printf("MSC:USBH_NAKE => Dismount MSC Device\n\r");
					USB_Status = USBH_DONE;
					USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_END;
					HCTD_Table.BULK[bulk_id].TD.Byte0_Actual_Byte = 0;
					
					/* Testing */
					if (USB_PDevice[devinx].DevClass == USB_MSC_CLASS)
					{	
						//CPU_SoftReset();
						USBHC_MSC_DismountFlag = 1;
					}	
					
					goto USBHC_MSC_Bulk_Done_Handle_Restart;
				}		
/*								
				MSC_Nake_Cnt = 0;
				
				if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_CSW_REQUEST)
				{
//					printf("CSW");
					USB_Status = USBH_DONE;
					USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_END;
#if (SYSTEM_USB_HC_BURST)
					HCTD_Table.BULK[bulk_id].TD.Byte0_Actual_Byte = USBHC_MSC_BOT_CSW_LENGTH;
#else
					HCTD_Table.ATL[atl_id].TD.Byte0_Actual_Byte = USBHC_MSC_BOT_CSW_LENGTH;
#endif
					goto USBHC_MSC_Bulk_Done_Handle_Restart;
				}
*/				
				break;
			case USBH_TIMEOUT:
			case USBH_ERROR:
				//printf("USBH_ERROR:0x%02bx\n\r",USB_Status);
				if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_DATA_IN)
				{
#if (SYSTEM_USB_HC_BURST)
					actual_byte = (U16_T) ((HCTD_Table.BULK[bulk_id].TD.Byte1 & 0x03) << 8) | HCTD_Table.BULK[bulk_id].TD.Byte0_Actual_Byte;
					printf("Actual_Byte=%d\n\r",actual_byte);
					if (USB_PDevice[devinx].MSC->Control.Total_Length >= actual_byte)
					{
						USB_PDevice[devinx].MSC->Control.Total_Length -= actual_byte;
					}
  #if (SYSTEM_EXTENDER_RECEIVER && SYSTEM_EXTENDER_MSC_SUPPORT)
					if (KVM_CurrentMSC == REMOTE_HOST_PORT)
					{
						ExtenderR_MSC_Burst_Done_Handle(devinx,bulk_id, 1);
					}
					else
  #endif //#if (SYSTEM_EXTENDER_RECEIVER)
						USBHC_MSC_BOT_Done_Handle(devinx, bulk_id, 1);
#endif
					break;
				}
				else if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_CSW_REQUEST)
				{
					USB_Status = USBH_DONE;
					goto USBHC_MSC_Bulk_Done_Handle_Restart;
				}
				else if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_END)
				{
					USB_Status = USBH_DONE;
					USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_CSW_REQUEST;
					goto USBHC_MSC_Bulk_Done_Handle_Restart;
				}
				break;
			case USBH_STALL:
				/* Control transfers completed, Exit the State Machine */
#if (SYSTEM_USB_HC_BURST)
				if ((HCTD_Table.BULK[bulk_id].TD.Byte5 & TD_DIR_TOKEN_MASK)==TD_DIR_TOKEN_OUT)
				{
					endpinx = USB_PDevice[devinx].MSC->Out_Endpinx;
					USB_PDevice[devinx].MSC->Toggle_Out = 0;
				}	
				else
				{
					endpinx = USB_PDevice[devinx].MSC->In_Endpinx;
					USB_PDevice[devinx].MSC->Toggle_In = 0;
				}
				USB_PDevice[devinx].MSC->BOT_State = MSC_BOT_IDLE;
				//USBHC_MSC_Bulk_Clear_Pipe(devinx, (HCTD_Table.BULK[bulk_id].TD.Byte5 & TD_DIR_TOKEN_MASK));
				
#if (SYSTEM_EXTENDER_RECEIVER && SYSTEM_EXTENDER_MSC_SUPPORT)
				if (KVM_CurrentMSC == REMOTE_HOST_PORT)
				{
					if ((HCTD_Table.BULK[bulk_id].TD.Byte5 & TD_DIR_TOKEN_MASK) == TD_DIR_TOKEN_OUT)
						ExtenderR_PassthroughMscBulkInStall(devinx, TD_DIR_TOKEN_OUT, bulk_id);
					else
						ExtenderR_PassthroughMscBulkInStall(devinx, TD_DIR_TOKEN_IN, bulk_id);
				}
				else
#endif //#if (SYSTEM_EXTENDER_RECEIVER && SYSTEM_EXTENDER_MSC_SUPPORT)				
					USBDC_HAL_Endp_Stalled(KVM_CurrentMSC,devinx,endpinx);
#endif //#if (SYSTEM_USB_HC_BURST)
				USBHC_MSC_State |= MSC_STATE_BULK_IN;
				break;
			case USBH_DATA_TOGGLE_ERR:
#if (SYSTEM_USB_HC_BURST == 0)
				printf("Data Toggle,atl=%d,outp=%d\n\r",(U16_T)atl_id,(U16_T)USBHC_MSC_Td_Outp);
#endif
				if (USB_PDevice[devinx].MSC->BOT_State == MSC_BOT_DATA_IN)
				{
					USB_Status = USBH_DONE;
					goto USBHC_MSC_Bulk_Done_Handle_Restart;
				}
				break;
			default:
				//printf("MSC TD Transmitte ERROR=%02x\n\r",(U16_T)USB_Status);
				break;
		}
	}

USBHC_MSC_Bulk_Done_Handle_Exit:
	if (++USBHC_ATL_Done_Q_Rp >= BULK_DONE_QUEUE_MAX)
	{
		USBHC_ATL_Done_Q_Rp = 0;
	}

}
#endif
/* End of usbhc_core.c */

