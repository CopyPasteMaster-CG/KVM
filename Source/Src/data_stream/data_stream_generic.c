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
 * Module Name : data_stream_generic.c
 * Purpose     : handle keyboard & mouse data streaming
 * Author      : Jack Wang
 * Date        :
 * Notes       : None
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include <stdio.h>
#include <string.h>

#include "project_include.h"

#if (PROJECT_USB_GENERIC_HID_ENABLE)
/*$GLOBAL DEFINATION DECLARATIONS */
/*$GLOBAL CONTANT DECLARATIONS 		*/   

	/*****************************************************************************/
	/*    PS2 Translate table for TYPE F0 key code                               */
	/*****************************************************************************/
	CONST U8_T Table_PS2_F0[] =
	{
		// 00,  01 , 02 , 03 , 04 , 05 , 06 , 07 , 08 , 09 , 0a , 0b , 0c , 0d ,0e , 0f
		 0x00,0x42,0x00,0x3e,0x3c,0x3a,0x3b,0x45,0x68,0x43,0x41,0x3f,0x3d,0x2b,0x35,0x67,
		// 10 , 11 , 12 , 13 , 14 , 15 , 16 , 17 , 18 , 19 , 1a , 1b , 1c , 1d ,1e , 1f
		 0x69,0xe2,0xe1,0x88,0xe0,0x14,0x1e,0x00,0x6a,0x00,0x1d,0x16,0x04,0x1a,0x1f,0x00,
		// 20 , 21 , 22 , 23 , 24 , 25 , 26 , 27 , 28 , 29 , 2a , 2b , 2c , 2d ,2e , 2f
		 0x6b,0x06,0x1b,0x07,0x08,0x21,0x20,0x8c,0x6c,0x2c,0x19,0x09,0x17,0x15,0x22,0x68,
		// 30 , 31 , 32 , 33 , 34 , 35 , 36 , 37 , 38 , 39 , 3a , 3b , 3c , 3d ,3e , 3f
		 0x6d,0x11,0x05,0x0b,0x0a,0x1c,0x23,0x69,0x6e,0x00,0x10,0x0d,0x18,0x24,0x25,0x6a,
		// 40 , 41 , 42 , 43 , 44 , 45 , 46 , 47 , 48 , 49 , 4a , 4b , 4c , 4d ,4e , 4f
		 0x6f,0x36,0x0e,0x0c,0x12,0x27,0x26,0x00,0x70,0x37,0x38,0x0f,0x33,0x13,0x2d,0x00,
		// 50 , 51 , 52 , 53 , 54 , 55 , 56 , 57 , 58 , 59 , 5a , 5b , 5c , 5d ,5e , 5f
		 0x71,0x87,0x34,0x03,0x2f,0x2e,0x00,0x72,0x39,0xe5,0x28,0x30,0x00,0x31,0x00,0x94,
		// 60 , 61 , 62 , 63 , 64 , 65 , 66 , 67 , 68 , 69 , 6a , 6b , 6c , 6d ,6e , 6f
		 0x00,0x64,0x93,0x92,0x8a,0x00,0x2a,0x8b,0x00,0x59,0x89,0x5c,0x5f,0x85,0x00,0x00,
		// 70 , 71 , 72 , 73 , 74 , 75 , 76 , 77 , 78 , 79 , 7a , 7b , 7c , 7d ,7e , 7f
		 0x62,0x63,0x5a,0x5d,0x5e,0x60,0x29,0x53,0x44,0x57,0x5b,0x56,0x55,0x61,0x47,0x00,
		// 80 , 81 , 82 , 83 , 84 , 85 , 86 , 87 , 88 , 89 , 8a , 8b , 8c , 8d ,8e , 8f
		 0x00,0x00,0x00,0x40,0x46,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  
	};
	
	/*****************************************************************************/
	/*    PS2 Translate table for TYPE E0 key code                               */
	/*****************************************************************************/
	CONST U8_T Table_PS2_E0[] = 
	{
		// 00 , 01 , 02 , 03 , 04 , 05 , 06 , 07 , 08 , 09 , 0a , 0b , 0c , 0d ,0e , 0f
		 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		// 10 , 11 , 12 , 13 , 14 , 15 , 16 , 17 , 18 , 19 , 1a , 1b , 1c , 1d ,1e , 1f
		 0xBe,0xe6,0x00,0x00,0xe4,0xb4,0x00,0x00,0xC4,0x00,0x00,0x00,0x00,0x00,0x00,0xe3,
		// 20 , 21 , 22 , 23 , 24 , 25 , 26 , 27 , 28 , 29 , 2a , 2b , 2c , 2d ,2e , 2f
		 0xC3,0xB9,0x00,0xB7,0x00,0x00,0x00,0xe7,0xC2,0x00,0x00,0xBc,0x00,0x92,0x00,0x65,
		// 30 , 31 , 32 , 33 , 34 , 35 , 36 , 37 , 38 , 39 , 3a , 3b , 3c , 3d ,3e , 3f
		 0xC1,0x00,0xB8,0x00,0xB6,0x00,0x00,0xb0,0xC0,0x00,0xBf,0xb5,0x00,0x00,0x00,0xb1,
		// 40 , 41 , 42 , 43 , 44 , 45 , 46 , 47 , 48 , 49 , 4a , 4b , 4c , 4d ,4e , 4f
		 0xBd,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xBb,0x00,0x54,0x00,0x00,0xb3,0x00,0x00,
		// 50 , 51 , 52 , 53 , 54 , 55 , 56 , 57 , 58 , 59 , 5a , 5b , 5c , 5d ,5e , 5f
		 0xBa,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x58,0x00,0x00,0x00,0xb2,0x00,
		// 60 , 61 , 62 , 63 , 64 , 65 , 66 , 67 , 68 , 69 , 6a , 6b , 6c , 6d ,6e , 6f
		 0x00,0x00,0x00,0x00,0x00,0x00,0x66,0x00,0x00,0x4d,0x00,0x50,0x4a,0x00,0x00,0x00,
		// 70 , 71 , 72 , 73 , 74 , 75 , 76 , 77 , 78 , 79 , 7a , 7b , 7c , 7d ,7e , 7f
		 0x49,0x4c,0x51,0x00,0x4f,0x52,0x00,0x48,0x00,0x00,0x4e,0x00,0x46,0x4b,0x48,0x00,
		// 80 , 81 , 82 , 83 , 84 , 85 , 86 , 87 , 88 , 89 , 8a , 8b , 8c , 8d ,8e , 8f
		 0x04,0x01,0x02,0x44,0x45,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x40,0x41,0x1e,0x1f,0x20,
		// 90 , 91 , 92 , 93 , 94 , 95 , 96 , 97 , 98 , 99 , 9a , 9b , 9c , 9d ,9e , 9f
		 0x00,0x00,0x00,0x00,0x00,0x78,0x76,0x65,0x75,0x66,0x77,0x74,0x7e,0x79,0x7a,0x7c,
		// a0 , a1 , a2 , a3 , a4 , 
		 0x7d,0x7b,0x7f,0x81,0x80 
	};
	
	/*****************************************************************************/
	/* USB Translate table for TYPE MulityMedia key code                         */
	/*****************************************************************************/
	CONST PS2_2_USB_MultiMediaTypeDef  PS2_USB_MultiMediaTab[21] = 
	{
		 0x02,0x81,0x00,  // 01.System Power
		 0x02,0x82,0x00,  // 02.System Sleep
		 0x02,0x83,0x00,  // 03.System Wake
		 0x03,0xb5,0x00,  // 04.Scan Next Track
		 0x03,0xb6,0x00,  // 05.Scan Previous Track
		 0x03,0xb7,0x00,  // 06.Stop
		 0x03,0xcd,0x00,  // 07.Play/Pause
		 0x03,0xe2,0x00,  // 08.Mute
		 0x03,0xe9,0x00,  // 09.Volume Up
		 0x03,0xea,0x00,  // 10.Volume Down
		 0x03,0x83,0x01,  // 11.Media Select
		 0x03,0x8a,0x01,  // 12.Mail
		 0x03,0x92,0x01,  // 13.Caculator
		 0x03,0x94,0x01,  // 14.My Computer
		 0x03,0x21,0x02,  // 15.WWW Search
		 0x03,0x23,0x02,  // 16.WWW Home
		 0x03,0x24,0x02,  // 17.WWW Back
		 0x03,0x25,0x02,  // 18.WWW Forward
		 0x03,0x26,0x02,  // 19.WWW Stop
		 0x03,0x27,0x02,  // 20.WWW Refresh
		 0x03,0x2a,0x02   // 21.WWW Favorite
	};
			  
/* $GLOBAL VARIABLES DECLARATIONS 	*/ 
//---------------------------------------------------------------------------------------
// GENERIC USB QUEUE
bit  DATAST_Generic_KB_Change_Flag;
U8_T TASK_DATAST_PS2_Generic_KB_ActiveFlag; 
U8_T TASK_DATAST_PS2_Generic_MS_ActiveFlag; 		
	
#ifdef SYNC	
U8_T DATAST_Generic_USB_KB_Queue[USBDC_PORT_MAX][8];  /* Byte1-Modifiers,Byte2-Reverse,Byte3~8=>Data */
#else
U8_T DATAST_Generic_USB_KB_Queue[8];  /* Byte1-Modifiers,Byte2-Reverse,Byte3~8=>Data */
#endif
	
