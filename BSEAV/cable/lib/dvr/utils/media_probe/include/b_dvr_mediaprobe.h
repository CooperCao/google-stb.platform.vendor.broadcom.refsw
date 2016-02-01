/***************************************************************************
 *     (c)2007-2012 Broadcom Corporation
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
 * Module Description:
 * Media Probe shall probe the media from a given offset of a recorded/TSB buffered
 * program/in-progress recording based on the sub-Streams added to the program.
 * Probing can be done on the encryted recording provided the DRMService 
 * associated with the program is passed to the mediaProbe module.
 * Probing is done by reading the data from the program and 
 * passing the transport packets to the magnum media module.
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#ifndef _B_DVR_MEDIAPROBE_H
#define _B_DVR_MEDIAPROBE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "b_dvr_const.h"
#include "b_dvr_datatypes.h"

BDBG_OBJECT_ID_DECLARE(B_DVR_MediaProbe);
/*********************************************************************
Summary:
B_DVR_MediaProbeHandle is a unique handle per mediaProbe context.
**********************************************************************/
typedef struct B_DVR_MediaProbe *B_DVR_MediaProbeHandle;

/*********************************************************************
Summary:
B_DVR_MediaProbe_Open shall initialize all the sub modules required
for media probing.
**********************************************************************/
B_DVR_MediaProbeHandle B_DVR_MediaProbe_Open(B_DVR_MediaNode mediaNode,
                                             NEXUS_FileRecordHandle nexusFileRecord,
                                             unsigned volumeIndex,
                                             unsigned serviceIndex);
/*********************************************************************
Summary:
B_DVR_MediaProbe_Parse shall probe the media from a given 
offset till all the stream parameters for all the esStreams in a 
program are extracted.
**********************************************************************/
bool B_DVR_MediaProbe_Parse(B_DVR_MediaProbeHandle mediaProbe,
                            B_DVR_DRMServiceHandle drmService,
                            off_t *probeStartOffset);

/*********************************************************************
Summary:
B_DVR_MediaProbe_Close shall de-initialize all the sub-modules
used for media probing.
**********************************************************************/
B_DVR_ERROR B_DVR_MediaProbe_Close(B_DVR_MediaProbeHandle mediaProbe);

#ifdef __cplusplus
}
#endif

#endif /*_B_DVR_MEDIAPROBE_H */
