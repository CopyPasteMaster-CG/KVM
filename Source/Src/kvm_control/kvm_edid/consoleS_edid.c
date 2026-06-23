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
 * Module Name : console_edid.c
 * Purpose     : This module handles the console EDID read function
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include <stdio.h>
#include <string.h>

/* System Relate Header File Seciton */
#include "project_include.h"

	
#if (SYSETM_CONSOLE_EDID_CONTROL_ENABLE) && (SYSTEM_SWEDID_CONTROL_ENABLE)
U8_T CODE CONSOLE_EDID_DEFAULT[]=
{
	// 00, 01 , 02 , 03 , 04 , 05 , 06 , 07 , 08 , 09 , 0a , 0b , 0c , 0d ,0e , 0f
	0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0xA1,0x01,0x20,0x13,0x01,0x40,0x15,0x01,
	// 10 , 11 , 12 , 13 , 14 , 15 , 16 , 17 , 18 , 19 , 1a , 1b , 1c , 1d ,1e , 1f
	0x20,0x11,0x01,0x03,0x68,0x29,0x1a,0x78,0xee,0xc1,0x25,0xa3,0x56,0x4b,0x99,0x27,
	// 20 , 21 , 22 , 23 , 24 , 25 , 26 , 27 , 28 , 29 , 2a , 2b , 2c , 2d ,2e , 2f
	0x11,0x50,0x54,0xbf,0xef,0x80,0x95,0x00,0x71,0x4f,0x81,0x80,0x95,0x0f,0x01,0x01,
	// 30 , 31 , 32 , 33 , 34 , 35 , 36 , 37 , 38 , 39 , 3a , 3b , 3c , 3d ,3e , 3f
	0x01,0x01,0x01,0x01,0x01,0x01,0x9a,0x29,0xa0,0xd0,0x51,0x84,0x22,0x30,0x50,0x98,
	// 40 , 41 , 42 , 43 , 44 , 45 , 46 , 47 , 48 , 49 , 4a , 4b , 4c , 4d ,4e , 4f
	0x36,0x00,0x98,0xff,0x10,0x00,0x00,0x1c,0x00,0x00,0x00,0xfd,0x00,0x38,0x4b,0x1e,
	// 50 , 51 , 52 , 53 , 54 , 55 , 56 , 57 , 58 , 59 , 5a , 5b , 5c , 5d ,5e , 5f
	0x53,0x0e,0x00,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0xff,0x00,0x55,
	// 60 , 61 , 62 , 63 , 64 , 65 , 66 , 67 , 68 , 69 , 6a , 6b , 6c , 6d ,6e , 6f
	0x4e,0x34,0x38,0x34,0x37,0x38,0x39,0x33,0x33,0x32,0x4c,0x0a,0x00,0x0,0x00,0xfc,
	// 70 , 71 , 72 , 73 , 74 , 75 , 76 , 77 , 78 , 79 , 7a , 7b , 7c , 7d ,7e , 7f
	0x00,0x44,0x45,0x4c,0x4c,0x20,0x53,0x45,0x31,0x39,0x38,0x57,0x46,0x50,0x00,0x1e
};
/* STATIC VARIABLE DECLARATIONS */
U8_T	ConsoleEdidTable[EDID_MAX] _at_ EDID_START_ADDR;// Console Edid Table

// For TASK Usaged ------------------------------------------------
U8_T  TASK_EDID_CONSOLE_PLUG_CHECK_ID;
EDID_Ctrl	EDID_ctrl;
// ----------------------------------------------------------------

