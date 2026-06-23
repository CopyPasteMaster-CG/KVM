/*
 *********************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : interrupt.c
 * Purpose     : This file include peripheral interrupt service routine in EINT4, 
 *               peripheral interrupt service routine in EINT5, and power management
 *               wake-up interrupt service routine in EINT6. Also provide a watchdog
 *               interrupt for reference.
 * Author      : 
 * Date        :
 * Notes       :
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	"project_include.h"
#include	"spim.h"

/* NAMING CONSTANT DECLARATIONS */
/* GLOBAL VARIABLE DECLARATIONS */
U8_T	spiStatus = 0;
//U8_T    buzzer_flag=0; 
extern SPI_MST_INST_T	spi_Mst;

/* STATIC VARIABLE DECLARATIONS */
/* LOCAL SUBPROGRAM DECLARATIONS */
static void intr_PeripheralIntr4ISR(void);
static void intr_PeripheralIntr5ISR(void);
static void intr_PmmWakeUpISR(void);

#if (INTR_ISR_INCLUDE_WATCHDOG)
static void intr_WatchDogISR(void);
#endif

/* LOCAL SUBPROGRAM BODIES */
static void intr_Timer0ISR(void) interrupt TF0_VECTOR //use timer0 interrupt 1 (0x0B)
{
	P3_1 = 0;
	TR0 = 0; // disable timer0
	//TH0 = 0xFA; // timer start from 0x00
	//TL0 = 0x50; 	    
	/*
	if (state == BUZZER_ON)
	{
		BCR_BK |= BUZZER_SE_MASK;
		BCR_BK &= ~BUZZER_TSE_SET;
	}
	else
	{
		BCR_BK |= BUZZER_TSE_SET;
		BCR_BK &= ~BUZZER_SE_MASK;
	}
	*/
	//_MISC_DR_SFR(BCR_BK);
	//_MISC_CIR_SFR(MISC_BCR);
	
	/*
	if (buzzer_flag)
	{//buffer on
		buzzer_flag = 0;
		P3_0 = 0;
		BCR_BK |= BUZZER_TSE_SET;
		BCR_BK &= ~BUZZER_SE_MASK;
		TH0 = BUZZER_Factor_TableH0[BUZZER_Current_Fector]; // timer start from 0x00
		TL0 = BUZZER_Factor_TableL0[BUZZER_Current_Fector]; 	    
	}	
    else
	{//Buffer off
		buzzer_flag = 1;
		P3_0 = 1;		
		BCR_BK |= BUZZER_SE_MASK;
		BCR_BK &= ~BUZZER_TSE_SET;
		TH0 = BUZZER_Factor_TableH1[BUZZER_Current_Fector]; // timer start from 0x00
		TL0 = BUZZER_Factor_TableL1[BUZZER_Current_Fector]; 	    
	}		
	_MISC_DR_SFR(BCR_BK);
	_MISC_CIR_SFR(MISC_BCR);
	*/
	
	TR0 = 1;
	//ET0 = 1;	
}	

/*
 *--------------------------------------------------------------------------------
 * static void intr_PeripheralIntr4ISR(void)
 * Purpose : The peripheral interface interrupt service routine of
 *           PS2A_INT_STU		0x01
 *           PS2B_INT_STU		0x02
 *           GP0_INT_STU		0x04
 *           GP2_INT_STU		0x08
 *           SPI1_INT_STU		0x10
 *           SPI0_INT_STU		0x20
 *           I2C_INT_STU		0x40
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
static void intr_PeripheralIntr4ISR(void) interrupt INT4_VECTOR //use external interrupt 4 (0x4B)
{
	idata U8_T intrStt = 0;
	idata U8_T state;
#if (KVM_BUTTON_SUPPORT) & (KVM_BUTTON_HARDWARE)	
#ifdef BTN_RESET
	idata U8_T regvalue0,regvalue1;
#endif
#endif	
#if (INTR_ISR_INCLUDE_I2C)
	idata U8_T state2;
#endif
	/* Interrupt type check */
	intrStt = PISSR;

