/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <type_traits>

#include "Spirv.h"
#include "Optional.h"
#include "NodeBase.h"
#include "PoolAllocator.h"

namespace bvk {

class NodeType;

///////////////////////////////////////////////////////////////////////////////
// Extractor
//
// Utility class that wraps the SPIRV object with the current instruction.
// Provides stream read to help pull arguments out and some direct access
// methods as well.
///////////////////////////////////////////////////////////////////////////////

class Module;

template <class T>
struct NumberOfWords
{
   static_assert(sizeof(T) == 4, "Meta function NumberOfWords not defined");

   static const uint32_t value = 1;
};

// "Node *" is translated from a result id which is one word
template <>
struct NumberOfWords<const Node *>
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
   Extractor(Module &module, const uint32_t *instr);

   uint32_t  GetWordCount() const  { return m_wordCount;           }
   uint32_t  WordsLeft()    const  { return m_wordCount - m_index; }
   spv::Core GetOpCode()    const  { return m_opCode;              }

   explicit operator bool() const  { return WordsLeft() > 0;       }

   const SpvAllocator &GetAllocator() const { return m_allocator; }

   template <typename T>
   T *NewData()
   {
      return m_allocator.New<T>(m_allocator);
   }

   spv::GLSL GetExtOpCode() const
   {
      assert(m_opCode == spv::Core::OpExtInst);
      return static_cast<spv::GLSL>(m_instr[4]);
   }

   // Catch all for non-pointer classes
   template <class T>
   Extractor &operator>>(T &x)
   {
      // Must be a word sized thing
      assert(sizeof(T) == sizeof(uint32_t));
      assert(!std::is_pointer<T>::value);

      x = *reinterpret_cast<const T *>(&m_instr[m_index]);
      m_index++;
      return *this;
   }

   Extractor &operator>>(const Node *&node);
   Extractor &operator>>(const NodeType *&node);

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
   SpvAllocator   &m_allocator;
   const uint32_t *m_instr;     // SPIRV raw instruction data
   uint32_t        m_index;     // Current word index
   uint32_t        m_wordCount; // Number of words in the instruction
   spv::Core       m_opCode;    // Opcode for this instruction
};

} // namespace bvk
