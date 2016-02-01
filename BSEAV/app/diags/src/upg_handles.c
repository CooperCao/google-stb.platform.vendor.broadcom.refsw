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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include "bstd.h"
#include "bkni.h"
#include "bchp.h"
#include "sys_handles.h"
#include "test.h"
#ifdef DIAGS_SPI_TEST
    #include "bspi.h"
#endif
#include "bpwm.h"
#ifdef DIAGS_BICP_TEST
    #include "bbcp.h"
#endif
#ifdef DIAGS_IRB_TEST
    #include "birb.h"
#endif
#include "nexus_platform_features.h"
#include "nexus_platform.h"
#include "nexus_base.h"
#include "upg_handles.h"
#include "sys_handles.h"
#include "bchp_sun_top_ctrl.h"
#include "priv/nexus_i2c_priv.h"

#define	UPG_CHK_RETCODE( rc, func )		\
do {										\
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {										\
        return rc;							\
    }										\
} while(0)

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/
#ifdef DIAGS_I2C_TEST
    NEXUS_I2cHandle 	diags_hI2c[NEXUS_NUM_I2C_CHANNELS];
    BREG_I2C_Handle   diags_hRegI2c[NEXUS_NUM_I2C_CHANNELS];
#endif

#ifdef DIAGS_SPI_TEST
    BSPI_Handle			diags_hSpi		= NULL;
    BSPI_ChannelHandle	diags_hSpiChan[MAX_SPI_CHANNELS];
    BREG_SPI_Handle		diags_hRegSpi [MAX_SPI_CHANNELS];
#endif

#ifdef DIAGS_BICAP_TEST
    BBCP_Handle			diags_hBcp		= NULL;
    BBCP_ChannelHandle	diags_hBcpChan[MAX_BCP_CHANNELS]	= { NULL, NULL };
#endif

#ifdef DIAGS_ICAP_TEST
    BICP_Handle			diags_hIcp		= NULL;
    BICP_ChannelHandle	diags_hIcpChan[MAX_ICP_CHANNELS]	= { NULL, NULL, NULL, NULL };
#endif

BKIR_Handle			diags_hKir		= NULL;
BKIR_ChannelHandle	diags_hKirChan[BKIR_N_CHANNELS];

#ifdef DIAGS_KPD_TEST
    BKPD_Handle			diags_hKpd		= NULL;
#endif

BLED_Handle			diags_hLed		= NULL;

#ifdef DIAGS_IRB_TEST
    BIRB_Handle			diags_hIrb		= NULL;
#endif

BPWM_Handle			diags_hPwm		= NULL;
BPWM_ChannelHandle	diags_hPwmChan[MAX_PWM_CHANNELS];

#ifdef DIAG_FE_BCM3250
    BKPD_Handle			diags_hKpd3250	= NULL;
    BLED_Handle			diags_hLed3250	= NULL;
#endif

/***********************************************************************
 *                      External References
 ***********************************************************************/

/***********************************************************************
 *                        Local Functions
 ***********************************************************************/

 /***********************************************************************
 *
 *  bcmOpenI2cDevice()
 * 
 *  Open I2C device and create I2C reg handle
 *
 ***********************************************************************/
#ifdef DIAGS_I2C_TEST
BERR_Code bcmOpenI2cDevice (void)
{
    int i;

    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Platform_GetConfiguration(&platformConfig);

    for (i=0; i<NEXUS_NUM_I2C_CHANNELS; i++)
    {
        #if (BCHP_CHIP==7344)
            if (i==3) continue; /* HW bug. Only affects A0? */
        #endif
        #ifdef NEXUS_MOCA_I2C_CHANNEL
            if (i==NEXUS_MOCA_I2C_CHANNEL) continue;
        #endif
        diags_hI2c[i] = platformConfig.i2c[i];
        diags_hRegI2c[i] = NEXUS_I2c_GetRegHandle(diags_hI2c[i], NULL);
    }

    return BERR_SUCCESS;
}
#endif

 /***********************************************************************
 *
 *  bcmOpenKirDevice()
 * 
 *  Open KIR device.
 *
 ***********************************************************************/
BERR_Code bcmOpenKirDevice ()
{
    unsigned int			chanNo;
    BKIR_ChannelSettings	defChnSettings;
    BERR_Code 				retCode = BERR_SUCCESS;

    UPG_CHK_RETCODE (retCode, BKIR_Open( &diags_hKir, bcmGetChipHandle(), 
                            bcmGetRegHandle(), bcmGetIntHandle(), 
                            (BKIR_Settings *)NULL ));

    for (chanNo=0; chanNo<BKIR_N_CHANNELS; chanNo++)
    {
        UPG_CHK_RETCODE (retCode, BKIR_GetChannelDefaultSettings( diags_hKir, chanNo, &defChnSettings ));
        UPG_CHK_RETCODE (retCode, BKIR_OpenChannel( diags_hKir, &diags_hKirChan[chanNo], chanNo, &defChnSettings ));
    }

    return retCode;
}

 /***********************************************************************
 *
 *  bcmOpenSpiDevice()
 * 
 *  Open SPI device and create SPI reg handle
 *
 ***********************************************************************/
