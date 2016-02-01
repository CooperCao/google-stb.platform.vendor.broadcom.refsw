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
*   API name: Frontend 3440
*    APIs to open, close, and setup initial settings for a BCM3440
*    Tuner Device.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "nexus_frontend_module.h"
#include "priv/nexus_i2c_priv.h"

BDBG_MODULE(nexus_tuner_3440);

BDBG_OBJECT_ID(NEXUS_3440);

typedef struct NEXUS_3440
{
    BDBG_OBJECT(NEXUS_3440)
    BREG_I2C_Handle i2c;
    uint16_t i2cAddr;
    unsigned frequency;
    int32_t ifOffset;
} NEXUS_3440;

static void NEXUS_Tuner_P_Write3440(NEXUS_3440 *pDevice, uint8_t reg, uint8_t val);
static void NEXUS_Tuner_P_Read3440(NEXUS_3440 *pDevice, uint8_t reg, uint8_t *pVal);
static NEXUS_Error NEXUS_Tuner_P_Tune3440(void *handle, NEXUS_TunerMode mode, unsigned frequency);
static void NEXUS_Tuner_P_Close3440(void *handle);
static void NEXUS_Tuner_P_LpfAuto3440(NEXUS_3440 *pDevice, uint8_t cutoff_mhz);
static NEXUS_Error NEXUS_Tuner_P_Set3440Cutoff(NEXUS_3440 *pDevice, uint32_t cutoff_mhz);

/***************************************************************************
Summary:
	Get the default settings for a BCM3440 tuner
 ***************************************************************************/
void NEXUS_Tuner_GetDefault3440Settings(
    NEXUS_3440Settings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->i2cAddr = 0xc8>>1;
}

/***************************************************************************
Summary:
	Open a handle to a BCM3440 device.
 ***************************************************************************/
NEXUS_TunerHandle NEXUS_Tuner_Open3440(  /* attr{destructor=NEXUS_Tuner_Close} */
    const NEXUS_3440Settings *pSettings
    )
{
    NEXUS_3440 *pDevice;
    NEXUS_TunerHandle tuner;
    NEXUS_TunerExtensionSettings tunerSettings;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pSettings->i2cDevice);
    
    pDevice = BKNI_Malloc(sizeof(NEXUS_3440));
    if ( NULL == pDevice )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    BKNI_Memset(pDevice, 0, sizeof(pDevice));
    BDBG_OBJECT_SET(pDevice, NEXUS_3440);
    pDevice->i2cAddr = pSettings->i2cAddr;
    pDevice->i2c = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NEXUS_MODULE_SELF);
    if ( NULL == pDevice->i2c )
    {
        BDBG_ERR(("Invalid I2C Handle"));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        BKNI_Free(pDevice);
        return NULL;
    }

    NEXUS_Tuner_P_GetDefaultExtensionSettings(&tunerSettings);
    tunerSettings.pDeviceHandle = pDevice;
    tunerSettings.close = NEXUS_Tuner_P_Close3440;
    tunerSettings.setFrequency = NEXUS_Tuner_P_Tune3440;
    tuner = NEXUS_Tuner_P_CreateExtension(&tunerSettings);
    if ( NULL == tuner )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_OBJECT_DESTROY(pDevice, NEXUS_3440);
        BKNI_Free(pDevice);
        return NULL;
    }

    NEXUS_Tuner_P_Write3440(pDevice, 0x07, 0x00);
    NEXUS_Tuner_P_Write3440(pDevice, 0x08, 0x00);
    NEXUS_Tuner_P_Write3440(pDevice, 0x09, 0x00);
    NEXUS_Tuner_P_Write3440(pDevice, 0x0A, 0x00);
    NEXUS_Tuner_P_Write3440(pDevice, 0x0B, 0x10);  
    NEXUS_Tuner_P_Write3440(pDevice, 0x0C, 0x40);
    NEXUS_Tuner_P_Write3440(pDevice, 0x0D, 0x00);
    NEXUS_Tuner_P_Write3440(pDevice, 0x0E, 0xC4);
    NEXUS_Tuner_P_Write3440(pDevice, 0x0F, 0x00);
    NEXUS_Tuner_P_Write3440(pDevice, 0x10, 0x44);
    NEXUS_Tuner_P_Write3440(pDevice, 0x11, 0x00);
    NEXUS_Tuner_P_Write3440(pDevice, 0x12, 0x00);
    NEXUS_Tuner_P_Write3440(pDevice, 0x13, 0x00);
    NEXUS_Tuner_P_Write3440(pDevice, 0x14, 0xE9);
    NEXUS_Tuner_P_Write3440(pDevice, 0x15, 0x00);
    NEXUS_Tuner_P_Write3440(pDevice, 0x16, 0x07);
    NEXUS_Tuner_P_Write3440(pDevice, 0x17, 0x04);  
    NEXUS_Tuner_P_Write3440(pDevice, 0x18, 0xEC);  
    NEXUS_Tuner_P_Write3440(pDevice, 0x19, 0x80);
    NEXUS_Tuner_P_Write3440(pDevice, 0x1A, 0x00);
    NEXUS_Tuner_P_Write3440(pDevice, 0x1B, 0xF0); 
    NEXUS_Tuner_P_Write3440(pDevice, 0x0F, 0x2C);
    NEXUS_Tuner_P_Write3440(pDevice, 0x18, 0xEC); 
    NEXUS_Tuner_P_Write3440(pDevice, 0x19, 0x80); 
    NEXUS_Tuner_P_Write3440(pDevice, 0x19, 0x00); 
    NEXUS_Tuner_P_Write3440(pDevice, 0x18, 0xE4); 
    BKNI_Sleep(2);
    NEXUS_Tuner_P_Write3440(pDevice, 0x18, 0xEC);  
    BKNI_Sleep(6);
    NEXUS_Tuner_P_Write3440(pDevice, 0x18, 0xE4); 
    BKNI_Sleep(2);
    NEXUS_Tuner_P_Write3440(pDevice, 0x18, 0xEC); 
    BKNI_Sleep(6);
    NEXUS_Tuner_P_Write3440(pDevice, 0x19, 0x80);

    return tuner;
}

