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
 ************************************************************/

/****************************************************************************/

#include "btfe_scd_priv.h"
#include "btfe_scd_int_priv.h"
/* #include "scd_oslib.h" */
#include "btfe_scd_35233_priv.h"
#include "btfe_scd_version_priv.h"
#include "bstd.h"
#include "bkni.h"
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

#include "bchp_vafe.h"

#include "btfe_scd_reg35230_hi_priv.h"
#include "math.h"

#include "btfe_scd_common_func_priv.h"
#include "btfe_scd_uagc_generic_priv.h"
 
BDBG_MODULE(btfe_scd_35233);


#define MAJOR  1
#define MINOR  5

#define FW_SIZE	60	/* in terms of kb */

#define SCD_LEGACY_QAM

/****************************************************************************/
/* build options */
/****************************************************************************/

/*
#define PALLADIUM*/
#define DISABLE_CRC_CHECK
/*#define DEBUG_SERVICES
#define VERIFY_FW_DOWNLOAD
*/
/****************************************************************************/
/* globals */
/****************************************************************************/
/* constants */
/****************************************************************************/

#define SCD_FE_POWER_DOWN_ALL           (0)         /* power down all cores */
#define SCD_FE_POWER_UP_VSB             (1 << 0)    /* power up FAT VSB */
#define SCD_FE_POWER_UP_QAM64           (1 << 1)    /* power up FAT QAM64 */
#define SCD_FE_POWER_UP_QAM256          (1 << 2)    /* power up FAT QAM256 */
#define SCD_FE_POWER_UP_COFDM           (1 << 3)    /* power up FAT COFDM */

#define SCD_FE_POWER_UP_BERT            (1 << 5)    /* power up BERT */
#define SCD_FE_POWER_UP_ANALOG          (1 << 6)    /* power up Analog (NTSC/PAL/SECAM) */
#define SCD_FE_POWER_UP_FAT             (SCD_FE_POWER_UP_VSB | SCD_FE_POWER_UP_QAM64 | SCD_FE_POWER_UP_QAM256 | SCD_FE_POWER_UP_ANALOG | SCD_FE_POWER_UP_COFDM)

#define SCD_FE_POWER_UP_ALL             (SCD_FE_POWER_UP_FAT | SCD_FE_POWER_UP_BERT)

#define FE_CRYSTAL_FREQ                 (54000*KHZ)
#define FE_CRYSTAL_FREQ_KHZ             (FE_CRYSTAL_FREQ/KHZ)

#define VSB_DEFAULT_IF_FREQUENCY        (44000000)
#define QAM_DEFAULT_IF_FREQUENCY        (44000000)
#define NTSC_DEFAULT_IF_FREQUENCY       (44000000)
#define PAL_DEFAULT_IF_FREQUENCY        (36150000)
#define SECAM_DEFAULT_IF_FREQUENCY      (36150000)
#define COFDM_DEFAULT_IF_FREQUENCY      (36166666)

#define LOW_IF_MAX_FREQ			(27000000)

#define FAT_SAMPLE_FREQ                 (FE_CRYSTAL_FREQ)
#define FAT_SAMPLE_FREQ_KHZ             (FAT_SAMPLE_FREQ/KHZ)

#define FAT_IF_TARGET_FREQ(if_freq)     (FAT_SAMPLE_FREQ - (if_freq))

#define VSB_SYMBOL_RATE                 (10762237)  /*(1000000*684*9/2/286) */
#define QAM64_SYMBOL_RATE               (5056941)
#define QAM256_SYMBOL_RATE              (5360537)

#define COFDM_CRYSTAL_FREQ              (FE_CRYSTAL_FREQ/4)
#define COFDM_CRYSTAL_FREQ_KHZ          (COFDM_CRYSTAL_FREQ/KHZ)

#define COFDM_FFT_SAMPLING_FREQ         (9142800)
#define COFDM_FFT_SAMPLING_FREQ_KHZ     (9143)

#ifdef PALLADIUM
 #define TIMEOUT_SCALE                  (100)
#else
 #define TIMEOUT_SCALE                  (1)
#endif

#define DEFAULT_TIMEOUT                 (500)

#define CRC_TIMEOUT                     (2*DEFAULT_TIMEOUT*TIMEOUT_SCALE)
#define CHAN_SCAN_TIMEOUT               (2*DEFAULT_TIMEOUT*TIMEOUT_SCALE)
#define PSD_TIMEOUT                     (4*DEFAULT_TIMEOUT*TIMEOUT_SCALE)
#define RS_TIMEOUT                      (DEFAULT_TIMEOUT*TIMEOUT_SCALE)
#define ATRACKING_TIMEOUT               (DEFAULT_TIMEOUT*TIMEOUT_SCALE)
#define GPIO_TIMEOUT                    (DEFAULT_TIMEOUT*TIMEOUT_SCALE)
#define SNR_TIMEOUT                    (DEFAULT_TIMEOUT*TIMEOUT_SCALE)

#define NFRAMES_TIMEOUT                 (DEFAULT_TIMEOUT*TIMEOUT_SCALE)
#define ACB_TIMEOUT                     (DEFAULT_TIMEOUT*TIMEOUT_SCALE)
#define TAPS_TIMEOUT                    (4*DEFAULT_TIMEOUT*TIMEOUT_SCALE)
#define I2C_TIMEOUT                     (DEFAULT_TIMEOUT*TIMEOUT_SCALE)

#define GPIO_MON_TIMEOUT                1

#ifdef PALLADIUM
 #define CLEAR_TIMEOUT                  (20)
#else
 #define CLEAR_TIMEOUT                  (20)
#endif

#define FAT_AGC_DELAY                   (200)

#define NO_ARGS                         (0)
#define NO_RESULTS                      (0)
#define NO_TIMEOUT                      (0)

#define TAPSECTIONSIZE                  (320)     /* size of each partial Equalizer tap section */

#define MPEG_DFMT_POL_3_0__INV_CLK      (1 << 3)
#define MPEG_DFMT_POL_3_0__INV_ERROR    (1 << 2)
#define MPEG_DFMT_POL_3_0__INV_SYNC     (1 << 1)
#define MPEG_DFMT_POL_3_0__INV_DATA     (1 << 0)

#define MPEG_DFMT_1_0__BIT              (1 << 1)
#define MPEG_DFMT_1_0__GATE             (1 << 0)

#define EQ_DFS_STATE__LOCK              (1 << 2)
#define EQ_DFS_STATE__ACQ               (1 << 0)

#define QAM_DET_STATE__LOCK             (1 << 1)
#define QAM_DET_STATE__VERIFY           (1 << 0)

#define MISC_TEST_MUX_SEL__FE_TMUX      (0x01)
#define MISC_TEST_MUX_SEL__EQ_TMUX      (0x02)
#define MISC_TEST_MUX_SEL__FEC_TMUX     (0x03)

#define MISC_TEST_MUX_SEL__AGC_TMUX     (0x07)
#define MISC_TEST_MUX_SEL__DW_TMUX      (0x08)

#define EQ_TESTMUX_MODE__SYMBOL_ESTIMATOR_OUTPUT  (0x04)

/* OFS_NOMINAL_NCO_2 */
#define SMOOTHER_NCO2_MAX               (0x800000)
#define SMOOTHER_NCO2_256QAM_SYNC_DIS   (0x5B5656)
#define SMOOTHER_NCO2_256QAM_SYNC_EN    (0x5BD360)
#define SMOOTHER_NCO2_256QAM_GAP        (0x5EC19C)
#define SMOOTHER_NCO2_64QAM_SYNC_DIS    (0x3F78DD)
#define SMOOTHER_NCO2_64QAM_SYNC_EN     (0x3FCFC2)
#define SMOOTHER_NCO2_64QAM_GAP         (0x5EB2F5)
#define SMOOTHER_NCO2_8VSB_SYNC_DIS     (0x2DA385)
#define SMOOTHER_NCO2_8VSB_SYNC_EN      (0x2DE200)
#define SMOOTHER_NCO2_8VSB_GAP          (0x32C393)

/* OFS_TARGET */
#define QAM_GAP_TARGET                  (0x90)

/* OFS_BANDWIDTH */
#define SMOOTHER_ZETA                   (0x10)  /* corresponds to z=0.75 */
#define SMOOTHER_BW_VSB                 (0x00)  /* corresponds to n=17, w_n=1.50 Hz */
#define SMOOTHER_BW_QAM64               (0x04)  /* corresponds to n=14, w_n= */
#define SMOOTHER_BW_QAM256              (0x06)

/* OFS_INSERT_COUNT */
#define VSB6_INSERT_COUNT               (0x14)
#define QAM64_INSERT_COUNT              (0x5B)
#define QAM256_INSERT_COUNT             (0x06)

/* OFS_INSERT_CONTROL */
#define GAP_INSERT_CONTROL              (0x01)

/* OFS_START_TARGET */
#define SMOOTHER_BUF_FILL_LEVEL_VSB     (0x90*2)    /* bytes - 0 to 510, in multiples of 2 */
#define SMOOTHER_BUF_FILL_LEVEL_QAM     (0x70*2)    /* bytes - 0 to 510, in multiples of 2 */

/*#define CABLE_CARD_SELECT               (0x10)*/

#define SIZE_ACB                        (8)

#define DEMOD_STATUS_COUNTER            (33)

#define NORMINAL_50KHZ (0x3CAE) /* (50K/54M) * 2**24, sampling rate is 54MHz */

#define RS_UNCORRECT_ERROR_LAYER_A		6
#define RS_CORRECT_ERROR_LAYER_A		18
#define RS_TOTAL_PACKETS_LAYER_A		30


/****************************************************************************/
/* macros */
/****************************************************************************/

#define GET_CHIP(chip_handle)  &g_chip[((SCD_CHIP *) (chip_handle))->chip_instance]

#define VAL2FLD(reg, field, val)        (((uint8_t)(val) << reg##__##field##__SHIFT) & reg##__##field##__MASK)
#define FLD2VAL(reg, field, val)        ((uint8_t)(((val) & reg##__##field##__MASK) >> reg##__##field##__SHIFT))

#define MODIFYFLD(var, reg, field, val)   (var = (uint8_t )( ((var) & (~(reg##__##field##__MASK <<  0))) | (((uint8_t )(val) << (reg##__##field##__SHIFT +  0)) & (reg##__##field##__MASK <<  0))) )
#define APER1_MODIFYFLD32(var, mask, shift, val) (var = (uint32_t)( ((var) & (~(mask << 0))) | (((uint32_t)(val) << (shift + 0)) & (mask << 0))) )

extern void BTFE_MultU32U32(uint32_t A, uint32_t B, uint32_t *P_hi, uint32_t *P_lo);
extern void BTFE_DivU64U32(uint32_t A_hi, uint32_t A_lo, uint32_t B, uint32_t *Q_hi, uint32_t *Q_lo);

/****************************************************************************/
/* private prototypes */
/****************************************************************************/

static SCD_RESULT BTFE_P_ChipDoMicroService(SCD_RESULT ret_val, SCD_HANDLE chip_handle, uint8_t commandType, uint32_t arg_byte_count, uint8_t *args, uint32_t result_byte_count, uint8_t *results, uint32_t timeout);
static SCD_RESULT BTFE_P_ChipWrite(SCD_HANDLE chip_handle, uint32_t aper, uint32_t offset, uint32_t length, uint8_t *values);
static SCD_RESULT BTFE_P_ChipRead(SCD_HANDLE chip_handle, uint32_t aper, uint32_t offset, uint32_t length, uint8_t *values);
static SCD_RESULT BTFE_P_ChipCheckMicrocode(SCD_RESULT ret_val, SCD_HANDLE chip_handle, uint8_t *pMicroCode);
/****************************************************************************/
/* private data */
/****************************************************************************/

static CHIP g_chip[SCD_MAX_CHIP];
uint8_t videoSubmodeMask = 0; 

static SCD_RESULT BTFE_P_ChipWrite(SCD_HANDLE chip_handle, uint32_t aper, uint32_t offset, uint32_t length, uint8_t *values)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;

    CHIP_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ChipWrite(chip_handle=%08X, aper=%u, offset=%08X, length=%u, &values=%08X)", chip_handle, aper, offset, length, values)));

    {
        uint32_t instance;

        if((ret_val = BTFE_P_ScdGetInstance(chip_handle, &instance)) == SCD_RESULT__OK)
        {
            ret_val = BTFE_P_HalWriteChip(instance, aper, offset, length, values); 
        }
    }

    return ret_val;
}


/****************************************************************************/
/* private functions */
/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetRegs(SCD_RESULT ret_val, SCD_HANDLE handle, uint32_t offset, uint32_t size, uint8_t *values)
{
    uint32_t instance;

    if(ret_val == SCD_RESULT__OK)
    {
        if((ret_val = BTFE_P_ScdGetInstance(handle, &instance)) == SCD_RESULT__OK)
        {
            ret_val = BTFE_P_HalWriteChip(instance, 0, offset, size, values);
        }
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipGetRegs(SCD_RESULT ret_val, SCD_HANDLE handle, uint32_t offset, uint32_t size, uint8_t *values)
{
    uint32_t instance;

    if(ret_val == SCD_RESULT__OK)
    {
        if((ret_val = BTFE_P_ScdGetInstance(handle, &instance)) == SCD_RESULT__OK)
        {
            ret_val = BTFE_P_HalReadChip(instance, 0, offset, size, values);
        }
    }

    return ret_val;
}

/****************************************************************************/
SCD_RESULT BTFE_P_ChipSetReg16(SCD_RESULT ret_val, SCD_HANDLE handle, uint32_t offset, uint16_t value)
{
    uint8_t temp[2];

    if(ret_val == SCD_RESULT__OK)
    {
        temp[1] = (uint8_t) ((value & 0x00ff) >> 0);
        temp[0] = (uint8_t) ((value & 0xff00) >> 8);

        ret_val =  BTFE_P_ChipWrite(handle, 0, offset, 2, temp);
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetReg24(SCD_RESULT ret_val, SCD_HANDLE handle, uint32_t offset, uint32_t value)
{
    uint8_t temp[3];

    if(ret_val == SCD_RESULT__OK)
    {
        temp[2] = (uint8_t) ((value & 0x000000ff) >> 0);
        temp[1] = (uint8_t) ((value & 0x0000ff00) >> 8);
        temp[0] = (uint8_t) ((value & 0x00ff0000) >> 16);

        ret_val =  BTFE_P_ChipWrite(handle, 0, offset, 3, temp);
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetReg8(SCD_RESULT ret_val, SCD_HANDLE handle, uint32_t offset, uint8_t value)
{
	(void)ret_val;
    return  BTFE_P_ChipWrite(handle, 0, offset, 1, &value);
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipGetReg32(SCD_RESULT ret_val, SCD_HANDLE handle, uint32_t offset, uint32_t *value)
{
    uint8_t temp[4];

	if((ret_val = BTFE_P_ChipRead(handle, 0, offset, 4, temp)) == SCD_RESULT__OK)
        *value = temp[3] + ((uint32_t) temp[2] << 8) + ((uint32_t) temp[1] << 16) + ((uint32_t) temp[0] << 24);
    else
        *value = 0;
    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipGetReg24(SCD_RESULT ret_val, SCD_HANDLE handle, uint32_t offset, uint32_t *value)
{
    uint8_t temp[3];


	if((ret_val = BTFE_P_ChipRead(handle, 0, offset, 3, temp)) == SCD_RESULT__OK)
        *value = temp[2] + ((uint32_t) temp[1] << 8) + ((uint32_t) temp[0] << 16);
    else
        *value = 0;

	return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipGetReg16(SCD_RESULT ret_val, SCD_HANDLE handle, uint32_t offset, uint16_t *value)
{
    uint8_t temp[2];

	if((ret_val = BTFE_P_ChipRead(handle, 0, offset, 2, temp)) == SCD_RESULT__OK)
        *value = (uint16_t) (temp[1] + ((uint16_t) temp[0] << 8));
    else
        *value = 0;

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipGetReg16_LE(SCD_RESULT ret_val, SCD_HANDLE handle, uint32_t offset, uint16_t *value)
{
    uint8_t temp[2];

	if((ret_val = BTFE_P_ChipRead(handle, 0, offset, 2, temp)) == SCD_RESULT__OK)
        *value = (uint16_t) (temp[0] + ((uint16_t) temp[1] << 8));
    else
        *value = 0;

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipGetReg8(SCD_RESULT ret_val, SCD_HANDLE handle, uint32_t offset, uint8_t *value)
{
    return BTFE_P_ChipGetRegs(ret_val, handle, offset, 1, value);
}

#if 0
/****************************************************************************/

static SCD_RESULT BTFE_P_aper1_ChipGetReg32(SCD_RESULT ret_val, SCD_HANDLE handle, uint32_t offset, uint32_t *value)
{
   uint8_t temp[4];

   if((ret_val = BTFE_P_ChipRead(handle, 1, offset, 4, temp)) == SCD_RESULT__OK)
      *value = temp[3] + ((uint32_t) temp[2] << 8) + ((uint32_t) temp[1] << 16) + ((uint32_t) temp[0] << 24);
   else
      *value = 0;

   return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_aper1_ChipSetReg32(SCD_RESULT ret_val, SCD_HANDLE handle, uint32_t offset, uint32_t value)
{
   uint8_t temp[4];

   if(ret_val == SCD_RESULT__OK)
   {
      temp[0] = (uint8_t) ((value & 0xff000000) >> 24);
      temp[1] = (uint8_t) ((value & 0x00ff0000) >> 16);
      temp[2] = (uint8_t) ((value & 0x0000ff00) >> 8);
      temp[3] = (uint8_t) ((value & 0x000000ff) >> 0);

      ret_val =  BTFE_P_ChipWrite(handle, 1, offset, 4, temp);
   }

   return ret_val;
}
#endif

/****************************************************************************/

#if 0
/* write to non dfe register */
static SCD_RESULT BTFE_P_ChipWriteReg(SCD_RESULT ret_val, SCD_HANDLE handle, uint32_t offset, uint32_t value)
{
    uint8_t temp[4];

    if(ret_val == SCD_RESULT__OK)
    {
        temp[3] = (uint8_t) ((value & 0x000000ff) >> 0);
        temp[2] = (uint8_t) ((value & 0x0000ff00) >> 8);
        temp[1] = (uint8_t) ((value & 0x00ff0000) >> 16);
        temp[0] = (uint8_t) ((value & 0xff000000) >> 24);

        ret_val =  BTFE_P_ChipWrite(handle, 1, offset, 4, temp);
    }

    return ret_val;
}
#endif

/****************************************************************************/

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipDoPllSetup(SCD_RESULT ret_val, SCD_HANDLE chip_handle)
{
    (void) chip_handle;

    return ret_val;
}


/****************************************************************************/

static SCD_RESULT BTFE_P_ChipDoAdc0Setup(SCD_RESULT ret_val, SCD_HANDLE chip_handle)
{
	uint8_t temp8;
	int temp = 0;

   if(ret_val == SCD_RESULT__OK)
   {
	   /* Enable the ADC FIFO */
	   temp |= (int)BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, &temp8);
	   MODIFYFLD(temp8, MISC_MODULATION_FMT_RESET_CONTROL_2, soft_reset_adc0_fifo, 1);
	   temp |= (int)BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, temp8);
	   MODIFYFLD(temp8, MISC_MODULATION_FMT_RESET_CONTROL_2, soft_reset_adc0_fifo, 0);
	   temp |= (int)BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, temp8);

	   ret_val = (SCD_RESULT)temp;
	   if(ret_val != SCD_RESULT__OK)
      {
         BDBG_ERR(("BTFE_P_ChipDoAdc0Setup: error=%u", ret_val));
      }
   }

   return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipDoAdc1Setup(SCD_RESULT ret_val, SCD_HANDLE chip_handle)
{
	uint8_t temp8;
	int temp = 0;
	
   if(ret_val == SCD_RESULT__OK)
   {
        /* Enable the ADC FIFO */
        temp |= BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, &temp8);
        MODIFYFLD(temp8, MISC_MODULATION_FMT_RESET_CONTROL_2, OFS_burst_rate, 1);
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, temp8);
        MODIFYFLD(temp8, MISC_MODULATION_FMT_RESET_CONTROL_2, OFS_burst_rate, 0);
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, temp8);

      ret_val = (SCD_RESULT)temp;

      if(ret_val != SCD_RESULT__OK)
      {
         BDBG_ERR(("BTFE_P_ChipDoAdc1Setup: error=%u", ret_val));
      }
   }

    return ret_val;
}

/****************************************************************************/

static uint32_t BTFE_P_ChipGetServiceStatusReg(uint8_t commandType)
{
    switch(commandType)
    {
    case SVC_GET_VSB_TAPS:  return ixHI_SERVICE_STATUS_2;

    case SVC_GET_PWR_SPECT:
    case SVC_GET_QAM_TAPS:  return ixHI_SERVICE_STATUS_1;

    default:                return ixHI_SERVICE_STATUS_0;
    }
}

/****************************************************************************/

#ifdef DEBUG_SERVICES
static char *BTFE_P_ChipGetCommandName(uint8_t commandType)
{
    switch(commandType)
    {
    case SVC_CSECT_WRITE:        return "SVC_CSECT_WRITE";
    case SVC_CSECT_READ:         return "SVC_CSECT_READ";
    case SVC_I2C_WRITE:          return "SVC_I2C_WRITE";
    case SVC_I2C_READ:           return "SVC_I2C_READ";
    case SVC_GET_ACQ_STATUS:     return "SVC_GET_ACQ_STATUS";
    case SVC_GET_DNLD_STATUS:    return "SVC_GET_DNLD_STATUS";
    case SVC_GET_RS_ERRORS:      return "SVC_GET_RS_ERRORS";
    case SVC_CONFIG_KEYPAD:      return "SVC_CONFIG_KEYPAD";
    case SVC_GET_KEYPAD:         return "SVC_GET_KEYPAD";
    case SVC_SET_INT_OUTPUT:     return "SVC_SET_INT_OUTPUT";
    case SVC_JUMP_TO_MONITOR:    return "SVC_JUMP_TO_MONITOR";
    case SVC_SCI_READ:           return "SVC_SCI_READ";
    case SVC_SCI_WRITE:          return "SVC_SCI_WRITE";
    case SVC_GPIO_CONFIG:        return "SVC_GPIO_CONFIG";
    case SVC_GPIO_READ:          return "SVC_GPIO_READ";
    case SVC_GPIO_WRITE:         return "SVC_GPIO_WRITE";
    case SVC_SET_NFRAMES:        return "SVC_SET_NFRAMES";
    case SVC_SET_AUDIO_CONFIG:   return "SVC_SET_AUDIO_CONFIG";
    case SVC_ATRACKING_CONFIG:   return "SVC_ATRACKING_CONFIG";
    case SVC_ATRACKING_RUN:      return "SVC_ATRACKING_RUN";
    case SVC_NXTENNA_RW:         return "SVC_NXTENNA_RW";
    case SVC_NXTENNA_TRACK:      return "SVC_NXTENNA_TRACK";
    case SVC_GPIO_STATUS_CONFIG: return "SVC_GPIO_STATUS_CONFIG";
    case SVC_GET_CHANNEL_EST:    return "SVC_GET_CHANNEL_EST";
    case SVC_GET_PWR_SPECT:      return "SVC_GET_PWR_SPECT";
    case SVC_GET_QAM_TAPS:       return "SVC_GET_QAM_TAPS";
    case SVC_GET_VSB_TAPS:       return "SVC_GET_VSB_TAPS";
    }

    return "<unknown>";
}
#endif

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipDoMicroServiceSetup(SCD_RESULT ret_val, SCD_HANDLE chip_handle, uint8_t commandType, uint32_t bytecount, uint8_t *datatosend)
{
    uint8_t data=0;

    /* check args */
    if(bytecount > SERVICE_PARAMS_LENGTH)
    {
        BDBG_ERR(("BTFE_P_ChipDoMicroServiceSetup: too many args"));
        ret_val = SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(ret_val == SCD_RESULT__OK)
    {
        /* check if busy */
        ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixHI_SERVICE_REQUEST, &data);

        if(ret_val == SCD_RESULT__OK)
        {
            if((data & SERVICE_REQUEST) != 0x00)
            {
                BDBG_WRN(("BTFE_P_ChipDoMicroServiceSetup: service %d still in progress", data & ~SERVICE_REQUEST));

                ret_val = SCD_RESULT__COMMAND_IN_PROGRESS;
            }
        }

        /* write args */
        ret_val = BTFE_P_ChipSetRegs(ret_val, chip_handle, ixHI_SERVICE_PARAMS_0, bytecount, datatosend);

        /* write command */
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_SERVICE_REQUEST, (uint8_t) (commandType | SERVICE_REQUEST));
        /* if(ret_val != SCD_RESULT__OK) { BDBG_ERR(("BTFE_P_ChipDoMicroServiceSetup: error=%u", ret_val)); } */
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipDoMicroServiceCheckWait(SCD_RESULT ret_val, SCD_HANDLE chip_handle, uint8_t commandType, uint8_t *status, uint32_t timeout)
{
    uint32_t tries=0;
    uint32_t done=0;

    if(ret_val == SCD_RESULT__OK)
    {
        do
        {
            tries += 1;
            done = 0;

            if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, BTFE_P_ChipGetServiceStatusReg(commandType), status)) == SCD_RESULT__OK)
            {
                if((*status & SERVICE_COMPLETE) == SERVICE_COMPLETE)
                {
                    done = 1;

                    if((*status & BUSY_ERROR) == BUSY_ERROR)
                    {
                        BDBG_ERR(("BTFE_P_ChipDoMicroService: service %u returned error", commandType));
                        ret_val = SCD_RESULT__CHIP_ERROR;
                    }
                }
                else if(tries > 10)
                {
                    BKNI_Sleep(1);
                }
            }
            else
            {
                return SCD_RESULT__ERROR;
            }
        }
        while(!done && (tries < timeout));

        if(!done)
        {
            BDBG_ERR(("BTFE_P_ChipDoMicroServiceCheckWait: service %u did not complete (status=0x%02X)", commandType, *status));
            ret_val = SCD_RESULT__ERROR;
        }
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipDoMicroServiceGetResults(SCD_RESULT ret_val, SCD_HANDLE chip_handle, uint32_t bytecount, uint8_t *results)
{
    if(ret_val == SCD_RESULT__OK)
    {
        if(bytecount > HI_SVC_RES_0_LENGTH)
        {
            BDBG_ERR(("BTFE_P_ChipDoMicroServiceGetResults: result length too long"));

            ret_val = SCD_RESULT__ARG_OUT_OF_RANGE;
        }

        ret_val = BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_SERVICE_RESULTS_0, bytecount, results);

        /* if(ret_val != SCD_RESULT__OK) { BDBG_ERR(("BTFE_P_ChipDoMicroServiceGetResults: error=%u", ret_val)); } */
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipDoMicroServiceClear(SCD_RESULT ret_val, SCD_HANDLE chip_handle, uint8_t commandType)
{
    uint8_t data;
    SCD_RESULT ret_val_2 = SCD_RESULT__OK;
    int32_t timeout;

    if(ret_val == SCD_RESULT__OK)
    {
        /* clear the status */
        ret_val_2 = BTFE_P_ChipSetReg8(ret_val_2, chip_handle, BTFE_P_ChipGetServiceStatusReg(commandType), 0);

        /* wait for service request to clear */
        for(timeout=CLEAR_TIMEOUT; timeout; timeout--)
        {
            ret_val_2 = BTFE_P_ChipGetReg8(ret_val_2, chip_handle, ixHI_SERVICE_REQUEST, &data);

            if((data & SERVICE_REQUEST) == 0x00) break;

            BKNI_Sleep(1); 
        }

        if(timeout == 0)
        {
            BDBG_WRN(("BTFE_P_ChipDoMicroServiceClear: timeout waiting for service %u to clear", commandType));
            ret_val_2 = SCD_RESULT__COMMAND_IN_PROGRESS;
        }

        if(ret_val_2 != SCD_RESULT__OK)
        {
            BDBG_ERR(("BTFE_P_ChipDoMicroServiceClear: error=%u", ret_val));
            ret_val = ret_val_2;
        }
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipDoMicroServiceStart(SCD_RESULT ret_val, SCD_HANDLE chip_handle, uint8_t commandType, uint32_t arg_byte_count, uint8_t *args, uint32_t timeout)
{
    uint8_t status;

    if(ret_val == SCD_RESULT__OK)
    {
        /* send command */
        ret_val = BTFE_P_ChipDoMicroServiceSetup(ret_val, chip_handle, commandType, arg_byte_count, args);
        ret_val = BTFE_P_ChipDoMicroServiceCheckWait(ret_val, chip_handle, commandType, &status, timeout);

        /* if(ret_val != SCD_RESULT__OK) BDBG_ERR(("BTFE_P_ChipDoMicroServiceStart: error=%u", ret_val)); */
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipDoMicroService(SCD_RESULT ret_val, SCD_HANDLE chip_handle, uint8_t commandType, uint32_t arg_byte_count, uint8_t *args, uint32_t result_byte_count, uint8_t *results, uint32_t timeout)
{
    SCD_RESULT ret_val_2;

    if(ret_val == SCD_RESULT__OK)
    {
        if((ret_val = BTFE_P_ChipDoMicroServiceStart(ret_val, chip_handle, commandType, arg_byte_count, args, timeout)) == SCD_RESULT__OK)
        {
            /* if results arg is not zero */
            if(results && result_byte_count)
            {
                /* get results */
                ret_val = BTFE_P_ChipDoMicroServiceGetResults(ret_val, chip_handle, result_byte_count, results);
            }
        }

        /* cleanup */
        if((ret_val_2 = BTFE_P_ChipDoMicroServiceClear(SCD_RESULT__OK, chip_handle, commandType)) != SCD_RESULT__OK)
        {
            ret_val = ret_val_2;
        }

        /* if(ret_val != SCD_RESULT__OK) BDBG_ERR(("BTFE_P_ChipDoMicroService: error=%u", ret_val)); */
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipLoadMicrocode(SCD_RESULT ret_val, SCD_HANDLE chip_handle, uint8_t *pMicroCode)
{
    CHIP *tchip = GET_CHIP(chip_handle);   
    uint8_t buffer[8];
    uint32_t temp32;
    uint32_t MicroCodeLen = 0;
    uint32_t crcValue = 0;
    uint8_t status;
    uint8_t *p;
    uint8_t temp8;
    uint8_t svc;
    int i;
#ifdef VERIFY_FW_DOWNLOAD
    uint8_t *pCurrByte, readData;
    bool bSuccess;
#endif
  
    if(ret_val == SCD_RESULT__OK)
    {
        /*BDBG_MSG(("downloading firmware..."));*/

        p = pMicroCode;

        /* Ensure the micro is in the reset state */
        ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixUC_MEM_AGC_DWNLD_UPLD_CNTRL, &temp8);
        MODIFYFLD(temp8, UC_MEM_AGC_DWNLD_UPLD_CNTRL, uc_reset, 1);
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixUC_MEM_AGC_DWNLD_UPLD_CNTRL, temp8);

        /* Verify the header information, and gather the length */
        if(ret_val == SCD_RESULT__OK)
        {
            temp32 = (*p<<24) + (*(p+1)<<16) + (*(p+2)<<8) + (*(p+3));

            if(temp32 != 0x58323333) /* 'X233' */
            {
                BDBG_WRN(("BTFE_P_ChipLoadMicrocode: bad header 1"));
                ret_val = SCD_RESULT__BAD_FIRMWARE;
            }
        }

        if(ret_val == SCD_RESULT__OK)
        {
            p += 4;
            temp32 = (*p<<24) + (*(p+1)<<16) + (*(p+2)<<8) + (*(p+3));
/* ' ATI' or ' AMD' or " BCM"*/
            if((temp32 != 0x20415449) && (temp32 != 0x20414D44) && (temp32 != 0x2042434D)) 
            {
                BDBG_WRN(("BTFE_P_ChipLoadMicrocode: bad header 2"));
                ret_val =  SCD_RESULT__BAD_FIRMWARE;
            }
        }

        if(ret_val == SCD_RESULT__OK)
        {
            p += 12; /* Now at the number of Bytes in segment 1 header */
            temp32 = (*p<<24) + (*(p+1)<<16) + (*(p+2)<<8) + (*(p+3));

            if(temp32 >= (FW_SIZE*1024)) /* just make sure the value is reasonable */
            {
                BDBG_WRN(("BTFE_P_ChipLoadMicrocode: bad segment 1 size"));
                ret_val =  SCD_RESULT__BAD_FIRMWARE;
            }

            MicroCodeLen = temp32;
        }

        if(ret_val == SCD_RESULT__OK)
        {
            p += 8;
            temp32 = (*p<<24) + (*(p+1)<<16) + (*(p+2)<<8) + (*(p+3));
            crcValue = temp32;
            p += 4;
            tchip->FwCRCvalue = crcValue;
        }

        /* dummy up micro services, so it will look ready */
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_SERVICE_REQUEST, 0x00);
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_SERVICE_STATUS_0, 0x00);
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_SERVICE_STATUS_1, 0x00);
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_SERVICE_STATUS_2, 0x00);

        /* Enable RAM Access */
        temp8 = 0;
        MODIFYFLD(temp8, MISC_UC_FEC_PSD_AGC_RAM_I2C_CNTRL, i2c_ram_access_enable, 1);
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_UC_FEC_PSD_AGC_RAM_I2C_CNTRL, temp8);

        /* Prep other registers for initial running of microprocessor */
        for(i=0;i<7;i++) buffer[i] = 0;
        ret_val = BTFE_P_ChipSetRegs(ret_val, chip_handle, ixHI_VIDEO_PAUSE, 6, buffer);
        ret_val = BTFE_P_ChipSetRegs(ret_val, chip_handle, ixHI_VIDEO_MODE_SELECT, 7, buffer);

        ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixAPERTURE_SEL, &temp8);
        temp8 = 1;
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixAPERTURE_SEL, temp8);
        
        /* The core should now be powered up and ready to accept a download */
        BDBG_MSG(("BTFE_P_ChipLoadMicrocode(): downloading microcode..."));
#ifndef VERIFY_FW_DOWNLOAD
        ret_val = BTFE_P_ChipSetRegs(ret_val, chip_handle, 0, MicroCodeLen, p);
#else		
        pCurrByte = p;
        for (i = 0; i < MicroCodeLen; i++)
		{
		   bSuccess = false;
		   while (!bSuccess)
		   {
	           BTFE_P_ChipSetRegs(ret_val, chip_handle, i, 1, pCurrByte);
			   BTFE_P_ChipGetRegs(ret_val, chip_handle, i, 1, &readData);	
	           if (readData != *pCurrByte)
	           {
	              BDBG_WRN(("BTFE_P_ChipLoadMicrocode: error in address 0x%04X, read=0x%02X, expected=0x%02X",
				              i, readData, *pCurrByte));							  
	           }
			   else
			      bSuccess = true;
			}
		    pCurrByte++;
		}
#endif

        ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixAPERTURE_SEL, &temp8);
        temp8 = 0;
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixAPERTURE_SEL, temp8);
        
        BDBG_MSG(("BTFE_P_ChipLoadMicrocode: starting microcode..."));
		/* startup the micro */
        ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixUC_MEM_AGC_DWNLD_UPLD_CNTRL, &temp8);
        MODIFYFLD(temp8, UC_MEM_AGC_DWNLD_UPLD_CNTRL, uc_reset, 0);
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixUC_MEM_AGC_DWNLD_UPLD_CNTRL, temp8);

#ifdef DISABLE_CRC_CHECK
        svc = 60;
#else
        svc = SVC_GET_DNLD_STATUS;
#endif
        /* write the length, and set up the micro, so the first command is the CRC check */
        buffer[0] = (uint8_t) ((MicroCodeLen & 0xff00) >> 8);   /* Param 0 */
        buffer[1] = (uint8_t) (MicroCodeLen & 0xff);            /* Param 1 */
        buffer[2] = (uint8_t) ((crcValue & 0xff00) >> 8);       /* Param 2 */
        buffer[3] = (uint8_t) (crcValue & 0xff) ;               /* Param 3 */
        ret_val = BTFE_P_ChipDoMicroServiceSetup(ret_val, chip_handle, svc, 4, buffer);

#ifdef DISABLE_CRC_CHECK
        BTFE_P_ChipDoMicroServiceCheckWait(ret_val, chip_handle, svc, &status, CRC_TIMEOUT);
#else
        /* time out to allow the micro to startup and write the values */
        ret_val = BTFE_P_ChipDoMicroServiceCheckWait(ret_val, chip_handle, svc, &status, CRC_TIMEOUT);
#endif

        /* clear the status */
        if(BTFE_P_ChipDoMicroServiceClear(SCD_RESULT__OK, chip_handle, svc) != SCD_RESULT__OK)
        {
            BDBG_ERR(("BTFE_P_ChipLoadMicrocode: service %d clear returned error", svc));
        }

        if(ret_val != SCD_RESULT__OK)
        {
           BDBG_ERR(("BTFE_P_ChipLoadMicrocode: error=%u", ret_val));
        }
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipPower(SCD_RESULT ret_val, SCD_HANDLE chip_handle, uint32_t vector)
{
    uint8_t   temp8;

    if(ret_val == SCD_RESULT__OK)
    {
        /* MISC_BIST_CONTROL_0 */
/*        ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_BIST_CONTROL_0, &temp8);

        MODIFYFLD(temp8, MISC_BIST_CONTROL_0, MBIST_RST_MEM, 0);
        MODIFYFLD(temp8, MISC_BIST_CONTROL_0, MBIST_SETUP, 0);
        MODIFYFLD(temp8, MISC_BIST_CONTROL_0, dw_8051_fec_ofs_mbist_enable, 0);

        MODIFYFLD(temp8, MISC_BIST_CONTROL_0, mbist_mode_en, 0);
        MODIFYFLD(temp8, MISC_BIST_CONTROL_0, mbist_debug_output_en, 1);
        MODIFYFLD(temp8, MISC_BIST_CONTROL_0, mbist_clk_pwr_dwn, 1);
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_BIST_CONTROL_0, temp8);
*/
        /* MISC_POWER_CONTROL_2 */
        temp8 = 0;
        MODIFYFLD(temp8, MISC_POWER_CONTROL_2, fec_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_2, ofs_pwr_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_2, dvbt_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_2, ntsc_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_2, bert_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_2, psd_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_2, fiji_power_down, 1);

        if(vector & (SCD_FE_POWER_UP_VSB | SCD_FE_POWER_UP_QAM64 | SCD_FE_POWER_UP_QAM256))
        {
            MODIFYFLD(temp8, MISC_POWER_CONTROL_2, fiji_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_2, psd_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_2, fec_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_2, ofs_pwr_down, 0);
        }

        if(vector & (SCD_FE_POWER_UP_ANALOG))
        {
            MODIFYFLD(temp8, MISC_POWER_CONTROL_2, fiji_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_2, ntsc_power_down, 0);
        }

        if(vector & SCD_FE_POWER_UP_COFDM)
        {
            MODIFYFLD(temp8, MISC_POWER_CONTROL_2, dvbt_power_down, 0);
        }

        if(vector & SCD_FE_POWER_UP_BERT)
        {
            MODIFYFLD(temp8, MISC_POWER_CONTROL_2, bert_power_down, 0);
        }

        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_2, temp8);

        /* MISC_POWER_CONTROL_1 */
        temp8 = 0;
        MODIFYFLD(temp8, MISC_POWER_CONTROL_1, fir_0_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_1, fir_1_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_1, fir_2_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_1, fir_3_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_1, fir_4_power_down, 1);
		/* MODIFYFLD(temp8, MISC_POWER_CONTROL_1, agc_power_down, 1); */
        MODIFYFLD(temp8, MISC_POWER_CONTROL_1, front_end_power_down, 1);
/*
        MODIFYFLD(temp8, MISC_POWER_CONTROL_1, serdeser_power_down, 1);
*/

        if(vector & (SCD_FE_POWER_UP_VSB | SCD_FE_POWER_UP_QAM64 | SCD_FE_POWER_UP_QAM256))
        {
            MODIFYFLD(temp8, MISC_POWER_CONTROL_1, fir_0_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_1, fir_1_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_1, fir_2_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_1, fir_3_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_1, fir_4_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_1, agc_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_1, front_end_power_down, 0);
        }

        if(vector & (SCD_FE_POWER_UP_ANALOG))
        {
            MODIFYFLD(temp8, MISC_POWER_CONTROL_1, agc_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_1, front_end_power_down, 0);
        }

        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_1, temp8);

        /* MISC_POWER_CONTROL_0 */
        temp8 = 0;
        MODIFYFLD(temp8, MISC_POWER_CONTROL_0, iir_0_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_0, iir_1_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_0, iir_2_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_0, iir_3_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_0, iir_4_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_0, rest_of_eq_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_0, eq_complete_power_down, 1);

        if(vector & SCD_FE_POWER_UP_VSB)
        {
            MODIFYFLD(temp8, MISC_POWER_CONTROL_0, iir_0_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_0, iir_1_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_0, iir_2_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_0, iir_3_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_0, iir_4_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_0, rest_of_eq_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_0, eq_complete_power_down, 0);
        }

        if(vector & (SCD_FE_POWER_UP_QAM64 | SCD_FE_POWER_UP_QAM256))
        {
            MODIFYFLD(temp8, MISC_POWER_CONTROL_0, iir_0_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_0, iir_3_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_0, rest_of_eq_power_down, 0);
            MODIFYFLD(temp8, MISC_POWER_CONTROL_0, eq_complete_power_down, 0);
        }

        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_0, temp8);

        /* PLLs */
        if(vector)
        {
            /* Enable AGC output pads */
            ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_CORE_PADS_OE_CNTRL, &temp8);
            MODIFYFLD(temp8, MISC_CORE_PADS_OE_CNTRL, pgm_agc1_pads_oen, 0);
            ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_CORE_PADS_OE_CNTRL, temp8);
			
            ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_CORE_PADS_OE_CNTRL, &temp8);
            MODIFYFLD(temp8, MISC_CORE_PADS_OE_CNTRL, pgm_agc2_pads_oen, 0);
            ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_CORE_PADS_OE_CNTRL, temp8);
            if(vector & SCD_FE_POWER_UP_FAT)
            {
                /* Enable the ADC FIFO */
                ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, &temp8);
                MODIFYFLD(temp8, MISC_MODULATION_FMT_RESET_CONTROL_2, soft_reset_adc0_fifo, 0);
                ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, temp8);

                /* Remove resets */
                ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, &temp8);
                MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_ntsc, 0);
                MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_dvbt, 0);
                ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, temp8);
            }
        }
        else
        {
            /* Halt FAT operations */
            temp8 = 0;
            MODIFYFLD(temp8, AGC_CONTROL, go_bit_agc, 0);
            ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixAGC_CONTROL, temp8);

            /* Reset BERT, OFS, FEC, EQ, FE, Analog and AGC */
            ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, &temp8);
            MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_agc, 1);
            MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_fe, 1);
            MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_equalizer, 1);
            MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_fec, 1);
            MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_ofs, 1);
            MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_bert, 1);
            MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_ntsc, 1);
            MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_dvbt, 1);
            ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, temp8);

            /* Reset the ADC FIFO's */
            ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, &temp8);
            MODIFYFLD(temp8, MISC_MODULATION_FMT_RESET_CONTROL_2, soft_reset_adc0_fifo, 1);
            MODIFYFLD(temp8, MISC_MODULATION_FMT_RESET_CONTROL_2, OFS_burst_rate, 1);
            ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, temp8);

            /* Halt FIR */
            ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixEQ_FIR_MODULE_NOOP, &temp8);
            MODIFYFLD(temp8, EQ_FIR_MODULE_NOOP, fir0_noop, 1);
            MODIFYFLD(temp8, EQ_FIR_MODULE_NOOP, fir1_noop, 1);
            MODIFYFLD(temp8, EQ_FIR_MODULE_NOOP, fir2_noop, 1);
            MODIFYFLD(temp8, EQ_FIR_MODULE_NOOP, fir3_noop, 1);
            MODIFYFLD(temp8, EQ_FIR_MODULE_NOOP, fir4_noop, 1);
            ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixEQ_FIR_MODULE_NOOP, temp8);

            /* Halt IIR */
            ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixEQ_IIR_MODULE_NOOP, &temp8);
            MODIFYFLD(temp8, EQ_IIR_MODULE_NOOP, iir0_noop, 1);
            MODIFYFLD(temp8, EQ_IIR_MODULE_NOOP, iir1_noop, 1);
            MODIFYFLD(temp8, EQ_IIR_MODULE_NOOP, iir2_noop, 1);
            MODIFYFLD(temp8, EQ_IIR_MODULE_NOOP, iir3_noop, 1);
            MODIFYFLD(temp8, EQ_IIR_MODULE_NOOP, iir4_noop, 1);
            ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixEQ_IIR_MODULE_NOOP, temp8);

            /* Power down IO's */
            temp8 = 0;
            MODIFYFLD(temp8, MISC_OUTPUT_FMT_CNTRL_1, mpeg_output_enable, 1);
            ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_OUTPUT_FMT_CNTRL_1, temp8);

            ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_CORE_PADS_OE_CNTRL, &temp8);
            MODIFYFLD(temp8, MISC_CORE_PADS_OE_CNTRL, pgm_agc1_pads_oen, 1);
            ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_CORE_PADS_OE_CNTRL, temp8);

            ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_CORE_PADS_OE_CNTRL, &temp8);
            MODIFYFLD(temp8, MISC_CORE_PADS_OE_CNTRL, pgm_agc2_pads_oen, 1);
            ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_CORE_PADS_OE_CNTRL, temp8);
			
            /* Select the OSC Clock clock and disable FCLK dividers */
            ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, &temp8);
            MODIFYFLD(temp8, MISC_MODULATION_FMT_RESET_CONTROL_2, agc_aux_select_enable, 0);
            ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, temp8);
        }

        if(ret_val != SCD_RESULT__OK)
        {
           BDBG_ERR(("BTFE_P_ChipPower: error=%u", ret_val));
        }
    }

    return ret_val;
}

