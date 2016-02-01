/***************************************************************************
 *     Copyright (c) 2010-2011, Broadcom Corporation
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
 * Module Description: GRC Packet Writing header
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#ifndef BGRC_PACKET_WRITE_H__
#define BGRC_PACKET_WRITE_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
BERR_Code BGRC_Packet_InitPacketPlane(BPXL_Plane *pSurface, BM2MC_PACKET_Plane *pPckPlane );

/***************************************************************************/
BERR_Code BGRC_Packet_SetSourcePlanePacket( BGRC_Handle hGrc, void **ppPacket,
	const BM2MC_PACKET_Plane *pSrcPlane0, const BM2MC_PACKET_Plane *pSrcPlane1, uint32_t color );

/***************************************************************************/
BERR_Code BGRC_Packet_SetStripedSourcePlanePacket( BGRC_Handle hGrc, void **ppPacket,
	const BM2MC_PACKET_StripedPlane *pSrcPlane, uint32_t color );

/*****************************************************************************/
BERR_Code BGRC_Packet_SetSourceControl( BGRC_Handle hGrc, void **ppPacket,
	bool zero_pad, bool chroma_filter, bool linear_destripe );

/*****************************************************************************/
BERR_Code BGRC_Packet_SetDestinationPlanePacket( BGRC_Handle hGrc, void **ppPacket,
	const BM2MC_PACKET_Plane *pPlane, uint32_t color );

/*****************************************************************************/
BERR_Code BGRC_Packet_SetDestinationControl( BGRC_Handle hGrc, void **ppPacket,
	bool zero_pad, bool chroma_filter );

/*****************************************************************************/
BERR_Code BGRC_Packet_SetOutputPlanePacket( BGRC_Handle hGrc, void **ppPacket,
	const BM2MC_PACKET_Plane *pPlane );

/*****************************************************************************/
BERR_Code BGRC_Packet_SetOutputControl( BGRC_Handle hGrc, void **ppPacket,
	bool dither, bool chroma_filter );

/*****************************************************************************/
BERR_Code BGRC_Packet_SetOutputCompressionPacket( BGRC_Handle hGrc, void **ppPacket,
	unsigned outputCompressionInfo );

/***************************************************************************/
BERR_Code BGRC_Packet_SetBlendPacket( BGRC_Handle hGrc, void **ppPacket,
	const BM2MC_PACKET_Blend *pColor, const BM2MC_PACKET_Blend *pAlpha, uint32_t color );

/***************************************************************************/
BERR_Code BGRC_Packet_SetRopPacket( BGRC_Handle hGrc, void **ppPacket,
	uint8_t rop, uint32_t* pattern, uint32_t color0, uint32_t color1 );

/***************************************************************************/
BERR_Code BGRC_Packet_SetSourceColorkeyPacket( BGRC_Handle hGrc, void **ppPacket,
	bool enable, uint32_t high, uint32_t low, uint32_t mask,
	uint32_t replacement, uint32_t replacement_mask );

/***************************************************************************/
BERR_Code BGRC_Packet_SetFilterControlPacket( BGRC_Handle hGrc, void **ppPacket,
	bool round, bool adjust_color, bool sub_alpha, BM2MC_PACKET_eFilterOrder sourceFilterOrder );

/***************************************************************************/
BERR_Code BGRC_Packet_SetDestinationColorkeyPacket( BGRC_Handle hGrc, void **ppPacket,
	bool enable, uint32_t high, uint32_t low, uint32_t mask,
	uint32_t replacement, uint32_t replacement_mask );

/***************************************************************************/
BERR_Code BGRC_Packet_SetFilterPacket( BGRC_Handle hGrc, void **ppPacket,
	BGRC_FilterCoeffs hor, BGRC_FilterCoeffs ver,
	BM2MC_PACKET_Rectangle *pSrcRect, BM2MC_PACKET_Rectangle *pOutRect );

/***************************************************************************/
BERR_Code BGRC_Packet_SetColorMatrixPacket( BGRC_Handle hGrc, void **ppPacket,
	const int32_t matrix[], uint32_t shift );

BERR_Code BGRC_Packet_SetSourcePalette( BGRC_Handle hGrc, void **ppPacket,
	uint64_t paletteAddress );

/*****************************************************************************/
BERR_Code BGRC_Packet_SetAlphaPremultiply( BGRC_Handle hGrc, void **ppPacket, bool enable );

/***************************************************************************/
BERR_Code BGRC_Packet_SetMirrorPacket( BGRC_Handle hGrc, void **ppPacket,
	bool srcHor, bool srcVer, bool dstHor, bool dstVer, bool outHor, bool outVer );

/***************************************************************************
 * Deprecated.
 */
BERR_Code BGRC_Packet_SetFixedScalePacket( BGRC_Handle hGrc, void **ppPacket,
	int32_t hor_phase, int32_t ver_phase, uint32_t hor_step, uint32_t ver_step, uint32_t shift );

/***************************************************************************
 * Deprecated.
 */
BERR_Code BGRC_Packet_SetDestripeFixedScalePacket( BGRC_Handle hGrc, void **ppPacket,
	BM2MC_PACKET_Rectangle *pChromaRect, int32_t hor_luma_phase, int32_t ver_luma_phase, int32_t hor_chroma_phase, int32_t ver_chroma_phase,
	uint32_t hor_luma_step, uint32_t ver_luma_step, uint32_t shift );

/***************************************************************************/
BERR_Code BGRC_Packet_SetBlitPacket( BGRC_Handle hGrc, void **ppPacket,
	BM2MC_PACKET_Rectangle *pSrcRect, BM2MC_PACKET_Point *pOutPoint, BM2MC_PACKET_Point *pDstPoint );

/***************************************************************************/
BERR_Code BGRC_Packet_SetScaleBlitPacket( BGRC_Handle hGrc, void **ppPacket,
	BM2MC_PACKET_Rectangle *pSrcRect, BM2MC_PACKET_Rectangle *pOutRect, BM2MC_PACKET_Point *pDstPoint );

/*****************************************************************************/
BERR_Code BGRC_Packet_SetDestripeBlitPacket( BGRC_Handle hGrc,  void **ppPacket,
	BM2MC_PACKET_Rectangle *pSrcRect, BM2MC_PACKET_Rectangle *pOutRect,
	uint32_t stripeWidth, uint32_t lumaStripeHeight, uint32_t chromaStripeHeight );

/*****************************************************************************/
BERR_Code BGRC_Packet_SetResetState( BGRC_Handle hGrc, void **ppPacket );

/*****************************************************************************/
BERR_Code BGRC_Packet_SetSaveState( BGRC_Handle hGrc, void **ppPacket );

/*****************************************************************************/
BERR_Code BGRC_Packet_SetRestoreState( BGRC_Handle hGrc, void **ppPacket );

/***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BGRC_PACKET_WRITE_H__ */

/* end of file */
