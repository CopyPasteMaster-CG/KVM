/*
******************************************************************************
 *     Copyright (c) 2016	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
 
/* INCLUDE FILE DECLARATIONS */
#include <stdio.h>
#include <string.h>

#include "project_include.h"
#include <intrins.h>

#if (SYSETM_GPIO_EEPROM_ENABLE)  
/* $GLOBAL DEFINATION DECLARATIONS 	*/
#define EEPROM_WRITE_LENGTH  8 
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define 	GPIO_FSCL 	P2_6
#define     GPIO_FSDA	P2_7

#define     DEFAULT_DELAY		     2
#define     DEFAULT_GPIO_DELAY	     1    
#define     DEFAULT_START_ADDR       0x00
#define     DEFAULT_TARGET			 0xa0

/*$bEEprom_Control */
#define  EEPROM_OPERATION_FLAG		0x01  /* 0-Read,1-Write */
#define  EERPOM_OPERATION_WRITE		0x01

#define  EEPROM_START_STAGE			(0x01 << 1)  
#define  EEPROM_STOP_STAGE			(0x02 << 1)
#define  EEPROM_READ_START			(0x03 << 1)
#define  EEPROM_READ_STAGE			(0x04 << 1)
#define  EEPROM_WRITE_STAGE			(0x05 << 1)
#define  EEPROM_TARGET_STAGE		(0x06 << 1)
#define  EEPROM_ADDR_STAGE			(0x07 << 1)
/* $GLOBAL CONTANT DECLARATIONS 	*/
/* $GLOBAL VARIABLES DECLARATIONS 	*/ 
bit  bEEprom_WriteFlag;
bit  bEEprom_ErrorFlag;
bit  bEEprom_ServerNameInitFlag=0;
U8_T bEEprom_Control;
U8_T bKVM_Current_Port;
U8_T TASK_GPIO_EEProm_Control_ID;
U8_T TASK_GPIO_EEProm_Control_ActiveID;
U8_T cEEprom_ErrCnt;
/* $STATIC VARIABLE DECLARATIONS  	*/
/* $LOCAL SUBPROGRAM DECLARATIONS 	*/
void GPIO_I2C_Delay(U8_T us);
void GPIO_I2C_Start(void);
void GPIO_I2C_Stop(void);
void TASK_GPIO_EEProm_Control(void);
/*
 * -------------------------------------------------------------------------------
 * void GPIO_EEProm_Init(void)
 * Purpose : gpio eeprom init 
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */
void GPIO_EEProm_Init(void)
{
	bEEprom_Control = EEPROM_START_STAGE; // Read Mode,Start Condition 
	bKVM_Current_Port = 0;
	cEEprom_ErrCnt = 0;
	TASK_GPIO_EEProm_Control_ActiveID = 0;
	TASK_GPIO_EEProm_Control_ID = TASK_Create(TASK_GPIO_EEProm_Control);

#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("TASK_GPIO_EEProm_Control_ID=%bu\n\r",TASK_GPIO_EEProm_Control_ID);
#endif	
}

/*
 * -------------------------------------------------------------------------------
 * void GPIO_I2C_Delay(void)
 * Purpose : gpio delay subroutine
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */
void GPIO_I2C_Delay(U8_T us)
{
	U8_T index;
	
	while (us--) 
	{
		index = DEFAULT_DELAY;
		while (index--)
		{	
			_nop_();
		}	
	}
} 

/*
 * -------------------------------------------------------------------------------
 * void GPIO_I2C_Start(void)
 * Purpose : generate the start I2C state by gpio pin
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */
void GPIO_I2C_Start(void)
{    
    GPIO_FSDA=1; 
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);
    GPIO_FSCL=1;
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY); 
    GPIO_FSDA=0;
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);    
    GPIO_FSCL=0;
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);    
}
 
