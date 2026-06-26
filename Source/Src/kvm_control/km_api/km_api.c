/*
 ******************************************************************************
 *     Copyright (c) 2014	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is an proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
 /*============================================================================
 * Module Name: km_sync.c
 * Purpose:
 * Author:
 * Date:
 *=============================================================================
 */
/* 
*/

/* INCLUDE FILE SECTION */
#include <stdio.h>
#include <string.h>
#include "project_include.h"

#ifdef API
/* NAMING CONSTANT DECLARATIONS 			*/
/* GLOBAL VARIABLES DECLARATIONS 			*/
U8_T  API_User_Call_Back_Control = 0;
U8_T  Last_Absolute_Coordinate[6];
U8_T  Last_Relative_Coordinate[4];
U8_T  Last_Keyboard_Data[4];
U8_T  API_Current_Main_SCREEN;
float API_One_3_Xf[2];
U16_T API_One_3_X[2];

float API_One_3_Yf[2];
U16_T API_One_3_Y[2];

void 	API_Roaming_1_3_Call(void);
void 	API_Roaming_1_2_Call(void);



API_Roaming_Mapping_Def	API_Roaming_One_2[4] = 
{
		//Item 0: Main Port 0 不使用，占位置	
				0,        
		
			0,  0,	0,		
	
		0,1, //Port 0 Left	
		0,1, //Port 0 Right
		0,1, //Port 0 Up	
		0,1, //Port 0 Down		
		API_Roaming_1_2_Call, //call back function
		//=============================	
		3,1, //Port 1 Left	
		1,1, //Port 1 Right			
		2,1, //Port 1 Up	
		1,1, //Port 1 Down		
		API_Roaming_1_2_Call, //call back function
		//=============================		
		3,1, //Port 2 Left	
		2,1, //Port 2 Right			
		2,1, //Port 2 Up	
		1,1, //Port 2 Down
		API_Roaming_1_2_Call, //call back function
		//=============================					
		3,1, //Port 3 Left	
		2,1, //Port 3 Right
		3,1, //Port 3 Up	
		3,1, //Port 3 Down
		API_Roaming_1_2_Call, //call back function		
	
	
	//--------------------------------
	//Item 0: Main Port 1
	//--------------------------------	 
	//Left_up,Left_Down,Right_Up,Right_Down		
			3,        
		
			2,  1,	0,		
	
		0,1, //Port 0 Left	
		0,1, //Port 0 Right
		0,1, //Port 0 Up	
		0,1, //Port 0 Down		
		API_Roaming_1_2_Call, //call back function
		//=============================	
		3,1, //Port 1 Left	
		1,1, //Port 1 Right			
		2,1, //Port 1 Up	
		1,1, //Port 1 Down		
		API_Roaming_1_2_Call, //call back function
		//=============================		
		3,1, //Port 2 Left	
		2,1, //Port 2 Right			
		2,1, //Port 2 Up	
		1,1, //Port 2 Down
		API_Roaming_1_2_Call, //call back function
		//=============================					
		3,1, //Port 3 Left	
		2,1, //Port 3 Right
		3,1, //Port 3 Up	
		3,1, //Port 3 Down
		API_Roaming_1_2_Call, //call back function		
	//--------------------------------
	//Item 1: Main Port 2
	//--------------------------------	 
	//Left_up,Left_Down,Right_Up,Right_Down		
			2,        
		1,	3,	0,		
	
		0,1, //Port 0 Left	
		0,1, //Port 0 Right
		0,1, //Port 0 Up	
		0,1, //Port 0 Down		
		API_Roaming_1_2_Call, //call back function
		//=============================	
		2,1, //Port 1 Left	
		1,1, //Port 1 Right			
		1,1, //Port 1 Up	
		3,1, //Port 1 Down		
		API_Roaming_1_2_Call, //call back function
		//=============================		
		2,1, //Port 2 Left	
		1,1, //Port 2 Right			
		2,1, //Port 2 Up	
		2,1, //Port 2 Down
		API_Roaming_1_2_Call, //call back function
		//=============================					
		2,1, //Port 3 Left	
		3,1, //Port 3 Right
		1,1, //Port 3 Up	
		3,1, //Port 3 Down
		API_Roaming_1_2_Call, //call back function		
	//--------------------------------
	//Item 2: Main Port 3
	//--------------------------------	 
	//Left_up,Left_Down,Right_Up,Right_Down		
				 1,        
		3,       2,     	  0,		
	
		0,1, //Port 0 Left	
		0,1, //Port 0 Right
		0,1, //Port 0 Up	
		0,1, //Port 0 Down		
		API_Roaming_1_2_Call, //call back function
		//=============================	
		1,1, //Port 1 Left	
		3,1, //Port 1 Right			
		1,1, //Port 1 Up	
		1,1, //Port 1 Down		
		API_Roaming_1_2_Call, //call back function
		//=============================		
		1,1, //Port 2 Left	
		2,1, //Port 2 Right			
		3,1, //Port 2 Up	
		2,1, //Port 2 Down
		API_Roaming_1_2_Call, //call back function
		//=============================					
		1,1, //Port 3 Left	
		3,1, //Port 3 Right
		3,1, //Port 3 Up	
		2,1, //Port 3 Down
		API_Roaming_1_2_Call, //call back function		
};




