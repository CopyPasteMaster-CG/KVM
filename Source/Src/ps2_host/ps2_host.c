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
/*================================================================================
 * Module Name : ps2_host.c
 * Purpose     : use PS/2 protocol to read PS/2 input data, this will use INT0,INT1 
 * Author      : Jack Wang
 * Date        :
 * Notes       : None
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include <stdio.h>
#include <string.h>
#include "project_include.h"

#if (SYSTEM_PS2_HOST_ENABLE) && (SYSTEM_HARDWARE_PS2_ENABLE)
/* GLOBAL DEFINATION DECLARATIONS */
/* GLOBAL CONTANT DECLARATIONS */
code U8_T					PS2_PORT_BIT[] ={BIT0,BIT1,BIT2,BIT3,BIT4,BIT5,BIT6,BIT7};
code PS2_CfgScriptStruct	PS2_CfgScriptTypeCheck[1] = {0xf2,3,0,0};
code PS2_CfgScriptStruct	PS2_CfgScriptKB[4] = 
{
							0xff,2,0,0,// 01-reset the device, answer 0xfa,0xaa
							0xf5,1,0,0,// 02-disable device first
							0xf6,1,0,0,// 03-default keyboard setting 
							0xf4,1,0,0 // 04-enable device
};

code PS2_CfgScriptStruct	PS2_CfgScriptMS[13] = 
{
							0xff,3,0,0,// 01-reset the device, answer 0xfa,0xaa,0x00(for mouse)
							0xf3,1,0xc8,1,// 02-get mouse type
							0xf3,1,0x64,1,// 03-get mouse type
							0xf3,1,0x50,1,// 04-get mouse type
							0xf3,1,0xc8,1,// 05-get mouse type
							0xf3,1,0xc8,1,// 06-get mouse type
							0xf3,1,0x50,1,// 07-get mouse type
							0xf2,2,0,0,// 08-ask mouse type
							0xe8,1,0x03,1,// 09-set resolution
							0xe6,1,0,0,// 10-set scale
							0xf3,1,0x64,1,// 11-set data rate to 100
							0xf4,1,0,0,// 12-enable device
							0xf2,2,0,0// 13-ask mouse type again for keeping
};

PS2_CfgScriptStruct			PS2_cfgScriptKB_F3[1] = {0xf3,1,0,1}; // 01-set F3
PS2_CfgScriptStruct			PS2_cfgScriptKB_Led[3] = 
{
							0xf5,1,0,0, // 01-set Led
							0xed,1,0,1, // 01-set Led
							0xf4,1,0,0 // 01-set Led
};
PS2_CfgScriptStruct			PS2_cfgScriptResend[1] = {0xfe,1,0,0}; // 01-Send out FE
PS2_CfgScriptStruct			PS2_cfgScriptEnable[1] = {0xf4,1,0,0}; // 01-Send out F4
PS2_CfgScriptStruct			PS2_cfgScriptDisable[1] = {0xf5,1,0,0}; // 01-Send out F5


/* GLOBAL VARIABLES DECLARATIONS */ 
/* PS/2 Control relative variable */
volatile U8_T	PS2_Clock_Cnt[PS2_PORT_MAX_NUM]; // PS2 port clock counter
#if (SYSTEM_EXTENDER_SUPPORT)
volatile U16_T	PS2_Status[PS2_PORT_MAX_NUM+2]; // PS2 Port Status
#else
volatile U16_T	PS2_Status[PS2_PORT_MAX_NUM]; // PS2 Port Status
#endif
volatile U8_T	PS2_Control[PS2_PORT_MAX_NUM]; // PS2 B port time out counter,unit ms
volatile U8_T	PS2_TASK_Timeout_ID[PS2_PORT_MAX_NUM]; 		// contain the ps2 time out task ID (IN Period_MS Table)
/* Script Control relative variable */
bit		PS2_CMD_RESEND_FLAG = 0;
U8_T	PS2_Mouse_Type[PS2_PORT_MAX_NUM]; // if the port is mouse, store the mouse type information
U8_T	PS2_Receive_Rp[PS2_PORT_MAX_NUM]; // the receive counter
U8_T	PS2_Receive_Wp[PS2_PORT_MAX_NUM]; // the receive counter
U8_T	PS2_KB_Balance_Cnt[PS2_PORT_MAX_NUM]; // the receive counter
U8_T	PS2_KB_Balance_CountDown[PS2_PORT_MAX_NUM]; // the receive counter
U8_T	PS2_KB_Balance_CountDownTask; // the receive counter
U8_T	PS2_KB_Last_Code[PS2_PORT_MAX_NUM];
U8_T	PS2_KB_Last_Type[PS2_PORT_MAX_NUM];
U8_T	PS2_Receive_FIFO_Queue[PS2_PORT_MAX_NUM][PS2_FIFO_QUEUE_MAX]; // Store the ps2 intput into Q
U8_T	PS2_Transmit_Buf[PS2_PORT_MAX_NUM]; // PS2 Transmit Buffer
U8_T	PS2_Start_Init_Flag; // for eache bit stand for a single port,after start init, this bit should set to low. 
U8_T	PS2_Start_Init_Index; // indicate which port is under start initial
/* Script Control relative variable */
PS2_CfgCtlStruct	PS2_CfgCtlTab[PS2_PORT_MAX_NUM]; // config control table
U8_T	PS2_Script_Receive_Queue[PS2_PORT_MAX_NUM][PS2_FIFO_QUEUE_MAX]; // Store the script task receive data
U8_T	PS2_Script_Receive_Index[PS2_PORT_MAX_NUM]; // Store the script task receive data buf index
/* Keyboard LED & F3 setting */
U8_T	PS2_F3_Setging = 0x4b; // PS2 keyboard delay & repeat setting
#if (SYSTEM_EXTENDER_SUPPORT)
/* PS/2 Mouse Device Handle Buffer */
U8_T	PS2_Mouse_Package_Index[PS2_PORT_MAX_NUM+2]; // Store the mouse device package index
U8_T	PS2_Mouse_Package[PS2_PORT_MAX_NUM+2][4]; // Store the mouse device package Buffer, each mouse device has 4 byte buffer
/* PS/2 Keyboard Device Handle Buffer */
U8_T	PS2_Keyboard_Package[PS2_PORT_MAX_NUM+2][5]; // Store the keyboard device package Buffer, each mouse device has 4 byte buffer
U8_T	PS2_Keyboard_Package_Index[PS2_PORT_MAX_NUM+2]; // Store the keyboard device package index
#else
/* PS/2 Mouse Device Handle Buffer */
U8_T	PS2_Mouse_Package_Index[PS2_PORT_MAX_NUM]; // Store the mouse device package index
U8_T	PS2_Mouse_Package[PS2_PORT_MAX_NUM][4]; // Store the mouse device package Buffer, each mouse device has 4 byte buffer
/* PS/2 Keyboard Device Handle Buffer */
U8_T	PS2_Keyboard_Package[PS2_PORT_MAX_NUM][5]; // Store the keyboard device package Buffer, each mouse device has 4 byte buffer
U8_T	PS2_Keyboard_Package_Index[PS2_PORT_MAX_NUM]; // Store the keyboard device package index
#endif
/* PS/2 Data Output Buffer */
U8_T	PS2_Keyboard_Tx_Control; // 0x01-Disable
U8_T	TASK_PS2_HOST_START_INITLIAL_ID;
U8_T	TASK_PS2_TIMEOUT_CHECK_ID;
U8_T	TASK_EVENT_PS2_HANDLE_ID;
U8_T	TASK_PS2_Balance_Maintain_ID;
U8_T	TASK_PS2_Device_Resend_ID;
U8_T	HPS2_ISR_FIFO_WP,HPS2_ISR_FIFO_RP;
PS2_ISR_TypeDef		HPS2_ISR_FIFO[HPS2_ISR_FIFO_DEPTH_MAX];

/* STATIC VARIABLE DECLARATIONS */
/* LOCAL SUBPROGRAM DECLARATIONS */
void TASK_PS2_Host_Start_Initlial(void);
void TASK_PS2_TimeOut_Active_Check(PS2Port_TypeDef PS2x,U16_T timeset);
void TASK_PS2_Balance_Maintain(void);
void TASK_PS2_Device_Resend(void);
U8_T PS2_Script_Cmd_Resend_Handle(PS2Port_TypeDef PS2x,U8_T flowcontrol,U8_T resendcnt,U8_T resendcmd);

/* EXTERNAL GLOBAL VARIABLE DECLARATIONS  */
extern bit	TASK_KVM_CONSOLE_PS2_KBLed_Control_Flag;

/*
 * -------------------------------------------------------------------------------
 * void    PS2_Init(void)
 * Purpose : Initializes the PS/2 control hardware ps/2 or external interrupt ps/2
 *       
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */
void PS2_Init(void)
{
	U8_T	index;

	/*1.Varable Init */
	for (index=EPS2A; index < (PS2_PORT_MAX_NUM); index++)
	{
		PS2_Status[index] = PS2_IDLE;
		PS2_Control[index] = PS2_NONE;
		PS2_Clock_Cnt[index] = 0;
		PS2_Receive_Rp[index] = 0;
		PS2_Receive_Wp[index] = 0;
		PS2_Keyboard_Package_Index[index]= 0; // reset keybard buffer index.
		PS2_Mouse_Package_Index[index] = 0; // reset mouse buffer index.
		PS2_Mouse_Package[index][0] = 0;
		PS2_Mouse_Package[index][1] = 0;
		PS2_Mouse_Package[index][2] = 0;
		PS2_Mouse_Package[index][3] = 0;
		PS2_KB_Balance_Cnt[index] = 0; // the receive counter
		PS2_KB_Last_Code[index] = 0;
		PS2_KB_Last_Type[index] = 0;
	}
#if (SYSTEM_EXTENDER_SUPPORT)
	PS2_Keyboard_Package_Index[PS2_PORT_MAX_NUM] = 0;
	PS2_Mouse_Package_Index[PS2_PORT_MAX_NUM]=0;
	PS2_Keyboard_Package_Index[PS2_PORT_MAX_NUM+1] = 0;
	PS2_Mouse_Package_Index[PS2_PORT_MAX_NUM+1]=0;
#endif
	PS2_KB_Balance_CountDownTask = 0;
	PS2_Keyboard_Tx_Control = 0;
	HPS2_ISR_FIFO_WP = 0;
	HPS2_ISR_FIFO_RP = 0;
	//2.Check Hardware PS/2 Control funcion setting
#if SYSTEM_HARDWARE_PS2_ENABLE	  
	HW_PS2_Init();
#endif
	//3.Check External Interrupt PS/2 Control funcion setting
	// Now start to initial the port
	PS2_Start_Init_Flag = 0xff >> (8-(PS2_PORT_MAX_NUM)); // set the init flag
	PS2_Start_Init_Index = 0;
	// Task Initial Start
	TASK_PS2_HOST_START_INITLIAL_ID = TASK_Create(TASK_PS2_Host_Start_Initlial);
	TASK_EVENT_PS2_HANDLE_ID = TASK_Create(TASK_Event_PS2_Handle);
	TASK_PS2_Balance_Maintain_ID = TASK_Create(TASK_PS2_Balance_Maintain);
	TASK_PS2_Device_Resend_ID = TASK_Create(TASK_PS2_Device_Resend);
	TASK_PS2_TIMEOUT_CHECK_ID = TASK_Create(TASK_PS2_TimeOut_Check);
	TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_PS2_HOST_START_INITLIAL_ID,0,0,500,500); // start to init PS2 device

