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

static KVM_RX_DATA kvm_rx;
#define MULTIVIEW_UART_DEBUG 1

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
	kvm_rx.state = KVM_RX_HEAD1;
	kvm_rx.cmd = 0;
	kvm_rx.mode = 0;
	MULTIVIEW_RxCount = 0;	
	MULTIVIEW_RxTail = uart1_RxHead;
#if MULTIVIEW_UART_DEBUG
	printf("MV init head=%u tail=%u\r\n", uart1_RxHead, uart1_RxTail);
#endif
	TASK_MULTIVIEW_Receive_TimeOut_ID = TASK_Create(TASK_MULTIVIEW_Receive_TimeOut);	
	TASK_MULTIVIEW_Receive_TimeOut_ActiveID = 0;
} 
 
U8_T KVM_CRC8_Calculate(const U8_T *crcdata, U8_T length)
{
    U8_T crc = 0x00;
    U8_T i;

    while (length--)
    {
        crc ^= *crcdata++;

        for (i = 0; i < 8; i++)
        {
            if (crc & 0x80)
            {
                crc = (U8_T)((crc << 1) ^ 0x07);
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}


void KVM_SET_mode(U8_T mode){

    switch (mode)
    {
    case KVM_MODE_PORT1_ONLY:
        API_Set_Roaming_Mode(API_ROAMING_DISABLE);
        API_Set_Sync_Mode(API_SYNC_DISABLE);
        KVM_Console_Port_Jump(3);
        break;
    case KVM_MODE_PORT2_ONLY:
        API_Set_Roaming_Mode(API_ROAMING_DISABLE);
        API_Set_Sync_Mode(API_SYNC_DISABLE);
        KVM_Console_Port_Jump(2);
       
        break;
    case KVM_MODE_PORT3_ONLY:
        API_Set_Roaming_Mode(API_ROAMING_DISABLE);
        API_Set_Sync_Mode(API_SYNC_DISABLE);
        KVM_Console_Port_Jump(1);
        break;
    case KVM_MODE_PORT4_ONLY:
        API_Set_Roaming_Mode(API_ROAMING_DISABLE);
        API_Set_Sync_Mode(API_SYNC_DISABLE);
        KVM_Console_Port_Jump(0);
            break;
    case KVM_MODE_TWO_SMALL_MAIN_PORT1:
        API_SET_ONE_2_SCREEN_MODE(1);
        API_Current_Main_SCREEN = 3;
        KVM_Console_Port_Jump(3);
            break;
    case KVM_MODE_TWO_SMALL_MAIN_PORT2:
        API_SET_ONE_2_SCREEN_MODE(2);
        API_Current_Main_SCREEN = 2;
        KVM_Console_Port_Jump(2);
            break;
    case KVM_MODE_TWO_SMALL_MAIN_PORT3:
        API_SET_ONE_2_SCREEN_MODE(3);
        API_Current_Main_SCREEN = 1;
        KVM_Console_Port_Jump(1);
            break;
    case KVM_MODE_PORT12_ONLY:
        API_Set_Roaming_Mode(API_ROAMING_ENABLE);					
        API_Active_Roaming_Mapping(TWO_SCREEN_MODE);
        KVM_Console_Port_Jump(3);
            break;
    case KVM_MODE_PORT34_ONLY:
        API_Set_Roaming_Mode(API_ROAMING_ENABLE);					
        API_Active_Roaming_Mapping(TWO_SCREEN_MODE);
        KVM_Console_Port_Jump(1);
            break;
    case KVM_MODE_PORT_ALL:
        API_Set_Roaming_Mode(API_ROAMING_ENABLE);					
        API_Active_Roaming_Mapping(QUAD_SCREEN_MODE);
        KVM_Console_Port_Jump(3);
            break;
    case KVM_MODE_PORT_ALL_SYNC:
        API_Set_Roaming_Mode(API_ROAMING_DISABLE);					
        API_Active_Roaming_Mapping(QUAD_SCREEN_MODE);
        API_Set_Sync_Mode(API_SYNC_ENABLE);
            break;
    case KVM_MODE_THREE_SMALL_MAIN_PORT1:
        API_SET_ONE_3_SCREEN_MODE(0);
        API_Current_Main_SCREEN = 3;
        KVM_Console_Port_Jump(3);
            break;
    case KVM_MODE_THREE_SMALL_MAIN_PORT2:
        API_SET_ONE_3_SCREEN_MODE(1);
        API_Current_Main_SCREEN = 2;
        KVM_Console_Port_Jump(2);
            break;
    case KVM_MODE_THREE_SMALL_MAIN_PORT3:
        API_SET_ONE_3_SCREEN_MODE(2);
        API_Current_Main_SCREEN = 1;
        KVM_Console_Port_Jump(1);
            break;
    case KVM_MODE_THREE_SMALL_MAIN_PORT4:
        API_SET_ONE_3_SCREEN_MODE(3);
        API_Current_Main_SCREEN = 0;
        KVM_Console_Port_Jump(0);        
            break;

    default:
        break;
    }
}




void KVM_UART_HandleFrame(U8_T cmd, U8_T mode)
{
    switch (cmd)
    {
        case KVM_CMD_SET_MODE:
        {
            if ((mode >= 0x01) && (mode <= 0x0F))
            {
                //接收并设置模式
                KVM_SET_mode(mode);
				printf("RX_mode\r\n");		
               
            }
            break;
        }

        case KVM_CMD_GET_MODE:
        {
            break;
        }

        default:
        {
            break;
        }
    }
}


void KVM_UART_ReceiveByte(U8_T byte)
{
    U8_T crc_data[2];
    U8_T crc;

    switch (kvm_rx.state)
    {
        case KVM_RX_HEAD1:
        {
            if (byte == 0x55)
            {
                kvm_rx.state = KVM_RX_HEAD2;
            }
            break;
        }

        case KVM_RX_HEAD2:
        {
            if (byte == 0xAA)
            {
                kvm_rx.state = KVM_RX_CMD;
            }
            else if (byte == 0x55)
            {
                kvm_rx.state = KVM_RX_HEAD2;
            }
            else
            {
                kvm_rx.state = KVM_RX_HEAD1;
            }
            break;
        }

        case KVM_RX_CMD:
        {
            kvm_rx.cmd = byte;
            kvm_rx.state = KVM_RX_MODE;
            break;
        }

        case KVM_RX_MODE:
        {
            kvm_rx.mode = byte;
            kvm_rx.state = KVM_RX_CRC;
            break;
        }

        case KVM_RX_CRC:
        {
            crc_data[0] = kvm_rx.cmd;
            crc_data[1] = kvm_rx.mode;

            crc = KVM_CRC8_Calculate(crc_data, 2);

            if (crc == byte)
            {
#if MULTIVIEW_UART_DEBUG
                printf("MV frame cmd=%02bx mode=%02bx crc=%02bx\r\n", kvm_rx.cmd, kvm_rx.mode, byte);
#endif
                KVM_UART_HandleFrame(kvm_rx.cmd, kvm_rx.mode);
            }
#if MULTIVIEW_UART_DEBUG
            else
            {
                printf("MV crc err cmd=%02bx mode=%02bx rx=%02bx cal=%02bx\r\n", kvm_rx.cmd, kvm_rx.mode, byte, crc);
            }
#endif

            kvm_rx.state = KVM_RX_HEAD1;
            break;
        }

        default:
        {
            kvm_rx.state = KVM_RX_HEAD1;
            break;
        }
    }
}


/**----------------------------------------------------------------------------
 * void MULTIVIEW_Receive_Handle(void)
 * Purpose : UART1 receive handle
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/
void MULTIVIEW_Receive_Handle(void)
{	
	U8_T uart_data;

	while (MULTIVIEW_RxTail != uart1_RxTail)
	{					
		uart_data = uart1_RxBuf[uart1_RxHead];
#if MULTIVIEW_UART_DEBUG
		printf("MV rx=%02bx head=%u tail=%u\r\n", uart_data, uart1_RxHead, uart1_RxTail);
#endif
		MULTIVIEW_TimeOut_Pause = 0;		
		TASK_MULTIVIEW_Receive_TimeOut_Start();

		if (MULTIVIEW_RxCount < sizeof(MULTIVIEW_RxBuf))
		{
			MULTIVIEW_RxBuf[MULTIVIEW_RxCount] = uart_data;
			KVM_UART_ReceiveByte(uart_data);
			MULTIVIEW_RxCount++;
		}
		else
		{
			MULTIVIEW_RxCount = 0;
			MULTIVIEW_TimeOut_Flag = 1;
		}

		uart1_RxHead++;
		uart1_RxHead &= MAX_RX_UART1_MASK;						
		MULTIVIEW_RxTail++;			
		MULTIVIEW_RxTail &= MAX_RX_UART1_MASK;												
	}
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
			TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_MULTIVIEW_Receive_TimeOut_ID,0,0,200,200)+1;		
	}		
	else
	{
		Task_Active_Table[TASK_MULTIVIEW_Receive_TimeOut_ActiveID-1].Task_Interval.w = 200; //200ms
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
