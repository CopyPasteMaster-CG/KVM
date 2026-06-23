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
 * Module Name : usbhc_roothub.c
 * Purpose     : The handler of root hub device
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	<string.h>
#include	<stdio.h>
#include	"project_include.h"

#define   ROOT_HUB_RESET_COUNTER     2

/* STATIC VARIABLE DECLARATIONS */ 
/*Constant or Code Declare ------------------------------------- */
code U8_T RootHubPortID[USB_HC_MAX_PORT] = {
#ifdef MCU_TYPE_AX68002
	0x01,
	0x02
#else
	0x01,
	0x02,
	0x04,
	0x08
#endif
};

/* Flag Declare ---------------------------- */
bit		USBHC_RootHub_Bus_Occupy_Flag;

/* Global Variable Declare ------------------------------------ */
U8_T	USBHC_Port_Power_On_State;         // for each bit standard for a port 
U8_T	USBHC_RootHub_State_Hold[USB_HC_MAX_PORT]; // the current Root Hub State
U8_T	USBHC_RootHub_Devinx[USB_HC_MAX_PORT];     // Store the devinx which is connect to root hub
U8_T	USBHC_RootHub_State_Report[USB_HC_MAX_PORT];
U8_T	USBHC_RootHub_StatusChanged_Report[USB_HC_MAX_PORT];
U8_T	USBHC_RootHub_State_OldReport[USB_HC_MAX_PORT];
U8_T	USBHC_RootHub_Bus_Occupy_ID;
U8_T	TASK_USBHC_RootHub_Reset_ID;
U8_T	TASK_USBHC_RootHub_OverCurrent_ID;
U8_T	TASK_USBHC_RootHub_Enumerate_Start_ID;
U8_T	USBHC_RootHub_ResetTaskSerial[USB_HC_MAX_PORT];
U8_T	USBHC_RootHub_ResetCnt;
U8_T	USBHC_RootHub_EnableErr[USB_HC_MAX_PORT];
U8_T	USBHC_RootHub_HcTdActivityRecord;
U8_T	USBHC_RootHub_HcTdActivityFailCnt;

TASK_ActiveTable_TypeDef USBHC_RootHub_PowerOn_Task = {
	TASK_TYPE_INTERVAL_MS, // Task ID from 0~255
	0, // Task ID from 0~255
	0, // Task Wait for Semaphore to active
	0, // Task Event
	0, // Task transfer parameter
	100, // Task Interval time from 0~65535(ms/us)
	100, // Task Interval Reload
};

TASK_ActiveTable_TypeDef TASK_USBHC_HUB_RemoveDevinx_Task = {
	//TASK_TYPE_EVENT, // Task ID from 0~255
	TASK_TYPE_INTERVAL_MS,
	0, // Task ID from 0~255
	0, // Task Wait for Semaphore to active
	0, // Task Event
	0, // Task transfer parameter
	2, // Task Interval time from 0~65535(ms/us)
	2, // Task Interval Reload
};

/* LOCAL SUBPROGRAM DECLARATIONS */
void TASK_USBHC_RootHub_Reset(void);
void USBHC_RootHub_PowerON(U8_T rhpid);
void TASK_USBHC_RootHub_OverCurrent(void);
void TASK_USBHC_RootHub_Enumerate_Start(void);

/* External Global Variable Declare ------------------------------------ */
extern U8_T   USBHC_HUB_Enumerate_Occupy_Flag;

/* LOCAL SUBPROGRAM BODIES */

/*
 * ----------------------------------------------------------------------------
 * void USBHC_RootHub_Init(void)
 * Purpose : 1.Init Memory 
 *           2.Assign TD buffer address into register(0x31)
 *           3.Assign different TD buffer information into register
 *             a.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void USBHC_RootHub_Init(void)
{
	U8_T control_reg;
	//1.Initial Power ON Root Hub relative control parameter
	USBHC_Port_Power_On_State = 0x0f; //bit0,bit1,bit2,bit3=> need to power on 4 ports

	//2.OverCurrent ReCheck
	//3.Set Descriptor
	EXTINT2_DISABLE;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	EXTINT3_DISABLE;
#endif

	/*Read Back the Register00,first*/
	_USBHC_HCCIR_SFR(HC_CONTROL_REG);
	_USBHC_HCDR_READ_SFR(control_reg);
	control_reg |= HC_PWP_SET;
	_USBHC_HCDR_SFR(control_reg);
	_USBHC_HCCIR_SFR(HC_CONTROL_REG);
	EXTINT2_ENABLE;
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	EXTINT3_ENABLE;
#endif