#if 0
/****************************************************************************/
static SCD_RESULT BTFE_P_ChipConfigCofdmAcq(SCD_RESULT ret_val, SCD_HANDLE chip_handle, uint32_t bandwidth_mhz)
{
    CHIP *tchip = GET_CHIP(chip_handle);
    uint32_t temp8;

    /* Set the HI options for acquisition */
    switch(bandwidth_mhz)
    {
    case 5: ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_COFDM_MODE_SELECT, COFDM_INPUT_5MHZ); break;
    case 6: ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_COFDM_MODE_SELECT, COFDM_INPUT_6MHZ); break;
    case 7: ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_COFDM_MODE_SELECT, COFDM_INPUT_7MHZ); break;
    case 8: ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_COFDM_MODE_SELECT, COFDM_INPUT_8MHZ); break;
    }

    temp8 = 0;

    if(tchip->Acquisition.bCoChannelRejection == TRUE)
        temp8 |= COFDM_CO_ENABLE;

    if(tchip->Acquisition.bAdjChannelRejection == TRUE)
        temp8 |= COFDM_ADJ_ENABLE;

    if(tchip->Acquisition.bMobileMode == TRUE)
        temp8 |= COFDM_MOBILE_ENABLE;

    if(tchip->Acquisition.bEnhancedMode == TRUE)
        temp8 |= COFDM_ENHANCED_ACQ_EN;

    if(tchip->Acquisition.bLowPriority == TRUE)
        temp8 |= COFDM_LOW_PRIORITY_EN;

    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_COFDM_CONFIG, temp8);

    return ret_val;

}
#endif

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipConfigSmoother(SCD_RESULT ret_val, SCD_HANDLE chip_handle)
{
    CHIP *tchip = GET_CHIP(chip_handle);
    uint8_t  temp8;
    uint8_t  ofs_target;
    uint8_t  ofs_start_target;
    uint8_t  ofs_insert_count;
    uint8_t  ofs_insert_control;
    uint32_t temp24 = 0;

    /* make sure the smoother is stopped */
    ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixOFS_CONTROL, &temp8);
    MODIFYFLD(temp8, OFS_CONTROL, go_bit_ofs, 0);
    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixOFS_CONTROL, temp8);

    /* reset the core */
    ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, &temp8);
    MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_ofs, 1);
    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, temp8);
    MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_ofs, 0);
    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, temp8);

    /* set OFS_CONTROL */
    ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixOFS_CONTROL, &temp8);
    MODIFYFLD(temp8, OFS_CONTROL, disable_error_filter, (tchip->FatModFormat != SCD_MOD_FORMAT__FAT_VSB));
    MODIFYFLD(temp8, OFS_CONTROL, enable_loop, (tchip->FatData.BurstMode != SCD_BURST_MODE__BURST_ON));
    MODIFYFLD(temp8, OFS_CONTROL, resume_data_output, (tchip->FatData.BurstMode == SCD_BURST_MODE__BURST_ON));
    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixOFS_CONTROL, temp8);

    /* set OFS_TARGET and OFS_START_TARGET */
    if(tchip->FatData.BurstMode == SCD_BURST_MODE__BURST_ON)
    {
        ofs_target = 0x00;
        ofs_start_target = 0x00;
    }
    else
    {
        if(tchip->FatModFormat == SCD_MOD_FORMAT__FAT_VSB)
        {
            ofs_target = SMOOTHER_BUF_FILL_LEVEL_VSB / 2;
            ofs_start_target = SMOOTHER_BUF_FILL_LEVEL_VSB / 2;
        }
        else
        {
            if(tchip->FatData.BurstMode == SCD_BURST_MODE__CONSTANT_PACKET)
                ofs_target = QAM_GAP_TARGET;
            else
                ofs_target = SMOOTHER_BUF_FILL_LEVEL_QAM / 2;

            ofs_start_target = SMOOTHER_BUF_FILL_LEVEL_QAM / 2;
        }
    }

    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixOFS_TARGET, ofs_target);
    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixOFS_START_TARGET, ofs_start_target);


    /* set OFS_NOMINAL_NCO_2 */
    switch(tchip->FatModFormat)
    {
#ifdef SCD_LEGACY_QAM
    case SCD_MOD_FORMAT__FAT_QAM64:
        if(tchip->FatData.HeaderEnable == 1)
        {
            if(tchip->FatData.BurstMode == SCD_BURST_MODE__CONSTANT_PACKET)
                temp24 = SMOOTHER_NCO2_64QAM_GAP;
            else
                temp24 = SMOOTHER_NCO2_64QAM_SYNC_EN;
        }
        else
        {
            temp24 = SMOOTHER_NCO2_64QAM_SYNC_DIS;
        }
        break;

    case SCD_MOD_FORMAT__FAT_QAM256:
        if(tchip->FatData.HeaderEnable == 1)
        {
            if(tchip->FatData.BurstMode == SCD_BURST_MODE__CONSTANT_PACKET)
              temp24 = SMOOTHER_NCO2_256QAM_GAP;
            else
              temp24 = SMOOTHER_NCO2_256QAM_SYNC_EN;
        }
        else
        {
            temp24 = SMOOTHER_NCO2_256QAM_SYNC_DIS;
        }
        break;
#endif
    case SCD_MOD_FORMAT__FAT_J83ABC:
	 if (tchip->j83abc.qamMode == QAM_MODE_DIGITAL_64QAM)
	 {
            if(tchip->FatData.HeaderEnable == 1)
            {
                if(tchip->FatData.BurstMode == SCD_BURST_MODE__CONSTANT_PACKET)
                    temp24 = SMOOTHER_NCO2_64QAM_GAP;
                else
                    temp24 = SMOOTHER_NCO2_64QAM_SYNC_EN;
            }
            else
            {
                temp24 = SMOOTHER_NCO2_64QAM_SYNC_DIS;
            }
	 }
	 if (tchip->j83abc.qamMode == QAM_MODE_DIGITAL_256QAM)
	 {
            if(tchip->FatData.HeaderEnable == 1)
            {
                if(tchip->FatData.BurstMode == SCD_BURST_MODE__CONSTANT_PACKET)
                    temp24 = SMOOTHER_NCO2_256QAM_GAP;
                else
                    temp24 = SMOOTHER_NCO2_256QAM_SYNC_EN;
            }
            else
            {
                temp24 = SMOOTHER_NCO2_256QAM_SYNC_DIS;
            }
	 }	 
        break;
    case SCD_MOD_FORMAT__FAT_COFDM_8:
    case SCD_MOD_FORMAT__FAT_COFDM_7:
    case SCD_MOD_FORMAT__FAT_COFDM_6:
    case SCD_MOD_FORMAT__FAT_COFDM_5:
    case SCD_MOD_FORMAT__FAT_UNIFIED_COFDM:
		temp24 = SMOOTHER_NCO2_MAX;
		break;
    default:
        if(tchip->FatData.HeaderEnable == 1)
        {
            if(tchip->FatData.BurstMode == SCD_BURST_MODE__CONSTANT_PACKET)
              temp24 = SMOOTHER_NCO2_8VSB_GAP;
            else
              temp24 = SMOOTHER_NCO2_8VSB_SYNC_EN;
        }
        else
        {
            temp24 = SMOOTHER_NCO2_8VSB_SYNC_DIS;
        }
        break;
    }

    if(tchip->FatData.BurstMode == SCD_BURST_MODE__BURST_ON)
    {
        temp24 = SMOOTHER_NCO2_MAX;
    }

    ret_val = BTFE_P_ChipSetReg24(ret_val, chip_handle, ixOFS_NOMINAL_NCO_2, temp24);


    /* set OFS_BANDWIDTH */
    switch(tchip->FatModFormat)
    {
    case SCD_MOD_FORMAT__FAT_VSB:
        temp8 = SMOOTHER_BW_VSB;
        break;
#ifdef SCD_LEGACY_QAM
    case SCD_MOD_FORMAT__FAT_QAM64:
        if (tchip->FatData.BurstMode == SCD_BURST_MODE__CONSTANT_PACKET)
            temp8 = SMOOTHER_ZETA;
        else
            temp8 = SMOOTHER_BW_QAM64;
        break;
#endif
    case SCD_MOD_FORMAT__FAT_J83ABC:
	 if (tchip->FatData.BurstMode == SCD_BURST_MODE__CONSTANT_PACKET)
            temp8 = SMOOTHER_ZETA;
        else if (tchip->j83abc.qamMode == QAM_MODE_DIGITAL_64QAM)
            temp8 = SMOOTHER_BW_QAM64;
        else
            temp8 = SMOOTHER_BW_QAM256;
        break;
		
    default:
        if(tchip->FatData.BurstMode == SCD_BURST_MODE__CONSTANT_PACKET)
            temp8 = SMOOTHER_ZETA;
        else
            temp8 = SMOOTHER_BW_QAM256;
        break;
    }

    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixOFS_BANDWIDTH, temp8);


    /* Do the insert count and control based on gapped mode or not */
    if(tchip->FatData.BurstMode == SCD_BURST_MODE__CONSTANT_PACKET)
    {
        switch(tchip->FatModFormat)
        {
#ifdef SCD_LEGACY_QAM
        case SCD_MOD_FORMAT__FAT_QAM64:
            ofs_insert_count = QAM64_INSERT_COUNT;
            ofs_insert_control = GAP_INSERT_CONTROL;
            break;

        case SCD_MOD_FORMAT__FAT_QAM256:		
            ofs_insert_count = QAM256_INSERT_COUNT;
            ofs_insert_control = GAP_INSERT_CONTROL;
            break;
#endif			
        case SCD_MOD_FORMAT__FAT_J83ABC:
            ofs_insert_control = GAP_INSERT_CONTROL;
            if (tchip->j83abc.qamMode == QAM_MODE_DIGITAL_64QAM)
                ofs_insert_count = QAM64_INSERT_COUNT;
	     else 
                ofs_insert_count = QAM256_INSERT_COUNT;		 	
            break;
			
        default:
            ofs_insert_count = VSB6_INSERT_COUNT;
            ofs_insert_control = GAP_INSERT_CONTROL;
            break;
        }
    }
    else
    {
        ofs_insert_count = 0x00;
        ofs_insert_control = 0x00;
    }

    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixOFS_INSERT_COUNT, ofs_insert_count);
    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixOFS_INSERT_CONTROL, ofs_insert_control);


    /* set the go bit */
    ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixOFS_CONTROL, &temp8);
    MODIFYFLD(temp8, OFS_CONTROL, go_bit_ofs, 1);
    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixOFS_CONTROL, temp8);

    return ret_val;
}

/****************************************************************************/

/********************************************************************************************************
BTFE_P_ChipStartFat:

This function handles many of the initializations related to the demodulator front end analog (AFE)
 and digital circuitry.  Other initialization related to these circuits are done by the 8051 uController.
 Some of the initializations done here are expected to vary depending on AFE differences and customer
 preferences.


   Simplified Block Diagram of AFE and demodulator (FAT)

    --------  AFE ----------          ------- Digital Front End -----------
                                                                                                               ________  (to MPEG decoders)
                                                              ---------------------------------------------->|        |--------------->
          ___    ---           ----         ___      -----   |   ___                 ___                     | Demods |
    RFin |   |  |   |   |`    |    |       |   |    |Brick|  |  |   |               |   |                    |  /FEC  | CVBS (to VDEC)
   ----->| X |--|BPF|---| )-->| AD |------>| X |--->|Wall |---->| X |-------------->| X |------------------->|        |--------------->
         |___|  |   |   |/    |    |  |    |___|    | LPF |     |___|  |            |___|   Video/Pilot |     --------
           ^     ---     ^      ---   |      ^       -----        ^    |              ^     Carrier     |         ^
           |     SAW     |            |      |                    |    |              |     Recovery    |         | 
           |     IF      |            |      |   Downconverter    |    |              |                 |         |
           |     Filter  |            |    -----                -----  |              |                 |         |
           |             |            |   |     |              |     | |              |   -----      -------      |      
        -------          |            |   | NCO |              | NCO | |              |  |     |    |  PLL  |     | 
       | Tuner |         |            |   |  1  |              |  2  | |               --| NCO |----|Control|     |        
       |  LO   |         |            |    -----                -----  |                 |  3  |    |       |     |  
       |       |         |            |                                |                  -----      -------      |   
        -------          |            |   Legacy/UAGC                  |                                          v
                         |            |     -------                    |      -----------                       -----      
                         |             --->|  AGC  |                   |     |    8051   |                     | FEC |    
                         |                 |Control|<------------------      |uController|<------------------->| RAM |                                                       
                          -----------------|       |                         |           |                     |     |    
                                            -------                           -----------                       -----     

*/

