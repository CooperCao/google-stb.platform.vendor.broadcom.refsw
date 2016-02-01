static void matrixCompMult__const_mat2__const_mat2__const_mat2(const_mat2* result, const_mat2* x, const_mat2* y)
{
   int i, j;

   for (i = 0; i < 2; i++)
      for (j = 0; j < 2; j++)
         (*result)[i][j] = op_f_mul((*x)[i][j], (*y)[i][j]);
}

static void matrixCompMult__const_mat2x3__const_mat2x3__const_mat2x3(const_mat2x3* result, const_mat2x3* x, const_mat2x3* y)
{
   int i, j;

   for (i = 0; i < 2; i++)
      for (j = 0; j < 3; j++)
         (*result)[i][j] = op_f_mul((*x)[i][j], (*y)[i][j]);
}

static void matrixCompMult__const_mat2x4__const_mat2x4__const_mat2x4(const_mat2x4* result, const_mat2x4* x, const_mat2x4* y)
{
   int i, j;

   for (i = 0; i < 2; i++)
      for (j = 0; j < 4; j++)
         (*result)[i][j] = op_f_mul((*x)[i][j], (*y)[i][j]);
}

static void matrixCompMult__const_mat3x2__const_mat3x2__const_mat3x2(const_mat3x2* result, const_mat3x2* x, const_mat3x2* y)
{
   int i, j;

   for (i = 0; i < 3; i++)
      for (j = 0; j < 2; j++)
         (*result)[i][j] = op_f_mul((*x)[i][j], (*y)[i][j]);
}

static void matrixCompMult__const_mat3__const_mat3__const_mat3(const_mat3* result, const_mat3* x, const_mat3* y)
{
   int i, j;

   for (i = 0; i < 3; i++)
      for (j = 0; j < 3; j++)
         (*result)[i][j] = op_f_mul((*x)[i][j], (*y)[i][j]);
}

static void matrixCompMult__const_mat3x4__const_mat3x4__const_mat3x4(const_mat3x4* result, const_mat3x4* x, const_mat3x4* y)
{
   int i, j;

   for (i = 0; i < 3; i++)
      for (j = 0; j < 4; j++)
         (*result)[i][j] = op_f_mul((*x)[i][j], (*y)[i][j]);
}

static void matrixCompMult__const_mat4x2__const_mat4x2__const_mat4x2(const_mat4x2* result, const_mat4x2* x, const_mat4x2* y)
{
   int i, j;

   for (i = 0; i < 4; i++)
      for (j = 0; j < 2; j++)
         (*result)[i][j] = op_f_mul((*x)[i][j], (*y)[i][j]);
}

static void matrixCompMult__const_mat4x3__const_mat4x3__const_mat4x3(const_mat4x3* result, const_mat4x3* x, const_mat4x3* y)
{
   int i, j;

   for (i = 0; i < 4; i++)
      for (j = 0; j < 3; j++)
         (*result)[i][j] = op_f_mul((*x)[i][j], (*y)[i][j]);
}

static void matrixCompMult__const_mat4__const_mat4__const_mat4(const_mat4* result, const_mat4* x, const_mat4* y)
{
   int i, j;

   for (i = 0; i < 4; i++)
      for (j = 0; j < 4; j++)
         (*result)[i][j] = op_f_mul((*x)[i][j], (*y)[i][j]);
}

static void determinant__const_float__const_mat2(const_float *result, const_mat2 *m)
{
   *result = op_f_sub( op_f_mul((*m)[0][0], (*m)[1][1]), op_f_mul((*m)[0][1], (*m)[1][0]));
}