API_Roaming_Mapping_Def	API_Roaming_One_3[4] = //include 3 screen mode
{
	//--------------------------------
	//Item 0: Main Port 0
	//--------------------------------	 
	//Left_up,Left_Down,Right_Up,Right_Down		
			3,        
		2,  1,	0,		
	
		3,1, //Port 0 Left	
		0,1, //Port 0 Right
		1,1, //Port 0 Up	
		0,1, //Port 0 Down		
		API_Roaming_1_3_Call, //call back function
		//=============================	
		3,1, //Port 1 Left	
		1,1, //Port 1 Right			
		2,1, //Port 1 Up	
		0,1, //Port 1 Down		
		API_Roaming_1_3_Call, //call back function
		//=============================		
		3,1, //Port 2 Left	
		2,1, //Port 2 Right			
		2,1, //Port 2 Up	
		1,1, //Port 2 Down
		API_Roaming_1_3_Call, //call back function
		//=============================					
		3,1, //Port 3 Left	
		2,1, //Port 3 Right
		3,1, //Port 3 Up	
		3,1, //Port 3 Down
		API_Roaming_1_3_Call, //call back function		
	//--------------------------------
	//Item 1: Main Port 1
	//--------------------------------	 
	//Left_up,Left_Down,Right_Up,Right_Down		
			2,        
		1,	0,	3,		
	
		2,1, //Port 0 Left	
		0,1, //Port 0 Right
		1,1, //Port 0 Up	
		3,1, //Port 0 Down		
		API_Roaming_1_3_Call, //call back function
		//=============================	
		2,1, //Port 1 Left	
		1,1, //Port 1 Right			
		1,1, //Port 1 Up	
		0,1, //Port 1 Down		
		API_Roaming_1_3_Call, //call back function
		//=============================		
		2,1, //Port 2 Left	
		1,1, //Port 2 Right			
		2,1, //Port 2 Up	
		2,1, //Port 2 Down
		API_Roaming_1_3_Call, //call back function
		//=============================					
		2,1, //Port 3 Left	
		3,1, //Port 3 Right
		0,1, //Port 3 Up	
		3,1, //Port 3 Down
		API_Roaming_1_3_Call, //call back function		
	//--------------------------------
	//Item 2: Main Port 2
	//--------------------------------	 
	//Left_up,Left_Down,Right_Up,Right_Down		
				 1,        
		0,       3,     	  2,		
	
		1,1, //Port 0 Left	
		0,1, //Port 0 Right
		0,1, //Port 0 Up	
		3,1, //Port 0 Down		
		API_Roaming_1_3_Call, //call back function
		//=============================	
		1,1, //Port 1 Left	
		0,1, //Port 1 Right			
		1,1, //Port 1 Up	
		1,1, //Port 1 Down		
		API_Roaming_1_3_Call, //call back function
		//=============================		
		1,1, //Port 2 Left	
		2,1, //Port 2 Right			
		3,1, //Port 2 Up	
		2,1, //Port 2 Down
		API_Roaming_1_3_Call, //call back function
		//=============================					
		1,1, //Port 3 Left	
		3,1, //Port 3 Right
		0,1, //Port 3 Up	
		2,1, //Port 3 Down
		API_Roaming_1_3_Call, //call back function		
		
	//--------------------------------
	//Item 3: Main Port 3
	//--------------------------------	 
	//Left_up,Left_Down,Right_Up,Right_Down		
			0,        
		3,	2,	1,		
	
		0,1, //Port 0 Left	
		3,1, //Port 0 Right
		0,1, //Port 0 Up	
		0,1, //Port 0 Down		
		API_Roaming_1_3_Call, //call back function
		//=============================	
		0,1, //Port 1 Left	
		1,1, //Port 1 Right			
		2,1, //Port 1 Up	
		1,1, //Port 1 Down		
		API_Roaming_1_3_Call, //call back function
		//=============================		
		0,1, //Port 2 Left	
		2,1, //Port 2 Right			
		3,1, //Port 2 Up	
		1,1, //Port 2 Down
		API_Roaming_1_3_Call, //call back function
		//=============================					
		0,1, //Port 3 Left	
		3,1, //Port 3 Right
		3,1, //Port 3 Up	
		2,1, //Port 3 Down
		API_Roaming_1_3_Call, //call back function			
};
//-----------------------------------------------
// Single screen
//	0 <-> 1 <-> 2 <-> 3 
//-----------------------------------------------
void 	API_Roaming_Single_Call(void);
//-----------------------------------------------
// Four screen
//	0 <-> 2 
//  |  X  | 
//  1 <-> 3 
//-----------------------------------------------
void 	API_Roaming_Quad_Call(void);
#ifdef SYNC
API_Roaming_Mapping_Def	API_Roaming[5] = //include 3 screen mode	//daniel
{	
#ifdef TOP_BOTTOM
	//Left_up,Left_Down,Right_Up,Right_Down		
			0,
			1,
			2,
			3,
	//--------------------------------
	//Item 0:Single Screen
	//--------------------------------			 
	0,1, //Port 0 Left	
	0,1, //Port 0 Right			
	3,1, //Port 0 Up	
	1,1, //Port 0 Down	
	API_Roaming_Single_Call, //call back function
	//=============================
	1,1, //Port 1 Left	
	1,1, //Port 1 Right			
	0,1, //Port 1 Up	
	2,1, //Port 1 Down	

	API_Roaming_Single_Call, //call back function
	//=============================
	2,1, //Port 2 Left	
	2,1, //Port 2 Right			
	2,1, //Port 2 Up	
	3,1, //Port 2 Down		
	API_Roaming_Single_Call, //call back function
	//=============================		
	3,1, //Port 3 Left	
	3,1, //Port 3 Right			
	2,1, //Port 3 Up	
	0,1, //Port 3 Down			
	API_Roaming_Single_Call, //call back function	 		
#else
	//Left_up,Left_Down,Right_Up,Right_Down		
			0,        1,       2,     	  3,
	//--------------------------------
	//Item 0:Single Screen
	//--------------------------------			 
	3,1, //Port 0 Left	
	1,1, //Port 0 Right			
	0,1, //Port 0 Up	
	0,1, //Port 0 Down	
	API_Roaming_Single_Call, //call back function
	//=============================
	0,1, //Port 1 Left	
	2,1, //Port 1 Right			
	1,1, //Port 1 Up	
	1,1, //Port 1 Down	

	API_Roaming_Single_Call, //call back function
	//=============================
	1,1, //Port 2 Left	
	3,1, //Port 2 Right			
	2,1, //Port 2 Up	
	2,1, //Port 2 Down		
	API_Roaming_Single_Call, //call back function
	//=============================		
	2,1, //Port 3 Left	
	0,1, //Port 3 Right			
	3,1, //Port 3 Up	
	3,1, //Port 3 Down			
	API_Roaming_Single_Call, //call back function	 		
#endif /* #ifdef TOP_BOTTOM */
	//--------------------------------
	//Item 1:Four Equal Screen
	//--------------------------------	
#ifdef CROSS_JUMP_MAP0	
	//Left_up,Left_Down,Right_Up,Right_Down		
			3,        1,       2,     	  0,
	//	3 <-> 2 
	//  |  X  | 
	//  1 <-> 0 

		
				// Port 0
		1,1,   // Left -> 1
		0,1,   // Right -> 0
		2,1,   // Up -> 2
		0,1,   // Down -> 0
		API_Roaming_Quad_Call,

		// Port 1
		1,1,
		0,1,
		3,1,
		1,1,
		API_Roaming_Quad_Call,

		// Port 2
		3,1,
		2,1,
		2,1,
		0,1,
		API_Roaming_Quad_Call,

		// Port 3
		3,1,
		2,1,
		3,1,
		1,1,
		API_Roaming_Quad_Call,
			
#endif /* #ifdef CROSS_JUMP_MAP0 */	

#ifdef CROSS_JUMP_MAP1	
	//Left_up,Left_Down,Right_Up,Right_Down		
			0,        2,       1,     	  3,
	//	0 <-> 1 
	//  |  X  | 
	//  2 <-> 3 

		1,1, //Port 0 Left	
		1,1, //Port 0 Right
		2,1, //Port 0 Up	
		2,1, //Port 0 Down		
		API_Roaming_Quad_Call, //call back function
		//=============================	
		0,1, //Port 1 Left	
		0,1, //Port 1 Right			
		3,1, //Port 1 Up	
		3,1, //Port 1 Down		
		API_Roaming_Quad_Call, //call back function
		//=============================		
		3,1, //Port 2 Left	
		3,1, //Port 2 Right			
		0,1, //Port 2 Up	
		0,1, //Port 2 Down
		API_Roaming_Quad_Call, //call back function
		//=============================					
		2,1, //Port 3 Left	
		2,1, //Port 3 Right
		1,1, //Port 3 Up	
		1,1, //Port 3 Down
		API_Roaming_Quad_Call, //call back function		 	
#endif		
		//双屏鼠标滑动设置
		//Left_up,Left_Down,Right_Up,Right_Down		
			3,        1,       2,     	  0,
	//	3 <-> 2 
	//  |  X  | 
	//  1 <-> 0 

				// Port 0
		1,1,   // Left -> 1
		0,1,   // Right -> 0
		0,1,   // Up -> 2
		0,1,   // Down -> 0
		API_Roaming_Quad_Call,

		// Port 1
		1,1,
		0,1,
		1,1,
		1,1,
		API_Roaming_Quad_Call,

		// Port 2
		3,1,
		2,1,
		2,1,
		2,1,
		API_Roaming_Quad_Call,

		// Port 3
		3,1,
		2,1,
		3,1,
		3,1,
		API_Roaming_Quad_Call,

#if 0
	//--------------------------------
	//Item 2:1 big , 3 small
	//--------------------------------	 
	//Left_up,Left_Down,Right_Up,Right_Down		
				 0,        
		1,       2,     	  3,		
	//		  0 
	//  |  X  |  X  |
	//  1 <-> 2 <-> 3  

		0,1, //Port 0 Left	
		0,1, //Port 0 Right
		1,1, //Port 0 Up	
		1,1, //Port 0 Down		
		API_Roaming_Quad_Call, //call back function
		//=============================	
		3,1, //Port 1 Left	
		2,1, //Port 1 Right			
		0,1, //Port 1 Up	
		0,1, //Port 1 Down		
		API_Roaming_Quad_Call, //call back function
		//=============================		
		1,1, //Port 2 Left	
		3,1, //Port 2 Right			
		0,1, //Port 2 Up	
		0,1, //Port 2 Down
		API_Roaming_Quad_Call, //call back function
		//=============================					
		2,1, //Port 3 Left	
		1,1, //Port 3 Right
		0,1, //Port 3 Up	
		0,1, //Port 3 Down
		API_Roaming_Quad_Call, //call back function		 
#endif
};
#endif /* #ifdef SYNC */
//#endif
/* LOCAL VARIABLES DECLARATIONS 			*/
/* LOCAL SUBPROGRAM DECLARATIONS 			*/
static void API_Debug_Print_Roaming_Map(U8_T main_port, API_Roaming_Mapping_Def *map);
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS 	*/
/* EXTERNAL SUBPROGRAM DECLARATIONS 		*/
void API_Set_Mouse_Mode(U8_T Mode);
void API_Set_Roaming_Mode(U8_T Mode);
void API_Set_Sync_Mode(U8_T Mode);
#ifdef SYNC
void API_Set_Roaming_Mapping(U8_T Mapid,API_Roaming_Mapping_Def *Romaing_map);
#endif /* #ifdef SYNC */
void API_Get_Relative_Mouse_Data(U8_T *Msbuf,U8_T *Msbuf_len);
void API_Get_Absolute_Mouse_Data(U8_T *Msbuf,U8_T *Msbuf_len);
void API_Get_Keyboard_Data(U8_T *Kbbuf,U8_T *Kbbuf_len);
void API_Set_Relative_Mouse_Data(U8_T *Msbuf,U8_T Msbuf_len);
void API_Set_Absolute_Mouse_Data(U8_T *Msbuf,U8_T Msbuf_len);
void API_Set_Keyboard_Data(U8_T *Kbbuf,U8_T Kbbuf_len);
void API_Port_Connect(U8_T Portmask,U8_T Connectmask);
void API_Port_Jump(U8_T Targetport);
void API_Call_Back_Control(U8_T Mask,U8_T Mode);
void API_Keyboard_Data_Call_Back(U8_T *Kbbuf,U8_T Kbbuf_len); 
void API_Relative_Mouse_Data_Call_Back(U8_T *Msbuf,U8_T Msbuf_len); 
void API_Absolute_Mouse_Data_Call_Back(U8_T *Msbuf,U8_T Msbuf_len); 