static SCD_RESULT BTFE_P_ChipStartFat(SCD_RESULT ret_val, SCD_HANDLE chip_handle, SCD_MOD_FORMAT mod_format)
{
	/* Local variables - General  */
		CHIP *tchip = GET_CHIP(chip_handle);
		uint8_t temp8;
	/* Local variables - AD Converter  */
        uint8_t ADCref;
	/* Local variables - Modulation Type  */
		typedef enum
		{
			MODULATION_GROUP__VSB,
			MODULATION_GROUP__ANALOG,
			MODULATION_GROUP__QAM_64,
			MODULATION_GROUP__QAM_256,
			MODULATION_GROUP__J83ABC,
			MODULATION_GROUP__COFDM
		} MODULATION_GROUP;

		MODULATION_GROUP modulationGroup;                                           /* Groupings affecting principal demod configuration */
		uint8_t  channelBandwidth_MHz;                                                /* Bandwidth of received signal in MHz */
		GenericVideoStandard analogModulationLineRate = VIDEO_STANDARD__60HZ;       /* 50 Hz or 60 Hz */
		VideoModulationType analogModulationPolarity = MODULATION_TYPE__NEGATIVE;   /* positive or negative modulation */
		AnalogVideoStandard analogStandardType = VIDEO_STANDARD__NTSC_M;
													/* There is another enumeraton of modulation types
														used for analog modulation, so the SCD 
														enum has to be translated to the 8051 FW enum */
 
	/* Local variables - AGC  */
		uint32_t AGC_delay;                /* delay in msecs to wait for tuner to settle */
		#define AGC_TYPE_LEGACY 1
		#define AGC_TYPE_UAGC 0
		uint32_t agcType = AGC_TYPE_UAGC;  /* 1 = Legacy AGC   0 = UAGC  */
		int32_t *pTableSigned;             /* pointer to read initilization tables where signed negative numbers in
									         tables are used as markers */
		uint32_t *pTable;                  /* pointer to read simple initilization tables */
		uint32_t tableLength;
		uint32_t i;

	/* Local variables - Downconverter  */
		uint32_t NCO_1_dPhi = 0;                /* Step size of Downconverter NCO 1 */
		uint32_t videoCarrierFrequency;         /* Nominal frequency of video carrier frequency at downconverter output */
		uint32_t NCO_2_dPhi = 0;                /* Step size of Downconverter NCO 2 */
		uint8_t tempBuffer[32];           /* buffer for passing Downconverter config to uController via Service */

	/* Local variables - Starting 8051 uC firmware  */
		uint8_t video_status;
		uint8_t videoModeMask = 0;              /* byte indicating to uC FW, which type of Modulation */
		
	    uint32_t timeout;
		uint32_t P_hi, P_lo, Q_hi, Q_lo;

	/* ---------------------------------------------------------------------------------------------
	 * To enable the application (e.g. SCD GUI) to choose the modulation format, for test scripts
     *   write the modulation format to a host interface register.
	 --------------------------------------------------------------------------------------------- */

		BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_SCD_MOD_FORMAT, (uint8_t) mod_format);

	/* ---------------------------------------------------------------------------------------------
	 * Determine TFE configuration based on the modulation type
     *
	 --------------------------------------------------------------------------------------------- */

		switch(mod_format)
		{
		case SCD_MOD_FORMAT__FAT_COFDM_5:
			modulationGroup = MODULATION_GROUP__COFDM;
			/* videoModeMask = (1<<VIDEO_COFDM_BIT); */
			videoModeMask = (VIDEO_MODE_COFDM << VIDEO_MODE_SHIFT);
			channelBandwidth_MHz = 5;
			videoSubmodeMask = SUBMODE_DVBT;
			break;
		case SCD_MOD_FORMAT__FAT_COFDM_6:
			modulationGroup = MODULATION_GROUP__COFDM;
			/* videoModeMask = (1<<VIDEO_COFDM_BIT); */
			videoModeMask = (VIDEO_MODE_COFDM << VIDEO_MODE_SHIFT);
			channelBandwidth_MHz = 6;
			videoSubmodeMask = SUBMODE_DVBT;
			break;
		case SCD_MOD_FORMAT__FAT_COFDM_7:
			modulationGroup = MODULATION_GROUP__COFDM;
			/* videoModeMask = (1<<VIDEO_COFDM_BIT); */
			videoModeMask = (VIDEO_MODE_COFDM << VIDEO_MODE_SHIFT);
			channelBandwidth_MHz = 7;
			videoSubmodeMask = SUBMODE_DVBT;
			break;
		case SCD_MOD_FORMAT__FAT_COFDM_8:
			modulationGroup = MODULATION_GROUP__COFDM;
			/* videoModeMask = (1<<VIDEO_COFDM_BIT); */
			videoModeMask = (VIDEO_MODE_COFDM << VIDEO_MODE_SHIFT);
			channelBandwidth_MHz = 8;
			videoSubmodeMask = SUBMODE_DVBT;
			break;
		case SCD_MOD_FORMAT__FAT_UNIFIED_COFDM:
			modulationGroup = MODULATION_GROUP__COFDM;
			videoModeMask = (VIDEO_MODE_UCOFDM << VIDEO_MODE_SHIFT);
			videoSubmodeMask = SUBMODE_DVBT;
			channelBandwidth_MHz = 8;
			break;
			
		case SCD_MOD_FORMAT__FAT_VSB:
			modulationGroup = MODULATION_GROUP__VSB;
			/* videoModeMask = (1<<VIDEO_8VSB_BIT); */
			videoModeMask = (VIDEO_MODE_VSB << VIDEO_MODE_SHIFT);
			channelBandwidth_MHz = 6;
			break;
#ifdef SCD_LEGACY_QAM
		case SCD_MOD_FORMAT__FAT_QAM64:
			modulationGroup = MODULATION_GROUP__QAM_64;
			/* videoModeMask = (1<<VIDEO_64QAM_BIT); */
			videoModeMask = (VIDEO_MODE_64QAM << VIDEO_MODE_SHIFT);
			channelBandwidth_MHz = 6;
			break;
		case SCD_MOD_FORMAT__FAT_QAM256:
			modulationGroup = MODULATION_GROUP__QAM_256;
			/* videoModeMask = (1<<VIDEO_256QAM_BIT); */
			videoModeMask = (VIDEO_MODE_256QAM << VIDEO_MODE_SHIFT);
			channelBandwidth_MHz = 6;
			break;
#endif
			
		case SCD_MOD_FORMAT__FAT_J83ABC:
			modulationGroup = MODULATION_GROUP__J83ABC;
			videoModeMask = (VIDEO_MODE_J83ABC << VIDEO_MODE_SHIFT);
			videoSubmodeMask = J83_QAM64 | SUBMODE_J83_B; /* not used */
			channelBandwidth_MHz = 6;
			break;

		case SCD_MOD_FORMAT__FAT_NTSC_M:
			analogStandardType = VIDEO_STANDARD__NTSC_M;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__60HZ; 
			channelBandwidth_MHz = 6;
			break;
		case SCD_MOD_FORMAT__FAT_NTSC_N:
			analogStandardType = VIDEO_STANDARD__NTSC_N;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			channelBandwidth_MHz = 6;
			break;
		case SCD_MOD_FORMAT__FAT_NTSC_J:
			analogStandardType = VIDEO_STANDARD__NTSC_J;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__60HZ; 
			channelBandwidth_MHz = 6;
			break;
		case SCD_MOD_FORMAT__FAT_NTSC_443:  
			analogStandardType = VIDEO_STANDARD__NTSC_443;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__60HZ; 
			channelBandwidth_MHz = 8;
			break;
		case SCD_MOD_FORMAT__FAT_PAL_I:
			analogStandardType = VIDEO_STANDARD__PAL_I;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			channelBandwidth_MHz = 8;
			break;
		case SCD_MOD_FORMAT__FAT_PAL_B:
			analogStandardType = VIDEO_STANDARD__PAL_B;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			channelBandwidth_MHz = 7;
			break;
		case SCD_MOD_FORMAT__FAT_PAL_B1:
			analogStandardType = VIDEO_STANDARD__PAL_B1;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			channelBandwidth_MHz = 8;
			break;
		case SCD_MOD_FORMAT__FAT_PAL_G:
			analogStandardType = VIDEO_STANDARD__PAL_G;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			channelBandwidth_MHz = 8;
			break;
		case SCD_MOD_FORMAT__FAT_PAL_H:
			analogStandardType = VIDEO_STANDARD__PAL_H;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			channelBandwidth_MHz = 8;
			break;
		case SCD_MOD_FORMAT__FAT_PAL_D:    
			analogStandardType = VIDEO_STANDARD__PAL_D;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			channelBandwidth_MHz = 8;
			break;
		case SCD_MOD_FORMAT__FAT_PAL_K:
			/* IFD Firmware does not have this in its list of modulation types
			    since system K generally uses SECAM color.  Other than color,
				system K is the same as system D, so the IFD will work
				if PAL D system is used
			    */
			analogStandardType = VIDEO_STANDARD__PAL_D;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			channelBandwidth_MHz = 8;
			break;
		case SCD_MOD_FORMAT__FAT_PAL_60:
			analogStandardType = VIDEO_STANDARD__PAL_60;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__60HZ; 
			channelBandwidth_MHz = 8;
			break;
		case SCD_MOD_FORMAT__FAT_PAL_M:
			analogStandardType = VIDEO_STANDARD__PAL_M;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__60HZ; 
			channelBandwidth_MHz = 6;
			break;
		case SCD_MOD_FORMAT__FAT_PAL_N:
			analogStandardType = VIDEO_STANDARD__PAL_N;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			channelBandwidth_MHz = 6;
			break;
		case SCD_MOD_FORMAT__FAT_PAL_NC:    
			analogStandardType = VIDEO_STANDARD__PAL_N_COMBO;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			channelBandwidth_MHz = 6;
			break;
		case SCD_MOD_FORMAT__FAT_SECAM_B:
			analogStandardType = VIDEO_STANDARD__SECAM_B;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			channelBandwidth_MHz = 7;
			break;
		case SCD_MOD_FORMAT__FAT_SECAM_D:
			analogStandardType = VIDEO_STANDARD__SECAM_D;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			channelBandwidth_MHz = 8;
			break;
		case SCD_MOD_FORMAT__FAT_SECAM_G:
			analogStandardType = VIDEO_STANDARD__SECAM_G;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			channelBandwidth_MHz = 8;
			break;
		case SCD_MOD_FORMAT__FAT_SECAM_H:
			analogStandardType = VIDEO_STANDARD__SECAM_H;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			channelBandwidth_MHz = 8;
			break;
		case SCD_MOD_FORMAT__FAT_SECAM_K:
			analogStandardType = VIDEO_STANDARD__SECAM_K;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			channelBandwidth_MHz = 8;
			break;
		case SCD_MOD_FORMAT__FAT_SECAM_K1:
			analogStandardType = VIDEO_STANDARD__SECAM_K1;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__NEGATIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			channelBandwidth_MHz = 8;
			break;
		case SCD_MOD_FORMAT__FAT_SECAM_L:
			analogStandardType = VIDEO_STANDARD__SECAM_L;
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__POSITIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			channelBandwidth_MHz = 8;
			break;
		case SCD_MOD_FORMAT__FAT_SECAM_L1:  
			modulationGroup = MODULATION_GROUP__ANALOG;
			analogModulationPolarity = MODULATION_TYPE__POSITIVE; 
			analogModulationLineRate = VIDEO_STANDARD__50HZ; 
			analogStandardType = VIDEO_STANDARD__SECAM_L1;
			channelBandwidth_MHz = 8;
			break;
		default:
			BDBG_ERR(("BTFE_P_ScdStartFat: invalid modulation format code"));
			return SCD_RESULT__CHIP_ERROR;
			break;
		}

		if	(modulationGroup == MODULATION_GROUP__ANALOG)
		{
			agcType = AGC_TYPE_UAGC;
			/* videoModeMask = (1<<VIDEO_NTSC_BIT);  *//* name, used historically to mean analog modulation */
			videoModeMask = (VIDEO_MODE_NTSC << VIDEO_MODE_SHIFT);
		}

	/* ---------------------------------------------------------------------------------------------
	 * A/D converter multiplexer and reference voltage programming
     *
	 --------------------------------------------------------------------------------------------- */
		/* Set the A/D reference voltage */
        ADCref = (uint8_t)  ((tchip->MISCandADCRef >> 4) & 0x03);

		/* Assert & Hold ADC0 FIFO in Reset until TFE cores are soft-reset*/ 
		ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, &temp8);
		MODIFYFLD(temp8, MISC_MODULATION_FMT_RESET_CONTROL_2, soft_reset_adc0_fifo, 1);
		ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, temp8);
		
	/* ---------------------------------------------------------------------------------------------
	 * Reset the hardware blocks and general configuration of hardware for the demodulation
     *  of the received signal type.

			  MISC_MODULATION_FMT_RESET_CONTROL_2    [R/W]    8 bits    Access: 8    primaryRegisterAperture:0xf00b  
				modulation_format 2:0 0x0 This selects the modulation format for the whole chip.
				000 - 8-VSB
				001 - NTSC
				010 - 64-QAM
				011 - 256-QAM
				100 - 111 Undefined

				vq_dvbt_switch 3 0x0 Selects between VSB/QAM/NTSC core and the DVB-T core
				Controls the shared memories and the Smoother block
				0 - VSB/QAM/NTSC.
				1 - DVB-T.
 
				agc_aux_select_enable 5 0x0 Select the Auxiliary Loop of the AGC. 
				0 - Disable Auxiliary Loop.		1 - Enable Auxiliary Loop.
 
				soft_reset_adc0_fifo 6 0x0 Selectively reset ADC0 FIFO.
				0 - Normal Mode			1 - Reset ADC0 FIFO
 
				soft_reset_adc1_fifo 7 0x0 Selectively reset ADC1 FIFO. 
				0 - Normal Mode			1 - Reset ADC1 FIFO
 
			MISC_RESET_CONTROL_0    [R/W]    8 bits    Access: 8    primaryRegisterAperture:0xf00d  
				soft_reset_to_agc       0 0x0 Selectively resets AGC to default power-up values. This is a non-self-clearing bit.
				soft_reset_to_fe        1 0x0 Selectively resets FE core to default power-up state. This is a non-self-clearing bit.
				soft_reset_to_equalizer 2 0x0 Selectively resets Equalizer to default power-up state. This is a non-self-clearing bit.
				soft_reset_to_fec       3 0x0 Selectively resets 2002 FEC to default power-up state. This is a non-self-clearing bit.
				soft_reset_to_ofs       4 0x0 Selectively resets OFS core to the default power-up state. This is a non-self-clearing bit.
				soft_reset_to_dvbt      5 0x0 Selectively resets DVB-T core to default power-up values. This is a non-self-clearing bit.
				soft_reset_to_ntsc      6 0x0 Selectively resets NTSC core to default power-up values. This is a non-self-clearing bit.
				soft_reset_to_bert      7 0x0 Selectively resets BERT core to default power-up values. This is a non-self-clearing bit.

	 --------------------------------------------------------------------------------------------- */

		/*Assert Soft Reset on all cores*/ 
		ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, &temp8);
		MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_agc, 1);
		MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_fe, 1);
		MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_equalizer, 1);
		MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_fec, 1);
		MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_ofs, 1);
		MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_dvbt, 1);
		MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_ntsc, 1);
		MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_bert, 1);
		ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, temp8);

		/*De-assert Soft Reset on all cores*/ 		
		MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_agc, 0);
		MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_fe, 0);
		MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_equalizer, 0);
		MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_fec, 0);
		MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_ofs, 0);
		MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_dvbt, 0);
		MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_ntsc, 0);
		MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_bert, 0);
		ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, temp8);
		
		/* De-assert ADC0 FIFO in Reset*/ 		
		ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, &temp8);
		MODIFYFLD(temp8, MISC_MODULATION_FMT_RESET_CONTROL_2, soft_reset_adc0_fifo, 0);
		ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, temp8);		

		if	(modulationGroup == MODULATION_GROUP__ANALOG)
		{
			/* IFD modes: configure intlvr RAM for uController use */
			ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_UC_FEC_PSD_AGC_RAM_I2C_CNTRL, &temp8);
			MODIFYFLD(temp8, MISC_UC_FEC_PSD_AGC_RAM_I2C_CNTRL, i2c_ram_access_enable, 1);
			MODIFYFLD(temp8, MISC_UC_FEC_PSD_AGC_RAM_I2C_CNTRL, uc_fec_ram_access_enable, 1);
			ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_UC_FEC_PSD_AGC_RAM_I2C_CNTRL, temp8);
		}

		if	(modulationGroup == MODULATION_GROUP__ANALOG)
		{
			/* Configure Demodulator Hardware for IF demod operation */
			ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, &temp8);
			MODIFYFLD(temp8, MISC_MODULATION_FMT_RESET_CONTROL_2, agc_aux_select_enable, 0);/* 0 = Disable Auxiliary Loop */
			ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, temp8);
		}

#if 0
		if	(modulationGroup == MODULATION_GROUP__COFDM)
		{
			/* Configure Demodulator Hardware for COFDM demod operation */

			ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, &temp8);
			ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, temp8);

			/* Configure COFDM demod */
			ret_val = BTFE_P_ChipConfigCofdmAcq(ret_val, chip_handle, channelBandwidth_MHz);

		}
#endif


   /* ---------------------------------------------------------------------------------------------
	 * AGC
    *  
	 --------------------------------------------------------------------------------------------- */


		/* ---------------------------------------------------------------------------------------------
		 * UAGC configuration
		 --------------------------------------------------------------------------------------------- */
		/* Configuration dependent on IF VGA  */
		
		pTableSigned = tchip->pUAGC_IF_VGAconfig;
		if (pTableSigned != 0)
		{
			for(i=0; (pTableSigned[i] > 0); i+=2)
				ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, pTableSigned[i], (uint8_t) pTableSigned[i+1]);
		}

		if (modulationGroup == MODULATION_GROUP__ANALOG)
		{

			/* UAGC register initializations common for all analog modulation types */
				pTable = tchip->UagcConfig_Analog;
				tableLength = tchip->UagcConfig_Analog_Length/(sizeof(int32_t));
				for(i=0; i < tableLength; i+=2)
					ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, pTable[i], (uint8_t) pTable[i+1]);

			/* UAGC register initializations depending on analog modulation polarity */

				/* Config not dependent on AFE */
				switch(analogModulationPolarity)
				{
				case MODULATION_TYPE__POSITIVE:
					pTable = tchip->UagcConfig_Analog_PositiveModulation;
					tableLength = tchip->UagcConfig_Analog_PositiveModulation_Length/(sizeof(int32_t));
					break;
				case MODULATION_TYPE__NEGATIVE:
					pTable = tchip->UagcConfig_Analog_NegativeModulation;
					tableLength = tchip->UagcConfig_Analog_NegativeModulation_Length/(sizeof(int32_t));
					break;
				}
				for(i=0; i < tableLength; i+=2)
					ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, pTable[i], (uint8_t) pTable[i+1]);

				/* Config dependent on AFE */
				switch(analogModulationPolarity)
				{
				case MODULATION_TYPE__POSITIVE:
					pTableSigned = tchip->pUAGCanalogPosConfig;
					break;
				case MODULATION_TYPE__NEGATIVE:
					pTableSigned = tchip->pUAGCanalogNegConfig;
					break;
				}

				for(i=0; (pTableSigned[i] > 0); i+=2)
					ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, pTableSigned[i], (uint8_t) pTableSigned[i+1]);

			 
			/* UAGC register initializations depending on analog modulation field rate */
			switch(analogModulationLineRate)
			{
			case VIDEO_STANDARD__50HZ:
				pTable = tchip->UagcConfig_Analog_50Hz;
				tableLength = tchip->UagcConfig_Analog_50Hz_Length/(sizeof(int32_t));
				break;
			case VIDEO_STANDARD__60HZ:
				pTable = tchip->UagcConfig_Analog_60Hz;
				tableLength = tchip->UagcConfig_Analog_60Hz_Length/(sizeof(int32_t));
				break;
			default:
				tableLength = 0;
				break;
			}
			for(i=0; i < tableLength; i+=2)
				ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, pTable[i], (uint8_t) pTable[i+1]);
		}
		else  /* modulationGroup != MODULATION_GROUP__ANALOG */
		{

			/* Config not dependent on AFE */
				pTable = tchip->UagcConfig_Digital_Modulation;
				tableLength = tchip->UagcConfig_Digital_Modulation_Length/(sizeof(int32_t));
				for(i=0; i < tableLength; i+=2)
					ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, pTable[i], (uint8_t) pTable[i+1]);

			/* Config dependent on AFE */
				pTableSigned = tchip->pUAGCdigitalConfig;

				for(i=0; (pTableSigned[i] > 0); i+=2)
					ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, pTableSigned[i], (uint8_t) pTableSigned[i+1]);
		}

		/* ---------------------------------------------------------------------------------------------
		 * IF Demod configuration dependent on Analog Front End
		--------------------------------------------------------------------------------------------- */
		
		pTableSigned = tchip->pIFDconfig;

		for(i=0; (pTableSigned[i] > 0); i+=2)
			ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, pTableSigned[i], (uint8_t) pTableSigned[i+1]);
		
 
	/* ---------------------------------------------------------------------------------------------
	 * Downconverter
     *  
	 *      Digital   ___      -----       ___  Complex
	 *      IF       |   |    |Brick|     |   | Baseband
	 *     --------->| X |===>|Wall |====>| X |============>
	 *               |___|    | LPF |     |___|  
	 *                 ^       -----        ^    
	 *                 |                    |    
	 *                 |   Downconverter    |    
	 *               -----                -----  
	 *   NCO_1_dPhi |     |   NCO_2_dPhi |     | 
	 *   ---------->| NCO |   ---------->| NCO | 
	 *              |  1  |              |  2  | 
	 *               -----                -----  
	 *
			NCO equations:     dPhi = F * 2^24 / Fclk
							   F    = dPhi * Fclk / 2^N
							   dPhi_normalized = F / Fclk
                           
							   dPhi:  step size of NCO phase accumulator (not normalized)
							   F:     sinusoid freuency (Hz)
							   Fclk:  NCO HW clock frequency (sampling rate)
							   N:     register width of NCO phase accumulator
                             
			e.g	 NCO 1 
				Fclk = 54.1e6
				F = 44e6 *** See Note below
				N = 24
				dPhi = int (F * 2^24 / Fclk) = 13645055
			
			e.g.  NCO 2
				Fclk = 13.525e6
				F = 1.75e6
				N = 24
				dPhi = int (F * 2^24 / Fclk) = 2170804
			
			Note: June 2009.  Analog demodulator firmware is programming the NCO 1 directly
			      to 44 MHz to match the analog IF frequency at the A/D.  This avoids 
				  some confusion.

				  Alternatively, NCO 1 could be programmed to 54.1-44.0=10.1 MHz and the 
				  conjugate_brickwall_out and neg_passback_freq can be asserted to correct
				  the spectrum.  This is the calculation used for digital demodulation.

	
	 --------------------------------------------------------------------------------------------- */

		{
			uint32_t x;
			int32_t diff;
			bool spectrumInv, tunerSpectrumInv;

			tchip->FatIfFrequency = tchip->tunerIfFrequency; 				
			if (tchip->FatIfFrequency > LOW_IF_MAX_FREQ) /* High IF, low IF is 3-9MHz */
			{
				diff = (int32_t) tchip->FatIfFrequency - FE_CRYSTAL_FREQ;
				if (diff >= 0)
					x = (uint32_t)diff;
				else
					x = (uint32_t)-diff;
			}
			else
			{
				diff = -1;
				x = (int32_t) tchip->FatIfFrequency;
			}
			
			BTFE_MultU32U32(16777216 * 2, x, &P_hi, &P_lo);
			BTFE_DivU64U32(P_hi, P_lo, FE_CRYSTAL_FREQ, &Q_hi, &Q_lo);

			Q_lo = (Q_lo + 1) >> 1; /* round */
			if (diff < 0)
				NCO_1_dPhi = 16777216 - Q_lo;
			else
				NCO_1_dPhi = 16777216 + Q_lo;

			tchip->defaultIFNomRate = NCO_1_dPhi;

			/* set ASM Config to Xdata 
				- byte 0:     acquire_t acquireConfig;          0 - Directed Acquire, 1 - Full Acquire, 2-Search/Scan
				- byte 1:     bandwidth_t bandwidthConfig;  0 - 6 MHz, 1 - 7 MHz, 2 - 8 MHz
				- byte 2:     spectral_t spectralConfig;   0 - non-inverted, 1 - inverted
				- byte 3-6:  uint32 defaultIFNomRate;  Nominal Rate of default IF center frequency in upper 3 bytes and lower byte 0x00.
				- byte 7:     bSpectrumAutoDetect    1: set to 1 for serach scan mode, other is set to 0.
			*/
			tempBuffer[0] = tchip->Acquisition.acqConfig;

			/*
			 * For J83A the BW filter is selected based on the symbol rate:
			 *     if (bandWidthConfig == SCD_BW_UNDEFINED) && demod == J83A)
			 *     then change the bandWidthConfig based on symbol rate.
			 */
			if (tchip->Acquisition.bandWidthConfig == SCD_BW_UNDEFINED)
			{
				if ((modulationGroup == MODULATION_GROUP__J83ABC) && (tchip->j83abc.j83abcMode == J83A))
				{
					uint32_t symbolRate;
					
					symbolRate = (uint32_t) tchip->j83abc.symbolRate;
					if (symbolRate > 7000000) tchip->Acquisition.bandWidthConfig = SCD_BW_9MHZ;
					else if (symbolRate > 6100000) tchip->Acquisition.bandWidthConfig = SCD_BW_8MHZ;
					else if (symbolRate > 5220000) tchip->Acquisition.bandWidthConfig = SCD_BW_7MHZ;
					else if (symbolRate > 4350000) tchip->Acquisition.bandWidthConfig = SCD_BW_6MHZ;
					else if (symbolRate > 3480000) tchip->Acquisition.bandWidthConfig = SCD_BW_5MHZ;
					else if (symbolRate > 2610000) tchip->Acquisition.bandWidthConfig = SCD_BW_4MHZ;
					else if (symbolRate > 1740000) tchip->Acquisition.bandWidthConfig = SCD_BW_3MHZ;
					else  tchip->Acquisition.bandWidthConfig = SCD_BW_2MHZ;
				}
				else
				{
					BDBG_ERR(("BTFE_P_ScdStartFat: invalid bandwidth"));
					return SCD_RESULT__CHIP_ERROR;
				}					
			}
			
			tempBuffer[1] = tchip->Acquisition.bandWidthConfig;
			tunerSpectrumInv = tchip->tunerSpectrum;
			if (tchip->FatIfFrequency > LOW_IF_MAX_FREQ)
				tunerSpectrumInv = tchip->tunerSpectrum ? false : true;
			spectrumInv = (tchip->Acquisition.bSpectrumInversion) ? ((tunerSpectrumInv) ? false: true): (tunerSpectrumInv);
			if (spectrumInv == true) 
				tempBuffer[2] = 1;
			else tempBuffer[2] = 0;

			tempBuffer[3] = (uint8_t)(NCO_1_dPhi >> 16);
			tempBuffer[4] = (uint8_t)(NCO_1_dPhi >> 8);
			tempBuffer[5] = (uint8_t)(NCO_1_dPhi >> 0);
			tempBuffer[6] = 0;
			tempBuffer[7] = tchip->Acquisition.bSpectrumAutoDetect;

			BTFE_P_ChipSetRegs(ret_val, chip_handle, ixHI_ASM_CONFIG_AREA, 8, tempBuffer);
		}

	
		switch(modulationGroup)
		{
		case MODULATION_GROUP__ANALOG:
		    	{
			bool bSpectrumInv, bTunerSpectrumInv;
			bool b216MHzADCclk;

			/* Program NCO 1 to offset the signal in the brick wall filter */

				/* Remove frequency shift, so that 
				 *  e.g.  Typical nominal North American IF frequency:  44 MHz 
				 *        Typical nominal North American IF spectrum is inverted 
				 *         so typically:
				 *          nominal signal center  44.00 MHz
				 *          nominal Sound IF       41.25 MHz
				 *          nominal Video Carrier  45.75 MHz
				 *        To reduce the SAW filter attenuation of the
				 *         audio signal set:
				 *          IfFrequencyShift  100 KHz
				 *         so now the shifted nominal frequencies are
				 *          nominal signal center  44.10 MHz (tunerIfFrequency)
				 *          nominal Sound IF       41.35 MHz
				 *          nominal Video Carrier  45.85 MHz
				 *        To provide the same offset filtering in the brickwall filter
				 *         set NCO 1 to 44.10 MHz - 0.1 MHz = 44.0 MHz (FatIfFrequency)
				 *         so the brickwall (3MHz LFF) output frequencies are: 
				 *          nominal signal center   0.10 MHz
				 *          nominal Sound IF       -2.65 MHz
				 *          nominal Video Carrier   1.85 MHz (videoCarrierFrequency)
				 *
				*/

				BTFE_P_ChipGetReg8(ret_val, chip_handle, ixUC_GP_6, &temp8);
				if (temp8)
					b216MHzADCclk = 1;
				else b216MHzADCclk = 0;

				tchip->FatIfFrequency = tchip->tunerIfFrequency - tchip->IfFrequencyShift;
				/* Calculate NCO 1 step size dPhi */
				/* dPhi    =                              F             *      2^24     /     Fclk  */
				/* NCO_1_dPhi = (uint32_t) ((double) (tchip->FatIfFrequency) * (16777216.0f) * (1.0f/54.1e6f)); */
				BTFE_MultU32U32(tchip->FatIfFrequency, 16777216, &P_hi, &P_lo);

				if (b216MHzADCclk)
					BTFE_DivU64U32(P_hi, P_lo, (FE_CRYSTAL_FREQ*4), &Q_hi, &NCO_1_dPhi);
				else
					BTFE_DivU64U32(P_hi, P_lo, FE_CRYSTAL_FREQ, &Q_hi, &NCO_1_dPhi);	

                        /* FE_FIFO_MODE 
                              irf_fifo_test_mode [1:1] RW Static  ADCFIFO test mode.  0-normal mode, 1-output 12h400.
                              irf_clk_sel [0:0] RW Static  ADC clock select.
                                    "0" - 54MHz input, "1" - enables 216MHz input and 1:4 decimation in DC  */

				ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixFE_FIFO_MODE, &temp8);
				MODIFYFLD(temp8, FE_FIFO_MODE, irf_clk_sel, b216MHzADCclk);  /* 0: 54 MHz ADC clk  1: 216 MHz ADC clk */
				ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixFE_FIFO_MODE, temp8);


				/* Program NCO 2 to downconvert the nominal video carrier to DC */

				switch (channelBandwidth_MHz)
				{
					case 6:
						videoCarrierFrequency = 1750000 + tchip->IfFrequencyShift;
						break;
					case 7:
						videoCarrierFrequency = 2250000 + tchip->IfFrequencyShift;
						break;
					case 8:
						videoCarrierFrequency = 2750000 + tchip->IfFrequencyShift;
						break;
					default:
						videoCarrierFrequency = 1750000 + tchip->IfFrequencyShift;
						break;
				}
    
				/* Calculate NCO 2 step size dPhi */
				/* dPhi    =                              F             *      2^24     /     Fclk  */
				/* NCO_2_dPhi = (uint32_t) ((double) (videoCarrierFrequency) * (16777216.0f) * (1.0f/13.525e6f)); */
				BTFE_MultU32U32(videoCarrierFrequency, 16777216, &P_hi, &P_lo);
				BTFE_DivU64U32(P_hi, P_lo, 13525000, &Q_hi, &NCO_2_dPhi);				

				/* Load NCO Control Registers */
				ret_val = BTFE_P_ChipSetReg24(ret_val, chip_handle, ixFE_DC_NORMALIZED_IF_2, NCO_1_dPhi);    
				ret_val = BTFE_P_ChipSetReg24(ret_val, chip_handle, ixFE_DC_VIDEO_CARRIER_FREQ_2, NCO_2_dPhi);    

				/* Load Downconverter Control Registers */
				bTunerSpectrumInv = tchip->tunerSpectrum;
				/* This is no longer needed for analog as the IF is directly programmed into NCO1 */
				/*
				if (tchip->FatIfFrequency > LOW_IF_MAX_FREQ)
					tunerSpectrumInv = tchip->tunerSpectrum ? false : true;
				*/
				bSpectrumInv = (tchip->Acquisition.bSpectrumInversion) ? ((bTunerSpectrumInv) ? false: true): (bTunerSpectrumInv);

				temp8=0;
				MODIFYFLD(temp8, FE_DC_CONTROL, conjugate_brickwall_out, !bSpectrumInv);
				MODIFYFLD(temp8, FE_DC_CONTROL, neg_passback_freq, !bSpectrumInv);
				ret_val = BTFE_P_ChipSetReg8 (ret_val, chip_handle, ixFE_DC_CONTROL, temp8);    

			break;
			}
		case MODULATION_GROUP__VSB:
		case MODULATION_GROUP__QAM_64:
		case MODULATION_GROUP__QAM_256:
		case MODULATION_GROUP__J83ABC:
			{
				bool spectrumInv, tunerSpectrumInv;
#if 0
				uint8_t timingNominalRate[6];
				uint32_t symbolRate; /* symbol rate in Hz */
				int i;
#endif

				/* old path to pass IF info to 8051 to program ixFE_DC_NORMALIZED_IF_2 */
				tempBuffer[0] = videoModeMask; /* needs more cases! */

				/* For L-Qam */
				tunerSpectrumInv = tchip->tunerSpectrum;
				if (tchip->FatIfFrequency > LOW_IF_MAX_FREQ)
					tunerSpectrumInv = tchip->tunerSpectrum ? false : true;
				spectrumInv = (tchip->Acquisition.bSpectrumInversion) ? ((tunerSpectrumInv) ? false: true): (tunerSpectrumInv);
				
				if (spectrumInv)
					tempBuffer[0] |= 0x02;
		  
				tempBuffer[1] = (uint8_t)(NCO_1_dPhi >> 16);
				tempBuffer[2] = (uint8_t)(NCO_1_dPhi >> 8);
				tempBuffer[3] = (uint8_t)(NCO_1_dPhi >> 0);
				tempBuffer[4] = 0;
				ret_val = BTFE_P_ChipDoMicroService(ret_val, chip_handle, SVC_DEFAULT_IF_CONFIG, 5, tempBuffer, 0, NO_RESULTS, ATRACKING_TIMEOUT);
				if(ret_val != SCD_RESULT__OK)
					return ret_val;

#if 0
				/* 
				 * set J83 Config to Xdata
				 *	- byte 0:      j 83abcMode_t j83abcMode; 0 - J83A, 1 - J83B, 2 - J83C
				 *	- byte 1:      qamModeDigital_t qamMode; 0 - 16QAM, 1-32QAM, 2-64QAM, 3-128QAM, 4-256QAM
				 *	- byte 2-5:   uint32 symbolRate;	
				 *	- byte 6-11:  uchar timingNominalRate[6];
				 *	if symbolRate = 6900000, then confusingArray[6]={0x00, 0xFA, 0x6F, 0x4D, 0xE9, 0xBD};
				 *    - byte 12: waitOnStart: for symbolrateVerify
				 */

				symbolRate = (uint32_t) tchip->j83abc.symbolRate;
                /*symbolRate = 5000000;*/
				BTFE_MultU32U32(13500000UL * 256, 2147483648UL, &P_hi, &P_lo);  /* P = [P_hi,P_lo] = 13.5e6 * 2^39 = (13.5e6*2^8)*2^31 */
				BTFE_DivU64U32(P_hi, P_lo, symbolRate, &Q_hi, &Q_lo); /* Q = [Q_hi,Q_lo] = P/symbolRate */
				
				timingNominalRate[0] =(uint8_t)((Q_hi >> 8) & 0xFF);
				timingNominalRate[1] = (uint8_t)(Q_hi & 0xFF);
				timingNominalRate[2] = (uint8_t)((Q_lo >> 24) & 0xFF);
				timingNominalRate[3] = (uint8_t)((Q_lo >> 16) & 0xFF);
				timingNominalRate[4] = (uint8_t)((Q_lo >> 8) & 0xFF);
				timingNominalRate[5] = (uint8_t)(Q_lo & 0xFF);
                /*for (i = 0; i < 6; ++i)	
                printf("timingNominalRate[%i] = 0x%x\n", i, timingNominalRate[i]);*/
				tempBuffer[0] = tchip->j83abc.j83abcMode;
				tempBuffer[1] = tchip->j83abc.qamMode;
				tempBuffer[2] = (uint8_t) (tchip->j83abc.symbolRate >> 24);
				tempBuffer[3] = (uint8_t) (tchip->j83abc.symbolRate >> 16);
				tempBuffer[4] = (uint8_t) (tchip->j83abc.symbolRate >> 8);
				tempBuffer[5] = (uint8_t) (tchip->j83abc.symbolRate >> 0);

				for (i = 0; i < 6; ++i)
					tempBuffer[i+6] = timingNominalRate[i];
				tempBuffer[12] = 0;
				
				BTFE_P_ChipSetRegs(ret_val, chip_handle, ixHI_J83_CONFIG_AREA, 13, tempBuffer);
#endif				
			}
			break;
		case MODULATION_GROUP__COFDM:
			ret_val = BTFE_P_ChipSetReg24(ret_val, chip_handle, ixFE_DC_NORMALIZED_IF_2, NCO_1_dPhi);    
			temp8=0;
			MODIFYFLD(temp8, FE_DC_CONTROL, conjugate_brickwall_out, 0);
			MODIFYFLD(temp8, FE_DC_CONTROL, neg_passback_freq, 0);
			ret_val = BTFE_P_ChipSetReg8 (ret_val, chip_handle, ixFE_DC_CONTROL, temp8); 