static void determinant__const_float__const_mat3(const_float *result, const_mat3 *m)
{
   const_mat2 minor00, minor01, minor02;
   const_float det00, det01, det02;
   minor00[0][0] = (*m)[1][1]; minor00[0][1] = (*m)[1][2];
   minor00[1][0] = (*m)[2][1]; minor00[1][1] = (*m)[2][2];

   minor01[0][0] = (*m)[1][0]; minor01[0][1] = (*m)[1][2];
   minor01[1][0] = (*m)[2][0]; minor01[1][1] = (*m)[2][2];

   minor02[0][0] = (*m)[1][0]; minor02[0][1] = (*m)[1][1];
   minor02[1][0] = (*m)[2][0]; minor02[1][1] = (*m)[2][1];

   determinant__const_float__const_mat2(&det00, &minor00);
   determinant__const_float__const_mat2(&det01, &minor01);
   determinant__const_float__const_mat2(&det02, &minor02);

   det00 = op_f_mul((*m)[0][0], det00);
   det01 = op_f_mul((*m)[0][1], det01);
   det02 = op_f_mul((*m)[0][2], det02);

   *result = op_f_add( op_f_sub(det00, det01), det02);
}

static void determinant__const_float__const_mat4(const_float *result, const_mat4 *m)
{
   const_mat3 minor00, minor01, minor02, minor03;
   const_float det00, det01, det02, det03;

   minor00[0][0] = (*m)[1][1]; minor00[0][1] = (*m)[1][2]; minor00[0][2] = (*m)[1][3];
   minor00[1][0] = (*m)[2][1]; minor00[1][1] = (*m)[2][2]; minor00[1][2] = (*m)[2][3];
   minor00[2][0] = (*m)[3][1]; minor00[2][1] = (*m)[3][2]; minor00[2][2] = (*m)[3][3];

   minor01[0][0] = (*m)[1][0]; minor01[0][1] = (*m)[1][2]; minor01[0][2] = (*m)[1][3];
   minor01[1][0] = (*m)[2][0]; minor01[1][1] = (*m)[2][2]; minor01[1][2] = (*m)[2][3];
   minor01[2][0] = (*m)[3][0]; minor01[2][1] = (*m)[3][2]; minor01[2][2] = (*m)[3][3];

   minor02[0][0] = (*m)[1][0]; minor02[0][1] = (*m)[1][1]; minor02[0][2] = (*m)[1][3];
   minor02[1][0] = (*m)[2][0]; minor02[1][1] = (*m)[2][1]; minor02[1][2] = (*m)[2][3];
   minor02[2][0] = (*m)[3][0]; minor02[2][1] = (*m)[3][1]; minor02[2][2] = (*m)[3][3];

   minor03[0][0] = (*m)[1][0]; minor03[0][1] = (*m)[1][1]; minor03[0][2] = (*m)[1][2];
   minor03[1][0] = (*m)[2][0]; minor03[1][1] = (*m)[2][1]; minor03[1][2] = (*m)[2][2];
   minor03[2][0] = (*m)[3][0]; minor03[2][1] = (*m)[3][1]; minor03[2][2] = (*m)[3][2];

   determinant__const_float__const_mat3(&det00, &minor00);
   determinant__const_float__const_mat3(&det01, &minor01);
   determinant__const_float__const_mat3(&det02, &minor02);
   determinant__const_float__const_mat3(&det03, &minor03);

   det00 = op_f_mul((*m)[0][0], det00);
   det01 = op_f_mul((*m)[0][1], det01);
   det02 = op_f_mul((*m)[0][2], det02);
   det03 = op_f_mul((*m)[0][3], det03);

   *result = op_f_sub(op_f_add( op_f_sub(det00, det01), det02), det03);
}

static void inverse__const_mat2__const_mat2(const_mat2 *result, const_mat2 *m)
{
   const_float det;
   determinant__const_float__const_mat2(&det, m);
   (*result)[0][0] = (*m)[1][1];
   (*result)[1][0] = op_f_negate((*m)[1][0]);
   (*result)[0][1] = op_f_negate((*m)[0][1]);
   (*result)[1][1] = (*m)[0][0];

   for (int i=0; i<2; i++) {
      for (int j=0; j<2; j++) {
         (*result)[i][j] = op_f_mul(op_recip(det), (*result)[i][j]);
      }
   }
}

