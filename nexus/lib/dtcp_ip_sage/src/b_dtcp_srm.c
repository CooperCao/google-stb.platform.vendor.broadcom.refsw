/********************************************************************************************
*     (c)2004-2015 Broadcom Corporation                                                     *
*                                                                                           *
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,   *
*  and may only be used, duplicated, modified or distributed pursuant to the terms and      *
*  conditions of a separate, written license agreement executed between you and Broadcom    *
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants*
*  no license (express or implied), right to use, or waiver of any kind with respect to the *
*  Software, and Broadcom expressly reserves all rights in and to the Software and all      *
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU       *
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY                    *
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.                                 *
*
*  Except as expressly set forth in the Authorized License,                                 *
*
*  1.     This program, including its structure, sequence and organization, constitutes     *
*  the valuable trade secrets of Broadcom, and you shall use all reasonable efforts to      *
*  protect the confidentiality thereof,and to use this information only in connection       *
*  with your use of Broadcom integrated circuit products.                                   *
*                                                                                           *
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"          *
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR                   *
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO            *
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES            *
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,            *
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION             *
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF              *
*  USE OR PERFORMANCE OF THE SOFTWARE.                                                      *
*                                                                                           *
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS         *
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR             *
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR               *
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF             *
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT              *
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE            *
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF              *
*  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *     DTCP Ake core functions, SRM (System Renewability Message) related functions.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
*********************************************************************************************/
/*! \file b_dtcp_srm.c
 *  \brief implement base DTCP SRM functions.
 */
#include <arpa/inet.h>
#include "bstd.h"
#include "bkni.h"
#include "b_dtcp_applib.h"
#include "b_dtcp_ake.h"
#include "b_dtcp_stack.h"
#include "b_dtcp_status_codes.h"

BDBG_MODULE(b_dtcp_ip);

/* \brief get srm version number.
 */
unsigned short B_DTCP_GetSrmV(unsigned char * buffer)
{
    B_Srm_Header_T * Header = (B_Srm_Header_T *)buffer;
    /* Endianess */
    return ntohs(Header->Version);
}
void B_DTCP_SetSrmV(unsigned short version, unsigned char * buffer)
{
    B_Srm_Header_T * Header = (B_Srm_Header_T *)buffer;
    Header->Version = htons(version);
}
unsigned short B_DTCP_GetSrmLength(unsigned char * buffer)
{
    B_Srm_Header_T * Header = (B_Srm_Header_T *)buffer;
    /* Endianess */
    return ntohs(Header->Length);
}
void B_DTCP_SetSrmLength(unsigned short length, unsigned char * buffer)
{
    B_Srm_Header_T * Header = (B_Srm_Header_T *)buffer;
    Header->Version = htons(length);
}
/* \brief get current srm generation number.
 */
unsigned char B_DTCP_GetSrmC(unsigned char * buffer)
{
    B_Srm_Header_T * Header = (B_Srm_Header_T *)buffer;
    return (0x0F & Header->Gen);
}
/*! \brief check if the given device has been revoked in Crl list
 *  \param[in] pSrm pointer to calling device's SRM message.
 *  \param[in] DeviceId device id to be checked.
 *  \retval true if revoked, false otherwise.
 */
bool B_DTCP_IsDeviceRevoked(B_Srm_T * pSrm, unsigned char DeviceId[DTCP_DEVICE_ID_SIZE])
{
    int i, length;
    unsigned long DeviceIdStart;
    unsigned long DeviceIdCheck;
    unsigned char * Crl, *End;
    unsigned short RangeCount;

    BDBG_ASSERT(pSrm);
    BDBG_ASSERT(pSrm->Crl);
    BDBG_ENTER(B_DTCP_IsDeviceRevoked);

    length = B_DTCP_GetSrmLength((unsigned char *)pSrm);
    Crl = pSrm->Crl;
    End = Crl + length - DTCP_SIGNATURE_SIZE - 2;

    while(Crl < End )
    {
        if( ((*Crl >> DTCP_CRL_TYPE_SHIFT) & DTCP_CRL_TYPE_MASK) ==
            DTCP_CRL_TYPE_ID )
        {
            int CrlCount = *Crl & DTCP_CRL_NUMBER_MASK;
            Crl += DTCP_CRL_TYPE_SIZE;
            /* Single CRl entries. */
            for( i = 0; i < CrlCount; i++)
            {
                if(!BKNI_Memcmp(DeviceId, Crl, DTCP_DEVICE_ID_SIZE))
                {
                    BDBG_MSG(("Device %02x %02x %02x %02x %02x is revoked\n",
                        *Crl, *(Crl+1), *(Crl+2), *(Crl+3), *(Crl+4)) );
                    return true;
                }
                Crl += DTCP_DEVICE_ID_SIZE;
            }
        }else if ( ((*Crl >> DTCP_CRL_TYPE_SHIFT) & DTCP_CRL_TYPE_MASK) ==
            DTCP_CRL_TYPE_IDS )
        {
            int CrlCount = *Crl & DTCP_CRL_NUMBER_MASK;
            Crl += DTCP_CRL_TYPE_SIZE;
            DeviceIdCheck = ntohl(*(unsigned long *)DeviceId);
            /* Range Crl entries. */
            for( i = 0; i < CrlCount; i++)
            {
                /*
                 * since maximum range is 0xFFFF (2 bytes long), we can ignore the
                 * first byte of dev id, and check only the rest of 4 bytes, see
                 * if the device to be checked is in revoked range.
                 */
                DeviceIdStart = ntohl(*(unsigned long *)(Crl + 1));
                RangeCount = ntohs(*(unsigned short *)(Crl + DTCP_DEVICE_ID_SIZE));

                if(DeviceIdCheck < DeviceIdStart + RangeCount &&
                        DeviceIdCheck > DeviceIdStart)
                {
                    BDBG_MSG(("Device %02x %02x %02x %02x %02x is revoked\n",
                        *Crl, *(Crl+1), *(Crl+2), *(Crl+3), *(Crl+4)) );
                    return true;
                }
                Crl += DTCP_DEVICE_ID_SIZE + sizeof(unsigned short);
            }
        }
    }
    BDBG_LEAVE(B_DTCP_IsDeviceRevoked);
    return false;
}
/*! \brief determine if update SRM is required for other device.
 *  \param[in] pDeviceParams calling device's parameter pointer.
 *  \param[in] pSession AKE session pointer.
 *  \retvalue BERR_SUCCESS or other error code.
 */
