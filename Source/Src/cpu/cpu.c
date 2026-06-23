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
 * Module Name : cpu.c
 * Purpose     : AX12000 initialization and globe values setting.
 * Author      : 
 * Date        : 
 * Notes       : In the initial function, all external interrupt are disable in
 *               default. User must enable a specific interrupt in its module.
 *               Several system values are initial in beginning
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	"project_include.h"
#include	<absacc.h>
#include	<stdio.h>


/* GLOBAL VARIABLES DECLARATIONS */
U8_T	CPU_WakeUpEventTable = 0;
U8_T	CPU_PowerSavingMode = 0;
U8_T	CPU_SysClk = 0;
U8_T	CPU_ProgWtst = 0;
U8_T	CPU_GlobeInt = 0;
U8_T	CPU_ReInitPhyFlag = 0;

U8_T IDATA FirmwareUpgradeFlag[3] _at_ 0x31;
/* STATIC VARIABLE DECLARATIONS */
static U8_T DATA	cpu_ReservedForStackPoint[2] _at_ 0x0D;
static U8_T IDATA	cpu_ExecuteRuntimeFlag _at_ 0x30;
static U16_T		cpu_GpioWakeupStatus;
static U8_T			PS2_WakeupStatus;
U8_T	cpu_Gpio0IntrStatus;
U8_T	cpu_Gpio2IntrStatus;

static bit cpu_isr;
/* LOCAL SUBPROGRAM DECLARATIONS */
/* LOCAL SUBPROGRAM BODIES */
/* EXPORTED SUBPROGRAM BODIES */
/*
 *--------------------------------------------------------------------------------
 * void CPU_Init(void)
 * Purpose : This function initializes CPU Core for the whole system operation.
 *           AX120xx supports 2 clock rates of 25MHz and 100MHz.
 *           The Program Wait State and Data Stretch Cycle are configured via
 *           different clock rates and the Memory Shadow Mode as below table,
 *           the range of both the Program Wait State and the Data Stretch Cycle are 0 ~7.
 *           All external interrupts are disabled during system initialization.
 * Params  : None
 * Returns : None
 * Note    : None
 *--------------------------------------------------------------------------------
 */
void CPU_Init(void)
{
	EA_GLOBE_DISABLE; // Turn off globe interrupt
	
	/*What is the function for RuntimeCode? */
#ifdef RuntimeCodeAt12KH
	cpu_ExecuteRuntimeFlag = 1;
#else
	cpu_ExecuteRuntimeFlag = 0;
#endif //#if (RuntimeCodeAt12KH)

	PCKEN = 0x00; // Turn off peripheral interface clock.

	P0 = 0xFF;
	P1 = 0xFF;
	P2 = 0xFF;
	P3 = 0xFF;

#if (MCU_REAL_CHIP)
	switch (CSREPR & SCS_96M)
	{
		case SCS_48M :
			CPU_SysClk = SCS_48M;
			WTST = 0x02;
			CKCON = 0x00;
			FDR = 0x00;
			FCIR = FLH_FWATR;
			MDR = 0x00;
			MDR = 0x5D;
			MDR = 0x00;
			MCIR = MISC_C12MSTBR;
			break;
		case SCS_96M :
		default :
			CPU_SysClk = SCS_96M;
			WTST = 0x03;
			CKCON = 0x00;
			FDR = 0x00;
			FCIR = FLH_FWATR;
			MDR = 0x00;
			MDR = 0xBB;
			MDR = 0x00;
			MCIR = MISC_C12MSTBR;
			break;
	}
#else // FPGA
	switch (CSREPR & SCS_96M)
	{
		case SCS_96M:
			CPU_SysClk = SCS_96M;
			WTST = 0x06;
			CKCON = 0x00;
			FDR = 0x03;
			FCIR = FLH_FWATR;
			MDR = 0x00;
			MDR = 0xBB;
			MDR = 0x00;
			MCIR = MISC_C12MSTBR;
			break;
		case SCS_48M:
		default :
			CPU_SysClk = SCS_48M;
			WTST = 0x06;
			CKCON = 0x00;
			FDR = 0x03;
			FCIR = FLH_FWATR;
			MDR = 0x03;
			MDR = 0x00;
			MDR = 0x00;
			MCIR = MISC_C12MSTBR;
			break;
	}
#endif

	CPU_ProgWtst = WTST;

	/* Clear the INT5F & INT6F of Interrupt5 & interrupt6 (write 1 to clear) */
	EIF = INT5F;	// Clear the flag of interrupt 5.
	EIF = INT6F;	// Clear the flag of interrupt 6.

	/* Clear Watch Dog Time Out Reset Flag */
	if (WTRF)
	{
		TA = 0xAA;
		TA = 0x55;
		WTRF = 0;	// Clear Watch Dog Time Out Reset Flag.
		CSREPR |= SW_RBT;
	}

	EXTINT6_ENABLE; // EINT6 interrupt for wake-up of power management mode and stop mode.
	
	/* Turn on the peripheral interface module clock */
#ifdef MCU_TYPE_AX68002
	PCKEN = 0xFC;
#else
	PCKEN = 0xFF;
#endif

	cpu_Gpio0IntrStatus = 0;

	EA_GLOBE_ENABLE;// Enable the globe interrupt.
}

