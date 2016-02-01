/***************************************************************************
*     (c)2015 Broadcom Corporation
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
***************************************************************************/
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "nexus_sage_svp_bvn.h"

BDBG_MODULE(BBVN);
BDBG_FILE_MODULE(BBVN_L1);
BDBG_FILE_MODULE(BBVN_L2);
BDBG_FILE_MODULE(BBVN_L3);
BDBG_OBJECT_ID(BBVN_BVN);

/***************************************************************************/
/* TODO: Move into nexus_sage_bvn_vnet.c/h. */
/***************************************************************************/
#include "bchp_vnet_f.h"
#include "bchp_vnet_b.h"
#include "bchp_common.h"
#include "bchp_cmp_0.h"
#ifdef BCHP_VEC_CFG_REG_START
#include "bchp_vec_cfg.h"
#endif
#ifdef BCHP_MVP_TOP_0_REG_START
#include "bchp_mvp_top_0.h"
#endif

#define BCHP_VNET_F_TERMINAL_SRC        (0xffffffff)
#define BCHP_VNET_B_TERMINAL_SRC        (0xffffffff)
#define BBVN_P_INVALID_COMPOSITOR_IDX   (0xff)
#define BBVN_P_COMPOSITOR_MAX           (7)
#define BBVN_P_WINDOW_MAX               (2) /* per compositor */

#define BBVN_P_VNET_NAME_LEN            (20)
#define BBVN_P_BVNMAP_NAME_LEN          (512)

/* Make things look cleaner */
#define BBVN_P_VNET_F(vnet_f_enum) \
	BCHP_VNET_F_SCL_0_SRC_SOURCE_##vnet_f_enum
#define BBVN_P_VNET_B(vnet_b_enum) \
	BCHP_VNET_B_CAP_0_SRC_SOURCE_##vnet_b_enum

/***************************************************************************
 * Enumeration that defines the front muxes.
 */
typedef enum
{
	BBVN_P_VnetF_eDisabled  = BBVN_P_VNET_F(Output_Disabled),
	BBVN_P_VnetF_eMpeg_0    = BBVN_P_VNET_F(MPEG_Feeder_0),
#if (BCHP_MFD_1_REG_START)
	BBVN_P_VnetF_eMpeg_1    = BBVN_P_VNET_F(MPEG_Feeder_1),
#endif
#if (BCHP_MFD_2_REG_START)
	BBVN_P_VnetF_eMpeg_2    = BBVN_P_VNET_F(MPEG_Feeder_2),
#endif
#if (BCHP_MFD_3_REG_START)
	BBVN_P_VnetF_eMpeg_3    = BBVN_P_VNET_F(MPEG_Feeder_3),
#endif
#if (BCHP_MFD_4_REG_START)
	BBVN_P_VnetF_eMpeg_4    = BBVN_P_VNET_F(MPEG_Feeder_4),
#endif
#if (BCHP_MFD_5_REG_START)
	BBVN_P_VnetF_eMpeg_5    = BBVN_P_VNET_F(MPEG_Feeder_5),
#endif

	BBVN_P_VnetF_eVideo_0   = BBVN_P_VNET_F(Video_Feeder_0),
	BBVN_P_VnetF_eVideo_1   = BBVN_P_VNET_F(Video_Feeder_1),
#if (BCHP_VFD_2_REG_START)
	BBVN_P_VnetF_eVideo_2   = BBVN_P_VNET_F(Video_Feeder_2),
#endif
#if (BCHP_VFD_3_REG_START)
	BBVN_P_VnetF_eVideo_3   = BBVN_P_VNET_F(Video_Feeder_3),
#endif
#if (BCHP_VFD_4_REG_START)
	BBVN_P_VnetF_eVideo_4   = BBVN_P_VNET_F(Video_Feeder_4),
#endif
#if (BCHP_VFD_5_REG_START)
	BBVN_P_VnetF_eVideo_5   = BBVN_P_VNET_F(Video_Feeder_5),
#endif
#if (BCHP_VFD_6_REG_START)
	BBVN_P_VnetF_eVideo_6   = BBVN_P_VNET_F(Video_Feeder_6),
#endif
#if (BCHP_VFD_7_REG_START)
	BBVN_P_VnetF_eVideo_7   = BBVN_P_VNET_F(Video_Feeder_7),
#endif

#if (BCHP_HD_DVI_0_REG_START)
	BBVN_P_VnetF_eHdDvi_0   = BBVN_P_VNET_F(HD_DVI_0),
#endif
#if (BCHP_HD_DVI_1_REG_START)
	BBVN_P_VnetF_eHdDvi_1   = BBVN_P_VNET_F(HD_DVI_1),
#endif

#if (BCHP_IN656_0_REG_START)
	BBVN_P_VnetF_eCcir656_0 = BBVN_P_VNET_F(CCIR_656_0),
#endif
#if (BCHP_IN656_1_REG_START)
	BBVN_P_VnetF_eCcir656_1 = BBVN_P_VNET_F(CCIR_656_1),
#endif
	BBVN_P_VnetF_eLoopback_0  = BBVN_P_VNET_F(Loopback_0),
	BBVN_P_VnetF_eLoopback_1  = BBVN_P_VNET_F(Loopback_1),
#if (BCHP_VNET_B_LOOPBACK_2_SRC)
	BBVN_P_VnetF_eLoopback_2  = BBVN_P_VNET_F(Loopback_2),
#endif
#if (BCHP_VNET_B_LOOPBACK_3_SRC)
	BBVN_P_VnetF_eLoopback_3  = BBVN_P_VNET_F(Loopback_3),
#endif
#if (BCHP_VNET_B_LOOPBACK_4_SRC)
	BBVN_P_VnetF_eLoopback_4  = BBVN_P_VNET_F(Loopback_4),
#endif
#if (BCHP_VNET_B_LOOPBACK_5_SRC)
	BBVN_P_VnetF_eLoopback_5  = BBVN_P_VNET_F(Loopback_5),
#endif
#if (BCHP_VNET_B_LOOPBACK_6_SRC)
	BBVN_P_VnetF_eLoopback_6  = BBVN_P_VNET_F(Loopback_6),
#endif
#if (BCHP_VNET_B_LOOPBACK_7_SRC)
	BBVN_P_VnetF_eLoopback_7  = BBVN_P_VNET_F(Loopback_7),
#endif
#if (BCHP_VNET_B_LOOPBACK_8_SRC)
	BBVN_P_VnetF_eLoopback_8  = BBVN_P_VNET_F(Loopback_8),
#endif
#if (BCHP_VNET_B_LOOPBACK_9_SRC)
	BBVN_P_VnetF_eLoopback_9  = BBVN_P_VNET_F(Loopback_9),
#endif
#if (BCHP_VNET_B_LOOPBACK_10_SRC)
	BBVN_P_VnetF_eLoopback_10  = BBVN_P_VNET_F(Loopback_10),
#endif
#if (BCHP_VNET_B_LOOPBACK_11_SRC)
	BBVN_P_VnetF_eLoopback_11  = BBVN_P_VNET_F(Loopback_11),
#endif
#if (BCHP_VNET_B_LOOPBACK_12_SRC)
	BBVN_P_VnetF_eLoopback_12  = BBVN_P_VNET_F(Loopback_12),
#endif
#if (BCHP_VNET_B_LOOPBACK_13_SRC)
	BBVN_P_VnetF_eLoopback_13  = BBVN_P_VNET_F(Loopback_13),
#endif
#if (BCHP_VNET_B_LOOPBACK_14_SRC)
	BBVN_P_VnetF_eLoopback_14  = BBVN_P_VNET_F(Loopback_14),
#endif
	BBVN_P_VnetF_eTerminal,
	BBVN_P_VnetF_eInvalid
} BBVN_P_VnetF;

/***************************************************************************
 * Enumeration that defines the back muxes.
 */
