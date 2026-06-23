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
 * Module Name : cpu.h
 * Purpose     : A header file belongs to cpu module.
 *               It define all globe parameters in cpu module.
 * Author      : 
 * Date        :
 * Notes       : None.
 *================================================================================
 */
#ifndef CPU_H
#define CPU_H

/* INCLUDE FILE DECLARATIONS */
#include	"cpu_cfg.h"


/* NAMING CONSTANT DECLARATIONS */
#define		PROG_WTST_0			0
#define		PROG_WTST_1			1
#define		PROG_WTST_2			2
#define		PROG_WTST_3			3
#define		PROG_WTST_4			4
#define		PROG_WTST_5			5
#define		PROG_WTST_6			6
#define		PROG_WTST_7			7
#define		DATA_STRETCH_0		0
#define		DATA_STRETCH_1		1
#define		DATA_STRETCH_2		2
#define		DATA_STRETCH_3		3
#define		DATA_STRETCH_4		4
#define		DATA_STRETCH_5		5
#define		DATA_STRETCH_6		6
#define		DATA_STRETCH_7		7

#define		WD_INTR_ENABLE		1
#define		WD_INTR_DISABLE		0
#define		WD_RESET_ENABLE		1
#define		WD_RESET_DISABLE	0
#define		WD_HIGH_PRIORITY	1
#define		WD_LOW_PRIORITY		0
#define		WD_INTERVAL_131K	0x00
#define		WD_INTERVAL_1M		WD0_
#define		WD_INTERVAL_8M		WD1_
#define		WD_INTERVAL_67M		WD1_|WD0_

/* #define SFR_WakeUp Flag */
#define		PSV_WAKEUP_BY_EXT_PIN		BIT6 //Just for external pin wakeup for debug test

/* SFR definition for driver accessing */
#define SFR_P0				0x80// Port 0
#define SFR_SP				0x81// Stack Pointer
#define SFR_DPL0			0x82// Data Pointer 0 Low Byte
#define SFR_DPH0			0x83// Data Pointer 0 High Byte
#define SFR_DPL1			0x84// Data Pointer 1 Low Byte
#define SFR_DPH1			0x85// Data Pointer 1 High Byte
#define SFR_DPS				0x86// Data Pointers Select Register
#define SFR_PCON			0x87// Power Configuration Register
#define SFR_TCON			0x88// Timer 0,1 Configuration Register
#define SFR_TMOD			0x89// Timer 0,1 Control Mode Register
#define SFR_TL0				0x8A// Timer 0 Low Byte Counter
#define SFR_TL1				0x8B// Timer 1 Low Byte Counter
#define SFR_TH0				0x8C// Timer 0 High Byte Counter
#define SFR_TH1				0x8D// Timer 1 High Byte Counter
#define SFR_CKCON			0x8E// Clock Control Register
#define SFR_CSREPR			0x8F// Software Reset and External Program RAM Select Register

#define SFR_P1				0x90// Port 1
#define SFR_EIF				0x91// Extended Interrupt Flag Register
#define SFR_WTST			0x92// Program Wait States Register
#define SFR_DPX0			0x93// Data Pointer Extended Register 0
#define SFR_DPX1			0x95// Data Pointer Extended Register 1
#define SFR_I2CCIR			0x96// I2C Command Index Register
#define SFR_I2CDR			0x97// I2C Data Register
#define SFR_SCON0			0x98// UART 0 Configuration Register
#define SFR_SBUF0			0x99// UART 0 Buffer Register
#define SFR_EMDMR			0x9A
#define SFR_WKUPSR			0x9C// Wake-up Status Summary Register
#define SFR_ACON			0x9D// Address Control Register
#define SFR_PISSR			0x9E// Peripheral Interrupt Status Summary Register
#define SFR_UDCSR			0x9F// USB Device Controller Status Register