#if (SYSTEM_TASK_DUMP_SUPPORT)
//	printf("TASK_PS2_HOST_START_INITLIAL_ID=%bu\n\r",TASK_PS2_HOST_START_INITLIAL_ID);
//	printf("TASK_EVENT_PS2_HANDLE_ID=%bu\n\r",TASK_EVENT_PS2_HANDLE_ID);
//	printf("TASK_PS2_Balance_Maintain_ID=%bu\n\r",TASK_PS2_Balance_Maintain_ID);
//	printf("TASK_PS2_Device_Resend_ID=%bu\n\r",TASK_PS2_Device_Resend_ID);
//	printf("TASK_PS2_TIMEOUT_CHECK_ID=%bu\n\r",TASK_PS2_TIMEOUT_CHECK_ID);
#endif	
} /* End of PS2_Init */

/*
 * ----------------------------------------------------------------------------
 * void TASK_PS2_TimeOut_Check
 * Purpose : Check the time out condition on transmitte or receive 
 *           includ hardware ps/2 & external interrupt ps/2
 * Params  : NONE
 * Returns : NONE
 * Note    : 
 * ----------------------------------------------------------------------------
 */ 
void TASK_PS2_TimeOut_Check(void)
{
	if (PS2_Status[TASK_Register0] & PS2_ACT) // if the port still active
	{
		if (PS2_Status[TASK_Register0] & PS2_WRITE_FLAG) // in transmitt status
		{
#if (PS2_DEBUG_MODE == 1)
			printf("@P:%bu,Transmitte TimeOut!!\n\r",TASK_Register0);
#endif
			PS2_Status[TASK_Register0] |= PS2_TIME_OUT_FLAG;
		}
		else
		{
#if (PS2_DEBUG_MODE == 1)
			printf("@P:%bu,Receive TimeOut!!\n\r",TASK_Register0);
#endif
			PS2_Status[TASK_Register0] |= PS2_TIME_OUT_FLAG;
		}
		TASK_Active(TASK_TYPE_EVENT,TASK_EVENT_PS2_HANDLE_ID,0,TASK_Register0,0,0); // now aticev the process for time out event
	}
	TASK_Destory_Current();
} /* End of PS2_TimeOut_Check */

/*
 * ----------------------------------------------------------------------------
 * void      PS2_Script_Assing_Cmd_Para
 * Purpose : assign the cmd or para to script transmit buffer
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *            this parameter can be one of the following values:
 *            HPS2A, HPS2B=>hardware ps/2 host
 *            EPS2A, EPS2B=>external interrupt
 *           @Scp_Type: CMD or PARA1
 *            this parameter can be one of the following values:
 *            Script_Cmd  : Command byte
 *            Script_Para1: Parameter1 byte
 * Returns : The received data.(include the address)
 * Note    : This function should place on 1ms task in main function
 * ----------------------------------------------------------------------------
 */ 
void PS2_Script_Assing_Cmd_Para(PS2Port_TypeDef PS2x,PS2ScriptTransmit_TypeDef Scp_Type)
{
	if (Scp_Type == Script_Cmd) // this is command byte
	{
		PS2_CfgCtlTab[PS2x].FlowControl = PS2_SCP_CMD;
		PS2_Transmit_Buf[PS2x] =((PS2_CfgScriptStructP)(PS2_CfgCtlTab[PS2x].ScriptPtr))->Cmd; // move command to output buffer
		if (PS2_CMD_RESEND_FLAG)
		{
			PS2_CMD_RESEND_FLAG = 0;
		}
	}
	else if (Scp_Type == Script_Para1)
	{
		PS2_CfgCtlTab[PS2x].FlowControl = PS2_SCP_PARA1;
		PS2_Transmit_Buf[PS2x] =((PS2_CfgScriptStructP)(PS2_CfgCtlTab[PS2x].ScriptPtr))->Parameter1; // move para1 to output buffer
		if (PS2_CMD_RESEND_FLAG)
		{
			PS2_CMD_RESEND_FLAG = 0;
		}
	}
	PS2_Status[PS2x] |= (PS2_TRANSMIT_START_FLAG|PS2_WRITE_FLAG|PS2_ACT); // now transmit
	PS2_Transmit_Start_Bit(PS2x);
	// Fork write procedure
	TASK_PS2_TimeOut_Active_Check(PS2x,PS2_TRANSMITT_TIME_MAX);
#if (PS2_DEBUG_MODE==1)
	if (Scp_Type == Script_Cmd)
		printf("@P:%d->Config_Cmd[%d],C_T[0x%02x]\n\r",(U16_T)PS2x,(U16_T)PS2_CfgCtlTab[PS2x].Cmdindex,(U16_T)PS2_Transmit_Buf[PS2x]); 
	else
		printf("@P:%d->Config_Para1[%d],C_T[0x%02x]\n\r",(U16_T)PS2x,(U16_T)PS2_CfgCtlTab[PS2x].Cmdindex,(U16_T)PS2_Transmit_Buf[PS2x]); 
#endif
} /* End of PS2_Script_Assing_Cmd_Para */

/*
 * ----------------------------------------------------------------------------
 * U8_T      PS2_Script_Check_Next_Para
 * Purpose : Check the Config script eext para status
 * Params  : PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *            HPS2A, HPS2B=>hardeare 
 *            EPS2A, EPS2B  =>external interrupt
 *            
 * Returns : 0x00-No more parameter to send
 *           0xff-No more parameter to send, but need to handle command
 *           0x01-Yes, has parameter to transfer confinully
 * Note    : 
 * ----------------------------------------------------------------------------
 */
U8_T PS2_Script_Check_Next_Para(PS2Port_TypeDef PS2x)
{
	if (((PS2_CfgScriptStructP)(PS2_CfgCtlTab[PS2x].ScriptPtr))->Parameter1 == 0x00) 
	{
		if (((PS2_CfgScriptStructP)(PS2_CfgCtlTab[PS2x].ScriptPtr))->Para1_RpnByteCnt == 0x00)
		{
			return 0;
		}
		if (((PS2_CfgScriptStructP)(PS2_CfgCtlTab[PS2x].ScriptPtr))->Para1_RpnByteCnt == 0xff)
		{
			return 0xff; // no parameter to transfer but need processing
		}
	}
	// have parameter to transfer
	PS2_Script_Assing_Cmd_Para(PS2x,Script_Para1);
	return 1;
} /* End of PS2_Script_Check_Next_Para */

/*
 * ----------------------------------------------------------------------------
 * U8_T      PS2_Script_Check_Next_Cmd
 * Purpose : Check the Config script eext command status
 * Params  : PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *            
 * Returns : 0x00-No more command to send, script transfef is over
 *           0x01-Yes, has another command need to transfer continully
 * Note    : 
 * ----------------------------------------------------------------------------
 */
U8_T PS2_Script_Check_Next_Cmd(PS2Port_TypeDef PS2x)
{
	PS2_CfgCtlTab[PS2x].Cmdindex++; //perform next command & parameter line
	if (PS2_CfgCtlTab[PS2x].Cmdindex >= PS2_CfgCtlTab[PS2x].ScriptCnt) // if script is done,then turn over it
	{
		PS2_Status[PS2x] &= ~PS2_SCRIPT_START_FLAG; // reset the script start flag
		return 0;
	}
	(PS2_CfgScriptStructP)(PS2_CfgCtlTab[PS2x].ScriptPtr) +=1; // move the charachter pointer
	PS2_Script_Assing_Cmd_Para(PS2x,Script_Cmd);
	return 1;
} /* End of  PS2_Script_Check_Next_Cmd */


/*
 * ----------------------------------------------------------------------------
 * void	     TASK_PS2_TimeOut_Active
 * Purpose : ps/2 receive or transmit time out task active
 * Params  : $PS2x: Select the PS/2 peripheral. 
 *            this parameter can be one of the following values:
 *            HPS2A, HPS2B=>hardeare 
 *            EPS2A, EPS2B  =>external interrupt
 *           $Time Out Value
 *            
 * Returns : NONE
 * Note    : 
 * ----------------------------------------------------------------------------
 */
void TASK_PS2_TimeOut_Active(PS2Port_TypeDef PS2x,U16_T state,U16_T timeout)
{
	PS2_Status[PS2x] |= state;
	// Active Time out Task
	TASK_PS2_TimeOut_Active_Check(PS2x,timeout);
}
/*
 * ----------------------------------------------------------------------------
 * U8_T      PS2_Script_Check_Next_Cmd_Rpn
 * Purpose : Check the Config script command responsed setting status
 * Params  : PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *            
 * Returns : 0x00-No more command to send, script transfef is over
 *           0x01-Yes, has another command need to transfer confinully
 * Note    : 
 * ----------------------------------------------------------------------------
 */
