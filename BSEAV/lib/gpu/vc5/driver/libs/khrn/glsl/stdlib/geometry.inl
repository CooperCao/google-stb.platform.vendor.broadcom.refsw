static void length__const_float__const_float(const_float* result, const_float* x)
{
   // return abs(x);

   abs__const_float__const_float(result, x);
}
static void length__const_float__const_vec2(const_float* result, const_vec2* x)
{
   // return sqrt(dot(x, x));

   const_float t1;

   dot__const_float__const_vec2__const_vec2(&t1, x, x);

   *result = op_sqrt(t1);
}
static void length__const_float__const_vec3(const_float* result, const_vec3* x)
{
   // return sqrt(dot(x, x));

   const_float t1;

   dot__const_float__const_vec3__const_vec3(&t1, x, x);

   *result = op_sqrt(t1);
}
static void length__const_float__const_vec4(const_float* result, const_vec4* x)
{
   // return sqrt(dot(x, x));

   const_float t1;

   dot__const_float__const_vec4__const_vec4(&t1, x, x);

   *result = op_sqrt(t1);
}
static void distance__const_float__const_float__const_float(const_float* result, const_float* p0, const_float* p1)
{
   // return length(p0 - p1);

   const_float t1 = op_f_sub(*p0, *p1);

   length__const_float__const_float(result, &t1);
}
static void distance__const_float__const_vec2__const_vec2(const_float* result, const_vec2* p0, const_vec2* p1)
{
   // return length(p0 - p1);

   const_vec2 t1;

   t1[0] = op_f_sub((*p0)[0], (*p1)[0]);
   t1[1] = op_f_sub((*p0)[1], (*p1)[1]);

   length__const_float__const_vec2(result, &t1);
}
static void distance__const_float__const_vec3__const_vec3(const_float* result, const_vec3* p0, const_vec3* p1)
{
   // return length(p0 - p1);

   const_vec3 t1;

   t1[0] = op_f_sub((*p0)[0], (*p1)[0]);
   t1[1] = op_f_sub((*p0)[1], (*p1)[1]);
   t1[2] = op_f_sub((*p0)[2], (*p1)[2]);

   length__const_float__const_vec3(result, &t1);
}
static void distance__const_float__const_vec4__const_vec4(const_float* result, const_vec4* p0, const_vec4* p1)
{
   // return length(p0 - p1);

   const_vec4 t1;

   t1[0] = op_f_sub((*p0)[0], (*p1)[0]);
   t1[1] = op_f_sub((*p0)[1], (*p1)[1]);
   t1[2] = op_f_sub((*p0)[2], (*p1)[2]);
   t1[3] = op_f_sub((*p0)[3], (*p1)[3]);

   length__const_float__const_vec4(result, &t1);
}
static void dot__const_float__const_float__const_float(const_float* result, const_float* x, const_float* y)
{
   // return x*y;

   *result = op_f_mul(*x, *y);
}
static void dot__const_float__const_vec2__const_vec2(const_float* result, const_vec2* x, const_vec2* y)
{
   // return x[0]*y[0] + x[1]*y[1];

   const_float t1, t2;

   t1 = op_f_mul((*x)[0], (*y)[0]);
   t2 = op_f_mul((*x)[1], (*y)[1]);
   *result = op_f_add(t1, t2);
}
static void dot__const_float__const_vec3__const_vec3(const_float* result, const_vec3* x, const_vec3* y)
{
   // return x[0]*y[0] + x[1]*y[1] + x[2]*y[2];

   const_float t1, t2, t3, t4;

   t1 = op_f_mul((*x)[0], (*y)[0]);
   t2 = op_f_mul((*x)[1], (*y)[1]);
   t3 = op_f_mul((*x)[2], (*y)[2]);
   t4 = op_f_add(t1, t2);
   *result = op_f_add(t4, t3);
}
static void dot__const_float__const_vec4__const_vec4(const_float* result, const_vec4* x, const_vec4* y)
{
   // return x[0]*y[0] + x[1]*y[1] + x[2]*y[2] + x[3]*y[3];

   const_float t1, t2, t3, t4, t5, t6;

   t1 = op_f_mul((*x)[0], (*y)[0]);
   t2 = op_f_mul((*x)[1], (*y)[1]);
   t3 = op_f_mul((*x)[2], (*y)[2]);
   t4 = op_f_mul((*x)[3], (*y)[3]);
   t5 = op_f_add(t1, t2);
   t6 = op_f_add(t5, t3);
   *result = op_f_add(t6, t4);
}
static void cross__const_vec3__const_vec3__const_vec3(const_vec3* result, const_vec3* x, const_vec3* y)
{
   // return vec3(x[1]*y[2] - x[2]*y[1], x[2]*y[0] - x[0]*y[2], x[0]*y[1] - x[1]*y[0]);

   const_float t1, t2;

   t1 = op_f_mul((*x)[1], (*y)[2]);
   t2 = op_f_mul((*x)[2], (*y)[1]);
   (*result)[0] = op_f_sub(t1, t2);

   t1 = op_f_mul((*x)[2], (*y)[0]);
   t2 = op_f_mul((*x)[0], (*y)[2]);
   (*result)[1] = op_f_sub(t1, t2);

   t1 = op_f_mul((*x)[0], (*y)[1]);
   t2 = op_f_mul((*x)[1], (*y)[0]);
   (*result)[2] = op_f_sub(t1, t2);
}
static void normalize__const_float__const_float(const_float* result, const_float* x)
{
   // return sign(x);
   sign__const_float__const_float(result, x);
}
static void normalize__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   // return x * $$rsqrt(dot(x, x));

   const_float t1, t2;

   dot__const_float__const_vec2__const_vec2(&t1, x, x);

   t2 = op_rsqrt(t1);
   (*result)[0] = op_f_mul((*x)[0], t2);
   (*result)[1] = op_f_mul((*x)[1], t2);
}
static void normalize__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   // return x * $$rsqrt(dot(x, x));

   const_float t1, t2;

   dot__const_float__const_vec3__const_vec3(&t1, x, x);

   t2 = op_rsqrt(t1);
   (*result)[0] = op_f_mul((*x)[0], t2);
   (*result)[1] = op_f_mul((*x)[1], t2);
   (*result)[2] = op_f_mul((*x)[2], t2);
}
static void normalize__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   // return x * $$rsqrt(dot(x, x));

   const_float t1, t2;

   dot__const_float__const_vec4__const_vec4(&t1, x, x);

   t2 = op_rsqrt(t1);
   (*result)[0] = op_f_mul((*x)[0], t2);
   (*result)[1] = op_f_mul((*x)[1], t2);
   (*result)[2] = op_f_mul((*x)[2], t2);
   (*result)[3] = op_f_mul((*x)[3], t2);
}
static void faceforward__const_float__const_float__const_float__const_float(const_float* result, const_float* N, const_float* I, const_float* Nref)
{
   /*
      if (dot(Nref, I) < 0.0)
         return N;
      else
         return -N;
   */
   const_float t1;
   dot__const_float__const_float__const_float(&t1, Nref, I);

   if (op_f_less_than(t1, CONST_FLOAT_ZERO))
      *result = *N;
   else
      *result = op_f_negate(*N);
}
static void faceforward__const_vec2__const_vec2__const_vec2__const_vec2(const_vec2* result, const_vec2* N, const_vec2* I, const_vec2* Nref)
{
   /*
      if (dot(Nref, I) < 0.0)
         return N;
      else
         return -N;
   */
   const_float t1;
   dot__const_float__const_vec2__const_vec2(&t1, Nref, I);

   if (op_f_less_than(t1, CONST_FLOAT_ZERO)) {
      (*result)[0] = (*N)[0];
      (*result)[1] = (*N)[1];
   } else {
      (*result)[0] = op_f_negate((*N)[0]);
      (*result)[1] = op_f_negate((*N)[1]);
   }
}
static void faceforward__const_vec3__const_vec3__const_vec3__const_vec3(const_vec3* result, const_vec3* N, const_vec3* I, const_vec3* Nref)
{
   /*
      if (dot(Nref, I) < 0.0)
         return N;
      else
         return -N;
   */
   const_float t1;
   dot__const_float__const_vec3__const_vec3(&t1, Nref, I);

   if (op_f_less_than(t1, CONST_FLOAT_ZERO)) {
      (*result)[0] = (*N)[0];
      (*result)[1] = (*N)[1];
      (*result)[2] = (*N)[2];
   } else {
      (*result)[0] = op_f_negate((*N)[0]);
      (*result)[1] = op_f_negate((*N)[1]);
      (*result)[2] = op_f_negate((*N)[2]);
   }
}
static void faceforward__const_vec4__const_vec4__const_vec4__const_vec4(const_vec4* result, const_vec4* N, const_vec4* I, const_vec4* Nref)
{
   /*
      if (dot(Nref, I) < 0.0)
         return N;
      else
         return -N;
   */
   const_float t1;
   dot__const_float__const_vec4__const_vec4(&t1, Nref, I);

   if (op_f_less_than(t1, CONST_FLOAT_ZERO)) {
      (*result)[0] = (*N)[0];
      (*result)[1] = (*N)[1];
      (*result)[2] = (*N)[2];
      (*result)[3] = (*N)[3];
   } else {
      (*result)[0] = op_f_negate((*N)[0]);
      (*result)[1] = op_f_negate((*N)[1]);
      (*result)[2] = op_f_negate((*N)[2]);
      (*result)[3] = op_f_negate((*N)[3]);
   }
}
static void reflect__const_float__const_float__const_float(const_float* result, const_float* I, const_float* N)
{
   // return I - 2.0 * dot(N, I) * N;

   const_float two = const_float_from_int(2);
   const_float t1, t2, t3;

   dot__const_float__const_float__const_float(&t1, N, I);

   t2 = op_f_mul(t1, two);

   t3 = op_f_mul(t2, *N);
   *result = op_f_sub(*I, t3);
}
static void reflect__const_vec2__const_vec2__const_vec2(const_vec2* result, const_vec2* I, const_vec2* N)
{
   // return I - 2.0 * dot(N, I) * N;

   const_float two = const_float_from_int(2);
   const_float t1, t2, t3;

   dot__const_float__const_vec2__const_vec2(&t1, N, I);

   t2 = op_f_mul(t1, two);

   t3 = op_f_mul(t2, (*N)[0]);
   (*result)[0] = op_f_sub((*I)[0], t3);

   t3 = op_f_mul(t2, (*N)[1]);
   (*result)[1] = op_f_sub((*I)[1], t3);
}
static void reflect__const_vec3__const_vec3__const_vec3(const_vec3* result, const_vec3* I, const_vec3* N)
{
   // return I - 2.0 * dot(N, I) * N;

   const_float two = const_float_from_int(2);
   const_float t1, t2, t3;

   dot__const_float__const_vec3__const_vec3(&t1, N, I);

   t2 = op_f_mul(t1, two);

   t3 = op_f_mul(t2, (*N)[0]);
   (*result)[0] = op_f_sub((*I)[0], t3);

   t3 = op_f_mul(t2, (*N)[1]);
   (*result)[1] = op_f_sub((*I)[1], t3);

   t3 = op_f_mul(t2, (*N)[2]);
   (*result)[2] = op_f_sub((*I)[2], t3);
}
static void reflect__const_vec4__const_vec4__const_vec4(const_vec4* result, const_vec4* I, const_vec4* N)
{
   // return I - 2.0 * dot(N, I) * N;

   const_float two = const_float_from_int(2);
   const_float t1, t2, t3;

   dot__const_float__const_vec4__const_vec4(&t1, N, I);

   t2 = op_f_mul(t1, two);

   t3 = op_f_mul(t2, (*N)[0]);
   (*result)[0] = op_f_sub((*I)[0], t3);

   t3 = op_f_mul(t2, (*N)[1]);
   (*result)[1] = op_f_sub((*I)[1], t3);

   t3 = op_f_mul(t2, (*N)[2]);
   (*result)[2] = op_f_sub((*I)[2], t3);

   t3 = op_f_mul(t2, (*N)[3]);
   (*result)[3] = op_f_sub((*I)[3], t3);
}