/***************************************************************************
Summary:
	Set the symbol rate for a 3440 tuner device
 ***************************************************************************/
NEXUS_Error NEXUS_Tuner_Set3440SymbolRate(
    NEXUS_TunerHandle handle,
    uint32_t symbolRate,
    bool nyquist20          /* If true, the nyquist filter is 0.20.  If false, 0.35 is assumed. */
    )
{
    NEXUS_3440 *pDevice;
    uint64_t cutoff;
    uint32_t freq4, actual_tuner_offset;

    BDBG_ASSERT(NULL != handle);
    pDevice = handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3440);

    cutoff = symbolRate;

    /* set the filter on BCM3440 */
    if ( nyquist20 )
        cutoff = (cutoff * (uint64_t)6)/(uint64_t)10;
    else
        cutoff = (cutoff * (uint64_t)675)/(uint64_t)1000;

    freq4 = pDevice->frequency >> 2;
    freq4 += 500000;
    freq4 /= 1000000;
    freq4 *= 4000000;
    if (freq4 > pDevice->frequency)
        actual_tuner_offset = (freq4 - pDevice->frequency);
    else
        actual_tuner_offset = (pDevice->frequency - freq4);
    cutoff += actual_tuner_offset;
    cutoff += 5000000;      /* lnb_offset */

    NEXUS_Tuner_P_Set3440Cutoff(pDevice, (uint32_t)(cutoff / 1000000));

    /* program the IF offset due to 4MHz step on BCM3440 */
    pDevice->ifOffset = (int32_t)freq4 - (int32_t)pDevice->frequency;

    return NEXUS_SUCCESS;
}

void NEXUS_Tuner_Get3440IfOffset(
    NEXUS_TunerHandle handle,
    int32_t *pIfOffset
    )
{
    NEXUS_3440 *pDevice;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pIfOffset);
    pDevice = handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3440);

    *pIfOffset = pDevice->ifOffset;
}

/***************************************************************************
Summary:
	Set the cutoff frequency for a 3440 tuner device
 ***************************************************************************/
static NEXUS_Error NEXUS_Tuner_P_Set3440Cutoff(
    NEXUS_3440 *pDevice,
    uint32_t cutoff_mhz
    )
{
    uint8_t d1, d2, c, sb;

    NEXUS_Tuner_P_LpfAuto3440(pDevice, 15);
    NEXUS_Tuner_P_Read3440(pDevice, 0x05, &d1);
    d1 &= 0x3F;

    NEXUS_Tuner_P_LpfAuto3440(pDevice, 25);
    NEXUS_Tuner_P_Read3440(pDevice, 0x05, &d2);
    d2 &= 0x3F;

    if ((d2 < 24) && (d1 < 13))
    {
       /* R- corner */
       if (cutoff_mhz < 26)
          c = (uint8_t)((unsigned long)(cutoff_mhz * (d2+2)) / 25);
       else
          c = cutoff_mhz + d2 - 23;
    }
    else if ((d2 < 24) && (d1 > 12) && (d1 < 17))
    {
       /* fast corner */
       c = cutoff_mhz;
    }
    else if (d2 > 26)
    {
       /* slow or R+ corner */
       if (cutoff_mhz < 26)
          c = (uint8_t)((uint32_t)(cutoff_mhz * (d2-2)) / 25);
       else
          c = ((cutoff_mhz * 6) / 5) + d2 - 32;      
    }
    else
    {
       /* nominal corner */
       if (cutoff_mhz < 26)
          c = cutoff_mhz;
       else
          c = (uint8_t)((uint32_t)(cutoff_mhz * 21) / 20);      
    }

    c &= 0x3F;

    NEXUS_Tuner_P_Read3440(pDevice, 0x19, &sb);
    sb &= 0x80;
    sb |= (0x40 | c);
    NEXUS_Tuner_P_Write3440(pDevice, 0x19, sb);

    NEXUS_Tuner_P_Read3440(pDevice, 0x1A, &sb);
    sb &= 0x80;
    sb |= (0x40 | c);
    NEXUS_Tuner_P_Write3440(pDevice, 0x1A, sb);

    return NEXUS_SUCCESS;
}

