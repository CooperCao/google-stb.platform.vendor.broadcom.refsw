/***************************************************************************
 *     Copyright (c) 2005-2013, Broadcom Corporation
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
 * Module Description: Porting Interface for the UHF Receiver Module.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 * ***************************************************************************/
#include "test.h"

#ifdef DIAGS_UHFR_TEST

#include <stdio.h>
#include <string.h>
#include "buhf.h"
#include "prompt.h"
#if (BCHP_CHIP == 7325 )
#include "bchp_clkgen.h"
#endif
#if (BCHP_CHIP == 7400 || BCHP_CHIP == 7403 || BCHP_CHIP == 7405)  
#include "bchp_clk.h"
#endif
#include "nexus_platform_features.h"
#include "sys_handles.h"

void uhfr_test (void)
{
    BUHF_Handle hUhfr[NEXUS_NUM_UHF_INPUTS];
    BUHF_Settings settings[NEXUS_NUM_UHF_INPUTS];
    BUHF_Data  data;
    unsigned char temp[NEXUS_NUM_UHF_INPUTS];
    unsigned int chan_num[NEXUS_NUM_UHF_INPUTS];
    unsigned int tempData=0;   
    int x;
    BCHP_Handle hChip = bcmGetChipHandle();
    BREG_Handle hRegister = bcmGetRegHandle();
    BINT_Handle hInterrupt = bcmGetIntHandle();
    
    /* NOTE: If required, get settings from the user, else use default. */
    for (x=0; x<NEXUS_NUM_UHF_INPUTS; x++)
    {
        BUHF_GetDefaultSettings (&settings[x]);
    }

    for (x=0; x<NEXUS_NUM_UHF_INPUTS; x++)
    {
        settings[x].mode = BUHF_Mode_eAdvanced;
    }

    /* Get channel no. from the user */
    for (x=0; x<NEXUS_NUM_UHF_INPUTS; x++)
    {
		while (1)
		{
	        printf("\nEnter channel number for uhf input %d (For EchoStar: 1-8, For DirecTv:9):  ", x);
			fflush(stdout);
			chan_num[x] = NoPrompt();
			if ((1 <= chan_num[x]) && (chan_num[x] <= 9))
			{
			    switch (chan_num[x])
			    {
			        case 1:
			            settings[x].channel = BUHF_ChanNum_eChan1;
			            break;
			        case 2:
			            settings[x].channel = BUHF_ChanNum_eChan2;
			            break;
			        case 3:
			            settings[x].channel = BUHF_ChanNum_eChan3;
			            break;
			        case 4:
			            settings[x].channel = BUHF_ChanNum_eChan4;
			            break;
			        case 5:
			            settings[x].channel = BUHF_ChanNum_eChan5;
			            break;
			        case 6:
			            settings[x].channel = BUHF_ChanNum_eChan6;
			            break;
			        case 7:
			            settings[x].channel = BUHF_ChanNum_eChan7;
			            break;
			        case 8:
			            settings[x].channel = BUHF_ChanNum_eChan8;
			            break;
			        case 9:
			            settings[x].channel = BUHF_ChanNum_eChan9;
			            break;
			    }
				break;
			}
		}

		while (1)
		{
	        printf("\nSelect Injection Type for uhf input %d (Enter %d for low side injection, %d for high side): ", x, BUHF_InjType_eLow, BUHF_InjType_eHigh);
			fflush(stdout);
			settings[x].injection = NoPrompt();
			if ((settings[x].injection==BUHF_InjType_eLow) || (settings[x].injection==BUHF_InjType_eHigh))
				break;
		}

		while (1)
		{
	        printf("\nSelect Freq Offset Adjustment for uhf input %d (Enter 1 to Enable, 0 to Disable): ", x);
			fflush(stdout);
	        settings[x].bFreqAdjust = NoPrompt();
			if ((settings[x].bFreqAdjust==0) || (settings[x].bFreqAdjust==1))
				break;
		}

		while (1)
		{
	        printf("\nSelect BCH Error Correction for uhf input %d (Enter 1 to Enable, 0 to Disable):", x);
			fflush(stdout);
	        settings[x].bchErrorCorrection = NoPrompt();
			if ((settings[x].bchErrorCorrection==0) || (settings[x].bchErrorCorrection==1))
				break;
		}
    }
    
#if (BCHP_CHIP == 7325)
    ui32RegVal=BREG_Read32(hRegister, BCHP_CLKGEN_MISC);
	printf("\nBCHP_CLKGEN_MISC=0x%x before\n", ui32RegVal);
    ui32RegVal |= (BCHP_FIELD_DATA (CLKGEN_MISC, UHFR_CLK_SEL, 1));
    BREG_Write32 (hRegister, BCHP_CLKGEN_MISC , ui32RegVal);   
    ui32RegVal=BREG_Read32(hRegister, BCHP_CLKGEN_MISC);
	printf("BCHP_CLKGEN_MISC=0x%x after\n", ui32RegVal);
#endif
#if (BCHP_CHIP == 7400 || BCHP_CHIP == 7405)
    {
        uint32_t ui32RegVal;
        ui32RegVal=BREG_Read32(hRegister, BCHP_CLK_MISC);
        ui32RegVal |= (BCHP_FIELD_DATA (CLK_MISC, UHFR_CLK_SEL, 1));
        BREG_Write32 (hRegister, BCHP_CLK_MISC , ui32RegVal);   
    }
#endif
#if (BCHP_CHIP == 7403)
    ui32RegVal=BREG_Read32(hRegister, BCHP_CLK_MISC);
    ui32RegVal |= (BCHP_FIELD_DATA (CLK_MISC, UHFR_CLK_INV_SEL, 1));
    BREG_Write32 (hRegister, BCHP_CLK_MISC , ui32RegVal);   
#endif

	#if (BCHP_CHIP==7346)
    for (x=0; x<NEXUS_NUM_UHF_INPUTS; x++)
    {
		settings[x].bShiftIf = false;
		settings[x].uiFreqShift = 0;
	}
	#else
	{
		unsigned int uiFreqShift;
	    char str[10];
	    printf("\nEnter frequency shift:  ");
	    rdstr(str);
	    sscanf(str, "%d", &uiFreqShift);
	    for (x=0; x<NEXUS_NUM_UHF_INPUTS; x++)
	    {
			settings[x].bShiftIf = true;
			settings[x].uiFreqShift = uiFreqShift;
		}
	}
	#endif
    
    for (x=0; x<NEXUS_NUM_UHF_INPUTS; x++)
    {
        BUHF_Open (&hUhfr[x], hChip, hRegister, hInterrupt, x, &settings[x]);
    }

    printf("\nPress buttons on the remote.  Pressing any key on the keyboard exits.\n");

    while (1)
    {   
        for (x=0; x<NEXUS_NUM_UHF_INPUTS; x++)
        {
            BUHF_IsDataReady (hUhfr[x], &temp[x]);
        }
        
        for (x=0; x<NEXUS_NUM_UHF_INPUTS; x++)
        {
            if (temp[x]) 
            {
                BUHF_Read (hUhfr[x], &data);
                if (settings[x].channel == BUHF_ChanNum_eChan9)
                {
                    /* prepend "binary 00010000" to the 32b data when packet
                       is valid. directv data packet is supposed to be 40b. */
                    printf("Data received at UHFR %d: 0x10%08x DirecTv type Packet \n\n", x, data.value);
                }            
                else
                {
                    if (data.prType == BUHF_PrType_e1) 
                        tempData = data.value >> 6;
                    else if (data.prType == BUHF_PrType_e2) 
                        tempData = data.value >> 27;
            
                    printf("Data received at UHFR %d: 0x%08x, Preamble type %d (orig data=0x%08x)\n\n", 
                            x, tempData, data.prType, data.value);
                }
            }
        }
		if (det_in_char()) break;
    }

    for (x=0; x<NEXUS_NUM_UHF_INPUTS; x++)
    {
        BUHF_Close (hUhfr[x]);
    }
}

#else

void uhfr_test (void)
{
    printf("Not enabled\n");
}

#endif /* DIAGS_UHFR_TEST */