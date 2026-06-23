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
 * Module Name : ax6800x.h
 * Purpose     : A header file belongs to AX6800x Special Function Register (SFR).
 *               It also makes bit definition of SFR.
 * Author      : 
 * Date        : 
 * Notes       : None.
 *================================================================================
 */
/*------------------------------------------------
Make sure that this file gets included only once.
------------------------------------------------*/
#ifndef AX6800X_HEADER_FILE
#define AX6800X_HEADER_FILE


/*------------------------------------------------
Byte Registers
------------------------------------------------*/
sfr P0				= 0x80;// Port 0
sfr SP				= 0x81;// Stack Pointer
sfr DPL0			= 0x82;// Data Pointer 0 Low Byte
sfr DPH0			= 0x83;// Data Pointer 0 High Byte
sfr DPL1			= 0x84;// Data Pointer 1 Low Byte
sfr DPH1			= 0x85;// Data Pointer 1 High Byte
sfr DPS				= 0x86;// Data Pointers Select Register
sfr PCON			= 0x87;// Power Configuration Register
sfr TCON			= 0x88;// Timer 0,1 Configuration Register
sfr TMOD			= 0x89;// Timer 0,1 Control Mode Register
sfr TL0				= 0x8A;// Timer 0 Low Byte Counter
sfr TL1				= 0x8B;// Timer 1 Low Byte Counter
sfr TH0				= 0x8C;// Timer 0 High Byte Counter
sfr TH1				= 0x8D;// Timer 1 High Byte Counter
sfr CKCON			= 0x8E;// Clock Control Register
sfr CSREPR			= 0x8F;// Software Reset and External Program RAM Select Register

sfr P1				= 0x90;// Port 1
sfr EIF				= 0x91;// Extended Interrupt Flag Register
sfr WTST			= 0x92;// Program Wait States Register
sfr DPX0			= 0x93;// Data Pointer Extended Register 0
sfr DPX				= 0x93;// Data Pointer Extended Register 0
sfr DPX1			= 0x95;// Data Pointer Extended Register 1
sfr I2CCIR			= 0x96;// I2C Command Index Register
sfr I2CDR			= 0x97;// I2C Data Register
sfr SCON0			= 0x98;// UART 0 Configuration Register
sfr SBUF0			= 0x99;// UART 0 Buffer Register
sfr EMDMR			= 0x9A;// External Memory DMA Mask Register
sfr WKUPSR			= 0x9C;// Wake-up Status Summary Register
sfr ACON			= 0x9D;// Address Control Register
sfr PISSR			= 0x9E;// Peripheral Interrupt Status Summary Register
sfr UDCSR			= 0x9F;// USB Device Controller Status Register

sfr P2				= 0xA0;// Port 2
sfr DC0ISR			= 0xA3;// USB Device Controller 0 Interrupt Status Register
sfr DC0INSR			= 0xA4;// USB Device Controller 0 Interrupt Index Status Register
sfr DC0ESMR			= 0xA5;// USB Device Controller 0 Endpoint Status Mirror Register
sfr DC0CIR			= 0xA6;// USB Device Controller 0 Command Index Register
sfr DC0DR			= 0xA7;// USB Device Controller 0 Data Register
sfr IE				= 0xA8;// Interrupt Enable register
sfr DC1ISR			= 0xAB;// USB Device Controller 1 Interrupt Status Register
sfr DC1INSR			= 0xAC;// USB Device Controller 1 Interrupt Index Status Register
sfr DC1ESMR			= 0xAD;// USB Device Controller 1 Endpoint Status Mirror Register
sfr DC1CIR			= 0xAE;// USB Device Controller 1 Command Index Register
sfr DC1DR			= 0xAF;// USB Device Controller 1 Data Register

