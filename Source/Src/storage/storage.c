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
 * Module Name : storage.c
 * Purpose     : This module handles the storage control function
 * Author      : Jack Wang
 * Date        :
 * Notes       : 
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include <stdio.h>
#include <string.h>
#include <absacc.h>
#include "project_include.h"

#if (SYSTEM_STORAGE_DEVICE_SUPPORT)
/* GLOBAL VARIABLE DECLARATIONS */
U8_T		STORAGE_AvailablePartitionMap;

/* STATIC VARIABLE DECLARATIONS */
/* LOCAL SUBPROGRAM DECLARATIONS */
/* LOCAL SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * void  STORAGE_Init(void)
 * Purpose : Stroage feature Initial
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void  STORAGE_Init(void) 
{
	//Internal Flash initialized
#if (SYSTEM_STORAGE_FLASH)
	STORAGE_AvailablePartitionMap = 0x0F;
	FLASH_Setup();
#endif //#if (SYSTEM_STORAGE_FLASH)
}

/*
 *--------------------------------------------------------------------------------
 * void  STORAGE_CRC_Calculate(U8_T *pSource)
 * Purpose : Caulate the CRC value for KVM_Flash
 * Params  : $pSource : the source buffer pointer
 * Returns : Return the checksum.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
U16_T  STORAGE_CRC_Calculate(U8_T *pSource)
{
	U32_T cksum = 0;
	U16_T i, length;
	U8_T *inChar = pSource;
	U16_T *point, *pBuf;

	length = sizeof(KVM_Flash);
    pBuf = (U16_T *)pSource;
	for (i = 0; i < (length/2); i++)
	{
		point = pBuf;
		cksum += *pBuf++;
		if (point > pBuf)
			(*inChar) = *inChar + 1;

		if (!(i % 0x8000))
		{
			cksum = (cksum >> 16) + (cksum & 0xffff);
			cksum += (cksum >> 16);
			cksum &= 0xffff;
		}
	}
		
	if (length % 2)
		cksum += (*pBuf & 0xff00);

	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	cksum &= 0xffff;

	return ((U16_T)cksum);
}

/*
 *--------------------------------------------------------------------------------
 * void  STORAGE_Default_Initial(U8_T initwrite)
 * Purpose : Stroage feature Initial feature
 * Params  : $initwrite : Init KVM_Flash data structure and write(if 1)
 * Returns : return the result value.
 * Note    : 
 *--------------------------------------------------------------------------------
 */