#if (INTR_ISR_INCLUDE_PS2A)
	if (intrStt & PS2A_INT_STU)
	{
#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
#if (PS2_WAKEUP_SUPPORT)
		PowerSavingInhibitFlag = 1;
#endif //#if (PS2_WAKEUP_SUPPORT)
#endif //#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)

		//1.read in status
		_PS2_CIR_SFR(PS2A_SR);
		_PS2_DR_READ_SFR(state);
		HPS2_ISR_FIFO[HPS2_ISR_FIFO_WP].State = state;
		HPS2_ISR_FIFO[HPS2_ISR_FIFO_WP].PID = HPS2A;

		//2.innhibit clock
		if (state & PS2SR_RX_INTR)
		{
			//1.Read in data
			_PS2_CIR_SFR(PS2A_DR);
			_PS2_DR_READ_SFR(HPS2_ISR_FIFO[HPS2_ISR_FIFO_WP].Data);
			HPS2_CR_BK[HPS2A] |= PS2CR_INH_FORCE;
			_PS2_DR_SFR(HPS2_CR_BK[HPS2A]);
			_PS2_CIR_SFR(PS2A_CR);
		}
		HPS2_ISR_FIFO_WP++;
		if (HPS2_ISR_FIFO_WP >= HPS2_ISR_FIFO_DEPTH_MAX)
			HPS2_ISR_FIFO_WP = 0;
	}
#endif //#if (INTR_ISR_INCLUDE_PS2A)

#if (INTR_ISR_INCLUDE_PS2B)
	if (intrStt & PS2B_INT_STU)
	{
#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
		PowerSavingInhibitFlag = 1;
#endif //#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
		_PS2_CIR_SFR(PS2B_SR);
		_PS2_DR_READ_SFR(state);
		HPS2_ISR_FIFO[HPS2_ISR_FIFO_WP].State = state;
		HPS2_ISR_FIFO[HPS2_ISR_FIFO_WP].PID = HPS2B;
		if (state & PS2SR_RX_INTR)
		{
			_PS2_CIR_SFR(PS2B_DR);
			_PS2_DR_READ_SFR(HPS2_ISR_FIFO[HPS2_ISR_FIFO_WP].Data);
			HPS2_CR_BK[HPS2B] |= PS2CR_INH_FORCE;
			_PS2_DR_SFR(HPS2_CR_BK[HPS2B]);
			_PS2_CIR_SFR(PS2B_CR);
		}
		HPS2_ISR_FIFO_WP++;
		if (HPS2_ISR_FIFO_WP >= HPS2_ISR_FIFO_DEPTH_MAX)
			HPS2_ISR_FIFO_WP = 0;
	}
#endif //#if (INTR_ISR_INCLUDE_PS2B)

