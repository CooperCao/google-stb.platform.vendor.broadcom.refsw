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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

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
#include "btfe_scd_chip_priv.h"
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

#if (BCHP_CHIP == 35233)
#include "bchp_35233.h"
#else
#include "bchp_7543.h"
#endif

#include "breg_mem.h"

BDBG_MODULE(btfe_scd_hal_mips);

#if (BCHP_CHIP == 35233)
#define BCHP_DFE_BASE_ADDR 0x100000
#else
#define BCHP_DFE_BASE_ADDR 0xe00000
#endif

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

#include "bcm7543_fw_a0.c"  /* bcm7543_ap_image array */



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

    g_X233micro[CHIP_0] = bcm7543_ap_image;

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

SCD_RESULT BTFE_P_HalWriteChip(uint32_t offset, uint32_t length, uint8_t *buffer)
{

	/*some place is still using 16 bit address type, can't track down one last place using it*/
	if (offset < 0x10000)
		offset = BCHP_DFE_BASE_ADDR+(offset << 2);

	while(length--){
		Dfe_WriteReg32(CHIP_INDEX, offset, (*buffer++));
		offset += 4;
	}

	return SCD_RESULT__OK;

}

/****************************************************************************/

SCD_RESULT BTFE_P_HalReadChip(uint32_t offset, uint32_t length, uint8_t *buffer)
{

	/*some place is still using 16 bit address type, can't track down one last place using it*/
	if (offset < 0x10000)
		offset = BCHP_DFE_BASE_ADDR+(offset << 2);

	while(length--){
		*buffer++ = (uint8_t)(Dfe_ReadReg32(CHIP_INDEX, offset) & 0xFF);
		offset += 4;
	}

	return SCD_RESULT__OK;

}

/****************************************************************************/

SCD_RESULT BTFE_P_HalCloseChip(uint32_t instance)
{
    (void) instance;

    HAL_FUNC_DEBUG(BDBG_MSG(("BTFE_P_HalCloseChip(instance=%u)", instance)));

    return SCD_RESULT__OK;
}


/****************************************************************************/