void STORAGE_Default_Initial(void) 
{
	/* Autoscan interval time */ 
	KVM_Flash.Total_Length = sizeof(KVM_Flash);
	KVM_Flash.AutoScanInterval = 5; // 0-1 defautl 5 second
	KVM_Flash.AutoScanStopCode = 0xff; // any key
	KVM_Flash.AutoScanStop_JumpCode = CHAR_SPACE;
	KVM_Flash.PowerSavingMode = SYSTEM_POWER_STOP_MODE; 	// Stop Mode
	/* Control relative */
	KVM_Flash.cSystemFlag0 = SYSTEM_BEEPER_MASK | SYSTEM_HOTKEY_ALTERNATE_MASK;	
	KVM_Flash.cSystemFlag1 = 0; // 4  ystem Flag2
	KVM_Flash.cSystemFlag2 = SYSTEM_TX_AUDIOCOMBO_MASK;	
	KVM_Flash.cSystemFlag3 = 0;
	
#ifdef 	SYNC_DEFAULT
	#if (SYNC_DEFAULT==ALL)		
		KVM_Flash.cSystemFlag2 |= (SYSTEM_ALL_SYNC_MASK | SYSTEM_KB_SYNC_MASK | SYSTEM_MS_SYNC_MASK);	
	#endif
		
	#if (SYNC_DEFAULT==KB)	
		KVM_Flash.cSystemFlag2 |= SYSTEM_KB_SYNC_MASK;	
	#endif

	#if (SYNC_DEFAULT==MS)	
		KVM_Flash.cSystemFlag2 |= SYSTEM_MS_SYNC_MASK;	
	#endif	
#else
	KVM_Flash.cSystemFlag2 |= SYSTEM_MS_DN_MASK;
#endif /* #ifdef 	SYNC_DEFAULT */

#ifdef MSWITCH
	KVM_Flash.cSystemFlag3 |= SYSTEM_MOUSE_SW_MASK;
#endif

//if ((KVM_Flash.cSystemFlag2 & SYSTEM_MS_SYNC_MASK) == 0x00)	
{	
#ifdef MPASS_DEFAULT
	KVM_Flash.cSystemFlag3 &= ~SYSTEM_MPASS_MASK;
	if ((KVM_Flash.cSystemFlag2 &(SYSTEM_ALL_SYNC_MASK | SYSTEM_KB_SYNC_MASK | SYSTEM_MS_SYNC_MASK)) == 0x00)	
	{	
	#if (MPASS_DEFAULT==ACROSS)		
		KVM_Flash.cSystemFlag3 |= MPASS_ACROSS_MODE;
	#endif		
	
	#if (MPASS_DEFAULT==BUTTON)	
		KVM_Flash.cSystemFlag3 |= MPASS_BUTTON_MODE;
	#endif
	}	
#endif
}
#if (KVM_HOTKEY_SUPPORT)
	HOTKEY_Default_Initial();
#endif /* #if (KVM_HOTKEY_SUPPORT) */

#ifdef 	VHID_DEFAULT
	#if (VHID_DEFAULT==ALL)	
		KVM_Flash.cSystemFlag3 |= (SYSTEM_VHID_MS_MASK | SYSTEM_VHID_KB_MASK);	
	#endif
		
	#if (VHID_DEFAULT==KB)	
		KVM_Flash.cSystemFlag3 |= SYSTEM_VHID_KB_MASK;	
	#endif

	#if (VHID_DEFAULT==MS)	
		KVM_Flash.cSystemFlag3 |= SYSTEM_VHID_MS_MASK;	
	#endif	
#endif /* #ifdef 	SYNC_DEFAULT */

#if (SYSETM_CONSOLE_EDID_CONTROL_ENABLE)	
#ifdef EDID_SAVE
	memset(KVM_Flash.Edid_Table,0x00,sizeof(KVM_Flash.Edid_Table));
	memcpy(KVM_Flash.Edid_Table,CONSOLE_EDID_DEFAULT,sizeof(CONSOLE_EDID_DEFAULT));  
#endif	
#endif	

#ifdef ONE_HID
	KVM_Flash.cSystemFlag3 |= (SYSTEM_VHID_KB_MASK|SYSTEM_VHID_MS_MASK);	
#endif
	

	if (KVM_Flash.Version != 1001) //version 1.0.0.1
	{
		//printf("Default Flash\n\r");
		KVM_Flash.Version = 1000;
		STORAGE_Write(sizeof(KVM_Flash),(U8_T *)&KVM_Flash);
	}		
}

