/***************************************************************************
 *     Copyright (c) 2007-2010, Broadcom Corporation
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

#include "bstd.h"
#include "bkni.h"
#include "bchp_ap_32.h"
#include "bchp_ds_ter_eq.h"
#include "bchp_ds_pll.h"
#include "bchp_ds_afe.h"
#include "bchp_clkgen.h"
#include "bchp_vdec_fe_0.h"
#include "bchp_ifd_ss.h"
#include "bchp_ifd_core.h" 
#include "bvsb.h"
#include "bvsb_priv.h"
#include "bvsb_3548_priv.h"
#include "bfmt.h"
#include "microcode/bcm3548ap.h"

BDBG_MODULE(bvsb_3548_priv);

/*#define BVSB_DEBUG*/

#define BVSB_HAB_DONE_TIMEOUT 100

/* AP internal memory addresses */
#define BVSB_AP_INT_RAM 0xB00000
#define BVSB_AP_HAB     0xB40000
#define HAB_ACQUIRE_IDLE 0xD

#define BCHP_INT_ID_HAB_DONE  BCHP_INT_ID_CREATE(BCHP_AP_32_H_STAT1, BCHP_AP_32_H_STAT1_HAB_DONE_SHIFT)
#define BCHP_INT_ID_INIT_AP   BCHP_INT_ID_CREATE(BCHP_AP_32_H_STAT2, 7)
#define BCHP_INT_ID_LOCK      BCHP_INT_ID_CREATE(BCHP_AP_32_H_STAT2, 4)
#define BCHP_INT_ID_UNLOCK    BCHP_INT_ID_CREATE(BCHP_AP_32_H_STAT2, 5)

#define NO_AP_MODE
/* #define BVSB_POWER_MANAGEMENT */

void BVSB_3548_P_Lock_isr(void *p, int param)
{
   BVSB_Handle h = *(BVSB_Handle*)p;
   BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

   BSTD_UNUSED(param);

   /* Send off event and we are done */
   BDBG_MSG(("Receive lock interrupt"));
   BKNI_SetEvent(p3548->hLockChangeEvent);
   BKNI_SetEvent(p3548->hLockEvent);

}
void BVSB_3548_P_Unlock_isr(void *p, int param)
{
   BVSB_Handle h = *(BVSB_Handle*)p;
   BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

   BSTD_UNUSED(param);

   /* Send off event and we are done */
   BDBG_MSG(("Receive unlock interrupt"));
   BKNI_SetEvent(p3548->hLockChangeEvent);
   BKNI_SetEvent(p3548->hUnlockEvent);
}

void BVSB_3548_P_Hab_Done_isr(void *p, int param)
{
   BVSB_Handle h = *(BVSB_Handle*)p;
   BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

   BSTD_UNUSED(param);

   /* Send off event and we are done */
   BDBG_MSG(("Hab done isr"));
   BKNI_SetEvent(p3548->hHabDoneEvent);
}

void BVSB_3548_P_InitAp_isr(void *p, int param)
{
   BVSB_Handle h = *(BVSB_Handle*)p;
   BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

   BSTD_UNUSED(param);

   /* Send off event and we are done */
   BDBG_MSG(("Init Ap isr\n"));
   BKNI_SetEvent(p3548->hInitDoneEvent);
}

/******************************************************************************
  BVSB_3548_P_TimerFunc()
 ******************************************************************************/
static void BVSB_3548_P_TimerFunc(
        void *myParam1,
        int myParam2
        )
{
    /* This function solves the "Men in Black II" VCR problem, in which missing*/
    /* HSYNCs before VSYNC causes an invalid combo AGC update (and vertical tearing).*/
    /* The solution is .PD_NUM_LINES=4 in which the max PD is held over 2^4 lines*/
    /* to coast through the missing HSYNCs.  But this also reduces AGC BW for large*/
    /* hum, so should only be used when VCR's detected (least disruptive to IFD).*/
    uint32_t regVal;
	uint32_t ulSpare, ulStuckThresh, ulPrevAgc;
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(myParam1);

    BSTD_UNUSED(myParam2);

    regVal = BREG_Read32(p3548->hRegister, BCHP_VDEC_FE_0_FE_STATUS0);

	if (regVal & 0x08000000)   /* [27]= .HEAD_SWITCH_DETECTED= 1*/
	{
       BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_APDCTL,   0x00000080);  /* .PD_NUM_LINES=4  (for bad VCR's)*/
	}
	else                      /* [27]= .HEAD_SWITCH_DETECTED= 0*/
	{
       BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_APDCTL,   0x00000020);  /* .PD_NUM_LINES=1  (nominal)*/
	}

    /* Add lock status monitoring. No hardware interrupt assigned to this */
    regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_SS_SS_STATUS);
    if ((regVal & BCHP_IFD_SS_SS_STATUS_LOCK_DETECT_MASK) == BCHP_IFD_SS_SS_STATUS_LOCK_DETECT_MASK)
    {
        if (p3548->bIfdLockStatus == false)
        {
            p3548->bIfdLockStatus = true;
            BKNI_SetEvent(p3548->hLockChangeEvent);
            BKNI_SetEvent(p3548->hLockEvent);
        }
    }
    else
    {
        if (p3548->bIfdLockStatus == true)
        {
            p3548->bIfdLockStatus = false;
            BKNI_SetEvent(p3548->hLockChangeEvent);
            BKNI_SetEvent(p3548->hUnlockEvent);
        }
    }

	/* code to detect that IF AGC is "stuck" and temporarily switch to peak detect mode */
	ulSpare = BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_VP_SPARE1);
    ulStuckThresh = ulSpare & 0xFF;

    if (ulStuckThresh > 0)
    {
        ulPrevAgc = p3548->ulNewAgc; /* set to previous IF AGC value */
        p3548->ulNewAgc = BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_FE_AINAPGA);
        if (p3548->ulNewAgc == ulPrevAgc)
        {
            p3548->ulStuckCount++; 
        }
        else
        {
            p3548->ulStuckCount = 0;
        }

        /* remember that stuck_thresh=0 wouldn't get to here */
        regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_FE_VARDEL);
        if (p3548->ulStuckCount < ulStuckThresh) 
        {
            regVal |= 0x00008000;
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_VARDEL, regVal);
        }
        else
        {
            regVal &= 0xFFFF7FFF;
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_VARDEL, regVal);
            p3548->ulStuckCount = 0;
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_SPARE1, ulSpare + 0x1000000);
        }

    }
    else
        p3548->ulStuckCount = 0; /* keep counter reset */

}


/******************************************************************************
  BVSB_3548_P_Open()
 ******************************************************************************/
BERR_Code BVSB_3548_P_Open(
        BVSB_Handle *h,       /* [out] BVSB handle */
        BCHP_Handle hChip,    /* [in] chip handle */
        void        *pReg,    /* [in] pointer to register or i2c handle */
        BINT_Handle hInterrupt, /* [in] interrupt handle */
        const BVSB_Settings *pDefSettings /* [in] default settings */
        )
{
    BERR_Code retCode = BERR_SUCCESS;
    BVSB_Handle hDev = NULL;
    BVSB_3548_P_Handle *p3548 = NULL;
    BTMR_Settings sTimerSettings;

    BSTD_UNUSED(hInterrupt);
    BSTD_UNUSED(hChip);
    BDBG_ASSERT(pReg);
    BDBG_ASSERT(pDefSettings);

    /* allocate memory for the handle */
    hDev = (BVSB_Handle)BKNI_Malloc(sizeof(BVSB_P_Handle));
    if (hDev == NULL)
    {
       retCode = BERR_OUT_OF_SYSTEM_MEMORY;
       goto done;
    }
    p3548 = (BVSB_3548_P_Handle *)BKNI_Malloc(sizeof(BVSB_3548_P_Handle));
    if (p3548 == NULL)
    {
       retCode = BERR_OUT_OF_SYSTEM_MEMORY;
       goto done;
    }
    BKNI_Memset(p3548, 0, sizeof(*p3548));
    hDev->pImpl = (void*)p3548;

    /* initialize our handle */
    p3548->hRegister = (BREG_Handle)pReg;
    BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BVSB_Settings));

    /* Create interrupt callbacks handle */
	 BVSB_CHK_RETCODE(BINT_CreateCallback( &(p3548->hInitApCallback), hInterrupt, 
               BCHP_INT_ID_INIT_AP, BVSB_3548_P_InitAp_isr, (void*)h, 0 ));
	 BVSB_CHK_RETCODE(BINT_CreateCallback( &(p3548->hHabDoneCallback), hInterrupt, 
               BCHP_INT_ID_HAB_DONE, BVSB_3548_P_Hab_Done_isr, (void*)h, 0 ));
	 BVSB_CHK_RETCODE(BINT_CreateCallback( &(p3548->hUnlockCallback), hInterrupt, 
               BCHP_INT_ID_UNLOCK, BVSB_3548_P_Lock_isr, (void*)h, 0 ));
	 BVSB_CHK_RETCODE(BINT_CreateCallback( &(p3548->hLockCallback), hInterrupt, 
               BCHP_INT_ID_LOCK, BVSB_3548_P_Unlock_isr, (void*)h, 0 ));
    
    /* create events */
    BVSB_CHK_RETCODE(BKNI_CreateEvent(&(p3548->hLockChangeEvent)));
    BVSB_CHK_RETCODE(BKNI_CreateEvent(&(p3548->hInitDoneEvent)));
    BVSB_CHK_RETCODE(BKNI_CreateEvent(&(p3548->hHabDoneEvent)));
    BVSB_CHK_RETCODE(BKNI_CreateEvent(&(p3548->hSignalLevelChangeEvent)));
    BVSB_CHK_RETCODE(BKNI_CreateEvent(&(p3548->hVideoKillEvent)));
    BVSB_CHK_RETCODE(BKNI_CreateEvent(&(p3548->hUnlockEvent)));  
    BVSB_CHK_RETCODE(BKNI_CreateEvent(&(p3548->hLockEvent)));

    /* Create timer for IFD */
    BTMR_GetDefaultTimerSettings(&sTimerSettings);
    sTimerSettings.type = BTMR_Type_ePeriodic;
    sTimerSettings.cb_isr = (BTMR_CallbackFunc)BVSB_3548_P_TimerFunc;
    sTimerSettings.pParm1 = (void*)p3548;
    sTimerSettings.parm2 = 0;
    sTimerSettings.exclusive = false;

    retCode = BTMR_CreateTimer (hDev->settings.hTmr, &p3548->hTimer, &sTimerSettings);
    if ( retCode != BERR_SUCCESS )
    {
        BDBG_ERR(("BVSB_Open: Create Timer Failed"));
        retCode = BERR_TRACE(retCode);
        goto done;
    }

    BVSB_CHK_RETCODE(BVSB_3548_P_PowerDown(hDev));
    *h = hDev;

done:
   if (BERR_SUCCESS != retCode)
	{
		if (hDev !=  NULL)
		{			
			BKNI_Free((void*)hDev);
		}		
		if (p3548 !=  NULL)
		{
			BKNI_Free((void*)p3548);
		}
	}

   return retCode;
}


/******************************************************************************
  BVSB_3548_P_Close()
 ******************************************************************************/
