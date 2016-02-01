//
// Vector Relational Functions
//


bvec2 lessThan(vec2 x, vec2 y)
{
   return bvec2(x[0] < y[0], x[1] < y[1]);
}


bvec3 lessThan(vec3 x, vec3 y)
{
   return bvec3(x[0] < y[0], x[1] < y[1], x[2] < y[2]);
}


bvec4 lessThan(vec4 x, vec4 y)
{
   return bvec4(x[0] < y[0], x[1] < y[1], x[2] < y[2], x[3] < y[3]);
}


bvec2 lessThan(ivec2 x, ivec2 y)
{
   return bvec2(x[0] < y[0], x[1] < y[1]);
}


bvec3 lessThan(ivec3 x, ivec3 y)
{
   return bvec3(x[0] < y[0], x[1] < y[1], x[2] < y[2]);
}


bvec4 lessThan(ivec4 x, ivec4 y)
{
   return bvec4(x[0] < y[0], x[1] < y[1], x[2] < y[2], x[3] < y[3]);
}


bvec2 lessThan(uvec2 x, uvec2 y)
{
   return bvec2(x[0] < y[0], x[1] < y[1]);
}


bvec3 lessThan(uvec3 x, uvec3 y)
{
   return bvec3(x[0] < y[0], x[1] < y[1], x[2] < y[2]);
}


bvec4 lessThan(uvec4 x, uvec4 y)
{
   return bvec4(x[0] < y[0], x[1] < y[1], x[2] < y[2], x[3] < y[3]);
}


bvec2 lessThanEqual(vec2 x, vec2 y)
{
   return bvec2(x[0] <= y[0], x[1] <= y[1]);
}


bvec3 lessThanEqual(vec3 x, vec3 y)
{
   return bvec3(x[0] <= y[0], x[1] <= y[1], x[2] <= y[2]);
}


bvec4 lessThanEqual(vec4 x, vec4 y)
{
   return bvec4(x[0] <= y[0], x[1] <= y[1], x[2] <= y[2], x[3] <= y[3]);
}


bvec2 lessThanEqual(ivec2 x, ivec2 y)
{
   return bvec2(x[0] <= y[0], x[1] <= y[1]);
}


bvec3 lessThanEqual(ivec3 x, ivec3 y)
{
   return bvec3(x[0] <= y[0], x[1] <= y[1], x[2] <= y[2]);
}


bvec4 lessThanEqual(ivec4 x, ivec4 y)
{
   return bvec4(x[0] <= y[0], x[1] <= y[1], x[2] <= y[2], x[3] <= y[3]);
}


bvec2 lessThanEqual(uvec2 x, uvec2 y)
{
   return bvec2(x[0] <= y[0], x[1] <= y[1]);
}


bvec3 lessThanEqual(uvec3 x, uvec3 y)
{
   return bvec3(x[0] <= y[0], x[1] <= y[1], x[2] <= y[2]);
}


bvec4 lessThanEqual(uvec4 x, uvec4 y)
{
   return bvec4(x[0] <= y[0], x[1] <= y[1], x[2] <= y[2], x[3] <= y[3]);
}


bvec2 greaterThan(vec2 x, vec2 y)
{
   return bvec2(x[0] > y[0], x[1] > y[1]);
}


bvec3 greaterThan(vec3 x, vec3 y)
{
   return bvec3(x[0] > y[0], x[1] > y[1], x[2] > y[2]);
}


bvec4 greaterThan(vec4 x, vec4 y)
{
   return bvec4(x[0] > y[0], x[1] > y[1], x[2] > y[2], x[3] > y[3]);
}


bvec2 greaterThan(ivec2 x, ivec2 y)
{
   return bvec2(x[0] > y[0], x[1] > y[1]);
}


bvec3 greaterThan(ivec3 x, ivec3 y)
{
   return bvec3(x[0] > y[0], x[1] > y[1], x[2] > y[2]);
}


bvec4 greaterThan(ivec4 x, ivec4 y)
{
   return bvec4(x[0] > y[0], x[1] > y[1], x[2] > y[2], x[3] > y[3]);
}


bvec2 greaterThan(uvec2 x, uvec2 y)
{
   return bvec2(x[0] > y[0], x[1] > y[1]);
}


bvec3 greaterThan(uvec3 x, uvec3 y)
{
   return bvec3(x[0] > y[0], x[1] > y[1], x[2] > y[2]);
}


bvec4 greaterThan(uvec4 x, uvec4 y)
{
   return bvec4(x[0] > y[0], x[1] > y[1], x[2] > y[2], x[3] > y[3]);
}


bvec2 greaterThanEqual(vec2 x, vec2 y)
{
   return bvec2(x[0] >= y[0], x[1] >= y[1]);
}


bvec3 greaterThanEqual(vec3 x, vec3 y)
{
   return bvec3(x[0] >= y[0], x[1] >= y[1], x[2] >= y[2]);
}


bvec4 greaterThanEqual(vec4 x, vec4 y)
{
   return bvec4(x[0] >= y[0], x[1] >= y[1], x[2] >= y[2], x[3] >= y[3]);
}


bvec2 greaterThanEqual(ivec2 x, ivec2 y)
{
   return bvec2(x[0] >= y[0], x[1] >= y[1]);
}


bvec3 greaterThanEqual(ivec3 x, ivec3 y)
{
   return bvec3(x[0] >= y[0], x[1] >= y[1], x[2] >= y[2]);
}


