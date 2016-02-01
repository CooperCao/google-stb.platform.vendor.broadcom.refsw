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
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BVDC_CAPTURE_PRIV_H__
#define BVDC_CAPTURE_PRIV_H__

#include "bvdc.h"
#include "bchp_common.h"
#include "bvdc_common_priv.h"
#include "bchp_cap_0.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_buffer_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * Private defines
 ***************************************************************************/
/* 3560, 7401A, 7400A:
 * no BVB_IN_SIZE. PIC_OFFSET seperate from PIC_SIZE -> RX_CTRL block
 */
#define BVDC_P_CAP_VER_0                      (0)
/* 7401B, 7400B, 7403, 7118, 7440:
 * PIC_OFFSET -> BVB_IN_SIZE seperate from PIC_SIZE -> RX_CTRL block
 */
#define BVDC_P_CAP_VER_1                      (1)
/* 3563, 7405:
 * PIC_OFFSET and BVB_IN_SIZE in block: PIC_SIZE -> LINE_CMP_TRIG_1_CFG
 */
#define BVDC_P_CAP_VER_2                      (2)
/* 3548:
 * Dither support added.
 */
#define BVDC_P_CAP_VER_3                      (3)
/* 7422:
 * 3D support added.
 */
#define BVDC_P_CAP_VER_4                      (4)
/* 7425Bx, 7344Bx, 7231Bx, 7346Bx:
 * PIC_OFFSET_R added.
 */
#define BVDC_P_CAP_VER_5                      (5)
/* 7435:
 * CAP_x_CTRL.ENABLE_CTRL added.
 */
#define BVDC_P_CAP_VER_6                      (6)

/* 7366B 7364A:
 * CAP_x_DCEM_RECT_SIZE[0..15]added.
 */
#define BVDC_P_CAP_VER_7                      (7)

/* 7271A:
 * New setting for CAP_0_PITCH in mosaic mode
 */
#define BVDC_P_CAP_VER_8                      (8)

/* New pitch setting for mosaic */
#define BVDC_P_CAP_SUPPORT_LPDDR4_MEMORY_PITCH           \
	(BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_8)

/***************************************************************************
 * Private register cracking macros
 ***************************************************************************/