U8_T DATAST_Generic_USB_MS_Queue[8];  /* Byte0-Package Length,Byte1-Page ID,Byte2~BYte5 Data */
U8_T DATAST_Generic_USB_MS_Report[6]; /* The first byte indicate the buffer is valid */	
	
#ifdef SYNC	
U8_T DATAST_Generic_KB_IN_Queue[KVM_MAX_PORT][DATAST_GENERIC_KB_IN_MAX]; 		/* Byte1-Modifiers,Byte2-Reverse,Byte3~8=>Data */	
#else		
U8_T DATAST_Generic_KB_IN_Queue[DATAST_GENERIC_KB_IN_MAX];		/* Byte1-Modifiers,Byte2-Reverse,Byte3~8=>Data */
#endif
	
#ifdef SYNC
	#if (DATAST_GENERIC_MS_IN_QUEUE)
	U8_T DATAST_Generic_MS_IN_Queue[DATAST_GENERIC_MS_IN_MAX][8];
	U8_T DATAST_Generic_MS_Port_Mask[DATAST_GENERIC_MS_IN_MAX];
	#else
	U8_T DATAST_Generic_MS_IN_Queue[KVM_MAX_PORT][8];
	#endif
	U8_T DATAST_Generic_KB_IN_WP[KVM_MAX_PORT],DATAST_Generic_KB_IN_RP[KVM_MAX_PORT];	
	U8_T DATAST_Generic_MS_Port_Mask[DATAST_GENERIC_MS_IN_MAX];
#else	
	U8_T DATAST_Generic_MS_IN_Queue[DATAST_GENERIC_MS_IN_MAX][8];
	U8_T DATAST_Generic_KB_IN_WP,DATAST_Generic_KB_IN_RP;  
	U8_T DATAST_Generic_MS_Port_Mask[DATAST_GENERIC_MS_IN_MAX];
#endif	

#ifdef SYNC
#if (DATAST_GENERIC_MS_IN_QUEUE)
U8_T DATAST_Generic_MS_IN_WP,DATAST_Generic_MS_IN_RP[8];  /* Byte1-Modifiers,Byte2-Reverse,Byte3~8=>Data */
#else
U8_T DATAST_Generic_MS_IN_WP[KVM_MAX_PORT],DATAST_Generic_MS_IN_RP[KVM_MAX_PORT];  /* Byte1-Modifiers,Byte2-Reverse,Byte3~8=>Data */
#endif /* #if (DATAST_GENERIC_MS_IN_QUEUE) */
//U8_T DATAST_Generic_MS_IN_WP[KVM_MAX_PORT],DATAST_Generic_MS_IN_RP[KVM_MAX_PORT]; 
#else
U8_T DATAST_Generic_MS_IN_WP,DATAST_Generic_MS_IN_RP; 
#endif

U8_T TASK_DATAST_PS2_Generic_KB_ID;
U8_T TASK_DATAST_PS2_Generic_MS_ID;
U8_T TASK_DATAST_PS2_Generic_MS_ID2;

static U8_T ps2RepeatCode;
static U8_T DATAST_PS2_RepeatKeyCheck(U8_T keyType, U8_T keyCode);
//---------------------------------------------------------------------------------------
/*$STATIC VARIABLE DECLARATIONS  	*/
/*$LOCAL SUBPROGRAM DECLARATIONS 	*/
void DATAST_PS2_MS_Convert2_USB(U8_T *ps2_msdata);
void TASK_DATAST_PS2_Generic_MS_Output_Handle(void);
void DATAST_Append_Generic_MS_IN_Q(U8_T datatype,U8_T *datacode);

/*$LOCAL SUBPROGRAM START 	*/
/*
 * -------------------------------------------------------------------------------
 * void    DATAST_Generic_Queue_Init(void)
 * Purpose : Initializes the Data stream control 
 *       
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */
void DATAST_Generic_Queue_Init(void)
{
	memset(DATAST_Generic_USB_KB_Queue,0x00,sizeof(DATAST_Generic_USB_KB_Queue));
	memset(DATAST_Generic_USB_MS_Queue,0x00,sizeof(DATAST_Generic_USB_MS_Queue));
#ifdef SYNC	
	memset(DATAST_Generic_KB_IN_WP,0x00,sizeof(DATAST_Generic_KB_IN_WP));
	memset(DATAST_Generic_KB_IN_RP,0x00,sizeof(DATAST_Generic_KB_IN_RP));	
#if (DATAST_GENERIC_MS_IN_QUEUE)
	DATAST_Generic_MS_IN_WP = 0;
#else
	memset(DATAST_Generic_MS_IN_WP,0x00,sizeof(DATAST_Generic_MS_IN_WP));
#endif		
	memset(DATAST_Generic_MS_IN_RP,0x00,sizeof(DATAST_Generic_MS_IN_RP));		
#else	
	DATAST_Generic_KB_IN_WP = 0;
	DATAST_Generic_KB_IN_RP = 0;
	DATAST_Generic_MS_IN_WP = 0;
	DATAST_Generic_MS_IN_RP = 0;
#endif		
	DATAST_Generic_KB_Change_Flag = 0;   /* Indicate the Generic USB KB Queue Changed */
	ps2RepeatCode = 0;	
}

/*
 * -------------------------------------------------------------------------------
 * void    DATAST_Reset_GenericQueue(void)
 * Purpose : Initializes the Data stream control 
 *       
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */
void DATAST_Reset_GenericQueue(void)
{
	USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_KB_ENDP_NUM] |= USBDC_UPORT[KVM_CurrentHost];
	USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_MS_ENDP_NUM] |= USBDC_UPORT[KVM_CurrentHost];
	DATAST_Generic_Queue_Init();
} /* End of DATAST_Init */ 

/*
 * -------------------------------------------------------------------------------
 * void DATAST_Keyboard_Send(U8_T usb_keytype,U8_T usb_keycode)
 * Purpose :  
 *       
 * Params  : none
 * Returns : none
 * Note    : 
 * -------------------------------------------------------------------------------
 */
#ifdef SYNC
void DATAST_Keyboard_Send(U8_T port,U8_T usb_keytype,U8_T usb_keycode)
{	
	//printf("S1");
	if (KVM_Check_KB_Skip_Condition()==0)
	{		
		//printf("S2:%02bx",TASK_DATAST_PS2_Generic_KB_ActiveFlag);
		DATAST_Append_Generic_KB_In_Queue(port,usb_keytype,usb_keycode);
		if ((TASK_DATAST_PS2_Generic_KB_ActiveFlag & BIT_MASK[port]) == 0)
		{
			//printf("S3");
			if (DATAST_Generic_KB_IN_WP[port] != DATAST_Generic_KB_IN_RP[port]) // check if there is data needed processed
			{
				//printf("p:%bu,task\n\r",port);
				TASK_DATAST_PS2_Generic_KB_ActiveFlag |= BIT_MASK[port];
				TASK_Active(TASK_TYPE_EVENT,TASK_DATAST_PS2_Generic_KB_ID,0,port,0,0); // now aticev the Generic KB Output Handle
			}
		}
	}		
}
#else
void DATAST_Keyboard_Send(U8_T port,U8_T usb_keytype,U8_T usb_keycode)
{
	port = 0;
	if (KVM_Check_KB_Skip_Condition()==0)
	{		
		DATAST_Append_Generic_KB_In_Queue(usb_keytype,usb_keycode);
		if (TASK_DATAST_PS2_Generic_KB_ActiveFlag  == 0)
		{
			if (DATAST_Generic_KB_IN_WP != DATAST_Generic_KB_IN_RP) // check if there is data needed processed
			{
				TASK_DATAST_PS2_Generic_KB_ActiveFlag =1;
				TASK_Active(TASK_TYPE_EVENT,TASK_DATAST_PS2_Generic_KB_ID,0,0,0,0); // now aticev the Generic KB Output Handle
			}
		}
	}		
}	
#endif
/**----------------------------------------------------------------------------------------------------------
 * void    DATAST_PS2_Convert_To_USB(U8_T codetype,U8_T buflen,U8_T *buf,U8_T *usb_keytype,U8_T *usb_keycode)
 * Purpose : Initializes the Data stream control      
 * Params  : $codetype
 *           $buflen
 *           $buf:
 *           $usb_keytype
 *           $usb_keycode
 * Returns : none
 * Note    : 
 *-----------------------------------------------------------------------------------------------------------*/
#if (SYSTEM_PS2_HOST_ENABLE)
void DATAST_PS2_Convert_To_USB(U8_T codetype,U8_T buflen,U8_T *buf,U8_T *usb_keytype,U8_T *usb_keycode)
{
	U8_T ps2_code;

	/*1.Process the Key type first */
	/*2.Convert the keycode */
	ps2_code = buf[buflen-1];
	if (codetype <= KB_TYPE_BREAK_F0 )
	{
		*usb_keycode = DATAST_TranslateF0(ps2_code);  // get usb cancode
	}
	else
	{
		*usb_keycode = DATAST_TranslateE0(ps2_code);   // get usb cancode
	}

	/* Check Multi Media Key */
	if ((*usb_keycode >= 0xb0) && (*usb_keycode <= 0xc4)) // system key & multi media key group
	{
		if ((codetype & 0x01) == 0x00) 
			*usb_keytype = HID_USAGE_PAGE_0C_MAKE;
		else 
			*usb_keytype = HID_USAGE_PAGE_0C_BREAK;
	}
	else
	{
		if ((codetype & 0x01) == 0x00) 
			*usb_keytype = HID_USAGE_PAGE_07_MAKE;
		else 
			*usb_keytype = HID_USAGE_PAGE_07_BREAK;
	}
}

