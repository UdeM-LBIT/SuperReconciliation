cmake_minimum_required(VERSION 3.0)
project(super_reconciliation)

# Compiler settings
set(CMAKE_CXX_STANDARD 17)

# Common library
add_library(common
    src/io/nhx_parser.cpp
    src/io/util.cpp
    src/model/Event.cpp
    src/model/Synteny.cpp
    src/super_reconciliation.cpp
)

target_include_directories(common PUBLIC lib)

# `main` executable
add_executable(main src/main.cpp)
target_link_libraries(main common)
target_include_directories(main PUBLIC lib)

# `random` executable
add_executable(random src/random.cpp)
target_link_libraries(random common)
target_include_directories(random PUBLIC lib)

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