U8_T PS2_Script_Check_Next_Cmd_Rpn(PS2Port_TypeDef PS2x)
{
	if (((PS2_CfgScriptStructP)(PS2_CfgCtlTab[PS2x].ScriptPtr))->Cmd_RpnByteCnt)
	{
		PS2_CfgCtlTab[PS2x].FlowControl = PS2_SCP_CMD_RPN; // wait for command responsed
		PS2_CfgCtlTab[PS2x].RpnCnt =((PS2_CfgScriptStructP)(PS2_CfgCtlTab[PS2x].ScriptPtr))->Cmd_RpnByteCnt; // wait responed
		PS2_Script_Receive_Index[PS2x] = 0;
#if (PS2_DEBUG_MODE==1)
		printf("@P:%bu->Config_CmdRpn,C_Rpn=%bu\n\r",PS2x,PS2_CfgCtlTab[PS2x].RpnCnt);
#endif
		TASK_PS2_TimeOut_Active(PS2x,PS2_ACT,PS2_RECEIVE_TIME_MAX);
		return PS2_CfgCtlTab[PS2x].RpnCnt;
	}
	return 0;
}	/* End of PS2_Script_Check_Next_Cmd_Rpn */

/*
 * ----------------------------------------------------------------------------
 * U8_T      PS2_Script_Check_Next_Para_Rpn
 * Purpose : Check the Config script parameter responsed setting status
 * Params  : PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *            
 * Returns : 0x00-No more command to send, script transfef is over
 *           0x01-Yes, has another command need to transfer confinully
 * Note    : 
 * ----------------------------------------------------------------------------
 */
U8_T PS2_Script_Check_Next_Para_Rpn(PS2Port_TypeDef PS2x)
{
	if (((PS2_CfgScriptStructP)(PS2_CfgCtlTab[PS2x].ScriptPtr))->Para1_RpnByteCnt)
	{
		PS2_CfgCtlTab[PS2x].FlowControl = PS2_SCP_PARA1_RPN; // wait for parameter responsed
		PS2_CfgCtlTab[PS2x].RpnCnt =((PS2_CfgScriptStructP)(PS2_CfgCtlTab[PS2x].ScriptPtr))->Para1_RpnByteCnt; // wait responed
		PS2_Script_Receive_Index[PS2x] = 0;
#if (PS2_DEBUG_MODE==1)
		printf("@P:%bu->Config_ParaRpn,C_Rpn=%bu\n\r",PS2x,PS2_CfgCtlTab[PS2x].RpnCnt);
#endif
		TASK_PS2_TimeOut_Active(PS2x,PS2_ACT,PS2_RECEIVE_TIME_MAX);
		return PS2_CfgCtlTab[PS2x].RpnCnt;
	}
	return 0;
} /* End of PS2_Script_Check_Next_Para_Rpn */

/*
 * ----------------------------------------------------------------------------
 * U8_T      PS2_Check_Transmit_Complete
 * Purpose : Check the transmit byte is in complete status or not
 *           after transmit status, the flag will be clear
 *           Clear Flag:PS2_TRANSMIT_COMPLETE_FLAG,PS2_TRANSMIT_START_FLAG,PS2_WRITE_FLAG
 * Params  : PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *            
 * Returns : 0x00-No, not complete
 *           0x01-Yes,complete 
 * Note    : 
 * ----------------------------------------------------------------------------
 */
U8_T PS2_Check_Transmit_Complete(PS2Port_TypeDef PS2x)
{
	if ((PS2_Status[PS2x] & PS2_TRANSMIT_START_FLAG)) // no transmit
	{
		if (PS2_Status[PS2x] & PS2_TRANSMIT_COMPLETE_FLAG) // transmit complete
		{
			PS2_Status[PS2x] &= ~(PS2_TRANSMIT_COMPLETE_FLAG|PS2_TRANSMIT_START_FLAG); // clear transmit complete & start flag
			return 1; // transmit completed
		}
	}
	return 0; // transmit not complete
} /* End of PS2_Check_Transmit_Complete */ 

/*
 * ----------------------------------------------------------------------------
 * U8_T      PS2_Script_Check_Receive_Complete
 * Purpose : Check the receive complete condition
 * Params  : PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *            
 * Returns : 0x00-No complete
 *           0x01-Yes,complete
 * Note    : 
 * ----------------------------------------------------------------------------
 */
U8_T PS2_Script_Check_Receive_Complete(PS2Port_TypeDef PS2x)
{
	U8_T	indata;
	U16_T	act;
	
	if ((PS2_Receive_Rp[PS2x] != PS2_Receive_Wp[PS2x]) && (PS2_Clock_Cnt[PS2x] < 10) ) // have received data
	{
		/* Check the parity error condition */
		indata=PS2_FIFO_Pup(PS2x);
		act = PS2_Status[PS2x];
#if (PS2_DEBUG_MODE==1)
		printf("@P:%bu->Config_CmdRpn:%bu,Code[0x%02bx]\n\r",PS2x,PS2_CfgCtlTab[PS2x].RpnCnt,indata);
#endif
		/*Handle the error condition */
		if (act & PS2_PARITY_ERR_FLAG) // parrity error, need to ask resend
		{
			PS2_Status[PS2x] &= ~PS2_PARITY_ERR_FLAG;
			return 0xff;
		}
		if (PS2_CfgCtlTab[PS2x].RpnCnt)
			PS2_CfgCtlTab[PS2x].RpnCnt--;
		if (PS2_CfgCtlTab[PS2x].RpnCnt) //still have data for reading
		{
			TASK_PS2_TimeOut_Active(PS2x,PS2_ACT,PS2_RECEIVE_TIME_MAX);
		}
		else
		{
			if (PS2_Clock_Cnt[PS2x] == 0) // if there is no any transmiter now
			{
				PS2_Status[PS2x] &= ~PS2_ACT; // clear active flag
			}
		}
		PS2_Script_Receive_Queue[PS2x][PS2_Script_Receive_Index[PS2x]]=indata;
		PS2_Script_Receive_Index[PS2x]++;
		/*2.Device ask for resending*/
		if (indata == PS2_RESEND_CODE)
		{
			return PS2_RESEND_CODE;
		}
		return 1;
	}
	return 0;
} /* End of PS2_Script_Check_Receive_Complete */

/*
 * ----------------------------------------------------------------------------
 * U8_T      PS2_Script_Check_Next_Operation
 * Purpose : Check the Config script next stage operation
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *           @NextOperationx: will be combination of below
 *           Script_Cmd
 *           Script_CmdRpn
 *           Script_Para1
 *           Script_Para1Rpn
 *           Script_Para2
 *           Script_Para2Rpn
 * Returns : 0x00-No more parameter to send
 *           0xff-No more parameter to send, but need to handle command
 *           0x01-Yes, has parameter to transfer confinully
 * Note    : 
 * ----------------------------------------------------------------------------
 */
U8_T PS2_Script_Check_Next_Operation(PS2Port_TypeDef PS2x,U8_T ScriptNextOperation)
{
	// Check Next Command aviable
	if (ScriptNextOperation & Script_Cmd)
	{
		if (PS2_Script_Check_Next_Cmd(PS2x))
		{
			return 1;
		}
	}
	// Check Command Responsed aviable
	if (ScriptNextOperation & Script_CmdRpn)
	{
		if (PS2_Script_Check_Next_Cmd_Rpn(PS2x))
		{
			return 1;
		}
	}
	// Check Para1 aviable
	if (ScriptNextOperation & Script_Para1)
	{
		if (PS2_Script_Check_Next_Para(PS2x))
		{
			return 1;
		}
	}
	// Check Para1 Responsed aviable
	if (ScriptNextOperation & Script_Para1Rpn)
	{
		if (PS2_Script_Check_Next_Para_Rpn(PS2x))
			return 1;
	}
	return 0;
} /* End of PS2_Script_Check_Next_Operation */

/*
 * ----------------------------------------------------------------------------
 * void      PS2_Config_Start_Standard
 * Purpose : Start to use standard script of configuration for PS/2 Device
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *            
 * Returns :  
 * Note    :  
 * ----------------------------------------------------------------------------
 */
void PS2_Config_Start_Standard(PS2Port_TypeDef PS2x)
{
	PS2_Control[PS2x] = PS2_CONFIG; // start configure
	PS2_CfgCtlTab[PS2x].ScriptPtr = (U8_T *)PS2_CfgScriptTypeCheck;
	PS2_CfgCtlTab[PS2x].ScriptCnt = sizeof(PS2_CfgScriptTypeCheck)/sizeof(struct _PS2_CfgScriptStruct);
	PS2_CfgCtlTab[PS2x].Script = TYPE_CHECK_SCRIPT; // need to check the type first
	PS2_CfgCtlTab[PS2x].ResendCnt = 0; // reset the resend counter
	PS2_CfgCtlTab[PS2x].Receive_Error = 0; // reset receive error condition
	PS2_Script_Cfg_Start(PS2x,&PS2_CfgCtlTab[PS2x]);
	PS2_Status[PS2x] &= ~PS2_DEVICE_BAT_RECEIVED; // clear BAT code status
} /* End of PS2_Config_Start_Standard */

/*
 * ----------------------------------------------------------------------------
 * void      PS2_Script_F4_Enable
 * Purpose : Send out enable command to enable ps/2 device
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *            
 * Returns :  
 * Note    :  
 * ----------------------------------------------------------------------------
 */
void PS2_Script_F4_Enable(PS2Port_TypeDef PS2x)
{
	PS2_CfgCtlTab[PS2x].ScriptPtr = (U8_T *)PS2_cfgScriptEnable;
	PS2_CfgCtlTab[PS2x].ScriptCnt = sizeof(PS2_cfgScriptEnable)/sizeof(struct _PS2_CfgScriptStruct);
	PS2_CfgCtlTab[PS2x].Script = SCRIPT_ENABLE; // need to check the type first
	PS2_CfgCtlTab[PS2x].ResendCnt = 0; // reset the resend counter
	PS2_CfgCtlTab[PS2x].Receive_Error = 0; // reset receive error condition
	PS2_Script_Cfg_Start(PS2x,&PS2_CfgCtlTab[PS2x]);
} /* End of PS2_Script_F4_Enable */

/*
 * ----------------------------------------------------------------------------
 * void      PS2_Script_F5_Disable
 * Purpose : Send out input disable command to emute the input of ps/2 device
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *            
 * Returns :  
 * Note    :  
 * ----------------------------------------------------------------------------
 */
