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

	
#if (SYSETM_CONSOLE_EDID_CONTROL_ENABLE) && (SYSTEM_HWEDID_CONTROL_ENABLE)
//For VGA Monitor
U8_T CONSOLE_EDID_DEFAULT[]=
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

/* 
//For HDMI Monitor
U8_T CONSOLE_EDID_DEFAULT[]=
{
	// 00, 01 , 02 , 03 , 04 , 05 , 06 , 07 , 08 , 09 , 0a , 0b , 0c , 0d ,0e , 0f
	0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x32,0x8d,0x00,0x00,0x00,0x00,0x00,0x00,
	// 10 , 11 , 12 , 13 , 14 , 15 , 16 , 17 , 18 , 19 , 1a , 1b , 1c , 1d ,1e , 1f
	0x2d,0x13,0x01,0x03,0x80,0x58,0x32,0x78,0x2a,0xee,0x91,0xa3,0x54,0x4c,0x99,0x26,
	// 20 , 21 , 22 , 23 , 24 , 25 , 26 , 27 , 28 , 29 , 2a , 2b , 2c , 2d ,2e , 2f
	0x0f,0x50,0x54,0xbd,0xef,0x80,0x71,0x4f,0x81,0x00,0x81,0x40,0x81,0x80,0x95,0x00,
	// 30 , 31 , 32 , 33 , 34 , 35 , 36 , 37 , 38 , 39 , 3a , 3b , 3c , 3d ,3e , 3f
	0x95,0x0f,0xb3,0x00,0xa9,0x40,0x02,0x3a,0x80,0x18,0x71,0x38,0x2d,0x40,0x58,0x2c,
	// 40 , 41 , 42 , 43 , 44 , 45 , 46 , 47 , 48 , 49 , 4a , 4b , 4c , 4d ,4e , 4f
	0x45,0x00,0xa0,0x5a,0x00,0x00,0x00,0x1e,0x66,0x21,0x50,0xb0,0x51,0x00,0x1b,0x30,
	// 50 , 51 , 52 , 53 , 54 , 55 , 56 , 57 , 58 , 59 , 5a , 5b , 5c , 5d ,5e , 5f
	0x40,0x70,0x36,0x00,0xa0,0x5a,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0xfd,0x00,0x32,
	// 60 , 61 , 62 , 63 , 64 , 65 , 66 , 67 , 68 , 69 , 6a , 6b , 6c , 6d ,6e , 6f
	0x3c,0x1e,0x44,0x0f,0x00,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0xfc,
	// 70 , 71 , 72 , 73 , 74 , 75 , 76 , 77 , 78 , 79 , 7a , 7b , 7c , 7d ,7e , 7f
	0x00,0x52,0x54,0x58,0x32,0x2e,0x39,0x4b,0x5f,0x41,0x4e,0x0a,0x20,0x20,0x01,0x76
};
*/

/* STATIC VARIABLE DECLARATIONS */
U8_T  ConsoleEdidTable[EDID_MAX]	_at_ 0x00007F00;// Console Edid Table

// For TASK Usaged ------------------------------------------------
U8_T  TASK_EDID_CONSOLE_PLUG_CHECK_ID;
EDID_Ctrl	EDID_ctrl;
// ----------------------------------------------------------------

/* LOCAL SUBPROGRAM DECLARATIONS */
void TASK_EDID_Console_Plug_Check(void);
void EDID_Active_Plug_Check_Now_Task(void);
/* LOCAL SUBPROGRAM BODIES */

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
#ifdef EDID_SAVE	
	memcpy(ConsoleEdidTable,KVM_Flash.Edid_Table,sizeof(ConsoleEdidTable));  
#else	
	memcpy(ConsoleEdidTable,CONSOLE_EDID_DEFAULT,sizeof(CONSOLE_EDID_DEFAULT));  
#endif		

	/* init the Hardward/Software I2C Master	Mode */
#if (SYSTEM_HWEDID_CONTROL_ENABLE)
	HW_EDID_Init();
#endif
	
#if (SYSTEM_SWEDID_CONTROL_ENABLE)
	SW_EDID_Init();
#endif
	
	//Task Control
	TASK_EDID_CONSOLE_PLUG_CHECK_ID = TASK_Create(TASK_EDID_Console_Plug_Check); 
	
#if (SYSTEM_HWEDID_CONTROL_ENABLE)
	EDID_Active_Plug_Check_Now_Task();
