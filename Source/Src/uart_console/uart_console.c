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
 * Module Name : uart_console.c
 * Purpose     : The UART module driver. It manages the character
 *               buffer and handles the ISR.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include    <ctype.h>
#include	<string.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	"project_include.h"

/* STATIC VARIABLE DECLARATIONS */
#ifdef UART_CONSOLE
#define STAGE_READ_WAIT			0
#define STAGE_READ_HANDLE		1
#define STAGE_SEND				2
#define STAGE_SEND_HANDLE		3
#define STAGE_SEND_WAIT			4
#define STAGE_SEND_WAIT			4

//------------------------------------------
//$ Console Behavior define
//------------------------------------------
#define AUTO_0A				1
#define CHAR_ECHO			1
#define CHAR_TERMINATE1		0x0d
#define CHAR_TERMINATE2		0x0a
#define AUTO_TOUPPER		0
#define AUTO_TOLOWER		0
#define UartStrMaxLength	10
#define UartStrMaxCnt		6

void UartConsole_PortSwitch_Handle(void);
void UartConsole_PowerSavingMode_Handle(void);
void UartConsole_Help_Handle(void);
void UartConsole_Config_Handle(void);

bit		UartConsole_R_Queue = 0;
U8_T    TASK_UART_Console_Receive_Handle_ID;
U8_T    TASK_UART_Console_Receive_Handle_ActiveID;
U8_T	UartConsole_Buf[MAX_RX_UART0_BUF_SIZE];
U16_T	UartConsole_RxHead  = 0;
U16_T	UartConsole_RxTail  = 0;
U16_T	UartConsole_RxCount = 0;
U16_T	UartConsole_P = 0;
U8_T    UartConsole_ProcessID;
U8_T    UartConsole_ProcessStage;
U8_T    UartConsole_MsgLen;
U8_T	UartStr[UartStrMaxCnt][UartStrMaxLength];
U16_T   WordP;
U8_T	WordCnt,WordLen;

UartConsole_Handle_TypeDef  Console_Handle_Tab[]=
{
	0,"PORTSW"	,1,UartConsole_PortSwitch_Handle, //Transmitter Target Screen Resolution Setting	
	0,"PWSM" 	,1,UartConsole_PowerSavingMode_Handle,//Transmitter Port Switch Handle
	0,"HELP"	,0,UartConsole_Help_Handle, //Transmitter Help Screen
	0,"?"		,0,UartConsole_Help_Handle, //Transmitter Help Screen
	0,"CONFIG"	,0,UartConsole_Config_Handle, //Transmitter Help Screen
};

#define UART_CONSOLE_TAB_MAX	(sizeof(Console_Handle_Tab) /  sizeof(UartConsole_Handle_TypeDef))
		
#define CMD_GET_USERNAME		0
#define CMD_GET_PASSWORD		1
#define CMD_SEND_USERNAME		2
#define CMD_SEND_PASSWORD		3
#define CMD_SEND_SSID			4
#define CMD_SEND_CONFIG			5
		
		
void 	TASK_UART_Console_Receive_Handle(void);

