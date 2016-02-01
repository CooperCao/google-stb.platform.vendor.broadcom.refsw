/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BSTD_IDS_H__
#define BSTD_IDS_H__

#include "bstd_defs.h"
#include "berr_ids.h"

/* The standard ID. This ID is present when the standard error codes are
   being used without a module specific ID */
#define BSTD_STD_ID    BERR_STD_ID

/* The application ID. This ID is reserved for use by top level applications
   so they can extend our error codes if they wish. This ID cannot be used
   in any module or library */
#define BSTD_APP_ID   BERR_APP_ID

#define BSTD_MODULE_ID_START   BERR_AUD_ID

#if 1
#define BSTD_AUD_ID   BERR_AUD_ID
#define BSTD_HDM_ID   BERR_HDM_ID
#define BSTD_ICM_ID   BERR_ICM_ID
#define BSTD_IFD_ID   BERR_IFD_ID
#define BSTD_MVD_ID   BERR_MVD_ID
#define BSTD_QDS_ID   BERR_QDS_ID
#define BSTD_QOB_ID   BERR_QOB_ID
#define BSTD_QUS_ID   BERR_QUS_ID
#define BSTD_RDC_ID   BERR_RDC_ID
#define BSTD_RFM_ID   BERR_RFM_ID
#define BSTD_SCD_ID   BERR_SCD_ID
#define BSTD_TNR_ID   BERR_TNR_ID
#define BSTD_VBI_ID   BERR_VBI_ID
#define BSTD_VDC_ID   BERR_VDC_ID
#define BSTD_XPT_ID   BERR_XPT_ID
#define BSTD_I2C_ID   BERR_I2C_ID
#define BSTD_SPI_ID   BERR_SPI_ID
#define BSTD_ICP_ID   BERR_ICP_ID
#define BSTD_IRB_ID   BERR_IRB_ID
#define BSTD_KIR_ID   BERR_KIR_ID
#define BSTD_KPD_ID   BERR_KPD_ID
#define BSTD_LED_ID   BERR_LED_ID
#define BSTD_PWM_ID   BERR_PWM_ID
#define BSTD_URT_ID   BERR_URT_ID
#define BSTD_SDS_ID   BERR_SDS_ID
#define BSTD_VSB_ID   BERR_VSB_ID
#define BSTD_ENC_ID   BERR_ENC_ID
#define BSTD_DMA_ID   BERR_DMA_ID
#define BSTD_GIO_ID   BERR_GIO_ID
#define BSTD_LNA_ID   BERR_LNA_ID
#define BSTD_GRC_ID   BERR_GRC_ID
#define BSTD_P3D_ID   BERR_P3D_ID
#define BSTD_XVD_ID   BERR_XVD_ID
#define BSTD_ARC_ID   BERR_ARC_ID
#define BSTD_RAP_ID   BERR_RAP_ID
#define BSTD_MRC_ID   BERR_MRC_ID
#define BSTD_AST_ID   BERR_AST_ID
#define BSTD_TMR_ID   BERR_TMR_ID
#define BSTD_RPC_ID   BERR_RPC_ID
#define BSTD_MEM_ID   BERR_MEM_ID
#define BSTD_INT_ID   BERR_INT_ID
#define BSTD_ADS_ID   BERR_ADS_ID
#define BSTD_AOB_ID   BERR_AOB_ID
#define BSTD_AUS_ID   BERR_AUS_ID
#define BSTD_XCD_ID   BERR_XCD_ID
#define BSTD_XCU_ID   BERR_XCU_ID
#define BSTD_NET_ID   BERR_NET_ID
#define BSTD_USB_ID   BERR_USB_ID
#define BSTD_ATA_ID   BERR_ATA_ID
#define BSTD_UHF_ID   BERR_UHF_ID
#define BSTD_UPG_ID   BERR_UPG_ID
#define BSTD_MDM_ID   BERR_MDM_ID
#define BSTD_VIF_ID   BERR_VIF_ID
#define BSTD_AST0_ID  BERR_AST_ID
#define BSTD_AST1_ID  BERR_AST1_ID
#define BSTD_FTM_ID   BERR_FTM_ID
#define BSTD_QPSK_ID  BERR_QPSK_ID
#define BSTD_CTK_ID   BERR_CTK_ID
#define BSTD_TMN_ID   BERR_TMN_ID
#define BSTD_TNF_ID   BERR_TNF_ID
#define BSTD_ANA_ID   BERR_ANA_ID
#define BSTD_ANV_ID   BERR_ANV_ID
#define BSTD_HDR_ID   BERR_HDR_ID
#define BSTD_LVD_ID   BERR_LVD_ID
#define BSTD_SID_ID   BERR_SID_ID
#define BSTD_TFE_ID   BERR_TFE_ID
#define BSTD_V3D_ID   BERR_V3D_ID


