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
 * Module Name : main.c
 * Purpose     :
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 *================================================================================
 */
/* INCLUDE FILE DECLARATIONS */
#include	<string.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	"project_include.h"

/* GLOBAL CONSTANT VARIABLES DECLARATIONS */
#define VERSION_STRING "2.1.3.4"
#define DATE_STRING    "2023-05-24"

/* GLOBAL VARIABLES DECLARATIONS */
#ifdef DEVICE_GPIO_DEFINE
bit	Audio_Device_Support_Gpio=0;
bit	MSC_Device_Support_Gpio=0;
#endif /* #ifdef DEVICE_GPIO_DEFINE */
ISR_TypeDef		ISR_FIFO[ISR_FIFO_DEPTH];
U8_T			ISR_FIFO_Rp,ISR_FIFO_Wp;


/* STATIC VARIABLE DECLARATIONS  */
/* LOCAL SUBPROGRAM DECLARATIONS */
/* LOCAL SUBPROGRAM BODIES       */
#if (KVM_INFORMATION_DISPLAY)
void DisplayKVMInformation(void);
#endif
void DisplayProjectVersionInformation(void);	
void Check_Devcie_Gpio_Setting(void);
/* EXTERNAL VARIABLE DECLATATIONS */
/* EXTERNAL SUBPROGRAM DECLATATIONS */

/*
 *--------------------------------------------------------------------------------
 * void main(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void main(void)
{
#if ((SYSTEM_EXTENDER_RECEIVER) && (SYSTEM_USBAUDIO_DEVICE_SUPPORT) && (SYSTEM_EA_CONTROL))
	bit tmpIsr;
#endif

	/* MCPU core initial */
	CPU_Init();
	/* UART port initial */
	UART_Init();

	/* ms timer initialize */
	SWTIMER_Init();
	SWTIMER_Start();
	Check_Devcie_Gpio_Setting();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	//				V I N S O N __ P R O J E C T
	//
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	
	/* ISR Event FIFO Handler */
	ISR_Init();

	/* Task Management Initial */
	TASK_Init();
	
#if defined(UART_CONSOLE) || defined(HSUART_CONSOLE) 
	UartConsole_Init();
#endif

	/* PS/2 Host Interface Initial */
#if (SYSTEM_PS2_HOST_ENABLE)
	PS2_Init();
#endif

	/* KVM Console Initial */
#if (PROJECT_KVM_CONSOLE_ENABLE)
	KVM_Console_Init();
	DATAST_Init();
#endif
	
	/* KVM Button Interface Initial */
#if (KVM_BUTTON_SUPPORT)
	KVM_BTN_Init();
#endif

	/* KVM Buzzer Interface Initial */
#if (KVM_BUZZER_SUPPORT)
	KVM_BUZZER_Init();
#endif

	/* KVM Console & Host EDID Interface Initial */
#if (SYSETM_CONSOLE_EDID_CONTROL_ENABLE)
	EDID_Console_Init();
#endif

	/* USB Host Controller-Downstream ports Interface Initial */
#if (SYSTEM_USB_HC_ENABLE)
	USBHC_Init();
#endif

	/* USB Devcie Controller-Upstream ports Interface Initial */
#if (SYSTEM_USB_DC_ENABLE)
	USBDC_Init();
#endif
	
	/* KVM Power saving mode Initial */
#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
	KVM_PowerSaving_Init();
#endif

#if (SYSTEM_EXTENDER_SUPPORT)
 	EXTENDER_Init();
#endif /* #if (SYSTEM_EXTENDER_SUPPORT) */

	/* KVM System Information Display */
#if (KVM_INFORMATION_DISPLAY)
	DisplayKVMInformation();
#endif

#if (PROJECT_KVM_CONSOLE_ENABLE)
	KVM_CONSOLE_Vga_Control(KVM_CurrentHost,VGA_ON);
	USBDC_Resume_Task_ActiveID[0] = TASK_Active(TASK_TYPE_INTERVAL_MS, TASK_KVM_ACCESSORY_ID, 0, 0, 500, 500);
#endif