/*
 * -------------------------------------------------------------------------------
 * void GPIO_I2C_Stop(void)
 * Purpose : generate the stop I2C state by gpio pin
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */
void GPIO_I2C_Stop(void)
{
    GPIO_FSCL=0;
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);
    GPIO_FSDA=0;
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);    
    GPIO_FSCL=1;
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);  
    GPIO_FSDA=1;
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);    
} 

/*
 * -------------------------------------------------------------------------------
 * void GPIO_Ack(void)
 * Purpose : generate the ack I2C state by gpio pin
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */
void GPIO_Ack(void) 
{    
    GPIO_FSDA=0;
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);    
    GPIO_FSCL=1;
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);    
    GPIO_FSCL=0;
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);    
    GPIO_FSDA = 1;
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);    
}

/*
 * -------------------------------------------------------------------------------
 * void GPIO_NoAck(void)
 * Purpose : generate the no ack I2C state by gpio pin
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */
void GPIO_NoAck(void)
{    
    GPIO_FSDA=1;
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);    
    GPIO_FSCL=1;
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);    
    GPIO_FSCL=0;
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);    
}

/*
 * -------------------------------------------------------------------------------
 * void GPIO_Check_Ack(void)
 * Purpose : check the ack state by gpio pin
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */ 
U8_T GPIO_Check_Ack(void)
{
    bit ErrorBit;
   
    GPIO_FSDA=1;
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);    
    GPIO_FSCL=1;   
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);     
    ErrorBit=GPIO_FSDA;
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);    
    GPIO_FSCL=0;
    GPIO_FSDA=1;
    GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);    
    
    if (ErrorBit == 0)
    {	
         return TRUE;
	}
    else
    {	
    	return FALSE;    
    }	
}  

/*
 * -------------------------------------------------------------------------------
 * void Gpio_WriteI2CByte(void)
 * Purpose : write a i2c byte by gpio pin
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */ 
void Gpio_WriteI2CByte(U8_T input) 
{	     
    U8_T i;
    
    GPIO_FSCL = 0;
    GPIO_FSDA = 1;
    
    for (i=8 ;i != 0; i--) 
	{
    	if (input&0x80)    		
        	GPIO_FSDA = 1;
        else
        	GPIO_FSDA = 0;
        	
        GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);        
        GPIO_FSCL = 1;
        GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);
        GPIO_FSCL = 0;
        GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);
        input = input << 1;
    }        
}

/*
 * -------------------------------------------------------------------------------
 * U8_T Gpio_ReadI2CByte(void)
 * Purpose : read a i2c byte by gpio pin
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */ 
U8_T GPIO_ReadI2CByte(void) 
{    
    U8_T rbyte=0,i;
    
    GPIO_FSDA = 1;
    
    for(i=0;i<8;i++)
    {
        GPIO_FSCL = 1;
        GPIO_I2C_Delay(DEFAULT_GPIO_DELAY); 
        rbyte = rbyte << 1; 
        rbyte = rbyte | (U8_T) GPIO_FSDA;
        GPIO_I2C_Delay(DEFAULT_GPIO_DELAY); 
        GPIO_FSCL = 0;
        GPIO_I2C_Delay(DEFAULT_GPIO_DELAY);         
    }
    return rbyte;
}  

/*
 * -------------------------------------------------------------------------------
 * void Gpio_ReadEEprom(void)
 * Purpose : read the 24c02 content
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */ 
void GPIO_ReadEEprom(void) 
{
	if (TASK_GPIO_EEProm_Control_ActiveID == 0)
	{
		bEEprom_Control = EEPROM_START_STAGE;
		TASK_GPIO_EEProm_Control_ActiveID = TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_GPIO_EEProm_Control_ID,0,0,1,1);
	}		     		 
}

/*
 * -------------------------------------------------------------------------------
 * void WriteEEprom(void)
 * Purpose : read the 24c02 content
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */
void GPIO_WriteEEprom(void) 
{
	if (TASK_GPIO_EEProm_Control_ActiveID == 0)
	{
		bEEprom_Control = EEPROM_START_STAGE | EERPOM_OPERATION_WRITE;
		TASK_GPIO_EEProm_Control_ActiveID = TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_GPIO_EEProm_Control_ID,0,0,1,1);
	}
}