#if 0
			/*
			 * Set Unified Cofdm Config data to Xdata
			 */
			tempBuffer[0] = (uint8_t) tchip->unifiedCofdm.ofdmStandard;
			tempBuffer[1] = (uint8_t) tchip->unifiedCofdm.cciEnable;
			tempBuffer[2] = (uint8_t) tchip->unifiedCofdm.aciEnable;
			tempBuffer[3] = (uint8_t) tchip->unifiedCofdm.mobilMode;
			tempBuffer[4] = (uint8_t) tchip->unifiedCofdm.priorityMode;
			tempBuffer[5] = (uint8_t) tchip->unifiedCofdm.carrierRange;			
			tempBuffer[6] = (uint8_t) tchip->unifiedCofdm.impluse;
			tempBuffer[7] = (uint8_t) tchip->unifiedCofdm.rsLayer;
			
			tempBuffer[8] = (uint8_t) tchip->unifiedCofdm.modeGuard;
			tempBuffer[9] = (uint8_t) tchip->unifiedCofdm.mode;
			tempBuffer[10] = (uint8_t) tchip->unifiedCofdm.guard;			
			tempBuffer[11] = (uint8_t) tchip->unifiedCofdm.tpsMode;
			tempBuffer[12] = (uint8_t) tchip->unifiedCofdm.codeLP;
			tempBuffer[13] = (uint8_t) tchip->unifiedCofdm.codeHP;
			tempBuffer[14] = (uint8_t) tchip->unifiedCofdm.hierarchy;
			tempBuffer[15] = (uint8_t) tchip->unifiedCofdm.modulation;
			
			tempBuffer[16] = (uint8_t) tchip->unifiedCofdm.modulationLayerA;			
			tempBuffer[17] = (uint8_t) tchip->unifiedCofdm.modulationLayerB;			
			tempBuffer[18] = (uint8_t) tchip->unifiedCofdm.modulationLayerC;
			tempBuffer[19] = (uint8_t) tchip->unifiedCofdm.codeRateLayerA;
			tempBuffer[20] = (uint8_t) tchip->unifiedCofdm.codeRateLayerB;
			tempBuffer[21] = (uint8_t) tchip->unifiedCofdm.codeRateLayerC;
			tempBuffer[22] = (uint8_t) tchip->unifiedCofdm.segmentLayerA;
			tempBuffer[23] = (uint8_t) tchip->unifiedCofdm.segmentLayerB;
			
			tempBuffer[24] = (uint8_t) tchip->unifiedCofdm.segmentLayerC;			
			tempBuffer[25] = (uint8_t) tchip->unifiedCofdm.timeInterleaveLayerA;
			tempBuffer[26] = (uint8_t) tchip->unifiedCofdm.timeInterleaveLayerB;
			tempBuffer[27] = (uint8_t) tchip->unifiedCofdm.timeInterleaveLayerC;
			tempBuffer[28] = (uint8_t) tchip->unifiedCofdm.partialReception;
			
			BTFE_P_ChipSetRegs(ret_val, chip_handle, ixHI_UNIFIED_COFDM_CONFIG_AREA, 29, tempBuffer);
#endif
		
			break;
		default:
			BDBG_ERR(("BTFE_P_ScdStartFat: invalid modulation group"));
			return SCD_RESULT__CHIP_ERROR;
			break;
		}

			
	/* ---------------------------------------------------------------------------------------------
	 * Pilot / Video Carrier PLL initializations
     *  
	 --------------------------------------------------------------------------------------------- */

	if (modulationGroup == MODULATION_GROUP__ANALOG)
	{
		/* Configure Video Carrier Recovery */

		BTFE_P_SetVideoCarrierSweepFrequencies(chip_handle, tchip->IfFrequencyShift);
	}

	/* ---------------------------------------------------------------------------------------------
	 * Misc initializations
     *  
	 --------------------------------------------------------------------------------------------- */
	
	/* Packet de-jittering function of digital demods */

	if (modulationGroup != MODULATION_GROUP__ANALOG)
	{
		ret_val = BTFE_P_ChipConfigSmoother(ret_val, chip_handle);
	}

		/* Configure IF Demod firmware control register - modulation type  */

		if (modulationGroup == MODULATION_GROUP__ANALOG)
		{
			ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_ANALOG_VIDEO_STANDARD, (uint8_t) analogStandardType);
		}

		/* ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_VIDEO_SUBMODE_SELECT, videoSubmodeMask); */
		
	/* ---------------------------------------------------------------------------------------------
	 * Start the 8051 Firmware 
     *  
	 --------------------------------------------------------------------------------------------- */

		/* ---------------------------------------------------------------------------------------------
		 * UAGC Go Bit
		 *   AGC_CONTROL 0xf101
		 *   go_bit_agc 2 0x0 Static
		 *   0 - AGC halt  1 - Normal Operation
		 --------------------------------------------------------------------------------------------- */
		ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixAGC_CONTROL, &temp8);
		MODIFYFLD(temp8, AGC_CONTROL, go_bit_agc, 0x01); 
		ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixAGC_CONTROL, temp8);

		/* This is probably related to re-clocking the AGC Delta-Sigma Output. */
		/* MISC_AGC_SYNC_START    [W]    8 bits    Access: 8    primaryRegisterAperture:0xf063  
			agc_synchronizer_start 0 0x0
			0 - External AGC synchronizer is off(Default)
			1 - External AGC synchronizer is on
		*/
		ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_AGC_SYNC_START, 0x00);
		BKNI_Sleep(1); 
		ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_AGC_SYNC_START, 0x01);

		
		/* start uagc ASM, 0x04 in ixHI_VIDEO_MODE_SELECT is a flag to start uagc ASM */
		ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_VIDEO_MODE_SELECT, (videoModeMask|AGC_START_MASK));

		/* ---------------------------------------------------------------------------------------------
		 * This delay is to make sure the signal at the A/D (or IF amp input?) is
		 *   stable before demodulator is started.  Otherwise the 
		 *   demodulator might give a bas lock indication on channel change or scan.
		 *   This wait time for AGC based on worst case tuner
		 --------------------------------------------------------------------------------------------- */

		AGC_delay = (modulationGroup == MODULATION_GROUP__ANALOG) ? 40 : tchip->Acquisition.agcDelay;

		if(AGC_delay != 0)
		{
			/* allow the AGC to settle */
			BKNI_Sleep(AGC_delay); 
		}
	
		/* the FW waits for ixHI_VIDEO_MODE_SELECT to be set to start the demodulation firmware */

		if((ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_VIDEO_MODE_SELECT, videoModeMask)) == SCD_RESULT__OK)
		{
			/* The SCD has set the TFE mode.  Wait until the 8051 TFE controller recognizes the mode

					#define ixHI_VIDEO_STATUS			ixUC_GP_1
					#define	VIDEO_STATUS_MODE_MASK		0xF8
					
					typedef enum
					{
   						VIDEO_MODE_VSB = 1,
    						VIDEO_MODE_64QAM,
    						VIDEO_MODE_256QAM,
    						VIDEO_MODE_J83ABC,
    						VIDEO_MODE_NTSC,
    						VIDEO_MODE_COFDM,
    						VIDEO_MODE_UCOFDM,
    						VIDEO_MODE_DVBT,
    						VIDEO_MODE_ISDBT
					} VideoStatusMode;

					#define	VIDEO_STATUS_LOSS_DET		0x02
					#define	VIDEO_STATUS_LOCK_DET		0x01
			*/

			timeout = DEMOD_STATUS_COUNTER;

			while(timeout--)
			{
				if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixHI_VIDEO_STATUS, &video_status)) == SCD_RESULT__OK)
				{
					if((video_status & VIDEO_STATUS_MODE_MASK) == videoModeMask) 
								  break;
				}

				BKNI_Sleep(1); 
			}

			if(timeout == 0)
			{
				BDBG_ERR(("BTFE_P_ScdStartFat: error in set HI_VIDEO_STATUS"));
				return SCD_RESULT__CHIP_ERROR;
			}
		}

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipStopFat(SCD_RESULT ret_val, SCD_HANDLE chip_handle)
{
    uint32_t timeout;
    uint8_t video_status, agc_control;

	/* mcg Nov 2009 - It appears that by the time the FW state machines get a stop command,
	                 the IF AGC has moved toward the max gain rail.  Probably the
	                 FW has somehow reconfigured the chip so the IF AGC power detectors
	                 no longer see the input signal. So the IF AGC Stop setting will be 
	                 moved here until the FW sequencing can be figured out.
	*/

	if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixUAGC_CONTROL_1, &agc_control)) == SCD_RESULT__OK)
	{
		MODIFYFLD(agc_control, UAGC_CONTROL_1, irf_freeze_vif, 1);
		ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixUAGC_CONTROL_1, agc_control);
	}

	/* mcg Nov 2009 - It appears that by the time the FW state machines get a stop command,
	                 the CVBS OUTPUT is frozen at a random DC level (wherever it was

	                 here until the FW sequencing can be figured out.
	*/

	ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixNTSC_GAIN_ADJUST_FOR_DAC, 0x00);

	/* Setting the Video Mode to 0 causes the FW to stop */

    if((ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_VIDEO_MODE_SELECT, 0x00)) == SCD_RESULT__OK)
    {
        timeout = DEMOD_STATUS_COUNTER;

        while(timeout--)
        {
            if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixHI_VIDEO_STATUS, &video_status)) == SCD_RESULT__OK)
            {
                if(video_status == 0) break;
            }

            BKNI_Sleep(2); 
        }

        if(timeout == 0)
        {
		      BDBG_ERR(("BTFE_P_ScdStartFat: error in clear HI_VIDEO_STATUS"));
            return SCD_RESULT__CHIP_ERROR;
        }
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetBert(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_CONFIG__BERT *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    CHIP *tchip = GET_CHIP(chip_handle);
    uint8_t temp8;

    (void) handle;

    if(size != sizeof(SCD_CONFIG__BERT))
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    tchip->BertInput = data->InputSelect;

    if(data->ON_Flag) /* QQQ */
        ret_val = BTFE_P_ChipPower(ret_val, chip_handle, SCD_FE_POWER_UP_FAT | /* SCD_FE_POWER_UP_FDC |*/ SCD_FE_POWER_UP_BERT);
    else
        ret_val = BTFE_P_ChipPower(ret_val, chip_handle, SCD_FE_POWER_UP_FAT /* | SCD_FE_POWER_UP_FDC */);

    /* reset bert */
    ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, &temp8);
    MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_bert, 1);
    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, temp8);
    MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_bert, 0);
    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, temp8);

    /* set bert window size */
    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixBERT_CTL_A+3, (uint8_t) (data->WindowSize & 0xFF));

    /* set Sync Error Threshold */
    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixBERT_CTL_A+8, (uint8_t) (data->SyncErrorThreshold & 0xFF));

    /* set Sync Acquire Counter and Sync Loss Counter */
    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixBERT_CTL_A+9, (uint8_t)(((data->SyncAcquireCounter & 0x0F)<<4) | (data->SyncLossCounter & 0x0F)));

    /* set remaining in BERT_CTL_A reg */
    ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixBERT_CTL_A, &temp8);
    MODIFYFLD(temp8, BERT_CTL_A, hd_rm, data->HeaderRemoval);
    MODIFYFLD(temp8, BERT_CTL_A, in_sel, data->InputSelect);
    MODIFYFLD(temp8, BERT_CTL_A, pn_sel, !data->PN_Selection);
    MODIFYFLD(temp8, BERT_CTL_A, pn_inv, data->PN_Inversion);
    MODIFYFLD(temp8, BERT_CTL_A, on_flag, data->ON_Flag);
    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixBERT_CTL_A, temp8);

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetGpio(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_CONFIG__GPIO *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    uint8_t  gpioConfig[5];

    (void) handle;

    if(size != sizeof(SCD_CONFIG__GPIO))
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    gpioConfig[0] = (uint8_t) (data->ownershipMask & 0xFF);
    gpioConfig[1] = (uint8_t) ((data->inputMask & 0xff00) >> 8);
    gpioConfig[2] = (uint8_t) (data->inputMask & 0xff);
    gpioConfig[3] = (uint8_t) ((data->outputType & 0xff00) >> 8);
    gpioConfig[4] = (uint8_t) (data->outputType & 0xff);

    ret_val = BTFE_P_ChipDoMicroService(ret_val, chip_handle, SVC_GPIO_CONFIG, 5, gpioConfig, 0, NO_RESULTS, GPIO_TIMEOUT);

    return ret_val;
}

static SCD_RESULT BTFE_P_ChipGetMemoryRead(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_STATUS__MEMORY_READ *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
	uint32_t instance;

    (void) handle;
	(void) chip_handle;

    if(size != sizeof(SCD_STATUS__MEMORY_READ))
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }
	BKNI_Memset(&(data->values), 0, data->size);

	/* BTFE_P_HalReadChip is called instead of BTFE_P_ChipGetRegs since it will be modified to have multibyte reg access go through 8051*/
    if((ret_val = BTFE_P_ScdGetInstance(handle, &instance)) == SCD_RESULT__OK)
    {
        ret_val = BTFE_P_HalReadChip(instance, 0, data->offset, data->size, data->values);
    }


    return ret_val;
}

static SCD_RESULT BTFE_P_ChipGetAcquireMode(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_STATUS__ACQUIRE_MODE *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
	CHIP *tchip = GET_CHIP(chip_handle);

    (void) handle;

    if(size != sizeof(SCD_STATUS__ACQUIRE_MODE))
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

	data->acquireMode = tchip->Acquisition.acqConfig;

    return ret_val;
}

static SCD_RESULT BTFE_P_ChipGetJ83abcStatus(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_STATUS__J83ABC *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    uint8_t buffer[28];

    (void) handle;

    if(size != sizeof(SCD_STATUS__J83ABC))
        return SCD_RESULT__ARG_OUT_OF_RANGE;

    if ((ret_val = BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_J83_STATUS_AREA, 28, buffer)) != SCD_RESULT__OK)
         return SCD_RESULT__CHIP_READ_ERROR;

    data->reacqCounter = buffer[0];
    data->acqConfig = (SCD_ACQUIRE_CONFIG)buffer[1];
    data->bandwidthConfig = (SCD_BAND_WIDTH)buffer[2];
    data->bSpectralInversion = buffer[3] ? true : false;
    data->mode = (SCD_J83ABC_MODE)buffer[4];
    data->constellation = (SCD_QAM_MODE_DIGITAL)buffer[5];            
    data->bOperationDone = buffer[12] ? true : false;
    data->bSignalDetected = buffer[13] ? true : false;
    data->bandEdgePosPower = (int8_t)buffer[14];
    data->bandEdgeNegPower = (int8_t)buffer[15];
    data->IFNomRate = (uint32_t)((buffer[16] << 24) | (buffer[17] << 16) | (buffer[18] << 8) | buffer[19]);
    data->baudRateDetected = (uint32_t)((buffer[20] << 24) | (buffer[21] << 16) | (buffer[22] << 8) | buffer[23]);
    data->rateNomFinal = (uint32_t)((buffer[24] << 24) | (buffer[25] << 16) | (buffer[26] << 8) | buffer[27]);

    return ret_val;
}

static SCD_RESULT BTFE_P_ChipGetVsbStatus(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_STATUS_VSB *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    uint8_t buffer[10];

    (void) handle;

    if(size != sizeof(SCD_STATUS_VSB))
        return SCD_RESULT__ARG_OUT_OF_RANGE;

    if ((ret_val = BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_VSB_STATUS_AREA, 10, buffer)) != SCD_RESULT__OK)
         return SCD_RESULT__CHIP_READ_ERROR;
       
    data->bOperationDone = buffer[8] ? true : false;
    data->bConfirmVSB = buffer[9] ? true : false;

    return ret_val;
}

static SCD_RESULT BTFE_P_ChipGetCofdmStatus(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_STATUS_COFDM *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    uint8_t buffer[10];

    (void) handle;

    if(size != sizeof(SCD_STATUS_COFDM))
        return SCD_RESULT__ARG_OUT_OF_RANGE;

    if ((ret_val = BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_UNIFIED_COFDM_STATUS_AREA+60, 9, buffer)) != SCD_RESULT__OK)
         return SCD_RESULT__CHIP_READ_ERROR;
       
    data->bOperationDone = buffer[0] ? true : false;
    data->scanResult= buffer[1];
    data->bSpectraInverted = buffer[2] ? true : false;
    data->carrierOffset= (int32_t)((buffer[3] << 24) | (buffer[4] << 16) | (buffer[5] << 8) | buffer[6]);			
    data->timingOffset=  (int16_t)((buffer[7] << 8) | buffer[8]);	
    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetFatData(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_CONFIG__FAT_DATA *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    CHIP *tchip = GET_CHIP(chip_handle);
    uint8_t temp8;
    uint8_t mpeg_dfmt_pol_3_0;
    uint8_t mpeg_dfmt_1_0;

    (void) handle;

    if(size != sizeof(SCD_CONFIG__FAT_DATA))
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    BKNI_Memcpy((void *)&tchip->FatData, data, sizeof(SCD_CONFIG__FAT_DATA));

    ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_OUTPUT_FMT_CNTRL_1, &temp8);
    MODIFYFLD(temp8, MISC_OUTPUT_FMT_CNTRL_1, mpeg_output_enable, (data->MpegOutputEnable == 0));
    MODIFYFLD(temp8, MISC_OUTPUT_FMT_CNTRL_1, mpeg_dis_pkt_hdr, (data->HeaderEnable == 0));
    mpeg_dfmt_pol_3_0 = 0;
    mpeg_dfmt_pol_3_0 |= (data->clockPolarity == SCD_SIGNAL_POLARITY__INVERT) ? MPEG_DFMT_POL_3_0__INV_CLK   : 0;
    mpeg_dfmt_pol_3_0 |= (data->errorPolarity == SCD_SIGNAL_POLARITY__INVERT) ? MPEG_DFMT_POL_3_0__INV_ERROR : 0;
    mpeg_dfmt_pol_3_0 |= (data->syncPolarity  == SCD_SIGNAL_POLARITY__INVERT) ? MPEG_DFMT_POL_3_0__INV_SYNC  : 0;
    mpeg_dfmt_pol_3_0 |= (data->dataPolarity  == SCD_SIGNAL_POLARITY__INVERT) ? MPEG_DFMT_POL_3_0__INV_DATA  : 0;
    MODIFYFLD(temp8, MISC_OUTPUT_FMT_CNTRL_1, mpeg_dfmt_pol_3_0, mpeg_dfmt_pol_3_0);
    mpeg_dfmt_1_0 = 0;
    mpeg_dfmt_1_0 |= data->ParallelOutputEnable ? 0 : MPEG_DFMT_1_0__BIT;
    mpeg_dfmt_1_0 |= data->GatedClockEnable     ? MPEG_DFMT_1_0__GATE : 0;
    MODIFYFLD(temp8, MISC_OUTPUT_FMT_CNTRL_1, mpeg_dfmt_1_0, mpeg_dfmt_1_0);
    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_OUTPUT_FMT_CNTRL_1, temp8);

    ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_OUTPUT_FMT_CNTRL_0, &temp8);
/*
    MODIFYFLD(temp8, MISC_OUTPUT_FMT_CNTRL_0, mpeg_output_format_sel, 1);
*/
    MODIFYFLD(temp8, MISC_OUTPUT_FMT_CNTRL_0, mpeg_flip_bit7_bit0, (data->FlipOrder != 0));
    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_OUTPUT_FMT_CNTRL_0, temp8);

    ret_val = BTFE_P_ChipConfigSmoother(ret_val, chip_handle);

