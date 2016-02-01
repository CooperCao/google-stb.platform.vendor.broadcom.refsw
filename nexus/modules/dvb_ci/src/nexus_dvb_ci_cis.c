/***************************************************************************
*     (c)2004-2009 Broadcom Corporation
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
* API Description:
*   API name: DvbCi
*    Specific APIs related DVB Common Interface
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_dvb_ci_module.h"

BDBG_MODULE(nexus_dvb_ci_cis);

#define TPL_LINKTARGET 0x13
#define TPL_DEVICE_0A 0x1d
#define TPL_DEVICE_0C 0x1c
#define TPL_VERS_1 0x15
#define TPL_CONFIG 0x1a
#define CCST_CIF 0xc0
#define TPL_CFTABLE_ENTRY 0x1b
#define TPL_END 0xff
#define TPL_NO_LINK 0x14

#define INCREMENT_INDEX(i,val,next) do { if ( (i)+(val) >= (1024*1024) || (i)+(val)>(next) ) goto overflow; (i) += (val); } while (0)

/* The CIS is written on even bytes.  This routine will parse the CIS and check for a valid device, extracting the COR address and value */
NEXUS_Error NEXUS_DvbCi_P_ReadCis(NEXUS_DvbCiHandle handle)
{
    uint8_t tuple, link;
    uint8_t noLinkFlag=0;
    int i, j, nextLink, cc=0;
    uint8_t byte;
    bool foundCor=false, foundCen=false;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DvbCi);

    #if 0
    BDBG_MSG(("CIS DUMP"));
    for ( i = 0; i < 511; i+=2 )
    {
        uint8_t value = ((uint8_t *)handle->pBaseAddress)[i];
        BDBG_MSG(("[%02x] 0x%02x ", i, value));
    }
    #endif

    BKNI_Memset(handle->manufacturerName, 0, sizeof(handle->manufacturerName));
    BKNI_Memset(handle->productName, 0, sizeof(handle->manufacturerName));
    BKNI_Memset(handle->additionalInfo, 0, sizeof(handle->additionalInfo));
    handle->vpp5v = true;   /* Default to 5v for DVB-CI */
    handle->interruptsSupported = false;
    handle->validCis = false;

    for ( i = 0;; )
    {
        BDBG_MSG(("CIS Parsing - offset 0x%x\n", i));
        tuple = ((uint8_t *)handle->pBaseAddress)[i];
        i+=2;
        link = ((uint8_t *)handle->pBaseAddress)[i];
        i+=2;
        nextLink = i + link + link;

        switch ( tuple )
        {
        case TPL_VERS_1:
            BDBG_MSG(("TPL_VERS_1: major %x, minor %x.", ((uint8_t *)handle->pBaseAddress)[i], ((uint8_t *)handle->pBaseAddress)[i+2]));
            INCREMENT_INDEX(i,4,nextLink);
            j=0;
            while ( (byte = ((uint8_t *)handle->pBaseAddress)[i]) )
            {
                if (byte == 0xff)
                    break;
                if ( j < NEXUS_DVB_CI_MANUFACTURER_NAME_MAX-1 )
                {
                    handle->manufacturerName[j++] = byte;
                }
                INCREMENT_INDEX(i,2,nextLink);
            }
            /* Skip NUL byte */
            INCREMENT_INDEX(i,2,nextLink);
            BDBG_MSG(("Manufacturer Name: '%s'", handle->manufacturerName));
            j=0;
            while ( (byte = ((uint8_t *)handle->pBaseAddress)[i]) )
            {
                if (byte == 0xff)
                    break;
                if ( j < NEXUS_DVB_CI_PRODUCT_NAME_MAX-1 )
                {
                    handle->productName[j++] = byte;
                }
                INCREMENT_INDEX(i,2,nextLink);
            }
            /* Skip NUL byte */
            INCREMENT_INDEX(i,2,nextLink);
            BDBG_MSG(("Product Name: '%s'", handle->productName));
            j=0;
            while ( (byte = ((uint8_t *)handle->pBaseAddress)[i]) )
            {
                if (byte == 0xff)
                    break;
                if ( j < NEXUS_DVB_CI_ADDITIONAL_INFO_MAX-1 )
                {
                    handle->additionalInfo[j++] = byte;
                }
                INCREMENT_INDEX(i,2,nextLink);
            }
            BDBG_MSG(("Additional Info: '%s'", handle->additionalInfo));
            break;
            
        case TPL_CONFIG:
        {
            uint8_t rmsz, rasz, last;
            uint32_t corVal;

            rmsz = ((((uint8_t *)handle->pBaseAddress)[i]&0x3c)>>2);
            rasz = (((uint8_t *)handle->pBaseAddress)[i]&0x03);
            INCREMENT_INDEX(i,2,nextLink);
            last = (((uint8_t *)handle->pBaseAddress)[i]&0x3f);
            INCREMENT_INDEX(i,2,nextLink);

            BDBG_MSG(("rmsz = %x, rasz = %x, last = %x", rmsz, rasz, last));

            corVal = 0;
            for (j=0; j<=rasz; j++)
                corVal |= (((uint32_t)handle->pBaseAddress[i+j*2])<<(j*8));
            corVal &= 0x03ffffff;
            BDBG_MSG(("cor address offset 0x%x", corVal));
            handle->corAddr = corVal;
            INCREMENT_INDEX(i,j*2,nextLink);

            if ( (handle->pBaseAddress[i] & 0x01) == 0 )
            {
                BDBG_WRN(("COR not present"));
            }
            foundCor = true;
            INCREMENT_INDEX(i,2*(rmsz+1),nextLink);
            if ( handle->pBaseAddress[i] != CCST_CIF )
            {
                BDBG_ERR(("CCST_CIF not present"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
            INCREMENT_INDEX(i,4,nextLink);
            #if 0
            /* CableCard - useful for HPNx testing */
            if ( handle->pBaseAddress[i] != 0x41 || handle->pBaseAddress[i+2] != 0x03 )
            #else
            /* DVB-CI */
            if ( handle->pBaseAddress[i] != 0x41 || handle->pBaseAddress[i+2] != 0x02 )
            #endif
            {
                BDBG_ERR(("CCST_CIF IFN not correct %x %x", handle->pBaseAddress[i], handle->pBaseAddress[i+2]));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }

            #if 0
            /* Useful for debug */
            INCREMENT_INDEX(i,4,nextLink);
            BDBG_MSG(("CCST_CIF: Interface Description: "));
            while ( handle->pBaseAddress[i] && i < nextLink)
            {
                BDBG_MSG(("   %c", handle->pBaseAddress[i]));
                INCREMENT_INDEX(i,2,nextLink);
            }
            #endif
        }
        break;
        case TPL_CFTABLE_ENTRY:
        {
            uint8_t fs, pdParSel, normv, val;

            handle->corValue = 0x40 | (handle->pBaseAddress[i] & 0x3f);
            foundCen = true;
            BDBG_MSG(("CISTPL_CFTABLE_ENTRY: TPCE_INDX %x", handle->corValue));
            INCREMENT_INDEX(i,2,nextLink);  /* Points to TPCE_IF */
            BDBG_MSG(("CISTPL_CFTABLE_ENTRY: TPCE_IF %x", handle->pBaseAddress[i]));
            INCREMENT_INDEX(i,2,nextLink);  /* Points to TPCE_FS */
            fs = handle->pBaseAddress[i];
            BDBG_MSG(("CISTPL_CFTABLE_ENTRY: TPCE_FS %x, i= 0x%x", fs, i));

	     val = fs & 0x03;
            switch (val)
            {
            case 0x02: /* VCC and VPP PD. */
            case 0x01: /* VCC */
                INCREMENT_INDEX(i,2,nextLink);  /* points to VCC PD par sel */
                pdParSel = handle->pBaseAddress[i];
                BDBG_MSG(("CISTPL_CFTABLE_ENTRY: VCC TPCE_PD Par Sel %x", pdParSel));
                for (j=0; j<8; j++)
                    if ((1<<j) & pdParSel)
                    {
                        while (handle->pBaseAddress[i] & 0x80)
                            INCREMENT_INDEX(i,2,nextLink);
                        INCREMENT_INDEX(i,2,nextLink);
                    }
					
	         if (val == 0x02) /* handle VPP */
	         {
                    INCREMENT_INDEX(i,2,nextLink);  /* points to Vpp PD par sel */
                    pdParSel = handle->pBaseAddress[i];
		      BDBG_MSG(("CISTPL_CFTABLE_ENTRY: VPP TPCE_PD Par Sel %x", pdParSel));
		      if (pdParSel & 1)
		      {
                        INCREMENT_INDEX(i,2,nextLink); 
                        normv = handle->pBaseAddress[i];
			   if ((normv & 0x07) == 5)
                        {
				if (normv & 0x80)
                                INCREMENT_INDEX(i,2,nextLink);
				
                            /* exponent is 1V. */
                            normv >>= 3;
                            normv &= 0x0f;
                            if (normv >= 6 && normv < 8) /* 6:3, 7:3.5. */
                            {
                                BDBG_MSG(("CISTPL_CFTABLE_ENTRY: VPP norminal 3.3V"));
                                handle->vpp5v = false;
                            }
                        }
		      }
                    for (j=1; j<8; j++)
                        if ((1<<j) & pdParSel)
                        {
                            while (handle->pBaseAddress[i] & 0x80)
                                INCREMENT_INDEX(i,2,nextLink);
                            INCREMENT_INDEX(i,2,nextLink);
                        }
	         }
                break;
                
            default:
                BDBG_WRN(("CISTPL_CFTABLE_ENTRY: NO TPCE_PD for VPP, use 5V for Vpp by default."));
                break;
            }
            if (fs & 0x04) /* timing */
		 INCREMENT_INDEX(i,2,nextLink); 

            if (fs & 0x08) /* IO Space */
            {
		 INCREMENT_INDEX(i,2,nextLink); 
		 val = handle->pBaseAddress[i];
		 if (val & 0x80) INCREMENT_INDEX(i,2,nextLink); 
            }
	     if (fs & 0x10) /* TPCE_IR */
            {
		 INCREMENT_INDEX(i,2,nextLink); 
		 val = handle->pBaseAddress[i];
		 BDBG_MSG(("CISTPL_CFTABLE_ENTRY: TPCE_IR %x", val));
               handle->interruptsSupported = (val & (1<<5))?true:false; /* Bit 5 of TPCE_IR indicates interrupt support */
               BDBG_WRN(("InterruptsSupported %d", handle->interruptsSupported));
            }		 
        }
        break;
        case TPL_NO_LINK:
            noLinkFlag = 1;
            break;
        case TPL_END:
            if ( noLinkFlag == 0 )
                BDBG_WRN(("have not rcved NO_LINK before END"));
            goto done;
        default:
            if (cc++ < 50)
            {
                BDBG_MSG(("CIS Tuple %x received.", tuple));
            }
            else
            {
                BDBG_ERR(("Too many tuples"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
            break;
        }

        i = nextLink;
    }

done:
    if ( !foundCor )
    {
        BDBG_ERR(("Did not find CISTPL_CONFIG"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    if ( !foundCen )
    {
        BDBG_ERR(("Did not find CISTPL_CFTABLE_ENTRY"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    handle->validCis = true;

    return BERR_SUCCESS;

overflow:
    BDBG_ERR(("Overflow processing CIS"));
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

