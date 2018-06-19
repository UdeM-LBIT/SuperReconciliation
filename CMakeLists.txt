cmake_minimum_required(VERSION 3.0)
project(super_reconciliation)

# Compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Common library
add_library(common
    src/io/nhx_parser.cpp
    src/io/util.cpp
    src/model/Event.cpp
    src/model/Synteny.cpp
    src/algo/super_reconciliation.cpp
)

target_include_directories(common PUBLIC lib)

# `super_reconciliation` executable
add_executable(super_reconciliation src/super_reconciliation.cpp)
target_link_libraries(super_reconciliation common)
target_include_directories(super_reconciliation PUBLIC lib)

# `simulate` executable
add_executable(simulate src/simulate.cpp)
target_link_libraries(simulate common)
target_include_directories(simulate PUBLIC lib)

# `viz` executable
add_executable(viz src/viz.cpp)
target_link_libraries(viz common)
target_include_directories(viz PUBLIC lib)

# `tests` executable
add_executable(tests
    src/tests.cpp
    src/io/nhx_parser.test.cpp
    src/model/Synteny.test.cpp
    src/util/ExtendedNumber.test.cpp
)
target_link_libraries(tests common)
target_include_directories(tests PUBLIC lib)