static void NEXUS_Tuner_P_Write3440(NEXUS_3440 *pDevice, uint8_t reg, uint8_t val)
{
   BERR_Code retCode;
   uint8_t buf[2];
   
   BDBG_OBJECT_ASSERT(pDevice, NEXUS_3440);

   buf[0] = reg;
   buf[1] = val;
   retCode = BREG_I2C_WriteNoAddr(pDevice->i2c, pDevice->i2cAddr, buf, 2);
   if (retCode != BERR_SUCCESS)
   {
       (void)BERR_TRACE(retCode);
   }
}

static void NEXUS_Tuner_P_Read3440(NEXUS_3440 *pDevice, uint8_t reg, uint8_t *pVal)
{
   BERR_Code retCode;
   
   BDBG_OBJECT_ASSERT(pDevice, NEXUS_3440);
   BDBG_ASSERT(NULL != pVal);

   retCode = BREG_I2C_Read(pDevice->i2c, pDevice->i2cAddr, reg, pVal, 1);
   if (retCode != BERR_SUCCESS)
   {
       (void)BERR_TRACE(retCode);
   }
}

static NEXUS_Error NEXUS_Tuner_P_Tune3440(void *handle, NEXUS_TunerMode mode, unsigned frequency)
{
    uint32_t ulval1, tuner_freq4, freq_mhz;
    uint8_t regval;
    NEXUS_3440 *pDevice = handle;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3440);
    BSTD_UNUSED(mode);

    freq_mhz = (frequency + 500000) / 1000000; /* round to nearest mhz */

    /* look for nearest 4MHz step */
    ulval1 = ((freq_mhz >> 1) + 1) >> 1;
    tuner_freq4 = ulval1 << 2;

    ulval1 -= 17;
    NEXUS_Tuner_P_Write3440(pDevice, 0x07, ulval1 & 0xFF);

    if (tuner_freq4 > 1450)
    {
       if (tuner_freq4 > 1850)
       {
          if (tuner_freq4 > 2350)
              regval = 3;
          else
             regval = 2;
       }
       else
          regval = 1;
    }
    else
       regval = 0;
    regval |= (0x14 | ((ulval1 >> 3) & 0xE0));

    /* set the VCO and all the dividers */
    NEXUS_Tuner_P_Write3440(pDevice, 0x08, regval);

    /* start the acquisition on the falling edge */
    NEXUS_Tuner_P_Write3440(pDevice, 0x08, regval & 0xFB);

    /* Store frequency for later */
    pDevice->frequency = frequency;

    return NEXUS_SUCCESS;
}

static void NEXUS_Tuner_P_Close3440(void *handle)
{
    NEXUS_3440 *pDevice;

    pDevice = handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3440);
    BDBG_OBJECT_DESTROY(pDevice, NEXUS_3440);
    BKNI_Free(pDevice);
}

static void NEXUS_Tuner_P_LpfAuto3440(NEXUS_3440 *pDevice, uint8_t cutoff_mhz)
{
   uint8_t bcm3440_0F, sb;
   BDBG_OBJECT_ASSERT(pDevice, NEXUS_3440);
   
   NEXUS_Tuner_P_Read3440(pDevice, 0x1A, &sb);
   sb &= ~0x40;
   NEXUS_Tuner_P_Write3440(pDevice, 0x1A, sb);
   
   NEXUS_Tuner_P_Read3440(pDevice, 0x19, &sb);
   sb &= 0x3F;
   NEXUS_Tuner_P_Write3440(pDevice, 0x19, sb);   
   
   bcm3440_0F = (cutoff_mhz << 1) & 0x7E;
   NEXUS_Tuner_P_Write3440(pDevice, 0x0F, bcm3440_0F);
   
   NEXUS_Tuner_P_Write3440(pDevice, 0x18, 0x6C & 0xF7);  /* low power setting */

   BKNI_Sleep(1);

   NEXUS_Tuner_P_Write3440(pDevice, 0x18, 0x6C | 0x08);  /* low power setting */

   BKNI_Sleep(2);
   
   NEXUS_Tuner_P_Write3440(pDevice, 0x18, 0x6C & 0xF7); /* low power setting */

   BKNI_Sleep(1);

   NEXUS_Tuner_P_Write3440(pDevice, 0x18, 0x6C | 0x08); /* low power setting */
   
   BKNI_Sleep(2);
   
   NEXUS_Tuner_P_Read3440(pDevice, 0x19, &sb);   
   sb |= 0x80;
   NEXUS_Tuner_P_Write3440(pDevice, 0x19, sb);      
}

