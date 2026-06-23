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
 * Module Name: usbdc.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __USBDC_H__
#define __USBDC_H__

/* INCLUDE FILE DECLARATIONS */
/* DEFINATION DECLARATIONS */


#define USBDC_VHUB_PORT_MAX		7   // 7 Port Virtual HUB port MAX number
#define VDEVICE_RESUME_KEEP_QUITE		15   // 15 ms period time delay
#define VBUS_RESUME_KEEP_QUITE		 	6000 // 6000 ms keep quite
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Standard Request Type command For Hub
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//#define PORT_COONNECTION		0
#define PORT_ENABLE				1
#define PORT_SUSPEND			2
//#define PORT_OVER_CURRENT		3
#define PORT_RESET				4
#define PORT_POWER				8
//#define PORT_LOW_SPEED			9

#define C_HUB_LOCAL_POWER		0
#define C_HUB_OVER_CURRENT		1
#define C_PORT_CONNECT			16
#define C_PORT_ENABLE			17
#define C_PORT_SUSPEND			18
#define C_PORT_OVER_CURRENT		19
#define C_PORT_RESET			20

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// GetStatusChange 4個 byte定義
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  Byte 1 wPortStatusL 
#define   L_PortStatus_Connection		0x01  //bit0
#define   L_PortStatus_Enable			0x02  //bit1
#define   L_PortStatus_Suspend			0x04  //bit2
#define   L_PortStatus_OverCurrent		0x08  //bit3
#define   L_PortStatus_Reset			0x10  //bit4
#define   L_PortStatus_Sleep			0x20  //bit5

//  Byte 2 wPortStatusH		
#define   H_PortStatus_Power			0x01  //bit0
#define   H_PortStatus_LowSpeed			0x02  //bit1
#define   H_PortStatus_HighSpeed		0x04  //bit2
#define   H_PortStatus_Test				0x08  //bit3
#define   H_PortStatus_Indicator		0x10  //bit4

//  Byte 3 wPortChangeL	
#define   L_PortChange_Connection		0x01  //bit0
#define   L_PortChange_Enable			0x02  //bit2
#define   L_PortChange_Suspend			0x04  //bit3
#define   L_PortChange_OverCurrent		0x08  //bit4
#define   L_PortChange_Reset			0x10  //bit5
//  Byte 4 wPortChangeH	=> All is zero

typedef struct _USBDC_VHubReportTypeDef
{
	U8_T  wPortStatusL;
	U8_T  wPortStatusH;
	U8_T  wPortChangeL;
	U8_T  wPortChangeH;
	U8_T  Devinx;   // the devinx which is connect to this port
} USBDC_VHubReportTypeDef;

typedef enum
{
	//不要改變順序，因為是按照 PortStatus來定義
	HUB_PORT_PORTSTATUS_NONE			= 0x0000,
	HUB_PORT_PORTSTATUS_CONNECT			= 0x0001, // device connect
	HUB_PORT_PORTSTATUS_ENABLE			= 0x0002, // port enable
	HUB_PORT_PORTSTATUS_SUSPEND			= 0x0004, // port suspend
	HUB_PORT_PORTSTATUS_OVERCURRENT		= 0x0008, // port overcurrent
	HUB_PORT_PORTSTATUS_RESET			= 0x0010, // port rest
	HUB_PORT_PORTSTATUS_SLEEP			= 0x0020, // port sleep
	
//  Byte 2 wPoRTSTATUSH		
	HUB_PORT_PORTSTATUS_POWER			= 0x0100, // port has power  	
	HUB_PORT_PORTSTATUS_LOWSPEED		= 0x0200, // device low speed  	
	HUB_PORT_PORTSTATUS_HIGHSPEED		= 0x0400, // device high speed  	
	HUB_PORT_PORTSTATUS_TEST			= 0x0800,
	HUB_PORT_PORTSTATUS_INDICATOR		= 0x1000,
} HUB_PORT_STATUS;