/*
    if(data->CableCardBypassEnable)
        BTFE_P_ChipModifyReg8(ret_val, chip_handle, ixMISC_TEST_REGISTER, CABLE_CARD_SELECT, CABLE_CARD_SELECT);
    else
        BTFE_P_ChipModifyReg8(ret_val, chip_handle, ixMISC_TEST_REGISTER, CABLE_CARD_SELECT, 0);
*/

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetFatAgc(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_CONFIG__FAT_AGC *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    BSTD_UNUSED(chip_handle);
    BSTD_UNUSED(data);

    (void) handle;

    if(size != sizeof(SCD_CONFIG__FAT_AGC))
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetAgcScript(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_CONFIG__AGC_SCRIPT *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    CHIP *tchip = GET_CHIP(chip_handle);
    int32_t *datapointer;
    BSTD_UNUSED(handle);

    if(size != sizeof(SCD_CONFIG__AGC_SCRIPT))
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(data->pdata == 0)
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    /* Parse through the data, and set the pointers appropriately */
    datapointer = data->pdata;

    while((*datapointer != -UAGC_SETUP_DONE ) && (ret_val == SCD_RESULT__OK))
    {
        if(*datapointer < 0)
        {
            if(*datapointer > -ATS_AGC_MAX)
            {
               /* ret_val = BTFE_P_ChipProcessATData(ret_val, chip_handle, datapointer); */
		         BDBG_MSG(("BTFE_P_ChipSetAgcScript: %d", *datapointer));		
            }
            else
            {
                switch(*datapointer)
                {
                case -IF_DEMOD:                tchip->pIFDconfig = datapointer+1;         break;
                case -UAGC_IF_VGA:             tchip->pUAGC_IF_VGAconfig = datapointer+1; break;
                case -UAGC_DIGITAL_MODULATION: tchip->pUAGCdigitalConfig = datapointer+1; break;
                case -UAGC_ANALOG_NEGATIVE_MODULATION: tchip->pUAGCanalogNegConfig = datapointer+1; break;
                case -UAGC_ANALOG_POSITIVE_MODULATION: tchip->pUAGCanalogPosConfig = datapointer+1; break;

                default:                    
                   BDBG_MSG(("BTFE_P_ChipSetAgcScript: %d", *datapointer));
                   break;
                }
            }
        }

        datapointer++;
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetIsdbtBuffer(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_CONFIG__ISDBT_BUFFER *data, uint32_t size)
{ 
   SCD_RESULT ret_val = SCD_RESULT__OK; 
   uint32_t isdbt_buffer, alignment;
   uint8_t temp8;
   
   BSTD_UNUSED(handle);   
   BDBG_ASSERT(data);
   BDBG_ASSERT(size == sizeof(SCD_CONFIG__ISDBT_BUFFER));
   
   isdbt_buffer = (uint32_t) data->bufferPtr;
   alignment = (uint32_t) data->alignment;
   if ((isdbt_buffer & (alignment - 1)) != 0)
   {
   	BDBG_ERR(("BTFE_P_ChipSetIsdbtBuffer: buffer alignment error, buffer 0x%s", isdbt_buffer));
   	return SCD_RESULT__ARG_OUT_OF_RANGE;
   }

   temp8 = (uint8_t) ((isdbt_buffer >> 27) & 0x1F); /* ms 5 bits */
   ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_MEM_READ_MARGIN_28, temp8);
   temp8 = (uint8_t) ((isdbt_buffer >> 22) & 0x1F); /* ls 5 bits */
   ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_MEM_READ_MARGIN_29, temp8);
   return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetAudioMagShift(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_CONFIG__AUDIO_MAG_SHIFT *data, uint32_t size)
{
   static uint8_t BTFE_AUDIO_MAGSHIFT[8] = 
   {
      0x04, 
      0x05, 
      0x06, 
      0x07, 
      0x00, 
      0x01, 
      0x02, 
      0x03
   };
   
   SCD_RESULT ret_val = SCD_RESULT__OK;
   /* CHIP *tchip = GET_CHIP(chip_handle); */   
   uint8_t mag_shift_idx, mag_shift;
   
   BSTD_UNUSED(handle);   
   BDBG_ASSERT(data);
   BDBG_ASSERT(size == sizeof(SCD_CONFIG__AUDIO_MAG_SHIFT));
   
   mag_shift_idx = (uint8_t)*data;
   if (mag_shift_idx >= 8)
      mag_shift_idx = 7;
   mag_shift = BTFE_AUDIO_MAGSHIFT[mag_shift_idx];
      
   /* write mag_shift value to the 8051 */
   ret_val = BTFE_P_ChipDoMicroService(ret_val, chip_handle, SVC_SET_AUDIO_CONFIG, 1, &mag_shift, 0, NO_RESULTS, ATRACKING_TIMEOUT);

   return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetIF(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_XPROP_TUNER_IF__DATA *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    CHIP *tchip = GET_CHIP(chip_handle);
    BSTD_UNUSED(handle);
    BSTD_UNUSED(size);

    tchip->tunerIfFrequency = data->centerIF;
    tchip->IfFrequencyShift = data->IFshift;
    tchip->tunerSpectrum = data->spectrumInv;

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetChannelScanControl(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_CONFIG__CHANNEL_SCAN_CONTROL *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    CHIP *tchip = GET_CHIP(chip_handle);

    (void) chip_handle;
    (void) handle;
    (void) data;

    if(size != sizeof(SCD_CONFIG__CHANNEL_SCAN_CONTROL))
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    BKNI_Memcpy((void *)&tchip->ChannelScanControl, data, sizeof(SCD_CONFIG__CHANNEL_SCAN_CONTROL));

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetAcquisition(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_CONFIG__ACQUISITION *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    /* uint8_t tune_config; */
    CHIP *tchip = GET_CHIP(chip_handle);

    (void) chip_handle;
    (void) handle;
    (void) data;

    if(size != sizeof(SCD_CONFIG__ACQUISITION))
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    BKNI_Memcpy(&(tchip->Acquisition), data, sizeof(SCD_CONFIG__ACQUISITION));

    return ret_val;
}


/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetJ83Abc(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_CONFIG__J83ABC *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    CHIP *tchip = GET_CHIP(chip_handle);
    BSTD_UNUSED(handle);

    if(size != sizeof(SCD_CONFIG__J83ABC))
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    BKNI_Memcpy(&(tchip->j83abc), data, sizeof(SCD_CONFIG__J83ABC));

    return ret_val;
}

#if 0
/****************************************************************************/

static SCD_RESULT BTFE_P_ChipTryNextScan(SCD_HANDLE chip_handle, SCD_HANDLE handle, int32_t *data, uint32_t size)
{
   SCD_RESULT ret_val=SCD_RESULT__OK;
   CHIP *tchip = GET_CHIP(chip_handle);
   uint32_t symbolRate, Q_hi, Q_lo, P_hi, P_lo, i;
	uint8_t timingNominalRate[6], tempBuffer[13];
	int32_t carrierOffset;
	uint32_t x;
	uint32_t defaultIFNomRate;
   
   BSTD_UNUSED(handle);
   BSTD_UNUSED(size);
   
   carrierOffset = *data;
   
   /* update j83abcConfig_t struct in the 8051 */
   symbolRate = (uint32_t) tchip->j83abc.symbolRate;
   BTFE_MultU32U32(13500000UL * 256, 2147483648UL, &P_hi, &P_lo);  /* P = [P_hi,P_lo] = 13.5e6 * 2^39 = (13.5e6*2^8)*2^31 */
   BTFE_DivU64U32(P_hi, P_lo, symbolRate, &Q_hi, &Q_lo); /* Q = [Q_hi,Q_lo] = P/symbolRate */

   timingNominalRate[0] =(uint8_t)((Q_hi >> 8) & 0xFF);
   timingNominalRate[1] = (uint8_t)(Q_hi & 0xFF);
   timingNominalRate[2] = (uint8_t)((Q_lo >> 24) & 0xFF);
   timingNominalRate[3] = (uint8_t)((Q_lo >> 16) & 0xFF);
   timingNominalRate[4] = (uint8_t)((Q_lo >> 8) & 0xFF);
   timingNominalRate[5] = (uint8_t)(Q_lo & 0xFF);
   
   tempBuffer[0] = tchip->j83abc.j83abcMode;
   tempBuffer[1] = tchip->j83abc.qamMode;
   tempBuffer[2] = (uint8_t)((tchip->j83abc.symbolRate >> 24) & 0xFF);
   tempBuffer[3] = (uint8_t)((tchip->j83abc.symbolRate >> 16) & 0xFF);
   tempBuffer[4] = (uint8_t)((tchip->j83abc.symbolRate >> 8) & 0xFF);
   tempBuffer[5] = (uint8_t)(tchip->j83abc.symbolRate & 0xFF);
   for (i = 0; i < 6; ++i)
      tempBuffer[i+6] = timingNominalRate[i];
   tempBuffer[12] = 0; /* clear waitOnStart */   
   ret_val = BTFE_P_ChipSetRegs(ret_val, chip_handle, ixHI_J83_CONFIG_AREA, 13, tempBuffer);   
   if (ret_val != SCD_RESULT__OK)
      return ret_val;
   
   /* clear operationDone in j83abcStatus struct */
   ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_J83_STATUS_AREA+12, 0);
   
   /* update part of asmConfig_t struct in the 8051 (i'm assuming defaultIFNomRate doesn't change) */
   tempBuffer[0] = tchip->Acquisition.acqConfig;
   tempBuffer[1] = tchip->Acquisition.bandWidthConfig;
   tempBuffer[2] = tchip->Acquisition.bSpectrumInversion;

   defaultIFNomRate = tchip->defaultIFNomRate;


   if (carrierOffset != 0)
   {
   	if (carrierOffset > 0)
		x = (uint32_t) carrierOffset;
	else
		x = (uint32_t) (-carrierOffset);

	/* (x/54M) * 2**24, sampling rate is 54MHz */
	BTFE_MultU32U32(16777216, x, &P_hi, &P_lo);
	BTFE_DivU64U32(P_hi, P_lo, FE_CRYSTAL_FREQ, &Q_hi, &Q_lo);

	if (carrierOffset > 0)
		defaultIFNomRate += Q_lo; /* plus: receviver has positive carrier offset */
	else 
		defaultIFNomRate -= Q_lo; /* minus */

	/* printf("BTFE_P_ChipTryNextScan: carrierOffset %d, Q_hi 0x%x, Q_lo 0x%x, defaultIFNomRate 0x%x\n", carrierOffset, Q_hi, Q_lo, defaultIFNomRate); */
   }
   else
   {
   	/* printf("BTFE_P_ChipTryNextScan: carrierOffset %d, defaultIFNomRate 0x%x\n", carrierOffset, defaultIFNomRate); */
   }

   tempBuffer[3] = (uint8_t)(defaultIFNomRate >> 16);
   tempBuffer[4] = (uint8_t)(defaultIFNomRate >> 8);
   tempBuffer[5] = (uint8_t)(defaultIFNomRate >> 0);
   ret_val = BTFE_P_ChipSetRegs(ret_val, chip_handle, ixHI_ASM_CONFIG_AREA, 6, tempBuffer);   
   if (ret_val != SCD_RESULT__OK)
      return ret_val;

   return ret_val;
}


/****************************************************************************/

static SCD_RESULT BTFE_P_ChipJ83NextScan(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_CONFIG_J83_NEXT_SCAN *data, uint32_t size)
{
   SCD_RESULT ret_val=SCD_RESULT__OK;
   CHIP *tchip = GET_CHIP(chip_handle);
   uint32_t symbolRate, Q_hi, Q_lo, P_hi, P_lo, i;
	uint8_t timingNominalRate[6], tempBuffer[13];
	int32_t carrierOffset;
	uint32_t x;
	uint32_t defaultIFNomRate;
   
   BSTD_UNUSED(handle);
   BSTD_UNUSED(size);
   
   
   /* update j83abcConfig_t struct in the 8051 */
   symbolRate = data->symbolRate;
   BTFE_MultU32U32(13500000UL * 256, 2147483648UL, &P_hi, &P_lo);  /* P = [P_hi,P_lo] = 13.5e6 * 2^39 = (13.5e6*2^8)*2^31 */
   BTFE_DivU64U32(P_hi, P_lo, symbolRate, &Q_hi, &Q_lo); /* Q = [Q_hi,Q_lo] = P/symbolRate */

   timingNominalRate[0] =(uint8_t)((Q_hi >> 8) & 0xFF);
   timingNominalRate[1] = (uint8_t)(Q_hi & 0xFF);
   timingNominalRate[2] = (uint8_t)((Q_lo >> 24) & 0xFF);
   timingNominalRate[3] = (uint8_t)((Q_lo >> 16) & 0xFF);
   timingNominalRate[4] = (uint8_t)((Q_lo >> 8) & 0xFF);
   timingNominalRate[5] = (uint8_t)(Q_lo & 0xFF);
   
   tempBuffer[0] = data->j83abcMode;
   tempBuffer[1] = data->qamMode;
   tempBuffer[2] = (uint8_t)((symbolRate >> 24) & 0xFF);
   tempBuffer[3] = (uint8_t)((symbolRate >> 16) & 0xFF);
   tempBuffer[4] = (uint8_t)((symbolRate >> 8) & 0xFF);
   tempBuffer[5] = (uint8_t)(symbolRate & 0xFF);
   for (i = 0; i < 6; ++i)
      tempBuffer[i+6] = timingNominalRate[i];
   tempBuffer[12] = 0; /* clear waitOnStart */   
   ret_val = BTFE_P_ChipSetRegs(ret_val, chip_handle, ixHI_J83_CONFIG_AREA, 13, tempBuffer);   
   if (ret_val != SCD_RESULT__OK)
      return ret_val;
   
   /* clear operationDone in j83abcStatus struct */
   ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_J83_STATUS_AREA+12, 0);
   
   /* update part of asmConfig_t struct in the 8051 (i'm assuming defaultIFNomRate doesn't change) */
   tempBuffer[0] = data->acqMode;
   tempBuffer[1] = data->bandwidth;
   tempBuffer[2] = tchip->Acquisition.bSpectrumInversion;

   defaultIFNomRate = tchip->defaultIFNomRate;
   carrierOffset = data->carrierOffset;

/* printf("SCD: carrierOffset %d, acqMode %d, bandwidth %d, symRate %d j83bMode %d\n", 
	carrierOffset, data->acqMode, data->bandwidth, data->symbolRate, data->j83abcMode); */

   if (carrierOffset != 0)
   {
   	if (carrierOffset > 0)
		x = (uint32_t) carrierOffset;
	else
		x = (uint32_t) (-carrierOffset);

	/* (x/54M) * 2**24, sampling rate is 54MHz */
	BTFE_MultU32U32(16777216, x, &P_hi, &P_lo);
	BTFE_DivU64U32(P_hi, P_lo, FE_CRYSTAL_FREQ, &Q_hi, &Q_lo);

	if (carrierOffset > 0)
		defaultIFNomRate += Q_lo; /* plus: receviver has positive carrier offset */
	else 
		defaultIFNomRate -= Q_lo; /* minus */

	/* printf("BTFE_P_ChipTryNextScan: carrierOffset %d, Q_hi 0x%x, Q_lo 0x%x, defaultIFNomRate 0x%x\n", carrierOffset, Q_hi, Q_lo, defaultIFNomRate); */
   }
   else
   {
   	/* printf("BTFE_P_ChipTryNextScan: carrierOffset %d, defaultIFNomRate 0x%x\n", carrierOffset, defaultIFNomRate); */
   }

   tempBuffer[3] = (uint8_t)(defaultIFNomRate >> 16);
   tempBuffer[4] = (uint8_t)(defaultIFNomRate >> 8);
   tempBuffer[5] = (uint8_t)(defaultIFNomRate >> 0);
   ret_val = BTFE_P_ChipSetRegs(ret_val, chip_handle, ixHI_ASM_CONFIG_AREA, 6, tempBuffer);   
   if (ret_val != SCD_RESULT__OK)
      return ret_val;

   return ret_val;
}
#endif

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetPowerLevel(SCD_HANDLE chip_handle, SCD_HANDLE handle, int16_t *data, uint32_t size)
{
   SCD_RESULT ret_val = SCD_RESULT__OK; 
   uint16_t offset;
   int16_t val16;
   uint8_t commandBuffer[8];
   
   BSTD_UNUSED(handle);   
   BDBG_ASSERT(data);
   BDBG_ASSERT(size == sizeof(int16_t));

   offset = ixUC_GP_42; /* use UC_GP42 and UC_GP_43 to store the tuner power level */
   val16 = *data;
   
   commandBuffer[0] = (uint8_t) (offset >> 8);
   commandBuffer[1] = (uint8_t) (offset & 0xFF);
   commandBuffer[2] = (uint8_t) size;
   commandBuffer[3] = (uint8_t) (val16 >> 8);;
   commandBuffer[4] = (uint8_t) (val16 & 0xFF);;
      
   /* write mag_shift value to the 8051 */
   ret_val = BTFE_P_ChipDoMicroService(ret_val, chip_handle, SVC_CSECT_WRITE, 5, commandBuffer, 0, NO_RESULTS, I2C_TIMEOUT);

   return ret_val;
}


/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetROffset(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_CONFIG__RF_OFFSET *data, uint32_t size)
{
   SCD_RESULT ret_val=SCD_RESULT__OK;
   CHIP *tchip = GET_CHIP(chip_handle);
   uint32_t Q_hi, Q_lo, P_hi, P_lo;
   int32_t freqOffset;
   uint32_t x;
   uint32_t defaultIFNomRate = 0;
   uint32_t symbolRate;
   uint8_t bwFilter;
   
   BSTD_UNUSED(handle);
   BSTD_UNUSED(size);
   
   freqOffset = data->freqOffset;
   symbolRate = data->symbolRate;
   
   if (tchip->rfOffset.freqOffset != freqOffset) /* make change if different than last setup */
   {
	tchip->rfOffset.freqOffset = freqOffset;
   	if (freqOffset > 0)
		x = (uint32_t) freqOffset;
	else
		x = (uint32_t) (-freqOffset);

	/* (x/54M) * 2**24, sampling rate is 54MHz */
	BTFE_MultU32U32(16777216, x, &P_hi, &P_lo);
	BTFE_DivU64U32(P_hi, P_lo, FE_CRYSTAL_FREQ, &Q_hi, &Q_lo);

	if (freqOffset > 0)
		defaultIFNomRate += Q_lo; /* plus: receviver has positive carrier offset */
	else 
		defaultIFNomRate -= Q_lo; /* minus */

	ret_val = BTFE_P_ChipSetReg24(ret_val, chip_handle, ixFE_DC_NORMALIZED_IF_2, defaultIFNomRate);
   }

   if ((tchip->rfOffset.symbolRate != symbolRate) && (ret_val == SCD_RESULT__OK))
   {	
	tchip->rfOffset.symbolRate = symbolRate;
	
	if (symbolRate > 7000000) bwFilter = (uint8_t) SCD_BW_9MHZ;
	else if (symbolRate > 6100000) bwFilter = (uint8_t) SCD_BW_8MHZ;
	else if (symbolRate > 5220000) bwFilter = (uint8_t) SCD_BW_7MHZ;
	else if (symbolRate > 4350000) bwFilter = (uint8_t) SCD_BW_6MHZ;
	else if (symbolRate > 3480000) bwFilter = (uint8_t) SCD_BW_5MHZ;
	else if (symbolRate > 2610000) bwFilter = (uint8_t) SCD_BW_4MHZ;
	else if (symbolRate > 1740000) bwFilter = (uint8_t) SCD_BW_3MHZ;
	else  bwFilter = (uint8_t) SCD_BW_2MHZ;

	/* call 8051 to set the BW filter */
	ret_val = BTFE_P_ChipDoMicroService(ret_val, chip_handle, SVC_SET_BW_FILTER, 1, &bwFilter, 0, NO_RESULTS, DEFAULT_TIMEOUT);
   }

   return ret_val;
}


/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetPowerSaving(SCD_HANDLE chip_handle, SCD_HANDLE handle, int32_t *data, uint32_t size)
{
   SCD_RESULT ret_val=SCD_RESULT__OK;
   CHIP *tchip = GET_CHIP(chip_handle);
   uint8_t temp8;
   int temp = 0;
   bool bPowerSaving;
   
   BSTD_UNUSED(handle);
   BSTD_UNUSED(size);
   
   bPowerSaving = (bool) *data;
   
   if (bPowerSaving == 1) /* Power down */
   {
        temp |= BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_3, &temp8);
        tchip->power_control_3 = temp8;	
        temp8 = 0xff;
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_3, temp8);

        temp |= BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_0, &temp8);
        tchip->power_control_0 = temp8;	
        MODIFYFLD(temp8, MISC_POWER_CONTROL_0, iir_4_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_0, iir_3_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_0, iir_2_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_0, iir_1_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_0, iir_0_power_down, 1);		
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_0, temp8);

	 /* DFE Eq */
        MODIFYFLD(temp8, MISC_POWER_CONTROL_0, eq_complete_power_down, 1);
        MODIFYFLD(temp8, MISC_POWER_CONTROL_0, rest_of_eq_power_down, 1);
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_0, temp8);

	 /* DFE serdes */
        temp |= BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_1, &temp8);
        tchip->power_control_1 = temp8;	
        MODIFYFLD(temp8, MISC_POWER_CONTROL_1, serdeser_power_down, 1);		
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_1, temp8);	

	 /* DFE agc */
        MODIFYFLD(temp8, MISC_POWER_CONTROL_1, agc_power_down, 1);		
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_1, temp8);			

	 /* DFE fec */
        temp |= BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_2, &temp8);
        tchip->power_control_2 = temp8;		 
        MODIFYFLD(temp8, MISC_POWER_CONTROL_2, fec_power_down, 1);		
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_2, temp8);	

	 /* DFE ofs */	 
        MODIFYFLD(temp8, MISC_POWER_CONTROL_2, ofs_pwr_down, 1);		
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_2, temp8);	

	 /* DFE dvbt */	 
        MODIFYFLD(temp8, MISC_POWER_CONTROL_2, dvbt_power_down, 1);		
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_2, temp8);	

	 /* DFE ntsc */	 
        MODIFYFLD(temp8, MISC_POWER_CONTROL_2, ntsc_power_down, 1);		
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_2, temp8);	
        ret_val = (SCD_RESULT)temp;

	 /* DFE bert */	 
        MODIFYFLD(temp8, MISC_POWER_CONTROL_2, bert_power_down, 1);		
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_2, temp8);	

	 /* DFE psd */	 
        MODIFYFLD(temp8, MISC_POWER_CONTROL_2, psd_power_down, 1);		
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_2, temp8);	

	 /* DFE fdc */	 
        MODIFYFLD(temp8, MISC_POWER_CONTROL_2, fdc_power_down, 1);		
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_2, temp8);	

	 /* DFE general */	 
        MODIFYFLD(temp8, MISC_POWER_CONTROL_2, fiji_power_down, 1);		
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_2, temp8);	
   }
   else /* Power up */
   {		
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_2, tchip->power_control_2);
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_1, tchip->power_control_1);
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_0, tchip->power_control_0);
        temp |= BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_POWER_CONTROL_3, tchip->power_control_3);
   }

   ret_val = (SCD_RESULT)temp;
   if(ret_val != SCD_RESULT__OK)
        BDBG_ERR(("BTFE_P_ChipSetPowerSaving: error=%u", ret_val));

   return ret_val;
}


