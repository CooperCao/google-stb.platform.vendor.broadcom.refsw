/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include "keymaster_debug.h"
#include "bstd.h"
#include "bkni.h"

#include "openssl/x509.h"
#include "openssl/x509_vfy.h"
#include "openssl/md5.h"

#include "nexus_platform.h"
#include "nexus_platform_init.h"

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

#include "keymaster_ids.h"
#include "keymaster_platform.h"
#include "gatekeeper_tl.h"
#include "keymaster_test.h"


BDBG_MODULE(gatekeeper_test);

#ifndef NXCLIENT_SUPPORT
static BERR_Code SAGE_app_join_nexus(void);
static void SAGE_app_leave_nexus(void);

static NEXUS_Error SAGE_app_join_nexus(void)
{
    NEXUS_Error err = 0;
    NEXUS_PlatformSettings platformSettings;

    BDBG_LOG(("\t*** Bringing up all Nexus modules for platform using default settings\n\n\n"));

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;

    err = NEXUS_Platform_Init(&platformSettings);
    if (err != NEXUS_SUCCESS) {
        BDBG_ERR(("\t### Failed to bring up Nexus\n\n"));
    }
    return err;
}

static void SAGE_app_leave_nexus(void)
{
    NEXUS_Platform_Uninit();
}
#endif


static void gk_print_buffer(const char *func, const char *tag, const uint8_t *buffer, size_t buf_size)
{
    char *p_buf = NULL;
    char *p;
    size_t i;

    p_buf = (char *)malloc(buf_size * 2 + 1);
    if (!p_buf) {
        BDBG_ERR(("%s: failed to malloc buffer", BSTD_FUNCTION));
        exit(1);
    }
    p = p_buf;
    for (i = 0; i < buf_size; i++) {
        p += snprintf(p, (buf_size - i) * 2 + 1, "%02x", buffer[i]);
    }
    BDBG_LOG(("%s: %s %s", func, tag, p_buf));
    free(p_buf);
}

