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
 * Module Name : system_cfg.h
 * Purpose     :
 * Author      :
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */
 
/*
MCU_TYPE   AX6800x
*/
#ifndef __SYSTEM_CONFIG_H__
#define __SYSTEM_CONFIG_H__ 

#define NO			0
#define ALL			1
#define KB			2
#define MS			3
#define BUTTON		4
#define ACROSS		5
/*---------------------------------------------------------------------------*/
/*
$ Hardware Relate Define
[1].1 MS System Ticker 
[2].Debug & Console UART Port 
*/
/*---------------------------------------------------------------------------*/
/*
void 01_SYSTEM_MCU_PLATFORM(void){}
*/
#ifdef FPGA
#define	MCU_REAL_CHIP				0		/* FPGA */
#else
#define	MCU_REAL_CHIP				1		/* Real IC */
#endif

/*
void 02_SYSTEM_PCB_LAYOUT(void){}
*/
#ifdef FPGA
#define PCB_TEST_BOARD				0		/* GPIO pin order is different in test board */
#else
#define PCB_TEST_BOARD				1		/* GPIO pin order is different in test board */
#endif

/*
void 03_SYSTEM_MCU_I2C_SUPPORT(void){}
*/
#define SYSETM_HWI2C_CONTROL_ENABLE		1	// Hardware I2C Control Enable

/*
void 04_SYSTEM_USBDC_USBHC_INTERRUPT_CONTROL_NAMEING(void){}
*/
#define USBDC_INT_DISABLE	(EINT3 = OFF)
#define USBDC_INT_ENABLE 	(EINT3 = ON)

#define USBHC_INT_DISABLE	(EINT2 = OFF)
#define USBHC_INT_ENABLE 	(EINT2 = ON)

#define KVM_INFORMATION_DISPLAY      0

/*---------------------------------------------
$ Define the Extender Environmnent 
void 05_SYSTEM_EXTENDER_Envionment_Define(void){}
 ---------------------------------------------*/
#ifdef KVM
  #define SYSTEM_KVM					1 
#endif

#ifndef SYSTEM_KVM
#define SYSTEM_KVM						0 
#endif
 
#ifdef KVM_EXTENDER_RECEIVER
  	#define SYSTEM_EXTENDER_SUPPORT			1
  	#define SYSTEM_EXTENDER_RECEIVER		1
  	#define SYSTEM_EXTENDER_TRANSMITTER		0
 	#define SYSTEM_EA_CONTROL				0
	#define SYSTEM_HUART_SUPPORT			1
	#define EXTENDER_LED_FLASH_INDICATE		1
#endif


#ifdef KVM_EXTENDER_TRANSMITTER
  	#define SYSTEM_EXTENDER_SUPPORT			1
  	#define SYSTEM_EXTENDER_RECEIVER		0
  	#define SYSTEM_EXTENDER_TRANSMITTER		1
  	#define SYSTEM_EA_CONTROL				0
	#define SYSTEM_HUART_SUPPORT			1
	#define EXTENDER_LED_FLASH_INDICATE		1
#endif

#ifndef EXTENDER_LED_FLASH_INDICATE
#define EXTENDER_LED_FLASH_INDICATE			0
#endif

#ifndef SYSTEM_EA_CONTROL
#define SYSTEM_EA_CONTROL				0
#endif

#ifndef SYSTEM_EXTENDER_SUPPORT
	#define SYSTEM_EXTENDER_SUPPORT			0
	#define SYSTEM_EXTENDER_TRANSMITTER		0
	#define SYSTEM_EXTENDER_RECEIVER		0	
	#ifdef KMLOG
		#define SYSTEM_HUART_SUPPORT		1		
	#else
		#define SYSTEM_HUART_SUPPORT		0	
	#endif /* #ifdef KMLOG */
#endif

#ifdef PWAYTEK_01
#define SYSTEM_EXTENDER_RS232_MODE		1 //0:For RS485 ; 1:For RS232
#endif

#ifdef RS232
#define SYSTEM_EXTENDER_RS232_MODE		1 //0:For RS485 ; 1:For RS232
#endif

#ifndef SYSTEM_EXTENDER_RS232_MODE
#define SYSTEM_EXTENDER_RS232_MODE		0
#endif

/*---------------------------------------------------------------------------*/
/*
$ KVM System Relate Define
  [1].KVM System Configuration
	[2].LED Control 
	[3].Button Control
	[4].Buzzer Control
	[5].EDID Control
	[6].PS/2 Host Control
	[7].USB DC Control
	[8].USB HC Control
	[9].Storage Control
	[10].MSC Class Class control
	[11].Audio Class Class control
	[12].KVM System Power Saving control
	[13].USB PEN Drive Control
	[14].TASK System Control
	[15].Data Stream  Control
	[16].Hotkey Control
*/
/*---------------------------------------------------------------------------*/

