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
    BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

    BDBG_MSG(("BHDM_MHL_P_Hdcp_GetVersion"));

    BDBG_ASSERT(hHdm->bMhlMode);

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

done:
    return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_GetRxBCaps
    ( BHDM_Handle         hHdm,
      uint8_t            *pucRxBcaps )
{
    BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
    uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */
    BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

    BDBG_MSG(("BHDM_MHL_P_Hdcp_GetRxBCaps"));

    BDBG_ASSERT(hHdm->bMhlMode);

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

done:
    return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_GetRxStatus
    ( BHDM_Handle        hHdm,
      uint16_t           *puiRxStatus )
{
    BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
    uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */
    BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

    BDBG_MSG(("BHDM_MHL_P_Hdcp_GetRxStatus"));

    BDBG_ASSERT(hHdm->bMhlMode);

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
    BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

    BDBG_MSG(("BHDM_MHL_P_Hdcp_GetRepeaterSha"));

    BDBG_ASSERT(hHdm->bMhlMode);

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
    BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

    BDBG_MSG(("BHDM_MHL_P_Hdcp_GetRepeaterKsvFifo"));

    BDBG_ASSERT(hHdm->bMhlMode);

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

done:
    return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_GetRxPj
    ( BHDM_Handle         hHdm,
      uint8_t            *pucRxPj )
{
    BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
    uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */
    BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

    BDBG_MSG(("BHDM_MHL_P_Hdcp_GetRxPj"));

    BDBG_ASSERT(hHdm->bMhlMode);

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

done:
    return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_GetRxRi
    ( BHDM_Handle         hHdm,
      uint16_t           *puiRxRi )
{
    BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
    uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */
    BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

    BDBG_MSG(("BHDM_MHL_P_Hdcp_GetRxRi"));

    BDBG_ASSERT(hHdm->bMhlMode);

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

done:
    return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_SendTxAksv
    ( BHDM_Handle         hHdm,
      const uint8_t      *pucTxAksv )
{
    BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
    uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */
    BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

    BDBG_MSG(("BHDM_MHL_P_Hdcp_SendTxAksv"));

    BDBG_ASSERT(hHdm->bMhlMode);

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

done:
    return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_GetRxBksv
    ( BHDM_Handle         hHdm,
      unsigned char       aucRxBksv[BHDM_HDCP_KSV_LENGTH] )
{
    BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
    uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */
    BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;
    uint32_t i;

    BDBG_MSG(("BHDM_MHL_P_Hdcp_GetRxBksv"));

    BDBG_ASSERT(hHdm->bMhlMode);

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

done:
    return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_SendAnValue
    ( BHDM_Handle         hHdm,
      uint8_t             *pucAnValue )
{
    BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
    uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */
    BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

    BDBG_MSG(("BHDM_MHL_P_Hdcp_SendAnValue"));

    BDBG_ASSERT(hHdm->bMhlMode);

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

done:
    return rc;
}

BERR_Code BHDM_MHL_P_Hdcp_SendAinfoByte
    ( BHDM_Handle         hHdm,
      uint8_t            *pucAinfoByte )
{
    BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
    uint8_t ucAddr = (BHDM_HDCP_RX_I2C_ADDR << 1); /* 0x74 which corresponds to read */
    BHDM_P_Mhl_DdcReq_Handle hDdcReq = hHdm->hMhl->hDdcReq;

    BDBG_MSG(("BHDM_MHL_P_Hdcp_SendAinfoByte"));

    BDBG_ASSERT(hHdm->bMhlMode);

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

done:
    return rc;
}