BERR_Code BVSB_3548_P_Close(BVSB_Handle h)
{
    BVSB_3548_P_Handle *p3548;
	BERR_Code retCode;
    BVSB_InbandParams inbandParams;
    BDBG_ASSERT(h);

    p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    retCode = BERR_SUCCESS;

    if (h->settings.bOpenDrainPinsOnClose)
    {
        /* Set demod to IFD mode to set pins open drain mode */
        /* Enable power before any acquistion. In case it comes from power off state */
        BVSB_3548_P_PowerEnable(h, BVSB_POWER_AP);

        /* Stop monitor timer before acquisition starts */
        BTMR_StopTimer(p3548->hTimer);

        inbandParams.mode = BVSB_InbandMode_eNTSC;
        inbandParams.eVideoMode = BVSB_AnalogVideoMode_eNtsc_M;
        inbandParams.eAudioMode = BVSB_AnalogAudioMode_eBtsc;
        inbandParams.eAnalogAudioDeviation = BVSB_AnalogAudioDeviation_eNormal;
        inbandParams.eAcquisitionMode = BVSB_IfdAcquisitionMode_eFast;

        h->settings.bRfDeltaSigmaOpenDrain = true;
        h->settings.bIfDeltaSigmaOpenDrain = true;
        h->settings.bRfDeltaSigmaInvert = true;
        h->settings.bIfDeltaSigmaInvert = true;
        h->settings.bIfAgcZero = true;
        h->settings.bRfAgcZero = true;

        p3548->bIfdLockStatus = false;
        BVSB_CHK_RETCODE(BVSB_3548_P_AcquireNtsc(h, &inbandParams));
    }

    BKNI_DestroyEvent(((BVSB_3548_P_Handle *)(h->pImpl))->hLockChangeEvent);
    BKNI_DestroyEvent(((BVSB_3548_P_Handle *)(h->pImpl))->hInitDoneEvent);
    BKNI_DestroyEvent(((BVSB_3548_P_Handle *)(h->pImpl))->hHabDoneEvent);
    BKNI_DestroyEvent(((BVSB_3548_P_Handle *)(h->pImpl))->hSignalLevelChangeEvent);
    BKNI_DestroyEvent(((BVSB_3548_P_Handle *)(h->pImpl))->hVideoKillEvent);
    BKNI_DestroyEvent(((BVSB_3548_P_Handle *)(h->pImpl))->hUnlockEvent);
    BKNI_DestroyEvent(((BVSB_3548_P_Handle *)(h->pImpl))->hLockEvent);

    /* Power off on exit */
    BVSB_3548_P_PowerDown(h);

    /* If Power Management is disabled, this will stop the timer as well */
    BTMR_DestroyTimer(p3548->hTimer);

    BKNI_Free((void*)h->pImpl);
    BKNI_Free((void*)h);

done:
    return retCode;
}

#ifdef BVSB_DEBUG
void BVSB_3548_P_DebugDump(BVSB_Handle h)
{
    uint32_t val;

    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    val = BREG_Read32(p3548->hRegister, BCHP_AP_32_H_CTL1);
    BDBG_MSG(("H_CTL1 = 0x%02X", val & 0xFF)); 
    val = BREG_Read32(p3548->hRegister, BCHP_AP_32_H_STAT2);
    BDBG_MSG(("H_STAT2 = 0x%02X", val & 0xFF)); 
    val = BREG_Read32(p3548->hRegister, BCHP_AP_32_H_STAT1);
    BDBG_MSG(("H_STAT1 = 0x%02X", val & 0xFF)); 
    val = BREG_Read32(p3548->hRegister, BCHP_AP_32_H_IE1);
    BDBG_MSG(("H_IE1 = 0x%02X", val & 0xFF)); 
    val = BREG_Read32(p3548->hRegister, BCHP_AP_32_H_IE2);
    BDBG_MSG(("H_IE2 = 0x%02X", val & 0xFF)); 
    val = BREG_Read32(p3548->hRegister, BCHP_AP_32_H_MSG1);
    BDBG_MSG(("H_MSG1 = 0x%02X", val & 0xFF)); 
    val = BREG_Read32(p3548->hRegister, BCHP_AP_32_H_MSG2);
    BDBG_MSG(("H_MSG2 = 0x%02X", val & 0xFF)); 
}
#endif


/******************************************************************************
  BVSB_3548_P_InitAp()
 ******************************************************************************/
BERR_Code BVSB_3548_P_InitAp(
        BVSB_Handle h,            /* [in] BVSB handle */
        const uint8_t *pHexImage  /* [in] pointer to BCM3548 microcode image */
        )
{
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    BERR_Code retCode;
    uint16_t n, addr;
    const uint8_t *pImage;

    BVSB_3548_P_PowerEnable(h, 0);

    /* reset the AP before downloading the microcode */
    BVSB_CHK_RETCODE(BVSB_3548_P_ResetAp(h));

    /* Enable interrupt callback */
    BVSB_CHK_RETCODE(BINT_EnableCallback(p3548->hInitApCallback));

    /* download to RAM */
	if (pHexImage == NULL)
	{
		pImage = bcm3548_ap_image;
		BDBG_MSG(("Loading default image\n"));
	}
	else
		pImage = pHexImage;

    while (1)
    {
        n = (*pImage++ << 8);
        n |= *pImage++;

        if (n == 0)
            break;

        addr = (*pImage++ << 8);
        addr |= *pImage++;

        /* write n bytes from pImage to addr */
        BVSB_3548_P_WriteMemory(h, addr, (uint8_t*)pImage, n);

        pImage += n;
    }

    /* start running the AP */
    if ((retCode = BVSB_3548_P_RunAp(h)) != BERR_SUCCESS)
        goto done;

    /* wait for init done interrupt */
    do
    {
        retCode = BKNI_WaitForEvent(p3548->hInitDoneEvent, 500);
    } while ((retCode != BERR_TIMEOUT) && (retCode != BERR_SUCCESS));

    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("AP initialization timeout\n")); 
        BERR_TRACE(retCode = BVSB_ERR_AP_NOT_INIT);            
#ifdef BVSB_DEBUG
        BVSB_3548_P_DebugDump(h);
#endif
    }

    /* Done with InitAp callback */
    BVSB_CHK_RETCODE(BINT_DisableCallback(p3548->hInitApCallback));
    BVSB_CHK_RETCODE(BINT_ClearCallback(p3548->hInitApCallback));

    retCode = BVSB_3548_P_SetThreshold(h);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("Set thresholds for channel search failed!\n"));
        return retCode;
    }

    /* Set initial PGA Gain */
    BVSB_3548_P_SetPgaGain(h, BVSB_InbandMode_e8VSB, h->settings.vsb.pgaGain);
    BVSB_3548_P_SetPgaGain(h, BVSB_InbandMode_e256QAM_B, h->settings.qam.pgaGain);
    BVSB_3548_P_SetPgaGain(h, BVSB_InbandMode_eAnalog, h->settings.ntsc.pgaGain);

done:
    return retCode;
}


/******************************************************************************
  BVSB_3548_P_GetApStatus()
 ******************************************************************************/
BERR_Code BVSB_3548_P_GetApStatus(
        BVSB_Handle h,           /* [in] VSB device handle */
        BVSB_ApStatus *pStatus   /* [out] AP status */
        )
{
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    uint32_t h_ctl1, h_stat1, h_stat2;

    h_ctl1 = BREG_Read32(p3548->hRegister, BCHP_AP_32_H_CTL1) & 0xFF;
    h_stat1 = BREG_Read32(p3548->hRegister, BCHP_AP_32_H_STAT1) & 0xFF;
    h_stat2 = BREG_Read32(p3548->hRegister, BCHP_AP_32_H_STAT2) & 0xFF;

    *pStatus = h_ctl1 | (h_stat1 << 8) | (h_stat2 << 16);

    return BERR_SUCCESS;
}


/******************************************************************************
  BVSB_3548_P_GetApVersion()
 ******************************************************************************/
BERR_Code BVSB_3548_P_GetApVersion(
        BVSB_Handle h,          /* [in] BVSB handle */
        uint16_t    *pChipId,   /* [out] BVSB chip ID */
        uint16_t    *pChipVer,  /* [out] chip revision number */
        uint8_t     *pApVer,    /* [out] AP microcode version */
        uint8_t     *pScrVer,   /* [out] acquisition script version */
        uint8_t     *pCfgVer    /* [out] host configuration version */
        )
{
    BERR_Code retCode;
    uint8_t buf[16];

    buf[0] = 0x09;
    buf[14] = 0x00;
    BVSB_CHK_RETCODE(BVSB_3548_P_SendHabCommand(h, buf, 15, buf, 14, true));

    *pChipId = ((buf[1] << 8) | buf[2]);
    *pChipVer = ((buf[3] << 8) | buf[4]);
    *pApVer = buf[5];
    *pScrVer = buf[6];
    *pCfgVer = buf[7];

done:
    return retCode;
}


/******************************************************************************
  BVSB_3548_P_ReadRegister() 
 ******************************************************************************/
BERR_Code BVSB_3548_P_ReadRegister(
        BVSB_Handle h,     /* [in] BVSB handle */
        uint32_t    reg,   /* [in] address of register to read */
        uint32_t    *val   /* [in] contains data that was read */
        )
{
    BERR_Code retCode;
    BVSB_ApStatus status;
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    uint8_t buf[16];

    BVSB_CHK_RETCODE(BVSB_GetApStatus(h, &status));
    if ((status & 0x00800007) == 0x00800000)
    {
        buf[0] = 0x07;
        buf[1] = 0x10;
        buf[2] = (reg >> 16) & 0xFF;
        buf[3] = (reg >> 8) & 0xFF;
        buf[4] = reg & 0xFF;
        buf[9] = 0;
        BVSB_CHK_RETCODE(BVSB_3548_P_SendHabCommand(h, buf, 10, buf, 10, true));
        *val = (buf[5] << 24) | (buf[6] << 16) | (buf[7] << 8) | buf[8];
    }
    else
    {
        *val = BREG_Read32(p3548->hRegister, reg & 0x00FFFFFF);
    }

done:
    return BERR_SUCCESS;
}


/******************************************************************************
  BVSB_3548_P_WriteRegister()
 ******************************************************************************/
BERR_Code BVSB_3548_P_WriteRegister(
        BVSB_Handle h,     /* [in] BVSB handle */
        uint32_t    reg,   /* [in] address of register to write */
        uint32_t    *val   /* [in] contains data to write */
        )
{
    BERR_Code retCode;
    BVSB_ApStatus status;
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    uint8_t buf[16];

    BVSB_CHK_RETCODE(BVSB_GetApStatus(h, &status));
    if ((status & 0x00800007) == 0x00800000)
    {
        buf[0] = 0x08;
        buf[1] = 0x10;
        buf[2] = (reg >> 16) & 0xFF;
        buf[3] = (reg >> 8) & 0xFF;
        buf[4] = reg & 0xFF;
        buf[5] = (*val >> 24) & 0xFF;
        buf[6] = (*val >> 16) & 0xFF;
        buf[7] = (*val >> 8) & 0xFF;
        buf[8] = *val & 0xFF;
        buf[9] = 0;
        BVSB_CHK_RETCODE(BVSB_3548_P_SendHabCommand(h, buf, 10, buf, 1, true));
    }
    else
    {
        BREG_Write32(p3548->hRegister, reg & 0x00FFFFFF, *val);
    }

done:
    return retCode;
}


/******************************************************************************
  BVSB_3548_P_AcquireInband()
 ******************************************************************************/