/*******************************************
void KVM_01_SYSTEM_CONFIGURATION(void){}
*/
#define PROJECT_KVM_CONSOLE_ENABLE          1

#ifdef KVM_EXTENDER_RECEIVER
	#if defined(PS2_HOST) || defined(ONEHID)
		#define PROJECT_USB_GENERIC_HID_ENABLE      1
	#else
		#define PROJECT_USB_GENERIC_HID_ENABLE      0		
	#endif /* #ifdef PS2_HOST */
#endif /* #ifdef KVM_EXTENDER_RECEIVER */

#ifdef KVM_EXTENDER_TRANSMITTER
	#if defined(PS2_HOST) || defined(ONEHID)
		#define PROJECT_USB_GENERIC_HID_ENABLE      1
	#else
		#ifdef USB_HID
			#define PROJECT_USB_GENERIC_HID_ENABLE      1
		#else
			#define PROJECT_USB_GENERIC_HID_ENABLE      0
		#endif	
	#endif /* #ifdef GENERIC_HID */
#endif /* #ifdef KVM_EXTENDER_TRANSMITTER */

#ifdef NO_HID
	#define PROJECT_USB_GENERIC_HID_ENABLE      0
#endif

#ifndef PROJECT_USB_GENERIC_HID_ENABLE    
	#ifdef VHID
		#define PROJECT_USB_GENERIC_HID_ENABLE      1
	#else
		#define PROJECT_USB_GENERIC_HID_ENABLE      0
	#endif
#endif

#if (PROJECT_KVM_CONSOLE_ENABLE)
    #ifdef KVM_EXTENDER_TRANSMITTER
		#define KVM_HOTKEY_SUPPORT              0 //not support in transmiiter
    #else
		#if defined(KM104U) || defined(KM108U)
			#define KVM_HOTKEY_SUPPORT              	1
		#else
			#ifdef KMLOG
				#define KVM_HOTKEY_SUPPORT              1
			#else
				#ifdef NO_HOTKEY
				#define KVM_HOTKEY_SUPPORT              0
				#else
				#define KVM_HOTKEY_SUPPORT              1
				#endif /* #ifdef NO_HOTKEY */
			#endif
		#endif	
    #endif

	#ifdef MCU_TYPE_AX68002
		#define KVM_MAX_PORT		2   // 2 ports KVM
	#endif
	
	#ifdef MCU_TYPE_AX68004
		#define KVM_MAX_PORT		4   // 4 ports KVM
	#endif

#define HOTKEY_LED_INDICATE_TIME		500 // 500 ms
#define KVM_SW_INTERVAL_TIME			30  // 500 ms
#define KVM_HOTKEY_KB_LED				1
#define KVM_HOTKEY_TIMEOUT_CHECK		1
	#if (KVM_HOTKEY_TIMEOUT_CHECK)
		#define KVM_HOTKEY_VALID_TIME	20  // 10 seconds
	#endif
#endif 

#ifdef MCU_TYPE_AX68002
#define KVM_2_PORT_HDMI					1	// 0 = VGA, 1 = HDMI
#endif

#ifdef GPIO_EEPROM
#define SYSETM_GPIO_EEPROM_ENABLE 		1
#endif

#ifndef SYSETM_GPIO_EEPROM_ENABLE
#define SYSETM_GPIO_EEPROM_ENABLE 0
#endif

/*******************************************
void KVM_02_BUTTON_CONFIGURATION(void){}
*/
#ifdef NO_BTN
	#define KVM_BUTTON_SUPPORT			0
#else
	#if defined(KM104U)
		#define KVM_BUTTON_SUPPORT		1	
	#endif
#endif

#ifndef KVM_BUTTON_SUPPORT
#define KVM_BUTTON_SUPPORT			1
#endif

#if (KVM_BUTTON_SUPPORT)	 
	#ifdef ONE_BTN
		#define KVM_MULITY_BUTTON_TYPE	0
	#else
		#define KVM_MULITY_BUTTON_TYPE	1
	#endif 
	
	#ifdef BTN_POLLING
		#define KVM_BUTTON_HARDWARE		0			// Hardware(GPIO Interrupt) or Software(GPIO Polling)
	#else
		#define KVM_BUTTON_HARDWARE		1			// Hardware(GPIO Interrupt) or Software(GPIO Polling)
	#endif
	
	//#if (KVM_BUTTON_HARDWARE == 1)
	#define KVM_BUTTON_WAKEUP		0			// Allow Button to wakeup Sleep Vinson
	//#endif
	#define KVM_BUTTON_DE_BOUNCE_DISALBE	0
#endif

#ifndef KVM_BUTTON_HARDWARE
#define KVM_BUTTON_HARDWARE  0
#endif

#define KVM_BUTTON_RELEASE_DELAY_INTERVAL	0

