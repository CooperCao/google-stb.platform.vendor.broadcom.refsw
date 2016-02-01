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
#ifndef _BHAB_45216_H_
#define _BHAB_45216_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "bhab.h"


/* HAB header macros */
#define BHAB_45216_ACK            0x80000000
#define BHAB_45216_WRITE 0
#define BHAB_45216_READ 1
#define BHAB_45216_MODULE_LEAP 0
#define BHAB_45216_MODULE_WFE  1
#define BHAB_45216_MODULE_SAT  2
#define BHAB_45216_MODULE_DSQ  3
#define BHAB_45216_MODULE_FSK  4
#define BHAB_45216_MODULE_MXT  5
#define BHAB_45216_MODULE_VSN  6

/* interrupt bits in LEAP_HOST_IRQ_STATUS0 */
#define BHAB_45216_HIRQ0_INIT_DONE            0x00000001
#define BHAB_45216_HIRQ0_HAB_DONE             0x00000002
#define BHAB_45216_HIRQ0_FW_ERROR             0x00000004
#define BHAB_45216_HIRQ0_FLASH_DONE           0x00000008
#define BHAB_45216_HIRQ0_FSK_RX               0x00000010
#define BHAB_45216_HIRQ0_FSK_TX               0x00000020
#define BHAB_45216_HIRQ0_WFE_SA_DONE          0x00000040
#define BHAB_45216_HIRQ0_WFE0_READY           0x00000080
#define BHAB_45216_HIRQ0_WFE1_READY           0x00000100
#define BHAB_45216_HIRQ0_SAT_INIT_DONE        0x00000200
#define BHAB_45216_HIRQ0_DSQ0_TX              0x00000400
#define BHAB_45216_HIRQ0_DSQ0_RX              0x00000800
#define BHAB_45216_HIRQ0_DSQ0_VSENSE          0x00001000
#define BHAB_45216_HIRQ0_DSQ1_TX              0x00002000
#define BHAB_45216_HIRQ0_DSQ1_RX              0x00004000
#define BHAB_45216_HIRQ0_DSQ1_VSENSE          0x00008000
#define BHAB_45216_HIRQ0_SAT10_MISC           0x00010000
#define BHAB_45216_HIRQ0_SAT11_LOCK_CHANGE    0x00020000
#define BHAB_45216_HIRQ0_SAT11_ACQ_DONE       0x00040000
#define BHAB_45216_HIRQ0_SAT11_MISC           0x00080000
#define BHAB_45216_HIRQ0_SAT12_LOCK_CHANGE    0x00100000
#define BHAB_45216_HIRQ0_SAT12_ACQ_DONE       0x00200000
#define BHAB_45216_HIRQ0_SAT12_MISC           0x00400000
#define BHAB_45216_HIRQ0_SAT13_LOCK_CHANGE    0x00800000
#define BHAB_45216_HIRQ0_SAT13_ACQ_DONE       0x01000000
#define BHAB_45216_HIRQ0_SAT13_MISC           0x02000000
#define BHAB_45216_HIRQ0_SAT14_LOCK_CHANGE    0x04000000
#define BHAB_45216_HIRQ0_SAT14_ACQ_DONE       0x08000000
#define BHAB_45216_HIRQ0_SAT14_MISC           0x10000000
#define BHAB_45216_HIRQ0_SAT15_LOCK_CHANGE    0x20000000
#define BHAB_45216_HIRQ0_SAT15_ACQ_DONE       0x40000000
#define BHAB_45216_HIRQ0_SAT15_MISC           0x80000000

#define BHAB_45216_HIRQ0_SAT_MASK             (0xFFFF0000 | BHAB_45216_HIRQ0_SAT_INIT_DONE)
#define BHAB_45216_HIRQ0_WFE_MASK             (BHAB_45216_HIRQ0_WFE_SA_DONE | BHAB_45216_HIRQ0_WFE0_READY | BHAB_45216_HIRQ0_WFE1_READY)
#define BHAB_45216_HIRQ0_FSK_MASK             (BHAB_45216_HIRQ0_FSK_RX | BHAB_45216_HIRQ0_FSK_TX)
#define BHAB_45216_HIRQ0_DSQ_MASK             (BHAB_45216_HIRQ0_DSQ0_TX | BHAB_45216_HIRQ0_DSQ0_RX | BHAB_45216_HIRQ0_DSQ0_VSENSE | BHAB_45216_HIRQ0_DSQ1_TX | BHAB_45216_HIRQ0_DSQ1_RX | BHAB_45216_HIRQ0_DSQ1_VSENSE)

