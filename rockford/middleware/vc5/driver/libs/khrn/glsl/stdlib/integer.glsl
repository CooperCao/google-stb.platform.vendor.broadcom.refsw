int bitfieldExtract(int value, int offset, int bits)
{
   // NB: arithmetic shift will replicate the most significant bit
   return bits == 0 ? 0 : (value << (32 - bits - offset)) >> (32 - bits);
}

ivec2 bitfieldExtract(ivec2 value, int offset, int bits)
{
   return ivec2(bitfieldExtract(value[0], offset, bits),
                bitfieldExtract(value[1], offset, bits) );
}

ivec3 bitfieldExtract(ivec3 value, int offset, int bits)
{
   return ivec3(bitfieldExtract(value[0], offset, bits),
                bitfieldExtract(value[1], offset, bits),
                bitfieldExtract(value[2], offset, bits) );
}

ivec4 bitfieldExtract(ivec4 value, int offset, int bits)
{
   return ivec4(bitfieldExtract(value[0], offset, bits),
                bitfieldExtract(value[1], offset, bits),
                bitfieldExtract(value[2], offset, bits),
                bitfieldExtract(value[3], offset, bits) );
}

uint bitfieldExtract(uint value, int offset, int bits)
{
   return bits == 0 ? 0u : (value << (32 - bits - offset)) >> (32 - bits);
}

uvec2 bitfieldExtract(uvec2 value, int offset, int bits)
{
   return uvec2(bitfieldExtract(value[0], offset, bits),
                bitfieldExtract(value[1], offset, bits) );
}

uvec3 bitfieldExtract(uvec3 value, int offset, int bits)
{
   return uvec3(bitfieldExtract(value[0], offset, bits),
                bitfieldExtract(value[1], offset, bits),
                bitfieldExtract(value[2], offset, bits) );
}

uvec4 bitfieldExtract(uvec4 value, int offset, int bits)
{
   return uvec4(bitfieldExtract(value[0], offset, bits),
                bitfieldExtract(value[1], offset, bits),
                bitfieldExtract(value[2], offset, bits),
                bitfieldExtract(value[3], offset, bits) );
}

int bitfieldInsert(int base, int insert, int offset, int bits)
{
   int mask = int((0xFFFFFFFFu >> (32 - bits)) << offset);
   return bits == 0 ? base : ((insert << offset) & mask) + (base & ~mask);
}

ivec2 bitfieldInsert(ivec2 base, ivec2 insert, int offset, int bits)
{
   return ivec2(bitfieldInsert(base[0], insert[0], offset, bits),
                bitfieldInsert(base[1], insert[1], offset, bits) );
}

ivec3 bitfieldInsert(ivec3 base, ivec3 insert, int offset, int bits)
{
   return ivec3(bitfieldInsert(base[0], insert[0], offset, bits),
                bitfieldInsert(base[1], insert[1], offset, bits),
                bitfieldInsert(base[2], insert[2], offset, bits) );
}

ivec4 bitfieldInsert(ivec4 base, ivec4 insert, int offset, int bits)
{
   return ivec4(bitfieldInsert(base[0], insert[0], offset, bits),
                bitfieldInsert(base[1], insert[1], offset, bits),
                bitfieldInsert(base[2], insert[2], offset, bits),
                bitfieldInsert(base[3], insert[3], offset, bits) );
}

uint bitfieldInsert(uint base, uint insert, int offset, int bits)
{
   return uint(bitfieldInsert(int(base), int(insert), offset, bits));
}

uvec2 bitfieldInsert(uvec2 base, uvec2 insert, int offset, int bits)
{
   return uvec2(bitfieldInsert(base[0], insert[0], offset, bits),
                bitfieldInsert(base[1], insert[1], offset, bits) );
}

uvec3 bitfieldInsert(uvec3 base, uvec3 insert, int offset, int bits)
{
   return uvec3(bitfieldInsert(base[0], insert[0], offset, bits),
                bitfieldInsert(base[1], insert[1], offset, bits),
                bitfieldInsert(base[2], insert[2], offset, bits) );
}