/*******************************************
void KVM_03_BUZZER_CONFIGURATION(void){}
*/
#ifndef KVM_BUZZER_SUPPORT               
	#if defined(KM104U) || defined(KM108U)		
		#define KVM_BUZZER_SUPPORT                      1	
	#else	
		#ifdef KMLOG
			#define KVM_BUZZER_SUPPORT                  0    
		#else
			#define KVM_BUZZER_SUPPORT                  1
		#endif	
	#endif	
#endif

#if (KVM_BUZZER_SUPPORT)             
	#define KVM_BUZZER_HARDWARE			1  // Hardware(GPIO Interrupt) or Software(GPIO Polling)
	#define KVM_BUZZER_TYPE_LEVEL		1  // User only high & low to control the buzzer
	#define KVM_BUZZER_TYPE_Frequency	0  // User Frequency to control ther buzzer
#else	
	#define KVM_BUZZER_HARDWARE			0  // Hardware(GPIO Interrupt) or Software(GPIO Polling)
	#define KVM_BUZZER_TYPE_LEVEL		0  // User only high & low to control the buzzer
	#define KVM_BUZZER_TYPE_Frequency	0  // User Frequency to control ther buzzer
#endif

/*******************************************
void KVM_04_CONSOLE_EDID_CONFIGURATION(void){}
*/
#if defined(KM104U) || defined(KM108U)
	#define SYSETM_CONSOLE_EDID_CONTROL_ENABLE	0   // Handle console monitor edid table
#else
	#ifdef KMLOG
		#define SYSETM_CONSOLE_EDID_CONTROL_ENABLE	1   // Handle console monitor edid table
	#else
		#ifdef NO_EDID
		#define SYSETM_CONSOLE_EDID_CONTROL_ENABLE	0   // Handle console monitor edid table	
		#else
		#define SYSETM_CONSOLE_EDID_CONTROL_ENABLE	1   // Handle console monitor edid table	
		#endif /* #ifdef NO_EDID */
	#endif
#endif


#if (SYSETM_CONSOLE_EDID_CONTROL_ENABLE)
	#ifdef I2C_BUS
		#define SYSTEM_HWEDID_CONTROL_ENABLE  0
		#define SYSTEM_SWEDID_CONTROL_ENABLE  1
	#else
		#define SYSTEM_HWEDID_CONTROL_ENABLE  1
		#define SYSTEM_SWEDID_CONTROL_ENABLE  0
	#endif	
#else
	#define SYSTEM_HWEDID_CONTROL_ENABLE  0
	#define SYSTEM_SWEDID_CONTROL_ENABLE  0
#endif

/*******************************************
void KVM_05_PS2_CONFIGURATION(void){}
*/
#if (SYSTEM_EXTENDER_SUPPORT)
	#ifdef KVM_EXTENDER_RECEIVER
		#ifdef PS2_HOST			
			#define SYSTEM_PS2_HOST_ENABLE      1			
		#else
			#define SYSTEM_PS2_HOST_ENABLE      0
		#endif /* #ifdef PS2_HOST   */
	#endif /* #ifdef KVM_EXTENDER_RECEIVER */		
	
	#ifdef KVM_EXTENDER_TRANSMITTER
		#ifdef PS2_HOST			
			#define SYSTEM_PS2_HOST_ENABLE      1			
		#else
			#define SYSTEM_PS2_HOST_ENABLE      0
		#endif /* #ifdef PS2_HOST   */
	#endif /* #ifdef KVM_EXTENDER_TRANSMITTER */		
	
#else
	#ifdef PS2_HOST  //Host PS/2 interface
		#define SYSTEM_PS2_HOST_ENABLE		1
		
		#ifdef HPS2
			#define SYSTEM_HARDWARE_PS2_ENABLE  1
		#else	
			#define SYSTEM_HARDWARE_PS2_ENABLE  0
		#endif
		
		#ifdef HPS2_GPIO
			#define SYSTEM_GPIO_PS2_ENABLE	 	1
		#else	
			#define SYSTEM_GPIO_PS2_ENABLE	 	0
		#endif	
	#else
		#define SYSTEM_PS2_HOST_ENABLE		0
		#define SYSTEM_HARDWARE_PS2_ENABLE  0
		#define SYSTEM_HARDWARE_PS2_ENABLE	0
		#define SYSTEM_GPIO_PS2_ENABLE		0
	#endif	
	
	/*
	#ifdef MCU_TYPE_AX68004
		#if (PROJECT_USB_GENERIC_HID_ENABLE)
			#define SYSTEM_PS2_HOST_ENABLE		0
			#define SYSTEM_HARDWARE_PS2_ENABLE  0
		#else
			#define SYSTEM_PS2_HOST_ENABLE		0
			#define SYSTEM_HARDWARE_PS2_ENABLE  0
		#endif	
	#endif
	*/
	/*
	#ifdef MCU_TYPE_AX68002
		#ifdef ONEHID
			#define SYSTEM_PS2_HOST_ENABLE	1
			#define SYSTEM_PS2_HARDWARE     0
		#else
			#define SYSTEM_PS2_HOST_ENABLE	0
			#define SYSTEM_PS2_HARDWARE     0
		#endif	// #ifdef ONEHID
	#endif		
	*/