/*
 * -------------------------------------------------------------------------------
 * void TASK_GPIO_EEProm_Control(void)
 * Purpose : write a i2c byte by gpio pin
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */
void TASK_GPIO_EEProm_Control(void)
{
	U8_T operation;
	U8_T state;
	
	operation = bEEprom_Control & 0x01;
	state = bEEprom_Control & 0xfe;
	
	switch(state)
	{
		case EEPROM_START_STAGE:
			GPIO_I2C_Start();
			//bEEprom_Control = EEPROM_TARGET_STAGE | operation;
			//break;
		case EEPROM_TARGET_STAGE:
			Gpio_WriteI2CByte(DEFAULT_TARGET);
			if ( GPIO_Check_Ack()) //Get 24C02 responsed
		 	{
		 	    bEEprom_Control = EEPROM_ADDR_STAGE | operation;		 		
		 	}
		 	else
		 	{ // no ack bit, this should not happened
		 		cEEprom_ErrCnt++;
		 		if (cEEprom_ErrCnt > 5)
		 		{ //print out error message, then stop the eeprom operation
		 			cEEprom_ErrCnt=0;
		 			printf("> (ERROR)EEProm not responsed, skip opertion !!\n\r");
		 			bEEprom_Control=0;
		 			TASK_Destory_Current();
					TASK_GPIO_EEProm_Control_ActiveID = 0;
		 		}	
		 	}	
			break;	
		case EEPROM_ADDR_STAGE:
			Gpio_WriteI2CByte(DEFAULT_START_ADDR);
			if ( GPIO_Check_Ack()) 
		 	{		 		
		 		if (bEEprom_Control & EERPOM_OPERATION_WRITE)
				{
					bEEprom_Control = EEPROM_WRITE_STAGE | operation;
				}	
				else
				{
					bEEprom_Control = EEPROM_READ_START | operation;
				}
		 	}
			break;
		case EEPROM_WRITE_STAGE:
			bKVM_Current_Port = KVM_CurrentHost;
			Gpio_WriteI2CByte(bKVM_Current_Port);
			if ( GPIO_Check_Ack())
			{
				printf("EEprom:%bu write done\n\r",bKVM_Current_Port);
				GPIO_I2C_Stop();
				bEEprom_Control = 0;
				TASK_Destory_Current();
				TASK_GPIO_EEProm_Control_ActiveID = 0;
			}	
			break;	
		case EEPROM_READ_START:
			GPIO_I2C_Start();        
			Gpio_WriteI2CByte(DEFAULT_TARGET|0x01);          
          	if ( GPIO_Check_Ack() ) 
          	{
          		bEEprom_Control = EEPROM_READ_STAGE | operation;               
          	}          
			break;	
		case EEPROM_READ_STAGE:
			bKVM_Current_Port = GPIO_ReadI2CByte();		
			printf("EEprom:%bu read done\n\r",bKVM_Current_Port);	
          	GPIO_NoAck();
          	GPIO_I2C_Stop();
          	bEEprom_Control = 0;
			TASK_Destory_Current();
			TASK_GPIO_EEProm_Control_ActiveID = 0;
			if (KVM_CurrentHost != bKVM_Current_Port)
			{	
				if (KVM_Power_Jump_Check(bKVM_Current_Port,0))
				{
					BTN_Processing_Flag = 1; // in btn processing
					KVM_Control_Status |= KVM_PORT_SW_MASK;
					TASK_KVM_Switch_Event_Control_Fork(bKVM_Current_Port);
					KVM_Audio_Combo_Switch_Check(bKVM_Current_Port);
				}
			}	
			break;	
	}		
}      
#endif  /* #if (SYSETM_GPIO_EEPROM_ENABLE) */ 
 