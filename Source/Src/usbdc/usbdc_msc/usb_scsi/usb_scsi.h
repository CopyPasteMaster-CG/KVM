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
 * Module Name : usb_scsi.h
 * Purpose     : 
 * Author      : 
 * Date        :
 * Notes       :
 *================================================================================
 */

#ifndef __USB_SCSI_H
#define __USB_SCSI_H

#define SPC_4	0x06
//Capacity size=(0xFF +1) x 4096 = 1MB

#define LBA_LL	0xFF
#define LBA_LH	0x00
#define LBA_HL	0x00
#define LBA_HH	0x00
#define BS_LL	0x00
#define BS_LH	0x10
#define BS_HL	0x00
#define BS_HH	0x00
 
#define LOGICAL_BLK_SZ	(BS_LL | BS_LH << 8 | BS_HL << 16 | BS_HH << 24)
#define MAX_DISK_SZ	((LBA_LL | LBA_LH << 8 | LBA_HL << 16 | LBA_HH << 24) * (LOGICAL_BLK_SZ))

/*********************************************/
/*************** SCSI definition******************/
/********************************************/
typedef enum{
	SIMPLE,
	HEAD_OF_QUEUE,
	ORDERED,
	ACA
} Task_Attributes;

typedef enum{
	NEW_COMMAND,
	DORMANT,
	ENABLED,
	BLOCKED,
	COMPLETED
}Command_State;

struct scsi_read_capacity_data
{
	U32_T return_logical_block_address;
	U32_T logical_block_length_in_bytes;
};

struct scsi_inquiry_data
{
	U8_T peripheral_qd;
	U8_T RMB;			// removable medium bit
	U8_T version;
	U8_T feature1;		// HiSup : hierarchical support bit
	U8_T additional_length;
	U8_T feature2;
	U8_T feature3;
	U8_T feature4;
	U8_T t10_vandor_id[8];
	U8_T product_id[16];
	U8_T prodcuct_revision_level[4];
	U8_T vendor_spec[20];
	U8_T reserved;
	U16_T version_descriptor[8];
};

struct scsi_fix_format_sense_data
{
	U8_T resp_code;
	U8_T sense_key;			// [0:3] 
	U8_T addi_sense_code;
	U8_T addi_sense_code_qua;
	U8_T reserved[3];
	U8_T addi_sense_length;	// n-7 
};

struct scsi_desc_format_sense_data
{
	U8_T resp_code;
	U8_T sense_key; // [0:3]
	U8_T addi_sense_code;
	U8_T addi_sense_code_qua;
	U8_T reserved[3];
	U8_T addi_sense_length;	// n-7
};


struct scsi_mode_parameter_header
{
	U8_T mode_data_length;
	U8_T medium_type;
	U8_T device_spec_para;
	U8_T block_decs_length;
};

struct scsi_cmd_read10
{
	U8_T op_code;
	U8_T feature;
	U32_T lba;
	U8_T group_num;
	U16_T transfer_length;
	U8_T control;
};

struct scsi_cmd_write10
{
	U8_T op_code;
	U8_T feature;
	U32_T lba;
	U8_T group_num;
	U16_T transfer_length;
	U8_T control;
};


/*********************************************/
/********************************************/

#define CARD_INDEX_NAND		0
#define CARD_INDEX_NAND_SS	1
#define CARD_INDEX_SPI_SS	2
#define CARD_INDEX_SRAM		7
#define CARD_TYPE_MAX_REPORT	1
	
typedef struct	// SCSI device response
{
	U8_T u8MemIndex;		// current access memory type
	U8_T u8Flags;			// In or Out
	U32_T u32IOAddr;		// block addr in Card controller
	U16_T u16DataResidue;	// residue (block count for card content, or byte count for information)
	U16_T u16TfSzCurrent;		// current transfer length (block count for card content, or byte count for information)
} SCSIDeviceResp;

typedef struct SCSI_Capacity_Header
{
	U8_T	Reserve1;
	U8_T	Reserve2;
	U8_T	Reserve3;
	U8_T	Capacity_List_Length;
}SCSICapacityHeader;

typedef struct SCSI_Current_Capacity
{
	U32_T	Num_Of_Blocks;
	U8_T	Desc_Code;
	U8_T	Block_Len0;
	U8_T	Block_Len1;
	U8_T	Block_Len2;
}SCSICurrentCapacity;
	
