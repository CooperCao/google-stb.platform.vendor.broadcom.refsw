/******************************************************************************
 *    (c)2009-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "b_os_lib.h"
#include "b_dtcp_applib.h"
#include "b_dtcp_ake.h"
#include "b_dtcp_srm.h"
#include "b_dtcp_status_codes.h"
#include "b_ecc_wrapper.h"
#include "b_dtcp_stack.h"
#include "b_dtcp_version.h"
#include "drm_dtcp_ip.h"
#include "drm_types.h"
#include "drm_metadata.h"
#include "nexus_platform.h"
#ifdef B_DTCP_IP_LEGACY_PRODUCTION_KEY
#include "nexus_memory.h"
#include "nexus_security.h"
#include "nexus_dma.h"
#include "nexus_keyladder.h"
#include "decrypto.h"
#endif


BDBG_MODULE(b_dtcp_ip);

char gRngSeed[20];
int  gRngSeedSize = 20;

#define DTCP_RNG_SEED_FILENAME "dtcp.rng"
#define DTCP_SRM_FILENAME "dtcp.srm"
#if 0
#define DTCP_CERT_FILENAME "dtcp.crt"
#define DTCP_PVT_KEY_FILENAME "dtcp.pvk"
#endif
#define DTCP_IDU_FILENAME "dtcp.idu"
#define DTCP_DEVICE_ID_FILENAME "/var/tmp/dtcp.devid"
#define DTCP_CONSTANT_FILENAME "dtcp.const"
#define DTCP_ENC_CONSTANT_FILENAME "dtcp_enc.const"
#define DTCP_ENC_CERT_FILENAME "dtcp_enc.crt"
#define DTCP_PUBLIC_KEY_OFFSET  8
#define DTCP_DEVICE_ID_OFFSET   3

static char sSrmFilename[256] = DTCP_SRM_FILENAME ;
static char gSrm[DTCP_SRM_SECOND_GEN_MAX_SIZE];
static int  gSrmSize = DTCP_SRM_SECOND_GEN_MAX_SIZE;

static char gCert[DTCP_BASELINE_FULL_CERT_SIZE];
static int  gCertSize = DTCP_BASELINE_FULL_CERT_SIZE;
static char gPrivKey[DTCP_PRIVATE_KEY_SIZE];
static int  gPrivKeySize = DTCP_PRIVATE_KEY_SIZE;

static char gIDu[DTCP_DEVICE_ID_SIZE];
static int gIDuSize = DTCP_DEVICE_ID_SIZE;

/*! \brief utility function, read bytes from file .
 *  param[in] aFilename name of the file to be read.
 *  param[in] aDest output buffer pointer.
 *  param[in] size of the output buffer.
 */
static int DtcpAppLib_LoadBytesFromFile(const char *aFilename, char *aDest, int *aDestSize)
{
    int returnValue = BERR_SUCCESS;
    FILE *fin = NULL;
    int bytesRead = 0;

    BDBG_ASSERT(aFilename);
    BDBG_ASSERT(aDest);
    BDBG_ASSERT(aDestSize);

    if((fin = fopen(aFilename, "rb")) != NULL)
    {
        bytesRead = (int) fread(aDest, 1, (size_t) *aDestSize, fin);
        *aDestSize = bytesRead;
        fclose(fin);
        BDBG_MSG(("Read %d bytes from %s\n", bytesRead, aFilename));
    }else {
        BDBG_ERR(("Failed to open %s for reading\n", aFilename));
        returnValue = BERR_IO_ERROR;
    }
    return returnValue;
}
/*! \brief utility write buffer to a file.
 *  param[in] aFilename output file name
 *  param[in] output buffer pointer.
 *  param[in] output buffer size.
 */