static BERR_Code gk_basic_tests(GatekeeperTl_Handle handle)
{
    BERR_Code err = BERR_UNKNOWN;
    gk_password_handle_t pass_handle;
    gk_password_handle_t pass_handle2;
    GatekeeperTl_Password pass;
    GatekeeperTl_Password pass2;
    char password[] = "good password";
    char password2[] = "second password";
    uint32_t uid = 1;
    uint64_t challenge = 52ull;
    uint64_t challenge2 = 42ull;
    uint32_t retry_timeout = 0;
    km_hw_auth_token_t auth_token;
    uint64_t timestamp;

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    memset(&pass, 0, sizeof(pass));
    memset(&pass2, 0, sizeof(pass2));
    TEST_ALLOCATE_BLOCK(pass, strlen(password));
    memcpy(pass.buffer, password, strlen(password));
    TEST_ALLOCATE_BLOCK(pass2, strlen(password2));
    memcpy(pass2.buffer, password2, strlen(password2));

    BDBG_LOG(("%s: enroll password", BSTD_FUNCTION));
    EXPECT_SUCCESS(GatekeeperTl_Enroll(handle, uid, NULL, NULL, &pass, NULL, &pass_handle));
    if (!pass_handle.hardware_backed || !pass_handle.user_id) {
        BDBG_ERR(("%s: unexpected return in password handle", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }
    BDBG_LOG(("%s: password handle UID: %llx", BSTD_FUNCTION, (unsigned long long)pass_handle.user_id));
    gk_print_buffer(BSTD_FUNCTION, "password handle signature", pass_handle.signature, sizeof(pass_handle.signature));
    BDBG_LOG(("%s: enroll password PASSED", BSTD_FUNCTION));

    BDBG_LOG(("%s: verify password", BSTD_FUNCTION));
    EXPECT_SUCCESS(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass, &retry_timeout, &auth_token));
    if ((auth_token.user_id != pass_handle.user_id) ||
        (auth_token.challenge != challenge) ||
        !auth_token.authenticator_id ||
        (auth_token.authenticator_type != htonl(((uint32_t)SKM_HW_AUTH_PASSWORD))) ||
        !auth_token.timestamp) {
        BDBG_ERR(("%s: unexpected return in auth token", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }

    /* Gatekeeper specification says timestamp is in BE, so fix it */
    timestamp = auth_token.timestamp;
    timestamp = GK_UINT64_SWAP(timestamp);
    BDBG_LOG(("%s: auth token ID: %llx timestamp:%llx", BSTD_FUNCTION, (unsigned long long)auth_token.authenticator_id, (unsigned long long)timestamp));
    gk_print_buffer(BSTD_FUNCTION, "          auth token HMAC", auth_token.hmac, sizeof(auth_token.hmac));
    BDBG_LOG(("%s: verify password PASSED", BSTD_FUNCTION));

    BDBG_LOG(("%s: verify with bad password data", BSTD_FUNCTION));
    memset(&auth_token, 0, sizeof(auth_token));
    EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass2, &retry_timeout, &auth_token), BSAGE_ERR_KM_RETRY);
    if (retry_timeout ||   /* first fail should have retry of 0 */
        (auth_token.user_id == pass_handle.user_id) ||
        (auth_token.challenge == challenge) ||
        (auth_token.authenticator_type == htonl(((uint32_t)SKM_HW_AUTH_PASSWORD))) ||
        auth_token.timestamp) {
        BDBG_ERR(("%s: unexpected return in auth token failure test", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }
    BDBG_LOG(("%s: verify with bad password data PASSED", BSTD_FUNCTION));

    BDBG_LOG(("%s: enroll password to change password (wrong enrolled password)", BSTD_FUNCTION));
    EXPECT_FAILURE_CODE(GatekeeperTl_Enroll(handle, uid, &pass_handle, &pass2, &pass2, &retry_timeout, &pass_handle), BSAGE_ERR_KM_RETRY);
    if (retry_timeout) {
        /* Second fail should have retry of 0 */
        BDBG_ERR(("%s: unexpected return in auth token failure test", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }
    BDBG_LOG(("%s: enroll password to change password (wrong enrolled password) PASSED", BSTD_FUNCTION));

    BDBG_LOG(("%s: enroll password to change password", BSTD_FUNCTION));
    EXPECT_SUCCESS(GatekeeperTl_Enroll(handle, uid, &pass_handle, &pass, &pass2, &retry_timeout, &pass_handle2));
    if (retry_timeout) {
        /* After success, should have retry of 0 */
        BDBG_ERR(("%s: unexpected return in auth token failure test", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }
    BDBG_LOG(("%s: password handle UID: %llx", BSTD_FUNCTION, (unsigned long long)pass_handle2.user_id));
    gk_print_buffer(BSTD_FUNCTION, "password handle signature", pass_handle2.signature, sizeof(pass_handle2.signature));
    BDBG_LOG(("%s: enroll password to change password PASSED", BSTD_FUNCTION));

    BDBG_LOG(("%s: verify password with old password", BSTD_FUNCTION));
    memset(&auth_token, 0, sizeof(auth_token));
    EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle2, &pass, &retry_timeout, &auth_token), BSAGE_ERR_KM_RETRY);
    BDBG_LOG(("%s: verify password with old password PASSED", BSTD_FUNCTION));

    BDBG_LOG(("%s: verify password with new password", BSTD_FUNCTION));
    memset(&auth_token, 0, sizeof(auth_token));
    EXPECT_SUCCESS(GatekeeperTl_Verify(handle, uid, challenge2, &pass_handle2, &pass2, &retry_timeout, &auth_token));
    gk_print_buffer(BSTD_FUNCTION, "          auth token HMAC", auth_token.hmac, sizeof(auth_token.hmac));
    BDBG_LOG(("%s: verify password with new password PASSED", BSTD_FUNCTION));

    BDBG_LOG(("%s: verify old password handle with old password", BSTD_FUNCTION));
    memset(&auth_token, 0, sizeof(auth_token));
    EXPECT_SUCCESS(GatekeeperTl_Verify(handle, uid, challenge2, &pass_handle, &pass, &retry_timeout, &auth_token));
    gk_print_buffer(BSTD_FUNCTION, "          auth token HMAC", auth_token.hmac, sizeof(auth_token.hmac));
    BDBG_LOG(("%s: verify old password handle with old password PASSED", BSTD_FUNCTION));

    err = BERR_SUCCESS;
    BDBG_LOG(("%s: tests PASSED", BSTD_FUNCTION));

done:
    TEST_FREE_BLOCK(pass);
    TEST_FREE_BLOCK(pass2);
    return err;
}

#define THIRTY_SECONDS_IN_MS   (30000)
#define NUM_TIMEOUT_TESTS       12

static BERR_Code gk_timeout_tests(GatekeeperTl_Handle handle)
{
    BERR_Code err = BERR_UNKNOWN;
    gk_password_handle_t pass_handle;
    GatekeeperTl_Password pass;
    GatekeeperTl_Password pass2;
    char password[] = "good password";
    char password2[] = "second password";
    uint32_t uid = 1;
    uint64_t challenge = 52ull;
    uint32_t retry_timeout = 0;
    km_hw_auth_token_t auth_token;
    int i;
    uint32_t expected_timeout[NUM_TIMEOUT_TESTS] =
                { 0, 0, 0, 0, THIRTY_SECONDS_IN_MS, 0, 0, 0, 0, THIRTY_SECONDS_IN_MS, THIRTY_SECONDS_IN_MS, THIRTY_SECONDS_IN_MS };

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    memset(&pass, 0, sizeof(pass));
    memset(&pass2, 0, sizeof(pass2));
    TEST_ALLOCATE_BLOCK(pass, strlen(password));
    memcpy(pass.buffer, password, strlen(password));
    TEST_ALLOCATE_BLOCK(pass2, strlen(password2));
    memcpy(pass2.buffer, password2, strlen(password2));

    BDBG_LOG(("%s: enroll password", BSTD_FUNCTION));
    EXPECT_SUCCESS(GatekeeperTl_Enroll(handle, uid, NULL, NULL, &pass, NULL, &pass_handle));
    if (!pass_handle.hardware_backed || !pass_handle.user_id) {
        BDBG_ERR(("%s: unexpected return in password handle", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }
    BDBG_LOG(("%s: password handle UID: %llx", BSTD_FUNCTION, (unsigned long long)pass_handle.user_id));
    gk_print_buffer(BSTD_FUNCTION, "password handle signature", pass_handle.signature, sizeof(pass_handle.signature));
    BDBG_LOG(("%s: enroll password PASSED", BSTD_FUNCTION));

    BDBG_LOG(("%s: verify password", BSTD_FUNCTION));
    memset(&auth_token, 0, sizeof(auth_token));
    EXPECT_SUCCESS(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass, &retry_timeout, &auth_token));
    gk_print_buffer(BSTD_FUNCTION, "          auth token HMAC", auth_token.hmac, sizeof(auth_token.hmac));
    BDBG_LOG(("%s: verify password PASSED", BSTD_FUNCTION));

    BDBG_LOG(("%s: verify with bad password data", BSTD_FUNCTION));
    for (i = 0; i < NUM_TIMEOUT_TESTS; i++) {
        BDBG_LOG(("%s: iteration %d", BSTD_FUNCTION, i));
        memset(&auth_token, 0, sizeof(auth_token));
        EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass2, &retry_timeout, &auth_token), BSAGE_ERR_KM_RETRY);
        if (retry_timeout != expected_timeout[i]) {
            BDBG_ERR(("%s: expected timeout value %u got %u", BSTD_FUNCTION, expected_timeout[i], retry_timeout));
        }
        if (retry_timeout) {
            /*
             * We should be able to guess multiple times without affecting failure count,
             * because we are in the retry timeout window and the password is not actually
             * tested - the function just exits.
             */
            EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass2, NULL, &auth_token), BSAGE_ERR_KM_RETRY);
            EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass2, NULL, &auth_token), BSAGE_ERR_KM_RETRY);
            EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass2, NULL, &auth_token), BSAGE_ERR_KM_RETRY);
            EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass2, NULL, &auth_token), BSAGE_ERR_KM_RETRY);
            EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass2, NULL, &auth_token), BSAGE_ERR_KM_RETRY);

            /* Sleep for required time and then go around for next iteration */
            BDBG_LOG(("%s: iteration %d waiting for timeout %d seconds", BSTD_FUNCTION, i, THIRTY_SECONDS_IN_MS / 1000 + 1));
            sleep(THIRTY_SECONDS_IN_MS / 1000 + 1);
        }
    }
    BDBG_LOG(("%s: verify with bad password data PASSED", BSTD_FUNCTION));

    BDBG_LOG(("%s: verify password", BSTD_FUNCTION));

    /* At the end of the last tests, we have to wait for timeout period before guessing correctly */
    BDBG_LOG(("%s: waiting for timeout %d seconds", BSTD_FUNCTION, THIRTY_SECONDS_IN_MS / 1000 + 1));
    sleep(THIRTY_SECONDS_IN_MS / 1000 + 1);

    memset(&auth_token, 0, sizeof(auth_token));
    EXPECT_SUCCESS(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass, &retry_timeout, &auth_token));
    gk_print_buffer(BSTD_FUNCTION, "          auth token HMAC", auth_token.hmac, sizeof(auth_token.hmac));
    BDBG_LOG(("%s: verify password PASSED", BSTD_FUNCTION));

    /* Correct password resets the counter so we can repeat the timeout tests in another way */
    BDBG_LOG(("%s: verify with bad password data", BSTD_FUNCTION));
    for (i = 0; i < NUM_TIMEOUT_TESTS - 2; i++) {
        BDBG_LOG(("%s: iteration %d", BSTD_FUNCTION, i));
        memset(&auth_token, 0, sizeof(auth_token));
        if (expected_timeout[i]) {
            /* Sleep before we guess to prove the guess will be tested */
            BDBG_LOG(("%s: iteration %d waiting for timeout %d seconds", BSTD_FUNCTION, i, THIRTY_SECONDS_IN_MS / 1000 + 1));
            sleep(THIRTY_SECONDS_IN_MS / 1000 + 1);
        }
        EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass2, &retry_timeout, &auth_token), BSAGE_ERR_KM_RETRY);
    }
    BDBG_LOG(("%s: verify with bad password data PASSED", BSTD_FUNCTION));

    BDBG_LOG(("%s: verify password", BSTD_FUNCTION));

    /* At the end of the last tests, we have to wait for timeout period before guessing correctly */
    BDBG_LOG(("%s: waiting for timeout %d seconds", BSTD_FUNCTION, THIRTY_SECONDS_IN_MS / 1000 + 1));
    sleep(THIRTY_SECONDS_IN_MS / 1000 + 1);

    memset(&auth_token, 0, sizeof(auth_token));
    EXPECT_SUCCESS(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass, &retry_timeout, &auth_token));
    gk_print_buffer(BSTD_FUNCTION, "          auth token HMAC", auth_token.hmac, sizeof(auth_token.hmac));
    BDBG_LOG(("%s: verify password PASSED", BSTD_FUNCTION));

    err = BERR_SUCCESS;
    BDBG_LOG(("%s: tests PASSED", BSTD_FUNCTION));

