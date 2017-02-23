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
highp int imageSize(readonly writeonly  imageBuffer  image) { return $$imageSize(image); }
highp int imageSize(readonly writeonly iimageBuffer  image) { return $$imageSize(image); }
highp int imageSize(readonly writeonly uimageBuffer  image) { return $$imageSize(image); }
highp ivec3 imageSize(readonly writeonly  imageCubeArray image) { return $$imageSize(image); }
highp ivec3 imageSize(readonly writeonly iimageCubeArray image) { return $$imageSize(image); }
highp ivec3 imageSize(readonly writeonly uimageCubeArray image) { return $$imageSize(image); }

highp  vec4 imageLoad(readonly  image2D image, ivec2 coord) { return $$texture(2, image, coord, 0); }
highp ivec4 imageLoad(readonly iimage2D image, ivec2 coord) { return $$texture(2, image, coord, 0); }
highp uvec4 imageLoad(readonly uimage2D image, ivec2 coord) { return $$texture(2, image, coord, 0); }
highp  vec4 imageLoad(readonly  image3D image, ivec3 coord) { return $$texture(2, image, coord, 0); }
highp ivec4 imageLoad(readonly iimage3D image, ivec3 coord) { return $$texture(2, image, coord, 0); }
highp uvec4 imageLoad(readonly uimage3D image, ivec3 coord) { return $$texture(2, image, coord, 0); }
highp  vec4 imageLoad(readonly  imageCube image, ivec3 coord) { return $$texture(2, image, coord, 0); }
highp ivec4 imageLoad(readonly iimageCube image, ivec3 coord) { return $$texture(2, image, coord, 0); }
highp uvec4 imageLoad(readonly uimageCube image, ivec3 coord) { return $$texture(2, image, coord, 0); }
highp  vec4 imageLoad(readonly  imageCubeArray image, ivec3 coord) { return $$texture(2, image, coord, 0); }
highp ivec4 imageLoad(readonly iimageCubeArray image, ivec3 coord) { return $$texture(2, image, coord, 0); }
highp uvec4 imageLoad(readonly uimageCubeArray image, ivec3 coord) { return $$texture(2, image, coord, 0); }
highp  vec4 imageLoad(readonly  image2DArray image, ivec3 coord) { return $$texture(2, image, coord, 0); }
highp ivec4 imageLoad(readonly iimage2DArray image, ivec3 coord) { return $$texture(2, image, coord, 0); }
highp uvec4 imageLoad(readonly uimage2DArray image, ivec3 coord) { return $$texture(2, image, coord, 0); }
highp  vec4 imageLoad(readonly  imageBuffer image, int coord) { return $$texture(2, image, coord, 0); }
highp ivec4 imageLoad(readonly iimageBuffer image, int coord) { return $$texture(2, image, coord, 0); }
highp uvec4 imageLoad(readonly uimageBuffer image, int coord) { return $$texture(2, image, coord, 0); }

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

highp void imageStore(writeonly  imageBuffer  image, int   coord,  vec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly iimageBuffer  image, int   coord, ivec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly uimageBuffer  image, int   coord, uvec4 data) { $$imageStore(image, coord, data); }

highp void imageStore(writeonly  imageCubeArray image, ivec3 coord,  vec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly iimageCubeArray image, ivec3 coord, ivec4 data) { $$imageStore(image, coord, data); }
highp void imageStore(writeonly uimageCubeArray image, ivec3 coord, uvec4 data) { $$imageStore(image, coord, data); }

highp  int imageAtomicAdd(iimage2D        image, ivec2 coord,  int data) { return $$imageAtomicAdd(image, coord, data); }
highp uint imageAtomicAdd(uimage2D        image, ivec2 coord, uint data) { return $$imageAtomicAdd(image, coord, data); }
highp  int imageAtomicAdd(iimage3D        image, ivec3 coord,  int data) { return $$imageAtomicAdd(image, coord, data); }
highp uint imageAtomicAdd(uimage3D        image, ivec3 coord, uint data) { return $$imageAtomicAdd(image, coord, data); }
highp  int imageAtomicAdd(iimageCube      image, ivec3 coord,  int data) { return $$imageAtomicAdd(image, coord, data); }
highp uint imageAtomicAdd(uimageCube      image, ivec3 coord, uint data) { return $$imageAtomicAdd(image, coord, data); }
highp  int imageAtomicAdd(iimage2DArray   image, ivec3 coord,  int data) { return $$imageAtomicAdd(image, coord, data); }
highp uint imageAtomicAdd(uimage2DArray   image, ivec3 coord, uint data) { return $$imageAtomicAdd(image, coord, data); }
highp  int imageAtomicAdd(iimageCubeArray image, ivec3 coord,  int data) { return $$imageAtomicAdd(image, coord, data); }
highp uint imageAtomicAdd(uimageCubeArray image, ivec3 coord, uint data) { return $$imageAtomicAdd(image, coord, data); }
highp  int imageAtomicAdd(iimageBuffer    image, int   coord,  int data) { return $$imageAtomicAdd(image, coord, data); }
highp uint imageAtomicAdd(uimageBuffer    image, int   coord, uint data) { return $$imageAtomicAdd(image, coord, data); }