static int DtcpAppLib_SaveBytesToFile(const char *aFilename, unsigned char *aSrc, unsigned int aSrcSize)
{
    int returnValue = BERR_SUCCESS;
    FILE *fout = NULL;
    int bytesWritten = 0;

    BDBG_ASSERT(aFilename);
    BDBG_ASSERT(aSrc);

    if((fout = fopen(aFilename, "wb")) != NULL)
    {
        bytesWritten = (int)fwrite(aSrc, 1, (size_t) aSrcSize, fout);
        fclose(fout);
        BDBG_MSG(("Wrote %d bytes to %s\n", bytesWritten, aFilename));
    }else {
        returnValue = BERR_IO_ERROR;
        BDBG_ERR(("Failed to write to %s\n", aFilename));
    }

    return returnValue;
}

/*! \brief load srm from file.
 */
static int DtcpAppLib_LoadSrm(char *aSrm, int *aSrmSize)
{
    int returnValue = BERR_SUCCESS;

    if ( DTCP_SRM_SECOND_GEN_MAX_SIZE > *aSrmSize )
    {
        returnValue = BERR_INVALID_PARAMETER;
    }
    else
    {
        const char *srmFn = getenv("DtcpSrmFilePath");
        *aSrmSize = DTCP_SRM_SECOND_GEN_MAX_SIZE;
        if (srmFn != NULL)
        {
            strncpy(sSrmFilename, srmFn, 255);
        } else {
            BDBG_MSG(("%s: DtcpSrmFilePath environment variable not defined. Use default", __FUNCTION__, sSrmFilename));
        }

        BDBG_MSG(("load SRM from %s", sSrmFilename));
        returnValue = DtcpAppLib_LoadBytesFromFile(sSrmFilename, aSrm, aSrmSize);
    }

    BDBG_MSG(("%s - Exiting function --------------------------------", __FUNCTION__));
    return returnValue;
}

#ifdef COMMON_DEVICE_CERT
/*! \brief load Idu from file, only for comman certificate.
 */
static int DtcpAppLib_LoadIDu(char *aIDu, int *aIDuSize)
{
    int returnValue = BERR_SUCCESS;

    if ( DTCP_DEVICE_ID_SIZE > *aIDuSize )
    {
        returnValue = BERR_INVALID_PARAMETER;
    }
    else
    {
        *aIDuSize = DTCP_DEVICE_ID_SIZE;
        returnValue = DtcpAppLib_LoadBytesFromFile(DTCP_IDU_FILENAME, aIDu, aIDuSize);
        if ( returnValue != BERR_SUCCESS || DTCP_DEVICE_ID_SIZE != *aIDuSize)
        {
            returnValue = BERR_IO_ERROR;
        }
    }
    return returnValue;
}
#endif

/*! \brief write srm into file.
 */
static int DtcpAppLib_UpdateSrm(unsigned char *aSrm, unsigned int aSrmSize)
{
    int returnValue = BERR_SUCCESS;

    if ( DTCP_SRM_SECOND_GEN_MAX_SIZE < aSrmSize )
    {
        returnValue = BERR_INVALID_PARAMETER;
    }
    else
    {
        returnValue = DtcpAppLib_SaveBytesToFile( sSrmFilename,
                                                   aSrm, aSrmSize);
        if (returnValue == BERR_SUCCESS)
        {
            BDBG_MSG(("SRM updated!!! Load new SRM values for future AKE's"));
            gSrmSize = DTCP_SRM_SECOND_GEN_MAX_SIZE;
            memset(gSrm, 0, gSrmSize);
            if ((returnValue = DtcpAppLib_LoadSrm(gSrm, &gSrmSize)) != BERR_SUCCESS ) {
                BDBG_ERR(("Failed to load SRM file\n"));
            }
        }
    }
    return returnValue;
}
/*! \brief initialize device parameters, read certificate and SRM from files.
 *  TODO: How is this be safe ?
 */