#endif

#ifndef SYSTEM_PS2_HOST_ENABLE
#define SYSTEM_PS2_HOST_ENABLE		0
#endif

#define SYSTEM_HARDWARE_PS2_PORT_MAX	2   // Howmay hardware port used
//#define SYSTEM_EXTI_PS2_PORT_MAX		0   // hwo many ps/2 port need to support, this number is 

#if (SYSTEM_PS2_HOST_ENABLE)
	//-------------------------------------------
	#define SYSTEM_HARDWARE_PS2_ENABLE  	1 	//Control PS/2 via Hardware
	#define SYSTEM_PS2_RESEND_MAX			10 	//only resend 2 times
#else
	#define SYSTEM_HARDWARE_PS2_ENABLE		0 //Control PS/2 via Hardware
#endif

/*******************************************
void KVM_06_USB_DC_CONFIGURATION(void){}
*/
#define SYSTEM_USB_DC_ENABLE				1

//#define SYSTEM_USB_DC_GPIO_PULLUP			0

/*******************************************
void KVM_08_USB_HC_CONFIGURATION(void){}
*/
#define SYSTEM_USB_HC_ENABLE				1
#define SYSTEM_USB_HC_BURST					1

/*******************************************
void KVM_09_USB_MSC_DEVICE_CONFIGURATION(void){}
*/ 
#if (SYSTEM_USB_HC_ENABLE)
//Customer's Extender Version
#ifdef PCT_MUA22
	#define SYSTEM_MSC_DEVICE_SUPPORT		0	//support mass storage class
	#define SYSTEM_EXTENDER_MSC_SUPPORT		0
#endif

#ifdef NO_MSC
	#define SYSTEM_MSC_DEVICE_SUPPORT		0	//support mass storage class
	#define SYSTEM_EXTENDER_MSC_SUPPORT		0
#endif /* #ifdef NO_MSC */

#ifdef USB_MSC
#define SYSTEM_MSC_DEVICE_SUPPORT		    1	//support mass storage class
#endif

//Standard Extender Version
#ifndef SYSTEM_MSC_DEVICE_SUPPORT //For standar Extender
	#if (SYSTEM_EXTENDER_RS232_MODE)
		#define SYSTEM_MSC_DEVICE_SUPPORT		0
		#define SYSTEM_EXTENDER_MSC_SUPPORT		0
	#else	
		#if (SYSTEM_EXTENDER_SUPPORT)
			#define SYSTEM_MSC_DEVICE_SUPPORT		1	
			#define SYSTEM_EXTENDER_MSC_SUPPORT		1
		#else
			#define SYSTEM_MSC_DEVICE_SUPPORT		1	
			#define SYSTEM_EXTENDER_MSC_SUPPORT		0
		#endif
	#endif			
#endif /* #ifndef SYSTEM_MSC_DEVICE_SUPPORT */

#if (SYSTEM_MSC_DEVICE_SUPPORT)
	#define  SYSTEM_MSC_MAX					1	// only support one msc device

	#if (SYSTEM_USB_HC_BURST)	//Setting with ECO mode
		#define  USBHC_MSC_TD_SIZ				4	// Support 2 TD as Bulk In and 2 TD as Bulk Out Buffer
		#define  USB_HC_MSC_BURST_OUT_TD0		8	// BULK OUT Burst First  TD index
		#define  USB_HC_MSC_BURST_OUT_TD1		9	// BULK OUT Burst Second TD index
		#define  USB_HC_MSC_BURST_IN_TD0		10	// BULK IN Burst First  TD index
		#define  USB_HC_MSC_BURST_IN_TD1		11	// BULK IN Burst Second TD index

		#define	 USB_HC_MSC_DC_IN_DBUF		0		//MSC DC support double buffer
		#define	 USB_HC_MSC_DC_OUT_DBUF		0		//MSC DC support double buffer
		#else
		#define  USBHC_MSC_TD_SIZ			8		// Support 8 TD as Bulk In/Out Buffer
		#define	 USB_HC_MSC_DC_IN_DBUF		1		//MSC DC support double buffer
		#define	 USB_HC_MSC_DC_OUT_DBUF		1		//MSC DC support double buffer
	#endif	/* #if (SYSTEM_USB_HC_BURST) */

	#define  USB_HC_MSC_START				8
	#define  USB_HC_MSC_BOT_SUPPORT			1
 #else /* #if (SYSTEM_MSC_DEVICE_SUPPORT) */
	#define  USBHC_MSC_TD_SIZ			0
	#define  USB_HC_MSC_BOT_SUPPORT		0
	#define  USB_HC_MSC_DC_IN_DBUF		0		//MSC DC support double buffer
	#define  USB_HC_MSC_DC_OUT_DBUF		0		//MSC DC support double buffer
 #endif /* End of #if (SYSTEM_MSC_DEVICE_SUPPORT) */

