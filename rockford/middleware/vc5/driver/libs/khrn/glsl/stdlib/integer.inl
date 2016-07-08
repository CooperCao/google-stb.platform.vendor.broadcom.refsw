static void bitfieldExtract__const_int__const_int__const_int__const_int(const_int *result,
                                                                        const_int *value,
                                                                        const_int *offset,
                                                                        const_int *bits)
{
   const_signed i;

   if (*bits == 0) *result = 0;
   else {
      i = *value << (32 - (*offset + *bits));
      *result = i >> (32 - *bits);
   }
}

static void bitfieldExtract__const_ivec2__const_ivec2__const_int__const_int(const_ivec2 *result,
                                                                            const_ivec2 *value,
                                                                            const_int   *offset,
                                                                            const_int   *bits)
{
   for (int i=0; i<2; i++)
      bitfieldExtract__const_int__const_int__const_int__const_int(&(*result)[i], &(*value)[i], offset, bits);
}

static void bitfieldExtract__const_ivec3__const_ivec3__const_int__const_int(const_ivec3 *result,
                                                                            const_ivec3 *value,
                                                                            const_int   *offset,
                                                                            const_int   *bits)
{
   for (int i=0; i<3; i++)
      bitfieldExtract__const_int__const_int__const_int__const_int(&(*result)[i], &(*value)[i], offset, bits);
}

static void bitfieldExtract__const_ivec4__const_ivec4__const_int__const_int(const_ivec4 *result,
                                                                            const_ivec4 *value,
                                                                            const_int   *offset,
                                                                            const_int   *bits)
{
   for (int i=0; i<4; i++)
      bitfieldExtract__const_int__const_int__const_int__const_int(&(*result)[i], &(*value)[i], offset, bits);
}

static void bitfieldExtract__const_uint__const_uint__const_int__const_int(const_uint *result,
                                                                          const_uint *value,
                                                                          const_int  *offset,
                                                                          const_int  *bits)
{
   if (*bits == 0) *result = 0;
   else *result = (*value << (32 - (*offset + *bits))) >> (32 - *bits);
}

static void bitfieldExtract__const_uvec2__const_uvec2__const_int__const_int(const_uvec2 *result,
                                                                            const_uvec2 *value,
                                                                            const_int   *offset,
                                                                            const_int   *bits)
{
   for (int i=0; i<2; i++)
      bitfieldExtract__const_uint__const_uint__const_int__const_int(&(*result)[i], &(*value)[i], offset, bits);
}

static void bitfieldExtract__const_uvec3__const_uvec3__const_int__const_int(const_uvec3 *result,
                                                                            const_uvec3 *value,
                                                                            const_int   *offset,
                                                                            const_int   *bits)
{
   for (int i=0; i<3; i++)
      bitfieldExtract__const_uint__const_uint__const_int__const_int(&(*result)[i], &(*value)[i], offset, bits);
}

static void bitfieldExtract__const_uvec4__const_uvec4__const_int__const_int(const_uvec4 *result,
                                                                            const_uvec4 *value,
                                                                            const_int   *offset,
                                                                            const_int   *bits)
{
   for (int i=0; i<4; i++)
      bitfieldExtract__const_uint__const_uint__const_int__const_int(&(*result)[i], &(*value)[i], offset, bits);
}

static void bitfieldInsert__const_int__const_int__const_int__const_int__const_int(const_int *result,
                                                                                  const_int *base,
                                                                                  const_int *insert,
                                                                                  const_int *offset,
                                                                                  const_int *bits)
{
   if (*bits == 0) *result = *base;
   else {
      const_int mask = (0xFFFFFFFFu >> (32 - *bits)) << *offset;
      *result = ((*insert << *offset) & mask) | (*base & ~mask);
   }
}

static void bitfieldInsert__const_ivec2__const_ivec2__const_ivec2__const_int__const_int(const_ivec2 *result,
                                                                                        const_ivec2 *base,
                                                                                        const_ivec2 *insert,
                                                                                        const_int   *offset,
                                                                                        const_int   *bits)
{
   for (int i=0; i<2; i++)
      bitfieldInsert__const_int__const_int__const_int__const_int__const_int(&(*result)[i], &(*base)[i],
                                                                            &(*insert)[i], offset, bits);
}