#define SFR_P2				0xA0// Port 2
#define SFR_DC0ISR			0xA3// USB Device Controller 0 Interrupt Status Register
#define SFR_DC0INSR			0xA4// USB Device Controller 0 Interrupt Index Status Register
#define SFR_DC0ESMR			0xA5// USB Device Controller 0 Endpoint Status Mirror Register
#define SFR_DC0CIR			0xA6// USB Device Controller 0 Command Index Register
#define SFR_DC0DR			0xA7// USB Device Controller 0 Data Register
#define SFR_IE				0xA8// Interrupt Enable register
#define SFR_DC1ISR			0xAB// USB Device Controller 1 Interrupt Status Register
#define SFR_DC1INSR			0xAC// USB Device Controller 1 Interrupt Index Status Register
#define SFR_DC1ESMR			0xAD// USB Device Controller 1 Endpoint Status Mirror Register
#define SFR_DC1CIR			0xAE// USB Device Controller 1 Command Index Register
#define SFR_DC1DR			0xAF// USB Device Controller 1 Data Register

#define SFR_P3				0xB0// Port 3
#define SFR_DC2ISR			0xB3// USB Device Controller 2 Interrupt Status Register
#define SFR_DC2INSR			0xB4// USB Device Controller 2 Interrupt Index Status Register
#define SFR_DC2ESMR			0xB5// USB Device Controller 2 Endpoint Status Mirror Register
#define SFR_DC2CIR			0xB6// USB Device Controller 2 Command Index Register
#define SFR_DC2DR			0xB7// USB Device Controller 2 Data Register
#define SFR_IP				0xB8// Interrupt SWDMA Register
#define SFR_DC3ISR			0xBB// USB Device Controller 3 Interrupt Status Register
#define SFR_DC3INSR			0xBC// USB Device Controller 3 Interrupt Index Status Register
#define SFR_DC3ESMR			0xBD// USB Device Controller 3 Endpoint Status Mirror Register
#define SFR_DC3CIR			0xBE// USB Device Controller 3 Command Index Register
#define SFR_DC3DR			0xBF// USB Device Controller 3 Data Register

#define SFR_SCON1			0xC0// UART 1 Configuration Register
#define SFR_SBUF1			0xC1// UART 1 Buffer Register
#define SFR_HCIS			0xC4// Host Controller Operation Interrupt Status Register
#define SFR_HCOIS			0xC5// Host Controller Operation Interrupt Status Register
#define SFR_HCCIR			0xC6// USB Host Controller Command Index Register
#define SFR_HCDR			0xC7// USB Host Controller Data Register
#define SFR_T2CON			0xC8// Timer 2 Configuration Register
#define SFR_T2IF			0xC9// Timer 2 Interrupt Flags
#define SFR_RLDL			0xCA// Timer 2 Reload Low Byte
#define SFR_RLDH			0xCB// Timer 2 Reload High Byte
#define SFR_TL2				0xCC// Timer 2 Low Byte Counter
#define SFR_TH2				0xCD// Timer 2 High Byte Counter

#define SFR_PSW				0xD0// Program Status Word
#define SFR_FCIR			0xD1// Flash Command Index Register
#define SFR_FDR				0xD2// Flash Data Register
#define SFR_FCISR			0xD3// Flash Controller Interrupt Status Register
#define SFR_FCDP			0xD4// Flash Controller Data Port
#define SFR_PCKEN			0xD5// Peripheral Clock Enable Register
#define SFR_WDCON			0xD8// Watchdog Configuration
#define SFR_SDSTSR			0xD9// Software DMA and Software Timer complete interrupt status
#define SFR_DCIR			0xDA// DMA Command Index Register
#define SFR_DDR				0xDB// DMA Data Register
#define SFR_PS2CIR			0xDC// PS2 Command Index Register
#define SFR_PS2DR			0xDD// PS2 Data Register
#define SFR_MR				0xDE// Miscellaneous Register
#define SFR_CRR				0xDF// Chip Revision Register