sfr P3				= 0xB0;// Port 3
sfr DC2ISR			= 0xB3;// USB Device Controller 2 Interrupt Status Register
sfr DC2INSR			= 0xB4;// USB Device Controller 2 Interrupt Index Status Register
sfr DC2ESMR			= 0xB5;// USB Device Controller 2 Endpoint Status Mirror Register
sfr DC2CIR			= 0xB6;// USB Device Controller 2 Command Index Register
sfr DC2DR			= 0xB7;// USB Device Controller 2 Data Register
sfr IP				= 0xB8;// Interrupt SWDMA Register
sfr DC3ISR			= 0xBB;// USB Device Controller 3 Interrupt Status Register
sfr DC3INSR			= 0xBC;// USB Device Controller 3 Interrupt Index Status Register
sfr DC3ESMR			= 0xBD;// USB Device Controller 3 Endpoint Status Mirror Register
sfr DC3CIR			= 0xBE;// USB Device Controller 3 Command Index Register
sfr DC3DR			= 0xBF;// USB Device Controller 3 Data Register

sfr SCON1			= 0xC0;// UART 1 Configuration Register
sfr SBUF1			= 0xC1;// UART 1 Buffer Register
sfr HCIS			= 0xC4;// Host Controller Interrupt Status Register
sfr HCOIS			= 0xC5;// Host Controller Operation Interrupt Status Register
sfr HCCIR			= 0xC6;// USB Host Controller Command Index Register
sfr HCDR			= 0xC7;// USB Host Controller Data Register
sfr T2CON			= 0xC8;// Timer 2 Configuration Register
sfr T2IF			= 0xC9;// Timer 2 Interrupt Flags
sfr RLDL			= 0xCA;// Timer 2 Reload Low Byte
sfr RLDH			= 0xCB;// Timer 2 Reload High Byte
sfr TL2				= 0xCC;// Timer 2 Low Byte Counter
sfr TH2				= 0xCD;// Timer 2 High Byte Counter

sfr PSW				= 0xD0;// Program Status Word
sfr FCIR			= 0xD1;// Flash Command Index Register
sfr FDR				= 0xD2;// Flash Data Register
sfr FCISR			= 0xD3;// Flash Controller Interrupt Status Register
sfr FCDP			= 0xD4;// Flash Controller Data Port
sfr PCKEN			= 0xD5;// Peripheral Clock Enable Register
sfr WDCON			= 0xD8;// Watchdog Configuration
sfr SDSTSR			= 0xD9;// Software DMA and Software Timer complete interrupt status
sfr DCIR			= 0xDA;// DMA Command Index Register
sfr DDR				= 0xDB;// DMA Data Register
sfr PS2CIR			= 0xDC;// PS2 Command Index Register
sfr PS2DR			= 0xDD;// PS2 Data Register
sfr CRR				= 0xDF;// Chip Revision Register

sfr ACC				= 0xE0;// Accumulator
sfr MCIR			= 0xE2;// Miscellaneous Command Index Register
sfr MDR				= 0xE3;// Miscellaneous Data Register
sfr UR2CIR			= 0xE4;// UART2 Command Index Register
sfr UR2DR			= 0xE5;// UART2 Data Register
sfr EIE				= 0xE8;// External Interrupt Enable
sfr STATUS			= 0xE9;// Status Register
sfr MXAX			= 0xEA;// MOVX @Ri Extended Register
sfr TA				= 0xEB;// Timed Access Protection
sfr SPICIR			= 0xEC;// SPI Command Index Register
sfr SPIDR			= 0xED;// SPI Data Register

sfr B				= 0xF0;// B Working Register
sfr EIP				= 0xF8;// Extended Interrupt SWDMA Register

/* Reserved for SW used */
sfr SFR_94			= 0x94;
sfr SFR_9B			= 0x9B;
sfr SFR_A1			= 0xA1;
sfr SFR_A2			= 0xA2;
sfr SFR_A9			= 0xA9;
sfr SFR_AA			= 0xAA;
sfr SFR_B1			= 0xB1;
sfr SFR_B2			= 0xB2;
sfr SFR_B9			= 0xB9;
sfr SFR_BA			= 0xBA;
sfr SFR_C2			= 0xC2;
sfr SFR_C3			= 0xC3;
sfr SFR_CE			= 0xCE;
sfr SFR_CF			= 0xCF;
sfr SFR_D6			= 0xD6;
sfr SFR_D7			= 0xD7;
sfr SFR_DE			= 0xDE;
sfr SFR_E1			= 0xE1;
sfr SFR_E4			= 0xE4;
sfr SFR_E5			= 0xE5;
sfr SFR_E6			= 0xE6;
sfr SFR_E7			= 0xE7;
sfr SFR_EE			= 0xEE;
sfr SFR_EF			= 0xEF;
sfr SFR_F1			= 0xF1;
sfr SFR_F2			= 0xF2;
sfr SFR_F3			= 0xF3;
sfr SFR_F4			= 0xF4;
sfr SFR_F5			= 0xF5;
sfr SFR_F6			= 0xF6;
sfr SFR_F7			= 0xF7;
sfr SFR_F9			= 0xF9;
sfr SFR_FA			= 0xFA;
sfr SFR_FB			= 0xFB;
sfr SFR_FC			= 0xFC;
sfr SFR_FD			= 0xFD;
sfr SFR_FE			= 0xFE;
sfr SFR_FF			= 0xFF;

