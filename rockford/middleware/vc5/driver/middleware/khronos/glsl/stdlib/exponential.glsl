//
// Exponential Functions
//


float pow(float x, float y)
{
   return $$exp2(y * $$log2(x));
}


vec2 pow(vec2 x, vec2 y)
{
   return vec2($$exp2(y[0] * $$log2(x[0])), $$exp2(y[1] * $$log2(x[1])));
}


vec3 pow(vec3 x, vec3 y)
{
   return vec3($$exp2(y[0] * $$log2(x[0])), $$exp2(y[1] * $$log2(x[1])), $$exp2(y[2] * $$log2(x[2])));
}


vec4 pow(vec4 x, vec4 y)
{
   return vec4($$exp2(y[0] * $$log2(x[0])), $$exp2(y[1] * $$log2(x[1])), $$exp2(y[2] * $$log2(x[2])), $$exp2(y[3] * $$log2(x[3])));
}


float exp2(float x)
{
   return $$exp2(x);
}


vec2 exp2(vec2 x)
{
   return vec2(exp2(x[0]), exp2(x[1]));
}


vec3 exp2(vec3 x)
{
   return vec3(exp2(x[0]), exp2(x[1]), exp2(x[2]));
}


vec4 exp2(vec4 x)
{
   return vec4(exp2(x[0]), exp2(x[1]), exp2(x[2]), exp2(x[3]));
}


float log2(float x)
{
   return $$log2(x);
}


vec2 log2(vec2 x)
{
   return vec2(log2(x[0]), log2(x[1]));
}


vec3 log2(vec3 x)
{
   return vec3(log2(x[0]), log2(x[1]), log2(x[2]));
}


vec4 log2(vec4 x)
{
   return vec4(log2(x[0]), log2(x[1]), log2(x[2]), log2(x[3]));
}


float exp(float x)
{
   const float e = 2.718281828;
   return pow(e, x);
}


vec2 exp(vec2 x)
{
   return vec2(exp(x[0]), exp(x[1]));
}


vec3 exp(vec3 x)
{
   return vec3(exp(x[0]), exp(x[1]), exp(x[2]));
}


vec4 exp(vec4 x)
{
   return vec4(exp(x[0]), exp(x[1]), exp(x[2]), exp(x[3]));
}


float log(float x)
{
   const float recip_log2_e = 0.6931471805;
   return $$log2(x) * recip_log2_e;
}


vec2 log(vec2 x)
{
   return vec2(log(x[0]), log(x[1]));
}


vec3 log(vec3 x)
{
   return vec3(log(x[0]), log(x[1]), log(x[2]));
}


vec4 log(vec4 x)
{
   return vec4(log(x[0]), log(x[1]), log(x[2]), log(x[3]));
}