typedef enum
{
	BBVN_P_VnetB_eDisabled  = BBVN_P_VNET_B(Output_Disabled),
	BBVN_P_VnetB_eScaler_0  = BBVN_P_VNET_B(Scaler_0),
	BBVN_P_VnetB_eScaler_1  = BBVN_P_VNET_B(Scaler_1),
#if BCHP_SCL_2_REG_START
	BBVN_P_VnetB_eScaler_2  = BBVN_P_VNET_B(Scaler_2),
#endif
#if BCHP_SCL_3_REG_START
	BBVN_P_VnetB_eScaler_3  = BBVN_P_VNET_B(Scaler_3),
#endif
#if BCHP_SCL_4_REG_START
	BBVN_P_VnetB_eScaler_4  = BBVN_P_VNET_B(Scaler_4),
#endif
#if BCHP_SCL_5_REG_START
	BBVN_P_VnetB_eScaler_5  = BBVN_P_VNET_B(Scaler_5),
#endif
#if BCHP_SCL_6_REG_START
	BBVN_P_VnetB_eScaler_6  = BBVN_P_VNET_B(Scaler_6),
#endif
#if BCHP_SCL_7_REG_START
	BBVN_P_VnetB_eScaler_7  = BBVN_P_VNET_B(Scaler_7),
#endif

#if (BCHP_VNET_F_MAD32_0_SRC && BCHP_VNET_F_MAD32_1_SRC)
	BBVN_P_VnetB_eMad32_0   = BBVN_P_VNET_B(MCVP_0),
	BBVN_P_VnetB_eMad32_1   = BBVN_P_VNET_B(MCVP_1),
#elif (BCHP_VNET_F_MAD_0_SRC && BCHP_VNET_F_MAD_1_SRC)
	BBVN_P_VnetB_eMad32_0   = BBVN_P_VNET_B(MAD_0),
	BBVN_P_VnetB_eMad32_1   = BBVN_P_VNET_B(MAD_1),
#else
#if (BCHP_VNET_F_MVP_0_SRC)
	BBVN_P_VnetB_eMvp_0     = BBVN_P_VNET_B(MVP_0),
#endif
#if (BCHP_VNET_F_MVP_1_SRC)
	BBVN_P_VnetB_eMvp_1     = BBVN_P_VNET_B(MVP_1),
#endif
#if (BCHP_VNET_F_MVP_2_SRC)
	BBVN_P_VnetB_eMvp_2     = BBVN_P_VNET_B(MVP_2),
#endif
#if (BCHP_VNET_F_MVP_3_SRC)
	BBVN_P_VnetB_eMvp_3     = BBVN_P_VNET_B(MVP_3),
#endif
#if (BCHP_VNET_F_MVP_4_SRC)
	BBVN_P_VnetB_eMvp_4     = BBVN_P_VNET_B(MVP_4),
#endif
#if (BCHP_VNET_F_MVP_5_SRC)
	BBVN_P_VnetB_eMvp_5     = BBVN_P_VNET_B(MVP_5),
#endif
#endif

	BBVN_P_VnetB_eDnr_0     = BBVN_P_VNET_B(DNR_0),
#if (BCHP_VNET_F_DNR_1_SRC)
	BBVN_P_VnetB_eDnr_1     = BBVN_P_VNET_B(DNR_1),
#endif
#if (BCHP_VNET_F_DNR_2_SRC)
	BBVN_P_VnetB_eDnr_2     = BBVN_P_VNET_B(DNR_2),
#endif
#if (BCHP_VNET_F_DNR_3_SRC)
	BBVN_P_VnetB_eDnr_3     = BBVN_P_VNET_B(DNR_3),
#endif
#if (BCHP_VNET_F_DNR_4_SRC)
	BBVN_P_VnetB_eDnr_4     = BBVN_P_VNET_B(DNR_4),
#endif
#if (BCHP_VNET_F_DNR_5_SRC)
	BBVN_P_VnetB_eDnr_5     = BBVN_P_VNET_B(DNR_5),
#endif

#if (BCHP_VNET_F_TNTD_0_SRC)
	BBVN_P_VnetB_eTntD_0    = BBVN_P_VNET_B(TNTD_0),
#endif

#if (BCHP_VNET_F_XSRC_0_SRC)
	BBVN_P_VnetB_eXsrc_0    = BBVN_P_VNET_B(XSRC_0),
#endif
#if (BCHP_VNET_F_XSRC_1_SRC)
	BBVN_P_VnetB_eXsrc_1    = BBVN_P_VNET_B(XSRC_1),
#endif

	BBVN_P_VnetB_eChannel_0 = BBVN_P_VNET_B(Free_Ch_0),
	BBVN_P_VnetB_eChannel_1 = BBVN_P_VNET_B(Free_Ch_1),
	BBVN_P_VnetB_eChannel_2 = BBVN_P_VNET_B(Free_Ch_2),
#if (BCHP_VNET_F_FCH_3_SRC)
	BBVN_P_VnetB_eChannel_3 = BBVN_P_VNET_B(Free_Ch_3),
#endif
#if (BCHP_VNET_F_FCH_4_SRC)
	BBVN_P_VnetB_eChannel_4 = BBVN_P_VNET_B(Free_Ch_4),
#endif
#if (BCHP_VNET_F_FCH_5_SRC)
	BBVN_P_VnetB_eChannel_5 = BBVN_P_VNET_B(Free_Ch_5),
#endif
#if (BCHP_VNET_F_FCH_6_SRC)
	BBVN_P_VnetB_eChannel_6 = BBVN_P_VNET_B(Free_Ch_6),
#endif
#if (BCHP_VNET_F_FCH_7_SRC)
	BBVN_P_VnetB_eChannel_7 = BBVN_P_VNET_B(Free_Ch_7),
#endif
#if (BCHP_VNET_F_FCH_8_SRC)
	BBVN_P_VnetB_eChannel_8 = BBVN_P_VNET_B(Free_Ch_8),
#endif
#if (BCHP_VNET_F_FCH_9_SRC)
	BBVN_P_VnetB_eChannel_9 = BBVN_P_VNET_B(Free_Ch_9),
#endif
#if (BCHP_VNET_F_FCH_10_SRC)
	BBVN_P_VnetB_eChannel_10 = BBVN_P_VNET_B(Free_Ch_10),
#endif
#if (BCHP_VNET_F_FCH_11_SRC)
	BBVN_P_VnetB_eChannel_11 = BBVN_P_VNET_B(Free_Ch_11),
#endif
#if (BCHP_VNET_F_FCH_12_SRC)
	BBVN_P_VnetB_eChannel_12 = BBVN_P_VNET_B(Free_Ch_12),
#endif
#if (BCHP_VNET_F_FCH_13_SRC)
	BBVN_P_VnetB_eChannel_13 = BBVN_P_VNET_B(Free_Ch_13),
#endif
	BBVN_P_VnetB_eTerminal,
	BBVN_P_VnetB_eInvalid
} BBVN_P_VnetB;

/***************************************************************************
 *
 */
typedef enum BBVN_P_VnetMuxType
{
	BBVN_P_VnetMuxType_eBack = 0,
	BBVN_P_VnetMuxType_eFront,
	BBVN_P_VnetMuxType_eTerminal,
	BBVN_P_VnetMuxType_eDisabled
} BBVN_P_VnetMuxType;

/***************************************************************************
 *
 */
typedef struct BBVN_P_VnetMux
{
	char achName[BBVN_P_VNET_NAME_LEN];
	union
	{
		BBVN_P_VnetF eFront;
		BBVN_P_VnetB eBack;
	} nodeType;
	uint32_t ulAddress;
	BBVN_P_VnetMuxType eUpstreamMuxType;
	BBVN_P_VnetMuxType eDownstreamMuxType;
	BAVC_CoreId eCoreId;

} BBVN_P_VnetMux;

/***************************************************************************
 *
 */
typedef struct BBVN_P_PathStatus
{
	bool                     bSecuredCt;
	bool                     bViolation;
	BAVC_CoreList            stBvnCores;
	int                      iStrIndex;
	char                     achBvnMap[BBVN_P_BVNMAP_NAME_LEN];

} BBVN_P_PathStatus;

/***************************************************************************
 *
 */