#ifdef DIAGS_SPI_TEST
    BERR_Code bcmOpenSpiDevice (void)
    {
        BSPI_ChannelSettings	defChnSettings;
        BERR_Code 				retCode = BERR_SUCCESS;
        uint32_t				i;

        UPG_CHK_RETCODE (retCode, BSPI_Open( &diags_hSpi, bcmGetChipHandle(), 
                                bcmGetRegHandle(), bcmGetIntHandle(), 
                                (BSPI_Settings *)NULL ));

        for (i=0; i < MAX_SPI_CHANNELS; i++)
        {
            UPG_CHK_RETCODE (retCode, BSPI_GetChannelDefaultSettings( diags_hSpi, i, &defChnSettings ));
            UPG_CHK_RETCODE (retCode, BSPI_OpenChannel( diags_hSpi, &diags_hSpiChan[i], i, &defChnSettings ));
            UPG_CHK_RETCODE (retCode, BSPI_CreateSpiRegHandle (diags_hSpiChan[i], &diags_hRegSpi[i]));
        }
        return retCode;
    }
#endif

 /***********************************************************************
 *
 *  bcmOpenKpdDevice()
 * 
 *  Open IRB device.
 *
 ***********************************************************************/
#ifdef DIAGS_KPD_TEST
    BERR_Code bcmOpenKpdDevice (bool use3250Kpd)
    {
        BERR_Code 				retCode = BERR_SUCCESS;
        BKPD_Settings			defSettings;

        if (use3250Kpd)
        {
            #if 0
                BKPD_GetDefaultSettings( &defSettings, bcmGet3250ChipHandle() );
                UPG_CHK_RETCODE (retCode, BKPD_Open( &diags_hKpd3250, bcmGet3250ChipHandle(), 
                                    bcmGet3250RegHandle(), bcmGet3250IntHandle(), 
                                    &defSettings ));
            #else
                retCode = BERR_INVALID_PARAMETER;
            #endif
        }
        else
        {
            BKPD_GetDefaultSettings( &defSettings, bcmGetChipHandle() );
            UPG_CHK_RETCODE (retCode, BKPD_Open( &diags_hKpd, bcmGetChipHandle(), 
                                bcmGetRegHandle(), bcmGetIntHandle(), 
                                &defSettings ));
        }

        return retCode;
    }
#endif

 /***********************************************************************
 *
 *  bcmOpenLedDevice()
 * 
 *  Open LED device.
 *
 ***********************************************************************/
BERR_Code bcmOpenLedDevice (bool use3250Led)
{
    BERR_Code 				retCode = BERR_SUCCESS;
    BLED_Settings			defSettings;

    if (use3250Led)
    {
        #if 0
            BLED_GetDefaultSettings( &defSettings, bcmGet3250ChipHandle() );
            UPG_CHK_RETCODE (retCode, BLED_Open( &diags_hLed3250, bcmGet3250ChipHandle(), 
                                bcmGet3250RegHandle(), &defSettings ));
        #else
            retCode = BERR_INVALID_PARAMETER;
        #endif
    }
    else
    {
        BLED_GetDefaultSettings( &defSettings, bcmGetChipHandle() );
        UPG_CHK_RETCODE (retCode, BLED_Open( &diags_hLed, bcmGetChipHandle(), 
                            bcmGetRegHandle(), &defSettings ));
    }
    return retCode;
}

 /***********************************************************************
 *
 *  bcmOpenIcpDevice()
 * 
 *  Open ICP device.
 *
 ***********************************************************************/
#ifdef DIAGS_ICAP_TEST
    BERR_Code bcmOpenIcpDevice (void)
    {
        unsigned int			i;
        BICP_ChannelSettings	defChnSettings;
        BERR_Code 				retCode = BERR_SUCCESS;

        UPG_CHK_RETCODE (retCode, BICP_Open( &diags_hIcp, bcmGetChipHandle(), 
                                bcmGetRegHandle(), bcmGetIntHandle(), 
                                (BICP_Settings *)NULL ));
        for (i=0; i < MAX_ICP_CHANNELS; i++)
        {
            UPG_CHK_RETCODE (retCode, BICP_GetChannelDefaultSettings( diags_hIcp, i, &defChnSettings ));
            UPG_CHK_RETCODE (retCode, BICP_OpenChannel( diags_hIcp, &diags_hIcpChan[i], i, &defChnSettings ));
        }

        return retCode;
    }