/* LOCAL SUBPROGRAM DECLARATIONS */
void TASK_EDID_Console_Plug_Check(void);
void EDID_Active_Plug_Check_Now_Task(void);
/* LOCAL SUBPROGRAM BODIES */
void Console_EDID_PlugIn(void);
void Console_EDID_Handle(void);
void Console_EDID_Monitor_Plugoff(void);
void Console_EDID_Monitor_Reload(void);
/*
 *--------------------------------------------------------------------------------
 * void  EDID_Console_Init(void)
 * Purpose : Console EDID I2C port & relative parameter init
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void EDID_Console_Init(void)
{
	memset((U8_T*)&EDID_ctrl, 0, sizeof(EDID_ctrl));
	EDID_ctrl.TaskState = CONSOLE_MONITOR_PLUG_OUT;
	EDID_ctrl.MonitorPlugStatus = MONITOR_UNPLUGGED;// default monitor is plug out
	EDID_ctrl.EdidReloadStatus = EDID_RELOAD_BUSY;// monitor edid has not been readed	
	EDID_ctrl.MonitorDebounceCnt = 0;// monitor edid has not been readed	
	memcpy(ConsoleEdidTable, CONSOLE_EDID_DEFAULT, sizeof(CONSOLE_EDID_DEFAULT));
	memset(&ConsoleEdidTable[128],0x00,sizeof(CONSOLE_EDID_DEFAULT));
	//Due the defaut table is locate in rom, so used the program dma copy
	//DMA_ProgramToData(ConsoleEdidTable,CONSOLE_EDID_DEFAULT,sizeof(CONSOLE_EDID_DEFAULT));
    //DMA_GrantXdata(ConsoleEdidTable,CONSOLE_EDID_DEFAULT, sizeof(CONSOLE_EDID_DEFAULT));
		
	/* init the Hardward/Software I2C Master	Mode */
	
	//Task Control
	TASK_EDID_CONSOLE_PLUG_CHECK_ID = TASK_Create(TASK_EDID_Console_Plug_Check); 
	TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_EDID_CONSOLE_PLUG_CHECK_ID,0,0,1000,1000); // now aticev the process for check edid process
#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("TASK_EDID_CONSOLE_PLUG_CHECK_ID=%bu\n\r",TASK_EDID_CONSOLE_PLUG_CHECK_ID);		
#endif	
	
	I2C_Core_Init();
}

/*
 *--------------------------------------------------------------------------------
 * void  TASK_EDID_Console_Plug_Check(void)
 * Purpose : read the EDID table, if fail then stand for the monitor not plug in
 *           this function shall be performed every 10ms
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void  TASK_EDID_Console_Plug_Check(void)
{
	U8_T buf[2];	
	
	if (EDID_ctrl.TaskState == CONSOLE_MONITOR_PLUG_CHECK) //the plug in check operation is not complete
	{
		return;
	}		
	
	EDID_ctrl.TaskState = CONSOLE_MONITOR_PLUG_CHECK; //now check the plug in	
	buf[0] = I2C_EDID; //transfer the A0 command, for read only, to check 24c02 if it answer ACK	
	
	I2C_Core_Cmd_Transmit(buf,
						  (I2C_CONTROL_TRANSMIT|I2C_CONTROL_USE_RING),
						  1,
						  Console_EDID_Handle);		
	//printf("[PlugCheck]");
//#if (EDID_DEBUG_SW)
//	printf("[Monitor Check]");
//#endif
}

/*--------------------------------------------------------------------------------
 * void Console_EDID_Handle(void)
 * Purpose : Handling the EDID event from I2C bus
 * Params  : 
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------*/
void Console_EDID_Handle(void)
{		 
#if (EDID_DEBUG_SW)
	U8_T address, index, n;
#endif
	
	if (I2C_Tx_State & I2CMISR_NO_ACK) // no ack
	{		
		if (EDID_ctrl.MonitorPlugStatus == MONITOR_PLUGGED)
		{
			EDID_ctrl.MonitorDebounceCnt++;
			if (EDID_ctrl.MonitorDebounceCnt > 2) // 3 times no responsed
			{
				Console_EDID_Monitor_Plugoff();
			}	
		}			
		else if (EDID_ctrl.MonitorPlugStatus == MONITOR_UNPLUGGED)
		{
			EDID_ctrl.MonitorDebounceCnt = 0;
			if (EDID_ctrl.TaskState == CONSOLE_MONITOR_RELOAD) //if monitor not responsed
			{
				Console_EDID_Monitor_Plugoff(); // goto monitor plug off state
			}
			else
			{				
				EDID_ctrl.TaskState = CONSOLE_MONITOR_PLUG_OUT; // not monitor plug in
			}	
					
		}	
	}	
	else
	{ // get ACK, then 
		if (I2C_Tx_State & I2CMISR_TC)
		{						
			if (EDID_ctrl.MonitorPlugStatus == MONITOR_UNPLUGGED)
			{				
				if (EDID_ctrl.TaskState == CONSOLE_MONITOR_PLUG_CHECK)
				{						
					EDID_ctrl.MonitorDebounceCnt++; //wait I2C bus stable
					if (EDID_ctrl.MonitorDebounceCnt > 1)
					{
						EDID_ctrl.TaskState = CONSOLE_MONITOR_RELOAD;						
						Console_EDID_Monitor_Reload(); //goto edid read step
					}
					else
					{
						EDID_ctrl.TaskState = CONSOLE_MONITOR_PLUG_OUT; // not monitor plug in
					}	
				}		
				else if (EDID_ctrl.TaskState == CONSOLE_MONITOR_RELOAD)
				{					
					//if (event == I2C_CONTROL_RECEIVE)
					{	
						//printf("I2C_Transmit_Len=%d\n\r",(U16_T)I2C_Transmit_Index);
						if ((I2C_Transmit_Index-1) == EDID_TABLE_LENGTH)
						{	
#if (EDID_DEBUG_SW)
							printf("Box Console_EDID():Monitor=>[");
							printf("%-22.22s",&ConsoleEdidTable[113]);
							printf("]\n\r");
							address =0;
							for (index=0; index < 16 ; index++)
							{
								address = index << 3;
								printf("[%03d~%03d][",(U16_T)address,(U16_T)address+7);
								for ( n=0; n < 8 ; n++)
								{
									if (n < 7)
										printf("%02x ",(U16_T)ConsoleEdidTable[address+n]);
									else
										printf("%02x",(U16_T)ConsoleEdidTable[address+n]);
								}
								printf("]\n\r");
							}							
#endif
							EDID_ctrl.MonitorPlugStatus = MONITOR_PLUGGED;
							EDID_ctrl.TaskState = CONSOLE_MONITOR_STATUS_CHK;
							Console_EDID_PlugIn();
						}	
					}	
				}	
			}	
			else
			{
				if (EDID_ctrl.TaskState == CONSOLE_MONITOR_PLUG_CHECK) //plug out check
				{	
					//Do nothing, because the monitor still in				
					EDID_ctrl.TaskState = CONSOLE_MONITOR_STATUS_CHK;	
				}		
			}	
		}	
	}	
}

