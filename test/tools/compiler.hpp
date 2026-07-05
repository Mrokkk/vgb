#pragma once

#ifdef __clang__
#define CLANG_PRAGMA(M) _Pragma(#M)
#define CLANG_DIAGNOSTIC_PUSH()     CLANG_PRAGMA(clang diagnostic push)
#define CLANG_DIAGNOSTIC_POP()      CLANG_PRAGMA(clang diagnostic pop)
#define CLANG_DIAGNOSTIC_IGNORED(M) CLANG_PRAGMA(clang diagnostic ignored M)
#else
#define CLANG_DIAGNOSTIC_PUSH()
#define CLANG_DIAGNOSTIC_POP()
#define CLANG_DIAGNOSTIC_IGNORED(M)
#endif

#define FORMAT(...)     __attribute__((format(__VA_ARGS__)))
#define ARRAY_SIZE(A)   (sizeof(A) / sizeof(*A))