/* LOCAL SUBPROGRAM DECLARATIONS */
void UartConsole_StageStart(void);		
U8_T UartConsole_GetNextWord(void);
void UartConsole_Prompt(void);
/* LOCAL SUBPROGRAM BODIES */
/*
 * ----------------------------------------------------------------------------
 * void UartConsole_Recieve_Handle(void)
 * Purpose : UART0 console receive handle 
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void UartConsole_Init(void)
{
	TASK_UART_Console_Receive_Handle_ID = TASK_Create(TASK_UART_Console_Receive_Handle);
	TASK_UART_Console_Receive_Handle_ActiveID = 0;		
	UartConsole_ProcessID = 0;
	UartConsole_StageStart();		
}		

/*
 * ----------------------------------------------------------------------------
 * void UartConsole_StageStart(void)
 * Purpose : start a new uart console event 
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void UartConsole_StageStart(void)
{
	
//UartConsole_StageStart_Loop:		
	UartConsole_RxCount = 0;
	UartConsole_ProcessStage = STAGE_READ_WAIT;		
	
	/*
	UartConsole_MsgLen = (U8_T)strlen(Console_Handle_Tab[UartConsole_ProcessID].WaitMsg);	
	if (Console_Handle_Tab[UartConsole_ProcessID].Operation == 0) //stage read
	{		
		UartConsole_ProcessStage = STAGE_READ_WAIT;		
	}		
	else
	{
		UartConsole_ProcessStage = STAGE_SEND;
		if (Console_Handle_Tab[UartConsole_ProcessID].HandleFunction)
			Console_Handle_Tab[UartConsole_ProcessID].HandleFunction();
		
		if (UartConsole_MsgLen)
		{
			UartConsole_ProcessStage = STAGE_SEND_WAIT;
		}		
		else
		{
			UartConsole_ProcessID++;
			if (UartConsole_ProcessID >= UART_CONSOLE_TAB_MAX)
			{
				UartConsole_ProcessID = 0;
			}
			else
			{
				goto UartConsole_StageStart_Loop;
			}				
		}			
	}
	*/	
	//UartConsole_RxTail = 0;		
	//UartConsole_RxHead = 0;			
}

/*
 * ----------------------------------------------------------------------------
 * void UartConsole_NextStage(void)
 * Purpose : start a new uart console event 
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void UartConsole_NextStage(void)
{
	UartConsole_ProcessID++;
	if (UartConsole_ProcessID >= UART_CONSOLE_TAB_MAX)
	{
		UartConsole_ProcessID = 0;
	}	
	else
	{
		UartConsole_StageStart();
	}			
}

/*
 * ----------------------------------------------------------------------------
 * void UartConsole_Recieve_Handle(void)
 * Purpose : UART0 console receive handle 
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void UartConsole_Recieve_Handle(void)
{	
	bit terminate=0;
	U8_T  cmd_get=0;
	
	while (UartConsole_RxTail != uart0_RxTail)	
	{
		//if (uart0_RxCount != MAX_RX_UART0_BUF_SIZE) 
		{

#if (AUTO_TOUPPER)
			UartConsole_Buf[UartConsole_RxCount]=toupper(uart0_RxBuf[uart0_RxHead]);		
#else
	#if (AUTO_TOLOWER)
			UartConsole_Buf[UartConsole_RxCount]=tolower(uart0_RxBuf[uart0_RxHead]);		
	#else
			UartConsole_Buf[UartConsole_RxCount]=uart0_RxBuf[uart0_RxHead];		
	#endif
#endif						

			if ((UartConsole_Buf[UartConsole_RxCount] == CHAR_TERMINATE1) ||(UartConsole_Buf[UartConsole_RxCount] == CHAR_TERMINATE2))
			{		
				terminate=1;				
#if (CHAR_ECHO) && (AUTO_0A) 							
				printf("\n");			
#endif			
			}	
			
			#if (CHAR_ECHO)
			//printf("%c-%bx",(U8_T)UartConsole_Buf[UartConsole_RxCount],(U8_T)UartConsole_Buf[UartConsole_RxCount]);
			printf("%c",(U8_T)UartConsole_Buf[UartConsole_RxCount]);
			#endif					
			
			UartConsole_RxCount++; //add the receive buffer lenght
			
			if (UartConsole_RxCount >=  MAX_RX_UART0_BUF_SIZE)
				UartConsole_RxCount = 0;
			//----------------------------------------------------------
			//update the ring buffer end pointer for RX ring buffer
			uart0_RxHead++;
			uart0_RxHead &= MAX_RX_UART0_MASK;			
			
			UartConsole_RxTail++;			
			UartConsole_RxTail &= MAX_RX_UART0_MASK;					
			//----------------------------------------------------------
			/*
			if ((UartConsole_ProcessStage == STAGE_READ_WAIT) ||
				(UartConsole_ProcessStage == STAGE_SEND_WAIT))
			{
				if (UartConsole_RxCount == UartConsole_MsgLen)
				{
					if (memcmp(UartConsole_Buf,Console_Handle_Tab[UartConsole_ProcessID].WaitMsg,UartConsole_MsgLen) == 0) //if string is same
					{
						if (UartConsole_ProcessStage == STAGE_READ_WAIT)
						{	
							if (Console_Handle_Tab[UartConsole_ProcessID].HandleFunction)
							{	
								Console_Handle_Tab[UartConsole_ProcessID].HandleFunction();
							}	
							UartConsole_NextStage();							
						}
						else
						{//this is send wait.						
							UartConsole_NextStage();							
						}							
					}	
				}	
			}
			*/	
			if (terminate)
			{									
				WordCnt = 0;	
				WordP = 0;				
				if (UartConsole_GetNextWord())
				{
					for (UartConsole_ProcessID = 0 ; UartConsole_ProcessID < UART_CONSOLE_TAB_MAX ; UartConsole_ProcessID++)
					{
						UartConsole_MsgLen = (U8_T)strlen(Console_Handle_Tab[UartConsole_ProcessID].WaitMsg);	
						if (UartConsole_MsgLen == WordLen)
						{	
						if (memcmp(UartConsole_Buf,Console_Handle_Tab[UartConsole_ProcessID].WaitMsg,WordLen) == 0) //if string is same
						{
							cmd_get = 1;
							if (Console_Handle_Tab[UartConsole_ProcessID].HandleFunction)
							{	
								Console_Handle_Tab[UartConsole_ProcessID].HandleFunction();
							}	
							break;
						}	
					}
					}
					
					if (cmd_get == 0)
					{
						printf("??\n\r");
						UartConsole_Prompt();
					}						
						
				}			
				else
				{
					#if !(CHAR_ECHO)
					printf("\n\r"); //responsed command
					#endif
					UartConsole_Prompt();
				}					
				UartConsole_RxCount = 0;
				
				/*
				if (UartConsole_RxTail != uart0_RxTail)
				{
					UartConsole_R_Queue = 1; //main loop should not process this processing
					if (TASK_UART_Console_Receive_Handle_ActiveID ==0)
					{
						TASK_UART_Console_Receive_Handle_ActiveID = 
							TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_UART_Console_Receive_Handle_ActiveID,0,0,1,1); 
					}						
					break;
				}
				*/				
			}	
		}		
	} /* End of if(RI0) */
}

