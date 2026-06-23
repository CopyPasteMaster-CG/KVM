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
 * Module Name : usbhc_hidparser.h
 * Purpose     : KB & Mouse hid data package data package parser program
 *               
 * Author      : Jack Wang
 * Date        :
 * Notes       :
 * no message
 *
 *
/*================================================================================*/

/*
-------------------------------------------------------------------------------
 表1. 報告描述元的標籤
--------------------       -----------------------------------------------------
主項目                     全域項目                     區域項目
--------------------       ---------------------        ------------------------
標籤 						代碼 	  	 標籤             代碼				標籤
--------------------       ---------------------        ------------------------
Input           0x8?       Usage Page       0x0?        Usage               0x0?
Output          0x9?       Logical Minimum  0x1?        Usage Minimum       0x1?
Feature         0xB?       Logical Maximum  0x2?        Usage Maximum       0x2?
Collection      0xA1       Physical Minimum 0x3?        Designator Index    0x3?
End Collection  0xC0       Physical Maximum 0x4?        Designator Minimum  0x4?
                           Unit Exponent    0x5?        Designator Maximum  0x5?
                           Unit             0x6?        String              0x7?
                           Report Size      0x7?        String Minimum      0x8?
                           Report ID        0x8?        String Maximum      0x9?
                           Report Count     0x9?        Delimiter           0xA?
                           Push             0xA? 
                           Pop              0xB? 
 
標籤
用途標籤只是報告描述元諸多標籤之一。表1 列出所有的標籤，利用這些標籤即
可以清楚完整的描述操控的用途。報告描述元的語法不同於USB 標準描述元
者，它是以項目(items)方式排列而成，無一定的長度﹔項目有一個前稱(prefix)，
然後跟著一個括號，內為該項目的資料(data)：item = prefix (data)。項目又分三
種型別：主項目，全域項目，區域項目。主項目中的Input，Output，Feature 三
個標籤用來表示報告中數據的種類，這些是報告描述元中最主要的項目，其他項
目都是用來修飾這三種項目。主項目中其他二個標籤容後再介紹，在此略過。
[1] Input 項：表示由裝置操控輸入到主機的數據格式。這個數據格式就形成一
    個輸入報告，雖然輸入報告可以用控制型管線以Get_Report(Input)來傳輸，
    但是通常用中斷型輸入管線來傳輸，以確保在每一固定週期內都能將更新的
    輸入報告傳給主機。
    
[2] Output 項：表示由主機輸出到裝置操控的數據格式。這個數據格式就形成
    一個輸出報告。輸出報告通常不適用輪詢方式來傳給裝置，而是由應用軟體
    依實際需求以指令方式要求送出輸出報告，所以大多是用控制型管線以
    Set_Report(Output)指令來將報告送到裝置。當然也可以選擇用中斷型輸出管
    線來傳送，只是通常不建議。
    
[3] Feature 項：表示由主機送到裝置的組態所需資料的數據格式。這個個數據
    格式就形成一個特徵報告。特徵報告只能用控制型管線以
    Get_Report(Feature)和Set_Report(Feature)指令分別來取得和設定裝置的特
    徵值。

==========================================================================================

編碼
報告描述元的項目編碼有二種：短項目和長項目。長項目僅是保留給未來使用，
所以沒有介紹之必要。短項目的編碼形式如下：
Bytes 2 1 0

Bits 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0

Byte 0 	|-Bit 0-1 => bSize
			 	|-Bit 2-3 => bType
				|-Bit 4-7 => bTag

Byte 1  Bit 8-15 => data

Byte 2  Bit 16-23 => data

最低位元組分別標註項目大小(bSize)，項目型別(bType)，和項目標籤(bTag)﹔其
中bTag 佔4 個位元，其餘二者各佔2 個位元。BSize 用來指出項目的資料所需
位元組的數目，該數目僅可以為0（當bSize=0），1（當bSize=1），2（當bSize=2），
和4（當bSize=3）﹔注意不可以為3 個位元組。大部分的標籤僅需一個位元組
的資料﹔全域項目的標籤Unit 比較特殊有可能最多用到4 個位元組來表示其資
料。
標籤代碼bTag 已經列於第一篇文章的表1 中，例如Input 的標籤代碼『0x8?』中
8 即為bTag 之值﹔再如標籤Feature 之bTag=11，而Unit 之bTag=6。
主項目之bType=0，全域項目之bType=1，而區域項目之bType= 2。所以在第一
篇文章的表1 中的主項目標籤代碼中的『?』可以改為『00nnB』，全域項目的可
以改為『01nnB』，而區域項目的可以改為『10nnB』，其中nn 代表bSize。 
*/ 
//--------------------------------------------------------------------------
//   定義宣告：Define area  
//--------------------------------------------------------------------------
#ifndef __USB_HC_HID_PARSER_H__
#define __USB_HC_HID_PARSER_H__

   #define HID_SIZE_MASK        0x03
   #define HID_TYPE_MASK        0x0C
   #define HID_TAG_MASK         0xF0
   
   /*
   * HID report descriptor item type (prefix bit 2,3)
   */

   #define HID_ITEM_TYPE_MAIN              0x00
   #define HID_ITEM_TYPE_GLOBAL            0x04
   #define HID_ITEM_TYPE_LOCAL             0x08
   #define HID_ITEM_TYPE_RESERVED          0x0c
   