/** ----------------------------------------------------------------------------
 * Function Name: API_Init(void)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void API_Init(void)
{	
	//Init call back funciton pointer
	API_One_3_Yf[0] = VHID_Y_MAX/3;
	API_One_3_Yf[1] = (VHID_Y_MAX / 3) * 2;
	
	API_One_3_Y[0] = (U16_T)API_One_3_Yf[0]; 
	API_One_3_Y[1] = (U16_T)API_One_3_Yf[1]; 
	
	//1.Assing SINGLE_SCREEN_MODE roaming control table
#ifdef SYNC
	API_Set_Roaming_Mapping(SINGLE_SCREEN_MODE,&API_Roaming[0]);
	
	//2.Assing QUAD_SCREEN_MODE roaming control table
	API_Set_Roaming_Mapping(QUAD_SCREEN_MODE  ,&API_Roaming[1]);
	
	//3.Assing ONE_3_SCREEN_MODE roaming control table
	API_Set_Roaming_Mapping(ONE_3_SCREEN_MODE  ,&API_Roaming[3]); //zbb
	API_Set_Roaming_Mapping(TWO_SCREEN_MODE  ,&API_Roaming[2]); //daniel

	API_Set_Roaming_Mapping(ONE_2_SCREEN_MODE  ,&API_Roaming[4]);
	
	//3.Default active Quad screen mode	
	API_Active_Roaming_Mapping(QUAD_SCREEN_MODE);
	
#endif /* #ifdef SYNC */
	//4.Set call back function disable
	//API_Call_Back_Control(CALL_BACK_RELATIVE_MOUSE_MASK|CALL_BACK_ABSOLUTE_MOUSE_MASK|CALL_BACK_KEYBOARD_MASK,CALL_BACK_DISABLE);
	//Enbable keyboard, mouse relative data report to UART1
	API_Call_Back_Control(CALL_BACK_RELATIVE_MOUSE_MASK|CALL_BACK_KEYBOARD_MASK,CALL_BACK_DISABLE);
	