#if (INTR_ISR_INCLUDE_I2C)
	if (intrStt & I2C_INT_STU)
	{
		_I2C_CIR_SFR(I2CMISR);
		_I2C_DR_READ_SFR(state);
		_I2C_CIR_SFR(I2CMSR);
		_I2C_DR_READ_SFR(state2);
		
#ifdef I2C_BUS
		if ((state & (I2CMISR_TC |I2CMISR_ARB_LOST)) == I2CMISR_TC) // transfer complete
		{			
			if (I2C_Control_State & I2C_CONTROL_TRANSMIT)
			{
				if (I2C_Transmit_End == I2C_Transmit_Index) // last byte
				{
					// Send out the Addres byte via I2c Master
					if (I2C_Control_State & I2C_CONTROL_USE_RING)		
					{	
						_I2C_DR_SFR(I2C_RingBuffer[I2C_Tx_Start]);						
						I2C_Tx_Start++;
						if (I2C_Tx_Start >= I2C_BUF_MAX)
							I2C_Tx_Start = 0;	
					}	
					else
						_I2C_DR_SFR(I2C_Transmit_Buf[I2C_Transmit_Index]);	
					_I2C_CIR_SFR(I2CMTR);
					
					// I2C Master GO with STOP bit
					if ((I2C_Control_State & I2C_CONTROL_TRANSMIT_NOSTOP) == 0)
					{	
						_I2C_DR_SFR(I2CMCR_MASTER_GO | I2CMCR_CMD_WRITE | I2CMCR_STOP_COND);							
					}	
					else
					{
						_I2C_DR_SFR(I2CMCR_MASTER_GO | I2CMCR_CMD_WRITE);							
					}
					_I2C_CIR_SFR(I2CMCR);						
				}					
				else
				{	
					if (I2C_Transmit_Index == I2C_Transmit_Len) // last byte has been done
					{
						ISR_FIFO[ISR_FIFO_Wp].ISR_Type = ISR_I2C;						
						ISR_FIFO[ISR_FIFO_Wp].State = state;						
						ISR_FIFO[ISR_FIFO_Wp].Data = I2C_CONTROL_TRANSMIT;						
						ISR_FIFO_Wp++;
						if (ISR_FIFO_Wp >= ISR_FIFO_DEPTH)
							ISR_FIFO_Wp = 0;
					}
					else
					{
						// Send out the content
						if (I2C_Control_State & I2C_CONTROL_USE_RING)		
						{	
							_I2C_DR_SFR(I2C_RingBuffer[I2C_Tx_Start]);						
							I2C_Tx_Start++;
							if (I2C_Tx_Start >= I2C_BUF_MAX)
								I2C_Tx_Start = 0;	
						}	
						else
							_I2C_DR_SFR(I2C_Transmit_Buf[I2C_Transmit_Index]);	
						
						_I2C_CIR_SFR(I2CMTR);
						
						// I2C Master GO with normal byte
						_I2C_DR_SFR(I2CMCR_MASTER_GO | I2CMCR_CMD_WRITE);	
						_I2C_CIR_SFR(I2CMCR);					
					}	
				}													
				I2C_Transmit_Index++;
			}	
			else
			{
				// Read in
				_I2C_CIR_SFR(I2CRR);
				if (I2C_Control_State & I2C_CONTROL_USE_RING)
				{	
					_I2C_DR_READ_SFR(I2C_RingBuffer[I2C_Tx_Start]);									
					I2C_Tx_Start++;
					if (I2C_Tx_Start >= I2C_BUF_MAX)
						I2C_Tx_Start = 0;
				}	
				else	
					_I2C_DR_READ_SFR(I2C_Transmit_Buf[I2C_Transmit_Index-1]);									
				if (I2C_Transmit_End == I2C_Transmit_Index) // last byte
				{					
					// I2C Master go for last byte read(no ACK)
					if ((I2C_Control_State & I2C_CONTROL_TRANSMIT_NOSTOP) == 0)
					{	
						_I2C_DR_SFR(I2CMCR_MASTER_GO | I2CMCR_CMD_READ | I2CMCR_STOP_COND);	
					}
					else	
					{	
						_I2C_DR_SFR(I2CMCR_MASTER_GO | I2CMCR_CMD_READ);	
					}	
					_I2C_CIR_SFR(I2CMCR);									
				}					
				else
				{	
					if (I2C_Transmit_Index == I2C_Transmit_Len) // last byte has been done
					{
						ISR_FIFO[ISR_FIFO_Wp].ISR_Type = ISR_I2C;
						ISR_FIFO[ISR_FIFO_Wp].State = state;		
						ISR_FIFO[ISR_FIFO_Wp].Data  = I2C_CONTROL_RECEIVE;					
						ISR_FIFO_Wp++;
						if (ISR_FIFO_Wp >= ISR_FIFO_DEPTH)
							ISR_FIFO_Wp = 0;
					}
					else
					{												
						// I2C Master GO with normal byte read(with ACK)
						_I2C_DR_SFR(I2CMCR_MASTER_GO | I2CMCR_CMD_READ);	
						
						_I2C_CIR_SFR(I2CMCR);					
					}	
				}											
				I2C_Transmit_Index++;
			}			
		}	
		else
		{
			/* Send out stop */				
/*			
#if (SYSETM_KEYPAD_CONTROL_ENABLE)			
			if (I2C_Control_Target == I2C_CH455)
			{	
				if (state & I2CMISR_NO_ACK)
				{
					goto I2C_Interrupt_Fource_Complete;
				}	
			}	
#endif
*/
			ISR_FIFO[ISR_FIFO_Wp].ISR_Type = ISR_I2C;
			ISR_FIFO[ISR_FIFO_Wp].State = state;	
			ISR_FIFO[ISR_FIFO_Wp].Data  = 0; // 						
			ISR_FIFO_Wp++;
			if (ISR_FIFO_Wp >= ISR_FIFO_DEPTH)
				ISR_FIFO_Wp = 0;
		}	
#endif		
						
		if (state & (I2CMISR_PSC | I2CMISR_EDID_DONE))
		{
			ISR_FIFO[ISR_FIFO_Wp].ISR_Type = ISR_EDID;
			ISR_FIFO[ISR_FIFO_Wp].State = state;
			ISR_FIFO[ISR_FIFO_Wp].Data = state2;
			
			if (++ISR_FIFO_Wp >= ISR_FIFO_DEPTH)
			{
				ISR_FIFO_Wp = 0;
			}
		}
	}