#if (SYSETM_GPIO_EEPROM_ENABLE)
	GPIO_EEProm_Init();
	GPIO_ReadEEprom();
#endif

#ifdef RTC
	KVM_RTC_Init();
#endif /* #ifdef RTC */

#ifdef IR
	KVM_IR_Init();
#endif /* #ifdef RTC */

#ifdef SYNC
	KM_SYNC_Init();
#endif /* #ifdef SYNC */

#ifdef KMLOG
	KMLOG_Init();
#endif /* #ifdef KMLOG */

#ifdef API
	API_Init();
#endif /* #ifdef API */

#ifdef MULTIVIEW
	 MULTIVIEW_Init();
#endif	 

//read btn state, set modes, ///zbb
{
//		U8_T 	btn_state = 0;	//zbb
//		
//		if (GPIO_GetPinValue(PORT0, GPIO_Pin_0))
//		{
//			btn_state |= 0x01;
//		}

//		if (GPIO_GetPinValue(PORT0, GPIO_Pin_1))
//		{
//			btn_state |= 0x02;
//		}

//		if (GPIO_GetPinValue(PORT0, GPIO_Pin_2))
//		{
//			btn_state |= 0x04;
//		}

//		if (GPIO_GetPinValue(PORT0, GPIO_Pin_3))
//		{
//			btn_state |= 0x08;
//		}

//		//check the btn_state
//		printf("---- btn_state:%bx ----\n\r", btn_state);
//		switch(btn_state)
//		{
//				case PORT0_ACTIVE:
//				API_Set_Roaming_Mode(API_ROAMING_DISABLE);
//				API_Set_Sync_Mode(API_SYNC_DISABLE);
//				KVM_Console_Port_Jump(3);
//				break;
//			case PORT1_ACTIVE:
//				API_Set_Roaming_Mode(API_ROAMING_DISABLE);
//				API_Set_Sync_Mode(API_SYNC_DISABLE);
//				KVM_Console_Port_Jump(2);
//				break;
//			case PORT2_ACTIVE:
//				API_Set_Roaming_Mode(API_ROAMING_DISABLE);
//				API_Set_Sync_Mode(API_SYNC_DISABLE);
//				KVM_Console_Port_Jump(1);
//				break;
//			case PORT3_ACTIVE:
//				API_Set_Roaming_Mode(API_ROAMING_DISABLE);
//				API_Set_Sync_Mode(API_SYNC_DISABLE);
//				KVM_Console_Port_Jump(0);
//				break;
//			case PORT12_ACTIVE:
//				API_Set_Roaming_Mode(API_ROAMING_ENABLE);					
//				API_Active_Roaming_Mapping(TWO_SCREEN_MODE);
//				KVM_Console_Port_Jump(3);
//				break;
//			case PORT34_ACTIVE:
//				API_Set_Roaming_Mode(API_ROAMING_ENABLE);					
//				API_Active_Roaming_Mapping(TWO_SCREEN_MODE);
//				KVM_Console_Port_Jump(1);
//				break;
//			case PORT_ALL_ACTIVE:
//				API_Set_Roaming_Mode(API_ROAMING_ENABLE);					
//				API_Active_Roaming_Mapping(QUAD_SCREEN_MODE);
//				KVM_Console_Port_Jump(3);
//				break;
//			case PORT_ALL_SYNC:
//				API_Set_Roaming_Mode(API_ROAMING_DISABLE);					
//				API_Active_Roaming_Mapping(QUAD_SCREEN_MODE);
//				API_Set_Sync_Mode(API_SYNC_ENABLE);
//				break;
//			case PORT_ONE_2_MAIN1_ACTIVE:
//				API_SET_ONE_2_SCREEN_MODE(1);
//				API_Current_Main_SCREEN = 3;
//				KVM_Console_Port_Jump(3);
//				break;
//			case PORT_ONE_2_MAIN2_ACTIVE:
//				API_SET_ONE_2_SCREEN_MODE(2);
//				API_Current_Main_SCREEN = 2;
//				KVM_Console_Port_Jump(2);
//				break;
//			case PORT_ONE_2_MAIN3_ACTIVE:
//				API_SET_ONE_2_SCREEN_MODE(3);
//				API_Current_Main_SCREEN = 1;
//				KVM_Console_Port_Jump(1);
//				break;
//			case PORT_ONE_3_MAIN0_ACTIVE:
//				API_SET_ONE_3_SCREEN_MODE(0);
//				API_Current_Main_SCREEN = 3;
//				KVM_Console_Port_Jump(3);
//				break;
//			case PORT_ONE_3_MAIN1_ACTIVE:
//				API_SET_ONE_3_SCREEN_MODE(1);
//				API_Current_Main_SCREEN = 2;
//				KVM_Console_Port_Jump(2);
//				break;
//			case PORT_ONE_3_MAIN2_ACTIVE:
//				API_SET_ONE_3_SCREEN_MODE(2);
//				API_Current_Main_SCREEN = 1;
//				KVM_Console_Port_Jump(1);
//				break;
//			case PORT_ONE_3_MAIN3_ACTIVE:
//				API_SET_ONE_3_SCREEN_MODE(3);
//				API_Current_Main_SCREEN = 0;
//				KVM_Console_Port_Jump(0);
//				break;
//			default:
//				break;
	//	}
}
	/* KVM Project Define Display */
	DisplayProjectVersionInformation();