BERR_Code BVSB_3548_P_AcquireInband(
        BVSB_Handle h,                    /* [in] BVSB handle */
        const BVSB_InbandParams *pParams  /* [in] inband acquisition parameters */
        )
{   
    BERR_Code retCode;
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    uint8_t   buf[8];

    BDBG_ASSERT(h);
    BDBG_ASSERT(pParams);

    /* Enable power before any acquistion. In case it comes from power off state */
    BVSB_3548_P_PowerEnable(h, BVSB_POWER_AP);

    /* Stop monitor timer before acquisition starts */
    BTMR_StopTimer(p3548->hTimer);

    /* First set Delta Sigma */
    BVSB_3548_P_ReadConfig(h, BVSB_CONFIG_PIN_CTRL_3, buf, 1);

    if (h->settings.bRfDeltaSigmaOpenDrain)
        buf[0] &= 0x7F; /* zero out bit 7 */

    if (h->settings.bIfDeltaSigmaOpenDrain)
        buf[0] &= 0xDF; /* zero out bit 5 */

    BVSB_3548_P_WriteConfig(h,BVSB_CONFIG_PIN_CTRL_3, buf, 1);

    switch (pParams->mode)
    {
        case BVSB_InbandMode_e8VSB:      /* 8-VSB */
        case BVSB_InbandMode_e16VSB:     /* 16-VSB */
            BVSB_CHK_RETCODE(BVSB_3548_P_AcquireVsb(h, pParams));
            break;

	  	case BVSB_InbandMode_e256QAM_A:  /* 256-QAM Annex A */
        case BVSB_InbandMode_e128QAM_A:  /* 128-QAM Annex A */
        case BVSB_InbandMode_e64QAM_A:    /* 64-QAM Annex A  */
        case BVSB_InbandMode_e32QAM_A:    /* 32-QAM Annex A  */
        case BVSB_InbandMode_e16QAM_A:    /* 16-QAM Annex A  */
		  h->settings.qam.bCh = 1;
		  BVSB_3548_P_SetQamSymbol(h, pParams->symbolRate);
        case BVSB_InbandMode_e1024QAM_B: /* 1024-QAM Annex B */
        case BVSB_InbandMode_e256QAM_B:  /* 256-QAM Annex B */
        case BVSB_InbandMode_e64QAM_B:    /* 64-QAM Annex B  */
        case BVSB_InbandMode_e64_256QAM_B: /* QAM Scan mode */
            BVSB_CHK_RETCODE(BVSB_3548_P_AcquireQam(h, pParams));
            h->settings.qam.bCh = 0;
            break;
        case BVSB_InbandMode_eAnalog:      /* NTSC */
		case BVSB_InbandMode_eAnalog_SIF:
            p3548->bIfdLockStatus = false;
            BVSB_CHK_RETCODE(BVSB_3548_P_AcquireNtsc(h, pParams));
            break;

        default:
            return (BERR_TRACE(BERR_INVALID_PARAMETER)); 
    }
    BVSB_CHK_RETCODE(BINT_EnableCallback(p3548->hLockCallback));
    BVSB_CHK_RETCODE(BINT_EnableCallback(p3548->hUnlockCallback));

done:
    return retCode;
}


/******************************************************************************
  BVSB_3548_P_GetVsbStatus()
 ******************************************************************************/
BERR_Code BVSB_3548_P_GetVsbStatus(
        BVSB_Handle h,           /* [in] BVSB handle */
        BVSB_VsbStatus *pStatus  /* [out] VSB status */
        )
{
    BERR_Code retCode;
    uint8_t   buf[128];
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    if (true == p3548->bPowerdown)
        return BVSB_ERR_POWER_DOWN;

    BVSB_3548_P_ReadConfig(h, BVSB_CONFIG_ACQUIRE_MODE, buf, 1);
    if (buf[0] != BVSB_MODE_VSB)
        return BVSB_ERR_INVALID_STATUS_CALL;

    if (pStatus == NULL)
        return (BERR_TRACE(BERR_INVALID_PARAMETER));

    BKNI_Memset(buf, 0, 128);
    buf[0] = 0x1E;
    BVSB_CHK_RETCODE(BVSB_3548_P_SendHabCommand(h, buf, 0x40, buf, 0x3D, true));

    switch ((buf[2] >> 4) & 0x07)
    {
        case 0:
            pStatus->acqParams.mode = BVSB_InbandMode_e8VSB;
            break;

        case 4:
            pStatus->acqParams.mode = BVSB_InbandMode_e16VSB;
            break;

        default:
            pStatus->acqParams.mode = BVSB_InbandMode_eUnknown;
            break;
    }

    switch ((buf[2] >> 1) & 0x03)
    {
        case 0:
            pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eLow;
            break;
        case 1:
            pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eMedium;
            break;
        default:
            pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eHigh;
            break;
    }

    pStatus->acqParams.symbolRateOffset = (int32_t)((int16_t)((buf[0x08] << 8) + buf[0x09]));
    pStatus->acqParams.ifFreqOffset = (int32_t)((buf[0x04] << 24) + (buf[0x05] << 16) + (buf[0x06] << 8) + buf[0x07]);
    pStatus->acqSettings.bAutoAcq = buf[0x01] & 0x20 ? true : false;
    pStatus->acqSettings.bFastAcq = buf[0x01] & 0x01 ? true : false;
    pStatus->acqSettings.bTei = buf[0x02] & 0x01 ? true : false;
    pStatus->acqSettings.bTerr = buf[0x02] & 0x80 ? true : false;
    pStatus->acqSettings.bNtscSweep = buf[0x01] & 0x08 ? true : false;
    pStatus->acqSettings.bRfiSweep = buf[0x01] & 0x04 ? true : false;

    pStatus->bFecLocked = buf[0x12] & 0x04 ? true : false;
    pStatus->bPllLocked = buf[0x12] & 0x02 ? true : false;
    pStatus->bNtscEngaged = buf[0x12] & 0x20 ? true : false;
    pStatus->bRfiEngaged = buf[0x12] & 0x10 ? true : false;
    pStatus->ifFreq = (buf[0x0A] << 24) + (buf[0x0B] << 16) + (buf[0x0C] << 8) + buf[0x0D];
    pStatus->ifFreqError = (int32_t)((buf[0x17] << 24) + (buf[0x18] << 16) + (buf[0x19] << 8) + buf[0x1A]);
    pStatus->symbolRate = (int32_t)((buf[0x0E] << 24) + (buf[0x0F] << 16) + (buf[0x10] << 8) + buf[0x11]);
    pStatus->symbolRateError = (int32_t)((int16_t)((buf[0x1B] << 8) + buf[0x1C]));
    pStatus->corrErrCount = (buf[0x29] << 24) + (buf[0x2A] << 16) + (buf[0x2B] << 8) + buf[0x2C];
    pStatus->corrBlockCount = (buf[0x2D] << 24) + (buf[0x2E] << 16) + (buf[0x2F] << 8) + buf[0x30];
    pStatus->ucorrBlockCount = (buf[0x31] << 24) + (buf[0x32] << 16) + (buf[0x33] << 8) + buf[0x34];
    pStatus->cleanBlockCount = (buf[0x35] << 24) + (buf[0x36] << 16) + (buf[0x37] << 8) + buf[0x38];
    pStatus->berCount = (buf[0x25] << 24) + (buf[0x26] << 16) + (buf[0x27] << 8) + buf[0x28];
    pStatus->snr = (buf[0x13] << 8) + buf[0x14];
    pStatus->agf = (buf[0x1D] << 8) + buf[0x1E];
    pStatus->rf_agc = (((buf[0x1F] << 8) + buf[0x20]) * 1000) / 65535;
    pStatus->if_agc = (((buf[0x21] << 8) + buf[0x22]) * 1000) / 65535;
    pStatus->reacqCount = (buf[0x39] << 24) + (buf[0x3A] << 16) + (buf[0x3B] << 8) + buf[0x3C];
    if(buf[0x25] <= 0x80)
    {
	    pStatus->BER = (buf[0x25] << 24) + (buf[0x26] << 16) + (buf[0x27] << 8) + buf[0x28];
    }
    else
    {
	    pStatus->BER = 0; /*over flow*/
    }
	 
    retCode = BERR_SUCCESS;

done:
    return retCode;
}

/******************************************************************************
  BVSB_3548_P_GetQamStatus()
 ******************************************************************************/
