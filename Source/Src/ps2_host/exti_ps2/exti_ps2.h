 /*
 ******************************************************************************
 *     Copyright (c) 2010	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*=============================================================================
 * Module Name: exti_ps2_read.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __EXTI_PS2_H__
#define __EXTI_PS2_H__

/* INCLUDE FILE DECLARATIONS */
#include "system_cfg.h"
/* DEFINATION DECLARATIONS */

/** 
  * @brief  PS/2 Config : Max Number
  */   
   
#define EXTI_PS2_DEBUG_MODE       0  // PS/2 debug mode
  
#define EXTI_PS2_CONCURRENT       1  //1-Support 2 external port get data in same time
                                     //0-when one port active, the other one clock will be pull low
                                     
#define EXTI_PS2_RISIGN_EDGE_SUPPORT  0  //1-Support rising edge level detected
                                         //0-No support rising edge level detected

typedef enum
{ 
	PS2_Line_Low=0,
  PS2_Line_High=1,
	PS2_Clock_Low=0,
  PS2_Clock_High=1,
  PS2_Data_Low=0,
  PS2_Data_High=1
} PS2PinControl_TypeDef; 
        
/** 
  * @brief  PS2 Macro Function for Check Status
  */     
#define _EPS2A_TimeOut_Reset() ( PS2_TimeOut_Cnt[0]=0 )  
#define _EPS2B_TimeOut_Reset() ( PS2_TimeOut_Cnt[1]=0 ) 
#define _EPS2C_TimeOut_Reset() ( PS2_TimeOut_Cnt[2]=0 )  
#define _EPS2D_TimeOut_Reset() ( PS2_TimeOut_Cnt[3]=0 )  
#define _EPS2E_TimeOut_Reset() ( PS2_TimeOut_Cnt[4]=0 )  
#define _EPS2F_TimeOut_Reset() ( PS2_TimeOut_Cnt[5]=0 )    

#ifdef EPS2A_CLK_REGISTER
       #define _EPS2A_Clk_Bit() ( EPS2A_CLK_REGISTER )  
       #define _EPS2A_Clock_Low()  ( EPS2A_CLK_REGISTER=0 )  
			 #define _EPS2A_Clock_High() ( EPS2A_CLK_REGISTER=1 )  
#endif

#ifdef EPS2A_DATA_REGISTER
			 #define _EPS2A_Data_Bit() ( EPS2A_DATA_REGISTER )
			 #define _EPS2A_Data_Low()  ( EPS2A_DATA_REGISTER=0 )  
			 #define _EPS2A_Data_High() ( EPS2A_DATA_REGISTER=1 )    
#endif

#ifdef EPS2B_CLK_REGISTER
       #define _EPS2B_Clk_Bit() ( EPS2B_CLK_REGISTER )  
       #define _EPS2B_Clock_Low()  ( EPS2B_CLK_REGISTER=0 )  
			 #define _EPS2B_Clock_High() ( EPS2B_CLK_REGISTER=1 )    
#endif

#ifdef EPS2B_DATA_REGISTER
			 #define _EPS2B_Data_Bit() ( EPS2B_DATA_REGISTER )
			 #define _EPS2B_Data_Low()  ( EPS2B_DATA_REGISTER=0 )  
			 #define _EPS2B_Data_High() ( EPS2B_DATA_REGISTER=1 )      
#endif

#ifdef EPS2C_CLK_REGISTER
       #define _EPS2C_Clk_Bit() ( EPS2C_CLK_REGISTER )  
       #define _EPS2C_Clock_Low()  ( EPS2C_CLK_REGISTER=0 )  
			 #define _EPS2C_Clock_High() ( EPS2C_CLK_REGISTER=1 )  
#endif

#ifdef EPS2C_DATA_REGISTER
			 #define _EPS2C_Data_Bit() ( EPS2C_DATA_REGISTER )  
			 #define _EPS2C_Data_Low()  ( EPS2C_DATA_REGISTER=0 )  
			 #define _EPS2C_Data_High() ( EPS2C_DATA_REGISTER=1 )    
#endif

#ifdef EPS2D_CLK_REGISTER
       #define _EPS2D_Clk_Bit() ( EPS2D_CLK_REGISTER )  
       #define _EPS2D_Clock_Low()  ( EPS2D_CLK_REGISTER=0 )  
			 #define _EPS2D_Clock_High() ( EPS2D_CLK_REGISTER=1 )  
#endif

#ifdef EPS2D_DATA_REGISTER
			 #define _EPS2D_Data_Bit() ( EPS2D_DATA_REGISTER )  
			 #define _EPS2D_Data_Low()  ( EPS2D_DATA_REGISTER=0 )  
			 #define _EPS2D_Data_High() ( EPS2D_DATA_REGISTER=1 )    
#endif

#ifdef EPS2E_CLK_REGISTER
       #define _EPS2E_Clk_Bit() ( EPS2E_CLK_REGISTER )  
       #define _EPS2E_Clock_Low()  ( EPS2E_CLK_REGISTER=0 )  
			 #define _EPS2E_Clock_High() ( EPS2E_CLK_REGISTER=1 )  
#endif

#ifdef EPS2E_DATA_REGISTER
			 #define _EPS2E_Data_Bit() ( EPS2E_DATA_REGISTER )  
			 #define _EPS2E_Data_Low()  ( EPS2E_DATA_REGISTER=0 )  
			 #define _EPS2E_Data_High() ( EPS2E_DATA_REGISTER=1 )    
#endif

#ifdef EPS2F_CLK_REGISTER
       #define _EPS2F_Clk_Bit() ( EPS2F_CLK_REGISTER )  
       #define _EPS2F_Clock_Low()  ( EPS2A_CLK_REGISTER=0 )  
			 #define _EPS2F_Clock_High() ( EPS2A_CLK_REGISTER=1 )  
#endif

#ifdef EPS2F_DATA_REGISTER
			 #define _EPS2F_Data_Bit() ( EPS2F_DATA_REGISTER )  
			 #define _EPS2F_Data_Low()  ( EPS2F_DATA_REGISTER=0 )  
			 #define _EPS2F_Data_High() ( EPS2F_DATA_REGISTER=1 )    
#endif
 
/* NAMING CONSTANT DECLARATIONS */
 
/* GLOBAL VARIABLES */ 
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
 
void EXTI_PS2_Init(void);
void EXTI_PS2_DeInit(PS2Port_TypeDef);
void EXTI_PS2_Disable(PS2Port_TypeDef);
void EXTI_PS2_Pin_Control(PS2Port_TypeDef,PS2PinControl_TypeDef,PS2PinControl_TypeDef);
void EXTI_PS2_Enable(PS2Port_TypeDef);
extern void DELAY_Us(U16_T);
#endif /* End of __EXTI_H__ */

/* End of exti.h */