/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
// The class ObjReader is a simple reader of the Alias Wavefront "obj" format.
//
// The reader handles all the "v" (vertex), "vt" (vertex texture) and
// "vn" (vertex normal) entries and the "f" face descriptions.
//
// It ignores all other directives in the source data.
//
// To read the data, simply define an object of ObjReader type and supply
// the stream from which the data should be read.  The constructed object will
// then contain the data which can be read via the accessor functions.

#include "bsg_obj_reader.h"
#include "bsg_exception.h"
#include "bsg_trackers.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <ctype.h>

using namespace std;

namespace bsg
{

static std::string GetToEnd(istream &is)
{
   stringstream   os;
   bool           started = false;

   noskipws(is);

   while (is.good())
   {
      char  ch;

      is >> ch;

      if (!started && !isspace(ch))
         started = true;

      if (started && !is.eof())
         os << ch;
   }

   return os.str();
}

ObjOptions  ObjReader::m_defaultOptions;

// IndexOf
//
// If index is strictly positive then return the zero based index
// If index is negative then return the corresponding zero based index according to the current size
//
static uint32_t IndexOf(int32_t idx, uint32_t size)
{
   if (idx > 0)
      return idx - 1;

   return size + idx;
}

void ObjReader::ChangeMaterial(const string &name)
{
   map<string, uint32_t>::const_iterator entry = m_materialObjectIndex.find(name);

   uint32_t index = 0;

   if (entry == m_materialObjectIndex.end())
   {
      index = m_objects.size();

      m_materialObjectIndex[name] = index;
      m_materialNames.push_back(name);
      m_objects.push_back(Object());
   }
   else
   {
      index = entry->second;
   }

   m_currentMaterialIx = index;
}

// A helper for parsing OBJ file in memory
//
class ObjTokenizer
{
public:
   ObjTokenizer(const char *buff) :
      m_ptr(buff)
   {}

   void SkipToWhite()
   {
      while (*m_ptr != 0 && *m_ptr != ' ')
         ++m_ptr;
   }

   void SkipToWhiteOrDelim(char delim)
   {
      while (*m_ptr != 0 && *m_ptr != ' ' && *m_ptr != delim)
         ++m_ptr;
   }

   void SkipWhite()
   {
      while (*m_ptr != 0 && *m_ptr == ' ')
         ++m_ptr;
   }

   void SkipLine()
   {
      while (*m_ptr != 0 && *m_ptr != '\n')
         ++m_ptr;

      if (*m_ptr == '\n')
         ++m_ptr;
   }

   void NextToken()
   {
      SkipToWhite();
      SkipWhite();
   }

   char Ch() const
   {
      return m_ptr[0];
   }

   bool Match(const char *s) const
   {
      const char *ptr = m_ptr;
      const char *str = s;

      while (*str == *ptr && *str != '\0' && *ptr != '\0')
      {
         ++str;
         ++ptr;
      }

      return *str == '\0';
   }

   void NextCh()
   {
      if (*m_ptr != '\0')
         ++m_ptr;
   }

   float GetFloat() const
   {
      return (float)atof(m_ptr);
   }

   int GetInt() const
   {
      return atoi(m_ptr);
   }

