uint atomicCounterIncrement(atomic_uint c) { return $$atomicAdd(c, 1u); }
uint atomicCounterDecrement(atomic_uint c) { return $$atomicSub(c, 1u) - 1u; }
uint atomicCounter         (atomic_uint c) { return $$atomicLoad(c); }

uint atomicAdd(inout uint mem, uint data) { return $$atomicAdd(mem, data); }
 int atomicAdd(inout  int mem,  int data) { return $$atomicAdd(mem, data); }
uint atomicMin(inout uint mem, uint data) { return $$atomicMin(mem, data); }
 int atomicMin(inout  int mem,  int data) { return $$atomicMin(mem, data); }
uint atomicMax(inout uint mem, uint data) { return $$atomicMax(mem, data); }
 int atomicMax(inout  int mem,  int data) { return $$atomicMax(mem, data); }
uint atomicAnd(inout uint mem, uint data) { return $$atomicAnd(mem, data); }
 int atomicAnd(inout  int mem,  int data) { return $$atomicAnd(mem, data); }
uint atomicOr (inout uint mem, uint data) { return $$atomicOr (mem, data); }
 int atomicOr (inout  int mem,  int data) { return $$atomicOr (mem, data); }
uint atomicXor(inout uint mem, uint data) { return $$atomicXor(mem, data); }
 int atomicXor(inout  int mem,  int data) { return $$atomicXor(mem, data); }
uint atomicExchange(inout uint mem, uint data) { return $$atomicXchg(mem, data); }
 int atomicExchange(inout  int mem,  int data) { return $$atomicXchg(mem, data); }
uint atomicCompSwap(inout uint mem, uint compare, uint data) { return $$atomicCmpxchg(mem, compare, data); }
 int atomicCompSwap(inout  int mem,  int compare,  int data) { return $$atomicCmpxchg(mem, compare, data); }