#ifdef SYNC
	//5.Set mouse absolute coordinate mode
	API_Set_Mouse_Mode(API_ABSOLUTE_MODE);
	
	//6.Set mouse roaming mode
	API_Set_Roaming_Mode(API_ROAMING_ENABLE);
	
	//7.Set mouse sync mode
	API_Set_Sync_Mode(API_SYNC_ENABLE);
#else
	//5.Set mouse absolute coordinate mode
	API_Set_Mouse_Mode(API_RELATIVE_MODE);
	
	//6.Set mouse roaming mode
	API_Set_Roaming_Mode(API_ROAMING_DISABLE);
	
	//7.Set mouse sync mode
	API_Set_Sync_Mode(API_SYNC_DISABLE);
#endif /* #ifdef SYNC */
}

static void API_Debug_Print_Roaming_Map(U8_T main_port, API_Roaming_Mapping_Def *map)
{
//	U8_T i;
//	
////	printf("CG:API_Roaming[%bu]\n\r", main_port);
//	printf("CG:Sequence=%bu,%bu,%bu,%bu\n\r",
//	       map->Sequence[0],
//	       map->Sequence[1],
//	       map->Sequence[2],
//	       map->Sequence[3]);
//	
////	for (i = 0; i < KVM_MAX_PORT; i++)
////	{
////		printf("CG:Port%bu L=%bu/%bu R=%bu/%bu U=%bu/%bu D=%bu/%bu\n\r",
////		       i,
////		       map->Edge.Item[i].Left.Portid,
////		       map->Edge.Item[i].Left.Latency,
////		       map->Edge.Item[i].Right.Portid,
////		       map->Edge.Item[i].Right.Latency,
////		       map->Edge.Item[i].Up.Portid,
////		       map->Edge.Item[i].Up.Latency,
////		       map->Edge.Item[i].Down.Portid,
////		       map->Edge.Item[i].Down.Latency);
////	}
}

