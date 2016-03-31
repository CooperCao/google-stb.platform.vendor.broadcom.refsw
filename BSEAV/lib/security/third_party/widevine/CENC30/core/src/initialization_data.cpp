// Copyright 2014 Google Inc. All Rights Reserved.

#include "initialization_data.h"

#include <string.h>

#include "buffer_reader.h"
#include "log.h"
#include "properties.h"
#include "wv_cdm_constants.h"

namespace wvcdm {

InitializationData::InitializationData(const std::string& type,
                                       const CdmInitData& data)
    : type_(type), is_cenc_(false), is_webm_(false) {
  if (type == ISO_BMFF_VIDEO_MIME_TYPE || type == ISO_BMFF_AUDIO_MIME_TYPE ||
      type == CENC_INIT_DATA_FORMAT) {
    is_cenc_ = true;
  } else if (type == WEBM_VIDEO_MIME_TYPE || type == WEBM_AUDIO_MIME_TYPE ||
             type == WEBM_INIT_DATA_FORMAT) {
    is_webm_ = true;
  }

  if (is_supported()) {
    if (is_cenc()) {
      ExtractWidevinePssh(data, &data_);
    } else {
      data_ = data;
    }
  }
}

// Parse a blob of multiple concatenated PSSH atoms to extract the first
// Widevine PSSH.
bool InitializationData::ExtractWidevinePssh(const CdmInitData& init_data,
                                             CdmInitData* output) {
  BufferReader reader(reinterpret_cast<const uint8_t*>(init_data.data()),
                      init_data.length());

  // Widevine's registered system ID.
  static const uint8_t kWidevineSystemId[] = {
      0xED, 0xEF, 0x8B, 0xA9, 0x79, 0xD6, 0x4A, 0xCE,
      0xA3, 0xC8, 0x27, 0xDC, 0xD5, 0x1D, 0x21, 0xED,
  };

  // one PSSH box consists of:
  // 4 byte size of the atom, inclusive.  (0 means the rest of the buffer.)
  // 4 byte atom type, "pssh".
  // (optional, if size == 1) 8 byte size of the atom, inclusive.
  // 1 byte version, value 0 or 1.  (skip if larger.)
  // 3 byte flags, value 0.  (ignored.)
  // 16 byte system id.
  // (optional, if version == 1) 4 byte key ID count. (K)
  // (optional, if version == 1) K * 16 byte key ID.
  // 4 byte size of PSSH data, exclusive. (N)
  // N byte PSSH data.
  while (!reader.IsEOF()) {
    size_t start_pos = reader.pos();

    // atom size, used for skipping.
    uint64_t size;
    if (!reader.Read4Into8(&size)) {
      LOGV("CdmEngine::ExtractWidevinePssh: Unable to read atom size.");
      return false;
    }
    std::vector<uint8_t> atom_type;
    if (!reader.ReadVec(&atom_type, 4)) {
      LOGV("CdmEngine::ExtractWidevinePssh: Unable to read atom type.");
      return false;
    }

    if (size == 1) {
      if (!reader.Read8(&size)) {
        LOGV("CdmEngine::ExtractWidevinePssh: Unable to read 64-bit atom "
             "size.");
        return false;
      }
    } else if (size == 0) {
      size = reader.size() - start_pos;
    }

    // "pssh"
    if (memcmp(&atom_type[0], "pssh", 4)) {
      LOGV("CdmEngine::ExtractWidevinePssh: PSSH literal not present.");
      if (!reader.SkipBytes(size - (reader.pos() - start_pos))) {
        LOGV("CdmEngine::ExtractWidevinePssh: Unable to skip the rest of "
             "the atom.");
        return false;
      }
      continue;
    }

    // version
    uint8_t version;
    if (!reader.Read1(&version)) {
      LOGV("CdmEngine::ExtractWidevinePssh: Unable to read PSSH version.");
      return false;
    }

    if (version > 1) {
      // unrecognized version - skip.
      if (!reader.SkipBytes(size - (reader.pos() - start_pos))) {
        LOGV("CdmEngine::ExtractWidevinePssh: Unable to skip the rest of "
             "the atom.");
        return false;
      }
      continue;
    }

    // flags
    if (!reader.SkipBytes(3)) {
      LOGV("CdmEngine::ExtractWidevinePssh: Unable to skip the PSSH flags.");
      return false;
    }

    // system id
    std::vector<uint8_t> system_id;
    if (!reader.ReadVec(&system_id, sizeof(kWidevineSystemId))) {
      LOGV("CdmEngine::ExtractWidevinePssh: Unable to read system ID.");
      return false;
    }

    if (memcmp(&system_id[0], kWidevineSystemId, sizeof(kWidevineSystemId))) {
      // skip non-Widevine PSSH boxes.
      if (!reader.SkipBytes(size - (reader.pos() - start_pos))) {
        LOGV("CdmEngine::ExtractWidevinePssh: Unable to skip the rest of "
             "the atom.");
        return false;
      }
      LOGV("CdmEngine::ExtractWidevinePssh: Skipping non-Widevine PSSH.");
      continue;
    }

    if (version == 1) {
      // v1 has additional fields for key IDs.  We can skip them.
      uint32_t num_key_ids;
      if (!reader.Read4(&num_key_ids)) {
        LOGV("CdmEngine::ExtractWidevinePssh: Unable to read num key IDs.");
        return false;
      }
      if (!reader.SkipBytes(num_key_ids * 16)) {
        LOGV("CdmEngine::ExtractWidevinePssh: Unable to skip key IDs.");
        return false;
      }
    }

    // size of PSSH data
    uint32_t data_length;
    if (!reader.Read4(&data_length)) {
      LOGV("CdmEngine::ExtractWidevinePssh: Unable to read PSSH data size.");
      return false;
    }

    output->clear();
    if (!reader.ReadString(output, data_length)) {
      LOGV("CdmEngine::ExtractWidevinePssh: Unable to read PSSH data.");
      return false;
    }

    return true;
  }

  // we did not find a matching record
  return false;
}

}  // namespace wvcdm
