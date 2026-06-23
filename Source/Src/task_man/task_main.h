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
/*=============================================================================
 * Module Name: task_main.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */
#ifndef __TASK_MAIN_H__
#define __TASK_MAIN_H__

/* INCLUDE FILE DECLARATIONS */
/* DEFINATION DECLARATIONS */

#define TASK_ACTIVE_MAX				128
#define TASK_SEARCH_MAX				(TASK_ACTIVE_MAX >> 3)
#define TASK_TABLE_MS_PERIOD_MAX	64
#ifdef NO_USB_EVENT
	#define TASK_TABLE_EVENT_MAX		96
	#define TASK_TABLE_USB_MAX			2
#else
	#define TASK_TABLE_EVENT_MAX		64
	#define TASK_TABLE_USB_MAX			64
#endif
#define TASK_TABLE_MAX				128   // 128 Task ID

/** 
  * @brief  Task_Control Structure definition  
  */
#define TASK_WAIT_ACTIVED			0x80

#define TASK_TYPE_MASK				0x07
#define TASK_TYPE_INTERVAL_MS		0x00
#define TASK_TYPE_EVENT				0x03
#define TASK_TYPE_USB				0x04
 
typedef struct
{
	void (* Task_FunctionP)(void); 	// Task Function Pointer
} TASK_TypeDef;

#define TASK_EVENT_MASK				0xf0
#define TASK_EVENT_IDLE				0x00
#define TASK_EVENT_ACTIVE			0x10
#define TASK_EVENT_DESTORY			0x20
#define TASK_EVENT_SKIP				0x40
#define TASK_EVENT_WAIT				0x80
#define TASK_EVENT_SERIAL_MASK		0x0f
/*----------------------------------------------------------------------------------------------*/
#if 0
typedef struct 
{
	U8_T 	Task_ID;				// Task ID from 0~255
	U8_T 	Task_Event;				// Task Event
	U8_T 	Task_Para;				// Task transfer parameter
	U8_T 	Task_Para2;				// Task transfer parameter2
} TASK_EventQueue_TypeDef;

typedef struct
{
	U8_T Task_ID;			// Task ID from 0~255
	U8_T Task_Para;			// Task transfer parameter
	U8_T Task_Event;		// Task Event
							// bit 0~3 => Control Status
							//     0000(0x00)-> IDLE
							//     0001(0x01)-> Next loop still Active( not load the reload value into interval)
							//     0010(0x02)-> After excete, then destory itself.
	U16_T Task_Interval;	// Task Interval time from 0~65535(ms/us)
	U16_T Task_Reload;		// Task Interval Reload
} TASK_PeriodicalQueue_TypeDef;
/*----------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------*/
typedef struct
{
	U8_T Task_Type;			// Task type
	U8_T Task_ID;			// Task ID from 0~255
	U8_T Task_Para;			// Task transfer parameter	
	U16_T Task_Interval;	// Task Interval time from 0~65535(ms/us)
	U16_T Task_Reload;		// Task Interval Reload
} TASK_IsrPendigTypeDef;
#endif
/*----------------------------------------------------------------------------------------------*/

typedef struct
{
	U8_T Task_Type;				// Task Type ID
	U8_T Task_ID;				// Task ID from 0~255	
	U8_T taskCtrl;				// Task internal control
	U8_T Task_Event;			// Task Event
								// bit 0~3 => Control Status
								//     0000(0x00)-> IDLE
								//     0001(0x01)-> Next loop still Active( not load the reload value into interval)
								//     0010(0x02)-> After excete, then destory itself.
	U8_T Task_Para;  			// Task transfer parameter
	u16_t_u8_t Task_Interval;	// Task Interval time from 0~65535(ms/us)
	u16_t_u8_t Task_Reload;		// Task Interval Reload
} TASK_ActiveTable_TypeDef;

/* NAMING CONSTANT DECLARATIONS */
/* GLOBAL VARIABLES */

/* $$ EXPORTED SUBPROGRAM SPECIFICATIONS $$ */
void TASK_Init(void);
U8_T TASK_Kill(U8_T,U8_T);
void TASK_Period_MS_Maintain(void);
void TASK_MS_Period_Main(void);
void TASK_Event_Main(void);
void TASK_USB_Main(void);
U8_T TASK_Active(U8_T,U8_T,U8_T,U8_T,U16_T,U16_T);
U8_T TASK_Create(void *);
void TASK_Table_Dump(U8_T);
U8_T TASK_Run(TASK_ActiveTable_TypeDef *task);
/* $$ EXPORTED GLOBAL VARIABLES $$ */
extern bit Task_MS_Period_Flag;
extern U8_T TASK_Type;
extern U8_T  Task_MS_Period_Table[TASK_TABLE_MS_PERIOD_MAX];
extern U8_T  Task_Event_Table[TASK_TABLE_EVENT_MAX];
extern U8_T  Task_USB_Table[TASK_TABLE_USB_MAX];
extern U8_T  TASK_EVENT_PS2_HANDLE_ID;
extern U8_T  TASK_Active_ID;
extern TASK_ActiveTable_TypeDef  	Task_Active_Table[TASK_ACTIVE_MAX];

extern idata volatile U8_T Task_MS_Period_Table_RP, Task_MS_Period_Table_WP;
extern idata volatile U8_T Task_USB_Table_RP, Task_USB_Table_WP;
extern idata volatile U8_T Task_Event_Table_RP, Task_Event_Table_WP, Task_Event_Counter;
extern U8_T  TASK_Register0,TASK_Register1,TASK_Register2,TASK_Register3,TASK_Register4,TASK_Event;

void TASK_Destory_Current(void);
void TASK_Destory_Task(U8_T taskId);
void TASK_Wait_Current(void);
U8_T TASK_Active_Wait(U8_T task_type,U8_T task_id,U8_T task_event,U8_T task_para,U16_T task_interval,U16_T task_reload,U16_T wait_time);
#endif /* End of __TASK_MAIN__ */

/* End of task_main.h */