#define SFR_ACC				0xE0// Accumulator
#define SFR_MCIR			0xE2// Miscellaneous Command Index Register
#define SFR_MDR				0xE3// Miscellaneous Data Register
#define SFR_EIE				0xE8// External Interrupt Enable
#define SFR_STATUS			0xE9// Status Register
#define SFR_MXAX			0xEA// MOVX @Ri Extended Register
#define SFR_TA				0xEB// Timed Access Protection
#define SFR_SPICIR			0xEC// SPI Command Index Register
#define SFR_SPIDR			0xED// SPI Data Register
#define SFR_B				0xF0// B Working Register
#define SFR_EIP				0xF8// Extended Interrupt SWDMA Register

/* Miscellaneous Register Map */
#define MISC_BCR			0x00
#define MISC_BISR			0x01
#define MISC_GP0DER			0x10
#define MISC_GP0TR			0x11
#define MISC_GP0ISR			0x13
#define MISC_GPWER			0x14
#define MISC_GPWSR			0x16
#define MISC_GP2DER			0x20
#define MISC_GP2TR			0x21
#define MISC_GP2ISR			0x23
#define MISC_C12MSTBR		0x80

/* BUTTON Trigger Register define */
#define  BTN_TRIGGER_MASK			0x03
#define  BTN_TRIGGER_DISABLE		0x00
#define  BTN_TRIGGER_FALLING		0x01
#define  BTN_TRIGGER_RISING			0x02
#define  BTN_TRIGGER_EDGE_CHANGED	0x03


/* MACRO DECLARATIONS */
/* wakeup events that want to be set to parameter CPU_WakeUpEventTable */
#define CPU_SetWakeUpEventTable(flag)		(CPU_WakeUpEventTable |= flag)
/* wakeup events that want to be clear from parameter CPU_WakeUpEventTable */
#define CPU_ClrWakeUpEventTable(flag)		(CPU_WakeUpEventTable &= ~flag)
/* to check the power saving is PMM mode or not */
#define CPU_GetPowerSavingMode()			(CPU_PowerSavingMode)
/* to get system clock */
#define CPU_GetSysClk()						(CPU_SysClk)
/* to get the program wait state */
#define CPU_GetProgWst()					(CPU_ProgWtst)
/* software reset to reset CPU core */
#define CPU_SoftReset()					{	\
	if (EA)									\
		CPU_GlobeInt = BIT0 | BIT7;			\
	else									\
		CPU_GlobeInt = BIT7;				\
	EA_GLOBE_DISABLE;						\
	CSREPR |= SW_RST;						}
/* software reboot to restart hardware core of all */
#define CPU_SoftReboot()					(CSREPR |= SW_RBT)


/* TYPE DECLARATIONS */


/* GLOBAL VARIABLES */
extern U8_T		CPU_WakeUpEventTable;
extern U8_T		CPU_PowerSavingMode;
extern U8_T		CPU_SysClk;
extern U8_T		CPU_ProgWtst;
extern U8_T		CPU_GlobeInt;
extern U8_T		cpu_Gpio0IntrStatus;
extern U8_T		cpu_Gpio2IntrStatus;
extern U8_T IDATA FirmwareUpgradeFlag[3];

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void	CPU_Init(void);
void	CPU_SetPowerSaveMode(U8_T mode);
#if (CPU_MODULE_INCLUDE_WATCHDOG)
void	CPU_WatchDogSetting(U8_T wdIntrEnb, U8_T wdRstEnb, U8_T wdPrty, U8_T wdTime);
#endif
void	CPU_SetPowerSaveMode(U8_T mode);
void	CPU_MscRegRead(U8_T regIndex, U8_T *pRegData, U8_T regLen);
void	CPU_Gpio2IntrEntryFunc(void);
void	CPU_GpioWkupEntryFunc(void);
void	PS2H_WkupEntryFunc(void);
#endif /* End of CPU_H */