/** ----------------------------------------------------------------------------
 * Function Name:API_Call_Back_Control(U8_T Mask,U8_T Mode)
 * Purpose:
 * Params:
 * 	Mask : CALL_BACK_RELATIVE_MOUSE_MASK,CALL_BACK_ABSOLUTE_MOUSE_MASK,CALL_BACK_KEYBOARD_MASK
 *  Mode : CALL_BACK_ENABLE,CALL_BACK_DISABLE
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void API_Call_Back_Control(U8_T Mask,U8_T Mode)
{
	if (Mode == CALL_BACK_ENABLE)
		API_User_Call_Back_Control |= Mask;
	else
		API_User_Call_Back_Control &= ~Mask;	
}

/** ----------------------------------------------------------------------------
 * Function Name: API_Set_Mouse_Mode(U8_T Mode)
 * Purpose:
 * Params:
 *  Mode : API_ABSOLUTE_MODE,API_RELATIVE_MODE
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void API_Set_Mouse_Mode(U8_T Mode)
{
	#ifdef SYNC
	if (Mode == API_ABSOLUTE_MODE)
	{
		KM_SYNC_Mouse_Absolute_Mode();
	}		
	else
	{
		KM_SYNC_Mouse_Relative_Mode();
	}			
	#else
	if (Mode == API_ABSOLUTE_MODE)
	{
		KVM_Flash.cSystemFlag2 &= ~SYSTEM_MS_DN_MASK;
	}		
	else
	{
		KVM_Flash.cSystemFlag2 |= SYSTEM_MS_DN_MASK;
	}
	#endif	
}	

/** ----------------------------------------------------------------------------
 * Function Name: API_Set_Roaming_Mode(U8_T Mode)
 * Purpose:
 * Params:
 *  Mode : API_ROAMING_ENABLE,API_ROAMING_DISALBE
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void API_Set_Roaming_Mode(U8_T Mode)
{	
	#ifdef SYNC
		KM_SYNC_ModeAccross_Control(Mode);
	#else
		if (Mode == API_ROAMING_ENABLE)
			KVM_Flash.cSystemFlag3 |= (MPASS_ACROSS_MODE | SYSTEM_MOUSE_SW_MASK);	
		else
			KVM_Flash.cSystemFlag3 &= ~(MPASS_ACROSS_MODE | SYSTEM_MOUSE_SW_MASK);			
	#endif
}	

#ifdef SYNC
/** ----------------------------------------------------------------------------
 * Function Name: API_Set_Sync_Mode(U8_T Mode)
 * Purpose:
 * Params:
 *  Mode : API_SYNC_ENABLE,API_SYNC_DISALBE
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void API_Set_Sync_Mode(U8_T Mode)
{
	KM_SYNC_ModeSync_Control(Mode);
}	

/** ------------------------------------------------------------------------------------------------------
 * Function Name: API_Set_Roaming_Mapping(U8_T Mapid,API_Roaming_Mapping_Def *Roaming)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ------------------------------------------------------------------------------------------------------*/
void API_Set_Roaming_Mapping(U8_T Mapid,API_Roaming_Mapping_Def *Roaming_Table)
{
	if (Mapid < API_ROAMING_TABLE_MAX)
	{	
		Roaming[Mapid]=Roaming_Table;
	}	
}	

/** ------------------------------------------------------------------------------------------------------
 * Function Name: API_Active_Roaming_Mapping(U8_T Mapid)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ------------------------------------------------------------------------------------------------------*/
void API_Active_Roaming_Mapping(U8_T Mapid)
{
	if (Mapid < API_ROAMING_TABLE_MAX)
	{	
		if (Roaming[Mapid] != NULL)
		{	
			Current_Roaming_Table = Mapid;
		}
	}	
}

/**----------------------------------------------------------------------------
 * void API_Send_Mouse_Absolute(void)
 * Purpose : 
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------*/ 
void API_Send_Mouse_Absolute(void)
{
	Last_Absolute_Coordinate[0] = Mouse_Data.b.wBtn.bbw.lsb; //button
	Last_Absolute_Coordinate[1] = (MouseX & 0xff00) >> 8;  	//X high byte
	Last_Absolute_Coordinate[2] = MouseX & 0x00ff;  		//X-high byte
	Last_Absolute_Coordinate[3] = (MouseY & 0xff00) >> 8;	//Y-high byte		
	Last_Absolute_Coordinate[4] = MouseY & 0x00ff;	 		//Y-low byte		
	Last_Absolute_Coordinate[5] = Mouse_Data.b.wZ.bbw.lsb;		 //Z-low byte		

	if (API_User_Call_Back_Control & CALL_BACK_ABSOLUTE_MOUSE_MASK)
	{		
		API_Absolute_Mouse_Data_Call_Back(Last_Absolute_Coordinate,6);
	}		
	/*	
	Km_Data.bHeader 	= KMLOG_HEADER0;
	Km_Data.bDataType 	= KMLOG_TYPE_MS;		
	Km_Data.bPayload[0] = msdata[0];
	Km_Data.bPayload[1] = msdata[1];
	Km_Data.bPayload[2] = msdata[2];
	Km_Data.bPayload[3] = msdata[3];	
	Km_Data.bPayload[4] = 0x00;
	Km_Data.bPayload[5] = 0x00;
	REMOTE_PutStr((U8_T *)&Km_Data,sizeof(Kmlog_Data_Packet_Def));
	*/
}