/******************************************************************************/
/*
**  Function Name: TranslateF0
**  Parameter    : code, ps/2 input scancode with F0 type
**  Description  : Translate the PS/2 input scan code into USB scan code
**
**
*/
U8_T DATAST_TranslateF0(U8_T usbcode)
{
	switch ( usbcode )
	{
		case 0xf1:
			return 0x91;
		case 0xf2:
			return 0x90;
	}

	return Table_PS2_F0[usbcode];
}

/******************************************************************************/
/*
**  Function Name: TranslateE0
**  Parameter    : code, ps/2 input scancode with E0 type
**  Description  : Translate the PS/2 input scan code into USB scan code
**
**
*/
U8_T DATAST_TranslateE0(U8_T ps2Code)
{
	if ( ps2Code <= 0x7F )
		return  Table_PS2_E0[ps2Code];
	return (0);
}
#endif

/******************************************************************************/
/*
**  Function Name: DATAST_PS2_KB_Handle(U8_T codetype,U8_T buflen,U8_T *buf)
**  Parameter    : 
**  Description  : Translate the PS/2 input scan code into USB scan code
**
**
*/
#if (SYSTEM_PS2_HOST_ENABLE)
void DATAST_PS2_KB_Handle(U8_T codetype,U8_T buflen,U8_T *buf)
{
	U8_T usb_keytype,usb_keycode=0;

#if (SYSTEM_EXTENDER_RECEIVER)
	if (KVM_CurrentHost == REMOTE_HOST_PORT)
	{
		goto PS2_KB_HANDLE_NEXT_01;
	}	
#endif		

	/*1.Convert keytype & keycode into KVM_USB_Keytype,KVM_USB_Keycode */
	DATAST_PS2_Convert_To_USB(codetype,buflen,buf,&usb_keytype,&usb_keycode);

	if (usb_keycode == 0)
		return;

	/*2.Check the hotkey setting */
	if (DATAST_PS2_RepeatKeyCheck(usb_keytype, usb_keycode) == 0)
	{
#if (SYSTEM_EXTENDER_RECEIVER)		
PS2_KB_HANDLE_NEXT_01:		
#endif	

#if (KVM_HOTKEY_SUPPORT)
		Kb_devinx = USBDC_VHID_DEVINX;
		HOTKEY_Handle(usb_keytype,usb_keycode); // check necessary operation, hotkey or autoscan break .....
#endif
		
#if (SYSTEM_EXTENDER_RECEIVER)
		if (KVM_CurrentHost == REMOTE_HOST_PORT)
		{			
#ifndef ONEHID			
			ExtenderR_PS2_DataTransfer(EXTENDER_PS2_KEYBOARD,buf,buflen);
#endif			
			return;
		}
#endif				
#ifdef SYNC
		DATAST_Keyboard_Send(KVM_CurrentHost,usb_keytype,usb_keycode);
#else		
		DATAST_Keyboard_Send(usb_keytype,usb_keycode);
#endif		
		/*
		if (KVM_Check_KB_Skip_Condition()==0)
		{
			DATAST_Append_Generic_KB_In_Queue(usb_keytype,usb_keycode);
			if (TASK_DATAST_PS2_Generic_KB_ActiveFlag  == 0)
			{
				if (DATAST_Generic_KB_IN_WP != DATAST_Generic_KB_IN_RP) // check if there is data needed processed
				{
					TASK_DATAST_PS2_Generic_KB_ActiveFlag =1;
					TASK_Active(TASK_TYPE_EVENT,TASK_DATAST_PS2_Generic_KB_ID,0,0,0,0); // now aticev the Generic KB Output Handle
				}
			}
		}		
		*/	
	}
	
	/*3.Check the Autoscan Sotp conditon */
	/*4.Put into Generic KB Queue */
}

/*----------------------------------------------------------------------------
 * voi DATAST_PS2_RepeatKeyCheck(U8_T keyType,U8_T keyCode)
 * Purpose : Check the key received is repeat
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
static U8_T DATAST_PS2_RepeatKeyCheck(U8_T keyType, U8_T keyCode)
{
	if ((keyType & 0x01) == 0)	// this is make key
	{
		// For PS/2 will generate repeat key, so this procedure
		// will handle the repeat status
		if (ps2RepeatCode == 0x00) // the first key
		{
			ps2RepeatCode = keyCode;
		}
		else
		{
			if (ps2RepeatCode == keyCode)	// repeat key, skip it !!!!!!
			{
				return 1;
			}
		}
	}
	else	// Break Key, reset repeat key
	{
		// keyboard break key, reset the repeat condition
		ps2RepeatCode = 0x00;
	}

	return 0;
}
#endif

/******************************************************************************/
/*
**  Function Name: DATAST_PS2_MS_Handle(void)
**  Parameter    : 
**  Description  : Translate the PS/2 input scan code into USB scan code
**
**
*/
#ifdef SYNC
void DATAST_PS2_MS_Handle(U8_T usb_type)
{
	U8_T pid,rp;	
	#if (DATAST_GENERIC_MS_IN_QUEUE==0)	
	U8_T maxport;
	#endif
	/*1.Convert keytype & keycode into KVM_USB_Keytype,KVM_USB_Keycode */	
	//if (KVM_Check_MS_Skip_Condition()==0)	
#if (DATAST_GENERIC_MS_IN_QUEUE)	
	for (pid=0; pid < KVM_MAX_PORT ; pid++)
	{
		if (KM_SYNC_Port_Setting(pid) == 0)
		{	
			rp = DATAST_Generic_MS_IN_RP[pid];
			if (DATAST_Generic_MS_Port_Mask[rp] & BIT_MASK[pid])
			{	
				//printf("{%02bx}",DATAST_Generic_MS_Port_Mask[rp]);
				if ((TASK_DATAST_PS2_Generic_MS_ActiveFlag & BIT_MASK[pid]) == 0)
				{			
					if (DATAST_Generic_MS_IN_WP != DATAST_Generic_MS_IN_RP[pid]) // check if there is data needed processed#endif			
					{		
						TASK_DATAST_PS2_Generic_MS_ActiveFlag |= BIT_MASK[pid];		
						if (TASK_DATAST_PS2_Generic_MS_ID2==0)
							TASK_DATAST_PS2_Generic_MS_ID2 = TASK_DATAST_PS2_Generic_MS_ID;					
						//printf("[T0:%bu.%bu]",TASK_DATAST_PS2_Generic_MS_ID2,TASK_DATAST_PS2_Generic_MS_ID);
						TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_DATAST_PS2_Generic_MS_ID2,usb_type,pid,1,1); // now aticev the Generic KB Output Handle
						continue;
					}
				}
			}
		}			
		DATAST_Generic_MS_IN_RP[pid] = DATAST_Generic_MS_IN_WP;					
	}
#else	
	if (KVM_Flash.cSystemFlag2 & (SYSTEM_ALL_SYNC_MASK | SYSTEM_MS_SYNC_MASK))		
	{
		pid=0;
		maxport = KVM_MAX_PORT;
	}
	else
	{
		pid = KVM_CurrentHost;
		maxport = KVM_CurrentHost+1;
	}		
	
	for (; pid < maxport ; pid++)
	{
		if (KM_SYNC_Port_Setting(pid) == 0)
		{	
			if ((TASK_DATAST_PS2_Generic_MS_ActiveFlag & BIT_MASK[pid]) == 0)
			{			
				if (DATAST_Generic_MS_IN_WP[pid] != DATAST_Generic_MS_IN_RP[pid]) // check if there is data needed processed#endif			
				{		
					TASK_DATAST_PS2_Generic_MS_ActiveFlag |= BIT_MASK[pid];		
					if (TASK_DATAST_PS2_Generic_MS_ID2==0)
						TASK_DATAST_PS2_Generic_MS_ID2 = TASK_DATAST_PS2_Generic_MS_ID;					
					//printf("[T0:%bu.%bu]",TASK_DATAST_PS2_Generic_MS_ID2,TASK_DATAST_PS2_Generic_MS_ID);
					TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_DATAST_PS2_Generic_MS_ID2,usbtype,pid,1,1); // now aticev the Generic KB Output Handle
				}
			}
		}	
	}