static void inverse__const_mat3__const_mat3(const_mat3 *result, const_mat3 *m)
{
   const_float det;
   const_mat2 minor[3][3];

   for (int minor_x=0; minor_x<3; minor_x++) {
      for (int minor_y=0; minor_y<3; minor_y++) {
         for (int i=0; i<2; i++) {
            for (int j=0; j<2; j++) {
               int src_i = i < minor_x ? i : i+1;
               int src_j = j < minor_y ? j : j+1;
               minor[minor_x][minor_y][i][j] = (*m)[src_i][src_j];
            }
         }
      }
   }

   determinant__const_float__const_mat3(&det, m);
   for (int i=0; i<3; i++) {
      for (int j=0; j<3; j++) {
         determinant__const_float__const_mat2(&(*result)[i][j], &minor[j][i]);
         if ((i&1) ^ (j&1)) (*result)[i][j] = op_f_negate((*result)[i][j]);

         (*result)[i][j] = op_f_mul(op_recip(det), (*result)[i][j]);
      }
   }
}

static void inverse__const_mat4__const_mat4(const_mat4 *result, const_mat4 *m)
{
   const_float det;
   const_mat3 minor[4][4];

   for (int minor_x=0; minor_x<4; minor_x++) {
      for (int minor_y=0; minor_y<4; minor_y++) {
         for (int i=0; i<3; i++) {
            for (int j=0; j<3; j++) {
               int src_i = i < minor_x ? i : i+1;
               int src_j = j < minor_y ? j : j+1;
               minor[minor_x][minor_y][i][j] = (*m)[src_i][src_j];
            }
         }
      }
   }

   determinant__const_float__const_mat4(&det, m);
   for (int i=0; i<4; i++) {
      for (int j=0; j<4; j++) {
         determinant__const_float__const_mat3(&(*result)[i][j], &minor[j][i]);
         if ((i&1) ^ (j&1)) (*result)[i][j] = op_f_negate((*result)[i][j]);

         (*result)[i][j] = op_f_mul(op_recip(det), (*result)[i][j]);
      }
   }
}

static void transpose__const_mat2__const_mat2(const_mat2 *result, const_mat2 *m)
{
   /* Do we need to worry about aliasing here? The code is more complex, just in case */
   int i, j;
   for (i=0; i<2; i++) {
      for (j=i; j<2; j++) {
         const_float temp;
         temp = (*m)[i][j];
         (*result)[i][j] = (*m)[j][i];
         (*result)[j][i] = temp;
      }
   }
}
static void transpose__const_mat3__const_mat3(const_mat3 *result, const_mat3 *m)
{
   int i, j;
   for (i=0; i<3; i++) {
      for (j=i; j<3; j++) {
         const_float temp;
         temp = (*m)[i][j];
         (*result)[i][j] = (*m)[j][i];
         (*result)[j][i] = temp;
      }
   }
}
static void transpose__const_mat4__const_mat4(const_mat4 *result, const_mat4 *m)
{
   int i, j;
   for (i=0; i<4; i++) {
      for (j=i; j<4; j++) {
         const_float temp;
         temp = (*m)[i][j];
         (*result)[i][j] = (*m)[j][i];
         (*result)[j][i] = temp;
      }
   }
}

static void transpose__const_mat2x3__const_mat3x2(const_mat2x3 *result, const_mat3x2 *m)
{
   /* No aliasing here because types differ... */
   int i, j;
   for (i=0; i<2; i++)
      for (j=0; j<3; j++)
         (*result)[i][j] = (*m)[j][i];
}
static void transpose__const_mat3x2__const_mat2x3(const_mat3x2 *result, const_mat2x3 *m)
{
   int i, j;
   for (i=0; i<3; i++)
      for (j=0; j<2; j++)
         (*result)[i][j] = (*m)[j][i];
}

