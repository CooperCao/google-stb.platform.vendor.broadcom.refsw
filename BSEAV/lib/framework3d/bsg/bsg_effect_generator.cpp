/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_effect_generator.h"
#include "bsg_exception.h"
#include "bsg_gl_texture.h"
#include "bsg_image.h"

using namespace std;

namespace bsg
{

static std::string Kind(EffectSemantics::eSemantic semantic)
{
   switch (semantic)
   {
   case EffectSemantics::eUNKNOWN_SEMANTIC:            // Bit of a hack -- only samplers will have this semantic
   case EffectSemantics::eMATRIX4_MODEL:
   case EffectSemantics::eMATRIX4_VIEW:
   case EffectSemantics::eMATRIX4_PROJECTION:
   case EffectSemantics::eMATRIX4_MODEL_VIEW:
   case EffectSemantics::eMATRIX4_VIEW_PROJECTION:
   case EffectSemantics::eMATRIX4_MODEL_VIEW_PROJECTION:
   case EffectSemantics::eMATRIX3_INVT_MODEL:
   case EffectSemantics::eMATRIX3_INVT_VIEW:
   case EffectSemantics::eMATRIX3_INVT_MODEL_VIEW:
   case EffectSemantics::eSCALAR_OPACITY:
   case EffectSemantics::eSCALAR_USER:
   case EffectSemantics::eVECTOR2_USER:
   case EffectSemantics::eVECTOR3_USER:
   case EffectSemantics::eVECTOR4_USER:
   case EffectSemantics::eMATRIX2_USER:
   case EffectSemantics::eMATRIX3_USER:
   case EffectSemantics::eMATRIX4_USER:
   case EffectSemantics::eVECTOR4_SCREEN_SIZE:
   case EffectSemantics::eVECTOR4_QUAD_OFFSET:
#ifdef BSG_USE_ES3
   case EffectSemantics::eMATRIX2x3_USER:
   case EffectSemantics::eMATRIX2x4_USER:
   case EffectSemantics::eMATRIX3x2_USER:
   case EffectSemantics::eMATRIX3x4_USER:
   case EffectSemantics::eMATRIX4x2_USER:
   case EffectSemantics::eMATRIX4x3_USER:
#endif
      return "uniform";

   case EffectSemantics::eVATTR_POSITION:
   case EffectSemantics::eVATTR_NORMAL:
   case EffectSemantics::eVATTR_TANGENT:
   case EffectSemantics::eVATTR_BINORMAL:
   case EffectSemantics::eVATTR_TEXCOORD1:
   case EffectSemantics::eVATTR_TEXCOORD2:
   case EffectSemantics::eVATTR_TEXCOORD3:
   case EffectSemantics::eVATTR_COLOR:
   case EffectSemantics::eVATTR_USER1:
   case EffectSemantics::eVATTR_USER2:
   case EffectSemantics::eVATTR_USER3:
   case EffectSemantics::eVATTR_USER4:
   case EffectSemantics::eVATTR_USER5:
   case EffectSemantics::eVATTR_USER6:
      return "attribute";
   }

   return "";
}

std::string Type(EffectSemantics::eSemantic semantic)
{
   switch (semantic)
   {
   case EffectSemantics::eUNKNOWN_SEMANTIC:
      return "";

   case EffectSemantics::eMATRIX4_MODEL:
   case EffectSemantics::eMATRIX4_VIEW:
   case EffectSemantics::eMATRIX4_PROJECTION:
   case EffectSemantics::eMATRIX4_MODEL_VIEW:
   case EffectSemantics::eMATRIX4_VIEW_PROJECTION:
   case EffectSemantics::eMATRIX4_MODEL_VIEW_PROJECTION:
   case EffectSemantics::eMATRIX4_USER:
      return "mat4";

#ifdef BSG_USE_ES3
   case EffectSemantics::eMATRIX2x3_USER:
      return "mat2x3";
   case EffectSemantics::eMATRIX2x4_USER:
      return "mat2x4";
   case EffectSemantics::eMATRIX3x2_USER:
      return "mat3x2";
   case EffectSemantics::eMATRIX3x4_USER:
      return "mat3x4";
   case EffectSemantics::eMATRIX4x2_USER:
      return "mat4x2";
   case EffectSemantics::eMATRIX4x3_USER:
      return "mat4x3";
#endif

   case EffectSemantics::eMATRIX3_INVT_MODEL:
   case EffectSemantics::eMATRIX3_INVT_VIEW:
   case EffectSemantics::eMATRIX3_INVT_MODEL_VIEW:
   case EffectSemantics::eMATRIX3_USER:
      return "mat3";

   case EffectSemantics::eSCALAR_OPACITY:
   case EffectSemantics::eSCALAR_USER:
      return "float";

   case EffectSemantics::eVECTOR2_USER:
      return "vec2";

   case EffectSemantics::eVECTOR3_USER:
      return "vec3";

   case EffectSemantics::eVECTOR4_USER:
   case EffectSemantics::eVECTOR4_SCREEN_SIZE:
   case EffectSemantics::eVECTOR4_QUAD_OFFSET:
      return "vec4";

   case EffectSemantics::eMATRIX2_USER:
      return "mat2";

   case EffectSemantics::eVATTR_POSITION:
   case EffectSemantics::eVATTR_NORMAL:
   case EffectSemantics::eVATTR_TANGENT:
   case EffectSemantics::eVATTR_BINORMAL:
   case EffectSemantics::eVATTR_TEXCOORD1:
   case EffectSemantics::eVATTR_TEXCOORD2:
   case EffectSemantics::eVATTR_TEXCOORD3:
   case EffectSemantics::eVATTR_COLOR:
   case EffectSemantics::eVATTR_USER1:
   case EffectSemantics::eVATTR_USER2:
   case EffectSemantics::eVATTR_USER3:
   case EffectSemantics::eVATTR_USER4:
   case EffectSemantics::eVATTR_USER5:
   case EffectSemantics::eVATTR_USER6:
      return "";
   }

   return "";
}

EffectGenerator::Variable::Variable(const string &name, const string &type, EffectSemantics::eSemantic semantic, eUsage usage) :
   m_name(name),
   m_type(type),
   m_semantic(semantic),
   m_usage(usage)
{
   std::string autoType = Type(semantic);

   if (autoType != "" && autoType != type)
      BSG_THROW("Variable declared with type incompatible with its semantics");
}

EffectGenerator::Variable::Variable(const string &name, EffectSemantics::eSemantic semantic, eUsage usage) :
   m_name(name),
   m_type(Type(semantic)),
   m_semantic(semantic),
   m_usage(usage)
{
   if (m_type == "")
      BSG_THROW("Variable declared without type when one is required");
}

