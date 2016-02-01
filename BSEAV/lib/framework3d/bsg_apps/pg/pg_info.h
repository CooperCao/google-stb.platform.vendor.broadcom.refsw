/******************************************************************************
 *   (c)2011-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
 * AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
 * SOFTWARE.  
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE
 * ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 * ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#ifndef __PG_INFO_H__
#define __PG_INFO_H__

#include <stdint.h>
#include <string>
#include <vector>

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
      m_name(name),
      m_type(type)
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
   ServiceType    m_type;
   uint32_t       m_onid;
   uint32_t       m_tsid;
   uint32_t       m_sid;
   uint32_t       m_id;
   std::string    m_providerName;
   bool           m_isEncrypted;
   bool           m_isHdtv;
   bool           m_isVisible;
   bool           m_isNumericSelectable;

   std::vector<ProgramInfo>  m_programs;
};

class GridInfo
{
public:
   void Lock() const                               { m_mutex.Lock(); }
   void Unlock() const                             { m_mutex.Unlock(); }

   void Clear()                                    { m_channels.clear();            }
   uint32_t AddChannel(const ChannelInfo &channel);
   uint32_t GetNumChannels() const                 { return m_channels.size();      }
   
   const ChannelInfo &GetChannel(uint32_t n) const { return m_channels[n];          }
   ChannelInfo &GetChannel(uint32_t n)             { return m_channels[n];          }

private:
   mutable bsg::Mutex            m_mutex;
   std::vector<ChannelInfo>      m_channels;
};

}

#endif