/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetUnifiedCofdm(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_CONFIG__UNIFIED_COFDM *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    CHIP *tchip = GET_CHIP(chip_handle);
    BSTD_UNUSED(handle);

    if(size != sizeof(SCD_CONFIG__UNIFIED_COFDM))
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    BKNI_Memcpy(&(tchip->unifiedCofdm), data, sizeof(SCD_CONFIG__UNIFIED_COFDM));

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipGetAgcIndicator(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_STATUS__AGC_INDICATOR *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    uint16_t temp16;
    uint8_t  temp8;
    /* CHIP *tchip = GET_CHIP(chip_handle); */

    if(size != sizeof(SCD_STATUS__AGC_INDICATOR))
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    /* set initial values */
    data->SDM1 = 0;
    data->SDM2 = 0;
    data->SDMX = 0;
    data->InternalAGC = 0;
    data->AdcMin = 0;
    data->AdcMax = 0;
    data->AdcPower = 0;
    data->PdetPower = 0;
	data->VidPower = 0;
	data->vdcLevel = 0;

    if(GET_HANDLE_TYPE(handle) == SCD_HANDLE_FAT)
    {
        if(data->Flags & SCD_STATUS_AGC__SDM1)
        {
		/* UAGC_VIF_K U[-1:16]  bytes are not in correct order !!!! */
		if((ret_val = BTFE_P_ChipGetReg16_LE(ret_val, chip_handle, ixUAGC_VIF_K_0, &temp16)) == SCD_RESULT__OK)
		{
			BTFE_P_ChipGetReg8(ret_val, chip_handle, ixUAGC_CONTROL_1, &temp8);
			if (VAL2FLD(UAGC_CONTROL_1,irf_invertVgaDeltaSigmaIF,temp8))
				data->SDM1 = 0xffff - temp16;
			else
				data->SDM1 = temp16;
		}
        }

        if(data->Flags & SCD_STATUS_AGC__ADC_MIN)
        {
            if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixUAGC_ADC_DATA_MIN, &temp8)) == SCD_RESULT__OK)
            {
                if(temp8 >= 0x80)
                    data->AdcMin = temp8 - 0x100;
                else
                    data->AdcMin = temp8;
            }
        }

        if(data->Flags & SCD_STATUS_AGC__ADC_MAX)
        {
            if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixUAGC_ADC_DATA_MAX, &temp8)) == SCD_RESULT__OK)
            {
                if(temp8 >= 0x80)
                    data->AdcMax = temp8 - 0x100;
                else
                    data->AdcMax = temp8;
            }
        }

        if(data->Flags & SCD_STATUS_AGC__ANALOG_PVID)
        {
            if((ret_val = BTFE_P_ChipGetReg16(ret_val, chip_handle, ixHI_UAGC_PVID_DBM_1, &temp16)) == SCD_RESULT__OK)
            {
                if(temp16 >= 0x8000)
                    data->VidPower = temp16 - 0x10000;
                else
                    data->VidPower = temp16;
            }
        }

        if((ret_val = BTFE_P_ChipGetReg16(ret_val, chip_handle, ixUAGC_VDC_K_1, &temp16)) == SCD_RESULT__OK)
        {
                data->vdcLevel = temp16;
        }
    }
    else
    {
        ret_val = SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipGetFat(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_STATUS__FAT *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    uint8_t temp8;
    /* uint8_t tmccReadRev1 = 0, tmccReadRev2 = 0, tmccReadRev3 = 0, tmccReadRev4 = 0, tmccReadRev5 = 0; */
    uint16_t temp16;
    uint32_t temp32;
    uint8_t acb_buffer[SIZE_ACB];
    uint8_t acb_args[1];
    uint8_t rs_buffer[64];
    CHIP *tchip = GET_CHIP(chip_handle);
    bool analog = 0;
    uint8_t cofdm = 0;
    /* uint8_t ber_ctrl; */
    uint32_t flags;
    uint8_t  Gi_exp;         /* IAGC gain Gi = Gi_mantissa * 2**Gi_exp */
    int32_t Gi_mantissa;     /* IAGC gain Gi = Gi_mantissa * 2**Gi_exp */
    int32_t Gi;              /* IAGC gain Gi = Gi_mantissa * 2**Gi_exp */
	 int32_t snr;             /* FAT SNR used for SQI calculation, various fixed point formats */
	 int32_t Prf_dBm;         /* FAT RF input power estimate in dBm   S+[29:-1] */
	 int32_t temp32signed;
	 bool  lock_status;
	 int layer;
	 uint8_t tempBuffer[32];
	/*uint8_t videoSubmodeMask = 0;*/

    if(size != sizeof(SCD_STATUS__FAT))
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    flags = data->Flags;         /* save the flags (which status variables App wants) */
    BKNI_Memset(data, 0, size);  /* clear status structure so zeros will be returned in variables not requested by App) */ 
    data->Flags = flags;         /* put the flags back in status structure */

    data->LockStatus = SCD_LOCK_STATUS__UNLOCKED;
    data->DemodulationFormat = SCD_MOD_FORMAT__UNKNOWN;
    data->RecommendedTimeoutValue = 1000;
    data->SpectrumPolarity = SCD_SIGNAL_POLARITY__NO_INVERT;
    data->DemodInputReference = tchip->ADCRefMilliVolt;

    if(GET_HANDLE_TYPE(handle) == SCD_HANDLE_FAT)
    {
		/* 1. Some flags might not be set by App, but the associated variable is needed to find othe status variables
			which are being requested */

        if(data->Flags & SCD_STATUS_FAT__EQUALIZER_SNR)
        {
            data->Flags |= SCD_STATUS_FAT__DEMOD_FORMAT;
        }

        if(data->Flags & SCD_STATUS_FAT__CARRIER_OFFSET)
        {
            data->Flags |= SCD_STATUS_FAT__DEMOD_FORMAT;
        }

        if(data->Flags & SCD_STATUS_FAT__COARSE_OFFSET)
        {
            data->Flags |= SCD_STATUS_FAT__DEMOD_FORMAT;
        }

        if(data->Flags & SCD_STATUS_FAT__PILOT_OFFSET)
        {
            data->Flags |= SCD_STATUS_FAT__DEMOD_FORMAT;
        }

		if(data->Flags & SCD_STATUS_FAT__ERRORS)
        {
            data->Flags |= SCD_STATUS_FAT__DEMOD_FORMAT;
        }

        if(data->Flags & SCD_STATUS_FAT__SQI)
        {
            data->Flags |= SCD_STATUS_FAT__DEMOD_FORMAT;
        }

        if(data->Flags & SCD_STATUS_FAT__DEMOD_FORMAT)
        {
            data->Flags |= SCD_STATUS_FAT__LOCK_STATUS;
        }

		/* 2. Get the status values */

        if(data->Flags & SCD_STATUS_FAT__LOCK_STATUS)
        {
            if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixHI_VIDEO_STATUS, &temp8)) == SCD_RESULT__OK)
            {
                if(temp8 & VIDEO_STATUS_LOCK_DET)
                {
                    data->LockStatus = SCD_LOCK_STATUS__LOCKED;
                }
            }
        }

        if(data->Flags & SCD_STATUS_FAT__DEMOD_FORMAT)
        {
            if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixHI_VIDEO_MODE_SELECT, &temp8)) == SCD_RESULT__OK)
            {
                temp8 = (uint8_t) (temp8 >> VIDEO_MODE_SHIFT);
                if (temp8 >= VIDEO_MODE_MAX)
                {
			BDBG_ERR(("GetFATstatus returning ERROR, temp8=%d !!!", temp8));
			return SCD_RESULT__ARG_OUT_OF_RANGE;
                }
		  switch (temp8)
		  {
		  	case VIDEO_MODE_VSB:
				data->DemodulationFormat = SCD_MOD_FORMAT__FAT_VSB;
				break;
#ifdef SCD_LEGACY_QAM
		  	case VIDEO_MODE_64QAM:
				data->DemodulationFormat = SCD_MOD_FORMAT__FAT_QAM64;
				break;
		  	case VIDEO_MODE_256QAM:
				data->DemodulationFormat = SCD_MOD_FORMAT__FAT_QAM256;
				break;
#endif
		  	case VIDEO_MODE_J83ABC:
				data->DemodulationFormat = SCD_MOD_FORMAT__FAT_J83ABC;
				break;
		  	case VIDEO_MODE_COFDM:
				data->DemodulationFormat = SCD_MOD_FORMAT__FAT_VSB;
				cofdm = 1;

				if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixHI_COFDM_MODE_SELECT, &temp8)) == SCD_RESULT__OK)
                    		{
                       	 	switch(temp8)
                        		{
                       	 	case COFDM_INPUT_5MHZ:   data->DemodulationFormat = SCD_MOD_FORMAT__FAT_COFDM_5;    break;
                       		case COFDM_INPUT_6MHZ:   data->DemodulationFormat = SCD_MOD_FORMAT__FAT_COFDM_6;    break;
                        		case COFDM_INPUT_7MHZ:   data->DemodulationFormat = SCD_MOD_FORMAT__FAT_COFDM_7;    break;
                        		case COFDM_INPUT_8MHZ:   data->DemodulationFormat = SCD_MOD_FORMAT__FAT_COFDM_8;    break;
                       	 	}
                       	 }
				break;
		  	case VIDEO_MODE_UCOFDM:
				data->DemodulationFormat = SCD_MOD_FORMAT__FAT_UNIFIED_COFDM;
				cofdm = 2;
				break;
		  	case VIDEO_MODE_NTSC:
                    		analog = 1;

                   		 if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixHI_ANALOG_VIDEO_STANDARD, &temp8)) == SCD_RESULT__OK)
                   		 {
                        		switch(temp8)
                       		{
                        			case VIDEO_STANDARD__NTSC_M:      data->DemodulationFormat = SCD_MOD_FORMAT__FAT_NTSC_M;    break;
                        			case VIDEO_STANDARD__NTSC_N:      data->DemodulationFormat = SCD_MOD_FORMAT__FAT_NTSC_N;    break;
                        			case VIDEO_STANDARD__NTSC_J:      data->DemodulationFormat = SCD_MOD_FORMAT__FAT_NTSC_J;    break;
                       			case VIDEO_STANDARD__NTSC_443:    data->DemodulationFormat = SCD_MOD_FORMAT__FAT_NTSC_443;  break;

                        			case VIDEO_STANDARD__PAL_I:       data->DemodulationFormat = SCD_MOD_FORMAT__FAT_PAL_I;     break;
                        			case VIDEO_STANDARD__PAL_B:       data->DemodulationFormat = SCD_MOD_FORMAT__FAT_PAL_B;     break;
                        			case VIDEO_STANDARD__PAL_B1:      data->DemodulationFormat = SCD_MOD_FORMAT__FAT_PAL_B1;    break;
                        			case VIDEO_STANDARD__PAL_G:       data->DemodulationFormat = SCD_MOD_FORMAT__FAT_PAL_G;     break;
                        			case VIDEO_STANDARD__PAL_H:       data->DemodulationFormat = SCD_MOD_FORMAT__FAT_PAL_H;     break;
                        			case VIDEO_STANDARD__PAL_D:       data->DemodulationFormat = SCD_MOD_FORMAT__FAT_PAL_D;     break;
                        			case VIDEO_STANDARD__PAL_60:      data->DemodulationFormat = SCD_MOD_FORMAT__FAT_PAL_60;    break;
                        			case VIDEO_STANDARD__PAL_M:       data->DemodulationFormat = SCD_MOD_FORMAT__FAT_PAL_M;     break;
                        			case VIDEO_STANDARD__PAL_N:       data->DemodulationFormat = SCD_MOD_FORMAT__FAT_PAL_N;     break;
                        			case VIDEO_STANDARD__PAL_N_COMBO: data->DemodulationFormat = SCD_MOD_FORMAT__FAT_PAL_NC;    break;

                        			case VIDEO_STANDARD__SECAM_B:     data->DemodulationFormat = SCD_MOD_FORMAT__FAT_SECAM_B;   break;
                        			case VIDEO_STANDARD__SECAM_D:     data->DemodulationFormat = SCD_MOD_FORMAT__FAT_SECAM_D;   break;
                        			case VIDEO_STANDARD__SECAM_G:     data->DemodulationFormat = SCD_MOD_FORMAT__FAT_SECAM_G;   break;
                        			case VIDEO_STANDARD__SECAM_H:     data->DemodulationFormat = SCD_MOD_FORMAT__FAT_SECAM_H;   break;
                        			case VIDEO_STANDARD__SECAM_K:     data->DemodulationFormat = SCD_MOD_FORMAT__FAT_SECAM_K;   break;
                        			case VIDEO_STANDARD__SECAM_K1:    data->DemodulationFormat = SCD_MOD_FORMAT__FAT_SECAM_K1;  break;
                        			case VIDEO_STANDARD__SECAM_L:     data->DemodulationFormat = SCD_MOD_FORMAT__FAT_SECAM_L;   break;
                        			case VIDEO_STANDARD__SECAM_L1:    data->DemodulationFormat = SCD_MOD_FORMAT__FAT_SECAM_L1;  break;
                        		}
                    		}
				break;
		  }
            }
        }

        if(data->Flags & SCD_STATUS_FAT__EQUALIZER_SNR)        
        {
            if (analog)
	     {
		  if((ret_val = BTFE_P_ChipGetReg16(ret_val, chip_handle, ixHI_UAGC_SNRVID_DB_1, &temp16)) == SCD_RESULT__OK)
                {
                    data->EqualizerSNR = temp16;
                }
	     }
	     else
            {
                if((ret_val = BTFE_P_ChipDoMicroService(ret_val, chip_handle, SVC_GET_EQDATA, 0, NO_ARGS, 6, tempBuffer, SNR_TIMEOUT)) == SCD_RESULT__OK)
                {
                    data->EqualizerSNR = (tempBuffer[0] << 8) | tempBuffer[1];
                }
            }
        }

        if(data->Flags & SCD_STATUS_FAT__TIMING_OFFSET)
        {
            if(cofdm)
            {
                 uint32_t TRL_NormalRate;
				  
                 if((ret_val = BTFE_P_ChipDoMicroService(ret_val, chip_handle, SVC_GET_COFDM_STATUS, 0, NO_ARGS, 6, tempBuffer, SNR_TIMEOUT)) == SCD_RESULT__OK)				  
                {
                    data->TimingOffset = (tempBuffer[4] << 8) | tempBuffer[5];
                    if (tchip->unifiedCofdm.ofdmStandard == OFDM_STANDARD_ISDBT)
                        TRL_NormalRate = 0x4d0e;
                    else if (tchip->Acquisition.bandWidthConfig == SCD_BW_6MHZ)
                        TRL_NormalRate = 0x4104;
                    else if (tchip->Acquisition.bandWidthConfig == SCD_BW_7MHZ)
                        TRL_NormalRate = 0x4BDA;
                    else TRL_NormalRate = 0x56B0; /* 8MHZ */

			/* TimeOffset (ppm) = (TRL_Timing _Offset / 268.4) * (TRL NOMINALRATE / 65536), divide 65536 will be done in app*/
			data->TimingOffset = (data->TimingOffset/268) * (TRL_NormalRate);
                }
            }
            else
            {
                if((ret_val = BTFE_P_ChipDoMicroService(ret_val, chip_handle, SVC_GET_TIMING_OFFSET, 0, NO_ARGS, 4, tempBuffer, SNR_TIMEOUT)) == SCD_RESULT__OK)
                {
                    data->TimingOffset =  (tempBuffer[0] << 24) | (tempBuffer[1] << 16) | (tempBuffer[2] << 8) | (tempBuffer[3]) ;;
                }
            }
        }

        /*
         * Get RS error
         */

	 /* RS Error for J83abc */
        if((data->Flags & SCD_STATUS_FAT__ERRORS)&&(IS_COFDM_FAT_MOD_FORMAT(data->DemodulationFormat)==0))
        {
            temp8 = 0;
            ret_val = BTFE_P_ChipDoMicroService(ret_val, chip_handle, SVC_SET_NFRAMES, 1, &temp8, 0, NO_RESULTS, NFRAMES_TIMEOUT);

			/* TODO: Need to update rs errors for Annex A in FW */
			/* Make sure DemodulationFormat is not Legacy QAM because Legacy QAM gets counters from FW 
			   and switching from Annex A to Legacy QAM doesn't chance tchip->j83abc.j83abcMode to J83B in BBS */
			if((data->DemodulationFormat == SCD_MOD_FORMAT__FAT_J83ABC) && (tchip->j83abc.j83abcMode == J83A || tchip->j83abc.j83abcMode == J83C))
			{
            			if((ret_val = BTFE_P_ChipDoMicroService(ret_val, chip_handle, SVC_GET_RS_ERRORS, 0, NO_ARGS, 42, rs_buffer, RS_TIMEOUT)) == SCD_RESULT__OK)
            			{
                			data->RSUncorrectableErrorsA = (uint32_t) ((rs_buffer[RS_UNCORRECT_ERROR_LAYER_A] << 24) +
								(rs_buffer[RS_UNCORRECT_ERROR_LAYER_A+1] << 16) + 
								(rs_buffer[RS_UNCORRECT_ERROR_LAYER_A+2] << 8) +
								rs_buffer[RS_UNCORRECT_ERROR_LAYER_A+3]);
                			data->RSCorrectableErrorsA = (uint32_t) ((rs_buffer[RS_CORRECT_ERROR_LAYER_A] << 24) +
								(rs_buffer[RS_CORRECT_ERROR_LAYER_A+1] << 16) + 
								(rs_buffer[RS_CORRECT_ERROR_LAYER_A+2] << 8) + 
								rs_buffer[RS_CORRECT_ERROR_LAYER_A+3]);
                			data->NumRSpacketsA = (uint32_t) ((rs_buffer[RS_TOTAL_PACKETS_LAYER_A] << 24) + 
								(rs_buffer[RS_TOTAL_PACKETS_LAYER_A+1] << 16) + 
								(rs_buffer[RS_TOTAL_PACKETS_LAYER_A+2] << 8) + 
								rs_buffer[RS_TOTAL_PACKETS_LAYER_A+3]);
            			}
			}
			else
			{
				if((ret_val = BTFE_P_ChipDoMicroService(ret_val, chip_handle, SVC_GET_RS_ERRORS, 0, NO_ARGS, 6, rs_buffer, RS_TIMEOUT)) == SCD_RESULT__OK)
            			{
                			data->RSUncorrectableErrorsA = (rs_buffer[0] << 8) + rs_buffer[1];
                			data->RSCorrectableErrorsA = (rs_buffer[2] << 8) + rs_buffer[3];
                			data->NumRSpacketsA = (rs_buffer[4] << 8) + rs_buffer[5];
            			}
        		}
        }

	 /* RS Error for cofdm */
	if (((data->Flags & SCD_STATUS_FAT__COFDM_BER)||(data->Flags & SCD_STATUS_FAT__ERRORS ))&&IS_COFDM_FAT_MOD_FORMAT(data->DemodulationFormat))
	{						
            if((ret_val = BTFE_P_ChipDoMicroService(ret_val, chip_handle, SVC_GET_RS_ERRORS, 0, NO_ARGS, 42, rs_buffer, RS_TIMEOUT)) == SCD_RESULT__OK)
            {
            		layer = 0;
                	data->RSUncorrectableErrorsA = (uint32_t) ((rs_buffer[RS_UNCORRECT_ERROR_LAYER_A+layer*4] << 24) +
								(rs_buffer[RS_UNCORRECT_ERROR_LAYER_A+layer*4+1] << 16) + 
								(rs_buffer[RS_UNCORRECT_ERROR_LAYER_A+layer*4+2] << 8) +
								rs_buffer[RS_UNCORRECT_ERROR_LAYER_A+layer*4+3]);
                	data->RSCorrectableErrorsA = (uint32_t) ((rs_buffer[RS_CORRECT_ERROR_LAYER_A+layer*4] << 24) +
								(rs_buffer[RS_CORRECT_ERROR_LAYER_A+layer*4+1] << 16) + 
								(rs_buffer[RS_CORRECT_ERROR_LAYER_A+layer*4+2] << 8) + 
								rs_buffer[RS_CORRECT_ERROR_LAYER_A+layer*4+3]);
                	data->NumRSpacketsA = (uint32_t) ((rs_buffer[RS_TOTAL_PACKETS_LAYER_A+layer*4] << 24) + 
								(rs_buffer[RS_TOTAL_PACKETS_LAYER_A+layer*4+1] << 16) + 
								(rs_buffer[RS_TOTAL_PACKETS_LAYER_A+layer*4+2] << 8) + 
								rs_buffer[RS_TOTAL_PACKETS_LAYER_A+layer*4+3]);
            		layer = 1;
                	data->RSUncorrectableErrorsB = (uint32_t) ((rs_buffer[RS_UNCORRECT_ERROR_LAYER_A+layer*4] << 24) +
								(rs_buffer[RS_UNCORRECT_ERROR_LAYER_A+layer*4+1] << 16) + 
								(rs_buffer[RS_UNCORRECT_ERROR_LAYER_A+layer*4+2] << 8) +
								rs_buffer[RS_UNCORRECT_ERROR_LAYER_A+layer*4+3]);
                	data->RSCorrectableErrorsB = (uint32_t) ((rs_buffer[RS_CORRECT_ERROR_LAYER_A+layer*4] << 24) +
								(rs_buffer[RS_CORRECT_ERROR_LAYER_A+layer*4+1] << 16) + 
								(rs_buffer[RS_CORRECT_ERROR_LAYER_A+layer*4+2] << 8) + 
								rs_buffer[RS_CORRECT_ERROR_LAYER_A+layer*4+3]);
                	data->NumRSpacketsB = (uint32_t) ((rs_buffer[RS_TOTAL_PACKETS_LAYER_A+layer*4] << 24) + 
								(rs_buffer[RS_TOTAL_PACKETS_LAYER_A+layer*4+1] << 16) + 
								(rs_buffer[RS_TOTAL_PACKETS_LAYER_A+layer*4+2] << 8) + 
								rs_buffer[RS_TOTAL_PACKETS_LAYER_A+layer*4+3]);
            		layer = 2;
                	data->RSUncorrectableErrorsC = (uint32_t) ((rs_buffer[RS_UNCORRECT_ERROR_LAYER_A+layer*4] << 24) +
								(rs_buffer[RS_UNCORRECT_ERROR_LAYER_A+layer*4+1] << 16) + 
								(rs_buffer[RS_UNCORRECT_ERROR_LAYER_A+layer*4+2] << 8) +
								rs_buffer[RS_UNCORRECT_ERROR_LAYER_A+layer*4+3]);
                	data->RSCorrectableErrorsC = (uint32_t) ((rs_buffer[RS_CORRECT_ERROR_LAYER_A+layer*4] << 24) +
								(rs_buffer[RS_CORRECT_ERROR_LAYER_A+layer*4+1] << 16) + 
								(rs_buffer[RS_CORRECT_ERROR_LAYER_A+layer*4+2] << 8) + 
								rs_buffer[RS_CORRECT_ERROR_LAYER_A+layer*4+3]);
                	data->NumRSpacketsC = (uint32_t) ((rs_buffer[RS_TOTAL_PACKETS_LAYER_A+layer*4] << 24) + 
								(rs_buffer[RS_TOTAL_PACKETS_LAYER_A+layer*4+1] << 16) + 
								(rs_buffer[RS_TOTAL_PACKETS_LAYER_A+layer*4+2] << 8) + 
								rs_buffer[RS_TOTAL_PACKETS_LAYER_A+layer*4+3]);

		  /* data->COFDMBerErrCnt = (data->RSUncorrectableErrors)*4*9 + (data->RSCorrectableErrors)*3; */
            }
	}

		/* TODO: Correct Coarse offset - difference between tuner IF nominal rate and value of DC_NORMALIZED_IF */
        if(data->Flags & SCD_STATUS_FAT__COARSE_OFFSET)
        {
            if((ret_val = BTFE_P_ChipGetReg16(ret_val, chip_handle, ixFE_DC_CO_STATUS_MSB, &temp16)) == SCD_RESULT__OK)
            {
                if(temp16 & (1<<13))
                    data->CoarseOffset = temp16 - (1<<14);
                else
                    data->CoarseOffset = temp16;
            }
        }

        if(data->Flags & SCD_STATUS_FAT__PILOT_OFFSET)
        {
            if((ret_val = BTFE_P_ChipGetReg32(ret_val, chip_handle, ixFE_PILOT_FREQUENCY_OFFSET_3, &temp32)) == SCD_RESULT__OK)
            {
                data->PilotOffset = temp32;
            }
        }

        if(data->Flags & SCD_STATUS_FAT__CARRIER_OFFSET)
        {
            uint32_t diffNormRate;
			
            if (analog)
            {				
		    if((ret_val = BTFE_P_ChipGetReg24(ret_val, chip_handle, ixHI_ANALOG_FREQ_OFFSET_HZ_2, &temp32)) == SCD_RESULT__OK)
		    {
		        if (temp32 & 0x800000)
		            data->CarrierOffset = (int32_t) temp32 - 0x1000000; /* negative */
			 else
		            data->CarrierOffset = (int32_t) temp32;
		    }
            }		
            else if ((data->DemodulationFormat == SCD_MOD_FORMAT__FAT_VSB) ||
		         (IS_QAM_FAT_MOD_FORMAT(data->DemodulationFormat))) /* VSB and Qam */
            {
            	  uint32_t P_hi, P_lo, Q_hi, Q_lo, sample_freq, symbol_rate, val;
		  int32_t freq_offset;
		  int32_t coarse_offset;
		  int32_t pilot_offset;
		  int32_t carrier_loop;

		  if ((data->DemodulationFormat == SCD_MOD_FORMAT__FAT_VSB) && (tchip->Acquisition.acqConfig == SCD_SEARCH_SCAN))
		  {
			BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_ASM_CONFIG_AREA, 8, tempBuffer);
			temp32 = (tempBuffer[3] << 16) | (tempBuffer[4] << 8) | tempBuffer[5];
		  }
		  else 
		  	BTFE_P_ChipGetReg24(ret_val, chip_handle, ixFE_DC_NORMALIZED_IF_2, &temp32);
		  
		  /* diffFreq = diffIfNormRate * (sample_rate) / (1 << 24) */
		  if (temp32 >= tchip->defaultIFNomRate) 
		  	diffNormRate = temp32 - tchip->defaultIFNomRate;
		  else 
		  	diffNormRate = tchip->defaultIFNomRate - temp32;

		  /* printf("    diffNormRate 0x%x, defaultIFNomRate 0x%x, temp32 0x%x\n", 
		  	diffNormRate, tchip->defaultIFNomRate, temp32); */

		  sample_freq = FE_CRYSTAL_FREQ;
		  BTFE_MultU32U32(diffNormRate, sample_freq, &P_hi, &P_lo);
		  BTFE_DivU64U32(P_hi, P_lo, 16777216, &Q_hi, &Q_lo);

		  if (temp32 >= tchip->defaultIFNomRate)
		  	freq_offset = (int32_t) Q_lo;
		  else 
		  	freq_offset = (int32_t) -Q_lo;
		  
		  /* reverse carrie offset for low IF tuner */
		  if (tchip->FatIfFrequency <= LOW_IF_MAX_FREQ)
		  	freq_offset = -freq_offset;

		  /* printf("    Q_hi %d, freq_offset %d\n", Q_hi, freq_offset); */
		  if ((data->DemodulationFormat == SCD_MOD_FORMAT__FAT_VSB) && (tchip->Acquisition.acqConfig == SCD_SEARCH_SCAN))
		  {
			data->CarrierOffset = freq_offset;
		  }		  	
		  else if (data->DemodulationFormat == SCD_MOD_FORMAT__FAT_VSB)
		  {
			/* 
			 * FE_DC_CO_STATUS: 14 bit register
			 * (coarse_offset) * 2^-16 * 54E6/2
			 */
		  	BTFE_P_ChipGetReg16(ret_val, chip_handle, ixFE_DC_CO_STATUS_MSB, &temp16);
			
			/* compute val = abs(temp16) since the fixed point routines only work for unsigned numbers */
			temp16 &= 0x3FFF; /* 14 bits register */
			if (temp16 & 0x2000)
			{
				temp32 = (uint32_t) (temp16 - 0x4000);
				val = ~temp32 + 1;
			}
			else
   				val = (uint32_t) temp16;

			BTFE_MultU32U32(val, 27000000, &P_hi, &P_lo); /* mutiply 54e6/2 */
			BTFE_DivU64U32(P_hi, P_lo, 65536, &Q_hi, &Q_lo); /* divide by 2^16 */
			if (temp16 & 0x2000)
				coarse_offset = (int32_t) -Q_lo;
			else 
				coarse_offset = (int32_t) Q_lo;			

			/* printf("    coarse_offset reg: 0x%x, in freq %d\n", temp16, coarse_offset ); */
			
			/* 
			 * pilot_offset * pow(2.0, -34.0) * symbol_rate, VSB_SYMBOL_RATE
			 */
		  	BTFE_P_ChipGetReg32(ret_val, chip_handle, ixFE_PILOT_FREQUENCY_OFFSET_3, &temp32);
			
			/* compute val = abs(temp32) since the fixed point routines only work for unsigned numbers */
			if (temp32 & 0x80000000)
				val = ~temp32 + 1;
			else
   				val = temp32;

			symbol_rate = VSB_SYMBOL_RATE;
			BTFE_MultU32U32(val, symbol_rate, &P_hi, &P_lo);
			BTFE_DivU64U32(P_hi, P_lo, 1073741824, &Q_hi, &Q_lo); /* divide by 2^30 */
			BTFE_DivU64U32(Q_hi, Q_lo, 16, &Q_hi, &Q_lo); /* divide by 2^4 again to make 2^-34 factor */
			if (temp32 & 0x80000000)
				pilot_offset = (int32_t) -Q_lo;
			else 
				pilot_offset = (int32_t) Q_lo;
			
			/* printf("    pilot_offset reg: 0x%x, in freq %d\n", temp32, pilot_offset ); */
			
			/* 
			 * qam_carrie_offset * pow(2,0, -35.0) * symbol_rate
			 */
		  	BTFE_P_ChipGetReg32(ret_val, chip_handle, ixEQ_CL_FREQ_OFFSET_3, &temp32);
			if (temp32 & 0x80000000)
				val = ~temp32 + 1;
			else
   				val = temp32;
			
			symbol_rate = tchip->j83abc.symbolRate;
			BTFE_MultU32U32(val, symbol_rate, &P_hi, &P_lo);
			BTFE_DivU64U32(P_hi, P_lo, 1073741824, &Q_hi, &Q_lo); /* divide by 2^30 */
			BTFE_DivU64U32(Q_hi, Q_lo, 32, &Q_hi, &Q_lo); /* divide by 2^5 again to make 2^-35 factor */
			if (temp32 & 0x80000000)
				carrier_loop = (int32_t) -Q_lo;
			else 
				carrier_loop = (int32_t) Q_lo;
			
			/* printf("    carrier_loop reg: 0x%x, in freq %d\n", temp32, carrier_loop ); */
			
			/* if spectrum inverted we need to reverse pilot_offset ? */
			data->CarrierOffset = freq_offset + coarse_offset + pilot_offset + carrier_loop;
		  }
		  else
		  {
			/* 
			 * qam_carrie_offset * pow(2,0, -35.0) * symbol_rate
			 */
		  	BTFE_P_ChipGetReg32(ret_val, chip_handle, ixEQ_CL_FREQ_OFFSET_3, &temp32);			
			if (temp32 & 0x80000000)
				val = ~temp32 + 1;
			else
   				val = temp32;
			
			symbol_rate = tchip->j83abc.symbolRate;
			BTFE_MultU32U32(val, symbol_rate, &P_hi, &P_lo);
			BTFE_DivU64U32(P_hi, P_lo, 1073741824, &Q_hi, &Q_lo); /* divide by 2^30 */
			BTFE_DivU64U32(Q_hi, Q_lo, 32, &Q_hi, &Q_lo); /* divide by 2^5 again to make 2^-35 factor */
			if (temp32 & 0x80000000)
				carrier_loop = (int32_t) -Q_lo;
			else 
				carrier_loop = (int32_t) Q_lo;

			/* printf("    carrier_loop reg: 0x%x, in freq %d\n", temp32, carrier_loop ); */

			/* if spectrum inverted we need to reverse carrier_loop ? */
			data->CarrierOffset = freq_offset + carrier_loop;
		  }		  	
            }             
	 }

        if(data->Flags & SCD_STATUS_FAT__ATSM_STATE)
        {
            if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixEQ_ATSM_READ_WRITE, &temp8)) == SCD_RESULT__OK)
            {
                data->ATSMstate = temp8;
            }
        }

        if(data->Flags & SCD_STATUS_FAT__DFS)
        {
            if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixEQ_DFS_STATE, &temp8)) == SCD_RESULT__OK)
            {
                data->DFSpolarity = FLD2VAL(EQ_DFS_STATE, eq_dfs_polarity, temp8);
                data->DFSstate = FLD2VAL(EQ_DFS_STATE, eq_dfs_state, temp8);
            }
        }

        if(data->Flags & SCD_STATUS_FAT__IFD_LOCK_STATUS)
        {
            if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixHI_ANALOG_DEMOD_STATUS, &temp8)) == SCD_RESULT__OK)
            {
                data->IFDlockStatus = temp8;
            }
        }


        if(data->Flags & SCD_STATUS_FAT__IAGC_GAIN)
        {
			/* 1. Read binary exponent */

			ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixFE_IAGC_ENHANCER, &temp8);
			Gi_exp = FLD2VAL(FE_IAGC_ENHANCER, iagcgainshiftup, temp8);

			/* 2. Read mantissa */
			
			/* FE_IAGC_GAIN_MSB 
			 * The 2 byte register contains a 12 bit value with a format S+[0:-10]
			 *   with a format sbbb bbbb xxxx bbbb  <- LSbits wired in wrong position !!
			 */
			ret_val = BTFE_P_ChipGetReg16(ret_val, chip_handle, ixFE_IAGC_GAIN_MSB, &temp16);
			temp8  = (uint8_t) (temp16 & 0x00ff);         /* save lower 4 bits */
			temp16 = (uint16_t)(((int32_t) temp16) >> 4);   /* move MSBits down */
			Gi_mantissa = (int32_t) temp8 + (int32_t) temp16;

			/* 3. For some reason, the gain can be negative, so it is being handled here */
            
			if(temp16 > 0x800)
				Gi_mantissa = Gi_mantissa - 0x01000;

			/* 4. Combine mantissa and exponent into fixed point number S+[20:-10] */
			
			Gi = Gi_mantissa << Gi_exp;

			/* 5. Move into status structure */
            data->IAGCGain = Gi;
        }

		/* UAGC based Signal Quality Index */
        if(data->Flags & SCD_STATUS_FAT__SQI)
        {
		/* 1. Get the SNR values */
			/* TODO: Add to BBS display */
           if (analog)
	    {
			ret_val = BTFE_P_ChipGetReg16(ret_val, chip_handle, ixHI_UAGC_SNRVID_DB_1, &temp16);
			snr = (int32_t) temp16;
	    }
	    else
	    {            
                /* Set to reading the regular equalizer output */
                ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixEQ_ERRCAL_CONTROL, &temp8);
                MODIFYFLD(temp8, EQ_ERRCAL_CONTROL, snr_input_select, 0);
                ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixEQ_ERRCAL_CONTROL, temp8);

                ret_val = BTFE_P_ChipGetReg16(ret_val, chip_handle, ixEQ_SNR_MSB, &temp16);
                snr = (int32_t) temp16;
            }
		/* 2. Get the RF Power estimate from the UAGC firmware */

            ret_val = BTFE_P_ChipGetReg16(ret_val, chip_handle, ixHI_UAGC_PVID_DBM_1, &temp16);
            temp32signed = (temp16 >= 0x8000) ?
				((int32_t)(temp16) - (int32_t)(0x10000)) :
				 (int32_t) temp16;
	    Prf_dBm = temp32signed;

		/* 3. Get the demodulator lock bit */

            ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixHI_VIDEO_STATUS, &temp8);
            lock_status = (temp8 & VIDEO_STATUS_LOCK_DET) ? 1 :0;

		/* 4. Compute SQI */

	    if (ret_val == SCD_RESULT__OK)
	    {
			data->SignalQualityIndex = BTFE_P_SignalQualityIndex(tchip, data->DemodulationFormat, snr, Prf_dBm, lock_status);
	    }
	}

        if(data->Flags & SCD_STATUS_FAT__EQ_CURSOR)
        {
            if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixEQ_CURSOR_READOUT, &temp8)) == SCD_RESULT__OK)
            {
                data->EqCursor = temp8;
            }
        }

        if(data->Flags & SCD_STATUS_FAT__PILOT_AMPLITUDE)
        {
            if((ret_val = BTFE_P_ChipGetReg16(ret_val, chip_handle, ixFE_PILOT_AMP_MSB, &temp16)) == SCD_RESULT__OK)
            {
                if(temp16 > 0x8000)
                    data->PilotAmplitude = temp16 - 0x10000;
                else
                    data->PilotAmplitude = temp16;
            }
        }

        if(data->Flags & SCD_STATUS_FAT__PILOT_ESTIMATE)
        {
            if((ret_val = BTFE_P_ChipGetReg16(ret_val, chip_handle, ixEQ_PILOTEST_MSB, &temp16)) == SCD_RESULT__OK)
            {
                data->PilotEstimate = temp16;
            }
        }

        if(data->Flags & SCD_STATUS_FAT__DUR)
        {
			{
				if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixHI_UAGC_DUR_DB, &temp8)) == SCD_RESULT__OK)
				{
					if(temp8 >= 0x80)
						data->DUR = temp8 - 0x100;
					else
						data->DUR = temp8;
				}
			}
        }

        if(data->Flags & SCD_STATUS_FAT__QAM_INTERLEAVER_MODE)
        {
            if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixFEC_QAMFSD_INT_MODE, &temp8)) == SCD_RESULT__OK)
            {
                data->QAMinterleaverMode = temp8;
            }
        }

        if(data->Flags & SCD_STATUS_FAT__ACB)
        {
            if(analog)
                acb_args[0] = ACQ_STATUS_INDX_NTSC;
            else if(data->DemodulationFormat == SCD_MOD_FORMAT__FAT_VSB)
                acb_args[0] = ACQ_STATUS_INDX_VSB;
#ifdef SCD_LEGACY_QAM
            else if(data->DemodulationFormat == SCD_MOD_FORMAT__FAT_QAM64)
                acb_args[0] = ACQ_STATUS_INDX_64QAM;
            else if(data->DemodulationFormat == SCD_MOD_FORMAT__FAT_QAM256)
                acb_args[0] = ACQ_STATUS_INDX_256QAM;
#endif
            else if (data->DemodulationFormat == SCD_MOD_FORMAT__FAT_J83ABC)
                acb_args[0] = ACQ_STATUS_INDX_J83;
            else if(cofdm == 1)
                acb_args[0] = ACQ_STATUS_INDX_COFDM;
            else if(cofdm == 2)
                acb_args[0] = ACQ_STATUS_INDX_UCOFDM;		
            else
                acb_args[0] = 0xFF;

            if((acb_args[0] != 0xFF) && (ret_val = BTFE_P_ChipDoMicroService(ret_val, chip_handle, SVC_GET_ACQ_STATUS, 1, acb_args, SIZE_ACB, acb_buffer, ACB_TIMEOUT)) == SCD_RESULT__OK)
            {
                data->ACBState = acb_buffer[0];
                data->ACBStatus = acb_buffer[1];
                data->ACBTimer = acb_buffer[2];
                data->ACBAcqTime = ((acb_buffer[3] << 8) + acb_buffer[4]);
                data->ACBNumReacqs = acb_buffer[5];	/* Fix reacqcount for Annex A in FW */
                data->AgcSettleTime = (acb_buffer[6] << 8) + acb_buffer[7];
            }
            else
            {
                data->ACBState = 0;
                data->ACBStatus = 0;
                data->ACBTimer = 0;
                data->ACBAcqTime = 0;
                data->ACBNumReacqs = 0;
                data->AgcSettleTime = 0;
            }
        }

        if(data->Flags & SCD_STATUS_FAT__SAMPLE_FREQUENCY)
        {
            data->SampleFrequency = FAT_SAMPLE_FREQ;
        }

        if(data->Flags & SCD_STATUS_FAT__TARGET_IF_FREQUENCY)
        {
            if ((tchip->FatIfFrequency > LOW_IF_MAX_FREQ) || IS_ANALOG_FAT_MOD_FORMAT(data->DemodulationFormat))
                data->TargetIfFrequency = FAT_IF_TARGET_FREQ(tchip->FatIfFrequency);
            else 
                data->TargetIfFrequency = tchip->FatIfFrequency;
        }

		/* TODO: Correct symbol rate for Annex A modes */
        if(data->Flags & SCD_STATUS_FAT__SYMBOL_RATE)
        {
            switch(data->DemodulationFormat)
            {
            case SCD_MOD_FORMAT__FAT_VSB:     data->SymbolRate = VSB_SYMBOL_RATE;    break;
#ifdef SCD_LEGACY_QAM
            case SCD_MOD_FORMAT__FAT_QAM64:  
				data->SymbolRate = QAM64_SYMBOL_RATE;  break;
            case SCD_MOD_FORMAT__FAT_QAM256:
				data->SymbolRate = QAM256_SYMBOL_RATE; break;
#endif
            case SCD_MOD_FORMAT__FAT_J83ABC: 
				if (tchip->j83abc.qamMode == QAM_MODE_DIGITAL_64QAM)
					data->SymbolRate = QAM64_SYMBOL_RATE;  
				if (tchip->j83abc.qamMode == QAM_MODE_DIGITAL_256QAM)
					data->SymbolRate = QAM256_SYMBOL_RATE;  
				if (tchip->j83abc.j83abcMode == J83A || tchip->j83abc.j83abcMode == J83C)
					data->SymbolRate = tchip->j83abc.symbolRate;
				break;
            default:  break;
            }
        }

        if(data->Flags & SCD_STATUS_FAT__NORMALIZED_IF)
        {
            if((ret_val = BTFE_P_ChipGetReg24(ret_val, chip_handle, ixFE_DC_NORMALIZED_IF_2, &temp32)) == SCD_RESULT__OK)
            {
                data->NormalizedIF = temp32 - 0x1000000;
            }
        }

	if (IS_COFDM_FAT_MOD_FORMAT(data->DemodulationFormat))
	{
		uint8_t tempBuffer[32];
		/* uint32_t log2Val[4] = {2, 2, 4, 6}; */

#define OFDM_IDX_REACQ_COUNTER		0
#define OFDM_IDX_MODE				1
#define OFDM_IDX_GUARD				2
#define OFDM_IDX_CONSTELLATION		3
#define OFDM_IDX_CODE_HP			4
#define OFDM_IDX_CODE_LP			5
#define OFDM_IDX_HIERARCHY			6
#define OFDM_IDX_CONSTELLATION_A	7
#define OFDM_IDX_CONSTELLATION_B	8
#define OFDM_IDX_CONSTELLATION_C	9
#define OFDM_IDX_CODE_A				10
#define OFDM_IDX_CODE_B				11
#define OFDM_IDX_CODE_C				12
#define OFDM_IDX_TDIMODE_A			13
#define OFDM_IDX_TDIMODE_B			14
#define OFDM_IDX_TDIMODE_C			15
#define OFDM_IDX_SEGMENT_A			16
#define OFDM_IDX_SEGMENT_B			17
#define OFDM_IDX_SEGMENT_C			18
#define OFDM_IDX_EWS				19
#define OFDM_IDX_PARTIALRECEPTION	20
#define OFDM_IDX_CELL_ID_LSB			21
#define OFDM_IDX_CELL_ID_MSB		22
#define OFDM_IDX_SNR				23
#define OFDM_IDX_TOTAL				24

		BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_UNIFIED_COFDM_STATUS_AREA, OFDM_IDX_TOTAL, tempBuffer);

		data->COFDMMode = (SCD_OFDM_MODE)tempBuffer[OFDM_IDX_MODE];
		data->COFDMGuardInt= (SCD_OFDM_GUARD)tempBuffer[OFDM_IDX_GUARD];
		data->COFDMModFormat = (SCD_OFDM_CONSTEL)tempBuffer[OFDM_IDX_CONSTELLATION];
		data->COFDMHierarchy = (SCD_OFDM_HIERARCHY)tempBuffer[OFDM_IDX_HIERARCHY];
		data->COFDMModFormatA = (SCD_OFDM_CONSTEL)tempBuffer[OFDM_IDX_CONSTELLATION_A];
		data->COFDMModFormatB = (SCD_OFDM_CONSTEL)tempBuffer[OFDM_IDX_CONSTELLATION_B];
		data->COFDMModFormatC = (SCD_OFDM_CONSTEL)tempBuffer[OFDM_IDX_CONSTELLATION_C];
		data->COFDMCodeRateA= (SCD_OFDM_CODERATE)tempBuffer[OFDM_IDX_CODE_A];
		data->COFDMCodeRateB = (SCD_OFDM_CODERATE)tempBuffer[OFDM_IDX_CODE_B];
		data->COFDMCodeRateC = (SCD_OFDM_CODERATE)tempBuffer[OFDM_IDX_CODE_C];		
		data->COFDMTdiA = (SCD_COFDM_TDI)tempBuffer[OFDM_IDX_TDIMODE_A];
		data->COFDMTdiB = (SCD_COFDM_TDI)tempBuffer[OFDM_IDX_TDIMODE_B];
		data->COFDMTdiC = (SCD_COFDM_TDI)tempBuffer[OFDM_IDX_TDIMODE_C];
		data->COFDMSegA = tempBuffer[OFDM_IDX_SEGMENT_A];
		data->COFDMSegB = tempBuffer[OFDM_IDX_SEGMENT_B];
		data->COFDMSegC = tempBuffer[OFDM_IDX_SEGMENT_C];
		data->ews = tempBuffer[OFDM_IDX_EWS];
		data->partialReception = tempBuffer[OFDM_IDX_PARTIALRECEPTION];
		data->cellId = (tempBuffer[OFDM_IDX_CELL_ID_MSB] << 8) | tempBuffer[OFDM_IDX_CELL_ID_LSB];
		if (tchip->unifiedCofdm.priorityMode == OFDM_PRIORITY_LOW) /* For DVB-T */
			data->COFDMCodeRate = (SCD_OFDM_CODERATE)tempBuffer[OFDM_IDX_CODE_LP];
		else data->COFDMCodeRate = (SCD_OFDM_CODERATE)tempBuffer[OFDM_IDX_CODE_HP];

	}

	/*
	 * Find out demod spectrum inversion status
	 *
	 * A) Auto detect mode: acquisition.bSpectrumAutoDetect == true
	 *       - cofdm: return the detected spectrum against acquisition.bSpectrumInversion
	 *       - qam83abc/vsb/L-qam don't support (just report as not auto detect mode)
	 * B) Not auto detect mode:
	 *       - cofdm/qam83abc/vsb: use acquisition.bSpectrumInversion
	 *       - L-qam: auto switch, and how to find out the spectrumInversion?
	 */
        {
		bool tunerSpectrumInv;

		tunerSpectrumInv = tchip->tunerSpectrum;
		if (tchip->FatIfFrequency > LOW_IF_MAX_FREQ)
			tunerSpectrumInv = (tchip->tunerSpectrum) ? false : true; /* we have tuner spec reversed */
		data->demodSpectrum = (tchip->Acquisition.bSpectrumInversion) ? (tunerSpectrumInv ? false : true) : (tunerSpectrumInv);

		/* cofdm returns with demod spectrum */
		if (IS_COFDM_FAT_MOD_FORMAT(data->DemodulationFormat))
		{
			BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_UNIFIED_COFDM_STATUS_AREA+62, 1, tempBuffer);
			if (tempBuffer[0]) 
				data->demodSpectrum = true;
			else data->demodSpectrum = false;
		}		
        }

    }
    else
    {
        ret_val = SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    /* 
     * Use by BBS to display 
     * - j83Mode: J83a/b/c
     * - qamMode: qam16/qam32/qam64/qam128/qam256
     * - confirmModulator: signal detected or not
     */
    data->j83Mode = tchip->j83abc.j83abcMode;
    data->qamMode = tchip->j83abc.qamMode;
    if (data->DemodulationFormat == SCD_MOD_FORMAT__FAT_VSB)
    {
        BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_VSB_STATUS_AREA+9, 1, tempBuffer);
    }
    else if (data->DemodulationFormat == SCD_MOD_FORMAT__FAT_UNIFIED_COFDM)
    {
        BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_UNIFIED_COFDM_STATUS_AREA+61, 1, tempBuffer);
    }	
    else if ((data->DemodulationFormat == SCD_MOD_FORMAT__FAT_J83ABC) ||
		(data->DemodulationFormat == SCD_MOD_FORMAT__FAT_QAM64) ||
		(data->DemodulationFormat == SCD_MOD_FORMAT__FAT_QAM256))
    {
        BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_J83_STATUS_AREA+13, 1, tempBuffer);
    }
    data->confirmModulator = (tempBuffer[0] == 1) ? 1: 0;
		
    if (ret_val != SCD_RESULT__OK)
    {
		BDBG_ERR(("GetFATstatus returning ERROR !!!"));
    }
    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipGetBert(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_STATUS__BERT *data, uint32_t size)
{
    CHIP *tchip = GET_CHIP(chip_handle);
    SCD_RESULT ret_val=SCD_RESULT__OK;
    uint8_t temp8;
    uint8_t bert_ctl_a;
    uint8_t bert_ctl_b;
    uint8_t bert_stat;
    bool fe_lock;

    if(size != sizeof(SCD_STATUS__BERT))
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    data->LockStatus = SCD_LOCK_STATUS__NOT_READY;
    data->ErrorCount = 0;

    fe_lock = 1;

    if(GET_HANDLE_TYPE(handle) == SCD_HANDLE_FAT)
    {
        switch(tchip->FatModFormat)
        {
        case SCD_MOD_FORMAT__FAT_VSB:
            if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixEQ_DFS_STATE, &temp8)) == SCD_RESULT__OK)
            {
                if(FLD2VAL(EQ_DFS_STATE, eq_dfs_state, temp8) & EQ_DFS_STATE__ACQ)
                {
                    BERT_DEBUG(BDBG_WRN(("BTFE_P_ChipGetBert: EQ DFS not locked")));
                    fe_lock = 0;
                }
            }
            break;