#define BBVN_P_MAKE_VNET_F_NODE(name, f_enum, b_addr, svp_core_id) \
    {#name, {BBVN_P_VnetF_e##f_enum}, BCHP_VNET_B_##b_addr##_SRC, BBVN_P_VnetMuxType_eBack, BBVN_P_VnetMuxType_eFront, BAVC_CoreId_e##svp_core_id}

#define BBVN_P_MAKE_VNET_F_TERMINAL_NODE(name, f_enum, b_addr, up_mux_type, dn_mux_type, svp_core_id) \
    {#name, {BBVN_P_VnetF_e##f_enum}, BCHP_VNET_B_##b_addr##_SRC, BBVN_P_VnetMuxType_e##up_mux_type, BBVN_P_VnetMuxType_e##dn_mux_type, BAVC_CoreId_e##svp_core_id}

#define BBVN_P_MAKE_VNET_F_DISABLED_NODE(name) \
    {#name, {BBVN_P_VnetF_eDisabled}, BBVN_P_VnetB_eDisabled, BBVN_P_VnetMuxType_eDisabled, BBVN_P_VnetMuxType_eDisabled, BAVC_CoreId_eInvalid}

#define BBVN_P_MAKE_VNET_B_NODE(name, b_enum, f_addr, svp_core_id) \
    {#name, {BBVN_P_VnetB_e##b_enum}, BCHP_VNET_F_##f_addr##_SRC, BBVN_P_VnetMuxType_eFront, BBVN_P_VnetMuxType_eBack, BAVC_CoreId_e##svp_core_id}

#define BBVN_P_MAKE_VNET_B_TERMINAL_NODE(name, f_enum, up_mux_type, dn_mux_type) \
    {#name,{BBVN_P_VnetB_eTerminal}, BCHP_VNET_F_##f_enum##_SRC, BBVN_P_VnetMuxType_e##up_mux_type, BBVN_P_VnetMuxType_e##dn_mux_type, BAVC_CoreId_eInvalid}

#define BBVN_P_MAKE_VNET_B_DISABLED_NODE(name) \
    {#name, {BBVN_P_VnetB_eDisabled}, BBVN_P_VnetF_eDisabled, BBVN_P_VnetMuxType_eDisabled, BBVN_P_VnetMuxType_eDisabled, BAVC_CoreId_eInvalid}

/* Core Id Table Information */
static const struct {
	const char achName[BBVN_P_VNET_NAME_LEN];
} s_aCoreInfoTbl[] = {
#define BCHP_P_MEMC_DEFINE_SVP_HWBLOCK(svp_block, access) { #svp_block },
#include "memc/bchp_memc_svp_hwblock.h"
	{"  -  "},
#undef BCHP_P_MEMC_DEFINE_SVP_HWBLOCK
};

/***************************************************************************
 * VNET_F enum to MUX addr LUT
 */
#define BBVN_P_VNET_F_NODE_COUNT \
			(sizeof(s_astVnetFrontMuxes)/sizeof(BBVN_P_VnetMux))

static const BBVN_P_VnetMux s_astVnetFrontMuxes[] =
{
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(MFD_0, Mpeg_0, TERMINAL, Terminal, Back, MFD_0),
#if (BCHP_MFD_1_REG_START)
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(MFD_1, Mpeg_1, TERMINAL, Terminal, Back, MFD_1),
#endif
#if (BCHP_MFD_2_REG_START)
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(MFD_2, Mpeg_2, TERMINAL, Terminal, Back, MFD_2),
#endif
#if (BCHP_MFD_3_REG_START)
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(MFD_3, Mpeg_3, TERMINAL, Terminal, Back, MFD_3),
#endif
#if (BCHP_MFD_4_REG_START)
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(MFD_4, Mpeg_4, TERMINAL, Terminal, Back, MFD_4),
#endif
#if (BCHP_MFD_5_REG_START)
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(MFD_5, Mpeg_5, TERMINAL, Terminal, Back, MFD_5),
#endif

	BBVN_P_MAKE_VNET_F_NODE(CAP_0 -> VFD_0, Video_0, CAP_0, CAP_0),
	BBVN_P_MAKE_VNET_F_NODE(CAP_1 -> VFD_1, Video_1, CAP_1, CAP_1),

#if (BCHP_VNET_B_CAP_2_SRC)
	BBVN_P_MAKE_VNET_F_NODE(CAP_2 -> VFD_2, Video_2, CAP_2, CAP_2),
#endif
#if (BCHP_VNET_B_CAP_3_SRC)
	BBVN_P_MAKE_VNET_F_NODE(CAP_3 -> VFD_3, Video_3, CAP_3, CAP_3),
#endif
#if (BCHP_VNET_B_CAP_4_SRC)
	BBVN_P_MAKE_VNET_F_NODE(CAP_4 -> VFD_4, Video_4, CAP_4, CAP_4),
#endif
#if (BCHP_VNET_B_CAP_5_SRC)
	BBVN_P_MAKE_VNET_F_NODE(CAP_5 -> VFD_5, Video_5, CAP_5, CAP_5),
#endif
#if (BCHP_VNET_B_CAP_6_SRC)
	BBVN_P_MAKE_VNET_F_NODE(CAP_6 -> VFD_6, Video_6, CAP_6, CAP_6),
#endif
#if (BCHP_VNET_B_CAP_7_SRC)
	BBVN_P_MAKE_VNET_F_NODE(CAP_7 -> VFD_7, Video_7, CAP_7, CAP_7),
#endif

#if (BCHP_HD_DVI_0_REG_START)
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(DVI_0, HdDvi_0, TERMINAL, Terminal, Back, Invalid),
#endif
#if (BCHP_HD_DVI_1_REG_START)
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(DVI_1, HdDvi_1, TERMINAL, Terminal, Back, Invalid),
#endif

#if (BCHP_IN656_0_REG_START)
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(656_0, Ccir656_0, TERMINAL, Terminal, Back, Invalid),
#endif
#if (BCHP_IN656_1_REG_START)
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(656_1, Ccir656_0, TERMINAL, Terminal, Back, Invalid),
#endif

	BBVN_P_MAKE_VNET_F_NODE(LPB_0,  Loopback_0, LOOPBACK_0, Invalid),
	BBVN_P_MAKE_VNET_F_NODE(LPB_1,  Loopback_1, LOOPBACK_1, Invalid),

#if (BCHP_VNET_B_LOOPBACK_2_SRC)
	BBVN_P_MAKE_VNET_F_NODE(LPB_2,  Loopback_2, LOOPBACK_2, Invalid),
#endif
#if (BCHP_VNET_B_LOOPBACK_3_SRC)
	BBVN_P_MAKE_VNET_F_NODE(LPB_3,  Loopback_3, LOOPBACK_3, Invalid),
#endif
#if (BCHP_VNET_B_LOOPBACK_4_SRC)
	BBVN_P_MAKE_VNET_F_NODE(LPB_4,  Loopback_4, LOOPBACK_4, Invalid),
#endif
#if (BCHP_VNET_B_LOOPBACK_5_SRC)
	BBVN_P_MAKE_VNET_F_NODE(LPB_5,  Loopback_5, LOOPBACK_5, Invalid),
#endif
#if (BCHP_VNET_B_LOOPBACK_6_SRC)
	BBVN_P_MAKE_VNET_F_NODE(LPB_6,  Loopback_6, LOOPBACK_6, Invalid),
#endif
#if (BCHP_VNET_B_LOOPBACK_7_SRC)
	BBVN_P_MAKE_VNET_F_NODE(LPB_7,  Loopback_7, LOOPBACK_7, Invalid),
#endif
#if (BCHP_VNET_B_LOOPBACK_8_SRC)
	BBVN_P_MAKE_VNET_F_NODE(LPB_8,  Loopback_8, LOOPBACK_8, Invalid),
#endif
#if (BCHP_VNET_B_LOOPBACK_9_SRC)
	BBVN_P_MAKE_VNET_F_NODE(LPB_9,  Loopback_9, LOOPBACK_9, Invalid),
#endif
#if (BCHP_VNET_B_LOOPBACK_10_SRC)
	BBVN_P_MAKE_VNET_F_NODE(LPB_10, Loopback_10, LOOPBACK_10, Invalid),
#endif
#if (BCHP_VNET_B_LOOPBACK_11_SRC)
	BBVN_P_MAKE_VNET_F_NODE(LPB_11, Loopback_11, LOOPBACK_11, Invalid),
#endif
#if (BCHP_VNET_B_LOOPBACK_12_SRC)
	BBVN_P_MAKE_VNET_F_NODE(LPB_12, Loopback_12, LOOPBACK_12, Invalid),
#endif
#if (BCHP_VNET_B_LOOPBACK_13_SRC)
	BBVN_P_MAKE_VNET_F_NODE(LPB_13, Loopback_13, LOOPBACK_13, Invalid),
#endif
#if (BCHP_VNET_B_LOOPBACK_14_SRC)
	BBVN_P_MAKE_VNET_F_NODE(LPB_14, Loopback_14, LOOPBACK_14, Invalid),
#endif

	BBVN_P_MAKE_VNET_F_DISABLED_NODE(OFF_0)
};

/***************************************************************************
 * VNET_B enum to MUX addr LUT
 */
#define BBVN_P_VNET_B_NODE_COUNT \
		(sizeof(s_astVnetBackMuxes)/sizeof(BBVN_P_VnetMux))

static const BBVN_P_VnetMux s_astVnetBackMuxes[] =
{
	BBVN_P_MAKE_VNET_B_NODE(SCL_0, Scaler_0, SCL_0, SCL_0),
	BBVN_P_MAKE_VNET_B_NODE(SCL_1, Scaler_1, SCL_1, SCL_1),
#if (BCHP_VNET_F_SCL_2_SRC)
	BBVN_P_MAKE_VNET_B_NODE(SCL_2, Scaler_2, SCL_2, SCL_2),
#endif
#if (BCHP_VNET_F_SCL_3_SRC)
	BBVN_P_MAKE_VNET_B_NODE(SCL_3, Scaler_3, SCL_3, SCL_3),
#endif
#if (BCHP_VNET_F_SCL_4_SRC)
	BBVN_P_MAKE_VNET_B_NODE(SCL_4, Scaler_4, SCL_4, SCL_4),
#endif
#if (BCHP_VNET_F_SCL_5_SRC)
	BBVN_P_MAKE_VNET_B_NODE(SCL_5, Scaler_5, SCL_5, SCL_5),
#endif
#if (BCHP_VNET_F_SCL_6_SRC)
	BBVN_P_MAKE_VNET_B_NODE(SCL_6, Scaler_6, SCL_6, SCL_6),
#endif
#if (BCHP_VNET_F_SCL_7_SRC)
	BBVN_P_MAKE_VNET_B_NODE(SCL_7, Scaler_7, SCL_7, SCL_7),
#endif

#if (BCHP_VNET_F_XSRC_0_SRC)
	BBVN_P_MAKE_VNET_B_NODE(XRC_0, Xsrc_0, XSRC_0, Invalid),
#endif
#if (BCHP_VNET_F_XSRC_1_SRC)
	BBVN_P_MAKE_VNET_B_NODE(XRC_1, Xsrc_1, XSRC_1, Invalid),
#endif

#if (BCHP_VNET_F_MAD32_0_SRC && BCHP_VNET_F_MAD32_1_SRC)
	BBVN_P_MAKE_VNET_B_NODE(MAD32_0, Mad32_0, MCVP_0, MVP_0),
	BBVN_P_MAKE_VNET_B_NODE(MAD32_1, Mad32_1, MCVP_1, MVP_0),
#elif (BCHP_VNET_F_MAD_0_SRC && BCHP_VNET_F_MAD_1_SRC)
	BBVN_P_MAKE_VNET_B_NODE(MAD32_0, Mad32_0, MAD_0, MAD_0),
	BBVN_P_MAKE_VNET_B_NODE(MAD32_1, Mad32_1, MAD_1, MAD_1),
#else
#ifdef BCHP_VNET_F_MVP_0_SRC
#if (BCHP_MVP_TOP_0_HW_CONFIGURATION_ANR_DEFAULT)
	BBVN_P_MAKE_VNET_B_NODE(MVP_0, Mvp_0, MVP_0, MVP_0),
#else
	BBVN_P_MAKE_VNET_B_NODE(MVP_0, Mvp_0, MVP_0, MAD_0),
#endif
#endif

#ifdef BCHP_VNET_F_MVP_1_SRC
#if (BCHP_MVP_TOP_0_HW_CONFIGURATION_ANR_DEFAULT)
	BBVN_P_MAKE_VNET_B_NODE(MVP_1, Mvp_1, MVP_1, MAD_0),
#else
	BBVN_P_MAKE_VNET_B_NODE(MVP_1, Mvp_1, MVP_1, MAD_1),
#endif
#endif

#ifdef BCHP_VNET_F_MVP_2_SRC
#if (BCHP_MVP_TOP_0_HW_CONFIGURATION_ANR_DEFAULT)
	BBVN_P_MAKE_VNET_B_NODE(MVP_2, Mvp_2, MVP_2, MAD_1),
#else
	BBVN_P_MAKE_VNET_B_NODE(MVP_2, Mvp_2, MVP_2, MAD_2),
#endif
#endif

#ifdef BCHP_VNET_F_MVP_3_SRC
#if (BCHP_MVP_TOP_0_HW_CONFIGURATION_ANR_DEFAULT)
	BBVN_P_MAKE_VNET_B_NODE(MVP_3, Mvp_2, MVP_3, MAD_2),
#else
	BBVN_P_MAKE_VNET_B_NODE(MVP_3, Mvp_3, MVP_3, MAD_3),
#endif
#endif

#ifdef BCHP_VNET_F_MVP_4_SRC
#if (BCHP_MVP_TOP_0_HW_CONFIGURATION_ANR_DEFAULT)
	BBVN_P_MAKE_VNET_B_NODE(MVP_4, Mvp_3, MVP_4, MAD_3),
#else
	BBVN_P_MAKE_VNET_B_NODE(MVP_4, Mvp_4, MVP_4, MAD_4),
#endif
#endif

#ifdef BCHP_VNET_F_MVP_5_SRC
#if (BCHP_MVP_TOP_0_HW_CONFIGURATION_ANR_DEFAULT)
	BBVN_P_MAKE_VNET_B_NODE(MVP_5, Mvp_5, MVP_5, MAD_4),
#else
	BBVN_P_MAKE_VNET_B_NODE(MVP_5, Mvp_5, MVP_5, MAD_5),
#endif /* ANR capable MAD? */
#endif /* MVP_n */
#endif /* MAD32 */

#if (BCHP_VNET_F_TNTD_0_SRC)
	BBVN_P_MAKE_VNET_B_NODE(TNT_0, TntD_0, TNTD_0, Invalid),
#endif

	BBVN_P_MAKE_VNET_B_NODE(DNR_0, Dnr_0, DNR_0, Invalid),
#if (BCHP_VNET_F_DNR_1_SRC)
	BBVN_P_MAKE_VNET_B_NODE(DNR_1, Dnr_1, DNR_1, Invalid),
#endif
#if (BCHP_VNET_F_DNR_2_SRC)
	BBVN_P_MAKE_VNET_B_NODE(DNR_2, Dnr_2, DNR_2, Invalid),
#endif
#if (BCHP_VNET_F_DNR_3_SRC)
	BBVN_P_MAKE_VNET_B_NODE(DNR_3, Dnr_3, DNR_3, Invalid),
#endif
#if (BCHP_VNET_F_DNR_4_SRC)
	BBVN_P_MAKE_VNET_B_NODE(DNR_4, Dnr_4, DNR_4, Invalid),
#endif
#if (BCHP_VNET_F_DNR_5_SRC)
	BBVN_P_MAKE_VNET_B_NODE(DNR_5, Dnr_5, DNR_5, Invalid),
#endif

	BBVN_P_MAKE_VNET_B_NODE(FCH_0, Channel_0, FCH_0, Invalid),
	BBVN_P_MAKE_VNET_B_NODE(FCH_1, Channel_1, FCH_1, Invalid),
#if (BCHP_VNET_F_FCH_2_SRC)
	BBVN_P_MAKE_VNET_B_NODE(FCH_2, Channel_2, FCH_2, Invalid),
#endif
#if (BCHP_VNET_F_FCH_3_SRC)
	BBVN_P_MAKE_VNET_B_NODE(FCH_3, Channel_3, FCH_3, Invalid),
#endif
#if (BCHP_VNET_F_FCH_4_SRC)
	BBVN_P_MAKE_VNET_B_NODE(FCH_4, Channel_4, FCH_4, Invalid),
#endif
#if (BCHP_VNET_F_FCH_5_SRC)
	BBVN_P_MAKE_VNET_B_NODE(FCH_5, Channel_5, FCH_5, Invalid),
#endif
#if (BCHP_VNET_F_FCH_6_SRC)
	BBVN_P_MAKE_VNET_B_NODE(FCH_6, Channel_6, FCH_6, Invalid),
#endif
#if (BCHP_VNET_F_FCH_7_SRC)
	BBVN_P_MAKE_VNET_B_NODE(FCH_7, Channel_7, FCH_7, Invalid),
#endif
#if (BCHP_VNET_F_FCH_8_SRC)
	BBVN_P_MAKE_VNET_B_NODE(FCH_8, Channel_8, FCH_8, Invalid),
#endif
#if (BCHP_VNET_F_FCH_9_SRC)
	BBVN_P_MAKE_VNET_B_NODE(FCH_9, Channel_9, FCH_9, Invalid),
#endif
#if (BCHP_VNET_F_FCH_10_SRC)
	BBVN_P_MAKE_VNET_B_NODE(FCH_10, Channel_10, FCH_10, Invalid),
#endif
#if (BCHP_VNET_F_FCH_11_SRC)
	BBVN_P_MAKE_VNET_B_NODE(FCH_11, Channel_11, FCH_11, Invalid),
#endif
#if (BCHP_VNET_F_FCH_12_SRC)
	BBVN_P_MAKE_VNET_B_NODE(FCH_12, Channel_12, FCH_12, Invalid),
#endif
#if (BCHP_VNET_F_FCH_13_SRC)
	BBVN_P_MAKE_VNET_B_NODE(FCH_13, Channel_13, FCH_13, Invalid),
#endif

	BBVN_P_MAKE_VNET_B_DISABLED_NODE(OFF_0)
};

/***************************************************************************
 * Terminal nodes
 */
#define BBVN_P_TERMINAL_NODE_COUNT \
	(sizeof(s_astVnetTerminalMuxes)/sizeof(BBVN_P_VnetMux))

static const BBVN_P_VnetMux s_astVnetTerminalMuxes[] =
{
	BBVN_P_MAKE_VNET_B_TERMINAL_NODE(BOX_0, LBOX_0,  Front, Terminal),

#if (BCHP_VNET_F_LBOX_1_SRC)
	BBVN_P_MAKE_VNET_B_TERMINAL_NODE(BOX_1, LBOX_1,  Front, Terminal),
#endif
#if (BCHP_VNET_F_LBOX_2_SRC)
	BBVN_P_MAKE_VNET_B_TERMINAL_NODE(BOX_2, LBOX_2,  Front, Terminal),
#endif

	BBVN_P_MAKE_VNET_B_TERMINAL_NODE(DRN_0, DRAIN_0, Front, Terminal),

#if (BCHP_VNET_F_DRAIN_1_SRC)
	BBVN_P_MAKE_VNET_B_TERMINAL_NODE(DRN_1, DRAIN_1, Front, Terminal),
#endif
#if (BCHP_VNET_F_DRAIN_2_SRC)
	BBVN_P_MAKE_VNET_B_TERMINAL_NODE(DRN_2, DRAIN_2, Front, Terminal),
#endif
#if (BCHP_VNET_F_DRAIN_3_SRC)
	BBVN_P_MAKE_VNET_B_TERMINAL_NODE(DRN_3, DRAIN_3, Front, Terminal),
	BBVN_P_MAKE_VNET_B_TERMINAL_NODE(DRN_4, DRAIN_4, Front, Terminal),
#endif

	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CAP_0, Terminal, TERMINAL, Back, Terminal, CAP_0),
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CAP_1, Terminal, TERMINAL, Back, Terminal, CAP_1),
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CAP_2, Terminal, TERMINAL, Back, Terminal, CAP_2),
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CAP_3, Terminal, TERMINAL, Back, Terminal, CAP_3),
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CAP_4, Terminal, TERMINAL, Back, Terminal, CAP_4),
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CAP_5, Terminal, TERMINAL, Back, Terminal, CAP_5),
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CAP_6, Terminal, TERMINAL, Back, Terminal, CAP_6),
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CAP_7, Terminal, TERMINAL, Back, Terminal, CAP_7),

	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CMP_0_V0, Terminal, CMP_0_V0, Back, Terminal, CMP_0),
