// Copyright 2015 Google Inc. All Rights Reserved.
#include <gtest/gtest.h>
#include <iostream>

#include "log.h"
#include "oemcrypto_test.h"
#include "OEMCryptoCENC.h"
#include "properties.h"
#ifdef BRCM_IMPL
#include "nexus_platform.h"
#if NXCLIENT_SUPPORT
#include "nxclient.h"
#endif
#endif

static void acknowledge_cast() {
  std::cout
      << "==================================================================\n"
      << "= This device is expected to load x509 certs as a cast receiver. =\n"
      << "==================================================================\n";
}

int main(int argc, char** argv) {
int rc;
  ::testing::InitGoogleTest(&argc, argv);
  wvcdm::Properties::Init();
  wvcdm::g_cutoff = wvcdm::LOG_INFO;
  bool is_cast_receiver = false;
  bool force_load_test_keybox = false;
  bool filter_tests = true;
#ifdef BRCM_IMPL
#if NXCLIENT_SUPPORT
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    std:: cout << std::endl << "================NXCLIENT PLATFORM INIT ===========" << std::endl;
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "oemcrypto_test");
    joinSettings.ignoreStandbyRequest = true;
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;
    NxClient_GetDefaultAllocSettings(&allocSettings);
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc)
        return BERR_TRACE(rc);
    std:: cout << "=================NXCLIENT PLATFORM INIT DONE ===========" << std::endl;
#else
    NEXUS_PlatformSettings platformSettings;
    std:: cout << std::endl << "================NEXUS PLATFORM INIT ===========" << std::endl;
    /* Platform init */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    std:: cout << "=================NEXUS PLATFORM INIT DONE ===========" << std::endl;
#endif // NXCLIENT_SUPPORT
#endif // BRCM_IMPL
  for(int i=0; i < argc; i++) {
    if (!strcmp(argv[i], "--cast")) {
      acknowledge_cast();
      is_cast_receiver = true;
    }
    if (!strcmp(argv[i], "--force_load_test_keybox")) {
      force_load_test_keybox = true;
    }
    if (!strcmp(argv[i], "--no_filter")) {
      filter_tests  = false;
    }
  }
  wvoec::global_features.Initialize(is_cast_receiver, force_load_test_keybox);
  // If the user requests --no_filter, we don't change the filter, otherwise, we
  // filter out features that are not supported.
  if (filter_tests) {
    ::testing::GTEST_FLAG(filter)
        = wvoec::global_features.RestrictFilter(::testing::GTEST_FLAG(filter));
  }
 rc = RUN_ALL_TESTS();
#ifdef BRCM_IMPL
#if NXCLIENT_SUPPORT
    NxClient_Free(&allocResults);
    NxClient_Uninit();
#else
    NEXUS_Platform_Uninit();
#endif
#endif

    return rc;
}