BERR_Code BVSB_3548_P_GetQamStatus(
        BVSB_Handle h,           /* [in] BVSB handle */
        BVSB_QamStatus *pStatus  /* [out] QAM status   */
        )
{
    BERR_Code retCode;
    uint8_t   buf[128];
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    if (true == p3548->bPowerdown)
        return BVSB_ERR_POWER_DOWN;

    if (pStatus == NULL)
        return (BERR_TRACE(BERR_INVALID_PARAMETER));

    BVSB_3548_P_ReadConfig(h, BVSB_CONFIG_ACQUIRE_MODE, buf, 1);
    if (buf[0] != BVSB_MODE_QAM)
        return BVSB_ERR_INVALID_STATUS_CALL;

    BKNI_Memset(buf, 0, 128);
    buf[0] = 0x20;
    BVSB_CHK_RETCODE(BVSB_3548_P_SendHabCommand(h, buf, 0x40, buf, 0x3D, true));

    if (buf[2] & 0x08)
    {
        /* annex B */
        switch ((buf[2] >> 4) & 0x07)
        {
            case 3:
                pStatus->acqParams.mode = BVSB_InbandMode_e64QAM_B;
                break;

            case 5:
                pStatus->acqParams.mode = BVSB_InbandMode_e256QAM_B;
                break;

            default:
                pStatus->acqParams.mode = BVSB_InbandMode_e1024QAM_B;
                break;
        }      
    }
    else
    {
        /* annex A */
        switch ((buf[2] >> 4) & 0x07)
        {
            case 1:
                pStatus->acqParams.mode = BVSB_InbandMode_e16QAM_A;
                break;

            case 2:
                pStatus->acqParams.mode = BVSB_InbandMode_e32QAM_A;
                break;

            case 3:
                pStatus->acqParams.mode = BVSB_InbandMode_e64QAM_A;
                break;

            case 4:
                pStatus->acqParams.mode = BVSB_InbandMode_e128QAM_A;
                break;

            default:
                pStatus->acqParams.mode = BVSB_InbandMode_e256QAM_A;
                break;
        }            
    }

    switch ((buf[1] >> 6) & 0x03)
    {
        case 0:
            pStatus->acqSettings.nyquist = BVSB_NyquistFilter_e12;
            break;

        case 1:
            pStatus->acqSettings.nyquist = BVSB_NyquistFilter_e15;
            break;

        default:
            pStatus->acqSettings.nyquist = BVSB_NyquistFilter_e18;
            break;
    }

    switch ((buf[2] >> 1) & 0x03)
    {
        case 0:
            pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eLow;
            break;
        case 1:
            pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eMedium;
            break;
        default:
            pStatus->acqSettings.bw = BVSB_PhaseLoopBw_eHigh;
            break;
    }

    switch ((buf[3] >> 4) & 0x0F)
    {
        case 0:
            pStatus->acqSettings.idepth = BVSB_IDepth_e204_1;
            break;

        case 1:
            pStatus->acqSettings.idepth = BVSB_IDepth_e102_2;
            break;

        case 2:
            pStatus->acqSettings.idepth = BVSB_IDepth_e68_3;
            break;

        case 3:
            pStatus->acqSettings.idepth = BVSB_IDepth_e51_4;
            break;

        case 4:
            pStatus->acqSettings.idepth = BVSB_IDepth_e34_6;
            break;

        case 5:
            pStatus->acqSettings.idepth = BVSB_IDepth_e17_12;
            break;

        case 6:
            pStatus->acqSettings.idepth = BVSB_IDepth_e12_17;
            break;

        case 7:
            pStatus->acqSettings.idepth = BVSB_IDepth_e6_34;
            break;

        case 8:
            pStatus->acqSettings.idepth = BVSB_IDepth_e4_51;
            break;

        case 9:
            pStatus->acqSettings.idepth = BVSB_IDepth_e3_68;
            break;

        case 10:
            pStatus->acqSettings.idepth = BVSB_IDepth_e2_102;
            break;

        default:
            pStatus->acqSettings.idepth = BVSB_IDepth_e1_204;
            break;
    }

    pStatus->acqParams.symbolRateOffset = (int32_t)((int16_t)((buf[0x08] << 8) + buf[0x09]));
    pStatus->acqParams.ifFreqOffset = (int32_t)((buf[0x04] << 24) + (buf[0x05] << 16) + (buf[0x06] << 8) + buf[0x07]);
    pStatus->acqSettings.bAutoAcq = buf[0x01] & 0x20 ? true : false;
    pStatus->acqSettings.bFastAcq = buf[0x01] & 0x01 ? true : false;
    pStatus->acqSettings.bTei = buf[0x02] & 0x01 ? true : false;
    pStatus->acqSettings.bTerr = buf[0x02] & 0x80 ? true : false;
    pStatus->acqSettings.bSpinv = buf[0x01] & 0x10 ? true : false;
    pStatus->acqSettings.bEq = buf[0x01] & 0x04 ? true : false;
    pStatus->acqSettings.bCh = buf[0x01] & 0x02 ? true : false;
    pStatus->acqSettings.bDavic = buf[0x03] & 0x08 ? true : false;
    pStatus->bFecLocked = buf[0x12] & 0x04 ? true : false;
    pStatus->bPllLocked = buf[0x12] & 0x02 ? true : false;
    pStatus->bSpinv = (buf[0x12] & 0x01 ? true : false);
    pStatus->ifFreq = (buf[0x0A] << 24) + (buf[0x0B] << 16) + (buf[0x0C] << 8) + buf[0x0D];
    pStatus->ifFreqError = (int32_t)((buf[0x17] << 24) + (buf[0x18] << 16) + (buf[0x19] << 8) + buf[0x1A]);
    pStatus->symbolRate = (int32_t)((buf[0x0E] << 24) + (buf[0x0F] << 16) + (buf[0x10] << 8) + buf[0x11]);
    pStatus->symbolRateError = (int32_t)((int16_t)((buf[0x1B] << 8) + buf[0x1C]));
    pStatus->corrErrCount = (buf[0x29] << 24) + (buf[0x2A] << 16) + (buf[0x2B] << 8) + buf[0x2C];
    pStatus->corrBlockCount = (buf[0x2D] << 24) + (buf[0x2E] << 16) + (buf[0x2F] << 8) + buf[0x30];
    pStatus->ucorrBlockCount = (buf[0x31] << 24) + (buf[0x32] << 16) + (buf[0x33] << 8) + buf[0x34];
    pStatus->cleanBlockCount = (buf[0x35] << 24) + (buf[0x36] << 16) + (buf[0x37] << 8) + buf[0x38];
    pStatus->berCount = (buf[0x25] << 24) + (buf[0x26] << 16) + (buf[0x27] << 8) + buf[0x28];
    pStatus->snr = (buf[0x13] << 8) + buf[0x14];
    pStatus->agf = (buf[0x1D] << 8) + buf[0x1E];
    pStatus->rf_agc = (((buf[0x1F] << 8) + buf[0x20]) * 1000) / 65535;
    pStatus->if_agc = (((buf[0x21] << 8) + buf[0x22]) * 1000) / 65535;
    pStatus->reacqCount = (buf[0x39] << 24) + (buf[0x3A] << 16) + (buf[0x3B] << 8) + buf[0x3C];
    if(buf[0x25] <= 0x80)
    {
	    pStatus->BER = (buf[0x25] << 24) + (buf[0x26] << 16) + (buf[0x27] << 8) + buf[0x28];
    }
    else
    {
	    pStatus->BER = 0; /*over flow*/
    }

    retCode = BERR_SUCCESS;
done:
    return retCode;
}

/******************************************************************************
  BVSB_3548_P_ResetInbandStatus()
 ******************************************************************************/
BERR_Code BVSB_3548_P_ResetInbandStatus(
        BVSB_Handle h /* [in] BVSB handle */
        )
{
    BERR_Code retCode;
    uint8_t buf[3];

    buf[0] = 0x1F;
    buf[1] = 0x04;
    buf[2] = 0;
    BVSB_CHK_RETCODE(BVSB_3548_P_SendHabCommand(h, buf, 3, buf, 1, true));

done:
    return retCode;
}

/******************************************************************************
 BVSB_3548_P_GetBtscStatus()
******************************************************************************/
BERR_Code BVSB_3548_P_GetBtscStatus(
    BVSB_Handle h,              /* [in] BVSB handle */
    BVSB_BtscStatus *pStatus /* [out] BTSC status */
)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(pStatus);
   return BERR_SUCCESS;
    
}

/******************************************************************************
  BVSB_3548_P_SetInbandOi()
 ******************************************************************************/
BERR_Code BVSB_3548_P_SetInbandOi(
        BVSB_Handle           h,         /* [in] BVSB handle */
        BVSB_InbandOiSettings *pInbandOi /* [in] inband transport output interface settings */
        )
{
    BERR_Code retCode;
    uint8_t buf[4];

    buf[0] = 0x0F;
    buf[1] = buf[2] = buf[3] = 0;
    buf[1] |= (pInbandOi->bHead4 ? 0x40 : 0x00); 
    buf[1] |= (pInbandOi->bSync1 ? 0x20 : 0x00); 
    buf[1] |= (pInbandOi->bXBERT ? 0x04 : 0x00); 
    buf[2] |= (pInbandOi->bErrinv ? 0x80 : 0x00); 
    buf[2] |= (pInbandOi->bSyncinv ? 0x40 : 0x00); 
    buf[2] |= (pInbandOi->bVldinv ? 0x20 : 0x00); 
    buf[2] |= (pInbandOi->bClksup ? 0x10 : 0x00); 
    buf[2] |= (pInbandOi->bClkinv ? 0x08 : 0x00); 
    buf[2] |= (pInbandOi->bSerial ? 0x02 : 0x00); 

    BVSB_CHK_RETCODE(BVSB_3548_P_SendHabCommand(h, buf, 4, buf, 1, true));

done:
    return retCode;
}


/******************************************************************************
  BVSB_3548_P_GetSoftDecisionBuf()
 ******************************************************************************/
BERR_Code BVSB_3548_P_GetSoftDecisionBuf(
        BVSB_Handle h,  /* [in] BVSB handle */
        int16_t *pI,    /* [out] 30 I-values */
        int16_t *pQ     /* [out] 30 Q-values */
        )
{

    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    uint32_t regval;
    uint8_t  i;

    if (true == p3548->bPowerdown)
        return BVSB_ERR_POWER_DOWN;

    for (i = 0; i < 30; i++)
    {
        regval = BREG_Read32(p3548->hRegister, BCHP_DS_TER_EQ_SOFT);
        pI[i] = (regval >> 20) & 0x0FFF;
        pQ[i] = (regval >> 4) & 0x0FFF;
        if (pI[i] & 0x800)
            pI[i] -= 0x1000;
        if (pQ[i] & 0x800)
            pQ[i] -= 0x1000;
    }

    return BERR_SUCCESS;
}

/******************************************************************************
  BVSB_3548_P_WriteConfig()
 ******************************************************************************/
BERR_Code BVSB_3548_P_WriteConfig(
        BVSB_Handle h,   /* [in] BVSB handle */
        uint16_t offset, /* [in] 16-bit offset within the host configuration space */
        uint8_t *buf,    /* [in] data to write */
        uint8_t len      /* [in] number of bytes to write */
        )
{
    BERR_Code retCode;
    uint8_t   hab[128], i;

    if ((len < 1) || (len > 121))
        return (BERR_TRACE(BERR_INVALID_PARAMETER));

    hab[0] = 0x13;
    hab[1] = (offset >> 8);
    hab[2] = (offset & 0xFF);   
    hab[3] = len;
    hab[4+len] = 0x00;

    for (i = 0; i < len; i++)
        hab[4+i] = buf[i];

    BVSB_CHK_RETCODE(BVSB_3548_P_SendHabCommand(h, hab, 5+len, hab, 1, true));

done:
    return retCode;
}


/******************************************************************************
  BVSB_3548_P_ReadConfig()
 ******************************************************************************/
BERR_Code BVSB_3548_P_ReadConfig(
        BVSB_Handle h,  /* [in] BVSB handle */
        uint16_t offset,   /* [in] 16-bit offset within the host configuration space */
        uint8_t *buf,      /* [in] buffer to hold the read data */
        uint8_t len        /* [in] number of bytes to read */
        )
{
    BERR_Code retCode;
    uint8_t   hab[128], i;

    if ((len < 1) || (len > 121))
        return (BERR_TRACE(BERR_INVALID_PARAMETER));

    hab[0] = 0x12;
    hab[1] = (offset >> 8);
    hab[2] = (offset & 0xFF);   
    hab[3] = len;
    hab[4+len] = 0;

    BVSB_CHK_RETCODE(BVSB_3548_P_SendHabCommand(h, hab, 5+len, hab, 5+len, true));

    for (i = 0; i < len; i++)
        buf[i] = hab[4+i];

done:
    return retCode;
}


/******************************************************************************
  BVSB_3548_P_GetUnlockstatusEventHandle()
 ******************************************************************************/
BERR_Code BVSB_3548_P_GetUnlockstatusEventHandle(
        BVSB_Handle h,            /* [in] BVSB handle */
        BKNI_EventHandle *hEvent  /* [out] lock event handle */
        )
{
    *hEvent = ((BVSB_3548_P_Handle *)(h->pImpl))->hUnlockEvent;
    return BERR_SUCCESS;
}

/******************************************************************************
  BVSB_3548_P_GetUnlockstatusEventHandle()
 ******************************************************************************/
BERR_Code BVSB_3548_P_GetLockstatusEventHandle(
        BVSB_Handle h,            /* [in] BVSB handle */
        BKNI_EventHandle *hEvent  /* [out] lock event handle */
        )
{
    *hEvent = ((BVSB_3548_P_Handle *)(h->pImpl))->hLockEvent;
    return BERR_SUCCESS;
}
/******************************************************************************
  BVSB_3548_P_GetLockStateChangeEventHandle()
 ******************************************************************************/
BERR_Code BVSB_3548_P_GetLockStateChangeEventHandle(
        BVSB_Handle h,            /* [in] BVSB handle */
        BKNI_EventHandle *hEvent  /* [out] lock event handle */
        )
{
    *hEvent = ((BVSB_3548_P_Handle *)(h->pImpl))->hLockChangeEvent;
    return BERR_SUCCESS;
}

/******************************************************************************
  BVSB_3548_P_ReadHab()
 ******************************************************************************/
BERR_Code BVSB_3548_P_ReadHab(BVSB_Handle h, uint8_t addr, uint8_t *buf, uint8_t n)
{
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    uint8_t i;

    if ((addr & 0x80) || (n > 128))
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    if ((addr + n) > 0x80)
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    for (i = addr; i < (addr+n); i++)
    {
        buf[i] = (uint8_t)(BREG_Read32(p3548->hRegister, BVSB_AP_HAB+(i*4)) & 0x000000FF);
    }
    BDBG_MSG(("hab reading %x\n", buf[0])); 

    return BERR_SUCCESS;
}


/******************************************************************************
  BVSB_3548_P_WriteHab()
 ******************************************************************************/