typedef struct _VHub_ReportTypeDef
{
	USBDC_VHubReportTypeDef PortStatus[USBDC_PORT_MAX][USBDC_VHUB_PORT_MAX];
	U8_T  HubChanged[USBDC_PORT_MAX];
} VHub_ReportTypeDef;

/*----------------------------------------------
$ The Index to Endp Buffer Address Structure
*/
#define DC_USED_MASK                  0x80
#define DC_INDEX_MASK                 0x07

#define VDEV_UNMOUNT                  0x00
#define VDEV_MOUNT                    0x01


/*----------------------------------------------
$USBDC_UpPortState()
*/
#define USBDC_ROOTHUB_ATTACHED_MASK				0x01
	#define USBDC_ROOTHUB_ATTACHED_RESET 		0x00
	#define USBDC_ROOTHUB_ATTACHED_SET			0x01

#define USBDC_ROOTHUB_SUSPEND_MASK				0x02
	#define USBDC_ROOTHUB_SUSPEND_RESET 		0x00
	#define USBDC_ROOTHUB_SUSPEND_SET	 		0x02
	
#define USBDC_ROOTHUB_RESUME_MASK				0x04
	#define USBDC_ROOTHUB_RESUME_RESET			0x00
	#define USBDC_ROOTHUB_RESUME_SET			0x04
	
#define USBDC_ROOTHUB_SOF_MASK					0x08
	#define USBDC_ROOTHUB_SOF_LOCK_RESET		0x00
	#define USBDC_ROOTHUB_SOF_LOCK_SET			0x08

#define USBDC_ROOTHUB_REMOTE_WAKEUP				0x10
/*-----------------------------------------------
$Attribute()
*/
#define USBDC_TABLE_DOUBF_MASK			0x80
#define USBDC_TABLE_ISO_MASK 			0x40
#define USBDC_TABLE_DIR_MASK 			0x20
#define USBDC_TABLE_DIR_IN_MASK			0x20
#define USBDC_TABLE_DIR_OUT_MASK		0x00
#define USBDC_TABLE_BURST_MASK 			0x10

/*------------------------------------------------
$Feature()
*/
#define FEATURE_SUSPEND              BIT0
#define FEATURE_RESUME	             BIT1
#define FEATURE_REMOTE               BIT5
#define FEATURE_BUSPOWER             BIT6

/*
$ ISR_Handle
*/
#define ISR_HANDLE_SET_ADDRESS      0x01


/*
$DevAttr
*/
#define RESET_ONCE_WHEN_SW_MASK			0x01

/* Unifying */
#define RESET_UNIFYING_INIT_MASK		0x02
#define UNIFYING_CHANGE_MODE_MASK		0x04
#define UNIFYING_CHANGE_HID_MASK		0x08
#define UNIFYING_RPT_MODE_MASK			0x10  //0x10-DJ,0x00-HID
#define UNIFYING_RPT_MODE_DJ			0x10  
#define UNIFYING_RPT_MODE_HID			0x00
#define UNIFYING_RPT_SIZE_MASK			0x20  //0x20-Long,0x00-Short
#define UNIFYING_RPT_MODE_LONG			0x20
#define UNIFYING_RPT_MODE_SHORT			0x00
#define UNIFYING_RPT_CHECK_MASK			0x40  //next is to check the report mode
#define INTERVAL_CHECK_MASK				0x80  //next is to check the report mode

/* TD2220 */
#define TD2220_FORMAT_MASK				0x10
#define TD2220_FORMAT_C14				0x10
#define TD2220_FORMAT_C6				0x00

#if (SYSTEM_EXTENDER_RECEIVER)
#define USBDC_DEVIDX_INTR_SEND_OK_MASK	0x01  //
#endif /* (SYSTEM_EXTENDER_RECEIVER) */

