from __future__ import print_function
# This builds our texture functions

import sys
from generation_utils import *

def make_type(base, count):
    if count == 1:
        if   base == '':  return 'float'
        elif base == 'i': return 'int'
        elif base == 'u': return 'uint'
    else:
        return base + 'vec' + str(count)

if __name__ == "__main__":
    output_dir, include_dirs, input_files = parse_opts()

    v3functions   = []
    v32functions  = []
    tex_1d_functions     = []
    cube_array_functions = []
    fragfunctions = []
    arg3offset    = []
    arg4offset    = []
    arg5offset    = []

    with open(os.path.join(output_dir,"textures.auto.glsl"), "w") as outf:
       print_disclaimer(outf, "//")

       for variant in ['','Bias','Fetch','Lod','Grad']:
          for proj in ['','Proj']:
             for offset in ['', 'Offset']:
                for dimx in ['1D', '1DX', '1DArray', '2D','2DX','3D','Cube','2DArray','CubeArray']:
                   if dimx == '1D':
                       dimcount = 1
                   elif dimx in ['2D', '1DArray']:
                       dimcount = 2
                   elif dimx == 'CubeArray':
                       dimcount = 4
                   else:
                       dimcount = 3

                   dim = dimx
                   if dimx == '2DX':
                      dim = '2D'
                   if dimx == '1DX':
                       dim = '1D'

                   if dim in ['1D', '1DArray']:
                       derivcount = 1
                   elif dim in ['2D','2DArray']:
                       derivcount = 2
                   else:
                       derivcount = 3

                   if (dim == 'Cube' or dim == 'CubeArray') and offset: continue  # No cube offset

                   for g in ['','i','u','s']:
                      if dimx in ['1DX', '2DX'] and (not proj or g=='s'): continue   # X variants only used with Proj, where we divide by fourth instead of third component
                      if dim in ['1D', '1DArray']  and variant == 'Grad': continue # TODO: 1D Grad not supported because textureSize is required
                      if dim in ['1D', '1DArray']  and g == 's': continue   # TODO: 1D Shadow not supported
                      if dim == '3D' and g == 's': continue   # No 3D shadow
                      if proj and dim in ['Cube','2DArray','CubeArray']: continue   # No Proj for cube or array

                      # Shadow variants which are ommitted for no obvious reason
                      if g == 's':
                         if variant == 'Lod' and (dim == 'Cube' or dim == 'CubeArray'): continue
                         if dim == 'CubeArray':
                             if variant == 'Bias' or variant == 'Grad': continue
                         if dim == '2DArray':
                            if variant == '' and offset: continue
                            if variant == 'Bias': continue
                            if variant == 'Lod': continue

                      suffix = ''
                      rtype = g + 'vec4'
                      ccount = dimcount
                      stype = 'sampler' + dim
                      cval = 'coord'
                      funcvariant = variant
                      drefarg = ''
                      lodarg = ''
                      offsetarg = ''
                      biasarg = ''
                      offsetval = ''
                      drefval = ''
                      bits = 0

                      if proj:
                         ccount = ccount + 1
                         cval = '__brcm_proj' + ('2D' if dimx=='2DX' else '') + ('1D' if dimx=='1DX' else '') + '(' + cval + ')'

                      if g == 's':
                         stype += 'Shadow'
                         rtype = 'float'
                         if dim != 'CubeArray':
                             swizzle = [ 'x', 'y', 'z', 'w' ]
                             remainder_swizzle = [ '??', 'x', 'xy', 'xyz' ]
                             drefval = ", %s.%s" % ( cval, swizzle[dimcount] )
                             cval = "%s.%s" % (cval, remainder_swizzle[dimcount])
                             ccount = ccount + 1
                         else:
                             drefarg = ', float dref'
                             drefval = ', dref'
                      else:
                         stype = g + stype

                      if dim == 'Cube' or dim == 'CubeArray':
                         cval = '__brcm_cube' + '(' + cval + ')'

                      ctype = make_type('', ccount)

                      texture = 'texture'
                      if variant == '':
                         lodval = '0.0'
                      elif variant == 'Bias':
                         funcvariant = ''    # Distinguished merely by whether "bias" param exists
                         biasarg = ', float bias'
                         lodval = 'bias'
                      elif variant == 'Lod':
                         lodarg = ', float lod'
                         lodval = 'lod'
                         bits |= 1<<3
                      elif variant == 'Fetch':
                         if g=='s' or dim=='Cube' or dim == 'CubeArray' or proj: continue    # No fetch with shadow or cube or proj
                         lodarg = ', int lod'
                         lodval = 'lod'
                         ctype = make_type('i', ccount)
                         bits |= 1<<1
                         texture = 'texel'
                      elif variant == 'Grad':
                         lodarg = ', ' + make_type('', derivcount) + ' dPdx, ' + make_type('', derivcount) + ' dPdy'
                         if dim=='2DArray' or dim == 'CubeArray':
                            size_swizzle = '.xy'
                         else:
                            size_swizzle = ''
                         if dim == 'Cube' or dim == 'CubeArray':
                             lodval = '__brcm_lod_from_cube_grads($$textureSize(sampler)' + size_swizzle + ', coord.xyz, dPdx, dPdy)'
                         else:
                             lodval = '__brcm_lod_from_grads($$textureSize(sampler)' + size_swizzle + ', dPdx, dPdy)'
                         bits |= 1<<3         # We've changed this to a lod, so OR in the lod bits
                      else:
                         assert(0)

                      if offset:
                         offsetarg = ', ' + make_type('i', derivcount) + ' offset'
                         offsetval = ', offset'

                      funcname  = texture + proj + funcvariant + offset
                      coordname = "coord" + lodarg + offsetarg + biasarg + drefarg

                      print("%s %s(%s sampler, %s %s) {" % (rtype, funcname, stype, ctype, coordname), file=outf)
                      print("   return $$texture(%s, sampler, %s, %s%s%s);" % (bits, cval, lodval, drefval, offsetval), file=outf)
                      print("}", file=outf)
                      print(file=outf)

                      proto = "  %s %s(%s sampler, %s %s);" % (rtype, funcname, stype, ctype, coordname)
                      if (dim in ['1D', '1DArray']):
                          tex_1d_functions.append(proto)
                      elif (dim == 'CubeArray'):
                          v32functions.append(proto)
                          cube_array_functions.append(proto)
                      else:
                          v3functions.append(proto)
                      if len(biasarg) > 0:
                          fragfunctions.append(proto)
                      if len(offsetarg) > 0:
                          if len(lodarg) == 0:
                             arg3offset.append(proto)
                          elif variant != 'Grad':
                             arg4offset.append(proto)
                          else:
                             arg5offset.append(proto)

    with open(os.path.join(output_dir,"textures.auto.props"),"w") as prpf:
       print_disclaimer(prpf, "#")
       print(file=prpf)
       print("version3 version31 version32:", file=prpf)
       for f in v3functions:
           print("  %s" % f, file=prpf)
       print("version32:", file=prpf)
       for f in v32functions:
           print("  %s" % f, file=prpf)
       print("GL_BRCM_texture_1d:", file=prpf)
       for f in tex_1d_functions:
           print(" %s" % f, file=prpf)
       print("GL_OES_texture_cube_map_array:", file=prpf)
       for f in cube_array_functions:
           print(" %s" % f, file=prpf)
       print("fragment:", file=prpf)
       for f in fragfunctions:
           print("  %s" % f, file=prpf)
       print("arg3-const:", file=prpf)
       for f in arg3offset:
           print("  %s" % f, file=prpf)
       print("arg4-const:", file=prpf)
       for f in arg4offset:
           print("  %s" % f, file=prpf)
       print("arg5-const:", file=prpf)
       for f in arg5offset:
           print("  %s" % f, file=prpf)