#endif /* #ifdef DATAST_GENERIC_MS_IN_QUEUE */
}
#else //None Sync condition
void DATAST_PS2_MS_Handle(U8_T usbtype)
{
	/*1.Convert keytype & keycode into KVM_USB_Keytype,KVM_USB_Keycode */	
	if (KVM_Check_MS_Skip_Condition()==0)
	{
		if (TASK_DATAST_PS2_Generic_MS_ActiveFlag  == 0)
		{
			if (DATAST_Generic_MS_IN_WP != DATAST_Generic_MS_IN_RP) // check if there is data needed processed
			{
				TASK_DATAST_PS2_Generic_MS_ActiveFlag =1;						
				TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_DATAST_PS2_Generic_MS_ID2,usbtype,0,1,1); // now aticev the Generic KB Output Handle
			}
		}
	}
}
#endif
/******************************************************************************/
/*
**  Function Name: DATAST_Search_Generic_USBKB_Q(U8_T keycode)
**  Parameter    : 
**  Description  : 
**
**
*/
#ifdef	SYNC
U8_T DATAST_Search_Generic_USBKB_Q(U8_T port,U8_T keycode)
{
	U8_T i;

	i = 2;
SEARCH_USB_BUF_START:
	if (DATAST_Generic_USB_KB_Queue[port][i] == keycode )
		return i;
	i++;
	if ( i < 8 )
		goto  SEARCH_USB_BUF_START;
	return GENERIC_KB_NOT_FOUND;
}
#else
U8_T DATAST_Search_Generic_USBKB_Q(U8_T keycode)
{
	U8_T i;

	i = 2;
SEARCH_USB_BUF_START:
	if (DATAST_Generic_USB_KB_Queue[i] == keycode )
		return i;
	i++;
	if ( i < 8 )
		goto  SEARCH_USB_BUF_START;
	return GENERIC_KB_NOT_FOUND;
}
#endif
//****************************************************************************************************
//* Program Name : DATAST_Adjust_Generic_USBKB_Q
//* Parameter    : index, the current index 
//* Description  : addjust the byte after the index, this is no effect in windows system, but will fit for
//                 Sun micro System
//****************************************************************************************************
#ifdef	SYNC
void DATAST_Adjust_Generic_USBKB_Q(U8_T port,U8_T index)
{
	U8_T i;

	for (i=index ; i < 7 ; i++ )
	{
		DATAST_Generic_USB_KB_Queue[port][i] = DATAST_Generic_USB_KB_Queue[port][i+1];
	}
	DATAST_Generic_USB_KB_Queue[port][7] = 0; // the last byte
}
#else
void DATAST_Adjust_Generic_USBKB_Q(U8_T index)
{
	U8_T i;

	for (i=index ; i < 7 ; i++ )
	{
		DATAST_Generic_USB_KB_Queue[i] = DATAST_Generic_USB_KB_Queue[i+1];
	}
	DATAST_Generic_USB_KB_Queue[7] = 0; // the last byte
}
#endif 
//****************************************************************************************************
//* Program Name : DATAST_Remove_Generic_USBKB_Q
//* Parameter    : code=> the code need to move from the usb output queue
//* Description  : Remove the usb code into the USB output queue
//****************************************************************************************************
#ifdef SYNC
void DATAST_Remove_Generic_USBKB_Q(U8_T port,U8_T keycode)
{
	U8_T i;
	
	if (keycode == CHAR_REPEAT)
	{	
		memcpy(DATAST_Generic_USB_KB_Queue[port],KM_SYNC_Sync_KB_HID_RepeatTableBreak,8);
		DATAST_Generic_KB_Change_Flag = 1;
		return;
	}
	
	i=DATAST_Search_Generic_USBKB_Q(port,keycode);	// is the code in buffer already?
	if (i != GENERIC_KB_NOT_FOUND)				// if find the code
	{
		DATAST_Adjust_Generic_USBKB_Q(port,i);		// in the queue, remove it
		DATAST_Generic_KB_Change_Flag = 1;
#if (SYSTEM_HARDWARE_PS2_ENABLE)
		//cPS2_KB_PressCnt--;
#endif		
	}
}
#else
void DATAST_Remove_Generic_USBKB_Q(U8_T keycode)
{
	U8_T i;

	i=DATAST_Search_Generic_USBKB_Q(keycode);	// is the code in buffer already?
	if (i != GENERIC_KB_NOT_FOUND)				// if find the code
	{
		DATAST_Adjust_Generic_USBKB_Q(i);		// in the queue, remove it
		DATAST_Generic_KB_Change_Flag = 1;
#if (SYSTEM_HARDWARE_PS2_ENABLE)
		cPS2_KB_PressCnt--;
#endif		
	}
}
#endif
//****************************************************************************************************
//* Program Name : DATAST_Append_Generic_USBKB_Q
//* Parameter    : code=> the code need to add into the usb output queue
//* Description  : Add the usb code into the USB output queue
//****************************************************************************************************
#ifdef SYNC
void DATAST_Append_Generic_USBKB_Q(U8_T port,U8_T keycode)
{
	U8_T i;

	if (keycode == CHAR_REPEAT)
	{	
		memcpy(DATAST_Generic_USB_KB_Queue[port],KM_SYNC_Sync_KB_HID_RepeatTableMake[port],8);
		goto DATAST_Append_Generic_USBKB_Q_Valid;
	}
		
	i = DATAST_Search_Generic_USBKB_Q(port,keycode);

	if (i != GENERIC_KB_NOT_FOUND) // is the code in buffer already?
	{
		return;
	}
	
	i = DATAST_Search_Generic_USBKB_Q(port,0x00); // find out the first byte for 0x00

	if (i == GENERIC_KB_NOT_FOUND)      // where is the first 0 byte position
	{
		DATAST_Adjust_Generic_USBKB_Q(port,2);
		i = 7; // the last byte
	}
	DATAST_Generic_USB_KB_Queue[port][i] = keycode; 
DATAST_Append_Generic_USBKB_Q_Valid:
	DATAST_Generic_KB_Change_Flag = 1;
#if (SYSTEM_PS2_HOST_ENABLE)
	//cPS2_KB_PressCnt++;
#endif	
}
#else
void DATAST_Append_Generic_USBKB_Q(U8_T keycode)
{
	U8_T i;

	i = DATAST_Search_Generic_USBKB_Q(keycode);

	if (i != GENERIC_KB_NOT_FOUND) // is the code in buffer already?
	{
		return;
	}
	
	i = DATAST_Search_Generic_USBKB_Q(0x00); // find out the first byte for 0x00

	if (i == GENERIC_KB_NOT_FOUND)      // where is the first 0 byte position
	{
		DATAST_Adjust_Generic_USBKB_Q(2);
		i = 7; // the last byte
	}
	DATAST_Generic_USB_KB_Queue[i] = keycode; 
	DATAST_Generic_KB_Change_Flag = 1;
#if (SYSTEM_PS2_HOST_ENABLE)
	cPS2_KB_PressCnt++;
#endif	
}
#endif
/******************************************************************************/
/*
**  Function Name: DATAST_Append_Generic_KB_In_Byte(U8_T byte)
**  Parameter    : 
**  Description  : 
**
**
*/
#ifdef SYNC
void DATAST_Append_Generic_KB_In_Byte(U8_T port,U8_T byte)
{
	DATAST_Generic_KB_IN_Queue[port][DATAST_Generic_KB_IN_WP[port]] = byte;
	DATAST_Generic_KB_IN_WP[port]++;
	if (DATAST_Generic_KB_IN_WP[port] >= DATAST_GENERIC_KB_IN_MAX)
	{
		DATAST_Generic_KB_IN_WP[port] = 0;
	}
}
#else
void DATAST_Append_Generic_KB_In_Byte(U8_T byte)
{
	DATAST_Generic_KB_IN_Queue[DATAST_Generic_KB_IN_WP] = byte;
	DATAST_Generic_KB_IN_WP++;
	if (DATAST_Generic_KB_IN_WP >= DATAST_GENERIC_KB_IN_MAX)
	{
		DATAST_Generic_KB_IN_WP = 0;
	}
}
#endif
/******************************************************************************/
/*
**  Function Name: DATAST_Append_Generic_KB_In_Queue(U8_T codetype,U8_T buflen,U8_T *buf)
**  Parameter    : 
**  Description  : 
**
**
*/
#ifdef SYNC
void DATAST_Append_Generic_KB_In_Queue(U8_T port,U8_T usb_keytype,U8_T usb_keycode) 
{
	U8_T byte;

	byte = DATAST_PACKAGE_DATA_USB | (usb_keytype << 4) | 0x01;

	DATAST_Append_Generic_KB_In_Byte(port,byte);
	DATAST_Append_Generic_KB_In_Byte(port,usb_keycode);

}
#else
void DATAST_Append_Generic_KB_In_Queue(U8_T usb_keytype,U8_T usb_keycode) 
{
	U8_T byte;

	byte = DATAST_PACKAGE_DATA_USB | (usb_keytype << 4) | 0x01;

	DATAST_Append_Generic_KB_In_Byte(byte);
	DATAST_Append_Generic_KB_In_Byte(usb_keycode);

}
#endif
/**************************************************************************************/
/*
**  Function Name: DATAST_Append_Generic_MS_IN_Q(U8_T codetype,U8_T buflen,U8_T *buf)
**  Parameter    : 
**  Description  : 
**
**
*/
void DATAST_Append_Generic_MS_IN_Q(U8_T datatype,U8_T *datacode)
{
	#if (DATAST_GENERIC_MS_IN_QUEUE==0)
	U8_T pid;
	#endif	
	U8_T port_mask;
	U8_T data_len;


#ifdef SYNC	
	//Handle the sync condition
	if (KVM_Flash.cSystemFlag2 & SYSTEM_ALL_SYNC_MASK) //if in sync mode
	{
		port_mask = 0xff;
	}		
	else
	{
		port_mask = BIT_MASK[KVM_CurrentHost];		
	}		
#endif /* #ifdef SYNC */
	
#if (DATAST_GENERIC_MS_IN_QUEUE)	
	DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_WP][0] = datacode[0];
	data_len = 3;
	if (datatype == GENERIC_USAGE_0C_MAKE)
	{				
		DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_WP][1] = datacode[1];
		DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_WP][2] = datacode[2];
	}
	else if (datatype == GENERIC_USAGE_0C_BREAK)
	{
		DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_WP][1] = 0x00;
		DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_WP][2] = 0x00;
	}
	else if (datatype == GENERIC_USAGE_07_MAKE) //relative mouse data
	{
		data_len = 4;
		DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_WP][0] = 0x01; //normal mouse data page id 1
		memcpy(&DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_WP][1],datacode,4);
	}
	else
	{
		data_len = 6;
		datatype = GENERIC_USAGE_ABS_MAKE;
		DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_WP][0] = 0x04; //normal mouse data page id 4
		memcpy(&DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_WP][1],datacode,6);
	}		
	DATAST_Generic_MS_Port_Mask[DATAST_Generic_MS_IN_WP] = port_mask;
	DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_WP][7] = data_len;
	DATAST_Generic_MS_IN_WP++;
	if (DATAST_Generic_MS_IN_WP >= DATAST_GENERIC_MS_IN_MAX)
	{
		DATAST_Generic_MS_IN_WP = 0;
	}