uvec4 bitfieldInsert(uvec4 base, uvec4 insert, int offset, int bits)
{
   return uvec4(bitfieldInsert(base[0], insert[0], offset, bits),
                bitfieldInsert(base[1], insert[1], offset, bits),
                bitfieldInsert(base[2], insert[2], offset, bits),
                bitfieldInsert(base[3], insert[3], offset, bits) );
}

highp uint bitfieldReverse(highp uint value)
{
   highp uint masks[4] = uint[4](0x55555555u, 0x33333333u, 0x0F0F0F0Fu, 0x00FF00FFu);
   int shifts[4] = int[4](1,2,4,8);
   value = ((value >> shifts[0]) & masks[0]) + ((value & masks[0]) << shifts[0]); // swap 1 bit groups
   value = ((value >> shifts[1]) & masks[1]) + ((value & masks[1]) << shifts[1]); // swap 2 bit groups
   value = ((value >> shifts[2]) & masks[2]) + ((value & masks[2]) << shifts[2]); // swap 4 bit groups
   value = ((value >> shifts[3]) & masks[3]) + ((value & masks[3]) << shifts[3]); // swap 8 bit groups
   return $$ror(value, 16u);
}

highp uvec2 bitfieldReverse(highp uvec2 value)
{
   return uvec2(bitfieldReverse(value[0]), bitfieldReverse(value[1]));
}

highp uvec3 bitfieldReverse(highp uvec3 value)
{
   return uvec3(bitfieldReverse(value[0]),
                bitfieldReverse(value[1]),
                bitfieldReverse(value[2]) );
}

highp uvec4 bitfieldReverse(highp uvec4 value)
{
   return uvec4(bitfieldReverse(value[0]), bitfieldReverse(value[1]),
                bitfieldReverse(value[2]), bitfieldReverse(value[3]) );
}

highp int bitfieldReverse(highp int value)
{
   return int(bitfieldReverse(uint(value)));
}

highp ivec2 bitfieldReverse(highp ivec2 value)
{
   return ivec2(bitfieldReverse(value[0]), bitfieldReverse(value[1]) );
}

highp ivec3 bitfieldReverse(highp ivec3 value)
{
   return ivec3(bitfieldReverse(value[0]),
                bitfieldReverse(value[1]),
                bitfieldReverse(value[2]) );
}

highp ivec4 bitfieldReverse(highp ivec4 value)
{
   return ivec4(bitfieldReverse(value[0]), bitfieldReverse(value[1]),
                bitfieldReverse(value[2]), bitfieldReverse(value[3]) );
}

lowp int bitCount(uint value)
{
   value = value - ((value >> 1) & 0x55555555u);                 // 2 bit counters
   value = (value & 0x33333333u) + ((value >> 2) & 0x33333333u); // 4 bit counters
   value = (value + (value >> 4)) & 0x0F0F0F0Fu;                 // 8 bit counters
   return int((value * 0x01010101u) >> 24);
}

lowp ivec2 bitCount(uvec2 value)
{
   return ivec2(bitCount(value[0]), bitCount(value[1]));
}

lowp ivec3 bitCount(uvec3 value)
{
   return ivec3(bitCount(value[0]), bitCount(value[1]), bitCount(value[2]));
}

lowp ivec4 bitCount(uvec4 value)
{
   return ivec4(bitCount(value[0]), bitCount(value[1]),
                bitCount(value[2]), bitCount(value[3]) );
}

lowp int bitCount(int value)
{
   return bitCount(uint(value));
}

lowp ivec2 bitCount(ivec2 value)
{
   return ivec2(bitCount(value[0]), bitCount(value[1]));
}

lowp ivec3 bitCount(ivec3 value)
{
   return ivec3(bitCount(value[0]), bitCount(value[1]), bitCount(value[2]));
}

lowp ivec4 bitCount(ivec4 value)
{
   return ivec4(bitCount(value[0]), bitCount(value[1]),
                bitCount(value[2]), bitCount(value[3]) );
}

lowp int findLSB(uint value)
{
   // clear all bits except LSB
   value = value & -value;
   return findMSB(value);
}