/*------------------------------------------------*/
typedef struct _USBDC_DeviceTypeDef
{  
	//~~~~~~~~~~~~~~~~~~~~~~~
	//1.Unique Part
	//~~~~~~~~~~~~~~~~~~~~~~~
	U8_T  DevIdx;											//bit 7-used
	U8_T  EndpInterval0[USBDC_PORT_MAX][USBDC_ENDP_MAX];	//the endpoint size of each endpoint buffer
	U8_T  EndpInterval1[USBDC_PORT_MAX][USBDC_ENDP_MAX];	//the endpoint size of each endpoint buffer
	U8_T  Interval_Check[USBDC_PORT_MAX];
	U8_T  Interval_Detect[USBDC_PORT_MAX];					//to match FSBSD behavior, it will check after detect a valid Endpoint Interrupt IN
	U8_T  VirHubNum[USBDC_PORT_MAX];						//Assing to which virtual hub port number
	U8_T  EndpNum;											//The total endpoint  number		
	U8_T  EndpMaxSize[USBDC_ENDP_MAX];						//the endpoint size of each endpoint buffer		
	U8_T  EndpAddr[USBDC_ENDP_MAX];							//Bit7-Double buffer Flag	  
	U8_T  EndpType[USBDC_ENDP_MAX];							//bit 7~5 , transfer type
															//bit 4~0 , TD id number       
#if (SYSTEM_EXTENDER_TRANSMITTER)
	U8_T  EndpIntrAttr;										//When in extender transmitter,each endp has a interrupt transfer
															//control attribute, each bit standard one endp index, only bit is 0,
															//can send data out
	U8_T  EndpIntrWait;
#endif /* #if (SYSTEM_EXTENDER_TRANSMITTER) */
	U8_T  DevAttr[USBDC_PORT_MAX];							//Reset when kvm siwtch condition
															//bit0-Reset Flag
															//bit1-Reset for driver issue
															//bit2-Reset for Set Report issue
															//bit3-
															//bit4-
															//bit5-
															//bit6-
															//bit7-
	U8_T  Unifying_index;									//bit0~3=>index
															//bit4~7=>software id
	U8_T  Reset_Issue[USBDC_PORT_MAX];						//Reset when kvm siwtch condition
	//~~~~~~~~~~~~~~~~~~~~~~~
	//2.Port Relative Part
	//~~~~~~~~~~~~~~~~~~~~~~~
	U8_T  DevAddr[USBDC_PORT_MAX];							//Address 0~127
	U8_T  Feature[USBDC_PORT_MAX];							//bit0-Suspend
															//bit5-Remote  
															//bit6-Bus Power

	U8_T  Endp_Stall[USBDC_PORT_MAX];						//bit0-Control,Bit1-Endpoint1 ....	
	U8_T  Current_Configuration[USBDC_PORT_MAX];
	U8_T  Current_AlternateSetting[USBDC_PORT_MAX][6];
	U8_T  Current_Protocol[USBDC_PORT_MAX];					// 0-report 1-boot
															// each bit standard for one interface.
	//~~~~~~~~~~~~~~~~~~~~~~~
	//3.Control Setup Part
	//~~~~~~~~~~~~~~~~~~~~~~~
	DCSETUP_CONTROL_STATE	ControlState[USBDC_PORT_MAX];
	U8_T					Control_Cnt[USBDC_PORT_MAX];
	DCSETUP_CONTROL_STATE	PassThroughState[USBDC_PORT_MAX];	// this is the DC pass through state
//	U8_T					PassThroughCounter[USBDC_PORT_MAX];	// this is the DC pass through counter
	U8_T					ISR_Handle[USBDC_PORT_MAX];
	U8_T					*Control_EndpBuf[USBDC_PORT_MAX];	//each endpoint buffer address 	
	U16_T					Ctrl_TotalByte[USBDC_PORT_MAX];		//Current Transfer-total byte to received or send
	U16_T					Ctrl_CurrentByte[USBDC_PORT_MAX];	//Current Transfer-how many byte has been received or sended
	USB_Setup_TypeDef		Setup[USBDC_PORT_MAX];

	//~~~~~~~~~~~~~~~~~~~~~~~
	//4.Endpx Control State
	//~~~~~~~~~~~~~~~~~~~~~~~	
	VHub_ReportTypeDef  *VHub;
} USBDC_DeviceTypeDef;

