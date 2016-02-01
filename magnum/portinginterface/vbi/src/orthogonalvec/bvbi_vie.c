
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

#include "bstd.h"                       /* standard types */
#include "bdbg.h"                       /* Dbglib */
#include "bvbi.h"                       /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"          /* VBI internal data structures */
#include "bchp_vec_cfg.h"

BDBG_MODULE(BVBI);

/* This is better than having many ifdefs scattered throughout the file */
#if (BCHP_CHIP != 7125) && (BCHP_CHIP != 7340) && (BCHP_CHIP != 7408) && \
    (BCHP_CHIP != 7420) && (BCHP_CHIP != 7468) /** { **/
#define BCHP_VEC_CFG_SW_RESET_CCE_0        BCHP_VEC_CFG_SW_INIT_CCE_0
#define BCHP_VEC_CFG_SW_RESET_WSE_0        BCHP_VEC_CFG_SW_INIT_WSE_0
#define BCHP_VEC_CFG_SW_RESET_TTE_0        BCHP_VEC_CFG_SW_INIT_TTE_0
#define BCHP_VEC_CFG_SW_RESET_GSE_0        BCHP_VEC_CFG_SW_INIT_GSE_0
#define BCHP_VEC_CFG_SW_RESET_AMOLE_0      BCHP_VEC_CFG_SW_INIT_AMOLE_0
#define BCHP_VEC_CFG_SW_RESET_CGMSAE_0     BCHP_VEC_CFG_SW_INIT_CGMSAE_0
#endif /** } BCHP... **/

#if (BCHP_CHIP != 7420) /** { **/
#define BCHP_VEC_CFG_SW_RESET_CCE_ANCIL_0   BCHP_VEC_CFG_SW_INIT_CCE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_WSE_ANCIL_0   BCHP_VEC_CFG_SW_INIT_WSE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_TTE_ANCIL_0   BCHP_VEC_CFG_SW_INIT_TTE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_GSE_ANCIL_0   BCHP_VEC_CFG_SW_INIT_GSE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_AMOLE_ANCIL_0                             \
    BCHP_VEC_CFG_SW_INIT_AMOLE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_ANCI656_ANCIL_0                           \
    BCHP_VEC_CFG_SW_INIT_ANCI656_ANCIL_0
#endif /** } BCHP... **/

/* This will make code more legible, in special cases. Like, chipsets that do
 * not support 656 output.
 */
#if (BVBI_NUM_CCE_656 == 0)
#undef BCHP_VEC_CFG_SW_RESET_CCE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_CCE_ANCIL_0 0xFFFFFFFF
#endif
#if (BVBI_NUM_WSE_656 == 0)
#undef BCHP_VEC_CFG_SW_RESET_WSE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_WSE_ANCIL_0 0xFFFFFFFF
#endif
#if (BVBI_NUM_TTE_656 == 0)
#undef BCHP_VEC_CFG_SW_RESET_TTE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_TTE_ANCIL_0 0xFFFFFFFF
#endif
#if (BVBI_NUM_GSE_656 == 0)
#undef BCHP_VEC_CFG_SW_RESET_GSE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_GSE_ANCIL_0 0xFFFFFFFF
#endif
#if (BVBI_NUM_AMOLE_656 == 0)
#undef BCHP_VEC_CFG_SW_RESET_AMOLE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_AMOLE_ANCIL_0 0xFFFFFFFF
#endif

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/


/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/


/***************************************************************************
* Implementation of supporting VBI_ENC functions that are not in API
***************************************************************************/

BERR_Code BVBI_P_VIE_SoftReset_isr (
        BREG_Handle hReg,
        bool is656,
        uint8_t hwCoreIndex,
        uint32_t whichStandard)
{
        uint32_t ulRegBase;
        uint32_t ulRegAddr;

        BDBG_ENTER(BVBI_P_VIE_SoftReset_isr);

        switch (whichStandard)
        {
        case BVBI_P_SELECT_CC:
                ulRegBase =
                        (is656 ?
                                BCHP_VEC_CFG_SW_RESET_CCE_ANCIL_0 :
                                BCHP_VEC_CFG_SW_RESET_CCE_0);
                break;
#if (BVBI_NUM_TTE > 0) || (BVBI_NUM_TTE_656 > 0)
        case BVBI_P_SELECT_TT:
                ulRegBase =
                        (is656 ?
                                BCHP_VEC_CFG_SW_RESET_TTE_ANCIL_0 :
                                BCHP_VEC_CFG_SW_RESET_TTE_0);
                break;
#endif
        case BVBI_P_SELECT_WSS:
                ulRegBase =
                        (is656 ?
                                BCHP_VEC_CFG_SW_RESET_WSE_ANCIL_0 :
                                BCHP_VEC_CFG_SW_RESET_WSE_0);
                break;
#if (BVBI_NUM_GSE > 0)
        case BVBI_P_SELECT_GS:
                ulRegBase =
                        (is656 ?
                                BCHP_VEC_CFG_SW_RESET_GSE_ANCIL_0 :
                                BCHP_VEC_CFG_SW_RESET_GSE_0);
                break;
#endif
#if (BVBI_NUM_AMOLE > 0)
        case BVBI_P_SELECT_AMOL:
                ulRegBase =
                        (is656 ?
                                BCHP_VEC_CFG_SW_RESET_AMOLE_ANCIL_0 :
                                BCHP_VEC_CFG_SW_RESET_AMOLE_0);
                break;
#endif
        case BVBI_P_SELECT_CGMSA:
        case BVBI_P_SELECT_CGMSB:
                ulRegBase = BCHP_VEC_CFG_SW_RESET_CGMSAE_0;
                break;
#if (BVBI_NUM_SCTEE > 0)
        case BVBI_P_SELECT_SCTE:
                ulRegBase = BCHP_VEC_CFG_SW_RESET_SCTE_0;
                break;
#endif
        default:
                /* This should never happen! */
                ulRegBase = 0xFFFFFFFF;
                break;
        }

        /* Take care of errors above */
        if (ulRegBase == 0xFFFFFFFF)
        {
                BDBG_LEAVE(BVBI_P_VIE_SoftReset_isr);
                return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        /* Finally, program the soft reset register */
        ulRegAddr = ulRegBase + 4 * hwCoreIndex;
        BREG_Write32 (hReg, ulRegAddr, 0x1);
        BREG_Write32 (hReg, ulRegAddr, 0x0);

        BDBG_LEAVE(BVBI_P_VIE_SoftReset_isr);
        return BERR_SUCCESS;
}

#if (BVBI_NUM_ANCI656_656 > 0)
BERR_Code BVBI_P_VIE_AncilSoftReset (
        BREG_Handle hReg,
        uint8_t hwCoreIndex)
{
        uint32_t ulRegBase;
        uint32_t ulRegAddr;

        BDBG_ENTER(BVBI_P_VIE_AncilSoftReset);

        /* Figure out which encoder core to use */
        ulRegBase = BCHP_VEC_CFG_SW_RESET_ANCI656_ANCIL_0;
        ulRegAddr = ulRegBase + 4 * hwCoreIndex;

        /* Program the soft reset register */
        BREG_Write32 (hReg, ulRegAddr, 0x1);
        BREG_Write32 (hReg, ulRegAddr, 0x0);

        BDBG_LEAVE(BVBI_P_AncilSoftReset);
        return BERR_SUCCESS;
}
#endif

/***************************************************************************
* Static (private) functions
***************************************************************************/

/* End of file */
