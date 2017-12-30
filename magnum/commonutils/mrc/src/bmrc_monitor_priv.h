/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 * Implementation of the Realtime Memory Monitor for 7038
 *
 ***************************************************************************/
#ifndef BMRC_MONITOR_PRIV_H_
#define BMRC_MONITOR_PRIV_H_
#ifdef __cplusplus
extern "C" {
#endif

struct BMRC_P_ClientMapEntry {
    BCHP_MemcClient client;
    BMRC_Monitor_HwBlock block;
};

struct BMRC_P_MonitorClientMap {
    uint8_t fileBlocks[BMRC_Monitor_HwBlock_eMax]; /* clients protected based on allocation place */
    struct BMRC_P_ClientMapEntry clientMap[BCHP_MemcClient_eMax+1]; /* this array sorted by value of memcClient and it's used to get list of MEMC clients by HW Block */
    uint16_t clientMapEntry[BMRC_Monitor_HwBlock_eMax]; /* this array has entry in sorted array of clientMap */
    uint16_t clientToBlock[BCHP_MemcClient_eMax]; /* map that translates BCHP_MemcClient to BMRC_Monitor_HwBlock */
};

struct BMRC_Monitor_P_ClientList {
    bool clients[BCHP_MemcClient_eMax];
};


void BMRC_Monitor_P_MapInit(struct BMRC_P_MonitorClientMap *map);
void BMRC_Monitor_P_MapGetClientsByFileName(const struct BMRC_P_MonitorClientMap *map, const char *fname, struct BMRC_Monitor_P_ClientList *clientList);
void BMRC_Monitor_P_MapGetHwClients(struct BMRC_P_MonitorClientMap *map, struct BMRC_Monitor_P_ClientList *clientList);
void BMRC_Monitor_P_PrintBitmap_isrsafe(const struct BMRC_P_MonitorClientMap *map, const uint32_t *bitmap, size_t bitmapSize, const char *blockedType);
void BMRC_Monitor_P_SetHwBlocks(const struct BMRC_P_MonitorClientMap *map, const uint8_t *hwBlocks, struct BMRC_Monitor_P_ClientList *clientList, bool add);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* BMRC_MONITOR_PRIV_H_ */

/* End of File */