#define USBDC_ISR_SKIP   0x80
/*--------------------------------
$$ ISR FIFO Buffer Data Structure
*/
typedef struct 
{  
	U8_T  Pid;  // port id
	U8_T  Isr;  // Interrupt status register
	U8_T  Insr; // Interrupt Index Status Register 
	U8_T  Esmr;	// Endipoint Status 
} USBDC_ISR_FIFO_TypeDef;


/*----------------------------------------------
/* NAMING CONSTANT DECLARATIONS */
/* GLOBAL VARIABLES */
extern volatile USBDC_DeviceTypeDef			USBDC_Device[USBDC_DEVICE_MAX];
extern volatile U8_T						USBDC_UpPortState[USBDC_PORT_MAX];
extern volatile USBDC_ISR_FIFO_TypeDef		USBDC_ISR_FIFO[USBDC_ISR_FIFO_DEPTH];	/*ISR FIFO Buffer*/
extern idata volatile U8_T					USBDC_ISRQ_Rp;	/*ISR FIFO Pointer*/
extern idata volatile U8_T					USBDC_ISRQ_Wp;	/*ISR FIFO Pointer*/
extern U8_T									TASK_USBDC_Get_EndpInterval_ID;
extern U8_T XDATA	USB_Dynamic_Memory[USB_DYNAMIC_MEMORY_SIZE];
extern U8_T    USBDC_Resume_Task_ActiveID[USBDC_PORT_MAX];
extern U8_T    USBDC_Resume_Timer_ActiveID[USBDC_DEVICE_MAX];
extern U8_T    VDevice_Keep_Quite_Flag[USBDC_PORT_MAX];
extern U8_T    DC_Keep_Quite_Flag;
extern U8_T    TASK_DC_Keep_Quite_TASKID;
extern U8_T    TASK_VDevice_Keep_Quite_TASKID;
/* EXPORTED SUBPROGRAM SPECIFICATIONS */

/* EXPORTED GLOBAL VARIABLES */
void		USBDC_Init(void);
U8_T		USBDC_VirtualDevice_Create(U8_T devinx);
void		USBDC_Virtual_Hub_DeviceMount_Control(U8_T devinx,U8_T hub_port,U8_T mounttype,U8_T active_port,U8_T mount_dev);
void		USBDC_Free_VDevice_Memory(U8_T devinx);
void		USBDC_Free_VDevice(U8_T devinx);
void		USBDC_Port_Normal(U8_T pid);
FlagStatus	USBDC_Search_EndpAddr(U8_T devidx,U8_T endpaddr,U8_T *endpidx);
U8_T		USBDC_PortSuspendCheck(void);
void		USBDC_Audio_Switch_Check(U8_T port,U8_T feature_select);
U8_T		USBDC_Check_Endp_Buffer_Valid(U8_T port,U8_T devinx,U8_T endpinx);
U8_T		USBDC_Get_Endp_Interval(U8_T port,U8_T devinx,U8_T endpinx);
void 		USBDC_Maintain_MSC_PDevice(U8_T pid);
void		USBDC_WakeupUpstreamHost(U8_T pid);
U8_T 		USBDC_Check_Upstream_Suspend_State(U8_T port,U8_T devinx);
void	 	USBDC_Virtual_Hub_Reset(U8_T pid,U8_T force);
#endif /* End of __USBDC_MAIN__ */

/* End of usbdc_main.h */