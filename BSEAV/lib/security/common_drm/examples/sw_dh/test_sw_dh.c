/***************************************************************************
 *    (c)2013 Broadcom Corporation
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
 *   test Diffie Hellman software implementation from Common DRM
 *   This application have two modes:
 *   - self testing without giving any peer public key
 *   - exchange testing, by providing a peer public key
 * 
 * Revision History:
 * 
 * 
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "nexus_platform.h"
#include "drm_types.h"
#include "drm_common.h"

/* due to drm_common_swcrypto.h, we need
 * to have X509 openssl's struct defined.
 * We dont want any openssl in here.
 * #include <openssl/x509.h>
 * is replaced by X509 preproc makeup
 */
#define X509 void

#include "drm_common_swcrypto.h"
#include "drm_common_swcrypto_types.h"

/* OpenSSL> gendh
   Generating DH parameters
   512 bits long safe prime:
   009c9891b95d9866325b466eb26cd523860c253f17973081b09c3899176b4310f85535952a36c54713b3648f2fbc5b36a59f5f787b5f4656985cca04e8b368720b
   generator:
   2
*/
const char * preLoadedPem = 
    "-----BEGIN DH PARAMETERS-----\n"
    "MEYCQQCcmJG5XZhmMltGbrJs1SOGDCU/F5cwgbCcOJkXa0MQ+FU1lSo2xUcTs2SP\n"
    "L7xbNqWfX3h7X0ZWmFzKBOizaHILAgEC\n"
    "-----END DH PARAMETERS-----\n";

/* Test Software Diffie Hellman structures definitions */
typedef struct DH_test_key_st {
    const char * data;
    int len;
} DH_data;

typedef struct DH_test_st {
    void * handle;
    DH_data publicKey;
    DH_data sharedSecret;
} DH_test;

/* Function to initialize the platform with default settings */
static void _initPlatform(void)
{
    NEXUS_PlatformSettings platformSettings;

    NEXUS_Platform_GetDefaultSettings(&platformSettings); 
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
}

/* Take a binary buffer and print it in hexadecimal */
static void _Print_Buffer(char *pTitle,
                          const char *pBuffer,
                          int uiSize,
                          int pretty)
{
    int byteNo;

    if (pTitle) {
        printf("%s: ", pTitle);
    }
    if (pretty) {
        for(byteNo = 0; byteNo < uiSize; byteNo++)
        {
            unsigned char c = pBuffer[byteNo];
            if((byteNo % 4) == 0)
            {
                printf(" ");
            }
            if((byteNo % 16) == 0)
            {
                printf("\n");
            }
            printf("%02x", c);
        }
    }
    else {
        for(byteNo = 0; byteNo < uiSize; byteNo++)
        {
            unsigned char c = pBuffer[byteNo];
            printf("%02x", c);
        }
    }
    printf("\n");

    return;
}

/* Convert an hexadecimal printable char to its binary value */
static char _hexCharToBin(char h)
{
    if (h >= '0' && h <= '9') {
        return h - '0';
    }
    if (h >= 'A' && h <= 'F') {
        return (h - 'A') + 10;
    }
    if (h >= 'a' && h <= 'f') {
        return (h - 'a') + 10;
    }
    return -1;
}

/* Dump an hexadecimal string into a binary array
 * ex. "0affff" -> {0x10, 0x255, 0x255} */
static int _Dump_HexStrToBin(const char *str, DH_data * dest)
{
    char * tmp = NULL;
    unsigned int tmpLen = 0;
    size_t len;
    unsigned int i, j;

    if (!str || !dest) {
        goto end;
    }

    len = strlen(str);
    if (len % 2) {
        goto end;
    }

    tmpLen = len/2;
    tmp = malloc(tmpLen + 1);
    for (i = 0, j = 0; i < len; i += 2, j++) {
        char c1, c2;
        if (j >= tmpLen) {
            goto end;
        }
        c1  = _hexCharToBin(str[i]);
        c2  = _hexCharToBin(str[i+1]);
        if (c1 < 0 || c2 < 0) {
            goto end;
        }
        tmp[j] = (c1 << 4) + c2;
    }
    tmp[tmpLen] = 0;

    /* Success: eat up tmp */
    dest->data = tmp;
    dest->len = tmpLen;
    tmp = NULL;
end:
    if (tmp) {
        /* error */
        free(tmp);
        return 1;
    }
    return 0;
}