#if (USBHC_INIT_DEBUG_MODE)
	USBHC_Reg_Dump(HC_CONTROL_REG,1);
	USBHC_Reg_Dump(HC_ROOTHUB_DESCRIPTOR_REG,1);
#endif
	//4.Handler Task for Powr On Init & PortStatus Change
	//5.Init Root Hub Status Change Control Table
	memset(USBHC_RootHub_State_Hold, 0x00,sizeof(USBHC_RootHub_State_Hold));
	memset(USBHC_RootHub_State_Report, 0x00,sizeof(USBHC_RootHub_State_Report));
	memset(USBHC_RootHub_State_OldReport,0x00,sizeof(USBHC_RootHub_State_OldReport));
	memset(USBHC_RootHub_StatusChanged_Report, 0x00,sizeof(USBHC_RootHub_StatusChanged_Report));
	memset(USBHC_RootHub_ResetTaskSerial, 0x00,sizeof(USBHC_RootHub_ResetTaskSerial));
	memset(USBHC_RootHub_EnableErr, 0x00,sizeof(USBHC_RootHub_EnableErr));
	memset(USBHC_RootHub_Devinx, 0x00,sizeof(USBHC_RootHub_Devinx));
	USBHC_RootHub_Bus_Occupy_ID = 0;
	USBHC_RootHub_Bus_Occupy_Flag = 0;
	USBHC_RootHub_ResetCnt = 0;
	USBHC_RootHub_HcTdActivityRecord = 0;
	USBHC_RootHub_HcTdActivityFailCnt = 0;
	//Task Control ....................
	USBHC_RootHub_PowerOn_Task.Task_ID  = TASK_Create(TASK_USBHC_RootHub_PowerON);
	TASK_USBHC_RootHub_Reset_ID = TASK_Create(TASK_USBHC_RootHub_Reset);
	TASK_USBHC_RootHub_OverCurrent_ID = TASK_Create(TASK_USBHC_RootHub_OverCurrent);
	TASK_USBHC_RootHub_Enumerate_Start_ID = TASK_Create(TASK_USBHC_RootHub_Enumerate_Start);
	TASK_USBHC_HUB_RemoveDevinx_Task.Task_ID = TASK_Create(TASK_USBHC_HUB_RemoveDevinx);
	//Star the Power Stage      
	TASK_Run(&USBHC_RootHub_PowerOn_Task);
	//USBHC_PDevice_ResetIndex = 0;
#if (SYSTEM_TASK_DUMP_SUPPORT)
	printf("USBHC_RootHub_PowerOn_Task.Task_ID=%bu\n\r",USBHC_RootHub_PowerOn_Task.Task_ID);
	printf("TASK_USBHC_RootHub_Reset_ID=%bu\n\r",TASK_USBHC_RootHub_Reset_ID);
	printf("TASK_USBHC_RootHub_OverCurrent_ID=%bu\n\r",TASK_USBHC_RootHub_OverCurrent_ID);
	printf("TASK_USBHC_RootHub_Enumerate_Start_ID=%bu\n\r",TASK_USBHC_RootHub_Enumerate_Start_ID);
	printf("TASK_USBHC_HUB_RemoveDevinx_Task.Task_ID=%bu\n\r",TASK_USBHC_HUB_RemoveDevinx_Task.Task_ID);
#endif
} /* End of USBHC_RootHub_Init */

/*
 * -------------------------------------------------------------------------------------------
 * void USBHC_RootHub_StatusChanged_Handle(U8_T portid)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------
 */
