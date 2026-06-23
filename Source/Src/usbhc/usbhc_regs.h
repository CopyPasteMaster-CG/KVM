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
 * Module Name: usbhc_regs.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __USBHC_REGISTER_H__
#define __USBHC_REGISTER_H__

/* INCLUDE FILE DECLARATIONS */

/* NAMING CONSTANT DECLARATIONS */
/*-------------------------------------------------
void HC_CONTROL_0x00()
*/
#define HC_CONTROL_REG						0x00
#define HCFS_MASK							0x03
#define HCFS_RESET							0x00
#define HCFS_RESUME							0x01
#define HCFS_OPERATIONAL					0x02
#define HCFS_SUSPEND						0x03
#define HC_RWE_MASK							0x04
#define HC_RWE_SET							0x04
#define HC_RWE_RESET						0x00
#define HC_SWR_MASK							0x08
#define HC_SWR_ENABLE						0x08
#define HC_SWR_DISABLE						0x00
/* New for RTO 0624 */
#define HC_HCR_MASK							0x10
#define HC_SWR_SET							0x10
#define HC_SWR_RESET						0x00
#define HC_PWP_MASK							0x20
#define HC_PWP_SET							0x20
#define HC_PWP_RESET						0x00

/*-------------------------------------------------
void HC_OPERATIONAL_INTERRUPT_CONTROL_0x02()
*/
#define HC_OPINT_CONTROL_REG				0x02
#define HC_OPINT_CON_SOE_MASK				0x01
#define HC_OPINT_CON_SOE_ENABLE				0x01
#define HC_OPINT_CON_SOE_DISABLE			0x00
#define HC_OPINT_CON_RDE_MASK				0x02
#define HC_OPINT_CON_RDE_ENABLE				0x02
#define HC_OPINT_CON_RDE_DISABLE			0x00
#define HC_OPINT_CON_FNOE_MASK				0x04
#define HC_OPINT_CON_FNOE_ENABLE			0x04
#define HC_OPINT_CON_FNOE_DISABLE			0x00
#define HC_OPINT_CON_RHSCE_MASK				0x78
#define HC_OPINT_CON_RHSCE_ENABLE			0x78
#define HC_OPINT_CON_RHSCE_DISABLE			0x00
#define HC_OPINT_CON_P0_RHSCE_MASK			0x08
#define HC_OPINT_CON_P1_RHSCE_MASK			0x10
#define HC_OPINT_CON_P2_RHSCE_MASK			0x20
#define HC_OPINT_CON_P3_RHSCE_MASK			0x40

/*-------------------------------------------------
void HC_OPERATION_INTERRUPT_STATE_0x03()
*/
#define HC_OPINT_STATE_REG					0x03
#define HC_OPINT_STATUS_MASK				0x07
#define HC_OPINT_STATE_SO_MASK				0x01
#define HC_OPINT_STATE_SO_ENABLE			0x01
#define HC_OPINT_STATE_SO_DISABLE			0x00
#define HC_OPINT_STATE_RD_MASK				0x02
#define HC_OPINT_STATE_RD_ENABLE			0x02
#define HC_OPINT_STATE_RD_DISABLE			0x00
#define HC_OPINT_STATE_FNO_MASK				0x04
#define HC_OPINT_STATE_FNO_ENABLE			0x04
#define HC_INT_STATE_FNO_DISABLE			0x00
#define HC_OPINT_STATE_RHSC_MAP_MASK		0x78
#define HC_ROOTHUB_STATUSCHANGED_PORT0_MASK	0x08
#define HC_ROOTHUB_STATUSCHANGED_PORT1_MASK	0x10
#define HC_ROOTHUB_STATUSCHANGED_PORT2_MASK	0x20
#define HC_ROOTHUB_STATUSCHANGED_PORT3_MASK	0x40

