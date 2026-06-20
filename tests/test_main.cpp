#include <iostream>

#include "test_framework.hpp"

int main() {
    int total = 0;
    int failed = 0;

    for (auto& test : testfw::registry()) {
        ++total;
        int failures_before = testfw::failure_count();
        try {
            test.fn();
        } catch (const testfw::TestFailure& e) {
            std::cerr << "FAIL " << test.name << ": " << e.message << "\n";
            ++failed;
            continue;
        } catch (const std::exception& e) {
            std::cerr << "FAIL " << test.name << ": unexpected exception: " << e.what() << "\n";
            ++failed;
            continue;
        }
        if (testfw::failure_count() != failures_before) {
            std::cerr << "FAIL " << test.name << "\n";
            ++failed;
        } else {
            std::cout << "PASS " << test.name << "\n";
        }
    }

    std::cout << "\n" << (total - failed) << "/" << total << " tests passed\n";
    return failed == 0 ? 0 : 1;
}