/*
 * ----------------------------------------------------------------------------
 * void TASK_UART_Console_Receive(void)
 * Purpose : UART0 console receive handle Task
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void TASK_UART_Console_Receive_Handle(void)
{
	TASK_Destory_Current();
	UartConsole_R_Queue = 0; //main loop should not to process this processing		
	TASK_UART_Console_Receive_Handle_ActiveID = 0;
	UartConsole_Recieve_Handle();	
}

/*
 * ----------------------------------------------------------------------------
 * void UartConsole_SetSSID_Handle(void)
 * Purpose : send out set ssid command
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
U8_T UartConsole_GetNextWord(void)
{
	U16_T startp;
	
	
	startp = WordP;
	for (; WordP < UartConsole_RxCount ; WordP++)
	{
		if ((UartConsole_Buf[WordP] == ' ') || (UartConsole_Buf[WordP] == CHAR_TERMINATE1) ||  (UartConsole_Buf[WordP] == CHAR_TERMINATE2))
		{
			if (WordP != startp) //skip the space
			{				
				WordLen = WordP - startp;				
				//Copy String
				if(WordCnt < (UartStrMaxCnt-1))
				{
					UartStr[WordCnt][0] = WordLen; //keep word lenght
					if (WordLen < UartStrMaxLength)						
						memcpy(&UartStr[WordCnt][1],&UartConsole_Buf[startp],WordLen); //copy word content
					else
						memcpy(&UartStr[WordCnt][1],&UartConsole_Buf[startp],UartStrMaxLength); //copy word content
					UartStr[WordCnt][WordLen+1] = 0; //word terminate					
				}			
				WordCnt++;
				return 1;
			}				
			else
			{
				if (UartConsole_Buf[WordP] == ' ')
				{
					startp++; //move the start pointer to next byte
				}					
			}				
		}					
	}
	return 0;
}



//-------------------------------------------------------------------------------------------------------------
//
//  UART  COMMAND HANDLE SECTION
//
//-------------------------------------------------------------------------------------------------------------
void UartConsole_Ack(void)
{
	printf(">OK\n\r"); //responsed command
	UartConsole_Prompt();
}

void UartConsole_Err(void)
{
	printf(">ERROR\n\r"); //responsed command
	UartConsole_Prompt();
}

void UartConsole_Prompt(void)
{
	printf(">"); //responsed command
}

U8_T UartConsole_GetParamter(U8_T para_cnt)
{
	U8_T wordcnt;
	
	for (wordcnt=0; wordcnt < para_cnt ; wordcnt++)
	{
		if (UartConsole_GetNextWord() == 0)
		{
			break;
		}				
	}
	
	return wordcnt;
}	

/*
 * ----------------------------------------------------------------------------
 * void UartConsole_Help_Handle(void)
 * Purpose : Transmitter Screen Mode Setting
 * Params  : none
 * Returns : none
 * Note    : Mode 0 1/2- 2 Screen with 1 2
 *			 Mode 1 
 * ----------------------------------------------------------------------------
 */