/*
void main_loop(void)
*/
	/* Main polling loop */
	while (1)
	{		
#ifdef VPID_CHANGER
		VPID_Changer_Seed++;
#endif		

#ifdef MULTIVIEW
		UART1_Polling_Receive_Handle();
		MULTIVIEW_Receive_Handle();
#endif
		
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		// ISR Event Handle
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		{
			if (ISR_FIFO_Wp != ISR_FIFO_Rp)
			{
				switch (ISR_FIFO[ISR_FIFO_Rp].ISR_Type)
				{
#if (KVM_BUTTON_SUPPORT) & (KVM_BUTTON_HARDWARE)
					case ISR_BTN:	
					printf("DBG: ISR_BTN data=%02bx\r\n", ISR_FIFO[ISR_FIFO_Rp].Data);							
	#ifdef BTN_RESET						
						TASK_Active(TASK_TYPE_EVENT,TASK_BTN_Polling_ID,ISR_FIFO[ISR_FIFO_Rp].State & 0x0f,ISR_FIFO[ISR_FIFO_Rp].Data,0,0);  // Generate the task TASK_BTN_Polling_ID
	#else
						TASK_Active(TASK_TYPE_EVENT,TASK_BTN_Polling_ID,0x00,ISR_FIFO[ISR_FIFO_Rp].Data,0,0);  // Generate the task TASK_BTN_Polling_ID	
	#endif							
						//TASK_Active(TASK_TYPE_EVENT,TASK_BTN_Polling_ID,0x00,ISR_FIFO[ISR_FIFO_Rp].Data,0,0);  // Generate the task TASK_BTN_Polling_ID
						break;
#endif
					
#if (SYSETM_CONSOLE_EDID_CONTROL_ENABLE) && (SYSTEM_HWEDID_CONTROL_ENABLE)
					case ISR_EDID:
						TASK_EDID_Console_ISR_Handle(&ISR_FIFO[ISR_FIFO_Rp]);
						break;
#endif

#ifndef HSUART
#if (INTR_ISR_INCLUDE_HSUR2)
					case ISR_HUART:
						/* Tx DMA complete */
						if (ISR_FIFO[ISR_FIFO_Rp].Data & HSDSR_TDC_CPL)
						{
#if (SYSTEM_EXTENDER_SUPPORT)
							Extender_Transmit_Complete_Handle();
#endif /* #if (SYSTEM_EXTENDER_SUPPORT) */
						}
						break;
	  				case ISR_HUART_RBDMA: //High speed uart Receiver Buffer DMA 					
#if (SYSTEM_EXTENDER_SUPPORT)
#if (SYSTEM_EXTENDER_RECEIVER)			
						EXTENDER_Receiver_Receive_Handle();																		
#else
						EXTENDER_Transmitter_Receive_Handle();						
#endif	/* #if (SYSTEM_EXTENDER_RECEIVER) */
#endif  /* #if (SYSTEM_EXTENDER_SUPPORT)  */
						break;
#endif /* #if (INTR_ISR_INCLUDE_HSUR2) */
#endif /* #ifndef HSUART */
					
#if (SYSTEM_IR_DEVICE_SUPPORT)
				    case ISR_IR:									    					    
					    KVM_IR_Key_Handle();
					    break;		
#endif /* #if (SYSTEM_IR_DEVICE_SUPPORT) */	
					
#ifdef UART_CONSOLE	
					case ISR_UART0:			
						if (UartConsole_R_Queue == 0)
							UartConsole_Recieve_Handle();
					    break;		
#endif										
						
#ifdef HSUART_CONSOLE
					case ISR_UART1:				
						if (UartConsole_R_Queue == 0)
							UartConsole_Recieve_Handle();
					    break;		
#endif						

#if 0
					case ISR_UART1:										
						printf("DBG: UART1_RX data=%02bx head=%u tail=%u\r\n",
							ISR_FIFO[ISR_FIFO_Rp].Data,
							uart1_RxHead,
							uart1_RxTail);
						MULTIVIEW_Receive_Handle();
					    break;		
#endif												
						
#ifdef KMLOG
					case ISR_UART1:				
						KMLog_Recieve_Handle();
					    break;		
#endif					
#ifdef I2C_BUS
					case ISR_I2C:				
						I2C_Core_Handle_After_Transfer_Complete(ISR_FIFO[ISR_FIFO_Rp].State); 
						break;
#endif										
						
					default:
						break;
				}
				
				if (++ISR_FIFO_Rp >= ISR_FIFO_DEPTH)
				{
					ISR_FIFO_Rp = 0;
				}
			}
		}

		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		// USB DC ISR Handle
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if (SYSTEM_USB_DC_ENABLE)
		if (USBDC_ISRQ_Wp != USBDC_ISRQ_Rp) // if no ISR interrupt FIFO wait to process
		{
			USBDC_ISR_Handle();
		}
#endif

		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		// USB HC ISR Handle
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if (SYSTEM_USB_HC_ENABLE) 
		if (USBHC_ISRQ_Wp != USBHC_ISRQ_Rp) // if no ISR interrupt FIFO wait to process
		{
			USBHC_ISR_Handle();
		}
#endif	

		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		// PS/2 ISR Handle
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if (SYSTEM_PS2_HOST_ENABLE)
	#if (SYSTEM_HARDWARE_PS2_ENABLE) 
		if (HPS2_ISR_FIFO_WP != HPS2_ISR_FIFO_RP)
		{
			HW_PS2_ISR_Handle();
		}
	#endif
#endif

		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		// USB type Task Handle
		// ====================
		// The task inside USB table will be executed 
		// by order, the handler will exit only when
		// all usb task has been fork.
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		if (Task_USB_Table_RP != Task_USB_Table_WP)
		{
			TASK_USB_Main();
		}

		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		// Event type Task Handle
		// ======================
		// The task inside Event table will be executed 
		// by order, but only one task will be forked,
		// the handler will ternimate after forking
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		if (Task_Event_Table_RP != Task_Event_Table_WP)
		{
			TASK_Event_Main();
		}

		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		// Periold Task type Task Handle
		// =============================
		// The task inside ms period table will be executed 
		// only when the timer count down to 0, like event
		// only one task will be fork in one loop
		// after fork, the handler will auto reload 
		// interval time into counter again.
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		if (Task_MS_Period_Flag)
		{
			if (Task_MS_Period_Table_WP)
			{
				TASK_Period_MS_Maintain();
			}
			Task_MS_Period_Flag = 0;
		}

		if (Task_MS_Period_Table_RP != Task_MS_Period_Table_WP)
		{
			TASK_MS_Period_Main();
		}
		
#if (SYSTEM_MSC_DEVICE_SUPPORT)
		if (USBHC_ATL_Done_Q_Rp != USBHC_ATL_Done_Q_Wp)
		{
			USBHC_CORE_BULK_Done_Handle();		
		}
#endif

#if (SYSTEM_EXTENDER_SUPPORT)
#if ((SYSTEM_EXTENDER_RECEIVER) && (SYSTEM_USBAUDIO_DEVICE_SUPPORT))		
		if (ETDR_AudioInHead != ETDR_AudioInTail)
		{					
			ExtenderR_Audio_Data_In_Stage();			
		}
		
#endif
		if (EXTENDER_Transmit_OutP != EXTENDER_Transmit_InP)
		{
			Extender_Transmit_FIFO_Send();
		}
#endif /* if (SYSTEM_EXTENDER_SUPPORT) */

#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
		if (PowerSavingExitFlag)
		{
			KVM_PowerSaving_Exit();
		}
#endif

#ifdef I2C_BUS
        if (I2C_Tx_Start != I2C_Tx_End)
		{		
			if ((I2C_Control_State & (I2C_CONTROL_TRANSMIT | I2C_CONTROL_RECEIVE)) == 0x00)	
			{				
				I2C_Core_Transmit_FIFO_Send();
			}	
		}
#endif /* #if (SYSTEM_CASCADE_MASTER) */		
		
		if (SWTIMER_ScCheckTimer256ms >= 234)
		{
			EXTINT5_DISABLE;
			SWTIMER_ScCheckTimer256ms = 0;
			EXTINT5_ENABLE;

			if (++SWTIMER_ScCheckTimerMinutes == 60)
			{
				SWTIMER_ScCheckTimerMinutes = 0;
				if (++SWTIMER_ScCheckTimerHours == 24)
				{
					SWTIMER_ScCheckTimerHours = 0;
#if (SYSTEM_STORAGE_DEVICE_SUPPORT)
					STORAGE_SoftwareConfigurationCheck(1);
#endif					
				}
			}
		}		
	}
}