#endif /* #ifdef SYNC */
/** ---------------------------------------------------------------------------- 
 * Function Name: API_Get_Relative_Mouse_Data(U8_T *Msbuf,U8_T *Msbuf_len)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void API_Get_Relative_Mouse_Data(U8_T *Msbuf,U8_T *Msbuf_len)
{
	memcpy(Msbuf,Last_Relative_Coordinate,4);
	*Msbuf_len = 4;
}	

/** ---------------------------------------------------------------------------- 
 * Function Name: API_Get_Absolute_Mouse_Data(U8_T *Msbuf,U8_T *Msbuf_len)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void API_Get_Absolute_Mouse_Data(U8_T *Msbuf,U8_T *Msbuf_len)
{
	memcpy(Msbuf,Last_Absolute_Coordinate,6);
	*Msbuf_len = 6;
}

/** ---------------------------------------------------------------------------- 
 * Function Name: API_Get_Keyboard_Data(U8_T *Kbbuf,U8_T *Kbbuf_len)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void API_Get_Keyboard_Data(U8_T *Kbbuf,U8_T *Kbbuf_len)
{
	if (Last_Keyboard_Data[1] == 0x0c)
		*Kbbuf_len = 4;
	else
		*Kbbuf_len = 3;
	memcpy(Kbbuf,Last_Keyboard_Data,*Kbbuf_len);
	
}

/** ---------------------------------------------------------------------------- 
 * Function Name: API_Set_Relative_Mouse_Data(U8_T *Msbuf,U8_T Msbuf_len)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void API_Set_Relative_Mouse_Data(U8_T *Msbuf,U8_T Msbuf_len)
{	
	Msbuf_len = 4;
	DATAST_Generic_USB_MS_Report[0]	= Msbuf[0]; //button
	DATAST_Generic_USB_MS_Report[1] = Msbuf[1]; //X
	DATAST_Generic_USB_MS_Report[2] = Msbuf[2]; //Y
 	DATAST_Generic_USB_MS_Report[3] = Msbuf[3]; //Z	
	USBHC_InterruptTransfer_HID_Mouse(DATAST_Generic_USB_MS_Report,GENERIC_USAGE_07_MAKE);		
}

/** ---------------------------------------------------------------------------- 
 * Function Name: API_Set_Absolute_Mouse_Data(U8_T *Msbuf,U8_T Msbuf_len)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
#ifdef SYNC
void API_Set_Absolute_Mouse_Data(U8_T *Msbuf,U8_T Msbuf_len)
{
	Msbuf_len = 6;
	MouseX = (Msbuf[1] << 8) + Msbuf[2];
	MouseY = (Msbuf[3] << 8) + Msbuf[4];	
	//MouseZ = Km_Data.bPayload[5];	
		//Km_Data.bHeader 	= KMLOG_HEADER0;
	//Km_Data.bDataType 	= KMLOG_TYPE_MSA;	
	//DATAST_Generic_USB_MS_Report[0] = 0x04; //Absolute
	DATAST_Generic_USB_MS_Report[0]	= Msbuf[0]; //button
	DATAST_Generic_USB_MS_Report[1] = Msbuf[5]; //Z
	DATAST_Generic_USB_MS_Report[2] = Msbuf[2]; //X low
	DATAST_Generic_USB_MS_Report[3] = Msbuf[1]; //X high
 	DATAST_Generic_USB_MS_Report[4] = Msbuf[4]; //Y low
	DATAST_Generic_USB_MS_Report[5] = Msbuf[3]; //Y high	
	USBHC_InterruptTransfer_HID_Mouse(DATAST_Generic_USB_MS_Report,GENERIC_USAGE_ABS_MAKE);
}	
#endif
/** ---------------------------------------------------------------------------- 
 * Function Name: API_Set_Keyboard_Data(U8_T *Kbbuf,U8_T Kbbuf_len)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void API_Set_Keyboard_Data(U8_T *Kbbuf,U8_T Kbbuf_len)
{
	U8_T  key_type;
	
	Kbbuf_len = 3;
	//printf("[Make=%02bx,keytype=%02bx,code=%02bx]",Receive_Data->bPayload[0],Receive_Data->bPayload[1],Receive_Data->bPayload[2]);
	switch(Kbbuf[1])
	{
		case 0x07: //normal keyboard data
			key_type = HID_USAGE_PAGE_07_MAKE + (Kbbuf[0] & 0x01);		
			//KM_SYNC_Send_VHID_Key(key_type,Kbbuf[2]);
#ifdef SYNC
			KM_SYNC_Send_VHID_Key(key_type,Kbbuf[2]);
#else
			DATAST_Keyboard_Send(KVM_CurrentHost,key_type,Kbbuf[2]);
#endif
			break;
		case 0x01: //system keyboard data
		case 0x0c: //consumer keyboard data	
			key_type = GENERIC_USAGE_0C_MAKE + (Kbbuf[0] & 0x01);
			if (Kbbuf[1] == 0x01)
				DATAST_Generic_USB_MS_Report[0] = 0x01; //system			 
			else
				DATAST_Generic_USB_MS_Report[0] = 0x04; //system			 
			DATAST_Generic_USB_MS_Report[2]	= Kbbuf[2];
			DATAST_Generic_USB_MS_Report[1]	= Kbbuf[3];		
			USBHC_InterruptTransfer_HID_Mouse(DATAST_Generic_USB_MS_Report,GENERIC_USAGE_07_MAKE);			
			break;		
	}	
}

/** ---------------------------------------------------------------------------- 
 * Function Name: API_Port_Connect(U8_T Portmask,U8_T Connectmask)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void API_Port_Connect(U8_T Portmask,U8_T Connectmask)
{
	/*
	Receive_Data->bPayload[1] = Portmask;
	Receive_Data->bPayload[2] = Connectmask;
	KMLog_Handle_Host_Connect_Set();
	*/
	//just skip the warning message for compile, need to mark when user want to
	//use this call back
	if (Portmask)
	{
		Connectmask = 0x0f;
	}
}

