/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_PARSE_UTILS_H__
#define __BSG_PARSE_UTILS_H__

#include "bsg_common.h"

// @cond

#include <string>

namespace bsg
{
   //! Helper utilities for parsing strings
   class ParseUtils
   {
   public:
      //! Strip whitespace from start and end of string
      static std::string StripWhite(const std::string &str);
      //! Remove delimiters listed from string
      static std::string RemoveDelims(const std::string &str, const std::string &delims);
      //! Convert to lower case
      static std::string ToLower(const std::string &str);
      //! Convert to upper case
      static std::string ToUpper(const std::string &str);
      //! Returns true if the string starts with the given string
      static bool StartsWith(const std::string &str, const std::string &startsWith);
      //! Returns true if the given path is a directory
      static bool IsDirectory(const std::string &str);
      //! Returns true if the given path is a file
      static bool IsFile(const std::string &str);
      //! Returns the basename of a path
      static std::string GetBasename(const std::string &str);
      //! Returns the pathname of a path
      static std::string GetPathname(const std::string &str);
   };

}

// @endcond

#endif /* __BSG_PARSE_UTILS_H__ */
