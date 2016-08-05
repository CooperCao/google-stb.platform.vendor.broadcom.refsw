//
// Geometric Functions
//

float dot(float x, float y) { return x*y; }
float dot(vec2  x, vec2  y) { return x[0]*y[0] + x[1]*y[1]; }
float dot(vec3  x, vec3  y) { return x[0]*y[0] + x[1]*y[1] + x[2]*y[2]; }
float dot(vec4  x, vec4  y) { return x[0]*y[0] + x[1]*y[1] + x[2]*y[2] + x[3]*y[3]; }

float length(float x) { return abs(x); }
float length(vec2  x) { return $$rcp($$rsqrt(dot(x, x))); }
float length(vec3  x) { return $$rcp($$rsqrt(dot(x, x))); }
float length(vec4  x) { return $$rcp($$rsqrt(dot(x, x))); }

float distance(float p0, float p1) { return length(p0 - p1); }
float distance(vec2  p0, vec2  p1) { return length(p0 - p1); }
float distance(vec3  p0, vec3  p1) { return length(p0 - p1); }
float distance(vec4  p0, vec4  p1) { return length(p0 - p1); }

vec3 cross(vec3 x, vec3 y) { return vec3(x[1]*y[2] - x[2]*y[1], x[2]*y[0] - x[0]*y[2], x[0]*y[1] - x[1]*y[0]); }

float normalize(float x) { return sign(x); }
vec2  normalize(vec2  x) { return x * $$rsqrt(dot(x, x)); }
vec3  normalize(vec3  x) { return x * $$rsqrt(dot(x, x)); }
vec4  normalize(vec4  x) { return x * $$rsqrt(dot(x, x)); }


float faceforward(float N, float I, float Nref)
{
   if (dot(Nref, I) < 0.0)
      return N;
   else
      return -N;
}

vec2 faceforward(vec2 N, vec2 I, vec2 Nref)
{
   if (dot(Nref, I) < 0.0)
      return N;
   else
      return -N;
}

vec3 faceforward(vec3 N, vec3 I, vec3 Nref)
{
   if (dot(Nref, I) < 0.0)
      return N;
   else
      return -N;
}

vec4 faceforward(vec4 N, vec4 I, vec4 Nref)
{
   if (dot(Nref, I) < 0.0)
      return N;
   else
      return -N;
}

float reflect(float I, float N) { return I - 2.0 * dot(N, I) * N; }
vec2  reflect(vec2  I, vec2  N) { return I - 2.0 * dot(N, I) * N; }
vec3  reflect(vec3  I, vec3  N) { return I - 2.0 * dot(N, I) * N; }
vec4  reflect(vec4  I, vec4  N) { return I - 2.0 * dot(N, I) * N; }


float refract(float I, float N, float eta)
{
   highp float d = dot(N, I);
   highp float k = 1.0 - eta * eta * (1.0 - d * d);
   if (k < 0.0)
      return float(0.0);
   else
      return eta * I - (eta * d + sqrt(k)) * N;
}

vec2 refract(vec2 I, vec2 N, float eta)
{
   highp float d = dot(N, I);
   highp float k = 1.0 - eta * eta * (1.0 - d * d);
   if (k < 0.0)
      return vec2(0.0);
   else
      return eta * I - (eta * d + sqrt(k)) * N;
}

vec3 refract(vec3 I, vec3 N, float eta)
{
   highp float d = dot(N, I);
   highp float k = 1.0 - eta * eta * (1.0 - d * d);
   if (k < 0.0)
      return vec3(0.0);
   else
      return eta * I - (eta * d + sqrt(k)) * N;
}

vec4 refract(vec4 I, vec4 N, float eta)
{
   highp float d = dot(N, I);
   highp float k = 1.0 - eta * eta * (1.0 - d * d);
   if (k < 0.0)
      return vec4(0.0);
   else
      return eta * I - (eta * d + sqrt(k)) * N;
}