lowp ivec2 findLSB(uvec2 value)
{
   return ivec2(findLSB(value[0]), findLSB(value[1]));
}

lowp ivec3 findLSB(uvec3 value)
{
   return ivec3(findLSB(value[0]), findLSB(value[1]), findLSB(value[2]));
}

lowp ivec4 findLSB(uvec4 value)
{
   return ivec4(findLSB(value[0]), findLSB(value[1]),
                findLSB(value[2]), findLSB(value[3]) );
}

lowp int findLSB(int value)
{
   return findLSB(uint(value));
}

lowp ivec2 findLSB(ivec2 value)
{
   return ivec2(findLSB(value[0]), findLSB(value[1]));
}

lowp ivec3 findLSB(ivec3 value)
{
   return ivec3(findLSB(value[0]), findLSB(value[1]), findLSB(value[2]));
}

lowp ivec4 findLSB(ivec4 value)
{
   return ivec4(findLSB(value[0]), findLSB(value[1]),
                findLSB(value[2]), findLSB(value[3]) );
}

lowp int findMSB(uint value)
{
   return 31 - int($$clz(value));
}

lowp ivec2 findMSB(uvec2 value)
{
   return ivec2(findMSB(value[0]), findMSB(value[1]));
}

lowp ivec3 findMSB(uvec3 value)
{
   return ivec3(findMSB(value[0]), findMSB(value[1]), findMSB(value[2]));
}

lowp ivec4 findMSB(uvec4 value)
{
   return ivec4(findMSB(value[0]), findMSB(value[1]),
                findMSB(value[2]), findMSB(value[3]) );
}

lowp int findMSB(int value)
{
   return findMSB(uint(value < 0 ? ~value : value));
}

lowp ivec2 findMSB(ivec2 value)
{
   return ivec2(findMSB(value[0]), findMSB(value[1]));
}

lowp ivec3 findMSB(ivec3 value)
{
   return ivec3(findMSB(value[0]), findMSB(value[1]), findMSB(value[2]));
}

lowp ivec4 findMSB(ivec4 value)
{
   return ivec4(findMSB(value[0]), findMSB(value[1]),
                findMSB(value[2]), findMSB(value[3]) );
}

highp uint uaddCarry(highp uint x, highp uint y, out lowp uint carry)
{
   uint r = x + y;
   carry = r < x ? 1u : 0u;
   return r;
}

highp uvec2 uaddCarry(highp uvec2 x, highp uvec2 y, out lowp uvec2 carry)
{
   return uvec2( uaddCarry(x[0], y[0], carry[0]),
                 uaddCarry(x[1], y[1], carry[1]) );
}

highp uvec3 uaddCarry(highp uvec3 x, highp uvec3 y, out lowp uvec3 carry)
{
   return uvec3( uaddCarry(x[0], y[0], carry[0]),
                 uaddCarry(x[1], y[1], carry[1]),
                 uaddCarry(x[2], y[2], carry[2]) );
}

highp uvec4 uaddCarry(highp uvec4 x, highp uvec4 y, out lowp uvec4 carry)
{
   return uvec4( uaddCarry(x[0], y[0], carry[0]),
                 uaddCarry(x[1], y[1], carry[1]),
                 uaddCarry(x[2], y[2], carry[2]),
                 uaddCarry(x[3], y[3], carry[3]) );
}

highp uint usubBorrow(highp uint x, highp uint y, out lowp uint borrow)
{
   uint r = x - y;
   borrow = x >= y ? 0u : 1u;
   return r;
}

highp uvec2 usubBorrow(highp uvec2 x, highp uvec2 y, out lowp uvec2 borrow)
{
   return uvec2( usubBorrow(x[0], y[0], borrow[0]),
                 usubBorrow(x[1], y[1], borrow[1]) );
}

highp uvec3 usubBorrow(highp uvec3 x, highp uvec3 y, out lowp uvec3 borrow)
{
   return uvec3( usubBorrow(x[0], y[0], borrow[0]),
                 usubBorrow(x[1], y[1], borrow[1]),
                 usubBorrow(x[2], y[2], borrow[2]) );
}