/*--------------------------------------------------------------------------------
 * void ISR_Init(void)
 * Purpose : Initial the ISR Fifo Table
 * Params  : $ *p  => Address
 *           $ len => display length
 * Returns : None
 * Note    : The length can not large than 0xff
 *--------------------------------------------------------------------------------*/
void ISR_Init(void)
{
	memset(ISR_FIFO,0x00,sizeof(ISR_FIFO));
	ISR_FIFO_Rp = 0;
	ISR_FIFO_Wp = 0;
}

/*--------------------------------------------------------------------------------
 * void malloc_free(void)
 * Purpose : free the malloc memory
 * Params  : None 
 * Returns : None
 * Note    : The length can not large than 0xff
 *--------------------------------------------------------------------------------*/
void malloc_free(void xdata *buf)
{
	if (buf != 0)
	{
		if (buf >= USB_Dynamic_Memory &&
			buf < (USB_Dynamic_Memory + USB_DYNAMIC_MEMORY_SIZE))
		{
			free(buf);
			//printf("m-(%lx)\n\r",(U32_T)buf);
		}
	}
}

/*--------------------------------------------------------------------------------
 * void xdata *m_malloc(void)
 * Purpose : malloc function
 * Params  : None 
 * Returns : None
 * Note    : The length can not large than 0xff
 *--------------------------------------------------------------------------------*/