#endif

#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("TASK_EDID_CONSOLE_PLUG_CHECK_ID=%bu\n\r",TASK_EDID_CONSOLE_PLUG_CHECK_ID); 
#endif
}

/*
 *--------------------------------------------------------------------------------
 * void  EDID_Active_Plug_Check_Now_Task(void)
 * Purpose : Setup the plugdetect task-default is 8ms, so check ater 10 ms
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void EDID_Active_Plug_Check_Now_Task(void)
{
	HW_EDID_PlugAutoDetectMode(0,ENABLE);
	TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_EDID_CONSOLE_PLUG_CHECK_ID,0,0,10,10); // now aticev the process for check edid process
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
#if (EDID_DEBUG_SW)
	U8_T address, index, n;
#endif

	if (EDID_ctrl.Timer)
		EDID_ctrl.Timer--;

	switch (EDID_ctrl.TaskState)
	{
	case CONSOLE_MONITOR_PLUG_OUT:
	case CONSOLE_MONITOR_STATUS_CHK:
		/* Debouncing process */
		if (EDID_ctrl.MonitorPlugStatus == MONITOR_PLUGGED)
		{
			if (EDID_ctrl.MonitorDebounceCnt < 100)
				EDID_ctrl.MonitorDebounceCnt++;
			else
				EDID_ctrl.MonitorDebouncedPlugStatus = MONITOR_PLUGGED;
		}	
		else
		{
			if (EDID_ctrl.MonitorDebounceCnt != 0)
				EDID_ctrl.MonitorDebounceCnt--;		
			else
				EDID_ctrl.MonitorDebouncedPlugStatus = MONITOR_UNPLUGGED;
		}
		
		if (EDID_ctrl.MonitorPrePlugStatus==0 && EDID_ctrl.MonitorDebouncedPlugStatus!=0)
		{
			EDID_ctrl.TaskState = CONSOLE_MONITOR_PLUG_IN;
//#if (EDID_DEBUG_SW)
			printf("EDID: Monitor has plug in\n\r");			
//#endif
		}
		else if (EDID_ctrl.MonitorPrePlugStatus!=0 && EDID_ctrl.MonitorDebouncedPlugStatus==0)
		{
			EDID_ctrl.TaskState = CONSOLE_MONITOR_PLUG_OUT;	
//#if (EDID_DEBUG_SW)		
	  		printf("EDID: Monitor has plug out\n\r");
//#endif
		}
		EDID_ctrl.MonitorPrePlugStatus = EDID_ctrl.MonitorDebouncedPlugStatus;

		/* EDID reload request detection */
		if (EDID_ctrl.EdidReloadRequest)
		{
			EDID_ctrl.EdidReloadRequest--;
			EDID_ctrl.TaskState = CONSOLE_MONITOR_PLUG_IN;	
		}
		
		/* EDID pause handling */
		if (EDID_ctrl.TaskPauseRequest)
		{
			EDID_ctrl.TaskState = CONSOLE_MONITOR_PLUG_IN;	
		}
		break;
		
	case CONSOLE_MONITOR_PLUG_IN:
		/* Disable ADP */
		HW_EDID_PlugAutoDetectMode(0,DISABLE);
		EDID_ctrl.Timer = 50;
		
		EDID_ctrl.EdidReloadRetryCnt = 3;
		EDID_ctrl.TaskState = CONSOLE_MONITOR_RELOAD;
		break;
		
	case CONSOLE_MONITOR_RELOAD:
		if (EDID_ctrl.Timer == 0)/* Waiting for ADP completly done */
		{
			if (EDID_ctrl.TaskPauseRequest)
			{
				EDID_ctrl.TaskState = CONSOLE_MONITOR_PLUG_PAUSED;
	  			printf("EDID: EDID task has been paused!\n\r");							
				break;						
			}
			
			EDID_ctrl.EdidReloadStatus = EDID_RELOAD_BUSY; // reset the edid reload state	
			/* Start EDID reload */
			HW_EDID_ReloadAutoMode();	
			EDID_ctrl.TaskState = CONSOLE_MONITOR_RELOAD_WAIT;
#if (EDID_DEBUG_SW)				
	  		printf("EDID: Start EDID reload...");			
#endif			
		}
		break;

	case CONSOLE_MONITOR_RELOAD_WAIT:
		if (EDID_ctrl.EdidReloadStatus == EDID_RELOAD_OK)/* Auto reload is successfully done */
		{
#if (EDID_DEBUG_SW)
			printf("Console_EDID():Monitor=>[");
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
#ifdef MCU_TYPE_AX68002
	#if (!KVM_2_PORT_HDMI)
			HW_EDID_EnableSlave();
	#endif
#else
			HW_EDID_EnableSlave();
#endif
			/* Enable ADP */
			HW_EDID_PlugAutoDetectMode(0,ENABLE);
			EDID_ctrl.TaskState = CONSOLE_MONITOR_STATUS_CHK;
#if (EDID_DEBUG_SW)
	  		printf("OK!\n\r");			
#endif

#if (SYSTEM_EXTENDER_RECEIVER)
			Externder_Receiver_EDID_PlugOff();
#endif			
			
#ifdef HDMI_KVM
			//Inform the PCs that HDMI monitor has been plug in
			KVM_CONSOLE_HPD_Control(VGA_OFF);	
			KVM_CONSOLE_Vga_Control(KVM_CurrentHost,VGA_ON);			
#endif /* #ifdef HDMI_KVM */

#ifdef EDID_SAVE
			if (memcmp(ConsoleEdidTable,KVM_Flash.Edid_Table,16)) //if the edid table is different
			{
				printf("EDID: Save New Table\n\r");
				memcpy(KVM_Flash.Edid_Table,ConsoleEdidTable,sizeof(KVM_Flash.Edid_Table));
				STORAGE_Write(sizeof(KVM_Flash),(U8_T *)&KVM_Flash);
			}				
#endif
		}
		else if (EDID_ctrl.EdidReloadStatus == EDID_RELOAD_FAIL)/* Auto reload fail */
		{
			if (EDID_ctrl.EdidReloadRetryCnt)
			{
				EDID_ctrl.EdidReloadRetryCnt--;
				EDID_ctrl.TaskState = CONSOLE_MONITOR_RELOAD;
			}
			else
			{
				/* Enable ADP */
				HW_EDID_PlugAutoDetectMode(0,ENABLE);
				EDID_ctrl.TaskState = CONSOLE_MONITOR_STATUS_CHK;			
#if (EDID_DEBUG_SW)
	  			printf("FAIL!\n\r");					
#endif
			}
		}
		break;
		
	case CONSOLE_MONITOR_PLUG_PAUSED:
		if (EDID_ctrl.TaskPauseRequest==0)
		{
#ifdef MCU_TYPE_AX68002
	#if (!KVM_2_PORT_HDMI)
			HW_EDID_EnableSlave();
	#endif
#else
            HW_EDID_EnableSlave();
#endif		

			/* Enable ADP */
			HW_EDID_PlugAutoDetectMode(0,ENABLE);		
			EDID_ctrl.TaskState = CONSOLE_MONITOR_STATUS_CHK;
  			//printf("EDID: EDID task has been restart!\n\r");										
		}
		break;		
	}
}

