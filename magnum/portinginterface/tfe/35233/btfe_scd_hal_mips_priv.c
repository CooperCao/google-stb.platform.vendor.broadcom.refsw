/***************************************************************************
 *     (c)2004-2010 Broadcom Corporation
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:] CD Hardware Abstraction layer for 35330 with PMON
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ************************************************************/
 
 
/****************************************************************************/
/*
 *  SCD Hardware Abstraction Layer (HAL) for X255 with PMON (solo mode)
 */
/****************************************************************************/

/****************************************************************************/
/* options */
/****************************************************************************/

/* #define LOAD_FW_FROM_FILE */

/****************************************************************************/
/* includes */
/****************************************************************************/

#include "btfe_scd_priv.h"
#include "btfe_scd_int_priv.h"
#include "btfe_scd_35233_priv.h"
#include "bchp_dfe_agcdec_8051.h"
#include "bchp_dfe_bertdec_8051.h"
#include "bchp_dfe_eqdec_8051.h"
#include "bchp_dfe_fecdec_8051.h"
#include "bchp_dfe_fedec_8051.h"
#include "bchp_dfe_mfdec_8051.h"
#include "bchp_dfe_miscdec_8051.h"
#include "bchp_dfe_ntscdec_8051.h"
#include "bchp_dfe_ofsdec_8051.h"
#include "bchp_dfe_ucdec_8051.h"

#include "bchp_dfe_agcdec.h"
#include "bchp_dfe_bertdec.h"
#include "bchp_dfe_eqdec.h"
#include "bchp_dfe_fecdec.h"
#include "bchp_dfe_fedec.h"
#include "bchp_dfe_mfdec.h"
#include "bchp_dfe_miscdec.h"
#include "bchp_dfe_ntscdec.h"
#include "bchp_dfe_ofsdec.h"
#include "bchp_dfe_ucdec.h"


#include "bchp_35233.h"


#include "breg_mem.h"

BDBG_MODULE(btfe_scd_hal_mips);

#define BCHP_DFE_BASE_ADDR 0x100000


#define Dfe_ReadReg32(a, offset) BREG_Read32(g_reg_handle, (offset))
#define Dfe_WriteReg32(a, offset, value) BREG_Write32(g_reg_handle, (offset), value)

#define GETMEM_REGMM32(offset)        (BREG_Read32(g_reg_handle, (offset)))
#define SETMEM_REGMM32(offset, value) (BREG_Write32(g_reg_handle, (offset), value))

#define GETREG_REGMM32(reg)           GETMEM_REGMM32(mm##reg)
#define SETREG_REGMM32(reg, value)    SETMEM_REGMM32(mm##reg, value)


#define MODIFYFLD(var, reg, field, val)	(var = (var & (~reg##__##field##__MASK)) | (((unsigned long)(val) << reg##__##field##__SHIFT) & reg##__##field##__MASK))


/*****************************************************************************
*                                                                            
*  Tuner selection is decided at compile time by defining the FAT_TUNER and
*  SECOND_TUNER environment variable to one of the options defined below                           
*                                                                            
*  If FAT_TUNER or SECOND_TUNER is not defined they default to NXPTD1636FN and
*  NXPTD1636FN_B.                    
*                                                                           
*  OEM TUNER SUPPORT:
*  If you wish to use your own tuner that is not supported through ACL, you have
*  to define ENABLE_OEM_TUNER_A_SUPPORT in oem_opt.h. You do not have to define
*  any other environment variable like FAT_TUNER. Further, once option is
*  defined the driver will not default to the default NXPTD1636FN tuner during run-time either. 
*  The symbols for default NXPTD1636FN tuner will however be built, but this is harmless.
*
*  A second OEM tuner can similarly be added by define ENABLE_OEM_TUNER_B_SUPPORT in 
*  oem_opt.h 
*
* NOTE:
*  If there is only one OEM tuner in the system, please use ENABLE_OEM_TUNER_A_SUPPORT.
*                                                                           
*****************************************************************************/

#include "X233hex.c"  /* X320 uCode array */



/*************/
/* constants */
/*************/

/* chips */
#define CHIP_0  0   /* X255 internal demod instance */
#define CHIP_1  1   /* external demod instance */

/* tuners */
#define CHIP_0_FDC_TUNER_INSTANCE                         0    
#define CHIP_0_FAT_TUNER_NXPTD1636F_INSTANCE              1    

uint16_t chipId[SCD_MAX_CHIP]   = {0x255, 0};
uint16_t chipAddr[SCD_MAX_CHIP] = {0x0, 0};

#define VPA13HR_INPUT_MASK          (3<<4)
#define VPA13HR_INPUT_ANTENNA_VALUE (1<<4)
#define VPA13HR_INPUT_CABLE_VALUE   (2<<4)

#define VPA13HR_MODE_MASK           (1<<6)
#define VPA13HR_MODE_ANALOG_VALUE   (1<<6)
#define VPA13HR_MODE_DIGITAL_VALUE  (0<<6)

#define CHIP_INDEX 0

/*******************/
/* local variables */
/*******************/

static SCD_HANDLE g_chip_handle[SCD_MAX_CHIP];
static uint8_t *g_X233micro[SCD_MAX_CHIP] = {0, 0};
static BREG_Handle g_reg_handle;

/*********************/
/* private functions */
/*********************/


/********************/
/* public functions */
/********************/