void xdata *m_malloc(U16_T len,U8_T id)
{
	unsigned char xdata *p;

	p = calloc(len,1); /* allocate len bytes */

	if ((p == 0) || (p == XDATA_BASE_ADDR))
	{
		p = 0;
		printf ("!!! m_malloc(%bu)=%d:ERROR,not enough memory space !!!\n\r",id,len);
	}
	
	//printf("m+(%lx:%bu)\n\r",(U32_T)p,id);
	return p;
}

/*--------------------------------------------------------------------------------
 * void DisplayProjectVersionInformation(void)
 * Purpose : Display the KVM system version informaiton
 * Params  : None 
 * Returns : None
 * Note    : 
 *--------------------------------------------------------------------------------*/
void DisplayProjectVersionInformation(void)
{
	U8_T temp;
	
#if MCU_REAL_CHIP
	U8_T verString[] = "IC";
#else
	U8_T verString[] = "FPGA_ECO";
#endif

#ifdef MCU_TYPE_AX68004
 #if (SYSTEM_KVM)
    printf("\n\rASIX68004 KM(%s) - Version:%s,Date:%s\n\r", verString, VERSION_STRING,DATE_STRING);
 #endif

 #endif //#ifdef MCU_TYPE_AX68004

#ifdef MCU_TYPE_AX68002
 #if (SYSTEM_KVM)
    printf("\n\rASIX68002 KM (%s) - MIT Version:%s,Date:%s\n\r", verString, VERSION_STRING,DATE_STRING);
 #endif
#endif //#ifdef MCU_TYPE_AX68002


	printf("Chip Revision: %bx\r\n", temp = CRR);
	//printf("Wait State: %bx\r\n", temp = WTST);	

//Customer Information		
	printf("\n\r");
	printf(" System Informaiton:\n\r", VERSION_STRING);	
	
#if defined(HID_PARSER_CORE_KB)
	printf("  (v) USB KB Data Parser Engine Support\n\r");
#endif

#if defined(HID_PARSER_CORE_MS)
	printf("  (v) USB MOUSE Data Parser Engine Support\n\r");
#endif
	
#ifdef DEVICE_GPIO_DEFINE		
	printf("  MSC/Audio GPIO Define Version.\n\r");
	
	if (Audio_Device_Support_Gpio)
		printf("  (v)");
	else
		printf("  (x)");		
	printf("Audio Storage Device\n\r");	
	
	if (MSC_Device_Support_Gpio)
		printf("  (v)");
	else
		printf("  (x)");		
	printf(" Mass Storage Device,PageSize=%d\n\r",MSC_Class_Page_Size);
	
#else
	#ifdef NO_MSC
	printf("  (x) USB Storage Device:NO\n\r");
	#else
	printf("  (v) USB Storage Device:Yes,PageSize=%d\n\r",(U16_T)MSC_Class_Page_Size);
	#endif

	#ifdef NO_AUDIO
		printf("  (x) USB Audio Device:NO\n\r");
	#else
		printf("  (v) USB Audio Device:Yes\n\r");
	#endif
#endif /* #ifdef DEVICE_GPIO_DEFINE */

#ifdef PS2_HOST
printf("  (v) PS/2 Device:Yes\n\r");
#endif

#if (PROJECT_USB_GENERIC_HID_ENABLE)
	printf("  (v) USB Generic HID Device:Yes\n\r");
 #if (SYSTEM_EXTENDER_SUPPORT)
	USBHC_Max_Device_Support--;
 #endif
#endif

#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
	printf("  (v) USB Virutal MSC Device:Yes\n\r");
 #if (SYSTEM_EXTENDER_SUPPORT)
	USBHC_Max_Device_Support--;
 #endif
#endif

#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
	printf("  (v) Power Saving Mode:%bu,Time=%d\n\r",SYSTEM_POWER_SAVING_TYPE,SYSTEM_POWER_SAVING_TIMER);
#endif

#if (SYSTEM_EXTENDER_SUPPORT)
	printf("  (*)Max Device Support:%bx\n\r",USBHC_Max_Device_Support);
#endif

#ifdef ONE_HID
	printf("One HID Mode\n\r");	
#endif
	
	if (KVM_Flash.cSystemFlag3 & SYSTEM_MOUSE_SW_MASK)
	{	
		printf("SYS:Mouse Switch Mode On\n\r");		
	}
	
	if (KVM_Flash.cSystemFlag2 & SYSTEM_MS_DN_MASK)
	{
		printf("SYS:Mouse Coordinateion Relative Mode\n\r");
	}	
	else
	{
		printf("SYS:Mouse Coordinateion Absolute Mode\n\r");
	}		
		
	if ((KVM_Flash.cSystemFlag3 & SYSTEM_MPASS_MASK) == MPASS_ACROSS_MODE)
	{
		printf("Mouse Acroess Mode\n\r");
	}		
	//else if ((KVM_Flash.cSystemFlag3 & SYSTEM_MPASS_MASK) == MPASS_BUTTON_MODE)	
	//{
	//	printf("Mouse Button Mode(Relative)\n\r");
	//}		
	
	if (KVM_Flash.cSystemFlag2 & SYSTEM_MS_SYNC_MASK)
	{
		printf("Mouse Sync Mode On\n\r");
	}		
	
	//if (KVM_Flash.cSystemFlag2 & SYSTEM_KB_SYNC_MASK)
	//{
	//	printf("Keyboard Sync Mode On\n\r");
	//}		
}

