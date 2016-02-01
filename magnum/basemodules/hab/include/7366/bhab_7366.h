/******************************************************************************
*    (c)2011-2013 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to the terms and
* conditions of a separate, written license agreement executed between you and Broadcom
* (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
* no license (express or implied), right to use, or waiver of any kind with respect to the
* Software, and Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
* secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
* and to use this information only in connection with your use of Broadcom integrated circuit products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
* AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
* WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
* THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
* OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
* LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
* OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
* USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
* LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
* EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
* USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
* ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
* LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
* ANY LIMITED REMEDY.
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
*****************************************************************************/
#ifndef BHAB_7366_H_
#define BHAB_7366_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "bhab.h"


/* HAB header macros */
#define BHAB_7366_ACK            0x80000000
#define BHAB_7366_ERR_CODE_MASK  0x7F000000;
#define BHAB_7366_ERR_CODE_SHIFT 24
#define BHAB_7366_WRITE 0
#define BHAB_7366_READ 1
#define BHAB_7366_MODULE_LEAP 0
#define BHAB_7366_MODULE_WFE  1
#define BHAB_7366_MODULE_SAT  2
#define BHAB_7366_MODULE_DSQ  3
#define BHAB_7366_MODULE_FSK  4
#define BHAB_7366_MODULE_MXT  5
#define BHAB_7366_MODULE_VSN  6

/* interrupt bits in LEAP_HOST_IRQ_STATUS0 */
#define BHAB_7366_HIRQ0_INIT_DONE                 0x00000001
#define BHAB_7366_HIRQ0_HAB_DONE                  0x00000002
#define BHAB_7366_HIRQ0_HAB_ERROR                 0x00000004 /* look at SPARE3[3:0] for error code */
#define BHAB_7366_HIRQ0_FW_ERROR                  0x00000008 /* look at SPARE3[7:4] for error code */
#define BHAB_7366_HIRQ0_FSK_RX                    0x00000010
#define BHAB_7366_HIRQ0_FSK_TX                    0x00000020
#define BHAB_7366_HIRQ0_FLASH_DONE                0x00000040 /* look at SPARE3[11:8] for error code */
#define BHAB_7366_HIRQ0_WFE_SA_DONE               0x00000080
#define BHAB_7366_HIRQ0_WFE0_READY                0x00000100
#define BHAB_7366_HIRQ0_WFE1_READY                0x00000200
#define BHAB_7366_HIRQ0_WFE2_READY                0x00000400
#define BHAB_7366_HIRQ0_WFE3_READY                0x00000800
#define BHAB_7366_HIRQ0_SAT_INIT_DONE             0x00001000

#define BHAB_7366_HIRQ0_DSEC0_TX                  0x00010000
#define BHAB_7366_HIRQ0_DSEC0_RX                  0x00020000
#define BHAB_7366_HIRQ0_DSEC0_VSENSE              0x00040000
#define BHAB_7366_HIRQ0_DSEC1_TX                  0x00080000
#define BHAB_7366_HIRQ0_DSEC1_RX                  0x00100000
#define BHAB_7366_HIRQ0_DSEC1_VSENSE              0x00200000
#define BHAB_7366_HIRQ0_DSEC2_TX                  0x00400000
#define BHAB_7366_HIRQ0_DSEC2_RX                  0x00800000
#define BHAB_7366_HIRQ0_DSEC2_VSENSE              0x01000000
#define BHAB_7366_HIRQ0_DSEC3_TX                  0x02000000
#define BHAB_7366_HIRQ0_DSEC3_RX                  0x04000000
#define BHAB_7366_HIRQ0_DSEC3_VSENSE              0x08000000

#define BHAB_7366_HIRQ0_SAT_MASK                  (BHAB_7366_HIRQ0_SAT_INIT_DONE)
#define BHAB_7366_HIRQ0_DSQ_MASK                  (0x0FFF0000)
#define BHAB_7366_HIRQ0_WFE_MASK                  (BHAB_7366_HIRQ0_WFE_SA_DONE | BHAB_7366_HIRQ0_WFE0_READY | BHAB_7366_HIRQ0_WFE1_READY | BHAB_7366_HIRQ0_WFE2_READY | BHAB_7366_HIRQ0_WFE3_READY)
#define BHAB_7366_HIRQ0_FSK_MASK                  (BHAB_7366_HIRQ0_FSK_RX | BHAB_7366_HIRQ0_FSK_TX)