/*------------------------------------------------
P0 (0x80) Bit Registers
------------------------------------------------*/
sbit P0_0			= 0x80;
sbit P0_1			= 0x81;
sbit P0_2			= 0x82;
sbit P0_3			= 0x83;
sbit P0_4			= 0x84;
sbit P0_5			= 0x85;
sbit P0_6			= 0x86;
sbit P0_7			= 0x87;

/*------------------------------------------------
DPS (0x86) Bit Values
------------------------------------------------*/
#define SEL_		0x01
#define TSL_		0x20
#define ID0_		0x40
#define ID1_		0x80

/*------------------------------------------------
PCON (0x87) Bit Values
------------------------------------------------*/
#define PMM_		0x01
#define STOP_		0x02
#define SWB_		0x04
#define PMMS_		0x08
#define PWE_		0x10
#define	SMOD1_		0x40
#define SMOD0_		0x80


/*------------------------------------------------
TCON (0x88) Bit Registers
------------------------------------------------*/
sbit IT0			= 0x88;
sbit IE0			= 0x89;
sbit IT1			= 0x8A;
sbit IE1			= 0x8B;
sbit TR0			= 0x8C;
sbit TF0			= 0x8D;
sbit TR1			= 0x8E;
sbit TF1			= 0x8F;

/*------------------------------------------------
TMOD (0x89) Bit Values
------------------------------------------------*/
#define	T0_M0_		0x01
#define	T0_M1_		0x02
#define	T0_CT_		0x04
#define	T0_GATE_	0x08
#define	T1_M0_		0x10
#define	T1_M1_		0x20
#define	T1_CT_		0x40
#define	T1_GATE_	0x80

/*------------------------------------------------
CKCON (0x8E) Bit Values
------------------------------------------------*/
#define MD0_		0x01
#define MD1_		0x02
#define MD2_		0x04
#define	T0M_		0x08
#define	T1M_		0x10
#define	T2M_		0x20
#define WD0_		0x40
#define WD1_		0x80

/*------------------------------------------------
CSREPR (0x8F) Bit Values
------------------------------------------------*/
#define	SW_RST		0x01
#define	SW_RBT		0x02
#define RST_OUT		0x04
#define TOFF_12M	0x08
#define DEBUG_ENB	0x40
#define	SCS_96M		0x80
#define	SCS_48M		0x00

/*------------------------------------------------
P1 (0x90) Bit Registers
------------------------------------------------*/
sbit P1_0			= 0x90;
sbit P1_1			= 0x91;
sbit P1_2			= 0x92;
sbit P1_3			= 0x93;
sbit P1_4			= 0x94;
sbit P1_5			= 0x95;
sbit P1_6			= 0x96;
sbit P1_7			= 0x97;

/*------------------------------------------------
EIF (0x91) Bit Values
------------------------------------------------*/
#define INT2F		0x01
#define INT3F		0x02
#define INT4F		0x04
#define INT5F		0x08
#define INT6F		0x10

/*------------------------------------------------
SCON0 (0x98) Bit Registers
------------------------------------------------*/
sbit RI0			= 0x98;
sbit TI0			= 0x99;
sbit RB08			= 0x9A;
sbit TB08			= 0x9B;
sbit REN0			= 0x9C;
sbit SM02			= 0x9D;
sbit SM01			= 0x9E;
sbit SM00			= 0x9F;

