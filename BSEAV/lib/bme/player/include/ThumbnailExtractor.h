/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef COMPONENTS_MEDIASTREAMER_IMPL_THUMBNAILEXTRACTOR_H_
#define COMPONENTS_MEDIASTREAMER_IMPL_THUMBNAILEXTRACTOR_H_

#include "nexus_types.h"
#include "nexus_playpump.h"
#include "bfile_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=**************
bthumbnail_extractor is a library to extract thumbnails from streams.
It uses Nexus for access to playback and for basic data types.
It uses media framework, specifically bmedia_player, to parse streams.
It supports any stream or codec supported by Nexus.

See BSEAV/app/thumbnail for a simple example of thumbnail extraction.

Brutus uses bthumbnail_extractor along with bthumbnail_stream and bthumbnail_manager.
bthumbnail_extractor does not depend on bthumbnail_stream and bthumbnail_manager.
****************/

typedef ssize_t (*buffer_read_t)(void* buf, size_t length, void* data);
/**
Summary:
**/
typedef struct bthumbnail_extractor *bthumbnail_extractor_t;

/**
Summary:
**/
typedef struct bthumbnail_extractor_create_settings
{
    unsigned buffer_size; /* set for maximum sized thumbnail */
} bthumbnail_extractor_create_settings;


/**
Summary:
**/
typedef struct bthumbnail_extractor_settings
{
    NEXUS_VideoCodec videoCodec;
    NEXUS_TransportType transportType;
    NEXUS_TransportTimestampType timestampType;
    unsigned short videoPid;
    NEXUS_PlaypumpHandle playpump;
    bfile_io_read_t datafile;
    bfile_io_read_t indexfile;
    buffer_read_t readBuffer;
    unsigned videoCdbSize;
    void* data;
} bthumbnail_extractor_settings;

/**
Summary:
**/
void bthumbnail_extractor_get_default_create_settings(
    bthumbnail_extractor_create_settings *p_settings); /* [out] */

/**
Summary:
**/
bthumbnail_extractor_t bthumbnail_extractor_create(
    const bthumbnail_extractor_create_settings *p_settings);

/**
Summary:
**/
void bthumbnail_extractor_destroy(bthumbnail_extractor_t handle);

/**
Summary:
**/
void bthumbnail_extractor_get_settings(bthumbnail_extractor_t handle,
                            bthumbnail_extractor_settings *p_settings); /* [out] */

/**
Summary:
**/
int bthumbnail_extractor_set_settings(bthumbnail_extractor_t handle,
                                      const bthumbnail_extractor_settings *p_settings);

/**
Summary:
**/
int bthumbnail_extractor_start_playpump(bthumbnail_extractor_t handle);

/**
Summary:
Extract a thumbnail and send it to transport using playpump.
**/
int bthumbnail_extractor_feed_picture(bthumbnail_extractor_t handle,
                                      unsigned timestamp); /* in milliseconds */

/**
Summary:
**/
void bthumbnail_extractor_stop_playpump(bthumbnail_extractor_t handle);

/**
Summary:
**/
typedef struct bthumbnail_extractor_status
{
    unsigned timestamp; /* Timestamp of the extracted picture in milliseconds */
} bthumbnail_extractor_status;

/**
Summary:
Get status of thumbnail extractor.
The status is cleared and populated again on each call to bthumbnail_extractor_feed_picture.
**/
void bthumbnail_extractor_get_status(bthumbnail_extractor_t handle,
                                     bthumbnail_extractor_status *status);
#ifdef __cplusplus
}
#endif

#endif  // COMPONENTS_MEDIASTREAMER_IMPL_THUMBNAILEXTRACTOR_H_