/* syslibs */
#define BSTD_TNRlib_ID   BERR_TNRlib_ID	
#define BSTD_VBIlib_ID   BERR_VBIlib_ID	
#define BSTD_BTSlib_ID   BERR_BTSlib_ID	

/* common utitlies */

#else
/* porting interfaces and common utilities */
#define BSTD_ADS_ID            BSTD_MODULE_ID_START + 0
#define BSTD_AOB_ID            BSTD_MODULE_ID_START + 1
#define BSTD_ARC_ID            BSTD_MODULE_ID_START + 2
#define BSTD_AST_ID            BSTD_MODULE_ID_START + 3
#define BSTD_AUD_ID            BSTD_MODULE_ID_START + 4    
#define BSTD_AUS_ID            BSTD_MODULE_ID_START + 5
#define BSTD_DMA_ID            BSTD_MODULE_ID_START + 6
#define BSTD_ENC_ID            BSTD_MODULE_ID_START + 7
#define BSTD_GIO_ID            BSTD_MODULE_ID_START + 8
#define BSTD_GRC_ID            BSTD_MODULE_ID_START + 9
#define BSTD_HDM_ID            BSTD_MODULE_ID_START + 10     
#define BSTD_I2C_ID            BSTD_MODULE_ID_START + 11
#define BSTD_ICM_ID            BSTD_MODULE_ID_START + 12     
#define BSTD_ICP_ID            BSTD_MODULE_ID_START + 13
#define BSTD_IFD_ID            BSTD_MODULE_ID_START + 14    
#define BSTD_INT_ID            BSTD_MODULE_ID_START + 15
#define BSTD_IRB_ID            BSTD_MODULE_ID_START + 16
#define BSTD_KIR_ID            BSTD_MODULE_ID_START + 17
#define BSTD_KPD_ID            BSTD_MODULE_ID_START + 18
#define BSTD_LED_ID            BSTD_MODULE_ID_START + 19
#define BSTD_LNA_ID            BSTD_MODULE_ID_START + 20
#define BSTD_MVD_ID            BSTD_MODULE_ID_START + 21
#define BSTD_MEM_ID            BSTD_MODULE_ID_START + 22
#define BSTD_MRC_ID            BSTD_MODULE_ID_START + 23
#define BSTD_P3D_ID            BSTD_MODULE_ID_START + 24
#define BSTD_PWM_ID            BSTD_MODULE_ID_START + 25
#define BSTD_QDS_ID            BSTD_MODULE_ID_START + 26
#define BSTD_QOB_ID            BSTD_MODULE_ID_START + 27
#define BSTD_QUS_ID            BSTD_MODULE_ID_START + 28
#define BSTD_RAP_ID            BSTD_MODULE_ID_START + 29    
#define BSTD_RDC_ID            BSTD_MODULE_ID_START + 30
#define BSTD_RPC_ID            BSTD_MODULE_ID_START + 31
#define BSTD_RFM_ID            BSTD_MODULE_ID_START + 32
#define BSTD_SCD_ID            BSTD_MODULE_ID_START + 33
#define BSTD_SDS_ID            BSTD_MODULE_ID_START + 34
#define BSTD_SPI_ID            BSTD_MODULE_ID_START + 35
#define BSTD_TMR_ID            BSTD_MODULE_ID_START + 36
#define BSTD_TNR_ID            BSTD_MODULE_ID_START + 37
#define BSTD_URT_ID            BSTD_MODULE_ID_START + 38
#define BSTD_VBI_ID            BSTD_MODULE_ID_START + 39
#define BSTD_VDC_ID            BSTD_MODULE_ID_START + 40
#define BSTD_VSB_ID            BSTD_MODULE_ID_START + 41
#define BSTD_XCD_ID            BSTD_MODULE_ID_START + 42
#define BSTD_XCU_ID            BSTD_MODULE_ID_START + 43
#define BSTD_XPT_ID            BSTD_MODULE_ID_START + 44  
#define BSTD_XVD_ID 	       BSTD_MODULE_ID_START + 45
#define BSTD_CTK_ID 	       BSTD_MODULE_ID_START + 46
#define BSTD_TMON_ID 	       BSTD_MODULE_ID_START + 47

/* syslibs */
#define BSTD_LIB_ID_START      0x100
#define BSTD_BTSlib_ID         BSTD_LIB_ID_START + 0
#define BSTD_TNRlib_ID         BSTD_LIB_ID_START + 1
#define BSTD_VBIlib_ID         BSTD_LIB_ID_START + 2
#endif

#define BSTD_Module            uint32_t


#endif /* #ifndef BSTD_IDS_H__ */

/* end of file */
