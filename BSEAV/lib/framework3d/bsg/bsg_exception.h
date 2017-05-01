/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_EXCEPTION_H__
#define __BSG_EXCEPTION_H__

#include "bsg_common.h"
#include "bsg_compiler_quirks.h"

#include <sstream>
#include <string>
#include <stdint.h>

namespace bsg
{

//! The Exception object can be thrown at any point whilst BSG is running.
//! You need to catch these exceptions if you want to clean up nicely.
class Exception
{
public:
   Exception(std::string message) :
      m_message(message)
   {}

   //! The error message associated with this exception
   const std::string &Message() const
   {
      return m_message;
   }

private:
   std::string m_message;
};

void Oops(Exception x) BSG_NO_RETURN;
void GLOops(const char *file, uint32_t line);

#define BSG_THROW(MESSAGE) bsg::Oops(bsg::Exception(((std::stringstream &)(std::stringstream() <<  \
                                                "File "   << __FILE__     <<             \
                                                ", Line " << __LINE__     <<             \
                                                " in "     << __FUNCTION__ <<             \
                                                " : "      << MESSAGE)).str()))

#define GLDB bsg::GLOops(__FILE__, __LINE__)

}
#endif