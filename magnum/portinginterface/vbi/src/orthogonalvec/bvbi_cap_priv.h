/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 *   See Module Overview below
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#ifndef BVBI_CAP_PRIV_H__
#define BVBI_CAP_PRIV_H__

#include "bchp_common.h"

#if defined(BCHP_IT_2_REG_START)
#define BVBI_NUM_VEC 3
#elif defined(BCHP_IT_1_REG_START)
#define BVBI_NUM_VEC 2
#elif defined(BCHP_IT_0_REG_START)
#define BVBI_NUM_VEC 1
#else
#define BVBI_NUM_VEC 0
#endif

#if defined(BCHP_ITU656_2_REG_START)
#define BVBI_NUM_PTVEC 3
#elif defined(BCHP_ITU656_1_REG_START)
#define BVBI_NUM_PTVEC 2
#elif defined(BCHP_ITU656_0_REG_START)
#define BVBI_NUM_PTVEC 1
#else
#define BVBI_NUM_PTVEC 0
#endif

#if defined(BCHP_AMOLE_2_REG_START)
#define BVBI_NUM_AMOLE 3
#elif defined(BCHP_AMOLE_1_REG_START)
#define BVBI_NUM_AMOLE 2
#elif defined(BCHP_AMOLE_0_REG_START)
#define BVBI_NUM_AMOLE 1
#else
#define BVBI_NUM_AMOLE 0
#endif

#if defined(BCHP_AMOLE_ANCIL_2_REG_START)
#define BVBI_NUM_AMOLE_656 3
#elif defined(BCHP_AMOLE_ANCIL_1_REG_START)
#define BVBI_NUM_AMOLE_656 2
#elif defined(BCHP_AMOLE_ANCIL_0_REG_START)
#define BVBI_NUM_AMOLE_656 1
#else
#define BVBI_NUM_AMOLE_656 0
#endif

#if defined(BCHP_CCE_2_REG_START)
#define BVBI_NUM_CCE 3
#elif defined(BCHP_CCE_1_REG_START)
#define BVBI_NUM_CCE 2
#elif defined(BCHP_CCE_0_REG_START)
#define BVBI_NUM_CCE 1
#else
#define BVBI_NUM_CCE 0
#endif

#if defined(BCHP_CCE_ANCIL_2_REG_START)
#define BVBI_NUM_CCE_656 3
#elif defined(BCHP_CCE_ANCIL_1_REG_START)
#define BVBI_NUM_CCE_656 2
#elif defined(BCHP_CCE_ANCIL_0_REG_START)
#define BVBI_NUM_CCE_656 1
#else
#define BVBI_NUM_CCE_656 0
#endif

#if defined(BCHP_CGMSAE_2_REG_START)
#define BVBI_NUM_CGMSAE 3
#elif defined(BCHP_CGMSAE_1_REG_START)
#define BVBI_NUM_CGMSAE 2
#elif defined(BCHP_CGMSAE_0_REG_START)
#define BVBI_NUM_CGMSAE 1
#else
#define BVBI_NUM_CGMSAE 0
#endif

#if defined(BCHP_CGMSAE_ANCIL_2_REG_START)
#define BVBI_NUM_CGMSAE_656 3
#elif defined(BCHP_CGMSAE_ANCIL_1_REG_START)
#define BVBI_NUM_CGMSAE_656 2
#elif defined(BCHP_CGMSAE_ANCIL_0_REG_START)
#define BVBI_NUM_CGMSAE_656 1
#else
#define BVBI_NUM_CGMSAE_656 0
#endif

#if defined(BCHP_GSE_2_REG_START)
#define BVBI_NUM_GSE 3
#elif defined(BCHP_GSE_1_REG_START)
#define BVBI_NUM_GSE 2
#elif defined(BCHP_GSE_0_REG_START)
#define BVBI_NUM_GSE 1
#else
#define BVBI_NUM_GSE 0
#endif

#if defined(BCHP_GSE_ANCIL_2_REG_START)
#define BVBI_NUM_GSE_656 3
#elif defined(BCHP_GSE_ANCIL_1_REG_START)
#define BVBI_NUM_GSE_656 2
#elif defined(BCHP_GSE_ANCIL_0_REG_START)
#define BVBI_NUM_GSE_656 1
#else
#define BVBI_NUM_GSE_656 0
#endif

#if defined(BCHP_SCTE_2_REG_START)
#define BVBI_NUM_SCTEE 3
#elif defined(BCHP_SCTE_1_REG_START)
#define BVBI_NUM_SCTEE 2
#elif defined(BCHP_SCTE_0_REG_START)
#define BVBI_NUM_SCTEE 1
#else
#define BVBI_NUM_SCTEE 0
#endif

#if defined(BCHP_SCTE_ANCIL_2_REG_START)
#define BVBI_NUM_SCTEE_656 3
#elif defined(BCHP_SCTEE_ANCIL_1_REG_START)
#define BVBI_NUM_SCTEE_656 2
#elif defined(BCHP_SCTEE_ANCIL_0_REG_START)
#define BVBI_NUM_SCTEE_656 1
#else
#define BVBI_NUM_SCTEE_656 0
#endif

#if defined(BCHP_TTE_2_REG_START)
#define BVBI_NUM_TTE 3
#elif defined(BCHP_TTE_1_REG_START)
#define BVBI_NUM_TTE 2
#elif defined(BCHP_TTE_0_REG_START)
#define BVBI_NUM_TTE 1
#else
#define BVBI_NUM_TTE 0
#endif

#if defined(BCHP_TTE_ANCIL_2_REG_START)
#define BVBI_NUM_TTE_656 3
#elif defined(BCHP_TTE_ANCIL_1_REG_START)
#define BVBI_NUM_TTE_656 2
#elif defined(BCHP_TTE_ANCIL_0_REG_START)
#define BVBI_NUM_TTE_656 1
#else
#define BVBI_NUM_TTE_656 0
#endif

#if defined(BCHP_WSE_2_REG_START)
#define BVBI_NUM_WSE 3
#elif defined(BCHP_WSE_1_REG_START)
#define BVBI_NUM_WSE 2
#elif defined(BCHP_WSE_0_REG_START)
#define BVBI_NUM_WSE 1
#else
#define BVBI_NUM_WSE 0
#endif

#if defined(BCHP_WSE_ANCIL_2_REG_START)
#define BVBI_NUM_WSE_656 3
#elif defined(BCHP_WSE_ANCIL_1_REG_START)
#define BVBI_NUM_WSE_656 2
#elif defined(BCHP_WSE_ANCIL_0_REG_START)
#define BVBI_NUM_WSE_656 1
#else
#define BVBI_NUM_WSE_656 0
#endif

#if defined(BCHP_IN656_2_REG_START)
#define BVBI_NUM_IN656 3
#elif defined(BCHP_IN656_1_REG_START)
#define BVBI_NUM_IN656 2
#elif defined(BCHP_IN656_0_REG_START)
#define BVBI_NUM_IN656 1
#else
#define BVBI_NUM_IN656 0
#endif

#if defined(BCHP_ANCI656_ANCIL_2_REG_START)
#define BVBI_NUM_ANCI656_656 3
#elif defined(BCHP_ANCI656_ANCIL_1_REG_START)
#define BVBI_NUM_ANCI656_656 2
#elif defined(BCHP_ANCI656_ANCIL_0_REG_START)
#define BVBI_NUM_ANCI656_656 1
#else
#define BVBI_NUM_ANCI656_656 0
#endif

#endif /* BVBI_CAP_PRIV_H__ */
