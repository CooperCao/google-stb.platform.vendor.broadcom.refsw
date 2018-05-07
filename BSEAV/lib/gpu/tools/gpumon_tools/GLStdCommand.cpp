/******************************************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "GLStdCommand.h"
#include "GLStdCommandMap.h"
#include "gl_public_api.h"

#include <assert.h>
#include <iomanip>
#include <sstream>

using namespace std;

map<string, uint32_t>   GLStdCommand::m_allEnums;

// std::numberStr doesn't seem to exist on old mips compilers, so roll our own
template <typename T>
std::string numberStr(T num)
{
   std::ostringstream stm;
   stm << num;
   return stm.str();
}

GLStdCommand::GLStdCommand(const Packet &packet, uint32_t context)
{
   assert(packet.NumItems() > 0);
   assert(packet.Type() == eAPI_FUNCTION);

   m_packet = packet;
   m_context = context;
   m_markerBits = 0;

   m_command = packet.Item(0).GetFunc();
}

#define EnumCase(e) case e : return #e; break;
#define EnumValCase(e, v) case ((uint32_t)e) : return v; break;

string GLStdCommand::MapEnum(uint32_t e, eGLCommand cmd)
{
   if (cmd >= cmd_eglGetError && cmd <= cmd_eglGetSyncAttribKHR)
      return GLStdCommand::MapEGLEnum(e, cmd);

   switch(e)
   {
   // Handle all the special cases first - i.e. ones with duplicate values
   case GL_ZERO:
   {
      switch (cmd)
      {
      case cmd_glIsBuffer              :
      case cmd_glIsEnabled             :
      case cmd_glIsFramebuffer         :
      case cmd_glIsProgram             :
      case cmd_glIsRenderbuffer        :
      case cmd_glIsShader              :
      case cmd_glColorMask             :
      case cmd_glDepthMask             :
      case cmd_glGetBooleanv           :
      case cmd_glSampleCoverage        :
      case cmd_glSampleCoveragex       :
      case cmd_glUniformMatrix2fv      :
      case cmd_glUniformMatrix3fv      :
      case cmd_glUniformMatrix4fv      :
      case cmd_glVertexAttribPointer   :
      case cmd_glIsRenderbufferOES     :
      case cmd_glIsFramebufferOES      :
      case cmd_glGetQueryObjectuiv     :
      case cmd_glGetVertexAttribiv     :
      case cmd_glGetProgramPipelineiv  :
      case cmd_glIsTexture             : return "GL_FALSE"; break;
      case cmd_glGetError              : return "GL_NO_ERROR"; break;
      case cmd_glBlendColor            :
      case cmd_glBlendEquation         :
      case cmd_glBlendEquationSeparate :
      case cmd_glBlendFunc             :
      case cmd_glBlendFuncSeparate     : return "GL_ZERO"; break;
      case cmd_glTexParameterf         :
      case cmd_glGetFramebufferParameteriv:
      case cmd_glGetFramebufferAttachmentParameteriv:
      case cmd_glCheckFramebufferStatus:
      case cmd_glGetIntegerv           : return "GL_NONE"; break;
      default                          : return "GL_POINTS"; break;
      }
   }
   case GL_ONE:
   {
      switch (cmd)
      {
      case cmd_glIsBuffer              :
      case cmd_glIsEnabled             :
      case cmd_glIsFramebuffer         :
      case cmd_glIsProgram             :
      case cmd_glIsRenderbuffer        :
      case cmd_glIsShader              :
      case cmd_glColorMask             :
      case cmd_glDepthMask             :
      case cmd_glGetBooleanv           :
      case cmd_glSampleCoverage        :
      case cmd_glSampleCoveragex       :
      case cmd_glUniformMatrix2fv      :
      case cmd_glUniformMatrix3fv      :
      case cmd_glUniformMatrix4fv      :
      case cmd_glVertexAttribPointer   :
      case cmd_glIsRenderbufferOES     :
      case cmd_glIsFramebufferOES      :
      case cmd_glGetQueryObjectuiv     :
      case cmd_glGetVertexAttribiv     :
      case cmd_glGetProgramPipelineiv  :
      case cmd_glIsTexture             : return "GL_TRUE"; break;
      case cmd_glBlendColor            :
      case cmd_glBlendEquation         :
      case cmd_glBlendEquationSeparate :
      case cmd_glBlendFunc             :
      case cmd_glBlendFuncSeparate     : return "GL_ONE"; break;
      case cmd_glMapBufferRange        : return "GL_MAP_READ_BIT"; break;
      case cmd_glClientWaitSync        : return "GL_SYNC_FLUSH_COMMANDS_BIT"; break;
#if GL_ES_VERSION_3_1
      case cmd_glUseProgramStages      : return "GL_VERTEX_SHADER_BIT"; break;
      case cmd_glMemoryBarrier         : return "GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT"; break;
#endif
      default                          : return "GL_LINES"; break;
      }
   }

   case GL_LINE_LOOP:
   {
      switch (cmd)
      {
      case cmd_glMapBufferRange   : return "GL_MAP_WRITE_BIT";
#if GL_ES_VERSION_3_1
      case cmd_glMemoryBarrier    : return "GL_ELEMENT_ARRAY_BARRIER_BIT";
      case cmd_glUseProgramStages : return "GL_FRAGMENT_SHADER_BIT";
#endif
      default: return "GL_LINE_LOOP";
      }
   }

   case GL_TRIANGLES:
   {
      switch (cmd)
      {
      case cmd_glMapBufferRange : return "GL_MAP_INVALIDATE_RANGE_BIT";
#if GL_ES_VERSION_3_1
      case cmd_glMemoryBarrier  : return "GL_UNIFORM_BARRIER_BIT";
#endif
      default: return "GL_TRIANGLES";
      }
   }

#if GL_ES_VERSION_3_0
   case GL_MAP_INVALIDATE_BUFFER_BIT:
   {
      switch (cmd)
      {
#if GL_ES_VERSION_3_1
      case cmd_glMemoryBarrier: return "GL_TEXTURE_FETCH_BARRIER_BIT";
#endif
      default: return "GL_MAP_INVALIDATE_BUFFER_BIT";
      }
   }
#endif

   case GL_LIGHT0:
   {
      switch (cmd)
      {
      case cmd_glClear  : return "GL_COLOR_BUFFER_BIT";
      default           : return "GL_LIGHT0";
      }
   }

#if GL_ES_VERSION_3_1
   case GL_COMPUTE_SHADER_BIT:
   {
      switch (cmd)
      {
      case cmd_glMapBufferRange    : return "GL_MAP_UNSYNCHRONIZED_BIT";
      case cmd_glMemoryBarrier     : return "GL_SHADER_IMAGE_ACCESS_BARRIER_BIT";
      default                      : return "GL_COMPUTE_SHADER_BIT";
      }
   }
#endif

   case GL_DEPTH_BUFFER_BIT:
   {
      switch (cmd)
      {
#if GL_ES_VERSION_3_1
      case cmd_glMemoryBarrier   : return "GL_TEXTURE_UPDATE_BARRIER_BIT";
#endif
      default                    : return "GL_DEPTH_BUFFER_BIT";
      }
   }

   case GL_EXP:
   {
      switch (cmd)
      {
#if GL_ES_VERSION_3_1
      case cmd_glMemoryBarrier   : return "GL_TRANSFORM_FEEDBACK_BARRIER_BIT";
#endif
      default                    : return "GL_EXP";
      }
   }

   case GL_NEVER:
   {
      switch (cmd)
      {
#if GL_ES_VERSION_3_1
      case cmd_glMemoryBarrier   : return "GL_BUFFER_UPDATE_BARRIER_BIT";
#endif
      default                    : return "GL_NEVER";
      }
   }

   case GL_STENCIL_BUFFER_BIT:
   {
      switch (cmd)
      {
#if GL_ES_VERSION_3_1
      case cmd_glMemoryBarrier   : return "GL_FRAMEBUFFER_BARRIER_BIT";
#endif
      default                    : return "GL_STENCIL_BUFFER_BIT";
      }
   }

#if GL_ES_VERSION_3_1
   case GL_ALL_SHADER_BITS:
   {
      switch (cmd)
      {
      case cmd_glGetUniformBlockIndex : return "GL_INVALID_INDEX";
#if GL_ES_VERSION_3_1
      case cmd_glMemoryBarrier        : return "GL_ALL_BARRIER_BITS";
#endif
      default                         : return "GL_ALL_SHADER_BITS";
      }
   }
#endif

#if GL_ES_VERSION_3_1
   case GL_TEXTURE_WIDTH:
   {
      switch (cmd)
      {
#if GL_ES_VERSION_3_1
      case cmd_glMemoryBarrier   : return "GL_ATOMIC_COUNTER_BARRIER_BIT";
#endif
      default                    : return "GL_TEXTURE_WIDTH";
      }
   }
#endif

#if GL_ES_VERSION_3_0
   EnumCase(GL_MAP_FLUSH_EXPLICIT_BIT);                        // =  16
#endif
   //EnumCase(GL_TESS_EVALUATION_SHADER_BIT_OES);              // =  16

   EnumCase(GL_FRAMEBUFFER_BINDING);                           // =  36006
   //EnumCase(GL_DRAW_FRAMEBUFFER_BINDING);                    // =  36006
   //EnumCase(GL_FRAMEBUFFER_BINDING_OES);                     // =  36006

   default:
      break;
   }

   // Now use the auto-generated code to lookup the unique GL enum names
   // This table content is auto-generated by running ./gen_hook_tables.py in v3dv3/tools/v3d/hook_codegen
   switch(e)
   {
   #include "mapglenums.inc"

   default: return GLStdCommand::MapEGLEnum(e, cmd); break;
   }
}

string GLStdCommand::MapEGLEnum(uint32_t e, eGLCommand cmd)
{
   // Deal with special cases first
   switch(e)
   {
   case EGL_FALSE: return "EGL_FALSE";
   case EGL_TRUE:
   {
      switch (cmd)
      {
      case cmd_eglChooseConfig      : return "EGL_OPENGL_ES_BIT/EGL_PBUFFER_BIT"; break;
      default                       : return "EGL_TRUE"; break;
      }
   }
   case (uint32_t)EGL_DONT_CARE:
   {
      switch (cmd)
      {
      case cmd_eglChooseConfig      : return "EGL_DONT_CARE"; break;
      default                       : return "EGL_UNKNOWN"; break;
      }
   }

   case EGL_PLATFORM_X11_EXT:
   {
      switch (cmd)
      {
      case cmd_eglCreateImageKHR       : return "EGL_WAYLAND_BUFFER_WL";
      default                          : return "EGL_PLATFORM_X11_EXT";
      }
   }

   case EGL_PLATFORM_X11_SCREEN_EXT:
   {
      switch (cmd)
      {
      case cmd_eglCreateImageKHR       : return "EGL_WAYLAND_PLANE_WL";
      default                          : return "EGL_PLATFORM_X11_SCREEN_EXT";
      }
   }
   case EGL_PLATFORM_WAYLAND_EXT:
   {
      switch (cmd)
      {
      case cmd_eglQueryWaylandBufferWL : return "EGL_TEXTURE_Y_UV_WL";
      default                          : return "EGL_PLATFORM_WAYLAND_EXT";
      }
   }

   EnumCase(EGL_WINDOW_BIT);                                   // =  4
   //EnumCase(EGL_OPENGL_ES2_BIT);                             // =  4
   //EnumCase(EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR);       // =  4

   EnumCase(EGL_PIXMAP_BIT);                                   // =  2
   //EnumCase(EGL_OPENVG_BIT);                                 // =  2
   //EnumCase(EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT);   // =  2
   //EnumCase(EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR);  // =  2
   //EnumCase(EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR);// =  2
   //EnumCase(EGL_WRITE_SURFACE_BIT_KHR);                      // =  2

   EnumCase(EGL_COLORSPACE);                                   // =  12423
   //EnumCase(EGL_VG_COLORSPACE);                              // =  12423

   EnumCase(EGL_ALPHA_FORMAT);                                 // =  12424
   //EnumCase(EGL_VG_ALPHA_FORMAT);                            // =  12424

   EnumCase(EGL_COLORSPACE_sRGB);                              // =  12425
   //EnumCase(EGL_VG_COLORSPACE_sRGB);                         // =  12425
   //EnumCase(EGL_GL_COLORSPACE_SRGB);                         // =  12425
   //EnumCase(EGL_GL_COLORSPACE_SRGB_KHR);                     // =  12425

   EnumCase(EGL_COLORSPACE_LINEAR);                            // =  12426
   //EnumCase(EGL_VG_COLORSPACE_LINEAR);                       // =  12426
   //EnumCase(EGL_GL_COLORSPACE_LINEAR);                       // =  12426
   //EnumCase(EGL_GL_COLORSPACE_LINEAR_KHR);                   // =  12426

   EnumCase(EGL_ALPHA_FORMAT_NONPRE);                          // =  12427
   //EnumCase(EGL_VG_ALPHA_FORMAT_NONPRE);                     // =  12427

   EnumCase(EGL_ALPHA_FORMAT_PRE);                             // =  12428
   //EnumCase(EGL_VG_ALPHA_FORMAT_PRE);                        // =  12428

   EnumCase(EGL_CONTEXT_CLIENT_VERSION);                       // =  12440
   //EnumCase(EGL_CONTEXT_MAJOR_VERSION);                      // =  12440
   //EnumCase(EGL_CONTEXT_MAJOR_VERSION_KHR);                  // =  12440

   EnumCase(EGL_OPENGL_ES3_BIT_KHR);                           // =  64
   //EnumCase(EGL_VG_ALPHA_FORMAT_PRE_BIT);                    // =  64
   //EnumCase(EGL_VG_ALPHA_FORMAT_PRE_BIT_KHR);                // =  64
   //EnumCase(EGL_OPENGL_ES3_BIT);                             // =  64

   // Now use the auto-generated code to lookup the unique EGL enum names
   // This table content is auto-generated by running ./gen_hook_tables.py in v3dv3/tools/v3d/hook_codegen
   default:
      break;
   }

   switch(e)
   {
   #include "mapeglenums.inc"

   default: return string("XXX_UNKNOWN_ENUM_XXX ") + numberStr(e); break;
   }
}

string GLStdCommand::MakeBitfieldStr(uint32_t val) const
{
   string str;

   if (val == 0)
      str = "0";
   else if (m_packet.Item(0).GetFunc() == cmd_glClear)
   {
      if (val & GL_COLOR_BUFFER_BIT)
         str += "GL_COLOR_BUFFER_BIT";
      if (val & GL_DEPTH_BUFFER_BIT)
         str += string(str.size() > 0 ? "|" : "") + "GL_DEPTH_BUFFER_BIT";
      if (val & GL_STENCIL_BUFFER_BIT)
         str += string(str.size() > 0 ? "|" : "") + "GL_STENCIL_BUFFER_BIT";
   }
   else
      str += numberStr(val);

   return str;
}

string GLStdCommand::MakeArgStr(uint8_t sig, const PacketItem &item) const
{
   string s;

   switch (sig)
   {
   case 'l' : // EGLClientBuffer
   case 's' : // Shader ID
   case 'r' : // Program ID
   case 'T' : // Texture ID
   case 'M' : // Framebuffer ID
   case 'i' : // Uniform location
   case 'a' : // Attrib location
   case 'g' : // EGLimage
   case 'N' : // Pipeline
   case 'R' : // Renderbuffer
   case 'S' : // Sampler
   case 'U' : // Buffer
   case 'q' : // Query
   case 'y' : // Sync
   case 'Z' : // TransformFeedback
   case 'w' : s += numberStr(item.GetUInt32()); break;                  // uint32
   case 'm' : s += MakeBitfieldStr(item.GetUInt32()); break;            // bitmask
   case 'e' : s += MapEnum(item.GetUInt32(), m_command); break;         // enum
   case 'j' :
      if (item.GetInt32() <= GL_TEXTURE31)
         s += MapEnum(item.GetUInt32(), m_command);                     // enum
      else
         // This can't be clamped to GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS minus one as different platforms have
         // unique values of this.
         s += "GL_TEXTURE0+" + numberStr(item.GetInt32() - GL_TEXTURE0);
      break;     // special case for GL_TEXTURE0+i enum
   case 'E' : s += MapEGLEnum(item.GetUInt32(), m_command); break;      // EGL enum
   case 'W' : s += numberStr(item.GetInt32()); break;                   // int32
   case 'f' : s += numberStr(item.GetFloat()); break;                   // float
   case 'F' : s += MapEnum((uint32_t)item.GetFloat(), m_command); break;// float based enum
   case 't' : s += item.GetBoolean() ? "GL_TRUE" : "GL_FALSE"; break;   // boolean
   case 'Y' : s += item.GetBoolean() ? "EGL_TRUE" : "EGL_FALSE"; break; // egl boolean
   case 'h' : s += numberStr(item.GetUInt16()); break;                  // uint16
   case 'H' : s += numberStr(item.GetInt16()); break;                   // int16
   case 'b' : s += numberStr(item.GetUInt8()); break;                   // uint8
   case 'B' : s += numberStr(item.GetInt8()); break;                    // int8
   case 'd': // EGLDisplay
   case 'u': // EGLSurface
   case 'n': // EGLContext
   case 'o': // EGLConfig
   case 'x' :
   {
      stringstream stream;
      stream << std::hex << item.GetUInt32();
      s += string("0x") + stream.str();
      break;
   }
   case 'c' : s += "\"" + string(item.GetCharPtr()) + "\""; break;      // const char*
   case 'C' :                                                           // char * (for output)
   case 'p' :                                                           // pointer
   case 'P' :
   {
      stringstream stream;
      stream << std::hex << (uintptr_t)item.GetVoidPtr();
      s += string("0x") + stream.str();
      break;
   }
   case 'v': break; // void
   default : assert(0); break;
   }
   return s;
}

string GLStdCommand::AsString() const
{
   string str;

   if (m_packet.NumItems() > 0)
   {
      str += GLStdCommandMapper::ToString(m_packet.Item(0).GetFunc());

      string sig = GLStdCommandMapper::Signature(m_packet.Item(0).GetFunc());
      assert(sig.size() <= m_packet.NumItems() - 1);

      str += "(";

      for (uint32_t i = 0; i < (uint32_t)sig.size(); i++)
      {
         // Examine signature and make appropriate fields
         str += MakeArgStr(sig[i], m_packet.Item(i + 1));

         if (i < (uint32_t)sig.size() - 1)
            str += ", ";
      }

      str += ");";
   }

   return str;
}

string GLStdCommand::AsHtmlString() const
{
   string str;

   if (m_packet.NumItems() > 0)
   {
      str += GLStdCommandMapper::ToString(m_packet.Item(0).GetFunc());

      string sig = GLStdCommandMapper::Signature(m_packet.Item(0).GetFunc());
      assert(sig.size() <= m_packet.NumItems() - 1);

      str += "(";

      for (uint32_t i = 0; i < (uint32_t)sig.size(); i++)
      {
         // Examine signature and make appropriate fields
         char sigCh = sig[i];
         string argStr = MakeArgStr(sigCh, m_packet.Item(i + 1));

         string anchorChar = "";

         switch (sigCh)
         {
         case 'M' :
         case 'U' :
         case 's' :
         case 'S' :
         case 'a' :
         case 'y' :
         case 'r' :
         case 'T' :
         case 'q':
         case 'R':
         case 'Z' : anchorChar = sigCh; break;
         default  : break;
         }

         if (anchorChar != "")
            str += "<a href=\"" + anchorChar + ":" + numberStr(m_context) + ":" + argStr + "\">" + argStr + "</a>";
         else
            str += argStr;

         if (i < (uint32_t)sig.size() - 1)
            str += ", ";
      }

      str += ");";
   }

   return str;
}

string GLStdCommand::StringArg(uint32_t num) const
{
   num++;
   if (num < m_packet.NumItems())
   {
      string sig = GLStdCommandMapper::Signature(m_packet.Item(0).GetFunc());
      assert(sig.size() <= m_packet.NumItems() - 1);

      return MakeArgStr(sig[num - 1], m_packet.Item(num));
   }

   return "";
}

uint32_t GLStdCommand::UIntArg(uint32_t num) const
{
   num++;
   if (num < m_packet.NumItems())
      return m_packet.Item(num).GetUInt32();

   return 0;
}

int32_t GLStdCommand::IntArg(uint32_t num) const
{
   num++;
   if (num < m_packet.NumItems())
      return m_packet.Item(num).GetInt32();

   return 0;
}


float GLStdCommand::FloatArg(uint32_t num) const
{
   num++;
   if (num < m_packet.NumItems())
      return m_packet.Item(num).GetFloat();

   return 0.0f;
}

string GLStdCommand::StringBufferArg(uint32_t num) const
{
   num++;
   if (num < m_packet.NumItems())
   {
      uint8_t *ptr;
      uint32_t bytes = m_packet.Item(num).GetArray(&ptr);

      string ret((const char *)ptr, bytes);
      return ret;
   }

   return "";
}

void *GLStdCommand::VoidPtrArg(uint32_t num) const
{
   num++;
   if (num < m_packet.NumItems())
      return m_packet.Item(num).GetVoidPtr();

   return NULL;
}

uint32_t GLStdCommand::NumArgs() const
{
   string sig = GLStdCommandMapper::Signature(m_packet.Item(0).GetFunc());
   return sig.size();
}

void GLStdCommand::Mark(uint32_t markerBits)
{
   m_markerBits |= markerBits;
}

uint32_t GLStdCommand::Markers() const
{
   return m_markerBits;
}

#define AddEnum(e) m_allEnums[#e] = (uint32_t)((uintptr_t)(e))

const map<string, uint32_t> &GLStdCommand::EnumStringMapTable()
{
   if (m_allEnums.empty())
   {
      // This table content is auto-generated by running ./gen_hook_tables.py in v3dv3/tools/v3d/hook_codegen
      #include "addenums.inc"
   }

   return m_allEnums;
}