BERR_Code BVSB_3548_P_WriteHab(BVSB_Handle h, uint8_t addr, uint8_t *buf, uint8_t n)
{
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    uint8_t i;

    if ((addr & 0x80) || (n & 0x80))
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    if ((addr + n) > 0x80)
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    BDBG_MSG(("Writing HAB: %x\n", buf[0]));
    for (i = addr; i < (addr+n); i++)
    {
        BREG_Write32(p3548->hRegister, BVSB_AP_HAB+(i*4), (uint32_t)(buf[i]));
    }
    BDBG_MSG(("\n", buf[i]));

    return BERR_SUCCESS;
}


/******************************************************************************
  BVSB_3548_P_ResetAp()
 ******************************************************************************/
BERR_Code BVSB_3548_P_ResetAp(BVSB_Handle h)
{
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t regval;
    
    /* initialize JDEC to boot from internal RAM */
    BREG_Write32(p3548->hRegister, BCHP_AP_32_JDEC, 2);

    /* reset the AP */
    BREG_Write32(p3548->hRegister, BCHP_AP_32_H_CTL1, 0x01);

    /* reset AP status */
    BREG_Write32(p3548->hRegister, BCHP_AP_32_AP_CMD, 0xB);

    /* reset all interrupt status */
    BREG_Write32(p3548->hRegister, BCHP_AP_32_H_STAT1, 0xFF);
    BREG_Write32(p3548->hRegister, BCHP_AP_32_H_STAT2, 0xFF);
    BREG_Write32(p3548->hRegister, BCHP_AP_32_H_STAT3, 0xFF);
    BREG_Write32(p3548->hRegister, BCHP_AP_32_H_STAT4, 0xFF);

    /* clear MSGx registers */
    BREG_Write32(p3548->hRegister, BCHP_AP_32_H_MSG1, 0);
    BREG_Write32(p3548->hRegister, BCHP_AP_32_H_MSG2, 0);

    /* verify that AP is reset */
    regval = BREG_Read32(p3548->hRegister, BCHP_AP_32_H_CTL1);
    if ((regval & 0x03) != 1)
    {
        BDBG_ERR(("unable to reset the AP\n"));
        BERR_TRACE(retCode = BVSB_ERR_AP_FAIL);
    }

    return retCode;
}


/******************************************************************************
  BVSB_3548_P_RunAp()
 ******************************************************************************/
BERR_Code BVSB_3548_P_RunAp(BVSB_Handle h)
{
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t  regval;

    /* check if the AP is currently running */
    regval = BREG_Read32(p3548->hRegister, BCHP_AP_32_H_CTL1); 

    if ((regval & 0x03) != 0)
    {
        /* start running the AP */
        BREG_Write32(p3548->hRegister, BCHP_AP_32_H_CTL1, 0x60);

        /* verify that the AP is running */
        regval = BREG_Read32(p3548->hRegister, BCHP_AP_32_H_CTL1);
        if ((regval & 0x03) != 0)
        {
            BDBG_ERR(("unable to run the AP\n"));
            BERR_TRACE(retCode = BVSB_ERR_AP_FAIL);
            goto done;
        }

        /* clear AP_change_state bit */
        BREG_Write32(p3548->hRegister, BCHP_AP_32_H_STAT1, 1); 
    }
    else
    {
        BDBG_WRN(("BVSB_3548_P_RunAp(): AP already running\n"));
    }

done:
    return retCode;
}

/******************************************************************************
  BVSB_3548_P_DecodeError()
 ******************************************************************************/
BERR_Code BVSB_3548_P_DecodeError(
        BVSB_Handle h,           /* [in] BVSB PI Handle */
        BVSB_ApStatus *pApStatus /* [in] AP status returned by BVSB_GetApStatus */
        )
{
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t mb;

    if (*pApStatus & BVSB_APSTATUS_HAB_ERR)
        retCode = BVSB_ERR_HABAV;
    else if (*pApStatus & BVSB_APSTATUS_MEM_ERR)
        retCode = BVSB_ERR_MEMAV;
    else if (*pApStatus & BVSB_APSTATUS_H_ERR)
        retCode = BVSB_ERR_HOST_XFER;
    else if (*pApStatus & BVSB_APSTATUS_IOMB_ERR)
        retCode = BVSB_ERR_IOMB_XFER;
    else if (*pApStatus & BVSB_APSTATUS_HABCMD_ERR)
        retCode = BVSB_ERR_HAB_ERR;
    else if (*pApStatus & BVSB_APSTATUS_AP_ERR)
    {
        mb = BREG_Read32(p3548->hRegister, BCHP_AP_32_H_MSG1);
		mb &= 0xFF;
        switch (mb)
        {
            case 1:
                retCode = BVSB_ERR_AP_BSC;
                break;

            case 2:
                retCode = BVSB_ERR_AP_STACK;
                break;

            case 3:
                retCode = BVSB_ERR_AP_WD;
                break;

            case 4:
                retCode = BVSB_ERR_AP_ISB;
                break;

            case 5:
                retCode = BVSB_ERR_AP_SCR;
                break;

            case 6:
                retCode = BVSB_ERR_AP_IRQ;
                break;

            case 7:
                retCode = BVSB_ERR_AP_COPY;
                break;

            case 8:
                retCode = BVSB_ERR_AP_EEPROM;
                break;

            case 9:
                retCode = BVSB_ERR_AP_HABAV;
                break;

            default:
                BDBG_ERR(("unknown MSG1 (=0x%02X)\n", mb));
                retCode = BVSB_ERR_AP_UNKNOWN;
                break;
        }
    }

    return retCode;
}


/******************************************************************************
  BVSB_3548_P_SendHabCommand()
 ******************************************************************************/
BERR_Code BVSB_3548_P_SendHabCommand(
        BVSB_Handle h,      /* [in] BVSB PI Handle */
        uint8_t *write_buf, /* [in] specifies the HAB command to send */
        uint8_t write_len,  /* [in] number of bytes in the HAB command */ 
        uint8_t *read_buf,  /* [out] holds the data read from the HAB */ 
        uint8_t read_len,   /* [in] number of bytes to read from the HAB */
        bool bCheckForAck   /* [in] true = determine if the AP has serviced the command */
        )
{
    BERR_Code retCode;

    if ((write_len > 127) || (read_len > 127) || (write_len == 0))
        return (BERR_TRACE(BERR_INVALID_PARAMETER));

    BVSB_CHK_RETCODE(BVSB_3548_P_CheckHab(h));

    /* write the command to the HAB */
    BVSB_CHK_RETCODE(BVSB_3548_P_WriteHab(h, 0, write_buf, write_len));

    /* wait for the AP to service the HAB, and then read any return data */
    BVSB_CHK_RETCODE(BVSB_3548_P_ServiceHab(h, read_buf, read_len, bCheckForAck, write_buf[0] | 0x80));

done:
    return retCode;
}


/******************************************************************************
  BVSB_3548_P_ServiceHab()
 ******************************************************************************/
BERR_Code BVSB_3548_P_ServiceHab(
        BVSB_Handle h,   /* [in] BVSB PI Handle */
        uint8_t *read_buf,  /* [out] holds the data read from the HAB */ 
        uint8_t read_len,   /* [in] number of bytes to read from the HAB */
        bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
        uint8_t ack_byte    /* [in] value of the ack byte to expect */
        )
{
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    BERR_Code retCode;

    BKNI_WaitForEvent(p3548->hHabDoneEvent, 0);      

    /* send the command */
	 BVSB_CHK_RETCODE(BINT_EnableCallback(p3548->hHabDoneCallback));
    BREG_Write32(p3548->hRegister, BCHP_AP_32_H_CTL1, 0x64);

    /* wait for HAB done interrupt */  
    do
    {
        retCode = BKNI_WaitForEvent(p3548->hHabDoneEvent, BVSB_HAB_DONE_TIMEOUT);
    } while ((retCode != BERR_TIMEOUT) && (retCode != BERR_SUCCESS));

	 BVSB_CHK_RETCODE(BINT_DisableCallback(p3548->hHabDoneCallback));

    if (retCode == BERR_TIMEOUT)
    {
        BDBG_ERR(("HAB timeout\n"));   
        BERR_TRACE(retCode = BVSB_ERR_HAB_TIMEOUT);
       
        goto done;
    }

    if (read_len > 0)
    {
        BVSB_CHK_RETCODE(BVSB_3548_P_ReadHab(h, 0, read_buf, read_len));
        if (bCheckForAck)
        {
            if (ack_byte != read_buf[0])
            {
                BDBG_ERR(("HAB command not serviced!, got 0x%02X, expected 0x%02X\n", read_buf[0], ack_byte));
                BERR_TRACE(retCode = BVSB_ERR_HAB_NO_ACK);
#if 0 /* debug */
                {
                    int i, j;
                    uint8_t buf[128];
                    for (j = 0; j < 2; j++)
                    { 
                        BVSB_3548_P_ReadHab(h, 0, buf, 128);
                        for (i = 0; i < 128; i++)
                            printf("hab[%d] = 0x%02X\n", i, buf[i]);
                        printf("------\n");
                    }
                }
#endif
            }
        }
    }

done:
    return retCode;
}


/******************************************************************************
  BVSB_3548_P_CheckHab()
 ******************************************************************************/
BERR_Code BVSB_3548_P_CheckHab(
        BVSB_Handle h    /* [in] BVSB PI Handle */
        )
{
    BERR_Code retCode;
    BVSB_ApStatus status;

    BVSB_CHK_RETCODE(BVSB_GetApStatus(h, &status));
    /*if ((status & BVSB_APSTATUS_HAB_MASK) == BVSB_APSTATUS_HAB_READY)*/
    if ((status & 0x000C0007) == 0x00000000)
        retCode = BERR_SUCCESS;
    else
    {
        /* Flag this as critical error */
        retCode = BVSB_3548_P_DecodeError(h, &status);
        BERR_TRACE(retCode);
#ifdef BVSB_DEBUG
        BVSB_3548_P_DebugDump(h);
#endif
    }

done:
    return retCode;
}

/******************************************************************************
  BVSB_3548_P_AcquireVsb()
 ******************************************************************************/
BERR_Code BVSB_3548_P_AcquireVsb(
        BVSB_Handle h,                    /* [in] BVSB handle */
        const BVSB_InbandParams *pParams  /* [in] inband acquisition parameters */
        )
{
    BERR_Code retCode;
    uint8_t   hab[16];
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    hab[0] = 0x0A;
    hab[1] = hab[2] = hab[10] = 0x00;
    hab[3] = 0x01;

    hab[1] |= ((h->settings.vsb.bAutoAcq) ? 0x20 : 0x00);
    hab[1] |= ((h->settings.vsb.bFastAcq) ? 0x01 : 0x00);
    hab[1] |= ((h->settings.vsb.bNtscSweep) ? 0x08 : 0x00);
    hab[1] |= ((h->settings.vsb.bRfiSweep) ? 0x04 : 0x00);
    hab[2] |= ((h->settings.vsb.bTei) ? 0x01 : 0x00);
    hab[2] |= ((h->settings.vsb.bTerr) ? 0x80 : 0x00);

    switch (pParams->mode)
    {
        case BVSB_InbandMode_e8VSB:
            /*hab[2] |= 0x00;*/
            break;

        case BVSB_InbandMode_e16VSB:
            hab[2] |= 0x40;
            break;

        default:
            return (BERR_TRACE(BERR_INVALID_PARAMETER));
    }

    switch (h->settings.vsb.bw)
    {
        case BVSB_PhaseLoopBw_eLow:
            /*hab[2] |= 0x00;*/
            break;

        case BVSB_PhaseLoopBw_eMedium:
            hab[2] |= 0x02;
            break;

        case BVSB_PhaseLoopBw_eHigh:
            hab[2] |= 0x04;
            break;

        default:
            return (BERR_TRACE(BERR_INVALID_PARAMETER));
    }

    if ((pParams->ifFreqOffset < -2097152) || (pParams->ifFreqOffset > 2097151))
        return (BERR_TRACE(BERR_INVALID_PARAMETER));

    if ((pParams->symbolRateOffset < -32768) || (pParams->symbolRateOffset > 32767))
        return (BERR_TRACE(BERR_INVALID_PARAMETER));

    hab[4] = (pParams->ifFreqOffset >> 24);
    hab[5] = (pParams->ifFreqOffset >> 16);
    hab[6] = (pParams->ifFreqOffset >> 8);
    hab[7] = (pParams->ifFreqOffset & 0xFF);

    hab[8] = (pParams->symbolRateOffset >> 8);
    hab[9] = (pParams->symbolRateOffset & 0xFF);

    BVSB_CHK_RETCODE(BVSB_3548_P_SendHabCommand(h, hab, 11, hab, 1, true));

#ifdef BVSB_POWER_MANAGEMENT 
    /* Power off IFD if we are not if SIF mode */
    if (p3548->eAnalogInput != BVSB_AnalogInput_eSif_1)
    {
		BDBG_MSG(("Power down IFD"));
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_RESET, 0x797);
    }
	else
		BDBG_MSG(("In SIF mode, so leave IFD power on"));
