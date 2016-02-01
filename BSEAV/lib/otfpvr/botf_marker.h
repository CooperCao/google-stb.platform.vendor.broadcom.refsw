/***************************************************************************
 *     (c)2012 Broadcom Corporation
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: File to build/parse PVR marker
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 **************************************************************************/
#ifndef BOTF_MARKER_H
#define BOTF_MARKER_H

#ifdef __cplusplus
extern "C"
{
#endif

#define B_MARKER_SC	0xAE

#define B_MARKER_CHUNK_EOF (0xFFFFFFFEul)

/* structure to be passed between feeder and gop manager inside MPEG stream */
typedef struct bpvr_marker {
	uint32_t chunk_cnt; /* chunk counter */
	uint32_t off_hi; /* file offset, high 32 bits */
	uint32_t off_lo; /* file offset, low 32 bits */
} bpvr_marker;

/***************************************************************************
Summary:
OTF marker Params
****************************************************************************/
typedef struct BOTF_MarkerParams
{
	uint32_t ChunkCntr;           /* chunk counter */
	uint32_t OffsetHi;            /* file offset, high 32 bits */
	uint32_t OffsetLow;           /* file offset, low 32 bits */
	uint32_t Pid;		      /* Transport stream PID */
        unsigned TimestampOffset;     /* 4 byte timestamp offset used for 192 byte TS packet*/ 
}BOTF_MarkerParams; 

/*
* This function is used to encapsulate marker into the MPEG-2 ES stream .
* Return value:
*   - number of bytes occupied in the destination buffer
*/
size_t
bpvr_marker_build_es(
		bpvr_marker *marker, /* marker to be encapsulated */
		void *pkt  /* pointer to the user allocated data, shall be at least 64 bytes wide */
		);

/***************************************************************************
Summary:
Build TS marker

Description:
This routine builds MPEG transport stream marker packets which is inserted 
at the end of each chunk to mark the discontinuity. MarkerBuf should have
space for at least two TS packets(=188*2).

Parameters:
MarkerParams - Marker parameters
MarkerBuf - Pointer to marker buffer

Returns:
    BERR_SUCCESS                - Built marker succssfully
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BOTF_BuildMarkerTS( const BOTF_MarkerParams *MarkerParams, void *MarkerBuf, size_t buf_len, size_t *pkt_len);

/***************************************************************************
Summary:
Build DSS marker

Description:
This routine builds DSS stream marker packet which is inserted 
at the end of each chunk to mark the discontinuity. MarkerBuf should have
space for at least one DSS packet(=130).

Parameters:
MarkerParams - Marker parameters
MarkerBuf - Pointer to marker buffer

Returns:
    BERR_SUCCESS                - Built marker succssfully
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BOTF_BuildMarkerDSS( const BOTF_MarkerParams *MarkerParams, void *MarkerBuf, size_t buf_len, size_t *pkt_len);

/***************************************************************************
Summary:
Build PES marker

Description:
This routine builds MPEG transport stream marker packets which is inserted 
at the end of each chunk to mark the discontinuity. MarkerBuf should have
space for at least two PES packets.

Parameters:
MarkerParams - Marker parameters
MarkerBuf - Pointer to marker buffer

Returns:
    BERR_SUCCESS                - Built marker succssfully
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BOTF_BuildMarkerPES( const BOTF_MarkerParams *MarkerParams, void *MarkerBuf, size_t buf_len, size_t *pkt_len);


#ifdef __cplusplus
}
#endif

#endif


