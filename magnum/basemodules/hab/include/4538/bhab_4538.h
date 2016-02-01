/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BHAB_4538_H
#define BHAB_4538_H

#ifdef __cplusplus
extern "C" {
#endif


#include "bhab.h"


/* bondout0 options (otp row 9) */
#define BHAB_4538_BONDOUT0_CHIP_VARIANT           0x0000000F
#define BHAB_4538_BONDOUT0_CHIP_ID                0x003FFFC0

/* bondout1 options (otp row 10) */
#define BHAB_4538_BONDOUT1_DISABLE_INPUT          0x00F00000
#define BHAB_4538_BONDOUT1_DISABLE_CHAN           0x000FF000
#define BHAB_4538_BONDOUT1_DISABLE_FSK_DTV        0x00000800
#define BHAB_4538_BONDOUT1_DISABLE_TURBO          0x00000400
#define BHAB_4538_BONDOUT1_DISABLE_LDPC           0x00000200
#define BHAB_4538_BONDOUT1_DISABLE_DTV            0x00000100
#define BHAB_4538_BONDOUT1_DISABLE_TS_CTRL        0x00000080
#define BHAB_4538_BONDOUT1_DISABLE_TS             0x00000040
#define BHAB_4538_BONDOUT1_DISABLE_LEG_MTSIF_SYNC 0x00000020
#define BHAB_4538_BONDOUT1_DISABLE_LEG_MTSIF      0x00000010

/* interrupt bits in LEAP_HOST_IRQ_STATUS0 */
#define BHAB_4538_HIRQ0_INIT_DONE            0x00000001
#define BHAB_4538_HIRQ0_HAB_DONE             0x00000002
#define BHAB_4538_HIRQ0_HAB_ERROR            0x00000004 /* look at SPARE3[3:0] for error code */
#define BHAB_4538_HIRQ0_FW_ERROR             0x00000008 /* look at SPARE3[7:4] for error code */
#define BHAB_4538_HIRQ0_DISEQC_DONE          0x00000010
#define BHAB_4538_HIRQ0_DISEQC_OVERVOLTAGE   0x00000020
#define BHAB_4538_HIRQ0_DISEQC_UNDERVOLTAGE  0x00000040
#define BHAB_4538_HIRQ0_FSK_MSG_READY        0x00000080
#define BHAB_4538_HIRQ0_FSK_ERROR            0x00000100
#define BHAB_4538_HIRQ0_FSK_TX_BUF_AVAILABLE 0x00000200
#define BHAB_4538_HIRQ0_FLASH_DONE           0x00000400
#define BHAB_4538_HIRQ0_WFE_SA_DONE          0x00000800
#define BHAB_4538_HIRQ0_WFE0_READY           0x00001000
#define BHAB_4538_HIRQ0_WFE1_READY           0x00002000
#define BHAB_4538_HIRQ0_WFE2_READY           0x00004000
#define BHAB_4538_HIRQ0_WFE3_READY           0x00008000

#define BHAB_4538_HIRQ0_AST_MASK             0x000003F0
#define BHAB_4538_HIRQ0_WFE_MASK             0x0000F800
#define BHAB_4538_HIRQ0_FSK_MASK             0x00000380

/* interrupt bits in LEAP_HOST_IRQ_STATUS1 */
#define BHAB_4538_HIRQ1_CHAN0_LOCK_CHANGE    0x00000001
#define BHAB_4538_HIRQ1_CHAN0_STATUS_EVENT   0x00000002
#define BHAB_4538_HIRQ1_CHAN0_RESERVED       0x00000004
#define BHAB_4538_HIRQ1_CHAN0_PEAK_SCAN_DONE 0x00000008
#define BHAB_4538_HIRQ1_CHAN1_LOCK_CHANGE    0x00000010
#define BHAB_4538_HIRQ1_CHAN1_STATUS_EVENT   0x00000020
#define BHAB_4538_HIRQ1_CHAN1_RESERVED       0x00000040
#define BHAB_4538_HIRQ1_CHAN1_PEAK_SCAN_DONE 0x00000080
#define BHAB_4538_HIRQ1_CHAN2_LOCK_CHANGE    0x00000100
#define BHAB_4538_HIRQ1_CHAN2_STATUS_EVENT   0x00000200
#define BHAB_4538_HIRQ1_CHAN2_RESERVED       0x00000400
#define BHAB_4538_HIRQ1_CHAN2_PEAK_SCAN_DONE 0x00000800
#define BHAB_4538_HIRQ1_CHAN3_LOCK_CHANGE    0x00001000
#define BHAB_4538_HIRQ1_CHAN3_STATUS_EVENT   0x00002000
#define BHAB_4538_HIRQ1_CHAN3_RESERVED       0x00004000
#define BHAB_4538_HIRQ1_CHAN3_PEAK_SCAN_DONE 0x00008000
#define BHAB_4538_HIRQ1_CHAN4_LOCK_CHANGE    0x00010000
#define BHAB_4538_HIRQ1_CHAN4_STATUS_EVENT   0x00020000
#define BHAB_4538_HIRQ1_CHAN4_RESERVED       0x00040000
#define BHAB_4538_HIRQ1_CHAN4_PEAK_SCAN_DONE 0x00080000
#define BHAB_4538_HIRQ1_CHAN5_LOCK_CHANGE    0x00100000
#define BHAB_4538_HIRQ1_CHAN5_STATUS_EVENT   0x00200000
#define BHAB_4538_HIRQ1_CHAN5_RESERVED       0x00400000
#define BHAB_4538_HIRQ1_CHAN5_PEAK_SCAN_DONE 0x00800000
#define BHAB_4538_HIRQ1_CHAN6_LOCK_CHANGE    0x01000000
#define BHAB_4538_HIRQ1_CHAN6_STATUS_EVENT   0x02000000
#define BHAB_4538_HIRQ1_CHAN6_RESERVED       0x04000000
#define BHAB_4538_HIRQ1_CHAN6_PEAK_SCAN_DONE 0x08000000
#define BHAB_4538_HIRQ1_CHAN7_LOCK_CHANGE    0x10000000
#define BHAB_4538_HIRQ1_CHAN7_STATUS_EVENT   0x20000000
#define BHAB_4538_HIRQ1_CHAN7_RESERVED       0x40000000
#define BHAB_4538_HIRQ1_CHAN7_PEAK_SCAN_DONE 0x80000000

#define BHAB_4538_HIRQ1_AST_MASK             0xFFFFFFFF
#define BHAB_4538_HIRQ1_WFE_MASK             0x00000000
#define BHAB_4538_HIRQ1_FSK_MASK             0x00000000

/* LEAP_CTRL_SW_SPARE0 bits */
#define BHAB_4538_SPARE0_LOCK0               0x00000001
#define BHAB_4538_SPARE0_LOCK1               0x00000002
#define BHAB_4538_SPARE0_LOCK2               0x00000004
#define BHAB_4538_SPARE0_LOCK3               0x00000008
#define BHAB_4538_SPARE0_LOCK4               0x00000010
#define BHAB_4538_SPARE0_LOCK5               0x00000020
#define BHAB_4538_SPARE0_LOCK6               0x00000040
#define BHAB_4538_SPARE0_LOCK7               0x00000080
#define BHAB_4538_SPARE0_INIT_DONE           0x80000000

/* LEAP_CTRL_SPARE1 bits */

/* LEAP_CTRL_SPARE2 bits */

/* LEAP_CTRL_SPARE3 bits */
#define BHAB_4538_SPARE3_HAB_ERROR_CODE_MASK    0x0000000F
#define BHAB_4538_SPARE3_HAB_ERROR_CODE_SHIFT   0
#define BHAB_4538_SPARE3_FSK_ERROR_CODE_MASK    0x000000F0
#define BHAB_4538_SPARE3_FSK_ERROR_CODE_SHIFT   4
#define BHAB_4538_SPARE3_FW_ERROR_CODE_MASK     0x00000F00
#define BHAB_4538_SPARE3_FW_ERROR_CODE_SHIFT    8
#define BHAB_4538_SPARE3_FLASH_ERROR_CODE_MASK  0x0000F000
#define BHAB_4538_SPARE3_FLASH_ERROR_CODE_SHIFT 12
#define BHAB_4538_SPARE3_LAST_HAB_CMD_RCVD_MASK 0x00FF0000
#define BHAB_4538_SPARE3_LAST_HAB_CMD_SVCD_MASK 0xFF000000


/***************************************************************************
Summary:
   This struct contains interrupt status information that is passed to the
   callback routine.
Description:
   A pointer to this struct will be passed as the first parameter (void*) 
   to the callback routine.
Returns:
   BERR_Code
See Also:
   BHAB_IntCallbackFunc
****************************************************************************/
typedef struct BHAB_4538_IrqStatus
{
   uint32_t status0;
   uint32_t status1;
   void     *pParm1;
   int      parm2;
} BHAB_4538_IrqStatus;


/***************************************************************************
Summary:
   This function returns the default settings for the 4538 HAB module.
Description:
   This function returns the default settings for the 4538 HAB module. 
Returns:
   BERR_Code
See Also:
   BHAB_Open()
****************************************************************************/
BERR_Code BHAB_4538_GetDefaultSettings(
   BHAB_Settings * pDefSetting     /* [in] Default settings */
);


#ifdef __cplusplus
}
#endif

#endif
