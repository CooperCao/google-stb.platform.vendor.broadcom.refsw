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

#ifndef BVBI_CAP_H__
#define BVBI_CAP_H__

/*
 * Explanation of VEC counts and capabilities:
 * BVBI_NUM_VEC:          Number of full VBI encoders. Does not include
 *                        ITU-R 656 passthrough output.
 * BVBI_NUM_PTVEC:        Number of passthrough or ancillary VBI encoders.
 * BVBI_NUM_AMOLE:        Number of AMOL encoder cores, not including
 *                        ITU-R 656.
 * BVBI_NUM_AMOLE_656:    Number of AMOL encoder cores that are specifically
 *                        ITU-R 656.
 * BVBI_NUM_CCE:          Number of closed caption encoder cores, not
 *                        including ITU-R 656.
 * BVBI_NUM_CCE_656:      Number of closed caption encoder cores that are
 *                        specifically for ITU-R 656 output.
 * BVBI_NUM_CGMSAE:       Number of CGMS encoder cores, not including ITU-R
 *                        656.
 * BVBI_NUM_CGMSAE_656:   Number of CGMS encoder cores that are specifically
 *                        for ITU-R 656 output.
 * BVBI_NUM_GSE:          Number of Gemstar encoder cores, not including
 *                        ITU-R 656.
 * BVBI_NUM_GSE_656:      Number of Gemstar encoder cores that are
 *                        specifically for ITU-R 656 output.
 * BVBI_NUM_SCTEE:        Number of SCTE encoder cores. Assume first core
 *                        that has one is on the primary VEC path.
 * BVBI_NUM_SCTEE_656:    Number of 656 SCTE encoder cores. At present time,
 *                        always zero.
 * BVBI_NUM_TTE:          Number of teletext encoder cores. Assume first core
 *                        that has one is on the primary VEC path. Does not
 *                        include 656 (bypass) cores.
 * BVBI_NUM_TTE_656:      Number of 656 (bypass) teletext encoder cores.
 *                        Assume first core that has one is on the primary
 *                        VEC path.
 * BVBI_NUM_WSE:          Number of WSE cores, not including ITU-R 656.
 * BVBI_NUM_WSE_656:      Number of WSE cores that are specifically for
 *                        ITU-R 656 output.
 * BVBI_NUM_IN656:        Number of ITU-R 656 inputs (IN656 cores)
 * BVBI_NUM_ANCI656_656:  Number of ANCI656_656 or ANCI656_Ancil VEC cores.
 */

#include "bvbi_cap_priv.h"

#endif /* BVBI_CAP_H__ */