/** ---------------------------------------------------------------------------- 
 * Function Name: API_Port_Jump(U8_T Targetport)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void API_Port_Jump(U8_T Targetport)
{
	KVM_Port_Jump(Targetport);	
}

/** ---------------------------------------------------------------------------- 
 * Function Name: API_Keyboard_Data_Call_Back(U8_T *Kbbuf,U8_T *Kbbuf_len)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void API_Keyboard_Data_Call_Back(U8_T *Kbbuf,U8_T Kbbuf_len)
{	
	//just skip the warning message for compile, need to mark when user want to
	//use this call back
	if (Kbbuf)
	{
		//output to UART1
		Kbbuf_len = 4;		
#ifdef KMLOG				
		Km_Data.bHeader 	= KMLOG_HEADER0;
		Km_Data.bDataType 	= KMLOG_TYPE_KB;		
		Km_Data.bPayload[0] = Kbbuf[0];
		Km_Data.bPayload[1] = Kbbuf[1];
		Km_Data.bPayload[2] = Kbbuf[2];
		Km_Data.bPayload[3] = Kbbuf[3];
		Km_Data.bPayload[4] = 0x00;
		Km_Data.bPayload[5] = 0x00;
		REMOTE_PutStr((U8_T *)&Km_Data,sizeof(Kmlog_Data_Packet_Def));	
#endif		
	}			
}	

/** ---------------------------------------------------------------------------- 
 * Function Name: API_Relative_Mouse_Data_Call_Back(U8_T *Msbuf,U8_T *Msbuf_len)
 * Purpose:
 * Params: 	Msbuf[0]: Mouse button value
 *			Msbuf[1]: X coordinate value(-127~+127)
 *			Msbuf[2]: Y coordinate value(-127~+127)
 *			Msbuf[3]: Z coordinate value(-127~+127)
 * 			Msbuf_len : should be always 4
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void API_Relative_Mouse_Data_Call_Back(U8_T *Msbuf,U8_T Msbuf_len)
{
	//just skip the warning message for compile, need to mark when user want to
	//use this call back
	if (Msbuf)
	{
		//output to UART1
		Msbuf_len = 4;	
#ifdef KMLOG						
		Km_Data.bHeader 	= KMLOG_HEADER0;
		Km_Data.bDataType 	= KMLOG_TYPE_MS;		
		Km_Data.bPayload[0] = Msbuf[0];
		Km_Data.bPayload[1] = Msbuf[1];
		Km_Data.bPayload[2] = Msbuf[2];
		Km_Data.bPayload[3] = Msbuf[3];	
		Km_Data.bPayload[4] = 0x00;
		Km_Data.bPayload[5] = 0x00;
		REMOTE_PutStr((U8_T *)&Km_Data,sizeof(Kmlog_Data_Packet_Def));	
#endif		
	}	
}	

/** ---------------------------------------------------------------------------- 
 * Function Name: API_Absolute_Mouse_Data_Call_Back(U8_T *Msbuf,U8_T *Msbuf_len)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void API_Absolute_Mouse_Data_Call_Back(U8_T *Msbuf,U8_T Msbuf_len)
{
	//just skip the warning message for compile, need to mark when user want to
	//use this call back
	if (Msbuf)
	{
		Msbuf_len = 6;
	}
}	

/** ---------------------------------------------------------------------------- 
 * Function Name: API_Roaming_Single_Call(void)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
void API_Roaming_Single_Call(void)
{	
//-----------------------------------------------
// Single screen call back function
//	0 <-> 1 <-> 2 <-> 3 
//-----------------------------------------------		
}
/** ---------------------------------------------------------------------------- 
 * Function Name: API_Roaming_Quad_Call(void)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------*/
//-----------------------------------------------
// Quad screen call back function
//	0 <-> 1 
//  |  X  | 
//  2 <-> 3 
//-----------------------------------------------
void API_Roaming_Quad_Call(void)
{
}