   EffectGenerator::Variable::Variable(const string &name, const string &type, eUsage usage) :
   m_name(name),
   m_type(type),
   m_semantic(EffectSemantics::eUNKNOWN_SEMANTIC),
   m_usage(usage)
{
}

std::string EffectGenerator::Variable::Semantics() const
{
   string   semantic = ToString(m_semantic);

   if (semantic != "")
      return m_name + "  =  " + semantic + ";";
   else
      return "";
}

std::string EffectGenerator::Variable::DeclareVert() const
{
   if (m_usage == eVERTEX || m_usage == eVERTEX_AND_FRAGMENT)
      return Kind(m_semantic) + "    " + m_type + "    " + m_name + ";";
   else
      return "";
}

std::string EffectGenerator::Variable::DeclareFrag() const
{
   if (m_usage == eFRAGMENT || m_usage == eVERTEX_AND_FRAGMENT)
      return Kind(m_semantic) + "    " + m_type + "    " + m_name + ";";
   else
      return "";
}

void EffectGenerator::Register(const string &name, EffectSemantics::eSemantic semantic , eUsage usage, uint32_t pass)
{
   vector<Variable>  &variables = m_variables[pass];

   variables.push_back(Variable(name, semantic, usage));
}

void EffectGenerator::Register(const string &name, const std::string &type, EffectSemantics::eSemantic semantic, eUsage usage, uint32_t pass)
{
   vector<Variable>  &variables = m_variables[pass];

   variables.push_back(Variable(name, type, semantic, usage));
}

void EffectGenerator::RegisterSampler(const string &name, const string &type, eUsage usage, uint32_t pass)
{
   vector<Variable>  &variables = m_variables[pass];

   variables.push_back(Variable(name, type, usage));
}

void EffectGenerator::Varying(const string &name, const string &type, uint32_t pass)
{
   vector<string>  &variables = m_varyings[pass];

   variables.push_back(type + "    " + name);
}

void EffectGenerator::Generate(ostream &os)
{
   m_indent = 0;
   m_os     = &os;

   Begin("OPTIONS");
      Options();
   End();
   Passes();
}

void EffectGenerator::Indent() const
{
   if (m_os != 0)
   {
      for (uint32_t i = 0; i < m_indent; ++i)
         (*m_os) << "   ";
   }
}

void EffectGenerator::Begin(const string &label)
{
   PrintLn(label);
   PrintLn("{");
   m_indent += 1;
}

void EffectGenerator::Begin()
{
   PrintLn("{");
   m_indent += 1;
}

void EffectGenerator::End(bool extraNewline)
{
   m_indent -= 1;
   if (extraNewline)
      PrintLn("}\n");
   else
      PrintLn("}");
}

template <class T>
void EffectGenerator::Print(const T &data) const
{
   if (m_os != 0)
   {
      Indent();
      (*m_os) << data;
   }
}

template <class T>
void EffectGenerator::PrintLn(const T &data) const
{
   if (m_os != 0)
   {
      Indent();
      (*m_os) << data << "\n";
   }
}

void EffectGenerator::PrintLines(const vector<string> &lines) const
{
   for (uint32_t l = 0; l < lines.size(); ++l)
      PrintLn(lines[l]);
}

void EffectGenerator::Options()
{
   vector<string> lines;

   m_options.ToLines(lines);
   PrintLines(lines);
}

void EffectGenerator::Passes()
{
   Begin("PASS 0");
      Pass(0);
   End();
}

void EffectGenerator::Semantics(uint32_t p)
{
   const vector<Variable>  &variables = m_variables[p];

   for (uint32_t v = 0; v < variables.size(); ++v)
   {
      std::string decl = variables[v].Semantics();

      if (decl != "")
         PrintLn(decl);
   }
}

void EffectGenerator::DeclareVert(uint32_t p)
{
   const vector<Variable>  &variables = m_variables[p];

   for (uint32_t v = 0; v < variables.size(); ++v)
   {
      std::string decl = variables[v].DeclareVert();

      if (decl != "")
         PrintLn(decl);
   }

   const vector<string> &varyings = m_varyings[p];

   for (uint32_t v = 0; v < varyings.size(); ++v)
      PrintLn("varying    "  + varyings[v] + ";");
}

void EffectGenerator::DeclareFrag(uint32_t p)
{
   const vector<Variable>  &variables = m_variables[p];

   for (uint32_t v = 0; v < variables.size(); ++v)
   {
      std::string decl = variables[v].DeclareFrag();

      if (decl != "")
         PrintLn(decl);
   }

   const vector<string> &varyings = m_varyings[p];

   for (uint32_t v = 0; v < varyings.size(); ++v)
      PrintLn("varying    "  + varyings[v] + ";");
}

void EffectGenerator::Pass(uint32_t p)
{
   Begin("SEMANTICS");
      Semantics(p);
   End();

   Begin("STATE");
      State(p);
   End();

   Samplers(p);

   Begin("VERTEX_SHADER");
      DeclareVert(p);
      VertexShader(p);
   End();

   Begin("FRAGMENT_SHADER");
      PrintLn("precision mediump float;");
      DeclareFrag(p);
      FragmentShader(p);
   End(false);
}

void EffectGenerator::Samplers(uint32_t /*p*/)
{
   // Do nothing by default
}

void EffectGenerator::State(uint32_t p)
{
   if (p < m_states.size())
   {
      vector<string> lines;

      m_states[p].ToLines(lines);
      PrintLines(lines);
   }
}

ObjEffectGenerator::ObjEffectGenerator(const ObjMaterial &material, const ObjMaterialOptions &options) :
   EffectGenerator(options.GetEffectOptions(), options.GetPassStates()),
   m_material(material),
   m_options(options)
{
   Register("u_mvpMatrix",         EffectSemantics::eMATRIX4_MODEL_VIEW_PROJECTION, EffectGenerator::eVERTEX);
   Register("u_diffuseColor",      EffectSemantics::eVECTOR3_USER,                  EffectGenerator::eFRAGMENT);
   Register("a_position", "vec4",  EffectSemantics::eVATTR_POSITION,                EffectGenerator::eVERTEX);

   if (Lighting())
   {
      Register("u_mvMatrix",       EffectSemantics::eMATRIX4_MODEL_VIEW,      EffectGenerator::eVERTEX);
      Register("u_imvtMatrix",     EffectSemantics::eMATRIX3_INVT_MODEL_VIEW, EffectGenerator::eVERTEX);

      EffectGenerator::eUsage  usage = FragmentLighting() ? EffectGenerator::eFRAGMENT : EffectGenerator::eVERTEX;

      Register("u_lightPos",       EffectSemantics::eVECTOR3_USER,            EffectGenerator::eVERTEX);
      Register("u_ambientColor",   EffectSemantics::eVECTOR3_USER,            EffectGenerator::eFRAGMENT);
      Register("u_specularColor",  EffectSemantics::eVECTOR3_USER,            usage);
      Register("u_specularPower",  EffectSemantics::eSCALAR_USER,             usage);

      Register("a_normal", "vec3", EffectSemantics::eVATTR_NORMAL,            EffectGenerator::eVERTEX);

      if (FragmentLighting())
      {
         Varying("v_v2l", "vec3");
         Varying("v_v2e", "vec3");
         Varying("v_normal", "vec3");
      }
      else
      {
         Varying("v_specular", "vec3");
         Varying("v_diffuse", "float");
      }
   }

   if (Texturing())
   {
      Register("a_tc", "vec2",     EffectSemantics::eVATTR_TEXCOORD1,         EffectGenerator::eVERTEX);

      Varying("v_tc", "vec2");
   }
}

bool ObjEffectGenerator::Texturing() const
{
   return m_material.m_map_Kd.size() != 0;
}

bool ObjEffectGenerator::Lighting() const
{
   return m_options.GetNumLights() != 0;
}

bool ObjEffectGenerator::FragmentLighting() const
{
   return m_options.GetFragmentLighting();
}

uint32_t ObjEffectGenerator::NumLights() const
{
   return m_options.GetNumLights();
}

void ObjEffectGenerator::Samplers(uint32_t /*p*/)
{
   if (Texturing())
   {
      Begin("SAMPLER_2D u_diffuseTex");

         PrintLn("Unit = 0;");
         if (m_material.m_map_Kd[0].m_clamp)
            PrintLn("Wrap = CLAMP, CLAMP;");
         else
            PrintLn("Wrap = REPEAT, REPEAT;");

         // TODO make this configurable
         PrintLn("Filter = LINEAR_MIPMAP_NEAREST, LINEAR;");

      End();

      RegisterSampler("u_diffuseTex", "sampler2D", EffectGenerator::eFRAGMENT);
   }
}

void ObjEffectGenerator::VertexShader(uint32_t /*p*/)
{
   PrintLn("void main()");
   Begin();

      if (Lighting())
      {
         if (FragmentLighting())
         {
            PrintLn("vec4  pos = u_mvMatrix * a_position;");
            PrintLn("v_v2e    = normalize(-pos.xyz);");
            PrintLn("v_normal = normalize(u_imvtMatrix * a_normal);");
            PrintLn("v_v2l    = normalize(u_lightPos - pos.xyz);");
         }
         else
         {
            PrintLn("vec4  pos    = u_mvMatrix * a_position;");
            PrintLn("vec3  v2e    = normalize(-pos.xyz);");
            PrintLn("vec3  normal = normalize(u_imvtMatrix * a_normal);");
            PrintLn("vec3  v2l    = normalize(u_lightPos - pos.xyz);");
            PrintLn("");
            PrintLn("float diff   = max(0.0, dot(v2l, normal));");
            PrintLn("");
            PrintLn("vec3  h      = normalize(v2l + v2e);");
            PrintLn("float spec   = pow(max(0.0, dot(normal, h)), u_specularPower);");

            PrintLn("v_specular   = u_specularColor * spec;");
            PrintLn("v_diffuse    = diff;");
         }
      }

      if (Texturing())
         PrintLn("v_tc = a_tc;");

      PrintLn("gl_Position = u_mvpMatrix * a_position;");
   End();
}

void ObjEffectGenerator::FragmentShader(uint32_t /*p*/)
{
   PrintLn("void main()");
   Begin();

      if (Lighting())
      {
         if (FragmentLighting())
         {
            PrintLn("vec3  v2l    = normalize(v_v2l);");
            PrintLn("vec3  normal = normalize(v_normal);");
            PrintLn("vec3  v2e    = normalize(v_v2e);");

            PrintLn("float diff   = max(0.0, dot(v2l, normal));");
            PrintLn("");
            PrintLn("vec3  h      = normalize(v2l + v2e);");
            PrintLn("float spec   = pow(max(0.0, dot(normal, h)), u_specularPower);");

            PrintLn("vec3  v_specular   = u_specularColor * spec;");
            PrintLn("float v_diffuse    = diff;");
         }
         else
         {
            // Nothing
         }
      }

      if (Texturing())
         PrintLn("vec3 col = v_diffuse * texture2D(u_diffuseTex, v_tc).rgb;");
      else
         PrintLn("vec3 col = vec3(v_diffuse);");

      PrintLn("col *= u_diffuseColor;");

      if (Lighting())
      {
         PrintLn("col += v_specular;");
         PrintLn("col += u_ambientColor;");
      }

      PrintLn("gl_FragColor = vec4(col, 1.0);");

   End();
}

ObjMaterialFactory::ObjMaterialFactory(const ObjReader &reader, const ObjMaterialOptions &options, const string &path) :
   m_path(path)
{
   const ObjReader::MaterialMap  &map = reader.GetMaterials();

   for (ObjReader::MaterialMap::const_iterator iter = map.begin(); iter != map.end(); ++iter)
      MakeMaterial(iter->first, iter->second, options);
}

void ObjMaterialFactory::MakeMaterial(const string &name, const ObjMaterial &objMaterial, const ObjMaterialOptions &options)
{
   MaterialHandle material(name);

   if (!material.IsNull())
      return;

   material = MaterialHandle(New, name);

   // Need to keep references to our materials or they will be deleted!
   m_materials.push_back(material);

   ObjEffectGenerator   gen(objMaterial, options);

   stringstream   effect_ss;
   gen.Generate(effect_ss);

   if (options.GetDebug())
      cout << effect_ss.str();

   EffectHandle   effect(New);
   effect->Load(effect_ss);

   material->SetEffect(effect);
   material->SetUniformValue("u_diffuseColor",  objMaterial.m_Kd);

   if (objMaterial.DiffuseTexturing())
   {
      TextureHandle  diffuseTexture(New);

      diffuseTexture->SetAutoMipmap(true);
      diffuseTexture->TexImage2D(Image(objMaterial.m_map_Kd[0].m_mapName, options.GetMapKdFormat()));

      material->SetTexture("u_diffuseTex", diffuseTexture);
   }

   if (options.HasLighting())
   {
      material->SetUniformValue("u_specularColor", objMaterial.m_Ks);
      material->SetUniformValue("u_specularPower", objMaterial.m_Ns);
      material->SetUniformValue("u_lightPos",      Vec3(-10.0f, 10.0f, 0.0f));
      material->SetUniformValue("u_ambientColor",  objMaterial.m_Ka);
   }
}

}