highp  int imageAtomicMin(iimage2D        image, ivec2 coord,  int data) { return $$imageAtomicMin(image, coord, data); }
highp uint imageAtomicMin(uimage2D        image, ivec2 coord, uint data) { return $$imageAtomicMin(image, coord, data); }
highp  int imageAtomicMin(iimage3D        image, ivec3 coord,  int data) { return $$imageAtomicMin(image, coord, data); }
highp uint imageAtomicMin(uimage3D        image, ivec3 coord, uint data) { return $$imageAtomicMin(image, coord, data); }
highp  int imageAtomicMin(iimageCube      image, ivec3 coord,  int data) { return $$imageAtomicMin(image, coord, data); }
highp uint imageAtomicMin(uimageCube      image, ivec3 coord, uint data) { return $$imageAtomicMin(image, coord, data); }
highp  int imageAtomicMin(iimage2DArray   image, ivec3 coord,  int data) { return $$imageAtomicMin(image, coord, data); }
highp uint imageAtomicMin(uimage2DArray   image, ivec3 coord, uint data) { return $$imageAtomicMin(image, coord, data); }
highp  int imageAtomicMin(iimageCubeArray image, ivec3 coord,  int data) { return $$imageAtomicMin(image, coord, data); }
highp uint imageAtomicMin(uimageCubeArray image, ivec3 coord, uint data) { return $$imageAtomicMin(image, coord, data); }
highp  int imageAtomicMin(iimageBuffer    image, int   coord,  int data) { return $$imageAtomicMin(image, coord, data); }
highp uint imageAtomicMin(uimageBuffer    image, int   coord, uint data) { return $$imageAtomicMin(image, coord, data); }

highp  int imageAtomicMax(iimage2D        image, ivec2 coord,  int data) { return $$imageAtomicMax(image, coord, data); }
highp uint imageAtomicMax(uimage2D        image, ivec2 coord, uint data) { return $$imageAtomicMax(image, coord, data); }
highp  int imageAtomicMax(iimage3D        image, ivec3 coord,  int data) { return $$imageAtomicMax(image, coord, data); }
highp uint imageAtomicMax(uimage3D        image, ivec3 coord, uint data) { return $$imageAtomicMax(image, coord, data); }
highp  int imageAtomicMax(iimageCube      image, ivec3 coord,  int data) { return $$imageAtomicMax(image, coord, data); }
highp uint imageAtomicMax(uimageCube      image, ivec3 coord, uint data) { return $$imageAtomicMax(image, coord, data); }
highp  int imageAtomicMax(iimage2DArray   image, ivec3 coord,  int data) { return $$imageAtomicMax(image, coord, data); }
highp uint imageAtomicMax(uimage2DArray   image, ivec3 coord, uint data) { return $$imageAtomicMax(image, coord, data); }
highp  int imageAtomicMax(iimageCubeArray image, ivec3 coord,  int data) { return $$imageAtomicMax(image, coord, data); }
highp uint imageAtomicMax(uimageCubeArray image, ivec3 coord, uint data) { return $$imageAtomicMax(image, coord, data); }
highp  int imageAtomicMax(iimageBuffer    image, int   coord,  int data) { return $$imageAtomicMax(image, coord, data); }
highp uint imageAtomicMax(uimageBuffer   image, int   coord, uint data) { return $$imageAtomicMax(image, coord, data); }

