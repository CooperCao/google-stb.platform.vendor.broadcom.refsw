/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <type_traits>

#include "Spirv.h"
#include "Optional.h"
#include "NodeIndex.h"
#include "ModuleAllocator.h"

namespace bvk {

class NodeType;

///////////////////////////////////////////////////////////////////////////////
// Extractor
//
// Utility class that wraps the SPIRV object with the current instruction.
// Provides stream read to help pull arguments out an some direct access
// methods as well.
///////////////////////////////////////////////////////////////////////////////

class Module;

template <class T>
struct NumberOfWords
{
   static_assert(sizeof(T) == 4, "Meta function NumberOfWords not defined");

   static const uint32_t value = 1;
};

template <>
struct NumberOfWords<NodeIndex>
{
   static const uint32_t value = 1;
};

template <class S, class T>
struct NumberOfWords<std::pair<S, T>>
{
   static const uint32_t value = NumberOfWords<S>::value + NumberOfWords<T>::value;
};

class Extractor
{
public:
   Extractor(Module &module, const uint32_t *instr) :
      m_module(module),
      m_instr(instr)
   {
      m_opCode    = static_cast<spv::Core>(instr[0] & 0xffff);
      m_wordCount = instr[0] >> 16;
      // Point at first argument
      m_index     = 1;
   }

   uint32_t  GetWordCount() const  { return m_wordCount;           }
   uint32_t  WordsLeft()    const  { return m_wordCount - m_index; }
   spv::Core GetOpCode()    const  { return m_opCode;              }

   explicit operator bool() const  { return WordsLeft() > 0;       }

   spv::ModuleAllocator<uint32_t> &GetAllocator() const;

   spv::GLSL GetExtOpCode() const
   {
      assert(m_opCode == spv::Core::OpExtInst);
      return static_cast<spv::GLSL>(m_instr[4]);
   }

   template <class T>
   Extractor &operator>>(T &x)
   {
      // Must be a word sized thing
      assert(sizeof(T) == sizeof(uint32_t));

      x = *reinterpret_cast<const T *>(&m_instr[m_index]);
      m_index++;
      return *this;
   }

   Extractor &operator>>(NodeIndex &index);
   Extractor &operator>>(const NodeType *&type);

   // Skip a word (if it's not needed or used)
   Extractor &operator>>(std::nullptr_t)
   {
      m_index++;
      return *this;
   }

   // Extract into a std::string
   Extractor &operator>>(std::string &str)
   {
      if (WordsLeft() > 0)
      {
         str = reinterpret_cast<const char *>(&m_instr[m_index]);
         m_index += str.size() / sizeof(uint32_t) + 1;
      }
      return *this;
   }

   // Extract into a spv::string
   Extractor &operator>>(spv::string &str)
   {
      if (WordsLeft() > 0)
      {
         str = reinterpret_cast<const char *>(&m_instr[m_index]);
         m_index += str.size() / sizeof(uint32_t) + 1;
      }
      return *this;
   }

   // Extract into a vector
   template <typename T>
   Extractor &operator>>(spv::vector<T> &vec)
   {
      uint32_t elements = WordsLeft() / NumberOfWords<T>::value;

      vec.resize(elements);

      for (auto &elem : vec)
         *this >> elem;

      return *this;
   }

   // Extract into an optional
   template <typename T>
   Extractor &operator>>(Optional<T> &opt)
   {
      if (WordsLeft() > 0)
      {
         T  tmp;
         *this >> tmp;
         opt = tmp;
      }

      return *this;
   }

   // Extract into a pair
   template <typename A, typename B>
   Extractor &operator>>(std::pair<A, B> &pair)
   {
      *this >> pair.first;
      *this >> pair.second;
      return *this;
   }

private:
   Module         &m_module;    // The module
   const uint32_t *m_instr;     // SPIRV raw instruction data
   uint32_t        m_index = 0; // Current word index
   uint32_t        m_wordCount; // Number of words in the instruction
   spv::Core       m_opCode;    // Opcode for this instruction
};

} // namespace bvk
