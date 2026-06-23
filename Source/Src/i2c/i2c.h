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
 * Module Name : i2c.h
 * Purpose     : A header file of I2C module.
 * Author      : 
 * Date        :
 * Notes       :
 *================================================================================
 */
#ifndef I2C_H
#define I2C_H

/* INCLUDE FILE DECLARATIONS */
/* NAMING CONSTANT DECLARATIONS */
/* I2C Register */
#define I2CCPR					0x00	// I2C Clock Pre-scale Register
#define I2CMTR					0x02	// I2C Master Transmit Register
#define I2CRR					0x03	// I2C Master Receive Register
#define I2CMCR					0x04	// I2C Master Control Register
#define I2CADPTI				0x05	// I2C Auto-Detection Plug Time Interval Register
#define I2CMIER					0x06	// I2C Master Interrupt Enable Register
#define I2CMISR					0x07	// I2C Master Interrupt Status Register
#define I2CMSR					0x08	// I2C Master Status Register
#define I2CCSR					0x09	// I2C Check Sum Register
#define I2CMESA					0x0A	// I2C Master EDID Start Address
#define I2CMTOI					0x0B	// I2C Master Time Out Interval Register
#define I2CSCR					0x10	// I2C Slave Control Register
#define I2CSTOI					0x11	// I2C Slave Time Out Interval Register
#define I2CSESA					0x12	// I2C Slave EDID Start Address
#define I2CSSR					0x13	// I2C Slave Status Register

/* I2CMCR */
#define I2CMCR_RECOVERY_EXE		BIT0	// Execute the Recovery function of Software
#define I2CMCR_ADP_ENB			BIT1	// I2C Auto Detection Plug module enable
#define I2CMCR_RLE_EDID			BIT2	// I2C reload extended display identification data(EDID)
#define I2CMCR_MASTER_GO		BIT3	// I2C master go command
#define I2CMCR_CMD_WRITE		BIT4	// Request to send data in master mode
#define I2CMCR_CMD_READ			BIT5	// Request to receive data in master mode
#define I2CMCR_STOP_COND		BIT6	// Generate a STOP condition in master mode
#define I2CMCR_NO_STOP			0		// No STOP condition
#define I2CMCR_START_COND		BIT7	// Generate a START condition in master mode


/* I2CMIER */
#define I2CMIER_TCIE			BIT0	// Transfer complete interrupt enable
#define I2CMIER_PSCIE			BIT1	// Plug status interrupt enable
#define I2CMIER_RLEDIE			BIT2	// Reload display's EDID done interrupt enable
#define I2CMIER_BPSTOIE			BIT3	// I2C Bypass Channel Time Out Interrupt Enable
#define I2CMIER_RCVYIE			BIT4	// The Recovery function of SW or HW is completed Interrupt Enable
#define I2CMIER_DEFAULT_INTE	0x1F

/* I2CMISR */
#define I2CMISR_TC				BIT0	// Transfer completed
#define I2CMISR_ARB_LOST		BIT1	// Arbitration lost
#define I2CMISR_NO_ACK			BIT2	// No ack after a byte transfer
#define I2CMISR_PSC				BIT3	// Plug status change
#define I2CMISR_EDID_DONE		BIT4	// Reload display's EDID is done
#define I2CMISR_BYPASS_TO		BIT5	// I2C Slave Bypass Channel Time Out Status
#define I2CMISR_RECOVERY_CPL	BIT6	// The Recovery function was Completed Status

/* I2CMSR */
#define I2CMSR_TIP				BIT0	// Transfer in progress
#define I2CMSR_BUS_BUSY			BIT1	// I2C bus busy
#define I2CMSR_EX_FLAG			BIT2	// I2C extenstion flag
#define I2CMSR_CHECKSUM_ERR		BIT3	// I2C check sum miss-matched
#define I2CMSR_EDID_OK			BIT4	// I2C extended display identification data loader done successfully
#define I2CMSR_DPS_ON			BIT5	// I2C display plug status
#define I2CMSR_RCVR_FAIL		BIT6	// The Recovery function of SW or HW has failed
#define I2CMSR_SDA_HIGH			BIT7	// I2C master SDA status

/* I2CSCR */
#define I2CSCR_SLV0_ENB			BIT0	// I2C slave0 enable
#define I2CSCR_SLV1_ENB			BIT1	// I2C slave1 enable
#define I2CSCR_SLV2_ENB			BIT2	// I2C slave2 enable
#define I2CSCR_SLV3_ENB			BIT3	// I2C slave3 enable
#define I2CSCR_SLV_GLITCH_FLT	BIT4	// I2C Slave Bus glitch filter
#define I2CSCR_BYPASS_ENB		BIT5	// I2C Bypass mode Enable
#define I2CSCR_BYPASS_SLV0		0		// Bypass I2C Slave_0 channel to I2C master
#define I2CSCR_BYPASS_SLV1		BIT6	// Bypass I2C Slave_1 channel to I2C master
#define I2CSCR_BYPASS_SLV2		BIT7	// Bypass I2C Slave_2 channel to I2C master
#define I2CSCR_BYPASS_SLV3		(BIT6|BIT7)	// Bypass I2C Slave_3 channel to I2C master

