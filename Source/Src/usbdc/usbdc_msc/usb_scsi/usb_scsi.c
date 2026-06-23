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
 * Module Name : usb_scsi.c
 * Purpose     : 
 * Author      : 
 * Date        :
 * Notes       :
 *================================================================================
 */
#include "project_include.h"
#include "usb_scsi.h"

#if (SYSTEM_USB_PEN_DRIVE_SUPPORT)

/* STATIC VARIABLE DECLARATIONS */

U8_T				scsi_Flash_Write_Protect_Flag = 0;
static const U8_T	scsi_InquiryData[DATA_LENGTH_INQUIRY] =
{
	0x00,		//  0, PDT-peripheral qualifier & device type
	0x80,		//  1, RMB :removable medium
	0x02,		//  2, version : obsolete
	0x02,		//  3, refer to spec. 
	0x1F,		//  4, additional length (n - 4), n = 35
	0x00,		//  5, refer to spec.
	0x00,		//  6, refer to spec.
	0x00,		//  7, refer to spec.

				// Vendor identification : 8 bytes
	'A', 'S', 'I', 'X', ' ', ' ', ' ', ' ',
				// Product identification : 16 bytes
	'V', 'I', 'N', 'S', 'O', 'N', ' ', ' ',
	'P', 'r', 'o', 'd', 'u', 'c', 't', ' ',
				// Product Revision Level : 4 bytes
	'1', '.', '0', '0'
};

static const U8_T scsi_RequestSenseData[DATA_LENGTH_REQUEST_SENSE] = {
	0x70,		// 0, response code : Fixed format sense data
	0x00,		// 1, obsolete
	0x00,		// 2, sense key : NO SENSE
	0x00,		// 3-6, information
	0x00,
	0x00,
	0x00,
	0x0A,		// 7, additional length (n - 7), n = 17

	0x00,		// 8-11, information
	0x00,
	0x00,
	0x00,
	0x00,		// 12, additional sense code
	0x00,		// 13, additional sense code qualifier
	0x00,		// 14, field replaceable unit code
	0x00,		// 15-17, sense-key specific
	0x00,
	0x00,
};

static const U8_T scsi_RequestSenseData_WriteProtect[DATA_LENGTH_REQUEST_SENSE] = {
	0x70,					// 0, response code : Fixed format sense data
	0x00,					// 1, obsolete
	KEY_DATA_PROTECT,		// 2, sense key : NO SENSE
	0x00,					// 3-6, information
	0x00,
	0x00,
	0x00,
	0x0A,					// 7, additional length (n - 7), n = 17

	0x00,					// 8-11, information
	0x00,
	0x00,
	0x00,
	ADDKEY_DATA_PROTECT,	// 12, additional sense code
	0x00,					// 13, additional sense code qualifier
	0x00,					// 14, field replaceable unit code
	0x00,					// 15-17, sense-key specific
	0x00,
	0x00,
};

static const U8_T scsi_ModeSenseData[DATA_LENGTH_MODE_SENSE] =
{
	0x17, 0x00, 0x00, 0x08,													// Mode parameter header(6)
	0x00, LBA_HL, LBA_LH, LBA_LL, 0x00, BS_HL, BS_LH, BS_LL,				// Block descriptor(s)
	0x01, 0x0A, 0x80, 0x10, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,	// Mode page(s)
};

static const U8_T scsi_ModeSenseDataWriteProtect[DATA_LENGTH_MODE_SENSE] =
{
	0x17, 0x00, 0x80, 0x08,														// Mode parameter header(6), Byte2(Bit7): Write Protect
	0x00, LBA_HL, LBA_LH, LBA_LL, 0x00, BS_HL, BS_LH, BS_LL,					// Block descriptor(s)
	0x01, 0x0A, 0x80, 0x10, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,		// Mode page(s)
};

static const U8_T scsi_FormatCapacityData[DATA_LENGTH_FORMATCAP] =
{
	0x00,		//  0	Reserved
	0x00,		//  1,  Reserved
	0x00,		//  2,  Reserved
	0x08,		//  3,  Capacity List Length
	LBA_HH,		//  4,  LBA (MSB)
	LBA_HL,		//  5,
	LBA_LH,		//  6,
	LBA_LL,		//  7,  LBA (LSB)
	0x01,		//  8,  desc code (unformatted media)
	BS_HL,		//  9,  block length1.(MSB)
	BS_LH,		//  10, block length2
	BS_LL		//  11, block length3(LSB)
};

