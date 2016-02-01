# This builds our texture functions

import sys
from generation_utils import print_disclaimer

if __name__ == "__main__":
    v3functions   = []
    fragfunctions = []

    with open("textures.auto.glsl", "w") as outf:
       print_disclaimer(outf, "//")

       for variant in ['','Bias','Fetch','Lod','Grad']:
          for proj in ['','Proj']:
             for offset in ['', 'Offset']:
                for dimx in ['2D','2DX','3D','Cube','2DArray']:
                   dimcount = 2 if dimx == '2D' else 3
                   dim = dimx
                   if dimx == '2DX':
                      dim = '2D'

                   derivcount = 2 if dim in ['2D','2DArray'] else 3
                   if dim == 'Cube' and offset: continue  # No cube offset

                   for g in ['','i','u','s']:
                      if dimx == '2DX' and (not proj or g=='s'): continue   # 2D variant only used with Proj, where we divide by fourth instead of third component
                      if dim == '3D' and g == 's': continue   # No 3D shadow
                      suffix = ''
                      rtype = g + 'vec4'
                      ccount = dimcount
                      stype = 'sampler' + dim
                      cval = 'coord'
                      funcvariant = variant
                      lodarg = ''
                      offsetarg = ''
                      biasarg = ''
                      offsetval = ''
                      bits = 0

                      if proj:
                         if dim in ['Cube','2DArray']: continue   # No Proj for cube or 2d array
                         ccount = ccount + 1
                         cval = '__brcm_proj' + ('2D' if dimx=='2DX' else '') + '(' + cval + ')'

                      if dim == 'Cube':
                         cval = '__brcm_cube' + ('Shadow' if g=='s' else '') + '(' + cval + ')'

                      if g == 's':
                         stype += 'Shadow'
                         rtype = 'float'
                         ccount = ccount + 1
                      else:
                         stype = g + stype

                      ctype = 'vec' + str(ccount)

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
                         bits |= 1<<6
                      elif variant == 'Fetch':
                         if g=='s' or dim=='Cube' or proj: continue    # No fetch with shadow or cube or proj
                         lodarg = ', int lod'
                         lodval = 'lod'
                         ctype = 'ivec' + str(ccount)
                         bits |= 1<<3
                         texture = 'texel'
                      elif variant == 'Grad':
                         lodarg = ', vec' + str(derivcount) + ' dPdx, vec' + str(derivcount) + ' dPdy'
                         if dim=='2DArray':
                            size_swizzle = '.xy'
                         else:
                            size_swizzle = ''
                         if dim == 'Cube':
                             lodval = '__brcm_lod_from_cube_grads(textureSize(sampler, 0), coord.xyz, dPdx, dPdy)'
                         else:
                             lodval = '__brcm_lod_from_grads(textureSize(sampler, 0)' + size_swizzle + ', dPdx, dPdy)'
                         bits |= 1<<6         # We've changed this to a lod, so OR in the lod bits
                      else:
                         assert(0)

                      if offset:
                         offsetarg = ', ivec' + str(derivcount) + ' offset'
                         offsetval = ', offset'

                      # Shadow variants which are ommitted for no obvious reason
                      if g == 's':
                         if variant == 'Lod' and dim == 'Cube': continue
                         if dim == '2DArray':
                            if variant == '' and offset: continue
                            if variant == 'Bias': continue
                            if variant == 'Lod': continue

                      funcname  = texture + proj + funcvariant + offset
                      coordname = "coord" + lodarg + offsetarg + biasarg

                      print >>outf, "%s %s(%s sampler, %s %s) {" % (rtype, funcname, stype, ctype, coordname)
                      print >>outf, "   return $$texture(%s, sampler, %s, %s%s);" % (bits, cval, lodval, offsetval)
                      print >>outf, "}"
                      print >>outf

                      proto = "  %s %s(%s sampler, %s %s);" % (rtype, funcname, stype, ctype, coordname)
                      v3functions.append(proto)
                      if len(biasarg) > 0:
                          fragfunctions.append(proto)

    with open("textures.auto.props","w") as prpf:
       print_disclaimer(prpf, "#")
       print >>prpf
       print >>prpf, "version3-only:"
       for v3fun in v3functions:
           print >>prpf, "  %s" % v3fun
       print >>prpf, "fragment-only:"
       for fragfun in fragfunctions:
           print >>prpf, "  %s" % fragfun