#if (BCHP_VNET_B_CMP_0_V1_SRC)
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CMP_0_V1, Terminal, CMP_0_V1, Back, Terminal, CMP_0),
#endif
#if (BCHP_VNET_B_CMP_1_V0_SRC)
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CMP_1_V0, Terminal, CMP_1_V0, Back, Terminal, CMP_1),
#endif
#if (BCHP_VNET_B_CMP_1_V1_SRC)
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CMP_1_V1, Terminal, CMP_1_V1, Back, Terminal, CMP_1),
#endif
#if (BCHP_VNET_B_CMP_2_V0_SRC)
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CMP_2_V0, Terminal, CMP_2_V0, Back, Terminal, CMP_2),
#endif
#if (BCHP_VNET_B_CMP_3_V0_SRC)
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CMP_3_V0, Terminal, CMP_3_V0, Back, Terminal, CMP_3),
#endif
#if (BCHP_VNET_B_CMP_4_V0_SRC)
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CMP_4_V0, Terminal, CMP_4_V0, Back, Terminal, CMP_4),
#endif
#if (BCHP_VNET_B_CMP_5_V0_SRC)
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CMP_5_V0, Terminal, CMP_5_V0, Back, Terminal, CMP_5),
#endif
#if (BCHP_VNET_B_CMP_6_V0_SRC)
	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CMP_6_V0, Terminal, CMP_6_V0, Back, Terminal, CMP_6),
