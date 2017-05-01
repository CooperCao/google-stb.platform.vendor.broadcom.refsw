/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __PG_INFO_H__
#define __PG_INFO_H__

#include <stdint.h>
#include <string>
#include <vector>
#include <mutex>

#include "bsg_time.h"
#include "bsg_task.h"

namespace pg
{

class Description
{
public:
   Description();
   Description(const std::string &desc);

   const std::string &GetDescription() const { return m_result; }

   bool HasAudioDescription() const { return m_hasAudioDescription;  }
   bool HasSubtitles()        const { return m_hasSubtitles;         }
   bool HasSigning()          const { return m_hasSigning;           }

private:
   void ParseString(const std::string &in);

private:
   std::string m_result;
   bool        m_hasAudioDescription;
   bool        m_hasSubtitles;
   bool        m_hasSigning;
};

// These classes hold information about the available programmes.
//
// They should probably work with some kind of ID to interrogate the programme database directly
//
class ProgramInfo
{
public:
   ProgramInfo() {}

   ProgramInfo(const bsg::Time &start, const bsg::Time &end, const std::string &title, const std::string &description, bool isHD = false) :
      m_start(start),
      m_end(end),
      m_duration(end - start),
      m_title(StripTrailingDots(title)),
      m_description(description),
      m_isHD(isHD)
   {
   }

   const bsg::Time &GetStartTime() const { return m_start;     }
   const bsg::Time &GetEndTime()   const { return m_end;       }
   const bsg::Time &GetDuration()  const { return m_duration;  }

   const std::string &GetTitle()       const { return m_title;                         }
   const std::string &GetDescription() const { return m_description.GetDescription();  }

   bool              HasAudioDescription() const { return m_description.HasAudioDescription();  }
   bool              HasSubtitles()        const { return m_description.HasSubtitles();         }
   bool              HasSigning()          const { return m_description.HasSigning();           }
   bool              IsHD()                const { return m_isHD;                               }

private:
   std::string StripTrailingDots(const std::string &str) const;

private:
   bsg::Time   m_start;
   bsg::Time   m_end;
   bsg::Time   m_duration;
   std::string m_title;
   Description m_description;
   bool        m_isHD;
};

class ChannelInfo
{
public:
   enum ServiceType
   {
      eTV = 0,
      eRadio
   };

   ChannelInfo(uint32_t number, const std::string &name, ServiceType type) :
      m_number(number),
      m_name(name)
   {}

   void Clear()                                    { m_programs.clear();            }
   void AddProgram(const ProgramInfo &program)     { m_programs.push_back(program); }
   uint32_t GetNumPrograms() const                 { return m_programs.size();      }
   const ProgramInfo &GetProgram(uint32_t n) const { return m_programs[n];          }

   uint32_t GetNumber() const                      { return m_number;               }
   const std::string &GetName() const              { return m_name;                 }

private:
   uint32_t       m_number;
   std::string    m_name;

   std::vector<ProgramInfo>  m_programs;
};

class GridInfo
{
public:
   void Lock() const                               { m_mutex.lock(); }
   void Unlock() const                             { m_mutex.unlock(); }

   void Clear()                                    { m_channels.clear();            }
   uint32_t AddChannel(const ChannelInfo &channel);
   uint32_t GetNumChannels() const                 { return m_channels.size();      }

   const ChannelInfo &GetChannel(uint32_t n) const { return m_channels[n];          }
   ChannelInfo &GetChannel(uint32_t n)             { return m_channels[n];          }

private:
   mutable std::mutex            m_mutex;
   std::vector<ChannelInfo>      m_channels;
};

}

#endif
