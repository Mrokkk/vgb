#pragma once

#if __cpp_static_assert >= 202306L
#define _STATIC_ASSERT(COND, MSG) static_assert(COND, MSG)
#else
#define _STATIC_ASSERT(COND, MSG) static_assert(COND && MSG)
#endif

#ifdef __clang__
#define CLANG_PRAGMA(M) _Pragma(#M)
#elif defined(__GNUC__) and defined(__GNUC_MINOR__) and defined(__GNUC_PATCHLEVEL__)
#define GCC_PRAGMA(M) _Pragma(#M)
#endif

#ifndef CLANG_PRAGMA
#define CLANG_PRAGMA(...)
#endif

#ifndef GCC_PRAGMA
#define GCC_PRAGMA(...)
#endif

#define CLANG_DIAGNOSTIC_PUSH()     CLANG_PRAGMA(clang diagnostic push)
#define CLANG_DIAGNOSTIC_POP()      CLANG_PRAGMA(clang diagnostic pop)
#define CLANG_DIAGNOSTIC_IGNORED(M) CLANG_PRAGMA(clang diagnostic ignored M)

#define GCC_DIAGNOSTIC_PUSH()       GCC_PRAGMA(GCC diagnostic push)
#define GCC_DIAGNOSTIC_POP()        GCC_PRAGMA(GCC diagnostic pop)
#define GCC_DIAGNOSTIC_IGNORED(M)   GCC_PRAGMA(GCC diagnostic ignored M)

#define FORMAT(...)     __attribute__((format(__VA_ARGS__)))
#define ARRAY_SIZE(A)   (sizeof(A) / sizeof(*A))