/*-------------------------------------------------
void HC_INTERRUPT_CONTROL_0x04()
*/
#define HC_INT_CONTROL_REG					0x04
#define HC_INT_CON_SOF_MASK					0x01
#define HC_INT_CON_SOF_ENABLE				0x01
#define HC_INT_CON_SOF_DISABLE				0x00
#define HC_INT_CON_ISTL0_MASK				0x02
#define HC_INT_CON_ISTL0_ENABLE				0x02
#define HC_INT_CON_ISTL0_DISABLE			0x00
#define HC_INT_CON_OPR_MASK					0x08
#define HC_INT_CON_OPR_ENABLE				0x08
#define HC_INT_CON_OPR_DISABLE				0x00
#define HC_INT_CON_SUS_MASK					0x10
#define HC_INT_CON_SUS_ENABLE				0x10
#define HC_INT_CON_SUS_DISABLE				0x00
#define HC_INT_CON_INTL_MASK				0x40
#define HC_INT_CON_INTL_ENABLE				0x40
#define HC_INT_CON_INTL_DISABLE				0x00
#define HC_INT_CON_AL_MASK					0x80
#define HC_INT_CON_ATL_ENABLE				0x80
#define HC_INT_CON_ATL_DISABLE				0x00

/*-------------------------------------------------
void HC_INTERRUPT_STATE_0x05()
*/
#define HC_INT_STATE_SOF_MASK				0x01
#define HC_INT_STATE_SOF_SET				0x01
#define HC_INT_STATE_SOF_DISABLE			0x00
#define HC_INT_STATE_ISTL0_MASK				0x02
#define HC_INT_STATE_ISTL0_SET				0x02
#define HC_INT_STATE_ISTL0_DISABLE			0x00
#define HC_INT_STATE_OPR_MASK				0x08
#define HC_INT_STATE_OPR_SET				0x08
#define HC_INT_STATE_OPR_DISABLE			0x00
#define HC_INT_STATE_SUS_MASK				0x10
#define HC_INT_STATE_SUS_SET				0x10
#define HC_INT_STATE_SUS_DISABLE			0x00
#define HC_INT_STATE_INTL_MASK				0x40
#define HC_INT_STATE_INTL_SET				0x40
#define HC_INT_STATE_INTL_DISABLE			0x00
#define HC_INT_STATE_ATL_MASK				0x80
#define HC_INT_STATE_ATL_SET				0x80
#define HC_INT_STATE_ATL_DISABLE			0x00

/*-------------------------------------------------
void HC_FMREMAINING_REG_0x08()
*/
#define HC_FMREMAINING_REG					0x08

/*-------------------------------------------------
void HC_FMNUMBER_REG_0x0A()
*/
#define HC_FMNUMBER_REG						0x0A

/*-------------------------------------------------
void HC_SOF_SEELCT_REG_0x0C()
*/
#define HC_SOF_SELECT_REG					0x0C
#define HC_SOF_SELECT_USPS					0x03
#define HC_SOF_SELECT_USPSE					0x04
#define HC_SOF_SELECT_USPSE_SET				0x04
#define HC_SOF_SELECT_USPSE_RESET			0x00

/*-------------------------------------------------
void HC_SOF_OFFSET_REG_0x0D()
*/
#define HC_SOF_OFFSET_REG					0x0D

/*-------------------------------------------------
void HC_ROOTHUB_STATE_0x12()
*/
#define HC_ROOTHUB0_STATE_REG				0x12
#define HC_ROOTHUB1_STATE_REG				0x14
#define HC_ROOTHUB2_STATE_REG				0x16
#define HC_ROOTHUB3_STATE_REG				0x18
#define HC_RH_STATE_CCS_MASK				0x01
#define HC_RH_STATE_CCS_SET					0x01
#define HC_RH_STATE_CCS_RESET				0x00
#define HC_RH_STATE_PES_MASK				0x02
#define HC_RH_STATE_PES_SET					0x02
#define HC_RH_STATE_PES_RESET				0x00
#define HC_RH_STATE_PSS_MASK				0x04
#define HC_RH_STATE_PSS_SET					0x04
#define HC_RH_STATE_PSS_RESET				0x00
#define HC_RH_STATE_POCI_MASK				0x08
#define HC_RH_STATE_POCI_SET				0x08
#define HC_RH_STATE_POCI_RESET				0x00
#define HC_RH_STATE_PRS_MASK				0x10
#define HC_RH_STATE_PRS_SET					0x10
#define HC_RH_STATE_PRS_RESET				0x00
#define HC_RH_STATE_PPS_MASK				0x20
#define HC_RH_STATE_PPS_SET					0x20
#define HC_RH_STATE_PPS_RESET				0x00
#define HC_RH_STATE_LSDA_MASK				0x40
#define HC_RH_STATE_LSDA_SHIFT				>> 6
#define HC_RH_STATE_LSDA_SET				0x40
#define HC_RH_STATE_LSDA_RESET				0x00
#define HC_RH_STATE_CHANGED_MASK			0x1f
#define HC_RH_STATE_NORMAL_CONNECT			0x23
#define HC_RH_CHANGE_CSC_MASK				0x01
#define HC_RH_CHANGE_CSC_SET				0x01
#define HC_RH_CHANGE_CSC_RESET				0x00
#define HC_RH_CHANGE_PESC_MASK				0x02
#define HC_RH_CHANGE_PESC_SET				0x02
#define HC_RH_CHANGE_PESC_RESET				0x00
#define HC_RH_CHANGE_PSSC_MASK				0x04
#define HC_RH_CHANGE_PSSC_SET				0x04
#define HC_RH_CHANGE_PSSC_RESET				0x00
#define HC_RH_CHANGE_POCIC_MASK				0x08
#define HC_RH_CHANGE_POCIC_SET				0x08
#define HC_RH_CHANGE_POCIC_RESET			0x00
#define HC_RH_CHANGE_PRSC_MASK				0x10
#define HC_RH_CHANGE_PRSC_SET				0x10
#define HC_RH_CHANGE_PRSC_RESET				0x00