#else //NO Queue Design
#ifdef SYNC
	for (pid=0; pid < KVM_MAX_PORT ; pid++)
	{
		DATAST_Generic_MS_IN_Queue[pid][0] = datacode[0];
		if (datatype == GENERIC_USAGE_0C_MAKE)
		{
			DATAST_Generic_MS_IN_Queue[pid][1] = datacode[1];
			DATAST_Generic_MS_IN_Queue[pid][2] = datacode[2];
		} 
		else if (datatype == GENERIC_USAGE_0C_BREAK)
		{
			DATAST_Generic_MS_IN_Queue[pid][1] = 0x00;
			DATAST_Generic_MS_IN_Queue[pid][2] = 0x00;
		}
		else if (datatype == GENERIC_USAGE_07_MAKE) //relative mouse data
		{
			DATAST_Generic_MS_IN_Queue[pid][0] = 0x01; //normal mouse data page id 1
			memcpy(&DATAST_Generic_MS_IN_Queue[pid][1],datacode,4);
	#if (SYSTEM_EXTENDER_RECEIVER)
			if (pid == REMOTE_HOST_PORT)
			{
	#ifndef ONEHID				
				ExtenderR_PS2_DataTransfer(EXTENDER_PS2_MOUSE,&DATAST_Generic_MS_IN_Queue[pid][1],4);
	#endif	/* #ifndef ONEHID */			
				return;
			}	
	#endif	/* #if (SYSTEM_EXTENDER_RECEIVER) */	
		}
		else
		{
			DATAST_Generic_MS_IN_Queue[pid][0] = 0x04; //normal mouse data page id 4
			memcpy(&DATAST_Generic_MS_IN_Queue[pid][1],datacode,6);
	#if (SYSTEM_EXTENDER_RECEIVER)
			if (pid == REMOTE_HOST_PORT)
			{
				ExtenderR_PS2_DataTransfer(EXTENDER_PS2_MOUSE,&DATAST_Generic_MS_IN_Queue[pid][1],6);
				return;
			}	
	#endif /* #if (SYSTEM_EXTENDER_RECEIVER) */
		}		
		DATAST_Generic_MS_IN_WP[pid] = 1;	
	}	
#else	
		DATAST_Generic_MS_IN_Queue[0][0] = datacode[0];
		if (datatype == GENERIC_USAGE_0C_MAKE)
		{
			DATAST_Generic_MS_IN_Queue[0][1] = datacode[1];
			DATAST_Generic_MS_IN_Queue[0][2] = datacode[2];
		} 
		else if (datatype == GENERIC_USAGE_0C_BREAK)
		{
			DATAST_Generic_MS_IN_Queue[0][1] = 0x00;
			DATAST_Generic_MS_IN_Queue[0][2] = 0x00;
		}
		else if (datatype == GENERIC_USAGE_07_MAKE) //relative mouse data
		{
			DATAST_Generic_MS_IN_Queue[0][0] = 0x01; //normal mouse data page id 1
			memcpy(&DATAST_Generic_MS_IN_Queue[0][1],datacode,4);
	#if (SYSTEM_EXTENDER_RECEIVER)
			if (KVM_CurrentHost == REMOTE_HOST_PORT)
			{
	#ifndef ONEHID				
				ExtenderR_PS2_DataTransfer(EXTENDER_PS2_MOUSE,&DATAST_Generic_MS_IN_Queue[0][1],4);
	#endif	/* #ifndef ONEHID */			
				return;
			}	
	#endif	/* #if (SYSTEM_EXTENDER_RECEIVER) */	
		}
		else
		{
			DATAST_Generic_MS_IN_Queue[0][0] = 0x04; //normal mouse data page id 4
			memcpy(&DATAST_Generic_MS_IN_Queue[0][1],datacode,6);
	#if (SYSTEM_EXTENDER_RECEIVER)
			if (KVM_CurrentHost == REMOTE_HOST_PORT)
			{
				ExtenderR_PS2_DataTransfer(EXTENDER_PS2_MOUSE,&DATAST_Generic_MS_IN_Queue[0][1],6);
				return;
			}	
	#endif			
		}		
		DATAST_Generic_MS_IN_WP = 1;	
#endif /* #ifdef SYNC */			
#endif /* #if (DATAST_GENERIC_MS_IN_QUEUE) */			
}

/***************************************************************************************************/
/*
**  Function Name: TASK_DATAST_PS2_Generic_KB_Output_Handle(void)
**  Parameter    : 
**  Description  : 
**
**
*/
#ifdef SYNC
void TASK_DATAST_PS2_Generic_KB_Output_Handle(void)
{
	U8_T length,datatype,datacode,modifier;
	U8_T port;

	/* 1.Check Task Ternimate Condition */	
	port = TASK_Register0;
	if (DATAST_Generic_KB_IN_WP[port] == DATAST_Generic_KB_IN_RP[port]) // check if there is data needed processed
	{
		goto TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit2;
	}
	
#if (SYSTEM_EXTENDER_RECEIVER)
	if (port == REMOTE_HOST_PORT)
	{
		goto GENERIC_KB_CHECK_SKIP;
	}
#endif			
	
	/*@@Check Port Suspend & Wakeup condition */
	if (USBDC_VHid_Resume_Check(port,USBDC_VHID_DEVINX) == SET) 
	{
		goto TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit2;
	}
	
	if (USBDC_VHid_Valid_Check(port,USBDC_VHID_DEVINX) == USB_ERROR)
	{
		DATAST_Generic_KB_IN_WP[port] = DATAST_Generic_KB_IN_RP[port] = 0; // check if there is data needed processed
		TASK_DATAST_PS2_Generic_KB_ActiveFlag &= ~BIT_MASK[port];
		return;
	}
	
#if (SYSTEM_EXTENDER_RECEIVER)
	GENERIC_KB_CHECK_SKIP:
#endif
	
	// if buffer has been filled and wait for send to host
	
	if ((USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_KB_ENDP_NUM] & USBDC_UPORT[port]) == 0) // if the buffer is not ready, just skip
	{
		goto TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit2;
	}

	/* 2.Process KB output */
	/* 2-1.Check Len & type first */
	DATAST_Generic_KB_Change_Flag = 0; // reset the key change status 
	length   = DATAST_Generic_KB_IN_Queue[port][DATAST_Generic_KB_IN_RP[port]] & DATAST_PACKAGE_LENGTH_MASK;
	datatype = DATAST_Generic_KB_IN_Queue[port][DATAST_Generic_KB_IN_RP[port]] & DATAST_PACKAGE_CODE_TYPE;	
	//if is make key, and the queue still have space to move in
	if ((datatype == GENERIC_USAGE_07_MAKE) || (datatype == GENERIC_USAGE_07_BREAK))
	{
		if (datatype == GENERIC_USAGE_07_MAKE)
		{
			
			if (DATAST_Generic_USB_KB_Queue[port][7] != 0) // still have space
			{
				if ((USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_KB_ENDP_NUM] & USBDC_UPORT[port]) == 0)
				{
					goto TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit2;
				}
			}
		}
		else // break key
		{
			// if buffer has been filled and wait for send to host
			if ((USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_KB_ENDP_NUM] & USBDC_UPORT[port]) == 0)
			{
				goto TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit2;
			}	
		}
	}
	DATAST_Generic_KB_IN_RP[port]++;
	if (DATAST_Generic_KB_IN_RP[port] >= DATAST_GENERIC_KB_IN_MAX)
		DATAST_Generic_KB_IN_RP[port] = 0;

	datacode = DATAST_Generic_KB_IN_Queue[port][DATAST_Generic_KB_IN_RP[port]];
	DATAST_Generic_KB_IN_RP[port]++;
	if (DATAST_Generic_KB_IN_RP[port] >= DATAST_GENERIC_KB_IN_MAX)
		DATAST_Generic_KB_IN_RP[port] = 0;
	if (datacode != CHAR_REPEAT)
	{	
		if (datatype <= GENERIC_USAGE_07_BREAK)
		{
			if ((datacode >= 0xe0) && (datacode <= 0xe7))
			{
				modifier = 0x01 << (datacode-0xe0);
				if (datatype == GENERIC_USAGE_07_MAKE) // Modifiers Make
				{
					if ((DATAST_Generic_USB_KB_Queue[port][0] & modifier) == 0)
					{
						DATAST_Generic_USB_KB_Queue[port][0] |= modifier;
#if (SYSTEM_PS2_HOST_ENABLE)					
						//cPS2_KB_PressCnt++;
#endif					
					}
				}
				else
				{                                   // Modifiers Break
					if (DATAST_Generic_USB_KB_Queue[port][0] & modifier)
					{
						DATAST_Generic_USB_KB_Queue[port][0] &= ~modifier;
#if (SYSTEM_PS2_HOST_ENABLE)					
						//cPS2_KB_PressCnt--;
#endif					
					}
				}

				DATAST_Generic_KB_Change_Flag  = 1;
				goto TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit;
			}
		}
	}
	switch (datatype)
	{
		case GENERIC_USAGE_07_MAKE: // this possible will make the key away.
			DATAST_Append_Generic_USBKB_Q(port,datacode);
			break;
		case GENERIC_USAGE_07_BREAK:
			DATAST_Remove_Generic_USBKB_Q(port,datacode);
			break;
		case GENERIC_USAGE_0C_MAKE: // this possible will make the key away.
		case GENERIC_USAGE_0C_BREAK:
			DATAST_Append_Generic_MS_IN_Q(datatype,(U8_T *)(&(PS2_USB_MultiMediaTab[datacode-0xb0])));
#if (SYSTEM_PS2_HOST_ENABLE)
			DATAST_PS2_MS_Handle(1);
#endif
			goto TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit2;
	}

	/* For Debug purpose */
TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit:
#if (GENERIC_USB_KB_DEBUG)
	if (DATAST_Generic_KB_Change_Flag) // content has changed
	{
		printf("KBQ=[ ");
		for (length=0; length < 8 ; length++)
		{
			printf("%02x ",(U16_T)DATAST_Generic_USB_KB_Queue[port][length]);
		}
		printf("]\n\r");
	}
#endif

	if (DATAST_Generic_KB_Change_Flag)
	{
		if (USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_KB_ENDP_NUM] & USBDC_UPORT[port])
		{
			DATAST_Generic_KB_Change_Flag = 0;
			USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_KB_ENDP_NUM] &= ~USBDC_UPORT[port];			
#if (SYSTEM_EXTENDER_RECEIVER) && (PROJECT_USB_GENERIC_HID_ENABLE)
			if (port != REMOTE_HOST_PORT)			
#endif				
			{
				USBDC_HAL_Endp_In_Buf_Move(DATAST_Generic_USB_KB_Queue[port],8,port,USBDC_VHID_DEVINX,USBDC_VHID_KB_ENDP_NUM);
				USBDC_REGS_Endp_ControlSet(port,USBDC_VHID_DEVINX,USBDC_VHID_KB_ENDP_NUM,DA_CR_BVLD_SET);
			}
		}
	}

TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit2:

	if (DATAST_Generic_KB_IN_WP[port] != DATAST_Generic_KB_IN_RP[port]) //There is still needed to handle queue 
	{
		
		TASK_Wait_Current();
	}
	else
	{
		TASK_DATAST_PS2_Generic_KB_ActiveFlag &= ~BIT_MASK[port];
	}
}

#else

void TASK_DATAST_PS2_Generic_KB_Output_Handle(void)
{
	U8_T length,datatype,datacode,modifier;

	/* 1.Check Task Ternimate Condition */
	if (DATAST_Generic_KB_IN_WP == DATAST_Generic_KB_IN_RP) // check if there is data needed processed
	{
		goto TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit2;
	}
#if (SYSTEM_EXTENDER_RECEIVER)
	if (KVM_CurrentHost == REMOTE_HOST_PORT)
	{
		goto GENERIC_KB_CHECK_SKIP;
	}
#endif			
	/*@@Check Port Suspend & Wakeup condition */
	if (USBDC_VHid_Resume_Check(KVM_CurrentHost,USBDC_VHID_DEVINX) == SET) 
	{
		goto TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit2;
	}
	
	if (USBDC_VHid_Valid_Check(KVM_CurrentHost,USBDC_VHID_DEVINX) == USB_ERROR)
	{
		DATAST_Generic_KB_IN_WP = DATAST_Generic_KB_IN_RP = 0; // check if there is data needed processed
		TASK_DATAST_PS2_Generic_KB_ActiveFlag  = 0;
		return;
	}
#if (SYSTEM_EXTENDER_RECEIVER)
	GENERIC_KB_CHECK_SKIP:
#endif
	
	// if buffer has been filled and wait for send to host
	if ((USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_KB_ENDP_NUM] & USBDC_UPORT[KVM_CurrentHost]) == 0) // if the buffer is not ready, just skip
	{
		goto TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit2;
	}

	/* 2.Process KB output */
	/* 2-1.Check Len & type first */
	DATAST_Generic_KB_Change_Flag = 0; // reset the key change status 
	length   = DATAST_Generic_KB_IN_Queue[DATAST_Generic_KB_IN_RP] & DATAST_PACKAGE_LENGTH_MASK;
	datatype = DATAST_Generic_KB_IN_Queue[DATAST_Generic_KB_IN_RP] & DATAST_PACKAGE_CODE_TYPE;

	//if is make key, and the queue still have space to move in
	if ((datatype == GENERIC_USAGE_07_MAKE) || (datatype == GENERIC_USAGE_07_BREAK))
	{
		if (datatype == GENERIC_USAGE_07_MAKE)
		{
			if (DATAST_Generic_USB_KB_Queue[7] != 0) // still have space
			{
				if ((USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_KB_ENDP_NUM] & USBDC_UPORT[KVM_CurrentHost]) == 0)
				{
					goto TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit2;
				}
			}
		}
		else // break key
		{
			// if buffer has been filled and wait for send to host
			if ((USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_KB_ENDP_NUM] & USBDC_UPORT[KVM_CurrentHost]) == 0)
			{
				goto TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit2;
			}	
		}
	}

	DATAST_Generic_KB_IN_RP++;
	if (DATAST_Generic_KB_IN_RP >= DATAST_GENERIC_KB_IN_MAX)
		DATAST_Generic_KB_IN_RP = 0;

	datacode = DATAST_Generic_KB_IN_Queue[DATAST_Generic_KB_IN_RP];
	DATAST_Generic_KB_IN_RP++;
	if (DATAST_Generic_KB_IN_RP >= DATAST_GENERIC_KB_IN_MAX)
		DATAST_Generic_KB_IN_RP = 0;

	if (datatype <= GENERIC_USAGE_07_BREAK)
	{
		if ((datacode >= 0xe0) && (datacode <= 0xe7))
		{
			modifier = 0x01 << (datacode-0xe0);
			if (datatype == GENERIC_USAGE_07_MAKE) // Modifiers Make
			{
				if ((DATAST_Generic_USB_KB_Queue[0] & modifier) == 0)
				{
					DATAST_Generic_USB_KB_Queue[0] |= modifier;
#if (SYSTEM_PS2_HOST_ENABLE)					
					cPS2_KB_PressCnt++;
#endif					
				}
			}
			else
			{                                   // Modifiers Break
				if (DATAST_Generic_USB_KB_Queue[0] & modifier)
				{
					DATAST_Generic_USB_KB_Queue[0] &= ~modifier;
#if (SYSTEM_PS2_HOST_ENABLE)					
					cPS2_KB_PressCnt--;
#endif					
				}
			}

			DATAST_Generic_KB_Change_Flag  = 1;
			goto TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit;
		}
	}

	switch (datatype)
	{
		case GENERIC_USAGE_07_MAKE: // this possible will make the key away.
			DATAST_Append_Generic_USBKB_Q(datacode);
			break;
		case GENERIC_USAGE_07_BREAK:
			DATAST_Remove_Generic_USBKB_Q(datacode);
			break;
		case GENERIC_USAGE_0C_MAKE: // this possible will make the key away.
		case GENERIC_USAGE_0C_BREAK:
			DATAST_Append_Generic_MS_IN_Q(datatype,(U8_T *)(&(PS2_USB_MultiMediaTab[datacode-0xb0])));
#if (SYSTEM_PS2_HOST_ENABLE)
			DATAST_PS2_MS_Handle(1);
#endif
			goto TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit2;
	}

	/* For Debug purpose */
TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit:
#if (GENERIC_USB_KB_DEBUG)
	if (DATAST_Generic_KB_Change_Flag) // content has changed
	{
		printf("KBQ=[ ");
		for (length=0; length < 8 ; length++)
		{
			printf("%02x ",(U16_T)DATAST_Generic_USB_KB_Queue[length]);
		}
		printf("]\n\r");
	}
#endif

	if (DATAST_Generic_KB_Change_Flag)
	{
		if (USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_KB_ENDP_NUM] & USBDC_UPORT[KVM_CurrentHost])
		{
			DATAST_Generic_KB_Change_Flag = 0;
			USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_KB_ENDP_NUM] &= ~USBDC_UPORT[KVM_CurrentHost];
#if (SYSTEM_EXTENDER_RECEIVER) && (PROJECT_USB_GENERIC_HID_ENABLE)
			if (KVM_CurrentHost != REMOTE_HOST_PORT)			
#endif				
			{
				USBDC_HAL_Endp_In_Buf_Move(DATAST_Generic_USB_KB_Queue,8,KVM_CurrentHost,USBDC_VHID_DEVINX,USBDC_VHID_KB_ENDP_NUM);
				USBDC_REGS_Endp_ControlSet(KVM_CurrentHost,USBDC_VHID_DEVINX,USBDC_VHID_KB_ENDP_NUM,DA_CR_BVLD_SET);
			}
		}
	}

TASK_DATAST_PS2_Generic_KB_Output_Handle_Exit2:

	if (DATAST_Generic_KB_IN_WP != DATAST_Generic_KB_IN_RP) //There is still needed to handle queue 
	{
		TASK_Wait_Current();
	}
	else
	{
		TASK_DATAST_PS2_Generic_KB_ActiveFlag = 0;
	}
}
#endif