/* I2CSSR */
#define I2CSSR_SLV0_BUSY		BIT0	// I2C slave0 busy
#define I2CSSR_SLV1_BUSY		BIT1	// I2C slave1 busy
#define I2CSSR_SLV2_BUSY		BIT2	// I2C slave2 busy
#define I2CSSR_SLV3_BUSY		BIT3	// I2C slave3 busy
#define I2CSSR_CUR_BPS_ON		BIT4	// I2C slave0 busy
#define I2CSSR_BPSON_SLV0		0		// I2C current bypass slave0
#define I2CSSR_BPSON_SLV1		BIT5	// I2C current bypass slave1
#define I2CSSR_BPSON_SLV2		BIT6	// I2C current bypass slave2
#define I2CSSR_BPSON_SLV3		(BIT5|BIT6)	// I2C current bypass slave3
#define	I2CSSR_SLV0123_BUSY		0x0F

/* I2C packet direction */
#define I2C_MASTER				BIT1
#define I2C_XMIT				BIT0
#define I2C_MST_XMIT			(BIT1+BIT0)	//11 (BIT1:Master, BIT0:Xmit)
#define I2C_MST_RCVR			BIT1		//10
#define I2C_SLV_XMIT			BIT0		//01
#define I2C_SLV_RCVR			0			//00

/* I2C SW State Check */
#define I2C_BUSY				BIT0	// I2C busy flag
#define I2C_RESTART				BIT1	// I2C restart flag
#define I2C_LOST				BIT2	// I2C arbitration lost flag
#define I2C_RCVR_FAIL			BIT3	// I2C arbitration lost flag
#define I2C_NACK				BIT7	// I2C NACK flag

/* I2C Master Clock */
#define I2C_FAST_48M			0x0017	// Fast mode pre-scale at 48M
#define I2C_FAST_96M			0x0030	// Fast mode pre-scale at 96M
#define I2C_STD_48M				0x005F	// Standard mode pre-scale at 48M
#define I2C_STD_96M				0x00BF	// Standard mode pre-scale at 96M

/* I2C Slave Command */
#define I2C_SLV_SRSFR			BIT5			// SFR single read
#define I2C_SLV_SWSFR			(BIT7+BIT5)		// SFR single write
#define I2C_SLV_IRSFR			(BIT5+BIT4)		// SFR indirect read
#define I2C_SLV_IWSFR			(BIT7+BIT5+BIT4)// SFR indirect write
#define I2C_SLV_BRDM			BIT6			// Memory burst read
#define I2C_SLV_BWDM			(BIT7+BIT6)		// Memory burst write
#define I2C_SLV_DEFAULT_TO		0x10

/* I2CMTOI */
#define I2C_BYPASS_TIMEOUT		0x02

/* I2C Polling Mode State */
#define 		I2C_POLL_MODE			0
#if I2C_POLL_MODE
	#define 	I2C_WRITE_MEMORY		BIT0
	#define 	I2C_READ_MEMORY			BIT1
	#define 	I2C_IDLE_STATE			0
	#define 	I2C_WRITE_INIT_STATE	BIT0
	#define 	I2C_WRITE_DATA_STATE	BIT1
	#define 	I2C_WRITE_STOP_STATE	BIT2
	#define 	I2C_READ_INIT_STATE		BIT3
	#define 	I2C_READ_DATA_STATE		BIT4
	#define 	I2C_READ_STOP_STATE		BIT5
	#define 	I2C_RESTART_STATE		BIT6
	#define 	I2C_BUSY_STATE			BIT7
#endif

/* MACRO DECLARATIONS */


/* TYPE DECLARATIONS */
typedef	struct	i2c_buf {
	U8_T		EndCondition;		//packet condition
	U8_T 		DirTransfer;		//packet direction
	U16_T 		PktLength;			//packet length including address and data
	U8_T		DevIdAddr;			//device address only 7-bit
	U8_T 		*DataPayloadPtr;	//data field
}	I2C_BUF;


/* GLOBAL VARIABLES */
extern U8_T		i2c_PollState;


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
//void	I2C_RegRead(U8_T regAddr, U8_T *pRegData, U8_T regLen);
void	I2C_RegWrite(U8_T regAddr, U8_T *pRegData, U8_T regLen);
void	I2C_MstSetup(U16_T preClk, U8_T intrCmd, U8_T detTime);
void	I2C_SlvSetup(U8_T ctrlCmd, U8_T timeout);
void	I2C_IntrEntryFunc(void);
BOOL	I2C_PktBuf(I2C_BUF *ptI2cBuf);
BOOL	I2C_FlagChk(U8_T chkBit);
void	I2C_FlagEnb(U8_T enbBit);
void	I2C_FlagClr(U8_T clrBit);
U8_T	I2C_EdidGetPlugStatus(void);
void	I2C_EdidWaitReloadDone(void);
BOOL	I2C_EdidContentReload(void);
#endif /* End of I2C_H */