#if (KVM_INFORMATION_DISPLAY)
/*--------------------------------------------------------------------------------
 * void DisplayKVMInformation(void) 
 * Purpose : Display System infromation
 * Params  : None 
 * Returns : None
 * Note    : 
 *--------------------------------------------------------------------------------*/
void DisplayKVMInformation(void) 
{
	printf("---------------- Project Define ----------------\n");
	printf("Console Device Support:\n\r");
#if (SYSTEM_PS2_HOST_ENABLE)
	printf("  [v]PS/2\n\r");
	#if (SYSTEM_HARDWARE_PS2_ENABLE)
		printf("     Hardware PS/2:%d\n\r",(U16_T)SYSTEM_HARDWARE_PS2_PORT_MAX);
	#endif
#endif

	printf("  [V]USB:\n\r");
	printf("     DC:\n\r");
	printf("        Max port:%d\n\r",(U16_T)KVM_MAX_PORT);
	printf("        Virtual HUB:y\n\r");
	printf("        Virtual Generic HID:y\n\r");
	printf("     HC:\n\r");
	printf("        Max Port:%d\n\r",(U16_T)USB_HC_MAX_PORT);
	printf("        Max Device:%d\n\r",(U16_T)USB_HC_MAX_DEVICE-2);
	printf("        PassThrough Level:Minimun\n\r");
	printf("        Audio Class Support:y\n\r");
	printf("        MSC Class Support:y\n\r");

#if (SYSETM_HWI2C_CONTROL_ENABLE)
	printf("Hardware I2C EDID:y\n\r");
#endif

#if (KVM_BUZZER_SUPPORT)
	printf("Hardware Buzzer:y\n\r");
#endif

#if (KVM_BUTTON_SUPPORT)
	printf("Hardware Button Detected:y\n\r");
#endif

#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
	printf("Power Saving Mode:%d,Time=%d\n\r",(U16_T)SYSTEM_POWER_SAVING_TYPE,(U16_T)SYSTEM_POWER_SAVING_TIMER);
	printf("  BTN Wakeup Eanble:");
	#if (BTN_WAKEUP_SUPPORT)
	printf("y\n\r");
	#else
	printf("n\n\r");
	#endif

	printf("  Hardware PS/2 Wakeup Eanble:");
	#if (PS2_WAKEUP_SUPPORT)
	printf("y\n\r");
	#else
	printf("n\n\r");
	#endif

	printf("  USB DC Wakeup Eanble:");
	#if (USBDC_WAKEUP_SUPPORT)
	printf("y\n\r");
	#else
	printf("n\n\r");
	#endif

	printf("  USB HC Wakeup Eanble:");
	#if (USBHC_WAKEUP_SUPPORT)
	printf("y\n\r");
	#else
	printf("n\n\r");
	#endif

	printf("  SPI Wakeup Eanble:");
	#if (SPI_WAKEUP_SUPPORT)
	printf("y\n\r");
	#else
	printf("n\n\r");
	#endif

	printf("  I2C Wakeup Eanble:");
	#if (I2C_WAKEUP_SUPPORT)
	printf("y\n\r");
	#else
	printf("n\n\r");
	#endif
#endif

	printf("\n\r");
#if (PROJECT_KVM_CONSOLE_ENABLE)
	printf("------- KVM Informpation -------\n\r");
	printf("KVM Port:%2d\n\r",(U16_T)KVM_MAX_PORT);
	printf("AutoScanInterval:%2d,Mode=",(U16_T)KVM_Flash.AutoScanInterval);

	switch ( (KVM_Flash.cSystemFlag0 & SYSTEM_AUTOSCAN_MODE_MASK) )
	{
		case SYSTEM_AUTOSCAN_MODE_FREE :
			printf("Free\n\r");
			break;
		case SYSTEM_AUTOSCAN_MODE_POWER :
			printf("Power\n\r");
			break;
		case SYSTEM_AUTOSCAN_MODE_TAG :
			printf("Tag\n\r");
			break;
		default :
			printf("???\n\r");
			break; 
	}

	printf("Buzzer Settiing: ");
	if (KVM_Flash.cSystemFlag0 & SYSTEM_BEEPER_MASK ) 
		printf("On\n\r");
	else
		printf("Off\n\r");
#endif
	printf("--------------------------------\n\r\n\r");
}
#endif /* KVM_INFORMATION_DISPLAY */