/***************************************************************************************************/
/*
**  Function Name: TASK_DATAST_PS2_Generic_MS_Output_Handle(void)
**  Parameter    : 
**  Description  : 
**
**
*/
#ifdef SYNC
void TASK_DATAST_PS2_Generic_MS_Output_Handle(void)
{
#define		BOOT_MS_LEN		3	
	U8_T port;
	U8_T length=0,index;
	U8_T endpinx;
	
	port = TASK_Register0;
	//printf("p%bu",port);
	
	/* 1.Check Task Ternimate Condition */
#if (DATAST_GENERIC_MS_IN_QUEUE)
{
		if (DATAST_Generic_MS_IN_WP == DATAST_Generic_MS_IN_RP[port]) // check if there is data needed processed
		{
			//printf("e0");
			goto TASK_DATAST_PS2_Generic_MS_Output_Handle_Exit2;
		}
}
#else
{
	if (DATAST_Generic_MS_IN_WP[port] == 0)
	{
		goto TASK_DATAST_PS2_Generic_MS_Output_Handle_Exit2;
	}
	DATAST_Generic_MS_IN_RP[port] = 0;
}
#endif /* #if (DATAST_GENERIC_MS_IN_QUEUE) */
	/*@@Check Port Suspend & Wakeup condition */
	if (USBDC_VHid_Resume_Check(port,USBDC_VHID_DEVINX) == SET) 
	{
		//printf("e1");
		goto TASK_DATAST_PS2_Generic_MS_Output_Handle_Exit2;
	}
	
	if (USBDC_VHid_Valid_Check(port,USBDC_VHID_DEVINX) == USB_ERROR)
	{
		#if (DATAST_GENERIC_MS_IN_QUEUE)
		DATAST_Generic_MS_IN_RP[port] = DATAST_Generic_MS_IN_WP;
		#else
		DATAST_Generic_MS_IN_WP[port] = DATAST_Generic_MS_IN_RP[port] = 0; // check if there is data needed processed
		#endif
		TASK_DATAST_PS2_Generic_MS_ActiveFlag &= ~BIT_MASK[port];		
		goto TASK_DATAST_PS2_Generic_MS_Output_Handle_Exit2;
		//return;
	}

	/* 2.Process MS output */
	/* 2-1.Check Len & type first */
	endpinx = USBDC_VHID_MS_ENDP_NUM;
	switch(DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_RP[port]][0])
	{
		case 1: //normal mouse data
			length = 5;
#ifdef ONEHID		
#else		
			//Convert PS/2 Mouse format to USB
			if ((TASK_Event & 0x0f)== 0)
				DATAST_PS2_MS_Convert2_USB(&DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_RP[port]][1]);
#endif	/* #ifdef ONEHID */	
			break;
		case 2: //system keyboard keycode
			length = 2;
			break;
		case 3: //consumer keyboard keycode
			length = 3;
			break;
		case 4: //absolute mouse data
			length = 7;
			endpinx = USBDC_VHID_MS2_ENDP_NUM;
			break;
	}
	
	//Check Boot Protocol issue
	index = 0x01 << (USBDC_VHID_MS_ENDP_NUM-1);
	if (USBDC_Device[USBDC_VHID_DEVINX].Current_Protocol[port] & index)
	{
		if (DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_RP[port]][0] == 1)
		{
			for (index=0; index < BOOT_MS_LEN ; index++)
			{
				DATAST_Generic_USB_MS_Queue[index] = DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_RP[port]][index+1];
				DATAST_Generic_USB_MS_Report[index+1] = DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_RP[port]][index+1];
			}
			DATAST_Generic_USB_MS_Report[0] = BOOT_MS_LEN; // three byte in
			length = BOOT_MS_LEN; // out put 3 bytes
			DATAST_Generic_USB_MS_Queue[0] &= 0x07; // keep only 2 buttons
		}
		else
			goto TASK_DATAST_PS2_Generic_MS_Output_Handle_Exit2; //skip multi media keyboard scancode
	}
	else
	{
		for (index=0; index < length ; index++)
		{
			DATAST_Generic_USB_MS_Queue[index] = DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_RP[port]][index];
		}
	}

#if (DATAST_GENERIC_MS_IN_QUEUE)
	#ifdef SYNC
	DATAST_Generic_MS_IN_RP[port]++;
	if (DATAST_Generic_MS_IN_RP[port] >= DATAST_GENERIC_MS_IN_MAX)
		DATAST_Generic_MS_IN_RP[port] = 0;
	#else
	DATAST_Generic_MS_IN_RP++;
	if (DATAST_Generic_MS_IN_RP >= DATAST_GENERIC_MS_IN_MAX)
		DATAST_Generic_MS_IN_RP = 0;
	#endif
#else	
	DATAST_Generic_MS_IN_WP[port] = 0;
#endif /* #if (DATAST_GENERIC_MS_IN_QUEUE) */
	/* For Debug purpose */
#if (GENERIC_USB_MS_DEBUG)
	printf("MSQ=[");
	for (index=0; index < length ; index++)
	{
		if (index < (length-1))
			printf("%02x ",(U16_T)DATAST_Generic_USB_MS_Queue[index]);
		else
			printf("%02x",(U16_T)DATAST_Generic_USB_MS_Queue[index]);
	}
	printf("]\n\r");
#endif /* #if (GENERIC_USB_MS_DEBUG) */
	
//#if (DATAST_GENERIC_MS_IN_QUEUE)
//	if (USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_MS_ENDP_NUM] & USBDC_UPORT[port])
//#endif /* #if (DATAST_GENERIC_MS_IN_QUEUE) */
	{		
		//printf("G");
		USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_MS_ENDP_NUM] &= ~USBDC_UPORT[port];
		USBDC_HAL_Endp_In_Buf_Move(DATAST_Generic_USB_MS_Queue,length,port,USBDC_VHID_DEVINX,endpinx);
		USBDC_REGS_Endp_ControlSet(port,USBDC_VHID_DEVINX,endpinx,DA_CR_BVLD_SET);
	}

TASK_DATAST_PS2_Generic_MS_Output_Handle_Exit2:
	#if (DATAST_GENERIC_MS_IN_QUEUE)
	if (DATAST_Generic_MS_IN_WP != DATAST_Generic_MS_IN_RP[port]) //There is still needed to handle queue 
	#else
	if (DATAST_Generic_MS_IN_WP[port] != DATAST_Generic_MS_IN_RP[port]) //There is still needed to handle queue 
	#endif
	{
		//printf("E0");
		if (TASK_Type != TASK_TYPE_INTERVAL_MS)
		{
			TASK_Wait_Current(); //wait for next chance
		}
		//TASK_Wait_Current(); //wait for next chance
	}
	else
	{
		//printf("E1");
		TASK_DATAST_PS2_Generic_MS_ActiveFlag &= ~BIT_MASK[port];
		if (TASK_Type == TASK_TYPE_INTERVAL_MS)
		{
			TASK_Destory_Current();
		}
	}
}
#else
//For NONE SYNC Project
void TASK_DATAST_PS2_Generic_MS_Output_Handle(void)
{
	U8_T port;
	U8_T length=0,index;
	U8_T endpinx=USBDC_VHID_MS_ENDP_NUM;
	
	#define		BOOT_MS_LEN		3

	port = TASK_Register0;
	/* 1.Check Task Ternimate Condition */
#if (DATAST_GENERIC_MS_IN_QUEUE)
{
		if (DATAST_Generic_MS_IN_WP == DATAST_Generic_MS_IN_RP) // check if there is data needed processed
		{
			goto TASK_DATAST_PS2_Generic_MS_Output_Handle_Exit2;
		}
}
#else
{ // For NONE SYNC project
	if (DATAST_Generic_MS_IN_WP == 0)
	{
		goto TASK_DATAST_PS2_Generic_MS_Output_Handle_Exit2;
	}
	DATAST_Generic_MS_IN_RP = 0;
}
#endif /* #if (DATAST_GENERIC_MS_IN_QUEUE) */
	/*@@Check Port Suspend & Wakeup condition */
	if (USBDC_VHid_Resume_Check(KVM_CurrentHost,USBDC_VHID_DEVINX) == SET) 
	{
		goto TASK_DATAST_PS2_Generic_MS_Output_Handle_Exit2;
	}
	
	if (USBDC_VHid_Valid_Check(KVM_CurrentHost,USBDC_VHID_DEVINX) == USB_ERROR)
	{
		DATAST_Generic_MS_IN_WP = DATAST_Generic_MS_IN_RP = 0; // check if there is data needed processed
		TASK_DATAST_PS2_Generic_MS_ActiveFlag  = 0;
		return;
	}
	
	/* 2.Process MS output */
	/* 2-1.Check Len & type first */
	endpinx = USBDC_VHID_MS_ENDP_NUM;
	switch(DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_RP][0])
	{
		case 1:
			length = 5;
#ifdef ONEHID		
#else		
			//Convert PS/2 Mouse format to USB
			if (TASK_Register0 == 0)
				DATAST_PS2_MS_Convert2_USB(&DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_RP][1]);
#endif		
			break;
		case 2:
			length = 2;
			break;
		case 3:
			length = 3;
			break;
		case 4:
			length = 7;
			break;
	}
	
	//Check Boot Protocol issue
	index = 0x01 << (USBDC_VHID_MS_ENDP_NUM-1);
	if (USBDC_Device[USBDC_VHID_DEVINX].Current_Protocol[KVM_CurrentHost]  & index)
	{
		if (DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_RP][0] == 1)
		{
			for (index=0; index < BOOT_MS_LEN ; index++)
			{
				DATAST_Generic_USB_MS_Queue[index] = DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_RP][index+1];
				DATAST_Generic_USB_MS_Report[index+1] = DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_RP][index+1];
			}
			DATAST_Generic_USB_MS_Report[0] = BOOT_MS_LEN; // three byte in
			length = BOOT_MS_LEN; // out put 3 bytes
			DATAST_Generic_USB_MS_Queue[0] &= 0x07; // keep only 2 buttons
		}
		else
			goto TASK_DATAST_PS2_Generic_MS_Output_Handle_Exit2; //skip multi media keyboard scancode
	}
	else
	{
		for (index=0; index < length ; index++)
		{
			DATAST_Generic_USB_MS_Queue[index] = DATAST_Generic_MS_IN_Queue[DATAST_Generic_MS_IN_RP][index];
		}
	}