/*------------------------------------------------
WKUPSR (0x9C) Bit Values
------------------------------------------------*/
#define	GPIO_WKUP	0x01
#define	SPI_WKUP	0x02
#define	PS2_WKUP	0x04
#define	UHC_WKUP	0x08
#define	UDC0_WKUP	0x10
#define	UDC1_WKUP	0x20
#define	UDC2_WKUP	0x40
#define	UDC3_WKUP	0x80


/*------------------------------------------------
ACON (0x9D) Bit Values
------------------------------------------------*/
#define	FLAT_MODE	0x02
#define	LARGE_MODE	0x00

/*------------------------------------------------
PISSR (0x9E) Bit Values
------------------------------------------------*/
#define PS2A_INT_STU		0x01
#define PS2B_INT_STU		0x02
#define GP0_INT_STU			0x04
#define GP2_INT_STU			0x08
#define SPI1_INT_STU		0x10
#define SPI0_INT_STU		0x20
#define I2C_INT_STU			0x40
#define UR2_INT_STU			0x80

/*------------------------------------------------
UDCSR (0x9F) Bit Values
------------------------------------------------*/
#define UDC0_INT_STU		0x01
#define UDC1_INT_STU		0x02
#define UDC2_INT_STU		0x04
#define UDC3_INT_STU		0x08

/*------------------------------------------------
P2 (0xA0) Bit Registers
------------------------------------------------*/
sbit P2_0			= 0xA0;
sbit P2_1			= 0xA1;
sbit P2_2			= 0xA2;
sbit P2_3			= 0xA3;
sbit P2_4			= 0xA4;
sbit P2_5			= 0xA5;
sbit P2_6			= 0xA6;
sbit P2_7			= 0xA7;

/*------------------------------------------------
IE (0xA8) Bit Registers
------------------------------------------------*/
sbit EX0			= 0xA8;       /* 1=Enable External interrupt 0 */
sbit ET0			= 0xA9;       /* 1=Enable Timer 0 interrupt */
sbit EX1			= 0xAA;       /* 1=Enable External interrupt 1 */
sbit ET1			= 0xAB;       /* 1=Enable Timer 1 interrupt */
sbit ES0			= 0xAC;       /* 1=Enable Serial port 0 interrupt */
sbit ET2			= 0xAD;       /* 1=Enable Timer 2 interrupt */
sbit ES1			= 0xAE;       /* 1=Enable Serial port 1 interrupt */
sbit EA				= 0xAF;       /* 0=Disable all interrupts */

/*------------------------------------------------
P3 (0xB0) Bit Registers (Mnemonics & Ports)
------------------------------------------------*/
sbit P3_0			= 0xB0;
sbit P3_1			= 0xB1;
sbit P3_2			= 0xB2;
sbit P3_3			= 0xB3;
sbit P3_4			= 0xB4;
sbit P3_5			= 0xB5;
sbit P3_6			= 0xB6;
sbit P3_7			= 0xB7;

/*------------------------------------------------
IP (0xB8) Bit Registers
------------------------------------------------*/
sbit PX0			= 0xB8;
sbit PT0			= 0xB9;
sbit PX1			= 0xBA;
sbit PT1			= 0xBB;
sbit PS0			= 0xBC;
sbit PT2			= 0xBD;
sbit PS1			= 0xBE;

/*------------------------------------------------
SCON1 (0xC0) Bit Registers
------------------------------------------------*/
sbit RI1			= 0xC0;
sbit TI1			= 0xC1;
sbit RB18			= 0xC2;
sbit TB18			= 0xC3;
sbit REN1			= 0xC4;
sbit SM12			= 0xC5;
sbit SM11			= 0xC6;
sbit SM10			= 0xC7;

/*------------------------------------------------
T2CON (0xC8) Bit Registers
------------------------------------------------*/
sbit CPRL2			= 0xC8;
sbit CT2			= 0xC9;
sbit TR2			= 0xCA;
sbit EXEN2			= 0xCB;
sbit TCLK			= 0xCC;
sbit RCLK			= 0xCD;
sbit EXF2			= 0xCE;
sbit TF2			= 0xCF;

/*------------------------------------------------
PSW (0xD0) Bit Registers
------------------------------------------------*/
sbit P				= 0xD0;
sbit F1				= 0xD1;
sbit OV				= 0xD2;
sbit RS0			= 0xD3;
sbit RS1			= 0xD4;
sbit F0				= 0xD5;
sbit AC				= 0xD6;
sbit CY				= 0xD7;

