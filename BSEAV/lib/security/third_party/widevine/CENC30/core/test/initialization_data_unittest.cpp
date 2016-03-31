// Copyright 2015 Google Inc. All Rights Reserved.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "initialization_data.h"
#include "string_conversions.h"
#include "wv_cdm_constants.h"

// References:
//  [1] http://dashif.org/identifiers/content-protection/
//  [2] http://www.w3.org/TR/encrypted-media/cenc-format.html#common-system

namespace wvcdm {

namespace {

const std::string kWidevinePssh = a2bs_hex(
    // Widevine PSSH box
    "00000042"                          // atom size
    "70737368"                          // atom type="pssh"
    "00000000"                          // v0, flags=0
    "edef8ba979d64acea3c827dcd51d21ed"  // system id (Widevine)
    "00000022"                          // data size
    // data:
    "08011a0d7769646576696e655f74657374220f73747265616d696e675f636c697031");

const std::string kWidevinePsshFirst = a2bs_hex(
    // first PSSH box, Widevine
    "00000042"                          // atom size
    "70737368"                          // atom type "pssh"
    "00000000"                          // v0, flags=0
    "edef8ba979d64acea3c827dcd51d21ed"  // system id (Widevine)
    "00000022"                          // data size
    // data:
    "08011a0d7769646576696e655f74657374220f73747265616d696e675f636c697031"

    // second PSSH box, Playready [1]
    "00000028"                          // atom size
    "70737368"                          // atom type "pssh"
    "00000000"                          // v0, flags=0
    "9a04f07998404286ab92e65be0885f95"  // system id (PlayReady)
    "00000008"                          // data size
    // arbitrary data:
    "0102030405060708");

const std::string kWidevinePsshAfterV0Pssh = a2bs_hex(
    // first PSSH box, Playready [1]
    "00000028"                          // atom size
    "70737368"                          // atom type "pssh"
    "00000000"                          // v0, flags=0
    "9a04f07998404286ab92e65be0885f95"  // system id (PlayReady)
    "00000008"                          // data size
    // arbitrary data:
    "0102030405060708"

    // second PSSH box, Widevine
    "00000042"                          // atom size
    "70737368"                          // atom type "pssh"
    "00000000"                          // v0, flags=0
    "edef8ba979d64acea3c827dcd51d21ed"  // system id (Widevine)
    "00000022"                          // data size
    // data:
    "08011a0d7769646576696e655f74657374220f73747265616d696e675f636c697031");

const std::string kWidevinePsshAfterNonZeroFlags = a2bs_hex(
    // first PSSH box, Playready [1]
    "00000028"                          // atom size
    "70737368"                          // atom type "pssh"
    "00abcdef"                          // v0, flags=abcdef
    "9a04f07998404286ab92e65be0885f95"  // system id (PlayReady)
    "00000008"                          // data size
    // arbitrary data:
    "0102030405060708"

    // second PSSH box, Widevine
    "00000042"                          // atom size
    "70737368"                          // atom type "pssh"
    "00000000"                          // v0, flags=0
    "edef8ba979d64acea3c827dcd51d21ed"  // system id (Widevine)
    "00000022"                          // data size
    // data:
    "08011a0d7769646576696e655f74657374220f73747265616d696e675f636c697031");

const std::string kWidevinePsshAfterV1Pssh = a2bs_hex(
    // first PSSH box, generic CENC [2]
    "00000044"                          // atom size
    "70737368"                          // atom type "pssh"
    "01000000"                          // v1, flags=0
    "1077efecc0b24d02ace33c1e52e2fb4b"  // system id (generic CENC)
    "00000002"                          // key ID count
    "30313233343536373839303132333435"  // key ID="0123456789012345"
    "38393031323334354142434445464748"  // key ID="ABCDEFGHIJKLMNOP"
    "00000000"                          // data size=0

    // second PSSH box, Widevine
    "00000042"                          // atom size
    "70737368"                          // atom type "pssh"
    "00000000"                          // v0, flags=0
    "edef8ba979d64acea3c827dcd51d21ed"  // system id (Widevine)
    "00000022"                          // data size
    // data:
    "08011a0d7769646576696e655f74657374220f73747265616d696e675f636c697031");

const std::string kWidevineV1Pssh = a2bs_hex(
    // Widevine PSSH box, v1 format
    "00000044"                          // atom size
    "70737368"                          // atom type "pssh"
    "01000000"                          // v1, flags=0
    "edef8ba979d64acea3c827dcd51d21ed"  // system id (Widevine)
    "00000002"                          // key ID count
    "30313233343536373839303132333435"  // key ID="0123456789012345"
    "38393031323334354142434445464748"  // key ID="ABCDEFGHIJKLMNOP"
    "00000022"                          // data size
    // data:
    "08011a0d7769646576696e655f74657374220f73747265616d696e675f636c697031");

const std::string kOtherBoxFirst = a2bs_hex(
    // first box, not a PSSH box
    "00000018"                          // atom size
    "77686174"                          // atom type "what"
    "deadbeefdeadbeefdeadbeefdeadbeef"  // garbage box data

    // second box, a Widevine PSSH box
    "00000042"                          // atom size
    "70737368"                          // atom type "pssh"
    "00000000"                          // v0, flags=0
    "edef8ba979d64acea3c827dcd51d21ed"  // system id (Widevine)
    "00000022"                          // data size
    // data:
    "08011a0d7769646576696e655f74657374220f73747265616d696e675f636c697031");

const std::string kZeroSizedPsshBox = a2bs_hex(
    // Widevine PSSH box
    "00000000"                          // atom size (whole buffer)
    "70737368"                          // atom type="pssh"
    "00000000"                          // v0, flags=0
    "edef8ba979d64acea3c827dcd51d21ed"  // system id (Widevine)
    "00000022"                          // data size
    // data:
    "08011a0d7769646576696e655f74657374220f73747265616d696e675f636c697031");

class InitializationDataTest : public ::testing::TestWithParam<std::string> {};

}  // namespace

TEST_P(InitializationDataTest, Parse) {
  InitializationData init_data(ISO_BMFF_VIDEO_MIME_TYPE, GetParam());
  EXPECT_FALSE(init_data.IsEmpty());
}

INSTANTIATE_TEST_CASE_P(
    ParsePssh, InitializationDataTest,
    ::testing::Values(
        kWidevinePssh,
        kWidevinePsshFirst,
        kWidevinePsshAfterV0Pssh,
        kWidevinePsshAfterNonZeroFlags,
        kWidevinePsshAfterV1Pssh,
        kWidevineV1Pssh,
        kOtherBoxFirst,
        kZeroSizedPsshBox
    ));

}  // namespace wvcdm