#ifdef SCD_LEGACY_QAM
        case SCD_MOD_FORMAT__FAT_QAM64:
        case SCD_MOD_FORMAT__FAT_QAM256:
#endif
        case SCD_MOD_FORMAT__FAT_J83ABC:
            switch(tchip->BertInput)
            {
            case SCD_BERT_INPUT__TRELLIS:
                if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixFEC_TD_MODE_CTRL, &temp8)) == SCD_RESULT__OK)
                {
                    if(FLD2VAL(FEC_TD_MODE_CTRL, td_sync_state, temp8) == 0)
                    {
                        BERT_DEBUG(BDBG_WRN(("BTFE_P_ChipGetBert: FEC TD not locked")));
                        fe_lock = 0;
                    }
                }
                break;

            case SCD_BERT_INPUT__DEINTERLEAVER:
            case SCD_BERT_INPUT__FAT:
                if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixFEC_QAMFSD_STATUS, &temp8)) == SCD_RESULT__OK)
                {
                    if(!(FLD2VAL(FEC_QAMFSD_STATUS, qam_det_state, temp8) & QAM_DET_STATE__LOCK) && !(FLD2VAL(FEC_QAMFSD_STATUS, qam_det_state, temp8) & QAM_DET_STATE__VERIFY))
                    {
                        BERT_DEBUG(BDBG_WRN(("BTFE_P_ChipGetBert: FEC QAM not locked")));
                        fe_lock = 0;
                    }
                }
                break;

            default:
                BDBG_ERR(("BTFE_P_ChipGetBert: invalid BERT input"));
                ret_val = SCD_RESULT__ERROR;
                break;
            }
            break;

        default:
            BDBG_ERR(("BTFE_P_ChipGetBert: invalid modulation format"));
            ret_val = SCD_RESULT__ERROR;
            break;
        }
    }
    else /* if(GET_HANDLE_TYPE(handle) != SCD_HANDLE_FDC) */
    {
        ret_val = SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if((ret_val == SCD_RESULT__OK) && fe_lock)
    {
        if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixBERT_CTL_A, &bert_ctl_a)) == SCD_RESULT__OK)
        {
            /* check if BERT is running */
            if(FLD2VAL(BERT_CTL_A, on_flag, bert_ctl_a) == 1)
            {
                if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixBERT_CTL_B, &bert_ctl_b)) == SCD_RESULT__OK)
                {
                    /* check for loss of lock */
                    if(FLD2VAL(BERT_CTL_B, lss_lck, bert_ctl_b) == 1)
                    {
                        BERT_DEBUG(BDBG_WRN(("BTFE_P_ChipGetBert: BERT lock lost")));

                        data->LockStatus = SCD_LOCK_STATUS__UNLOCKED;
                    }
                    else
                    {
                        if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixBERT_STAT, &bert_stat)) == SCD_RESULT__OK)
                        {
                            if((FLD2VAL(BERT_STAT, sync_st, bert_stat) == 1) && (FLD2VAL(BERT_CTL_B, nw_err, bert_ctl_b) == 1))
                            {
                                data->LockStatus = SCD_LOCK_STATUS__LOCKED;

                                ret_val = BTFE_P_ChipGetReg32(ret_val, chip_handle, ixBERT_ERR_CNT3, &data->ErrorCount);
                            }
                            else
                            {
                                BERT_DEBUG(BDBG_WRN(("BTFE_P_ChipGetBert: BERT synchronizer not locked")));
                            }
                        }
                    }
                }
            }
            else
            {
                BERT_DEBUG(BDBG_WRN(("BTFE_P_ChipGetBert: BERT not on")));
                ret_val = SCD_RESULT__ERROR;
            }
        }
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipGetPsdFrontEnd(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_STATUS__PSD *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    SCD_RESULT ret_val_2;
    uint8_t tempdata[PSDARRAYSIZE*2];
    int32_t *dataPointer;
    uint32_t j;
    uint8_t psd[4];
    uint8_t temp8;

    (void) handle;

    if(size != sizeof(SCD_STATUS__PSD))
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    /*
        PSD_SETTINGS    [R/W]    8 bits    Access: 8    primaryRegisterAperture:0xf269  
        Field Name Bits Default Description 
        bincount 1:0 0x3 Static
        Selects the PSD bin count of the PSD calculator. This register can only be changed while the SignalProcessingEnable bit is 0. 
        00 - 64 Bin PSD
        01 - 128 Bin PSD
        10 - 256 Bin PSD
        11 - 512 Bin PSD (default)
 
        iterationcount 4:2 0x3 Static
        Number of samples per bin in the PSD calculator. This register can only be changed while the SignalProcessingEnable bit is 0.
        000 - 2^ 4 samples
        001 - 2^ 6 samples
        010 - 2^ 8 samples
        011 - 2^ 10 samples (default)
        100 - 2^ 11 samples
        101 - 2^ 12 samples
        110 - 2^ 13 samples
        111 - 2^ 14 samples
 
        alpha_bincount 6:5 0x1 Static
        Selects the PSD alpha bandpass filter term of the PSD calculator. The alpha term basically selects the sharpness of the bandpass filter. This register can only be changed while the SignalProcessingEnable bit is 0. 
        00 - 2^ -7
        01 - 2^ -8 (default)
        10 - 2^ -9
        11 - 2^ -10 
    */

    psd[0] = 0;    /* PSDSource = FAT ADC */
    psd[1] = 0;    /* StartBin */
    psd[2] = 255;  /* StopBin */

    /*  xaas ssbb    aa - alpha   sss - samples  bb - # bins */
    psd[3] = 0x2F; /* 2f = 0010 1111 aa = 00   sss = 011  b = 11  */
    /*psd[3] = 0x5F;*/ /* 5f = 0101 1111   aa = 10   sss = 111  b = 11  alpha (aa) is a little too narrow for bin ! */
    /* psd[3] = 0x7F; */ /* 7f = 0111 1111   aa = 11   sss = 111  b = 11  alpha (aa) is too narrow for bin ! */
    /* psd[3] = 0x3F; */ /* 3f = 0011 1111   aa = 01   sss = 111  b = 11  alpha (aa) is a little too narrow for bin ! */

    if((ret_val = BTFE_P_ChipDoMicroServiceStart(ret_val, chip_handle, SVC_GET_PWR_SPECT, 4, psd, PSD_TIMEOUT)) == SCD_RESULT__OK)
    {
        /* get results */
        ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_UC_FEC_PSD_AGC_RAM_I2C_CNTRL, &temp8);
        MODIFYFLD(temp8, MISC_UC_FEC_PSD_AGC_RAM_I2C_CNTRL, i2c_ram_access_enable, 1);
        MODIFYFLD(temp8, MISC_UC_FEC_PSD_AGC_RAM_I2C_CNTRL, uc_fec_ram_access_enable, 1);
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_UC_FEC_PSD_AGC_RAM_I2C_CNTRL, temp8);

        if((ret_val = BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_SERVICE_RESULTS_1, PSDARRAYSIZE*2, tempdata)) == SCD_RESULT__OK)
        {
            dataPointer = data->power_spectrum_data;

            for(j=0; j<PSDARRAYSIZE; j++)
            {
                *dataPointer++ = (tempdata[j*2] <<8) + (tempdata[j*2+1]) + (((tempdata[j*2] & 0x80) == 0x80) ? 0xffff0000 : 0x0);
            }
        }
    }

    /* end service request */
    if((ret_val_2 = BTFE_P_ChipDoMicroServiceClear(SCD_RESULT__OK, chip_handle, SVC_GET_PWR_SPECT)) != SCD_RESULT__OK)
    {
        ret_val = ret_val_2;
    }

    return ret_val;
}

/****************************************************************************/

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipGetEqTaps(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_STATUS__EQ_TAPS_PLUS *data, uint32_t size)
{
    CHIP *tchip = GET_CHIP(chip_handle);
    SCD_RESULT ret_val=SCD_RESULT__OK;
	uint8_t tempdata[4800];
	uint8_t tempNorm[288];
    int32_t *dataPointer;
	int32_t *adjustmentPointer;
	int32_t *avgnormPointer;
    uint32_t j;
    uint8_t eqtap[1];
    uint8_t temp8;
    uint8_t status;
	uint16_t FIR_TAPSECTIONSIZE=704;
	uint16_t IIR_TAPSECTIONSIZE=448;
	uint16_t FIR_UNITSIZE=88;
	uint16_t IIR_UNITSIZE=56;


    (void) handle;

    if(size != sizeof(SCD_STATUS__EQ_TAPS_PLUS))
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    eqtap[0] = 0;   /* FAT only for now */

    /* Set up Micro service */
    /* Determine modulation mode */

#ifdef SCD_LEGACY_QAM
    if((tchip->FatModFormat == SCD_MOD_FORMAT__FAT_QAM64) ||
	(tchip->FatModFormat == SCD_MOD_FORMAT__FAT_QAM256) ||
	(tchip->FatModFormat == SCD_MOD_FORMAT__FAT_J83ABC))
#else
    if(tchip->FatModFormat == SCD_MOD_FORMAT__FAT_J83ABC)
#endif
        ret_val = BTFE_P_ChipDoMicroServiceStart(ret_val, chip_handle, SVC_GET_QAM_TAPS, 1, eqtap, TAPS_TIMEOUT);
    else if(tchip->FatModFormat == SCD_MOD_FORMAT__FAT_VSB)
        ret_val = BTFE_P_ChipDoMicroServiceStart(ret_val, chip_handle, SVC_GET_VSB_TAPS, 1, eqtap, TAPS_TIMEOUT);
    else
        ret_val = SCD_RESULT__ARG_OUT_OF_RANGE;


#ifdef SCD_LEGACY_QAM
    if((tchip->FatModFormat == SCD_MOD_FORMAT__FAT_QAM64) ||
	(tchip->FatModFormat == SCD_MOD_FORMAT__FAT_QAM256) ||
	(tchip->FatModFormat == SCD_MOD_FORMAT__FAT_J83ABC))
#else
    if(tchip->FatModFormat == SCD_MOD_FORMAT__FAT_J83ABC)
#endif
      ret_val = BTFE_P_ChipDoMicroServiceCheckWait(ret_val, chip_handle, SVC_GET_QAM_TAPS, &status, TAPS_TIMEOUT);
    else
      ret_val = BTFE_P_ChipDoMicroServiceCheckWait(ret_val, chip_handle, SVC_GET_VSB_TAPS, &status, TAPS_TIMEOUT);

    /* Get the data */
    /* open up the RAM/FEC RAM area  */
    temp8 = 0;
    MODIFYFLD(temp8, MISC_UC_FEC_PSD_AGC_RAM_I2C_CNTRL, i2c_ram_access_enable, 1);
    MODIFYFLD(temp8, MISC_UC_FEC_PSD_AGC_RAM_I2C_CNTRL, uc_fec_ram_access_enable, 1);
    ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_UC_FEC_PSD_AGC_RAM_I2C_CNTRL, temp8);

    /* read dependent on mode QAM or VSB */
#ifdef SCD_LEGACY_QAM
    if((tchip->FatModFormat == SCD_MOD_FORMAT__FAT_QAM64) ||
	(tchip->FatModFormat == SCD_MOD_FORMAT__FAT_QAM256) ||
	(tchip->FatModFormat == SCD_MOD_FORMAT__FAT_J83ABC))
#else
    if(tchip->FatModFormat == SCD_MOD_FORMAT__FAT_J83ABC)
#endif
    {
        /* QAM and FDC use this area */
        ret_val = BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_SERVICE_RESULTS_1, 1024, tempdata);
    }
    else if(tchip->FatModFormat == SCD_MOD_FORMAT__FAT_VSB)
    {
        /* This read has to be done in multiple parts */

		   /* Read FIR Q [7:0] */
           ret_val = BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_SERVICE_RESULTS_2 + 0x0000, FIR_TAPSECTIONSIZE, tempdata + 0*FIR_TAPSECTIONSIZE);
		   /* Read FIR I[3:0], Q[11:8] */  
           ret_val = BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_SERVICE_RESULTS_2 + 0x0800, FIR_TAPSECTIONSIZE, tempdata + 1*FIR_TAPSECTIONSIZE);
		   /* Read FIR I [11:4] */
           ret_val = BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_SERVICE_RESULTS_2 + 0x1000, FIR_TAPSECTIONSIZE, tempdata + 2*FIR_TAPSECTIONSIZE);
		   /* Read IIR Q [7:0] */  
           ret_val = BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_SERVICE_RESULTS_2 + 0x0400, IIR_TAPSECTIONSIZE, tempdata + 3*FIR_TAPSECTIONSIZE);
		   /* Read IIR I[3:0], Q[11:8] */
           ret_val = BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_SERVICE_RESULTS_2 + 0x0c00, IIR_TAPSECTIONSIZE, tempdata + 3*FIR_TAPSECTIONSIZE+IIR_TAPSECTIONSIZE);
		   /* Read IIR I [11:4] */
           ret_val = BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_SERVICE_RESULTS_2 + 0x1400, IIR_TAPSECTIONSIZE, tempdata + 3*FIR_TAPSECTIONSIZE+2*IIR_TAPSECTIONSIZE);
 
		   /* Read FIR AVG NORM [7:0] */
		   ret_val = BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_SERVICE_RESULTS_2 - 0x1000, FIR_UNITSIZE, tempNorm + 0*FIR_UNITSIZE);
		   /* Read FIR AVG NORM [15:8] */
		   ret_val = BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_SERVICE_RESULTS_2 - 0x0800, FIR_UNITSIZE, tempNorm + 1*FIR_UNITSIZE);
		   /* Read IIR AVG NORM [7:0] */
		   ret_val = BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_SERVICE_RESULTS_2 - 0x0c00, IIR_UNITSIZE, tempNorm + 2*FIR_UNITSIZE);
		   /* Read IIR AVG NORM [15:8] */
		   ret_val = BTFE_P_ChipGetRegs(ret_val, chip_handle, ixHI_SERVICE_RESULTS_2 - 0x0400, IIR_UNITSIZE, tempNorm + 2*FIR_UNITSIZE+IIR_UNITSIZE);


    }

    /* Clear the buffer in use flag */
#ifdef SCD_LEGACY_QAM
    if((tchip->FatModFormat == SCD_MOD_FORMAT__FAT_QAM64) ||
	(tchip->FatModFormat == SCD_MOD_FORMAT__FAT_QAM256) ||
	(tchip->FatModFormat == SCD_MOD_FORMAT__FAT_J83ABC))
#else
    if(tchip->FatModFormat == SCD_MOD_FORMAT__FAT_J83ABC)
#endif
    {
        BTFE_P_ChipDoMicroServiceClear(ret_val, chip_handle, SVC_GET_QAM_TAPS);

        dataPointer = data->taps;

        for(j=0; j<512; j++)
        {
            *dataPointer++ = (int32_t) ((int16_t) (256 *tempdata[j*2] + tempdata[j*2+1]));
        }
    }
    else if(tchip->FatModFormat == SCD_MOD_FORMAT__FAT_VSB)
    {
        BTFE_P_ChipDoMicroServiceClear(ret_val, chip_handle, SVC_GET_VSB_TAPS);

        dataPointer = data->taps;
		/*adjustmentPointer++=data->adjustment;*/
		adjustmentPointer=data->adjustment;
		avgnormPointer=data->avgnorm;


		  for(j=0; j<FIR_TAPSECTIONSIZE; j++)
        {
            /* do the FIR Q tap */
			*adjustmentPointer=*dataPointer;
            *dataPointer = (int32_t) ((tempdata[j]) + ((tempdata[FIR_TAPSECTIONSIZE + j] & 0x0f) <<8)) + ((tempdata[FIR_TAPSECTIONSIZE + j] & 0x08) ? 0xfffff000 : 0);
    		*adjustmentPointer=(*dataPointer++)-(*adjustmentPointer);
                adjustmentPointer++;

            /* do the FIR I tap */
			*adjustmentPointer=*dataPointer;
            *dataPointer = (int32_t) (((tempdata[FIR_TAPSECTIONSIZE + j] & 0xf0) >> 4) + ((tempdata[FIR_TAPSECTIONSIZE *2 + j]) <<4)) + ((tempdata[FIR_TAPSECTIONSIZE *2 + j] & 0x80) ? 0xfffff000 : 0);
            /**adjustmentPointer++=(*dataPointer++)-(*adjustmentPointer);*/
            *adjustmentPointer=(*dataPointer++)-(*adjustmentPointer);
            adjustmentPointer++;
		
       /*     *avgnormPointer++=(int32_t)(((tempNorm[j] & 0xf0) >> 4)+((tempNorm[FIR_TAPSECTIONSIZE + j]) <<4))+((tempNorm[FIR_TAPSECTIONSIZE + j] & 0x80) ? 0xfffff000 : 0);
	   		*avgnormPointer++=(int32_t)(tempNorm[j] +((tempNorm[FIR_TAPSECTIONSIZE + j]) <<8))+((tempNorm[FIR_TAPSECTIONSIZE + j] & 0x80) ? 0xffff0000 : 0); */
	   		*avgnormPointer++=(int32_t)((tempNorm[(uint8_t)(j/8)])+((tempNorm[FIR_UNITSIZE + (uint8_t)(j/8)]) <<8));
			/*		*avgnormPointer++=((tempNorm[FIR_TAPSECTIONSIZE + j] & 0x80) ? 0xfffffE00 : 100);
			*(avgnormPointer+FIR_TAPSECTIONSIZE)=abs(*avgnormPointer-temp32); */
			/**avgnormPointer++=*(avgnormPointer-1);*/
			*avgnormPointer=*(avgnormPointer-1);
			avgnormPointer++;
		  }
		 

        for(j=0; j<IIR_TAPSECTIONSIZE; j++)
        {
            /* do the IIR Q tap */
            if(((j % 8) == 0) && (j != 0))
            {
                dataPointer += 8;
				adjustmentPointer += 8;
            }
            *adjustmentPointer=*dataPointer;
            *dataPointer = (int32_t) ((tempdata[j + 3*FIR_TAPSECTIONSIZE]) + ((tempdata[3*FIR_TAPSECTIONSIZE+IIR_TAPSECTIONSIZE + j] & 0x0f) <<8)) + ((tempdata[3*FIR_TAPSECTIONSIZE+IIR_TAPSECTIONSIZE + j] & 0x08) ? 0xfffff000 : 0);
            *adjustmentPointer=(*dataPointer)-(*adjustmentPointer);
			 
            /* do the IIR I tap */
            *(adjustmentPointer+8)=*(dataPointer+8);            
            *(dataPointer+8) = (int32_t) (((tempdata[3*FIR_TAPSECTIONSIZE+IIR_TAPSECTIONSIZE +j] & 0xf0) >> 4) + ((tempdata[3*FIR_TAPSECTIONSIZE+2*IIR_TAPSECTIONSIZE + j]) <<4)) + ((tempdata[3*FIR_TAPSECTIONSIZE+2*IIR_TAPSECTIONSIZE + j] & 0x80) ? 0xfffff000 : 0);
            *(adjustmentPointer+8)=*(dataPointer+8)-*(adjustmentPointer+8);
            dataPointer++;
			adjustmentPointer++;

		/*	*avgnormPointer++=(((int32_t)(((tempNorm[2*FIR_TAPSECTIONSIZE+j]& 0xf0) >> 4)+((tempNorm[2*FIR_TAPSECTIONSIZE +IIR_TAPSECTIONSIZE+j]) <<4))+((tempNorm[2*FIR_TAPSECTIONSIZE +IIR_TAPSECTIONSIZE+j] & 0x80) ? 0xfffff000 : 0)));
			*avgnormPointer++=(((int32_t)((tempNorm[2*FIR_TAPSECTIONSIZE+j])+((tempNorm[2*FIR_TAPSECTIONSIZE +IIR_TAPSECTIONSIZE+j]) <<8))+((tempNorm[2*FIR_TAPSECTIONSIZE +IIR_TAPSECTIONSIZE+j] & 0x80) ? 0xffff0000 : 0)));
			*avgnormPointer++=((tempNorm[2*FIR_TAPSECTIONSIZE +IIR_TAPSECTIONSIZE+j] & 0x80) ? 0xfffffE00 : 100);
            temp32=*avgnormPointer; */
			*avgnormPointer++=(int32_t)((tempNorm[2*FIR_UNITSIZE+(uint8_t)(j/8)])+((tempNorm[2*FIR_UNITSIZE +IIR_UNITSIZE+(uint8_t)(j/8)]) <<8));
			/**avgnormPointer++=*(avgnormPointer-1);*/
			*avgnormPointer=*(avgnormPointer-1);
			avgnormPointer++;
        }
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipGetConstellationData(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_STATUS__CONSTELLATION_DATA *data, uint32_t size)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    uint16_t temp16;

    if(size != sizeof(SCD_STATUS__CONSTELLATION_DATA))
    {
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(GET_HANDLE_TYPE(handle) == SCD_HANDLE_FAT)
    {
#if 0
        if((ret_val = BTFE_P_ChipDoMicroService(ret_val, chip_handle, SVC_GET_EQDATA, 0, NO_ARGS, 6, tempBuffer, SNR_TIMEOUT)) == SCD_RESULT__OK)
        {
			data->constX = (int32_t) (int16_t) (tempBuffer[2] << 8) | (tempBuffer[3]);
			data->constY = (int32_t) (int16_t) (tempBuffer[4] << 8) | (tempBuffer[5]);
        }
#endif

        if((ret_val = BTFE_P_ChipGetReg16(ret_val, chip_handle, ixEQ_CONSTELLATION_OUTPUT_3, &temp16)) == SCD_RESULT__OK)
		data->constX = (int32_t) (int16_t) temp16;

        if((ret_val = BTFE_P_ChipGetReg16(ret_val, chip_handle, ixEQ_CONSTELLATION_OUTPUT_1, &temp16)) == SCD_RESULT__OK)
		data->constY = (int32_t) (int16_t) temp16;
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetConfig(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_CONFIG_ITEM item, void *data, uint32_t size)
{
   CHIP_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ChipSetConfig(chip_handle=%08X, handle=%08X, item=%u, &data=%08X, size=%u)", chip_handle, handle, item, data, size)));

   switch(item)
   {
      case SCD_CONFIG_ITEM__BERT:                 return BTFE_P_ChipSetBert(chip_handle, handle, (SCD_CONFIG__BERT*)data, size);
      case SCD_CONFIG_ITEM__GPIO:                 return BTFE_P_ChipSetGpio(chip_handle, handle, (SCD_CONFIG__GPIO*)data, size);
      case SCD_CONFIG_ITEM__FAT_DATA:             return BTFE_P_ChipSetFatData(chip_handle, handle, (SCD_CONFIG__FAT_DATA*)data, size);
      case SCD_CONFIG_ITEM__FAT_AGC:              return BTFE_P_ChipSetFatAgc(chip_handle, handle, (SCD_CONFIG__FAT_AGC*)data, size);
      case SCD_CONFIG_ITEM__AGC_SCRIPT:           return BTFE_P_ChipSetAgcScript(chip_handle, handle, (SCD_CONFIG__AGC_SCRIPT*)data, size);
      case SCD_CONFIG_ITEM__CHANNEL_SCAN_CONTROL: return BTFE_P_ChipSetChannelScanControl(chip_handle, handle, (SCD_CONFIG__CHANNEL_SCAN_CONTROL*)data, size); 
      case SCD_CONFIG_ITEM__ACQUISITION:          return BTFE_P_ChipSetAcquisition(chip_handle, handle, (SCD_CONFIG__ACQUISITION*)data, size);
      case SCD_CONFIG_ITEM__J83ABC:               return BTFE_P_ChipSetJ83Abc(chip_handle, handle, (SCD_CONFIG__J83ABC *)data, size);	  
      case SCD_CONFIG_ITEM__UNIFIED_COFDM:        return BTFE_P_ChipSetUnifiedCofdm(chip_handle, handle, (SCD_CONFIG__UNIFIED_COFDM *)data, size);	  
      case SCD_CONFIG_ITEM__SET_IF:               return BTFE_P_ChipSetIF(chip_handle, handle, (SCD_XPROP_TUNER_IF__DATA *)data, size);
      case SCD_CONFIG_ITEM__ISDBT_BUFFER:         return BTFE_P_ChipSetIsdbtBuffer(chip_handle, handle, (SCD_CONFIG__ISDBT_BUFFER *)data, size);	  
      case SCD_CONFIG_ITEM__AUDIO_MAG_SHIFT:      return BTFE_P_ChipSetAudioMagShift(chip_handle, handle, (SCD_CONFIG__AUDIO_MAG_SHIFT*)data, size);
#if 0
      case SCD_CONFIG_ITEM__TRY_NEXT_SCAN:        return BTFE_P_ChipTryNextScan(chip_handle, handle, (int32_t *)data, size);
      case SCD_CONFIG_ITEM__J83_NEXT_SCAN:        return BTFE_P_ChipJ83NextScan(chip_handle, handle, (SCD_CONFIG_J83_NEXT_SCAN *)data, size);
#endif
      case SCD_CONFIG_ITEM__POWER_LEVEL:        return BTFE_P_ChipSetPowerLevel(chip_handle, handle, (int16_t *)data, size);	
      case SCD_CONFIG_ITEM__RF_OFFSET:        return BTFE_P_ChipSetROffset(chip_handle, handle, (SCD_CONFIG__RF_OFFSET *)data, size);	
      case SCD_CONFIG_ITEM__POWER_SAVING:        return BTFE_P_ChipSetPowerSaving(chip_handle, handle, (int32_t *)data, size);
      default: break;
   }

   BDBG_ERR(("BTFE_P_ChipSetConfig: unknown item"));
   return SCD_RESULT__ARG_OUT_OF_RANGE;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipGetStatus(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_STATUS_ITEM item, void *data, uint32_t size)
{
    CHIP_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ChipGetStatus(chip_handle=%08X, handle=%08X, item=%u, &data=%08X, size=%u)", chip_handle, handle, item, data, size)));

    switch(item)
    {
    case SCD_STATUS_ITEM__AGC_INDICATOR:           return BTFE_P_ChipGetAgcIndicator(chip_handle, handle, (SCD_STATUS__AGC_INDICATOR*)data, size);  
    case SCD_STATUS_ITEM__BERT:                    return BTFE_P_ChipGetBert(chip_handle, handle, (SCD_STATUS__BERT*)data, size);
    case SCD_STATUS_ITEM__PSD_FRONTEND:            return BTFE_P_ChipGetPsdFrontEnd(chip_handle, handle, (SCD_STATUS__PSD*)data, size);
    case SCD_STATUS_ITEM__EQ_TAPS_PLUS:            return BTFE_P_ChipGetEqTaps(chip_handle, handle, (SCD_STATUS__EQ_TAPS_PLUS*)data, size);
    case SCD_STATUS_ITEM__CONSTELLATION_DATA:      return BTFE_P_ChipGetConstellationData(chip_handle, handle, (SCD_STATUS__CONSTELLATION_DATA*)data, size);
    case SCD_STATUS_ITEM__FAT:                     return BTFE_P_ChipGetFat(chip_handle, handle, (SCD_STATUS__FAT*)data, size);
    case SCD_STATUS_ITEM__MEMORY_READ:			      return BTFE_P_ChipGetMemoryRead(chip_handle, handle, (SCD_STATUS__MEMORY_READ*)data, size);
    case SCD_STATUS_ITEM__ACQUIRE_MODE:		      return BTFE_P_ChipGetAcquireMode(chip_handle, handle, (SCD_STATUS__ACQUIRE_MODE*)data, size);
	 case SCD_STATUS_ITEM__J83ABC:		            return BTFE_P_ChipGetJ83abcStatus(chip_handle, handle, (SCD_STATUS__J83ABC*)data, size);
	 case SCD_STATUS_ITEM__VSB:			            return BTFE_P_ChipGetVsbStatus(chip_handle, handle, (SCD_STATUS_VSB*)data, size);
	 case SCD_STATUS_ITEM__COFDM:		               return BTFE_P_ChipGetCofdmStatus(chip_handle, handle, (SCD_STATUS_COFDM*)data, size);
    default: break;
    }

    BDBG_ERR(("BTFE_P_ChipGetStatus: unknown item"));

    return SCD_RESULT__ARG_OUT_OF_RANGE;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipOpen(SCD_HANDLE chip_handle, SCD_HANDLE handle)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    CHIP *tchip = GET_CHIP(chip_handle);
    uint8_t *pMicroCode;
    void* vp;
    uint8_t temp8;
	uint8_t regVal8;

    CHIP_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ChipOpen(chip_handle=%08X, handle=%08X)", chip_handle, handle)));
	   
    switch(GET_HANDLE_TYPE(handle))
    {
    case SCD_HANDLE_CHIP:
        CHIP_DEBUG(BDBG_MSG(("BTFE_P_ChipOpen: CHIP open (handle=%08X)", handle)));

        /* get chip id */
/*
        CHIP_DEBUG(BDBG_MSG(("BTFE_P_ChipOpen: get chip id (handle=%08X)", handle)));

        ret_val = BTFE_P_ChipGetReg16(ret_val, chip_handle, ixMISC_DEV_ID, &tchip->ChipId);

        if((tchip->ChipId != 0x314) && (tchip->ChipId != 0x311) && (tchip->ChipId != 0x312))
        {
            BDBG_ERR(("BTFE_P_ChipOpen: bad chip id (%X)", tchip->ChipId));
            ret_val = SCD_RESULT__ERROR;
        }
*/

        /* initialize chip data */
        tchip->ChannelScanControl.tryFullAcq = 0;
        tchip->ChannelScanControl.ifPower = 100;
        tchip->ChannelScanControl.scanOrder = SCD_SCAN_ORDER__SCANALL;
        tchip->FatData.dataPolarity = SCD_SIGNAL_POLARITY__NO_INVERT;
        tchip->FatData.errorPolarity = SCD_SIGNAL_POLARITY__NO_INVERT;
        tchip->FatData.clockPolarity = SCD_SIGNAL_POLARITY__NO_INVERT;
        tchip->FatData.syncPolarity = SCD_SIGNAL_POLARITY__NO_INVERT;
        tchip->FatData.validPolarity = SCD_SIGNAL_POLARITY__NO_INVERT;
        tchip->FatData.BurstMode = SCD_BURST_MODE__BURST_OFF;
        tchip->FatData.GatedClockEnable = FALSE;
        tchip->FatData.ParallelOutputEnable = FALSE;
        tchip->FatData.HeaderEnable = TRUE;
        tchip->FatData.CableCardBypassEnable = FALSE;
        tchip->FatData.FlipOrder = FALSE;
        tchip->FatData.MpegOutputEnable = FALSE;
        tchip->FatData.dataStrength = SCD_SIGNAL_STRENGTH__DEFAULT;
        tchip->FatData.errorStrength = SCD_SIGNAL_STRENGTH__DEFAULT;
        tchip->FatData.clockStrength = SCD_SIGNAL_STRENGTH__DEFAULT;
        tchip->FatData.syncStrength = SCD_SIGNAL_STRENGTH__DEFAULT;
        tchip->FatData.validStrength = SCD_SIGNAL_STRENGTH__DEFAULT;
        tchip->Acquisition.TuneMode = SCD_TUNE_MODE__APP;
        tchip->Acquisition.bSpectrumInversion = 0;
        tchip->Acquisition.bCoChannelRejection = 0;
        tchip->Acquisition.bAdjChannelRejection = 0;
        tchip->Acquisition.bMobileMode = 0;
        tchip->Acquisition.bEnhancedMode = 0;
        tchip->Acquisition.bLowPriority = 0;
        tchip->Acquisition.ifFrequency = SCD_IF_FREQUENCY__DEFAULT;
        tchip->Acquisition.bLegacyAGC = 0;   /* mcg June 2009.  1 = Legacy AGC   0 = UAGC  */ 
        tchip->FatModFormat = SCD_MOD_FORMAT__UNKNOWN;
        tchip->pIFDconfig = 0;
        tchip->pUAGC_IF_VGAconfig = 0;
        tchip->pUAGCdigitalConfig = 0;
        tchip->pUAGCanalogNegConfig = 0;
        tchip->pUAGCanalogPosConfig = 0;
        tchip->BertInput = SCD_BERT_INPUT__NONE;
        tchip->FwVersion = 0;
        tchip->FatIfFrequency = 44000000;
        tchip->tunerIfFrequency = 0;
        tchip->IfFrequencyShift = 0;

	BTFE_P_InitUagcData(tchip);

        /* init clocks */
        CHIP_DEBUG(BDBG_MSG(("BTFE_P_ChipOpen: clock init (handle=%08X)", handle)));
        ret_val = BTFE_P_ChipDoPllSetup(ret_val, chip_handle);

        /* Power down all */
        CHIP_DEBUG(BDBG_MSG(("BTFE_P_ChipOpen: power down all (handle=%08X)", handle)));
        BTFE_P_ChipPower(ret_val, chip_handle, SCD_FE_POWER_DOWN_ALL);

        /* Power up the necessary sections */
        CHIP_DEBUG(BDBG_MSG(("BTFE_P_ChipOpen: power up (handle=%08X)", handle)));
        ret_val = BTFE_P_ChipPower(ret_val, chip_handle, SCD_FE_POWER_UP_FAT) ; /* QQQ | SCD_FE_POWER_UP_FDC); */

        /* reset all */
        ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, &temp8);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_agc, 1);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_fe, 1);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_equalizer, 1);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_fec, 1);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_ofs, 1);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_dvbt, 1);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_ntsc, 1);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_bert, 1);
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, temp8);
        ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, &temp8);
        MODIFYFLD(temp8, MISC_MODULATION_FMT_RESET_CONTROL_2, soft_reset_adc0_fifo, 1);
        MODIFYFLD(temp8, MISC_MODULATION_FMT_RESET_CONTROL_2, OFS_burst_rate, 1);
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, temp8);

        /* unreset all */
        ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, &temp8);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_agc, 0);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_fe, 0);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_equalizer, 0);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_fec, 0);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_ofs, 0);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_dvbt, 0);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_ntsc, 0);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_bert, 0);
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, temp8);
        ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, &temp8);
        MODIFYFLD(temp8, MISC_MODULATION_FMT_RESET_CONTROL_2, soft_reset_adc0_fifo, 0);
        MODIFYFLD(temp8, MISC_MODULATION_FMT_RESET_CONTROL_2, OFS_burst_rate, 0);
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_MODULATION_FMT_RESET_CONTROL_2, temp8);

        /* download firmware if needed */