/*-------------------------------------------------
void HC_ROOTHUB_CONTROL_0x1A()
*/
#define HC_ROOTHUB0_CON_REG					0x1A
#define HC_ROOTHUB1_CON_REG					0x1C
#define HC_ROOTHUB2_CON_REG					0x1E
#define HC_ROOTHUB3_CON_REG					0x20
#define HC_RH_CON_CPE_MASK					0x01
#define HC_RH_CON_CPE_SET					0x01
#define HC_RH_CON_CPE_RESET					0x00
#define HC_RH_CON_SPE_MASK					0x02
#define HC_RH_CON_SPE_SET					0x02
#define HC_RH_CON_SPE_RESET					0x00
#define HC_RH_CON_SPS_MASK					0x04
#define HC_RH_CON_SPS_SET					0x04
#define HC_RH_CON_SPS_RESET					0x00
#define HC_RH_CON_CPS_MASK					0x08
#define HC_RH_CON_CPS_SET					0x08
#define HC_RH_CON_CPS_RESET					0x00
#define HC_RH_CON_SPR_MASK					0x10
#define HC_RH_CON_SPR_SET					0x10
#define HC_RH_CON_SPR_RESET					0x00
#define HC_RH_CON_SPP_MASK					0x20
#define HC_RH_CON_SPP_SET					0x20
#define HC_RH_CON_SPP_RESET					0x00
#define HC_RH_CON_CPP__MASK					0x40
#define HC_RH_CON_CPP_SET					0x40
#define HC_RH_CON_CPP_RESET					0x00
#define HC_RH_CON_CCSC_MASK					0x01
#define HC_RH_CON_CCSC_SET					0x01
#define HC_RH_CON_CCSC_RESET				0x00
#define HC_RH_CON_CPESC_MASK				0x02
#define HC_RH_CON_CPESC_SET					0x02
#define HC_RH_CON_CPESC_RESET				0x00
#define HC_RH_CON_CPSSC_MASK				0x04
#define HC_RH_CON_CPSSC_SET					0x04
#define HC_RH_CON_CPSSC_RESET				0x00
#define HC_RH_CON_CPOCIC_MASK				0x08
#define HC_RH_CON_CPOCIC_SET				0x08
#define HC_RH_CON_CPOCIC_RESET				0x00
#define HC_RH_CON_CPRSC_MASK				0x10
#define HC_RH_CON_CPRSC_SET					0x10
#define HC_RH_CON_CPRSC_RESET				0x00