#if (SYSTEM_TASK_DUMP_SUPPORT)
/*--------------------------------------------------------------------------------
 * void Main_System_Halt(void)
 * Purpose : Halt the system
 * Params  : None 
 * Returns : None
 * Note    : 
 *--------------------------------------------------------------------------------*/
void Main_System_Halt(void)
{
	while (1);
}
#endif

/*--------------------------------------------------------------------------------
 * void Disp_Str(U8_T *p,U16_T len)
 * Purpose : Display the content into UART port, for debug purpose
 * Params  : $ *p  => Address
 *           $ len => display length
 * Returns : None
 * Note    : The length can not large than 0xff
 *--------------------------------------------------------------------------------*/
void Disp_Str(U8_T *p,U16_T len)
{
	U16_T index;
	
	printf("[");
	if (len)
	{
		len--;
		for (index=0;index < len ; index++)
		{
			printf("%02bx ",p[index]);
		}
		printf("%02bx]",p[index]);
	}	
	/*
	printf("A(%lx)[",(U32_T)p);
	if (len)
	{
		len--;
		for (index=0;index < len ; index++)
		{
			printf("%02bx ",p[index]);
		}
		printf("%02bx]\n\r",p[index]);
	}
	else
		printf("]\n\r");
	*/
}

/*--------------------------------------------------------------------------------
 * void Check_Devcie_Gpio_Setting(void)
 * Purpose : 
 * Params  : 
 *           
 * Returns : None
 * Note    : 
 *--------------------------------------------------------------------------------*/
