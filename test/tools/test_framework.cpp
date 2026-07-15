#include "test_framework.hpp"

#include <assert.h>
#include <cstdarg>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "src/utils/time.hpp"

namespace tools
{

detail::Config& getConfig()
{
    static detail::Config config;
    return config;
}

}  // namespace tools

namespace tools::detail
{

#define RED                 "\e[31m"
#define GREEN               "\e[32m"
#define YELLOW              "\e[33m"
#define BLUE                "\e[34m"
#define GREY                "\e[38;5;245m"
#define RESET               "\e[0m"

#define FAIL_MESSAGE        RED    "[ FAIL ]" RESET
#define PASS_MESSAGE        GREEN  "[ PASS ]" RESET
#define SKIP_MESSAGE        YELLOW "[ SKIP ]" RESET
#define RUN_MESSAGE         GREEN  "[ RUN  ]" RESET
#define GEN_GREEN_MESSAGE   GREEN  "[======]" RESET
#define GEN_RED_MESSAGE     RED    "[======]" RESET
#define GEN_YELLOW_MESSAGE  YELLOW "[======]" RESET

struct TestCaseRuntime final
{
    bool        skip;
    uint32_t    failures;
    const char* name;
    std::string param;
};

struct TestContext final
{
    uint32_t        tests;
    uint32_t        passedTests;
    uint32_t        failedTests;
    uint32_t        skippedTests;
    uint32_t        assertions;
    uint32_t        failedAssertions;
    TestCaseRuntime current;
};

static TestCaseRuntime* currentTest;
static TestCase* testCases;
static TestCase* lastTestCase;
static utils::Timer timer;
static TestContext ctx{};

static void printRow(int indentation, const char* column1, const char* column2, char end = '\n')
{
    for (int i = 0; i < indentation; ++i)
    {
        fputs("    ", stdout);
    }
    fprintf(stdout, "%-*s %s", 40 - indentation * 4, column1, column2);
    if (end)
    {
        putc(end, stdout);
    }
}

static void printNewline()
{
    fputc('\n', stdout);
}

static void printValue(const std::string& str)
{
    for (const uint8_t c : str)
    {
        switch (c)
        {
            case 0x00 ... 0x1f:
            case 0x7f ... 0xff:
                fprintf(stdout, GREY "<%02x>" RESET, c);
                break;
            default:
                putc(c, stdout);
        }
    }
}

static void printTestName(TestCaseRuntime& t, const char* message)
{
    fprintf(stdout, "%s %s", message, t.name);
    if (not t.param.empty())
    {
        fprintf(stdout, " with param \"");
        printValue(t.param);
        putc('\"', stdout);
    }
    putc('\n', stdout);
}

void registerTest(TestCase& tc)
{
    if (lastTestCase)
    {
        lastTestCase->next = &tc;
        lastTestCase = &tc;
        return;
    }

    if (not testCases)
    {
        testCases = &tc;
        lastTestCase = testCases;
    }
}

int main(int argc, char* argv[])
{
    auto& config = getConfig();

    if (config.useColors)
    {
        config.useColors = isatty(STDOUT_FILENO);
    }

    for (int i = 0; i < argc; ++i)
    {
        if (not strcmp(argv[i], "-m"))
        {
            config.minimal = true;
        }
        else if (not strcmp(argv[i], "--no-color"))
        {
            config.useColors = false;
        }
        else if (not strcmp(argv[i], "--force-color"))
        {
            config.useColors = true;
        }
    }

    currentTest = &ctx.current;

    for (auto tc = testCases; tc; tc = tc->next)
    {
        tc->testBody();
    }

    fprintf(stdout, GEN_GREEN_MESSAGE " %u passed\n", ctx.passedTests);
    if (ctx.skippedTests)
    {
        fprintf(stdout, GEN_YELLOW_MESSAGE " %u skipped\n", ctx.skippedTests);
    }
    if (ctx.failedTests)
    {
        fprintf(stdout, GEN_RED_MESSAGE " %u failed\n", ctx.failedTests);
    }
    return ctx.failedAssertions > 0;
}

void startTest(const char* name, std::string param)
{
    ctx.current = {
        .skip = false,
        .failures = 0,
        .name = name,
        .param = std::move(param),
    };
    timer = utils::startTimeMeasurement();
    if (not getConfig().minimal)
    {
        printTestName(ctx.current, RUN_MESSAGE);
    }
}

void stopTest()
{
    const bool print = not getConfig().minimal;

    if (ctx.current.skip)
    {
        ctx.skippedTests++;
        if (print)
        {
            printTestName(ctx.current, SKIP_MESSAGE);
        }
    }
    else if (ctx.current.failures)
    {
        ctx.failedTests++;
        printTestName(ctx.current, FAIL_MESSAGE);
    }
    else
    {
        ctx.passedTests++;
        if (print)
        {
            printTestName(ctx.current, PASS_MESSAGE);
        }
    }
}

static void printFailureHeader(const char* file, size_t line)
{
    fprintf(stdout, "    " RED "Failure from " BLUE "%s" RESET ":%zu\n", file, line);
}

static void printActual(const char* stringified, const std::string& value)
{
    printRow(2, "expected:", stringified);
    printRow(3, "which is:", "", '\0');
    printValue(value);
    printNewline();
}

static void printExpected(const char* stringified, BinaryOp op, const std::string& value)
{
    const char* operation = nullptr;
    switch (op)
    {
        case BinaryOp::EQ:
            operation = "to be equal to:";
            break;
        case BinaryOp::NE:
            operation = "to be not equal to:";
            break;
        case BinaryOp::LT:
            operation = "to be less than:";
            break;
        case BinaryOp::GT:
            operation = "to be greater than:";
            break;
        case BinaryOp::LE:
            operation = "to be less or equal to:";
            break;
        case BinaryOp::GE:
            operation = "to be greater or equal to:";
            break;
    }
    assert(operation);
    printRow(2, operation, stringified);
    printRow(3, "which is:", "", '\0');
    printValue(value);
    printNewline();
}

static void printExpected(bool value)
{
    printRow(2, "to be:", stringifyValue(value).c_str());
}

void reportBinaryOpFailure(
    const char* file,
    size_t line,
    const char* actual,
    const std::string& actualValue,
    const char* expected,
    const std::string& expectedValue,
    BinaryOp op)
{
    ++ctx.current.failures;
    printFailureHeader(file, line);
    printActual(actual, actualValue);
    printExpected(expected, op, expectedValue);
    printNewline();
}

void reportFailure(
    const char* file,
    size_t line,
    const char* fmt,
    ...)
{
    ++ctx.current.failures;
    printFailureHeader(file, line);
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    printNewline();
}

void reportBoolFailure(
    const char* file,
    size_t line,
    bool expected,
    const char* actual,
    const std::string& actualValue)
{
    ++ctx.current.failures;
    printFailureHeader(file, line);
    printActual(actual, actualValue);
    printExpected(expected);
    printNewline();
}

#if __cpp_exceptions >= 199711L
static std::string getException()
{
    try
    {
        throw;
    }
    catch (std::string e)
    {
        return e;
    }
    catch (const char* e)
    {
        return e;
    }
    catch (const std::exception& e)
    {
        return e.what();
    }
    catch (...)
    {
        return "Unrecognized exception";
    }
}

void reportUnexpectedException(const char* file, size_t line)
{
    ++ctx.current.failures;
    printFailureHeader(file, line);
    printRow(2, "unexpected exception:", getException().c_str());
    printNewline();
}

void reportMissingExceptionFailure(const char* file, size_t line)
{
    ++ctx.current.failures;
    printFailureHeader(file, line);
    printRow(2, "expected:", "exception to be thrown");
    printNewline();
}
#endif

void reportSkip()
{
    ctx.current.skip = true;
}

std::string convertInteger(int64_t value)
{
    char buffer[128];
    int size = 0;
    if (getConfig().printIntegersAsHex)
    {
        if (value < 0)
        {
            size = sprintf(buffer, "-%#" PRIx64, labs(value));
        }
        else
        {
            size = sprintf(buffer, "%#" PRIx64, value);
        }
    }
    else
    {
        size = sprintf(buffer, "%" PRIi64, value);
    }
    return std::string(buffer, size);
}

std::string convertInteger(uint64_t value)
{
    char buffer[128];
    int size = 0;
    if (getConfig().printIntegersAsHex)
    {
        size = sprintf(buffer, "%#" PRIx64, value);
    }
    else
    {
        size = sprintf(buffer, "%" PRIu64, value);
    }
    return std::string(buffer, size);
}

#define MIN(a, b)   ({ a < b ? a : b; })

std::string convertBinary(const uint8_t* data, size_t size)
{
    char buffer[128];
    char* it = buffer;
    *it++ = '{';

    size_t i;
    size_t bytesToPrint = MIN(size, 8);
    for (i = 0; i < bytesToPrint; ++i)
    {
        it += sprintf(it, "%02x", data[i]);
        if (i != bytesToPrint - 1)
        {
            *it++ = ' ';
        }
    }
    if (i != size)
    {
        it += sprintf(it, "...");
    }
    *it++ = '}';
    return std::string(buffer, it - buffer);
}

const std::string& stringifyValue()
{
    static std::string emptyString;
    return emptyString;
}

const std::string& stringifyValue(bool b)
{
    static std::string str[] = {"false", "true"};
    return str[!!b];
}

std::string stringifyValue(const std::string_view& c)
{
    return std::string(c);
}

std::string stringifyValue(const char* c)
{
    return c;
}

}  // namespace tools::detail
