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
 * Module Name: usbdc_regs.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __USBDC_REGS_H__
#define __USBDC_REGS_H__
 
/* INCLUDE FILE DECLARATIONS */
/* DEFINATION DECLARATIONS */
/*
void DAyEPxCFGR_0x00_0x7F()
*/
#define   DA0EP0CFGR	0x00 
#define   DA1EP0CFGR	0x10 
#define   DA2EP0CFGR	0x20 
#define   DA3EP0CFGR	0x30 
#define   DA4EP0CFGR	0x40 
#define   DA5EP0CFGR	0x50 
#define   DA6EP0CFGR	0x60 
#define   DA7EP0CFGR	0x70 

/* Byte 1 */
#define   DA_EP_CFGR_EPID_MASK		0x0f
#define   DA_EP_CFGR_DIR_MASK		0x10
#define   DA_EP_CFGR_DIR_OUT		0x00
#define   DA_EP_CFGR_DIR_IN			0x10

#define   DA_EP_CFGR_DBUF_MASK		0x20
#define   DA_EP_CFGR_DBUF_RESET		0x00
#define   DA_EP_CFGR_DBUF_SET		0x20

#define   DA_EP_CFGR_ISOCH_MASK		0x40
#define   DA_EP_CFGR_ISOCH_RESET	0x00
#define   DA_EP_CFGR_ISOCH_SET		0x40

#define   DA_EP_CFGR_EPEN_MASK		0x80
#define   DA_EP_CFGR_EPEN_RESET		0x00
#define   DA_EP_CFGR_EPEN_SET		0x80

/* Byte 2 */
#define  ENDP_PKT_SIZE_MASK			0x03
#define  ENDP_PKT_SIZE_8			0x00
#define  ENDP_PKT_SIZE_16			0x01
#define  ENDP_PKT_SIZE_32			0x02
#define  ENDP_PKT_SIZE_64			0x03

#define  ISO_ENDP_PKT_SIZE_16		0x00
#define  ISO_ENDP_PKT_SIZE_64		0x01
#define  ISO_ENDP_PKT_SIZE_128		0x02
#define  ISO_ENDP_PKT_SIZE_192		0x03

/*
$BULK BURST Mode
2014-01-06
*/
#define   DA_EP_CFGR_BUREN_MASK		0x04
#define   DA_EP_CFGR_BUREN_RESET	0x00
#define   DA_EP_CFGR_BUREN_SET		0x04

/*
void DAyIDCFGR_0x80_0x87()
*/
#define   DA0IDCFGR		0x80
#define   DA1IDCFGR		0x81
#define   DA2IDCFGR		0x82
#define   DA3IDCFGR		0x83
#define   DA4IDCFGR		0x84
#define   DA5IDCFGR		0x85
#define   DA6IDCFGR		0x86
#define   DA7IDCFGR		0x87

#define   DA_ID_CFGR_DEVADR_MASK	0x7f
#define   DA_ID_CFGR_ADRACT_MASK	0x80
#define   DA_ID_CFGR_ADRACT_RESET	0x00
#define   DA_ID_CFGR_ADRACT_SET		0x80

/*
void INDBASR_0x88_0x8A()
*/
#define  INDBASR	0x88
/*
void DAyCR_0x90_0x97()
*/
#define  DA0CR		0x90
#define  DA1CR		0x91
#define  DA2CR		0x92
#define  DA3CR		0x93
#define  DA4CR		0x94
#define  DA5CR		0x95
#define  DA6CR		0x96
#define  DA7CR		0x97

/*Bit Define*/
#define   DA_CR_EPIND_MASK 				0x07

#define   DA_CR_SCLR_MASK				0x08
#define   DA_CR_SCLR_RESET				0x00
#define   DA_CR_SCLR_SET				0x08

#define   DA_CR_BVLD_MASK				0x10
#define   DA_CR_BVLD_RESET				0x00
#define   DA_CR_BVLD_SET				0x10

#define   DA_CR_BCLR_MASK				0x20
#define   DA_CR_BCLR_RESET				0x00
#define   DA_CR_BCLR_SET				0x20

#define   DA_CR_STALL_MASK				0x40
#define   DA_CR_STALL_RESET				0x00
#define   DA_CR_STALL_SET				0x40

#define   DA_CR_IN_CLEAR				0x80
#define   DA_CR_BURST_ABORT				0x80

#define   DA_CR_ENPX_IN_NAK_MASK		0x80
#define   DA_CR_ENPX_IN_NAK_RESET		0x00
#define   DA_CR_ENPX_IN_NAK_SET			0x80

/*
void DCCR_0x98()
*/
#define DCCR				0x98

#define DC_CR_RST_MASK		0x01
#define DC_CR_RST_RESET		0x00
#define DC_CR_RST_SET		0x01