#endif //#if (INTR_ISR_INCLUDE_I2C)

#if (INTR_ISR_INCLUDE_SPI_MASTER)
	if (intrStt & SPI0_INT_STU)
	{
#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
		PowerSavingInhibitFlag = 1;
#endif //#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)

		SPICIR = SPIMISR;
		spiStatus = SPIDR;
		spi_Mst.IntrStatus |= (spiStatus & 0x03);
		
		if (spiStatus & SPIMISR_DMAERRS) // when DMA error collision
		{
			/*Write error handling code here*/
		}
		
		if (spiStatus & SPIMISR_SDTCS) // when master complete a DMA transfer		
		{
			spi_Mst.ActFlag &= ~(SPI_DMA_TXRX_CPL);
		}
	}
#endif //#if (INTR_ISR_INCLUDE_SPI_MASTER)

#if (INTR_ISR_INCLUDE_SPI_SLAVE)
	if (intrStt &SPI1_INT_STU)
	{
#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
		PowerSavingInhibitFlag = 1;
#endif //#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
		SPI_SlvIntrEntryFunc();
	}
#endif //#if (INTR_ISR_INCLUDE_SPI_SLAVE)

#if (INTR_ISR_INCLUDE_GPIO0)
	if (intrStt & GP0_INT_STU)
	{
		_MISC_CIR_SFR(MISC_GP0ISR);
		_MISC_DR_READ_SFR(cpu_Gpio0IntrStatus);	
#if (MCU_REAL_CHIP)
		#if (SYSTEM_IR_DEVICE_SUPPORT)		
		if (cpu_Gpio0IntrStatus & (IR_PIN_MASK | 0x0f)) // this is the btn mask
		#else
		if (cpu_Gpio0IntrStatus & 0x0f) // this is the btn mask
		#endif	
#else		
		if (cpu_Gpio0IntrStatus & 0xf0) // this is the btn mask	
#endif /* #if (MCU_REAL_CHIP) */			
		{
#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
			PowerSavingInhibitFlag = 1;
#endif //#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
			if (KVM_System_Not_Ready_Flag == 0)
			{	
#if (SYSTEM_IR_DEVICE_SUPPORT)
				if (cpu_Gpio0IntrStatus & IR_PIN_MASK)
				{											
					P3_0 = 0;
					IR_GPIO_PIN = 1; // Pull up the IR input pin to prevent the output low by AX68002
					if ((IR_Control_State & IR_START) == 0x00) //the data start bit not start
					{						
   						IR_Control_State |= (IR_WAIT_HIGH | IR_START);
   						IR_Plus_Cnt = 0;
   						IR_Buffer_Cnt = 0;   			
   						IR_Buffer[0] = 0;
   						IR_Edge_Setting[1] &= ~(BTN_TRIGGER_MASK << IR_TRIGGER_LEVEL_SHIFT);
						IR_Edge_Setting[1] = IR_Edge_Setting[1] | (BTN_TRIGGER_RISING << IR_TRIGGER_LEVEL_SHIFT);		
						_MISC_DR_SFR(IR_Edge_Setting[0]);
						_MISC_DR_SFR(IR_Edge_Setting[1]);
						_MISC_CIR_SFR(MISC_GP0TR);					
					}
					else
					{						
						if (IR_Control_State & IR_WAIT_HIGH)
						{								 
							//2-0.next must wait level trigger by falling edge
							{
								IR_Edge_Setting[1] &= ~(BTN_TRIGGER_MASK << IR_TRIGGER_LEVEL_SHIFT);
								IR_Edge_Setting[1] = IR_Edge_Setting[1] | (BTN_TRIGGER_FALLING << IR_TRIGGER_LEVEL_SHIFT);	
								_MISC_DR_SFR(IR_Edge_Setting[0]);
								_MISC_DR_SFR(IR_Edge_Setting[1]);
								_MISC_CIR_SFR(MISC_GP0TR);							
							}										
							IR_Control_State &= ~IR_WAIT_HIGH;
							
							//2-1.Check start bit valid condition-first plus must 9 ms
							if ((IR_MS_Cnt >= 8) && (IR_MS_Cnt <= 10))
							{																								
							}	
							else
							{	//Fail
								IR_Control_State &= ~IR_START;								
							}									
						}	
						else
						{							
							//3-0.Check start bit valid condition-second plus must be 4 ms
							if (IR_Plus_Cnt == 0)
							{	
								//ISR_FIFO[ISR_FIFO_Wp].Timer = SWTIMER_Counter;		
								if ((IR_MS_Cnt >= 4) && (IR_MS_Cnt <= 5))
								{								
									IR_Plus_Cnt=1; //next bit 1									
									IR_Plus_Timer0 = (TH0 << 8);
									IR_Plus_Timer0 += TL0;																				
								}	
								else
								{	//Fail.. clear the ir condition state
									IR_Control_State &= ~IR_START;								
								}								
							}
							else
							{	
								if (IR_MS_Cnt <= 4)
								{								
									IR_Plus_Timer1 = TH0 << 8;
									IR_Plus_Timer1 += TL0;								 
				   					if (IR_Plus_Timer1 >= IR_Plus_Timer0)
				   					{			   							
				   						IR_Plus_Timer_Dif = (IR_Plus_Timer1 - IR_Plus_Timer0);
				   					}
				   					else
				   					{
				   						IR_Plus_Timer_Dif = (0xffff - IR_Plus_Timer0)+IR_Plus_Timer1;
				   					}		
				   					IR_Plus_Timer0 = IR_Plus_Timer1; // reset the timer0
				   					//timer0 clock duty->1/96 and divide 12, for 1T, do each clock is 1/8=0.125us
				   					//for 2ms = 16000,			   					
				   					if (IR_Plus_Timer_Dif >= IR_2MS_Counter) //this is bit value 1
				   					{			   						
				   						IR_Buffer[IR_Buffer_Cnt] |= (0x01 << (IR_Plus_Cnt-1));			   						
				   					}
				   					else
				   					{
				   						if (IR_Plus_Timer_Dif > IR_1MS_Counter) //this is bit value 0
				   						{
				   							//ISR_FIFO[ISR_FIFO_Wp].Data = 0x14;	
				   						}	
				   						else
				   						{ //error			   										   							
				   								IR_Control_State &= ~IR_START;			   							
				   						}	
				   					}		
				   								   					
				   					IR_Plus_Cnt++;
				   					if (IR_Plus_Cnt > 8) // byte complete
				   					{
				   						   						
				   						IR_Plus_Cnt = 1; //next is bit 1.
				   						ISR_FIFO[ISR_FIFO_Wp].ISR_Type = ISR_IR;									
										ISR_FIFO[ISR_FIFO_Wp].Data = IR_Buffer[IR_Buffer_Cnt];
										IR_Buffer_Cnt++;			
				   						IR_Buffer[IR_Buffer_Cnt] = 0;
										ISR_FIFO_Wp++;
										if (ISR_FIFO_Wp >= ISR_FIFO_DEPTH)
											ISR_FIFO_Wp = 0;
				   					}
			   					}								
			   					else
			   					{//Fail.. clear the ir condition state
									IR_Control_State &= ~IR_START;		
									IR_Plus_Cnt = 0; //next is bit 1.		   						
			   					}
								
							}										
						}												 				 
					}
					IR_MS_Cnt = 0;
					P3_0 = 1;
				}	
				else						
#endif
				{
#if (KVM_BUTTON_SUPPORT) & (KVM_BUTTON_HARDWARE)
#ifdef BTN_RESET																	
					_MISC_CIR_SFR(BTN_PORT_SFR[0]);
					_MISC_DR_READ_SFR(regvalue0);
					_MISC_DR_READ_SFR(regvalue1);							
					if (regvalue0 & (BTN_TRIGGER_FALLING << BTN_POLLING_SHIFT[0]))
					{									
						regvalue0 &= ~(BTN_TRIGGER_MASK << BTN_POLLING_SHIFT[0]);				
						regvalue0  |= (BTN_TRIGGER_RISING << BTN_POLLING_SHIFT[0]);								
						ISR_FIFO[ISR_FIFO_Wp].State = 0;
					}	
					else
					{ //Rising						
						regvalue0 &= ~(BTN_TRIGGER_MASK << BTN_POLLING_SHIFT[0]);
						regvalue0  |= (BTN_TRIGGER_FALLING << BTN_POLLING_SHIFT[0]);															
						ISR_FIFO[ISR_FIFO_Wp].State = 1;
					}
					_MISC_DR_SFR(regvalue0);
					_MISC_DR_SFR(regvalue1);
					_MISC_CIR_SFR(BTN_PORT_SFR[0]);
#endif	// #ifdef BTN_RESET																					
#endif  // #if (KVM_BUTTON_SUPPORT) & (KVM_BUTTON_HARDWARE)					
					ISR_FIFO[ISR_FIFO_Wp].ISR_Type = ISR_BTN;
					ISR_FIFO[ISR_FIFO_Wp].Data = cpu_Gpio0IntrStatus;
					if (++ISR_FIFO_Wp >= ISR_FIFO_DEPTH)
					{
						ISR_FIFO_Wp = 0;
					}	
				}	
			}
		}
	}