/* load a PEM content from a given file */
static char * _load_pem_file(const char * path)
{    
    char * ret = NULL;
    char * pem = NULL;
    off_t off;
    ssize_t pemLen;
    int fd = -1;

    /* extract PEM string from file */
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "cannot open %s\n", path);
        goto end;
    }
    off = lseek(fd, 0, SEEK_END);
    if (off < 0 || off > SSIZE_MAX){
        fprintf(stderr, "cannot determine %s size\n", path);
        goto end;
    }
    pemLen = (ssize_t)off;
    pem = (char *)malloc(off+1);
    if (!pem) {
        fprintf(stderr, "cannot allocate %d bytes for pem\n", (int)off);
        goto end;
    }
    if (lseek(fd, 0, SEEK_SET) != 0) {
        fprintf(stderr, "cannot rewind\n");
        goto end;
    }
    if (read(fd, pem, pemLen) != pemLen) {
        fprintf(stderr, "read failure\n");
        goto end;
    }
    /* success: eat-up pem */
    ret = pem;
    pem = NULL;
end:
    if (pem) {
        free(pem);
    }
    if (fd >=0) {
        close(fd);
    }
    return ret;
}

/* reset a DH_test structure to default */
static void _DH_test_reset(DH_test *d)
{
    BKNI_Memset(d, 0, sizeof(*d));
}

/* init a DH_test structure using DH_data from PEM */
static DrmRC _DH_test_init(DH_test * d, DH_data *pem)
{
    DrmRC rc;

    /* Allocate Diffie Hellman instance */
    rc = DRM_Common_SwDHFromPem(pem->data, pem->len, &d->handle);
    if (rc != Drm_Success) {
        fprintf(stderr, "DRM_Common_SwDHFromPem(%p) failure (%d)\n", (void *)d, rc);
        goto end;
    }

    /* Get public key */
    rc = DRM_Common_SwDHGetPublicKey(d->handle, (uint8_t **)&d->publicKey.data, &d->publicKey.len);
    if (rc != Drm_Success) {
        fprintf(stderr, "DRM_Common_SwDHGetPublicKey(%p) failure (%d)\n", (void *)d, rc);
        goto end;
    }

end:
    return rc;
}

/* cleanup (release resources) of a DH_data structure */
static void _DH_data_cleanup(DH_data *d)
{
    if (!d){
        return;
    }
    if (d->data) {
        free((void *)d->data);
        d->data = NULL;
        d->len = 0;
    }
}

/* cleanup (release resources) of a DH_test structure */
static void _DH_test_cleanup(DH_test *d)
{
    if (!d){
        return;
    }
    if (d->handle) {
        DRM_Common_SwDHCleanup(d->handle);
    }
    else {
        _DH_data_cleanup(&d->publicKey);
        _DH_data_cleanup(&d->sharedSecret);
    }
}

/* compute DH shared secret using a DH instance and peer public key */
static DrmRC _compute_shared_DH_test(DH_test * d, DH_test * peer)
{
    DrmRC rc;

    if (!d->handle) {
        /* not a local instance */
        return Drm_Success;
    }

    /* Use peer public   key to compute shared secret */
    rc = DRM_Common_SwDHComputeSharedSecret(d->handle, (uint8_t *)peer->publicKey.data, peer->publicKey.len,
                                            (uint8_t **)&d->sharedSecret.data, &d->sharedSecret.len);
    if (rc != Drm_Success) {
        fprintf(stderr, "DRM_Common_SwDHComputeSharedSecret() failure (%d)\n", rc);
    }
    return rc;
}

/* compare two DH instance shared secrets */
static DrmRC _compare_secrets_DH_tests(DH_test * d, DH_test *peer)
{
    DrmRC rc = Drm_Success;

    /* compare secrets */
    if (d->sharedSecret.len != peer->sharedSecret.len ||
        memcmp(d->sharedSecret.data, peer->sharedSecret.data, d->sharedSecret.len)) {
        fprintf(stderr, "Shared secrets are diferent (%u bytes) %p != %p (%u bytes) (err %d)\n",
                d->sharedSecret.len, d->sharedSecret.data,
                peer->sharedSecret.data, peer->sharedSecret.len, rc);
        _Print_Buffer("DH1  Shared Secret", d->sharedSecret.data, d->sharedSecret.len, 1);
        _Print_Buffer("peer Shared Secret", peer->sharedSecret.data, peer->sharedSecret.len, 1);
        rc = Drm_Err;
    }
    return rc;
}