bvec4 greaterThanEqual(ivec4 x, ivec4 y)
{
   return bvec4(x[0] >= y[0], x[1] >= y[1], x[2] >= y[2], x[3] >= y[3]);
}


bvec2 greaterThanEqual(uvec2 x, uvec2 y)
{
   return bvec2(x[0] >= y[0], x[1] >= y[1]);
}


bvec3 greaterThanEqual(uvec3 x, uvec3 y)
{
   return bvec3(x[0] >= y[0], x[1] >= y[1], x[2] >= y[2]);
}


bvec4 greaterThanEqual(uvec4 x, uvec4 y)
{
   return bvec4(x[0] >= y[0], x[1] >= y[1], x[2] >= y[2], x[3] >= y[3]);
}


bvec2 equal(vec2 x, vec2 y)
{
   return bvec2(x[0] == y[0], x[1] == y[1]);
}


bvec3 equal(vec3 x, vec3 y)
{
   return bvec3(x[0] == y[0], x[1] == y[1], x[2] == y[2]);
}


bvec4 equal(vec4 x, vec4 y)
{
   return bvec4(x[0] == y[0], x[1] == y[1], x[2] == y[2], x[3] == y[3]);
}


bvec2 equal(ivec2 x, ivec2 y)
{
   return bvec2(x[0] == y[0], x[1] == y[1]);
}


bvec3 equal(ivec3 x, ivec3 y)
{
   return bvec3(x[0] == y[0], x[1] == y[1], x[2] == y[2]);
}


bvec4 equal(ivec4 x, ivec4 y)
{
   return bvec4(x[0] == y[0], x[1] == y[1], x[2] == y[2], x[3] == y[3]);
}


bvec2 equal(uvec2 x, uvec2 y)
{
   return bvec2(x[0] == y[0], x[1] == y[1]);
}


bvec3 equal(uvec3 x, uvec3 y)
{
   return bvec3(x[0] == y[0], x[1] == y[1], x[2] == y[2]);
}


bvec4 equal(uvec4 x, uvec4 y)
{
   return bvec4(x[0] == y[0], x[1] == y[1], x[2] == y[2], x[3] == y[3]);
}


bvec2 equal(bvec2 x, bvec2 y)
{
   return bvec2(x[0] == y[0], x[1] == y[1]);
}


bvec3 equal(bvec3 x, bvec3 y)
{
   return bvec3(x[0] == y[0], x[1] == y[1], x[2] == y[2]);
}


bvec4 equal(bvec4 x, bvec4 y)
{
   return bvec4(x[0] == y[0], x[1] == y[1], x[2] == y[2], x[3] == y[3]);
}


bvec2 notEqual(vec2 x, vec2 y)
{
   return bvec2(x[0] != y[0], x[1] != y[1]);
}


bvec3 notEqual(vec3 x, vec3 y)
{
   return bvec3(x[0] != y[0], x[1] != y[1], x[2] != y[2]);
}


bvec4 notEqual(vec4 x, vec4 y)
{
   return bvec4(x[0] != y[0], x[1] != y[1], x[2] != y[2], x[3] != y[3]);
}


bvec2 notEqual(ivec2 x, ivec2 y)
{
   return bvec2(x[0] != y[0], x[1] != y[1]);
}


bvec3 notEqual(ivec3 x, ivec3 y)
{
   return bvec3(x[0] != y[0], x[1] != y[1], x[2] != y[2]);
}


bvec4 notEqual(ivec4 x, ivec4 y)
{
   return bvec4(x[0] != y[0], x[1] != y[1], x[2] != y[2], x[3] != y[3]);
}


bvec2 notEqual(uvec2 x, uvec2 y)
{
   return bvec2(x[0] != y[0], x[1] != y[1]);
}


bvec3 notEqual(uvec3 x, uvec3 y)
{
   return bvec3(x[0] != y[0], x[1] != y[1], x[2] != y[2]);
}


bvec4 notEqual(uvec4 x, uvec4 y)
{
   return bvec4(x[0] != y[0], x[1] != y[1], x[2] != y[2], x[3] != y[3]);
}


bvec2 notEqual(bvec2 x, bvec2 y)
{
   return bvec2(x[0] != y[0], x[1] != y[1]);
}


bvec3 notEqual(bvec3 x, bvec3 y)
{
   return bvec3(x[0] != y[0], x[1] != y[1], x[2] != y[2]);
}


bvec4 notEqual(bvec4 x, bvec4 y)
{
   return bvec4(x[0] != y[0], x[1] != y[1], x[2] != y[2], x[3] != y[3]);
}


bool any(bvec2 x)
{
   return x[0] || x[1];
}


bool any(bvec3 x)
{
   return x[0] || x[1] || x[2];
}


bool any(bvec4 x)
{
   return x[0] || x[1] || x[2] || x[3];
}


bool all(bvec2 x)
{
   return x[0] && x[1];
}


bool all(bvec3 x)
{
   return x[0] && x[1] && x[2];
}


bool all(bvec4 x)
{
   return x[0] && x[1] && x[2] && x[3];
}


bvec2 not(bvec2 x)
{
   return bvec2(!x[0], !x[1]);
}


bvec3 not(bvec3 x)
{
   return bvec3(!x[0], !x[1], !x[2]);
}


bvec4 not(bvec4 x)
{
   return bvec4(!x[0], !x[1], !x[2], !x[3]);
}
