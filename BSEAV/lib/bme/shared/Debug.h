/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __SHARED_DEBUG_H_
#define __SHARED_DEBUG_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>

// Remove this to reduce log messages
#define BME_VERBOSE 1

#define BME_CHECK(x...)                                                  \
{                                                                       \
    NEXUS_Error err = (NEXUS_Error)x;                                   \
    if (err != NEXUS_SUCCESS) {                                         \
        BME_DEBUG_PRINT(("Nexus error retured %d", err));             \
        BME_DEBUG_THROW_EXCEPTION(ReturnFailure);                   \
    }                                                                   \
}

// Annotate a virtual method indicating it must be overriding a virtual
// method in the parent class.
// Use like:
//   virtual void foo() OVERRIDE;
#if !defined(OVERRIDE)
#if __cplusplus >= 201103 && \
      (__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= 40700
// GCC 4.7 supports explicit virtual overrides when C++11 support is enabled.
#define OVERRIDE override
#else
#define OVERRIDE
#endif
#endif

template <typename T>
static std::string toString(T value)
{
    std::ostringstream ss;
    ss << value;
    return ss.str();
}

#define BME_SO_EXPORT __attribute__((visibility("default")))

namespace Broadcom
{

enum ReturnCode {
    ReturnSuccess,      /* Success */
    ReturnInvalidParam, /* Invalid Parameter */
    ReturnInvalidState, /* Invalid State */
    ReturnOutofMemory,  /* Out of memory failure */
    ReturnNotSupported, /* Not supported */
    ReturnTimeout,      /* Timed out */
    ReturnFailure,      /* Generic Failure */
    ReturnNotFound,     /* Not found */
    ReturnIncorrectPassword, /* Incorrect password */
    ReturnMultipleApsFound /* Multiple access points were found error */
};

enum DebugLevel {
    DebugNone = 0x0,
    DebugError = 0x1,
    DebugWarning = 0x2,
    DebugTrace = 0x4,
    DebugAll = 0x7
};

static const std::string RETURN_SUCCESS       = "Success";
static const std::string RETURN_INVALID_PARAM = "Invalid parameter";
static const std::string RETURN_INVALID_STATE = "Invalid state";
static const std::string RETURN_OUT_OF_MEMORY = "Out of memory";
static const std::string RETURN_NOT_SUPPORTED = "Not supported";
static const std::string RETURN_TIMEOUT       = "Timed out";
static const std::string RETURN_FAILURE       = "Generic failure";
static const std::string RETURN_NOT_FOUND     = "Not Found";
static const std::string RETURN_INCORRECT_PASSWORD= "Incorrect Password";
static const std::string RETURN_MULTIPLE_APS_FOUND       = "Multiple AP's Found";

class BME_SO_EXPORT Exception
{
public:
    Exception() : code(ReturnSuccess) {}
    Exception(const Exception& rhs) : code(rhs.getCode()), error(rhs.what()) {}
    Exception(const std::exception& rhs) : code(ReturnFailure), error(rhs.what()) {}
    Exception(const std::string& str) : code(convertString(str)), error(str) {}
    Exception(const ReturnCode& code) : code(code) {}
    Exception(const ReturnCode& code, const char *f, int l)  : code(ReturnFailure) {
        std::ostringstream os;
        os << "code:" << code << " file:" << f << " (" << l <<  ")" << std::endl;
        error = os.str();
    }
    Exception& operator=(const Exception& rhs) {
        code = rhs.getCode();
        error = rhs.what();
        return *this;
    }
    virtual ~Exception() throw() {}

    static ReturnCode convertString(const std::string& string) {
        if (string == RETURN_SUCCESS) {
            return ReturnSuccess;
        } else if (string == RETURN_INVALID_PARAM) {
            return ReturnInvalidParam;
        } else if (string == RETURN_INVALID_STATE) {
            return ReturnInvalidState;
        } else if (string == RETURN_OUT_OF_MEMORY) {
            return ReturnOutofMemory;
        } else if (string == RETURN_NOT_SUPPORTED) {
            return ReturnNotSupported;
        } else if (string == RETURN_TIMEOUT) {
            return ReturnTimeout;
        } else if (string == RETURN_NOT_FOUND) {
            return ReturnNotFound;
        } else if (string == RETURN_INCORRECT_PASSWORD) {
            return ReturnIncorrectPassword;
        } else if (string == RETURN_MULTIPLE_APS_FOUND) {
            return ReturnMultipleApsFound;
        } else {
            return ReturnFailure;
        }
    }

    virtual ReturnCode getCode() const {
        return code;
    }

    virtual const char* what() const
    {
        switch (code) {
            case ReturnSuccess:
                return RETURN_SUCCESS.c_str();
            case ReturnInvalidParam:
                return RETURN_INVALID_PARAM.c_str();
            case ReturnInvalidState:
                return RETURN_INVALID_STATE.c_str();
            case ReturnOutofMemory:
                return RETURN_OUT_OF_MEMORY.c_str();
            case ReturnNotSupported:
                return RETURN_NOT_SUPPORTED.c_str();
            case ReturnTimeout:
                return RETURN_TIMEOUT.c_str();
            case ReturnNotFound:
                return RETURN_NOT_FOUND.c_str();
            case ReturnIncorrectPassword:
                return RETURN_INCORRECT_PASSWORD.c_str();
            case ReturnMultipleApsFound:
                return RETURN_MULTIPLE_APS_FOUND.c_str();
            case ReturnFailure:
            default:
                return error.c_str();
        }
    }

private:
    ReturnCode code;
    std::string error;
};

#define TRLS_TRACE_MODULES "bme_trace_modules"
BME_SO_EXPORT void bmeDebugPrint(DebugLevel level, const char* category, const char* fileName, int32_t lineNumber, const char *format, ...);
BME_SO_EXPORT void bmeDebugSetSettings(uint32_t level, uint32_t category);
void debugInitialize(const char * contextName = NULL);
}

#ifndef BME_ENABLE_RTTI
#define dynamic_cast static_cast
#endif

#define TRLS_DBG_MODULE(cat) static const char* bme_module_name=#cat

#define TRLS_P_UNWRAP(...) __VA_ARGS__
#define BME_DEBUG_PRINT(fmt) bmeDebugPrint(DebugAll, bme_module_name, bme_module_name, __LINE__, TRLS_P_UNWRAP fmt)
#define BME_DEBUG_ERROR(fmt) bmeDebugPrint(DebugError, bme_module_name, bme_module_name, __LINE__, TRLS_P_UNWRAP fmt)
#define BME_DEBUG_WARNING(fmt) bmeDebugPrint(DebugWarning, bme_module_name, bme_module_name, __LINE__, TRLS_P_UNWRAP fmt)
#if defined(BME_VERBOSE) && BME_VERBOSE
#define BME_DEBUG_TRACE(fmt) bmeDebugPrint(DebugTrace, bme_module_name, bme_module_name, __LINE__, TRLS_P_UNWRAP fmt)
#define BME_DEBUG_ENTER() bmeDebugPrint(DebugTrace, bme_module_name, bme_module_name, __LINE__, "ENTER %s", __FUNCTION__)
#define BME_DEBUG_EXIT() bmeDebugPrint(DebugTrace, bme_module_name, bme_module_name, __LINE__, "EXIT %s", __FUNCTION__)
#endif

#define TRLS_ASSERT(x) if (!(x)) { printf("\n>>>> assertion failed in %s(%d) <<<<\n\n", __FILE__, __LINE__); fflush(stdout); abort(); }

#define TRLS_UNUSED(x) (void)x

#define BME_DEBUG_THROW_EXCEPTION(code)\
{\
    if (code == ReturnSuccess) {\
        bmeDebugPrint(DebugError, "", "", __LINE__, "%s(): invalid code path for ReturnSuccess\n", __FUNCTION__);\
    } else {\
        Exception exception(code, __FILE__, __LINE__);\
        bmeDebugPrint(DebugError, "", "", __LINE__, "%s(): throwing exception: %s\n", __FUNCTION__, exception.what());\
    }\
    abort();\
}

#endif /* __SHARED_DEBUG_H_ */
