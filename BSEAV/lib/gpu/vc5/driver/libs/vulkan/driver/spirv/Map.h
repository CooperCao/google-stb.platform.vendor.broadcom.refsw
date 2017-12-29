/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <glsl_map.h>

// PtrMap
//
// Wrapper over a glsl map which handles lifetime and
// provides a type-safe interface
template <typename K, typename V>
class PtrMap
{
public:
   PtrMap()
   {
      m_map = glsl_map_new();
   }

   ~PtrMap()
   {
      glsl_map_delete(m_map);
   }

   void Put(const K *key, V *val)
   {
      glsl_map_put(m_map, key, val);
   }

   V *Get(const K *key) const
   {
      return static_cast<V *>(glsl_map_get(m_map, key));
   }

   Map *GetMap() const { return m_map; }

   // TODO -- currently we don't do any map
   // iteration, but we might want iterators in future.

private:
   Map *m_map;
};