/* interrupt bits in LEAP_HOST_IRQ_STATUS1 */
#define BHAB_7366_HIRQ1_CHAN0_LOCK_CHANGE         0x00000001
#define BHAB_7366_HIRQ1_CHAN0_ACQ_DONE            0x00000002
#define BHAB_7366_HIRQ1_CHAN0_SIGNAL_NOTIFICATION 0x00000004
#define BHAB_7366_HIRQ1_CHAN0_READY               0x00000008
#define BHAB_7366_HIRQ1_CHAN1_LOCK_CHANGE         0x00000010
#define BHAB_7366_HIRQ1_CHAN1_ACQ_DONE            0x00000020
#define BHAB_7366_HIRQ1_CHAN1_SIGNAL_NOTIFICATION 0x00000040
#define BHAB_7366_HIRQ1_CHAN1_READY               0x00000080
#define BHAB_7366_HIRQ1_CHAN2_LOCK_CHANGE         0x00000100
#define BHAB_7366_HIRQ1_CHAN2_ACQ_DONE            0x00000200
#define BHAB_7366_HIRQ1_CHAN2_SIGNAL_NOTIFICATION 0x00000400
#define BHAB_7366_HIRQ1_CHAN2_READY               0x00000800
#define BHAB_7366_HIRQ1_CHAN3_LOCK_CHANGE         0x00001000
#define BHAB_7366_HIRQ1_CHAN3_ACQ_DONE            0x00002000
#define BHAB_7366_HIRQ1_CHAN3_SIGNAL_NOTIFICATION 0x00004000
#define BHAB_7366_HIRQ1_CHAN3_READY               0x00008000
#define BHAB_7366_HIRQ1_CHAN4_LOCK_CHANGE         0x00010000
#define BHAB_7366_HIRQ1_CHAN4_ACQ_DONE            0x00020000
#define BHAB_7366_HIRQ1_CHAN4_SIGNAL_NOTIFICATION 0x00040000
#define BHAB_7366_HIRQ1_CHAN4_READY               0x00080000
#define BHAB_7366_HIRQ1_CHAN5_LOCK_CHANGE         0x00100000
#define BHAB_7366_HIRQ1_CHAN5_ACQ_DONE            0x00200000
#define BHAB_7366_HIRQ1_CHAN5_SIGNAL_NOTIFICATION 0x00400000
#define BHAB_7366_HIRQ1_CHAN5_READY               0x00800000
#define BHAB_7366_HIRQ1_CHAN6_LOCK_CHANGE         0x01000000
#define BHAB_7366_HIRQ1_CHAN6_ACQ_DONE            0x02000000
#define BHAB_7366_HIRQ1_CHAN6_SIGNAL_NOTIFICATION 0x04000000
#define BHAB_7366_HIRQ1_CHAN6_READY               0x08000000
#define BHAB_7366_HIRQ1_CHAN7_LOCK_CHANGE         0x10000000
#define BHAB_7366_HIRQ1_CHAN7_ACQ_DONE            0x20000000
#define BHAB_7366_HIRQ1_CHAN7_SIGNAL_NOTIFICATION 0x40000000
#define BHAB_7366_HIRQ1_CHAN7_READY               0x80000000

/* LEAP_CTRL_SPARE0 bits */
#define BHAB_7366_SPARE0_LOCK_CHAN0               0x00000001
#define BHAB_7366_SPARE0_LOCK_CHAN1               0x00000002
#define BHAB_7366_SPARE0_LOCK_CHAN2               0x00000004
#define BHAB_7366_SPARE0_LOCK_CHAN3               0x00000008
#define BHAB_7366_SPARE0_LOCK_CHAN4               0x00000010
#define BHAB_7366_SPARE0_LOCK_CHAN5               0x00000020
#define BHAB_7366_SPARE0_LOCK_CHAN6               0x00000040
#define BHAB_7366_SPARE0_LOCK_CHAN7               0x00000080
#define BHAB_7366_SPARE0_LOCK_MASK                0x000000FF
#define BHAB_7366_SPARE0_ACQ_DONE_CHAN0           0x00000100
#define BHAB_7366_SPARE0_ACQ_DONE_CHAN1           0x00000200
#define BHAB_7366_SPARE0_ACQ_DONE_CHAN2           0x00000400
#define BHAB_7366_SPARE0_ACQ_DONE_CHAN3           0x00000800
#define BHAB_7366_SPARE0_ACQ_DONE_CHAN4           0x00001000
#define BHAB_7366_SPARE0_ACQ_DONE_CHAN5           0x00002000
#define BHAB_7366_SPARE0_ACQ_DONE_CHAN6           0x00004000
#define BHAB_7366_SPARE0_ACQ_DONE_CHAN7           0x00008000
#define BHAB_7366_SPARE0_ACQ_DONE_MASK            0x0000FF00
#define BHAB_7366_SPARE0_INIT_DONE                0x80000000

/* LEAP_CTRL_SPARE1 bits */
/* TBD... */

/* LEAP_CTRL_SPARE2 bits */
/* TBD... */