/*
 *--------------------------------------------------------------------------------
 * void  TASK_EDID_Console_ISR_Handle(ISR_TypeDef *pIsr)
 * Purpose : Handling the interrupt events of EDID interface. 
 * Params  : A pointer to the ISR data structure that contains events related of EDID interface.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void TASK_EDID_Console_ISR_Handle(ISR_TypeDef *pIsr)
{
	U8_T	reg8b;

	if (pIsr->State & I2CMISR_PSC) // plug status changed
	{
		/* Record the plug status */
		EDID_ctrl.MonitorPlugStatus = (pIsr->Data & I2CMSR_DPS_ON) ? MONITOR_PLUGGED:MONITOR_UNPLUGGED;
	}

	if (pIsr->State & I2CMISR_EDID_DONE) // if edid table read has done
	{
		EDID_ctrl.EdidReloadStatus = (pIsr->Data & I2CMSR_EDID_OK) ? EDID_RELOAD_OK:EDID_RELOAD_FAIL;
		if (EDID_ctrl.EdidReloadStatus == EDID_RELOAD_FAIL)
		{
			/* Trigger the reload EDID bit to recovery the bus */
			reg8b = I2CMCR_RLE_EDID;
			I2C_RegWrite(I2CMCR, &reg8b, 1);
		}
	}
}

#endif //(SYSETM_CONSOLE_EDID_CONTROL_ENABLE)

/* End of console_edid.c */