//-----------------------------------------------
// void API_Roaming_1_3_Call(void)
//	   0 
//  |  X  | 
//  2 <-> 3 
//-----------------------------------------------
void API_Roaming_1_3_Call(void)
{
	U16_T one_quarter;
	U16_T half;
	U16_T three_quarter;
	U8_T i;
	
	one_quarter = (Y_MAX >> 2);
	half = (Y_MAX >> 1);
	three_quarter = ((Y_MAX * 3) >> 2);
	
	if ((KVM_CurrentHost == API_Current_Main_SCREEN) &&
		(Current_Roaming_Direction == ROAMING_RIGHT))
	{
		if (MouseY < one_quarter)
		{
			MouseY = half + (MouseY * 2);
		}
		else if (MouseY < three_quarter)
		{
			MouseY = (MouseY - one_quarter) * 2;
		}
		else
		{
			MouseY = (MouseY - three_quarter) * 2;
		}
		
		MouseX = X_LEFT_EDGE;
		if (MouseY > Y_MAX)
		{
			MouseY = Y_MAX;
		}
	}
	else if ((KVM_CurrentHost != API_Current_Main_SCREEN) &&
			 (Current_Roaming_Direction == ROAMING_LEFT))
	{
		for (i = 1; i < 4; i++)
		{
			if (Roaming[Current_Roaming_Table]->Sequence[i] == KVM_CurrentHost)
			{
				if (i == 1)
				{
					if (MouseY < half)
					{
						MouseY = 0;
					}
					else
					{
						MouseY = (MouseY - half) / 2;
					}
				}
				else if (i == 2)
				{
					MouseY = one_quarter + (MouseY / 2);
				}
				else
				{
					if (MouseY > half)
					{
						MouseY = Y_MAX;
					}
					else
					{
						MouseY = three_quarter + (MouseY / 2);
					}
				}
				
				MouseX = X_RIGHT_EDGE;
				if (MouseY > Y_MAX)
				{
					MouseY = Y_MAX;
				}
				break;
			}
		}
	}
}
void API_Roaming_1_2_Call(void)
{
	U16_T section_start;
	U8_T i;
	
	if ((KVM_CurrentHost == API_Current_Main_SCREEN) &&
		(Current_Roaming_Direction == ROAMING_RIGHT))
	{
		if (MouseY < (Y_MAX >> 1))
		{
			section_start = 0;
		}
		else
		{
			section_start = (Y_MAX >> 1);
		}
		
		MouseX = X_LEFT_EDGE;
		MouseY = (MouseY - section_start) * 2;
		if (MouseY > Y_MAX)
		{
			MouseY = Y_MAX;
		}
	}
	else if ((KVM_CurrentHost != API_Current_Main_SCREEN) &&
			 (Current_Roaming_Direction == ROAMING_LEFT))
	{
		for (i = 1; i < 3; i++)
		{
			if (Roaming[Current_Roaming_Table]->Sequence[i] == KVM_CurrentHost)
			{
				if (i == 1)
				{
					section_start = 0;
				}
				else
				{
					section_start = (Y_MAX >> 1);
				}
				
				MouseX = X_RIGHT_EDGE;
				MouseY = section_start + (MouseY / 2);
				if (MouseY > Y_MAX)
				{
					MouseY = Y_MAX;
				}
				break;
			}
		}
	}
}
/** ------------------------------------------------------------------------------------------------------
 * Function Name: API_Roaming_Mapping2_Handle(U8_T Mapid,API_Roaming_Mapping_Def *Roaming)
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ------------------------------------------------------------------------------------------------------*/
void API_Roaming_Mapping2_Handle(U8_T main_port)
{	
	// API_Current_Map2_Id = main_port;
	// printf("ONE_THREE Screen Roaming Mode:%bu\n\r",main_port);
	// API_Set_Roaming_Mode(API_ROAMING_ENABLE);			
	// API_Active_Roaming_Mapping(ONE_3_SCREEN_MODE);		 //zbb

	// memcpy(&API_Roaming[2],&API_Roaming[main_port],sizeof(API_Roaming_Mapping_Def));	
}	
#endif /* #ifdef API */



void API_SET_ONE_3_SCREEN_MODE(U8_T main_port)
{	
	if (main_port >= KVM_MAX_PORT)
	{
		return;
	}

	API_Debug_Print_Roaming_Map(main_port, &API_Roaming_One_3[main_port]);

	memcpy(&API_Roaming[3],
	       &API_Roaming_One_3[main_port],
	       sizeof(API_Roaming_Mapping_Def));
	printf("one_3\r\n");
	API_Set_Sync_Mode(API_SYNC_DISABLE);
	API_Set_Roaming_Mode(API_ROAMING_ENABLE);			
	API_Active_Roaming_Mapping(ONE_3_SCREEN_MODE);

}

void API_SET_ONE_2_SCREEN_MODE(U8_T main_port)
{

	if ((main_port < 1) || (main_port > 3))
	{
		printf("main_port %d",main_port);
		return;
	}
	API_Debug_Print_Roaming_Map(main_port, &API_Roaming_One_2[main_port]);
	printf("one_2\r\n");
	
	memcpy(&API_Roaming[4],
	       &API_Roaming_One_2[main_port],
	       sizeof(API_Roaming_Mapping_Def));
		   
	API_Set_Sync_Mode(API_SYNC_DISABLE);
	API_Set_Roaming_Mode(API_ROAMING_ENABLE);
	API_Active_Roaming_Mapping(ONE_2_SCREEN_MODE);
}

/* End of km_api.c */
