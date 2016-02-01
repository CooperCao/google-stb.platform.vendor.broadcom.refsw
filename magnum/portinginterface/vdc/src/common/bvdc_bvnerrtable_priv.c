/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
 *   See Module Overview below.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include "bstd.h"
#include "bdbg.h"
#include "bvdc.h"
#include "bvdc_priv.h"
#include "bvdc_dbg.h"

#include "bchp_rdc.h"
#include "bchp_int_id_bvnb_intr2.h"
#include "bchp_bvnb_intr2.h"
#include "bchp_int_id_bvnf_intr2_5.h"
#include "bchp_bvnf_intr2_5.h"

#ifdef BCHP_BVNM_INTR2_0_REG_START
#include "bchp_int_id_bvnm_intr2_0.h"
#include "bchp_bvnm_intr2_0.h"
#endif

#if BCHP_BVNM_INTR2_1_REG_START
#include "bchp_int_id_bvnm_intr2_1.h"
#include "bchp_bvnm_intr2_1.h"
#endif

#ifdef BCHP_BVNB_INTR2_1_REG_START
#include "bchp_int_id_bvnb_intr2_1.h"
#endif

#include "bchp_mfd_0.h"
#include "bchp_vfd_0.h"
#include "bchp_cap_0.h"
#include "bchp_scl_0.h"
#include "bchp_gfd_0.h"
#include "bchp_cmp_0.h"

#ifdef BCHP_XSRC_0_REG_START
#include "bchp_xsrc_0.h"
#endif

#ifdef BCHP_DNR_0_REG_START
#include "bchp_dnr_0.h"
#endif

#ifdef BCHP_HSCL_0_REG_START
#include "bchp_hscl_0.h"
#endif

#ifdef BCHP_MVP_TOP_0_REG_START
#include "bchp_mvp_top_0.h"
#endif

#ifdef BCHP_MDI_TOP_0_REG_START
#include "bchp_mdi_top_0.h"
#endif

#ifdef BCHP_HD_ANR_MCTF_0_REG_START
#include "bchp_hd_anr_mctf_0.h"
#endif

#ifdef BCHP_MAD_0_REG_START
#include "bchp_mad_0.h"
#endif

#ifdef BCHP_TNTD_0_REG_START
#include "bchp_tntd_0.h"
#endif

BDBG_MODULE(BVDC_DBG);

/* Null int id */
#define BVDC_P_NULL_BINTID                      ((BINT_Id)(-1))