#endif

 /***********************************************************************
 *
 *  bcmOpenBcpDevice()
 * 
 *  Open BICAP device.
 *
 ***********************************************************************/
#ifdef DIAGS_BICAP_TEST
    BERR_Code bcmOpenBcpDevice (void)
    {
        unsigned int			i;
        BBCP_Settings			defSettings;
        BBCP_ChannelSettings	defChnSettings;
        BERR_Code 				retCode = BERR_SUCCESS;

        UPG_CHK_RETCODE (retCode, BBCP_GetDefaultSettings( &defSettings ));
        UPG_CHK_RETCODE (retCode, BBCP_Open( &diags_hBcp, bcmGetChipHandle(), 
                                bcmGetRegHandle(), bcmGetIntHandle(), 
                                &defSettings ));
        for (i=0; i < MAX_BCP_CHANNELS; i++)
        {
            UPG_CHK_RETCODE (retCode, BBCP_GetChannelDefaultSettings( diags_hBcp, i, &defChnSettings ));
            UPG_CHK_RETCODE (retCode, BBCP_OpenChannel( diags_hBcp, &diags_hBcpChan[i], i, &defChnSettings ));
        }

        return retCode;
    }
#endif

 /***********************************************************************
 *
 *  bcmOpenIrbDevice()
 * 
 *  Open IRB device.
 *
 ***********************************************************************/
#ifdef DIAGS_IRB_TEST
    BERR_Code bcmOpenIrbDevice (void)
    {
        BERR_Code 				retCode = BERR_SUCCESS;
        BIRB_Settings			defSettings;

        UPG_CHK_RETCODE (retCode, BIRB_GetDefaultSettings( &defSettings, bcmGetChipHandle() ));
        UPG_CHK_RETCODE (retCode, BIRB_Open( &diags_hIrb, bcmGetChipHandle(), 
                                bcmGetRegHandle(), bcmGetIntHandle(), 
                                &defSettings ));
        return retCode;
    }
#endif

 /***********************************************************************
 *
 *  bcmOpenPwmDevice()
 * 
 *  Open PWM device.
 *
 ***********************************************************************/
BERR_Code bcmOpenPwmDevice ()
{
    unsigned int			i;
    BPWM_ChannelSettings	defChnSettings;
    BERR_Code 				retCode = BERR_SUCCESS;
    unsigned int			num_pwm_channels;
    #if (BCHP_CHIP == 7420)
        uint32_t				ulReg;
    #endif

    UPG_CHK_RETCODE (retCode, BPWM_Open( &diags_hPwm, bcmGetChipHandle(), 
                            bcmGetRegHandle(), (BPWM_Settings *)NULL ));
    
    #if (BCHP_CHIP == 7420)
        ulReg = BREG_Read32(bcmGetRegHandle(), BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS_0);
        if (BCHP_GET_FIELD_DATA( ulReg, SUN_TOP_CTRL_OTP_OPTION_STATUS_0, otp_option_pwm_pair_disable ))
            num_pwm_channels = MAX_PWM_CHANNELS / 2;
        else
    #endif
    num_pwm_channels = MAX_PWM_CHANNELS;

    #if (BCHP_CHIP == 7425)
        num_pwm_channels = MAX_PWM_CHANNELS / 2;
    #endif

    for (i=0; i < num_pwm_channels; i++)
    {
        UPG_CHK_RETCODE (retCode, BPWM_GetChannelDefaultSettings( diags_hPwm, i, &defChnSettings ));
        UPG_CHK_RETCODE (retCode, BPWM_OpenChannel( diags_hPwm, &diags_hPwmChan[i], i, &defChnSettings ));
    }
    
    return retCode;
}

/***********************************************************************
 *                        Public Functions
 ***********************************************************************/
 /***********************************************************************
 *
 *  bcmOpenUpgDevices()
 * 
 *  Open and initialize all UPG devices.
 *
 ***********************************************************************/
