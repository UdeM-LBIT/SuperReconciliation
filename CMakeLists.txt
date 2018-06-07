cmake_minimum_required(VERSION 3.0)

project(super_reconciliation)
set(CMAKE_CXX_STANDARD 14)

set(SOURCES
    src/Event.cpp
    src/Synteny.cpp)

set(TEST_SOURCES
    src/ExtendedNumber.test.cpp
    src/Synteny.test.cpp
    src/test.cpp)

include_directories(lib)
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} src/main.cpp)
add_executable(test ${SOURCES} ${TEST_SOURCES})