#endif //#if (INTR_ISR_INCLUDE_GPIO0)

#if (INTR_ISR_INCLUDE_GPIO2)
	if (intrStt & GP2_INT_STU)
	{
		CPU_Gpio2IntrEntryFunc();
	}
#endif //#if (INTR_ISR_INCLUDE_GPIO2)

#if (INTR_ISR_INCLUDE_HSUR2)
	if (intrStt & UR2_INT_STU)
	{
		HSUR2_IntrEntryFunc();
	}
#endif	/* #if (INTR_ISR_INCLUDE_HSUR2) */
}

/*
 *--------------------------------------------------------------------------------
 * static void intr_PeripheralIntr5ISR(void)
 * Purpose : The interrupt service routine of MSTIMER and RTC.
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
static void intr_PeripheralIntr5ISR(void) interrupt INT5_VECTOR //use external interrupt pin 5 (0x53)
{
	idata U8_T	intr5Stt = 0;
#if (INTR_ISR_INCLUDE_SWDMA)
	idata U8_T	dmastate;
#endif

	EIF = INT5F;
	intr5Stt = SDSTSR;

#if (INTR_ISR_INCLUDE_SWDMA)
	if (intr5Stt & SDC)
	{
		DCIR = SW_DMA_STATUS_REG;
		dmastate = DDR;
		if (dmastate & (DMA_COMPLETE_STU |DMA_ERROR_STU))
		{
			if ((dmastate & (DMA_COMPLETE_STU |DMA_ERROR_STU)) == DMA_COMPLETE_STU)
			{
				USBHC_SWDMA_Inused_Flag = 0; //release the SW DAM channel
			}
		}
	}
#endif //#if (INTR_ISR_INCLUDE_SWDMA)

#if (INTR_ISR_INCLUDE_MSTIMER)
	if (intr5Stt & STT)
	{
		if (++SWTIMER_Counter == 0)
		{
			SWTIMER_ScCheckTimer256ms++;
		}
		Task_MS_Period_Flag = 1;
#if (SYSTEM_IR_DEVICE_SUPPORT)
		IR_MS_Cnt++;
#endif		
	}
#endif //#if (INTR_ISR_INCLUDE_MSTIMER)

#if (INTR_ISR_INCLUDE_BUZZER)
	if (intr5Stt & BZRI)
	{
		//read the buzzer interrupt status
		_MISC_CIR_SFR(MISC_BISR);
		_MISC_DR_READ_SFR(buzzer_instate);
	}
#endif //#if (INTR_ISR_INCLUDE_BUZZER)
}

/*
 *--------------------------------------------------------------------------------
 * static void intr_PmmWakeUpISR(void)
 * Purpose : The interrupt service routine of the
 *           wake up event in power management mode.
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
static void intr_PmmWakeUp_Intr6ISR(void) interrupt INT6_VECTOR //use external interrupt 6 (0x5B)
{
	U8_T	WakeStatus;

	EXTINT6_DISABLE;

	EIF = INT6F; // Clear the flag of interrupt 6.
	WakeStatus = PCON;

	if (!(WakeStatus & SWB_))
	{
		PCON &= ~ (PMM_ | STOP_);
	}
	WakeStatus = WKUPSR;
	
#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
	PowerSavingExitSate = WakeStatus;
	PowerSavingExitFlag = 1;
#endif //#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)

	while (WakeStatus)
	{
#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
		if (WakeStatus & GPIO_WKUP)
		{
			if (CPU_WakeUpEventTable & GPIO_WKUP)
				CPU_GpioWkupEntryFunc();
		}
		
		if (WakeStatus & SPI_WKUP)
		{
			if (CPU_WakeUpEventTable & SPI_WKUP)
			{
			}
		}

		if (WakeStatus & PS2_WKUP)
		{
			if (CPU_WakeUpEventTable & PS2_WKUP)
				PS2H_WkupEntryFunc();
		}
#endif //#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)

		/* check the EIF for INTF6 again */
		if (EIF & INT6F)
		{
			EIF = INT6F;	// Clear the flag of interrupt 6.
			WakeStatus = WKUPSR;
		}
		else
		{
			break;
		}
	}

	EXTINT6_ENABLE;
}

#if (INTR_ISR_INCLUDE_WATCHDOG)
/*
 *--------------------------------------------------------------------------------
 * void intr_WatchDogISR(void)
 * Purpose : The interrupt service routine of the Watchdog timeout.
 * Params  : None
 * Returns : None
 * Note    : None
 *--------------------------------------------------------------------------------
 */
void intr_WatchDogISR(void) interrupt WDIF_VECTOR
{
	if (EWDI)
	{
		if (EWT)
		{
			TA = 0xAA;
			TA = 0x55;
			WDIF = 0; // clear WatchDog Interrupt Flag.
			TA = 0xAA;
			TA = 0x55;
			WTRF = 0; // clear the Watchdog Timer Reset Flag.
		}
		else
		{
			TA = 0xAA;
			TA = 0x55;
			WDIF = 0; // clear WatchDog Interrupt Flag.
		}
	}
}
#endif
/* EXPORTED SUBPROGRAM BODIES */

/* End of interrupt.c */
