highp ivec2 imageSize(readonly writeonly  image2D image) { return $$imageSize(image); }
highp ivec2 imageSize(readonly writeonly iimage2D image) { return $$imageSize(image); }
highp ivec2 imageSize(readonly writeonly uimage2D image) { return $$imageSize(image); }
highp ivec3 imageSize(readonly writeonly  image3D image) { return $$imageSize(image); }
highp ivec3 imageSize(readonly writeonly iimage3D image) { return $$imageSize(image); }
highp ivec3 imageSize(readonly writeonly uimage3D image) { return $$imageSize(image); }
highp ivec2 imageSize(readonly writeonly  imageCube image) { return $$imageSize(image); }
highp ivec2 imageSize(readonly writeonly iimageCube image) { return $$imageSize(image); }
highp ivec2 imageSize(readonly writeonly uimageCube image) { return $$imageSize(image); }
highp ivec3 imageSize(readonly writeonly  image2DArray image) { return $$imageSize(image); }
highp ivec3 imageSize(readonly writeonly iimage2DArray image) { return $$imageSize(image); }
highp ivec3 imageSize(readonly writeonly uimage2DArray image) { return $$imageSize(image); }
highp ivec3 imageSize(readonly writeonly  imageCubeArray image) { return $$imageSize(image); }
highp ivec3 imageSize(readonly writeonly iimageCubeArray image) { return $$imageSize(image); }
highp ivec3 imageSize(readonly writeonly uimageCubeArray image) { return $$imageSize(image); }

highp  vec4 imageLoad(readonly  image2D image, ivec2 coord) { return $$texture(2, image, coord, 0); }
highp ivec4 imageLoad(readonly iimage2D image, ivec2 coord) { return $$texture(2, image, coord, 0); }
highp uvec4 imageLoad(readonly uimage2D image, ivec2 coord) { return $$texture(2, image, coord, 0); }
highp  vec4 imageLoad(readonly  image3D image, ivec3 coord) { return $$texture(2, image, coord, 0); }
highp ivec4 imageLoad(readonly iimage3D image, ivec3 coord) { return $$texture(2, image, coord, 0); }
highp uvec4 imageLoad(readonly uimage3D image, ivec3 coord) { return $$texture(2, image, coord, 0); }

highp  vec4 imageLoad(readonly  imageCube image, ivec3 coord)
{
   return (uint(coord[2]) >= 6u) ? vec4(0) : $$texture(2, image, coord, 0);
}
highp ivec4 imageLoad(readonly iimageCube image, ivec3 coord)
{
   return (uint(coord[2]) >= 6u) ? ivec4(0) : $$texture(2, image, coord, 0);
}
highp uvec4 imageLoad(readonly uimageCube image, ivec3 coord)
{
   return (uint(coord[2]) >= 6u) ? uvec4(0) : $$texture(2, image, coord, 0);
}

highp  vec4 imageLoad(readonly  imageCubeArray image, ivec3 coord)
{
   uint array_size = uint(imageSize(image).z);
   return (uint(coord[2]) >= array_size) ? vec4(0) : $$texture(2, image, coord, 0);
}
highp ivec4 imageLoad(readonly iimageCubeArray image, ivec3 coord)
{
   uint array_size = uint(imageSize(image).z);
   return (uint(coord[2]) >= array_size) ? ivec4(0) : $$texture(2, image, coord, 0);
}
highp uvec4 imageLoad(readonly uimageCubeArray image, ivec3 coord)
{
   uint array_size = uint(imageSize(image).z);
   return (uint(coord[2]) >= array_size) ? uvec4(0) : $$texture(2, image, coord, 0);
}

highp  vec4 imageLoad(readonly  image2DArray image, ivec3 coord)
{
   uint array_size = uint(imageSize(image).z);
   return (uint(coord[2]) >= array_size) ? vec4(0) : $$texture(2, image, coord, 0);
}
highp ivec4 imageLoad(readonly iimage2DArray image, ivec3 coord)
{
   uint array_size = uint(imageSize(image).z);
   return (uint(coord[2]) >= array_size) ? ivec4(0) : $$texture(2, image, coord, 0);
}
highp uvec4 imageLoad(readonly uimage2DArray image, ivec3 coord)
{
   uint array_size = uint(imageSize(image).z);
   return (uint(coord[2]) >= array_size) ? uvec4(0) : $$texture(2, image, coord, 0);
}

highp void imageStore(writeonly  image2D image, ivec2 coord,  vec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly iimage2D image, ivec2 coord, ivec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly uimage2D image, ivec2 coord, uvec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly  image3D image, ivec3 coord,  vec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly iimage3D image, ivec3 coord, ivec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly uimage3D image, ivec3 coord, uvec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly  imageCube    image, ivec3 coord,  vec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly iimageCube    image, ivec3 coord, ivec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly uimageCube    image, ivec3 coord, uvec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly  image2DArray image, ivec3 coord,  vec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly iimage2DArray image, ivec3 coord, ivec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly uimage2DArray image, ivec3 coord, uvec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly  imageCubeArray image, ivec3 coord,  vec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly iimageCubeArray image, ivec3 coord, ivec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly uimageCubeArray image, ivec3 coord, uvec4 data) { $$imageStore(image, coord, data); }