void Check_Devcie_Gpio_Setting(void)
{
#ifdef DEVICE_GPIO_DEFINE
	#if (SYSTEM_EXTENDER_TRANSMITTER)
		Audio_Device_Support_Gpio = DEVCIE_AUDIO_SUPPORT;	
		MSC_Device_Support_Gpio = DEVCIE_MSC_SUPPORT;
		MSC_Class_Page_Size = USB_MSC_CLASS_PAGE_SIZ;		
	#else
		if (DEVICE_AUDIO_GPIO_PIN == DEVCIE_AUDIO_SUPPORT)
			Audio_Device_Support_Gpio = DEVCIE_AUDIO_SUPPORT;
		else
			Audio_Device_Support_Gpio = DEVCIE_NO_AUDIO_SUPPORT;	
		
		if (DEVICE_MSC_GPIO_PIN == DEVCIE_MSC_SUPPORT)			
		{	
			MSC_Device_Support_Gpio = DEVCIE_MSC_SUPPORT;
			if (Audio_Device_Support_Gpio)
			{			
				MSC_Class_Page_Size = USB_MSC_SMALLEAST_SZIE;
			}
			else
			{
				MSC_Class_Page_Size = USB_MSC_LARGEAST_SZIE;
			}		
		}	
		else	
		{	
			MSC_Device_Support_Gpio = DEVCIE_NO_MSC_SUPPORT;
			MSC_Class_Page_Size = 64;
		}
		
		DEVICE_AUDIO_GPIO_PIN = 0;
		DEVICE_MSC_GPIO_PIN = 0;	
	#endif	
#else	
	MSC_Class_Page_Size = USB_MSC_CLASS_PAGE_SIZ;
#endif	
}


/* End of main.c */

