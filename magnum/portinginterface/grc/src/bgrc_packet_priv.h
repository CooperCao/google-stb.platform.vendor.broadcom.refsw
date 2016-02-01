/***************************************************************************
 *     Copyright (c) 2009-2014, Broadcom Corporation
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
 * Module Description: GRC Packet private header
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#ifndef BGRC_PACKET_PRIV_H__
#define BGRC_PACKET_PRIV_H__

#include "bdbg.h"
#include "bchp.h"
#include "bmem.h"
#include "bsur.h"
#include "bint.h"
#include "bkni.h"
#include "breg_mem.h"
#include "bgrc_packet.h"
#include "bgrc_errors.h"
#include "blst_list.h"

#include "bchp_common.h"
#include "bchp_m2mc.h"
#ifdef BCHP_M2MC1_REG_START
#include "bchp_m2mc1.h"
#endif
#ifdef BCHP_M2MC_1_REG_START
#include "bchp_m2mc_1.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
Basic concept and handling:

BGRC:  represent a HW m2mc engine
context:  represent a software virtual engine.

software packet fifo:  belongs to a context
hardware packet fifo:  belongs to a BGRC

software packet:  app sets to context.
	app sets software packets to a context of a BGRC. They are stored in the
	software packet fifo of the context.

hardware packet:  BGRC sets to HW to execute.
	BGRC switch among BGRC's contexts, to process the software packets of each
	context, to write resulting setting into the context's shadow registers,
	and to write HW packets into the the BGRC's hardware packet fifo according
	to header masks and shadow registers. A HW packet is created whenever a sw
	packet header->execute is on && header->type is a blit type.

BGRC_PACKET_P_ProcessSwPkt
	process one software packet of the context and write setting to the context's
	shadow registers

BGRC_PACKET_P_ProcessSwPktFifo
	implement a loop to call BGRC_PACKET_P_ProcessSwPkt to process each software
	packet in the context's software packet fifo. When a fill/blit software packet
	is processed, it will write a HW packet according to header masks and shadow
	registers, it will also link the current HW packet to previous HW packet.

*/

/***************************************************************************/
#ifndef BDBG_OBJECT_ID
#define BDBG_OBJECT(P0)
#define BDBG_OBJECT_SET(P0,P1)
#define BDBG_OBJECT_ASSERT(P0,P1)
#define BDBG_OBJECT_DESTROY(P0,P1)
#define BDBG_OBJECT_ID(P0)         extern const char bgrc_id__##P0
#define BDBG_OBJECT_ID_DECLARE(P0) extern const char bgrc_id_declare_##P0
#endif

/***************************************************************************/
BDBG_OBJECT_ID_DECLARE(BGRC_PacketContext);

/***************************************************************************/
/*#define BGRC_P_CHECK_RE_ENTRY 1*/

/*#define BGRC_PACKET_P_DEBUG_SW_PKT  1*/ /* print sw pkt name */
/*#define BGRC_PACKET_P_DEBUG_DESC 1*/    /* print hw pkt */
/*#define BGRC_PACKET_P_VERIFY_SURFACE_RECTANGLE*/
/*#define BGRC_PACKET_P_DEBUG_SWPKT_FIFO  1*/
/*#define BGRC_PACKET_P_DEBUG_EXTRA_FLUSH  1*/
/*#define BGRC_PACKET_P_DEBUG_FORCEDSTDISABLE 1*/
#define BGRC_PACKET_P_DEBUG_WATCHDOG     1
#define BGRC_PACKET_P_WORK_AROUND_DCEG_HW_ISSUE   1

/***************************************************************************/
/* checkpoint_stress shows 7563, ca
   che_flush_stress shows 7445 need CHECK_SYNC_BLIT_PXL.
 * Note: When M2MC HW reports that blits are completed, it currently only means M2MC has outputted
 * the last pixel, indeed this last pixel could still be any where in the bus/memory control
 * system and not in the memory yet.  So for all old/current chips we still needs a sync blit
 * appended after every checkpoint (i.e. sync), so that BGRC can check the output pixel value
 * to ensure the last output is in memeory */
#define BGRC_P_EXTRA_FLUSH_THRESH                  (10) /*(0x70000000)*/
#define BGRC_PACKET_FLUSH_BLIT_WIDTH               (4)

/***************************************************************************/
#define BGRC_PACKET_P_BLIT_GROUP_SIZE_MAX (1024*16)

/***************************************************************************/
#define BGRC_PACKET_P_MEMORY_ALIGN_BITS   5
#define BGRC_PACKET_P_MEMORY_ALIGN_MASK   ((1 << BGRC_PACKET_P_MEMORY_ALIGN_BITS) - 1)