#define DC_CR_SRSM_MASK		0x02
#define DC_CR_SRSM_RESET	0x00
#define DC_CR_SRSM_SET		0x02

#define DC_CR_SUSP_MASK		0x04
#define DC_CR_SUSP_RESET	0x00
#define DC_CR_SUSP_SET		0x04

#define DC_CR_RESUM_MASK	0x08
#define DC_CR_RESUM_RESET	0x00
#define DC_CR_RESUM_SET		0x08

#define DC_CR_VBUS_MASK		0x10
#define DC_CR_URST_MASK		0x20
#define DC_CR_FSE0_MASK		0x40

#define DC_CR_FSE0_RESET	0x00
#define DC_CR_FSE0_SET		0x40

#define DC_CR_DCEN_MASK		0x80
#define DC_CR_DCEN_RESET	0x00
#define DC_CR_DCEN_SET		0x80 

/*
void DCIER_0x99()
*/
#define   DCIER						0x99

#define   DC_IER_SOF_MASK			0x01
#define   DC_IER_SOF_RESET			0x00
#define   DC_IER_SOF_SET			0x01

#define   DC_IER_ESOF_MASK			0x02
#define   DC_IER_ESOF_RESET			0x00
#define   DC_IER_ESOF_SET			0x02

#define   DC_IER_URST_MASK			0x04
#define   DC_IER_URST_RESET			0x00
#define   DC_IER_URST_SET			0x04

#define   DC_IER_VCHG_MASK			0x08
#define   DC_IER_VCHG_RESET			0x00
#define   DC_IER_VCHG_SET			0x08

#define   DC_IER_USUS_MASK			0x10
#define   DC_IER_USUS_RESET			0x00
#define   DC_IER_USUS_SET			0x10

#define   DC_IER_RSM_MASK			0x20
#define   DC_IER_RSM_RESET			0x00
#define   DC_IER_RSM_SET			0x20

#define   DC_IER_ERR_MASK			0x40
#define   DC_IER_ERR_RESET			0x00
#define   DC_IER_ERR_SET			0x40

#define   DC_IER_CT_MASK			0x80
#define   DC_IER_CT_RESET			0x00
#define   DC_IER_CT_SET				0x80

/*-------------------------------------------------
void DCIEPPR_0x9A()
*/
#define	  DCIEPPR					0x9a

#define	  DC_IEPPR_ENDP_MASK		0x07
#define	  DC_IEPPR_DVID_MASK		0x70
#define	  DC_IEPPR_ENABLE_SET		0x80
#define	  DC_IEPPR_DISABLE_SET		0x00

/*
void FMR_0x9E()
*/
#define   DCFMR 					0x9E
#define   DC_FMR_LCK_MASK			0x20

/*-------------------------------------------------
void UDCSR_0x9f()
*/  
//sfr UDCSR	= 0x9f;


#define		UDC_PORT0_ACT			0x01
#define		UDC_PORT1_ACT			0x02
#define		UDC_PORT2_ACT			0x04
#define		UDC_PORT3_ACT			0x08

#define		UDC_PORT_ALL_ACT		0x0F

/*-------------------------------------------------
void DAyEPzSR_0xA0()
*/

#define   DA_EP_CBUF				0x80
#define   DA_EP_BAVA1				0x40
#define   DA_EP_BAVA0				0x20
#define   DA_EP_IN_NAKE				0x10
#define   DA_EP_STALL				0x08
#define   DA_EP_SSCOM				0x04	//2013-05-13 add
#define   DA_EP_SETOW				0x02
#define   DA_EP_SETUP				0x01

/*-------------------------------------------------
void DCISR_0xA3() // This is SFR Address
*/

#define   DC_ISR_SOF_MASK			0x01
#define   DC_ISR_SOF_RESET			0x00
#define   DC_ISER_SOF_SET			0x01

#define   DC_ISR_ESOF_MASK			0x02
#define   DC_ISR_ESOF_RESET			0x00
#define   DC_ISR_ESOF_SET			0x02

#define   DC_ISR_URST_MASK			0x04
#define   DC_ISR_URST_RESET			0x00
#define   DC_ISR_URST_SET			0x04

#define   DC_ISR_VCHG_MASK			0x08
#define   DC_ISR_VCHG_RESET			0x00
#define   DC_ISR_VCHG_SET			0x08

#define   DC_ISR_USUS_MASK			0x10
#define   DC_ISR_USUS_RESET			0x00
#define   DC_ISR_USUS_SET			0x10

#define   DC_ISR_RWK_MASK			0x20
#define   DC_ISR_RWK_RESET			0x00
#define   DC_ISR_RWk_SET			0x20

#define   DC_ISR_ERR_MASK			0x40
#define   DC_ISR_ERR_RESET			0x00
#define   DC_ISR_ERR_SET			0x40

