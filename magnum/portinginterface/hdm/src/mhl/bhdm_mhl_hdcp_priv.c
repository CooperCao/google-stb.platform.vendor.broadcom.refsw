/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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
******************************************************************************/
#include "../common/bhdm_priv.h"
#include "bhdm_mhl_hdcp_priv.h"

BDBG_MODULE(BHDM_MHL_HDCP);
BDBG_OBJECT_ID(BHDM_MHL_HDCP);

#define BHDM_MHL_P_HDCP_TRANSACTION_TIMEOUT           100 /* msecs */

static void BHDM_MHL_P_Hdcp_SendCommand_isr
	( BHDM_P_Mhl_DdcReq_Handle  hDdcReq )
{
	/* If nothing is going on in the DDC Requester, send the next command from DDC REQ Queue */
	BHDM_P_Mhl_Req_Active_isr(hDdcReq->hReq, &hDdcReq->hReq->bActive, &hDdcReq->hReq->bAbortActive);

	if(!hDdcReq->hReq->bAbortActive && !hDdcReq->hReq->bActive)
	{
		BDBG_MSG(("Send DDC Command in HDCP"));

		/* Start send command-DDC done interrupt sequence. The whole sequence of EDID read command should
		   complete in DDC done ISR handler. */
		BHDM_P_Mhl_Req_SendNextCmd_isr(hDdcReq->hReq, &hDdcReq->hReq->stLastCmd,
										&hDdcReq->hReq->bRetryLastCmd);
	}
}

BERR_Code BHDM_MHL_P_Hdcp_GetVersion
	( BHDM_Handle         hHdm,
	  uint8_t            *pucHdcp2Version )
{
	BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
	uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1) + 1; /* 0x74 which corresponds to read */

	BDBG_MSG(("BHDM_MHL_P_Hdcp_GetVersion"));

	if (hHdm->bMhlMode)
	{
		BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

		BKNI_EnterCriticalSection();
		rc = BHDM_P_Mhl_DdcReq_OffsetRead_isr(hDdcReq, BHDM_P_Mhl_CbusCmdType_eHdcp,
												ucAddr, BHDM_HDCP_RX_HDCP2VERSION, 1,
												&hDdcReq->stHdcpInfo.ucHdcpVersion, 0);

		if (rc == BHDM_P_MHL_CBUS_SUCCESS)
		{
			hDdcReq->eHdcpEvent |= BHDM_P_MHL_HDCP_EVENT_VERSION;
			BHDM_MHL_P_Hdcp_SendCommand_isr(hDdcReq);
		}
		BKNI_LeaveCriticalSection();

		if (rc != BHDM_P_MHL_CBUS_SUCCESS) goto done;

		rc = BKNI_WaitForEvent(hDdcReq->hHdcpVersionValueEvent, BHDM_MHL_P_HDCP_TRANSACTION_TIMEOUT);
		if (rc == BERR_TIMEOUT)
		{
			BDBG_ERR(("Timed-out waiting for HDCP version."));
		}
		else
		{
			*pucHdcp2Version = hDdcReq->stHdcpInfo.ucHdcpVersion;
			BDBG_MSG(("HDCP version = %d", *pucHdcp2Version));
		}
	}
	else
	{
		/* check for HDCP 2.x Support */
		rc = BHDM_P_BREG_I2C_Read(hHdm, BHDM_HDCP_RX_I2C_ADDR,	BHDM_HDCP_RX_HDCP2VERSION, pucHdcp2Version, 1);
	}