/* print usage on stderr */
static void _usage(const char *bin)
{
    fprintf(stderr, "\nUsages:\n\n");
    fprintf(stderr, "%s [DH parameter file in PEM format]\n\n", bin);
    fprintf(stderr, "%s <DH parameter file in PEM format> <peer public key in hex>\n\n", bin);
}

/* main entry point 
 * see usage() for command line arguments
 *
 * DRM_Common_SwDH* API usage is as follow:      
 * step 1 - DRM_Common_SwDHFromPem() to initialize a DH instance
 *          with DH parameters from PEM
 * step 2 - DRM_Common_SwDHGetPublicKey() to retrieve DH public key
 * step 3 - DRM_Common_SwDHComputeSharedSecret() to compute the shared secret,
 *          given a peer public key
 */
int main(int argc, char* argv[])
{
    DrmRC rc = Drm_Success;

    char * filePem = NULL;

    DH_test DH1;
    DH_test DHpeer;

    DH_data pem;
    DrmCommonInit_t commonDrmInit;

    BKNI_Memset((uint8_t*)&commonDrmInit, 0x00, sizeof(DrmCommonInit_t));

    _DH_test_reset(&DH1);
    _DH_test_reset(&DHpeer);

    /* Verify parameter */
    if(argc > 3) {
        rc = Drm_InvalidParameter;
        _usage(argv[0]);
        goto end;
    }

    /* Initializes the platform (memory heaps etc) */
    _initPlatform();

    /* load PEM data, either from a file or from preload (static definition) */
    if (argc >= 2) {
        filePem = _load_pem_file(argv[1]);
        if (!filePem) {
            rc = Drm_InvalidParameter;
            fprintf(stderr, "Cannot load PEM from file\n");
            goto end;
        }
        pem.data = filePem;
        printf("Using pem from file %s\n", argv[1]);
    }
    else
    {
        /* Use Pre loaded PEM string */
        pem.data = preLoadedPem;
        printf("Using preloaded pem\n");
    }

    pem.len = strlen(pem.data);
    printf("PEM data (%d bytes):\n%.*s\n", pem.len, pem.len, pem.data);

    /* Initialize DRM common capabilities */
    rc = DRM_Common_BasicInitialize(&commonDrmInit);
    if (rc != Drm_Success) {
        fprintf(stderr, "DRM_Common_BasicInitialize() failure (%d)\n", rc);
        goto end;
    }

    /* step 1 : initialize first DH instance using PEM DH parameters
     * step 2 : get public key 
     * uses DRM_Common_SwDHFromPem(), DRM_Common_SwDHGetPublicKey() */
    if (_DH_test_init(&DH1, &pem) != Drm_Success) {
        goto end;
    }
    _Print_Buffer("DH1 Public Key ", DH1.publicKey.data, DH1.publicKey.len, 0);

    /* initialize second DH instance using either PEM DH parameters or given public key */
    if (argc >= 3) {
        if (_Dump_HexStrToBin(argv[2], &DHpeer.publicKey)) {
            goto end;
        }
    }
    else {
        if (_DH_test_init(&DHpeer, &pem) != Drm_Success) {
            goto end;
        }
    }
    _Print_Buffer("peer Public Key", DHpeer.publicKey.data, DHpeer.publicKey.len, 0);

    /* step 3 : compute shared keys if possible
     * uses DRM_Common_SwDHComputeSharedSecret() */
    if (_compute_shared_DH_test(&DH1, &DHpeer) != Drm_Success) {
        goto end;
    }
    if (_compute_shared_DH_test(&DHpeer, &DH1) != Drm_Success) {
        goto end;
    }

    /* compare resulting shared secrets if both are set */
    if (DHpeer.sharedSecret.data) {
        if (_compare_secrets_DH_tests(&DH1, &DHpeer) != Drm_Success) {
            goto end;
        }
    }

    /* success */
    _Print_Buffer("Shared Secret ", DH1.sharedSecret.data, DH1.sharedSecret.len, 0);
    printf("\n%s succeed\n", argv[0]);

end:
    /* cleanup in reverse init order */

    _DH_test_cleanup(&DH1);
    _DH_test_cleanup(&DHpeer);

    if (filePem) {
        free(filePem);
    }

    DRM_Common_Finalize();
    NEXUS_Platform_Uninit();

    if (rc == Drm_Success) {
        return 0;
    }
    return -1;
}

