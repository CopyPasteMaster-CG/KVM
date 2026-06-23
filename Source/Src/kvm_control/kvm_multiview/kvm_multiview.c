/*
 ******************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is an proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
 /*============================================================================
 * Module Name: vs4210.c
 * Purpose: MULTIVIEW contorl & handle 
 * Author:
 * Date:
 * Modify :
 *=============================================================================
 */

/* INCLUDE FILE SECTION */
#include <stdio.h>
#include <string.h>
#include "project_include.h"

#ifdef MULTIVIEW //USE MULTIVIEW Chip
/* NAMING CONSTANT DECLARATIONS */
#define  MULTIVIEW_HEADER0			0xaa
#define  MULTIVIEW_HEADER1			0x55

#define  MULTIVIEW_INIT_DONE			0x01
#define  MULTIVIEW_ACK					0x02
#define  MULTIVIEW_CURRENT_VALUE		0x03
#define  MULTIVIEW_DISPLAY_MODE		0x04
#define  MULTIVIEW_RESOLUTION_SELECT	0x05
#define  MULTIVIEW_HDMI_SELECT			0x06
#define  MULTIVIEW_AUDIO_SELECT		0x07

/* GLOBAL VARIABLES DECLARATIONS */
bit	   MULTIVIEW_TimeOut_Pause=0;
bit	   MULTIVIEW_TimeOut_Flag=0;
U8_T   HDMI_Port[4] = {0,1,2,3};		
U8_T   MULTIVIEW_RxBuf[16];
U8_T   MULTIVIEW_Wait_Len;
U8_T   TASK_MULTIVIEW_Receive_TimeOut_ID;
U8_T   TASK_MULTIVIEW_Receive_TimeOut_ActiveID;
U16_T  MULTIVIEW_RxTail = 0;
U16_T  MULTIVIEW_RxCount = 0;
U16_T  MULTIVIEW_RxTail_Hold=0;

U8_T MULTIVIEW_Cmd[7][8] =
{
	// 1    2    3    4    5    6    7    8 
	0xaa,0x55,0x06,MULTIVIEW_INIT_DONE			,0xcc,0x33,0x00,0x00,  //0x01-VS4201 init done	
	0xaa,0x55,0x06,MULTIVIEW_ACK				,0xcc,0x33,0x00,0x00,  //0x02-ACK
	0xaa,0x55,0x07,MULTIVIEW_CURRENT_VALUE		,0x01,0x00,0x00,0x00,  //0x03-Current Parameter
	0xaa,0x55,0x05,MULTIVIEW_DISPLAY_MODE		,0x01,0x00,0x00,0x00,  //0x04-Display Mode select,0-Single,1-Quad
	0xaa,0x55,0x05,MULTIVIEW_RESOLUTION_SELECT	,0x01,0x00,0x00,0x00,  //0x05-Resolution select,0-1080P,1-720P
	0xaa,0x55,0x05,MULTIVIEW_HDMI_SELECT		,0x01,0x00,0x00,0x00,  //0x06-HDMI Port Select	
	0xaa,0x55,0x05,MULTIVIEW_AUDIO_SELECT		,0x01,0x00,0x00,0x00,  //0x07-Audio Port Select	
};

/* LOCAL VARIABLES DECLARATIONS */
/* LOCAL SUBPROGRAM DECLARATIONS */
void TASK_MULTIVIEW_Receive_TimeOut_Start(void);
void TASK_MULTIVIEW_Receive_TimeOut(void);
void MULTIVIEW_Send_Command(U8_T *buf, U8_T len);
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
/* EXTERNAL SUBPROGRAM DECLARATIONS */
/*
 * ----------------------------------------------------------------------------
 * Function Name: MULTIVIEW_Init(void)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MULTIVIEW_Init(void)
{
	#ifdef HSUART
	HSUART_Init();
	#endif	
	MULTIVIEW_RxCount = 0;	
	TASK_MULTIVIEW_Receive_TimeOut_ID = TASK_Create(TASK_MULTIVIEW_Receive_TimeOut);	
	TASK_MULTIVIEW_Receive_TimeOut_ActiveID = 0;
} 
 
/**----------------------------------------------------------------------------
 * void MULTIVIEW_Receive_Handle(void)
 * Purpose : UART1 console receive handle 
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
void MULTIVIEW_Receive_Handle(void)
{	
	bit terminate=0;
	U8_T  cmd_get=0;
	
	printf("(%d-%d)\n\r",MULTIVIEW_RxTail,hsuart_RxTail);			
	if (MULTIVIEW_RxTail != hsuart_RxTail)	
	{					
		//TASK_MULTIVIEW_Receive_TimeOut_Start();
		printf("|%d-%02bx|",MULTIVIEW_RxCount,hsuart_RxBuf[hsuart_RxHead]);		
		MULTIVIEW_TimeOut_Pause = 0;		
		TASK_MULTIVIEW_Receive_TimeOut_Start();
		MULTIVIEW_RxBuf[MULTIVIEW_RxCount]=hsuart_RxBuf[hsuart_RxHead];
					 			
		MULTIVIEW_RxCount++;		
		//----------------------------------------------------------
		//update the ring buffer end pointer for RX ring buffer
//MULTIVIEW_Recieve_Exit:		
		hsuart_RxHead++;
		hsuart_RxHead &= MAX_RX_HSUART_MASK;						
		MULTIVIEW_RxTail++;			
		MULTIVIEW_RxTail &= MAX_RX_HSUART_MASK;															
	} /* End of if(RI0) */
}

/**----------------------------------------------------------------------------
 * void TASK_MULTIVIEW_Receive_TimeOut_Start(void)
 * Purpose : 
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
void TASK_MULTIVIEW_Receive_TimeOut_Start(void)
{
	if (TASK_MULTIVIEW_Receive_TimeOut_ActiveID == 0)
	{
		TASK_MULTIVIEW_Receive_TimeOut_ActiveID =
			TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_MULTIVIEW_Receive_TimeOut_ID,0,0,500,500)+1;		
	}		
	else
	{
		Task_Active_Table[TASK_MULTIVIEW_Receive_TimeOut_ActiveID-1].Task_Interval.w = 500; //200ms
	}		
}

/**----------------------------------------------------------------------------
 * void TASK_MULTIVIEW_Receive_TimeOut(void)
 * Purpose : 
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
void TASK_MULTIVIEW_Receive_TimeOut(void)
{
	if (MULTIVIEW_TimeOut_Pause == 0)
	{
		MULTIVIEW_TimeOut_Flag=1;
		//if (MULTIVIEW_RxCount != 0)
		//{
		//	printf("TClear_Rx:%d\n\r",MULTIVIEW_RxCount);
		//}	
		MULTIVIEW_RxCount = 0;
	}	
	
	TASK_Destory_Current();
	TASK_MULTIVIEW_Receive_TimeOut_ActiveID = 0;
}

/**----------------------------------------------------------------------------
 * void MULTIVIEW_Send_Command(U8_T *buf, U8_T len)
 * Purpose : 
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
void MULTIVIEW_Send_Command(U8_T *buf, U8_T len)
{
	U8_T i;
			
	for (i=0 ; i < len;i++)
	{		
		HSUART_PutChar(buf[i]);
	}		
}	
#endif /*  #ifdef MULTIVIEW  */

/* End of buzzer_hardware.c */
