/*
 *********************************************************************************
 *     Copyright (c) 2015	ASIX Electronic Corporation    All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : ISL59920.c
 * Purpose     : This module handles the ISL59920 Chip(RGB delay IC)
 *
 * Author      : 
 * Date        :
 * Notes       : 
 *
 *================================================================================
 */
//
//  HEADER FILE 
//
#include	"stdio.h"
#include	"string.h"

/* System Relate Header File Seciton */
#include "project_include.h"

/* NAMING CONSTANT DECLARATIONS */ 
#if (EXTENDER_ISL59920_SUPPORT)
#ifndef TRUE  
#define TRUE 	1
#endif

#ifndef FALSE 
#define FALSE	0
#endif

#ifndef LOW
#define LOW		0
#endif

#ifndef HIGH
#define HIGH	1
#endif

#define ISL_SCL	 		P0_2        
#define ISL_SDA	 		P0_1   
#define ISL_SEnablePin  P0_3   

/* STATIC GLOABLE VARIABLE DECLARATIONS */
bit  bOperateFlag;
U8_T cOperateColor;
U8_T cOperateCtrl;
U8_T cOperateValue;

/* LOCAL SUBPROGRAM DECLARATIONS */
/* EXTERNAL SUBPROGRAM DECLARATIONS */
/* LOCAL SUBPROGRAM BODIES */
void ISL59920_Bit_Delay(U8_T delay)
{
	while (delay)
	{
		delay--;
	}
}
/******************************************************************************
**  Function Name:  WriteISL59920
**  Parameter    :  NONE
**  return       :  NONE
**  Description  :  ³]©w ISL59920 
**  NOTICE       :   
*/ 
void ISL59920_Write_Data(U8_T color,U8_T delay) 
{
 	 U8_T  i;	
 		 		 		     		  	  
	 //printf("Color=%02bx,Delaye=%02bx\n\r",color,delay);
		 
	 color  = (color+1) << 5;  
	 color |= delay;		
		 
	//1.enable the Serial Enable Pin
	ISL_SEnablePin = LOW;		
	
	//2.output the Data	
    //---------------------------------------------------------------
	for (i=8 ;i != 0; i--) 
	{
		ISL_SCL = HIGH;
		if (color & 0x80)
		{
			ISL_SDA = HIGH;		     
		}			     
		else
		{
			ISL_SDA = LOW;		     
		}	
		
		ISL59920_Bit_Delay(10); // delay 10 us
		ISL_SCL = LOW;  // data lock by ISL59920
		ISL59920_Bit_Delay(10); // delay 10 us
		color = color << 1;        
	}
     
	//3.close the serial 
	//ISL59920_Bit_Delay(10); // delay 10 us
	ISL_SDA        = HIGH;
	ISL_SCL        = LOW;
	ISL_SEnablePin = HIGH;	         
}     

/******************************************************************************
**  Function Name:  ISL59920_init
**  Parameter    :  NONE
**  return       :  NONE
**  Description  :  
**  NOTICE       :   
*/ 
void ISL59920_Init(void) 
{	
	ISL_SDA        = HIGH;
	ISL_SCL        = LOW;
	ISL_SEnablePin = HIGH;	   	 		
	
	bOperateFlag  = 0;
	cOperateColor = 0;
	cOperateCtrl  = 0;
	cOperateValue = 0;
	
	printf("59920_Init[%02bx,%02bx,%02bx]\n\r",KVM_Flash.Color_Delay[0],KVM_Flash.Color_Delay[1],KVM_Flash.Color_Delay[2]);
	ISL59920_Write_Data(R_COLOR,KVM_Flash.Color_Delay[0]); 
	ISL59920_Write_Data(G_COLOR,KVM_Flash.Color_Delay[1]); 
	ISL59920_Write_Data(B_COLOR,KVM_Flash.Color_Delay[2]); 
}

/******************************************************************************
**  Function Name:  ISL59920_Color_Delay_Setting
**  Parameter    :  NONE
**  return       :  NONE
**  Description  :  
**  NOTICE       :   
*/ 
void ISL59920_Color_Delay_Setting(U8_T color,U8_T operation)
{	
	if (color == R_COLOR)
		printf("R");
	else if (color == G_COLOR)
		printf("G");
	else if (color == B_COLOR)
		printf("B");
	
	if(operation == OPERATION_PLUS)
	   printf("_+\n\r");		
	else
	   printf("_-\n\r");		
	   
	bOperateFlag  = 1;
	cOperateColor = color;
	cOperateCtrl  = operation;
	cOperateValue = KVM_Flash.Color_Delay[cOperateColor]; 
	ISL59920_Adjust(cOperateColor,cOperateCtrl,1,&cOperateValue);
	HOTKEY_Control_Status |= KVM_HOTKEY_ACTIVE;
	TASK_KVM_LED_Indicate_ActiveID = TASK_Active(TASK_TYPE_INTERVAL_MS,TASK_HOTKEY_LED_INDICATE_ID,0,0,HOTKEY_LED_INDICATE_TIME,HOTKEY_LED_INDICATE_TIME);
} 

