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

#include <list>
#include <map>
#include <iostream>
#include <sstream>
#include <string>

#include "bsg_scene_node.h"

class PlanetEffects;
class Planets;

class Body
{
private:
   static float ScaleDist(float f)  { return 2.0f * f; }
   static float ScaleSize(float f)  { return f; }

public:
   typedef std::list<const Body *>     BodyList;
   typedef std::map<std::string, Body> BodyTable;

   Body();
   Body(std::istream &is, const std::string &name, uint32_t &lineNumber, const BodyTable &symbols);

   bsg::SceneNodeHandle   CreateGraph(const PlanetEffects &effects) const;

   float GetRadius()       const { return ScaleSize(m_radius); }
   float GetRealRadius()   const { return m_realRadius;        }
   float GetOblateness()   const { return m_oblateness;        }
   float GetAxialTilt()    const { return m_axialTilt;         }
   float GetDay()          const { return m_day;               }
   float GetCloudDay()     const { return m_cloudDay;          }

   float GetOrbitRadius()     const { return ScaleDist(m_orbitRadius);  }
   float GetRealOrbitRadius() const { return m_realOrbitRadius;         }
   float GetOrbitTilt()       const { return m_orbitTilt;               }
   float GetEccentricity()    const { return m_eccentricity;            }
   float GetYear()            const { return m_year;                    }
   float GetPhase()           const { return m_phase;                   }

   // Scale the rings by the same factor as the planet.
   float GetRingInner()    const { float r = GetRadius(); return r + m_ringInner * r / m_radius;  }
   float GetRingOuter()    const { float r = GetRadius(); return r + m_ringOuter * r / m_radius;  }

   const bsg::Vec3 &GetAtmosphereColor() const { return m_atmosphereColor; }

   // Scale the atmosphere radius by the same factor as the planet.
   float GetAtmosphereRadius() const { float r = GetRadius(); return r + m_atmosphereHeight * r / m_radius; }

   const std::string &GetName()        const { return m_name;        }
   const std::string &GetSurfaceFile() const { return m_surfaceFile; }
   const std::string &GetCloudFile()   const { return m_cloudFile;   }
   const std::string &GetRingFile()    const { return m_ringFile;    }

   const BodyList &GetSatellites() const { return m_satellites; }

   bool HasClouds()     const { return m_cloudFile != "";            }
   bool HasRings()      const { return m_ringFile  != "";            }
   bool HasAtmosphere() const { return m_atmosphereHeight != 0.0f;   }
   bool IsLight()       const { return m_light;                      }

private:
   bool ParseLine(const std::string &in, const BodyTable &symbols);
   void ParseRing(std::istringstream &iss);
   void ParseClouds(std::istringstream &iss);
   void ParseAtmosphere(std::istringstream &iss);
   void ParseSatellites(std::istringstream &iss, const BodyTable &symbols);
   void UnexpectedToken(const std::string &token) const;
   void Initialise();

   void                 CreatePlanetGeometry(bsg::SceneNodeHandle &node, const PlanetEffects &effects) const;
   bsg::GeometryHandle  CreateRingGeometry(const PlanetEffects &effects) const;
   bsg::SceneNodeHandle CreateAtmosphere(const PlanetEffects &effects) const;

   enum eState
   {
      eEXPECT_OPEN,
      eIN_BODY
   };

private:
   BodyList    m_satellites;        // List of satellites

   std::string m_name;              // Name of body
   std::string m_surfaceFile;       // Texture file name for visible surface

   // The body
   float       m_radius;            // Radius of body for display
   float       m_realRadius;        // Radius of body in km
   float       m_oblateness;        // Oblateness
   float       m_axialTilt;         // Tilt of the axis
   float       m_day;               // Length of day in hours

   // The clouds
   float       m_cloudDay;          // Length of cloud day in hours (difference from m_day will determine speed)
   std::string m_cloudFile;         // Texture file for cloud layer (should have alpha channel)

   // The orbit
   float       m_orbitRadius;       // Radius of orbit for display
   float       m_realOrbitRadius;   // Radius of orbit in km
   float       m_orbitTilt;         // Tilt of orbit from equator of parent body
   float       m_eccentricity;      // Eccentricity of orbit
   float       m_year;              // Year (time to orbit parent) in hours
   float       m_phase;             // Offset around orbit to start at

   // The rings
   float       m_ringInner;         // Height in km of inner ring above equator of parent
   float       m_ringOuter;         // Height in km of outer ring above equator of parent
   std::string m_ringFile;          // 1D texture file for ring

   // The atmosphere
   bsg::Vec3   m_atmosphereColor;   // Color of atmosphere glow
   float       m_atmosphereHeight;  // Height of atmosphere above surface

   // Is it a light source?
   bool        m_light;             // Is it the sun

   // Parser data
   eState      m_state;             // Used during construction
};

class BodyBuilder
{
public:
   BodyBuilder(std::istream &in);

   const Body *GetRoot() const;

   const Body *Find(const std::string &name) const;

private:
   Body::BodyTable      m_symbols;
   std::string          m_root;
};