//void PS2_Script_F5_Disable(PS2Port_TypeDef PS2x)
//{
//	PS2_CfgCtlTab[PS2x].ScriptPtr = (U8_T *)PS2_cfgScriptDisable;
//	PS2_CfgCtlTab[PS2x].ScriptCnt = sizeof(PS2_cfgScriptDisable)/sizeof(struct _PS2_CfgScriptStruct);
//	PS2_CfgCtlTab[PS2x].Script = SCRIPT_ENABLE; // need to check the type first
//	PS2_CfgCtlTab[PS2x].ResendCnt = 0; // reset the resend counter
//	PS2_CfgCtlTab[PS2x].Receive_Error = 0; // reset receive error condition
//	PS2_Script_Cfg_Start(PS2x,&PS2_CfgCtlTab[PS2x]);
//} /* End of PS2_Script_F5_Disable */

/*
 * ----------------------------------------------------------------------------
 * void      PS2_Script_Receive_Resend
 * Purpose : send out a 0xfe command to ask device resend last byte
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *            
 * Returns :  
 * Note    :  
 * ----------------------------------------------------------------------------
 */
void PS2_Script_Receive_Resend(PS2Port_TypeDef PS2x)
{
	PS2_CfgCtlTab[PS2x].ScriptPtr = (U8_T *)PS2_cfgScriptResend;
	PS2_CfgCtlTab[PS2x].ScriptCnt = sizeof(PS2_cfgScriptResend)/sizeof(struct _PS2_CfgScriptStruct);
	PS2_CfgCtlTab[PS2x].Script = SCRIPT_RESEND; // need to check the type first
	PS2_CfgCtlTab[PS2x].ResendCnt = 0; // reset the resend counter
	PS2_CfgCtlTab[PS2x].Receive_Error = 0; // reset receive error condition
	PS2_Script_Cfg_Start(PS2x,&PS2_CfgCtlTab[PS2x]);
} // End of PS2_Script_Receive_Resend

/*
 * ----------------------------------------------------------------------------
 * void      PS2_Script_Ask_Device_Resend
 * Purpose : send out a 0xfe command to ask device resend last byte
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *            
 * Returns :  
 * Note    :  
 * ----------------------------------------------------------------------------
 */
//U8_T PS2_Script_Ask_Device_Resend(PS2Port_TypeDef PS2x)
//{
//	PS2_CfgCtlTab[PS2x].Receive_Error++;
//	if ( PS2_CfgCtlTab[PS2x].Receive_Error < SYSTEM_PS2_RESEND_MAX )
//	{
		//Send out 0xfe, but not interrupt the sequence.
//		PS2_Transmit_Buf[PS2x] = 0xfe; // move para1 to output buffer
//		PS2_Status[PS2x] |= (PS2_TRANSMIT_START_FLAG|PS2_WRITE_FLAG|PS2_ACT|PS2_RECEIVE_RESEND_FLAG);
//		PS2_Transmit_Start_Bit(PS2x); 		   
//		return 1;
//	}
//	return 0; // counter full
//}
/*
 * ----------------------------------------------------------------------------
 * void      TASK_Event_PS2_Handle
 * Purpose : Handle PS/2 device input include script & normal process
 *
 * Params  :  
 *            
 * Returns :  
 * Note    :  
 * ----------------------------------------------------------------------------
 */
void TASK_Event_PS2_Handle(void)
{
	U8_T	ps2index,indata,rece_status;

	ps2index = TASK_Register0; // PS/2 port ids
	if ((PS2_Status[ps2index] & PS2_SCRIPT_START_FLAG) == 0) // in mornal case
	{
		//1.Check error condition
		if (PS2_Status[ps2index] & PS2_TIME_OUT_FLAG )
		{
			PS2_Status[ps2index] &= ~(PS2_TIME_OUT_FLAG|PS2_ACT);
			PS2_Clock_Cnt[ps2index] = 0; // reset clock counter
			if ((PS2_Status[ps2index] & PS2_DEVICE_BAT_RECEIVED) || (PS2_Control[ps2index]==PS2_NONE)) // clear BAT code status
			{
				PS2_Config_Start_Standard(ps2index);
			}
		}
		//2.Check receive data complete condition
		if ((PS2_Receive_Rp[ps2index] != PS2_Receive_Wp[ps2index]) && (PS2_Clock_Cnt[ps2index] < 10) )
		{
			indata=PS2_FIFO_Pup(ps2index);
			//1.Check the devie plug in status(device is not configued
			switch (PS2_Control[ps2index])
			{
				case PS2_NONE :
					if (indata == 0xaa) // BAT code
					{
						TASK_PS2_TimeOut_Active(ps2index,PS2_ACT|PS2_DEVICE_BAT_RECEIVED,PS2_RECEIVE_TIME_MAX);
					}
					else
					{
						PS2_Config_Start_Standard(ps2index);
					}
					PS2_Start_Init_Flag &= ~(0x01 << ps2index);
					break;
				case PS2_MOUSE :
					PS2_Process_Mouse_Package(ps2index,indata);
					break;
				case PS2_KB :
					PS2_Process_Keyboard_Package(ps2index,indata);
					break;
			}
#if (PS2_DEBUG_MODE==1)
			printf("@P:%bu,R[0x%02bx]",ps2index,indata);
#endif
		}
	}
	else //in script mode
	{
		switch (PS2_CfgCtlTab[ps2index].FlowControl)
		{
PS2_SCRIPT_RESTART:
			case PS2_SCP_PENDING: // output pending(script & transmit not start)
				if ((PS2_Status[ps2index] & PS2_TRANSMIT_START_FLAG) == 0) // no transmit
				{
					if (PS2_Clock_Cnt[ps2index] == 0 ) // no any receive now
					{
						PS2_Receive_Rp[ps2index] = 0;
						PS2_Receive_Wp[ps2index] = 0;
						PS2_Script_Assing_Cmd_Para(ps2index,Script_Cmd); // assing the first command
					}
				}
				else // this should not happened, but when happened, just restart the Scritp again
				{
					PS2_Status[ps2index] &= ~(PS2_ACT|PS2_WRITE_FLAG|PS2_TRANSMIT_START_FLAG);
					PS2_Port_Recover(ps2index);
					goto PS2_SCRIPT_RESTART;
				}
				break;
			case PS2_SCP_CMD: // Command output
				if (PS2_Check_Transmit_Complete(ps2index))
				{
					if (PS2_Script_Cmd_Handle(ps2index))  // continue to process
					{
						if (PS2_Script_Check_Next_Operation(ps2index,Script_CmdRpn|Script_Para1|Script_Para1Rpn)==0)
						{
#if (PS2_DEBUG_MODE==1)
							printf("@P:%d->Config_Script[%d] Complete\n\r",(U16_T)ps2index,(U16_T)PS2_CfgCtlTab[ps2index].Script);
#endif
							PS2_Script_Status_Complete_Handle(ps2index); // continue to process
						}
					}
				}
				else
				{
					if (PS2_Status[ps2index] & PS2_TIME_OUT_FLAG) // device not responsed
					{
#if (PS2_DEBUG_MODE==1)
						printf("@P:%d->Config_Script[%d] Timeout,Stage=PS2_SCP_CMD\n\r",(U16_T)ps2index,(U16_T)PS2_CfgCtlTab[ps2index].Script);
#endif
						PS2_Script_Check_TimeOut(ps2index);
					}
				}
				break;
			case PS2_SCP_CMD_RPN: // Command respsed
				rece_status = PS2_Script_Check_Receive_Complete(ps2index);
				//1.Handle Parity error condition
				if ((rece_status == 0xff) || (rece_status == PS2_RESEND_CODE)) // parity error
				{
					if (rece_status == PS2_RESEND_CODE) // parity error
					{
						/* Stage-Command Responsed => Check the device ask for resend command, by received a 0xfe command */
						if (PS2_Script_Cmd_Resend_Handle(ps2index,PS2_SCP_CMD_RPN,((PS2_CfgScriptStructP)(PS2_CfgCtlTab[ps2index].ScriptPtr))->Cmd_RpnByteCnt,Script_Cmd) == 0 )
						{
							printf("Resend Fail\n\r");
							break;
						}
					}
					else
					{
						//PS2 Port Initial Start
						TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_PS2_Device_Resend_ID,0,ps2index,5,5); // start to resend
					}
					break;
				}
				//2.Handle Resend condition
				if (rece_status == 0x01) // receive complete 
				{
					if (PS2_Script_Cmd_Handle(ps2index)) //check every receive data if necessary, for reset command, need to wait long, or time out issue
					{
						if (PS2_CfgCtlTab[ps2index].RpnCnt == 0 ) // no more received
						{
							if (PS2_Script_Cmd_Handle(ps2index)) //after receive all the data,judge any special step need to be token
							{
								if (PS2_Script_Check_Next_Operation(ps2index,Script_Para1)==0)
								{
									if (PS2_Script_Check_Next_Operation(ps2index,Script_Cmd)==0)
									{
#if (PS2_DEBUG_MODE==1)
										printf("@P:%d->Config_Script[%d] Complete\n\r",(U16_T)ps2index,(U16_T)PS2_CfgCtlTab[ps2index].Script);
#endif
										PS2_Script_Status_Complete_Handle(ps2index); // continue to process
									}
								}
							}
						}
						else
						{
							PS2_Status[ps2index] |= PS2_ACT;
						}
					}
				}
				else
				{
					if (PS2_Status[ps2index] & PS2_TIME_OUT_FLAG) // device not responsed
					{
#if (PS2_DEBUG_MODE==1)
						printf("@P:%d->Config_Script[%d] Timeout,Stage=PS2_SCP_CMD_RPN\n\r",(U16_T)ps2index,(U16_T)PS2_CfgCtlTab[ps2index].Script);
#endif
						PS2_Status[ps2index] &= ~(PS2_SCRIPT_START_FLAG|PS2_TIME_OUT_FLAG|PS2_ACT);
						if ((PS2_CfgCtlTab[ps2index].Script == TYPE_CHECK_SCRIPT) && (PS2_CfgCtlTab[ps2index].RpnCnt == 1)) // this is could be mouse, so go to mouse config
						{
#if (PS2_RELEASE_MODE==1)
							printf("\r\n");
#endif
#if (PS2_DEBUG_MODE==1)
							printf("@P:%d->Config_Script[%d] Complete\n\r",(U16_T)ps2index,(U16_T)PS2_CfgCtlTab[ps2index].Script);
#endif
							PS2_Script_Status_Complete_Handle(ps2index); // continue to process
						}
						else
						{
							PS2_Script_Check_TimeOut(ps2index);
						}
					}
				}
				break;
			case PS2_SCP_PARA1: // Parameter output	
				if (PS2_Check_Transmit_Complete(ps2index))
				{
					if (PS2_Script_Check_Next_Operation(ps2index,Script_Para1Rpn)==0)
					{
						if (PS2_Script_Check_Next_Operation(ps2index,Script_Cmd)==0)
						{
#if (PS2_DEBUG_MODE==1)
							printf("@P:%d->Config_Script[%d] Complete\n\r",(U16_T)ps2index,(U16_T)PS2_CfgCtlTab[ps2index].Script);
#endif
							PS2_Script_Status_Complete_Handle(ps2index);// continue to process	
						}
					}
				}
				else
				{
					if (PS2_Status[ps2index] & PS2_TIME_OUT_FLAG) // device not responsed
					{
#if (PS2_DEBUG_MODE==1)
						printf("@P:%d->Config_Script[%d] Timeout,Stage=PS2_SCP_PARA1\n\r",(U16_T)ps2index,(U16_T)PS2_CfgCtlTab[ps2index].Script);
#endif
						PS2_Script_Check_TimeOut(ps2index);
					}
				}
				break;
			case PS2_SCP_PARA1_RPN : // Parameter Responsed
				rece_status = PS2_Script_Check_Receive_Complete(ps2index);
				if ((rece_status == 0xff) || (rece_status == PS2_RESEND_CODE)) // parity error
				{
					if (rece_status == PS2_RESEND_CODE) // parity error
					{	
						//printf("(%d)CMD_RPN_CmdIndex=%d,Resend\n\r",(U16_T)ps2index,(U16_T)PS2_CfgCtlTab[ps2index].Cmdindex);
						/* Stage-Parameter1 Responsed => Check the device ask for resend command, by received a 0xfe command */
						if (PS2_Script_Cmd_Resend_Handle(ps2index,PS2_SCP_PARA1_RPN,((PS2_CfgScriptStructP)(PS2_CfgCtlTab[ps2index].ScriptPtr))->Para1_RpnByteCnt,Script_Para1) == 0 )
						{
							//printf("Resend Fail\n\r");
							/*how to continue process? */
						}
					}
					else
					{
						//printf("Parrity1 error\n\r");
						TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_PS2_Device_Resend_ID,0,ps2index,5,5); // start to resend
					}
					break;
				}
				if (rece_status == 0x01) // receive complete
				{
					if (PS2_CfgCtlTab[ps2index].RpnCnt == 0 ) // no more received
					{
						if (PS2_Script_Cmd_Handle(ps2index)) // continue to process
						{
							if (PS2_Script_Check_Next_Operation(ps2index,Script_Cmd)==0)
							{
#if (PS2_DEBUG_MODE==1)
								printf("@P:%d->Config_Script[%d] Complete\n\r",(U16_T)ps2index,(U16_T)PS2_CfgCtlTab[ps2index].Script);
#endif
								PS2_Script_Status_Complete_Handle(ps2index); // continue to process
							}
						}
					}
				}
				else
				{
					if (PS2_Status[ps2index] & PS2_TIME_OUT_FLAG) // device not responsed
					{
#if (PS2_DEBUG_MODE==1)
						printf("@P:%d->Config_Script[%d] Timeout,Stage=PS2_SCP_PARA1_RPN\n\r",(U16_T)ps2index,(U16_T)PS2_CfgCtlTab[ps2index].Script); 
#endif
						PS2_Script_Check_TimeOut(ps2index);
					}
				}
				break;
		}
	}
} /* End of TASK_Event_PS2_Handle */

