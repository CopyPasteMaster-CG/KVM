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
 * Module Name : km_api.h
 * Purpose     : 
 * Author      : Jack Wang
 * Date        :
 * Notes       : 
 * no message
 *
 *================================================================================
 */
#ifndef __KM_API_H
#define __KM_API_H
/* INCLUDE FILE DECLARATIONS */
/* NAMING CONSTANT DECLARATIONS */
#define SINGLE_SCREEN_MODE  	0
#define QUAD_SCREEN_MODE  		1
#define TWO_SCREEN_MODE  		2	//daniel
#define ONE_3_SCREEN_MODE  		3	//zbb 2025/12/8
#define ONE_2_SCREEN_MODE  		4

#define CALL_BACK_ENABLE		1
#define CALL_BACK_DISABLE		0

#define CALL_BACK_RELATIVE_MOUSE_MASK	1
#define CALL_BACK_ABSOLUTE_MOUSE_MASK	2
#define CALL_BACK_KEYBOARD_MASK			4

#define	API_ABSOLUTE_MODE				0
#define	API_RELATIVE_MODE				1

#define	API_ROAMING_ENABLE				1
#define	API_ROAMING_DISABLE				0

#define	API_SYNC_ENABLE					1
#define	API_SYNC_DISABLE				0

#define	API_SWITCH_ENABLE				1
#define	API_SWITCH_DISABLE				0

#if !defined(CROSS_JUMP_MAP0) && !defined(CROSS_JUMP_MAP1)
#define CROSS_JUMP_MAP0
#endif

/* MACRO DECLARATIONS */
/* TYPE DECLARATIONS */
/* GLOBAL VARIABLES */ 
extern U8_T  API_User_Call_Back_Control;
extern U8_T  Last_Absolute_Coordinate[6];
extern U8_T  Last_Relative_Coordinate[4];
extern U8_T  Last_Keyboard_Data[4];

extern U8_T  API_Current_Main_SCREEN;

extern float API_One_3_Xf[2];
extern U16_T API_One_3_X[2];

extern float API_One_3_Yf[2];
extern U16_T API_One_3_Y[2];

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void API_Init(void);
void API_Keyboard_Data_Call_Back(U8_T *Kbbuf,U8_T Kbbuf_len); 
void API_Relative_Mouse_Data_Call_Back(U8_T *Msbuf,U8_T Msbuf_len); 
void API_Absolute_Mouse_Data_Call_Back(U8_T *Msbuf,U8_T Msbuf_len); 
void API_Set_Mouse_Mode(U8_T Mode);
void API_Set_Roaming_Mode(U8_T Mode);
void API_Set_Sync_Mode(U8_T Mode);
void API_Get_Relative_Mouse_Data(U8_T *Msbuf,U8_T *Msbuf_len);
void API_Get_Absolute_Mouse_Data(U8_T *Msbuf,U8_T *Msbuf_len);
void API_Get_Keyboard_Data(U8_T *Kbbuf,U8_T *Kbbuf_len);
void API_Set_Relative_Mouse_Data(U8_T *Msbuf,U8_T Msbuf_len);
void API_Set_Absolute_Mouse_Data(U8_T *Msbuf,U8_T Msbuf_len);
void API_Set_Keyboard_Data(U8_T *Kbbuf,U8_T Kbbuf_len);
void API_Port_Connect(U8_T Portmask,U8_T Connectmask);
void API_Port_Jump(U8_T Targetport);
void API_Active_Roaming_Mapping(U8_T Mapid);
#ifdef SYNC
void API_Set_Roaming_Mapping(U8_T Mapid,API_Roaming_Mapping_Def *Roaming);
void API_Send_Mouse_Absolute(void);
#endif /* #ifdef SYNC */
void API_Roaming_Mapping2_Handle(U8_T main_port);

void API_SET_ONE_3_SCREEN_MODE(U8_T main_port);
void API_SET_ONE_2_SCREEN_MODE(U8_T main_port);

#endif /* End of __KM_API_H */

