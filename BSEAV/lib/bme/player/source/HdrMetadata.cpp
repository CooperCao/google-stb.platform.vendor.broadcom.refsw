/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "HdrMetadata.h"

typedef uint UINT32_C;

namespace Broadcom
{
namespace Media
{
MasteringMetadata::MasteringMetadata() {
    _red_chromaticity_x = 0.0;
    _red_chromaticity_y = 0.0;
    _green_chromaticity_x = 0.0;
    _green_chromaticity_y = 0.0;
    _blue_chromaticity_x = 0.0;
    _blue_chromaticity_y = 0.0;
    _white_chromaticity_x = 0.0;
    _white_chromaticity_y = 0.0;
    _luminance_max = 0.0;
    _luminance_min = 0.0;
}

MasteringMetadata::MasteringMetadata(const MasteringMetadata& rhs) {
    _red_chromaticity_x = rhs._red_chromaticity_x;
    _red_chromaticity_y = rhs._red_chromaticity_y;
    _green_chromaticity_x = rhs._green_chromaticity_x;
    _green_chromaticity_y = rhs._green_chromaticity_y;
    _blue_chromaticity_x = rhs._blue_chromaticity_x;
    _blue_chromaticity_y = rhs._blue_chromaticity_y;
    _white_chromaticity_x = rhs._white_chromaticity_x;
    _white_chromaticity_y = rhs._white_chromaticity_y;
    _luminance_max = rhs._luminance_max;
    _luminance_max = rhs._luminance_max;
}

MasteringMetadata::MasteringMetadata(
        float red_chromaticity_x, float red_chromaticity_y,
        float green_chromaticity_x, float green_chromaticity_y,
        float blue_chromaticity_x, float blue_chromaticity_y,
        float white_chromaticity_x, float white_chromaticity_y,
        float luminance_max, float luminance_min) {
    _red_chromaticity_x = red_chromaticity_x;
    _red_chromaticity_y = red_chromaticity_y;
    _green_chromaticity_x = green_chromaticity_x;
    _green_chromaticity_y = green_chromaticity_y;
    _blue_chromaticity_x = blue_chromaticity_x;
    _blue_chromaticity_y = blue_chromaticity_y;
    _white_chromaticity_x = white_chromaticity_x;
    _white_chromaticity_y = white_chromaticity_y;
    _luminance_max = luminance_max;
    _luminance_max = luminance_max;
}

HDRMetadata::HDRMetadata() {
    _transfer_id = kTransferIdUnknown;
    _max_cll = 0;
    _max_fall = 0;
}

HDRMetadata::HDRMetadata(
        TransferID transfer_id, uint32_t max_cll, uint32_t max_fall,
        MasteringMetadata mastering_data) {
    _transfer_id = transfer_id;
    _max_cll = max_cll;
    _max_fall = max_fall;
    _mastering_metadata = mastering_data;
}

HDRMetadata::HDRMetadata(
        TransferID transfer_id, uint32_t max_cll, uint32_t max_fall,
        float red_chromaticity_x, float red_chromaticity_y,
        float green_chromaticity_x, float green_chromaticity_y,
        float blue_chromaticity_x, float blue_chromaticity_y,
        float white_chromaticity_x, float white_chromaticity_y,
        float luminance_max, float luminance_min) {
    _transfer_id = transfer_id;
    _max_cll = max_cll;
    _max_fall = max_fall;

    _mastering_metadata._red_chromaticity_x = red_chromaticity_x;
    _mastering_metadata._red_chromaticity_y = red_chromaticity_y;
    _mastering_metadata._green_chromaticity_x = green_chromaticity_x;
    _mastering_metadata._green_chromaticity_y = green_chromaticity_y;
    _mastering_metadata._blue_chromaticity_x = blue_chromaticity_x;
    _mastering_metadata._blue_chromaticity_y = blue_chromaticity_y;
    _mastering_metadata._white_chromaticity_x = white_chromaticity_x;
    _mastering_metadata._white_chromaticity_y = white_chromaticity_y;
    _mastering_metadata._luminance_max = luminance_max;
    _mastering_metadata._luminance_max = luminance_max;
}

}  // namespace Media
}  // namespace Broadcom