BERR_Code B_DTCP_CheckSrmUpdate(B_DeviceParams_T * pDeviceParams, B_AkeCoreSessionData_T * pSession)
{
    BERR_Code retValue = BERR_SUCCESS;
    unsigned short OurSrmV, OurSrmC, OurSrmG;

    BDBG_ENTER(B_DTCP_CheckSrmUpdate);

    OurSrmV = B_DTCP_GetSrmV((unsigned char *)pDeviceParams->Srm);
    OurSrmC = B_DTCP_GetSrmC((unsigned char *)pDeviceParams->Srm);
    if(pSession->AkeType == B_AkeType_eFull)
    {
        B_BaselineFullCert_T * cert = (B_BaselineFullCert_T*)pDeviceParams->Cert;
        OurSrmG = 0xFFFF & cert->DevGen;
    }else {
        BDBG_ERR(("AkeType: %d is not supported\n", pSession->AkeType));
        return BERR_NOT_SUPPORTED;
    }
#if 1
    BDBG_MSG(("OurSrmV=%d OurSrmC=%d OurSrmG=%d\n", OurSrmV, OurSrmC, OurSrmG));
    BDBG_MSG(("OtherSrmV=%d OtherSrmC=%d OtherSrmG=%d\n", pSession->OtherSrmV, pSession->OtherSrmC, pSession->OtherSrmG));
#endif
    if(OurSrmV < pSession->OtherSrmV )
    {
        pSession->OtherSrmUpdateRequired = false;
        pSession->OtherSrmReplaceRequired = false;
    }else if (OurSrmV > pSession->OtherSrmV)
    {
        if(OurSrmC <= pSession->OtherSrmG)
            pSession->OtherSrmReplaceRequired = true;
        else
            pSession->OtherSrmUpdateRequired = true;
    }else {
        if(OurSrmC <= pSession->OtherSrmC || (OurSrmC > pSession->OtherSrmC && pSession->OtherSrmC == pSession->OtherSrmG)) {
                pSession->OtherSrmUpdateRequired = false;
                pSession->OtherSrmReplaceRequired = false;
        }else if(OurSrmC <= pSession->OtherSrmG) {
            pSession->OtherSrmReplaceRequired = true;
        }else if(OurSrmC > pSession->OtherSrmG) {
            pSession->OtherSrmUpdateRequired = true;
        }
    }

    if(pSession->OtherSrmV < OurSrmV )
    {
        pSession->OurSrmUpdateRequired = false;
        pSession->OurSrmReplaceRequired = false;
    }else if (pSession->OtherSrmV > OurSrmV)
    {
        if(pSession->OtherSrmC <= OurSrmG)
            pSession->OurSrmReplaceRequired = true;
        else
            pSession->OurSrmUpdateRequired = true;
    }else {
        if(pSession->OtherSrmC <= OurSrmC ||
            ( pSession->OtherSrmC > OurSrmC && OurSrmC == OurSrmG)) {
                pSession->OurSrmUpdateRequired = false;
                pSession->OurSrmReplaceRequired = false;
        }else if( pSession->OtherSrmC <= OurSrmG) {
            pSession->OurSrmReplaceRequired = true;
        }else if( pSession->OtherSrmC > OurSrmG) {
            pSession->OurSrmUpdateRequired = true;
        }
    }

    BDBG_LEAVE(B_DTCP_CheckSrmUpdate);

    return retValue;
}