typedef struct SCSI_Format_Capacity
{
	U32_T	Num_Of_Blocks;
	U8_T	Desc_Code;
	U8_T	Reserved;
	U8_T	Block_Len1;
	U8_T	Block_Len2;
}SCSIFormatCapacity;
	


typedef struct SCSI_Format_Descriptor
{
	SCSICapacityHeader	Cap_Head;
	SCSIFormatCapacity	Current_Cap;
}SCSIFormatDescriptor;


/*********************************************/
/********************************************/

/* SCSI command operation code */
#define SCSI_OP_TEST_UNIT_READY			0x00
#define SCSI_OP_REQUEST_SENSE			0x03
#define SCSI_OP_INQUIRY					0x12
#define SCSI_OP_MODE_SELECT_6			0x15
#define SCSI_OP_MODE_SENSE_6			0x1A
#define SCSI_OP_START_STOP_UNIT			0x1B
#define SCSI_OP_MEDIUM_REMOVAL			0x1E
#define SCSI_OP_READ_FORMAT_CAPACITY	0x23
#define SCSI_OP_READ_CAPACITY			0x25
#define SCSI_OP_READ_10					0x28
#define SCSI_OP_WRITE_10				0x2A
#define SCSI_OP_VERIFY					0x2F
#define SCSI_OP_MODE_SELECT_10			0x55
#define SCSI_OP_MODE_SENSE_10			0x5A
#define SCSI_OP_READ_12					0xA8
#define SCSI_OP_WRITE_12				0xAA
#define SCSI_OP_REPORT_LUNS				0xA0

// Request Sense data format
#define SENSE_OFFSET_KEY			0x02
#define SENSE_OFFSET_ADD			0x0C
#define SENSE_OFFSET_ADD_QUALIFIER	0x0D

// Sense key
#define KEY_NO_SENSE				0x00
#define KEY_RECOVERED_ERROR			0x01
#define KEY_NOT_READY				0x02
#define KEY_MEDIUM_ERROR			0x03
#define KEY_HARDWARE_ERROR			0x04
#define KEY_ILLEGAL_REQUEST			0x05
#define KEY_UNIT_ATTENTION			0x06
#define KEY_DATA_PROTECT			0x07
#define KEY_BLANK_CHECK				0x08
#define KEY_VENDOR_SPECIFIC			0x09
#define KEY_COPY_ABORTED			0x0A
#define KEY_ABORTED_CMD				0x0B
#define KEY_OBSOLETE				0x0C
#define KEY_VOLUMN_OVERFLOW			0x0D
#define KEY_MISCOMPARE				0x0E
#define KEY_RESERVED				0x0F

// KEY_NO_SENSE: Additional key
#define ADDKEY_NO_ADDITIONAL		0x00

// KEY_NOT_READY: Additional key
#define ADDKEY_LOGICAL_UNIT_NOT_READY	0x04
#define ADDKEY_LOGICAL_UNIT_NOT_SUPPORT	0x25
#define ADDKEY_MEDIUM_NOT_PRESENT		0x3A

// KEY_ILLEGAL_REQUEST: Additional key
#define ADDKEY_INVALID_PARM_LEN			0x1A
#define ADDKEY_INVALID_CMD_OP_CODE		0x20
#define ADDKEY_INVALID_FIELD_IN_CMD 	0x24

// KEY_UNIT_ATTENTION: Additional key
#define ADDKEY_MEDIUM_CHANGED			0x28

// KEY_DATA_PROTECT: Additional key
#define ADDKEY_DATA_PROTECT			0x27

// all response data length
#define DATA_LENGTH_INQUIRY			36
#define DATA_LENGTH_REQUEST_SENSE	18
#define DATA_LENGTH_MODE_SENSE		24
#define DATA_LENGTH_FORMATCAP		12
#define DATA_LENGTH_READCAP			8

#define DATA_LENGTH_LUNS			8

/* variable */
extern U32_T SCSI_DataLength;	//device intends to transfer
extern U8_T	*SCSI_SramPt;
extern U32_T SCSI_FlashAddr;
extern U8_T  scsi_Flash_Write_Protect_Flag;

/* function */
U8_T SCSI_CmdDecode(U8_T* cdb, U8_T *cmd_support);

#endif //#ifndef __USB_SCSI_H