/*  Entity Item Tags (MAIN)- HID 5.4.3  */
   #define MAIN_INPUT 				0x80
   #define MAIN_OUTPUT 				0x90
   #define MAIN_COLLECTION 			0xA0
   #define MAIN_FEATURE 			0xB0
   #define MAIN_END_COLLECTION		0xC0
   
   /*  Entity Attribute Item Tags(GLOBAL) HID 5.4.4 */
   #define GLOBAL_USAGE_PAGE 			0x04
   #define GLOBAL_LOGICAL_MINIMUM 		0x14
   #define GLOBAL_LOGICAL_MAXIMUM 		0x24
   #define GLOBAL_PHYSICAL_MINIMUM 		0x34
   #define GLOBAL_PHYSICAL_MAXIMUM 		0x44
   #define GLOBAL_EXPONENT 				0x54
   #define GLOBAL_UNIT 					0x64
   #define GLOBAL_REPORT_SIZE 			0x74
   #define GLOBAL_REPORT_ID 			0x84
   #define GLOBAL_REPORT_COUNT 			0x94
   #define GLOBAL_PUSH 					0xA4
   #define GLOBAL_POP 					0xB4
   
   /*  Control Attribute Item Tags(LOCAL)  */
   #define LOCAL_USAGE 					0x08
   #define LOCAL_USAGE_MIN 				0x18
   #define LOCAL_USAGE_MAX 				0x28
   #define LOCAL_DESIGNATOR_INDEX 		0x38
   #define LOCAL_DESIGNATOR_MIN 		0x48
   #define LOCAL_DESIGNATOR_MAX 		0x58
   #define LOCAL_STRING_INDEX 			0x78
   #define LOCAL_STRING_MIN 			0x88
   #define LOCAL_STRING_MAX 			0x98
   #define LOCAL_SET_DELIMITER 			0xA8
	/*
	 * HID usage page tables
	 */
		#define HID_UP_UNDEFINED        0x0000
		#define HID_UP_GENDESK          0x0001
		#define HID_UP_SIMULATION       0x0002
		#define HID_UP_GENDEVCTRLS      0x0006
		#define HID_UP_KEYBOARD         0x0007
		#define HID_UP_LED              0x0008
		#define HID_UP_BUTTON           0x0009
		#define HID_UP_ORDINAL          0x000a
		#define HID_UP_CONSUMER         0x000c
		#define HID_UP_DIGITIZER        0x000d
		#define HID_UP_PID              0x000f
		#define HID_UP_HPVENDOR         0xff7f
		#define HID_UP_HPVENDOR2        0xff01
		#define HID_UP_MSVENDOR         0xff00
		#define HID_UP_CUSTOM           0x00ff
		#define HID_UP_LOGIVENDOR       0xffbc
		#define HID_UP_SENSOR           0x0020

	/*
	 * HID usage tables
	 */
	#define HID_GD_POINTER          0x0001
	#define HID_GD_MOUSE            0x0002
	#define HID_GD_JOYSTICK         0x0004
	#define HID_GD_GAMEPAD          0x0005
	#define HID_GD_KEYBOARD         0x0006
	#define HID_GD_KEYPAD           0x0007
	#define HID_GD_MULTIAXIS        0x0008
	#define HID_GD_X                0x0030
	#define HID_GD_Y                0x0031
	#define HID_GD_Z                0x0032
	#define HID_GD_RX               0x0033
	#define HID_GD_RY               0x0034
	#define HID_GD_RZ               0x0035
	#define HID_GD_SLIDER           0x0036
	#define HID_GD_DIAL             0x0037
	#define HID_GD_WHEEL            0x0038
	#define HID_GD_HATSWITCH        0x0039
	#define HID_GD_BUFFER           0x003a
	#define HID_GD_BYTECOUNT        0x003b
	#define HID_GD_MOTION           0x003c
	#define HID_GD_START            0x003d
	#define HID_GD_SELECT           0x003e
	#define HID_GD_VX               0x0040
	#define HID_GD_VY               0x0041
	#define HID_GD_VZ               0x0042
	#define HID_GD_VBRX             0x0043
	#define HID_GD_VBRY             0x0044
	#define HID_GD_VBRZ             0x0045
	#define HID_GD_VNO              0x0046
	#define HID_GD_FEATURE          0x0047
	#define HID_GD_UP               0x0090
	#define HID_GD_DOWN             0x0091
	#define HID_GD_RIGHT            0x0092
	#define HID_GD_LEFT             0x0093 

	/*  Long Item Tage  */
	#define R_ITEM_LOGN				0xFE

	#define HID_USAGE_PAGE          0x05
	#define HID_USAGE_PAGE_3        0x06
	#define HID_USAGE               0x09
	#define HID_USAGE_3             0x0A
	#define HID_LOGICAL_MINIMUN_2   0x15
	#define HID_LOGICAL_MINIMUM_3   0x16
	#define HID_USAGE_MINIMUN_2     0x19
	#define HID_USAGE_MINIMUN_3     0x1a
	#define HID_LOGICAL_MAXIMUM_2   0x25
	#define HID_LOGICAL_MAXIMUM_3   0x26
	#define HID_USAGE_MAXIMUM_2     0x29
	#define HID_USAGE_MAXIMUM_3     0x2a

	#define HID_REPORT_SIZE         0x75
	#define HID_REPORT_SIZE_3       0x76
	#define HID_PAGE_ID             0x85
	#define HID_REPORT_COUNT        0x95
	#define HID_REPORT_COUNT_3      0x96

	/*
	* Main_Item Data
	*/
	#define MAIN_IO_DATA_ARRAY      0x0000
	#define MAIN_IO_DATA_CONSTANT   0x0001
	#define MAIN_IO_DATA_VARABLE    0x0002