#define BGRC_PACKET_P_ALIGN_HW_PKT( addr ) \
	(uint8_t *)(((uintptr_t)(addr) + BGRC_PACKET_P_MEMORY_ALIGN_MASK) & (~BGRC_PACKET_P_MEMORY_ALIGN_MASK))

/***************************************************************************/
#ifdef BCHP_M2MC0_REVISION
#define BGRC_M2MC(val)   BCHP_M2MC_##val
#else
#define BGRC_M2MC(val)   BCHP_M2MC_##val
#endif

/***************************************************************************/
#define BGRC_PACKET_P_REGISTER_COUNT \
	((BGRC_M2MC(SRC_CLUT_ENTRY_i_ARRAY_BASE) - BGRC_M2MC(SRC_FEEDER_ENABLE)) / 4 + 1)
#define BGRC_PACKET_P_GET_STORED_REG_INDEX( reg ) \
	((BGRC_M2MC(reg) - BGRC_M2MC(SRC_FEEDER_ENABLE)) / sizeof (uint32_t))

/***************************************************************************/
#ifdef BGRC_PACKET_P_DEBUG_SW_PKT
#define BGRC_PACKET_P_DEBUG_PRINT_CTX( value ) BKNI_Printf( "ctx %d: %s", hContext->ulId, value )
#define BGRC_PACKET_P_DEBUG_PRINT( value ) BKNI_Printf( "%s", value )
#define BGRC_PACKET_P_DEBUG_PRINT_VALUE( value ) BKNI_Printf( "%08x ", value )
#else
#define BGRC_PACKET_P_DEBUG_PRINT_CTX( value )
#define BGRC_PACKET_P_DEBUG_PRINT( value )
#define BGRC_PACKET_P_DEBUG_PRINT_VALUE( value )
#endif

#if defined(BGRC_PACKET_P_DEBUG_DESC)
#define BGRC_PACKET_P_PRINT_DESC( value ) BKNI_Printf( "\n%s ", value )
#define BGRC_PACKET_P_PRINT_DESC_VALUE( value ) BKNI_Printf( "%08x ", value )
#else
#define BGRC_PACKET_P_PRINT_DESC( value )
#define BGRC_PACKET_P_PRINT_DESC_VALUE( value )
#endif