/* LEAP_CTRL_SPARE3 bits */
#define BHAB_7366_SPARE3_HAB_ERROR_CODE_MASK       0x0000000F
#define BHAB_7366_SPARE3_HAB_ERROR_CODE_SHIFT      0
#define BHAB_7366_SPARE3_FW_ERROR_CODE_MASK        0x000000F0
#define BHAB_7366_SPARE3_FW_ERROR_CODE_SHIFT       4
#define BHAB_7366_SPARE3_FLASH_ERROR_CODE_MASK     0x00000F00
#define BHAB_7366_SPARE3_FLASH_ERROR_CODE_SHIFT    8
#define BHAB_7366_SPARE3_RESERVED_MASK             0x0000F000
#define BHAB_7366_SPARE3_LAST_HAB_CMD_RCVD_MASK    0x00FF0000
#define BHAB_7366_SPARE3_LAST_HAB_CMD_RCVD_SHIFT   16
#define BHAB_7366_SPARE3_LAST_HAB_CMD_SVCD_MASK    0xFF000000
#define BHAB_7366_SPARE3_LAST_HAB_CMD_SVCD_SHIFT   24

/* FW error codes stored in SPARE3[7:4] */
#define BHAB_7366_FW_ERROR_NONE                0 /* no error */
#define BHAB_7366_FW_ERROR_UNHANDLED_EXCEPTION 1 /* MIPS unhandled exception */
#define BHAB_7366_FW_ERROR_SAT                 2 /* SAT PI error */
#define BHAB_7366_FW_ERROR_CREATE_EVENT        3 /* BKNI_CreateEvent error */
#define BHAB_7366_FW_ERROR_CREATE_CALLBACK     4 /* BKNI_CreateCallback error */
#define BHAB_7366_FW_ERROR_WFE                 5 /* WFE PI error */
#define BHAB_7366_FW_ERROR_CREATE_MUTEX        6 /* BKNI_CreateMutex error */
#define BHAB_7366_FW_ERROR_BAC                 7 /* BAC error */
#define BHAB_7366_FW_ERROR_MPU_VIOLATION       8 /* MPU error */
#define BHAB_7366_FW_ERROR_INSTRUCTION_ERROR   9 /* instruction error */
#define BHAB_7366_FW_ERROR_TASK_CLOSED         10 /* task closed unexpectedly */
#define BHAB_7366_FW_ERROR_DSQ                 11 /* DSQ PI error */
#define BHAB_7366_FW_ERROR_FSK                 12 /* FSK PI error */

/* channel interrupt mask used in BXXX_7366_P_EnableChannelInterrupt() */
#define BHAB_7366_CHAN_IRQ_LOCK_STATUS_CHANGE  0x1
#define BHAB_7366_CHAN_IRQ_ACQ_DONE            0x2
#define BHAB_7366_CHAN_IRQ_SIGNAL_NOTIFICATION 0x4
#define BHAB_7366_CHAN_IRQ_READY               0x8
#define BHAB_7366_CHAN_IRQ_ACQ                 (BHAB_7366_CHAN_IRQ_LOCK_STATUS_CHANGE | BHAB_7366_CHAN_IRQ_ACQ_DONE | BHAB_7366_CHAN_IRQ_SIGNAL_NOTIFICATION)
#define BHAB_7366_CHAN_IRQ_ALL                 0xF
#define BHAB_7366_DSEC_IRQ_TX_DONE             0x1
#define BHAB_7366_DSEC_IRQ_RX_DONE             0x2
#define BHAB_7366_DSEC_IRQ_VSENSE              0x4
#define BHAB_7366_DSEC_IRQ_ALL                 (BHAB_7366_DSEC_IRQ_VSENSE | BHAB_7366_DSEC_IRQ_TX_DONE | BHAB_7366_DSEC_IRQ_RX_DONE)


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
typedef struct BHAB_7366_IrqStatus
{
   uint32_t status0;
   uint32_t status1;
   void     *pParm1;
   int      parm2;
} BHAB_7366_IrqStatus;


typedef struct BHAB_7366_Settings
{
   bool bUseInternalMemory; /* true if LEAP is loaded in 7366 internal memory */
   uint32_t physAddr;       /* physical address of 1MB LEAP memory (applies only when bUseInternalMemory=false) */
   void *pRam;              /* pointer to 1MB LEAP memory (applies only when bUseInternalMemory=false) */
   void *heap;              /* heap handle where pRam is allocated */
} BHAB_7366_Settings;


/***************************************************************************
Summary:
   This function returns the default settings for the 7366 HAB module.
Description:
   This function returns the default settings for the 7366 HAB module.
Returns:
   BERR_Code
See Also:
   BHAB_Open()
****************************************************************************/
BERR_Code BHAB_7366_GetDefaultSettings(
   BHAB_Settings * pDefSetting     /* [in] Default settings */
);

uint32_t BHAB_7366_InitHeader(uint8_t cmd, uint8_t chn, uint8_t dir, uint8_t module);
BERR_Code BHAB_7366_PrintUart(BHAB_Handle h, char *pStr);
BERR_Code BHAB_7366_Configure(BHAB_Handle h, BHAB_7366_Settings *pSettings);

#ifdef __cplusplus
}
#endif

#endif /* _BHAB_7366_H_ */

