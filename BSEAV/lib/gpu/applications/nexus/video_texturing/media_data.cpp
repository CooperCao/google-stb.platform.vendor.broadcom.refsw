/******************************************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************************************/

#include "media_data.h"

MediaProber *MediaProber::m_instance = NULL;

void MediaProber::GetStreamData(const std::string &filename, MediaData *data)
{
   data->filename = filename;
   probe_media(filename.c_str(), &data->data);
}