/*------------------------------------------------
FCISR (0xD3) Bit Values
------------------------------------------------*/
#define	PWC_CS		0x01
#define	PWC_TO		0x02
#define	PWD_NR		0x04
#define	SDPWC_TO	0x08
#define PWD_CHKF	0x10

/*------------------------------------------------
PCKEN (0xD5) Bit Values
------------------------------------------------*/
#define UDC2_CKEN	0x01
#define UDC3_CKEN	0x02
#define SPI_CKEN	0x10
#define PS2_CKEN	0x20

/*------------------------------------------------
WDCON (0xD8) Bit Registers
------------------------------------------------*/
sbit RWT			= 0xD8;
sbit EWT			= 0xD9;
sbit WTRF			= 0xDA;
sbit WDIF			= 0xDB;

/*------------------------------------------------
SDSTSR (0xD9) Bit Registers
------------------------------------------------*/
#define	SDC			0x01
#define	STT			0x02
#define	BZRI		0x04
#define	FLASH_IS	0x10

/*------------------------------------------------
EIE (0xE8) Bit Registers
------------------------------------------------*/
sbit EINT2			= 0xE8;
sbit EINT3			= 0xE9;
sbit EINT4			= 0xEA;
sbit EINT5			= 0xEB;
sbit EINT6			= 0xEC;
sbit EWDI			= 0xED;

/*------------------------------------------------
EIP (0xF8) Bit Registers
------------------------------------------------*/
sbit PINT2			= 0xF8;
sbit PINT3			= 0xF9;
sbit PINT4			= 0xFA;
sbit PINT5			= 0xFB;
sbit PINT6			= 0xFC;
sbit PWDI			= 0xFD;

/*------------------------------------------------
Interrupt Vectors:
Interrupt Address = (Number * 8) + 3
------------------------------------------------*/
#define IE0_VECTOR	0	/* 0x03 External Interrupt 0 */
#define TF0_VECTOR	1	/* 0x0B Timer 0 */
#define IE1_VECTOR	2	/* 0x13 External Interrupt 1 */
#define TF1_VECTOR	3	/* 0x1B Timer 1 */
#define UR0_VECTOR	4	/* 0x23 Serial port UART0 */

#define TF2_VECTOR	5	/* 0x2B Timer 2 */
#define UR1_VECTOR	6	/* 0x33 Serial port UART1 */
#define INT2_VECTOR	7	/* 0x3B External Interrupt 2 */
#define INT3_VECTOR	8	/* 0x43 External Interrupt 3 */
#define INT4_VECTOR	9	/* 0x4B External Interrupt 4 */

#define INT5_VECTOR	10	/* 0x53 External Interrupt 5 */
#define INT6_VECTOR	11	/* 0x5B External Interrupt 6 */
#define WDIF_VECTOR	12	/* 0x63 Watchdog */

/*------------------------------------------------
DCxISR (DC0ISR~DC3ISR) Bit Registers
------------------------------------------------*/
#define	DCISR_SOF			0x01
#define	DCISR_ESOF			0x02
#define	DCISR_URST			0x04
#define	DCISR_VCHG			0x08
#define	DCISR_USUS			0x10
#define	DCISR_RWK			0x20
#define	DCISR_ERR			0x40
#define	DCISR_CT			0x80

/*------------------------------------------------
DCxINSR (DC0INSR~DC3INSR) Bit Registers
------------------------------------------------*/
#define	DCINSR_DIR			0x08

/*------------------------------------------------
DCxESMR (DC0ESMR~DC3ESMR) Bit Registers
------------------------------------------------*/
#define	DCESMR_SETUP		0x01
#define	DCESMR_SETOW		0x02
#define	DCESMR_PRE_TOK		0x04
#define	DCESMR_STALL		0x08
#define	DCESMR_DPID			0x10
#define	DCESMR_BUF0_AVLB	0x20
#define	DCESMR_BUF1_AVLB	0x40
#define	DCESMR_CBUF			0x80

/*------------------------------------------------
------------------------------------------------*/

#endif