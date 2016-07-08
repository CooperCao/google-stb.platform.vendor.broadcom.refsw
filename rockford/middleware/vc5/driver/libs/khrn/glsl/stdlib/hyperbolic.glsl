//
// Hyperbolic Functions
//


float sinh(float x)
{
   const float e = 2.718281828;
   return 0.5*(pow(e, x)-pow(e, -x));
}


vec2 sinh(vec2 x)
{
   return vec2(sinh(x[0]), sinh(x[1]));
}


vec3 sinh(vec3 x)
{
   return vec3(sinh(x[0]), sinh(x[1]), sinh(x[2]));
}


vec4 sinh(vec4 x)
{
   return vec4(sinh(x[0]), sinh(x[1]), sinh(x[2]), sinh(x[3]));
}


float cosh(float x)
{
   const float e = 2.718281828;
   return 0.5*(pow(e, x)+pow(e, -x));
}


vec2 cosh(vec2 x)
{
   return vec2(cosh(x[0]), cosh(x[1]));
}


vec3 cosh(vec3 x)
{
   return vec3(cosh(x[0]), cosh(x[1]), cosh(x[2]));
}


vec4 cosh(vec4 x)
{
   return vec4(cosh(x[0]), cosh(x[1]), cosh(x[2]), cosh(x[3]));
}


float tanh(float x)
{
   return sinh(x)/cosh(x);
}


vec2 tanh(vec2 x)
{
   return vec2(tanh(x[0]), tanh(x[1]));
}


vec3 tanh(vec3 x)
{
   return vec3(tanh(x[0]), tanh(x[1]), tanh(x[2]));
}


vec4 tanh(vec4 x)
{
   return vec4(tanh(x[0]), tanh(x[1]), tanh(x[2]), tanh(x[3]));
}


float asinh(float x)
{
   return log(x+sqrt(x*x+1.0));
}


vec2 asinh(vec2 x)
{
   return vec2(asinh(x[0]), asinh(x[1]));
}


vec3 asinh(vec3 x)
{
   return vec3(asinh(x[0]), asinh(x[1]), asinh(x[2]));
}


vec4 asinh(vec4 x)
{
   return vec4(asinh(x[0]), asinh(x[1]), asinh(x[2]), asinh(x[3]));
}


float acosh(float x)
{
   return log(x+sqrt((x-1.0)*(x+1.0)));
}


vec2 acosh(vec2 x)
{
   return vec2(acosh(x[0]), acosh(x[1]));
}


vec3 acosh(vec3 x)
{
   return vec3(acosh(x[0]), acosh(x[1]), acosh(x[2]));
}


vec4 acosh(vec4 x)
{
   return vec4(acosh(x[0]), acosh(x[1]), acosh(x[2]), acosh(x[3]));
}


float atanh(float x)
{
   return 0.5*log((1.0+x)/(1.0-x));
}


vec2 atanh(vec2 x)
{
   return vec2(atanh(x[0]), atanh(x[1]));
}


vec3 atanh(vec3 x)
{
   return vec3(atanh(x[0]), atanh(x[1]), atanh(x[2]));
}


vec4 atanh(vec4 x)
{
   return vec4(atanh(x[0]), atanh(x[1]), atanh(x[2]), atanh(x[3]));
}