static void bitfieldInsert__const_ivec3__const_ivec3__const_ivec3__const_int__const_int(const_ivec3 *result,
                                                                                        const_ivec3 *base,
                                                                                        const_ivec3 *insert,
                                                                                        const_int   *offset,
                                                                                        const_int   *bits)
{
   for (int i=0; i<3; i++)
      bitfieldInsert__const_int__const_int__const_int__const_int__const_int(&(*result)[i], &(*base)[i],
                                                                            &(*insert)[i], offset, bits);
}

static void bitfieldInsert__const_ivec4__const_ivec4__const_ivec4__const_int__const_int(const_ivec4 *result,
                                                                                        const_ivec4 *base,
                                                                                        const_ivec4 *insert,
                                                                                        const_int   *offset,
                                                                                        const_int   *bits)
{
   for (int i=0; i<4; i++)
      bitfieldInsert__const_int__const_int__const_int__const_int__const_int(&(*result)[i], &(*base)[i],
                                                                            &(*insert)[i], offset, bits);
}

static void bitfieldInsert__const_uint__const_uint__const_uint__const_int__const_int(const_uint *result,
                                                                                     const_uint *base,
                                                                                     const_uint *insert,
                                                                                     const_int  *offset,
                                                                                     const_int  *bits)
{
   bitfieldInsert__const_int__const_int__const_int__const_int__const_int(result, base, insert,
                                                                         offset, bits);
}

static void bitfieldInsert__const_uvec2__const_uvec2__const_uvec2__const_int__const_int(const_uvec2 *result,
                                                                                        const_uvec2 *base,
                                                                                        const_uvec2 *insert,
                                                                                        const_int   *offset,
                                                                                        const_int   *bits)
{
   for (int i=0; i<2; i++)
      bitfieldInsert__const_uint__const_uint__const_uint__const_int__const_int(&(*result)[i], &(*base)[i],
                                                                               &(*insert)[i], offset, bits);
}

static void bitfieldInsert__const_uvec3__const_uvec3__const_uvec3__const_int__const_int(const_uvec3 *result,
                                                                                        const_uvec3 *base,
                                                                                        const_uvec3 *insert,
                                                                                        const_int   *offset,
                                                                                        const_int   *bits)
{
   for (int i=0; i<3; i++)
      bitfieldInsert__const_uint__const_uint__const_uint__const_int__const_int(&(*result)[i], &(*base)[i],
                                                                               &(*insert)[i], offset, bits);
}

static void bitfieldInsert__const_uvec4__const_uvec4__const_uvec4__const_int__const_int(const_uvec4 *result,
                                                                                        const_uvec4 *base,
                                                                                        const_uvec4 *insert,
                                                                                        const_int   *offset,
                                                                                        const_int   *bits)
{
   for (int i=0; i<4; i++)
      bitfieldInsert__const_uint__const_uint__const_uint__const_int__const_int(&(*result)[i], &(*base)[i],
                                                                               &(*insert)[i], offset, bits);
}

static void bitfieldReverse__const_highp_uint__const_highp_uint(const_uint *result, const_uint *value)
{
   const_uint v = *value;
   v = ((v >> 1) & 0x55555555u) | ((v & 0x55555555u) << 1);
   v = ((v >> 2) & 0x33333333u) | ((v & 0x33333333u) << 2);
   v = ((v >> 4) & 0x0F0F0F0Fu) | ((v & 0x0F0F0F0Fu) << 4);
   v = ((v >> 8) & 0x00FF00FFu) | ((v & 0x00FF00FFu) << 8);
   *result = (v >> 16) | (v << 16);
}

static void bitfieldReverse__const_highp_uvec2__const_highp_uvec2(const_uvec2 *result, const_uvec2 *value)
{
   for (int i=0; i<2; i++)
      bitfieldReverse__const_highp_uint__const_highp_uint(&(*result)[i], &(*value)[i]);
}