#if (SYSTEM_STORAGE_FLASH)
/*
 *--------------------------------------------------------------------------------
 * void  STORAGE_Init_Read(U16_T readlen,U8_T *readbuf,U8_T master)
 * Purpose : Read the Flash content into KVM_Flash
 * Params  : $readlen : length to read
 *           $readbuf : the storeage buffer address
 			 $partition : Partition number
 * Returns : return the result value
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void STORAGE_Init_Read(U16_T readlen,U8_T *readbuf,U8_T partition)
{
	U16_T	index;
	U32_T	addr;
	
	switch (partition)
	{
		case 0:
			addr = STORAGE_START_PRIMARY_ADDRESS;
			break;
		case 1:
			addr = STORAGE_START_SECONDARY_ADDRESS;
			break;
		case 2:
			addr = STORAGE_START_THIRDARY_ADDRESS;
			break;
		case 3:
			addr = STORAGE_START_FOURTH_ADDRESS;
			break;
		default:
			printf("Wrong partition number!\n\r");
			return;
	}

	for (index = 0 ; index < readlen ; index++)
	{
		*(readbuf + index) = FCVAR(U8_T, addr + index);
	}
}
#endif //#if (SYSTEM_STORAGE_FLASH)

/*
 *--------------------------------------------------------------------------------
 * void STORAGE_Write(U16_T writelen,U8_T *writebuf) 
 * Purpose : Write data into flash
 * Params  : $writelen: data length for writting
 *           $writebuf: the data buffer for writting
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void STORAGE_Write(U16_T writelen,U8_T *writebuf)
{
	KVM_Flash.CRC_Check = 0;
	KVM_Flash.CRC_Check = ~STORAGE_CRC_Calculate(writebuf);

#if (SYSTEM_STORAGE_FLASH)
	if (FLASH_SectorErase())
	{
		if (writelen & 0x7)
		{
			writelen = (writelen & 0xFFF8) + 8;
		}
		DMA_DataToProgram(STORAGE_START_PRIMARY_ADDRESS, (U32_T)writebuf & 0xFFFF, writelen);
		DMA_DataToProgram(STORAGE_START_SECONDARY_ADDRESS, (U32_T)writebuf & 0xFFFF, writelen);
		DMA_DataToProgram(STORAGE_START_THIRDARY_ADDRESS, (U32_T)writebuf & 0xFFFF, writelen);
		DMA_DataToProgram(STORAGE_START_FOURTH_ADDRESS, (U32_T)writebuf & 0xFFFF, writelen);
		STORAGE_AvailablePartitionMap = 0x0F;
	}
#endif //#if (SYSTEM_STORAGE_FLASH)
}

/*
 *--------------------------------------------------------------------------------
 * void STORAGE_SoftwareConfigurationCheck(U8_T houseKeeping) 
 * Purpose : Check all software configuration partitions on flash
 * Params  : $houseKeeping : run this routine for house keeping
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void STORAGE_SoftwareConfigurationCheck(U8_T houseKeeping)
{
	U8_T				partition;
	U8_T				availableCnt = 0;
	struct _KVM_Flash	*pFlash;
	
	if (houseKeeping)
	{
		pFlash = &KVM_FlashTemporary;
	}
	else
	{
		pFlash = &KVM_Flash;
	}
	
	for (partition = 0; partition < 4; partition++)
	{
		if (STORAGE_AvailablePartitionMap & (0x01 << partition))
		{
			STORAGE_Init_Read(sizeof(KVM_Flash),(U8_T *)pFlash, partition);
			if (0xFFFF != STORAGE_CRC_Calculate((U8_T *)pFlash) || pFlash->Total_Length != sizeof(KVM_Flash))
			{
				STORAGE_AvailablePartitionMap &= ~(0x01 << partition);
			}
			else if (houseKeeping)
			{
				availableCnt++;
			}
			else
			{
				if (availableCnt == 0)
				{
					memcpy((U8_T *)&KVM_FlashTemporary, pFlash, sizeof(KVM_Flash));
				}
				availableCnt++;
			}
		}
	}

	if (houseKeeping == 0)
	{
		if ((STORAGE_AvailablePartitionMap & 0x08) == 0)
		{
			if (availableCnt)
			{
				memcpy(pFlash, (U8_T *)&KVM_FlashTemporary, sizeof(KVM_Flash));
			}
			else
			{
				STORAGE_Default_Initial();
			}
		}
	}
	
	if (availableCnt < 2)
	{
		STORAGE_Write(sizeof(KVM_Flash),(U8_T *)&KVM_Flash);
	}
}

#endif /* End of SYSTEM_STORAGE_DEVICE_SUPPORT */

/* End of storage.c */
