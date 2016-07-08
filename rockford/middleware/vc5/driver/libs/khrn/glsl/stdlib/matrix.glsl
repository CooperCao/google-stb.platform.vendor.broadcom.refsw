//
// Matrix Functions
//


mat2 matrixCompMult(mat2 x, mat2 y)
{
   return mat2(x[0] * y[0], x[1] * y[1]);
}

mat2x3 matrixCompMult(mat2x3 x, mat2x3 y)
{
   return mat2x3(x[0] * y[0], x[1] * y[1]);
}

mat2x4 matrixCompMult(mat2x4 x, mat2x4 y)
{
   return mat2x4(x[0] * y[0], x[1] * y[1]);
}


mat3x2 matrixCompMult(mat3x2 x, mat3x2 y)
{
   return mat3x2(x[0] * y[0], x[1] * y[1], x[2] * y[2]);
}

mat3 matrixCompMult(mat3 x, mat3 y)
{
   return mat3(x[0] * y[0], x[1] * y[1], x[2] * y[2]);
}

mat3x4 matrixCompMult(mat3x4 x, mat3x4 y)
{
   return mat3x4(x[0] * y[0], x[1] * y[1], x[2] * y[2]);
}


mat4x2 matrixCompMult(mat4x2 x, mat4x2 y)
{
   return mat4x2(x[0] * y[0], x[1] * y[1], x[2] * y[2], x[3] * y[3]);
}

mat4x3 matrixCompMult(mat4x3 x, mat4x3 y)
{
   return mat4x3(x[0] * y[0], x[1] * y[1], x[2] * y[2], x[3] * y[3]);
}

mat4 matrixCompMult(mat4 x, mat4 y)
{
   return mat4(x[0] * y[0], x[1] * y[1], x[2] * y[2], x[3] * y[3]);
}


float determinant(mat2 x)
{
  return x[0][0]*x[1][1]-x[0][1]*x[1][0];
}


float determinant(mat3 x)
{
  mat2 minor00, minor01, minor02;

  minor00[0][0] = x[1][1]; minor00[0][1] = x[1][2]; minor00[1][0] = x[2][1]; minor00[1][1] = x[2][2];
  minor01[0][0] = x[1][0]; minor01[0][1] = x[1][2]; minor01[1][0] = x[2][0]; minor01[1][1] = x[2][2];
  minor02[0][0] = x[1][0]; minor02[0][1] = x[1][1]; minor02[1][0] = x[2][0]; minor02[1][1] = x[2][1];

  return x[0][0]*determinant(minor00) - x[0][1]*determinant(minor01) + x[0][2]*determinant(minor02);
}


float determinant(mat4 x)
{
  mat3 minor00, minor01, minor02, minor03;

  minor00[0][0] = x[1][1]; minor00[0][1] = x[1][2]; minor00[0][2] = x[1][3];
  minor00[1][0] = x[2][1]; minor00[1][1] = x[2][2]; minor00[1][2] = x[2][3];
  minor00[2][0] = x[3][1]; minor00[2][1] = x[3][2]; minor00[2][2] = x[3][3];

  minor01[0][0] = x[1][0]; minor01[0][1] = x[1][2]; minor01[0][2] = x[1][3];
  minor01[1][0] = x[2][0]; minor01[1][1] = x[2][2]; minor01[1][2] = x[2][3];
  minor01[2][0] = x[3][0]; minor01[2][1] = x[3][2]; minor01[2][2] = x[3][3];

  minor02[0][0] = x[1][0]; minor02[0][1] = x[1][1]; minor02[0][2] = x[1][3];
  minor02[1][0] = x[2][0]; minor02[1][1] = x[2][1]; minor02[1][2] = x[2][3];
  minor02[2][0] = x[3][0]; minor02[2][1] = x[3][1]; minor02[2][2] = x[3][3];

  minor03[0][0] = x[1][0]; minor03[0][1] = x[1][1]; minor03[0][2] = x[1][2];
  minor03[1][0] = x[2][0]; minor03[1][1] = x[2][1]; minor03[1][2] = x[2][2];
  minor03[2][0] = x[3][0]; minor03[2][1] = x[3][1]; minor03[2][2] = x[3][2];

  return x[0][0]*determinant(minor00) - x[0][1]*determinant(minor01) + x[0][2]*determinant(minor02) - x[0][3]*determinant(minor03);
}


mat2 inverse(mat2 x)
{
  mat2 res;
  float det;

  det = determinant(x);
  res =  mat2(  x[1][1], -x[0][1],
               -x[1][0],  x[0][0]);

  return (1.0/det)*res;
}