static int DtcpAppLib_InitializeDeviceParams(B_DeviceParams_T *aDeviceParams,
        B_DTCP_KeyFormat_T key_format)
{
    int returnValue = BERR_SUCCESS;
    DrmRC rc = Drm_Err;
    gSrmSize = DTCP_SRM_SECOND_GEN_MAX_SIZE;
    gCertSize = DTCP_BASELINE_FULL_CERT_SIZE;
    gPrivKeySize = DTCP_PRIVATE_KEY_SIZE;
    /* zero data arrays */
    memset(gRngSeed, 0, gRngSeedSize);
    memset(gSrm, 0, gSrmSize);
    memset(gCert, 0, gCertSize);
    memset(gPrivKey, 0, gPrivKeySize);
    memset(gIDu, 0, gIDuSize);

    BDBG_ENTER(DtcpAppLib_InitializeDeviceParams);
    BSTD_UNUSED(key_format);
    BDBG_MSG(("%s - Entered function ......................\n", __FUNCTION__));

#ifdef COMMON_DEVICE_CERT
    /* Source device is capable of processing Idus */
    aDeviceParams->capability = 1;
    if ((returnValue = DtcpAppLib_LoadIDu(gIDu, &gIDuSize)) != BERR_SUCCESS) {
        BDBG_ERR(("Failed to load IDU file\n"));
        goto error;
    }
    aDeviceParams->CommonDeviceCert = true;
    memcpy(&aDeviceParams->IDu[0], gIDu, gIDuSize);
    aDeviceParams->IDuSize = gIDuSize;
#else
    /* Source device is NOT capable of processing Idus */
    aDeviceParams->capability = 0;
    aDeviceParams->CommonDeviceCert = false;
#endif
#ifdef BRIDGE_DEVICE
    aDeviceParams->BridgeDevice = true;
#else
    aDeviceParams->BridgeDevice = false;
#endif
    if ((returnValue = DtcpAppLib_LoadSrm(gSrm, &gSrmSize)) != BERR_SUCCESS ) {
        BDBG_ERR(("Failed to load SRM file\n"));
        goto error;
    }
    aDeviceParams->Srm = (B_Srm_T *)gSrm;
    aDeviceParams->SrmSize = gSrmSize;
#ifdef DTCP_DEMO_MODE
    BDBG_MSG(("gSrmSize=%d\n", gSrmSize));
    BDBG_MSG(("SRM header\n"));
    BDBG_BUFF((unsigned char*)aDeviceParams->Srm, sizeof(B_Srm_Header_T));
    BDBG_MSG(("CRL entries\n"));
    BDBG_BUFF((unsigned char*)aDeviceParams->Srm->Crl, B_DTCP_GetSrmLength((unsigned char *)gSrm));
#endif

    if((aDeviceParams->Cert = (unsigned char *)BKNI_Malloc(DTCP_BASELINE_FULL_CERT_SIZE) ) == NULL)
    {
        BDBG_ERR(("Faled to allocate memory for device params: size=%d\n", sizeof(B_DeviceParams_T)));
        return rc;
    }
    aDeviceParams->CertSize = DTCP_BASELINE_FULL_CERT_SIZE;
    BKNI_Memset(aDeviceParams->Cert, 0, DTCP_BASELINE_FULL_CERT_SIZE);
    rc = DRM_DtcpIpTl_GetDeviceCertificate(aDeviceParams->hDtcpIpTl, aDeviceParams->Cert, aDeviceParams->CertSize, aDeviceParams->dtlaPublicKey);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s:%d - DRM_DtcpIpTl_GetDeviceCertificate() failed\n",__FUNCTION__,__LINE__));
        return rc;
    }
    else
    {
        /* Get Device public key from certificate */
        memcpy(aDeviceParams->PublicKey, &aDeviceParams->Cert[DTCP_PUBLIC_KEY_OFFSET], DTCP_PUBLIC_KEY_SIZE);
        BDBG_ERR(("%s:%d - DRM_DtcpIpTl_GetDeviceCertificate() succeeded.\n",__FUNCTION__,__LINE__));
    }
error:
    BDBG_LEAVE(DtcpAppLib_InitializeDeviceParams);
    BDBG_MSG(("%s - Exiting function........................................", __FUNCTION__));
    return returnValue;
}

