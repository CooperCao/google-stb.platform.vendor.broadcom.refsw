/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_application.h"
#include "bsg_shape.h"
#include "bsg_obj_reader.h"
#include "bsg_effect_generator.h"

namespace bsg
{

// Convenience constants
static const float    HALF_PI      = (float)M_PI / 2.0f;  // M_PI is a double
static const float    TWO_PI       = (float)M_PI * 2.0f;

///////////////////////////////////////////////////////////////////////////////////////////////////

Vec3 ChangeAxis(const Vec3 &v, eAxis axis)
{
   switch (axis)
   {
   case eX_AXIS:
      return Vec3(v.Y(), v.X(), v.Z());
      break;

   case eZ_AXIS:
      return Vec3(v.X(), v.Z(), v.Y());
      break;

   case eY_AXIS:
   default:
      // Returns v
      break;
   }

   return v;
}

static EffectSemantics::eSemantic Semantic(ShapeFactory::eAttr e)
{
   return (EffectSemantics::eSemantic)(e + EffectSemantics::eVATTR_FIRST);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ShapeFactory::Push(std::vector<float> &arr, const Vec4 &v)
{
   arr.push_back(v.X());
   arr.push_back(v.Y());
   arr.push_back(v.Z());
   arr.push_back(v.W());
}

void ShapeFactory::Push(std::vector<float> &arr, const Vec3 &v)
{
   arr.push_back(v.X());
   arr.push_back(v.Y());
   arr.push_back(v.Z());
}

void ShapeFactory::Push(std::vector<float> &arr, const Vec2 &v)
{
   arr.push_back(v.X());
   arr.push_back(v.Y());
}

void ShapeFactory::Push(std::vector<float> &arr, const float v)
{
   arr.push_back(v);
}

template <class T>
static const void *Addr(const T &arr)
{
   if (arr.size() == 0)
      return 0;

   return &arr[0];
}

void ShapeFactory::SetPointer(SurfaceHandle &surf, eAttr attr, const GLVertexPointer &ptrInfo) const
{
   if (Uses(attr))
      surf->SetPointer(Semantic(attr), ptrInfo);
}

SurfaceHandle ShapeFactory::MakeSurface() const
{
   std::vector<float>   shape;
   std::vector<short>   indices;

   // Create the raw geometry
   PushVertices(shape);
   PushIndices(indices);

   // How many of everything do we have
   uint32_t    numIndices      = indices.size();
   uint32_t    numDrawVertices = numIndices == 0 ? m_numVertices : numIndices;

   // Create the surface
   SurfaceHandle surf(New);

   // Create the VBOs
   surf->SetDraw(GetDrawMode(), numDrawVertices, shape.size()   * sizeof(GLfloat),  &shape[0],
                                                 indices.size() * sizeof(GLushort), Addr(indices));

   // Set up the attribute pointers
   uint32_t vertexByteSize = GetVertexSize() * sizeof(GLfloat);

   for (eAttr i = eATTR_FIRST; i <=  eATTR_LAST; i = (eAttr)(i + 1))
      SetPointer(surf, i,  GLVertexPointer(GetAttrSize(i), GL_FLOAT, vertexByteSize, sizeof(float) * GetOffset(i)));

   // Set the bounds from the vertex positions (first 3 floats of each stride)
   uint32_t posSize = GetAttrSize(ePOSITION);

   if (posSize != 0)
      surf->SetBound(Bound(m_numVertices, &shape[0], GetVertexSize(), posSize));

   // Set culling properties if required
   surf->SetCull(GetCullMode());

   return surf;
}

GeometryHandle ShapeFactory::MakeGeometry(const MaterialHandle &material) const
{
   GeometryHandle geom(New);

   geom->AppendSurface(MakeSurface(), material);

   return geom;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QuadFactory::QuadFactory(const Vec2 &bl, const Vec2 &tr, float offset, eAxis axis) :
   StdFactory(GL_TRIANGLE_STRIP, NUM_VERTICES, true)
{
   Init(bl, tr, offset, axis);
}

QuadFactory::QuadFactory(float w, float h, float offset, eAxis axis) :
   StdFactory(GL_TRIANGLE_STRIP, NUM_VERTICES, true)
{
   float w2 = w / 2.0f;
   float h2 = h / 2.0f;

   Init(Vec2(-w2, -h2), Vec2(w2, h2), offset, axis);
}

void QuadFactory::Init(const Vec2 &bl, const Vec2 &tr, float offset, eAxis axis)
{
   float x0 = bl.X();
   float x1 = tr.X();
   float y0 = bl.Y();
   float y1 = tr.Y();

   AddVertex(x0, offset, y1,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,  axis);
   AddVertex(x0, offset, y0,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,  axis);
   AddVertex(x1, offset, y1,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,  axis);
   AddVertex(x1, offset, y0,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,  axis);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// Sphere normal
static inline Vec3 SphereN(float theta1, float theta2)
{
   return Vec3(cosf(theta1) * cosf(theta2),
               sinf(theta1),
               cosf(theta1) * sinf(theta2));
}

// Sphere vertex position
static inline StdVertex SphereV(float theta1, float theta2, const Vec3 &centre, float radius, float steps, uint32_t ii, uint32_t jj)
{
   Vec3  normal(SphereN(theta1, theta2));
   Vec3  position(normal * radius + centre);
   Vec2  tc(1.0f - ii / steps, jj * 2.0f / steps);

   return StdVertex(position, normal, tc);
}

static uint32_t SphereSteps(uint32_t isteps)
{
   uint32_t is = isteps & ~1;

   return is * is + 3 * is - 4;
}

SphereFactory::SphereFactory(const Vec3 &centre, float radius, uint32_t is) :
   StdFactory(GL_TRIANGLE_STRIP, SphereSteps(is), true)
{
   // Make sure its even
   uint32_t isteps   = is & ~1;
   bool     notFirst = false;

   if (isteps == 0)
      BSG_THROW("Sphere must have at least 2 steps");

   // Handy float
   const float    steps = (float)isteps;

   // Generate the geometry
   for (uint32_t jj = 0; jj < isteps / 2; ++jj)
   {
      float theta1 = ((jj + 0) * TWO_PI) / steps - HALF_PI;
      float theta2 = ((jj + 1) * TWO_PI) / steps - HALF_PI;

      // Add additional geometry to make a degenerate triangle bridge
      // to the next strip (not wanted on the first strip)
      if (notFirst)
      {
         DupVertex();
         AddVertex(SphereV(theta1, 0.0f, centre, radius, steps, 0, jj));
         DupVertex();
         AddVertex(SphereV(theta2, 0.0f, centre, radius, steps, 0, jj + 1));
      }
      else
      {
         notFirst = true;
      }

      for (uint32_t ii = 0; ii <= isteps; ++ii)
      {
         float theta3 = ii * TWO_PI / steps;

         AddVertex(SphereV(theta1, theta3, centre, radius, steps, ii, jj));
         AddVertex(SphereV(theta2, theta3, centre, radius, steps, ii, jj + 1));
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CuboidFactory::CuboidFactory(float s) :
   StdFactory(GL_TRIANGLES, NUM_VERTICES, true)
{
   Init(s, s, s);
}

CuboidFactory::CuboidFactory(float w, float h, float d) :
   StdFactory(GL_TRIANGLES, NUM_VERTICES, true)
{
   Init(w, h, d);
}

void CuboidFactory::Init(float w, float h, float d)
{
   float x = w * 0.5f;
   float y = h * 0.5f;
   float z = d * 0.5f;

   //        POSITION      NORMAL       TC
   AddVertex( x,  y, -z,   0,  0, -1,   0, 1);
   AddVertex( x, -y, -z,   0,  0, -1,   0, 0);
   AddVertex(-x, -y, -z,   0,  0, -1,   1, 0);
   AddVertex(-x,  y, -z,   0,  0, -1,   1, 1);

   AddVertex(-x, -y,  z,  -1,  0,  0,   1, 0);
   AddVertex(-x,  y,  z,  -1,  0,  0,   1, 1);
   AddVertex(-x,  y, -z,  -1,  0,  0,   0, 1);
   AddVertex(-x, -y, -z,  -1,  0,  0,   0, 0);

   AddVertex( x, -y,  z,   0,  0,  1,   1, 0);
   AddVertex( x,  y,  z,   0,  0,  1,   1, 1);
   AddVertex(-x, -y,  z,   0,  0,  1,   0, 0);
   AddVertex(-x,  y,  z,   0,  0,  1,   0, 1);

   AddVertex( x, -y, -z,   1,  0,  0,   1, 0);
   AddVertex( x,  y, -z,   1,  0,  0,   1, 1);
   AddVertex( x, -y,  z,   1,  0,  0,   0, 0);
   AddVertex( x,  y,  z,   1,  0,  0,   0, 1);

   AddVertex( x,  y, -z,   0,  1,  0,   1, 0);
   AddVertex(-x,  y, -z,   0,  1,  0,   0, 0);
   AddVertex( x,  y,  z,   0,  1,  0,   1, 1);
   AddVertex(-x,  y,  z,   0,  1,  0,   0, 1);

   AddVertex( x, -y, -z,   0, -1,  0,   1, 0);
   AddVertex( x, -y,  z,   0, -1,  0,   1, 1);
   AddVertex(-x, -y,  z,   0, -1,  0,   0, 1);
   AddVertex(-x, -y, -z,   0, -1,  0,   0, 0);
}

void CuboidFactory::PushIndices(std::vector<short> &indices) const
{
   static const GLushort cube_idx[] =
   {
      0,  1,  2,    3,  0,  2,
      4,  5,  6,    7,  4,  6,
      8,  9,  10,   9,  11, 10,
      12, 13, 14,   13, 15, 14,
      16, 17, 18,   17, 19, 18,
      20, 21, 22,   23, 20, 22,
   };

   const uint32_t NUM_INDICES = sizeof(cube_idx) / sizeof(GLushort);

   indices.reserve(NUM_INDICES);

   for (uint32_t i = 0; i < NUM_INDICES; ++i)
      indices.push_back(cube_idx[i]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static StdVertex DiskV(const Vec3 &centre, float theta, float radius, eAxis axis)
{
   Vec3    pos(cosf(theta), 0.0f, sinf(theta));
   Vec3    norm(0.0f, 1.0f, 0.0f);
   Vec2    tc((pos.X() + 1.0f) / 2.0f, (pos.Z() + 1.0f) / 2.0f);

   return StdVertex(pos * radius, norm, tc, centre, axis);
}

DiskFactory::DiskFactory(const Vec3 &centre, float radius, uint32_t isteps, eAxis axis) :
   StdFactory(GL_TRIANGLE_FAN, isteps + 2, true)
{
   // Centre
   AddVertex(StdVertex(Vec3(), Vec3(0.0f, 1.0f, 0.0f), Vec2(0.5f, 0.5f), centre, axis));

   float steps = (float)isteps;

   for (uint32_t i = 0; i <= isteps; ++i)
   {
      float   frac = i / steps;
      float   theta = -frac * TWO_PI;

      AddVertex(DiskV(centre, theta, radius, axis));
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static StdVertex RingV(const Vec3 &centre, float theta, float radius, float u, float v, eAxis axis)
{
    Vec3    pos(cosf(theta), 0.0f, sinf(theta));
    Vec3    norm(0.0f, 1.0f, 0.0f);
    Vec2    tc(1.0f - u, 1.0f - v);

    return StdVertex(pos * radius, norm, tc, centre, axis);
}

RingFactory::RingFactory(const Vec3 &centre, float innerRadius, float outerRadius, uint32_t isteps, eAxis axis) :
   StdFactory(GL_TRIANGLE_STRIP, isteps * 2 + 2, true)
{
   AddVertex(RingV(centre, 0.0f, innerRadius, 0.0f, 0.0f, axis));
   AddVertex(RingV(centre, 0.0f, outerRadius, 0.0f, 1.0f, axis));

   float steps = (float)isteps;

   for (uint32_t i = 1; i <= isteps; ++i)
   {
      float   frac  = i / steps;
      float   theta = -frac * TWO_PI;

      AddVertex(RingV(centre, theta, innerRadius, frac, 0.0f, axis));
      AddVertex(RingV(centre, theta, outerRadius, frac, 1.0f, axis));
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

namespace prv
{

class ConeVertex
{
public:
   ConeVertex(const Vec3 &centre, float innerRadius, float outerRadius, float height, eAxis axis) :
      m_centre(centre),
      m_height(height),
      m_axis(axis)
   {
      m_radiusDiff = outerRadius - innerRadius;
      m_B1         = 1.0f / sqrtf(m_radiusDiff * m_radiusDiff + height * height);
   }

   StdVertex Vertex(float theta, float offset, float radius, float u, float v)
   {
      Vec3    pos(cosf(theta) * radius, offset, sinf(theta) * radius);
      Vec3    norm(Vec3(m_height * cosf(theta), m_radiusDiff, m_height * sinf(theta)) * m_B1);
      Vec2    tc(u, 1.0f - v);

      return StdVertex(pos, norm, tc, m_centre, m_axis);
   }

private:
   Vec3     m_centre;
   float    m_height;
   eAxis    m_axis;

   float    m_B1;
   float    m_radiusDiff;
};

}

ConeFactory::ConeFactory(const Vec3 &centre, float innerRadius, float outerRadius, float height, uint32_t isteps, eAxis axis) :
   StdFactory(GL_TRIANGLE_STRIP, isteps * 2 + 2, true)
{
   prv::ConeVertex   cv(centre, innerRadius, outerRadius, height, axis);

   AddVertex(cv.Vertex(0.0f, height, innerRadius, 0.0f, 0.0f));
   AddVertex(cv.Vertex(0.0f, 0.0f,   outerRadius, 0.0f, 1.0f));

   float steps = (float)isteps;

   for (uint32_t i = 1; i <= isteps; ++i)
   {
      float   frac  = i / steps;
      float   theta = -frac * TWO_PI;

      AddVertex(cv.Vertex(theta, height, innerRadius, frac, 0.0f));
      AddVertex(cv.Vertex(theta, 0.0f,   outerRadius, frac, 1.0f));
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Vec2 RoundedRectFactory::PosToTC(const Vec3 &pos) const
{
   return Vec2(pos.X() / m_width + 0.5f, (pos.Z() / m_height + 0.5f));
}

uint32_t RoundedRectFactory::Fan(const Vec3 &centre, float theta1, float theta2)
{
   Vec3  normal(0.0f, 1.0f, 0.0f);

   float steps = (float)m_isteps;

   uint32_t centreIndex = AddVertex(centre, normal, PosToTC(centre), m_centre, m_axis);

   for (uint32_t i = 0; i <= m_isteps; ++i)
   {
      float alpha = i / steps;
      float theta = (1.0f - alpha) * theta1 + alpha * theta2;

      Vec3  outer(m_cornerRadius * cos(theta), 0.0f, m_cornerRadius * sin(theta));

      outer += centre;

      m_ib.push_back(centreIndex);
      m_ib.push_back(AddVertex(outer, normal, PosToTC(outer), m_centre, m_axis));
   }

   return centreIndex;
}

RoundedRectFactory::RoundedRectFactory(const Vec3 &centre, float width, float height, float cornerRadius, uint32_t isteps, eAxis axis) :
   StdFactory(GL_TRIANGLE_STRIP, 4 * (isteps + 2), CullMode(GL_CW, GL_BACK)),
   m_centre(centre),
   m_width(width),
   m_height(height),
   m_cornerRadius(cornerRadius),
   m_isteps(isteps),
   m_axis(axis)
{
   m_ib.reserve(8 * isteps + 7);

   float w2 = width  / 2.0f;
   float h2 = height / 2.0f;

   float w2o = w2 - cornerRadius;
   float h2o = h2 - cornerRadius;

   Vec3 c00(-w2o, 0.0f, -h2o);
   Vec3 c01(-w2o, 0.0f,  h2o);
   Vec3 c10( w2o, 0.0f, -h2o);
   Vec3 c11( w2o, 0.0f,  h2o);

   short inner00 = Fan(c00, 3.0f * HALF_PI,  2.0f * HALF_PI);
   short inner01 = Fan(c01, 2.0f * HALF_PI,  1.0f * HALF_PI);
   short inner11 = Fan(c11, 1.0f * HALF_PI,  0.0f * HALF_PI);
   short inner10 = Fan(c10, 0.0f * HALF_PI, -1.0f * HALF_PI);

   // Finish off the last edge
   m_ib.push_back(inner00);
   m_ib.push_back(inner00 + 1);

   // And fill in the middle
   m_ib.push_back(inner00);
   m_ib.push_back(inner00);
   m_ib.push_back(inner10);
   m_ib.push_back(inner01);
   m_ib.push_back(inner11);
}

void RoundedRectFactory::PushIndices(std::vector<short> &indices) const
{
   indices.reserve(m_ib.size());

   // TODO use a copy algorithm?
   for (uint32_t i = 0; i < m_ib.size(); ++i)
      indices.push_back(m_ib[i]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

PanelFactory::PanelFactory(const Vec2 &bl, const Vec2 &tr, float bevel, float texBevel, eAxis axis) :
   StdFactory(GL_TRIANGLE_STRIP, 16, CullMode(GL_CW, GL_BACK))
{
   float x0 = bl.X();      float u0 = 0.0f;
   float x1 = x0 + bevel;  float u1 = texBevel;
   float x3 = tr.X();      float u3 = 1.0f;
   float x2 = x3 - bevel;  float u2 = 1.0f - texBevel;

   float y0 = bl.Y();      float v0 = 0.0f;
   float y1 = y0 + bevel;  float v1 = texBevel;
   float y3 = tr.Y();      float v3 = 1.0f;
   float y2 = y3 - bevel;  float v2 = 1.0f - texBevel;

   AddVertex(x0, 0.0f, y0,  0.0f, 1.0f, 0.0f,  u0, v0,  axis);
   AddVertex(x1, 0.0f, y0,  0.0f, 1.0f, 0.0f,  u1, v0,  axis);
   AddVertex(x2, 0.0f, y0,  0.0f, 1.0f, 0.0f,  u2, v0,  axis);
   AddVertex(x3, 0.0f, y0,  0.0f, 1.0f, 0.0f,  u3, v0,  axis);

   AddVertex(x0, 0.0f, y1,  0.0f, 1.0f, 0.0f,  u0, v1,  axis);
   AddVertex(x1, 0.0f, y1,  0.0f, 1.0f, 0.0f,  u1, v1,  axis);
   AddVertex(x2, 0.0f, y1,  0.0f, 1.0f, 0.0f,  u2, v1,  axis);
   AddVertex(x3, 0.0f, y1,  0.0f, 1.0f, 0.0f,  u3, v1,  axis);

   AddVertex(x0, 0.0f, y2,  0.0f, 1.0f, 0.0f,  u0, v2,  axis);
   AddVertex(x1, 0.0f, y2,  0.0f, 1.0f, 0.0f,  u1, v2,  axis);
   AddVertex(x2, 0.0f, y2,  0.0f, 1.0f, 0.0f,  u2, v2,  axis);
   AddVertex(x3, 0.0f, y2,  0.0f, 1.0f, 0.0f,  u3, v2,  axis);

   AddVertex(x0, 0.0f, y3,  0.0f, 1.0f, 0.0f,  u0, v3,  axis);
   AddVertex(x1, 0.0f, y3,  0.0f, 1.0f, 0.0f,  u1, v3,  axis);
   AddVertex(x2, 0.0f, y3,  0.0f, 1.0f, 0.0f,  u2, v3,  axis);
   AddVertex(x3, 0.0f, y3,  0.0f, 1.0f, 0.0f,  u3, v3,  axis);
}

PanelFactory::PanelFactory(const Vec2 &bl, const Vec2 &tr, const Vec2 &bevel, float texBevel, eAxis axis) :
   StdFactory(GL_TRIANGLE_STRIP, 16, CullMode(GL_CW, GL_BACK))
{
   float x0 = bl.X();          float u0 = 0.0f;
   float x1 = x0 + bevel.X();  float u1 = texBevel;
   float x3 = tr.X();          float u3 = 1.0f;
   float x2 = x3 - bevel.X();  float u2 = 1.0f - texBevel;

   float y0 = bl.Y();          float v0 = 0.0f;
   float y1 = y0 + bevel.Y();  float v1 = texBevel;
   float y3 = tr.Y();          float v3 = 1.0f;
   float y2 = y3 - bevel.Y();  float v2 = 1.0f - texBevel;

   AddVertex(x0, 0.0f, y0,  0.0f, 1.0f, 0.0f,  u0, v0,  axis);
   AddVertex(x1, 0.0f, y0,  0.0f, 1.0f, 0.0f,  u1, v0,  axis);
   AddVertex(x2, 0.0f, y0,  0.0f, 1.0f, 0.0f,  u2, v0,  axis);
   AddVertex(x3, 0.0f, y0,  0.0f, 1.0f, 0.0f,  u3, v0,  axis);

   AddVertex(x0, 0.0f, y1,  0.0f, 1.0f, 0.0f,  u0, v1,  axis);
   AddVertex(x1, 0.0f, y1,  0.0f, 1.0f, 0.0f,  u1, v1,  axis);
   AddVertex(x2, 0.0f, y1,  0.0f, 1.0f, 0.0f,  u2, v1,  axis);
   AddVertex(x3, 0.0f, y1,  0.0f, 1.0f, 0.0f,  u3, v1,  axis);

   AddVertex(x0, 0.0f, y2,  0.0f, 1.0f, 0.0f,  u0, v2,  axis);
   AddVertex(x1, 0.0f, y2,  0.0f, 1.0f, 0.0f,  u1, v2,  axis);
   AddVertex(x2, 0.0f, y2,  0.0f, 1.0f, 0.0f,  u2, v2,  axis);
   AddVertex(x3, 0.0f, y2,  0.0f, 1.0f, 0.0f,  u3, v2,  axis);

   AddVertex(x0, 0.0f, y3,  0.0f, 1.0f, 0.0f,  u0, v3,  axis);
   AddVertex(x1, 0.0f, y3,  0.0f, 1.0f, 0.0f,  u1, v3,  axis);
   AddVertex(x2, 0.0f, y3,  0.0f, 1.0f, 0.0f,  u2, v3,  axis);
   AddVertex(x3, 0.0f, y3,  0.0f, 1.0f, 0.0f,  u3, v3,  axis);
}

void PanelFactory::PushIndices(std::vector<short> &indices) const
{
   static short   ind[] = {  0,  4,  1,  5,  2,  6,  3,  7,  7,  4,
                             4,  8,  5,  9,  6, 10,  7, 11, 11,  8,
                             8, 12,  9, 13, 10, 14, 11, 15 };

   for (uint32_t i = 0; i < sizeof(ind) / sizeof(short); ++i)
      indices.push_back(ind[i]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t FloatOffset(uint32_t n)
{
   return sizeof(GLfloat) * n;
}

static void EmitVertex(std::vector<float> &buffer, const ObjReader &reader, const ObjReader::Face &face, uint32_t v, bool hasN, bool hasTC)
{
   const Vec3  &pos = reader.GetPosition(face, v);

   buffer.push_back(pos.X());
   buffer.push_back(pos.Y());
   buffer.push_back(pos.Z());

   if (hasN)
   {
      const Vec3  &norm = reader.GetNormal(face, v);

      buffer.push_back(norm.X());
      buffer.push_back(norm.Y());
      buffer.push_back(norm.Z());
   }

   if (hasTC)
   {
      const Vec2  &tc = reader.GetTextureCoordinate(face, v);

      buffer.push_back(tc.X());
      buffer.push_back(tc.Y());
   }
}

static void EmitTriangle(std::vector<float> &buffer, const ObjReader &reader, const ObjReader::Face &face, uint32_t v, bool hasN, bool hasTC)
{
   EmitVertex(buffer, reader, face, 0    , hasN, hasTC);
   EmitVertex(buffer, reader, face, v    , hasN, hasTC);
   EmitVertex(buffer, reader, face, v + 1, hasN, hasTC);
}

static uint32_t ObjVertexSize(bool hasN, bool hasTC)
{
   uint32_t res = 3;

   if (hasN)
      res += 3;

   if (hasTC)
      res += 2;

   return res;
}

SurfaceHandle ObjFactory::CreateSurfaceFromObj(const ObjReader &reader, uint32_t o) const
{
   uint32_t       totalVertices = 0;

   ObjReader::Object object = reader.GetObject(o);

   uint32_t numFaces    = object.size();
   bool     hasN        = reader.HasNormals(object);
   bool     hasTC       = reader.HasTextureCoordinates(object);
   uint32_t vertexSize  = ObjVertexSize(hasN, hasTC);

   std::vector<float>   vertexData;

   for (uint32_t f = 0; f < numFaces; ++f)
   {
      const ObjReader::Face  &face = object[f];

      for (uint32_t v = 1; v < face.size() - 1; ++v)
      {
         EmitTriangle(vertexData, reader, face, v, hasN, hasTC);
         totalVertices += 3;
      }
   }

   SurfaceHandle  surf(New);

   surf->SetDraw(GL_TRIANGLES, totalVertices, sizeof(float) * vertexData.size(), &vertexData[0]);

   uint32_t offset = 0;

   surf->SetPointer(EffectSemantics::eVATTR_POSITION,  GLVertexPointer(3, GL_FLOAT, vertexSize * sizeof(GLfloat), FloatOffset(offset)));
   offset += 3;

   if (hasN)
   {
      surf->SetPointer(EffectSemantics::eVATTR_NORMAL, GLVertexPointer(3, GL_FLOAT, vertexSize * sizeof(GLfloat), FloatOffset(offset)));
      offset += 3;
   }

   if (hasTC)
   {
      surf->SetPointer(EffectSemantics::eVATTR_TEXCOORD1, GLVertexPointer(2, GL_FLOAT, vertexSize * sizeof(GLfloat), FloatOffset(offset)));
      offset += 2;
   }

   surf->SetBound(Bound(totalVertices, &vertexData[0], offset));

   return surf;
}

ObjFactory::ObjFactory(const std::string &fileName) :
   m_reader(0),
   m_path(Application::Instance()->FindResourceFolder(fileName))
{
   m_reader = new ObjReader(fileName, m_path);
}

ObjFactory::~ObjFactory()
{
   delete m_reader;
}

//! Applies the same material to all the objects
GeometryHandle ObjFactory::MakeGeometry(const MaterialHandle &material)
{
   GeometryHandle geom(New);

   for (uint32_t o = 0; o < m_reader->GetNumObjects(); ++o)
   {
      const ObjReader::Object  &object = m_reader->GetObject(o);

      if (object.size() > 0)
      {
         SurfaceHandle  surface = CreateSurfaceFromObj(*m_reader, o);

         geom->AppendSurface(surface, material);
      }
   }

   return geom;
}

//! Uses already defined materials to clothe the objects
GeometryHandle ObjFactory::MakeGeometry(std::set<MaterialHandle> *materials)
{
   GeometryHandle geom(New);

   for (uint32_t o = 0; o < m_reader->GetNumObjects(); ++o)
   {
      const ObjReader::Object &object       = m_reader->GetObject(o);
      const std::string       &materialName = m_reader->GetMaterialName(o);

      if (object.size() > 0)
      {
         SurfaceHandle  surface  = CreateSurfaceFromObj(*m_reader, o);
         MaterialHandle material(materialName);

         if (material.IsNull())
            BSG_THROW("Cannot find material '" << materialName << "'");

         if (materials != 0)
            materials->insert(material);

         geom->AppendSurface(surface, material);
      }
   }

   return geom;
}

//! Uses already defined materials to clothe the object -- if they don't exist create a new one using the options
GeometryHandle ObjFactory::MakeGeometry(const ObjMaterialOptions &options, std::set<MaterialHandle> *materials)
{
   GeometryHandle geom(New);

   ObjMaterialFactory   factory(*m_reader, options, m_path);

   for (uint32_t o = 0; o < m_reader->GetNumObjects(); ++o)
   {
      const ObjReader::Object &object       = m_reader->GetObject(o);
      const std::string       &materialName = m_reader->GetMaterialName(o);

      if (object.size() > 0)
      {
         SurfaceHandle surface = CreateSurfaceFromObj(*m_reader, o);

         MaterialHandle material(materialName);

         if (materials != 0)
            materials->insert(material);

         geom->AppendSurface(surface, material);
      }
   }

   return geom;
}

///////////////////////////////////////////////////////////////////////////////
// ObjFactoryTask
///////////////////////////////////////////////////////////////////////////////

// Runs in
void ObjFactoryTask::CreateSurface(bool hasN, bool hasTC, uint32_t vertexSize, uint32_t totalVertices)
{
   m_surface = SurfaceHandle(New);
   m_geometry->AppendSurface(m_surface, m_material);

   uint32_t offset     = 0;

   m_surface->SetPointer(EffectSemantics::eVATTR_POSITION,  GLVertexPointer(3, GL_FLOAT, vertexSize * sizeof(GLfloat), FloatOffset(offset)));
   offset += 3;

   if (hasN)
   {
      m_surface->SetPointer(EffectSemantics::eVATTR_NORMAL, GLVertexPointer(3, GL_FLOAT, vertexSize * sizeof(GLfloat), FloatOffset(offset)));
      offset += 3;
   }

   if (hasTC)
      m_surface->SetPointer(EffectSemantics::eVATTR_TEXCOORD1, GLVertexPointer(2, GL_FLOAT, vertexSize * sizeof(GLfloat), FloatOffset(offset)));

   m_surface->SetDraw(GL_TRIANGLES, totalVertices, totalVertices * vertexSize * sizeof(float), 0);
}

void ObjFactoryTask::SubmitVertexData(uint32_t floatOffset, uint32_t floatCount, float *vertArray)
{
   m_surface->UpdateVertices(floatOffset * sizeof(float), floatCount * sizeof(float), vertArray);
}

static Box CalcBox(uint32_t count, float *buff, uint32_t size)
{
   uint32_t ix = 0;

   Box   box;

   for (uint32_t i = 0; i < count; ++i, ix += size)
   {
      float *ptr = &buff[ix];

      box += Vec3(ptr[0], ptr[1], ptr[2]);
   }

   return box;
}

void ObjFactoryTask::OnThread()
{
   m_reader = std::auto_ptr<ObjReader>(new ObjReader(m_fileName, Application::Instance()->FindResourceFolder(m_fileName)));

   Call(&ObjFactoryTask::OnLoaded);

   for (uint32_t o = 0; o < m_reader->GetNumObjects(); ++o)
   {
      const ObjReader::Object  &object = m_reader->GetObject(o);

      if (object.size() > 0)
      {
         uint32_t             numFaces      = object.size();
         bool                 hasN          = m_reader->HasNormals(object);
         bool                 hasTC         = m_reader->HasTextureCoordinates(object);
         uint32_t             vertexSize    = ObjVertexSize(hasN, hasTC);

         for (uint32_t b = 0; b < numFaces; b += m_faceBatchSize)
         {
            uint32_t remaining  = numFaces - b;
            uint32_t batchSize  = std::min(remaining, m_faceBatchSize);
            uint32_t totalVerts = 0;

            std::vector<float>   vertexData;

            for (uint32_t f = 0; f < batchSize; ++f)
            {
               const ObjReader::Face  &face = object[b + f];

               for (uint32_t v = 1; v < face.size() - 1; ++v)
               {
                  EmitTriangle(vertexData, *m_reader.get(), face, v, hasN, hasTC);
                  totalVerts    += 3;
                  m_numVertices += 3;
               }
            }

            m_numSurfaces += 1;

            Call(&ObjFactoryTask::CreateSurface, hasN, hasTC, vertexSize, totalVerts);

            Box   bound = CalcBox(totalVerts, &vertexData[0], vertexSize);

            m_boundingBox += bound;

            m_surface->SetBound(Bound(bound.Min(), bound.Max()));

            Call(&ObjFactoryTask::SubmitVertexData, 0u, (uint32_t)vertexData.size(), &vertexData[0]);
         }
      }
   }
}

}