done:
    TEST_FREE_BLOCK(pass);
    TEST_FREE_BLOCK(pass2);
    return err;
}

static BERR_Code gk_enroll_parameter_tests(GatekeeperTl_Handle handle)
{
    BERR_Code err = BERR_UNKNOWN;
    gk_password_handle_t pass_handle;
    GatekeeperTl_Password pass;
    GatekeeperTl_Password pass2;
    GatekeeperTl_Password pass3;
    char password[] = "good password";
    uint32_t uid = 1;

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    memset(&pass, 0, sizeof(pass));
    TEST_ALLOCATE_BLOCK(pass, strlen(password));
    memcpy(pass.buffer, password, strlen(password));
    TEST_ALLOCATE_BLOCK(pass3, GATEKEEPER_MAX_PASSWORD + 1);
    memset(pass3.buffer, 'a', pass3.size);

    /* NULL handle */
    EXPECT_FAILURE_CODE(GatekeeperTl_Enroll(NULL, uid, NULL, NULL, &pass, NULL, &pass_handle), BERR_INVALID_PARAMETER);
    /* NULL provided password */
    EXPECT_FAILURE_CODE(GatekeeperTl_Enroll(handle, uid, NULL, NULL, NULL, NULL, &pass_handle), BERR_INVALID_PARAMETER);
    /* Provided password too big */
    EXPECT_FAILURE_CODE(GatekeeperTl_Enroll(handle, uid, NULL, NULL, &pass3, NULL, &pass_handle), BERR_INVALID_PARAMETER);
    /* NULL output pass handle */
    EXPECT_FAILURE_CODE(GatekeeperTl_Enroll(handle, uid, NULL, NULL, &pass, NULL, NULL), BERR_INVALID_PARAMETER);

    /* Invalid provided password structure */
    pass2.buffer = NULL;
    pass2.size = pass.size;
    EXPECT_FAILURE_CODE(GatekeeperTl_Enroll(handle, uid, NULL, NULL, &pass2, NULL, &pass_handle), BERR_INVALID_PARAMETER);
    /* Invalid provided password structure */
    pass2.buffer = pass.buffer;
    pass2.size = 0;
    EXPECT_FAILURE_CODE(GatekeeperTl_Enroll(handle, uid, NULL, NULL, &pass2, NULL, &pass_handle), BERR_INVALID_PARAMETER);

    /* Incoming pass handle with no enroll password */
    EXPECT_FAILURE_CODE(GatekeeperTl_Enroll(handle, uid, &pass_handle, NULL, &pass, NULL, &pass_handle), BERR_INVALID_PARAMETER);

    /* Invalid enroll password structure */
    pass2.buffer = NULL;
    pass2.size = pass.size;
    EXPECT_FAILURE_CODE(GatekeeperTl_Enroll(handle, uid, &pass_handle, &pass2, &pass, NULL, &pass_handle), BERR_INVALID_PARAMETER);
    /* Invalid enroll password structure */
    pass2.buffer = pass.buffer;
    pass2.size = 0;
    EXPECT_FAILURE_CODE(GatekeeperTl_Enroll(handle, uid, &pass_handle, &pass2, &pass, NULL, &pass_handle), BERR_INVALID_PARAMETER);
    /* Enroll password too big */
    EXPECT_FAILURE_CODE(GatekeeperTl_Enroll(handle, uid, &pass_handle, &pass3, &pass, NULL, &pass_handle), BERR_INVALID_PARAMETER);

    BDBG_LOG(("%s: all tests passed", BSTD_FUNCTION));
    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(pass);
    TEST_FREE_BLOCK(pass3);
    return err;
}