#ifndef BBS_DOWNLOAD_FW
        CHIP_DEBUG(BDBG_MSG(("BTFE_P_ChipOpen: download firmware (handle=%08X)", handle)));
        ret_val = BTFE_P_ScdGetUser(chip_handle, (void**) &vp);
        pMicroCode = (unsigned char *)vp;
        ret_val = BTFE_P_ChipCheckMicrocode(ret_val, chip_handle, pMicroCode);
        pMicroCode = (unsigned char *)vp; 
        if (ret_val != SCD_RESULT__OK)
        {  
          ret_val = SCD_RESULT__OK; /* this is a must*/
          ret_val = BTFE_P_ChipLoadMicrocode(ret_val, chip_handle, pMicroCode);
        }
#endif
        ret_val = BTFE_P_ChipGetReg24(ret_val, chip_handle, ixMISC_RAM_VER_VALUE_2, &tchip->FwVersion);

        /* setup ADCs */
        CHIP_DEBUG(BDBG_MSG(("BTFE_P_ChipOpen: ADC setup (handle=%08X)", handle)));
        ret_val = BTFE_P_ChipDoAdc0Setup(ret_val, chip_handle);
        ret_val = BTFE_P_ChipDoAdc1Setup(ret_val, chip_handle);

        /* clear firmware boot bit */
        BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_RAM_VER_VALUE_2, &temp8);
        temp8 &= ~(1<<7);
        BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_RAM_VER_VALUE_2, temp8);
        break;

    case SCD_HANDLE_FAT:

        FAT_DEBUG(BDBG_MSG(("BTFE_P_ChipOpen: FAT open (handle=%08X)", handle)));

		/* Nov 2009 - Enable ADC FIFO 
		 *
		 *   AGC_CONTROL 0xf101
		 *   go_bit_agc 2 0x0 Static
		 *   0 - AGC halt  1 - Normal Operation
		 --------------------------------------------------------------------------------------------- */

		if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixAGC_CONTROL, &regVal8)) == SCD_RESULT__OK)
		{
			MODIFYFLD(regVal8, AGC_CONTROL, go_bit_agc, 1);
			ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixAGC_CONTROL, regVal8);
		}

		/* Nov 2009 - Enable AGC Delta-Sigma output */

		if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_CORE_PADS_OE_CNTRL, &regVal8)) == SCD_RESULT__OK)
		{
			MODIFYFLD(regVal8, MISC_CORE_PADS_OE_CNTRL, pgm_agc1_pads_oen, 0);    /* 0 = Enable */
			ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_CORE_PADS_OE_CNTRL, regVal8);
		}

		if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_CORE_PADS_OE_CNTRL, &regVal8)) == SCD_RESULT__OK)
		{
			MODIFYFLD(regVal8, MISC_CORE_PADS_OE_CNTRL, pgm_agc2_pads_oen, 0);    /* 0 = Enable */
			ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_CORE_PADS_OE_CNTRL, regVal8);
		}
		
		/* Nov 2009 - Enable AGC Delta-Sigma output synchronizer */
		/* MISC_AGC_SYNC_START    [W]    8 bits    Access: 8    primaryRegisterAperture:0xf063  
			agc_synchronizer_start 0 0x0
			0 - External AGC synchronizer is off(Default)
			1 - External AGC synchronizer is on
		*/

		if((ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_AGC_SYNC_START, &regVal8)) == SCD_RESULT__OK)
		{
			MODIFYFLD(regVal8, MISC_AGC_SYNC_START, agc_synchronizer_start, 1);
			ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_AGC_SYNC_START, regVal8);
		}


        break;

    default:
        BDBG_ERR(("BTFE_P_ChipOpen: unknown handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipClose(SCD_HANDLE chip_handle, SCD_HANDLE handle)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;

    CHIP_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ChipClose(chip_handle=%08X, handle=%08X)", chip_handle, handle)));

    (void) chip_handle;

    switch(GET_HANDLE_TYPE(handle))
    {
    case SCD_HANDLE_CHIP:
        CHIP_DEBUG(BDBG_MSG(("BTFE_P_ChipClose: CHIP close (handle=%08X)", handle)));

        /* put chip in low power state */
        CHIP_DEBUG(BDBG_MSG(("BTFE_P_ChipClose: power down (handle=%08X)", handle)));

        if((ret_val = BTFE_P_ChipPower(ret_val, chip_handle, SCD_FE_POWER_DOWN_ALL)) != SCD_RESULT__OK)
        {
            BDBG_WRN(("BTFE_P_ChipClose: power down error (%u)", ret_val));
        }
        break;

    case SCD_HANDLE_FAT:
        FAT_DEBUG(BDBG_MSG(("BTFE_P_ChipClose: FAT close (handle=%08X)", handle)));
        break;

    default:
        BDBG_ERR(("BTFE_P_ChipClose: unknown handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipStart(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_MOD_FORMAT format)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    CHIP *tchip = GET_CHIP(chip_handle);
    SCD_DEMOD *demod;
    uint8_t temp8;

    CHIP_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ChipStart(chip_handle=%08X, handle=%08X, format=%08X)", chip_handle, handle, format)));

#if !BTFE_BBS
    /* if format is J83B and not in symbolrateVerify mode, then use the Legacy Qam64/Qam256 */
    if ((format == SCD_MOD_FORMAT__FAT_J83ABC) && (tchip->Acquisition.acqConfig <= SCD_FULL_ACQUIRE))
    {
        if (tchip->j83abc.j83abcMode == J83B)
        {
            if (tchip->j83abc.qamMode == QAM_MODE_DIGITAL_64QAM)
                format = SCD_MOD_FORMAT__FAT_QAM64;
            else if (tchip->j83abc.qamMode == QAM_MODE_DIGITAL_256QAM)
                format = SCD_MOD_FORMAT__FAT_QAM256;				
        }
    }
#endif

    tchip->rfOffset.freqOffset = 0;
    tchip->rfOffset.symbolRate = 0;
	
    switch(GET_HANDLE_TYPE(handle))
    {
    case SCD_HANDLE_FAT:
        FAT_DEBUG(BDBG_MSG(("BTFE_P_ChipStart: FAT start (handle=%08X)", handle)));

        /* reset FAT AGC */
        ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, &temp8);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_agc, 1);
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, temp8);
        MODIFYFLD(temp8, MISC_RESET_CONTROL_0, soft_reset_to_agc, 0);
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_RESET_CONTROL_0, temp8);

        tchip->FatModFormat = format;

        switch(format)
        {
        case SCD_MOD_FORMAT__FAT_VSB:
#ifdef SCD_LEGACY_QAM
        case SCD_MOD_FORMAT__FAT_QAM64:
        case SCD_MOD_FORMAT__FAT_QAM256:
#endif
        case SCD_MOD_FORMAT__FAT_J83ABC:		
        case SCD_MOD_FORMAT__FAT_NTSC_M:
        case SCD_MOD_FORMAT__FAT_NTSC_N:
        case SCD_MOD_FORMAT__FAT_NTSC_J:
        case SCD_MOD_FORMAT__FAT_NTSC_443:
        case SCD_MOD_FORMAT__FAT_PAL_I:
        case SCD_MOD_FORMAT__FAT_PAL_B:
        case SCD_MOD_FORMAT__FAT_PAL_B1:
        case SCD_MOD_FORMAT__FAT_PAL_G:
        case SCD_MOD_FORMAT__FAT_PAL_H:
        case SCD_MOD_FORMAT__FAT_PAL_D:
        case SCD_MOD_FORMAT__FAT_PAL_K:
        case SCD_MOD_FORMAT__FAT_PAL_60:
        case SCD_MOD_FORMAT__FAT_PAL_M:
        case SCD_MOD_FORMAT__FAT_PAL_N:
        case SCD_MOD_FORMAT__FAT_PAL_NC:
        case SCD_MOD_FORMAT__FAT_SECAM_B:
        case SCD_MOD_FORMAT__FAT_SECAM_D:
        case SCD_MOD_FORMAT__FAT_SECAM_G:
        case SCD_MOD_FORMAT__FAT_SECAM_H:
        case SCD_MOD_FORMAT__FAT_SECAM_K:
        case SCD_MOD_FORMAT__FAT_SECAM_K1:
        case SCD_MOD_FORMAT__FAT_SECAM_L:
        case SCD_MOD_FORMAT__FAT_SECAM_L1:
        case SCD_MOD_FORMAT__FAT_COFDM_8:
        case SCD_MOD_FORMAT__FAT_COFDM_7:
        case SCD_MOD_FORMAT__FAT_COFDM_6:
        case SCD_MOD_FORMAT__FAT_COFDM_5:
        case SCD_MOD_FORMAT__FAT_UNIFIED_COFDM:  ret_val = BTFE_P_ChipStartFat(ret_val, chip_handle, format);  break;
/* Starting Sep 2009 - not supported
        case SCD_MOD_FORMAT__FAT_AUTO:     ret_val = BTFE_P_ChipStartFatAuto(ret_val, chip_handle);      break;
*/
        default:
            BDBG_ERR(("BTFE_P_ChipStart: bad FAT format"));
            return SCD_RESULT__ARG_OUT_OF_RANGE;
        }

        demod = (SCD_DEMOD *) handle;

        if(ret_val == SCD_RESULT__OK) demod->state = SCD_STATE_RUNNING;
        break;

    default:
        BDBG_ERR(("BTFE_P_ChipStart: unknown handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipStop(SCD_HANDLE chip_handle, SCD_HANDLE handle)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;

    CHIP_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ChipStop(chip_handle=%08X, handle=%08X)", chip_handle, handle)));

    switch(GET_HANDLE_TYPE(handle))
    {
    case SCD_HANDLE_FAT:
        FAT_DEBUG(BDBG_MSG(("BTFE_P_ChipStop: FAT stop (handle=%08X)", handle)));
        ret_val = BTFE_P_ChipStopFat(ret_val, chip_handle);
        break;

    default:
        BDBG_ERR(("BTFE_P_ChipStop: unknown handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipWriteGPIO(SCD_HANDLE chip_handle, uint32_t mask, uint32_t value)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    uint8_t temp8;

    CHIP_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ChipWriteGPIO(chip_handle=%08X, mask=%08X, value=%08X)", chip_handle, mask, value)));

    if(mask & 0x000000FF)
    {
        ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_GPIO_7_0_DOUT, &temp8);
        temp8 &= ~mask;
        temp8 |= (value & mask);
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_GPIO_7_0_DOUT, temp8);
    }

    if(mask & 0x0000FF00)
    {
        ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_GPIO_15_8_DOUT, &temp8);
        temp8 &= ~(mask>>8);
        temp8 |= (value & mask)>>8;
        ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_GPIO_15_8_DOUT, temp8);
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipReadGPIO(SCD_HANDLE chip_handle, uint32_t mask, uint32_t *value)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    uint8_t v;

    CHIP_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ChipReadGPIO(chip_handle=%08X, mask=%08X, &value=%08X)", chip_handle, mask, value)));

    *value = 0;

    if(mask & 0x000000FF)
    {
        ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_GPIO_7_0_DOUT, &v);

        *value |= v & mask;
    }

    if(mask & 0x0000FF00)
    {
        ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixMISC_GPIO_15_8_DOUT, &v);

        *value |= (((uint32_t) v) << 8) & mask;
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipWriteI2C(SCD_HANDLE chip_handle, uint32_t i2c_addr, uint32_t i2c_subaddr, uint32_t subaddr_len, uint32_t data_len, uint8_t *buffer)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    uint8_t args[16];
    uint8_t i, length;

    CHIP_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ChipWriteI2C(chip_handle=%08X, i2c_addr=%08X, i2c_subaddr=%08X, subaddr_len=%u, data_len=%u, &buffer=%08X)", chip_handle, i2c_addr, i2c_subaddr, subaddr_len, data_len, buffer)));

    i = 1;
	length = (uint8_t)(data_len & 0x7F);

    /* slave address (read bit cleared) */
    args[i++] = (uint8_t) (i2c_addr & ~0x01);

    /* subaddress */
    if(subaddr_len >= 4) args[i++] = (uint8_t) ((i2c_subaddr & 0xFF000000) >> 24);
    if(subaddr_len >= 3) args[i++] = (uint8_t) ((i2c_subaddr & 0x00FF0000) >> 16);
    if(subaddr_len >= 2) args[i++] = (uint8_t) ((i2c_subaddr & 0x0000FF00) >> 8);
    if(subaddr_len >= 1) args[i++] = (uint8_t) ((i2c_subaddr & 0x000000FF) >> 0);

    if((length + subaddr_len) > 14)
    {
        return SCD_RESULT__ERROR;
    }

    if(buffer)
    {
        while(length--)
        {
            args[i++] = *buffer++;
        }
    }

    /* allow extended writes */
	args[0] = (uint8_t) ((0x80 & data_len) | (i -1));

    ret_val = BTFE_P_ChipDoMicroService(ret_val, chip_handle, SVC_I2C_WRITE, i, args, 0, NO_RESULTS, I2C_TIMEOUT);

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipReadI2C(SCD_HANDLE chip_handle, uint32_t i2c_addr, uint32_t i2c_subaddr, uint32_t subaddr_len, uint32_t data_len, uint8_t *buffer)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    uint8_t args[2];

    CHIP_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ChipReadI2C(chip_handle=%08X, i2c_addr=%08X, i2c_subaddr=%08X, subaddr_len=%u, data_len=%u, &buffer=%08X)", chip_handle, i2c_addr, i2c_subaddr, subaddr_len, data_len, buffer)));

    if(data_len > HI_SVC_RES_0_LENGTH)
    {
        return SCD_RESULT__ERROR;
    }

    if(subaddr_len)
    {
        /* write subaddr */
        ret_val =  BTFE_P_ChipWriteI2C(chip_handle, i2c_addr, i2c_subaddr, subaddr_len, 0, 0);
    }

    /* read data length */
    args[0] = (uint8_t) (data_len);

    /* slave address (read bit set) */
    args[1] = (uint8_t) (i2c_addr | 0x01);

    ret_val = BTFE_P_ChipDoMicroService(ret_val, chip_handle, SVC_I2C_READ, 2, args, (uint8_t) data_len, buffer, I2C_TIMEOUT);

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipRead(SCD_HANDLE chip_handle, uint32_t aper, uint32_t offset, uint32_t length, uint8_t *values)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;

    CHIP_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ChipRead(chip_handle=%08X, aper=%u, offset=%08X, length=%u, &values=%08X)", chip_handle, aper, offset, length, values)));
    {
        uint32_t instance;

        if((ret_val = BTFE_P_ScdGetInstance(chip_handle, &instance)) == SCD_RESULT__OK)
        {
            ret_val = BTFE_P_HalReadChip(instance, aper, offset, length, values);
        }
    }

    return ret_val;

}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipSetFrequency(SCD_HANDLE chip_handle, SCD_HANDLE handle, uint32_t frequency, SCD_MOD_FORMAT mod_format)
{
    SCD_RESULT ret_val=SCD_RESULT__OK;
    CHIP *tchip = GET_CHIP(chip_handle);
    (void) mod_format;
    BSTD_UNUSED(frequency);
    BSTD_UNUSED(handle);

    CHIP_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ChipSetFrequency(chip_handle=%08X, frequency=%u, mod_format=%u)", chip_handle, frequency, mod_format)));

    /*tchip->tunerIfFrequency = VSB_DEFAULT_IF_FREQUENCY;*/

    switch(mod_format)
    {
        case SCD_MOD_FORMAT__FAT_COFDM_8:
        case SCD_MOD_FORMAT__FAT_COFDM_7:
        case SCD_MOD_FORMAT__FAT_COFDM_6:
        case SCD_MOD_FORMAT__FAT_COFDM_5:   tchip->tunerIfFrequency = COFDM_DEFAULT_IF_FREQUENCY; break;

        case SCD_MOD_FORMAT__FAT_VSB:       tchip->tunerIfFrequency = VSB_DEFAULT_IF_FREQUENCY; break;

#ifdef SCD_LEGACY_QAM
        case SCD_MOD_FORMAT__FAT_QAM64:
        case SCD_MOD_FORMAT__FAT_QAM256:    tchip->tunerIfFrequency = QAM_DEFAULT_IF_FREQUENCY; break;
#endif
        case SCD_MOD_FORMAT__FAT_J83ABC:    tchip->tunerIfFrequency = QAM_DEFAULT_IF_FREQUENCY; break;
		
        case SCD_MOD_FORMAT__FAT_NTSC_M:
        case SCD_MOD_FORMAT__FAT_NTSC_N:
        case SCD_MOD_FORMAT__FAT_NTSC_J:
        case SCD_MOD_FORMAT__FAT_NTSC_443:  tchip->tunerIfFrequency = NTSC_DEFAULT_IF_FREQUENCY;
                                           /* tchip->IfFrequencyShift = 300000;*/
          break;
        case SCD_MOD_FORMAT__FAT_PAL_I:
        case SCD_MOD_FORMAT__FAT_PAL_B:
        case SCD_MOD_FORMAT__FAT_PAL_B1:
        case SCD_MOD_FORMAT__FAT_PAL_G:
        case SCD_MOD_FORMAT__FAT_PAL_H:
        case SCD_MOD_FORMAT__FAT_PAL_D:
        case SCD_MOD_FORMAT__FAT_PAL_K:
        case SCD_MOD_FORMAT__FAT_PAL_60:
        case SCD_MOD_FORMAT__FAT_PAL_M:
        case SCD_MOD_FORMAT__FAT_PAL_N:
        case SCD_MOD_FORMAT__FAT_PAL_NC:    tchip->tunerIfFrequency = PAL_DEFAULT_IF_FREQUENCY; break;

        case SCD_MOD_FORMAT__FAT_SECAM_B:
        case SCD_MOD_FORMAT__FAT_SECAM_D:
        case SCD_MOD_FORMAT__FAT_SECAM_G:
        case SCD_MOD_FORMAT__FAT_SECAM_H:
        case SCD_MOD_FORMAT__FAT_SECAM_K:
        case SCD_MOD_FORMAT__FAT_SECAM_K1:
        case SCD_MOD_FORMAT__FAT_SECAM_L:
        case SCD_MOD_FORMAT__FAT_SECAM_L1:  tchip->tunerIfFrequency = PAL_DEFAULT_IF_FREQUENCY; break;

        default:
          tchip->tunerIfFrequency = 0;
          break;
    }

    if(ret_val != SCD_RESULT__OK) 
    {
       BDBG_ERR(("BTFE_P_ChipSetFrequency: error=%u", ret_val));
    }

    return ret_val;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipGetVersion(SCD_HANDLE chip_handle, SCD_ITEM item, SCD_VERSION *version)
{
    uint8_t temp[3];
    uint32_t chip_instance;

    if(version)
    {
        switch(item)
        {
        case SCD_ITEM__CHIP:
        case SCD_ITEM__FAT:
            version->name = "X233";
            version->major = MAJOR;
            version->customer = CUSTOMER;
            version->minor = MINOR;
            version->device_id = 0;
            return SCD_RESULT__OK;

        case SCD_ITEM__FIRMWARE:
            version->name = "X233 FW";
            version->major = MAJOR;
            version->customer = CUSTOMER;
            version->minor = MINOR;
            version->device_id = 0;

            if(BTFE_P_ScdGetInstance(chip_handle, &chip_instance) == SCD_RESULT__OK)
            {
                if(BTFE_P_HalReadChip(chip_instance, 0, ixMISC_RAM_VER_VALUE_2, 3, temp) == SCD_RESULT__OK)
                {
                    version->major = temp[0];
                    version->customer = temp[1];
                    version->minor = temp[2];
                    version->device_id = (g_chip[chip_instance]).FwCRCvalue;                
                }
            }
            return SCD_RESULT__OK;

        default:
            return SCD_RESULT__ARG_OUT_OF_RANGE;
        }
    }

    return SCD_RESULT__ARG_OUT_OF_RANGE;
}

/****************************************************************************/

#define TEST_PATTERN 0xABCDEF5A

static SCD_RESULT BTFE_P_ChipTestChip(uint32_t chip_instance)
{
#ifdef chip_NO_TEST
    (void) chip_instance;
#else
    SCD_RESULT r;
    uint32_t wreg;
    uint32_t rreg;

    /* write pattern 1 */
    wreg = TEST_PATTERN;

    if((r = BTFE_P_HalWriteChip(chip_instance, 0, ixUC_GP_28, 4, (uint8_t *) &wreg)) != SCD_RESULT__OK) return r;
    if((r = BTFE_P_HalReadChip(chip_instance, 0, ixUC_GP_28, 4, (uint8_t *) &rreg)) != SCD_RESULT__OK) return r;

    if(rreg != TEST_PATTERN)
    {
        BDBG_ERR(("BTFE_P_ChipTestChip: write=%08X, read=%08X", wreg, rreg));
        return SCD_RESULT__CHIP_NOT_AVAILABLE;
    }

    /* write pattern 2 */
    wreg = ~TEST_PATTERN;

    if((r = BTFE_P_HalWriteChip(chip_instance, 0, ixUC_GP_28, 4, (uint8_t *) &wreg)) != SCD_RESULT__OK) return r;
    if((r = BTFE_P_HalReadChip(chip_instance, 0, ixUC_GP_28, 4, (uint8_t *) &rreg)) != SCD_RESULT__OK) return r;

    if(rreg != ~TEST_PATTERN)
    {
        BDBG_ERR(("BTFE_P_ChipTestChip: write=%08X, read=%08X", wreg, rreg));
        return SCD_RESULT__CHIP_NOT_AVAILABLE;
    }
#endif

    return SCD_RESULT__OK;
}

/****************************************************************************/
/* public functions */
/****************************************************************************/

SCD_RESULT BTFE_P_X233AddChip(uint32_t chip_instance, uint32_t fat_tuner_instance, uint32_t fdc_tuner_instance, SCD_HANDLE *chip_handle, uint8_t *pMicroCode)
{
    SCD_RESULT r;
    SCD_CHIP_FUNCTIONS functions;
    BSTD_UNUSED(fdc_tuner_instance);

    CHIP_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ChipAddChip(chip_instance=%u, fat_tuner_instance=%u, fdc_tuner_instance=%u, &chip_handle=%08X, &pMicroCode=%08X)", chip_instance, fat_tuner_instance, fdc_tuner_instance, chip_handle, pMicroCode)));

    BKNI_Memset(&functions, 0, sizeof(SCD_CHIP_FUNCTIONS));

    functions.get_version       = BTFE_P_ChipGetVersion;
    functions.open              = BTFE_P_ChipOpen;
    functions.close             = BTFE_P_ChipClose;
    functions.start             = BTFE_P_ChipStart;
    functions.stop              = BTFE_P_ChipStop;
    functions.set_frequency     = BTFE_P_ChipSetFrequency;
    functions.set_config        = BTFE_P_ChipSetConfig;
    functions.get_status        = BTFE_P_ChipGetStatus;
    functions.write_gpio        = BTFE_P_ChipWriteGPIO;
    functions.read_gpio         = BTFE_P_ChipReadGPIO;
    functions.write_i2c         = BTFE_P_ChipWriteI2C;
    functions.read_i2c          = BTFE_P_ChipReadI2C;
    functions.write             = BTFE_P_ChipWrite;
    functions.read              = BTFE_P_ChipRead;

    if((r = BTFE_P_ScdAddChip(chip_instance, &functions, chip_handle, pMicroCode)) != SCD_RESULT__OK) return r;

    if(((r = BTFE_P_ChipTestChip(chip_instance)) != SCD_RESULT__OK))
    {
        BDBG_ERR(("BTFE_P_ChipAddChip: chip access failed"));
        return r;
    }

    if((r = BTFE_P_ScdAddFat(chip_instance, fat_tuner_instance, "X233 FAT", 0)) != SCD_RESULT__OK) return r;
  
    return SCD_RESULT__OK;
}

/****************************************************************************/

static SCD_RESULT BTFE_P_ChipCheckMicrocode(SCD_RESULT ret_val, SCD_HANDLE chip_handle, uint8_t *pMicroCode)
{
    CHIP *tchip = GET_CHIP(chip_handle);
    uint32_t temp32;
    uint32_t MicroCodeLen = 0;
    uint32_t crcValue = 0;
    uint8_t *p;
	uint8_t temp8;
    
    tchip->FwCRCvalue = 0;

        p = pMicroCode;

        /* Verify the header information, and gather the length */
        if(ret_val == SCD_RESULT__OK)
        {
            temp32 = (*p<<24) + (*(p+1)<<16) + (*(p+2)<<8) + (*(p+3));

            if (temp32 !=  0x58323333) /* 'X233' */
            {
                BDBG_WRN(("BTFE_P_ChipLoadMicrocode: bad header 1"));
                ret_val = SCD_RESULT__BAD_FIRMWARE;
            }
        }

        if(ret_val == SCD_RESULT__OK)
        {
            p += 4;
            temp32 = (*p<<24) + (*(p+1)<<16) + (*(p+2)<<8) + (*(p+3));
			/* ' ATI' or ' AMD' or " BCM"*/
            if((temp32 != 0x20415449) && (temp32 != 0x20414D44) && (temp32 != 0x2042434D)) 
            {
                BDBG_WRN(("BTFE_P_ChipLoadMicrocode: bad header 2"));
                ret_val =  SCD_RESULT__BAD_FIRMWARE;
            }
        }

        if(ret_val == SCD_RESULT__OK)
        {
            p += 12; /* Now at the number of Bytes in segment 1 header */
            temp32 = (*p<<24) + (*(p+1)<<16) + (*(p+2)<<8) + (*(p+3));

            if(temp32 >= (FW_SIZE*1024)) /* just make sure the value is reasonable */
            {
                BDBG_WRN(("BTFE_P_ChipLoadMicrocode: bad segment 1 size"));
                ret_val =  SCD_RESULT__BAD_FIRMWARE;
            }

            MicroCodeLen = temp32;
        }

        if(ret_val == SCD_RESULT__OK)
        {
            p += 8;
            temp32 = (*p<<24) + (*(p+1)<<16) + (*(p+2)<<8) + (*(p+3));
            crcValue = temp32;
            p += 4;
            tchip->FwCRCvalue = crcValue; 
        }

        {
            uint32_t testCodeLen = 150;
            unsigned char *b;

            if((b = (unsigned char *)BKNI_Malloc(testCodeLen)) != 0)
            {
                BDBG_MSG(("BTFE_P_ChipLoadMicrocode: verifying microcode"));

				/* Enable RAM Access */
				temp8 = 0;
				MODIFYFLD(temp8, MISC_UC_FEC_PSD_AGC_RAM_I2C_CNTRL, i2c_ram_access_enable, 1);
				ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixMISC_UC_FEC_PSD_AGC_RAM_I2C_CNTRL, temp8);

				/* Set for instruction memory */
				ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixAPERTURE_SEL, &temp8);
				temp8 = 1;
				ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixAPERTURE_SEL, temp8);

				/* Read testCodeLen of bytes for microcode comparison */
                ret_val = BTFE_P_ChipGetRegs(ret_val, chip_handle, 0, testCodeLen, b);

				/* Set back for data memory */
				ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixAPERTURE_SEL, &temp8);
				temp8 = 0;
				ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixAPERTURE_SEL, temp8);

                if(BKNI_Memcmp(p, b, testCodeLen))
                {
                    BDBG_MSG(("BTFE_P_ChipLoadMicrocode: microcode compare failed"));
                    ret_val = SCD_RESULT__ERROR;
                }
                else
                {
                    BDBG_MSG(("BTFE_P_ChipLoadMicrocode: microcode compare ok"));
                    ret_val = SCD_RESULT__OK; 
                }
                BKNI_Free(b);
            }
        }

        if (ret_val == SCD_RESULT__OK)
        {
        	/* clear service request & status */
              ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_SERVICE_REQUEST, 0x00);
        	ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_SERVICE_STATUS_0, 0x00);
       	ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_SERVICE_STATUS_1, 0x00);
        	ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixHI_SERVICE_STATUS_2, 0x00);

	 	/* reset the micro */
        	ret_val = BTFE_P_ChipGetReg8(ret_val, chip_handle, ixUC_MEM_AGC_DWNLD_UPLD_CNTRL, &temp8);
        	MODIFYFLD(temp8, UC_MEM_AGC_DWNLD_UPLD_CNTRL, uc_reset, 1);
        	ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixUC_MEM_AGC_DWNLD_UPLD_CNTRL, temp8);
		MODIFYFLD(temp8, UC_MEM_AGC_DWNLD_UPLD_CNTRL, uc_reset, 0);
		ret_val = BTFE_P_ChipSetReg8(ret_val, chip_handle, ixUC_MEM_AGC_DWNLD_UPLD_CNTRL, temp8);	
	}
	
        return ret_val;
}