#define   DC_ISR_CT_MASK			0x80
#define   DC_ISR_CT_RESET			0x00
#define   DC_ISR_CT_SET				0x80

/*-------------------------------------------------
void DCINSR_0xA4() // This is SFR Address
*/
 
#define   DC_INSR_EPIND_MASK		0x07
#define   DC_INSR_EPIND_SHFIT		

#define   DC_INSR_DIR_MASK			0x08
#define   DC_INSR_DIR_OUT			0x00
#define   DC_INSR_DIR_IN			0x08

#define   DC_INSR_DEVIND_MASK		0x70
#define   DC_INSR_DEVIND_SHIFT		>> 4

/*-------------------------------------------------
void DCIESMR_0xA5() // This is SFR Address
*/

#define   DC_IESMR_CBUF				0x80
#define   DC_IESMR_BAVA1			0x40
#define   DC_IESMR_BAVA0			0x20
#define   DC_IESMR_IN_NAK			0x10	//2013-07-23 add
#define   DC_IESMR_STALL			0x08
#define   DC_IESMR_SSCOM			0x04	//2013-05-13 add
#define   DC_IESMR_SETOW			0x02
#define   DC_IESMR_SETUP			0x01

/*-------------------------------------------------
void DAyINAKER_0xE0()
*/
#define   DAyINAKE					0xe0



#define _USBDC_DC0DR_SFR(DC0DR_Value)			(DC0DR=DC0DR_Value)
#define _USBDC_DC0CIR_SFR(DC0CIR_Value)			(DC0CIR=DC0CIR_Value)
#define _USBDC_DC0DR_READ_SFR(DC0DR_Value)		(DC0DR_Value=DC0DR)

#define _USBDC_DC1DR_SFR(DC1DR_Value)			(DC1DR=DC1DR_Value)
#define _USBDC_DC1CIR_SFR(DC1CIR_Value)			(DC1CIR=DC1CIR_Value)
#define _USBDC_DC1DR_READ_SFR(DC1DR_Value)		(DC1DR_Value=DC1DR)

#define _USBDC_DC2DR_SFR(DC2DR_Value)			(DC2DR=DC2DR_Value)
#define _USBDC_DC2CIR_SFR(DC2CIR_Value)			(DC2CIR=DC2CIR_Value)
#define _USBDC_DC2DR_READ_SFR(DC2DR_Value)		(DC2DR_Value=DC2DR)

#define _USBDC_DC3DR_SFR(DC3DR_Value)			(DC3DR=DC3DR_Value)
#define _USBDC_DC3CIR_SFR(DC3CIR_Value)			(DC3CIR=DC3CIR_Value)
#define _USBDC_DC3DR_READ_SFR(DC3DR_Value)		(DC3DR_Value=DC3DR) 

/* NAMING CONSTANT DECLARATIONS */
/* GLOBAL VARIABLES */

extern const U8_T	DA_ID_CFGR[USBDC_DEVICE_MAX];
extern const U8_T	DA_EP_SR[USBDC_DEVICE_MAX][USBDC_ENDP_MAX];
extern const U8_T	DA_EP_CFGR[USBDC_DEVICE_MAX];
extern const U8_T	DA_CR[USBDC_DEVICE_MAX];
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
/* EXPORTED GLOBAL VARIABLES */ 
void USBDC_Write_Regs(U8_T port,U8_T dccir,U8_T *dcdr,U8_T len);
void USBDC_Read_Regs(U8_T port,U8_T dccir,U8_T *dcdr,U8_T len);
U8_T USBDC_REGS_Get_Pkt_Size(U8_T ep_iso,U16_T epsize);
void USBDC_Reg_Dump(U8_T port,U8_T address,U8_T length);
void USBDC_REGS_Endp_ControlClear(U8_T port,U8_T devinx,U8_T endpinx,U8_T flag);
void USBDC_REGS_Endp_ControlSet(U8_T port,U8_T devinx,U8_T endpinx,U8_T flag);
void USBDC_REGS_Address_Active(U8_T port,U8_T devinx,FlagStatus state);
void USBDC_REGS_Address_Write(U8_T port,U8_T devinx,U8_T address);
void USBDC_REGS_Endp_Reset(U8_T port,U8_T devinx,U8_T endpinx);
void USBDC_REGS_Device_Reset(U8_T port,U8_T devinx);
void USBDC_HAL_Port_Resume(U8_T port);
void USBDC_HAL_Port_Suspend(U8_T port);
U8_T USBDC_HAL_Port_Check_SOF(U8_T port);
void USBDC_EPCFGR_Write(U8_T port,U8_T devinx,U8_T ep,U8_T reg0,U8_T reg1);
#endif /* End of __USBDC_REGS_H__ */

/* End of usbdc_main.h */