mat3 inverse(mat3 x)
{
  mat3 res;
  float det;

  mat2 minor00 = mat2( x[1][1], x[1][2],
                       x[2][1], x[2][2]);
  mat2 minor01 = mat2( x[1][0], x[1][2],
                       x[2][0], x[2][2]);
  mat2 minor02 = mat2( x[1][0], x[1][1],
                       x[2][0], x[2][1]);
  mat2 minor10 = mat2( x[0][1], x[0][2],
                       x[2][1], x[2][2]);
  mat2 minor11 = mat2( x[0][0], x[0][2],
                       x[2][0], x[2][2]);
  mat2 minor12 = mat2( x[0][0], x[0][1],
                       x[2][0], x[2][1]);
  mat2 minor20 = mat2( x[0][1], x[0][2],
                       x[1][1], x[1][2]);
  mat2 minor21 = mat2( x[0][0], x[0][2],
                       x[1][0], x[1][2]);
  mat2 minor22 = mat2( x[0][0], x[0][1],
                       x[1][0], x[1][1]);

  det = determinant(x);
  res = mat3(  determinant(minor00), -determinant(minor10),  determinant(minor20),
              -determinant(minor01),  determinant(minor11), -determinant(minor21),
               determinant(minor02), -determinant(minor12),  determinant(minor22));

  return (1.0/det)*res;
}


mat4 inverse(mat4 x)
{
  mat4 res;
  float det;

  mat3 minor00 = mat3( x[1][1], x[1][2], x[1][3],
                       x[2][1], x[2][2], x[2][3],
                       x[3][1], x[3][2], x[3][3]);
  mat3 minor01 = mat3( x[1][0], x[1][2], x[1][3],
                       x[2][0], x[2][2], x[2][3],
                       x[3][0], x[3][2], x[3][3]);
  mat3 minor02 = mat3( x[1][0], x[1][1], x[1][3],
                       x[2][0], x[2][1], x[2][3],
                       x[3][0], x[3][1], x[3][3]);
  mat3 minor03 = mat3( x[1][0], x[1][1], x[1][2],
                       x[2][0], x[2][1], x[2][2],
                       x[3][0], x[3][1], x[3][2]);

  mat3 minor10 = mat3( x[0][1], x[0][2], x[0][3],
                       x[2][1], x[2][2], x[2][3],
                       x[3][1], x[3][2], x[3][3]);
  mat3 minor11 = mat3( x[0][0], x[0][2], x[0][3],
                       x[2][0], x[2][2], x[2][3],
                       x[3][0], x[3][2], x[3][3]);
  mat3 minor12 = mat3( x[0][0], x[0][1], x[0][3],
                       x[2][0], x[2][1], x[2][3],
                       x[3][0], x[3][1], x[3][3]);
  mat3 minor13 = mat3( x[0][0], x[0][1], x[0][2],
                       x[2][0], x[2][1], x[2][2],
                       x[3][0], x[3][1], x[3][2]);

  mat3 minor20 = mat3( x[0][1], x[0][2], x[0][3],
                       x[1][1], x[1][2], x[1][3],
                       x[3][1], x[3][2], x[3][3]);
  mat3 minor21 = mat3( x[0][0], x[0][2], x[0][3],
                       x[1][0], x[1][2], x[1][3],
                       x[3][0], x[3][2], x[3][3]);
  mat3 minor22 = mat3( x[0][0], x[0][1], x[0][3],
                       x[1][0], x[1][1], x[1][3],
                       x[3][0], x[3][1], x[3][3]);
  mat3 minor23 = mat3( x[0][0], x[0][1], x[0][2],
                       x[1][0], x[1][1], x[1][2],
                       x[3][0], x[3][1], x[3][2]);

  mat3 minor30 = mat3( x[0][1], x[0][2], x[0][3],
                       x[1][1], x[1][2], x[1][3],
                       x[2][1], x[2][2], x[2][3]);
  mat3 minor31 = mat3( x[0][0], x[0][2], x[0][3],
                       x[1][0], x[1][2], x[1][3],
                       x[2][0], x[2][2], x[2][3]);
  mat3 minor32 = mat3( x[0][0], x[0][1], x[0][3],
                       x[1][0], x[1][1], x[1][3],
                       x[2][0], x[2][1], x[2][3]);
  mat3 minor33 = mat3( x[0][0], x[0][1], x[0][2],
                       x[1][0], x[1][1], x[1][2],
                       x[2][0], x[2][1], x[2][2]);

  det = determinant(x);
  res = mat4(  determinant(minor00), -determinant(minor10),  determinant(minor20), -determinant(minor30),
              -determinant(minor01),  determinant(minor11), -determinant(minor21),  determinant(minor31),
               determinant(minor02), -determinant(minor12),  determinant(minor22), -determinant(minor32),
              -determinant(minor03),  determinant(minor13), -determinant(minor23),  determinant(minor33));

  return (1.0/det)*res;
}


