/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "Media.h"

#ifndef BME_HDRMETADATA_H_
#define BME_HDRMETADATA_H_

namespace Broadcom
{
namespace Media
{
enum TransferID {
    // The first 0-255 values should match the H264 specification (see Table E-4
    // Transfer Characteristics in https://www.itu.int/rec/T-REC-H.264/en).
    kTransferIdReserved0 = 0,
    kTransferIdBt709 = 1,
    kTransferIdUnspecified = 2,
    kTransferIdReserved = 3,
    kTransferIdGamma22 = 4,
    kTransferIdGamma28 = 5,
    kTransferIdSmpte170M = 6,
    kTransferIdSmpte240M = 7,
    kTransferIdLinear = 8,
    kTransferIdLog = 9,
    kTransferIdLogSqrt = 10,
    kTransferIdIec6196624 = 11,
    kTransferIdBt1361Ecg = 12,
    kTransferIdIec6196621 = 13,
    kTransferId10BitBt2020 = 14,
    kTransferId12BitBt2020 = 15,
    kTransferIdSmpteSt2084 = 16,
    kTransferIdSmpteSt4281 = 17,
    kTransferIdAribStdB67 = 18,  // AKA hybrid-log gamma, HLG.

    kTransferIdLastStandardValue = kTransferIdSmpteSt4281,

    // Chrome-specific values start at 1000.
    kTransferIdUnknown = 1000,
    kTransferIdGamma24,

    // This is an ad-hoc transfer function that decodes SMPTE 2084 content
    // into a 0-1 range more or less suitable for viewing on a non-hdr
    // display.
    kTransferIdSmpteSt2084NonHdr,

    kTransferIdCustom,
    kTransferIdLast = kTransferIdCustom,
};

// SMPTE ST 2086 mastering metadata.
struct BME_SO_EXPORT MasteringMetadata {
    float _red_chromaticity_x;
    float _red_chromaticity_y;
    float _green_chromaticity_x;
    float _green_chromaticity_y;
    float _blue_chromaticity_x;
    float _blue_chromaticity_y;
    float _white_chromaticity_x;
    float _white_chromaticity_y;
    float _luminance_max;
    float _luminance_min;

    MasteringMetadata();
    MasteringMetadata(const MasteringMetadata& rhs);
    MasteringMetadata(float red_chromaticity_x, float red_chromaticity_y,
        float green_chromaticity_x, float green_chromaticity_y,
        float blue_chromaticity_x, float blue_chromaticity_y,
        float white_chromaticity_x, float white_chromaticity_y,
        float luminance_max, float luminance_min);
};

// HDR metadata common for HDR10 and WebM/VP9-based HDR formats.
struct BME_SO_EXPORT HDRMetadata {
    TransferID _transfer_id;
    unsigned _max_cll;
    unsigned _max_fall;
    MasteringMetadata _mastering_metadata;

    HDRMetadata();
    HDRMetadata(TransferID transfer_id, unsigned max_cll, unsigned max_fall,
        MasteringMetadata mastering_metadata);
    HDRMetadata(
        TransferID transfer_id, uint32_t max_cll, uint32_t max_fall,
        float red_chromaticity_x, float red_chromaticity_y,
        float green_chromaticity_x, float green_chromaticity_y,
        float blue_chromaticity_x, float blue_chromaticity_y,
        float white_chromaticity_x, float white_chromaticity_y,
        float luminance_max, float luminance_min);
};

}
}
#endif // ifndef BME_HDRMETADATA_H_