/* interrupt bits in LEAP_HOST_IRQ_STATUS1 */
#define BHAB_45216_HIRQ1_SAT0_LOCK_CHANGE     0x00000001
#define BHAB_45216_HIRQ1_SAT0_ACQ_DONE        0x00000002
#define BHAB_45216_HIRQ1_SAT0_MISC            0x00000004
#define BHAB_45216_HIRQ1_SAT1_LOCK_CHANGE     0x00000008
#define BHAB_45216_HIRQ1_SAT1_ACQ_DONE        0x00000010
#define BHAB_45216_HIRQ1_SAT1_MISC            0x00000020
#define BHAB_45216_HIRQ1_SAT2_LOCK_CHANGE     0x00000040
#define BHAB_45216_HIRQ1_SAT2_ACQ_DONE        0x00000080
#define BHAB_45216_HIRQ1_SAT2_MISC            0x00000100
#define BHAB_45216_HIRQ1_SAT3_LOCK_CHANGE     0x00000200
#define BHAB_45216_HIRQ1_SAT3_ACQ_DONE        0x00000400
#define BHAB_45216_HIRQ1_SAT3_MISC            0x00000800
#define BHAB_45216_HIRQ1_SAT4_LOCK_CHANGE     0x00001000
#define BHAB_45216_HIRQ1_SAT4_ACQ_DONE        0x00002000
#define BHAB_45216_HIRQ1_SAT4_MISC            0x00004000
#define BHAB_45216_HIRQ1_SAT5_LOCK_CHANGE     0x00008000
#define BHAB_45216_HIRQ1_SAT5_ACQ_DONE        0x00010000
#define BHAB_45216_HIRQ1_SAT5_MISC            0x00020000
#define BHAB_45216_HIRQ1_SAT6_LOCK_CHANGE     0x00040000
#define BHAB_45216_HIRQ1_SAT6_ACQ_DONE        0x00080000
#define BHAB_45216_HIRQ1_SAT6_MISC            0x00100000
#define BHAB_45216_HIRQ1_SAT7_LOCK_CHANGE     0x00200000
#define BHAB_45216_HIRQ1_SAT7_ACQ_DONE        0x00400000
#define BHAB_45216_HIRQ1_SAT7_MISC            0x00800000
#define BHAB_45216_HIRQ1_SAT8_LOCK_CHANGE     0x01000000
#define BHAB_45216_HIRQ1_SAT8_ACQ_DONE        0x02000000
#define BHAB_45216_HIRQ1_SAT8_MISC            0x04000000
#define BHAB_45216_HIRQ1_SAT9_LOCK_CHANGE     0x08000000
#define BHAB_45216_HIRQ1_SAT9_ACQ_DONE        0x10000000
#define BHAB_45216_HIRQ1_SAT9_MISC            0x20000000
#define BHAB_45216_HIRQ1_SAT10_LOCK_CHANGE    0x40000000
#define BHAB_45216_HIRQ1_SAT10_ACQ_DONE       0x80000000

#define BHAB_45216_HIRQ1_SAT_MASK             0xFFFFFFFF
#define BHAB_45216_HIRQ1_WFE_MASK             0x00000000
#define BHAB_45216_HIRQ1_FSK_MASK             0x00000000
#define BHAB_45216_HIRQ1_DSQ_MASK             0x00000000