#else 
    BSTD_UNUSED(p3548);
#endif

done:
    return retCode;
}


/******************************************************************************
  BVSB_3548_P_AcquireQam()
 ******************************************************************************/
BERR_Code BVSB_3548_P_AcquireQam(
        BVSB_Handle h,                    /* [in] BVSB handle */
        const BVSB_InbandParams *pParams  /* [in] inband acquisition parameters */
        )
{
    BERR_Code retCode;
    uint8_t   hab[16];
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    hab[0] = 0x0B;
    hab[1] = hab[2] = hab[0x0A] = 0x00;
    hab[3] = 0x03;

    hab[1] |= ((h->settings.qam.bAutoAcq) ? 0x20 : 0x00);
    hab[1] |= ((h->settings.qam.bFastAcq) ? 0x01 : 0x00);
    hab[1] |= ((h->settings.qam.bSpinv) ? 0x10 : 0x00);
    hab[1] |= ((h->settings.qam.bEq) ? 0x04 : 0x00);
    hab[1] |= ((h->settings.qam.bCh) ? 0x02 : 0x00);

    hab[2] |= ((h->settings.qam.bTei) ? 0x01 : 0x00);
    hab[2] |= ((h->settings.qam.bTerr) ? 0x80 : 0x00);
    hab[2] |= ((pParams->mode >= BVSB_InbandMode_e256QAM_A) ? 0x00 : 0x08);
    hab[3] |= ((h->settings.qam.bDavic) ? 0x08 : 0x00);
    hab[3] |= (((uint8_t)(h->settings.qam.idepth) << 4) & 0xF0);

    switch (pParams->mode)
    {
        case BVSB_InbandMode_e1024QAM_B:
            hab[2] |= 0x70;
            break;

        case BVSB_InbandMode_e256QAM_B:
            hab[2] |= 0x50;
            break;

        case BVSB_InbandMode_e64QAM_B:
            hab[2] |= 0x30;
            break;

		  case BVSB_InbandMode_e64_256QAM_B:
			   hab[2] &= 0x8f;
			   break;

        case BVSB_InbandMode_e256QAM_A:
            hab[2] |= 0x50;
            break;

        case BVSB_InbandMode_e128QAM_A:
            hab[2] |= 0x40;
            break;

        case BVSB_InbandMode_e64QAM_A:
            hab[2] |= 0x30;
            break;

        case BVSB_InbandMode_e32QAM_A:
            hab[2] |= 0x20;
            break;

        case BVSB_InbandMode_e16QAM_A:
            hab[2] |= 0x10;
            break;

        default:
            return (BERR_TRACE(BERR_INVALID_PARAMETER));
    }

    switch (h->settings.qam.bw)
    {
        case BVSB_PhaseLoopBw_eLow:
            /*hab[2] |= 0x00;*/
            break;

        case BVSB_PhaseLoopBw_eMedium:
            hab[2] |= 0x02;
            break;

        case BVSB_PhaseLoopBw_eHigh:
            hab[2] |= 0x04;
            break;

        default:
            return (BERR_TRACE(BERR_INVALID_PARAMETER));
    }

    switch (h->settings.qam.nyquist)
    {
        case BVSB_NyquistFilter_e12:
            /*hab[1] |= 0x00;*/
            break;

        case BVSB_NyquistFilter_e15:
            hab[1] |= 0x40;
            break;

        case BVSB_NyquistFilter_e18:
            hab[1] |= 0x80;
            break;

        default:
            return (BERR_TRACE(BERR_INVALID_PARAMETER));
    }

    if ((pParams->ifFreqOffset < -2097152) || (pParams->ifFreqOffset > 2097151))
        return (BERR_TRACE(BERR_INVALID_PARAMETER));

    if ((pParams->symbolRateOffset < -32768) || (pParams->symbolRateOffset > 32767))
        return (BERR_TRACE(BERR_INVALID_PARAMETER));

    hab[4] = (pParams->ifFreqOffset >> 24);
    hab[5] = (pParams->ifFreqOffset >> 16);
    hab[6] = (pParams->ifFreqOffset >> 8);
    hab[7] = (pParams->ifFreqOffset & 0xFF);

    hab[8] = (pParams->symbolRateOffset >> 8);
    hab[9] = (pParams->symbolRateOffset & 0xFF);

    BVSB_CHK_RETCODE(BVSB_3548_P_SendHabCommand(h, hab, 11, hab, 1, true));

#ifdef BVSB_POWER_MANAGEMENT 
    /* Power off IFD if we are not if SIF mode */
    if (p3548->eAnalogInput != BVSB_AnalogInput_eSif_1)
    {
		BDBG_MSG(("Power down IFD"));
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_RESET, 0x797);
    }
	else
		BDBG_MSG(("In SIF mode, so leave IFD power on"));
#else
    BSTD_UNUSED(p3548);
#endif


done:
    return retCode;
}


/******************************************************************************
  BVSB_3548_P_AcquireNtsc()
 ******************************************************************************/
BERR_Code BVSB_3548_P_AcquireNtsc(
        BVSB_Handle h,                    /* [in] BVSB handle */
        const BVSB_InbandParams *pParams  /* [in] inband acquisition parameters */
        )
{
#ifdef NO_AP_MODE
	BERR_Code retCode;
	uint8_t hab[5];
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    BVSB_IfdParams ifdParam;

   /* Decode audio/vide mode first */
   BVSB_CHK_RETCODE(BVSB_3548_P_GetIfdMode(pParams->eVideoMode, pParams->eAudioMode, &p3548->eIfdMode));

   /* Decode input source */
   if (pParams->mode == BVSB_InbandMode_eAnalog_SIF)
      p3548->eAnalogInput = BVSB_AnalogInput_eSif_1;
   else
      p3548->eAnalogInput = BVSB_AnalogInput_eIf;

   /* We power everything on include THD because we need to control PGA gain */
   BVSB_3548_P_PowerEnable(h, BVSB_POWER_AP);

    if (p3548->eAnalogInput == BVSB_AnalogInput_eIf)
    {
        /* Idle downstream acquistion if we are going to IFD mode */
        hab[0] = HAB_ACQUIRE_IDLE;
	    hab[1] = 0;
        BDBG_MSG(("Putting AP in idle mode\n"));
		
        BVSB_CHK_RETCODE(BVSB_3548_P_SendHabCommand(h, hab, 2 , hab, 1, true));

    }
    BDBG_MSG(("Acquiring IFD mode: 0x%x\n", p3548->eIfdMode));

    ifdParam.eVideoMode = pParams->eVideoMode;
    ifdParam.eAudioMode = pParams->eAudioMode;
    ifdParam.eAnalogInput = p3548->eAnalogInput;
    ifdParam.ifFreqOffset = pParams->ifFreqOffset;
    ifdParam.eAcquisitionMode = pParams->eAcquisitionMode;
    ifdParam.ifdAgcControl = pParams->ifdAgcControl;
    return BVSB_3548_P_IfdTop(h, &ifdParam, p3548->eIfdMode, pParams->eAnalogAudioDeviation );

#else
    BERR_Code retCode;
    uint8_t   hab[9];
	uint8_t   buf;

    hab[0] = 0x0C;
    hab[1] = hab[2] = hab[8] = 0x00;
    hab[3] = 0x01;

    hab[1] |= ((h->settings.ntsc.bFastAcq) ? 0x01 : 0x00);
    hab[1] |= ((h->settings.ntsc.bAutoAcq) ? 0x20 : 0x00);

    switch (h->settings.ntsc.bw)
    {
        case BVSB_PhaseLoopBw_eLow:
            /*hab[2] |= 0x00;*/
            break;

        case BVSB_PhaseLoopBw_eMedium:
            hab[2] |= 0x02;
            break;

        case BVSB_PhaseLoopBw_eHigh:
            hab[2] |= 0x04;
            break;

        default:
            return (BERR_TRACE(BERR_INVALID_PARAMETER));
    }
    switch (h->settings.ntsc.ifd_pullin_range)
    {
    case BVSB_PullInRange_eWide:
	    hab[3] |= 0x00;
	    break;
    case BVSB_PullInRange_e282KHz:
	    hab[3] |= 0x90;
	    break;
    case BVSB_PullInRange_e32kHz:
	    hab[3] |= 0x80;
	    break;
    default:
	    h->settings.ntsc.ifd_pullin_range = BVSB_PullInRange_eWide;
	    hab[3] |= 0x00;
    }

	switch (h->settings.ntsc.analog_mode)
    {
    case BVSB_AnalogMode_eNTSC:
        if (h->settings.ntsc.btsc_country == BVSB_BtscCountry_eUS_High_Deviation)
        {
           buf = 0x01;
           BVSB_3548_P_WriteConfig(h, BVSB_CONFIG_IFD_CONFIG2, &buf, 1);
        }
        if (h->settings.ntsc.btsc_country == BVSB_BtscCountry_eUS)
        {
    	      hab[2] |= VIDEO_MODE_NTSC;
        }
        else if (h->settings.ntsc.btsc_country == BVSB_BtscCountry_eKorea)
        {
            hab[2] |= VIDEO_MODE_NTSC_A2;
        }
        break;
    case BVSB_AnalogMode_ePalD:
        hab[2] |= VIDEO_MODE_PALD;
    	  break;
    case BVSB_AnalogMode_ePalI:
    	  hab[2] |= VIDEO_MODE_PALI;
    	  break;
    case BVSB_AnalogMode_ePalBGH:
    	  hab[2] |= VIDEO_MODE_PAL_BGH;
    	  break;
    default:
    	  return (BERR_TRACE(BERR_INVALID_PARAMETER));
    }
	
    if (pParams->mode == BVSB_InbandMode_eAnalog_SIF)
    {		
	    buf = AUDIO_SIF_MODE;
       BVSB_3548_P_WriteConfig(h, BVSB_CONFIG_IFD_CONFIG1, &buf, 1);
    }

    if ((pParams->ifFreqOffset < -2097152) || (pParams->ifFreqOffset > 2097151))
        return BERR_INVALID_PARAMETER;

    hab[4] = (pParams->ifFreqOffset >> 24);
    hab[5] = (pParams->ifFreqOffset >> 16);
    hab[6] = (pParams->ifFreqOffset >> 8);
    hab[7] = (pParams->ifFreqOffset & 0xFF);

    BDBG_MSG(("Acquire NTSC Cmd: %x %x %x\n", hab[1], hab[2], hab[3]));
    BVSB_CHK_RETCODE(BVSB_3548_P_SendHabCommand(h, hab, 9, hab, 1, true));
#endif

done:
    return retCode;
}