//	#define  USBHC_SWITCH_RESET_SUPPORT		1
#endif	/* #if (SYSTEM_USB_HC_ENABLE) */
 
/*******************************************
void KVM_10_STORAGE_DEVICE_CONFIGURATION(void){}
*/
#if (SYSTEM_EXTENDER_TRANSMITTER)
#define SYSTEM_STORAGE_DEVICE_SUPPORT		0
#else
#define SYSTEM_STORAGE_DEVICE_SUPPORT		1
#endif

#if (SYSTEM_STORAGE_DEVICE_SUPPORT)
	#define SYSTEM_STORAGE_FLASH			1	//Used internal flash as the storeage media
#else
	#define SYSTEM_STORAGE_FLASH			0	//Not used internal flash as the storeage media
#endif

/*******************************************
void KVM_11_AUDIO_DEVICE_CONFIGURATION(void){}
*/
#ifdef PCT_MUA22
#define SYSTEM_USBAUDIO_DEVICE_SUPPORT	1	   //Support Audio Class Devcie
#endif

#if defined(NO_AUDIO) || defined(NO_USBAUDIO)
#define SYSTEM_USBAUDIO_DEVICE_SUPPORT	0	   //Not Support Audio Class Devcie
#endif /* #ifdef NO_AUDIO */

#ifdef AAUDIO
#define SYSTEM_AUDIO_DEVICE_SUPPORT		1	   //Not Support Audio Class Devcie
#endif

//Standard Version
#ifndef SYSTEM_USBAUDIO_DEVICE_SUPPORT
	#if (SYSTEM_EXTENDER_RS232_MODE)
		#define SYSTEM_USBAUDIO_DEVICE_SUPPORT	0	   //Support Audio Class Devcie
	#else
		#define SYSTEM_USBAUDIO_DEVICE_SUPPORT	1	   //Support Audio Class Devcie
	#endif /* #if (SYSTEM_EXTENDER_RS232_MODE) */
#endif /* #ifndef SYSTEM_USBAUDIO_DEVICE_SUPPORT */

#ifndef SYSTEM_AUDIO_DEVICE_SUPPORT
#define SYSTEM_AUDIO_DEVICE_SUPPORT		0	   //Not Support Audio Class Devcie
#endif

#ifdef KVM_EXTENDER_TRANSMITTER
#define SYSTEM_AUDIO_MAX				0		// always 0 (not support)
#endif

#ifdef KVM_EXTENDER_RECEIVER
#define SYSTEM_AUDIO_MAX				1		// always 1 or 0 (1: 1 audio device, 0: not support)
#endif

#ifndef SYSTEM_AUDIO_MAX
#define SYSTEM_AUDIO_MAX				1
#endif

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
	#ifdef UAUDIO_SW_DELAY
		#define SYSTEM_SWITCH_AUDIO_DELAY   1
		#define SWITCH_PORT_DELAY_AUDIO_CNT 2500
    #else		
		#define SYSTEM_SWITCH_AUDIO_DELAY   0
	#endif	/* #ifdef UAUDIO_DELAY */	
#else
	#define SYSTEM_SWITCH_AUDIO_DELAY   	0
#endif	/* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */
/*******************************************
void KVM_12_SYSTEM_POWER_SAVING_CONFIGURATION(void){}
*/
#ifdef POWER_SAVING
#define SYSTEM_POWER_SAVING_MODE_SUPPORT 1
#else
#define SYSTEM_POWER_SAVING_MODE_SUPPORT 0
#endif

#define SYSTEM_POWER_SAVING_TIMER  5000 // 5 seconds interval check 
#define SYSTEM_POWER_SAVING_TYPE   3 // stop the clock also

#define BTN_WAKEUP_SUPPORT         1
#define PS2_WAKEUP_SUPPORT         1
#define USBHC_WAKEUP_SUPPORT       1
#define SPI_WAKEUP_SUPPORT         0
#define I2C_WAKEUP_SUPPORT         0

/************************************************
void KVM_13_SYSTEM_USB_PEN_DRIVE_CONFIGURATION(void){}
*/
#ifdef KVM_EXTENDER_RECEIVER
#define SYSTEM_USB_PEN_DRIVE_SUPPORT	0
#endif /* #ifndef KVM_EXTENDER_RECEIVER */