#if (DATAST_GENERIC_MS_IN_QUEUE)
	DATAST_Generic_MS_IN_RP++;
	if (DATAST_Generic_MS_IN_RP >= DATAST_GENERIC_MS_IN_MAX)
		DATAST_Generic_MS_IN_RP = 0;
#else
	DATAST_Generic_MS_IN_WP = 0;
#endif /* #if (DATAST_GENERIC_MS_IN_QUEUE) */
	/* For Debug purpose */
#if (GENERIC_USB_MS_DEBUG)
	printf("MSQ=[");
	for (index=0; index < length ; index++)
	{
		if (index < (length-1))
			printf("%02x ",(U16_T)DATAST_Generic_USB_MS_Queue[index]);
		else
			printf("%02x",(U16_T)DATAST_Generic_USB_MS_Queue[index]);
	}
	printf("]\n\r");
#endif /* #if (GENERIC_USB_MS_DEBUG) */

#if (DATAST_GENERIC_MS_IN_QUEUE)
	if (USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_MS_ENDP_NUM] & USBDC_UPORT[KVM_CurrentHost])
#endif /* #if (DATAST_GENERIC_MS_IN_QUEUE) */
	{
		//printf("[D%bu]",KVM_CurrentHost);	
		USBDC_HID_ENPx_Data_Wait_Flag[USBDC_VHID_MS_ENDP_NUM] &= ~USBDC_UPORT[KVM_CurrentHost];
		USBDC_HAL_Endp_In_Buf_Move(DATAST_Generic_USB_MS_Queue,length,KVM_CurrentHost,USBDC_VHID_DEVINX,endpinx);
		USBDC_REGS_Endp_ControlSet(KVM_CurrentHost,USBDC_VHID_DEVINX,endpinx,DA_CR_BVLD_SET);
	}

TASK_DATAST_PS2_Generic_MS_Output_Handle_Exit2:
	if (DATAST_Generic_MS_IN_WP != DATAST_Generic_MS_IN_RP) //There is still needed to handle queue 
	{
		TASK_Wait_Current(); //wait for next chance
	}
	else
	{
		TASK_DATAST_PS2_Generic_MS_ActiveFlag  = 0;
	}
}
#endif
/***************************************************************************************************/
/*
**  Function Name: DATAST_PS2_MS_Convert2_USB(UINT8 *cHost_MS_Buf)
**  Parameter    : 
**  Description  : 
**
**
*/     
void DATAST_PS2_MS_Convert2_USB(U8_T *ps2_msdata) 
{
	U8_T temp1;
	
	temp1 =  (ps2_msdata[3] & 0xf0 ) >> 1;
	ps2_msdata[0] = (ps2_msdata[0] & 0x07) | temp1;    // prepare the 4,5 buttons
	ps2_msdata[2] = ~ps2_msdata[2]+1 ;	                // 3rd Mouse Data
	ps2_msdata[3] = ~( ps2_msdata[3] & 0x0f)+1;        // 4th Mouse Data
	if ( ps2_msdata[3] <= 0xf7 )
	{
		ps2_msdata[3] &= 0x0f;
	}
}

/*-------------------------------------------------------------------------------------
 * U8_T DATA_Generic_Check_MSB(U8_T rptsize,U16_T value)
 * Purpose : Print out the UART message when memory alloc fault
 * Params  : $msgindex: the process id 
 * Returns : none
 * Note    : none
 *--------------------------------------------------------------------------------------*/
U8_T DATA_Generic_Check_MSB(U8_T rptsize,U16_T value)
{	
	if ((rptsize == 12) && (value & 0x0800))
	{	
		if ((value & 0x0080) == 0)
		{	
			//printf("[0-%04x]\n\r",value);
			return 1;
		}	
	}	
	if ((rptsize == 16) && (value & 0x8000))
	{		
		if ((value & 0x0080) == 0)
		{	
			//printf("[1-%04x]\n\r",value);		
			return 1;
		}	
	}	
	return 0;
}

/*-------------------------------------------------------------------------------------
 * void DATAST_Generic_Relative_Mouse_Data(U8_T devinx,U8_T inttid,U8_T *buf,U8_T len)
 * Purpose : Print out the UART message when memory alloc fault
 * Params  : $msgindex: the process id 
 * Returns : none
 * Note    : none
 *--------------------------------------------------------------------------------------
 */
U8_T DATAST_Generic_Mouse_Data(USB_MS_Data_TypeDef *ms_data,U8_T *buf_type,U8_T mouse_mode)
{
	U8_T    *mbuf;
	//U8_T	buf_type = GENERIC_USAGE_07_MAKE;
	U8_T    coordinate_data_flag=1;
		
	mbuf = DATAST_Generic_USB_MS_Report;
	*buf_type = GENERIC_USAGE_07_MAKE;	
	//1.Check System keycode
	if (Hid_Key_Parser_Flag & HID_KEY_SYSTEM_MASK)
	{			
		mbuf[0] = 0x02;				
		if (ms_data->b.wSystem.w) 	// System key
		{ // Make key			
			mbuf[1] = ms_data->b.wSystem.bbw.lsb;			
			mbuf[2] = 0x00;
			*buf_type = GENERIC_USAGE_0C_MAKE;
		}
		else
		{ // Break key			
			mbuf[1] = 0x00;
			mbuf[2] = 0x00;
			*buf_type = GENERIC_USAGE_0C_BREAK;
		}		
		Hid_Key_Parser_Flag &= ~HID_KEY_SYSTEM_MASK;		
		coordinate_data_flag=0;
		goto DATAST_Generic_Mouse_Data_Out;
	}	
	//2.Check Consumer keycode
	if (Hid_Key_Parser_Flag & HID_KEY_CONSUMER_MASK)
	{	
		//printf("C++:%04x",ms_data->b.wConsumer.w);
		mbuf[0] = 0x03;
		if (ms_data->b.wConsumer.w) 	// Consumer key
		{ // Make key						
			mbuf[1] = ms_data->b.wConsumer.bbw.lsb;			
			mbuf[2] = ms_data->b.wConsumer.bbw.msb;
			*buf_type = GENERIC_USAGE_0C_MAKE;			
		}	
		else
		{ // Break key			
			mbuf[1] = 0x00;
			mbuf[2] = 0x00;
			*buf_type = GENERIC_USAGE_0C_BREAK;				
		}			
		Hid_Key_Parser_Flag &= ~HID_KEY_CONSUMER_MASK;		
		coordinate_data_flag=0;
		goto DATAST_Generic_Mouse_Data_Out;
	}	
	//3.Check Mouse coordination data	
	mbuf[0] = ms_data->b.wBtn.bbw.lsb; 	// buttons
	//printf("[%02bx]",mbuf[0]);
	if (DATA_Generic_Check_MSB(ms_data->Xsize,ms_data->b.wX.w))	 
		mbuf[1] = 0x80;  	// X-lsb		
	else
		mbuf[1] = ms_data->b.wX.bbw.lsb;  	// X-lsb		
	
	//if (ms_data->b.wY.bbw.msb & 0x08)
	if (DATA_Generic_Check_MSB(ms_data->Ysize,ms_data->b.wY.w))			
		mbuf[2] = 0x80; 	// X-msb
	else
		mbuf[2] = ms_data->b.wY.bbw.lsb; 	// X-msb
	
	mbuf[3] = ms_data->b.wZ.bbw.lsb;   	// z		
	
	//if (mouse_mode == ABSOLUTE_MODE)	
	//if not in NC mode
	if ((KVM_Flash.cSystemFlag2 & SYSTEM_MS_DN_MASK) == 0x00)
	{	
		*buf_type = GENERIC_USAGE_ABS_MAKE;	
	}	

#ifdef KMLOG
	KMLog_Send_Mouse_Relative(mbuf);
#else	
	memcpy(Last_Relative_Coordinate,mbuf,4);
	if (API_User_Call_Back_Control & CALL_BACK_RELATIVE_MOUSE_MASK)
	{		
		API_Relative_Mouse_Data_Call_Back(Last_Relative_Coordinate,4);
	}				
#endif	
	
#ifdef SYNC			
	#ifdef SYNC_MS_HOTKEY
	KM_SYNC_MS_Hotkey_Check(mbuf[0]);
	#endif
	
	if (mouse_mode)
	{	
		KM_SYNC_Abs_Mouse_Coordinate_Convert(mbuf);
	}	
#endif
	
DATAST_Generic_Mouse_Data_Out:	
	mouse_mode = 0;
	//if (mouse_mode == 0)
	//	return 0;
		
	//DATAST_Append_Generic_MS_IN_Q(buf_type,mbuf); // normal mouse package	
//#if	(SYSTEM_EXTENDER_RECEIVER)
//#else	
//	DATAST_PS2_MS_Handle();
//#endif	/* #if	(SYSTEM_EXTENDER_RECEIVER) */	

	return 1;
}	 
			
#endif /* End of PROJECT_USB_GENERIC_HID_ENABLE */

/* End of data_stream_generic.c */