/*
 * ----------------------------------------------------------------------------
 * U8_T      PS2_FIFO_Pup
 * Purpose : Check the time out condition on transmitte or receive 
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 * 
 * Returns : The received data.(include the address)
 * Note    : This function should place on 1ms task in main function
 * ----------------------------------------------------------------------------
 */
U8_T PS2_FIFO_Pup(PS2Port_TypeDef PS2x)
{
	U8_T	receive_data;

	receive_data = PS2_Receive_FIFO_Queue[PS2x][PS2_Receive_Rp[PS2x]];
	PS2_Receive_Rp[PS2x]++;
	if (PS2_Receive_Rp[PS2x] >= PS2_FIFO_QUEUE_MAX)
		PS2_Receive_Rp[PS2x] = 0;

	return receive_data;
} /* End of PS2_FIFO_Pup */

/*
 * ----------------------------------------------------------------------------
 * void      PS2_FIFO_Push
 * Purpose : Check the time out condition on transmitte or receive 
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
  *          HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 
 *           @PS2_Byte: the data need to store
 *
 * Returns : The result for push operation
 * Note    : 
 * ----------------------------------------------------------------------------
 */ 
void PS2_FIFO_Push(PS2Port_TypeDef PS2x,U8_T PS2_Byte)
{
	PS2_Receive_FIFO_Queue[PS2x][PS2_Receive_Wp[PS2x]] = PS2_Byte;
	PS2_Receive_Wp[PS2x]++;
	if (PS2_Receive_Wp[PS2x] >= PS2_FIFO_QUEUE_MAX)
		PS2_Receive_Wp[PS2x] = 0;
} /* End of PS2_FIFO_Push */

/*
 * ----------------------------------------------------------------------------
 * void      PS2_Script_Cfg_Start
 * Purpose : start to use Script control to initlize of PS/2 device
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 
 *           @ctrlp: The Script Table pointer
 *
 * Returns : 
 * Note    : 
 * ----------------------------------------------------------------------------
 */
void PS2_Script_Cfg_Start(PS2Port_TypeDef PS2x,PS2_CfgCtlStruct *ctrlp)
{
	ctrlp->Cmdindex = 0; // start from first
	ctrlp->FlowControl = PS2_SCP_PENDING;// wait the loop to start write
	PS2_Status[PS2x] |= PS2_SCRIPT_START_FLAG;
	// Fork write procedure
	TASK_Active(TASK_TYPE_EVENT,TASK_EVENT_PS2_HANDLE_ID,0,PS2x,0,0);
} /* End of PS2_Script_Cfg_Start */

/*
 * ----------------------------------------------------------------------------
 * void      PS2_Transmit_Start_Bit
 * Purpose : generate the transmit start bit condistion in clock & data line
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 
 *           @ctrlp: The Script Table pointer
 *
 * Returns : 
 * Note    : 
 * ----------------------------------------------------------------------------
 */
void PS2_Transmit_Start_Bit(PS2Port_TypeDef PS2x)
{
//#if SYSTEM_EXTI_PS2_ENABLE
//	U8_T	exti_port;
//#endif
	//1.Check Hardware PS/2 Control funcion setting
#if SYSTEM_HARDWARE_PS2_ENABLE
	if (PS2x <= HPS2B)
	{
		HW_PS2_Enable(PS2x); // Enable the hardware first
		HW_PS2_Transmit(PS2x,PS2_Transmit_Buf[PS2x]);
		return;
	}
#endif
	//2.Check External Interrupt PS/2 Control funcion setting
//#if SYSTEM_EXTI_PS2_ENABLE
//	if (PS2x >= EPS2A)
//	{
//		exti_port = PS2x-SYSTEM_HARDWARE_PS2_PORT_MAX;
//		EXTI_PS2_Disable(exti_port); // pull low clock, inhibit device input
//		DELAY_Us(100);
//		EXTI_PS2_Pin_Control(exti_port,PS2_Clock_Low,PS2_Data_Low);
//		DELAY_Us(100);
//		EXTI_PS2_Enable(exti_port); // this will pull up the clock line also
//	}
//#endif
} /* End of PS2_Transmit_Start_Bit */

/*
 * ----------------------------------------------------------------------------
 * U8_T      PS2_ParitySub
 * Purpose : subroutine for calculate the parity bit value of inputdata
 * Params  : @inputdata: the character need to calculate
 *
 * Returns : the counter of bit value is 1
 * Note    : 
 * ----------------------------------------------------------------------------
 */
//U8_T PS2_ParitySub(U8_T inputdata)
//{
//	U8_T	loop,cPS2_Parity_Cnt;
//
//	cPS2_Parity_Cnt = 0 ;
//	for( loop=0 ; loop < 8 ; loop++)
//	{
//		if( inputdata & 0x01)
//			cPS2_Parity_Cnt++;
//		inputdata=inputdata >> 1;
//	}
//	return cPS2_Parity_Cnt;
//} /* End of PS2_ParitySub */

/*
 * ----------------------------------------------------------------------------
 * U8_T      PS2_GenerateParity
 * Purpose : calculate the parity bit value of inputdata
 * Params  : @inputdata: the character need to calculate
 *
 * Returns : the parity value of inputdata
 * Note    : 
 * ----------------------------------------------------------------------------
 */
//U8_T PS2_GenerateParity(U8_T inputdata)
//{
//	U8_T	parity;
//
//	parity = (~PS2_ParitySub(inputdata)) & 0x01;
//	return parity;
//} /* End of PS2_GenerateParity */

/*
 * ----------------------------------------------------------------------------
 * void      PS2_Script_Status_Complete_Handle
 * Purpose : After send out all script content, do the next task check
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *
 * Returns : 
 * Note    : 
 * ----------------------------------------------------------------------------
 */
