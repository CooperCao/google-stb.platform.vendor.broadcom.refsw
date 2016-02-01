/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
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

#include "nexus_types.h"
#include "nexus_platform.h"

#if NEXUS_HAS_I2C && NEXUS_HAS_FRONTEND

#include "priv/nexus_core.h"
#include "nexus_frontend.h"
#include "nexus_platform_features.h"
#include "nexus_platform_priv.h"
#include "nexus_base.h"
#include "nexus_input_band.h"
#if NEXUS_PLATFORM_7420_DBS
#include "nexus_frontend_4506.h"
#include "bchp_hif_cpu_intr1.h"
#endif
#if NEXUS_PLATFORM_7420_CABLE
#include "nexus_frontend_3255.h"
#endif

BDBG_MODULE(nexus_platform_frontend);

#if NEXUS_PLATFORM_7420_DBS
static NEXUS_FrontendHandle g_onboard4506[NEXUS_MAX_FRONTENDS] = {NULL};

NEXUS_Error NEXUS_Platform_InitFrontend()
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_FrontendUserParameters userParams;
    unsigned i;
    NEXUS_4506Settings settings4506;
	NEXUS_FrontendHandle frontend = NULL;
	unsigned frontendNum = 0;

    /* Open on-board 4506 */
    NEXUS_Frontend_GetDefault4506Settings(&settings4506);
    settings4506.i2cDevice = pConfig->i2c[3];

    if (!settings4506.i2cDevice) {
		BDBG_ERR(("Unable to initialize I2C \n"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }


	/* Initialize the 3 onboard BCM4506 tuners on 97420DBS */
	for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
	{
		switch(i)
		{
			/* Tuner 0 */
			case 0:
			case 1:
    			settings4506.i2cAddr = 0x69;
    			settings4506.isrNumber = BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_0_CPU_INTR_SHIFT+32;
				break;

			/* Tuner 1 */
			case 2:
			case 3:
    			settings4506.i2cAddr = 0x68;
    			settings4506.isrNumber = BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_1_CPU_INTR_SHIFT+32;
				break;
					
			/* Tuner 2 */
			case 4:
			case 5:
    			settings4506.i2cAddr = 0x67;
    			settings4506.isrNumber = BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_3_CPU_INTR_SHIFT+32;
				break;
					
    		default:
        		BDBG_ERR(("Invalid Frontend = %d", i));
        		break;
		}	

		/* Channel 0 of Tuner */
		if ( (i%2) == 0)	
   			settings4506.channelNumber = 0;
		/* Channel 1 of Tuner */
		else
   			settings4506.channelNumber = 1;

    	g_onboard4506[frontendNum] = frontend = NEXUS_Frontend_Open4506(&settings4506);
				
    	if ( NULL == frontend )
    	{
			BDBG_ERR(("Unable to init on-board 4506 \n"));
    	}

		/* Set the appropriate input bands */
    	NEXUS_Frontend_GetUserParameters(frontend, &userParams);
    	userParams.pParam2 = NULL;

		switch(i)
		{
			/* Tuner 0 */
			case 0:
    			userParams.param1 = NEXUS_InputBand_e0;
				break;
			case 1:
    			userParams.param1 = NEXUS_InputBand_e1;
				break;

			/* Tuner 1 */
			case 2:
    			userParams.param1 = NEXUS_InputBand_e2;
				break;
			case 3:
    			userParams.param1 = NEXUS_InputBand_e3;
				break;
					
			/* Tuner 2 */
			case 4:
    			userParams.param1 = NEXUS_InputBand_e4;
				break;
			case 5:
    			userParams.param1 = NEXUS_InputBand_e5;
				break;
					
    		default:
        		BDBG_ERR(("Invalid Frontend = %d", i));
        		break;
		}	

    	NEXUS_Frontend_SetUserParameters(frontend, &userParams);
    	pConfig->frontend[frontendNum] = frontend;
		frontend = NULL;
		frontendNum++;
	}

    return BERR_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    unsigned i;

	for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
	{
    	NEXUS_Frontend_Close(g_onboard4506[i]);
    	g_onboard4506[i] = NULL;
	}
}

#elif NEXUS_PLATFORM_7420_CABLE
static NEXUS_FrontendHandle g_onboard3255[NEXUS_MAX_FRONTENDS] = {NULL};


NEXUS_Error NEXUS_Platform_InitFrontend()
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_InputBandSettings inputBandSettings;
    unsigned i;
	NEXUS_FrontendHandle frontend = NULL;
	unsigned frontendNum = 0;

    for ( i=0; i< NEXUS_MAX_FRONTENDS; i++)
    {
        NEXUS_3255Settings st3255Settings;
        NEXUS_Frontend_GetDefault3255Settings(&st3255Settings);
        st3255Settings.channelNumber = i;
        st3255Settings.type = (i < NEXUS_MAX_3255_ADSCHN) ?
        NEXUS_3255ChannelType_eInBand : NEXUS_3255ChannelType_eOutOfBand;
        BDBG_MSG(("Opening onboard 3255 %d", i));
        g_onboard3255[i] = NEXUS_Frontend_Open3255(&st3255Settings);

        if ( NULL == g_onboard3255[i] )
        {
            BDBG_ERR(("Unable to open onboard 3255 tuner/demodulator  %d", i));
            continue;
            /*return BERR_TRACE(BERR_NOT_SUPPORTED);*/
        }

        NEXUS_Frontend_GetUserParameters(g_onboard3255[i], &userParams);
        userParams.param1 = NEXUS_InputBand_e0 + i;
        userParams.pParam2 = NULL;
        NEXUS_Frontend_SetUserParameters(g_onboard3255[i], &userParams);

        NEXUS_InputBand_GetSettings(i, &inputBandSettings);
        inputBandSettings.clockActiveHigh = false;
        NEXUS_InputBand_SetSettings(i, &inputBandSettings);
    }
    /* Store in config structure */
    /* frontend[0] is reserved for Docsis tuner, so map it to the third tuner*/
    pConfig->frontend[0] = g_onboard3255[1];
    pConfig->frontend[1] = g_onboard3255[2];
    pConfig->frontend[2] = g_onboard3255[0];
    pConfig->frontend[3] = g_onboard3255[3];

    return BERR_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    unsigned i;

    for (i =0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if ( g_onboard3255[i] ) NEXUS_Frontend_Close(g_onboard3255[i]);
        g_onboard3255[i] = NULL;
    }
}

#else

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
	return 0;
}

void NEXUS_Platform_UninitFrontend(void)
{
}

#endif

BTRC_MODULE(ChnChange_TuneStreamer, ENABLE);

NEXUS_Error
NEXUS_Platform_GetStreamerInputBand(unsigned index, NEXUS_InputBand *pInputBand)
{
    BDBG_ASSERT(pInputBand);
    if (index > 0) {
        BDBG_ERR(("Only 1 streamer input available"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BTRC_TRACE(ChnChange_TuneStreamer, START);
    *pInputBand = NEXUS_InputBand_e5;
    BTRC_TRACE(ChnChange_TuneStreamer, STOP);
    return NEXUS_SUCCESS;
}

#endif

NEXUS_FrontendHandle NEXUS_Platform_OpenFrontend(
    unsigned id /* platform assigned ID for this frontend. See NEXUS_FrontendUserParameters.id.
                   See nexus_platform_frontend.c for ID assignment and/or see 
                   nexus_platform_features.h for possible platform-specific macros.
                */
    )
{
    NEXUS_Error errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
    BSTD_UNUSED(errCode);
    BSTD_UNUSED(id);
    return NULL;
}