/* Short hand for table entry */
#define BVDC_P_MAKE_BVN_ERR(enum_name, int_id, L2ClearReg, groupbase, base, bvbStatus) \
{                                                                           \
	BVDC_BvnError_##enum_name,                                              \
	BCHP_INT_ID_##int_id##_INTR,                                            \
	BCHP_##L2ClearReg##_CLEAR,                                              \
	BCHP_##L2ClearReg##_CLEAR_##int_id##_INTR_MASK,                         \
	BCHP_##groupbase##_REG_START,                                           \
	BCHP_##base##_REG_START,                                                \
	(BCHP_##bvbStatus - BCHP_##groupbase##_REG_START),                      \
	BVDC_P_BvnErrorHandler_isr,                                             \
	BDBG_STRING(#enum_name),                                                \
}

#define BVDC_P_MAKE_INVALID(enum_name, int_id, L2ClearReg, groupbase, base, bvbStatus) \
{                                                                           \
	BVDC_BvnError_##enum_name,                                              \
	BVDC_P_NULL_BINTID,                                                     \
	0,                                                                      \
	0,                                                                      \
	0,                                                                      \
	0,                                                                      \
	0,                                                                      \
	NULL,                                                                   \
	BDBG_STRING(#enum_name),                                                \
}

/* count of bint entry */
#define BVDC_P_BVN_ERR_COUNT \
	(sizeof(s_apfErrorHandlers) / sizeof(BVDC_P_IntCbTbl))


/***************************************************************************
 *
 */
static const BVDC_P_IntCbTbl s_apfErrorHandlers[] =
{
#if (BCHP_CHIP==7400)
	BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

	BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_1,    MFD_1,        BVNF_INTR2_5_R5F, MFD_0,         MFD_1,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eMfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_2,    VFD_2,        BVNF_INTR2_5_R5F, VFD_0,         VFD_2,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_3,    VFD_3,        BVNF_INTR2_5_R5F, VFD_0,         VFD_3,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eVfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNF_INTR2_5_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNF_INTR2_5_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_2,    SCL_2_ERR,    BVNF_INTR2_5_R5F, SCL_0,         SCL_2,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_3,    SCL_3_ERR,    BVNF_INTR2_5_R5F, SCL_0,         SCL_3,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eScl_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNF_INTR2_5_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eDnr_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMad_0,    MAD_BVB_IN,   BVNF_INTR2_5_R5F, MAD_0,         MAD_0,         MAD_0_BVB_IN_STATUS),

	BVDC_P_MAKE_INVALID(eMvp_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMcdi_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMctf_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eHscl_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_2,    CAP2,         BVNB_INTR2_CPU,   CAP_0,         CAP_2,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_3,    CAP3,         BVNB_INTR2_CPU,   CAP_0,         CAP_3,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_INVALID(eCap_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_2,    GFD2,         BVNB_INTR2_CPU,   GFD_0,         GFD_2,         GFD_0_STATUS),
	BVDC_P_MAKE_INVALID(eGfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_2_V0, CMP2_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_3_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V1, CMP0_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V1_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V1, CMP1_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V1_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_2_G0, CMP2_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_3_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif (BCHP_CHIP==7335) || (BCHP_CHIP==7325)
	BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

	BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eMfd_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eVfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNF_INTR2_5_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNF_INTR2_5_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eScl_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNF_INTR2_5_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eDnr_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMad_0,    MAD_BVB_IN,   BVNF_INTR2_5_R5F, MAD_0,         MAD_0,         MAD_0_BVB_IN_STATUS),

	BVDC_P_MAKE_INVALID(eMvp_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMcdi_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMctf_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eHscl_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_INVALID(eCap_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
	BVDC_P_MAKE_INVALID(eGfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_V1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_1_V1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif (BCHP_CHIP==7340) || (BCHP_CHIP==7342) || (BCHP_CHIP==7125) || (BCHP_CHIP==7408) || (BCHP_CHIP==7468)
	BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

	BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eMfd_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eVfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNF_INTR2_5_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNF_INTR2_5_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eScl_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNF_INTR2_5_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eDnr_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMad_0,    MAD_BVB_IN,   BVNF_INTR2_5_R5F, MAD_0,         MAD_0,         MAD_0_BVB_IN_STATUS),

	BVDC_P_MAKE_INVALID(eMvp_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMcdi_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMctf_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eHscl_0,   MAD_HSCL,     BVNF_INTR2_5_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eHscl_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_INVALID(eCap_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
	BVDC_P_MAKE_INVALID(eGfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_V1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_1_V1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif (BCHP_CHIP==7550)
	BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

	BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eMfd_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eVfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNF_INTR2_5_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNF_INTR2_5_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eScl_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eDnr_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMad_0,    MAD_BVB_IN,   BVNF_INTR2_5_R5F, MAD_0,         MAD_0,         MAD_0_BVB_IN_STATUS),

	BVDC_P_MAKE_INVALID(eMvp_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMcdi_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMctf_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eHscl_0,   MAD_HSCL,     BVNF_INTR2_5_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eHscl_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_INVALID(eCap_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
	BVDC_P_MAKE_INVALID(eGfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_V1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_1_V1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif ((BCHP_CHIP==7552) || (BCHP_CHIP==7358) || (BCHP_CHIP==7360) || \
	   (BCHP_CHIP==7362) || (BCHP_CHIP==7228) || (BCHP_CHIP==7364) || \
	   (BCHP_CHIP==7250) || (BCHP_CHIP==73625))
	BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

	BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eMfd_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eVfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eScl_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eDnr_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMad_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#if (BCHP_CHIP==7364) || (BCHP_CHIP==7250) || (BCHP_CHIP==73625)
	BVDC_P_MAKE_BVN_ERR(eMvp_0,    MVP_TOP_0,   BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),
#else
	BVDC_P_MAKE_BVN_ERR(eMvp_0,    MAD_BVB_IN,   BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),
#endif
	BVDC_P_MAKE_INVALID(eMvp_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMcdi_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMctf_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#if (BCHP_CHIP==7364)|| (BCHP_CHIP==7250) || (BCHP_CHIP==73625)
	BVDC_P_MAKE_BVN_ERR(eHscl_0,   HSCL_0,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
#else
	BVDC_P_MAKE_BVN_ERR(eHscl_0,   MAD_HSCL,     BVNM_INTR2_0_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
#endif
	BVDC_P_MAKE_INVALID(eHscl_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_INVALID(eCap_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
	BVDC_P_MAKE_INVALID(eGfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_V1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_1_V1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif (BCHP_CHIP==7346) || (BCHP_CHIP==7344) || (BCHP_CHIP==7231) || (BCHP_CHIP==7429) || (BCHP_CHIP==74295) || (BCHP_CHIP==73465)
	BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

	BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_1,    MFD_1,        BVNF_INTR2_5_R5F, MFD_0,         MFD_1,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eMfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_2,    VFD_2,        BVNF_INTR2_5_R5F, VFD_0,         VFD_2,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_3,    VFD_3,        BVNF_INTR2_5_R5F, VFD_0,         VFD_3,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eVfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_2,    SCL_2_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_2,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_3,    SCL_3_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_3,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eScl_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eDnr_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMad_0,    MAD_BVB_IN,   BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),

	BVDC_P_MAKE_INVALID(eMvp_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMcdi_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMctf_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eHscl_0,   MAD_HSCL,     BVNM_INTR2_0_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eHscl_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_2,    CAP2,         BVNB_INTR2_CPU,   CAP_0,         CAP_2,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_3,    CAP3,         BVNB_INTR2_CPU,   CAP_0,         CAP_3,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_INVALID(eCap_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
	BVDC_P_MAKE_INVALID(eGfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V1, CMP0_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V1_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V1, CMP1_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V1_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif (BCHP_CHIP==7420)
	BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

	BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_1,    MFD_1,        BVNF_INTR2_5_R5F, MFD_0,         MFD_1,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eMfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_2,    VFD_2,        BVNF_INTR2_5_R5F, VFD_0,         VFD_2,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_3,    VFD_3,        BVNF_INTR2_5_R5F, VFD_0,         VFD_3,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eVfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNF_INTR2_5_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNF_INTR2_5_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_2,    SCL_2_ERR,    BVNF_INTR2_5_R5F, SCL_0,         SCL_2,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_3,    SCL_3_ERR,    BVNF_INTR2_5_R5F, SCL_0,         SCL_3,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eScl_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNF_INTR2_5_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eDnr_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMad_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMvp_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMcdi_0,   MCDI_BVB_IN,  BVNF_INTR2_5_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eMcdi_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMctf_0,   MCTF,         BVNF_INTR2_5_R5F, HD_ANR_MCTF_0, HD_ANR_MCTF_0, HD_ANR_MCTF_0_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eHscl_0,   HSCL,         BVNF_INTR2_5_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eHscl_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_2,    CAP2,         BVNB_INTR2_CPU,   CAP_0,         CAP_2,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_3,    CAP3,         BVNB_INTR2_CPU,   CAP_0,         CAP_3,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_INVALID(eCap_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_2,    GFD2,         BVNB_INTR2_CPU,   GFD_0,         GFD_2,         GFD_0_STATUS),
	BVDC_P_MAKE_INVALID(eGfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_2_V0, CMP2_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_3_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V1, CMP0_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V1_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V1, CMP1_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V1_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_2_G0, CMP2_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_3_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif (BCHP_CHIP==7422) || ((BCHP_CHIP==7425) && (BCHP_VER < BCHP_VER_B0))
	BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

	BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_1,    MFD_1,        BVNF_INTR2_5_R5F, MFD_0,         MFD_1,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_2,    MFD_2,        BVNF_INTR2_5_R5F, MFD_0,         MFD_2,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eMfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_2,    VFD_2,        BVNF_INTR2_5_R5F, VFD_0,         VFD_2,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_3,    VFD_3,        BVNF_INTR2_5_R5F, VFD_0,         VFD_3,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_4,    VFD_4,        BVNF_INTR2_5_R5F, VFD_0,         VFD_4,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eVfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_2,    SCL_2_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_2,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_3,    SCL_3_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_3,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_4,    SCL_4_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_4,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eScl_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eDnr_1,    DNR_1_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_1,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eDnr_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMad_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMvp_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMcdi_0,   MDI_0_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_1,   MDI_1_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_1,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eMcdi_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMctf_0,   MCTF_0,       BVNM_INTR2_0_R5F, HD_ANR_MCTF_0, HD_ANR_MCTF_0, HD_ANR_MCTF_0_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eHscl_0,   HSCL_0,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_1,   HSCL_1,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_1,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eHscl_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_2,    CAP2,         BVNB_INTR2_CPU,   CAP_0,         CAP_2,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_3,    CAP3,         BVNB_INTR2_CPU,   CAP_0,         CAP_3,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_4,    CAP4,         BVNB_INTR2_CPU,   CAP_0,         CAP_4,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_INVALID(eCap_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_2,    GFD2,         BVNB_INTR2_CPU,   GFD_0,         GFD_2,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_3,    GFD3,         BVNB_INTR2_CPU,   GFD_0,         GFD_3,         GFD_0_STATUS),
	BVDC_P_MAKE_INVALID(eGfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_2_V0, CMP2_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_3_V0, CMP3_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_3,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_4_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V1, CMP0_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V1_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V1, CMP1_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V1_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_2_G0, CMP2_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_3_G0, CMP3_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_3,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_4_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif ((BCHP_CHIP==7425) && (BCHP_VER >= BCHP_VER_B0))
	BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

	BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_1,    MFD_1,        BVNF_INTR2_5_R5F, MFD_0,         MFD_1,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_2,    MFD_2,        BVNF_INTR2_5_R5F, MFD_0,         MFD_2,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eMfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_2,    VFD_2,        BVNF_INTR2_5_R5F, VFD_0,         VFD_2,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_3,    VFD_3,        BVNF_INTR2_5_R5F, VFD_0,         VFD_3,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_4,    VFD_4,        BVNF_INTR2_5_R5F, VFD_0,         VFD_4,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eVfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_2,    SCL_2_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_2,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_3,    SCL_3_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_3,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_4,    SCL_4_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_4,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eScl_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eDnr_1,    DNR_1_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_1,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eDnr_2,    DNR_2_ERR,    BVNM_INTR2_1_R5F, DNR_0,         DNR_2,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eDnr_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMad_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMvp_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMcdi_0,   MDI_0_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_1,   MDI_1_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_1,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_2,   MDI_2_BVB_IN, BVNM_INTR2_1_R5F, MDI_TOP_0,     MDI_TOP_2,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eMcdi_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMctf_0,   MCTF_0,       BVNM_INTR2_0_R5F, HD_ANR_MCTF_0, HD_ANR_MCTF_0, HD_ANR_MCTF_0_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eHscl_0,   HSCL_0,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_1,   HSCL_1,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_1,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_2,   HSCL_2,       BVNM_INTR2_1_R5F, HSCL_0,        HSCL_2,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eHscl_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_2,    CAP2,         BVNB_INTR2_CPU,   CAP_0,         CAP_2,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_3,    CAP3,         BVNB_INTR2_CPU,   CAP_0,         CAP_3,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_4,    CAP4,         BVNB_INTR2_CPU,   CAP_0,         CAP_4,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_INVALID(eCap_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_2,    GFD2,         BVNB_INTR2_CPU,   GFD_0,         GFD_2,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_3,    GFD3,         BVNB_INTR2_CPU,   GFD_0,         GFD_3,         GFD_0_STATUS),
	BVDC_P_MAKE_INVALID(eGfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_2_V0, CMP2_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_3_V0, CMP3_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_3,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_4_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V1, CMP0_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V1_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V1, CMP1_V1,      BVNB_INTR2_CPU,   CMP_1,         CMP_1,         CMP_0_V1_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_2_G0, CMP2_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_3_G0, CMP3_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_3,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_4_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif (BCHP_CHIP==7435)
	BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

	BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_1,    MFD_1,        BVNF_INTR2_5_R5F, MFD_0,         MFD_1,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_2,    MFD_2,        BVNF_INTR2_5_R5F, MFD_0,         MFD_2,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_3,    MFD_3,        BVNF_INTR2_5_R5F, MFD_0,         MFD_3,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_2,    VFD_2,        BVNF_INTR2_5_R5F, VFD_0,         VFD_2,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_3,    VFD_3,        BVNF_INTR2_5_R5F, VFD_0,         VFD_3,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_4,    VFD_4,        BVNF_INTR2_5_R5F, VFD_0,         VFD_4,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_5,    VFD_5,        BVNF_INTR2_5_R5F, VFD_0,         VFD_5,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eVfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_2,    SCL_2_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_2,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_3,    SCL_3_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_3,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_4,    SCL_4_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_4,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_5,    SCL_5_ERR,    BVNM_INTR2_1_R5F, SCL_0,         SCL_5,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eScl_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eDnr_1,    DNR_1_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_1,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eDnr_2,    DNR_2_ERR,    BVNM_INTR2_1_R5F, DNR_0,         DNR_2,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eDnr_3,    DNR_3_ERR,    BVNM_INTR2_1_R5F, DNR_0,         DNR_3,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMad_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMvp_0,    MVP_TOP_0,    BVNM_INTR2_0_R5F, MVP_TOP_0,     MVP_TOP_0,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMvp_1,    MVP_TOP_1,    BVNM_INTR2_0_R5F, MVP_TOP_0,     MVP_TOP_1,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMvp_2,    MVP_TOP_2,    BVNM_INTR2_1_R5F, MVP_TOP_0,     MVP_TOP_2,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMvp_3,    MVP_TOP_3,    BVNM_INTR2_1_R5F, MVP_TOP_0,     MVP_TOP_3,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMvp_4,    MVP_TOP_4,    BVNM_INTR2_1_R5F, MVP_TOP_0,     MVP_TOP_4,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMvp_5,    MVP_TOP_4,    BVNM_INTR2_1_R5F, MVP_TOP_0,     MVP_TOP_4,     MVP_TOP_0_STATUS),

	BVDC_P_MAKE_BVN_ERR(eMcdi_0,   MDI_0_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_1,   MDI_1_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_1,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_2,   MDI_2_BVB_IN, BVNM_INTR2_1_R5F, MDI_TOP_0,     MDI_TOP_2,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_3,   MDI_3_BVB_IN, BVNM_INTR2_1_R5F, MDI_TOP_0,     MDI_TOP_3,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_4,   MDI_4_BVB_IN, BVNM_INTR2_1_R5F, MDI_TOP_0,     MDI_TOP_4,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMctf_0,   MCTF_0,       BVNM_INTR2_0_R5F, HD_ANR_MCTF_0, HD_ANR_MCTF_0, HD_ANR_MCTF_0_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eHscl_0,   HSCL_0,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_1,   HSCL_1,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_1,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_2,   HSCL_2,       BVNM_INTR2_1_R5F, HSCL_0,        HSCL_2,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_3,   HSCL_3,       BVNM_INTR2_1_R5F, HSCL_0,        HSCL_3,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_4,   HSCL_4,       BVNM_INTR2_1_R5F, HSCL_0,        HSCL_4,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_5,   HSCL_4,       BVNM_INTR2_1_R5F, HSCL_0,        HSCL_4,        HSCL_0_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_2,    CAP2,         BVNB_INTR2_CPU,   CAP_0,         CAP_2,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_3,    CAP3,         BVNB_INTR2_CPU,   CAP_0,         CAP_3,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_4,    CAP4,         BVNB_INTR2_CPU,   CAP_0,         CAP_4,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_5,    CAP5,         BVNB_INTR2_CPU,   CAP_0,         CAP_5,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_INVALID(eCap_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_2,    GFD2,         BVNB_INTR2_CPU,   GFD_0,         GFD_2,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_3,    GFD3,         BVNB_INTR2_CPU,   GFD_0,         GFD_3,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_4,    GFD4,         BVNB_INTR2_CPU,   GFD_0,         GFD_4,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_5,    GFD5,         BVNB_INTR2_CPU,   GFD_0,         GFD_5,         GFD_0_STATUS),
	BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_2_V0, CMP2_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_3_V0, CMP3_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_3,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_4_V0, CMP4_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_4,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_5_V0, CMP5_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_5,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V1, CMP0_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V1_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V1, CMP1_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V1_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_2_G0, CMP2_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_3_G0, CMP3_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_3,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_4_G0, CMP4_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_4,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_5_G0, CMP5_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_5,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif (BCHP_CHIP==7584) || (BCHP_CHIP==75845)
	BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

	BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_1,    MFD_1,        BVNF_INTR2_5_R5F, MFD_0,         MFD_1,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eMfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_2,    VFD_2,        BVNF_INTR2_5_R5F, VFD_0,         VFD_2,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_3,    VFD_3,        BVNF_INTR2_5_R5F, VFD_0,         VFD_3,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eVfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_2,    SCL_2_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_2,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_3,    SCL_3_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_3,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eScl_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eDnr_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMad_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMvp_0,    MVP_TOP_0,    BVNM_INTR2_0_R5F, MVP_TOP_0,     MVP_TOP_0,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_INVALID(eMvp_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMcdi_0,   MDI_0_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eMcdi_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMctf_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eHscl_0,   HSCL_0,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eHscl_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_2,    CAP2,         BVNB_INTR2_CPU,   CAP_0,         CAP_2,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_3,    CAP3,         BVNB_INTR2_CPU,   CAP_0,         CAP_3,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_INVALID(eCap_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
	BVDC_P_MAKE_INVALID(eGfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V1, CMP0_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V1_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V1, CMP1_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V1_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif (BCHP_CHIP==7563) || (BCHP_CHIP==7543) || (BCHP_CHIP==75635) || (BCHP_CHIP==75525)

	BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

	BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eMfd_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eVfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eScl_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eDnr_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMad_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMvp_0,    MVP_TOP_0,    BVNM_INTR2_0_R5F, MVP_TOP_0,     MVP_TOP_0,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_INVALID(eMvp_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMcdi_0,   MDI_0_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eMcdi_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMctf_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eHscl_0,   HSCL_0,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eHscl_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_INVALID(eCap_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
	BVDC_P_MAKE_INVALID(eGfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_V1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_1_V1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif (BCHP_CHIP==7445) || (BCHP_CHIP==11360)
	BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

	BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_1,    MFD_1,        BVNF_INTR2_5_R5F, MFD_0,         MFD_1,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_2,    MFD_2,        BVNF_INTR2_5_R5F, MFD_0,         MFD_2,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_3,    MFD_3,        BVNF_INTR2_5_R5F, MFD_0,         MFD_3,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_4,    MFD_4,        BVNF_INTR2_5_R5F, MFD_0,         MFD_4,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_5,    MFD_5,        BVNF_INTR2_5_R5F, MFD_0,         MFD_5,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),

	BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_2,    VFD_2,        BVNF_INTR2_5_R5F, VFD_0,         VFD_2,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_3,    VFD_3,        BVNF_INTR2_5_R5F, VFD_0,         VFD_3,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_4,    VFD_4,        BVNF_INTR2_5_R5F, VFD_0,         VFD_4,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_5,    VFD_5,        BVNF_INTR2_5_R5F, VFD_0,         VFD_5,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_6,    VFD_6,        BVNF_INTR2_5_R5F, VFD_0,         VFD_6,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_7,    VFD_7,        BVNF_INTR2_5_R5F, VFD_0,         VFD_7,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),

	BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_2,    SCL_2_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_2,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_3,    SCL_3_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_3,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_4,    SCL_4_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_4,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_5,    SCL_5_ERR,    BVNM_INTR2_1_R5F, SCL_0,         SCL_5,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_6,    SCL_6_ERR,    BVNM_INTR2_1_R5F, SCL_0,         SCL_6,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_7,    SCL_7_ERR,    BVNM_INTR2_1_R5F, SCL_0,         SCL_7,         SCL_0_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eDnr_1,    DNR_1_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_1,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eDnr_2,    DNR_2_ERR,    BVNM_INTR2_1_R5F, DNR_0,         DNR_2,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eDnr_3,    DNR_3_ERR,    BVNM_INTR2_1_R5F, DNR_0,         DNR_3,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eDnr_4,    DNR_4_ERR,    BVNM_INTR2_1_R5F, DNR_0,         DNR_4,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eDnr_5,    DNR_5_ERR,    BVNM_INTR2_1_R5F, DNR_0,         DNR_5,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eXsrc_0,   XSRC_0_ERR,   BVNM_INTR2_0_R5F, XSRC_0,        XSRC_0,        XSRC_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eXsrc_1,   XSRC_1_ERR,   BVNM_INTR2_0_R5F, XSRC_0,        XSRC_1,        XSRC_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eTntd_0,   TNTD_0_ERR,   BVNM_INTR2_0_R5F, TNTD_0,        TNTD_0,        TNTD_0_BVB_IN_STATUS),

	BVDC_P_MAKE_INVALID(eMad_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMvp_0,    MVP_TOP_0,    BVNM_INTR2_0_R5F, MVP_TOP_0,     MVP_TOP_0,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMvp_1,    MVP_TOP_1,    BVNM_INTR2_0_R5F, MVP_TOP_0,     MVP_TOP_1,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMvp_2,    MVP_TOP_2,    BVNM_INTR2_1_R5F, MVP_TOP_0,     MVP_TOP_2,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMvp_3,    MVP_TOP_3,    BVNM_INTR2_1_R5F, MVP_TOP_0,     MVP_TOP_3,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMvp_4,    MVP_TOP_4,    BVNM_INTR2_1_R5F, MVP_TOP_0,     MVP_TOP_4,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMvp_5,    MVP_TOP_5,    BVNM_INTR2_1_R5F, MVP_TOP_0,     MVP_TOP_5,     MVP_TOP_0_STATUS),

	BVDC_P_MAKE_BVN_ERR(eMcdi_0,   MDI_0_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_1,   MDI_1_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_1,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_2,   MDI_2_BVB_IN, BVNM_INTR2_1_R5F, MDI_TOP_0,     MDI_TOP_2,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_3,   MDI_3_BVB_IN, BVNM_INTR2_1_R5F, MDI_TOP_0,     MDI_TOP_3,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_4,   MDI_4_BVB_IN, BVNM_INTR2_1_R5F, MDI_TOP_0,     MDI_TOP_4,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_5,   MDI_5_BVB_IN, BVNM_INTR2_1_R5F, MDI_TOP_0,	  MDI_TOP_5,	 MDI_TOP_0_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eMctf_0,   MCTF_0,       BVNM_INTR2_0_R5F, HD_ANR_MCTF_0, HD_ANR_MCTF_0, HD_ANR_MCTF_0_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eHscl_0,   HSCL_0,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_1,   HSCL_1,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_1,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_2,   HSCL_2,       BVNM_INTR2_1_R5F, HSCL_0,        HSCL_2,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_3,   HSCL_3,       BVNM_INTR2_1_R5F, HSCL_0,        HSCL_3,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_4,   HSCL_4,       BVNM_INTR2_1_R5F, HSCL_0,        HSCL_4,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_5,   HSCL_5,       BVNM_INTR2_1_R5F, HSCL_0,        HSCL_5,        HSCL_0_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_2,    CAP2,         BVNB_INTR2_CPU,   CAP_0,         CAP_2,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_3,    CAP3,         BVNB_INTR2_CPU,   CAP_0,         CAP_3,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_4,    CAP4,         BVNB_INTR2_CPU,   CAP_0,         CAP_4,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_5,    CAP5,         BVNB_INTR2_CPU,   CAP_0,         CAP_5,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_6,    CAP6,         BVNB_INTR2_CPU,   CAP_0,         CAP_6,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_7,    CAP7,         BVNB_INTR2_CPU,   CAP_0,         CAP_7,         CAP_0_BVB_STATUS),

	BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_2,    GFD2,         BVNB_INTR2_CPU,   GFD_0,         GFD_2,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_3,    GFD3,         BVNB_INTR2_CPU,   GFD_0,         GFD_3,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_4,    GFD4,         BVNB_INTR2_CPU,   GFD_0,         GFD_4,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_5,    GFD5,         BVNB_INTR2_CPU,   GFD_0,         GFD_5,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_6,    GFD6,         BVNB_INTR2_1_CPU, GFD_0,         GFD_6,         GFD_0_STATUS),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_2_V0, CMP2_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_3_V0, CMP3_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_3,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_4_V0, CMP4_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_4,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_5_V0, CMP5_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_5,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_6_V0, CMP6_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_6,         CMP_0_V0_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V1, CMP0_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V1_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V1, CMP1_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V1_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_2_G0, CMP2_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_3_G0, CMP3_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_3,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_4_G0, CMP4_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_4,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_5_G0, CMP5_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_5,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_6_G0, CMP6_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_6,         CMP_0_G0_BVB_IN_STATUS),

	BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif (BCHP_CHIP==7145)
	BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

	BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_1,    MFD_1,        BVNF_INTR2_5_R5F, MFD_0,         MFD_1,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_2,    MFD_2,        BVNF_INTR2_5_R5F, MFD_0,         MFD_2,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_3,    MFD_3,        BVNF_INTR2_5_R5F, MFD_0,         MFD_3,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_2,    VFD_2,        BVNF_INTR2_5_R5F, VFD_0,         VFD_2,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_3,    VFD_3,        BVNF_INTR2_5_R5F, VFD_0,         VFD_3,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_4,    VFD_4,        BVNF_INTR2_5_R5F, VFD_0,         VFD_4,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_5,    VFD_5,        BVNF_INTR2_5_R5F, VFD_0,         VFD_5,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_6,    VFD_6,        BVNF_INTR2_5_R5F, VFD_0,         VFD_6,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_7,    VFD_7,        BVNF_INTR2_5_R5F, VFD_0,         VFD_7,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),

	BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_2,    SCL_2_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_2,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_3,    SCL_3_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_3,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_4,    SCL_4_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_4,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_5,    SCL_5_ERR,    BVNM_INTR2_1_R5F, SCL_0,         SCL_5,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_6,    SCL_6_ERR,    BVNM_INTR2_1_R5F, SCL_0,         SCL_6,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_7,    SCL_7_ERR,    BVNM_INTR2_1_R5F, SCL_0,         SCL_7,         SCL_0_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eDnr_1,    DNR_1_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_1,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eDnr_2,    DNR_2_ERR,    BVNM_INTR2_1_R5F, DNR_0,         DNR_2,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eDnr_3,    DNR_3_ERR,    BVNM_INTR2_1_R5F, DNR_0,         DNR_3,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMad_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMvp_0,    MVP_TOP_0,    BVNM_INTR2_0_R5F, MVP_TOP_0,     MVP_TOP_0,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMvp_1,    MVP_TOP_1,    BVNM_INTR2_0_R5F, MVP_TOP_0,     MVP_TOP_1,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMvp_2,    MVP_TOP_2,    BVNM_INTR2_1_R5F, MVP_TOP_0,     MVP_TOP_2,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMvp_3,    MVP_TOP_3,    BVNM_INTR2_1_R5F, MVP_TOP_0,     MVP_TOP_3,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMvp_4,    MVP_TOP_4,    BVNM_INTR2_1_R5F, MVP_TOP_0,     MVP_TOP_4,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_INVALID(eMvp_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMcdi_0,   MDI_0_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_1,   MDI_1_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_1,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_2,   MDI_2_BVB_IN, BVNM_INTR2_1_R5F, MDI_TOP_0,     MDI_TOP_2,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_3,   MDI_3_BVB_IN, BVNM_INTR2_1_R5F, MDI_TOP_0,     MDI_TOP_3,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_4,   MDI_4_BVB_IN, BVNM_INTR2_1_R5F, MDI_TOP_0,     MDI_TOP_4,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMctf_0,   MCTF_0,       BVNM_INTR2_0_R5F, HD_ANR_MCTF_0, HD_ANR_MCTF_0, HD_ANR_MCTF_0_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eHscl_0,   HSCL_0,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_1,   HSCL_1,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_1,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_2,   HSCL_2,       BVNM_INTR2_1_R5F, HSCL_0,        HSCL_2,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_3,   HSCL_3,       BVNM_INTR2_1_R5F, HSCL_0,        HSCL_3,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_4,   HSCL_4,       BVNM_INTR2_1_R5F, HSCL_0,        HSCL_4,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eHscl_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_2,    CAP2,         BVNB_INTR2_CPU,   CAP_0,         CAP_2,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_3,    CAP3,         BVNB_INTR2_CPU,   CAP_0,         CAP_3,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_4,    CAP4,         BVNB_INTR2_CPU,   CAP_0,         CAP_4,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_5,    CAP5,         BVNB_INTR2_CPU,   CAP_0,         CAP_5,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_6,    CAP6,         BVNB_INTR2_CPU,   CAP_0,         CAP_6,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_7,    CAP7,         BVNB_INTR2_CPU,   CAP_0,         CAP_7,         CAP_0_BVB_STATUS),

	BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_2,    GFD2,         BVNB_INTR2_CPU,   GFD_0,         GFD_2,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_3,    GFD3,         BVNB_INTR2_CPU,   GFD_0,         GFD_3,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_4,    GFD4,         BVNB_INTR2_CPU,   GFD_0,         GFD_4,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_5,    GFD5,         BVNB_INTR2_CPU,   GFD_0,         GFD_5,         GFD_0_STATUS),
	BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_2_V0, CMP2_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_3_V0, CMP3_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_3,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_4_V0, CMP4_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_4,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_5_V0, CMP5_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_5,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V1, CMP0_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V1_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V1, CMP1_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V1_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_2_G0, CMP2_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_3_G0, CMP3_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_3,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_4_G0, CMP4_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_4,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_5_G0, CMP5_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_5,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif  ((BCHP_CHIP==7439) && (BCHP_VER>=BCHP_VER_B0))
    BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

    BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
    BVDC_P_MAKE_BVN_ERR(eMfd_1,    MFD_1,        BVNF_INTR2_5_R5F, MFD_0,         MFD_1,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
    BVDC_P_MAKE_BVN_ERR(eMfd_2,    MFD_2,        BVNF_INTR2_5_R5F, MFD_0,         MFD_2,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
    BVDC_P_MAKE_BVN_ERR(eMfd_3,    MFD_3,        BVNF_INTR2_5_R5F, MFD_0,         MFD_3,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
    BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
    BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
    BVDC_P_MAKE_BVN_ERR(eVfd_2,    VFD_2,        BVNF_INTR2_5_R5F, VFD_0,         VFD_2,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
    BVDC_P_MAKE_BVN_ERR(eVfd_3,    VFD_3,        BVNF_INTR2_5_R5F, VFD_0,         VFD_3,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
    BVDC_P_MAKE_BVN_ERR(eVfd_4,    VFD_4,        BVNF_INTR2_5_R5F, VFD_0,         VFD_4,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
    BVDC_P_MAKE_BVN_ERR(eVfd_5,    VFD_5,        BVNF_INTR2_5_R5F, VFD_0,         VFD_5,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
    BVDC_P_MAKE_INVALID(eVfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eVfd_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eScl_2,    SCL_2_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_2,         SCL_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eScl_3,    SCL_3_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_3,         SCL_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eScl_4,    SCL_4_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_4,         SCL_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eScl_5,    SCL_5_ERR,    BVNM_INTR2_1_R5F, SCL_0,         SCL_5,         SCL_0_BVB_IN_STATUS),
    BVDC_P_MAKE_INVALID(eScl_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eScl_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eDnr_1,    DNR_1_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_1,         DNR_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eDnr_2,    DNR_2_ERR,    BVNM_INTR2_1_R5F, DNR_0,         DNR_2,         DNR_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eDnr_3,    DNR_3_ERR,    BVNM_INTR2_1_R5F, DNR_0,         DNR_3,         DNR_0_BVB_IN_STATUS),
    BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eXsrc_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eXsrc_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_INVALID(eMad_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eMvp_0,    MVP_TOP_0,    BVNM_INTR2_0_R5F, MVP_TOP_0,     MVP_TOP_0,     MVP_TOP_0_STATUS),
    BVDC_P_MAKE_BVN_ERR(eMvp_1,    MVP_TOP_1,    BVNM_INTR2_0_R5F, MVP_TOP_0,     MVP_TOP_1,     MVP_TOP_0_STATUS),
    BVDC_P_MAKE_BVN_ERR(eMvp_2,    MVP_TOP_2,    BVNM_INTR2_1_R5F, MVP_TOP_0,     MVP_TOP_2,     MVP_TOP_0_STATUS),
    BVDC_P_MAKE_BVN_ERR(eMvp_3,    MVP_TOP_3,    BVNM_INTR2_1_R5F, MVP_TOP_0,     MVP_TOP_3,     MVP_TOP_0_STATUS),
    BVDC_P_MAKE_INVALID(eMvp_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eMvp_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eMcdi_0,   MDI_0_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eMcdi_1,   MDI_1_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_1,     MDI_TOP_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eMcdi_2,   MDI_2_BVB_IN, BVNM_INTR2_1_R5F, MDI_TOP_0,     MDI_TOP_2,     MDI_TOP_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eMcdi_3,   MDI_3_BVB_IN, BVNM_INTR2_1_R5F, MDI_TOP_0,     MDI_TOP_3,     MDI_TOP_0_BVB_IN_STATUS),
    BVDC_P_MAKE_INVALID(eMcdi_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eMctf_0,   MCTF_0,       BVNM_INTR2_0_R5F, HD_ANR_MCTF_0, HD_ANR_MCTF_0, HD_ANR_MCTF_0_BVB_IN_STATUS),

    BVDC_P_MAKE_BVN_ERR(eHscl_0,   HSCL_0,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eHscl_1,   HSCL_1,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_1,        HSCL_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eHscl_2,   HSCL_2,       BVNM_INTR2_1_R5F, HSCL_0,        HSCL_2,        HSCL_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eHscl_3,   HSCL_3,       BVNM_INTR2_1_R5F, HSCL_0,        HSCL_3,        HSCL_0_BVB_IN_STATUS),
    BVDC_P_MAKE_INVALID(eHscl_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eHscl_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCap_2,    CAP2,         BVNB_INTR2_CPU,   CAP_0,         CAP_2,         CAP_0_BVB_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCap_3,    CAP3,         BVNB_INTR2_CPU,   CAP_0,         CAP_3,         CAP_0_BVB_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCap_4,    CAP4,         BVNB_INTR2_CPU,   CAP_0,         CAP_4,         CAP_0_BVB_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCap_5,    CAP5,         BVNB_INTR2_CPU,   CAP_0,         CAP_5,         CAP_0_BVB_STATUS),
    BVDC_P_MAKE_INVALID(eCap_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eCap_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
    BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
    BVDC_P_MAKE_BVN_ERR(eGfd_2,    GFD2,         BVNB_INTR2_CPU,   GFD_0,         GFD_2,         GFD_0_STATUS),
    BVDC_P_MAKE_BVN_ERR(eGfd_3,    GFD3,         BVNB_INTR2_CPU,   GFD_0,         GFD_3,         GFD_0_STATUS),
    BVDC_P_MAKE_INVALID(eGfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eGfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCmp_2_V0, CMP2_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_V0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCmp_3_V0, CMP3_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_3,         CMP_0_V0_BVB_IN_STATUS),
    BVDC_P_MAKE_INVALID(eCmp_4_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eCmp_5_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eCmp_0_V1, CMP0_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V1_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCmp_1_V1, CMP1_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V1_BVB_IN_STATUS),

    BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCmp_2_G0, CMP2_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_G0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCmp_3_G0, CMP3_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_3,         CMP_0_G0_BVB_IN_STATUS),
    BVDC_P_MAKE_INVALID(eCmp_4_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eCmp_5_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif  ((BCHP_CHIP==7366) && (BCHP_VER>=BCHP_VER_B0))
    BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

    BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
    BVDC_P_MAKE_BVN_ERR(eMfd_1,    MFD_1,        BVNF_INTR2_5_R5F, MFD_0,         MFD_1,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
    BVDC_P_MAKE_BVN_ERR(eMfd_2,    MFD_2,        BVNF_INTR2_5_R5F, MFD_0,         MFD_2,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
    BVDC_P_MAKE_INVALID(eMfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
    BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
    BVDC_P_MAKE_BVN_ERR(eVfd_2,    VFD_2,        BVNF_INTR2_5_R5F, VFD_0,         VFD_2,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
    BVDC_P_MAKE_BVN_ERR(eVfd_3,    VFD_3,        BVNF_INTR2_5_R5F, VFD_0,         VFD_3,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
    BVDC_P_MAKE_INVALID(eVfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eVfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eVfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eVfd_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eScl_2,    SCL_2_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_2,         SCL_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eScl_3,    SCL_3_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_3,         SCL_0_BVB_IN_STATUS),
    BVDC_P_MAKE_INVALID(eScl_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eScl_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eScl_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eScl_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eDnr_1,    DNR_1_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_1,         DNR_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eDnr_2,    DNR_2_ERR,    BVNM_INTR2_1_R5F, DNR_0,         DNR_2,         DNR_0_BVB_IN_STATUS),
    BVDC_P_MAKE_INVALID(eDnr_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eXsrc_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eXsrc_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_INVALID(eMad_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eMvp_0,    MVP_TOP_0,    BVNM_INTR2_0_R5F, MVP_TOP_0,     MVP_TOP_0,     MVP_TOP_0_STATUS),
    BVDC_P_MAKE_BVN_ERR(eMvp_1,    MVP_TOP_1,    BVNM_INTR2_0_R5F, MVP_TOP_0,     MVP_TOP_1,     MVP_TOP_0_STATUS),
    BVDC_P_MAKE_BVN_ERR(eMvp_2,    MVP_TOP_2,    BVNM_INTR2_1_R5F, MVP_TOP_0,     MVP_TOP_2,     MVP_TOP_0_STATUS),
    BVDC_P_MAKE_INVALID(eMvp_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eMvp_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eMvp_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eMcdi_0,   MDI_0_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eMcdi_1,   MDI_1_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_1,     MDI_TOP_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eMcdi_2,   MDI_2_BVB_IN, BVNM_INTR2_1_R5F, MDI_TOP_0,     MDI_TOP_2,     MDI_TOP_0_BVB_IN_STATUS),
    BVDC_P_MAKE_INVALID(eMcdi_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eMcdi_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eMctf_0,   MCTF_0,       BVNM_INTR2_0_R5F, HD_ANR_MCTF_0, HD_ANR_MCTF_0, HD_ANR_MCTF_0_BVB_IN_STATUS),

    BVDC_P_MAKE_BVN_ERR(eHscl_0,   HSCL_0,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eHscl_1,   HSCL_1,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_1,        HSCL_0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eHscl_2,   HSCL_2,       BVNM_INTR2_1_R5F, HSCL_0,        HSCL_2,        HSCL_0_BVB_IN_STATUS),
    BVDC_P_MAKE_INVALID(eHscl_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eHscl_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eHscl_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCap_2,    CAP2,         BVNB_INTR2_CPU,   CAP_0,         CAP_2,         CAP_0_BVB_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCap_3,    CAP3,         BVNB_INTR2_CPU,   CAP_0,         CAP_3,         CAP_0_BVB_STATUS),
    BVDC_P_MAKE_INVALID(eCap_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eCap_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eCap_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eCap_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
    BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
    BVDC_P_MAKE_BVN_ERR(eGfd_2,    GFD2,         BVNB_INTR2_CPU,   GFD_0,         GFD_2,         GFD_0_STATUS),
    BVDC_P_MAKE_INVALID(eGfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eGfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eGfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCmp_2_V0, CMP2_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_V0_BVB_IN_STATUS),
    BVDC_P_MAKE_INVALID(eCmp_3_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eCmp_4_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eCmp_5_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_BVN_ERR(eCmp_0_V1, CMP0_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V1_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCmp_1_V1, CMP1_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V1_BVB_IN_STATUS),

    BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
    BVDC_P_MAKE_BVN_ERR(eCmp_2_G0, CMP2_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_G0_BVB_IN_STATUS),
    BVDC_P_MAKE_INVALID(eCmp_3_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eCmp_4_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eCmp_5_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

    BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
    BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif ((BCHP_CHIP==7366) && (BCHP_VER==BCHP_VER_A0)) || \
	  ((BCHP_CHIP==7439) && (BCHP_VER==BCHP_VER_A0)) || \
	  ((BCHP_CHIP==74371) && (BCHP_VER==BCHP_VER_A0))
	BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

	BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_1,    MFD_1,        BVNF_INTR2_5_R5F, MFD_0,         MFD_1,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eMfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_2,    VFD_2,        BVNF_INTR2_5_R5F, VFD_0,         VFD_2,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_3,    VFD_3,        BVNF_INTR2_5_R5F, VFD_0,         VFD_3,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eVfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_2,    SCL_2_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_2,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_3,    SCL_3_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_3,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eScl_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eDnr_1,    DNR_1_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_1,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eDnr_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMad_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMvp_0,    MVP_TOP_0,    BVNM_INTR2_0_R5F, MVP_TOP_0,     MVP_TOP_0,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMvp_1,    MVP_TOP_1,    BVNM_INTR2_0_R5F, MVP_TOP_0,     MVP_TOP_1,     MVP_TOP_0_STATUS),
	BVDC_P_MAKE_INVALID(eMvp_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMcdi_0,   MDI_0_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_1,   MDI_1_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_1,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eMcdi_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMctf_0,   MCTF_0,       BVNM_INTR2_0_R5F, HD_ANR_MCTF_0, HD_ANR_MCTF_0, HD_ANR_MCTF_0_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eHscl_0,   HSCL_0,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eHscl_1,   HSCL_1,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_1,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eHscl_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_2,    CAP2,         BVNB_INTR2_CPU,   CAP_0,         CAP_2,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_3,    CAP3,         BVNB_INTR2_CPU,   CAP_0,         CAP_3,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_INVALID(eCap_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_2,    GFD2,         BVNB_INTR2_CPU,   GFD_0,         GFD_2,         GFD_0_STATUS),
	BVDC_P_MAKE_INVALID(eGfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_2_V0, CMP2_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_3_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V1, CMP0_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V1_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V1, CMP1_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V1_BVB_IN_STATUS),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_2_G0, CMP2_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_2,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_3_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif (BCHP_CHIP==7586)
	BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

	BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eMfd_1,    MFD_1,        BVNF_INTR2_5_R5F, MFD_0,         MFD_1,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eMfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_2,    VFD_2,        BVNF_INTR2_5_R5F, VFD_0,         VFD_2,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_3,    VFD_3,        BVNF_INTR2_5_R5F, VFD_0,         VFD_3,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eVfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_2,    SCL_2_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_2,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_3,    SCL_3_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_3,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eScl_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eDnr_1,    DNR_1_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_1,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eDnr_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eXsrc_0,   XSRC_0_ERR,   BVNM_INTR2_0_R5F, XSRC_0_ERR,    XSRC_0_ERR,    XSRC_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eXsrc_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMad_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMvp_0,    MVP_TOP_0,   BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eMvp_1,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMcdi_0,   MDI_0_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eMcdi_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMctf_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eHscl_0,   HSCL_0,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eHscl_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_2,    CAP2,         BVNB_INTR2_CPU,   CAP_0,         CAP_2,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_3,    CAP3,         BVNB_INTR2_CPU,   CAP_0,         CAP_3,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_INVALID(eCap_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
	BVDC_P_MAKE_INVALID(eGfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_V1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_1_V1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#elif (BCHP_CHIP==7271) || (BCHP_CHIP==7268)
	BVDC_P_MAKE_BVN_ERR(eRdc,      RDC_ERR,      BVNF_INTR2_5_R5F, RDC,           RDC,           RDC_error_status),

	BVDC_P_MAKE_BVN_ERR(eMfd_0,    MFD_0,        BVNF_INTR2_5_R5F, MFD_0,         MFD_0,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMfd_1,    MFD_1,        BVNF_INTR2_5_R5F, MFD_0,         MFD_1,         MFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eMfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eVfd_0,    VFD_0,        BVNF_INTR2_5_R5F, VFD_0,         VFD_0,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_BVN_ERR(eVfd_1,    VFD_1,        BVNF_INTR2_5_R5F, VFD_0,         VFD_1,         VFD_0_FEEDER_ERROR_INTERRUPT_STATUS),
	BVDC_P_MAKE_INVALID(eVfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eVfd_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eScl_0,    SCL_0_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_0,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eScl_1,    SCL_1_ERR,    BVNM_INTR2_0_R5F, SCL_0,         SCL_1,         SCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eScl_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eScl_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eDnr_0,    DNR_0_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_0,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eDnr_1,    DNR_1_ERR,    BVNM_INTR2_0_R5F, DNR_0,         DNR_1,         DNR_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eDnr_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eDnr_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_BVN_ERR(eXsrc_0,   XSRC_0_ERR,   BVNM_INTR2_0_R5F, XSRC_0,        XSRC_0,        XSRC_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eXsrc_1,   XSRC_1_ERR,   BVNM_INTR2_0_R5F, XSRC_0,        XSRC_1,        XSRC_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eTntd_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMad_0,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMvp_0,    MVP_TOP_0,    BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMvp_1,    MVP_TOP_1,    BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_1,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eMvp_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMvp_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eMcdi_0,   MDI_0_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_0,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eMcdi_1,   MDI_1_BVB_IN, BVNM_INTR2_0_R5F, MDI_TOP_0,     MDI_TOP_1,     MDI_TOP_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eMcdi_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eMcdi_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eMctf_0,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eHscl_0,   HSCL_0,       BVNM_INTR2_0_R5F, HSCL_0,        HSCL_0,        HSCL_0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eHscl_1,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_2,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_3,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_4,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eHscl_5,   UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCap_0,    CAP0,         BVNB_INTR2_CPU,   CAP_0,         CAP_0,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCap_1,    CAP1,         BVNB_INTR2_CPU,   CAP_0,         CAP_1,         CAP_0_BVB_STATUS),
	BVDC_P_MAKE_INVALID(eCap_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCap_7,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eGfd_0,    GFD0,         BVNB_INTR2_CPU,   GFD_0,         GFD_0,         GFD_0_STATUS),
	BVDC_P_MAKE_BVN_ERR(eGfd_1,    GFD1,         BVNB_INTR2_CPU,   GFD_0,         GFD_1,         GFD_0_STATUS),
	BVDC_P_MAKE_INVALID(eGfd_2,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_3,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_4,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_5,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eGfd_6,    UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V0, CMP0_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_V0, CMP1_V0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_V0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_V0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_V1, CMP0_V1,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_V1_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_1_V1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_BVN_ERR(eCmp_0_G0, CMP0_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_0,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_BVN_ERR(eCmp_1_G0, CMP1_G0,      BVNB_INTR2_CPU,   CMP_0,         CMP_1,         CMP_0_G0_BVB_IN_STATUS),
	BVDC_P_MAKE_INVALID(eCmp_2_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_3_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_4_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_5_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_6_G0, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

	BVDC_P_MAKE_INVALID(eCmp_0_G1, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),
	BVDC_P_MAKE_INVALID(eCmp_0_G2, UNKNOWN,      UNKNOWN,          UNKNOWN,       UNKNOWN,       UNKNOWN),

#else
#error "Port required for BVN ERR."
#endif
};

/***************************************************************************
 *
 * API support functions
 *
 ***************************************************************************/
const BVDC_P_IntCbTbl * BVDC_P_GetBvnErrorCb_isr
	( BVDC_BvnError            eBvnErrId )
{
	BDBG_CASSERT(BVDC_P_BVN_ERR_COUNT == BVDC_BvnError_eMaxCount);
	if(BVDC_BvnError_eMaxCount <= eBvnErrId ||
	   s_apfErrorHandlers[eBvnErrId].ErrIntId == BVDC_P_NULL_BINTID)
		return NULL;
	else
	{
		BDBG_ASSERT(eBvnErrId == s_apfErrorHandlers[eBvnErrId].eBvnError);
		return &(s_apfErrorHandlers[eBvnErrId]);
	}
}

const BVDC_P_IntCbTbl * BVDC_P_GetBvnErrorCb
    ( BVDC_BvnError            eBvnErrId )
{
    const BVDC_P_IntCbTbl *pIntCb;

    BKNI_EnterCriticalSection();
    pIntCb = BVDC_P_GetBvnErrorCb_isr(eBvnErrId);
    BKNI_LeaveCriticalSection();

    return pIntCb;
}

/* End of File */