static BERR_Code gk_verify_parameter_tests(GatekeeperTl_Handle handle)
{
    BERR_Code err = BERR_UNKNOWN;
    gk_password_handle_t pass_handle;
    km_hw_auth_token_t auth_token;
    GatekeeperTl_Password pass;
    GatekeeperTl_Password pass2;
    GatekeeperTl_Password pass3;
    char password[] = "good password";
    uint32_t uid = 1;
    uint64_t challenge = 42ull;

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    memset(&pass, 0, sizeof(pass));
    TEST_ALLOCATE_BLOCK(pass, strlen(password));
    memcpy(pass.buffer, password, strlen(password));
    TEST_ALLOCATE_BLOCK(pass3, GATEKEEPER_MAX_PASSWORD + 1);
    memset(pass3.buffer, 'a', pass3.size);

    /* NULL handle */
    EXPECT_FAILURE_CODE(GatekeeperTl_Verify(NULL, uid, challenge, &pass_handle, &pass, NULL, &auth_token), BERR_INVALID_PARAMETER);
    /* NULL pass handle */
    EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, NULL, &pass, NULL, &auth_token), BERR_INVALID_PARAMETER);
    /* NULL provided password */
    EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, NULL, NULL, &auth_token), BERR_INVALID_PARAMETER);
    /* Provided password too big */
    EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass3, NULL, &auth_token), BERR_INVALID_PARAMETER);
    /* NULL output auth token */
    EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass, NULL, NULL), BERR_INVALID_PARAMETER);

    /* Invalid provided password structure */
    pass2.buffer = NULL;
    pass2.size = pass.size;
    EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass2, NULL, &auth_token), BERR_INVALID_PARAMETER);
    /* Invalid provided password structure */
    pass2.buffer = pass.buffer;
    pass2.size = 0;
    EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass2, NULL, &auth_token), BERR_INVALID_PARAMETER);

    BDBG_LOG(("%s: all tests passed", BSTD_FUNCTION));
    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(pass);
    TEST_FREE_BLOCK(pass3);
    return err;
}