/* LEAP_CTRL_SW_SPARE0 bits */
#define BHAB_45216_SPARE0_CH0_LOCK            0x00000001
#define BHAB_45216_SPARE0_CH1_LOCK            0x00000002
#define BHAB_45216_SPARE0_CH2_LOCK            0x00000004
#define BHAB_45216_SPARE0_CH3_LOCK            0x00000008
#define BHAB_45216_SPARE0_CH4_LOCK            0x00000010
#define BHAB_45216_SPARE0_CH5_LOCK            0x00000020
#define BHAB_45216_SPARE0_CH6_LOCK            0x00000040
#define BHAB_45216_SPARE0_CH7_LOCK            0x00000080
#define BHAB_45216_SPARE0_CH8_LOCK            0x00000100
#define BHAB_45216_SPARE0_CH9_LOCK            0x00000200
#define BHAB_45216_SPARE0_CH10_LOCK           0x00000400
#define BHAB_45216_SPARE0_CH11_LOCK           0x00000800
#define BHAB_45216_SPARE0_CH12_LOCK           0x00001000
#define BHAB_45216_SPARE0_CH13_LOCK           0x00002000
#define BHAB_45216_SPARE0_CH14_LOCK           0x00004000
#define BHAB_45216_SPARE0_CH15_LOCK           0x00008000
#define BHAB_45216_SPARE0_CH0_ACQ_DONE        0x00010000
#define BHAB_45216_SPARE0_CH1_ACQ_DONE        0x00020000
#define BHAB_45216_SPARE0_CH2_ACQ_DONE        0x00040000
#define BHAB_45216_SPARE0_CH3_ACQ_DONE        0x00080000
#define BHAB_45216_SPARE0_CH4_ACQ_DONE        0x00100000
#define BHAB_45216_SPARE0_CH5_ACQ_DONE        0x00200000
#define BHAB_45216_SPARE0_CH6_ACQ_DONE        0x00400000
#define BHAB_45216_SPARE0_CH7_ACQ_DONE        0x00800000
#define BHAB_45216_SPARE0_CH8_ACQ_DONE        0x01000000
#define BHAB_45216_SPARE0_CH9_ACQ_DONE        0x02000000
#define BHAB_45216_SPARE0_CH10_ACQ_DONE       0x04000000
#define BHAB_45216_SPARE0_CH11_ACQ_DONE       0x08000000
#define BHAB_45216_SPARE0_CH12_ACQ_DONE       0x10000000
#define BHAB_45216_SPARE0_CH13_ACQ_DONE       0x20000000
#define BHAB_45216_SPARE0_CH14_ACQ_DONE       0x40000000
#define BHAB_45216_SPARE0_CH15_ACQ_DONE       0x80000000

/* LEAP_CTRL_SPARE1 bits */
#define BHAB_45216_SPARE1_CH0_SIG_NOT         0x00000001
#define BHAB_45216_SPARE1_CH0_READY           0x00000002
#define BHAB_45216_SPARE1_CH1_SIG_NOT         0x00000004
#define BHAB_45216_SPARE1_CH1_READY           0x00000008
#define BHAB_45216_SPARE1_CH2_SIG_NOT         0x00000010
#define BHAB_45216_SPARE1_CH2_READY           0x00000020
#define BHAB_45216_SPARE1_CH3_SIG_NOT         0x00000040
#define BHAB_45216_SPARE1_CH3_READY           0x00000080
#define BHAB_45216_SPARE1_CH4_SIG_NOT         0x00000100
#define BHAB_45216_SPARE1_CH4_READY           0x00000200
#define BHAB_45216_SPARE1_CH5_SIG_NOT         0x00000400
#define BHAB_45216_SPARE1_CH5_READY           0x00000800
#define BHAB_45216_SPARE1_CH6_SIG_NOT         0x00001000
#define BHAB_45216_SPARE1_CH6_READY           0x00002000
#define BHAB_45216_SPARE1_CH7_SIG_NOT         0x00004000
#define BHAB_45216_SPARE1_CH7_READY           0x00008000
#define BHAB_45216_SPARE1_CH8_SIG_NOT         0x00010000
#define BHAB_45216_SPARE1_CH8_READY           0x00020000
#define BHAB_45216_SPARE1_CH9_SIG_NOT         0x00040000
#define BHAB_45216_SPARE1_CH9_READY           0x00080000
#define BHAB_45216_SPARE1_CH10_SIG_NOT        0x00100000
#define BHAB_45216_SPARE1_CH10_READY          0x00200000
#define BHAB_45216_SPARE1_CH11_SIG_NOT        0x00400000
#define BHAB_45216_SPARE1_CH11_READY          0x00800000
#define BHAB_45216_SPARE1_CH12_SIG_NOT        0x01000000
#define BHAB_45216_SPARE1_CH12_READY          0x02000000
#define BHAB_45216_SPARE1_CH13_SIG_NOT        0x04000000
#define BHAB_45216_SPARE1_CH13_READY          0x08000000
#define BHAB_45216_SPARE1_CH14_SIG_NOT        0x10000000
#define BHAB_45216_SPARE1_CH14_READY          0x20000000
#define BHAB_45216_SPARE1_CH15_SIG_NOT        0x40000000
#define BHAB_45216_SPARE1_CH15_READY          0x80000000