void USBHC_RootHub_StatusChanged_Handle(void)
{
	U8_T	port, op_state, portmask;
	U8_T	port_status, port_status_change;
	U8_T	free_deviceid;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//$1.Connect Status Change => Do the Reset 
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	op_state = USBHC_ISR_FIFO[USBHC_ISRQ_Rp].OP_ISR_State;
	portmask = HC_ROOTHUB_STATUSCHANGED_PORT0_MASK;
	for (port = 0; port < USB_HC_MAX_PORT; port++)
	{
		if (op_state & portmask)
		{
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			//(1).Read in the PORT status & changed status
			//EXTINT2_DISABLE;
			port_status = USBHC_RootHub_State_Report[port];
			port_status_change = USBHC_RootHub_StatusChanged_Report[port];
			//EXTINT2_ENABLE;

#if (USBHC_ROOTHUB_DEBUG_MODE)
			printf("A_(RH):P:%d,(St=0x%02x,Ch=0x%02x)=>",(U16_T)port,(U16_T)port_status,(U16_T)port_status_change);
#endif
			//handle some strange errors, when unplug the device, state change not report
			if (port_status == 0x60) //HC_RH_STATE_PES_SET|HC_RH_STATE_PSS_SET
			{
				if (port_status_change == 0x00) 
				{
					printf("force");
					port_status_change = 0x03; //HC_RH_CHANGE_CSC_SET|HC_RH_CHANGE_PESC_SET
				}
			}				
			if (port_status_change)
			{	
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				//$1.Overcnrrent Indicate Changed =>1.After over current occur, the state 
				//                                    could be suspend, or over current
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				if (port_status_change & HC_RH_CHANGE_POCIC_MASK)
				{
					if (port_status & HC_RH_STATE_POCI_MASK)
					{
	#if (USBHC_ROOTHUB_DEBUG_MODE)
						printf("Over current occurs, Port = %bu\n\r", port);
	#endif
#if (KVM_BUZZER_HARDWARE)
						BUZZER_Script_Active(DeviceOverCurrentSound);
#endif						
						
						if (USB_HAL_Check_PDevice_Status(port))
						{
	#if USBHC_OC_FORCE_SW_RESET
							printf("Do software reset after 3 seconds\n\r");
							TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_USBHC_RootHub_OverCurrent_ID,0,port,3000,3000);
	#else
							EXTINT2_DISABLE;
							USBHC_RootHub_HcTdActivityRecord = USBHC_TD_Activity_Cnt;
							EXTINT2_ENABLE;
							printf("Monitor HC activity...\n\r");
							TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_USBHC_RootHub_OverCurrent_ID,0,port,1000,1000);
	#endif
						}	
						
						USBHC_RootHub_State_Hold[port] |= HC_RH_STATE_POCI_MASK;
						if (USBHC_RootHub_Devinx[port] >= USBDC_VIRTUAL_DEVINX)
						{
							TASK_USBHC_HUB_RemoveDevinx_Fork(USBHC_RootHub_Devinx[port]);
	#if (USBHC_ROOTHUB_DEBUG_MODE)
							printf("Clear Port..\n\r");
	#endif
						}
					}
					else
					{
	//#if (USBHC_ROOTHUB_DEBUG_MODE)
						printf("Over current clear, Port = %bu\n\r", port);
	//#endif
						USBHC_RootHub_State_Hold[port] &= ~HC_RH_STATE_POCI_MASK;
						USBHC_RootHub_PowerON(port);
					}
				}
	
				//--------------------------------
				//2.Check Connect Status Changed
				if (port_status_change & HC_RH_CHANGE_CSC_MASK)
				{
	#if (USBHC_ROOTHUB_DEBUG_MODE)
					printf("@Connect_Ch@,Connect=%s\n\r",(port_status & HC_RH_STATE_CCS_MASK) ? "Y" : "N");
	#endif

ROOT_HUB_CONNECT_CHANGED:	
	/*				
	#if (SYSTEM_EXTENDER_RECEIVER)
					if (KVM_CurrentHost != REMOTE_HOST_PORT)	
	#endif
					{
						USBDC_WakeupUpstreamHost(KVM_CurrentHost);
					}
	*/
					// if Connection codition is not same with hold table
					if ((USBHC_RootHub_State_Hold[port]&HC_RH_STATE_CCS_MASK) != (port_status&HC_RH_STATE_CCS_MASK))
					{
						//--------------------------------------------------------
						// Device Connected
						// if no other device is during reset, then reset now
						//--------------------------------------------------------
						if (port_status & HC_RH_STATE_CCS_MASK) //device attached in
						{
							if (USBHC_Current_Total_Device_Count < USB_HC_MAX_DEVICE)
							{
	#if (USBHC_ROOTHUB_DEBUG_MODE)
								printf("Next=>Reset\n\r");
	#endif	
								//USBHC_RootHub_Reset_Port(port,100 * (port + 1));
								USBHC_RootHub_Reset_Port(port,30);
							}
							else
							{
	#if (USBHC_ROOTHUB_DEBUG_MODE)
							
								printf("OverMaxDevices=>Ignore %bx\n\r", USBHC_Current_Total_Device_Count);
	#else
	#if (KVM_BUZZER_SUPPORT)
								BUZZER_Script_Active(DeviceErrorSound);
	#endif //#if (KVM_BUZZER_SUPPORT)
								printf("> KVM ERROR:No Free device, Skip operation\n\r");
	#endif
							}
						}
						else //Device disconnected
						{
							USBHC_RootHub_State_Hold[port] &= ~HC_RH_STATE_CCS_MASK;
							USBHC_RootHub_EnableErr[port] = 0;
							USBHC_RootHub_State_OldReport[port] = 0;
							if (USBHC_RootHub_Bus_Occupy_Flag && (USBHC_RootHub_Bus_Occupy_ID == port))
							{
								USBHC_RootHub_Bus_Occupy_Flag = 0; // release the bus to other port
							}
	#if (USBHC_DEBUG_MODE)
							printf("Clear Port Operation\n\r");
	#endif	
							if (USBHC_RootHub_Devinx[port] >= USBDC_VIRTUAL_DEVINX)
							{								
								TASK_USBHC_HUB_RemoveDevinx_Fork(USBHC_RootHub_Devinx[port]);
								USBHC_RootHub_Devinx[port] = 0;
							}
						}
					}
	#if (USBHC_ROOTHUB_DEBUG_MODE)
					else
					{
						printf("Skip\n\r");
					}
	#endif
				}
	
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				//$3.Port Enable Status Change =>1.After reset, the device should enter 
				//                                 this mode automatically
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				if (port_status_change & HC_RH_CHANGE_PESC_MASK)
				{
	#if (USBHC_ROOTHUB_DEBUG_MODE)
					printf("@Port_Enable_Ch@,Enable=%s\n\r",(port_status&HC_RH_STATE_PES_MASK) ? "Y" : "N");
	#endif
					if ((USBHC_RootHub_State_Hold[port]&HC_RH_STATE_PES_MASK) != (port_status&HC_RH_STATE_PES_MASK))
					{
						if (port_status&HC_RH_STATE_PES_MASK)
						{
							USBHC_RootHub_State_Hold[port] |= HC_RH_STATE_PES_MASK;
						}
						else
						{
							USBHC_RootHub_State_Hold[port] &= ~HC_RH_STATE_PES_MASK;
						}
					}
	
					if ((port_status&HC_RH_STATE_PES_MASK) == HC_RH_STATE_PES_RESET) // if port is reset status changed
					{
						if ((port_status & (HC_RH_STATE_CCS_MASK|HC_RH_STATE_PPS_MASK)) == (HC_RH_STATE_CCS_MASK|HC_RH_STATE_PPS_MASK)) //if power on & connect
						{
							if (USBHC_RootHub_Devinx[port] >= USBDC_VIRTUAL_DEVINX)
							{								 
								if (USB_PDevice[USBHC_RootHub_Devinx[port]].Addr & USBHC_DEVICE_USED_MASK)  // if Device in used, then clear it
								{									
								 	TASK_USBHC_HUB_RemoveDevinx_Fork(USBHC_RootHub_Devinx[port]);							 	
								}  								
																																									
								USBHC_RootHub_Devinx[port] = 0;
	#if (USBHC_ROOTHUB_DEBUG_MODE)
								printf("@Operation=>Reset Port\n\r");
	#endif
								if (USBHC_RootHub_EnableErr[port] < 5)
								{
									if (USBHC_RootHub_ResetCnt == 0) // start ot reset
									{		
										//USBHC_RootHub_Reset_Port(port,100 * (port + 1));
										USBHC_RootHub_Reset_Port(port,30);
										USBHC_RootHub_ResetCnt = 1; // start ot reset
									}								
								}	
							}
						}
					}
				}
	
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				//$4.Suspend Status Change =>1.After resume, the device active again
				//                           2.After send out suspend command,device sleep
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				if (port_status_change&HC_RH_CHANGE_PSSC_MASK)
				{
	#if (USBHC_ROOTHUB_DEBUG_MODE)
					printf("@Port_Suspend_Ch@,Suspend=%s,",(port_status&HC_RH_STATE_PSS_MASK)? "Y" : "N");
	#endif
					//Check suspend status
					if ((USBHC_RootHub_State_Hold[port]&HC_RH_STATE_PSS_MASK) != (port_status&HC_RH_STATE_PSS_MASK))
					{
						if (port_status&HC_RH_STATE_PSS_MASK)
						{
	#if (USBHC_ROOTHUB_DEBUG_MODE)
							printf("Enter Suspend Mode\n\r");
	#endif
							USBHC_RootHub_State_Hold[port] |= HC_RH_STATE_PSS_MASK;
						}
						else // get into suspend mode
						{
	#if (USBHC_ROOTHUB_DEBUG_MODE)
							printf("Leave Suspend Mode\n\r");
	#endif
							USBHC_RootHub_State_Hold[port] &= ~HC_RH_STATE_PSS_MASK;
						}
					}
	#if (USBHC_ROOTHUB_DEBUG_MODE) 
					else
					{
						// Nothing happened
						printf("Skip\n\r");
					}
	#endif
				}
	
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				//$5.Reset Complete
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				if (port_status_change & HC_RH_CHANGE_PRSC_MASK)
				{
ROOT_HUB_RESET_CHANGED:					
					//Port Enable & Connect
					if ((port_status &(HC_RH_STATE_PES_MASK|HC_RH_STATE_CCS_MASK)) == (HC_RH_STATE_PES_MASK|HC_RH_STATE_CCS_MASK))
					{
						//in root hub port, the port0 will assign to device 0, port 3 assing to device 3
						if (USBHC_RootHub_ResetCnt >= ROOT_HUB_RESET_COUNTER)
						{
	#if (USBHC_ROOTHUB_DEBUG_MODE)
							printf("@Port_Reset_Ch@,DONE");
							printf(",Enable=%s,Connect=%s,",(port_status&HC_RH_STATE_PES_MASK) ? "Y" : "N",
															(port_status&HC_RH_STATE_CCS_MASK) ? "Y" : "N");
	#endif
							//Check the CurrentPort Connection
							//This is happened only reset the downstream port
							if (USBHC_RootHub_Devinx[port] != 0)
							{								
								free_deviceid = USBHC_RootHub_Devinx[port];
								goto USBHC_ROOT_HUB_GO_ENUMERATION;
							}
	
							if (USB_HAL_Alloc_Free_PDevice(&free_deviceid, port))
							{
								//1.Assign device address 0								
								USBHC_RootHub_Devinx[port] = free_deviceid;
								
								//log the devinx into hub table
	//							(USB_PDevice[0].HUB.ReportState+port)->Devinx = free_deviceid;	
								USB_PDevice[free_deviceid].Hub_NbrPorts = (port USBHC_ROOTHUBPORT_PORT_NUM_SHIFT);
								//2.Store device speed.
								if (port_status & HC_RH_STATE_LSDA_MASK)     // Low Speed 
									USB_PDevice[free_deviceid].InterfaceNum |= USBHC_DEVICE_SPEED_MASK;  // store speed type
	
	#ifdef USBHC_ROOT_FULL_SEED_TEST
								if (port_status&HC_RH_STATE_LSDA_MASK) //low speed device
								{
									printf("\n\r !!! SPEED Not matched\n\r");
								}
								else
								{
									printf("OK\n\r");
								}
	#endif
	#ifdef USBHC_ROOT_LOW_SEED_TEST
								if ((port_status&HC_RH_STATE_LSDA_MASK) == 0x00) //Full Speed
								{
									printf("\n\r !!! SPEED Not matched\n\r");
								}
								else
								{
									printf("OK\n\r");
								}
	#endif
								//4.Assign device attatched task
	USBHC_ROOT_HUB_GO_ENUMERATION:
								USBHC_Enumerate_New_Addr = port + 1; // for Root HUB Layer, start from 0x01~0x04
								USB_PDevice[free_deviceid].Hc.gState = HOST_DEV_ATTACHED; // Do the root hub reset process
								USBHC_RootHub_EnableErr[port] = 0; // reset the enable error counter
								USBHC_RootHub_ResetCnt = 0;
								TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_USBHC_RootHub_Enumerate_Start_ID,0x00,free_deviceid,200,200);  // Generate the task
	#if (USBHC_ROOTHUB_DEBUG_MODE)
								printf("Devinx:%bd,Next(Enumerate)\n\r",free_deviceid);
	#endif
							}
							else
							{
								USBHC_RootHub_Bus_Occupy_Flag = 0;
							}
						}
					}
					else
					{
	#if (USBHC_ROOTHUB_DEBUG_MODE)
						printf("Error Condition\n\r");
	#endif
						USBHC_RootHub_Bus_Occupy_Flag = 0;
					}
				}
			}
			else
			{
#if (USBHC_ROOTHUB_DEBUG_MODE)				
				printf("B_(RH):P:%d,(St=0x%02x,Ch=0x%02x)=>",(U16_T)port,(U16_T)port_status,(U16_T)port_status_change);
#endif				
				//1.Check device connect
				if ((USBHC_RootHub_State_OldReport[port] & HC_RH_STATE_CCS_MASK) == 0x00)
				{	
					if (USBHC_RootHub_State_Report[port] & HC_RH_STATE_CCS_MASK)
					{
#if (USBHC_ROOTHUB_DEBUG_MODE)						
						printf("Connect Changed\n\r");
#endif						
						goto ROOT_HUB_CONNECT_CHANGED;
					}	
				}
				//2.Check the reset complete
				if (USBHC_RootHub_State_OldReport[port] & HC_RH_STATE_PRS_MASK)
				{	
					if ((USBHC_RootHub_State_Report[port] & HC_RH_STATE_PRS_MASK) == 0x00)
					{
#if (USBHC_ROOTHUB_DEBUG_MODE)						
						printf("Reset Change\n\r");
#endif						
						goto ROOT_HUB_RESET_CHANGED;
					}	
				}		
			}	
			USBHC_RootHub_State_OldReport[port] = USBHC_RootHub_State_Report[port];
		}

		op_state &= ~portmask;
		if ((op_state & HC_OPINT_STATE_RHSC_MAP_MASK) == 0)
		{
			break;
		}

		portmask = portmask << 1; // notice this must check with SFR
	}
}