static void bitfieldReverse__const_highp_uvec3__const_highp_uvec3(const_uvec3 *result, const_uvec3 *value)
{
   for (int i=0; i<3; i++)
      bitfieldReverse__const_highp_uint__const_highp_uint(&(*result)[i], &(*value)[i]);
}

static void bitfieldReverse__const_highp_uvec4__const_highp_uvec4(const_uvec4 *result, const_uvec4 *value)
{
   for (int i=0; i<4; i++)
      bitfieldReverse__const_highp_uint__const_highp_uint(&(*result)[i], &(*value)[i]);
}

static void bitfieldReverse__const_highp_int__const_highp_int(const_int *result, const_int *value)
{
   bitfieldReverse__const_highp_uint__const_highp_uint(result, value);
}

static void bitfieldReverse__const_highp_ivec2__const_highp_ivec2(const_ivec2 *result, const_ivec2 *value)
{
   for (int i=0; i<2; i++)
      bitfieldReverse__const_highp_int__const_highp_int(&(*result)[i], &(*value)[i]);
}

static void bitfieldReverse__const_highp_ivec3__const_highp_ivec3(const_ivec3 *result, const_ivec3 *value)
{
   for (int i=0; i<3; i++)
      bitfieldReverse__const_highp_int__const_highp_int(&(*result)[i], &(*value)[i]);
}

static void bitfieldReverse__const_highp_ivec4__const_highp_ivec4(const_ivec4 *result, const_ivec4 *value)
{
   for (int i=0; i<4; i++)
      bitfieldReverse__const_highp_int__const_highp_int(&(*result)[i], &(*value)[i]);
}

static void bitCount__const_lowp_int__const_uint(const_int *result, const_uint *value)
{
   const_int v = *value;
   v = v - ((v >> 1) & 0x55555555u);                 // 2 bit counters
   v = (v & 0x33333333u) + ((v >> 2) & 0x33333333u); // 4 bit counters
   v = (v + (v >> 4)) & 0x0F0F0F0Fu;                 // 8 bit counters
   v = (v + (v >> 8));  // 16 bit counters (8 LSB valid)
   v = (v + (v >> 16)); // 32 bit counter (8 LSB valid)
   *result = v & 0xFFu; // mask out invalid bits
}

static void bitCount__const_lowp_ivec2__const_uvec2(const_ivec2 *result, const_uvec2 *value)
{
   for (int i=0; i<2; i++)
      bitCount__const_lowp_int__const_uint(&(*result)[i], &(*value)[i]);
}

static void bitCount__const_lowp_ivec3__const_uvec3(const_ivec3 *result, const_uvec3 *value)
{
   for (int i=0; i<3; i++)
      bitCount__const_lowp_int__const_uint(&(*result)[i], &(*value)[i]);
}

static void bitCount__const_lowp_ivec4__const_uvec4(const_ivec4 *result, const_uvec4 *value)
{
   for (int i=0; i<4; i++)
      bitCount__const_lowp_int__const_uint(&(*result)[i], &(*value)[i]);
}

static void bitCount__const_lowp_int__const_int(const_int *result, const_int *value)
{
   bitCount__const_lowp_int__const_uint(result, value);
}

static void bitCount__const_lowp_ivec2__const_ivec2(const_ivec2 *result, const_ivec2 *value)
{
   for (int i=0; i<2; i++)
      bitCount__const_lowp_int__const_int(&(*result)[i], &(*value)[i]);
}

static void bitCount__const_lowp_ivec3__const_ivec3(const_ivec3 *result, const_ivec3 *value)
{
   for (int i=0; i<3; i++)
      bitCount__const_lowp_int__const_int(&(*result)[i], &(*value)[i]);
}

static void bitCount__const_lowp_ivec4__const_ivec4(const_ivec4 *result, const_ivec4 *value)
{
   for (int i=0; i<4; i++)
      bitCount__const_lowp_int__const_int(&(*result)[i], &(*value)[i]);
}

static void findLSB__const_lowp_int__const_uint(const_int *result, const_uint *value)
{
   int i;
   const_int v = *value;
   if (v == 0) {
      *result = const_value_from_signed(-1);
      return;
   }

   for (i=0; i<32; i++) {
      if (v & (1<<i)) break;
   }
   *result = i;
}