BERR_Code bcmOpenUpgDevices(void)
{
    BERR_Code 	retCode = BERR_SUCCESS;

    #ifdef DIAGS_I2C_TEST
        UPG_CHK_RETCODE (retCode, bcmOpenI2cDevice());
    #endif
    /*Support PWM in Linux*/
    UPG_CHK_RETCODE (retCode, bcmOpenPwmDevice());

    #ifndef LINUX /* TBD */
        UPG_CHK_RETCODE (retCode, bcmOpenKirDevice());

        #ifdef DIAGS_SPI_TEST
            UPG_CHK_RETCODE (retCode, bcmOpenSpiDevice());
        #endif

        UPG_CHK_RETCODE (retCode, bcmOpenLedDevice(false));

        #ifdef NEXUS_HAS_KPD
            UPG_CHK_RETCODE (retCode, bcmOpenKpdDevice(false));
        #endif

        #ifdef DIAGS_IRB_TEST
            UPG_CHK_RETCODE (retCode, bcmOpenIrbDevice());
        #endif

        #ifdef DIAGS_ICAP_TEST
            UPG_CHK_RETCODE (retCode, bcmOpenIcpDevice());
        #endif

        #if 0 /*(def DIAGS_BICAP_TEST*/
            UPG_CHK_RETCODE (retCode, bcmOpenBcpDevice());
        #endif
    #endif

    return retCode;
}
      
 /***********************************************************************
 *
 *  bcmGetI2cRegHandle()
 * 
 *  Get I2C register handle
 *
 ***********************************************************************/
#ifdef DIAGS_I2C_TEST
    NEXUS_I2cHandle bcmGetI2cHandle (uint32_t chan) { return diags_hI2c[chan]; }
#endif

 /***********************************************************************
 *
 *  bcmGetKirHandle()
 *  bcmGetKirChanHandle()
 *
 *  Get KIR, KIR channel handle
 *
 ***********************************************************************/
BKIR_Handle bcmGetKirHandle (void) { return diags_hKir; }
BKIR_ChannelHandle bcmGetKirChannelHandle (int chan) { return diags_hKirChan[chan]; }

 /***********************************************************************
 *
 *  bcmGetSpiRegHandle()
 * 
 *  Get SPI register handle
 *
 ***********************************************************************/
#ifdef DIAGS_SPI_TEST
    BREG_SPI_Handle bcmGetSpiRegHandle (uint32_t chan) { return diags_hRegSpi[chan]; }
#endif

 /***********************************************************************
 *
 *  bcmGetKpdHandle()
 *
 *  Get KPD handle
 *
 ***********************************************************************/
#ifdef DIAGS_KPD_TEST
    BKPD_Handle bcmGetKpdHandle (void) { return diags_hKpd; }
#endif

 /***********************************************************************
 *
 *  bcmGetLedHandle()
 *
 *  Get LED handle
 *
 ***********************************************************************/
BLED_Handle bcmGetLedHandle (void) { return diags_hLed; }

 /***********************************************************************
 *
 *  bcmGetIrbHandle()
 *
 *  Get IRB handle
 *
 ***********************************************************************/
#ifdef DIAGS_IRB_TEST
    BIRB_Handle bcmGetIrbHandle (void) { return diags_hIrb; }
#endif

 /***********************************************************************
 *
 *  bcmGetPwmHandle()
 *  bcmGetPwmChannelHandle()
 *
 *  Get PWM handle
 *  Get PWM channel handle
 *
 ***********************************************************************/
BPWM_Handle bcmGetPwmHandle (void) { return diags_hPwm; }
BPWM_ChannelHandle bcmGetPwmChannelHandle (uint32_t chan) { return diags_hPwmChan[chan]; }

 /***********************************************************************
 *
 *  bcmGetBcpHandle()
 *  bcmGetBcpChannelHandle()
 *
 *  Get BCP handle
 *  Get BCP channel handle
 *
 ***********************************************************************/
#ifdef DIAGS_BICAP_TEST
    BBCP_Handle bcmGetBcpHandle (void) { return diags_hBcp; }
    BBCP_ChannelHandle bcmGetBcpChannelHandle (uint32_t chan) { return diags_hBcpChan[chan]; }
#endif

 /***********************************************************************
 *
 *  bcmGetIcpHandle()
 *  bcmGetIcpChannelHandle()
 *
 *  Get ICP handle
 *  Get ICP channel handle
 *
 ***********************************************************************/
#ifdef DIAGS_ICAP_TEST
    BICP_Handle bcmGetIcpHandle (void) { return diags_hIcp; }
    BICP_ChannelHandle bcmGetIcpChannelHandle (uint32_t chan) { return diags_hIcpChan[chan]; }
#endif

 /***********************************************************************
 *
 *  bcmGet3250KpdHandle()
 *
 *  Get 3250 KPD handle
 *
 ***********************************************************************/
#ifdef DIAG_FE_BCM3250
    BKPD_Handle bcmGet3250KpdHandle (void) { return diags_hKpd3250; }
#endif

 /***********************************************************************
 *
 *  bcmGet3250LedHandle()
 *
 *  Get LED handle
 *
 ***********************************************************************/
#ifdef DIAG_FE_BCM3250
    BLED_Handle bcmGet3250LedHandle (void) { return diags_hLed3250; }
#endif