void PS2_Script_Status_Complete_Handle(PS2Port_TypeDef PS2x)
{
	U8_T	ps2_led;

	switch (PS2_CfgCtlTab[PS2x].Script)
	{
		case TYPE_CHECK_SCRIPT:
			PS2_Control[PS2x] = PS2_CONFIG; // start configure
			if (PS2_Script_Receive_Queue[PS2x][1] == 0xab) // keyboard type is ab,83
			{
#if (PS2_DEBUG_MODE==1)
				printf("0_Table=%d,element=%d,",(U16_T)sizeof(PS2_CfgScriptKB),(U16_T)sizeof(struct _PS2_CfgScriptStruct));
#endif
				PS2_CfgCtlTab[PS2x].ScriptPtr = (U8_T *)PS2_CfgScriptKB;
				PS2_CfgCtlTab[PS2x].ScriptCnt = sizeof(PS2_CfgScriptKB)/sizeof(struct _PS2_CfgScriptStruct);
				PS2_CfgCtlTab[PS2x].Script = CONFIG_SCRIPT_KB; // need to check the type first
			}
			else if (PS2_Script_Receive_Queue[PS2x][1] <= 0x04) // mouse type is 00,02,03,04
			{
#if (PS2_DEBUG_MODE==1)
				printf("1_Table=%d,element=%d,",(U16_T)sizeof(PS2_CfgScriptMS),(U16_T)sizeof(struct _PS2_CfgScriptStruct));
#endif
				PS2_CfgCtlTab[PS2x].ScriptPtr = (U8_T *)PS2_CfgScriptMS;
				PS2_CfgCtlTab[PS2x].ScriptCnt = sizeof(PS2_CfgScriptMS)/sizeof(struct _PS2_CfgScriptStruct);
				PS2_CfgCtlTab[PS2x].Script = CONFIG_SCRIPT_MS; // need to check the type first
			}
#if (PS2_DEBUG_MODE==1)
			printf("Assign New Script[%d],Cnt=%d\n\r",(U16_T)PS2_CfgCtlTab[PS2x].Script,(U16_T)PS2_CfgCtlTab[PS2x].ScriptCnt);
#endif
			USBDC_WakeupUpstreamHost(KVM_CurrentHost);
			PS2_Script_Cfg_Start(PS2x,&PS2_CfgCtlTab[PS2x]);
			break;
		case CONFIG_SCRIPT_KB:
			if (PS2_Control[PS2x] == PS2_CONFIG)
			{
				PS2_cfgScriptKB_F3[0].Parameter1 = PS2_F3_Setging;
				PS2_CfgCtlTab[PS2x].ScriptPtr = (U8_T *)PS2_cfgScriptKB_F3;
				PS2_CfgCtlTab[PS2x].ScriptCnt = sizeof(PS2_cfgScriptKB_F3)/sizeof(struct _PS2_CfgScriptStruct);
				PS2_CfgCtlTab[PS2x].Script = CONFIG_SCRIPT_KB_F3; // need to check the type first
				PS2_Script_Cfg_Start(PS2x,&PS2_CfgCtlTab[PS2x]);
			}
			break;
		case CONFIG_SCRIPT_KB_F3:
			if (PS2_Control[PS2x] == PS2_CONFIG)
			{
				ps2_led = KVM_Hostkey_LED_PS2_Convert(KVM_HostLed[KVM_CurrentHost]);
				PS2_Keyboad_LED_Script(PS2x,ps2_led);
			}
			break;
		case CONFIG_SCRIPT_KB_LED:
			if (PS2_Control[PS2x] == PS2_CONFIG)
			{
				PS2_Control[PS2x] = PS2_KB;
				PS2_Keyboard_Package_Index[PS2x]=0; // Reset the Keyboad receive buffer Index
			}
			TASK_KVM_CONSOLE_PS2_KBLed_Control_Flag = 0;
			break;
		case CONFIG_SCRIPT_MS:
			PS2_Control[PS2x] = PS2_MOUSE; // how about mouse type?	
			PS2_Mouse_Type[PS2x] = PS2_Script_Receive_Queue[PS2x][1]; // store the mouse type
			PS2_Mouse_Package_Index[PS2x] = 0x00;// Reset the Mouse receive buffer Index
			break;
	}
} /* End of PS2_Script_Status_Complete_Handle */

/*
 * ----------------------------------------------------------------------------
 * U8_T      PS2_Script_Cmd_Resend_Handle
 * Purpose : Handle the script resend status
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 * 
 *           @flowcontrol: can onley be 
 *            PS2_SCP_CMD_RPN
 *            PS2_SCP_PARA1_RPN
 *
 *           @resendcnt: can onley be struct PS2_CfgCtlTab
 *            Cmd_RpnByteCnt 
 *            Para1_RpnByteCnt
 *
 *           @resendcmd: if device send 0xfe, the resend operation
 *            PS2_SCP_CMD 
 *            PS2_SCP_PARA1
 * Returns : return command has bee handle
 * Note    : 
 * ----------------------------------------------------------------------------
 */
U8_T PS2_Script_Cmd_Resend_Handle(PS2Port_TypeDef PS2x,U8_T flowcontrol,U8_T resendcnt,U8_T resendcmd)
{
	/* Stage-Command Responsed => Check the device ask for resend command, by received a 0xfe command */
	if (PS2_CfgCtlTab[PS2x].FlowControl == flowcontrol) // in waiting receive stage
	{
		if (resendcnt == (PS2_CfgCtlTab[PS2x].RpnCnt+1)) //the first recevied byte
		{
			if ((PS2_Script_Receive_Queue[PS2x][PS2_Script_Receive_Index[PS2x]-1] == 0xfe) || (PS2_Script_Receive_Queue[PS2x][PS2_Script_Receive_Index[PS2x]-1] == 0xfc)) // need to resend command) // need to resend command
			{
				PS2_CfgCtlTab[PS2x].ResendCnt++;
				PS2_Script_Receive_Index[PS2x]--;
				printf("Script:Resend,PS2_Script_Receive_Index=%d\n\r",(U16_T)PS2_Script_Receive_Index[PS2x]);
				if (PS2_CfgCtlTab[PS2x].ResendCnt > SYSTEM_PS2_RESEND_MAX) // if too many time for reseind, then skip this command and continue 
				{
					PS2_CfgCtlTab[PS2x].ResendCnt=0;
					return 0;
				}
				PS2_CMD_RESEND_FLAG = 1;
				PS2_Script_Assing_Cmd_Para(PS2x,resendcmd); // assing the first command
				return 1; // need to resend
			}	
			else
			{
				PS2_CfgCtlTab[PS2x].ResendCnt = 0;
			}
		}
	}
	return 0; // no resend condition
}
/*
 * ----------------------------------------------------------------------------
 * U8_T      PS2_Script_Cmd_Handle
 * Purpose : after receive or transmit need to do some special control
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *
 * Returns : return command has bee handle
 * Note    : 
 * ----------------------------------------------------------------------------
 */
U8_T PS2_Script_Cmd_Handle(PS2Port_TypeDef PS2x)
{
	if ((PS2_CfgCtlTab[PS2x].Script == CONFIG_SCRIPT_KB) || (PS2_CfgCtlTab[PS2x].Script == CONFIG_SCRIPT_MS))
	{
		if (PS2_CfgCtlTab[PS2x].FlowControl == PS2_SCP_CMD_RPN)
		{
			if (PS2_CfgCtlTab[PS2x].RpnCnt) 
			{
				if ((((PS2_CfgScriptStructP)(PS2_CfgCtlTab[PS2x].ScriptPtr))->Cmd == 0xff) && (PS2_CfgCtlTab[PS2x].RpnCnt == (((PS2_CfgScriptStructP)(PS2_CfgCtlTab[PS2x].ScriptPtr))->Cmd_RpnByteCnt-1))) // reset command
				{
					TASK_PS2_TimeOut_Active_Check(PS2x,PS2_RECEIVE_ACT_TIME_BAT);
				}
				else
				{
					TASK_PS2_TimeOut_Active_Check(PS2x,PS2_RECEIVE_ACT_TIME_MAX);
				}
			}
		}
	}
	return 1;
}

/*
 * ----------------------------------------------------------------------------
 * void PS2_Port_Recover(PS2Port_TypeDef PS2x)
 * Purpose : Pull up the Clock line for ps2 port
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *
 * Returns : return command has bee handle
 * Note    : 
 * ----------------------------------------------------------------------------
 */
void PS2_Port_Recover(PS2Port_TypeDef PS2x)
{
//#if SYSTEM_EXTI_PS2_ENABLE
//	U8_T	exti_port;
//#endif
	PS2_Clock_Cnt[PS2x] = 0;
	//2.Check External Interrupt PS/2 Control funcion setting
#if SYSTEM_HARDWARE_PS2_ENABLE
	if (PS2x <= HPS2B)
	{
		HW_PS2_Enable(PS2x);
		return;
	}
#endif
//#if SYSTEM_EXTI_PS2_ENABLE
//	if (PS2x >= EPS2A)
//	{
//		exti_port = PS2x-SYSTEM_HARDWARE_PS2_PORT_MAX;
//		EXTI_PS2_Pin_Control(exti_port,PS2_Clock_High,PS2_Data_High);
//		EXTI_PS2_Enable(exti_port); // Enable the interrupt of clock line
//	}
//#endif

}
/*
 * ----------------------------------------------------------------------------
 * void      PS2_Script_Check_TimeOut
 * Purpose : during config script, it time out happened, then return the 
 *           PS2_Control[PS2x] to  PS2_NONE
 
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *
 * Returns : return command has bee handle
 * Note    : 
 * ----------------------------------------------------------------------------
 */
void PS2_Script_Check_TimeOut(PS2Port_TypeDef PS2x)
{
	/* clear status */
	PS2_Status[PS2x] &= ~(PS2_SCRIPT_START_FLAG|PS2_TIME_OUT_FLAG|PS2_ACT|PS2_WRITE_FLAG|PS2_TRANSMIT_START_FLAG);
	PS2_Start_Init_Flag &= ~(0x01 << PS2x);
	PS2_CfgCtlTab[PS2x].FlowControl = 0;
	PS2_Control[PS2x] = PS2_NONE; /* no device conntected */
	PS2_Port_Recover(PS2x);
	if (PS2_CfgCtlTab[PS2x].Script == CONFIG_SCRIPT_KB_LED)
	{
		TASK_KVM_CONSOLE_PS2_KBLed_Control_Flag = 0;
	}
}

/*
 * ----------------------------------------------------------------------------
 * void      PS2_Process_Mouse_Package
 * Purpose : receive the mouse package, according PS/2 mouse type will receive
 *           different byte count of package 
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *
 * Returns : 
 * Note    : 
 * ----------------------------------------------------------------------------
 */    
