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

#ifndef __BSG_OBJ_READER_H__
#define __BSG_OBJ_READER_H__

#include "bsg_common.h"
#include "bsg_vector.h"

#include <stdint.h>
#include <vector>
#include <map>
#include <string>
#include <istream>
#include <sstream>

// @cond

namespace bsg
{

class ObjOptions
{
public:
   ObjOptions() :
      m_loadMaterialFiles(false)
   {
   }

   bool GetLoadMaterialFiles() const      { return m_loadMaterialFiles; }
   void SetLoadMaterialFiles(bool load)   { m_loadMaterialFiles = load; }

private:
   bool  m_loadMaterialFiles;
};

// The class ObjReader is a simple reader of the Alias Wavefron "obj" format.
//
// The reader handles all the "v" (vertex), "vt" (vertex texture) and
// "vn" (vertex normal) entries and the "f" face descriptions.
//
// It currently ignores all other directives in the source data.
//
// To read the data, simply define an object of ObjReader type and supply
// the stream from which the data should be read.  The constructed object will
// then contain the data which can be read via the accessor functions.
//

struct ObjTexMap
{
   ObjTexMap();
   ObjTexMap(std::stringstream &is);

   enum eChan
   {
      R, G, B,
      M, L, Z  // Matte, luminance, depth
   };

   enum eMapType
   {
      NONE,
      SPHERE,
      CUBE_TOP,
      CUBE_BOTTOM,
      CUBE_FRONT,
      CUBE_BACK,
      CUBE_LEFT,
      CUBE_RIGHT
   };

   bool     m_blendU;   // Turns on/off texture "blending" in U and V direction
   bool     m_blendV;
   float    m_bm;       // Bump multiplier
   float    m_boost;    // Increases "clarity"
   bool     m_cc;       // Color correction control
   bool     m_clamp;    // Turns on texture coordinate clamping
   eChan    m_imfchan;  // Channel used for bump
   float    m_base;     // Offset and scale for texture
   float    m_gain;
   Vec3     m_o;        // Offset for texture coordinates
   Vec3     m_s;        // Scale of the texture coordinates
   Vec3     m_t;        // Turbulence
   uint32_t m_texres;   // Requested resolution

   eMapType          m_mapType;        // Ephemeral (bound to last map type read)

   std::string       m_mapName;
   std::string       m_sphereName;
   std::string       m_cubeTopName;
   std::string       m_cubeBottomName;
   std::string       m_cubeFrontName;
   std::string       m_cubeBackName;
   std::string       m_cubeLeftName;
   std::string       m_cubeRightName;

private:
   void Initialise();
};

class ObjMaterial
{
public:
   ObjMaterial();
   ObjMaterial(std::istream &is);

   bool DiffuseTexturing() const { return m_map_Kd.size() != 0; }

private:
   void Initialise();

public:
   Vec3  m_Ka;                // Ambient color
   Vec3  m_Kd;                // Diffuse color
   Vec3  m_Ks;                // Specular color
   Vec3  m_Tf;                // Transmission filter color

   uint32_t    m_illum;       // Illumination model

   float       m_d;           // Disolve factor
   bool        m_halo;        // Is disolve dependent on viewing angle

   float       m_Ns;          // Specular exponent
   float       m_Ni;          // Refractive index

   float       m_sharpness;   // Reflection sharpness

   std::vector<ObjTexMap>  m_map_Ka;      // Ambient color map
   std::vector<ObjTexMap>  m_map_Kd;      // Diffuse color map
   std::vector<ObjTexMap>  m_map_Ks;      // Specular color map
   std::vector<ObjTexMap>  m_map_Ns;      // Specular exponent map
   std::vector<ObjTexMap>  m_map_d;       // Disolve map
   std::vector<ObjTexMap>  m_disp;        // Displacement map
   std::vector<ObjTexMap>  m_decal;       // Decal map
   std::vector<ObjTexMap>  m_bump;        // Bump offset map

   std::vector<ObjTexMap>  m_refl;        // Reflection map
};

class ObjTokenizer;

class ObjReader
{
public:
   ObjReader(const std::string &fileName, const std::string &path, const ObjOptions &options = m_defaultOptions);
   ObjReader(std::istream &file, const std::string &path, const ObjOptions &options = m_defaultOptions);

   enum
   {
      NO_INDEX = -1
   };

   class Index
   {
   public:
      Index() :
         m_v(NO_INDEX), m_vt(NO_INDEX), m_vn(NO_INDEX)
      {}

      int32_t V()  const { return m_v; }
      int32_t VT() const { return m_vt; }
      int32_t VN() const { return m_vn; }

      int32_t &V()  { return m_v; }
      int32_t &VT() { return m_vt; }
      int32_t &VN() { return m_vn; }

   private:
      int32_t        m_v;
      int32_t        m_vt;
      int32_t        m_vn;
   };

   typedef std::vector<Index>                   Face;
   typedef std::vector<Face>                    Object;
   typedef std::map<std::string, ObjMaterial>   MaterialMap;
   typedef std::map<std::string, uint32_t>      IndexMap;

   bool        HasNormals(const Object &object)                   const;
   bool        HasTextureCoordinates(const Object &object)        const;

   uint32_t    GetNumObjects()                                    const { return m_objects.size(); }
   const Object &GetObject(uint32_t o)                            const { return m_objects[o];     }

   const Vec3 &GetPosition(const Face &face, uint32_t i)          const { return m_vertices [face[i].V()];  }
   const Vec2 &GetTextureCoordinate(const Face &face, uint32_t i) const { return m_texCoords[face[i].VT()]; }
   const Vec3 &GetNormal(const Face &face, uint32_t i)            const { return m_normals  [face[i].VN()]; }

   uint32_t          GetNumMaterials()                            const { return m_materials.size(); }
   const std::string &GetMaterialName(uint32_t o)                 const { return m_materialNames[o]; }
   const ObjMaterial &GetMaterial(const std::string &name)        const;
   const MaterialMap &GetMaterials()                              const;

   static const ObjOptions &GetDefaultOptions()                         { return m_defaultOptions; }

private:
   void Initialise(const char *buffer, const std::string &path, const ObjOptions &options);
   void ChangeMaterial(const std::string &name);
   void LoadMaterials(const std::string &fileName);
   void ReadV(ObjTokenizer &tok);
   void ReadVT(ObjTokenizer &tok);
   void ReadVN(ObjTokenizer &tok);
   void ReadF(ObjTokenizer &tok);
   void ReadUsemtl(ObjTokenizer &tok);
   void ReadMtllib(ObjTokenizer &tok);

private:
   int32_t                    m_currentMaterialIx;

   std::vector<Vec3>          m_vertices;
   std::vector<Vec2>          m_texCoords;
   std::vector<Vec3>          m_normals;
   IndexMap                   m_materialObjectIndex;
   std::vector<std::string>   m_materialNames;
   std::vector<Object>        m_objects;

   std::string                m_path;

   MaterialMap                m_materials;

   ObjOptions                 m_options;

   static ObjOptions          m_defaultOptions;
};

} // namespace bsg

// @endcond

#endif