done:
	return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_GetRxBCaps
	( BHDM_Handle         hHdm,
	  uint8_t            *pucRxBcaps )
{
	BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
	uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */

	BDBG_MSG(("BHDM_MHL_P_Hdcp_GetRxBCaps"));

	if (hHdm->bMhlMode)
	{
		BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

		BKNI_EnterCriticalSection();
		rc = BHDM_P_Mhl_DdcReq_OffsetRead_isr(hDdcReq, BHDM_P_Mhl_CbusCmdType_eHdcp,
												ucAddr, BHDM_HDCP_RX_BCAPS, 1,
												&hDdcReq->stHdcpInfo.ucRxBcaps, 0);
		if (rc == BHDM_P_MHL_CBUS_SUCCESS)
		{
			hDdcReq->eHdcpEvent |= BHDM_P_MHL_HDCP_EVENT_RX_BCAPS;
			BHDM_MHL_P_Hdcp_SendCommand_isr(hDdcReq);
		}
		BKNI_LeaveCriticalSection();

		if (rc != BHDM_P_MHL_CBUS_SUCCESS) goto done;

		rc = BKNI_WaitForEvent(hDdcReq->hHdcpRxBcapsValueEvent, BHDM_MHL_P_HDCP_TRANSACTION_TIMEOUT);
		if (rc == BERR_TIMEOUT)
		{
			BDBG_ERR(("Timed-out waiting for Rx Bcaps."));
		}
		else
		{
			*pucRxBcaps = hDdcReq->stHdcpInfo.ucRxBcaps;
			BDBG_MSG(("Rx BCaps = 0x%x", *pucRxBcaps));
		}
	}
	else
	{
#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
		/* We can't access I2C if control has been handed over to HW */
		if (hHdm->bAutoRiPjCheckingEnabled)
		{
			*pucRxBcaps = hHdm->RxBCaps;
		}
		else
		{
			rc = BHDM_P_BREG_I2C_Read(hHdm,	BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_BCAPS, pucRxBcaps, 1);
		}
#else
		rc = BHDM_P_BREG_I2C_Read(hHdm, BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_BCAPS, pucRxBcaps, 1);
#endif
	}
done:
	return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_GetRxStatus
	( BHDM_Handle        hHdm,
	  uint16_t           *puiRxStatus )
{
	BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
	uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */

	BDBG_MSG(("BHDM_MHL_P_Hdcp_GetRxStatus"));

	if (hHdm->bMhlMode)
	{
		BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

		BKNI_EnterCriticalSection();
		rc = BHDM_P_Mhl_DdcReq_OffsetRead_isr(hDdcReq, BHDM_P_Mhl_CbusCmdType_eHdcp,
												ucAddr, BHDM_HDCP_RX_BSTATUS, BHDM_P_MHL_HDCP_RX_STATUS_SIZE,
												hDdcReq->stHdcpInfo.aucRxStatus, 0);
		if (rc == BHDM_P_MHL_CBUS_SUCCESS)
		{
			hDdcReq->eHdcpEvent |= BHDM_P_MHL_HDCP_EVENT_RX_STATUS;
			BHDM_MHL_P_Hdcp_SendCommand_isr(hDdcReq);
		}
		BKNI_LeaveCriticalSection();

		if (rc != BHDM_P_MHL_CBUS_SUCCESS) goto done;

		rc = BKNI_WaitForEvent(hDdcReq->hHdcpRxStatusValueEvent, BHDM_MHL_P_HDCP_TRANSACTION_TIMEOUT);
		if (rc == BERR_TIMEOUT)
		{
			BDBG_ERR(("Timed-out waiting for Rx Status."));
			goto done;
		}
		else
		{
			*puiRxStatus = hDdcReq->stHdcpInfo.aucRxStatus[0] | (hDdcReq->stHdcpInfo.aucRxStatus[1] << 8);
			BDBG_MSG(("Rx Status = 0x%x", *puiRxStatus));
		}
	}
	else
	{
        rc = BHDM_P_BREG_I2C_Read(hHdm, BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_BSTATUS, (uint8_t *)puiRxStatus, 2);
	}

done:
	return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_GetRepeaterSha
	( BHDM_Handle         hHdm,
	  uint8_t             aucKsv[BHDM_P_MHL_HDCP_REPEATER_KSV_SIZE],
	  uint8_t             ucHOffset )
{
	BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
	uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */

	BDBG_MSG(("BHDM_MHL_P_Hdcp_GetRepeaterSha"));

	if (hHdm->bMhlMode)
	{
		BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

		BKNI_EnterCriticalSection();
		rc = BHDM_P_Mhl_DdcReq_OffsetRead_isr(hDdcReq, BHDM_P_Mhl_CbusCmdType_eHdcp,
												ucAddr, BHDM_HDCP_REPEATER_SHA1_V_H0 + (ucHOffset * 4),
												BHDM_P_MHL_HDCP_REPEATER_SHA_SIZE,
												hDdcReq->stHdcpInfo.aucKsv, 0);
		if (rc == BHDM_P_MHL_CBUS_SUCCESS)
		{
			hDdcReq->eHdcpEvent |= BHDM_P_MHL_HDCP_EVENT_KSV;
			BHDM_MHL_P_Hdcp_SendCommand_isr(hDdcReq);
		}
		BKNI_LeaveCriticalSection();

		if (rc != BHDM_P_MHL_CBUS_SUCCESS) goto done;

		rc = BKNI_WaitForEvent(hDdcReq->hHdcpKsvValueEvent, BHDM_MHL_P_HDCP_TRANSACTION_TIMEOUT);
		if (rc == BERR_TIMEOUT)
		{
			BDBG_ERR(("Timed-out waiting for Ksv List."));
			goto done;
		}
		else
		{
			int j;
			for (j=0; j<BHDM_P_MHL_HDCP_REPEATER_SHA_SIZE; j++)
			{
				aucKsv[j] = hDdcReq->stHdcpInfo.aucKsv[j];
				BDBG_MSG(("Ksv[%d] = 0x%x", j, aucKsv[j]));
			}
		}
	}
	else
	{
		rc = BHDM_P_BREG_I2C_Read(hHdm,
							BHDM_HDCP_RX_I2C_ADDR,
							BHDM_HDCP_REPEATER_SHA1_V_H0 + (ucHOffset * 4),
							aucKsv, 4);
	}

done:
	return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_GetRepeaterKsvFifo
	( BHDM_Handle         hHdm,
	  uint8_t            *pucRxKsvList,
	  uint16_t            uiRepeaterDeviceCount )
{
	BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
	uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */
	uint16_t uiBufSize;

	BDBG_MSG(("BHDM_MHL_P_Hdcp_GetRepeaterKsvFifo"));

	if (hHdm->bMhlMode)
	{
		BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

		hDdcReq->stHdcpInfo.uiRepeaterKsvListSize = uiBufSize = uiRepeaterDeviceCount * BHDM_HDCP_KSV_LENGTH;
		hDdcReq->stHdcpInfo.pucRepeaterKsvList = (uint8_t *)BKNI_Malloc(sizeof(uint8_t) * uiBufSize);

		if (hDdcReq->stHdcpInfo.pucRepeaterKsvList == NULL)
		{
			BDBG_ERR(("Failed to allocate memory for Repeater KSV."));
			return BERR_OUT_OF_SYSTEM_MEMORY;
		}

		BKNI_EnterCriticalSection();
		rc = BHDM_P_Mhl_DdcReq_OffsetRead_isr(hDdcReq, BHDM_P_Mhl_CbusCmdType_eHdcp,
												ucAddr, BHDM_HDCP_REPEATER_KSV_FIFO, uiBufSize,
												hDdcReq->stHdcpInfo.pucRepeaterKsvList, 0);
		if (rc == BHDM_P_MHL_CBUS_SUCCESS)
		{
			hDdcReq->eHdcpEvent |= BHDM_P_MHL_HDCP_EVENT_RX_KSV_LIST;
			BHDM_MHL_P_Hdcp_SendCommand_isr(hDdcReq);
		}
		BKNI_LeaveCriticalSection();

		if (rc != BHDM_P_MHL_CBUS_SUCCESS) goto done;

		rc = BKNI_WaitForEvent(hDdcReq->hHdcpRepeaterKsvListValueEvent, BHDM_MHL_P_HDCP_TRANSACTION_TIMEOUT);
		if (rc == BERR_TIMEOUT)
		{
			BDBG_ERR(("Timed-out waiting for Repeater Ksv List."));
			goto done;
		}
		else
		{
			BKNI_Memcpy(pucRxKsvList, hDdcReq->stHdcpInfo.pucRepeaterKsvList, sizeof(uint8_t) * uiRepeaterDeviceCount);
			{
				int i;
				for (i=0; i<uiRepeaterDeviceCount; i++)
				{
					BDBG_MSG(("pucRxKsvList[%d] = 0x%x", i, *pucRxKsvList));
					pucRxKsvList++;
				}
			}
		}
	}
	else
	{
		uint16_t uiBufSize = uiRepeaterDeviceCount * BHDM_HDCP_KSV_LENGTH;
		rc = BHDM_P_BREG_I2C_Read(hHdm,	BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_REPEATER_KSV_FIFO, pucRxKsvList, uiBufSize);
	}

done:
	return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_GetRxPj
	( BHDM_Handle         hHdm,
	  uint8_t            *pucRxPj )
{
	BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
	uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */

	BDBG_MSG(("BHDM_MHL_P_Hdcp_GetRxPj"));

	if (hHdm->bMhlMode)
	{
		BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

		BKNI_EnterCriticalSection();
		rc = BHDM_P_Mhl_DdcReq_OffsetRead_isr(hDdcReq, BHDM_P_Mhl_CbusCmdType_eHdcp,
												ucAddr, BHDM_HDCP_RX_PJ, BHDM_P_MHL_HDCP_RX_PJ_SIZE,
												&hDdcReq->stHdcpInfo.ucRxPj, 0);
		if (rc == BHDM_P_MHL_CBUS_SUCCESS)
		{
			hDdcReq->eHdcpEvent |= BHDM_P_MHL_HDCP_EVENT_RX_PJ;
			BHDM_MHL_P_Hdcp_SendCommand_isr(hDdcReq);
		}
		BKNI_LeaveCriticalSection();

		if (rc != BHDM_P_MHL_CBUS_SUCCESS) goto done;

		rc = BKNI_WaitForEvent(hDdcReq->hHdcpRxPjValueEvent, BHDM_MHL_P_HDCP_TRANSACTION_TIMEOUT);
		if (rc == BERR_TIMEOUT)
		{
			BDBG_ERR(("Timed-out waiting for Rx Pj."));
		}
		else
		{
			*pucRxPj = hDdcReq->stHdcpInfo.ucRxPj;
			BDBG_MSG(("Rx Pj = 0x%x", *pucRxPj));
		}
	}
	else
	{
        rc = BHDM_P_BREG_I2C_Read(hHdm, BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_PJ, pucRxPj, 1) ;
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Tx%d: Pj LIC: Rx Pj I2C read failure %d", hHdm->eCoreId, rc));
		}
	}

done:
	return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_GetRxRi
	( BHDM_Handle         hHdm,
	  uint16_t           *puiRxRi )
{
	BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
	uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */

	BDBG_MSG(("BHDM_MHL_P_Hdcp_GetRxRi"));

	if (hHdm->bMhlMode)
	{
		BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

		BKNI_EnterCriticalSection();
		rc = BHDM_P_Mhl_DdcReq_OffsetRead_isr(hDdcReq, BHDM_P_Mhl_CbusCmdType_eHdcp,
												ucAddr, BHDM_HDCP_RX_RI0, BHDM_P_MHL_HDCP_RX_RI_SIZE,
												(uint8_t *)hDdcReq->stHdcpInfo.aucRxRi, 0);
		if (rc == BHDM_P_MHL_CBUS_SUCCESS)
		{
			hDdcReq->eHdcpEvent |= BHDM_P_MHL_HDCP_EVENT_RX_RI;
			BHDM_MHL_P_Hdcp_SendCommand_isr(hDdcReq);
		}
		BKNI_LeaveCriticalSection();

		if (rc != BHDM_P_MHL_CBUS_SUCCESS) goto done;

		rc = BKNI_WaitForEvent(hDdcReq->hHdcpRxRiValueEvent, BHDM_MHL_P_HDCP_TRANSACTION_TIMEOUT);
		if (rc == BERR_TIMEOUT)
		{
			BDBG_ERR(("Timed-out waiting for Rx Ri."));
		}
		else
		{
			*puiRxRi = hDdcReq->stHdcpInfo.aucRxRi[0] | (hDdcReq->stHdcpInfo.aucRxRi[1] << 8);
			BDBG_MSG(("Rx Ri = 0x%x", *puiRxRi));
		}
	}
	else
	{
#if BHDM_CONFIG_HDCP_RI_SHORT_READ
		BDBG_MSG(("HDCP Ri check configured for SHORT read")) ;
		rc = BHDM_P_BREG_I2C_ReadNoAddr(hHdm, BHDM_HDCP_RX_I2C_ADDR, (uint8_t *)puiRxRi, 2) ;
#else
		BDBG_MSG(("HDCP Ri check configured for NORMAL read")) ;
		rc = BHDM_P_BREG_I2C_Read(hHdm,	BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_RI0, (uint8_t *)puiRxRi, 2) ;
#endif
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Tx%d: Ri LIC: RxRi I2C read failure %x", hHdm->eCoreId, rc)) ;
		}
	}
done:
	return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_SendTxAksv
	( BHDM_Handle         hHdm,
	  const uint8_t      *pucTxAksv )
{
	BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
	uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */

	BDBG_MSG(("BHDM_MHL_P_Hdcp_SendTxAksv"));

	if (hHdm->bMhlMode)
	{
		BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

		BKNI_EnterCriticalSection();
		rc = BHDM_P_Mhl_DdcReq_Write_isr(hDdcReq, BHDM_P_Mhl_CbusCmdType_eHdcp,
										ucAddr, BHDM_HDCP_RX_AKSV0, BHDM_HDCP_KSV_LENGTH,
										pucTxAksv, 0);
		if (rc == BHDM_P_MHL_CBUS_SUCCESS)
		{
			hDdcReq->eHdcpEvent |= BHDM_P_MHL_HDCP_EVENT_TX_AKSV;
			BHDM_MHL_P_Hdcp_SendCommand_isr(hDdcReq);
		}
		BKNI_LeaveCriticalSection();

		if (rc != BHDM_P_MHL_CBUS_SUCCESS) goto done;

		rc = BKNI_WaitForEvent(hDdcReq->hHdcpTxAksvValueEvent, BHDM_MHL_P_HDCP_TRANSACTION_TIMEOUT);
		if (rc == BERR_TIMEOUT)
		{
			BDBG_ERR(("Timed-out waiting for writing Tx Aksv to complete."));
		}
	}
	else
	{
		/* Write the TxAksv value to the HDCP Rx */
		rc = BREG_I2C_Write(hHdm->hI2cRegHandle, BHDM_HDCP_RX_I2C_ADDR,	BHDM_HDCP_RX_AKSV0, pucTxAksv, BHDM_HDCP_KSV_LENGTH) ;
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Tx%d: Aksv I2C write error", hHdm->eCoreId));
			rc = BHDM_HDCP_TX_AKSV_I2C_WRITE_ERROR;
		}
	}
done:
	return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_GetRxBksv
	( BHDM_Handle         hHdm,
	  unsigned char       aucRxBksv[BHDM_HDCP_KSV_LENGTH] )
{
	BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
	uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */

	BDBG_MSG(("BHDM_MHL_P_Hdcp_GetRxBksv"));

	if (hHdm->bMhlMode)
	{
		BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;
		uint32_t i;

		BKNI_EnterCriticalSection();
		rc = BHDM_P_Mhl_DdcReq_OffsetRead_isr(hDdcReq, BHDM_P_Mhl_CbusCmdType_eHdcp,
												ucAddr, BHDM_HDCP_RX_BKSV0, BHDM_HDCP_KSV_LENGTH,
												hDdcReq->stHdcpInfo.aucRxBksv, 0);
		if (rc == BHDM_P_MHL_CBUS_SUCCESS)
		{
			hDdcReq->eHdcpEvent |= BHDM_P_MHL_HDCP_EVENT_RX_BKSV;
			BHDM_MHL_P_Hdcp_SendCommand_isr(hDdcReq);
		}
		BKNI_LeaveCriticalSection();

		if (rc != BHDM_P_MHL_CBUS_SUCCESS) goto done;

		rc = BKNI_WaitForEvent(hDdcReq->hHdcpRxBksvValueEvent, BHDM_MHL_P_HDCP_TRANSACTION_TIMEOUT);
		if (rc == BERR_TIMEOUT)
		{
			BDBG_ERR(("Timed-out waiting for Rx Bksv."));
			goto done;
		}

		for (i=0; i<BHDM_HDCP_KSV_LENGTH; i++)
		{
			aucRxBksv[i] = hDdcReq->stHdcpInfo.aucRxBksv[i];
			BDBG_MSG(("Rx Bksv[%i] = 0x%x", i, aucRxBksv[i]));
		}

	}
	else /* HDMI mode */
	{
		rc = BHDM_P_BREG_I2C_Read(hHdm,	BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_BKSV0, aucRxBksv, BHDM_HDCP_KSV_LENGTH ) ;
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Tx%d: Bksw I2C read error", hHdm->eCoreId));
			rc = BHDM_HDCP_RX_BKSV_I2C_READ_ERROR;
		}
	}

done:
	return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_SendAnValue
	( BHDM_Handle         hHdm,
	  uint8_t             *pucAnValue )
{
	BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
	uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */

	BDBG_MSG(("BHDM_MHL_P_Hdcp_SendAnValue"));

	if (hHdm->bMhlMode)
	{
		BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

		BKNI_EnterCriticalSection();
		rc = BHDM_P_Mhl_DdcReq_Write_isr(hDdcReq, BHDM_P_Mhl_CbusCmdType_eHdcp,
											ucAddr, BHDM_HDCP_RX_AN0, BHDM_HDCP_AN_LENGTH,
											(const uint8_t *)pucAnValue, 0);
		if (rc == BHDM_P_MHL_CBUS_SUCCESS)
		{
			hDdcReq->eHdcpEvent |= BHDM_P_MHL_HDCP_EVENT_AN;
			BHDM_MHL_P_Hdcp_SendCommand_isr(hDdcReq);
		}
		BKNI_LeaveCriticalSection();

		if (rc != BHDM_P_MHL_CBUS_SUCCESS) goto done;

		rc = BKNI_WaitForEvent(hDdcReq->hHdcpAnValueEvent, BHDM_MHL_P_HDCP_TRANSACTION_TIMEOUT);
		if (rc == BERR_TIMEOUT)
		{
			BDBG_ERR(("Timed-out waiting for writing AN value to complete."));
		}
	}
	else
	{
		/* write the generate An value to the Receiver */
		rc = BREG_I2C_Write(hHdm->hI2cRegHandle, BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_AN0, pucAnValue, BHDM_HDCP_AN_LENGTH);
	}

done:
	return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_SendAinfoByte
	( BHDM_Handle         hHdm,
	  uint8_t            *pucAinfoByte )
{
	BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
	uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */

	BDBG_MSG(("BHDM_MHL_P_Hdcp_SendAinfoByte"));

	if (hHdm->bMhlMode)
	{
		BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

		BKNI_EnterCriticalSection();
		rc = BHDM_P_Mhl_DdcReq_Write_isr(hDdcReq, BHDM_P_Mhl_CbusCmdType_eHdcp,
												ucAddr, BHDM_HDCP_RX_AINFO, 1,
												(const uint8_t *)pucAinfoByte, 0);
		if (rc == BHDM_P_MHL_CBUS_SUCCESS)
		{
			hDdcReq->eHdcpEvent |= BHDM_P_MHL_HDCP_EVENT_AINFO;
			BHDM_MHL_P_Hdcp_SendCommand_isr(hDdcReq);
		}
		BKNI_LeaveCriticalSection();

		if (rc != BHDM_P_MHL_CBUS_SUCCESS) goto done;

		rc = BKNI_WaitForEvent(hDdcReq->hHdcpAinfoByteValueEvent, BHDM_MHL_P_HDCP_TRANSACTION_TIMEOUT);
		if (rc == BERR_TIMEOUT)
		{
			BDBG_ERR(("Timed-out waiting for writing Ainfo byte to complete."));
		}
	}
	else
	{
#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
		if (!hHdm->bAutoRiPjCheckingEnabled)
		{
			rc = BREG_I2C_Write(hHdm->hI2cRegHandle, BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_AINFO, pucAinfoByte, 1);
		}
#else
		rc = BREG_I2C_Write(hHdm->hI2cRegHandle, BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_AINFO, pucAinfoByte, 1);
#endif
	}

done:
	return rc;
}
