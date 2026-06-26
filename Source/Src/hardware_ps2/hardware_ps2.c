/*
******************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*================================================================================
 * Module Name : hardware_ps2.c
 * Purpose     : Use External Interrupt to access and control PS/2 device 
 * Author      : Jack Wang
 * Date        :
 * Notes       : None
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	<stdio.h>
#include	<string.h>
#include	"project_include.h"

#if (SYSTEM_HARDWARE_PS2_ENABLE)
/* GLOBAL DEFINATION DECLARATIONS */
/* GLOBAL CONTANT DECLARATIONS */
const U8_T HW_PS2_CR[]= {PS2A_CR, PS2B_CR};
const U8_T HW_PS2_DR[]= {PS2A_DR, PS2B_DR};

/* GLOBAL VARIABLES DECLARATIONS */
U8_T HPS2_CR_BK[SYSTEM_HARDWARE_PS2_PORT_MAX];
U8_T TASK_HW_PS2_Clear_Disable_ID;

/* STATIC VARIABLE DECLARATIONS */
/* LOCAL SUBPROGRAM DECLARATIONS */
void HW_PS2_ISR_Handle(void);
void TASK_HW_PS2_Clear_Disable(void);

/*
 * ----------------------------------------------------------------------------
 * void HW_PS2_Init(void)
 * Purpose : Initializes the PS/2 relative Hardware interrupt peripheral and GPIO
 * Params  : none
 * Returns : none
 * Note    : 
 * ----------------------------------------------------------------------------
 */
void HW_PS2_Init(void)
{
	HPS2_ISR_FIFO_WP = 0;
	HPS2_ISR_FIFO_RP = 0;
	
	/*1.Enable+Enable Interrupt */
#if (PS2_WAKEUP_SUPPORT)
	HPS2_CR_BK[0] = PS2CR_HOST_ENB | PS2CR_RX_IE | PS2CR_TX_IE | PS2CR_WE;
#else
	HPS2_CR_BK[0] = PS2CR_HOST_ENB | PS2CR_RX_IE | PS2CR_TX_IE;
#endif
#if (SYSTEM_HARDWARE_PS2_PORT_MAX >= 1)
	HPS2_CR_BK[1] = HPS2_CR_BK[0];
#endif

	/*2.Setup TINH Time - 200uS */
	EXTINT4_DISABLE;
	_PS2_DR_SFR(0x15);
	_PS2_CIR_SFR(PS2_DIVR);
	EXTINT4_ENABLE;

	/*3.Tranfer Time Keep Defaul Value-2880us */
	/*4.Response Clock Timeout Keep Defaul Value-24960us */
	/*5.WRITE int the PS2_CR for PS2A, PS2B */
	EXTINT4_DISABLE;
	_PS2_DR_SFR(HPS2_CR_BK[0]);
	_PS2_CIR_SFR(PS2A_CR);
	EXTINT4_ENABLE;

#if (SYSTEM_HARDWARE_PS2_PORT_MAX >= 1)
	EXTINT4_DISABLE;
	_PS2_DR_SFR(HPS2_CR_BK[1]);
	_PS2_CIR_SFR(PS2B_CR);
	EXTINT4_ENABLE;
#endif

	TASK_HW_PS2_Clear_Disable_ID = TASK_Create(TASK_HW_PS2_Clear_Disable);
	
#if (SYSTEM_TASK_DUMP_SUPPORT)
	//printf("TASK_HW_PS2_Clear_Disable_ID=%bu\n\r",TASK_HW_PS2_Clear_Disable_ID);
#endif	
} /* End of HW_PS2_Init */

/*
 * ----------------------------------------------------------------------------
 * void HW_PS2_Disable
 * Purpose : Set the PS2 port to disable condition: EXTIx interupt disable, the 
 *           clock is low
 * Params  : PS2x: Select the PS2 port peripheral. 
 *           this parameter can be one of the following values:
 *           EPS2A, EPS2B
 * Returns : None
 * Note    :  
 * ----------------------------------------------------------------------------
 */
//void HW_PS2_Disable(PS2Port_TypeDef PS2x)
//{
	/*Enable the inhibit bit,pull the clock low */
//	HPS2_CR_BK[PS2x] |= PS2CR_INH_FORCE;
//	EXTINT4_DISABLE;
//	_PS2_DR_SFR(HPS2_CR_BK[PS2x]);
//	_PS2_CIR_SFR(HW_PS2_CR[PS2x]);
//	EXTINT4_ENABLE;
//} /* End of EXTI_PS2_Disable */

/*
 * ----------------------------------------------------------------------------
 * void HW_PS2_Enable
 * Purpose : Set the PS2 port to disable condition: EXTIx interupt disable, the 
 *           clock is low
 * Params  : PS2x: Select the PS2 port peripheral. 
 *           this parameter can be one of the following values:
 *           EPS2A, EPS2B
 * Returns : None
 * Note    :  
 * ----------------------------------------------------------------------------
 */
void HW_PS2_Enable(PS2Port_TypeDef PS2x)
{
	/*Disable the inhibit bit,pull the clock low */
	HPS2_CR_BK[PS2x] &= ~PS2CR_INH_FORCE;

	EXTINT4_DISABLE;
	_PS2_DR_SFR(HPS2_CR_BK[PS2x]);
	_PS2_CIR_SFR(HW_PS2_CR[PS2x]);
	EXTINT4_ENABLE;
} /* End of EXTI_PS2_Pin_Control */

