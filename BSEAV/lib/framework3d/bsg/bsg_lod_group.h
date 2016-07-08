/******************************************************************************
 *   Broadcom Proprietary and Confidential. (c)2011-2012 Broadcom.  All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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

#ifndef __BSG_LOD_GROUP_H__
#define __BSG_LOD_GROUP_H__

#include "bsg_common.h"
#include "bsg_geometry.h"

#undef min
#undef max

#include <limits>

namespace bsg
{

//! Lots of geometric detail is only useful if the object is drawn at a reasonable size on the display.
//! The idea behind geometric level of detail (LOD) is to select different models according to the
//! projected size of the object.
//! The implementation uses an approximate method based on the size of the bounding volume.  This usually
//! results in an overestimate of the objects screen size, but this is preferable to an underestimate
//! which would lead to chunky looking models and visible transitions between LOD levels.
//!
//! Applications can select the level of detail to be used when rendering an object via:
//! \code
//! LODGroup  lodGroup(GetOptions().GetHeight());
//!
//! lodGroup.Begin();
//! lodGroup.Add(geom1, 200.0f); // 200 and above
//! lodGroup.Add(geom2, 50.0f);  // 50 to 200
//! lodGroup.Add(geom3, 12.0f);  // 12 to 50
//! lodGroup.Add(geom4);         // 0 to 12
//! lodGroup.End();
//! \endcode
//!
//! This selects geom1 for projected sizes of 200 pixels, geom2 for 50 to 200 and so on.
class LODGroup
{
public:
   LODGroup() :
      m_inBegin(false),
      m_previous(std::numeric_limits<float>::max())
   {
   }

   ~LODGroup()
   {
      if (m_inBegin)
         BSG_THROW("LODGroup destroyed whilst still in a begin/end");
   }

   void Begin()
   {
      m_inBegin = true;
   }

   void End();

   void Add(GeometryHandle geom, float screenMinSize)
   {
      if (!m_inBegin)
         BSG_THROW("LODGroup Add called outside of a begin/end");

      if (screenMinSize >= m_previous)
         BSG_THROW("LODGroup Add called in wrong order.  Highest values must be set first");

      m_geometries.push_back(geom);
      geom->GetLodRange().SetRange(screenMinSize, m_previous);
      m_previous = screenMinSize;
   }

   void Add(GeometryHandle geom)
   {
      Add(geom, 0.0f);
   }

private:
   Geometries  m_geometries;
   bool        m_inBegin;
   float       m_previous;
};

}

#endif /* __BSG_LOD_GROUP_H__ */