/*--------------------------------------------------------------------------------
 * void Console_EDID_Monitor_Plugoff(void)
 * Purpose : Monitor in plug off state
 * Params  : 
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------*/
void Console_EDID_Monitor_Plugoff(void)
{
	EDID_ctrl.TaskState = CONSOLE_MONITOR_PLUG_OUT;
	EDID_ctrl.MonitorPlugStatus = MONITOR_UNPLUGGED;// default monitor is plug out	
	EDID_ctrl.MonitorDebounceCnt = 0;// monitor edid has not been readed	
}

/*--------------------------------------------------------------------------------
 * void Console_EDID_Monitor_Reload(void)
 * Purpose : Monitor in plug off state
 * Params  : 
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------*/
void Console_EDID_Monitor_Reload(void)
{
	U8_T buf[2];

	//printf("[Reload]");		
	//1.Send start address out			
	buf[0] = I2C_EDID; //transfer the A0 command for read
	buf[1] = 0x00; //start address is 0;
	I2C_Core_Cmd_Transmit(buf,
						 (I2C_CONTROL_TRANSMIT|I2C_CONTROL_USE_RING|I2C_CONTROL_TRANSMIT_NOSTOP),
						 2,
						 NULL);	
			
	buf[0] = I2C_EDID | 0x01; //transfer the A1 command, for read start
	I2C_Core_Cmd_Transmit(buf,
						 (I2C_CONTROL_TRANSMIT|I2C_CONTROL_USE_RING|I2C_CONTROL_TRANSMIT_NOSTOP),
						 1,
						 NULL);					 
					 
	//2.Read data in 256 bytes		
	I2C_Core_Cmd_Transmit(ConsoleEdidTable,
						  (I2C_CONTROL_RECEIVE),
						  256,
						  Console_EDID_Handle);	
						  
}

/*----------------------------------------------------------------------------
 * Function Name: Console_EDID_PlugIn 
 * Purpose:  
 * Params : 
 * Returns: NONE
 * Note:
 *----------------------------------------------------------------------------
 */
void Console_EDID_PlugIn(void)
{	
	//Cascade_EDID_Control();
	//Cascade_Console_State |= CASCADEM_EDID_VALID;	
	printf("** Monitor Plug in\n\r");
}   
#endif //(SYSETM_CONSOLE_EDID_CONTROL_ENABLE)

/* End of console_edid.c */