/*
 * ----------------------------------------------------------------------------
 * void PS2H_Ps2aIntrEntryFunc(void)
 * Purpose : Hardware PS2A interrupt service routine.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void HW_PS2_Transmit(PS2Port_TypeDef PS2x,U8_T ps2data)
{
	//write data out
	HPS2_CR_BK[PS2x] |= PS2CR_TX_ACT;

	//1.write Data into data register
	EXTINT4_DISABLE;
	_PS2_DR_SFR(ps2data);
	_PS2_CIR_SFR(HW_PS2_DR[PS2x]);
	EXTINT4_ENABLE;

	//2.write TX Active into
	EXTINT4_DISABLE;
	_PS2_DR_SFR(HPS2_CR_BK[PS2x]);
	_PS2_CIR_SFR(HW_PS2_CR[PS2x]);
	EXTINT4_ENABLE;

	HPS2_CR_BK[PS2x] &= ~PS2CR_TX_ACT; // Clear the TX_ACTIVE
} /* End of EXTI0_ISR */

/*
 * ----------------------------------------------------------------------------
 * void HW_PS2_ISR_Handle(void)
 * Purpose : Initializes the PS/2 relative Hardware interrupt peripheral and GPIO
 * Params  : none
 * Returns : none
 * Note    : 
 * ----------------------------------------------------------------------------
 */
void HW_PS2_ISR_Handle(void)
{
	idata U8_T state,pid,ps2data;
	bit   enable_inh_force=0; 
	
	/*-----------------------------------------------------
	// Judge the Clock must low, another is false trigger
	-------------------------------------------------------*/
	if (HPS2_ISR_FIFO_WP == HPS2_ISR_FIFO_RP)
		return;

	pid      = HPS2_ISR_FIFO[HPS2_ISR_FIFO_RP].PID;
	state    = HPS2_ISR_FIFO[HPS2_ISR_FIFO_RP].State;
	ps2data  = HPS2_ISR_FIFO[HPS2_ISR_FIFO_RP].Data;
	HPS2_ISR_FIFO_RP++;
	if (HPS2_ISR_FIFO_RP >= HPS2_ISR_FIFO_DEPTH_MAX)
		HPS2_ISR_FIFO_RP = 0;

	if ((PS2_Status[pid] & PS2_WRITE_FLAG) == 0) // PS/2 receive mode
	{
		// 0.Check Complete codition/Parrity Error
		state = state & PS2SR_RX_STATE_MASK;
		if ((state  == PS2SR_RX_COMPLETE) || (state & PS2SR_RX_PARITY_ERR))
		{
			if ((state & PS2SR_RX_FORMAT_ERR) != PS2SR_RX_FORMAT_ERR)
			{
				PS2_FIFO_Push(pid,ps2data);
				if (state & PS2SR_RX_PARITY_ERR)
				{
					PS2_Status[pid] |= PS2_PARITY_ERR_FLAG; //Parity Error
				}
			}
			PS2_Status[pid] |= PS2_IDLE; // PS/2 Port active, may be is false alarm
			PS2_Status[pid] &= ~PS2_ACT; // reset active flag
		}
		// 1.Check Timeout codition
		if (state & PS2SR_RX_STATE_MASK)
		{
#if (PS2_DEBUG_MODE == 1)
			printf("@P:%d,Receive TimeOut!!\n\r",(U16_T)pid);
#endif
			PS2_Status[pid] |= PS2_TIME_OUT_FLAG;
		}
		if ((PS2_Status[pid] & PS2_SCRIPT_START_FLAG) == 0) // in mornal case
		{
			if (PS2_Control[pid] == PS2_KB)
			{
				enable_inh_force = 1;
				TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_HW_PS2_Clear_Disable_ID,0,pid,1,2); //after 1000ms check overcurrent again
			}
		}
		if (enable_inh_force==0)
		{
			HPS2_CR_BK[pid] &= ~PS2CR_INH_FORCE;
			EXTINT4_DISABLE;  
			_PS2_DR_SFR(HPS2_CR_BK[pid]);
			_PS2_CIR_SFR(HW_PS2_CR[pid]);
			EXTINT4_ENABLE;  
		}
	}
	else // PS/2 write mode
	{
		// 0.Check Complete codition
		state = state & PS2SR_TX_STATE_MASK;
		if ((state & PS2SR_TX_STATE_MASK) == PS2SR_TX_COMPLETE)
		{
			PS2_Status[pid] &= ~(PS2_WRITE_FLAG|PS2_ACT); // PS/2 receive mode
			PS2_Status[pid] |=  PS2_TRANSMIT_COMPLETE_FLAG; // data write complete
		}
		// 1.Check Timeout codition
		if (state & (PS2SR_TX_TIME_OUT|PS2SR_RSP_TIME_OUT))
		{
#if (PS2_DEBUG_MODE == 1)
			printf("@P:%d,Transmitte TimeOut!!\n\r",(U16_T)pid);
#endif
			PS2_Status[pid] |= PS2_TIME_OUT_FLAG;
		}
	}
	TASK_Active(TASK_TYPE_EVENT,TASK_EVENT_PS2_HANDLE_ID,0x00,pid,0,0); // now aticev the process for time out event
} /* End of EXTI0_ISR */

void TASK_HW_PS2_Clear_Disable(void)
{
	U8_T pid;

	pid = TASK_Register0;
	HPS2_CR_BK[pid] &= ~PS2CR_INH_FORCE;
	EXTINT4_DISABLE;
	_PS2_DR_SFR(HPS2_CR_BK[pid]);
	_PS2_CIR_SFR(HW_PS2_CR[pid]);
	EXTINT4_ENABLE;

	TASK_Destory_Current();
}

#endif /* End of SYSTEM_HARDWARE_PS2_ENABLE */

/* End of hardware_ps2.c */
