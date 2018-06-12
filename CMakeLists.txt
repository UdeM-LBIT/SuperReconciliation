cmake_minimum_required(VERSION 3.0)

project(super_reconciliation)
set(CMAKE_CXX_STANDARD 14)

set(SOURCES
    src/Event.cpp
    src/EventTree.cpp
    src/Synteny.cpp
    src/util.cpp)

set(TEST_SOURCES
    src/ExtendedNumber.test.cpp
    src/Synteny.test.cpp
    src/test.cpp)

include_directories(lib)
add_executable(event_tree_viz ${SOURCES} src/event_tree_viz.cpp)
add_executable(small_philogeny ${SOURCES} src/small_philogeny.cpp)
add_executable(test ${SOURCES} ${TEST_SOURCES})
