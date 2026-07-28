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
 * Module Name : project_include.h
 * Purpose     :
 * Author      :
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */
  
#include	<stdio.h>
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//This is for Vinson ASIC Standard IO & Peripheral
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "ax6800x.h"
#include "system_cfg.h"
#include "types.h"
#include "cpu_cfg.h"
#include "cpu.h"
#include "main.h"
#include "mstimer.h"
#include "interrupt.h"
#include "interrupt_cfg.h"
#include "dma.h"
#include "uart.h"
#include "i2c.h"
#include "flash.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//This is for Vinson Project Used Include File
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "task_main.h"
#include "gpio.h"
#include "data_stream.h"
#if defined(HID_PARSER_CORE_KB) || defined(HID_PARSER_CORE_MS)
#include "hidparser_core.h"
#endif
#include "data_stream_generic.h"

//#if (SYSTEM_PS2_HOST_ENABLE)
#include "ps2_host.h"
#if (SYSTEM_PS2_HOST_ENABLE)
	#if (SYSTEM_HARDWARE_PS2_ENABLE)
		#include "hardware_ps2.h"
	#endif

//	#if (SYSTEM_EXTI_PS2_ENABLE)
//		#include "exti_ps2.h"
//	#endif
#endif

#if (SYSETM_CONSOLE_EDID_CONTROL_ENABLE)	
	#if (SYSTEM_HWEDID_CONTROL_ENABLE)
		#include "console_edid.h"
		#include "edid_hardware.h"
	#else
		#include "consoleS_edid.h"	
	#endif
#endif

#include "kvm_hotkey.h"
#include "kvm_console.h"

#if (KVM_BUTTON_SUPPORT)
	#if (KVM_BUTTON_HARDWARE)
		#include "btn_interrupt.h"
	#else
		#include "btn_polling.h"
	#endif
#endif

#if (KVM_BUZZER_SUPPORT)
	#if (KVM_BUZZER_HARDWARE)
		#include "buzzer_hardware.h"
	#else
		#include "buzzer_gpio.h"
	#endif
#endif

#if (SYSTEM_STORAGE_DEVICE_SUPPORT)
	#include "storage.h"
#endif


#if (SYSTEM_POWER_SAVING_MODE_SUPPORT)
	#include "power_saving.h"
#endif

#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)
#include "usb_scsi.h"
#endif

//-----------------------------------------------------
//$ USB Control
//-----------------------------------------------------
#include "usb_def.h"
#include "usbdc_cfg.h"
#include "usbhc_cfg.h" 

//-----------------------------------------------------
//$ USB Control
//-----------------------------------------------------
#include "usbdc_regs.h"
#include "usbdc.h"
#include "usbdc_core.h"
#include "usbdc_virtual.h"
#include "usbdc_vhub_desc.h"
#include "usbdc_vhub.h"
#include "usbdc_vhid.h"
#include "usbdc_vhid_desc.h"
#include "usbdc_hal.h"
#include "usbdc_vmsc.h"
#include "usbdc_vmsc_desc.h"
#include "usbdc_vendor.h"
//-----------------------------------------------------
//$ USB Host Control
//-----------------------------------------------------
#include "usbhc_regs.h" 
#include "usbhc_core.h"
#if !defined(HID_PARSER_CORE_KB) && !defined(HID_PARSER_CORE_MS)
#include "usbhc_hidparser.h"
#endif
#include "usbhc.h"
#include "usbhc_roothub.h"
#include "task_main.h"
#include "usbhc_ioreq.h"
#include "usbhc_stdreq.h"
#include "usbhc_hid.h"
#include "usbhc_hub.h"
#if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST))
#include "usbhc_msc_burst.h"
#include "usbhc_msc_bot_burst.h"
#endif /* if ((SYSTEM_MSC_DEVICE_SUPPORT || SYSTEM_EXTENDER_MSC_SUPPORT) && (SYSTEM_USB_HC_BURST)) */
#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)
#include "usbhc_audio.h"
#endif //#if (SYSTEM_USBAUDIO_DEVICE_SUPPORT)




//-----------------------------------------------------
//$ KVM Extender System
//-----------------------------------------------------
#if (SYSTEM_HUART_SUPPORT)
#include "hsuart.h"
#include "hsuart2.h"
#endif

#if (SYSTEM_EXTENDER_SUPPORT)
#include "extender.h"
#if (SYSTEM_EXTENDER_RECEIVER)
#include "extender_receiver.h"
#include "ISL59920.h"
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */
#if (SYSTEM_EXTENDER_TRANSMITTER)
#include "extender_transmitter.h"
#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */
#endif /* #if (SYSTEM_EXTENDER_SUPPORT) */

#if (SYSETM_GPIO_EEPROM_ENABLE)
#include "24c02_gpio.h"
#endif
//-----------------------------------------------------
//$ USB Special Handler
//-----------------------------------------------------
#include  "logitech.h"
#include  "a4_tech.h"

//-----------------------------------------------------
//$ IR Device Handle
//-----------------------------------------------------
#if (SYSTEM_IR_DEVICE_SUPPORT)
#include  "kvm_ir.h"
#endif /* #if (SYSTEM_IR_DEVICE_SUPPORT) */		

#ifdef UART_CONSOLE
#include  "uartx_console.h"
#endif

#if defined(HSUART)
#include  "hsuart.h"
#include  "hsuart_core.h"
#include  "hsuart_driver.h"
#endif

#ifdef MULTIVIEW
#include  "kvm_multiview.h"
#endif

#ifdef RTC
#include  "kvm_rtc.h"
#endif

#ifdef IR
#include  "kvm_ir.h"
#endif

#ifdef SYNC
#include  "km_sync.h"
#endif

#ifdef KMLOG
#include  "km_log.h"
#endif

#ifdef API
#include "km_api.h"
#endif

#ifdef I2C_BUS
#include "i2c_core.h"
#endif