void PS2_Process_Mouse_Package(PS2Port_TypeDef PS2x,U8_T indata)
{
	U8_T	index;

	//1. Check Time out Status
	if (PS2_Status[PS2x] & PS2_TIME_OUT_FLAG)
	{
		PS2_Status[PS2x] &= ~PS2_TIME_OUT_FLAG;
		PS2_Mouse_Package_Index[PS2x]=0;
		// This is Mouse Replug condition
		if ((PS2_Mouse_Package[PS2x][0] == 0xaa) || ((PS2_Mouse_Package[PS2x][0] == 0xff) && (PS2_Mouse_Package[PS2x][1] == 0xff)))
		{
			PS2_Mouse_Type[PS2x] = 0; // reset moues type
			PS2_Config_Start_Standard(PS2x);
			goto PROCESS_PS2_MOUSE_END;
		}
		if ( PS2_Mouse_Package_Index[PS2x] > 0 )
		{
			PS2_Script_F4_Enable(PS2x);
			goto PROCESS_PS2_MOUSE_END;
		}
	}
	if ((PS2_Status[PS2x] & PS2_TRANSMIT_RESEND_FLAG) && ((indata == 0xfa) || (indata == 0xfe)))
	{
		PS2_Script_F4_Enable(PS2x);
		PS2_Status[PS2x] &= ~PS2_TRANSMIT_RESEND_FLAG;
		goto PROCESS_PS2_MOUSE_END;
	}
	//5. Store Data
	index = PS2_Mouse_Package_Index[PS2x];
	PS2_Mouse_Package[PS2x][index] = indata;
	PS2_Mouse_Package_Index[PS2x]++;
	PS2_Status[PS2x] &= ~PS2_TRANSMIT_RESEND_FLAG;
	//6. Process
	switch ( index )
	{
		case 0 : //1st Mouse Data
			if ((indata & 0x08 ) == 0x00)
			{
			// the first byte is not correct, so need to skip all the package
				PS2_Script_F4_Enable(PS2x);
				PS2_Mouse_Package_Index[PS2x]=0;
			}
#if  PS2_MS_DATA_DEBUG_MODE
			printf("$P:%d-M[%02x,",(U16_T)PS2x,(U16_T)PS2_Mouse_Package[PS2x][0]);
#endif
			PS2_Status[PS2x] |= PS2_DEVICE_BAT_RECEIVED;
			goto PROCESS_PS2_MOUSE_END;
		case 1 : // 2nd Mouse Data
#if PS2_MS_DATA_DEBUG_MODE
			printf("%02x",(U16_T)PS2_Mouse_Package[PS2x][1]);
#endif
			if ((PS2_Mouse_Package[PS2x][0] == 0xaa) && (indata == 0x00))
			{
				PS2_Mouse_Type[PS2x] = 0; // reset moues type
				PS2_Config_Start_Standard(PS2x);
				PS2_Mouse_Package_Index[PS2x]=0;
#if PS2_MS_DATA_DEBUG_MODE
				printf("]=>Config\n\r");
#endif
				goto PROCESS_PS2_MOUSE_END;
			}
#if PS2_MS_DATA_DEBUG_MODE
			printf(",");
#endif
			PS2_Mouse_Package[PS2x][0] &= 0x3f; // prevent the device send false data
			PS2_Status[PS2x] &= ~PS2_DEVICE_BAT_RECEIVED;
			goto PROCESS_PS2_MOUSE_END;
		case 2 : // 3rd Mouse Data
			PS2_Mouse_Package[PS2x][3] = 0; // reset last byte first
#if PS2_MS_DATA_DEBUG_MODE
			printf("%02x,",(U16_T)PS2_Mouse_Package[PS2x][2]);
#endif
			if ( PS2_Mouse_Type[PS2x] == 0 )
			{
			// mouse type = 2, 3 bytes package
				goto PS2_MS_GET_OUT;
			}
			goto PROCESS_PS2_MOUSE_END;
		case 3 : // last byte
			if ( PS2_Mouse_Type[PS2x] == 3)
			{
				// type 3 
				PS2_Mouse_Package[PS2x][3] = indata & 0x0f;
			}
			else
			{
				PS2_Mouse_Package[PS2x][3] = indata & 0x3f; // get rid off bit7 & 6 for in case mouse report error
			}
#if  PS2_MS_DATA_DEBUG_MODE
			printf("%02x]\n\r",(U16_T)PS2_Mouse_Package[PS2x][3]);
#endif
			break;
	}
PS2_MS_GET_OUT:
	PS2_Mouse_Package_Index[PS2x] = 0; // clear index first, for next package
	DATAST_Append_Generic_MS_IN_Q(GENERIC_USAGE_07_MAKE,PS2_Mouse_Package[PS2x]); // normal mouse package
	DATAST_PS2_MS_Handle(0);
	return;
PROCESS_PS2_MOUSE_END :
	TASK_PS2_TimeOut_Active(PS2x,PS2_ACT,PS2_RECEIVE_TIME_MAX);
	return;
}

/*
 * ----------------------------------------------------------------------------
 * void PS2_Process_Keyboard_AddKB_Buf(PS2Port_TypeDef PS2x,U8_T indata)
 * Purpose : add keyboard data package into queue
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *           @indata: ps/2 keycode
 * Returns : 
 * Note    : 
 * ----------------------------------------------------------------------------
 */
void PS2_Process_Keyboard_AddKB_Buf(PS2Port_TypeDef PS2x,U8_T indata)
{
	PS2_Keyboard_Package[PS2x][PS2_Keyboard_Package_Index[PS2x]] = indata; // put the input into keyboard buffer
	PS2_Keyboard_Package_Index[PS2x]++; // add the value of keyboard buf counter
}

/*
 *--------------------------------------------------------------------------------
 * U8_T HOTKEY_Check_PS2_Key_Break(U8_T buftype)
 * Purpose : check break key status
 * Params  : None.
 * Returns : 0/1
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
#if (!KVM_HOTKEY_SUPPORT)
U8_T HOTKEY_Check_PS2_Key_Break(U8_T buftype)
{
	if (buftype == KB_TYPE_BREAK_F0)
	{
		return 1;
	}

	if (buftype == KB_TYPE_BREAK_E0) 
	{
		return 1;
	}

	if (buftype == KB_TYPE_BREAK_E1) 
	{
		return 1;
	}

	return 0;
}  
#endif

/*
 * ----------------------------------------------------------------------------
 * void PS2_Process_Keyboard_Package_Sub(PS2Port_TypeDef PS2x,U8_T buftype,U8_T indata) {	
 * Purpose : internal subroutine for PS2_Process_Keyboard_Package
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 *           @buftype:keycode type
 *           @indata: ps/2 keycode
 * Returns : 
 * Note    : 
 * ----------------------------------------------------------------------------
 */
void PS2_Process_Keyboard_Package_Sub(PS2Port_TypeDef PS2x,U8_T buftype,U8_T indata)
{
	if ((PS2_Keyboard_Tx_Control & PS2_KB_TX_DISABLE_FLAG) == 0x00) // if ps/2 tx buffer is enable
	{
		PS2_Process_Keyboard_AddKB_Buf(PS2x,indata); // move indata into PS2_Keyboard_Package[PS2x]
	}
#if PS2_KB_DATA_DEBUG_MODE
	printf("$P:%d-K",(U16_T)PS2x);
	for (index=0; index < PS2_Keyboard_Package_Index[PS2x] ; index++)
	{
		printf("[%02x]",(U16_T)PS2_Keyboard_Package[PS2x][index]);
	}
	printf("\n\r");
#endif
	//.Do the PS/2 keyboard make & break key balance maintain
	if (HOTKEY_Check_PS2_Key_Break(buftype))
	{
		if (PS2_KB_Balance_Cnt[PS2x])
			PS2_KB_Balance_Cnt[PS2x]--;
		PS2_KB_Last_Code[PS2x] = 0x00;
	}
	else
	{
		if (PS2_KB_Last_Code[PS2x] != indata)
		{
			PS2_KB_Last_Code[PS2x] = indata;
			PS2_KB_Last_Type[PS2x] = buftype;
			PS2_KB_Balance_Cnt[PS2x]++;
		}
		else if (PS2_KB_Last_Type[PS2x] != buftype)
		{
			PS2_KB_Last_Type[PS2x] = buftype;
			PS2_KB_Balance_Cnt[PS2x]++;
		}
	}
	//---------------------------------------------------------------------------------------------------
	// For PS2 Keyboard Balance
	//---------------------------------------------------------------------------------------------------
	PS2_KB_Balance_CountDown[PS2x] = 0;
	if ((PS2_KB_Balance_CountDownTask & PS2_PORT_BIT[PS2x]) == 0x00 )
	{
		PS2_KB_Balance_CountDownTask |= PS2_PORT_BIT[PS2x]; // task already in processing
		TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_PS2_Balance_Maintain_ID,TASK_EVENT_ACTIVE,PS2x,100,100);
	}
	//----------------------------------------------------------------------------------------------------
	if (KVM_PS2_HotkeySwitch_Flag==0)
		DATAST_PS2_KB_Handle(buftype,PS2_Keyboard_Package_Index[PS2x],PS2_Keyboard_Package[PS2x]);

	PS2_Keyboard_Package_Index[PS2x]=0; // reset the counter
}

