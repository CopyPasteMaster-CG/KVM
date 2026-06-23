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
 * Module Name : console_edid.h
 * Purpose     : A header file of Console EDID program
 * Author      : Jack Wang
 * Date        :
 * Notes       : 
 * no message
 *
 *================================================================================
 */
#ifndef __CONSOLE_EDID_H
#define __CONSOLE_EDID_H

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define CONSOLE_EDID_DEFAULT_LEN	128

/*
$ Console_EDID_State
*/
#define CONSOLE_MONITOR_PLUG_OUT             0
#define CONSOLE_MONITOR_PLUG_IN              1
#define CONSOLE_MONITOR_RELOAD               3
#define CONSOLE_MONITOR_PLUG_IN_ERROR        5
#define CONSOLE_MONITOR_PLUG_IN_SKIP         7
#define CONSOLE_MONITOR_PLUG_CHECK           9
#define CONSOLE_MONITOR_STATUS_CHK			 11
#define CONSOLE_MONITOR_RELOAD_WAIT          13
#define CONSOLE_MONITOR_PLUG_PAUSED			 15
/*
$ Console_EDID_Stage
*/
#define EDID_STAGE_IDLE						0 // no any active stage
#define EDID_STAGE_WAIT_STANDBY             2 // Send start+0xA0 then check ack bit
#define EDID_STAGE_WRITE_START              4 // Send start+0xA0 then check ack bit
#define EDID_STAGE_READ_START               6 // Send start+0xA1 then check ack bit
#define EDID_STAGE_WRITE_ADDRESS            8 // Send start+0xA0 then check ack bit
#define EDID_STAGE_READ_ACK                10  // read data, then check ack bit
#define EDID_STAGE_READ_NOACK              12 // read data, then no check ack bit
#define EDID_STAGE_STOP                    14 // generate a stop condition
#define EDID_STAGE_DETECT_MONITOR			16 // After Monitor plug in, then get into
                                              // plug out status monitor

#define  EDID_MAX							 256 

/*
$ ConsoleEdidTimer
*/
#define  EDID_CONSOLE_CHECK_TIME             100  // 1 second

/* MACRO DECLARATIONS */


/* TYPE DECLARATIONS */
typedef enum {
	MONITOR_UNPLUGGED=0,
	MONITOR_PLUGGED,
} MONITOR_STATUS;

typedef enum {
	EDID_RELOAD_BUSY=0,
	EDID_RELOAD_OK,	
	EDID_RELOAD_FAIL,	
} EDID_RELOAD_STATUS;

typedef struct _EDID_Ctrl {
	U8_T  MonitorPlugStatus;
	U8_T  MonitorDebounceCnt;		
	U8_T  MonitorDebouncedPlugStatus;
	U8_T  MonitorPrePlugStatus;
	U8_T  EdidReloadStatus;
	U8_T  EdidReloadRetryCnt;
		
	U8_T  TaskState;
	U16_T  Timer;
	U8_T  TaskPauseRequest;	
	U8_T  EdidReloadRequest;
} EDID_Ctrl;

/*
$ Macro Function
*/

/* GLOBAL VARIABLES */
extern EDID_Ctrl	EDID_ctrl;
extern U8_T CONSOLE_EDID_DEFAULT[CONSOLE_EDID_DEFAULT_LEN];

extern U8_T  ConsoleEdidTable[];// Console Edid Table
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void  EDID_Console_Init(void);
void  TASK_EDID_Console_Plug_Check(void);
void  TASK_EDID_Console_ISR_Handle(ISR_TypeDef *pIsr);
#endif /* End of __CONSOLE_EDID_H */




