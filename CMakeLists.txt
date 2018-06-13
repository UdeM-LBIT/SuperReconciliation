cmake_minimum_required(VERSION 3.0)

project(super_reconciliation)
set(CMAKE_CXX_STANDARD 17)

include_directories(lib)

set(MAIN_SOURCES
    src/main.cpp
    src/Event.cpp
    src/small_philogeny.cpp
    src/Synteny.cpp
    src/tree_parser.cpp
    src/util.cpp)

add_executable(main ${MAIN_SOURCES})

set(RANDOM_SOURCES
    src/random.cpp
    src/Event.cpp
    src/small_philogeny.cpp
    src/Synteny.cpp
    src/tree_parser.cpp
    src/util.cpp)

add_executable(random ${RANDOM_SOURCES})

set(VIZ_SOURCES
    src/viz.cpp
    src/Event.cpp
    src/Synteny.cpp
    src/tree_parser.cpp
    src/util.cpp)

add_executable(viz ${VIZ_SOURCES})

set(TESTS_SOURCES
    src/tests.cpp
    src/ExtendedNumber.test.cpp
    src/Synteny.cpp
    src/Synteny.test.cpp)

add_executable(tests ${TESTS_SOURCES})