highp uvec4 usubBorrow(highp uvec4 x, highp uvec4 y, out lowp uvec4 borrow)
{
   return uvec4( usubBorrow(x[0], y[0], borrow[0]),
                 usubBorrow(x[1], y[1], borrow[1]),
                 usubBorrow(x[2], y[2], borrow[2]),
                 usubBorrow(x[3], y[3], borrow[3]) );
}


void umulExtended(highp uint x, highp uint y, out highp uint msb, out highp uint lsb)
{
   #define LOW(x)  ((x) & 0xFFFFu)
   #define HIGH(x) ((x) >> 16)

   highp uint mul0 = LOW(x)  * LOW(y);  // << 0
   highp uint mul1 = LOW(x)  * HIGH(y); // << 16
   highp uint mul2 = HIGH(x) * LOW(y);  // << 16
   highp uint mul3 = HIGH(x) * HIGH(y); // << 32

   //                         HIGH(mul0) LOW(mul0)
   // +            HIGH(mul1) LOW(mul1)
   // +            HIGH(mul2) LOW(mul2)
   // + HIGH(mul3) LOW(mul3)
   // --------------------------------------------
   // = HIGH(msb)  LOW(msb)   HIGH(lsb)  LOW(lsb)

   highp uint tmp; // << 16
   lsb = LOW(mul0);
   tmp = HIGH(mul0);
   tmp += mul1; // does not overflow
   msb = HIGH(tmp); // move out high bits
   tmp = LOW(tmp);
   tmp += mul2; // does not overflow
   lsb += tmp << 16;
   msb += HIGH(tmp);
   msb += mul3;

   #undef LOW
   #undef HIGH
}

void umulExtended(highp uvec2 x, highp uvec2 y, out highp uvec2 msb, out highp uvec2 lsb)
{
   umulExtended(x[0], y[0], msb[0], lsb[0]);
   umulExtended(x[1], y[1], msb[1], lsb[1]);
}

void umulExtended(highp uvec3 x, highp uvec3 y, out highp uvec3 msb, out highp uvec3 lsb)
{
   umulExtended(x[0], y[0], msb[0], lsb[0]);
   umulExtended(x[1], y[1], msb[1], lsb[1]);
   umulExtended(x[2], y[2], msb[2], lsb[2]);
}

void umulExtended(highp uvec4 x, highp uvec4 y, out highp uvec4 msb, out highp uvec4 lsb)
{
   umulExtended(x[0], y[0], msb[0], lsb[0]);
   umulExtended(x[1], y[1], msb[1], lsb[1]);
   umulExtended(x[2], y[2], msb[2], lsb[2]);
   umulExtended(x[3], y[3], msb[3], lsb[3]);
}

void imulExtended(highp int x, highp int y, out highp int msb, out highp int lsb)
{
   highp uint _msb, _lsb;
   umulExtended(uint(x), uint(y), _msb, _lsb);
   msb = int(_msb);
   lsb = int(_lsb);
   if (x < 0) msb -= y;
   if (y < 0) msb -= x;
}

void imulExtended(highp ivec2 x, highp ivec2 y, out highp ivec2 msb, out highp ivec2 lsb)
{
   imulExtended(x[0], y[0], msb[0], lsb[0]);
   imulExtended(x[1], y[1], msb[1], lsb[1]);
}

void imulExtended(highp ivec3 x, highp ivec3 y, out highp ivec3 msb, out highp ivec3 lsb)
{
   imulExtended(x[0], y[0], msb[0], lsb[0]);
   imulExtended(x[1], y[1], msb[1], lsb[1]);
   imulExtended(x[2], y[2], msb[2], lsb[2]);
}

void imulExtended(highp ivec4 x, highp ivec4 y, out highp ivec4 msb, out highp ivec4 lsb)
{
   imulExtended(x[0], y[0], msb[0], lsb[0]);
   imulExtended(x[1], y[1], msb[1], lsb[1]);
   imulExtended(x[2], y[2], msb[2], lsb[2]);
   imulExtended(x[3], y[3], msb[3], lsb[3]);
}
