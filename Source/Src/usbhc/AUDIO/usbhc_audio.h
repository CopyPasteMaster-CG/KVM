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
 * Module Name : usbhc_audio.h
 * Purpose     : USB HC Audio Class Header file
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 *================================================================================
 */
#ifndef _USB_HC_AUDIO_H
#define _USB_HC_AUDIO_H

/* INCLUDE FILE DECLARATIONS */


/* MACRO DECLARATIONS */
#define USBAUDIO_REPORT_ALL     1
#define USBAUDIO_REPORT_SINGLE  0

/* TYPE DECLARATIONS */

/* EXPORTED GLOBAL VARIABLES FOR OTHER SUBPROGRAM USED */
extern U8_T		USBHC_Audio_Device_Cnt;
extern bit		USBHC_Audio_IN_Flag;
extern bit 		USBHC_Audio_Switch_In_Processing_Flag,USBHC_Audio_Hotkey_Sw_Flag;
extern bit		USBHC_Audio_Enumerate_Processing_Flag;
extern U8_T		USBHC_Audio_Device_Cnt;
extern bit		USBHC_Audio_IN_Start_Flag;
extern U8_T		USBHC_Audio_In_Endpinx;
extern U8_T		USBHC_Audio_Out_Endpinx;
extern U8_T		USBHC_Audio_In_Interface;
extern U8_T		USBHC_Audio_Out_Interface;
extern U8_T		USBHC_Audio_Devinx;
extern U8_T		USBHC_Audio_Enumerate_Task_Id;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void USBHC_AUDIO_Init(void);
void USBHC_AUDIO_Switch(U8_T KVM_NextAudio);
void USBHC_AUDIO_In_Start(void);
void USBHC_AUDIO_In_Stop(U8_T pid);
void USBHC_AUDIO_ReportModeChanged_Mount_Control(U8_T);
void USBHC_AUDIO_In_Control_Check(U8_T devinx,U8_T intf_id,U8_T configuration);
void USBHC_AUDIO_Sync(U8_T up);
void USBHC_AUDIO_Sync_Disable(void);
void USBHC_KVM_USB_Audio_Port_Sw(U8_T newport);
#endif /* End of _USB_HC_AUDIO_H */