#ifdef KVM_EXTENDER_TRANSMITTER
#define SYSTEM_USB_PEN_DRIVE_SUPPORT	0
#endif /* #ifndef KVM_EXTENDER_TRANSMITTER */

#ifdef KVM
#define SYSTEM_USB_PEN_DRIVE_SUPPORT	0
#endif

#ifndef SYSTEM_USB_PEN_DRIVE_SUPPORT
#define SYSTEM_USB_PEN_DRIVE_SUPPORT	0
#endif

/************************************************
void KVM_14_SYSTEM_TASK_CONTROL_CONFIGURATION(void){}
*/
#define SYSTEM_TASK_DUMP_SUPPORT		0

/************************************************
void KVM_15_SYSTEM_DATA_STREAM_CONFIGURATION(void){}
*/
#define  DATAST_VIRTUAL_MS_IN_QUEUE		0
#define  DATAST_GENERIC_MS_IN_QUEUE		1

/************************************************
void KVM_16_SYSTEM_HOTKEY_CONTROL_CONFIGURATION(void){}
*/
/*---------------------------------------------------------
void Hotkey_Function_Control(void){} 
---------------------------------------------------------*/
//x #define ENABLE_HOTKEY_FUN_PORT_SW_NUMERIC			1
//x #define ENABLE_HOTKEY_FUN_PORT_SW_NONUM				1

#define ENABLE_HOTKEY_FUN_AUTOAUDIO_SW					1
#if (SYSTEM_AUDIO_DEVICE_SUPPORT)
#define ENABLE_HOTKEY_FUN_AUDIO_SW						1
#else
#define ENABLE_HOTKEY_FUN_AUDIO_SW						0
#endif

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
#define ENABLE_HOTKEY_FUN_USBAUDIO_SW					1
#else
#define ENABLE_HOTKEY_FUN_USBAUDIO_SW					0
#endif

#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
#define ENABLE_HOTKEY_FUN_USBAUDIO_REPORT				1
#else
#define ENABLE_HOTKEY_FUN_USBAUDIO_REPORT				0
#endif

#if (KVM_BUZZER_SUPPORT)
#define ENABLE_HOTKEY_FUN_BUZZER_CONTROL				1
#else
#define ENABLE_HOTKEY_FUN_BUZZER_CONTROL				0
#endif //#if (KVM_BUZZER_SUPPORT)

#define ENABLE_HOTKEY_FUN_AUTOSCAN_CONTROL				1
#define ENABLE_HOTKEY_FUN_AUTOSCAN_INTERVAL				1
#define ENABLE_HOTKEY_FUN_HUB_SW						1
#define ENABLE_HOTKEY_FUN_PORT_RESET					0
#define ENABLE_HOTKEY_FUN_FLASH_DEFAULT					1
#define ENABLE_HOTKEY_FUN_POWER_JUMP_SW					1 
#define ENABLE_HOTKEY_FUN_PLUGIN_JUMP_SW				1 

#if (SYSTEM_MSC_DEVICE_SUPPORT)
#define ENABLE_HOTKEY_FUN_MSC_CONTROL					0
#define ENABLE_HOTKEY_FUN_MSC_SW						1
#else
#define ENABLE_HOTKEY_FUN_MSC_SW						0
#define ENABLE_HOTKEY_FUN_MSC_CONTROL					0
#endif

#if (SYSETM_CONSOLE_EDID_CONTROL_ENABLE)
#define ENABLE_HOTKEY_FUN_EDID_REREAD					1
#else
#define ENABLE_HOTKEY_FUN_EDID_REREAD					0
#endif

#define ENABLE_HOTKEY_FUN_RAPID_SW						1

#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
#define ENABLE_HOTKEY_FUN_PEN_CONTROL					1
#define ENABLE_HOTKEY_FUN_PEN_SW						1
#else
#define ENABLE_HOTKEY_FUN_PEN_CONTROL					0
#define ENABLE_HOTKEY_FUN_PEN_SW						0
#endif

#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
#define ENABLE_HOTKEY_FUN_PWSAVING_CONTROL				1
#else
#define ENABLE_HOTKEY_FUN_PWSAVING_CONTROL				0
#endif

#if (SYSTEM_EXTENDER_RECEIVER)
	#define SYSTEM_TRSNSMITTER_PORT_SWITCH_SUPPORT		1		
	#if (SYSTEM_TRSNSMITTER_PORT_SWITCH_SUPPORT)
		#define ENABLE_HOTKEY_FUN_T_PORT_SW				1	
	#endif
 

	#define SYSTEM_TRSNSMITTER_BUZZER_SUPPORT				1		
	#if (SYSTEM_TRSNSMITTER_BUZZER_SUPPORT)
		#define ENABLE_HOTKEY_FUN_T_BUZZER_CONTROL			1
	#endif

 
	#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
		#define ENABLE_HOTKEY_T_USBAUDIO_SW					1
		#define ENABLE_HOTKEY_T_AUDIO_AUTO_SW				1
	#endif /* #if (SYSTEM_USBAUDIO_DEVICE_SUPPORT) */
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */

#ifndef ENABLE_HOTKEY_FUN_T_PORT_SW
#define ENABLE_HOTKEY_FUN_T_PORT_SW						0
#endif

#ifndef ENABLE_HOTKEY_FUN_T_BUZZER_CONTROL
#define ENABLE_HOTKEY_FUN_T_BUZZER_CONTROL				0
#endif

#if (SYSTEM_MSC_DEVICE_SUPPORT)
#define ENABLE_HOTKEY_T_MSC_SW							1
#endif


#ifndef ENABLE_HOTKEY_T_MSC_SW							
#define ENABLE_HOTKEY_T_MSC_SW							0
#endif

#ifndef ENABLE_HOTKEY_T_USBAUDIO_SW							
#define ENABLE_HOTKEY_T_USBAUDIO_SW						0
#endif

#ifndef ENABLE_HOTKEY_T_AUDIO_AUTO_SW						
#define ENABLE_HOTKEY_T_AUDIO_AUTO_SW					0
#endif

#ifndef SYSTEM_STANDARD_HOTKEY
#define SYSTEM_STANDARD_HOTKEY							1
#endif

#ifdef RTC
	#define ENABLE_HOTKEY_FUN_RTC_CONTROL				1
#else
	#define ENABLE_HOTKEY_FUN_RTC_CONTROL				0
#endif
/************************************************
void KVM_17_SYSTEM_VENDOR_CONTROL_CONFIGURATION(void){}
*/

#define SYSTEM_VENDOR_COMMAND_SUPPORT					0


/************************************************
void KVM_18_SYSTEM_EXTENDER_CONTROL_CONFIGURATION(void){}
*/
#if (SYSTEM_EXTENDER_RS232_MODE)
#define KVM_EXTENDER_MSC_NO_ACK					1
#define KVM_EXTENDER_SETUP_NO_ACK				1
#define KVM_EXTENDER_HID_NO_ACK					1
#define KVM_EXTENDER_HID_SHORT_PACKET			1
#else
#define KVM_EXTENDER_MSC_NO_ACK					1
#define KVM_EXTENDER_HID_NO_ACK					1
#define KVM_EXTENDER_SETUP_NO_ACK				0
#define KVM_EXTENDER_HID_SHORT_PACKET			1
#endif

#ifdef KVM_EXTENDER_RECEIVER
  #ifdef PCT_MUA22
  #define EXTENDER_ISL59920_SUPPORT				1
  #endif  
  #define EXTENDER_EDID_SUPPORT					1 
  #define SYSTEM_EXTENDER_ISO_IN_SUPPORT		1
  #define EXTENDER_T_HID_REPORT_PASSTHROUGH		0
#endif /* #ifdef KVM_EXTENDER_RECEIVER */

#ifdef KVM_EXTENDER_TRANSMITTER
  #define EXTENDER_ISL59920_SUPPORT				0
  #define EXTENDER_T_DEV_DESP_PASSTHROUGH		0
  #define EXTENDER_T_CONF_DESP_PASSTHROUGH		0
  #define EXTENDER_T_VENDOR_ID_PASSTHROUGH		0
  #define EXTENDER_T_PROD_ID_PASSTHROUGH		0
  #define EXTENDER_T_SERIAL_ID_PASSTHROUGH		0
  #define EXTENDER_T_HID_REPORT_PASSTHROUGH		0

  #define SYSTEM_EXTENDER_ISO_OUT_SUPPORT		1
  
  #define EXTENDER_T_HC_ENABLE					1
#endif /* #ifdef KVM_EXTENDER_TRNASMITTER */

#ifndef EXTENDER_ISL59920_SUPPORT
#define EXTENDER_ISL59920_SUPPORT 				0
#endif


#ifndef KVM_EXTENDER_RECEIVER
#ifndef KVM_EXTENDER_TRANSMITTER
  #define EXTENDER_T_HID_REPORT_PASSTHROUGH		0
#endif
#endif

/***********************************************************
void KVM_19_SYSTEM_IR_CONTROL_CONFIGURATION(void){}
***********************************************************/
#ifdef IR
#define SYSTEM_IR_DEVICE_SUPPORT				1
#endif

#ifndef SYSTEM_IR_DEVICE_SUPPORT
#define SYSTEM_IR_DEVICE_SUPPORT				0
#endif
/*
void DEBUG_CONFIGURATION(void)
*/
#define ENUMERATE_VENDOR_TEST		0
#define KVM_CONSOLE_DEBUG_MODE		0
#define HOTKEY_DEBUG_MODE			0
#define POWER_SAVING_DEBUG_MODE		0
#define POWER_SAVING_DEBUG_MODE2	1