/******************************************************************************
**  void PS2_Process_Keyboard_Package(PS2Port_TypeDef PS2x,U8_T indata)
**  Parameter    :  buftype
**  return       :  NONE
**  Description  :  handle the ps2 keyboard input data
**  NOTICE       :  NONE
*/
void PS2_Process_Keyboard_Package(PS2Port_TypeDef PS2x,U8_T indata)
{
	//1. Check out Parity error status
	if (PS2_Status[PS2x] & PS2_PARITY_ERR_FLAG)
	{
		PS2_Status[PS2x] &= ~PS2_PARITY_ERR_FLAG;
		//printf("Rece:Resend\n");
		PS2_Script_Receive_Resend(PS2x);
		goto PROCESS_PS2_KEYBOARD_END;
	}
	//2. Check out Time out status
	if (PS2_Status[PS2x] & PS2_TIME_OUT_FLAG)
	{
		PS2_Status[PS2x] &= ~PS2_TIME_OUT_FLAG;
		goto PROCESS_PS2_KEYBOARD_END;
	}
	if (indata == 0xaa )
	{
		// it is BAT code, means keyboard plug off and plug in
		PS2_Config_Start_Standard(PS2x);
#if PS2_KB_DATA_DEBUG_MODE
		printf("$P:%d-K[%02x]-Config\n\r",(U16_T)PS2x,(U16_T)indata);
#endif
		goto PROCESS_PS2_KEYBOARD_END;
	}
	if (indata == 0x00 || indata == 0xfa || indata == 0xfe )
	{
		// Skip this code
		goto PROCESS_PS2_KEYBOARD_END;
	}
	switch (PS2_Keyboard_Package_Index[PS2x])
	{
		case 0 : // first byte, the buf is empty
			if ((indata != 0xf0) && (indata != 0xe0) && (indata != 0xe1)) // Check keycode type
			{
#if PROJECT_KVM_CONSOLE_ENABLE
				KVM_CheckPS2Modifiers(KB_TYPE_MAKE_F0,indata);
#endif
				PS2_Process_Keyboard_Package_Sub(PS2x,KB_TYPE_MAKE_F0,indata);
			}
			else
			{
				PS2_Process_Keyboard_AddKB_Buf(PS2x,indata);
			}
			break;
		case 1 : // have one byte in
			// check the F0 break first
			if ( PS2_Keyboard_Package[PS2x][0] == 0xf0 )
			{
				if ( indata != 0xf0 ) // prevent strange problem
				{
#if PROJECT_KVM_CONSOLE_ENABLE
					KVM_CheckPS2Modifiers(KB_TYPE_BREAK_F0,indata);
#endif
					PS2_Process_Keyboard_Package_Sub(PS2x,KB_TYPE_BREAK_F0,indata);
				}
				break;
			}
			// check the E0 make key
			if ( PS2_Keyboard_Package[PS2x][0] == 0xe0 )
			{
				if ( indata != 0xe0 ) // this is the F0 break key
				{
					if ( indata != 0xf0 ) // this e0 make, move to usb queue
					{
#if PROJECT_KVM_CONSOLE_ENABLE
						KVM_CheckPS2Modifiers(KB_TYPE_MAKE_E0,indata);
#endif
						PS2_Process_Keyboard_Package_Sub(PS2x,KB_TYPE_MAKE_E0,indata);
						break;
					}
				}
				else
					break;
			}
			PS2_Process_Keyboard_AddKB_Buf(PS2x,indata);
			break; 
		case 2 :
		case 3 :
		case 4 : // 2~4 bytes in buffer
			if ( PS2_Keyboard_Package[PS2x][0] == 0xe0 )
			{
				if ( indata != 0xf0 ) // this is the F0 break key
				{
#if (PROJECT_KVM_CONSOLE_ENABLE)
					KVM_CheckPS2Modifiers(KB_TYPE_BREAK_E0,indata);
#endif
					PS2_Process_Keyboard_Package_Sub(PS2x,KB_TYPE_BREAK_E0,indata);
				}
				break;
			}
			if ( PS2_Keyboard_Package[PS2x][0] == 0xe1 )
			{
				// this is the E1 process
				if ( PS2_Keyboard_Package[PS2x][1] == 0x14 )
				{
					PS2_Process_Keyboard_Package_Sub(PS2x,KB_TYPE_MAKE_E1,indata); // this should be 0x77
				}
				else
				{
					if ( indata == 0x77 ) // skip the 0x14, and 0xf0 before the 0x77
						PS2_Process_Keyboard_Package_Sub(PS2x,KB_TYPE_BREAK_E1,indata); // this should be 0x77 break
					else
						PS2_Process_Keyboard_AddKB_Buf(PS2x,indata);
				}
			}
			break;
		default :
			PS2_Keyboard_Package_Index[PS2x] = 0;
			break;
	}
PROCESS_PS2_KEYBOARD_END:
	return;
}

/*
 * ----------------------------------------------------------------------------
 * void PS2_Keyboad_LED_Script(PS2Port_TypeDef PS2x,U8_T led)
 * Purpose : set ps/2 keyboard led control script 
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 * Returns : 
 * Note    : 
 * ----------------------------------------------------------------------------
 */
void PS2_Keyboad_LED_Script(PS2Port_TypeDef PS2x,U8_T led)
{
	PS2_cfgScriptKB_Led[1].Parameter1 = (led & 0x07); // assign led to keyboard
	PS2_CfgCtlTab[PS2x].ScriptPtr = (U8_T *)PS2_cfgScriptKB_Led;
	PS2_CfgCtlTab[PS2x].ScriptCnt = sizeof(PS2_cfgScriptKB_Led)/sizeof(struct _PS2_CfgScriptStruct);
	PS2_CfgCtlTab[PS2x].Script = CONFIG_SCRIPT_KB_LED; // need to check the type first
	PS2_Script_Cfg_Start(PS2x,&PS2_CfgCtlTab[PS2x]);
}

/*
 * ----------------------------------------------------------------------------
 * void      TASK_PS2_Host_Start_Initlial
 * Purpose : This task will initial all the ps2 device, 
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 * Returns : 
 * Note    : 
 * ----------------------------------------------------------------------------
 */ 
void TASK_PS2_Host_Start_Initlial(void)
{
	if (PS2_Start_Init_Flag)
	{
		for (; PS2_Start_Init_Index < (PS2_PORT_MAX_NUM); PS2_Start_Init_Index++)
		{
			if (PS2_Start_Init_Flag & (0x01 << PS2_Start_Init_Index))
			{
				PS2_Start_Init_Flag &= ~(0x01 << PS2_Start_Init_Index); // clear the init flag
				PS2_Config_Start_Standard(PS2_Start_Init_Index);
				PS2_Start_Init_Index++;
				break;
			}
		}
	}
	if (PS2_Start_Init_Flag == 0)
	{
		TASK_Destory_Current();
	}
}

/*
 * ----------------------------------------------------------------------------
 * void      TASK_PS2_TimeOut_Active_Check
 * Purpose : This task will initial all the ps2 device, 
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 * Returns : 
 * Note    : 
 * ----------------------------------------------------------------------------
 */
void TASK_PS2_TimeOut_Active_Check(PS2Port_TypeDef PS2x,U16_T timeset)
{
	U8_T	index,taskid;

	for (index=0; index < Task_MS_Period_Table_WP ; index++)
	{
		taskid = Task_MS_Period_Table[index];
		if (Task_Active_Table[taskid].Task_ID == TASK_PS2_TIMEOUT_CHECK_ID)
		{
			if (Task_Active_Table[taskid].Task_Para == PS2x)
			{
				Task_Active_Table[taskid].Task_Interval.w = timeset;
				Task_Active_Table[taskid].Task_Reload.w = timeset;
				PS2_TASK_Timeout_ID[PS2x] = taskid;
				return;
			}
		}
	}
	PS2_TASK_Timeout_ID[PS2x] = TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_PS2_TIMEOUT_CHECK_ID,0x00,PS2x,timeset,timeset);
}

/*
 * ----------------------------------------------------------------------------
 * void      PS2_Reset_Counter
 * Purpose : This task will initial all the ps2 device, 
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 * Returns : 
 * Note    : 
 * ----------------------------------------------------------------------------
 */
//void PS2_Reset_Counter(void)
//{
//	U8_T	ps2index;
//
//	for (ps2index=0; ps2index < (PS2_PORT_MAX_NUM); ps2index++)
//	{
//		if (PS2_Control[ps2index] == PS2_KB)
//		{
//			PS2_Keyboard_Package_Index[ps2index]= 0;
//			PS2_KB_Balance_Cnt[ps2index] = 0;
//		}
//	}
//}

/*
 * ----------------------------------------------------------------------------
 * void      TASK_PS2_Balance_Maintain
 * Purpose : This task will maintain the balance counter if last for 1 seconds
 *           still have the balance counter, then reset to 0
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 * Returns : 
 * Note    : 
 * ----------------------------------------------------------------------------
 */
void TASK_PS2_Balance_Maintain(void)
{
	U8_T	ps2inx;

	ps2inx = TASK_Register0;
	PS2_KB_Balance_CountDown[ps2inx]++;
	if (PS2_KB_Balance_CountDown[ps2inx] >= 10)
	{
		PS2_KB_Balance_Cnt[ps2inx] = 0; // the receive counter
		PS2_KB_Balance_CountDown[ps2inx]=0; // the receive counter
		PS2_KB_Last_Code[ps2inx]=0; 
		PS2_KB_Last_Type[ps2inx]=0; 
		TASK_Destory_Current();
		PS2_KB_Balance_CountDownTask &= ~PS2_PORT_BIT[ps2inx];
	}
}

/*
 * ----------------------------------------------------------------------------
 * void      TASK_PS2_Device_Resend
 * Purpose : This task will send out 0xfe to ask device resend the last byte
 *           the interval time is 5ms
 * Params  : @PS2x: Select the PS/2 peripheral. 
 *           this parameter can be one of the following values:
 *           HPS2A, HPS2B=>hardeare 
 *           EPS2A, EPS2B  =>external interrupt
 * Returns : 
 * Note    : 
 * ----------------------------------------------------------------------------
 */
void TASK_PS2_Device_Resend(void)
{
	U8_T	PS2x;

	PS2x = TASK_Register0;
	PS2_CfgCtlTab[PS2x].Receive_Error++;
	if ( PS2_CfgCtlTab[PS2x].Receive_Error < SYSTEM_PS2_RESEND_MAX )
	{
		//Send out 0xfe, but not interrupt the sequence.
		PS2_Transmit_Buf[PS2x] = 0xfe; // move para1 to output buffer
		PS2_Status[PS2x] |= (PS2_TRANSMIT_START_FLAG|PS2_WRITE_FLAG|PS2_ACT|PS2_RECEIVE_RESEND_FLAG); // now transmit
		PS2_Transmit_Start_Bit(PS2x);
	}
	TASK_Destory_Current();
}

/*
 *--------------------------------------------------------------------------------
 * void PS2H_Read_Regs(U8_T regIndex, U8_T *pRegData, U8_T regLen)
 * Purpose: Read data bytes from a register with interrupt locked.
 * Params : regIndex:An index address to interface register.
 *          pRegData:A pointer to indicate the register data.
 *          regLen:A number of bytes to indicate how many bytes will be read.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void PS2H_Read_Regs(U8_T regIndex, U8_T *pRegData, U8_T regLen)
{
	EXTINT4_DISABLE;
	PS2CIR = regIndex;
	while (regLen --)
	{
		*(pRegData + regLen) = PS2DR;
	}
	EXTINT4_ENABLE;
}

/*
 *--------------------------------------------------------------------------------
 * void PS2H_Write_Regs(U8_T regIndex, U8_T *pRegData, U8_T regLen)
 * Purpose: write data bytes from a register with interrupt locked.
 * Params : regIndex:An index address to interface register.
 *          pRegData:A pointer to indicate the register data.
 *          regLen:A number of bytes to indicate how many bytes will be read.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
//void PS2H_Write_Regs(U8_T regIndex, U8_T *pRegData, U8_T regLen)
//{
//	EXTINT4_DISABLE;	
//	while (regLen --)
//	{
//		PS2DR = *(pRegData + regLen);
//	}
//	PS2CIR = regIndex;
//	EXTINT4_ENABLE;
//}

#endif /* End of SYSTEM_PS2_HOST_ENABLE */

/* End of ps2_host.c */