highp  int imageAtomicAnd(iimage2D        image, ivec2 coord,  int data) { return $$imageAtomicAnd(image, coord, data); }
highp uint imageAtomicAnd(uimage2D        image, ivec2 coord, uint data) { return $$imageAtomicAnd(image, coord, data); }
highp  int imageAtomicAnd(iimage3D        image, ivec3 coord,  int data) { return $$imageAtomicAnd(image, coord, data); }
highp uint imageAtomicAnd(uimage3D        image, ivec3 coord, uint data) { return $$imageAtomicAnd(image, coord, data); }
highp  int imageAtomicAnd(iimageCube      image, ivec3 coord,  int data) { return $$imageAtomicAnd(image, coord, data); }
highp uint imageAtomicAnd(uimageCube      image, ivec3 coord, uint data) { return $$imageAtomicAnd(image, coord, data); }
highp  int imageAtomicAnd(iimage2DArray   image, ivec3 coord,  int data) { return $$imageAtomicAnd(image, coord, data); }
highp uint imageAtomicAnd(uimage2DArray   image, ivec3 coord, uint data) { return $$imageAtomicAnd(image, coord, data); }
highp  int imageAtomicAnd(iimageCubeArray image, ivec3 coord,  int data) { return $$imageAtomicAnd(image, coord, data); }
highp uint imageAtomicAnd(uimageCubeArray image, ivec3 coord, uint data) { return $$imageAtomicAnd(image, coord, data); }
highp  int imageAtomicAnd(iimageBuffer    image, int   coord,  int data) { return $$imageAtomicAnd(image, coord, data); }
highp uint imageAtomicAnd(uimageBuffer    image, int   coord, uint data) { return $$imageAtomicAnd(image, coord, data); }

highp  int imageAtomicOr(iimage2D        image, ivec2 coord,  int data) { return $$imageAtomicOr(image, coord, data); }
highp uint imageAtomicOr(uimage2D        image, ivec2 coord, uint data) { return $$imageAtomicOr(image, coord, data); }
highp  int imageAtomicOr(iimage3D        image, ivec3 coord,  int data) { return $$imageAtomicOr(image, coord, data); }
highp uint imageAtomicOr(uimage3D        image, ivec3 coord, uint data) { return $$imageAtomicOr(image, coord, data); }
highp  int imageAtomicOr(iimageCube      image, ivec3 coord,  int data) { return $$imageAtomicOr(image, coord, data); }
highp uint imageAtomicOr(uimageCube      image, ivec3 coord, uint data) { return $$imageAtomicOr(image, coord, data); }
highp  int imageAtomicOr(iimage2DArray   image, ivec3 coord,  int data) { return $$imageAtomicOr(image, coord, data); }
highp uint imageAtomicOr(uimage2DArray   image, ivec3 coord, uint data) { return $$imageAtomicOr(image, coord, data); }
highp  int imageAtomicOr(iimageCubeArray image, ivec3 coord,  int data) { return $$imageAtomicOr(image, coord, data); }
highp uint imageAtomicOr(uimageCubeArray image, ivec3 coord, uint data) { return $$imageAtomicOr(image, coord, data); }
highp  int imageAtomicOr(iimageBuffer    image, int   coord,  int data) { return $$imageAtomicOr(image, coord, data); }
highp uint imageAtomicOr(uimageBuffer    image, int   coord, uint data) { return $$imageAtomicOr(image, coord, data); }

highp  int imageAtomicXor(iimage2D        image, ivec2 coord,  int data) { return $$imageAtomicXor(image, coord, data); }
highp uint imageAtomicXor(uimage2D        image, ivec2 coord, uint data) { return $$imageAtomicXor(image, coord, data); }
highp  int imageAtomicXor(iimage3D        image, ivec3 coord,  int data) { return $$imageAtomicXor(image, coord, data); }
highp uint imageAtomicXor(uimage3D        image, ivec3 coord, uint data) { return $$imageAtomicXor(image, coord, data); }
highp  int imageAtomicXor(iimageCube      image, ivec3 coord,  int data) { return $$imageAtomicXor(image, coord, data); }
highp uint imageAtomicXor(uimageCube      image, ivec3 coord, uint data) { return $$imageAtomicXor(image, coord, data); }
highp  int imageAtomicXor(iimage2DArray   image, ivec3 coord,  int data) { return $$imageAtomicXor(image, coord, data); }
highp uint imageAtomicXor(uimage2DArray   image, ivec3 coord, uint data) { return $$imageAtomicXor(image, coord, data); }
highp  int imageAtomicXor(iimageCubeArray image, ivec3 coord,  int data) { return $$imageAtomicXor(image, coord, data); }
highp uint imageAtomicXor(uimageCubeArray image, ivec3 coord, uint data) { return $$imageAtomicXor(image, coord, data); }
highp  int imageAtomicXor(iimageBuffer    image, int   coord,  int data) { return $$imageAtomicXor(image, coord, data); }
highp uint imageAtomicXor(uimageBuffer    image, int   coord, uint data) { return $$imageAtomicXor(image, coord, data); }