/* LEAP_CTRL_SPARE2 bits */
/* TBD... */

/* LEAP_CTRL_SPARE3 bits */
#define BHAB_45216_SPARE3_FW_ERROR_CODE_MASK     0x0000000F
#define BHAB_45216_SPARE3_INIT_DONE              0x00008000
#define BHAB_45216_SPARE3_LAST_HAB_CMD_RCVD_MASK 0x00FF0000
#define BHAB_45216_SPARE3_LAST_HAB_CMD_SVCD_MASK 0xFF000000


/* channel interrupt mask used in BXXX_45216_P_EnableChannelInterrupt() */
#define BHAB_45216_CHAN_IRQ_LOCK_STATUS_CHANGE  0x1
#define BHAB_45216_CHAN_IRQ_ACQ_DONE            0x2
#define BHAB_45216_CHAN_IRQ_MISC                0x4
#define BHAB_45216_CHAN_IRQ_ACQ                 0x7
#define BHAB_45216_CHAN_IRQ_ALL                 0x7
#define BHAB_45216_DSEC_IRQ_TX_DONE             0x1
#define BHAB_45216_DSEC_IRQ_RX                  0x2
#define BHAB_45216_DSEC_IRQ_VSENSE              0x4
#define BHAB_45216_DSEC_IRQ_ALL                 0x7


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
typedef struct BHAB_45216_IrqStatus
{
   uint32_t status0;
   uint32_t status1;
   void     *pParm1;
   int      parm2;
} BHAB_45216_IrqStatus;


/***************************************************************************
Summary:
   This function returns the default settings for the 45216 HAB module.
Description:
   This function returns the default settings for the 45216 HAB module.
Returns:
   BERR_Code
See Also:
   BHAB_Open()
****************************************************************************/
BERR_Code BHAB_45216_GetDefaultSettings(
   BHAB_Settings * pDefSetting     /* [in] Default settings */
);


/***************************************************************************
Summary:
   This function returns the handle to the flash done event.
Description:
   The flash done event is set when the flash sector write operation that
   was initiated by BHAB_45216_WriteFlashSector() has completed.
Returns:
   BERR_Code
See Also:
   BHAB_45216_WriteFlashSector()
****************************************************************************/
BERR_Code BHAB_45216_GetFlashEventHandle(
   BHAB_Handle handle,            /* [in] BHAB handle */
   BKNI_EventHandle *hEvent       /* [out] flash operation done event handle */
);


/***************************************************************************
Summary:
   This function programs a given sector in flash.
Description:
   This function starts writing a specified sector in flash.  This function
   will return before the sector has been written.  The flash event will be
   set when the sector programming has completed.
Returns:
   BERR_Code
See Also:
   BHAB_45216_GetFlashEventHandle()
****************************************************************************/
BERR_Code BHAB_45216_WriteFlashSector(
   BHAB_Handle h,     /* [in] BHAB handle */
   uint32_t    addr,  /* [in] sector address in flash to program */
   uint8_t     *pData /* [in] pointer to 4KB buffer containing data to program to flash sector */
);


/***************************************************************************
Summary:
   This function reads the flash.
Description:
   This function reads the flash.
Returns:
   BERR_Code
See Also:
   BHAB_45216_WriteFlashSector()
****************************************************************************/
BERR_Code BHAB_45216_ReadFlash(
   BHAB_Handle h,    /* [in] BHAB handle */
   uint32_t    addr, /* [in] offset in flash */
   uint32_t    n,    /* [in] number of bytes to read */
   uint8_t     *pBuf /* [out] bytes read from flash */
);


/***************************************************************************
Summary:
   This function transmits the given string from the 45216 LEAP UART.
Description:
   This function transmits the given string from the 45216 LEAP UART.
Returns:
   BERR_Code
See Also:
   BHAB_Open()
****************************************************************************/
BERR_Code BHAB_45216_PrintUart(
   BHAB_Handle h,  /* [in] BHAB handle */
   char *pStr      /* [in] null-terminated string */
);


/***************************************************************************
Summary:
   This function initializes the first 32-bit word of an HAB command.
Description:
   This function initializes the first 32-bit word of an HAB command.
Returns:
   BERR_Code
See Also:
   BHAB_SendHabCommand()
****************************************************************************/
uint32_t BHAB_45216_InitHeader(uint8_t cmd, uint8_t chn, uint8_t dir, uint8_t module);


#ifdef __cplusplus
}
#endif

#endif