#define EDID_DEBUG_SW				1
/*-------------------------------------------------------------------------
$ Debug Support Define
---------------------------------------------------------------------------*/
#define PS2_RELEASE_MODE				0
#define PS2_DEBUG_MODE					0
#define PS2_MS_DATA_DEBUG_MODE 			0
#define PS2_KB_DATA_DEBUG_MODE			0
#define USBDC_DEBUG_MODE				0
#define GENERIC_USB_KB_DEBUG			0
#define GENERIC_USB_MS_DEBUG			0
#define VIRTUAL_USB_HID_DEBUG			0
//#define VIRTUAL_USB_DC_DEBUG			0
#define USBHC_HUB_DEBUG_MODE			0
/* USB HC Interface */
#define USBHC_OC_FORCE_SW_RESET			0
#define USBHC_PARSER_DEBUG_MODE			0
#define USBHC_INIT_DEBUG_MODE			0
#define USBHC_DEBUG_MODE				0
#define USBHC_ROOTHUB_DEBUG_MODE		1
#define USB_PASSTHROUGH_DEBUG_MODE		0

#define USB_MSC_CLASS_DEBUG_MODE		0
#define USB_MSC_SMALLEAST_SZIE			256
#define USB_MSC_LARGEAST_SZIE			512
#define USB_MSC_NO_SUPPORT_SZIE			64

#ifdef DEVICE_GPIO_DEFINE
	#define USB_MSC_CLASS_PAGE_SIZ			USB_MSC_LARGEAST_SZIE
#else
	#ifdef NO_MSC
		#define USB_MSC_CLASS_PAGE_SIZ			USB_MSC_NO_SUPPORT_SZIE
	#else	
		#ifdef NO_AUDIO			
			#define USB_MSC_CLASS_PAGE_SIZ			USB_MSC_LARGEAST_SZIE
		#else
			#if (SYSTEM_KVM)
				#define USB_MSC_CLASS_PAGE_SIZ			USB_MSC_LARGEAST_SZIE
			#else
				#define USB_MSC_CLASS_PAGE_SIZ			USB_MSC_SMALLEAST_SZIE
			#endif /* #if (SYSTEM_KVM) */
		#endif /* #ifdef NO_AUDIO */
	#endif /* #ifdef NO_MSC */	

#endif /* #ifdef DEVICE_GPIO_DEFINE */

#define USB_ENUMERATE_HID_PARSER_DEBUG_SW			0
#define USB_ENUMERATE_HID_PARSER_TABLE_DEBUG_SW		0

/* Extender */
#define EXTENDER_DEBUG0							0
#define RECEIVER_USB_PASSTHROUGH_DEBUG_MODE		0
#define TRANSMITTER_USB_PASSTHROUGH_DEBUG_MODE	0
#define TRANSMITTER_CMD_DEBUG					0
#define RECEIVER_CMD_DEBUG						0
/*---------------------------------------------------------------------------*/
#define SYSETM_DATA_STREAM_CONTROL_ENABLE		1	// Enable Dsta Streaming feature
/*---------------------------------------------------------------------------*/

//#define SYSETM_GPIO_BIDIRECTIONAL		1	// configure the GPIO port for bidirectional way

/* DEFINATION DECLARATIONS */
 
 
/*MISC Register Macro */
#define _MISC_DR_SFR(MISCDR_Value) (MDR=MISCDR_Value)
#define _MISC_CIR_SFR(MISCCIR_Value) (MCIR=MISCCIR_Value)
#define _MISC_DR_READ_SFR(reg) (reg=MDR)

/*USB HC Register Macro */
#define _USBHC_HCDR_SFR(HCDR_Value) (HCDR=HCDR_Value)
#define _USBHC_HCCIR_SFR(HCCIR_Value) (HCCIR=HCCIR_Value)
#define _USBHC_HCDR_READ_SFR(reg) (reg=HCDR)

/*PS/2 Host Register Macro */
#define _PS2_DR_SFR(PS2DR_Value) (PS2DR=PS2DR_Value)
#define _PS2_CIR_SFR(PS2CIR_Value) (PS2CIR=PS2CIR_Value)
#define _PS2_DR_READ_SFR(reg) (reg=PS2DR)

/*I2C Register Macro */
#define _I2C_DR_SFR(I2CDR_Value) (I2CDR=I2CDR_Value)
#define _I2C_CIR_SFR(I2CCIR_Value) (I2CCIR=I2CCIR_Value)
#define _I2C_DR_READ_SFR(reg) (reg=I2CDR)  

/* NAMING CONSTANT DECLARATIONS */
/* GLOBAL VARIABLES */
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
#endif /* End of __SYSTEM_CONFIG_H__ */

/* End of system_cfg.h */
