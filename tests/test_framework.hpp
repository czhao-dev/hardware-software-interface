// A minimal, header-only, zero-dependency test harness.
//
// TEST_CASE("name") { ... } registers a test function that the runner in
// test_main.cpp discovers and executes automatically. CHECK records a
// failure and keeps running the test; REQUIRE aborts the current test case.
#pragma once

#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace testfw {

struct TestFailure : std::exception {
    std::string message;
    explicit TestFailure(std::string m) : message(std::move(m)) {}
    const char* what() const noexcept override { return message.c_str(); }
};

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct Registrar {
    Registrar(std::string name, std::function<void()> fn) {
        registry().push_back({std::move(name), std::move(fn)});
    }
};

inline int& failure_count() {
    static int count = 0;
    return count;
}

}  // namespace testfw

#define TESTFW_CONCAT_(a, b) a##b
#define TESTFW_CONCAT(a, b) TESTFW_CONCAT_(a, b)

#define TEST_CASE(name)                                                                  \
    static void TESTFW_CONCAT(testfw_fn_, __LINE__)();                                   \
    static ::testfw::Registrar TESTFW_CONCAT(testfw_reg_, __LINE__)(                     \
        name, TESTFW_CONCAT(testfw_fn_, __LINE__));                                      \
    static void TESTFW_CONCAT(testfw_fn_, __LINE__)()

#define CHECK(expr)                                                                      \
    do {                                                                                 \
        if (!(expr)) {                                                                  \
            std::cerr << "  CHECK failed: " #expr " at " << __FILE__ << ":" << __LINE__ \
                       << "\n";                                                         \
            ++::testfw::failure_count();                                                \
        }                                                                               \
    } while (0)

#define REQUIRE(expr)                                                                    \
    do {                                                                                \
        if (!(expr)) {                                                                  \
            std::ostringstream oss;                                                     \
            oss << "REQUIRE failed: " #expr " at " << __FILE__ << ":" << __LINE__;       \
            throw ::testfw::TestFailure(oss.str());                                      \
        }                                                                               \
    } while (0)

#define CHECK_THROWS_AS(expr, ExceptionType)                                             \
    do {                                                                                \
        bool testfw_threw = false;                                                      \
        try {                                                                           \
            (void)(expr);                                                              \
        } catch (const ExceptionType&) {                                                \
            testfw_threw = true;                                                        \
        }                                                                               \
        if (!testfw_threw) {                                                            \
            std::cerr << "  CHECK_THROWS_AS failed: " #expr " did not throw "           \
                       #ExceptionType " at " << __FILE__ << ":" << __LINE__ << "\n";     \
            ++::testfw::failure_count();                                                \
        }                                                                               \
    } while (0)