static const U8_T scsi_ReadCapacityData[DATA_LENGTH_READCAP] =
{
	LBA_HH,		// 0 ~ 3 Returned logical block address
	LBA_HL,
	LBA_LH,
	LBA_LL,	
	BS_HH,		// Block length in bytes
	BS_HL,
	BS_LH,
	BS_LL
};

static const U8_T scsi_ReportLunsData[DATA_LENGTH_LUNS] =
{
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,		// 4 Reserved
	0x00,		// 5 Reserved
	0x00,		// 6 Reserved
	0x00		// 7 Reserved
};

/* GLOBAL VARIABLE DECLARATIONS */
U8_T 	*SCSI_SramPt = 0;
U32_T	SCSI_FlashAddr = 0;
U32_T	SCSI_DataLength = 0;	//device intends to transfer

/* LOCAL SUBPROGRAM DECLARATIONS */
static void scsi_CmdModeSelect(U8_T* cdb);
static void scsi_CmdModeSense(U8_T* cdb);
static void scsi_CmdRead(U8_T* cdb);
static void scsi_CmdWrite(U8_T* cdb);

/* LOCAL SUBPROGRAM BODIES */
/*
 *--------------------------------------------------------------------------------
 * void	scsi_CmdModeSelect(U8_T* cdb)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void scsi_CmdModeSelect(U8_T* cdb)
{
	switch (cdb[0]) {
		case SCSI_OP_MODE_SELECT_6:
			SCSI_DataLength = cdb[4];
			break;

		case SCSI_OP_MODE_SELECT_10:
			SCSI_DataLength = ((U32_T)cdb[7] << 8) + cdb[8];
			break;
	}

	if (SCSI_DataLength > sizeof(scsi_ModeSenseData))
		SCSI_DataLength = sizeof(scsi_ModeSenseData);

	//memcpy(scsi_RespData, scsi_ModeSenseData, SCSI_DataLength);
	//SCSI_SramPt = scsi_RespData;
	if (scsi_Flash_Write_Protect_Flag==0)
		SCSI_SramPt = scsi_ModeSenseData;
	else
		SCSI_SramPt = scsi_ModeSenseDataWriteProtect;
} /* End of scsi_CmdModeSelect() */

/*
 *--------------------------------------------------------------------------------
 * void	scsi_CmdModeSense(U8_T* cdb)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void scsi_CmdModeSense(U8_T* cdb)
{
	switch (cdb[0]) {
		case SCSI_OP_MODE_SENSE_6:
			SCSI_DataLength = cdb[4];
			break;

		case SCSI_OP_MODE_SENSE_10:
			SCSI_DataLength = ((U32_T)cdb[7] << 8) + cdb[8];
			break;
	}

	if (SCSI_DataLength > sizeof(scsi_ModeSenseData))
		SCSI_DataLength = sizeof(scsi_ModeSenseData);

	if (scsi_Flash_Write_Protect_Flag==0)
		SCSI_SramPt = scsi_ModeSenseData;
	else
		SCSI_SramPt = scsi_ModeSenseDataWriteProtect;
} /* End of scsi_CmdModeSense() */

/*
 *--------------------------------------------------------------------------------
 * void	scsi_CmdRead(U8_T* cdb)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void scsi_CmdRead(U8_T* cdb)
{
	switch (cdb[0]) {
		case SCSI_OP_READ_10:
			SCSI_DataLength = ((U32_T)cdb[7] << 8 | (U32_T)cdb[8]) * LOGICAL_BLK_SZ;
			break;

		case SCSI_OP_READ_12:
			SCSI_DataLength = ((U32_T)cdb[6] << 24 | (U32_T)cdb[7] << 16 | (U32_T)cdb[8] << 8 | (U32_T)cdb[9]) * LOGICAL_BLK_SZ;
			break;
	}

	SCSI_FlashAddr = (((U32_T)cdb[2] << 24 | (U32_T)cdb[3] << 16 | (U32_T)cdb[4] << 8 | (U32_T)cdb[5]) * LOGICAL_BLK_SZ);
} /* End of scsi_CmdRead() */

