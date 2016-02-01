/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
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

/***************************************************************************
 * Definitions
 ***************************************************************************/

/*#define THD_DEBUG     // Enables printf statements on uart */
/*#define THD_PROFILE   // Enables primitive profiling */
/*#define THD_PLL2      // Use PLL2 */

/* Mode definition */
#define THD_MODE_AUTO              0
#define THD_MODE_FIXED             1
#define THD_MODE_PEDESTRIAN        2
#define THD_MODE_MOBILE            3

/*Event definitions - match THD_CORE_STATUS bit locations  */
#define THD_EVENT_FFT_SYNC         0
#define THD_EVENT_SP_SYNC          1
#define THD_EVENT_TPS_SYNC         3
#define THD_EVENT_TMCC_SYNC        6
#define THD_EVENT_VIT_SYNC         9
#define THD_EVENT_FEC_SYNC        11
#define THD_EVENT_TS_ONE_PKT      14
#define THD_EVENT_FSCNT_ZERO      17
#define THD_EVENT_FBCNT_ZERO      18
#define THD_EVENT_FW_CORR_MAX_RDY 21

/* TS definitions */
#define THD_TS_NONE                0
#define THD_TS_PARALLEL_DC         1
#define THD_TS_SERIAL_DC           2
#define THD_TS_PARALLEL            3
#define THD_TS_SERIAL              4


BERR_Code BTHD_7550_P_Open(BTHD_Handle *, BCHP_Handle, void*, BINT_Handle, const struct BTHD_Settings*);
BERR_Code BTHD_7550_P_Close(BTHD_Handle);
BERR_Code BTHD_7550_P_Init(BTHD_Handle, const uint8_t *, uint32_t);
BERR_Code BTHD_7550_P_TuneAcquire(BTHD_Handle, const BTHD_InbandParams *);
void BTHD_7550_P_ResetIrq(BTHD_Handle);
BERR_Code BTHD_7550_P_GetLockStateChangeEvent( BTHD_Handle, BKNI_EventHandle*);
BERR_Code BTHD_7550_P_GetUnlockStateChangeEvent( BTHD_Handle, BKNI_EventHandle*);
BERR_Code BTHD_7550_P_GetInterruptEventHandle(BTHD_Handle, BKNI_EventHandle*);
BERR_Code BTHD_7550_P_GetEWSEventHandle(BTHD_Handle h, BKNI_EventHandle* hEvent);
BERR_Code BTHD_7550_P_ProcessUnlockEvent(BTHD_Handle);
BERR_Code BTHD_7550_P_ProcessStatusEvent(BTHD_Handle);
BERR_Code BTHD_7550_P_ProcessInterruptEvent(BTHD_Handle);
BERR_Code BTHD_7550_P_TimerFunc(void *myParam1, int myParam2);
BERR_Code BTHD_7550_P_TuneAcquire(BTHD_Handle, const BTHD_InbandParams *);


#define BTHD_7550_P_WriteField(h, Register, Field, Data) \
  BREG_Write32(h, BCHP_##Register, ((BREG_Read32(h, BCHP_##Register) & \
  ~((uint32_t)BCHP_MASK(Register, Field))) | \
  BCHP_FIELD_DATA(Register, Field, Data)))


#define BTHD_7550_P_ReadField(h, Register, Field) \
  ((((BREG_Read32(h, BCHP_##Register)) & BCHP_MASK(Register,Field)) >> \
  BCHP_SHIFT(Register,Field)))