#endif

	BBVN_P_MAKE_VNET_F_TERMINAL_NODE(CRC_0, Terminal, CRC, Back, Terminal, Invalid)
};

/* Chip based loop count optimization. */
#define BBVN_P_NUM_HDM_MAX    (BBVN_NUM_HDMI_PATH)
#define BBVN_P_NUM_AIT_MAX    (sizeof(s_aAitCmpIdx)/sizeof(s_aAitCmpIdx[0]))
#define BBVN_P_NUM_STG_MAX    (sizeof(s_aStgCmpIdx)/sizeof(s_aStgCmpIdx[0]))

#define BBVN_P_MAKE_STG_INFO(stg_id, core_id, channel_id) \
	{(stg_id), (core_id), (channel_id), BCHP_VEC_CFG_STG_##stg_id##_SOURCE, BAVC_CoreId_eVIP_##channel_id}

#define BBVN_P_MAKE_AIT_INFO(ait_id) \
	{(BCHP_VEC_CFG_IT_##ait_id##_SOURCE)}

static const struct
{
	uint8_t        iStgId;
	uint8_t        iViceCoreId;
	uint8_t        iViceChannelId;
	uint32_t       ulStgMuxAddr;
	BAVC_CoreId    eVipCoreId;
} s_aStgCmpIdx[] = {
#ifdef BCHP_VEC_CFG_STG_0_SOURCE
		  BBVN_P_MAKE_STG_INFO(0, 0, 0)
#endif
#ifdef BCHP_VEC_CFG_STG_1_SOURCE
		, BBVN_P_MAKE_STG_INFO(1, 0, 1)
#endif
#ifdef BCHP_VEC_CFG_STG_2_SOURCE
		, BBVN_P_MAKE_STG_INFO(2, 1, 0)
#endif
#ifdef BCHP_VEC_CFG_STG_3_SOURCE
		, BBVN_P_MAKE_STG_INFO(3, 1, 1)
#endif
#ifdef BCHP_VEC_CFG_STG_4_SOURCE
		, BBVN_P_MAKE_STG_INFO(4, 0, 2)
#endif
#ifdef BCHP_VEC_CFG_STG_5_SOURCE
		, BBVN_P_MAKE_STG_INFO(5, 1, 2)
#endif
};

static const struct
{
	uint32_t       ulAitMuxAddr;
} s_aAitCmpIdx[] = {
#ifdef BCHP_VEC_CFG_IT_0_SOURCE
		  BBVN_P_MAKE_AIT_INFO(0)
#endif
#ifdef BCHP_VEC_CFG_IT_1_SOURCE
		, BBVN_P_MAKE_AIT_INFO(1)
#endif
#ifdef BCHP_VEC_CFG_IT_2_SOURCE
		, BBVN_P_MAKE_AIT_INFO(2)
#endif
};

/***************************************************************************
 * Main monitoring function.
 */
static BERR_Code BBVN_P_TraverseBvn_recursive
	( BREG_Handle                      hReg,
	  const BBVN_P_VnetMux            *pCurMux,
	  const BAVC_CoreList             *pSecureCores,
	  BBVN_P_PathStatus               *pPath
	)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ulSource = 0xffffffff;
	const BBVN_P_VnetMux *pUpStreamMux;
	char chSecured = '-';

	if(BBVN_P_VnetMuxType_eDisabled != pCurMux->eUpstreamMuxType)
	{
		bool bFoundNode = false;

		if(BAVC_CoreId_eInvalid != pCurMux->eCoreId)
		{
			if(pSecureCores->aeCores[pCurMux->eCoreId])
			{
				chSecured = 's';
			}
			else
			{
				chSecured = 'u';
			}

			/* Recording all the cores in this path */
			if(pPath->stBvnCores.aeCores[pCurMux->eCoreId])
			{
				BDBG_WRN(("Possible infinite loop or security violation."));
			}
			pPath->stBvnCores.aeCores[pCurMux->eCoreId] = true;
		}

		if(BBVN_P_VnetMuxType_eTerminal != pCurMux->eUpstreamMuxType)
		{
			/* Read SOURCE value */
			ulSource = BREG_Read32(hReg, pCurMux->ulAddress);
			ulSource = BCHP_GET_FIELD_DATA(ulSource, VNET_B_CAP_0_SRC, SOURCE);
			BDBG_MODULE_MSG(BBVN_L2, ("Mux[%16s]: core_id[%s][%02d][%c] addr[0x%08x], upstream_src[0x%02x]",
				pCurMux->achName, s_aCoreInfoTbl[pCurMux->eCoreId].achName, pCurMux->eCoreId,
				chSecured, pCurMux->ulAddress, ulSource));
		}
		else
		{
			BDBG_MODULE_MSG(BBVN_L2, ("Mux[%16s]: core_id[%s][%02d][%c]",
				pCurMux->achName, s_aCoreInfoTbl[pCurMux->eCoreId].achName, pCurMux->eCoreId,
				chSecured));
		}

		/* Look for corresponding SOURCE mux based on SOURCE value using a two-table search.
		   First, is check if SOURCE value is in BBVN_P_VneF or BBVN_P_VnetB enum. If so,
		   using the found enum entry, search for the mux address in the VnetBackNodes or
		   VnetFrontNodes table. */
		if(BBVN_P_VnetMuxType_eBack == pCurMux->eUpstreamMuxType)
		{
			BBVN_P_VnetB eVnetBNode;

			for (eVnetBNode = BBVN_P_VnetB_eScaler_0;
				 eVnetBNode < BBVN_P_VnetB_eInvalid;
				 eVnetBNode++)
			{
				if(ulSource == (uint32_t)eVnetBNode)
				{
					unsigned i;
					for (i=0; i<=BBVN_P_VNET_B_NODE_COUNT; i++)
					{
						if(eVnetBNode == s_astVnetBackMuxes[i].nodeType.eBack)
						{
							bFoundNode = true;
							pUpStreamMux = &s_astVnetBackMuxes[i];
							break;
						}
					}
				}
				if(bFoundNode) break;
			}
		}
		else if(BBVN_P_VnetMuxType_eFront == pCurMux->eUpstreamMuxType)
		{
			BBVN_P_VnetF eVnetFNode;

			for (eVnetFNode = BBVN_P_VnetF_eMpeg_0;
				 eVnetFNode < BBVN_P_VnetF_eInvalid;
				 eVnetFNode++)
			{
				if(ulSource == (uint32_t)eVnetFNode)
				{
					unsigned i;
					for (i = 0; i <= BBVN_P_VNET_F_NODE_COUNT; i++)
					{
						if(eVnetFNode == s_astVnetFrontMuxes[i].nodeType.eFront)
						{
							bFoundNode = true;
							pUpStreamMux = &s_astVnetFrontMuxes[i];
							break;
						}
					}
				}
				if(bFoundNode) break;
			}
		}

		if(bFoundNode)
		{
			err = BBVN_P_TraverseBvn_recursive(hReg, pUpStreamMux, pSecureCores,pPath);
			if(err != BERR_SUCCESS) return err;
		}

		if(BBVN_P_VnetMuxType_eTerminal == pCurMux->eDownstreamMuxType)
		{
			pPath->iStrIndex += BKNI_Snprintf(&pPath->achBvnMap[pPath->iStrIndex],
				BBVN_P_VNET_NAME_LEN, "%s", pCurMux->achName);
		}
		else if((BBVN_P_VnetF_eDisabled != pCurMux->nodeType.eFront) &&
		        (BBVN_P_VnetB_eDisabled != pCurMux->nodeType.eBack))
		{
			pPath->iStrIndex += BKNI_Snprintf(&pPath->achBvnMap[pPath->iStrIndex],
				BBVN_P_VNET_NAME_LEN, "%s -> ", pCurMux->achName);
			if(pSecureCores->aeCores[pCurMux->eCoreId])
			{
				pPath->bSecuredCt = true;
			}
		}
	}

	return err;
}

/***************************************************************************/
/* TODO: Rename if necessary to conform with SAGE code guideline and style.*/
/***************************************************************************/
typedef struct BBVN_P_Monitor_Context
{
	BDBG_OBJECT(BBVN_BVN)

	BREG_Handle                        hReg;

	/* Reduced stack size */
	BBVN_P_PathStatus                  stPathStatus;
	uint8_t                            aiHdmCmpIdx[BBVN_P_NUM_HDM_MAX]; /* aiHdmCmpIdx[DTG_x] = CMP_x */
	uint8_t                            aiAitCmpIdx[BBVN_P_NUM_AIT_MAX]; /* aiAitCmpIdx[AIT_x] = CMP_x */
	uint8_t                            aiStgCmpIdx[BBVN_P_NUM_STG_MAX]; /* aiStgCmpIdx[STG_x] = CMP_x */

	/* collection of status of all paths */
	uint8_t                            abCmpSecuredCt[BBVN_P_COMPOSITOR_MAX][BBVN_P_WINDOW_MAX];
	uint8_t                            abCmpViolation[BBVN_P_COMPOSITOR_MAX][BBVN_P_WINDOW_MAX];
	uint32_t                           aulCmpHSize[BBVN_P_COMPOSITOR_MAX];
	uint32_t                           aulCmpVSize[BBVN_P_COMPOSITOR_MAX];

} BBVN_P_Monitor_Context;

/***************************************************************************
Helper functions
****************************************************************************/
/***************************************************************************
 * Get Compositor index if go thru decimator
 */
static void BBVN_P_GetDecimCmpIdx
	( BREG_Handle                      hReg,
	  uint8_t                         *piCmpIdx
	)
{
#ifdef BCHP_VEC_CFG_DECIM_0_SOURCE
	uint32_t ulSource = BREG_Read32(hReg, BCHP_VEC_CFG_DECIM_0_SOURCE);
	*piCmpIdx = BCHP_GET_FIELD_DATA(ulSource, VEC_CFG_DECIM_0_SOURCE, SOURCE);
#else
	BSTD_UNUSED(hReg);
	*piCmpIdx = BBVN_P_INVALID_COMPOSITOR_IDX;
#endif
	return;
}

/***************************************************************************
 * Get Compositor index that drives HDMI
 */
static void BBVN_P_GetVecSourceCmpIdx
	( BREG_Handle                      hReg,
	  uint32_t                         ulVecSrcMuxAddr,
	  uint8_t                         *piCmpIdx
	)
{
#if BCHP_VEC_CFG_IT_0_SOURCE
	uint32_t ulSource = BREG_Read32(hReg, ulVecSrcMuxAddr);
	*piCmpIdx = BCHP_GET_FIELD_DATA(ulSource, VEC_CFG_IT_0_SOURCE, SOURCE);

	if(*piCmpIdx == BCHP_VEC_CFG_IT_0_SOURCE_SOURCE_DECIM_0)
	{
		BBVN_P_GetDecimCmpIdx(hReg, piCmpIdx);
	}
	else if(*piCmpIdx > BCHP_VEC_CFG_IT_0_SOURCE_SOURCE_DECIM_0)
	{
		*piCmpIdx = BBVN_P_INVALID_COMPOSITOR_IDX;
	}
#else
		BSTD_UNUSED(piCmpIdx);
		BSTD_UNUSED(ulVecSrcMuxAddr);
		*piCmpIdx = BBVN_P_INVALID_COMPOSITOR_IDX;
#endif

	return;
}

/***************************************************************************
 * Get Compositor index that drives HDMI
 */
static void BBVN_P_GetHdmCmpIdx
	( BREG_Handle                      hReg,
	  uint8_t                          aiHdmCmpIdx[BBVN_P_NUM_HDM_MAX] /* aiHdmCmpIdx[HDM_x] = CMP_x */
	)
{
	BKNI_Memset(aiHdmCmpIdx, BBVN_P_INVALID_COMPOSITOR_IDX, sizeof(aiHdmCmpIdx[0]) * BBVN_P_NUM_HDM_MAX);

#ifdef BCHP_DVI_DTG_0_REG_START
	BBVN_P_GetVecSourceCmpIdx(hReg, BCHP_VEC_CFG_DVI_DTG_0_SOURCE, &aiHdmCmpIdx[0]);
#else
	BSTD_UNUSED(hReg);
#endif

	return;
}

/***************************************************************************
 * Get Compositor index information for STG.
 */
static void BBVN_P_GetStgCmpIdx
	( BREG_Handle                      hReg,
	  uint8_t                          aiStgCmpIdx[BBVN_P_NUM_STG_MAX] /* aiStgCmpIdx[STG_x] = CMP_x */
	)
{
	uint8_t i;
	BKNI_Memset(aiStgCmpIdx, BBVN_P_INVALID_COMPOSITOR_IDX, sizeof(aiStgCmpIdx[0]) * BBVN_P_NUM_STG_MAX);

	for(i = 0; i < BBVN_P_NUM_STG_MAX; i++)
	{
		BBVN_P_GetVecSourceCmpIdx(hReg, s_aStgCmpIdx[i].ulStgMuxAddr, &aiStgCmpIdx[i]);
	}

	return;
}

/***************************************************************************
 * Get Compositor index information for STG.
 */
static void BBVN_P_GetAItCmpIdx
	( BREG_Handle                      hReg,
	  uint8_t                          aiAitCmpIdx[BBVN_P_NUM_AIT_MAX] /* aiAitCmpIdx[IT_x] = CMP_x */
	)
{
	uint8_t i;
	BKNI_Memset(aiAitCmpIdx, BBVN_P_INVALID_COMPOSITOR_IDX, sizeof(aiAitCmpIdx[0]) * BBVN_P_NUM_AIT_MAX);

	for(i = 0; i < BBVN_P_NUM_AIT_MAX; i++)
	{
		BBVN_P_GetVecSourceCmpIdx(hReg, s_aAitCmpIdx[i].ulAitMuxAddr, &aiAitCmpIdx[i]);
	}

	return;
}

/***************************************************************************
 * Get Compositor/Window index information from given VNET Mux of a
 * compositor
 */
#define BBVN_P_MAKE_CMP_INFO(cmp_idx, win_idx)      \
	 {BCHP_VNET_B_CMP_##cmp_idx##_V##win_idx##_SRC, \
	  BCHP_CMP_##cmp_idx##_REG_START - BCHP_CMP_0_REG_START, cmp_idx, win_idx}

/***************************************************************************
 * Get Compositor/Window index information from given VNET Mux of a
 * compositor
 */
static bool BBVN_P_GetCmpInfo
	( BREG_Handle                      hReg,
	  uint32_t                         ulCmpVnetAddr,
	  uint8_t                         *piCmpIdx,
	  uint8_t                         *piWinIdx,
	  uint32_t                        *pulHSize,
	  uint32_t                        *pulVSize
	)
{
	uint8_t i;

	static const struct
	{
		uint32_t   ulCmpVnetAddr;
		uint32_t   ulCanvasAddr;
		uint8_t    iCmpIdx;
		uint8_t    iWinIdx;
	} s_aVnetCmpIdx[] = {
		 BBVN_P_MAKE_CMP_INFO(0, 0)
#ifdef BCHP_VNET_B_CMP_0_V1_SRC
		,BBVN_P_MAKE_CMP_INFO(0, 1)
#endif
#ifdef BCHP_VNET_B_CMP_1_V0_SRC
		,BBVN_P_MAKE_CMP_INFO(1, 0)
#endif
#ifdef BCHP_VNET_B_CMP_1_V1_SRC
		,BBVN_P_MAKE_CMP_INFO(1, 1)
#endif
#ifdef BCHP_VNET_B_CMP_2_V0_SRC
		,BBVN_P_MAKE_CMP_INFO(2, 0)
#endif
#ifdef BCHP_VNET_B_CMP_3_V0_SRC
		,BBVN_P_MAKE_CMP_INFO(3, 0)
#endif
#ifdef BCHP_VNET_B_CMP_4_V0_SRC
		,BBVN_P_MAKE_CMP_INFO(4, 0)
#endif
#ifdef BCHP_VNET_B_CMP_5_V0_SRC
		,BBVN_P_MAKE_CMP_INFO(5, 0)
#endif
#ifdef BCHP_VNET_B_CMP_6_V0_SRC
		,BBVN_P_MAKE_CMP_INFO(6, 0)
#endif
	};

	for(i = 0; i < sizeof(s_aVnetCmpIdx)/sizeof(s_aVnetCmpIdx[0]); i++)
	{
		if(s_aVnetCmpIdx[i].ulCmpVnetAddr == ulCmpVnetAddr)
		{
			uint32_t ulData;
			*piCmpIdx = s_aVnetCmpIdx[i].iCmpIdx;
			*piWinIdx = s_aVnetCmpIdx[i].iWinIdx;

			ulData = BREG_Read32(hReg, BCHP_CMP_0_CANVAS_SIZE + s_aVnetCmpIdx[i].ulCanvasAddr);
			*pulHSize = BCHP_GET_FIELD_DATA(ulData, CMP_0_CANVAS_SIZE, HSIZE);
			*pulVSize = BCHP_GET_FIELD_DATA(ulData, CMP_0_CANVAS_SIZE, VSIZE);
			return true;
		}
	}
	return false;
}

/***************************************************************************
Summary: Begin of all public functions
****************************************************************************/
BERR_Code BBVN_Monitor_Init
	( BBVN_Monitor_Handle             *phBvn,    /* [out] a reference to a BBVN_Handle. */
	  BREG_Handle                      hReg      /* [in] to traverse vnet/vec routing, maybe opaque handle with hReg. */
	)
{
	BBVN_Monitor_Handle hBvn = NULL;

	BDBG_ENTER(BBVN_Monitor_Init);

	/* Make sure array is bounded for new chip.*/
	BDBG_ASSERT(BBVN_P_NUM_HDM_MAX <= BBVN_NUM_HDMI_PATH);
	BDBG_ASSERT(BBVN_P_NUM_AIT_MAX <= BBVN_NUM_ANALOG_PATH);
	BDBG_ASSERT(BBVN_P_NUM_STG_MAX <= BBVN_NUM_XCODE_PATH);

	BDBG_MSG(("Chip configuration NUM_HDMI=%d, NUM_ANALOG=%d, NUM_XCODE=%d",
		BBVN_P_NUM_HDM_MAX, BBVN_P_NUM_AIT_MAX, BBVN_P_NUM_STG_MAX));

	/* Create opaque handle */
	hBvn = (BBVN_Monitor_Handle)BKNI_Malloc(sizeof(BBVN_P_Monitor_Context));
	if(!hBvn)
	{
		BDBG_ERR(("Out of System Memory"));
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}

	/* Initialize context of handle */
	BKNI_Memset(hBvn, 0x0, sizeof(BBVN_P_Monitor_Context));
	BDBG_OBJECT_SET(hBvn, BBVN_BVN);

	/* Fill in */
	hBvn->hReg = hReg;

	/* Return to user */
	*phBvn = hBvn;

	BDBG_MSG(("BBVN Context size is %d bytes.", sizeof(BBVN_P_Monitor_Context)));

	BDBG_LEAVE(BBVN_Monitor_Init);

	return BERR_SUCCESS;
}

/***************************************************************************
Summary:
****************************************************************************/
BERR_Code BBVN_Monitor_Uninit
	( BBVN_Monitor_Handle              hBvn     /* [out] BVN Monitor Handle becomes invalid. */
	)
{
	BDBG_ENTER(BBVN_Monitor_Uninit);

	BDBG_OBJECT_ASSERT(hBvn, BBVN_BVN);

	/* Do other uninit here */

	BDBG_OBJECT_DESTROY(hBvn, BBVN_BVN);
	BKNI_Free(hBvn);

	BDBG_LEAVE(BBVN_Monitor_Uninit);
	return BERR_SUCCESS;
}

/***************************************************************************
Summary:
****************************************************************************/
void BBVN_Monitor_isr
	( BBVN_Monitor_Handle              hBvnMonitor,     /* [in] A valid BVN Monitor Handle created earlier. */
	  const BAVC_CoreList             *pSecureCores,    /* [in] list of secure cores */
	  BBVN_Monitor_Status             *pStatus          /* [out] BVN status */
	)
{
	uint8_t i, j;
	uint32_t ulSource;

/* TESTING CODE ONLY TO BE: removed */
#ifndef BBVN_TESTING
#define BBVN_TESTING (0)
#endif
#if BBVN_TESTING
	BAVC_CoreList stSecureCores, *pSCores = &stSecureCores;
	BKNI_Memset(pSCores, 0, sizeof(BAVC_CoreList));
	pSCores->aeCores[BAVC_CoreId_eMFD_0] = true;
	pSCores->aeCores[BAVC_CoreId_eMVP_0] = true;
	pSCores->aeCores[BAVC_CoreId_eSCL_0] = true;
	pSCores->aeCores[BAVC_CoreId_eCAP_0] = true;
	pSCores->aeCores[BAVC_CoreId_eVFD_0] = true;
	pSecureCores = pSCores;
#endif

	BDBG_ENTER(BBVN_Monitor_isr);
	BDBG_OBJECT_ASSERT(hBvnMonitor, BBVN_BVN);

	/* Cleared out to avoid unitialized new value. */
	BKNI_Memset(pStatus, 0, sizeof(BBVN_Monitor_Status));
	BKNI_Memset(hBvnMonitor->abCmpSecuredCt, 0, sizeof(hBvnMonitor->abCmpSecuredCt));
	BKNI_Memset(hBvnMonitor->abCmpViolation, 0, sizeof(hBvnMonitor->abCmpViolation));

	/* For sanity checks to see if pSecureCores */
	BDBG_MODULE_MSG(BBVN_L3, ("Secure Cores:"));
	for(i = 0; i < BAVC_CoreId_eMax; i++)
	{
		if(pSecureCores->aeCores[i])
		{
			BDBG_MODULE_MSG(BBVN_L3, ("\t%s(%02d)", s_aCoreInfoTbl[i].achName, i));
		}
	}

	/* Traverse BVN (bvn only no vec/stg) and mark each CMP_x path
	 *    [1] if it has secure content or not
	 *    [2] if bvn's memory client violation. */
	for (i = 0; i < BBVN_P_TERMINAL_NODE_COUNT; i++)
	{
		uint8_t iCmpIdx, iWinIdx;
		uint32_t ulHSize, ulVSize;
		BBVN_P_PathStatus *pPath = &hBvnMonitor->stPathStatus;
		const BBVN_P_VnetMux *pCurMux = &s_astVnetTerminalMuxes[i];

		if(BCHP_VNET_F_TERMINAL_SRC != pCurMux->ulAddress)
		{
			ulSource = BREG_Read32(hBvnMonitor->hReg, pCurMux->ulAddress);
			ulSource = BCHP_GET_FIELD_DATA(ulSource, VNET_B_CAP_0_SRC, SOURCE);

			if(BBVN_P_VnetF_eDisabled != ulSource)
			{
				/* Clear the state for each path */
				BKNI_Memset(pPath, 0, sizeof(BBVN_P_PathStatus));

				BBVN_P_TraverseBvn_recursive(hBvnMonitor->hReg, pCurMux, pSecureCores, pPath);

				/* Check if the path has violation */
				for(j = 0; j < BAVC_CoreId_eMax; j++)
				{
					if((pPath->bSecuredCt) &&
					   (pPath->stBvnCores.aeCores[j]) && (!pSecureCores->aeCores[j]))
					{
						pPath->bViolation = true;
					}
				}

				BDBG_MODULE_MSG(BBVN_L1,("%s - [%c][%c]", pPath->achBvnMap,
					pPath->bSecuredCt ? 's' : 'u', pPath->bViolation ? 'y' : 'n'));
				BDBG_MODULE_MSG(BBVN_L2,(""));

				if(BBVN_P_GetCmpInfo(hBvnMonitor->hReg, pCurMux->ulAddress,
					&iCmpIdx, &iWinIdx, &ulHSize, &ulVSize))
				{
					hBvnMonitor->abCmpSecuredCt[iCmpIdx][iWinIdx] = pPath->bSecuredCt;
					hBvnMonitor->abCmpViolation[iCmpIdx][iWinIdx] = pPath->bViolation;
					/* Records its width & height */
					hBvnMonitor->aulCmpHSize[iCmpIdx] = ulHSize;
					hBvnMonitor->aulCmpVSize[iCmpIdx] = ulVSize;
				}
			}
		}
	}

	/* Find out what compositor drives the outputs */
	BBVN_P_GetHdmCmpIdx(hBvnMonitor->hReg, hBvnMonitor->aiHdmCmpIdx);
	BBVN_P_GetAItCmpIdx(hBvnMonitor->hReg, hBvnMonitor->aiAitCmpIdx);
	BBVN_P_GetStgCmpIdx(hBvnMonitor->hReg, hBvnMonitor->aiStgCmpIdx);

	BDBG_MSG(("***************************************************************"));
	for(i = 0; i < BBVN_P_COMPOSITOR_MAX; i++)
	{
		/* (1) Populate HDMI: status */
		for(j = 0; j < BBVN_P_NUM_HDM_MAX; j++)
		{
			if(i == hBvnMonitor->aiHdmCmpIdx[j])
			{
				/* HDMI has secured content if either Main or PIP has secured content */
				if(hBvnMonitor->abCmpSecuredCt[i][0] ||
				   hBvnMonitor->abCmpSecuredCt[i][1])
				{
					pStatus->Hdmi[j].bSecure = true;
					pStatus->Hdmi[j].ulHSize = hBvnMonitor->aulCmpHSize[i];
					pStatus->Hdmi[j].ulVSize = hBvnMonitor->aulCmpVSize[i];
				}
				BDBG_MSG(("Cmp[%d] (%dx%d) routes '%s' content to Hdmi[%d].", i,
					pStatus->Hdmi[j].ulHSize,
					pStatus->Hdmi[j].ulVSize,
					pStatus->Hdmi[j].bSecure ? "secured" : "non-secured", j));
			}
		}
		/* (2) Populate ANALOG: status */
		for(j = 0; j < BBVN_P_NUM_AIT_MAX; j++)
		{
			if(i == hBvnMonitor->aiAitCmpIdx[j])
			{
				if(hBvnMonitor->abCmpSecuredCt[i][0] ||
				   hBvnMonitor->abCmpSecuredCt[i][1])
				{
					pStatus->Analog[j].bSecure = true;
					pStatus->Analog[j].ulHSize = hBvnMonitor->aulCmpHSize[i];
					pStatus->Analog[j].ulVSize = hBvnMonitor->aulCmpVSize[i];
				}
				BDBG_MSG(("Cmp[%d] (%dx%d) routes '%s' content to Analog[%d].", i,
					pStatus->Analog[j].ulHSize,
					pStatus->Analog[j].ulVSize,
					pStatus->Analog[j].bSecure ? "secured" : "non-secured", j));
			}
		}
		/* (3) Populate STG: status */
		for(j = 0; j < BBVN_P_NUM_STG_MAX; j++)
		{
			if(i == hBvnMonitor->aiStgCmpIdx[j])
			{
				if(hBvnMonitor->abCmpSecuredCt[i][0] ||
				   hBvnMonitor->abCmpSecuredCt[i][1])
				{
					pStatus->Xcode[j].bSecure = true;
					pStatus->Xcode[j].ulHSize = hBvnMonitor->aulCmpHSize[i];
					pStatus->Xcode[j].ulVSize = hBvnMonitor->aulCmpVSize[i];
				}

				/* If video goes thru STG which goes thru VIP report violation
				 * when VIP is not part of pSecureCores list */
				if(pStatus->Xcode[j].bSecure && !pSecureCores->aeCores[s_aStgCmpIdx[j].eVipCoreId])
				{
					hBvnMonitor->abCmpViolation[i][0] = true;
					hBvnMonitor->abCmpViolation[i][1] = true;
				}
				BDBG_MSG(("Cmp[%d] (%dx%d) routes '%s' content to Stg[%d]-> %s.", i,
					pStatus->Xcode[j].ulHSize,
					pStatus->Xcode[j].ulVSize,
					pStatus->Xcode[j].bSecure ? "secured" : "non-secured", j,
					s_aCoreInfoTbl[s_aStgCmpIdx[j].eVipCoreId].achName));
			}
		}

		/* Violation if secured content is routed to non-secured memory clients */
		if(hBvnMonitor->abCmpViolation[i][0] ||
		   hBvnMonitor->abCmpViolation[i][1])
		{
			pStatus->bViolation = true;
		}
	}

	BDBG_MSG(("***************************************************************"));
	BDBG_MSG(("                    BVN Violation = '%s'",
		pStatus->bViolation ? "yes" : "no"));
	BDBG_MSG(("***************************************************************"));
	BDBG_MSG((""));

	BDBG_LEAVE(BBVN_Monitor_isr);
	return;
}
/* End of file */
