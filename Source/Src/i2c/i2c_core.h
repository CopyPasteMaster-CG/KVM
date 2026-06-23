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
/*=============================================================================
 * Module Name: cascade_i2c_core.h
 * Purpose: i2c bus control and handle
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __I2C_BUS_H__
#define __I2C_BUS_H__

/* INCLUDE FILE DECLARATIONS 		  */
/* DEFINATION DECLARATIONS   		  */
#define	I2C_BUF_MAX				 	 256 

/*********************************
$ Control (Defination)
*********************************/
#define I2C_CONTROL_IDLE           		0x00
#define I2C_CONTROL_TRANSMIT     		0x01
#define I2C_CONTROL_TRANSMIT_NOSTOP		0x02
#define I2C_CONTROL_RECEIVE	    		0x04
#define I2C_CONTROL_USE_RING    		0x10
#define I2C_CONTROL_FREE 	     		0x80 //buffer should not free after send complete


#define I2C_HEADER						0x96
//#define I2C_WRITE						0x00
/*************************************
$ Target (Defination)
**************************************/
#define	 I2C_EDID				0xA0	
#define	 I2C_EEPROM_24C0x		0xA0	
/*************************************/

typedef struct _I2C_Transimit
{	
	U8_T	Header;							// Header byte=0xa3	
	U8_T	Control;						// Target Device
	U16_T 	DataLen;						// FIFO Data payload buffer length											
	void    (* Funp)(void);
	U8_T  	*Buf;							// Transmitter Buffer pointer			
} I2C_Transimit_Def;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void I2C_Core_Init(void);
void I2C_Core_Cmd_Transmit(U8_T *buf,U8_T rwattr,U16_T len,void *functionp);
void I2C_Core_Transmit_FIFO_Send(void);
void I2C_Core_Handle_After_Transfer_Complete(U8_T i2c_state);
/* EXPORTED GLOBAL VARIABLES		 	*/

extern  U16_T  I2C_Tx_Start;
extern  U16_T  I2C_Tx_End;
extern	U8_T  I2C_Control_State;
extern  U8_T  I2C_Control_Target;
extern  U8_T  I2C_RingBuffer[I2C_BUF_MAX];
extern  U8_T  I2C_Tx_State;
extern	U16_T I2C_Transmit_OutP;
extern	U16_T I2C_Transmit_InP;

extern	idata volatile U16_T  I2C_Transmit_Index;
extern	idata volatile U16_T  I2C_Transmit_Len;
extern	idata volatile U16_T  I2C_Transmit_End;
extern	idata volatile U8_T  *I2C_Transmit_Buf;

#endif /* #ifndef __CASCADE_I2C_H__ */
/*
$ End of file i2c_core.h
*/