SCD_RESULT BTFE_P_HalInitialize(uint32_t flags, void *reg_handle)
{
    BSTD_UNUSED(flags);
    BSTD_UNUSED(reg_handle);

    g_reg_handle = (BREG_Handle) reg_handle;

    HAL_FUNC_DEBUG(BDBG_MSG(("BTFE_P_HalInitialize(flags=%08X)", flags)));

    /* detect if external demod is present

       IMPORTANT NOTE:
       external demod requires GPIO10.22 = GPIO5.7 = 0 in xilleon_nvm.conf 
    halFindChipI2cAddrAndType(-1, &chipId[CHIP_1], &chipAddr[CHIP_1]);
    */

    g_X233micro[CHIP_0] = X233micro;

    if (BTFE_P_X233AddChip(CHIP_0, CHIP_0_FAT_TUNER_NXPTD1636F_INSTANCE, CHIP_0_FDC_TUNER_INSTANCE, &g_chip_handle[CHIP_0], g_X233micro[CHIP_0]) != SCD_RESULT__OK) return SCD_RESULT__CHIP_NOT_AVAILABLE;

    return SCD_RESULT__OK;
}

/****************************************************************************/

SCD_RESULT BTFE_P_HalCleanup(void)
{
    HAL_FUNC_DEBUG(BDBG_MSG(("BTFE_P_HalCleanup()")));

    return SCD_RESULT__OK;
}

/****************************************************************************/

SCD_RESULT BTFE_P_HalOpenChip(uint32_t instance)
{
    (void) instance;

    HAL_FUNC_DEBUG(BDBG_MSG(("BTFE_P_HalOpenChip(instance=%u)", instance)));

    return SCD_RESULT__OK;
}

/****************************************************************************/

SCD_RESULT BTFE_P_HalWriteChip(uint32_t instance, uint32_t aper, uint32_t offset, uint32_t length, uint8_t *buffer)
{
    uint32_t treg;

/*    HAL_FUNC_DEBUG(BDBG_MSG(("BTFE_P_HalWriteChip(instance=%u, aper=%u, offset=%08X, &buffer=%08X, length=%u)", instance, aper, offset, buffer, length))); */

   /* switch based on instance

      aper determines write type:
      0 = dfe_indirect (all registers are 8 bit, accessed through DFE_INDEX_DATA in mmr0)
      1 = mmr0 (only 32 bit access allowed - length must be 4, only used for debug)       */

    switch(instance)
    {
        case CHIP_0:
            if(0 == aper)
            {
                while(length--)
                {
                    Dfe_WriteReg32(CHIP_INDEX, (BCHP_DFE_BASE_ADDR+(offset++ << 2)), (*buffer++));
                }
                return SCD_RESULT__OK;
            }
            else if(1 == aper)
            {
                if(4 == length)
                {
                    treg = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | (buffer[3]);

                    Dfe_WriteReg32(CHIP_INDEX, offset, treg);
   
                    return SCD_RESULT__OK;
                }
            }
            else
            {
                BDBG_ERR(("BTFE_P_HalWriteChip: instance %u bad aper (%u)", instance, aper));
            }
            return SCD_RESULT__ERROR;
    }

    BDBG_ERR(("BTFE_P_HalWriteChip: bad instance (%u)", instance));

    return SCD_RESULT__ERROR;
}

/****************************************************************************/

SCD_RESULT BTFE_P_HalReadChip(uint32_t instance, uint32_t aper, uint32_t offset, uint32_t length, uint8_t *buffer)
{
    uint32_t treg;

    HAL_FUNC_DEBUG(BDBG_MSG(("BTFE_P_HalReadChip(instance=%u, aper=%u, offset=%08X, &buffer=%08X, length=%u)", instance, aper, offset, buffer, length)));

   /* switch based on instance

      aper determines write type:
      0 = dfe_indirect (all registers are 8 bit, accessed through DFE_INDEX_DATA in mmr0)
      1 = mmr0 (only 32 bit access allowed - length must be 4, only used for debug)       */

    switch(instance)
    {
        case CHIP_0:
            if(aper == 0)
            {
                while(length--)
                {
                    *buffer++ = (uint8_t)(Dfe_ReadReg32(CHIP_INDEX, (BCHP_DFE_BASE_ADDR+(offset++ << 2))) & 0xFF);			
                }
                return SCD_RESULT__OK;
            }
            else if(aper == 1)
            {
                if(length == 4)
                {
                    treg = Dfe_ReadReg32(CHIP_INDEX, offset);

                    buffer[0] = (uint8_t) ((treg >> 24) & 0xFF);
                    buffer[1] = (uint8_t) ((treg >> 16) & 0xFF);
                    buffer[2] = (uint8_t) ((treg >>  8) & 0xFF);
                    buffer[3] = (uint8_t) ((treg >>  0) & 0xFF);

                    return SCD_RESULT__OK;
                }
            }
            else
            {
                BDBG_ERR(("BTFE_P_HalReadChip: instance %u bad aper (%u)", instance, aper));
            }
            return SCD_RESULT__ERROR;

        }

    BDBG_MSG(("BTFE_P_HalReadChip: bad instance (%u)", instance));

    return SCD_RESULT__ERROR;
}

/****************************************************************************/

SCD_RESULT BTFE_P_HalCloseChip(uint32_t instance)
{
    (void) instance;

    HAL_FUNC_DEBUG(BDBG_MSG(("BTFE_P_HalCloseChip(instance=%u)", instance)));

    return SCD_RESULT__OK;
}


/****************************************************************************/