/*!\brief exproted library startup function,must be called before any other AKE function call.
 * \param[in] mode device mode source/sink
 * \param[in] use_pcp_ur true if use PCP_UR flag in PCP header.
 * \param[in] key_format  DTCP-IP key format, e.g. production/test.
 * \param[in] ckc_check Enable content key confirmation procedure for sink device.
 * \param[in] dump  dump extra messages for debugging.
 */
void * DtcpAppLib_Startup(B_DeviceMode_T mode,
        bool use_pcp_ur,
        B_DTCP_KeyFormat_T key_format,
        bool ckc_check)
{
    BERR_Code retValue;
    DrmRC rc = Drm_Err;
    DRM_DtcpIpTlHandle hDtcpIpTl = NULL;
    B_DeviceParams_T * pDeviceParams = NULL;
    B_DTCP_StackHandle_T pStack = NULL;
    char PlatformVersion[30];
    char DtcpSageVersion[40];

    NEXUS_Platform_GetReleaseVersion(PlatformVersion, 30);
    sprintf(DtcpSageVersion, "%s %s", PlatformVersion, DTCP_IP_SAGE_VERSION);
    BDBG_WRN(("******DTCP-IP SAGE Lib Release Version ## %s", DtcpSageVersion));

    if(key_format != B_DTCP_KeyFormat_eCommonDRM)
    {
        BDBG_ERR(("****This version of DTCP-IP library with SAGE does not support this configuration****"));
        BDBG_ERR(("****Currently supported Key format <B_DTCP_KeyFormat_eCommonDRM>****"));
        return NULL;
    }

    if((pDeviceParams = BKNI_Malloc(sizeof(B_DeviceParams_T)) ) == NULL)
    {
        BDBG_ERR(("Faled to allocate memory for device params: size=%d\n", sizeof(B_DeviceParams_T)));
        return NULL;
    }
    BKNI_Memset(pDeviceParams, 0, sizeof(B_DeviceParams_T));
    if (use_pcp_ur == true)
        pDeviceParams->pcp_ur_capability = 1;
    else
        pDeviceParams->pcp_ur_capability = 0;
    pDeviceParams->Mode = mode;

    /* Init the SAGE DTCP module, parse DRM.bin*/
    rc = DRM_DtcpIpTl_Initialize("./drm.bin", (int)mode, (DRM_DtcpIpTlHandle *) &hDtcpIpTl);
    if(rc != Drm_Success)
    {
        printf("%s:%d - DRM_DtcpIpTl_Initialize() failed\n",__FUNCTION__,__LINE__);
        return NULL;
    }
    else
    {
        printf("%s:%d - DRM_DtcpIpTl_Initialize() succeeded.\n",__FUNCTION__,__LINE__);
    }

    pDeviceParams->hDtcpIpTl = hDtcpIpTl;          /* Store DTCP thin layer handle*/
    if ((retValue = DtcpAppLib_InitializeDeviceParams(pDeviceParams, key_format)) != BERR_SUCCESS )
    {
        BDBG_ERR(("Failed to initialize device params: retValue=%d\n", retValue));
        goto error1;
    }

    if ((pStack = B_DTCP_Stack_Init(pDeviceParams, &DtcpAppLib_UpdateSrm, B_StackId_eIP)) == NULL)
    {
        BDBG_ERR(("Failed to initialize DTCP-IP stack\n"));
        goto error1;
    }
    pStack->pAkeCoreData->ckc_check = ckc_check;
    pStack->key_format = key_format;
    pStack->pAkeCoreData->pStack = pStack;   /* Store a pointer to stack data for future refernce .*/

    return (void*)pStack;

error1:
    B_DTCP_Stack_UnInit(pStack);
    BKNI_Free(pDeviceParams);
    return NULL;
}

/*Returns the DTCPIP cert used by the local lib*/
int DtcpAppLib_GetDtcpCert(unsigned char *pLocalCert, unsigned int *pLocalCertSize)
{
    int returnValue = BERR_SUCCESS;
    BDBG_ASSERT(pLocalCert);

    memcpy(pLocalCert, gCert, gCertSize);
    *pLocalCertSize = gCertSize;

    return returnValue;
}