   std::string GetString(char delim) const
   {
      const char  *ptr = m_ptr;
      std::string res;

      while (*ptr != '\0' && *ptr != '\r' && *ptr != delim)
      {
         res += *ptr;
         ++ptr;
      }

      return res;
   }

private:
   const char *m_ptr;
};

void ObjReader::ReadV(ObjTokenizer &tok)
{
   Vec3  v;

   tok.NextToken();
   v.X() = tok.GetFloat();
   tok.NextToken();
   v.Y() = tok.GetFloat();
   tok.NextToken();
   v.Z() = tok.GetFloat();
   m_vertices.push_back(v);
}

void ObjReader::ReadVT(ObjTokenizer &tok)
{
   Vec2  vt;

   tok.NextToken();
   vt.X() = tok.GetFloat();
   tok.NextToken();
   vt.Y() = tok.GetFloat();
   m_texCoords.push_back(vt);
}

void ObjReader::ReadVN(ObjTokenizer &tok)
{
   Vec3  vn;

   tok.NextToken();
   vn.X() = tok.GetFloat();
   tok.NextToken();
   vn.Y() = tok.GetFloat();
   tok.NextToken();
   vn.Z() = tok.GetFloat();
   m_normals.push_back(vn);
}

void ObjReader::ReadF(ObjTokenizer &tok)
{
   vector<Index>    face;
   // If no material has been specified, create a default one.
   if (m_currentMaterialIx == -1)
      ChangeMaterial("default");

   while (tok.Ch() != '\0' && tok.Ch() != '\n')
   {
      Index indices;

      tok.NextToken();

      indices.V() = IndexOf(tok.GetInt(), m_vertices.size());

      tok.SkipToWhiteOrDelim('/');

      if (tok.Ch() == '/')
         tok.NextCh();

      if (tok.Ch() != '/')
         indices.VT() = IndexOf(tok.GetInt(), m_texCoords.size());

      tok.SkipToWhiteOrDelim('/');

      if (tok.Ch() == '/')
      {
         tok.NextCh();
         indices.VN() = IndexOf(tok.GetInt(), m_normals.size());
      }

      tok.SkipToWhiteOrDelim('\n');

      face.push_back(indices);
   }

   m_objects[m_currentMaterialIx].push_back(face);
}

void ObjReader::ReadUsemtl(ObjTokenizer &tok)
{
   tok.NextToken();
   std::string materialName = tok.GetString('\n');
   ChangeMaterial(materialName);
}

void ObjReader::ReadMtllib(ObjTokenizer &tok)
{
   tok.NextToken();
   std::string library = tok.GetString('\n');
   LoadMaterials(m_path + library);
}


// ObjReader
//
// CONSTRUCTORS
//
void ObjReader::Initialise(const char *buffer, const string &path, const ObjOptions &options)
{
   ObjTokenizer   tok(buffer);

   m_path    = path;
   m_options = options;

   m_currentMaterialIx = -1;

   while (tok.Ch() != 0)
   {
      if (tok.Match("vt"))
         ReadVT(tok);
      else if (tok.Match("vn"))
         ReadVN(tok);
      else if (tok.Match("v"))
         ReadV(tok);
      else if (tok.Match("f"))
         ReadF(tok);
      else if (tok.Match("usemtl"))
         ReadUsemtl(tok);
      else if (tok.Match("mtllib"))
         ReadMtllib(tok);

      tok.SkipLine();
   }
}

ObjReader::ObjReader(const string &fileName, const string &path, const ObjOptions &options)
{
   std::ifstream   is((path + fileName).c_str());

   if (!is)
      BSG_THROW("File '" + fileName + "' not found");

   std::stringstream ss;
   ss << is.rdbuf();

   Initialise(ss.str().c_str(), path, options);
}

ObjReader::ObjReader(istream &is, const string &path, const ObjOptions &options)
{
   std::stringstream ss;
   ss << is.rdbuf();

   Initialise(ss.str().c_str(), path, options);
}

bool ObjReader::HasNormals(const Object &object) const
{
   if (object.size() == 0)
      return false;

   const Face &face = object[0];

   return face[0].VN() != NO_INDEX;
}

bool ObjReader::HasTextureCoordinates(const Object &object) const
{
   if (object.size() == 0)
      return false;

   const Face &face = object[0];

   return face[0].VT() != NO_INDEX;
}

void ObjReader::LoadMaterials(const string &file)
{
   ifstream is(file.c_str());

   if (is.bad())
      BSG_THROW("Couldn't locate material file '" << file << "'");

   while (is.good())
   {
      string   line;

      getline(is, line, '\n');

      if (line.size() > 0)
      {
         stringstream    line_ss(line);
         string          keyword;

         line_ss >> keyword;

         if (keyword == "newmtl")
         {
            string   name;
            line_ss >> name;

            if (m_materials.find(name) == m_materials.end())
            {
               m_materials[name] = ObjMaterial(is);
            }
         }
      }
   }
}

const ObjMaterial &ObjReader::GetMaterial(const string &name) const
{
   std::map<std::string, ObjMaterial>::const_iterator iter = m_materials.find(name);

   if (iter == m_materials.end())
      BSG_THROW("Cannot find material '" + name + "'");

   return iter->second;
}

const ObjReader::MaterialMap &ObjReader::GetMaterials() const
{
   return m_materials;
}

ObjMaterial::ObjMaterial()
{
   Initialise();
}

ObjMaterial::ObjMaterial(std::istream &is)
{
   bool  end = false;

   Initialise();

   while (!end && is.good())
   {
      string      line;
      streampos   keyPos = is.tellg();

      getline(is, line, '\n');

      if (line.size() > 0)
      {
         stringstream    line_ss(line);
         string          keyword;

         line_ss >> keyword;

         if (keyword == "Ka")
         {
            line_ss >> m_Ka.X() >> m_Ka.Y() >> m_Ka.Z();
         }
         else if (keyword == "Kd")
         {
            line_ss >> m_Kd.X() >> m_Kd.Y() >> m_Kd.Z();
         }
         else if (keyword == "Ks")
         {
            line_ss >> m_Ks.X() >> m_Ks.Y() >> m_Ks.Z();
         }
         else if (keyword == "Tf")
         {
            line_ss >> m_Tf.X() >> m_Tf.Y() >> m_Tf.Z();
         }
         else if (keyword == "illum")
         {
            line_ss >> m_illum;
         }
         else if (keyword == "d")
         {
            string   opt;

            line_ss >> opt;

            if (opt == "-halo")
            {
               m_halo = true;
               line_ss >> m_d;
            }
            else
            {
               stringstream opt_ss(opt);
               opt_ss >> m_d;
            }
         }
         else if (keyword == "Ns")
         {
            line_ss >> m_Ns;
         }
         else if (keyword == "sharpness")
         {
            line_ss >> m_sharpness;
         }
         else if (keyword == "Ni")
         {
            line_ss >> m_Ni;
         }
         else if (keyword == "map_Ka")
         {
            m_map_Ka.push_back(ObjTexMap(line_ss));
         }
         else if (keyword == "map_Kd")
         {
            m_map_Kd.push_back(ObjTexMap(line_ss));
         }
         else if (keyword == "map_Ks")
         {
            m_map_Ks.push_back(ObjTexMap(line_ss));
         }
         else if (keyword == "map_Ns")
         {
            m_map_Ns.push_back(ObjTexMap(line_ss));
         }
         else if (keyword == "map_d")
         {
            m_map_d.push_back(ObjTexMap(line_ss));
         }
         else if (keyword == "disp" || keyword == "map_Disp")
         {
            m_disp.push_back(ObjTexMap(line_ss));
         }
         else if (keyword == "decal" || keyword == "map_Decal")
         {
            m_decal.push_back(ObjTexMap(line_ss));
         }
         else if (keyword == "bump" || keyword == "map_Bump")
         {
            m_bump.push_back(ObjTexMap(line_ss));
         }
         else if (keyword == "newmtl")
         {
            // Reset so we read newmtl again
            end = true;
            is.seekg(keyPos);
         }
      }
   }
}

void ObjMaterial::Initialise()
{
   m_Ka = Vec3(1.0f, 1.0f, 1.0f);
   m_Kd = Vec3(1.0f, 1.0f, 1.0f);
   m_Ks = Vec3(0.0f, 0.0f, 0.0f);
   m_Tf = Vec3(1.0f, 1.0f, 1.0f);

   m_illum     = 0;
   m_d         = 1.0f;
   m_halo      = false;
   m_Ns        = 10.0f;
   m_Ni        = 1.0f;
   m_sharpness = 60.0f;
}


ObjTexMap::ObjTexMap()
{
   Initialise();
}

static bool OnOff(istream &is)
{
   string   flag;

   is >> flag;

   return flag == "on";
}

static ObjTexMap::eChan ImfChan(istream &is)
{
   string   type;
   is >> type;

   if (type == "r") return ObjTexMap::R;
   if (type == "g") return ObjTexMap::G;
   if (type == "b") return ObjTexMap::B;
   if (type == "m") return ObjTexMap::M;
   if (type == "l") return ObjTexMap::L;
   if (type == "z") return ObjTexMap::Z;

   return ObjTexMap::L;
}

static ObjTexMap::eMapType MapType(istream &is)
{
   string   type;
   is >> type;

   if (type == "sphere")      return ObjTexMap::SPHERE;
   if (type == "cube_top")    return ObjTexMap::CUBE_TOP;
   if (type == "cube_bottom") return ObjTexMap::CUBE_BOTTOM;
   if (type == "cube_front")  return ObjTexMap::CUBE_FRONT;
   if (type == "cube_back")   return ObjTexMap::CUBE_BACK;
   if (type == "cube_left")   return ObjTexMap::CUBE_LEFT;
   if (type == "cube_right")  return ObjTexMap::CUBE_RIGHT;

   return ObjTexMap::NONE;
}

ObjTexMap::ObjTexMap(stringstream &line_ss)
{
   Initialise();

   while (line_ss.good())
   {
      uint32_t lookNext = line_ss.peek();

      // Is it a file name?
      if (lookNext != '-')
      {
         string fileName = GetToEnd(line_ss);

         switch (m_mapType)
         {
         default:
         case NONE:
            m_mapName        = fileName;
            break;
         case SPHERE:
            m_sphereName     = fileName;
            break;
         case CUBE_TOP:
            m_cubeTopName    = fileName;
            break;
         case CUBE_BOTTOM:
            m_cubeBottomName = fileName;
            break;
         case CUBE_FRONT:
            m_cubeFrontName  = fileName;
            break;
         case CUBE_BACK:
            m_cubeBackName   = fileName;
            break;
         case CUBE_LEFT:
            m_cubeLeftName   = fileName;
            break;
         case CUBE_RIGHT:
            m_cubeRightName  = fileName;
            break;
         }
      }
      else
      {
         string keyword;

         line_ss >> keyword;

         if (keyword == "-blendu")
            m_blendU = OnOff(line_ss);
         else if (keyword == "-blendv")
            m_blendV = OnOff(line_ss);
         else if (keyword == "-bm")
            line_ss >> m_bm;
         else if (keyword == "-boost")
            line_ss >> m_boost;
         else if (keyword == "-cc")
            m_cc = OnOff(line_ss);
         else if (keyword == "-clamp")
            m_clamp = OnOff(line_ss);
         else if (keyword == "-imfchan")
            m_imfchan = ImfChan(line_ss);
         else if (keyword == "-mm")
         {
            line_ss >> m_base;
            line_ss >> m_gain;
         }
         else if (keyword == "-o")
         {
            line_ss >> m_o.X() >> m_o.Y() >> m_o.Z();
         }
         else if (keyword == "-s")
         {
            line_ss >> m_s.X() >> m_s.Y() >> m_s.Z();
         }
         else if (keyword == "-t")
         {
            line_ss >> m_t.X() >> m_t.Y() >> m_t.Z();
         }
         else if (keyword == "-type")
         {
            m_mapType = MapType(line_ss);
         }
         else if (keyword == "-texres")
         {
            line_ss >> m_texres;
         }
      }
   }
}

void ObjTexMap::Initialise()
{
   m_blendU = true;                    // Turns on/off texture "blending" in U and V direction
   m_blendV = true;
   m_bm     = 1.0f;                    // Bump multiplier
   m_boost  = 0.0f;                    // Increases "clarity"
   m_cc     = false;                   // Color correction control
   m_clamp  = false;                   // Turns on texture coordinate clamping
   m_imfchan = L;                      // Channel used for bump
   m_base    = 0.0f;                   // Offset and scale for texture
   m_gain    = 1.0f;
   m_o       = Vec3(0.0f, 0.0f, 0.0f); // Offset for texture coordinates
   m_s       = Vec3(1.0f, 1.0f, 1.0f); // Scale of the texture coordinates
   m_t       = Vec3(0.0f, 0.0f, 0.0f); // Turbulence

   m_texres  = 0;                      // Requested resolution

   m_mapType = NONE;
}

} // namespace bsg
