/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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

 ******************************************************************************/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/crypto.h>
#include <pthread.h>

#define MIN( a, b ) (( a>b ) ? b : a )

static bool            g_mutex_init = false;
static pthread_mutex_t g_mutex_lock;

int bmon_compute_sha256(
    const char *inbuffer,
    int         inbuffer_len,
    char       *sha256OutBuffer
    )
{
    int           rc         = 0;
    unsigned int  i          = 0;
    EVP_MD_CTX   *mdctx      = NULL;
    const EVP_MD *md         = NULL;
    unsigned int  md_len     = 0;
    unsigned int  offset     = 0;
    unsigned int  update_len = 0;
    unsigned char md_value[EVP_MAX_MD_SIZE];

    do {
        if (g_mutex_init == false )
        {
            /** create mutex ***/
            rc = pthread_mutex_init( &g_mutex_lock, NULL );
            if (rc)
            {
                fprintf( stderr, "%s - pthread_mutex_init() Failed. \n", __FUNCTION__ );
                break;
            }
            g_mutex_init = true;
        }

        rc = pthread_mutex_lock( &g_mutex_lock );
        if (rc)
        {
            fprintf( stderr, "%s - pthread_mutex_lock() Failed \n", __FUNCTION__ );
            break;
        }

        OpenSSL_add_all_digests();

        md = EVP_get_digestbyname( "sha256" );

        if (!md)
        {
            fprintf( stderr, "%s - Could not find digest sha256 ... EVP_get_digestbyname() Failed \n", __FUNCTION__ );
            break;
        }
        mdctx = EVP_MD_CTX_create();
        //fprintf( stderr, "SHA512_DIGEST_LENGTH %u\n", SHA512_DIGEST_LENGTH );
        //fprintf( stderr, "SHA256_DIGEST_LENGTH %u\n", SHA256_DIGEST_LENGTH );
        //fprintf( stderr, "%s - inbuffer %p ... len %d \n", __FUNCTION__, inbuffer, inbuffer_len );

        EVP_DigestInit_ex( mdctx, md, NULL );
        do {
            update_len = MIN( EVP_MAX_MD_SIZE, inbuffer_len - offset );
            //fprintf( stderr, "%s - inbuffer %p ... offset %u ... update_len %u\n", __FUNCTION__, inbuffer, offset, update_len );
            EVP_DigestUpdate( mdctx, &inbuffer[offset], update_len );
            offset += EVP_MAX_MD_SIZE;
        } while (offset < inbuffer_len);
        EVP_DigestFinal_ex( mdctx, md_value, &md_len );
        EVP_MD_CTX_destroy( mdctx );
        //fprintf( stderr, "%s - inbuffer %p ... offset %u ... update_len %u\n", __FUNCTION__, inbuffer, offset, update_len );

        for (i = 0; i < md_len; i++) {
            sprintf( &( sha256OutBuffer[i*2] ), "%02x", md_value[i] );
            //fprintf( stderr, "%02x ", md_value[i] );
        }
    } while (0);
    /* Call this once before exit. */
    EVP_cleanup();

    rc = pthread_mutex_unlock( &g_mutex_lock );
    if (rc)
    {
        fprintf( stderr, "%s - pthread_mutex_unlock() Failed \n", __FUNCTION__ );
    }

    //fprintf( stderr, "%s - returning (%s)\n", __FUNCTION__, sha256OutBuffer );
    return( 0 );
}                                                          /* bmon_compute_sha256 */