/* assume data is already shifted to field */
#define BGRC_P_COMPARE_FIELD_DATA(reg_value, reg, field, data) \
	(((reg_value) & BCHP_M2MC_##reg##_##field##_MASK) == (data))

#define BGRC_P_COMPARE_FIELD_NAME(reg_value, reg, field, name) \
	(((reg_value) & BCHP_M2MC_##reg##_##field##_MASK) == (uint32_t)(BCHP_M2MC_##reg##_##field##_##name << BCHP_M2MC_##reg##_##field##_SHIFT))
#define BGRC_P_WRITE_FIELD_NAME(reg_value, reg, field, name) \
	(reg_value) = ((reg_value) & ~ BCHP_M2MC_##reg##_##field##_MASK) | (BCHP_M2MC_##reg##_##field##_##name << BCHP_M2MC_##reg##_##field##_SHIFT)

#define BGRC_PACKET_P_COMPARE_FIELD_DATA(reg, field, data) \
	(BGRC_P_COMPARE_FIELD_DATA(hContext->stored_registers[BGRC_PACKET_P_GET_STORED_REG_INDEX(reg)], reg, field, (data)))

#ifdef BGRC_PACKET_P_DEBUG_SW_PKT__
#define BGRC_PACKET_P_STORE_REG( reg, value ) { BKNI_Printf("%08x ", (value)); hContext->stored_registers[BGRC_PACKET_P_GET_STORED_REG_INDEX(reg)] = (value); }
#else
#define BGRC_PACKET_P_STORE_REG( reg, value ) hContext->stored_registers[BGRC_PACKET_P_GET_STORED_REG_INDEX(reg)] = (value)
#endif

#define BGRC_PACKET_P_STORE_REG_MASK( reg, value, mask ) BGRC_PACKET_P_STORE_REG( reg, (BGRC_PACKET_P_GET_REG(reg) & (mask)) | (value) )

#define BGRC_PACKET_P_STORE_REG_FIELD( reg, field, value ) \
{ \
	hContext->stored_registers[BGRC_PACKET_P_GET_STORED_REG_INDEX(reg)] &= ~BGRC_M2MC(reg##_##field##_MASK); \
	hContext->stored_registers[BGRC_PACKET_P_GET_STORED_REG_INDEX(reg)] |= BGRC_M2MC(reg##_##field##_##value) << BGRC_M2MC(reg##_##field##_SHIFT); \
}

#define BGRC_PACKET_P_STORE_REG_FIELD_DATA( reg, field, value ) \
{ \
	hContext->stored_registers[BGRC_PACKET_P_GET_STORED_REG_INDEX(reg)] &= ~BGRC_M2MC(reg##_##field##_MASK); \
	hContext->stored_registers[BGRC_PACKET_P_GET_STORED_REG_INDEX(reg)] |= (value) << BGRC_M2MC(reg##_##field##_SHIFT); \
}

#define BGRC_PACKET_P_STORE_REG_FIELD_COMP( reg, field, value_false, value_true, comp ) \
{ \
	if( comp ) \
		BGRC_PACKET_P_STORE_REG_FIELD(reg, field, value_true) \
	else \
		BGRC_PACKET_P_STORE_REG_FIELD(reg, field, value_false) \
}

#if defined(BCHP_M2MC_BLIT_SRC_TOP_LEFT_1) || defined(BCHP_M2MC0_BLIT_SRC_TOP_LEFT_1)
#define BGRC_PACKET_P_STORE_RECT_REGS( src_pos0, src_size0, src_pos1, src_size1, dst_pos, dst_size, out_pos, out_size ) \
{ \
	BGRC_PACKET_P_STORE_REG( BLIT_SRC_TOP_LEFT, src_pos0 ); \
	BGRC_PACKET_P_STORE_REG( BLIT_SRC_SIZE, src_size0 ); \
	BGRC_PACKET_P_STORE_REG( BLIT_SRC_TOP_LEFT_1, src_pos1 ); \
	BGRC_PACKET_P_STORE_REG( BLIT_SRC_SIZE_1, src_size1 ); \
	BGRC_PACKET_P_STORE_REG( BLIT_DEST_TOP_LEFT, dst_pos ); \
	BGRC_PACKET_P_STORE_REG( BLIT_DEST_SIZE, dst_size ); \
	BGRC_PACKET_P_STORE_REG( BLIT_OUTPUT_TOP_LEFT, out_pos ); \
	BGRC_PACKET_P_STORE_REG( BLIT_OUTPUT_SIZE, out_size ); \
}
#else
#define BGRC_PACKET_P_STORE_RECT_REGS( src_pos0, src_size0, src_pos1, src_size1, dst_pos, dst_size, out_pos, out_size ) \
{ \
	BGRC_PACKET_P_STORE_REG( BLIT_SRC_TOP_LEFT, src_pos0 ); \
	BGRC_PACKET_P_STORE_REG( BLIT_SRC_SIZE, src_size0 ); \
	BGRC_PACKET_P_STORE_REG( BLIT_DEST_TOP_LEFT, dst_pos ); \
	BGRC_PACKET_P_STORE_REG( BLIT_DEST_SIZE, dst_size ); \
	BGRC_PACKET_P_STORE_REG( BLIT_OUTPUT_TOP_LEFT, out_pos ); \
	BGRC_PACKET_P_STORE_REG( BLIT_OUTPUT_SIZE, out_size ); \
}
#endif

#if defined(BCHP_M2MC_HORIZ_FIR_1_COEFF_PHASE0_01) || defined(BCHP_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_01)
#define BGRC_PACKET_P_STORE_SCALE_REGS( reg, num, phase, step ) \
{ \
	BGRC_PACKET_P_STORE_REG( reg##_SCALER_##num##_INITIAL_PHASE, (phase) & BGRC_M2MC(reg##_SCALER_##num##_INITIAL_PHASE_PHASE_MASK) ); \
	BGRC_PACKET_P_STORE_REG( reg##_SCALER_##num##_STEP, (step) & BGRC_M2MC(reg##_SCALER_##num##_STEP_STEP_MASK) ); \
}
#else
#define BGRC_PACKET_P_STORE_SCALE_REGS( reg, num, phase, step ) \
{ \
	BGRC_PACKET_P_STORE_REG( reg##_SCALER_INITIAL_PHASE, (phase) & BGRC_M2MC(reg##_SCALER_INITIAL_PHASE_PHASE_MASK) ); \
	BGRC_PACKET_P_STORE_REG( reg##_SCALER_STEP, (step) & BGRC_M2MC(reg##_SCALER_STEP_STEP_MASK) ); \
}
#endif

/***************************************************************************/
#ifdef BGRC_PACKET_P_DEBUG_SW_PKT__
#define BGRC_PACKET_P_WRITE_REG( dst, index ) { BKNI_Printf("%08x ", hContext->stored_registers[index]); dst = hContext->stored_registers[index]; }
#define BGRC_PACKET_P_WRITE_DATA( dst, data ) { BKNI_Printf("%08x ", (data)); dst = data; }
#else
#define BGRC_PACKET_P_WRITE_REG( dst, index ) dst = hContext->stored_registers[index]
#define BGRC_PACKET_P_WRITE_DATA( dst, data ) dst = data
#endif

#define BGRC_PACKET_P_GET_REG_FROM_INDEX( index )       hContext->stored_registers[index]
#define BGRC_PACKET_P_GET_REG( reg )                    BGRC_PACKET_P_GET_REG_FROM_INDEX(BGRC_PACKET_P_GET_STORED_REG_INDEX(reg))

#ifdef BCHP_M2MC0_REVISION
#define BGRC_PACKET_P_GET_REG_FIELD( reg, field )       (BGRC_PACKET_P_GET_REG(reg) & BCHP_M2MC0_##reg##_##field##_MASK)
#define BGRC_PACKET_P_GET_REG_FIELD_SHIFT( reg, field ) ((BGRC_PACKET_P_GET_REG(reg) & BCHP_M2MC0_##reg##_##field##_MASK) >> BCHP_M2MC0_##reg##_##field##_SHIFT)
#else
#define BGRC_PACKET_P_GET_REG_FIELD( reg, field )       (BGRC_PACKET_P_GET_REG(reg) & BCHP_M2MC_##reg##_##field##_MASK)
#define BGRC_PACKET_P_GET_REG_FIELD_SHIFT( reg, field ) ((BGRC_PACKET_P_GET_REG(reg) & BCHP_M2MC_##reg##_##field##_MASK) >> BCHP_M2MC_##reg##_##field##_SHIFT)
#endif

/***************************************************************************/
#if BGRC_PACKET_P_DEBUG_SWPKT_FIFO
#define BGRC_P_SWPKT_MSG    BDBG_ERR
#else
#define BGRC_P_SWPKT_MSG(a)
#endif
#if BGRC_PACKET_P_DEBUG_WATCHDOG
#define BGRC_P_WATCHDOG_MSG    BDBG_WRN
#else
#define BGRC_P_WATCHDOG_MSG(a)
#endif
#if BGRC_PACKET_P_DEBUG_FORCEDSTDISABLE
#define BGRC_P_FORCEDSTDISABLE_MSG    BDBG_ERR
#else
#define BGRC_P_FORCEDSTDISABLE_MSG(a)
#endif

/***************************************************************************/
#ifdef BCHP_M2MC1_REVISION
#define BGRC_P_WriteReg32( handle, reg, data ) BREG_Write32( handle, BCHP_M2MC_##reg + hGrc->ulDeviceNum * (BCHP_M2MC1_REVISION - BCHP_M2MC_REVISION), data )
#define BGRC_P_ReadReg32( handle, reg )        BREG_Read32( handle, BCHP_M2MC_##reg + hGrc->ulDeviceNum * (BCHP_M2MC1_REVISION - BCHP_M2MC_REVISION) )
#else
#ifdef BCHP_M2MC_1_REVISION
#define BGRC_P_WriteReg32( handle, reg, data ) BREG_Write32( handle, BCHP_M2MC_##reg + hGrc->ulDeviceNum * (BCHP_M2MC_1_REVISION - BCHP_M2MC_REVISION), data )
#define BGRC_P_ReadReg32( handle, reg )        BREG_Read32( handle, BCHP_M2MC_##reg + hGrc->ulDeviceNum * (BCHP_M2MC_1_REVISION - BCHP_M2MC_REVISION) )
#else
#define BGRC_P_WriteReg32( handle, reg, data ) BREG_Write32( handle, BCHP_M2MC_##reg, data )
#define BGRC_P_ReadReg32( handle, reg )        BREG_Read32( handle, BCHP_M2MC_##reg )
#endif
#endif

/***************************************************************************/
#define BGRC_P_MIN_WIDTH_WITH_VERT_SCALE  4

#define BGRC_PACKET_P_IS_420(type) \
   (((type) >= M2MC_FT_YCbCr420) && ((type) != M2MC_FT_YCbCr444_10) && ((type) != M2MC_FT_CompressedARGB8888))
#define BGRC_PACKET_P_IS_422_OR_420(type) \
   (((type) >= M2MC_FT_YCbCr422) && ((type) != M2MC_FT_Alpha) && ((type) != M2MC_FT_YCbCr444_10) && ((type) != M2MC_FT_CompressedARGB8888))

/* don't read or write outside */
#define BGRC_PACKET_P_PXL_ALIGN(type, x, w, y, h) \
	if (BGRC_PACKET_P_IS_422_OR_420(type)) {\
		uint32_t e; \
		if ((x) & 1) BDBG_MSG(("Align 442/420 left point %d -> %d", (x), (x) + 1)); \
		if (((x) + (w)) & 1) BDBG_MSG(("Align 442/420 right end point %d -> %d", (x) + (w), (x) + (w) - 1)); \
		e = ((x) + (w)) & (~0x1); (x) = ((x) + 1) & (~0x1); (w) = e - (x); \
		if (BGRC_PACKET_P_IS_420(type)) {\
		    if ((y) & 1) BDBG_MSG(("Align 420 top point %d -> %d", (y), (y) + 1)); \
		    if (((y) + (h)) & 1) BDBG_MSG(("Align 420 bottom end point %d -> %d", (y) + (h), (y) + (h) - 1)); \
			e = ((y) + (h)) & (~0x1); (y) = ((y) + 1) & (~0x1); (h) = e - (y); \
		}\
	} \
	if (((w)==0) || ((h)==0)) {\
		BDBG_MSG(("Dropped 0 size Blit %d x %d", (w), (h))); \
		hContext->bBlitInvalid = true; \
	}

#define BGRC_PACKET_P_SRC_PXL_ALIGN(x, w, y, h) \
{\
	uint32_t formatType = BGRC_PACKET_P_GET_REG_FIELD(SRC_SURFACE_0_FORMAT_DEF_1, FORMAT_TYPE);\
	BGRC_PACKET_P_PXL_ALIGN(formatType, (x), (w), (y), (h)) \
}

#define BGRC_PACKET_P_DST_PXL_ALIGN(x, w, y, h) \
{\
	uint32_t formatType = BGRC_PACKET_P_GET_REG_FIELD(DEST_SURFACE_FORMAT_DEF_1, FORMAT_TYPE);\
	BGRC_PACKET_P_PXL_ALIGN(formatType, (x), (w), (y), (h)) \
}

#define BGRC_PACKET_P_OUT_PXL_ALIGN(x, w, y, h) \
{\
	uint32_t formatType = BGRC_PACKET_P_GET_REG_FIELD(OUTPUT_SURFACE_FORMAT_DEF_1, FORMAT_TYPE);\
	BGRC_PACKET_P_PXL_ALIGN(formatType, (x), (w), (y), (h)) \
}

#define BGRC_PACKET_P_SRC_OUT_PXL_ALIGN(sx, sw, ox, sy, sh, oy) \
{\
	uint32_t e, w = (sw), h = (sh); \
	uint32_t srcType = BGRC_PACKET_P_GET_REG_FIELD(SRC_SURFACE_0_FORMAT_DEF_1, FORMAT_TYPE);\
	uint32_t outType = BGRC_PACKET_P_GET_REG_FIELD(OUTPUT_SURFACE_FORMAT_DEF_1, FORMAT_TYPE);\
	if (BGRC_PACKET_P_IS_422_OR_420(srcType)) {\
		if ((sx) & 1) BDBG_MSG(("Align 442/420 left point %d -> %d", (sx), (sx) + 1)); \
		if (((sx) + (w)) & 1) BDBG_MSG(("Align 442/420 right end point %d -> %d", (sx) + (w), (sx) + (w) - 1)); \
		e = ((sx) + w) & (~0x1); (sx) = ((sx) + 1) & (~0x1); (sw) = e - (sx); \
		if (BGRC_PACKET_P_IS_420(srcType)) {\
		    if ((sy) & 1) BDBG_MSG(("Align 420 top point %d -> %d", (sy), (sy) + 1)); \
		    if (((sy) + (h)) & 1) BDBG_MSG(("Align 420 bottom end point %d -> %d", (sy) + (h), (sy) + (h) - 1)); \
			e = ((sy) + h) & (~0x1); (sy) = ((sy) + 1) & (~0x1); (sh) = e - (sy); }\
	}\
	if (BGRC_PACKET_P_IS_422_OR_420(outType)) {\
		e = ((ox) + w) & (~0x1); (ox) = ((ox) + 1) & (~0x1); (sw) = BGRC_P_MIN((sw), e - (ox)); \
		if (BGRC_PACKET_P_IS_420(outType)) {\
			e = ((oy) + h) & (~0x1); (oy) = ((oy) + 1) & (~0x1); (sh) = BGRC_P_MIN((sh), e - (oy)); }\
	}\
	if (((sw)==0) || ((sh)==0)) {\
		BDBG_MSG(("Dropped 0 size Blit %d x %d", (sw), (sh))); \
		hContext->bBlitInvalid = true; \
	}\
}

#define BGRC_PACKET_P_DST_OUT_PXL_ALIGN(ox, ow, dx, oy, oh, dy) \
	BGRC_PACKET_P_SRC_OUT_PXL_ALIGN(ox, ow, dx, oy, oh, dy)

#define BGRC_PACKET_P_SRC_DST_OUT_PXL_ALIGN(sx, sw, dx, ox, sy, sh, dy, oy) \
{\
	uint32_t e, w = (sw), h = (sh); \
	uint32_t srcType = BGRC_PACKET_P_GET_REG_FIELD(SRC_SURFACE_0_FORMAT_DEF_1, FORMAT_TYPE);\
	uint32_t dstType = BGRC_PACKET_P_GET_REG_FIELD(DEST_SURFACE_FORMAT_DEF_1, FORMAT_TYPE);\
	uint32_t outType = BGRC_PACKET_P_GET_REG_FIELD(OUTPUT_SURFACE_FORMAT_DEF_1, FORMAT_TYPE);\
	if (BGRC_PACKET_P_IS_422_OR_420(srcType)) {\
		if ((sx) & 1) BDBG_MSG(("Align 442/420 left point %d -> %d", (sx), (sx) + 1)); \
		if (((sx) + (w)) & 1) BDBG_MSG(("Align 442/420 right end point %d -> %d", (sx) + (w), (sx) + (w) - 1)); \
		e = ((sx) + w) & (~0x1); (sx) = ((sx) + 1) & (~0x1); (sw) = e - (sx); \
		if (BGRC_PACKET_P_IS_420(srcType)) {\
		    if ((sy) & 1) BDBG_MSG(("Align 420 top point %d -> %d", (sy), (sy) + 1)); \
		    if (((sy) + (h)) & 1) BDBG_MSG(("Align 420 bottom end point %d -> %d", (sy) + (h), (sy) + (h) - 1)); \
			e = ((sy) + h) & (~0x1); (sy) = ((sy) + 1) & (~0x1); (sh) = e - (sy); }\
	}\
	if (BGRC_PACKET_P_IS_422_OR_420(dstType)) {\
		if ((dx) & 1) BDBG_MSG(("Align 442/420 left point %d -> %d", (dx), (dx) + 1)); \
		if (((dx) + (w)) & 1) BDBG_MSG(("Align 442/420 right end point %d -> %d", (dx) + (w), (dx) + (w) - 1)); \
		e = ((dx) + w) & (~0x1); (dx) = ((dx) + 1) & (~0x1); (sw) = BGRC_P_MIN((sw), e - (dx)); \
		if (BGRC_PACKET_P_IS_420(dstType)) {\
		    if ((dy) & 1) BDBG_MSG(("Align 420 top point %d -> %d", (dy), (dy) + 1)); \
		    if (((dy) + (h)) & 1) BDBG_MSG(("Align 420 bottom end point %d -> %d", (dy) + (h), (dy) + (h) - 1)); \
			e = ((dy) + h) & (~0x1); (dy) = ((dy) + 1) & (~0x1); (sh) = BGRC_P_MIN((sh), e - (dy)); }\
	}\
	if (BGRC_PACKET_P_IS_422_OR_420(outType)) {\
		if ((ox) & 1) BDBG_MSG(("Align 442/420 left point %d -> %d", (ox), (ox) + 1)); \
		if (((ox) + (w)) & 1) BDBG_MSG(("Align 442/420 right end point %d -> %d", (ox) + (w), (ox) + (w) - 1)); \
		e = ((ox) + w) & (~0x1); (ox) = ((ox) + 1) & (~0x1); (sw) = BGRC_P_MIN((sw), e - (ox)); \
		if (BGRC_PACKET_P_IS_420(outType)) {\
		    if ((oy) & 1) BDBG_MSG(("Align 420 top point %d -> %d", (oy), (oy) + 1)); \
		    if (((oy) + (h)) & 1) BDBG_MSG(("Align 420 bottom end point %d -> %d", (oy) + (h), (oy) + (h) - 1)); \
			e = ((oy) + h) & (~0x1); (oy) = ((oy) + 1) & (~0x1); (sh) = BGRC_P_MIN((sh), e - (oy)); }\
	}\
	if (((sw)==0) || ((sh)==0)) {\
		BDBG_MSG(("Dropped 0 size BlendBlit %d x %d", (sw), (sh))); \
		hContext->bBlitInvalid = true; \
	}\
}

/***************************************************************************/
#define BGRC_PACKET_P_DEVICE_GROUP_MASK_FULL ( \
	BCHP_M2MC_LIST_PACKET_HEADER_1_SRC_FEEDER_GRP_CNTRL_MASK | \
	BCHP_M2MC_LIST_PACKET_HEADER_1_DST_FEEDER_GRP_CNTRL_MASK | \
	BCHP_M2MC_LIST_PACKET_HEADER_1_OUTPUT_FEEDER_GRP_CNTRL_MASK | \
	BCHP_M2MC_LIST_PACKET_HEADER_1_BLIT_GRP_CNTRL_MASK | \
	BCHP_M2MC_LIST_PACKET_HEADER_1_SCALE_PARAM_GRP_CNTRL_MASK | \
	BCHP_M2MC_LIST_PACKET_HEADER_1_BLEND_PARAM_GRP_CNTRL_MASK | \
	BCHP_M2MC_LIST_PACKET_HEADER_1_ROP_GRP_CNTRL_MASK | \
	BCHP_M2MC_LIST_PACKET_HEADER_1_SRC_COLOR_KEY_GRP_CNTRL_MASK | \
	BCHP_M2MC_LIST_PACKET_HEADER_1_DST_COLOR_KEY_GRP_CNTRL_MASK | \
	BCHP_M2MC_LIST_PACKET_HEADER_1_SCALE_COEF_GRP_CNTRL_MASK | \
	BCHP_M2MC_LIST_PACKET_HEADER_1_SRC_COLOR_MATRIX_GRP_CNTRL_MASK)
	/*BCHP_M2MC_LIST_PACKET_HEADER_1_SRC_CLUT_GRP_CNTRL_MASK)*/

/***************************************************************************/
#define BGRC_PACKET_P_FLUSH_PACKET_SIZE (\
	sizeof (BM2MC_PACKET_PacketSaveState) + \
	sizeof (BM2MC_PACKET_PacketSourceNone) + \
	sizeof (BM2MC_PACKET_PacketDestinationNone) + \
	sizeof (BM2MC_PACKET_PacketOutputFeeder) + \
	sizeof (BM2MC_PACKET_PacketBlend) + \
	sizeof (BM2MC_PACKET_PacketRop) + \
	sizeof (BM2MC_PACKET_PacketSourceColorkeyEnable) + \
	sizeof (BM2MC_PACKET_PacketDestinationColorkeyEnable) + \
	sizeof (BM2MC_PACKET_PacketFilterEnable) + \
	sizeof (BM2MC_PACKET_PacketFilterControl) + \
	sizeof (BM2MC_PACKET_PacketSourceColorMatrixEnable) + \
	sizeof (BM2MC_PACKET_PacketAlphaPremultiply) + \
	sizeof (BM2MC_PACKET_PacketAlphaDemultiply) + \
	sizeof (BM2MC_PACKET_PacketDestAlphaPremultiply) + \
	sizeof (BM2MC_PACKET_PacketMirror) + \
	sizeof (BM2MC_PACKET_PacketFillBlit) + \
	sizeof (BM2MC_PACKET_PacketRestoreState))

#define BGRC_PACKET_P_EXTRA_PACKET_SIZE \
	(BGRC_PACKET_P_FLUSH_PACKET_SIZE + sizeof (BM2MC_PACKET_Header))

/***************************************************************************/
typedef struct
{
	uint32_t stripe_count;
	uint32_t stripe_num;
	uint32_t output_stripe_width;
	uint32_t input_stripe_width;
	uint32_t hor_chroma_phase; /* TODO: remove */
	uint32_t hor_phase;
	uint32_t hor_step;
	uint32_t ver_chroma_phase; /* TODO: remove */
	uint32_t ver_phase;
	uint32_t ver_step;
	uint16_t stripe_overlap;
}
BGRC_PACKET_P_Scaler;

/***************************************************************************/
typedef struct
{
	uint32_t hor_phase;
	uint32_t src_x;
	uint32_t dst_x;
	uint32_t out_x;
	uint32_t src_width;
	uint32_t out_width;
}
BGRC_PACKET_P_Stripe;

/***************************************************************************/
typedef enum
{
	BGRC_PACKET_P_SyncState_eCleared,
	BGRC_PACKET_P_SyncState_eRequested,
	BGRC_PACKET_P_SyncState_eQueuedInSw,
	BGRC_PACKET_P_SyncState_eQueuedInHw,
	BGRC_PACKET_P_SyncState_eSynced
}
BGRC_PACKET_P_SyncState;


/****************************************************************************
 * Surface invalid bits
 *
 */
typedef union
{
	struct
	{
		uint32_t   bSrc   : 1;  /* 0 */
		uint32_t   bDst   : 1;
		uint32_t   bOut   : 1;
	} stBits;
	uint32_t ulInts;
} BGRC_P_Packet_SurInvalidBits;

/***************************************************************************/
BDBG_OBJECT_ID_DECLARE(BGRC_PacketContext);
typedef struct BGRC_P_PacketContext
{
	BDBG_OBJECT(BGRC_PacketContext)
	BGRC_Handle hGrc;
	BGRC_PacketContext_CreateSettings create_settings;
	BINT_CallbackHandle hCallback;

	BLST_D_ENTRY(BGRC_P_PacketContext) context_link;
	uint32_t ulId;

	/* used to drop following blits */
	BGRC_P_Packet_SurInvalidBits stSurInvalid;
	bool      bBlitInvalid;

	BGRC_PACKET_P_Scaler scaler;
	BM2MC_PACKET_Header *scaler_header;
	BGRC_P_Packet_SurInvalidBits saved_stSurInvalid;
	bool      saved_bBlitInvalid;


	void     *pSwPktFifoBaseAlloc;
	uint8_t  *pSwPktFifoBase;
	uint32_t  ulSwPktFifoSize;

	uint8_t  *pSwPktWritePtr;
	uint8_t  *pSwPktReadPtr;
	uint8_t  *pSwPktWrapPtr; /* at the ptr pSwPktWritePtr wrapped to base */
	uint32_t  ulLockedSwPktBufSize;

	uint32_t  ulSwPktSizeToProc;  /* 0 means no more sw pkts in sw pkt fifo */
	bool      bPktsInPipe;   /* need to report status in next BGRC_Packet_GetContextStatus */

	uint32_t  ulSyncHwPktOffset;
	uint32_t  ulSwPktSizeBeforeSync;
	BGRC_PACKET_P_SyncState eSyncState;
	uint32_t  ulSyncCntr;

	uint8_t  *pLastHwPktPtr;
	uint32_t *pLastBlitHeaderPtr;
	uint32_t  ulGroupMask;

	bool      bNeedDestForBlend;
	bool      bDestForceDisabled; /* force to disable dest if not used anywhere to save bandwidth */
	bool      saved_bNeedDestForBlend;
	bool      saved_bDestForceDisabled;

	bool      bResetState;

	uint8_t last_blit_type;

	uint32_t stored_registers[BGRC_PACKET_P_REGISTER_COUNT];
	uint32_t saved_registers[BGRC_PACKET_P_REGISTER_COUNT];

	BM2MC_PACKET_PacketFixedScale fixed_scale;
	BM2MC_PACKET_PacketFixedScale saved_fixed_scale;
	BM2MC_PACKET_PacketDestripeFixedScale destripe_fixed_scale;
	BM2MC_PACKET_PacketDestripeFixedScale saved_destripe_fixed_scale;
	uint32_t src_format0;
	uint32_t dst_format0;
	uint32_t out_format0;
	uint32_t saved_out_format0;
	uint32_t saved_src_format0;
	bool b420Src;
	bool saved_b420Src;

#if BGRC_PACKET_P_VERIFY_SURFACE_RECTANGLE && BDBG_DEBUG_BUILD
	uint32_t SRC_surface_width;
	uint32_t SRC_surface_height;
	uint32_t SRC_surface_format;
	uint32_t SRC_surface_pitch;
	uint32_t DEST_surface_width;
	uint32_t DEST_surface_height;
	uint32_t DEST_surface_format;
	uint32_t DEST_surface_pitch;
	uint32_t OUTPUT_surface_width;
	uint32_t OUTPUT_surface_height;
	uint32_t OUTPUT_surface_format;
	uint32_t OUTPUT_surface_pitch;

	uint32_t saved_SRC_surface_width;
	uint32_t saved_SRC_surface_height;
	uint32_t saved_SRC_surface_format;
	uint32_t saved_SRC_surface_pitch;
	uint32_t saved_DEST_surface_width;
	uint32_t saved_DEST_surface_height;
	uint32_t saved_DEST_surface_format;
	uint32_t saved_DEST_surface_pitch;
	uint32_t saved_OUTPUT_surface_width;
	uint32_t saved_OUTPUT_surface_height;
	uint32_t saved_OUTPUT_surface_format;
	uint32_t saved_OUTPUT_surface_pitch;
#endif
}
BGRC_P_PacketContext;

/***************************************************************************/
void BGRC_P_CheckTableMismach( void );
void BGRC_PACKET_P_PrintRegisters( BGRC_Handle hGrc );

void BGRC_PACKET_P_Isr( void *pvParam1, int iParam2 );
void BGRC_PACKET_P_ResetState( BGRC_PacketContext_Handle hContext );
void BGRC_PACKET_P_ProcessSwPktFifo( BGRC_Handle hGrc, BGRC_PacketContext_Handle hContext );
void BGRC_PACKET_P_InsertFlushBlit( BGRC_Handle hGrc, BGRC_PacketContext_Handle hContext);
void BGRC_PACKET_P_CheckFlush( BGRC_Handle hGrc );
void BGRC_PACKET_P_WriteHwPkt( BGRC_Handle hGrc, BGRC_PacketContext_Handle hContext, uint32_t ulGroupMask );
void BGRC_P_CheckHwStatus( BGRC_Handle hGrc );

BM2MC_PACKET_PixelFormat BGRC_PACKET_P_ConvertPixelFormat( BPXL_Format format );
void BGRC_PACKET_P_ConvertFilterCoeffs( BM2MC_PACKET_FilterCoeffs *coeffs, BGRC_FilterCoeffs filter, size_t src_size, size_t out_size );
void BGRC_PACKET_P_ConvertColorMatrix( BM2MC_PACKET_ColorMatrix *matrix_out, const int32_t *matrix_in, size_t shift );

/***************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* #ifndef BGRC_PACKET_PRIV_H__ */

/* end of file */