/*
 *--------------------------------------------------------------------------------
 * void	scsi_CmdWrite(U8_T* cdb)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void scsi_CmdWrite(U8_T* cdb)
{
	switch (cdb[0]) {
		case SCSI_OP_WRITE_10:
			SCSI_DataLength = ((U32_T)cdb[7] << 8 | (U32_T)cdb[8]) * LOGICAL_BLK_SZ;
			break;

		case SCSI_OP_WRITE_12:
			SCSI_DataLength = ((U32_T)cdb[6] << 24 | (U32_T)cdb[7] << 16 | (U32_T)cdb[8] << 8 | (U32_T)cdb[9]) * LOGICAL_BLK_SZ;
			break;
	}

	SCSI_FlashAddr = (((U32_T)cdb[2] << 24 | (U32_T)cdb[3] << 16 | (U32_T)cdb[4] << 8 | (U32_T)cdb[5]) * LOGICAL_BLK_SZ);
} /* End of scsi_CmdWrite() */

/* EXPORTED SUBPROGRAM BODIES */
/*
 *--------------------------------------------------------------------------------
 * MSC_STATE SCSI_CmdDecode(U8_T* cdb)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
U8_T SCSI_CmdDecode(U8_T *cdb, U8_T *cmdSupport)
{
	MSC_STATE MSstate;

	/* Parse SCSI command */
	switch (cdb[0]){		//Operation Code
		case SCSI_OP_REQUEST_SENSE:
			SCSI_DataLength = cdb[4];

			if (SCSI_DataLength > sizeof(scsi_RequestSenseData))
				SCSI_DataLength = sizeof(scsi_RequestSenseData);
				
			if (scsi_Flash_Write_Protect_Flag==0)
				SCSI_SramPt = scsi_RequestSenseData;
			else
				SCSI_SramPt = scsi_RequestSenseData_WriteProtect;
			MSstate = MSC_CMD_RESPONSED;
			break;

		case SCSI_OP_INQUIRY:
			SCSI_DataLength = cdb[4];
			if (SCSI_DataLength > sizeof(scsi_InquiryData))
				SCSI_DataLength = sizeof(scsi_InquiryData);
				
			SCSI_SramPt = scsi_InquiryData;
			MSstate = MSC_CMD_RESPONSED;
			break;

		case SCSI_OP_MODE_SELECT_6:
		case SCSI_OP_MODE_SELECT_10:
			scsi_CmdModeSelect(cdb);
			MSstate = MSC_CMD_RESPONSED;
			break;
			
		case SCSI_OP_MODE_SENSE_6:
		case SCSI_OP_MODE_SENSE_10:
			scsi_CmdModeSense(cdb);
			MSstate = MSC_CMD_RESPONSED;
			break;

		case SCSI_OP_READ_FORMAT_CAPACITY:
			SCSI_DataLength = sizeof(scsi_FormatCapacityData);
			
			SCSI_SramPt = scsi_FormatCapacityData;
			MSstate = MSC_CMD_RESPONSED;
			break;
		case SCSI_OP_READ_CAPACITY:
			SCSI_DataLength = sizeof(scsi_ReadCapacityData);
			
			SCSI_SramPt = scsi_ReadCapacityData;
			MSstate = MSC_CMD_RESPONSED;
			break;

		case SCSI_OP_READ_10:
		case SCSI_OP_READ_12:
			scsi_CmdRead(cdb);
			MSstate = MSC_DATA_IN;
			break;
 
		case SCSI_OP_WRITE_10:
		case SCSI_OP_WRITE_12:
			scsi_CmdWrite(cdb);
			MSstate = MSC_DATA_OUT;
			break;
			
		case SCSI_OP_REPORT_LUNS:
			SCSI_DataLength = sizeof(scsi_ReportLunsData);
			
			SCSI_SramPt = scsi_ReportLunsData;
			MSstate = MSC_CMD_RESPONSED;
			break;

		case SCSI_OP_START_STOP_UNIT :
		case SCSI_OP_TEST_UNIT_READY:
		case SCSI_OP_VERIFY:		 
			SCSI_DataLength = 0;
			MSstate = MSC_CSW;
			break;
		case SCSI_OP_MEDIUM_REMOVAL:
			SCSI_DataLength = 0;
			MSstate = MSC_CSW;
			//*cmdSupport = 0;
			break;
		default:
			SCSI_DataLength = 0;
			*cmdSupport = 0;
			MSstate = MSC_OP_NOT_SUPPORT;
			break;
	}

	return MSstate;
} /* End of SCSI_CmdDecode() */
#endif /* #if (SYSTEM_USB_PEN_DRIVE_SUPPORT) */

/* End of usb_scsi.c */