void UartConsole_Help_Handle(void)
{
	printf(">Help screen\n\r");
	printf(" [1].portsw 1/4\n\r");
	printf("     port switch control\n\r");
	printf("     1/4-port id\n\r");	
	printf(" [2].pwsm 2/3\n\r");
	printf("     power saving mode\n\r");
	printf("     2-stop mode\n\r");
	printf("     3-sleep mode\n\r");	
	UartConsole_Prompt();
}

/*
 * ----------------------------------------------------------------------------
 * void UartConsole_Config_Handle(void)
 * Purpose : Transmitter Screen Mode Setting
 * Params  : none
 * Returns : none
 * Note    : Mode 0 1/2- 2 Screen with 1 2
 *			 Mode 1 
 * ----------------------------------------------------------------------------
 */
void UartConsole_Config_Handle(void)
{	 
	UartConsole_Prompt();
}

/*
 * ----------------------------------------------------------------------------
 * void UartConsole_PortSwitch_Handle(void)
 * Purpose : port switch handle
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void UartConsole_PortSwitch_Handle(void)
{
	U8_T wordcnt;
	U8_T pid;
	
	printf("Port Switch Handle\n\r");	
	wordcnt=UartConsole_GetParamter(1);
		
	if (wordcnt == 0)
	{
		UartConsole_Ack();		
		KVM_Jump_Next_Power_Port(KVM_CurrentHost);
	}		
	else
	{
		pid = atoi(&UartStr[1][1]);
		if ((pid <= KVM_MAX_PORT) && (pid > 0))
		{	
			UartConsole_Ack();		
			KVM_NextHost = pid-1;
			KVM_Console_Port_Jump(KVM_NextHost);
		}	
		else
		{
			UartConsole_Err();		
		}			
	}			
}	

/*
 * ----------------------------------------------------------------------------
 * void UartConsole_PowerSavingMode_Handle(void)
 * Purpose : Power Saving handle
 *			 SYSTEM_POWER_STOP_MODE = 2
 *			 SYSTEM_POWER_SLEEP_MODE= 3
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void UartConsole_PowerSavingMode_Handle(void)
{
	U8_T wordcnt;	
	U8_T  pid;
	
	printf("Power Saving Mode Handle\n\r");	
	wordcnt=UartConsole_GetParamter(1);
	
	if (wordcnt == 0)
	{
		UartConsole_Err();		
	}		
	else
	{
		pid = atoi(&UartStr[1][1]);
		if (pid == SYSTEM_POWER_STOP_MODE)
		{
			KVM_Flash.PowerSavingMode = SYSTEM_POWER_STOP_MODE; 	// Stop Mode
		}
		else if (pid == SYSTEM_POWER_SLEEP_MODE)
		{	
			KVM_Flash.PowerSavingMode = SYSTEM_POWER_SLEEP_MODE; 	// Deep Sleep Mode
		}	
		else			
		{
			UartConsole_Err();
			return;
		}
		UartConsole_Ack();		
	}			
}

#endif  /* #ifdef UART_CONSOLE */
/* End of uart_console.c */