#define KB_IN_SECTION_MAX           0x03
#define MS_IN_SECTION_MAX           0x06
#define USB_MULTI_HOLD_Buf_Max      4


#define HID_KEYBOARD       1
#define HID_MOUSE          2

#define USB_KB_BUF_MAX              16
#define USB_MS_BUF_MAX              16

#define OPERATION_BIT      0
#define OPERATION_BYTE     1

#define  LED_USAGE_PAGE             0x01
#define  LED_OUTPUT_USAGE_PAGE      0x02

#define NORMAL_KEY_MAKE               1
#define NORMAL_KEY_BREAK              2
#define MODIFIER_KEY_MAKE             3
#define MODIFIER_KEY_BREAK            4
#define CONSUMER_KEY_MAKE             5
#define CONSUMER_KEY_BREAK            6

#define KB_HID_DETAIL_MAX             20
#define MS_HID_DETAIL_MAX             30

#define HID_INDEX_USED_MASK           0x80
#define HID_INDEX_TYPE_MASK           0x70   //1-Mouse,0-Keyboard

#define REPORT_USAGE_PAGE_MAX		5
 

struct   _HID_NAME_DEBUG_TABLE
{
	U8_T Id;
	U8_T Content[20];
};

/*
 * HID report descriptor global item tags
 */
typedef struct _HID_Global
{
	U8_T  Report_Id;
	U16_T Usage_Page;
	U16_T Logical_Minimum;
	U16_T Logical_Maximum;
	U8_T Report_Size;
	U8_T Report_Count;
} HID_Global;