#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
/*
 *--------------------------------------------------------------------------------
 * Function Name: CPU_SetPowerSaveMode()
 * Purpose : Setup the system power saving mode.
 * Params  : mode - 0:Disable,  2:STOP mode,  3:STOP with TOFF_12M
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void CPU_SetPowerSaveMode(U8_T mode)
{
	cpu_isr = EA;
	EA = 0;
	PCON &= ~(PMM_ | STOP_ | SWB_ | PMMS_);
	if (mode == 2)
	{
		CPU_PowerSavingMode = STOP_;
	}
	else if (mode == 3)
	{
		CSREPR |= TOFF_12M;
		CPU_PowerSavingMode = STOP_;
	}
	else
	{
		CPU_PowerSavingMode = 0;
		EA = cpu_isr;
		return;
	}
	
	EA = cpu_isr;
	PCON |= CPU_PowerSavingMode;
	
} /* End of CPU_SetPowerSaveMode() */
#endif /* end of SYSTEM_POWER_SAVING_MODE_SUPPORT */

/*
 *--------------------------------------------------------------------------------
 * void CPU_MscRegRead(U8_T regIndex, U8_T *pRegData, U8_T regLen)
 * Purpose: Read data bytes from a register with interrupt locked.
 * Params : regIndex:An index address to register.
 *          pRegData:A pointer to indicate the register data.
 *          regLen:A number of bytes to indicate how many bytes will be read.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */  
void CPU_MscRegRead(U8_T regIndex, U8_T *pRegData, U8_T regLen)
{
	cpu_isr = EA;
	EA = 0;
	MCIR = regIndex;
	while (regLen --)
	{
		*(pRegData + regLen) = MDR;
	}
	EA = cpu_isr;
}

#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
/*
 *--------------------------------------------------------------------------------
 * void CPU_GpioWkupEntryFunc(void)
 * Purpose :
 * Params  :
 * Returns : 
 * Note    : 
 *--------------------------------------------------------------------------------
 */
void CPU_GpioWkupEntryFunc(void)
{
	CPU_MscRegRead(MISC_GPWSR, (U8_T *)&cpu_GpioWakeupStatus, 2);
}


/*
 *--------------------------------------------------------------------------------
 * void PS2H_WkupEntryFunc(void)
 * Purpose : Initialize the PS2A and PS2B both. Enable interrupt mode and wakeup
 *           function. Use the default value for other time relative registers.
 * Params  : 
 * Returns : 
 * Note    :
 *--------------------------------------------------------------------------------
 */
void PS2H_WkupEntryFunc(void)
{
#if (SYSTEM_PS2_HOST_ENABLE)
	PS2H_Read_Regs(PS2_WSR, &PS2_WakeupStatus, 1);
#endif /* #if (SYSTEM_PS2_HOST_ENABLE) */
}

#endif /* end of SYSTEM_POWER_SAVING_MODE_SUPPORT */

/* End of cpu.c */