highp float imageAtomicExchange( image2D        image, ivec2 coord, float data) { return $$imageAtomicXchg(image, coord, data); }
highp   int imageAtomicExchange(iimage2D        image, ivec2 coord,   int data) { return $$imageAtomicXchg(image, coord, data); }
highp  uint imageAtomicExchange(uimage2D        image, ivec2 coord,  uint data) { return $$imageAtomicXchg(image, coord, data); }
highp float imageAtomicExchange( image3D        image, ivec3 coord, float data) { return $$imageAtomicXchg(image, coord, data); }
highp   int imageAtomicExchange(iimage3D        image, ivec3 coord,   int data) { return $$imageAtomicXchg(image, coord, data); }
highp  uint imageAtomicExchange(uimage3D        image, ivec3 coord,  uint data) { return $$imageAtomicXchg(image, coord, data); }
highp float imageAtomicExchange( imageCube      image, ivec3 coord, float data) { return $$imageAtomicXchg(image, coord, data); }
highp   int imageAtomicExchange(iimageCube      image, ivec3 coord,   int data) { return $$imageAtomicXchg(image, coord, data); }
highp  uint imageAtomicExchange(uimageCube      image, ivec3 coord,  uint data) { return $$imageAtomicXchg(image, coord, data); }
highp float imageAtomicExchange( image2DArray   image, ivec3 coord, float data) { return $$imageAtomicXchg(image, coord, data); }
highp   int imageAtomicExchange(iimage2DArray   image, ivec3 coord,   int data) { return $$imageAtomicXchg(image, coord, data); }
highp  uint imageAtomicExchange(uimage2DArray   image, ivec3 coord,  uint data) { return $$imageAtomicXchg(image, coord, data); }
highp float imageAtomicExchange( imageCubeArray image, ivec3 coord, float data) { return $$imageAtomicXchg(image, coord, data); }
highp   int imageAtomicExchange(iimageCubeArray image, ivec3 coord,   int data) { return $$imageAtomicXchg(image, coord, data); }
highp  uint imageAtomicExchange(uimageCubeArray image, ivec3 coord,  uint data) { return $$imageAtomicXchg(image, coord, data); }
highp float imageAtomicExchange( imageBuffer    image, int   coord, float data) { return $$imageAtomicXchg(image, coord, data); }
highp   int imageAtomicExchange(iimageBuffer    image, int   coord,   int data) { return $$imageAtomicXchg(image, coord, data); }
highp  uint imageAtomicExchange(uimageBuffer    image, int   coord,  uint data) { return $$imageAtomicXchg(image, coord, data); }

highp  int imageAtomicCompSwap(iimage2D        image, ivec2 coord,  int compare,  int data) { return $$imageAtomicCmpxchg(image, coord, compare, data); }
highp uint imageAtomicCompSwap(uimage2D        image, ivec2 coord, uint compare, uint data) { return $$imageAtomicCmpxchg(image, coord, compare, data); }
highp  int imageAtomicCompSwap(iimage3D        image, ivec3 coord,  int compare,  int data) { return $$imageAtomicCmpxchg(image, coord, compare, data); }
highp uint imageAtomicCompSwap(uimage3D        image, ivec3 coord, uint compare, uint data) { return $$imageAtomicCmpxchg(image, coord, compare, data); }
highp  int imageAtomicCompSwap(iimageCube      image, ivec3 coord,  int compare,  int data) { return $$imageAtomicCmpxchg(image, coord, compare, data); }
highp uint imageAtomicCompSwap(uimageCube      image, ivec3 coord, uint compare, uint data) { return $$imageAtomicCmpxchg(image, coord, compare, data); }
highp  int imageAtomicCompSwap(iimage2DArray   image, ivec3 coord,  int compare,  int data) { return $$imageAtomicCmpxchg(image, coord, compare, data); }
highp uint imageAtomicCompSwap(uimage2DArray   image, ivec3 coord, uint compare, uint data) { return $$imageAtomicCmpxchg(image, coord, compare, data); }
highp  int imageAtomicCompSwap(iimageCubeArray image, ivec3 coord,  int compare,  int data) { return $$imageAtomicCmpxchg(image, coord, compare, data); }
highp uint imageAtomicCompSwap(uimageCubeArray image, ivec3 coord, uint compare, uint data) { return $$imageAtomicCmpxchg(image, coord, compare, data); }
highp  int imageAtomicCompSwap(iimageBuffer    image, int   coord,  int compare,  int data) { return $$imageAtomicCmpxchg(image, coord, compare, data); }
highp uint imageAtomicCompSwap(uimageBuffer    image, int   coord, uint compare, uint data) { return $$imageAtomicCmpxchg(image, coord, compare, data); }
