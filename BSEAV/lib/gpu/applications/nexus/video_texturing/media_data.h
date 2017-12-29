/******************************************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <map>

#include <stdint.h>

#include "media_probe.h"

struct MediaData
{
   std::string          filename;
   struct probe_results data;
};

// Singleton
class MediaProber
{
private:
   MediaProber() {};

public:
   static MediaProber *Instance()
   {
      if (m_instance == NULL)
         m_instance = new MediaProber();
      return m_instance;
   }

   void GetStreamData(const std::string &filename, MediaData *data);

private:
   static MediaProber  *m_instance;
};