static BERR_Code gk_enroll_signature_tests(GatekeeperTl_Handle handle)
{
    BERR_Code err = BERR_UNKNOWN;
    gk_password_handle_t pass_handle;
    GatekeeperTl_Password pass;
    char password[] = "good password";
    uint32_t uid = 1;
    uint64_t challenge = 52ull;
    uint32_t retry_timeout = 0;
    km_hw_auth_token_t auth_token;
    int i;

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    memset(&pass, 0, sizeof(pass));
    TEST_ALLOCATE_BLOCK(pass, strlen(password));
    memcpy(pass.buffer, password, strlen(password));

    /* Enroll a password to check that verify will fail on a corrupted pass handle */
    EXPECT_SUCCESS(GatekeeperTl_Enroll(handle, uid, NULL, NULL, &pass, NULL, &pass_handle));
    if (!pass_handle.hardware_backed || !pass_handle.user_id) {
        BDBG_ERR(("%s: unexpected return in password handle", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }
    EXPECT_SUCCESS(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass, &retry_timeout, &auth_token));

    /* Perform a successful verify between the failure tests to not invoke a throttle of 30 seconds */
    pass_handle.version += 1;
    EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass, &retry_timeout, &auth_token), BERR_INVALID_PARAMETER);
    pass_handle.version -= 1;
    EXPECT_SUCCESS(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass, &retry_timeout, &auth_token));
    pass_handle.user_id += 1;
    EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass, &retry_timeout, &auth_token), BSAGE_ERR_KM_RETRY);
    pass_handle.user_id -= 1;
    EXPECT_SUCCESS(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass, &retry_timeout, &auth_token));
    pass_handle.flags += 1;
    EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass, &retry_timeout, &auth_token), BSAGE_ERR_KM_RETRY);
    pass_handle.flags -= 1;
    EXPECT_SUCCESS(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass, &retry_timeout, &auth_token));
    pass_handle.salt += 1;
    EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass, &retry_timeout, &auth_token), BSAGE_ERR_KM_RETRY);
    pass_handle.salt -= 1;
    EXPECT_SUCCESS(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass, &retry_timeout, &auth_token));

    for (i = 0; i < 32; i++) {
        pass_handle.signature[i] += 1;
        EXPECT_FAILURE_CODE(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass, &retry_timeout, &auth_token), BSAGE_ERR_KM_RETRY);
        pass_handle.signature[i] -= 1;
        EXPECT_SUCCESS(GatekeeperTl_Verify(handle, uid, challenge, &pass_handle, &pass, &retry_timeout, &auth_token));
    }

    BDBG_LOG(("%s: all tests passed", BSTD_FUNCTION));
    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(pass);
    return err;
}