static void findLSB__const_lowp_ivec2__const_uvec2(const_ivec2 *result, const_uvec2 *value)
{
   for (int i=0; i<2; i++)
      findLSB__const_lowp_int__const_uint(&(*result)[i], &(*value)[i]);
}

static void findLSB__const_lowp_ivec3__const_uvec3(const_ivec3 *result, const_uvec3 *value)
{
   for (int i=0; i<3; i++)
      findLSB__const_lowp_int__const_uint(&(*result)[i], &(*value)[i]);
}

static void findLSB__const_lowp_ivec4__const_uvec4(const_ivec4 *result, const_uvec4 *value)
{
   for (int i=0; i<4; i++)
      findLSB__const_lowp_int__const_uint(&(*result)[i], &(*value)[i]);
}

static void findLSB__const_lowp_int__const_int(const_int *result, const_uint *value)
{
   findLSB__const_lowp_int__const_uint(result, value);
}

static void findLSB__const_lowp_ivec2__const_ivec2(const_ivec2 *result, const_ivec2 *value)
{
   for (int i=0; i<2; i++)
      findLSB__const_lowp_int__const_int(&(*result)[i], &(*value)[i]);
}

static void findLSB__const_lowp_ivec3__const_ivec3(const_ivec3 *result, const_ivec3 *value)
{
   for (int i=0; i<3; i++)
      findLSB__const_lowp_int__const_int(&(*result)[i], &(*value)[i]);
}

static void findLSB__const_lowp_ivec4__const_ivec4(const_ivec4 *result, const_ivec4 *value)
{
   for (int i=0; i<4; i++)
      findLSB__const_lowp_int__const_int(&(*result)[i], &(*value)[i]);
}

static void findMSB__const_lowp_int__const_uint(const_int *result, const_uint *value)
{
   int i;
   const_int v = *value;
   if (v == 0) {
      *result = const_value_from_signed(-1);
      return;
   }

   // clear the next bit after MSB (round down)
   v = v & ~(v >> 1);
   for (i=31; i>=0; i--) {
      if ( v & (1<<i) ) break;
   }
   *result = i;
}

static void findMSB__const_lowp_ivec2__const_uvec2(const_ivec2 *result, const_uvec2 *value)
{
   for (int i=0; i<2; i++)
      findMSB__const_lowp_int__const_uint(&(*result)[i], &(*value)[i]);
}

static void findMSB__const_lowp_ivec3__const_uvec3(const_ivec3 *result, const_uvec3 *value)
{
   for (int i=0; i<3; i++)
      findMSB__const_lowp_int__const_uint(&(*result)[i], &(*value)[i]);
}

static void findMSB__const_lowp_ivec4__const_uvec4(const_ivec4 *result, const_uvec4 *value)
{
   for (int i=0; i<4; i++)
      findMSB__const_lowp_int__const_uint(&(*result)[i], &(*value)[i]);
}

static void findMSB__const_lowp_int__const_int(const_int *result, const_int *value)
{
   const_signed v0 = const_signed_from_value(*value);
   const_value  v1;
   if (v0 < 0) v0 = ~v0;
   v1 = const_value_from_signed(v0);
   findMSB__const_lowp_int__const_uint(result, &v1);
}

static void findMSB__const_lowp_ivec2__const_ivec2(const_ivec2 *result, const_ivec2 *value)
{
   for (int i=0; i<2; i++)
      findMSB__const_lowp_int__const_int(&(*result)[i], &(*value)[i]);
}

static void findMSB__const_lowp_ivec3__const_ivec3(const_ivec3 *result, const_ivec3 *value)
{
   for (int i=0; i<3; i++)
      findMSB__const_lowp_int__const_int(&(*result)[i], &(*value)[i]);
}

static void findMSB__const_lowp_ivec4__const_ivec4(const_ivec4 *result, const_ivec4 *value)
{
   for (int i=0; i<4; i++)
      findMSB__const_lowp_int__const_int(&(*result)[i], &(*value)[i]);
}
