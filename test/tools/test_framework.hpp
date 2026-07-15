#pragma once

#include <concepts>
#include <stdint.h>
#include <string.h>

#include <string>
#include <string_view>
#include <type_traits>

#include "test/tools/compiler.hpp"

#ifdef USE_FMTLIB
#ifndef FMTLIB_FORMAT_INCLUDE
#define FMTLIB_FORMAT_INCLUDE <fmt/format.h>
#include FMTLIB_FORMAT_INCLUDE
#endif
#endif

#ifdef USE_STD_OSTREAM
#include <sstream>
#endif

#ifdef USE_STD_FORMAT
#include <format>
#endif

#if defined(NO_EXCEPTIONS) or __cpp_exceptions < 199711L
#define _USE_EXCEPTIONS 0
#else
#define _USE_EXCEPTIONS 1
#endif

template <typename T>
struct TestStringConverter
{
};

namespace tools::detail
{

struct Config
{
    bool minimal = false;
    bool useColors = true;
    bool printIntegersAsHex = false;
};

struct TestCase final
{
    void (*testBody)();
    TestCase* next;
};

void registerTest(TestCase& tc);
int main(int argc, char* argv[]);

enum class BinaryOp
{
    EQ,
    NE,
    LT,
    GT,
    LE,
    GE,
};

std::string convertInteger(int64_t value);
std::string convertInteger(uint64_t value);
std::string convertBinary(const uint8_t* data, size_t size);

void reportBinaryOpFailure(
    const char* file,
    size_t line,
    const char* expected,
    const std::string& expectedValue,
    const char* actual,
    const std::string& actualValue,
    BinaryOp op);

void reportFailure(
    const char* file,
    size_t line,
    const char* fmt,
    ...) FORMAT(printf, 3, 4);

void reportBoolFailure(
    const char* file,
    size_t line,
    bool expected,
    const char* actual,
    const std::string& actualValue);

#if _USE_EXCEPTIONS
void reportUnexpectedException(const char* file, size_t line);
void reportMissingExceptionFailure(const char* file, size_t line);
#endif

void reportSkip();

const std::string& stringifyValue(bool b);

std::string stringifyValue(const char* c);
std::string stringifyValue(const std::string_view& c);

constexpr const std::string& stringifyValue(const std::string& s)
{
    return s;
}

template <typename T>
concept HasStringConverter = requires(T t)
{
    requires std::same_as<std::string, decltype(::TestStringConverter<T>::convert(t))>;
};

#ifdef USE_STD_OSTREAM
template <typename T>
concept HasOstreamPrinter = requires(T t)
{
    std::stringstream() << t;
};
#endif

const std::string& stringifyValue();

template <typename T>
std::string stringifyValue(const T& value)
{
    if constexpr (HasStringConverter<T>)
    {
        return ::TestStringConverter<T>::convert(value);
    }
    else if constexpr (std::is_integral_v<T> and std::is_signed_v<T>)
    {
        return convertInteger(static_cast<int64_t>(value));
    }
    else if constexpr (std::is_integral_v<T>)
    {
        return convertInteger(static_cast<uint64_t>(value));
    }
#ifdef USE_FMTLIB
    else if constexpr (fmt::is_formattable<T>::value)
    {
        return ::fmt::format("{}", value);
    }
#endif
#ifdef USE_STD_OSTREAM
    else if constexpr (HasOstreamPrinter<T>)
    {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
#endif
#ifdef USE_STD_FORMAT
    else if constexpr (std::formattable<T, char>)
    {
        return std::format("{}", value);
    }
#endif
    else
    {
        return convertBinary(reinterpret_cast<const uint8_t*>(&value), sizeof(value));
    }
}

void startTest(const char* name, std::string param);
void stopTest();

#define TEST_NS         ::tools::detail
#define USE_TEST_NS     using namespace TEST_NS

#define SUPPRESS_WARNINGS_START() \
    GCC_DIAGNOSTIC_PUSH(); \
    GCC_DIAGNOSTIC_IGNORED("-Wsign-compare"); \
    GCC_DIAGNOSTIC_IGNORED("-Wmaybe-uninitialized"); \
    CLANG_DIAGNOSTIC_PUSH(); \
    CLANG_DIAGNOSTIC_IGNORED("-Wsign-compare"); \

#define SUPPRESS_WARNINGS_END() \
    GCC_DIAGNOSTIC_POP(); \
    CLANG_DIAGNOSTIC_POP()

#define _EXPECT_OP_IMPL(LHS, RHS, OP, OPERATOR, ON_FAIL) \
    ({ \
        SUPPRESS_WARNINGS_START(); \
        const auto& lhs = LHS; \
        const auto& rhs = RHS; \
        const bool b = lhs OPERATOR rhs; \
        if (not b) [[unlikely]] \
        { \
            USE_TEST_NS; \
            reportBinaryOpFailure( \
                __FILE__, \
                __LINE__, \
                #LHS, \
                stringifyValue(lhs), \
                #RHS, \
                stringifyValue(rhs), \
                BinaryOp::OP); \
            ON_FAIL; \
        } \
        SUPPRESS_WARNINGS_END(); \
        b; \
    })

#define _EXPECT_STREQ_IMPL(LHS, RHS, ON_FAIL) \
    ({ \
        const auto lhs = LHS; \
        const auto rhs = RHS; \
        const bool b = not strcmp(LHS, RHS); \
        if (not b) [[unlikely]] \
        { \
            USE_TEST_NS; \
            reportBinaryOpFailure( \
                __FILE__, \
                __LINE__, \
                #LHS, \
                stringifyValue(lhs), \
                #RHS, \
                stringifyValue(rhs), \
                BinaryOp::EQ); \
            ON_FAIL; \
        } \
        b; \
    })

#define _EXPECT_BOOL_IMPL(COND, BOOL, ON_FAIL) \
    ({ \
        const auto b = static_cast<bool>(COND); \
        if (b != BOOL) [[unlikely]] \
        { \
            USE_TEST_NS; \
            reportBoolFailure(__FILE__, __LINE__, BOOL, #COND, stringifyValue(COND)); \
            ON_FAIL; \
        } \
        b; \
    })

#if _USE_EXCEPTIONS
#define _EXPECT_THROWS_IMPL(ON_FAIL, ...) \
    ({ \
        try \
        { \
            (void)(__VA_ARGS__); \
            USE_TEST_NS; \
            reportMissingExceptionFailure(__FILE__, __LINE__); \
            ON_FAIL; \
        } \
        catch (...) \
        { \
        } \
    })
#else
#define _EXPECT_THROWS_IMPL(ON_FAIL, ...) \
    ({ \
        _STATIC_ASSERT(0, "error \"Exceptions has been disabled with NO_EXCEPTIONS or -fno-exceptions\""); \
    })
#endif

#define _FAIL_AT_IMPL(FILE, LINE, ON_FAIL, ...) \
    ({ \
        USE_TEST_NS; \
        reportFailure(FILE, LINE __VA_OPT__(,) __VA_ARGS__); \
        ON_FAIL; \
    })

#define _TEST_CASE_REGISTRATOR(NAME, FUNC) \
    static void registerSelf() \
    { \
        USE_TEST_NS; \
        static TestCase t{ \
            .testBody = &FUNC, \
            .next = nullptr, \
        }; \
        registerTest(t); \
    } \
    static inline bool registered = (registerSelf(), true)

#if _USE_EXCEPTIONS
#define _TEST_RUN(CLASS, NAME, ...) \
    TEST_NS::startTest(NAME, TEST_NS::stringifyValue(__VA_ARGS__)); \
    { \
        try \
        { \
            CLASS f; \
            f.testBody(__VA_ARGS__); \
        } \
        catch (...) \
        { \
            TEST_NS::reportUnexpectedException(__FILE__, __LINE__); \
        } \
    } \
    TEST_NS::stopTest()
#else
#define _TEST_RUN(CLASS, NAME, ...) \
    TEST_NS::startTest(NAME, TEST_NS::stringifyValue(__VA_ARGS__)); \
    { \
        CLASS f; \
        f.testBody(__VA_ARGS__); \
    } \
    TEST_NS::stopTest()
#endif

#define _TEST_CASE_IMPL(CLASS, FUNC, FIXTURE, NAME) \
    namespace \
    { \
    static void FUNC(); \
    class CLASS final FIXTURE \
    { \
        friend void FUNC(); \
        _TEST_CASE_REGISTRATOR(NAME, FUNC); \
        void testBody(); \
    }; \
    static void FUNC() \
    { \
        _TEST_RUN(CLASS, NAME); \
    } \
    } \
    void CLASS::testBody()

#define _TEST_CASE_P_IMPL(CLASS, FUNC, FIXTURE, NAME, ...) \
    namespace \
    { \
    static void FUNC(); \
    class CLASS final FIXTURE \
    { \
        friend void FUNC(); \
        _TEST_CASE_REGISTRATOR(NAME, FUNC); \
        template <typename T> \
        void testBody(const T&); \
    }; \
    void FUNC() \
    { \
        for (const auto& e : __VA_ARGS__) \
        { \
            _TEST_RUN(CLASS, NAME, e); \
        } \
    } \
    } \
    template <typename T> \
    void CLASS::testBody(const T& arg)

#define _CAT_IMPL(A, B)      A ## B
#define _CAT(A, B)           _CAT_IMPL(A, B)
#define _UNIQUE_NAME(PREFIX) _CAT(PREFIX ##_, __COUNTER__)

}  // namespace tools::detail

namespace tools
{
detail::Config& getConfig();
}  // namespace tools

#define EXPECT(COND)                _EXPECT_BOOL_IMPL(COND, true,)
#define EXPECT_FALSE(COND)          _EXPECT_BOOL_IMPL(COND, false,)
#define EXPECT_EQ(LHS, RHS)         _EXPECT_OP_IMPL(LHS, RHS, EQ, ==,)
#define EXPECT_NE(LHS, RHS)         _EXPECT_OP_IMPL(LHS, RHS, NE, !=,)
#define EXPECT_LT(LHS, RHS)         _EXPECT_OP_IMPL(LHS, RHS, LT, <,)
#define EXPECT_GT(LHS, RHS)         _EXPECT_OP_IMPL(LHS, RHS, GT, >,)
#define EXPECT_LE(LHS, RHS)         _EXPECT_OP_IMPL(LHS, RHS, LE, <=,)
#define EXPECT_GE(LHS, RHS)         _EXPECT_OP_IMPL(LHS, RHS, GE, >=,)
#define EXPECT_STREQ(LHS, RHS)      _EXPECT_STREQ_IMPL(LHS, RHS, )
#define EXPECT_THROWS(...)          _EXPECT_THROWS_IMPL(, __VA_ARGS__)
#define FAIL(...)                   _FAIL_AT_IMPL(__FILE__, __LINE__,, __VA_ARGS__)

#define REQUIRE(COND)               _EXPECT_BOOL_IMPL(COND, true, return)
#define REQUIRE_FALSE(COND)         _EXPECT_BOOL_IMPL(COND, false, return)
#define REQUIRE_EQ(LHS, RHS)        _EXPECT_OP_IMPL(LHS, RHS, EQ, ==, return)
#define REQUIRE_NE(LHS, RHS)        _EXPECT_OP_IMPL(LHS, RHS, NE, !=, return)
#define REQUIRE_LT(LHS, RHS)        _EXPECT_OP_IMPL(LHS, RHS, LT, <, return)
#define REQUIRE_GT(LHS, RHS)        _EXPECT_OP_IMPL(LHS, RHS, GT, >, return)
#define REQUIRE_LE(LHS, RHS)        _EXPECT_OP_IMPL(LHS, RHS, LE, <=, return)
#define REQUIRE_GE(LHS, RHS)        _EXPECT_OP_IMPL(LHS, RHS, GE, >=, return)
#define REQUIRE_STREQ(LHS, RHS)     _EXPECT_STREQ_IMPL(LHS, RHS, return)
#define REQUIRE_THROWS(...)         _EXPECT_THROWS_IMPL(return, __VA_ARGS__)
#define FATAL(...)                  _FAIL_AT_IMPL(__FILE__, __LINE__, return, __VA_ARGS__)

#define SKIP() \
    ({ \
        TEST_NS::reportSkip(); \
        return; \
    })

#define TEST_CASE(NAME) \
    _TEST_CASE_IMPL(_UNIQUE_NAME(TestCase), _UNIQUE_NAME(runWrapper), , NAME)

#define TEST_CASE_FIXTURE(FIXTURE, NAME) \
    _TEST_CASE_IMPL(_UNIQUE_NAME(TestCase), _UNIQUE_NAME(runWrapper), : FIXTURE, NAME)

#define TEST_CASE_FIXTURE_P(FIXTURE, NAME, ...) \
    _TEST_CASE_P_IMPL(_UNIQUE_NAME(TestCase), _UNIQUE_NAME(runWrapper), : FIXTURE, NAME, __VA_ARGS__)

#define TEST_CASES_RUN(ARGC, ARGV) \
    TEST_NS::main(ARGC, ARGV)