/*
 * -------------------------------------------------------------------------------------------
 * void USBHC_RootHub_PowerON(U8_T portid)
 * Purpose :  
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------
 */
void USBHC_RootHub_PowerON(U8_T rhpid)
{
	U8_T reg[2];

	reg[0] = HC_RH_CON_SPP_SET;
	reg[1] = 0;
	USBHC_Write_Regs(USBHC_RootHub_Cmd_Reg[rhpid],reg,2);
#if (USBHC_ROOTHUB_DEBUG_MODE)
	printf("USBHC:@Port(%bu),PowerOn\n\r", rhpid);
#endif
}

/*
 * -------------------------------------------------------------------------------------------
 * void TASK_USBHC_RootHub_PowerON(void)
 * Purpose : Will do the Root hub initial process after the hc controller reset
 * Params  :   
 * Returns : none
 * Note    : 
 *           root hub power on flow:
 *           Power Power On => generate a interrupt of port chagned
 * ISR       root hub state:0x61,0x01 => Connect Change+Power On status
 *           Port Reset=> need 30ms (generate a interrupt of port chagned)
 * ISR       root hub state:0x63,0x10 => Reste Complete,Power On+Connect+Enable
 * -------------------------------------------------------------------------------------------
 */
void TASK_USBHC_RootHub_PowerON(void)
{
	U8_T port;

	for (port=0; port < USB_HC_MAX_PORT; port++)
	{
		if (USBHC_Port_Power_On_State & RootHubPortID[port])
		{
			USBHC_RootHub_PowerON(port);
			USBHC_Port_Power_On_State &= ~RootHubPortID[port];
		}
	}
	TASK_Destory_Current();
} 