int main(int argc, char *argv[])
{
    GatekeeperTl_Handle handle = NULL;
    GatekeeperTl_InitSettings initSettings;
    NEXUS_Error err = NEXUS_SUCCESS;
    BERR_Code berr;
    int rc = 0;
    bool shortTest = false;
    int arg = 1;

#ifdef NXCLIENT_SUPPORT
    NxClient_JoinSettings joinSettings;
#endif

#ifdef NXCLIENT_SUPPORT
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "gatekeeper_test");
    joinSettings.ignoreStandbyRequest = true;
    err = NxClient_Join(&joinSettings);
    if (err) return (-1);
#else
    err = SAGE_app_join_nexus();
    if (err) return (-1);
#endif

    if ((arg < argc) && !strncmp(argv[arg], "-short", strlen(argv[arg]))) {
        shortTest = true;
        arg++;
    }

    GatekeeperTl_GetDefaultInitSettings(&initSettings);

    if ((arg < argc) && argv[arg]) {
        if (argc != arg + 1) {
            BDBG_ERR(("Too many command-line arguments. Usage:\n\tgatekeeper_test [-short] [/path/to/drm.bin]"));
            rc = -1;
            goto done;
        }
        if (strlen(argv[arg]) > sizeof(initSettings.drm_binfile_path)) {
            BDBG_ERR(("\tString length of argument '%s' is too large (%u bytes)\n", argv[arg], (uint32_t)strlen(argv[arg])));
            rc = -1;
            goto done;
        }
        memcpy(initSettings.drm_binfile_path, argv[arg], strlen(argv[arg]));
    }

    berr = GatekeeperTl_Init(&handle, &initSettings);
    if (berr != BERR_SUCCESS) {
        BDBG_ERR(("### Gatekeeper init failed (%x)\n", berr));
        rc = -1;
        goto done;
    }

    EXPECT_SUCCESS(gk_basic_tests(handle));
    if (!shortTest) {
        EXPECT_SUCCESS(gk_timeout_tests(handle));
    }
    EXPECT_SUCCESS(gk_enroll_parameter_tests(handle));
    EXPECT_SUCCESS(gk_verify_parameter_tests(handle));
    EXPECT_SUCCESS(gk_enroll_signature_tests(handle));

    BDBG_LOG(("%s: ***** ALL TESTS PASSED *****", BSTD_FUNCTION));

done:
    if (handle) {
        GatekeeperTl_Uninit(handle);
    }
#ifdef NXCLIENT_SUPPORT
    NxClient_Uninit();
#else
    SAGE_app_leave_nexus();
#endif

    if (err != BERR_SUCCESS) {
        rc = -1;
    }

    if (rc) {
        BDBG_ERR(("Failure in SAGE Gatekeeper test\n"));
    }

    return rc;
}