#define BVDC_P_CAP_GET_REG_IDX(reg) \
	((BCHP##_##reg - BCHP_CAP_0_REG_START) / sizeof(uint32_t))

/* Get/Set reg data */
#define BVDC_P_CAP_GET_REG_DATA(reg) \
	(hCapture->aulRegs[BVDC_P_CAP_GET_REG_IDX(reg)])
#define BVDC_P_CAP_SET_REG_DATA(reg, data) \
	(BVDC_P_CAP_GET_REG_DATA(reg) = (uint32_t)(data))

#define BVDC_P_CAP_GET_REG_DATA_I(idx, reg) \
	(hCapture->aulRegs[BVDC_P_CAP_GET_REG_IDX(reg) + (idx)])

/* Get field */
#define BVDC_P_CAP_GET_FIELD_NAME(reg, field) \
	(BVDC_P_GET_FIELD(BVDC_P_CAP_GET_REG_DATA(reg), reg, field))

/* Compare field */
#define BVDC_P_CAP_COMPARE_FIELD_DATA(reg, field, data) \
	(BVDC_P_COMPARE_FIELD_DATA(BVDC_P_CAP_GET_REG_DATA(reg), reg, field, (data)))
#define BVDC_P_CAP_COMPARE_FIELD_NAME(reg, field, name) \
	(BVDC_P_COMPARE_FIELD_NAME(BVDC_P_CAP_GET_REG_DATA(reg), reg, field, name))

/* This macro does a write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_CAP_WRITE_TO_RUL(reg, addr_ptr) \
{ \
	*addr_ptr++ = BRDC_OP_IMM_TO_REG(); \
	*addr_ptr++ = BRDC_REGISTER(BCHP##_##reg + hCapture->ulRegOffset); \
	*addr_ptr++ = BVDC_P_CAP_GET_REG_DATA(reg); \
}

/* This macro does a block write into RUL */
#define BVDC_P_CAP_BLOCK_WRITE_TO_RUL(from, to, pulCurrent) \
do { \
	uint32_t ulBlockSize = \
		BVDC_P_REGS_ENTRIES(from, to);\
	*pulCurrent++ = BRDC_OP_IMMS_TO_REGS( ulBlockSize ); \
	*pulCurrent++ = BRDC_REGISTER(BCHP##_##from + hCapture->ulRegOffset); \
	BKNI_Memcpy((void*)pulCurrent, \
		(void*)&(hCapture->aulRegs[BVDC_P_CAP_GET_REG_IDX(from)]), \
		ulBlockSize * sizeof(uint32_t)); \
	pulCurrent += ulBlockSize; \
} while(0)

/* This macro does a block write into RUL */
#define BVDC_P_CAP_RECT_BLOCK_WRITE_TO_RUL(from, cnt, pulCurrent) \
do { \
	*pulCurrent++ = BRDC_OP_IMMS_TO_REGS( cnt ); \
	*pulCurrent++ = BRDC_REGISTER(BCHP##_##from + hCapture->ulRegOffset); \
	BKNI_Memcpy((void*)pulCurrent, \
		(void*)&(hCapture->aulRegs[BVDC_P_CAP_GET_REG_IDX(from)]), \
		(cnt) * sizeof(uint32_t)); \
	pulCurrent += cnt; \
} while(0)

/* number of registers in one block. */
#define BVDC_P_CAP_REGS_COUNT      \
	BVDC_P_REGS_ENTRIES(CAP_0_REG_START, CAP_0_REG_END)

#define BVDC_P_Capture_MuxAddr(hCap)   (BCHP_VNET_B_CAP_0_SRC + (hCap)->eId * sizeof(uint32_t))
#define BVDC_P_Capture_SetVnet_isr(hCap, ulSrcMuxValue, eVnetPatchMode) \
   BVDC_P_SubRul_SetVnet_isr(&((hCap)->SubRul), ulSrcMuxValue, eVnetPatchMode)
#define BVDC_P_Capture_UnsetVnet_isr(hCap) \
   BVDC_P_SubRul_UnsetVnet_isr(&((hCap)->SubRul))

/***************************************************************************
 * Capture private data sturctures
 ***************************************************************************/
typedef enum BVDC_P_Capture_Trigger
{
	BVDC_P_Capture_Trigger_eDisabled = 0,
	BVDC_P_Capture_Trigger_eBvb,
	BVDC_P_Capture_Trigger_eLineCmp = 2
} BVDC_P_Capture_Trigger;

/* BVN path capture data mode */
typedef enum BVDC_P_Capture_DataMode
{
	BVDC_P_Capture_DataMode_e8Bit422 = 0,
	BVDC_P_Capture_DataMode_e10Bit422,
	BVDC_P_Capture_DataMode_e10Bit444,

	BVDC_P_Capture_DataMode_eMaxCount

} BVDC_P_Capture_DataMode;


typedef struct BVDC_P_CaptureContext
{
	BDBG_OBJECT(BVDC_CAP)

	/* Window associated with this capture.
	 * note: one capture can not be shared by more than one window */
	BVDC_Window_Handle             hWindow;

	/* flag initial state, requires reset; */
	bool                           bInitial;
	uint32_t                       ulResetRegAddr;
	uint32_t                       ulResetMask;

	/* private fields. */
	BVDC_P_CaptureId               eId;
	BRDC_Trigger                   eTrig;
	uint32_t                       ulRegOffset; /* CAP_0, CAP_1, and etc. */
	uint32_t                       aulRegs[BVDC_P_CAP_REGS_COUNT];

	/* A register handle.  Triggers need to be enable by host writes.
	 * A memory handle to do address/offset converting. */
	BREG_Handle                    hRegister;
	BRDC_Handle 				   hRdc;

	/* Keeps track of when ISR executed */
	uint32_t                       ulTimestamp;

#if (!BVDC_P_USE_RDC_TIMESTAMP)
	BTMR_TimerHandle               hTimer;
	BTMR_TimerRegisters            stTimerReg;
	/* a capture block's scratch register */
	uint32_t                       ulTimestampRegAddr;
#endif

	/* sub-struct to manage vnet and rul build opreations */
	BVDC_P_SubRulContext           SubRul;

	BVDC_444To422DnSampler         stDnSampler;

	/* Data mode */
	BVDC_P_Capture_DataMode        eCapDataMode;
	bool                           bEnableDcxm;

} BVDC_P_CaptureContext;


/***************************************************************************
 * Capture private functions
 ***************************************************************************/
BERR_Code BVDC_P_Capture_Create
	( BVDC_P_Capture_Handle           *phCapture,
	  BRDC_Handle					   hRdc,
	  BREG_Handle                      hRegister,
	  BVDC_P_CaptureId                 eCaptureId,
#if (!BVDC_P_USE_RDC_TIMESTAMP)
	  BTMR_TimerHandle                 hTimer,
#endif
	  BVDC_P_Resource_Handle           hResource );

void BVDC_P_Capture_Destroy
	( BVDC_P_Capture_Handle            hCapture );

void BVDC_P_Capture_Init
	( BVDC_P_Capture_Handle            hCapture,
	  BVDC_Window_Handle               hWindow );

void BVDC_P_Capture_BuildRul_isr
	( const BVDC_P_Capture_Handle      hCapture,
	  BVDC_P_ListInfo                 *pList,
	  BVDC_P_State                     eVnetState,
	  const BVDC_P_PictureNodePtr      pPicture);

BERR_Code BVDC_P_Capture_SetBuffer_isr
	( BVDC_P_Capture_Handle            hCapture,
	  uint32_t                         ulDeviceAddr,
	  uint32_t                         ulDeviceAddr_R,
	  uint32_t                         ulPitch );

BERR_Code BVDC_P_Capture_SetEnable_isr
	( BVDC_P_Capture_Handle            hCapture,
	  bool                             bEnable );

BERR_Code BVDC_P_Capture_SetInfo_isr
	( BVDC_P_Capture_Handle            hCapture,
	  BVDC_Window_Handle               hWindow,
	  const BVDC_P_PictureNodePtr      pPicture,
	  uint32_t                         ulPictureIdx,
	  bool                             bLastPic );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_CAPTURE_PRIV_H__ */
/* End of file. */
