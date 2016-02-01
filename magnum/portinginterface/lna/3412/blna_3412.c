/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 * This module is the driver for BCM3412 LNA.
 * Revision History:
 *
 * $brcm_Log: $
 * 
  ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "blna.h"
#include "blna_priv.h"
#include "blna_3412.h"
#include "bchp_bcm3412_regs.h"

BDBG_MODULE(blna_3412);

#define	CHK_RETCODE( rc, func )				\
do {										\
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										\
		goto done;							\
	}										\
} while(0)


#define	DEV_MAGIC_ID			((BERR_LNA_ID<<16) | 0xFACE)

#define BLNA_3412_ReadRegisterField(h, Register, Field) \
  ((((BLNA_3412_ReadRegister(h, BCHP_BCM341X_regs_##Register)) & BCHP_MASK(BCM341X_regs_##Register,Field)) >> \
  BCHP_SHIFT(BCM341X_regs_##Register,Field)))

#define BLNA_3412_WriteRegisterField(h, Register, Field, Data) \
  BLNA_3412_WriteRegister(h, BCHP_BCM341X_regs_##Register, ((BLNA_3412_ReadRegister(h, BCHP_BCM341X_regs_##Register) & \
  ~((uint32_t)BCHP_MASK(BCM341X_regs_##Register, Field))) | \
  BCHP_FIELD_DATA(BCM341X_regs_##Register, Field, Data)))

#define	BLNA_MN_AGC_OUTPUT_LEVEL			(0)
#define	BLNA_MX_AGC_OUTPUT_LEVEL			(63)
#define	BLNA_MN_MGC_OUTPUT_LEVEL			(1)
#define	BLNA_MX_MGC_OUTPUT_LEVEL			(70)

/*******************************************************************************
*
*	Design Notes:
*
*		TODO
*
*******************************************************************************/



/*******************************************************************************
*
*	Private Module Handles
*
*******************************************************************************/
typedef enum BLNA_3412_InitMode
{
	BLNA_3412_InitMode_eRaw,
	BLNA_3412_InitMode_eHalfCooked,
	BLNA_3412_InitMode_eCooked
} BLNA_3412_InitMode;
	
typedef struct BLNA_3412_P_Settings
{
	BLNA_3412_InitMode initMode;
	BLNA_GainMode gainMode;				/* See BLNA_GainMode for description */
	BREG_I2C_Handle hI2CReg;
	unsigned short i2cAddr;
	BLNA_InBandMode inBandPorts[BLNA_MX_INBAND_PORTS];		/* InBand port mode */
	BLNA_OutOfBandMode outOfBandPorts[BLNA_MX_OUTOFBAND_PORTS];	/* OutOfBand port mode */
	bool isExternalDriverEnabled[BLNA_MX_EXTDRV]; /* Ext. Driver Enabled=1, otherwise disabled */
	unsigned int agcOutputLevel;		/* AGC Output Level, in 2/10 dB */
	int agcDeltaVal;					/* AGC Delta Value, in 2/10 dB */
	unsigned int manualGainVal;			/* If Manual Gain is enabled, manual gain value, in 2/10 dB */
} BLNA_3412_P_Settings;


typedef struct BLNA_P_3412_Handle		*BLNA_3412_Handle;

typedef struct BLNA_P_3412_Handle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */

	BLNA_3412_P_Settings settings;
	uint32_t chipId;                    /* LNA chip ID. Can support 3410, other family members */
    bool hasSuperboost;                 /* super-sensitivity noise figure */
    bool hasFixedGainLoopThru;          /* fixed gain loop-through option */
    unsigned nInBandPorts;              /* number of in-band outputs */
	unsigned nOutOfBandPorts;           /* number of out-of-band outputs */
} BLNA_P_3412_Handle;

/*******************************************************************************
*
*	Default Module Settings
*
*******************************************************************************/
static const BLNA_3412_Settings defDevSettings =
{
	BLNA_3412_SETTINGS_I2CADDR,

	BLNA_3412_SETTINGS_GAINMODE,
	{ BLNA_3412_SETTINGS_INBANDPORTS, BLNA_3412_SETTINGS_INBANDPORTS, BLNA_3412_SETTINGS_INBANDPORTS },
	{ BLNA_3412_SETTINGS_OUTOFBANDPORTS, BLNA_3412_SETTINGS_OUTOFBANDPORTS },
	BLNA_3412_SETTINGS_AGCOUTPUTLEVEL,
	BLNA_3412_SETTINGS_AGCDELTALEVEL,
	BLNA_3412_SETTINGS_MANUALGAINVAL,
    false
};


/*******************************************************************************
*
*	Private Module Data
*
*******************************************************************************/


/*******************************************************************************
*
*	Private Module Functions
*
*******************************************************************************/
static BERR_Code BLNA_3412_Init( BLNA_3412_Handle hDev );
static BERR_Code BLNA_3412_EnablePowerSaver(BLNA_3412_Handle hDev);
static BERR_Code BLNA_3412_DisablePowerSaver(BLNA_3412_Handle hDev);
static BERR_Code BLNA_3412_SetInBandMode(BLNA_3412_Handle hDev,unsigned int portNbr,BLNA_InBandMode mode);
static BERR_Code BLNA_3412_EnableLoopThrough(BLNA_3412_Handle hDev);
static BERR_Code BLNA_3412_DisableLoopThrough(BLNA_3412_Handle hDev);
static BERR_Code BLNA_3412_SetBoostMode(BLNA_3412_Handle hDev, BLNA_BoostMode boostMode);
static BERR_Code BLNA_3412_GetStatus(BLNA_3412_Handle hDev,BLNA_Status *pStatus);
static void BLNA_3412_WriteRegister(BLNA_3412_Handle hDev, uint8_t registerAddress, uint32_t value);
static uint32_t BLNA_3412_ReadRegister(BLNA_3412_Handle hDev, uint8_t registerAddress );


static void BLNA_3412_WriteRegister(BLNA_3412_Handle hDev, uint8_t registerAddress, uint32_t value)
{	
	uint8_t buf[8];
			
	buf[0]  = (char)(value & 0xFF);
	buf[1]  = (char)((value >> 8) & 0xFF);
	buf[2]  = (char)((value >> 16) & 0xFF);
	buf[3]  = (char)((value >> 24) & 0xFF);

	BREG_I2C_Write(hDev->settings.hI2CReg,hDev->settings.i2cAddr, (registerAddress >> 2), &buf[0], 4);
}

static uint32_t BLNA_3412_ReadRegister(BLNA_3412_Handle hDev, uint8_t registerAddress )
{

	uint32_t reg = 0;
	uint8_t	 buf[5];

	BREG_I2C_Read(hDev->settings.hI2CReg,hDev->settings.i2cAddr, (registerAddress >> 2), &buf[0], 4);

	reg = 	 buf[0];
	reg |= 	 buf[1] << 8;
	reg |=	 buf[2] << 16;
	reg |=	 buf[3] << 24;

	return reg;
}

static BERR_Code BLNA_3412_Close(
	BLNA_Handle hDev						/* [in] Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ENTER(BLNA_3412_Close);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	hDev->magicId = 0x00;		/* clear it to catch inproper use */
	BKNI_Free( (void *) hDev->hDevImpl );
	BKNI_Free( (void *) hDev );

	BDBG_LEAVE(BLNA_3412_Close);
	return( retCode );
}

static BERR_Code BLNA_3412_GetLnaAgcRegVal(
	BLNA_3412_Handle hDev,					/* [in] Device handle */
	uint32_t regOffset,					/* [in] AGC register offset */
	uint32_t *agcVal					/* [out] output value */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	BDBG_ENTER(BLNA_3412_GetLnaAgcRegVal);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BSTD_UNUSED(regOffset);

    if( agcVal == NULL)
	{
		retCode = BERR_INVALID_PARAMETER;
		goto done;
	}

	*agcVal = BLNA_3412_ReadRegisterField(hDev, STATUS_READ2, STG1_GAIN);
	
done:
	BDBG_LEAVE(BLNA_3412_GetLnaAgcRegVal);
	return( retCode );
}


static BERR_Code BLNA_3412_SetSecondStagePower(
		BLNA_3412_Handle hDev,		/* [in] Device handle */
		uint8_t second_stage,		/* [in] first/second 2nd stage, 1 for first, 2 for second */
		uint32_t power_switch		/* [in] on/off, 1 for on, 0 fro off */
	)
{
	BERR_Code retCode = BERR_SUCCESS;	
	BDBG_ENTER(BLNA_3412_SetSecondStagePower);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	if( second_stage != 1 && second_stage != 2 )
	{
		retCode = BERR_INVALID_PARAMETER;
		goto done;
	}

	if( power_switch != 1 && power_switch != 2 )
	{
		retCode = BERR_INVALID_PARAMETER;
		goto done;
	}

	if ( second_stage == 2 ) {	BLNA_3412_WriteRegisterField(hDev, BG_LNA2_RCAL_CNTL, STAGE2_PD2, power_switch); }
	else	{ BLNA_3412_WriteRegisterField(hDev, BG_LNA2_RCAL_CNTL, STAGE2_PD1, power_switch); }	
	
done:
	BDBG_LEAVE(BLNA_3412_SetSecondStagePower);
	return( retCode );	
}

/****************************************************************************************************
 * BLNA_3412_WaitForLock_LNA()  This routine waits for the BCM3410 AGC state Machine to obtain lock
 * From DOCSIS code: WaitForLock()
 ****************************************************************************************************/
static void BLNA_3412_WaitForLock_LNA( BLNA_3412_Handle hDev )
{
    uint32_t Status, AGC_Lock, Gain, i;

    /* Temporarily speed up the response of the AGC in the 3410 */
    BLNA_3412_WriteRegisterField(hDev, RAMP_RATE_CNTL,SLOW_UP_RAMP_RATE,0x40);
    BLNA_3412_WriteRegisterField(hDev, RAMP_RATE_CNTL,SLOW_DWN_RAMP_RATE,0x40);

    /* max ramp */
    BLNA_3412_WriteRegisterField(hDev, SM_BYPASS,ATE,1);

    /*Freeze*/
    BLNA_3412_WriteRegisterField(hDev, LNA_ADDR,STG1_GAIN_I2C,0x0F);
    BLNA_3412_WriteRegisterField(hDev, SM_BYPASS,DEC_LAT_ENB_STG1_I2C,1);
    BLNA_3412_WriteRegisterField(hDev, SM_BYPASS,DEC_LAT_ENB_STG1_I2C,0);

    /*Release SM and PD*/
    BLNA_3412_WriteRegisterField(hDev, MWC_CNTL,SM_RESET,0);
    BLNA_3412_WriteRegisterField(hDev, PD_OOB_OSC_CNTL,PD_RESET,0);

	BKNI_Sleep(100);

    /*Normal Ramp*/
    BLNA_3412_WriteRegisterField(hDev, SM_BYPASS,ATE,0);

    i = 0;
    while( i < 100 ) /* max 5 sec loop */
    {
        i++;
        BKNI_Sleep(50);
        Status = BLNA_3412_ReadRegister(hDev,BCHP_BCM341X_regs_STATUS_READ2);
        /* BDBG_MSG((" inLoop%ld: LNA Status = 0x%08lx", i, Status)); */
        AGC_Lock = Status & BCHP_BCM341X_regs_STATUS_READ2_AGC_LOCK_MASK;
        Gain = Status & 0x0000001F;
        if( (AGC_Lock==0x80) || (Gain==0x1F) )
            break;
    }

    /*Restore ramp rate to defaults*/
    BLNA_3412_WriteRegisterField(hDev, RAMP_RATE_CNTL,SLOW_UP_RAMP_RATE,0x04);
    BLNA_3412_WriteRegisterField(hDev, RAMP_RATE_CNTL,SLOW_DWN_RAMP_RATE,0x10);
}

/****************************************************************************************************
 * BLNA_3412_Optimize_LNA()  This routine optimizes tilt/boost settings of the external LNA
 * From DOCSIS code: BCM341X_regs_ReConfig(), formerly BCM341X_regs_TiltBoost() 
 ****************************************************************************************************/
void BLNA_3412_Optimize_LNA( BLNA_3412_Handle hDev )
{
    unsigned int currentGain, tiltGain;
    unsigned int tolBoost, tolTilt;
	int BoostOn=1;

    BDBG_MSG(("Initial Boost Status=%d. ", BLNA_3412_ReadRegisterField(hDev, LNA1_CNTL,STAGE1_GAIN_BOOST) ));
    BDBG_MSG(("Tilt Status=%d", BLNA_3412_ReadRegisterField(hDev, LNA1_CNTL,STAGE1_TILT_ON) ));

    /* reset SM and PD */
    BLNA_3412_WriteRegisterField(hDev, MWC_CNTL,SM_RESET,1);
    BLNA_3412_WriteRegisterField(hDev, PD_OOB_OSC_CNTL,PD_RESET,1);

    BLNA_3412_WriteRegisterField(hDev, LNA1_CNTL,STAGE1_GAIN_BOOST,1);
    BLNA_3412_WriteRegisterField(hDev, LNA1_CNTL,STAGE1_TILT_ON,1);

    tolBoost = 3;  /* Adjustable tolerance on the decision */
    tolTilt  = 0;

    if(hDev->hasSuperboost)
    {
        tolTilt  = 1;   /* script uses different value */
        BLNA_3412_WriteRegisterField(hDev, LNA_ADDR, SuperBoost_en, 1);
        BDBG_MSG(("(SuperBoost==1)"));
    }

    BLNA_3412_WaitForLock_LNA(hDev);
	/* valid value is 1-31 although it has 6 bits definition */
    currentGain = BLNA_3412_ReadRegister(hDev,BCHP_BCM341X_regs_STATUS_READ2);
    currentGain &= 0x0000001F;  
    BDBG_MSG(("(Boost=Tilt=1)LNA Gain = 0x%x", currentGain));

    if( currentGain < 0x1F )
    {
        BDBG_MSG(("Turning off Tilt..."));
        BLNA_3412_WriteRegisterField(hDev, MWC_CNTL,        SM_RESET, 1);
        BLNA_3412_WriteRegisterField(hDev, PD_OOB_OSC_CNTL, PD_RESET, 1);
        BLNA_3412_WriteRegisterField(hDev, LNA1_CNTL,       STAGE1_TILT_ON, 0);
        BLNA_3412_WaitForLock_LNA(hDev);
        tiltGain = BLNA_3412_ReadRegister(hDev,BCHP_BCM341X_regs_STATUS_READ2);
        tiltGain &= 0x0000001F;
        BDBG_MSG(("(Boost/Tilt = 1/0)LNA Gain = 0x%x.", tiltGain));

        if( ((tiltGain-currentGain) <= tolTilt) )
        {
            BDBG_MSG(("Turning Tilt back on..."));
            BLNA_3412_WriteRegisterField(hDev, MWC_CNTL,        SM_RESET, 1);
            BLNA_3412_WriteRegisterField(hDev, PD_OOB_OSC_CNTL, PD_RESET, 1);
            BLNA_3412_WriteRegisterField(hDev, LNA1_CNTL,       STAGE1_TILT_ON, 1);
            BLNA_3412_WaitForLock_LNA(hDev);
            currentGain = BLNA_3412_ReadRegister(hDev,BCHP_BCM341X_regs_STATUS_READ2);
            currentGain &= 0x0000001F;  
            BDBG_MSG(( "LNA Gain = 0x%x", currentGain));
        }

        /* chip with superboost mode */
        if(hDev->hasSuperboost)
        {
            if( currentGain < (0x1F-tolBoost) )
            {
                BDBG_MSG(("Turning off Enhanced Boost..."));
                BLNA_3412_WriteRegisterField(hDev, MWC_CNTL,			SM_RESET, 1);
                BLNA_3412_WriteRegisterField(hDev, PD_OOB_OSC_CNTL,	PD_RESET, 1);
                BLNA_3412_WriteRegisterField(hDev, LNA_ADDR,			SuperBoost_en, 0);
                BLNA_3412_WaitForLock_LNA(hDev);
                currentGain = BLNA_3412_ReadRegister(hDev,BCHP_BCM341X_regs_STATUS_READ2);
                currentGain &= 0x0000001F;  
                BDBG_MSG(( "LNA Gain = 0x%x", currentGain));
            }
        }

        if( currentGain < (0x1F-tolBoost) )
        {
            BDBG_MSG(("Turning off Boost..."));
            BLNA_3412_WriteRegisterField(hDev, MWC_CNTL,        SM_RESET, 1);
            BLNA_3412_WriteRegisterField(hDev, PD_OOB_OSC_CNTL, PD_RESET, 1);
            BLNA_3412_WriteRegisterField(hDev, LNA1_CNTL,       STAGE1_GAIN_BOOST, 0);
			BoostOn=0;
            BLNA_3412_WaitForLock_LNA(hDev);
            currentGain = BLNA_3412_ReadRegister(hDev,BCHP_BCM341X_regs_STATUS_READ2);
            currentGain &= 0x0000001F;  
            BDBG_MSG(( "LNA Gain = 0x%x", currentGain));
        }
    }
    else
    {
        BLNA_3412_WriteRegisterField(hDev, LNA1_CNTL, STAGE1_TILT_ON, 0);
    }

    currentGain = BLNA_3412_ReadRegister(hDev,BCHP_BCM341X_regs_STATUS_READ2);
    currentGain &= 0x0000003F;  

    if(hDev->hasSuperboost)
    {
        BDBG_MSG(("(Final SuperBoost=%d)",BLNA_3412_ReadRegisterField(hDev, LNA_ADDR,SuperBoost_en) ));
        BDBG_MSG(("Final(Boost/Tilt=%d/%d) LNA Gain=0x%x.\n",
                  BLNA_3412_ReadRegisterField(hDev, LNA1_CNTL,STAGE1_GAIN_BOOST),
                  BLNA_3412_ReadRegisterField(hDev, LNA1_CNTL,STAGE1_TILT_ON),
                  currentGain ));
    }
}

static BERR_Code BLNA_3412_Init(
	BLNA_3412_Handle hDev
	)
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    BLNA_3412_WriteRegisterField(hDev, MWC_CNTL, SM_RESET, 1);
    
    BLNA_3412_WriteRegisterField(hDev, MWC_CNTL, SM_RESET, 0);
    
    BLNA_3412_WriteRegister(hDev, BCHP_BCM341X_regs_MWC_CNTL, 0x3A252000);

    /* Change default MWC_LO, enable PM, select Poly High */
    BLNA_3412_WriteRegister(hDev, BCHP_BCM341X_regs_MWC_PM_CNTL, 0x101B0100); 

    /* PD_RESET=0 PD_BO=1 PD_OD=0 PD_PSET=4 OOB_PD=1 OOB_BIAS=0x34 OSC_PD=0 OSC_RESET=0 OSC_FREQ=0xf LT_LowNF=3 */
    BLNA_3412_WriteRegister(hDev, BCHP_BCM341X_regs_PD_OOB_OSC_CNTL, 0x24740f60);

    if(hDev->hasSuperboost)
    {   
        BLNA_3412_WriteRegisterField(hDev, PD_OOB_OSC_CNTL, PD_PSET_B, 3); /* set TOP */
    }
    else
    {
        BLNA_3412_WriteRegisterField(hDev, PD_OOB_OSC_CNTL, PD_PSET_B, 4);  /* set TOP to -23dB */
    }
    
    BLNA_3412_WriteRegisterField(hDev, PD_OOB_OSC_CNTL, PD_BO, 1);
    
    BLNA_3412_WriteRegisterField(hDev, PD_OOB_OSC_CNTL, PD_OD, 0);
    
    BLNA_3412_WriteRegisterField(hDev, PD_OOB_OSC_CNTL, LT_PD, 0);   /*MCW: OOB PD should be a setting? */
    
    {
        unsigned high = 63;
        unsigned low = 0;
        unsigned mid = 31;
        int i;
        unsigned PVT_COMP = 0;
        
        for (i = 0; i < 6; i++) {
          /* tgt.BCM3412.regset0.MWC_PM_CNTL.PM_VREF = CInt(mid) */
          BLNA_3412_WriteRegisterField(hDev, MWC_PM_CNTL, PM_VREF, (uint32_t)mid);
        
          BKNI_Sleep(2);
          BLNA_3412_WriteRegisterField(hDev, MWC_PM_CNTL, COMP_LAT_EN, 0);
          BLNA_3412_WriteRegisterField(hDev, MWC_PM_CNTL, COMP_LAT_EN, 1);
          PVT_COMP = BLNA_3412_ReadRegisterField(hDev, STATUS_READ2, COMP_OUT);
        
          if (PVT_COMP) {
            low = mid + 1;
            mid = (high + low) / 2;
          }
          else {
            high = mid - 1;
            mid = (high + low) / 2;
          }    
        }
        
        if (mid > 36) {
          BLNA_3412_WriteRegisterField(hDev, BG_LNA2_RCAL_CNTL, STAGE1_RCAL, 0);
          BLNA_3412_WriteRegisterField(hDev, BG_LNA2_RCAL_CNTL, STAGE2_RCAL, 1);
        }
        else if (mid > 31) {
          BLNA_3412_WriteRegisterField(hDev, BG_LNA2_RCAL_CNTL, STAGE1_RCAL, 1);
          BLNA_3412_WriteRegisterField(hDev, BG_LNA2_RCAL_CNTL, STAGE2_RCAL, 3);
        }
        else if (mid > 26) {
          BLNA_3412_WriteRegisterField(hDev, BG_LNA2_RCAL_CNTL, STAGE1_RCAL, 3);
          BLNA_3412_WriteRegisterField(hDev, BG_LNA2_RCAL_CNTL, STAGE2_RCAL, 3);
        }
        else if (mid > 21) {
          BLNA_3412_WriteRegisterField(hDev, BG_LNA2_RCAL_CNTL, STAGE1_RCAL, 5);
          BLNA_3412_WriteRegisterField(hDev, BG_LNA2_RCAL_CNTL, STAGE2_RCAL, 3);
        }
        else {
          BLNA_3412_WriteRegisterField(hDev, BG_LNA2_RCAL_CNTL, STAGE1_RCAL, 7);
          BLNA_3412_WriteRegisterField(hDev, BG_LNA2_RCAL_CNTL, STAGE2_RCAL, 5);
        }
        
        BLNA_3412_WriteRegisterField(hDev, MWC_PM_CNTL, PM_PD, 1);	/* Turn off the process monitor */
        
        if (hDev->settings.gainMode == BLNA_GainMode_eMGC)
        {
            BLNA_3412_WriteRegisterField(hDev, LNA_ADDR, STG1_GAIN_I2C, hDev->settings.manualGainVal);
            BLNA_3412_WriteRegisterField(hDev, SM_BYPASS, DEC_LAT_ENB_STG1_I2C, 1);
            BLNA_3412_WriteRegisterField(hDev, SM_BYPASS, DEC_LAT_ENB_STG1_I2C, 0);
        }
    }

	if( (hDev->chipId==0x3412) || (hDev->chipId==0x3413) )
	{
		/*Bcm93382.bcm3412.regset0.SM_BYPASS.ATE = &h0 */
		BLNA_3412_WriteRegisterField(hDev, SM_BYPASS, ATE, 0x0);	 	
	}
	   
	if(hDev->hasSuperboost)
	{
		/*Bcm93382.bcm3412.regset0.MWC_CNTL.SM_RESET = &h1 */
		BLNA_3412_WriteRegisterField(hDev, MWC_CNTL, SM_RESET, 0x01);
		/*Bcm93382.bcm3412.regset0.MWC_CNTL.SM_RESET = &h0 */
		BLNA_3412_WriteRegisterField(hDev, MWC_CNTL, SM_RESET, 0x00);
	}

    if (hDev->settings.gainMode == BLNA_GainMode_eAGC)
    {
        BLNA_3412_Optimize_LNA(hDev);
    }

    return( retCode );
}


/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/


BERR_Code BLNA_3412_GetDefaultSettings(
	BLNA_3412_Settings *pDefSettings,		/* [out] Returns default setting */
	BCHP_Handle hChip						/* [in] Chip handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;	
	BDBG_ENTER(BLNA_3412_GetDefaultSettings);
	BSTD_UNUSED(hChip);

	*pDefSettings = defDevSettings;

	BDBG_LEAVE(BLNA_3412_GetDefaultSettings);
	return( retCode );
}

BERR_Code BLNA_3412_Open(
	BLNA_Handle *phLna,					/* [out] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_I2C_Handle hI2CReg,			/* [in] I2C Register handle */
	const BLNA_3412_Settings *pDefSettings	/* [in] Default settings */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
 	BLNA_Handle hDev;
 	BLNA_3412_Handle h3412Dev;
	BLNA_3412_P_Settings *pLnaImplData;
    unsigned i;
    uint32_t	data;
	uint16_t i2c_addrs[]={0x60,0x6c,0x61,0x6d,0x62,0x6e,0x63,0x6f};

	BDBG_ENTER(BLNA_3412_Open);
	BDBG_ASSERT( hI2CReg );
	BSTD_UNUSED(hChip);

	hDev = NULL;
	if( pDefSettings == NULL)
	{
		retCode = BERR_INVALID_PARAMETER;
		BDBG_ERR(("BLNA_3431_Open: get a null settings\n"));
		goto done;
	}
	/* Alloc memory from the system heap */
	h3412Dev = (BLNA_3412_Handle) BKNI_Malloc( sizeof( BLNA_P_3412_Handle ) );
	if( h3412Dev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BLNA_3412_Open: BKNI_malloc() failed\n"));
		goto done;
	}
	BKNI_Memset( h3412Dev, 0x00, sizeof( BLNA_P_3412_Handle ) );

	hDev = (BLNA_Handle) BKNI_Malloc( sizeof( BLNA_P_Handle ) );
	if( hDev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BLNA_3412_Open: BKNI_malloc() failed\n"));
		BKNI_Free( h3412Dev );
		goto done;
	}
	BKNI_Memset( hDev, 0x00, sizeof( BLNA_P_Handle ) );

	h3412Dev->magicId = DEV_MAGIC_ID;
	pLnaImplData = &h3412Dev->settings;
	pLnaImplData->hI2CReg = hI2CReg;
	pLnaImplData->i2cAddr = pDefSettings->i2cAddr;
	if( (pDefSettings->i2cAddr & 0x80) )
	{
		BDBG_ERR(("I2C Address must be 7 bit format"));
		BDBG_ASSERT( !(pDefSettings->i2cAddr & 0x80) );
	}

    pLnaImplData->gainMode = pDefSettings->gainMode;
    for (i=0;i<BLNA_3412_MX_INBAND_PORTS;i++)
    {
        pLnaImplData->inBandPorts[i] = pDefSettings->inBandPorts[i];
    }
    for (i=0;i<BLNA_3412_MX_OUTOFBAND_PORTS;i++)
    {
        pLnaImplData->outOfBandPorts[i] = pDefSettings->outOfBandPorts[i];
    }
    pLnaImplData->agcOutputLevel = pDefSettings->agcOutputLevel;
    pLnaImplData->agcDeltaVal = pDefSettings->agcDeltaVal;
    pLnaImplData->manualGainVal = pDefSettings->manualGainVal;

	hDev->hDevImpl = (void *) h3412Dev;
	hDev->magicId = DEV_MAGIC_ID;
    hDev->pEnableAutomaticGainControl = NULL;
    hDev->pEnableManualGainControl = NULL;
    hDev->pSetInBandMode =  (BLNA_SetInBandModeFunc)BLNA_3412_SetInBandMode;
    hDev->pSetOutOfBandMode = NULL;
    hDev->pEnableExternalDriver = NULL;
    hDev->pDisableExternalDriver = NULL;
    hDev->pGetStatus = (BLNA_GetStatusFunc)BLNA_3412_GetStatus;
    hDev->pEnablePowerSaver = (BLNA_EnablePowerSaverFunc)BLNA_3412_EnablePowerSaver;
    hDev->pDisablePowerSaver = (BLNA_DisablePowerSaverFunc)BLNA_3412_DisablePowerSaver;
    hDev->pEnableLoopThrough = (BLNA_EnableLoopThroughFunc)BLNA_3412_EnableLoopThrough;
    hDev->pDisableLoopThrough = (BLNA_DisableLoopThroughFunc)BLNA_3412_DisableLoopThrough;
    hDev->pSetBoostMode = (BLNA_SetBoostModeFunc)BLNA_3412_SetBoostMode;

	hDev->pGetLnaAgcRegVal = (BLNA_GetLnaAgcRegValFunc) BLNA_3412_GetLnaAgcRegVal;
	hDev->pClose = (BLNA_CloseFunc) BLNA_3412_Close;	

	if (0==pLnaImplData->i2cAddr) {
		for (i=0;i<sizeof(i2c_addrs)/sizeof(uint16_t);i++) {
			BDBG_MSG(("Probe LNA Chip ID at addr %x",i2c_addrs[i]));
			if (BERR_SUCCESS == BREG_I2C_WriteNoAddr(pLnaImplData->hI2CReg, i2c_addrs[i], (uint8_t *)&data, 0)){
				pLnaImplData->i2cAddr = i2c_addrs[i];
				break;
			}
		}	
	}
    h3412Dev->chipId = BLNA_3412_ReadRegisterField(h3412Dev,CHIP_ID, CHIP_PART_NUMBER);
    BDBG_MSG(("Read LNA Chip ID is %x",h3412Dev->chipId));

    switch (h3412Dev->chipId)
    {
        case 0x3408 : 
        case 0x3410 :
            h3412Dev->hasSuperboost = false;
            h3412Dev->hasFixedGainLoopThru = false;
            h3412Dev->nInBandPorts = 1;
            h3412Dev->nOutOfBandPorts = 1;
            break;
        case 0x3406 :
        case 0x3412 :
            h3412Dev->hasSuperboost = true;
            h3412Dev->hasFixedGainLoopThru = true;
            h3412Dev->nInBandPorts = 2;
            h3412Dev->nOutOfBandPorts = 0;
            break;
        case 0x3413 :
            h3412Dev->hasSuperboost = true;
            h3412Dev->hasFixedGainLoopThru = false;
            h3412Dev->nInBandPorts = 1;
            h3412Dev->nOutOfBandPorts = 0;
            break;
        case 0x3414 :
            h3412Dev->hasSuperboost = true;
            h3412Dev->hasFixedGainLoopThru = false;
            h3412Dev->nInBandPorts = 2;
            h3412Dev->nOutOfBandPorts = 0;
            break;
        default :
            retCode = BERR_TRACE(BERR_NOT_SUPPORTED);
            BDBG_ERR(("BLNA_3412_Open: Chip ID 0x%04x not supported by this driver.\n",h3412Dev->chipId));
            BKNI_Free( h3412Dev );
            goto done;
            break;
    }

    BLNA_3412_DisablePowerSaver(h3412Dev);

    if (!pDefSettings->skipInit)
    {
        BLNA_3412_Init(h3412Dev);
    }
	
done:
	if( retCode != BERR_SUCCESS )
	{
		if( hDev != NULL )
		{
			BKNI_Free( hDev );
			hDev = NULL;
		}
	}
	*phLna = hDev;

	BDBG_LEAVE(BLNA_3412_Open);
	return( retCode );
}

static BERR_Code BLNA_3412_EnablePowerSaver(
    BLNA_3412_Handle hDev					/* [in] Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;	
	BDBG_ENTER(BLNA_3412_EnablePowerSaver);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    BLNA_3412_WriteRegisterField(hDev, BG_LNA2_RCAL_CNTL, BG_PD, 1);  /* power down band gap*/
	
	BDBG_LEAVE(BLNA_3412_EnablePowerSaver);
	return( retCode );	
}

static BERR_Code BLNA_3412_DisablePowerSaver(
    BLNA_3412_Handle hDev					/* [in] Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;	
	BDBG_ENTER(BLNA_3412_DisablePowerSaver);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	BLNA_3412_WriteRegisterField(hDev, BG_LNA2_RCAL_CNTL, BG_PD, 0);
	
	BDBG_LEAVE(BLNA_3412_DisablePowerSaver);
	return( retCode );	
}


static BERR_Code BLNA_3412_EnableLoopThrough(
    BLNA_3412_Handle hDev					/* [in] Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;	
	BDBG_ENTER(BLNA_3412_EnableLoopThrough);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if (hDev->hasFixedGainLoopThru)
    {
        BLNA_3412_WriteRegisterField(hDev, PD_OOB_OSC_CNTL, LT_PD, 0);  /* don't power down loop through */
    }
	
	BDBG_LEAVE(BLNA_3412_EnableLoopThrough);
	return( retCode );	
}

static BERR_Code BLNA_3412_DisableLoopThrough(
    BLNA_3412_Handle hDev					/* [in] Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;	
	BDBG_ENTER(BLNA_3412_DisableLoopThrough);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if (hDev->hasFixedGainLoopThru)
    {
        BLNA_3412_WriteRegisterField(hDev, PD_OOB_OSC_CNTL, LT_PD, 1);  /* power down loop through */
    }
	
	BDBG_LEAVE(BLNA_3412_DisableLoopThrough);
	return( retCode );	
}

static BERR_Code BLNA_3412_SetBoostMode(
    BLNA_3412_Handle hDev,					/* [in] Device handle */
	BLNA_BoostMode boostMode
	)
{
	BERR_Code retCode = BERR_SUCCESS;	
	BDBG_ENTER(BLNA_3412_SetBoostMode);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if(hDev->hasSuperboost)
    {
        switch (boostMode)
        {
            case BLNA_BoostMode_eNormal:
                BLNA_3412_WriteRegisterField(hDev, LNA1_CNTL,STAGE1_GAIN_BOOST, 0);
                BLNA_3412_WriteRegisterField(hDev, LNA_ADDR, SuperBoost_en, 0);
                break;
                
            case BLNA_BoostMode_eBoost:
                BLNA_3412_WriteRegisterField(hDev, LNA1_CNTL,STAGE1_GAIN_BOOST, 1);
                BLNA_3412_WriteRegisterField(hDev, LNA_ADDR, SuperBoost_en, 0);
                break;
                
            case BLNA_BoostMode_eSuperBoost:
                BLNA_3412_WriteRegisterField(hDev, LNA1_CNTL,STAGE1_GAIN_BOOST, 1);
                BLNA_3412_WriteRegisterField(hDev, LNA_ADDR, SuperBoost_en, 1);
                break;
                
            default:
               	retCode = BERR_INVALID_PARAMETER;	
                break;
        }
    }
    else
    	retCode = BERR_INVALID_PARAMETER;	
    

	BDBG_LEAVE(BLNA_3412_SetBoostMode);
	return( retCode );	
}

static BERR_Code BLNA_3412_SetInBandMode(
										 BLNA_3412_Handle hDev,		/* [in] Device handle */
										 unsigned int portNbr,		/* [in] Port number, 0-1 */
										 BLNA_InBandMode mode		/* [in] InBand mode */
										 )
{
	BERR_Code retCode = BERR_SUCCESS;
	
	BDBG_ENTER(BLNA_3412_SetInBandMode);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	if (portNbr+1 > hDev->nInBandPorts)
    {
        retCode = BLNA_ERR_INBAND_PORTNBR_OUT_OF_RANGE;
        goto done;
    }
	switch( mode )
	{
	case BLNA_InBandMode_eOpenCable:
	case BLNA_InBandMode_eAnalog:
	case BLNA_InBandMode_eDigital:
	case BLNA_InBandMode_ePowerDownRf:
		break;
	default:
		retCode = BLNA_ERR_INBAND_MODE;
		goto done;
	}

    retCode = BLNA_3412_SetSecondStagePower(hDev,(portNbr+1),(mode != BLNA_InBandMode_ePowerDownRf));

done:
	BDBG_LEAVE(BLNA_3412_SetInBandMode);
	return( retCode );
	
}

static BERR_Code BLNA_3412_GetStatus(
                         BLNA_3412_Handle hDev,		/* [in] Device handle */
						 BLNA_Status *pStatus		/* [out] Returns status */
						 )
{
	BERR_Code retCode = BERR_SUCCESS;
	BDBG_ENTER(BLNA_GetStatus);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	if( pStatus == NULL )
	{
		retCode = BERR_INVALID_PARAMETER;
		goto done;
	}
    else
    {
        unsigned i;
        pStatus->gainMode = hDev->settings.gainMode;
        BDBG_MSG(("gainMode = %u",pStatus->gainMode));
        for (i=0;i<BLNA_MX_INBAND_PORTS;i++)
        {
            pStatus->inBandPorts[i] = hDev->settings.inBandPorts[i];
            BDBG_MSG(("inBandPortMode[%u] = %u",i,pStatus->inBandPorts[i]));
        }
        for (i=0;i<BLNA_MX_OUTOFBAND_PORTS;i++)
        {
            pStatus->outOfBandPorts[i] = hDev->settings.outOfBandPorts[i];
            BDBG_MSG(("outOfBandPortMode[%u] = %u",i,pStatus->outOfBandPorts[i]));
        }
        for (i=0;i<BLNA_MX_EXTDRV;i++)
        {
            pStatus->isExternalDriverEnabled[i] = hDev->settings.isExternalDriverEnabled[i];
            BDBG_MSG(("isExternalDriverEnabled[%u] = %u",i,pStatus->isExternalDriverEnabled[i]));
        }

        pStatus->gainBoostEnabled = BLNA_3412_ReadRegisterField(hDev, LNA1_CNTL, STAGE1_GAIN_BOOST);
		BDBG_MSG(("gainBoostEnabled = %u",pStatus->gainBoostEnabled));
		pStatus->tiltEnabled = BLNA_3412_ReadRegisterField(hDev, LNA1_CNTL, STAGE1_TILT_ON);
		BDBG_MSG(("tiltEnabled = %u",pStatus->tiltEnabled));
		if(hDev->hasSuperboost)
		{
			pStatus->superBoostEnabled = BLNA_3412_ReadRegisterField(hDev, LNA_ADDR, SuperBoost_en);
		}
		else
		{
			pStatus->superBoostEnabled = false;
		}
		BDBG_MSG(("superBoostEnabled = %u",pStatus->superBoostEnabled));

        pStatus->agcOutputLevel = hDev->settings.agcOutputLevel;
        BDBG_MSG(("agcOutputLevel = %u",pStatus->agcOutputLevel));
        pStatus->agcDeltaVal = hDev->settings.agcDeltaVal;
        BDBG_MSG(("agcDeltaVal = %u",pStatus->agcDeltaVal));
        pStatus->manualGainVal = hDev->settings.manualGainVal;
        BDBG_MSG(("manualGainVal = %u",pStatus->manualGainVal));
    }
done:
	BDBG_LEAVE(BLNA_GetStatus);
	return( retCode );
}

