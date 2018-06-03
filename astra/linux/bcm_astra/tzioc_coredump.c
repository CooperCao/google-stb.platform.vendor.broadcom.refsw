/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/

#include <linux/string.h>

#include "tzioc_drv.h"
#include "tzioc_msg.h"
#include "tzioc_peer.h"
#include "uappd_msg.h"


int _tzioc_peer_coredump(
	struct tzioc_client *pClient,
	const char *pPeerName,
	uintptr_t ulPaddr,
    uint32_t ulBytes)
{
	uint8_t msg[sizeof(struct tzioc_msg_hdr) +
				sizeof(struct uappd_msg_uapp_coredump_cmd)];
	struct tzioc_msg_hdr *pHdr =
		(tzioc_msg_hdr *)&msg;
	struct uappd_msg_uapp_coredump_cmd *pCmd =
		(struct uappd_msg_uapp_coredump_cmd *)TZIOC_MSG_PAYLOAD(pHdr);
	int err = 0;

	pHdr->ucType = UAPPD_MSG_UAPP_COREDUMP;
	pHdr->ucOrig = pClient->id;
	pHdr->ucDest = TZIOC_CLIENT_ID_UAPPD;
	pHdr->ucSeq  = 0;
	pHdr->ulLen  = sizeof(*pCmd);

	strncpy(pCmd->name, pPeerName, UAPPD_NAME_LEN_MAX-1);
    /* Coverity:Explicitly null terminate string */
    pCmd->name[UAPPD_NAME_LEN_MAX-1] = '\0';
	pCmd->paddr  = ulPaddr;
	pCmd->bytes  = ulBytes;
	pCmd->cookie = (uintptr_t)pClient;

	LOGI("Sending Coredump Message pAddr=0x%zx size=0x%x", (size_t)pCmd->paddr, pCmd->bytes);
	err = _tzioc_msg_send(
		pClient,
		pHdr, (uint8_t *)pCmd);

	if (err) {
		LOGE("failed to send user app stop msg");
	}

	/* immediately switch to TZOS */
	_tzioc_call_smc(0x83000007);
	return 0;
}