/*
 * -------------------------------------------------------------------------------------------
 * void TASK_USBHC_RootHub_Reset(void)
 * Purpose : Monitor the Will do the Root hub housekeeping maintain process
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------
 */
void TASK_USBHC_RootHub_Reset(void)
{
	U8_T rhpid,reg[2];

	reg[0] = TASK_Event & TASK_EVENT_SERIAL_MASK;  // get Port ID
	rhpid  = TASK_Register0;  // get Port ID
		
	if (reg[0] != USBHC_RootHub_ResetTaskSerial[rhpid])
	{	
		TASK_Destory_Current();
		return;
	}
	
	if ((USBHC_RootHub_State_Hold[rhpid] & HC_RH_STATE_CCS_MASK) == HC_RH_STATE_CCS_SET) // if device still connected
	{
		if (USBHC_RootHub_Bus_Occupy_Flag == 0) // wait for other port reset complete
		{
			if (USBHC_HUB_Enumerate_Occupy_Flag == 0) /*if no other hub port in enumearation opeartion */
			{
				//1.Fill Up root hub port reset register				
				USBHC_RootHub_Bus_Occupy_Flag = 1;
				USBHC_RootHub_Bus_Occupy_ID = rhpid;

				//Port Reset
				reg[0] = HC_RH_CON_SPR_SET;
				reg[1] = 0;
				USBHC_Write_Regs(USBHC_RootHub_Cmd_Reg[rhpid],reg,2);
				USBHC_RootHub_ResetCnt=1;
#if (USBHC_ROOTHUB_DEBUG_MODE)
				printf("0USBHC:Port(%d),Reset %d Times..\n\r",(U16_T)rhpid,(U16_T)USBHC_RootHub_ResetCnt);
#endif
			}
		}
		else
		{
			if (USBHC_RootHub_Bus_Occupy_ID == rhpid) // this is second time to get 
			{
				USBHC_RootHub_ResetCnt++;
#if (USBHC_ROOTHUB_DEBUG_MODE)
				printf("1USBHC:Port(%d),Reset %d Times..\n\r",(U16_T)rhpid,(U16_T)USBHC_RootHub_ResetCnt);
#endif
				//Port Reset
				if (USBHC_RootHub_ResetCnt == ROOT_HUB_RESET_COUNTER)
				{
					reg[0] = HC_RH_CON_SPR_SET;
					reg[1] = 0;
					USBHC_Write_Regs(USBHC_RootHub_Cmd_Reg[rhpid],reg,2);
				}

				if (USBHC_RootHub_ResetCnt >= ROOT_HUB_RESET_COUNTER)
				{
					TASK_Destory_Current(); // terminiate now
				}
			}
		}
	}
	else
	{
#if (USBHC_ROOTHUB_DEBUG_MODE)		
		printf("USBHC:Cancel reset for device disconnected\n\r");		
#endif		
		if (USBHC_RootHub_Bus_Occupy_ID == rhpid) // this is second time to get 
		{
			USBHC_RootHub_Bus_Occupy_Flag = 0;
		}	
		TASK_Destory_Current();
	}
}

