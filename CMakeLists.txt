cmake_minimum_required(VERSION 3.16)
project(riscv_sim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra)

add_library(riscv_sim_core
    src/registers.cpp
    src/memory.cpp
    src/decoder.cpp
    src/encoder.cpp
    src/cpu.cpp
    src/loader.cpp
)
target_include_directories(riscv_sim_core PUBLIC include)

add_executable(riscv-sim src/main.cpp)
target_link_libraries(riscv-sim PRIVATE riscv_sim_core)

add_executable(build_examples tools/build_examples.cpp)
target_link_libraries(build_examples PRIVATE riscv_sim_core)

enable_testing()
add_executable(riscv_sim_tests
    tests/test_main.cpp
    tests/test_registers.cpp
    tests/test_memory.cpp
    tests/test_decoder.cpp
    tests/test_instructions.cpp
    tests/test_integration.cpp
)
target_link_libraries(riscv_sim_tests PRIVATE riscv_sim_core)
target_include_directories(riscv_sim_tests PRIVATE tests)
target_compile_definitions(riscv_sim_tests PRIVATE
    RISCV_SIM_EXECUTABLE="$<TARGET_FILE:riscv-sim>"
    EXAMPLES_DIR="${CMAKE_SOURCE_DIR}/examples"
)
add_dependencies(riscv_sim_tests riscv-sim)
add_test(NAME riscv_sim_tests COMMAND riscv_sim_tests)