static void transpose__const_mat2x4__const_mat4x2(const_mat2x4 *result, const_mat4x2 *m)
{
   int i, j;
   for (i=0; i<2; i++)
      for (j=0; j<4; j++)
         (*result)[i][j] = (*m)[j][i];
}
static void transpose__const_mat4x2__const_mat2x4(const_mat4x2 *result, const_mat2x4 *m)
{
   int i, j;
   for (i=0; i<4; i++)
      for (j=0; j<2; j++)
         (*result)[i][j] = (*m)[j][i];
}

static void transpose__const_mat3x4__const_mat4x3(const_mat3x4 *result, const_mat4x3 *m)
{
   int i, j;
   for (i=0; i<3; i++)
      for (j=0; j<4; j++)
         (*result)[i][j] = (*m)[j][i];
}
static void transpose__const_mat4x3__const_mat3x4(const_mat4x3 *result, const_mat3x4 *m)
{
   int i, j;
   for (i=0; i<4; i++)
      for (j=0; j<3; j++)
         (*result)[i][j] = (*m)[j][i];
}

static void outerProduct__const_mat2__const_vec2__const_vec2(const_mat2 *result, const_vec2 *c, const_vec2 *r)
{
   int i, j;
   for (i=0; i<2; i++)
      for (j=0; j<2; j++)
         (*result)[i][j] = op_f_mul((*r)[i], (*c)[j]);
}
static void outerProduct__const_mat3__const_vec3__const_vec3(const_mat3 *result, const_vec3 *c, const_vec3 *r)
{
   int i, j;
   for (i=0; i<3; i++)
      for (j=0; j<3; j++)
         (*result)[i][j] = op_f_mul((*r)[i], (*c)[j]);
}
static void outerProduct__const_mat4__const_vec4__const_vec4(const_mat4 *result, const_vec4 *c, const_vec4 *r)
{
   int i, j;
   for (i=0; i<4; i++)
      for (j=0; j<4; j++)
         (*result)[i][j] = op_f_mul((*r)[i], (*c)[j]);
}

static void outerProduct__const_mat2x3__const_vec3__const_vec2(const_mat2x3 *result, const_vec3 *c, const_vec2 *r)
{
   int i, j;
   for (i=0; i<2; i++)
      for (j=0; j<3; j++)
         (*result)[i][j] = op_f_mul((*r)[i], (*c)[j]);
}
static void outerProduct__const_mat3x2__const_vec2__const_vec3(const_mat3x2 *result, const_vec2 *c, const_vec3 *r)
{
   int i, j;
   for (i=0; i<3; i++)
      for (j=0; j<2; j++)
         (*result)[i][j] = op_f_mul((*r)[i], (*c)[j]);
}

static void outerProduct__const_mat2x4__const_vec4__const_vec2(const_mat2x4 *result, const_vec4 *c, const_vec2 *r)
{
   int i, j;
   for (i=0; i<2; i++)
      for (j=0; j<4; j++)
         (*result)[i][j] = op_f_mul((*r)[i], (*c)[j]);
}
static void outerProduct__const_mat4x2__const_vec2__const_vec4(const_mat4x2 *result, const_vec2 *c, const_vec4 *r)
{
   int i, j;
   for (i=0; i<4; i++)
      for (j=0; j<2; j++)
         (*result)[i][j] = op_f_mul((*r)[i], (*c)[j]);
}

static void outerProduct__const_mat3x4__const_vec4__const_vec3(const_mat3x4 *result, const_vec4 *c, const_vec3 *r)
{
   int i, j;
   for (i=0; i<3; i++)
      for (j=0; j<4; j++)
         (*result)[i][j] = op_f_mul((*r)[i], (*c)[j]);
}
static void outerProduct__const_mat4x3__const_vec3__const_vec4(const_mat4x3 *result, const_vec3 *c, const_vec4 *r)
{
   int i, j;
   for (i=0; i<4; i++)
      for (j=0; j<3; j++)
         (*result)[i][j] = op_f_mul((*r)[i], (*c)[j]);
}