/******************************************************************************
  BVSB_3548_P_ReadMemory()
 ******************************************************************************/
BERR_Code BVSB_3548_P_ReadMemory(BVSB_Handle h, uint16_t addr, uint8_t *buf, uint16_t n)
{
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    uint16_t i;

    if (((uint32_t)addr + (uint32_t)n) > 0x10000)
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    for (i = 0; i < n; i++)
    {
        buf[i] = (uint8_t)(BREG_Read32(p3548->hRegister, BVSB_AP_INT_RAM+((i+addr)*4)));
    }

    return BERR_SUCCESS;
}


/******************************************************************************
  BVSB_3548_P_WriteMemory()
 ******************************************************************************/
BERR_Code BVSB_3548_P_WriteMemory(BVSB_Handle h, uint16_t addr, uint8_t *buf, uint16_t n)
{
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    uint16_t i;

    if ((addr >= 0x8000) || (n >= 0x8000))
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    if ((addr + n) > 0x8000)
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    for (i = 0; i < n; i++)
    {
        BREG_Write32(p3548->hRegister, BVSB_AP_INT_RAM+((i+addr)*4), (uint32_t)(buf[i]));
    }

    return BERR_SUCCESS;
}

/******************************************************************************
  BVSB_3548_P_DetectChannelSignal()
 ******************************************************************************/
BERR_Code BVSB_3548_P_DetectChannelSignal(
        BVSB_Handle h,                  /* [in] BVSB handle */
        BVSB_DetectType eDetectType,    /* [in] signal detect type */
        BVSB_SignalMode* pCheckStatus   /* [out] inband acquisition parameters */
        )
{
	unsigned char write_buf[3];
	unsigned char read_buffer[16];
	uint32_t regVal;
	int iCounter = 0;
    BERR_Code retCode;
	BVSB_3548_P_Handle *p3548; 

	if (h ==  NULL || pCheckStatus == NULL)
	{
      retCode = BERR_INVALID_PARAMETER;
      return retCode;
	}

    /* Power everything on to prepare for the scan */
    BVSB_3548_P_PowerEnable(h, BVSB_POWER_AP);

    p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    /* Set gain to match VSB parameters */
    BVSB_3548_P_SetPgaGain(h, BVSB_InbandMode_e8VSB, h->settings.vsb.pgaGain);
    
	write_buf[0] = 0x1A;
   switch (eDetectType)
	{
	case BVSB_DetectType_ePresent:
		write_buf[1] = 0x00;
		break;
		
	case BVSB_DetectType_eVSB:
		write_buf[1] = 0x02;
		break;
		
	case BVSB_DetectType_eNTSC:
		write_buf[1] = 0x04;
		break;
		
	case BVSB_DetectType_eNTSC_VSB:
		write_buf[1] = 0x06;
		break;
		
	case BVSB_DetectType_eQAM:
		write_buf[1] = 0x08;
		break;
		
	case BVSB_DetectType_eNTSC_QAM:
		write_buf[1] = 0x0C;
		break;
		
	case BVSB_DetectType_eNTSC_VSB_QAM:
		write_buf[1] = 0x0E;
		break;

	default:
		write_buf[1] = 0x0E;
		break;
	}

	write_buf[2] = 0x0;
	BVSB_CHK_RETCODE(BVSB_3548_P_SendHabCommand(h, write_buf, 3, read_buffer, 1, true));
	regVal = BREG_Read32(p3548->hRegister, BCHP_AP_32_H_STAT3);
	regVal = regVal & 0x80;
	BKNI_Sleep(100);
	while (regVal == 0 && iCounter < 30)
	{
		BKNI_Sleep(10);
		regVal = BREG_Read32(p3548->hRegister, BCHP_AP_32_H_STAT3);
		BDBG_MSG(("reg BCHP_AP_32_H_STAT3 value:0x%x\n", regVal));
		regVal = regVal & 0x80;
		iCounter ++;
	}	

	if (iCounter >= 29)
	{
		*pCheckStatus = BVSB_InbandMode_eUnknown;
		return BERR_INVALID_PARAMETER;
	}
	else
	{
		regVal = BREG_Read32(p3548->hRegister,BCHP_AP_32_H_MSG2);
		
		regVal = regVal & 0xff;
		BDBG_MSG(("reg BCHP_AP_32_H_MSG2 value:0x%x\n", regVal));
		switch (regVal) {
		case 0x01:
			*pCheckStatus = BVSB_SignalMode_eVSB;
			break;
		case 0x02:
			*pCheckStatus = BVSB_SignalMode_eNTSC;
			break;	
      case 0x03:
			*pCheckStatus = BVSB_SignalMode_eQAM_64;
			break;
		case 0x04:
			*pCheckStatus = BVSB_SignalMode_eQAM_256;
			break;
		case 0x10:
		case 0x11:
		case 0x12:
			*pCheckStatus = BVSB_SignalMode_eNoSignal;
			break;
		default:
			*pCheckStatus = BVSB_SignalMode_eUnknown;
		}		
	}	

done:
	return retCode;
}


BERR_Code BVSB_3548_P_SetInbandIfFreq(
        BVSB_Handle           h,         /* [in] BVSB handle */
        uint32_t if_freq_hz /* [in] inband transport output interface settings */
        )
{

    BERR_Code retCode;
    uint8_t buf[9];
    buf[0] = 0x13;
    buf[1] = (BVSB_CONFIG_INBAND_IF_FREQ & 0xff00) >> 8;

    buf[2] = BVSB_CONFIG_INBAND_IF_FREQ & 0xff;
    buf[3] = 0x04;
    buf[4] = (if_freq_hz & 0xFF000000) >> 24;
    buf[5] = (if_freq_hz & 0x00FF0000) >> 16;
    buf[6] = (if_freq_hz & 0x0000FF00) >> 8;
    buf[7] = if_freq_hz & 0x000000FF;
    buf[8] = 0x00;

    BVSB_CHK_RETCODE(BVSB_3548_P_SendHabCommand(h, buf, 9, buf, 1, true));

done:
    return retCode;
}

BERR_Code BVSB_3548_P_SetQamSymbol(
        BVSB_Handle           h,         /* [in] BVSB handle */
        uint32_t uSymRate_hz /* [in] QAM symbol rate */
        )
{
	BERR_Code retCode;
	uint8_t buf[9];
	buf[0] = 0x13;
	buf[1] = (BVSB_CONFIG_QAM_A_SYM_RATE & 0xff00) >> 8;
	
	buf[2] = BVSB_CONFIG_QAM_A_SYM_RATE & 0xff;
	buf[3] = 0x04;
	buf[4] = (uSymRate_hz & 0xFF000000) >> 24;
	buf[5] = (uSymRate_hz & 0x00FF0000) >> 16;
	buf[6] = (uSymRate_hz & 0x0000FF00) >> 8;
	buf[7] = uSymRate_hz & 0x000000FF;
	buf[8] = 0x00;
	
	BVSB_CHK_RETCODE(BVSB_3548_P_SendHabCommand(h, buf, 9, buf, 1, true));
	
done:
	return retCode;
}
BERR_Code BVSB_3548_P_SetIfdPullInRange(
    BVSB_Handle h,        
    BVSB_Ifd_PullInRange if_pulling_range)
{
	BERR_Code retCode = BERR_SUCCESS;
	if (h == NULL)
	{
		return BERR_INVALID_PARAMETER;
	}
	h->settings.ntsc.ifd_pullin_range = if_pulling_range;
	return retCode;
}
BERR_Code BVSB_3548_P_SetThreshold(BVSB_Handle h)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[4];
    uint32_t qamParam4;
    uint16_t agf;
    uint16_t agc;

    if (h == NULL)
    {
        return BERR_INVALID_PARAMETER;
    }

    qamParam4 = h->settings.detect.qamParam4;
    agf = h->settings.detect.agf;
    agc = h->settings.detect.agc;
    if (qamParam4 == BVSB_P_UNCHANGED_THRESHOLD && agf == BVSB_P_UNCHANGED_THRESHOLD && agc == BVSB_P_UNCHANGED_THRESHOLD)
    {
		BDBG_MSG(("Using default thresholds!\n"));
        return retCode;
    }

    buf[0] = ((qamParam4 >> 24) & 0xff);
    buf[1] = ((qamParam4 >> 16) & 0xff);
    buf[2] = ((qamParam4 >> 8) & 0xff);
    buf[3] = (qamParam4 & 0xff);
    retCode = BVSB_WriteConfig(h, BVSB_CONFIG_SCAN_QAM_PARAM4, buf, 4);
    if (retCode!= BERR_SUCCESS)
    {	
        return retCode;
    } 

    buf[0] = ((agf >> 8) & 0xff);
    buf[1] = (agf & 0xff);
    retCode = BVSB_WriteConfig(h, BVSB_CONFIG_SCAN_AGF, buf, 2);
    if (retCode!= BERR_SUCCESS)
    {
        return retCode;
    }

    buf[0] = ((agc >> 8) & 0xff);
    buf[1] = (agc & 0xff);
    retCode = BVSB_WriteConfig(h, BVSB_CONFIG_SCAN_AGC, buf, 2);
    if (retCode!= BERR_SUCCESS)
    {
        return retCode;
    }

    return retCode;
}

BERR_Code BVSB_3548_P_SetQamSettings(BVSB_Handle h, bool fastAcq)
{
    BERR_Code retCode = BERR_SUCCESS;

    if (h == NULL)
    {
        return BERR_INVALID_PARAMETER;
    }

    h->settings.qam.bFastAcq = fastAcq;

    return retCode;
}

BERR_Code BVSB_3548_P_SetIfdPixCarrier(
        BVSB_Handle           h,         /* [in] BVSB handle */
        uint32_t uIfdPixCarrier_hz /* [in] IFD picture carrier */
        )
{
    BERR_Code retCode = BERR_SUCCESS;
	 uint8_t buf[4];
    if (h == NULL)
    {
        return BERR_INVALID_PARAMETER;
    }
    buf[0] = ((uIfdPixCarrier_hz >> 24) & 0xff);
    buf[1] = ((uIfdPixCarrier_hz >> 16) & 0xff);
    buf[2] = ((uIfdPixCarrier_hz >> 8) & 0xff);
    buf[3] = (uIfdPixCarrier_hz & 0xff);
    retCode = BVSB_3548_P_WriteConfig(h, BVSB_CONFIG_PIX_CARRIER, buf, 4);

    return retCode;
}

