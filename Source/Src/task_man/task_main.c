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
 * Module Name: task_main.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */
/*
 
*/
/* INCLUDE FILE SECTION */
/* INCLUDE FILE DECLARATIONS */
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	"project_include.h"

/* NAMING CONSTANT DECLARATIONS */
#define TaskControlEnable 0

/* GLOBAL VARIABLES DECLARATIONS */
bit  Task_MS_Period_Flag;
U8_T Task_Table_Max;
U8_T TASK_Type=0,TASK_Event;
U8_T  TASK_Register0,TASK_Register1,TASK_Register2,TASK_Register3,TASK_Register4,TASK_Active_ID;
U8_T  Task_Current_Active_ID;
// Possible ISR Used ------------------------------------------------------------------------

idata volatile U8_T Task_MS_Period_Table_RP, Task_MS_Period_Table_WP;
idata volatile U8_T Task_Event_Table_RP, Task_Event_Table_WP;
idata volatile U8_T Task_USB_Table_RP, Task_USB_Table_WP;
idata volatile U8_T Task_USB_Counter, Task_Event_Counter;
volatile U8_T Task_Search[TASK_SEARCH_MAX];


TASK_TypeDef Task_Table[TASK_TABLE_MAX];
U8_T  Task_MS_Period_Table[TASK_TABLE_MS_PERIOD_MAX];
U8_T  Task_Event_Table[TASK_TABLE_EVENT_MAX];
U8_T  Task_USB_Table[TASK_TABLE_USB_MAX];

TASK_ActiveTable_TypeDef  	Task_Active_Table[TASK_ACTIVE_MAX];

/* LOCAL VARIABLES DECLARATIONS */
/* LOCAL SUBPROGRAM DECLARATIONS */
U8_T TASK_Malloc(U8_T *freeid); 
void TASK_Free(U8_T freeid); 
/* EXTERNAL GLOBAL VARIABLES DECLARATIONS */
/* EXTERNAL SUBPROGRAM DECLARATIONS */

/*
 * ----------------------------------------------------------------------------
 * Function Name: Task_Init
 * Purpose: initial the task manager system  
 * Params:  NONE
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------
 */
void TASK_Init(void)
{
	Task_Table_Max = 0;
	Task_MS_Period_Table_RP = 0;
	Task_MS_Period_Table_WP = 0;
	Task_Event_Table_RP = 0;
	Task_Event_Table_WP = 0;
	Task_USB_Table_RP = 0;
	Task_USB_Table_WP = 0;
	Task_MS_Period_Flag = 0;
	Task_Event_Counter = 0;
	Task_USB_Counter = 0;  
	memset(Task_Search,0x00,sizeof(Task_Search));
}

/*
 * ----------------------------------------------------------------------------
 * void TASK_Kill_MS_Period(U8_T taskid)
 * Purpose: internal used in task system control, this function will
 *          remove the task, and adjust the table
 * Params:  taskid: the active task id need to be free
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------
 */