/** -------------------------------------------------------------------------------------------
 * void TASK_USBHC_RootHub_OverCurrent(void)
 * Purpose : After the overcurrent status happenied, need software to set power on, and check
 *           the ovrecurrent release condition.
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------*/
void TASK_USBHC_RootHub_OverCurrent(void)
{
#if USBHC_OC_FORCE_SW_RESET
	CPU_SoftReboot();
	TASK_Destory_Current();
#else
	/* Check hardware TD activity */
	if (USBHC_RootHub_HcTdActivityRecord == USBHC_TD_Activity_Cnt)
	{
		/* TD not activity, record fail count */
		if (++USBHC_RootHub_HcTdActivityFailCnt >= 3)
		{
			/* HC is deadlock, do software reset */
			USBHC_RootHub_HcTdActivityFailCnt = 0;
			TASK_Destory_Current();
			//printf("Do software reset\n\r");
			CPU_SoftReboot();
			return;
		}
		/* Wait for the checking next time */
	}
	else
	{
		/* HC is still alived, destroy current task */
		//printf("HC TD still alived!!\n\r");
		TASK_Destory_Current();
	}
#endif
}

/** -------------------------------------------------------------------------------------------
 * void TASK_USBHC_RootHub_Enumerate_Start(void)
 * Purpose : After Reset complete, there is a buffer time to send out the first command 
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------*/
void TASK_USBHC_RootHub_Enumerate_Start(void)
{
	TASK_Destory_Current();
	TASK_Active(TASK_TYPE_USB,TASK_USBHC_Process_ID,0x00,TASK_Register0,0,0);  // Generate the task
}

/** -------------------------------------------------------------------------------------------
 * void USBHC_RootHub_Reset_Port(void)
 * Purpose : Rset the root hub port
 * Params  :  
 * Returns : none
 * Note    : none
 * -------------------------------------------------------------------------------------------*/
void USBHC_RootHub_Reset_Port(U8_T port,U16_T reset_timer)
{
	USBHC_RootHub_ResetTaskSerial[port]++;
	USBHC_RootHub_ResetTaskSerial[port] &= 0x0f;
	USBHC_RootHub_State_Hold[port] |= HC_RH_STATE_CCS_MASK;
	USBHC_RootHub_ResetCnt = 0; // start ot reset
	TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_USBHC_RootHub_Reset_ID,USBHC_RootHub_ResetTaskSerial[port],port,reset_timer,200);
}

/* End of usbhc_roothub.c */