static const_value calc_k(const_float d, const_float eta)
{
   const_float one = CONST_FLOAT_ONE;
   const_float t1, t2, t3, t4;

   t1 = op_f_mul(d,   d);
   t2 = op_f_sub(one, t1);
   t3 = op_f_mul(eta, eta);
   t4 = op_f_mul(t2,  t3);
   return op_f_sub( one,  t4);
}

static void refract__const_float__const_float__const_float__const_float(const_float* result, const_float* I, const_float* N, const_float* eta)
{
   /*
      float d = dot(N, I);
      float k = 1.0 - eta * eta * (1.0 - d * d);
      if (k < 0.0)
         return float(0.0);
      else
         return eta * I - (eta * d + sqrt(k)) * N;
   */
   const_float d;
   dot__const_float__const_float__const_float(&d, N, I);

   const_float k = calc_k(d, *eta);

   if (op_f_less_than(k, CONST_FLOAT_ZERO)) {
      *result = CONST_FLOAT_ZERO;
   } else {
      const_float t1, t2, t3, t4, t5;

      sqrt__const_float__const_float(&t1, &k);

      t2 = op_f_mul(*eta, d);
      t3 = op_f_add( t1, t2);

      t4 = op_f_mul(*eta, *I);
      t5 = op_f_mul( t3,  *N);
      *result = op_f_sub(t4, t5);
   }
}
static void refract__const_vec2__const_vec2__const_vec2__const_float(const_vec2* result, const_vec2* I, const_vec2* N, const_float* eta)
{
   /*
      float d = dot(N, I);
      float k = 1.0 - eta * eta * (1.0 - d * d);
      if (k < 0.0)
         return float(0.0);
      else
         return eta * I - (eta * d + sqrt(k)) * N;
   */
   const_float d;
   dot__const_float__const_vec2__const_vec2(&d, N, I);

   const_float k = calc_k(d, *eta);

   if (op_f_less_than(k, CONST_FLOAT_ZERO)) {
      (*result)[0] = CONST_FLOAT_ZERO;
      (*result)[1] = CONST_FLOAT_ZERO;
   } else {
      const_float t1, t2, t3, t4, t5;

      sqrt__const_float__const_float(&t1, &k);

      t2 = op_f_mul(*eta, d);
      t3 = op_f_add( t1, t2);

      for (int j=0; j<2; j++) {
         t4 = op_f_mul(*eta, (*I)[j]);
         t5 = op_f_mul( t3,  (*N)[j]);
         (*result)[j] = op_f_sub(t4, t5);
      }
   }
}
static void refract__const_vec3__const_vec3__const_vec3__const_float(const_vec3* result, const_vec3* I, const_vec3* N, const_float* eta)
{
   /*
      float d = dot(N, I);
      float k = 1.0 - eta * eta * (1.0 - d * d);
      if (k < 0.0)
         return float(0.0);
      else
         return eta * I - (eta * d + sqrt(k)) * N;
   */
   const_float d;
   dot__const_float__const_vec3__const_vec3(&d, N, I);

   const_float k = calc_k(d, *eta);

   if (op_f_less_than(k, CONST_FLOAT_ZERO)) {
      (*result)[0] = CONST_FLOAT_ZERO;
      (*result)[1] = CONST_FLOAT_ZERO;
      (*result)[2] = CONST_FLOAT_ZERO;
   } else {
      const_float t1, t2, t3, t4, t5;

      sqrt__const_float__const_float(&t1, &k);

      t2 = op_f_mul(*eta, d);
      t3 = op_f_add( t1,  t2);

      for (int j=0; j<3; j++) {
         t4 = op_f_mul(*eta, (*I)[j]);
         t5 = op_f_mul(  t3, (*N)[j]);
         (*result)[j] = op_f_sub(t4, t5);
      }
   }
}
static void refract__const_vec4__const_vec4__const_vec4__const_float(const_vec4* result, const_vec4* I, const_vec4* N, const_float* eta)
{
   /*
      float d = dot(N, I);
      float k = 1.0 - eta * eta * (1.0 - d * d);
      if (k < 0.0)
         return float(0.0);
      else
         return eta * I - (eta * d + sqrt(k)) * N;
   */
   const_float d;
   dot__const_float__const_vec4__const_vec4(&d, N, I);

   const_float k = calc_k(d, *eta);

   if (op_f_less_than(k, CONST_FLOAT_ZERO)) {
      (*result)[0] = CONST_FLOAT_ZERO;
      (*result)[1] = CONST_FLOAT_ZERO;
      (*result)[2] = CONST_FLOAT_ZERO;
      (*result)[3] = CONST_FLOAT_ZERO;
   } else {
      const_float t1, t2, t3, t4, t5;

      sqrt__const_float__const_float(&t1, &k);

      t2 = op_f_mul(*eta, d);
      t3 = op_f_add( t1,  t2);

      for (int j=0; j<4; j++) {
         t4 = op_f_mul(*eta, (*I)[j]);
         t5 = op_f_mul(  t3, (*N)[j]);
         (*result)[j] = op_f_sub(t4, t5);
      }
   }
}