/*-------------------------------------------------
void HC_BUF_STATE_0x30()
*/
#define HC_BUF_STATE_REG					0x30
#define HC_BUF_STATE_ISTL0_MASK				0x01
#define HC_BUF_STATE_ISTL0_SET				0x01
#define HC_BUF_STATE_ISTL0_RSET				0x00
#define HC_BUF_STATE_ISTL1_MASK				0x02
#define HC_BUF_STATE_ISTL1_SET				0x02
#define HC_BUF_STATE_ISTL1_RSET				0x00
#define HC_BUF_STATE_INTLATV_MASK			0x04
#define HC_BUF_STATE_INTLATV_SET			0x04
#define HC_BUF_STATE_INTLATV_RSET			0x00
#define HC_BUF_STATE_ATLATV_MASK			0x08
#define HC_BUF_STATE_ATLATV_SET				0x08
#define HC_BUF_STATE_ATLATV_RSET			0x00
#define HC_BUF_STATE_ISTL0ATV_MASK			0x10
#define HC_BUF_STATE_ISTL0ATV_SET			0x10
#define HC_BUF_STATE_ISTL0ATV_RSET			0x00
#define HC_BUF_STATE_ISTL1ATV_MASK			0x20
#define HC_BUF_STATE_ISTL1ATV_SET			0x20
#define HC_BUF_STATE_ISTL1ATV_RSET			0x00
#define HC_BUF_STATE_ISTL0BD_MASK			0x40
#define HC_BUF_STATE_ISTL0BD_SET			0x40
#define HC_BUF_STATE_ISTL0BD_RSET			0x00
#define HC_BUF_STATE_ISTL1BD_MASK			0x80
#define HC_BUF_STATE_ISTL1BD_SET			0x80
#define HC_BUF_STATE_ISTL1BD_RSET			0x00

/*-------------------------------------------------
void HC_BUF_START_ADDR_0x31()
*/
#define HC_BUF_START_ADDR_REG				0x31
#define HC_BSA_MASK							0xff

/*-------------------------------------------------
void HC_ISTL_BUF_SIZE_0x32()
*/
#define HC_ISTL_BUF_SIZE_REG				0x32
#define HC_ISTLBS0_MASK						0xff
#define HC_ISTLBS1_MASK						0xff

/*-------------------------------------------------
void HC_ISTL_BLOCK_SIZE_0x34()
*/
#define HC_ISTL_BLK_SIZE_REG				0x34
#define HC_ISTLBLK0_MASK					0xff
#define HC_ISTLBLK1_MASK					0x03

/*-------------------------------------------------
void HC_ISTL_TD_DONE_MAP_0x36()
*/
#define HC_ISTL_TDDONE_MAP_REG				0x36
#define HC_ISTLTDDM0_MASK					0xff

/*-------------------------------------------------
void HC_ISTL_TD_SKIP_MAP_0x37()
*/
#define HC_ISTL_TDSKIP_MAP_REG				0x37
#define HC_ISTLTDSM_MASK					0xff

/*-------------------------------------------------
void HC_ISTL_TD_LAST_MAP_0x38()
*/
#define HC_ISTL_TDLAST_MAP_REG				0x38
#define HC_ISTLLTD_MASK						0xff

/*-------------------------------------------------
void HC_ISTLTDR_0x3a()
*/
#define HC_ISTL_TD_RELOAD_REG				0x3a

/*-------------------------------------------------
void HC_INTL_BUF_SIZE_0x40()
*/
#define HC_INTL_BUF_SIZE_REG				0x40
#define HC_INTLBS0_MASK						0xff
#define HC_INTLBS1_MASK						0xff

/*-------------------------------------------------
void HC_INTL_BLOCK_SIZE_0x42()
*/
#define HC_INTL_BLK_SIZE_REG				0x42
#define HC_INTLBLK0_MASK					0xff
#define HC_INTLBLK1_MASK					0x03

/*-------------------------------------------------
void HC_INTL_TD_DONE_MAP_0x44()
*/
#define HC_INTL_TDDONE_MAP_REG				0x44
#define HC_INTLTDDM0_MASK					0xff
#define HC_INTLTDDM1_MASK					0xff
#define HC_INTLTDDM2_MASK					0xff
#define HC_INTLTDDM3_MASK					0xff

/*-------------------------------------------------
void HC_INTL_TD_SKIP_MAP_0x48()
*/
#define HC_INTL_TDSKIP_MAP_REG				0x48
#define HC_INTLTDSM0_MASK					0xff
#define HC_INTLTDSM1_MASK					0xff
#define HC_INTLTDSM2_MASK					0xff
#define HC_INTLTDSM3_MASK					0xff

/*-------------------------------------------------
void HC_INTL_TD_LAST_MAP_0x4C()
*/
#define HC_INTL_TDLAST_MAP_REG				0x4C
#define HC_INTLLTD0_MASK					0xff
#define HC_INTLLTD1_MASK					0xff
#define HC_INTLLTD2_MASK					0xff
#define HC_INTLLTD3_MASK					0xff