/******************************************************************************
**  Function Name:  ISL59920_Adjust
**  Parameter    :  NONE
**  return       :  NONE
**  Description  : 
**  NOTICE       :   
*/ 
void ISL59920_Adjust(U8_T color,U8_T operation,U8_T delay,U8_T *current) 
{
 	 /* Check the valid value */ 	 
 	printf("Before:WRITE:Color:%bu,Delay=%02bx,Current=%02bx\n\r",color,delay,*current);
	if (operation == OPERATION_PLUS)
	{	
		if ((delay+(*current)) >= COLOR_MAX_SEG)
		{			
			*current = COLOR_MAX_SEG;
#if (KVM_BUZZER_SUPPORT)
			BUZZER_Script_Active(Error_Sound);
#endif //#if (KVM_BUZZER_SUPPORT)			
		}
		else
		{
			*current += delay;
		}	
	}	
	else
	{
		if (delay > *current)
		{
			*current = COLOR_MIN_SEG;
#if (KVM_BUZZER_SUPPORT)
			BUZZER_Script_Active(Error_Sound);
#endif //#if (KVM_BUZZER_SUPPORT)						
		}	
		else
		{
			*current -= delay;
		}	
	}		
		
	printf("DelayColor:%bu,Delay=%02bx\n\r",color,*current);	
	ISL59920_Write_Data(color,*current); 	
}

/******************************************************************************
**  Function Name:  ISL59920_Write
**  Parameter    :  NONE
**  return       :  NONE
**  Description  : 
**  NOTICE       :   
*/ 
void ISL59920_Hotkey_Control(U8_T keycode) 
{
 	/* Check the valid value */ 	 
 	//printf("Before:WRITE:Color:%bu,Delay=%02bx,Current=%02bx\n\r",color,delay,KVM_Flash.Color_Delay[color]);
 	switch (keycode)
 	{
 		case CHAR_R_ARROW:
 		case CHAR_PAD_6_RIGHT: //adjust the direction
 			if (cOperateCtrl == OPERATION_MINUS) //change direction
	 		{
	 			cOperateCtrl = OPERATION_PLUS;
	 		}	
			ISL59920_Adjust(cOperateColor,cOperateCtrl,1,&cOperateValue);	 		
 			break;
 		case CHAR_L_ARROW:
 		case CHAR_PAD_4_LEFT: //adjust the direction	
 			if (cOperateCtrl == OPERATION_PLUS) //change direction
	 		{
	 			cOperateCtrl = OPERATION_MINUS;
	 		}	
			ISL59920_Adjust(cOperateColor,cOperateCtrl,1,&cOperateValue);	 		
 			break;
 		case CHAR_ESCAPE: //give the adjust
 			printf("Delay Give Up\n\r");
 			HOTKEY_Deactive();	// reset the hotkey state	
 			bOperateFlag = 0;
 			ISL59920_Write_Data(cOperateColor,KVM_Flash.Color_Delay[cOperateColor]);	 		
 			break;
 		case CHAR_RETURN: 
 		case CHAR_PAD_ENTER: 
 			bOperateFlag = 0; 			
 			HOTKEY_Deactive();	// reset the hotkey state	
#if (KVM_BUZZER_SUPPORT)
			BUZZER_Script_Active(PortSwitch_Sound);
#endif //#if (KVM_BUZZER_SUPPORT)			
			KVM_Flash.Color_Delay[cOperateColor] = cOperateValue;	
			printf("Store Write:Color:%bu,Delay=%02bx\n\r",cOperateColor,KVM_Flash.Color_Delay[cOperateColor]);	
#if (SYSTEM_STORAGE_DEVICE_SUPPORT)
			STORAGE_Write(sizeof(KVM_Flash),(U8_T *)&KVM_Flash); 
#endif /* #if (SYSTEM_STORAGE_DEVICE_SUPPORT) */
 			break;		
 	} 	
}   
#endif  /* #if (EXTENDER_ISL59920_SUPPORT) */