/*
* HID report descriptor local item tags
*/
typedef struct _HID_Local
{
	U16_T Usage;
	U16_T Usage_Minimum;
	U16_T Usage_Maximum;
} HID_Local;

/*
$ Keyboard HID report descriptor Parser Structure

  State define
*/
#define HID_PARSER_RANGE_MSB_MASK  0xf0
#define HID_PARSER_RANGE_LSB_MASK  0x0f

#define HID_PARSER_REPORT_TYPE     0x80
#define HID_PARSER_REPORT_DATA     0x40
#define HID_PARSER_REPORT_ARRAY    0x20

#define HID_STATE_SKIP_COLLECTION  0x01
#define HID_STATE_SKIP_MAIN        0x02
#define HID_STATE_COLLECTION       0x04

typedef struct _HID_Rpt_Parser
{
	U8_T  *HID_DescriptorP;		//HID Report Descriptor Pointer
	U8_T  Current_In_MainItem;
	U8_T  Current_Out_MainItem;
	U8_T  Current_In_BitLen;
	U8_T  Total_In_BitLen;
	U8_T  Current_Out_BitLen;
	U8_T  Total_Out_BitLen;
	U8_T  State;			//HID parser State
	U16_T Desc_RemainLen;	//HID Report Descriptor Praser Remain Length
	U16_T Desc_Cnt;			//HID Report Descriptor Praser Counter
	U16_T Collection_UsagePage;
	U16_T Collection_Usage;
	HID_Global	Global;
	HID_Local   Local;
} HID_Rpt_Parser_TypeDef;

/*
$HID Keyboard Parser Table Structure 
*/
typedef struct _HID_Rpt_Element
{
	U8_T  RangeLSB;		//bit Order LSB
	U8_T  RangeMSB;		//bit Order MSB
	U8_T  Rpt_Size;		//Report Size
	U8_T  Rpt_Count;	//Report Count
	U16_T Usage_Min;	//Usage Minimun
} HID_Rpt_Element;

/*
$Item
*/
#define  HID_RPT_MODIFIERS     0
#define  HID_RPT_LED           1
#define  HID_RPT_KEYCODE       2

/*
$Attr_PageID
*/
#define  HID_RPT_DATATYPE_MASK 0x80
#define  HID_RPT_DATATYPE_MASK 0x80
#define  HID_RPT_DATATYPE_BIT  0x00

#define  HID_RPT_PAGEID_MASK   0x1f


typedef struct _HID_Rpt_Table
{
	U8_T  Attr_PageID;		//7-Data type(0-Bit,1-Byte)
							//4~0 page ID(0~31)
//	U8_T  Inf_EndpAddr;		//bit7-4 interface id
							//bit3~0 endp addr
	U8_T  PackageLen;		//package length
//	U8_T  *HidBuf;			//Last keyboard hid data package buffer pointer
	HID_Rpt_Element Item[3];	//0-for modifier
								//1-for out led
								//2-for keycode
} HID_Rpt_Table_TypeDef; 

//-------------------------------------------------------------------------- 
//   外部全域變數宣告：External Global variable define  
//--------------------------------------------------------------------------

//-------------------------------------------------------------------------- 
//   外部函數宣告：External functio declaration
//--------------------------------------------------------------------------
void USBHC_HidParser_Init(void);
void USBHC_HidParser_Buf_Initc(U8_T devinx);
U8_T USBHC_HidParser_Buf_Malloc(U8_T devinx,U8_T intf,U8_T endp);
void USBHC_HidParser_Buf_Free(U8_T devinx);
U8_T USBHC_HidParser_Package_Search(U8_T usbcode,U8_T *buf);
void USBHC_HidParser_Package_Handle(U8_T devinx,U8_T inf_id,U8_T *buf,U8_T len);
U8_T USBHC_HidParser_Codetype(U8_T keytype,U8_T usb_keycode); // key press
void USBHC_HidParser_ReportDescription(U8_T devinx);
void USBHC_Parser_Gigabyte_2270(U8_T devinx,U8_T *buf);
#endif /* __USB_HC_HID_PARSER_H__ */