/*-------------------------------------------------
void HC_INTL_TD_ACTIVE_ID_0x50()
*/
#define HC_INTL_CATD_REG					0x50
#define HC_INTLCATD_MASK					0x1f

/*-------------------------------------------------
void HC_INTLTDR_0x52()
*/
#define HC_INT_TD_RELOAD_REG				0x52

/*-------------------------------------------------
void HC_ATL_BUF_SIZE_0x60()
*/
#define HC_ATL_BUF_SIZE_REG					0x60
#define HC_ATLBS0_MASK						0xff
#define HC_ATLBS1_MASK						0xff

/*-------------------------------------------------
void HC_ATL_BLOCK_SIZE_0x62 0x64()
*/
#define HC_ATL_BLK_SIZE0_REG				0x62
#define HC_ATL_BLK_SIZE1_REG				0x64
#define HC_ATLBLK0_MASK						0xff
#define HC_ATLBLK1_MASK						0x03

/*-------------------------------------------------
void HC_ATL_BLOCK_SIZE_INDEX_MAP_0x66()
*/
#define HC_ATL_BLK_SIZE_INDEX_MAP_REG		0x66

/*-------------------------------------------------
void HC_ATL_TD_DONE_MAP_0x6A()
*/
#define HC_ATL_TDDONE_MAP_REG				0x6a
#define HC_ATLTDDM0_MASK					0xff
#define HC_ATLTDDM1_MASK					0xff
#define HC_ATLTDDM2_MASK					0xff
#define HC_ATLTDDM3_MASK					0xff

/*-------------------------------------------------
void HC_ATL_TD_SKIP_MAP_0x6E()
*/
#define HC_ATL_TDSKIP_MAP_REG				0x6e
#define HC_ATLTDSM0_MASK					0xff
#define HC_ATLTDSM1_MASK					0xff
#define HC_ATLTDSM2_MASK					0xff
#define HC_ATLTDSM3_MASK					0xff

/*-------------------------------------------------
void HC_ATL_TD_LAST_MAP_0x72()
*/
#define HC_ATL_TDLAST_MAP_REG				0x72
#define HC_ATLLTD0_MASK						0xff
#define HC_ATLLTD1_MASK						0xff
#define HC_ATLLTD2_MASK						0xff
#define HC_ATLLTD3_MASK						0xff

/*-------------------------------------------------
void HC_ATL_TD_ACTIVE_ID_0x76()
*/
#define HC_ATL_CATD_REG						0x76
#define HC_ATLCATD_MASK						0x1f

/*-------------------------------------------------
void HC_ATL_TD_DONE_THRESHOLD_COUNT_0x77()
*/
#define HC_ATL_TD_DONE_COUNT_REG			0x77
#define HC_ATL_TD_DONE_COUNT_MASK			0x3f

/*-------------------------------------------------
void HC_ATL_TD_DONE_THRESHOLD_TIMEOUT_0x78()
*/
#define HC_ATL_TD_DONE_TIMEOUT_REG			0x78
#define HC_ATL_TD_DONE_TIMEOUT_MASK			0xff

/* GLOBAL VARIABLES */
#define _USBHC_Reset()				(HCDR=HC_SWR_ENABLE;HCCIR=HC_CONTROL_REG)
#define _USBHC_LocalReset()			(HCDR=HC_HCR_ENABLE;HCCIR=HC_CMD_STATUS_REG)
#define _USBHC_HCDR_READ_SFR(reg)	(reg=HCDR)


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
extern const U8_T		USBHC_RootHub_Cmd_Reg[USB_HC_MAX_PORT];
extern const U8_T		USBHC_RootHub_Status_Reg[USB_HC_MAX_PORT];
extern const U8_T		HC_ROOTHUB_STATE_REG[USB_HC_MAX_PORT];


/* EXPORTED GLOBAL VARIABLES */
#if (USBHC_INIT_DEBUG_MODE)
void USBHC_Reg_Dump(U8_T hccir,U8_T length);
#endif
void USBHC_Read_Regs(U8_T hccir,U8_T *hcdr,U8_T len);
void USBHC_Write_Regs(U8_T hccir,U8_T *hcdr,U8_T len);
#endif /* End of __USBHC_REGISTER_H__ */

/* End of usbdc_regs.h */