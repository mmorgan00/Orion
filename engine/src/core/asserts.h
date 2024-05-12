#pragma once

#include <defines.h>

#define OASSERTIONS_ENABLED

#ifdef OASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

OAPI void report_assertion_failed(const char *expression, const char *message,
                                  const char *file, i32 line);

#define OASSERT(expr)                                                          \
  {                                                                            \
    if (expr) {                                                                \
    } else {                                                                   \
      report_assertion_failed(#expr, "", __FILE__, __LINE__);                  \
      debugBreak();                                                            \
    }                                                                          \
  }

#define OASSERT_MSG(expr, message)                                             \
  {                                                                            \
    if (expr) {                                                                \
    } else {                                                                   \
      report_assertion_failed(#expr, message, __FILE__, __LINE__);             \
      debugBreak();                                                            \
    }                                                                          \
  }

#ifdef _DEBUG
#define OASSERT_DEBUG(expr)                                                    \
  {                                                                            \
    if (expr) {                                                                \
    } else {                                                                   \
      report_assertion_failed(#expr, "", __FILE__, __LINE__);                  \
      debugBreak();                                                            \
    }                                                                          \
  }
#else
#define OASSERT_DEBUG(expr) // Does nothing at all
#endif

#else
#define OASSERT(expr)              // Does nothing at all
#define OASSERT_MSG(expr, message) // Does nothing at all
#define OASSERT_DEBUG(expr)        // Does nothing at all
#endif