mat2 transpose(mat2 x)
{
  return mat2(x[0][0], x[1][0],
              x[0][1], x[1][1]);
}


mat3 transpose(mat3 x)
{
  return mat3(x[0][0], x[1][0], x[2][0],
              x[0][1], x[1][1], x[2][1],
              x[0][2], x[1][2], x[2][2]);
}


mat4 transpose(mat4 x)
{
  return mat4(x[0][0], x[1][0], x[2][0], x[3][0],
              x[0][1], x[1][1], x[2][1], x[3][1],
              x[0][2], x[1][2], x[2][2], x[3][2],
              x[0][3], x[1][3], x[2][3], x[3][3]);
}


mat2x3 transpose(mat3x2 x)
{
  return mat2x3(x[0][0], x[1][0], x[2][0],
                x[0][1], x[1][1], x[2][1]);
}


mat3x2 transpose(mat2x3 x)
{
  return mat3x2(x[0][0], x[1][0],
                x[0][1], x[1][1],
                x[0][2], x[1][2]);
}


mat2x4 transpose(mat4x2 x)
{
  return mat2x4(x[0][0], x[1][0], x[2][0], x[3][0],
                x[0][1], x[1][1], x[2][1], x[3][1]);
}


mat4x2 transpose(mat2x4 x)
{
  return mat4x2(x[0][0], x[1][0],
                x[0][1], x[1][1],
                x[0][2], x[1][2],
                x[0][3], x[1][3]);
}


mat3x4 transpose(mat4x3 x)
{
  return mat3x4(x[0][0], x[1][0], x[2][0], x[3][0],
                x[0][1], x[1][1], x[2][1], x[3][1],
                x[0][2], x[1][2], x[2][2], x[3][2]);
}


mat4x3 transpose(mat3x4 x)
{
  return mat4x3(x[0][0], x[1][0], x[2][0],
                x[0][1], x[1][1], x[2][1],
                x[0][2], x[1][2], x[2][2],
                x[0][3], x[1][3], x[2][3]);
}


mat2 outerProduct(vec2 c, vec2 r)
{
   return mat2(c[0]*r[0], c[1]*r[0],
               c[0]*r[1], c[1]*r[1]);
}


mat3 outerProduct(vec3 c, vec3 r)
{
   return mat3(c[0]*r[0], c[1]*r[0], c[2]*r[0],
               c[0]*r[1], c[1]*r[1], c[2]*r[1],
               c[0]*r[2], c[1]*r[2], c[2]*r[2]);
}


mat4 outerProduct(vec4 c, vec4 r)
{
   return mat4(c[0]*r[0], c[1]*r[0], c[2]*r[0], c[3]*r[0],
               c[0]*r[1], c[1]*r[1], c[2]*r[1], c[3]*r[1],
               c[0]*r[2], c[1]*r[2], c[2]*r[2], c[3]*r[2],
               c[0]*r[3], c[1]*r[3], c[2]*r[3], c[3]*r[3]);
}


mat2x3 outerProduct(vec3 c, vec2 r)
{
   return mat2x3(c[0]*r[0], c[1]*r[0], c[2]*r[0],
                 c[0]*r[1], c[1]*r[1], c[2]*r[1]);
}


mat3x2 outerProduct(vec2 c, vec3 r)
{
   return mat3x2(c[0]*r[0], c[1]*r[0],
                 c[0]*r[1], c[1]*r[1],
                 c[0]*r[2], c[1]*r[2]);
}


mat2x4 outerProduct(vec4 c, vec2 r)
{
   return mat2x4(c[0]*r[0], c[1]*r[0], c[2]*r[0], c[3]*r[0],
                 c[0]*r[1], c[1]*r[1], c[2]*r[1], c[3]*r[1]);
}


mat4x2 outerProduct(vec2 c, vec4 r)
{
   return mat4x2(c[0]*r[0], c[1]*r[0],
                 c[0]*r[1], c[1]*r[1],
                 c[0]*r[2], c[1]*r[2],
                 c[0]*r[3], c[1]*r[3]);
}


mat3x4 outerProduct(vec4 c, vec3 r)
{
   return mat3x4(c[0]*r[0], c[1]*r[0], c[2]*r[0], c[3]*r[0],
                 c[0]*r[1], c[1]*r[1], c[2]*r[1], c[3]*r[1],
                 c[0]*r[2], c[1]*r[2], c[2]*r[2], c[3]*r[2]);
}


mat4x3 outerProduct(vec3 c, vec4 r)
{
   return mat4x3(c[0]*r[0], c[1]*r[0], c[2]*r[0],
                 c[0]*r[1], c[1]*r[1], c[2]*r[1],
                 c[0]*r[2], c[1]*r[2], c[2]*r[2],
                 c[0]*r[3], c[1]*r[3], c[2]*r[3]);
}