BERR_Code BVSB_3548_P_PowerEnable(
        BVSB_Handle h, 
        uint32_t uiFlag
        )
{

#ifndef BVSB_POWER_MANAGEMENT
    BSTD_UNUSED(h);
    BSTD_UNUSED(uiFlag);
#else
    uint32_t regVal;
    BERR_Code retCode;
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    BDBG_MSG(("IFD/DS powered on %d", p3548->bPowerdown));

    /* We always have to enable each core because we don't know if we are in middle of switching mode */
    BREG_AtomicUpdate32(p3548->hRegister, BCHP_CLKGEN_PWRDN_CTRL_0,  
            BCHP_CLKGEN_PWRDN_CTRL_0_PWRDN_CLOCK_216_CG_DS_MASK, 0 );

    BREG_AtomicUpdate32(p3548->hRegister, BCHP_CLKGEN_PWRDN_CTRL_1,  
            BCHP_CLKGEN_PWRDN_CTRL_1_PWRDN_CLOCK_108_CG_DS_MASK, 0 );

    BREG_AtomicUpdate32(p3548->hRegister, BCHP_CLKGEN_PWRDN_CTRL_0,  
            BCHP_CLKGEN_PWRDN_CTRL_0_PWRDN_CLOCK_216_CG_IFD_MASK, 0 );

    BREG_AtomicUpdate32(p3548->hRegister, BCHP_CLKGEN_PWRDN_CTRL_1,  
            BCHP_CLKGEN_PWRDN_CTRL_1_PWRDN_CLOCK_108_CG_IFD_MASK, 0 );

    BREG_AtomicUpdate32(p3548->hRegister, BCHP_CLKGEN_PWRDN_CTRL_3,  
            BCHP_CLKGEN_PWRDN_CTRL_3_PWRDN_CLOCK_54_CG_IFD_MASK, 0 );

    
    /* If we are recovering from a power down than we need to bring AFE/PLL back up */
    if (true == p3548->bPowerdown)
    {
        /* Always power AFE and PLL */
        regVal = BREG_Read32(p3548->hRegister, BCHP_DS_AFE_CTRL1);
        regVal &= ~BCHP_DS_AFE_CTRL1_PWRDN_MASK;
        BREG_Write32(p3548->hRegister, BCHP_DS_AFE_CTRL1, regVal);

        /* Power on PLL */
        regVal = BREG_Read32(p3548->hRegister, BCHP_DS_PLL_MISC1);
        regVal &= ~BCHP_DS_PLL_MISC1_PWRDN_MASK;
        regVal &= ~BCHP_DS_PLL_MISC1_REFCOMP_PWRDN_MASK;
        regVal &= ~BCHP_DS_PLL_MISC1_NDIV_PWRDN_MASK;
        BREG_Write32(p3548->hRegister, BCHP_DS_PLL_MISC1, regVal);

        regVal = BREG_Read32(p3548->hRegister, BCHP_DS_PLL_CH1);
        regVal &= ~BCHP_DS_PLL_CH1_PWRDN_CH1_MASK;
        BREG_Write32(p3548->hRegister, BCHP_DS_PLL_CH1, regVal);

        regVal = BREG_Read32(p3548->hRegister, BCHP_DS_PLL_CH2);
        regVal &= ~BCHP_DS_PLL_CH2_PWRDN_CH2_MASK;
        BREG_Write32(p3548->hRegister, BCHP_DS_PLL_CH2, regVal);

        regVal = BREG_Read32(p3548->hRegister, BCHP_DS_PLL_CH3);
        regVal &= ~BCHP_DS_PLL_CH3_PWRDN_CH3_MASK;
        BREG_Write32(p3548->hRegister, BCHP_DS_PLL_CH3, regVal);
    }

    if ((uiFlag & BVSB_POWER_AP) && (true == p3548->bPowerdown))
    {
        if ((retCode = BVSB_3548_P_RunAp(h)) != BERR_SUCCESS)
            return retCode;

        /* wait for init done interrupt */
        do
        {
            retCode = BKNI_WaitForEvent(p3548->hInitDoneEvent, 500);
        } while ((retCode != BERR_TIMEOUT) && (retCode != BERR_SUCCESS));
    }
    p3548->bPowerdown = false;

#endif
    return BERR_SUCCESS;

}

BERR_Code BVSB_3548_P_PowerDown(
        BVSB_Handle h
        )
{

#ifndef BVSB_POWER_MANAGEMENT
    BSTD_UNUSED(h);
    return BERR_SUCCESS;
#else
    BERR_Code retCode;
    uint32_t regVal;

    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    BDBG_MSG(("IFD/DS powered off"));

    /* Make sure IFD timer is off */
    BTMR_StopTimer(p3548->hTimer); 

    /* Turn on power for a little bit, so we can have access to register */
    BREG_AtomicUpdate32(p3548->hRegister, BCHP_CLKGEN_PWRDN_CTRL_0,  
            BCHP_CLKGEN_PWRDN_CTRL_0_PWRDN_CLOCK_216_CG_DS_MASK, 0 );

    BREG_AtomicUpdate32(p3548->hRegister, BCHP_CLKGEN_PWRDN_CTRL_1,  
            BCHP_CLKGEN_PWRDN_CTRL_1_PWRDN_CLOCK_108_CG_DS_MASK, 0 );

    /* Idle AP but not power off for quick recovery */
    BVSB_CHK_RETCODE(BVSB_3548_P_ResetAp(h));

    /* Power off AFE */
    regVal = BREG_Read32(p3548->hRegister, BCHP_DS_AFE_CTRL1);
    regVal |= BCHP_DS_AFE_CTRL1_PWRDN_MASK;
    BREG_Write32(p3548->hRegister, BCHP_DS_AFE_CTRL1, regVal);

    /* Power off PLL */
    regVal = BREG_Read32(p3548->hRegister, BCHP_DS_PLL_MISC1);
    regVal |= BCHP_DS_PLL_MISC1_PWRDN_MASK;
    regVal |= BCHP_DS_PLL_MISC1_REFCOMP_PWRDN_MASK;
    regVal |= BCHP_DS_PLL_MISC1_NDIV_PWRDN_MASK;
    BREG_Write32(p3548->hRegister, BCHP_DS_PLL_MISC1, regVal);

    regVal = BREG_Read32(p3548->hRegister, BCHP_DS_PLL_CH1);
    regVal |= BCHP_DS_PLL_CH1_PWRDN_CH1_MASK;
    BREG_Write32(p3548->hRegister, BCHP_DS_PLL_CH1, regVal);

    regVal = BREG_Read32(p3548->hRegister, BCHP_DS_PLL_CH2);
    regVal |= BCHP_DS_PLL_CH2_PWRDN_CH2_MASK;
    BREG_Write32(p3548->hRegister, BCHP_DS_PLL_CH2, regVal);

    regVal = BREG_Read32(p3548->hRegister, BCHP_DS_PLL_CH3);
    regVal |= BCHP_DS_PLL_CH3_PWRDN_CH3_MASK;
    BREG_Write32(p3548->hRegister, BCHP_DS_PLL_CH3, regVal);

    /* Power down DS */
    BREG_AtomicUpdate32(p3548->hRegister, BCHP_CLKGEN_PWRDN_CTRL_0,  0,
            BCHP_FIELD_DATA( CLKGEN_PWRDN_CTRL_0, PWRDN_CLOCK_216_CG_DS, 1 ));

    BREG_AtomicUpdate32(p3548->hRegister, BCHP_CLKGEN_PWRDN_CTRL_1, 0,
            BCHP_FIELD_DATA( CLKGEN_PWRDN_CTRL_1, PWRDN_CLOCK_108_CG_DS, 1 ));

    /* Power down IFD */
    BREG_AtomicUpdate32(p3548->hRegister, BCHP_CLKGEN_PWRDN_CTRL_0, 0,
            BCHP_FIELD_DATA( CLKGEN_PWRDN_CTRL_0, PWRDN_CLOCK_216_CG_IFD, 1 ));

    BREG_AtomicUpdate32(p3548->hRegister, BCHP_CLKGEN_PWRDN_CTRL_1, 0,
            BCHP_FIELD_DATA( CLKGEN_PWRDN_CTRL_1, PWRDN_CLOCK_108_CG_IFD, 1 ));

    BREG_AtomicUpdate32(p3548->hRegister, BCHP_CLKGEN_PWRDN_CTRL_3, 0,
            BCHP_FIELD_DATA( CLKGEN_PWRDN_CTRL_3, PWRDN_CLOCK_54_CG_IFD, 1 ));
    p3548->bPowerdown = true;

done:
    return retCode;
#endif
}

BERR_Code BVSB_3548_P_SetPgaGain(
	BVSB_Handle h,      
    BVSB_InbandMode mode,
	BVSB_PgaGain pgaGain
	)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf;
    uint8_t gain_value;
    if (h == NULL)
    {
        return BERR_INVALID_PARAMETER;
    }

    if ( pgaGain > 6 )
    {
        BDBG_ERR(("This core cannot support PGA gain greater then 6 dB!!!"));
        return BERR_INVALID_PARAMETER;
    }
    gain_value = pgaGain;

    switch (mode)
    {
        case BVSB_InbandMode_e8VSB:      /* 8-VSB */
        case BVSB_InbandMode_e16VSB:     /* 16-VSB */
            /*vsb pga gain*/
            BVSB_3548_P_ReadConfig(h, BVSB_CONFIG_VSB_TER_SETUP1, &buf, 1);
            buf &= 0xF8;
            buf |= gain_value;
            BVSB_3548_P_WriteConfig(h, BVSB_CONFIG_VSB_TER_SETUP1, &buf, 1);
            BVSB_3548_P_ReadConfig(h, BVSB_CONFIG_VSB_CABLE_SETUP1, &buf, 1);
            buf &= 0xF8;
            buf |= gain_value;
            BVSB_3548_P_WriteConfig(h, BVSB_CONFIG_VSB_CABLE_SETUP1, &buf, 1);
            break;

	  	case BVSB_InbandMode_e256QAM_A:  /* 256-QAM Annex A */
        case BVSB_InbandMode_e128QAM_A:  /* 128-QAM Annex A */
        case BVSB_InbandMode_e64QAM_A:    /* 64-QAM Annex A  */
        case BVSB_InbandMode_e32QAM_A:    /* 32-QAM Annex A  */
        case BVSB_InbandMode_e16QAM_A:    /* 16-QAM Annex A  */
        case BVSB_InbandMode_e1024QAM_B: /* 1024-QAM Annex B */
        case BVSB_InbandMode_e256QAM_B:  /* 256-QAM Annex B */
        case BVSB_InbandMode_e64QAM_B:    /* 64-QAM Annex B  */
        case BVSB_InbandMode_e64_256QAM_B: /* QAM Scan mode */
            /*qam pga gain*/
            BVSB_3548_P_ReadConfig(h, BVSB_CONFIG_QAM_TER_SETUP1, &buf, 1);
            buf &= 0xF8;
            buf |= gain_value;
            BVSB_3548_P_WriteConfig(h, BVSB_CONFIG_QAM_TER_SETUP1, &buf, 1);
            BVSB_3548_P_ReadConfig(h, BVSB_CONFIG_QAM_CABLE_SETUP1, &buf, 1);
            buf &= 0xF8;
            buf |= gain_value;
            BVSB_3548_P_WriteConfig(h, BVSB_CONFIG_QAM_CABLE_SETUP1, &buf, 1);
            break;

        case BVSB_InbandMode_eAnalog:      /* NTSC */
		case BVSB_InbandMode_eAnalog_SIF:
            /*ntsc pga gain*/
            BVSB_3548_P_ReadConfig(h, BVSB_CONFIG_NTSC_TER_SETUP1, &buf, 1);
            buf &= 0xF8;
            buf |= gain_value;
            BVSB_3548_P_WriteConfig(h, BVSB_CONFIG_NTSC_TER_SETUP1, &buf, 1);

            BVSB_3548_P_ReadConfig(h, BVSB_CONFIG_NTSC_CABLE_SETUP1, &buf, 1);
            buf &= 0xF8;
            buf |= gain_value;
            BVSB_3548_P_WriteConfig(h, BVSB_CONFIG_NTSC_CABLE_SETUP1, &buf, 1);
            break;

        default:
            return (BERR_TRACE(BERR_INVALID_PARAMETER)); 
    }
    return retCode;
}