void TASK_Kill_MS_Period(U8_T taskid)
{
	U8_T remain,wp;

	TASK_Free(taskid);
	wp = Task_MS_Period_Table_WP - 1;
	for (remain = Task_MS_Period_Table_RP; remain < wp ; remain++)
	{
		Task_MS_Period_Table[remain] = Task_MS_Period_Table[remain+1];
	}
	Task_MS_Period_Table_WP = wp;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: TASK_Period_MS_Maintain
 * Purpose: MS period Table maitain
 * Params:  NONE
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------
 */
void TASK_Period_MS_Maintain(void)
{
	U8_T activeId, index, found=0;

	for (index=0; index < Task_MS_Period_Table_WP; index++)
	{
		activeId = Task_MS_Period_Table[index];
		if (Task_Active_Table[activeId].Task_Interval.w)
		{
			Task_Active_Table[activeId].Task_Interval.w--;
		}
		/* Update the pointer to point to the first task of time expired */
		if (!found && Task_Active_Table[activeId].Task_Interval.w == 0)
		{
#if (TaskControlEnable)
			if (Task_Active_Table[activeId].taskCtrl & TASK_EVENT_SKIP == 0) //for external kill
#else
			if ((Task_Active_Table[activeId].Task_Event & TASK_EVENT_SKIP) == 0)
#endif
			{
				Task_MS_Period_Table_RP = index;
				found = 1;
			}
		}
	}
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: Task_Period_MS_Main
 * Purpose: MS period Table management mian program
 * Params:  NONE
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------
 */
void TASK_MS_Period_Main(void)
{
	U8_T taskid;

	taskid = Task_MS_Period_Table[Task_MS_Period_Table_RP];
#if (TaskControlEnable)
	if ((Task_Active_Table[taskid].taskCtrl & (TASK_EVENT_DESTORY|TASK_EVENT_SKIP)) == 0x00)
#else
	if ((Task_Active_Table[taskid].Task_Event & (TASK_EVENT_DESTORY|TASK_EVENT_SKIP)) == 0x00)
#endif
	{
		if (Task_Active_Table[taskid].Task_Interval.w == 0)
		{
			TASK_Event = Task_Active_Table[taskid].Task_Event;
			TASK_Register0 = Task_Active_Table[taskid].Task_Para;
			TASK_Active_ID = taskid;
			TASK_Type = TASK_TYPE_INTERVAL_MS;
			Task_Table[Task_Active_Table[taskid].Task_ID].Task_FunctionP();

			// Check for next loop still active control status
#if (TaskControlEnable)
			if ((Task_Active_Table[taskid].taskCtrl & TASK_EVENT_ACTIVE) == 0x00)
#else
			if ((Task_Active_Table[taskid].Task_Event & TASK_EVENT_ACTIVE) == 0x00)
#endif
			{
				Task_Active_Table[taskid].Task_Interval.w = Task_Active_Table[taskid].Task_Reload.w;
			}
			else
			{
#if (TaskControlEnable)
				Task_Active_Table[taskid].taskCtrl &= ~TASK_EVENT_ACTIVE;
#else
				Task_Active_Table[taskid].Task_Event &= ~TASK_EVENT_ACTIVE;
#endif
			}
		}
	}
#if (TaskControlEnable)
	else if (Task_Active_Table[taskid].taskCtrl & TASK_EVENT_SKIP)
#else
	else if (Task_Active_Table[taskid].Task_Event & TASK_EVENT_SKIP)
#endif
	{
		if (Task_Active_Table[taskid].Task_Interval.w == 0)
			Task_Active_Table[taskid].Task_Interval.w = Task_Active_Table[taskid].Task_Reload.w;
	}
	// Check for Task Destory status
#if (TaskControlEnable)
	if (Task_Active_Table[taskid].taskCtrl & TASK_EVENT_DESTORY) //for external kill
#else
	if (Task_Active_Table[taskid].Task_Event & TASK_EVENT_DESTORY) //for external kill
#endif
	{
		TASK_Kill_MS_Period(taskid);
	}
	else
	{
		Task_MS_Period_Table_RP++;
	}
	
	if (Task_MS_Period_Table_RP >= Task_MS_Period_Table_WP)
	{
		Task_MS_Period_Table_RP = 0;
	}
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: Task_Event_Main
 * Purpose: Event Task management main program
 * Params:  NONE
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------
 */
void TASK_Event_Main(void)
{
	U8_T activeId;

	activeId = Task_Event_Table[Task_Event_Table_RP];
#if (TaskControlEnable)
	if ((Task_Active_Table[activeId].taskCtrl & (TASK_EVENT_DESTORY | TASK_EVENT_SKIP)) == 0)
#else
	if ((Task_Active_Table[activeId].Task_Event & (TASK_EVENT_DESTORY|TASK_EVENT_SKIP)) == 0x00)
#endif
	{
		TASK_Type = TASK_TYPE_EVENT;
		TASK_Event = Task_Active_Table[activeId].Task_Event;
		TASK_Register0 = Task_Active_Table[activeId].Task_Para;
		TASK_Register1 = Task_Active_Table[activeId].Task_Interval.bw.msb;
		TASK_Register2 = Task_Active_Table[activeId].Task_Interval.bw.lsb;
		TASK_Register3 = Task_Active_Table[activeId].Task_Reload.bw.msb;
		TASK_Register4 = Task_Active_Table[activeId].Task_Reload.bw.lsb;
		TASK_Active_ID = activeId;
		Task_Table[Task_Active_Table[activeId].Task_ID].Task_FunctionP();
#if (TaskControlEnable)
		if (Task_Active_Table[activeId].taskCtrl & TASK_EVENT_WAIT)
#else
		if (Task_Active_Table[activeId].Task_Event & TASK_EVENT_WAIT)
#endif
		{
#if (TaskControlEnable)
			Task_Active_Table[activeId].taskCtrl &= ~TASK_EVENT_WAIT;
#else
			Task_Active_Table[activeId].Task_Event &= ~TASK_EVENT_WAIT;
#endif
			if (Task_Event_Counter < (TASK_TABLE_EVENT_MAX - 1))
			{
				Task_Event_Table[Task_Event_Table_WP] = TASK_Active_ID;
				if (++Task_Event_Table_WP >= TASK_TABLE_EVENT_MAX)
				{
					Task_Event_Table_WP = 0;
				}
				Task_Event_Counter++;
			}
			else
			{
				printf("TASK_Event_Main(): !!! SYSTEM ERROR:TASK_EVENT OVERFLOW[%bu] !!!\n\r",Task_Active_Table[activeId].Task_ID);
				TASK_Free(activeId);
#if (SYSTEM_TASK_DUMP_SUPPORT)
				TASK_Table_Dump(TASK_TYPE_EVENT);
				Main_System_Halt();
#endif
			}
		}
		else
		{
			TASK_Free(activeId);
		}
	}
	else
	{
		TASK_Free(activeId);
	}

	if (Task_Event_Counter)
	{
		Task_Event_Counter--;
	}

	//Chek next task
	if (++Task_Event_Table_RP >= TASK_TABLE_EVENT_MAX)
	{
		Task_Event_Table_RP = 0;
	}
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: Task_USB_Main
 * Purpose: USB Task management main program
 * Params:  NONE
 * Returns: NONE
 * Note:
 * ----------------------------------------------------------------------------
 */
void TASK_USB_Main(void)
{
	U8_T taskid;

	taskid = Task_USB_Table[Task_USB_Table_RP];
#if (TaskControlEnable)
	if ((Task_Active_Table[taskid].taskCtrl & (TASK_EVENT_DESTORY|TASK_EVENT_SKIP)) == 0)
#else
	if ((Task_Active_Table[taskid].Task_Event & (TASK_EVENT_DESTORY|TASK_EVENT_SKIP)) == 0x00)
#endif
	{	
		TASK_Type = TASK_TYPE_USB;
		TASK_Event = Task_Active_Table[taskid].Task_Event;
		TASK_Register0 = Task_Active_Table[taskid].Task_Para;
		TASK_Register1 = Task_Active_Table[taskid].Task_Interval.bw.msb;
		TASK_Register2 = Task_Active_Table[taskid].Task_Interval.bw.lsb;
		TASK_Register3 = Task_Active_Table[taskid].Task_Reload.bw.msb;
		TASK_Register4 = Task_Active_Table[taskid].Task_Reload.bw.lsb;
		TASK_Active_ID = taskid;
		Task_Table[Task_Active_Table[taskid].Task_ID].Task_FunctionP();
	}

	TASK_Free(taskid);
	Task_USB_Table_RP++;	//Chek next task
	if (Task_USB_Table_RP >= TASK_TABLE_USB_MAX)
	{
		Task_USB_Table_RP = 0;
	}

	if (Task_USB_Counter)
		Task_USB_Counter--; // reset the counter

}

/*
 * ----------------------------------------------------------------------------
 * Function Name: TASK_Active
 * Purpose: Append a Task to task manager
 * Params: $task_type-TASK_TYPE_INTERVAL_MS
 * 					TASK_TYPE_EVENT
 * 					TASK_TYPE_USB
 *			$task_id- 0~255
 *			$task_interval- 1~65535
 *			$task_reload- 1~65535
 * Returns: 1-Success
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T TASK_Active(U8_T task_type,U8_T task_id,U8_T task_event,U8_T task_para,U16_T task_interval,U16_T task_reload)
{	
	U8_T rtn_id=0,freeid;	

	if (TASK_Malloc(&freeid))
	{
		rtn_id = freeid;
		Task_Active_Table[freeid].Task_ID = task_id;
		Task_Active_Table[freeid].Task_Event = task_event;
		Task_Active_Table[freeid].taskCtrl = 0;
		Task_Active_Table[freeid].Task_Para = task_para;
		Task_Active_Table[freeid].Task_Interval.w = task_interval;
		Task_Active_Table[freeid].Task_Reload.w = task_reload;
		switch(task_type)
		{
			case TASK_TYPE_USB:
				if (Task_USB_Counter < (TASK_TABLE_USB_MAX-1))
				{
					Task_USB_Table[Task_USB_Table_WP] = freeid;
					Task_USB_Table_WP++;
					if (Task_USB_Table_WP >= TASK_TABLE_USB_MAX)
						Task_USB_Table_WP = 0;

					Task_USB_Counter++;
				}
				else
				{
					
					printf("!!! SYSTEM ERROR:TASK_USB OVERFLOW !!!\n\r");
					TASK_Free(freeid);
#if (SYSTEM_TASK_DUMP_SUPPORT)
					TASK_Table_Dump(TASK_TYPE_USB);
					Main_System_Halt();
#endif
					return 0xff;
				}
				break;
			case TASK_TYPE_EVENT:
				if (Task_Event_Counter < (TASK_TABLE_EVENT_MAX-1))
				{
					Task_Event_Table[Task_Event_Table_WP] = freeid;
					if (++Task_Event_Table_WP >= TASK_TABLE_EVENT_MAX)
					{
						Task_Event_Table_WP = 0;
					}
					Task_Event_Counter++; 
					break;
				}
				else
				{
					printf("TASK_Active(): !!! SYSTEM ERROR:TASK_EVENT OVERFLOW[%bu] !!!\n\r",freeid);
					TASK_Free(freeid);
#if (SYSTEM_TASK_DUMP_SUPPORT)
					TASK_Table_Dump(TASK_TYPE_EVENT);
					Main_System_Halt();
#endif
					return 0xff;
				}
				break;
			case TASK_TYPE_INTERVAL_MS:
				if (Task_MS_Period_Table_WP < TASK_TABLE_MS_PERIOD_MAX)
				{
					Task_MS_Period_Table[Task_MS_Period_Table_WP] = freeid;
					Task_MS_Period_Table_WP++;
				}
				else
				{					
					printf("!!! SYSTEM ERROR:TASK_MS_INTERVAL OVERFLOW !!!\n\r");
					TASK_Free(freeid);
#if (SYSTEM_TASK_DUMP_SUPPORT)
					TASK_Table_Dump(TASK_TYPE_INTERVAL_MS);
					Main_System_Halt();
#endif
					return 0xff;
				}
				break;
		}
	}
	else
	{		
		printf("!!! TASK_ERROR:TASK_Malloc(),NO FREE ID !!!\n\r");
		return 0xff;
	}
	return rtn_id;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: Task_Kill
 * Purpose: Create a task in Task_Table
 * Returns: 1-Success
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T TASK_Create(void *functionp)
{
	if (Task_Table_Max >= TASK_TABLE_MAX)
	{
		return 0;
	}

	Task_Table[Task_Table_Max].Task_FunctionP = functionp;
	Task_Table_Max++;

	return (Task_Table_Max-1);
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: TASK_Table_Dump
 * Purpose: routine 1 sec task
 * Params: $task_type-TASK_TYPE_INTERVAL_MS
 *					TASK_TYPE_EVENT
 *					TASK_TYPE_USB
 *			$task_id- 0~255
 *			$task_interval- 1~65535
 *			$task_reload- 1~65535
 * Returns: 1-Success
 * Note:
 * ----------------------------------------------------------------------------
 */
#if (SYSTEM_TASK_DUMP_SUPPORT)
void TASK_Table_Dump(U8_T task_type)
{
	U8_T rp;

	if (task_type == TASK_TYPE_USB)
	{
		printf("--------------------\n\r");
		printf("|  Task USB Table  |\n\r");
		printf("--------------------\n\r");
		printf("No TaskID Event Para\n\r");
		printf("== ====== ===== ====\n\r");
		rp = Task_USB_Table_RP;
		while (rp != Task_USB_Table_WP)
		{
			printf("%02bu "   ,rp);
			printf("  %02bu  ",Task_USB_Table[rp]);			
			printf("%02bu\n\r",Task_Active_Table[Task_USB_Table[rp]].Task_ID);			
			++rp;
			if (rp >= TASK_TABLE_USB_MAX)
				rp = 0;
		}
		return;
	}

	if (task_type == TASK_TYPE_EVENT)
	{
		printf("--------------------\n\r");
		printf("| Task Event Table |\n\r");
		printf("\n\r");
		printf("No TaskID Event Para\n\r");
		printf("== ====== ===== ====\n\r");
		rp = Task_Event_Table_RP;
		while (rp != Task_Event_Table_WP)
		{
			printf("%02bu "   ,rp);
			printf("  %02bu " ,Task_Event_Table[rp]);			
			printf("%02bu\n\r",Task_Active_Table[Task_Event_Table[rp]].Task_ID);			
			++rp;
			if (rp >= TASK_TABLE_EVENT_MAX)
				rp = 0;
		}
		return;
	}

	if (task_type == TASK_TYPE_INTERVAL_MS)
	{
		printf("-------------------------------------\n\r");
		printf("        Task MS Period Table         \n\r");
		printf("\n\r");
		printf("No TaskID Event Para Interval Reload\n\r");
		printf("== ====== ===== ==== ======== ======\n\r");
		rp = 0;
		for (rp=0; rp < Task_MS_Period_Table_WP ; rp++)
		{
			printf("%02bu   " ,rp);
			printf("%02bu   " ,Task_MS_Period_Table[rp]);			
			printf("%02bu\n\r",Task_Active_Table[Task_MS_Period_Table[rp]].Task_ID);
		}
		return;
	}
}
#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: TASK_Destory_Current
 * Purpose: Destory the current task, only type of INTERVLA_MS
 * Returns: void
 * Note:
 * ----------------------------------------------------------------------------
 */
void TASK_Destory_Current(void)
{
#if (TaskControlEnable)
	Task_Active_Table[TASK_Active_ID].taskCtrl |= TASK_EVENT_DESTORY;
#else
	Task_Active_Table[TASK_Active_ID].Task_Event |= TASK_EVENT_DESTORY;
#endif
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: TASK_Destory_Task
 * Purpose: Destory the specific task
 * Returns: void
 * Note:
 * ----------------------------------------------------------------------------
 */
void TASK_Destory_Task(U8_T taskId)
{
#if (TaskControlEnable)
	Task_Active_Table[taskId].taskCtrl |= TASK_EVENT_DESTORY;
#else
	Task_Active_Table[taskId].Task_Event |= TASK_EVENT_DESTORY;
#endif
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: TASK_Wait_Current
 * Purpose: Keep alive and wait for next time
 * Returns: void
 * Note:
 * ----------------------------------------------------------------------------
 */
void TASK_Wait_Current(void)
{
#if (TaskControlEnable)
	Task_Active_Table[TASK_Active_ID].taskCtrl |= TASK_EVENT_WAIT;
#else
	Task_Active_Table[TASK_Active_ID].Task_Event |= TASK_EVENT_WAIT;
#endif
}

/*
 * ----------------------------------------------------------------------------
 * U8_T TASK_Malloc(U8_T *freeid)
 * Purpose: 
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static U8_T TASK_Malloc(U8_T *freeid)
{
	U8_T index,bidx,bitmap;

	for(index=0; index < TASK_SEARCH_MAX ; index++)
	{
		if (Task_Search[index] != 0xff )
		{
			for (bidx=0; bidx < 8 ; bidx++)
			{
				bitmap = 0x01 << bidx;
				if ((Task_Search[index] & bitmap) == 0x00)
				{
					Task_Search[index] |= bitmap;
					*freeid = (index <<3) + bidx;
					return 1;
				}
			}
		}
	}

	return 0;
}

/*
 * ----------------------------------------------------------------------------
 * void TASK_Free(U8_T freeid)
 * Purpose: Free the task from active table
 * Returns: void
 * Note:
 * ----------------------------------------------------------------------------
 */
static void TASK_Free(U8_T freeid)
{
	U8_T byte_inx,bitmpa;

	byte_inx = freeid >> 3;
	bitmpa = 0x01 << (freeid & 0x07);
	Task_Search[byte_inx] &= ~bitmpa;
#if (TaskControlEnable)
	Task_Active_Table[freeid].taskCtrl |= TASK_EVENT_DESTORY;
#else
	Task_Active_Table[freeid].Task_Event |= TASK_EVENT_DESTORY;
#endif
}

/*
 * ----------------------------------------------------------------------------
 * U8_T TASK_Run(TASK_ActiveTable_TypeDef *task)
 * Purpose: 
 * Returns: 
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T TASK_Run(TASK_ActiveTable_TypeDef *task)
{
	U8_T rtn_id;

	rtn_id = TASK_Active(task->Task_Type,task->Task_ID,task->Task_Event,task->Task_Para,task->Task_Interval.w,task->Task_Reload.w);

	return rtn_id;
}

/* End of task_main.c */
