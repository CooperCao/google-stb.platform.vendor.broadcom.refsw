/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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
 *     Define features that are hardware-specific. For private use by BVBI
 *     software module.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#ifndef BVBI_CAP_PRIV_H__
#define BVBI_CAP_PRIV_H__

#if (BCHP_CHIP==7325)
	#define BVBI_NUM_VEC 2
	#define BVBI_NUM_PTVEC 1
	#define BVBI_NUM_AMOLE 1
	#define BVBI_NUM_AMOLE_656 1
	#define BVBI_NUM_CCE 2
	#define BVBI_NUM_CCE_656 1
	#define BVBI_NUM_CGMSAE 2
	#define BVBI_NUM_CGMSAE_656 1
	#define BVBI_NUM_GSE 2
	#define BVBI_NUM_GSE_656 0
	#define BVBI_NUM_SCTEE 1
	#define BVBI_NUM_SCTEE_656 0
	#define BVBI_NUM_TTE 2
	#define BVBI_NUM_TTE_656 1
	#define BVBI_NUM_WSE 2
	#define BVBI_NUM_WSE_656 1
	#define BVBI_NUM_IN656 0
	#define BVBI_NUM_ANCI656_656 1
#elif (BCHP_CHIP==7335) ||  (BCHP_CHIP==7336)
	#define BVBI_NUM_VEC 2
	#define BVBI_NUM_PTVEC 1
	#define BVBI_NUM_AMOLE 1
	#define BVBI_NUM_AMOLE_656 1
	#define BVBI_NUM_CCE 2
	#define BVBI_NUM_CCE_656 1
	#define BVBI_NUM_CGMSAE 2
	#define BVBI_NUM_CGMSAE_656 1
	#define BVBI_NUM_GSE 2
	#define BVBI_NUM_GSE_656 0
	#define BVBI_NUM_SCTEE 1
	#define BVBI_NUM_SCTEE_656 0
	#define BVBI_NUM_TTE 2
	#define BVBI_NUM_TTE_656 1
	#define BVBI_NUM_WSE 2
	#define BVBI_NUM_WSE_656 1
	#define BVBI_NUM_IN656 0
	#define BVBI_NUM_ANCI656_656 1
#elif (BCHP_CHIP==7400)
	/* NOTE: chip revision -A0 is not supported */
	#define BVBI_NUM_VEC 3
	#define BVBI_NUM_AMOLE 3
	#define BVBI_NUM_CCE 3
	#define BVBI_NUM_CGMSAE 3
	#define BVBI_NUM_GSE 3
	#define BVBI_NUM_AMOLE_656 1
	#define BVBI_NUM_CCE_656 1
	#define BVBI_NUM_CGMSAE_656 1
	#define BVBI_NUM_GSE_656 0
	#define BVBI_NUM_ANCI656_656 1
	#define BVBI_NUM_SCTEE 3
	#define BVBI_NUM_SCTEE_656 0
	#define BVBI_NUM_TTE 3
	#define BVBI_NUM_TTE_656 1
	#define BVBI_NUM_WSE 3
	#define BVBI_NUM_WSE_656 1
	#define BVBI_NUM_IN656 2
	#define BVBI_NUM_PTVEC 1
#elif (BCHP_CHIP==7405)
	#define BVBI_NUM_VEC 2
	#define BVBI_NUM_PTVEC 1
	#define BVBI_NUM_AMOLE 1
	#define BVBI_NUM_AMOLE_656 1
	#define BVBI_NUM_CCE 2
	#define BVBI_NUM_CCE_656 1
	#define BVBI_NUM_CGMSAE 2
	#define BVBI_NUM_CGMSAE_656 1
	#define BVBI_NUM_GSE 2
	#define BVBI_NUM_GSE_656 1
	#define BVBI_NUM_SCTEE 1
	#define BVBI_NUM_SCTEE_656 0
	#define BVBI_NUM_TTE 2
	#define BVBI_NUM_TTE_656 1
	#define BVBI_NUM_WSE 2
	#define BVBI_NUM_WSE_656 1
	#define BVBI_NUM_IN656 1
	#define BVBI_NUM_ANCI656_656 1
#elif (BCHP_CHIP==7601)
	#define BVBI_NUM_IN656 0
	#define BVBI_NUM_ANCI656_656 0
	#define BVBI_NUM_PTVEC 0
	#define BVBI_NUM_VEC 2
	#define BVBI_NUM_PTVEC 0
	#if (BCHP_VER < BCHP_VER_B0)
		#define BVBI_NUM_AMOLE 0
		#define BVBI_NUM_AMOLE_656 0
		#define BVBI_NUM_CCE 1
		#define BVBI_NUM_CCE_656 0
		#define BVBI_NUM_CGMSAE 1
		#define BVBI_NUM_CGMSAE_656 0
		#define BVBI_NUM_GSE 1
		#define BVBI_NUM_GSE_656 0
		#define BVBI_NUM_TTE 1
		#define BVBI_NUM_TTE_656 0
		#define BVBI_NUM_WSE 1
		#define BVBI_NUM_WSE_656 0
	#else
		#define BVBI_NUM_AMOLE 0
		#define BVBI_NUM_AMOLE_656 0
		#define BVBI_NUM_CCE 2
		#define BVBI_NUM_CCE_656 0
		#define BVBI_NUM_CGMSAE 2
		#define BVBI_NUM_CGMSAE_656 0
		#define BVBI_NUM_GSE 2
		#define BVBI_NUM_GSE_656 0
		#define BVBI_NUM_TTE 2
		#define BVBI_NUM_TTE_656 0
		#define BVBI_NUM_WSE 2
		#define BVBI_NUM_WSE_656 0
	#endif
	#define BVBI_NUM_SCTEE 0
	#define BVBI_NUM_SCTEE_656 0
#else
	#error Unknown video chip name
#endif

#endif /* BVBI_CAP_PRIV_H__ */
